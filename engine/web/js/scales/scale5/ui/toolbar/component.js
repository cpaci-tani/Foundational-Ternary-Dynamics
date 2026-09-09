import { getScale5ScenarioToolbarTemplate, getScale5TelemetryToolbarTemplate } from './template.js';
import { SCALE5_TOGGLES } from '../../../../config/toggles.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

// The scenario toolbar group is built ONCE at app start (TopbarComponent
// mounts every registered toolbar factory a single time; visibility across
// scales is CSS-driven, not create/destroy). The CosmicMockBridge instance
// it must talk to, however, is recreated on every scenario load (Scale5
// controller.js `loadCosmicScenario`). `_activeBridge` bridges that gap: the
// checkbox 'change' listener below is bound once but always reads the
// latest bridge via this module-level reference, kept current by
// syncScale5Toggles() (called by the controller right after it builds a
// fresh bridge).
let _activeBridge = null;

export function createScale5ScenarioToolbarGroup() {
    const el = htmlToElement(getScale5ScenarioToolbarTemplate());
    _bindToggleCheckboxes(el);
    return el;
}

export function createScale5TelemetryToolbarGroup() {
    return htmlToElement(getScale5TelemetryToolbarTemplate());
}

function _bindToggleCheckboxes(root) {
    for (const [key, , domId] of SCALE5_TOGGLES) {
        const input = root.querySelector('#' + domId);
        if (!input) continue;
        input.addEventListener('change', () => {
            _activeBridge?.setToggle?.(key, input.checked);
        });
    }
}

/**
 * Reflect `bridge`'s current toggle state onto the Scale 5 toolbar
 * checkboxes and make it the target of their 'change' listeners. Call this
 * once a scenario load has produced a bridge (Scale5 controller.js,
 * loadCosmicScenario) so a freshly-loaded scenario's toggle state (e.g. a
 * gas laboratory that sets `this._toggles.sph_monaghan = true` in its
 * setup) is visible on the checkbox, and so the checkbox drives THAT
 * bridge rather than a stale/destroyed one.
 *
 * @param {{getToggle?: Function}|null} bridge
 */
export function syncScale5Toggles(bridge) {
    _activeBridge = bridge || null;
    if (!bridge?.getToggle) return;
    for (const [key, , domId] of SCALE5_TOGGLES) {
        const input = document.getElementById(domId);
        if (!input) continue;
        input.checked = !!bridge.getToggle(key);
    }
}
