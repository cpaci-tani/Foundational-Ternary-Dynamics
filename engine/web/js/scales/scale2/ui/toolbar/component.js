import { getScale2ScenarioToolbarTemplate } from './template.js';
import { populateAEScenarioSelect, AE_DEFAULT_SCENARIO } from '../../scenario-registry.js';
import { htmlToElement } from '../../../../ui/utils/scale-toolbar.js';

export function createScale2ScenarioToolbarGroup() {
    const element = htmlToElement(getScale2ScenarioToolbarTemplate());
    populateAEScenarioSelect(element.querySelector('#ae-scenario-select'), AE_DEFAULT_SCENARIO);
    return element;
}
