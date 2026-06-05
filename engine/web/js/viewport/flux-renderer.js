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

// Flux-volume vertex shader (sqrt depth scaling) — centralized in
// viewport/shaders.js (D-1).
import { FLUX_VOL_VERT, PARTICLE_FRAG } from './shaders.js';

// (MAX_FIELD_GRID was declared here but never referenced — flux volume
// buffers size from lattice³, not the field-grid cap. Removed under D-6;
// the canonical constant now lives in viewport/constants.js.)

// Flux-volume sampling — adaptive point budget with a FRACTIONAL stride (2026-06-04).
// The draw loop emits a fixed `samples`-per-axis grid spread evenly across the lattice
// (stride = N/samples ≥ 1), so the rendered cloud has ~constant density at EVERY size.
// This replaces the earlier integer-step (1/2/4) subsample, whose tier boundaries left
// awkwardly sparse sizes — e.g. N=65 dropped to ⅛ density just past the step 1→2 jump.
// For N ≤ FLUX_MAX_AXIS_POINTS every voxel is drawn (stride exactly 1, no regression at
// small L); above it `samples` saturates and the stride grows continuously. Worst-case
// drawn/buffer count is samples³ ≤ FLUX_MAX_AXIS_POINTS³ ≈ 149K. Used by BOTH
// _buildFluxVolume (buffer sizing) and updateFluxVolume (scan + write) — they MUST
// share this or the write loop would over/under-run the geometry buffer.
// Raise FLUX_MAX_AXIS_POINTS for a denser cloud (and a larger geometry buffer).
const FLUX_MAX_AXIS_POINTS = 53;   // 53³ ≈ 148.9K-point worst-case budget
function fluxVolumeAxisSamples(N) {
    return Math.min(N, FLUX_MAX_AXIS_POINTS);
}

// Dim-dot floor size, in units of the grid spacing (`stride`). The flux volume is a soft
// round point cloud; if the dimmest dots are smaller than the inter-sample spacing the
// regular grid shows through as a "lattice of cubes". A floor of a few spacings makes even
// low-flux dots overlap into a continuous haze (high-flux dots grow on top, up to the
// fluxPointScale·10 ceiling). Tunable: raise for a smoother/denser cloud, lower for crisper
// individual dots.
const FLUX_DOT_MIN = 2.4;

