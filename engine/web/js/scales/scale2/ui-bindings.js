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
 *     Restore every AE toggle checkbox to the default value encoded in
 *     SCALE2_TOGGLES, and push the default into the bridge.
 *
 *   aeSetPhase3(bridge, flags)
 *     Enable a subset of Phase-3 forces { hbonds, angle, dipole,
 *     thermostat, elec, temp } and sync the matching UI checkboxes.
 *     Used by scenarios to turn on only the forces they demonstrate.
 *
 *   bindScale2ControlsUI()
 *     Mount the Scale 2 control card into the controls panel (called
 *     once during app startup after the DOM is ready).
 *
 *   AE_DEFAULT_TOGGLES
 *     Re-export of SCALE2_TOGGLES for callers that want the raw list.
 */

import { SCALE2_TOGGLES } from '../../config/toggles.js';
import { Scale2ControlsComponent } from './ui/controls/component.js';


export const AE_DEFAULT_TOGGLES = SCALE2_TOGGLES;


import { syncAEParamsFromUI, resetAETogglesToDefaults } from '../scale-utils.js';

export { syncAEParamsFromUI, resetAETogglesToDefaults };



/**
 * Enable Phase 3 forces for specific scenarios and sync UI checkboxes.
 * flags: { hbonds, angle, dipole, thermostat, elec, temp }
 */
export function aeSetPhase3(bridge, flags) {
    const map = {
        hbonds:     ['ae-hbonds',             'aeSetHBonds'],
        angle:      ['ae-angle',              'aeSetAngleStrain'],
        dipole:     ['ae-dipole',             'aeSetDipoleDipole'],
        thermostat: ['ae-thermostat',         'aeSetThermostat'],
        elec:       ['ae-electronegativity',  'aeSetElectronegativity'],
    };
    for (const [key, [elId, setter]] of Object.entries(map)) {
        if (flags[key] !== undefined && bridge[setter]) {
            bridge[setter](flags[key]);
            const el = document.getElementById(elId);
            if (el) el.checked = flags[key];
        }
    }
    if (flags.temp !== undefined && bridge.aeSetThermostatTemp) {
        bridge.aeSetThermostatTemp(flags.temp);
    }
}


/**
 * Mount the Scale 2 control card into the controls panel.
 * Called once during app startup after the DOM is ready.
 */
export function bindScale2ControlsUI() {
    const controlsPanel = document.getElementById('panel-controls');
    if (controlsPanel) new Scale2ControlsComponent(controlsPanel).init();
}
