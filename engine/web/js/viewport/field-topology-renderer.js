/** fieldTopologyMethods — ViewportFieldRenderer mixin (halo / damping / genesis / strings). */
import * as THREE from 'three';
import { K_GENESIS } from '../constants.js';
import { rampCyclicHSL } from './color-ramps.js';
import { knotHue } from '../scales/scale0/runtime/field-line-knots.js';
import {
    CONFINEMENT_PAIR_DIST2,
    _makeParticleFragMaterial,
    _ensureManifestAttrs,
} from './field-renderer-shared.js';

function resolveFluxMagnitudeGrid(fluxMag, latticeSize) {
    const N = Math.trunc(Number(latticeSize) || 0);
    if (fluxMag && !ArrayBuffer.isView(fluxMag) && ArrayBuffer.isView(fluxMag.data)) {
        const axisCount = Math.trunc(Number(fluxMag.axisCount) || 0);
        const stride = Math.max(1, Number(fluxMag.stride) || 1);
        if (Math.trunc(Number(fluxMag.latticeSize)) === N && axisCount > 0
            && fluxMag.data.length === axisCount * axisCount * axisCount) {
            return { data: fluxMag.data, axisCount, stride, compact: true };
        }
        return null;
    }
    if (ArrayBuffer.isView(fluxMag) && N > 0 && fluxMag.length >= N * N * N) {
        return { data: fluxMag, axisCount: N, stride: 1, compact: false };
    }
    return null;
}

