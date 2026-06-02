/**
 * Conservation-law audit micropanel.
 *
 * Always-on small overlay at the top-LEFT of the lattice viewport (top-right
 * is occupied by viewport-overlay + symmetry-panel — Auditor #1).
 *
 * Shows ΔE, Δp, ΔL, ΔQ over a rolling 100-tick window. Each value is
 * color-coded with hysteresis to prevent 4-Hz flicker. Clicking the panel
 * opens a fullscreen modal with full sparkline history.
 *
 * Uses bridge.getDiagnostics() (totalEnergy, chargeBalance, angMomX/Y/Z)
 * and bridge.getEnergyAudit() (totalPoynting → field momentum) — no
 * engine changes required.
 */

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import {
    formatExp,
    statusToken,
    createHysteresis,
} from './_card-helpers.js';
import { attachFullscreen } from '../../../../ui/charts/chart-fullscreen.js';
import { getPhysicsHarness } from '../../../../physics/index.js';

const PANEL_ID = 'conservation-micropanel';
const HZ = 4;                                // sample rate
const WINDOW_TICKS = 1000;                   // rolling window for Δ display + history (20+ s @ 50tps)
const HEADLINE_WINDOW_TICKS = 100;           // shorter window for the headline rows
const SPARK_SAMPLES = 60;                    // sparkline history length (compact view)

// Helper: read totals via the PhysicsHarness — single canonical source.
// Dropped the inline diag+audit aggregation in favor of harness's
// getConservationTotals() so any future change to "what counts as a
// conservation quantity" lives in ONE place (the harness), not here
// AND in p1 panel AND in spectrum panel.
function sampleTotals(bridge) {
    const harness = getPhysicsHarness(bridge);
    return harness ? harness.getConservationTotals() : null;
}

function l2(x, y, z) {
    return Math.sqrt(x * x + y * y + z * z);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    // chart-card class enables shared fullscreen portal styling
    root.className = 'scale0-only conservation-micropanel chart-card';
    root.style.cssText = `
        position: absolute;
        top: 80px;                 /* clears the toolbar */
        left: 12px;
        width: min(220px, calc(100vw - 20px));
        min-height: 156px;
        z-index: 60;
        padding: 10px 12px;
        background: rgba(8, 12, 20, 0.88);
        border: 1px solid var(--border-light, rgba(255,255,255,0.10));
        border-radius: 6px;
        backdrop-filter: blur(6px);
        font-family: var(--font-sans, system-ui, -apple-system, "Segoe UI", sans-serif);
        font-size: 12px;
        color: var(--text-primary);
        user-select: none;
        transition: background 180ms ease;
    `;
    root.innerHTML = `
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px;gap:6px;">
            <span style="font-weight:600;color:var(--text-muted);text-transform:uppercase;letter-spacing:0.04em;font-size:11px;flex:1;">CONSERVATION</span>
            <span id="${PANEL_ID}-status" style="font-size:10px;color:var(--text-muted);font-family:var(--font-mono);">live</span>
            <button class="chart-card-expand" type="button" title="Expand to fullscreen history"
                aria-label="Expand conservation history"
                style="background:none;border:none;color:var(--text-secondary);cursor:pointer;font-size:14px;padding:0 2px;line-height:1;">⛶</button>
        </div>
        <div id="${PANEL_ID}-rows" style="display:grid;grid-template-columns:24px 1fr 12px;gap:4px 6px;align-items:center;font-variant-numeric:tabular-nums;"></div>
        <div id="${PANEL_ID}-history" class="chart-card-plot" style="display:none;margin-top:14px;"></div>
        <div style="margin-top:6px;font-size:10px;color:var(--text-muted);opacity:0.7;text-align:center;">
            Δ over ${HEADLINE_WINDOW_TICKS} ticks · click ⛶ for full ${WINDOW_TICKS}-tick history
        </div>
    `;
    return root;
}

function renderRow(label, value, color) {
    return `
        <span style="color:var(--text-muted);font-family:var(--font-mono);font-size:11px;">${label}</span>
        <span style="font-family:var(--font-mono);font-size:12px;text-align:right;color:${color};white-space:pre;">${formatExp(value)}</span>
        <span style="display:inline-block;width:8px;height:8px;border-radius:50%;background:${color};box-shadow:inset 0 0 0 1px rgba(0,0,0,0.4);"></span>
    `;
}

