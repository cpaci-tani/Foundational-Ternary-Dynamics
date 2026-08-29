/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/anisotropy.js
 * @purpose Lattice Anisotropy & SO(2) Recovery component.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { cardStyle, titleStyle, tagBadge } from '../_card-helpers.js';

const RADII = [1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5];
const ANGULAR_SAMPLES = 16;
const NATIVE_SAMPLE_INTERVAL_MS = 1000;
const LOCAL_SAMPLE_INTERVAL_MS = 250;

/**
 * Turn either a dense local volume or native FTV2 compact frame into one
 * regular-grid sampler. FTV2's cells are measured block representatives from
 * the engine's flux field, not synthesized chart values.
 */
export function makeFluxMagnitudeSampler(volume, latticeSize) {
    const L = Math.max(1, Math.trunc(Number(latticeSize) || 0));
    if (ArrayBuffer.isView(volume)) {
        const needed = L * L * L;
        if (volume.length < needed) return null;
        return { data: volume, latticeSize: L, axisCount: L, stride: 1 };
    }
    if (!volume || !ArrayBuffer.isView(volume.data)) return null;
    const axisCount = Math.trunc(Number(volume.axisCount) || 0);
    const stride = Math.max(1, Math.trunc(Number(volume.stride) || 1));
    const frameL = Math.trunc(Number(volume.latticeSize) || L);
    if (frameL !== L || axisCount < 1 || volume.data.length < axisCount ** 3) return null;
    return { data: volume.data, latticeSize: L, axisCount, stride };
}

/** Read one actual regular-grid |J| value at a periodic lattice position. */
export function sampleFluxMagnitude(sampler, x, y, z) {
    if (!sampler) return null;
    const { data, latticeSize: L, axisCount, stride } = sampler;
    const wrap = (v) => ((Math.round(v) % L) + L) % L;
    const xi = Math.min(axisCount - 1, Math.floor(wrap(x) / stride));
    const yi = Math.min(axisCount - 1, Math.floor(wrap(y) / stride));
    const zi = Math.min(axisCount - 1, Math.floor(wrap(z) / stride));
    const value = Number(data[(zi * axisCount + yi) * axisCount + xi]);
    return Number.isFinite(value) ? value : null;
}

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
        this._lastBridge = null;
        this._lastSourceKey = '';
        this._lastSampleAt = -Infinity;
        this._decayPoints = null;
        this._sampleStride = 1;
    }

    update(bridge, now = performance.now(), particles = null) {
        if (!bridge) return;
        const particleList = particles || bridge.getScale0ParticleList?.() || [];
        const activeSource = particleList.find((p) => (p.state ?? 0) !== 0);
        const L = (typeof bridge.getLatticeSize === 'function' ? bridge.getLatticeSize() : bridge.latticeSize) || 32;
        const cx = activeSource ? activeSource.x : L / 2;
        const cy = activeSource ? activeSource.y : L / 2;
        const cz = activeSource ? activeSource.z : L / 2;
        const sourceKey = `${L}:${cx}:${cy}:${cz}:${activeSource?.id ?? 'center'}`;
        const interval = bridge.isNativeGPU ? NATIVE_SAMPLE_INTERVAL_MS : LOCAL_SAMPLE_INTERVAL_MS;
        const sourceChanged = bridge !== this._lastBridge || sourceKey !== this._lastSourceKey;
        if (sourceChanged) this._decayPoints = null;

        // The old implementation made 128 independent inspect_voxel requests
        // on every 4 Hz panel pass (plus the inspector's own requests). Native
        // runs now consume one bounded FTV2 |J| frame, then evaluate all of the
        // same 8×16 circular probes locally from that measured regular grid.
        if (sourceChanged || now - this._lastSampleAt >= interval) {
            this._lastBridge = bridge;
            this._lastSourceKey = sourceKey;
            this._lastSampleAt = now;
            const sampler = makeFluxMagnitudeSampler(bridge.getFluxVolume?.(), L);
            if (sampler) {
                this._decayPoints = this._computeDecayPoints(sampler, cx, cy, cz);
                this._sampleStride = sampler.stride;
            }
        }

        const decayPoints = this._decayPoints;
        this._renderAnisotropyDecay(this.refs.plot, decayPoints);
        const sourceLabel = activeSource ? `charge ID ${activeSource.id} (${activeSource.state > 0 ? '+' : ''}${activeSource.state})` : 'grid center';
        if (!decayPoints) {
            this.refs.desc.innerHTML = `
                <div class="p1-anisotropy-desc-sub">${tagBadge('M')}Waiting for the engine's compact |J| field sample; no inferred zero-value probe is shown.</div>
            `;
            return;
        }

        const minAniso = decayPoints[decayPoints.length - 1].aniso;
        this.refs.desc.innerHTML = `
            <div class="p1-anisotropy-desc-flex">
                <span>${tagBadge('M')}Source: ${sourceLabel}</span>
                <span>${tagBadge('~M')}Anisotropy: ${minAniso.toFixed(2)}% (at r=8.5a)</span>
            </div>
            <div class="p1-anisotropy-desc-sub">
                ${tagBadge('M')} Rotational symmetry recovery O_h → SO(2) quantified via σ_rel(r) = σ(r)/⟨|J|(r)⟩ × 100% over 16-point circular samplers. The values are measured from the engine's bounded regular |J| grid (stride ${this._sampleStride}a), not from synthetic fallback values.
            </div>
        `;
    }

    _computeDecayPoints(sampler, cx, cy, cz) {
        const decayPoints = [];
        for (const r of RADII) {
            const values = [];
            for (let i = 0; i < ANGULAR_SAMPLES; i++) {
                const theta = (i * 2.0 * Math.PI) / ANGULAR_SAMPLES;
                const value = sampleFluxMagnitude(
                    sampler,
                    cx + r * Math.cos(theta),
                    cy + r * Math.sin(theta),
                    cz,
                );
                if (value !== null) values.push(value);
            }
            if (values.length !== ANGULAR_SAMPLES) return null;
            const mean = values.reduce((sum, value) => sum + value, 0) / values.length;
            const variance = values.reduce((sum, value) => sum + (value - mean) ** 2, 0) / values.length;
            decayPoints.push({ r, aniso: mean > 1e-9 ? Math.sqrt(variance) / mean * 100.0 : 0.0, mean });
        }
        return decayPoints;
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

        svg += `<text x="${margin.left - 6}" y="${margin.top + 4}" text-anchor="end" fill="var(--text-muted)" font-size="16" font-family="var(--font-mono)">${hasData ? amax.toFixed(1) + '%' : ''}</text>`;
        svg += `<text x="${margin.left - 6}" y="${margin.top + innerH + 4}" text-anchor="end" fill="var(--text-muted)" font-size="16" font-family="var(--font-mono)">0%</text>`;

        svg += `<text x="${margin.left}" y="${margin.top + innerH + 14}" fill="var(--text-muted)" font-size="16" font-family="var(--font-mono)">r=${rmin.toFixed(1)}a</text>`;
        svg += `<text x="${margin.left + innerW}" y="${margin.top + innerH + 14}" text-anchor="end" fill="var(--text-muted)" font-size="16" font-family="var(--font-mono)">r=${rmax.toFixed(1)}a</text>`;

        svg += `<text x="${margin.left + innerW / 2}" y="${H - 4}" text-anchor="middle" fill="var(--text-muted)" font-size="16">Radius r from charge center</text>`;

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
