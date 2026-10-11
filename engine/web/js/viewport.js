import { LINK_DISPLACEMENT, siteIndex } from './link-geometry.js';
import { DEFAULT_FLUX_THRESHOLD } from './viewport/flux-threshold.js';
import { fluxCellIndex, mooreCellIndices, nearestVisiblePoint } from './viewport/flux-point-grid.js';
import { OUTERMOST_LANDMARK_M, framedLandmarkVoxels, metresToVoxels } from './ui/components/live-rulers/scale-landmarks.js';
import { mountLiveRulers } from './ui/components/live-rulers/mount.js';
import { activationEnergyEv, clockEnvironmentStep, energyAreaPath, energyStringPath, globalClockPhase, LIVE_MEASURE_DEFAULTS, manifestedSiteName, pointAttributeSize, pointSpritePixels, relativeClockEnvironment, spectrumColor, voxelClockPhase } from './ui/components/live-rulers/measure.js';
/**
 * @file viewport.js
 * @brief Three.js 3D Viewport — renders particles and fields from the simulation bridge.
 *
 * [EXTENDED] Uses THREE.Points with custom ShaderMaterial for antialiased circles.
 * Orbital camera with smooth controls.
 *
 * ## Categorization of concerns
 *
 * This file is large (~3.3k LOC) and currently implements everything the
 * Scale 0-3 dashboard needs in a single `Viewport` class. It groups the
 * following areas (decomposition deferred — see docs/adr/0001-viewport-
 * decomposition.md):
 *
 *   1. **Scene lifecycle** — constructor, post-processing pipeline, resize,
 *      dispose, main render() (no-op fallback for scales that own their
 *      own renderer).
 *
 *   2. **Camera & input** — perspective camera setup, OrbitControls wiring,
 *      picking helpers (_pickParticle, screen-to-world conversion).
 *
 *   3. **Particle rendering** — _initParticles, updateParticles,
 *      _buildVelocityVectors / updateVelocityVectors, _buildTrails /
 *      updateTrails, _buildParticleForces / updateParticleForces.
 *
 *   4. **Boundary rendering** — _buildBoundary dispatches into the
 *      _build{Cube,Sphere,Platonic,Cylinder,Torus}Boundary helpers;
 *      _disposeBoundary tears them down; _insideBoundary is the
 *      point-containment test used by the lattice wiring.
 *
 *   5. **Molecular rendering** — _buildBondLines / updateBondLines for
 *      Scale 2 atoms and Scale 3 molecules.
 *
 *   6. **Field visualization** — E/B/Poynting/divergence/flux/force/gravity
 *      grids and streamlines: _build*Field / update*Field pairs, plus the
 *      dark matter halo, damping zones, genesis isosurface, and confinement
 *      strings for the Scale 0 field overlays.
 *
 *   7. **Volumetric rendering** — _buildFluxVolume / updateFluxVolume and
 *      the slice variant (updateFluxSlice) for 3D lattice visualization.
 *
 *   8. **Scenario chrome** — event horizon marker, axes, grid, ontic cube,
 *      scenario-specific scale application (_applyScenarioScale).
 *
 * Keep new code grouped within the appropriate section. Any new concern
 * that doesn't fit in 1-8 probably belongs in a separate module, not
 * another method on Viewport.
 */

import * as THREE from 'three';
import { createScalarVolumeRenderer } from './viewport/scalar-volume-renderer.js';
import { visualSampleGrid } from './lib/visual-sample-grid.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
// EffectComposer / RenderPass / UnrealBloomPass moved to viewport/scene-core.js (Phase 3a).
// getById moved with applyParticleColors / updateTrails to viewport/particle-renderer.js (Phase 3d).
// Molecular rendering (bonds, orbital shells/lobes, AE force arrows,
// element labels, nucleus glow) extracted to its own module as Wave 2
// ticket 4 of the large-file refactor. Viewport composes a
// MolecularRenderer and delegates every public method through a thin
// wrapper. See engine/web/docs/INDEX.md for modularization provenance.
import { MolecularRenderer } from './viewport/molecular-renderer.js';
import { SpinArrowManager } from './viewport/spin-arrow-manager.js';
// Boundary wireframe builders + containment predicate — extracted to keep
// viewport.js under the refactor-plan LOC target (refactoring-analyst RF-4).
// Pure geometry; no state beyond the returned Three.js Group.
// Note: Phase 3a moved boundary BUILDING into ViewportSceneCore; Viewport
// itself only consumes `insideBoundary` here for the cross-renderer
// containment-test callback that's passed to flux/particle renderers.
import { insideBoundary } from './viewport/boundary-geometry.js';
// Scene decoration (boundary wireframe, axes, post-processing pipeline,
// camera presets, render dispatch, dispose) extracted as Phase 3a of the
// viewport decomposition. Viewport composes a ViewportSceneCore and
// forwards every scene-decoration method through a thin wrapper. See
// viewport/REFACTOR_MAP.md §3a.
import { ViewportSceneCore } from './viewport/scene-core.js?v=8';
// Rubber-sheet visualizations (gravitational potential + 10 topology fields).
// Extracted per refactoring-analyst RF-1. Viewport holds the instance as
// this._topoRenderer and forwards via thin delegators.
import { TopologySheetRenderer } from './viewport/topology-sheet-renderer.js?v=3';
// Flux volume + flux streamlines extracted as Phase 3b of the viewport
// decomposition. Viewport composes a ViewportFluxRenderer and forwards
// every flux-volume/streamline method through a thin wrapper. See
// viewport/REFACTOR_MAP.md.
import { ViewportFluxRenderer } from './viewport/flux-renderer.js?v=6';
import { NativeTransportRenderer } from './viewport/native-transport-renderer.js';
// Particle Points mesh + trails + velocity-vectors + per-particle force arrows
// extracted as Phase 3d. Viewport composes a ViewportParticleRenderer and
// forwards every particle-mesh method through a thin wrapper. Atom/bond/
// orbital rendering is owned by MolecularRenderer (see import above) and
// remains delegated separately. See viewport/REFACTOR_MAP.md §3d.
import { ViewportParticleRenderer } from './viewport/particle-renderer.js?v=14';
// Field overlays (E/B/Poynting/divergence/force volumes/dark matter/damping/
// genesis/confinement/dual flux/chirality/light/horizon + quantum overlays)
// extracted as Phase 3c — the largest viewport sub-renderer (66 methods, 27+
// meshes). Mesh-factory helpers (buildStreamlineMesh, buildArrowFieldMesh,
// writeArrowFieldIntoMesh, writeStreamlinesIntoMesh) live HERE as the
// canonical home; FluxRenderer + ParticleRenderer's constructor callbacks
// route through bound methods on FieldRenderer. See viewport/REFACTOR_MAP.md §3c.
import { ViewportFieldRenderer } from './viewport/field-renderer.js?v=8';

// Pre-allocated buffer-size constants (MAX_PARTICLES / MAX_FIELD_GRID)
// were centralized into viewport/constants.js (D-6). They were unused in
// this orchestrator (every buffer allocation lives in the Phase-3
// sub-renderers), so they are not re-imported here.

// A framed view reached by travelling takes this long per power of ten of
// camera distance (clamped), so the empty range passes at a steady pace and
// each reference ring stays on screen for a second or more as it goes by.
const CAMERA_FLIGHT_MS_PER_DECADE = 520;
const CAMERA_FLIGHT_MAX_MS = 12500;
// Wheel zoom beyond the quasi-domain: gain per decade past the point where
// the shell has shrunk away, and its ceiling (about four notches a decade).
const FAR_ZOOM_GAIN_PER_DECADE = 6;
const FAR_ZOOM_GAIN_MAX = 9.4;

export class Viewport {
    constructor(container) {
        this.container = container;

        // Scene
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(0x0f1729);

        // Camera
        this.camera = new THREE.PerspectiveCamera(45, 1, 0.001, 2000);
        this.camera.position.set(60, 45, 60);

        // Define getter/setter on camera.far to track base value set by controllers
        this._baseFar = 2000;
        this._actualFar = 2000;
        Object.defineProperty(this.camera, 'far', {
            get: () => this._actualFar,
            set: (val) => {
                this._baseFar = val;
                this._actualFar = val;
            },
            configurable: true,
            enumerable: true
        });

        // Renderer
        this.renderer = new THREE.WebGLRenderer({ antialias: true, alpha: false });
        this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        this.renderer.toneMapping = THREE.ACESFilmicToneMapping;
        this.renderer.toneMappingExposure = 1.0;
        container.appendChild(this.renderer.domElement);

        // Controls
        this.controls = new OrbitControls(this.camera, this.renderer.domElement);
        this.controls.enableDamping = true;
        this.controls.dampingFactor = 0.12;
        this.controls.rotateSpeed = 0.6;
        this.controls.zoomSpeed = 1.2;
        this.controls.panSpeed = 1;
        this._orbitBase = { rotate: 0.6, zoom: 1.2, pan: 1, damp: 0.12 };
        this._orbitFine = false;
        this._onOrbitFineKey = (event) => this._setOrbitFine(event);
        this._onOrbitFineBlur = () => this._setOrbitFine(null);
        document.addEventListener('keydown', this._onOrbitFineKey, true);
        document.addEventListener('keyup', this._onOrbitFineKey, true);
        window.addEventListener('blur', this._onOrbitFineBlur);
        this._liveMeasure = { ...LIVE_MEASURE_DEFAULTS };
        this.controls.minDistance = 0.01;
        this.controls.maxDistance = 100000000;

        // Visual settings for particle size and opacity. Shared with
        // ViewportParticleRenderer (Phase 3d) — both sides hold a reference
        // to this same object so opacity changes from setOpacity propagate
        // both ways without explicit syncing.
        this.visualSettings = {
            globalScale: 1.0,
            manifestedSize: 12.0,
            positiveSize: 14.0,
            negativeSize: 10.0,
            voidSize: 4.0,
            opacity: 0.95,
            particleOpacity: 0.9,
            glowIntensity: 0.15,
            particleShape: 0,
            // Color particles by their real genesis-assigned color charge
            // (Voxel::color in {0,1,2,3} = colorless/red/green/blue) instead
            // of the default charge-sign coloring. Off by default so every
            // existing scenario keeps its established charge-sign look.
            colorByColorCharge: false,
        };

        // Particle system extracted to ViewportParticleRenderer (Phase 3d).
        // The renderer is constructed below alongside the other sub-renderers
        // so it can capture live-bound callbacks for boundary clipping and
        // the cross-renderer arrow-field writer.

        // Wireframe / axes / post-processing state owned by ViewportSceneCore
        // (Phase 3a). The orchestrator keeps showHeatmap because it gates the
        // flux-slice heatmap which lives on the orchestrator (Phase 3c
        // territory). Backward-compat getters/setters at the bottom of the
        // class forward `wireframe`, `axes`, `peAxes`, `peGrid`,
        // `_engineMode`, `_boundaryShape`, `_boundaryMode`, etc. to SceneCore.
        // showFlux is owned by FluxRenderer (Phase 3b); see backward-compat
        // getter/setter near end of class so external code can still read it.
        this.showHeatmap = false;

        // Lattice reference (orchestrator-owned; cascaded to all sub-renderers
        // by setLatticeSize via onLatticeSizeChanged callbacks).
        this.latticeSize = 33;
        this._latticeSize = 33;  // mirrored so quantum overlays can read it too
        this._halfN = 16;
        this._reflectiveBoundary = false;

        // Scene decoration (boundary wireframe, axes, post-processing, camera
        // presets) — Phase 3a extraction. Constructed BEFORE the flux/particle
        // renderers so that boundary state queried via the `_insideBoundary`
        // callback (which delegates to viewport/boundary-geometry.js using
        // `this._boundaryShape` — itself forwarded through the backward-compat
        // getter to SceneCore) is well-defined when those sub-renderers run
        // their first frame.
        this._sceneCore = new ViewportSceneCore({
            scene: this.scene,
            camera: this.camera,
            renderer: this.renderer,
            controls: this.controls,
            container: this.container,
            latticeSize: this.latticeSize,
            halfN: this._halfN,
            boundaryShape: 'cube',
            boundaryMode: 'lattice',
            engineMode: 'lattice',
            insideBoundary: (nx, ny, nz) => this._insideBoundary(nx, ny, nz),
        });

        // Ambient light for subtle depth cues
        const ambient = new THREE.AmbientLight(0x404060, 0.5);
        this.scene.add(ambient);

        // Molecular renderer (bonds, orbital shells, AE force arrows,
        // element labels). Takes the scene by reference; owns its own
        // meshes and tears them down from its own dispose().
        this._molRenderer = new MolecularRenderer(this.scene);
        // Spin-arrow primitive — Three.js arrow that follows tracked
        // particles. Used by the P1 g-2 panel's "Track this particle"
        // affordance. Each tracked particle gets a Group (arrow + reference
        // axis + phase tick) updated per render frame.
        this.spinArrowManager = new SpinArrowManager(this.scene);
        this._lastSpinArrowUpdateMs = performance.now();

        // Rubber-sheet visualizations — gravitational potential + 10 topology
        // fields. Uses live-state getters so lattice-size changes propagate.
        this._topoRenderer = new TopologySheetRenderer({
            scene: this.scene,
            getLatticeSize: () => this._latticeSize || 32,
            getHalfN: () => this._halfN,
            // Topology toggles trigger quantum-renderer visibility coordination
            // (matches the pre-refactor call to this._quantumSetVisibility()).
            onVisibilityChange: () => this._quantumSetVisibility(),
        });

        // Field overlays — extracted Phase 3c. Owns all 27+ field-overlay
        // meshes plus the mesh-factory helpers (buildStreamlineMesh /
        // buildArrowFieldMesh / writeStreamlinesIntoMesh /
        // writeArrowFieldIntoMesh) that FluxRenderer + ParticleRenderer call
        // via constructor-injected callbacks. MUST be constructed BEFORE
        // FluxRenderer + ParticleRenderer so those callbacks can bind to
        // its methods.
        this._fieldRenderer = new ViewportFieldRenderer({
            scene: this.scene,
            camera: this.camera,
            latticeSize: this.latticeSize,
            halfN: this._halfN,
            boundaryShape: this._boundaryShape,
            insideBoundary: (nx, ny, nz) => this._insideBoundary(nx, ny, nz),
            getBoundaryMode: () => this._boundaryMode,
        });

        // Flux volume + flux streamlines — extracted Phase 3b. Viewport owns
        // the orchestrator; FluxRenderer owns its meshes + scenario-scale
        // helpers. The two streamline-mesh helpers (Phase 3c) now live on
        // FieldRenderer — we pass them in as bound callbacks.
        this._fluxRenderer = new ViewportFluxRenderer({
            scene: this.scene,
            latticeSize: this.latticeSize,
            halfN: this._halfN,
            boundaryShape: this._boundaryShape,
            insideBoundary: (nx, ny, nz) => this._insideBoundary(nx, ny, nz),
            applyScenarioScale: () => this._applyScenarioScale(),
            buildStreamlineMesh: (m, o) => this._fieldRenderer.buildStreamlineMesh(m, o),
            writeStreamlinesIntoMesh: (m, s, c) => this._fieldRenderer.writeStreamlinesIntoMesh(m, s, c),
        });

        // Native transport on lattice links (spec 2026-09-15 native transport overlays).
        this._nativeTransportRenderer = new NativeTransportRenderer({ scene: this.scene });

        // Particle Points mesh + trails + velocity vectors + per-particle
        // force arrows — extracted Phase 3d. Atom/bond/orbital rendering is
        // owned by MolecularRenderer (composed above) and remains a separate
        // delegation. visualSettings is passed by REFERENCE so setOpacity
        // writes are visible to both sides without re-syncing. The
        // arrow-field-writer callback (Phase 3c) now routes through
        // FieldRenderer.
        this._particleRenderer = new ViewportParticleRenderer({
            scene: this.scene,
            latticeSize: this.latticeSize,
            halfN: this._halfN,
            insideBoundary: (nx, ny, nz) => this._insideBoundary(nx, ny, nz),
            getBoundaryShape: () => this._boundaryShape,
            getBoundaryMode: () => this._boundaryMode,
            getEngineMode: () => this._engineMode,
            visualSettings: this.visualSettings,
            writeArrowFieldIntoMesh: (m, f, c, k, b, t) => this._fieldRenderer.writeArrowFieldIntoMesh(m, f, c, k, b, t),
        });

        // Axis helper + boundary wireframe + post-processing pipeline
        // are all owned by ViewportSceneCore (Phase 3a) — see ctor above.

        // Handle resize
        this._onResize();
        this._resizeObserver = new ResizeObserver(() => this._onResize());
        this._resizeObserver.observe(container);
        this._liveRulers = mountLiveRulers(container);

        // One-time shader preparation belongs to boot, not the first user
        // interaction. The flux-slice mesh remains hidden after compilation.
        this._fieldRenderer.prewarmFluxSlice(this.renderer);
    }

