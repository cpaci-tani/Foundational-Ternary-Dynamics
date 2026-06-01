import * as THREE from 'three';
import { GLSL_SIMPLEX_NOISE_3D } from './constants.js';
import { BaseRenderer } from './core/BaseRenderer.js';
// D-16: shared distance→temperature/biome classifier (single definition lives
// with the biome semantics in the inspector). Renderer consumes only .uTemp.
import { classifyBiome } from './inspector/scales/planetary.js';

// F-18: distance LOD for the procedural spheres. The vertex shader runs
// 4-octave fbm per vertex every frame, so cost scales with vertex count. Near
// bodies keep the full 64×64 mesh (appearance unchanged); far bodies — where
// the 5%-radius displacement silhouette and the extra triangles are sub-pixel —
// drop to a 24×24 mesh (~7× fewer vertices, still smooth, no visible popping).
//
// LOD metric is angular size = (visual radius) / (camera distance), which is
// proportional to on-screen size. The switch point is chosen conservatively:
// at a 60° vertical FOV, screen-height fraction ≈ ratio / tan(30°) ≈ ratio /
// 0.577, so the 0.015 threshold only drops detail once a body covers under
// ~2.6% of the viewport height (≈24 px on a 900 px canvas) — far below where
// 64- vs 24-segment silhouette detail is perceptible. Near, hero-sized bodies
// stay at full detail and render byte-identically.
const SPHERE_SEGMENTS_HIGH = 64;   // full-detail tessellation (unchanged from prior)
const SPHERE_SEGMENTS_LOW  = 24;   // far-body tessellation (still visibly smooth)
const LOD_ANGULAR_THRESHOLD = 0.015; // (radius / camera-distance) below this → low detail

const proceduralVertexShader = `
    varying vec2 vUv;
    varying vec3 vNormal;
    varying vec3 vPosition;
    
    uniform float uSeed;
    uniform float uTime;
    
    ${GLSL_SIMPLEX_NOISE_3D}

    float fbm(vec3 x) {
        float v = 0.0;
        float a = 0.5;
        vec3 shift = vec3(100.0);
        for (int i = 0; i < 4; ++i) {
            v += a * snoise(x);
            x = x * 2.0 + shift;
            a *= 0.5;
        }
        return v;
    }

    void main() {
        vUv = uv;
        vNormal = normal;
        
        // Displace vertices based on fbm for Rocky planets only (we pass a uniform)
        vec3 p = position;
        float displacement = fbm(p * 2.0 + uSeed);
        
        // Small rugged displacement
        p += normal * (displacement * 0.05);

        vPosition = p;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(p, 1.0);
    }
`;

const rockyFragmentShader = `
    varying vec2 vUv;
    varying vec3 vNormal;
    varying vec3 vPosition;
    
    uniform float uSeed;
    uniform float uTime;
    uniform float uTemp;    // Hot/Cold influence
    
    ${GLSL_SIMPLEX_NOISE_3D}

    float fbm(vec3 x) {
        float v = 0.0; float a = 0.5; vec3 shift = vec3(100.0);
        for (int i = 0; i < 4; ++i) { v += a * snoise(x); x = x * 2.0 + shift; a *= 0.5; }
        return v;
    }

    void main() {
        vec3 lightDir = normalize(vec3(-1.0, 1.0, 1.0)); // Simple orbital light approximation
        float diff = max(dot(vNormal, lightDir), 0.2); // slight ambient

        float noiseVal = fbm(vPosition * 2.0 + uSeed);
        
        vec3 baseColor = vec3(0.1, 0.4, 0.2); // Grass
        vec3 oceanColor = vec3(0.05, 0.2, 0.6); // Water
        vec3 sandColor = vec3(0.8, 0.7, 0.4); // Sand
        vec3 iceColor = vec3(0.9, 0.9, 0.95);
        vec3 magmaColor = vec3(0.8, 0.2, 0.0);
        vec3 basaltColor = vec3(0.15, 0.15, 0.15);

        // Biome logic based on uTemp roughly
        // Cold worlds (- uTemp), Hot worlds (+ uTemp)
        vec3 c = baseColor;
        
        if (uTemp > 0.8) {
            // Lava world
            c = mix(magmaColor, basaltColor, smoothstep(-0.2, 0.2, noiseVal));
        } else if (uTemp < -0.8) {
            // Ice world
            c = mix(oceanColor, iceColor, smoothstep(-0.3, 0.3, noiseVal));
        } else {
            // Earth-like
            if (noiseVal < -0.1) {
                c = oceanColor;
            } else if (noiseVal < 0.0) {
                c = mix(sandColor, baseColor, smoothstep(-0.1, 0.0, noiseVal));
            } else if (noiseVal > 0.3) {
                c = mix(baseColor, iceColor, smoothstep(0.3, 0.5, noiseVal)); // mountains
            }
        }
        
        // Atmosphere overlay / Clouds
        float cloudNoise = fbm(vPosition * 3.0 + uSeed * 0.5 + uTime * 0.1);
        if (cloudNoise > 0.3) {
            c = mix(c, vec3(1.0), smoothstep(0.3, 0.6, cloudNoise));
        }

        gl_FragColor = vec4(c * diff, 1.0);
    }
`;

