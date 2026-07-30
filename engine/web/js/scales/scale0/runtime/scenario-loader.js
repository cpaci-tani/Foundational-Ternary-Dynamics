import { runScale0PhysicsTicks } from './tick.js';
import { getPhysicsHarness } from '../../../physics/index.js';
import { WasmBridgeProxy } from '../../../bridge/wasm-bridge-proxy.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import { K_B, G_N, DAMPING, K_GENESIS } from '../../../constants.js';
import { SCALE0_TOGGLES, SCALE0_SCENARIO_OVERRIDES, LIGHT_SCENARIO_OVERRIDES, SCALE0_SCENARIO_BOUNDARY, SCALE0_ABSORBING_SCENARIOS, SCALE0_MASS_GRAVITY_SCENARIOS } from '../../../config/toggles.js';
import { getScale0Scenario } from '../scenario-registry.js';
import { mountGenesisBurstPanel } from '../ui/overlays/genesis-burst-panel.js';
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
    getScale0State,
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
import { applyScale0OverlayApplicability } from '../ui/overlays/applicability.js';

// Toggle-reset whitelist used by `applyToggleDefaults`.
//
// Contract: every scenario in `bridge/scenarios/*.js` and
// `engine/src/scenarios/*.cpp` MUST only mutate toggle keys that
// appear in `SCALE0_TOGGLES`. Keys outside the whitelist (e.g.
// `pair_production`, `langevin`, `latency_field`, `emergent_forces`)
// are intentionally NOT reset between scenarios — they are long-term
// research controls owned by the user, not scenario state.
//
// The scenario implementations now exercise the full production-facing
// whitelist (wave, coupling, damping, genesis/evaporation, projection,
// forces, movement, and selected interaction terms). If you add a scenario
// that needs to flip a non-whitelisted toggle,
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
    's0-seed-dynamical-flux-dressing': {
        // Show the manifested source, generated divergence, and integral
        // curves together. The curves visualize J; they are not extra strings.
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.8,
        fluxThreshold: 0.0002,
        fluxOpacity: 0.9,
        fieldOverlays: ['toggle-flux-lines', 'toggle-state-field', 'toggle-div-field'],
    },
    's0-seed-moving-source-reciprocity': {
        // Separate what the eye otherwise conflates: J geometry, the ternary
        // source, -wave_vel field change, and Poynting-like flow. FTD-0477
        // found only a sub-voxel response, so the lattice marker does not hop.
        // None of these overlays is a stored trajectory or radiation proof.
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.5,
        fluxThreshold: 0.0005,
        fluxOpacity: 0.72,
        fieldOverlays: [
            'toggle-flux-lines',
            'toggle-state-field',
            'toggle-e-field',
            'toggle-poynting',
        ],
    },
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

