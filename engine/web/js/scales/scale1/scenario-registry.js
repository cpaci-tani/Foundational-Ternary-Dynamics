/**
 * Scale-1 scenario execution adapter.
 *
 * Scientific metadata, availability, ownership, and status come from the
 * shared native/WASM Scale1ScenarioSpec registry. This file owns only web
 * presentation defaults and setup handlers keyed by the native setupId.
 */

import {
    C_SPEED, K_B, PROTON_RATIO,
    M_E_PHYS, M_MU_PHYS, M_TAU_PHYS, M_P_PHYS,
    M_U_PHYS, M_D_PHYS, M_S_PHYS,
} from '../../constants.js';

function verifiedPhysicsProfile(registry) {
    const profile = { dt: 1.0, softening: 0.1 };
    for (const spec of Array.from(registry?.physics || [])) {
        profile[spec.toggle] = !!(spec.available && spec.verifiedProfile);
    }
    return profile;
}

function registeredScenarioPhysicsProfile(scenario, registry) {
    if (!scenario || !Number.isInteger(scenario.physicsMask)) return {};
    const profile = {};
    Array.from(registry?.physics || []).forEach((spec, index) => {
        profile[spec.toggle] = !!(scenario.physicsMask & (1 << index)) && !!spec.available;
    });
    return profile;
}

const BASE_OVERLAYS = Object.freeze({
    velocities: false, trails: false, efield: false, potential: false,
    fieldBattery: false,
    gravityField: false, forceCoulomb: false, forceGravity: false,
    forceStrong: false, forceNet: false, system: false,
    admissibilityRing: false, provenanceLabel: false,
});

function add(bridge, charge, position, velocity, mass, radius = 0.35) {
    return bridge.peAddParticle(
        null, charge,
        position[0], position[1], position[2],
        velocity[0], velocity[1], velocity[2],
        mass, radius,
    );
}

function addLocked(bridge, charge, position, mass, radius = 0.35) {
    return bridge.peAddLockedParticle(
        null, charge,
        position[0], position[1], position[2],
        mass, radius,
    );
}

function addTyped(bridge, catalogId, charge, position, velocity, mass, radius = 0.35) {
    return bridge.peAddParticle(
        catalogId, charge,
        position[0], position[1], position[2],
        velocity[0], velocity[1], velocity[2],
        mass, radius,
    );
}

function registeredReplaySetup({ bridge }) {
    bridge.peUseRegisteredM3Replay?.();
}

