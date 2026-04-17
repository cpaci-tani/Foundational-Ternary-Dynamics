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
import { updateFieldOverlays } from './runtime/field-overlays.js?v=5';
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

const state = getScale0State();

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
