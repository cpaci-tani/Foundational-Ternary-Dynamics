/**
 * Three.js 3D Viewport — renders particles from the simulation bridge.
 *
 * Uses THREE.Points with custom ShaderMaterial for antialiased circles.
 * Orbital camera with smooth controls.
 */

import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { EffectComposer } from 'three/addons/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/addons/postprocessing/RenderPass.js';
import { UnrealBloomPass } from 'three/addons/postprocessing/UnrealBloomPass.js';
import { getById } from './particle-catalog.js';
import { potentialToColor, magnitudeToColor, fluxToColor } from './fields.js';

const MAX_PARTICLES = 100000;
const MAX_FIELD_GRID = 4096;  // up to 64×64 grid points (must cover lattice²)

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
        gl_PointSize = clamp(gl_PointSize, 0.5, 16.0);
        gl_Position = projectionMatrix * mvPosition;
    }
`;

const PARTICLE_FRAG = `
    varying vec3 vColor;
    varying float vSize;

    void main() {
        vec2 center = gl_PointCoord - vec2(0.5);
        float dist = length(center);
        if (dist > 0.5) discard;
        float alpha = 1.0 - smoothstep(0.35, 0.5, dist);
        // Subtle glow at center
        float glow = exp(-dist * dist * 8.0) * 0.3;
        gl_FragColor = vec4(vColor + glow, alpha * 0.95);
    }
