/**
 * Lattice Spectroscopy — Scale-0 field-structure instrument.
 *
 * Characterizes the lattice field itself (NOT emergent particle masses — those
 * live in the Zoo). Four sections:
 *   ① E(k) energy spectrum (hero) — FFT-derived spatial power spectrum of the
 *      flux field J, Parseval-validated against the audit; live (band-limited,
 *      downsampled) + a Deep Measure full-band snapshot.
 *   ② Topology — Gauss violation, defect/monopole proxy, flux-tube count, chirality.
 *   ③ Field metrics + distributions — vorticity/helicity/coherence/Fisher/
 *      Kretschmann/entropy: value + spatial histogram.
 *   ④ Energy partition — E/B/wave/field split, Poynting, drift.
 *
 * Honesty (CLAUDE.md): [M] measured, [D] derived/computed, [≈] approximate
 * (downsampled / band-limited). See SPEC_SCALE0_LATTICE_SPECTROSCOPY.md.
 */

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { cardStyle, titleStyle, heroStyle, tagBadge, formatExp, formatFixed } from './_card-helpers.js';
import {
    energySpectrum, spectralPeak, spectralSlope, denseVectorGridFromSamples,
} from '../../analysis/lattice-spectrum.js';
import {
    defectCount, fluxTubeComponents, metricStats, histogram, chiralityFromAudit,
} from '../../analysis/lattice-topology.js';
import { getScale0State } from '../../state/store.js';

const PANEL_ID = 'spectrum-panel';
const HZ = 2;                 // exploratory data — slower cadence
const M_LIVE = 32;            // live FFT grid (band-limited)
const M_DEEP = 64;            // Deep Measure FFT grid (full band)

const METRIC_KINDS = [
    { kind: 'vorticity',   name: 'Vorticity',   sym: 'ω',  unit: '|∇×J|' },
    { kind: 'helicity',    name: 'Helicity',    sym: 'H',  unit: 'J·∇×J' },
    { kind: 'coherence',   name: 'Coherence',   sym: 'C',  unit: '' },
    { kind: 'fisher',      name: 'Fisher info', sym: 'I',  unit: '' },
    { kind: 'kretschmann', name: 'Kretschmann', sym: 'K',  unit: 'curv' },
];

function liveStride(L) { return Math.max(1, Math.min(6, Math.round(L / 40))); }

// ── Compute ──────────────────────────────────────────────────────────────────

/** Spectrum + the dense magnitude grid (reused for flux-tube topology). */
function computeSpectrum(caps, L, stride, M) {
    const samples = caps.getScale0FieldSamples({ kind: 'fluxVector', stride });
    const grid = denseVectorGridFromSamples(samples, L, stride);
    const spec = energySpectrum(grid, grid.srcN, M, L);
    const peak = spectralPeak(spec.k, spec.E);
    const slope = spectralSlope(spec.k, spec.E);
    const parseval = spec.sumReal > 0 ? spec.totalE / spec.sumReal : 1;
    // magnitude grid for flux-tube CC (on the same srcN grid)
    const Nc = grid.srcN ** 3;
    const mag = new Float64Array(Nc);
    for (let i = 0; i < Nc; i++) mag[i] = Math.hypot(grid.jx[i], grid.jy[i], grid.jz[i]);
    return { spec, peak, slope, parseval, mag, srcN: grid.srcN, stride, M, sampleCount: samples.count };
}

function computeTopology(caps, sp, audit) {
    const dj = caps.getScale0FieldSamples({ kind: 'divJ', stride: sp.stride });
    const defects = defectCount(dj.values || dj.vectors || [], dj.count | 0, 0.5);
    const tubes = fluxTubeComponents(sp.mag, sp.srcN, 0.35);
    const chir = chiralityFromAudit(audit);
    return {
        defects, tubes, chir,
        gauss: audit ? (audit.gaussViolation ?? 0) : 0,
        gaussMax: audit ? (audit.maxGaussError ?? 0) : 0,
    };
}

function computeMetrics(caps, stride) {
    const out = [];
    for (const m of METRIC_KINDS) {
        const s = caps.getScale0FieldSamples({ kind: m.kind, stride });
        const vals = s.values || new Float32Array(0), n = s.count | 0;
        out.push({ ...m, stats: metricStats(vals, n), hist: histogram(vals, n, 22) });
    }
    return out;
}

// ── Render: ① spectrum (log–log E(k)) ────────────────────────────────────────

