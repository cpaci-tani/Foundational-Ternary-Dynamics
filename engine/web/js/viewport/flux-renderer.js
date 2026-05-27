/**
 * @file engine/web/js/viewport/flux-renderer.js
 * @purpose Owns flux volume, flux streamlines for the Scale-0 lattice
 *          dashboard. One of 4 sub-renderers extracted from the
 *          monolithic Viewport class in Phase 3 of the refactor sweep.
 * @consumers engine/web/js/viewport.js (composes this via constructor)
 * @contract CONTRACTS.md §2 (Capability Factory Contract — applies to
 *          any sub-renderer with onLatticeSizeChanged/dispose lifecycle)
 * @related ./scene-core.js (3a, sibling), ./field-renderer.js
 *          (3c, owns _fieldHeatmap which updateFluxSlice writes — stays
 *          on orchestrator), ./particle-renderer.js (3d, sibling),
 *          ./REFACTOR_MAP.md (extraction guide)
 *
 * Phase 3b of refactor sweep. updateFluxSlice / toggleFluxSlice REMAIN
 * on the Viewport orchestrator — they write _fieldHeatmap which
 * Phase 3c FieldRenderer will own. The cross-cutting concern is
 * documented in REFACTOR_MAP.md and CONTRACTS.md §2.
 *
 * Imports:
 *   - FLUX_VOL_VERT, PARTICLE_FRAG (shaders) — keep imports same as viewport.js
 *   - fluxToColorInto from ../color-ramps.js
 *   - THREE
 *
 * Helper-method note (2026-04-28 Phase 3b extraction):
 *   `_buildStreamlineMesh` and `_writeStreamlinesIntoMesh` remain on the
 *   Viewport orchestrator because Phase 3c FieldRenderer also uses them
 *   (E-field, B-field, PE streamlines). They are passed in as
 *   `buildStreamlineMesh` / `writeStreamlinesIntoMesh` callbacks. When 3c
 *   lands, those helpers can move to FieldRenderer or to a shared
 *   `viewport/_mesh-factories.js` and the callbacks re-wired here.
 */

import * as THREE from 'three';
import { fluxToColorInto, fluxToColor } from '../fields.js';

// Flux-volume variant: sqrt depth scaling instead of linear 1/z.
// (Identical to viewport.js's FLUX_VOL_VERT — duplicated here so this
// module is self-contained.)
const FLUX_VOL_VERT = `
    attribute float size;
    attribute vec3 particleColor;
    varying vec3 vColor;
    varying float vSize;

    void main() {
        vColor = particleColor;
        vSize = size;
        vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
        float depth = max(-mvPosition.z, 0.1);
        gl_PointSize = size * sqrt(60.0 / depth);
        gl_PointSize = clamp(gl_PointSize, 1.0, 512.0);
        gl_Position = projectionMatrix * mvPosition;
    }
`;

import { PARTICLE_FRAG } from './shaders.js';


const MAX_FIELD_GRID = 16384;

export class ViewportFluxRenderer {
    constructor({
        scene,
        latticeSize,
        halfN,
        boundaryShape,
        insideBoundary,
        applyScenarioScale,
        buildStreamlineMesh,
        writeStreamlinesIntoMesh,
    }) {
        this._scene = scene;
        this._latticeSize = latticeSize;
        this._halfN = halfN;
        this._boundaryShape = boundaryShape;
        this._insideBoundary = insideBoundary;
        this._applyScenarioScale = applyScenarioScale;
        this._buildStreamlineMesh = buildStreamlineMesh;
        this._writeStreamlinesIntoMesh = writeStreamlinesIntoMesh;

        // State owned by FluxRenderer (moved from Viewport's constructor)
        this._fluxVolume = null;
        this._fluxVolumeSize = 0;
        this._fluxStreamlines = null;
        this._fluxPointScale = 1.0;
        this._fluxThreshold = 0.005;
        this._scenarioScale = 1.0;
        this._fluxLatticeSpacing = 1.0;
        this.showFlux = true;  // flux volume ON by default
    }

