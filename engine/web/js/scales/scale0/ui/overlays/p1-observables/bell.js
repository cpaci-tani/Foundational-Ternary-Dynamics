/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/bell.js
 * @purpose Bell CHSH component.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { TSIRELSON_BOUND } from '../../../../../constants.js';
import { cardStyle, titleStyle } from '../_card-helpers.js';

const BELL_SCENARIOS = new Set(['quantum-entangle']);
const PANEL_ID = 'p1-observables-panel';

const TEMPLATE = `
    <section data-section="bell" style="${cardStyle(170)}">
        <div style="${titleStyle()}">Bell CHSH</div>
        <div ref="body" class="p1-empty-state">
            Load <code>quantum-entangle</code> to interact with the CHSH correlator.
        </div>
    </section>
`;

export class BellComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
        this.renderedFor = null;
        this.bellAngles = { a: 0, ap: 0.5, b: 0.25, bp: 0.75 };
    }

    update(bridge, scenarioId) {
        if (BELL_SCENARIOS.has(scenarioId)) {
            if (this.renderedFor !== scenarioId) {
                this.refs.body.className = '';
                this._renderBellSection();
                this.renderedFor = scenarioId;
            }
        } else if (this.renderedFor !== null) {
            this.refs.body.className = 'p1-empty-state';
            this.refs.body.innerHTML = 'Load <code>quantum-entangle</code> to interact with the CHSH correlator.';
            this.renderedFor = null;
        }
    }

    _bellCorrelation(theta) {
        return Math.cos(theta);
    }

    _computeCHSH(angles) {
        const a  = angles.a  * Math.PI;
        const ap = angles.ap * Math.PI;
        const b  = angles.b  * Math.PI;
        const bp = angles.bp * Math.PI;
        const Eab   = this._bellCorrelation(a - b);
        const Eabp  = this._bellCorrelation(a - bp);
        const Eapb  = this._bellCorrelation(ap - b);
        const Eapbp = this._bellCorrelation(ap - bp);
        const S = Eab - Eabp + Eapb + Eapbp;
        return { a, ap, b, bp, Eab, Eabp, Eapb, Eapbp, S };
    }

    _renderBellSection() {
        const chsh = this._computeCHSH(this.bellAngles);
        const sAbs = Math.abs(chsh.S);
        let sColor = '#888';
        let sLabel = '';
        if (sAbs > 2.0 + 1e-9) {
            sColor = '#6fc';
            sLabel = `quantum (|S|>2 violates classical bound)`;
        } else {
            sColor = '#fc6';
            sLabel = 'classical';
        }
        if (sAbs > TSIRELSON_BOUND - 1e-3) sLabel = 'at Tsirelson 2√2';

        this.refs.body.innerHTML = `
            <div class="p1-bell-grid">
                <label>a/π</label>
                <input id="${PANEL_ID}-bell-a"  type="range" min="-1" max="1" step="0.01" value="${this.bellAngles.a}"  class="p1-bell-slider">
                <span id="${PANEL_ID}-bell-a-val"  class="p1-bell-val">${this.bellAngles.a.toFixed(2)}</span>
                <label>a'/π</label>
                <input id="${PANEL_ID}-bell-ap" type="range" min="-1" max="1" step="0.01" value="${this.bellAngles.ap}" class="p1-bell-slider">
                <span id="${PANEL_ID}-bell-ap-val" class="p1-bell-val">${this.bellAngles.ap.toFixed(2)}</span>
                <label>b/π</label>
                <input id="${PANEL_ID}-bell-b"  type="range" min="-1" max="1" step="0.01" value="${this.bellAngles.b}"  class="p1-bell-slider">
                <span id="${PANEL_ID}-bell-b-val"  class="p1-bell-val">${this.bellAngles.b.toFixed(2)}</span>
                <label>b'/π</label>
                <input id="${PANEL_ID}-bell-bp" type="range" min="-1" max="1" step="0.01" value="${this.bellAngles.bp}" class="p1-bell-slider">
                <span id="${PANEL_ID}-bell-bp-val" class="p1-bell-val">${this.bellAngles.bp.toFixed(2)}</span>
            </div>
            <div class="p1-bell-flex">
                <div class="p1-bell-stats">
                    E(a,b)=${chsh.Eab.toFixed(3)}  E(a,b')=${chsh.Eabp.toFixed(3)}<br>
                    E(a',b)=${chsh.Eapb.toFixed(3)}  E(a',b')=${chsh.Eapbp.toFixed(3)}
                </div>
                <button id="${PANEL_ID}-bell-optimal" type="button" class="p1-btn-optimal">
                    set optimal
                </button>
            </div>
            <div class="p1-bell-result">
                S = <span class="p1-bell-result-val" style="color:${sColor};">${chsh.S.toFixed(4)}</span>
                <span class="p1-bell-desc">(|S| = ${sAbs.toFixed(4)}, ${sLabel}; classical ≤ 2; Tsirelson = 2√2 ≈ 2.8284)</span>
            </div>
            <div class="p1-bell-footer">
                Singlet correlation E(a,b)=cos(a-b) per FTD's QM emergence (DERIV_QM_FROM_LATTICE / DERIV_SINGLET_FROM_VOID_EVENT). Lattice produces this when the entangled flux pair is measured at given angles; the analytic prediction is shown live. test_bell_aggregate.cpp validates S = 2√2 to 1e-12 via the same formula. <b>Lattice-statistical aggregation across many shots is follow-up.</b>
            </div>
        `;

        // Wire sliders
        for (const k of ['a', 'ap', 'b', 'bp']) {
            const slider = this.refs.body.querySelector(`#${PANEL_ID}-bell-${k}`);
            const display = this.refs.body.querySelector(`#${PANEL_ID}-bell-${k}-val`);
            if (slider) {
                slider.addEventListener('input', () => {
                    const v = parseFloat(slider.value);
                    if (display) display.textContent = v.toFixed(2);
                    this.bellAngles[k] = v;
                    this._renderBellSection(); // refresh values
                });
            }
        }
        this.refs.body.querySelector(`#${PANEL_ID}-bell-optimal`)?.addEventListener('click', () => {
            this.bellAngles = { a: 0, ap: 0.5, b: 0.25, bp: 0.75 };
            this._renderBellSection();
        });
    }
}
