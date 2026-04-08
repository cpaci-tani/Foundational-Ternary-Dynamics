/**
 * CosmicRenderer: Scale 5 Three.js rendering
 *
 * Layered rendering with additive blending for a cinematic deep-space look.
 * Stars use blackbody colors. Gas uses temperature-mapped diffuse clouds.
 * Black holes get a multi-ring accretion disk with radial heat gradient,
 * photon ring, and gravitational darkening at the event horizon.
 */

import * as THREE from 'three';

const BT = {
    DARK_ENERGY: -3, QUASAR: -2, BLACK_HOLE: -1,
    DARK_MATTER: 0, GAS: 1, STAR: 2,
    NEUTRON_STAR: 3, NEBULA: 4, WHITE_DWARF: 5
};

// Attempt to load a round sprite texture for soft particle rendering.
// Falls back to square points if the image is unavailable.
let _circleTexture = null;
(function() {
    const c = document.createElement('canvas');
    c.width = 64; c.height = 64;
    const ctx = c.getContext('2d');
    const grad = ctx.createRadialGradient(32, 32, 0, 32, 32, 32);
    grad.addColorStop(0, 'rgba(255,255,255,1)');
    grad.addColorStop(0.3, 'rgba(255,255,255,0.6)');
    grad.addColorStop(0.7, 'rgba(255,255,255,0.15)');
    grad.addColorStop(1, 'rgba(255,255,255,0)');
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, 64, 64);
    _circleTexture = new THREE.CanvasTexture(c);
})();

