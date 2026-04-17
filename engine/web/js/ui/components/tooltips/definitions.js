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
    ['#btn-play',       'Global play/pause. Freezes the whole simulation. Keyboard: Space.'],
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
    ['#cs-scenario-select', 'Choose the active Scale 11 consciousness scenario.'],
    ['label[for="cs-scenario-select"]', 'Scenario selector for the consciousness engine.'],
    ['#cs-figure-select', 'Choose the figure or glyph family used by the consciousness visualization.'],
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

const SCALE0_DIAGNOSTIC_TOOLTIPS = {
    'Manifested': 'Count of lattice sites currently manifesting a non-void ternary state.',
    'Positive': 'Count of manifested lattice sites carrying positive ternary charge.',
    'Negative': 'Count of manifested lattice sites carrying negative ternary charge.',
    'Charge net': 'Net manifested charge, computed as positive minus negative occupancy.',
    'Spin Up/Down': 'Manifested site counts split by the two spin channels.',
    'Color R/G/B': 'Occupancy of the emergent red, green, and blue color channels.',
    'Colorless': 'Manifested sites whose color composition resolves to a neutral state.',
    'Total Energy': 'Aggregate energy estimate across field, wave, and particle terms.',
    'Field |J|²': 'Energy proxy stored in the flux-field magnitude.',
    'Wave |w|²': 'Energy proxy stored in the wave substrate.',
    'Particle KE': 'Kinetic-energy contribution from manifested particle motion.',
    'Coulomb PE': 'Electrostatic potential-energy contribution.',
    'Total Flux': 'Aggregate flux magnitude or transport across the lattice.',
    'Entropy': 'Coarse disorder metric for the current lattice configuration.',
    'E-Field |E|²/2': 'Electric-field energy-density proxy.',
    'B-Field |B|²/2': 'Magnetic-field energy-density proxy.',
    'Poynting |S|': 'Magnitude of electromagnetic energy flow.',
    'Angular Mom': 'Total angular momentum vector of the current lattice state.',
    'Gauss Σdiv J-s²': 'Global Gauss-law residual across the substrate.',
    'Max Gauss err': 'Largest local Gauss-law residual at any sampled site.',
    'Self-field inj': 'Energy injected by self-field stabilization or correction terms.',
    'EL left': 'Left-handed substrate energy channel.',
    'ER right': 'Right-handed substrate energy channel.',
    'Chirality': 'Left/right asymmetry metric across the dual substrate.',
    'Wave L / R': 'Left- and right-handed wave occupancy summary.',
};

const SCALE0_SECTION_TOOLTIPS = {
    'Particle State': 'Counts and channel breakdown for currently manifested lattice matter.',
    'Energy Budget': 'High-level decomposition of the current lattice energy accounting.',
    'Electromagnetic': 'Field-energy and energy-flow diagnostics derived from E and B structure.',
    'Constraints': 'Constraint residuals and projection-quality metrics for the lattice solver.',
    'Dual Substrate': 'Diagnostics for the left/right substrate split and chiral balance.',
    'Manifested': 'Sparkline history for manifested lattice occupancy.',
    'Charge': 'Sparkline history for net charge evolution.',
    'Flux': 'Sparkline history for total flux evolution.',
    'Energy': 'Sparkline history for total energy evolution.',
    'Entropy': 'Sparkline history for lattice entropy evolution.',
};

const PE_ROW_TOOLTIPS = {
    'Energy': 'Total particle-engine energy. This should remain stable in well-behaved closed scenarios.',
    '|p|': 'Magnitude of total system momentum.',
    '|L|': 'Magnitude of total system angular momentum.',
    'Drift': 'Percent drift from the initial energy baseline.',
};

const PE_CARD_TOOLTIPS = {
    'Particles': 'Number of active Scale 1 particles currently in the simulation.',
    'Virial 2K/|U|': 'Virial-ratio diagnostic for bound-system balance.',
    'Temperature MeV': 'Effective temperature estimate in MeV units.',
    'RMS Velocity c': 'Root-mean-square particle velocity in units of c.',
    'System Radius lu': 'Characteristic system radius in lattice units.',
    'Tick': 'Current particle-engine tick.',
    'KE MeV': 'Total kinetic energy for the particle system.',
    'PE MeV': 'Total potential energy for the particle system.',
    'CoM': 'Center-of-mass position vector.',
    'PE Coulomb MeV': 'Electrostatic contribution to potential energy.',
    'PE Gravity MeV': 'Gravitational contribution to potential energy.',
    'Separation r lu': 'Instantaneous separation between the two orbital bodies.',
    'Reduced Mass μ MeV': 'Reduced mass for the current two-body configuration.',
    'Spec. Ang. Mom h': 'Specific angular momentum for the current two-body orbit.',
    'Semi-major a lu': 'Semi-major axis estimated from current orbital state.',
    'Eccentricity e': 'Orbital eccentricity estimated from the current two-body state.',
    'Period T': 'Estimated orbital period for the current two-body system.',
    'Vis-viva Check': 'Consistency check against the vis-viva orbital relation.',
    'Phase Space r, v_r': 'Radial position versus radial velocity trace for the current two-body state.',
};

const PE_TABLE_HEADER_TOOLTIPS = {
    'ID': 'Stable particle identifier.',
    'q': 'Particle charge in simulation units.',
    'm MeV': 'Particle rest mass in MeV.',
    '|r| lu': 'Distance from the origin in lattice units.',
    '|v| c': 'Speed in units of c.',
    '|a|': 'Acceleration magnitude.',
    '|F| Pl': 'Force magnitude in Planck-like units used by this UI.',
    'KE MeV': 'Per-particle kinetic energy in MeV.',
    'Lk': 'Whether the particle is locked or constrained.',
};

function annotateScale0Diagnostics(root) {
    root.querySelectorAll('#panel-diagnostics .scale0-only dt').forEach((dt) => {
        const label = normalizeLabel(dt.textContent);
        const text = SCALE0_DIAGNOSTIC_TOOLTIPS[label];
        if (!text) return;
        setTooltip(dt, text);
        const dd = dt.nextElementSibling;
        if (dd) setTooltip(dd, text);
    });

    root.querySelectorAll('#panel-diagnostics .scale0-only .combo-section-label').forEach((labelEl) => {
        const label = normalizeLabel(labelEl.textContent);
        const text = SCALE0_SECTION_TOOLTIPS[label];
        if (text) setTooltip(labelEl, text);
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
}
