import { getScale4ScenarioToolbarTemplate } from './template.js?v=4';
import { htmlToElement } from '../../../../ui/utils/scale-toolbar.js';

export function createScale4ScenarioToolbarGroup() {
    return htmlToElement(getScale4ScenarioToolbarTemplate());
}
