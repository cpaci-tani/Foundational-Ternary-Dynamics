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
import { ViewportSceneCore } from './viewport/scene-core.js';
// Rubber-sheet visualizations (gravitational potential + 10 topology fields).
// Extracted per refactoring-analyst RF-1. Viewport holds the instance as
// this._topoRenderer and forwards via thin delegators.
import { TopologySheetRenderer } from './viewport/topology-sheet-renderer.js';
// Flux volume + flux streamlines extracted as Phase 3b of the viewport
// decomposition. Viewport composes a ViewportFluxRenderer and forwards
// every flux-volume/streamline method through a thin wrapper. See
// viewport/REFACTOR_MAP.md.
import { ViewportFluxRenderer } from './viewport/flux-renderer.js';
// Particle Points mesh + trails + velocity-vectors + per-particle force arrows
// extracted as Phase 3d. Viewport composes a ViewportParticleRenderer and
// forwards every particle-mesh method through a thin wrapper. Atom/bond/
// orbital rendering is owned by MolecularRenderer (see import above) and
// remains delegated separately. See viewport/REFACTOR_MAP.md §3d.
import { ViewportParticleRenderer } from './viewport/particle-renderer.js';
// Field overlays (E/B/Poynting/divergence/force volumes/dark matter/damping/
// genesis/confinement/dual flux/chirality/light/horizon + quantum overlays)
// extracted as Phase 3c — the largest viewport sub-renderer (66 methods, 27+
// meshes). Mesh-factory helpers (buildStreamlineMesh, buildArrowFieldMesh,
// writeArrowFieldIntoMesh, writeStreamlinesIntoMesh) live HERE as the
// canonical home; FluxRenderer + ParticleRenderer's constructor callbacks
// route through bound methods on FieldRenderer. See viewport/REFACTOR_MAP.md §3c.
import { ViewportFieldRenderer } from './viewport/field-renderer.js';

// Pre-allocated buffer-size constants (MAX_PARTICLES / MAX_FIELD_GRID)
// were centralized into viewport/constants.js (D-6). They were unused in
// this orchestrator (every buffer allocation lives in the Phase-3
// sub-renderers), so they are not re-imported here.

