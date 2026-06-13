/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/thomson.js
 * @purpose Live readout for the Thomson scattering observatory scenario.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { cardStyle, titleStyle, tagBadge, formatExp } from '../_card-helpers.js';

const SCENARIO_ID = 's0-field-thomson-scattering';

const TEMPLATE = `
    <section data-section="thomson" ref="root" style="${cardStyle(180)};display:none;">
        <div style="${titleStyle()}">Thomson observatory</div>
        <div ref="body"></div>
    </section>
`;

function row(label, value, tag = 'M') {
    return `
        <div style="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;align-items:baseline;margin:3px 0;">
            <span style="color:var(--text-muted);">${tagBadge(tag)}${label}</span>
            <span style="font-family:var(--font-mono);font-variant-numeric:tabular-nums;color:var(--accent);">${value}</span>
        </div>
    `;
}

export class ThomsonComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
    }

    update(bridge, scenarioId) {
        if (scenarioId !== SCENARIO_ID) {
            this.refs.root.style.display = 'none';
            return;
        }
        this.refs.root.style.display = '';
        const m = bridge.getThomsonScatteringMetrics?.();
        if (!m || !m.active) {
            this.refs.body.innerHTML = `
                <div style="color:var(--text-muted);font-style:italic;">
                    ${tagBadge('~M')} waiting for field buffers
                </div>
            `;
            return;
        }
        const e = m.energy || {};
        const c = m.center || {};
        const p = m.poynting || {};
        const electron = m.electron
            ? `id ${m.electron.id}, locked=${m.electron.locked ? 'yes' : 'no'}`
            : 'not found';
        this.refs.body.innerHTML = `
            ${row('tick / lattice', `${m.tick} / L=${m.latticeSize}`)}
            ${row('electron site', electron)}
            ${row('|wv_y| at charge', formatExp(Math.abs(c.waveVelY)))}
            ${row('near energy r=3', formatExp(e.centerR3))}
            ${row('lateral energy y+10', formatExp(e.lateralYPlus10R3))}
            ${row('forward energy x+10', formatExp(e.forwardXPlus10R3))}
            ${row('Poynting Px', formatExp(p.x ?? 0))}
            <div style="margin-top:8px;color:var(--text-muted);font-size:12px;line-height:1.35;">
                ${tagBadge('~M')}This shows wave-field response around a locked charge.
                The native precision campaign separates beam-only, electron-only,
                and combined runs to test whether the scene is true interaction
                or linear superposition.
            </div>
        `;
    }
}
