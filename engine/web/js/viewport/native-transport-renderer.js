// engine/web/js/viewport/native-transport-renderer.js
//
// Native transport of the Scale-0 reference engine, drawn on lattice links (spec
// 2026-09-15 native transport overlays, section 7). Segments join site centres
// only; nothing is interpolated or integrated. The half nearer the receiving
// site is brighter. White markers are the engine tracker's manifested clusters.
// Magenta markers are sites where energy is exchanged off the links; they are
// drawn only when the balance genuinely fails and never while the thermostat
// runs, because the thermostat exchanges energy at nearly every site.
import * as THREE from 'three';
import { buildStreamlineMesh } from './mesh-factory.js';
import { selectLinks, writeSelectedLinks } from '../link-geometry.js';
import { fluxToColorInto } from '../fields.js';

const LINK_CAP = 20000;
const TAIL_DIM = 0.4;
const MARKER_CAP = 6000;
const LANGEVIN_BIT = 1 << 0;
const EXCHANGE_CLOSURE_FLOOR = 1e-9;

export class NativeTransportRenderer {
    constructor({ scene }) {
        this._scene = scene;
        this._visible = false;
        this._mesh = null;
        this._markers = null;
    }

    _ensure() {
        if (this._mesh) return;
        this._mesh = buildStreamlineMesh(this._scene, 4 * LINK_CAP, 0.9);
        this._head = new Uint8Array(LINK_CAP);
        this._scratch = new Float32Array(12 * LINK_CAP);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(new Float32Array(3 * MARKER_CAP), 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(new Float32Array(3 * MARKER_CAP), 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.PointsMaterial({
            size: 0.9, vertexColors: true, sizeAttenuation: true, transparent: true, depthWrite: false,
        });
        this._markers = new THREE.Points(geo, mat);
        this._markers.visible = false;
        this._markers.frustumCulled = false;
        this._scene.add(this._markers);
    }

    setVisible(on) {
        this._visible = !!on;
        if (!this._mesh) {
            if (!this._visible) return;
            this._ensure();
        }
        this._mesh.visible = this._visible && this._mesh.geometry.drawRange.count > 0;
        this._markers.visible = this._visible && this._markers.geometry.drawRange.count > 0;
    }

    update({ sample, knots = null, fraction = 0.05, cap = LINK_CAP } = {}) {
        this._ensure();
        const summary = {
            status: sample?.status ?? 'off',
            reason: sample?.reason ?? '',
            closure: Number(sample?.closure ?? 0),
            exchangeTerms: (sample?.activeExchangeTerms ?? 0) >>> 0,
            drawnLinks: 0, max: 0, knotCount: 0, exchangeSites: 0, exchangeHidden: false,
        };
        const ok = summary.status === 'ok' && sample.L > 0;
        const N = ok ? sample.L ** 3 : 0;

        let vertices = 0;
        if (ok && sample.links?.length === 9 * N) {
            const { ids, max } = selectLinks(sample.links, fraction, Math.min(cap, LINK_CAP));
            vertices = writeSelectedLinks(sample.L, sample.links, ids, this._scratch, this._head);
            const pos = this._mesh.geometry.getAttribute('position');
            const col = this._mesh.geometry.getAttribute('color');
            pos.array.set(this._scratch.subarray(0, 3 * vertices));
            for (let i = 0; i < vertices / 4; i++) {
                const base = 12 * i;
                fluxToColorInto(col.array, base, Math.abs(sample.links[ids[i]]), max);
                const r = col.array[base], g = col.array[base + 1], b = col.array[base + 2];
                const ownerScale = this._head[i] ? TAIL_DIM : 1;
                const neighbourScale = this._head[i] ? 1 : TAIL_DIM;
                for (let v = 0; v < 4; v++) {
                    const s = v < 2 ? ownerScale : neighbourScale;
                    col.array[base + 3 * v] = r * s;
                    col.array[base + 3 * v + 1] = g * s;
                    col.array[base + 3 * v + 2] = b * s;
                }
            }
            pos.needsUpdate = true;
            col.needsUpdate = true;
            summary.drawnLinks = vertices / 4;
            summary.max = max;
        }
        this._mesh.geometry.setDrawRange(0, vertices);

        const mpos = this._markers.geometry.getAttribute('position');
        const mcol = this._markers.geometry.getAttribute('color');
        let m = 0;
        if (knots?.count > 0 && knots.fields) {
            const stride = knots.stride || 11;
            for (let k = 0; k < knots.count && m < MARKER_CAP; k++, m++) {
                mpos.array[3 * m] = knots.fields[stride * k] + 0.5;
                mpos.array[3 * m + 1] = knots.fields[stride * k + 1] + 0.5;
                mpos.array[3 * m + 2] = knots.fields[stride * k + 2] + 0.5;
                mcol.array[3 * m] = 1; mcol.array[3 * m + 1] = 1; mcol.array[3 * m + 2] = 1;
            }
            summary.knotCount = knots.count;
        }
        if (ok && sample.residual?.length === N) {
            if (summary.exchangeTerms & LANGEVIN_BIT) {
                summary.exchangeHidden = true;
            } else if (summary.closure > EXCHANGE_CLOSURE_FLOOR) {
                const L = sample.L;
                const { ids } = selectLinks(sample.residual, fraction, MARKER_CAP - m);
                for (let q = 0; q < ids.length; q++, m++) {
                    const s = ids[q];
                    mpos.array[3 * m] = Math.floor(s / (L * L)) + 0.5;
                    mpos.array[3 * m + 1] = (Math.floor(s / L) % L) + 0.5;
                    mpos.array[3 * m + 2] = (s % L) + 0.5;
                    mcol.array[3 * m] = 1; mcol.array[3 * m + 1] = 0.25; mcol.array[3 * m + 2] = 0.85;
                }
                summary.exchangeSites = ids.length;
            }
        }
        mpos.needsUpdate = true;
        mcol.needsUpdate = true;
        this._markers.geometry.setDrawRange(0, m);

        this._mesh.visible = this._visible && vertices > 0;
        this._markers.visible = this._visible && m > 0;
        return summary;
    }

    dispose() {
        for (const obj of [this._mesh, this._markers]) {
            if (!obj) continue;
            this._scene.remove(obj);
            obj.geometry?.dispose();
            obj.material?.dispose();
        }
        this._mesh = null;
        this._markers = null;
    }
}
