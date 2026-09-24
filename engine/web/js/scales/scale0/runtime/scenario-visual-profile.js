/** Scenario-owned presentation profiles, isolated from backend transactions. */
import { FIELD_TOGGLE_BINDINGS, setButtonActive, setInputValue, setScalarRenderButtons } from '../ui/dom.js?v=3';
import { markFieldDirty, setFieldToggle, setScalarRenderMode } from '../state/store.js';

const FIELD_BUTTON_TO_FLAG = Object.fromEntries(FIELD_TOGGLE_BINDINGS);
const COMPACT_SEED_FOCUS = Object.freeze({ focusRadius: 5, focusMinL: 65 });
export const SCALE0_SCENARIO_VISUAL_PROFILES = {
    'flux-thermalization': {
        fluxVolume: false, scalarRenderMode: 'volume',
        fieldOverlays: ['toggle-em-energy'], focusRadiusFraction: 0.6,
    },
    's0-seed-dynamical-flux-dressing': {
        // Show the manifested source, generated divergence, and integral
        // curves together. The curves visualize J; they are not extra strings.
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.8,
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
        // Enlarge the softened opposite-source J markers without changing
        // engine physics. Threshold uses the same near-zero default as all loads.
        fluxVolume: true,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
        fieldOverlays: ['toggle-flux-lines'],
    },
    's0-field-magnetic-dipole': {
        // The imposed quantity is a vector potential; B=curl(J) is the honest
        // magnetic-dipole view. Compact large-L samples are also below 0.005.
        fluxVolume: true,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
        fieldOverlays: ['toggle-b-field', 'toggle-flux-lines'],
    },
    's0-seed-schwarzschild': {
        // This is an inert inverse-square J ansatz (not a live horizon or
        // latency solution). Keep those absent overlays off and reveal J.
        fluxVolume: true,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
        fieldOverlays: ['toggle-flux-lines'],
    },
    's0-seed-time-horizon': {
        // Exact alias of the inert radial ansatz. Do not imply that the absent
        // latency/horizon channels are computed by turning them on by default.
        fluxVolume: true,
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
        // mass and the engine latency-Poisson mapping (FTS2 kind 17).
        fieldOverlays: ['toggle-state-field', 'toggle-latency'],
    },
    's0-field-spacetime-forcing-boundary': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
    },
    's0-field-rf-lattice-wave': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
    },
    's0-field-light-lattice-wave': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
    },
    's0-field-sound-lattice-wave': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
    },
    's0-field-shear-layer': {
        // Wave-family presentation, matching the other s0-field-*-wave
        // entries: reveal J and lead with the flux-slice overlay so the
        // propagating (not diffusing) sheared profile is visible.
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.6,
        fluxOpacity: 0.85,
    },
    's0-field-thomson-scattering': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.4,
        fluxOpacity: 0.85,
    },
    's0-field-thomson-unlocked-recoil': {
        fluxVolume: true,
        fluxSlice: true,
        fluxPointScale: 2.4,
        fluxOpacity: 0.85,
    },
    // -- s0-cell-* flux cells ---------------------------------------------
    's0-cell-capacitor': {
        // Locked plates plus the Gauss-built gap flux: show the plates, the
        // J curves between them, and div(J) so the source law is visible.
        fluxVolume: true,
        fieldOverlays: ['toggle-flux-lines', 'toggle-state-field', 'toggle-div-field'],
    },
    's0-cell-torus': {
        // Azimuthal ring of J with W=0 at tick 0: lead with J curves, the
        // honest B=curl(J) view, and Poynting flow so the LC-like exchange is
        // visible as it develops. E=-wave_vel starts at zero and is not the
        // default channel.
        fluxVolume: true,
        fieldOverlays: ['toggle-flux-lines', 'toggle-b-field', 'toggle-poynting'],
    },
    's0-cell-torus-reverse': {
        // Azimuthal ring of J with W=0 at tick 0: lead with J curves, the
        // honest B=curl(J) view, and Poynting flow so the LC-like exchange is
        // visible as it develops. E=-wave_vel starts at zero and is not the
        // default channel.
        fluxVolume: true,
        fieldOverlays: ['toggle-flux-lines', 'toggle-b-field', 'toggle-poynting'],
    },
    's0-cell-torus-scrambled': {
        // Azimuthal ring of J with W=0 at tick 0: lead with J curves, the
        // honest B=curl(J) view, and Poynting flow so the LC-like exchange is
        // visible as it develops. E=-wave_vel starts at zero and is not the
        // default channel.
        fluxVolume: true,
        fieldOverlays: ['toggle-flux-lines', 'toggle-b-field', 'toggle-poynting'],
    },
    's0-cell-torus-open': {
        // Azimuthal ring of J with W=0 at tick 0: lead with J curves, the
        // honest B=curl(J) view, and Poynting flow so the LC-like exchange is
        // visible as it develops. E=-wave_vel starts at zero and is not the
        // default channel.
        fluxVolume: true,
        fieldOverlays: ['toggle-flux-lines', 'toggle-b-field', 'toggle-poynting'],
    },
    's0-cell-torus-walled': {
        // Azimuthal ring of J with W=0 at tick 0: lead with J curves, the
        // honest B=curl(J) view, and Poynting flow so the LC-like exchange is
        // visible as it develops. E=-wave_vel starts at zero and is not the
        // default channel.
        fluxVolume: true,
        fieldOverlays: ['toggle-flux-lines', 'toggle-b-field', 'toggle-poynting'],
    },
    's0-cell-triad': {
        // Three standing arms with nonzero W at tick 0: J curves, B=curl(J),
        // and Poynting flow (net zero by symmetry, locally nonzero).
        fluxVolume: true,
        fieldOverlays: ['toggle-flux-lines', 'toggle-b-field', 'toggle-poynting'],
    },
    's0-cell-torus-membrane': {
        // Ring inside a locked clocked shell: show the shell (state field),
        // the J curves, and the Poynting flow that the wall turns back.
        fluxVolume: true,
        fieldOverlays: ['toggle-state-field', 'toggle-flux-lines', 'toggle-poynting'],
    },
    's0-cell-torus-membrane-gated': {
        // Same, plus the aperture that opens at tick 150 on the +x side.
        fluxVolume: true,
        fieldOverlays: ['toggle-state-field', 'toggle-flux-lines', 'toggle-poynting'],
    },
    's0-cell-membrane-pumped': {
        // Starts empty: the pump fills the ring over the first 20 ticks.
        fluxVolume: true,
        fieldOverlays: ['toggle-state-field', 'toggle-flux-lines', 'toggle-poynting'],
    },
    's0-cell-membrane-pumped-resonant': {
        // Same, with increments spaced by the cell period.
        fluxVolume: true,
        fieldOverlays: ['toggle-state-field', 'toggle-flux-lines', 'toggle-poynting'],
    },
    's0-cell-membrane-transfer': {
        // Two tangent walls; the receiver on +x fills after the port opens.
        fluxVolume: true,
        fieldOverlays: ['toggle-state-field', 'toggle-flux-lines', 'toggle-poynting'],
    },
};

export function setDisplayText(id, text) {
    if (typeof document === 'undefined') return;
    const el = document.getElementById(id);
    if (el) el.textContent = text;
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

    if (profile.scalarRenderMode) {
        ctx._scale0ForcedScalarModePreference ??= prefs?.scalarRenderMode || 'default';
        setScalarRenderMode(profile.scalarRenderMode);
        setScalarRenderButtons(profile.scalarRenderMode);
        viewportAdapter.syncScalarRenderMode(profile.scalarRenderMode, { ...state.fieldFlags });
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