    // _initParticles extracted to ViewportParticleRenderer (Phase 3d).
    // External callers should not invoke this method directly — the
    // particle Points mesh is built eagerly inside ParticleRenderer's
    // constructor and is reachable via the `particles` getter on Viewport.

    // ── Boundary system ────────────────────────────────────────────────
    // Phase 3a: extracted to viewport/scene-core.js. Thin delegators
    // preserve any internal callers that still call these names.

    _disposeBoundary() { this._sceneCore?._disposeBoundary(); }

    _buildBoundary(shape, mode) { this._sceneCore?._buildBoundary(shape, mode); }



    setBoundaryShape(shape) {
        // Forward to SceneCore (rebuilds wireframe), FluxRenderer
        // (rebuilds clipped flux volume), and FieldRenderer (caches shape
        // for per-frame clipping). ParticleRenderer reads boundary shape
        // via its `getBoundaryShape` callback, so no explicit notify there.
        this._sceneCore?.setBoundaryShape(shape);
        this._fluxRenderer?.setBoundaryShape(shape);
        this._fieldRenderer?.setBoundaryShape(shape);
    }

    setReflectiveBoundary(on) {
        this._reflectiveBoundary = !!on;
    }

    setBoundaryDynamics(mode, periodicAxis) {
        this._sceneCore?.setBoundaryDynamics(mode, periodicAxis);
    }

    setGlobalClockTick(tick) {
        this._sceneCore?.setGlobalClockTick(tick);
    }

    setGlobalClockState(state) {
        this._sceneCore?.setGlobalClockState(state);
    }

    /**
     * Test whether a point (normalized -1..1 from center) is inside the
     * current boundary. Delegated to viewport/boundary-geometry.js.
     * Stays on the orchestrator so flux/particle/field renderers all
     * share a single callback (avoids 4 duplicate definitions).
     */
    _insideBoundary(nx, ny, nz) {
        return insideBoundary(this._boundaryShape, nx, ny, nz);
    }

    // Phase 3a: extracted to viewport/scene-core.js.
    _buildAxes() { this._sceneCore?._buildAxes(); }

    setLatticeSize(size) {
        this.clearScalarVolumes();
        this.latticeSize = size;
        this._latticeSize = size;  // mirrored so quantum overlays can read it too
        this._halfN = size / 2;

        // Sub-renderer cascade — every sub-renderer rebuilds/refreshes its
        // meshes for the new lattice size. SceneCore handles boundary
        // wireframe + axes + camera recentering (Phase 3a). FluxRenderer
        // rebuilds the flux volume + clears streamlines (Phase 3b).
        // FieldRenderer rebuilds field heatmap + clears draw ranges on
        // all 20+ field overlays (Phase 3c). ParticleRenderer refreshes
        // its cached _halfN (Phase 3d).
        this._sceneCore?.onLatticeSizeChanged(size, this._halfN);
        this._fluxRenderer?.onLatticeSizeChanged(size, this._halfN);
        this._fieldRenderer?.onLatticeSizeChanged(size, this._halfN);
        this._particleRenderer?.onLatticeSizeChanged(size, this._halfN);
        this._topoRenderer?.onLatticeSizeChanged(size, this._halfN);

        // Tracked particles may have stale ids after a scenario / lattice resize;
        // dispose all spin arrows so the next track() request gets a clean Group.
        if (this.spinArrowManager) this.spinArrowManager.dispose();
        // TopologySheetRenderer has resized all built surfaces immediately.

        // Rebuild void box for raycasting (orchestrator-owned — it's a
        // raycasting bounding volume used by the inspector, not scene
        // decoration; lives here until a future picker module).
        if (this._voidBox) {
            this.scene.remove(this._voidBox);
            this._voidBox.geometry.dispose();
            this._voidBox.material.dispose();
        }
        const boxGeo = new THREE.BoxGeometry(size, size, size);
        const boxMat = new THREE.MeshBasicMaterial({ transparent: true, opacity: 0, depthWrite: false, side: THREE.DoubleSide });
        this._voidBox = new THREE.Mesh(boxGeo, boxMat);
        const c = size / 2;
        this._voidBox.position.set(c, c, c);
        this.scene.add(this._voidBox);

        if (this._applyScenarioScale) this._applyScenarioScale();
    }

    toggleWireframe(on) { this._sceneCore?.toggleWireframe(on); }

    // ── Camera presets ────────────────────────────────────────────────
    // Snap the orbit camera to a named viewpoint. All positions are
    // computed from the current lattice size so the preset reads the
    // same at N=32 and N=128. The target is always the voxel-center
    // midpoint (N/2) — matches where every physics overlay centers.
    //
    // `which` values:
    //   'front' — looking along -Z (standard "face-on" view)
    //   'side'  — looking along -X
    //   'top'   — looking along -Y (birds-eye)
    // The front preset is the canonical boot/resize default.
    setCameraPreset(which) { return this._sceneCore?.setCameraPreset(which) ?? false; }

    // Frame the camera so the lattice / active boundary fills the view.
    // Uses the bounding sphere of the flux-volume geometry when possible
    // so the zoom reflects what's actually non-empty; falls back to the
    // full lattice extent otherwise.
    zoomToFit() {
        if (this._boundaryMode !== 'lattice') return false;
        const N = this.latticeSize || 32;
        const c = N / 2;
        // Use flux-volume geometry's bounding sphere when populated;
        // otherwise frame the whole lattice cube.
        let radius = N * 0.6;
        if (this._fluxVolume && this._fluxVolume.geometry) {
            const bs = this._fluxVolume.geometry.boundingSphere;
            if (bs && isFinite(bs.radius) && bs.radius > 0.5) radius = bs.radius * 1.3;
        }
        const fov = (this.camera.fov || 60) * Math.PI / 180;
        const dist = radius / Math.tan(fov / 2);
        // Preserve current view direction; just scale the camera's distance.
        const dir = this.camera.position.clone().sub(this.controls.target);
        const curDist = Math.max(1e-6, dir.length());
        dir.multiplyScalar(dist / curDist);
        this.controls.target.set(c, c, c);
        this.camera.position.copy(this.controls.target).add(dir);
        this.controls.update();
        return true;
    }

    getReferenceDistance() {
        const mode = this._engineMode || 'lattice';
        switch (mode) {
            case 'lattice':
                return (this.latticeSize || 32) * 1.6666;
            case 'particles':
            case 'atoms':
            case 'molecules':
                return 64.03;
            case 'planetary':
                return 11.18;
            case 'cosmic':
                return 570.09;
            default:
                return (this.latticeSize || 32) * 1.6666;
        }
    }

    /**
     * Place the camera on a three-quarter view of a canned subject.
     * Distances are chosen so the subject fills a set share of the view.
     * Beyond the quasi-domain the subjects are external reference lengths
     * (scale-landmarks.js): nothing is simulated out there.
     * With `animate` the camera travels there at a steady rate per power of
     * ten instead of jumping; any user input takes over.
     */
    setFramedView(id, { animate = false } = {}) {
        const frames = {
            moore: { voxels: 3, fill: 0.5, focus: 'voxel' },
            neighborhood: { voxels: 9, fill: 0.62, focus: 'voxel' },
            detail: { voxels: null, fill: 1.35, focus: 'lattice', span: 'edge' },
            lattice: { voxels: null, fill: 0.86, focus: 'lattice', span: 'diagonal' },
            'lattice-out': { voxels: null, fill: 0.2, focus: 'lattice', span: 'diagonal' },
            quasi: { voxels: null, fill: 0.74, focus: 'lattice', span: 'shell' },
        };
        const landmarkVoxels = framedLandmarkVoxels(id);
        const frame = frames[id] || (landmarkVoxels ? { voxels: landmarkVoxels, fill: 1, focus: 'lattice' } : null);
        if (!frame) return false;
        const unit = this._worldPerLatticeUnit();
        const n = Math.max(1, this.latticeSize || 32);
        let subject = frame.voxels ? frame.voxels * unit : n * unit;
        if (frame.span === 'diagonal') subject *= Math.sqrt(3);
        if (frame.span === 'shell') {
            const shell = this._shellDiameterUnits();
            subject = (shell > 0 ? shell : n * 8) * unit;
        }
        let distance = this._distanceForWorldSize(subject, frame.fill);
        const target = frame.focus === 'voxel'
            ? this._attachedVoxelWorld()
            : this._latticeCenterWorld();
        const view = new THREE.Vector3(1, 0.62, 1.05).normalize();
        // The whole-lattice view keeps the clock above the lattice clear of
        // the view ruler's row.
        if (id === 'lattice') distance += this._clockClearanceAt(target, view, distance);
        this.controls.maxDistance = Math.max(this.controls.maxDistance || 0, distance * 4, 1e8);
        this.controls.minDistance = Math.min(this.controls.minDistance || 0.01, 0.01);
        this.camera.up.set(0, 1, 0);
        this._cancelCameraFlight();
        const from = this.camera.position.distanceTo(this.controls.target);
        if (animate && from > 0 && typeof requestAnimationFrame === 'function') {
            this._flyCamera(target, view, from, distance);
            return true;
        }
        this.camera.position.copy(target).addScaledVector(view, distance);
        this.controls.update();
        this._updateLiveRulers();
        return true;
    }

    /**
     * Extra orbit distance the clock face needs to clear the view ruler when
     * the camera looks at `target` from `distance` along `view`. The camera is
     * placed there only for the measurement and put back.
     */
    _clockClearanceAt(target, view, distance) {
        const core = this._sceneCore;
        if (!core?.clockClearance) return 0;
        const position = this.camera.position.clone();
        const orientation = this.camera.quaternion.clone();
        this.camera.position.copy(target).addScaledVector(view, distance);
        this.camera.lookAt(target);
        const extra = core.clockClearance(this._viewSize?.height || undefined);
        this.camera.position.copy(position);
        this.camera.quaternion.copy(orientation);
        return extra;
    }

    /**
     * Travel from one orbit distance to another along a fixed view direction,
     * at an even rate per power of ten, so twenty decades of empty space pass
     * as steadily as two. Wheel, drag or another framed view cancels it.
     */
    _flyCamera(target, view, from, to) {
            // The flight starts from the present target and direction.
        const decades = Math.abs(Math.log10(to / from));
        const duration = Math.min(CAMERA_FLIGHT_MAX_MS, Math.max(450, decades * CAMERA_FLIGHT_MS_PER_DECADE));
        const started = performance.now();
        this.controls.target.copy(target);
        const flight = { cancelled: false, frame: 0 };
        this._cameraFlight = flight;
        const cancel = () => this._cancelCameraFlight();
        this.controls.addEventListener('start', cancel);
        this.renderer?.domElement?.addEventListener('wheel', cancel, { passive: true });
        flight.release = () => {
            this.controls.removeEventListener('start', cancel);
            this.renderer?.domElement?.removeEventListener('wheel', cancel);
        };
        const step = (now) => {
            if (flight.cancelled) return;
            const t = Math.min(1, (now - started) / duration);
            // Ease in and out in log-distance, so the start and the stop are soft.
            const eased = t * t * (3 - 2 * t);
            const distance = from * Math.pow(to / from, eased);
            if (swing && t < 1) {
                aim.copy(startTarget).lerp(target, eased);
                along.copy(startView).lerp(view, eased).normalize();
            } else {
                aim.copy(target);
                along.copy(view);
            }
            this.controls.target.copy(aim);
            this.camera.position.copy(aim).addScaledVector(along, distance);
            this.controls.update();
            if (t < 1) {
                flight.frame = requestAnimationFrame(step);
            } else {
                this._cancelCameraFlight();
                this._updateLiveRulers();
            }
        };
        flight.frame = requestAnimationFrame(step);
    }

    _cancelCameraFlight() {
        const flight = this._cameraFlight;
        if (!flight) return;
        this._cameraFlight = null;
        flight.cancelled = true;
        if (flight.frame && typeof cancelAnimationFrame === 'function') cancelAnimationFrame(flight.frame);
        flight.release?.();
    }
        // Where the flight starts: the look-at point and the direction to the
        // camera. Both turn into the destination's along the way, so a hop
        // between nearby views swings round instead of snapping. Opposite
        // directions have no path between them and are taken at once.
        const startTarget = this.controls.target.clone();
        const startView = this.camera.position.clone().sub(startTarget).normalize();
        const swing = startView.dot(view) > -0.99;
        const aim = new THREE.Vector3();
        const along = new THREE.Vector3();

    /**
     * How far the orbit may dolly out. In the lattice view that is far enough
     * for the outermost reference length (the electron) to sit well inside
     * the view; every other mode keeps the earlier limit.
     */
    _zoomOutLimit() {
        if (this._engineMode && this._engineMode !== 'lattice') return 1e8;
        // Called every frame: recomputed only when the view size or the
        // lattice's drawn scale changes.
        const unit = (this.scene.scale.x || 1) * (this._fluxRenderer?._fluxVolume?.scale.x || 1);
        const size = this._viewSize;
        const cache = this._zoomLimit || (this._zoomLimit = { unit: NaN, size: null, value: 1e8 });
        if (cache.unit !== unit || cache.size !== size) {
            cache.unit = unit;
            cache.size = size;
            cache.value = Math.max(1e8, this._distanceForWorldSize(metresToVoxels(OUTERMOST_LANDMARK_M) * unit, 0.2));
        }
        return cache.value;
    }

    /** Screen position of the lattice centre and the pixels one voxel covers there. */
    _latticeCentreOnScreen(viewWidth, viewHeight) {
        if (this._engineMode && this._engineMode !== 'lattice') return null;
        const world = this._latticeCenterWorld();
        const eye = (this._centreEye || (this._centreEye = new THREE.Vector3()))
            .copy(world).applyMatrix4(this.camera.matrixWorldInverse);
        const depth = -eye.z;
        if (!(depth > 0)) return null;
        world.project(this.camera);
        const worldPerPixel = (2 * Math.tan((this.camera.fov * Math.PI) / 360) * depth) / Math.max(1, viewHeight);
        return {
            x: (world.x * 0.5 + 0.5) * viewWidth,
            y: (-world.y * 0.5 + 0.5) * viewHeight,
            pixelsPerVoxel: this._worldPerLatticeUnit() / worldPerPixel,
        };
    }

    _worldPerLatticeUnit() {
        const flux = this._fluxRenderer?._fluxVolume;
        if (flux) {
            flux.updateMatrixWorld();
            const e = flux.matrixWorld.elements;
            const scale = Math.hypot(e[0], e[1], e[2]);
            if (scale > 0) return scale;
        }
        this.scene.updateMatrixWorld();
        return this.scene.scale.x || 1;
    }

    _latticeCenterWorld() {
        const n = this.latticeSize || 32;
        const point = new THREE.Vector3(n / 2, n / 2, n / 2);
        const flux = this._fluxRenderer?._fluxVolume;
        if (flux) {
            flux.updateMatrixWorld();
            return point.applyMatrix4(flux.matrixWorld);
        }
        this.scene.updateMatrixWorld();
        return point.applyMatrix4(this.scene.matrixWorld);
    }

    _attachedVoxelWorld() {
        const flux = this._fluxRenderer?._fluxVolume;
        const position = flux?.geometry?.getAttribute('position');
        const index = this._attachedPoint;
        if (!position || index == null || index >= position.count) return this._latticeCenterWorld();
        const arr = position.array;
        const point = new THREE.Vector3(arr[index * 3], arr[index * 3 + 1], arr[index * 3 + 2]);
        flux.updateMatrixWorld();
        return point.applyMatrix4(flux.matrixWorld);
    }

    _distanceForWorldSize(worldSize, fill) {
        const fov = (this.camera.fov || 50) * Math.PI / 180;
        const view = this._viewSize || this.container?.getBoundingClientRect?.() || { width: 1000, height: 600 };
        const aspect = Math.max(0.2, view.width / Math.max(1, view.height));
        const vertical = 2 * Math.tan(fov / 2);
        const limit = Math.min(vertical, vertical * aspect);
        const share = Math.max(0.05, fill);
        return Math.max(worldSize, 1e-4) / (limit * share);
    }

