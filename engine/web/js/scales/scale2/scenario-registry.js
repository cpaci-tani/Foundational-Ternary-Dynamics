/**
 * Scale 2 — AE scenario registry (canonical metadata + select population).
 *
 * Curated demos live here; element scenarios (ae-el-*) are generated from
 * elements.js at populate time. Hydrogen is covered by ae-hydrogen-atom — Z=1
 * is omitted from the element list to avoid a duplicate entry.
 */

import { getElement } from '../../elements.js';

export const AE_DEFAULT_SCENARIO = 'ae-hydrogen-atom';

/**
 * @typedef {object} AEScenarioMeta
 * @property {string} id
 * @property {string} title
 * @property {string} category
 * @property {string} summary
 * @property {string[]} [tags]
 * @property {'static_reference'|'effective_dynamics'|'sandbox'} scenarioClass
 * @property {'empirical'|'parametric'|'imposed'|'mixed'} epistemicStatus
 * @property {string} owner
 * @property {number} seed
 * @property {Readonly<Record<string, boolean|number>>} physics
 * @property {Readonly<{dt:number, softening:number, thermostatTemp:number}>} parameters
 * @property {Readonly<Record<string, boolean|string>>} overlays
 * @property {Readonly<Record<string, number|string|boolean>>} expected
 * @property {string} [reaction] Legacy single-channel identifier
 * @property {Readonly<Record<string, string|number>>} [nuclear]
 * @property {Readonly<{protocol:string,label:string,observation:string,phases:ReadonlyArray<{tick:number,label:string}>}>} [experiment]
 * @property {string} evidence
 */

