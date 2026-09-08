import * as THREE from 'three';
import { GLSL_SIMPLEX_NOISE_3D } from './constants.js';
import { BaseRenderer } from './core/BaseRenderer.js';
import { KM_PER_AU } from './config/solar-system-data.js?v=4';
import { PlanetaryPhysicsOverlays } from './planetary-physics-overlays.js?v=2';

const DEG = Math.PI / 180;
const HIGH_SEGMENTS = 128;
const LOW_SEGMENTS = 48;
const LOD_ANGULAR_THRESHOLD = 0.012;

const bodyVertexShader = `
    varying vec2 vUv;
    varying vec3 vLocal;
    varying vec3 vWorldPosition;
    varying vec3 vWorldNormal;

    void main() {
        vUv = uv;
        vLocal = normalize(position);
        // Terrain remains a color signal. Displacing the surface would make
        // the rendered radius differ from the physical radius carried by the
        // simulation state, so geometry stays on the unit sphere.
        vec3 displaced = position;
        vec4 world = modelMatrix * vec4(displaced, 1.0);
        vWorldPosition = world.xyz;
        vWorldNormal = normalize(mat3(modelMatrix) * normal);
        gl_Position = projectionMatrix * viewMatrix * world;
    }
`;

const bodyFragmentShader = `
    precision highp float;
    varying vec2 vUv;
    varying vec3 vLocal;
    varying vec3 vWorldPosition;
    varying vec3 vWorldNormal;
    uniform float uSeed;
    uniform float uStyle;
    uniform float uTime;
    uniform vec3 uStarPosition;
    uniform vec3 uBaseColor;
    ${GLSL_SIMPLEX_NOISE_3D}

    float fbm(vec3 p) {
        float n = 0.0;
        n += 0.54 * snoise(p);
        n += 0.27 * snoise(p * 2.13 + 17.0);
        n += 0.13 * snoise(p * 4.71 - 9.0);
        n += 0.06 * snoise(p * 10.3 + 31.0);
        return n;
    }

    float wrappedDistance(float a, float b) {
        float d = abs(a - b);
        return min(d, 1.0 - d);
    }

    vec3 surfaceColor() {
        float n = fbm(vLocal * 3.2 + uSeed * 0.017);
        float fine = snoise(vLocal * 25.0 + uSeed);
        float lat = abs(vLocal.y);
        vec3 result = uBaseColor;

        if (uStyle < 0.5) {
            float cells = smoothstep(-0.35, 0.65, fbm(vLocal * 7.0 + uTime * 0.015));
            float spots = smoothstep(0.56, 0.78, fbm(vLocal * 3.0 - uTime * 0.01));
            result = mix(uBaseColor * 0.72, mix(uBaseColor, vec3(1.0), 0.58), cells) * (1.0 - spots * 0.34);
        } else if (uStyle < 1.5) {
            float crater = smoothstep(0.45, 0.70, fine);
            result = mix(vec3(0.25, 0.23, 0.21), vec3(0.62, 0.57, 0.50), n * 0.5 + 0.5) * (1.0 - crater * 0.30);
        } else if (uStyle < 2.5) {
            float clouds = fbm(vLocal * 5.0 + vec3(uTime * 0.025, 0.0, 0.0));
            result = mix(vec3(0.60, 0.31, 0.08), vec3(1.0, 0.91, 0.59), smoothstep(-0.55, 0.55, clouds));
        } else if (uStyle < 3.5) {
            float continents = fbm(vLocal * 2.7 + vec3(2.0, 0.0, 1.0));
            vec3 ocean = mix(vec3(0.012, 0.10, 0.31), vec3(0.02, 0.31, 0.63), n * 0.5 + 0.5);
            vec3 land = mix(vec3(0.13, 0.30, 0.08), vec3(0.56, 0.43, 0.20), smoothstep(-0.1, 0.55, fine));
            vec3 earth = mix(ocean, land, smoothstep(-0.04, 0.08, continents));
            earth = mix(earth, vec3(0.94, 0.97, 1.0), smoothstep(0.82, 0.95, lat));
            float clouds = smoothstep(0.38, 0.67, fbm(vLocal * 6.0 + vec3(uTime * 0.04, 0.0, 0.0)));
            result = mix(earth, vec3(1.0), clouds * 0.58);
        } else if (uStyle < 4.5) {
            vec3 mars = mix(vec3(0.28, 0.07, 0.025), vec3(0.78, 0.30, 0.10), n * 0.5 + 0.5);
            mars *= 0.76 + 0.24 * smoothstep(-0.7, 0.6, fine);
            result = mix(mars, vec3(0.92, 0.82, 0.68), smoothstep(0.89, 0.98, lat));
        } else if (uStyle < 5.5) {
            float bands = sin(vUv.y * 98.0 + n * 2.8) * 0.5 + 0.5;
            vec3 gas = mix(vec3(0.42, 0.22, 0.12), vec3(0.92, 0.78, 0.57), bands);
            float spotD = length(vec2(wrappedDistance(vUv.x, 0.67) * 2.7, (vUv.y - 0.43) * 5.2));
            result = mix(gas, vec3(0.72, 0.16, 0.07), 1.0 - smoothstep(0.05, 0.17, spotD));
        } else if (uStyle < 6.5) {
            float bands = sin(vUv.y * 125.0 + n * 1.4) * 0.5 + 0.5;
            result = mix(vec3(0.54, 0.43, 0.27), vec3(0.96, 0.86, 0.61), bands * 0.72);
        } else if (uStyle < 7.5) {
            float bands = sin(vUv.y * 42.0 + n) * 0.5 + 0.5;
            result = mix(vec3(0.22, 0.67, 0.73), vec3(0.66, 0.91, 0.91), bands * 0.35);
        } else if (uStyle < 8.5) {
            float bands = sin(vUv.y * 58.0 + n * 2.0) * 0.5 + 0.5;
            float storm = smoothstep(0.20, 0.02, length(vec2(wrappedDistance(vUv.x, 0.31) * 2.0, (vUv.y - 0.58) * 4.0)));
            result = mix(mix(vec3(0.025, 0.11, 0.47), vec3(0.12, 0.43, 0.84), bands), vec3(0.58, 0.76, 0.93), storm);
        } else if (uStyle < 9.5) {
            float crater = smoothstep(0.48, 0.78, fine);
            result = mix(vec3(0.24), vec3(0.72), n * 0.5 + 0.5) * (1.0 - crater * 0.32);
        } else if (uStyle < 10.5) {
            float fissures = smoothstep(0.48, 0.61, abs(snoise(vLocal * 18.0 + uSeed)));
            result = mix(vec3(0.46, 0.64, 0.72), vec3(0.94, 0.98, 1.0), fissures);
        } else if (uStyle < 11.5) {
            result = mix(vec3(0.31, 0.13, 0.035), vec3(0.91, 0.78, 0.24), smoothstep(-0.4, 0.55, n));
        } else if (uStyle < 12.5) {
            result = mix(uBaseColor * 0.48, uBaseColor * 1.25, smoothstep(-0.55, 0.55, n));
        } else if (uStyle < 13.5) {
            float basins = smoothstep(-0.18, 0.28, fbm(vLocal * 2.0 + uSeed));
            float crust = smoothstep(-0.42, 0.62, fine);
            result = mix(uBaseColor * 0.48, uBaseColor * 1.18, basins);
            result *= 0.78 + 0.22 * crust;
        } else if (uStyle < 14.5) {
            float hazeBands = sin(vUv.y * 54.0 + n * 2.4) * 0.5 + 0.5;
            float haze = smoothstep(-0.38, 0.58, fbm(vLocal * 5.0 + vec3(uTime * 0.012, 0.0, 0.0)));
            result = mix(uBaseColor * 0.62, uBaseColor * 1.28, hazeBands * 0.55 + haze * 0.18);
        } else {
            float youngBands = sin(vUv.y * 72.0 + n * 3.1) * 0.5 + 0.5;
            float clouds = smoothstep(-0.24, 0.55, fbm(vLocal * 4.2 + vec3(uTime * 0.009, 0.0, 0.0)));
            result = mix(uBaseColor * 0.55, uBaseColor * 1.34, youngBands * 0.48 + clouds * 0.28);
        }
        return result;
    }

    void main() {
        vec3 color = surfaceColor();
        if (uStyle < 0.5) {
            gl_FragColor = vec4(color * 1.65, 1.0);
            return;
        }
        vec3 normal = normalize(vWorldNormal);
        vec3 lightDir = normalize(uStarPosition - vWorldPosition);
        float diffuse = max(dot(normal, lightDir), 0.0);
        float nightRim = pow(1.0 - max(dot(normalize(cameraPosition - vWorldPosition), normal), 0.0), 3.0);
        float light = 0.075 + 0.925 * diffuse;
        gl_FragColor = vec4(color * light + uBaseColor * nightRim * 0.025, 1.0);
    }
`;

