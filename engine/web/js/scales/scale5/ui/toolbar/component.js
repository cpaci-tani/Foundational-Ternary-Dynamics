import { getScale5ScenarioToolbarTemplate, getScale5TelemetryToolbarTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale5ScenarioToolbarGroup() {
    return htmlToElement(getScale5ScenarioToolbarTemplate());
}

export function createScale5TelemetryToolbarGroup() {
    return htmlToElement(getScale5TelemetryToolbarTemplate());
}
