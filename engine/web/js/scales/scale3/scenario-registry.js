/**
 * Scale 3 — canonical molecule scenario contracts.
 *
 * Reference structures declare their graph explicitly. Dynamics are the
 * AtomEngine's effective classical kernels and retain their epistemic labels;
 * no entry claims ab-initio chemistry or substrate-QM recovery.
 */

import { getAllMolecules, getCategories } from '../../molecules.js';
import { AE_PHYSICS_SPECS } from '../scale2/scenario-registry.js';

export const SCALE3_DEFAULT_SCENARIO = 'mol-h2-vibration';

const PHYSICS_OFF = Object.freeze(Object.fromEntries(AE_PHYSICS_SPECS.map((term) => [term.key, false])));
const profile = (overrides = {}) => Object.freeze({ ...PHYSICS_OFF, ...overrides });
const parameters = (overrides = {}) => Object.freeze({
    dt: 0.05, softening: 0.3, thermostatTemp: 1.0, ...overrides,
});
const overlays = (overrides = {}) => Object.freeze({
    clouds: false, labels: true, shells: false, shellBounds: false, lobes: false,
    bondStyle: 'cylinders', field: false, velocities: false,
    dipoles: false, hbondLines: false,
    nuclearEvents: false, radiation: false, heat: false, nuclearBoundary: false,
    forceIonic: false, forceVdw: false, forceBond: false,
    forceHbond: false, forceAngle: false, forceDipole: false, forceNet: false,
    ...overrides,
});

const categoryLabels = new Map(getCategories().map((entry) => [entry.id, entry.label]));
const POLAR = new Set(['hcl', 'water', 'nh3', 'h2o2', 'h2s', 'methanol', 'formaldehyde',
    'ethanol', 'acetic_acid', 'glycine', 'urea', 'adenine', 'caffeine']);

function topologyComponentCount(molecule) {
    const parent = molecule.atoms.map((_, index) => index);
    const find = (value) => parent[value] === value ? value : (parent[value] = find(parent[value]));
    for (const bond of molecule.bonds) {
        const left = find(bond.a), right = find(bond.b);
        if (left !== right) parent[right] = left;
    }
    return new Set(parent.map((_, index) => find(index))).size;
}

const references = getAllMolecules().map((molecule, index) => {
    const ionic = molecule.id === 'nacl';
    const monatomic = molecule.id === 'noble';
    const covalent = molecule.bonds.length > 0;
    const cleanFormula = molecule.formula.replace(/<[^>]+>/g, '');
    return Object.freeze({
        id: `mol-${molecule.id}`,
        moleculeId: molecule.id,
        category: categoryLabels.get(molecule.category) || 'Reference Structures',
        title: `${cleanFormula} — ${molecule.name}`,
        summary: `${molecule.description} The displayed graph is declared reference topology; motion uses effective classical kernels.`,
        tags: Object.freeze(['reference', ionic ? 'ionic' : (covalent ? 'covalent' : 'atomic')]),
        scenarioClass: 'effective_dynamics',
        epistemicStatus: ionic ? 'parametric' : 'imposed',
        owner: 'js_effective_molecule_engine',
        seed: 0x030100 + index,
        physics: profile(ionic
            ? { ionic: true, vdw: true, speed_limit: true }
            : monatomic
                ? { vdw: true, speed_limit: true }
                : { vdw: true, bonds_force: true, angle_strain: true,
                    speed_limit: true, electronegativity: POLAR.has(molecule.id) }),
        parameters: parameters(),
        overlays: overlays(POLAR.has(molecule.id) ? { dipoles: true } : {}),
        expected: Object.freeze({
            atomCount: molecule.atoms.length,
            bondCount: molecule.bonds.length,
            componentCount: topologyComponentCount(molecule),
            dynamic: true,
        }),
        evidence: ionic
            ? '[PARAMETRIC] Effective charged-center Coulomb plus Lennard-Jones reference; no periodic electronic-structure solution.'
            : '[IMPOSED] Declared molecular graph plus harmonic bonds, Lennard-Jones pairs, and effective angle targets; not ab-initio chemistry.',
    });
});

