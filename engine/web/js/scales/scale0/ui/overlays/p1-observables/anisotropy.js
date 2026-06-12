/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/anisotropy.js
 * @purpose Lattice Anisotropy & SO(2) Recovery component.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { cardStyle, titleStyle, tagBadge } from '../_card-helpers.js';

const TEMPLATE = `
    <section data-section="anisotropy" style="${cardStyle(220)}">
        <div style="${titleStyle()}">Lattice Anisotropy & SO(2) Recovery</div>
        <div ref="plot" class="p1-anisotropy-plot-box"></div>
        <div ref="desc" class="p1-anisotropy-desc-box"></div>
    </section>
`;

export class AnisotropyComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
    }

    update(bridge) {
        const particles = bridge.getScale0ParticleList?.() || [];
        const activeSource = particles.find((p) => (p.state ?? 0) !== 0);
        const L = (typeof bridge.getLatticeSize === 'function' ? bridge.getLatticeSize() : bridge.latticeSize) || 32;
        const cx = activeSource ? activeSource.x : L / 2;
        const cy = activeSource ? activeSource.y : L / 2;
        const cz = activeSource ? activeSource.z : L / 2;

        const decayPoints = [];
        const radii = [1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5];
        const N_pts = 16;

        for (const r of radii) {
            const values = [];
            for (let i = 0; i < N_pts; i++) {
                const theta = (i * 2.0 * Math.PI) / N_pts;
                const sx = Math.round(cx + r * Math.cos(theta));
                const sy = Math.round(cy + r * Math.sin(theta));
                const sz = Math.round(cz);

                // Wrap periodic boundaries
                const wx = (sx % L + L) % L;
                const wy = (sy % L + L) % L;
                const wz = (sz % L + L) % L;

                const voxel = bridge.inspectVoxel(wx, wy, wz);
                if (voxel) {
                    const val = (voxel.Emag !== undefined && voxel.Emag !== null) ? voxel.Emag :
                                (typeof voxel.density === 'number' ? voxel.density :
                                Math.sqrt((voxel.fluxX || 0)**2 + (voxel.fluxY || 0)**2 + (voxel.fluxZ || 0)**2));
                    values.push(val);
                } else {
                    values.push(0);
                }
            }

            // Compute mean
            const sum = values.reduce((a, b) => a + b, 0);
            const mean = sum / N_pts;

            // Compute standard deviation
            let variance = 0;
            if (N_pts > 1) {
                const sqDiffs = values.map(v => (v - mean) ** 2);
                const sumSqDiffs = sqDiffs.reduce((a, b) => a + b, 0);
                variance = sumSqDiffs / N_pts;
            }
            const stdDev = Math.sqrt(variance);

            // Relative standard deviation in percent
            const aniso = mean > 1e-9 ? (stdDev / mean) * 100.0 : 0.0;
            decayPoints.push({ r, aniso, mean });
        }

        this._renderAnisotropyDecay(this.refs.plot, decayPoints);

        const minAniso = decayPoints.length > 0 ? decayPoints[decayPoints.length - 1].aniso : 0;
        const sourceLabel = activeSource ? `charge ID ${activeSource.id} (${activeSource.state > 0 ? '+' : ''}${activeSource.state})` : 'grid center';

        this.refs.desc.innerHTML = `
            <div class="p1-anisotropy-desc-flex">
                <span>${tagBadge('M')}Source: ${sourceLabel}</span>
                <span>${tagBadge('~M')}Anisotropy: ${minAniso.toFixed(2)}% (at r=8.5a)</span>
            </div>
            <div class="p1-anisotropy-desc-sub">
                ${tagBadge('D')} Rotational symmetry recovery O_h → SO(2) quantified via relative standard deviation σ_rel(r) = σ(r)/⟨E(r)⟩ × 100% over 16-point circular samplers. Note the power-law decay of grid discretization noise as r → ∞.
            </div>
        `;
    }

    _renderAnisotropyDecay(container, decayPoints) {
        const W = 360;
        const H = 130;
        const margin = { top: 15, right: 14, bottom: 25, left: 50 };
        const innerW = W - margin.left - margin.right;
        const innerH = H - margin.top - margin.bottom;
        const hasData = !!decayPoints && decayPoints.length > 0;

        let rmin = 1.5, rmax = 8.5, amin = 0, amax = 100;
        if (hasData) {
            rmin = Math.min(...decayPoints.map(p => p.r));
            rmax = Math.max(...decayPoints.map(p => p.r));
            const anis = decayPoints.map(p => p.aniso);
            amin = 0;
            amax = Math.max(...anis) * 1.05 || 10;
            if (amax < 10) amax = 10;
        }

        const xpx = (r) => margin.left + ((r - rmin) / (rmax - rmin || 1)) * innerW;
        const ypx = (aniso) => margin.top + (1 - (aniso - amin) / (amax - amin || 1)) * innerH;

        let svg = `<svg viewBox="0 0 ${W} ${H}" class="p1-svg-plot">`;
        svg += `<rect x="${margin.left}" y="${margin.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light, rgba(255,255,255,0.08))" stroke-width="1"/>`;

        svg += `<text x="${margin.left - 6}" y="${margin.top + 4}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? amax.toFixed(1) + '%' : ''}</text>`;
        svg += `<text x="${margin.left - 6}" y="${margin.top + innerH + 4}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">0%</text>`;

        svg += `<text x="${margin.left}" y="${margin.top + innerH + 14}" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">r=${rmin.toFixed(1)}a</text>`;
        svg += `<text x="${margin.left + innerW}" y="${margin.top + innerH + 14}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">r=${rmax.toFixed(1)}a</text>`;

        svg += `<text x="${margin.left + innerW / 2}" y="${H - 4}" text-anchor="middle" fill="var(--text-muted)" font-size="10">Radius r from charge center</text>`;

        if (hasData) {
            let path = '';
            for (let i = 0; i < decayPoints.length; i++) {
                const p = decayPoints[i];
                const px = xpx(p.r).toFixed(2);
                const py = ypx(p.aniso).toFixed(2);
                path += (i === 0 ? 'M' : 'L') + px + ',' + py;
            }
            svg += `<path d="${path}" stroke="var(--accent)" stroke-width="1.8" fill="none"/>`;

            for (const p of decayPoints) {
                const cx = xpx(p.r).toFixed(2);
                const cy = ypx(p.aniso).toFixed(2);
                svg += `<circle cx="${cx}" cy="${cy}" r="3" fill="var(--accent)"/>`;
            }
        }

        svg += `</svg>`;
        container.innerHTML = svg;
    }
}