const starFragmentShader = `
    varying vec2 vUv;
    varying vec3 vPosition;
    
    uniform float uSeed;
    uniform float uTime;
    uniform float uTemp;

    // D-15: shared Simplex-3D noise (mod289/permute/taylorInvSqrt/snoise) injected
    // from the single GLSL_SIMPLEX_NOISE_3D source. Verified byte-identical to the
    // copy previously inlined here (modulo indentation), so the star surface is
    // visually unchanged. The vertex + rocky-fragment shaders inject the same
    // constant; only fbm below is per-shader.
    ${GLSL_SIMPLEX_NOISE_3D}

    float fbm(vec3 x) {
        float v = 0.0; float a = 0.5; vec3 shift = vec3(100.0);
        for (int i = 0; i < 4; ++i) { v += a * snoise(x); x = x * 2.0 + shift; a *= 0.5; }
        return v;
    }

    void main() {
        float noiseVal = fbm(vPosition * 4.0 + uSeed - uTime * 0.2);
        
        vec3 colorA = vec3(1.0, 0.9, 0.5); // Yellowish
        vec3 colorB = vec3(1.0, 0.4, 0.1); // Orange/Red
        
        // Slightly tweak colors for temperature
        if (uTemp < -0.5) {
            colorA = vec3(0.5, 0.2, 0.1); // Red Dwarf
            colorB = vec3(0.8, 0.1, 0.0);
        } else if (uTemp > 0.5) {
            colorA = vec3(0.5, 0.8, 1.0); // Blue Giant
            colorB = vec3(0.8, 0.9, 1.0);
        }
        
        vec3 finalColor = mix(colorB, colorA, smoothstep(-0.5, 0.5, noiseVal * 1.5));
        gl_FragColor = vec4(finalColor * 1.5, 1.0); // emissive boost
    }
`;


export class PlanetaryRenderer extends BaseRenderer {
    constructor(scene, camera, renderer) {
        super(scene, camera, renderer);
        
        this._orbitLines = [];

        // High-res geometry for displacement (near bodies — appearance unchanged).
        this._sphereGeo = new THREE.SphereGeometry(1, SPHERE_SEGMENTS_HIGH, SPHERE_SEGMENTS_HIGH);
        // F-18: low-res geometry for distant bodies (far LOD). Shared across all
        // far bodies; meshes swap between this and _sphereGeo by angular size.
        this._sphereGeoLow = new THREE.SphereGeometry(1, SPHERE_SEGMENTS_LOW, SPHERE_SEGMENTS_LOW);

        // F-7: one ShaderMaterial *program* per body-type, not per body.
        // Every rocky body shares the same (vertex, fragment) GLSL source and
        // every star shares another, so there are only two distinct programs.
        // We keep two template materials and hand each body a .clone() of its
        // type's template: clones reuse the template's compiled program (the
        // WebGLPrograms cache key is the shader source, identical across
        // clones) while owning an independent `uniforms` object, so per-body
        // uSeed/uTime/uTemp still vary. Net: TRAPPIST-1's 8 bodies trigger 2
        // program compiles instead of 8, with byte-identical visual output.
        this._materialTemplates = {
            // type 0 (STAR) → emissive star surface shader
            star:  this._makeMaterialTemplate(true),
            // all other types (rocky / gas / moon / asteroid) → terrain shader
            rocky: this._makeMaterialTemplate(false),
        };
        
        // Add soft ambient light
        this._ambientLight = new THREE.AmbientLight(0xffffff, 0.2); // Reduced for realistic harsh light
        this._group.add(this._ambientLight);
        
        // Let BaseRenderer handle disposal of these
        this._cleanGeometries = () => {
            if (this._sphereGeo) this._sphereGeo.dispose();
            if (this._sphereGeoLow) this._sphereGeoLow.dispose();  // F-18: far-LOD geometry
            if (this._orbitLines) {
                this._orbitLines.forEach(l => {
                    if (l && l.geometry) l.geometry.dispose();
                });
            }
            // F-7: the per-type template materials are never assigned to a mesh
            // and so are not in this._materials (which BaseRenderer.dispose
            // walks). Dispose them here to free their compiled programs.
            if (this._materialTemplates) {
                Object.values(this._materialTemplates).forEach(m => {
                    if (m) m.dispose();
                });
            }
        };
    }

