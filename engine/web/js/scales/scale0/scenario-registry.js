/** Toggle bundle for FTD-0107 emergent-spectrum reproduction scenarios. */
const EMERGENT_IC_TOGGLES = Object.freeze({
    wave_propagation: true,
    gauss_projection: true,
    genesis: true,
    langevin: true,
    dual_substrate: false,
});

const DE_BROGLIE_CLOCK_TOGGLES = Object.freeze({
    wave_propagation: true,
    coupling: true,
    genesis: false,
    damping: false,
    selective_damping: false,
    weak_transmutation: false,
    dual_substrate: false,
    forces: false,
    movement: false,
    lorentz_force: false,
    gauss_projection: false,
    de_broglie_clock: true,
});

const THERMAL_IGNITION_TOGGLES = Object.freeze({
    wave_propagation: true,
    gauss_projection: true,
    genesis: true,
    langevin: true,
    dual_substrate: false,
});

const QGP_TOGGLES = Object.freeze({
    wave_propagation: true,
    gauss_projection: true,
    langevin: true,
});

const EW_PHASE_TOGGLES = Object.freeze({
    wave_propagation: true,
    gauss_projection: true,
    genesis: true,
});

function applyHarnessToggles(harness, toggles, logTag) {
    try {
        for (const [key, value] of Object.entries(toggles)) {
            harness.setToggle?.(key, value);
        }
    } catch (e) {
        if (logTag) console.warn(`[${logTag}] toggle setup partial:`, e);
    }
}

/** Pre-set emergent-ic toggles + Langevin params, then dispatch scenario seed. */
function setupEmergentSpectrumScenario(harness, scenarioId, params = {}, opts = {}) {
    applyHarnessToggles(harness, EMERGENT_IC_TOGGLES, scenarioId);
    const T = opts.langevinT ?? 0.005;
    const gamma = opts.langevinGamma ?? 0.02;
    harness.setLangevinParams?.(T, gamma);
    harness.setupScenario?.(params.id || scenarioId);
}

function activateStateFieldOverlay(delayMs = 100) {
    setTimeout(() => {
        const stateBtn = document.getElementById('toggle-state-field');
        if (stateBtn && !stateBtn.classList.contains('active')) stateBtn.click();
    }, delayMs);
}

function setupDeBroglieClockScenario(harness, params = {}) {
    harness.setupScenario(params.id || 's0-seed-de-broglie-clock');
    applyHarnessToggles(harness, DE_BROGLIE_CLOCK_TOGGLES, 's0-seed-de-broglie-clock');
    harness.setOmega0?.(0.30);
}

function setupThermalIgnitionScenario(harness, params = {}) {
    harness.setupScenario(params.id || 's0-seed-thermal-ignition');
    applyHarnessToggles(harness, THERMAL_IGNITION_TOGGLES, 's0-seed-thermal-ignition');
    harness.setLangevinTemp?.(0.03);
}