const EXECUTION = Object.freeze({
    // One immutable FTD-0760 artifact; the observation view is presentation
    // state, not a second scenario or dynamics owner.
    m3_anatomy: {
        physics: {}, overlays: { provenanceLabel: true }, setup: registeredReplaySetup,
    },
    charge_sign_matrix: {
        physics: {}, overlays: { velocities: true, forceCoulomb: true },
        setup({ bridge }) {
            const pairs = [
                { y: 9, qa: 1, qb: 1 }, { y: 3, qa: -1, qb: -1 },
                { y: -3, qa: 1, qb: -1 }, { y: -9, qa: -1, qb: 1 },
            ];
            for (const pair of pairs) {
                add(bridge, pair.qa, [-7, pair.y, 0], [0, 0, 0], 12 * K_B);
                add(bridge, pair.qb, [7, pair.y, 0], [0, 0, 0], 12 * K_B);
            }
        },
    },
    coulomb_orbit: {
        physics: {}, overlays: { trails: true, forceCoulomb: true },
        setup({ bridge }) {
            add(bridge, +1, [0, 0, 0], [0, 0, 0], 200 * K_B, 0.5);
            const orbiter = add(bridge, -1, [12, 0, 0], [0, 0, 0], K_B, 0.3);
            bridge.peApplyEquilibriumOrbit(orbiter, { tangent: [0, 1, 0] });
        },
    },
    open_terminal_battery: {
        physics: { dt: 0.2, softening: 0.12 },
        overlays: { trails: true, velocities: true },
        setup({ bridge, viewport }) {
            const electrodeX = 9.4;
            const offsets = [-3, 0, 3];
            const sourceMass = 50 * K_B;
            const portHalfSize = 1.35;

            // This is a native ParticleEngine constraint, not viewport
            // decoration: every face is a perfect specular insulator except
            // for the centered, finite-radius-aware terminal apertures. In
            // this imposed discharge orientation, electrons may enter only
            // at the positive port and leave only at the negative port.
            bridge.peConfigureInsulatingBox(0, 0, 0, 12, 4.5, 4);
            bridge.peAddInsulatingPort(
                0, -1, 0, 0, portHalfSize, portHalfSize, -1, -1,
            );
            bridge.peAddInsulatingPort(
                0, +1, 0, 0, portHalfSize, portHalfSize, -1, +1,
            );

            // The positive electrode has nine locked + records. The negative
            // electrode has eight locked - records plus one mobile electron,
            // so the complete seeded system remains exactly charge neutral.
            // The center of the negative sheet is deliberately the carrier
            // launch channel rather than a second record at the same point.
            for (const y of offsets) {
                for (const z of offsets) {
                    addLocked(bridge, +1, [-electrodeX, y, z], sourceMass, 0.42);
                    if (y !== 0 || z !== 0) {
                        addLocked(bridge, -1, [electrodeX, y, z], sourceMass, 0.42);
                    }
                }
            }

            // One electron begins between the negative electrode and its
            // terminal with an imposed outward discharge velocity. No
            // positron or positive mobile carrier is synthesized at the
            // positive end. A returning electron can enter there, but cannot
            // emerge through it under this discharge contract.
            addTyped(bridge, 'electron', -1, [10.7, 0, 0], [0.22, 0, 0], K_B, 0.32);

            viewport?.setPEScenarioVisual?.({
                type: 'open-terminal-battery',
                length: 24,
                height: 9,
                depth: 8,
                positiveEndX: -12,
                negativeEndX: 12,
                portWidth: portHalfSize * 2,
                portHeight: portHalfSize * 2,
            });
        },
    },
    finite_port_gauss_battery: {
        physics: { dt: 1.0, softening: 0.1 },
        overlays: { fieldBattery: true, system: true },
        setup({ bridge }) {
            // FTD-0884 isolated reference instrument. The locked markers show
            // the compatible dipole source only; ParticleEngine forces do not
            // consume the matched-face field.
            bridge.peConfigureFinitePortBattery?.(6, 8, 1, 10);
            bridge.peStepFinitePortBattery?.();
            addLocked(bridge, +1, [-3, 0, 0], 20 * K_B, 0.45);
            addLocked(bridge, -1, [3, 0, 0], 20 * K_B, 0.45);
        },
    },
    cluster_pair: {
        physics: { gravity: true }, overlays: { velocities: true, trails: true },
        setup({ bridge }) {
            const count = 20;
            const a = add(bridge, +count, [10, 0, 0], [0, 0, 0], count * K_B, 1.1);
            const b = add(bridge, -count, [-10, 0, 0], [0, 0, 0], count * K_B, 1.1);
            bridge.peApplyEquilibriumOrbitBatch([
                { particleId: a, center: [0, 0, 0], tangent: [0, 1, 0], sign: 1 },
                { particleId: b, center: [0, 0, 0], tangent: [0, 1, 0], sign: -1 },
            ]);
        },
    },
    rutherford_scattering: {
        physics: { dt: 0.25, softening: 0.05 },
        overlays: { trails: true, velocities: true, forceCoulomb: true },
        setup({ bridge }) {
            add(bridge, +24, [0, 0, 0], [0, 0, 0], 4000 * K_B, 0.8);
            add(bridge, +2, [-24, 4, 0], [0.16, 0, 0], 4 * K_B, 0.35);
        },
    },
    force_decomposition: {
        physics: { gravity: true },
        overlays: { forceCoulomb: true, forceGravity: true, forceNet: true, system: true },
        setup({ bridge }) {
            add(bridge, +16, [-8, 0, 0], [0, 0, 0], 16 * K_B, 0.7);
            add(bridge, -16, [8, 0, 0], [0, 0, 0], 16 * K_B, 0.7);
        },
    },
    three_body: {
        physics: {}, overlays: { trails: true, velocities: true },
        setup({ bridge }) {
            const a = add(bridge, +1, [8, 0, 0], [0, 0, 0], PROTON_RATIO * K_B, 0.5);
            const b = add(bridge, +1, [-8, 0, 0], [0, 0, 0], PROTON_RATIO * K_B, 0.5);
            add(bridge, -1, [0, 0, 1.5], [0, 0, 0], K_B, 0.3);
            bridge.peApplyEquilibriumOrbitBatch([
                { particleId: a, center: [0, 0, 0], tangent: [0, 1, 0], sign: 1 },
                { particleId: b, center: [0, 0, 0], tangent: [0, 1, 0], sign: -1 },
            ]);
        },
    },
    relativistic_integrator: {
        physics: { coulomb: false, relativistic_verlet: true, dt: 0.25 },
        overlays: { trails: true, velocities: true, system: true },
        setup({ bridge }) {
            add(bridge, 0, [-20, 0, 0], [0.92 * C_SPEED, 0, 0], 20 * K_B, 0.45);
        },
    },
    damping_sink: {
        physics: { coulomb: false, damping: true, dt: 0.5 },
        overlays: { trails: true, velocities: true, system: true },
        setup({ bridge }) {
            add(bridge, 0, [-16, 0, 0], [0.35 * C_SPEED, 0, 0], 20 * K_B, 0.45);
        },
    },
    contact_selection: {
        physics: { contact_events: true, dt: 0.25 },
        overlays: { trails: true, velocities: true, system: true },
        setup({ bridge }) {
            add(bridge, +1, [-0.65, 2, 0], [0.03, 0, 0], K_B, 0.45);
            add(bridge, -1, [0.65, 2, 0], [-0.03, 0, 0], K_B, 0.45);
            add(bridge, +1, [-0.75, -2, 0], [0.03, 0, 0], K_B, 0.45);
            add(bridge, -1, [0.75, -2, 0], [-0.03, 0, 0], K_B, 0.45);
        },
    },
    advanced_force_isolation: {
        physics: { coulomb: false, exchange: true },
        overlays: { forceNet: true, velocities: true },
        setup({ bridge }) {
            add(bridge, +1, [-2.5, 0, 0], [0, 0, 0], K_B, 0.4);
            add(bridge, +1, [2.5, 0, 0], [0, 0, 0], K_B, 0.4);
        },
    },
    incomplete_conservation: {
        physics: { exchange: true },
        overlays: { forceCoulomb: true, forceNet: true, system: true },
        setup({ bridge }) {
            add(bridge, +1, [-4, 0, 0], [0, 0, 0], K_B, 0.4);
            add(bridge, -1, [4, 0, 0], [0, 0, 0], K_B, 0.4);
        },
    },
    quantum_exchange_eligible: {
        physics: { coulomb: false, exchange: true, dt: 0.2, softening: 0.05 },
        overlays: { forceExchange: true, forceNet: true, velocities: true, system: true },
        setup({ bridge }) {
            addTyped(bridge, 'electron', -1, [-1.5, 0, 0], [0, 0, 0], K_B, 0.3);
            addTyped(bridge, 'electron', -1, [1.5, 0, 0], [0, 0, 0], K_B, 0.3);
        },
    },
    quantum_exchange_spinless_control: {
        physics: { coulomb: false, exchange: true, dt: 0.2, softening: 0.05 },
        overlays: { forceExchange: true, forceNet: true, velocities: true, system: true },
        setup({ bridge }) {
            add(bridge, -1, [-1.5, 0, 0], [0, 0, 0], K_B, 0.3);
            add(bridge, -1, [1.5, 0, 0], [0, 0, 0], K_B, 0.3);
        },
    },
    quantum_exchange_range: {
        physics: { coulomb: false, exchange: true, dt: 0.15, softening: 0.05 },
        overlays: { forceExchange: true, forceNet: true, velocities: true, system: true },
        setup({ bridge }) {
            addTyped(bridge, 'electron', -1, [-1, 6, 0], [0, 0, 0], K_B, 0.25);
            addTyped(bridge, 'electron', -1, [1, 6, 0], [0, 0, 0], K_B, 0.25);
            addTyped(bridge, 'electron', -1, [-3, -6, 0], [0, 0, 0], K_B, 0.25);
            addTyped(bridge, 'electron', -1, [3, -6, 0], [0, 0, 0], K_B, 0.25);
        },
    },
    quantum_spin_orbit_parallel: {
        physics: { coulomb: false, spin_orbit: true, dt: 0.15, softening: 0.05 },
        overlays: { trails: true, velocities: true, forceSpinOrbit: true, forceNet: true, system: true },
        setup({ bridge }) {
            addLocked(bridge, +1, [0, 0, 0], 200 * K_B, 0.4);
            const mover = addTyped(
                bridge, 'electron', -1, [7, 0, 0], [0, 0.08, 0], K_B, 0.25,
            );
            bridge.peSetSpinAxis(mover, 0, 0, -1);
        },
    },
    quantum_spin_orbit_antiparallel: {
        physics: { coulomb: false, spin_orbit: true, dt: 0.15, softening: 0.05 },
        overlays: { trails: true, velocities: true, forceSpinOrbit: true, forceNet: true, system: true },
        setup({ bridge }) {
            addLocked(bridge, +1, [0, 0, 0], 200 * K_B, 0.4);
            const mover = addTyped(
                bridge, 'electron', -1, [7, 0, 0], [0, 0.08, 0], K_B, 0.25,
            );
            bridge.peSetSpinAxis(mover, 0, 0, 1);
        },
    },
    quantum_dipole_antiparallel: {
        physics: { coulomb: false, magnetic_dipole: true, dt: 0.15, softening: 0.05 },
        overlays: { forceMagneticDipole: true, forceNet: true, velocities: true, system: true },
        setup({ bridge }) {
            const a = addTyped(bridge, 'electron', -1, [-2, 0, 0], [0, 0, 0], K_B, 0.25);
            const b = addTyped(bridge, 'electron', -1, [2, 0, 0], [0, 0, 0], K_B, 0.25);
            bridge.peSetSpinAxis(a, 1, 0, 0);
            bridge.peSetSpinAxis(b, -1, 0, 0);
        },
    },
    quantum_dipole_transverse: {
        physics: { coulomb: false, magnetic_dipole: true, dt: 0.15, softening: 0.05 },
        overlays: { forceMagneticDipole: true, forceNet: true, velocities: true, system: true },
        setup({ bridge }) {
            const a = addTyped(bridge, 'electron', -1, [-2, 0, 0], [0, 0, 0], K_B, 0.25);
            const b = addTyped(bridge, 'electron', -1, [2, 0, 0], [0, 0, 0], K_B, 0.25);
            bridge.peSetSpinAxis(a, 0, 1, 0);
            bridge.peSetSpinAxis(b, 0, 1, 0);
        },
    },
    quantum_lorentz_charge_control: {
        physics: { coulomb: false, lorentz: true, dt: 0.15, softening: 0.05 },
        overlays: { trails: true, velocities: true, forceLorentz: true, forceNet: true, system: true },
        setup({ bridge }) {
            const source = addLocked(bridge, +1, [0, 0, 0], 20 * K_B, 0.35);
            bridge.peSetSpinAxis(source, 0, 0, 1);
            add(bridge, -1, [-6, 2, 0], [0.12 * C_SPEED, 0, 0], K_B, 0.25);
            add(bridge, +1, [-6, -2, 0], [0.12 * C_SPEED, 0, 0], K_B, 0.25);
        },
    },
    quantum_lorentz_velocity_control: {
        physics: { coulomb: false, lorentz: true, dt: 0.15, softening: 0.05 },
        overlays: { trails: true, velocities: true, forceLorentz: true, forceNet: true, system: true },
        setup({ bridge }) {
            const source = addLocked(bridge, +1, [0, 0, 0], 20 * K_B, 0.35);
            bridge.peSetSpinAxis(source, 0, 0, 1);
            add(bridge, -1, [-6, 2, 0], [0.12 * C_SPEED, 0, 0], K_B, 0.25);
            add(bridge, -1, [6, -2, 0], [-0.12 * C_SPEED, 0, 0], K_B, 0.25);
        },
    },
    quantum_radiation_scattering: {
        physics: { radiation: true, dt: 0.15, softening: 0.05 },
        overlays: { trails: true, velocities: true, forceCoulomb: true, forceRadiation: true, forceNet: true, system: true },
        setup({ bridge }) {
            addLocked(bridge, +24, [0, 0, 0], 4000 * K_B, 0.7);
            add(bridge, +2, [-9, 2, 0], [0.2 * C_SPEED, 0, 0], 4 * K_B, 0.3);
        },
    },
    quantum_relativistic_counterstream: {
        physics: { coulomb: false, relativistic_verlet: true, dt: 0.2 },
        overlays: { trails: true, velocities: true, system: true },
        setup({ bridge }) {
            add(bridge, 0, [-12, 2, 0], [0.88 * C_SPEED, 0, 0], 10 * K_B, 0.35);
            add(bridge, 0, [12, -2, 0], [-0.88 * C_SPEED, 0, 0], 10 * K_B, 0.35);
        },
    },
    quantum_color_triplet: {
        physics: { coulomb: false, strong: true, dt: 0.05, softening: 0.05 },
        overlays: { trails: true, velocities: true, forceStrong: true, forceNet: true, system: true },
        setup({ bridge }) {
            // The native particle record has integer electric charge. Keep this
            // strong-force control electrically neutral instead of silently
            // truncating fractional catalog charges.
            addTyped(bridge, 'up', 0, [-4, -2, 0], [0, 0, 0], M_U_PHYS, 0.3);
            addTyped(bridge, 'down', 0, [4, -2, 0], [0, 0, 0], M_D_PHYS, 0.3);
            addTyped(bridge, 'strange', 0, [0, 4, 0], [0, 0, 0], M_S_PHYS, 0.3);
        },
    },
    qed_static_coulomb: {
        physics: { dt: 0.25, softening: 0.1 },
        overlays: { efield: true, potential: true, forceCoulomb: true, system: true },
        setup({ bridge }) {
            addLocked(bridge, +1, [-7, 0, 0], 20 * K_B, 0.4);
            addLocked(bridge, -1, [7, 0, 0], 20 * K_B, 0.4);
        },
    },
    qed_moller_reference: {
        physics: { dt: 0.25, softening: 0.08 },
        overlays: { trails: true, velocities: true, forceCoulomb: true, system: true },
        setup({ bridge }) {
            add(bridge, -1, [-16, -1.5, 0], [0.18 * C_SPEED, 0, 0], K_B, 0.25);
            add(bridge, -1, [16, 1.5, 0], [-0.18 * C_SPEED, 0, 0], K_B, 0.25);
        },
    },
    qed_bhabha_reference: {
        physics: { dt: 0.25, softening: 0.08, contact_events: false },
        overlays: { trails: true, velocities: true, forceCoulomb: true, system: true },
        setup({ bridge }) {
            add(bridge, -1, [-16, -2, 0], [0.15 * C_SPEED, 0, 0], K_B, 0.25);
            add(bridge, +1, [16, 2, 0], [-0.15 * C_SPEED, 0, 0], K_B, 0.25);
        },
    },
    qed_magnetic_dipole: {
        physics: { coulomb: false, magnetic_dipole: true, dt: 0.25, softening: 0.05 },
        overlays: { trails: true, velocities: true, forceMagneticDipole: true, forceNet: true, system: true },
        setup({ bridge }) {
            const a = add(bridge, +1, [-2, 0, 0], [0, 0, 0], K_B, 0.2);
            const b = add(bridge, +1, [2, 0, 0], [0, 0, 0], K_B, 0.2);
            bridge.peSetSpinAxis(a, 1, 0, 0);
            bridge.peSetSpinAxis(b, 1, 0, 0);
        },
    },
    qed_lorentz_dipole: {
        physics: { coulomb: false, lorentz: true, dt: 0.25, softening: 0.05 },
        overlays: { trails: true, velocities: true, forceLorentz: true, forceNet: true, system: true },
        setup({ bridge }) {
            const source = addLocked(bridge, +1, [0, 2, 0], K_B, 0.3);
            bridge.peSetSpinAxis(source, 0, 0, 1);
            add(bridge, -1, [-7, 0, 0], [0.2 * C_SPEED, 0, 0], K_B, 0.25);
        },
    },
    qed_spin_orbit: {
        physics: { spin_orbit: true, dt: 0.25, softening: 0.08 },
        overlays: { trails: true, velocities: true, forceCoulomb: true, forceSpinOrbit: true, forceNet: true, system: true },
        setup({ bridge }) {
            addLocked(bridge, +1, [0, 0, 0], 200 * K_B, 0.45);
            const orbiter = add(bridge, -1, [7, 0, 0], [0, 0, 0], K_B, 0.25);
            bridge.peApplyEquilibriumOrbit(orbiter, { tangent: [0, 1, 0] });
            bridge.peSetSpinAxis(orbiter, 0, 0, 1);
        },
    },
    qed_radiation_reaction: {
        physics: { radiation: true, dt: 0.25, softening: 0.08 },
        overlays: { trails: true, velocities: true, forceCoulomb: true, forceRadiation: true, forceNet: true, system: true },
        setup({ bridge }) {
            addLocked(bridge, +1, [0, 0, 0], 200 * K_B, 0.45);
            const orbiter = add(bridge, -1, [8, 0, 0], [0, 0, 0], K_B, 0.25);
            bridge.peApplyEquilibriumOrbit(orbiter, { tangent: [0, 1, 0] });
        },
    },
    empty_zoo: { physics: {}, overlays: {}, setup() {} },
    parametric_species: {
        physics: { coulomb: false }, overlays: { provenanceLabel: true },
        setup({ bridge }) {
            const rows = [
                [-12, -1, M_E_PHYS], [-4, -1, M_MU_PHYS],
                [4, -1, M_TAU_PHYS], [12, +1, M_P_PHYS],
            ];
            for (const [x, q, mass] of rows) {
                add(bridge, q, [x, 0, 0], [0, 0, 0], mass, 0.45);
            }
        },
    },
    mass_ladder: {
        physics: { coulomb: false }, overlays: {},
        setup({ bridge }) {
            const masses = [M_E_PHYS, M_MU_PHYS, M_TAU_PHYS, M_P_PHYS];
            masses.forEach((mass, index) => add(
                bridge, 0, [-12 + 8 * index, 0, 0], [0, 0, 0], mass,
                0.25 + 0.2 * index,
            ));
        },
    },
});