function blackbodyColor(T) {
    const t = Math.max(0, Math.min(1, (T - 1500) / 30000));
    if (t < 0.15) return [1.0, 0.3, 0.05];
    if (t < 0.3)  return [1.0, 0.55, 0.15];
    if (t < 0.45) return [1.0, 0.85, 0.4];
    if (t < 0.6)  return [1.0, 0.95, 0.8];
    if (t < 0.75) return [0.85, 0.9, 1.0];
    return [0.6, 0.7, 1.0];
}

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

        this._initBackground();
    }

    // -- Background: distant stars on a sphere --
    _initBackground() {
        this.scene.background = new THREE.Color(0x020208);
        const N = 8000;
        const pos = new Float32Array(N * 3);
        const col = new Float32Array(N * 3);
        const rng = this._rng(42);
        for (let i = 0; i < N; i++) {
            const theta = Math.acos(2 * rng() - 1);
            const phi = 2 * Math.PI * rng();
            const r = 3000 + rng() * 4000;
            pos[i*3] = r * Math.sin(theta) * Math.cos(phi);
            pos[i*3+1] = r * Math.sin(theta) * Math.sin(phi);
            pos[i*3+2] = r * Math.cos(theta);
            const T = 2000 + rng() * 28000;
            const [cr, cg, cb] = blackbodyColor(T);
            const b = 0.3 + rng() * 0.7;
            col[i*3] = cr*b; col[i*3+1] = cg*b; col[i*3+2] = cb*b;
        }
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.BufferAttribute(pos, 3));
        geo.setAttribute('color', new THREE.BufferAttribute(col, 3));
        this._bgStars = new THREE.Points(geo, new THREE.PointsMaterial({
            size: 1.5, vertexColors: true, sizeAttenuation: false,
            transparent: true, opacity: 0.9
        }));
        this._group.add(this._bgStars);
    }

    // -- Particle cloud helper --
    _ensureCloud(name, maxCount, size, opacity, blending) {
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
            map: _circleTexture, alphaTest: 0.01
        }));
        this[key].name = 'cosmic-' + name;
        this._group.add(this[key]);
        return this[key];
    }

    // -- Main update --
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

        // Stars — bright, large, blackbody-colored
        if (this._showStars && stars.length > 0) {
            const cloud = this._ensureCloud('star', Math.max(stars.length, 500), 3.0, 1.0, THREE.AdditiveBlending);
            const p = cloud.geometry.attributes.position.array;
            const c = cloud.geometry.attributes.color.array;
            for (let j = 0; j < stars.length; j++) {
                const s = stars[j];
                p[j*3] = s.x; p[j*3+1] = s.y; p[j*3+2] = s.z;
                const T = temperatures ? temperatures[s.i] : 5800;
                const [r, g, b] = blackbodyColor(Math.max(T, 2000));
                const br = 0.8 + Math.min((sizes ? sizes[s.i] : 5) * 0.02, 0.2);
                c[j*3] = r*br; c[j*3+1] = g*br; c[j*3+2] = b*br;
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            cloud.geometry.setDrawRange(0, stars.length);
            cloud.visible = true;
        } else if (this._starCloud) this._starCloud.visible = false;

        // Gas — large soft clouds, temperature-colored
        if (this._showGas && gas.length > 0) {
            const cloud = this._ensureCloud('gas', Math.max(gas.length, 200), 6.0, 0.35, THREE.AdditiveBlending);
            const p = cloud.geometry.attributes.position.array;
            const c = cloud.geometry.attributes.color.array;
            for (let j = 0; j < gas.length; j++) {
                const g = gas[j];
                p[j*3] = g.x; p[j*3+1] = g.y; p[j*3+2] = g.z;
                const T = temperatures ? temperatures[g.i] : 1e4;
                const t = Math.max(0, Math.min(1, Math.log10(T + 1) / 7));
                if (t < 0.4)      { c[j*3] = 0.15; c[j*3+1] = 0.25+t; c[j*3+2] = 0.7; }
                else if (t < 0.65){ c[j*3] = 0.9;  c[j*3+1] = 0.35;   c[j*3+2] = 0.5; }
                else              { c[j*3] = 1.0;  c[j*3+1] = 0.85;   c[j*3+2] = 0.6; }
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            cloud.geometry.setDrawRange(0, gas.length);
            cloud.visible = true;
        } else if (this._gasCloud) this._gasCloud.visible = false;

        // Dark matter — faint purple haze
        if (this._showDM && dm.length > 0) {
            const cloud = this._ensureCloud('dm', Math.max(dm.length, 500), 5.0, 0.08, THREE.AdditiveBlending);
            const p = cloud.geometry.attributes.position.array;
            const c = cloud.geometry.attributes.color.array;
            for (let j = 0; j < dm.length; j++) {
                const d = dm[j]; p[j*3] = d.x; p[j*3+1] = d.y; p[j*3+2] = d.z;
                c[j*3] = 0.3; c[j*3+1] = 0.18; c[j*3+2] = 0.55;
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            cloud.geometry.setDrawRange(0, dm.length);
            cloud.visible = true;
        } else if (this._dmCloud) this._dmCloud.visible = false;

        // Black holes
        this._updateBlackHoles(bhs, bodyData);
    }

    // -- Black hole: event horizon + multi-layer accretion disk + photon ring --
    _updateBlackHoles(bhs, bodyData) {
        for (const m of this._bhMeshes) { this._group.remove(m); m.geometry.dispose(); m.material.dispose(); }
        this._bhMeshes = [];
        if (!this._showBH) return;

        for (const bh of bhs) {
            const mass = bodyData.sizes ? bodyData.sizes[bh.i] : 100;
            const rs = Math.max(0.8, Math.cbrt(mass) * 0.25);

            // Event horizon — dark sphere with subtle purple rim
            const sGeo = new THREE.SphereGeometry(rs, 32, 32);
            const sMat = new THREE.MeshBasicMaterial({ color: 0x030006 });
            const sphere = new THREE.Mesh(sGeo, sMat);
            sphere.position.set(bh.x, bh.y, bh.z);
            this._group.add(sphere); this._bhMeshes.push(sphere);

            if (!this._showDisks) continue;

            // Photon ring — thin bright ring at 1.5 rs (photon sphere)
            const prGeo = new THREE.TorusGeometry(rs * 1.5, rs * 0.06, 8, 64);
            const prMat = new THREE.MeshBasicMaterial({
                color: 0xffcc44, transparent: true, opacity: 0.7,
                blending: THREE.AdditiveBlending, depthWrite: false
            });
            const photonRing = new THREE.Mesh(prGeo, prMat);
            photonRing.position.set(bh.x, bh.y, bh.z);
            photonRing.rotation.x = Math.PI * 0.5;
            this._group.add(photonRing); this._bhMeshes.push(photonRing);

            // Accretion disk — 3 concentric rings with heat gradient
            const diskLayers = [
                { inner: rs * 2.0, outer: rs * 4.5,  opacity: 0.85, colors: [[1,1,0.85],[1,0.7,0.3]] },
                { inner: rs * 4.5, outer: rs * 8.0,  opacity: 0.6,  colors: [[1,0.6,0.2],[0.9,0.3,0.08]] },
                { inner: rs * 8.0, outer: rs * 14.0, opacity: 0.3,  colors: [[0.8,0.25,0.05],[0.3,0.08,0.02]] },
            ];

            for (const layer of diskLayers) {
                const rGeo = new THREE.RingGeometry(layer.inner, layer.outer, 96, 3);
                const colors = new Float32Array(rGeo.attributes.position.count * 3);
                for (let v = 0; v < rGeo.attributes.position.count; v++) {
                    const px = rGeo.attributes.position.getX(v);
                    const py = rGeo.attributes.position.getY(v);
                    const r = Math.sqrt(px*px + py*py);
                    const t = Math.max(0, Math.min(1, (r - layer.inner) / (layer.outer - layer.inner)));
                    const c0 = layer.colors[0], c1 = layer.colors[1];
                    colors[v*3]   = c0[0] + (c1[0] - c0[0]) * t;
                    colors[v*3+1] = c0[1] + (c1[1] - c0[1]) * t;
                    colors[v*3+2] = c0[2] + (c1[2] - c0[2]) * t;
                }
                rGeo.setAttribute('color', new THREE.BufferAttribute(colors, 3));
                const rMat = new THREE.MeshBasicMaterial({
                    vertexColors: true, transparent: true, opacity: layer.opacity,
                    side: THREE.DoubleSide, blending: THREE.AdditiveBlending, depthWrite: false
                });
                const ring = new THREE.Mesh(rGeo, rMat);
                ring.position.set(bh.x, bh.y, bh.z);
                ring.rotation.x = Math.PI * 0.5;
                ring.rotation.z = this._time * 0.08;
                this._group.add(ring); this._bhMeshes.push(ring);
            }

            // Inner glow — bright hot spot at center
            const glowGeo = new THREE.BufferGeometry();
            glowGeo.setAttribute('position', new THREE.BufferAttribute(new Float32Array([bh.x, bh.y, bh.z]), 3));
            const glow = new THREE.Points(glowGeo, new THREE.PointsMaterial({
                size: rs * 6, color: 0xffaa33, transparent: true, opacity: 0.4,
                blending: THREE.AdditiveBlending, depthWrite: false, sizeAttenuation: true,
                map: _circleTexture
            }));
            this._group.add(glow); this._bhMeshes.push(glow);

            // Outer halo — faint wide glow
            const haloGeo = new THREE.BufferGeometry();
            haloGeo.setAttribute('position', new THREE.BufferAttribute(new Float32Array([bh.x, bh.y, bh.z]), 3));
            const halo = new THREE.Points(haloGeo, new THREE.PointsMaterial({
                size: rs * 20, color: 0x4422aa, transparent: true, opacity: 0.12,
                blending: THREE.AdditiveBlending, depthWrite: false, sizeAttenuation: true,
                map: _circleTexture
            }));
            this._group.add(halo); this._bhMeshes.push(halo);
        }
    }

    setCameraPreset(name, bodyData) {
        const presets = {
            overview:  { pos: [0, 300, 400], target: [0, 0, 0], fov: 60 },
            galaxy:    { pos: [30, 50, 70],  target: [0, 0, 0], fov: 50 },
            blackhole: { pos: [0, 30, 60],   target: [0, 0, 0], fov: 45 },
            merger:    { pos: [0, 80, 160],  target: [0, 0, 0], fov: 55 },
            quasar:    { pos: [0, 20, 40],   target: [0, 0, 0], fov: 45 }
        };
        const p = presets[name] || presets.overview;
        this.camera.position.set(p.pos[0], p.pos[1], p.pos[2]);
        this.camera.lookAt(p.target[0], p.target[1], p.target[2]);
        if (p.fov && this.camera.fov !== p.fov) {
            this.camera.fov = p.fov;
            this.camera.updateProjectionMatrix();
        }
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
        for (const m of this._bhMeshes) { this._group.remove(m); m.geometry.dispose(); m.material.dispose(); }
        if (this._bgStars) { this._group.remove(this._bgStars); this._bgStars.geometry.dispose(); this._bgStars.material.dispose(); }
        this.scene.remove(this._group);
    }

    _rng(seed) {
        let s = seed;
        return () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
    }
}
