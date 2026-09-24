import { enhanceScenarioPicker } from './scenario-picker.js';
import { populateScale0ScenarioSelect } from '../../scenario-registry.js';
import { getScale0ScenarioToolbarTemplate, getScale0LatticeSizeToolbarTemplate } from './template.js?v=2';
import { htmlToElement } from '../../../../ui/utils/scale-toolbar.js';

export function createScale0ScenarioToolbarGroup() {
    const element = htmlToElement(getScale0ScenarioToolbarTemplate());
    populateScale0ScenarioSelect(element.querySelector('#scenario-select'), 'flux-pulse');
    enhanceScenarioPicker(element.querySelector('#scenario-select'));
    return element;
}

export function createScale0LatticeSizeToolbarGroup() {
    return htmlToElement(getScale0LatticeSizeToolbarTemplate());
}