const BOOTSTRAP_SCENARIO = Object.freeze({
    id: 's1-native-m3-replay', label: 'M3 Evidence Replay',
    family: 'Particle evidence replay', group: 'Particle evidence replay',
    workspace: 'particle_observatory', mode: 'native_matter',
    owner: 'native_matter_observer', scenarioClass: 'qualified_replay',
    status: 'measured', canonicalSource: 'FTD-0760', setupId: 'm3_anatomy',
    summary: 'Qualified finite-time relational-matter replay.',
    expectedObservable: 'Constituent relation, qualification margins, and field availability.',
    prohibitedClaim: 'No Standard Model identity or asymptotic stability claim.',
    available: true, interactive: false, performanceClass: 'light', unavailableReason: '',
    behavior: 'read_only_replay', pairedScenarioId: '',
    ...EXECUTION.m3_anatomy,
});

export let SCALE1_SCENARIOS = [BOOTSTRAP_SCENARIO];
export const SCALE1_SCENARIO_TARGET_COUNT = 36;
let byId = new Map(SCALE1_SCENARIOS.map(row => [row.id, row]));

export const SCALE1_M3_VIEWS = Object.freeze([
    { id: 'anatomy', label: 'Anatomy', cue: 'Inspect the qualified object and its constituent records.' },
    { id: 'graph', label: 'Constituent graph', cue: 'Inspect parent links, lineage, and graph margin.' },
    { id: 'fields', label: 'Field channels', cue: 'Compare actual, selected-bound, residual, outgoing, and background records.' },
    { id: 'centers', label: 'Center observers', cue: 'Compare integer and selected fractional center coordinates.' },
    { id: 'identity', label: 'Identity margins', cue: 'Inspect qualification, age, graph margin, and energy margin.' },
    { id: 'coverage', label: 'Coverage ledger', cue: 'Inspect covered, missing, and non-conservative masks before reading drift.' },
]);