const experiments = [
    {
        id: 'mol-h2-vibration', category: 'Molecular Experiments', title: 'H₂ Normal-Mode Pulse',
        summary: 'Equal and opposite velocities excite the H–H stretch while center-of-mass momentum remains zero.',
        setup: 'h2-vibration', cameraDistance: 16, epistemicStatus: 'imposed', seed: 0x030001,
        physics: profile({ vdw: true, bonds_force: true, speed_limit: true }),
        overlays: overlays({ velocities: true }),
        expected: { atomCount: 2, bondCount: 1, componentCount: 1, dynamic: true },
        evidence: '[IMPOSED] Classical two-mass harmonic normal-mode demonstration in simulation units.',
    },
    {
        id: 'mol-water-rotation', category: 'Molecular Experiments', title: 'H₂O Rigid-Body Rotation',
        summary: 'A tangential velocity field initializes near-rigid rotation and exposes rotational versus vibrational kinetic energy.',
        setup: 'water-rotation', cameraDistance: 18, epistemicStatus: 'imposed', seed: 0x030002,
        physics: profile({ vdw: true, bonds_force: true, angle_strain: true, speed_limit: true }),
        overlays: overlays({ velocities: true, dipoles: true }),
        expected: { atomCount: 3, bondCount: 2, componentCount: 1, dynamic: true },
        evidence: '[IMPOSED] Classical rigid-body initialization of a declared bent water graph.',
    },
    {
        id: 'mol-h2-dissociation', category: 'Molecular Experiments', title: 'H₂ Dissociation',
        summary: 'An outward relative impulse stretches a declared H₂ bond through the explicit break rule.',
        setup: 'h2-dissociation', cameraDistance: 18, epistemicStatus: 'imposed', seed: 0x030003,
        physics: profile({ vdw: true, bonds_force: true, bonding: true, speed_limit: true }),
        overlays: overlays({ velocities: true }),
        expected: { atomCount: 2, initialBondCount: 1, eventualBondCount: 0, dynamic: true },
        evidence: '[IMPOSED] Distance-threshold bond removal under an explicit mechanical impulse; not a quantum dissociation cross section.',
    },
    {
        id: 'mol-h2-recombination', category: 'Molecular Experiments', title: 'H + H Capture',
        summary: 'Two hydrogen records approach with zero total momentum and may enter the explicit distance/valence capture rule.',
        setup: 'h2-recombination', cameraDistance: 22, epistemicStatus: 'imposed', seed: 0x030004,
        physics: profile({ vdw: true, bonds_force: true, bonding: true, damping: true, speed_limit: true }),
        overlays: overlays({ velocities: true }),
        expected: { atomCount: 2, initialBondCount: 0, eventualBondCount: 1, dynamic: true },
        evidence: '[IMPOSED] Effective capture and damping fixture; no photon-emission or electronic transition model.',
    },
    {
        id: 'mol-water-dimer-hbond', category: 'Intermolecular Experiments', title: 'Water Dimer Hydrogen Bond',
        summary: 'Two oriented water graphs expose donor–H···acceptor geometry and the effective hydrogen-bond kernel.',
        setup: 'water-dimer', cameraDistance: 24, epistemicStatus: 'parametric', seed: 0x030005,
        physics: profile({ vdw: true, bonds_force: true, h_bonds: true, angle_strain: true,
            speed_limit: true, electronegativity: true }),
        overlays: overlays({ dipoles: true, hbondLines: true }),
        expected: { atomCount: 6, bondCount: 4, componentCount: 2, dynamic: true },
        evidence: '[PARAMETRIC] Directional effective hydrogen-bond force with incomplete scalar-potential accounting.',
    },
    {
        id: 'mol-dipole-alignment', category: 'Intermolecular Experiments', title: 'HCl Dipole Alignment',
        summary: 'Two HCl graphs begin misaligned so their effective dipole response can be inspected directly.',
        setup: 'dipole-alignment', cameraDistance: 24, epistemicStatus: 'parametric', seed: 0x030006,
        physics: profile({ vdw: true, bonds_force: true, dipole_dipole: true,
            speed_limit: true, electronegativity: true }),
        overlays: overlays({ dipoles: true }),
        expected: { atomCount: 4, bondCount: 2, componentCount: 2, dynamic: true },
        evidence: '[PARAMETRIC] Effective electronegativity-derived dipoles; the dipole potential is not in tracked total energy.',
    },
    {
        id: 'mol-molecular-collision', category: 'Intermolecular Experiments', title: 'Methane–Methane Collision',
        summary: 'Matched CH₄ molecules collide head-on with zero net momentum, exposing translation-to-internal-mode transfer.',
        setup: 'molecular-collision', cameraDistance: 34, epistemicStatus: 'imposed', seed: 0x030007,
        physics: profile({ vdw: true, bonds_force: true, angle_strain: true, speed_limit: true }),
        overlays: overlays({ velocities: true }),
        expected: { atomCount: 10, bondCount: 8, componentCount: 2, dynamic: true },
        evidence: '[IMPOSED] Finite classical molecular collision in reduced simulation units.',
    },
    {
        id: 'mol-water-thermal-cycle', category: 'Thermal Experiments', title: 'Water Heat–Quench–Release',
        summary: 'A finite water cluster is heated, quenched, then released so thermal and structural histories remain visible.',
        setup: 'water-thermal-cycle', cameraDistance: 32, epistemicStatus: 'imposed', seed: 0x030008,
        physics: profile({ vdw: true, bonds_force: true, h_bonds: true, angle_strain: true,
            thermostat: true, speed_limit: true, electronegativity: true }),
        parameters: parameters({ dt: 0.03, thermostatTemp: 1.4 }),
        overlays: overlays({ dipoles: true, hbondLines: true }),
        experiment: Object.freeze({
            protocol: 'molecular-thermal-cycle', label: 'Heat → quench → release',
            observation: 'Track temperature, molecular components, bond strain, radius of gyration, and kinetic-mode transfer.',
            phases: Object.freeze([
                Object.freeze({ tick: 0, label: 'Heat · T*=1.4' }),
                Object.freeze({ tick: 180, label: 'Quench · T*=0.12' }),
                Object.freeze({ tick: 360, label: 'Microcanonical release' }),
            ]),
        }),
        expected: { atomCount: 12, bondCount: 8, componentCount: 4, dynamic: true },
        evidence: '[IMPOSED] Berendsen thermal protocol over four effective water records; not a canonical ensemble.',
    },
].map((entry) => Object.freeze({
    tags: Object.freeze(['experiment', 'molecular-dynamics']),
    scenarioClass: 'effective_dynamics', owner: 'js_effective_molecule_engine',
    parameters: entry.parameters || parameters(),
    ...entry,
    expected: Object.freeze(entry.expected),
}));