const atmosphereVertexShader = `
    varying vec3 vWorldNormal;
    varying vec3 vWorldPosition;
    void main() {
        vec4 world = modelMatrix * vec4(position, 1.0);
        vWorldPosition = world.xyz;
        vWorldNormal = normalize(mat3(modelMatrix) * normal);
        gl_Position = projectionMatrix * viewMatrix * world;
    }
`;

const atmosphereFragmentShader = `
    precision highp float;
    varying vec3 vWorldNormal;
    varying vec3 vWorldPosition;
    uniform vec3 uColor;
    uniform float uStrength;
    void main() {
        vec3 viewDir = normalize(cameraPosition - vWorldPosition);
        float fresnel = pow(1.0 - abs(dot(normalize(vWorldNormal), viewDir)), 2.4);
        gl_FragColor = vec4(uColor, fresnel * uStrength);
    }
`;

const ringVertexShader = `
    varying float vRadius;
    void main() {
        vRadius = length(position.xy);
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }
`;

const ringFragmentShader = `
    precision highp float;
    varying float vRadius;
    uniform vec3 uColor;
    uniform float uOpacity;
    uniform float uInner;
    uniform float uOuter;
    void main() {
        float t = clamp((vRadius - uInner) / max(0.001, uOuter - uInner), 0.0, 1.0);
        float bands = 0.48 + 0.30 * sin(t * 145.0) + 0.14 * sin(t * 411.0);
        float cassini = smoothstep(0.012, 0.035, abs(t - 0.64));
        float edge = smoothstep(0.0, 0.055, t) * smoothstep(0.0, 0.055, 1.0 - t);
        gl_FragColor = vec4(uColor * (0.62 + bands * 0.38), uOpacity * edge * cassini);
    }
`;