    setZoomMagnitude(factor) {
        if (!Number.isFinite(factor) || factor <= 0) return;
        const refDist = this.getReferenceDistance();
        const targetDist = refDist / factor;

        const minD = this.controls.minDistance || 0.01;
        const maxD = this.controls.maxDistance || 100000000;
        const clampedDist = Math.max(minD, Math.min(maxD, targetDist));

        const dir = this.camera.position.clone().sub(this.controls.target);
        if (dir.lengthSq() === 0) {
            dir.set(0, 0, 1);
        }
        dir.normalize().multiplyScalar(clampedDist);
        this.camera.position.copy(this.controls.target).add(dir);
        this.controls.update();
        this.render();
    }

    setWireframeBrightness(val) { this._sceneCore?.setWireframeBrightness(val); }

    getViewControlState() { return this._sceneCore?.getViewControlState() ?? {}; }

    toggleAxes(on) { this._sceneCore?.toggleAxes(on); }

    toggleBoundaryOrientation(on) { this._sceneCore?.toggleBoundaryOrientation(on); }

    toggleGlobalClock(on) { this._sceneCore?.toggleGlobalClock(on); }

    setVoxelHighlight(x, y, z, active) { this._sceneCore?.setVoxelHighlight(x, y, z, active); }

    toggleGrid(on) { this._sceneCore?.toggleGrid(on); }

    // ── Velocity Vectors / Trails ───────────────────────────────────────
    // Phase 3d: extracted to viewport/particle-renderer.js. These thin
    // delegators preserve the public API for app.js and panel code.
    updateVelocityVectors(positions, velocities, count, ids = null) {
        this._particleRenderer.updateVelocityVectors(positions, velocities, count, ids);
    }
    toggleVelocityVectors(on) { this._particleRenderer.toggleVelocityVectors(on); }
    updateSpinVectors(positions, spinAxes, spins, count, ids = null) {
        this._particleRenderer.updateSpinVectors(positions, spinAxes, spins, count, ids);
    }
    toggleSpinVectors(on) { this._particleRenderer.toggleSpinVectors(on); }
    updateTrails(trailHistory, typeMap, settings, currentTick) {
        return this._particleRenderer.updateTrails(trailHistory, typeMap, settings, currentTick);
    }
    toggleTrails(on) { this._particleRenderer.toggleTrails(on); }
    clearTrails() { this._particleRenderer.clearTrails(); }

    // ── Bond Lines (Scale 2 — Atom mode) ──────────────────────────────
    // Moved to viewport/molecular-renderer.js (Wave 2 ticket 4).
    // `this.bondLines` is preserved as a getter for external callers.
    get bondLines() { return this._molRenderer?.bondLines ?? null; }
    updateBondLines(atomData) { this._molRenderer.updateBondLines(atomData); }
    toggleBondLines(on)       { this._molRenderer.toggleBondLines(on); }

    // ══════════════════════════════════════════════════════════════════
    // Phase 3c: Field overlays delegated to viewport/field-renderer.js.
    // Every method below this line is a thin one-line forwarder. The 27+
    // field-overlay meshes (E/B/Poynting/divergence/forces/dark matter/
    // damping/genesis/confinement/dual flux/chirality/light/horizon +
    // quantum scaffolding/phase/Lagrangian/entropy) live there.
    // Mesh-factory helpers (buildStreamlineMesh, buildArrowFieldMesh,
    // writeStreamlinesIntoMesh, writeArrowFieldIntoMesh) are also owned
    // by FieldRenderer; FluxRenderer + ParticleRenderer call them via
    // bound callbacks set up in this constructor.
    // ══════════════════════════════════════════════════════════════════

    // ── Field Heatmap (potential colored grid dots on XZ plane) ───────
    _buildFieldHeatmap() { this._fieldRenderer._buildFieldHeatmap(); }
    updateFieldHeatmap(gridPositions, potentials, count, maxAbsPotential) {
        this._fieldRenderer.updateFieldHeatmap(gridPositions, potentials, count, maxAbsPotential);
    }
    toggleFieldHeatmap(on) { this._fieldRenderer.toggleFieldHeatmap(on); }

    // ── Field Vectors (force arrows on XZ plane) ─────────────────────
    _buildFieldVectors() { this._fieldRenderer._buildFieldVectors(); }
    updateFieldVectors(gridPositions, forces, count, maxForce, arrowScale = 8.0, coordinateOffset) {
        this._fieldRenderer.updateFieldVectors(
            gridPositions, forces, count, maxForce, arrowScale, coordinateOffset,
        );
    }
    toggleFieldVectors(on) { this._fieldRenderer.toggleFieldVectors(on); }

    // ── PE E-Field Streamlines (3D Coulomb field lines) ────────────────
    _buildPEStreamlines() { this._fieldRenderer._buildPEStreamlines(); }
    updatePEStreamlines(lines) { this._fieldRenderer.updatePEStreamlines(lines); }
    togglePEStreamlines(on) { this._fieldRenderer.togglePEStreamlines(on); }

    // ── Gravity Field Vectors (XZ plane) ──────────────────────────────
    _buildGravityVectors() { this._fieldRenderer._buildGravityVectors(); }
    updateGravityVectors(gridPositions, forces, count, maxForce, arrowScale = 8.0) {
        this._fieldRenderer.updateGravityVectors(gridPositions, forces, count, maxForce, arrowScale);
    }
    toggleGravityVectors(on) { this._fieldRenderer.toggleGravityVectors(on); }

    // ── Per-Particle Force Arrows ─────────────────────────────────────
    // Phase 3d: extracted to viewport/particle-renderer.js.
    updateParticleForces(positions, forces, count, maxForce, ids = null) {
        this._particleRenderer.updateParticleForces(positions, forces, count, maxForce, ids);
    }
    toggleParticleForces(on) { this._particleRenderer.toggleParticleForces(on); }
    updatePEForceDecomposition(decomp, gravityVisGain, ids = null) {
        this._particleRenderer.updatePEForceDecomposition(decomp, gravityVisGain, ids);
    }
    togglePEForceCoulomb(on) { this._particleRenderer.togglePEForceCoulomb(on); }
    togglePEForceGravity(on) { this._particleRenderer.togglePEForceGravity(on); }
    togglePEForceLorentz(on) { this._particleRenderer.togglePEForceLorentz(on); }
    togglePEForceExchange(on) { this._particleRenderer.togglePEForceExchange(on); }
    togglePEForceStrong(on)  { this._particleRenderer.togglePEForceStrong(on); }
    togglePEForceRadiation(on) { this._particleRenderer.togglePEForceRadiation(on); }
    togglePEForceMagneticDipole(on) { this._particleRenderer.togglePEForceMagneticDipole(on); }
    togglePEForceSpinOrbit(on) { this._particleRenderer.togglePEForceSpinOrbit(on); }
    togglePEForceNet(on)     { this._particleRenderer.togglePEForceNet(on); }

    // ── System Observables (center of mass + momentum p + ang.-mom. L) ──
    updatePESystem(com, p, l) { this._particleRenderer.updatePESystem(com, p, l); }
    togglePESystem(on) { this._particleRenderer.togglePESystem(on); }

    // ── Native-record admissibility ring overlay ────────────────────────
    updateAdmissibilityRings(peData, seedById, ids) {
        this._particleRenderer.updateAdmissibilityRings(peData, seedById, ids);
    }
    toggleAdmissibilityRings(on) { this._particleRenderer.toggleAdmissibilityRings(on); }

    // ── Native-record provenance label overlay ──────────────────────────
    updateProvenanceLabels(peData, seedById, ids) {
        this._particleRenderer.updateProvenanceLabels(peData, seedById, ids);
    }
    toggleProvenanceLabels(on) { this._particleRenderer.toggleProvenanceLabels(on); }

    setPEInspectionFocus(focus) {
        this._peInspectionFocus = focus || null;
        this._particleRenderer.setPEInspectionFocus(focus);
        this._fieldRenderer.clearPEInspectionGeometry();
    }

    setPEScenarioVisual(spec) { this._particleRenderer.setPEScenarioVisual(spec); }
    clearPEScenarioVisual() { this._particleRenderer.clearPEScenarioVisual(); }
    togglePEScenarioVisual(on) { this._particleRenderer.togglePEScenarioVisual(on); }

    getPEInspectionFocus() { return this._peInspectionFocus || null; }

    // ── Flux Volume Rendering (Scale 0 -- substrate mode) ──────────────
    // Phase 3b extracted into ViewportFluxRenderer (./viewport/flux-renderer.js).
    // This class keeps thin delegators for backward compatibility.
    _buildFluxVolume(latticeSize) { this._fluxRenderer._buildFluxVolume(latticeSize); }

    updateFluxVolume(volumeData, latticeSize, particleData = null) {
        this._fluxRenderer.updateFluxVolume(volumeData, latticeSize, particleData);
    }

    resetFluxNormalization() { this._fluxRenderer?.resetFluxNormalization(); }

    /**
     * Update the flux slice overlay from one or more 2D planes of flux
     * magnitudes. Owned by FieldRenderer's dedicated _fluxSliceMesh.
     */
    updateFluxSlice(sliceData, latticeSize, axis, index) {
        this._fieldRenderer.updateFluxSlice(sliceData, latticeSize, axis, index);
    }

    updateFluxSlices(planes, latticeSize, index) {
        this._fieldRenderer.updateFluxSlices(planes, latticeSize, index);
    }

    setLiveMeasure(patch) {
        if (!patch || !this._liveMeasure) return;
        Object.assign(this._liveMeasure, patch);
        if (Object.prototype.hasOwnProperty.call(patch, 'mooreHz')) this._telemetryAt = 0;
        this._updateLiveRulers();
    }

    /** Shared refresh rate for every Moore telemetry, including the side panel. */
    mooreTelemetryHz() {
        const hz = Number(this._liveMeasure?.mooreHz);
        if (!(hz > 0)) return 20;
        return Math.min(60, Math.max(1, hz));
    }

    _telemetryRefresh(now = performance.now()) {
        const gap = 1000 / this.mooreTelemetryHz();
        if (this._telemetryAt == null || now - this._telemetryAt >= gap) {
            this._telemetryAt = now;
            return true;
        }
        return false;
    }

    toggleFluxVolume(on) {
        this._fluxRenderer.toggleFluxVolume(on);
        this._updateLiveRulers();
    }

    toggleFluxSlice(on) {
        this._fieldRenderer.toggleFluxSlice(on);
        this.showHeatmap = on;
    }

    // ── Flux Volume Controls ──────────────────────────────────────────
    // Phase 3b — delegated to ViewportFluxRenderer.

    setFluxOpacity(val) { this._fluxRenderer.setFluxOpacity(val); }
    setFluxShape(shapeIndex) { this._fluxRenderer.setFluxShape(shapeIndex); }
    setFluxPointScale(scale) { this._fluxRenderer.setFluxPointScale(scale); }
    setFluxThreshold(val) { this._fluxRenderer.setFluxThreshold(val); }
    setFluxOrganic(on) { this._fluxRenderer.setFluxOrganic(on); }
    setFluxGlow(on) { this._fluxRenderer.setFluxGlow(on); }
    setScenarioScale(scale) { this._fluxRenderer.setScenarioScale(scale); }
    setFluxLatticeSpacing(val) { this._fluxRenderer.setFluxLatticeSpacing(val); }

    // ── Flux Slice Controls ───────────────────────────────────────────
    // Mirror the Flux Volume appearance controls onto the dedicated flux-slice
    // mesh (FieldRenderer), plus per-axis visibility for the all-axis overlay.
    setFluxSliceOpacity(val) { this._fieldRenderer.setFluxSliceOpacity(val); }
    setFluxSliceShape(shapeIndex) { this._fieldRenderer.setFluxSliceShape(shapeIndex); }
    setFluxSlicePointScale(scale) { this._fieldRenderer.setFluxSlicePointScale(scale); }
    setFluxSliceThreshold(val) { this._fieldRenderer.setFluxSliceThreshold(val); }
    setFluxSliceAxisEnabled(axis, on) { this._fieldRenderer.setFluxSliceAxisEnabled(axis, on); }
    getEnabledFluxSliceAxes() { return this._fieldRenderer.getEnabledFluxSliceAxes(); }

    _applyScenarioScale() {
        if (this._engineMode === 'lattice' || !this._engineMode) {
            const scale = this._scenarioScale || 1.0;
            const N = this.latticeSize || 32;
            const offset = (1 - scale) * N / 2;
            this.scene.scale.setScalar(scale);
            this.scene.position.set(offset, offset, offset);
        } else {
            this.scene.scale.setScalar(1);
            this.scene.position.set(0, 0, 0);
        }
    }

    // ══════════════════════════════════════════════════════════════════
    // ── Field Visualization Overlays (Scale 0) ───────────────────────
    // Phase 3c: every field overlay below is a thin delegator forwarding
    // to ViewportFieldRenderer (./viewport/field-renderer.js). Mesh-factory
    // helpers (_buildStreamlineMesh, _buildArrowFieldMesh,
    // _writeArrowFieldIntoMesh, _writeStreamlinesIntoMesh) live there too;
    // FluxRenderer + ParticleRenderer call them via bound callbacks set up
    // in this constructor.
    // ══════════════════════════════════════════════════════════════════

    // Mesh-factory helpers — preserved as delegators in case any external
    // call site references them by name. New code should call them on
    // `this._fieldRenderer` directly.
    _buildStreamlineMesh(maxVerts, opacity = 0.7) {
        return this._fieldRenderer.buildStreamlineMesh(maxVerts, opacity);
    }
    _buildArrowFieldMesh(maxArrows, opacity = 0.7) {
        return this._fieldRenderer.buildArrowFieldMesh(maxArrows, opacity);
    }
    _writeArrowFieldIntoMesh(mesh, fieldData, colors, magCacheKey, arrowBase = 1.5, thresholdFrac = 0.03) {
        return this._fieldRenderer.writeArrowFieldIntoMesh(mesh, fieldData, colors, magCacheKey, arrowBase, thresholdFrac);
    }
    _writeStreamlinesIntoMesh(mesh, streamlines, colorFn) {
        return this._fieldRenderer.writeStreamlinesIntoMesh(mesh, streamlines, colorFn);
    }

    // -- E-Field Lines (Cyan) --
    _buildEFieldLines() { this._fieldRenderer._buildEFieldLines(); }
    updateEFieldLines(streamlines, knotColoring) { this._fieldRenderer.updateEFieldLines(streamlines, knotColoring); }
    toggleEFieldLines(on) { this._fieldRenderer.toggleEFieldLines(on); }

    // -- B-Field Lines (Green) --
    _buildBFieldLines() { this._fieldRenderer._buildBFieldLines(); }
    updateBFieldLines(streamlines, knotColoring) { this._fieldRenderer.updateBFieldLines(streamlines, knotColoring); }
    toggleBFieldLines(on) { this._fieldRenderer.toggleBFieldLines(on); }

    setFlowLineOpacity(value) {
        this._fieldRenderer.setFlowLineOpacity(value);
        this._fluxRenderer.setFlowLineOpacity(value);
    }

    // -- Poynting Vectors (Yellow-Orange arrows) --
    _buildPoyntingVectors() { this._fieldRenderer._buildPoyntingVectors(); }
    updatePoyntingVectors(fieldData) { this._fieldRenderer.updatePoyntingVectors(fieldData); }
    togglePoyntingVectors(on) { this._fieldRenderer.togglePoyntingVectors(on); }

    // -- Divergence Field (Red-Blue dots) --
    _buildDivergenceField() { this._fieldRenderer._buildDivergenceField(); }
    updateDivergenceField(fieldData) { this._fieldRenderer.updateDivergenceField(fieldData); }
    toggleDivergenceField(on) { this._fieldRenderer.toggleDivergenceField(on); }

    // -- Flux Streamlines (flux colormap) --
    // Phase 3b -- delegated to ViewportFluxRenderer.
    _buildFluxStreamlines() { this._fluxRenderer._buildFluxStreamlines(); }
    updateFluxStreamlines(streamlines, maxFluxMag, mags) {
        this._fluxRenderer.updateFluxStreamlines(streamlines, maxFluxMag, mags);
    }
    toggleFluxStreamlines(on) { this._fluxRenderer.toggleFluxStreamlines(on); }