function renderSpectrum(container, r, isDeep) {
    const { spec, peak, slope, parseval } = r;
    const ks = [], es = [];
    for (let i = 0; i < spec.E.length; i++) { if (spec.E[i] > 0 && spec.k[i] > 0) { ks.push(spec.k[i]); es.push(spec.E[i]); } }
    if (ks.length < 2) {
        container.innerHTML = `<div class="spec-hist-empty">No field energy yet — load/seed a flux scenario.</div>`;
        return;
    }
    const W = 360, H = 188, m = { top: 16, right: 14, bottom: 34, left: 44 };
    const iW = W - m.left - m.right, iH = H - m.top - m.bottom;
    const lkMin = Math.log10(ks[0]), lkMax = Math.log10(ks[ks.length - 1]);
    const leMin = Math.log10(Math.min(...es)), leMax = Math.log10(Math.max(...es));
    const kSpan = (lkMax - lkMin) || 1, eSpan = (leMax - leMin) || 1;
    const X = (k) => m.left + ((Math.log10(k) - lkMin) / kSpan) * iW;
    const Y = (e) => m.top + (1 - (Math.log10(e) - leMin) / eSpan) * iH;

    let svg = `<svg viewBox="0 0 ${W} ${H}" class="spec-svg-plot">`;
    svg += `<rect x="${m.left}" y="${m.top}" width="${iW}" height="${iH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light)" stroke-width="1"/>`;
    // grid decade lines (y)
    for (let e = Math.ceil(leMin); e <= Math.floor(leMax); e++) {
        const yy = Y(Math.pow(10, e));
        svg += `<line x1="${m.left}" y1="${yy.toFixed(1)}" x2="${m.left + iW}" y2="${yy.toFixed(1)}" stroke="var(--border-light)" stroke-width="0.4" opacity="0.4"/>`;
        svg += `<text x="${m.left - 4}" y="${(yy + 3).toFixed(1)}" text-anchor="end" font-size="9" font-family="var(--font-mono)" fill="var(--text-muted)">1e${e}</text>`;
    }
    // peak marker
    if (peak.kPeak > 0) {
        const xp = X(peak.kPeak);
        svg += `<line x1="${xp.toFixed(1)}" y1="${m.top}" x2="${xp.toFixed(1)}" y2="${m.top + iH}" stroke="var(--warning)" stroke-width="0.8" stroke-dasharray="2,3" opacity="0.7"/>`;
        svg += `<text x="${xp.toFixed(1)}" y="${m.top - 3}" text-anchor="middle" font-size="9" fill="var(--warning)">k*</text>`;
    }
    // slope reference line over the inertial range
    if (Number.isFinite(slope.slope)) {
        const k1 = ks[2] || ks[0], k2 = ks[ks.length - 3] || ks[ks.length - 1];
        const eAtK1 = es[ks.indexOf(k1)] ?? es[0];
        const yLine = (k) => Y(eAtK1 * Math.pow(k / k1, slope.slope));
        svg += `<line x1="${X(k1).toFixed(1)}" y1="${yLine(k1).toFixed(1)}" x2="${X(k2).toFixed(1)}" y2="${yLine(k2).toFixed(1)}" stroke="var(--positive)" stroke-width="1" stroke-dasharray="4,2" opacity="0.65"/>`;
    }
    // E(k) polyline + points
    let path = '';
    for (let i = 0; i < ks.length; i++) path += `${i ? 'L' : 'M'}${X(ks[i]).toFixed(1)},${Y(es[i]).toFixed(1)} `;
    svg += `<path d="${path}" fill="none" stroke="var(--accent)" stroke-width="1.4"/>`;
    for (let i = 0; i < ks.length; i++) svg += `<circle cx="${X(ks[i]).toFixed(1)}" cy="${Y(es[i]).toFixed(1)}" r="1.6" fill="var(--accent)"/>`;
    // axis labels
    svg += `<text x="${m.left + iW / 2}" y="${H - 3}" text-anchor="middle" font-size="10" fill="var(--text-muted)">k (rad/voxel) — log</text>`;
    svg += `<text x="11" y="${m.top + iH / 2}" transform="rotate(-90 11 ${m.top + iH / 2})" text-anchor="middle" font-size="10" fill="var(--text-muted)">E(k) — log</text>`;
    svg += `</svg>`;

    const pOk = Math.abs(parseval - 1) < 0.05;
    const lam = Number.isFinite(peak.lambdaPeak) ? peak.lambdaPeak.toFixed(1) : '∞';
    container.innerHTML = `
        ${svg}
        <div class="spec-readouts">
            <span>${tagBadge('D')}peak λ* <b>${lam}</b> vox (k*=${peak.kPeak.toFixed(3)})</span>
            <span>${tagBadge('D')}slope p <b>${Number.isFinite(slope.slope) ? slope.slope.toFixed(2) : '—'}</b></span>
            <span title="ΣE(k) / Σ|J|² — should be 1 if the FFT is correct">${tagBadge('M')}Parseval <b style="color:${pOk ? 'var(--positive)' : 'var(--warning)'}">${parseval.toFixed(3)}</b></span>
            <span>${isDeep ? `${tagBadge('D')}DEEP M=${r.M}³ · k&lt;${spec.kNyq.toFixed(2)}` : `${tagBadge('≈')}live M=${r.M}³ · band-limited k&lt;${spec.kNyq.toFixed(2)}`}</span>
        </div>`;
}

