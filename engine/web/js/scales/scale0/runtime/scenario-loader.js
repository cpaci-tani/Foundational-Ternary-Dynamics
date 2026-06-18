import { runScale0PhysicsTicks } from './tick.js';
import { getPhysicsHarness } from '../../../physics/index.js';
import { WasmBridgeProxy } from '../../../bridge/wasm-bridge-proxy.js';
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
    getActiveScale0Bridge,
    getActiveLatticeSize,
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
const SCALE0_SCENARIO_VISUAL_PROFILES = {
    'flux-vortex': {
        // E Field streamlines make the vortex ring's curl structure immediately visible.
        fieldOverlays: ['toggle-e-field'],
    },
    's0-field-spacetime-forcing-boundary': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxThreshold: 0.001,
        fluxOpacity: 0.85,
    },
    's0-field-rf-lattice-wave': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxThreshold: 0.0005,
        fluxOpacity: 0.85,
    },
    's0-field-light-lattice-wave': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxThreshold: 0.0005,
        fluxOpacity: 0.85,
    },
    's0-field-sound-lattice-wave': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxThreshold: 0.0005,
        fluxOpacity: 0.85,
    },
    's0-field-thomson-scattering': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.4,
        fluxThreshold: 0.001,
        fluxOpacity: 0.85,
    },
    's0-field-thomson-unlocked-recoil': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.4,
        fluxThreshold: 0.001,
        fluxOpacity: 0.85,
    },
};

// WASM-engine worker hosting (Phase 1): for WASM-OWNED scenarios (NOT flux-*/
// mock-owned — i.e. the ones that would otherwise run on the main-thread WASM
// bridge, e.g. s0-seed-hydrogen) host the real C++ engine in a Web Worker
// (WasmBridgeProxy) so the heavy tick never stalls the render thread. Requires
// COI + SAB + a primary WASM bridge. window.__ftdWasmWorker = false forces the
// in-thread WASM path (tests/fallback).
const FTD_WASM_WORKER = (typeof window !== 'undefined' && window.__ftdWasmWorker !== undefined)
    ? !!window.__ftdWasmWorker
    : true;
function wasmWorkerEligible(scenarioId, bridge) {
    return FTD_WASM_WORKER
        && typeof SharedArrayBuffer !== 'undefined'
        && globalThis.crossOriginIsolated === true
        && !!(bridge && bridge.isWasm);
}

