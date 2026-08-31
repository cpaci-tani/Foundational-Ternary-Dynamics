/**
 * Scale 0 (Lattice) Controller
 *
 * Refactored into a package-style module with explicit runtime phases,
 * a viewport adapter, scenario registry, and UI bindings owned by Scale 0.
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { createScale0ViewportAdapter } from './viewport-adapter.js?v=2';
import {
    getFieldStateSnapshot,
    getScale0State,
    getActiveScale0Bridge,
    getActiveScale0Capability,
    setScale0PlaybackRunning,
    setScale0PlaybackSpeed,
    setFieldToggle as setFieldToggleState,
    setForceStyle as setForceStyleState,
    setLatticeNeedsUpload as markLatticeUpload,
    markFieldDirty,
    getPrimeTickOnLoad,
    setPrimeTickOnLoad,
    completeScale0AuthoritativeLoad,
    failScale0AuthoritativeLoad,
} from './state/store.js';
import { advanceSimulation } from './runtime/tick.js';
import { syncRenderableData } from './runtime/frame-sync.js';
import { disposeFieldOverlayRuntime, updateFieldOverlays } from './runtime/field-overlays.js?v=17';
import { updateDiagnosticsAndPanels } from './runtime/diagnostics.js?v=3';
import {
    exitScale0,
    loadScale0Scenario,
    resetScale0Scenario,
    resetScale0VisualState,
    resizeScale0Lattice,
    syncComboSliders,
    syncScale0ToggleUiFromEngine,
    stepScale0,
} from './runtime/scenario-loader.js?v=21';
import { bindScale0UI, handleScale0ShortcutKey } from './ui/bindings.js?v=11';
import { getSelectedScenarioId } from './ui/dom.js';
import { syncScale0LatticeSizeAvailability } from './ui/toolbar/limits.js?v=2';
import { Scale0ControlsComponent } from './ui/controls/component.js?v=6';
import {
    syncScale0FlowLineControls,
    syncScale0ParticleDisplay,
    wireScale0Controls,
} from './ui/controls/wire.js?v=14';
import { mountSymmetryPanel } from './ui/overlays/symmetry-panel.js';
// The Scale-0 overlay panels are first created by app.js at boot
// ("Creating panels…", one-time). The controller ALSO drives their
// lifecycle on engineMode switch: dispose() on destroy() (audit P1-4)
// paired with idempotent init*() on mount() so a switch back to lattice
// re-creates them. The init*() functions reuse their window singleton
// when present, so the boot-time calls and the mount() calls do not
// double-mount.
import { initFluxSlicePanel } from './ui/overlays/flux-slice-panel.js';
import { initWaveLabPanel } from './ui/overlays/wave-lab-panel.js?v=2';
import { initP1ObservablesPanel } from './ui/overlays/p1-observables-panel.js?v=3';
import { initConservationMicropanel } from './ui/overlays/conservation-micropanel.js';
import { initSpectrumPanel } from './ui/overlays/spectrum-panel.js';
import { initGravityPanel } from './ui/overlays/gravity-panel.js?v=6';
import { appRegistry } from '../../core/registry.js';
import { initTimePanel } from './ui/overlays/time-panel.js';
import { initThermoPanel } from './ui/overlays/thermo-panel.js?v=3';
import { initDispersionPanel } from './ui/overlays/dispersion-panel.js';
import { initKnotsPanel } from './ui/overlays/knots-panel.js';
import {
    initScaleContextPanel,
    SCALE0_LATTICE_SIZE_ACK_EVENT,
} from './ui/overlays/scale-context-panel.js?v=3';
import { PlayBarComponent } from '../../ui/components/play-bar/component.js';

const state = getScale0State();

let _playBar = null;
let _lastScenarioRequestBridge = null;
let _lastScenarioRequestId = null;

/** Synchronize every size-dependent Scale-0 UI surface from an engine ACK. */
export function syncScale0AuthoritativeLatticeSize(ctx, acknowledgedLatticeSize) {
    const size = Number(acknowledgedLatticeSize ?? ctx?.bridge?.latticeSize);
    if (!Number.isInteger(size) || size < 1) return false;
    const select = document.getElementById('lattice-size');
    if (select) select.value = String(size);
    ctx?.syncScale0InjectionBounds?.(size);
    ctx?.syncScale0SelectionBounds?.(size);
    ctx?.syncScale0FlowLineControls?.(size);
    if (ctx?.viewport?.setLatticeSize
        && Number(ctx.viewport.latticeSize) !== size) {
        ctx.viewport.setLatticeSize(size);
    }
    if (typeof window !== 'undefined') {
        window.dispatchEvent(new CustomEvent(SCALE0_LATTICE_SIZE_ACK_EVENT, {
            detail: { size, source: 'engine-acknowledgement' },
        }));
    }
    return true;
}