    // -- Native transport (lattice links; spec 2026-09-15) --
    toggleNativeTransport(on) {
        if (!on) this._linkEnergySample = null;
        this._nativeTransportRenderer?.setVisible(on);
    }
    updateNativeTransport(frame) {
        const sample = frame?.sample;
        const count = sample?.L > 0 ? sample.L ** 3 : 0;
        this._linkEnergySample = sample?.status === 'ok' && sample.links?.length === 9 * count
            ? sample
            : null;
        return this._nativeTransportRenderer?.update(frame) ?? null;
    }

    // -- EM Force Volume (Cyan arrows) --
    _buildForceVolume() { this._fieldRenderer._buildForceVolume(); }
    updateForceVolume(fieldData) { this._fieldRenderer.updateForceVolume(fieldData); }
    toggleForceVolume(on) { this._fieldRenderer.toggleForceVolume(on); }

    // -- Gravity Field Volume (density gradient vectors) --
    _buildGravityField() { this._fieldRenderer._buildGravityField(); }
    updateGravityField(fieldData) { this._fieldRenderer.updateGravityField(fieldData); }
    toggleGravityField(on) { this._fieldRenderer.toggleGravityField(on); }

    // -- Aliases for new badge naming --
    updateEMForceField(data) { this._fieldRenderer.updateEMForceField(data); }
    showEMForce(on) { this._fieldRenderer.showEMForce(on); }
    updateGravityForceField(data) { this._fieldRenderer.updateGravityForceField(data); }
    showGravityForce(on) { this._fieldRenderer.showGravityForce(on); }

    // -- Strong Force Volume (Red arrows) --
    _buildStrongForce() { this._fieldRenderer._buildStrongForce(); }
    updateStrongForceField(fieldData) { this._fieldRenderer.updateStrongForceField(fieldData); }
    toggleStrongForce(on) { this._fieldRenderer.toggleStrongForce(on); }
    showStrongForce(on) { this._fieldRenderer.showStrongForce(on); }

    // -- Weak Force Overlay --
    _buildWeakField() { this._fieldRenderer._buildWeakField(); }
    updateWeakField(fieldData) { this._fieldRenderer.updateWeakField(fieldData); }
    toggleWeakField(on) { this._fieldRenderer.toggleWeakField(on); }
    showWeakField(on) { this._fieldRenderer.showWeakField(on); }

    // -- Force visualization styles (heatmap / streamlines / glyphs) --
    _buildForceHeatmap() { this._fieldRenderer._buildForceHeatmap(); }
    initForceHeatmap() { this._fieldRenderer.initForceHeatmap(); }
    updateForceHeatmap(fieldData, forceType) { this._fieldRenderer.updateForceHeatmap(fieldData, forceType); }
    showForceHeatmap(visible) { this._fieldRenderer.showForceHeatmap(visible); }

    _buildForceStreamlines() { this._fieldRenderer._buildForceStreamlines(); }
    initForceStreamlines() { this._fieldRenderer.initForceStreamlines(); }
    updateForceStreamlines(lines, forceType) { this._fieldRenderer.updateForceStreamlines(lines, forceType); }
    animateForceStreamlines(dt) { this._fieldRenderer.animateForceStreamlines(dt); }
    showForceStreamlines_vis(visible) { this._fieldRenderer.showForceStreamlines_vis(visible); }
    clearForceVisualization(type, style) { this._fieldRenderer.clearForceVisualization(type, style); }

    _buildForceGlyphMesh(forceType) { return this._fieldRenderer._buildForceGlyphMesh(forceType); }
    _ensureForceGlyphInfra() { this._fieldRenderer._ensureForceGlyphInfra(); }
    _buildForceGlyphs() { this._fieldRenderer._buildForceGlyphs(); }
    initForceGlyphs() { this._fieldRenderer.initForceGlyphs(); }
    updateForceGlyphs(fieldData, forceType) { this._fieldRenderer.updateForceGlyphs(fieldData, forceType); }
    showForceGlyphs(visible) { this._fieldRenderer.showForceGlyphs(visible); }

    hideAllForceStyles() { this._fieldRenderer.hideAllForceStyles(); }
    showArrowForces(fieldState) { this._fieldRenderer.showArrowForces(fieldState); }

    // -- Dark Matter Halo Overlay --
    _buildDarkMatterHalo() { this._fieldRenderer._buildDarkMatterHalo(); }
    updateDarkMatterHalo(particles, fluxMag, latticeSize) { this._fieldRenderer.updateDarkMatterHalo(particles, fluxMag, latticeSize); }
    toggleDarkMatterHalo(on) { this._fieldRenderer.toggleDarkMatterHalo(on); }

    // -- Event Horizon Sphere (Scale 1 black hole scenario) --
    _buildEventHorizon() { this._fieldRenderer._buildEventHorizon(); }
    setEventHorizon(active, radius) { this._fieldRenderer.setEventHorizon(active, radius); }

    // -- Selective Damping Zones --
    _buildDampingZones() { this._fieldRenderer._buildDampingZones(); }
    updateDampingZones(particles, latticeSize) { this._fieldRenderer.updateDampingZones(particles, latticeSize); }
    toggleDampingZones(on) { this._fieldRenderer.toggleDampingZones(on); }
    updateKnotZones(particles, latticeSize) { this._fieldRenderer.updateKnotZones(particles, latticeSize); }
    toggleKnotZones(on) { this._fieldRenderer.toggleKnotZones(on); }

    // -- Genesis Threshold Isosurface --
    _buildGenesisIsosurface() { this._fieldRenderer._buildGenesisIsosurface(); }
    updateGenesisIsosurface(fluxMag, latticeSize, kGenesis) { this._fieldRenderer.updateGenesisIsosurface(fluxMag, latticeSize, kGenesis); }
    toggleGenesisIsosurface(on) { this._fieldRenderer.toggleGenesisIsosurface(on); }

    // -- Color-charge particle rendering (real genesis Voxel::color field) --
    // visualSettings is shared by reference with ViewportParticleRenderer,
    // so flipping this flag takes effect on the next updateParticles() call
    // with no extra invalidation needed.
    toggleColorChargeRender(on) { this.visualSettings.colorByColorCharge = !!on; }

    // -- Confinement Strings --
    _buildConfinementStrings() { this._fieldRenderer._buildConfinementStrings(); }
    updateConfinementStrings(bridge) { this._fieldRenderer.updateConfinementStrings(bridge); }
    toggleConfinement(on) { this._fieldRenderer.toggleConfinement(on); }

    // -- Dual Substrate Volume --
    _buildDualFluxVolume() { this._fieldRenderer._buildDualFluxVolume(); }
    updateDualFluxVolume(lData, rData) { this._fieldRenderer.updateDualFluxVolume(lData, rData); }
    toggleDualFluxVolume(on) { this._fieldRenderer.toggleDualFluxVolume(on); }

    // -- Chirality Field --
    _buildChiralityField() { this._fieldRenderer._buildChiralityField(); }
    updateChiralityField(fieldData) { this._fieldRenderer.updateChiralityField(fieldData); }
    toggleChiralityField(on) { this._fieldRenderer.toggleChiralityField(on); }

    // -- Quantum scaffolding --
    _buildSoftDiscTexture() { return this._fieldRenderer._buildSoftDiscTexture(); }
    _buildQuantumField() { this._fieldRenderer._buildQuantumField(); }
    _quantumSetVisibility() { this._fieldRenderer._quantumSetVisibility(); }
    _populateQuantumField(data, kind, options) { this._fieldRenderer._populateQuantumField(data, kind, options); }

    // -- Quantum overlays --
    togglePsiSquaredField(on) { this._fieldRenderer.togglePsiSquaredField(on); }
    updatePsiSquaredField(data) { this._fieldRenderer.updatePsiSquaredField(data); }
    _buildPhaseNeedles() { this._fieldRenderer._buildPhaseNeedles(); }
    togglePhaseField(on) { this._fieldRenderer.togglePhaseField(on); }
    updatePhaseField(data) { this._fieldRenderer.updatePhaseField(data); }
    toggleLagrangianDensityField(on) { this._fieldRenderer.toggleLagrangianDensityField(on); }
    updateLagrangianDensityField(data) { this._fieldRenderer.updateLagrangianDensityField(data); }
    toggleEntropyDensityField(on) { this._fieldRenderer.toggleEntropyDensityField(on); }
    updateEntropyDensityField(data) { this._fieldRenderer.updateEntropyDensityField(data); }

    // -- Horizon Field --
    _buildHorizonField() { this._fieldRenderer._buildHorizonField(); }
    toggleHorizonField(on) { this._fieldRenderer.toggleHorizonField(on); }
    updateHorizonField(data) { this._fieldRenderer.updateHorizonField(data); }

    // -- State field s (ternary {-1,0,+1} manifestation point cloud) --
    toggleStateField(on) { this._fieldRenderer.toggleStateField(on); }
    updateStateField(data) { this._fieldRenderer.updateStateField(data); }

    // -- Latency / time-dilation + Gauss-residual scalar point clouds --
    toggleLatencyField(on) { this._fieldRenderer.toggleLatencyField(on); }
    updateLatencyField(data) { this._fieldRenderer.updateLatencyField(data); }
    toggleGaussResidualField(on) { this._fieldRenderer.toggleGaussResidualField(on); }
    updateGaussResidualField(data) { this._fieldRenderer.updateGaussResidualField(data); }

    // -- Proper time τ / lapse dτ/dt / de Broglie phase φ (2026-09-03) --
    toggleProperTimeField(on) { this._fieldRenderer.toggleProperTimeField(on); }
    updateProperTimeField(data) { this._fieldRenderer.updateProperTimeField(data); }
    toggleLapseField(on) { this._fieldRenderer.toggleLapseField(on); }
    updateLapseField(data) { this._fieldRenderer.updateLapseField(data); }
    toggleDBPhaseField(on) { this._fieldRenderer.toggleDBPhaseField(on); }
    updateDBPhaseField(data) { this._fieldRenderer.updateDBPhaseField(data); }

    // -- Topological Sheet (deformable rubber-sheet) overlays --
    toggleGravPotentialField(on) { this._topoRenderer?.toggleGravPotential(on); }
    updateGravPotentialField(data) { this._topoRenderer?.updateGravPotential(data); }
    toggleEmEnergyField(on) { this._topoRenderer?.toggle('emEnergy', on); }
    updateEmEnergyField(data) { this._topoRenderer?.update('emEnergy', data); }
    toggleEPressureField(on) { this._topoRenderer?.toggle('ePressure', on); }
    updateEPressureField(data) { this._topoRenderer?.update('ePressure', data); }
    toggleBPressureField(on) { this._topoRenderer?.toggle('bPressure', on); }
    updateBPressureField(data) { this._topoRenderer?.update('bPressure', data); }
    toggleChargeDensityField(on) { this._topoRenderer?.toggle('chargeDensity', on); }
    updateChargeDensityField(data) { this._topoRenderer?.update('chargeDensity', data); }
    toggleVorticityField(on) { this._topoRenderer?.toggle('vorticity', on); }
    updateVorticityField(data) { this._topoRenderer?.update('vorticity', data); }
    // Volumetric scalar heat-map (overlays "Heat Map" meta-toggle) — the glow
    // clouds live on the field renderer's scalar-cloud pool (field-quantum-renderer).
    updateScalarHeatmap(key, data, ramp, signed) { this._fieldRenderer.updateScalarHeatmap(key, data, ramp, signed); }
    _scalarVolume(key) {
        this._scalarVolumes ??= new Map();
        if (!this._scalarVolumes.has(key)) this._scalarVolumes.set(key,
            createScalarVolumeRenderer(this.scene, () => this.latticeSize));
        return this._scalarVolumes.get(key);
    }
    updateScalarVolume(key, data, ramp, signed) {
        const volume = this._scalarVolume(key);
        if (!data) { volume.clear(); return; }
        // Vorticity is sampled only on interior voxel centres by the native
        // sampler. Do not invent an extra outer row of zero measurements.
        const frame = key === 'vorticity' ? { ...data,
            sampleGrid: visualSampleGrid(this.latticeSize, data.effectiveStride, true) } : data;
        volume.update(key, frame, { ramp, signed, normalizer: data.normalizer,
            opacity: this._scalarVolumeOpacity ?? 0.75 });
    }
    setScalarVolumeOpacity(value) {
        if (!Number.isFinite(value) || value < 0 || value > 1) return;
        this._scalarVolumeOpacity = value;
        for (const volume of this._scalarVolumes?.values() || []) volume.setOpacity(value);
    }
    showScalarVolume(key, on) {
        if (on) this._scalarVolume(key).show(key, true);
        else { this._scalarVolumes?.get(key)?.show(key, false); this._scalarVolumes?.get(key)?.clear(); }
    }
    clearScalarVolumes() { for (const volume of this._scalarVolumes?.values() || []) volume.clear(); }
    showScalarHeatmap(key, on) { this._fieldRenderer.showScalarHeatmap(key, on); }
    hideAllScalarHeatmaps() { this._fieldRenderer.hideAllScalarHeatmaps(); }
    // Slide a rubber-sheet overlay's slice plane up/down (frac 0..0.999 of the
    // box): the sheet floats at y=frac·N and re-samples the field at that height.
    // key ∈ {gravPotential, emEnergy, ePressure, bPressure, chargeDensity, vorticity}.
    setTopologySheetHeight(key, frac) { this._topoRenderer?.setHeight(key, frac); }

    // -- |psi|^2 breathing animation -- delegated; orchestrator forwards animation clock --
    _animateQuantumField() {
        this._fieldRenderer.setAnimationClock(this._animationClock || 0);
        this._fieldRenderer._animateQuantumField();
    }

    // Monotonic animation clock. Accumulates only when the sim is running;
    // the controller calls this each animate() tick with the frame delta
    // (wall-clock seconds). Anything time-based in viewport.js that should
    // freeze during pause reads from `this._animationClock` instead of
    // `performance.now()`.
    advanceAnimationClock(dtSeconds) {
        if (!this._animationClock) this._animationClock = 0;
        if (document.body.getAttribute('data-reduced-motion') !== '1') {
            this._animationClock += (dtSeconds || 0) * 1000;
        }
    }

    // ══════════════════════════════════════════════════════════════════
    // ── Molecular visuals ─ moved to viewport/molecular-renderer.js ──
    //
    // The MolecularRenderer class owns: nucleus shells, bond cylinders,
    // bond lines (above), orbital shells, orbital lobes, AE force
    // arrows, and element-label sprites. Viewport keeps thin delegators
    // for every method so external callers see no API change.
    //
    // `_defaultNeutronCount` is read once inside MolecularRenderer
    // .updateNucleusShells; a getter/setter pair on Viewport forwards
    // reads and writes so legacy external callers that set
    // `viewport._defaultNeutronCount = fn` continue to work.
    get _defaultNeutronCount() { return this._molRenderer?._defaultNeutronCount ?? null; }
    set _defaultNeutronCount(fn) { if (this._molRenderer) this._molRenderer._defaultNeutronCount = fn; }

    updateNucleusShells(atomData) { this._molRenderer.updateNucleusShells(atomData); }
    toggleNucleusShells(on)       { this._molRenderer.toggleNucleusShells(on); }
    updateNuclearEffects(data, options) { this._molRenderer.updateNuclearEffects(data, options); }
    toggleNuclearEvents(on)       { this._molRenderer.toggleNuclearEvents(on); }
    toggleNuclearRadiation(on)    { this._molRenderer.toggleNuclearRadiation(on); }
    toggleNuclearHeat(on)         { this._molRenderer.toggleNuclearHeat(on); }
    toggleNuclearBoundary(on)     { this._molRenderer.toggleNuclearBoundary(on); }