function simToWorld(x, y, z, target = new THREE.Vector3()) {
    return target.set(x, z, y);
}

function seeded(index) {
    const x = Math.sin(index * 12.9898 + 78.233) * 43758.5453;
    return x - Math.floor(x);
}

function createLabelTexture(text, color) {
    const canvas = document.createElement('canvas');
    canvas.width = 512;
    canvas.height = 128;
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.font = '600 38px system-ui, sans-serif';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.shadowColor = '#020617';
    ctx.shadowBlur = 9;
    ctx.lineWidth = 7;
    ctx.strokeStyle = '#020617';
    ctx.strokeText(text, 256, 64);
    ctx.fillStyle = color || '#e5eefc';
    ctx.fillText(text, 256, 64);
    const texture = new THREE.CanvasTexture(canvas);
    texture.colorSpace = THREE.SRGBColorSpace;
    return texture;
}

function atmosphereSpec(style, body) {
    const radiusKm = Math.max(Number(body?.radiusKm) || 0, 1e-12);
    const atmosphereHeightKm = Math.max(Number(body?.atmosphereHeightKm) || 0, 0);
    const scale = 1 + atmosphereHeightKm / radiusKm;
    if (style === 2) return { color: 0xffd38a, strength: 0.38, scale };
    if (style === 3) return { color: 0x4aa9ff, strength: 0.55, scale };
    if (style === 5) return { color: 0xd9b58a, strength: 0.20, scale };
    if (style === 6) return { color: 0xf4d28d, strength: 0.18, scale };
    if (style === 7) return { color: 0x7ee5ef, strength: 0.28, scale };
    if (style === 8) return { color: 0x3a78ff, strength: 0.32, scale };
    if (style === 12 && atmosphereHeightKm > 0) return { color: 0xe7a643, strength: 0.22, scale };
    return null;
}

