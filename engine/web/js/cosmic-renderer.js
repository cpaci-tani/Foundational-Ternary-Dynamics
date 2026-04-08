/**
 * CosmicRenderer: Scale 5 — cinematic deep-space rendering
 *
 * Design goals:
 *   - Stars: soft glow sprites with diffraction cross, blackbody color, size ~ luminosity
 *   - Gas: large volumetric nebula sprites, temperature-mapped, layered opacity
 *   - Dark matter: ultra-faint violet haze revealing large-scale structure
 *   - Black holes: multi-layer accretion disk with Doppler beaming (approaching side
 *     brighter), gravitational redshift gradient, photon ring, Einstein ring glow,
 *     event horizon with Hawking corona
 *   - Background: dense star field with faint nebula color patches
 */

import * as THREE from 'three';

const BT = {
    DARK_ENERGY: -3, QUASAR: -2, BLACK_HOLE: -1,
    DARK_MATTER: 0, GAS: 1, STAR: 2,
    NEUTRON_STAR: 3, NEBULA: 4, WHITE_DWARF: 5
};

// -- Procedural sprite textures (generated once) --

function makeStarSprite() {
    const c = document.createElement('canvas');
    c.width = 128; c.height = 128;
    const ctx = c.getContext('2d');
    const cx = 64, cy = 64;

    // Soft radial glow
    const grad = ctx.createRadialGradient(cx, cy, 0, cx, cy, 64);
    grad.addColorStop(0, 'rgba(255,255,255,1)');
    grad.addColorStop(0.08, 'rgba(255,255,255,0.9)');
    grad.addColorStop(0.2, 'rgba(255,240,220,0.4)');
    grad.addColorStop(0.45, 'rgba(255,200,150,0.1)');
    grad.addColorStop(0.7, 'rgba(200,150,255,0.03)');
    grad.addColorStop(1, 'rgba(0,0,0,0)');
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, 128, 128);

    // Diffraction cross (4-point star)
    ctx.globalCompositeOperation = 'lighter';
    for (let angle = 0; angle < 4; angle++) {
        const a = angle * Math.PI / 2;
        const gd = ctx.createLinearGradient(
            cx, cy,
            cx + Math.cos(a) * 60, cy + Math.sin(a) * 60
        );
        gd.addColorStop(0, 'rgba(255,255,255,0.5)');
        gd.addColorStop(0.3, 'rgba(255,255,255,0.08)');
        gd.addColorStop(1, 'rgba(255,255,255,0)');
        ctx.strokeStyle = gd;
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.moveTo(cx, cy);
        ctx.lineTo(cx + Math.cos(a) * 58, cy + Math.sin(a) * 58);
        ctx.stroke();
    }

    return new THREE.CanvasTexture(c);
}

function makeGasSprite() {
    const c = document.createElement('canvas');
    c.width = 128; c.height = 128;
    const ctx = c.getContext('2d');
    // Soft cloud with irregular edges
    const grad = ctx.createRadialGradient(64, 64, 0, 64, 64, 64);
    grad.addColorStop(0, 'rgba(255,255,255,0.6)');
    grad.addColorStop(0.25, 'rgba(255,255,255,0.3)');
    grad.addColorStop(0.5, 'rgba(255,255,255,0.1)');
    grad.addColorStop(0.75, 'rgba(255,255,255,0.03)');
    grad.addColorStop(1, 'rgba(255,255,255,0)');
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, 128, 128);
    return new THREE.CanvasTexture(c);
}

function makeHaloSprite() {
    const c = document.createElement('canvas');
    c.width = 256; c.height = 256;
    const ctx = c.getContext('2d');
    const grad = ctx.createRadialGradient(128, 128, 0, 128, 128, 128);
    grad.addColorStop(0, 'rgba(255,255,255,0.25)');
    grad.addColorStop(0.15, 'rgba(255,200,100,0.15)');
    grad.addColorStop(0.35, 'rgba(200,100,255,0.06)');
    grad.addColorStop(0.6, 'rgba(100,50,200,0.02)');
    grad.addColorStop(1, 'rgba(0,0,0,0)');
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, 256, 256);
    return new THREE.CanvasTexture(c);
}

const _starTex = makeStarSprite();
const _gasTex = makeGasSprite();
const _haloTex = makeHaloSprite();

