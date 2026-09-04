// Transaction Tracker panel — live view of the native history-journal drain
// (native-charge gate, opt-in, observation-only; see
// engine/include/ftd/eft/history_event_journal.h and
// engine/web/js/scales/scale0/runtime/transaction-tracker.js).
//
// Structural pattern mirrors knots-panel.js: a plain-DOM tab panel mounted
// into the registry's #panel-transactions host (see
// engine/web/js/ui/scale-registry/panel-registry.js), not a floating
// scenario-scoped overlay like genesis-burst-panel.js — the journal is a
// generic engine capability available on any WASM-owned scenario, not tied
// to one scenario id.

import {
    getScale0State,
    resolveActiveScale0BridgeFromWindow,
} from '../../state/store.js';
import {
    getOrCreateTransactionTracker,
    isTransactionTrackingSupported,
} from '../../runtime/transaction-tracker.js';

const PANEL_ID = 'transaction-panel';
const POLL_MS = 400;

function ensureCss() {
    if (typeof document === 'undefined' || document.getElementById('transaction-panel-css')) return;
    const s = document.createElement('style');
    s.id = 'transaction-panel-css';
    s.textContent = `
    #${PANEL_ID}{font-family:var(--font-sans,sans-serif);font-size:16px;color:var(--text-primary,#eee);padding:2px}
    #${PANEL_ID} .tp-title{font-weight:600;margin:2px 0 6px;font-size:16px}
    #${PANEL_ID} .tp-title small{color:var(--text-muted,#888);font-weight:400;font-size:16px}
    #${PANEL_ID} .tp-ctl{display:flex;align-items:center;gap:6px;cursor:pointer;margin:5px 0 6px;font-size:16px}
    #${PANEL_ID} .tp-ctl input{margin-right:2px}
    #${PANEL_ID} .tp-ctl b{color:var(--text-primary,#eee);font-weight:600}
    #${PANEL_ID} .tp-unavailable{color:var(--text-muted,#888);font-style:italic;font-size:16px;padding:8px 2px;line-height:1.5;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.08))}
    #${PANEL_ID} .tp-grid{display:grid;grid-template-columns:1fr 1fr;gap:4px 12px;font-family:var(--font-mono,monospace);font-size:16px;line-height:1.5;margin:6px 0}
    #${PANEL_ID} .tp-grid .tp-k{color:var(--text-muted,#888)}
    #${PANEL_ID} .tp-grid .tp-v{color:var(--text-primary,#eee);text-align:right}
    #${PANEL_ID} .tp-section-h{margin-top:8px;font-size:16px;letter-spacing:0.04em;color:var(--text-muted,#888);font-weight:600;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.08));padding-top:6px}
    #${PANEL_ID} .tp-note{margin-top:8px;padding-top:6px;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.1));font-size:16px;color:var(--text-muted,#777);line-height:1.5}
    #${PANEL_ID} .tp-note b{color:var(--text-secondary,#999)}
    #${PANEL_ID} button{padding:4px 8px;border-radius:6px;cursor:pointer;font-size:16px}
    `;
    document.head.appendChild(s);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.innerHTML = `
      <div class="tp-title">Transaction Tracker <small>&middot; per-record manifestation lifecycles</small></div>
      <label class="tp-ctl" title="Enable the native history journal (opt-in, observation-only) and reconstruct null orientation / cycles from its drain.">
        <input type="checkbox" id="tp-toggle-journal"> <b>Track manifestation transactions</b>
      </label>
      <div id="tp-unavailable" class="tp-unavailable" hidden></div>
      <div id="tp-live" hidden>
        <div class="tp-section-h">EVENT COUNTS (this session)</div>
        <div class="tp-grid" id="tp-kind-grid"></div>
        <div class="tp-section-h">RECORDS</div>
        <div class="tp-grid">
          <span class="tp-k">live records</span><span class="tp-v" id="tp-live-count">0</span>
          <span class="tp-k">total records seen</span><span class="tp-v" id="tp-total-count">0</span>
        </div>
        <div class="tp-section-h" title="Ticks a site's occupancy lasted from birth (Genesis/PairProduction/Movement-arrival) to expiry (Evaporation/Annihilation/Movement-departure) — site-level, not particle-level.">BIRTH-TO-EXPIRY LIFETIME (ticks)</div>
        <div class="tp-grid" id="tp-lifetime-grid"></div>
        <div class="tp-section-h" title="Ticks a lattice site's readout stayed at 0 between a death and the next birth at that same site — the transaction latency.">TRANSACTION LATENCY — NULL DWELL TIME (ticks)</div>
        <div class="tp-grid" id="tp-latency-grid"></div>
        <div class="tp-section-h" title="A complete +1 -> 0 -> -1 -> 0 -> +1 cycle (or the mirror) reconstructed at a single site, birth-to-birth.">COMPLETE +1&harr;0&harr;-1 CYCLES (ticks, birth-to-birth)</div>
        <div class="tp-grid" id="tp-cycle-grid"></div>
        <button id="tp-reset">Reset tracker</button>
      </div>
      <div class="tp-note">
        The engine's ternary readout <b>state &isin; {&minus;1, 0, +1}</b> has only ONE zero —
        Voxel stores no birth tick and no null-orientation field (P3: manifestation is a
        many-to-one quotient of a larger finite record). A null's orientation
        (<b>0&darr;</b> = entered from +1, <b>0&uarr;</b> = entered from &minus;1) is therefore
        <b>reconstructed</b> here from the event journal's transition history, never read from a
        stored field. Lifetime and null-dwell figures are <b>site-level</b> (the lattice site's own
        occupancy history), since a site index is not a stable particle identity across Movement.
      </div>`;
    return root;
}

