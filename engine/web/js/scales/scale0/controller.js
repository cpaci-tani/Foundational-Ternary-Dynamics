/**
 * Scale 0 (Lattice) Controller
 *
 * Refactored into a package-style module with explicit runtime phases,
 * a viewport adapter, scenario registry, and UI bindings owned by Scale 0.
 */

import { createScale0ViewportAdapter } from './viewport-adapter.js?v=2';
import {
    getFieldStateSnapshot,
    getScale0State,
    setFieldToggle as setFieldToggleState,
    setForceStyle as setForceStyleState,
    setLatticeNeedsUpload as markLatticeUpload,
} from './state/store.js?v=s1';
import { advanceSimulation } from './runtime/tick.js';
import { syncRenderableData } from './runtime/frame-sync.js';
import { updateFieldOverlays } from './runtime/field-overlays.js?v=6';
import { updateDiagnosticsAndPanels } from './runtime/diagnostics.js';
import {
    exitScale0,
    loadScale0Scenario,
    resetScale0Scenario,
    resetScale0VisualState,
    resizeScale0Lattice,
    shouldUseFluxMock,
    stepScale0,
} from './runtime/scenario-loader.js?v=q2';
import { bindScale0UI, handleScale0ShortcutKey } from './ui/bindings.js?v=2';
import { Scale0ControlsComponent } from './ui/controls/component.js?v=3';
import { wireScale0Controls } from './ui/controls/wire.js?v=2';
import { mountSymmetryPanel } from './ui/overlays/symmetry-panel.js';
import { MemoryRecorder } from './timeline/memory-recorder.js';
import { RenderController } from './timeline/render-controller.js';
import { ScrubBarComponent } from '../../ui/components/scrub-bar/component.js';
import { RenderChipComponent } from '../../ui/components/render-chip/component.js';

const state = getScale0State();

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
    _renderController.start(seconds);
}

export function cancelScale0Render() {
    _renderController?.cancel();
}

export function getScale0RenderController() { return _renderController; }

/**
 * Restore the engine state to the closest LOD-0 snapshot at or before `tick`,
 * then fast-forward the remainder. Returns true if hydration succeeded.
 */
export function hydrateToTick(ctx, tick) {
    const rec = _memoryRecorder;
    if (!rec) return false;
    const snap = rec.buffer.nearestBefore(tick);
    if (!snap || snap.lod !== 0) return false; // only LOD 0 is engine-loadable
    const ok = ctx.bridge.capabilities.scale0.loadScale0Snapshot?.(snap);
    if (!ok) return false;
    const delta = Math.max(0, tick - snap.tick);
    for (let i = 0; i < delta; i++) ctx.bridge.capabilities.scale0.tickScale0();
    return true;
}

export function resumeLive() { /* reserved for future use (e.g. resync UI) */ }

export function getScale0ScrubBar() { return _scrubBar; }

function viewportAdapter(ctx) {
    return createScale0ViewportAdapter(ctx.viewport);
}

function renderFrame(ctx) {
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

    // Mount the floating scrub bar + render chip inside the viewport.
    const viewportEl = document.getElementById('viewport');
    if (viewportEl && !_scrubBar) {
        _scrubBar = new ScrubBarComponent(viewportEl, {
            getMemoryBuffer: () => _memoryRecorder?.buffer ?? null,
            getRenderBuffer: () => _renderController?.buffer ?? null,
            getNowTick:      () => ctx.bridge.capabilities.scale0.getScale0Diagnostics?.()?.tick ?? 0,
            onScrub:         (tick) => hydrateToTick(ctx, tick),
            onScrubEnd:      () => resumeLive(),
            onRender:        (seconds) => startScale0Render(ctx, seconds),
        }).mount();
    }
    if (viewportEl && !_renderChip) {
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
