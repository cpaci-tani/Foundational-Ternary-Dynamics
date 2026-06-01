function setTooltip(el, text, { force = false, source = 'definition' } = {}) {
    if (!(el instanceof HTMLElement) || !text) return;
    const existing = el.dataset.uiTooltip;
    const existingSource = el.dataset.uiTooltipSource || '';
    if (!force && existing && existingSource !== 'title') return;
    el.dataset.uiTooltip = text;
    el.dataset.uiTooltipSource = source;
}

function setTooltipForSelector(root, selector, text, options) {
    if (root instanceof HTMLElement && root.matches(selector)) {
        setTooltip(root, text, options);
    }
    root.querySelectorAll(selector).forEach((el) => setTooltip(el, text, options));
}

function normalizeLabel(text) {
    return (text || '')
        .replace(/\s+/g, ' ')
        .replace(/[()]/g, '')
        .trim();
}

const SELECTOR_TOOLTIPS = [
    ['#btn-play',       'Global play/pause. Freezes or resumes the simulation loop. Keyboard shortcut: Space.'],
    ['#btn-local-play', 'Local play/pause. Freezes scenario physics; visualization keeps animating. Keyboard: Shift+Space.'],
    ['#btn-step', 'Advance the simulation by exactly one step without entering continuous play mode. Keyboard shortcut: S.'],
    ['#btn-reset', 'Reset the active scale to its current scenario defaults. Keyboard shortcut: R.'],
    ['#engine-mode', 'Choose which simulation scale and renderer stack the dashboard is currently driving.'],
    ['label[for="engine-mode"]', 'Select the active simulation scale. Changing scale swaps controllers, overlays, and panel context.'],
    ['#ticks-per-frame', 'Adjust simulation speed. Lower values slow time; higher values advance more ticks per rendered frame.'],
    ['label[for="ticks-per-frame"]', 'Simulation speed control.'],
    ['#tpf-display', 'Current simulation speed expressed as ticks per rendered frame.'],
    ['#btn-settings', 'Open theme, scale, and interface settings. Keyboard shortcut: Ctrl+,'],
    ['#btn-ftd-assistant', 'Open the assistant sidebar. This is reserved for an eventual FTD-tuned local language model and research copilot.'],
    ['#btn-toolbar-menu', 'Open compact toolbar controls on smaller screens.'],
    ['#scenario-select', 'Choose the active Scale 0 lattice scenario.'],
    ['label[for="scenario-select"]', 'Scenario selector for the active Scale 0 lattice setup.'],
    ['#lattice-size', 'Set the lattice edge length. Larger sizes expose more room for emergence but cost more compute.'],
    ['label[for="lattice-size"]', 'Lattice edge dimension selector.'],
    ['#pe-scenario-select', 'Choose the active Scale 1 particle-engine scenario.'],
    ['label[for="pe-scenario-select"]', 'Scenario selector for the particle engine.'],
    ['#ae-scenario-select', 'Choose the active Scale 2 atom-engine scenario.'],
    ['label[for="ae-scenario-select"]', 'Scenario selector for the atom engine.'],
    ['#mol-scenario-select', 'Choose the active Scale 3 molecular scenario.'],
    ['label[for="mol-scenario-select"]', 'Scenario selector for the molecular engine.'],
    ['#planetary-scenario-select', 'Choose the active Scale 4 planetary sandbox scenario.'],
    ['label[for="planetary-scenario-select"]', 'Scenario selector for the planetary engine.'],
    ['#cosmic-scenario-select', 'Choose the active Scale 5 cosmic scenario.'],
    ['label[for="cosmic-scenario-select"]', 'Scenario selector for the cosmic engine.'],
    ['#cosmic-camera-select', 'Switch between preset camera framings for the active cosmic scenario.'],
    ['label[for="cosmic-camera-select"]', 'Camera preset selector for the cosmic renderer.'],
    ['#bg-select', 'Choose the environment or panorama behind the simulation viewport.'],
    ['#boundary-select', 'Select the active simulation boundary geometry used for confinement and reflection.'],
    ['#toggle-axes', 'Show or hide the global axis indicator in the viewport.'],
    ['#toggle-grid', 'Show or hide the global reference grid beneath the simulation.'],
    ['#toggle-reflective', 'Toggle reflective boundaries so particles and fields bounce instead of exiting.'],
    ['.vcp-label', 'Viewport control label. These selectors adjust shared environment and boundary settings.'],
    ['#status-state', 'Current run-state indicator for the simulation loop.'],
    ['#status-tick', 'Current integer tick of the active simulation.'],
    ['#status-ptime', 'Current physical or presentation time reported by the active scale.'],
    ['#status-particles', 'Current manifested or simulated particle count for the active scale.'],
    ['#status-energy', 'Current total energy readout reported by the active simulation.'],
    ['#status-fps', 'Approximate UI frame rate in frames per second.'],
    ['#status-engine', 'Current backend engine in use: Native, WASM, or Mock.'],
    ['#status-compute', 'Current compute path, typically CPU or GPU.'],
    ['#raycast-threshold', 'Inspector selection radius for point clouds. Increase it to make selection easier in sparse scenes.'],
    ['#raycast-threshold-val', 'Current inspector point-cloud hit radius.'],
    ['#cosmic-tb-bodies', 'Scale 5 telemetry: current number of simulated cosmic bodies.'],
    ['#cosmic-tb-tick', 'Scale 5 telemetry: current cosmic simulation tick.'],
    ['#cosmic-tb-hubble', 'Scale 5 telemetry: current Hubble-like expansion parameter.'],
];

