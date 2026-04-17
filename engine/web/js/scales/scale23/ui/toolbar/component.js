import { getScale23ForceToolbarTemplate, getScale23VisualToolbarTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale23VisualToolbarGroup() {
    return htmlToElement(getScale23VisualToolbarTemplate());
}

export function createScale23ForceToolbarGroup() {
    return htmlToElement(getScale23ForceToolbarTemplate());
}
