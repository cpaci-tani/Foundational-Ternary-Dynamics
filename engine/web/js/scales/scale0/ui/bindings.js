import { formatS0SeedMetadata } from '../../../config/scenarios.js';
import { FORCE_FIELD_KEYS, getFieldStateSnapshot, setFieldToggle, setForceStyle } from '../state/store.js';
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
} from './dom.js';
import { COL_TO_TOGGLES } from './overlays/presets.js';

let _bound = false;

function updateScenarioMetadata(scenarioId) {
    renderScenarioDescription(scenarioId, formatS0SeedMetadata(scenarioId));
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

    // Per-axis flux-slice plane toggles (xy/xz/yz). Axis index: yz=0, xz=1, xy=2.
    // These modify which mid-planes the all-axis flux slice overlay renders; they
    // default all-on and are independent of the volume-column badge/clear.
    for (const [axisName, axisIdx] of [['xy', 2], ['xz', 1], ['yz', 0]]) {
        const btnId = `flux-slice-axis-${axisName}`;
        const axisBtn = getEl(btnId);
        if (!axisBtn) continue;
        axisBtn.addEventListener('click', () => {
            const on = !readButtonActive(btnId);
            setButtonActive(btnId, on);
            api.viewportAdapter(ctx).setFluxSliceAxisEnabled(axisIdx, on);
            api.setLatticeNeedsUpload();
        });
    }

    // Flux-volume style sub-toggles (organic scatter vs grid; additive glow on/off).
    const organicBtn = getEl('toggle-flux-organic');
    if (organicBtn) {
        organicBtn.addEventListener('click', () => {
            const on = !readButtonActive('toggle-flux-organic');
            setButtonActive('toggle-flux-organic', on);
            api.viewportAdapter(ctx).setFluxOrganic(on);
            api.setLatticeNeedsUpload();   // dot positions change → re-upload
        });
    }

    const glowBtn = getEl('toggle-flux-glow');
    if (glowBtn) {
        glowBtn.addEventListener('click', () => {
            const on = !readButtonActive('toggle-flux-glow');
            setButtonActive('toggle-flux-glow', on);
            api.viewportAdapter(ctx).setFluxGlow(on);   // live material change, no re-upload
        });
    }

    // Shared apply-toggle helper: works for both user clicks and
    // programmatic clear-column actions. `silent` means skip the
    // latticeNeedsUpload push (useful for bulk updates where we push
    // once at the end). Always keeps DOM button state, state-store
    // flag, and viewport visibility in sync.
    const setToggleState = (buttonId, fieldKey, on, { silent = false } = {}) => {
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
        if (!silent) api.setLatticeNeedsUpload();
    };

    const updateOverlayBadges = () => {
        for (const [colName, toggles] of Object.entries(COL_TO_TOGGLES)) {
            const badge = document.querySelector(`[data-count-for="${colName}"]`);
            if (!badge) continue;
            let count = 0;
            for (const buttonId of toggles) {
                if (readButtonActive(buttonId)) count++;
            }
            badge.textContent = String(count);
            badge.classList.toggle('is-zero', count === 0);
        }
    };

    // Map from buttonId → fieldKey so clear-column can look up the
    // state-store key for any managed toggle.
    const buttonIdToFieldKey = new Map(FIELD_TOGGLE_BINDINGS);

    for (const [buttonId, fieldKey] of FIELD_TOGGLE_BINDINGS) {
        const btn = getEl(buttonId);
        if (!btn) continue;
        btn.addEventListener('click', () => {
            const on = !readButtonActive(buttonId);
            setToggleState(buttonId, fieldKey, on);
            updateOverlayBadges();
        });
    }

    // Per-column × clear buttons — turn off every toggle in one column.
    for (const clearBtn of document.querySelectorAll('.s0-overlay-col-clear')) {
        const colName = clearBtn.getAttribute('data-clear-col');
        const toggles = COL_TO_TOGGLES[colName];
        if (!toggles) continue;
        clearBtn.addEventListener('click', () => {
            for (const buttonId of toggles) {
                if (!readButtonActive(buttonId)) continue;
                const fieldKey = buttonIdToFieldKey.get(buttonId);
                if (!fieldKey) continue;
                setToggleState(buttonId, fieldKey, false, { silent: true });
            }
            api.setLatticeNeedsUpload();
            updateOverlayBadges();
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

    // Initial badge sync on first bind so counts reflect whatever toggles
    // the scenario loader set up during boot.
    updateOverlayBadges();
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