    updateBondCylinders(atomData)  { this._molRenderer.updateBondCylinders(atomData); }
    toggleBondCylinders(on)        { this._molRenderer.toggleBondCylinders(on); }
    updateOrbitalShells(atomData, electronConfigFn, slaterZeffFn, a0Display) {
        this._molRenderer.updateOrbitalShells(atomData, electronConfigFn, slaterZeffFn, a0Display);
    }
    toggleOrbitalShells(on)        { this._molRenderer.toggleOrbitalShells(on); }
    updateOrbitalLobes(atomData, electronConfigFn, slaterZeffFn, a0Display) {
        this._molRenderer.updateOrbitalLobes(atomData, electronConfigFn, slaterZeffFn, a0Display);
    }
    toggleOrbitalLobes(on)         { this._molRenderer.toggleOrbitalLobes(on); }
    updateAEForces(positions, forceData, count) {
        this._molRenderer.updateAEForces(positions, forceData, count);
    }
    toggleAEForceIonic(on)         { this._molRenderer.toggleAEForceIonic(on); }
    toggleAEForceVdw(on)           { this._molRenderer.toggleAEForceVdw(on); }
    toggleAEForceBond(on)          { this._molRenderer.toggleAEForceBond(on); }
    toggleAEForceHBond(on)         { this._molRenderer.toggleAEForceHBond(on); }
    toggleAEForceAngle(on)         { this._molRenderer.toggleAEForceAngle(on); }
    toggleAEForceDipole(on)        { this._molRenderer.toggleAEForceDipole(on); }
    toggleAEForceNet(on)           { this._molRenderer.toggleAEForceNet(on); }
    updateAEDipoles(positions, dipoles, count) {
        this._molRenderer.updateAEDipoles(positions, dipoles, count);
    }
    toggleAEDipoles(on)            { this._molRenderer.toggleAEDipoles(on); }
    updateHBondLines(segments, count) {
        this._molRenderer.updateHBondLines(segments, count);
    }
    toggleHBondLines(on)           { this._molRenderer.toggleHBondLines(on); }

    // ══════════════════════════════════════════════════════════════════

    // Switch between lattice wireframe (Scale 0), coordinate axes (Scale 1), atom view (Scale 2), molecule view (Scale 3)
    setEngineMode(mode) {
        this.clearScalarVolumes();
        this._engineMode = mode;

        // Helper to hide all overlays from ALL scales unconditionally
        const hideAllOverlays = () => {
            if (this.wireframe) this.wireframe.visible = false;
            if (this.axes) this.axes.visible = false;
            if (this.peAxes) this.peAxes.visible = false;
            if (this.peGrid) this.peGrid.visible = false;
            if (this.particles) this.particles.visible = false;
            if (this.velocityVectors) this.velocityVectors.visible = false;
            if (this.trails) this.trails.visible = false;
            this._particleRenderer?.togglePEScenarioVisual(false);
            // Molecular renderer owns bondLines, bondCylinders, nucleusShells,
            // orbitalShells, orbitalLobes, element labels, and AE force arrows.
            this._molRenderer?.setAllVisible(false);
            if (this._fieldHeatmap) this._fieldHeatmap.visible = false;
            if (this._fieldVectors) this._fieldVectors.visible = false;
            // Scale 0 specific visuals
            if (this._fluxVolume) this._fluxVolume.visible = false;
            if (this._fluxSlice) this._fluxSlice.visible = false;
            if (this._eFieldLines) this._eFieldLines.visible = false;
            if (this._bFieldLines) this._bFieldLines.visible = false;
            if (this._poyntingVectors) this._poyntingVectors.visible = false;
            if (this._divField) this._divField.visible = false;
            if (this._fluxStreamlines) this._fluxStreamlines.visible = false;
            if (this._forceVolume) this._forceVolume.visible = false;
            if (this._gravityField) this._gravityField.visible = false;
            if (this._strongForce) this._strongForce.visible = false;
            if (this._weakField) this._weakField.visible = false;
            if (this._forceHeatmap) this._forceHeatmap.visible = false;
            if (this._forceGlyphMeshes) {
                for (const m of Object.values(this._forceGlyphMeshes)) m.visible = false;
            }
            if (this._forceStreamlinePool) {
                for (const l of this._forceStreamlinePool) l.visible = false;
            }
            if (this._dualFluxVolume) this._dualFluxVolume.visible = false;
            if (this._chiralityField) this._chiralityField.visible = false;
            if (this._darkMatterHalo) this._darkMatterHalo.visible = false;
            if (this._dampingZones) this._dampingZones.visible = false;
            if (this._genesisIsosurface) this._genesisIsosurface.visible = false;
            if (this._confinementStrings) this._confinementStrings.visible = false;
            // Boundary box
            if (this.wireframe) this.wireframe.visible = false;
        };

        // ── Cosmic / planetary / meta: hide lattice overlays; those scales
        // own their own cameras and meshes.
        if (mode === 'cosmic' || mode === 'planetary' || mode === 'meta') {
            hideAllOverlays();
            return;
        }



        // Defensive post-processing cleanup (no current scale uses it).
        if (this._usePostProcessing) {
            this.disablePostProcessing();
            this.scene.background = new THREE.Color(0x0f1729);
            this.camera.fov = 45;
            this.camera.updateProjectionMatrix();
        }

        if (mode === 'particles' || mode === 'atoms' || mode === 'molecules') {
            hideAllOverlays();
            // Rebuild boundary at origin for PE/AE/molecule modes
            this._boundaryMode = 'origin';
            this._buildBoundary(this._boundaryShape, 'origin');
            if (!this.peAxes) this._buildPEAxes();
            this.peAxes.visible = this._showAxes;
            if (this.peGrid) this.peGrid.visible = this._showGrid;
            this.particles.visible = true; // Particles point cloud used by Scale 1, 2, 3
            this._particleRenderer?.togglePEScenarioVisual(mode === 'particles');
            if (this.wireframe) this.wireframe.visible = this.showWireframe;

            // Recenter camera at origin
            this.controls.target.set(0, 0, 0);
            this.camera.position.set(40, 30, 40);
            this.controls.update();

            const isAtomMol = (mode === 'atoms' || mode === 'molecules');
            // bondCylinders / bondLight / nucleusShells / element labels are atom-scale visuals
            this._molRenderer?.setAtomMolVisible(isAtomMol);
        } else {
            hideAllOverlays();
            // Rebuild boundary at lattice center for Scale 0
            this._boundaryMode = 'lattice';
            this._buildBoundary(this._boundaryShape, 'lattice');
            if (this.axes) this.axes.visible = this._showAxes;
            this.particles.visible = true; // Lattice fallback point cloud
            if (this.wireframe) this.wireframe.visible = this.showWireframe;

            if (this._fieldHeatmap) this._fieldHeatmap.visible = this.showHeatmap;
            // Restore flux volume/slice if enabled
            if (this._fluxVolume) this._fluxVolume.visible = this.showFlux;
            if (this._fluxSlice) this._fluxSlice.visible = this.showSlice ?? false;

            // Returning to Scale 0 is another default-camera boundary: restore
            // the canonical face-on framing rather than the former isometric.
            this.setCameraPreset('front');
        }
        if (this._applyScenarioScale) this._applyScenarioScale();
    }

    // Phase 3a: extracted to viewport/scene-core.js.
    _buildPEAxes() { this._sceneCore?._buildPEAxes(); }

    // Phase 3d: updateParticles / setPointShape / setOpacity /
    // applyParticleColors extracted to viewport/particle-renderer.js.
    updateParticles(data) { this._particleRenderer.updateParticles(data); }
    setPointShape(shapeIndex) { this._particleRenderer.setPointShape(shapeIndex); }
    setOpacity(val) { this._particleRenderer.setOpacity(val); }
    setParticleSizes(positive, negative) {
        const positiveChanged = this.visualSettings.positiveSize !== positive;
        const negativeChanged = this.visualSettings.negativeSize !== negative;
        if (!positiveChanged && !negativeChanged) return;
        this.visualSettings.positiveSize = positive;
        this.visualSettings.negativeSize = negative;
        this._particleRenderer?.updateParticleSizes();
    }
    setPositiveSize(val) { this.setParticleSizes(val, this.visualSettings.negativeSize); }
    setNegativeSize(val) { this.setParticleSizes(this.visualSettings.positiveSize, val); }
    setParticleOpacity(val) {
        this._particleRenderer.setOpacity(val);
    }
    setParticleGlow(val) { this._particleRenderer.setGlow(val); }
    setParticleShape(idx) { this._particleRenderer.setPointShape(idx); }
    setPEManifestation(enabled, timeSec, fill) {
        this._particleRenderer.setManifestation(enabled, timeSec, fill);
    }
    setAreaHighlight(cx, cy, cz, radius, active) { this._sceneCore?.setAreaHighlight(cx, cy, cz, radius, active); }

    // ── Element labels + clearMolecularMeshes — delegated to viewport/molecular-renderer.js
    updateElementLabels(labels) { this._molRenderer.updateElementLabels(labels); }
    toggleElementLabels(on)     { this._molRenderer.toggleElementLabels(on); }
    clearElementLabels()        { this._molRenderer.clearElementLabels(); }
    clearMolecularMeshes()      { this._molRenderer.clearMolecularMeshes(); }

    applyParticleColors(data, typeMap) {
        this._particleRenderer.applyParticleColors(data, typeMap);
    }

    // ── Post-Processing ────────────────────────────────────────────
    // Phase 3a: extracted to viewport/scene-core.js. Thin delegators
    // preserve the public API for any caller that opts into bloom.

    enablePostProcessing() { this._sceneCore?.enablePostProcessing(); }

    disablePostProcessing() { this._sceneCore?.disablePostProcessing(); }

    getBloomPass() { return this._sceneCore?.getBloomPass() ?? null; }

    setBloomParams(params) { this._sceneCore?.setBloomParams(params); }

    /** Suspend presentation while retaining scene, camera and simulation owner. */
    setPresentationSuspended(suspended) {
        this.presentationSuspended = !!suspended;
        this._fluxRenderer?.setPresentationSuspended(suspended);
    }

    /** Alt scales pan, orbit, and zoom down so a close view can be nudged. */
    _setOrbitFine(event) {
        if (event && event.key !== 'Alt') return;
        const editing = event?.target instanceof Element
            && event.target.closest('input, textarea, select, [contenteditable]:not([contenteditable="false"])');
        if (event?.type === 'keydown' && editing) return;
        if (event?.type === 'keydown') event.preventDefault();
        const next = event?.type === 'keydown';
        if (this._orbitFine === next) return;
        this._orbitFine = next;
        const distance = this.camera?.position.distanceTo(this.controls?.target);
        if (distance > 0) this._tuneOrbitSensitivity(distance);
    }

    /**
     * Pan and orbit deltas are sized for the current camera distance. Far from
     * the subject a small drag flings the target, and that leftover motion is
     * still applied after dollying back in. Scale the speeds with distance and
     * shrink any queued delta as the camera closes. Alt applies another 0.2 scale.
     */
    _tuneOrbitSensitivity(distance) {
        const controls = this.controls;
        const base = this._orbitBase;
        if (!controls || !base || !(distance > 0)) return;
        const fine = this._orbitFine ? 0.2 : 1;
        if (this._liveMeasure?.smoothOrbit === false) {
            controls.panSpeed = base.pan * fine;
            controls.rotateSpeed = base.rotate * fine;
            controls.zoomSpeed = base.zoom * Math.max(1, this._farZoomGain(distance)) * fine;
            return;
        }
        const ref = Math.max(this.getReferenceDistance(), 1);
        const ratio = distance / ref;
        const gain = Math.min(1, Math.max(0.12, 1 / Math.sqrt(Math.max(ratio, 1))));
        const zoomGain = Math.min(1.15, Math.max(0.4, 1 / Math.sqrt(Math.max(ratio, 0.35))));
        controls.panSpeed = base.pan * gain * fine;
        controls.rotateSpeed = base.rotate * gain * fine;
        // Past the quasi-domain there are twenty powers of ten with nothing
        // simulated in them. The wheel speeds up as the shell shrinks away,
        // and slows again on the way back in.
        controls.zoomSpeed = base.zoom * Math.max(zoomGain, this._farZoomGain(distance)) * fine;
        const far = 1 + Math.log2(Math.max(ratio, 1));
        controls.dampingFactor = base.damp / Math.min(far, 4);
        const previous = this._lastOrbitDist;
        if (previous > distance) {
            const shrink = distance / previous;
            controls._panOffset.multiplyScalar(shrink);
            controls._sphericalDelta.theta *= shrink;
            controls._sphericalDelta.phi *= shrink;
        }
        this._lastOrbitDist = distance;
    }

    /** Wheel-zoom gain for the empty range beyond the quasi-domain (lattice view only). */
    _farZoomGain(distance) {
        if (this._engineMode && this._engineMode !== 'lattice') return 0;
        const shell = this._shellDiameterUnits() * (this.scene.scale.x || 1);
        // The gain starts where the quasi-domain view sits (the shell about
        // fills the view) and reaches its ceiling a decade and a half out.
        const decades = Math.log10(distance / Math.max(shell * 1.5, 1));
        return decades > 0 ? Math.min(FAR_ZOOM_GAIN_MAX, decades * FAR_ZOOM_GAIN_PER_DECADE) : 0;
    }

    render() {
        if (this.presentationSuspended) return;
        // Dynamically adjust camera.far to prevent culling at extreme zoom out
        const dist = this.camera.position.distanceTo(this.controls.target);
        this.controls.maxDistance = this._zoomOutLimit();
        this._tuneOrbitSensitivity(dist);
        const desiredFar = Math.max(this._baseFar || 2000, dist * 5);
        // Far out, the near plane follows the camera so the depth range the
        // GPU works with stays representable in single precision.
        const desiredNear = dist > 1e6 ? dist * 1e-6 : 0.001;
        if (this._actualFar !== desiredFar || this.camera.near !== desiredNear) {
            this._actualFar = desiredFar;
            this.camera.near = desiredNear;
            this.camera.updateProjectionMatrix();
        }

        this.controls.update();
        // Animation clock is advanced externally via advanceAnimationClock()
        // so this call is safe to make unconditionally — it just reads the
        // current clock value and paints. When the controller has frozen
        // the clock (sim paused), opacity stays pinned and no "one-step"
        // advance is perceivable on overlay-toggle-triggered repaints.
        this._animateQuantumField();
        // Spin-arrow primitive update — slerps orientations + advances
        // axial-spin angle for any tracked particles. dtMs gates per-arrow
        // ω·dt accumulation; passed in so all arrows share the same clock.
        if (this.spinArrowManager) {
            const now = performance.now();
            let dtMs = now - (this._lastSpinArrowUpdateMs || now);
            if (document.body.getAttribute('data-reduced-motion') === '1') {
                dtMs = 0; // Freezes theta rotation while retaining slerp/lerp positional follow
            }
            this.spinArrowManager.update(dtMs);
            this._lastSpinArrowUpdateMs = now;
        }
        // SceneCore decides composer-vs-renderer based on _usePostProcessing.
        this._sceneCore?.render(this.scene, this.camera);
        this._updateLiveRulers(false);
    }

    /**
     * Has anything the rulers are drawn from changed since the last update?
     * Camera pose and projection, orbit target, scene and flux-cloud
     * placement, view size, lattice, mode, attached voxel, drawn point count.
     */
    _liveRulerViewChanged(rect) {
        const flux = this._fluxRenderer?._fluxVolume;
        const last = this._rulerView || (this._rulerView = { values: new Float64Array(34), mode: null });
        const next = this._rulerViewNext || (this._rulerViewNext = new Float64Array(34));
        next.set(this.camera.matrixWorld.elements, 0);
        next[16] = this.camera.projectionMatrix.elements[0];
        next[17] = this.camera.projectionMatrix.elements[5];
        next[18] = this.controls.target.x;
        next[19] = this.controls.target.y;
        next[20] = this.controls.target.z;
        next[21] = this.scene.scale.x;
        next[22] = this.scene.position.x;
        next[23] = this.scene.position.y;
        next[24] = this.scene.position.z;
        next[25] = flux ? flux.scale.x : 0;
        next[26] = flux ? flux.position.x : 0;
        next[27] = flux?.geometry?.drawRange?.count ?? -1;
        next[28] = rect.width;
        next[29] = rect.height;
        next[30] = this.latticeSize || this._latticeSize || 0;
        next[31] = this.showFlux === false ? 0 : 1;
        next[32] = this._attachedPoint ?? -1;
        next[33] = this._shellDiameterUnits();
        let changed = last.mode !== this._engineMode;
        for (let i = 0; i < next.length && !changed; i++) changed = next[i] !== last.values[i];
        if (changed) {
            last.values.set(next);
            last.mode = this._engineMode;
        }
        return changed;
    }

