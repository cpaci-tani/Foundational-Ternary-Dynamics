import { getScale3ScenarioToolbarTemplate } from './template.js';
import { htmlToElement } from '../../../../ui/utils/scale-toolbar.js';

export function createScale3ScenarioToolbarGroup() {
    return htmlToElement(getScale3ScenarioToolbarTemplate());
}