function setupQgpScenario(harness, params = {}) {
    applyHarnessToggles(harness, QGP_TOGGLES, 's0-seed-quark-gluon-plasma');
    harness.setLangevinParams?.(0.02, 0.05);
    harness.setupScenario(params.id || 's0-seed-quark-gluon-plasma');
}

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
        load(harness, params = {}) {
            harness.setupScenario(params.id || id);
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
    {
        id: 's0-seed-ew-phase-transition',
        scale: 'lattice',
        title: 'EW Phase Transition (Hysteresis)',
        category: 'Genesis & Manifestation',
        tags: ['seed', 'genesis', 'hysteresis'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[DERIVED]',
        load(harness, params = {}) {
            harness.setupScenario('empty');
            applyHarnessToggles(harness, EW_PHASE_TOGGLES, 's0-seed-ew-phase-transition');
            activateStateFieldOverlay();

            // Background flux sweep logic to display hysteresis
            let t = 0;
            const max_D = 0.05;

            if (window.__ftdEwInterval) clearInterval(window.__ftdEwInterval);
            window.__ftdEwInterval = setInterval(() => {
                const selectEl = document.getElementById('scenario-select');
                if (selectEl && selectEl.value !== 's0-seed-ew-phase-transition') {
                     clearInterval(window.__ftdEwInterval);
                     return;
                }

                const ctx = window.__ftdCtx;
                if (!ctx || !ctx.running) return;

                t += 0.01;
                const D = (Math.sin(t) + 1.0) / 2.0 * max_D;
                harness.injectUniformFluxAdd(D, 0, 0);
            }, 16);
        },
    },
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
    makeScenario('Substrate Physics', 'flux-zero-point', 'Zero-Point Energy', ['substrate', 'vacuum']),
    makeScenario('Light & EM', 'light-rainbow', 'Rainbow (3 Colors)', ['light', 'em']),
    makeScenario('Light & EM', 'light-dipole', 'Dipole Radiation', ['light', 'em']),
    makeScenario('Light & EM', 'light-two-slit', 'Two-Slit Interference', ['light', 'em']),
    makeScenario('Light & EM', 'light-photon-race', 'Photon Race', ['light', 'em']),
    makeScenario('Quantum Lab', 'quantum-born-rule', 'Born Rule Test', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-double-slit', 'Double-Slit (Quantitative)', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-eraser', 'Quantum Eraser (which-way)', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-tunnel', 'Quantum Tunneling', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-well', 'Particle in a Box', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-entangle', 'Entanglement Correlation', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-aharonov-bohm', 'Aharonov-Bohm Effect', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-casimir', 'Casimir Effect', ['quantum']),
    makeScenario('Quantum Lab', 'quantum-zeno', 'Quantum Zeno Effect', ['quantum']),
    // Audit-3 + Audit-4 2026-04-28 removals from this group:
    //   s0-seed-{electron, muon, tau, photon} — use s0-vacuum-* counterparts.
    //   s0-seed-positron — use s0-vacuum-electron + s0-seed-ee-annihilation.
    //   s0-seed-electron-l3, proton-candidate — duplicates/older variants.

    // LHC Standard Model — quark flavours (2026-04-17)
    makeScenario('SM Quarks', 's0-seed-up-quark', 'Up quark (u, 1st gen, +2/3)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-down-quark', 'Down quark (d, 1st gen, −1/3)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-strange-quark', 'Strange quark (s, 2nd gen)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-charm-quark', 'Charm quark (c, 2nd gen, m≈1.27 GeV)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-bottom-quark', 'Bottom quark (b, 3rd gen, m≈4.2 GeV)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Quarks', 's0-seed-top-quark', 'Top quark (t, 3rd gen, m≈v_Higgs)', ['seed', 'sm'], '[CONJECTURE]'),

    // LHC Standard Model — gauge + Higgs (2026-04-17)
    // Audit-4 2026-04-28 removals: s0-seed-{higgs-boson, w-boson, z-boson} —
    // use s0-vacuum-{higgs, w-boson, z-boson} (canonical).
    makeScenario('SM Bosons', 's0-seed-higgs-field', 'Higgs field vacuum (VEV background)', ['seed', 'sm'], '[CONJECTURE]'),
    makeScenario('SM Bosons', 's0-seed-gluon', 'Gluon (massless, colored)', ['seed', 'sm'], '[CONJECTURE]'),

    // LHC Standard Model — processes (2026-04-17)
    makeScenario('SM Processes', 's0-seed-beta-decay', 'Beta decay (n → p + e⁻ + ν̅, dynamic)', ['seed', 'sm', 'process'], '[CONJECTURE]'),
    makeScenario('SM Processes', 's0-seed-ee-annihilation', 'e⁺ e⁻ annihilation (collision → flux burst)', ['seed', 'sm', 'process'], '[CONJECTURE]'),
    {
        id: 's0-seed-quark-gluon-plasma',
        scale: 'lattice',
        title: 'Quark-gluon plasma (QGP, thermal deconfined)',
        category: 'SM Processes',
        tags: ['seed', 'sm', 'process'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[CONJECTURE]',
        load(harness, params = {}) {
            setupQgpScenario(harness, params);
        },
    },
    // Audit 2026-04-28 removals: s0-seed-{neutrino, quark, antiquark}.
    //   neutrino  → superseded by s0-vacuum-{electron,muon,tau}-neutrino
    //   quark/antiquark → superseded by s0-seed-{up,down,strange,charm,bottom,top}-quark
    // Audit-4 2026-04-28 removals: s0-seed-{pion, proton-l4, neutron} —
    // use s0-vacuum-{pion-charged, proton, neutron} (canonical).
    makeScenario('Atoms & Molecules', 's0-seed-hydrogen', 'Hydrogen atom', ['seed'], '[CONJECTURE]'),
    makeScenario('Atoms & Molecules', 's0-seed-helium', 'Helium atom (⁴He, 2p+2n + 1s²)', ['seed'], '[CONJECTURE]'),
    makeScenario('Atoms & Molecules', 's0-seed-h2-bond-formation', 'H₂ covalent bond formation (dynamic)', ['seed'], '[CONJECTURE]'),
    makeScenario('Life / Abiogenesis', 's0-seed-spark-of-life', 'Spark of Life (abiogenesis threshold)', ['seed', 'genesis', 'life', 'abiogenesis', 'autocatalytic', 'demo'], '[DEMO]'),
    makeScenario('Gauge / Topological', 's0-seed-wilson-loop', 'Wilson loop', ['seed'], '[CONJECTURE]'),
    makeScenario('Gauge / Topological', 's0-seed-flux-tube', 'Flux tube (q-qbar)', ['seed'], '[CONJECTURE]'),
    makeScenario('Gauge / Topological', 's0-seed-monopole', 'Magnetic monopole', ['seed'], '[CONJECTURE]'),
    makeScenario('Gauge / Topological', 's0-seed-instanton', 'Instanton', ['seed'], '[CONJECTURE]'),
    makeScenario('Gravity', 's0-seed-schwarzschild', 'Schwarzschild well', ['seed'], '[CONJECTURE]'),
    makeScenario('Gravity', 's0-seed-gravitational-lensing', 'Gravitational lensing (dynamic bending)', ['seed'], '[CONJECTURE]'),
    makeScenario('Gravity', 's0-seed-gravitational-wave', 'Gravitational wave', ['seed'], '[CONJECTURE]'),
    makeScenario('Gravity', 's0-seed-massive-body', 'Massive body (real mass)', ['seed'], '[DERIVED]'),
    // Time — gravitational clock-slowdown demos for the Time Observatory panel.
    // Thin reuse wrappers over the flux gravity wells (gravitational-wave /
    // Schwarzschild seeds); the dτ/dt readout is the |J|² latency proxy [~M]
    // (the real Poisson latency is surfaced in the panel's [C++] block).
    makeScenario('Time', 's0-seed-time-gravity-well', 'Gravity well (dτ/dt across a well)', ['seed', 'time', 'gravity'], '[CONJECTURE]'),
    makeScenario('Time', 's0-seed-time-twin-clocks', 'Twin clocks (Δτ deep vs far)', ['seed', 'time', 'gravity'], '[CONJECTURE]'),
    makeScenario('Time', 's0-seed-time-horizon', 'Horizon well (deep dilation)', ['seed', 'time', 'gravity'], '[CONJECTURE]'),
    makeScenario('Reference frame context / Observer', 's0-seed-sloop', 'sLoop (self-referential ring)', ['seed'], '[CONJECTURE]'),
    makeScenario('Reference frame context / Observer', 's0-seed-observer-cell', 'Observer cell (3³ lattice)', ['seed'], '[CONJECTURE]'),
    makeScenario('Field Configurations', 's0-field-plane-wave', 'Plane wave', ['field']),
    makeScenario('Field Configurations', 's0-field-standing-wave', 'Standing wave', ['field']),
    makeScenario('Field Configurations', 's0-field-uniform-e', 'Uniform E field', ['field']),
    makeScenario('Field Configurations', 's0-field-uniform-b', 'Uniform B field', ['field']),
    makeScenario('Field Configurations', 's0-field-photon-pulse', 'Photon pulse', ['field']),
    makeScenario('Field Configurations', 's0-field-rf-lattice-wave', 'RF lattice wave', ['field', 'rf', 'wave'], '[INSTRUMENT]'),
    makeScenario('Field Configurations', 's0-field-light-lattice-wave', 'Light lattice wave', ['field', 'light', 'wave'], '[INSTRUMENT]'),
    makeScenario('Field Configurations', 's0-field-sound-lattice-wave', 'Sound lattice proxy', ['field', 'sound', 'wave'], '[INSTRUMENT]'),
    makeScenario('Field Configurations', 's0-field-sound-collision', 'Sound lattice collision', ['field', 'sound', 'wave'], '[INSTRUMENT]'),
    makeScenario('Field Configurations', 's0-field-thomson-scattering', 'Flux recoil locked', ['field', 'light', 'charge'], '[INSTRUMENT]'),
    makeScenario('Field Configurations', 's0-field-thomson-unlocked-recoil', 'Flux recoil unlocked', ['field', 'light', 'charge'], '[MEASUREMENT]'),
    makeScenario('Field Configurations', 's0-field-spacetime-forcing-boundary', 'Spacetime forcing boundary (FTD-0253)', ['field', 'spacetime', 'locality', 'demo'], '[DEMO]+[BOUNDARY]'),
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
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-emergent-ic1', params);
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
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-emergent-ic3-collision', params);
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
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-emergent-ic4-subthreshold', params);
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
        load(harness, params = {}) {
            // Elevated Langevin T = 0.05 (10× ic1) — drives runaway genesis
            // from pure thermal noise, no flux injection.
            setupEmergentSpectrumScenario(harness, 's0-seed-emergent-ic2-thermal-runaway', params, {
                langevinT: 0.05,
            });
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
        load(harness, params = {}) {
            // Same total flux magnitude as ic1 but along body diagonal.
            setupEmergentSpectrumScenario(harness, 's0-seed-emergent-ic1-diagonal', params);
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
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-emergent-ic1-isotropic', params);
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
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-emergent-ic1-viz', params, { langevinT: 0.0 });
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
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-emergent-ic1-diagonal-viz', params, { langevinT: 0.0 });
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
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-emergent-ic1-isotropic-viz', params, { langevinT: 0.0 });
        },
    },
    {
        id: 's0-seed-cluster-law',
        scale: 'lattice',
        title: 'Genesis-Burst N(A) Law — interactive (FTD-0269)',
        category: 'Genesis-Burst N(A) Law (FTD-0269)',
        tags: ['seed', 'genesis', 'cluster', 'na-law', 'interactive'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[MEASURED — BOUNDARY, FTD-0269]',
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-cluster-law', params);
            activateStateFieldOverlay();
            // Mount the interactive fire panel + live N(A) plot (lazy import).
            import('./ui/overlays/genesis-burst-panel.js')
                .then((m) => m.mountGenesisBurstPanel(harness))
                .catch((e) => console.warn('[cluster-law] panel mount failed:', e));
        },
    },
    {
        id: 's0-seed-cluster-law-subknee',
        scale: 'lattice',
        title: 'N(A) law — sub-knee (A=12, geometry-limited)',
        category: 'Genesis-Burst N(A) Law (FTD-0269)',
        tags: ['seed', 'genesis', 'cluster', 'na-law', 'subknee', 'viz'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[VISUALISATION]',
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-cluster-law-subknee', params, { langevinT: 0.0 });
        },
    },
    {
        id: 's0-seed-cluster-law-knee',
        scale: 'lattice',
        title: 'N(A) law — the knee (A=16, 27-block escape)',
        category: 'Genesis-Burst N(A) Law (FTD-0269)',
        tags: ['seed', 'genesis', 'cluster', 'na-law', 'knee', 'viz'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[VISUALISATION]',
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-cluster-law-knee', params, { langevinT: 0.0 });
        },
    },
    {
        id: 's0-seed-cluster-law-superknee',
        scale: 'lattice',
        title: 'N(A) law — super-knee (A=40, energy budget N=k·A²)',
        category: 'Genesis-Burst N(A) Law (FTD-0269)',
        tags: ['seed', 'genesis', 'cluster', 'na-law', 'superknee', 'viz'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[VISUALISATION]',
        load(harness, params = {}) {
            setupEmergentSpectrumScenario(harness, 's0-seed-cluster-law-superknee', params, { langevinT: 0.0 });
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
    {
        id: 's0-seed-de-broglie-clock',
        scale: 'lattice',
        title: 'De Broglie Clock (pilot wave) — interactive (FTD-0271)',
        category: 'Quantum Foundations',
        tags: ['seed', 'quantum', 'de-broglie', 'pilot-wave', 'interactive'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT, FTD-0271]',
        load(harness, params = {}) {
            setupDeBroglieClockScenario(harness, params);
            activateStateFieldOverlay();
        },
    },
    {
        id: 's0-seed-thermal-ignition',
        scale: 'lattice',
        title: 'Thermal Ignition — lattice condensation (FTD-0274)',
        category: 'Quantum Foundations',
        tags: ['seed', 'thermal', 'temperature', 'condensation', 'first-order', 'interactive'],
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus: '[MEASURED — BOUNDARY, FTD-0274]',
        load(harness, params = {}) {
            setupThermalIgnitionScenario(harness, params);
            activateStateFieldOverlay();
        },
    },
];

export const SCALE0_SCENARIO_MAP = new Map(SCALE0_SCENARIOS.map((scenario) => [scenario.id, scenario]));

export function getScale0Scenario(id) {
    const scenario = SCALE0_SCENARIO_MAP.get(id);
    if (!scenario && id) {
        // C6: surface a typo'd / unregistered id instead of silently loading the
        // default. (Unknown ids legitimately fall back to flux-pulse, but quietly
        // doing so hides bugs like the B4 orphan.)
        console.warn(`[scenario-registry] unknown scenario id "${id}" — falling back to flux-pulse`);
    }
    return scenario || SCALE0_SCENARIO_MAP.get('flux-pulse');
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

// C5: run the validator once at module load so registry drift (duplicate ids,
// bad scale/category, malformed capabilities) surfaces as a console warning
// immediately, instead of the validator only ever being callable and never run.
// No-op output for a healthy registry (errors === []).
{
    const _registryCheck = validateScale0ScenarioRegistry();
    if (!_registryCheck.ok) {
        console.warn('[scenario-registry] registry validation failed:', _registryCheck.errors);
    }
}
