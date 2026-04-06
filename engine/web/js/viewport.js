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
        this.camera = new THREE.PerspectiveCamera(45, 1, 0.01, 2000);
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
        this.controls.minDistance = 0.5;
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

        geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
        geometry.setAttribute('particleColor', new THREE.BufferAttribute(colors, 3));
        geometry.setAttribute('size', new THREE.BufferAttribute(sizes, 1));
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

        // Subdivision lines only in lattice mode (sparse — just midpoint cross)
        if (mode === 'lattice') {
            const step = Math.max(8, Math.floor(s / 2));
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
        // Rebuild flux volume for new size
        if (this._fluxVolume) {
            this.scene.remove(this._fluxVolume);
            this._fluxVolume.geometry.dispose();
            this._fluxVolume.material.dispose();
            this._fluxVolume = null;
            this._fluxVolumeSize = 0;
        }
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

    toggleAxes(on) {
        this._showAxes = on;
        const mode = this._engineMode || 'lattice';
        if (mode === 'consciousness') return;
        if (mode === 'lattice') {
            if (this.axes) this.axes.visible = on;
        } else {
            if (this.peAxes) this.peAxes.visible = on;
        }
    }

    toggleGrid(on) {
        this._showGrid = on;
        const mode = this._engineMode || 'lattice';
        if (mode === 'consciousness') return;
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

    // ── Confinement Strings ───────────────────────────────────────────
    _buildConfinementStrings() {
        const MAX_STRINGS = 50;
        const vertices = new Float32Array(MAX_STRINGS * 2 * 3);
        const colors = new Float32Array(MAX_STRINGS * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.9, linewidth: 2,
        });
        this.confinementStrings = new THREE.LineSegments(geo, mat);
        this.confinementStrings.visible = false;
        this.scene.add(this.confinementStrings);
    }

    updateConfinementStrings(particles, N) {
        if (!this.confinementStrings) this._buildConfinementStrings();
        if (!particles || particles.length === 0) {
            this.confinementStrings.geometry.setDrawRange(0, 0);
            return;
        }

        const posAttr = this.confinementStrings.geometry.getAttribute('position');
        const colAttr = this.confinementStrings.geometry.getAttribute('color');
        const halfN = N / 2;
        const R_BREAK = N / 4;
        let seg = 0;
        const maxSegs = posAttr.array.length / 6;

        for (let i = 0; i < particles.length && seg < maxSegs; i++) {
            const pi = particles[i];
            if (pi.state === 0) continue;
            for (let j = i + 1; j < particles.length && seg < maxSegs; j++) {
                const pj = particles[j];
                if (pj.state === 0 || pi.state * pj.state >= 0) continue;
                let dx = pj.x - pi.x, dy = pj.y - pi.y, dz = pj.z - pi.z;
                if (dx > halfN) dx -= N; else if (dx < -halfN) dx += N;
                if (dy > halfN) dy -= N; else if (dy < -halfN) dy += N;
                if (dz > halfN) dz -= N; else if (dz < -halfN) dz += N;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                if (r > R_BREAK) continue;
                // Tension color: orange (low) → red (high)
                const tension = Math.min(r / R_BREAK, 1.0);
                const cr1 = 1.0, cg1 = 0.6 * (1 - tension), cb1 = 0.1 * (1 - tension);
                const cr2 = 1.0, cg2 = 0.3 * (1 - tension), cb2 = 0.0;
                // Positions
                posAttr.array[seg * 6]     = pi.x;
                posAttr.array[seg * 6 + 1] = pi.y;
                posAttr.array[seg * 6 + 2] = pi.z;
                posAttr.array[seg * 6 + 3] = pi.x + dx;
                posAttr.array[seg * 6 + 4] = pi.y + dy;
                posAttr.array[seg * 6 + 5] = pi.z + dz;
                // Colors
                colAttr.array[seg * 6]     = cr1;
                colAttr.array[seg * 6 + 1] = cg1;
                colAttr.array[seg * 6 + 2] = cb1;
                colAttr.array[seg * 6 + 3] = cr2;
                colAttr.array[seg * 6 + 4] = cg2;
                colAttr.array[seg * 6 + 5] = cb2;
                seg++;
            }
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this.confinementStrings.geometry.setDrawRange(0, seg * 2);
    }

    toggleConfinementStrings(on) {
        if (!this.confinementStrings) this._buildConfinementStrings();
        this.confinementStrings.visible = on;
        if (!on) this.confinementStrings.geometry.setDrawRange(0, 0);
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
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.7 } },
            transparent: true,
            depthWrite: false,
            depthTest: true,
            blending: THREE.NormalBlending,
        });

        this._fluxVolume = new THREE.Points(geo, mat);
        this._fluxVolume.visible = false;
        this._fluxVolume.renderOrder = 10; // render after background stars (order 0)
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
        const MAX_SIZE  = (this._fluxPointScale || 1.0) * 10.0;
        const FLUX_THRESHOLD = this._fluxThreshold !== undefined ? this._fluxThreshold : 0.005;
        const halfN = N / 2;

        // Subsample for large lattices: step=2 for L>48 (renders 1/8 of voxels)
        const step = N > 48 ? 2 : 1;

        for (let z = 0; z < N && count < maxPts; z += step) {
            for (let y = 0; y < N && count < maxPts; y += step) {
                for (let x = 0; x < N && count < maxPts; x += step) {
                    // Boundary clipping: normalize to -1..1 from center
                    const nx = (x - halfN + 0.5) / halfN;
                    const ny = (y - halfN + 0.5) / halfN;
                    const nz = (z - halfN + 0.5) / halfN;
                    if (!this._insideBoundary(nx, ny, nz)) continue;

                    const mag = volumeData[z * N * N + y * N + x];

                    posAttr.array[count * 3]     = x;
                    posAttr.array[count * 3 + 1] = y;
                    posAttr.array[count * 3 + 2] = z;

                    // Skip inactive voxels entirely — no ghost dots
                    if (mag < FLUX_THRESHOLD || maxFlux < 1e-20) continue;

                    const [r, g, b] = fluxToColor(mag, maxFlux);
                    colAttr.array[count * 3]     = r;
                    colAttr.array[count * 3 + 1] = g;
                    colAttr.array[count * 3 + 2] = b;
                    const t = mag / (maxFlux + 1e-20);
                    const sizeScale = step > 1 ? step * 0.8 : 1.0; // compensate for subsampling
                    sizeAttr.array[count] = (1.0 + (MAX_SIZE - 1.0) * t) * sizeScale;

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
        // Sample every 2nd voxel for performance on 32^3
        const step = N > 24 ? 2 : 1;
        for (let z = 0; z < N && vi < maxPts; z += step) {
            for (let y = 0; y < N && vi < maxPts; y += step) {
                for (let x = 0; x < N && vi < maxPts; x += step) {
                    const idx = z * N * N + y * N + x;
                    const mag = fluxMag[idx];
                    // Sub-threshold: flux exists but below genesis
                    if (mag > 0.003 && mag < kGen) {
                        const t = mag / kGen; // 0..1 normalized
                        // Use raw lattice coordinates (matches updateFluxVolume and updateParticles)
                        posAttr.array[vi * 3]     = x;
                        posAttr.array[vi * 3 + 1] = y;
                        posAttr.array[vi * 3 + 2] = z;
                        // Purple gradient: faint → bright purple as flux approaches threshold
                        colAttr.array[vi * 3]     = 0.3 + t * 0.4;  // R
                        colAttr.array[vi * 3 + 1] = 0.1 + t * 0.15; // G
                        colAttr.array[vi * 3 + 2] = 0.5 + t * 0.4;  // B
                        sizeAttr.array[vi] = 1.5 + t * 6.0;
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
            [0,0,0, 1,0,0], [0,1,0, 1,1,0], [0,0,1, 1,0,1], [0,1,1, 1,1,1],
            [0,0,0, 0,1,0], [1,0,0, 1,1,0], [0,0,1, 0,1,1], [1,0,1, 1,1,1],
            [0,0,0, 0,0,1], [1,0,0, 1,0,1], [0,1,0, 0,1,1], [1,1,0, 1,1,1],
        ];

        for (const p of particles) {
            if (si >= 1200) break;
            const cx = p.x, cy = p.y, cz = p.z;
            for (const e of edges) {
                const i = si * 6;
                posAttr.array[i]     = cx - 1.5 + e[0] * 3;
                posAttr.array[i + 1] = cy - 1.5 + e[1] * 3;
                posAttr.array[i + 2] = cz - 1.5 + e[2] * 3;
                posAttr.array[i + 3] = cx - 1.5 + e[3] * 3;
                posAttr.array[i + 4] = cy - 1.5 + e[4] * 3;
                posAttr.array[i + 5] = cz - 1.5 + e[5] * 3;
                // Red tint
                colAttr.array[i] = 0.8; colAttr.array[i+1] = 0.2; colAttr.array[i+2] = 0.2;
                colAttr.array[i+3] = 0.8; colAttr.array[i+4] = 0.2; colAttr.array[i+5] = 0.2;
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

        for (let z = 0; z < N && vi < 4000; z++) {
            for (let y = 0; y < N && vi < 4000; y++) {
                for (let x = 0; x < N && vi < 4000; x++) {
                    const mag = fluxMag[z * N * N + y * N + x];
                    const dist = Math.abs(mag - kGenesis);
                    if (dist < band && mag > 0.01) {
                        const t = 1.0 - dist / band; // 1=on threshold, 0=edge of band
                        // Raw lattice coordinates (matches updateFluxVolume)
                        posAttr.array[vi * 3]     = x;
                        posAttr.array[vi * 3 + 1] = y;
                        posAttr.array[vi * 3 + 2] = z;
                        // Green glow: bright at threshold, fading at band edges
                        colAttr.array[vi * 3]     = 0.15 + t * 0.15;
                        colAttr.array[vi * 3 + 1] = 0.7 + t * 0.3;
                        colAttr.array[vi * 3 + 2] = 0.2 + t * 0.15;
                        sizeAttr.array[vi] = 2.0 + t * 5.0;
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
            return [[1,0,0], [0,1,0], [0,0,1]];
        } else if (l === 2) {
            // d-orbitals: dz², dxz, dyz, dx²-y², dxy (simplified to 4 main axes)
            return [[1,0,0], [0,1,0], [0,0,1], [0.707,0.707,0]];
        } else {
            // f-orbitals: 6 axes for symmetry
            return [[1,0,0], [0,1,0], [0,0,1], [0.707,0.707,0], [0.707,0,0.707], [0,0.707,0.707]];
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
        this._aeForceVdw   = createArrowSet(0x44ff44); // green for vdW
        this._aeForceBond  = createArrowSet(0xff8844); // orange for bond
        this._aeForceNet   = createArrowSet(0xffffff); // white for net
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

                posAttr.array[i * 6]     = px;
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
    toggleAEForceVdw(on)   { if (!this._aeForceVdw)   this._buildAEForceArrows(); this._aeForceVdw.visible = on; }
    toggleAEForceBond(on)  { if (!this._aeForceBond)  this._buildAEForceArrows(); this._aeForceBond.visible = on; }
    toggleAEForceNet(on)   { if (!this._aeForceNet)   this._buildAEForceArrows(); this._aeForceNet.visible = on; }

    // ══════════════════════════════════════════════════════════════════

    // Switch between lattice wireframe (Scale 0), coordinate axes (Scale 1), atom view (Scale 2), molecule view (Scale 3)
    setEngineMode(mode) {
        this._engineMode = mode;

        // ── Meta mode: hide all physics visuals, keep scene clean ──
        if (mode === 'meta') {
            this._boundaryMode = 'origin';
            // Hide everything from other scales
            if (this.wireframe) this.wireframe.visible = false;
            if (this.axes) this.axes.visible = false;
            if (this.peAxes) this.peAxes.visible = false;
            if (this.peGrid) this.peGrid.visible = false;
            if (this.particles) this.particles.visible = false;
            if (this._fluxVolume) this._fluxVolume.visible = false;
            if (this._fluxSlice) this._fluxSlice.visible = false;
            if (this._eFieldLines) this._eFieldLines.visible = false;
            if (this._bFieldLines) this._bFieldLines.visible = false;
            if (this._bondCylinders) this._bondCylinders.visible = false;
            if (this._bondLight) this._bondLight.visible = false;
            if (this._nucleusShells) this._nucleusShells.visible = false;
            if (this._orbitalShells) this._orbitalShells.visible = false;
            if (this._orbitalLobes) this._orbitalLobes.visible = false;
            if (this.velocityVectors) this.velocityVectors.visible = false;
            if (this.trails) this.trails.visible = false;
            if (this.bondLines) this.bondLines.visible = false;
            // Hide boundary box
            if (this._boundary) this._boundary.visible = false;
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
        this.controls.minDistance = 0.5;
        this.camera.near = 0.01;
        this.camera.updateProjectionMatrix();
        if (this._boundary) this._boundary.visible = true;

        // ── Consciousness mode: dark background, bloom, centered camera ──
        if (mode === 'consciousness') {
            this.scene.background = new THREE.Color(0x050510);
            this._boundaryMode = 'origin';
            this._buildBoundary(this._boundaryShape, 'origin');
            if (this.wireframe) this.wireframe.visible = false;
            if (this.axes) this.axes.visible = false;
            if (this.peAxes) this.peAxes.visible = false;
            if (this.peGrid) this.peGrid.visible = false;
            // Hide all overlays
            if (this.velocityVectors) this.velocityVectors.visible = false;
            if (this.trails) this.trails.visible = false;
            if (this.bondLines) this.bondLines.visible = false;
            if (this._bondCylinders) this._bondCylinders.visible = false;
            if (this._bondLight) this._bondLight.visible = false;
            if (this._nucleusShells) this._nucleusShells.visible = false;
            if (this._orbitalShells) this._orbitalShells.visible = false;
            if (this._orbitalLobes) this._orbitalLobes.visible = false;
            if (this._aeForceIonic) this._aeForceIonic.visible = false;
            if (this._aeForceVdw) this._aeForceVdw.visible = false;
            if (this._aeForceBond) this._aeForceBond.visible = false;
            if (this._aeForceNet) this._aeForceNet.visible = false;
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
            this.peAxes.visible = this._showAxes;
            if (this.peGrid) this.peGrid.visible = this._showGrid;
            // Recenter camera at origin
            this.controls.target.set(0, 0, 0);
            this.camera.position.set(40, 30, 40);
            this.controls.update();
            // Toggle overlays
            if (this.velocityVectors) this.velocityVectors.visible = false;
            if (this.trails) this.trails.visible = false;
            const isAtomMol = (mode === 'atoms' || mode === 'molecules');
            if (this.bondLines) this.bondLines.visible = false; // replaced by cylinders
            if (this._bondCylinders) this._bondCylinders.visible = isAtomMol;
            if (this._bondLight) this._bondLight.visible = isAtomMol;
            if (this._nucleusShells) this._nucleusShells.visible = isAtomMol;
            // Shells and lobes default OFF (user toggleable)
            if (this._orbitalShells) this._orbitalShells.visible = false;
            if (this._orbitalLobes) this._orbitalLobes.visible = false;
            // Force arrows default OFF
            if (this._aeForceIonic) this._aeForceIonic.visible = false;
            if (this._aeForceVdw) this._aeForceVdw.visible = false;
            if (this._aeForceBond) this._aeForceBond.visible = false;
            if (this._aeForceNet) this._aeForceNet.visible = false;
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
            if (this.axes) this.axes.visible = this._showAxes;
            if (this.peAxes) this.peAxes.visible = false;
            if (this.peGrid) this.peGrid.visible = false;
            // Hide PE/AE overlays
            if (this.velocityVectors) this.velocityVectors.visible = false;
            if (this.trails) this.trails.visible = false;
            if (this.bondLines) this.bondLines.visible = false;
            if (this._bondCylinders) this._bondCylinders.visible = false;
            if (this._bondLight) this._bondLight.visible = false;
            if (this._nucleusShells) this._nucleusShells.visible = false;
            if (this._orbitalShells) this._orbitalShells.visible = false;
            if (this._orbitalLobes) this._orbitalLobes.visible = false;
            if (this._aeForceIonic) this._aeForceIonic.visible = false;
            if (this._aeForceVdw) this._aeForceVdw.visible = false;
            if (this._aeForceBond) this._aeForceBond.visible = false;
            if (this._aeForceNet) this._aeForceNet.visible = false;
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

        // ── Axes (RGB lines through origin) ──
        const axVerts = [];
        const axColors = [];
        // X axis (red)
        axVerts.push(-len,0,0, len,0,0);
        axColors.push(0.5,0.2,0.2, 0.9,0.3,0.3);
        // Y axis (green)
        axVerts.push(0,-len,0, 0,len,0);
        axColors.push(0.2,0.5,0.2, 0.3,0.9,0.3);
        // Z axis (blue)
        axVerts.push(0,0,-len, 0,0,len);
        axColors.push(0.2,0.2,0.5, 0.3,0.3,0.9);

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
            grVerts.push(i,0,-len, i,0,len);
            grColors.push(0.15,0.18,0.25, 0.15,0.18,0.25);
            grVerts.push(-len,0,i, len,0,i);
            grColors.push(0.15,0.18,0.25, 0.15,0.18,0.25);
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
        // Atom/molecule visual enhancements
        const atomOverlays = [
            '_nucleusShells', '_bondCylinders', '_orbitalShells', '_orbitalLobes',
            '_aeForceIonic', '_aeForceVdw', '_aeForceBond', '_aeForceNet'
        ];
        for (const name of atomOverlays) {
            if (this[name]) {
                this[name].geometry.dispose();
                this[name].material.dispose();
            }
        }
        if (this._bondLight) this.scene.remove(this._bondLight);

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