const KIND_ORDER = ['Genesis', 'Evaporation', 'Annihilation', 'PairProduction', 'WeakTransmutation', 'Movement'];

function renderGrid(el, pairs) {
    el.innerHTML = pairs.map(([k, v]) => `<span class="tp-k">${k}</span><span class="tp-v">${v}</span>`).join('');
}

function fmt(v) { return (v === null || v === undefined) ? '—' : v; }

export function mountTransactionPanel(host) {
    if (!host) return null;
    ensureCss();
    document.getElementById(PANEL_ID)?.remove();
    if (typeof window !== 'undefined' && window.__ftdTransactionPanel) {
        try { window.__ftdTransactionPanel.dispose(); } catch (e) { /* noop */ }
    }
    const panel = buildPanel();
    host.appendChild(panel);

    const toggle = panel.querySelector('#tp-toggle-journal');
    const unavailableEl = panel.querySelector('#tp-unavailable');
    const liveEl = panel.querySelector('#tp-live');
    const kindGrid = panel.querySelector('#tp-kind-grid');
    const liveCountEl = panel.querySelector('#tp-live-count');
    const totalCountEl = panel.querySelector('#tp-total-count');
    const lifetimeGrid = panel.querySelector('#tp-lifetime-grid');
    const latencyGrid = panel.querySelector('#tp-latency-grid');
    const cycleGrid = panel.querySelector('#tp-cycle-grid');
    const resetBtn = panel.querySelector('#tp-reset');

    let disposed = false;
    let syncingCheckbox = false;

    function activeCtxAndBridge() {
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        const bridge = resolveActiveScale0BridgeFromWindow() || ctx?.bridge || null;
        return { ctx, bridge };
    }

    toggle.addEventListener('change', () => {
        if (syncingCheckbox) return;
        const { bridge } = activeCtxAndBridge();
        if (bridge && typeof bridge.enableHistoryJournal === 'function') {
            bridge.enableHistoryJournal(toggle.checked);
        }
    });

    resetBtn.addEventListener('click', () => {
        const { ctx } = activeCtxAndBridge();
        if (ctx) getOrCreateTransactionTracker(ctx).reset();
    });

    function render() {
        if (disposed) return;
        const { ctx, bridge } = activeCtxAndBridge();
        const supported = isTransactionTrackingSupported(bridge);

        toggle.disabled = !supported;
        unavailableEl.hidden = supported;
        liveEl.hidden = !supported;

        if (!supported) {
            const state = getScale0State?.();
            const scenarioNote = state?.currentScenarioId
                ? ` (scenario "${state.currentScenarioId}")`
                : '';
            unavailableEl.textContent = bridge?.isWorker
                ? `Not available: the worker-backed WASM bridge does not yet forward the history journal across the worker boundary${scenarioNote}.`
                : `Not available: this bridge has no native history journal (mock-owned scenario${scenarioNote}, or the WASM build predates this feature). The transaction tracker is WASM-only.`;
            syncingCheckbox = true;
            toggle.checked = false;
            syncingCheckbox = false;
            return;
        }

        syncingCheckbox = true;
        toggle.checked = !!bridge.historyJournalEnabled();
        syncingCheckbox = false;

        const tracker = ctx ? getOrCreateTransactionTracker(ctx) : null;
        const snap = tracker ? tracker.snapshotTelemetry() : null;
        if (!snap) return;

        renderGrid(kindGrid, KIND_ORDER.map((k) => [k, snap.eventCounts[k] ?? 0]));
        liveCountEl.textContent = String(snap.liveRecordCount);
        totalCountEl.textContent = String(snap.totalRecordCount);
        renderGrid(lifetimeGrid, [
            ['count', snap.lifetime.count],
            ['min', fmt(snap.lifetime.min)],
            ['median', fmt(snap.lifetime.median)],
            ['max', fmt(snap.lifetime.max)],
        ]);
        renderGrid(latencyGrid, [
            ['count', snap.transactionLatency.count],
            ['min', fmt(snap.transactionLatency.min)],
            ['median', fmt(snap.transactionLatency.median)],
            ['max', fmt(snap.transactionLatency.max)],
        ]);
        renderGrid(cycleGrid, [
            ['complete cycles', snap.cycle.count],
            ['min length', fmt(snap.cycle.min)],
            ['median length', fmt(snap.cycle.median)],
            ['max length', fmt(snap.cycle.max)],
        ]);
    }

    render();
    const poll = setInterval(render, POLL_MS);

    const api = {
        element: panel,
        render,
        dispose: () => {
            disposed = true;
            clearInterval(poll);
            if (typeof window !== 'undefined' && window.__ftdTransactionPanel === api) window.__ftdTransactionPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdTransactionPanel = api;
    return api;
}

export function initTransactionPanel() {
    if (typeof window === 'undefined') return;
    if (window.__ftdTransactionPanel) return window.__ftdTransactionPanel;
    const host = document.getElementById('panel-transactions');
    if (!host) return;
    return mountTransactionPanel(host);
}