// Scale 0 diagnostic rows.
//
// Key format: the row label after `normalizeLabel` (whitespace collapsed,
// parentheses stripped). Tooltip text covers the definition, formula,
// unit, and — where relevant — what the user should watch it for. Long
// enough to educate, short enough to skim on hover.
//
// IMPORTANT: Keys must match the descriptor's `label` field after
// normalizeLabel. Silent mismatches are common — run the runtime-verify
// block at the bottom of this file during development to catch them.
const SCALE0_DIAGNOSTIC_TOOLTIPS = {
    // ─── Particle State ────────────────────────────────────────────────
    'Manifested': 'Voxels whose ternary state s \u2260 0 (either +1 or \u22121). These are the lattice sites currently carrying "crystallised" matter. Unit: count.',
    'Positive':   'Voxels with s = +1. Part of the manifested count. Unit: count.',
    'Negative':   'Voxels with s = \u22121. Part of the manifested count. Unit: count.',
    // Descriptor label is "Charge (net)" — normalizeLabel strips the parens
    // so the live key is "Charge net".
    'Charge net': 'Net manifested charge: Positive \u2212 Negative. Conserved modulo Gauss-projection correction; drift here signals a broken charge-balance invariant. Unit: count.',
    'Spin Up/Down': 'Manifested sites partitioned by spin channel. For tower scenarios this is where the \u00b11\u20442 split shows up.',
    'Color R/G/B': 'Occupancy of the three SU(3) color channels from triad-binding logic. A colour-confined proton appears as (1,1,1); a free quark as one non-zero channel.',
    'Colorless':  'Sites whose colour composition sums to the neutral state \u2014 meson pairs or symmetric triad combinations.',

    // ─── Energy Budget ─────────────────────────────────────────────────
    'Total Energy': 'Sum of all lattice energy terms: Field + Wave + Particle KE + Coulomb PE. Should stay constant (modulo Dissipation) \u2014 drift \u226b O(1) per tick indicates a broken conservation law.',
    'Field |J|²': '\u222b \u00bd|J|\u00b2 dV \u2014 flux-field energy (analogue of A\u00b7A/2 in a gauge theory). This is the energy stored in the substrate itself, independent of any wave motion.',
    'Wave |w|²': '\u222b \u00bd|\u2202\u209cJ|\u00b2 dV \u2014 wave-substrate kinetic energy. Rises when the field is in motion (propagating pulses), drops in stationary configurations.',
    'Particle KE': '\u03a3 \u00bdm|v|\u00b2 over manifested particles. Only non-zero when the Movement toggle is on AND particles have accumulated velocity.',
    'Coulomb PE': 'Pairwise electrostatic energy: \\(\\sum \\alpha Q_i Q_j / (4\\pi r_{ij})\\). Computed from the Poisson-solved potential when that toggle is on.',
    'Total Flux': '\u222b |J| dV \u2014 integrated flux magnitude across the whole lattice. Doesn\'t separate kinetic and potential \u2014 use this as a "something is happening" indicator.',
    'Entropy':    'Shannon entropy of the ternary-state distribution across all voxels: \\(-\\sum p_s \\log p_s\\). Rises with disorder, falls under condensation / symmetry-breaking.',

    // ─── Electromagnetic ──────────────────────────────────────────────
    'E-Field |E|²/2': '\u222b \u00bd|E|\u00b2 dV with E = \u2212\u2202\u209cJ. Electric-field energy density. Non-zero wherever flux is changing in time.',
    'B-Field |B|²/2': '\u222b \u00bd|B|\u00b2 dV with B = \u2207\u00d7J. Magnetic-field energy density. Non-zero wherever flux has rotational structure.',
    'Poynting |S|': '|E \u00d7 B| summed across the lattice. The magnitude of electromagnetic energy flow. Points along the Poynting vector \u2014 direction hidden in this scalar.',
    'Angular Mom':  'Total angular momentum L = \u03a3 r \u00d7 p. Vector quantity (x,y,z). Should be conserved in closed lattices; non-zero in rotating / orbiting configurations.',

    // ─── Constraints ──────────────────────────────────────────────────
    // Descriptor uses U+2212 (minus sign) in the label; matching here.
    'Gauss Σdiv J−s²': 'Global Gauss-law residual: \u222b (\u2207\u00b7J \u2212 \u03c1)\u00b2 dV. Should be \u2248 0 when Gauss Projection is on \u2014 non-zero values flag either a disabled projection or a solver convergence problem. Unit: E*\u00b2.',
    'Max Gauss err': 'Largest local |\u2207\u00b7J \u2212 \u03c1| anywhere on the lattice. Catches pointwise violations that the summed residual could mask (e.g. a single badly-converged voxel). Unit: E*.',
    'Self-field inj': 'Energy added per tick by the self-field correction term \u2014 a stability patch that keeps particles from sitting in their own singular potential. Large values indicate numerical sourcing worth investigating.',

    // ─── Dual Substrate ───────────────────────────────────────────────
    // Descriptor labels include underscore: "E_L (left)" \u2192 normalised "E_L left".
    'E_L left':  'Left-handed substrate energy channel. Populated only when the Dual Substrate toggle is on; otherwise this reads 0.',
    'E_R right': 'Right-handed substrate energy channel. Its balance vs E_L drives the Chirality diagnostic below.',
    'Chirality': 'Dimensionless left/right asymmetry: \\((E_L - E_R) / (E_L + E_R)\\). 0 = parity-symmetric, \u00b11 = fully polarised. The weak-force toggle biases this.',
    'Wave L / R': 'Wave-substrate energy split by chirality. Reads a pair \\((|w_L|^2, |w_R|^2)\\); lets you see whether asymmetry lives in the static field or in propagating modes.',
};