/** @type {AEScenarioMeta[]} */
const AE_SCENARIO_PRESENTATION = [
    {
        id: 'ae-hydrogen-atom',
        category: 'Single-Atom Physics',
        title: 'Hydrogen Reference (composite)',
        summary: 'A locked composite hydrogen record with an empirical 1s cloud visualization. The AtomEngine does not integrate an internal proton-electron pair or a quantum orbital.',
        tags: ['static', 'reference'],
    },
    {
        id: 'ae-rutherford-scattering',
        category: 'Single-Atom Physics',
        title: 'Rutherford Scattering',
        summary: 'Effective He2+ point-center deflection from a locked +79 charged center. This is classical Coulomb scattering, not resolved nuclear structure.',
        tags: ['ionic', 'dynamics'],
    },
    {
        id: 'ae-he-cluster',
        category: 'Noble Gas Clusters',
        title: 'He Cluster (6 atoms, vdW)',
        summary: 'Helium clustering is a weak-binding problem dominated by van der Waals attraction and excluded-volume repulsion, not strong covalent directionality.',
        tags: ['vdw'],
    },
    {
        id: 'ae-ar-cluster',
        category: 'Noble Gas Clusters',
        title: 'Ar Cluster (8 atoms, vdW)',
        summary: 'Argon makes the same noble-gas story visually stronger because dispersion attraction is deeper and the cluster compacts more readily.',
        tags: ['vdw'],
    },
    {
        id: 'ae-noble-mix',
        category: 'Noble Gas Clusters',
        title: 'Noble Mix (He + Ne + Ar)',
        summary: 'The noble mix scenario is about species-dependent \u03c3 and \u03b5 values: same broad force law, different preferred spacing and clustering depth.',
        tags: ['vdw'],
    },
    {
        id: 'ae-nacl-form',
        category: 'Ionic Formation',
        title: 'Na + Cl \u2192 NaCl',
        summary: 'NaCl formation is the textbook ionic case: opposite charges attract, a preferred separation appears, and the bond is governed mainly by electrostatic balance.',
        tags: ['ionic'],
    },
    {
        id: 'ae-nacl-lattice',
        category: 'Ionic Formation',
        title: 'NaCl 3\u00d73 Lattice',
        summary: 'NaCl lattice extends ionic bonding into periodic packing, so lattice energy and coordination become the right language.',
        tags: ['ionic'],
    },
    {
        id: 'ae-mgf2',
        category: 'Ionic Formation',
        title: 'Mg\u00b2\u207a + 2F\u207b \u2192 MgF\u2082',
        summary: 'MgF\u2082 is a stoichiometry lesson as much as a force lesson: total charge balance determines the preferred assembly pattern.',
        tags: ['ionic'],
    },
    {
        id: 'ae-h2-form',
        category: 'Covalent Formation',
        title: 'H + H \u2192 H\u2082',
        summary: 'H\u2082 formation is the simplest covalent-bonding case, where bond length and spring-like stabilization are the main quantities to watch.',
        tags: ['covalent'],
    },
    {
        id: 'ae-bond-rupture-cycle',
        category: 'Validation Laboratories',
        title: 'Bond Rupture & Recombination',
        summary: 'A bonded H₂ pair is pulled beyond the imposed break distance, driven back together, and damped after recapture so the complete bond-topology cycle is directly observable.',
        tags: ['validation', 'bonding', 'topology', 'protocol'],
    },
    {
        id: 'ae-o2-form',
        category: 'Covalent Formation',
        title: 'O + O \u2192 O\u2082',
        summary: 'O\u2082 formation pushes beyond the minimal H\u2082 picture and invites discussion of stronger bonding and molecular stability.',
        tags: ['covalent'],
    },
    {
        id: 'ae-ch4-form',
        category: 'Covalent Formation',
        title: 'C + 4H \u2192 CH\u2084',
        summary: 'CH\u2084 is the tetrahedral geometry showcase, so symmetry and bond-angle stabilization matter as much as raw radial attraction.',
        tags: ['covalent', 'vsepr'],
    },
    {
        id: 'ae-water-dimer',
        category: 'H-Bonding',
        title: 'Water Dimer (H-bond)',
        summary: 'The water dimer exposes the effective directional H-bond kernel and intramolecular angle response. Dipole forces remain independently toggle-gated.',
        tags: ['hbond'],
    },
    {
        id: 'ae-water-cluster',
        category: 'H-Bonding',
        title: 'Water Pentamer',
        summary: 'Water clusters quickly turn into network problems: local H-bond rules create global geometry.',
        tags: ['hbond'],
    },
    {
        id: 'ae-vsepr-linear',
        category: 'VSEPR Geometry',
        title: 'CO\u2082 \u2192 Linear (180\u00b0)',
        summary: 'The CO\u2082 case shows how repulsion geometry can favor a 180\u00b0 arrangement even when the molecule is built from more than two atoms.',
        tags: ['vsepr'],
    },
    {
        id: 'ae-vsepr-tetrahedral',
        category: 'VSEPR Geometry',
        title: 'CH\u2084 \u2192 Tetrahedral (109.5\u00b0)',
        summary: 'CH\u2084 tetrahedral is the classic 109.5\u00b0 geometry lesson.',
        tags: ['vsepr'],
    },
    {
        id: 'ae-vsepr-bent',
        category: 'VSEPR Geometry',
        title: 'H\u2082O \u2192 Bent (104.5\u00b0)',
        summary: 'H\u2082O bent geometry is the standard \u201clone pairs change the angle\u201d teaching case.',
        tags: ['vsepr'],
    },
    {
        id: 'ae-thermal-gas',
        category: 'Thermal Dynamics',
        title: 'Ar Gas (12 atoms + thermostat)',
        summary: 'Thermal gas is about ensemble behavior, temperature control, and whether kinetic agitation overwhelms short-range ordering.',
        tags: ['thermal', 'vdw'],
    },
    {
        id: 'ae-argon-thermal-cycle',
        category: 'Validation Laboratories',
        title: 'Argon Heat–Quench–Release',
        summary: 'A dense 27-atom Lennard-Jones argon cluster is heated, quenched, then released from its thermostat while temperature, energy, and coordination evolve.',
        tags: ['validation', 'thermal', 'vdw', 'protocol'],
    },
    {
        id: 'ae-collision',
        category: 'Thermal Dynamics',
        title: 'Head-On Collision',
        summary: 'Head-on collision is the atom-engine momentum-conservation demo.',
        tags: ['vdw', 'dynamics'],
    },
    {
        id: 'ae-conservative-pair',
        category: 'Validation Laboratories',
        title: 'Conservative Ar Pair',
        summary: 'A closed two-body Lennard-Jones trajectory with every driven intervention disabled, intended for tracked-energy and momentum drift checks.',
        tags: ['validation', 'conservation'],
    },
    {
        id: 'ae-damped-relaxation',
        category: 'Validation Laboratories',
        title: 'Damped Cluster Relaxation',
        summary: 'An intentionally strained argon cluster loses kinetic energy through the explicit damping sink and settles toward lower-energy spacing.',
        tags: ['validation', 'damping'],
    },
    {
        id: 'ae-polar-dimer',
        category: 'Validation Laboratories',
        title: 'Polar HF Dimer',
        summary: 'Two bonded HF records expose electronegativity charge transfer, intermolecular Coulomb response, and the effective dipole-dipole term.',
        tags: ['validation', 'dipole', 'electronegativity'],
    },
    {
        id: 'ae-dt-fusion',
        category: 'Nuclear Reactions',
        title: '²H + ³H Fusion',
        summary: 'Live deuterium and tritium trajectories enter an explicit energy-dependent collision hazard, then accepted events transform into ⁴He + n with evaluated-mass Q = 17.589 MeV and exact nucleon, momentum, and energy ledgers.',
        tags: ['nuclear', 'fusion', 'isotopes', 'conservation'],
    },
    {
        id: 'ae-u235-fission',
        category: 'Nuclear Reactions',
        title: '²³⁵U Neutron-Induced Fission',
        summary: 'A live neutron trajectory intersects ²³⁵U and a seeded one-group hazard can produce the reference ¹⁴¹Ba + ⁹²Kr + 3n channel with evaluated-mass Q = 173.280 MeV and exact bookkeeping.',
        tags: ['nuclear', 'fission', 'isotopes', 'conservation'],
    },
    {
        id: 'ae-dt-fusion-burn',
        category: 'Nuclear Reactions',
        title: 'Finite D-T Fusion Burn',
        summary: 'A finite D-T initial population evolves through live swept collisions. Each accepted rendered transaction carries an explicit ensemble weight while microscopic conservation and macroscopic energy remain separate ledgers.',
        tags: ['nuclear', 'fusion', 'population', 'energy-transport'],
    },
    {
        id: 'ae-u235-chain-reaction',
        category: 'Nuclear Reactions',
        title: 'Finite U-235 Chain Reaction',
        summary: 'Deterministic-seed free-neutron transport evolves through finite U-235 fuel and measures generation, reproduction, scattering, leakage, absorption, deposition, and extinction or depletion.',
        tags: ['nuclear', 'fission', 'chain-reaction', 'energy-transport'],
    },
    {
        id: 'ae-u235-criticality-controls',
        category: 'Nuclear Reactions',
        title: 'U-235 Criticality Control Laboratory',
        summary: 'A repeatable finite U-235 source experiment exposes boundary, moderation, absorption, source, and reactivity controls so neutron reproduction is compared as an outcome rather than prescribed.',
        tags: ['nuclear', 'fission', 'criticality', 'controls', 'validation'],
    },
    {
        id: 'ae-fe-bcc',
        category: 'Metallic Clusters',
        title: 'Fe BCC Cluster (9 atoms)',
        summary: 'Fe BCC is a packing-and-coordination scenario where geometry matters as much as pair potential.',
        tags: ['metallic'],
    },
    {
        id: 'ae-cu-fcc',
        category: 'Metallic Clusters',
        title: 'Cu FCC Seed (7 atoms)',
        summary: 'Cu FCC is the close-packed comparison case to BCC iron.',
        tags: ['metallic'],
    },
    {
        id: 'ae-crystal-impulse-vacancy',
        category: 'Metallic Clusters',
        title: 'Crystal Impulse & Vacancy',
        summary: 'Matched finite iron chains receive the same impulse; one has complete harmonic connectivity while the other contains a vacancy that interrupts strain-energy transport.',
        tags: ['metallic', 'validation', 'energy-transport', 'defect'],
    },
    {
        id: 'ae-periodic',
        category: 'Special',
        title: 'Periodic Table (All 118)',
        summary: 'Periodic Table mode is a parameter atlas rather than one fixed simulation; the lesson is periodic trends, valence, and how element identity changes force-relevant quantities.',
        tags: ['static', 'elements'],
    },
    {
        id: 'ae-custom',
        category: 'Special',
        title: 'Custom (Manual)',
        summary: 'Custom atom mode lets you test your own composition, force toggles, and geometry under the same atom-engine rules.',
        tags: ['sandbox'],
    },
];