// ── Render mode ──────────────────────────────────────────────────────
// Removed as part of simplifying UI and removing the render system.

// Cache the Scale-0 viewport adapter per viewport instance. animate() calls
// viewportAdapter(ctx) 3×/frame (frame-sync, field-overlays, renderFrame) and the
// adapter is a ~50-closure object literal — rebuilding it each call was pure
// per-frame GC churn. The adapter only closes over `viewport` (read live on every
// method call), so a single instance per viewport stays valid; it rebuilds only
// when ctx.viewport identity changes (a scale switch reassigns it).
let _vaCachedViewport = null;
let _vaCachedAdapter = null;
function viewportAdapter(ctx) {
    const vp = ctx.viewport;
    if (vp !== _vaCachedViewport) {
        _vaCachedViewport = vp;
        _vaCachedAdapter = createScale0ViewportAdapter(vp);
    }
    return _vaCachedAdapter;
}

function renderFrame(ctx) {
    // Advance the viewport's animation clock ONLY when the sim is running.
    // This is what makes wall-clock-driven visuals (|ψ|² breathing, etc.)
    // stay static during pause even when overlay toggles force a repaint.
    // The dt here is nominal — picks up frame cadence at ~60Hz. Animations
    // that need hard-real-time accuracy should use their own clocks, but
    // the slow-pulse visuals (<1Hz) are fine with a nominal 16ms step.
    if (ctx.running && ctx.viewport?.advanceAnimationClock) {
        ctx.viewport.advanceAnimationClock(0.016);
    }
    viewportAdapter(ctx).render();
}

