/**
 * Scale-1 scenario registry.
 *
 * Declarative replacement for the retired 26-scenario pe-* switch
 * (scenarios.js). Every scenario runs on the native C++/WASM
 * ParticleEngine; particle identity is lattice-flavored (masses in K_B
 * multiples, charges in ±N) except where the [PARAMETRIC] Zoo injects
 * catalog particles.
 *
 * Epistemic framing (binding — see REF_SCALE1_DYNAMICS_FTD_FORM.md):
 *   - the 1/r² Coulomb FORM is [THEOREM]-grade lattice geometry for
 *     r ≳ 8 (Phase G geometric Coulomb); the α in the engine's prefactor
 *     is a [PARAMETRIC] insertion;
 *   - cluster mass N·K_B is [DERIVED-linear]/[SMC] (FTD-0110);
 *   - G_PE magnitude is [SMC]-floored (FTD-0131);
 *   - the Velocity-Verlet integrator and all ICs are [IMPOSED];
 *   - annihilation-by-contact is [SELECTION];
 *   - c = 1/√3 speed clamp is [SELECTION] (FTD-0407).
 * No scenario claims SM particles emerge from the lattice.
 */

import { K_B, PROTON_RATIO } from '../../constants.js';
import { takePromotionSeeds } from './promotion.js';
import { scale1State } from './state/store.js';

const BASE_PHYSICS = Object.freeze({
    coulomb: true,
    gravity: false,
    damping: false,
    lorentz: false,
    exchange: false,
    strong: false,
    magnetic_dipole: false,
    spin_orbit: false,
    radiation: false,
    relativistic: false,
    relativistic_verlet: false,
    dt: 1.0,
    softening: 0.1,
});

const BASE_OVERLAYS = Object.freeze({
    velocities: false,
    trails: false,
    efield: false,
    potential: false,
    gravityField: false,
    forceCoulomb: false,
    forceGravity: false,
    forceStrong: false,
    forceNet: false,
    system: false,
    voxelDebug: false,
    admissibilityRing: false,
    provenanceLabel: false,
    massComparison: false,
});

/** Contact radius for a promoted cluster's equivalent sphere (display units). */
function clusterContactRadius(size, displayScale) {
    return Math.max(0.5, Math.cbrt((3 * size) / (4 * Math.PI)) * (displayScale || 1));
}

/** Seed the engine from a promotion payload. Returns the seed count. */
function seedPromotionPayload(bridge, payload) {
    for (const s of payload.seeds) {
        const id = bridge.peAddParticle(null, s.charge,
            s.position[0], s.position[1], s.position[2],
            s.velocity[0], s.velocity[1], s.velocity[2],
            s.mass, clusterContactRadius(s.size, payload.displayScale));
        scale1State.promotedSeedById.set(id, s);
    }
    return payload.seeds.length;
}

/**
 * Resolve the promotion payload: fresh registry stash first (consumes it),
 * else the last consumed payload retained on the store (lets
 * s1-voxel-debug re-seed the same capture).
 */
function resolvePromotion() {
    const fresh = takePromotionSeeds();
    if (fresh) {
        scale1State.lastPromotion = fresh;
        return fresh;
    }
    return scale1State.lastPromotion;
}