// -- Accretion disk shader (Doppler beaming + radial heat gradient) --
const DISK_VERT = `
varying vec2 vUv;
varying vec3 vWorldPos;
void main() {
    vUv = uv;
    vWorldPos = (modelMatrix * vec4(position, 1.0)).xyz;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}`;

const DISK_FRAG = `
uniform float time;
uniform float innerRadius;
uniform float outerRadius;
uniform vec3 bhPosition;
uniform float opacity;
varying vec2 vUv;
varying vec3 vWorldPos;

void main() {
    vec2 local = vWorldPos.xz - bhPosition.xz;
    float r = length(local);
    float t = clamp((r - innerRadius) / (outerRadius - innerRadius), 0.0, 1.0);

    // Radial heat gradient: inner = white-blue, mid = orange, outer = deep red
    vec3 colInner = vec3(1.0, 0.95, 0.85);
    vec3 colMid   = vec3(1.0, 0.55, 0.15);
    vec3 colOuter = vec3(0.4, 0.08, 0.02);
    vec3 col = t < 0.35
        ? mix(colInner, colMid, t / 0.35)
        : mix(colMid, colOuter, (t - 0.35) / 0.65);

    // Keplerian spiral pattern
    float angle = atan(local.y, local.x);
    float spiral = sin(angle * 3.0 - time * 2.5 / pow(max(r, 0.1), 1.5)) * 0.5 + 0.5;
    col *= 0.75 + 0.25 * spiral;

    // Doppler beaming: approaching side (positive x) is brighter
    float doppler = 0.7 + 0.3 * (local.x / max(r, 0.01));
    col *= doppler;

    // Opacity: strong inner, fading outer with soft edge
    float alpha = (1.0 - t * t) * smoothstep(outerRadius, outerRadius * 0.85, r);
    alpha *= smoothstep(innerRadius * 0.9, innerRadius * 1.2, r);
    alpha *= 0.85;

    gl_FragColor = vec4(col, alpha * opacity);
}`;

function blackbodyColor(T) {
    const t = Math.max(0, Math.min(1, (T - 1500) / 30000));
    if (t < 0.15) return [1.0, 0.3, 0.05];
    if (t < 0.3)  return [1.0, 0.55, 0.15];
    if (t < 0.45) return [1.0, 0.85, 0.4];
    if (t < 0.6)  return [1.0, 0.95, 0.8];
    if (t < 0.75) return [0.85, 0.9, 1.0];
    return [0.6, 0.7, 1.0];
}

// ====================================================================
export class CosmicRenderer {
    constructor(scene, camera, renderer) {
        this.scene = scene;
        this.camera = camera;
        this.renderer = renderer;
        this._time = 0;

        this._group = new THREE.Group();
        this._group.name = 'cosmic-layer';
        this.scene.add(this._group);

        this._starCloud = null;
        this._gasCloud = null;
        this._dmCloud = null;
        this._bhMeshes = [];
        this._bgStars = null;

        this._showDM = true;
        this._showGas = true;
        this._showStars = true;
        this._showBH = true;
        this._showDisks = true;
        this._bhAge = new Map(); // track when each BH first appeared (for fade-in)

        this._initBackground();
    }

    // ================================================================
    // Background: dense star field with faint nebula color patches
    // ================================================================
    _initBackground() {
        this.scene.background = new THREE.Color(0x010104);

        const N = 12000;
        const pos = new Float32Array(N * 3);
        const col = new Float32Array(N * 3);
        const siz = new Float32Array(N);
        const rng = this._rng(42);

        for (let i = 0; i < N; i++) {
            const theta = Math.acos(2 * rng() - 1);
            const phi = 2 * Math.PI * rng();
            const r = 2500 + rng() * 5000;
            pos[i*3]   = r * Math.sin(theta) * Math.cos(phi);
            pos[i*3+1] = r * Math.sin(theta) * Math.sin(phi);
            pos[i*3+2] = r * Math.cos(theta);

            const T = 2000 + rng() * 30000;
            const [cr, cg, cb] = blackbodyColor(T);
            const bright = 0.15 + rng() * 0.85;
            col[i*3] = cr * bright;
            col[i*3+1] = cg * bright;
            col[i*3+2] = cb * bright;
            siz[i] = 0.8 + rng() * 2.5;
        }

        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.BufferAttribute(pos, 3));
        geo.setAttribute('color', new THREE.BufferAttribute(col, 3));
        geo.setAttribute('size', new THREE.BufferAttribute(siz, 1));

