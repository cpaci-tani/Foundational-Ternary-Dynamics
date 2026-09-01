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
import { clampFluxThreshold } from './flux-threshold.js';
import { computeFluxActivation, createFluxActivationStepper } from './flux-activation.js';

// (MAX_FIELD_GRID was declared here but never referenced — flux volume
// buffers size from lattice³, not the field-grid cap. Removed under D-6;
// the canonical constant now lives in viewport/constants.js.)

// Native FTV2 may publish up to 53 samples/axis. This is a transport validation
// ceiling, not renderer decimation: every sample present in either the dense
// WASM volume or compact native descriptor is evaluated independently.
const FLUX_SOURCE_MAX_AXIS_POINTS = 53;
const FLUX_LATTICE_MIN_POINT_SIZE = 1.0;
const FLUX_LATTICE_INSPECTION_COLOR_FLOOR = [0.16, 0.35, 0.55];
function fluxVolumeAxisSamples(N) {
    return Math.min(N, FLUX_SOURCE_MAX_AXIS_POINTS);
}

// Dim-dot floor size, in units of the bounded visual footprint stride. The flux volume is a soft
// round point cloud; if the dimmest dots are smaller than the inter-sample spacing the
// regular grid shows through as a "lattice of cubes". A floor of a few spacings makes even
// low-flux dots overlap into a continuous haze (high-flux dots grow on top, up to the
// fluxPointScale·10 ceiling). Tunable: raise for a smoother/denser cloud, lower for crisper
// individual dots.
const FLUX_POINT_FOOTPRINT_MAX_STRIDE = 1.5;

