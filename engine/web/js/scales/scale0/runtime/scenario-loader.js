import { MockBridge } from '../../../bridge-init.js';
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
    setFieldToggle,
    setFluxMock,
    setForceStyle,
} from '../state/store.js';
import {
    markScenarioOverrideRows,
    readButtonActive,
    readCheckboxValue,
    readInputValue,
    setButtonActive,
    setCheckboxValue,
    setForceStyleButtons,
    setInputValue,
    setSelectedScenarioId,
} from '../ui/dom.js';
import { clearScale0Timeline } from '../controller.js';

// Toggle-reset whitelist used by `applyToggleDefaults`.
//
// Contract: every scenario in `bridge/scenarios/*.js` and
// `engine/src/scenarios/*.cpp` MUST only mutate toggle keys that
// appear in `SCALE0_TOGGLES`. Keys outside the whitelist (e.g.
// `pair_production`, `langevin`, `latency_field`, `emergent_forces`)
// are intentionally NOT reset between scenarios — they are long-term
// research controls owned by the user, not scenario state.
//
// As of 2026-04-27 the actual mutated set is {genesis, coupling,
// damping, weak_transmutation, dual_substrate}, all whitelisted. If
// you add a scenario that needs to flip a non-whitelisted toggle,
// either (a) add the key to SCALE0_TOGGLES with its default value, or
// (b) restore the previous value at scenario-end. Don't leave the
// mutation hanging — that's the toggle-leak vector ARC-1 audited.
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
    // Tier 1 quantum overlays — treated as persistent user preferences
    'toggle-psi-squared',
    'toggle-phase',
    'toggle-lagrangian-density',
    'toggle-entropy-density',
    'toggle-grav-potential',
    'toggle-em-energy',
    'toggle-charge-density',
    'toggle-vorticity',
    // Tier 1/2/3 (2026-04-18).
    'toggle-helicity',
    'toggle-kretschmann',
    'toggle-horizon',
    'toggle-e-pressure',
    'toggle-b-pressure',
    'toggle-kinetic-energy',
    'toggle-fisher',
    'toggle-coherence',
];

// Map every overlay button id → corresponding state flag so we can round-trip
// the user's overlay preferences across scenario switches.
const FIELD_BUTTON_TO_FLAG = {
    'toggle-e-field':              'showEField',
    'toggle-b-field':              'showBField',
    'toggle-poynting':             'showPoynting',
    'toggle-div-field':            'showDivField',
    'toggle-flux-lines':           'showFluxLines',
    'toggle-force-em':             'showForceEM',
    'toggle-force-gravity':        'showForceGravity',
    'toggle-force-strong':         'showForceStrong',
    'toggle-force-weak':           'showForceWeak',
    'toggle-dual-substrate':       'showDualSubstrate',
    'toggle-chirality':            'showChirality',
    'toggle-light':                'showLight',
    'toggle-dark-halo':            'showDarkMatterHalo',
    'toggle-damping-zones':        'showDampingZones',
    'toggle-genesis-iso':          'showGenesisIsosurface',
    'toggle-confinement':          'showConfinement',
    'toggle-psi-squared':          'showPsiSquared',
    'toggle-phase':                'showPhase',
    'toggle-lagrangian-density':   'showLagrangianDensity',
    'toggle-entropy-density':      'showEntropyDensity',
    'toggle-grav-potential':       'showGravPotential',
    'toggle-em-energy':             'showEmEnergy',
    'toggle-charge-density':        'showChargeDensity',
    'toggle-vorticity':             'showVorticity',
    'toggle-helicity':              'showHelicity',
    'toggle-kretschmann':           'showKretschmann',
    'toggle-horizon':               'showHorizon',
    'toggle-e-pressure':            'showEPressure',
    'toggle-b-pressure':            'showBPressure',
    'toggle-kinetic-energy':        'showKineticEnergy',
    'toggle-fisher':                'showFisher',
    'toggle-coherence':             'showCoherence',
};

