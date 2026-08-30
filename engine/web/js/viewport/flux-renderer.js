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
import { FLUX_VOL_VERT, PARTICLE_FRAG, PARTICLE_SHADER_UNIFORMS } from './shaders.js';

// (MAX_FIELD_GRID was declared here but never referenced — flux volume
// buffers size from lattice³, not the field-grid cap. Removed under D-6;
// the canonical constant now lives in viewport/constants.js.)

// Flux-volume presentation budget. This is renderer-only decimation: physics,
// telemetry, panel samplers, and the shared dense/SAB volume remain untouched.
// A 53³ translucent point cloud produced ~12 FPS under browser SwiftShader at
// L=97 even while paused; 20³ retained the spatial envelope at 60 FPS. Native
// FTV2 may still publish as many as 53 samples/axis, so source acceptance and
// render density are deliberately separate constants. The 12³ ceiling keeps
// robust headroom for the concurrently active sidepanel and base scene on the
// software-renderer fallback as well as hardware WebGL.
const FLUX_SOURCE_MAX_AXIS_POINTS = 53;
const FLUX_RENDER_MAX_AXIS_POINTS = 12; // 12³ = 1,728 point-sprite ceiling
const FLUX_RENDER_MAX_POINTS = FLUX_RENDER_MAX_AXIS_POINTS ** 3;
function fluxVolumeAxisSamples(N) {
    return Math.min(N, FLUX_RENDER_MAX_AXIS_POINTS);
}

// Dim-dot floor size, in units of the bounded visual footprint stride. The flux volume is a soft
// round point cloud; if the dimmest dots are smaller than the inter-sample spacing the
// regular grid shows through as a "lattice of cubes". A floor of a few spacings makes even
// low-flux dots overlap into a continuous haze (high-flux dots grow on top, up to the
// fluxPointScale·10 ceiling). Tunable: raise for a smoother/denser cloud, lower for crisper
// individual dots.
const FLUX_DOT_MIN = 2.4;
// Decimation increases physical spacing between retained samples. Scaling the
// sprite footprint by that full spacing negates the point-count win through
// translucent fragment overdraw, so cap the visual footprint independently.
const FLUX_POINT_FOOTPRINT_MAX_STRIDE = 1.5;