/**
 * Canonical production-AtomEngine physics surface.  These are effective
 * classical/empirical terms; none is a substrate derivation of quantum
 * chemistry.  The registry is consumed by scenario loading, controls, tests,
 * diagnostics, and the knowledge surface so toggle applicability has one
 * owner.
 */
export const AE_PHYSICS_SPECS = Object.freeze([
    { key: 'ionic', elementId: 'ae-ionic', label: 'Ionic (Coulomb)', setter: 'aeSetIonic', defaultValue: false, status: 'parametric', conservative: true, energy: 'ionic' },
    { key: 'vdw', elementId: 'ae-vdw', label: 'Van der Waals', setter: 'aeSetVdw', defaultValue: false, status: 'parametric', conservative: true, energy: 'vdw' },
    { key: 'bonds_force', elementId: 'ae-bonds-force', label: 'Bond springs', setter: 'aeSetBondsForce', defaultValue: false, status: 'parametric', conservative: true, energy: 'bond' },
    { key: 'bonding', elementId: 'ae-bonding', label: 'Auto-bonding', setter: 'aeSetBonding', defaultValue: false, status: 'imposed', conservative: false, energy: null },
    { key: 'damping', elementId: 'ae-damping', label: 'Damping', setter: 'aeSetDamping', defaultValue: false, status: 'imposed', conservative: false, energy: null },
    { key: 'speed_limit', elementId: 'ae-speed-limit', label: 'Speed limit', setter: 'aeSetSpeedLimit', defaultValue: true, status: 'imposed', conservative: false, energy: null },
    { key: 'h_bonds', elementId: 'ae-hbonds', label: 'Hydrogen-bond force', setter: 'aeSetHBonds', defaultValue: false, status: 'parametric', conservative: false, energy: null },
    { key: 'angle_strain', elementId: 'ae-angle', label: 'VSEPR angle strain', setter: 'aeSetAngleStrain', defaultValue: false, status: 'parametric', conservative: true, energy: 'angle' },
    { key: 'dipole_dipole', elementId: 'ae-dipole', label: 'Dipole-dipole force', setter: 'aeSetDipoleDipole', defaultValue: false, status: 'parametric', conservative: false, energy: null },
    { key: 'thermostat', elementId: 'ae-thermostat', label: 'Berendsen thermostat', setter: 'aeSetThermostat', defaultValue: false, status: 'imposed', conservative: false, energy: null },
    { key: 'electronegativity', elementId: 'ae-electronegativity', label: 'Electronegativity charge transfer', setter: 'aeSetElectronegativity', defaultValue: false, status: 'empirical', conservative: false, energy: null },
]);

const PHYSICS_OFF = Object.freeze(Object.fromEntries(AE_PHYSICS_SPECS.map((term) => [term.key, false])));
const profile = (overrides = {}) => Object.freeze({ ...PHYSICS_OFF, ...overrides });
const DEFAULT_PARAMETERS = Object.freeze({ dt: 0.1, softening: 0.3, thermostatTemp: 1.0 });
const overlays = (overrides = {}) => Object.freeze({
    clouds: true, labels: true, shells: true, shellBounds: false, lobes: false,
    bondStyle: 'cylinders', field: false, velocities: false,
    dipoles: false, hbondLines: false,
    nuclearEvents: false, radiation: false, heat: false, nuclearBoundary: false,
    forceIonic: false, forceVdw: false, forceBond: false,
    forceHbond: false, forceAngle: false, forceDipole: false, forceNet: false,
    ...overrides,
});
const AE_OVERLAY_KEYS = Object.freeze(Object.keys(overlays()));
const AE_BOND_STYLES = Object.freeze(['cylinders', 'lines', 'off']);
const AE_FORCE_OVERLAY_PHYSICS = Object.freeze({
    forceIonic: 'ionic',
    forceVdw: 'vdw',
    forceBond: 'bonds_force',
    forceHbond: 'h_bonds',
    forceAngle: 'angle_strain',
    forceDipole: 'dipole_dipole',
});