const SCALE0_SECTION_TOOLTIPS = {
    // Section headers (rendered by DiagnosticsTable as <h3 class="diag-section-title">).
    'Particle State':  'Counts and channel breakdown for the currently manifested lattice matter \u2014 totals, charge split, spin, and colour decomposition.',
    'Energy Budget':   'Full breakdown of the lattice energy. If Total Energy is drifting, each sub-term here tells you where the leak lives.',
    'Electromagnetic': 'E, B, and Poynting diagnostics derived from the flux-field\'s time derivative and curl. These populate only when the E/B overlays and the wave-propagation toggle are both active.',
    'Constraints':     'Residuals from the physical constraints the solver is supposed to enforce (Gauss, self-field stability). Near-zero means solid; growing values indicate numerical trouble.',
    'Dual Substrate':  'Left / right substrate split and chiral-asymmetry summary. All rows here read 0 until the Dual Substrate toggle is on.',
    // Sparkline section labels (separate UI element).
    'Charge':  'Sparkline history for net charge evolution over the recent tick window.',
    'Flux':    'Sparkline history for total flux evolution over the recent tick window.',
    'Energy':  'Sparkline history for total energy evolution \u2014 the #1 place to catch conservation violations.',
};

// Scale 1 Particle Engine — conservation-summary rows. Each corresponds to
// a class .pe-conservation-row block at the top of PE telemetry.
const PE_ROW_TOOLTIPS = {
    'Energy': 'Total particle-engine energy KE + PE_coulomb + PE_gravity. In well-behaved closed scenarios this should be constant \u2014 watch it in combination with Drift below.',
    '|p|':    'Magnitude of the total linear momentum \u03a3 m\u1d62 v\u1d62. Conserved under translation invariance; drift here flags a broken Newton\'s-third-law pairing in the force calculation.',
    '|L|':    'Magnitude of the total angular momentum \u03a3 r\u1d62 \u00d7 m\u1d62 v\u1d62. Conserved under rotational invariance; useful for verifying orbit integrators.',
    'Drift':  'Relative energy drift (E \u2212 E\u2080)/|E\u2080| since the baseline tick. Under 0.01 % = excellent, 0.1 % = acceptable, \u226b 1 % = integrator needs shorter step.',
};