const special = [
    Object.freeze({
        id: 'mol-crystal', category: 'Special', title: 'NaCl Crystal (3×3×3)',
        summary: 'A finite alternating-ion crystal fragment with open boundaries; it is not a periodic Ewald calculation.',
        tags: Object.freeze(['ionic', 'crystal']), scenarioClass: 'effective_dynamics',
        epistemicStatus: 'parametric', owner: 'js_effective_molecule_engine', seed: 0x0300f0,
        setup: 'nacl-crystal', cameraDistance: 38, physics: profile({ ionic: true, vdw: true, speed_limit: true }),
        parameters: parameters(), overlays: overlays({ field: true, forceIonic: true }),
        expected: Object.freeze({ atomCount: 27, bondCount: 0, componentCount: 27, dynamic: true }),
        evidence: '[PARAMETRIC] Finite charged-center crystal fragment; no periodic electrostatic sum or band structure.',
    }),
    Object.freeze({
        id: 'mol-custom', category: 'Special', title: 'Custom Molecular Sandbox',
        summary: 'An empty effective sandbox for manual atom placement, graph construction, and force selection.',
        tags: Object.freeze(['sandbox']), scenarioClass: 'sandbox', epistemicStatus: 'mixed',
        owner: 'js_effective_molecule_engine', seed: 0x0300ff, setup: 'custom',
        physics: profile({ ionic: true, vdw: true, bonds_force: true, bonding: true, speed_limit: true }),
        parameters: parameters(), overlays: overlays(),
        expected: Object.freeze({ atomCount: 0, bondCount: 0, componentCount: 0, dynamic: false }),
        evidence: 'User-authored effective sandbox; no scenario-level validation claim.',
    }),
];

export const SCALE3_SCENARIOS = Object.freeze([...experiments, ...references, ...special]);
const scenarioMap = new Map(SCALE3_SCENARIOS.map((scenario) => [scenario.id, scenario]));

export function getScale3ScenarioMeta(id) {
    return scenarioMap.get(id) || null;
}

export function populateScale3ScenarioSelect(select, selectedId = SCALE3_DEFAULT_SCENARIO) {
    if (!select) return;
    const groups = new Map();
    for (const scenario of SCALE3_SCENARIOS) {
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
            group.appendChild(option);
        }
        select.appendChild(group);
    }
    select.value = scenarioMap.has(selectedId) ? selectedId : SCALE3_DEFAULT_SCENARIO;
}

export function validateScale3ScenarioRegistry() {
    const errors = [];
    const seen = new Set();
    for (const scenario of SCALE3_SCENARIOS) {
        if (seen.has(scenario.id)) errors.push(`duplicate:${scenario.id}`);
        seen.add(scenario.id);
        for (const field of ['category', 'title', 'summary', 'scenarioClass', 'epistemicStatus', 'owner', 'evidence']) {
            if (!scenario[field]) errors.push(`${field}:${scenario.id}`);
        }
        if (!Number.isInteger(scenario.seed)) errors.push(`seed:${scenario.id}`);
        for (const spec of AE_PHYSICS_SPECS) {
            if (typeof scenario.physics?.[spec.key] !== 'boolean') errors.push(`physics.${spec.key}:${scenario.id}`);
        }
        if (!Number.isFinite(scenario.parameters?.dt) || !Number.isFinite(scenario.parameters?.softening) ||
            !Number.isFinite(scenario.parameters?.thermostatTemp)) errors.push(`parameters:${scenario.id}`);
        if (!scenario.overlays || !scenario.expected) errors.push(`presentation:${scenario.id}`);
    }
    return { ok: errors.length === 0, errors, count: SCALE3_SCENARIOS.length };
}

const validation = validateScale3ScenarioRegistry();
if (!validation.ok) console.warn('[scale3/scenario-registry] validation failed:', validation.errors.join(', '));