const CONTRACTS = Object.freeze({
    'ae-hydrogen-atom': {
        scenarioClass: 'static_reference', epistemicStatus: 'empirical', seed: 0x020001,
        physics: profile(), overlays: overlays({ shellBounds: true }),
        expected: { atomCount: 1, bondCount: 0, dynamic: false },
        evidence: 'Static composite-atom presentation; orbital cloud is empirical visualization, not an electron trajectory.',
    },
    'ae-rutherford-scattering': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020002,
        physics: profile({ ionic: true, speed_limit: true }),
        overlays: overlays({ field: true, velocities: true, forceIonic: true }),
        expected: { atomCount: 2, bondCount: 0, dynamic: true },
        evidence: 'Effective point-center Coulomb scattering; not a resolved nucleus/electron calculation.',
    },
    'ae-he-cluster': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020003,
        physics: profile({ vdw: true, speed_limit: true }), overlays: overlays({ forceVdw: true }),
        expected: { atomCount: 6, dynamic: true }, evidence: 'Lennard-Jones noble-gas cluster fixture.',
    },
    'ae-ar-cluster': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020004,
        physics: profile({ vdw: true, speed_limit: true }), overlays: overlays({ forceVdw: true }),
        expected: { atomCount: 8, dynamic: true }, evidence: 'Lennard-Jones noble-gas cluster fixture.',
    },
    'ae-noble-mix': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020005,
        physics: profile({ vdw: true, speed_limit: true }), overlays: overlays({ forceVdw: true }),
        expected: { atomCount: 6, dynamic: true }, evidence: 'Mixed-species Lennard-Jones fixture.',
    },
    'ae-nacl-form': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020006,
        physics: profile({ ionic: true, vdw: true, speed_limit: true }),
        overlays: overlays({ field: true, forceIonic: true }),
        expected: { atomCount: 2, bondCount: 0, dynamic: true }, evidence: 'Effective charged-center Coulomb plus excluded-volume fixture.',
    },
    'ae-nacl-lattice': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020007,
        physics: profile({ ionic: true, vdw: true, speed_limit: true }),
        overlays: overlays({ field: true, forceIonic: true }),
        expected: { atomCount: 9, bondCount: 0, dynamic: true }, evidence: 'Finite ionic crystal fragment; no periodic Ewald sum.',
    },
    'ae-mgf2': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020008,
        physics: profile({ ionic: true, vdw: true, speed_limit: true }),
        overlays: overlays({ field: true, forceIonic: true }),
        expected: { atomCount: 3, bondCount: 0, dynamic: true }, evidence: 'Effective charged-center MgF2 assembly fixture.',
    },
    'ae-h2-form': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x020009,
        physics: profile({ vdw: true, bonds_force: true, bonding: true, speed_limit: true }),
        overlays: overlays({ forceBond: true }), expected: { atomCount: 2, dynamic: true },
        evidence: 'Distance/valence capture plus harmonic spring; not electronic covalent-bond formation.',
    },
    'ae-bond-rupture-cycle': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x02001e,
        physics: profile({ vdw: true, bonds_force: true, bonding: true }),
        parameters: Object.freeze({ dt: 0.02, softening: 0.3, thermostatTemp: 1.0 }),
        overlays: overlays({ clouds: false, shells: false, bondStyle: 'cylinders',
            velocities: true, forceBond: true, forceVdw: true, forceNet: true }),
        experiment: Object.freeze({
            protocol: 'bond-rupture-cycle', label: 'Rupture → return → recapture',
            observation: 'Track bond count, separation, bond potential, and the explicit damping intervention after recapture.',
            phases: Object.freeze([
                Object.freeze({ tick: 0, label: 'Outward rupture drive' }),
                Object.freeze({ tick: 200, label: 'Controlled return drive' }),
                Object.freeze({ tick: 950, label: 'Recapture settling' }),
            ]),
        }),
        expected: { atomCount: 2, bondCount: 1, dynamic: true, topologyCycle: true },
        evidence: '[IMPOSED] Distance/valence bond topology and harmonic spring. Timed velocity reversal and final damping are declared protocol interventions, not emergent chemical kinetics.',
    },
    'ae-o2-form': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x02000a,
        physics: profile({ vdw: true, bonds_force: true, bonding: true, speed_limit: true }),
        overlays: overlays({ forceBond: true }), expected: { atomCount: 2, dynamic: true },
        evidence: 'Empirical valence-based bond-order inference and harmonic spring.',
    },
    'ae-ch4-form': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x02000b,
        physics: profile({ vdw: true, bonds_force: true, bonding: true, angle_strain: true, speed_limit: true }),
        overlays: overlays({ forceBond: true, forceAngle: true, forceNet: true }), expected: { atomCount: 5, dynamic: true },
        evidence: 'Distance capture plus imposed VSEPR angle potential.',
    },
    'ae-water-dimer': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x02000c,
        physics: profile({ vdw: true, bonds_force: true, h_bonds: true, angle_strain: true, speed_limit: true }),
        overlays: overlays({ hbondLines: true, forceHbond: true, forceAngle: true, forceNet: true }), expected: { atomCount: 6, bondCount: 4, dynamic: true },
        evidence: 'Explicit imposed 10-12 directional H-bond plus harmonic intramolecular terms.',
    },
    'ae-water-cluster': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x02000d,
        physics: profile({ vdw: true, bonds_force: true, h_bonds: true, angle_strain: true, speed_limit: true }),
        overlays: overlays({ hbondLines: true, forceHbond: true, forceAngle: true, forceNet: true }), expected: { atomCount: 15, bondCount: 10, dynamic: true },
        evidence: 'Finite five-water network under the same imposed effective force field.',
    },
    'ae-vsepr-linear': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x02000e,
        physics: profile({ bonds_force: true, angle_strain: true, speed_limit: true }), overlays: overlays({ forceAngle: true, forceNet: true }),
        expected: { atomCount: 3, bondCount: 2, dynamic: true }, evidence: 'Harmonic angle relaxation toward the imposed linear target.',
    },
    'ae-vsepr-tetrahedral': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x02000f,
        physics: profile({ bonds_force: true, angle_strain: true, speed_limit: true }), overlays: overlays({ forceAngle: true, forceNet: true }),
        expected: { atomCount: 5, bondCount: 4, dynamic: true }, evidence: 'Harmonic angle relaxation toward the imposed tetrahedral target.',
    },
    'ae-vsepr-bent': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020010,
        physics: profile({ bonds_force: true, angle_strain: true, speed_limit: true }), overlays: overlays({ forceAngle: true, forceNet: true }),
        expected: { atomCount: 3, bondCount: 2, dynamic: true }, evidence: 'Harmonic angle relaxation toward the empirical water angle.',
    },
    'ae-thermal-gas': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x020011,
        physics: profile({ vdw: true, speed_limit: true, thermostat: true }), overlays: overlays({ velocities: true }),
        expected: { atomCount: 12, bondCount: 0, dynamic: true }, evidence: 'Lennard-Jones ensemble with an imposed Berendsen thermostat in simulation units.',
    },
    'ae-argon-thermal-cycle': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x02001f,
        physics: profile({ vdw: true, speed_limit: true, thermostat: true }),
        parameters: Object.freeze({ dt: 0.05, softening: 0.15, thermostatTemp: 1.8 }),
        overlays: overlays({ clouds: false, labels: false, shells: false,
            bondStyle: 'off', velocities: true }),
        experiment: Object.freeze({
            protocol: 'argon-thermal-cycle', label: 'Heat → quench → free evolution',
            observation: 'Compare temperature, kinetic/potential exchange, radius, and ordering across explicitly driven phases.',
            phases: Object.freeze([
                Object.freeze({ tick: 0, label: 'Heat at T*=1.80' }),
                Object.freeze({ tick: 500, label: 'Quench at T*=0.08' }),
                Object.freeze({ tick: 1500, label: 'Thermostat released' }),
            ]),
        }),
        expected: { atomCount: 27, bondCount: 0, dynamic: true, protocolTicks: 1500 },
        evidence: '[IMPOSED] Lennard-Jones cluster plus a declared Berendsen heat/quench schedule in simulation units. It is not a calibrated argon phase diagram or canonical ensemble.',
    },
    'ae-collision': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020012,
        physics: profile({ vdw: true, speed_limit: true }), overlays: overlays({ velocities: true, forceVdw: true }),
        expected: { atomCount: 2, bondCount: 0, dynamic: true }, evidence: 'Two-body Lennard-Jones momentum-conservation fixture.',
    },
    'ae-conservative-pair': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020017,
        physics: profile({ vdw: true }), overlays: overlays({ velocities: true, forceVdw: true, forceNet: true }),
        expected: { atomCount: 2, bondCount: 0, dynamic: true },
        evidence: 'Closed two-body Lennard-Jones regression fixture with no damping, thermostat, topology changes, or safety ceiling.',
    },
    'ae-damped-relaxation': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x020018,
        physics: profile({ vdw: true, damping: true, speed_limit: true }), overlays: overlays({ velocities: true, forceVdw: true }),
        expected: { atomCount: 4, bondCount: 0, dynamic: true },
        evidence: 'Explicit non-conservative damping-sink regression fixture.',
    },
    'ae-polar-dimer': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020019,
        physics: profile({ ionic: true, vdw: true, bonds_force: true, speed_limit: true, dipole_dipole: true, electronegativity: true }),
        overlays: overlays({ field: true, dipoles: true, forceIonic: true, forceDipole: true, forceNet: true }),
        expected: { atomCount: 4, bondCount: 2, dynamic: true },
        evidence: 'Effective QEq-like partial charges and point-dipole interaction; not ab initio HF electronic structure.',
    },
    'ae-dt-fusion': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x02001a,
        reaction: 'dt_fusion',
        nuclear: Object.freeze({ channel: 'dt_fusion', mode: 'single', eventLimit: 1,
            eventWeight: 1, reactivityScale: 4, transportRadius: 10,
            neutronContainment: 0.85, gammaContainment: 1, seed: 0x02001a }),
        physics: profile({ speed_limit: true }),
        parameters: Object.freeze({ dt: 0.1, softening: 0.3, thermostatTemp: 1.0 }),
        overlays: overlays({ clouds: false, shells: true, bondStyle: 'off', velocities: false,
            nuclearEvents: true, radiation: true, heat: true, nuclearBoundary: true }),
        expected: { atomCount: 2, finalAtomCount: 2, bondCount: 0, dynamic: true, reactionEvents: 1 },
        evidence: '[PARAMETRIC] D-T product identity and Q use evaluated atomic masses. The live swept-collision radius and normalized energy-dependent hazard are imposed; no tunnelling or absolute cross-section recovery is claimed.',
    },
    'ae-u235-fission': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x02001b,
        reaction: 'u235_fission',
        nuclear: Object.freeze({ channel: 'u235_fission', mode: 'single', eventLimit: 1,
            eventWeight: 1, reactivityScale: 8, transportRadius: 12,
            neutronContainment: 0.75, gammaContainment: 0.9, seed: 0x02001b }),
        physics: profile({ speed_limit: true }),
        parameters: Object.freeze({ dt: 0.1, softening: 0.3, thermostatTemp: 1.0 }),
        overlays: overlays({ clouds: false, shells: true, bondStyle: 'off', velocities: false,
            nuclearEvents: true, radiation: true, heat: true, nuclearBoundary: true }),
        expected: { atomCount: 2, finalAtomCount: 5, bondCount: 0, dynamic: true, reactionEvents: 1 },
        evidence: '[PARAMETRIC] IAEA reference product channel and evaluated atomic-mass Q. The live collision hazard, average energy partition, containment, and directions are imposed; no yield distribution is claimed.',
    },
    'ae-dt-fusion-burn': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x02001c,
        reaction: 'dt_fusion',
        nuclear: Object.freeze({ channel: 'dt_fusion', mode: 'batch', eventLimit: 12,
            eventWeight: 1e18, reactivityScale: 3, transportRadius: 14,
            neutronContainment: 0.9, gammaContainment: 1, seed: 0x02001c }),
        physics: profile({ speed_limit: true }),
        parameters: Object.freeze({ dt: 0.1, softening: 0.3, thermostatTemp: 1.0 }),
        overlays: overlays({ clouds: false, shells: true, bondStyle: 'off', velocities: false,
            nuclearEvents: true, radiation: true, heat: true, nuclearBoundary: true }),
        expected: { atomCount: 24, finalAtomCount: 24, bondCount: 0, dynamic: true, reactionEvents: 12 },
        evidence: '[PARAMETRIC] D-T identity and 17.589 MeV release use evaluated masses and standard 3.52/14.07 MeV product partition. The 10^18 ensemble weight, collision hazard, initial plasma state, and deposition time constants are imposed presentation and population parameters.',
    },
    'ae-u235-chain-reaction': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x02001d,
        reaction: 'u235_fission',
        nuclear: Object.freeze({ channel: 'u235_fission', mode: 'chain', eventLimit: 27,
            eventWeight: 1e18, reactivityScale: 12, collisionRadiusScale: 1.5,
            transportRadius: 11, boundaryMode: 'leak', moderatorStrength: 0.08,
            neutronContainment: 0.88, gammaContainment: 0.92, seed: 0x02001d }),
        physics: profile({ speed_limit: true }),
        parameters: Object.freeze({ dt: 0.1, softening: 0.3, thermostatTemp: 1.0 }),
        overlays: overlays({ clouds: false, shells: true, bondStyle: 'off', velocities: false,
            nuclearEvents: true, radiation: true, heat: true, nuclearBoundary: true }),
        expected: { atomCount: 28, bondCount: 0, dynamic: true, finiteFuel: 27, maxReactionEvents: 27 },
        evidence: '[PARAMETRIC] The 200 MeV recoverable budget follows the standard average U-235 fission partition. Free neutron trajectories, seeded energy-dependent collision hazards, ambient moderation, leakage, containment, the selected Ba/Kr channel, and the 10^18 ensemble weight define an effective transport laboratory; measured k-effective is an outcome, not an input, and this is not reactor certification.',
    },
    'ae-u235-criticality-controls': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'parametric', seed: 0x020020,
        reaction: 'u235_fission',
        nuclear: Object.freeze({ channel: 'u235_fission', mode: 'sandbox', eventLimit: 27,
            eventWeight: 1e16, reactivityScale: 8, collisionRadiusScale: 1.5,
            transportRadius: 12, boundaryMode: 'leak', moderatorStrength: 0,
            absorberStrength: 0, sourceEnabled: true, sourceRate: 0.05,
            sourceEnergyMeV: 2.53e-8, particleLimit: 256,
            neutronContainment: 0.88, gammaContainment: 0.92, seed: 0x020020 }),
        physics: profile({ speed_limit: true }),
        parameters: Object.freeze({ dt: 0.1, softening: 0.3, thermostatTemp: 1.0 }),
        overlays: overlays({ clouds: false, shells: true, bondStyle: 'off',
            nuclearEvents: true, radiation: true, heat: true, nuclearBoundary: true }),
        experiment: Object.freeze({
            protocol: 'criticality-controls', label: 'Interactive matched-seed controls',
            observation: 'Reload the same seed, change one leakage, reflection, moderation, absorption, source, or reactivity control before playback, then compare measured neutron births and resolved losses.',
            phases: Object.freeze([
                Object.freeze({ tick: 0, label: 'Open-boundary baseline' }),
            ]),
        }),
        expected: { atomCount: 28, bondCount: 0, dynamic: true, finiteFuel: 27, interactiveControls: true },
        evidence: '[PARAMETRIC] Same finite one-group U-235 transport model as the chain-reaction laboratory. Controls are explicit imposed coefficients; observed reproduction is diagnostic only and is not reactor criticality certification.',
    },
    'ae-fe-bcc': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x020013,
        physics: profile({ vdw: true, bonds_force: true, speed_limit: true }), overlays: overlays({ forceBond: true }),
        expected: { atomCount: 9, dynamic: true }, evidence: 'Finite BCC geometry with generic empirical springs; not a metallic electronic-structure model.',
    },
    'ae-cu-fcc': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x020014,
        physics: profile({ vdw: true, bonds_force: true, speed_limit: true }), overlays: overlays({ forceBond: true }),
        expected: { atomCount: 7, dynamic: true }, evidence: 'Finite FCC seed with generic empirical springs; not a metallic electronic-structure model.',
    },
    'ae-crystal-impulse-vacancy': {
        scenarioClass: 'effective_dynamics', epistemicStatus: 'imposed', seed: 0x020021,
        physics: profile({ bonds_force: true }),
        parameters: Object.freeze({ dt: 0.08, softening: 0.3, thermostatTemp: 1.0 }),
        overlays: overlays({ clouds: false, labels: false, shells: false,
            bondStyle: 'cylinders', velocities: true }),
        experiment: Object.freeze({
            protocol: 'crystal-impulse-vacancy', label: 'Matched complete/defect impulse',
            observation: 'Compare velocity and bond-energy propagation along a complete harmonic chain and an otherwise matched chain interrupted by one vacancy.',
            phases: Object.freeze([
                Object.freeze({ tick: 0, label: 'Matched impulses launched' }),
            ]),
        }),
        expected: { atomCount: 17, bondCount: 14, dynamic: true, completeChainAtoms: 9, vacancyChainAtoms: 8 },
        evidence: '[IMPOSED] Finite one-dimensional harmonic iron-record analogue for causal visualization. It does not recover metallic bonding, a phonon dispersion, or bulk defect energetics.',
    },
    'ae-periodic': {
        scenarioClass: 'static_reference', epistemicStatus: 'empirical', seed: 0x020015,
        physics: profile(), overlays: overlays({ clouds: false, shells: false, bondStyle: 'off' }),
        expected: { atomCount: 118, bondCount: 0, dynamic: false }, evidence: 'Static periodic-table parameter atlas.',
    },
    'ae-custom': {
        scenarioClass: 'sandbox', epistemicStatus: 'mixed', seed: 0x020016,
        physics: profile({ ionic: true, vdw: true, bonds_force: true, speed_limit: true }), overlays: overlays(),
        expected: { atomCount: 0, dynamic: false }, evidence: 'User-authored effective sandbox; no scenario-level validation claim.',
    },
});