export function bindUI(ctx) {
    // Scenario-loader owns worker/in-thread acknowledgement timing; expose one
    // controller callback so every accepted size reaches the same UI sync path.
    ctx.syncScale0AuthoritativeLatticeSize = (size) => (
        syncScale0AuthoritativeLatticeSize(ctx, size)
    );
    // bootBridge has already constructed/accepted the initial engine lattice
    // before bindUI runs, so publish that first acknowledgement immediately.
    ctx.syncScale0AuthoritativeLatticeSize(ctx.bridge?.latticeSize);
    // Initialize Scale 0 control cards in the controls panel
    const controlsPanel = document.getElementById('panel-controls');
    if (controlsPanel) {
        new Scale0ControlsComponent(controlsPanel).init();
    }

    // Mount floating symmetry panel
    mountSymmetryPanel(document.getElementById('app'));

    // Flux-slice, Wave Lab, and P1-observables panels are mounted by
    // app.js into the side-panel tab system (#panel-flux-slice and
    // sibling slots) via their init*Panel() functions. They read live
    // ctx.bridge / state.fluxMock
    // through window.__ftdCtx + getScale0State() per frame, so the
    // closure-mount no longer needs to live here. See:
    //   engine/web/js/scales/scale0/ui/overlays/flux-slice-panel.js
    //   engine/web/js/scales/scale0/ui/overlays/p1-observables-panel.js

    bindScale0UI(ctx, {
        loadScenario,
        resize,
        viewportAdapter,
        getForceStyle,
        setLatticeNeedsUpload,
    });

    // Wire all Scale 0 control-panel interactions (physics toggles, injection,
    // parameter sliders, flux volume, field actions). Previously scattered in
    // app.js's wireControls(); now owned by Scale 0.
    //
    // Pass ctx through directly so that ctx.bridge / ctx.viewport remain
    // live-reading accessors (scale switches reassign them).
    wireScale0Controls(ctx, {
        setLatticeNeedsUpload,
        loadScenario,
    });

    if (typeof window !== 'undefined') window.__ftdCtx = ctx;
    appRegistry.register('scale0Ctx', ctx);

    // Browser/WebView form restoration can set an already-selected scenario
    // without emitting `change`. The explicit boot load below bindUI reads the
    // restored selector synchronously. Only a genuine BFCache pageshow needs a
    // later reconciliation; initial pageshow/rAF callbacks raced the explicit
    // load and could begin a second worker replacement after the UI was ready.
    // Native reconnect is different: the server may own a fresh default
    // RenderBridge even though the DOM and client state still agree, so force
    // one atomic profile replay for every completed socket generation.
    const reconcileRestoredScenario = (event) => {
        if (event && event.type === 'pageshow' && event.persisted !== true) return;
        if (ctx.engineMode && ctx.engineMode !== 'lattice') return;
        loadSelectedScenario(ctx);
    };
    if (!ctx._scale0ScenarioRestoreBound && typeof window !== 'undefined') {
        ctx._scale0ScenarioRestoreBound = true;
        window.addEventListener('pageshow', reconcileRestoredScenario);
    }
    ctx.onBridgeConnectionReady = () => {
        if (ctx.engineMode && ctx.engineMode !== 'lattice') return;
        syncScale0LatticeSizeAvailability(ctx.bridge?.isNativeGPU);
        loadSelectedScenario(ctx, { force: true });
    };

    // Provide a callback for the bridge worker's asynchronous 'frame' signal.
    // When paused: trigger lattice + overlay refresh so the UI doesn't stay blank.
    // When running: overlays normally refresh via fieldDataVersion (CTRL.FRAME
    // atomic). The one gap is the very first sampler delivery after a scenario
    // load — the rAF that consumed fieldNeedsUpdate=true fired with an empty
    // _samplerCache, and no further fieldDataVersion change fires until the next
    // tick. ctx._samplersPending=true (set by loadScale0Scenario) marks this
    // window; one markFieldDirty() forces the overlay to repaint as soon as real
    // sampler data arrives, without bypassing the per-frame throttle afterwards.
    ctx.onBridgePostFrame = (hadNewSamplers, forceUpload = false) => {
        if (ctx.engineMode && ctx.engineMode !== 'lattice') return;   // late native frame after a scale switch
        // Native FTS1 samples arrive independently of the lattice/particle
        // frame. The sweep that requested them has already consumed EMPTY (or
        // the previous epoch), so every new sampler delivery must schedule a
        // fresh overlay pass. Worker frames remain safe: markFieldDirty is
        // idempotent and the scheduler coalesces deliveries within a frame.
        if (hadNewSamplers) markFieldDirty();
        if (!ctx.running || forceUpload) {
            setLatticeNeedsUpload();
            if (!ctx.running) markFieldDirty();
        } else if (hadNewSamplers && ctx._samplersPending) {
            markFieldDirty();
        }
        // _samplersPending is NOT cleared here. The overlay sweep clears it once
        // it produces jobs from real sampler data (field-overlays.js). The worker
        // proxy returns EMPTY on the first _wantSampler(kind) call, so the first
        // postFrame can carry hadNewSamplers=true while the specific overlay's
        // data is still empty; clearing now would disarm the forced repaint and
        // leave the overlay blank until the next tick.
    };

    // Native physics is asynchronous and the bridge intentionally coalesces
    // playback demand. Version the field only from server acknowledgements so
    // overlay scheduling, telemetry, and lattice uploads describe completed
    // physics rather than attempted rAF calls.
    ctx.onBridgeSimulationComplete = ({ ticks = 1 } = {}) => {
        if (ctx.engineMode && ctx.engineMode !== 'lattice') return;   // late native ack after a scale switch
        const completed = Math.max(1, Math.trunc(Number(ticks) || 1));
        state.fieldDataVersion = (state.fieldDataVersion || 0) + completed;
        setLatticeNeedsUpload();
        markFieldDirty();
    };

    // A typed server error proves the socket is still responsive, so keep it
    // available for Reset/reload/manual Step. Pause automatic playback to avoid
    // resubmitting the same rejected CUDA tick every animation frame. A true
    // no-response watchdog additionally retires/reconnects the socket below.
    ctx.onBridgeSimulationError = () => {
        if (ctx.engineMode && ctx.engineMode !== 'lattice') return;   // a Scale-0 sim error must not pause another active scale
        ctx.pauseSimulation?.();
    };

    // Native live profile edits are optimistic only until ws_server validates
    // the whole TermToggles candidate. Repaint from the acknowledgement (or
    // rollback snapshot) so dependency/conflict rejection cannot leave the
    // checkbox card or boundary selector claiming physics the engine refused.
    ctx.onBridgeProfileUpdate = ({
        fluxBoundaryMode,
        fluxPeriodicAxis,
        latticeSize: acknowledgedLatticeSize = null,
        authoritativeScenarioAck = false,
        scenarioId: acknowledgedScenarioId = null,
        loadGeneration: acknowledgedLoadGeneration = null,
        error = null,
    } = {}) => {
        if (ctx.engineMode && ctx.engineMode !== 'lattice') return;   // late native profile ack after a scale switch
        const scenarioId = state.currentScenarioId || 'flux-pulse';
        const loadGeneration = Number(ctx._loadGeneration || 0);
        if (authoritativeScenarioAck
            && (acknowledgedScenarioId !== scenarioId
                || Number(acknowledgedLoadGeneration) !== loadGeneration)) {
            return;
        }
        const syncAcknowledgedLatticeSize = () => syncScale0AuthoritativeLatticeSize(
            ctx,
            acknowledgedLatticeSize,
        );
        const syncBoundaryUi = () => {
            if (Number.isInteger(fluxBoundaryMode)) {
                const select = document.getElementById('flux-boundary-mode');
                if (select) select.value = String(fluxBoundaryMode);
            }
            if (Number.isInteger(fluxPeriodicAxis)) {
                const axisSelect = document.getElementById('flux-periodic-axis');
                if (axisSelect) {
                    axisSelect.value = String(fluxPeriodicAxis);
                    axisSelect.disabled = false;
                }
            }
            ctx.viewport?.setBoundaryDynamics?.(
                Number.isInteger(fluxBoundaryMode) ? fluxBoundaryMode : 2,
                Number.isInteger(fluxPeriodicAxis) ? fluxPeriodicAxis : 2,
            );
        };
        if (authoritativeScenarioAck && error) {
            // WebSocketBridge has rolled its optimistic staged profile back to
            // the last server-confirmed mirror (or an authoritative rejection
            // snapshot). Repaint that truth before surfacing the failed load.
            syncComboSliders(ctx, state);
            syncScale0ToggleUiFromEngine(ctx, viewportAdapter(ctx), scenarioId);
            syncBoundaryUi();
            syncAcknowledgedLatticeSize();
            setLatticeNeedsUpload();
            markFieldDirty();
            failScale0AuthoritativeLoad({
                scenarioId,
                loadGeneration,
                reason: error,
            });
            return;
        }
        // Native scenario/profile acknowledgements carry the authoritative
        // constant values. Refresh the disabled K_B/G_N/damping controls from
        // that echo rather than leaving a cosmetic pre-ack value behind.
        syncComboSliders(ctx, state);
        const profileSynchronized = syncScale0ToggleUiFromEngine(
            ctx,
            viewportAdapter(ctx),
            scenarioId,
        );
        syncBoundaryUi();
        if (profileSynchronized && authoritativeScenarioAck) {
            // A previous resize transport failure is commit-uncertain. The
            // profile ACK's authoritative N must reach every size-dependent UI
            // surface before qualification is restored.
            syncAcknowledgedLatticeSize();
            let tick = null;
            try {
                const reportedTick = Number(ctx.bridge?.getDiagnostics?.()?.tick);
                if (Number.isFinite(reportedTick)) tick = reportedTick;
            } catch { /* acknowledgement remains valid without a reported tick */ }
            completeScale0AuthoritativeLoad({
                scenarioId,
                loadGeneration,
                tick,
                source: 'native-profile-ack',
            });
        }
        setLatticeNeedsUpload();
        markFieldDirty();
    };

    // Ensure the play bar is mounted (idempotent; may have been
    // pre-mounted by mountScale0PlaybackUI() before wireToolbar).
    mountScale0PlaybackUI();
    // Wire + show the prime-tick toggle now that the play bar exists.
    ensurePrimeTickButton(true);
}

