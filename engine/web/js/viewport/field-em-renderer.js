/** fieldEmMethods — ViewportFieldRenderer mixin (heatmap / EM / phase / state). */
import * as THREE from 'three';
import { fluxToColor, potentialToColorInto, magnitudeToColorInto } from '../fields.js';
import { buildStreamlineMesh, buildArrowFieldMesh } from './mesh-factory.js';
import { rampCyclicHSL } from './color-ramps.js';
import { knotHue } from '../scales/scale0/runtime/field-line-knots.js';
import { MAX_FIELD_GRID } from './constants.js';
import {
    VOXEL_CENTER_OFFSET,
    _makeParticleFragMaterial,
    _ensureManifestAttrs,
} from './field-renderer-shared.js';

export const fieldEmMethods = {
    _buildFieldHeatmap() {
        const positions = new Float32Array(MAX_FIELD_GRID * 3);
        const colors = new Float32Array(MAX_FIELD_GRID * 3);
        const sizes = new Float32Array(MAX_FIELD_GRID);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        _ensureManifestAttrs(geo, MAX_FIELD_GRID);
        geo.setDrawRange(0, 0);
        const mat = _makeParticleFragMaterial({ uOpacity: { value: 0.9 } });
        this._fieldHeatmap = new THREE.Points(geo, mat);
        this._fieldHeatmap.visible = false;
        this._fieldHeatmap.frustumCulled = false;
        this._fieldHeatmap.renderOrder = -1;
        this._scene.add(this._fieldHeatmap);
    },
    updateFieldHeatmap(gridPositions, potentials, count, maxAbsPotential) {
        this._syncCenterAndRadius();
        if (!this._fieldHeatmap) this._buildFieldHeatmap();
        const posAttr = this._fieldHeatmap.geometry.getAttribute('position');
        const colAttr = this._fieldHeatmap.geometry.getAttribute('particleColor');
        const sizeAttr = this._fieldHeatmap.geometry.getAttribute('size');
        const n = Math.min(count, MAX_FIELD_GRID);

        for (let i = 0; i < n; i++) {
            posAttr.array[i * 3] = gridPositions[i * 3];
            posAttr.array[i * 3 + 1] = gridPositions[i * 3 + 1] - 0.3;
            posAttr.array[i * 3 + 2] = gridPositions[i * 3 + 2];

            potentialToColorInto(colAttr.array, i * 3, potentials[i], maxAbsPotential);

            sizeAttr.array[i] = 10.0;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._fieldHeatmap.geometry.setDrawRange(0, n);
    },
    toggleFieldHeatmap(on) {
        if (!this._fieldHeatmap) this._buildFieldHeatmap();
        this._fieldHeatmap.visible = on;
        if (!on) this._fieldHeatmap.geometry.setDrawRange(0, 0);
    },

    // ── Flux Slice (dedicated _fluxSliceMesh, all-axis) ───────────────
    //
    // Renders the flux magnitude on one or more lattice mid-planes
    // (xy @ z=L/2, xz @ y=L/2, yz @ x=L/2) as a colored point cloud in a
    // dedicated THREE.Points mesh sized for all three planes (3·N²). The
    // Flux Volume appearance controls drive this via setFluxSlice*; per-axis
    // visibility lives in _fluxSliceAxes and is read by frame-sync.js, which
    // gathers the enabled planes and calls updateFluxSlices().
    _buildFluxSliceMesh(latticeSize) {
        const maxPts = 3 * latticeSize * latticeSize;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        _ensureManifestAttrs(geo, maxPts);
        geo.setDrawRange(0, 0);
        const mat = _makeParticleFragMaterial({ uOpacity: { value: 0.7 } });
        this._fluxSliceMesh = new THREE.Points(geo, mat);
        this._fluxSliceMesh.visible = this.showHeatmap;
        this._fluxSliceMesh.frustumCulled = false;
        this._fluxSliceMesh.renderOrder = -1;
        this._fluxSliceMeshSize = latticeSize;
        this._scene.add(this._fluxSliceMesh);
    }

    /**
     * Render one or more flux mid-planes into the dedicated slice mesh.
     * @param {{axis:0|1|2, data:Float64Array}[]} planes  one entry per enabled axis
     * @param {number} latticeSize  side length N
     * @param {number} index        slice plane index along each axis (the mid-plane)
     */,
    updateFluxSlices(planes, latticeSize, index) {
        this._syncCenterAndRadius();
        const N = latticeSize;
        if (!Number.isFinite(N) || N < 2) return;
        // (Re)build if missing or the lattice resized — capacity is 3·N².
        if (!this._fluxSliceMesh || this._fluxSliceMeshSize !== N) {
            if (this._fluxSliceMesh) {
                this._scene.remove(this._fluxSliceMesh);
                this._fluxSliceMesh.geometry.dispose();
                this._fluxSliceMesh.material.dispose();
                this._fluxSliceMesh = null;
            }
            this._buildFluxSliceMesh(N);
        }
        const posAttr = this._fluxSliceMesh.geometry.getAttribute('position');
        const colAttr = this._fluxSliceMesh.geometry.getAttribute('particleColor');
        const sizeAttr = this._fluxSliceMesh.geometry.getAttribute('size');
        const total = N * N;

        if (!planes || planes.length === 0) {
            this._fluxSliceMesh.geometry.setDrawRange(0, 0);
            return;
        }

        // Shared max across every supplied plane → consistent plane-to-plane color.
        let maxFlux = 0;
        for (const p of planes) {
            const d = p.data;
            if (!d || d.length !== total) continue;
            for (let i = 0; i < total; i++) if (d[i] > maxFlux) maxFlux = d[i];
        }
        if (maxFlux <= 0) {
            this._fluxSliceMesh.geometry.setDrawRange(0, 0);
            return;
        }

        const _needsClip = this._clipActive();
        const threshold = this._fluxSliceThreshold || 0;
        const pointScale = this._fluxSlicePointScale || 1.0;
        const posArr = posAttr.array;
        const colArr = colAttr.array;
        const sizeArr = sizeAttr.array;
        const maxPts = posArr.length / 3;
        const invMax = 1.0 / (maxFlux + 1e-20);
        let count = 0;

        for (const p of planes) {
            const data = p.data;
            const axis = p.axis;
            if (!data || data.length !== total) continue;
            for (let i = 0; i < total && count < maxPts; i++) {
                const mag = data[i];
                if (mag < threshold) continue;
                const a = (i / N) | 0;
                const b = i % N;
                let x, y, z;
                if (axis === 0) { x = index; y = a; z = b; }
                else if (axis === 1) { x = a; y = index; z = b; }
                else { x = a; y = b; z = index; }

                // Clip to boundary shape (skip entirely when the shape never clips).
                if (_needsClip) {
                    const nx = (x + 0.5 - this._center) / this._radius;
                    const ny = (y + 0.5 - this._center) / this._radius;
                    const nz = (z + 0.5 - this._center) / this._radius;
                    if (!this._insideBoundary(nx, ny, nz)) continue;
                }

                const c3 = count * 3;
                posArr[c3]     = x + 0.5;
                posArr[c3 + 1] = y + 0.5;
                posArr[c3 + 2] = z + 0.5;

                const [r, g, b2] = fluxToColor(mag, maxFlux);
                colArr[c3]     = r;
                colArr[c3 + 1] = g;
                colArr[c3 + 2] = b2;

                const t = mag * invMax;
                sizeArr[count] = (1.0 + 4.0 * t) * pointScale;
                count++;
            }
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._fluxSliceMesh.geometry.setDrawRange(0, count);
    }

    /** Back-compat single-plane entry (delegates to updateFluxSlices). */,
    updateFluxSlice(sliceData, latticeSize, axis, index) {
        this.updateFluxSlices([{ axis, data: sliceData }], latticeSize, index);
    },
    toggleFluxSlice(on) {
        if (!this._fluxSliceMesh) this._buildFluxSliceMesh(this._latticeSize);
        this._fluxSliceMesh.visible = on;
        this.showHeatmap = on;
        if (!on) this._fluxSliceMesh.geometry.setDrawRange(0, 0);
    },

    // Flux-slice appearance controls — wired in parallel with the Flux Volume
    // card (see wire.js::wireFluxVolume + ViewportFluxRenderer.setFlux*).
    setFluxSliceOpacity(val) {
        if (this._fluxSliceMesh) this._fluxSliceMesh.material.uniforms.uOpacity.value = val;
    },
    setFluxSliceShape(shapeIndex) {
        if (this._fluxSliceMesh) this._fluxSliceMesh.material.uniforms.shapeType.value = shapeIndex;
    },
    setFluxSlicePointScale(scale) {
        this._fluxSlicePointScale = scale;   // applied on the next updateFluxSlices
    },
    setFluxSliceThreshold(val) {
        this._fluxSliceThreshold = val;       // applied on the next updateFluxSlices
    },

    // Per-axis visibility (axis index 0=yz, 1=xz, 2=xy). frame-sync reads the
    // enabled set each upload tick to decide which planes to gather + pack.
    setFluxSliceAxisEnabled(axis, on) {
        if (axis in this._fluxSliceAxes) this._fluxSliceAxes[axis] = !!on;
    },
    getEnabledFluxSliceAxes() {
        const out = [];
        for (const k of [0, 1, 2]) if (this._fluxSliceAxes[k]) out.push(k);
        return out;
    },

    // ── Field Vectors (force arrows on XZ plane) ─────────────────────
    _buildFieldVectors() {
        const vertices = new Float32Array(MAX_FIELD_GRID * 2 * 3);
        const colors = new Float32Array(MAX_FIELD_GRID * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.75,
        });
        this._fieldVectors = new THREE.LineSegments(geo, mat);
        this._fieldVectors.frustumCulled = false;
        this._fieldVectors.visible = false;
        this._scene.add(this._fieldVectors);
    },
    updateFieldVectors(gridPositions, forces, count, maxForce, arrowScale = 8.0) {
        this._syncCenterAndRadius();
        if (!this._fieldVectors) this._buildFieldVectors();
        const posAttr = this._fieldVectors.geometry.getAttribute('position');
        const colAttr = this._fieldVectors.geometry.getAttribute('color');
        const n = Math.min(count, MAX_FIELD_GRID);

        for (let i = 0; i < n; i++) {
            // +VOXEL_CENTER_OFFSET so arrows align with particles + flux volume.
            const gx = gridPositions[i * 3]     + VOXEL_CENTER_OFFSET;
            const gy = gridPositions[i * 3 + 1] + VOXEL_CENTER_OFFSET;
            const gz = gridPositions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            const fx = forces[i * 3], fy = forces[i * 3 + 1], fz = forces[i * 3 + 2];
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);

            posAttr.array[i * 6] = gx;
            posAttr.array[i * 6 + 1] = gy;
            posAttr.array[i * 6 + 2] = gz;

            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce + 1e-20) * 10) : 0;
            posAttr.array[i * 6 + 3] = gx + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[i * 6 + 4] = gy + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[i * 6 + 5] = gz + (mag > 1e-20 ? fz / mag * scale : 0);

            magnitudeToColorInto(colAttr.array, i * 6 + 3, mag, maxForce);
            colAttr.array[i * 6] = colAttr.array[i * 6 + 3] * 0.5;
            colAttr.array[i * 6 + 1] = colAttr.array[i * 6 + 4] * 0.5;
            colAttr.array[i * 6 + 2] = colAttr.array[i * 6 + 5] * 0.5;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._fieldVectors.geometry.setDrawRange(0, n * 2);
    },
    toggleFieldVectors(on) {
        if (!this._fieldVectors) this._buildFieldVectors();
        this._fieldVectors.visible = on;
        if (!on) this._fieldVectors.geometry.setDrawRange(0, 0);
    },

    // ── PE E-Field Streamlines (3D Coulomb field lines) ────────────────
    _buildPEStreamlines() {
        const MAX_VERTS = 20000;
        const vertices = new Float32Array(MAX_VERTS * 3);
        const colors = new Float32Array(MAX_VERTS * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.7,
            blending: THREE.AdditiveBlending, depthWrite: false,
        });
        this._peStreamlines = new THREE.LineSegments(geo, mat);
        this._peStreamlines.frustumCulled = false;
        this._peStreamlines.visible = false;
        this._scene.add(this._peStreamlines);
    },

    // `lines` is the POOLED StreamlineResult from computeStreamlines
    // ({count, buffer, offsets, lengths}). Iterate [0,count) and read the float
    // run buffer[offsets[li] .. offsets[li]+lengths[li]); `base + j` indexes the
    // same floats the old `line[j]` did, so output is byte-identical.
    updatePEStreamlines(lines) {
        this._syncCenterAndRadius();
        if (!this._peStreamlines) this._buildPEStreamlines();
        const posAttr = this._peStreamlines.geometry.getAttribute('position');
        const colAttr = this._peStreamlines.geometry.getAttribute('color');
        const maxVerts = posAttr.count;
        const lineCount = lines.count;
        const buffer = lines.buffer;
        const offsets = lines.offsets;
        const lengths = lines.lengths;
        let vi = 0;

        for (let li = 0; li < lineCount; li++) {
            const base = offsets[li];
            const nPts = lengths[li] / 3;
            for (let i = 0; i < nPts - 1 && vi + 2 <= maxVerts; i++) {
                // +VOXEL_CENTER_OFFSET — see header convention note.
                posAttr.array[vi * 3]     = buffer[base + i * 3]         + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 1] = buffer[base + i * 3 + 1]     + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 2] = buffer[base + i * 3 + 2]     + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 3] = buffer[base + (i + 1) * 3]     + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 4] = buffer[base + (i + 1) * 3 + 1] + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 5] = buffer[base + (i + 1) * 3 + 2] + VOXEL_CENTER_OFFSET;

                const t = i / Math.max(1, nPts - 2);
                const bright = 1.0 - t * 0.6;
                colAttr.array[vi * 3] = 0.26 * bright;
                colAttr.array[vi * 3 + 1] = 0.65 * bright;
                colAttr.array[vi * 3 + 2] = 0.97 * bright;
                colAttr.array[vi * 3 + 3] = 0.26 * bright * 0.8;
                colAttr.array[vi * 3 + 4] = 0.65 * bright * 0.8;
                colAttr.array[vi * 3 + 5] = 0.97 * bright * 0.8;
                vi += 2;
            }
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._peStreamlines.geometry.setDrawRange(0, vi);
    },
    togglePEStreamlines(on) {
        if (!this._peStreamlines) this._buildPEStreamlines();
        this._peStreamlines.visible = on;
        if (!on) this._peStreamlines.geometry.setDrawRange(0, 0);
    },

    // ── Gravity Field Vectors (XZ plane) ──────────────────────────────
    _buildGravityVectors() {
        this._gravityVectors = this._buildArrowFieldMesh(MAX_FIELD_GRID, 0.65);
    },
    updateGravityVectors(gridPositions, forces, count, maxForce, arrowScale = 8.0) {
        this._syncCenterAndRadius();
        if (!this._gravityVectors) this._buildGravityVectors();
        const posAttr = this._gravityVectors.geometry.getAttribute('position');
        const colAttr = this._gravityVectors.geometry.getAttribute('color');
        const n = Math.min(count, MAX_FIELD_GRID);

        for (let i = 0; i < n; i++) {
            // +VOXEL_CENTER_OFFSET so arrows align with particles + flux volume.
            const gx = gridPositions[i * 3]     + VOXEL_CENTER_OFFSET;
            const gy = gridPositions[i * 3 + 1] + VOXEL_CENTER_OFFSET;
            const gz = gridPositions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            const fx = forces[i * 3], fy = forces[i * 3 + 1], fz = forces[i * 3 + 2];
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);

            posAttr.array[i * 6] = gx;
            posAttr.array[i * 6 + 1] = gy;
            posAttr.array[i * 6 + 2] = gz;

            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce + 1e-20) * 10) : 0;
            posAttr.array[i * 6 + 3] = gx + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[i * 6 + 4] = gy + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[i * 6 + 5] = gz + (mag > 1e-20 ? fz / mag * scale : 0);

            const t = mag / (maxForce + 1e-20);
            const c = 0.3 + 0.5 * t;
            colAttr.array[i * 6] = c * 0.5;
            colAttr.array[i * 6 + 1] = c * 0.55;
            colAttr.array[i * 6 + 2] = c * 0.6;
            colAttr.array[i * 6 + 3] = c;
            colAttr.array[i * 6 + 4] = c * 1.05;
            colAttr.array[i * 6 + 5] = c * 1.1;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._gravityVectors.geometry.setDrawRange(0, n * 2);
    },
    toggleGravityVectors(on) {
        if (!this._gravityVectors) this._buildGravityVectors();
        this._gravityVectors.visible = on;
        if (!on) this._gravityVectors.geometry.setDrawRange(0, 0);
    },

    // ── Shared mesh-factory helpers (also exposed as public callbacks for
    //    FluxRenderer + ParticleRenderer via Viewport orchestrator) ────
    _buildStreamlineMesh(maxVerts, opacity = 0.7) {
        return buildStreamlineMesh(this._scene, maxVerts, opacity);
    },
    _buildArrowFieldMesh(maxArrows, opacity = 0.7) {
        return buildArrowFieldMesh(this._scene, maxArrows, opacity);
    },
    _writeArrowFieldIntoMesh(mesh, fieldData, colors, magCacheKey, arrowBase = 1.5, thresholdFrac = 0.03) {
        this._syncCenterAndRadius();
        const posAttr = mesh.geometry.getAttribute('position');
        const colAttr = mesh.geometry.getAttribute('color');
        const { positions, vectors, count } = fieldData;
        const maxArrows = posAttr.array.length / 6;
        let maxMag = 0;
        if (!this[magCacheKey] || this[magCacheKey].length < count) {
            this[magCacheKey] = new Float32Array(count);
        }
        const mags = this[magCacheKey];
        for (let i = 0; i < count; i++) {
            const a = vectors[i * 3], b = vectors[i * 3 + 1], c = vectors[i * 3 + 2];
            const m = Math.sqrt(a * a + b * b + c * c);
            mags[i] = m;
            if (m > maxMag) maxMag = m;
        }
        const threshold = maxMag * thresholdFrac;
        const _needsClip = this._clipActive();
        const [br, bg, bb] = colors.base;
        const [tr, tg, tb] = colors.tip;

        // Gather all active indices that pass threshold and boundary checks
        // (F-16: into a reused Int32Array, ascending order preserved).
        const activeIndices = this._ensureActiveIdx(count);
        let activeCount = 0;
        for (let i = 0; i < count; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            activeIndices[activeCount++] = i;
        }

        const step = activeCount > maxArrows ? Math.ceil(activeCount / maxArrows) : 1;

        let vi = 0;
        for (let k = 0; k < activeCount && vi < maxArrows; k += step) {
            const i = activeIndices[k];
            const mag = mags[i];
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            const scale = Math.log(1 + mag / maxMag) * arrowBase;
            const vx = vectors[i * 3], vy = vectors[i * 3 + 1], vz = vectors[i * 3 + 2];
            const nx = vx / mag * scale, ny = vy / mag * scale, nz = vz / mag * scale;
            // px/py/pz already include VOXEL_CENTER_OFFSET (line above) so arrows
            // root at voxel centres where particles render.
            posAttr.array[vi * 6]     = px;      posAttr.array[vi * 6 + 1] = py;      posAttr.array[vi * 6 + 2] = pz;
            colAttr.array[vi * 6]     = br;      colAttr.array[vi * 6 + 1] = bg;      colAttr.array[vi * 6 + 2] = bb;
            posAttr.array[vi * 6 + 3] = px + nx; posAttr.array[vi * 6 + 4] = py + ny; posAttr.array[vi * 6 + 5] = pz + nz;
            colAttr.array[vi * 6 + 3] = tr;      colAttr.array[vi * 6 + 4] = tg;      colAttr.array[vi * 6 + 5] = tb;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        mesh.geometry.setDrawRange(0, vi * 2);
    },

    // `streamlines` is the POOLED StreamlineResult from computeStreamlines
    // ({count, buffer, offsets, lengths}) — fieldlines.js (web/engine-
    // optimization-2026-05-31). Line li is the float run
    // buffer[offsets[li] .. offsets[li]+lengths[li]); iterate [0,count) and read
    // lengths[li] (the flat buffer is grown to a high-water mark, so its own
    // .length is over-long). The inner per-segment math is unchanged — `base + j`
    // indexes the same floats the old per-line `line[j]` did, so output is
    // byte-identical.
    _writeStreamlinesIntoMesh(mesh, streamlines, colorFn) {
        this._syncCenterAndRadius();
        const posAttr = mesh.geometry.getAttribute('position');
        const colAttr = mesh.geometry.getAttribute('color');
        const maxVerts = posAttr.array.length / 3;
        const _needsClip = this._clipActive();
        const rgb = [0, 0, 0];
        const lineCount = streamlines.count;
        const buffer = streamlines.buffer;
        const offsets = streamlines.offsets;
        const lengths = streamlines.lengths;
        let vi = 0;
        for (let li = 0; li < lineCount; li++) {
            const base = offsets[li];
            const nPts = lengths[li] / 3;
            for (let i = 0; i < nPts - 1 && vi + 2 <= maxVerts; i++) {
                const sx = buffer[base + i * 3], sy = buffer[base + i * 3 + 1], sz = buffer[base + i * 3 + 2];
                const px = sx + VOXEL_CENTER_OFFSET;
                const py = sy + VOXEL_CENTER_OFFSET;
                const pz = sz + VOXEL_CENTER_OFFSET;
                if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
                colorFn(i, nPts, rgb, li);
                // +VOXEL_CENTER_OFFSET so the line aligns with particles + flux volume.
                posAttr.array[vi * 3]     = px;
                posAttr.array[vi * 3 + 1] = py;
                posAttr.array[vi * 3 + 2] = pz;
                colAttr.array[vi * 3]     = rgb[0];
                colAttr.array[vi * 3 + 1] = rgb[1];
                colAttr.array[vi * 3 + 2] = rgb[2];
                vi++;
                posAttr.array[vi * 3]     = buffer[base + (i + 1) * 3]     + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 1] = buffer[base + (i + 1) * 3 + 1] + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 2] = buffer[base + (i + 1) * 3 + 2] + VOXEL_CENTER_OFFSET;
                colAttr.array[vi * 3]     = rgb[0];
                colAttr.array[vi * 3 + 1] = rgb[1];
                colAttr.array[vi * 3 + 2] = rgb[2];
                vi++;
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        mesh.geometry.setDrawRange(0, vi);
    },

    // Public mesh-factory callbacks (FluxRenderer / ParticleRenderer call these
    // via constructor-injected callbacks routed through the orchestrator).
    buildStreamlineMesh(maxVerts, opacity = 0.7) {
        return this._buildStreamlineMesh(maxVerts, opacity);
    },
    buildArrowFieldMesh(maxArrows, opacity = 0.7) {
        return this._buildArrowFieldMesh(maxArrows, opacity);
    },
    writeArrowFieldIntoMesh(mesh, fieldData, colors, magCacheKey, arrowBase, thresholdFrac) {
        return this._writeArrowFieldIntoMesh(mesh, fieldData, colors, magCacheKey, arrowBase, thresholdFrac);
    },
    writeStreamlinesIntoMesh(mesh, streamlines, colorFn) {
        return this._writeStreamlinesIntoMesh(mesh, streamlines, colorFn);
    },

    // ── E-Field Lines (Cyan) ─────────────────────────────────────────
    _buildEFieldLines() {
        this._eFieldLines = this._buildStreamlineMesh(300 * 160 * 2, 0.7);
    },
    updateEFieldLines(streamlines, knotColoring) {
        if (!this._eFieldLines) this._buildEFieldLines();
        // knotColoring = { lineIds:Int32Array, selectedId:int, perKnotColor:bool } |
        // null. When present, each flowline is tinted with the color of the knot it
        // belongs to (white for the selected knot), matching the panel rows + boxes.
        // The alpha fade along the line is preserved. Lines with no knot (id<0) and
        // the default (no coloring / perKnotColor off) keep the cyan fade.
        const kc = knotColoring;
        this._writeStreamlinesIntoMesh(this._eFieldLines, streamlines, (i, nPts, rgb, li) => {
            const alpha = 1.0 - (i / (nPts - 1)) * 0.7;
            if (kc && kc.perKnotColor && kc.lineIds && kc.lineIds[li] >= 0) {
                const id = kc.lineIds[li];
                if (id === kc.selectedId) { rgb[0] = alpha; rgb[1] = alpha; rgb[2] = alpha; return; } // white
                rampCyclicHSL(knotHue(id) * (Math.PI / 2), rgb, 0);
                rgb[0] *= alpha; rgb[1] *= alpha; rgb[2] *= alpha;
                return;
            }
            rgb[0] = 0.3 * alpha; rgb[1] = 0.82 * alpha; rgb[2] = 0.88 * alpha;
        });
    },
    toggleEFieldLines(on) {
        if (!this._eFieldLines) this._buildEFieldLines();
        this._eFieldLines.visible = on;
        if (!on) this._eFieldLines.geometry.setDrawRange(0, 0);
    },

    // ── B-Field Lines (Green) ────────────────────────────────────────
    _buildBFieldLines() {
        this._bFieldLines = this._buildStreamlineMesh(300 * 240 * 2, 0.7);
    },
    updateBFieldLines(streamlines, knotColoring) {
        if (!this._bFieldLines) this._buildBFieldLines();
        // Like updateEFieldLines but for the orthogonal B field: when per-knot
        // colors is on, tint each loop with its B-knot hue (field 'b' → half-turn
        // from E); selected B-knot → white; else the default green fade.
        const kc = knotColoring;
        this._writeStreamlinesIntoMesh(this._bFieldLines, streamlines, (i, nPts, rgb, li) => {
            const alpha = 1.0 - (i / (nPts - 1)) * 0.5;
            if (kc && kc.perKnotColor && kc.lineIds && kc.lineIds[li] >= 0) {
                const id = kc.lineIds[li];
                if (id === kc.selectedId) { rgb[0] = alpha; rgb[1] = alpha; rgb[2] = alpha; return; }
                rampCyclicHSL(knotHue(id, 'b') * (Math.PI / 2), rgb, 0);
                rgb[0] *= alpha; rgb[1] *= alpha; rgb[2] *= alpha;
                return;
            }
            rgb[0] = 0.4 * alpha; rgb[1] = 0.73 * alpha; rgb[2] = 0.42 * alpha;
        });
    },
    toggleBFieldLines(on) {
        if (!this._bFieldLines) this._buildBFieldLines();
        this._bFieldLines.visible = on;
        if (!on) this._bFieldLines.geometry.setDrawRange(0, 0);
    },

    // ── Poynting Vectors (Yellow-Orange arrows) ──────────────────────
    _buildPoyntingVectors() {
        const maxArrows = 32768;
        const positions = new Float32Array(maxArrows * 2 * 3);
        const colors = new Float32Array(maxArrows * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.8,
            blending: THREE.AdditiveBlending, depthWrite: false
        });
        this._poyntingVectors = new THREE.LineSegments(geo, mat);
        this._poyntingVectors.visible = false;
        this._poyntingVectors.frustumCulled = false;
        this._scene.add(this._poyntingVectors);
    },
    updatePoyntingVectors(fieldData) {
        this._syncCenterAndRadius();
        if (!this._poyntingVectors) this._buildPoyntingVectors();
        const _needsClip = this._clipActive();
        const posAttr = this._poyntingVectors.geometry.getAttribute('position');
        const colAttr = this._poyntingVectors.geometry.getAttribute('color');
        const { positions, vectors, count } = fieldData;
        const maxArrows = posAttr.array.length / 6;
        let maxMag = 0;
        if (!this._magCache || this._magCache.length < count) this._magCache = new Float32Array(count);
        const mags = this._magCache;
        for (let i = 0; i < count; i++) {
            const a = vectors[i * 3], b = vectors[i * 3 + 1], c = vectors[i * 3 + 2];
            const m = Math.sqrt(a * a + b * b + c * c);
            mags[i] = m;
            if (m > maxMag) maxMag = m;
        }
        const threshold = maxMag * 0.05;
        const arrowBase = 2.0;

        // Gather all active indices (F-16: reused Int32Array, ascending order).
        const activeIndices = this._ensureActiveIdx(count);
        let activeCount = 0;
        for (let i = 0; i < count; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            activeIndices[activeCount++] = i;
        }

        const step = activeCount > maxArrows ? Math.ceil(activeCount / maxArrows) : 1;

        let vi = 0;
        for (let k = 0; k < activeCount && vi < maxArrows; k += step) {
            const i = activeIndices[k];
            const mag = mags[i];
            const vx = vectors[i * 3], vy = vectors[i * 3 + 1], vz = vectors[i * 3 + 2];
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            const scale = Math.log(1 + mag / maxMag) * arrowBase;
            const nx = vx / mag * scale, ny = vy / mag * scale, nz = vz / mag * scale;

            posAttr.array[vi * 6] = px; posAttr.array[vi * 6 + 1] = py; posAttr.array[vi * 6 + 2] = pz;
            colAttr.array[vi * 6] = 0.8; colAttr.array[vi * 6 + 1] = 0.55; colAttr.array[vi * 6 + 2] = 0.15;
            posAttr.array[vi * 6 + 3] = px + nx; posAttr.array[vi * 6 + 4] = py + ny; posAttr.array[vi * 6 + 5] = pz + nz;
            colAttr.array[vi * 6 + 3] = 1.0; colAttr.array[vi * 6 + 4] = 0.85; colAttr.array[vi * 6 + 5] = 0.15;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._poyntingVectors.geometry.setDrawRange(0, vi * 2);
    },
    togglePoyntingVectors(on) {
        if (!this._poyntingVectors) this._buildPoyntingVectors();
        this._poyntingVectors.visible = on;
        if (!on) this._poyntingVectors.geometry.setDrawRange(0, 0);
    },

    // ── Divergence Field (Red-Blue dots) ─────────────────────────────
    _buildDivergenceField() {
        const maxPts = 16384;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3)); // changed attribute to color for standard mesh, wait!
        // No, original was particleColor! Wait, let's keep exact attributes to not break shader material!
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        _ensureManifestAttrs(geo, maxPts);
        geo.setDrawRange(0, 0);
        const mat = _makeParticleFragMaterial(
            { uOpacity: { value: 0.8 } },
            { blending: THREE.AdditiveBlending },
        );
        this._divField = new THREE.Points(geo, mat);
        this._divField.visible = false;
        this._divField.frustumCulled = false;
        this._scene.add(this._divField);
    },
    updateDivergenceField(fieldData) {
        this._syncCenterAndRadius();
        if (!this._divField) this._buildDivergenceField();
        const _needsClip = this._clipActive();
        const posAttr = this._divField.geometry.getAttribute('position');
        const colAttr = this._divField.geometry.getAttribute('particleColor');
        const sizeAttr = this._divField.geometry.getAttribute('size');
        const { positions, values, count } = fieldData;
        const maxPts = posAttr.array.length / 3;
        let maxVal = 0;
        for (let i = 0; i < count; i++) {
            const a = Math.abs(values[i]);
            if (a > maxVal) maxVal = a;
        }
        const threshold = maxVal * 0.01;

        // Gather active indices (F-16: reused Int32Array, ascending order).
        const activeIndices = this._ensureActiveIdx(count);
        let activeCount = 0;
        for (let i = 0; i < count; i++) {
            const v = values[i];
            if (Math.abs(v) < threshold) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            activeIndices[activeCount++] = i;
        }

        const step = activeCount > maxPts ? Math.ceil(activeCount / maxPts) : 1;

        let vi = 0;
        for (let k = 0; k < activeCount && vi < maxPts; k += step) {
            const i = activeIndices[k];
            const v = values[i];
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;

            posAttr.array[vi * 3] = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;

            const t = Math.abs(v) / maxVal;
            if (v > 0) {
                colAttr.array[vi * 3] = 0.9; colAttr.array[vi * 3 + 1] = 0.2; colAttr.array[vi * 3 + 2] = 0.15;
            } else {
                colAttr.array[vi * 3] = 0.15; colAttr.array[vi * 3 + 1] = 0.3; colAttr.array[vi * 3 + 2] = 0.9;
            }
            sizeAttr.array[vi] = 1.0 + 2.0 * t;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._divField.geometry.setDrawRange(0, vi);
    },
    toggleDivergenceField(on) {
        if (!this._divField) this._buildDivergenceField();
        this._divField.visible = on;
        if (!on) this._divField.geometry.setDrawRange(0, 0);
    },

    // ── EM Force Volume (Cyan arrows) ────────────────────────────────
    updateEMForceField(data) { this.updateForceVolume(data); },
    updateGravityForceField(data) { this.updateGravityField(data); },
    updateStrongForceField(fieldData) {
        if (!this._strongForce) this._buildStrongForce();
        this._writeArrowFieldIntoMesh(this._strongForce, fieldData,
            { base: [1.0, 0.09, 0.27], tip: [1.0, 0.5, 0.5] },
            '_strongMagCache', 1.5, 0.03);
    },
    togglePhaseField(on) {
        this._phaseVisible = !!on;
        if (!this._phaseNeedles) this._buildPhaseNeedles();
        this._phaseNeedles.visible = !!on;
        if (!on) this._phaseNeedles.geometry.setDrawRange(0, 0);
        this._quantumSetVisibility();
    },
    updatePhaseField(data) {
        this._syncCenterAndRadius();
        this._phaseData = data;
        if (!this._phaseVisible || !data?.count) return;
        if (!this._phaseNeedles) this._buildPhaseNeedles();
        const posAttr = this._phaseNeedles.geometry.getAttribute('position');
        const colAttr = this._phaseNeedles.geometry.getAttribute('color');
        const maxSegments = posAttr.array.length / 6;
        const _needsClip = this._clipActive();
        const len = 1.2;
        const { positions, values, count } = data;
        const rgb = new Float32Array(3);
        let si = 0;
        for (let i = 0; i < count && si < maxSegments; i++) {
            const px = positions[i * 3]     + VOXEL_CENTER_OFFSET;
            const py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET;
            const pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            const phase = values[i];
            if (Math.abs(phase) < 0.02) continue;
            const dx = Math.cos(phase) * len;
            const dz = Math.sin(phase) * len;
            const base = si * 6;
            posAttr.array[base]     = px - dx;
            posAttr.array[base + 1] = py;
            posAttr.array[base + 2] = pz - dz;
            posAttr.array[base + 3] = px + dx;
            posAttr.array[base + 4] = py;
            posAttr.array[base + 5] = pz + dz;
            rampCyclicHSL(phase, rgb, 0);
            colAttr.array[base]     = rgb[0]; colAttr.array[base + 1] = rgb[1]; colAttr.array[base + 2] = rgb[2];
            colAttr.array[base + 3] = rgb[0]; colAttr.array[base + 4] = rgb[1]; colAttr.array[base + 5] = rgb[2];
            si++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._phaseNeedles.geometry.setDrawRange(0, si * 2);
    },

    // ── ℒ(x) Lagrangian density ──────────────────────────────────────
    _buildStateField() {
        const max = 16384;
        const geo = new THREE.BufferGeometry();
        const pos = new Float32Array(max * 3);
        const col = new Float32Array(max * 3);
        geo.setAttribute('position', new THREE.BufferAttribute(pos, 3));
        geo.setAttribute('color', new THREE.BufferAttribute(col, 3));
        geo.setDrawRange(0, 0);
        const tex = this._buildSoftDiscTexture();
        const mat = new THREE.PointsMaterial({
            map: tex, alphaMap: tex, size: 3.4, vertexColors: true,
            transparent: true, opacity: 0.95, depthWrite: false,
            sizeAttenuation: true, blending: THREE.AdditiveBlending,
        });
        const points = new THREE.Points(geo, mat);
        points.visible = false;
        points.renderOrder = 4;
        points.frustumCulled = false;
        this._scene.add(points);
        this._stateField = { points, geo, capacity: max };
    },
    toggleStateField(on) {
        if (!this._stateField) this._buildStateField();
        this._stateField.points.visible = !!on;
        if (!on) this._stateField.geo.setDrawRange(0, 0);
    },
    updateStateField(data) {
        this._syncCenterAndRadius();
        if (!this._stateField) this._buildStateField();
        const sf = this._stateField;
        if (!sf.points.visible) return;
        if (!data || !data.count) { sf.geo.setDrawRange(0, 0); return; }
        const posAttr = sf.geo.attributes.position;
        const colAttr = sf.geo.attributes.color;
        const cap = sf.capacity;
        const _needsClip = this._clipActive();
        const { positions, values, count } = data;
        let vi = 0;
        for (let i = 0; i < count && vi < cap; i++) {
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET;
            const py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET;
            const pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            posAttr.array[vi * 3] = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;
            if (values[i] < 0) { // s = -1 → cool blue
                colAttr.array[vi * 3] = 0.25; colAttr.array[vi * 3 + 1] = 0.45; colAttr.array[vi * 3 + 2] = 1.0;
            } else {             // s = +1 → warm red
                colAttr.array[vi * 3] = 1.0;  colAttr.array[vi * 3 + 1] = 0.35; colAttr.array[vi * 3 + 2] = 0.22;
            }
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sf.geo.setDrawRange(0, vi);
    }

};