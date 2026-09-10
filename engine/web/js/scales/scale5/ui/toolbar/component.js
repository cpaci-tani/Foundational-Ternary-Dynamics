import { getScale5ScenarioToolbarTemplate, getScale5TelemetryToolbarTemplate } from './template.js';
import { bindScale5ToggleCheckboxes } from '../toggle-sync.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

// The scenario toolbar group is built ONCE at app start (TopbarComponent
// mounts every registered toolbar factory a single time; visibility across
// scales is CSS-driven, not create/destroy). Its checkbox(es) are bound via
// the shared multi-surface toggle-sync module (Pass B, Ruling J2) — see
// ui/toggle-sync.js for why this moved out of this file: `sph_monaghan`
// gained a second UI surface (the Gas controls-card checkbox) and Pass D's
// eleven remaining checkboxes need the identical shape.
export function createScale5ScenarioToolbarGroup() {
    const el = htmlToElement(getScale5ScenarioToolbarTemplate());
    bindScale5ToggleCheckboxes(el);
    return el;
}

export function createScale5TelemetryToolbarGroup() {
    return htmlToElement(getScale5TelemetryToolbarTemplate());
}

// Re-exported for back-compat/discoverability — scale5/controller.js now
// imports syncScale5Toggles directly from ui/toggle-sync.js (the canonical
// home, shared with the controls-panel Gas card), but this module is where
// a reader would historically look for it.
export { syncScale5Toggles } from '../toggle-sync.js';
