/**
 * Scale 0 (Lattice) Controller
 *
 * Refactored into a package-style module with explicit runtime phases,
 * a viewport adapter, scenario registry, and UI bindings owned by Scale 0.
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { createScale0ViewportAdapter } from './viewport-adapter.js';
import {
    getFieldStateSnapshot,
    getScale0State,
    getActiveScale0Capability,
    setFieldToggle as setFieldToggleState,
    setForceStyle as setForceStyleState,
    setLatticeNeedsUpload as markLatticeUpload,
    markFieldDirty,
    getPrimeTickOnLoad,
    setPrimeTickOnLoad,
} from './state/store.js';
import { advanceSimulation } from './runtime/tick.js';
import { syncRenderableData } from './runtime/frame-sync.js';
import { updateFieldOverlays } from './runtime/field-overlays.js';
import { updateDiagnosticsAndPanels } from './runtime/diagnostics.js';
import {
    exitScale0,
    loadScale0Scenario,
    resetScale0Scenario,
    resetScale0VisualState,
    resizeScale0Lattice,
    stepScale0,
} from './runtime/scenario-loader.js';
import { bindScale0UI, handleScale0ShortcutKey } from './ui/bindings.js';
import { Scale0ControlsComponent } from './ui/controls/component.js';
import { wireScale0Controls } from './ui/controls/wire.js';
import { mountSymmetryPanel } from './ui/overlays/symmetry-panel.js';
// The Scale-0 overlay panels are first created by app.js at boot
// ("Creating panels…", one-time). The controller ALSO drives their
// lifecycle on engineMode switch: dispose() on destroy() (audit P1-4)
// paired with idempotent init*() on mount() so a switch back to lattice
// re-creates them. The init*() functions reuse their window singleton
// when present, so the boot-time calls and the mount() calls do not
// double-mount.
import { initFluxSlicePanel } from './ui/overlays/flux-slice-panel.js';
import { initWaveLabPanel } from './ui/overlays/wave-lab-panel.js';
import { initP1ObservablesPanel } from './ui/overlays/p1-observables-panel.js';
import { initConservationMicropanel } from './ui/overlays/conservation-micropanel.js';
import { initSpectrumPanel } from './ui/overlays/spectrum-panel.js';
import { initGravityPanel } from './ui/overlays/gravity-panel.js';
import { appRegistry } from '../../core/registry.js';
import { initTimePanel } from './ui/overlays/time-panel.js';
import { initThermoPanel } from './ui/overlays/thermo-panel.js';
import { initDispersionPanel } from './ui/overlays/dispersion-panel.js';
import { initKnotsPanel } from './ui/overlays/knots-panel.js';
import { initScaleContextPanel } from './ui/overlays/scale-context-panel.js';
import { PlayBarComponent } from '../../ui/components/play-bar/component.js';

const state = getScale0State();

let _playBar = null;

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
    });

    if (typeof window !== 'undefined') window.__ftdCtx = ctx;
    appRegistry.register('scale0Ctx', ctx);

    // Provide a callback for the bridge worker's asynchronous 'frame' signal.
    // When paused: trigger lattice + overlay refresh so the UI doesn't stay blank.
    // When running: overlays normally refresh via fieldDataVersion (CTRL.FRAME
    // atomic). The one gap is the very first sampler delivery after a scenario
    // load — the rAF that consumed fieldNeedsUpdate=true fired with an empty
    // _samplerCache, and no further fieldDataVersion change fires until the next
    // tick. ctx._samplersPending=true (set by loadScale0Scenario) marks this
    // window; one markFieldDirty() forces the overlay to repaint as soon as real
    // sampler data arrives, without bypassing the per-frame throttle afterwards.
    ctx.onBridgePostFrame = (hadNewSamplers) => {
        if (!ctx.running) {
            setLatticeNeedsUpload();
            markFieldDirty();
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
        try { initGravityPanel(); } catch (e) { /* ignore */ }
        try { initTimePanel(); } catch (e) { /* ignore */ }
        try { initThermoPanel(); } catch (e) { /* ignore */ }
        try { initDispersionPanel(); } catch (e) { /* ignore */ }
        try { initKnotsPanel(); } catch (e) { /* ignore */ }
        try { initScaleContextPanel(); } catch (e) { /* ignore */ }
        // Reveal the prime-tick toggle whenever Scale 0 becomes active.
        try { ensurePrimeTickButton(true); } catch (e) { /* ignore */ }
    }

    destroy(ctx) {
        super.destroy(ctx);
        try { exitScale0(); } catch (e) { /* ignore */ }
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
    loadScale0Scenario(ctx, state, viewportAdapter(ctx), scenarioId, params);
}

export function animate(ctx) {
    advanceSimulation(ctx, state);
    syncRenderableData(ctx, state, viewportAdapter(ctx));
    updateFieldOverlays(ctx, state, viewportAdapter(ctx));
    renderFrame(ctx);
    updateDiagnosticsAndPanels(ctx, state);
    _playBar?.refresh();
    // Live flux-slice panel: cheap no-op when hidden; internally
    // gated to every Nth render frame when visible.
    if (typeof window !== 'undefined') window.__ftdFluxSlicePanel?.update?.();
}

export function step(ctx) {
    stepScale0(ctx, state);
}

export function reset(ctx) {
    resetScale0Scenario(ctx, state, viewportAdapter(ctx));
}

export async function resize(ctx, newSize) {
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
