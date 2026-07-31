/** fieldForceMethods — ViewportFieldRenderer mixin (EM/gravity/strong/weak force viz). */
import * as THREE from 'three';
import { FORCE_PALETTES, lerpPalette } from './color-ramps.js';
import {
    VOXEL_CENTER_OFFSET,
    _softSpriteTexture,
} from './field-renderer-shared.js';

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
        if (!this._forceVolume) this._buildForceVolume();
        this._forceVolume.visible = on;
        if (!on) this._forceVolume.geometry.setDrawRange(0, 0);
    },

    // ── Gravity Field Volume (density gradient vectors) ─────────────
    _buildGravityField() {
        const maxArrows = 32768;
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
    },
    toggleGravityField(on) {
        if (!this._gravityField) this._buildGravityField();
        this._gravityField.visible = on;
        if (!on) this._gravityField.geometry.setDrawRange(0, 0);
    },

    // ── EM Force aliases ─────────────────────────────────────────────
    showEMForce(on) { this.toggleForceVolume(on); },
    showGravityForce(on) { this.toggleGravityField(on); },

    // ── Strong Force Volume (Red arrows) ──────────────────────────────
    _buildStrongForce() {
        this._strongForce = this._buildArrowFieldMesh(32768, 0.7);
    },
    toggleStrongForce(on) {
        if (!this._strongForce) this._buildStrongForce();
        this._strongForce.visible = on;
        if (!on) this._strongForce.geometry.setDrawRange(0, 0);
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
            const rgb = lerpPalette(pal, t);

            posAttr.array[vi * 3]     = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;
            colAttr.array[vi * 3]     = rgb[0];
            colAttr.array[vi * 3 + 1] = rgb[1];
            colAttr.array[vi * 3 + 2] = rgb[2];
            vi++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._weakField.geometry.setDrawRange(0, vi);
    },
    toggleWeakField(on) {
        if (!this._weakField) this._buildWeakField();
        this._weakField.visible = on;
        if (!on) this._weakField.geometry.setDrawRange(0, 0);
    },
    showWeakField(on) { this.toggleWeakField(on); },

    // ══════════════════════════════════════════════════════════════════
    //  FORCE VISUALIZATION STYLES (Heatmap / Streamlines / Glyphs)
    // ══════════════════════════════════════════════════════════════════
    _buildForceHeatmap() {
        const maxPts = 8000;
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
                float gauss = exp(-r2 * 16.0);
                gl_FragColor = vec4(vColor * gauss, gauss * uOpacity);
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
        this._forceHeatmap = new THREE.Points(geo, mat);
        this._forceHeatmap.visible = false;
        this._forceHeatmap.frustumCulled = false;
        this._forceHeatmap.renderOrder = 2;
        this._scene.add(this._forceHeatmap);
    },
    initForceHeatmap() { if (!this._forceHeatmap) this._buildForceHeatmap(); },
    updateForceHeatmap(fieldData, forceType) {
        this._syncCenterAndRadius();
        if (!this._forceHeatmap) this._buildForceHeatmap();
        const posAttr  = this._forceHeatmap.geometry.getAttribute('position');
        const colAttr  = this._forceHeatmap.geometry.getAttribute('particleColor');
        const sizeAttr = this._forceHeatmap.geometry.getAttribute('size');
        const { positions, vectors, count } = fieldData;
        const maxPts = posAttr.array.length / 3;
        const pal = FORCE_PALETTES[forceType] || FORCE_PALETTES.em;

        let maxMag = 0;
        if (!this._heatMagCache || this._heatMagCache.length < count) {
            this._heatMagCache = new Float32Array(count);
        }
        const mags = this._heatMagCache;
        for (let i = 0; i < count; i++) {
            const a = vectors[i * 3], b = vectors[i * 3 + 1], c = vectors[i * 3 + 2];
            mags[i] = Math.sqrt(a * a + b * b + c * c);
            if (mags[i] > maxMag) maxMag = mags[i];
        }
        if (maxMag < 1e-15) maxMag = 1;
        const threshold = maxMag * 0.02;
        const _needsClip = this._clipActive();
        const sizeBase = 15 + 10 * (this._latticeSize / 64);
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;

            const t = mag / maxMag;
            const [r, g, b] = lerpPalette(pal, t);
            posAttr.array[vi * 3]     = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;
            colAttr.array[vi * 3]     = r;
            colAttr.array[vi * 3 + 1] = g;
            colAttr.array[vi * 3 + 2] = b;
            sizeAttr.array[vi] = Math.log(1 + t * 9) / Math.log(10) * sizeBase;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._forceHeatmap.geometry.setDrawRange(0, vi);
    },
    showForceHeatmap(visible) {
        if (!this._forceHeatmap) this._buildForceHeatmap();
        this._forceHeatmap.visible = visible;
        if (!visible) this._forceHeatmap.geometry.setDrawRange(0, 0);
    },

    // ── Animated Streamlines (Flow) ──────────────────────────────────
    _buildForceStreamlines() {
        this._forceStreamlinePool = [];
        this._forceStreamlineMats = [];
        const maxLines = 200;
        const maxSegs = 40;
        for (let i = 0; i < maxLines; i++) {
            const posArr = new Float32Array((maxSegs + 1) * 3);
            const geo = new THREE.BufferGeometry();
            geo.setAttribute('position', new THREE.Float32BufferAttribute(posArr, 3));
            geo.setDrawRange(0, 0);
            const mat = new THREE.LineDashedMaterial({
                color: 0x00e5ff,
                dashSize: 1.5,
                gapSize: 0.8,
                transparent: true,
                opacity: 0.7,
                depthWrite: false,
            });
            const line = new THREE.Line(geo, mat);
            line.visible = false;
            line.frustumCulled = false;
            line.computeLineDistances();
            this._scene.add(line);
            this._forceStreamlinePool.push(line);
            this._forceStreamlineMats.push(mat);
        }
        this._forceStreamlineCount = 0;
        // F-5 change-detection cache — must be (re)initialised whenever the
        // pool is (re)built so a fresh empty geometry is never mistaken for an
        // already-uploaded line. -1 = "no data resident yet" → first update
        // always writes + computes distances.
        this._forceStreamlineDrawn = new Int32Array(maxLines).fill(-1);
    },
    initForceStreamlines() { if (!this._forceStreamlinePool) this._buildForceStreamlines(); },
    updateForceStreamlines(lines, forceType) {
        this._syncCenterAndRadius();
        if (!this._forceStreamlinePool) this._buildForceStreamlines();
        const pool = this._forceStreamlinePool;
        const mats = this._forceStreamlineMats;
        const pal = FORCE_PALETTES[forceType] || FORCE_PALETTES.em;
        const baseColor = pal.mid;
        const colorHex = new THREE.Color(baseColor[0], baseColor[1], baseColor[2]);

        // PERF (F-5): cache the per-line drawn vertex count so we can detect
        // when an incoming streamline is element-wise identical to the one
        // already resident in the GPU buffer. computeLineDistances() (dash
        // arc-length integration) + computeBoundingSphere() + the position
        // re-upload are deterministic functions of the vertex data, so when
        // nothing changed they reproduce a byte-identical result — skipping
        // them is output-exact, not approximate. The comparison is O(verts)
        // but avoids the strictly-more-expensive distance integration + GPU
        // upload for unchanged lines (overlay refreshes fire far more often
        // than the field actually changes).
        // `lines` is the POOLED StreamlineResult from computeStreamlines
        // ({count, buffer, offsets, lengths}) — web/engine-optimization-
        // 2026-05-31. Line li is buffer[offsets[li] .. offsets[li]+lengths[li]);
        // `base + v` indexes the floats the old per-line `verts[v]` did, so the
        // F-5 element-wise change comparison and the upload are byte-identical.
        const drawnCounts = this._forceStreamlineDrawn ||
            (this._forceStreamlineDrawn = new Int32Array(pool.length).fill(-1));
        const buffer = lines.buffer;
        const offsets = lines.offsets;
        const lengths = lines.lengths;
        const usedCount = Math.min(lines.count, pool.length);
        for (let li = 0; li < usedCount; li++) {
            const base = offsets[li];
            const line = pool[li];
            const posAttr = line.geometry.getAttribute('position');
            const maxVerts = posAttr.array.length / 3;
            const vertCount = Math.min(lengths[li] / 3, maxVerts);

            const arr = posAttr.array;
            const n3 = vertCount * 3;
            let changed = drawnCounts[li] !== vertCount;
            if (!changed) {
                for (let v = 0; v < n3; v++) {
                    if (arr[v] !== buffer[base + v]) { changed = true; break; }
                }
            }
            if (changed) {
                for (let v = 0; v < n3; v++) {
                    arr[v] = buffer[base + v];
                }
                posAttr.needsUpdate = true;
                line.geometry.setDrawRange(0, vertCount);
                line.geometry.computeBoundingSphere();
                line.computeLineDistances();
                drawnCounts[li] = vertCount;
            }

            mats[li].color.copy(colorHex);
            mats[li].opacity = Math.min(0.8, 0.3 + vertCount / 40 * 0.5);
            line.visible = true;
        }

        for (let li = usedCount; li < pool.length; li++) {
            pool[li].visible = false;
        }
        this._forceStreamlineCount = usedCount;
    },
    animateForceStreamlines(dt) {
        if (!this._forceStreamlineMats) return;
        const speed = 2.0;
        for (let i = 0; i < this._forceStreamlineCount; i++) {
            this._forceStreamlineMats[i].dashOffset -= speed * dt;
        }
    },
    showForceStreamlines_vis(visible) {
        if (!this._forceStreamlinePool) this._buildForceStreamlines();
        for (let i = 0; i < this._forceStreamlinePool.length; i++) {
            if (!visible) this._forceStreamlinePool[i].visible = false;
        }
    },

    // ── Glyph Field (Instanced Cones) ────────────────────────────────
    _buildForceGlyphMesh(forceType) {
        const maxInstances = 8000;
        const coneGeo = new THREE.ConeGeometry(0.3, 1.0, 6);
        coneGeo.rotateX(Math.PI / 2);
        const mat = new THREE.MeshBasicMaterial({
            transparent: true,
            opacity: 0.7,
            depthWrite: false,
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
        const maxInstances = mesh.count === undefined ? 8000 : (mesh.instanceMatrix.array.length / 16);
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

            const [r, g, b] = lerpPalette(pal, t);
            colorArr[vi * 3]     = r;
            colorArr[vi * 3 + 1] = g;
            colorArr[vi * 3 + 2] = b;
            vi++;
        }

        mesh.count = vi;
        mesh.instanceMatrix.needsUpdate = true;
        mesh.instanceColor.needsUpdate = true;
    },
    showForceGlyphs(visible) {
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