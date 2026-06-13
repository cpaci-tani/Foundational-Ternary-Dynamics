/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/thomson.js
 * @purpose Live readout for the Thomson scattering observatory scenario.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { cardStyle, titleStyle, tagBadge, formatExp } from '../_card-helpers.js';

const SCENARIO_IDS = new Set([
    's0-field-thomson-scattering',
    's0-field-thomson-unlocked-recoil',
]);

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
        if (!SCENARIO_IDS.has(scenarioId)) {
            this.refs.root.style.display = 'none';
            return;
        }
        this.refs.root.style.display = '';
        const unlocked = scenarioId === 's0-field-thomson-unlocked-recoil';
        this.refs.root.querySelector('div').textContent = unlocked ? 'Thomson unlocked recoil' : 'Thomson observatory';
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
        const motionRows = unlocked && m.electron ? `
            ${row('displacement', `${formatExp(m.electron.dx)}, ${formatExp(m.electron.dy)}, ${formatExp(m.electron.dz)}`)}
            ${row('velocity', `${formatExp(m.electron.vx)}, ${formatExp(m.electron.vy)}, ${formatExp(m.electron.vz)}`)}
            ${row('|v|', formatExp(m.electron.speed ?? 0))}
            ${row('|F_emergent|', formatExp(m.electron.fieldForceMag ?? 0))}
        ` : '';
        this.refs.body.innerHTML = `
            ${row('tick / lattice', `${m.tick} / L=${m.latticeSize}`)}
            ${row('electron site', electron)}
            ${row('|wv_y| at charge', formatExp(Math.abs(c.waveVelY)))}
            ${motionRows}
            ${row('near energy r=3', formatExp(e.centerR3))}
            ${row('lateral energy y+10', formatExp(e.lateralYPlus10R3))}
            ${row('forward energy x+10', formatExp(e.forwardXPlus10R3))}
            ${row('Poynting Px', formatExp(p.x ?? 0))}
            <div style="margin-top:8px;color:var(--text-muted);font-size:12px;line-height:1.35;">
                ${tagBadge(unlocked ? 'M' : '~M')}${unlocked
                    ? 'FTD-0288 channel: native emergent flux-gradient recoil. Not a Thomson cross-section or alpha derivation.'
                    : 'Locked field observatory: FTD-0287 measured linear superposition, not mechanical recoil.'}
            </div>
        `;
    }
}
