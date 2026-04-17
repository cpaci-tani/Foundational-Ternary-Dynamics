import { getScale4ScenarioToolbarTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale4ScenarioToolbarGroup() {
    return htmlToElement(getScale4ScenarioToolbarTemplate());
}