export const SCALE1_BEHAVIOR_PRESENTATION = Object.freeze({
    dynamic: {
        label: 'DYNAMIC', panel: 'diagnostics',
        cue: 'Time evolves. Watch trajectories, force arrows, and registered diagnostics.',
    },
    read_only_replay: {
        label: 'READ-ONLY REPLAY', panel: 'diagnostics',
        cue: 'No motion is expected. This is one immutable evidence artifact with selectable observables.',
    },
    static_field: {
        label: 'STATIC FIELD', panel: 'diagnostics',
        cue: 'Sources are intentionally locked. Inspect field lines, potential, force direction, and pair energy.',
    },
    null_control: {
        label: 'ZERO EXPECTED', panel: 'diagnostics',
        cue: 'Zero target-force response is the expected result. Use the A/B control to compare the eligible case.',
    },
    awaiting_input: {
        label: 'WAITING FOR INJECTION', panel: 'zoo',
        cue: 'The scene is intentionally empty until a parametric catalog record is injected from the Zoo panel.',
    },
    static_reference: {
        label: 'STATIC REFERENCE', panel: 'diagnostics',
        cue: 'No interaction is expected. Compare imported masses, labels, provenance, and display scale.',
    },
});

export function scale1BehaviorPresentation(behavior) {
    return SCALE1_BEHAVIOR_PRESENTATION[behavior]
        || SCALE1_BEHAVIOR_PRESENTATION.dynamic;
}

