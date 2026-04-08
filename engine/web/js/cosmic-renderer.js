/**
 * CosmicRenderer: Scale 5 photorealistic Three.js rendering
 *
 * Uses proven PointsMaterial (vertex colors) + MeshBasicMaterial for reliability.
 * Additive blending for stars/gas gives bloom-like glow without post-processing.
 */

import * as THREE from 'three';

// Body type constants (match CosmicBodyType enum)
const BT = {
    DARK_ENERGY: -3, QUASAR: -2, BLACK_HOLE: -1,
    DARK_MATTER: 0, GAS: 1, STAR: 2,
    NEUTRON_STAR: 3, NEBULA: 4, WHITE_DWARF: 5
};

// Blackbody color from temperature (Kelvin)
function blackbodyColor(T) {
    const t = Math.max(0, Math.min(1, (T - 1500) / 30000));
    if (t < 0.15) return [1.0, 0.3, 0.05];        // deep red (M dwarf)
    if (t < 0.3)  return [1.0, 0.55, 0.15];        // orange (K star)
    if (t < 0.45) return [1.0, 0.85, 0.4];         // yellow (G star / Sun)
    if (t < 0.6)  return [1.0, 0.95, 0.8];         // white-yellow (F star)
    if (t < 0.75) return [0.85, 0.9, 1.0];         // blue-white (A star)
    return [0.6, 0.7, 1.0];                         // blue (O/B star)
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

        // Point cloud objects
        this._starCloud = null;
        this._gasCloud = null;
        this._dmCloud = null;

        // Mesh objects
        this._bhSpheres = [];
        this._diskMeshes = [];
        this._bgStars = null;

        // Layer toggles
        this._showDM = true;
        this._showGas = true;
        this._showStars = true;
        this._showBH = true;
        this._showDisks = true;

        this._initBackground();
    }

    // ====================================================================
    // Background: thousands of distant stars on a large sphere
    // ====================================================================
    _initBackground() {
        this.scene.background = new THREE.Color(0x020208);

        const N = 6000;
        const pos = new Float32Array(N * 3);
        const col = new Float32Array(N * 3);
        const rng = this._rng(42);

        for (let i = 0; i < N; i++) {
            const theta = Math.acos(2 * rng() - 1);
            const phi = 2 * Math.PI * rng();
            const r = 4000 + rng() * 3000;
            pos[i*3]   = r * Math.sin(theta) * Math.cos(phi);
            pos[i*3+1] = r * Math.sin(theta) * Math.sin(phi);
            pos[i*3+2] = r * Math.cos(theta);

            const T = 2000 + rng() * 28000;
            const [cr, cg, cb] = blackbodyColor(T);
            const bright = 0.3 + rng() * 0.7;
            col[i*3] = cr * bright;
            col[i*3+1] = cg * bright;
            col[i*3+2] = cb * bright;
        }

        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.BufferAttribute(pos, 3));
        geo.setAttribute('color', new THREE.BufferAttribute(col, 3));

        this._bgStars = new THREE.Points(geo, new THREE.PointsMaterial({
            size: 1.5,
            vertexColors: true,
            sizeAttenuation: false,
            transparent: true,
            opacity: 0.85
        }));
        this._group.add(this._bgStars);
    }

    // ====================================================================
    // Ensure point cloud buffers exist with enough capacity
    // ====================================================================
    _ensureCloud(name, maxCount, size, opacity, blending, attenuation = true) {
        const key = '_' + name + 'Cloud';
        if (this[key] && this[key].geometry.attributes.position.count >= maxCount) return this[key];
        if (this[key]) { this._group.remove(this[key]); this[key].geometry.dispose(); this[key].material.dispose(); }

        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.BufferAttribute(new Float32Array(maxCount * 3), 3));
        geo.setAttribute('color', new THREE.BufferAttribute(new Float32Array(maxCount * 3), 3));
        geo.setDrawRange(0, 0);

        const mat = new THREE.PointsMaterial({
            size,
            vertexColors: true,
            transparent: true,
            opacity,
            blending,
            depthWrite: false,
            sizeAttenuation: attenuation
        });

        this[key] = new THREE.Points(geo, mat);
        this[key].name = 'cosmic-' + name;
        this._group.add(this[key]);
        return this[key];
    }

    // ====================================================================
    // Main update: sort bodies by type, update point clouds + meshes
    // ====================================================================
    update(bodyData, diagnostics) {
        if (!bodyData || bodyData.count === 0) return;
        this._time += 0.016;

        const { positions, types, temperatures, sizes, count } = bodyData;

        // Classify bodies
        const stars = [], gas = [], dm = [], bhs = [];
        for (let i = 0; i < count; i++) {
            const t = types[i];
            const entry = { i, x: positions[i*3], y: positions[i*3+1], z: positions[i*3+2] };
            if (t === BT.STAR || t === BT.WHITE_DWARF || t === BT.NEUTRON_STAR) stars.push(entry);
            else if (t === BT.GAS || t === BT.NEBULA) gas.push(entry);
            else if (t === BT.DARK_MATTER || t === BT.DARK_ENERGY) dm.push(entry);
            else if (t === BT.BLACK_HOLE || t === BT.QUASAR) bhs.push(entry);
        }

        // --- Stars: bright points with blackbody colors ---
        if (this._showStars && stars.length > 0) {
            const cloud = this._ensureCloud('star', Math.max(stars.length, 500), 6, 0.95, THREE.AdditiveBlending);
            const posA = cloud.geometry.attributes.position.array;
            const colA = cloud.geometry.attributes.color.array;
            for (let j = 0; j < stars.length; j++) {
                const s = stars[j];
                posA[j*3] = s.x; posA[j*3+1] = s.y; posA[j*3+2] = s.z;
                const T = temperatures ? temperatures[s.i] : 5800;
                const [r, g, b] = blackbodyColor(Math.max(T, 2000));
                const bright = 0.7 + Math.min(sizes ? sizes[s.i] * 0.05 : 0.3, 0.3);
                colA[j*3] = r*bright; colA[j*3+1] = g*bright; colA[j*3+2] = b*bright;
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            cloud.geometry.setDrawRange(0, stars.length);
            cloud.visible = true;
        } else if (this._starCloud) this._starCloud.visible = false;

        // --- Gas: warm diffuse glow ---
        if (this._showGas && gas.length > 0) {
            const cloud = this._ensureCloud('gas', Math.max(gas.length, 200), 14, 0.4, THREE.AdditiveBlending);
            const posA = cloud.geometry.attributes.position.array;
            const colA = cloud.geometry.attributes.color.array;
            for (let j = 0; j < gas.length; j++) {
                const g = gas[j];
                posA[j*3] = g.x; posA[j*3+1] = g.y; posA[j*3+2] = g.z;
                const T = temperatures ? temperatures[g.i] : 1e4;
                // Gas coloring: cool=blue, warm=pink, hot=white
                const t = Math.max(0, Math.min(1, Math.log10(T + 1) / 7));
                if (t < 0.4) {
                    colA[j*3] = 0.2; colA[j*3+1] = 0.3+t; colA[j*3+2] = 0.8;
                } else if (t < 0.7) {
                    colA[j*3] = 0.8; colA[j*3+1] = 0.3; colA[j*3+2] = 0.5;
                } else {
                    colA[j*3] = 1.0; colA[j*3+1] = 0.9; colA[j*3+2] = 0.7;
                }
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            cloud.geometry.setDrawRange(0, gas.length);
            cloud.visible = true;
        } else if (this._gasCloud) this._gasCloud.visible = false;

        // --- Dark matter: faint blue-purple haze ---
        if (this._showDM && dm.length > 0) {
            const cloud = this._ensureCloud('dm', Math.max(dm.length, 500), 8, 0.12, THREE.AdditiveBlending);
            const posA = cloud.geometry.attributes.position.array;
            const colA = cloud.geometry.attributes.color.array;
            for (let j = 0; j < dm.length; j++) {
                const d = dm[j];
                posA[j*3] = d.x; posA[j*3+1] = d.y; posA[j*3+2] = d.z;
                colA[j*3] = 0.25; colA[j*3+1] = 0.15; colA[j*3+2] = 0.5;
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            cloud.geometry.setDrawRange(0, dm.length);
            cloud.visible = true;
        } else if (this._dmCloud) this._dmCloud.visible = false;

        // --- Black holes: event horizon + accretion disk ---
        this._updateBlackHoles(bhs, bodyData);
    }

    // ====================================================================
    // Black holes: dark sphere + glowing accretion ring
    // ====================================================================
    _updateBlackHoles(bhs, bodyData) {
        // Remove old
        for (const m of this._bhSpheres) { this._group.remove(m); m.geometry.dispose(); m.material.dispose(); }
        for (const m of this._diskMeshes) { this._group.remove(m); m.geometry.dispose(); m.material.dispose(); }
        this._bhSpheres = [];
        this._diskMeshes = [];

        if (!this._showBH) return;

        for (const bh of bhs) {
            const mass = bodyData.sizes ? bodyData.sizes[bh.i] : 100;
            const rs = Math.max(1.0, Math.cbrt(mass) * 0.3); // Visual Schwarzschild radius

            // Event horizon: black sphere with faint dark purple edge
            const sGeo = new THREE.SphereGeometry(rs, 24, 24);
            const sMat = new THREE.MeshBasicMaterial({ color: 0x050008 });
            const sphere = new THREE.Mesh(sGeo, sMat);
            sphere.position.set(bh.x, bh.y, bh.z);
            this._group.add(sphere);
            this._bhSpheres.push(sphere);

            // Accretion disk: glowing ring
            if (this._showDisks) {
                const innerR = rs * 2.5;
                const outerR = rs * 12;
                const rGeo = new THREE.RingGeometry(innerR, outerR, 64, 4);
                // Color the ring: hot inner (white-yellow) to cool outer (red-orange)
                const colors = new Float32Array(rGeo.attributes.position.count * 3);
                for (let v = 0; v < rGeo.attributes.position.count; v++) {
                    const px = rGeo.attributes.position.getX(v);
                    const py = rGeo.attributes.position.getY(v);
                    const r = Math.sqrt(px*px + py*py);
                    const t = Math.max(0, Math.min(1, (r - innerR) / (outerR - innerR)));
                    // Inner=white-hot, outer=deep red
                    colors[v*3]   = 1.0;
                    colors[v*3+1] = 1.0 - t * 0.7;
                    colors[v*3+2] = 0.9 - t * 0.85;
                }
                rGeo.setAttribute('color', new THREE.BufferAttribute(colors, 3));

                const rMat = new THREE.MeshBasicMaterial({
                    vertexColors: true,
                    transparent: true,
                    opacity: 0.7,
                    side: THREE.DoubleSide,
                    blending: THREE.AdditiveBlending,
                    depthWrite: false
                });
                const ring = new THREE.Mesh(rGeo, rMat);
                ring.position.set(bh.x, bh.y, bh.z);
                // Tilt the disk slightly for visual interest
                ring.rotation.x = Math.PI * 0.45 + Math.sin(this._time * 0.3) * 0.02;
                ring.rotation.z = this._time * 0.1;
                this._group.add(ring);
                this._diskMeshes.push(ring);

                // Inner glow: bright point at BH center
                const glowGeo = new THREE.BufferGeometry();
                glowGeo.setAttribute('position', new THREE.BufferAttribute(new Float32Array([bh.x, bh.y, bh.z]), 3));
                const glowMat = new THREE.PointsMaterial({
                    size: rs * 4,
                    color: 0xff6600,
                    transparent: true,
                    opacity: 0.5,
                    blending: THREE.AdditiveBlending,
                    depthWrite: false,
                    sizeAttenuation: true
                });
                const glow = new THREE.Points(glowGeo, glowMat);
                this._group.add(glow);
                this._bhSpheres.push(glow); // cleanup with BH meshes
            }
        }
    }

    // ====================================================================
    // Camera presets
    // ====================================================================
    setCameraPreset(name, bodyData) {
        const presets = {
            overview:  { pos: [0, 400, 400], target: [0, 0, 0], fov: 60 },
            galaxy:    { pos: [40, 60, 80],  target: [0, 0, 0], fov: 50 },
            blackhole: { pos: [0, 50, 80],   target: [0, 0, 0], fov: 45 },
            merger:    { pos: [0, 100, 200],  target: [0, 0, 0], fov: 55 },
            quasar:    { pos: [0, 30, 50],    target: [0, 0, 0], fov: 45 }
        };
        const p = presets[name] || presets.overview;
        this.camera.position.set(p.pos[0], p.pos[1], p.pos[2]);
        this.camera.lookAt(p.target[0], p.target[1], p.target[2]);
        if (p.fov && this.camera.fov !== p.fov) {
            this.camera.fov = p.fov;
            this.camera.updateProjectionMatrix();
        }
    }

    // ====================================================================
    // Toggle visibility
    // ====================================================================
    toggleDarkMatter(on)     { this._showDM = on; }
    toggleGasClouds(on)      { this._showGas = on; }
    toggleStars(on)          { this._showStars = on; }
    toggleBlackHoles(on)     { this._showBH = on; }
    toggleAccretionDisks(on) { this._showDisks = on; }

    // ====================================================================
    // Cleanup
    // ====================================================================
    dispose() {
        if (this._starCloud) { this._group.remove(this._starCloud); this._starCloud.geometry.dispose(); this._starCloud.material.dispose(); }
        if (this._gasCloud) { this._group.remove(this._gasCloud); this._gasCloud.geometry.dispose(); this._gasCloud.material.dispose(); }
        if (this._dmCloud) { this._group.remove(this._dmCloud); this._dmCloud.geometry.dispose(); this._dmCloud.material.dispose(); }
        for (const m of this._bhSpheres) { this._group.remove(m); m.geometry.dispose(); m.material.dispose(); }
        for (const m of this._diskMeshes) { this._group.remove(m); m.geometry.dispose(); m.material.dispose(); }
        if (this._bgStars) { this._group.remove(this._bgStars); this._bgStars.geometry.dispose(); this._bgStars.material.dispose(); }
        this.scene.remove(this._group);
    }

    // ====================================================================
    // Utility
    // ====================================================================
    _rng(seed) {
        let s = seed;
        return () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
    }
}