export function shouldUseFluxMock(bridge, scenarioName) {
    if (bridge && (bridge.isNativeGPU || bridge.constructor.name === 'WebSocketBridge')) {
        return false;
    }
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
    ctx.applyReflectiveBoundary(false);
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
        // Apply prerequisite toggles before dependents (C-arch-6).
        //
        // The C++ TermToggles::validate() inspects pairwise dependencies
        // every tick:
        //   weak_transmutation   requires dual_substrate
        //   triad_binding        requires dual_substrate
        //   pair_production      requires genesis
        //   strong_force / color_forces  amplify forces
        //   selective_damping    is a damping mode
        //
        // If a dependent toggle is enabled while its prerequisite is
        // still off, the validator fires a warning for one tick and the
        // engine briefly runs in an inconsistent regime. Sorting the
        // override list so prerequisites land first eliminates that
        // window without requiring a two-pass apply at every callsite.
        const prerequisites = ['dual_substrate', 'genesis', 'forces', 'damping'];
        const sorted = [...overrides].sort((a, b) => {
            const ia = prerequisites.indexOf(a[0]);
            const ib = prerequisites.indexOf(b[0]);
            if (ia !== -1 && ib === -1) return -1;
            if (ib !== -1 && ia === -1) return 1;
            if (ia !== -1 && ib !== -1) return ia - ib;
            return 0;
        });
        for (const [key, val, elId] of sorted) {
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

/**
 * Capture the user's current overlay-toggle preferences so they can be
 * re-applied after a scenario load resets the visual state.
 *
 * Covers: flux-volume + flux-slice, every field overlay button in
 * FIELD_BUTTON_IDS, and the force render-style.
 */
export function captureOverlayPreferences(state) {
    const overlays = {};
    for (const id of FIELD_BUTTON_IDS) {
        overlays[id] = readButtonActive(id);
    }
    return {
        fluxVolume: readButtonActive('toggle-flux-volume'),
        fluxSlice:  readButtonActive('toggle-flux-slice'),
        overlays,
        forceStyle: state?.forceStyle || 'arrows',
    };
}

/**
 * Re-apply captured overlay preferences. Runs AFTER a scenario has finished
 * loading and the default visual state has been reset. Updates:
 *   1. the DOM button `.active` classes (so the UI reflects the preference)
 *   2. the scale-0 state-store flags (so runtime samplers pick them up)
 *   3. the viewport overlay visibility (so the 3D scene renders them)
 */
export function restoreOverlayPreferences(prefs, state, viewportAdapter, getForceStyleFn) {
    if (!prefs) return;

    // Flux volume / slice — these have their own adapter paths
    setButtonActive('toggle-flux-volume', prefs.fluxVolume);
    setButtonActive('toggle-flux-slice',  prefs.fluxSlice);
    viewportAdapter.setFluxVolumeVisible(prefs.fluxVolume);
    viewportAdapter.setFluxSliceVisible(prefs.fluxSlice);

    // Every field overlay — button + store flag + viewport toggle
    for (const id of FIELD_BUTTON_IDS) {
        const wasOn = !!prefs.overlays?.[id];
        const flagKey = FIELD_BUTTON_TO_FLAG[id];
        setButtonActive(id, wasOn);
        if (flagKey) setFieldToggle(flagKey, wasOn);
        viewportAdapter.setOverlayVisible(flagKey, wasOn);
    }

    // Force render style (arrows / heatmap / flow / glyphs)
    if (prefs.forceStyle) {
        setForceStyle(prefs.forceStyle);
        setForceStyleButtons(prefs.forceStyle);
        const fieldSnapshot = { ...state.fieldFlags };
        viewportAdapter.syncForceStyle(prefs.forceStyle, fieldSnapshot);
    }

    // Mark the lattice dirty so the next tick recomputes and repaints.
    state.fieldNeedsUpdate = true;
    recomputeAnyFieldActive();
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

    // Preserve the user's current overlay-toggle preferences across the reset.
    // ctx.resetAllVisualState() → resetScale0VisualState() wipes every field
    // flag + button; without this snapshot the user has to re-enable their
    // chosen overlays every time they pick a new scenario.
    const overlayPrefs = captureOverlayPreferences(state);

    ctx.resetAllVisualState();
    applyAuxiliaryDefaults(ctx, viewportAdapter);

    const mainScale0 = ctx.bridge.capabilities.scale0;
    scenario.load({ bridge: ctx.bridge, capabilities: mainScale0, params }, params);

    // Bridge.tick just reset to 0 — any snapshots still in the timeline are
    // from the previous scenario and their tick numbers no longer match the
    // new sim. Wipe them so the scrub bar re-anchors on the fresh scenario.
    clearScale0Timeline();

    // Allocate a parallel JS MockBridge ("fluxMock") ONLY when
    // `shouldUseFluxMock` says it will own the physics for this
    // scenario. For WASM-canonical scenarios (quantum-*, light-*) the
    // mock is unused — skipping it saves an L³ buffer + a redundant
    // scenario-seeding pass and removes the divergence-masking risk
    // the architecture audit flagged (ARC-2).
    const useFluxMock = shouldUseFluxMock(ctx.bridge, scenario.id);
    const latticeSize = ctx.bridge.latticeSize || 32;
    let fluxMock = null;
    if (useFluxMock) {
        fluxMock = new MockBridge(latticeSize);
        fluxMock.capabilities.scale0.setBoundaryShape(readInputValue('boundary-select', 'cube'));
        fluxMock.capabilities.scale0.setReflectiveBoundary(readButtonActive('toggle-reflective'));
        fluxMock.capabilities.scale0.setupScenario(scenario.id);
    }

    applyToggleDefaults(mainScale0, fluxMock?.capabilities?.scale0 ?? null, scenario.id);
    if (fluxMock) {
        for (const [key, , elId] of DEFAULT_TOGGLES) {
            fluxMock.capabilities.scale0.setToggle(key, readCheckboxValue(elId));
        }
    }

    setFluxMock(fluxMock, useFluxMock);
    setCurrentScenarioId(scenario.id);
    setSelectedScenarioId(scenario.id);
    markScenarioOverrideRows(DEFAULT_TOGGLES);
    syncComboSliders(ctx.bridge);
    state.latticeNeedsUpload = true;

    // Restore the captured overlay preferences. Runs last so it overrides any
    // defaults applied by applyAuxiliaryDefaults or resetScale0VisualState.
    restoreOverlayPreferences(overlayPrefs, state, viewportAdapter);

    state.fieldNeedsUpdate = true;
    recomputeAnyFieldActive();
}

export async function resizeScale0Lattice(ctx, state, viewportAdapter, newSize) {
    const scenarioId = state.currentScenarioId || readInputValue('scenario-select', 'flux-pulse');
    const bridge = ctx.bridge;
    // Scale 0 now allocates ~988 bytes/site on the C++ heap (due to SU(2)/SU(3) link structures).
    const projectedBytes = Math.ceil(newSize ** 3 * 1000 * 1.3);
    const maxWasmMemory = 2 * 1024 * 1024 * 1024;

    if (projectedBytes >= maxWasmMemory) {
        const projGB = (projectedBytes / 1024 / 1024 / 1024).toFixed(2);
        const msg = `L=${newSize} would need ~${projGB} GB of WASM heap (max 2 GB). Refusing to resize.`;
        if (typeof window.showToast === 'function') window.showToast(msg, 'error');
        else console.warn('[Scale0] ' + msg);
        setInputValue('lattice-size', bridge.latticeSize || 32);
        return;
    }

    if (bridge && typeof bridge.resize === 'function') {
        try {
            await bridge.resize(newSize);
        } catch (e) {
            console.error('[Scale0] Failed to resize simulation lattice:', e);
            if (typeof window.showToast === 'function') {
                window.showToast(`Lattice resize failed: ${e.message}`, 'error');
            }
        }
    } else {
        bridge.latticeSize = newSize;
    }
    getScale0Scenario(scenarioId).load({ bridge }, { id: scenarioId });
    ctx.viewport.setLatticeSize(newSize);
    viewportAdapter.setFluxVolumeVisible(ctx.viewport.showFlux);

    if (bridge && bridge.capabilities && bridge.capabilities.scale0) {
        if (typeof bridge.capabilities.scale0.setBoundaryShape === 'function') {
            bridge.capabilities.scale0.setBoundaryShape(readInputValue('boundary-select', 'cube'));
        }
        if (typeof bridge.capabilities.scale0.setReflectiveBoundary === 'function') {
            bridge.capabilities.scale0.setReflectiveBoundary(readButtonActive('toggle-reflective'));
        }
    }

    // Same lazy-allocation rule as loadScale0Scenario (ARC-2).
    const useFluxMock = shouldUseFluxMock(bridge, scenarioId);
    let fluxMock = null;
    if (useFluxMock) {
        fluxMock = new MockBridge(newSize);
        fluxMock.capabilities.scale0.setBoundaryShape(readInputValue('boundary-select', 'cube'));
        fluxMock.capabilities.scale0.setReflectiveBoundary(readButtonActive('toggle-reflective'));
        fluxMock.capabilities.scale0.setupScenario(scenarioId);
    }

    for (const [key, , elId] of DEFAULT_TOGGLES) {
        const checked = readCheckboxValue(elId);
        bridge.capabilities.scale0.setToggle(key, checked);
        fluxMock?.capabilities.scale0.setToggle(key, checked);
    }

    setFluxMock(fluxMock, useFluxMock);
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