// ── Render: ② topology ───────────────────────────────────────────────────────

function row(label, value, tag = 'D', color = 'var(--text-primary)') {
    return `<div class="spec-row"><span class="spec-row-l">${tagBadge(tag)}${label}</span><span class="spec-row-v" style="color:${color}">${value}</span></div>`;
}

function renderTopology(container, t) {
    const gaugeOk = t.gauss < 1e-4;
    container.innerHTML =
        row('Gauss violation Σ(∇·E−ρ)²', formatExp(t.gauss), 'M', gaugeOk ? 'var(--positive)' : 'var(--warning)') +
        row('  max |Gauss error|', formatExp(t.gaussMax), 'M', 'var(--text-muted)') +
        row('Defects (src / sink / net)', `${t.defects.sources} / ${t.defects.sinks} / ${t.defects.net >= 0 ? '+' : ''}${t.defects.net}`, 'D') +
        row('Flux tubes (count / largest)', `${t.tubes.count} / ${t.tubes.largest}`, 'D') +
        row('Chirality (E asym / wv asym)', `${formatFixed(t.chir.eAsym, 3)} / ${formatFixed(t.chir.wvAsym, 3)}`, 'M');
}

// ── Render: ③ metrics + distributions ────────────────────────────────────────

function miniHist(hist, w = 90, h = 22) {
    const c = hist.counts, n = c.length, mx = Math.max(1, ...c);
    let s = `<svg viewBox="0 0 ${w} ${h}" class="spec-mini-hist">`;
    const bw = w / n;
    for (let i = 0; i < n; i++) {
        const bh = (c[i] / mx) * h;
        s += `<rect x="${(i * bw).toFixed(1)}" y="${(h - bh).toFixed(1)}" width="${(bw - 0.4).toFixed(1)}" height="${bh.toFixed(1)}" fill="var(--accent)" opacity="0.7"/>`;
    }
    return s + `</svg>`;
}

function renderMetrics(container, metrics) {
    let html = '';
    for (const m of metrics) {
        html += `<div class="spec-metric-row">
            <span class="spec-metric-name">${m.sym} <span class="spec-metric-sub">${m.name}</span></span>
            <span class="spec-metric-val">${formatExp(m.stats.rms)}<span class="spec-metric-unit">rms</span></span>
            <span class="spec-metric-hist">${miniHist(m.hist)}</span>
        </div>`;
    }
    container.innerHTML = html || `<div class="spec-hist-empty">No metric data.</div>`;
}

// ── Render: ④ energy partition ───────────────────────────────────────────────

function renderEnergy(container, audit, entropy) {
    if (!audit) { container.innerHTML = `<div class="spec-hist-empty">No audit data.</div>`; return; }
    const parts = [
        { k: 'E-field', v: audit.eFieldEnergy ?? audit.EFieldEnergy ?? 0, c: 'var(--positive)' },
        { k: 'B-field', v: audit.bFieldEnergy ?? audit.BFieldEnergy ?? 0, c: 'var(--negative)' },
        { k: 'Wave',    v: audit.waveEnergy ?? 0, c: 'var(--chart-flux, #fb8c00)' },
        { k: 'Field',   v: audit.fieldEnergy ?? 0, c: 'var(--accent)' },
    ];
    const total = parts.reduce((a, p) => a + Math.max(0, p.v), 0) || 1;
    let bar = `<div class="spec-energy-bar">`;
    for (const p of parts) { const pct = Math.max(0, p.v) / total * 100; bar += `<span style="width:${pct.toFixed(1)}%;background:${p.c}" title="${p.k}: ${formatExp(p.v)}"></span>`; }
    bar += `</div><div class="spec-energy-legend">`;
    for (const p of parts) bar += `<span><i style="background:${p.c}"></i>${p.k} ${formatExp(p.v)}</span>`;
    bar += `</div>`;
    const px = audit.totalPoynting?.x ?? 0, py = audit.totalPoynting?.y ?? 0, pz = audit.totalPoynting?.z ?? 0;
    const pMag = Math.hypot(px, py, pz);
    container.innerHTML = bar +
        row('Poynting |S|', formatExp(pMag), 'M') +
        row('Energy drift', `${formatFixed(audit.energyDrift ?? 0, 3)} %`, 'M') +
        row('Entropy', formatExp(entropy ?? 0), 'M');
}