export class PlanetaryRenderer extends BaseRenderer {
    constructor(scene, camera, renderer, controls = null) {
        super(scene, camera, renderer);
        this.controls = controls;
        this._nodes = [];
        this._nodeById = new Map();
        this._orbitLines = [];
        this._renderOrbits = true;
        this._renderAxes = false;
        this._renderLabels = true;
        this._renderMoons = true;
        this._renderBelts = true;
        this._renderHabitableZone = true;
        this._selectedId = -1;
        this._followId = -1;
        this._followPosition = null;
        this._starPosition = new THREE.Vector3();
        this._scratch = new THREE.Vector3();
        this._sphereHigh = new THREE.SphereGeometry(1, HIGH_SEGMENTS, HIGH_SEGMENTS);
        this._sphereLow = new THREE.SphereGeometry(1, LOW_SEGMENTS, LOW_SEGMENTS);
        this._buildEnvironment();
        this._physicsOverlays = new PlanetaryPhysicsOverlays(this._group);

        this._cleanGeometries = () => {
            this._physicsOverlays?.dispose();
            this._physicsOverlays = null;
            this._sphereHigh?.dispose();
            this._sphereLow?.dispose();
            for (const line of this._orbitLines) line.geometry?.dispose();
            for (const node of this._nodes) {
                node.label?.material?.map?.dispose();
                node.ring?.geometry?.dispose();
                node.selection?.geometry?.dispose();
            }
            this._eclipticGrid?.geometry?.dispose();
            this._habitableZone?.geometry?.dispose();
            this._asteroidBelt?.geometry?.dispose();
            this._kuiperBelt?.geometry?.dispose();
        };
    }

    _buildEnvironment() {
        this._ambient = new THREE.AmbientLight(0x9bb7d8, 0.06);
        this._group.add(this._ambient);

        this._eclipticGrid = new THREE.GridHelper(100, 20, 0x315b79, 0x172b42);
        this._eclipticGrid.material.transparent = true;
        this._eclipticGrid.material.opacity = 0.22;
        this._group.add(this._eclipticGrid);

        const hzGeo = new THREE.RingGeometry(0.95, 1.67, 192, 1);
        hzGeo.rotateX(-Math.PI / 2);
        const hzMat = new THREE.MeshBasicMaterial({ color: 0x4ade80, transparent: true, opacity: 0.09, side: THREE.DoubleSide, depthWrite: false });
        this._habitableZone = new THREE.Mesh(hzGeo, hzMat);
        this._materials.push(hzMat);
        this._group.add(this._habitableZone);

        this._asteroidBelt = this._makeBelt(1800, 2.08, 3.27, 0x9e8d76, 0.018);
        this._kuiperBelt = this._makeBelt(1400, 30, 50, 0x89a9bd, 0.06);
        this._kuiperBelt.visible = false;
        this._group.add(this._asteroidBelt, this._kuiperBelt);
    }

