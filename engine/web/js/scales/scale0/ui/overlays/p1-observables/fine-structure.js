/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/fine-structure.js
 * @purpose Fine-structure constant instrument panel for flux-recoil scenarios.
 */

import { BaseComponent } from '../../../../../core/component.js';
import {
    ALPHA,
    ALPHA_TREE,
    COULOMB_K_FORCE,
    DAMPING,
    G_C,
    X_PLUS,
    X_PLUS_PRECISION,
} from '../../../../../constants.js';
import { cardStyle, titleStyle, tagBadge, formatExp } from '../_card-helpers.js';

const SCENARIO_IDS = new Set([
    's0-field-thomson-scattering',
    's0-field-thomson-unlocked-recoil',
]);

const FTD0289_CANONICAL = {
    relL2: 5.4899329705502643e-5,
    maxAbs: 6.3648580289611865e-5,
};

const TEMPLATE = `
    <section data-section="fine-structure" ref="root" style="${cardStyle(300)};display:none;">
        <div style="${titleStyle()}">Fine structure instrument</div>
        <div ref="body"></div>
    </section>
`;

function row(label, value, tag = 'T') {
    return `
        <div style="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;align-items:baseline;margin:3px 0;">
            <span style="color:var(--text-muted);">${tagBadge(tag)}${label}</span>
            <span style="font-family:var(--font-mono);font-variant-numeric:tabular-nums;color:var(--accent);">${value}</span>
        </div>
    `;
}

function setting(key, label, checked) {
    return `
        <label style="display:flex;align-items:center;gap:7px;color:var(--text-muted);min-width:0;">
            <input data-alpha-toggle="${key}" type="checkbox" ${checked ? 'checked' : ''} style="accent-color:var(--accent);">
            <span>${label}</span>
        </label>
    `;
}

function readToggle(bridge, toggles, key, fallback = false) {
    if (toggles && key in toggles) return !!toggles[key];
    if (typeof bridge?.getToggle === 'function') return !!bridge.getToggle(key);
    return fallback;
}

export class FineStructureComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
        this.bridgeRef = null;
        this.element.addEventListener('change', (e) => {
            const input = e.target.closest('[data-alpha-toggle]');
            if (!input || !this.bridgeRef?.setToggle) return;
            const key = input.dataset.alphaToggle;
            this.bridgeRef.setToggle(key, input.checked);
            this._syncGlobalToggleCheckbox(key, input.checked);
        });
    }

    update(bridge, scenarioId) {
        if (!SCENARIO_IDS.has(scenarioId)) {
            this.refs.root.style.display = 'none';
            return;
        }
        this.refs.root.style.display = '';
        this.bridgeRef = bridge;

        const m = bridge.getThomsonScatteringMetrics?.() || null;
        const constants = bridge.getConstants?.() || {};
        const alpha = constants.ALPHA ?? ALPHA;
        const gC = constants.G_C ?? G_C;
        const forceK = COULOMB_K_FORCE;
        const damping = constants.DAMPING ?? DAMPING;
        const t = m?.toggles || {};
        const fc = m?.fluxCentroid || {};
        const fd = fc.delta || {};
        const p = m?.poynting || {};
        const e = m?.energy || {};
        const xr = m?.excessResidual || {};
        const xc = xr.localCentroid || {};
        const pyOverPx = Math.abs(p.x ?? 0) > 1e-15 ? (p.y ?? 0) / p.x : 0;

        this.refs.body.innerHTML = `
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:6px 10px;margin-bottom:8px;">
                ${setting('wave_propagation', 'Flux field unlocked', readToggle(bridge, t, 'wave_propagation', true))}
                ${setting('coupling', 'Coupling', readToggle(bridge, t, 'coupling', true))}
                ${setting('emergent_forces', 'Flux-gradient force', readToggle(bridge, t, 'emergent_forces', false))}
                ${setting('movement', 'Marker movement', readToggle(bridge, t, 'movement', false))}
                ${setting('poisson_coulomb', 'Poisson Coulomb', readToggle(bridge, t, 'poisson_coulomb', false))}
            </div>
            ${row('α = G_C²', alpha.toExponential(12))}
            ${row('1/α', (1 / alpha).toFixed(9))}
            ${row('tree 1/x+', ALPHA_TREE.toExponential(12))}
            ${row('x+ / x+ precision', `${X_PLUS.toFixed(6)} / ${X_PLUS_PRECISION.toFixed(6)}`)}
            ${row('G_C', gC.toExponential(12))}
            ${row('α/(4π) force K', forceK.toExponential(12))}
            ${row('damping γ', damping.toExponential(12))}
            ${row('|flux Δ|', formatExp(fd.mag ?? 0), 'E')}
            ${row('flux Δy', formatExp(fd.y ?? 0), 'E')}
            ${row('|P|', formatExp(p.mag ?? 0), 'M')}
            ${row('P_y / P_x', formatExp(pyOverPx), 'M')}
            ${row('field / wave E', `${formatExp(e.field ?? 0)} / ${formatExp(e.wave ?? 0)}`, 'M')}
            ${row('live residual |R|', formatExp(xr.l2 ?? 0), 'E')}
            ${row('live residual rel', formatExp(xr.relL2 ?? 0), 'E')}
            ${row('live comp x/y/z', `${formatExp(xr.compX ?? 0)} / ${formatExp(xr.compY ?? 0)} / ${formatExp(xr.compZ ?? 0)}`, 'E')}
            ${row('live local centroid', `${formatExp(xc.x ?? 0)}, ${formatExp(xc.y ?? 0)}, ${formatExp(xc.z ?? 0)}`, 'E')}
            ${row('FTD-0289 C++ rel R', formatExp(FTD0289_CANONICAL.relL2), 'M')}
            ${row('FTD-0289 C++ max R', formatExp(FTD0289_CANONICAL.maxAbs), 'M')}
            <div style="margin-top:8px;color:var(--text-muted);font-size:12px;line-height:1.35;">
                ${tagBadge('T')}α is the dashboard's configured coupling constant. ${tagBadge('E')}Live residual is visual-bridge telemetry. ${tagBadge('M')}FTD-0289 is the C++ run of record, not an α derivation.
            </div>
        `;
    }

    _syncGlobalToggleCheckbox(key, checked) {
        const domIds = {
            wave_propagation: 't-wave',
            coupling: 't-coupling',
            movement: 't-movement',
            poisson_coulomb: 't-poisson',
        };
        const el = document.getElementById(domIds[key]);
        if (el) {
            el.checked = !!checked;
            el.closest('.toggle-row')?.classList.remove('scenario-override');
        }
    }
}
