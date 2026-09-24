// Selected genesis response N(A) — interactive fire panel (FTD-0269 provenance).
//
// Mounts a floating panel over the Scale-0 viewport for the `s0-seed-cluster-law`
// scenario. The user picks an injection amplitude A, "fires" a one-shot genesis
// burst at the lattice center (A·K_GENESIS, canonical ic1 stack), and the panel
// reads the resulting steady cluster size N (diag.manifested) and plots the
// point on a live N(A) curve. Historical campaign points and a quadratic curve
// are shown only as labeled comparisons; this panel does not establish either
// a universal broken-power law or a geometrically forced knee.
//
// The fire panel drives the active physics owner (flux mock for cluster-law):
// pauses the canonical transport, resets the lattice, injects, ticks
// ~220 steps, reads N, then restores. The 3D cluster itself is best viewed via
// the fixed-A `*-subknee/-knee/-superknee` answer-key scenarios (clean T=0 view).

import { BaseComponent } from '../../../../core/component.js';
import { K_GENESIS } from '../../../../constants.js';
import { configureGenesisClusterTerms } from '../../runtime/genesis-cluster-profile.js';
import { createCanvasSurface } from '../../../../ui/utils/canvas-surface.js';
import { LifetimeScope } from '../../../../ui/utils/lifetime-scope.js';
import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { attachInstrumentPanelCollapse } from '../../../../ui/utils/instrument-panel.js';
import {
    commitScale0ScientificMutation,
    getScale0State,
    resolveActiveScale0BridgeFromWindow,
    setScale0PlaybackRunning,
    SCALE0_MUTATION_REASONS,
    SCALE0_MUTATION_SOURCES,
} from '../../state/store.js';

const PANEL_ID = 'genesis-burst-panel';
const SCENARIO_ID = 's0-seed-cluster-law';
const SETTLE_TICKS = 220;

// Historical FTD-0261 GPU campaign points plus a selected quadratic comparison.
const FTD0261 = [[10, 4.0], [12, 8.4], [14, 16.4], [16, 21.6], [20, 27.4],
                 [25, 32.6], [30, 45.0], [40, 91.8], [50, 130.2], [70, 260.2], [90, 383.3]];
const K_EFF = 0.052;
const KNEE_A = 16;
const SWEEP_GRID = [10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90];

const TEMPLATE = `
    <div id="genesis-burst-panel" class="genesis-burst-panel instrument-panel" role="region" aria-label="Selected genesis response">
        <div class="genesis-burst-title">Selected genesis response N(A)</div>
        <div class="genesis-burst-amplitude">
            <span>A</span>
            <input ref="slider" type="range" min="5" max="90" step="1" value="16">
            <span ref="aval" class="genesis-burst-value">16</span>
        </div>
        <div class="genesis-burst-actions">
            <button ref="fire" class="genesis-burst-fire">Fire</button>
            <button ref="sweep" class="genesis-burst-sweep">Sweep</button>
            <button ref="clear" class="genesis-burst-clear">Clear</button>
        </div>
        <div ref="status" class="genesis-burst-status">ready</div>
        <canvas ref="plot" width="276" height="200" class="genesis-burst-plot"></canvas>
        <div class="genesis-burst-legend">
            <span class="genesis-burst-marker-live">&#9679;</span> live (active owner) &nbsp;
            <span class="genesis-burst-marker-historical">&#9675;</span> historical GPU run &nbsp;
            <span class="genesis-burst-marker-comparison">&#8211;</span> selected quadratic comparison
        </div>
    </div>
`;

export class GenesisBurstPanelComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
    }
}

