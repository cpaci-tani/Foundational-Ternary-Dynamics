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
import { EffectComposer } from 'three/addons/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/addons/postprocessing/RenderPass.js';
import { UnrealBloomPass } from 'three/addons/postprocessing/UnrealBloomPass.js';
import { getById } from './particle-catalog.js';
import { potentialToColor, magnitudeToColor, fluxToColor, fluxToColorInto } from './fields.js';
import { K_B } from './constants.js';
import {
    rampViridis,
    rampCyclicHSL,
    rampDivergingRdBu,
    rampGrayscale,
    rampGravWell,
    rampEmEnergy,
    rampCharge,
    rampVorticity,
    rampHelicity,
    rampKretschmann,
    rampEPressure,
    rampBPressure,
    rampKineticEnergy,
    rampFisher,
    rampCoherence,
    FORCE_PALETTES,
    lerpPalette,
    RAMP_BY_NAME,
} from './viewport/color-ramps.js';
// Molecular rendering (bonds, orbital shells/lobes, AE force arrows,
// element labels, nucleus glow) extracted to its own module as Wave 2
// ticket 4 of the large-file refactor. Viewport composes a
// MolecularRenderer and delegates every public method through a thin
// wrapper. See docs/SPEC_REFACTOR_LARGE_FILES.md §5.
import { MolecularRenderer } from './viewport/molecular-renderer.js';
import { SpinArrowManager } from './viewport/spin-arrow-manager.js';
// Boundary wireframe builders + containment predicate — extracted to keep
// viewport.js under the refactor-plan LOC target (refactoring-analyst RF-4).
// Pure geometry; no state beyond the returned Three.js Group.
import { buildBoundary, insideBoundary } from './viewport/boundary-geometry.js';
// Rubber-sheet visualizations (gravitational potential + 10 topology fields).
// Extracted per refactoring-analyst RF-1. Viewport holds the instance as
// this._topoRenderer and forwards via thin delegators.
import { TopologySheetRenderer } from './viewport/topology-sheet-renderer.js';
// Flux volume + flux streamlines extracted as Phase 3b of the viewport
// decomposition. Viewport composes a ViewportFluxRenderer and forwards
// every flux-volume/streamline method through a thin wrapper. See
// viewport/REFACTOR_MAP.md.
import { ViewportFluxRenderer } from './viewport/flux-renderer.js';

// Pre-allocated buffer sizes. Particle buffer is fixed at init to avoid
// dynamic GPU reallocation; draw range controls visible count each frame.
const MAX_PARTICLES = 100000;
const MAX_FIELD_GRID = 16384;  // up to 128x128 grid points (must cover lattice^2)

// Custom particle shaders
const PARTICLE_VERT = `
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

// Flux-volume variant: sqrt depth scaling instead of linear 1/z.
// For N=8 the camera is only ~9-19 units away, so linear 1/z gives a
// 2× size ratio between near and far faces, making the sphere look
// wildly asymmetric.  sqrt(60/z) compresses that to ~1.4× so both
// hemispheres stay visually balanced regardless of lattice size.
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

const PARTICLE_FRAG = `
    uniform int shapeType;
    uniform float uOpacity;
    varying vec3 vColor;
    varying float vSize;

    void main() {
        vec2 c = gl_PointCoord - vec2(0.5);
        float dist;

        if (shapeType == 1) {
            // Square
            dist = max(abs(c.x), abs(c.y));
            if (dist > 0.48) discard;
        } else if (shapeType == 2) {
            // Diamond
            dist = abs(c.x) + abs(c.y);
            if (dist > 0.5) discard;
        } else if (shapeType == 3) {
            // Star (5-pointed)
            float angle = atan(c.y, c.x);
            float r = length(c);
            float star = cos(5.0 * angle) * 0.15 + 0.35;
            if (r > star) discard;
            dist = r / star * 0.5;
        } else if (shapeType == 4) {
            // Triangle
            float x = c.x, y = c.y + 0.15;
            if (y > 0.35 || y < -0.35 + 0.7 * abs(x) / 0.4) discard;
            dist = length(c);
        } else if (shapeType == 5) {
            // Hexagon
            vec2 a = abs(c);
            dist = max(a.x * 0.866 + a.y * 0.5, a.y);
            if (dist > 0.45) discard;
            dist /= 0.45;
        } else if (shapeType == 6) {
            // Ring
            float r = length(c);
            if (r > 0.5 || r < 0.3) discard;
            dist = abs(r - 0.4) / 0.1;
        } else if (shapeType == 7) {
            // Cross
            float ax = abs(c.x), ay = abs(c.y);
            if (ax > 0.15 && ay > 0.15) discard;
            dist = max(ax, ay);
        } else {
            // Circle (default, shapeType == 0)
            dist = length(c);
            if (dist > 0.5) discard;
        }

        float alpha = 1.0 - smoothstep(0.15, 0.5, dist);
        float glow = exp(-dist * dist * 4.0) * 0.15;
        gl_FragColor = vec4(vColor + glow, alpha * alpha * uOpacity);
    }
