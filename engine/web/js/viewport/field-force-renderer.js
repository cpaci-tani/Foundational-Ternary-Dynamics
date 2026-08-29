/** fieldForceMethods — ViewportFieldRenderer mixin (EM/gravity/strong/weak force viz). */
import * as THREE from 'three';
import { FORCE_PALETTES, lerpPaletteInto } from './color-ramps.js';
import {
    VOXEL_CENTER_OFFSET,
    _softSpriteTexture,
} from './field-renderer-shared.js';

// Glyph meshes are heavier than point/line primitives. Deterministic
// qualifying-index decimation below preserves global lattice coverage while
// this fixed visual budget keeps the layer independent of quotient size.
const MAX_FORCE_GLYPHS = 128;

// Heatmap sprites are one draw call, but their translucent fragments overlap
// heavily around compact sources. A quotient-size-independent cap prevents
// fill-rate collapse while deterministic decimation retains coverage of the
// complete sampled lattice instead of rendering only its first linear block.
const MAX_FORCE_HEATMAP_POINTS = 64;

// The line style is the densest exact-direction view. Cap only its visual
// representatives; the sampler still evaluates the complete bounded field.
const MAX_GRAVITY_ARROWS = 256;

export const fieldForceMethods = {
    _buildForceVolume() {
        this._forceVolume = this._buildArrowFieldMesh(32768, 0.6);
    },
    updateForceVolume(fieldData) {
        if (!this._forceVolume) this._buildForceVolume();
        this._writeArrowFieldIntoMesh(this._forceVolume, fieldData,
            { base: [0.0, 0.9, 1.0], tip: [0.7, 1.0, 1.0] },
            '_magCache', 1.5, 0.03);
    },
    toggleForceVolume(on) {
        const next = !!on;
        if (!this._forceVolume) { if (!next) return; this._buildForceVolume(); }
        if (this._forceVolume.visible === next) return;
        this._forceVolume.visible = next;
        if (!next) this._forceVolume.geometry.setDrawRange(0, 0);
    },

    // ── Gravity Field Volume (density gradient vectors) ─────────────
    _buildGravityField() {
        const maxArrows = MAX_GRAVITY_ARROWS;
        const positions = new Float32Array(maxArrows * 2 * 3);
        const colors = new Float32Array(maxArrows * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.7,
            depthWrite: false
        });
        this._gravityField = new THREE.LineSegments(geo, mat);
        this._gravityField.visible = false;
        this._gravityField.frustumCulled = false;
        this._scene.add(this._gravityField);
    },
    updateGravityField(fieldData) {
        this._syncCenterAndRadius();
        if (!this._gravityField) this._buildGravityField();
        if (!fieldData?.count) {
            this._gravityField.geometry.setDrawRange(0, 0);
            this._gravityField.visible = false;
            return;
        }
        const _needsClip = this._clipActive();
        const posAttr = this._gravityField.geometry.getAttribute('position');
        const colAttr = this._gravityField.geometry.getAttribute('color');
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
        if (!(maxMag > 0)) {
            this._gravityField.geometry.setDrawRange(0, 0);
            this._gravityField.visible = false;
            return;
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
            const t = mag / maxMag;
            const scale = Math.log(1 + t) * arrowBase;
            const nx = vx / mag * scale, ny = vy / mag * scale, nz = vz / mag * scale;

            posAttr.array[vi * 6] = px; posAttr.array[vi * 6 + 1] = py; posAttr.array[vi * 6 + 2] = pz;
            colAttr.array[vi * 6] = 1.0;
            colAttr.array[vi * 6 + 1] = 0.67;
            colAttr.array[vi * 6 + 2] = 0.0;
            posAttr.array[vi * 6 + 3] = px + nx; posAttr.array[vi * 6 + 4] = py + ny; posAttr.array[vi * 6 + 5] = pz + nz;
            colAttr.array[vi * 6 + 3] = 1.0;
            colAttr.array[vi * 6 + 4] = 0.9;
            colAttr.array[vi * 6 + 5] = 0.4;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._gravityField.geometry.setDrawRange(0, vi * 2);
        this._gravityField.visible = !!this._gravityFieldRequested && vi > 0;
    },
    toggleGravityField(on) {
        const next = !!on;
        this._gravityFieldRequested = next;
        if (!this._gravityField) { if (!next) return; this._buildGravityField(); }
        if (!next) {
            this._gravityField.visible = false;
            this._gravityField.geometry.setDrawRange(0, 0);
            return;
        }
        this._gravityField.visible = this._gravityField.geometry.drawRange.count > 0;
    },

    // ── EM Force aliases ─────────────────────────────────────────────
    showEMForce(on) { this.toggleForceVolume(on); },
    showGravityForce(on) { this.toggleGravityField(on); },

    // ── Strong Force Volume (Red arrows) ──────────────────────────────
    _buildStrongForce() {
        this._strongForce = this._buildArrowFieldMesh(32768, 0.7);
    },
    toggleStrongForce(on) {
        const next = !!on;
        if (!this._strongForce) { if (!next) return; this._buildStrongForce(); }
        if (this._strongForce.visible === next) return;
        this._strongForce.visible = next;
        if (!next) this._strongForce.geometry.setDrawRange(0, 0);
    },
    showStrongForce(on) { this.toggleStrongForce(on); },

    // ── Weak Force Overlay (soft radial sprites at chirality sites) ───
    _buildWeakField() {
        const maxPts = 4000;
        const positions = new Float32Array(maxPts * 3);
        const colors    = new Float32Array(maxPts * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color',    new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);

        const mat = new THREE.PointsMaterial({
            size: 1.6,
            map: _softSpriteTexture(),
            alphaTest: 0.01,
            transparent: true,
            depthWrite: false,
            vertexColors: true,
            sizeAttenuation: true,
            blending: THREE.AdditiveBlending,
        });

        this._weakField = new THREE.Points(geo, mat);
        this._weakField.visible = false;
        this._weakField.frustumCulled = false;
        this._scene.add(this._weakField);
    },
    updateWeakField(fieldData) {
        this._syncCenterAndRadius();
        if (!this._weakField) this._buildWeakField();
        const posAttr = this._weakField.geometry.getAttribute('position');
        const colAttr = this._weakField.geometry.getAttribute('color');
        const { positions, values, count } = fieldData;
        const maxPts = posAttr.array.length / 3;

        let maxVal = 0;
        for (let i = 0; i < count; i++) {
            const v = Math.abs(values[i]);
            if (v > maxVal) maxVal = v;
        }
        if (maxVal <= 0) {
            this._weakField.geometry.setDrawRange(0, 0);
            return;
        }

        const pal = FORCE_PALETTES.weak;
        const threshold = maxVal * 0.08;
        const _needsClip = this._clipActive();
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const abs = Math.abs(values[i]);
            if (abs < threshold) continue;
            const px = positions[i * 3]     + VOXEL_CENTER_OFFSET;
            const py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET;
            const pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius,
                                      (py - this._center) / this._radius,
                                      (pz - this._center) / this._radius)) continue;

            const t   = Math.min(1, abs / maxVal);

            posAttr.array[vi * 3]     = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;
            lerpPaletteInto(pal, t, colAttr.array, vi * 3);
            vi++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._weakField.geometry.setDrawRange(0, vi);
    },
    toggleWeakField(on) {
        const next = !!on;
        if (!this._weakField) { if (!next) return; this._buildWeakField(); }
        if (this._weakField.visible === next) return;
        this._weakField.visible = next;
        if (!next) this._weakField.geometry.setDrawRange(0, 0);
    },
    showWeakField(on) { this.toggleWeakField(on); },

    // ══════════════════════════════════════════════════════════════════
    //  FORCE VISUALIZATION STYLES (Heatmap / Streamlines / Glyphs)
    // ══════════════════════════════════════════════════════════════════
    _buildForceHeatmap(forceType = 'em') {
        if (!this._forceHeatmaps) this._forceHeatmaps = Object.create(null);
        if (this._forceHeatmaps[forceType]) return this._forceHeatmaps[forceType];
        const maxPts = MAX_FORCE_HEATMAP_POINTS;
        const positions = new Float32Array(maxPts * 3);
        const colors    = new Float32Array(maxPts * 3);
        const sizes     = new Float32Array(maxPts);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        geo.setDrawRange(0, 0);

        const heatVert = `
            attribute float size;
            attribute vec3 particleColor;
            varying vec3 vColor;
            varying float vSize;
            void main() {
                vColor = particleColor;
                vSize = size;
                vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
                gl_PointSize = size * (150.0 / -mvPosition.z);
                gl_PointSize = clamp(gl_PointSize, 1.0, 512.0);
                gl_Position = projectionMatrix * mvPosition;
            }
        `;
        const heatFrag = `
            uniform float uOpacity;
            varying vec3 vColor;
            void main() {
                vec2 c = gl_PointCoord - vec2(0.5);
                float r2 = dot(c, c);
                if (r2 > 0.25) discard;
                float falloff = max(0.0, 1.0 - r2 * 4.0);
                float alpha = falloff * falloff;
                gl_FragColor = vec4(vColor * alpha, alpha * uOpacity);
            }
        `;
        const mat = new THREE.ShaderMaterial({
            vertexShader: heatVert,
            fragmentShader: heatFrag,
            uniforms: { uOpacity: { value: 0.8 } },
            transparent: true,
            blending: THREE.AdditiveBlending,
            depthWrite: false,
        });
        const points = new THREE.Points(geo, mat);
        points.visible = false;
        points.frustumCulled = false;
        points.renderOrder = 2;
        points.userData.forceType = forceType;
        this._scene.add(points);
        this._forceHeatmaps[forceType] = points;
        // Legacy introspection alias. Ownership/disposal lives in the typed map.
        if (!this._forceHeatmap) this._forceHeatmap = points;
        return points;
    },
    initForceHeatmap(forceType = 'em') { this._buildForceHeatmap(forceType); },
    updateForceHeatmap(fieldData, forceType) {
        this._syncCenterAndRadius();
        const type = forceType || 'em';
        const heatmap = this._buildForceHeatmap(type);
        if (!fieldData?.count) {
            heatmap.geometry.setDrawRange(0, 0);
            heatmap.visible = false;
            return;
        }
        const posAttr  = heatmap.geometry.getAttribute('position');
        const colAttr  = heatmap.geometry.getAttribute('particleColor');
        const sizeAttr = heatmap.geometry.getAttribute('size');
        const { positions, vectors, count } = fieldData;
        const maxPts = posAttr.array.length / 3;
        const pal = FORCE_PALETTES[type] || FORCE_PALETTES.em;

        const magKey = `_heatMagCache_${type}`;
        if (!this[magKey] || this[magKey].length < count) {
            this[magKey] = new Float32Array(count);
        }
        const mags = this[magKey];
        let maxMag = 0;
        for (let i = 0; i < count; i++) {
            const a = vectors[i * 3], b = vectors[i * 3 + 1], c = vectors[i * 3 + 2];
            mags[i] = Math.sqrt(a * a + b * b + c * c);
            if (mags[i] > maxMag) maxMag = mags[i];
        }
        if (!(maxMag > 0)) {
            heatmap.geometry.setDrawRange(0, 0);
            heatmap.visible = false;
            return;
        }
        const threshold = maxMag * 0.02;
        const _needsClip = this._clipActive();
        const sizeBase = 15 + 10 * (this._latticeSize / 64);
        let vi = 0;

        let qualifying = 0;
        for (let i = 0; i < count; i++) if (mags[i] >= threshold) qualifying++;
        const sampleStride = qualifying > maxPts
            ? Math.ceil(qualifying / maxPts)
            : 1;
        let qualifyingSeen = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            if ((qualifyingSeen++ % sampleStride) !== 0) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;

            const t = mag / maxMag;
            posAttr.array[vi * 3]     = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;
            lerpPaletteInto(pal, t, colAttr.array, vi * 3);
            sizeAttr.array[vi] = Math.log(1 + t * 9) / Math.log(10) * sizeBase;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        heatmap.geometry.setDrawRange(0, vi);
        heatmap.visible = !!this._forceHeatmapRequested?.[type] && vi > 0;
    },
    showForceHeatmap(visible) {
        if (!this._forceHeatmapRequested) {
            this._forceHeatmapRequested = { em: false, gravity: false, strong: false, weak: false };
        }
        const requested = this._forceHeatmapRequested;
        if (typeof visible === 'object' && visible !== null) {
            for (const type of Object.keys(requested)) requested[type] = !!visible[type];
        } else {
            const next = !!visible;
            for (const type of Object.keys(requested)) requested[type] = next;
        }
        if (!this._forceHeatmaps) return;
        for (const [type, points] of Object.entries(this._forceHeatmaps)) {
            const next = !!requested[type];
            points.visible = next && points.geometry.drawRange.count > 0;
            if (!next) points.geometry.setDrawRange(0, 0);
        }
    },

    // ── Animated Streamlines (Flow) ──────────────────────────────────
    _buildForceStreamlines(forceType = 'em') {
        if (!this._forceStreamlineMeshes) this._forceStreamlineMeshes = Object.create(null);
        if (this._forceStreamlineMeshes[forceType]) return this._forceStreamlineMeshes[forceType];
        const maxLines = 200;
        const maxSegs = 40;
        const maxVertices = maxLines * maxSegs * 2;
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(
            new Float32Array(maxVertices * 3), 3,
        ));
        geo.setAttribute('lineDistance', new THREE.Float32BufferAttribute(
            new Float32Array(maxVertices), 1,
        ));
        geo.setDrawRange(0, 0);
        const pal = FORCE_PALETTES[forceType] || FORCE_PALETTES.em;
        const mat = new THREE.LineDashedMaterial({
            color: new THREE.Color(pal.mid[0], pal.mid[1], pal.mid[2]),
            dashSize: 1.5,
            gapSize: 0.8,
            transparent: true,
            opacity: 0.7,
            depthWrite: false,
        });
        // One batched LineSegments draw replaces 200 individual Line draws.
        // lineDistance is written per segment below so dash phase restarts at
        // each discrete streamline without a computeLineDistances traversal.
        const mesh = new THREE.LineSegments(geo, mat);
        mesh.visible = false;
        mesh.frustumCulled = false;
        mesh.userData.forceType = forceType;
        this._scene.add(mesh);
        const entry = { mesh, material: mat, maxLines, maxSegs, vertexCount: 0 };
        this._forceStreamlineMeshes[forceType] = entry;
        // Legacy introspection alias; ownership/disposal lives in the typed map.
        if (!this._forceStreamlinePool) this._forceStreamlinePool = [mesh];
        return entry;
    },
    initForceStreamlines(forceType = 'em') { this._buildForceStreamlines(forceType); },
    updateForceStreamlines(lines, forceType) {
        this._syncCenterAndRadius();
        const type = forceType || 'em';
        const entry = this._buildForceStreamlines(type);
        const { mesh, maxLines, maxSegs } = entry;
        if (!lines?.count) {
            mesh.geometry.setDrawRange(0, 0);
            mesh.visible = false;
            entry.vertexCount = 0;
            return;
        }
        const posAttr = mesh.geometry.getAttribute('position');
        const distAttr = mesh.geometry.getAttribute('lineDistance');
        const out = posAttr.array;
        const distances = distAttr.array;
        const buffer = lines.buffer;
        const offsets = lines.offsets;
        const lengths = lines.lengths;
        const usedCount = Math.min(lines.count, maxLines);
        let vertex = 0;
        for (let li = 0; li < usedCount; li++) {
            const base = offsets[li];
            const points = Math.min(lengths[li] / 3, maxSegs + 1);
            let cumulative = 0;
            for (let p = 1; p < points; p++) {
                const a = base + (p - 1) * 3;
                const b = base + p * 3;
                const o0 = vertex * 3;
                const o1 = o0 + 3;
                const ax = buffer[a], ay = buffer[a + 1], az = buffer[a + 2];
                const bx = buffer[b], by = buffer[b + 1], bz = buffer[b + 2];
                out[o0] = ax; out[o0 + 1] = ay; out[o0 + 2] = az;
                out[o1] = bx; out[o1 + 1] = by; out[o1 + 2] = bz;
                const dx = bx - ax, dy = by - ay, dz = bz - az;
                const nextDistance = cumulative + Math.sqrt(dx * dx + dy * dy + dz * dz);
                distances[vertex] = cumulative;
                distances[vertex + 1] = nextDistance;
                cumulative = nextDistance;
                vertex += 2;
            }
        }
        posAttr.needsUpdate = true;
        distAttr.needsUpdate = true;
        mesh.geometry.setDrawRange(0, vertex);
        entry.vertexCount = vertex;
        mesh.visible = !!this._forceStreamlineRequested?.[type] && vertex > 0;
    },
    animateForceStreamlines(dt) {
        if (!this._forceStreamlineMeshes) return;
        const speed = 2.0;
        for (const entry of Object.values(this._forceStreamlineMeshes)) {
            if (entry.mesh.visible && entry.vertexCount > 0) {
                entry.material.dashOffset -= speed * dt;
            }
        }
    },
    showForceStreamlines_vis(visible) {
        if (!this._forceStreamlineRequested) {
            this._forceStreamlineRequested = { em: false, gravity: false, strong: false, weak: false };
        }
        const requested = this._forceStreamlineRequested;
        if (typeof visible === 'object' && visible !== null) {
            for (const type of Object.keys(requested)) requested[type] = !!visible[type];
        } else {
            const next = !!visible;
            for (const type of Object.keys(requested)) requested[type] = next;
        }
        if (!this._forceStreamlineMeshes) return;
        for (const [type, entry] of Object.entries(this._forceStreamlineMeshes)) {
            const next = !!requested[type];
            entry.mesh.visible = next && entry.vertexCount > 0;
            if (!next) {
                entry.mesh.geometry.setDrawRange(0, 0);
                entry.vertexCount = 0;
            }
        }
    },

    // ── Glyph Field (Instanced Cones) ────────────────────────────────
    _buildForceGlyphMesh(forceType) {
        const maxInstances = MAX_FORCE_GLYPHS;
        // Four open faces make the discrete direction marker legible without
        // the blended cap/overdraw cost of the previous six-sided cone.
        const coneGeo = new THREE.ConeGeometry(0.3, 1.0, 4, 1, true);
        coneGeo.rotateX(Math.PI / 2);
        const mat = new THREE.MeshBasicMaterial({
            transparent: false,
            opacity: 1,
            depthWrite: true,
        });
        const mesh = new THREE.InstancedMesh(coneGeo, mat, maxInstances);
        mesh.instanceMatrix.setUsage(THREE.DynamicDrawUsage);
        mesh.visible = false;
        mesh.frustumCulled = false;
        mesh.count = 0;
        mesh.instanceColor = new THREE.InstancedBufferAttribute(
            new Float32Array(maxInstances * 3), 3
        );
        mesh.instanceColor.setUsage(THREE.DynamicDrawUsage);
        mesh.userData.forceType = forceType;
        this._scene.add(mesh);
        return mesh;
    },
    _ensureForceGlyphInfra() {
        if (!this._forceGlyphMeshes) {
            this._forceGlyphMeshes = {
                em:      this._buildForceGlyphMesh('em'),
                gravity: this._buildForceGlyphMesh('gravity'),
                strong:  this._buildForceGlyphMesh('strong'),
                weak:    this._buildForceGlyphMesh('weak'),
            };
            this._glyphMatrix = new THREE.Matrix4();
            this._glyphQuat = new THREE.Quaternion();
            this._glyphUp = new THREE.Vector3(0, 0, 1);
            this._glyphDir = new THREE.Vector3();
            this._glyphScale = new THREE.Vector3();
        }
    },
    _buildForceGlyphs() { this._ensureForceGlyphInfra(); },
    initForceGlyphs() { this._ensureForceGlyphInfra(); },
    updateForceGlyphs(fieldData, forceType) {
        this._syncCenterAndRadius();
        this._ensureForceGlyphInfra();
        const mesh = this._forceGlyphMeshes[forceType] || this._forceGlyphMeshes.em;
        const { positions, vectors, count } = fieldData;
        const maxInstances = mesh.instanceMatrix.array.length / 16;
        const pal = FORCE_PALETTES[forceType] || FORCE_PALETTES.em;

        const magKey = `_glyphMagCache_${forceType}`;
        if (!this[magKey] || this[magKey].length < count) {
            this[magKey] = new Float32Array(count);
        }
        const mags = this[magKey];
        let maxMag = 0;
        for (let i = 0; i < count; i++) {
            const a = vectors[i * 3], b = vectors[i * 3 + 1], c = vectors[i * 3 + 2];
            mags[i] = Math.sqrt(a * a + b * b + c * c);
            if (mags[i] > maxMag) maxMag = mags[i];
        }
        if (maxMag < 1e-15) maxMag = 1;
        const threshold = maxMag * 0.03;
        const _needsClip = this._clipActive();
        const scaleBase = 0.8;
        let vi = 0;

        const mat4 = this._glyphMatrix;
        const quat = this._glyphQuat;
        const up = this._glyphUp;
        const dir = this._glyphDir;
        const scaleVec = this._glyphScale;
        const colorArr = mesh.instanceColor.array;

        let qualifying = 0;
        for (let i = 0; i < count; i++) if (mags[i] >= threshold) qualifying++;
        const sampleStride = qualifying > maxInstances
            ? Math.ceil(qualifying / maxInstances)
            : 1;
        let qualifyingSeen = 0;

        for (let i = 0; i < count && vi < maxInstances; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            if ((qualifyingSeen++ % sampleStride) !== 0) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;

            const t = mag / maxMag;
            const scale = Math.log(1 + t * 9) / Math.log(10) * scaleBase;

            dir.set(vectors[i * 3] / mag, vectors[i * 3 + 1] / mag, vectors[i * 3 + 2] / mag);
            quat.setFromUnitVectors(up, dir);

            scaleVec.set(scale, scale, scale * 1.5);
            mat4.makeRotationFromQuaternion(quat);
            mat4.scale(scaleVec);
            mat4.setPosition(px, py, pz);
            mesh.setMatrixAt(vi, mat4);

            lerpPaletteInto(pal, t, colorArr, vi * 3);
            vi++;
        }

        mesh.count = vi;
        mesh.instanceMatrix.needsUpdate = true;
        mesh.instanceColor.needsUpdate = true;
    },
    showForceGlyphs(visible) {
        if (!this._forceGlyphMeshes && !visible) return;
        this._ensureForceGlyphInfra();
        if (typeof visible === 'object' && visible !== null) {
            for (const type of Object.keys(this._forceGlyphMeshes)) {
                const mesh = this._forceGlyphMeshes[type];
                mesh.visible = !!visible[type];
                if (!mesh.visible) mesh.count = 0;
            }
        } else {
            for (const type of Object.keys(this._forceGlyphMeshes)) {
                const mesh = this._forceGlyphMeshes[type];
                mesh.visible = !!visible;
                if (!mesh.visible) mesh.count = 0;
            }
        }
    },
    clearForceVisualization(forceType, style) {
        const type = forceType || 'em';
        if (!style || style === 'arrows') {
            const mesh = type === 'em' ? this._forceVolume
                : type === 'gravity' ? this._gravityField
                    : type === 'strong' ? this._strongForce
                        : this._weakField;
            if (mesh?.geometry) mesh.geometry.setDrawRange(0, 0);
            if (mesh) mesh.visible = false;
        }
        if (!style || style === 'heatmap') {
            const mesh = this._forceHeatmaps?.[type];
            if (mesh) {
                mesh.geometry.setDrawRange(0, 0);
                mesh.visible = false;
            }
        }
        if (!style || style === 'flow') {
            const entry = this._forceStreamlineMeshes?.[type];
            if (entry) {
                entry.mesh.geometry.setDrawRange(0, 0);
                entry.mesh.visible = false;
                entry.vertexCount = 0;
            }
        }
        if (!style || style === 'glyphs') {
            const mesh = this._forceGlyphMeshes?.[type];
            if (mesh) {
                mesh.count = 0;
                mesh.visible = false;
            }
        }
    },
    hideAllForceStyles() {
        if (this._forceVolume) this._forceVolume.visible = false;
        if (this._gravityField) this._gravityField.visible = false;
        if (this._strongForce) this._strongForce.visible = false;
        if (this._weakField) this._weakField.visible = false;
        this.showForceHeatmap(false);
        this.showForceStreamlines_vis(false);
        this.showForceGlyphs(false);
    },
    showArrowForces(fieldState) {
        if (fieldState.showForceEM) this.toggleForceVolume(true);
        if (fieldState.showForceGravity) this.toggleGravityField(true);
        if (fieldState.showForceStrong) this.toggleStrongForce(true);
        if (fieldState.showForceWeak) this.toggleWeakField(true);
    }

};