// Flux-volume glow presets, toggled by setFluxGlow(). ON = additive bloom (weakened
// from the original — it was too strong); OFF = flat normal-blended translucent dots.
const FLUX_GLOW_UGLOW    = 0.06;   // gaussian halo intensity when glow on
const FLUX_GLOW_UOPACITY = 0.34;   // per-dot opacity when glow on (additive)
const FLUX_FLAT_UOPACITY = 0.60;   // per-dot opacity when glow off (normal blending)
const FLUX_FLOW_LINE_MAX_VERTS = 16000;

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
        this._fluxStreamlinesRequested = false;
        this._flowLineOpacity = 0.7;
        this._fluxPointScale = 1.0;
        this._fluxThreshold = 0.005;
        this._scenarioScale = 1.0;
        this._fluxLatticeSpacing = 1.0;
        this.showFlux = true;      // flux volume ON by default
        this._fluxOrganic = true;  // organic (3D-jittered scatter) vs regular lattice grid
        this._fluxGlow = true;     // additive glow bloom (weakened) vs flat translucent dots
        this._fluxOpacity = null;  // user opacity override (null = use the glow-mode default)
        this._fluxShape = 0;       // point shape (0 = circle)

        // Peak-hold-with-decay normalizer state for Flux Volume (audit fix —
        // dynamical accuracy). Colour/size normalization used to divide by THIS
        // FRAME's own instant max |J|, which stretches a trivial field and an
        // extreme field into the identical color range and hides whether the
        // field is growing or decaying. This tracks a VU-meter-style running
        // peak instead: fast attack (jumps immediately to a new instant max),
        // slow release (decays geometrically when the instant max isn't
        // re-hit), so a decaying field visibly fades over ~seconds instead of
        // snapping back to full saturation every frame.
        this._fluxMaxDecay = 0;

        // Reused one-maximum-per-spatial-stratum pool. The source-volume scan
        // is O(source voxels), while every allocation and every GPU write stays
        // bounded by FLUX_RENDER_MAX_POINTS (1,728).
        this._fluxPoolMagnitude = new Float64Array(FLUX_RENDER_MAX_POINTS);
        this._fluxPoolSourceIndex = new Int32Array(FLUX_RENDER_MAX_POINTS);
        this._fluxPoolAxisMap = null;
        this._fluxPoolAxisMapSourceN = 0;
        this._fluxPoolAxisMapSamples = 0;
    }

    // Peak-hold-with-decay update, instance-local (see the constructor
    // comment). Not shared with overlay-frames.js's updateDecayingMax helper —
    // this is a different module (the Three.js renderer, not the JS field
    // sampler).
    _updatePeakHoldDecay(fieldName, instantMax, decay = 0.985) {
        const prev = this[fieldName] || 0;
        const next = Math.max(instantMax, prev * decay);
        this[fieldName] = next;
        return next;
    }

    /** Reset visual normalization at an authoritative scenario/resize boundary. */
    resetFluxNormalization() {
        this._fluxMaxDecay = 0;
    }

    setBoundaryShape(shape) {
        this._boundaryShape = shape;
    }

    onLatticeSizeChanged(size, halfN) {
        this._latticeSize = size;
        this._halfN = halfN;
        this.resetFluxNormalization();
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
            this._fluxStreamlines.visible = false;
        }
    }

    // ── Flux Volume Rendering (Scale 0 -- substrate mode) ──────────────
    // Renders the continuous flux field J as sparse point cloud.
    // Each voxel above threshold emits a colored dot sized by magnitude.
    // Sampling: one maximum representative per bounded uniform 3D stratum.
    // Boundary clipping uses _insideBoundary() for non-cube shapes.

    _buildFluxVolume(latticeSize) {
        // Buffer capacity matches the bounded stratum grid exactly.
        const sampledN = fluxVolumeAxisSamples(latticeSize);
        const maxPts = sampledN * sampledN * sampledN;
        const positions = new Float32Array(maxPts * 3);
        const sourcePositions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);
        const manifestPhases = new Float32Array(maxPts);
        const manifestRates = new Float32Array(maxPts);

        const geo = new THREE.BufferGeometry();
        const posAttr = new THREE.Float32BufferAttribute(positions, 3);
        const sourcePosAttr = new THREE.Float32BufferAttribute(sourcePositions, 3);
        const colAttr = new THREE.Float32BufferAttribute(colors, 3);
        const sizeAttr = new THREE.Float32BufferAttribute(sizes, 1);
        const phaseAttr = new THREE.Float32BufferAttribute(manifestPhases, 1);
        const rateAttr = new THREE.Float32BufferAttribute(manifestRates, 1);
        posAttr.setUsage(THREE.DynamicDrawUsage);
        sourcePosAttr.setUsage(THREE.DynamicDrawUsage);
        colAttr.setUsage(THREE.DynamicDrawUsage);
        sizeAttr.setUsage(THREE.DynamicDrawUsage);
        phaseAttr.setUsage(THREE.DynamicDrawUsage);
        rateAttr.setUsage(THREE.DynamicDrawUsage);
        geo.setAttribute('position', posAttr);
        // Scientific coordinate of the maximum-magnitude source sample chosen
        // for each stratum. `position` may be presentation-jittered in Organic
        // mode; sourcePosition always retains the physical sample coordinate.
        geo.setAttribute('sourcePosition', sourcePosAttr);
        geo.setAttribute('particleColor', colAttr);
        geo.setAttribute('size', sizeAttr);
        geo.setAttribute('manifestPhase', phaseAttr);
        geo.setAttribute('manifestRate', rateAttr);
        geo.setDrawRange(0, 0);

        // Glow ON = additive blend so overlapping soft dots ACCUMULATE into a continuous
        // luminous volume (uGlow adds a gaussian halo past each dot's core); OFF = normal
        // blend, flat translucent dots. depthWrite off so the cloud is order-independent.
        const glow = this._fluxGlow;
        const mat = new THREE.ShaderMaterial({
            vertexShader: FLUX_VOL_VERT,
            fragmentShader: PARTICLE_FRAG,
            uniforms: {
                ...PARTICLE_SHADER_UNIFORMS,
                uOpacity: { value: glow ? FLUX_GLOW_UOPACITY : FLUX_FLAT_UOPACITY },
                uGlow: { value: glow ? FLUX_GLOW_UGLOW : 0.0 },
                uManifestEnabled: { value: 0 },
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
        // Re-apply every persisted setting so a rebuild (resize / scenario / toggle) keeps
        // the user's flux-volume settings continuous instead of resetting them to the
        // freshly-built material/mesh defaults.
        this._fluxVolume.visible = this.showFlux;
        this._applyFluxMaterialState();                       // glow + opacity + shape
        if (this._fluxLatticeSpacing !== 1.0) this.setFluxLatticeSpacing(this._fluxLatticeSpacing);
    }

    /**
     * Update flux volume rendering from either a legacy dense N^3 magnitude
     * array or a native FTV2 descriptor:
     *   { data, latticeSize, stride, axisCount }
     * Both layouts are x-fastest. FTV2 is already sampled on the GPU/server,
     * avoiding a full N^3 device-to-host copy and socket payload at large L.
     * @param {Float32Array|Float64Array|{data:Float32Array,latticeSize:number,stride:number,axisCount:number}} volumeData
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
            // _buildFluxVolume now restores visible + re-applies every persisted material
            // and spacing setting at its tail, so nothing extra is needed here.
        }

        const posAttr = this._fluxVolume.geometry.getAttribute('position');
        const sourcePosAttr = this._fluxVolume.geometry.getAttribute('sourcePosition');
        const colAttr = this._fluxVolume.geometry.getAttribute('particleColor');
        const sizeAttr = this._fluxVolume.geometry.getAttribute('size');
        const N = latticeSize;

        const compact = volumeData && !ArrayBuffer.isView(volumeData)
            && ArrayBuffer.isView(volumeData.data) ? volumeData : null;
        const density = compact ? compact.data : volumeData;

        // Early exit if no data.
        if (!density || density.length === 0) {
            this._fluxVolume.geometry.setDrawRange(0, 0);
            return;
        }

        let samples;
        let renderSpacing;
        let sourceN;
        let compactSpacing = 1;
        let compactOrigin = 0;
        if (compact) {
            sourceN = Math.trunc(Number(compact.axisCount));
            compactSpacing = Number(compact.stride);
            compactOrigin = Number.isFinite(Number(compact.origin))
                ? Number(compact.origin)
                : 0;
            const compactCount = sourceN * sourceN * sourceN;
            if (Math.trunc(Number(compact.latticeSize)) !== N
                || sourceN < 1 || sourceN > FLUX_SOURCE_MAX_AXIS_POINTS
                || !Number.isFinite(compactSpacing) || compactSpacing < 1
                || density.length !== compactCount) {
                // Async resize transition or malformed descriptor: retain the
                // previous valid draw until the matching cache arrives.
                return;
            }
            samples = fluxVolumeAxisSamples(sourceN);
            renderSpacing = compactSpacing * (sourceN / samples);
        } else {
            const total = N * N * N;
            if (density.length !== total) {
                // Size mismatch during an async resize/startup transition.
                return;
            }
            samples = fluxVolumeAxisSamples(N);
            sourceN = N;
            renderSpacing = N / samples;
        }

        // Uniformly partition all source samples into at most 12³ spatial
        // strata, retaining exactly the maximum-magnitude source sample from
        // each. Unlike nearest-stride sampling, this cannot miss a localized
        // center or off-stride feature. The pool and GPU write remain bounded
        // at 1,728 representatives for both dense and compact inputs.
        if (!this._fluxPoolAxisMap
            || this._fluxPoolAxisMapSourceN !== sourceN
            || this._fluxPoolAxisMapSamples !== samples) {
            this._fluxPoolAxisMap = new Uint8Array(sourceN);
            this._fluxPoolAxisMapSourceN = sourceN;
            this._fluxPoolAxisMapSamples = samples;
            for (let i = 0; i < sourceN; i++) {
                this._fluxPoolAxisMap[i] = Math.min(
                    samples - 1,
                    Math.floor((i * samples) / sourceN),
                );
            }
        }
        const axisMap = this._fluxPoolAxisMap;
        const poolMagnitude = this._fluxPoolMagnitude;
        const poolSourceIndex = this._fluxPoolSourceIndex;
        const samplePlane = samples * samples;
        const poolCount = samplePlane * samples;
        poolMagnitude.fill(0, 0, poolCount);
        poolSourceIndex.fill(-1, 0, poolCount);

        // For shaped boundaries, pool only scientifically drawable source
        // samples. Otherwise a larger out-of-bound value could win a stratum,
        // be clipped later, and erase a smaller in-bound localized feature.
        const _bs = this._boundaryShape;
        const needsClip = !(_bs === 'cube' || _bs === 'none' || _bs === undefined);
        const boundaryCenter = N / 2;
        const boundaryRadius = N / 2;
        const compactCoord = (axisIndex) => Math.max(
            0,
            Math.min(compactOrigin + axisIndex * compactSpacing, N - 1),
        ) + 0.5;

        let instantMaxFlux = 0;
        let sourceIndex = 0;
        for (let z = 0; z < sourceN; z++) {
            const poolZ = axisMap[z] * samplePlane;
            for (let y = 0; y < sourceN; y++) {
                const poolZY = poolZ + axisMap[y] * samples;
                for (let x = 0; x < sourceN; x++, sourceIndex++) {
                    const mag = density[sourceIndex];
                    // Flux volume is a magnitude channel: ignore invalid and
                    // non-positive samples rather than poisoning normalization.
                    if (!(mag > 0 && mag < Infinity)) continue;
                    if (needsClip) {
                        const physicalX = compact ? compactCoord(x) : x + 0.5;
                        const physicalY = compact ? compactCoord(y) : y + 0.5;
                        const physicalZ = compact ? compactCoord(z) : z + 0.5;
                        if (!this._insideBoundary(
                            (physicalX - boundaryCenter) / boundaryRadius,
                            (physicalY - boundaryCenter) / boundaryRadius,
                            (physicalZ - boundaryCenter) / boundaryRadius,
                        )) continue;
                    }
                    const poolIndex = poolZY + axisMap[x];
                    if (mag > poolMagnitude[poolIndex]) {
                        poolMagnitude[poolIndex] = mag;
                        poolSourceIndex[poolIndex] = sourceIndex;
                    }
                    if (mag > instantMaxFlux) instantMaxFlux = mag;
                }
            }
        }

        // Skip the write loop if the field is essentially zero THIS FRAME (an
        // elevated held peak from earlier should not force an empty field to
        // keep drawing dots — nothing would pass FLUX_THRESHOLD below anyway).
        if (instantMaxFlux < 1e-20) {
            this._fluxVolume.geometry.setDrawRange(0, 0);
            return;
        }

        // Peak-hold-with-decay: color/size normalize against the held running
        // peak, not this instant's own max, so a decaying field visibly fades
        // instead of being re-stretched to fill the ramp every frame.
        const maxFlux = this._updatePeakHoldDecay('_fluxMaxDecay', instantMaxFlux);

        // Render every retained sample — base dots + flux-driven glow.
        // Clip to boundary shape (normalized coords -1..1 from lattice center)
        let count = 0;
        const maxPts = posAttr.array.length / 3;
        const MAX_SIZE = (this._fluxPointScale || 1.0) * 10.0;
        const FLUX_THRESHOLD = this._fluxThreshold !== undefined ? this._fluxThreshold : 0.005;
        // The write loop emits each stratum's maximum representative. Organic
        // mode may jitter the presentation position, while sourcePosition keeps
        // the exact winning physical coordinate.

        // PERF: cache geometry attribute backing arrays as locals so the JIT
        // can keep them in registers. posArr/colArr/sizeArr writes dominate
        // the hot loop.
        const posArr = posAttr.array;
        const sourcePosArr = sourcePosAttr.array;
        const colArr = colAttr.array;
        const sizeArr = sizeAttr.array;

        // Jitter amplitude: when Organic is on, scatter each dot inside its render cell;
        // when off, 0 → exact grid. The 3D hash per (ix,iy,iz) breaks ALL planar alignment
        // (the additive-blend moiré / blocks) — unlike a per-axis jitter, which leaves
        // shared sheets and reads as plaid — and is deterministic (no per-frame shimmer).
        const jamp = this._fluxOrganic ? renderSpacing : 0;
        const footprintStride = Math.min(
            renderSpacing,
            FLUX_POINT_FOOTPRINT_MAX_STRIDE,
        );
        const sourcePlane = sourceN * sourceN;
        for (let poolIndex = 0; poolIndex < poolCount && count < maxPts; poolIndex++) {
            const mag = poolMagnitude[poolIndex];
            const retainedSourceIndex = poolSourceIndex[poolIndex];
            if (retainedSourceIndex < 0 || mag < FLUX_THRESHOLD) continue;

            const sourceZ = Math.floor(retainedSourceIndex / sourcePlane);
            const sourceRem = retainedSourceIndex - sourceZ * sourcePlane;
            const sourceY = Math.floor(sourceRem / sourceN);
            const sourceX = sourceRem - sourceY * sourceN;
            const physicalX = compact ? compactCoord(sourceX) : sourceX + 0.5;
            const physicalY = compact ? compactCoord(sourceY) : sourceY + 0.5;
            const physicalZ = compact ? compactCoord(sourceZ) : sourceZ + 0.5;

            const stratumZ = Math.floor(poolIndex / samplePlane);
            const stratumRem = poolIndex - stratumZ * samplePlane;
            const stratumY = Math.floor(stratumRem / samples);
            const stratumX = stratumRem - stratumY * samples;
            let h = (stratumX * 92837111)
                ^ (stratumY * 689287499)
                ^ (stratumZ * 283923481);
            h = (h ^ (h >>> 15)) >>> 0;
            const xr = Math.max(0.5, Math.min(
                N - 0.5,
                physicalX + ((h & 1023) / 1024 - 0.5) * jamp,
            ));
            const yr = Math.max(0.5, Math.min(
                N - 0.5,
                physicalY + (((h >>> 10) & 1023) / 1024 - 0.5) * jamp,
            ));
            const zr = Math.max(0.5, Math.min(
                N - 0.5,
                physicalZ + (((h >>> 20) & 1023) / 1024 - 0.5) * jamp,
            ));

            const c3 = count * 3;
            posArr[c3] = xr;
            posArr[c3 + 1] = yr;
            posArr[c3 + 2] = zr;
            sourcePosArr[c3] = physicalX;
            sourcePosArr[c3 + 1] = physicalY;
            sourcePosArr[c3 + 2] = physicalZ;

            // PERF: in-place colormap write. Pre-fix this allocated a fresh
            // [r,g,b] array per voxel.
            fluxToColorInto(colArr, c3, mag, maxFlux);

            const t = mag / (maxFlux + 1e-20);
            const lo = FLUX_DOT_MIN * footprintStride;
            const hi = Math.max(MAX_SIZE, FLUX_DOT_MIN) * footprintStride;
            sizeArr[count] = lo + (hi - lo) * t;
            count++;
        }

        posAttr.needsUpdate = true;
        sourcePosAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._fluxVolume.geometry.setDrawRange(0, count);
    }

    toggleFluxVolume(on) {
        const next = !!on;
        this.showFlux = next;
        if (!this._fluxVolume) { if (!next) return; this._buildFluxVolume(this._latticeSize); }
        if (this._fluxVolume.visible === next) return;
        this._fluxVolume.visible = next;
        if (!next) this._fluxVolume.geometry.setDrawRange(0, 0);
    }

    // ── Flux Volume Controls ──────────────────────────────────────────

    setFluxOpacity(val) {
        if (this._fluxOpacity === val) return;
        this._fluxOpacity = val;   // persisted; re-applied on every (re)build
        if (this._fluxVolume) this._fluxVolume.material.uniforms.uOpacity.value = val;
    }

    setFluxShape(shapeIndex) {
        const shape = shapeIndex | 0;
        if (this._fluxShape === shape) return;
        this._fluxShape = shape;   // persisted; re-applied on every (re)build
        if (this._fluxVolume) this._fluxVolume.material.uniforms.shapeType.value = this._fluxShape;
    }

    setFluxPointScale(scale) {
        // Store scale factor; applied in updateFluxVolume via _fluxPointScale
        if (this._fluxPointScale === scale) return;
        this._fluxPointScale = scale;
    }

    setFluxThreshold(val) {
        // Store threshold; applied in updateFluxVolume
        if (this._fluxThreshold === val) return;
        this._fluxThreshold = val;
    }

    // Organic (3D-jittered scatter) vs regular lattice grid. Changes dot POSITIONS, so the
    // caller must trigger a re-upload (latticeNeedsUpload) for it to take effect.
    setFluxOrganic(on) {
        const next = !!on;
        if (this._fluxOrganic === next) return;
        this._fluxOrganic = next;
    }

    // Additive glow bloom vs flat translucent dots. Swaps the material blend + uniforms
    // live (picked up on the next render — no re-upload needed).
    setFluxGlow(on) {
        const next = !!on;
        if (this._fluxGlow === next) return;
        this._fluxGlow = next;
        this._applyFluxMaterialState();
    }

    // Single source of truth for the material-driven flux-volume settings — glow blend +
    // halo, opacity (the user's override if set, else the glow-mode default), and point
    // shape. Called by setFluxGlow AND at the tail of every (re)build, so user settings
    // stay continuous instead of resetting to the freshly-built material's defaults.
    _applyFluxMaterialState() {
        if (!this._fluxVolume) return;
        const mat = this._fluxVolume.material;
        const glow = this._fluxGlow;
        mat.blending = glow ? THREE.AdditiveBlending : THREE.NormalBlending;
        mat.uniforms.uGlow.value = glow ? FLUX_GLOW_UGLOW : 0.0;
        mat.uniforms.uOpacity.value = (this._fluxOpacity != null)
            ? this._fluxOpacity
            : (glow ? FLUX_GLOW_UOPACITY : FLUX_FLAT_UOPACITY);
        mat.uniforms.shapeType.value = this._fluxShape | 0;
        mat.needsUpdate = true;
    }

    setScenarioScale(scale) {
        if (this._scenarioScale === scale) return;
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
        if (this._fluxLatticeSpacing === val) return;
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
        // Same audited cap as E-field (matching maxSteps profile).
        this._fluxStreamlines = this._buildStreamlineMesh(FLUX_FLOW_LINE_MAX_VERTS, this._flowLineOpacity);
    }

    // `mags` (optional) is a flat per-vertex |J| magnitude array parallel to
    // `streamlines.buffer` (one scalar per vertex, i.e. index = offsets[li]/3 + i)
    // — built by buildFluxStreamlines in scale0/runtime/field-overlays.js by
    // sampling the same field buffer used to integrate the lines. Coloring by
    // this LOCAL magnitude (rather than by i/(nPts-1), the vertex's arc-length
    // position along its own line) makes the ramp mean the same thing here as
    // it does on Flux Volume: blue=weak, red=strong AT THAT POINT (audit fix —
    // the old position-based coloring was a real mismatch, not a stylistic
    // choice). Falls back to the old position-based fade if `mags` is absent.
    updateFluxStreamlines(streamlines, maxFluxMag, mags) {
        if (!this._fluxStreamlines) this._buildFluxStreamlines();
        const maxMag = maxFluxMag || 1;
        const offsets = streamlines.offsets;
        this._writeStreamlinesIntoMesh(this._fluxStreamlines, streamlines, (i, nPts, rgb, li) => {
            let mag;
            if (mags) {
                mag = mags[(offsets[li] / 3) + i];
            } else {
                mag = (i / (nPts - 1)) * maxMag;
            }
            const [r, g, b] = fluxToColor(mag, maxMag);
            rgb[0] = r; rgb[1] = g; rgb[2] = b;
        });
        this._fluxStreamlines.visible = this._fluxStreamlinesRequested
            && this._fluxStreamlines.geometry.drawRange.count > 0;
    }

    toggleFluxStreamlines(on) {
        const next = !!on;
        this._fluxStreamlinesRequested = next;
        if (!this._fluxStreamlines) { if (!next) return; this._buildFluxStreamlines(); }
        if (!next) {
            this._fluxStreamlines.visible = false;
            if (this._fluxStreamlines.geometry.drawRange.count !== 0) {
                this._fluxStreamlines.geometry.setDrawRange(0, 0);
            }
            return;
        }
        const drawable = this._fluxStreamlines.geometry.drawRange.count > 0;
        if (this._fluxStreamlines.visible !== drawable) this._fluxStreamlines.visible = drawable;
    }

    setFlowLineOpacity(value) {
        const numeric = Number(value);
        if (!Number.isFinite(numeric)) return;
        const next = Math.max(0, Math.min(1, numeric));
        if (this._flowLineOpacity === next) return;
        this._flowLineOpacity = next;
        if (this._fluxStreamlines?.material) this._fluxStreamlines.material.opacity = next;
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