`;

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

        // Particle system
        this._initParticles();

        // Visual settings for particle size and opacity
        this.visualSettings = {
            globalScale: 1.0,
            manifestedSize: 12.0,
            voidSize: 4.0,
            opacity: 0.95,
        };

        // Boundary / wireframe
        this.wireframe = null;
        this.showWireframe = true;
        this._wireframeBrightness = 0.18;
        // showFlux is owned by FluxRenderer (Phase 3b); see backward-compat
        // getter/setter near end of class so external code can still read it.
        this.showHeatmap = false;
        this._showAxes = true;   // user preference for axes visibility
        this._showGrid = true;   // user preference for grid visibility
        this._engineMode = 'lattice';
        this._boundaryShape = 'cube';
        this._boundaryMode = 'lattice'; // 'lattice' (Scale 0) or 'origin' (Scale 1+)

        // Lattice reference
        this.latticeSize = 32;
        this._halfN = 16;
        this._buildBoundary(this._boundaryShape, 'lattice');

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

        // Flux volume + flux streamlines — extracted Phase 3b. Viewport owns
        // the orchestrator; FluxRenderer owns its meshes + scenario-scale
        // helpers. The two streamline-mesh helpers (`_buildStreamlineMesh`
        // / `_writeStreamlinesIntoMesh`) live on Viewport because Phase 3c
        // FieldRenderer also uses them — we pass them in as callbacks.
        this._fluxRenderer = new ViewportFluxRenderer({
            scene: this.scene,
            latticeSize: this.latticeSize,
            halfN: this._halfN,
            boundaryShape: this._boundaryShape,
            insideBoundary: (nx, ny, nz) => this._insideBoundary(nx, ny, nz),
            applyScenarioScale: () => this._applyScenarioScale(),
            buildStreamlineMesh: (m, o) => this._buildStreamlineMesh(m, o),
            writeStreamlinesIntoMesh: (m, s, c) => this._writeStreamlinesIntoMesh(m, s, c),
        });

        // Axis helper (subtle)
        this._buildAxes();

        // Post-processing (lazy init for consciousness mode)
        this._composer = null;
        this._bloomPass = null;
        this._usePostProcessing = false;

        // Handle resize
        this._onResize();
        this._resizeObserver = new ResizeObserver(() => this._onResize());
        this._resizeObserver.observe(container);
    }

    _initParticles() {
        const geometry = new THREE.BufferGeometry();
        const positions = new Float32Array(MAX_PARTICLES * 3);
        const colors = new Float32Array(MAX_PARTICLES * 3);
        const sizes = new Float32Array(MAX_PARTICLES);

        const posAttr = new THREE.BufferAttribute(positions, 3);
        const colAttr = new THREE.BufferAttribute(colors, 3);
        const sizeAttr = new THREE.BufferAttribute(sizes, 1);
        posAttr.setUsage(THREE.DynamicDrawUsage);
        colAttr.setUsage(THREE.DynamicDrawUsage);
        sizeAttr.setUsage(THREE.DynamicDrawUsage);
        geometry.setAttribute('position', posAttr);
        geometry.setAttribute('particleColor', colAttr);
        geometry.setAttribute('size', sizeAttr);
        geometry.setDrawRange(0, 0);

        const material = new THREE.ShaderMaterial({
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.9 } },
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            depthTest: true,
            blending: THREE.NormalBlending,
        });

        this.particles = new THREE.Points(geometry, material);
        this.particles.frustumCulled = false; // skip bounding sphere recompute for dynamic geometry
        this.scene.add(this.particles);
    }

    // ── Boundary system ────────────────────────────────────────────────

    _disposeBoundary() {
        if (this.wireframe) {
            this.scene.remove(this.wireframe);
            this.wireframe.traverse(child => {
                if (child.geometry) child.geometry.dispose();
                if (child.material) child.material.dispose();
            });
            this.wireframe = null;
        }
    }

    _buildBoundary(shape, mode) {
        this._disposeBoundary();
        this._boundaryShape = shape;
        this._boundaryMode = mode;

        if (shape === 'none') return;

        const mat = new THREE.LineBasicMaterial({
            color: 0x1e2d44, transparent: true, opacity: this._wireframeBrightness,
            depthWrite: false,
        });

        const group = buildBoundary(shape, mode, { latticeSize: this.latticeSize }, mat);

        // Scale and position based on mode
        // Non-cube shapes are inscribed within the lattice cube (radius = s/2)
        // so the flux volume clips to the shape boundary
        if (mode === 'lattice') {
            const s = this.latticeSize;
            if (shape === 'cube') {
                // Cube is already built at lattice coords — no transform needed
            } else {
                group.scale.setScalar(s / 2);
                group.position.set(s / 2, s / 2, s / 2);
            }
        } else {
            // origin mode (PE/AE/molecules)
            const radius = 35;
            if (shape === 'cube') {
                group.scale.setScalar(radius / (this.latticeSize / 2));
                group.position.set(0, 0, 0);
            } else {
                group.scale.setScalar(radius);
                group.position.set(0, 0, 0);
            }
        }

        this.wireframe = group;
        this.wireframe.visible = this.showWireframe;
        this.scene.add(this.wireframe);
    }






    setBoundaryShape(shape) {
        this._buildBoundary(shape, this._boundaryMode);
        this._fluxRenderer?.setBoundaryShape(shape);
    }

    /**
     * Test whether a point (in normalized coords -1..1 from center) is inside
     * the current boundary shape. Used to clip flux volume rendering.
     */
    /**
     * Test whether a point (normalized -1..1 from center) is inside the
     * current boundary. Delegated to viewport/boundary-geometry.js.
     */
    _insideBoundary(nx, ny, nz) { return insideBoundary(this._boundaryShape, nx, ny, nz); }

    _buildAxes() {
        // Axis indicator at origin — length scales with lattice size
        const axisLen = Math.max(3, this.latticeSize * 0.1);
        const axisGeo = new THREE.BufferGeometry();
        axisGeo.setAttribute('position', new THREE.Float32BufferAttribute([
            0, 0, 0, axisLen, 0, 0,  // X
            0, 0, 0, 0, axisLen, 0,  // Y
            0, 0, 0, 0, 0, axisLen,  // Z
        ], 3));
        axisGeo.setAttribute('color', new THREE.Float32BufferAttribute([
            0.9, 0.3, 0.3, 0.9, 0.3, 0.3,  // X = red
            0.3, 0.9, 0.3, 0.3, 0.9, 0.3,  // Y = green
            0.3, 0.3, 0.9, 0.3, 0.3, 0.9,  // Z = blue
        ], 3));
        const axisMat = new THREE.LineBasicMaterial({ vertexColors: true, transparent: true, opacity: 0.5 });
        this.axes = new THREE.LineSegments(axisGeo, axisMat);
        this.scene.add(this.axes);
    }

    setLatticeSize(size) {
        this.latticeSize = size;
        this._latticeSize = size;  // mirrored so quantum overlays can read it too
        this._halfN = size / 2;
        this._buildBoundary(this._boundaryShape, this._boundaryMode);
        // Tracked particles may have stale ids after a scenario / lattice resize;
        // dispose all spin arrows so the next track() request gets a clean Group.
        if (this.spinArrowManager) this.spinArrowManager.dispose();
        // TopologySheetRenderer re-queries latticeSize via its getter on next
        // update; grav-surface rebuild now happens inside that module.
        
        // Rebuild void box for raycasting
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

        // Rebuild flux volume + clear flux-streamlines draw range — owned by
        // FluxRenderer (Phase 3b extraction).
        this._fluxRenderer?.onLatticeSizeChanged(size, this._halfN);
        // Rebuild field heatmap for new lattice capacity
        if (this._fieldHeatmap) {
            this.scene.remove(this._fieldHeatmap);
            this._fieldHeatmap.geometry.dispose();
            this._fieldHeatmap.material.dispose();
            this._fieldHeatmap = null;
        }

        // Clear draw ranges on all field overlays so stale data from old L
        // doesn't persist until the next field update frame
        const fieldOverlays = [
            this._eFieldLines, this._bFieldLines, this._poyntingVectors,
            this._divField, this._fluxStreamlines, this._forceVolume,
            this._gravityField, this._strongForce, this._weakField,
            this._darkMatterHalo, this._dampingZones,
            this._genesisIsosurface, this._confinementStrings,
            this._dualFluxVolume, this._chiralityField, this._lightField
        ];
        for (const obj of fieldOverlays) {
            if (obj && obj.geometry) obj.geometry.setDrawRange(0, 0);
        }

        // Rebuild axes so length scales with lattice
        if (this.axes) {
            this.scene.remove(this.axes);
            this.axes.geometry.dispose();
            this.axes.material.dispose();
        }
        this._buildAxes();

        // Recenter camera for lattice mode
        if (this._boundaryMode === 'lattice') {
            const center = size / 2;
            const dist = size * 1.6;
            this.controls.target.set(center, center, center);
            this.camera.position.set(center + dist * 0.25, center + dist * 0.15, center + dist);
            this.controls.update();
        }
        if (this._applyScenarioScale) this._applyScenarioScale();
    }

    toggleWireframe(on) {
        this.showWireframe = on;
        if (this.wireframe) this.wireframe.visible = on;
    }

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
    setCameraPreset(which) {
        if (this._boundaryMode !== 'lattice') return false;
        const N = this.latticeSize || 32;
        const c = N / 2;
        let dist, pos;
        switch (which) {
            case 'front': dist = N * 1.6; pos = [c, c, c + dist]; break;
            case 'side':  dist = N * 1.6; pos = [c + dist, c, c]; break;
            case 'top':   dist = N * 1.6; pos = [c, c + dist, c + 0.001]; break;  // tiny Z offset so OrbitControls can roll freely
            case 'iso':   dist = N * 1.6; pos = [c + dist * 0.25, c + dist * 0.15, c + dist]; break;
            case 'moore': dist = Math.max(6, N * 0.35); pos = [c + dist * 0.6, c + dist * 0.4, c + dist]; break;
            default: return false;
        }
        this.controls.target.set(c, c, c);
        this.camera.position.set(pos[0], pos[1], pos[2]);
        this.controls.update();
        return true;
    }

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

    setWireframeBrightness(val) {
        this._wireframeBrightness = val;
        if (!this.wireframe) return;
        this.wireframe.traverse(child => {
            if (child.material && 'opacity' in child.material) {
                child.material.opacity = val;
            }
        });
    }

    toggleAxes(on) {
        this._showAxes = on;
        const mode = this._engineMode || 'lattice';
        if (mode === 'consciousness' || mode === 'cosmic' || mode === 'meta') return;
        if (mode === 'lattice') {
            if (this.axes) this.axes.visible = on;
        } else {
            if (this.peAxes) this.peAxes.visible = on;
        }
    }

    setVoxelHighlight(x, y, z, active) {
        if (!this._voxelHighlight) {
            const geo = new THREE.BoxGeometry(1.2, 1.2, 1.2);
            const edges = new THREE.EdgesGeometry(geo);
            geo.dispose();   // EdgesGeometry copied what it needs; source is orphan
            const mat = new THREE.LineBasicMaterial({ color: 0xffff00, linewidth: 2 });
            this._voxelHighlight = new THREE.LineSegments(edges, mat);
            this.scene.add(this._voxelHighlight);
        }
        if (active) {
            // Voxel k's rendered centre is at world (k+0.5). Previously this
            // snapped the highlight box to integer world coords, so the box
            // sat on the voxel's lower-left corner instead of its centre —
            // half-voxel shift visible when overlaid on particles/flux.
            this._voxelHighlight.position.set(x + 0.5, y + 0.5, z + 0.5);
            this._voxelHighlight.visible = true;
        } else {
            this._voxelHighlight.visible = false;
        }
    }

    setSymmetryHighlights(x, y, z, u1, su2, su3) {
        if (!this._symHighlights) {
            const geo = new THREE.BoxGeometry(1.0, 1.0, 1.0);
            const edges = new THREE.EdgesGeometry(geo);
            geo.dispose();   // EdgesGeometry copied what it needs; source is orphan
            const mat = new THREE.LineBasicMaterial({ color: 0x4ade80, linewidth: 1, transparent: true, opacity: 0.6 });
            this._symHighlights = new THREE.InstancedMesh(edges, mat, 26);
            this._symHighlights.instanceMatrix.setUsage(THREE.DynamicDrawUsage);
            this.scene.add(this._symHighlights);
        }
        
        let count = 0;
        const dummy = new THREE.Object3D();
        
        if (u1 || su2 || su3) {
            for (let dx = -1; dx <= 1; dx++) {
                for (let dy = -1; dy <= 1; dy++) {
                    for (let dz = -1; dz <= 1; dz++) {
                        if (dx === 0 && dy === 0 && dz === 0) continue;
                        
                        const norm = Math.abs(dx) + Math.abs(dy) + Math.abs(dz);
                        let include = false;
                        if (u1 && norm === 1) include = true;   // Face
                        if (su2 && norm === 2) include = true;  // Edge
                        if (su3 && norm === 3) include = true;  // Corner
                        
                        if (include) {
                            // Same voxel-centre convention as setVoxelHighlight:
                            // neighbour voxel (x+dx, y+dy, z+dz) is rendered at
                            // world centre (x+dx+0.5, y+dy+0.5, z+dz+0.5).
                            dummy.position.set(x + dx + 0.5, y + dy + 0.5, z + dz + 0.5);
                            dummy.updateMatrix();
                            this._symHighlights.setMatrixAt(count++, dummy.matrix);
                        }
                    }
                }
            }
        }
        
        this._symHighlights.count = count;
        this._symHighlights.instanceMatrix.needsUpdate = true;
        this._symHighlights.visible = count > 0;
    }

    toggleGrid(on) {
        this._showGrid = on;
        const mode = this._engineMode || 'lattice';
        if (mode === 'consciousness' || mode === 'cosmic' || mode === 'meta') return;
        if (mode === 'lattice') {
            // Scale 0: the wireframe cube serves as the grid reference
            if (this.wireframe) this.wireframe.visible = on;
            this.showWireframe = on;
        } else {
            // Scale 1+: separate XZ plane grid
            if (this.peGrid) this.peGrid.visible = on;
        }
    }

    // ── Velocity Vectors (PE mode overlay) ──────────────────────────────
    _buildVelocityVectors() {
        const MAX_VEC = 200; // max particles for velocity vectors
        const vertices = new Float32Array(MAX_VEC * 2 * 3);
        const colors = new Float32Array(MAX_VEC * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.8,
        });
        this.velocityVectors = new THREE.LineSegments(geo, mat);
        this.velocityVectors.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.velocityVectors.visible = false;
        this.scene.add(this.velocityVectors);
    }

    updateVelocityVectors(positions, velocities, count) {
        if (!this.velocityVectors) this._buildVelocityVectors();
        if (!velocities) return;

        const posAttr = this.velocityVectors.geometry.getAttribute('position');
        const colAttr = this.velocityVectors.geometry.getAttribute('color');
        const maxLines = posAttr.array.length / 6;
        const n = Math.min(count, maxLines);
        const scale = 50; // scale factor so velocity vectors are visible

        for (let i = 0; i < n; i++) {
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            const vx = velocities[i * 3], vy = velocities[i * 3 + 1], vz = velocities[i * 3 + 2];

            // Start point (particle center)
            posAttr.array[i * 6] = px;
            posAttr.array[i * 6 + 1] = py;
            posAttr.array[i * 6 + 2] = pz;
            // End point (position + velocity * scale)
            posAttr.array[i * 6 + 3] = px + vx * scale;
            posAttr.array[i * 6 + 4] = py + vy * scale;
            posAttr.array[i * 6 + 5] = pz + vz * scale;

            // Color: yellow at tail → orange at tip, intensity by speed
            const speed = Math.sqrt(vx * vx + vy * vy + vz * vz);
            const t = Math.min(speed * 20, 1);
            // Start: bright yellow
            colAttr.array[i * 6] = 1.0;
            colAttr.array[i * 6 + 1] = 0.9 - t * 0.3;
            colAttr.array[i * 6 + 2] = 0.2;
            // End: orange/red
            colAttr.array[i * 6 + 3] = 1.0;
            colAttr.array[i * 6 + 4] = 0.4 - t * 0.3;
            colAttr.array[i * 6 + 5] = 0.1;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this.velocityVectors.geometry.setDrawRange(0, n * 2);
    }

    toggleVelocityVectors(on) {
        if (!this.velocityVectors) this._buildVelocityVectors();
        this.velocityVectors.visible = on;
        if (!on) this.velocityVectors.geometry.setDrawRange(0, 0);
    }

    // ── Orbit Trails (PE mode overlay) ───────────────────────────────
    _buildTrails() {
        // Pre-allocate for up to 50 particles × 200 trail segments
        const MAX_SEGMENTS = 50 * 200;
        const vertices = new Float32Array(MAX_SEGMENTS * 2 * 3);
        const colors = new Float32Array(MAX_SEGMENTS * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.5,
        });
        this.trails = new THREE.LineSegments(geo, mat);
        this.trails.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.trails.visible = false;
        this.scene.add(this.trails);
    }

    updateTrails(trailHistory, typeMap) {
        if (!this.trails) this._buildTrails();
        if (!trailHistory || trailHistory.size === 0) {
            this.trails.geometry.setDrawRange(0, 0);
            return;
        }

        const posAttr = this.trails.geometry.getAttribute('position');
        const colAttr = this.trails.geometry.getAttribute('color');
        const maxSegments = posAttr.array.length / 6;
        let seg = 0;

        for (const [particleId, trail] of trailHistory) {
            if (trail.length < 2) continue;

            // Determine trail color from catalog
            const catId = typeMap ? typeMap.get(particleId) : null;
            const cat = catId ? getById(catId) : null;
            const cr = cat ? cat.display_color[0] : 0.5;
            const cg = cat ? cat.display_color[1] : 0.5;
            const cb = cat ? cat.display_color[2] : 0.5;

            const len = trail.length;
            const maxLen = 200; // TRAIL_MAX_LENGTH
            // Read from circular buffer in order (oldest → newest)
            const start = trail.length < maxLen ? 0 : trail.head;

            for (let j = 0; j < len - 1 && seg < maxSegments; j++) {
                const idx0 = (start + j) % maxLen;
                const idx1 = (start + j + 1) % maxLen;

                // Segment start
                posAttr.array[seg * 6] = trail.positions[idx0 * 3];
                posAttr.array[seg * 6 + 1] = trail.positions[idx0 * 3 + 1];
                posAttr.array[seg * 6 + 2] = trail.positions[idx0 * 3 + 2];
                // Segment end
                posAttr.array[seg * 6 + 3] = trail.positions[idx1 * 3];
                posAttr.array[seg * 6 + 4] = trail.positions[idx1 * 3 + 1];
                posAttr.array[seg * 6 + 5] = trail.positions[idx1 * 3 + 2];

                // Fade: old segments dim, new segments bright
                const fade = (j + 1) / len;
                colAttr.array[seg * 6] = cr * fade * 0.8;
                colAttr.array[seg * 6 + 1] = cg * fade * 0.8;
                colAttr.array[seg * 6 + 2] = cb * fade * 0.8;
                colAttr.array[seg * 6 + 3] = cr * fade;
                colAttr.array[seg * 6 + 4] = cg * fade;
                colAttr.array[seg * 6 + 5] = cb * fade;

                seg++;
            }
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this.trails.geometry.setDrawRange(0, seg * 2);
    }

    toggleTrails(on) {
        if (!this.trails) this._buildTrails();
        this.trails.visible = on;
        if (!on) this.trails.geometry.setDrawRange(0, 0);
    }

    clearTrails() {
        if (this.trails) {
            this.trails.geometry.setDrawRange(0, 0);
        }
    }

    // ── Bond Lines (Scale 2 — Atom mode) ──────────────────────────────
    // Moved to viewport/molecular-renderer.js (Wave 2 ticket 4).
    // `this.bondLines` is preserved as a getter for external callers.
    get bondLines() { return this._molRenderer?.bondLines ?? null; }
    updateBondLines(atomData) { this._molRenderer.updateBondLines(atomData); }
    toggleBondLines(on)       { this._molRenderer.toggleBondLines(on); }

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
        // Uniforms must match PARTICLE_FRAG expectations (shapeType, uOpacity)
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
        this.scene.add(this._fieldHeatmap);
    }

    updateFieldHeatmap(gridPositions, potentials, count, maxAbsPotential) {
        if (!this._fieldHeatmap) this._buildFieldHeatmap();
        const posAttr = this._fieldHeatmap.geometry.getAttribute('position');
        const colAttr = this._fieldHeatmap.geometry.getAttribute('particleColor');
        const sizeAttr = this._fieldHeatmap.geometry.getAttribute('size');
        const n = Math.min(count, MAX_FIELD_GRID);

        for (let i = 0; i < n; i++) {
            posAttr.array[i * 3] = gridPositions[i * 3];
            posAttr.array[i * 3 + 1] = gridPositions[i * 3 + 1] - 0.3;
            posAttr.array[i * 3 + 2] = gridPositions[i * 3 + 2];

            const [r, g, b] = potentialToColor(potentials[i], maxAbsPotential);
            colAttr.array[i * 3] = r;
            colAttr.array[i * 3 + 1] = g;
            colAttr.array[i * 3 + 2] = b;

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
        this._fieldVectors.frustumCulled = false; // dynamic geo — see _eFieldLines
        this._fieldVectors.visible = false;
        this.scene.add(this._fieldVectors);
    }

    updateFieldVectors(gridPositions, forces, count, maxForce, arrowScale = 8.0) {
        if (!this._fieldVectors) this._buildFieldVectors();
        const posAttr = this._fieldVectors.geometry.getAttribute('position');
        const colAttr = this._fieldVectors.geometry.getAttribute('color');
        const n = Math.min(count, MAX_FIELD_GRID);

        for (let i = 0; i < n; i++) {
            const gx = gridPositions[i * 3], gy = gridPositions[i * 3 + 1], gz = gridPositions[i * 3 + 2];
            const fx = forces[i * 3], fy = forces[i * 3 + 1], fz = forces[i * 3 + 2];
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);

            // Start (grid point)
            posAttr.array[i * 6] = gx;
            posAttr.array[i * 6 + 1] = gy;
            posAttr.array[i * 6 + 2] = gz;

            // End: normalized direction × log-compressed length
            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce + 1e-20) * 10) : 0;
            posAttr.array[i * 6 + 3] = gx + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[i * 6 + 4] = gy + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[i * 6 + 5] = gz + (mag > 1e-20 ? fz / mag * scale : 0);

            // Color: dim tail → bright tip
            const [cr, cg, cb] = magnitudeToColor(mag, maxForce);
            colAttr.array[i * 6] = cr * 0.5;
            colAttr.array[i * 6 + 1] = cg * 0.5;
            colAttr.array[i * 6 + 2] = cb * 0.5;
            colAttr.array[i * 6 + 3] = cr;
            colAttr.array[i * 6 + 4] = cg;
            colAttr.array[i * 6 + 5] = cb;
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
        this._peStreamlines.frustumCulled = false; // dynamic geo — see _eFieldLines
        this._peStreamlines.visible = false;
        this.scene.add(this._peStreamlines);
    }

    updatePEStreamlines(lines) {
        if (!this._peStreamlines) this._buildPEStreamlines();
        const posAttr = this._peStreamlines.geometry.getAttribute('position');
        const colAttr = this._peStreamlines.geometry.getAttribute('color');
        const maxVerts = posAttr.count;
        let vi = 0;

        for (const line of lines) {
            const nPts = line.length / 3;
            for (let i = 0; i < nPts - 1 && vi + 2 <= maxVerts; i++) {
                // Segment: point i → point i+1
                posAttr.array[vi * 3] = line[i * 3];
                posAttr.array[vi * 3 + 1] = line[i * 3 + 1];
                posAttr.array[vi * 3 + 2] = line[i * 3 + 2];
                posAttr.array[vi * 3 + 3] = line[(i + 1) * 3];
                posAttr.array[vi * 3 + 4] = line[(i + 1) * 3 + 1];
                posAttr.array[vi * 3 + 5] = line[(i + 1) * 3 + 2];

                // Color: fade from bright to dim along line
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

    // Gravity arrows — grey. Uses shared arrow-mesh helper (RF-2), though
    // the writer stays inline because gravity takes (gridPositions, forces,
    // count, maxForce) as separate args (caller-computed maxForce) rather
    // than the {positions, vectors, count} bag of other arrow fields.
    _buildGravityVectors() {
        this._gravityVectors = this._buildArrowFieldMesh(MAX_FIELD_GRID, 0.65);
    }

    updateGravityVectors(gridPositions, forces, count, maxForce, arrowScale = 8.0) {
        if (!this._gravityVectors) this._buildGravityVectors();
        const posAttr = this._gravityVectors.geometry.getAttribute('position');
        const colAttr = this._gravityVectors.geometry.getAttribute('color');
        const n = Math.min(count, MAX_FIELD_GRID);

        for (let i = 0; i < n; i++) {
            const gx = gridPositions[i * 3], gy = gridPositions[i * 3 + 1], gz = gridPositions[i * 3 + 2];
            const fx = forces[i * 3], fy = forces[i * 3 + 1], fz = forces[i * 3 + 2];
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);

            posAttr.array[i * 6] = gx;
            posAttr.array[i * 6 + 1] = gy;
            posAttr.array[i * 6 + 2] = gz;

            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce + 1e-20) * 10) : 0;
            posAttr.array[i * 6 + 3] = gx + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[i * 6 + 4] = gy + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[i * 6 + 5] = gz + (mag > 1e-20 ? fz / mag * scale : 0);

            // Grey color for gravity
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

    // ── Per-Particle Force Arrows ─────────────────────────────────────

    _buildParticleForces() {
        const MAX_PFORCES = 200;  // max particles
        const vertices = new Float32Array(MAX_PFORCES * 2 * 3);
        const colors = new Float32Array(MAX_PFORCES * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.85,
        });
        this._particleForces = new THREE.LineSegments(geo, mat);
        this._particleForces.frustumCulled = false; // dynamic geo — see _eFieldLines
        this._particleForces.visible = false;
        this.scene.add(this._particleForces);
    }

    updateParticleForces(positions, forces, count, maxForce) {
        if (!this._particleForces) this._buildParticleForces();
        const posAttr = this._particleForces.geometry.getAttribute('position');
        const colAttr = this._particleForces.geometry.getAttribute('color');
        const n = Math.min(count, 200);
        const arrowScale = 12.0;

        for (let i = 0; i < n; i++) {
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            const fx = forces[i * 3], fy = forces[i * 3 + 1], fz = forces[i * 3 + 2];
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);

            posAttr.array[i * 6] = px;
            posAttr.array[i * 6 + 1] = py;
            posAttr.array[i * 6 + 2] = pz;

            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce + 1e-20) * 10) : 0;
            posAttr.array[i * 6 + 3] = px + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[i * 6 + 4] = py + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[i * 6 + 5] = pz + (mag > 1e-20 ? fz / mag * scale : 0);

            // Green color for net force
            const t = mag / (maxForce + 1e-20);
            colAttr.array[i * 6] = 0.2;
            colAttr.array[i * 6 + 1] = 0.4 + 0.3 * t;
            colAttr.array[i * 6 + 2] = 0.2;
            colAttr.array[i * 6 + 3] = 0.3;
            colAttr.array[i * 6 + 4] = 0.7 + 0.3 * t;
            colAttr.array[i * 6 + 5] = 0.3;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._particleForces.geometry.setDrawRange(0, n * 2);
    }

    toggleParticleForces(on) {
        if (!this._particleForces) this._buildParticleForces();
        this._particleForces.visible = on;
        if (!on) this._particleForces.geometry.setDrawRange(0, 0);
    }

    // ── Flux Volume Rendering (Scale 0 -- substrate mode) ──────────────
    // Phase 3b extracted into ViewportFluxRenderer (./viewport/flux-renderer.js).
    // This class keeps thin delegators for backward compatibility.
    _buildFluxVolume(latticeSize) { this._fluxRenderer._buildFluxVolume(latticeSize); }

    updateFluxVolume(volumeData, latticeSize) {
        this._fluxRenderer.updateFluxVolume(volumeData, latticeSize);
    }

    /**
     * Update flux slice heatmap from a 2D slice of flux magnitudes.
     * Uses the existing field heatmap infrastructure with flux colormap.
     * @param {Float64Array} sliceData — N^2 flux magnitudes
     * @param {number} latticeSize — side length N
     * @param {number} axis — 0=X, 1=Y, 2=Z slice normal
     * @param {number} index — slice position along axis
     */
    updateFluxSlice(sliceData, latticeSize, axis, index) {
        if (!this._fieldHeatmap) this._buildFieldHeatmap();
        const posAttr = this._fieldHeatmap.geometry.getAttribute('position');
        const colAttr = this._fieldHeatmap.geometry.getAttribute('particleColor');
        const sizeAttr = this._fieldHeatmap.geometry.getAttribute('size');
        const N = latticeSize;

        // Find max for normalization
        let maxFlux = 0;
        const total = N * N;
        for (let i = 0; i < total; i++) {
            if (sliceData[i] > maxFlux) maxFlux = sliceData[i];
        }

        const halfN = N / 2;
        let count = 0;
        const maxPts = Math.min(total, MAX_FIELD_GRID);
        for (let i = 0; i < total && count < maxPts; i++) {
            const a = Math.floor(i / N);
            const b = i % N;
            let x, y, z;
            if (axis === 0) { x = index; y = a; z = b; }
            else if (axis === 1) { x = a; y = index; z = b; }
            else { x = a; y = b; z = index; }

            // Clip to boundary shape
            const nx = (x - halfN + 0.5) / halfN;
            const ny = (y - halfN + 0.5) / halfN;
            const nz = (z - halfN + 0.5) / halfN;
            if (!this._insideBoundary(nx, ny, nz)) continue;

            posAttr.array[count * 3]     = x + 0.5;
            posAttr.array[count * 3 + 1] = y + 0.5;
            posAttr.array[count * 3 + 2] = z + 0.5;

            const [r, g, b2] = fluxToColor(sliceData[i], maxFlux);
            colAttr.array[count * 3] = r;
            colAttr.array[count * 3 + 1] = g;
            colAttr.array[count * 3 + 2] = b2;

            // Point size is in WORLD units (voxels). PARTICLE_VERT applies the
            // 150/depth perspective term; combined with camera distance N·1.6,
            // a `size = 1.0` always renders ≈1 voxel on screen at any lattice
            // size. The earlier `8 × 32/N` formula did the opposite, blowing up
            // points to fill the whole viewport at small N.
            //   base 1.0 (one voxel) + up to 4× swell at peak |J|
            const t = sliceData[i] / (maxFlux + 1e-20);
            sizeAttr.array[count] = 1.0 + 4.0 * t;
            count++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        this._fieldHeatmap.geometry.setDrawRange(0, count);
    }

    toggleFluxVolume(on) { this._fluxRenderer.toggleFluxVolume(on); }

    toggleFluxSlice(on) {
        if (!this._fieldHeatmap) this._buildFieldHeatmap();
        this._fieldHeatmap.visible = on;
        this.showHeatmap = on;
        if (!on) this._fieldHeatmap.geometry.setDrawRange(0, 0);
    }

    // ── Flux Volume Controls ──────────────────────────────────────────
    // Phase 3b — delegated to ViewportFluxRenderer.

    setFluxOpacity(val) { this._fluxRenderer.setFluxOpacity(val); }
    setFluxShape(shapeIndex) { this._fluxRenderer.setFluxShape(shapeIndex); }
    setFluxPointScale(scale) { this._fluxRenderer.setFluxPointScale(scale); }
    setFluxThreshold(val) { this._fluxRenderer.setFluxThreshold(val); }
    setScenarioScale(scale) { this._fluxRenderer.setScenarioScale(scale); }
    setFluxLatticeSpacing(val) { this._fluxRenderer.setFluxLatticeSpacing(val); }

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
    // E-field, B-field, Poynting, Divergence, Flux streamlines, Forces,
    // Dual substrate, Chirality
    // ══════════════════════════════════════════════════════════════════

    // Shared streamline-mesh builder (refactoring-analyst RF-3). Preallocates
    // the LineSegments buffer sized for N=128 worst-case and returns the mesh.
    // Three copies of this boilerplate were living inline before consolidation.
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
        // Disable frustum culling — geometry is updated each frame without
        // recomputing the bounding sphere, so Three.js's stale-bounds test
        // would falsely cull when the camera zooms close to the lattice.
        mesh.frustumCulled = false;
        this.scene.add(mesh);
        return mesh;
    }

    // Shared arrow-mesh builder (refactoring-analyst RF-2). Strong-force,
    // EM-force-volume, and (pending) weak-field arrow overlays all use the
    // same LineSegments scratch-buffer pattern — preallocate `maxArrows` × 2
    // vertices, vertex-colored LineBasicMaterial, frustum-culling off for
    // dynamic geometry. The 3 copies that used to be inline differed only in
    // opacity and color palette; both are now parameterized.
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
        mesh.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.scene.add(mesh);
        return mesh;
    }

    // Shared arrow-mesh writer. Expects fieldData = { positions, vectors, count }
    // and paints magnitude-filtered arrows with log-scaled length. The color
    // is two RGB triples (base + tip) for a subtle brightness gradient. The
    // magnitude cache is keyed by field name so two overlays don't race for
    // the same scratch buffer.
    //
    // @param {THREE.LineSegments} mesh
    // @param {{positions:Float32Array, vectors:Float32Array, count:number}} fieldData
    // @param {{base: [number,number,number], tip: [number,number,number]}} colors
    // @param {string} magCacheKey — `_magCache_<name>` lookup key on `this`
    // @param {number} arrowBase — world-units base length before log-scaling
    // @param {number} thresholdFrac — fraction of maxMag below which arrows are dropped
    _writeArrowFieldIntoMesh(mesh, fieldData, colors, magCacheKey, arrowBase = 1.5, thresholdFrac = 0.03) {
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
        const [br, bg, bb] = colors.base;
        const [tr, tg, tb] = colors.tip;
        let vi = 0;
        for (let i = 0; i < count && vi < maxArrows; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            const scale = Math.log(1 + mag / maxMag) * arrowBase;
            const vx = vectors[i * 3], vy = vectors[i * 3 + 1], vz = vectors[i * 3 + 2];
            const nx = vx / mag * scale, ny = vy / mag * scale, nz = vz / mag * scale;
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

    // Shared streamline writer. Walks each polyline's segments, clips endpoints
    // against the current boundary shape, and writes interleaved (start, end)
    // vertex pairs to the mesh's position/color buffers via a per-segment
    // color callback `colorFn(i, nPts, [out])`. Returns nothing — mutates mesh.
    _writeStreamlinesIntoMesh(mesh, streamlines, colorFn) {
        const posAttr = mesh.geometry.getAttribute('position');
        const colAttr = mesh.geometry.getAttribute('color');
        const maxVerts = posAttr.array.length / 3;
        const halfN = this._halfN;
        const rgb = [0, 0, 0];
        let vi = 0;
        for (const line of streamlines) {
            const nPts = line.length / 3;
            for (let i = 0; i < nPts - 1 && vi + 2 <= maxVerts; i++) {
                const sx = line[i * 3], sy = line[i * 3 + 1], sz = line[i * 3 + 2];
                if (!this._insideBoundary((sx - halfN) / halfN, (sy - halfN) / halfN, (sz - halfN) / halfN)) continue;
                colorFn(i, nPts, rgb);
                posAttr.array[vi * 3]     = sx;
                posAttr.array[vi * 3 + 1] = sy;
                posAttr.array[vi * 3 + 2] = sz;
                colAttr.array[vi * 3]     = rgb[0];
                colAttr.array[vi * 3 + 1] = rgb[1];
                colAttr.array[vi * 3 + 2] = rgb[2];
                vi++;
                posAttr.array[vi * 3]     = line[(i + 1) * 3];
                posAttr.array[vi * 3 + 1] = line[(i + 1) * 3 + 1];
                posAttr.array[vi * 3 + 2] = line[(i + 1) * 3 + 2];
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

    // ── E-Field Lines (Cyan) ─────────────────────────────────────────
    _buildEFieldLines() {
        // N=128 worst case: 300 lines × ~144 segments × 2 verts ≈ 86K. Round up.
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
        // B lines integrate longer (closed loops, 1.5× E maxSteps).
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
        const maxArrows = 4000;
        const positions = new Float32Array(maxArrows * 2 * 3); // 2 verts per arrow
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
        this._poyntingVectors.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.scene.add(this._poyntingVectors);
    }

    updatePoyntingVectors(fieldData) {
        if (!this._poyntingVectors) this._buildPoyntingVectors();
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
        // Arrow length in world units (voxels). Constant ≈2-vox base + log
        // magnitude scaling so a strong Poynting arrow is ~2 voxels long
        // regardless of lattice size — keeps the field-direction read intact
        // without arrows growing into the next neighborhood.
        const arrowBase = 2.0;
        let vi = 0;

        for (let i = 0; i < count && vi < maxArrows; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const vx = vectors[i * 3], vy = vectors[i * 3 + 1], vz = vectors[i * 3 + 2];

            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            const scale = Math.log(1 + mag / maxMag) * arrowBase;
            const nx = vx / mag * scale, ny = vy / mag * scale, nz = vz / mag * scale;

            // Arrow base (darker orange)
            posAttr.array[vi * 6] = px; posAttr.array[vi * 6 + 1] = py; posAttr.array[vi * 6 + 2] = pz;
            colAttr.array[vi * 6] = 0.8; colAttr.array[vi * 6 + 1] = 0.55; colAttr.array[vi * 6 + 2] = 0.15;
            // Arrow tip (bright yellow)
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
        this.scene.add(this._divField);
    }

    updateDivergenceField(fieldData) {
        if (!this._divField) this._buildDivergenceField();
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
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const v = values[i];
            if (Math.abs(v) < threshold) continue;

            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;

            posAttr.array[vi * 3] = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;

            const t = Math.abs(v) / maxVal;
            if (v > 0) {
                // Red (positive divergence = source)
                colAttr.array[vi * 3] = 0.9; colAttr.array[vi * 3 + 1] = 0.2; colAttr.array[vi * 3 + 2] = 0.15;
            } else {
                // Blue (negative divergence = sink)
                colAttr.array[vi * 3] = 0.15; colAttr.array[vi * 3 + 1] = 0.3; colAttr.array[vi * 3 + 2] = 0.9;
            }
            // Size in world-units (voxels). Sources/sinks render 1-3 voxels wide
            // regardless of lattice size — divergence is a per-voxel scalar so
            // each marker should sit on its voxel, not span half the box.
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

    // ── Flux Streamlines (flux colormap) ─────────────────────────────
    // Phase 3b — delegated to ViewportFluxRenderer.
    _buildFluxStreamlines() { this._fluxRenderer._buildFluxStreamlines(); }
    updateFluxStreamlines(streamlines, maxFluxMag) {
        this._fluxRenderer.updateFluxStreamlines(streamlines, maxFluxMag);
    }
    toggleFluxStreamlines(on) { this._fluxRenderer.toggleFluxStreamlines(on); }

    // ── EM Force Volume (Cyan arrows — repurposed from generic Forces) ──
    // EM-force volume — cyan arrows. Uses shared arrow-mesh helpers (RF-2).
    _buildForceVolume() {
        this._forceVolume = this._buildArrowFieldMesh(8000, 0.6);
    }

    updateForceVolume(fieldData) {
        if (!this._forceVolume) this._buildForceVolume();
        // Arrow length ~1.5 vox base × log(magnitude) — keeps EM-force arrows
        // local to their voxel at any lattice size.
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
        const maxArrows = 8000;
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
        this._gravityField.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.scene.add(this._gravityField);
    }

    updateGravityField(fieldData) {
        if (!this._gravityField) this._buildGravityField();
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
        // Gravity arrows ~2-vox base in world units — slightly longer than EM
        // because gravity has gentler gradients, so the log(t) modulation is
        // smaller; constant world size keeps adjacent voxels readable at any N.
        const arrowBase = 2.0;
        let vi = 0;

        for (let i = 0; i < count && vi < maxArrows; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const vx = vectors[i * 3], vy = vectors[i * 3 + 1], vz = vectors[i * 3 + 2];

            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            const t = mag / maxMag; // 0..1
            const scale = Math.log(1 + t) * arrowBase;
            const nx = vx / mag * scale, ny = vy / mag * scale, nz = vz / mag * scale;

            // Base: amber
            posAttr.array[vi * 6] = px; posAttr.array[vi * 6 + 1] = py; posAttr.array[vi * 6 + 2] = pz;
            colAttr.array[vi * 6] = 1.0;
            colAttr.array[vi * 6 + 1] = 0.67;
            colAttr.array[vi * 6 + 2] = 0.0;
            // Tip: bright amber
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

    // ── EM Force (alias to ForceVolume for new badge naming) ─────────
    updateEMForceField(data) { this.updateForceVolume(data); }
    showEMForce(on) { this.toggleForceVolume(on); }

    // ── Gravity Force (alias to GravityField for new badge naming) ──
    updateGravityForceField(data) { this.updateGravityField(data); }
    showGravityForce(on) { this.toggleGravityField(on); }

    // ── Strong Force Volume (Red arrows) ──────────────────────────────
    // Strong-force arrows — red. Uses shared arrow-mesh helpers (RF-2).
    _buildStrongForce() {
        this._strongForce = this._buildArrowFieldMesh(8000, 0.7);
    }

    updateStrongForceField(fieldData) {
        if (!this._strongForce) this._buildStrongForce();
        // 1.5-vox world-space base, identical convention to EM/gravity so the
        // four force overlays render at the same scale.
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
    /** Lazily build a radial-gradient sprite texture shared by all
     *  Points-based overlays that want a soft disc look. */
    static _softSpriteTexture() {
        if (Viewport.__softSprite) return Viewport.__softSprite;
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
        Viewport.__softSprite = tex;
        return tex;
    }

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
            map: Viewport._softSpriteTexture(),
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
        this.scene.add(this._weakField);
    }

    updateWeakField(fieldData) {
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
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const abs = Math.abs(values[i]);
            if (abs < threshold) continue;
            const px = positions[i * 3];
            const py = positions[i * 3 + 1];
            const pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN,
                                      (py - halfN) / halfN,
                                      (pz - halfN) / halfN)) continue;

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

    // FORCE_PALETTES + lerpPalette moved to viewport/color-ramps.js
    // (Wave 1 ticket 1 — docs/SPEC_REFACTOR_LARGE_FILES.md).

    // ── Gaussian Heatmap ─────────────────────────────────────────────
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

        // Custom Gaussian sprite shader for soft circular falloff
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
        this.scene.add(this._forceHeatmap);
    }

    initForceHeatmap() { if (!this._forceHeatmap) this._buildForceHeatmap(); }

    updateForceHeatmap(fieldData, forceType) {
        if (!this._forceHeatmap) this._buildForceHeatmap();
        const posAttr  = this._forceHeatmap.geometry.getAttribute('position');
        const colAttr  = this._forceHeatmap.geometry.getAttribute('particleColor');
        const sizeAttr = this._forceHeatmap.geometry.getAttribute('size');
        const { positions, vectors, count } = fieldData;
        const maxPts = posAttr.array.length / 3;
        const pal = FORCE_PALETTES[forceType] || FORCE_PALETTES.em;

        // Compute magnitudes and max
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
        const sizeBase = 15 + 10 * (this.latticeSize / 64);
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;

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
        // Pre-allocate a pool of Line objects with dashed materials.
        // We reuse a fixed pool and control count via visibility.
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
            this.scene.add(line);
            this._forceStreamlinePool.push(line);
            this._forceStreamlineMats.push(mat);
        }
        this._forceStreamlineCount = 0;
    }

    initForceStreamlines() { if (!this._forceStreamlinePool) this._buildForceStreamlines(); }

    /**
     * Update streamline geometries from pre-computed line arrays.
     * @param {Array<Float32Array>} lines - Array of vertex arrays [x0,y0,z0, x1,y1,z1, ...]
     * @param {string} forceType - 'em' | 'gravity' | 'strong' | 'weak'
     */
    updateForceStreamlines(lines, forceType) {
        if (!this._forceStreamlinePool) this._buildForceStreamlines();
        const pool = this._forceStreamlinePool;
        const mats = this._forceStreamlineMats;
        const pal = FORCE_PALETTES[forceType] || FORCE_PALETTES.em;
        const baseColor = pal.mid;
        const colorHex = new THREE.Color(baseColor[0], baseColor[1], baseColor[2]);

        const usedCount = Math.min(lines.length, pool.length);
        for (let li = 0; li < usedCount; li++) {
            const verts = lines[li];
            const line = pool[li];
            const posAttr = line.geometry.getAttribute('position');
            const maxVerts = posAttr.array.length / 3;
            const vertCount = Math.min(verts.length / 3, maxVerts);

            for (let v = 0; v < vertCount * 3; v++) {
                posAttr.array[v] = verts[v];
            }
            posAttr.needsUpdate = true;
            line.geometry.setDrawRange(0, vertCount);
            line.geometry.computeBoundingSphere();
            line.computeLineDistances();

            mats[li].color.copy(colorHex);
            // Fade opacity for shorter lines
            mats[li].opacity = Math.min(0.8, 0.3 + vertCount / 40 * 0.5);
            line.visible = true;
        }

        // Hide unused pool lines
        for (let li = usedCount; li < pool.length; li++) {
            pool[li].visible = false;
        }
        this._forceStreamlineCount = usedCount;
    }

    /**
     * Animate dash offsets to show flow direction.
     * Call once per frame when flow style is active.
     * @param {number} dt - Time step (frame delta, ~0.016)
     */
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
        // When showing, visibility is set per-line by updateForceStreamlines
    }

    // ── Glyph Field (Instanced Cones) ────────────────────────────────
    // ── Glyph-style force overlays ────────────────────────────────────
    // Each force type (em / gravity / strong / weak) needs its OWN
    // InstancedMesh so multiple forces can be visualised simultaneously.
    // The earlier singleton `_forceGlyphs` was shared across all types →
    // every call to updateForceGlyphs reset the instance count and the
    // LAST force to run would overwrite the previous one (toggling EM
    // while Gravity was on visually "erased" Gravity even though its
    // state flag stayed true). The map pattern mirrors how the arrow
    // overlays already live in separate meshes (_forceVolume for EM,
    // _gravityField for gravity, _strongForce, _weakField).
    _buildForceGlyphMesh(forceType) {
        // Capacity matches the gravity/strong ARROW meshes (8000) so dense
        // overlays render fully. The old 2000-slot cap was smaller than the
        // number of qualifying voxels for whole-lattice fields like gravity
        // on a flux pulse at N=32 (stride=1 → 32³ samples, ~4–6 K pass the
        // 3 % threshold), causing the mesh to fill in z-scan order and
        // silently drop every voxel past the halfway point — a visible
        // "cut in half" effect on the rendered glyph cloud.
        const maxInstances = 8000;
        const coneGeo = new THREE.ConeGeometry(0.3, 1.0, 6);
        // Rotate cone so it points along +Z by default (setFromUnitVectors
        // orients the +Z axis toward the force direction — see _glyphUp).
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
        this.scene.add(mesh);
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
            // Shared math scratch — one allocation reused across all glyph
            // meshes because updateForceGlyphs runs sequentially per type.
            this._glyphMatrix = new THREE.Matrix4();
            this._glyphQuat = new THREE.Quaternion();
            this._glyphUp = new THREE.Vector3(0, 0, 1);
            this._glyphDir = new THREE.Vector3();
            this._glyphScale = new THREE.Vector3();
        }
    }

    // Legacy shim kept for external callers (init / dispose / scale switch).
    _buildForceGlyphs() { this._ensureForceGlyphInfra(); }
    initForceGlyphs() { this._ensureForceGlyphInfra(); }

    updateForceGlyphs(fieldData, forceType) {
        this._ensureForceGlyphInfra();
        const mesh = this._forceGlyphMeshes[forceType] || this._forceGlyphMeshes.em;
        const { positions, vectors, count } = fieldData;
        // Read capacity from the mesh itself — stays in lockstep with
        // whatever _buildForceGlyphMesh allocated, even if that constant
        // changes again in the future.
        const maxInstances = mesh.count === undefined ? 8000 : (mesh.instanceMatrix.array.length / 16);
        const pal = FORCE_PALETTES[forceType] || FORCE_PALETTES.em;

        // Compute magnitudes on a per-type cache — sharing one mag cache
        // across types would race if glyphs ever become async; keep it
        // per-type for safety and because the extra ~16KB is negligible.
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
        // Glyph world-size: ~0.8 voxel base × log(magnitude). Each glyph stays
        // local to its voxel at any lattice size — same convention as arrow
        // overlays, just visualized as instanced meshes instead of line segments.
        const scaleBase = 0.8;
        let vi = 0;

        const mat4 = this._glyphMatrix;
        const quat = this._glyphQuat;
        const up = this._glyphUp;
        const dir = this._glyphDir;
        const scaleVec = this._glyphScale;
        const colorArr = mesh.instanceColor.array;

        // First pass: count how many samples pass the magnitude threshold.
        // If that's more than the mesh capacity, stride-subsample so the
        // rendered glyphs cover the WHOLE filtered set uniformly instead
        // of being truncated at scan-order position maxInstances (which
        // clipped gravity to the first ~half of z and produced a visible
        // straight cut in the middle of the rendered field).
        let qualifying = 0;
        for (let i = 0; i < count; i++) if (mags[i] >= threshold) qualifying++;
        const sampleStride = qualifying > maxInstances
            ? Math.ceil(qualifying / maxInstances)
            : 1;
        let qualifyingSeen = 0;

        for (let i = 0; i < count && vi < maxInstances; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            // Subsample by skipping (stride-1) of every `stride` qualifying
            // voxels. When qualifying ≤ maxInstances this is a no-op.
            if ((qualifyingSeen++ % sampleStride) !== 0) continue;
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;

            const t = mag / maxMag;
            const scale = Math.log(1 + t * 9) / Math.log(10) * scaleBase;

            // Direction quaternion
            dir.set(vectors[i * 3] / mag, vectors[i * 3 + 1] / mag, vectors[i * 3 + 2] / mag);
            quat.setFromUnitVectors(up, dir);

            // Build matrix: translation * rotation * scale. Reuse a scratch
            // Vector3 for scale to avoid per-instance allocation.
            scaleVec.set(scale, scale, scale * 1.5);
            mat4.makeRotationFromQuaternion(quat);
            mat4.scale(scaleVec);
            mat4.setPosition(px, py, pz);
            mesh.setMatrixAt(vi, mat4);

            // Color (per-type palette — gravity is purple, EM is cyan/blue,
            // strong is red, weak is violet — so stacked overlays are
            // distinguishable by hue alone).
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

    // `visible` can be a boolean (apply to ALL force-glyph meshes) or an
    // object { em, gravity, strong, weak } mapping each to a boolean so the
    // caller can drive per-force visibility from the field-flag state.
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

    /**
     * Hide all force visualization styles (called on style switch).
     */
    hideAllForceStyles() {
        // Arrows (existing meshes)
        if (this._forceVolume) this._forceVolume.visible = false;
        if (this._gravityField) this._gravityField.visible = false;
        if (this._strongForce) this._strongForce.visible = false;
        if (this._weakField) this._weakField.visible = false;
        // Heatmap
        this.showForceHeatmap(false);
        // Streamlines
        this.showForceStreamlines_vis(false);
        // Glyphs
        this.showForceGlyphs(false);
    }

    /**
     * Re-show arrow-style force meshes for active forces.
     * Called when switching back to arrows style.
     * @param {object} fieldState - { showForceEM, showForceGravity, showForceStrong, showForceWeak }
     */
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
        this.scene.add(this._darkMatterHalo);
    }

    updateDarkMatterHalo(particles, fluxMag, latticeSize) {
        if (!this._darkMatterHalo) this._buildDarkMatterHalo();
        const posAttr = this._darkMatterHalo.geometry.getAttribute('position');
        const colAttr = this._darkMatterHalo.geometry.getAttribute('particleColor');
        const sizeAttr = this._darkMatterHalo.geometry.getAttribute('size');
        const N = latticeSize;
        const kGen = 1.533; // K_GENESIS = 3 * K_B
        let vi = 0;
        const maxPts = 8000;

        // For each voxel: if sub-threshold flux AND void state -> dark matter
        // Subsample for performance: step=4 for N>64, step=2 for N>24, else 1
        // (avoids 110K+ iterations at L=96 with step=2)
        const step = N > 64 ? 4 : (N > 24 ? 2 : 1);
        for (let z = 0; z < N && vi < maxPts; z += step) {
            for (let y = 0; y < N && vi < maxPts; y += step) {
                for (let x = 0; x < N && vi < maxPts; x += step) {
                    const idx = z * N * N + y * N + x;
                    const mag = fluxMag[idx];
                    // Sub-threshold: flux exists but below genesis
                    if (mag > 0.003 && mag < kGen) {
                        const t = mag / kGen; // 0..1 normalized
                        // Cell-centered coordinates so this overlay aligns with
                        // Flux Volume / Slice / E / B / Poynting / Divergence,
                        // all of which place samples at voxel centers (x+0.5).
                        posAttr.array[vi * 3]     = x + 0.5;
                        posAttr.array[vi * 3 + 1] = y + 0.5;
                        posAttr.array[vi * 3 + 2] = z + 0.5;
                        // Purple gradient: faint → bright purple as flux approaches threshold
                        colAttr.array[vi * 3] = 0.3 + t * 0.4;  // R
                        colAttr.array[vi * 3 + 1] = 0.1 + t * 0.15; // G
                        colAttr.array[vi * 3 + 2] = 0.5 + t * 0.4;  // B
                        // Size in world units: 1 voxel base, swell up to 5× at threshold.
                        // Multiply by `step` (not step·0.7) so a subsampled point covers
                        // exactly its skipped neighbors — preserves visual density.
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
        // Dark translucent sphere — FTD "capture radius" where v_circ > C_SPEED
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
        this.scene.add(this._eventHorizonSphere);

        // Orange equatorial ring — accretion disk boundary indicator
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
        this.scene.add(this._eventHorizonRing);
    }

    setEventHorizon(active, radius) {
        if (!this._eventHorizonSphere) this._buildEventHorizon();
        if (active && radius > 0) {
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
        const maxSegments = 1200; // 100 particles * 12 edges
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
        this._dampingZones.frustumCulled = false; // dynamic geo — see _eFieldLines
        this._dampingZones.renderOrder = 2;
        this.scene.add(this._dampingZones);
    }

    updateDampingZones(particles, latticeSize) {
        if (!this._dampingZones) this._buildDampingZones();
        const posAttr = this._dampingZones.geometry.getAttribute('position');
        const colAttr = this._dampingZones.geometry.getAttribute('color');
        let si = 0;

        // 12 edges of a unit cube, scaled to 3x3x3 around particle.
        // Particles render at voxel CENTRE world-coords (p.x + 0.5) — the
        // universal Scale-0 convention. Earlier comment here wrongly claimed
        // "raw lattice coordinates"; that was the bug — the 3×3×3 damping
        // cage was drawn centred on voxel CORNER `p.x`, so it sat 0.5 voxels
        // NW-down from every particle. Adding the same +0.5 offset used by
        // updateParticles/Flux/E/B/etc. re-centres the cage on the particle.
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
                // Red tint
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
        this.scene.add(this._genesisIsosurface);
    }

    updateGenesisIsosurface(fluxMag, latticeSize, kGenesis) {
        if (!this._genesisIsosurface) this._buildGenesisIsosurface();
        const posAttr = this._genesisIsosurface.geometry.getAttribute('position');
        const colAttr = this._genesisIsosurface.geometry.getAttribute('particleColor');
        const sizeAttr = this._genesisIsosurface.geometry.getAttribute('size');
        const N = latticeSize;
        let vi = 0;
        const band = kGenesis * 0.15; // 15% band around threshold

        // Subsample for large lattices: step=4 for N>64, step=2 for N>24, else 1
        // (aligned with DarkMatter halo thresholds to avoid 110K+ iterations at L=48/96)
        const step = N > 64 ? 4 : (N > 24 ? 2 : 1);

        for (let z = 0; z < N && vi < 4000; z += step) {
            for (let y = 0; y < N && vi < 4000; y += step) {
                for (let x = 0; x < N && vi < 4000; x += step) {
                    const mag = fluxMag[z * N * N + y * N + x];
                    const dist = Math.abs(mag - kGenesis);
                    if (dist < band && mag > 0.01) {
                        const t = 1.0 - dist / band; // 1=on threshold, 0=edge of band
                        // Cell-centered: aligns with Flux Volume + Dark Matter + DivJ.
                        posAttr.array[vi * 3]     = x + 0.5;
                        posAttr.array[vi * 3 + 1] = y + 0.5;
                        posAttr.array[vi * 3 + 2] = z + 0.5;
                        // Green glow: bright at threshold, fading at band edges
                        colAttr.array[vi * 3] = 0.15 + t * 0.15;
                        colAttr.array[vi * 3 + 1] = 0.7 + t * 0.3;
                        colAttr.array[vi * 3 + 2] = 0.2 + t * 0.15;
                        // World-space size: 1.5-vox base + 4× swell on threshold,
                        // multiplied by subsampling step so a single rendered point
                        // covers the voxels we skipped.
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
    // O(N^2) pair evaluation for manifest particles. Performance note:
    // with typical particle counts (<200) this is negligible, but would
    // need spatial hashing if counts exceed ~1000.
    _buildConfinementStrings() {
        const maxVerts = 400 * 2; // 400 strings
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
        this._confinementStrings.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.scene.add(this._confinementStrings);
    }

    updateConfinementStrings(bridge) {
        if (!this._confinementStrings) this._buildConfinementStrings();

        const posAttr = this._confinementStrings.geometry.getAttribute('position');
        const colAttr = this._confinementStrings.geometry.getAttribute('color');
        const maxVerts = posAttr.array.length / 3;

        let vi = 0;
        const kb = bridge.getParam ? bridge.getParam('kb') : K_B;
        const J2_threshold_dist2 = 120.0; // Break threshold scale mimicking V(r) ~ sigma*r tension snap

        // Use getParticleData's positions buffer, which is already in voxel-
        // centre world coords (p.x+0.5). Legacy code here read from a
        // `ptData.states` flat buffer that no current bridge emits — the
        // toggle was silently dead (undefined dereference in the bridge
        // never reached; confinement lines never drew). Switching to the
        // live positions buffer restores the overlay AND aligns it with
        // every other Scale-0 overlay by construction.
        const ptData = bridge.getParticleData();
        if (!ptData || ptData.count < 2 || !ptData.positions) {
            this._confinementStrings.geometry.setDrawRange(0, 0);
            return;
        }
        const pos = ptData.positions;

        // O(N²) evaluation for topological deformation between manifest states.
        // getParticleData already filters void-density noise, so every emitted
        // particle is a legitimate manifest source for the confinement tension.
        for (let i = 0; i < ptData.count; i++) {
            const xi = pos[i * 3], yi = pos[i * 3 + 1], zi = pos[i * 3 + 2];
            for (let j = i + 1; j < ptData.count; j++) {
                const dx = pos[j * 3]     - xi;
                const dy = pos[j * 3 + 1] - yi;
                const dz = pos[j * 3 + 2] - zi;
                const r2 = dx * dx + dy * dy + dz * dz;

                // If they are separated but before the snap point (hadronization)
                if (r2 > 1.0 && r2 < J2_threshold_dist2) {
                    const t = r2 / J2_threshold_dist2;
                    const alpha = 1.0 - t * 0.4;
                    const invR = 1.0 / Math.sqrt(r2);
                    // Color axis simulation (mapping spatial differentiation to
                    // RGB SU(3) proxies — projection of separation direction).
                    const r = Math.abs(dx) * invR * alpha + 0.2;
                    const g = Math.abs(dy) * invR * alpha + 0.2;
                    const b = Math.abs(dz) * invR * alpha + 0.2;

                    if (vi + 2 > maxVerts) break;

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
        this.scene.add(this._dualFluxVolume);
    }

    updateDualFluxVolume(lData, rData) {
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

        // Boundary clipping for dual substrate
        const halfN = this._halfN;

        // L substrate (warm: orange-red). Size in world-units: 1 voxel base
        // + 4× swell at peak |J_L|. Constant absolute size so the L/R dot
        // pair sits on each voxel rather than smearing over neighbors.
        for (let i = 0; i < lCount && vi < maxPts; i++) {
            const mag = dualMags[i];
            if (mag < threshold) continue;
            const px = lData.positions[i * 3], py = lData.positions[i * 3 + 1], pz = lData.positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            posAttr.array[vi * 3] = px; posAttr.array[vi * 3 + 1] = py; posAttr.array[vi * 3 + 2] = pz;
            const t = mag / maxVal;
            colAttr.array[vi * 3] = 0.9 * t; colAttr.array[vi * 3 + 1] = 0.4 * t; colAttr.array[vi * 3 + 2] = 0.15 * t;
            sizeAttr.array[vi] = 1.0 + 4.0 * t;
            vi++;
        }
        // R substrate (cool: blue-purple)
        for (let i = 0; i < rCount && vi < maxPts; i++) {
            const mag = dualMags[lCount + i];
            if (mag < threshold) continue;
            const px = rData.positions[i * 3], py = rData.positions[i * 3 + 1], pz = rData.positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
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
        this.scene.add(this._chiralityField);
    }

    updateChiralityField(fieldData) {
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
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const v = values[i];
            if (Math.abs(v) < threshold) continue;

            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;

            posAttr.array[vi * 3] = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;

            const t = Math.abs(v) / maxVal;
            if (v > 0) {
                // L-dominant: warm red
                colAttr.array[vi * 3] = 0.9 * t; colAttr.array[vi * 3 + 1] = 0.25 * t; colAttr.array[vi * 3 + 2] = 0.15 * t;
            } else {
                // R-dominant: cool blue
                colAttr.array[vi * 3] = 0.15 * t; colAttr.array[vi * 3 + 1] = 0.35 * t; colAttr.array[vi * 3 + 2] = 0.9 * t;
            }
            // World-space size: 1 voxel base + 4× swell at strong L/R dominance.
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
        this.scene.add(this._lightField);
    }

    updateLightField(poyntingData) {
        if (!this._lightField) this._buildLightField();
        const posAttr = this._lightField.geometry.getAttribute('position');
        const colAttr = this._lightField.geometry.getAttribute('particleColor');
        const sizeAttr = this._lightField.geometry.getAttribute('size');
        const { positions, vectors, count } = poyntingData;
        const maxPts = posAttr.array.length / 3;

        // Compute |S| magnitudes and find max
        let maxMag = 0;
        for (let i = 0; i < count; i++) {
            const sx = vectors[i * 3], sy = vectors[i * 3 + 1], sz = vectors[i * 3 + 2];
            const m = Math.sqrt(sx * sx + sy * sy + sz * sz);
            if (m > maxMag) maxMag = m;
        }
        const threshold = maxMag * 0.03;
        const halfN = this._halfN;
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const sx = vectors[i * 3], sy = vectors[i * 3 + 1], sz = vectors[i * 3 + 2];
            const mag = Math.sqrt(sx * sx + sy * sy + sz * sz);
            if (mag < threshold) continue;

            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;

            posAttr.array[vi * 3] = px;
            posAttr.array[vi * 3 + 1] = py;
            posAttr.array[vi * 3 + 2] = pz;

            // Warm yellow-white: brighter at higher |S|
            const t = mag / maxMag;
            colAttr.array[vi * 3] = 1.0 * t;         // R
            colAttr.array[vi * 3 + 1] = 0.92 * t;        // G
            colAttr.array[vi * 3 + 2] = 0.23 * t;        // B (warm yellow)
            // World-space size — light "intensity dots" are 1.5–6 voxels wide,
            // scaled to the local energy flux not the lattice size.
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
    // See docs/SPEC_S0_QUANTUM_OVERLAYS.md for the full catalog.
    //
    // Shared point-cloud renderer. Each of the 5 quantum toggles feeds this
    // same Points object with its own color ramp. Users typically enable
    // one at a time; if multiple are enabled, the latest-updated wins.

    _buildSoftDiscTexture() {
        // Radial-gradient canvas texture. Points rendered with this map look
        // like soft round discs instead of hard square cards — the physics
        // convention for scalar-field density (probability cloud, entropy,
        // potential well, etc.).
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
        const maxPts = 16384;  // ≥ 64³/stride² for a lattice of 64 with stride 2
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        // Soft circular sprite — the standard physics-visualization look for
        // scalar fields (Born density, entropy, potential wells). A future
        // enhancement could swap in a directional glyph for Phase φ, but the
        // cyclic hue already conveys phase adequately and the shared round
        // sprite keeps all 5 overlays visually consistent.
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
        this._quantumField.frustumCulled = false; // dynamic geo — see _eFieldLines
        this._quantumField.renderOrder = 4;
        this.scene.add(this._quantumField);
        this._quantumFieldKind = null;
    }

    _quantumSetVisibility() {
        if (!this._quantumField) return;
        // Phase needles + Φ rubber-sheet have their own objects; the shared
        // point cloud renders only the three truly-point-cloud overlays.
        const pointCloudOn = !!(this._psi2Visible || this._lagrangianVisible || this._entropyVisible);
        this._quantumField.visible = pointCloudOn;
        if (!pointCloudOn) this._quantumField.geometry.setDrawRange(0, 0);
    }

    // Color ramps (rampViridis, rampCyclicHSL, rampDivergingRdBu,
    // rampGrayscale, rampGravWell) moved to viewport/color-ramps.js
    // (Wave 1 ticket 1 — docs/SPEC_REFACTOR_LARGE_FILES.md).

    _populateQuantumField(data, kind, options = {}) {
        if (!this._quantumField) this._buildQuantumField();
        if (!data || !data.positions || !data.values || !data.count) return;
        const posAttr = this._quantumField.geometry.getAttribute('position');
        const colAttr = this._quantumField.geometry.getAttribute('color');
        const maxPts = posAttr.array.length / 3;
        const halfN = this._halfN;
        const { positions, values, count } = data;

        // Normalization range
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
        let vi = 0;
        for (let i = 0; i < count && vi < maxPts; i++) {
            const raw = values[i];
            const v = signed ? raw / denom : Math.abs(raw) / denom;
            if (!signed && v < threshold) continue;
            if (signed && Math.abs(v) < threshold) continue;
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
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

    // ══════════════════════════════════════════════════════════════════
    // ── |ψ|² Born density — viridis probability cloud with breathing ──
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

    // ══════════════════════════════════════════════════════════════════
    // ── Phase φ — directional line-segments (needles) in XZ plane ─────
    // A complex phase is a DIRECTION on the unit circle, not a scalar.
    // Render each voxel as a short line segment pointing at angle φ so
    // users SEE the rotation pattern, not just a colored dot.

    _buildPhaseNeedles() {
        const maxPts = 8192;  // 2 vertices per needle
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
            linewidth: 2,  // note: most WebGL ignores >1 on desktop; texture-free fallback is a line
        });
        this._phaseNeedles = new THREE.LineSegments(geo, mat);
        this._phaseNeedles.visible = false;
        this._phaseNeedles.frustumCulled = false; // dynamic geo — see _eFieldLines
        this._phaseNeedles.renderOrder = 5;
        this.scene.add(this._phaseNeedles);
    }

    togglePhaseField(on) {
        this._phaseVisible = !!on;
        if (!this._phaseNeedles) this._buildPhaseNeedles();
        this._phaseNeedles.visible = !!on;
        if (!on) this._phaseNeedles.geometry.setDrawRange(0, 0);
        this._quantumSetVisibility();
    }

    updatePhaseField(data) {
        this._phaseData = data;
        if (!this._phaseVisible || !data?.count) return;
        if (!this._phaseNeedles) this._buildPhaseNeedles();
        const posAttr = this._phaseNeedles.geometry.getAttribute('position');
        const colAttr = this._phaseNeedles.geometry.getAttribute('color');
        const maxSegments = posAttr.array.length / 6;
        const halfN = this._halfN;
        const len = 1.2;  // needle half-length in lattice units
        const { positions, values, count } = data;
        const rgb = new Float32Array(3);
        let si = 0;
        for (let i = 0; i < count && si < maxSegments; i++) {
            const px = positions[i * 3];
            const py = positions[i * 3 + 1];
            const pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            const phase = values[i];
            // Needle direction: rotate in XZ plane by phase angle. Skip trivial
            // (phase ≈ 0) to avoid visual noise when Dual Substrate is off.
            if (Math.abs(phase) < 0.02) continue;
            const dx = Math.cos(phase) * len;
            const dz = Math.sin(phase) * len;
            // 2 vertices per segment: (origin − dir, origin + dir)
            const base = si * 6;
            posAttr.array[base]     = px - dx;
            posAttr.array[base + 1] = py;
            posAttr.array[base + 2] = pz - dz;
            posAttr.array[base + 3] = px + dx;
            posAttr.array[base + 4] = py;
            posAttr.array[base + 5] = pz + dz;
            rampCyclicHSL(phase, rgb, 0);
            // Same color on both endpoints — creates a solid hue line.
            colAttr.array[base]     = rgb[0]; colAttr.array[base + 1] = rgb[1]; colAttr.array[base + 2] = rgb[2];
            colAttr.array[base + 3] = rgb[0]; colAttr.array[base + 4] = rgb[1]; colAttr.array[base + 5] = rgb[2];
            si++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._phaseNeedles.geometry.setDrawRange(0, si * 2);
    }

    // ══════════════════════════════════════════════════════════════════
    // ── ℒ(x) Lagrangian density — size-attenuated signed cloud ────────
    // Magnitude → point size, sign → color (red=kinetic, blue=potential).
    toggleLagrangianDensityField(on) {
        this._lagrangianVisible = !!on;
        if (!this._quantumField) this._buildQuantumField();
        this._quantumSetVisibility();
    }
    updateLagrangianDensityField(data) {
        this._lagrangianData = data;
        if (!this._lagrangianVisible) return;
        // Concentrate points where |ℒ| is largest — visual cue that action is
        // accumulating there. Lowering the threshold keeps near-zero regions
        // invisible (which is also correct: ℒ ≈ 0 = field is quiescent).
        this._populateQuantumField(data, 'lagrangian', {
            signed: true,
            ramp: (t, out, i) => rampDivergingRdBu(t, out, i),
            normalizer: data?.normalizer,
            threshold: 0.10,  // up from 0.04 — only show points with meaningful ℒ
        });
    }

    // ══════════════════════════════════════════════════════════════════
    // ── Entropy s(x) — jittering sparkles ─────────────────────────────
    // Disorder IS randomness; high-entropy voxels visually jitter around
    // their true position. Low-entropy voxels stay still (crystallised).
    toggleEntropyDensityField(on) {
        this._entropyVisible = !!on;
        if (!this._quantumField) this._buildQuantumField();
        this._entropyJitterSeed = Date.now();
        this._quantumSetVisibility();
    }
    updateEntropyDensityField(data) {
        this._entropyData = data;
        if (!this._entropyVisible) return;
        if (!this._quantumField) this._buildQuantumField();
        const posAttr = this._quantumField.geometry.getAttribute('position');
        const colAttr = this._quantumField.geometry.getAttribute('color');
        const maxPts = posAttr.array.length / 3;
        const halfN = this._halfN;
        const { positions, values, count } = data;
        const JITTER_SCALE = 0.8;  // lattice units of jitter at max entropy
        let vi = 0;
        for (let i = 0; i < count && vi < maxPts; i++) {
            const s = Math.max(0, Math.min(1, values[i]));
            if (s < 0.04) continue;
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            // Random displacement proportional to entropy. Use a deterministic
            // per-voxel seed so the jitter is stable frame-to-frame (avoids
            // epileptic flicker) — visual wobble comes from reseeding on toggle.
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
    // ── Rubber-sheet visualizations ─ moved to topology-sheet-renderer ──
    //
    // Owned by this._topoRenderer (viewport/topology-sheet-renderer.js).
    // Φ gravitational potential + 10 topology sheets (emEnergy, helicity,
    // kretschmann, ePressure, bPressure, kineticEnergy, fisher, coherence,
    // chargeDensity, vorticity). Viewport keeps thin delegators so the
    // external toggleXxxField / updateXxxField API is unchanged.
    // ══════════════════════════════════════════════════════════════════

    toggleGravPotentialField(on) { this._topoRenderer.toggleGravPotential(on); }
    updateGravPotentialField(data) { this._topoRenderer.updateGravPotential(data); }

    toggleEmEnergyField(on)      { this._topoRenderer.toggle('emEnergy', on); }
    toggleChargeDensityField(on) { this._topoRenderer.toggle('chargeDensity', on); }
    toggleVorticityField(on)     { this._topoRenderer.toggle('vorticity', on); }
    toggleHelicityField(on)      { this._topoRenderer.toggle('helicity', on); }
    toggleKretschmannField(on)   { this._topoRenderer.toggle('kretschmann', on); }
    toggleEPressureField(on)     { this._topoRenderer.toggle('ePressure', on); }
    toggleBPressureField(on)     { this._topoRenderer.toggle('bPressure', on); }
    toggleKineticEnergyField(on) { this._topoRenderer.toggle('kineticEnergy', on); }
    toggleFisherField(on)        { this._topoRenderer.toggle('fisher', on); }
    toggleCoherenceField(on)     { this._topoRenderer.toggle('coherence', on); }

    updateEmEnergyField(data)      { this._topoRenderer.update('emEnergy', data); }
    updateChargeDensityField(data) { this._topoRenderer.update('chargeDensity', data); }
    updateVorticityField(data)     { this._topoRenderer.update('vorticity', data); }
    updateHelicityField(data)      { this._topoRenderer.update('helicity', data); }
    updateKretschmannField(data)   { this._topoRenderer.update('kretschmann', data); }
    updateEPressureField(data)     { this._topoRenderer.update('ePressure', data); }
    updateBPressureField(data)     { this._topoRenderer.update('bPressure', data); }
    updateKineticEnergyField(data) { this._topoRenderer.update('kineticEnergy', data); }
    updateFisherField(data)        { this._topoRenderer.update('fisher', data); }
    updateCoherenceField(data)     { this._topoRenderer.update('coherence', data); }

    // ══════════════════════════════════════════════════════════════════
    // ── Event-horizon isosurface overlay (Tier 1, 2026-04-18) ────────
    // Rendered as a semi-transparent point cloud at voxels where the
    // latency proxy L(x) ≥ 0.95.  Not a rubber sheet — the geometry
    // needs to sit in 3D at the horizon location, not on a floor plane.
    // ══════════════════════════════════════════════════════════════════
    _buildHorizonField() {
        // Lazy-alloc 8k points (caps cost; we'll only upload `count` verts).
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
        this.scene.add(points);
        this._horizonField = { points, geo, capacity: max };
    }

    toggleHorizonField(on) {
        if (!this._horizonField) this._buildHorizonField();
        this._horizonField.points.visible = !!on;
    }

    updateHorizonField(data) {
        if (!data?.count) return;
        if (!this._horizonField) this._buildHorizonField();
        const hf = this._horizonField;
        if (!hf.points.visible) return;
        const pos = hf.geo.attributes.position;
        // The horizon sampler emits voxel-centred positions. If the horizon
        // fits in capacity we copy the whole buffer; otherwise we stride-
        // downsample across the full array so the rendered shell stays
        // geometrically representative (not just the first 8k voxels in
        // scan order, which would clip a horizon's bottom-right quadrant).
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
    // Called from the main render loop so the probability cloud "breathes"
    // at ~0.3Hz — conveys that this is a DYNAMIC quantum state, not a
    // static heatmap. No-op when |ψ|² isn't the active field.
    //
    // Time source: `_animationClock` (monotonic ms) accumulated by
    // `advanceAnimationClock(dt)` from the controller only when the sim is
    // running. Previously this used `performance.now()` directly, which
    // advanced on every render call regardless of pause state — so toggling
    // an overlay (which forces one render cycle to repaint) would bump the
    // breathing phase by one frame. Sourcing from the accumulator keeps the
    // opacity pinned whenever the sim is paused, regardless of how many
    // times the viewport re-renders for layout / overlay-toggle reasons.
    _animateQuantumField() {
        if (!this._quantumField || !this._psi2Visible) return;
        if (this._quantumFieldKind !== 'psi2') return;
        const tMs = this._animationClock || 0;
        const phase = (tMs / 1000) * Math.PI * 0.6;  // ~0.3Hz pulse
        const pulse = 0.85 + 0.15 * Math.sin(phase);
        this._quantumField.material.opacity = pulse;
    }

    // Monotonic animation clock. Accumulates only when the sim is running;
    // the controller calls this each animate() tick with the frame delta
    // (wall-clock seconds). Anything time-based in viewport.js that should
    // freeze during pause reads from `this._animationClock` instead of
    // `performance.now()`.
    advanceAnimationClock(dtSeconds) {
        if (!this._animationClock) this._animationClock = 0;
        this._animationClock += (dtSeconds || 0) * 1000;
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
            if (this._boundary) this._boundary.visible = false;
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

        // ── Consciousness mode: dark background, bloom, centered camera ──
        if (mode === 'consciousness') {
            this.scene.background = new THREE.Color(0x050510);
            this._boundaryMode = 'origin';
            this._buildBoundary(this._boundaryShape, 'origin');
            hideAllOverlays();
            // Camera: closer, centered at origin, wider FOV
            this.camera.fov = 55;
            this.camera.updateProjectionMatrix();
            this.controls.target.set(0, 1, 0);
            this.camera.position.set(8, 4, 8);
            this.controls.update();
            // Enable bloom
            this.enablePostProcessing();
            return;
        }

        // Leaving consciousness mode — restore defaults
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
            if (this._boundary) this._boundary.visible = true;

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
            if (this._boundary) this._boundary.visible = true;

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

    _buildPEAxes() {
        // Idempotent rebuild guard (Three-M1 audit, 2026-04-27): if a
        // prior build exists, dispose its geometry+material before
        // overwriting the field reference. Prevents the rare leak path
        // where _buildPEAxes is called twice across a lattice resize.
        const tearDown = (obj) => {
            if (!obj) return;
            this.scene.remove(obj);
            if (obj.geometry) obj.geometry.dispose();
            if (obj.material) obj.material.dispose();
        };
        tearDown(this.peAxes); this.peAxes = null;
        tearDown(this.peGrid); this.peGrid = null;

        const len = 30;

        // ── Axes (RGB lines through origin) ──
        const axVerts = [];
        const axColors = [];
        // X axis (red)
        axVerts.push(-len, 0, 0, len, 0, 0);
        axColors.push(0.5, 0.2, 0.2, 0.9, 0.3, 0.3);
        // Y axis (green)
        axVerts.push(0, -len, 0, 0, len, 0);
        axColors.push(0.2, 0.5, 0.2, 0.3, 0.9, 0.3);
        // Z axis (blue)
        axVerts.push(0, 0, -len, 0, 0, len);
        axColors.push(0.2, 0.2, 0.5, 0.3, 0.3, 0.9);

        const axGeo = new THREE.BufferGeometry();
        axGeo.setAttribute('position', new THREE.Float32BufferAttribute(axVerts, 3));
        axGeo.setAttribute('color', new THREE.Float32BufferAttribute(axColors, 3));
        const axMat = new THREE.LineBasicMaterial({ vertexColors: true, transparent: true, opacity: 0.6 });
        this.peAxes = new THREE.LineSegments(axGeo, axMat);
        this.peAxes.visible = false;
        this.scene.add(this.peAxes);

        // ── Grid (XZ plane lines, separate object for independent toggle) ──
        const grVerts = [];
        const grColors = [];
        for (let i = -len; i <= len; i += 5) {
            if (i === 0) continue;
            grVerts.push(i, 0, -len, i, 0, len);
            grColors.push(0.15, 0.18, 0.25, 0.15, 0.18, 0.25);
            grVerts.push(-len, 0, i, len, 0, i);
            grColors.push(0.15, 0.18, 0.25, 0.15, 0.18, 0.25);
        }
        const grGeo = new THREE.BufferGeometry();
        grGeo.setAttribute('position', new THREE.Float32BufferAttribute(grVerts, 3));
        grGeo.setAttribute('color', new THREE.Float32BufferAttribute(grColors, 3));
        const grMat = new THREE.LineBasicMaterial({ vertexColors: true, transparent: true, opacity: 0.6 });
        this.peGrid = new THREE.LineSegments(grGeo, grMat);
        this.peGrid.visible = false;
        this.scene.add(this.peGrid);
    }

    updateParticles(data) {
        const geo = this.particles.geometry;
        const posAttr = geo.getAttribute('position');
        const colAttr = geo.getAttribute('particleColor');
        const sizeAttr = geo.getAttribute('size');

        const rawCount = Math.min(data.count, MAX_PARTICLES);

        // Clip particles to current boundary shape
        const halfN = this._halfN;
        const needsClip = this._boundaryShape && this._boundaryShape !== 'none' && this._boundaryShape !== 'cube';
        let count = 0;
        for (let i = 0; i < rawCount; i++) {
            const px = data.positions[i * 3];
            const py = data.positions[i * 3 + 1];
            const pz = data.positions[i * 3 + 2];
            if (needsClip) {
                const nx = (px - halfN) / halfN;
                const ny = (py - halfN) / halfN;
                const nz = (pz - halfN) / halfN;
                if (!this._insideBoundary(nx, ny, nz)) continue;
            }
            posAttr.array[count * 3] = px;
            posAttr.array[count * 3 + 1] = py;
            posAttr.array[count * 3 + 2] = pz;
            colAttr.array[count * 3] = data.colors[i * 3];
            colAttr.array[count * 3 + 1] = data.colors[i * 3 + 1];
            colAttr.array[count * 3 + 2] = data.colors[i * 3 + 2];
            sizeAttr.array[count] = data.sizes[i] * this.visualSettings.globalScale;
            count++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;

        geo.setDrawRange(0, count);

    }

    // ── Particle shape and opacity ──────────────────────────────────

    setPointShape(shapeIndex) {
        if (this.particles && this.particles.material.uniforms) {
            this.particles.material.uniforms.shapeType.value = shapeIndex;
        }
    }

    setOpacity(val) {
        if (this.particles && this.particles.material.uniforms) {
            this.particles.material.uniforms.uOpacity.value = val;
        }
        this.visualSettings.opacity = val;
    }

    // ── Element labels + clearMolecularMeshes — delegated to viewport/molecular-renderer.js
    updateElementLabels(labels) { this._molRenderer.updateElementLabels(labels); }
    toggleElementLabels(on)     { this._molRenderer.toggleElementLabels(on); }
    clearElementLabels()        { this._molRenderer.clearElementLabels(); }
    clearMolecularMeshes()      { this._molRenderer.clearMolecularMeshes(); }

    // Override colors from catalog type map (PE mode)
    applyParticleColors(data, typeMap) {
        if (!typeMap || typeMap.size === 0) return;
        const colAttr = this.particles.geometry.getAttribute('particleColor');
        const sizeAttr = this.particles.geometry.getAttribute('size');
        const count = Math.min(data.count, MAX_PARTICLES);

        for (let i = 0; i < count; i++) {
            const pid = data.ids ? data.ids[i] : -1;
            const catId = typeMap.get(pid);
            if (!catId) continue;
            const p = getById(catId);
            if (!p) continue;
            const [r, g, b] = p.display_color;
            colAttr.array[i * 3] = r;
            colAttr.array[i * 3 + 1] = g;
            colAttr.array[i * 3 + 2] = b;
            // Scale size by log mass (relative to electron mass K_B)
            const s = 3.0 + 2.0 * Math.log10(p.mass_mev / K_B + 1.0);
            sizeAttr.array[i] = Math.min(s, 40);
        }
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
    }

    // ── Post-Processing (Consciousness Mode) ──────────────────────

    enablePostProcessing() {
        if (this._composer) {
            this._usePostProcessing = true;
            return;
        }
        const rect = this.container.getBoundingClientRect();
        const w = rect.width || 800;
        const h = rect.height || 600;

        this._composer = new EffectComposer(this.renderer);
        this._composer.addPass(new RenderPass(this.scene, this.camera));

        this._bloomPass = new UnrealBloomPass(
            new THREE.Vector2(w, h),
            1.5,  // strength
            0.4,  // radius
            0.2   // threshold
        );
        this._composer.addPass(this._bloomPass);
        this._usePostProcessing = true;
    }

    disablePostProcessing() {
        this._usePostProcessing = false;
    }

    /** Accessor for the bloom pass. Null when post-processing has never
     *  been enabled (first call to enablePostProcessing constructs it).
     *  The Scene panel's adapter uses this to read current values
     *  without importing Three.js or touching the _composer directly. */
    getBloomPass() {
        return this._bloomPass;
    }

    /** Write bloom parameters without reaching into _bloomPass from
     *  outside. Unknown keys are ignored. No-op when the pass has not
     *  been created yet (toggle bloom on first to make it effective). */
    setBloomParams({ strength, radius, threshold } = {}) {
        const pass = this._bloomPass;
        if (!pass) return;
        if (typeof strength === 'number' && Number.isFinite(strength)) pass.strength = strength;
        if (typeof radius === 'number' && Number.isFinite(radius)) pass.radius = radius;
        if (typeof threshold === 'number' && Number.isFinite(threshold)) pass.threshold = threshold;
    }

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
            const dtMs = now - (this._lastSpinArrowUpdateMs || now);
            this.spinArrowManager.update(dtMs);
            this._lastSpinArrowUpdateMs = now;
        }
        if (this._usePostProcessing && this._composer) {
            this._composer.render();
        } else {
            this.renderer.render(this.scene, this.camera);
        }
    }

    _onResize() {
        const rect = this.container.getBoundingClientRect();
        const w = rect.width;
        const h = rect.height;
        if (w === 0 || h === 0) return;
        this.camera.aspect = w / h;
        this.camera.updateProjectionMatrix();
        this.renderer.setSize(w, h);
        if (this._composer) {
            this._composer.setSize(w, h);
        }
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

        // Helper: dispose a Group by traversing all children
        const disposeGroup = (group) => {
            if (!group) return;
            this.scene.remove(group);
            group.traverse(child => {
                if (child.geometry) child.geometry.dispose();
                if (child.material) {
                    if (child.material.map) child.material.map.dispose();
                    child.material.dispose();
                }
            });
        };

        this.renderer.dispose();
        disposeMesh(this.particles);

        // Wireframe is a Group containing LineSegments — traverse children
        disposeGroup(this.wireframe);

        // Post-processing composer render targets
        if (this._composer) {
            this._composer.renderTarget1.dispose();
            this._composer.renderTarget2.dispose();
            this._composer = null;
        }

        // Phase 3b: FluxRenderer owns _fluxVolume + _fluxStreamlines.
        this._fluxRenderer?.dispose();

        // Core overlays (geometry+material pairs)
        const simpleOverlays = [
            'velocityVectors', 'trails',
            '_fieldHeatmap', '_fieldVectors',
            '_peStreamlines', '_gravityVectors', '_particleForces',
        ];
        for (const name of simpleOverlays) disposeMesh(this[name]);

        // Molecular renderer owns: bondLines, _bondCylinders, _bondLight,
        // _nucleusShells, _orbitalShells, _orbitalLobes,
        // _aeForceIonic/Vdw/Bond/Net, and element labels.
        this._molRenderer?.dispose();

        // Field visualization overlays (Scale 0 streamlines, volumes, etc.)
        const fieldOverlays = [
            '_eFieldLines', '_bFieldLines', '_poyntingVectors', '_divField',
            '_forceVolume', '_gravityField', '_strongForce', '_weakField',
            '_forceHeatmap',
            '_dualFluxVolume',
            '_chiralityField', '_lightField',
            '_darkMatterHalo', '_dampingZones', '_genesisIsosurface',
            '_confinementStrings',
        ];
        for (const name of fieldOverlays) disposeMesh(this[name]);

        // Per-force glyph meshes (one InstancedMesh per force type so stacked
        // forces render simultaneously — see _buildForceGlyphMesh).
        if (this._forceGlyphMeshes) {
            for (const m of Object.values(this._forceGlyphMeshes)) disposeMesh(m);
            this._forceGlyphMeshes = null;
        }

        // Force streamline pool (array of Line objects, not a single mesh)
        if (this._forceStreamlinePool) {
            for (const line of this._forceStreamlinePool) disposeMesh(line);
            this._forceStreamlinePool = null;
            this._forceStreamlineMats = null;
        }

        // Rubber-sheet visualizations (10 topology sheets + Φ gravitational
        // potential) are owned by TopologySheetRenderer — it tears them down.
        this._topoRenderer?.dispose();
        // Event-horizon point cloud (distinct from the Scale 5 black-hole
        // event-horizon sphere/ring above — this is the Scale 0 latency
        // isosurface at L ≥ 0.95).
        if (this._horizonField) {
            disposeMesh(this._horizonField.points);
            this._horizonField = null;
        }
        // Quantum-field |ψ|² volumetric cloud.
        disposeMesh(this._quantumField);
        this._quantumField = null;
        // Phase needles (line-segment bundle rendering arg(J)).
        disposeMesh(this._phaseNeedles);
        this._phaseNeedles = null;

        // Raycasting/inspector helpers
        disposeMesh(this._voidBox);
        disposeMesh(this._voxelHighlight);
        disposeMesh(this._symHighlights);

        // Event horizon (black hole visualization)
        disposeMesh(this._eventHorizonSphere);
        disposeMesh(this._eventHorizonRing);

        // Coordinate helpers
        disposeMesh(this.axes);
        disposeMesh(this.peAxes);
        disposeMesh(this.peGrid);
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
}
