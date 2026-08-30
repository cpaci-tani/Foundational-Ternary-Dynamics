import { runScale0PhysicsTicks } from './tick.js';
import { getPhysicsHarness } from '../../../physics/index.js';
import { WasmBridgeProxy } from '../../../bridge/wasm-bridge-proxy.js?v=3';
import { telemetryHub } from '../../../telemetry-hub.js';
import { K_B, G_N, DAMPING, K_GENESIS } from '../../../constants.js';
import {
    SCALE0_TOGGLES,
    SCALE0_ADVANCED_TOGGLES,
    SCALE0_SCENARIO_OVERRIDES,
    LIGHT_SCENARIO_OVERRIDES,
    SCALE0_SCENARIO_BOUNDARY,
    SCALE0_ABSORBING_SCENARIOS,
    SCALE0_MASS_GRAVITY_SCENARIOS,
    SCALE0_SCENARIO_RESEARCH_TERMS,
} from '../../../config/toggles.js';
import { getScale0Scenario } from '../scenario-registry.js';
import { mountGenesisBurstPanel } from '../ui/overlays/genesis-burst-panel.js?v=2';
import { forEachKnotTracker } from './field-line-knots.js';
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
    setScalarRenderMode,
    getActiveScale0Bridge,
    getScale0State,
    beginScale0AuthoritativeLoad,
    completeScale0AuthoritativeLoad,
    failScale0AuthoritativeLoad,
} from '../state/store.js';
import {
    FIELD_TOGGLE_BINDINGS,
    getEl,
    markScenarioOverrideRows,
    readButtonActive,
    readInputValue,
    setButtonActive,
    setCheckboxValue,
    setForceStyleButtons,
    setScalarRenderButtons,
    setInputValue,
    setSelectedScenarioId,
} from '../ui/dom.js?v=3';
import { applyScale0OverlayApplicability } from '../ui/overlays/applicability.js?v=4';
import { MAX_WASM_INTERACTIVE_LATTICE } from '../ui/toolbar/limits.js';

// Toggle-reset whitelist used by `applyToggleDefaults`.
//
// Contract (aligned with engine/include/ftd/scenarios.h):
//   • SCALE0_TOGGLES is the UI-visible subset the loader resets every load.
//   • Isolation helpers (configure_*_terms) MAY zero the full TermToggles
//     registry, including research keys — required for certified ICs.
//   • Research keys outside the whitelist normally persist across loads unless
//     a configure_* helper clears them, or SCALE0_SCENARIO_RESEARCH_TERMS pins
//     them for a specific scenario (applied after the whitelist reset).
const DEFAULT_TOGGLES = SCALE0_TOGGLES;
const PHYSICS_UI_TOGGLES = [...SCALE0_TOGGLES, ...SCALE0_ADVANCED_TOGGLES];

function setPhysicsToggleCardPending(pending) {
    const card = getEl('physics-profile-warning')?.closest('.card');
    if (!card) return;
    const busyValue = pending ? 'true' : 'false';
    if (card.getAttribute('aria-busy') !== busyValue) {
        card.setAttribute('aria-busy', busyValue);
    }
    for (const input of card.querySelectorAll('input[type="checkbox"]')) {
        if (pending) {
            if (!input.disabled) {
                input.disabled = true;
                input.dataset.scale0PendingDisabled = '1';
            }
        } else if (input.dataset.scale0PendingDisabled === '1') {
            input.disabled = false;
            delete input.dataset.scale0PendingDisabled;
        }
    }
}

function reportScenarioSetupFailure(msg) {
    console.error('[Scale0]', msg);
    if (typeof window !== 'undefined' && typeof window.showToast === 'function') {
        window.showToast(msg, 'error');
    }
}

