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
    ['toggle-light', 'showLight'],
    ['toggle-dark-halo', 'showDarkMatterHalo'],
    ['toggle-damping-zones', 'showDampingZones'],
    ['toggle-genesis-iso', 'showGenesisIsosurface'],
    ['toggle-confinement', 'showConfinement'],
];

export const FORCE_STYLE_VALUES = ['arrows', 'heatmap', 'flow', 'glyphs'];

export function getEl(id) {
    return document.getElementById(id);
}

export function setButtonActive(id, active) {
    const el = getEl(id);
    if (el) el.classList.toggle('active', !!active);
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
        btn.classList.toggle('active', btn.dataset.style === style);
    }
}

export function markScenarioOverrideRows(toggleDefs) {
    const advDetails = document.querySelector('.toggle-advanced');
    let advNeedsOpen = false;
    for (const [, defaultVal, elId] of toggleDefs) {
        const el = getEl(elId);
        if (!el) continue;
        const row = el.closest('.toggle-row');
        if (!row) continue;
        if (el.checked !== defaultVal) {
            row.classList.add('scenario-override');
            if (advDetails && advDetails.contains(el)) advNeedsOpen = true;
        } else {
            row.classList.remove('scenario-override');
        }
    }
    if (advDetails) advDetails.open = advNeedsOpen;
}

export function renderScenarioDescription(_scenarioId, descriptionText) {
    const wrap = getEl('lat-scenario-desc');
    const text = getEl('lat-scenario-desc-text');
    if (!wrap || !text) return;
    if (descriptionText) {
        text.textContent = descriptionText;
        wrap.style.display = '';
        wrap.open = true;
    } else {
        text.textContent = '';
        wrap.style.display = 'none';
        wrap.open = false;
    }
}