    /**
     * F-7: build one of the two shared shader-program templates. The body
     * clones from these so the GPU compiles each program exactly once.
     * @param {boolean} isStar  true → star surface shader; false → terrain shader
     */
    _makeMaterialTemplate(isStar) {
        return new THREE.ShaderMaterial({
            vertexShader: proceduralVertexShader,
            fragmentShader: isStar ? starFragmentShader : rockyFragmentShader,
            uniforms: {
                uSeed: { value: 0 },
                uTime: { value: 0 },
                uTemp: { value: 0 },
            },
        });
    }

    _getMaterial(index, type, seed) {
        const wantStar = type === 0;
        if (index < this._materials.length) {
            const mat = this._materials[index];
            // A body slot keeps its type for the renderer's lifetime (scenario
            // reload disposes + recreates the renderer), so the cached clone is
            // always the right type. Guard defensively anyway: if the type ever
            // changed, re-clone from the correct template so the program stays
            // shared and the visuals stay correct.
            if (mat._isStarMaterial === wantStar) {
                mat.uniforms.uSeed.value = seed;
                return mat;
            }
            mat.dispose();
            const replacement = this._cloneTypeTemplate(wantStar, seed);
            this._materials[index] = replacement;
            return replacement;
        }

        // F-7: clone the shared per-type template instead of compiling a fresh
        // ShaderMaterial. Clones reuse the template's compiled program; only the
        // uniforms object is independent (per-body uSeed/uTime/uTemp).
        const mat = this._cloneTypeTemplate(wantStar, seed);
        this._materials.push(mat);
        return mat;
    }

    /** Clone the star/rocky template and stamp this body's seed + a type tag. */
    _cloneTypeTemplate(isStar, seed) {
        const template = isStar ? this._materialTemplates.star : this._materialTemplates.rocky;
        const mat = template.clone();
        mat._isStarMaterial = isStar;
        mat.uniforms.uSeed.value = seed;
        return mat;
    }

    _getMesh(index) {
        if (index < this._meshes.length) {
            return this._meshes[index];
        }
        const mesh = new THREE.Mesh(this._sphereGeo);
        mesh._lodHigh = true;  // F-18: track current LOD bucket to avoid churn
        this._meshes.push(mesh);
        this._group.add(mesh);
        return mesh;
    }

    /**
     * F-18: select the sphere geometry LOD for one body by its on-screen
     * angular size. ratio = worldRadius / cameraDistance is proportional to
     * projected size; above LOD_ANGULAR_THRESHOLD the body is large/near and
     * keeps the full-detail mesh (unchanged appearance), below it switches to
     * the low-detail mesh. The geometry reference is only reassigned when the
     * bucket flips, so a body sitting at full or low detail incurs no per-frame
     * geometry writes — and the fbm vertex shader simply runs over fewer
     * vertices once a body is far.
     * @param {THREE.Mesh} mesh
     * @param {number} scaleR  world-space (visual) radius of this body
     */
    _applySphereLOD(mesh, scaleR) {
        // Distance from camera to the body centre. Reuse one scratch vector to
        // avoid per-body, per-frame allocation.
        const cam = this.camera;
        if (!cam) return;
        if (!this._lodScratch) this._lodScratch = new THREE.Vector3();
        const dist = this._lodScratch.copy(mesh.position).sub(cam.position).length();
        // Guard a degenerate near-zero distance (camera inside the body): treat
        // as "near" so the hero view never loses detail.
        const ratio = dist > 1e-6 ? (scaleR / dist) : Infinity;
        const wantHigh = ratio >= LOD_ANGULAR_THRESHOLD;
        if (wantHigh === mesh._lodHigh) return; // bucket unchanged → no write
        mesh._lodHigh = wantHigh;
        mesh.geometry = wantHigh ? this._sphereGeo : this._sphereGeoLow;
    }