function scenarioDescription(row) {
    return [
        `[${String(row.status || 'open').toUpperCase()}] ${row.summary || ''}`,
        row.owner ? `Dynamics: ${row.owner}.` : '',
        row.behavior ? `Behavior: ${scale1BehaviorPresentation(row.behavior).label}.` : '',
        row.validationState ? `Validation: ${row.validationState}. ${row.validationCriterion || ''}` : '',
        row.validationEvidence ? `Evidence: ${row.validationEvidence}` : '',
        row.expectedObservable ? `Observe: ${row.expectedObservable}` : '',
        row.prohibitedClaim ? `Boundary: ${row.prohibitedClaim}` : '',
        row.canonicalSource ? `Source: ${row.canonicalSource}` : '',
        !row.available && row.unavailableReason ? `Unavailable: ${row.unavailableReason}` : '',
    ].filter(Boolean).join('\n');
}

/** Synchronize behavior, M3 subview, and paired-control affordances. */
export function syncScale1ScenarioBehaviorUI(row, m3ViewId = 'anatomy') {
    if (!row) return;
    const presentation = scale1BehaviorPresentation(row.behavior);
    const badge = document.getElementById('pe-scenario-behavior');
    if (badge) {
        badge.textContent = presentation.label;
        badge.dataset.behavior = row.behavior || 'dynamic';
        badge.dataset.uiTooltip = presentation.cue;
        badge.setAttribute('aria-label', `${presentation.label}. ${presentation.cue}`);
    }
    const subviewGroup = document.getElementById('pe-m3-view-group');
    if (subviewGroup) subviewGroup.hidden = row.id !== 's1-native-m3-replay';
    const subview = document.getElementById('pe-m3-view-select');
    if (subview) subview.value = SCALE1_M3_VIEWS.some(view => view.id === m3ViewId)
        ? m3ViewId : 'anatomy';
    const pairButton = document.getElementById('pe-paired-scenario');
    if (pairButton) {
        pairButton.hidden = !row.pairedScenarioId;
        pairButton.dataset.targetScenario = row.pairedScenarioId || '';
        const peer = getScale1Scenario(row.pairedScenarioId);
        pairButton.textContent = 'Compare A/B';
        pairButton.setAttribute('aria-label', peer
            ? `Load matched A/B control: ${peer.label}` : 'Load matched A/B control');
        pairButton.dataset.uiTooltip = peer
            ? `Load the registered matched control: ${peer.label}.`
            : 'Load the registered matched control.';
    }
}

