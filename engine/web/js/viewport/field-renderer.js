/**
 * @file engine/web/js/viewport/field-renderer.js
 * @purpose Owns ALL field overlays for the Scale-0 lattice dashboard:
 *          E/B fields, Poynting, divergence, force volumes (EM/gravity/
 *          strong/weak in 4 styles), dark matter halo, damping zones,
 *          genesis isosurface, confinement strings, dual flux, chirality,
 *          light, horizon, plus quantum field / phase / Lagrangian /
 *          entropy overlays. The largest of the 4 viewport sub-renderers
 *          extracted in Phase 3 of the refactor sweep — owns 27+
 *          distinct field-overlay meshes.
 * @consumers engine/web/js/viewport.js (composes this via constructor);
 *            engine/web/js/viewport/flux-renderer.js (calls
 *            buildStreamlineMesh + writeStreamlinesIntoMesh — the
 *            canonical mesh-factory home is now here);
 *            engine/web/js/viewport/particle-renderer.js (may call
 *            writeArrowFieldIntoMesh for force-glyph rendering).
 * @contract CONTRACTS.md §2 (Capability Factory Contract)
 * @related ./scene-core.js (3a, settled), ./flux-renderer.js (3b,
 *          settled), ./particle-renderer.js (3d, settled),
 *          ./REFACTOR_MAP.md (extraction guide)
 *
 * Phase 3c — final viewport sub-phase. Mesh-factory helpers
 * (buildStreamlineMesh, buildArrowFieldMesh, writeArrowFieldIntoMesh,
 * writeStreamlinesIntoMesh) live here as the canonical home;
 * FluxRenderer + ParticleRenderer's constructor callbacks now route
 * to FieldRenderer's bound methods.
 */

import * as THREE from 'three';
import { potentialToColor, magnitudeToColor, fluxToColor, potentialToColorInto, magnitudeToColorInto } from '../fields.js';
import { K_B, K_GENESIS } from '../constants.js';

// Confinement-string visual: draw a color-pair proximity glyph between any
// two particles whose squared separation is below this cutoff. [IMPOSED]
// (audit P2-9: replaces a bare `J2_threshold_dist2 = 120` magic literal).
// √120 ≈ 10.95 voxels — the visual reach of a flux string before it fades.
// Defined module-locally (not in the shared constants.js) to keep this
// Section-F change confined to the owned file; value preserved verbatim so
// the rendered pair set is bit-identical to the prior hardcode.
const CONFINEMENT_PAIR_DIST2 = 120.0;

// ── Voxel-center rendering convention ─────────────────────────────────
// Lattice voxel index k is rendered at world centre k+0.5 (see
// scene-core.js, flux-renderer.js, ftd_wasm.cpp particle pos_cache, and
// the wireframe crosshair in boundary-geometry.js). As of the April-19
// patch, ftd_wasm.cpp handles the +0.5f centering natively for all samplers.
// Thus, VOXEL_CENTER_OFFSET = 0.0 is correct and MUST NOT BE CHANGED.
// DO NOT "fix" this to 0.5, it will break visual alignment!
let VOXEL_CENTER_OFFSET = 0.0;
import {
    rampViridis,
    rampCyclicHSL,
    rampDivergingRdBu,
    rampGrayscale,
    FORCE_PALETTES,
    lerpPalette,
} from './color-ramps.js';

// Pre-allocated buffer size — centralized in viewport/constants.js (D-6).
import { MAX_FIELD_GRID } from './constants.js';

// Shared particle shaders — centralized in viewport/shaders.js (D-1).
import { PARTICLE_VERT, PARTICLE_FRAG } from './shaders.js';


// Lazy-built static texture for soft-disc sprite (weak-field / quantum overlays).
let __softSpriteTex = null;
function _softSpriteTexture() {
    if (__softSpriteTex) return __softSpriteTex;
    const s = 64;
    const canvas = document.createElement('canvas');
    canvas.width = s; canvas.height = s;
    const ctx = canvas.getContext('2d');
    const g = ctx.createRadialGradient(s/2, s/2, 0, s/2, s/2, s/2);
    g.addColorStop(0.00, 'rgba(255, 220, 255, 1.00)');
    g.addColorStop(0.20, 'rgba(255, 255, 255, 0.85)');
    g.addColorStop(0.55, 'rgba(255, 255, 255, 0.35)');
    g.addColorStop(1.00, 'rgba(255, 255, 255, 0.00)');
    ctx.fillStyle = g;
    ctx.fillRect(0, 0, s, s);
    const tex = new THREE.CanvasTexture(canvas);
    tex.needsUpdate = true;
    __softSpriteTex = tex;
    return tex;
}

export class ViewportFieldRenderer {
    constructor({
        scene,
        camera,
        latticeSize,
        halfN,
        boundaryShape,
        insideBoundary,
        getBoundaryMode,
    }) {
        this._scene = scene;
        this._camera = camera;
        this._latticeSize = latticeSize;
        this._halfN = halfN;
        this._center = latticeSize / 2;
        this._radius = latticeSize / 2;
        this._boundaryShape = boundaryShape;
        this._insideBoundary = insideBoundary;
        this._getBoundaryMode = getBoundaryMode || (() => 'lattice');

        // State owned by FieldRenderer (every mesh starts null and is built lazily).
        this._fieldHeatmap = null;
        this._fieldVectors = null;
        this._peStreamlines = null;
        this._gravityVectors = null;
        this._eFieldLines = null;
        this._bFieldLines = null;
        this._poyntingVectors = null;
        this._divField = null;
        this._forceVolume = null;
        this._gravityField = null;
        this._strongForce = null;
        this._weakField = null;
        this._forceHeatmap = null;
        this._forceStreamlinePool = null;
        this._forceStreamlineMats = null;
        this._forceStreamlineCount = 0;
        this._forceGlyphMeshes = null;
        this._darkMatterHalo = null;
        this._eventHorizonSphere = null;
        this._eventHorizonRing = null;
        this._dampingZones = null;
        this._genesisIsosurface = null;
        this._confinementStrings = null;
        this._dualFluxVolume = null;
        this._chiralityField = null;
        this._lightField = null;
        this._quantumField = null;
        this._quantumFieldKind = null;
        this._softDiscTex = null;
        this._phaseNeedles = null;
        this._horizonField = null;

        // Visibility state flags (mirrors viewport.js originals).
        this.showHeatmap = false;
        this._psi2Visible = false;
        this._phaseVisible = false;
        this._lagrangianVisible = false;
        this._entropyVisible = false;
        this._psi2Data = null;
        this._phaseData = null;
        this._lagrangianData = null;
        this._entropyData = null;
        this._entropyJitterSeed = 0;
        this._animationClock = 0;

        // Per-overlay magnitude scratch caches.
        this._magCache = null;
        this._strongMagCache = null;
        this._heatMagCache = null;
        this._magCacheDual = null;
    }


