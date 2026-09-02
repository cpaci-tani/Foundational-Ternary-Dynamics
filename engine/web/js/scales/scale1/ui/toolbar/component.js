import { getScale1ScenarioToolbarTemplate } from './template.js?v=6';
import {
    populateScale1ScenarioSelect, DEFAULT_SCALE1_SCENARIO,
    SCALE1_M3_VIEWS,
} from '../../scenario-registry.js?v=15';
import { scale1State } from '../../state/store.js?v=7';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale1ScenarioToolbarGroup() {
    const element = htmlToElement(getScale1ScenarioToolbarTemplate());
    populateScale1ScenarioSelect(
        element.querySelector('#pe-scenario-select'), DEFAULT_SCALE1_SCENARIO);
    const viewSelect = element.querySelector('#pe-m3-view-select');
    for (const view of SCALE1_M3_VIEWS) {
        const option = document.createElement('option');
        option.value = view.id;
        option.textContent = view.label;
        viewSelect.appendChild(option);
    }
    viewSelect.addEventListener('change', () => {
        scale1State.m3ViewId = viewSelect.value;
        const view = SCALE1_M3_VIEWS.find(row => row.id === viewSelect.value);
        viewSelect.dataset.uiTooltip = view?.cue || '';
        document.getElementById('panel-diagnostics')?._ftdDiagnosticsPanel?.update(true);
    });
    element.querySelector('#pe-paired-scenario')?.addEventListener('click', (event) => {
        const target = event.currentTarget?.dataset?.targetScenario;
        const select = document.getElementById('pe-scenario-select');
        if (!target || !select) return;
        select.value = target;
        select.dispatchEvent(new Event('change', { bubbles: true }));
    });
    return element;
}