    /**
     * Redraw the rulers and Moore readouts. The per-frame call passes
     * `force = false` and returns early unless the telemetry is due or the
     * view moved; every other caller (a setting, a toggle, a framed view)
     * forces the update.
     */
    _updateLiveRulers(force = true) {
        if (!this._liveRulers) return;
        const refresh = this._telemetryRefresh();
        // Size is kept current by _onResize; no layout read per frame.
        const rect = this._viewSize || this.container.getBoundingClientRect();
        const moved = this._liveRulerViewChanged(rect);
        if (!force && !refresh && !moved) return;
        const distance = this.camera.position.distanceTo(this.controls.target);
        this._liveRulers.update({
            fovDeg: this.camera.fov,
            distance,
            viewWidth: rect.width,
            viewHeight: rect.height,
            scenarioScale: this.scene.scale.x || this._scenarioScale || 1,
            latticeSize: this.latticeSize || this._latticeSize || 1,
            engineMode: this._engineMode,
            projectUnits: (units) => this._projectUnits(units, rect.width, rect.height),
            projectDiameter: (units) => this._projectDiameter(units, rect.width, rect.height),
            shellDiameter: this._shellDiameterUnits(),
            pointSprite: this._pointSprite(rect.width, rect.height, refresh),
            voxelClocks: this._voxelClocks(rect.width, rect.height, refresh),
            mooreNeighborhood: this._mooreNeighborhood(rect.width, rect.height, refresh),
            latticeCentre: this._latticeCentreOnScreen(rect.width, rect.height),
            fluxVisible: this.showFlux !== false,
            measures: this._liveMeasure,
        });
    }

    /** Screen point of a lattice coordinate. Matrices are already current. */
    _projectLattice(x, y, z, viewWidth, viewHeight) {
        const flux = this._fluxRenderer?._fluxVolume;
        if (!flux || !Number.isFinite(x)) return null;
        const world = this._pointWorld || (this._pointWorld = new THREE.Vector3());
        world.set(x, y, z).applyMatrix4(flux.matrixWorld);
        const ndc = this._pointNdc || (this._pointNdc = new THREE.Vector3());
        ndc.copy(world).project(this.camera);
        if (ndc.z < -1 || ndc.z > 1) return null;
        return {
            x: (ndc.x * 0.5 + 0.5) * viewWidth,
            y: (-ndc.y * 0.5 + 0.5) * viewHeight,
        };
    }

    /** Screen width of the flux point the view is attached to, updated from its live size. */
    _pointSprite(viewWidth, viewHeight, refresh = true) {
        if (!refresh && this._spriteFrame) {
            const frame = this._spriteFrame;
            const flux = this._fluxRenderer?._fluxVolume;
            if (flux && frame.lx != null && frame.screen) {
                flux.updateMatrixWorld();
                this.camera.updateMatrixWorld();
                const placed = this._projectLattice(frame.lx, frame.ly, frame.lz, viewWidth, viewHeight);
                if (placed) {
                    frame.screen.left = placed.x;
                    frame.screen.top = placed.y - frame.px / 2 - 20;
                }
            }
            return frame;
        }
        const scale = this._fluxRenderer?._fluxPointScale ?? 1;
        const ceiling = pointAttributeSize(scale);
        const flux = this._fluxRenderer?._fluxVolume;
        const hit = flux && this._attachedFluxPoint(flux, viewWidth, viewHeight);
        const size = hit ? hit.size : ceiling;
        const depth = hit ? hit.depth : Math.max(this.camera.position.distanceTo(this.controls.target), 0.1);
        const px = pointSpritePixels(size, depth);
        const t = (size - 1) / Math.max(ceiling - 1, 1e-6);
        const screen = hit ? {
            left: hit.x,
            top: hit.y - px / 2 - 20,
            width: px,
            height: px,
            placed: true,
        } : null;
        const position = flux?.geometry?.getAttribute('position');
        const focus = this._attachedPoint;
        const frame = {
            px,
            fill: spectrumColor(t),
            screen,
            name: hit?.name || '',
            wave: hit ? hit.wave : { value: 0, min: 0, max: 0, samples: [] },
            index: focus,
            lx: position && focus != null ? position.getX(focus) : null,
            ly: position && focus != null ? position.getY(focus) : null,
            lz: position && focus != null ? position.getZ(focus) : null,
        };
        this._spriteFrame = frame;
        return frame;
    }

    /**
     * One flux point per cell, in the point cloud's x-fastest order (not the
     * link sample's siteIndex). `source` holds exact cell centres. -1 if that
     * cell is absent, -2 if this buffer is not that grid.
     */
    _fluxCellIndex(source, count, visibility, x, y, z) {
        return fluxCellIndex(
            source, count, this.latticeSize || this._latticeSize || 0, visibility?.array ?? null, x, y, z,
        );
    }

    /** The 27 (or 26) cells around a lattice centre. Null when the buffer is not a dense grid. */
    _neighborhoodIndices(source, count, visibility, ax, ay, az, includeSelf) {
        return mooreCellIndices(
            source, count, this.latticeSize || this._latticeSize || 0, visibility?.array ?? null,
            ax, ay, az, includeSelf,
        );
    }

    /** SVG path for a trace, rebuilt only when a sample is pushed. */
    _sealWave(trace, ev) {
        if (trace.pathGeneration !== trace.generation) {
            trace.path = energyStringPath(trace.samples, trace.min, trace.max);
            trace.area = energyAreaPath(trace.samples, trace.min, trace.max);
            trace.pathGeneration = trace.generation;
        }
        return {
            value: ev,
            min: trace.min,
            max: trace.max,
            samples: trace.samples,
            path: trace.path,
            area: trace.area,
        };
    }

    /**
     * Global tick plus a phase function. The neighborhood mean environment
     * keeps the global rate; voxels above or below it run relatively faster or slower.
     */
    _clockReading(items) {
        const tick = this._sceneCore?._globalTick || 0;
        let mean = 0;
        let lo = Infinity;
        let hi = -Infinity;
        for (let i = 0; i < items.length; i++) {
            const value = items[i].joules;
            mean += value;
            if (value < lo) lo = value;
            if (value > hi) hi = value;
        }
        mean = items.length ? mean / items.length : 0;
        const span = Number.isFinite(lo) ? hi - lo : 0;
        const viewport = this;
        return {
            tick,
            globalPhase: globalClockPhase(tick),
            phase(index, value) {
                const offset = viewport._clockOffset(
                    index, tick, relativeClockEnvironment(value, mean, span),
                );
                return voxelClockPhase(tick, offset);
            },
        };
    }

    /** Accumulated turn offset. A new scenario, or the first sample, starts at zero. */
    _clockOffset(index, tick, relative) {
        if (!this._clockOffsets) this._clockOffsets = new Map();
        const ordinal = Math.max(0, Math.trunc(Number(tick) || 0));
        if (this._clockOffsetTick == null || ordinal < this._clockOffsetTick) {
            this._clockOffsets.clear();
            this._clockOffsetTick = ordinal;
        }
        let entry = this._clockOffsets.get(index);
        if (!entry || ordinal < entry.tick) entry = { offset: 0, tick: ordinal };
        const running = Boolean(this._sceneCore?._globalClockRunning);
        if (running && ordinal > entry.tick) {
            entry.offset += clockEnvironmentStep(ordinal - entry.tick, relative);
        }
        entry.tick = ordinal;
        this._clockOffsets.set(index, entry);
        this._clockOffsetTick = ordinal;
        return entry.offset;
    }

    /** Clock faces for voxels near the one under the view. Farther faces fade out. */
    _voxelClocks(viewWidth, viewHeight, refresh = true) {
        const distance = this.camera.position.distanceTo(this.controls.target);
        if (distance > Math.max(this.getReferenceDistance(), 1) * 0.35) {
            this._clockFrame = null;
            return [];
        }
        const flux = this._fluxRenderer?._fluxVolume;
        const geometry = flux?.geometry;
        const position = geometry?.getAttribute('position');
        if (!position) return [];
        const count = geometry.drawRange ? geometry.drawRange.count : position.count;
        if (count <= 0) {
            this._clockFrame = null;
            return [];
        }
        flux.updateMatrixWorld();
        this.camera.updateMatrixWorld();
        if (!refresh && this._clockFrame) {
            for (const clock of this._clockFrame) {
                const placed = this._projectLattice(clock.lx, clock.ly, clock.lz, viewWidth, viewHeight);
                if (!placed) continue;
                clock.x = placed.x;
                clock.y = placed.y;
            }
            return this._clockFrame;
        }
        const inverse = this._fluxInv || (this._fluxInv = new THREE.Matrix4());
        inverse.copy(flux.matrixWorld).invert();
        const local = this._pointLocal || (this._pointLocal = new THREE.Vector3());
        local.copy(this.controls.target).applyMatrix4(inverse);
        const arr = position.array;
        // Cells are matched on their exact centres. `position` is where the
        // dot is drawn, which Organic mode moves by up to half a cell.
        const source = geometry.getAttribute('sourcePosition')?.array || arr;
        const visibility = geometry.getAttribute('particleVisibility');
        const shown = visibility?.array;
        const focus = this._attachedPoint;
        const origin = focus != null && focus >= 0 && focus < count
            ? [source[focus * 3], source[focus * 3 + 1], source[focus * 3 + 2]]
            : [local.x, local.y, local.z];
        let found = this._neighborhoodIndices(source, count, visibility, origin[0], origin[1], origin[2], true);
        if (!found) {
            // Not a dense lattice grid (a compact native sample): scan it.
            found = [];
            for (let i = 0; i < count; i++) {
                if (shown && shown[i] < 0.5) continue;
                const i3 = i * 3;
                const dx = source[i3] - origin[0];
                const dy = source[i3 + 1] - origin[1];
                const dz = source[i3 + 2] - origin[2];
                const span = Math.max(Math.abs(dx), Math.abs(dy), Math.abs(dz));
                if (span > 1.25) continue;
                found.push({ index: i, d: dx * dx + dy * dy + dz * dz });
            }
        }
        found.sort((a, b) => a.d - b.d);
        const pending = [];
        const reach = Math.sqrt(3);
        for (let n = 0; n < found.length && pending.length < 27; n++) {
            const index = found[n].index;
            const i3 = index * 3;
            const world = this._pointWorld || (this._pointWorld = new THREE.Vector3());
            world.set(arr[i3], arr[i3 + 1], arr[i3 + 2]).applyMatrix4(flux.matrixWorld);
            const ndc = this._pointNdc || (this._pointNdc = new THREE.Vector3());
            ndc.copy(world).project(this.camera);
            if (ndc.z < -1 || ndc.z > 1) continue;
            const fade = 1 - Math.sqrt(found[n].d) / reach;
            pending.push({
                index,
                x: (ndc.x * 0.5 + 0.5) * viewWidth,
                y: (-ndc.y * 0.5 + 0.5) * viewHeight,
                opacity: Math.max(0.45, Math.min(1, fade)),
                joules: this._mooreScalars(index).joules,
                lx: arr[i3],
                ly: arr[i3 + 1],
                lz: arr[i3 + 2],
            });
        }
        const reading = this._clockReading(pending);
        this._clockReadingCache = reading;
        const clocks = [];
        for (let n = 0; n < pending.length; n++) {
            const item = pending[n];
            clocks.push({
                index: item.index,
                x: item.x,
                y: item.y,
                opacity: item.opacity,
                phase: reading.phase(item.index, item.joules),
                lx: item.lx,
                ly: item.ly,
                lz: item.lz,
            });
        }
        this._clockFrame = clocks;
        return clocks;
    }

    /** The 26 lattice sites around the attached voxel. Only the close Moore view, not a zoom-out. */
    _mooreNeighborhood(viewWidth, viewHeight, refresh = true) {
        const measures = this._liveMeasure || {};
        const perSite = measures.mooreBars !== false || measures.mooreWaves !== false || measures.mooreJoules !== false;
        const wantEnergy = measures.mooreEnergy !== false;
        if (!perSite && !wantEnergy) return this._mooreHidden();
        const distance = this.camera.position.distanceTo(this.controls.target);
        if (distance > Math.max(this.getReferenceDistance(), 1) * 0.2) return this._mooreHidden();
        const flux = this._fluxRenderer?._fluxVolume;
        const geometry = flux?.geometry;
        const position = geometry?.getAttribute('position');
        const sizes = geometry?.getAttribute('size');
        if (!position || !sizes) return this._mooreHidden();
        const count = geometry.drawRange ? geometry.drawRange.count : position.count;
        const focus = this._attachedPoint;
        if (count <= 0 || focus == null || focus >= count) return this._mooreHidden();
        if (!refresh && this._mooreFrame && this._mooreFrame.focusIndex === focus) {
            return this._placeMooreFrame(this._mooreFrame, viewWidth, viewHeight);
        }
        const arr = position.array;
        // Drawn position of the focus (for on-screen placement) and its exact
        // cell centre (for identity: neighbours, links, published coordinates).
        const ax = arr[focus * 3];
        const ay = arr[focus * 3 + 1];
        const az = arr[focus * 3 + 2];
        const source = geometry.getAttribute('sourcePosition')?.array || arr;
        const sx = source[focus * 3];
        const sy = source[focus * 3 + 1];
        const sz = source[focus * 3 + 2];
        const visibility = geometry.getAttribute('particleVisibility');
        const shown = visibility?.array;
        const ceiling = pointAttributeSize(this._fluxRenderer?._fluxPointScale ?? 1);
        const sites = [];
        const records = [];
        const focusScalars = this._mooreScalars(focus);
        let sum = focusScalars.joules;
        flux.updateMatrixWorld();
        this.camera.updateMatrixWorld();
        let neighbors = 0;
        let indexed = this._neighborhoodIndices(source, count, visibility, sx, sy, sz, false);
        if (!indexed) {
            // Not a dense lattice grid (a compact native sample): scan it.
            indexed = [];
            for (let i = 0; i < count && indexed.length < 26; i++) {
                if (i === focus) continue;
                if (shown && shown[i] < 0.5) continue;
                const i3 = i * 3;
                const span = Math.max(Math.abs(source[i3] - sx), Math.abs(source[i3 + 1] - sy), Math.abs(source[i3 + 2] - sz));
                if (span < 0.25 || span > 1.25) continue;
                indexed.push({ index: i });
            }
        }
        for (let n = 0; n < indexed.length && neighbors < 26; n++) {
            const i = indexed[n].index;
            const i3 = i * 3;
            const world = this._pointWorld || (this._pointWorld = new THREE.Vector3());
            world.set(arr[i3], arr[i3 + 1], arr[i3 + 2]).applyMatrix4(flux.matrixWorld);
            const view = this._pointView || (this._pointView = new THREE.Vector3());
            view.copy(world).applyMatrix4(this.camera.matrixWorldInverse);
            const ndc = this._pointNdc || (this._pointNdc = new THREE.Vector3());
            ndc.copy(world).project(this.camera);
            if (ndc.z < -1 || ndc.z > 1) continue;
            const size = sizes.getX(i);
            const depth = Math.max(-view.z, 0.1);
            const px = pointSpritePixels(size, depth);
            const scalars = this._mooreScalars(i);
            const wave = this._neighborEnergy(i, scalars.joules);
            const reading = this._clockReadingCache;
            sum += wave.value;
            neighbors += 1;
            records.push({
                x: source[i3], y: source[i3 + 1], z: source[i3 + 2],
                name: manifestedSiteName(this._fluxRenderer?._fluxSiteKind?.[i] || 0),
                joules: wave.value,
                activation: scalars.activation,
                flux: scalars.flux,
                size: px,
                phase: reading ? reading.phase(i, scalars.joules) : 0,
                tick: reading ? reading.tick : 0,
            });
            const t = (size - 1) / Math.max(ceiling - 1, 1e-6);
            if (!perSite) continue;
            sites.push({
                x: (ndc.x * 0.5 + 0.5) * viewWidth,
                y: (-ndc.y * 0.5 + 0.5) * viewHeight - px / 2 - 8,
                px,
                fill: spectrumColor(t),
                opacity: 0.9,
                bar: measures.mooreBars !== false,
                wave: measures.mooreWaves === false ? null : wave,
                showJoule: measures.mooreJoules !== false,
                joule: wave.value,
                lx: arr[i3],
                ly: arr[i3 + 1],
                lz: arr[i3 + 2],
                name: manifestedSiteName(this._fluxRenderer?._fluxSiteKind?.[i] || 0),
            });
        }
        const energy = wantEnergy ? this._mooreEnergyRuler(focus, ax, ay, az, sum, flux, viewWidth, viewHeight, sites) : null;
        const publishedSum = energy ? energy.value : sum;
        const neighborhoodLinks = this._mooreLinks(sx, sy, sz);
        const focusReading = this._clockReadingCache;
        const focusPhase = focusReading ? focusReading.phase(focus, focusScalars.joules) : 0;
        const focusTick = focusReading ? focusReading.tick : 0;
        let signature = `${focus}|${neighbors}|${publishedSum}|${focusScalars.flux}|${focusScalars.activation}|${focusPhase.toFixed(4)}|${focusTick}|${neighborhoodLinks ? neighborhoodLinks.signature : 'off'}`;
        for (let i = 0; i < records.length; i++) {
            signature += `|${records[i].joules}|${records[i].flux}|${records[i].activation}|${records[i].phase.toFixed(4)}`;
        }
        if (signature !== this._moorePublishSig) {
            this._moorePublishSig = signature;
            this._publishMoore({
                focus: {
                    x: sx, y: sy, z: sz,
                    name: manifestedSiteName(this._fluxRenderer?._fluxSiteKind?.[focus] || 0),
                    joules: focusScalars.joules,
                    activation: focusScalars.activation,
                    flux: focusScalars.flux,
                    phase: focusPhase,
                    tick: focusTick,
                },
                count: neighbors,
                sum: publishedSum,
                min: energy?.min ?? sum,
                max: energy?.max ?? sum,
                sites: records,
                links: neighborhoodLinks,
            });
        }
        const frame = { sites, energy, focusIndex: focus, ax, ay, az };
        this._mooreFrame = frame;
        return frame;
    }

