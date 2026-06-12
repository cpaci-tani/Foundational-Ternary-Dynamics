/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/gravity.js
 * @purpose Gravitational time dilation component.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { cardStyle, titleStyle } from '../_card-helpers.js';

const GRAVITY_SCENARIOS = new Set(['s0-seed-schwarzschild', 's0-seed-gravitational-wave']);
const TWO_PI = 2.0 * Math.PI;

const TEMPLATE = `
    <section data-section="gravity" style="${cardStyle(140)}">
        <div style="${titleStyle()}">Gravitational time dilation</div>
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
        if (GRAVITY_SCENARIOS.has(scenarioId)) {
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
        if (!latSample || !latSample.values || !latSample.positions || latSample.count === 0) return null;

        const L = bridge?.latticeSize || 32;
        const mid = L / 2;
        let bestCenter = { d2: Infinity, idx: 0 };
        let bestCorner = { d2: Infinity, idx: 0 };
        const cornerX = 1, cornerY = 1, cornerZ = 1;
        for (let i = 0; i < latSample.count; i++) {
            const x = latSample.positions[i * 3];
            const y = latSample.positions[i * 3 + 1];
            const z = latSample.positions[i * 3 + 2];
            const dC2 = (x - mid) ** 2 + (y - mid) ** 2 + (z - mid) ** 2;
            if (dC2 < bestCenter.d2) { bestCenter.d2 = dC2; bestCenter.idx = i; }
            const dE2 = (x - cornerX) ** 2 + (y - cornerY) ** 2 + (z - cornerZ) ** 2;
            if (dE2 < bestCorner.d2) { bestCorner.d2 = dE2; bestCorner.idx = i; }
        }
        const latCenter = latSample.values[bestCenter.idx];
        const latCorner = latSample.values[bestCorner.idx];
        const tauCenter = Math.sqrt(Math.max(0, 1.0 - latCenter));
        const tauCorner = Math.sqrt(Math.max(1e-6, 1.0 - latCorner));
        const ratio = tauCenter / tauCorner;
        return { latCenter, latCorner, tauCenter, tauCorner, ratio, latticeSize: L };
    }

    _renderGravitySection(container, probe, tickPhase) {
        if (!probe) {
            container.innerHTML = `<div class="p1-empty-state">Latency proxy unavailable on this bridge.</div>`;
            return;
        }
        container.className = '';
        const { latCenter, latCorner, tauCenter, tauCorner, ratio } = probe;
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
                τ<sub>well</sub> / τ<sub>far</sub> = <span class="p1-gravity-ratio">${ratio.toExponential(3)}</span>
                <span class="p1-bell-desc">  (clock at well runs ${(ratio < 1 ? `${(1 / ratio).toFixed(2)}× slower` : 'as fast')} than far clock)</span>
            </div>
            <div class="p1-gravity-footer">
                Lattice latency proxy L(x) ∈ [0,1] modifies effective tick rate. Proper-time rate τ′ ≈ √(1−L), analogous to GR's √(1 − 2GM/(rc²)). L=32³ lattice. test_einstein_equations.cpp validates time dilation to 0.004% match against GR after the latency-fix patch (April 13).
            </div>
        `;
    }
}