function scenarioTooltip(row) {
    if (!row) return 'Choose a registered Scale 1 scenario.';
    return [
        `${row.label} — ${row.family || 'Scale 1'}`,
        `[${String(row.status || 'open').toUpperCase()} · ${String(row.validationState || 'unvalidated').replaceAll('_', ' ').toUpperCase()}] ${row.summary || ''}`,
        row.owner ? `Dynamics: ${row.owner}.` : '',
        row.expectedObservable ? `Observe: ${row.expectedObservable}` : '',
        row.prohibitedClaim ? `Boundary: ${row.prohibitedClaim}` : '',
        !row.available && row.unavailableReason ? `Unavailable: ${row.unavailableReason}` : '',
    ].filter(Boolean).join('\n');
}

/** Keep the shared hover overlay synchronized with the selected registry row. */
export function syncScale1ScenarioTooltip(selectEl, scenarioId = selectEl?.value) {
    if (!selectEl) return;
    const scenario = getScale1Scenario(scenarioId)
        ?? SCALE1_SCENARIOS.find(row => row.available)
        ?? null;
    const text = scenarioTooltip(scenario);
    const targets = [
        selectEl,
        selectEl.closest('.tb-group-scenario')?.querySelector('label[for="pe-scenario-select"]'),
    ];
    for (const target of targets) {
        if (!(target instanceof HTMLElement)) continue;
        target.dataset.uiTooltip = text;
        target.dataset.uiTooltipSource = 'scale1-scenario';
        target.removeAttribute('title');
    }
    if (scenario) {
        selectEl.setAttribute('aria-label', `Scale 1 scenario: ${scenario.label}. ${scenario.summary}`);
    }
}

