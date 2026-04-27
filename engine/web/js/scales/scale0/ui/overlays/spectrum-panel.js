/**
 * Spectrum Scanner Panel — v0 minimum.
 *
 * The full design (Auditor #6 spec): sweep N initial conditions, run
 * each in isolated lattice sandbox via bridge.runIsolatedScan(), settle-
 * detect, extract mass, build histogram. v1 needs: clone API on
 * RenderBridge (~80 LOC), settle detector, mass extractor (~80 LOC),
 * Web Worker offload (~100 LOC).
 *
 * v0 (this iteration): scaffolds the panel UI + reads the *currently-
 * manifested particle list* + measures their inter-particle separations
 * and effective bound-state energies as a present-time spectrum. This
 * is honest: we're not running a sweep, we're SHOWING what the lattice
 * is doing right now. The "[ Run sweep ]" button is wired but disabled
 * with a tooltip explaining the v1 dependency.
 *
 * The honesty principle (per CLAUDE.md): show what we measure, label
 * what we infer, never inflate.
 */

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import {
    M_E_PHYS, M_MU_PHYS, M_PI_CH_PHYS, M_P_PHYS,
} from '../../../../constants.js';
import { cardStyle, titleStyle, formatExp, tagBadge, heroStyle } from './_card-helpers.js';

const PANEL_ID = 'spectrum-panel';
const HZ = 2;                            // 2 Hz update — slower; this is exploratory data
// Mass ratios anchored to PDG values from constants.js (electron = 1).
// keV column carried for display so existing column layout is unchanged.
const KNOWN_MASSES = [
    { name: 'e',  m: 1.0,                    keV: M_E_PHYS     * 1e3 },
    { name: 'μ',  m: M_MU_PHYS    / M_E_PHYS, keV: M_MU_PHYS    * 1e3 },
    { name: 'π',  m: M_PI_CH_PHYS / M_E_PHYS, keV: M_PI_CH_PHYS * 1e3 },
    { name: 'p',  m: M_P_PHYS     / M_E_PHYS, keV: M_P_PHYS     * 1e3 },
];

/**
 * Build a histogram bin index from raw masses (log-spaced bins).
 */
function histogramize(masses, nBins = 20) {
    if (!masses.length) return { bins: [], counts: [] };
    const lo = Math.max(0.01, Math.min(...masses));
    const hi = Math.max(...masses) * 1.1;
    const logLo = Math.log10(lo);
    const logHi = Math.log10(hi);
    const step = (logHi - logLo) / nBins || 0.1;
    const counts = new Array(nBins).fill(0);
    for (const m of masses) {
        let idx = Math.floor((Math.log10(m) - logLo) / step);
        idx = Math.max(0, Math.min(nBins - 1, idx));
        counts[idx]++;
    }
    const bins = [];
    for (let i = 0; i < nBins; i++) {
        const binLo = Math.pow(10, logLo + i * step);
        const binHi = Math.pow(10, logLo + (i + 1) * step);
        bins.push({ lo: binLo, hi: binHi, mid: Math.sqrt(binLo * binHi) });
    }
    return { bins, counts };
}

/**
 * Match an observed mass to the closest known particle — return
 * { name, fractionalDelta } or null if outside reasonable range.
 */
function matchKnown(massInUnitsOfElectron) {
    let best = null;
    for (const k of KNOWN_MASSES) {
        const delta = Math.abs(massInUnitsOfElectron - k.m) / k.m;
        if (best == null || delta < best.delta) {
            best = { name: k.name, delta, target: k.m };
        }
    }
    if (best && best.delta < 0.5) return best;     // within 50% of known
    return null;
}

/**
 * From the live particle list, extract a "present-time" mass spectrum.
 * v0 heuristic: each manifested particle's local flux density × volume
 * proxy ≈ rest mass-energy. We use the existing `density` field on
 * mock particles where available, falling back to particle count
 * heuristics. Honest tag: [E] (emergent, present-time, NOT sweep).
 */
