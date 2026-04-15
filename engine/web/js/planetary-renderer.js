import * as THREE from 'three';
import { GLSL_SIMPLEX_NOISE_3D } from './constants.js';
import { BaseRenderer } from './core/BaseRenderer.js';

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
    
    // ... noise omitted for brevity ...
    vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
    vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
    vec4 permute(vec4 x) { return mod289(((x*34.0)+1.0)*x); }
    vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }
    float snoise(vec3 v) {
        const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
        const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);
        vec3 i  = floor(v + dot(v, C.yyy) );
        vec3 x0 = v - i + dot(i, C.xxx) ;
        vec3 g = step(x0.yzx, x0.xyz);
        vec3 l = 1.0 - g;
        vec3 i1 = min( g.xyz, l.zxy );
        vec3 i2 = max( g.xyz, l.zxy );
        vec3 x1 = x0 - i1 + C.xxx;
        vec3 x2 = x0 - i2 + C.yyy;
        vec3 x3 = x0 - D.yyy;
        i = mod289(i); 
        vec4 p = permute( permute( permute( i.z + vec4(0.0, i1.z, i2.z, 1.0 )) + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));
        float n_ = 0.142857142857; 
        vec3  ns = n_ * D.wyz - D.xzx;
        vec4 j = p - 49.0 * floor(p * ns.z * ns.z);
        vec4 x_ = floor(j * ns.z);
        vec4 y_ = floor(j - 7.0 * x_ );
        vec4 x = x_ *ns.x + ns.yyyy;
        vec4 y = y_ *ns.x + ns.yyyy;
        vec4 h = 1.0 - abs(x) - abs(y);
        vec4 b0 = vec4( x.xy, y.xy );
        vec4 b1 = vec4( x.zw, y.zw );
        vec4 s0 = floor(b0)*2.0 + 1.0;
        vec4 s1 = floor(b1)*2.0 + 1.0;
        vec4 sh = -step(h, vec4(0.0));
        vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
        vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;
        vec3 p0 = vec3(a0.xy,h.x);
        vec3 p1 = vec3(a0.zw,h.y);
        vec3 p2 = vec3(a1.xy,h.z);
        vec3 p3 = vec3(a1.zw,h.w);
        vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
        p0 *= norm.x; p1 *= norm.y; p2 *= norm.z; p3 *= norm.w;
        vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
        m = m * m;
        return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3) ) );
    }
    float fbm(vec3 x) {
        float v = 0.0; float a = 0.5; vec3 shift = vec3(100.0);
        for (int i = 0; i < 4; ++i) { v += a * snoise(x); x = x * 2.0 + shift; a *= 0.5; }
        return v;
    }
    // ... end noise ...

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
        
        // High-res geometry for displacement
        this._sphereGeo = new THREE.SphereGeometry(1, 64, 64);
        
        // Add soft ambient light
        this._ambientLight = new THREE.AmbientLight(0xffffff, 0.2); // Reduced for realistic harsh light
        this._group.add(this._ambientLight);
        
        // Let BaseRenderer handle disposal of these
        this._cleanGeometries = () => {
            if (this._sphereGeo) this._sphereGeo.dispose();
            if (this._orbitLines) {
                this._orbitLines.forEach(l => {
                    if (l && l.geometry) l.geometry.dispose();
                });
            }
        };
    }

    _getMaterial(index, type, seed) {
        if (index < this._materials.length) {
            const mat = this._materials[index];
            mat.uniforms.uSeed.value = seed;
            return mat;
        }

        const isStar = type === 0;
        
        const mat = new THREE.ShaderMaterial({
            vertexShader: proceduralVertexShader,
            fragmentShader: isStar ? starFragmentShader : rockyFragmentShader,
            uniforms: {
                uSeed: { value: seed },
                uTime: { value: 0 },
                uTemp: { value: 0 }
            }
        });

        this._materials.push(mat);
        return mat;
    }

    _getMesh(index) {
        if (index < this._meshes.length) {
            return this._meshes[index];
        }
        const mesh = new THREE.Mesh(this._sphereGeo);
        this._meshes.push(mesh);
        this._group.add(mesh);
        return mesh;
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

            const mat = this._getMaterial(i, type, seed);
            mat.uniforms.uTime.value = time;
            
            // Heuristic temp based on distance to star
            let d = starPos.distanceTo(new THREE.Vector3(x,y,z));
            let uTemp = 0.0;
            if (type === 0 && mass > 1.2) uTemp = 0.8; // Blue giant
            if (type === 0 && mass < 0.6) uTemp = -0.8; // Red dwarf
            
            if (type !== 0) {
                // If close, hot. If far, cold.
                if (d < 0.5) uTemp = 1.0;
                else if (d > 2.0) uTemp = -1.0;
                else uTemp = (1.25 - d); // Rough middle
                
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