    setBoundaryShape(shape) {
        this._boundaryShape = shape;
    }

    onLatticeSizeChanged(size, halfN) {
        this._latticeSize = size;
        this._halfN = halfN;
        // Rebuild flux volume for new size (mirrors viewport.js setLatticeSize behaviour).
        if (this._fluxVolume) {
            this._scene.remove(this._fluxVolume);
            this._fluxVolume.geometry.dispose();
            this._fluxVolume.material.dispose();
            this._fluxVolume = null;
            this._fluxVolumeSize = 0;
        }
        // Clear stale flux-streamlines draw range so old-L data doesn't persist.
        if (this._fluxStreamlines && this._fluxStreamlines.geometry) {
            this._fluxStreamlines.geometry.setDrawRange(0, 0);
        }
    }

    // ── Flux Volume Rendering (Scale 0 -- substrate mode) ──────────────
    // Renders the continuous flux field J as sparse point cloud.
    // Each voxel above threshold emits a colored dot sized by magnitude.
    // Subsampling tiers: step=1 for L<=48, step=2 for L<=96, step=4 for L>96.
    // Boundary clipping uses _insideBoundary() for non-cube shapes.

    _buildFluxVolume(latticeSize) {
        // Compute the subsampled grid dimension to determine buffer capacity.
        // Subsampling mirrors updateFluxVolume: step=4 for L>96, step=2 for L>48, else 1.
        const step = latticeSize > 96 ? 4 : (latticeSize > 48 ? 2 : 1);
        const sampledN = Math.ceil(latticeSize / step);
        const maxPts = sampledN * sampledN * sampledN;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);

        const geo = new THREE.BufferGeometry();
        const posAttr = new THREE.Float32BufferAttribute(positions, 3);
        const colAttr = new THREE.Float32BufferAttribute(colors, 3);
        const sizeAttr = new THREE.Float32BufferAttribute(sizes, 1);
        posAttr.setUsage(THREE.DynamicDrawUsage);
        colAttr.setUsage(THREE.DynamicDrawUsage);
        sizeAttr.setUsage(THREE.DynamicDrawUsage);
        geo.setAttribute('position', posAttr);
        geo.setAttribute('particleColor', colAttr);
        geo.setAttribute('size', sizeAttr);
        geo.setDrawRange(0, 0);