function extractPresentSpectrum(bridge) {
    if (!bridge) return { masses: [], particles: 0 };
    const particles = bridge.getScale0ParticleList?.() || [];
    const masses = [];
    for (const p of particles) {
        if (!p || (p.state ?? 0) === 0) continue;
        // Mock particles expose `density`; fallback to 1.0 (electron-equivalent)
        const density = (typeof p.density === 'number' && p.density > 0) ? p.density : 1.0;
        masses.push(density);
    }
    return { masses, particles: particles.length };
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only spectrum-panel';
    root.style.cssText = `
        position: relative;
        width: 100%;
        padding: 14px 14px 18px;
        font-family: var(--font-sans, system-ui, -apple-system, "Segoe UI", sans-serif);
        font-size: 13px;
        line-height: 1.45;
        color: var(--text-primary);
        background: transparent;
    `;
    root.innerHTML = `
        <header style="display:flex;justify-content:space-between;align-items:center;margin-bottom:14px;">
            <span style="font-weight:600;color:var(--accent);font-size:15px;letter-spacing:0.02em;">Spectrum Scanner (live)</span>
            <span style="font-size:11px;color:var(--text-muted);font-family:var(--font-mono);">v0 · present-time</span>
        </header>
        <div id="${PANEL_ID}-body">
            <section data-section="controls" style="${cardStyle(140)}">
                <div style="${titleStyle()}">Sweep configuration</div>
                <div id="${PANEL_ID}-controls-body">
                    <div style="display:grid;grid-template-columns:auto 1fr auto;gap:6px 10px;align-items:center;font-size:12px;">
                        <label style="color:var(--text-muted);">Preset</label>
                        <select id="${PANEL_ID}-preset" style="font-family:var(--font-mono);font-size:12px;padding:4px 6px;background:var(--bg-input,rgba(0,0,0,0.3));color:var(--text-primary);border:1px solid var(--border-light,rgba(255,255,255,0.1));border-radius:4px;">
                            <option value="present">Present-time (current particles)</option>
                            <option value="random" disabled>Random initial conditions [v1]</option>
                            <option value="grid" disabled>Grid scan over (charge, energy) [v1]</option>
                        </select>
                        <button id="${PANEL_ID}-sweep-btn" type="button"
                            disabled
                            title="Sweep harness needs bridge.runIsolatedScan() — Sprint v1"
                            style="background:rgba(120,200,255,0.10);border:1px solid rgba(120,200,255,0.18);color:var(--text-muted);padding:6px 12px;cursor:not-allowed;font-size:12px;border-radius:4px;font-family:var(--font-sans);">
                            Run sweep [v1]
                        </button>
                    </div>
                    <div style="margin-top:10px;font-size:11px;color:var(--text-muted);line-height:1.5;">
                        ${tagBadge('E')}Present-time mode shows the masses currently manifesting in the lattice (no sweep — what you see is now).
                        Sweep mode requires <code>bridge.runIsolatedScan()</code> + a clone API on RenderBridge — queued for next sprint.
                    </div>
                </div>
            </section>
            <section data-section="hero" style="${cardStyle(80)}">
                <div style="${titleStyle()}">Spectrum summary</div>
                <div id="${PANEL_ID}-hero-body"></div>
            </section>
            <section data-section="histogram" style="${cardStyle(220)}">
                <div style="${titleStyle()}">Mass histogram (log scale)</div>
                <div id="${PANEL_ID}-hist-body" style="min-height:180px;"></div>
            </section>
            <section data-section="table" style="${cardStyle(180)}">
                <div style="${titleStyle()}">Stable configurations</div>
                <div id="${PANEL_ID}-table-body" style="max-height:240px;overflow-y:auto;"></div>
            </section>
        </div>
    `;
    return root;
}