/** Install the authoritative native manifest after WASM becomes available. */
export function installScale1ScenarioManifest(registry) {
    const rows = Array.from(registry?.scenarios || []);
    if (!rows.length) return SCALE1_SCENARIOS;
    SCALE1_SCENARIOS = rows.map(nativeRow => {
        const execution = EXECUTION[nativeRow.setupId]
            || { physics: {}, overlays: {}, setup() {} };
        return {
            ...nativeRow,
            group: nativeRow.family,
            description: scenarioDescription(nativeRow),
            ...execution,
        };
    });
    byId = new Map(SCALE1_SCENARIOS.map(row => [row.id, row]));
    return SCALE1_SCENARIOS;
}

export const DEFAULT_SCALE1_SCENARIO = 's1-native-m3-replay';

export function getScale1Scenario(id) {
    return byId.get(id) ?? null;
}

export function getScale1ScenarioPreset(id, registry = null) {
    const scenario = byId.get(id);
    return {
        // dt/softening remain setup-owned numerical parameters. Every boolean
        // physics module is overwritten by the native-owned scenario mask.
        physics: {
            ...verifiedPhysicsProfile(registry),
            ...(scenario?.physics || {}),
            ...registeredScenarioPhysicsProfile(scenario, registry),
        },
        overlays: { ...BASE_OVERLAYS, ...(scenario?.overlays || {}) },
        description: scenario?.description
            ?? 'Unknown Scale-1 scenario; no dynamics were started.',
    };
}

export function populateScale1ScenarioSelect(selectEl, defaultId = DEFAULT_SCALE1_SCENARIO) {
    if (!selectEl) return;
    selectEl.innerHTML = '';
    const groups = new Map();
    for (const scenario of SCALE1_SCENARIOS) {
        if (!groups.has(scenario.group)) {
            const group = document.createElement('optgroup');
            group.label = scenario.group;
            groups.set(scenario.group, group);
            selectEl.appendChild(group);
        }
        const option = document.createElement('option');
        option.value = scenario.id;
        option.textContent = scenario.available ? scenario.label : `○ ${scenario.label}`;
        option.disabled = !scenario.available;
        option.title = scenarioTooltip(scenario);
        option.dataset.uiTooltip = option.title;
        option.dataset.uiTooltipSource = 'scale1-scenario-option';
        option.dataset.uiTooltipSkip = 'true';
        if (scenario.id === defaultId) option.selected = true;
        groups.get(scenario.group).appendChild(option);
    }
    syncScale1ScenarioTooltip(selectEl, selectEl.value || defaultId);
}
