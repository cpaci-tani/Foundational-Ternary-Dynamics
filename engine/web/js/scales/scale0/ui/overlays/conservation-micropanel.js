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
 * Uses PhysicsHarness.getConservationTotals() (hub-first diag/audit) —
 * no direct bridge polling.
 */

import { BaseComponent } from '../../../../core/component.js';
import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import {
    formatExp,
    statusToken,
    createHysteresis,
} from './_card-helpers.js';
import { attachFullscreen } from '../../../../ui/charts/chart-fullscreen.js';
import { getPhysicsHarness } from '../../../../physics/index.js';
import { resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { telemetryHub } from '../../../../telemetry-hub.js';

const PANEL_ID = 'conservation-micropanel';
const HZ = 4;                                // sample rate
const WINDOW_TICKS = 1000;                   // rolling window for Δ display + history (20+ s @ 50tps)
const HEADLINE_WINDOW_TICKS = 100;           // shorter window for the headline rows

// Helper: read totals via the PhysicsHarness — single canonical source.
// Dropped the inline diag+audit aggregation in favor of harness's
// getConservationTotals() so any future change to "what counts as a
// conservation quantity" lives in ONE place (the harness), not here
// AND in p1 panel AND in spectrum panel.
function sampleTotals(bridge) {
    const harness = getPhysicsHarness(bridge);
    return harness ? harness.getConservationTotals() : null;
}

function groupSourceBoundary(meta) {
    if (!meta) return null;
    const epoch = meta.sourceEpoch ?? meta.epoch;
    if (epoch === null || epoch === undefined || epoch === '') return null;
    return `${meta.source ?? 'unknown'}:${String(epoch)}`;
}

/** Scientific-history boundary shared by the panel and focused tests. */
export function getConservationSourceBoundary(hub = telemetryHub) {
    const expected = hub?.s0?.meta?.expectedSourceEpoch;
    if (expected !== null && expected !== undefined && expected !== '') {
        return `${hub?.s0?.meta?.expectedSource ?? 'unknown'}:${String(expected)}`;
    }
    const diagnostics = hub?.getScale0TelemetryMeta?.('diagnostics') ?? null;
    const audit = hub?.getScale0TelemetryMeta?.('audit') ?? null;
    const diagBoundary = groupSourceBoundary(diagnostics);
    const auditBoundary = groupSourceBoundary(audit);
    if (diagBoundary === null && auditBoundary === null) return null;
    return `diagnostics=${diagBoundary ?? 'none'}|audit=${auditBoundary ?? 'none'}`;
}

function l2(x, y, z) {
    return Math.sqrt(x * x + y * y + z * z);
}

const TEMPLATE = `
    <div id="conservation-micropanel" class="scale0-only conservation-micropanel chart-card">
        <div class="cons-header">
            <span class="cons-title">CONSERVATION</span>
            <span id="conservation-micropanel-status" ref="status" class="cons-status">live</span>
            <button class="chart-card-expand cons-btn-expand" type="button" title="Expand to fullscreen history"
                aria-label="Expand conservation history">⛶</button>
        </div>
        <div ref="rows" class="cons-rows-grid"></div>
        <div ref="history" class="chart-card-plot cons-history-plot"></div>
        <div class="cons-footer">
            Δ over ${HEADLINE_WINDOW_TICKS} ticks · click ⛶ for full ${WINDOW_TICKS}-tick history
        </div>
    </div>
`;

export class ConservationMicropanelComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
    }
}

function renderRow(label, value, color, { missing = false, key = '', reason = '' } = {}) {
    const val = missing ? '        —' : formatExp(value);
    const title = missing
        ? (reason || 'Collecting a current source snapshot and the full lookback window.')
        : '';
    const titleAttr = title ? ` title="${title}"` : '';
    const keyAttr = key ? ` data-cons="${key}"` : '';
    const valKeyAttr = key ? ` data-cons-val="${key}"` : '';
    return `
        <span class="cons-row-label"${keyAttr}${titleAttr}>${label}</span>
        <span class="cons-row-val"${valKeyAttr} style="color:${color};">${val}</span>
        <span class="cons-row-dot" style="background:${color};"></span>
    `;
}

