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

const FTD0289_CANONICAL = {
    l2: 4.2546065759857619e-4,
    relL2: 5.4899329705502643e-5,
    maxAbs: 6.3648580289611865e-5,
    localEnergy: 3.2674008350733898e-8,
};

const TEMPLATE = `
    <section data-section="thomson" ref="root" style="${cardStyle(260)};display:none;">
        <div ref="title" style="${titleStyle()}">Flux recoil observatory</div>
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
        this.bridgeRef = null;
        this.element.addEventListener('change', (e) => {
            const input = e.target.closest('[data-thomson-mode]');
            if (input) {
                this._selectScenario(input.checked
                    ? 's0-field-thomson-unlocked-recoil'
                    : 's0-field-thomson-scattering');
                return;
            }
            const fluxInput = e.target.closest('[data-thomson-flux-unlocked]');
            if (fluxInput) this._setFluxUnlocked(fluxInput.checked);
        });
    }

    update(bridge, scenarioId) {
        if (!SCENARIO_IDS.has(scenarioId)) {
            this.refs.root.style.display = 'none';
            return;
        }
        this.refs.root.style.display = '';
        this.bridgeRef = bridge;
        const unlocked = scenarioId === 's0-field-thomson-unlocked-recoil';
        this.refs.title.textContent = unlocked ? 'Flux recoil unlocked' : 'Flux recoil locked';
        const m = bridge.getThomsonScatteringMetrics?.();
        const fluxUnlocked = m?.toggles?.wave_propagation ?? bridge.getToggle?.('wave_propagation') ?? true;
        if (!m || !m.active) {
            this.refs.body.innerHTML = `
                ${this._modeControl(unlocked, fluxUnlocked)}
                <div style="color:var(--text-muted);font-style:italic;">
                    ${tagBadge('~M')} waiting for field buffers
                </div>
            `;
            return;
        }
        const e = m.energy || {};
        const c = m.center || {};
        const p = m.poynting || {};
        const fc = m.fluxCentroid || {};
        const fd = fc.delta || {};
        const fv = fc.velocity || {};
        const xr = m.excessResidual || {};
        const xc = xr.localCentroid || {};
        const electron = m.electron
            ? `id ${m.electron.id}, locked=${m.electron.locked ? 'yes' : 'no'}`
            : 'not found';
        const carrierRows = m.electron ? `
            ${row('carrier displacement', `${formatExp(m.electron.dx)}, ${formatExp(m.electron.dy)}, ${formatExp(m.electron.dz)}`, '~M')}
            ${row('carrier |v|', formatExp(m.electron.speed ?? 0), '~M')}
            ${row('|F_emergent|', formatExp(m.electron.fieldForceMag ?? 0))}
        ` : '';
        this.refs.body.innerHTML = `
            ${this._modeControl(unlocked, fluxUnlocked)}
            ${row('tick / lattice', `${m.tick} / L=${m.latticeSize}`)}
            ${row('charge marker', electron, '~M')}
            ${row('flux field', fluxUnlocked ? 'unlocked' : 'locked', fluxUnlocked ? 'E' : '~M')}
            ${row('|wv_y| at charge', formatExp(Math.abs(c.waveVelY)))}
            ${row('flux centroid Δ', `${formatExp(fd.x ?? 0)}, ${formatExp(fd.y ?? 0)}, ${formatExp(fd.z ?? 0)}`, 'E')}
            ${row('|flux Δ|', formatExp(fd.mag ?? 0), 'E')}
            ${row('flux centroid v', `${formatExp(fv.x ?? 0)}, ${formatExp(fv.y ?? 0)}, ${formatExp(fv.z ?? 0)}`, 'E')}
            ${row('|flux v|', formatExp(fv.mag ?? 0), 'E')}
            ${carrierRows}
            ${row('near energy r=3', formatExp(e.centerR3))}
            ${row('lateral energy y+10', formatExp(e.lateralYPlus10R3))}
            ${row('forward energy x+10', formatExp(e.forwardXPlus10R3))}
            ${row('Poynting Px', formatExp(p.x ?? 0))}
            ${row('Poynting |P|', formatExp(p.mag ?? 0))}
            ${row('live excess |R|', formatExp(xr.l2 ?? 0), 'E')}
            ${row('live excess rel', formatExp(xr.relL2 ?? 0), 'E')}
            ${row('live excess max', formatExp(xr.maxAbs ?? 0), 'E')}
            ${row('live excess local E', formatExp(xr.localEnergy ?? 0), 'E')}
            ${row('live excess centroid', `${formatExp(xc.x ?? 0)}, ${formatExp(xc.y ?? 0)}, ${formatExp(xc.z ?? 0)}`, 'E')}
            ${unlocked ? `
                ${row('FTD-0289 C++ |R|', formatExp(FTD0289_CANONICAL.l2), 'M')}
                ${row('FTD-0289 C++ rel', formatExp(FTD0289_CANONICAL.relL2), 'M')}
                ${row('FTD-0289 C++ max', formatExp(FTD0289_CANONICAL.maxAbs), 'M')}
                ${row('FTD-0289 local E', formatExp(FTD0289_CANONICAL.localEnergy), 'M')}
            ` : ''}
            <div style="margin-top:8px;color:var(--text-muted);font-size:12px;line-height:1.35;">
                ${tagBadge(unlocked ? 'M' : '~M')}${unlocked
                    ? 'Live JS residual uses plus minus beam minus charge inside the visual bridge. Exact C++ FTD-0289 run above is canonical; no alpha or cross-section claim.'
                    : 'Locked field observatory: FTD-0287 measured linear superposition with no unlocked flux-gradient recoil.'}
            </div>
        `;
    }

    _modeControl(unlocked, fluxUnlocked) {
        return `
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:6px 10px;margin:0 0 8px;">
                <label style="display:flex;align-items:center;gap:8px;color:var(--text-primary);">
                    <input data-thomson-flux-unlocked type="checkbox" ${fluxUnlocked ? 'checked' : ''} style="accent-color:var(--accent);">
                    <span>Flux field unlocked</span>
                </label>
                <label style="display:flex;align-items:center;gap:8px;color:var(--text-muted);">
                    <input data-thomson-mode type="checkbox" ${unlocked ? 'checked' : ''} style="accent-color:var(--accent);">
                    <span>Recoil branch unlocked</span>
                </label>
            </div>
        `;
    }

    _selectScenario(id) {
        const select = document.getElementById('scenario-select');
        if (!select || select.value === id) return;
        select.value = id;
        select.dispatchEvent(new Event('change', { bubbles: true }));
    }

    _setFluxUnlocked(checked) {
        this.bridgeRef?.setToggle?.('wave_propagation', !!checked);
        const globalToggle = document.getElementById('t-wave');
        if (globalToggle) {
            globalToggle.checked = !!checked;
            globalToggle.closest('.toggle-row')?.classList.remove('scenario-override');
        }
    }
}
