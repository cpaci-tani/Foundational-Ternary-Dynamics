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

/** The existing 8×16 ring preparation, rounded and wrapped to dense cells. */
export function circularFluxProbeIndices(L, cx, cy, cz) {
    if (!Number.isSafeInteger(L) || L < 1 || !Number.isSafeInteger(L ** 3)
        || ![cx, cy, cz].every(Number.isFinite)) return null;
    const wrap = (v) => ((Math.round(v) % L) + L) % L;
    const indices = new Float64Array(RADII.length * ANGULAR_SAMPLES);
    let next = 0;
    for (const r of RADII) {
        for (let i = 0; i < ANGULAR_SAMPLES; i++) {
            const theta = (i * 2.0 * Math.PI) / ANGULAR_SAMPLES;
            const x = wrap(cx + r * Math.cos(theta));
            const y = wrap(cy + r * Math.sin(theta));
            const z = wrap(cz);
            indices[next++] = (z * L + y) * L + x;
        }
    }
    return indices;
}

/** Preserve ring order and the original two-pass mean/variance operations. */
export function circularFluxDecay(samples) {
    if (!ArrayBuffer.isView(samples) || samples.length !== RADII.length * ANGULAR_SAMPLES) return null;
    const decayPoints = [];
    for (let ring = 0; ring < RADII.length; ring++) {
        const values = [];
        for (let i = 0; i < ANGULAR_SAMPLES; i++) {
            const value = Number(samples[ring * ANGULAR_SAMPLES + i]);
            if (!Number.isFinite(value)) return null;
            values.push(value);
        }
        const mean = values.reduce((sum, value) => sum + value, 0) / values.length;
        if (!Number.isFinite(mean) || mean <= 0) return null;
        const variance = values.reduce((sum, value) => sum + (value - mean) ** 2, 0) / values.length;
        const aniso = Math.sqrt(variance) / mean * 100;
        if (!Number.isFinite(aniso)) return null;
        decayPoints.push({ r: RADII[ring], aniso, mean });
    }
    return decayPoints;
}

const TEMPLATE = `
    <section data-section="anisotropy" style="${cardStyle(220)}">
        <div style="${titleStyle()}">Sampled circular field anisotropy</div>
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
        const sourceKey = `${bridge.configurationToken ?? ''}:${L}:${cx}:${cy}:${cz}:${activeSource?.id ?? 'center'}`;
        const interval = bridge.isNativeGPU ? NATIVE_SAMPLE_INTERVAL_MS : LOCAL_SAMPLE_INTERVAL_MS;
        const sourceChanged = bridge !== this._lastBridge || sourceKey !== this._lastSourceKey;
        if (sourceChanged) this._decayPoints = null;

        // One worker pin supplies exactly the same 128 nearest-cell samples.
        // Direct/reference bridges without this API retain their dense or
        // compact-volume sampling path and its declared regular-grid stride.
        if (sourceChanged || now - this._lastSampleAt >= interval) {
            this._lastBridge = bridge;
            this._lastSourceKey = sourceKey;
            this._lastSampleAt = now;
            if (typeof bridge.sampleFluxAtCells === 'function') {
                if (sourceChanged || !this._probeIndices) {
                    this._probeIndices = circularFluxProbeIndices(L, cx, cy, cz);
                }
                const samples = this._probeIndices ? bridge.sampleFluxAtCells(this._probeIndices) : null;
                this._decayPoints = circularFluxDecay(samples);
                this._sampleStride = 1;
            } else {
                const sampler = makeFluxMagnitudeSampler(bridge.getFluxVolume?.(), L);
                if (sampler) {
                    this._decayPoints = this._computeDecayPoints(sampler, cx, cy, cz);
                    this._sampleStride = sampler.stride;
                } else this._decayPoints = null;
            }
        }

        const decayPoints = this._decayPoints;
        const sourceLabel = activeSource ? `charge ID ${activeSource.id} (${activeSource.state > 0 ? '+' : ''}${activeSource.state})` : 'grid center';
        // Retained observations do not need a new SVG/description each UI pass.
        if (this._renderedDecayPoints === decayPoints
            && this._renderedSourceLabel === sourceLabel && this._renderedStride === this._sampleStride) return;
        this._renderedDecayPoints = decayPoints;
        this._renderedSourceLabel = sourceLabel;
        this._renderedStride = this._sampleStride;
        this._renderAnisotropyDecay(this.refs.plot, decayPoints);
        if (!decayPoints) {
            this.refs.desc.innerHTML = `
                <div class="p1-anisotropy-desc-sub">Anisotropy unavailable: complete finite samples and a positive mean on every ring are required.</div>
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
                ${tagBadge('M')} σ_rel(r) = σ(r)/⟨|J|(r)⟩ × 100% over 16 circular samples from the bounded regular |J| grid (stride ${this._sampleStride}a). Nearest-cell sampling and periodic wrap affect this diagnostic; it does not establish continuum rotational symmetry.
            </div>
        `;
    }

    _computeDecayPoints(sampler, cx, cy, cz) {
        const samples = new Float64Array(RADII.length * ANGULAR_SAMPLES);
        let next = 0;
        for (const r of RADII) {
            for (let i = 0; i < ANGULAR_SAMPLES; i++) {
                const theta = (i * 2.0 * Math.PI) / ANGULAR_SAMPLES;
                const value = sampleFluxMagnitude(
                    sampler,
                    cx + r * Math.cos(theta),
                    cy + r * Math.sin(theta),
                    cz,
                );
                if (value === null) return null;
                samples[next++] = value;
            }
        }
        return circularFluxDecay(samples);
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