// ── Panel shell ──────────────────────────────────────────────────────────────

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only spectrum-panel';
    root.innerHTML = `
        <header class="spec-header">
            <span class="spec-title">Lattice Spectroscopy</span>
            <span class="spec-mode" id="${PANEL_ID}-mode">live</span>
        </header>
        <section style="${cardStyle(230)}">
            <div style="${titleStyle()}">Energy spectrum E(k)</div>
            <div id="${PANEL_ID}-spec" class="spec-hist-box"></div>
            <div class="spec-actions">
                <button id="${PANEL_ID}-deep" type="button" class="spec-btn" title="Full-resolution snapshot spectrum + topology of the current tick">Deep Measure</button>
                <button id="${PANEL_ID}-live" type="button" class="spec-btn spec-btn-ghost" hidden title="Resume the live band-limited view">↻ Live</button>
            </div>
        </section>
        <section style="${cardStyle(150)}">
            <div style="${titleStyle()}">Topology</div>
            <div id="${PANEL_ID}-topo"></div>
        </section>
        <section style="${cardStyle(170)}">
            <div style="${titleStyle()}">Field metrics &amp; distributions</div>
            <div id="${PANEL_ID}-metrics"></div>
        </section>
        <section style="${cardStyle(130)}">
            <div style="${titleStyle()}">Energy partition</div>
            <div id="${PANEL_ID}-energy"></div>
        </section>
    `;
    return root;
}

export function mountSpectrumPanel(host, getBridge) {
    if (!host) return null;
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);

    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);
    const specBody = el('spec'), topoBody = el('topo'), metBody = el('metrics'), enBody = el('energy');
    const modeBadge = el('mode'), deepBtn = el('deep'), liveBtn = el('live');

    let mode = 'live';   // 'live' | 'deep' (deep freezes the hero on a full-band snapshot)
    let lastSpec = null; // last computed spectrum (exposed for tests/diagnostics)

    function getCaps() {
        const b = getBridge?.();
        return b?.capabilities?.scale0 || null;
    }

    function renderHero(caps, L, stride, M, isDeep) {
        const sp = computeSpectrum(caps, L, stride, M);
        renderSpectrum(specBody, sp, isDeep);
        lastSpec = sp;
        return sp;
    }

    function update() {
        const caps = getCaps();
        if (!caps) return;
        const L = caps.latticeSize || 33;
        const audit = caps.getScale0EnergyAudit?.() || null;
        const diag = caps.getScale0Diagnostics?.() || null;

        // ① hero — only when live (deep freezes the snapshot)
        let sp;
        if (mode === 'live') {
            sp = renderHero(caps, L, liveStride(L), M_LIVE, false);
        } else {
            // still need a magnitude grid for topology while frozen — cheap live one
            sp = computeSpectrum(caps, L, liveStride(L), M_LIVE);
        }
        // ②③④ always live
        renderTopology(topoBody, computeTopology(caps, sp, audit));
        renderMetrics(metBody, computeMetrics(caps, liveStride(L)));
        renderEnergy(enBody, audit, diag?.entropy);
    }

    deepBtn.addEventListener('click', () => {
        const caps = getCaps();
        if (!caps) return;
        const L = caps.latticeSize || 33;
        mode = 'deep';
        modeBadge.textContent = 'measuring…';
        deepBtn.disabled = true;
        // Let the "measuring…" state paint before the (brief) full-res FFT.
        setTimeout(() => {
            try { renderHero(caps, L, 1, M_DEEP, true); modeBadge.textContent = 'deep (frozen)'; }
            catch (e) { modeBadge.textContent = 'live'; mode = 'live'; console.error('[spectrum] deep measure', e); }
            deepBtn.disabled = false;
            liveBtn.hidden = false;
        }, 30);
    });
    liveBtn.addEventListener('click', () => { mode = 'live'; modeBadge.textContent = 'live'; liveBtn.hidden = true; });

    update();
    const sub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });

    const api = {
        update,
        element: panel,
        get lastSpec() { return lastSpec; },
        deepMeasure: () => deepBtn.click(),
        get mode() { return mode; },
        dispose: () => {
            sub.unsubscribe();
            if (typeof window !== 'undefined' && window.__ftdSpectrumPanel === api) window.__ftdSpectrumPanel = null;
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
    // Resolve the bridge that actually owns the live flux physics: for flux-*
    // scenarios that is the fluxMock (worker/mock), NOT the idle ctx.bridge
    // (same pattern as flux-slice / p1-observables; RF-9). Re-evaluated per call.
    const getBridge = () => {
        const state = getScale0State?.();
        if (state?.useFluxMock && state?.fluxMock) return state.fluxMock;
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        return ctx?.bridge || null;
    };
    return mountSpectrumPanel(host, getBridge);
}