// Scale 1 PE card titles. Keys match the card title after normalizeLabel
// (strips parens, collapses whitespace).
const PE_CARD_TOOLTIPS = {
    'Particles':          'Active Scale 1 particles in the simulation (excludes locked particles that don\'t integrate).',
    'Virial 2K/|U|':      'Virial-theorem ratio 2\u27e8K\u27e9 / |\u27e8U\u27e9|. Equals 1 for a steady bound system, > 1 if the system is unbound, < 1 if the KE hasn\'t ramped up to equilibrium yet.',
    'Temperature MeV':    'Mean kinetic energy per particle scaled by 2/3: \\(T_{\\text{sim}} = (2/3)\\langle K\\rangle / N\\). Reported in MeV (sim units, k_B = 1). Meaningful only for statistical-ensemble-sized N; for N=2 this is just 2/3 of the mean KE — not an SI temperature.',
    'RMS Velocity c':     'Root-mean-square particle speed \u221a\u27e8|v|\u00b2\u27e9 expressed in units of c (so 0.5 = half the speed of light on the lattice).',
    'System Radius lu':   'Characteristic radius \\(\\langle|r - R_\\mathrm{CoM}|\\rangle\\) \u2014 average distance of particles from the centre of mass. Grows as the system expands, shrinks as it contracts.',
    'Tick':               'Particle-engine tick counter. PE is integrated in its own loop; may run at a different rate than the Scale-0 lattice tick.',
    'KE MeV':             'Total kinetic energy \u03a3 \u00bd m\u1d62 v\u1d62\u00b2 in MeV.',
    'PE MeV':             'Total potential energy across all enabled force terms (Coulomb + gravity at minimum), in MeV.',
    'CoM':                'Centre-of-mass position vector \u03a3 m\u1d62 r\u1d62 / \u03a3 m\u1d62, in lattice units.',
    'PE Coulomb MeV':     'Electrostatic \\(\\sum \\alpha q_i q_j / (4\\pi r_{ij})\\) summed over all pairs. Negative for unlike-sign bound systems, positive for like-sign.',
    'PE Gravity MeV':     'Gravitational \\(-\\sum G_N m_i m_j / r_{ij}\\) summed over pairs. Always negative; grows more negative as bodies fall together.',
    // Two-body specific cards ("Orbital Analytics" section).
    'Separation r lu':    'Instantaneous two-body separation |r\u2081 \u2212 r\u2082| in lattice units. Oscillates between perihelion and aphelion for bound orbits.',
    'Reduced Mass μ MeV': 'Reduced mass \u03bc = m\u2081 m\u2082 / (m\u2081 + m\u2082). The effective mass in the equivalent one-body Kepler problem.',
    'Spec. Ang. Mom h':   'Specific angular momentum h = |r \u00d7 v|. Conserved along Kepler orbits \u2014 useful for detecting integrator drift.',
    'Semi-major a lu':    'Semi-major axis a of the osculating Kepler ellipse, computed from energy and angular momentum. For a bound orbit: E = \u2212G M \u03bc / 2a.',
    'Eccentricity e':     'Eccentricity e = \u221a(1 \u2212 b\u00b2/a\u00b2). 0 = perfect circle, 1 = parabolic escape, > 1 = hyperbolic flyby.',
    'Period T':           'Estimated orbital period T = 2\u03c0\u221a(a\u00b3/GM) via Kepler\'s third law. Compare against the tick-counted actual period.',
    'Vis-viva Check':     'Residual of the vis-viva equation v\u00b2 = GM(2/r \u2212 1/a). Should read \u2248 0 \u2014 non-zero means the integrator has drifted off the Kepler surface.',
    'Phase Space r, v_r': 'Canvas plot of radial position \\(r\\) vs radial velocity \\(v_r\\). Bound orbits trace closed loops here; escaping trajectories spiral outward.',
};