function syncComboSliders(ctx, state) {
    const defaults = { kb: K_B, gn: G_N, damping: DAMPING };
    const activeBridge = getActiveScale0Bridge(ctx, state) ?? ctx?.bridge;
    const map = [
        { id: 'combo-kb', valId: 'combo-kb-val', param: 'kb', fmt: 3 },
        { id: 'combo-gn', valId: 'combo-gn-val', param: 'gn', fmt: 3 },
        { id: 'combo-damp', valId: 'combo-damp-val', param: 'damping', fmt: 3 },
    ];
    for (const slider of map) {
        const el = document.getElementById(slider.id);
        const display = document.getElementById(slider.valId);
        if (!el || !display) continue;
        const value = activeBridge?.getParam ? activeBridge.getParam(slider.param) : defaults[slider.param];
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

function applyScenarioVisualProfile(ctx, state, viewportAdapter, scenarioId, prefs) {
    const profile = SCALE0_SCENARIO_VISUAL_PROFILES[scenarioId];
    if (!profile || !ctx?.viewport) return;

    if (profile.fluxVolume === true && prefs?.fluxVolume !== false) {
        viewportAdapter.setFluxVolumeVisible(true);
        setButtonActive('toggle-flux-volume', true);
    }
    if (profile.fluxSlice === true && prefs?.fluxSlice !== false) {
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

    if (Array.isArray(profile.fieldOverlays)) {
        for (const btnId of profile.fieldOverlays) {
            const flagKey = FIELD_BUTTON_TO_FLAG[btnId];
            setButtonActive(btnId, true);
            if (flagKey) {
                setFieldToggle(flagKey, true);
                viewportAdapter.setOverlayVisible(flagKey, true);
            }
        }
    }

    state.latticeNeedsUpload = true;
    markFieldDirty();
}

function applyGravityAbsorbingToggles(scenarioId, mainScale0, mockScale0) {
    const isFluxGravity = SCALE0_ABSORBING_SCENARIOS.has(scenarioId);
    const isMassGravity = SCALE0_MASS_GRAVITY_SCENARIOS.has(scenarioId);
    for (const sc of [mainScale0, mockScale0]) {
        if (!sc) continue;
        sc.setToggle?.('absorbing_boundary', isFluxGravity);
        sc.setToggle?.('latency_field', isFluxGravity || isMassGravity);
        sc.setToggle?.('field_energy_gravity', isFluxGravity);
        if (isMassGravity) sc.setToggle?.('genesis', false);
    }
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

    const latticeSize = ctx.bridge.latticeSize || 33;
    let useFluxMock = false;
    let fluxMock = null;
    if (wasmWorkerEligible(scenario.id, ctx.bridge)) {
        // Off-thread WASM engine (Phase 1): host the real C++ physics in a Web Worker.
        fluxMock = new WasmBridgeProxy(latticeSize);
        useFluxMock = true;
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

    const activeBridge = (useFluxMock && fluxMock) ? fluxMock : ctx.bridge;
    const harness = getPhysicsHarness(activeBridge);
    scenario.load(harness, params);

    // applyAuxiliaryDefaults runs AFTER scenario.load so that the flux boundary
    // mode command isn't discarded. On the worker path, scenario.load calls
    // setupScenario which sends { type:'create' } to the worker AND clears
    // _pendingCommands. Any command queued before that call is wiped before the
    // worker's 'ready' reply can replay it. By running here, the setFluxBoundary
    // command lands in _pendingCommands after the clear and is replayed correctly.
    applyAuxiliaryDefaults(ctx, viewportAdapter, scenario.id);

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
    applyGravityAbsorbingToggles(
        scenario.id,
        ctx.bridge.capabilities.scale0,
        fluxMock?.capabilities?.scale0 ?? null,
    );

    setCurrentScenarioId(scenario.id);
    setSelectedScenarioId(scenario.id);
    markScenarioOverrideRows(DEFAULT_TOGGLES);
    syncComboSliders(ctx, state);
    state.latticeNeedsUpload = true;

    // Restore the captured overlay preferences. Runs last so it overrides any
    // defaults applied by applyAuxiliaryDefaults or resetScale0VisualState.
    restoreOverlayPreferences(overlayPrefs, state, viewportAdapter);
    applyScenarioVisualProfile(ctx, state, viewportAdapter, scenario.id, overlayPrefs);

    // Keep viewport world coords (wireframe, clip, streamlines) aligned with
    // the bridge that owns physics — resize already does this; load must too.
    const activeN = activeBridge.latticeSize || latticeSize;
    if (ctx.viewport?.latticeSize !== activeN) {
        ctx.viewport.setLatticeSize(activeN);
    }

    state.fieldNeedsUpdate = true;
    recomputeAnyFieldActive();
    // Signal that the first worker frame with real sampler data should trigger
    // a forced overlay repaint even when ctx.running=true (see controller.js
    // onBridgePostFrame). Without this the rAF that consumes fieldNeedsUpdate
    // fires with an empty _samplerCache and overlays stay blank until the next
    // tick increments fieldDataVersion again.
    if (ctx) ctx._samplersPending = true;
}

export async function resizeScale0Lattice(ctx, state, viewportAdapter, newSize) {
    const scenarioId = state.currentScenarioId || readInputValue('scenario-select', 'flux-pulse');
    const bridge = ctx.bridge;
    // The resize guard estimates the WASM heap cost: ≈1300 bytes/voxel
    // (Voxel + SU(2)/SU(3) link structures). Bounded by 8 GB on the wasm64
    // build or 2 GB on the wasm32 fallback.
    const useWasmWorker = wasmWorkerEligible(scenarioId, bridge);
    const bytesPerVoxel = 1300;
    const capGB = bridge?.isWasm64 ? 8 : 2;
    const projectedBytes = Math.ceil(newSize ** 3 * bytesPerVoxel);
    const maxBytes = capGB * 1024 * 1024 * 1024;

    if (projectedBytes >= maxBytes) {
        const projGB = (projectedBytes / 1024 / 1024 / 1024).toFixed(2);
        const msg = `L=${newSize} would need ~${projGB} GB of WASM heap (max ${capGB} GB here). Refusing to resize.`;
        if (typeof window.showToast === 'function') window.showToast(msg, 'error');
        else console.warn('[Scale0] ' + msg);
        setInputValue('lattice-size', bridge.latticeSize || 33);
        return;
    }

    // Skip the in-thread WASM realloc when the off-thread worker owns physics
    // (the WasmBridgeProxy rebuilds the engine at newSize inside the worker).
    if (!useWasmWorker && bridge && typeof bridge.resize === 'function') {
        try {
            await bridge.resize(newSize);
        } catch (e) {
            console.error('[Scale0] Failed to resize simulation lattice:', e);
            if (typeof window.showToast === 'function') {
                window.showToast(`Lattice resize failed: ${e.message}`, 'error');
            }
            setInputValue('lattice-size', bridge.latticeSize || 33);
            return;
        }
    }
    bridge.latticeSize = newSize;

    telemetryHub.resetScale(0);

    let useFluxMock = false;
    let fluxMock = null;
    if (useWasmWorker) {
        fluxMock = new WasmBridgeProxy(newSize);   // off-thread WASM at the new size
        useFluxMock = true;
    }

    applyToggleDefaults(ctx.bridge.capabilities.scale0, fluxMock?.capabilities?.scale0 ?? null, scenarioId);
    if (fluxMock) {
        for (const [key, , elId] of DEFAULT_TOGGLES) {
            fluxMock.capabilities.scale0.setToggle(key, readCheckboxValue(elId));
        }
    }

    setFluxMock(fluxMock, useFluxMock);
    ctx.useFluxMock = useFluxMock;
    ctx.fluxMock = fluxMock;

    const activeBridge = (useFluxMock && fluxMock) ? fluxMock : bridge;
    const harness = getPhysicsHarness(activeBridge);
    getScale0Scenario(scenarioId).load(harness, { id: scenarioId });

    applyGravityAbsorbingToggles(
        scenarioId,
        bridge.capabilities.scale0,
        fluxMock?.capabilities?.scale0 ?? null,
    );

    if (bridge?.capabilities?.scale0) {
        bridge.capabilities.scale0.setBoundaryShape(boundaryShapeFor(scenarioId));
        bridge.capabilities.scale0.setReflectiveBoundary(reflectiveFor(scenarioId));
    }

    ctx.viewport.setLatticeSize(newSize);
    viewportAdapter.setFluxVolumeVisible(ctx.viewport.showFlux);
    setInputValue('lattice-size', activeBridge.latticeSize || newSize);
    state.latticeNeedsUpload = true;
    markFieldDirty();
    state.tickAccumulator.reset();
    if (ctx) ctx._samplersPending = true;
}

export function stepScale0(ctx, state) {
    runScale0PhysicsTicks(ctx, state, 1);
    state.fieldNeedsUpdate = true;
}

export function resetScale0Scenario(ctx, state, viewportAdapter) {
    loadScale0Scenario(ctx, state, viewportAdapter, state.currentScenarioId || 'flux-pulse');
}

export function exitScale0() {
    clearFluxMock();
}

export { K_GENESIS };
