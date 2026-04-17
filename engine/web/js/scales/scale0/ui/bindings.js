import { QUANTUM_SCENARIO_DESCRIPTIONS, formatS0SeedMetadata } from '../../../config/scenarios.js';
import { FORCE_FIELD_KEYS, getFieldStateSnapshot, setFieldToggle, setForceStyle } from '../state/store.js?v=s1';
import { getScale0Scenario, populateScale0ScenarioSelect } from '../scenario-registry.js';
import {
    FIELD_TOGGLE_BINDINGS,
    FORCE_STYLE_VALUES,
    getEl,
    getSelectedScenarioId,
    readButtonActive,
    renderScenarioDescription,
    /* v=2: Tier 1 quantum overlay bindings added — see SPEC_S0_QUANTUM_OVERLAYS.md */
    setButtonActive,
    setForceStyleButtons,
} from './dom.js?v=2';

let _bound = false;

function updateScenarioMetadata(scenarioId) {
    renderScenarioDescription(scenarioId, formatS0SeedMetadata(scenarioId));
}

function syncQuantumLabUI(ctx, scenarioId) {
    if (!scenarioId.startsWith('quantum-')) return;
    const qlabSel = getEl('qlab-experiment');
    if (qlabSel) qlabSel.value = scenarioId;
    const descEl = getEl('qlab-description');
    if (descEl && QUANTUM_SCENARIO_DESCRIPTIONS[scenarioId]) {
        descEl.textContent = QUANTUM_SCENARIO_DESCRIPTIONS[scenarioId];
    }
    ctx.switchToQuantumLabTab?.();
}

export function bindScale0UI(ctx, api) {
    if (_bound) return;
    _bound = true;

    populateScale0ScenarioSelect(getEl('scenario-select'), getSelectedScenarioId('flux-pulse'));
    updateScenarioMetadata(getSelectedScenarioId('flux-pulse'));

    const boundarySelect = getEl('boundary-select');
    if (boundarySelect) {
        boundarySelect.addEventListener('change', () => {
            ctx.applyBoundaryShape(boundarySelect.value);
        });
    }

    const reflectiveBtn = getEl('toggle-reflective');
    if (reflectiveBtn) {
        reflectiveBtn.addEventListener('click', () => {
            reflectiveBtn.classList.toggle('active');
            ctx.applyReflectiveBoundary(reflectiveBtn.classList.contains('active'));
        });
    }

    const scenarioSelect = getEl('scenario-select');
    if (scenarioSelect) {
        scenarioSelect.addEventListener('change', () => {
            ctx.pauseSimulation();
            const scenarioId = getScale0Scenario(scenarioSelect.value).id;
            api.loadScenario(ctx, scenarioId);
            updateScenarioMetadata(scenarioId);
            syncQuantumLabUI(ctx, scenarioId);
        });
    }

    const latticeSize = getEl('lattice-size');
    if (latticeSize) {
        latticeSize.addEventListener('change', () => {
            api.resize(ctx, parseInt(latticeSize.value, 10));
        });
    }

    const fluxVolBtn = getEl('toggle-flux-volume');
    if (fluxVolBtn) {
        fluxVolBtn.addEventListener('click', () => {
            const on = !readButtonActive('toggle-flux-volume');
            setButtonActive('toggle-flux-volume', on);
            api.viewportAdapter(ctx).setFluxVolumeVisible(on);
            api.setLatticeNeedsUpload();
        });
    }

    const fluxSliceBtn = getEl('toggle-flux-slice');
    if (fluxSliceBtn) {
        fluxSliceBtn.addEventListener('click', () => {
            const on = !readButtonActive('toggle-flux-slice');
            setButtonActive('toggle-flux-slice', on);
            api.viewportAdapter(ctx).setFluxSliceVisible(on);
            api.setLatticeNeedsUpload();
        });
    }

    for (const [buttonId, fieldKey] of FIELD_TOGGLE_BINDINGS) {
        const btn = getEl(buttonId);
        if (!btn) continue;
        btn.addEventListener('click', () => {
            const on = !readButtonActive(buttonId);
            setButtonActive(buttonId, on);
            setFieldToggle(fieldKey, on);

            const adapter = api.viewportAdapter(ctx);
            if (FORCE_FIELD_KEYS.has(fieldKey)) {
                const style = api.getForceStyle();
                if (style === 'arrows') {
                    adapter.setOverlayVisible(fieldKey, on);
                } else {
                    adapter.setOverlayVisible(fieldKey, false);
                    adapter.syncForceStyle(style, getFieldStateSnapshot());
                }
            } else {
                adapter.setOverlayVisible(fieldKey, on);
            }
            api.setLatticeNeedsUpload();
        });
    }

    const styleRow = getEl('force-style-row');
    if (styleRow) {
        for (const btn of styleRow.querySelectorAll('.style-btn')) {
            btn.addEventListener('click', () => {
                const style = btn.dataset.style;
                if (!FORCE_STYLE_VALUES.includes(style) || style === api.getForceStyle()) return;
                setForceStyle(style);
                setForceStyleButtons(style);
                api.viewportAdapter(ctx).syncForceStyle(style, getFieldStateSnapshot());
                api.setLatticeNeedsUpload();
            });
        }
    }
}

export function handleScale0ShortcutKey(key) {
    const shortcutMap = {
        '1': 'toggle-e-field',
        '2': 'toggle-b-field',
        '3': 'toggle-poynting',
        '4': 'toggle-div-field',
        '5': 'toggle-flux-lines',
        '6': 'toggle-force-em',
        '7': 'toggle-dual-substrate',
        '8': 'toggle-chirality',
        '9': 'toggle-light',
    };
    const buttonId = shortcutMap[key];
    if (!buttonId) return false;
    getEl(buttonId)?.click();
    return true;
}
