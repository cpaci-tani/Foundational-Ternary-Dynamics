// @ts-check
import { transformVelocity } from './math.js';
import { rotationMatrix } from './geometry.js';
/** @typedef {import('./types.js').WorldSnapshot} WorldSnapshot */
/** Pooled measurement annotations; geometry and optical hits remain untouched. */
export class ObserverSpatialLabels {
    /** @param {HTMLElement} host */
    constructor(host) {
        this.element = document.createElement('div');
        this.element.className = 'observer-spatial-labels';
        this.element.setAttribute('aria-hidden', 'true');
        /** @type {HTMLElement[]} */ this.labels = [];
        host.append(this.element);
    }
    /** @param {WorldSnapshot} snapshot @param {any[]} anchors @param {Record<string,boolean>} fields */
    update(snapshot, anchors, fields) {
        for (let index = 0; index < Math.min(32, anchors.length); index++) {
            const anchor = anchors[index];
            let label = this.labels[index];
            if (!label) {
                label = document.createElement('div'); label.className = 'observer-spatial-label';
                this.element.append(label); this.labels.push(label);
            }
            label.hidden = false;
            label.style.left = `${anchor.x * 100}%`; label.style.top = `${anchor.y * 100}%`;
            label.dataset.entity = anchor.entityId;
            label.dataset.mirrored = String(anchor.mirrored === true);
            const entity = anchor.entity;
            const received = anchor.segment ?? entity;
            const parts = [];
            if (fields.name !== false) parts.push(received.name || entity.name);
            if (fields.position) parts.push(`x ${received.position.map((/** @type {number} */ v, /** @type {number} */ i) => (v + (anchor.emissionTime - received.originTime) * received.velocity[i]).toFixed(2)).join(' · ')}`);
            if (fields.velocity) parts.push(`β ${Math.hypot(...received.velocity).toFixed(3)}`);
            if (fields.properTime) {
                const rate = snapshot.profile === 'sr' ? Math.sqrt(Math.max(0, 1 - received.velocity.reduce((/** @type {number} */ s, /** @type {number} */ v) => s + v * v, 0))) : 1;
                const clock = snapshot.profile === 'playground' ? entity.clockOffset : received.clockOffset + (anchor.emissionTime - received.originTime) * rate;
                parts.push(`τ ${clock.toFixed(2)}`);
            }
            if (fields.distance) parts.push(`d ${anchor.distance.toFixed(2)}`);
            if (fields.emissionTime) parts.push(`emitted ${anchor.emissionTime.toFixed(2)}`);
            if (fields.dimensions) parts.push(`size ${received.size.map((/** @type {number} */ v) => v.toFixed(2)).join(' × ')}`);
            if (fields.bounds) parts.push(`rest bounds ±[${received.size.map((/** @type {number} */ v) => (v / 2).toFixed(2)).join(', ')}]`);
            if (fields.axes) {
                const basis = rotationMatrix(received.rotation);
                for (let j = 0; j < 3; j++) parts.push(`${'XYZ'[j]}̂ [${[basis[j], anchor.mirrored ? -basis[j + 3] : basis[j + 3], basis[j + 6]].map(v => v.toFixed(2)).join(', ')}]`);
            }
            if (fields.trajectory) parts.push(`segment ${Number(received.start ?? entity.originTime).toFixed(2)} → ${received.end == null ? 'ongoing' : Number(received.end).toFixed(2)}`);
            if (fields.frameVelocity) {
                const velocity = snapshot.profile === 'sr' ? transformVelocity(received.velocity, snapshot.observer.velocity)
                    : received.velocity.map((/** @type {number} */ v, /** @type {number} */ j) => v - snapshot.observer.velocity[j]);
                parts.push(`your frame v [${velocity.map((/** @type {number} */ v) => v.toFixed(3)).join(', ')}]`);
            }
            if (anchor.historical) parts.push('HISTORICAL IMAGE');
            if (anchor.mirrored) parts.push('MIRRORED IMAGE · source object');
            label.textContent = parts.join('\n');
        }
        for (let index = anchors.length; index < this.labels.length; index++) this.labels[index].hidden = true;
    }
    dispose() { this.labels.length = 0; this.element.remove(); }
}