    _makeBelt(count, inner, outer, color, thickness) {
        const positions = new Float32Array(count * 3);
        for (let i = 0; i < count; i++) {
            const r = inner + (outer - inner) * Math.sqrt(seeded(i * 3 + 1));
            const theta = seeded(i * 3 + 2) * Math.PI * 2;
            positions[i * 3] = Math.cos(theta) * r;
            positions[i * 3 + 1] = (seeded(i * 3 + 3) - 0.5) * thickness * r;
            positions[i * 3 + 2] = Math.sin(theta) * r;
        }
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.BufferAttribute(positions, 3));
        const mat = new THREE.PointsMaterial({ color, size: inner < 5 ? 0.012 : 0.025, transparent: true, opacity: 0.62, sizeAttenuation: true, depthWrite: false });
        this._materials.push(mat);
        return new THREE.Points(geo, mat);
    }

    _physicalRadius(body) {
        const stateRadius = Number(body.r);
        if (Number.isFinite(stateRadius) && stateRadius > 0) return stateRadius;
        return Math.max(0, (Number(body.radiusKm) || 0) / KM_PER_AU);
    }

    _bodyWorldPosition(body, target = new THREE.Vector3()) {
        // One simulation AU is exactly one world-space unit. Axis permutation
        // adapts the ecliptic plane to Three.js Y-up without changing distance.
        return simToWorld(body.x, body.y, body.z, target);
    }

    _createNode(body) {
        const group = new THREE.Group();
        const material = new THREE.ShaderMaterial({
            vertexShader: bodyVertexShader,
            fragmentShader: bodyFragmentShader,
            uniforms: {
                uSeed: { value: body.seed || 1 },
                uStyle: { value: body.style ?? (body.type === 0 ? 0 : 12) },
                uTime: { value: 0 },
                uStarPosition: { value: this._starPosition },
                uBaseColor: { value: new THREE.Color(body.color || '#8ca4ba') },
            },
        });
        const mesh = new THREE.Mesh(this._sphereHigh, material);
        mesh.userData = { id: body.id, type: body.type, name: body.name || `Body ${body.id}` };
        group.add(mesh);
        this._materials.push(material);
        this._meshes.push(mesh);

        let atmosphere = null;
        const atmo = atmosphereSpec(body.style, body);
        if (atmo) {
            const atmoMat = new THREE.ShaderMaterial({
                vertexShader: atmosphereVertexShader,
                fragmentShader: atmosphereFragmentShader,
                uniforms: { uColor: { value: new THREE.Color(atmo.color) }, uStrength: { value: atmo.strength } },
                transparent: true,
                depthWrite: false,
                blending: THREE.AdditiveBlending,
                side: THREE.BackSide,
            });
            atmosphere = new THREE.Mesh(this._sphereLow, atmoMat);
            atmosphere.scale.setScalar(atmo.scale);
            group.add(atmosphere);
            this._materials.push(atmoMat);
        }

        let ring = null;
        if (body.rings) {
            const ringGeo = new THREE.RingGeometry(body.rings.innerRadius, body.rings.outerRadius, 256, 3);
            const ringMat = new THREE.ShaderMaterial({
                vertexShader: ringVertexShader,
                fragmentShader: ringFragmentShader,
                uniforms: {
                    uColor: { value: new THREE.Color(body.color || '#d7c294') },
                    uOpacity: { value: body.rings.opacity || 0.6 },
                    uInner: { value: body.rings.innerRadius },
                    uOuter: { value: body.rings.outerRadius },
                },
                transparent: true, side: THREE.DoubleSide, depthWrite: false,
            });
            ring = new THREE.Mesh(ringGeo, ringMat);
            ring.rotation.x = -Math.PI / 2;
            group.add(ring);
            this._materials.push(ringMat);
        }

        if (body.type === 0) {
            const starColor = new THREE.Color(body.color || '#ffd36a');
            const coronaMat = new THREE.MeshBasicMaterial({ color: starColor, transparent: true, opacity: 0.13, side: THREE.BackSide, blending: THREE.AdditiveBlending, depthWrite: false });
            const corona = new THREE.Mesh(this._sphereLow, coronaMat);
            corona.scale.setScalar(1.28);
            group.add(corona);
            this._materials.push(coronaMat);
            const light = new THREE.PointLight(starColor.clone().lerp(new THREE.Color('#ffffff'), 0.72), 4.8, 160, 1.35);
            group.add(light);
            this._lights.push(light);
        }

        const axis = new THREE.ArrowHelper(new THREE.Vector3(0, 1, 0), new THREE.Vector3(0, -1.35, 0), 2.7, 0x75d7ff, 0.25, 0.13);
        axis.visible = this._renderAxes;
        group.add(axis);

        const labelTexture = createLabelTexture(body.name || `Body ${body.id}`, body.color);
        const labelMaterial = new THREE.SpriteMaterial({ map: labelTexture, transparent: true, depthTest: false, depthWrite: false });
        const label = new THREE.Sprite(labelMaterial);
        label.scale.set(0.72, 0.18, 1);
        label.position.set(0, 1.55, 0);
        label.visible = this._renderLabels;
        group.add(label);
        this._materials.push(labelMaterial);

        const selectGeo = new THREE.TorusGeometry(1.34, 0.025, 12, 96);
        const selectMat = new THREE.MeshBasicMaterial({ color: 0x7dd3fc, transparent: true, opacity: 0.9, depthTest: false });
        const selection = new THREE.Mesh(selectGeo, selectMat);
        selection.visible = body.id === this._selectedId;
        selection.renderOrder = 10;
        group.add(selection);
        this._materials.push(selectMat);

        group.rotation.z = (body.axialTiltDeg || 0) * DEG;
        this._group.add(group);
        const node = {
            id: body.id,
            bodyType: body.type,
            isMoon: body.type === 3 && !body.rocheFragment,
            group, mesh, atmosphere, ring, axis, label, selection, radius: 1,
        };
        this._nodes.push(node);
        this._nodeById.set(body.id, node);
        return node;
    }

    _ensureOrbit(body) {
        let line = this._orbitLines.find((candidate) => candidate.userData.bodyId === body.id);
        if (line || !body.orbit || body.type === 0) return line;
        const points = [];
        const a = body.orbit.a;
        const e = body.orbit.e || 0;
        const inc = (body.orbit.i || 0) * DEG;
        const node = (body.orbit.longNode || 0) * DEG;
        const omega = ((body.orbit.longPeri || 0) - (body.orbit.longNode || 0)) * DEG;
        const cO = Math.cos(node), sO = Math.sin(node), co = Math.cos(omega), so = Math.sin(omega), ci = Math.cos(inc), si = Math.sin(inc);
        const isMoon = body.type === 3 && !body.rocheFragment;
        for (let k = 0; k <= 256; k++) {
            const anomaly = (k / 256) * Math.PI * 2;
            const xp = a * (Math.cos(anomaly) - e);
            const yp = a * Math.sqrt(1 - e * e) * Math.sin(anomaly);
            const x = (cO * co - sO * so * ci) * xp + (-cO * so - sO * co * ci) * yp;
            const y = (sO * co + cO * so * ci) * xp + (-sO * so + cO * co * ci) * yp;
            const z = so * si * xp + co * si * yp;
            points.push(simToWorld(x, y, z));
        }
        const geometry = new THREE.BufferGeometry().setFromPoints(points);
        const material = new THREE.LineBasicMaterial({
            color: isMoon ? 0x7395a8 : (body.color || 0x8db4cc),
            transparent: true, opacity: isMoon ? 0.34 : 0.30, depthWrite: false,
        });
        line = new THREE.Line(geometry, material);
        line.userData = { bodyId: body.id, parentId: body.parentId, bodyType: body.type, isMoon };
        this._orbitLines.push(line);
        this._materials.push(material);
        this._group.add(line);
        return line;
    }

    _applyLOD(node) {
        const distance = this._scratch.copy(node.group.position).sub(this.camera.position).length();
        const high = distance <= 1e-7 || node.radius / distance >= LOD_ANGULAR_THRESHOLD;
        const geometry = high ? this._sphereHigh : this._sphereLow;
        if (node.mesh.geometry !== geometry) node.mesh.geometry = geometry;
    }

    update(data) {
        const bodies = data.bodies || [];
        this._isSolarScenario = data.scenario === 'planetary-solar';
        const byId = new Map(bodies.map((body) => [body.id, body]));
        for (const node of this._nodes) {
            if (!byId.has(node.id)) node.group.visible = false;
        }
        for (const orbit of this._orbitLines) {
            if (!byId.has(orbit.userData.bodyId)) orbit.visible = false;
        }
        if (this._selectedId >= 0 && !byId.has(this._selectedId)) {
            this._selectedId = -1;
            this.stopFollowingBody();
        }
        const star = bodies.find((body) => body.type === 0);
        if (star) this._bodyWorldPosition(star, this._starPosition);
        else this._starPosition.set(3, 4, 2);
        const simTimeDays = (data.timeYears || 0) * 365.25;

        for (const body of bodies) {
            const node = this._nodeById.get(body.id) || this._createNode(body);
            const visible = !node.isMoon || this._renderMoons;
            node.group.visible = visible;
            if (!visible) continue;
            this._bodyWorldPosition(body, node.group.position);
            const radius = this._physicalRadius(body);
            node.radius = radius;
            node.group.scale.setScalar(radius);
            node.mesh.material.uniforms.uTime.value = simTimeDays;
            node.mesh.material.uniforms.uStarPosition.value.copy(this._starPosition);
            // Spin phase is integrated by the same physical clock as orbital
            // motion. A minute, hour, or day tick therefore advances every
            // surface by exactly that elapsed duration, independent of render
            // frames and internal Verlet substeps.
            node.mesh.rotation.y = Number.isFinite(body.rotationPhaseRad)
                ? body.rotationPhaseRad
                : 0;
            // Counter the body's display scale and adapt to camera distance.
            // This keeps labels readable in the system view without letting a
            // focused body's label fill the viewport.
            const viewDistance = node.group.position.distanceTo(this.camera.position);
            const labelWidth = THREE.MathUtils.clamp(viewDistance * 0.035, 0.16, 0.85);
            node.label.scale.set(labelWidth / radius, labelWidth * 0.25 / radius, 1 / radius);
            node.label.position.set(0, 1.30 + (labelWidth * 0.19) / radius, 0);
            node.axis.visible = this._renderAxes;
            node.label.visible = this._renderLabels;
            node.selection.visible = body.id === this._selectedId;
            this._applyLOD(node);

            const orbit = this._ensureOrbit(body);
            if (orbit) {
                const parent = byId.get(body.parentId);
                if (parent) this._bodyWorldPosition(parent, orbit.position);
                orbit.visible = this._renderOrbits && (!orbit.userData.isMoon || this._renderMoons);
            }
        }

        this._habitableZone.visible = this._renderHabitableZone && this._isSolarScenario;
        this._asteroidBelt.visible = this._renderBelts && this._isSolarScenario;
        this._kuiperBelt.visible = this._renderBelts && this._isSolarScenario;
        this._physicsOverlays?.update(data, this._selectedId);
        this._updateFollowCamera();
    }

    getInteractables() {
        return this._meshes.filter((mesh) => mesh.parent?.visible !== false && mesh.visible !== false);
    }

    setRenderOrbits(visible) { this._renderOrbits = !!visible; this._orbitLines.forEach((line) => { line.visible = this._renderOrbits && (!line.userData.isMoon || this._renderMoons); }); }
    setRenderAxes(visible) { this._renderAxes = !!visible; this._nodes.forEach((node) => { node.axis.visible = this._renderAxes; }); }
    setRenderLabels(visible) { this._renderLabels = !!visible; this._nodes.forEach((node) => { node.label.visible = this._renderLabels; }); }
    setRenderMoons(visible) {
        this._renderMoons = !!visible;
        this._nodes.forEach((node) => { if (node.isMoon) node.group.visible = this._renderMoons; });
        this._orbitLines.forEach((line) => { if (line.userData.isMoon) line.visible = this._renderMoons && this._renderOrbits; });
    }
    setRenderBelts(visible) {
        this._renderBelts = !!visible;
        if (this._asteroidBelt) this._asteroidBelt.visible = this._renderBelts && this._isSolarScenario;
        if (this._kuiperBelt) this._kuiperBelt.visible = this._renderBelts && this._isSolarScenario;
    }
    setRenderHabitableZone(visible) {
        this._renderHabitableZone = !!visible;
        if (this._habitableZone) this._habitableZone.visible = this._renderHabitableZone && this._isSolarScenario;
    }
    setEclipticVisible(visible) { this._eclipticGrid.visible = !!visible; }
    setRenderGravityField(visible) { this._physicsOverlays?.setEnabled('gravityField', visible); }
    setRenderAccelerationVectors(visible) { this._physicsOverlays?.setEnabled('accelerationVectors', visible); }
    setRenderVelocityVectors(visible) { this._physicsOverlays?.setEnabled('velocityVectors', visible); }
    setRenderHillSpheres(visible) { this._physicsOverlays?.setEnabled('hillSpheres', visible); }
    setRenderRocheLimits(visible) { this._physicsOverlays?.setEnabled('rocheLimits', visible); }
    setRenderCollisionShells(visible) { this._physicsOverlays?.setEnabled('collisionShells', visible); }
    getPhysicsOverlayStatus() { return this._physicsOverlays?.getStatus?.() || null; }

    setSelectedBody(id) {
        this._selectedId = Number.isFinite(Number(id)) ? Number(id) : -1;
        this._nodes.forEach((node) => { node.selection.visible = node.id === this._selectedId; });
        this._physicsOverlays?.setSelectedBody(this._selectedId);
        if (this._selectedId >= 0) return this.focusBody(this._selectedId, this.controls);
        this.stopFollowingBody();
        return true;
    }

    getBodyVisualState(id) {
        const node = this._nodeById.get(Number(id));
        if (!node) return null;
        return {
            position: node.group.position.clone(),
            radius: node.radius,
            rotationPhaseRad: node.mesh.rotation.y,
            axialTiltRad: node.group.rotation.z,
        };
    }

    focusBody(id, controls = this.controls) {
        const state = this.getBodyVisualState(id);
        if (!state || !this.camera || !controls) return false;
        this.controls = controls;
        const distance = Math.max(state.radius * 7.5, 1e-10);
        let direction;
        if (Number(id) !== 0) {
            // A three-quarter light-facing view reveals surface detail and the
            // day/night terminator. Looking from the old system-view direction
            // could frame a selected planet as an almost black silhouette.
            const towardStar = this._starPosition.clone().sub(state.position).normalize();
            const referenceUp = Math.abs(towardStar.y) > 0.86
                ? new THREE.Vector3(1, 0, 0)
                : new THREE.Vector3(0, 1, 0);
            const tangent = new THREE.Vector3().crossVectors(towardStar, referenceUp).normalize();
            direction = towardStar.multiplyScalar(0.61)
                .addScaledVector(tangent, 0.56)
                .addScaledVector(referenceUp, 0.56)
                .normalize();
        } else {
            direction = this.camera.position.clone().sub(controls.target).normalize();
            if (direction.lengthSq() < 0.5) direction.set(0.45, 0.35, 1).normalize();
        }
        controls.target.copy(state.position);
        controls.minDistance = Math.max(state.radius * 1.2, 1e-10);
        this.camera.near = Math.max(state.radius * 0.02, 1e-12);
        this.camera.updateProjectionMatrix();
        this.camera.position.copy(state.position).addScaledVector(direction, distance);
        this.camera.lookAt(state.position);
        this._followId = Number(id);
        this._followPosition = state.position.clone();
        controls.update?.();
        return true;
    }

    stopFollowingBody() {
        this._followId = -1;
        this._followPosition = null;
    }

    _updateFollowCamera() {
        if (this._followId < 0 || !this.camera || !this.controls) return;
        const state = this.getBodyVisualState(this._followId);
        if (!state) {
            this.stopFollowingBody();
            return;
        }
        if (this._followPosition) {
            this.camera.position.add(this._scratch.copy(state.position).sub(this._followPosition));
        }
        this.controls.target.copy(state.position);
        this._followPosition.copy(state.position);
        this.controls.update?.();
    }
}
