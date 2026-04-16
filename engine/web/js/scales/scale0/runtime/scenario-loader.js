import { MockBridge } from '../../../wasm-bridge-dag.js';
import { K_B, G_N, DAMPING, K_GENESIS } from '../../../constants.js';
import { SCALE0_TOGGLES, SCALE0_SCENARIO_OVERRIDES, LIGHT_SCENARIO_OVERRIDES } from '../../../config/toggles.js';
import { getScale0Scenario } from '../scenario-registry.js';
import {
    clearFluxMock,
    markFieldDirty,
    recomputeAnyFieldActive,
    resetFieldFlags,
    resetFrameState,
    setCurrentScenarioId,
    setFluxMock,
} from '../state/store.js';
import {
    markScenarioOverrideRows,
    readCheckboxValue,
    readInputValue,
    setButtonActive,
    setCheckboxValue,
    setForceStyleButtons,
    setInputValue,
    setSelectedScenarioId,
} from '../ui/dom.js';

const DEFAULT_TOGGLES = SCALE0_TOGGLES;
const FIELD_BUTTON_IDS = [
    'toggle-e-field',
    'toggle-b-field',
    'toggle-poynting',
    'toggle-div-field',
    'toggle-flux-lines',
    'toggle-force-em',
    'toggle-force-gravity',
    'toggle-force-strong',
    'toggle-force-weak',
    'toggle-dual-substrate',
    'toggle-chirality',
    'toggle-light',
    'toggle-dark-halo',
    'toggle-damping-zones',
    'toggle-genesis-iso',
    'toggle-confinement',
];

export function shouldUseFluxMock(bridge, scenarioName) {
    if (scenarioName.startsWith('flux-')) return true;
    if (scenarioName.startsWith('s0-seed-')) return true;
    if (scenarioName.startsWith('s0-field-')) return true;
    try {
        const probe = bridge.getFluxVolume && bridge.getFluxVolume();
        return !(probe && probe.length > 0);
    } catch (_e) {
        return true;
    }
}

function syncComboSliders(bridge) {
    const defaults = { kb: K_B, gn: G_N, damping: DAMPING };
    const map = [
        { id: 'combo-kb', valId: 'combo-kb-val', param: 'kb', fmt: 3 },
        { id: 'combo-gn', valId: 'combo-gn-val', param: 'gn', fmt: 3 },
        { id: 'combo-damp', valId: 'combo-damp-val', param: 'damping', fmt: 3 },
    ];
    for (const slider of map) {
        const el = document.getElementById(slider.id);
        const display = document.getElementById(slider.valId);
        if (!el || !display) continue;
        const value = bridge?.getParam ? bridge.getParam(slider.param) : defaults[slider.param];
        if (value != null) {
            el.value = value;
            display.textContent = value.toFixed(slider.fmt);
        }
    }
}

function applyAuxiliaryDefaults(ctx, viewportAdapter) {
    ctx.applyTicksPerFrameFromSlider(50);
    ctx.applyBoundaryShape('cube');
    ctx.applyReflectiveBoundary(true);
    viewportAdapter.setFluxVolumeVisible(true);
    viewportAdapter.setFluxSliceVisible(false);
    setButtonActive('toggle-flux-volume', true);
    setButtonActive('toggle-flux-slice', false);
}

function applyToggleDefaults(mainScale0, mockScale0, scenarioName) {
    for (const [key, val, elId] of DEFAULT_TOGGLES) {
        mainScale0.setToggle(key, val);
        setCheckboxValue(elId, val);
        mockScale0?.setToggle(key, val);
    }

    const overrides = SCALE0_SCENARIO_OVERRIDES[scenarioName];
    if (overrides) {
        for (const [key, val, elId] of overrides) {
            mainScale0.setToggle(key, val);
            setCheckboxValue(elId, val);
            mockScale0?.setToggle(key, val);
        }
    }

    if (scenarioName.startsWith('light-')) {
        for (const [key, val, elId] of LIGHT_SCENARIO_OVERRIDES) {
            mainScale0.setToggle(key, val);
            setCheckboxValue(elId, val);
            mockScale0?.setToggle(key, val);
        }
    }
}

export function resetScale0VisualState(ctx, state, viewportAdapter) {
    resetFieldFlags();
    state.forceStyle = 'arrows';
    resetFrameState();
    viewportAdapter.setFluxVolumeVisible(true);
    viewportAdapter.setFluxSliceVisible(false);
    viewportAdapter.clearScaleVisuals();
    setButtonActive('toggle-flux-volume', true);
    setButtonActive('toggle-flux-slice', false);
    for (const id of FIELD_BUTTON_IDS) setButtonActive(id, false);
    setForceStyleButtons('arrows');
}

