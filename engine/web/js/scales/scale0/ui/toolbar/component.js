import { populateScale0ScenarioSelect } from '../../scenario-registry.js';
import { getScale0ScenarioToolbarTemplate, getScale0LatticeSizeToolbarTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale0ScenarioToolbarGroup() {
    const element = htmlToElement(getScale0ScenarioToolbarTemplate());
    populateScale0ScenarioSelect(element.querySelector('#scenario-select'), 'flux-pulse');
    return element;
}

export function createScale0LatticeSizeToolbarGroup() {
    return htmlToElement(getScale0LatticeSizeToolbarTemplate());
}
