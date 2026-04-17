import { getScale11ScenarioToolbarTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale11ScenarioToolbarGroup() {
    return htmlToElement(getScale11ScenarioToolbarTemplate());
}
