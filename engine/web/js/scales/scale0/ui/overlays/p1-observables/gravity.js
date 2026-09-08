/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/gravity.js
 * @purpose Gravitational time dilation component.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { cardStyle, titleStyle, tagBadge } from '../_card-helpers.js';
import { getScale0Scenario } from '../../../scenario-registry.js';


const TWO_PI = 2.0 * Math.PI;

const TEMPLATE = `
    <section data-section="gravity" style="${cardStyle(140)}">
        <div style="${titleStyle()}">${tagBadge('T', 'sqrt(1-latency) diagnostic proxy; not an accumulated proper-time measurement')}Latency clock proxy</div>
        <div ref="body" class="p1-empty-state">
            Load <code>s0-seed-schwarzschild</code> to see proper-time ratio.
        </div>
    </section>
`;

export class GravityComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
        this.renderedFor = null;
        this.startTime = performance.now();
    }

    update(bridge, scenarioId, now) {
        const scenario = getScale0Scenario(scenarioId);
        if (scenario?.tags?.includes('gravity') && !scenario?.tags?.includes('time')) {
            const probe = this._probeTimeDilation(bridge);
            const isReduced = document.body.getAttribute('data-reduced-motion') === '1';
            const tickPhase = isReduced ? 0 : (now - this.startTime) * 0.0006;
            this._renderGravitySection(this.refs.body, probe, tickPhase);
            this.renderedFor = scenarioId;
        } else if (this.renderedFor !== null) {
            this.refs.body.className = 'p1-empty-state';
            this.refs.body.innerHTML = 'Load <code>s0-seed-schwarzschild</code> to see proper-time ratio.';
            this.renderedFor = null;
        }
    }

    _probeTimeDilation(bridge) {
        const latSample = bridge?.getLatencySampled?.(2);
        if (!latSample || !Number.isInteger(latSample.count) || latSample.count <= 0
            || latSample.values?.length < latSample.count || latSample.positions?.length < 3*latSample.count
            || !latSample.values || !latSample.positions) return null;

        const L = Number(bridge?.getLatticeSize?.() ?? bridge?.latticeSize);
        if (!Number.isInteger(L) || L < 1) return null;
        const mid = L / 2;
        let bestCenter = { d2: Infinity, idx: 0 };
        let bestCorner = { d2: Infinity, idx: 0 };
        const cornerX = 1, cornerY = 1, cornerZ = 1;
        for (let i = 0; i < latSample.count; i++) {
            const x = latSample.positions[i * 3];
            const y = latSample.positions[i * 3 + 1];
            const z = latSample.positions[i * 3 + 2];
            const latency = latSample.values[i];
            if (![x,y,z,latency].every(Number.isFinite) || latency < 0 || latency > 1) return null;
            const dC2 = (x - mid) ** 2 + (y - mid) ** 2 + (z - mid) ** 2;
            if (dC2 < bestCenter.d2) { bestCenter.d2 = dC2; bestCenter.idx = i; }
            const dE2 = (x - cornerX) ** 2 + (y - cornerY) ** 2 + (z - cornerZ) ** 2;
            if (dE2 < bestCorner.d2) { bestCorner.d2 = dE2; bestCorner.idx = i; }
        }
        const latCenter = latSample.values[bestCenter.idx];
        const latCorner = latSample.values[bestCorner.idx];
        const tauCenter = Math.sqrt(Math.max(0, 1.0 - latCenter));
        const tauCorner = Math.sqrt(1.0 - latCorner);
        const ratio = tauCorner > 0 ? tauCenter / tauCorner : null;
        return { latCenter, latCorner, tauCenter, tauCorner, ratio, latticeSize: L };
    }

    _renderGravitySection(container, probe, tickPhase) {
        if (!probe) {
            container.innerHTML = `<div class="p1-empty-state">Latency proxy unavailable on this bridge.</div>`;
            return;
        }
        container.className = '';
        const { latCenter, latCorner, tauCenter, tauCorner, ratio, latticeSize } = probe;
        const angCorner = (tickPhase * tauCorner) % TWO_PI;
        const angCenter = (tickPhase * tauCenter) % TWO_PI;
        const farX = 18 + 14 * Math.cos(angCorner - Math.PI / 2);
        const farY = 22 + 14 * Math.sin(angCorner - Math.PI / 2);
        const wellX = 18 + 14 * Math.cos(angCenter - Math.PI / 2);
        const wellY = 22 + 14 * Math.sin(angCenter - Math.PI / 2);

        container.innerHTML = `
            <div class="p1-gravity-grid">
                <div class="p1-gravity-col">
                    <svg viewBox="0 0 36 44" class="p1-gravity-clock">
                        <circle cx="18" cy="22" r="16" fill="none" stroke="var(--text-muted,#666)" stroke-width="1"/>
                        <line x1="18" y1="22" x2="${farX.toFixed(2)}" y2="${farY.toFixed(2)}" stroke="#6fc" stroke-width="1.5"/>
                        <circle cx="18" cy="22" r="1.2" fill="#6fc"/>
                    </svg>
                    <div>far (corner)</div>
                    <div style="color:var(--accent);">L=${latCorner.toFixed(3)}<br>τ′=${tauCorner.toFixed(3)}</div>
                </div>
                <div class="p1-gravity-col">
                    <svg viewBox="0 0 36 44" class="p1-gravity-clock">
                        <circle cx="18" cy="22" r="16" fill="none" stroke="var(--text-muted,#666)" stroke-width="1"/>
                        <line x1="18" y1="22" x2="${wellX.toFixed(2)}" y2="${wellY.toFixed(2)}" stroke="#fc6" stroke-width="1.5"/>
                        <circle cx="18" cy="22" r="1.2" fill="#fc6"/>
                    </svg>
                    <div>well (center)</div>
                    <div style="color:#fc6;">L=${latCenter.toFixed(3)}<br>τ′=${tauCenter.toFixed(3)}</div>
                </div>
            </div>
            <div class="p1-gravity-stats">
                proxy τ′<sub>well</sub> / τ′<sub>far</sub> = <span class="p1-gravity-ratio">${ratio === null ? 'unavailable' : ratio.toExponential(3)}</span>
                <span class="p1-bell-desc">${ratio === 0 ? ' (zero center proxy rate)' : ''}</span>
            </div>
            <div class="p1-gravity-footer">
                Selected clock illustration τ′=√(1−latency), sampled on the ${latticeSize}³ reference-engine lattice. The hands use display time; this panel does not measure accumulated proper time or certify a gravitational continuum limit.
            </div>
        `;
    }
}