// Flux-volume glow presets, toggled by setFluxGlow(). ON = additive bloom (weakened
// from the original — it was too strong); OFF = flat normal-blended translucent dots.
const FLUX_GLOW_UGLOW    = 0.06;   // gaussian halo intensity when glow on
const FLUX_GLOW_UOPACITY = 0.34;   // per-dot opacity when glow on (additive)
const FLUX_FLAT_UOPACITY = 0.60;   // per-dot opacity when glow off (normal blending)

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
        this.showFlux = true;      // flux volume ON by default
        this._fluxOrganic = true;  // organic (3D-jittered scatter) vs regular lattice grid
        this._fluxGlow = true;     // additive glow bloom (weakened) vs flat translucent dots
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
    // Sampling: a fixed fractional-stride grid (fluxVolumeAxisSamples per axis) so the
    // cloud has ~constant density at every L — full detail up to FLUX_MAX_AXIS_POINTS.
    // Boundary clipping uses _insideBoundary() for non-cube shapes.

    _buildFluxVolume(latticeSize) {
        // Buffer capacity = the fractional-stride sample grid (samples per axis), via
        // the shared fluxVolumeAxisSamples() helper so it matches exactly what the
        // updateFluxVolume write loop will emit (≤ samples³ points).
        const sampledN = fluxVolumeAxisSamples(latticeSize);
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

        // Glow ON = additive blend so overlapping soft dots ACCUMULATE into a continuous
        // luminous volume (uGlow adds a gaussian halo past each dot's core); OFF = normal
        // blend, flat translucent dots. depthWrite off so the cloud is order-independent.
        const glow = this._fluxGlow;
        const mat = new THREE.ShaderMaterial({
            vertexShader: FLUX_VOL_VERT,
            fragmentShader: PARTICLE_FRAG,
            uniforms: {
                shapeType: { value: 0 },
                uOpacity: { value: glow ? FLUX_GLOW_UOPACITY : FLUX_FLAT_UOPACITY },
                uGlow: { value: glow ? FLUX_GLOW_UGLOW : 0.0 },
            },
            transparent: true,
            depthWrite: false,
            depthTest: true,
            blending: glow ? THREE.AdditiveBlending : THREE.NormalBlending,
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

        const total = N * N * N;
        if (volumeData.length !== total) {
            // Size mismatch (e.g. during async resize transition or startup lag) — skip rendering this frame
            return;
        }

        // Sample grid — `samples` points per axis (stride = N/samples ≥ 1). The DOTS are
        // rendered at evenly-spaced stratum centres ((i+0.5)·stride apart, exactly uniform)
        // and each dot reads its field value from the NEAREST voxel. Rendering at the even
        // positions — rather than at the floor()-snapped voxel — is what kills the uneven
        // "blocks": floor(i·stride) at a fractional stride bunches voxels 1,1,1,2,… into
        // visible bands, but the even render grid has no beat. Collapses to exact voxel
        // centres when stride==1 (N≤53). vox[] caches the nearest-voxel index per sample.
        const samples = fluxVolumeAxisSamples(N);
        const stride = N / samples;   // ≥ 1; exactly 1 when N ≤ FLUX_MAX_AXIS_POINTS
        if (!this._fluxVox || this._fluxVox.length < samples) {
            this._fluxVox = new Int32Array(FLUX_MAX_AXIS_POINTS);
        }
        const vox = this._fluxVox;
        for (let i = 0; i < samples; i++) {
            const v = ((i + 0.5) * stride) | 0;   // nearest voxel to the stratum centre
            vox[i] = v < N ? v : N - 1;
        }

        // Find max for normalization over the sampled grid.
        let maxFlux = 0;
        for (let iz = 0; iz < samples; iz++) {
            const zNN = vox[iz] * N * N;
            for (let iy = 0; iy < samples; iy++) {
                const zNNyN = zNN + vox[iy] * N;
                for (let ix = 0; ix < samples; ix++) {
                    const m = volumeData[zNNyN + vox[ix]];
                    if (m > maxFlux) maxFlux = m;
                }
            }
        }

        // Skip the write loop if the field is essentially zero.
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

        // The write loop draws dots at evenly-spaced render positions ((i+0.5)·stride),
        // each reading the nearest voxel (vox[], same as the scan) — uniform, no beat.

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

        // Jitter amplitude: 0 at stride==1 (N≤53 stays an exact voxel grid), else scatter
        // each dot inside its stride-wide cell so the regular subsample grid never aligns
        // into rows / rings / rays (the additive-blend moiré). 3D-hashed per (ix,iy,iz) so
        // it is deterministic (no per-frame shimmer) and breaks ALL planar alignment —
        // unlike a per-axis jitter, which leaves shared sheets and reads as plaid.
        const jamp = (this._fluxOrganic && stride > 1.0001) ? stride : 0;
        for (let iz = 0; iz < samples && count < maxPts; iz++) {
            const zNN = vox[iz] * N * N;
            const ze = (iz + 0.5) * stride;
            for (let iy = 0; iy < samples && count < maxPts; iy++) {
                const zNNyN = zNN + vox[iy] * N;
                const ye = (iy + 0.5) * stride;
                for (let ix = 0; ix < samples && count < maxPts; ix++) {
                    const mag = volumeData[zNNyN + vox[ix]];

                    // Skip inactive voxels before writing any attributes,
                    // otherwise stale color/size from a prior frame leak through
                    if (mag < FLUX_THRESHOLD) continue;

                    // Stable 3D sub-cell offsets in [-0.5,0.5)·jamp → organic scatter.
                    let h = (ix * 92837111) ^ (iy * 689287499) ^ (iz * 283923481);
                    h = (h ^ (h >>> 15)) >>> 0;
                    const xr = (ix + 0.5) * stride + ((h & 1023) / 1024 - 0.5) * jamp;
                    const yr = ye + (((h >>> 10) & 1023) / 1024 - 0.5) * jamp;
                    const zr = ze + (((h >>> 20) & 1023) / 1024 - 0.5) * jamp;

                    if (needsClip) {
                        const center = N / 2;
                        const radius = N / 2;
                        const nx = (xr - center) / radius;
                        const ny = (yr - center) / radius;
                        const nz = (zr - center) / radius;
                        if (!this._insideBoundary(nx, ny, nz)) continue;
                    }

                    const c3 = count * 3;
                    // Jittered render position — organic scatter, no grid/moiré. Field
                    // value (mag/color) comes from the nearest even-grid voxel (vox[]).
                    posArr[c3]     = xr;
                    posArr[c3 + 1] = yr;
                    posArr[c3 + 2] = zr;

                    // PERF: in-place colormap write. Pre-fix this allocated a
                    // fresh [r,g,b] array per voxel -- ~1.8M allocs/sec at L=32.
                    fluxToColorInto(colArr, c3, mag, maxFlux);

                    const t = mag / (maxFlux + 1e-20);
                    // Floor the dim dots at FLUX_DOT_MIN·stride so they tile the grid
                    // spacing (no visible lattice); high-flux dots grow up to MAX_SIZE·stride.
                    const lo = FLUX_DOT_MIN * stride;
                    const hi = Math.max(MAX_SIZE, FLUX_DOT_MIN) * stride;
                    sizeArr[count] = lo + (hi - lo) * t;

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

    // Organic (3D-jittered scatter) vs regular lattice grid. Changes dot POSITIONS, so the
    // caller must trigger a re-upload (latticeNeedsUpload) for it to take effect.
    setFluxOrganic(on) {
        this._fluxOrganic = !!on;
    }

    // Additive glow bloom vs flat translucent dots. Swaps the material blend + uniforms
    // live (picked up on the next render — no re-upload needed).
    setFluxGlow(on) {
        this._fluxGlow = !!on;
        if (!this._fluxVolume) return;
        const mat = this._fluxVolume.material;
        mat.blending = this._fluxGlow ? THREE.AdditiveBlending : THREE.NormalBlending;
        mat.uniforms.uOpacity.value = this._fluxGlow ? FLUX_GLOW_UOPACITY : FLUX_FLAT_UOPACITY;
        mat.uniforms.uGlow.value = this._fluxGlow ? FLUX_GLOW_UGLOW : 0.0;
        mat.needsUpdate = true;
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
