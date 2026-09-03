/**
 * Scale 2 — AE UI Bindings
 * ────────────────────────────────────────────────────────────────────
 *
 * Houses the DOM-coupled helpers that sync AE physics parameters and
 * toggles between the Scale 2 control card checkboxes/sliders and the
 * AtomEngine bridge. Extracted from scales/scale2/controller.js (S2-2).
 *
 * EXPORTS:
 *   syncAEParamsFromUI(bridge)
 *     Read dt + softening sliders and all AE_DEFAULT_TOGGLES checkboxes,
 *     push their current values into the bridge. Called after initAE()
 *     and after resetAETogglesToDefaults() to establish clean state.
 *
 *   resetAETogglesToDefaults(bridge)
 *     Restore every AE toggle checkbox to the canonical registry default,
 *     and push the default into the bridge.
 *
 *   bindScale2ControlsUI()
 *     Mount the Scale 2 control card into the controls panel (called
 *     once during app startup after the DOM is ready).
 *
 *   AE_DEFAULT_TOGGLES
 *     Registry-derived tuples for callers that want the raw bindings.
 */

import { AE_PHYSICS_SPECS } from './scenario-registry.js';
import { Scale2ControlsComponent } from './ui/controls/component.js';


export const AE_DEFAULT_TOGGLES = Object.freeze(AE_PHYSICS_SPECS.map((spec) =>
    Object.freeze([spec.elementId, spec.defaultValue, spec.setter])));


import { syncAEParamsFromUI, resetAETogglesToDefaults } from '../scale-utils.js';

export { syncAEParamsFromUI, resetAETogglesToDefaults };



/**
 * Mount the Scale 2 control card into the controls panel.
 * Called once during app startup after the DOM is ready.
 */
export function bindScale2ControlsUI() {
    const controlsPanel = document.getElementById('panel-controls');
    if (controlsPanel) new Scale2ControlsComponent(controlsPanel).init();
}

/** Keep the editable nuclear laboratory card aligned with live engine state. */
export function syncAENuclearControlsFromBridge(bridge) {
    const diag = bridge?.aeGetNuclearDiagnostics?.();
    const setValue = (id, value) => {
        const element = document.getElementById(id);
        if (element) element.value = String(value);
    };
    const setText = (id, value) => {
        const element = document.getElementById(id);
        if (element) element.textContent = value;
    };
    setValue('ae-nuclear-channel', diag?.channel || '');
    setValue('ae-nuclear-reactivity', diag?.reactivityScale ?? 1);
    setValue('ae-nuclear-collision-radius', diag?.collisionRadiusScale ?? 1);
    setValue('ae-nuclear-transport-radius', diag?.transportRadius ?? 18);
    setValue('ae-nuclear-boundary', diag?.boundaryMode || 'leak');
    setValue('ae-nuclear-moderator', diag?.moderatorStrength ?? 0);
    setValue('ae-nuclear-absorber', diag?.absorberStrength ?? 0);
    setValue('ae-nuclear-source-rate', diag?.sourceRate ?? 0);
    setValue('ae-nuclear-source-energy', diag?.sourceEnergyMeV ?? 2.53e-8);
    const enabled = document.getElementById('ae-nuclear-source-enabled');
    if (enabled) enabled.checked = !!diag?.sourceEnabled;
    setText('ae-nuclear-reactivity-value', (diag?.reactivityScale ?? 1).toFixed(1));
    setText('ae-nuclear-collision-radius-value', `${(diag?.collisionRadiusScale ?? 1).toFixed(2)}×`);
    setText('ae-nuclear-transport-radius-value', `${(diag?.transportRadius ?? 18).toFixed(0)} lu`);
    setText('ae-nuclear-moderator-value', (diag?.moderatorStrength ?? 0).toFixed(2));
    setText('ae-nuclear-absorber-value', (diag?.absorberStrength ?? 0).toFixed(2));
    setText('ae-nuclear-source-rate-value', `${(diag?.sourceRate ?? 0).toFixed(2)}/tick`);
}
