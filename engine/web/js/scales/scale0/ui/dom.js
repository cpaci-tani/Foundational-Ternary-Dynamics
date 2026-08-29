export const FIELD_TOGGLE_BINDINGS = [
    ['toggle-e-field', 'showEField'],
    ['toggle-b-field', 'showBField'],
    ['toggle-poynting', 'showPoynting'],
    ['toggle-div-field', 'showDivField'],
    ['toggle-flux-lines', 'showFluxLines'],
    ['toggle-force-em', 'showForceEM'],
    ['toggle-force-gravity', 'showForceGravity'],
    ['toggle-force-strong', 'showForceStrong'],
    ['toggle-force-weak', 'showForceWeak'],
    ['toggle-dual-substrate', 'showDualSubstrate'],
    ['toggle-chirality', 'showChirality'],
    ['toggle-dark-halo', 'showDarkMatterHalo'],
    ['toggle-damping-zones', 'showDampingZones'],
    ['toggle-genesis-iso', 'showGenesisIsosurface'],
    ['toggle-confinement', 'showConfinement'],
    ['toggle-color-charge', 'showColorCharge'],
    // Tier 1 quantum overlays — see docs/SPEC_S0_QUANTUM_OVERLAYS.md
    ['toggle-psi-squared',        'showPsiSquared'],
    ['toggle-phase',              'showPhase'],
    ['toggle-lagrangian-density', 'showLagrangianDensity'],
    ['toggle-entropy-density',    'showEntropyDensity'],
    ['toggle-grav-potential',     'showGravPotential'],
    // Physics-topology overlays — see docs/SPEC_S0_QUANTUM_OVERLAYS.md §Topology
    ['toggle-em-energy',           'showEmEnergy'],
    ['toggle-charge-density',      'showChargeDensity'],
    ['toggle-vorticity',           'showVorticity'],
    // Tier 1/2 (2026-04-18) — horizon, stress-energy split.
    ['toggle-horizon',             'showHorizon'],
    ['toggle-e-pressure',          'showEPressure'],
    ['toggle-b-pressure',          'showBPressure'],
    // New substrate overlays (2026-06-03)
    ['toggle-state-field',         'showStateField'],
    ['toggle-latency',             'showLatency'],
    ['toggle-gauss-residual',      'showGaussResidual'],
];

export const FORCE_STYLE_VALUES = ['arrows', 'heatmap', 'flow', 'glyphs'];

export function getEl(id) {
    return document.getElementById(id);
}

export function setButtonActive(id, active) {
    const el = getEl(id);
    if (el) {
        el.classList.toggle('active', !!active);
        el.setAttribute('aria-pressed', active ? 'true' : 'false');
    }
}

export function readButtonActive(id) {
    return !!getEl(id)?.classList.contains('active');
}

export function setCheckboxValue(id, value) {
    const el = getEl(id);
    if (el) el.checked = !!value;
}

export function readCheckboxValue(id, fallback = false) {
    const el = getEl(id);
    return el ? !!el.checked : fallback;
}

export function setInputValue(id, value) {
    const el = getEl(id);
    if (el) el.value = String(value);
}

export function readInputValue(id, fallback = '') {
    const el = getEl(id);
    return el ? el.value : fallback;
}

export function getSelectedScenarioId(fallback = 'flux-pulse') {
    return readInputValue('scenario-select', fallback) || fallback;
}

export function setSelectedScenarioId(id) {
    setInputValue('scenario-select', id);
}

export function setForceStyleButtons(style) {
    const row = getEl('force-style-row');
    if (!row) return;
    for (const btn of row.querySelectorAll('.style-btn')) {
        const active = btn.dataset.style === style;
        btn.classList.toggle('active', active);
        btn.setAttribute('aria-pressed', active ? 'true' : 'false');
    }
}

// Sibling of setForceStyleButtons for the volumetric-scalar render-mode
// meta-toggle (Default / Heat Map). Buttons carry data-scalar-mode.
export function setScalarRenderButtons(mode) {
    const row = getEl('scalar-render-row');
    if (!row) return;
    for (const btn of row.querySelectorAll('.style-btn')) {
        const active = btn.dataset.scalarMode === mode;
        btn.classList.toggle('active', active);
        btn.setAttribute('aria-pressed', active ? 'true' : 'false');
    }
}

export function markScenarioOverrideRows(toggleDefs) {
    const advancedSections = new Map();
    for (const [, defaultVal, elId] of toggleDefs) {
        const el = getEl(elId);
        if (!el) continue;
        const row = el.closest('.toggle-row');
        if (!row) continue;
        const details = row.closest('details.toggle-advanced');
        const overridden = el.checked !== defaultVal;
        if (row.classList.contains('scenario-override') !== overridden) {
            row.classList.toggle('scenario-override', overridden);
        }
        if (overridden) {
            if (details) advancedSections.set(details, true);
        } else {
            if (details && !advancedSections.has(details)) advancedSections.set(details, false);
        }
    }
    for (const [details, needsOpen] of advancedSections) {
        if (details.open !== needsOpen) details.open = needsOpen;
    }
}

export function renderScenarioDescription(_scenarioId, descriptionText, { preserveOpen = false } = {}) {
    const wrap = getEl('lat-scenario-desc');
    const text = getEl('lat-scenario-desc-text');
    if (!wrap || !text) return;
    const wasOpen = wrap.open;
    if (descriptionText) {
        if (text.textContent !== descriptionText) text.textContent = descriptionText;
        if (wrap.style.display !== '') wrap.style.display = '';
        // Collapsed by default — the "Epistemic status" panel is shown but closed;
        // the user expands it on demand (it is advisory detail, not primary UI).
        if (!preserveOpen && wrap.open) wrap.open = false;
        else if (preserveOpen && wrap.open !== wasOpen) wrap.open = wasOpen;
    } else {
        if (text.textContent !== '') text.textContent = '';
        if (wrap.style.display !== 'none') wrap.style.display = 'none';
        if (wrap.open) wrap.open = false;
    }
}