export function loadScale0Scenario(ctx, state, viewportAdapter, scenarioId, params = {}) {
    const scenario = getScale0Scenario(scenarioId);
    ctx.resetAllVisualState();
    applyAuxiliaryDefaults(ctx, viewportAdapter);

    const mainScale0 = ctx.bridge.capabilities.scale0;
    scenario.load({ bridge: ctx.bridge, capabilities: mainScale0, params }, params);

    const latticeSize = ctx.bridge.latticeSize || 32;
    const fluxMock = new MockBridge(latticeSize);
    fluxMock.capabilities.scale0.setBoundaryShape(readInputValue('boundary-select', 'cube'));
    fluxMock.capabilities.scale0.setReflectiveBoundary(readCheckboxValue('reflective-boundary', true));
    fluxMock.capabilities.scale0.setupScenario(scenario.id);

    applyToggleDefaults(mainScale0, fluxMock.capabilities.scale0, scenario.id);
    for (const [key, , elId] of DEFAULT_TOGGLES) {
        fluxMock.capabilities.scale0.setToggle(key, readCheckboxValue(elId));
    }

    setFluxMock(fluxMock, shouldUseFluxMock(ctx.bridge, scenario.id));
    setCurrentScenarioId(scenario.id);
    setSelectedScenarioId(scenario.id);
    markScenarioOverrideRows(DEFAULT_TOGGLES);
    syncComboSliders(ctx.bridge);
    state.latticeNeedsUpload = true;
    state.fieldNeedsUpdate = true;
    recomputeAnyFieldActive();
}

export function resizeScale0Lattice(ctx, state, viewportAdapter, newSize) {
    const scenarioId = state.currentScenarioId || readInputValue('scenario-select', 'flux-pulse');
    const bridge = ctx.bridge;
    const projectedBytes = Math.ceil(newSize ** 3 * 330 * 1.3);
    const maxWasmMemory = 2 * 1024 * 1024 * 1024;

    if (projectedBytes >= maxWasmMemory) {
        const projGB = (projectedBytes / 1024 / 1024 / 1024).toFixed(2);
        const msg = `L=${newSize} would need ~${projGB} GB of WASM heap (max 2 GB). Refusing to resize.`;
        if (typeof window.showToast === 'function') window.showToast(msg, 'error');
        else console.warn('[Scale0] ' + msg);
        setInputValue('lattice-size', bridge.latticeSize || 32);
        return;
    }

    bridge.latticeSize = newSize;
    getScale0Scenario(scenarioId).load({ bridge }, { id: scenarioId });
    ctx.viewport.setLatticeSize(newSize);
    viewportAdapter.setFluxVolumeVisible(ctx.viewport.showFlux);

    const fluxMock = new MockBridge(newSize);
    fluxMock.capabilities.scale0.setBoundaryShape(readInputValue('boundary-select', 'cube'));
    fluxMock.capabilities.scale0.setReflectiveBoundary(readCheckboxValue('reflective-boundary', true));
    fluxMock.capabilities.scale0.setupScenario(scenarioId);

    for (const [key, , elId] of DEFAULT_TOGGLES) {
        const checked = readCheckboxValue(elId);
        bridge.capabilities.scale0.setToggle(key, checked);
        fluxMock.capabilities.scale0.setToggle(key, checked);
    }

    setFluxMock(fluxMock, shouldUseFluxMock(bridge, scenarioId));
    state.latticeNeedsUpload = true;
    markFieldDirty();
    state.tickAccumulator.reset();
}

export function stepScale0(ctx, state) {
    const mainScale0 = ctx.bridge.capabilities.scale0;
    const mockScale0 = state.fluxMock?.capabilities?.scale0 || null;
    if (!state.useFluxMock) mainScale0.tickScale0();
    if (mockScale0) mockScale0.tickScale0();
    state.latticeNeedsUpload = true;
    state.fieldNeedsUpdate = true;
}

export function resetScale0Scenario(ctx, state, viewportAdapter) {
    loadScale0Scenario(ctx, state, viewportAdapter, state.currentScenarioId || 'flux-pulse');
}

export function exitScale0() {
    clearFluxMock();
}

export { K_GENESIS };