/**
 * Mount the play bar in the viewport. Idempotent. Safe to
 * call before any Scale 0 context exists — the callbacks read the live
 * `window.__ftdCtx` at interaction time, so the controls remain functional
 * after a scale switch.
 *
 * Called from app.js BEFORE wireToolbar() so that the playback
 * button IDs (btn-play, btn-step, btn-reset,
 * ticks-per-frame, tpf-display) exist in the DOM when the toolbar
 * wirer looks them up.
 */
export function mountScale0PlaybackUI() {
    const viewportEl = document.getElementById('viewport');
    if (!viewportEl) return;
    if (!_playBar) {
        _playBar = new PlayBarComponent(viewportEl).mount();
    } else {
        _playBar.mount();
    }
}

// Wire + reveal the "prime tick on load" toggle in the play bar (Scale-0 only).
// Idempotent: the click handler is attached once (guarded by _ftdWired);
// subsequent calls only update visibility. The button ships hidden in the
// play-bar template and is revealed here so it never appears on non-lattice
// scales (the play bar persists across scale switches).
function ensurePrimeTickButton(visible) {
    const btn = document.getElementById('btn-prime-tick');
    if (!btn) return;
    if (!btn._ftdWired) {
        btn._ftdWired = true;
        const paint = (on) => {
            btn.classList.toggle('is-on', on);
            btn.setAttribute('aria-pressed', on ? 'true' : 'false');
        };
        paint(getPrimeTickOnLoad());
        btn.addEventListener('click', () => paint(setPrimeTickOnLoad(!getPrimeTickOnLoad())));
    }
    btn.hidden = !visible;
}