    // PERF (F-13): true only when the active boundary shape actually clips.
    // For 'cube' / 'none' / undefined, insideBoundary() returns true for every
    // point, so the per-voxel insideBoundary() call in the hot arrow/streamline
    // loops is pure overhead. Hoisting `const _needsClip = this._clipActive();`
    // once per update and gating the call on it is output-EXACT (identical
    // control flow — the call would have returned true and never `continue`d)
    // while skipping ~100k function calls + 3 divisions per upload at L=64.
    // Mirrors the hoist already present in flux-renderer.js:233-234.
    _clipActive() {
        const bs = this._boundaryShape;
        return !(bs === 'cube' || bs === 'none' || bs === undefined);
    }

    _checkInsideBoundary(x, y, z) {
        const isOrigin = this._getBoundaryMode && this._getBoundaryMode() === 'origin';
        if (isOrigin) {
            const boundaryRadius = 35.0; // PE boundary radius
            return this._insideBoundary(x / boundaryRadius, y / boundaryRadius, z / boundaryRadius);
        } else {
            return this._insideBoundary((x - this._center) / this._radius, (y - this._center) / this._radius, (z - this._center) / this._radius);
        }
    }

    _syncCenterAndRadius() {
        const isOrigin = this._getBoundaryMode && this._getBoundaryMode() === 'origin';
        if (isOrigin) {
            this._center = 0.0;
            this._radius = 35.0;
            VOXEL_CENTER_OFFSET = 0.0;
        } else {
            this._center = this._latticeSize / 2;
            this._radius = this._latticeSize / 2;
            VOXEL_CENTER_OFFSET = 0.0;
        }
    }

    onLatticeSizeChanged(size, halfN) {
        this._latticeSize = size;
        this._halfN = halfN;
        this._center = size / 2;
        this._radius = size / 2;

        const isOrigin = this._getBoundaryMode && this._getBoundaryMode() === 'origin';
        const cx = isOrigin ? 0.0 : this._center;
        const cy = isOrigin ? 0.0 : this._center;
        const cz = isOrigin ? 0.0 : this._center;

        if (this._eventHorizonSphere) {
            this._eventHorizonSphere.position.set(cx, cy, cz);
        }
        if (this._eventHorizonRing) {
            this._eventHorizonRing.position.set(cx, cy, cz);
        }

        // Rebuild field heatmap for new lattice capacity (it sizes from MAX_FIELD_GRID
        // so capacity is fine, but ensure stale data is cleared).
        if (this._fieldHeatmap) {
            this._scene.remove(this._fieldHeatmap);
            this._fieldHeatmap.geometry.dispose();
            this._fieldHeatmap.material.dispose();
            this._fieldHeatmap = null;
        }

        // Clear draw ranges on every dynamic field mesh so stale L-data doesn't persist.
        const dynamicMeshes = [
            this._fieldVectors, this._peStreamlines, this._gravityVectors,
            this._eFieldLines, this._bFieldLines, this._poyntingVectors,
            this._divField, this._forceVolume, this._gravityField,
            this._strongForce, this._weakField, this._forceHeatmap,
            this._darkMatterHalo, this._dampingZones, this._genesisIsosurface,
            this._confinementStrings, this._dualFluxVolume,
            this._chiralityField, this._lightField, this._phaseNeedles,
            this._quantumField,
        ];
        for (const m of dynamicMeshes) {
            if (m && m.geometry) m.geometry.setDrawRange(0, 0);
        }
    }

    setBoundaryShape(shape) {
        this._boundaryShape = shape;
        // Most field meshes don't rebuild on shape change — clipping is checked
        // dynamically via the insideBoundary callback per-frame.
    }

    // Animation clock pass-through used by _animateQuantumField. The orchestrator
    // sets this each frame from its own advanceAnimationClock accumulator.
    setAnimationClock(ms) {
        this._animationClock = ms;
    }

    // ── Field Heatmap (potential colored grid dots on XZ plane) ───────