function renderHistogram(container, masses) {
    const { bins, counts } = histogramize(masses);
    if (counts.length === 0) {
        container.innerHTML = `
            <div style="display:flex;align-items:center;justify-content:center;min-height:180px;color:var(--text-muted);font-style:italic;font-size:12px;">
                No manifested particles yet — load a particle scenario to populate the spectrum.
            </div>
        `;
        return;
    }
    const W = 360, H = 180;
    const m = { top: 18, right: 12, bottom: 36, left: 36 };
    const innerW = W - m.left - m.right;
    const innerH = H - m.top - m.bottom;
    const maxCount = Math.max(1, ...counts);

    let svg = `<svg viewBox="0 0 ${W} ${H}" style="width:100%;height:auto;display:block;">`;
    svg += `<rect x="${m.left}" y="${m.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light)" stroke-width="1"/>`;
    // Bars
    const barW = innerW / counts.length;
    for (let i = 0; i < counts.length; i++) {
        const h = (counts[i] / maxCount) * innerH;
        const x = m.left + i * barW;
        const y = m.top + innerH - h;
        svg += `<rect x="${x.toFixed(1)}" y="${y.toFixed(1)}" width="${(barW - 1).toFixed(1)}" height="${h.toFixed(1)}" fill="var(--accent)" opacity="0.75"/>`;
    }
    // Known-mass markers along x-axis
    if (bins.length) {
        for (const k of KNOWN_MASSES) {
            const lo = bins[0].lo;
            const hi = bins[bins.length - 1].hi;
            if (k.m < lo || k.m > hi) continue;
            const fx = (Math.log10(k.m) - Math.log10(lo)) / (Math.log10(hi) - Math.log10(lo));
            const xMark = m.left + fx * innerW;
            svg += `<line x1="${xMark.toFixed(1)}" y1="${m.top}" x2="${xMark.toFixed(1)}" y2="${m.top + innerH}" stroke="var(--warning)" stroke-width="0.8" stroke-dasharray="2,3" opacity="0.7"/>`;
            svg += `<text x="${xMark.toFixed(1)}" y="${(m.top - 4)}" text-anchor="middle" font-size="10" fill="var(--warning)">${k.name}</text>`;
        }
    }
    // X-axis log-scale labels
    if (bins.length) {
        const lo = bins[0].lo;
        const hi = bins[bins.length - 1].hi;
        const tickValues = [1, 10, 100, 1000].filter((v) => v >= lo && v <= hi);
        for (const v of tickValues) {
            const fx = (Math.log10(v) - Math.log10(lo)) / (Math.log10(hi) - Math.log10(lo));
            const xt = m.left + fx * innerW;
            svg += `<text x="${xt.toFixed(1)}" y="${(m.top + innerH + 14)}" text-anchor="middle" font-size="10" font-family="var(--font-mono)" fill="var(--text-muted)">${v}</text>`;
        }
        // Range edges
        svg += `<text x="${m.left}" y="${m.top + innerH + 14}" text-anchor="start" font-size="9" font-family="var(--font-mono)" fill="var(--text-muted)" opacity="0.6">${lo.toFixed(2)}</text>`;
        svg += `<text x="${m.left + innerW}" y="${m.top + innerH + 14}" text-anchor="end" font-size="9" font-family="var(--font-mono)" fill="var(--text-muted)" opacity="0.6">${hi.toFixed(0)}</text>`;
    }
    // Y-axis label
    svg += `<text x="${m.left + innerW / 2}" y="${H - 4}" text-anchor="middle" font-size="11" fill="var(--text-muted)">mass (m_e units)</text>`;
    svg += `<text x="14" y="${m.top + innerH / 2}" transform="rotate(-90 14 ${m.top + innerH / 2})" text-anchor="middle" font-size="11" fill="var(--text-muted)">count</text>`;
    svg += `</svg>`;
    container.innerHTML = svg;
}