export function mountGenesisBurstPanel(harness) {
    // Portal the scenario instrument at shell level. The viewport owns a
    // stacking context; keeping the panel inside it makes the mobile transport
    // and dock siblings intercept the chart regardless of panel z-index.
    const host = document.getElementById('app') || document.body;
    document.getElementById(PANEL_ID)?.remove();
    if (typeof window !== 'undefined' && window.__ftdGenesisBurstPanel) {
        try { window.__ftdGenesisBurstPanel.dispose(); } catch (e) { /* noop */ }
    }
    const comp = new GenesisBurstPanelComponent();
    comp.mount(host);
    const panel = comp.element;

    const slider = comp.refs.slider, aval = comp.refs.aval, status = comp.refs.status, canvas = comp.refs.plot;
    const points = [];   // [{ A, N }]
    let busy = false;
    let disposed = false;
    let activeToken = null;
    let canvasSurface = null;
    const lifetime = new LifetimeScope();
    const collapse = attachInstrumentPanelCollapse({
        element: panel,
        lifetime,
        label: 'Selected genesis response',
    });
    const redraw = () => canvasSurface?.redrawNow();

    const nativeUnavailableMessage = 'Live N(A) is unavailable on the native backend until reset, injection, and stepping have one acknowledged transaction. Switch to WASM for this experiment.';
    const nativeExperimentUnavailable = () => {
        const owner = resolveActiveScale0BridgeFromWindow();
        return !!(owner?.isNativeGPU || harness.bridge?.isNativeGPU);
    };
    const renderBackendSupport = () => {
        const unavailable = nativeExperimentUnavailable();
        const nextStatus = unavailable
            ? 'unavailable-native-unacknowledged'
            : 'available';
        if (panel.dataset.liveExperimentStatus === nextStatus) return !unavailable;
        const previousStatus = panel.dataset.liveExperimentStatus;
        panel.dataset.liveExperimentStatus = nextStatus;
        for (const button of [comp.refs.fire, comp.refs.sweep]) {
            button.disabled = unavailable;
            button.setAttribute('aria-disabled', unavailable ? 'true' : 'false');
            button.title = unavailable ? nativeUnavailableMessage : '';
        }
        if (unavailable) status.textContent = nativeUnavailableMessage;
        else if (previousStatus === 'unavailable-native-unacknowledged') status.textContent = 'ready';
        return !unavailable;
    };

    lifetime.on(slider, 'input', () => { aval.textContent = slider.value; });
    lifetime.on(comp.refs.fire, 'click', () => fire(parseInt(slider.value, 10)));
    lifetime.on(comp.refs.sweep, 'click', () => sweep());
    lifetime.on(comp.refs.clear, 'click', () => { points.length = 0; redraw(); status.textContent = 'cleared'; });

    function tokenIsCurrent(token) {
        const liveCtx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        return !disposed
            && !!token
            && !token.cancelled
            && liveCtx === token.ctx
            && Number(liveCtx?._loadGeneration) === token.loadGeneration
            && getScale0State().currentScenarioId === SCENARIO_ID
            && resolveActiveScale0BridgeFromWindow() === token.owner;
    }

    function resetAndInject(A, token) {
        if (!tokenIsCurrent(token)) return false;
        try { harness.setupScenario('empty'); } catch (e) { /* noop */ }
        try {
            configureGenesisClusterTerms(harness, 0.005, 0.02);
        } catch (e) { /* noop */ }
        const L = harness.getLatticeSize?.() ?? 32;
        const mc = Math.round((L - 1) / 2);
        harness.injectFlux(mc, mc, mc, A * K_GENESIS, 0, 0);
        return true;
    }

    async function runFire(A, token) {
        if (!resetAndInject(A, token)) return null;
        for (let t = 0; t < SETTLE_TICKS; t++) {
            if (!tokenIsCurrent(token)) return null;
            harness.tickScale0?.();
            if (t % 20 === 0) {
                status.textContent = `firing A=${A}… (tick ${t}/${SETTLE_TICKS})`;
                await new Promise((r) => setTimeout(r, 0));
                if (!tokenIsCurrent(token)) return null;
            }
        }
        // Worker-backed steps are asynchronous. setupScenario('empty')
        // resets the worker tick to zero and PhysicsHarness routes each
        // step through tickOnce(); wait for the posted diagnostics instead
        // of reading the stale pre-batch frame as N=0.
        if (harness.bridge?.isWorker) {
            const deadline = performance.now() + 30_000;
            while (harness.getTick() < SETTLE_TICKS) {
                if (!tokenIsCurrent(token)) return null;
                if (performance.now() > deadline) {
                    throw new Error(`genesis response worker stopped at tick ${harness.getTick()}/${SETTLE_TICKS}`);
                }
                await new Promise((r) => setTimeout(r, 10));
            }
        }
        if (!tokenIsCurrent(token)) return null;
        const N = harness.getDiagnostics?.()?.manifested ?? 0;
        points.push({ A, N });
        redraw();
        status.textContent = `A=${A} → N=${N}  (k=${(N / (A * A)).toFixed(3)})`;
        return N;
    }

    function startExperiment(work) {
        if (busy || disposed) return Promise.resolve(null);
        if (!renderBackendSupport()) return Promise.resolve(null);
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        const owner = resolveActiveScale0BridgeFromWindow();
        const loadGeneration = Number(ctx?._loadGeneration);
        if (!ctx || !owner || harness.bridge !== owner || !Number.isInteger(loadGeneration)) {
            return Promise.resolve(null);
        }
        const token = {
            cancelled: false,
            ctx,
            owner,
            loadGeneration,
        };
        const committed = commitScale0ScientificMutation(ctx, {
            reason: SCALE0_MUTATION_REASONS.GENESIS_EXPERIMENT,
            source: SCALE0_MUTATION_SOURCES.GENESIS_BURST,
            loadGeneration,
            owner,
        }, () => {
            busy = true;
            activeToken = token;
            const wasRunning = !!ctx.running;
            // Pause through the canonical transport boundary: this cancels
            // queued native ticks and synchronously publishes worker RUNNING=0.
            // Merely flipping ctx.running waits until a later app rAF and lets
            // autonomous ticks interleave with the deterministic experiment.
            if (typeof ctx.pauseSimulation === 'function') ctx.pauseSimulation();
            else {
                ctx.running = false;
                setScale0PlaybackRunning(ctx, false, getScale0State());
            }
            return (async () => {
                try {
                    return await work(token);
                } finally {
                    // A newer scenario owns running state after generation or
                    // owner turnover. Never restore the pre-experiment value
                    // into that newer record.
                    if (tokenIsCurrent(token)) {
                        ctx.running = wasRunning;
                        setScale0PlaybackRunning(ctx, wasRunning, getScale0State());
                        ctx.updatePlayButton?.();
                    }
                    if (activeToken === token) activeToken = null;
                    busy = false;
                }
            })();
        });
        return committed.accepted ? committed.result : Promise.resolve(null);
    }

    function fire(A) {
        return startExperiment((token) => runFire(A, token));
    }

    function sweep() {
        return startExperiment(async (token) => {
            for (const A of SWEEP_GRID) {
                if (!tokenIsCurrent(token)) return null;
                slider.value = String(A); aval.textContent = String(A);
                const result = await runFire(A, token);
                if (result === null || !tokenIsCurrent(token)) return null;
            }
            status.textContent = 'sweep complete';
            return points.map((p) => ({ ...p }));
        });
    }

    // ---- bespoke log-log N(A) plotter -------------------------------------
    function draw(surface) {
        const { ctx: ctx2d, width: W, height: H } = surface;
        ctx2d.font = '16px sans-serif';
        const layout = computeGenesisPlotLayout(W, H, (text) => ctx2d.measureText(text).width);
        const { x0, x1, y0, y1 } = layout;
        const lAlo = Math.log10(5), lAhi = Math.log10(90);
        const lNlo = Math.log10(2), lNhi = Math.log10(700);
        const px = (A) => x0 + (Math.log10(A) - lAlo) / (lAhi - lAlo) * (x1 - x0);
        const py = (N) => y0 - (Math.log10(Math.max(N, 1.01)) - lNlo) / (lNhi - lNlo) * (y0 - y1);

        ctx2d.clearRect(0, 0, W, H);
        ctx2d.strokeStyle = 'rgba(136,135,128,0.5)'; ctx2d.lineWidth = 1;
        ctx2d.beginPath(); ctx2d.moveTo(x0, y1); ctx2d.lineTo(x0, y0); ctx2d.lineTo(x1, y0); ctx2d.stroke();
        ctx2d.fillStyle = 'rgba(150,150,150,0.9)';
        ctx2d.fillText('N', 4, y1 + 16);
        ctx2d.fillText('A', layout.axisLabel.left, layout.labelBaseline);
        for (const tick of layout.ticks) ctx2d.fillText(tick.text, tick.left, layout.labelBaseline);

        // knee marker
        ctx2d.strokeStyle = 'rgba(95,94,90,0.8)'; ctx2d.setLineDash([3, 3]);
        ctx2d.beginPath(); ctx2d.moveTo(px(KNEE_A), y1); ctx2d.lineTo(px(KNEE_A), y0); ctx2d.stroke();
        ctx2d.setLineDash([]);
        ctx2d.fillStyle = 'rgba(150,150,150,0.9)'; ctx2d.fillText('knee', px(KNEE_A) + 2, y1 + 8);

        // analytic energy-budget line N = k_eff·A² (super-knee)
        ctx2d.strokeStyle = '#639922'; ctx2d.lineWidth = 1.5; ctx2d.setLineDash([5, 3]);
        ctx2d.beginPath();
        ctx2d.moveTo(px(KNEE_A), py(K_EFF * KNEE_A * KNEE_A));
        ctx2d.lineTo(px(90), py(K_EFF * 90 * 90)); ctx2d.stroke(); ctx2d.setLineDash([]);

        // FTD-0261 GPU ghost points
        ctx2d.strokeStyle = '#BA7517'; ctx2d.lineWidth = 1;
        for (const [A, N] of FTD0261) { ctx2d.beginPath(); ctx2d.arc(px(A), py(N), 3, 0, 6.2832); ctx2d.stroke(); }

        // live measured points + connecting line
        ctx2d.fillStyle = '#378ADD'; ctx2d.strokeStyle = '#378ADD'; ctx2d.lineWidth = 1.5;
        const sorted = [...points].sort((a, b) => a.A - b.A);
        ctx2d.beginPath();
        sorted.forEach((p, i) => { const X = px(p.A), Y = py(p.N); i ? ctx2d.lineTo(X, Y) : ctx2d.moveTo(X, Y); });
        ctx2d.stroke();
        for (const p of sorted) { ctx2d.beginPath(); ctx2d.arc(px(p.A), py(p.N), 3.2, 0, 6.2832); ctx2d.fill(); }
    }
    canvasSurface = createCanvasSurface(canvas, draw);
    canvasSurface.redrawNow();
    renderBackendSupport();

    // ---- scenario-switch disposal guard -----------------------------------
    const guard = rafCoordinator.subscribe('genesis-burst-panel-guard', { hz: 2, cb: () => {
        const sel = document.getElementById('scenario-select');
        if (sel && sel.value !== SCENARIO_ID) api.dispose();
        else renderBackendSupport();
    } });
    lifetime.defer(() => guard.unsubscribe());

    const api = {
        element: panel,
        suspendBackgroundWork: () => { if (activeToken) activeToken.cancelled = true; },
        fire,
        getPoints: () => points.map((p) => ({ ...p })),
        getSupportStatus: () => panel.dataset.liveExperimentStatus,
        collapse,
        dispose: () => {
            disposed = true;
            if (activeToken) activeToken.cancelled = true;
            lifetime.dispose();
            canvasSurface?.dispose();
            canvasSurface = null;
            if (typeof window !== 'undefined' && window.__ftdGenesisBurstPanel === api) window.__ftdGenesisBurstPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdGenesisBurstPanel = api;
    return api;
}

/** Logical plot geometry, exported so responsive label bounds can be tested. */
export function computeGenesisPlotLayout(width, height, measureText = (text) => String(text).length * 8) {
    const W = Math.max(1, Number(width) || 1);
    const H = Math.max(1, Number(height) || 1);
    const axisWidth = measureText('A');
    const rightTickWidth = measureText('90');
    const labelGap = 8;
    const outerRight = 6;
    const padL = 30;
    const padR = Math.ceil(rightTickWidth / 2 + labelGap + axisWidth + outerRight);
    const padT = 8;
    const padB = 26;
    const x0 = padL;
    const x1 = Math.max(x0 + 1, W - padR);
    const y0 = Math.max(padT + 1, H - padB);
    const y1 = padT;
    const lAlo = Math.log10(5);
    const lAhi = Math.log10(90);
    const px = (A) => x0 + (Math.log10(A) - lAlo) / (lAhi - lAlo) * (x1 - x0);
    const ticks = [10, 16, 30, 90].map((value) => {
        const text = String(value);
        const textWidth = measureText(text);
        const left = px(value) - textWidth / 2;
        return { value, text, left, right: left + textWidth };
    });
    const axisLabel = {
        text: 'A',
        left: W - outerRight - axisWidth,
        right: W - outerRight,
    };
    return {
        x0,
        x1,
        y0,
        y1,
        labelBaseline: H - 8,
        ticks,
        axisLabel,
        labelGap,
    };
}
