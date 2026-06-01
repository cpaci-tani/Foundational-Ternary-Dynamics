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
    setFieldToggle as setFieldToggleState,
    setForceStyle as setForceStyleState,
    setLatticeNeedsUpload as markLatticeUpload,
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
    shouldUseFluxMock,
    stepScale0,
} from './runtime/scenario-loader.js';
import { bindScale0UI, handleScale0ShortcutKey } from './ui/bindings.js';
import { Scale0ControlsComponent } from './ui/controls/component.js';
import { wireScale0Controls } from './ui/controls/wire.js';
import { mountSymmetryPanel } from './ui/overlays/symmetry-panel.js';
// The four Scale-0 overlay panels are first created by app.js at boot
// ("Creating panels…", one-time). The controller ALSO drives their
// lifecycle on engineMode switch: dispose() on destroy() (audit P1-4)
// paired with idempotent init*() on mount() so a switch back to lattice
// re-creates them. The init*() functions reuse their window singleton
// when present, so the boot-time calls and the mount() calls do not
// double-mount.
import { initFluxSlicePanel } from './ui/overlays/flux-slice-panel.js';
import { initP1ObservablesPanel } from './ui/overlays/p1-observables-panel.js';
import { initConservationMicropanel } from './ui/overlays/conservation-micropanel.js';
import { initSpectrumPanel } from './ui/overlays/spectrum-panel.js';
import { MemoryRecorder } from './timeline/memory-recorder.js';
import * as _lodMod from './timeline/lod.js';
import { ScrubBarComponent } from '../../ui/components/scrub-bar/component.js';

const state = getScale0State();

// Publish LOD helpers on window so the WASM bridge can upsample any-LOD
// snapshots during scrub playback without creating a circular import.
if (typeof window !== 'undefined') window.__ftdTimelineLod = _lodMod;

// ── Playback timeline ─────────────────────────────────────────────────
// Default budget split: 60% Memory / 40% Render of a 50 MB overall cap.
// resetScale0MemoryBudget(totalBytes) lets a future Settings panel re-tune.
const DEFAULT_MEMORY_BYTES = 30 * 1024 * 1024;
let _memoryBudgetBytes = DEFAULT_MEMORY_BYTES;
let _memoryRecorder = null;

function getMemoryRecorder(latticeN) {
    if (!_memoryRecorder || _memoryRecorder.latticeN !== latticeN) {
        _memoryRecorder = new MemoryRecorder({
            budgetBytes: _memoryBudgetBytes,
            latticeN,
        });
    }
    return _memoryRecorder;
}

export function getScale0MemoryRecorder() { return _memoryRecorder; }

export function resetScale0MemoryBudget(totalBytes) {
    _memoryBudgetBytes = Math.max(1 * 1024 * 1024, Math.floor(totalBytes * 0.6));
    _memoryRecorder = null; // lazy rebuild on next getMemoryRecorder call
}

let _scrubBar = null;

// ── Render mode ──────────────────────────────────────────────────────
// Removed as part of simplifying UI and removing the render system.

/**
 * Forget every recorded snapshot so the scrub bar tracks the fresh scenario
 * from tick 0. Called whenever a scenario (re)loads or the lattice resizes.
 */
export function clearScale0Timeline() {
    _memoryRecorder?.clear?.();
    _scrubBar?._resetPlayhead?.();
}

/**
 * Restore the engine state to the snapshot nearest `tick` for scrub display.
 *
 * Pure "load, don't re-simulate" — we always pick the nearest stored
 * snapshot (from render buffer if active, else memory buffer) and load it
 * directly. This makes drag-scrubbing instant regardless of LOD; the price
 * is that time resolves to the sample grid (coarser with older LOD zones),
 * which is the desired trade.
 *
 * Live sim resumes from the currently-loaded snapshot when the user
 * releases the scrub thumb (see `resumeLive`).
 */
export function hydrateToTick(ctx, tick) {
    const mb = _memoryRecorder?.buffer;
    if (!mb || mb.size === 0) return false;
    const snap = _nearest(mb, tick);
    if (!snap) return false;
    const ok = !!ctx.bridge.capabilities.scale0.loadScale0Snapshot?.(snap);
    if (!ok) return false;

    // Freeze live physics for the duration of the drag; the scrub end hook
    // clears this. advanceSimulation() reads state.scrubbing and short-circuits.
    state.scrubbing = true;

    // Force the next animate pass to upload the fresh lattice to the GPU and
    // recompute field overlays against it — otherwise the viewport keeps
    // showing the pre-scrub state even though the engine buffers changed.
    state.latticeNeedsUpload = true;
    state.fieldNeedsUpdate   = true;
    return true;
}