        const mat = new THREE.ShaderMaterial({
            vertexShader: FLUX_VOL_VERT,
            fragmentShader: PARTICLE_FRAG,
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.7 } },
            transparent: true,
            depthWrite: false,
            depthTest: true,
            blending: THREE.NormalBlending,
        });

        this._fluxVolume = new THREE.Points(geo, mat);
        this._fluxVolume.visible = false;
        this._fluxVolume.frustumCulled = false; // skip bounding sphere recompute for dynamic geometry
        this._fluxVolume.renderOrder = 10; // render after background stars (order 0)
        this._fluxVolumeSize = latticeSize;
        this._scene.add(this._fluxVolume);
    }

    /**
     * Update flux volume rendering from a flat Float64Array of flux magnitudes.
     * ALL voxels are rendered: inactive ones as tiny dark dots, active ones with
     * flux-driven color and size (blue→cyan→white→yellow→red).
     * @param {Float64Array} volumeData — N^3 flux magnitudes in x-fastest order
     * @param {number} latticeSize — side length N
     */
    updateFluxVolume(volumeData, latticeSize) {
        // Rebuild if missing or if lattice size changed (buffer capacity depends on L)
        if (!this._fluxVolume || this._fluxVolumeSize !== latticeSize) {
            if (this._fluxVolume) {
                this._scene.remove(this._fluxVolume);
                this._fluxVolume.geometry.dispose();
                this._fluxVolume.material.dispose();
                this._fluxVolume = null;
            }
            this._buildFluxVolume(latticeSize);
            // _buildFluxVolume initialises visible=false; restore the user's current
            // showFlux state so the volume doesn't disappear after a size change.
            this._fluxVolume.visible = this.showFlux;
            // Re-apply spacing if configured
            if (this._fluxLatticeSpacing !== 1.0) {
                this.setFluxLatticeSpacing(this._fluxLatticeSpacing);
            }
        }

        const posAttr = this._fluxVolume.geometry.getAttribute('position');
        const colAttr = this._fluxVolume.geometry.getAttribute('particleColor');
        const sizeAttr = this._fluxVolume.geometry.getAttribute('size');
        const N = latticeSize;

        // Early exit if no data
        if (!volumeData || volumeData.length === 0) {
            this._fluxVolume.geometry.setDrawRange(0, 0);
            return;
        }

        // Find max for normalization
        let maxFlux = 0;
        const total = N * N * N;
        for (let i = 0; i < total; i++) {
            if (volumeData[i] > maxFlux) maxFlux = volumeData[i];
        }

        // Skip full scan if field is essentially zero
        if (maxFlux < 1e-20) {
            this._fluxVolume.geometry.setDrawRange(0, 0);
            return;
        }

        // Render every voxel — base dots + flux-driven glow
        // Clip to boundary shape (normalized coords -1..1 from lattice center)
        let count = 0;
        const maxPts = posAttr.array.length / 3;
        const MAX_SIZE = (this._fluxPointScale || 1.0) * 10.0;
        const FLUX_THRESHOLD = this._fluxThreshold !== undefined ? this._fluxThreshold : 0.005;
        const halfN = N / 2;

        // Subsample for large lattices to maintain interactive frame rates:
        //   L<=48:  step=1  → up to 48^3 = 110K points
        //   L<=96:  step=2  → up to 48^3 = 110K points (from 96^3)
        //   L>96:   step=4  → up to 32^3 =  32K points (from 128^3)
        const step = N > 96 ? 4 : (N > 48 ? 2 : 1);

        // PERF: hoist boundary-shape check OUT of the per-voxel loop. For the
        // default 'cube'/'none' boundary _insideBoundary() always returns
        // true, but the function-call overhead alone costs ~100K calls per
        // upload at L=64. Skip the call (and the nx/ny/nz division) entirely
        // when no clipping is needed.
        const _bs = this._boundaryShape;
        const needsClip = !(_bs === 'cube' || _bs === 'none' || _bs === undefined);

        // PERF: cache geometry attribute backing arrays as locals so the JIT
        // can keep them in registers. posArr/colArr/sizeArr writes dominate
        // the hot loop.
        const posArr = posAttr.array;
        const colArr = colAttr.array;
        const sizeArr = sizeAttr.array;

        for (let z = 0; z < N && count < maxPts; z += step) {
            const zNN = z * N * N;
            for (let y = 0; y < N && count < maxPts; y += step) {
                const zNNyN = zNN + y * N;
                for (let x = 0; x < N && count < maxPts; x += step) {
                    if (needsClip) {
                        const nx = (x - halfN + 0.5) / halfN;
                        const ny = (y - halfN + 0.5) / halfN;
                        const nz = (z - halfN + 0.5) / halfN;
                        if (!this._insideBoundary(nx, ny, nz)) continue;
                    }

                    const mag = volumeData[zNNyN + x];

                    // Skip inactive voxels before writing any attributes,
                    // otherwise stale color/size from a prior frame leak through
                    if (mag < FLUX_THRESHOLD) continue;

                    const c3 = count * 3;
                    // +0.5: render at unit-cell centre so voxel 0 sits at 0.5
                    // and voxel N-1 at N-0.5 — perfectly filling the [0,N] wireframe.
                    posArr[c3]     = x + 0.5;
                    posArr[c3 + 1] = y + 0.5;
                    posArr[c3 + 2] = z + 0.5;

                    // PERF: in-place colormap write. Pre-fix this allocated a
                    // fresh [r,g,b] array per voxel -- ~1.8M allocs/sec at L=32.
                    fluxToColorInto(colArr, c3, mag, maxFlux);

                    const t = mag / (maxFlux + 1e-20);
                    const sizeScale = step > 1 ? step * 0.8 : 1.0; // compensate for subsampling
                    sizeArr[count] = (1.0 + (MAX_SIZE - 1.0) * t) * sizeScale;

                    count++;
                }
            }
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._fluxVolume.geometry.setDrawRange(0, count);
    }

    toggleFluxVolume(on) {
        if (!this._fluxVolume) this._buildFluxVolume(this._latticeSize);
        this._fluxVolume.visible = on;
        this.showFlux = on;
        if (!on) this._fluxVolume.geometry.setDrawRange(0, 0);
    }

    // ── Flux Volume Controls ──────────────────────────────────────────

    setFluxOpacity(val) {
        if (!this._fluxVolume) return;
        this._fluxVolume.material.uniforms.uOpacity.value = val;
    }

    setFluxShape(shapeIndex) {
        if (!this._fluxVolume) return;
        this._fluxVolume.material.uniforms.shapeType.value = shapeIndex;
    }

    setFluxPointScale(scale) {
        // Store scale factor; applied in updateFluxVolume via _fluxPointScale
        this._fluxPointScale = scale;
    }

    setFluxThreshold(val) {
        // Store threshold; applied in updateFluxVolume
        this._fluxThreshold = val;
    }

    setScenarioScale(scale) {
        this._scenarioScale = scale;
        this._applyScenarioScale();
    }

    /**
     * Visual-only spacing multiplier for the flux-volume point cloud.
     * Does NOT affect physics (dx stays 1 voxel). Multiplies the mesh's
     * local scale so the rendered lattice can be spread out or packed in
     * for pedagogy, while sampling/thresholds still key off integer
     * voxel positions.
     */
    setFluxLatticeSpacing(val) {
        this._fluxLatticeSpacing = val;
        if (this._fluxVolume) {
            const s = val || 1.0;
            const N = this._latticeSize || 32;
            // Re-centre so the expanded/contracted cloud stays visually
            // anchored on the original lattice origin.
            const offset = (1 - s) * N / 2;
            this._fluxVolume.scale.setScalar(s);
            this._fluxVolume.position.set(offset, offset, offset);
        }
    }

    // ── Flux Streamlines (flux colormap) ─────────────────────────────
    _buildFluxStreamlines() {
        // Same cap as E-field (matching maxSteps profile — see field-overlays.js).
        this._fluxStreamlines = this._buildStreamlineMesh(300 * 160 * 2, 0.7);
    }

    updateFluxStreamlines(streamlines, maxFluxMag) {
        if (!this._fluxStreamlines) this._buildFluxStreamlines();
        const maxMag = maxFluxMag || 1;
        this._writeStreamlinesIntoMesh(this._fluxStreamlines, streamlines, (i, nPts, rgb) => {
            const t = i / (nPts - 1);
            const [r, g, b] = fluxToColor(t * maxMag, maxMag);
            rgb[0] = r; rgb[1] = g; rgb[2] = b;
        });
    }

    toggleFluxStreamlines(on) {
        if (!this._fluxStreamlines) this._buildFluxStreamlines();
        this._fluxStreamlines.visible = on;
        if (!on) this._fluxStreamlines.geometry.setDrawRange(0, 0);
    }

    dispose() {
        if (this._fluxVolume) {
            this._scene.remove(this._fluxVolume);
            if (this._fluxVolume.geometry) this._fluxVolume.geometry.dispose();
            if (this._fluxVolume.material) this._fluxVolume.material.dispose();
            this._fluxVolume = null;
            this._fluxVolumeSize = 0;
        }
        if (this._fluxStreamlines) {
            this._scene.remove(this._fluxStreamlines);
            if (this._fluxStreamlines.geometry) this._fluxStreamlines.geometry.dispose();
            if (this._fluxStreamlines.material) this._fluxStreamlines.material.dispose();
            this._fluxStreamlines = null;
        }
    }

    destroy(ctx) {
        this.dispose();
    }
}
