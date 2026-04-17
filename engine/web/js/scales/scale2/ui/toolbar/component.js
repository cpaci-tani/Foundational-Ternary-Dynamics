import { getScale2ScenarioToolbarTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale2ScenarioToolbarGroup() {
    return htmlToElement(getScale2ScenarioToolbarTemplate());
}