function applyAuxiliaryDefaults(ctx, viewportAdapter, scenarioId, { resetSpeed = true } = {}) {
    // Scenario loads snap the transport to 1× (slider=50). Lattice resize
    // keeps the user's current speed — only boundary / flux-volume defaults
    // need re-applying there.
    if (resetSpeed) ctx.applyTicksPerFrameFromSlider(50);
    // Boundary: a scenario may pin its own (SCALE0_SCENARIO_BOUNDARY); otherwise
    // reset to the default cube + non-reflective. Without honoring the config
    // here, this step would clobber the reflective boundary the scenario needs
    // (it runs after the flux-mock boundary is set). flux-zero-point relies on
    // this to keep its energy trapped → persistent floor.
    const bnd = SCALE0_SCENARIO_BOUNDARY[scenarioId] || {};
    const mode = bnd.mode ?? (bnd.reflective === true ? 1 : 2);
    ctx.applyBoundaryShape(bnd.shape ?? 'cube');
    ctx.applyFluxBoundaryMode(mode);
    // Particle rendering still consumes the legacy reflective flag. It must
    // describe only mode 1; periodic flux is not a reflective particle wall.
    ctx.viewport?.setReflectiveBoundary?.(mode === 1);
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
        //   pair_production      is independent of genesis (both apply their
        //                        own state==0 and K_GENESIS gate)
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
 * Repaint the physics-toggles card and overlay applicability from ENGINE state.
 *
 * `scenario.load` → `setupScenario` → `reset()` reconstructs the RenderBridge at
 * C++ defaults, and each `configure_*_terms` helper then zeroes every
 * TOGGLE_SPECS entry before setting its own profile. Everything applied before
 * that point — `applyToggleDefaults`, `SCALE0_SCENARIO_OVERRIDES` — therefore
 * describes what the dashboard REQUESTED, not what the engine is running. Left
 * unreconciled, the checkbox card asserts engine state the engine does not have
 * and `applicability.js` offers overlay channels the profile cannot populate.
 *
 * Returns false when the active bridge cannot answer authoritatively yet: the
 * worker publishes its readback asynchronously, so the proxy re-invokes this
 * from its `onEngineToggles` callback once the first real frame lands.
 */
export function syncScale0ToggleUiFromEngine(ctx, viewportAdapter, scenarioId) {
    const bridge = (ctx.useFluxMock && ctx.fluxMock) ? ctx.fluxMock : ctx.bridge;
    if (!bridge || typeof bridge.getToggle !== 'function') return false;
    if (bridge.isWorker && !bridge.hasEngineToggles) return false;

    const terms = {};
    for (const [key, , elId] of DEFAULT_TOGGLES) {
        const value = !!bridge.getToggle(key);
        terms[key] = value;
        setCheckboxValue(elId, value);
    }
    markScenarioOverrideRows(DEFAULT_TOGGLES);
    applyScale0OverlayApplicability(scenarioId, viewportAdapter, terms);
    return true;
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

/**
 * Worker-init failure recovery: switch Scale-0 from the (dead) off-thread
 * WasmBridgeProxy to the in-thread WasmBridge and re-run the scenario load.
 *
 * Wired to WasmBridgeProxy's onInitFailure. The proxy has already terminated
 * itself by the time we get here. We:
 *   1. latch ctx._wasmWorkerDisabled so subsequent loads/resizes skip the worker
 *      path entirely (no point retrying a broken environment),
 *   2. clear the flux-mock so getActiveScale0Bridge() resolves to ctx.bridge
 *      (the in-thread WasmBridge, which loads ftd_core.wasm and ticks fine), and
 *   3. re-run loadScale0Scenario — now it takes the in-thread branch and seeds
 *      the engine on ctx.bridge so the panel (and everything) works.
 *
 * Idempotent: the latch means a second failure callback (if any) is a no-op.
 */
function fallbackToInThreadEngine(ctx, state, viewportAdapter, scenarioId, params) {
    if (ctx._wasmWorkerDisabled) return;     // already fell back once
    ctx._wasmWorkerDisabled = true;
    setFluxMock(null, false);                // disposes the dead proxy; useFluxMock=false
    ctx.useFluxMock = false;
    ctx.fluxMock = null;
    try {
        loadScale0Scenario(ctx, state, viewportAdapter, scenarioId, params);
    } catch (e) {
        console.error('[Scale0] in-thread fallback load failed:', e);
    }
}

export function loadScale0Scenario(ctx, state, viewportAdapter, scenarioId, params = {}, opts = {}) {
    const { resetSpeed = true, resetTickAccumulator = false } = opts;
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
    if (wasmWorkerEligible(scenario.id, ctx.bridge) && !ctx._wasmWorkerDisabled) {
        // Off-thread WASM engine (Phase 1): host the real C++ physics in a Web Worker.
        // If the worker fails to initialise (e.g. importScripts NetworkError on the
        // -pthread MT glue), onInitFailure fires once: we disable the worker path for
        // this ctx and re-run the load on the in-thread WasmBridge so the engine never
        // silently dies (the proxy is the ACTIVE bridge while useFluxMock=true, so a
        // dead worker means NOTHING ticks).
        fluxMock = new WasmBridgeProxy(latticeSize, {
            onInitFailure: () => fallbackToInThreadEngine(ctx, state, viewportAdapter, scenarioId, params),
            // The worker owns the truth about which terms the C++ body left live.
            // Reconcile the UI to it whenever it republishes, against the CURRENT
            // scenario id — a late frame from a superseded load must not repaint
            // the card for a scenario the user already switched away from.
            onEngineToggles: () => {
                const activeId = getScale0State().currentScenarioId;
                if (activeId) syncScale0ToggleUiFromEngine(ctx, viewportAdapter, activeId);
            },
        });
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

    // Load the scenario IMMEDIATELY on selection, whether or not the run loop is
    // ticking. Previously a paused selection was deferred into
    // state._pendingScenarioLoad and only fired on the first tick after Play, so
    // picking a scenario while paused appeared to do nothing until Play. Loading
    // here means the lattice is seeded + displayed right away; applyAuxiliaryDefaults
    // below still runs AFTER scenario.load (the worker-path _pendingCommands replay
    // ordering documented there is preserved).
    scenario.load(harness, params);

    // Scenario-owned observatory panels are mounted from the same canonical
    // load path as their physics. The genesis panel used to be orphaned: its
    // module existed and tests expected it, but no production code mounted it.
    // Dispose eagerly on every switch so hidden research scenarios cannot
    // leave stale controls attached to a different active bridge.
    if (typeof window !== 'undefined' && window.__ftdGenesisBurstPanel) {
        window.__ftdGenesisBurstPanel.dispose?.();
    }
    if (scenario.id === 's0-seed-cluster-law') {
        mountGenesisBurstPanel(harness);
    }

    // applyAuxiliaryDefaults runs AFTER scenario.load so that the flux boundary
    // mode command isn't discarded. On the worker path, scenario.load calls
    // setupScenario which sends { type:'create' } to the worker AND clears
    // _pendingCommands. Any command queued before that call is wiped before the
    // worker's 'ready' reply can replay it. By running here, the setFluxBoundary
    // command lands in _pendingCommands after the clear and is replayed correctly.
    // If scenario load is deferred, we still apply defaults now to ready the UI.
    applyAuxiliaryDefaults(ctx, viewportAdapter, scenario.id, { resetSpeed });

    // Gravity/wave family (SCALE0_ABSORBING_SCENARIOS): set LAST, after
    // scenario.load (which resets the engine toggles under the dual-bridge
    // routing) so these actually reach the running bridge. Two things:
    //   • absorbing_boundary — imposed D-deep quadratic damping at the faces
    //     (render_bridge.cpp Rule 5b). It is not a derived radiation condition.
    //     Scoped here so other scenarios keep
    //     their full flux volume (enabling it broadly collapsed volumes to slabs).
    //   • latency_field — run the REAL latency Poisson so the gravity panel shows
    //     the genuine C++ potential, not only the |J|² proxy. Source is the
    //     [IMPOSED] field-energy density (field_energy_gravity) for the FLUX
    //     scenarios, OR imposed manifested gravity charge M_GRAVITATIONAL·|state| for the
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
    // Prefer engine truth. The in-thread bridge answers synchronously here; the
    // worker path cannot yet, so it falls back to the JS model for this frame and
    // the proxy's onEngineToggles callback corrects both card and mask on arrival.
    if (!syncScale0ToggleUiFromEngine(ctx, viewportAdapter, scenario.id)) {
        applyScale0OverlayApplicability(scenario.id, viewportAdapter);
    }

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

    // Prime tick on load: when enabled (play-bar toggle, persisted via
    // state.primeTickOnLoad), advance exactly one physics tick right after the
    // scenario is seeded so motion-derived overlays (E/B/Poynting/vorticity) and
    // particle/manifestation overlays (state/forces/…) have data to render at the
    // initial paused view instead of staying blank until the user presses Play.
    // One tick on a freshly-seeded field is visually ~identical to tick 0. Uses
    // the existing single-tick path (worker: tickOnce, replayed when the worker is
    // ready; in-thread: tickScale0), which bumps fieldDataVersion so the overlay
    // sweep renders the primed state. The C++ golden sequence is a separate fixed
    // loop and is unaffected by this UI-driven tick.
    // window.__ftdPrimeTickOnLoad (boolean) is a live override for tests / the
    // overlay-audit harness, letting them force true tick-0 (false) or primed
    // (true) regardless of the persisted user toggle. Falls back to the toggle.
    const primeOnLoad = (typeof window !== 'undefined' && typeof window.__ftdPrimeTickOnLoad === 'boolean')
        ? window.__ftdPrimeTickOnLoad
        : state.primeTickOnLoad;
    if (primeOnLoad) {
        try {
            runScale0PhysicsTicks(ctx, state, 1);
        } catch (e) {
            console.warn('[Scale0] prime tick on load failed:', e);
        }
        markFieldDirty();
    }

    if (resetTickAccumulator) state.tickAccumulator.reset();
}

export async function resizeScale0Lattice(ctx, state, viewportAdapter, newSize) {
    const scenarioId = state.currentScenarioId || readInputValue('scenario-select', 'flux-pulse');
    const bridge = ctx.bridge;
    // The resize guard estimates the WASM heap cost: ≈1300 bytes/voxel
    // (Voxel + SU(2)/SU(3) link structures). Bounded by 8 GB on the wasm64
    // build or 2 GB on the wasm32 fallback.
    const useWasmWorker = wasmWorkerEligible(scenarioId, bridge) && !ctx._wasmWorkerDisabled;
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
    // (loadScale0Scenario builds a fresh WasmBridgeProxy at the new size).
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
    // Point the app-level bridge at the new N so the canonical load path
    // constructs the worker / reset() at this size. Do NOT fork a second
    // install path here — the old resize body skipped onInitFailure,
    // onEngineToggles, applyAuxiliaryDefaults (dual-bridge boundary), and
    // toggle UI reconciliation.
    bridge.latticeSize = newSize;

    loadScale0Scenario(ctx, state, viewportAdapter, scenarioId, { id: scenarioId }, {
        resetSpeed: false,
        resetTickAccumulator: true,
    });
    setInputValue('lattice-size', getActiveScale0Bridge(ctx, state)?.latticeSize || newSize);
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
