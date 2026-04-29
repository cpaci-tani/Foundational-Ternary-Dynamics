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
    // Audit-3 + Audit-4 2026-04-28 removals from this group:
    //   s0-seed-{electron, muon, tau, photon} \u2014 use s0-vacuum-* counterparts.
    //   s0-seed-positron \u2014 use s0-vacuum-electron + s0-seed-ee-annihilation.
    //   s0-seed-electron-l3, proton-candidate \u2014 duplicates/older variants.

    // LHC Standard Model — quark flavours (2026-04-17)
    makeScenario('SM Quarks', 's0-seed-up-quark', 'Up quark (u, 1st gen, +2/3)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-down-quark', 'Down quark (d, 1st gen, \u22121/3)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-strange-quark', 'Strange quark (s, 2nd gen)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-charm-quark', 'Charm quark (c, 2nd gen, m\u22481.27 GeV)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-bottom-quark', 'Bottom quark (b, 3rd gen, m\u22484.2 GeV)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-top-quark', 'Top quark (t, 3rd gen, m\u2248v_Higgs)', ['seed', 'sm'], '[CONJECTURE]'),

    // LHC Standard Model — gauge + Higgs (2026-04-17)
    // Audit-4 2026-04-28 removals: s0-seed-{higgs-boson, w-boson, z-boson} \u2014
    // use s0-vacuum-{higgs, w-boson, z-boson} (canonical).
    makeScenario('SM Bosons', 's0-seed-higgs-field', 'Higgs field vacuum (VEV background)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Bosons', 's0-seed-gluon', 'Gluon (massless, colored)', ['seed', 'sm'], '[CONJECTURE]'),

    // LHC Standard Model — processes (2026-04-17)
    makeScenario('SM Processes', 's0-seed-beta-decay', 'Beta decay (n \u2192 p + e\u207b + \u03bd\u0304, dynamic)', ['seed', 'sm', 'process'], '[CONJECTURE]'),
    makeScenario('SM Processes', 's0-seed-ee-annihilation', 'e\u207a e\u207b annihilation (collision \u2192 flux burst)', ['seed', 'sm', 'process'], '[CONJECTURE]'),
    // Audit 2026-04-28 removals: s0-seed-{neutrino, quark, antiquark}.
    //   neutrino  → superseded by s0-vacuum-{electron,muon,tau}-neutrino
    //   quark/antiquark → superseded by s0-seed-{up,down,strange,charm,bottom,top}-quark
    // Audit-4 2026-04-28 removals: s0-seed-{pion, proton-l4, neutron} —
    // use s0-vacuum-{pion-charged, proton, neutron} (canonical).
    makeScenario('Atoms & Molecules', 's0-seed-hydrogen', 'Hydrogen atom', ['seed'], '[CONJECTURE]'),
    makeScenario('Atoms & Molecules', 's0-seed-helium', 'Helium atom (⁴He, 2p+2n + 1s²)', ['seed'], '[CONJECTURE]'),
    // s0-seed-h2-molecule renamed 2026-04-28 → s0-seed-2-hydrogen-atoms (the body
    // places two independent H atoms side-by-side with no shared bonding orbital,
    // so the new name reflects the actual topology).
    makeScenario('Atoms & Molecules', 's0-seed-2-hydrogen-atoms', 'Two hydrogen atoms (no bond)', ['seed'], '[CONJECTURE]'),
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

    // FTD-0102 / FTD-0107 emergent-spectrum reproduction.
    // Custom load() pre-sets the required toggles before injecting the seed.
    {
        id: 's0-seed-emergent-ic1',
        scale: 'lattice',
        title: 'Emergent ic1 (FTD-0107: 25-voxel L¹-ball-radius-2 cluster)',
        category: 'Emergent Bound States (FTD-0107)',
        tags: ['seed', 'emergent', 'cluster'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[STRUCTURAL HYPOTHESIS]',
        load({ bridge }, params = {}) {
            // Required toggle state per `campaign_emergent_spectrum_2026-04-27.cpp`:
            //   wave_propagation, gauss_projection, genesis, langevin (T=0.005, γ=0.02).
            // dual_substrate must be OFF.
            // The langevin_T / langevin_gamma controls live in the dashboard's
            // Langevin slider section; user can adjust if desired but T=0.005
            // is the FTD-0107 measured value.
            try {
                bridge.setToggle('wave_propagation', true);
                bridge.setToggle('gauss_projection', true);
                bridge.setToggle('genesis', true);
                bridge.setToggle('langevin', true);
                bridge.setToggle('dual_substrate', false);
                if (typeof bridge.setLangevinParams === 'function') {
                    bridge.setLangevinParams(0.005, 0.02);
                }
            } catch (e) {
                console.warn('[s0-seed-emergent-ic1] toggle setup partial:', e);
            }
            bridge.setupScenario(params.id || 's0-seed-emergent-ic1');
        },
    },
    {
        id: 's0-seed-emergent-ic3-collision',
        scale: 'lattice',
        title: 'Emergent ic3 (FTD-0107: 2-cluster collision, 2-3 voxels each)',
        category: 'Emergent Bound States (FTD-0107)',
        tags: ['seed', 'emergent', 'cluster', 'collision'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[STRUCTURAL HYPOTHESIS]',
        load({ bridge }, params = {}) {
            try {
                bridge.setToggle('wave_propagation', true);
                bridge.setToggle('gauss_projection', true);
                bridge.setToggle('genesis', true);
                bridge.setToggle('langevin', true);
                bridge.setToggle('dual_substrate', false);
                if (typeof bridge.setLangevinParams === 'function') {
                    bridge.setLangevinParams(0.005, 0.02);
                }
            } catch (e) {
                console.warn('[s0-seed-emergent-ic3-collision] toggle setup partial:', e);
            }
            bridge.setupScenario(params.id || 's0-seed-emergent-ic3-collision');
        },
    },
    {
        id: 's0-seed-emergent-ic4-subthreshold',
        scale: 'lattice',
        title: 'Emergent ic4 (FTD-0107: sub-threshold, 0 voxels — negative control)',
        category: 'Emergent Bound States (FTD-0107)',
        tags: ['seed', 'emergent', 'control'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[STRUCTURAL HYPOTHESIS]',
        load({ bridge }, params = {}) {
            try {
                bridge.setToggle('wave_propagation', true);
                bridge.setToggle('gauss_projection', true);
                bridge.setToggle('genesis', true);
                bridge.setToggle('langevin', true);
                bridge.setToggle('dual_substrate', false);
                if (typeof bridge.setLangevinParams === 'function') {
                    bridge.setLangevinParams(0.005, 0.02);
                }
            } catch (e) {
                console.warn('[s0-seed-emergent-ic4-subthreshold] toggle setup partial:', e);
            }
            bridge.setupScenario(params.id || 's0-seed-emergent-ic4-subthreshold');
        },
    },
    {
        id: 's0-seed-emergent-ic2-thermal-runaway',
        scale: 'lattice',
        title: 'Emergent ic2 (FTD-0107: thermal-driven runaway — unstable phase)',
        category: 'Emergent Bound States (FTD-0107)',
        tags: ['seed', 'emergent', 'runaway', 'thermal'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[STRUCTURAL HYPOTHESIS]',
        load({ bridge }, params = {}) {
            // Elevated Langevin T = 0.05 (10× ic1) — drives runaway genesis
            // from pure thermal noise, no flux injection.
            try {
                bridge.setToggle('wave_propagation', true);
                bridge.setToggle('gauss_projection', true);
                bridge.setToggle('genesis', true);
                bridge.setToggle('langevin', true);
                bridge.setToggle('dual_substrate', false);
                if (typeof bridge.setLangevinParams === 'function') {
                    bridge.setLangevinParams(0.05, 0.02);   // 10× ic1
                }
            } catch (e) {
                console.warn('[s0-seed-emergent-ic2-thermal-runaway] toggle setup partial:', e);
            }
            bridge.setupScenario(params.id || 's0-seed-emergent-ic2-thermal-runaway');
        },
    },
    {
        id: 's0-seed-emergent-ic1-diagonal',
        scale: 'lattice',
        title: 'Emergent ic1 — body-diagonal injection (D3g: Z₄ vs Z₃ test)',
        category: 'Emergent Bound States (FTD-0107)',
        tags: ['seed', 'emergent', 'cluster', 'D3g', 'diagonal'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[STRUCTURAL HYPOTHESIS]',
        load({ bridge }, params = {}) {
            // Same total flux magnitude as ic1 but along body diagonal.
            // Predicted: k = 1/3 (Z_3 about body diagonal) → 33-voxel cluster
            // if the cluster-efficiency origin is the rotation cycle around
            // the injection axis. If k stays at ¼ → 25-voxel cluster, the
            // origin is N_base (global, not direction-specific).
            try {
                bridge.setToggle('wave_propagation', true);
                bridge.setToggle('gauss_projection', true);
                bridge.setToggle('genesis', true);
                bridge.setToggle('langevin', true);
                bridge.setToggle('dual_substrate', false);
                if (typeof bridge.setLangevinParams === 'function') {
                    bridge.setLangevinParams(0.005, 0.02);
                }
            } catch (e) {
                console.warn('[s0-seed-emergent-ic1-diagonal] toggle setup partial:', e);
            }
            bridge.setupScenario(params.id || 's0-seed-emergent-ic1-diagonal');
        },
    },
    {
        id: 's0-seed-emergent-ic1-isotropic',
        scale: 'lattice',
        title: 'Emergent ic1 — isotropic 6-axis injection (D3h: full O_h symmetry test)',
        category: 'Emergent Bound States (FTD-0107)',
        tags: ['seed', 'emergent', 'cluster', 'D3h', 'isotropic'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[STRUCTURAL HYPOTHESIS]',
        load({ bridge }, params = {}) {
            // Symmetrise the injection direction. Predicts: cluster has
            // full O_h symmetry — no +x/−x asymmetry as in standard ic1.
            try {
                bridge.setToggle('wave_propagation', true);
                bridge.setToggle('gauss_projection', true);
                bridge.setToggle('genesis', true);
                bridge.setToggle('langevin', true);
                bridge.setToggle('dual_substrate', false);
                if (typeof bridge.setLangevinParams === 'function') {
                    bridge.setLangevinParams(0.005, 0.02);
                }
            } catch (e) {
                console.warn('[s0-seed-emergent-ic1-isotropic] toggle setup partial:', e);
            }
            bridge.setupScenario(params.id || 's0-seed-emergent-ic1-isotropic');
        },
    },
    {
        id: 's0-seed-emergent-ic1-viz',
        scale: 'lattice',
        title: 'Emergent ic1 — clean view (T=0, no thermal background)',
        category: 'Emergent Bound States — Clean View (T=0)',
        tags: ['seed', 'emergent', 'cluster', 'viz', 'clean'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[VISUALISATION]',
        load({ bridge }, params = {}) {
            try {
                bridge.setToggle('wave_propagation', true);
                bridge.setToggle('gauss_projection', true);
                bridge.setToggle('genesis', true);
                bridge.setToggle('langevin', true);
                bridge.setToggle('dual_substrate', false);
                if (typeof bridge.setLangevinParams === 'function') {
                    bridge.setLangevinParams(0.0, 0.02);   // T=0
                }
            } catch (e) { console.warn('[ic1-viz]', e); }
            bridge.setupScenario(params.id || 's0-seed-emergent-ic1-viz');
        },
    },
    {
        id: 's0-seed-emergent-ic1-diagonal-viz',
        scale: 'lattice',
        title: 'Emergent ic1 body-diagonal — clean view (T=0)',
        category: 'Emergent Bound States — Clean View (T=0)',
        tags: ['seed', 'emergent', 'cluster', 'viz', 'clean', 'diagonal'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[VISUALISATION]',
        load({ bridge }, params = {}) {
            try {
                bridge.setToggle('wave_propagation', true);
                bridge.setToggle('gauss_projection', true);
                bridge.setToggle('genesis', true);
                bridge.setToggle('langevin', true);
                bridge.setToggle('dual_substrate', false);
                if (typeof bridge.setLangevinParams === 'function') {
                    bridge.setLangevinParams(0.0, 0.02);
                }
            } catch (e) { console.warn('[ic1-diag-viz]', e); }
            bridge.setupScenario(params.id || 's0-seed-emergent-ic1-diagonal-viz');
        },
    },
    {
        id: 's0-seed-emergent-ic1-isotropic-viz',
        scale: 'lattice',
        title: 'Emergent ic1 isotropic — clean view (T=0)',
        category: 'Emergent Bound States — Clean View (T=0)',
        tags: ['seed', 'emergent', 'cluster', 'viz', 'clean', 'isotropic'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[VISUALISATION]',
        load({ bridge }, params = {}) {
            try {
                bridge.setToggle('wave_propagation', true);
                bridge.setToggle('gauss_projection', true);
                bridge.setToggle('genesis', true);
                bridge.setToggle('langevin', true);
                bridge.setToggle('dual_substrate', false);
                if (typeof bridge.setLangevinParams === 'function') {
                    bridge.setLangevinParams(0.0, 0.02);
                }
            } catch (e) { console.warn('[ic1-iso-viz]', e); }
            bridge.setupScenario(params.id || 's0-seed-emergent-ic1-isotropic-viz');
        },
    },
    // s0-seed-symmetry-regression removed 2026-04-28 (audit removal): engine CI
    // regression artefact (voxel_uniform() determinism check), not user-facing
    // physics. Fold into engine/tests/ as a ctest if still needed.

    // ── Vacuum Particles (s0-vacuum-* group, 2026-04-28) ───────────────
    // 15 single-particle-in-vacuum scenarios. See
    // engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md for the catalog.
    makeScenario('Vacuum Particles', 's0-vacuum-electron',          'Electron in vacuum (e⁻)',                 ['vacuum', 'lepton'],   '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-muon',              'Muon in vacuum (μ⁻)',                     ['vacuum', 'lepton'],   '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-tau',               'Tau in vacuum (τ⁻)',                      ['vacuum', 'lepton'],   '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-electron-neutrino', 'Electron neutrino in vacuum (ν_e)',       ['vacuum', 'lepton', 'neutrino'], '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-muon-neutrino',     'Muon neutrino in vacuum (ν_μ)',           ['vacuum', 'lepton', 'neutrino'], '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-tau-neutrino',      'Tau neutrino in vacuum (ν_τ)',            ['vacuum', 'lepton', 'neutrino'], '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-photon',            'Photon in vacuum (γ)',                    ['vacuum', 'gauge'],    '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-w-boson',           'W boson in vacuum (W±)',                  ['vacuum', 'gauge'],    '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-z-boson',           'Z boson in vacuum (Z⁰)',                  ['vacuum', 'gauge'],    '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-higgs',             'Higgs boson in vacuum (H)',               ['vacuum', 'gauge'],    '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-proton',            'Proton in vacuum (p)',                    ['vacuum', 'baryon'],   '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-neutron',           'Neutron in vacuum (n)',                   ['vacuum', 'baryon'],   '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-pion-charged',      'Charged pion in vacuum (π±)',             ['vacuum', 'meson'],    '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-pion-neutral',      'Neutral pion in vacuum (π⁰)',             ['vacuum', 'meson'],    '[CONJECTURE]'),
    makeScenario('Vacuum Particles', 's0-vacuum-kaon-charged',      'Charged kaon in vacuum (K±)',             ['vacuum', 'meson'],    '[CONJECTURE]'),
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
