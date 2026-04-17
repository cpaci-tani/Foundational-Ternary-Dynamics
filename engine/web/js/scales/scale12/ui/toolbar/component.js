import { getScale12MetaToolbarTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale12MetaToolbarGroup() {
    return htmlToElement(getScale12MetaToolbarTemplate());
}
