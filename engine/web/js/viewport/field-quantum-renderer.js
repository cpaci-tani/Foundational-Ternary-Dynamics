/** fieldQuantumMethods — ViewportFieldRenderer mixin (dual / chirality / quantum / entropy). */
import * as THREE from 'three';
import { rampViridis, rampDivergingRdBu, rampGrayscale } from './color-ramps.js';
import {
    VOXEL_CENTER_OFFSET,
    _makeParticleFragMaterial,
    _ensureManifestAttrs,
} from './field-renderer-shared.js';

export const fieldQuantumMethods = {
    _buildDualFluxVolume() {
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
            { uOpacity: { value: 0.7 } },
            { blending: THREE.AdditiveBlending },
        );
        this._dualFluxVolume = new THREE.Points(geo, mat);
        this._dualFluxVolume.visible = false;
        this._dualFluxVolume.frustumCulled = false;
        this._scene.add(this._dualFluxVolume);
    },
    updateDualFluxVolume(lData, rData) {
        this._syncCenterAndRadius();
        if (!this._dualFluxVolume) this._buildDualFluxVolume();
        const posAttr = this._dualFluxVolume.geometry.getAttribute('position');
        const colAttr = this._dualFluxVolume.geometry.getAttribute('particleColor');
        const sizeAttr = this._dualFluxVolume.geometry.getAttribute('size');
        const maxPts = posAttr.array.length / 3;
        let maxL = 0, maxR = 0;

        const lCount = lData.count, rCount = rData.count;
        const totalDual = lCount + rCount;
        if (!this._magCacheDual || this._magCacheDual.length < totalDual) this._magCacheDual = new Float32Array(totalDual);
        const dualMags = this._magCacheDual;
        for (let i = 0; i < lCount; i++) {
            const a = lData.vectors[i * 3], b = lData.vectors[i * 3 + 1], c = lData.vectors[i * 3 + 2];
            const m = Math.sqrt(a * a + b * b + c * c);
            dualMags[i] = m;
            if (m > maxL) maxL = m;
        }
        for (let i = 0; i < rCount; i++) {
            const a = rData.vectors[i * 3], b = rData.vectors[i * 3 + 1], c = rData.vectors[i * 3 + 2];
            const m = Math.sqrt(a * a + b * b + c * c);
            dualMags[lCount + i] = m;
            if (m > maxR) maxR = m;
        }
        const maxVal = Math.max(maxL, maxR, 1e-20);
        const threshold = maxVal * 0.02;
        let vi = 0;

        const _needsClip = this._clipActive();

        for (let i = 0; i < lCount && vi < maxPts; i++) {
            const mag = dualMags[i];
            if (mag < threshold) continue;
            const px = lData.positions[i * 3], py = lData.positions[i * 3 + 1], pz = lData.positions[i * 3 + 2];
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            posAttr.array[vi * 3] = px; posAttr.array[vi * 3 + 1] = py; posAttr.array[vi * 3 + 2] = pz;
            const t = mag / maxVal;
            colAttr.array[vi * 3] = 0.9 * t; colAttr.array[vi * 3 + 1] = 0.4 * t; colAttr.array[vi * 3 + 2] = 0.15 * t;
            sizeAttr.array[vi] = 1.0 + 4.0 * t;
            vi++;
        }
        for (let i = 0; i < rCount && vi < maxPts; i++) {
            const mag = dualMags[lCount + i];
            if (mag < threshold) continue;
            const px = rData.positions[i * 3], py = rData.positions[i * 3 + 1], pz = rData.positions[i * 3 + 2];
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            posAttr.array[vi * 3] = px; posAttr.array[vi * 3 + 1] = py; posAttr.array[vi * 3 + 2] = pz;
            const t = mag / maxVal;
            colAttr.array[vi * 3] = 0.3 * t; colAttr.array[vi * 3 + 1] = 0.2 * t; colAttr.array[vi * 3 + 2] = 0.9 * t;
            sizeAttr.array[vi] = 1.0 + 4.0 * t;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._dualFluxVolume.geometry.setDrawRange(0, vi);
    },
    toggleDualFluxVolume(on) {
        if (!this._dualFluxVolume) this._buildDualFluxVolume();
        this._dualFluxVolume.visible = on;
        if (!on) this._dualFluxVolume.geometry.setDrawRange(0, 0);
    },

    // ── Chirality Field (Red L-dominant / Blue R-dominant) ───────────
    _buildChiralityField() {
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
            { uOpacity: { value: 0.7 } },
            { blending: THREE.AdditiveBlending },
        );
        this._chiralityField = new THREE.Points(geo, mat);
        this._chiralityField.visible = false;
        this._chiralityField.frustumCulled = false;
        this._scene.add(this._chiralityField);
    },
    updateChiralityField(fieldData) {
        this._syncCenterAndRadius();
        if (!this._chiralityField) this._buildChiralityField();
        const posAttr = this._chiralityField.geometry.getAttribute('position');
        const colAttr = this._chiralityField.geometry.getAttribute('particleColor');
        const sizeAttr = this._chiralityField.geometry.getAttribute('size');
        const { positions, values, count } = fieldData;
        const maxPts = posAttr.array.length / 3;
        let maxVal = 0;
        for (let i = 0; i < count; i++) {
            const a = Math.abs(values[i]);
            if (a > maxVal) maxVal = a;
        }
        const threshold = maxVal * 0.02;
        const _needsClip = this._clipActive();
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const v = values[i];
            if (Math.abs(v) < threshold) continue;

            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;

            posAttr.array[vi * 3] = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;

            const t = Math.abs(v) / maxVal;
            if (v > 0) {
                colAttr.array[vi * 3] = 0.9 * t; colAttr.array[vi * 3 + 1] = 0.25 * t; colAttr.array[vi * 3 + 2] = 0.15 * t;
            } else {
                colAttr.array[vi * 3] = 0.15 * t; colAttr.array[vi * 3 + 1] = 0.35 * t; colAttr.array[vi * 3 + 2] = 0.9 * t;
            }
            sizeAttr.array[vi] = 1.0 + 4.0 * t;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._chiralityField.geometry.setDrawRange(0, vi);
    },
    toggleChiralityField(on) {
        if (!this._chiralityField) this._buildChiralityField();
        this._chiralityField.visible = on;
        if (!on) this._chiralityField.geometry.setDrawRange(0, 0);
    },

    // ══════════════════════════════════════════════════════════════════
    // ── Tier 1 Quantum Overlays ───────────────────────────────────────
    _buildSoftDiscTexture() {
        if (this._softDiscTex) return this._softDiscTex;
        const size = 64;
        const c = document.createElement('canvas');
        c.width = size; c.height = size;
        const ctx = c.getContext('2d');
        const grad = ctx.createRadialGradient(size / 2, size / 2, 0, size / 2, size / 2, size / 2);
        grad.addColorStop(0.0,   'rgba(255,255,255,1.0)');
        grad.addColorStop(0.45,  'rgba(255,255,255,0.6)');
        grad.addColorStop(0.85,  'rgba(255,255,255,0.08)');
        grad.addColorStop(1.0,   'rgba(255,255,255,0.0)');
        ctx.fillStyle = grad;
        ctx.fillRect(0, 0, size, size);
        const tex = new THREE.CanvasTexture(c);
        tex.needsUpdate = true;
        this._softDiscTex = tex;
        return tex;
    },
    _buildQuantumField() {
        const maxPts = 16384;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const tex = this._buildSoftDiscTexture();
        const mat = new THREE.PointsMaterial({
            map: tex,
            alphaMap: tex,
            size: 2.8,
            vertexColors: true,
            transparent: true,
            opacity: 0.95,
            depthWrite: false,
            sizeAttenuation: true,
            blending: THREE.AdditiveBlending,
        });
        this._quantumField = new THREE.Points(geo, mat);
        this._quantumField.visible = false;
        this._quantumField.frustumCulled = false;
        this._quantumField.renderOrder = 4;
        this._scene.add(this._quantumField);
        this._quantumFieldKind = null;
    },
    _quantumSetVisibility() {
        if (!this._quantumField) return;
        const pointCloudOn = !!(this._psi2Visible || this._lagrangianVisible || this._entropyVisible);
        this._quantumField.visible = pointCloudOn;
        if (!pointCloudOn) this._quantumField.geometry.setDrawRange(0, 0);
    },
    _populateQuantumField(data, kind, options = {}) {
        this._syncCenterAndRadius();
        if (!this._quantumField) this._buildQuantumField();
        if (!data || !data.positions || !data.values || !data.count) return;
        const posAttr = this._quantumField.geometry.getAttribute('position');
        const colAttr = this._quantumField.geometry.getAttribute('color');
        const maxPts = posAttr.array.length / 3;
        const { positions, values, count } = data;

        const signed = options.signed === true;
        let maxAbs = options.normalizer;
        if (!maxAbs) {
            maxAbs = 0;
            for (let i = 0; i < count; i++) {
                const v = Math.abs(values[i]);
                if (v > maxAbs) maxAbs = v;
            }
        }
        const eps = 1e-9;
        const denom = Math.max(maxAbs, eps);
        const ramp = options.ramp;
        const threshold = options.threshold !== undefined ? options.threshold : 0.02;
        const _needsClip = this._clipActive();
        let vi = 0;
        for (let i = 0; i < count && vi < maxPts; i++) {
            const raw = values[i];
            const v = signed ? raw / denom : Math.abs(raw) / denom;
            if (!signed && v < threshold) continue;
            if (signed && Math.abs(v) < threshold) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            posAttr.array[vi * 3]     = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;
            ramp(signed ? v : v, colAttr.array, vi * 3);
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._quantumField.geometry.setDrawRange(0, vi);
        this._quantumFieldKind = kind;
    },

    // ── |ψ|² Born density ─────────────────────────────────────────────
    togglePsiSquaredField(on) {
        this._psi2Visible = !!on;
        if (!this._quantumField) this._buildQuantumField();
        this._quantumSetVisibility();
    },
    updatePsiSquaredField(data) {
        this._psi2Data = data;
        if (!this._psi2Visible) return;
        this._populateQuantumField(data, 'psi2', {
            signed: false,
            ramp: (t, out, i) => rampViridis(t, out, i),
            normalizer: data?.normalizer,
        });
    },

    // ── Phase φ — directional line-segments (needles) ────────────────
    _buildPhaseNeedles() {
        const maxPts = 8192;
        const positions = new Float32Array(maxPts * 6);
        const colors    = new Float32Array(maxPts * 6);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color',    new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true,
            transparent: true,
            opacity: 0.85,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
            linewidth: 2,
        });
        this._phaseNeedles = new THREE.LineSegments(geo, mat);
        this._phaseNeedles.visible = false;
        this._phaseNeedles.frustumCulled = false;
        this._phaseNeedles.renderOrder = 5;
        this._scene.add(this._phaseNeedles);
    },
    toggleLagrangianDensityField(on) {
        this._lagrangianVisible = !!on;
        if (!this._quantumField) this._buildQuantumField();
        this._quantumSetVisibility();
    },
    updateLagrangianDensityField(data) {
        this._lagrangianData = data;
        if (!this._lagrangianVisible) return;
        this._populateQuantumField(data, 'lagrangian', {
            signed: true,
            ramp: (t, out, i) => rampDivergingRdBu(t, out, i),
            normalizer: data?.normalizer,
            threshold: 0.10,
        });
    },

    // ── Entropy s(x) — jittering sparkles ─────────────────────────────
    toggleEntropyDensityField(on) {
        this._entropyVisible = !!on;
        if (!this._quantumField) this._buildQuantumField();
        this._entropyJitterSeed = Date.now();
        this._quantumSetVisibility();
    },
    updateEntropyDensityField(data) {
        this._syncCenterAndRadius();
        this._entropyData = data;
        if (!this._entropyVisible) return;
        if (!this._quantumField) this._buildQuantumField();
        const posAttr = this._quantumField.geometry.getAttribute('position');
        const colAttr = this._quantumField.geometry.getAttribute('color');
        const maxPts = posAttr.array.length / 3;
        const _needsClip = this._clipActive();
        const { positions, values, count } = data;
        const JITTER_SCALE = 0.8;
        let vi = 0;
        for (let i = 0; i < count && vi < maxPts; i++) {
            const s = Math.max(0, Math.min(1, values[i]));
            if (s < 0.04) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            const seed = (i * 9301 + this._entropyJitterSeed) & 0x7fffffff;
            const r1 = ((seed * 49297) % 233280) / 233280 - 0.5;
            const r2 = ((seed * 2147) % 233280) / 233280 - 0.5;
            const r3 = ((seed * 8191) % 233280) / 233280 - 0.5;
            const offset = s * JITTER_SCALE;
            posAttr.array[vi * 3]     = px + r1 * offset;
            posAttr.array[vi * 3 + 1] = py + r2 * offset;
            posAttr.array[vi * 3 + 2] = pz + r3 * offset;
            rampGrayscale(s, colAttr.array, vi * 3);
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._quantumField.geometry.setDrawRange(0, vi);
        this._quantumFieldKind = 'entropy';
    },

    // ══════════════════════════════════════════════════════════════════
    // ── Event-horizon isosurface overlay ─────────────────────────────
    _buildHorizonField() {
        const max = 8192;
        const geo = new THREE.BufferGeometry();
        const pos = new Float32Array(max * 3);
        geo.setAttribute('position', new THREE.BufferAttribute(pos, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.PointsMaterial({
            color: 0x110022, size: 0.85, transparent: true, opacity: 0.85,
            depthWrite: false, sizeAttenuation: true,
        });
        const points = new THREE.Points(geo, mat);
        points.visible = false;
        points.renderOrder = 4;
        points.frustumCulled = false;
        this._scene.add(points);
        this._horizonField = { points, geo, capacity: max };
    },
    toggleHorizonField(on) {
        if (!this._horizonField) this._buildHorizonField();
        this._horizonField.points.visible = !!on;
    },
    updateHorizonField(data) {
        this._syncCenterAndRadius();
        if (!data?.count) return;
        if (!this._horizonField) this._buildHorizonField();
        const hf = this._horizonField;
        if (!hf.points.visible) return;
        const pos = hf.geo.attributes.position;
        if (data.count <= hf.capacity) {
            pos.array.set(data.positions.subarray(0, data.count * 3));
            pos.needsUpdate = true;
            hf.geo.setDrawRange(0, data.count);
            return;
        }
        const step = data.count / hf.capacity;
        for (let i = 0; i < hf.capacity; i++) {
            const src = Math.min(data.count - 1, (i * step) | 0);
            pos.array[i * 3]     = data.positions[src * 3];
            pos.array[i * 3 + 1] = data.positions[src * 3 + 1];
            pos.array[i * 3 + 2] = data.positions[src * 3 + 2];
        }
        pos.needsUpdate = true;
        hf.geo.setDrawRange(0, hf.capacity);
    },

    // ══════════════════════════════════════════════════════════════════
    // ── State field s — ternary {-1,0,+1} manifestation point cloud ───
    // The literal FTD ontology (Postulate 3). Void (s=0) is the implicit
    // background and is not drawn; manifested voxels render as a point
    // cloud coloured by sign: s=-1 blue, s=+1 red. Data comes from the
    // engine's ternary state buffer (getStateFieldSampled) on WASM, or the
    // manifested-particle set on the MockBridge.
    _buildScalarCloud(key, size = 16384) {
        if (!this._scalarClouds) this._scalarClouds = {};
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.BufferAttribute(new Float32Array(size * 3), 3));
        geo.setAttribute('color', new THREE.BufferAttribute(new Float32Array(size * 3), 3));
        geo.setDrawRange(0, 0);
        const tex = this._buildSoftDiscTexture();
        const mat = new THREE.PointsMaterial({
            map: tex, alphaMap: tex, size: 3.0, vertexColors: true,
            transparent: true, opacity: 0.9, depthWrite: false,
            sizeAttenuation: true, blending: THREE.AdditiveBlending,
        });
        const points = new THREE.Points(geo, mat);
        points.visible = false;
        points.renderOrder = 4;
        points.frustumCulled = false;
        this._scene.add(points);
        this._scalarClouds[key] = { points, geo, capacity: size };
    },
    _toggleScalarCloud(key, on) {
        if (!this._scalarClouds || !this._scalarClouds[key]) this._buildScalarCloud(key);
        this._scalarClouds[key].points.visible = !!on;
        if (!on) this._scalarClouds[key].geo.setDrawRange(0, 0);
    },
    _updateScalarCloud(key, data, colorize, opts = {}) {
        this._syncCenterAndRadius();
        if (!this._scalarClouds || !this._scalarClouds[key]) this._buildScalarCloud(key);
        const sc = this._scalarClouds[key];
        if (!sc.points.visible) return;
        if (!data || !data.count) { sc.geo.setDrawRange(0, 0); return; }
        const posAttr = sc.geo.attributes.position;
        const colAttr = sc.geo.attributes.color;
        const cap = sc.capacity;
        const _needsClip = this._clipActive();
        const { positions, values, count } = data;
        const signed = opts.signed === true;
        let maxAbs = opts.normalizer;
        if (!maxAbs) { maxAbs = 0; for (let i = 0; i < count; i++) { const a = Math.abs(values[i]); if (a > maxAbs) maxAbs = a; } }
        const denom = Math.max(maxAbs, 1e-9);
        const thr = opts.threshold !== undefined ? opts.threshold : 0.02;
        const rgb = [0, 0, 0];
        let vi = 0;
        for (let i = 0; i < count && vi < cap; i++) {
            const t = signed ? values[i] / denom : Math.abs(values[i]) / denom;
            if (Math.abs(t) < thr) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET;
            const py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET;
            const pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            posAttr.array[vi * 3] = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;
            colorize(t, rgb);
            colAttr.array[vi * 3] = rgb[0];
            colAttr.array[vi * 3 + 1] = rgb[1];
            colAttr.array[vi * 3 + 2] = rgb[2];
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sc.geo.setDrawRange(0, vi);
    },

    // ── Volumetric scalar HEAT MAP (overlays-panel "Heat Map" meta-toggle) ──
    // Render any scalar frame {positions, values, count} as an additive glow
    // cloud coloured by a color-ramps.js ramp — the thermal "heat map" view the
    // Heat Map toggle switches the volumetric scalar overlays into, in place of
    // their default rubber-sheet (emEnergy/pressures/charge/vorticity/gravPot)
    // or native cloud (psi²/latency/…). Reuses the tested _updateScalarCloud
    // infra under a 'heat:' key namespace so a field's heat-map cloud never
    // collides with its own default cloud. `ramp` is a color-ramps writer
    // (t,out,i); `signed` picks diverging vs magnitude normalisation.
    updateScalarHeatmap(key, data, ramp, signed) {
        this._updateScalarCloud('heat:' + key, data,
            (t, rgb) => { ramp(t, rgb, 0); },
            { signed: !!signed, normalizer: data && data.normalizer, threshold: 0.02 });
    },
    showScalarHeatmap(key, on) { this._toggleScalarCloud('heat:' + key, on); },
    hideAllScalarHeatmaps() {
        if (!this._scalarClouds) return;
        for (const k in this._scalarClouds) {
            if (k.indexOf('heat:') === 0) {
                this._scalarClouds[k].points.visible = false;
                this._scalarClouds[k].geo.setDrawRange(0, 0);
            }
        }
    },

    // Latency L ∈ [0,1] — blue (low / flat space) → red (high / gravity well).
    toggleLatencyField(on) { this._toggleScalarCloud('latency', on); },
    updateLatencyField(data) {
        this._updateScalarCloud('latency', data,
            (t, rgb) => { rgb[0] = Math.min(1, t); rgb[1] = 0.18; rgb[2] = Math.min(1, 1 - t); },
            { normalizer: data?.normalizer || 1, threshold: 0.02 });
    },

    // Gauss residual — signed: red = positive (source excess), blue = negative.
    toggleGaussResidualField(on) { this._toggleScalarCloud('gaussResidual', on); },
    updateGaussResidualField(data) {
        this._updateScalarCloud('gaussResidual', data,
            (t, rgb) => { if (t >= 0) { rgb[0] = Math.min(1, t); rgb[1] = 0.15; rgb[2] = 0.10; } else { rgb[0] = 0.10; rgb[1] = 0.20; rgb[2] = Math.min(1, -t); } },
            { signed: true, normalizer: data?.normalizer, threshold: 0.05 });
    },

    // ══════════════════════════════════════════════════════════════════
    // ── |ψ|² breathing animation ──────────────────────────────────────
    _animateQuantumField() {
        if (!this._quantumField || !this._psi2Visible) return;
        if (this._quantumFieldKind !== 'psi2') return;
        const tMs = this._animationClock || 0;
        const phase = (tMs / 1000) * Math.PI * 0.6;
        const pulse = 0.85 + 0.15 * Math.sin(phase);
        this._quantumField.material.opacity = pulse;
    }

};