import { copyScalarActivation } from './scalar-activation.js';
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
import { attachBackToFrontOrdering } from './point-cloud-draw-order.js';
import { fluxToColorInto, fluxToColor } from '../fields.js';

// Flux-volume vertex shader (sqrt depth scaling) — centralized in
// viewport/shaders.js (D-1).
import { FLUX_VOL_VERT, PARTICLE_FRAG, PARTICLE_SHADER_UNIFORMS } from './shaders.js';
import { clampFluxThreshold, DEFAULT_FLUX_THRESHOLD } from './flux-threshold.js';
import { computeFluxActivation, createFluxActivationStepper } from './flux-activation.js';
import { FluxVolumeWorkerClient } from './flux-volume-worker-client.js';

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
// Release rate of the peak-hold normaliser, per committed frame.
const FLUX_PEAK_HOLD_DECAY = 0.985;

// Deterministic per-site jitter (Organic mode) around the physical source
// coordinate, clamped to the lattice. `sourcePosition` keeps the exact value.
function writeFluxPosition(
    posArr, sourcePosArr, c3, N,
    sourceX, sourceY, sourceZ,
    physicalX, physicalY, physicalZ, jamp,
) {
    let h = (sourceX * 92837111)
        ^ (sourceY * 689287499)
        ^ (sourceZ * 283923481);
    h = (h ^ (h >>> 15)) >>> 0;
    posArr[c3] = Math.max(0.5, Math.min(
        N - 0.5,
        physicalX + ((h & 1023) / 1024 - 0.5) * jamp,
    ));
    posArr[c3 + 1] = Math.max(0.5, Math.min(
        N - 0.5,
        physicalY + (((h >>> 10) & 1023) / 1024 - 0.5) * jamp,
    ));
    posArr[c3 + 2] = Math.max(0.5, Math.min(
        N - 0.5,
        physicalZ + (((h >>> 20) & 1023) / 1024 - 0.5) * jamp,
    ));
    sourcePosArr[c3] = physicalX;
    sourcePosArr[c3 + 1] = physicalY;
    sourcePosArr[c3 + 2] = physicalZ;
}