    /** Move the last telemetry sample with the camera. Values stay on the shared rate. */
    _placeMooreFrame(frame, viewWidth, viewHeight) {
        const flux = this._fluxRenderer?._fluxVolume;
        if (!flux) return frame;
        flux.updateMatrixWorld();
        this.camera.updateMatrixWorld();
        for (const site of frame.sites) {
            const placed = this._projectLattice(site.lx, site.ly, site.lz, viewWidth, viewHeight);
            if (!placed) continue;
            site.x = placed.x;
            site.y = placed.y - site.px / 2 - 8;
        }
        if (frame.energy) {
            const placed = this._mooreEnergyRuler(
                frame.focusIndex, frame.ax, frame.ay, frame.az,
                frame.energy.value, flux, viewWidth, viewHeight, frame.sites, true,
            );
            frame.energy.x = placed.x;
            frame.energy.y = placed.y;
            frame.energy.width = placed.width;
        }
        return frame;
    }

    _mooreHidden() {
        this._moorePublishSig = '';
        this._mooreFrame = null;
        if (this._mooreSnapshot !== null) this._publishMoore(null);
        return { sites: [], energy: null };
    }

    /**
     * Links already held by the native-transport overlay, limited to segments
     * whose both ends sit in this Moore neighborhood. Null when that overlay
     * has no sample. This does not request a new lattice read.
     */
    _mooreLinks(ax, ay, az) {
        const sample = this._linkEnergySample;
        if (!sample) return null;
        const axis = sample.L | 0;
        const values = sample.links;
        const fx = Math.round(ax - 0.5);
        const fy = Math.round(ay - 0.5);
        const fz = Math.round(az - 0.5);
        const links = [];
        let max = 0;
        let signature = '0';
        for (let oz = -1; oz <= 1; oz++) {
            for (let oy = -1; oy <= 1; oy++) {
                for (let ox = -1; ox <= 1; ox++) {
                    const x = fx + ox;
                    const y = fy + oy;
                    const z = fz + oz;
                    if (x < 0 || y < 0 || z < 0 || x >= axis || y >= axis || z >= axis) continue;
                    const base = siteIndex(axis, x, y, z) * 9;
                    for (let k = 0; k < LINK_DISPLACEMENT.length; k++) {
                        const step = LINK_DISPLACEMENT[k];
                        const nx = ox + step[0];
                        const ny = oy + step[1];
                        const nz = oz + step[2];
                        if (Math.abs(nx) > 1 || Math.abs(ny) > 1 || Math.abs(nz) > 1) continue;
                        const value = Number(values[base + k]);
                        if (!value) continue;
                        max = Math.max(max, Math.abs(value));
                        links.push({
                            ox, oy, oz,
                            dx: step[0], dy: step[1], dz: step[2],
                            value,
                        });
                        signature += `|${ox}${oy}${oz}${k}:${value}`;
                    }
                }
            }
        }
        return { links, max, signature };
    }

    /** Energy, activation amplitude, and flux magnitude already stored for this site. */
    _mooreScalars(index) {
        const activation = Number(this._fluxRenderer?._fluxActivation?.[index]);
        const flux = Number(this._fluxRenderer?._fluxDensitySnapshot?.[index]);
        return {
            joules: activationEnergyEv(activation),
            activation: Number.isFinite(activation) ? activation : 0,
            flux: Number.isFinite(flux) ? flux : 0,
        };
    }

    /** Panel charts run after this frame paints, so the labels are not left behind the view. */
    _publishMoore(detail) {
        this._mooreSnapshot = detail;
        this._mooreDeferredDetail = detail;
        if (this._mooreDeferTimer) return;
        this._mooreDeferTimer = setTimeout(() => {
            this._mooreDeferTimer = 0;
            const next = this._mooreDeferredDetail;
            this._mooreDeferredDetail = undefined;
            window.dispatchEvent(new CustomEvent('ftd:moore-neighborhood', { detail: next }));
        }, 0);
    }

    /** Screen ruler across the 3-voxel Moore block, with the summed energy. */
    _mooreEnergyRuler(focus, ax, ay, az, sum, flux, viewWidth, viewHeight, sites, hold = false) {
        const trace = hold && this._mooreEnergyTrace
            ? this._mooreEnergyTrace
            : this._trackMooreEnergy(focus, sum);
        const wave = this._sealWave(trace, trace.last);
        const unit = this._worldPerLatticeUnit();
        const scratch = this._mooreRuler || (this._mooreRuler = {
            center: new THREE.Vector3(),
            right: new THREE.Vector3(),
            up: new THREE.Vector3(),
            point: new THREE.Vector3(),
            view: new THREE.Vector3(),
        });
        const center = scratch.center.set(ax, ay, az).applyMatrix4(flux.matrixWorld);
        scratch.right.setFromMatrixColumn(this.camera.matrixWorld, 0).normalize().multiplyScalar(1.5 * unit);
        scratch.up.setFromMatrixColumn(this.camera.matrixWorld, 1).normalize().multiplyScalar(1.5 * unit);
        const project = (rx, uz) => {
            scratch.point.copy(center);
            if (rx) scratch.point.addScaledVector(scratch.right, rx);
            if (uz) scratch.point.addScaledVector(scratch.up, uz);
            scratch.point.project(this.camera);
            return {
                x: (scratch.point.x * 0.5 + 0.5) * viewWidth,
                y: (-scratch.point.y * 0.5 + 0.5) * viewHeight,
            };
        };
        const left = project(-1, 0);
        const edge = project(1, 0);
        const mid = project(0, 0);
        const cap = project(0, 1);
        let crown = Math.min(mid.y, cap.y);
        for (const site of sites || []) crown = Math.min(crown, site.y - 56);
        const sizes = flux.geometry?.getAttribute('size');
        if (sizes) {
            const depth = Math.max(-scratch.view.copy(center).applyMatrix4(this.camera.matrixWorldInverse).z, 0.1);
            const focusPx = pointSpritePixels(sizes.getX(focus), depth);
            crown = Math.min(crown, mid.y - focusPx / 2 - 20 - 96);
        }
        return {
            x: (left.x + edge.x) / 2,
            y: crown - 18,
            width: Math.max(48, Math.hypot(edge.x - left.x, edge.y - left.y)),
            value: wave.value,
            min: wave.min,
            max: wave.max,
            samples: wave.samples,
            area: wave.area,
        };
    }

    _trackMooreEnergy(index, electronVolts) {
        const ev = Number.isFinite(electronVolts) ? electronVolts : 0;
        let trace = this._mooreEnergyTrace;
        if (!trace || trace.index !== index) {
            trace = this._mooreEnergyTrace = { index, min: ev, max: ev, last: ev, samples: [ev], generation: 1 };
        } else if (ev !== trace.last) {
            trace.samples.push(ev);
            if (trace.samples.length > 48) trace.samples.shift();
            trace.last = ev;
            if (ev < trace.min) trace.min = ev;
            if (ev > trace.max) trace.max = ev;
            trace.generation += 1;
        }
        return trace;
    }

    _neighborEnergy(index, electronVolts) {
        const ev = Number.isFinite(electronVolts) ? electronVolts : 0;
        if (!this._neighborTraces) this._neighborTraces = new Map();
        let trace = this._neighborTraces.get(index);
        if (!trace) {
            trace = { min: ev, max: ev, last: ev, samples: [ev], generation: 1 };
            this._neighborTraces.set(index, trace);
        } else if (ev !== trace.last) {
            trace.samples.push(ev);
            if (trace.samples.length > 24) trace.samples.shift();
            trace.last = ev;
            if (ev < trace.min) trace.min = ev;
            if (ev > trace.max) trace.max = ev;
            trace.generation += 1;
        }
        if (this._neighborTraces.size > 64) {
            const first = this._neighborTraces.keys().next().value;
            this._neighborTraces.delete(first);
        }
        return this._sealWave(trace, ev);
    }

    /** Recent energy of one voxel. The string spans that voxel's own min and max. */
    _trackVoxelEnergy(index, electronVolts) {
        const ev = Number.isFinite(electronVolts) ? electronVolts : 0;
        let trace = this._voxelEnergyTrace;
        if (!trace || trace.index !== index) {
            trace = this._voxelEnergyTrace = { index, min: ev, max: ev, last: ev, samples: [ev], generation: 1 };
        } else if (ev !== trace.last) {
            trace.samples.push(ev);
            if (trace.samples.length > 48) trace.samples.shift();
            trace.last = ev;
            if (ev < trace.min) trace.min = ev;
            if (ev > trace.max) trace.max = ev;
            trace.generation += 1;
        }
        return this._sealWave(trace, ev);
    }

    /** Lattice point nearest the orbit target. Holds that point while it stays under the view. */
    _attachedFluxPoint(flux, viewWidth, viewHeight) {
        const geometry = flux.geometry;
        const position = geometry?.getAttribute('position');
        const sizes = geometry?.getAttribute('size');
        if (!position || !sizes) return null;
        const count = geometry.drawRange ? geometry.drawRange.count : position.count;
        if (count <= 0) return null;
        flux.updateMatrixWorld();
        this.camera.updateMatrixWorld();
        const inverse = this._fluxInv || (this._fluxInv = new THREE.Matrix4());
        inverse.copy(flux.matrixWorld).invert();
        const local = this._pointLocal || (this._pointLocal = new THREE.Vector3());
        local.copy(this.controls.target).applyMatrix4(inverse);
        const arr = position.array;
        const visibility = geometry.getAttribute('particleVisibility');
        let index = this._attachedPoint;
        const keep = index != null && index < count
            && this._pointDistance2(arr, index, local) < 0.75
            && (!visibility || visibility.getX(index) >= 0.5);
        if (!keep) {
            // Nearest visible dot to the orbit target: a few shells of cells
            // around the target on a dense grid, not every point.
            const best = nearestVisiblePoint(
                arr, geometry.getAttribute('sourcePosition')?.array || arr, count,
                this.latticeSize || this._latticeSize || 0, visibility?.array ?? null,
                local.x, local.y, local.z,
            );
            if (best < 0) return null;
            index = best;
            this._attachedPoint = index;
        }
        const i3 = index * 3;
        const world = this._pointWorld || (this._pointWorld = new THREE.Vector3());
        world.set(arr[i3], arr[i3 + 1], arr[i3 + 2]).applyMatrix4(flux.matrixWorld);
        const view = this._pointView || (this._pointView = new THREE.Vector3());
        view.copy(world).applyMatrix4(this.camera.matrixWorldInverse);
        const ndc = this._pointNdc || (this._pointNdc = new THREE.Vector3());
        ndc.copy(world).project(this.camera);
        const width = viewWidth || this.container.clientWidth || 1;
        const height = viewHeight || this.container.clientHeight || 1;
        const activation = this._fluxRenderer?._fluxActivation?.[index];
        return {
            size: sizes.getX(index),
            depth: Math.max(-view.z, 0.1),
            x: (ndc.x * 0.5 + 0.5) * width,
            y: (-ndc.y * 0.5 + 0.5) * height,
            wave: this._trackVoxelEnergy(index, activationEnergyEv(activation)),
            name: manifestedSiteName(this._fluxRenderer?._fluxSiteKind?.[index] || 0),
        };
    }

    _pointDistance2(arr, index, local) {
        const i3 = index * 3;
        const dx = arr[i3] - local.x;
        const dy = arr[i3 + 1] - local.y;
        const dz = arr[i3 + 2] - local.z;
        return dx * dx + dy * dy + dz * dz;
    }

    /** Lattice-unit diameter of the environment background sphere. */
    _shellDiameterUnits() {
        return this._environmentShell().diameter;
    }

    /** Bounding diameter of the active environment background, in scene-local units. */
    _environmentShell() {
        const group = this.scene.children.find((child) => typeof child.name === 'string' && child.name.startsWith('bg-'));
        const scale = this.scene.scale.x || 1;
        const origin = this.scene.position;
        if (!group) return { diameter: 1000, center: new THREE.Vector3() };
        if (this._envShell && this._envShellId === group.uuid) {
            return this._envShell;
        }
        const box = this._envBox || (this._envBox = new THREE.Box3());
        box.setFromObject(group);
        if (box.isEmpty()) return { diameter: 1000, center: new THREE.Vector3() };
        const size = this._envSize || (this._envSize = new THREE.Vector3());
        const center = this._envCenter || (this._envCenter = new THREE.Vector3());
        box.getSize(size);
        box.getCenter(center);
        center.sub(origin).divideScalar(scale);
        this._envShellId = group.uuid;
        this._envShell = { diameter: Math.max(size.x, size.y, size.z) / scale, center: center.clone() };
        return this._envShell;
    }

    /** Screen span of a diameter through the environment center, perpendicular to the view. */
    _projectDiameter(diameterUnits, viewWidth, viewHeight) {
        const shell = this._environmentShell();
        const scale = this.scene.scale.x || 1;
        const origin = this.scene.position;
        const local = shell.center;
        const center = this._rulerCenter || (this._rulerCenter = new THREE.Vector3());
        center.set(origin.x + scale * local.x, origin.y + scale * local.y, origin.z + scale * local.z);
        this.camera.updateMatrixWorld();
        const right = this._rulerRight || (this._rulerRight = new THREE.Vector3());
        right.set(1, 0, 0).applyQuaternion(this.camera.quaternion).multiplyScalar((diameterUnits * scale) / 2);
        const up = this._rulerUp || (this._rulerUp = new THREE.Vector3());
        up.set(0, 1, 0).applyQuaternion(this.camera.quaternion).multiplyScalar((diameterUnits * scale) / 2);
        const edge = center.clone().sub(right);
        const other = center.clone().add(right);
        const crown = center.clone().add(up);
        const toScreen = (point) => {
            const v = point.project(this.camera);
            return {
                x: (v.x * 0.5 + 0.5) * viewWidth,
                y: (-v.y * 0.5 + 0.5) * viewHeight,
                ok: v.z >= -1 && v.z <= 1,
            };
        };
        const a = toScreen(edge);
        const b = toScreen(other);
        const top = toScreen(crown);
        if (!a.ok || !b.ok) return null;
        const left = Math.min(a.x, b.x);
        return { left, top: top.ok ? top.y : Math.min(a.y, b.y), width: Math.abs(a.x - b.x), height: Math.abs(a.y - b.y) };
    }

    /** Screen box of a cube `units` across, in the lattice's own coordinates. */
    _projectUnits(units, viewWidth, viewHeight) {
        const span = Math.max(0, units);
        const lattice = !this._engineMode || this._engineMode === 'lattice';
        const N = this.latticeSize || this._latticeSize || span;
        const scale = this.scene.scale.x || 1;
        const origin = this.scene.position;
        const center = lattice ? N / 2 : 0;
        const min = lattice && Math.abs(span - N) < 0.5 ? 0 : center - span / 2;
        const max = lattice && Math.abs(span - N) < 0.5 ? N : center + span / 2;
        this.camera.updateMatrixWorld();
        const v = this._rulerCorner || (this._rulerCorner = new THREE.Vector3());
        let left = Infinity, right = -Infinity, top = Infinity, bottom = -Infinity;
        for (const x of [min, max]) {
            for (const y of [min, max]) {
                for (const z of [min, max]) {
                    v.set(origin.x + scale * x, origin.y + scale * y, origin.z + scale * z);
                    v.project(this.camera);
                    if (v.z < -1 || v.z > 1) continue;
                    const sx = (v.x * 0.5 + 0.5) * viewWidth;
                    const sy = (-v.y * 0.5 + 0.5) * viewHeight;
                    left = Math.min(left, sx);
                    right = Math.max(right, sx);
                    top = Math.min(top, sy);
                    bottom = Math.max(bottom, sy);
                }
            }
        }
        if (!Number.isFinite(left) || right <= left) return null;
        return { left, top, width: right - left, height: bottom - top };
    }