export const AE_CURATED_SCENARIOS = Object.freeze(AE_SCENARIO_PRESENTATION.map((row) => Object.freeze({
    ...row,
    owner: 'js_effective_atom_engine',
    ...CONTRACTS[row.id],
    parameters: CONTRACTS[row.id]?.parameters || DEFAULT_PARAMETERS,
})));

const AE_CURATED_MAP = new Map(AE_CURATED_SCENARIOS.map((s) => [s.id, s]));

const ELEMENT_PERIODS = [
    { label: 'Period 1', start: 1, end: 2 },
    { label: 'Period 2', start: 3, end: 10 },
    { label: 'Period 3', start: 11, end: 18 },
    { label: 'Period 4', start: 19, end: 36 },
    { label: 'Period 5', start: 37, end: 54 },
    { label: 'Period 6', start: 55, end: 86 },
    { label: 'Period 7', start: 87, end: 118 },
];

/** Skip Z=1 — ae-hydrogen-atom is the canonical hydrogen entry. */
const ELEMENT_Z_SKIP = new Set([1]);

/**
 * @param {string} id
 * @returns {AEScenarioMeta | null}
 */
export function getAEScenarioMeta(id) {
    if (!id) return null;
    const curated = AE_CURATED_MAP.get(id);
    if (curated) return curated;
    if (id.startsWith('ae-el-')) {
        const Z = parseInt(id.slice(6), 10);
        const el = getElement(Z);
        if (!el) return null;
        return {
            id,
            category: 'Elements',
            title: `${Z} ${el.symbol} \u2014 ${el.name}`,
            summary: `Isolated ${el.name} atom (Z = ${Z}). Orbital clouds and shell boundary spheres are enabled; dynamics are off (locked atom).`,
            tags: ['elements', 'static'],
            scenarioClass: 'static_reference',
            epistemicStatus: 'empirical',
            owner: 'js_effective_atom_engine',
            seed: 0x020100 + Z,
            physics: profile(),
            parameters: DEFAULT_PARAMETERS,
            overlays: overlays({ shellBounds: true }),
            expected: Object.freeze({ atomCount: 1, bondCount: 0, dynamic: false }),
            evidence: 'Static empirical element-table and orbital-visualization entry.',
        };
    }
    return null;
}

