import { getPhysicsHarness } from '../../../physics/index.js';
import { MockBridge } from '../../../bridge-init.js';
import { MockBridgeProxy } from '../../../bridge/mock-bridge-proxy.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import { K_B, G_N, DAMPING, K_GENESIS } from '../../../constants.js';
import { SCALE0_TOGGLES, SCALE0_SCENARIO_OVERRIDES, LIGHT_SCENARIO_OVERRIDES, SCALE0_SCENARIO_BOUNDARY, SCALE0_ABSORBING_SCENARIOS, SCALE0_MASS_GRAVITY_SCENARIOS } from '../../../config/toggles.js';
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
    FIELD_TOGGLE_BINDINGS,
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

// ── Per-scenario boundary resolution ────────────────────────────────
// A scenario may declare a boundary preference in SCALE0_SCENARIO_BOUNDARY
// (config/toggles.js); when it does, that wins over the live DOM controls.
// Otherwise fall back to the user's #boundary-select / #toggle-reflective.
// (Keeps a scenario's boundary need out of raw DOM reads — the UI↔bridge
// coupling noted in SPEC_SCALE0_SCENARIO_ARCHITECTURE.md §6.6.)
function boundaryShapeFor(id) {
    const b = SCALE0_SCENARIO_BOUNDARY[id];
    return (b && b.shape) ? b.shape : readInputValue('boundary-select', 'cube');
}
function reflectiveFor(id) {
    const b = SCALE0_SCENARIO_BOUNDARY[id];
    return (b && typeof b.reflective === 'boolean') ? b.reflective : readButtonActive('toggle-reflective');
}

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
// Round-trip the user's overlay preferences across scenario switches. Both maps
// are DERIVED from the canonical button↔flag list in ui/dom.js
// (FIELD_TOGGLE_BINDINGS, shared with bindings.js) so this can never drift behind
// it again — the old hand-maintained mirror had silently fallen 4 overlays behind
// (the 2026-06-03 substrate overlays: state-field / latency / gauss-residual /
// moore-decomp were missing). B1 fix, 2026-06-05.
const FIELD_BUTTON_IDS = FIELD_TOGGLE_BINDINGS.map(([id]) => id);
const FIELD_BUTTON_TO_FLAG = Object.fromEntries(FIELD_TOGGLE_BINDINGS);
// Scenarios in this list are intentionally owned by the JS scenario harness for
// the user-visible dashboard path. Keep the C++ mirror branch in place for
// parity and future WASM rebuilds, but do not depend on a checked-in WASM bundle
// already containing the new branch.
const SCALE0_MOCK_OWNED_SCENARIOS = new Set([
    's0-field-spacetime-forcing-boundary',
]);
const SCALE0_SCENARIO_VISUAL_PROFILES = {
    's0-field-spacetime-forcing-boundary': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxThreshold: 0.001,
        fluxOpacity: 0.85,
    },
};

export function shouldUseFluxMock(bridge, scenarioName) {
    if (bridge && (bridge.isNativeGPU || bridge.constructor.name === 'WebSocketBridge')) {
        return false;
    }
    if (SCALE0_MOCK_OWNED_SCENARIOS.has(scenarioName)) return true;
    if (scenarioName.startsWith('flux-')) return true;
    try {
        const probe = bridge.getFluxVolume && bridge.getFluxVolume();
        return !(probe && probe.length > 0);
    } catch (_e) {
        return true;
    }
}

// Phase 2: when the flag is on AND the page is cross-origin isolated
// (SharedArrayBuffer available), run flux-*/s0-* physics in a Web Worker
// (MockBridgeProxy) so the heavy tick never stalls render. Otherwise fall back
// to the in-thread MockBridge (Safari/iOS, or a deploy host without COOP/COEP).
// Set FTD_PHYSICS_WORKER false to force the in-thread path everywhere.
// Window-overridable (default true) so tests can force the synchronous in-thread
// MockBridge — e.g. audit-regression's manual `b.tick()` + immediate
// `b.getEnergyAudit()` pattern, which the async worker proxy cannot serve. Set
// `window.__ftdPhysicsWorker = false` before load to opt out.
const FTD_PHYSICS_WORKER = (typeof window !== 'undefined' && window.__ftdPhysicsWorker !== undefined)
    ? !!window.__ftdPhysicsWorker
    : true;