// A queued frame outlives the producer's particle buffers.
function snapshotParticleSites(particleData) {
    const positions = particleData?.positions;
    if (!positions) return null;
    return {
        positions: positions.slice(),
        colors: particleData.colors ? particleData.colors.slice() : null,
        locked: particleData.locked ? particleData.locked.slice() : null,
        count: particleData.count,
    };
}

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
        this._fluxThreshold = DEFAULT_FLUX_THRESHOLD;
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
        this._fluxPendingDensitySnapshot = new Float64Array(0);
        this._fluxAsyncJob = null;
        this._fluxPendingFrame = null;
        this._fluxAsyncRaf = 0;
        this._fluxPositionSignature = '';
        this._fluxVisibleCount = 0;

        // Off-thread path for large lattices (see _queueWorkerFluxUpdate).
        // The worker is created on the first frame that needs it.
        this._fluxWorkerEnabled = true;
        this._fluxWorkerClient = null;
        this._fluxWorkerSpare = null;          // output arrays last shown, reused by the next job
        this._fluxWorkerDensityPool = [];      // owned density copies
        this._fluxWorkerOwnedDensity = null;   // the copy currently published as the snapshot
        this._fluxWorkerStateMask = null;
        this._fluxInsideMask = null;
        this._fluxInsideMaskKey = '';
    }

    // Peak-hold-with-decay update, instance-local (see the constructor
    // comment). Not shared with overlay-frames.js's updateDecayingMax helper —
    // this is a different module (the Three.js renderer, not the JS field
    // sampler).
    _updatePeakHoldDecay(fieldName, instantMax, decay = FLUX_PEAK_HOLD_DECAY) {
        const prev = this[fieldName] || 0;
        const next = Math.max(instantMax, prev * decay);
        this[fieldName] = next;
        return next;
    }

    /** Reset visual normalization at an authoritative scenario/resize boundary. */
    resetFluxNormalization() {
        this._cancelFluxAsyncUpdate();
        this._fluxVolume?.geometry.setDrawRange(0, 0);
        this._fluxMaxDecay = 0;
    }

    setBoundaryShape(shape) {
        this._boundaryShape = shape;
    }

    onLatticeSizeChanged(size, halfN) {
        this._cancelFluxAsyncUpdate();
        this._latticeSize = size;
        this._halfN = halfN;
        this._releaseFluxWorkerBuffers();
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
        // Every rebuild path funnels through here: drop the previous cloud's
        // draw-order hook before its geometry is replaced.
        if (this._fluxDrawOrderDetach) { this._fluxDrawOrderDetach(); this._fluxDrawOrderDetach = null; }
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
                ...Object.fromEntries(Object.entries(PARTICLE_SHADER_UNIFORMS).map(([key, uniform]) => [key, { ...uniform }])),
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
        // With glow OFF the cloud is NormalBlending + depthWrite:false, so its
        // draw order is its composite order; keep it back-to-front for the
        // camera (point-cloud-draw-order.js). Skipped automatically when
        // glow ON switches the material to additive blending.
        this._fluxDrawOrderDetach = attachBackToFrontOrdering(this._fluxVolume, {
            mode: 'axis', positionAttr: 'sourcePosition',
            wrapIndex: order => new THREE.Uint32BufferAttribute(order, 1),
        });
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
        this._fluxSiteKind = new Int8Array(count);
        this._fluxActivation = new Float64Array(count);
    }

    // Scratch and snapshot grids for the two main-thread paths (synchronous,
    // and the cooperative fallback). The worker owns its own scratch, so a
    // large lattice that never falls back never allocates these.
    _ensureFluxFallbackScratch(count) {
        if (this._fluxActivationScratchA?.length === count) return;
        this._fluxActivationScratchA = new Float64Array(count);
        this._fluxActivationScratchB = new Float64Array(count);
        this._fluxFallbackDensitySnapshot = new Float64Array(count);
        this._fluxPendingDensitySnapshot = new Float64Array(count);
    }

    _mapManifestedState(particleData, sourceN, compactSpacing, compactOrigin, compact) {
        const mask = this._fluxStateMask;
        mask.fill(0);
        const kinds = this._fluxSiteKind?.length === mask.length
            ? this._fluxSiteKind
            : (this._fluxSiteKind = new Int8Array(mask.length));
        kinds.fill(0);
        const positions = particleData?.positions;
        const colors = particleData?.colors;
        const locked = particleData?.locked;
        const particleCount = Math.min(
            Math.max(0, Math.trunc(Number(particleData?.count) || 0)),
            positions ? Math.floor(positions.length / 3) : 0,
        );
        let mapped = 0;
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
            const index = (sz * sourceN + sy) * sourceN + sx;
            mask[index] = 1;
            const green = colors ? colors[i * 3 + 1] : 0;
            const red = colors ? colors[i * 3] : 0;
            let kind = green > 0.7 ? 1 : red > 0.8 ? -1 : 1;
            if (locked?.[i]) kind = kind < 0 ? -2 : 2;
            kinds[index] = kind;
            mapped++;
        }
        return mapped;
    }

    _cancelFluxAsyncUpdate() {
        if (this._fluxAsyncRaf && typeof cancelAnimationFrame === 'function') {
            cancelAnimationFrame(this._fluxAsyncRaf);
        }
        this._fluxAsyncRaf = 0;
        // A worker request in flight cannot be recalled. Clearing the job
        // makes its result stale; _onFluxWorkerResult then recycles it.
        if (this._fluxPendingFrame?.worker) {
            this._recycleFluxWorkerDensity(this._fluxPendingFrame.densitySource);
        }
        this._fluxAsyncJob = null;
        this._fluxPendingFrame = null;
    }

    _fluxPointCeiling(frame) {
        const renderSpacing = frame.compact ? frame.compactSpacing : 1;
        const footprintStride = Math.min(renderSpacing, FLUX_POINT_FOOTPRINT_MAX_STRIDE);
        return FLUX_LATTICE_MIN_POINT_SIZE
            + Math.max(0.1, this._fluxPointScale || 1.0) * 9.0 * footprintStride;
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
        const pointCeiling = this._fluxPointCeiling(frame);
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
                    writeFluxPosition(
                        posArr, sourcePosArr, c3, frame.N,
                        sourceX, sourceY, sourceZ,
                        physicalX, physicalY, physicalZ, jamp,
                    );
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
        // One reusable pending snapshot is distinct from the active job's
        // snapshot. Bridge buffers may be reused before this job starts.
        this._ensureFluxFallbackScratch(frame.density.length);
        this._fluxPendingDensitySnapshot.set(frame.density);
        const positions = particleData?.positions?.slice();
        this._fluxPendingFrame = { ...frame,
            densitySource: this._fluxPendingDensitySnapshot,
            particleData: positions ? { positions, count: particleData.count } : null,
        };
        if (!this._fluxAsyncJob) this._startPendingFluxUpdate();
    }

    _startPendingFluxUpdate() {
        const pending = this._fluxPendingFrame;
        if (!pending) return;
        this._fluxPendingFrame = null;
        this._fluxDensitySnapshot = this._fluxFallbackDensitySnapshot;
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
        if (this._presentationSuspended || this._fluxAsyncRaf || typeof requestAnimationFrame !== 'function') return;
        this._fluxAsyncRaf = requestAnimationFrame(() => this._runFluxAsyncSlice());
    }

    /** Preserve partially prepared attributes while another workspace renders. */
    setPresentationSuspended(suspended) {
        this._presentationSuspended = !!suspended;
        if (suspended && this._fluxAsyncRaf) {
            cancelAnimationFrame(this._fluxAsyncRaf);
            this._fluxAsyncRaf = 0;
        } else if (!suspended && this._fluxAsyncJob && !this._fluxAsyncJob.worker) this._scheduleFluxAsyncSlice();
    }

    _runFluxAsyncSlice() {
        this._fluxAsyncRaf = 0;
        if (this._presentationSuspended) return;
        const frame = this._fluxAsyncJob;
        // A worker request has no phases to step; it completes by message.
        if (!frame || frame.worker || !this._fluxVolume) return;
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

    // ── Off-thread path (source grids above FLUX_ASYNC_SOURCE_COUNT) ────
    // flux-volume-worker.js computes activation, normalisation, colour, size
    // and visibility. The main thread copies the input, then swaps the
    // finished arrays into the geometry. `_fluxAsyncJob` and
    // `_fluxPendingFrame` keep their meaning: one request in flight, and the
    // single latest frame waiting behind it. The cooperative slices above
    // remain the fallback when no worker is available.

    /** Force the cooperative main-thread path (tests, before/after measurement). */
    setFluxVolumeWorkerEnabled(on) {
        this._fluxWorkerEnabled = !!on;
    }

    _ensureFluxWorker() {
        if (!this._fluxWorkerEnabled || globalThis.__ftdFluxVolumeWorker === false) return null;
        if (!this._fluxWorkerClient) {
            this._fluxWorkerClient = new FluxVolumeWorkerClient({
                onResult: (result) => this._onFluxWorkerResult(result),
                onFailure: (message) => this._onFluxWorkerFailure(message),
            });
        }
        return this._fluxWorkerClient.available ? this._fluxWorkerClient : null;
    }

    _takeFluxWorkerDensity(source) {
        const pool = this._fluxWorkerDensityPool;
        while (pool.length) {
            const candidate = pool.pop();
            if (candidate.length === source.length && candidate.constructor === source.constructor) {
                candidate.set(source);
                return candidate;
            }
        }
        return source.slice();
    }

    _recycleFluxWorkerDensity(array) {
        // A transferred (detached) array has length 0 and is dropped here.
        if (array?.length > 0 && this._fluxWorkerDensityPool?.length < 3) {
            this._fluxWorkerDensityPool.push(array);
        }
    }

    _releaseFluxWorkerBuffers() {
        this._fluxWorkerSpare = null;
        this._fluxWorkerStateMask = null;
        this._fluxInsideMask = null;
        this._fluxInsideMaskKey = '';
        if (this._fluxWorkerDensityPool) this._fluxWorkerDensityPool.length = 0;
    }

    /** Drawable-site mask for a shaped boundary, built once per shape and grid layout. */
    _fluxBoundaryMask(frame) {
        const key = [
            this._boundaryShape,
            frame.N,
            frame.sourceN,
            frame.compact ? 1 : 0,
            frame.compactSpacing,
            frame.compactOrigin,
        ].join(':');
        if (this._fluxInsideMaskKey === key && this._fluxInsideMask?.length === frame.sourceCount) {
            return this._fluxInsideMask;
        }
        const sourceN = frame.sourceN;
        const mask = new Uint8Array(frame.sourceCount);
        let sourceIndex = 0;
        for (let sourceZ = 0; sourceZ < sourceN; sourceZ++) {
            for (let sourceY = 0; sourceY < sourceN; sourceY++) {
                for (let sourceX = 0; sourceX < sourceN; sourceX++) {
                    mask[sourceIndex++] = this._sourceInsideBoundary(frame, sourceX, sourceY, sourceZ) ? 1 : 0;
                }
            }
        }
        this._fluxInsideMask = mask;
        this._fluxInsideMaskKey = key;
        return mask;
    }

    /** Positions depend only on the grid layout and Organic mode: one pass per layout. */
    _writeFluxPositions(frame) {
        const geometry = this._fluxVolume.geometry;
        const posAttr = geometry.getAttribute('position');
        const sourcePosAttr = geometry.getAttribute('sourcePosition');
        const sourceN = frame.sourceN;
        const jamp = this._fluxOrganic ? (frame.compact ? frame.compactSpacing : 1) : 0;
        let c3 = 0;
        for (let sourceZ = 0; sourceZ < sourceN; sourceZ++) {
            const physicalZ = this._sourceCoordinate(frame, sourceZ);
            for (let sourceY = 0; sourceY < sourceN; sourceY++) {
                const physicalY = this._sourceCoordinate(frame, sourceY);
                for (let sourceX = 0; sourceX < sourceN; sourceX++, c3 += 3) {
                    writeFluxPosition(
                        posAttr.array, sourcePosAttr.array, c3, frame.N,
                        sourceX, sourceY, sourceZ,
                        this._sourceCoordinate(frame, sourceX), physicalY, physicalZ, jamp,
                    );
                }
            }
        }
        posAttr.needsUpdate = true;
        sourcePosAttr.needsUpdate = true;
        this._fluxPositionSignature = frame.positionSignature;
    }

    /** @returns {boolean} false when no worker is available; the caller falls back. */
    _queueWorkerFluxUpdate(frame, particleData) {
        // Only typed arrays can be copied into a transferable buffer.
        if (!ArrayBuffer.isView(frame.density)) return false;
        const client = this._ensureFluxWorker();
        if (!client) return false;
        // A cooperative job must not race the worker for the same arrays.
        if ((this._fluxAsyncJob && !this._fluxAsyncJob.worker)
            || (this._fluxPendingFrame && !this._fluxPendingFrame.worker)) {
            this._cancelFluxAsyncUpdate();
        }
        if (frame.writePositions) this._writeFluxPositions(frame);

        // Latest wins: a frame still waiting is overwritten in place. The
        // bridge may reuse its buffer before the worker is free, so the
        // density is always copied here.
        const waiting = this._fluxPendingFrame;
        let densitySource;
        if (waiting && waiting.densitySource.length === frame.density.length
            && waiting.densitySource.constructor === frame.density.constructor) {
            densitySource = waiting.densitySource;
            densitySource.set(frame.density);
        } else {
            if (waiting) this._recycleFluxWorkerDensity(waiting.densitySource);
            densitySource = this._takeFluxWorkerDensity(frame.density);
        }
        const idle = !client.busy;
        this._fluxPendingFrame = {
            ...frame,
            worker: true,
            writePositions: false,
            density: null,
            densitySource,
            // Kept only so a worker failure can redraw from the producer.
            producerDensity: frame.density,
            // An idle worker maps the sites at once, straight from the producer.
            particleData: idle ? particleData : snapshotParticleSites(particleData),
        };
        if (idle) this._dispatchWorkerFluxUpdate();
        return true;
    }

    _dispatchWorkerFluxUpdate() {
        const pending = this._fluxPendingFrame;
        const client = this._fluxWorkerClient;
        if (!pending?.worker || !client?.available || client.busy || !this._fluxVolume) return;
        this._fluxPendingFrame = null;

        const sourceCount = pending.sourceCount;
        const manifested = this._mapManifestedState(
            pending.particleData,
            pending.sourceN,
            pending.compactSpacing,
            pending.compactOrigin,
            pending.compact,
        );
        let stateMask = null;
        if (manifested > 0) {
            stateMask = this._fluxWorkerStateMask?.length === sourceCount
                ? this._fluxWorkerStateMask
                : new Uint8Array(sourceCount);
            this._fluxWorkerStateMask = null;
            stateMask.set(this._fluxStateMask);
        }
        const spare = this._fluxWorkerSpare;
        this._fluxWorkerSpare = null;
        const reuse = spare
            && spare.activation.length === sourceCount
            && spare.colors.length === sourceCount * 3
            && spare.sizes.length === sourceCount
            && spare.visibilities.length === sourceCount ? spare : null;
        const job = {
            sourceN: pending.sourceN,
            density: pending.densitySource,
            stateMask,
            // Cloned, not transferred: the cached mask stays here.
            insideMask: pending.needsClip ? this._fluxBoundaryMask(pending) : null,
            thresholdFraction: pending.thresholdFraction,
            peakHold: this._fluxMaxDecay || 0,
            peakHoldDecay: FLUX_PEAK_HOLD_DECAY,
            pointFloor: FLUX_LATTICE_MIN_POINT_SIZE,
            pointCeiling: this._fluxPointCeiling(pending),
            colorFloor: FLUX_LATTICE_INSPECTION_COLOR_FLOOR,
            activation: reuse ? reuse.activation : null,
            colors: reuse ? reuse.colors : null,
            sizes: reuse ? reuse.sizes : null,
            visibilities: reuse ? reuse.visibilities : null,
        };
        const transfer = [job.density.buffer];
        if (stateMask) transfer.push(stateMask.buffer);
        if (reuse) {
            transfer.push(
                reuse.activation.buffer,
                reuse.colors.buffer,
                reuse.sizes.buffer,
                reuse.visibilities.buffer,
            );
        }
        const id = client.submit(job, transfer);
        if (!id) {
            this._onFluxWorkerFailure('request could not be posted', pending);
            return;
        }
        this._fluxAsyncJob = { ...pending, id, densitySource: null, particleData: null };
    }

    _onFluxWorkerResult(result) {
        if (result.stateMask) this._fluxWorkerStateMask = result.stateMask;
        const job = this._fluxAsyncJob;
        const geometry = this._fluxVolume?.geometry;
        const colorAttr = geometry?.getAttribute('particleColor');
        const sizeAttr = geometry?.getAttribute('size');
        const visibilityAttr = geometry?.getAttribute('particleVisibility');
        const current = !!job?.worker && job.id === result.id && !!colorAttr
            && colorAttr.array.length === result.colors.length
            && sizeAttr.array.length === result.sizes.length
            && visibilityAttr.array.length === result.visibilities.length;
        if (current) {
            // Swap, do not copy: the arrays that were on screen become the
            // next job's output buffers. three.js re-uploads an attribute
            // whose array was replaced by one of equal byte length.
            this._fluxWorkerSpare = {
                activation: this._fluxActivation,
                colors: colorAttr.array,
                sizes: sizeAttr.array,
                visibilities: visibilityAttr.array,
            };
            colorAttr.array = result.colors;
            sizeAttr.array = result.sizes;
            visibilityAttr.array = result.visibilities;
            this._fluxActivation = result.activation;
            if (this._fluxDensitySnapshot === this._fluxWorkerOwnedDensity) {
                this._recycleFluxWorkerDensity(this._fluxWorkerOwnedDensity);
            }
            this._fluxDensitySnapshot = result.density;
            this._fluxWorkerOwnedDensity = result.density;
            this._fluxMaxDecay = result.maxActivation;
            this._fluxWorkerComputeMs = result.computeMs;
            this._fluxAsyncJob = null;
            this._commitFluxAttributes({ ...job, writePositions: false, visibleCount: result.visibleCount });
        } else {
            // Cancelled, hidden, or superseded by a rebuild: commit nothing,
            // keep the buffers for the next job.
            this._fluxWorkerSpare = {
                activation: result.activation,
                colors: result.colors,
                sizes: result.sizes,
                visibilities: result.visibilities,
            };
            this._recycleFluxWorkerDensity(result.density);
        }
        this._dispatchWorkerFluxUpdate();
    }

    /**
     * The worker can no longer be trusted: every later frame uses the
     * cooperative path. The request inside the worker is gone with its
     * buffers, so the newest frame still known here is redrawn instead.
     */
    _onFluxWorkerFailure(message, unsent = null) {
        console.warn('[flux-volume] worker unavailable; using the main-thread path:', message);
        const lost = this._fluxAsyncJob?.worker ? this._fluxAsyncJob : null;
        const waiting = this._fluxPendingFrame?.worker ? this._fluxPendingFrame : null;
        if (lost) this._fluxAsyncJob = null;
        if (waiting) this._fluxPendingFrame = null;
        this._releaseFluxWorkerBuffers();
        const replay = waiting || unsent || lost;
        if (!replay || !this._fluxVolume) return;
        const density = replay.densitySource?.length === replay.sourceCount
            ? replay.densitySource
            : replay.producerDensity;
        if (density?.length !== replay.sourceCount) return;
        this._queueLargeFluxUpdate({
            ...replay,
            worker: false,
            density,
            writePositions: this._fluxPositionSignature !== replay.positionSignature,
        }, replay.particleData);
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
            this._cancelFluxAsyncUpdate();
            if (this._fluxVolume) this._fluxVolume.geometry.setDrawRange(0, 0);
            return;
        }
        this._fluxClockTick = (this._fluxClockTick || 0) + 1;

        const thresholdFraction = clampFluxThreshold(
            this._fluxThreshold !== undefined ? this._fluxThreshold : DEFAULT_FLUX_THRESHOLD,
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
                this._cancelFluxAsyncUpdate();
                this._fluxVolume?.geometry.setDrawRange(0, 0);
                return;
            }
        } else {
            const total = N * N * N;
            if (density.length !== total) {
                this._cancelFluxAsyncUpdate();
                this._fluxVolume?.geometry.setDrawRange(0, 0);
                return;
            }
            sourceN = N;
        }

        // Capacity and scientific coordinates depend only on the received
        // source grid. The threshold is deliberately absent from this call.
        if (this._fluxVolumeSize !== latticeSize || this._fluxVolumeAxisCapacity !== sourceN) {
            this._cancelFluxAsyncUpdate();
        }
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

        if (sourceCount > FLUX_ASYNC_SOURCE_COUNT && !compact?.scalarCounts) {
            // Off-thread when a worker is available; cooperative main-thread
            // slices otherwise.
            if (!this._queueWorkerFluxUpdate(frame, particleData)) {
                this._queueLargeFluxUpdate(frame, particleData);
            }
            return;
        }
        this._cancelFluxAsyncUpdate();
        this._ensureFluxFallbackScratch(sourceCount);
        this._mapManifestedState(
            particleData,
            sourceN,
            compactSpacing,
            compactOrigin,
            !!compact,
        );
        const { instantMax: computedInstantMax } = compact?.scalarCounts
            ? copyScalarActivation(density, this._fluxActivation)
            : computeFluxActivation(
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
        if (!next) this._cancelFluxAsyncUpdate();
        if (!this._fluxVolume) { if (!next) return; this._buildFluxVolume(this._latticeSize); }
        if (this._fluxVolume.visible === next) return;
        this._fluxVolume.visible = next;
        const geometry = this._fluxVolume.geometry;
        if (!next) {
            this._fluxHiddenDrawCount = geometry.drawRange?.count || 0;
            geometry.setDrawRange(0, 0);
        } else if (this._fluxHiddenDrawCount > 0) {
            geometry.setDrawRange(0, this._fluxHiddenDrawCount);
        }
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
        if (this._fluxDrawOrderDetach) { this._fluxDrawOrderDetach(); this._fluxDrawOrderDetach = null; }
        this._cancelFluxAsyncUpdate();
        this._fluxWorkerClient?.dispose();
        this._fluxWorkerClient = null;
        this._releaseFluxWorkerBuffers();
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