class Scale0LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
    }

    mount(ctx) {
        if (typeof window !== 'undefined') {
            this.bindEvent(window, 'pagehide', () => {
                try { exitScale0(); } catch { /* defensive: never block teardown */ }
            });
        }
        // (Re-)create the Scale-0 overlay panels (audit P1-4). On first
        // boot these already exist (app.js created them) and init*() is a
        // no-op reuse; after a switch away from lattice their destroy()
        // disposed them, so this re-creates them on re-entry. Each init*()
        // is idempotent and guards its own DOM host, so a missing host
        // (early boot ordering) is a safe no-op.
        try { initFluxSlicePanel(); } catch (e) { /* ignore */ }
        try { initWaveLabPanel(); } catch (e) { /* ignore */ }
        try { initP1ObservablesPanel(); } catch (e) { /* ignore */ }
        try { initConservationMicropanel(); } catch (e) { /* ignore */ }
        try { initSpectrumPanel(); } catch (e) { /* ignore */ }
        try {
            const gravityPanel = initGravityPanel();
            if (gravityPanel) appRegistry.register('panel:gravity', gravityPanel);
        } catch (e) { /* ignore */ }
        try { initTimePanel(); } catch (e) { /* ignore */ }
        try { initThermoPanel(); } catch (e) { /* ignore */ }
        try { initDispersionPanel(); } catch (e) { /* ignore */ }
        try { initKnotsPanel(); } catch (e) { /* ignore */ }
        try { initScaleContextPanel(); } catch (e) { /* ignore */ }
        // Reveal the prime-tick toggle whenever Scale 0 becomes active.
        try { ensurePrimeTickButton(true); } catch (e) { /* ignore */ }
        // Scale 1 owns the same particle shader while active and applies its
        // own shape/glow preset. Replay Scale 0's retained card values on every
        // re-entry so the visible controls and shared renderer cannot diverge.
        try { syncScale0ParticleDisplay(ctx); } catch (e) { /* ignore */ }
        try { syncScale0FlowLineControls(ctx); } catch (e) { /* ignore */ }
    }

    destroy(ctx) {
        _playBar?.cancelPendingSteps();
        try { disposeFieldOverlayRuntime(state); } catch (e) { /* ignore */ }
        super.destroy(ctx);
        try { exitScale0(); } catch (e) { /* ignore */ }
        // exitScale0/clearFluxMock null only the state.* copies; the ctx.* copies
        // otherwise keep a disposed WasmBridgeProxy (with useFluxMock=true) — a
        // footgun for any non-lattice reader of ctx.useFluxMock && ctx.fluxMock.
        if (ctx) { ctx.fluxMock = null; ctx.useFluxMock = false; }
        // Hide the prime-tick toggle when leaving Scale 0 (the play bar persists).
        try { ensurePrimeTickButton(false); } catch (e) { /* ignore */ }
        // Dispose the Scale-0 overlay panels on engineMode switch
        // (audit P1-4, 2026-05-27). Each has a self-driving rAF loop that
        // calls bridge.getDiagnostics() / getConservationTotals() every
        // frame and rebuilds DOM; without disposal they keep running in
        // non-lattice scales. dispose() unsubscribes the rAF, removes the
        // DOM subtree, and clears its window singleton. Each is idempotent
        // and re-created on the next Scale-0 mount via its init*() call.
        appRegistry.unregister('scale0Ctx');
        appRegistry.unregister('panel:gravity');
        if (typeof window !== 'undefined') {
            try { window.__ftdConservationPanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdWaveLabPanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdP1Panel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdSpectrumPanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdGravityPanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdTimePanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdThermoPanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdDispersionPanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdKnotsPanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdScaleContextPanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdFluxSlicePanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdGenesisBurstPanel?.dispose?.(); } catch (e) { /* ignore */ }
        }
    }
}

