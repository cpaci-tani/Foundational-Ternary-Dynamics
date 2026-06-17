import { getScale2ScenarioToolbarTemplate } from './template.js';
import { populateAEScenarioSelect, AE_DEFAULT_SCENARIO } from '../../scenario-registry.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale2ScenarioToolbarGroup() {
    const element = htmlToElement(getScale2ScenarioToolbarTemplate());
    populateAEScenarioSelect(element.querySelector('#ae-scenario-select'), AE_DEFAULT_SCENARIO);
    return element;
}