export class Viewport {
    constructor(container) {
        this.container = container;

        // Scene
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(0x0f1729);

        // Camera
        this.camera = new THREE.PerspectiveCamera(45, 1, 0.001, 2000);
        this.camera.position.set(60, 45, 60);

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
        this.controls.minDistance = 0.01;
        this.controls.maxDistance = 500;

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
            visualSettings: this.visualSettings,
            writeArrowFieldIntoMesh: (m, f, c, k, b, t) => this._fieldRenderer.writeArrowFieldIntoMesh(m, f, c, k, b, t),
        });

        // Axis helper + boundary wireframe + post-processing pipeline
        // are all owned by ViewportSceneCore (Phase 3a) — see ctor above.

        // Handle resize
        this._onResize();
        this._resizeObserver = new ResizeObserver(() => this._onResize());
        this._resizeObserver.observe(container);
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
    //   'iso'   — default isometric (matches boot / resize position)
    //   'moore' — zoomed-in iso that frames a 3×3×3 Moore neighbourhood
    //             around the lattice centre (useful for seed scenarios)
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

    setWireframeBrightness(val) { this._sceneCore?.setWireframeBrightness(val); }

    toggleAxes(on) { this._sceneCore?.toggleAxes(on); }

    setVoxelHighlight(x, y, z, active) { this._sceneCore?.setVoxelHighlight(x, y, z, active); }

    setSymmetryHighlights(x, y, z, u1, su2, su3) {
        this._sceneCore?.setSymmetryHighlights(x, y, z, u1, su2, su3);
    }

    toggleGrid(on) { this._sceneCore?.toggleGrid(on); }

    // ── Velocity Vectors / Trails ───────────────────────────────────────
    // Phase 3d: extracted to viewport/particle-renderer.js. These thin
    // delegators preserve the public API for app.js and panel code.
    updateVelocityVectors(positions, velocities, count) {
        this._particleRenderer.updateVelocityVectors(positions, velocities, count);
    }
    toggleVelocityVectors(on) { this._particleRenderer.toggleVelocityVectors(on); }
    updateTrails(trailHistory, typeMap) { this._particleRenderer.updateTrails(trailHistory, typeMap); }
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
    updateFieldVectors(gridPositions, forces, count, maxForce, arrowScale = 8.0) {
        this._fieldRenderer.updateFieldVectors(gridPositions, forces, count, maxForce, arrowScale);
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
    updateParticleForces(positions, forces, count, maxForce) {
        this._particleRenderer.updateParticleForces(positions, forces, count, maxForce);
    }
    toggleParticleForces(on) { this._particleRenderer.toggleParticleForces(on); }

    // ── Flux Volume Rendering (Scale 0 -- substrate mode) ──────────────
    // Phase 3b extracted into ViewportFluxRenderer (./viewport/flux-renderer.js).
    // This class keeps thin delegators for backward compatibility.
    _buildFluxVolume(latticeSize) { this._fluxRenderer._buildFluxVolume(latticeSize); }

    updateFluxVolume(volumeData, latticeSize) {
        this._fluxRenderer.updateFluxVolume(volumeData, latticeSize);
    }

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

    toggleFluxVolume(on) { this._fluxRenderer.toggleFluxVolume(on); }

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
    updateEFieldLines(streamlines) { this._fieldRenderer.updateEFieldLines(streamlines); }
    toggleEFieldLines(on) { this._fieldRenderer.toggleEFieldLines(on); }

    // -- B-Field Lines (Green) --
    _buildBFieldLines() { this._fieldRenderer._buildBFieldLines(); }
    updateBFieldLines(streamlines) { this._fieldRenderer.updateBFieldLines(streamlines); }
    toggleBFieldLines(on) { this._fieldRenderer.toggleBFieldLines(on); }

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
    updateFluxStreamlines(streamlines, maxFluxMag) {
        this._fluxRenderer.updateFluxStreamlines(streamlines, maxFluxMag);
    }
    toggleFluxStreamlines(on) { this._fluxRenderer.toggleFluxStreamlines(on); }

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

    // -- Genesis Threshold Isosurface --
    _buildGenesisIsosurface() { this._fieldRenderer._buildGenesisIsosurface(); }
    updateGenesisIsosurface(fluxMag, latticeSize, kGenesis) { this._fieldRenderer.updateGenesisIsosurface(fluxMag, latticeSize, kGenesis); }
    toggleGenesisIsosurface(on) { this._fieldRenderer.toggleGenesisIsosurface(on); }

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

    // -- Light Field --
    _buildLightField() { this._fieldRenderer._buildLightField(); }
    updateLightField(poyntingData) { this._fieldRenderer.updateLightField(poyntingData); }
    toggleLightField(on) { this._fieldRenderer.toggleLightField(on); }

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

    // -- Moore-neighbourhood decomposition (static structural wireframe) --
    toggleMooreDecomp(on) { this._fieldRenderer.toggleMooreDecomp(on); }

    // -- Topological Sheet (deformable rubber-sheet) overlays --
    toggleGravPotentialField(on) { this._topoRenderer?.toggleGravPotential(on); }
    updateGravPotentialField(data) { this._topoRenderer?.updateGravPotential(data); }
    toggleEmEnergyField(on) { this._topoRenderer?.toggle('emEnergy', on); }
    updateEmEnergyField(data) { this._topoRenderer?.update('emEnergy', data); }
    toggleHelicityField(on) { this._topoRenderer?.toggle('helicity', on); }
    updateHelicityField(data) { this._topoRenderer?.update('helicity', data); }
    toggleKretschmannField(on) { this._topoRenderer?.toggle('kretschmann', on); }
    updateKretschmannField(data) { this._topoRenderer?.update('kretschmann', data); }
    toggleEPressureField(on) { this._topoRenderer?.toggle('ePressure', on); }
    updateEPressureField(data) { this._topoRenderer?.update('ePressure', data); }
    toggleBPressureField(on) { this._topoRenderer?.toggle('bPressure', on); }
    updateBPressureField(data) { this._topoRenderer?.update('bPressure', data); }
    toggleKineticEnergyField(on) { this._topoRenderer?.toggle('kineticEnergy', on); }
    updateKineticEnergyField(data) { this._topoRenderer?.update('kineticEnergy', data); }
    toggleFisherField(on) { this._topoRenderer?.toggle('fisher', on); }
    updateFisherField(data) { this._topoRenderer?.update('fisher', data); }
    toggleCoherenceField(on) { this._topoRenderer?.toggle('coherence', on); }
    updateCoherenceField(data) { this._topoRenderer?.update('coherence', data); }
    toggleChargeDensityField(on) { this._topoRenderer?.toggle('chargeDensity', on); }
    updateChargeDensityField(data) { this._topoRenderer?.update('chargeDensity', data); }
    toggleVorticityField(on) { this._topoRenderer?.toggle('vorticity', on); }
    updateVorticityField(data) { this._topoRenderer?.update('vorticity', data); }

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
            if (this._lightField) this._lightField.visible = false;
            if (this._darkMatterHalo) this._darkMatterHalo.visible = false;
            if (this._dampingZones) this._dampingZones.visible = false;
            if (this._genesisIsosurface) this._genesisIsosurface.visible = false;
            if (this._confinementStrings) this._confinementStrings.visible = false;
            // Boundary box
            if (this.wireframe) this.wireframe.visible = false;
        };

        // ── Cosmic mode: hide all non-cosmic visuals ──
        if (mode === 'cosmic') {
            hideAllOverlays();
            return;
        }

        // ── Meta mode: hide all physics visuals, keep scene clean ──
        if (mode === 'meta') {
            this._boundaryMode = 'origin';
            hideAllOverlays();
            // Camera: close-up at origin, allow very close zoom
            this.controls.minDistance = 0.1;
            this.controls.target.set(0, 0, 0);
            this.camera.position.set(5, 3.5, 5);
            this.camera.near = 0.005;
            this.camera.updateProjectionMatrix();
            this.controls.update();
            return;
        }

        // ── Leaving meta mode — restore camera limits ──
        this.controls.minDistance = 0.01;
        this.camera.near = 0.001;
        this.camera.updateProjectionMatrix();

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

            // Recenter at lattice center
            const center = this.latticeSize / 2;
            const dist = this.latticeSize * 1.6;
            this.controls.target.set(center, center, center);
            this.camera.position.set(center + dist * 0.25, center + dist * 0.15, center + dist);
            this.controls.update();
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
    setPositiveSize(val) {
        this.visualSettings.positiveSize = val;
        this._particleRenderer?.updateParticleSizes();
        this.render();
    }
    setNegativeSize(val) {
        this.visualSettings.negativeSize = val;
        this._particleRenderer?.updateParticleSizes();
        this.render();
    }
    setParticleOpacity(val) {
        this.visualSettings.particleOpacity = val;
        this._particleRenderer.setOpacity(val);
    }
    setParticleGlow(val) { this._particleRenderer.setGlow(val); }
    setParticleShape(idx) { this._particleRenderer.setPointShape(idx); }
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

    render() {
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
    }

    _onResize() {
        const rect = this.container.getBoundingClientRect();
        const w = rect.width;
        const h = rect.height;
        if (w === 0 || h === 0) return;
        this.camera.aspect = w / h;
        this.camera.updateProjectionMatrix();
        this.renderer.setSize(w, h);
        // SceneCore resizes its composer (no-op when post-processing disabled).
        this._sceneCore?.onResize(w, h);
    }

    dispose() {
        this._resizeObserver.disconnect();

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
        // _dualFluxVolume, _chiralityField, _lightField, _quantumField,
        // _phaseNeedles, _horizonField, plus instance-owned soft-disc texture.
        this._fieldRenderer?.dispose();

        // Raycasting (orchestrator-owned bounding volume — used by inspector)
        disposeMesh(this._voidBox);

        // Phase 3a: SceneCore disposes wireframe, axes, peAxes, peGrid,
        // _voxelHighlight, _symHighlights, post-processing composer/bloom.
        this._sceneCore?.dispose();

        // Renderer last (after every sub-renderer has freed its GPU resources).
        this.renderer.dispose();
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
    get _fluxThreshold() { return this._fluxRenderer?._fluxThreshold ?? 0.005; }
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
    get _lightField() { return this._fieldRenderer?._lightField ?? null; }
    set _lightField(v) { if (this._fieldRenderer) this._fieldRenderer._lightField = v; }
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
    set _engineMode(v) { if (this._sceneCore) this._sceneCore._engineMode = v; }
    get _boundaryShape() { return this._sceneCore?._boundaryShape ?? 'cube'; }
    set _boundaryShape(v) { if (this._sceneCore) this._sceneCore._boundaryShape = v; }
    get _boundaryMode() { return this._sceneCore?._boundaryMode ?? 'lattice'; }
    set _boundaryMode(v) { if (this._sceneCore) this._sceneCore._boundaryMode = v; }
    get _voxelHighlight() { return this._sceneCore?._voxelHighlight ?? null; }
    set _voxelHighlight(v) { if (this._sceneCore) this._sceneCore._voxelHighlight = v; }
    get _symHighlights() { return this._sceneCore?._symHighlights ?? null; }
    set _symHighlights(v) { if (this._sceneCore) this._sceneCore._symHighlights = v; }
    get _composer() { return this._sceneCore?._composer ?? null; }
    set _composer(v) { if (this._sceneCore) this._sceneCore._composer = v; }
    get _bloomPass() { return this._sceneCore?._bloomPass ?? null; }
    set _bloomPass(v) { if (this._sceneCore) this._sceneCore._bloomPass = v; }
    get _usePostProcessing() { return this._sceneCore?._usePostProcessing ?? false; }
    set _usePostProcessing(v) { if (this._sceneCore) this._sceneCore._usePostProcessing = v; }
}
