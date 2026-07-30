import { getScale1ScenarioToolbarTemplate } from './template.js';
import {
    populateScale1ScenarioSelect, DEFAULT_SCALE1_SCENARIO,
} from '../../scenario-registry.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale1ScenarioToolbarGroup() {
    const element = htmlToElement(getScale1ScenarioToolbarTemplate());
    populateScale1ScenarioSelect(
        element.querySelector('#pe-scenario-select'), DEFAULT_SCALE1_SCENARIO);
    return element;
}