export function mountConservationMicropanel(host, getBridge) {
    if (!host) return null;
    const existing = document.getElementById(PANEL_ID);
    if (existing) existing.remove();

    const panel = buildPanel();
    host.appendChild(panel);

    const rowsEl = panel.querySelector(`#${PANEL_ID}-rows`);
    const statusEl = panel.querySelector(`#${PANEL_ID}-status`);
    const historyEl = panel.querySelector(`#${PANEL_ID}-history`);

    // Per-quantity hysteresis state.
    const hyst = {
        E:  createHysteresis(),
        p:  createHysteresis(),
        L:  createHysteresis(),
        Q:  createHysteresis(),
    };

    // Rolling-window baseline: keep totals from WINDOW_TICKS ago for the
    // full history (used by the fullscreen modal); compute headline deltas
    // against the most-recent HEADLINE_WINDOW_TICKS samples for the dock view.
    const history = [];                     // [{tick, totals}, ...]
    let lastBridge = null;

    /** Locate the nearest history entry that's at least lookback ticks behind. */
    function findBaseline(currentTick, lookback) {
        for (let i = 0; i < history.length; i++) {
            if (currentTick - history[i].tick <= lookback) return history[i];
        }
        return history[0];
    }

    /** Render the 4 SVG sparklines used in the fullscreen modal. */
    function renderHistorySparklines() {
        if (!history.length) {
            historyEl.innerHTML = '<div style="color:var(--text-muted);font-style:italic;font-size:12px;text-align:center;padding:20px;">Collecting history…</div>';
            return;
        }
        const tBase = history[0].tick;
        const series = [
            { label: 'ΔE', extract: (h) => h.totals.E - history[0].totals.E },
            { label: 'Δp', extract: (h) => l2(h.totals.px - history[0].totals.px, h.totals.py - history[0].totals.py, h.totals.pz - history[0].totals.pz) },
            { label: 'ΔL', extract: (h) => l2(h.totals.Lx - history[0].totals.Lx, h.totals.Ly - history[0].totals.Ly, h.totals.Lz - history[0].totals.Lz) },
            { label: 'ΔQ', extract: (h) => h.totals.Q - history[0].totals.Q },
        ];
        const W = 720, ROW_H = 90;
        let html = `<div style="font-family:var(--font-mono);font-size:12px;color:var(--text-muted);">`;
        html += `<div style="margin-bottom:8px;color:var(--text-primary);font-size:14px;">Conservation drift — last ${history[history.length-1].tick - tBase} ticks (window ${WINDOW_TICKS})</div>`;
        for (const s of series) {
            const values = history.map(s.extract);
            const absVals = values.map(Math.abs);
            const peakAbs = Math.max(...absVals, 1e-30);
            const status = statusToken(peakAbs);
            const margin = { left: 90, right: 60, top: 14, bottom: 18 };
            const innerW = W - margin.left - margin.right;
            const innerH = ROW_H - margin.top - margin.bottom;
            const minV = Math.min(...values);
            const maxV = Math.max(...values);
            const span = (maxV - minV) || (peakAbs * 2 || 1e-12);
            let path = '';
            for (let i = 0; i < values.length; i++) {
                const fx = i / Math.max(1, values.length - 1);
                const fy = 1 - (values[i] - minV) / span;
                const x = (margin.left + fx * innerW).toFixed(1);
                const y = (margin.top + fy * innerH).toFixed(1);
                path += (i === 0 ? 'M' : 'L') + x + ',' + y;
            }
            html += `<svg viewBox="0 0 ${W} ${ROW_H}" style="width:100%;height:auto;display:block;margin-bottom:6px;">`;
            html += `<rect x="${margin.left}" y="${margin.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light, rgba(255,255,255,0.08))" stroke-width="0.5"/>`;
            html += `<text x="8" y="${margin.top + innerH/2 + 4}" fill="var(--text-muted)" font-size="13" font-weight="600">${s.label}</text>`;
            html += `<text x="${margin.left + innerW + 8}" y="${margin.top + 8}" fill="${status}" font-size="11" font-family="var(--font-mono)">${formatExp(peakAbs)}</text>`;
            html += `<text x="${margin.left + innerW + 8}" y="${margin.top + innerH}" fill="var(--text-muted)" font-size="9" opacity="0.7">peak |Δ|</text>`;
            html += `<path d="${path}" stroke="${status}" stroke-width="1.2" fill="none"/>`;
            html += `</svg>`;
        }
        html += `</div>`;
        historyEl.innerHTML = html;
    }

    /** Show / hide the history sparklines block based on fullscreen state. */
    function syncFullscreenView() {
        const fs = panel._ftdCard?._isFullscreen;
        historyEl.style.display = fs ? 'block' : 'none';
        if (fs) renderHistorySparklines();
    }

    function update() {
        const bridge = getBridge?.();
        if (!bridge) return;
        const totals = sampleTotals(bridge);
        if (!totals) return;

        // Reset history if bridge identity changed (scale switch / scenario reload).
        if (bridge !== lastBridge) {
            history.length = 0;
            lastBridge = bridge;
        }

        // Append + trim to window
        history.push({ tick: totals.tick, totals });
        // Drop entries older than WINDOW_TICKS
        while (history.length > 0 && (totals.tick - history[0].tick) > WINDOW_TICKS) {
            history.shift();
        }

        // Compute headline deltas vs HEADLINE_WINDOW_TICKS-ago snapshot
        const start = findBaseline(totals.tick, HEADLINE_WINDOW_TICKS).totals;
        const dE = totals.E - start.E;
        const dp = l2(totals.px - start.px, totals.py - start.py, totals.pz - start.pz);
        const dL = l2(totals.Lx - start.Lx, totals.Ly - start.Ly, totals.Lz - start.Lz);
        const dQ = totals.Q - start.Q;

        const colE = hyst.E.update(statusToken(dE));
        const colP = hyst.p.update(statusToken(dp));
        const colL = hyst.L.update(statusToken(dL));
        const colQ = hyst.Q.update(statusToken(dQ));

        rowsEl.innerHTML = [
            renderRow('ΔE', dE, colE),
            renderRow('Δp', dp, colP),
            renderRow('ΔL', dL, colL),
            renderRow('ΔQ', dQ, colQ),
        ].join('');

        statusEl.textContent = `t=${totals.tick}`;

        // If currently fullscreen, refresh the modal sparklines too
        if (panel._ftdCard?._isFullscreen) {
            renderHistorySparklines();
        }
    }

    // Subscribe to the shared rAF coordinator
    const sub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });

    // Wire fullscreen portal — chart-card-expand button is in the header.
    // attachFullscreen handles the click, Esc, backdrop, and DOM portaling.
    attachFullscreen(panel);

    // Wrap _enterFullscreen / _exitFullscreen to sync the history view.
    if (panel._ftdCard) {
        const origEnter = panel._ftdCard._enterFullscreen.bind(panel._ftdCard);
        const origExit  = panel._ftdCard._exitFullscreen.bind(panel._ftdCard);
        panel._ftdCard._enterFullscreen = function () {
            origEnter();
            syncFullscreenView();
        };
        panel._ftdCard._exitFullscreen = function () {
            origExit();
            syncFullscreenView();
        };
    }

    // Hover affordance
    panel.addEventListener('mouseenter', () => {
        panel.style.background = 'rgba(8, 12, 20, 0.95)';
    });
    panel.addEventListener('mouseleave', () => {
        panel.style.background = 'rgba(8, 12, 20, 0.88)';
    });

    const api = {
        update,
        element: panel,
        dispose: () => {
            sub.unsubscribe();
            // Clear the window-singleton ref so the detached api +
            // panel subtree are GC-eligible. (Audit pass 2:
            // cross-cutting __ftd*Panel retention fix.)
            if (typeof window !== 'undefined' && window.__ftdConservationPanel === api) {
                window.__ftdConservationPanel = null;
            }
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdConservationPanel = api;
    return api;
}

/**
 * Side-panel-tab init function (mirrors initFluxSlicePanel pattern).
 * Mounts the conservation micropanel into the dashboard's app element.
 * Idempotent: re-calling re-attaches with a fresh bridge accessor.
 */
export function initConservationMicropanel() {
    if (typeof document === 'undefined') return null;
    const host = document.getElementById('app');
    if (!host) return null;
    // Bridge accessor: read from window.__ftdCtx.bridge (reads live state)
    const getBridge = () => {
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        return ctx?.bridge || null;
    };
    if (typeof window !== 'undefined' && window.__ftdConservationPanel) {
        return window.__ftdConservationPanel;
    }
    return mountConservationMicropanel(host, getBridge);
}