    _onResize() {
        const rect = this.container.getBoundingClientRect();
        const w = rect.width;
        const h = rect.height;
        if (w === 0 || h === 0) return;
        this._viewSize = { width: w, height: h };
        this.camera.aspect = w / h;
        this.camera.updateProjectionMatrix();
        this.renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
        this.renderer.setSize(w, h);
        // SceneCore resizes its composer (no-op when post-processing disabled).
        this._sceneCore?.onResize(w, h);
    }

    dispose() {
        for (const volume of this._scalarVolumes?.values() || []) volume.dispose();
        this._scalarVolumes?.clear();
        if (this._disposed) return;
        this._disposed = true;
        document.removeEventListener('keydown', this._onOrbitFineKey, true);
        document.removeEventListener('keyup', this._onOrbitFineKey, true);
        window.removeEventListener('blur', this._onOrbitFineBlur);
        this._liveRulers?.dispose();
        this._liveRulers = null;
        this._resizeObserver.disconnect();
        this.controls.dispose();

        // Helper: dispose geometry+material for any Three.js Object3D
        const disposeMesh = (obj) => {
            if (!obj) return;
            this.scene.remove(obj);
            if (obj.geometry) obj.geometry.dispose();
            if (obj.material) {
                if (obj.material.map) obj.material.map.dispose();
                obj.material.dispose();
            }
        };

        // Sub-renderer dispose cascade — every sub-renderer tears down its
        // own meshes / materials / textures. Order: leaf renderers first
        // (their meshes are children of the scene); MolecularRenderer +
        // SpinArrowManager + TopologySheetRenderer next; FieldRenderer next
        // (Phase 3c — owns 27+ field overlays); SceneCore last (it owns
        // wireframe, axes, post-processing pipeline, highlights).
        // Phase 3b: FluxRenderer owns _fluxVolume + _fluxStreamlines.
        this._fluxRenderer?.dispose();
        this._nativeTransportRenderer?.dispose();
        // Phase 3d: ParticleRenderer owns particles, velocityVectors,
        // trails, _particleForces.
        this._particleRenderer?.dispose();

        // Molecular renderer owns: bondLines, _bondCylinders, _bondLight,
        // _nucleusShells, _orbitalShells, _orbitalLobes,
        // _aeForceIonic/Vdw/Bond/Net, and element labels.
        this._molRenderer?.dispose();

        // Rubber-sheet visualizations (10 topology sheets + Φ gravitational
        // potential) are owned by TopologySheetRenderer — it tears them down.
        this._topoRenderer?.dispose();
        // Spin-arrow primitives (per-tracked-particle arrows).
        this.spinArrowManager?.dispose();

        // Phase 3c: FieldRenderer disposes _fieldHeatmap, _fieldVectors,
        // _peStreamlines, _gravityVectors, _eFieldLines, _bFieldLines,
        // _poyntingVectors, _divField, _forceVolume, _gravityField,
        // _strongForce, _weakField, _forceHeatmap, _forceStreamlinePool,
        // _forceGlyphMeshes, _darkMatterHalo, _eventHorizonSphere/Ring,
        // _dampingZones, _genesisIsosurface, _confinementStrings,
        // _dualFluxVolume, _chiralityField, _quantumField,
        // _phaseNeedles, _horizonField, plus instance-owned soft-disc texture.
        this._fieldRenderer?.dispose();

        // Raycasting (orchestrator-owned bounding volume — used by inspector)
        disposeMesh(this._voidBox);

        // Phase 3a: SceneCore disposes wireframe, axes, peAxes, peGrid,
        // _voxelHighlight and post-processing composer/bloom.
        this._sceneCore?.dispose();

        // Renderer last (after every sub-renderer has freed its GPU resources).
        this.renderer.dispose();
        this.renderer.domElement.remove();
    }

    // ── Backward-compat getters/setters for Phase 3b extracted state ──
    // External code (zoomToFit, hideAllOverlays, _engineMode handlers,
    // setLatticeSize field-overlay sweep, etc.) reads these directly.
    // They forward to the FluxRenderer so the existing call sites keep
    // working without renaming. Remove these once all readers move to
    // `viewport._fluxRenderer.X`.
    get _fluxVolume() { return this._fluxRenderer?._fluxVolume; }
    set _fluxVolume(v) { if (this._fluxRenderer) this._fluxRenderer._fluxVolume = v; }
    get _fluxVolumeSize() { return this._fluxRenderer?._fluxVolumeSize ?? 0; }
    set _fluxVolumeSize(v) { if (this._fluxRenderer) this._fluxRenderer._fluxVolumeSize = v; }
    get _fluxStreamlines() { return this._fluxRenderer?._fluxStreamlines; }
    set _fluxStreamlines(v) { if (this._fluxRenderer) this._fluxRenderer._fluxStreamlines = v; }
    get _fluxPointScale() { return this._fluxRenderer?._fluxPointScale ?? 1.0; }
    set _fluxPointScale(v) { if (this._fluxRenderer) this._fluxRenderer._fluxPointScale = v; }
    get _fluxThreshold() { return this._fluxRenderer?._fluxThreshold ?? DEFAULT_FLUX_THRESHOLD; }
    set _fluxThreshold(v) { if (this._fluxRenderer) this._fluxRenderer._fluxThreshold = v; }
    get _scenarioScale() { return this._fluxRenderer?._scenarioScale ?? 1.0; }
    set _scenarioScale(v) { if (this._fluxRenderer) this._fluxRenderer._scenarioScale = v; }
    get _fluxLatticeSpacing() { return this._fluxRenderer?._fluxLatticeSpacing ?? 1.0; }
    set _fluxLatticeSpacing(v) { if (this._fluxRenderer) this._fluxRenderer._fluxLatticeSpacing = v; }
    get showFlux() { return this._fluxRenderer?.showFlux ?? true; }
    set showFlux(v) { if (this._fluxRenderer) this._fluxRenderer.showFlux = v; }

    // ── Backward-compat getters/setters for Phase 3d extracted state ──
    // inspector.js reads viewport.particles for raycasting; the scale-N
    // controllers toggle viewport.particles.visible directly. Forward to
    // ParticleRenderer so existing call sites keep working without
    // renaming. Remove these once all readers move to
    // `viewport._particleRenderer.X`.
    get particles() { return this._particleRenderer?.particles ?? null; }
    set particles(v) { if (this._particleRenderer) this._particleRenderer.particles = v; }
    get velocityVectors() { return this._particleRenderer?.velocityVectors ?? null; }
    set velocityVectors(v) { if (this._particleRenderer) this._particleRenderer.velocityVectors = v; }
    get trails() { return this._particleRenderer?.trails ?? null; }
    set trails(v) { if (this._particleRenderer) this._particleRenderer.trails = v; }
    get _particleForces() { return this._particleRenderer?._particleForces ?? null; }
    set _particleForces(v) { if (this._particleRenderer) this._particleRenderer._particleForces = v; }
    // visualSettings is shared by reference between Viewport and
    // ParticleRenderer — both sides read/write the same object — so it
    // remains a plain own-property on Viewport (no getter/setter needed).

    // ── Backward-compat getters/setters for Phase 3c extracted state ──
    // setEngineMode's hideAllOverlays helper, the dispose() flow, and
    // various external panels (scale-N controllers, app.js, etc.) read
    // these fields directly. Forward to FieldRenderer so the existing call
    // sites keep working without renaming. Remove once readers move to
    // `viewport._fieldRenderer.X`.
    get _fieldHeatmap() { return this._fieldRenderer?._fieldHeatmap ?? null; }
    set _fieldHeatmap(v) { if (this._fieldRenderer) this._fieldRenderer._fieldHeatmap = v; }
    get _fieldVectors() { return this._fieldRenderer?._fieldVectors ?? null; }
    set _fieldVectors(v) { if (this._fieldRenderer) this._fieldRenderer._fieldVectors = v; }
    get _peStreamlines() { return this._fieldRenderer?._peStreamlines ?? null; }
    set _peStreamlines(v) { if (this._fieldRenderer) this._fieldRenderer._peStreamlines = v; }
    get _gravityVectors() { return this._fieldRenderer?._gravityVectors ?? null; }
    set _gravityVectors(v) { if (this._fieldRenderer) this._fieldRenderer._gravityVectors = v; }
    get _eFieldLines() { return this._fieldRenderer?._eFieldLines ?? null; }
    set _eFieldLines(v) { if (this._fieldRenderer) this._fieldRenderer._eFieldLines = v; }
    get _bFieldLines() { return this._fieldRenderer?._bFieldLines ?? null; }
    set _bFieldLines(v) { if (this._fieldRenderer) this._fieldRenderer._bFieldLines = v; }
    get _poyntingVectors() { return this._fieldRenderer?._poyntingVectors ?? null; }
    set _poyntingVectors(v) { if (this._fieldRenderer) this._fieldRenderer._poyntingVectors = v; }
    get _divField() { return this._fieldRenderer?._divField ?? null; }
    set _divField(v) { if (this._fieldRenderer) this._fieldRenderer._divField = v; }
    get _forceVolume() { return this._fieldRenderer?._forceVolume ?? null; }
    set _forceVolume(v) { if (this._fieldRenderer) this._fieldRenderer._forceVolume = v; }
    get _gravityField() { return this._fieldRenderer?._gravityField ?? null; }
    set _gravityField(v) { if (this._fieldRenderer) this._fieldRenderer._gravityField = v; }
    get _strongForce() { return this._fieldRenderer?._strongForce ?? null; }
    set _strongForce(v) { if (this._fieldRenderer) this._fieldRenderer._strongForce = v; }
    get _weakField() { return this._fieldRenderer?._weakField ?? null; }
    set _weakField(v) { if (this._fieldRenderer) this._fieldRenderer._weakField = v; }
    get _forceHeatmap() { return this._fieldRenderer?._forceHeatmap ?? null; }
    set _forceHeatmap(v) { if (this._fieldRenderer) this._fieldRenderer._forceHeatmap = v; }
    get _forceStreamlinePool() { return this._fieldRenderer?._forceStreamlinePool ?? null; }
    set _forceStreamlinePool(v) { if (this._fieldRenderer) this._fieldRenderer._forceStreamlinePool = v; }
    get _forceStreamlineMats() { return this._fieldRenderer?._forceStreamlineMats ?? null; }
    set _forceStreamlineMats(v) { if (this._fieldRenderer) this._fieldRenderer._forceStreamlineMats = v; }
    get _forceGlyphMeshes() { return this._fieldRenderer?._forceGlyphMeshes ?? null; }
    set _forceGlyphMeshes(v) { if (this._fieldRenderer) this._fieldRenderer._forceGlyphMeshes = v; }
    get _darkMatterHalo() { return this._fieldRenderer?._darkMatterHalo ?? null; }
    set _darkMatterHalo(v) { if (this._fieldRenderer) this._fieldRenderer._darkMatterHalo = v; }
    get _eventHorizonSphere() { return this._fieldRenderer?._eventHorizonSphere ?? null; }
    set _eventHorizonSphere(v) { if (this._fieldRenderer) this._fieldRenderer._eventHorizonSphere = v; }
    get _eventHorizonRing() { return this._fieldRenderer?._eventHorizonRing ?? null; }
    set _eventHorizonRing(v) { if (this._fieldRenderer) this._fieldRenderer._eventHorizonRing = v; }
    get _dampingZones() { return this._fieldRenderer?._dampingZones ?? null; }
    set _dampingZones(v) { if (this._fieldRenderer) this._fieldRenderer._dampingZones = v; }
    get _genesisIsosurface() { return this._fieldRenderer?._genesisIsosurface ?? null; }
    set _genesisIsosurface(v) { if (this._fieldRenderer) this._fieldRenderer._genesisIsosurface = v; }
    get _confinementStrings() { return this._fieldRenderer?._confinementStrings ?? null; }
    set _confinementStrings(v) { if (this._fieldRenderer) this._fieldRenderer._confinementStrings = v; }
    get _dualFluxVolume() { return this._fieldRenderer?._dualFluxVolume ?? null; }
    set _dualFluxVolume(v) { if (this._fieldRenderer) this._fieldRenderer._dualFluxVolume = v; }
    get _chiralityField() { return this._fieldRenderer?._chiralityField ?? null; }
    set _chiralityField(v) { if (this._fieldRenderer) this._fieldRenderer._chiralityField = v; }
    get _quantumField() { return this._fieldRenderer?._quantumField ?? null; }
    set _quantumField(v) { if (this._fieldRenderer) this._fieldRenderer._quantumField = v; }
    get _phaseNeedles() { return this._fieldRenderer?._phaseNeedles ?? null; }
    set _phaseNeedles(v) { if (this._fieldRenderer) this._fieldRenderer._phaseNeedles = v; }
    get _horizonField() { return this._fieldRenderer?._horizonField ?? null; }
    set _horizonField(v) { if (this._fieldRenderer) this._fieldRenderer._horizonField = v; }
    // Note: showHeatmap is owned BOTH by Viewport (toggleFluxSlice writes it
    // at the orchestrator level for setEngineMode lookup) AND by
    // FieldRenderer (toggleFluxSlice's internal copy). The orchestrator's
    // copy remains a plain own-property so setEngineMode reads it directly.

    // ── Backward-compat getters/setters for Phase 3a extracted state ──
    // External code reads / writes these via the orchestrator (notably
    // setEngineMode in this same class, plus zoomToFit which reads
    // _boundaryMode). Forward to SceneCore so existing call sites keep
    // working without renaming.
    get wireframe() { return this._sceneCore?.wireframe ?? null; }
    set wireframe(v) { if (this._sceneCore) this._sceneCore.wireframe = v; }
    get showWireframe() { return this._sceneCore?.showWireframe ?? true; }
    set showWireframe(v) { if (this._sceneCore) this._sceneCore.showWireframe = v; }
    get _wireframeBrightness() { return this._sceneCore?._wireframeBrightness ?? 0.18; }
    set _wireframeBrightness(v) { if (this._sceneCore) this._sceneCore._wireframeBrightness = v; }
    get axes() { return this._sceneCore?.axes ?? null; }
    set axes(v) { if (this._sceneCore) this._sceneCore.axes = v; }
    get peAxes() { return this._sceneCore?.peAxes ?? null; }
    set peAxes(v) { if (this._sceneCore) this._sceneCore.peAxes = v; }
    get peGrid() { return this._sceneCore?.peGrid ?? null; }
    set peGrid(v) { if (this._sceneCore) this._sceneCore.peGrid = v; }
    get _showAxes() { return this._sceneCore?._showAxes ?? true; }
    set _showAxes(v) { if (this._sceneCore) this._sceneCore._showAxes = v; }
    get _showGrid() { return this._sceneCore?._showGrid ?? true; }
    set _showGrid(v) { if (this._sceneCore) this._sceneCore._showGrid = v; }
    get _engineMode() { return this._sceneCore?._engineMode ?? 'lattice'; }
    set _engineMode(v) { this._sceneCore?.setEngineMode(v); }
    get _boundaryShape() { return this._sceneCore?._boundaryShape ?? 'cube'; }
    set _boundaryShape(v) { if (this._sceneCore) this._sceneCore._boundaryShape = v; }
    get _boundaryMode() { return this._sceneCore?._boundaryMode ?? 'lattice'; }
    set _boundaryMode(v) { if (this._sceneCore) this._sceneCore._boundaryMode = v; }
    get _voxelHighlight() { return this._sceneCore?._voxelHighlight ?? null; }
    set _voxelHighlight(v) { if (this._sceneCore) this._sceneCore._voxelHighlight = v; }
    get _composer() { return this._sceneCore?._composer ?? null; }
    set _composer(v) { if (this._sceneCore) this._sceneCore._composer = v; }
    get _bloomPass() { return this._sceneCore?._bloomPass ?? null; }
    set _bloomPass(v) { if (this._sceneCore) this._sceneCore._bloomPass = v; }
    get _usePostProcessing() { return this._sceneCore?._usePostProcessing ?? false; }
    set _usePostProcessing(v) { if (this._sceneCore) this._sceneCore._usePostProcessing = v; }
}
