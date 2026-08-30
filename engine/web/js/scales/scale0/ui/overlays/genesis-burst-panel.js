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
    <div id="genesis-burst-panel" style="position:absolute; top:12px; right:12px; z-index:40; width:300px; padding:12px 14px; border-radius:12px; font-family:var(--font-sans,sans-serif); font-size:16px; background:var(--color-background-primary,rgba(20,20,24,0.92)); border:0.5px solid var(--color-border-secondary,rgba(255,255,255,0.25)); color:var(--color-text-primary,#eee); box-shadow:0 2px 12px rgba(0,0,0,0.3)">
        <div style="font-weight:500;margin-bottom:8px">Selected genesis response N(A)</div>
        <div style="display:flex;align-items:center;gap:8px;margin-bottom:6px">
            <span>A</span>
            <input ref="slider" type="range" min="5" max="90" step="1" value="16" style="flex:1">
            <span ref="aval" style="width:24px;text-align:right">16</span>
        </div>
        <div style="display:flex;gap:6px;margin-bottom:8px">
            <button ref="fire" style="flex:1;padding:5px;border-radius:8px;cursor:pointer">Fire</button>
            <button ref="sweep" style="flex:1;padding:5px;border-radius:8px;cursor:pointer">Sweep</button>
            <button ref="clear" style="padding:5px 8px;border-radius:8px;cursor:pointer">Clear</button>
        </div>
        <div ref="status" style="margin-bottom:6px;color:var(--color-text-secondary,#aaa);min-height:15px">ready</div>
        <canvas ref="plot" width="276" height="200" style="width:100%;display:block;border-radius:8px;background:var(--color-background-secondary,rgba(255,255,255,0.04))"></canvas>
        <div style="margin-top:6px;font-size:16px;color:var(--color-text-tertiary,#888);line-height:1.4">
            <span style="color:#378ADD">&#9679;</span> live (active owner) &nbsp;
            <span style="color:#BA7517">&#9675;</span> historical GPU run &nbsp;
            <span style="color:#639922">&#8211;</span> selected quadratic comparison
        </div>
    </div>
`;

export class GenesisBurstPanelComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
    }
}

export function mountGenesisBurstPanel(harness) {
    const host = document.getElementById('viewport') || document.body;
    document.getElementById(PANEL_ID)?.remove();
    if (typeof window !== 'undefined' && window.__ftdGenesisBurstPanel) {
        try { window.__ftdGenesisBurstPanel.dispose(); } catch (e) { /* noop */ }
    }
    const comp = new GenesisBurstPanelComponent();
    comp.mount(host);
    const panel = comp.element;

    const slider = comp.refs.slider, aval = comp.refs.aval, status = comp.refs.status, canvas = comp.refs.plot;
    const ctx2d = canvas.getContext('2d');
    const points = [];   // [{ A, N }]
    let busy = false;
    let disposed = false;
    let activeToken = null;

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

    slider.addEventListener('input', () => { aval.textContent = slider.value; });
    comp.refs.fire.addEventListener('click', () => fire(parseInt(slider.value, 10)));
    comp.refs.sweep.addEventListener('click', () => sweep());
    comp.refs.clear.addEventListener('click', () => { points.length = 0; draw(); status.textContent = 'cleared'; });

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
        draw();
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
    function draw() {
        const W = canvas.width, H = canvas.height;
        const padL = 30, padR = 8, padT = 8, padB = 18;
        const x0 = padL, x1 = W - padR, y0 = H - padB, y1 = padT;
        const lAlo = Math.log10(5), lAhi = Math.log10(90);
        const lNlo = Math.log10(2), lNhi = Math.log10(700);
        const px = (A) => x0 + (Math.log10(A) - lAlo) / (lAhi - lAlo) * (x1 - x0);
        const py = (N) => y0 - (Math.log10(Math.max(N, 1.01)) - lNlo) / (lNhi - lNlo) * (y0 - y1);

        ctx2d.clearRect(0, 0, W, H);
        ctx2d.strokeStyle = 'rgba(136,135,128,0.5)'; ctx2d.lineWidth = 1;
        ctx2d.beginPath(); ctx2d.moveTo(x0, y1); ctx2d.lineTo(x0, y0); ctx2d.lineTo(x1, y0); ctx2d.stroke();
        ctx2d.fillStyle = 'rgba(150,150,150,0.9)'; ctx2d.font = '16px sans-serif';
        ctx2d.fillText('N', 4, y1 + 8); ctx2d.fillText('A', x1 - 8, y0 + 14);
        for (const A of [10, 16, 30, 90]) ctx2d.fillText(String(A), px(A) - 5, y0 + 12);

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
    draw();
    renderBackendSupport();

    // ---- scenario-switch disposal guard -----------------------------------
    const guard = setInterval(() => {
        const sel = document.getElementById('scenario-select');
        if (sel && sel.value !== SCENARIO_ID) api.dispose();
        else renderBackendSupport();
    }, 500);

    const api = {
        element: panel,
        fire,
        getPoints: () => points.map((p) => ({ ...p })),
        getSupportStatus: () => panel.dataset.liveExperimentStatus,
        dispose: () => {
            disposed = true;
            if (activeToken) activeToken.cancelled = true;
            clearInterval(guard);
            if (typeof window !== 'undefined' && window.__ftdGenesisBurstPanel === api) window.__ftdGenesisBurstPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdGenesisBurstPanel = api;
    return api;
}