const _lifecycleController = new Scale0LifecycleController();

export function enter(ctx, options = {}) {
    _lifecycleController.mount(ctx);
}

export function exit(ctx) {
    _lifecycleController.destroy(ctx);
}

export function mount(ctx) {
    _lifecycleController.mount(ctx);
}

export function destroy(ctx) {
    _lifecycleController.destroy(ctx);
}

export function loadScenario(ctx, scenarioId, params) {
    _playBar?.cancelPendingSteps();
    _lastScenarioRequestBridge = ctx?.bridge ?? null;
    _lastScenarioRequestId = scenarioId;
    loadScale0Scenario(ctx, state, viewportAdapter(ctx), scenarioId, params);
}

/**
 * Load the currently displayed Scale-0 selection without relying on a DOM
 * `change` event. Returns false when the same bridge already received the same
 * request, unless `force` is used for a new native socket generation.
 */
export function loadSelectedScenario(ctx, { force = false } = {}) {
    const scenarioId = getSelectedScenarioId('flux-pulse');
    const bridge = ctx?.bridge ?? null;
    if (!force
        && _lastScenarioRequestBridge === bridge
        && _lastScenarioRequestId === scenarioId) return false;
    ctx?.pauseSimulation?.();
    loadScenario(ctx, scenarioId);
    return true;
}

export function animate(ctx) {
    advanceSimulation(ctx, state);
    syncRenderableData(ctx, state, viewportAdapter(ctx));
    updateFieldOverlays(ctx, state, viewportAdapter(ctx));
    renderFrame(ctx);
    updateDiagnosticsAndPanels(ctx, state);
    _playBar?.refresh();
}

export function step(ctx, tickCount = 1) {
    stepScale0(ctx, state, tickCount);
}

export function reset(ctx) {
    _playBar?.cancelPendingSteps();
    resetScale0Scenario(ctx, state, viewportAdapter(ctx));
}

export async function resize(ctx, newSize) {
    _playBar?.cancelPendingSteps();
    try {
        await resizeScale0Lattice(ctx, state, viewportAdapter(ctx), newSize);
    } catch (e) {
        console.error('[Scale0] Controller resize failed:', e);
    }
}

export function resetScale0(ctx) {
    resetScale0VisualState(ctx, state, viewportAdapter(ctx));
}

export function getFieldState() {
    return getFieldStateSnapshot();
}

export function getForceStyle() {
    return state.forceStyle;
}

export function setLatticeNeedsUpload() {
    markLatticeUpload();
}

export function getFluxMock() {
    return state.fluxMock;
}

export function getActivePhysicsOwner(ctx) {
    return getActiveScale0Bridge(ctx, state);
}

export function setPlaybackRunning(ctx, running) {
    return setScale0PlaybackRunning(ctx, running, state);
}

export function setPlaybackSpeed(ctx, speed) {
    return setScale0PlaybackSpeed(ctx, speed, state);
}

export function clearFluxMock() {
    exitScale0();
}

export function setFieldToggle(key, value) {
    setFieldToggleState(key, value);
}

export function setForceStyle(style) {
    setForceStyleState(style);
}

export function getCurrentScenarioId() {
    return state.currentScenarioId;
}

export function handleShortcutKey(key) {
    return handleScale0ShortcutKey(key);
}

export const animateLattice = animate;
export const resizeLattice = resize;

// managed via BaseLifecycleController
