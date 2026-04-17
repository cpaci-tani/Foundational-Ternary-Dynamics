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
        this.showFlux = true;  // flux volume ON by default
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
            color: 0x1e2d44, transparent: true, opacity: 0.18,
            depthWrite: false,
        });

        let group;
        switch (shape) {
            case 'cube': group = this._buildCubeBoundary(mat, mode); break;
            case 'sphere': group = this._buildSphereBoundary(mat); break;
            case 'dodecahedron': group = this._buildPlatonicBoundary('dodecahedron', mat); break;
            case 'icosahedron': group = this._buildPlatonicBoundary('icosahedron', mat); break;
            case 'octahedron': group = this._buildPlatonicBoundary('octahedron', mat); break;
            case 'cylinder': group = this._buildCylinderBoundary(mat); break;
            case 'torus': group = this._buildTorusBoundary(mat); break;
            default: group = this._buildCubeBoundary(mat, mode); break;
        }

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

    _buildCubeBoundary(mat, mode) {
        const vertices = [];
        const s = (mode === 'lattice') ? this.latticeSize : 1;

        // 12 edges of bounding cube
        const h = s / 2;
        const corners = (mode === 'lattice')
            ? [[0, 0, 0], [s, 0, 0], [s, s, 0], [0, s, 0], [0, 0, s], [s, 0, s], [s, s, s], [0, s, s]]
            : [[-h, -h, -h], [h, -h, -h], [h, h, -h], [-h, h, -h], [-h, -h, h], [h, -h, h], [h, h, h], [-h, h, h]];
        const edges = [
            [0, 1], [1, 2], [2, 3], [3, 0],
            [4, 5], [5, 6], [6, 7], [7, 4],
            [0, 4], [1, 5], [2, 6], [3, 7]
        ];
        for (const [a, b] of edges) {
            vertices.push(...corners[a], ...corners[b]);
        }

        // Subdivision lines only in lattice mode (sparse — just midpoint cross)
        if (mode === 'lattice') {
            const step = Math.max(8, Math.floor(s / 2));
            for (let i = step; i < s; i += step) {
                vertices.push(i, 0, 0, i, s, 0);
                vertices.push(i, 0, s, i, s, s);
                vertices.push(0, i, 0, s, i, 0);
                vertices.push(0, i, s, s, i, s);
                vertices.push(0, 0, i, s, 0, i);
                vertices.push(0, s, i, s, s, i);
            }
        }

        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        const group = new THREE.Group();
        group.add(new THREE.LineSegments(geo, mat));
        return group;
    }

    _buildSphereBoundary(mat) {
        const group = new THREE.Group();

        // Wireframe sphere
        const sphereGeo = new THREE.SphereGeometry(1, 24, 16);
        const edgesGeo = new THREE.EdgesGeometry(sphereGeo);
        group.add(new THREE.LineSegments(edgesGeo, mat));
        sphereGeo.dispose();

        // 3 great-circle rings for structure
        const ringMat = mat.clone();
        ringMat.opacity = 0.5;
        const segments = 64;
        for (let axis = 0; axis < 3; axis++) {
            const pts = [];
            for (let i = 0; i <= segments; i++) {
                const t = (i / segments) * Math.PI * 2;
                const c = Math.cos(t), sn = Math.sin(t);
                if (axis === 0) pts.push(new THREE.Vector3(0, c, sn));
                else if (axis === 1) pts.push(new THREE.Vector3(c, 0, sn));
                else pts.push(new THREE.Vector3(c, sn, 0));
            }
            const ringGeo = new THREE.BufferGeometry().setFromPoints(pts);
            group.add(new THREE.Line(ringGeo, ringMat));
        }

        return group;
    }

    _buildPlatonicBoundary(shape, mat) {
        const group = new THREE.Group();
        let solidGeo;
        const detail = 0;
        switch (shape) {
            case 'dodecahedron': solidGeo = new THREE.DodecahedronGeometry(1, detail); break;
            case 'icosahedron': solidGeo = new THREE.IcosahedronGeometry(1, detail); break;
            case 'octahedron': solidGeo = new THREE.OctahedronGeometry(1, detail); break;
        }
        const edgesGeo = new THREE.EdgesGeometry(solidGeo);
        group.add(new THREE.LineSegments(edgesGeo, mat));
        solidGeo.dispose();
        return group;
    }

    _buildCylinderBoundary(mat) {
        const group = new THREE.Group();

        // Cylinder wireframe
        const cylGeo = new THREE.CylinderGeometry(1, 1, 2, 24, 1, true);
        const edgesGeo = new THREE.EdgesGeometry(cylGeo);
        group.add(new THREE.LineSegments(edgesGeo, mat));
        cylGeo.dispose();

        // Top and bottom cap circles
        const capMat = mat.clone();
        capMat.opacity = 0.4;
        const segments = 48;
        for (const y of [-1, 1]) {
            const pts = [];
            for (let i = 0; i <= segments; i++) {
                const t = (i / segments) * Math.PI * 2;
                pts.push(new THREE.Vector3(Math.cos(t), y, Math.sin(t)));
            }
            const capGeo = new THREE.BufferGeometry().setFromPoints(pts);
            group.add(new THREE.Line(capGeo, capMat));
        }

        return group;
    }

    _buildTorusBoundary(mat) {
        const group = new THREE.Group();
        const torusGeo = new THREE.TorusGeometry(0.7, 0.3, 12, 36);
        const edgesGeo = new THREE.EdgesGeometry(torusGeo);
        const mesh = new THREE.LineSegments(edgesGeo, mat);
        // Three.js TorusGeometry lies in XY plane (hole along Z) by default.
        // Rotate so major circle lies in XZ plane (hole along Y) to match
        // _insideBoundary clipping and the PE grid orientation.
        mesh.rotation.x = -Math.PI / 2;
        group.add(mesh);
        torusGeo.dispose();
        return group;
    }

    setBoundaryShape(shape) {
        this._buildBoundary(shape, this._boundaryMode);
    }

    /**
     * Test whether a point (in normalized coords -1..1 from center) is inside
     * the current boundary shape. Used to clip flux volume rendering.
     */
    _insideBoundary(nx, ny, nz) {
        switch (this._boundaryShape) {
            case 'none':
            case 'cube':
                return true; // cube = full lattice, no clipping
            case 'sphere':
                return (nx * nx + ny * ny + nz * nz) <= 1.0;
            case 'octahedron':
                return (Math.abs(nx) + Math.abs(ny) + Math.abs(nz)) <= 1.0;
            case 'dodecahedron': {
                // Dodecahedron defined by 6 pairs of face normals
                // Inradius of unit dodecahedron ≈ 0.7946
                const phi = 1.618033988749895;
                const ir = 0.7946; // inradius / circumradius
                const normals = [
                    [0, 1, phi], [0, -1, phi], [0, 1, -phi], [0, -1, -phi],
                    [1, phi, 0], [-1, phi, 0], [1, -phi, 0], [-1, -phi, 0],
                    [phi, 0, 1], [-phi, 0, 1], [phi, 0, -1], [-phi, 0, -1],
                ];
                for (const n of normals) {
                    const len = Math.sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
                    const d = (nx * n[0] + ny * n[1] + nz * n[2]) / len;
                    if (d > ir) return false;
                }
                return true;
            }
            case 'icosahedron': {
                // Icosahedron defined by 10 pairs of face normals
                // Inradius of unit icosahedron ≈ 0.7558
                const phi = 1.618033988749895;
                const ir = 0.7558;
                const normals = [
                    [1, 1, 1], [1, 1, -1], [1, -1, 1], [1, -1, -1],
                    [-1, 1, 1], [-1, 1, -1], [-1, -1, 1], [-1, -1, -1],
                    [0, phi, 1 / phi], [0, phi, -1 / phi], [0, -phi, 1 / phi], [0, -phi, -1 / phi],
                    [1 / phi, 0, phi], [-1 / phi, 0, phi], [1 / phi, 0, -phi], [-1 / phi, 0, -phi],
                    [phi, 1 / phi, 0], [phi, -1 / phi, 0], [-phi, 1 / phi, 0], [-phi, -1 / phi, 0],
                ];
                for (const n of normals) {
                    const len = Math.sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
                    const d = (nx * n[0] + ny * n[1] + nz * n[2]) / len;
                    if (d > ir) return false;
                }
                return true;
            }
            case 'cylinder':
                return (nx * nx + nz * nz) <= 1.0 && Math.abs(ny) <= 1.0;
            case 'torus': {
                // Torus: major R=0.7, minor r=0.3 (matches _buildTorusBoundary)
                const dist_xz = Math.sqrt(nx * nx + nz * nz);
                const dx = dist_xz - 0.7;
                return (dx * dx + ny * ny) <= (0.3 * 0.3);
            }
            default:
                return true;
        }
    }

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
        // Gravity rubber-sheet needs to match the new lattice footprint.
        this._rebuildGravSurfaceIfResized?.();
        
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

        // Rebuild flux volume for new size
        if (this._fluxVolume) {
            this.scene.remove(this._fluxVolume);
            this._fluxVolume.geometry.dispose();
            this._fluxVolume.material.dispose();
            this._fluxVolume = null;
            this._fluxVolumeSize = 0;
        }
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
            const mat = new THREE.LineBasicMaterial({ color: 0xffff00, linewidth: 2 });
            this._voxelHighlight = new THREE.LineSegments(edges, mat);
            this.scene.add(this._voxelHighlight);
        }
        if (active) {
            this._voxelHighlight.position.set(x, y, z);
            this._voxelHighlight.visible = true;
        } else {
            this._voxelHighlight.visible = false;
        }
    }

    setSymmetryHighlights(x, y, z, u1, su2, su3) {
        if (!this._symHighlights) {
            const geo = new THREE.BoxGeometry(1.0, 1.0, 1.0);
            const edges = new THREE.EdgesGeometry(geo);
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
                            dummy.position.set(x + dx, y + dy, z + dz);
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
    _buildBondLines() {
        const MAX_BONDS = 500;
        const vertices = new Float32Array(MAX_BONDS * 2 * 3);
        const colors = new Float32Array(MAX_BONDS * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.8,
        });
        this.bondLines = new THREE.LineSegments(geo, mat);
        this.bondLines.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.bondLines.visible = true;
        this.scene.add(this.bondLines);
    }

    updateBondLines(atomData) {
        if (!this.bondLines) this._buildBondLines();
        if (!atomData || !atomData.bonds || atomData.bondCount === 0) {
            this.bondLines.geometry.setDrawRange(0, 0);
            return;
        }

        const posAttr = this.bondLines.geometry.getAttribute('position');
        const colAttr = this.bondLines.geometry.getAttribute('color');
        const maxBonds = posAttr.array.length / 6;
        const n = Math.min(atomData.bondCount, maxBonds);

        for (let b = 0; b < n; b++) {
            const idxA = atomData.bonds[b * 2];
            const idxB = atomData.bonds[b * 2 + 1];

            // Start vertex (atom A position)
            posAttr.array[b * 6] = atomData.positions[idxA * 3];
            posAttr.array[b * 6 + 1] = atomData.positions[idxA * 3 + 1];
            posAttr.array[b * 6 + 2] = atomData.positions[idxA * 3 + 2];
            // End vertex (atom B position)
            posAttr.array[b * 6 + 3] = atomData.positions[idxB * 3];
            posAttr.array[b * 6 + 4] = atomData.positions[idxB * 3 + 1];
            posAttr.array[b * 6 + 5] = atomData.positions[idxB * 3 + 2];

            // Bond color: blend the two atom colors
            const rA = atomData.colors[idxA * 3], gA = atomData.colors[idxA * 3 + 1], bA = atomData.colors[idxA * 3 + 2];
            const rB = atomData.colors[idxB * 3], gB = atomData.colors[idxB * 3 + 1], bB = atomData.colors[idxB * 3 + 2];
            colAttr.array[b * 6] = rA;
            colAttr.array[b * 6 + 1] = gA;
            colAttr.array[b * 6 + 2] = bA;
            colAttr.array[b * 6 + 3] = rB;
            colAttr.array[b * 6 + 4] = gB;
            colAttr.array[b * 6 + 5] = bB;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this.bondLines.geometry.setDrawRange(0, n * 2);
    }

    toggleBondLines(on) {
        if (!this.bondLines) this._buildBondLines();
        this.bondLines.visible = on;
        if (!on) this.bondLines.geometry.setDrawRange(0, 0);
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

    _buildGravityVectors() {
        const vertices = new Float32Array(MAX_FIELD_GRID * 2 * 3);
        const colors = new Float32Array(MAX_FIELD_GRID * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.65,
        });
        this._gravityVectors = new THREE.LineSegments(geo, mat);
        this._gravityVectors.frustumCulled = false; // dynamic geo — see _eFieldLines
        this._gravityVectors.visible = false;
        this.scene.add(this._gravityVectors);
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
        this.scene.add(this._fluxVolume);
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
                this.scene.remove(this._fluxVolume);
                this._fluxVolume.geometry.dispose();
                this._fluxVolume.material.dispose();
                this._fluxVolume = null;
            }
            this._buildFluxVolume(latticeSize);
            // _buildFluxVolume initialises visible=false; restore the user's current
            // showFlux state so the volume doesn't disappear after a size change.
            this._fluxVolume.visible = this.showFlux;
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

    toggleFluxVolume(on) {
        if (!this._fluxVolume) this._buildFluxVolume(this.latticeSize);
        this._fluxVolume.visible = on;
        this.showFlux = on;
        if (!on) this._fluxVolume.geometry.setDrawRange(0, 0);
    }

    toggleFluxSlice(on) {
        if (!this._fieldHeatmap) this._buildFieldHeatmap();
        this._fieldHeatmap.visible = on;
        this.showHeatmap = on;
        if (!on) this._fieldHeatmap.geometry.setDrawRange(0, 0);
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

    // ── E-Field Lines (Cyan) ─────────────────────────────────────────
    _buildEFieldLines() {
        // Sized for worst case (N=128 with continuous streamline scaling):
        // 300 lines × ~144 segments × 2 verts = ~86K. Round up for safety.
        const maxVerts = 300 * 160 * 2;
        const positions = new Float32Array(maxVerts * 3);
        const colors = new Float32Array(maxVerts * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.7,
            blending: THREE.AdditiveBlending, depthWrite: false
        });
        this._eFieldLines = new THREE.LineSegments(geo, mat);
        this._eFieldLines.visible = false;
        // Disable frustum culling — geometry is updated each frame without
        // recomputing the bounding sphere, so Three.js's stale-bounds test
        // would falsely cull when the camera zooms close to the lattice.
        this._eFieldLines.frustumCulled = false;
        this.scene.add(this._eFieldLines);
    }

    updateEFieldLines(streamlines) {
        if (!this._eFieldLines) this._buildEFieldLines();
        const posAttr = this._eFieldLines.geometry.getAttribute('position');
        const colAttr = this._eFieldLines.geometry.getAttribute('color');
        const maxVerts = posAttr.array.length / 3;
        const halfN = this._halfN;
        let vi = 0; // vertex index for LineSegments (pairs)

        for (const line of streamlines) {
            const nPts = line.length / 3;
            for (let i = 0; i < nPts - 1 && vi + 2 <= maxVerts; i++) {
                const sx = line[i * 3], sy = line[i * 3 + 1], sz = line[i * 3 + 2];
                if (!this._insideBoundary((sx - halfN) / halfN, (sy - halfN) / halfN, (sz - halfN) / halfN)) continue;
                const t = i / (nPts - 1); // fade along length
                const alpha = 1.0 - t * 0.7;
                // Cyan: (0.3, 0.82, 0.88) fading to dim
                const r = 0.3 * alpha, g = 0.82 * alpha, b = 0.88 * alpha;

                posAttr.array[vi * 3] = sx;
                posAttr.array[vi * 3 + 1] = sy;
                posAttr.array[vi * 3 + 2] = sz;
                colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                vi++;

                posAttr.array[vi * 3] = line[(i + 1) * 3];
                posAttr.array[vi * 3 + 1] = line[(i + 1) * 3 + 1];
                posAttr.array[vi * 3 + 2] = line[(i + 1) * 3 + 2];
                colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                vi++;
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._eFieldLines.geometry.setDrawRange(0, vi);
    }

    toggleEFieldLines(on) {
        if (!this._eFieldLines) this._buildEFieldLines();
        this._eFieldLines.visible = on;
        if (!on) this._eFieldLines.geometry.setDrawRange(0, 0);
    }

    // ── B-Field Lines (Green) ────────────────────────────────────────
    _buildBFieldLines() {
        // B lines integrate longer (closed loops, 1.5× E maxSteps).
        // Worst case: 300 lines × ~216 segments × 2 = ~130K. Round up.
        const maxVerts = 300 * 240 * 2;
        const positions = new Float32Array(maxVerts * 3);
        const colors = new Float32Array(maxVerts * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.7,
            blending: THREE.AdditiveBlending, depthWrite: false
        });
        this._bFieldLines = new THREE.LineSegments(geo, mat);
        this._bFieldLines.visible = false;
        this._bFieldLines.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.scene.add(this._bFieldLines);
    }

    updateBFieldLines(streamlines) {
        if (!this._bFieldLines) this._buildBFieldLines();
        const posAttr = this._bFieldLines.geometry.getAttribute('position');
        const colAttr = this._bFieldLines.geometry.getAttribute('color');
        const maxVerts = posAttr.array.length / 3;
        const halfN = this._halfN;
        let vi = 0;

        for (const line of streamlines) {
            const nPts = line.length / 3;
            for (let i = 0; i < nPts - 1 && vi + 2 <= maxVerts; i++) {
                const sx = line[i * 3], sy = line[i * 3 + 1], sz = line[i * 3 + 2];
                if (!this._insideBoundary((sx - halfN) / halfN, (sy - halfN) / halfN, (sz - halfN) / halfN)) continue;
                const t = i / (nPts - 1);
                const alpha = 1.0 - t * 0.5;
                // Green: (0.4, 0.73, 0.42)
                const r = 0.4 * alpha, g = 0.73 * alpha, b = 0.42 * alpha;

                posAttr.array[vi * 3] = sx;
                posAttr.array[vi * 3 + 1] = sy;
                posAttr.array[vi * 3 + 2] = sz;
                colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                vi++;

                posAttr.array[vi * 3] = line[(i + 1) * 3];
                posAttr.array[vi * 3 + 1] = line[(i + 1) * 3 + 1];
                posAttr.array[vi * 3 + 2] = line[(i + 1) * 3 + 2];
                colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                vi++;
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._bFieldLines.geometry.setDrawRange(0, vi);
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
    _buildFluxStreamlines() {
        // Sized to match E-field cap (same maxSteps profile, see field-overlays.js).
        const maxVerts = 300 * 160 * 2;
        const positions = new Float32Array(maxVerts * 3);
        const colors = new Float32Array(maxVerts * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.7,
            blending: THREE.AdditiveBlending, depthWrite: false
        });
        this._fluxStreamlines = new THREE.LineSegments(geo, mat);
        this._fluxStreamlines.visible = false;
        this._fluxStreamlines.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.scene.add(this._fluxStreamlines);
    }

    updateFluxStreamlines(streamlines, maxFluxMag) {
        if (!this._fluxStreamlines) this._buildFluxStreamlines();
        const posAttr = this._fluxStreamlines.geometry.getAttribute('position');
        const colAttr = this._fluxStreamlines.geometry.getAttribute('color');
        const maxVerts = posAttr.array.length / 3;
        const halfN = this._halfN;
        let vi = 0;

        for (const line of streamlines) {
            const nPts = line.length / 3;
            for (let i = 0; i < nPts - 1 && vi + 2 <= maxVerts; i++) {
                const sx = line[i * 3], sy = line[i * 3 + 1], sz = line[i * 3 + 2];
                if (!this._insideBoundary((sx - halfN) / halfN, (sy - halfN) / halfN, (sz - halfN) / halfN)) continue;
                // Use flux colormap
                const t = i / (nPts - 1);
                const [r, g, b] = fluxToColor(t * (maxFluxMag || 1), maxFluxMag || 1);

                posAttr.array[vi * 3] = sx;
                posAttr.array[vi * 3 + 1] = sy;
                posAttr.array[vi * 3 + 2] = sz;
                colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                vi++;

                posAttr.array[vi * 3] = line[(i + 1) * 3];
                posAttr.array[vi * 3 + 1] = line[(i + 1) * 3 + 1];
                posAttr.array[vi * 3 + 2] = line[(i + 1) * 3 + 2];
                colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                vi++;
            }
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._fluxStreamlines.geometry.setDrawRange(0, vi);
    }

    toggleFluxStreamlines(on) {
        if (!this._fluxStreamlines) this._buildFluxStreamlines();
        this._fluxStreamlines.visible = on;
        if (!on) this._fluxStreamlines.geometry.setDrawRange(0, 0);
    }

    // ── EM Force Volume (Cyan arrows — repurposed from generic Forces) ──
    _buildForceVolume() {
        const maxArrows = 8000;
        const positions = new Float32Array(maxArrows * 2 * 3);
        const colors = new Float32Array(maxArrows * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.6,
            depthWrite: false
        });
        this._forceVolume = new THREE.LineSegments(geo, mat);
        this._forceVolume.visible = false;
        this._forceVolume.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.scene.add(this._forceVolume);
    }

    updateForceVolume(fieldData) {
        if (!this._forceVolume) this._buildForceVolume();
        const posAttr = this._forceVolume.geometry.getAttribute('position');
        const colAttr = this._forceVolume.geometry.getAttribute('color');
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
        const threshold = maxMag * 0.03;
        const halfN = this._halfN;
        // Arrow length in world units. ~1.5 vox base × log(magnitude) — keeps
        // EM-force arrows local to the voxel they originate from, regardless
        // of lattice size, so adjacent arrows don't overlap and the field
        // direction reads correctly.
        const arrowBase = 1.5;
        let vi = 0;

        for (let i = 0; i < count && vi < maxArrows; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const vx = vectors[i * 3], vy = vectors[i * 3 + 1], vz = vectors[i * 3 + 2];

            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            const scale = Math.log(1 + mag / maxMag) * arrowBase;
            const nx = vx / mag * scale, ny = vy / mag * scale, nz = vz / mag * scale;

            // Base (cyan)
            posAttr.array[vi * 6] = px; posAttr.array[vi * 6 + 1] = py; posAttr.array[vi * 6 + 2] = pz;
            colAttr.array[vi * 6] = 0.0; colAttr.array[vi * 6 + 1] = 0.9; colAttr.array[vi * 6 + 2] = 1.0;
            // Tip (bright cyan)
            posAttr.array[vi * 6 + 3] = px + nx; posAttr.array[vi * 6 + 4] = py + ny; posAttr.array[vi * 6 + 5] = pz + nz;
            colAttr.array[vi * 6 + 3] = 0.7; colAttr.array[vi * 6 + 4] = 1.0; colAttr.array[vi * 6 + 5] = 1.0;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._forceVolume.geometry.setDrawRange(0, vi * 2);
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
    _buildStrongForce() {
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
        this._strongForce = new THREE.LineSegments(geo, mat);
        this._strongForce.visible = false;
        this._strongForce.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.scene.add(this._strongForce);
    }

    updateStrongForceField(fieldData) {
        if (!this._strongForce) this._buildStrongForce();
        const posAttr = this._strongForce.geometry.getAttribute('position');
        const colAttr = this._strongForce.geometry.getAttribute('color');
        const { positions, vectors, count } = fieldData;
        const maxArrows = posAttr.array.length / 6;
        let maxMag = 0;
        if (!this._strongMagCache || this._strongMagCache.length < count) this._strongMagCache = new Float32Array(count);
        const mags = this._strongMagCache;
        for (let i = 0; i < count; i++) {
            const a = vectors[i * 3], b = vectors[i * 3 + 1], c = vectors[i * 3 + 2];
            const m = Math.sqrt(a * a + b * b + c * c);
            mags[i] = m;
            if (m > maxMag) maxMag = m;
        }
        const threshold = maxMag * 0.03;
        const halfN = this._halfN;
        // Strong-force arrows: 1.5-vox world-space base, identical convention
        // to EM/gravity so the four force overlays render at the same scale.
        const arrowBase = 1.5;
        let vi = 0;

        for (let i = 0; i < count && vi < maxArrows; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const vx = vectors[i * 3], vy = vectors[i * 3 + 1], vz = vectors[i * 3 + 2];
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            const scale = Math.log(1 + mag / maxMag) * arrowBase;
            const nx = vx / mag * scale, ny = vy / mag * scale, nz = vz / mag * scale;

            // Base (red)
            posAttr.array[vi * 6] = px; posAttr.array[vi * 6 + 1] = py; posAttr.array[vi * 6 + 2] = pz;
            colAttr.array[vi * 6] = 1.0; colAttr.array[vi * 6 + 1] = 0.09; colAttr.array[vi * 6 + 2] = 0.27;
            // Tip (bright red)
            posAttr.array[vi * 6 + 3] = px + nx; posAttr.array[vi * 6 + 4] = py + ny; posAttr.array[vi * 6 + 5] = pz + nz;
            colAttr.array[vi * 6 + 3] = 1.0; colAttr.array[vi * 6 + 4] = 0.5; colAttr.array[vi * 6 + 5] = 0.5;
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._strongForce.geometry.setDrawRange(0, vi * 2);
    }

    toggleStrongForce(on) {
        if (!this._strongForce) this._buildStrongForce();
        this._strongForce.visible = on;
        if (!on) this._strongForce.geometry.setDrawRange(0, 0);
    }

    showStrongForce(on) { this.toggleStrongForce(on); }

    // ── Weak Force Overlay (Purple points at chirality sites) ─────────
    _buildWeakField() {
        const maxPts = 4000;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.PointsMaterial({
            size: 3.0, vertexColors: true, transparent: true, opacity: 0.7,
            depthWrite: false, sizeAttenuation: true
        });
        this._weakField = new THREE.Points(geo, mat);
        this._weakField.visible = false;
        this._weakField.frustumCulled = false; // dynamic geo — see _eFieldLines
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
        const threshold = maxVal * 0.1;
        const halfN = this._halfN;
        let vi = 0;
        for (let i = 0; i < count && vi < maxPts; i++) {
            if (Math.abs(values[i]) < threshold) continue;
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            posAttr.array[vi * 3] = px; posAttr.array[vi * 3 + 1] = py; posAttr.array[vi * 3 + 2] = pz;
            // Purple color
            colAttr.array[vi * 3] = 0.67; colAttr.array[vi * 3 + 1] = 0.0; colAttr.array[vi * 3 + 2] = 1.0;
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

    // Color palettes for each force type (low → mid → high)
    static FORCE_PALETTES = {
        em:      { low: [0.0, 0.2, 0.4], mid: [0.0, 0.9, 1.0], high: [0.7, 1.0, 1.0] },
        gravity: { low: [0.4, 0.2, 0.0], mid: [1.0, 0.67, 0.0], high: [1.0, 1.0, 0.6] },
        strong:  { low: [0.4, 0.0, 0.05], mid: [1.0, 0.09, 0.27], high: [1.0, 0.7, 0.7] },
        weak:    { low: [0.2, 0.0, 0.4], mid: [0.67, 0.0, 1.0], high: [0.9, 0.6, 1.0] },
    };

    /**
     * Interpolate a 3-stop color palette at parameter t in [0,1].
     * @param {object} pal - { low: [r,g,b], mid: [r,g,b], high: [r,g,b] }
     * @param {number} t   - 0..1
     * @returns {[number,number,number]}
     */
    static _lerpPalette(pal, t) {
        const tt = Math.max(0, Math.min(1, t));
        if (tt < 0.5) {
            const u = tt * 2;
            return [
                pal.low[0] + (pal.mid[0] - pal.low[0]) * u,
                pal.low[1] + (pal.mid[1] - pal.low[1]) * u,
                pal.low[2] + (pal.mid[2] - pal.low[2]) * u,
            ];
        }
        const u = (tt - 0.5) * 2;
        return [
            pal.mid[0] + (pal.high[0] - pal.mid[0]) * u,
            pal.mid[1] + (pal.high[1] - pal.mid[1]) * u,
            pal.mid[2] + (pal.high[2] - pal.mid[2]) * u,
        ];
    }

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
        const pal = Viewport.FORCE_PALETTES[forceType] || Viewport.FORCE_PALETTES.em;

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
            const [r, g, b] = Viewport._lerpPalette(pal, t);
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
        const pal = Viewport.FORCE_PALETTES[forceType] || Viewport.FORCE_PALETTES.em;
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
    _buildForceGlyphs() {
        const maxInstances = 2000;
        const coneGeo = new THREE.ConeGeometry(0.3, 1.0, 6);
        // Rotate cone so it points along +Y by default (lookAt will orient it)
        coneGeo.rotateX(Math.PI / 2);
        const mat = new THREE.MeshBasicMaterial({
            transparent: true,
            opacity: 0.7,
            depthWrite: false,
        });
        this._forceGlyphs = new THREE.InstancedMesh(coneGeo, mat, maxInstances);
        this._forceGlyphs.instanceMatrix.setUsage(THREE.DynamicDrawUsage);
        this._forceGlyphs.visible = false;
        this._forceGlyphs.frustumCulled = false;
        this._forceGlyphs.count = 0;
        // Enable per-instance color
        this._forceGlyphs.instanceColor = new THREE.InstancedBufferAttribute(
            new Float32Array(maxInstances * 3), 3
        );
        this._forceGlyphs.instanceColor.setUsage(THREE.DynamicDrawUsage);
        this.scene.add(this._forceGlyphs);
        // Reusable math objects
        this._glyphMatrix = new THREE.Matrix4();
        this._glyphQuat = new THREE.Quaternion();
        this._glyphUp = new THREE.Vector3(0, 0, 1); // cone default direction after rotateX
        this._glyphDir = new THREE.Vector3();
        this._glyphColor = new THREE.Color();
    }

    initForceGlyphs() { if (!this._forceGlyphs) this._buildForceGlyphs(); }

    updateForceGlyphs(fieldData, forceType) {
        if (!this._forceGlyphs) this._buildForceGlyphs();
        const { positions, vectors, count } = fieldData;
        const maxInstances = 2000;
        const pal = Viewport.FORCE_PALETTES[forceType] || Viewport.FORCE_PALETTES.em;

        // Compute magnitudes
        let maxMag = 0;
        if (!this._glyphMagCache || this._glyphMagCache.length < count) {
            this._glyphMagCache = new Float32Array(count);
        }
        const mags = this._glyphMagCache;
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
        const col = this._glyphColor;
        const colorArr = this._forceGlyphs.instanceColor.array;

        for (let i = 0; i < count && vi < maxInstances; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;

            const t = mag / maxMag;
            const scale = Math.log(1 + t * 9) / Math.log(10) * scaleBase;

            // Direction quaternion
            dir.set(vectors[i * 3] / mag, vectors[i * 3 + 1] / mag, vectors[i * 3 + 2] / mag);
            quat.setFromUnitVectors(up, dir);

            // Build matrix: translation * rotation * scale
            mat4.makeRotationFromQuaternion(quat);
            mat4.scale(new THREE.Vector3(scale, scale, scale * 1.5));
            mat4.setPosition(px, py, pz);
            this._forceGlyphs.setMatrixAt(vi, mat4);

            // Color
            const [r, g, b] = Viewport._lerpPalette(pal, t);
            colorArr[vi * 3]     = r;
            colorArr[vi * 3 + 1] = g;
            colorArr[vi * 3 + 2] = b;
            vi++;
        }

        this._forceGlyphs.count = vi;
        this._forceGlyphs.instanceMatrix.needsUpdate = true;
        this._forceGlyphs.instanceColor.needsUpdate = true;
    }

    showForceGlyphs(visible) {
        if (!this._forceGlyphs) this._buildForceGlyphs();
        this._forceGlyphs.visible = visible;
        if (!visible) this._forceGlyphs.count = 0;
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

        // 12 edges of a unit cube, scaled to 3x3x3 around particle
        // Particles use raw lattice coordinates (not centered), matching updateParticles()
        const edges = [
            [0, 0, 0, 1, 0, 0], [0, 1, 0, 1, 1, 0], [0, 0, 1, 1, 0, 1], [0, 1, 1, 1, 1, 1],
            [0, 0, 0, 0, 1, 0], [1, 0, 0, 1, 1, 0], [0, 0, 1, 0, 1, 1], [1, 0, 1, 1, 1, 1],
            [0, 0, 0, 0, 0, 1], [1, 0, 0, 1, 0, 1], [0, 1, 0, 0, 1, 1], [1, 1, 0, 1, 1, 1],
        ];

        for (const p of particles) {
            if (si >= 1200) break;
            const cx = p.x, cy = p.y, cz = p.z;
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

        // This utilizes getParticleData from the active engine
        const ptData = bridge.getParticleData();
        if (!ptData || ptData.count < 2) {
            this._confinementStrings.geometry.setDrawRange(0, 0);
            return;
        }

        // O(N^2) evaluation for topological deformation between manifest states
        for (let i = 0; i < ptData.count; i++) {
            if (ptData.states[i * 4 + 3] === 0) continue; // Only process active manifest nodes
            for (let j = i + 1; j < ptData.count; j++) {
                if (ptData.states[j * 4 + 3] === 0) continue;

                const dx = ptData.states[j * 4] - ptData.states[i * 4];
                const dy = ptData.states[j * 4 + 1] - ptData.states[i * 4 + 1];
                const dz = ptData.states[j * 4 + 2] - ptData.states[i * 4 + 2];
                const r2 = dx * dx + dy * dy + dz * dz;

                // If they are separated but before the snap point (hadronization)
                if (r2 > 1.0 && r2 < J2_threshold_dist2) {
                    const t = r2 / J2_threshold_dist2;
                    const alpha = 1.0 - t * 0.4;
                    // Color axis simulation (Mapping spatial differentiation to RGB SU(3) proxies)
                    const r = (Math.abs(dx) / Math.sqrt(r2)) * alpha + 0.2;
                    const g = (Math.abs(dy) / Math.sqrt(r2)) * alpha + 0.2;
                    const b = (Math.abs(dz) / Math.sqrt(r2)) * alpha + 0.2;

                    if (vi + 2 > maxVerts) break;

                    posAttr.array[vi * 3] = ptData.states[i * 4];
                    posAttr.array[vi * 3 + 1] = ptData.states[i * 4 + 1];
                    posAttr.array[vi * 3 + 2] = ptData.states[i * 4 + 2];
                    colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                    vi++;

                    posAttr.array[vi * 3] = ptData.states[j * 4];
                    posAttr.array[vi * 3 + 1] = ptData.states[j * 4 + 1];
                    posAttr.array[vi * 3 + 2] = ptData.states[j * 4 + 2];
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

    // ── Color ramps (CPU-side) ──
    // Each ramp takes a normalized input and writes (r,g,b) into a 3-element dest.
    _rampViridis(t, out, i) {
        // Approximate viridis: purple → teal → yellow
        t = Math.max(0, Math.min(1, t));
        if (t < 0.5) {
            const u = t * 2;
            out[i]     = 0.267 * (1 - u) + 0.13  * u;
            out[i + 1] = 0.004 * (1 - u) + 0.566 * u;
            out[i + 2] = 0.329 * (1 - u) + 0.551 * u;
        } else {
            const u = (t - 0.5) * 2;
            out[i]     = 0.13  * (1 - u) + 0.993 * u;
            out[i + 1] = 0.566 * (1 - u) + 0.906 * u;
            out[i + 2] = 0.551 * (1 - u) + 0.144 * u;
        }
    }

    _rampCyclicHSL(phase, out, i) {
        // Phase ∈ [0, π/2] for atan2(|J_R|,|J_L|) — map full hue cycle
        const hue = (phase / (Math.PI / 2)) % 1;
        // HSL → RGB (S=1, L=0.5)
        const h6 = hue * 6;
        const c = 1;  // saturation * (1 - |2L-1|) = 1 * 1 = 1
        const x = c * (1 - Math.abs((h6 % 2) - 1));
        let r, g, b;
        if (h6 < 1)      { r = c; g = x; b = 0; }
        else if (h6 < 2) { r = x; g = c; b = 0; }
        else if (h6 < 3) { r = 0; g = c; b = x; }
        else if (h6 < 4) { r = 0; g = x; b = c; }
        else if (h6 < 5) { r = x; g = 0; b = c; }
        else             { r = c; g = 0; b = x; }
        // L=0.5 means we just output (r,g,b) directly
        out[i] = r; out[i + 1] = g; out[i + 2] = b;
    }

    _rampDivergingRdBu(t, out, i) {
        // t ∈ [-1, 1]; negative=blue, zero=white, positive=red
        t = Math.max(-1, Math.min(1, t));
        if (t >= 0) {
            const u = t;
            out[i]     = 0.969 * (1 - u) + 0.698 * u;
            out[i + 1] = 0.969 * (1 - u) + 0.094 * u;
            out[i + 2] = 0.969 * (1 - u) + 0.169 * u;
        } else {
            const u = -t;
            out[i]     = 0.969 * (1 - u) + 0.129 * u;
            out[i + 1] = 0.969 * (1 - u) + 0.400 * u;
            out[i + 2] = 0.969 * (1 - u) + 0.675 * u;
        }
    }

    _rampGrayscale(t, out, i) {
        t = Math.max(0, Math.min(1, t));
        out[i] = t; out[i + 1] = t; out[i + 2] = t;
    }

    _rampGravWell(t, out, i) {
        // t ∈ [0, 1] — deeper well (higher t) = deep blue; peak = yellow
        t = Math.max(0, Math.min(1, t));
        if (t > 0.5) {
            const u = (t - 0.5) * 2;
            out[i]     = 0.0 + 0.0   * u;
            out[i + 1] = 0.4 * (1 - u);
            out[i + 2] = 0.8 * (1 - u) + 0.2 * u;
        } else {
            const u = t * 2;
            out[i]     = 1.0 * (1 - u) + 0.0 * u;
            out[i + 1] = 1.0 * (1 - u) + 0.4 * u;
            out[i + 2] = 0.0 * (1 - u) + 0.8 * u;
        }
    }

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
            ramp: (t, out, i) => this._rampViridis(t, out, i),
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
            this._rampCyclicHSL(phase, rgb, 0);
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
            ramp: (t, out, i) => this._rampDivergingRdBu(t, out, i),
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
            this._rampGrayscale(s, colAttr.array, vi * 3);
            vi++;
        }
        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._quantumField.geometry.setDrawRange(0, vi);
        this._quantumFieldKind = 'entropy';
    }

    // ══════════════════════════════════════════════════════════════════
    // ── Φ(x) potential — rubber-sheet landscape on the XZ floor ───────
    // A potential is a HEIGHT FIELD. Canonical QM/GR visualization is a
    // deformable surface where wells dip down and peaks rise up. Rendered
    // as a subdivided plane at y=0 with Y-displacement = −Φ.

    _buildGravSurface() {
        const N = this._latticeSize || 32;
        this._gravSurfaceSize = N;
        const segments = Math.max(16, Math.min(N, 48));  // cap subdivision for perf
        // Plane spans the lattice footprint (slightly inset). PlaneGeometry
        // is centered at (0,0) so we translate to (halfN, halfN-reference, halfN)
        // to align with lattice coords which run [0, N].
        const geo = new THREE.PlaneGeometry(N * 0.95, N * 0.95, segments, segments);
        geo.rotateX(-Math.PI / 2);  // lie flat on XZ plane, normal = +Y
        const colors = new Float32Array(geo.attributes.position.count * 3);
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        const mat = new THREE.MeshBasicMaterial({
            vertexColors: true,
            transparent: true,
            opacity: 0.55,
            side: THREE.DoubleSide,
            wireframe: false,
            depthWrite: false,
        });
        const wireMat = new THREE.MeshBasicMaterial({
            vertexColors: true,
            transparent: true,
            opacity: 0.35,
            wireframe: true,
            depthWrite: false,
        });
        this._gravSurface = new THREE.Mesh(geo, mat);
        // Center the plane on the lattice horizontally (X,Z) and anchor its
        // reference height at the lattice midplane (y = halfN). Wells dip
        // below midplane, peaks rise above — both stay within the lattice.
        this._gravSurface.position.set(N / 2, N / 2, N / 2);
        this._gravSurface.visible = false;
        this._gravSurface.renderOrder = 3;
        // Plane mesh has dynamic Y-displacement on its vertices; turn off
        // frustum culling so the deformed surface doesn't disappear when the
        // camera zooms inside it (well-shaped surface can extend below the
        // original bounding box).
        this._gravSurface.frustumCulled = false;
        this._gravSurfaceWire = new THREE.Mesh(geo, wireMat);
        this._gravSurfaceWire.position.set(N / 2, N / 2 + 0.02, N / 2);
        this._gravSurfaceWire.visible = false;
        this._gravSurfaceWire.renderOrder = 3;
        this._gravSurfaceWire.frustumCulled = false;
        this.scene.add(this._gravSurface);
        this.scene.add(this._gravSurfaceWire);
    }

    _rebuildGravSurfaceIfResized() {
        // Called from updateGravPotentialField when the lattice size has changed.
        // Avoid leaking the old mesh by disposing geometry + removing from scene.
        if (!this._gravSurface) return;
        const N = this._latticeSize || 32;
        if (this._gravSurfaceSize === N) return;
        this._gravSurface.geometry?.dispose();
        this._gravSurfaceWire.geometry?.dispose();
        this.scene.remove(this._gravSurface);
        this.scene.remove(this._gravSurfaceWire);
        this._gravSurface = null;
        this._gravSurfaceWire = null;
        this._buildGravSurface();
        this._gravSurface.visible = this._gravPotVisible;
        this._gravSurfaceWire.visible = this._gravPotVisible;
    }

    toggleGravPotentialField(on) {
        this._gravPotVisible = !!on;
        if (!this._gravSurface) this._buildGravSurface();
        this._gravSurface.visible = !!on;
        this._gravSurfaceWire.visible = !!on;
        this._quantumSetVisibility();
    }

    updateGravPotentialField(data) {
        this._gravPotData = data;
        if (!this._gravPotVisible || !data?.count) return;
        if (!this._gravSurface) this._buildGravSurface();
        this._rebuildGravSurfaceIfResized();
        const geo = this._gravSurface.geometry;
        const pos = geo.attributes.position;
        const col = geo.attributes.color;
        const verts = pos.count;
        const N = this._latticeSize || 32;
        const halfN = this._halfN;
        // Vertex positions in geometry-local space are centered at (0,0,0)
        // (PlaneGeometry default), so we convert sample positions to local
        // by subtracting halfN on X and Z. The Y coordinate in local space
        // IS the displacement — that's what we're solving for.
        const { positions, values, count, normalizer } = data;
        const denom = Math.max(normalizer, 1e-9);
        const DEPTH = N * 0.25;  // max vertical displacement (quarter lattice)
        const rgb = new Float32Array(3);
        for (let v = 0; v < verts; v++) {
            const vx = pos.array[v * 3];        // local X, centered on 0
            const vz = pos.array[v * 3 + 2];    // local Z, centered on 0
            // Find the sample closest to this vertex on the XZ plane, lightly
            // weighted by y-distance so we prefer samples near the midplane.
            let bestD = Infinity, bestVal = 0;
            for (let i = 0; i < count; i++) {
                const sxLocal = positions[i * 3]     - halfN;
                const syLocal = positions[i * 3 + 1] - halfN;
                const szLocal = positions[i * 3 + 2] - halfN;
                const d = (sxLocal - vx) * (sxLocal - vx)
                        + (szLocal - vz) * (szLocal - vz)
                        + Math.abs(syLocal) * 2;
                if (d < bestD) { bestD = d; bestVal = values[i]; }
            }
            // Φ is negative for wells (computed as −|J|² proxy). Scaling by
            // DEPTH gives a proportional dip; negative t → vertex dips below
            // the reference plane, which is at local-y = 0 (world y = halfN).
            const t = bestVal / denom;
            pos.array[v * 3 + 1] = t * DEPTH;
            // Color by |t|: deeper well OR higher peak → warmer color.
            this._rampGravWell(Math.min(1, Math.abs(t)), rgb, 0);
            col.array[v * 3]     = rgb[0];
            col.array[v * 3 + 1] = rgb[1];
            col.array[v * 3 + 2] = rgb[2];
        }
        pos.needsUpdate = true;
        col.needsUpdate = true;
        geo.computeVertexNormals();
    }

    // ══════════════════════════════════════════════════════════════════
    // ── |ψ|² breathing animation ──────────────────────────────────────
    // Called from the main render loop so the probability cloud "breathes"
    // at ~0.3Hz — conveys that this is a DYNAMIC quantum state, not a
    // static heatmap. No-op when |ψ|² isn't the active field.
    _animateQuantumField(now) {
        if (!this._quantumField || !this._psi2Visible) return;
        if (this._quantumFieldKind !== 'psi2') return;
        const phase = (now / 1000) * Math.PI * 0.6;  // ~0.3Hz pulse
        const pulse = 0.85 + 0.15 * Math.sin(phase);
        this._quantumField.material.opacity = pulse;
    }

    // ══════════════════════════════════════════════════════════════════
    // ── Nucleus Shells (strong force glow spheres) ─────────────────

    _buildNucleusShells() {
        const maxShells = 100;
        const geo = new THREE.SphereGeometry(1, 16, 12);
        const mat = new THREE.MeshBasicMaterial({
            color: 0xff6633, transparent: true, opacity: 0.12,
            blending: THREE.AdditiveBlending, depthWrite: false, side: THREE.DoubleSide,
        });
        this._nucleusShells = new THREE.InstancedMesh(geo, mat, maxShells);
        this._nucleusShells.count = 0;
        this._nucleusShells.visible = true;
        this._nucleusShells.renderOrder = -2;
        this.scene.add(this._nucleusShells);
    }

    updateNucleusShells(atomData) {
        if (!this._nucleusShells) this._buildNucleusShells();
        if (!atomData || atomData.count === 0) { this._nucleusShells.count = 0; return; }
        const n = Math.min(atomData.count, 100);
        const mat4 = new THREE.Matrix4();
        for (let i = 0; i < n; i++) {
            const Z = atomData.atomicNums[i];
            const N_neutrons = this._defaultNeutronCount ? this._defaultNeutronCount(Z) : Math.round(Z * 1.2);
            const A = Z + N_neutrons;
            const radius = 0.5 * Math.cbrt(Math.max(A, 1)) * 1.8;
            mat4.makeScale(radius, radius, radius);
            mat4.setPosition(atomData.positions[i * 3], atomData.positions[i * 3 + 1], atomData.positions[i * 3 + 2]);
            this._nucleusShells.setMatrixAt(i, mat4);
        }
        this._nucleusShells.count = n;
        this._nucleusShells.instanceMatrix.needsUpdate = true;
    }

    toggleNucleusShells(on) {
        if (!this._nucleusShells) this._buildNucleusShells();
        this._nucleusShells.visible = on;
    }

    // ── Bond Cylinders (thick styled bonds) ─────────────────────────

    _buildBondCylinders() {
        const maxInstances = 1500;
        const geo = new THREE.CylinderGeometry(1, 1, 1, 8);
        geo.translate(0, 0.5, 0); // pivot at base so scaling works from one end
        const mat = new THREE.MeshLambertMaterial({
            color: 0xffffff, transparent: true, opacity: 0.85,
        });
        this._bondCylinders = new THREE.InstancedMesh(geo, mat, maxInstances);
        this._bondCylinders.count = 0;
        this._bondCylinders.visible = true;
        this.scene.add(this._bondCylinders);

        // Add directional light for bond shading (only active in atoms/molecules)
        this._bondLight = new THREE.DirectionalLight(0xffffff, 0.4);
        this._bondLight.position.set(10, 20, 10);
        this._bondLight.visible = true;
        this.scene.add(this._bondLight);
    }

    // Renders covalent bonds as oriented cylinders. Single/double/triple bonds
    // use 1/2/3 parallel cylinders respectively. Each bond creates new Vector3
    // temporaries -- acceptable because atom counts are typically <200.
    updateBondCylinders(atomData) {
        if (!this._bondCylinders) this._buildBondCylinders();
        if (!atomData || atomData.bondCount === 0) { this._bondCylinders.count = 0; return; }

        // Build id→index lookup
        const idToIdx = new Map();
        for (let i = 0; i < atomData.count; i++) idToIdx.set(atomData.ids[i], i);

        const mat4 = new THREE.Matrix4();
        const up = new THREE.Vector3(0, 1, 0);
        const dir = new THREE.Vector3();
        const quat = new THREE.Quaternion();
        const color = new THREE.Color();
        let instIdx = 0;

        for (let b = 0; b < atomData.bondCount && instIdx < 1500; b++) {
            const idA = atomData.bonds[b * 2];
            const idB = atomData.bonds[b * 2 + 1];
            const iA = idToIdx.get(idA), iB = idToIdx.get(idB);
            if (iA === undefined || iB === undefined) continue;

            const ax = atomData.positions[iA * 3], ay = atomData.positions[iA * 3 + 1], az = atomData.positions[iA * 3 + 2];
            const bx = atomData.positions[iB * 3], by = atomData.positions[iB * 3 + 1], bz = atomData.positions[iB * 3 + 2];
            const dx = bx - ax, dy = by - ay, dz = bz - az;
            const bondLen = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (bondLen < 1e-10) continue;

            dir.set(dx, dy, dz).normalize();
            quat.setFromUnitVectors(up, dir);

            // Color: blend CPK colors of bonded atoms
            const cA = new THREE.Color(atomData.colors[iA * 3], atomData.colors[iA * 3 + 1], atomData.colors[iA * 3 + 2]);
            const cB = new THREE.Color(atomData.colors[iB * 3], atomData.colors[iB * 3 + 1], atomData.colors[iB * 3 + 2]);
            color.copy(cA).lerp(cB, 0.5);

            const order = atomData.bondOrders ? atomData.bondOrders[b] : 1;

            if (order === 1) {
                // Single bond: 1 cylinder, radius 0.15
                mat4.compose(new THREE.Vector3(ax, ay, az), quat, new THREE.Vector3(0.15, bondLen, 0.15));
                this._bondCylinders.setMatrixAt(instIdx, mat4);
                this._bondCylinders.setColorAt(instIdx, color);
                instIdx++;
            } else if (order === 2) {
                // Double bond: 2 parallel cylinders offset ±0.18
                const perp = new THREE.Vector3().crossVectors(dir, new THREE.Vector3(0, 0, 1));
                if (perp.lengthSq() < 0.001) perp.crossVectors(dir, new THREE.Vector3(1, 0, 0));
                perp.normalize().multiplyScalar(0.18);
                for (let s = -1; s <= 1; s += 2) {
                    const ox = ax + perp.x * s, oy = ay + perp.y * s, oz = az + perp.z * s;
                    mat4.compose(new THREE.Vector3(ox, oy, oz), quat, new THREE.Vector3(0.12, bondLen, 0.12));
                    if (instIdx < 1500) {
                        this._bondCylinders.setMatrixAt(instIdx, mat4);
                        this._bondCylinders.setColorAt(instIdx, color);
                        instIdx++;
                    }
                }
            } else if (order >= 3) {
                // Triple bond: 3 cylinders in triangle arrangement
                const perp = new THREE.Vector3().crossVectors(dir, new THREE.Vector3(0, 0, 1));
                if (perp.lengthSq() < 0.001) perp.crossVectors(dir, new THREE.Vector3(1, 0, 0));
                perp.normalize();
                const perp2 = new THREE.Vector3().crossVectors(dir, perp).normalize();
                const offsets = [
                    [0, 0], // center
                    [perp.x * 0.2 + perp2.x * 0.12, perp.y * 0.2 + perp2.y * 0.12],
                    [-perp.x * 0.2 + perp2.x * 0.12, -perp.y * 0.2 + perp2.y * 0.12],
                ];
                const angles = [0, 2 * Math.PI / 3, 4 * Math.PI / 3];
                for (const angle of angles) {
                    const offX = Math.cos(angle) * 0.2, offY = Math.sin(angle) * 0.2;
                    const ox = ax + perp.x * offX + perp2.x * offY;
                    const oy = ay + perp.y * offX + perp2.y * offY;
                    const oz = az + perp.z * offX + perp2.z * offY;
                    mat4.compose(new THREE.Vector3(ox, oy, oz), quat, new THREE.Vector3(0.10, bondLen, 0.10));
                    if (instIdx < 1500) {
                        this._bondCylinders.setMatrixAt(instIdx, mat4);
                        this._bondCylinders.setColorAt(instIdx, color);
                        instIdx++;
                    }
                }
            }
        }

        this._bondCylinders.count = instIdx;
        this._bondCylinders.instanceMatrix.needsUpdate = true;
        if (this._bondCylinders.instanceColor) this._bondCylinders.instanceColor.needsUpdate = true;
    }

    toggleBondCylinders(on) {
        if (!this._bondCylinders) this._buildBondCylinders();
        this._bondCylinders.visible = on;
        if (this._bondLight) this._bondLight.visible = on;
    }

    // ── Orbital Shell Boundaries (translucent spheres per n) ────────

    _buildOrbitalShells() {
        const maxShells = 200;
        const geo = new THREE.SphereGeometry(1, 24, 16);
        const mat = new THREE.MeshBasicMaterial({
            color: 0x66bfff, transparent: true, opacity: 0.05,
            depthWrite: false, side: THREE.DoubleSide,
        });
        this._orbitalShells = new THREE.InstancedMesh(geo, mat, maxShells);
        this._orbitalShells.count = 0;
        this._orbitalShells.visible = false; // default OFF
        this._orbitalShells.renderOrder = -3;
        this.scene.add(this._orbitalShells);
    }

    updateOrbitalShells(atomData, electronConfigFn, slaterZeffFn, a0Display) {
        if (!this._orbitalShells) this._buildOrbitalShells();
        if (!atomData || atomData.count === 0 || !electronConfigFn) {
            this._orbitalShells.count = 0;
            return;
        }

        const mat4 = new THREE.Matrix4();
        const shellColors = {
            1: new THREE.Color(0x66bfff),  // blue
            2: new THREE.Color(0x4de673),  // green
            3: new THREE.Color(0xffb333),  // orange
            4: new THREE.Color(0xd94db3),  // pink
        };
        const shellOpacities = { 1: 0.06, 2: 0.04, 3: 0.03, 4: 0.02 };
        let instIdx = 0;

        for (let i = 0; i < atomData.count && instIdx < 200; i++) {
            const Z = atomData.atomicNums[i];
            const config = electronConfigFn(Z);
            const seenN = new Set();
            for (const sub of config) {
                if (seenN.has(sub.n)) continue;
                seenN.add(sub.n);
                const zEff = slaterZeffFn(Z, sub.n, sub.l);
                const radius = (sub.n * sub.n / zEff) * a0Display;
                const cx = atomData.positions[i * 3];
                const cy = atomData.positions[i * 3 + 1];
                const cz = atomData.positions[i * 3 + 2];

                mat4.makeScale(radius, radius, radius);
                mat4.setPosition(cx, cy, cz);
                this._orbitalShells.setMatrixAt(instIdx, mat4);

                const col = shellColors[Math.min(sub.n, 4)] || shellColors[4];
                this._orbitalShells.setColorAt(instIdx, col);
                instIdx++;
                if (instIdx >= 200) break;
            }
        }

        this._orbitalShells.count = instIdx;
        this._orbitalShells.instanceMatrix.needsUpdate = true;
        if (this._orbitalShells.instanceColor) this._orbitalShells.instanceColor.needsUpdate = true;
    }

    toggleOrbitalShells(on) {
        if (!this._orbitalShells) this._buildOrbitalShells();
        this._orbitalShells.visible = on;
    }

    // ── Orbital Lobes (p/d/f shaped meshes) ─────────────────────────

    _buildOrbitalLobes() {
        const maxLobes = 2000;
        // Elongated ellipsoid for p-orbital lobe shape
        const baseSphere = new THREE.SphereGeometry(1, 12, 8);
        const pos = baseSphere.attributes.position;
        for (let i = 0; i < pos.count; i++) {
            const x = pos.getX(i), y = pos.getY(i), z = pos.getZ(i);
            pos.setXYZ(i, x * 0.5, y * 1.6, z * 0.5); // elongated along Y
        }
        pos.needsUpdate = true;
        baseSphere.computeVertexNormals();

        const mat = new THREE.MeshBasicMaterial({
            color: 0xffffff, transparent: true, opacity: 0.08,
            depthWrite: false, side: THREE.DoubleSide,
            blending: THREE.AdditiveBlending,
        });
        this._orbitalLobes = new THREE.InstancedMesh(baseSphere, mat, maxLobes);
        this._orbitalLobes.count = 0;
        this._orbitalLobes.visible = false; // default OFF
        this._orbitalLobes.renderOrder = -4;
        this.scene.add(this._orbitalLobes);
    }

    updateOrbitalLobes(atomData, electronConfigFn, slaterZeffFn, a0Display) {
        if (!this._orbitalLobes) this._buildOrbitalLobes();
        if (!atomData || atomData.count === 0 || !electronConfigFn) {
            this._orbitalLobes.count = 0;
            return;
        }

        const mat4 = new THREE.Matrix4();
        const lobeColors = {
            1: new THREE.Color(0x30ee55), // p — green
            2: new THREE.Color(0xffaa22), // d — gold
            3: new THREE.Color(0xdd44bb), // f — magenta
        };
        let instIdx = 0;

        for (let i = 0; i < atomData.count && instIdx < 2000; i++) {
            const Z = atomData.atomicNums[i];
            const config = electronConfigFn(Z);
            const maxN = Math.max(...config.map(s => s.n));
            const cx = atomData.positions[i * 3];
            const cy = atomData.positions[i * 3 + 1];
            const cz = atomData.positions[i * 3 + 2];

            // Only show lobes for valence shell (outermost occupied orbitals)
            for (const sub of config) {
                if (sub.l === 0) continue; // s-orbitals are spherical (no lobes)
                const isValence = (sub.n === maxN) || (sub.n === maxN - 1 && sub.l >= 2);
                if (!isValence) continue;

                const zEff = slaterZeffFn(Z, sub.n, sub.l);
                const radius = (sub.n * sub.n / zEff) * a0Display * 0.6;
                const col = lobeColors[sub.l] || lobeColors[3];

                // Generate lobe orientations based on l
                const axes = this._getLobeAxes(sub.l);
                for (const axis of axes) {
                    if (instIdx >= 2000) break;
                    // Place lobe: scale by radius, rotate to axis orientation, translate to atom
                    const quat = new THREE.Quaternion();
                    const up = new THREE.Vector3(0, 1, 0);
                    const target = new THREE.Vector3(axis[0], axis[1], axis[2]);
                    quat.setFromUnitVectors(up, target.normalize());

                    mat4.compose(
                        new THREE.Vector3(cx, cy, cz),
                        quat,
                        new THREE.Vector3(radius * 0.5, radius, radius * 0.5)
                    );
                    this._orbitalLobes.setMatrixAt(instIdx, mat4);
                    this._orbitalLobes.setColorAt(instIdx, col);
                    instIdx++;

                    // Mirror lobe (opposite direction)
                    if (instIdx >= 2000) break;
                    target.negate();
                    quat.setFromUnitVectors(up, target.normalize());
                    mat4.compose(
                        new THREE.Vector3(cx, cy, cz),
                        quat,
                        new THREE.Vector3(radius * 0.5, radius, radius * 0.5)
                    );
                    this._orbitalLobes.setMatrixAt(instIdx, mat4);
                    this._orbitalLobes.setColorAt(instIdx, col);
                    instIdx++;
                }
            }
        }

        this._orbitalLobes.count = instIdx;
        this._orbitalLobes.instanceMatrix.needsUpdate = true;
        if (this._orbitalLobes.instanceColor) this._orbitalLobes.instanceColor.needsUpdate = true;
    }

    _getLobeAxes(l) {
        if (l === 1) {
            // p-orbitals: px, py, pz
            return [[1, 0, 0], [0, 1, 0], [0, 0, 1]];
        } else if (l === 2) {
            // d-orbitals: dz², dxz, dyz, dx²-y², dxy (simplified to 4 main axes)
            return [[1, 0, 0], [0, 1, 0], [0, 0, 1], [0.707, 0.707, 0]];
        } else {
            // f-orbitals: 6 axes for symmetry
            return [[1, 0, 0], [0, 1, 0], [0, 0, 1], [0.707, 0.707, 0], [0.707, 0, 0.707], [0, 0.707, 0.707]];
        }
    }

    toggleOrbitalLobes(on) {
        if (!this._orbitalLobes) this._buildOrbitalLobes();
        this._orbitalLobes.visible = on;
    }

    // ── Per-Atom Force Arrows ───────────────────────────────────────

    _buildAEForceArrows() {
        const maxAtoms = 200;
        const createArrowSet = (color) => {
            const vertices = new Float32Array(maxAtoms * 6); // 2 verts per atom
            const geo = new THREE.BufferGeometry();
            geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
            geo.setDrawRange(0, 0);
            const mat = new THREE.LineBasicMaterial({ color, linewidth: 2, transparent: true, opacity: 0.8 });
            const lines = new THREE.LineSegments(geo, mat);
            lines.visible = false;
            this.scene.add(lines);
            return lines;
        };

        this._aeForceIonic = createArrowSet(0xff4444); // red for Coulomb
        this._aeForceVdw = createArrowSet(0x44ff44); // green for vdW
        this._aeForceBond = createArrowSet(0xff8844); // orange for bond
        this._aeForceNet = createArrowSet(0xffffff); // white for net
    }

    updateAEForces(positions, forceData, count) {
        if (!this._aeForceIonic) this._buildAEForceArrows();
        if (!forceData || count === 0) {
            [this._aeForceIonic, this._aeForceVdw, this._aeForceBond, this._aeForceNet].forEach(l => l.geometry.setDrawRange(0, 0));
            return;
        }

        const scale = 8.0; // visual scale factor for force arrows
        const n = Math.min(count, 200);

        const updateArrows = (lines, forceArr) => {
            const posAttr = lines.geometry.getAttribute('position');
            for (let i = 0; i < n; i++) {
                const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
                const fx = forceArr[i * 3], fy = forceArr[i * 3 + 1], fz = forceArr[i * 3 + 2];

                // Log-compress force magnitude for visibility
                const fmag = Math.sqrt(fx * fx + fy * fy + fz * fz);
                const logScale = fmag > 1e-10 ? scale * Math.log1p(fmag) / fmag : 0;

                posAttr.array[i * 6] = px;
                posAttr.array[i * 6 + 1] = py;
                posAttr.array[i * 6 + 2] = pz;
                posAttr.array[i * 6 + 3] = px + fx * logScale;
                posAttr.array[i * 6 + 4] = py + fy * logScale;
                posAttr.array[i * 6 + 5] = pz + fz * logScale;
            }
            posAttr.needsUpdate = true;
            lines.geometry.setDrawRange(0, n * 2);
        };

        updateArrows(this._aeForceIonic, forceData.ionic);
        updateArrows(this._aeForceVdw, forceData.vdw);
        updateArrows(this._aeForceBond, forceData.bond);
        updateArrows(this._aeForceNet, forceData.net);
    }

    toggleAEForceIonic(on) { if (!this._aeForceIonic) this._buildAEForceArrows(); this._aeForceIonic.visible = on; }
    toggleAEForceVdw(on) { if (!this._aeForceVdw) this._buildAEForceArrows(); this._aeForceVdw.visible = on; }
    toggleAEForceBond(on) { if (!this._aeForceBond) this._buildAEForceArrows(); this._aeForceBond.visible = on; }
    toggleAEForceNet(on) { if (!this._aeForceNet) this._buildAEForceArrows(); this._aeForceNet.visible = on; }

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
            if (this.bondLines) this.bondLines.visible = false;
            if (this._bondCylinders) this._bondCylinders.visible = false;
            if (this._bondLight) this._bondLight.visible = false;
            if (this._nucleusShells) this._nucleusShells.visible = false;
            if (this._orbitalShells) this._orbitalShells.visible = false;
            if (this._orbitalLobes) this._orbitalLobes.visible = false;
            if (this._elementLabels) this._elementLabels.visible = false;
            if (this._aeForceIonic) this._aeForceIonic.visible = false;
            if (this._aeForceVdw) this._aeForceVdw.visible = false;
            if (this._aeForceBond) this._aeForceBond.visible = false;
            if (this._aeForceNet) this._aeForceNet.visible = false;
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
            if (this._forceGlyphs) this._forceGlyphs.visible = false;
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
            if (this._bondCylinders) this._bondCylinders.visible = isAtomMol;
            if (this._bondLight) this._bondLight.visible = isAtomMol;
            if (this._nucleusShells) this._nucleusShells.visible = isAtomMol;
            // Element Labels (e.g. H H) only valid in Atoms/Molecules scale
            if (this._elementLabels) this._elementLabels.visible = isAtomMol;
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

    // ── Element Labels (Scale 2 — Atom mode) ──────────────────────────
    // Sprite-based text labels that always face the camera.
    // Each label is a canvas-textured sprite positioned at the atom center.
    _makeTextSprite(text, color = '#ffffff', fontSize = 48) {
        const canvas = document.createElement('canvas');
        canvas.width = 128;
        canvas.height = 64;
        const ctx = canvas.getContext('2d');
        ctx.font = `bold ${fontSize}px 'Inter', 'Segoe UI', sans-serif`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        // Outline for readability
        ctx.strokeStyle = '#000000';
        ctx.lineWidth = 4;
        ctx.strokeText(text, 64, 32);
        ctx.fillStyle = color;
        ctx.fillText(text, 64, 32);
        const texture = new THREE.CanvasTexture(canvas);
        texture.minFilter = THREE.LinearFilter;
        const mat = new THREE.SpriteMaterial({ map: texture, transparent: true, depthTest: false });
        const sprite = new THREE.Sprite(mat);
        sprite.scale.set(4, 2, 1);
        return sprite;
    }

    /**
     * Update element labels — creates/recycles sprites to match atom data.
     * @param {Array<{x,y,z,symbol,color}>} labels — array of label descriptors
     */
    updateElementLabels(labels) {
        if (!this._elementLabels) {
            this._elementLabels = new THREE.Group();
            this._elementLabels.visible = true;
            this.scene.add(this._elementLabels);
            this._labelPool = [];
        }

        const group = this._elementLabels;
        const pool = this._labelPool;
        const needed = labels ? labels.length : 0;

        // Hide excess sprites
        for (let i = needed; i < pool.length; i++) {
            pool[i].visible = false;
        }

        if (!labels) return;

        for (let i = 0; i < needed; i++) {
            const lb = labels[i];
            let sprite;
            if (i < pool.length) {
                sprite = pool[i];
                // Update texture if symbol changed
                if (sprite._symbol !== lb.symbol) {
                    sprite.material.map.dispose();
                    sprite.material.dispose();
                    const newSprite = this._makeTextSprite(lb.symbol, lb.color || '#ffffff');
                    newSprite._symbol = lb.symbol;
                    // Replace in pool and group
                    group.remove(sprite);
                    pool[i] = newSprite;
                    group.add(newSprite);
                    sprite = newSprite;
                }
            } else {
                sprite = this._makeTextSprite(lb.symbol, lb.color || '#ffffff');
                sprite._symbol = lb.symbol;
                pool.push(sprite);
                group.add(sprite);
            }
            sprite.position.set(lb.x, lb.y + 2.5, lb.z); // offset above atom center
            sprite.visible = true;
        }
    }

    toggleElementLabels(on) {
        if (this._elementLabels) this._elementLabels.visible = on;
    }

    clearElementLabels() {
        if (!this._elementLabels) return;
        for (const sprite of this._labelPool) {
            sprite.material.map.dispose();
            sprite.material.dispose();
        }
        this.scene.remove(this._elementLabels);
        this._elementLabels = null;
        this._labelPool = [];
    }

    clearMolecularMeshes() {
        if (this._bondCylinders) this._bondCylinders.count = 0;
        if (this.bondLines) this.bondLines.geometry.setDrawRange(0, 0);
        if (this._nucleusShells) this._nucleusShells.count = 0;
        if (this._orbitalShells) this._orbitalShells.count = 0;
        if (this._orbitalLobes) this._orbitalLobes.count = 0;
        if (this._aeForceIonic) [this._aeForceIonic, this._aeForceVdw, this._aeForceBond, this._aeForceNet].forEach(l => l.geometry.setDrawRange(0, 0));
    }

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

    render() {
        this.controls.update();
        this._animateQuantumField(performance.now());
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

        // Core overlays (geometry+material pairs)
        const simpleOverlays = [
            'velocityVectors', 'trails', 'bondLines',
            '_fieldHeatmap', '_fieldVectors', '_fluxVolume',
            '_peStreamlines', '_gravityVectors', '_particleForces',
        ];
        for (const name of simpleOverlays) disposeMesh(this[name]);

        // Atom/molecule visual enhancements (InstancedMesh or LineSegments)
        const atomOverlays = [
            '_nucleusShells', '_bondCylinders', '_orbitalShells', '_orbitalLobes',
            '_aeForceIonic', '_aeForceVdw', '_aeForceBond', '_aeForceNet'
        ];
        for (const name of atomOverlays) disposeMesh(this[name]);
        if (this._bondLight) this.scene.remove(this._bondLight);

        // Field visualization overlays (Scale 0 streamlines, volumes, etc.)
        const fieldOverlays = [
            '_eFieldLines', '_bFieldLines', '_poyntingVectors', '_divField',
            '_fluxStreamlines', '_forceVolume', '_gravityField', '_strongForce', '_weakField',
            '_forceHeatmap', '_forceGlyphs',
            '_dualFluxVolume',
            '_chiralityField', '_lightField',
            '_darkMatterHalo', '_dampingZones', '_genesisIsosurface',
            '_confinementStrings',
        ];
        for (const name of fieldOverlays) disposeMesh(this[name]);

        // Force streamline pool (array of Line objects, not a single mesh)
        if (this._forceStreamlinePool) {
            for (const line of this._forceStreamlinePool) disposeMesh(line);
            this._forceStreamlinePool = null;
            this._forceStreamlineMats = null;
        }

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

        this.clearElementLabels();
    }
}