export const fieldTopologyMethods = {
    _buildDarkMatterHalo() {
        const maxPts = 8000;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        _ensureManifestAttrs(geo, maxPts);
        geo.setDrawRange(0, 0);
        const mat = _makeParticleFragMaterial(
            { uOpacity: { value: 0.35 } },
            { blending: THREE.AdditiveBlending },
        );
        this._darkMatterHalo = new THREE.Points(geo, mat);
        this._darkMatterHalo.visible = false;
        this._darkMatterHalo.frustumCulled = false;
        this._darkMatterHalo.renderOrder = 1;
        this._scene.add(this._darkMatterHalo);
    },
    updateDarkMatterHalo(particles, fluxMag, latticeSize) {
        this._syncCenterAndRadius();
        if (!this._darkMatterHalo) this._buildDarkMatterHalo();
        const posAttr = this._darkMatterHalo.geometry.getAttribute('position');
        const colAttr = this._darkMatterHalo.geometry.getAttribute('particleColor');
        const sizeAttr = this._darkMatterHalo.geometry.getAttribute('size');
        const N = latticeSize;
        const grid = resolveFluxMagnitudeGrid(fluxMag, N);
        if (!grid) {
            this._darkMatterHalo.geometry.setDrawRange(0, 0);
            return;
        }
        const kGen = K_GENESIS; // 3 * K_MANIFEST = 1.5164 (W_SC kinetics, FTD-0388; audit P2-9 fix: import the named constant, 2026-05-27)
        let vi = 0;
        const maxPts = 8000;

        const gridStep = grid.compact ? 1 : (N > 64 ? 4 : (N > 24 ? 2 : 1));
        const pointScale = grid.compact ? grid.stride : gridStep;
        const A = grid.axisCount;
        for (let gz = 0; gz < A && vi < maxPts; gz += gridStep) {
            for (let gy = 0; gy < A && vi < maxPts; gy += gridStep) {
                for (let gx = 0; gx < A && vi < maxPts; gx += gridStep) {
                    const idx = (gz * A + gy) * A + gx;
                    const mag = grid.data[idx];
                    if (mag > 0.003 && mag < kGen) {
                        const t = mag / kGen;
                        posAttr.array[vi * 3]     = Math.min(gx * grid.stride, N - 1) + 0.5;
                        posAttr.array[vi * 3 + 1] = Math.min(gy * grid.stride, N - 1) + 0.5;
                        posAttr.array[vi * 3 + 2] = Math.min(gz * grid.stride, N - 1) + 0.5;
                        colAttr.array[vi * 3] = 0.3 + t * 0.4;
                        colAttr.array[vi * 3 + 1] = 0.1 + t * 0.15;
                        colAttr.array[vi * 3 + 2] = 0.5 + t * 0.4;
                        sizeAttr.array[vi] = (1.0 + 4.0 * t) * pointScale;
                        vi++;
                    }
                }
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._darkMatterHalo.geometry.setDrawRange(0, vi);
    },
    toggleDarkMatterHalo(on) {
        if (!this._darkMatterHalo) this._buildDarkMatterHalo();
        this._darkMatterHalo.visible = on;
        if (!on) this._darkMatterHalo.geometry.setDrawRange(0, 0);
    },

    // ── Event Horizon Sphere (Scale 1 black hole scenario) ─────────────
    _buildEventHorizon() {
        const geo = new THREE.SphereGeometry(1, 32, 24);
        const mat = new THREE.MeshBasicMaterial({
            color: 0x000000,
            transparent: true,
            opacity: 0.75,
            side: THREE.FrontSide,
            depthWrite: false,
        });
        this._eventHorizonSphere = new THREE.Mesh(geo, mat);
        this._eventHorizonSphere.visible = false;
        this._eventHorizonSphere.renderOrder = 10;
        this._scene.add(this._eventHorizonSphere);

        const ringGeo = new THREE.TorusGeometry(1, 0.06, 8, 48);
        const ringMat = new THREE.MeshBasicMaterial({
            color: 0xff8800,
            transparent: true,
            opacity: 0.65,
            depthWrite: false,
        });
        this._eventHorizonRing = new THREE.Mesh(ringGeo, ringMat);
        this._eventHorizonRing.visible = false;
        this._eventHorizonRing.renderOrder = 11;
        this._scene.add(this._eventHorizonRing);
    },
    _buildDampingZones() {
        const maxSegments = 1200;
        const positions = new Float32Array(maxSegments * 2 * 3);
        const colors = new Float32Array(maxSegments * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.4,
        });
        this._dampingZones = new THREE.LineSegments(geo, mat);
        this._dampingZones.visible = false;
        this._dampingZones.frustumCulled = false;
        this._dampingZones.renderOrder = 2;
        this._scene.add(this._dampingZones);
    },
    updateDampingZones(particles, latticeSize) {
        this._syncCenterAndRadius();
        if (!this._dampingZones) this._buildDampingZones();
        const posAttr = this._dampingZones.geometry.getAttribute('position');
        const colAttr = this._dampingZones.geometry.getAttribute('color');
        let si = 0;

        const edges = [
            [0, 0, 0, 1, 0, 0], [0, 1, 0, 1, 1, 0], [0, 0, 1, 1, 0, 1], [0, 1, 1, 1, 1, 1],
            [0, 0, 0, 0, 1, 0], [1, 0, 0, 1, 1, 0], [0, 0, 1, 0, 1, 1], [1, 0, 1, 1, 1, 1],
            [0, 0, 0, 0, 0, 1], [1, 0, 0, 1, 0, 1], [0, 1, 0, 0, 1, 1], [1, 1, 0, 1, 1, 1],
        ];

        // `particles` is the flat Float32Array from getParticleData() (x,y,z,x,y,z…).
        // Iterate by index, not as {x,y,z} objects (which would yield NaN).
        const maxSegments = 1200;
        const nParticles = particles ? (particles.length / 3) | 0 : 0;
        for (let pi = 0; pi < nParticles; pi++) {
            if (si + 12 > maxSegments) break;
            const cx = particles[pi * 3] + 0.5;
            const cy = particles[pi * 3 + 1] + 0.5;
            const cz = particles[pi * 3 + 2] + 0.5;
            for (const e of edges) {
                const i = si * 6;
                posAttr.array[i] = cx - 1.5 + e[0] * 3;
                posAttr.array[i + 1] = cy - 1.5 + e[1] * 3;
                posAttr.array[i + 2] = cz - 1.5 + e[2] * 3;
                posAttr.array[i + 3] = cx - 1.5 + e[3] * 3;
                posAttr.array[i + 4] = cy - 1.5 + e[4] * 3;
                posAttr.array[i + 5] = cz - 1.5 + e[5] * 3;
                colAttr.array[i] = 0.8; colAttr.array[i + 1] = 0.2; colAttr.array[i + 2] = 0.2;
                colAttr.array[i + 3] = 0.8; colAttr.array[i + 4] = 0.2; colAttr.array[i + 5] = 0.2;
                si++;
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._dampingZones.geometry.setDrawRange(0, si * 2);
    },
    toggleDampingZones(on) {
        if (!this._dampingZones) this._buildDampingZones();
        this._dampingZones.visible = on;
        if (!on) this._dampingZones.geometry.setDrawRange(0, 0);
    },

    // ── Topological Knots (wireframe cubes around manifested states) ──
    _buildKnotZones() {
        const maxSegments = 1600;   // E + B knot boxes (up to 64+64 knots × 12 edges)
        const positions = new Float32Array(maxSegments * 2 * 3);
        const colors = new Float32Array(maxSegments * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.6,
        });
        this._knotZones = new THREE.LineSegments(geo, mat);
        this._knotZones.visible = false;
        this._knotZones.frustumCulled = false;
        this._knotZones.renderOrder = 2;
        this._scene.add(this._knotZones);
    },

    // Accepts: a dual field-line-knot frame { e:{...}, b:{...} } (draws both the
    // E and B knot families, hued per field); a single { centroids, extents, count
    // ids, selectedId, perKnotColor } frame; or a bare Float32Array of particle
    // positions (legacy, fixed 3-voxel box).
    updateKnotZones(frame, latticeSize) {
        this._syncCenterAndRadius();
        if (!this._knotZones) this._buildKnotZones();
        const posAttr = this._knotZones.geometry.getAttribute('position');
        const colAttr = this._knotZones.geometry.getAttribute('color');
        const maxSegments = 1600;
        let si = 0;
        if (frame && (frame.e || frame.b || frame.flux)) {
            si = this._writeKnotBoxSet(posAttr, colAttr, si, maxSegments, frame.e, 'e');
            si = this._writeKnotBoxSet(posAttr, colAttr, si, maxSegments, frame.b, 'b');
            si = this._writeKnotBoxSet(posAttr, colAttr, si, maxSegments, frame.flux, 'flux');
        } else {
            si = this._writeKnotBoxSet(posAttr, colAttr, si, maxSegments, frame, 'e');
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._knotZones.geometry.setDrawRange(0, si * 2);
    },
    _writeKnotBoxSet(posAttr, colAttr, si, maxSegments, frame, field) {
        const edges = [
            [0, 0, 0, 1, 0, 0], [0, 1, 0, 1, 1, 0], [0, 0, 1, 1, 0, 1], [0, 1, 1, 1, 1, 1],
            [0, 0, 0, 0, 1, 0], [1, 0, 0, 1, 1, 0], [0, 0, 1, 0, 1, 1], [1, 0, 1, 1, 1, 1],
            [0, 0, 0, 0, 0, 1], [1, 0, 0, 1, 0, 1], [0, 1, 0, 0, 1, 1], [1, 1, 0, 1, 1, 1],
        ];
        let centroids, extents, count, ids = null, selectedId = -1, perKnotColor = true;
        if (frame && frame.centroids) {
            centroids = frame.centroids; extents = frame.extents; count = frame.count | 0;
            ids = frame.ids || null;
            selectedId = (frame.selectedId === undefined) ? -1 : frame.selectedId;
            perKnotColor = frame.perKnotColor !== false;
        } else if (frame && ArrayBuffer.isView(frame)) {
            centroids = frame; extents = null; count = (frame.length / 3) | 0;   // legacy particles
        } else {
            return si;
        }
        const rgb = [0, 0, 0];
        for (let pi = 0; pi < count; pi++) {
            if (si + 12 > maxSegments) break;
            const cx = centroids[pi * 3] + (extents ? 0 : 0.5);
            const cy = centroids[pi * 3 + 1] + (extents ? 0 : 0.5);
            const cz = centroids[pi * 3 + 2] + (extents ? 0 : 0.5);
            const hx = extents ? Math.max(1.0, extents[pi * 3]) : 1.5;
            const hy = extents ? Math.max(1.0, extents[pi * 3 + 1]) : 1.5;
            const hz = extents ? Math.max(1.0, extents[pi * 3 + 2]) : 1.5;
            // Per-knot color (field-aware: E vs B hues differ); selected → white;
            // legacy/colors-off → cyan.
            let r = 0.0, g = 0.8, b = 0.8;
            if (ids) {
                const id = ids[pi];
                if (id === selectedId) { r = 1.0; g = 1.0; b = 1.0; }
                else if (perKnotColor) { rampCyclicHSL(knotHue(id, field) * (Math.PI / 2), rgb, 0); r = rgb[0]; g = rgb[1]; b = rgb[2]; }
            }
            for (const e of edges) {
                const i = si * 6;
                posAttr.array[i] = cx - hx + e[0] * 2 * hx;
                posAttr.array[i + 1] = cy - hy + e[1] * 2 * hy;
                posAttr.array[i + 2] = cz - hz + e[2] * 2 * hz;
                posAttr.array[i + 3] = cx - hx + e[3] * 2 * hx;
                posAttr.array[i + 4] = cy - hy + e[4] * 2 * hy;
                posAttr.array[i + 5] = cz - hz + e[5] * 2 * hz;
                colAttr.array[i] = r; colAttr.array[i + 1] = g; colAttr.array[i + 2] = b;
                colAttr.array[i + 3] = r; colAttr.array[i + 4] = g; colAttr.array[i + 5] = b;
                si++;
            }
        }
        return si;
    },
    toggleKnotZones(on) {
        if (!this._knotZones) this._buildKnotZones();
        this._knotZones.visible = on;
        if (!on) this._knotZones.geometry.setDrawRange(0, 0);
    },

    // ── Genesis Threshold Isosurface (birth boundary) ────────────────
    _buildGenesisIsosurface() {
        const maxPts = 4000;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        _ensureManifestAttrs(geo, maxPts);
        geo.setDrawRange(0, 0);
        const mat = _makeParticleFragMaterial(
            { uOpacity: { value: 0.45 } },
            { blending: THREE.AdditiveBlending },
        );
        this._genesisIsosurface = new THREE.Points(geo, mat);
        this._genesisIsosurface.visible = false;
        this._genesisIsosurface.frustumCulled = false;
        this._genesisIsosurface.renderOrder = 1;
        this._scene.add(this._genesisIsosurface);
    },
    updateGenesisIsosurface(fluxMag, latticeSize, kGenesis) {
        this._syncCenterAndRadius();
        if (!this._genesisIsosurface) this._buildGenesisIsosurface();
        const posAttr = this._genesisIsosurface.geometry.getAttribute('position');
        const colAttr = this._genesisIsosurface.geometry.getAttribute('particleColor');
        const sizeAttr = this._genesisIsosurface.geometry.getAttribute('size');
        const N = latticeSize;
        const grid = resolveFluxMagnitudeGrid(fluxMag, N);
        if (!grid) {
            this._genesisIsosurface.geometry.setDrawRange(0, 0);
            return;
        }
        let vi = 0;
        const band = kGenesis * 0.15;

        const gridStep = grid.compact ? 1 : (N > 64 ? 4 : (N > 24 ? 2 : 1));
        const pointScale = grid.compact ? grid.stride : gridStep;
        const A = grid.axisCount;

        for (let gz = 0; gz < A && vi < 4000; gz += gridStep) {
            for (let gy = 0; gy < A && vi < 4000; gy += gridStep) {
                for (let gx = 0; gx < A && vi < 4000; gx += gridStep) {
                    const mag = grid.data[(gz * A + gy) * A + gx];
                    const dist = Math.abs(mag - kGenesis);
                    if (dist < band && mag > 0.01) {
                        const t = 1.0 - dist / band;
                        posAttr.array[vi * 3]     = Math.min(gx * grid.stride, N - 1) + 0.5;
                        posAttr.array[vi * 3 + 1] = Math.min(gy * grid.stride, N - 1) + 0.5;
                        posAttr.array[vi * 3 + 2] = Math.min(gz * grid.stride, N - 1) + 0.5;
                        colAttr.array[vi * 3] = 0.15 + t * 0.15;
                        colAttr.array[vi * 3 + 1] = 0.7 + t * 0.3;
                        colAttr.array[vi * 3 + 2] = 0.2 + t * 0.15;
                        sizeAttr.array[vi] = (1.5 + 4.0 * t) * pointScale;
                        vi++;
                    }
                }
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._genesisIsosurface.geometry.setDrawRange(0, vi);
    },
    toggleGenesisIsosurface(on) {
        if (!this._genesisIsosurface) this._buildGenesisIsosurface();
        this._genesisIsosurface.visible = on;
        if (!on) this._genesisIsosurface.geometry.setDrawRange(0, 0);
    },

    // ── Confinement Strings (SU(3) 1D topological defects) ───────────
    _buildConfinementStrings() {
        const maxVerts = 3000 * 2;
        const positions = new Float32Array(maxVerts * 3);
        const colors = new Float32Array(maxVerts * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.9,
            linewidth: 2, depthWrite: false, blending: THREE.AdditiveBlending
        });
        this._confinementStrings = new THREE.LineSegments(geo, mat);
        this._confinementStrings.visible = false;
        this._confinementStrings.frustumCulled = false;
        this._scene.add(this._confinementStrings);
    },
    updateConfinementStrings(bridge) {
        if (!this._confinementStrings) this._buildConfinementStrings();

        const posAttr = this._confinementStrings.geometry.getAttribute('position');
        const colAttr = this._confinementStrings.geometry.getAttribute('color');
        const maxVerts = posAttr.array.length / 3;

        let vi = 0;
        const J2_threshold_dist2 = CONFINEMENT_PAIR_DIST2;

        const ptData = bridge.getParticleData();
        if (!ptData || ptData.count < 2 || !ptData.positions) {
            this._confinementStrings.geometry.setDrawRange(0, 0);
            return;
        }
        const pos = ptData.positions;
        const count = ptData.count;

        // PERF (F-4): the pair test below was O(N²) — 40k pair-tests/frame at
        // N=200. Replace the all-pairs inner scan with a uniform spatial hash
        // so each particle only checks others within its 27-cell neighbourhood.
        // EXACTNESS: this is output-identical, not approximate. Cell size is
        // exactly √threshold, so every pair with r² < threshold necessarily
        // falls in the same or an adjacent cell — none are missed. For each i
        // (ascending, outer order unchanged) the surviving candidates are
        // SORTED ascending by j before emission, reproducing the original
        // (i asc, j asc) lexicographic order — which is what the `maxVerts`
        // truncation depends on, so the selected/written segment set is bit-
        // identical to the brute-force version.
        const cell = Math.sqrt(J2_threshold_dist2);
        const buckets = this._confBuckets || (this._confBuckets = new Map());
        buckets.clear();
        // Integer cell key: exact base-4096 packing with a +1024 bias so the
        // ±1 neighbour offsets stay non-negative. This is a bijection on cell
        // indices in [-1024, 3071] — far outside any lattice we run (native
        // caps at L=256) — so distinct cells still map to distinct buckets and
        // the bit-exactness guarantee above is preserved. Avoids building the
        // ~27·N short strings per frame the "cx,cy,cz" key used to churn.
        const keyOf = (cx, cy, cz) =>
            (cx + 1024) + (cy + 1024) * 4096 + (cz + 1024) * 16777216;
        for (let p = 0; p < count; p++) {
            const cx = Math.floor(pos[p * 3]     / cell);
            const cy = Math.floor(pos[p * 3 + 1] / cell);
            const cz = Math.floor(pos[p * 3 + 2] / cell);
            const k = keyOf(cx, cy, cz);
            let arr = buckets.get(k);
            if (arr === undefined) { arr = []; buckets.set(k, arr); }
            arr.push(p);
        }

        const cand = this._confCand || (this._confCand = []);
        outer:
        for (let i = 0; i < count; i++) {
            const xi = pos[i * 3], yi = pos[i * 3 + 1], zi = pos[i * 3 + 2];
            const cix = Math.floor(xi / cell);
            const ciy = Math.floor(yi / cell);
            const ciz = Math.floor(zi / cell);
            cand.length = 0;
            for (let ax = -1; ax <= 1; ax++)
            for (let ay = -1; ay <= 1; ay++)
            for (let az = -1; az <= 1; az++) {
                const arr = buckets.get(keyOf(cix + ax, ciy + ay, ciz + az));
                if (arr === undefined) continue;
                for (let n = 0; n < arr.length; n++) {
                    const j = arr[n];
                    if (j > i) cand.push(j);
                }
            }
            // Restore the original ascending-j emission order so the
            // maxVerts truncation picks exactly the same pairs.
            cand.sort((a, b) => a - b);
            for (let c = 0; c < cand.length; c++) {
                const j = cand[c];
                const dx = pos[j * 3]     - xi;
                const dy = pos[j * 3 + 1] - yi;
                const dz = pos[j * 3 + 2] - zi;
                const r2 = dx * dx + dy * dy + dz * dz;

                if (r2 > 1.0 && r2 < J2_threshold_dist2) {
                    const t = r2 / J2_threshold_dist2;
                    const alpha = 1.0 - t * 0.4;
                    const invR = 1.0 / Math.sqrt(r2);
                    const r = Math.abs(dx) * invR * alpha + 0.2;
                    const g = Math.abs(dy) * invR * alpha + 0.2;
                    const b = Math.abs(dz) * invR * alpha + 0.2;

                    if (vi + 2 > maxVerts) break outer;

                    posAttr.array[vi * 3]     = xi;
                    posAttr.array[vi * 3 + 1] = yi;
                    posAttr.array[vi * 3 + 2] = zi;
                    colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                    vi++;

                    posAttr.array[vi * 3]     = pos[j * 3];
                    posAttr.array[vi * 3 + 1] = pos[j * 3 + 1];
                    posAttr.array[vi * 3 + 2] = pos[j * 3 + 2];
                    colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                    vi++;
                }
            }
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._confinementStrings.geometry.setDrawRange(0, vi);
    },
    toggleConfinement(on) {
        if (!this._confinementStrings) this._buildConfinementStrings();
        this._confinementStrings.visible = on;
        if (!on) this._confinementStrings.geometry.setDrawRange(0, 0);
    }

};