/**
 * @param {HTMLSelectElement | null} select
 * @param {string} [selectedId]
 */
export function populateAEScenarioSelect(select, selectedId = AE_DEFAULT_SCENARIO) {
    if (!select) return;

    const groups = new Map();
    for (const scenario of AE_CURATED_SCENARIOS) {
        if (!groups.has(scenario.category)) groups.set(scenario.category, []);
        groups.get(scenario.category).push(scenario);
    }

    select.innerHTML = '';

    for (const [category, scenarios] of groups) {
        const group = document.createElement('optgroup');
        group.label = category;
        for (const scenario of scenarios) {
            const option = document.createElement('option');
            option.value = scenario.id;
            option.textContent = scenario.title;
            option.title = `${scenario.summary} [${scenario.epistemicStatus}]`;
            option.selected = scenario.id === selectedId;
            group.appendChild(option);
        }
        select.appendChild(group);
    }

    for (const period of ELEMENT_PERIODS) {
        const group = document.createElement('optgroup');
        group.label = period.label;
        for (let Z = period.start; Z <= period.end; Z++) {
            if (ELEMENT_Z_SKIP.has(Z)) continue;
            const el = getElement(Z);
            if (!el) continue;
            const id = `ae-el-${Z}`;
            const option = document.createElement('option');
            option.value = id;
            option.title = `Static empirical element reference for ${el.name}; no atomic quantum dynamics are integrated.`;
            option.textContent = `${Z} ${el.symbol} \u2014 ${el.name}`;
            option.selected = id === selectedId;
            group.appendChild(option);
        }
        select.appendChild(group);
    }

    if (!select.querySelector(`option[value="${selectedId}"]`)) {
        select.value = AE_DEFAULT_SCENARIO;
    } else {
        select.value = selectedId;
    }
}