        this._bgStars = new THREE.Points(geo, new THREE.PointsMaterial({
            vertexColors: true, sizeAttenuation: false,
            transparent: true, opacity: 0.92, size: 1.5,
            map: _starTex, alphaTest: 0.01,
            blending: THREE.AdditiveBlending, depthWrite: false
        }));
        this._group.add(this._bgStars);
    }

    // ================================================================
    // Particle cloud helper
    // ================================================================
    _ensureCloud(name, maxCount, size, opacity, blending, texture) {
        const key = '_' + name + 'Cloud';
        if (this[key] && this[key].geometry.attributes.position.count >= maxCount) return this[key];
        if (this[key]) { this._group.remove(this[key]); this[key].geometry.dispose(); this[key].material.dispose(); }

        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.BufferAttribute(new Float32Array(maxCount * 3), 3));
        geo.setAttribute('color', new THREE.BufferAttribute(new Float32Array(maxCount * 3), 3));
        geo.setDrawRange(0, 0);

        this[key] = new THREE.Points(geo, new THREE.PointsMaterial({
            size, vertexColors: true, transparent: true, opacity, blending,
            depthWrite: false, sizeAttenuation: true,
            map: texture || _starTex, alphaTest: 0.001
        }));
        this[key].name = 'cosmic-' + name;
        this._group.add(this[key]);
        return this[key];
    }

    // ================================================================
    // Main update
    // ================================================================
    update(bodyData, diagnostics) {
        if (!bodyData || bodyData.count === 0) return;
        this._time += 0.016;

        const { positions, types, temperatures, sizes, count } = bodyData;
        const stars = [], gas = [], dm = [], bhs = [];

        for (let i = 0; i < count; i++) {
            const t = types[i];
            const e = { i, x: positions[i*3], y: positions[i*3+1], z: positions[i*3+2] };
            if (t === BT.STAR || t === BT.WHITE_DWARF || t === BT.NEUTRON_STAR) stars.push(e);
            else if (t === BT.GAS || t === BT.NEBULA) gas.push(e);
            else if (t === BT.DARK_MATTER || t === BT.DARK_ENERGY) dm.push(e);
            else if (t === BT.BLACK_HOLE || t === BT.QUASAR) bhs.push(e);
        }

        // -- Stars: diffraction-spike sprites, blackbody colored --
        if (this._showStars && stars.length > 0) {
            const cloud = this._ensureCloud('star', Math.max(stars.length, 500), 2.5, 1.0, THREE.AdditiveBlending, _starTex);
            const p = cloud.geometry.attributes.position.array;
            const c = cloud.geometry.attributes.color.array;
            for (let j = 0; j < stars.length; j++) {
                const s = stars[j];
                p[j*3] = s.x; p[j*3+1] = s.y; p[j*3+2] = s.z;
                const T = temperatures ? temperatures[s.i] : 5800;
                const [r, g, b] = blackbodyColor(Math.max(T, 2000));
                const br = 0.7 + Math.min((sizes ? sizes[s.i] : 5) * 0.03, 0.3);
                c[j*3] = r * br; c[j*3+1] = g * br; c[j*3+2] = b * br;
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            cloud.geometry.setDrawRange(0, stars.length);
            cloud.visible = true;
        } else if (this._starCloud) this._starCloud.visible = false;

        // -- Gas: large soft nebula sprites --
        if (this._showGas && gas.length > 0) {
            const cloud = this._ensureCloud('gas', Math.max(gas.length, 200), 8.0, 0.3, THREE.AdditiveBlending, _gasTex);
            const p = cloud.geometry.attributes.position.array;
            const c = cloud.geometry.attributes.color.array;
            for (let j = 0; j < gas.length; j++) {
                const g = gas[j];
                p[j*3] = g.x; p[j*3+1] = g.y; p[j*3+2] = g.z;
                const T = temperatures ? temperatures[g.i] : 1e4;
                const t = Math.max(0, Math.min(1, Math.log10(T + 1) / 7));
                // Nebula palette: cool blue-violet -> warm pink -> hot white-gold
                if (t < 0.35)      { c[j*3] = 0.15; c[j*3+1] = 0.12 + t * 0.5; c[j*3+2] = 0.5 + t * 0.5; }
                else if (t < 0.6)  { c[j*3] = 0.7 + t * 0.3; c[j*3+1] = 0.2; c[j*3+2] = 0.4; }
                else               { c[j*3] = 1.0; c[j*3+1] = 0.8; c[j*3+2] = 0.5; }
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            cloud.geometry.setDrawRange(0, gas.length);
            cloud.visible = true;
        } else if (this._gasCloud) this._gasCloud.visible = false;

        // -- Dark matter: ultra-faint violet revealing structure --
        if (this._showDM && dm.length > 0) {
            const cloud = this._ensureCloud('dm', Math.max(dm.length, 500), 4.0, 0.06, THREE.AdditiveBlending, _gasTex);
            const p = cloud.geometry.attributes.position.array;
            const c = cloud.geometry.attributes.color.array;
            for (let j = 0; j < dm.length; j++) {
                const d = dm[j];
                p[j*3] = d.x; p[j*3+1] = d.y; p[j*3+2] = d.z;
                c[j*3] = 0.25; c[j*3+1] = 0.15; c[j*3+2] = 0.5;
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            cloud.geometry.setDrawRange(0, dm.length);
            cloud.visible = true;
        } else if (this._dmCloud) this._dmCloud.visible = false;

        // -- Black holes --
        this._updateBlackHoles(bhs, bodyData);
    }

    // ================================================================
    // Black holes: event horizon + shader accretion disk + photon ring
    // ================================================================
    _updateBlackHoles(bhs, bodyData) {
        for (const m of this._bhMeshes) {
            this._group.remove(m);
            if (m.geometry) m.geometry.dispose();
            if (m.material) {
                if (m.material.map) m.material.map = null;
                m.material.dispose();
            }
        }
        this._bhMeshes = [];
        if (!this._showBH) return;

        for (const bh of bhs) {
            const mass = bodyData.sizes ? bodyData.sizes[bh.i] : 100;
            const rs = Math.max(0.6, Math.cbrt(mass) * 0.35);
            const bhPos = new THREE.Vector3(bh.x, bh.y, bh.z);

            // Track BH age for fade-in (disk appears gradually over ~5 seconds)
            if (!this._bhAge.has(bh.i)) this._bhAge.set(bh.i, this._time);
            const age = this._time - this._bhAge.get(bh.i);
            const fadeIn = Math.min(1.0, age / 5.0); // 0→1 over 5 seconds

            // --- Event horizon: pure black sphere ---
            const sGeo = new THREE.SphereGeometry(rs, 48, 48);
            const sMat = new THREE.MeshBasicMaterial({ color: 0x000000 });
            const sphere = new THREE.Mesh(sGeo, sMat);
            sphere.position.copy(bhPos);
            this._group.add(sphere);
            this._bhMeshes.push(sphere);

            // --- Hawking corona: faint warm glow just outside horizon ---
            const coronaGeo = new THREE.SphereGeometry(rs * 1.15, 32, 32);
            const coronaMat = new THREE.MeshBasicMaterial({
                color: 0x331100, transparent: true, opacity: 0.25 * fadeIn,
                blending: THREE.AdditiveBlending, depthWrite: false, side: THREE.BackSide
            });
            const corona = new THREE.Mesh(coronaGeo, coronaMat);
            corona.position.copy(bhPos);
            this._group.add(corona);
            this._bhMeshes.push(corona);

            if (!this._showDisks) continue;

            // --- Accretion disk: starts right at the event horizon ---
            const innerR = rs * 1.05;  // just outside the black sphere
            const outerR = rs * 14.0;
            const diskGeo = new THREE.RingGeometry(innerR, outerR, 128, 8);
            const diskMat = new THREE.ShaderMaterial({
                vertexShader: DISK_VERT,
                fragmentShader: DISK_FRAG,
                uniforms: {
                    time: { value: this._time },
                    innerRadius: { value: innerR },
                    outerRadius: { value: outerR },
                    bhPosition: { value: bhPos },
                    opacity: { value: fadeIn }
                },
                transparent: true,
                side: THREE.DoubleSide,
                blending: THREE.AdditiveBlending,
                depthWrite: false
            });
            const disk = new THREE.Mesh(diskGeo, diskMat);
            disk.position.copy(bhPos);
            disk.rotation.x = Math.PI * 0.5;
            this._group.add(disk);
            this._bhMeshes.push(disk);

            // --- Secondary thin disk (tilted, fainter — visual depth) ---
            const disk2Geo = new THREE.RingGeometry(innerR * 1.05, outerR * 0.7, 96, 4);
            const disk2Mat = new THREE.ShaderMaterial({
                vertexShader: DISK_VERT,
                fragmentShader: DISK_FRAG,
                uniforms: {
                    time: { value: this._time * 1.3 },
                    innerRadius: { value: innerR * 1.1 },
                    outerRadius: { value: outerR * 0.7 },
                    bhPosition: { value: bhPos },
                    opacity: { value: fadeIn * 0.7 }
                },
                transparent: true,
                side: THREE.DoubleSide,
                blending: THREE.AdditiveBlending,
                depthWrite: false
            });
            const disk2 = new THREE.Mesh(disk2Geo, disk2Mat);
            disk2.position.copy(bhPos);
            disk2.rotation.x = Math.PI * 0.5 + 0.08; // slight tilt for parallax
            disk2.rotation.z = 0.5;
            this._group.add(disk2);
            this._bhMeshes.push(disk2);

            // --- Einstein ring glow: large soft halo ---
            const einsteinGeo = new THREE.BufferGeometry();
            einsteinGeo.setAttribute('position', new THREE.BufferAttribute(
                new Float32Array([bhPos.x, bhPos.y, bhPos.z]), 3));
            const einstein = new THREE.Points(einsteinGeo, new THREE.PointsMaterial({
                size: rs * 28, color: 0x5533aa, transparent: true, opacity: 0.08 * fadeIn,
                blending: THREE.AdditiveBlending, depthWrite: false, sizeAttenuation: true,
                map: _haloTex
            }));
            this._group.add(einstein);
            this._bhMeshes.push(einstein);

            // --- Inner hot glow ---
            const glowGeo = new THREE.BufferGeometry();
            glowGeo.setAttribute('position', new THREE.BufferAttribute(
                new Float32Array([bhPos.x, bhPos.y, bhPos.z]), 3));
            const glow = new THREE.Points(glowGeo, new THREE.PointsMaterial({
                size: rs * 5, color: 0xffaa44, transparent: true, opacity: 0.35 * fadeIn,
                blending: THREE.AdditiveBlending, depthWrite: false, sizeAttenuation: true,
                map: _haloTex
            }));
            this._group.add(glow);
            this._bhMeshes.push(glow);
        }

        // Update disk shader time uniforms
        for (const m of this._bhMeshes) {
            if (m.material && m.material.uniforms && m.material.uniforms.time) {
                m.material.uniforms.time.value = this._time;
            }
        }
    }

    // ================================================================
    setCameraPreset(name, bodyData) {
        const presets = {
            overview:  { pos: [0, 350, 450], target: [0, 0, 0], fov: 60 },
            galaxy:    { pos: [40, 70, 100],  target: [0, 0, 0], fov: 55 },
            blackhole: { pos: [0, 35, 70],   target: [0, 0, 0], fov: 45 },
            merger:    { pos: [0, 80, 170],  target: [0, 0, 0], fov: 55 },
            quasar:    { pos: [0, 20, 45],   target: [0, 0, 0], fov: 42 }
        };
        const p = presets[name] || presets.overview;
        this.camera.position.set(p.pos[0], p.pos[1], p.pos[2]);
        this.camera.lookAt(p.target[0], p.target[1], p.target[2]);
        if (p.fov) { this.camera.fov = p.fov; this.camera.updateProjectionMatrix(); }
    }

    toggleDarkMatter(on)     { this._showDM = on; }
    toggleGasClouds(on)      { this._showGas = on; }
    toggleStars(on)          { this._showStars = on; }
    toggleBlackHoles(on)     { this._showBH = on; }
    toggleAccretionDisks(on) { this._showDisks = on; }

    dispose() {
        if (this._starCloud) { this._group.remove(this._starCloud); this._starCloud.geometry.dispose(); this._starCloud.material.dispose(); }
        if (this._gasCloud) { this._group.remove(this._gasCloud); this._gasCloud.geometry.dispose(); this._gasCloud.material.dispose(); }
        if (this._dmCloud) { this._group.remove(this._dmCloud); this._dmCloud.geometry.dispose(); this._dmCloud.material.dispose(); }
        for (const m of this._bhMeshes) { this._group.remove(m); if (m.geometry) m.geometry.dispose(); if (m.material) m.material.dispose(); }
        if (this._bgStars) { this._group.remove(this._bgStars); this._bgStars.geometry.dispose(); this._bgStars.material.dispose(); }
        this.scene.remove(this._group);
    }

    _rng(seed) {
        let s = seed;
        return () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
    }
}