/** Nearest-by-tick (not nearest-before) for the smoothest scrub feel. */
function _nearest(buffer, tick) {
    const before = buffer.nearestBefore(tick);
    if (!before) return null;
    const snaps = buffer.snapshots();
    const idx = snaps.indexOf(before);
    const after = snaps[idx + 1];
    if (!after) return before;
    return (tick - before.tick) <= (after.tick - tick) ? before : after;
}

/**
 * Release the scrub-induced physics freeze. Called from the scrub-bar's
 * `onScrubEnd` (pointerup / strip dblclick / reset). Leaves the engine
 * state exactly as the last hydrated snapshot set it; sim resumes ticking
 * from there on the next animate pass.
 */
export function resumeLive() {
    state.scrubbing = false;
    state.latticeNeedsUpload = true;
    state.fieldNeedsUpdate   = true;
}

export function getScale0ScrubBar() { return _scrubBar; }

function viewportAdapter(ctx) {
    return createScale0ViewportAdapter(ctx.viewport);
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

    // Flux-slice diagnostic and P1-observables panels are mounted by
    // app.js into the side-panel tab system (#panel-flux-slice and
    // #panel-p1-observables slots) via initFluxSlicePanel() and
    // initP1ObservablesPanel(). They read live ctx.bridge / state.fluxMock
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

    // Pre-create the memory recorder so the first tick starts capturing.
    getMemoryRecorder(ctx.bridge.latticeSize || 32);
    if (typeof window !== 'undefined') window.__ftdCtx = ctx;

    // Ensure the scrub bar + render chip are mounted (idempotent; may
    // have been pre-mounted by mountScale0PlaybackUI() before wireToolbar).
    mountScale0PlaybackUI();
}

/**
 * Mount the scrub bar + render chip in the viewport. Idempotent. Safe to
 * call before any Scale 0 context exists — the callbacks read the live
 * `window.__ftdCtx` at interaction time, so the controls remain functional
 * after a scale switch.
 *
 * Called from app.js BEFORE wireToolbar() so that the playback
 * button IDs (btn-play, btn-local-play, btn-step, btn-reset,
 * ticks-per-frame, tpf-display) exist in the DOM when the toolbar
 * wirer looks them up.
 */
export function mountScale0PlaybackUI() {
    const viewportEl = document.getElementById('viewport');
    if (!viewportEl) return;
    if (!_scrubBar) {
        _scrubBar = new ScrubBarComponent(viewportEl, {
            getMemoryBuffer: () => _memoryRecorder?.buffer ?? null,
            getNowTick:      () => {
                const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
                const state = getScale0State();
                const mockScale0 = state.fluxMock?.capabilities?.scale0 || null;
                const activeScale0 = (state.useFluxMock && mockScale0) ? mockScale0 : ctx?.bridge?.capabilities?.scale0;
                return activeScale0?.getScale0Diagnostics?.()?.tick ?? 0;
            },
            onScrub: (tick) => {
                const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
                return ctx ? hydrateToTick(ctx, tick) : false;
            },
            onScrubEnd: () => resumeLive(),
        }).mount();
    }
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
        // (Re-)create the four Scale-0 overlay panels (audit P1-4). On first
        // boot these already exist (app.js created them) and init*() is a
        // no-op reuse; after a switch away from lattice their destroy()
        // disposed them, so this re-creates them on re-entry. Each init*()
        // is idempotent and guards its own DOM host, so a missing host
        // (early boot ordering) is a safe no-op.
        try { initFluxSlicePanel(); } catch (e) { /* ignore */ }
        try { initP1ObservablesPanel(); } catch (e) { /* ignore */ }
        try { initConservationMicropanel(); } catch (e) { /* ignore */ }
        try { initSpectrumPanel(); } catch (e) { /* ignore */ }
    }

    destroy(ctx) {
        super.destroy(ctx);
        try { exitScale0(); } catch (e) { /* ignore */ }
        try { clearScale0Timeline(); } catch (e) { /* ignore */ }
        // Dispose the four Scale-0 overlay panels on engineMode switch
        // (audit P1-4, 2026-05-27). Each has a self-driving rAF loop that
        // calls bridge.getDiagnostics() / getConservationTotals() every
        // frame and rebuilds DOM; without disposal they keep running in
        // non-lattice scales. dispose() unsubscribes the rAF, removes the
        // DOM subtree, and clears its window singleton. Each is idempotent
        // and re-created on the next Scale-0 mount via its init*() call.
        if (typeof window !== 'undefined') {
            try { window.__ftdConservationPanel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdP1Panel?.dispose?.(); } catch (e) { /* ignore */ }
            try { window.__ftdSpectrumPanel?.dispose?.(); } catch (e) { /* ignore */ }
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
    _scrubBar?.refresh();
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
export { shouldUseFluxMock };

// managed via BaseLifecycleController