// Scale 1 PE particle table column headers. Keys match <th> textContent
// after normalizeLabel.
const PE_TABLE_HEADER_TOOLTIPS = {
    'ID':     'Stable particle identifier, preserved across ticks. Use this to track a specific particle over time in the time-series panel.',
    'q':      'Particle charge in lattice units. Signed; +1, \u22121, 0 are the common values.',
    'm MeV':  'Particle rest mass in MeV. Derived from the FTD mass formulae for the relevant species.',
    '|r| lu': 'Particle distance from the world origin in lattice units.',
    '|v| c':  'Particle speed as a fraction of the lattice light-speed c = 1/\u221a3.',
    '|a|':    'Acceleration magnitude (change in velocity per tick).',
    '|F| Pl': 'Net force magnitude in lattice-native Planck-like units. Includes every enabled force term combined.',
    'KE MeV': 'Per-particle kinetic energy \u00bd m |v|\u00b2 in MeV.',
    'Lk':     'Lock status: locked particles don\'t integrate motion (useful for pinning test charges at fixed positions).',
};

// Scale 2/3 Atom-Engine diagnostics. Shown in .scale-ae panel-grid rows
// adjacent to but NOT inside #pe-telemetry. Keys match card-title text
// after normalizeLabel (so "Kinetic Energy (sim)" \u2192 "Kinetic Energy sim",
// "Mass (K_B)" \u2192 "Mass KB" because the DOM has <sub>B</sub>).
const AE_CARD_TOOLTIPS = {
    // ─── Core counts ──────────────────────────────────────────────────
    'Atom Count':   'Number of atoms currently in the AtomEngine. In Scale 2 this is one atom; in Scale 3 it can be hundreds of atoms belonging to molecules.',
    'Bond Count':   'Number of covalent bonds tracked by the engine. A water molecule H\u2082O has 2; methane has 4; zero in Scale 2.',

    // ─── Energy terms (sim units) ─────────────────────────────────────
    // Audit P1-19 (2026-05-31): these are raw MD sim-unit energies. The
    // engine applies no eV calibration — formatEnergy(value, 2) only
    // auto-scales the magnitude suffix, it does not convert sim units to
    // eV. The cards are suffixed "(sim)" so they no longer claim eV.
    'Kinetic Energy sim': '⚠ Sim-unit energy (no eV calibration applied). Total KE across all atoms \\(\\sum \\tfrac{1}{2}m|v|^2\\) in the AtomEngine\'s native MD units. Related to the sim-unit Temperature proxy via \\(T_{\\text{sim}} = 2\\langle KE\\rangle/(3 N)\\).',
    'Total Energy sim':   '⚠ Sim-unit energy (no eV calibration applied). KE + PE_ionic + PE_vdw + PE_bonds in native MD units. Should be conserved when no thermostat is attached.',
    'PE Ionic sim':       '⚠ Sim-unit energy (no eV calibration applied). Electrostatic energy from atom partial charges: \\(\\sum k q_i q_j / r\\). Usually positive (repulsion) for salts; dominant term in ionic crystals.',
    'PE Van der Waals sim': '⚠ Sim-unit energy (no eV calibration applied). Lennard-Jones 12-6 potential summed over non-bonded atom pairs: 4\u03b5[(\u03c3/r)\u00b9\u00b2 \u2212 (\u03c3/r)\u2076]. Captures hard-core repulsion + weak dispersion attraction.',
    'PE Bonds sim':      '⚠ Sim-unit energy (no eV calibration applied). Harmonic bond potential \\(\\sum \\tfrac{1}{2} k_b (r - r_0)^2\\) for every covalent bond. Zero at equilibrium length, grows quadratically with strain.',

    // ─── Thermo + momentum ────────────────────────────────────────────
    'Temperature sim': '\u26a0 Sim-unit equipartition proxy: \\(T_{\\text{sim}} = 2\\langle KE\\rangle / (3 N)\\) with implicit k_B = 1. NOT kelvin \u2014 no Boltzmann conversion is applied, so the value (suffixed "(sim)") is in the same sim units as the AE energy cards. (Audit P0-10: kelvin claim corrected 2026-05-27.)',
    'Momentum |p|':  'Magnitude of the total linear momentum. Conserved in a closed system; non-conservation flags a bug.',

    // ─── Bookkeeping ──────────────────────────────────────────────────
    'AE Tick':      'AtomEngine tick counter. Runs faster than Scale-0 ticks because atomic vibration rates are orders of magnitude below the lattice speed limit.',
    'Energy Drift': 'Cumulative percent drift of Total Energy from its initial value. Same interpretation as the Scale-1 Drift \u2014 well-tuned scenarios stay below 0.1 %.',

    // ─── Atomic-scale properties ──────────────────────────────────────
    'Atomic Mass':  'The selected element\'s standard atomic mass in atomic mass units (u). Sourced from the periodic-table data table.',
    'Nuclear B.E.': 'Nuclear binding energy from the Semi-Empirical Mass Formula (SEMF), in MeV. Energy released if the nucleus were fully separated into free nucleons.',
    'B/A MeV':      'Binding energy per nucleon B/A. Peaks at \u2248 8.8 MeV around iron \u2014 this curve is what drives fusion (low A) and fission (high A) energetics.',
    'Electron B.E.': 'Total electron binding energy from the Thomas–Fermi atomic-binding prefactor: \\(E_{\\text{atom}} \\approx -20.93 \\cdot Z^{7/3}\\) eV (Lieb–Simon 1977, [IMPOSED — external]). Not a shell-summed Slater-hydrogenic calculation despite earlier tooltip wording. Typical magnitude: tens to hundreds of eV for light atoms; thousands for heavy.',
    // Card title is "Mass (K_B)" with <sub>B</sub>; textContent renders as "Mass (KB)" \u2192 normalized "Mass KB".
    'Mass KB':      'Composite atomic mass in units of the PDG electron mass \\(m_e = 0.510999\\) MeV (the divisor used in atomic-energy.js), including both nuclear and electron binding corrections. Despite the K_B-labelled key, the value is NOT divided by the FTD anchor K_B = 0.511; the two agree by construction to ~0.2%. This card is display-only — Scale-0 genesis uses K_GENESIS = N_c · K_B directly, not this value (audit P0-11 / P1-19).',
};