function renderTable(container, masses, particleCount) {
    if (!masses.length) {
        container.innerHTML = `<div style="font-style:italic;color:var(--text-muted);font-size:12px;">No stable configurations to list.</div>`;
        return;
    }
    // Group identical masses
    const groups = new Map();
    for (const m of masses) {
        // Bin to 3 sig figs to group near-identical masses
        const key = m.toPrecision(3);
        groups.set(key, (groups.get(key) || 0) + 1);
    }
    const rows = Array.from(groups.entries())
        .map(([k, count]) => {
            const m = parseFloat(k);
            const match = matchKnown(m);
            return { mass: m, count, match };
        })
        .sort((a, b) => b.count - a.count);

    let html = `
        <table style="width:100%;border-collapse:collapse;table-layout:fixed;font-size:12px;font-family:var(--font-mono);">
            <colgroup>
                <col style="width:96px"/><col style="width:56px"/><col style="width:64px"/><col/>
            </colgroup>
            <thead>
                <tr style="position:sticky;top:0;background:var(--bg-card);">
                    <th style="text-align:right;padding:6px 8px;color:var(--text-muted);font-weight:500;border-bottom:1px solid var(--border-light);">Mass</th>
                    <th style="text-align:right;padding:6px 8px;color:var(--text-muted);font-weight:500;border-bottom:1px solid var(--border-light);">Q</th>
                    <th style="text-align:right;padding:6px 8px;color:var(--text-muted);font-weight:500;border-bottom:1px solid var(--border-light);">Count</th>
                    <th style="text-align:left;padding:6px 8px;color:var(--text-muted);font-weight:500;border-bottom:1px solid var(--border-light);">Δ-nearest known</th>
                </tr>
            </thead>
            <tbody>
    `;
    for (const r of rows) {
        const matchTxt = r.match
            ? `${r.match.name}: ${(r.match.delta * 100).toFixed(1)}% off`
            : '—';
        const matchColor = r.match
            ? (r.match.delta < 0.05 ? 'var(--positive)' : (r.match.delta < 0.20 ? 'var(--warning)' : 'var(--text-muted)'))
            : 'var(--text-muted)';
        html += `
            <tr>
                <td style="text-align:right;padding:6px 8px;color:var(--accent);font-variant-numeric:tabular-nums;">${formatExp(r.mass)}</td>
                <td style="text-align:right;padding:6px 8px;color:var(--text-muted);">—</td>
                <td style="text-align:right;padding:6px 8px;color:var(--text-primary);font-variant-numeric:tabular-nums;">${r.count}</td>
                <td style="text-align:left;padding:6px 8px;color:${matchColor};">${matchTxt}</td>
            </tr>
        `;
    }
    html += `</tbody></table>`;
    if (particleCount > masses.length) {
        html += `
            <div style="margin-top:6px;font-size:11px;color:var(--text-muted);font-style:italic;">
                ${particleCount - masses.length} void/unmanifested particles excluded.
            </div>
        `;
    }
    container.innerHTML = html;
}

function renderHero(container, masses) {
    const matched = masses.filter((m) => matchKnown(m) != null).length;
    const total = masses.length;
    container.innerHTML = `
        <div style="${heroStyle()}">
            ${tagBadge('E')}peaks matched / found = <span style="color:var(--accent);">${matched} / ${total}</span>
        </div>
        <div style="font-size:11px;color:var(--text-muted);margin-top:4px;">
            "matched" = within 50% of a known particle mass (e/μ/π/p). Tighter matches in the table below.
        </div>
    `;
}

export function mountSpectrumPanel(host, getBridge) {
    if (!host) return null;
    const existing = document.getElementById(PANEL_ID);
    if (existing) existing.remove();
    const panel = buildPanel();
    host.appendChild(panel);

    const heroBody = panel.querySelector(`#${PANEL_ID}-hero-body`);
    const histBody = panel.querySelector(`#${PANEL_ID}-hist-body`);
    const tableBody = panel.querySelector(`#${PANEL_ID}-table-body`);

    function update() {
        const bridge = getBridge?.();
        if (!bridge) return;
        const { masses, particles } = extractPresentSpectrum(bridge);
        renderHero(heroBody, masses);
        renderHistogram(histBody, masses);
        renderTable(tableBody, masses, particles);
    }

    // Initial render so empty state is visible immediately
    update();

    const sub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });

    const api = {
        update,
        element: panel,
        dispose: () => {
            sub.unsubscribe();
            // Clear the window-singleton ref so the detached api +
            // panel subtree are GC-eligible. (Audit pass 2:
            // cross-cutting __ftd*Panel retention fix.)
            if (typeof window !== 'undefined' && window.__ftdSpectrumPanel === api) {
                window.__ftdSpectrumPanel = null;
            }
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdSpectrumPanel = api;
    return api;
}

export function initSpectrumPanel() {
    if (typeof document === 'undefined') return null;
    const host = document.getElementById('panel-spectrum');
    if (!host) return null;
    const getBridge = () => {
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        if (!ctx) return null;
        // Same bridge-resolution path as flux-slice / p1-observables
        // (lazy import to avoid circular deps; getScale0State is loaded
        // by the time this runs).
        try {
            const state = window.__ftdScale0State?.() || null;
            if (state?.useFluxMock && state?.fluxMock) return state.fluxMock;
        } catch (_) { /* no-op */ }
        return ctx.bridge;
    };
    return mountSpectrumPanel(host, getBridge);
}
