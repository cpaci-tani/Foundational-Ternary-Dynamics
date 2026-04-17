import { getScale1ScenarioToolbarTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale1ScenarioToolbarGroup() {
    return htmlToElement(getScale1ScenarioToolbarTemplate());
}