    _getLight(index) {
        if (index < this._lights.length) {
            return this._lights[index];
        }
        const light = new THREE.PointLight(0xffffff, 1.5, 2000);
        this._lights.push(light);
        this._group.add(light);
        return light;
    }

    _getOrbitLine(index, r) {
        if (index < this._orbitLines.length) {
            return this._orbitLines[index];
        }
        const geometry = new THREE.BufferGeometry();
        const points = [];
        for(let i=0; i<=64; i++) {
            const theta = (i / 64) * Math.PI * 2;
            points.push(new THREE.Vector3(Math.cos(theta)*r, Math.sin(theta)*r, 0));
        }
        geometry.setFromPoints(points);
        const line = new THREE.Line(geometry, new THREE.LineBasicMaterial({ color: 0xffffff, transparent: true, opacity: 0.15 }));
        this._orbitLines.push(line);
        this._group.add(line);
        return line;
    }

    getInteractables() {
        return this._meshes.filter(m => m.visible);
    }

    setRenderOrbits(visible) {
        this._orbitLines.forEach(l => l.visible = visible);
    }

    setRenderAxes(visible) {
        this._meshes.forEach(m => {
            if (!m._axesHelper) {
                m._axesHelper = new THREE.AxesHelper(1.5); // scale relative to mesh
                m.add(m._axesHelper);
            }
            m._axesHelper.visible = visible;
        });
    }

    update(data) {
        const { count, buffer } = data; // buffer encodes: x,y,z, type, mass, r, id, seed, etc. (16 floats per body)
        const time = this.clock.getElapsedTime();

        let lightIndex = 0;
        let lineIndex = 0;

        // Hide excess
        for (let i = count; i < this._meshes.length; i++) this._meshes[i].visible = false;
        for (let i = 0; i < this._lights.length; i++) this._lights[i].intensity = 0;
        for (let i = 0; i < this._orbitLines.length; i++) this._orbitLines[i].visible = false;

        // Pass 1: Find star to position lights
        let starPos = new THREE.Vector3(0,0,0);
        for (let i = 0; i < count; i++) {
            if (buffer[i * 16 + 3] === 0) {
                starPos.set(buffer[i*16+0], buffer[i*16+1], buffer[i*16+2]);
            }
        }

        for (let i = 0; i < count; i++) {
            const off = i * 16;
            const x = buffer[off + 0];
            const y = buffer[off + 1];
            const z = buffer[off + 2];
            const type = buffer[off + 3];
            const mass = buffer[off + 4];
            const r = buffer[off + 5] * 0.1; // scale radius visually down
            const id = buffer[off + 6];
            const seed = buffer[off + 7];

            const mesh = this._getMesh(i);
            mesh.visible = true;
            mesh.position.set(x, y, z);

            // Visual sizing.
            const scaleR = Math.max(0.01, r);
            mesh.scale.set(scaleR, scaleR, scaleR);

            // F-18: pick geometry LOD by angular size (visual radius / camera
            // distance). Near bodies use the full 64×64 mesh — byte-identical to
            // before; far bodies (sub-threshold angular size) use the 24×24 mesh,
            // cutting per-vertex fbm cost ~7× with no perceptible change. Only
            // swap when the bucket changes to avoid per-frame geometry churn.
            this._applySphereLOD(mesh, scaleR);

            const mat = this._getMaterial(i, type, seed);
            mat.uniforms.uTime.value = time;
            
            // Heuristic temp based on distance to star
            let d = starPos.distanceTo(new THREE.Vector3(x,y,z));
            let uTemp = 0.0;
            if (type === 0 && mass > 1.2) uTemp = 0.8; // Blue giant
            if (type === 0 && mass < 0.6) uTemp = -0.8; // Red dwarf
            
            if (type !== 0) {
                // D-16: shared distance classification (close→hot, far→cold,
                // else 1.25 - d). Renderer needs only the temperature.
                uTemp = classifyBiome(d).uTemp;

                // Draw local orbit ring!
                const line = this._getOrbitLine(lineIndex++, d);
                line.visible = true;
                line.position.copy(starPos);
            }
            
            mat.uniforms.uTemp.value = uTemp;
            mesh.material = mat;
            mesh.userData = { id, type };

            if (type === 0) { // Star
                const light = this._getLight(lightIndex++);
                light.position.set(x, y, z);
                light.intensity = mass * 2.0;
            }
        }
    }
}
