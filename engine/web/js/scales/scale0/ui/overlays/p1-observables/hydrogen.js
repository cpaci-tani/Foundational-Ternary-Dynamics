/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/hydrogen.js
 * @purpose Hydrogen Spectrum component.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { renderEnergyLevels, hydrogenEnergyLevel, ionizationEnergy } from '../../../../../spectroscopy.js';
import { RYDBERG_EV_CODATA } from '../../../../../constants.js';
import { cardStyle, titleStyle } from '../_card-helpers.js';

const HYDROGEN_SCENARIOS = new Set(['s0-seed-hydrogen', 's0-seed-helium']);

const TEMPLATE = `
    <section data-section="hydrogen" style="${cardStyle(120)}">
        <div style="${titleStyle()}">Hydrogen Spectrum</div>
        <div ref="body" class="p1-empty-state">
            Load <code>s0-seed-hydrogen</code> to see the predicted level diagram.
        </div>
    </section>
`;

export class HydrogenComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
        this.renderedFor = null;
    }

    update(bridge, scenarioId) {
        if (HYDROGEN_SCENARIOS.has(scenarioId)) {
            if (this.renderedFor !== scenarioId) {
                this.refs.body.innerHTML = '';
                this.refs.body.className = '';
                renderEnergyLevels(1, this.refs.body);
                const E1eV = hydrogenEnergyLevel(1, 1) * 1e6;
                const ionEV = ionizationEnergy(1) * 1e6;
                const relErr = Math.abs(ionEV - RYDBERG_EV_CODATA) / RYDBERG_EV_CODATA * 100;
                this.refs.body.insertAdjacentHTML(
                    'beforeend',
                    `<div class="p1-hydrogen-footer">
                        E₁ = <span class="p1-hydrogen-val">${E1eV.toFixed(3)} eV</span>
                        &nbsp;|&nbsp; ionization = <span class="p1-hydrogen-val">${ionEV.toFixed(3)} eV</span>
                        (CODATA: ${RYDBERG_EV_CODATA.toFixed(3)} eV; rel err ${relErr.toFixed(3)}%).
                        <br><span class="p1-hydrogen-desc">All levels follow E_n = -m_e·Z²·α²/(2n²) using FTD's α and m_e from the ontic chain. Lyman/Balmer/Paschen transitions shown.</span>
                    </div>`
                );
                this.renderedFor = scenarioId;
            }
        } else if (this.renderedFor !== null) {
            this.refs.body.className = 'p1-empty-state';
            this.refs.body.innerHTML = 'Load <code>s0-seed-hydrogen</code> or <code>s0-seed-helium</code> to see the predicted level diagram.';
            this.renderedFor = null;
        }
    }
}
