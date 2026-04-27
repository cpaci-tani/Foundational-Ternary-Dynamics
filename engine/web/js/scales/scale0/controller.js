/**
 * Scale 0 (Lattice) Controller
 *
 * Refactored into a package-style module with explicit runtime phases,
 * a viewport adapter, scenario registry, and UI bindings owned by Scale 0.
 */

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
// flux-slice-panel and p1-observables-panel are mounted by app_dag.js into
// the side-panel tab system; their init functions are imported there.
import { MemoryRecorder } from './timeline/memory-recorder.js';
import { RenderController } from './timeline/render-controller.js';
import * as _lodMod from './timeline/lod.js';
import { ScrubBarComponent } from '../../ui/components/scrub-bar/component.js';
import { RenderChipComponent } from '../../ui/components/render-chip/component.js';

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
const DEFAULT_RENDER_BYTES = 20 * 1024 * 1024; // ~40% of default 50 MB cap
let _renderController = null;
let _renderChip = null;

export function startScale0Render(ctx, seconds = 30) {
    if (_renderController?.running) return;
    const latticeN = ctx.bridge.latticeSize || 32;
    _renderController = new RenderController({
        budgetBytes: DEFAULT_RENDER_BYTES,
        latticeN,
        scale0Caps: ctx.bridge.capabilities.scale0,
    });
    _renderChip?.bindController(_renderController);

    // Freeze the live animate loop's tick path for the render's lifetime so
    // render-controller ticks don't fight the main-loop ticks. Also push an
    // upload flag after every slice so the canvas visibly fast-forwards
    // through the clip while it builds, and once more on completion so the
    // restored original snapshot is actually drawn.
    state.rendering = true;
    _renderController.addEventListener('progress', () => {
        state.latticeNeedsUpload = true;
        state.fieldNeedsUpdate   = true;
    });
    const clearRendering = () => {
        state.rendering = false;
        state.latticeNeedsUpload = true;
        state.fieldNeedsUpdate   = true;
    };
    _renderController.addEventListener('done',   clearRendering);
    _renderController.addEventListener('cancel', clearRendering);
    _renderController.addEventListener('error',  clearRendering);

    _renderController.start(seconds);
}

export function cancelScale0Render() {
    _renderController?.cancel();
}

/**
 * Forget every recorded snapshot so the scrub bar tracks the fresh scenario
 * from tick 0. Called whenever a scenario (re)loads or the lattice resizes —
 * stale snapshots from the previous run carry tick numbers that don't match
 * the new simulation, which would skew the scrub fraction-to-tick mapping
 * and hydrate wrong-scenario state on drag.
 *
 * Also cancels any in-flight render, since the render buffer is only
 * meaningful inside the scenario it was recorded against.
 */
export function clearScale0Timeline() {
    _memoryRecorder?.clear?.();
    if (_renderController?.running) _renderController.cancel();
    if (_renderController) _renderController.buffer = null;
    _scrubBar?._resetPlayhead?.();
}

export function getScale0RenderController() { return _renderController; }

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
    const rb = _renderController?.buffer;
    const mb = _memoryRecorder?.buffer;
    const source = (rb && rb.size > 0) ? rb : mb;
    if (!source || source.size === 0) return false;
    const snap = _nearest(source, tick);
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
    // app_dag.js into the side-panel tab system (#panel-flux-slice and
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
    // app_dag.js's wireControls(); now owned by Scale 0.
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
 * Called from app_dag.js BEFORE wireToolbar() so that the playback
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
            getRenderBuffer: () => _renderController?.buffer ?? null,
            getNowTick:      () => {
                const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
                return ctx?.bridge?.capabilities?.scale0?.getScale0Diagnostics?.()?.tick ?? 0;
            },
            onScrub: (tick) => {
                const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
                return ctx ? hydrateToTick(ctx, tick) : false;
            },
            onScrubEnd: () => resumeLive(),
            onRender:   (seconds) => {
                const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
                if (ctx) startScale0Render(ctx, seconds);
            },
        }).mount();
    }
    if (!_renderChip) {
        _renderChip = new RenderChipComponent(viewportEl, {
            onCancel: () => cancelScale0Render(),
        }).mount();
    }
}

// ── Test hooks (see design spec §Testing). Also convenient for console. ─
if (typeof window !== 'undefined') {
    window.__ftdStartRender = (seconds = 5) => {
        const ctx = window.__ftdCtx;
        if (ctx) startScale0Render(ctx, seconds);
    };
    window.__ftdCancelRender = () => cancelScale0Render();
}

export function enter(_ctx, _options = {}) {}

export function exit(_ctx) {
    exitScale0();
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

export function resize(ctx, newSize) {
    resizeScale0Lattice(ctx, state, viewportAdapter(ctx), newSize);
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

// Page-shutdown hook (Bridge-M1 audit, 2026-04-27): release the lazy
// fluxMock so its typed-array buffers (~21 MB at L=96) don't survive
// into a backgrounded tab. `pagehide` fires for both close and bfcache
// freeze; idempotent because `clearFluxMock` no-ops on a null mock.
if (typeof window !== 'undefined') {
    window.addEventListener('pagehide', () => {
        try { exitScale0(); } catch { /* defensive: never block teardown */ }
    });
}