export function workerEligible(scenarioId, bridge) {
    return FTD_PHYSICS_WORKER
        && typeof SharedArrayBuffer !== 'undefined'
        && globalThis.crossOriginIsolated === true
        && shouldUseFluxMock(bridge, scenarioId);
}
function makeFluxMock(latticeSize, scenarioId, bridge) {
    return workerEligible(scenarioId, bridge)
        ? new MockBridgeProxy(latticeSize)
        : new MockBridge(latticeSize);
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

function setDisplayText(id, text) {
    if (typeof document === 'undefined') return;
    const el = document.getElementById(id);
    if (el) el.textContent = text;
}

function applyScenarioVisualProfile(ctx, state, viewportAdapter, scenarioId) {
    const profile = SCALE0_SCENARIO_VISUAL_PROFILES[scenarioId];
    if (!profile || !ctx?.viewport) return;

    if (profile.fluxVolume === true) {
        viewportAdapter.setFluxVolumeVisible(true);
        setButtonActive('toggle-flux-volume', true);
    }
    if (profile.fluxSlice === true) {
        viewportAdapter.setFluxSliceVisible(true);
        setButtonActive('toggle-flux-slice', true);
    }
    if (typeof profile.fluxPointScale === 'number') {
        ctx.viewport.setFluxPointScale(profile.fluxPointScale);
        ctx.viewport.setFluxSlicePointScale?.(profile.fluxPointScale);
        setInputValue('flux-point-scale', profile.fluxPointScale);
        setDisplayText('flux-point-scale-val', profile.fluxPointScale.toFixed(1));
    }
    if (typeof profile.fluxThreshold === 'number') {
        ctx.viewport.setFluxThreshold(profile.fluxThreshold);
        ctx.viewport.setFluxSliceThreshold?.(profile.fluxThreshold);
        setInputValue('flux-threshold', profile.fluxThreshold);
        setDisplayText('flux-threshold-val', profile.fluxThreshold.toFixed(3));
    }
    if (typeof profile.fluxOpacity === 'number') {
        ctx.viewport.setFluxOpacity(profile.fluxOpacity);
        ctx.viewport.setFluxSliceOpacity?.(profile.fluxOpacity);
        setInputValue('flux-opacity', profile.fluxOpacity);
        setDisplayText('flux-opacity-val', profile.fluxOpacity.toFixed(2));
    }

    state.latticeNeedsUpload = true;
    markFieldDirty();
}

function applyAuxiliaryDefaults(ctx, viewportAdapter, scenarioId) {
    ctx.applyTicksPerFrameFromSlider(50);
    // Boundary: a scenario may pin its own (SCALE0_SCENARIO_BOUNDARY); otherwise
    // reset to the default cube + non-reflective. Without honoring the config
    // here, this step would clobber the reflective boundary the scenario needs
    // (it runs after the flux-mock boundary is set). flux-zero-point relies on
    // this to keep its energy trapped → persistent floor.
    const bnd = SCALE0_SCENARIO_BOUNDARY[scenarioId] || {};
    ctx.applyBoundaryShape(bnd.shape ?? 'cube');
    ctx.applyReflectiveBoundary(bnd.reflective ?? false);
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
    telemetryHub.resetScale(0);

    // Preserve the user's current overlay-toggle preferences across the reset.
    // ctx.resetAllVisualState() → resetScale0VisualState() wipes every field
    // flag + button; without this snapshot the user has to re-enable their
    // chosen overlays every time they pick a new scenario.
    const overlayPrefs = captureOverlayPreferences(state);

    const useFluxMock = shouldUseFluxMock(ctx.bridge, scenario.id);
    const latticeSize = ctx.bridge.latticeSize || 33;
    let fluxMock = null;
    if (useFluxMock) {
        fluxMock = makeFluxMock(latticeSize, scenario.id, ctx.bridge);
        fluxMock.capabilities.scale0.setBoundaryShape(boundaryShapeFor(scenario.id));
        fluxMock.capabilities.scale0.setReflectiveBoundary(reflectiveFor(scenario.id));
        fluxMock.capabilities.scale0.setupScenario(scenario.id);
    }

    applyToggleDefaults(ctx.bridge.capabilities.scale0, fluxMock?.capabilities?.scale0 ?? null, scenario.id);
    if (fluxMock) {
        for (const [key, , elId] of DEFAULT_TOGGLES) {
            fluxMock.capabilities.scale0.setToggle(key, readCheckboxValue(elId));
        }
    }

    setFluxMock(fluxMock, useFluxMock);
    ctx.useFluxMock = useFluxMock;
    ctx.fluxMock = fluxMock;

    ctx.resetAllVisualState();
    applyAuxiliaryDefaults(ctx, viewportAdapter, scenario.id);

    const activeBridge = (useFluxMock && fluxMock) ? fluxMock : ctx.bridge;
    const harness = getPhysicsHarness(activeBridge);
    scenario.load(harness, params);

    // Gravity/wave family (SCALE0_ABSORBING_SCENARIOS): set LAST, after
    // scenario.load (which resets the engine toggles under the dual-bridge
    // routing) so these actually reach the running bridge. Two things:
    //   • absorbing_boundary — outgoing waves disperse into the void at the
    //     faces (render_bridge.cpp Rule 5b). Scoped here so other scenarios keep
    //     their full flux volume (enabling it broadly collapsed volumes to slabs).
    //   • latency_field — run the REAL latency Poisson so the gravity panel shows
    //     the genuine C++ potential, not only the |J|² proxy. Source is the
    //     [IMPOSED] field-energy density (field_energy_gravity) for the FLUX
    //     scenarios, OR real manifested rest mass M_REST·|state| for the
    //     MASS-gravity scenarios (SCALE0_MASS_GRAVITY_SCENARIOS, e.g. massive-body).
    const isFluxGravity = SCALE0_ABSORBING_SCENARIOS.has(scenario.id);
    const isMassGravity = SCALE0_MASS_GRAVITY_SCENARIOS.has(scenario.id);
    for (const sc of [ctx.bridge.capabilities.scale0, fluxMock?.capabilities?.scale0]) {
        sc?.setToggle?.('absorbing_boundary', isFluxGravity);
        sc?.setToggle?.('latency_field', isFluxGravity || isMassGravity);
        sc?.setToggle?.('field_energy_gravity', isFluxGravity);
        // Mass-gravity body is a STATIC pre-seeded rest mass: disable genesis so the
        // body's self-field (Gauss flux) cannot trigger runaway manifestation.
        if (isMassGravity) sc?.setToggle?.('genesis', false);
    }

    setCurrentScenarioId(scenario.id);
    setSelectedScenarioId(scenario.id);
    markScenarioOverrideRows(DEFAULT_TOGGLES);
    syncComboSliders(ctx.bridge);
    state.latticeNeedsUpload = true;

    // Restore the captured overlay preferences. Runs last so it overrides any
    // defaults applied by applyAuxiliaryDefaults or resetScale0VisualState.
    restoreOverlayPreferences(overlayPrefs, state, viewportAdapter);
    applyScenarioVisualProfile(ctx, state, viewportAdapter, scenario.id);

    state.fieldNeedsUpdate = true;
    recomputeAnyFieldActive();
}

export async function resizeScale0Lattice(ctx, state, viewportAdapter, newSize) {
    const scenarioId = state.currentScenarioId || readInputValue('scenario-select', 'flux-pulse');
    const bridge = ctx.bridge;
    // The resize guard estimates the heap of the bridge that ACTUALLY owns this
    // scenario — the two owners have very different per-voxel costs:
    //   - flux-*/s0-* scenarios run on the JS MockBridge (state.fluxMock): a
    //     handful of N³ typed arrays (flux J + wave-vel Float64×3, |J|, state,
    //     masks) ≈ 150 bytes/voxel, bounded by the JS tab heap. Crucially the
    //     C++ RenderBridge is NOT reallocated on a flux-* resize — this function
    //     only sets bridge.latticeSize and builds a fresh MockBridge(newSize) —
    //     so the C++ 1300 B/voxel cost is irrelevant and must NOT gate flux-*.
    //     (Pre-fix this branch wrongly used 1300 B/voxel + a 2 GB cap, refusing
    //     big flux-* lattices over memory that is never allocated.)
    //   - empty/light/quantum run on the compiled C++ engine ≈ 1300 bytes/voxel
    //     (Voxel + SU(2)/SU(3) link structures), bounded by the WASM heap:
    //     8 GB on the Memory64 (wasm64) build, 2 GB on the wasm32 fallback.
    const ownerIsMock = shouldUseFluxMock(bridge, scenarioId);
    const bytesPerVoxel = ownerIsMock ? 150 : 1300;
    const capGB = ownerIsMock ? 2 : (bridge?.isWasm64 ? 8 : 2);
    const projectedBytes = Math.ceil(newSize ** 3 * bytesPerVoxel);
    const maxBytes = capGB * 1024 * 1024 * 1024;

    if (projectedBytes >= maxBytes) {
        const projGB = (projectedBytes / 1024 / 1024 / 1024).toFixed(2);
        const owner = ownerIsMock ? 'JS' : 'WASM';
        const msg = `L=${newSize} would need ~${projGB} GB of ${owner} heap (max ${capGB} GB here). Refusing to resize.`;
        if (typeof window.showToast === 'function') window.showToast(msg, 'error');
        else console.warn('[Scale0] ' + msg);
        setInputValue('lattice-size', bridge.latticeSize || 33);
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
    telemetryHub.resetScale(0);
    getScale0Scenario(scenarioId).load(getPhysicsHarness(bridge), { id: scenarioId });
    ctx.viewport.setLatticeSize(newSize);
    viewportAdapter.setFluxVolumeVisible(ctx.viewport.showFlux);

    if (bridge && bridge.capabilities && bridge.capabilities.scale0) {
        if (typeof bridge.capabilities.scale0.setBoundaryShape === 'function') {
            bridge.capabilities.scale0.setBoundaryShape(boundaryShapeFor(scenarioId));
        }
        if (typeof bridge.capabilities.scale0.setReflectiveBoundary === 'function') {
            bridge.capabilities.scale0.setReflectiveBoundary(reflectiveFor(scenarioId));
        }
    }

    // Same lazy-allocation rule as loadScale0Scenario (ARC-2).
    const useFluxMock = shouldUseFluxMock(bridge, scenarioId);
    let fluxMock = null;
    if (useFluxMock) {
        fluxMock = makeFluxMock(newSize, scenarioId, bridge);
        fluxMock.capabilities.scale0.setBoundaryShape(boundaryShapeFor(scenarioId));
        fluxMock.capabilities.scale0.setReflectiveBoundary(reflectiveFor(scenarioId));
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