function annotateScale0Diagnostics(root) {
    // The DiagnosticsTable component renders rows as
    //   <tr data-row="..."><td class="diag-metric">Label</td>...</tr>
    // NOT the legacy <dt>/<dd> pairs. The old selector silently matched
    // nothing, so every Scale 0 diagnostic hover showed no tooltip at
    // all. Target .diag-metric and propagate the tooltip to the whole
    // row so hovering any cell (value, unit, trend) reveals the text.
    root.querySelectorAll('#panel-diagnostics .diag-metric').forEach((metricEl) => {
        const label = normalizeLabel(metricEl.textContent);
        const text = SCALE0_DIAGNOSTIC_TOOLTIPS[label];
        if (!text) return;
        setTooltip(metricEl, text);
        const row = metricEl.closest('tr');
        if (row) {
            setTooltip(row, text);
            row.querySelectorAll('td').forEach((td) => setTooltip(td, text));
        }
    });

    // Section title h3s are emitted by DiagnosticsTable with the class
    // `.diag-section-title`. Sparkline-section labels under the legacy
    // panel-resources template still use `.combo-section-label`, so we
    // cover both selectors.
    root.querySelectorAll('#panel-diagnostics .diag-section-title, #panel-diagnostics .combo-section-label').forEach((labelEl) => {
        const label = normalizeLabel(labelEl.textContent);
        const text = SCALE0_SECTION_TOOLTIPS[label];
        if (text) setTooltip(labelEl, text);
    });
}