`;

export class Viewport {
    constructor(container) {
        this.container = container;

        // Scene
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(0x0f1729);

        // Camera
        this.camera = new THREE.PerspectiveCamera(45, 1, 0.1, 1000);
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
        this.controls.dampingFactor = 0.08;
        this.controls.rotateSpeed = 0.8;
        this.controls.zoomSpeed = 1.2;
        this.controls.minDistance = 5;
        this.controls.maxDistance = 300;

        // Particle system
        this._initParticles();

        // Boundary / wireframe
        this.wireframe = null;
        this.showWireframe = true;
        this.showFlux = false;
        this.showHeatmap = false;
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

        geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
        geometry.setAttribute('particleColor', new THREE.BufferAttribute(colors, 3));
        geometry.setAttribute('size', new THREE.BufferAttribute(sizes, 1));
        geometry.setDrawRange(0, 0);

        const material = new THREE.ShaderMaterial({
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });

        this.particles = new THREE.Points(geometry, material);
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
            color: 0x2a3a5a, transparent: true, opacity: 0.3,
        });

        let group;
        switch (shape) {
            case 'cube':        group = this._buildCubeBoundary(mat, mode); break;
            case 'sphere':      group = this._buildSphereBoundary(mat); break;
            case 'dodecahedron': group = this._buildPlatonicBoundary('dodecahedron', mat); break;
            case 'icosahedron':  group = this._buildPlatonicBoundary('icosahedron', mat); break;
            case 'octahedron':   group = this._buildPlatonicBoundary('octahedron', mat); break;
            case 'cylinder':     group = this._buildCylinderBoundary(mat); break;
            case 'torus':        group = this._buildTorusBoundary(mat); break;
            default:             group = this._buildCubeBoundary(mat, mode); break;
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
            ? [[0,0,0],[s,0,0],[s,s,0],[0,s,0],[0,0,s],[s,0,s],[s,s,s],[0,s,s]]
            : [[-h,-h,-h],[h,-h,-h],[h,h,-h],[-h,h,-h],[-h,-h,h],[h,-h,h],[h,h,h],[-h,h,h]];
        const edges = [
            [0,1],[1,2],[2,3],[3,0],
            [4,5],[5,6],[6,7],[7,4],
            [0,4],[1,5],[2,6],[3,7]
        ];
        for (const [a, b] of edges) {
            vertices.push(...corners[a], ...corners[b]);
        }

        // Subdivision lines only in lattice mode
        if (mode === 'lattice') {
            const step = Math.max(4, Math.floor(s / 4));
            for (let i = step; i < s; i += step) {
                vertices.push(i,0,0, i,s,0);
                vertices.push(i,0,s, i,s,s);
                vertices.push(0,i,0, s,i,0);
                vertices.push(0,i,s, s,i,s);
                vertices.push(0,0,i, s,0,i);
                vertices.push(0,s,i, s,s,i);
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
            case 'icosahedron':  solidGeo = new THREE.IcosahedronGeometry(1, detail); break;
            case 'octahedron':   solidGeo = new THREE.OctahedronGeometry(1, detail); break;
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
                    const len = Math.sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
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
                    [0, phi, 1/phi], [0, phi, -1/phi], [0, -phi, 1/phi], [0, -phi, -1/phi],
                    [1/phi, 0, phi], [-1/phi, 0, phi], [1/phi, 0, -phi], [-1/phi, 0, -phi],
                    [phi, 1/phi, 0], [phi, -1/phi, 0], [-phi, 1/phi, 0], [-phi, -1/phi, 0],
                ];
                for (const n of normals) {
                    const len = Math.sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
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
        // Small axis indicator at origin
        const axisLen = 3;
        const axisGeo = new THREE.BufferGeometry();
        axisGeo.setAttribute('position', new THREE.Float32BufferAttribute([
            0,0,0, axisLen,0,0,  // X
            0,0,0, 0,axisLen,0,  // Y
            0,0,0, 0,0,axisLen,  // Z
        ], 3));
        axisGeo.setAttribute('color', new THREE.Float32BufferAttribute([
            0.9,0.3,0.3, 0.9,0.3,0.3,  // X = red
            0.3,0.9,0.3, 0.3,0.9,0.3,  // Y = green
            0.3,0.3,0.9, 0.3,0.3,0.9,  // Z = blue
        ], 3));
        const axisMat = new THREE.LineBasicMaterial({ vertexColors: true, transparent: true, opacity: 0.5 });
        this.axes = new THREE.LineSegments(axisGeo, axisMat);
        this.scene.add(this.axes);
    }

    setLatticeSize(size) {
        this.latticeSize = size;
        this._halfN = size / 2;
        this._buildBoundary(this._boundaryShape, this._boundaryMode);
        // Recenter camera for lattice mode
        if (this._boundaryMode === 'lattice') {
            const center = size / 2;
            const dist = size * 1.6;
            this.controls.target.set(center, center, center);
            this.camera.position.set(center + dist, center + dist * 0.5, center + dist);
            this.controls.update();
        }
    }

    toggleWireframe(on) {
        this.showWireframe = on;
        if (this.wireframe) this.wireframe.visible = on;
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
            posAttr.array[i * 6]     = px;
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
            colAttr.array[i * 6]     = 1.0;
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
                posAttr.array[seg * 6]     = trail.positions[idx0 * 3];
                posAttr.array[seg * 6 + 1] = trail.positions[idx0 * 3 + 1];
                posAttr.array[seg * 6 + 2] = trail.positions[idx0 * 3 + 2];
                // Segment end
                posAttr.array[seg * 6 + 3] = trail.positions[idx1 * 3];
                posAttr.array[seg * 6 + 4] = trail.positions[idx1 * 3 + 1];
                posAttr.array[seg * 6 + 5] = trail.positions[idx1 * 3 + 2];

                // Fade: old segments dim, new segments bright
                const fade = (j + 1) / len;
                colAttr.array[seg * 6]     = cr * fade * 0.8;
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
            posAttr.array[b * 6]     = atomData.positions[idxA * 3];
            posAttr.array[b * 6 + 1] = atomData.positions[idxA * 3 + 1];
            posAttr.array[b * 6 + 2] = atomData.positions[idxA * 3 + 2];
            // End vertex (atom B position)
            posAttr.array[b * 6 + 3] = atomData.positions[idxB * 3];
            posAttr.array[b * 6 + 4] = atomData.positions[idxB * 3 + 1];
            posAttr.array[b * 6 + 5] = atomData.positions[idxB * 3 + 2];

            // Bond color: blend the two atom colors
            const rA = atomData.colors[idxA * 3], gA = atomData.colors[idxA * 3 + 1], bA = atomData.colors[idxA * 3 + 2];
            const rB = atomData.colors[idxB * 3], gB = atomData.colors[idxB * 3 + 1], bB = atomData.colors[idxB * 3 + 2];
            colAttr.array[b * 6]     = rA;
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
        const mat = new THREE.ShaderMaterial({
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.NormalBlending,
        });
        this._fieldHeatmap = new THREE.Points(geo, mat);
        this._fieldHeatmap.visible = false;
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
            posAttr.array[i * 3]     = gridPositions[i * 3];
            posAttr.array[i * 3 + 1] = gridPositions[i * 3 + 1] - 0.3;
            posAttr.array[i * 3 + 2] = gridPositions[i * 3 + 2];

            const [r, g, b] = potentialToColor(potentials[i], maxAbsPotential);
            colAttr.array[i * 3]     = r;
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
            posAttr.array[i * 6]     = gx;
            posAttr.array[i * 6 + 1] = gy;
            posAttr.array[i * 6 + 2] = gz;

            // End: normalized direction × log-compressed length
            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce + 1e-20) * 10) : 0;
            posAttr.array[i * 6 + 3] = gx + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[i * 6 + 4] = gy + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[i * 6 + 5] = gz + (mag > 1e-20 ? fz / mag * scale : 0);

            // Color: dim tail → bright tip
            const [cr, cg, cb] = magnitudeToColor(mag, maxForce);
            colAttr.array[i * 6]     = cr * 0.5;
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
                posAttr.array[vi * 3]     = line[i * 3];
                posAttr.array[vi * 3 + 1] = line[i * 3 + 1];
                posAttr.array[vi * 3 + 2] = line[i * 3 + 2];
                posAttr.array[vi * 3 + 3] = line[(i + 1) * 3];
                posAttr.array[vi * 3 + 4] = line[(i + 1) * 3 + 1];
                posAttr.array[vi * 3 + 5] = line[(i + 1) * 3 + 2];

                // Color: fade from bright to dim along line
                const t = i / Math.max(1, nPts - 2);
                const bright = 1.0 - t * 0.6;
                colAttr.array[vi * 3]     = 0.26 * bright;
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

            posAttr.array[i * 6]     = gx;
            posAttr.array[i * 6 + 1] = gy;
            posAttr.array[i * 6 + 2] = gz;

            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce + 1e-20) * 10) : 0;
            posAttr.array[i * 6 + 3] = gx + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[i * 6 + 4] = gy + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[i * 6 + 5] = gz + (mag > 1e-20 ? fz / mag * scale : 0);

            // Grey color for gravity
            const t = mag / (maxForce + 1e-20);
            const c = 0.3 + 0.5 * t;
            colAttr.array[i * 6]     = c * 0.5;
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

            posAttr.array[i * 6]     = px;
            posAttr.array[i * 6 + 1] = py;
            posAttr.array[i * 6 + 2] = pz;

            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce + 1e-20) * 10) : 0;
            posAttr.array[i * 6 + 3] = px + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[i * 6 + 4] = py + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[i * 6 + 5] = pz + (mag > 1e-20 ? fz / mag * scale : 0);

            // Green color for net force
            const t = mag / (maxForce + 1e-20);
            colAttr.array[i * 6]     = 0.2;
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

    // ── Flux Volume Rendering (Scale 0 — substrate mode) ──────────────
    // Sparse instanced points that glow where flux is nonzero.
    // Updated each frame from getFluxVolume() or getFluxSlice() data.

    _buildFluxVolume(latticeSize) {
        // Pre-allocate for full lattice (capped at 64^3 for performance)
        const cap = Math.min(latticeSize, 64);
        const maxPts = cap * cap * cap;
        const positions = new Float32Array(maxPts * 3);
        const colors = new Float32Array(maxPts * 3);
        const sizes = new Float32Array(maxPts);

        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        geo.setAttribute('particleColor', new THREE.Float32BufferAttribute(colors, 3));
        geo.setAttribute('size', new THREE.Float32BufferAttribute(sizes, 1));
        geo.setDrawRange(0, 0);

        const mat = new THREE.ShaderMaterial({
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.NormalBlending,
        });

        this._fluxVolume = new THREE.Points(geo, mat);
        this._fluxVolume.visible = false;
        this._fluxVolumeSize = cap;
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
        if (!this._fluxVolume) this._buildFluxVolume(latticeSize);

        const posAttr = this._fluxVolume.geometry.getAttribute('position');
        const colAttr = this._fluxVolume.geometry.getAttribute('particleColor');
        const sizeAttr = this._fluxVolume.geometry.getAttribute('size');
        const N = Math.min(latticeSize, this._fluxVolumeSize);

        // Find max for normalization
        let maxFlux = 0;
        const total = N * N * N;
        for (let i = 0; i < total; i++) {
            if (volumeData[i] > maxFlux) maxFlux = volumeData[i];
        }

        // Render every voxel — base dots + flux-driven glow
        // Clip to boundary shape (normalized coords -1..1 from lattice center)
        let count = 0;
        const maxPts = posAttr.array.length / 3;
        const BASE_SIZE = 1.2;      // tiny base dot for inactive voxels (2× for 150 factor)
        const MAX_SIZE  = 18.0;     // bright active voxel (2× for 150 factor)
        const FLUX_THRESHOLD = 0.003; // below this = "inactive" (dark dot)
        const halfN = N / 2;

        for (let z = 0; z < N && count < maxPts; z++) {
            for (let y = 0; y < N && count < maxPts; y++) {
                for (let x = 0; x < N && count < maxPts; x++) {
                    // Boundary clipping: normalize to -1..1 from center
                    const nx = (x - halfN + 0.5) / halfN;
                    const ny = (y - halfN + 0.5) / halfN;
                    const nz = (z - halfN + 0.5) / halfN;
                    if (!this._insideBoundary(nx, ny, nz)) continue;

                    const mag = volumeData[z * N * N + y * N + x];

                    posAttr.array[count * 3]     = x;
                    posAttr.array[count * 3 + 1] = y;
                    posAttr.array[count * 3 + 2] = z;

                    if (mag < FLUX_THRESHOLD || maxFlux < 1e-20) {
                        colAttr.array[count * 3]     = 0.15;
                        colAttr.array[count * 3 + 1] = 0.18;
                        colAttr.array[count * 3 + 2] = 0.25;
                        sizeAttr.array[count] = BASE_SIZE;
                    } else {
                        const [r, g, b] = fluxToColor(mag, maxFlux);
                        colAttr.array[count * 3]     = r;
                        colAttr.array[count * 3 + 1] = g;
                        colAttr.array[count * 3 + 2] = b;
                        const t = mag / (maxFlux + 1e-20);
                        sizeAttr.array[count] = BASE_SIZE + (MAX_SIZE - BASE_SIZE) * t;
                    }

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
            if (axis === 0)      { x = index; y = a; z = b; }
            else if (axis === 1) { x = a; y = index; z = b; }
            else                 { x = a; y = b; z = index; }

            // Clip to boundary shape
            const nx = (x - halfN + 0.5) / halfN;
            const ny = (y - halfN + 0.5) / halfN;
            const nz = (z - halfN + 0.5) / halfN;
            if (!this._insideBoundary(nx, ny, nz)) continue;

            posAttr.array[count * 3]     = x;
            posAttr.array[count * 3 + 1] = y;
            posAttr.array[count * 3 + 2] = z;

            const [r, g, b2] = fluxToColor(sliceData[i], maxFlux);
            colAttr.array[count * 3]     = r;
            colAttr.array[count * 3 + 1] = g;
            colAttr.array[count * 3 + 2] = b2;

            sizeAttr.array[count] = 8.0 + 8.0 * (sliceData[i] / (maxFlux + 1e-20));
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

    // ══════════════════════════════════════════════════════════════════
    // ── Field Visualization Overlays (Scale 0) ───────────────────────
    // E-field, B-field, Poynting, Divergence, Flux streamlines, Forces,
    // Dual substrate, Chirality
    // ══════════════════════════════════════════════════════════════════

    // ── E-Field Lines (Cyan) ─────────────────────────────────────────
    _buildEFieldLines() {
        const maxVerts = 200 * 100 * 2; // 200 lines × 100 segments × 2 verts
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

                posAttr.array[vi * 3]     = sx;
                posAttr.array[vi * 3 + 1] = sy;
                posAttr.array[vi * 3 + 2] = sz;
                colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                vi++;

                posAttr.array[vi * 3]     = line[(i + 1) * 3];
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
        const maxVerts = 200 * 200 * 2; // B lines can be longer (closed loops)
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

                posAttr.array[vi * 3]     = sx;
                posAttr.array[vi * 3 + 1] = sy;
                posAttr.array[vi * 3 + 2] = sz;
                colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                vi++;

                posAttr.array[vi * 3]     = line[(i + 1) * 3];
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
            const a = vectors[i*3], b = vectors[i*3+1], c = vectors[i*3+2];
            const m = Math.sqrt(a*a + b*b + c*c);
            mags[i] = m;
            if (m > maxMag) maxMag = m;
        }
        const threshold = maxMag * 0.05;
        const halfN = this._halfN;
        let vi = 0;

        for (let i = 0; i < count && vi < maxArrows; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const vx = vectors[i*3], vy = vectors[i*3+1], vz = vectors[i*3+2];

            const px = positions[i*3], py = positions[i*3+1], pz = positions[i*3+2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            const scale = Math.log(1 + mag / maxMag) * 3;
            const nx = vx/mag * scale, ny = vy/mag * scale, nz = vz/mag * scale;

            // Arrow base (darker orange)
            posAttr.array[vi*6]   = px; posAttr.array[vi*6+1] = py; posAttr.array[vi*6+2] = pz;
            colAttr.array[vi*6]   = 0.8; colAttr.array[vi*6+1] = 0.55; colAttr.array[vi*6+2] = 0.15;
            // Arrow tip (bright yellow)
            posAttr.array[vi*6+3] = px+nx; posAttr.array[vi*6+4] = py+ny; posAttr.array[vi*6+5] = pz+nz;
            colAttr.array[vi*6+3] = 1.0; colAttr.array[vi*6+4] = 0.85; colAttr.array[vi*6+5] = 0.15;
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
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });
        this._divField = new THREE.Points(geo, mat);
        this._divField.visible = false;
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

            const px = positions[i*3], py = positions[i*3+1], pz = positions[i*3+2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;

            posAttr.array[vi*3]   = px;
            posAttr.array[vi*3+1] = py;
            posAttr.array[vi*3+2] = pz;

            const t = Math.abs(v) / maxVal;
            if (v > 0) {
                // Red (positive divergence = source)
                colAttr.array[vi*3] = 0.9; colAttr.array[vi*3+1] = 0.2; colAttr.array[vi*3+2] = 0.15;
            } else {
                // Blue (negative divergence = sink)
                colAttr.array[vi*3] = 0.15; colAttr.array[vi*3+1] = 0.3; colAttr.array[vi*3+2] = 0.9;
            }
            sizeAttr.array[vi] = 4 + 12 * t;
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
        const maxVerts = 200 * 100 * 2;
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

                posAttr.array[vi * 3]     = sx;
                posAttr.array[vi * 3 + 1] = sy;
                posAttr.array[vi * 3 + 2] = sz;
                colAttr.array[vi * 3] = r; colAttr.array[vi * 3 + 1] = g; colAttr.array[vi * 3 + 2] = b;
                vi++;

                posAttr.array[vi * 3]     = line[(i + 1) * 3];
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

    // ── 3D Force Volume (Gray-steel arrows) ──────────────────────────
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
            const a = vectors[i*3], b = vectors[i*3+1], c = vectors[i*3+2];
            const m = Math.sqrt(a*a + b*b + c*c);
            mags[i] = m;
            if (m > maxMag) maxMag = m;
        }
        const threshold = maxMag * 0.03;
        const halfN = this._halfN;
        let vi = 0;

        for (let i = 0; i < count && vi < maxArrows; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const vx = vectors[i*3], vy = vectors[i*3+1], vz = vectors[i*3+2];

            const px = positions[i*3], py = positions[i*3+1], pz = positions[i*3+2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            const scale = Math.log(1 + mag / maxMag) * 2;
            const nx = vx/mag * scale, ny = vy/mag * scale, nz = vz/mag * scale;

            // Base (dark steel)
            posAttr.array[vi*6]   = px; posAttr.array[vi*6+1] = py; posAttr.array[vi*6+2] = pz;
            colAttr.array[vi*6]   = 0.47; colAttr.array[vi*6+1] = 0.56; colAttr.array[vi*6+2] = 0.61;
            // Tip (brighter steel)
            posAttr.array[vi*6+3] = px+nx; posAttr.array[vi*6+4] = py+ny; posAttr.array[vi*6+5] = pz+nz;
            colAttr.array[vi*6+3] = 0.7; colAttr.array[vi*6+4] = 0.78; colAttr.array[vi*6+5] = 0.82;
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
            const a = vectors[i*3], b = vectors[i*3+1], c = vectors[i*3+2];
            const m = Math.sqrt(a*a + b*b + c*c);
            mags[i] = m;
            if (m > maxMag) maxMag = m;
        }
        const threshold = maxMag * 0.05;
        const halfN = this._halfN;
        let vi = 0;

        for (let i = 0; i < count && vi < maxArrows; i++) {
            const mag = mags[i];
            if (mag < threshold) continue;
            const vx = vectors[i*3], vy = vectors[i*3+1], vz = vectors[i*3+2];

            const px = positions[i*3], py = positions[i*3+1], pz = positions[i*3+2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            const t = mag / maxMag; // 0..1
            const scale = Math.log(1 + t) * 2.5;
            const nx = vx/mag * scale, ny = vy/mag * scale, nz = vz/mag * scale;

            // Base: cool blue-grey
            posAttr.array[vi*6]   = px; posAttr.array[vi*6+1] = py; posAttr.array[vi*6+2] = pz;
            colAttr.array[vi*6]   = 0.55 + t * 0.3;
            colAttr.array[vi*6+1] = 0.60 + t * 0.25;
            colAttr.array[vi*6+2] = 0.68 + t * 0.2;
            // Tip: bright white-blue
            posAttr.array[vi*6+3] = px+nx; posAttr.array[vi*6+4] = py+ny; posAttr.array[vi*6+5] = pz+nz;
            colAttr.array[vi*6+3] = 0.85 + t * 0.15;
            colAttr.array[vi*6+4] = 0.88 + t * 0.12;
            colAttr.array[vi*6+5] = 0.92 + t * 0.08;
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
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });
        this._dualFluxVolume = new THREE.Points(geo, mat);
        this._dualFluxVolume.visible = false;
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
            const a = lData.vectors[i*3], b = lData.vectors[i*3+1], c = lData.vectors[i*3+2];
            const m = Math.sqrt(a*a + b*b + c*c);
            dualMags[i] = m;
            if (m > maxL) maxL = m;
        }
        for (let i = 0; i < rCount; i++) {
            const a = rData.vectors[i*3], b = rData.vectors[i*3+1], c = rData.vectors[i*3+2];
            const m = Math.sqrt(a*a + b*b + c*c);
            dualMags[lCount + i] = m;
            if (m > maxR) maxR = m;
        }
        const maxVal = Math.max(maxL, maxR, 1e-20);
        const threshold = maxVal * 0.02;
        let vi = 0;

        // Boundary clipping for dual substrate
        const halfN = this._halfN;

        // L substrate (warm: orange-red)
        for (let i = 0; i < lCount && vi < maxPts; i++) {
            const mag = dualMags[i];
            if (mag < threshold) continue;
            const px = lData.positions[i*3], py = lData.positions[i*3+1], pz = lData.positions[i*3+2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            posAttr.array[vi*3] = px; posAttr.array[vi*3+1] = py; posAttr.array[vi*3+2] = pz;
            const t = mag / maxVal;
            colAttr.array[vi*3] = 0.9 * t; colAttr.array[vi*3+1] = 0.4 * t; colAttr.array[vi*3+2] = 0.15 * t;
            sizeAttr.array[vi] = 4 + 12 * t;
            vi++;
        }
        // R substrate (cool: blue-purple)
        for (let i = 0; i < rCount && vi < maxPts; i++) {
            const mag = dualMags[lCount + i];
            if (mag < threshold) continue;
            const px = rData.positions[i*3], py = rData.positions[i*3+1], pz = rData.positions[i*3+2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;
            posAttr.array[vi*3] = px; posAttr.array[vi*3+1] = py; posAttr.array[vi*3+2] = pz;
            const t = mag / maxVal;
            colAttr.array[vi*3] = 0.3 * t; colAttr.array[vi*3+1] = 0.2 * t; colAttr.array[vi*3+2] = 0.9 * t;
            sizeAttr.array[vi] = 4 + 12 * t;
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
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });
        this._chiralityField = new THREE.Points(geo, mat);
        this._chiralityField.visible = false;
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

            const px = positions[i*3], py = positions[i*3+1], pz = positions[i*3+2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;

            posAttr.array[vi*3]   = px;
            posAttr.array[vi*3+1] = py;
            posAttr.array[vi*3+2] = pz;

            const t = Math.abs(v) / maxVal;
            if (v > 0) {
                // L-dominant: warm red
                colAttr.array[vi*3] = 0.9 * t; colAttr.array[vi*3+1] = 0.25 * t; colAttr.array[vi*3+2] = 0.15 * t;
            } else {
                // R-dominant: cool blue
                colAttr.array[vi*3] = 0.15 * t; colAttr.array[vi*3+1] = 0.35 * t; colAttr.array[vi*3+2] = 0.9 * t;
            }
            sizeAttr.array[vi] = 4 + 12 * t;
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
            vertexShader: PARTICLE_VERT,
            fragmentShader: PARTICLE_FRAG,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });
        this._lightField = new THREE.Points(geo, mat);
        this._lightField.visible = false;
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
            const sx = vectors[i*3], sy = vectors[i*3+1], sz = vectors[i*3+2];
            const m = Math.sqrt(sx*sx + sy*sy + sz*sz);
            if (m > maxMag) maxMag = m;
        }
        const threshold = maxMag * 0.03;
        const halfN = this._halfN;
        let vi = 0;

        for (let i = 0; i < count && vi < maxPts; i++) {
            const sx = vectors[i*3], sy = vectors[i*3+1], sz = vectors[i*3+2];
            const mag = Math.sqrt(sx*sx + sy*sy + sz*sz);
            if (mag < threshold) continue;

            const px = positions[i*3], py = positions[i*3+1], pz = positions[i*3+2];
            if (!this._insideBoundary((px - halfN) / halfN, (py - halfN) / halfN, (pz - halfN) / halfN)) continue;

            posAttr.array[vi*3]   = px;
            posAttr.array[vi*3+1] = py;
            posAttr.array[vi*3+2] = pz;

            // Warm yellow-white: brighter at higher |S|
            const t = mag / maxMag;
            colAttr.array[vi*3]   = 1.0 * t;         // R
            colAttr.array[vi*3+1] = 0.92 * t;        // G
            colAttr.array[vi*3+2] = 0.23 * t;        // B (warm yellow)
            sizeAttr.array[vi] = 6 + 16 * t;
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

    // Switch between lattice wireframe (Scale 0), coordinate axes (Scale 1), atom view (Scale 2), molecule view (Scale 3)
    setEngineMode(mode) {
        this._engineMode = mode;

        // ── Consciousness mode: dark background, bloom, centered camera ──
        if (mode === 'consciousness') {
            this.scene.background = new THREE.Color(0x050510);
            this._boundaryMode = 'origin';
            this._buildBoundary(this._boundaryShape, 'origin');
            if (this.wireframe) this.wireframe.visible = false;
            if (this.axes) this.axes.visible = false;
            if (this.peAxes) this.peAxes.visible = false;
            // Hide all overlays
            if (this.velocityVectors) this.velocityVectors.visible = false;
            if (this.trails) this.trails.visible = false;
            if (this.bondLines) this.bondLines.visible = false;
            if (this._fluxVolume) this._fluxVolume.visible = false;
            if (this._eFieldLines) this._eFieldLines.visible = false;
            if (this._bFieldLines) this._bFieldLines.visible = false;
            if (this._poyntingVectors) this._poyntingVectors.visible = false;
            if (this._divField) this._divField.visible = false;
            if (this._fluxStreamlines) this._fluxStreamlines.visible = false;
            if (this._forceVolume) this._forceVolume.visible = false;
            if (this._gravityField) this._gravityField.visible = false;
            if (this._dualFluxVolume) this._dualFluxVolume.visible = false;
            if (this._chiralityField) this._chiralityField.visible = false;
            if (this._lightField) this._lightField.visible = false;
            this.particles.visible = false;
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
            this.particles.visible = true;
        }

        if (mode === 'particles' || mode === 'atoms' || mode === 'molecules') {
            // Rebuild boundary at origin for PE/AE/molecule modes
            this._boundaryMode = 'origin';
            this._buildBoundary(this._boundaryShape, 'origin');
            if (this.axes) this.axes.visible = false;
            if (!this.peAxes) this._buildPEAxes();
            this.peAxes.visible = true;
            // Recenter camera at origin
            this.controls.target.set(0, 0, 0);
            this.camera.position.set(40, 30, 40);
            this.controls.update();
            // Toggle overlays
            if (this.velocityVectors) this.velocityVectors.visible = false;
            if (this.trails) this.trails.visible = false;
            if (this.bondLines) this.bondLines.visible = (mode === 'atoms' || mode === 'molecules');
            if (this._fieldHeatmap) this._fieldHeatmap.visible = false;
            if (this._fieldVectors) this._fieldVectors.visible = false;
            // Hide flux volume (Scale 0 only)
            if (this._fluxVolume) this._fluxVolume.visible = false;
            // Hide all field visualization overlays (Scale 0 only)
            if (this._eFieldLines) this._eFieldLines.visible = false;
            if (this._bFieldLines) this._bFieldLines.visible = false;
            if (this._poyntingVectors) this._poyntingVectors.visible = false;
            if (this._divField) this._divField.visible = false;
            if (this._fluxStreamlines) this._fluxStreamlines.visible = false;
            if (this._forceVolume) this._forceVolume.visible = false;
            if (this._gravityField) this._gravityField.visible = false;
            if (this._dualFluxVolume) this._dualFluxVolume.visible = false;
            if (this._chiralityField) this._chiralityField.visible = false;
            if (this._lightField) this._lightField.visible = false;
        } else {
            // Rebuild boundary at lattice center for Scale 0
            this._boundaryMode = 'lattice';
            this._buildBoundary(this._boundaryShape, 'lattice');
            if (this.axes) this.axes.visible = true;
            if (this.peAxes) this.peAxes.visible = false;
            // Hide PE/AE overlays
            if (this.velocityVectors) this.velocityVectors.visible = false;
            if (this.trails) this.trails.visible = false;
            if (this.bondLines) this.bondLines.visible = false;
            if (this._fieldHeatmap) this._fieldHeatmap.visible = this.showHeatmap;
            if (this._fieldVectors) this._fieldVectors.visible = false;
            // Restore flux volume if enabled
            if (this._fluxVolume) this._fluxVolume.visible = this.showFlux;
            // Recenter at lattice center
            const center = this.latticeSize / 2;
            const dist = this.latticeSize * 1.6;
            this.controls.target.set(center, center, center);
            this.camera.position.set(center + dist, center + dist * 0.5, center + dist);
            this.controls.update();
        }
    }

    _buildPEAxes() {
        const len = 30;
        const vertices = [];
        const colors = [];

        // X axis (red)
        vertices.push(-len,0,0, len,0,0);
        colors.push(0.5,0.2,0.2, 0.9,0.3,0.3);
        // Y axis (green)
        vertices.push(0,-len,0, 0,len,0);
        colors.push(0.2,0.5,0.2, 0.3,0.9,0.3);
        // Z axis (blue)
        vertices.push(0,0,-len, 0,0,len);
        colors.push(0.2,0.2,0.5, 0.3,0.3,0.9);

        // Grid lines on XZ plane (subtle)
        for (let i = -len; i <= len; i += 5) {
            if (i === 0) continue;
            vertices.push(i,0,-len, i,0,len);
            colors.push(0.15,0.18,0.25, 0.15,0.18,0.25);
            vertices.push(-len,0,i, len,0,i);
            colors.push(0.15,0.18,0.25, 0.15,0.18,0.25);
        }

        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        const mat = new THREE.LineBasicMaterial({ vertexColors: true, transparent: true, opacity: 0.6 });
        this.peAxes = new THREE.LineSegments(geo, mat);
        this.peAxes.visible = false;
        this.scene.add(this.peAxes);
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
                const nx = (px - halfN + 0.5) / halfN;
                const ny = (py - halfN + 0.5) / halfN;
                const nz = (pz - halfN + 0.5) / halfN;
                if (!this._insideBoundary(nx, ny, nz)) continue;
            }
            posAttr.array[count * 3]     = px;
            posAttr.array[count * 3 + 1] = py;
            posAttr.array[count * 3 + 2] = pz;
            colAttr.array[count * 3]     = data.colors[i * 3];
            colAttr.array[count * 3 + 1] = data.colors[i * 3 + 1];
            colAttr.array[count * 3 + 2] = data.colors[i * 3 + 2];
            sizeAttr.array[count] = data.sizes[i];
            count++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;

        geo.setDrawRange(0, count);
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
            // Scale size by log mass
            const s = 3.0 + 2.0 * Math.log10(p.mass_mev / 0.511 + 1.0);
            sizeAttr.array[i] = Math.min(s, 12);
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
        this.renderer.dispose();
        this.particles.geometry.dispose();
        this.particles.material.dispose();
        if (this.wireframe) {
            this.wireframe.geometry.dispose();
            this.wireframe.material.dispose();
        }
        if (this.velocityVectors) {
            this.velocityVectors.geometry.dispose();
            this.velocityVectors.material.dispose();
        }
        if (this.trails) {
            this.trails.geometry.dispose();
            this.trails.material.dispose();
        }
        if (this.bondLines) {
            this.bondLines.geometry.dispose();
            this.bondLines.material.dispose();
        }
        if (this._fieldHeatmap) {
            this._fieldHeatmap.geometry.dispose();
            this._fieldHeatmap.material.dispose();
        }
        if (this._fieldVectors) {
            this._fieldVectors.geometry.dispose();
            this._fieldVectors.material.dispose();
        }
        if (this._fluxVolume) {
            this._fluxVolume.geometry.dispose();
            this._fluxVolume.material.dispose();
        }
        // Field visualization overlays
        const fieldOverlays = [
            '_eFieldLines', '_bFieldLines', '_poyntingVectors', '_divField',
            '_fluxStreamlines', '_forceVolume', '_gravityField', '_dualFluxVolume', '_chiralityField',
            '_lightField'
        ];
        for (const name of fieldOverlays) {
            if (this[name]) {
                this[name].geometry.dispose();
                this[name].material.dispose();
            }
        }
        this.clearElementLabels();
    }
}