export function mountConservationMicropanel(host, getBridge, hub = telemetryHub) {
    if (!host) return null;
    const existing = document.getElementById(PANEL_ID);
    if (existing) existing.remove();

    const comp = new ConservationMicropanelComponent();
    comp.mount(host);
    const panel = comp.element;

    const rowsEl = comp.refs.rows;
    const statusEl = comp.refs.status;
    const historyEl = comp.refs.history;

    // Per-quantity hysteresis state.
    const hyst = {
        E:  createHysteresis(),
        p:  createHysteresis(),
        L:  createHysteresis(),
        Q:  createHysteresis(),
    };

    // Diagnostics and audit reductions have independent clocks. Never append a
    // retained audit reduction under a newer diagnostics tick: each quantity's
    // history is keyed by its own source observation.
    const diagnosticHistory = [];           // [{tick, totals, stamp}, ...]
    const energyHistory = [];               // [{tick, value, stamp}, ...]
    const momentumHistory = [];             // [{tick, x, y, z, stamp}, ...]
    let lastBridge = null;
    let lastResetVersion = -1;
    let lastSourceBoundary = null;
    let lastDiagnosticStamp = null;
    let lastEnergyStamp = null;
    let lastMomentumStamp = null;
    let lastRenderState = '';

    function renderWaiting(status = 'waiting') {
        if (lastRenderState === status) return;
        lastRenderState = status;
        const muted = 'var(--text-muted)';
        rowsEl.innerHTML = [
            renderRow('ΔE', Number.NaN, muted, { key: 'E', missing: true }),
            renderRow('Δp', Number.NaN, muted, { key: 'p', missing: true }),
            renderRow('ΔL', Number.NaN, muted, { key: 'L', missing: true }),
            renderRow('ΔQ', Number.NaN, muted, { key: 'Q', missing: true }),
        ].join('');
        statusEl.textContent = status;
        if (panel._ftdCard?._isFullscreen) renderHistorySparklines();
    }

    function resetHistory(status = 'collecting') {
        diagnosticHistory.length = 0;
        energyHistory.length = 0;
        momentumHistory.length = 0;
        lastDiagnosticStamp = null;
        lastEnergyStamp = null;
        lastMomentumStamp = null;
        renderWaiting(status);
    }

    /** Locate the nearest history entry that's at least lookback ticks behind. */
    function findBaseline(history, currentTick, lookback) {
        const targetTick = currentTick - lookback;
        let baseline = null;
        for (const entry of history) {
            if (entry.tick > targetTick) break;
            baseline = entry;
        }
        return baseline;
    }

    function trimHistory(history, currentTick) {
        while (history.length > 0 && currentTick - history[0].tick > WINDOW_TICKS) {
            history.shift();
        }
    }

    /** Render the 4 SVG sparklines used in the fullscreen modal. */
    function renderHistorySparklines() {
        const allHistories = [energyHistory, momentumHistory, diagnosticHistory];
        if (!allHistories.some(history => history.length)) {
            historyEl.innerHTML = '<div class="cons-history-empty">Collecting history…</div>';
            return;
        }
        const series = [
            { label: 'ΔE', history: energyHistory,
              extract: (h, first) => h.value - first.value },
            { label: 'Δp', history: momentumHistory,
              extract: (h, first) => l2(h.x - first.x, h.y - first.y, h.z - first.z) },
            { label: 'ΔL', history: diagnosticHistory,
              extract: (h, first) => h.totals.LAvailable && first.totals.LAvailable
                  ? l2(h.totals.Lx - first.totals.Lx, h.totals.Ly - first.totals.Ly, h.totals.Lz - first.totals.Lz)
                  : Number.NaN },
            { label: 'ΔQ', history: diagnosticHistory,
              extract: (h, first) => h.totals.QAvailable && first.totals.QAvailable
                  ? h.totals.Q - first.totals.Q : Number.NaN },
        ];
        const firstTicks = allHistories.filter(history => history.length).map(history => history[0].tick);
        const lastTicks = allHistories.filter(history => history.length).map(history => history.at(-1).tick);
        const tBase = Math.min(...firstTicks);
        const tLast = Math.max(...lastTicks);
        const W = 720, ROW_H = 90;
        let html = `<div class="cons-history-container">`;
        html += `<div class="cons-history-title">Conservation drift — last ${tLast - tBase} ticks (window ${WINDOW_TICKS})</div>`;
        for (const s of series) {
            const first = s.history[0] ?? null;
            const values = first ? s.history.map(entry => s.extract(entry, first)) : [];
            const finiteValues = values.filter(Number.isFinite);
            const peakAbs = finiteValues.reduce(
                (peak, value) => Math.max(peak, Math.abs(value)), 1e-30,
            );
            const status = finiteValues.length ? statusToken(peakAbs) : 'var(--text-muted)';
            const margin = { left: 90, right: 60, top: 14, bottom: 18 };
            const innerW = W - margin.left - margin.right;
            const innerH = ROW_H - margin.top - margin.bottom;
            const minV = finiteValues.length ? Math.min(...finiteValues) : 0;
            const maxV = finiteValues.length ? Math.max(...finiteValues) : 0;
            const span = (maxV - minV) || (peakAbs * 2 || 1e-12);
            let path = '';
            for (let i = 0; i < values.length; i++) {
                const fx = i / Math.max(1, values.length - 1);
                if (!Number.isFinite(values[i])) { path += ' '; continue; }
                const fy = 1 - (values[i] - minV) / span;
                const x = (margin.left + fx * innerW).toFixed(1);
                const y = (margin.top + fy * innerH).toFixed(1);
                const previousFinite = i > 0 && Number.isFinite(values[i - 1]);
                path += (previousFinite ? 'L' : 'M') + x + ',' + y;
            }
            html += `<svg viewBox="0 0 ${W} ${ROW_H}" class="cons-history-svg">`;
            html += `<rect x="${margin.left}" y="${margin.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light, rgba(255,255,255,0.08))" stroke-width="0.5"/>`;
            html += `<text x="8" y="${margin.top + innerH/2 + 4}" fill="var(--text-muted)" font-size="16" font-weight="600">${s.label}</text>`;
            const peakLabel = finiteValues.length ? formatExp(peakAbs) : '—';
            html += `<text x="${margin.left + innerW + 8}" y="${margin.top + 8}" fill="${status}" font-size="16" font-family="var(--font-mono)">${peakLabel}</text>`;
            html += `<text x="${margin.left + innerW + 8}" y="${margin.top + innerH}" fill="var(--text-muted)" font-size="16" opacity="0.7">peak |Δ|</text>`;
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

    /**
     * Is this micropanel actually rendered right now?
     *
     * It is an inline `.chart-card` mounted into `#app`, NOT a dock tab and not
     * (normally) a floating window, so `isPanelLive(host)` could never return
     * true: `#app` carries neither `.active` nor a `.floating-window` ancestor,
     * and the shared predicate requires one of those. The guard therefore closed
     * permanently and the DeltaE/Deltap/DeltaL/DeltaQ grid was never populated —
     * the status element kept its literal template string and the fullscreen view
     * showed "Collecting history..." forever.
     *
     * Visibility here is CSS-driven by the `scale0-only` class, so the honest
     * test is "does the panel's OWN root render". If it has been floated, defer
     * to the shared predicate so a collapsed window still stops the work.
     */
    function panelIsLive() {
        const el = document.getElementById(PANEL_ID);
        if (!el) return false;
        if (el.closest('.floating-window')) return isPanelLive(el);
        return el.getClientRects().length > 0;
    }

    function update() {
        if (!panelIsLive()) return;
        const bridge = getBridge?.();
        if (!bridge) return;

        // Reset before sampling on bridge, scenario/reset, or telemetry-source
        // turnover. Native mutation invalidation advances sourceEpoch without
        // replacing the bridge or calling resetScale(0); preserving the old
        // conservation baseline across that intervention would turn a deliberate
        // write into apparent drift.
        const resetVersion = hub.getResetVersion?.(0) ?? 0;
        const sourceBoundary = getConservationSourceBoundary(hub);
        const sourceChanged = lastSourceBoundary !== null && sourceBoundary !== null
            && sourceBoundary !== lastSourceBoundary;
        if (bridge !== lastBridge || resetVersion !== lastResetVersion || sourceChanged) {
            resetHistory(sourceChanged ? 'source changed · collecting' : 'collecting');
            lastBridge = bridge;
            lastResetVersion = resetVersion;
        }
        if (sourceBoundary !== null) lastSourceBoundary = sourceBoundary;

        const diagMeta = hub.getScale0TelemetryMeta?.('diagnostics') ?? null;
        if (!diagMeta || diagMeta.stale === true || !Number.isFinite(diagMeta.tick)) {
            renderWaiting('waiting');
            return;
        }
        const totals = sampleTotals(bridge);
        if (!totals || !Number.isFinite(totals.tick)) {
            renderWaiting('waiting');
            return;
        }

        const boundary = sourceBoundary ?? 'local';
        const diagnosticsObservation = totals.diagnosticsObservation;
        const energyObservation = totals.energyObservation;
        const momentumObservation = totals.momentumObservation;
        const diagnosticStamp = diagnosticsObservation?.stamp
            ? `${boundary}|${diagnosticsObservation.stamp}` : null;
        const energyStamp = energyObservation?.available && energyObservation.stamp
            ? `${boundary}|${energyObservation.stamp}` : null;
        const momentumStamp = momentumObservation?.available && momentumObservation.stamp
            ? `${boundary}|${momentumObservation.stamp}` : null;

        // Each history follows its producer's observation identity. A worker
        // may publish diagnostics eight times while reusing one audit reduction;
        // that is one energy/momentum sample, not eight samples at newer ticks.
        if (diagnosticStamp && diagnosticStamp !== lastDiagnosticStamp) {
            lastDiagnosticStamp = diagnosticStamp;
            diagnosticHistory.push({ tick: totals.tick, totals, stamp: diagnosticStamp });
            trimHistory(diagnosticHistory, totals.tick);
        }
        if (energyStamp && energyStamp !== lastEnergyStamp) {
            lastEnergyStamp = energyStamp;
            energyHistory.push({
                tick: energyObservation.sampleTick,
                value: energyObservation.value,
                stamp: energyStamp,
            });
            trimHistory(energyHistory, energyObservation.sampleTick);
        }
        if (momentumStamp && momentumStamp !== lastMomentumStamp) {
            lastMomentumStamp = momentumStamp;
            momentumHistory.push({
                tick: momentumObservation.sampleTick,
                x: momentumObservation.x,
                y: momentumObservation.y,
                z: momentumObservation.z,
                stamp: momentumStamp,
            });
            trimHistory(momentumHistory, momentumObservation.sampleTick);
        }

        const renderStamp = [diagnosticStamp, energyStamp ?? 'energy-waiting',
            momentumStamp ?? 'momentum-waiting'].join('|');
        if (renderStamp === lastRenderState) return;

        // Compute each headline against a baseline from the same producer.
        const energyBase = energyObservation?.available
            ? findBaseline(energyHistory, energyObservation.sampleTick, HEADLINE_WINDOW_TICKS)
            : null;
        const momentumBase = momentumObservation?.available
            ? findBaseline(momentumHistory, momentumObservation.sampleTick, HEADLINE_WINDOW_TICKS)
            : null;
        const diagnosticBaseEntry = findBaseline(
            diagnosticHistory, totals.tick, HEADLINE_WINDOW_TICKS,
        );
        const diagnosticBase = diagnosticBaseEntry?.totals ?? null;
        const eLive = !!energyBase && energyObservation?.available;
        const dE = eLive ? energyObservation.value - energyBase.value : Number.NaN;
        const pLive = !!momentumBase && momentumObservation?.available;
        const dp = pLive
            ? l2(momentumObservation.x - momentumBase.x,
                momentumObservation.y - momentumBase.y,
                momentumObservation.z - momentumBase.z)
            : Number.NaN;
        const lLive = !!diagnosticBase && totals.LAvailable && diagnosticBase.LAvailable;
        const qLive = !!diagnosticBase && totals.QAvailable && diagnosticBase.QAvailable;
        const dL = lLive
            ? l2(totals.Lx - diagnosticBase.Lx, totals.Ly - diagnosticBase.Ly,
                totals.Lz - diagnosticBase.Lz)
            : Number.NaN;
        const dQ = qLive ? totals.Q - diagnosticBase.Q : Number.NaN;

        const colE = eLive ? hyst.E.update(statusToken(dE)) : 'var(--text-muted)';
        const colP = pLive ? hyst.p.update(statusToken(dp)) : 'var(--text-muted)';
        const colL = lLive ? hyst.L.update(statusToken(dL)) : 'var(--text-muted)';
        const colQ = qLive ? hyst.Q.update(statusToken(dQ)) : 'var(--text-muted)';

        rowsEl.innerHTML = [
            renderRow('ΔE', dE, colE, { key: 'E', missing: !eLive,
                reason: energyObservation?.available
                    ? 'Collecting the full 100-tick audit-energy lookback.'
                    : 'Dynamic energy needs a current energy-audit snapshot.' }),
            renderRow('Δp', dp, colP, { key: 'p', missing: !pLive,
                reason: momentumObservation?.available
                    ? 'Collecting the full 100-tick Poynting lookback.'
                    : 'Poynting needs a current energy-audit snapshot.' }),
            renderRow('ΔL', dL, colL, { key: 'L', missing: !lLive }),
            renderRow('ΔQ', dQ, colQ, { key: 'Q', missing: !qLive }),
        ].join('');

        const energyClock = energyObservation?.available
            ? `t=${energyObservation.sampleTick}` : 'waiting';
        const momentumClock = momentumObservation?.available
            ? `t=${momentumObservation.sampleTick}` : 'waiting';
        statusEl.textContent = `state t=${totals.tick} · E ${energyClock} · p ${momentumClock}`;
        lastRenderState = renderStamp;

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

    // Hover affordance handled by CSS :hover


    const api = {
        update,
        element: panel,
        get historyLength() { return diagnosticHistory.length; },
        get diagnosticHistoryLength() { return diagnosticHistory.length; },
        get energyHistoryLength() { return energyHistory.length; },
        get momentumHistoryLength() { return momentumHistory.length; },
        get sourceBoundary() { return lastSourceBoundary; },
        get lastSampleStamp() { return lastDiagnosticStamp; },
        get lastEnergyStamp() { return lastEnergyStamp; },
        get lastMomentumStamp() { return lastMomentumStamp; },
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
    // Bridge accessor: use the active physics owner. flux-* scenarios are
    // ticked by the JS MockBridge/worker, while ctx.bridge can remain an idle
    // WASM bridge; sampling that idle bridge makes the conservation energy look
    // frozen even though the pulse itself is evolving.
    const getBridge = () => resolveActiveScale0BridgeFromWindow();
    if (typeof window !== 'undefined' && window.__ftdConservationPanel) {
        return window.__ftdConservationPanel;
    }
    return mountConservationMicropanel(host, getBridge);
}