function annotateAEDiagnostics(root) {
    // Atom Engine cards sit inside `.scale-ae` panel-grids, NOT inside
    // #pe-telemetry \u2014 the PE telemetry annotator wouldn't reach them.
    // Iterate every AE card and match its title against AE_CARD_TOOLTIPS.
    root.querySelectorAll('.scale-ae .card').forEach((card) => {
        const titleEl = card.querySelector('.card-title');
        const valueEl = card.querySelector('.stat-value');
        const key = normalizeLabel(titleEl?.textContent);
        const text = AE_CARD_TOOLTIPS[key];
        if (!text) return;
        setTooltip(card, text);
        if (titleEl) setTooltip(titleEl, text);
        if (valueEl) setTooltip(valueEl, text);
    });
}

function annotatePETelemetry(root) {
    root.querySelectorAll('#pe-telemetry .pe-conservation-row').forEach((row) => {
        const labelEl = row.querySelector('.pe-cons-label');
        const valueEl = row.querySelector('.pe-cons-value');
        const key = normalizeLabel(labelEl?.textContent);
        const text = PE_ROW_TOOLTIPS[key];
        if (!text) return;
        setTooltip(row, text);
        setTooltip(labelEl, text);
        setTooltip(valueEl, text);
    });

    root.querySelectorAll('#pe-telemetry .card').forEach((card) => {
        const titleEl = card.querySelector('.card-title');
        const valueEl = card.querySelector('.stat-value');
        const key = normalizeLabel(titleEl?.textContent);
        const text = PE_CARD_TOOLTIPS[key];
        if (!text) return;
        setTooltip(card, text);
        setTooltip(titleEl, text);
        if (valueEl) setTooltip(valueEl, text);
    });

    root.querySelectorAll('#pe-telemetry th').forEach((th) => {
        const key = normalizeLabel(th.textContent);
        const text = PE_TABLE_HEADER_TOOLTIPS[key];
        if (text) setTooltip(th, text);
    });
}

function annotateStatusItems(root) {
    const groups = [
        ['#status-state', 'Current run-state indicator for the simulation loop.'],
        ['#status-tick', 'Current integer tick of the active simulation.'],
        ['#status-ptime', 'Current physical or presentation time reported by the active scale.'],
        ['#status-particles', 'Current manifested or simulated particle count for the active scale.'],
        ['#status-energy', 'Current total energy reported by the active simulation.'],
        ['#status-fps', 'Approximate UI frame rate in frames per second.'],
        ['#status-engine', 'Current backend engine implementation.'],
        ['#status-compute', 'Current compute path used by the engine.'],
    ];

    groups.forEach(([selector, text]) => {
        const el = root.querySelector(selector);
        if (!el) return;
        setTooltip(el, text, { force: true });
        const item = el.closest('.status-item');
        if (item) setTooltip(item, text, { force: true });
    });
}

export function applyUiTooltipDefinitions(root = document) {
    if (!(root instanceof Document) && !(root instanceof HTMLElement) && !(root instanceof DocumentFragment)) {
        return;
    }

    SELECTOR_TOOLTIPS.forEach(([selector, text]) => setTooltipForSelector(root, selector, text, { force: true }));
    annotateStatusItems(root);
    annotateScale0Diagnostics(root);
    annotatePETelemetry(root);
    annotateAEDiagnostics(root);
}