function readAuthoritativeTick(bridge) {
    try {
        const tick = Number(bridge?.getDiagnostics?.()?.tick);
        return Number.isFinite(tick) ? tick : null;
    } catch {
        return null;
    }
}
// Round-trip the user's overlay preferences across scenario switches. Both maps
// are DERIVED from the canonical button↔flag list in ui/dom.js
// (FIELD_TOGGLE_BINDINGS, shared with bindings.js) so this can never drift behind
// it again — the old hand-maintained mirror had silently fallen 4 overlays behind
// (the 2026-06-03 substrate overlays: state-field / latency / gauss-residual /
// moore-decomp were missing). B1 fix, 2026-06-05.
const FIELD_BUTTON_IDS = FIELD_TOGGLE_BINDINGS.map(([id]) => id);
const FIELD_BUTTON_TO_FLAG = Object.fromEntries(FIELD_TOGGLE_BINDINGS);
const COMPACT_SEED_FOCUS = Object.freeze({ focusRadius: 5, focusMinL: 65 });
export const SCALE0_SCENARIO_VISUAL_PROFILES = {
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
        // This is imposed circulating J geometry. Show J integral curves plus
        // both honest curl views: B = curl(J) and scalar vorticity |curl(J)|.
        // E = -wave_vel is initially zero here and made the native scene look
        // blank, so it is deliberately not the default channel.
        fieldOverlays: ['toggle-b-field', 'toggle-vorticity', 'toggle-flux-lines'],
    },
    's0-field-uniform-e': {
        // The scenario is an inert canonical-momentum field with nonzero
        // E-proxy (-wave_vel). Make that populated channel visible by default.
        fieldOverlays: ['toggle-e-field'],
    },
    's0-field-uniform-b': {
        // J is the vector-potential ansatz and grows radially; the promised
        // uniform observable is B=curl(J). Lead with B and suppress the default
        // J-magnitude cloud so the native scene does not imply J itself is the
        // uniform magnetic field. Users can still re-enable the volume.
        fluxVolume: false,
        fieldOverlays: ['toggle-b-field'],
    },
    's0-field-electric-dipole': {
        // Softened opposite-source J peaks below the global 0.005 display
        // cutoff. A scenario-local visibility threshold reveals the imposed
        // field without changing any engine physics.
        fluxVolume: true,
        fluxThreshold: 0.0001,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
        fieldOverlays: ['toggle-flux-lines'],
    },
    's0-field-magnetic-dipole': {
        // The imposed quantity is a vector potential; B=curl(J) is the honest
        // magnetic-dipole view. Compact large-L samples are also below 0.005.
        fluxVolume: true,
        fluxThreshold: 0.0001,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
        fieldOverlays: ['toggle-b-field', 'toggle-flux-lines'],
    },
    's0-seed-schwarzschild': {
        // This is an inert inverse-square J ansatz (not a live horizon or
        // latency solution). Keep those absent overlays off and reveal J.
        fluxVolume: true,
        fluxThreshold: 0.0001,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
        fieldOverlays: ['toggle-flux-lines'],
    },
    's0-seed-time-horizon': {
        // Exact alias of the inert radial ansatz. Do not imply that the absent
        // latency/horizon channels are computed by turning them on by default.
        fluxVolume: true,
        fluxThreshold: 0.0001,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
        fieldOverlays: ['toggle-flux-lines'],
    },
    's0-seed-wilson-loop': {
        ...COMPACT_SEED_FOCUS,
        // The native seed is an oriented square of J with radius L/8 and no
        // ternary matter. Native sparse samples cannot support streamline
        // integration through omitted zero neighbors, so lead with the exact
        // sampled support points and scale the camera envelope with L. Keep the
        // canonical point scale: live L=181 calibration shows 1.0 resolves the
        // discrete bins, while 0.3 vanishes and 3.0 merges them.
        focusRadiusFraction: 0.16,
        fluxVolume: true,
        fieldOverlays: [],
    },
    's0-seed-octahedron': { ...COMPACT_SEED_FOCUS },
    's0-seed-cuboctahedron': { ...COMPACT_SEED_FOCUS },
    's0-seed-stella-octangula': { ...COMPACT_SEED_FOCUS },
    's0-seed-moore-cell': { ...COMPACT_SEED_FOCUS },
    's0-seed-moore-decomposition': { ...COMPACT_SEED_FOCUS },
    's0-seed-observer-cell': { ...COMPACT_SEED_FOCUS },
    's0-seed-massive-body': {
        ...COMPACT_SEED_FOCUS,
        // J is exactly zero. The populated native views are the locked ternary
        // mass and the real latency-Poisson solution (FTS2 kind 17).
        fieldOverlays: ['toggle-state-field', 'toggle-latency'],
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
export const WASM32_LATTICE_CAP = 117;
function wasmWorkerEligible(scenarioId, bridge) {
    const N = (bridge?.latticeSize | 0);
    if (N > WASM32_LATTICE_CAP) return false;
    return FTD_WASM_WORKER
        && typeof SharedArrayBuffer !== 'undefined'
        && globalThis.crossOriginIsolated === true
        && !!(bridge && bridge.isWasm);
}

export function syncComboSliders(ctx, state) {
    const defaults = { kb: K_B, gn: G_N, damping: DAMPING };
    const activeBridge = getActiveScale0Bridge(ctx, state) ?? ctx?.bridge;
    const map = [
        { id: 'combo-kb', valId: 'combo-kb-val', param: 'kb', fmt: 3 },
        { id: 'combo-gn', valId: 'combo-gn-val', param: 'gn', fmt: 3 },
        { id: 'combo-damp', valId: 'combo-damp-val', param: 'damping', fmt: 4 },
    ];
    for (const slider of map) {
        const el = document.getElementById(slider.id);
        const display = document.getElementById(slider.valId);
        if (!el || !display) continue;
        const value = activeBridge?.getParam ? activeBridge.getParam(slider.param) : defaults[slider.param];
        // Engine constants are authoritative readouts, not user-selectable
        // range values. HTML range inputs quantize assignments to their
        // configured edit step, which made an acknowledged value such as
        // 0.5123 appear as 0.51 even while the text label showed 0.512.  Remove
        // that edit quantization before mirroring the read-only value.
        el.step = 'any';
        if (value != null) {
            const valueText = String(value);
            const displayText = value.toFixed(slider.fmt);
            if (el.value !== valueText) el.value = valueText;
            if (display.textContent !== displayText) display.textContent = displayText;
        }
        el.disabled = true;
        el.setAttribute('aria-readonly', 'true');
        el.setAttribute('aria-disabled', 'true');
        el.title = ctx?.bridge?.isNativeGPU
            ? `${slider.param} is read-only; value echoed by the native engine profile.`
            : `${slider.param} is a read-only compile-time engine constant.`;
        el.classList.add('ctrl-slider-disabled');
        el.closest('.pe-ctrl-row')?.classList.add('ctrl-native-readonly');
    }
}

function setDisplayText(id, text) {
    if (typeof document === 'undefined') return;
    const el = document.getElementById(id);
    if (el) el.textContent = text;
}

function formatFluxThreshold(value) {
    return value < 0.001 ? value.toFixed(4) : value.toFixed(3);
}

export function applyScenarioVisualProfile(ctx, state, viewportAdapter, scenarioId, prefs) {
    const profile = SCALE0_SCENARIO_VISUAL_PROFILES[scenarioId];
    if (!profile || !ctx?.viewport) return;

    const rememberParameterPreference = (key) => {
        if (!ctx._scale0ForcedVisualParameterPreferences) {
            ctx._scale0ForcedVisualParameterPreferences = {};
        }
        if (!(key in ctx._scale0ForcedVisualParameterPreferences)) {
            ctx._scale0ForcedVisualParameterPreferences[key] = prefs?.[key];
        }
    };

    if (profile.fluxVolume === false) {
        // Preserve the user's real preference behind this scenario-local
        // suppression. The next scenario load restores it rather than treating
        // uniform-B's canonical "show B, not A" presentation as a global user
        // choice. bindings.js updates this marker if the user explicitly
        // toggles the volume while the suppression is active.
        if (typeof ctx._scale0ForcedFluxVolumePreference !== 'boolean') {
            ctx._scale0ForcedFluxVolumePreference = prefs?.fluxVolume !== false;
        }
        viewportAdapter.setFluxVolumeVisible(false);
        setButtonActive('toggle-flux-volume', false);
    } else if (profile.fluxVolume === true && prefs?.fluxVolume !== false) {
        viewportAdapter.setFluxVolumeVisible(true);
        setButtonActive('toggle-flux-volume', true);
    }
    if (profile.fluxSlice === true && prefs?.fluxSlice !== false) {
        viewportAdapter.setFluxSliceVisible(true);
        setButtonActive('toggle-flux-slice', true);
    }
    if (typeof profile.fluxPointScale === 'number') {
        rememberParameterPreference('fluxPointScale');
        ctx.viewport.setFluxPointScale(profile.fluxPointScale);
        ctx.viewport.setFluxSlicePointScale?.(profile.fluxPointScale);
        setInputValue('flux-point-scale', profile.fluxPointScale);
        setDisplayText('flux-point-scale-val', profile.fluxPointScale.toFixed(1));
    }
    if (typeof profile.fluxThreshold === 'number') {
        rememberParameterPreference('fluxThreshold');
        ctx.viewport.setFluxThreshold(profile.fluxThreshold);
        ctx.viewport.setFluxSliceThreshold?.(profile.fluxThreshold);
        setInputValue('flux-threshold', profile.fluxThreshold);
        setDisplayText('flux-threshold-val', formatFluxThreshold(profile.fluxThreshold));
    }
    if (typeof profile.fluxOpacity === 'number') {
        rememberParameterPreference('fluxOpacity');
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

/**
 * Frame a compact or bounded center-seeded structure once when its scenario is
 * loaded. Fixed-radius profiles cover 3x3x3 constructions; an optional radius
 * fraction covers structures such as the Wilson square whose footprint scales
 * with L. This preserves the current orbit direction and is never called from
 * frame/readback callbacks, so manual camera movement remains untouched after
 * the initial load.
 */
export function applyScenarioCameraFocus(ctx, scenarioId, latticeSize, initialLoad = true) {
    const profile = SCALE0_SCENARIO_VISUAL_PROFILES[scenarioId];
    const N = Number(latticeSize);
    const fixedRadius = Number(profile?.focusRadius);
    const radiusFraction = Number(profile?.focusRadiusFraction);
    const radius = Math.max(
        Number.isFinite(fixedRadius) && fixedRadius > 0 ? fixedRadius : 0,
        Number.isFinite(radiusFraction) && radiusFraction > 0 && Number.isFinite(N)
            ? radiusFraction * N
            : 0,
    );
    const minL = Number(profile?.focusMinL) || 0;
    const viewport = ctx?.viewport;
    const camera = viewport?.camera;
    const controls = viewport?.controls;
    if (!initialLoad || !Number.isFinite(radius) || radius <= 0
        || !Number.isFinite(N) || N < minL
        || !camera?.position || !controls?.target) return false;

    const center = N / 2;
    let dx = Number(camera.position.x) - Number(controls.target.x);
    let dy = Number(camera.position.y) - Number(controls.target.y);
    let dz = Number(camera.position.z) - Number(controls.target.z);
    let length = Math.hypot(dx, dy, dz);
    if (!(length > 1e-9)) {
        dx = 0.25;
        dy = 0.15;
        dz = 1;
        length = Math.hypot(dx, dy, dz);
    }
    const fov = Math.max(10, Math.min(120, Number(camera.fov) || 60)) * Math.PI / 180;
    const unclampedDistance = radius * 1.35 / Math.tan(fov / 2);
    const minDistance = Number(controls.minDistance) || 0.01;
    const maxDistance = Number(controls.maxDistance) || 100000000;
    const distance = Math.max(minDistance, Math.min(maxDistance, unclampedDistance));
    const scale = distance / length;
    controls.target.set(center, center, center);
    camera.position.set(center + dx * scale, center + dy * scale, center + dz * scale);
    controls.update?.();
    return true;
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
    // Boundary: a scenario may pin its own (SCALE0_SCENARIO_BOUNDARY). When no
    // entry exists the loader defaults to dispersal (mode 2) — NOT the live
    // DOM boundary controls. Any body / configure_* that sets Periodic MUST
    // also register SCALE0_SCENARIO_BOUNDARY[id] = { mode: 0 }, or this step
    // (after scenario.load) sponges the seed.
    const bnd = SCALE0_SCENARIO_BOUNDARY[scenarioId] || {};
    const mode = bnd.mode ?? (bnd.reflective === true ? 1 : 2);
    ctx.applyBoundaryShape(bnd.shape ?? 'cube');
    ctx.applyFluxBoundaryMode(mode);
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

    // Research terms outside SCALE0_TOGGLES (no checkbox). Applied after the
    // whitelist so isolation profiles that enable langevin / ew_background_sweep
    // / pair_production / emergent_forces reach both bridges.
    const research = SCALE0_SCENARIO_RESEARCH_TERMS[scenarioName];
    if (research) {
        for (const [key, val] of Object.entries(research)) {
            mainScale0.setToggle?.(key, val);
            mockScale0?.setToggle?.(key, val);
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
 * from its frame callback and again at the explicit post-configuration barrier.
 */
export function syncScale0ToggleUiFromEngine(ctx, viewportAdapter, scenarioId) {
    // Legacy state fields: useFluxMock / fluxMock hold the WASM worker proxy.
    const bridge = (ctx.useFluxMock && ctx.fluxMock) ? ctx.fluxMock : ctx.bridge;
    if (!bridge || typeof bridge.getToggle !== 'function') return false;
    if (bridge.isWorker && !bridge.hasEngineToggles) return false;

    const terms = {};
    const generation = String(ctx._loadGeneration || 0);
    const captureBaseline = String(ctx._scale0ToggleBaselineGeneration ?? '') !== generation;
    for (const [key, , elId] of PHYSICS_UI_TOGGLES) {
        // The native `confinement` field is serialization intent only; no C++
        // phase consumes it. Keep the physics card false/disabled and leave the
        // separate viewport confinement overlay as the explicit visual proxy.
        const value = bridge.isNativeGPU && key === 'confinement'
            ? false
            : !!bridge.getToggle(key);
        terms[key] = value;
        setCheckboxValue(elId, value);
        const el = getEl(elId);
        if (el && captureBaseline) el.dataset.scale0ProfileValue = value ? '1' : '0';
    }
    if (captureBaseline) ctx._scale0ToggleBaselineGeneration = generation;
    markScenarioOverrideRows(PHYSICS_UI_TOGGLES);
    setPhysicsToggleCardPending(false);
    ctx.onScale0ToggleProfileSynced?.(scenarioId);
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
export function captureOverlayPreferences(state, ctx = null) {
    const overlays = {};
    for (const id of FIELD_BUTTON_IDS) {
        overlays[id] = readButtonActive(id);
    }
    const forcedFluxPreference = ctx?._scale0ForcedFluxVolumePreference;
    const forcedParameters = ctx?._scale0ForcedVisualParameterPreferences || {};
    if (ctx && typeof forcedFluxPreference === 'boolean') {
        delete ctx._scale0ForcedFluxVolumePreference;
    }
    if (ctx && ctx._scale0ForcedVisualParameterPreferences) {
        delete ctx._scale0ForcedVisualParameterPreferences;
    }
    return {
        fluxVolume: typeof forcedFluxPreference === 'boolean'
            ? forcedFluxPreference
            : readButtonActive('toggle-flux-volume'),
        fluxSlice:  readButtonActive('toggle-flux-slice'),
        fluxPointScale: Number.isFinite(forcedParameters.fluxPointScale)
            ? forcedParameters.fluxPointScale
            : Number(readInputValue('flux-point-scale', 1.0)),
        fluxThreshold: Number.isFinite(forcedParameters.fluxThreshold)
            ? forcedParameters.fluxThreshold
            : Number(readInputValue('flux-threshold', 0.005)),
        fluxOpacity: Number.isFinite(forcedParameters.fluxOpacity)
            ? forcedParameters.fluxOpacity
            : Number(readInputValue('flux-opacity', 0.70)),
        overlays,
        forceStyle: state?.forceStyle || 'arrows',
        scalarRenderMode: state?.scalarRenderMode || 'default',
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

    // Restore the user's renderer tuning before the next scenario applies any
    // of its local visibility aids. Without this, a 1e-4 dipole threshold and
    // enlarged point scale leaked into every later scenario.
    if (Number.isFinite(prefs.fluxPointScale)) {
        viewportAdapter.raw?.setFluxPointScale?.(prefs.fluxPointScale);
        viewportAdapter.raw?.setFluxSlicePointScale?.(prefs.fluxPointScale);
        setInputValue('flux-point-scale', prefs.fluxPointScale);
        setDisplayText('flux-point-scale-val', prefs.fluxPointScale.toFixed(1));
    }
    if (Number.isFinite(prefs.fluxThreshold)) {
        viewportAdapter.raw?.setFluxThreshold?.(prefs.fluxThreshold);
        viewportAdapter.raw?.setFluxSliceThreshold?.(prefs.fluxThreshold);
        setInputValue('flux-threshold', prefs.fluxThreshold);
        setDisplayText('flux-threshold-val', formatFluxThreshold(prefs.fluxThreshold));
    }
    if (Number.isFinite(prefs.fluxOpacity)) {
        viewportAdapter.raw?.setFluxOpacity?.(prefs.fluxOpacity);
        viewportAdapter.raw?.setFluxSliceOpacity?.(prefs.fluxOpacity);
        setInputValue('flux-opacity', prefs.fluxOpacity);
        setDisplayText('flux-opacity-val', prefs.fluxOpacity.toFixed(2));
    }

    // Every field overlay — button + store flag + viewport toggle
    for (const id of FIELD_BUTTON_IDS) {
        const wasOn = !!prefs.overlays?.[id];
        const flagKey = FIELD_BUTTON_TO_FLAG[id];
        setButtonActive(id, wasOn);
        if (flagKey) setFieldToggle(flagKey, wasOn);
        viewportAdapter.setOverlayVisible(flagKey, wasOn);
    }

    // Knot-zone boxes use a retained desired/effective state rather than an
    // overlay-panel button. resetScale0VisualState() always clears their mesh;
    // restore the store-qualified effective value even when the Knots panel is
    // absent so state and renderer cannot diverge across scenario loads.
    viewportAdapter.setOverlayVisible(
        'showKnotZones',
        !!state.fieldFlags.showKnotZones,
    );

    // Force render style (arrows / heatmap / flow / glyphs)
    if (prefs.forceStyle) {
        setForceStyle(prefs.forceStyle);
        setForceStyleButtons(prefs.forceStyle);
        const fieldSnapshot = { ...state.fieldFlags };
        viewportAdapter.syncForceStyle(prefs.forceStyle, fieldSnapshot);
    }

    // Volumetric-scalar render mode (default / heat map). Same treatment as
    // forceStyle: reset() cleared the viewport, so re-sync the meta-toggle
    // against the just-restored active-field set or the heat-map clouds stay
    // hidden while the default sheets show.
    if (prefs.scalarRenderMode) {
        setScalarRenderMode(prefs.scalarRenderMode);
        setScalarRenderButtons(prefs.scalarRenderMode);
        viewportAdapter.syncScalarRenderMode(prefs.scalarRenderMode, { ...state.fieldFlags });
    }

    // Mark the lattice dirty so the next tick recomputes and repaints.
    state.fieldNeedsUpdate = true;
    recomputeAnyFieldActive();
}

export function resetScale0VisualState(ctx, state, viewportAdapter) {
    resetFieldFlags();
    state.forceStyle = 'arrows';
    state.scalarRenderMode = 'default';
    resetFrameState();
    // Reset the field-line knot trackers on scenario change so the next record()
    // doesn't match the new field's knots against the previous scenario's stale
    // cell→ID map / histories (phantom birth/death/fission events + inherited
    // IDs). Independent of the knots panel + the preserved knotTracking flag.
    forEachKnotTracker((t) => t.reset());
    viewportAdapter.setFluxVolumeVisible(true);
    viewportAdapter.setFluxSliceVisible(false);
    viewportAdapter.clearScaleVisuals();
    setButtonActive('toggle-flux-volume', true);
    setButtonActive('toggle-flux-slice', false);
    for (const id of FIELD_BUTTON_IDS) setButtonActive(id, false);
    setForceStyleButtons('arrows');
    setScalarRenderButtons('default');
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
    const { resetSpeed = true, resetTickAccumulator = false, loadGeneration = null } = opts;
    const scenario = getScale0Scenario(scenarioId);
    const previousScenarioId = getScale0State().currentScenarioId;
    telemetryHub.resetScale(0);

    // Monotonic load generation: ignore late worker callbacks from a superseded load.
    const suppliedGeneration = Number(loadGeneration);
    let loadGen;
    if (loadGeneration !== null && loadGeneration !== undefined) {
        if (!Number.isInteger(suppliedGeneration)
            || suppliedGeneration < 0
            || suppliedGeneration !== Number(ctx._loadGeneration || 0)) {
            return false;
        }
        loadGen = suppliedGeneration;
    } else {
        ctx._loadGeneration = (ctx._loadGeneration || 0) + 1;
        loadGen = ctx._loadGeneration;
    }
    ctx._scale0ToggleBaselineGeneration = -1;
    ctx._scale0ToggleUserEditGeneration = -1;
    setPhysicsToggleCardPending(true);
    // Publish the intended id BEFORE async worker create so worker callbacks
    // can gate against the current selection immediately.
    setCurrentScenarioId(scenario.id);
    setSelectedScenarioId(scenario.id);
    beginScale0AuthoritativeLoad({
        scenarioId: scenario.id,
        loadGeneration: loadGen,
    });

    // Preserve the user's current overlay-toggle preferences across the reset.
    // ctx.resetAllVisualState() → resetScale0VisualState() wipes every field
    // flag + button; without this snapshot the user has to re-enable their
    // chosen overlays every time they pick a new scenario.
    const overlayPrefs = captureOverlayPreferences(state, ctx);

    const latticeSize = ctx.bridge.latticeSize || 33;
    // Local names: wasmWorker is the off-thread WASM Scale-0 owner. State still
    // uses the legacy fluxMock / useFluxMock fields (historical MockBridge slot).
    let useWasmWorker = false;
    let wasmWorker = null;
    if (wasmWorkerEligible(scenario.id, ctx.bridge) && !ctx._wasmWorkerDisabled) {
        // Off-thread WASM engine: host the real C++ physics in a Web Worker.
        // If the worker fails to initialise (e.g. importScripts NetworkError on the
        // -pthread MT glue), onInitFailure fires once: we disable the worker path for
        // this ctx and re-run the load on the in-thread WasmBridge so the engine never
        // silently dies (the proxy is the ACTIVE bridge while useFluxMock=true, so a
        // dead worker means NOTHING ticks).
        wasmWorker = new WasmBridgeProxy(latticeSize, {
            onInitFailure: () => {
                if (ctx._loadGeneration !== loadGen) return;
                fallbackToInThreadEngine(ctx, state, viewportAdapter, scenarioId, params);
            },
            onSetupFailure: (msg) => {
                if (ctx._loadGeneration !== loadGen) return;
                failScale0AuthoritativeLoad({
                    scenarioId: scenario.id,
                    loadGeneration: loadGen,
                    reason: msg || 'worker-setup-failed',
                });
                reportScenarioSetupFailure(msg || `Scenario setup failed: ${scenario.id}`);
            },
            // Frames may reconcile UI truth, but the first frame is published
            // before post-setup loader commands have crossed the worker FIFO.
            // It must never qualify the scenario.
            onEngineToggles: () => {
                if (ctx._loadGeneration !== loadGen) return;
                const activeId = getScale0State().currentScenarioId;
                if (!activeId || activeId !== scenario.id) return;
                syncScale0ToggleUiFromEngine(ctx, viewportAdapter, activeId);
            },
            // This acknowledgement is emitted only after the worker applies
            // the complete queued post-setup batch and publishes final truth.
            onConfigurationApplied: (acknowledgement) => {
                if (ctx._loadGeneration !== loadGen) return;
                const activeId = getScale0State().currentScenarioId;
                if (!activeId || activeId !== scenario.id) return;
                if (!acknowledgement?.ok) {
                    const details = Array.isArray(acknowledgement?.errors)
                        ? acknowledgement.errors.join('; ')
                        : 'worker configuration was not acknowledged';
                    failScale0AuthoritativeLoad({
                        scenarioId: activeId,
                        loadGeneration: loadGen,
                        reason: details,
                    });
                    reportScenarioSetupFailure(`Scenario configuration failed: ${details}`);
                    return;
                }
                if (syncScale0ToggleUiFromEngine(ctx, viewportAdapter, activeId)) {
                    completeScale0AuthoritativeLoad({
                        scenarioId: activeId,
                        loadGeneration: loadGen,
                        tick: readAuthoritativeTick(wasmWorker),
                        source: 'worker-configuration-applied',
                    });
                }
            },
        });
        useWasmWorker = true;
    }

    // Native setup is an atomic profile transaction. Every setToggle and
    // boundary mutation below is staged by WebSocketBridge until the scenario
    // body, JS overrides, research pins, and boundary policy are all known.
    // This prevents setup_scenario from rebuilding the engine after the loader
    // already sent its toggles (which used to discard them), and avoids dozens
    // of transient TermToggles validation warnings per selection.
    const nativeScenarioTransaction = !!ctx.bridge?.isNativeGPU
        && typeof ctx.bridge.beginScenarioConfiguration === 'function'
        && typeof ctx.bridge.commitScenarioConfiguration === 'function';
    if (nativeScenarioTransaction) ctx.bridge.beginScenarioConfiguration(scenario.id, loadGen);

    // Apply the in-memory scenario profile (defaults + overrides + research terms).
    // Do NOT re-read DOM checkboxes onto the worker — that re-coupled physics to
    // stale UI and undid applyToggleDefaults.
    applyToggleDefaults(ctx.bridge.capabilities.scale0, wasmWorker?.capabilities?.scale0 ?? null, scenario.id);

    setFluxMock(wasmWorker, useWasmWorker);
    ctx.useFluxMock = useWasmWorker;
    ctx.fluxMock = wasmWorker;

    ctx.resetAllVisualState();

    const activeBridge = (useWasmWorker && wasmWorker) ? wasmWorker : ctx.bridge;
    if (typeof activeBridge.setupScenario !== 'function') {
        reportScenarioSetupFailure(`Active Scale-0 bridge has no setupScenario (${scenario.id})`);
    }
    const harness = getPhysicsHarness(activeBridge);

    // Load immediately on selection (paused or running). Worker path posts
    // create asynchronously; in-thread returns bool for unknown ids.
    const setupOk = scenario.load(harness);
    if (setupOk === false) {
        failScale0AuthoritativeLoad({
            scenarioId: scenario.id,
            loadGeneration: loadGen,
            reason: 'scenario-setup-rejected',
        });
        reportScenarioSetupFailure(`Unknown or unhandled scenario: ${scenario.id}`);
    }

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
        wasmWorker?.capabilities?.scale0 ?? null,
    );
    if (nativeScenarioTransaction) ctx.bridge.commitScenarioConfiguration(scenario.id);

    markScenarioOverrideRows(DEFAULT_TOGGLES);
    syncComboSliders(ctx, state);
    state.latticeNeedsUpload = true;

    // Restore the captured overlay preferences. Runs last so it overrides any
    // defaults applied by applyAuxiliaryDefaults or resetScale0VisualState.
    restoreOverlayPreferences(overlayPrefs, state, viewportAdapter);
    applyScenarioVisualProfile(ctx, state, viewportAdapter, scenario.id, overlayPrefs);
    // Prefer engine truth. The in-thread bridge answers synchronously here; the
    // worker path cannot yet, so it falls back to the JS model for this frame and
    // worker callbacks correct both card and mask as authoritative readbacks arrive.
    const canSynchronouslyAcknowledge = !useWasmWorker && !activeBridge?.isNativeGPU;
    if (!canSynchronouslyAcknowledge
        || !syncScale0ToggleUiFromEngine(ctx, viewportAdapter, scenario.id)) {
        applyScale0OverlayApplicability(scenario.id, viewportAdapter);
    }

    // Keep viewport world coords (wireframe, clip, streamlines) aligned with
    // the bridge that owns physics — resize already does this; load must too.
    const activeN = activeBridge.latticeSize || latticeSize;
    ctx.syncScale0InjectionBounds?.(activeN);
    ctx.syncScale0SelectionBounds?.(activeN);
    ctx.syncScale0FlowLineControls?.(activeN);
    if (ctx.viewport?.latticeSize !== activeN) {
        ctx.viewport.setLatticeSize(activeN);
    }
    applyScenarioCameraFocus(
        ctx,
        scenario.id,
        activeN,
        previousScenarioId !== scenario.id || resetTickAccumulator,
    );

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
            if (activeBridge?.isNativeGPU
                && typeof activeBridge.queueScenarioPrimeTick === 'function') {
                activeBridge.queueScenarioPrimeTick();
            } else {
                runScale0PhysicsTicks(ctx, state, 1);
            }
        } catch (e) {
            console.warn('[Scale0] prime tick on load failed:', e);
        }
        markFieldDirty();
    }

    if (resetTickAccumulator) state.tickAccumulator.reset();
    if (setupOk !== false && canSynchronouslyAcknowledge) {
        completeScale0AuthoritativeLoad({
            scenarioId: scenario.id,
            loadGeneration: loadGen,
            tick: readAuthoritativeTick(activeBridge),
            source: 'in-thread-engine-readback',
        });
    }
    return setupOk !== false;
}

async function performScale0LatticeResize(
    ctx,
    state,
    viewportAdapter,
    newSize,
    scenarioId,
    loadGen,
    resizeIntent,
) {
    const bridge = ctx.bridge;
    const previousSize = bridge?.latticeSize || 33;
    const recoverCurrentBaseline = (reason) => {
        const stillCurrent = (!ctx.engineMode || ctx.engineMode === 'lattice')
            && (ctx._loadGeneration || 0) === loadGen
            && (ctx._scale0ResizeIntentGeneration || 0) === resizeIntent;
        if (!stillCurrent) return false;
        const actualSize = bridge?.latticeSize || previousSize;
        setInputValue('lattice-size', actualSize);
        const restored = loadScale0Scenario(
            ctx,
            state,
            viewportAdapter,
            scenarioId,
            { id: scenarioId },
            {
                resetSpeed: false,
                resetTickAccumulator: true,
                loadGeneration: loadGen,
            },
        );
        if (!restored) {
            failScale0AuthoritativeLoad({ scenarioId, loadGeneration: loadGen, reason });
        }
        return false;
    };
    // Snapshot the load generation + engine mode. The native/worker resize below
    // awaits a round-trip (seconds on ws_server); if the user changes scenario or
    // switches scale meanwhile, the post-await load MUST NOT clobber it.
    const wasEngineMode = ctx.engineMode;
    const nativeCombinedResize = !!bridge?.isNativeGPU
        && typeof bridge.resizeScenario === 'function';
    // The resize guard estimates the WASM heap cost: ≈1300 bytes/voxel
    // (Voxel + SU(2)/SU(3) link structures). Bounded by 8 GB on the wasm64
    // build or 2 GB on the wasm32 fallback. Native CUDA uses the server's live
    // RAM/VRAM preflight instead of this WASM-only estimate.
    const useWasmWorker = wasmWorkerEligible(scenarioId, bridge) && !ctx._wasmWorkerDisabled;
    const bytesPerVoxel = 1300;
    const capGB = bridge?.isWasm64 ? 8 : 2;
    const projectedBytes = Math.ceil(newSize ** 3 * bytesPerVoxel);
    const maxBytes = capGB * 1024 * 1024 * 1024;

    if (!nativeCombinedResize && newSize > MAX_WASM_INTERACTIVE_LATTICE) {
        const msg = `L=${newSize} requires the native GPU backend. Browser WASM is limited to `
            + `L=${MAX_WASM_INTERACTIVE_LATTICE} to preserve the 60 FPS UI budget.`;
        if (typeof window.showToast === 'function') window.showToast(msg, 'error');
        else console.warn('[Scale0] ' + msg);
        setInputValue('lattice-size', previousSize);
        return recoverCurrentBaseline('wasm-size-limit');
    }

    if (!nativeCombinedResize && projectedBytes >= maxBytes) {
        const projGB = (projectedBytes / 1024 / 1024 / 1024).toFixed(2);
        const msg = `L=${newSize} would need ~${projGB} GB of WASM heap (max ${capGB} GB here). Refusing to resize.`;
        if (typeof window.showToast === 'function') window.showToast(msg, 'error');
        else console.warn('[Scale0] ' + msg);
        setInputValue('lattice-size', bridge.latticeSize || 33);
        return recoverCurrentBaseline('wasm-memory-limit');
    }

    // Native CUDA combines resize + scenario construction in one transaction,
    // so the canonical load path below can update UI/toggles without allocating
    // the same RenderBridge twice. WASM keeps its existing ownership split.
    if (nativeCombinedResize) {
        try {
            if (typeof window.showToast === 'function') {
                window.showToast(`Preparing L=${newSize} on CUDA...`, 'info');
            }
            await bridge.resizeScenario(newSize, scenarioId, loadGen, () => (
                ctx.engineMode === wasEngineMode
                && Number(ctx._loadGeneration || 0) === loadGen
                && Number(ctx._scale0ResizeIntentGeneration || 0) === resizeIntent
            ));
        } catch (e) {
            if (e?.resizeSuperseded) return false;
            console.error('[Scale0] Failed to resize native CUDA lattice:', e);
            const commitUncertain = e?.resizeFailurePhase !== 'preflight';
            if (typeof window.showToast === 'function') {
                const suffix = commitUncertain
                    ? ' Authoritative size is unconfirmed; scientific mutations remain suspended.'
                    : '';
                window.showToast(`Lattice resize failed: ${e.message}.${suffix}`, 'error');
            }
            setInputValue('lattice-size', previousSize);
            if (commitUncertain) {
                failScale0AuthoritativeLoad({
                    scenarioId,
                    loadGeneration: loadGen,
                    reason: 'native-resize-commit-uncertain',
                });
                return false;
            }
            return recoverCurrentBaseline('native-resize-failed');
        }
    } else if (!useWasmWorker && bridge && typeof bridge.resize === 'function') {
        try {
            await bridge.resize(newSize);
        } catch (e) {
            console.error('[Scale0] Failed to resize simulation lattice:', e);
            if (typeof window.showToast === 'function') {
                window.showToast(`Lattice resize failed: ${e.message}`, 'error');
            }
            setInputValue('lattice-size', previousSize);
            return recoverCurrentBaseline('wasm-resize-failed');
        }
    }
    // If an async resize round-trip above was overtaken by a scenario change or
    // a scale switch (see snapshot at entry), bail — otherwise we reload the
    // stale scenario or run a Scale-0 load while another scale is active.
    if (ctx.engineMode !== wasEngineMode
        || (ctx._loadGeneration || 0) !== loadGen
        || (ctx._scale0ResizeIntentGeneration || 0) !== resizeIntent) {
        bridge?.discardPreparedScenario?.(scenarioId, loadGen);
        return false;
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
        loadGeneration: loadGen,
    });
    setInputValue('lattice-size', getActiveScale0Bridge(ctx, state)?.latticeSize || newSize);
    return true;
}

export function resizeScale0Lattice(ctx, state, viewportAdapter, newSize) {
    const scenarioId = state.currentScenarioId || readInputValue('scenario-select', 'flux-pulse');
    const resizeIntent = (ctx._scale0ResizeIntentGeneration || 0) + 1;
    ctx._scale0ResizeIntentGeneration = resizeIntent;
    ctx._loadGeneration = (ctx._loadGeneration || 0) + 1;
    const loadGen = ctx._loadGeneration;
    setPhysicsToggleCardPending(true);
    beginScale0AuthoritativeLoad({ scenarioId, loadGeneration: loadGen });

    // Serialize native/WASM resize transactions. A second size choice supersedes
    // the first before dispatch when possible; if the first is already in flight,
    // the latest request runs only after it settles so the backend's final N is
    // guaranteed to match the last accepted UI intent.
    const prior = ctx._scale0ResizeQueue || Promise.resolve();
    const queued = Promise.resolve(prior)
        .catch(() => false)
        .then(() => {
            if ((ctx._scale0ResizeIntentGeneration || 0) !== resizeIntent
                || (ctx._loadGeneration || 0) !== loadGen
                || (ctx.engineMode && ctx.engineMode !== 'lattice')) {
                return false;
            }
            return performScale0LatticeResize(
                ctx,
                state,
                viewportAdapter,
                newSize,
                scenarioId,
                loadGen,
                resizeIntent,
            );
        });
    ctx._scale0ResizeQueue = queued;
    return queued;
}

export function stepScale0(ctx, state, tickCount = 1) {
    runScale0PhysicsTicks(ctx, state, tickCount);
    state.fieldNeedsUpdate = true;
}

export function resetScale0Scenario(ctx, state, viewportAdapter) {
    loadScale0Scenario(ctx, state, viewportAdapter, state.currentScenarioId || 'flux-pulse');
}

export function exitScale0() {
    clearFluxMock();
}

export { K_GENESIS };