export function validateAEScenarioRegistry() {
    const seen = new Set();
    const errors = [];
    for (const scenario of AE_CURATED_SCENARIOS) {
        if (seen.has(scenario.id)) errors.push(`duplicate:${scenario.id}`);
        seen.add(scenario.id);
        if (!scenario.category) errors.push(`category:${scenario.id}`);
        if (!scenario.title) errors.push(`title:${scenario.id}`);
        if (!scenario.scenarioClass) errors.push(`scenarioClass:${scenario.id}`);
        if (!scenario.epistemicStatus) errors.push(`epistemicStatus:${scenario.id}`);
        if (!scenario.owner) errors.push(`owner:${scenario.id}`);
        if (!Number.isInteger(scenario.seed)) errors.push(`seed:${scenario.id}`);
        if (!scenario.physics) errors.push(`physics:${scenario.id}`);
        if (!scenario.parameters || !Number.isFinite(scenario.parameters.dt) ||
            !Number.isFinite(scenario.parameters.softening) ||
            !Number.isFinite(scenario.parameters.thermostatTemp)) errors.push(`parameters:${scenario.id}`);
        if (!scenario.overlays) errors.push(`overlays:${scenario.id}`);
        if (!scenario.expected) errors.push(`expected:${scenario.id}`);
        if (scenario.reaction && !['dt_fusion', 'u235_fission'].includes(scenario.reaction)) {
            errors.push(`reaction:${scenario.id}`);
        }
        if (scenario.nuclear && scenario.nuclear.channel !== scenario.reaction) {
            errors.push(`nuclear.channel:${scenario.id}`);
        }
        if (scenario.experiment) {
            const experiment = scenario.experiment;
            if (!experiment.protocol || !experiment.label || !experiment.observation ||
                !Array.isArray(experiment.phases) || experiment.phases.length === 0) {
                errors.push(`experiment:${scenario.id}`);
            } else {
                let priorTick = -1;
                for (const phase of experiment.phases) {
                    if (!Number.isInteger(phase.tick) || phase.tick < 0 || phase.tick <= priorTick || !phase.label) {
                        errors.push(`experiment.phase:${scenario.id}`);
                        break;
                    }
                    priorTick = phase.tick;
                }
            }
        }
        for (const term of AE_PHYSICS_SPECS) {
            if (typeof scenario.physics?.[term.key] !== 'boolean') {
                errors.push(`physics.${term.key}:${scenario.id}`);
            }
        }
        for (const key of AE_OVERLAY_KEYS) {
            if (!(key in (scenario.overlays || {}))) {
                errors.push(`overlays.${key}:${scenario.id}`);
            } else if (key !== 'bondStyle' && typeof scenario.overlays[key] !== 'boolean') {
                errors.push(`overlays.${key}.type:${scenario.id}`);
            }
        }
        if (!AE_BOND_STYLES.includes(scenario.overlays?.bondStyle)) {
            errors.push(`overlays.bondStyle:${scenario.id}`);
        }
        for (const [overlayKey, physicsKey] of Object.entries(AE_FORCE_OVERLAY_PHYSICS)) {
            if (scenario.overlays?.[overlayKey] && !scenario.physics?.[physicsKey]) {
                errors.push(`overlays.${overlayKey}.inapplicable:${scenario.id}`);
            }
        }
        if (scenario.overlays?.forceNet &&
            !AE_PHYSICS_SPECS.some(term => term.key !== 'speed_limit' && scenario.physics?.[term.key])) {
            errors.push(`overlays.forceNet.inapplicable:${scenario.id}`);
        }
        for (const key of ['nuclearEvents', 'radiation', 'heat', 'nuclearBoundary']) {
            if (scenario.overlays?.[key] && !scenario.nuclear) {
                errors.push(`overlays.${key}.inapplicable:${scenario.id}`);
            }
        }
    }
    return { ok: errors.length === 0, errors, count: AE_CURATED_SCENARIOS.length };
}

{
    const check = validateAEScenarioRegistry();
    if (!check.ok) {
        console.warn('[scale2/scenario-registry] validation failed:', check.errors.join(', '));
    }
}
