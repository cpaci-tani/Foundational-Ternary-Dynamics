function makeScenario(category, id, title, tags = [], epistemicStatus = '[OPEN]') {
    return {
        id,
        scale: 'lattice',
        title,
        category,
        tags,
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus,
        load({ bridge }, params = {}) {
            bridge.setupScenario(params.id || id);
        },
    };
}

export const SCALE0_SCENARIOS = [
    makeScenario('Empty', 'empty', 'Empty Lattice', ['baseline']),
    makeScenario('Wave Dynamics', 'flux-pulse', 'Flux Pulse', ['flux', 'wave']),
    makeScenario('Wave Dynamics', 'flux-dipole', 'Flux Dipole', ['flux', 'wave']),
    makeScenario('Wave Dynamics', 'flux-standing', 'Standing Wave', ['flux', 'wave']),
    makeScenario('Wave Dynamics', 'flux-nested-standing', 'Nested Standing', ['flux', 'wave']),
    makeScenario('Wave Dynamics', 'flux-soliton', 'Soliton', ['flux', 'wave']),
    makeScenario('Wave Dynamics', 'flux-interference', '4-Source Interference', ['flux', 'wave']),
    makeScenario('Wave Dynamics', 'flux-vortex', 'Flux Vortex (Spin)', ['flux', 'spin']),
    makeScenario('Wave Dynamics', 'flux-dual-substrate', 'Dual Substrate', ['flux', 'dual-substrate']),
    makeScenario('Genesis & Manifestation', 'flux-cascade', 'Genesis Cascade', ['genesis']),
    makeScenario('Genesis & Manifestation', 'flux-random-genesis', 'Random Genesis', ['genesis']),
    makeScenario('Genesis & Manifestation', 'flux-pair-production', 'Pair Production', ['genesis']),
    makeScenario('Genesis & Manifestation', 'flux-annihilation', 'Pair Annihilation', ['genesis']),
    makeScenario('Genesis & Manifestation', 'flux-vacuum-foam', 'Vacuum Fluctuations', ['genesis']),
    makeScenario('Confinement', 'flux-meson', 'Meson (Confinement)', ['confinement']),
    makeScenario('Confinement', 'flux-string-breaking', 'String Breaking', ['confinement']),
    makeScenario('Confinement', 'flux-baryon', 'Baryon (3-Quark)', ['confinement']),
    makeScenario('Substrate Physics', 'flux-cyclotron', 'Cyclotron Motion', ['substrate']),
    makeScenario('Substrate Physics', 'flux-screening', 'Charge Screening', ['substrate']),
    makeScenario('Substrate Physics', 'flux-thermalization', 'Thermalization', ['substrate']),
    makeScenario('Substrate Physics', 'flux-triad', 'Triad Formation', ['substrate']),
    makeScenario('Light & EM', 'light-rainbow', 'Rainbow (3 Colors)', ['light', 'em']),
    makeScenario('Light & EM', 'light-dipole', 'Dipole Radiation', ['light', 'em']),
    makeScenario('Light & EM', 'light-two-slit', 'Two-Slit Interference', ['light', 'em']),
    makeScenario('Light & EM', 'light-photon-race', 'Photon Race', ['light', 'em']),
    makeScenario('Quantum Lab', 'quantum-born-rule', 'Born Rule Test', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-double-slit', 'Double-Slit (Quantitative)', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-tunnel', 'Quantum Tunneling', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-well', 'Particle in a Box', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-entangle', 'Entanglement Correlation', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-aharonov-bohm', 'Aharonov-Bohm Effect', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-casimir', 'Casimir Effect', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-zeno', 'Quantum Zeno Effect', ['quantum']),
    makeScenario('SM Seeds (epistemic-tagged)', 's0-seed-electron', 'Electron seed', ['seed'], '[CONJECTURE]'),
    makeScenario('SM Seeds (epistemic-tagged)', 's0-seed-photon', 'Photon seed', ['seed'], '[CONJECTURE]'),
    makeScenario('SM Seeds (epistemic-tagged)', 's0-seed-proton-candidate', 'Proton candidate (3-cluster)', ['seed'], '[CONJECTURE]'),
    makeScenario('Elementary Particles', 's0-seed-electron-l3', 'Electron (flux-dressed)', ['seed'], '[CONJECTURE]'),
    makeScenario('Elementary Particles', 's0-seed-positron', 'Positron', ['seed'], '[CONJECTURE]'),
    makeScenario('Elementary Particles', 's0-seed-neutrino', 'Neutrino (chiral)', ['seed'], '[CONJECTURE]'),
    makeScenario('Elementary Particles', 's0-seed-quark', 'Quark (colored)', ['seed'], '[CONJECTURE]'),
    makeScenario('Elementary Particles', 's0-seed-antiquark', 'Antiquark', ['seed'], '[CONJECTURE]'),
    makeScenario('Composite Particles', 's0-seed-pion', 'Pion (quark-antiquark)', ['seed'], '[CONJECTURE]'),
    makeScenario('Composite Particles', 's0-seed-proton-l4', 'Proton (3-quark triad)', ['seed'], '[CONJECTURE]'),
    makeScenario('Composite Particles', 's0-seed-neutron', 'Neutron (3-quark triad)', ['seed'], '[CONJECTURE]'),
    makeScenario('Atoms & Molecules', 's0-seed-hydrogen', 'Hydrogen atom', ['seed'], '[CONJECTURE]'),
    makeScenario('Atoms & Molecules', 's0-seed-helium', 'Helium atom', ['seed'], '[CONJECTURE]'),
    makeScenario('Atoms & Molecules', 's0-seed-h2-molecule', 'H₂ molecule', ['seed'], '[CONJECTURE]'),
    makeScenario('Gauge / Topological', 's0-seed-wilson-loop', 'Wilson loop', ['seed'], '[CONJECTURE]'),
    makeScenario('Gauge / Topological', 's0-seed-flux-tube', 'Flux tube (q-qbar)', ['seed'], '[CONJECTURE]'),
    makeScenario('Gauge / Topological', 's0-seed-monopole', 'Magnetic monopole', ['seed'], '[CONJECTURE]'),
    makeScenario('Gauge / Topological', 's0-seed-instanton', 'Instanton', ['seed'], '[CONJECTURE]'),
    makeScenario('Gravity / Cosmology', 's0-seed-schwarzschild', 'Schwarzschild well', ['seed'], '[CONJECTURE]'),
    makeScenario('Gravity / Cosmology', 's0-seed-frw-patch', 'FRW cosmological patch', ['seed'], '[CONJECTURE]'),
    makeScenario('Gravity / Cosmology', 's0-seed-gravitational-wave', 'Gravitational wave', ['seed'], '[CONJECTURE]'),
    makeScenario('Consciousness / Observer', 's0-seed-sloop', 'sLoop (self-referential ring)', ['seed'], '[CONJECTURE]'),
    makeScenario('Consciousness / Observer', 's0-seed-observer-cell', 'Observer cell (3³ lattice)', ['seed'], '[CONJECTURE]'),
    makeScenario('Field Configurations', 's0-field-plane-wave', 'Plane wave', ['field']),
    makeScenario('Field Configurations', 's0-field-standing-wave', 'Standing wave', ['field']),
    makeScenario('Field Configurations', 's0-field-uniform-e', 'Uniform E field', ['field']),
    makeScenario('Field Configurations', 's0-field-uniform-b', 'Uniform B field', ['field']),
    makeScenario('Field Configurations', 's0-field-photon-pulse', 'Photon pulse', ['field']),
    makeScenario('Field Configurations', 's0-field-electric-dipole', 'Electric dipole', ['field']),
    makeScenario('Field Configurations', 's0-field-magnetic-dipole', 'Magnetic dipole', ['field']),
    makeScenario('Field Configurations', 's0-field-vortex-line', 'Vortex line', ['field']),
    makeScenario('Moore Seeds (geometric)', 's0-seed-octahedron', 'Octahedron (6 face-neighbors)', ['seed'], '[CONJECTURE]'),
    makeScenario('Moore Seeds (geometric)', 's0-seed-cuboctahedron', 'Cuboctahedron (12 edge-neighbors)', ['seed'], '[CONJECTURE]'),
    makeScenario('Moore Seeds (geometric)', 's0-seed-stella-octangula', 'Stella octangula (8 corners)', ['seed'], '[CONJECTURE]'),
    makeScenario('Moore Seeds (geometric)', 's0-seed-moore-cell', 'Moore cell (full 26)', ['seed'], '[CONJECTURE]'),
    makeScenario('Moore Seeds (geometric)', 's0-seed-moore-decomposition', 'Moore decomposition (3 shells)', ['seed'], '[CONJECTURE]'),
];

export const SCALE0_SCENARIO_MAP = new Map(SCALE0_SCENARIOS.map((scenario) => [scenario.id, scenario]));

export function getScale0Scenario(id) {
    return SCALE0_SCENARIO_MAP.get(id) || SCALE0_SCENARIO_MAP.get('flux-pulse');
}

export function populateScale0ScenarioSelect(select, selectedId = 'flux-pulse') {
    if (!select) return;
    const groups = new Map();
    for (const scenario of SCALE0_SCENARIOS) {
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
            option.selected = scenario.id === selectedId;
            group.appendChild(option);
        }
        select.appendChild(group);
    }
}

export function validateScale0ScenarioRegistry() {
    const seen = new Set();
    const errors = [];
    for (const scenario of SCALE0_SCENARIOS) {
        if (seen.has(scenario.id)) errors.push(`duplicate:${scenario.id}`);
        seen.add(scenario.id);
        if (scenario.scale !== 'lattice') errors.push(`scale:${scenario.id}:${scenario.scale}`);
        if (!scenario.category) errors.push(`category:${scenario.id}`);
        if (!Array.isArray(scenario.requiredCapabilities)) errors.push(`capabilities:${scenario.id}`);
    }
    return { ok: errors.length === 0, errors, count: SCALE0_SCENARIOS.length };
}
