/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/coulomb.js
 * @purpose Coulomb V(r) and E-field probe component.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { ALPHA } from '../../../../../constants.js';
import { getParticleCharge, findOppositeChargePairFromList, getPhysicsHarness } from '../../../../../physics/index.js';
import { cardStyle, titleStyle, heroStyle, tagBadge, formatExp } from '../_card-helpers.js';

const PROBE_SAMPLES = 80;
const FOUR_PI = 4.0 * Math.PI;

const TEMPLATE = `
    <section data-section="coulomb" style="${cardStyle(360)}">
        <div style="${titleStyle()}">Coulomb V(r) probe</div>
        <div class="p1-coulomb-meta" ref="meta"></div>
        <div class="p1-coulomb-plot-box" ref="plot"></div>
        <div class="p1-coulomb-hero" ref="hero"></div>
        <div class="p1-coulomb-footer" ref="footer"></div>
    </section>
`;

export class CoulombComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
    }

    update(bridge) {
        const particles = bridge.getScale0ParticleList?.() || [];
        const engineProbe = this._probeCoulombEngineE(bridge, particles);

        let metaLine, heroLine, footerHTML;
        if (engineProbe) {
            const m = engineProbe.meta;
            const modeLabel = m.probeMode === 'wasm-direct-V' ? 'wasm-direct-V (engine-side)'
                            : m.probeMode === 'js-trilinear' ? 'js-trilinear (E-field interp)'
                            : m.probeMode || 'unknown';
            metaLine = `sources: q₁=${m.q1 > 0 ? '+' : ''}${m.q1}, q₂=${m.q2 > 0 ? '+' : ''}${m.q2}, sep=${m.dist}, N=${m.N}, mode=${modeLabel}`;
            heroLine = `
                <div style="${heroStyle()}">
                    ${tagBadge('M')}max |residual| =
                    <span class="p1-coulomb-hero-warning">${formatExp(m.maxAbsResidual)}</span>
                </div>
                <div class="p1-coulomb-hero-sub">
                    ${tagBadge('M')}⟨|residual|⟩ = ${formatExp(m.meanAbsResidual)}
                </div>
            `;
            footerHTML = `
                <span style="opacity:0.7;">Engine probe samples |E| via <code>getEFieldSampled</code> + JS trilinear interp; analytic ref = |α·q₁/(4π·r²) − α·q₂/(4π·(d−r)²)|. Residual amplified to be visible alongside curves.</span>
            `;
        } else {
            metaLine = `Engine field unavailable — falling back to analytic-source probe`;
            heroLine = `
                <div style="font-size:13px;color:var(--text-muted);">
                    ${tagBadge('T')} no engine field samples — chart frame shown for layout stability
                </div>
            `;
            footerHTML = `
                <span style="font-style:italic;">Need: ≥2 opposite-charge particles AND EM toggle on. Try <code>flux-screening</code>, <code>s0-seed-hydrogen</code>, or enable EM in Visualization panel.</span>
            `;
        }

        this.refs.meta.textContent = metaLine;
        this.refs.hero.innerHTML = heroLine;
        this.refs.footer.innerHTML = footerHTML;

        this._renderCoulombEngineProbe(this.refs.plot, engineProbe?.samples ?? null);
    }

    _probeCoulombEngineE(bridge, particles) {
        const pair = findOppositeChargePairFromList(particles);
        if (!pair) return null;
        const { pPos, pNeg } = pair;
        const dx = pNeg.x - pPos.x;
        const dy = pNeg.y - pPos.y;
        const dz = pNeg.z - pPos.z;
        const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
        if (dist < 1.0) return null;

        const q1 = getParticleCharge(pPos, +1);
        const q2 = getParticleCharge(pNeg, -1);

        let engineSamples = null;
        let probeMode = 'analytic-fallback';
        if (typeof bridge.sampleVAtRay === 'function') {
            const rMin = 0.5;
            const rMax = dist * 0.85;
            const tMin = rMin / dist;
            const tMax = rMax / dist;
            const direct = bridge.sampleVAtRay(
                pPos.x + dx * tMin, pPos.y + dy * tMin, pPos.z + dz * tMin,
                pPos.x + dx * tMax, pPos.y + dy * tMax, pPos.z + dz * tMax,
                PROBE_SAMPLES
            );
            if (direct && direct.count > 0 && direct.V && direct.V.length === direct.count) {
                const ds = (rMax - rMin) / Math.max(1, direct.count - 1);
                engineSamples = new Array(direct.count);
                for (let i = 0; i < direct.count; i++) {
                    const im = Math.max(0, i - 1);
                    const ip = Math.min(direct.count - 1, i + 1);
                    const dV = (direct.V[ip] - direct.V[im]) / ((ip - im) * ds || 1e-9);
                    engineSamples[i] = {
                        r: rMin + (rMax - rMin) * (i / (direct.count - 1)),
                        E_mag: Math.abs(dV),
                    };
                }
                probeMode = 'wasm-direct-V';
            }
        }
        if (!engineSamples) {
            const harness = getPhysicsHarness(bridge);
            engineSamples = harness ? harness.sampleEFieldAlongRay(
                { x: pPos.x, y: pPos.y, z: pPos.z },
                { x: pNeg.x, y: pNeg.y, z: pNeg.z },
                PROBE_SAMPLES,
            ) : null;
            if (engineSamples) probeMode = 'js-trilinear';
        }
        if (!engineSamples) return null;

        const out = new Array(engineSamples.length);
        let maxAbsResidual = 0;
        let sumAbsResidual = 0;
        for (let i = 0; i < engineSamples.length; i++) {
            const { r, E_mag } = engineSamples[i];
            const rFar = dist - r;
            const E_analytic_signed = (rFar > 0)
                ? (ALPHA * q1) / (FOUR_PI * r * r) - (ALPHA * q2) / (FOUR_PI * rFar * rFar)
                : 0;
            const E_analytic_mag = Math.abs(E_analytic_signed);
            const residual = E_mag - E_analytic_mag;
            sumAbsResidual += Math.abs(residual);
            if (Math.abs(residual) > Math.abs(maxAbsResidual)) {
                maxAbsResidual = residual;
            }
            out[i] = { r, lattice: E_mag, analytic: E_analytic_mag, residual };
        }
        const meanAbsResidual = sumAbsResidual / out.length;
        return {
            samples: out,
            meta: {
                q1, q2,
                dist: dist.toFixed(2),
                count: particles.length,
                maxAbsResidual,
                meanAbsResidual,
                N: out.length,
                rMin: out[0].r,
                rMax: out[out.length - 1].r,
                probeMode,
            },
        };
    }

    _renderCoulombEngineProbe(container, samples) {
        const W = 360;
        const H = 180;
        const margin = { top: 28, right: 14, bottom: 28, left: 50 };
        const innerW = W - margin.left - margin.right;
        const innerH = H - margin.top - margin.bottom;

        const hasData = !!samples && samples.length > 0;

        let rmin = 0, rmax = 1, emin = 0, emax = 1, erange = 1;
        if (hasData) {
            const rs = samples.map((s) => s.r);
            const es = samples.flatMap((s) => [s.lattice, s.analytic]);
            rmin = Math.min(...rs);
            rmax = Math.max(...rs);
            emin = Math.min(...es);
            emax = Math.max(...es);
            if (emax === emin) { emax += 1e-9; emin -= 1e-9; }
            erange = emax - emin;
        }

        const xpx = (r) => margin.left + ((r - rmin) / (rmax - rmin || 1)) * innerW;
        const ypx = (e) => margin.top + (1 - (e - emin) / erange) * innerH;

        let svg = `<svg viewBox="0 0 ${W} ${H}" class="p1-svg-plot">`;
        svg += `<rect x="${margin.left}" y="${margin.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light, rgba(255,255,255,0.08))" stroke-width="1"/>`;

        svg += `<text x="${margin.left - 6}" y="${margin.top + 4}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? formatExp(emax) : ''}</text>`;
        svg += `<text x="${margin.left - 6}" y="${margin.top + innerH + 4}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? formatExp(emin) : ''}</text>`;

        svg += `<text x="${margin.left}" y="${margin.top + innerH + 14}" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">r=${rmin.toFixed(1)}</text>`;
        svg += `<text x="${margin.left + innerW}" y="${margin.top + innerH + 14}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">r=${rmax.toFixed(1)}</text>`;

        svg += `<text x="${margin.left + innerW / 2}" y="${H - 4}" text-anchor="middle" fill="var(--text-muted)" font-size="10">Radius r from positive charge center</text>`;

        if (hasData) {
            let pLattice = '';
            let pAnalytic = '';
            for (let i = 0; i < samples.length; i++) {
                const s = samples[i];
                const px = xpx(s.r).toFixed(2);
                const pyL = ypx(s.lattice).toFixed(2);
                const pyA = ypx(s.analytic).toFixed(2);
                pLattice += (i === 0 ? 'M' : 'L') + px + ',' + pyL;
                pAnalytic += (i === 0 ? 'M' : 'L') + px + ',' + pyA;
            }
            svg += `<path d="${pLattice}" stroke="var(--accent)" stroke-width="1.8" fill="none"/>`;
            svg += `<path d="${pAnalytic}" stroke="var(--text-muted)" stroke-width="1.2" stroke-dasharray="3,3" fill="none"/>`;

            const maxIndex = samples.reduce((bestIdx, currentVal, currentIdx, arr) =>
                Math.abs(currentVal.residual) > Math.abs(arr[bestIdx].residual) ? currentIdx : bestIdx
            , 0);

            const sMax = samples[maxIndex];
            const rx = xpx(sMax.r);
            const ryL = ypx(sMax.lattice);
            const ryA = ypx(sMax.analytic);

            svg += `<line x1="${rx.toFixed(2)}" y1="${ryL.toFixed(2)}" x2="${rx.toFixed(2)}" y2="${ryA.toFixed(2)}" stroke="red" stroke-width="1.2"/>`;
            svg += `<circle cx="${rx.toFixed(2)}" cy="${ryL.toFixed(2)}" r="3" fill="red"/>`;
            svg += `<circle cx="${rx.toFixed(2)}" cy="${ryA.toFixed(2)}" r="3" fill="red"/>`;
        } else {
            svg += `<text x="${margin.left + innerW / 2}" y="${margin.top + innerH / 2 + 4}" text-anchor="middle" fill="var(--text-muted)" font-size="11" font-style="italic">Need: ≥2 opposite-charge particles</text>`;
        }

        svg += `</svg>`;
        container.innerHTML = svg;
    }
}
