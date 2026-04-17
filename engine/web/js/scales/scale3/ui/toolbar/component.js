import { getScale3ScenarioToolbarTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale3ScenarioToolbarGroup() {
    return htmlToElement(getScale3ScenarioToolbarTemplate());
}