export const SCALE1_SCENARIOS = [
    {
        id: 's1-promoted-lattice',
        label: 'Lattice Clusters as Particles',
        group: 'Lattice',
        description:
            'One continuous particle per lattice cluster captured by "⤴ Scale up" '
            + 'from Scale 0. mass = N·K_B [DERIVED-linear]/[SMC] (FTD-0110); '
            + 'charge = sign·N [DERIVED from telemetry]; centroid position/velocity '
            + '[DERIVED from telemetry]; Verlet integration [IMPOSED].',
        physics: { gravity: true },
        overlays: { velocities: true, admissibilityRing: true, provenanceLabel: true },
        setup({ bridge }) {
            const payload = resolvePromotion();
            if (!payload || !payload.seeds?.length) {
                console.info('[Scale1] No promotion stash — use "⤴ Scale up" '
                    + 'from Scale 0 to promote live lattice clusters.');
                return;
            }
            const n = seedPromotionPayload(bridge, payload);
            console.info(`[Scale1] Promoted ${n} lattice cluster(s) from tick `
                + `${payload.sourceTick} (scenario ${payload.sourceScenario ?? 'unknown'}, `
                + `source: ${payload.clusterSource}).`);
        },
    },
    {
        id: 's1-voxel-debug',
        label: 'Clusters with Their Source Voxels',
        group: 'Lattice',
        description:
            'The same promoted clusters with the per-voxel coarse-graining '
            + 'snapshot ghosted behind them — cluster-level vs voxel-level '
            + 'comparison. Ghost voxel mass convention is the scale-bridge\'s '
            + 'max(ρ, K_B) [IMPOSED, display only]; the dynamic particles use '
            + 'N·K_B. Requires a prior "⤴ Scale up" capture.',
        physics: { gravity: true },
        overlays: { velocities: true, voxelDebug: true, admissibilityRing: true,
                    provenanceLabel: true, massComparison: true },
        setup({ bridge }) {
            const payload = resolvePromotion();
            if (!payload || !payload.seeds?.length) {
                console.info('[Scale1] No promotion capture available — use '
                    + '"⤴ Scale up" from Scale 0 first.');
                return;
            }
            seedPromotionPayload(bridge, payload);
        },
    },
    {
        id: 's1-coulomb-orbit',
        label: 'An Electron-Style Orbit',
        group: 'Dynamics',
        description:
            'A light −1 body orbiting a heavy +1 anchor at r = 12, inside the '
            + 'r ≳ 8 window where the lattice\'s geometric Coulomb tail is '
            + '[THEOREM]-grade 1/(4πr²) form (Phase G). The α coupling in the '
            + 'engine prefactor is [PARAMETRIC]; the orbit IC comes from a '
            + 'native force-balance probe at t=0 [IMPOSED].',
        physics: {},
        overlays: { trails: true },
        setup({ bridge }) {
            bridge.peAddParticle(null, +1, 0, 0, 0, 0, 0, 0, 200 * K_B, 0.5);
            const orbiter = bridge.peAddParticle(null, -1, 12, 0, 0, 0, 0, 0, K_B, 0.3);
            bridge.peApplyEquilibriumOrbit(orbiter, { tangent: [0, 1, 0] });
        },
    },
    {
        id: 's1-cluster-pair',
        label: 'A Pair of Orbiting Charges',
        group: 'Dynamics',
        description:
            'Two synthetic promoted-style clusters (N = 20): charges ±20, '
            + 'masses 20·K_B [DERIVED-linear]/[SMC] mass law, mutually orbiting '
            + 'from native force-balance ICs [IMPOSED]. What a promoted '
            + 'cluster binary looks like without needing a live capture.',
        physics: { gravity: true },
        overlays: { velocities: true, trails: true },
        setup({ bridge }) {
            const N = 20;
            const a = bridge.peAddParticle(null, +N, 10, 0, 0, 0, 0, 0, N * K_B, 1.1);
            const b = bridge.peAddParticle(null, -N, -10, 0, 0, 0, 0, 0, N * K_B, 1.1);
            bridge.peApplyEquilibriumOrbitBatch([
                { particleId: a, center: [0, 0, 0], tangent: [0, 1, 0], sign: 1 },
                { particleId: b, center: [0, 0, 0], tangent: [0, 1, 0], sign: -1 },
            ]);
        },
    },
    {
        id: 's1-three-body',
        label: 'Three-Body Chaos',
        group: 'Dynamics',
        description:
            'Two heavy +1 bodies (PROTON_RATIO·K_B) and one light −1 body (K_B), all '
            + 'dynamic — genuinely three-body, chaotic. ICs [IMPOSED]; '
            + 'integrator [IMPOSED]; c = 1/√3 clamp [SELECTION] (FTD-0407).',
        physics: {},
        overlays: { trails: true },
        setup({ bridge }) {
            const pA = bridge.peAddParticle(null, +1, 8, 0, 0, 0, 0, 0, PROTON_RATIO * K_B, 0.5);
            const pB = bridge.peAddParticle(null, +1, -8, 0, 0, 0, 0, 0, PROTON_RATIO * K_B, 0.5);
            bridge.peAddParticle(null, -1, 0, 0, 1.5, 0, 0, 0, K_B, 0.3);
            bridge.peApplyEquilibriumOrbitBatch([
                { particleId: pA, center: [0, 0, 0], tangent: [0, 1, 0], sign: 1 },
                { particleId: pB, center: [0, 0, 0], tangent: [0, 1, 0], sign: -1 },
            ]);
        },
    },
    {
        id: 's1-empty-zoo',
        label: 'Empty Sandbox',
        group: 'Sandbox',
        description:
            'Empty scene. Inject catalog particles from the Particle Zoo — '
            + '[PARAMETRIC] extras: PDG masses and catalog quantum numbers, '
            + 'NOT lattice-derived objects (genesis produces hybrid colored '
            + 'objects, not SM particles).',
        physics: {},
        overlays: {},
        setup() { /* empty — the Zoo injects */ },
    },
];

const BY_ID = new Map(SCALE1_SCENARIOS.map(s => [s.id, s]));

export const DEFAULT_SCALE1_SCENARIO = 's1-coulomb-orbit';

export function getScale1Scenario(id) {
    return BY_ID.get(id) ?? null;
}

/** Preset in the shape the controller applies (physics + overlays merged over base). */
export function getScale1ScenarioPreset(id) {
    const s = BY_ID.get(id);
    return {
        physics: { ...BASE_PHYSICS, ...(s?.physics || {}) },
        overlays: { ...BASE_OVERLAYS, ...(s?.overlays || {}) },
        description: s?.description
            ?? 'Unknown scenario — empty scene on the native particle engine.',
    };
}

/** Fill a <select> with grouped scenario options. */
export function populateScale1ScenarioSelect(selectEl, defaultId = DEFAULT_SCALE1_SCENARIO) {
    if (!selectEl) return;
    selectEl.innerHTML = '';
    const groups = new Map();
    for (const s of SCALE1_SCENARIOS) {
        if (!groups.has(s.group)) {
            const og = document.createElement('optgroup');
            og.label = s.group;
            groups.set(s.group, og);
            selectEl.appendChild(og);
        }
        const opt = document.createElement('option');
        opt.value = s.id;
        opt.textContent = s.label;
        if (s.id === defaultId) opt.selected = true;
        groups.get(s.group).appendChild(opt);
    }
}