    _buildFieldHeatmap() {
        const positions = new Float32Array(MAX_FIELD_GRID * 3);
        const colors = new Float32Array(MAX_FIELD_GRID * 3);
        const sizes = new Float32Array(MAX_FIELD_GRID);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        geo.setDrawRange(0, 0);
        const mat = new THREE.ShaderMaterial({
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.9 } },
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.NormalBlending,
        });
        this._fieldHeatmap = new THREE.Points(geo, mat);
        this._fieldHeatmap.visible = false;
        this._fieldHeatmap.frustumCulled = false;
        this._fieldHeatmap.renderOrder = -1;
        this._scene.add(this._fieldHeatmap);
    }

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
    }

    toggleFieldHeatmap(on) {
        if (!this._fieldHeatmap) this._buildFieldHeatmap();
        this._fieldHeatmap.visible = on;
        if (!on) this._fieldHeatmap.geometry.setDrawRange(0, 0);
    }

    // ── Flux Slice (uses _fieldHeatmap mesh) ──────────────────────────

    updateFluxSlice(sliceData, latticeSize, axis, index) {
        this._syncCenterAndRadius();
        if (!this._fieldHeatmap) this._buildFieldHeatmap();
        const posAttr = this._fieldHeatmap.geometry.getAttribute('position');
        const colAttr = this._fieldHeatmap.geometry.getAttribute('particleColor');
        const sizeAttr = this._fieldHeatmap.geometry.getAttribute('size');
        const N = latticeSize;

        // Find max for normalization
        let maxFlux = 0;
        const total = N * N;
        if (!sliceData || sliceData.length !== total) {
            // Size mismatch (e.g. during async resize transition or startup lag) — skip rendering this frame
            return;
        }
        for (let i = 0; i < total; i++) {
            if (sliceData[i] > maxFlux) maxFlux = sliceData[i];
        }

        const halfN = N / 2;
        const _needsClip = this._clipActive();
        let count = 0;
        const maxPts = Math.min(total, MAX_FIELD_GRID);
        for (let i = 0; i < total && count < maxPts; i++) {
            const a = Math.floor(i / N);
            const b = i % N;
            let x, y, z;
            if (axis === 0) { x = index; y = a; z = b; }
            else if (axis === 1) { x = a; y = index; z = b; }
            else { x = a; y = b; z = index; }

            // Clip to boundary shape (F-13: skip entirely when the shape never
            // clips — same control flow, no per-voxel call or 3 divisions).
            if (_needsClip) {
                const nx = (x + 0.5 - this._center) / this._radius;
                const ny = (y + 0.5 - this._center) / this._radius;
                const nz = (z + 0.5 - this._center) / this._radius;
                if (!this._insideBoundary(nx, ny, nz)) continue;
            }

            posAttr.array[count * 3]     = x + 0.5;
            posAttr.array[count * 3 + 1] = y + 0.5;
            posAttr.array[count * 3 + 2] = z + 0.5;

            const [r, g, b2] = fluxToColor(sliceData[i], maxFlux);
            colAttr.array[count * 3] = r;
            colAttr.array[count * 3 + 1] = g;
            colAttr.array[count * 3 + 2] = b2;

            const t = sliceData[i] / (maxFlux + 1e-20);
            sizeAttr.array[count] = 1.0 + 4.0 * t;
            count++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._fieldHeatmap.geometry.setDrawRange(0, count);
    }

    toggleFluxSlice(on) {
        if (!this._fieldHeatmap) this._buildFieldHeatmap();
        this._fieldHeatmap.visible = on;
        this.showHeatmap = on;
        if (!on) this._fieldHeatmap.geometry.setDrawRange(0, 0);
    }

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
    }

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
    }

    toggleFieldVectors(on) {
        if (!this._fieldVectors) this._buildFieldVectors();
        this._fieldVectors.visible = on;
        if (!on) this._fieldVectors.geometry.setDrawRange(0, 0);
    }

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
    }

    updatePEStreamlines(lines) {
        this._syncCenterAndRadius();
        if (!this._peStreamlines) this._buildPEStreamlines();
        const posAttr = this._peStreamlines.geometry.getAttribute('position');
        const colAttr = this._peStreamlines.geometry.getAttribute('color');
        const maxVerts = posAttr.count;
        let vi = 0;

        for (const line of lines) {
            const nPts = line.length / 3;
            for (let i = 0; i < nPts - 1 && vi + 2 <= maxVerts; i++) {
                // +VOXEL_CENTER_OFFSET — see header convention note.
                posAttr.array[vi * 3]     = line[i * 3]         + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 1] = line[i * 3 + 1]     + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 2] = line[i * 3 + 2]     + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 3] = line[(i + 1) * 3]     + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 4] = line[(i + 1) * 3 + 1] + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 5] = line[(i + 1) * 3 + 2] + VOXEL_CENTER_OFFSET;

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
    }

    togglePEStreamlines(on) {
        if (!this._peStreamlines) this._buildPEStreamlines();
        this._peStreamlines.visible = on;
        if (!on) this._peStreamlines.geometry.setDrawRange(0, 0);
    }

    // ── Gravity Field Vectors (XZ plane) ──────────────────────────────

    _buildGravityVectors() {
        this._gravityVectors = this._buildArrowFieldMesh(MAX_FIELD_GRID, 0.65);
    }

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
    }

    toggleGravityVectors(on) {
        if (!this._gravityVectors) this._buildGravityVectors();
        this._gravityVectors.visible = on;
        if (!on) this._gravityVectors.geometry.setDrawRange(0, 0);
    }

    // ── Shared mesh-factory helpers (also exposed as public callbacks for
    //    FluxRenderer + ParticleRenderer via Viewport orchestrator) ────

    _buildStreamlineMesh(maxVerts, opacity = 0.7) {
        const positions = new Float32Array(maxVerts * 3);
        const colors = new Float32Array(maxVerts * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity,
            blending: THREE.AdditiveBlending, depthWrite: false,
        });
        const mesh = new THREE.LineSegments(geo, mat);
        mesh.visible = false;
        mesh.frustumCulled = false;
        this._scene.add(mesh);
        return mesh;
    }

    _buildArrowFieldMesh(maxArrows, opacity = 0.7) {
        const positions = new Float32Array(maxArrows * 2 * 3);
        const colors    = new Float32Array(maxArrows * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color',    new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity,
            depthWrite: false,
        });
        const mesh = new THREE.LineSegments(geo, mat);
        mesh.visible = false;
        mesh.frustumCulled = false;
        this._scene.add(mesh);
        return mesh;
    }

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
        const halfN = this._halfN;
        const _needsClip = this._clipActive();
        const [br, bg, bb] = colors.base;
        const [tr, tg, tb] = colors.tip;

        // Gather all active indices that pass threshold and boundary checks
        const activeIndices = [];
        for (let i = 0; i < count; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            activeIndices.push(i);
        }

        const activeCount = activeIndices.length;
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
    }

    _writeStreamlinesIntoMesh(mesh, streamlines, colorFn) {
        this._syncCenterAndRadius();
        const posAttr = mesh.geometry.getAttribute('position');
        const colAttr = mesh.geometry.getAttribute('color');
        const maxVerts = posAttr.array.length / 3;
        const halfN = this._halfN;
        const _needsClip = this._clipActive();
        const rgb = [0, 0, 0];
        let vi = 0;
        for (const line of streamlines) {
            const nPts = line.length / 3;
            for (let i = 0; i < nPts - 1 && vi + 2 <= maxVerts; i++) {
                const sx = line[i * 3], sy = line[i * 3 + 1], sz = line[i * 3 + 2];
                const px = sx + VOXEL_CENTER_OFFSET;
                const py = sy + VOXEL_CENTER_OFFSET;
                const pz = sz + VOXEL_CENTER_OFFSET;
                if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
                colorFn(i, nPts, rgb);
                // +VOXEL_CENTER_OFFSET so the line aligns with particles + flux volume.
                posAttr.array[vi * 3]     = px;
                posAttr.array[vi * 3 + 1] = py;
                posAttr.array[vi * 3 + 2] = pz;
                colAttr.array[vi * 3]     = rgb[0];
                colAttr.array[vi * 3 + 1] = rgb[1];
                colAttr.array[vi * 3 + 2] = rgb[2];
                vi++;
                posAttr.array[vi * 3]     = line[(i + 1) * 3]     + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 1] = line[(i + 1) * 3 + 1] + VOXEL_CENTER_OFFSET;
                posAttr.array[vi * 3 + 2] = line[(i + 1) * 3 + 2] + VOXEL_CENTER_OFFSET;
                colAttr.array[vi * 3]     = rgb[0];
                colAttr.array[vi * 3 + 1] = rgb[1];
                colAttr.array[vi * 3 + 2] = rgb[2];
                vi++;
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        mesh.geometry.setDrawRange(0, vi);
    }

    // Public mesh-factory callbacks (FluxRenderer / ParticleRenderer call these
    // via constructor-injected callbacks routed through the orchestrator).
    buildStreamlineMesh(maxVerts, opacity = 0.7) {
        return this._buildStreamlineMesh(maxVerts, opacity);
    }
    buildArrowFieldMesh(maxArrows, opacity = 0.7) {
        return this._buildArrowFieldMesh(maxArrows, opacity);
    }
    writeArrowFieldIntoMesh(mesh, fieldData, colors, magCacheKey, arrowBase, thresholdFrac) {
        return this._writeArrowFieldIntoMesh(mesh, fieldData, colors, magCacheKey, arrowBase, thresholdFrac);
    }
    writeStreamlinesIntoMesh(mesh, streamlines, colorFn) {
        return this._writeStreamlinesIntoMesh(mesh, streamlines, colorFn);
    }

    // ── E-Field Lines (Cyan) ─────────────────────────────────────────
    _buildEFieldLines() {
        this._eFieldLines = this._buildStreamlineMesh(300 * 160 * 2, 0.7);
    }

    updateEFieldLines(streamlines) {
        if (!this._eFieldLines) this._buildEFieldLines();
        this._writeStreamlinesIntoMesh(this._eFieldLines, streamlines, (i, nPts, rgb) => {
            const alpha = 1.0 - (i / (nPts - 1)) * 0.7;
            rgb[0] = 0.3 * alpha; rgb[1] = 0.82 * alpha; rgb[2] = 0.88 * alpha;
        });
    }

    toggleEFieldLines(on) {
        if (!this._eFieldLines) this._buildEFieldLines();
        this._eFieldLines.visible = on;
        if (!on) this._eFieldLines.geometry.setDrawRange(0, 0);
    }

    // ── B-Field Lines (Green) ────────────────────────────────────────
    _buildBFieldLines() {
        this._bFieldLines = this._buildStreamlineMesh(300 * 240 * 2, 0.7);
    }

    updateBFieldLines(streamlines) {
        if (!this._bFieldLines) this._buildBFieldLines();
        this._writeStreamlinesIntoMesh(this._bFieldLines, streamlines, (i, nPts, rgb) => {
            const alpha = 1.0 - (i / (nPts - 1)) * 0.5;
            rgb[0] = 0.4 * alpha; rgb[1] = 0.73 * alpha; rgb[2] = 0.42 * alpha;
        });
    }

    toggleBFieldLines(on) {
        if (!this._bFieldLines) this._buildBFieldLines();
        this._bFieldLines.visible = on;
        if (!on) this._bFieldLines.geometry.setDrawRange(0, 0);
    }

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
    }

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
        const halfN = this._halfN;
        const arrowBase = 2.0;

        // Gather all active indices
        const activeIndices = [];
        for (let i = 0; i < count; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            activeIndices.push(i);
        }

        const activeCount = activeIndices.length;
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
    }

    togglePoyntingVectors(on) {
        if (!this._poyntingVectors) this._buildPoyntingVectors();
        this._poyntingVectors.visible = on;
        if (!on) this._poyntingVectors.geometry.setDrawRange(0, 0);
    }

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
        geo.setDrawRange(0, 0);
        const mat = new THREE.ShaderMaterial({
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.8 } },
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });
        this._divField = new THREE.Points(geo, mat);
        this._divField.visible = false;
        this._divField.frustumCulled = false;
        this._scene.add(this._divField);
    }

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
        const halfN = this._halfN;

        // Gather active indices
        const activeIndices = [];
        for (let i = 0; i < count; i++) {
            const v = values[i];
            if (Math.abs(v) < threshold) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            activeIndices.push(i);
        }

        const activeCount = activeIndices.length;
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
    }

    toggleDivergenceField(on) {
        if (!this._divField) this._buildDivergenceField();
        this._divField.visible = on;
        if (!on) this._divField.geometry.setDrawRange(0, 0);
    }

    // ── EM Force Volume (Cyan arrows) ────────────────────────────────
    _buildForceVolume() {
        this._forceVolume = this._buildArrowFieldMesh(32768, 0.6);
    }

    updateForceVolume(fieldData) {
        if (!this._forceVolume) this._buildForceVolume();
        this._writeArrowFieldIntoMesh(this._forceVolume, fieldData,
            { base: [0.0, 0.9, 1.0], tip: [0.7, 1.0, 1.0] },
            '_magCache', 1.5, 0.03);
    }

    toggleForceVolume(on) {
        if (!this._forceVolume) this._buildForceVolume();
        this._forceVolume.visible = on;
        if (!on) this._forceVolume.geometry.setDrawRange(0, 0);
    }

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
    }

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
        const halfN = this._halfN;
        const arrowBase = 2.0;

        // Gather all active indices
        const activeIndices = [];
        for (let i = 0; i < count; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;
            activeIndices.push(i);
        }

        const activeCount = activeIndices.length;
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
    }

    toggleGravityField(on) {
        if (!this._gravityField) this._buildGravityField();
        this._gravityField.visible = on;
        if (!on) this._gravityField.geometry.setDrawRange(0, 0);
    }

    // ── EM Force aliases ─────────────────────────────────────────────
    updateEMForceField(data) { this.updateForceVolume(data); }
    showEMForce(on) { this.toggleForceVolume(on); }
    updateGravityForceField(data) { this.updateGravityField(data); }
    showGravityForce(on) { this.toggleGravityField(on); }

    // ── Strong Force Volume (Red arrows) ──────────────────────────────
    _buildStrongForce() {
        this._strongForce = this._buildArrowFieldMesh(32768, 0.7);
    }

    updateStrongForceField(fieldData) {
        if (!this._strongForce) this._buildStrongForce();
        this._writeArrowFieldIntoMesh(this._strongForce, fieldData,
            { base: [1.0, 0.09, 0.27], tip: [1.0, 0.5, 0.5] },
            '_strongMagCache', 1.5, 0.03);
    }

    toggleStrongForce(on) {
        if (!this._strongForce) this._buildStrongForce();
        this._strongForce.visible = on;
        if (!on) this._strongForce.geometry.setDrawRange(0, 0);
    }

    showStrongForce(on) { this.toggleStrongForce(on); }

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
    }

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
        const halfN = this._halfN;
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
    }

    toggleWeakField(on) {
        if (!this._weakField) this._buildWeakField();
        this._weakField.visible = on;
        if (!on) this._weakField.geometry.setDrawRange(0, 0);
    }

    showWeakField(on) { this.toggleWeakField(on); }

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
    }

    initForceHeatmap() { if (!this._forceHeatmap) this._buildForceHeatmap(); }

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
        const halfN = this._halfN;
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
    }

    showForceHeatmap(visible) {
        if (!this._forceHeatmap) this._buildForceHeatmap();
        this._forceHeatmap.visible = visible;
        if (!visible) this._forceHeatmap.geometry.setDrawRange(0, 0);
    }

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
    }

    initForceStreamlines() { if (!this._forceStreamlinePool) this._buildForceStreamlines(); }

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
        const drawnCounts = this._forceStreamlineDrawn ||
            (this._forceStreamlineDrawn = new Int32Array(pool.length).fill(-1));
        const usedCount = Math.min(lines.length, pool.length);
        for (let li = 0; li < usedCount; li++) {
            const verts = lines[li];
            const line = pool[li];
            const posAttr = line.geometry.getAttribute('position');
            const maxVerts = posAttr.array.length / 3;
            const vertCount = Math.min(verts.length / 3, maxVerts);

            const arr = posAttr.array;
            const n3 = vertCount * 3;
            let changed = drawnCounts[li] !== vertCount;
            if (!changed) {
                for (let v = 0; v < n3; v++) {
                    if (arr[v] !== verts[v]) { changed = true; break; }
                }
            }
            if (changed) {
                for (let v = 0; v < n3; v++) {
                    arr[v] = verts[v];
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
    }

    animateForceStreamlines(dt) {
        if (!this._forceStreamlineMats) return;
        const speed = 2.0;
        for (let i = 0; i < this._forceStreamlineCount; i++) {
            this._forceStreamlineMats[i].dashOffset -= speed * dt;
        }
    }

    showForceStreamlines_vis(visible) {
        if (!this._forceStreamlinePool) this._buildForceStreamlines();
        for (let i = 0; i < this._forceStreamlinePool.length; i++) {
            if (!visible) this._forceStreamlinePool[i].visible = false;
        }
    }

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
    }

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
    }

    _buildForceGlyphs() { this._ensureForceGlyphInfra(); }
    initForceGlyphs() { this._ensureForceGlyphInfra(); }

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
        const halfN = this._halfN;
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
    }

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
    }

    hideAllForceStyles() {
        if (this._forceVolume) this._forceVolume.visible = false;
        if (this._gravityField) this._gravityField.visible = false;
        if (this._strongForce) this._strongForce.visible = false;
        if (this._weakField) this._weakField.visible = false;
        this.showForceHeatmap(false);
        this.showForceStreamlines_vis(false);
        this.showForceGlyphs(false);
    }

    showArrowForces(fieldState) {
        if (fieldState.showForceEM) this.toggleForceVolume(true);
        if (fieldState.showForceGravity) this.toggleGravityField(true);
        if (fieldState.showForceStrong) this.toggleStrongForce(true);
        if (fieldState.showForceWeak) this.toggleWeakField(true);
    }

    // ── Dark Matter Halo Overlay (sub-threshold flux envelope) ──────
    _buildDarkMatterHalo() {
        const maxPts = 8000;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        geo.setDrawRange(0, 0);
        const mat = new THREE.ShaderMaterial({
            vertexShader: PARTICLE_VERT, fragmentShader: PARTICLE_FRAG,
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.35 } },
            transparent: true, blending: THREE.AdditiveBlending,
            depthWrite: false,
        });
        this._darkMatterHalo = new THREE.Points(geo, mat);
        this._darkMatterHalo.visible = false;
        this._darkMatterHalo.frustumCulled = false;
        this._darkMatterHalo.renderOrder = 1;
        this._scene.add(this._darkMatterHalo);
    }

    updateDarkMatterHalo(particles, fluxMag, latticeSize) {
        this._syncCenterAndRadius();
        if (!this._darkMatterHalo) this._buildDarkMatterHalo();
        const posAttr = this._darkMatterHalo.geometry.getAttribute('position');
        const colAttr = this._darkMatterHalo.geometry.getAttribute('particleColor');
        const sizeAttr = this._darkMatterHalo.geometry.getAttribute('size');
        const N = latticeSize;
        const kGen = K_GENESIS; // 3 * K_B = 1.533 (audit P2-9 fix: import the named constant, 2026-05-27)
        let vi = 0;
        const maxPts = 8000;

        const step = N > 64 ? 4 : (N > 24 ? 2 : 1);
        for (let z = 0; z < N && vi < maxPts; z += step) {
            for (let y = 0; y < N && vi < maxPts; y += step) {
                for (let x = 0; x < N && vi < maxPts; x += step) {
                    const idx = z * N * N + y * N + x;
                    const mag = fluxMag[idx];
                    if (mag > 0.003 && mag < kGen) {
                        const t = mag / kGen;
                        posAttr.array[vi * 3]     = x + 0.5;
                        posAttr.array[vi * 3 + 1] = y + 0.5;
                        posAttr.array[vi * 3 + 2] = z + 0.5;
                        colAttr.array[vi * 3] = 0.3 + t * 0.4;
                        colAttr.array[vi * 3 + 1] = 0.1 + t * 0.15;
                        colAttr.array[vi * 3 + 2] = 0.5 + t * 0.4;
                        sizeAttr.array[vi] = (1.0 + 4.0 * t) * step;
                        vi++;
                    }
                }
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._darkMatterHalo.geometry.setDrawRange(0, vi);
    }

    toggleDarkMatterHalo(on) {
        if (!this._darkMatterHalo) this._buildDarkMatterHalo();
        this._darkMatterHalo.visible = on;
        if (!on) this._darkMatterHalo.geometry.setDrawRange(0, 0);
    }

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
    }

    setEventHorizon(active, radius) {
        if (!this._eventHorizonSphere) this._buildEventHorizon();
        if (active && radius > 0) {
            const isOrigin = this._getBoundaryMode && this._getBoundaryMode() === 'origin';
            const cx = isOrigin ? 0.0 : this._center;
            const cy = isOrigin ? 0.0 : this._center;
            const cz = isOrigin ? 0.0 : this._center;
            this._eventHorizonSphere.position.set(cx, cy, cz);
            this._eventHorizonRing.position.set(cx, cy, cz);
            this._eventHorizonSphere.scale.setScalar(radius);
            this._eventHorizonSphere.visible = true;
            this._eventHorizonRing.scale.setScalar(radius * 3.0);
            this._eventHorizonRing.visible = true;
        } else {
            this._eventHorizonSphere.visible = false;
            this._eventHorizonRing.visible = false;
        }
    }

    // ── Selective Damping Zones (wireframe cubes around damped voxels) ─
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
    }

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

        for (const p of particles) {
            if (si >= 1200) break;
            const cx = p.x + 0.5, cy = p.y + 0.5, cz = p.z + 0.5;
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
    }

    toggleDampingZones(on) {
        if (!this._dampingZones) this._buildDampingZones();
        this._dampingZones.visible = on;
        if (!on) this._dampingZones.geometry.setDrawRange(0, 0);
    }

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
        geo.setDrawRange(0, 0);
        const mat = new THREE.ShaderMaterial({
            vertexShader: PARTICLE_VERT, fragmentShader: PARTICLE_FRAG,
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.45 } },
            transparent: true, blending: THREE.AdditiveBlending,
            depthWrite: false,
        });
        this._genesisIsosurface = new THREE.Points(geo, mat);
        this._genesisIsosurface.visible = false;
        this._genesisIsosurface.frustumCulled = false;
        this._genesisIsosurface.renderOrder = 1;
        this._scene.add(this._genesisIsosurface);
    }

    updateGenesisIsosurface(fluxMag, latticeSize, kGenesis) {
        this._syncCenterAndRadius();
        if (!this._genesisIsosurface) this._buildGenesisIsosurface();
        const posAttr = this._genesisIsosurface.geometry.getAttribute('position');
        const colAttr = this._genesisIsosurface.geometry.getAttribute('particleColor');
        const sizeAttr = this._genesisIsosurface.geometry.getAttribute('size');
        const N = latticeSize;
        let vi = 0;
        const band = kGenesis * 0.15;

        const step = N > 64 ? 4 : (N > 24 ? 2 : 1);

        for (let z = 0; z < N && vi < 4000; z += step) {
            for (let y = 0; y < N && vi < 4000; y += step) {
                for (let x = 0; x < N && vi < 4000; x += step) {
                    const mag = fluxMag[z * N * N + y * N + x];
                    const dist = Math.abs(mag - kGenesis);
                    if (dist < band && mag > 0.01) {
                        const t = 1.0 - dist / band;
                        posAttr.array[vi * 3]     = x + 0.5;
                        posAttr.array[vi * 3 + 1] = y + 0.5;
                        posAttr.array[vi * 3 + 2] = z + 0.5;
                        colAttr.array[vi * 3] = 0.15 + t * 0.15;
                        colAttr.array[vi * 3 + 1] = 0.7 + t * 0.3;
                        colAttr.array[vi * 3 + 2] = 0.2 + t * 0.15;
                        sizeAttr.array[vi] = (1.5 + 4.0 * t) * step;
                        vi++;
                    }
                }
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._genesisIsosurface.geometry.setDrawRange(0, vi);
    }

    toggleGenesisIsosurface(on) {
        if (!this._genesisIsosurface) this._buildGenesisIsosurface();
        this._genesisIsosurface.visible = on;
        if (!on) this._genesisIsosurface.geometry.setDrawRange(0, 0);
    }

    // ── Confinement Strings (SU(3) 1D topological defects) ───────────
    _buildConfinementStrings() {
        const maxVerts = 400 * 2;
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
    }

    updateConfinementStrings(bridge) {
        if (!this._confinementStrings) this._buildConfinementStrings();

        const posAttr = this._confinementStrings.geometry.getAttribute('position');
        const colAttr = this._confinementStrings.geometry.getAttribute('color');
        const maxVerts = posAttr.array.length / 3;

        let vi = 0;
        const kb = bridge.getParam ? bridge.getParam('kb') : K_B;
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
        const keyOf = (cx, cy, cz) => cx + ',' + cy + ',' + cz;
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
    }

    toggleConfinement(on) {
        if (!this._confinementStrings) this._buildConfinementStrings();
        this._confinementStrings.visible = on;
        if (!on) this._confinementStrings.geometry.setDrawRange(0, 0);
    }

    // ── Dual Substrate Volume (Warm L / Cool R) ─────────────────────
    _buildDualFluxVolume() {
        const maxPts = 8000;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        geo.setDrawRange(0, 0);
        const mat = new THREE.ShaderMaterial({
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.7 } },
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });
        this._dualFluxVolume = new THREE.Points(geo, mat);
        this._dualFluxVolume.visible = false;
        this._dualFluxVolume.frustumCulled = false;
        this._scene.add(this._dualFluxVolume);
    }

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

        const halfN = this._halfN;
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
    }

    toggleDualFluxVolume(on) {
        if (!this._dualFluxVolume) this._buildDualFluxVolume();
        this._dualFluxVolume.visible = on;
        if (!on) this._dualFluxVolume.geometry.setDrawRange(0, 0);
    }

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
        geo.setDrawRange(0, 0);
        const mat = new THREE.ShaderMaterial({
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.7 } },
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });
        this._chiralityField = new THREE.Points(geo, mat);
        this._chiralityField.visible = false;
        this._chiralityField.frustumCulled = false;
        this._scene.add(this._chiralityField);
    }

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
        const halfN = this._halfN;
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
    }

    toggleChiralityField(on) {
        if (!this._chiralityField) this._buildChiralityField();
        this._chiralityField.visible = on;
        if (!on) this._chiralityField.geometry.setDrawRange(0, 0);
    }

    // ── Light Field (warm yellow glow from |Poynting|) ─────────────
    _buildLightField() {
        const maxPts = 5000;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        geo.setDrawRange(0, 0);
        const mat = new THREE.ShaderMaterial({
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.8 } },
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });
        this._lightField = new THREE.Points(geo, mat);
        this._lightField.visible = false;
        this._lightField.frustumCulled = false;
        this._scene.add(this._lightField);
    }

    updateLightField(poyntingData) {
        this._syncCenterAndRadius();
        if (!this._lightField) this._buildLightField();
        const posAttr = this._lightField.geometry.getAttribute('position');
        const colAttr = this._lightField.geometry.getAttribute('particleColor');
        const sizeAttr = this._lightField.geometry.getAttribute('size');
        const { positions, vectors, count } = poyntingData;
        const maxPts = posAttr.array.length / 3;

        let maxMag = 0;
        for (let i = 0; i < count; i++) {
            const sx = vectors[i * 3], sy = vectors[i * 3 + 1], sz = vectors[i * 3 + 2];
            const m = Math.sqrt(sx * sx + sy * sy + sz * sz);
            if (m > maxMag) maxMag = m;
        }
        const threshold = maxMag * 0.03;
        const halfN = this._halfN;
        const _needsClip = this._clipActive();
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const sx = vectors[i * 3], sy = vectors[i * 3 + 1], sz = vectors[i * 3 + 2];
            const mag = Math.sqrt(sx * sx + sy * sy + sz * sz);
            if (mag < threshold) continue;

            const px = positions[i * 3] + VOXEL_CENTER_OFFSET, py = positions[i * 3 + 1] + VOXEL_CENTER_OFFSET, pz = positions[i * 3 + 2] + VOXEL_CENTER_OFFSET;
            if (_needsClip && !this._insideBoundary((px - this._center) / this._radius, (py - this._center) / this._radius, (pz - this._center) / this._radius)) continue;

            posAttr.array[vi * 3] = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;

            const t = mag / maxMag;
            colAttr.array[vi * 3] = 1.0 * t;
            colAttr.array[vi * 3 + 1] = 0.92 * t;
            colAttr.array[vi * 3 + 2] = 0.23 * t;
            sizeAttr.array[vi] = 1.5 + 4.5 * t;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._lightField.geometry.setDrawRange(0, vi);
    }

    toggleLightField(on) {
        if (!this._lightField) this._buildLightField();
        this._lightField.visible = on;
        if (!on) this._lightField.geometry.setDrawRange(0, 0);
    }

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
    }

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
    }

    _quantumSetVisibility() {
        if (!this._quantumField) return;
        const pointCloudOn = !!(this._psi2Visible || this._lagrangianVisible || this._entropyVisible);
        this._quantumField.visible = pointCloudOn;
        if (!pointCloudOn) this._quantumField.geometry.setDrawRange(0, 0);
    }

    _populateQuantumField(data, kind, options = {}) {
        this._syncCenterAndRadius();
        if (!this._quantumField) this._buildQuantumField();
        if (!data || !data.positions || !data.values || !data.count) return;
        const posAttr = this._quantumField.geometry.getAttribute('position');
        const colAttr = this._quantumField.geometry.getAttribute('color');
        const maxPts = posAttr.array.length / 3;
        const halfN = this._halfN;
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
    }

    // ── |ψ|² Born density ─────────────────────────────────────────────
    togglePsiSquaredField(on) {
        this._psi2Visible = !!on;
        if (!this._quantumField) this._buildQuantumField();
        this._quantumSetVisibility();
    }
    updatePsiSquaredField(data) {
        this._psi2Data = data;
        if (!this._psi2Visible) return;
        this._populateQuantumField(data, 'psi2', {
            signed: false,
            ramp: (t, out, i) => rampViridis(t, out, i),
            normalizer: data?.normalizer,
        });
    }

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
    }

    togglePhaseField(on) {
        this._phaseVisible = !!on;
        if (!this._phaseNeedles) this._buildPhaseNeedles();
        this._phaseNeedles.visible = !!on;
        if (!on) this._phaseNeedles.geometry.setDrawRange(0, 0);
        this._quantumSetVisibility();
    }

    updatePhaseField(data) {
        this._syncCenterAndRadius();
        this._phaseData = data;
        if (!this._phaseVisible || !data?.count) return;
        if (!this._phaseNeedles) this._buildPhaseNeedles();
        const posAttr = this._phaseNeedles.geometry.getAttribute('position');
        const colAttr = this._phaseNeedles.geometry.getAttribute('color');
        const maxSegments = posAttr.array.length / 6;
        const halfN = this._halfN;
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
    }

    // ── ℒ(x) Lagrangian density ──────────────────────────────────────
    toggleLagrangianDensityField(on) {
        this._lagrangianVisible = !!on;
        if (!this._quantumField) this._buildQuantumField();
        this._quantumSetVisibility();
    }
    updateLagrangianDensityField(data) {
        this._lagrangianData = data;
        if (!this._lagrangianVisible) return;
        this._populateQuantumField(data, 'lagrangian', {
            signed: true,
            ramp: (t, out, i) => rampDivergingRdBu(t, out, i),
            normalizer: data?.normalizer,
            threshold: 0.10,
        });
    }

    // ── Entropy s(x) — jittering sparkles ─────────────────────────────
    toggleEntropyDensityField(on) {
        this._entropyVisible = !!on;
        if (!this._quantumField) this._buildQuantumField();
        this._entropyJitterSeed = Date.now();
        this._quantumSetVisibility();
    }
    updateEntropyDensityField(data) {
        this._syncCenterAndRadius();
        this._entropyData = data;
        if (!this._entropyVisible) return;
        if (!this._quantumField) this._buildQuantumField();
        const posAttr = this._quantumField.geometry.getAttribute('position');
        const colAttr = this._quantumField.geometry.getAttribute('color');
        const maxPts = posAttr.array.length / 3;
        const halfN = this._halfN;
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
    }

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
    }

    toggleHorizonField(on) {
        if (!this._horizonField) this._buildHorizonField();
        this._horizonField.points.visible = !!on;
    }

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
    }

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

    // ══════════════════════════════════════════════════════════════════
    // ── Disposal ──────────────────────────────────────────────────────
    dispose() {
        const disposeMesh = (obj) => {
            if (!obj) return;
            this._scene.remove(obj);
            if (obj.geometry) obj.geometry.dispose();
            if (obj.material) {
                if (obj.material.map) obj.material.map.dispose();
                obj.material.dispose();
            }
        };

        // Simple geometry+material pairs (Points / LineSegments / Mesh).
        const simpleMeshFields = [
            '_fieldHeatmap', '_fieldVectors', '_peStreamlines', '_gravityVectors',
            '_eFieldLines', '_bFieldLines', '_poyntingVectors', '_divField',
            '_forceVolume', '_gravityField', '_strongForce', '_weakField',
            '_forceHeatmap',
            '_darkMatterHalo', '_dampingZones', '_genesisIsosurface',
            '_confinementStrings',
            '_dualFluxVolume', '_chiralityField', '_lightField',
            '_quantumField', '_phaseNeedles',
            '_eventHorizonSphere', '_eventHorizonRing',
        ];
        for (const name of simpleMeshFields) {
            disposeMesh(this[name]);
            this[name] = null;
        }

        // Per-force glyph meshes (one InstancedMesh per force type).
        if (this._forceGlyphMeshes) {
            for (const m of Object.values(this._forceGlyphMeshes)) disposeMesh(m);
            this._forceGlyphMeshes = null;
        }

        // Force streamline pool (array of Line objects).
        if (this._forceStreamlinePool) {
            for (const line of this._forceStreamlinePool) disposeMesh(line);
            this._forceStreamlinePool = null;
            this._forceStreamlineMats = null;
        }

        // Horizon field (wraps a Points object plus metadata).
        if (this._horizonField) {
            disposeMesh(this._horizonField.points);
            this._horizonField = null;
        }

        // Quantum scaffolding texture (instance-owned, not the static
        // _softSpriteTexture used by weak-field).
        if (this._softDiscTex) {
            this._softDiscTex.dispose();
            this._softDiscTex = null;
        }
    }

    destroy(ctx) {
        this.dispose();
    }
}
