// Navigation describes the subject of each setup, not its certification.
// Particle and atomic model groups retain the registry's identity disclaimers;
// random-wave and shear preparations remain field scenarios, not fluid recovery.
const CATEGORY_MEMBERS = [
    ['Baselines & Controls', [
        'empty',
        's0-seed-emergent-ic4-subthreshold',
    ]],
    ['Waves · Propagation', [
        'flux-pulse', 'flux-soliton', 'light-rainbow', 'light-dipole',
        'light-photon-race', 's0-field-plane-wave', 's0-field-photon-pulse',
        's0-field-rf-lattice-wave', 's0-field-light-lattice-wave',
        's0-field-sound-lattice-wave', 's0-field-spacetime-forcing-boundary',
    ]],
    ['Waves · Interference & Standing Modes', [
        'flux-dipole', 'flux-standing', 'flux-nested-standing',
        'flux-interference', 'flux-dual-substrate', 'light-two-slit',
        'quantum-double-slit', 's0-field-standing-wave', 's0-field-sound-collision',
    ]],
    ['Waves · Boundaries & Barriers', [
        'quantum-tunnel', 'quantum-well', 'quantum-aharonov-bohm', 'quantum-casimir',
    ]],
    ['Fields · Sources & Electric Profiles', [
        's0-seed-dynamical-flux-dressing', 'flux-screening', 'quantum-eraser',
        's0-field-uniform-e', 's0-field-electric-dipole',
    ]],
    ['Fields · Magnetic Profiles', [
        's0-seed-monopole', 's0-field-uniform-b', 's0-field-magnetic-dipole',
    ]],
    ['Energy · Storage & Boundaries', [
        's0-cell-capacitor', 's0-cell-torus', 's0-cell-torus-reverse',
        's0-cell-torus-scrambled', 's0-cell-torus-open', 's0-cell-torus-walled',
        's0-cell-triad', 's0-cell-torus-membrane',
    ]],
    ['Energy · Driving & Transfer', [
        's0-cell-torus-membrane-gated', 's0-cell-membrane-pumped',
        's0-cell-membrane-transfer', 's0-cell-membrane-pumped-resonant',
    ]],
    ['Collective Fields · Noise & Shear', [
        'flux-vacuum-foam', 'flux-thermalization', 'flux-zero-point',
        's0-field-shear-layer',
    ]],
    ['State Dynamics · Genesis & Decay', [
        'flux-cascade', 'flux-random-genesis', 'flux-genesis-between-gates',
        's0-seed-ew-phase-transition', 'flux-pair-production', 'quantum-born-rule',
        'quantum-zeno', 's0-seed-beta-decay', 's0-seed-spark-of-life',
        's0-seed-emergent-ic1', 's0-seed-emergent-ic3-collision',
        's0-seed-emergent-ic2-thermal-runaway', 's0-seed-emergent-ic1-diagonal',
        's0-seed-emergent-ic1-isotropic', 's0-seed-emergent-ic1-viz',
        's0-seed-emergent-ic1-diagonal-viz', 's0-seed-emergent-ic1-isotropic-viz',
        's0-seed-cluster-law', 's0-seed-cluster-law-subknee',
        's0-seed-cluster-law-knee', 's0-seed-cluster-law-superknee',
        's0-seed-thermal-ignition',
    ]],
    ['Particle Motion & Collisions', [
        's0-seed-moving-source-reciprocity', 'flux-annihilation', 'flux-meson',
        'flux-string-breaking', 'flux-baryon', 'flux-cyclotron', 'quantum-entangle',
        's0-seed-ee-annihilation', 's0-seed-quark-gluon-plasma',
        's0-field-thomson-scattering', 's0-field-thomson-unlocked-recoil',
    ]],
    ['Particle Models · Leptons', [
        's0-vacuum-electron', 's0-vacuum-muon', 's0-vacuum-tau',
        's0-vacuum-positron', 's0-vacuum-antimuon', 's0-vacuum-antitau',
        's0-vacuum-electron-neutrino', 's0-vacuum-muon-neutrino',
        's0-vacuum-tau-neutrino', 's0-vacuum-electron-antineutrino',
        's0-vacuum-muon-antineutrino', 's0-vacuum-tau-antineutrino',
    ]],
    ['Particle Models · Quarks', [
        's0-seed-up-quark', 's0-seed-down-quark', 's0-seed-strange-quark',
        's0-seed-charm-quark', 's0-seed-bottom-quark', 's0-seed-top-quark',
        's0-seed-anti-up-quark', 's0-seed-anti-down-quark',
        's0-seed-anti-strange-quark', 's0-seed-anti-charm-quark',
        's0-seed-anti-bottom-quark', 's0-seed-anti-top-quark',
    ]],
    ['Particle Models · Bosons', [
        's0-seed-higgs-field', 's0-seed-gluon', 's0-vacuum-photon',
        's0-vacuum-w-boson', 's0-vacuum-w-minus-boson', 's0-vacuum-z-boson',
        's0-vacuum-higgs',
    ]],
    ['Particle Models · Hadrons', [
        's0-vacuum-proton', 's0-vacuum-neutron', 's0-vacuum-pion-charged',
        's0-vacuum-pion-neutral', 's0-vacuum-kaon-charged',
    ]],
    ['Atomic & Molecular Models', [
        's0-seed-hydrogen', 's0-seed-helium', 's0-seed-h2-bond-formation',
    ]],
    ['Gravity & Clocks', [
        's0-seed-schwarzschild', 's0-seed-gravitational-lensing',
        's0-seed-gravitational-wave', 's0-seed-massive-body',
        's0-seed-time-gravity-well', 's0-seed-time-twin-clocks',
        's0-seed-time-horizon', 's0-seed-de-broglie-clock',
    ]],
    ['Geometry & Topology', [
        'flux-vortex', 'flux-triad', 's0-seed-wilson-loop', 's0-seed-flux-tube',
        's0-seed-instanton', 's0-seed-sloop', 's0-seed-observer-cell',
        's0-field-vortex-line', 's0-seed-octahedron', 's0-seed-cuboctahedron',
        's0-seed-stella-octangula', 's0-seed-moore-cell', 's0-seed-moore-decomposition',
    ]],
];

export const SCALE0_SCENARIO_CATEGORIES = Object.freeze(
    CATEGORY_MEMBERS.map(([category]) => category),
);
const categoryById = new Map();
const categoryOrder = new Map();
for (const [order, [category, ids]] of CATEGORY_MEMBERS.entries()) {
    categoryOrder.set(category, order);
    for (const id of ids) {
        if (categoryById.has(id)) throw new Error(`Duplicate Scale 0 category assignment: ${id}`);
        categoryById.set(id, category);
    }
}

// Require a deliberate assignment when adding a scenario; do not silently
// publish it under a generic fallback or infer physics from an ID prefix.
export function getScale0ScenarioCategory(id) {
    if (!categoryById.has(id)) throw new Error(`Missing Scale 0 scenario category: ${id}`);
    return categoryById.get(id);
}

export function compareScale0ScenarioCategories(a, b) {
    return categoryOrder.get(a.category) - categoryOrder.get(b.category);
}