// Flux-volume glow presets, toggled by setFluxGlow(). ON = additive bloom (weakened
// from the original — it was too strong); OFF = flat normal-blended translucent dots.
const FLUX_GLOW_UGLOW    = 0.06;   // gaussian halo intensity when glow on
const FLUX_GLOW_UOPACITY = 0.34;   // per-dot opacity when glow on (additive)
const FLUX_FLAT_UOPACITY = 0.60;   // per-dot opacity when glow off (normal blending)
const FLUX_FLOW_LINE_MAX_VERTS = 16000;
// Above this support size, activation and attribute writes are cooperatively
// sliced across animation frames. L=97 contains 912,673 voxels; processing
// that complete support synchronously creates a ~30 ms main-thread stall.
const FLUX_ASYNC_SOURCE_COUNT = 200000;
const FLUX_ASYNC_FRAME_BUDGET_MS = 4.0;

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
        this._fluxVolumeAxisCapacity = 0;
        this._fluxStreamlines = null;
        this._fluxStreamlinesRequested = false;
        this._flowLineOpacity = 0.7;
        this._fluxPointScale = 1.0;
        this._fluxThreshold = 0.005;
        this._scenarioScale = 1.0;
        this._fluxLatticeSpacing = 1.0;
        this.showFlux = true;      // flux volume ON by default
        this._fluxOrganic = false; // regular lattice by default; Organic explicitly enables jitter
        this._fluxGlow = true;     // additive glow bloom (weakened) vs flat translucent dots
        this._fluxOpacity = null;  // user opacity override (null = use the glow-mode default)
        this._fluxShape = 0;       // point shape (0 = circle)

        // Peak-hold-with-decay normalizer for the visualization-only local
        // activation amplitude. Fast attack and slow release let a decaying
        // field visibly fade instead of restretching every frame.
        this._fluxMaxDecay = 0;

        // Reused per-source buffers for the visualization-only activation
        // proxy. Their length always matches the complete received source
        // grid; threshold never changes capacity or coordinates.
        this._fluxStateMask = new Uint8Array(0);
        this._fluxActivation = new Float64Array(0);
        this._fluxActivationScratchA = new Float64Array(0);
        this._fluxActivationScratchB = new Float64Array(0);
        this._fluxDensitySnapshot = new Float64Array(0);
        this._fluxAsyncJob = null;
        this._fluxPendingFrame = null;
        this._fluxAsyncRaf = 0;
        this._fluxPositionSignature = '';
        this._fluxVisibleCount = 0;
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
        this._cancelFluxAsyncUpdate();
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
            this._fluxVolumeAxisCapacity = 0;
            this._fluxPositionSignature = '';
        }
        // Clear stale flux-streamlines draw range so old-L data doesn't persist.
        if (this._fluxStreamlines && this._fluxStreamlines.geometry) {
            this._fluxStreamlines.geometry.setDrawRange(0, 0);
            this._fluxStreamlines.visible = false;
        }
    }

    // ── Flux Volume Rendering (Scale 0 -- substrate mode) ──────────────
    // Renders the continuous flux field J as a point cloud.
    // Every available voxel is evaluated. A voxel above the selected relative
    // activation-energy threshold emits a dot; stronger activation grows its
    // size and advances its colour phase.
    // Boundary clipping uses _insideBoundary() for non-cube shapes.

    _buildFluxVolume(latticeSize, axisCapacity = fluxVolumeAxisSamples(latticeSize)) {
        // Allocate the complete received source grid. Threshold changes draw
        // count only; it never changes this capacity or its coordinate support.
        const sampledN = Math.max(1, Math.trunc(axisCapacity));
        const maxPts = sampledN * sampledN * sampledN;
        const positions = new Float32Array(maxPts * 3);
        const sourcePositions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);
        const manifestPhases = new Float32Array(maxPts);
        const manifestRates = new Float32Array(maxPts);
        const visibilities = new Float32Array(maxPts);
        visibilities.fill(1);

        const geo = new THREE.BufferGeometry();
        const posAttr = new THREE.Float32BufferAttribute(positions, 3);
        const sourcePosAttr = new THREE.Float32BufferAttribute(sourcePositions, 3);
        const colAttr = new THREE.Float32BufferAttribute(colors, 3);
        const sizeAttr = new THREE.Float32BufferAttribute(sizes, 1);
        const phaseAttr = new THREE.Float32BufferAttribute(manifestPhases, 1);
        const rateAttr = new THREE.Float32BufferAttribute(manifestRates, 1);
        const visibilityAttr = new THREE.Float32BufferAttribute(visibilities, 1);
        posAttr.setUsage(THREE.DynamicDrawUsage);
        sourcePosAttr.setUsage(THREE.DynamicDrawUsage);
        colAttr.setUsage(THREE.DynamicDrawUsage);
        sizeAttr.setUsage(THREE.DynamicDrawUsage);
        phaseAttr.setUsage(THREE.DynamicDrawUsage);
        rateAttr.setUsage(THREE.DynamicDrawUsage);
        visibilityAttr.setUsage(THREE.DynamicDrawUsage);
        geo.setAttribute('position', posAttr);
        // `position` may be presentation-jittered in Organic mode;
        // sourcePosition always retains the physical source coordinate.
        geo.setAttribute('sourcePosition', sourcePosAttr);
        geo.setAttribute('particleColor', colAttr);
        geo.setAttribute('size', sizeAttr);
        geo.setAttribute('manifestPhase', phaseAttr);
        geo.setAttribute('manifestRate', rateAttr);
        geo.setAttribute('particleVisibility', visibilityAttr);
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
        this._fluxVolumeAxisCapacity = sampledN;
        this._fluxPositionSignature = '';
        this._scene.add(this._fluxVolume);
        // Re-apply every persisted setting so a rebuild (resize / scenario / toggle) keeps
        // the user's flux-volume settings continuous instead of resetting them to the
        // freshly-built material/mesh defaults.
        this._fluxVolume.visible = this.showFlux;
        this._applyFluxMaterialState();                       // glow + opacity + shape
        if (this._fluxLatticeSpacing !== 1.0) {
            const spacing = this._fluxLatticeSpacing;
            const offset = (1 - spacing) * latticeSize / 2;
            this._fluxVolume.scale.setScalar(spacing);
            this._fluxVolume.position.set(offset, offset, offset);
        }
    }

    _ensureFluxVolumeCapacity(latticeSize, axisCapacity = fluxVolumeAxisSamples(latticeSize)) {
        const nextAxisCapacity = Math.max(1, Math.trunc(axisCapacity));
        if (this._fluxVolume
            && this._fluxVolumeSize === latticeSize
            && this._fluxVolumeAxisCapacity === nextAxisCapacity) return;
        if (this._fluxVolume) {
            this._scene.remove(this._fluxVolume);
            this._fluxVolume.geometry.dispose();
            this._fluxVolume.material.dispose();
            this._fluxVolume = null;
        }
        this._buildFluxVolume(latticeSize, nextAxisCapacity);
    }

    _ensureFluxActivationCapacity(count) {
        if (this._fluxActivation.length === count) return;
        this._fluxStateMask = new Uint8Array(count);
        this._fluxActivation = new Float64Array(count);
        this._fluxActivationScratchA = new Float64Array(count);
        this._fluxActivationScratchB = new Float64Array(count);
        this._fluxDensitySnapshot = new Float64Array(count);
    }

    _mapManifestedState(particleData, sourceN, compactSpacing, compactOrigin, compact) {
        const mask = this._fluxStateMask;
        mask.fill(0);
        const positions = particleData?.positions;
        const particleCount = Math.min(
            Math.max(0, Math.trunc(Number(particleData?.count) || 0)),
            positions ? Math.floor(positions.length / 3) : 0,
        );
        for (let i = 0; i < particleCount; i++) {
            const px = Math.floor(Number(positions[i * 3]));
            const py = Math.floor(Number(positions[i * 3 + 1]));
            const pz = Math.floor(Number(positions[i * 3 + 2]));
            if (!Number.isFinite(px) || !Number.isFinite(py) || !Number.isFinite(pz)) continue;
            const sx = compact
                ? Math.round((px - compactOrigin) / compactSpacing)
                : px;
            const sy = compact
                ? Math.round((py - compactOrigin) / compactSpacing)
                : py;
            const sz = compact
                ? Math.round((pz - compactOrigin) / compactSpacing)
                : pz;
            if (sx < 0 || sy < 0 || sz < 0
                || sx >= sourceN || sy >= sourceN || sz >= sourceN) continue;
            mask[(sz * sourceN + sy) * sourceN + sx] = 1;
        }
    }

    _cancelFluxAsyncUpdate() {
        if (this._fluxAsyncRaf && typeof cancelAnimationFrame === 'function') {
            cancelAnimationFrame(this._fluxAsyncRaf);
        }
        this._fluxAsyncRaf = 0;
        this._fluxAsyncJob = null;
        this._fluxPendingFrame = null;
    }

    _sourceCoordinate(frame, axisIndex) {
        if (!frame.compact) return axisIndex + 0.5;
        return Math.max(
            0,
            Math.min(frame.compactOrigin + axisIndex * frame.compactSpacing, frame.N - 1),
        ) + 0.5;
    }

    _sourceInsideBoundary(frame, sourceX, sourceY, sourceZ) {
        if (!frame.needsClip) return true;
        const physicalX = this._sourceCoordinate(frame, sourceX);
        const physicalY = this._sourceCoordinate(frame, sourceY);
        const physicalZ = this._sourceCoordinate(frame, sourceZ);
        return this._insideBoundary(
            (physicalX - frame.boundaryCenter) / frame.boundaryRadius,
            (physicalY - frame.boundaryCenter) / frame.boundaryRadius,
            (physicalZ - frame.boundaryCenter) / frame.boundaryRadius,
        );
    }

    _writeFluxAttributesUntil(frame, startIndex, deadline = Infinity) {
        const geometry = this._fluxVolume.geometry;
        const posArr = geometry.getAttribute('position').array;
        const sourcePosArr = geometry.getAttribute('sourcePosition').array;
        const colArr = geometry.getAttribute('particleColor').array;
        const sizeArr = geometry.getAttribute('size').array;
        const visibilityArr = geometry.getAttribute('particleVisibility').array;
        const density = frame.density;
        const activationValues = this._fluxActivation;
        const sourceN = frame.sourceN;
        const sourcePlane = sourceN * sourceN;
        const renderSpacing = frame.compact ? frame.compactSpacing : 1;
        const jamp = this._fluxOrganic ? renderSpacing : 0;
        const footprintStride = Math.min(renderSpacing, FLUX_POINT_FOOTPRINT_MAX_STRIDE);
        const pointCeiling = FLUX_LATTICE_MIN_POINT_SIZE
            + Math.max(0.1, this._fluxPointScale || 1.0) * 9.0 * footprintStride;
        let sourceIndex = startIndex;

        while (sourceIndex < frame.sourceCount) {
            const end = Math.min(frame.sourceCount, sourceIndex + 2048);
            for (; sourceIndex < end; sourceIndex++) {
                const sourceZ = Math.floor(sourceIndex / sourcePlane);
                const sourceRem = sourceIndex - sourceZ * sourcePlane;
                const sourceY = Math.floor(sourceRem / sourceN);
                const sourceX = sourceRem - sourceY * sourceN;
                const physicalX = this._sourceCoordinate(frame, sourceX);
                const physicalY = this._sourceCoordinate(frame, sourceY);
                const physicalZ = this._sourceCoordinate(frame, sourceZ);
                const c3 = sourceIndex * 3;

                if (frame.writePositions) {
                    let h = (sourceX * 92837111)
                        ^ (sourceY * 689287499)
                        ^ (sourceZ * 283923481);
                    h = (h ^ (h >>> 15)) >>> 0;
                    posArr[c3] = Math.max(0.5, Math.min(
                        frame.N - 0.5,
                        physicalX + ((h & 1023) / 1024 - 0.5) * jamp,
                    ));
                    posArr[c3 + 1] = Math.max(0.5, Math.min(
                        frame.N - 0.5,
                        physicalY + (((h >>> 10) & 1023) / 1024 - 0.5) * jamp,
                    ));
                    posArr[c3 + 2] = Math.max(0.5, Math.min(
                        frame.N - 0.5,
                        physicalZ + (((h >>> 20) & 1023) / 1024 - 0.5) * jamp,
                    ));
                    sourcePosArr[c3] = physicalX;
                    sourcePosArr[c3 + 1] = physicalY;
                    sourcePosArr[c3 + 2] = physicalZ;
                }

                const magnitude = Number(density[sourceIndex]);
                const activation = activationValues[sourceIndex];
                const relativeInstantEnergy = frame.instantMaxActivation > 1e-20
                    ? (activation / frame.instantMaxActivation) ** 2
                    : 0;
                const finiteSource = magnitude >= 0 && magnitude < Infinity;
                const inside = this._sourceInsideBoundary(frame, sourceX, sourceY, sourceZ);
                const meetsThreshold = frame.thresholdFraction === 0
                    || relativeInstantEnergy >= frame.thresholdFraction;
                const visible = finiteSource && inside && meetsThreshold;
                visibilityArr[sourceIndex] = visible ? 1 : 0;
                if (visible) frame.visibleCount++;

                const energyPhase = frame.maxActivation > 1e-20
                    ? Math.min(1, (activation / frame.maxActivation) ** 2)
                    : 0;
                fluxToColorInto(colArr, c3, energyPhase, 1);
                if (frame.thresholdFraction === 0) {
                    colArr[c3] = Math.max(colArr[c3], FLUX_LATTICE_INSPECTION_COLOR_FLOOR[0]);
                    colArr[c3 + 1] = Math.max(colArr[c3 + 1], FLUX_LATTICE_INSPECTION_COLOR_FLOOR[1]);
                    colArr[c3 + 2] = Math.max(colArr[c3 + 2], FLUX_LATTICE_INSPECTION_COLOR_FLOOR[2]);
                }
                sizeArr[sourceIndex] = FLUX_LATTICE_MIN_POINT_SIZE
                    + (pointCeiling - FLUX_LATTICE_MIN_POINT_SIZE) * energyPhase;
            }
            if (performance.now() >= deadline) break;
        }
        return sourceIndex;
    }

    _commitFluxAttributes(frame) {
        const geometry = this._fluxVolume.geometry;
        if (frame.writePositions) {
            geometry.getAttribute('position').needsUpdate = true;
            geometry.getAttribute('sourcePosition').needsUpdate = true;
            this._fluxPositionSignature = frame.positionSignature;
        }
        geometry.getAttribute('particleColor').needsUpdate = true;
        geometry.getAttribute('size').needsUpdate = true;
        geometry.getAttribute('particleVisibility').needsUpdate = true;
        // Stable one-to-one source indexing: threshold changes visibility, not
        // geometry support or coordinate ordering.
        geometry.setDrawRange(0, frame.sourceCount);
        this._fluxVisibleCount = frame.visibleCount;
    }

    _queueLargeFluxUpdate(frame, particleData) {
        this._fluxPendingFrame = { ...frame, densitySource: frame.density, particleData };
        if (!this._fluxAsyncJob) this._startPendingFluxUpdate();
    }

    _startPendingFluxUpdate() {
        const pending = this._fluxPendingFrame;
        if (!pending) return;
        this._fluxPendingFrame = null;
        this._fluxDensitySnapshot.set(pending.densitySource);
        this._mapManifestedState(
            pending.particleData,
            pending.sourceN,
            pending.compactSpacing,
            pending.compactOrigin,
            pending.compact,
        );
        const frame = {
            ...pending,
            density: this._fluxDensitySnapshot,
            phase: 'activation',
            index: 0,
            visibleCount: 0,
        };
        frame.stepper = createFluxActivationStepper(
            frame.density,
            frame.sourceN,
            this._fluxStateMask,
            this._fluxActivationScratchA,
            this._fluxActivationScratchB,
            this._fluxActivation,
        );
        this._fluxAsyncJob = frame;
        this._scheduleFluxAsyncSlice();
    }

    _scheduleFluxAsyncSlice() {
        if (this._fluxAsyncRaf || typeof requestAnimationFrame !== 'function') return;
        this._fluxAsyncRaf = requestAnimationFrame(() => this._runFluxAsyncSlice());
    }

    _runFluxAsyncSlice() {
        this._fluxAsyncRaf = 0;
        const frame = this._fluxAsyncJob;
        if (!frame || !this._fluxVolume) return;
        const deadline = performance.now() + FLUX_ASYNC_FRAME_BUDGET_MS;

        while (performance.now() < deadline && this._fluxAsyncJob === frame) {
            if (frame.phase === 'activation') {
                const result = frame.stepper.step(deadline);
                if (!result.done) break;
                frame.instantMaxActivation = result.instantMax;
                frame.phase = frame.needsClip ? 'clip-max' : 'normalize';
                frame.index = 0;
            } else if (frame.phase === 'clip-max') {
                const sourcePlane = frame.sourceN * frame.sourceN;
                const end = Math.min(frame.sourceCount, frame.index + 2048);
                for (; frame.index < end; frame.index++) {
                    const z = Math.floor(frame.index / sourcePlane);
                    const rem = frame.index - z * sourcePlane;
                    const y = Math.floor(rem / frame.sourceN);
                    const x = rem - y * frame.sourceN;
                    if (this._sourceInsideBoundary(frame, x, y, z)) {
                        frame.instantMaxActivation = Math.max(
                            frame.instantMaxActivationInside || 0,
                            this._fluxActivation[frame.index],
                        );
                        frame.instantMaxActivationInside = frame.instantMaxActivation;
                    }
                }
                if (frame.index >= frame.sourceCount) frame.phase = 'normalize';
            } else if (frame.phase === 'normalize') {
                if (frame.needsClip) {
                    frame.instantMaxActivation = frame.instantMaxActivationInside || 0;
                }
                frame.maxActivation = frame.instantMaxActivation > 1e-20
                    ? this._updatePeakHoldDecay('_fluxMaxDecay', frame.instantMaxActivation)
                    : this._fluxMaxDecay;
                frame.phase = 'write';
                frame.index = 0;
            } else if (frame.phase === 'write') {
                frame.index = this._writeFluxAttributesUntil(frame, frame.index, deadline);
                if (frame.index < frame.sourceCount) break;
                this._commitFluxAttributes(frame);
                this._fluxAsyncJob = null;
                this._startPendingFluxUpdate();
                return;
            }
        }
        this._scheduleFluxAsyncSlice();
    }

    /**
     * Update flux volume rendering from either a legacy dense N^3 magnitude
     * array or a native FTV2 descriptor:
     *   { data, latticeSize, stride, axisCount }
     * Both layouts are x-fastest. FTV2 is already sampled on the GPU/server,
     * avoiding a full N^3 device-to-host copy and socket payload at large L.
     * @param {Float32Array|Float64Array|{data:Float32Array,latticeSize:number,stride:number,axisCount:number}} volumeData
     * @param {number} latticeSize — side length N
     * @param {{positions?:Float32Array,count?:number}|null} particleData — manifested state sites
     */
    updateFluxVolume(volumeData, latticeSize, particleData = null) {
        const N = latticeSize;

        const compact = volumeData && !ArrayBuffer.isView(volumeData)
            && ArrayBuffer.isView(volumeData.data) ? volumeData : null;
        const density = compact ? compact.data : volumeData;

        // Early exit if no data.
        if (!density || density.length === 0) {
            if (this._fluxVolume) this._fluxVolume.geometry.setDrawRange(0, 0);
            return;
        }

        const thresholdFraction = clampFluxThreshold(
            this._fluxThreshold !== undefined ? this._fluxThreshold : 0.005,
        );
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
        } else {
            const total = N * N * N;
            if (density.length !== total) {
                // Size mismatch during an async resize/startup transition.
                return;
            }
            sourceN = N;
        }

        // Capacity and scientific coordinates depend only on the received
        // source grid. The threshold is deliberately absent from this call.
        this._ensureFluxVolumeCapacity(latticeSize, sourceN);

        const _bs = this._boundaryShape;
        const needsClip = !(_bs === 'cube' || _bs === 'none' || _bs === undefined);
        const boundaryCenter = N / 2;
        const boundaryRadius = N / 2;
        const compactCoord = (axisIndex) => Math.max(
            0,
            Math.min(compactOrigin + axisIndex * compactSpacing, N - 1),
        ) + 0.5;

        const sourceCount = sourceN * sourceN * sourceN;
        this._ensureFluxActivationCapacity(sourceCount);
        const positionSignature = [
            N,
            sourceN,
            compactSpacing,
            compactOrigin,
            this._fluxOrganic ? 1 : 0,
        ].join(':');
        const frame = {
            density,
            N,
            sourceN,
            sourceCount,
            compact: !!compact,
            compactSpacing,
            compactOrigin,
            thresholdFraction,
            needsClip,
            boundaryCenter,
            boundaryRadius,
            positionSignature,
            writePositions: this._fluxPositionSignature !== positionSignature,
        };

        if (sourceCount > FLUX_ASYNC_SOURCE_COUNT) {
            this._queueLargeFluxUpdate(frame, particleData);
            return;
        }
        this._cancelFluxAsyncUpdate();
        this._mapManifestedState(
            particleData,
            sourceN,
            compactSpacing,
            compactOrigin,
            !!compact,
        );
        const { instantMax: computedInstantMax } = computeFluxActivation(
            density,
            sourceN,
            this._fluxStateMask,
            this._fluxActivationScratchA,
            this._fluxActivationScratchB,
            this._fluxActivation,
        );
        let instantMaxActivation = computedInstantMax;
        if (needsClip && computedInstantMax > 0) {
            // Normalise only against drawable cells. Energy outside a shaped
            // presentation boundary must not raise the cutoff and suppress a
            // weaker in-bound voxel.
            instantMaxActivation = 0;
            const plane = sourceN * sourceN;
            for (let sourceIndex = 0; sourceIndex < sourceCount; sourceIndex++) {
                const sourceZ = Math.floor(sourceIndex / plane);
                const sourceRem = sourceIndex - sourceZ * plane;
                const sourceY = Math.floor(sourceRem / sourceN);
                const sourceX = sourceRem - sourceY * sourceN;
                const physicalX = compact ? compactCoord(sourceX) : sourceX + 0.5;
                const physicalY = compact ? compactCoord(sourceY) : sourceY + 0.5;
                const physicalZ = compact ? compactCoord(sourceZ) : sourceZ + 0.5;
                if (!this._insideBoundary(
                    (physicalX - boundaryCenter) / boundaryRadius,
                    (physicalY - boundaryCenter) / boundaryRadius,
                    (physicalZ - boundaryCenter) / boundaryRadius,
                )) continue;
                if (this._fluxActivation[sourceIndex] > instantMaxActivation) {
                    instantMaxActivation = this._fluxActivation[sourceIndex];
                }
            }
        }

        // Peak-hold-with-decay keeps a decaying field visibly decaying instead
        // of re-stretching every frame. A truly empty frame does not erase the
        // prior normalization; scenario/resize boundaries reset it explicitly.
        const maxActivation = instantMaxActivation > 1e-20
            ? this._updatePeakHoldDecay('_fluxMaxDecay', instantMaxActivation)
            : this._fluxMaxDecay;

        frame.instantMaxActivation = instantMaxActivation;
        frame.maxActivation = maxActivation;
        frame.visibleCount = 0;
        this._writeFluxAttributesUntil(frame, 0, Infinity);
        this._commitFluxAttributes(frame);
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
        this._cancelFluxAsyncUpdate();
        if (this._fluxVolume) {
            this._scene.remove(this._fluxVolume);
            if (this._fluxVolume.geometry) this._fluxVolume.geometry.dispose();
            if (this._fluxVolume.material) this._fluxVolume.material.dispose();
            this._fluxVolume = null;
            this._fluxVolumeSize = 0;
            this._fluxVolumeAxisCapacity = 0;
            this._fluxPositionSignature = '';
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
