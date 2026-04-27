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
import { BaseRenderer } from './core/BaseRenderer.js';
import { makeStarSprite, makeGasSprite, makeHaloSprite } from './cosmic/sprites.js';
import { DISK_VERT, DISK_FRAG, JET_VERT, JET_FRAG, blackbodyColor } from './cosmic/shaders.js';

const BT = {
    DARK_ENERGY: -3, QUASAR: -2, BLACK_HOLE: -1,
    DARK_MATTER: 0, GAS: 1, STAR: 2,
    NEUTRON_STAR: 3, NEBULA: 4, WHITE_DWARF: 5
};

const _starTex = makeStarSprite();
const _gasTex = makeGasSprite();
const _haloTex = makeHaloSprite();

// ====================================================================
export class CosmicRenderer extends BaseRenderer {
    constructor(scene, camera, renderer) {
        super(scene, camera, renderer);
        this._time = 0;

        this._group.name = 'cosmic-layer';

        this._starCloud = null;
        this._gasCloud = null;
        this._dmCloud = null;
        this._nebulaCloud = null;   // populated lazily by _ensureCloud('nebula', …)
        this._bgStars = null;

        // Subclass-specific geometry teardown. Called by BaseRenderer.dispose()
        // (core/BaseRenderer.js:37). Idempotent: nulls each reference after
        // disposing so a re-entry can rebuild from a clean slate and a
        // second dispose() call no-ops instead of double-freeing.
        this._cleanGeometries = () => {
            const disposeCloud = (cloud) => {
                if (!cloud) return null;
                if (cloud.geometry) cloud.geometry.dispose();
                if (cloud.material) cloud.material.dispose();
                return null;
            };
            this._starCloud = disposeCloud(this._starCloud);
            this._gasCloud = disposeCloud(this._gasCloud);
            this._dmCloud = disposeCloud(this._dmCloud);
            this._nebulaCloud = disposeCloud(this._nebulaCloud);  // CR-H2 fix
            this._bgStars = disposeCloud(this._bgStars);

            // Black-hole meshes are created via `_group.add(sphere, ...)` and
            // tracked in `_bhMeshCache` keyed by body id. The cache.forEach
            // walk reaches every mesh in every bundle (sphere, corona, disk,
            // disk2, einstein, glow, jetUp, jetDown).
            const disposeBundle = (bundle) => {
                if (!bundle) return;
                Object.values(bundle).forEach(mesh => {
                    if (mesh && mesh.geometry) mesh.geometry.dispose();
                    if (mesh && mesh.material) mesh.material.dispose();
                });
            };
            if (this._bhMeshCache) {
                this._bhMeshCache.forEach(disposeBundle);
                this._bhMeshCache.clear();
            }
        }

        this._showDM = true;
        this._showGas = true;
        this._showStars = true;
        this._showBH = true;
        this._showDisks = true;
        this._bhAge = new Map(); // track when each BH first appeared (for fade-in)
        this._bhMeshCache = new Map(); // persistent mesh caching

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


    // ================================================================
    // Main update
    // ================================================================
    update(bodyData, diagnostics) {
        if (!bodyData || bodyData.count === 0) return;
        
        // Use the physics engine's time so visuals respect the speed slider (dt)
        // If not provided, fallback to standard real-time tick (but it should be passed from app)
        const simTime = diagnostics ? diagnostics.tick * 0.05 : this._time + 0.016;
        this._time = simTime;

        const { positions, types, temperatures, sizes, count } = bodyData;
        const stars = [], gas = [], nebulae = [], dm = [], bhs = [];

        for (let i = 0; i < count; i++) {
            const t = types[i];
            const bodyId = bodyData.ids ? bodyData.ids[i] : i;
            const e = { i, id: bodyId, x: positions[i*3], y: positions[i*3+1], z: positions[i*3+2] };
            if (t === BT.STAR || t === BT.WHITE_DWARF || t === BT.NEUTRON_STAR) stars.push(e);
            else if (t === BT.GAS) gas.push(e);
            else if (t === BT.NEBULA) nebulae.push(e);
            else if (t === BT.DARK_MATTER || t === BT.DARK_ENERGY) dm.push(e);
            else if (t === BT.BLACK_HOLE || t === BT.QUASAR) bhs.push(e);
        }

        // -- Stars: diffraction-spike sprites, blackbody colored --
        if (this._showStars && stars.length > 0) {
            const cloud = this._ensureCloud('star', Math.max(stars.length, 500), 2.5, 1.0, THREE.AdditiveBlending, _starTex);
            const p = cloud.geometry.attributes.position.array;
            const c = cloud.geometry.attributes.color.array;
            const ids = cloud.userData.ids;
            for (let j = 0; j < stars.length; j++) {
                const s = stars[j];
                p[j*3] = s.x; p[j*3+1] = s.y; p[j*3+2] = s.z;
                ids[j] = s.id;
                const T = temperatures ? temperatures[s.i] : 5800;
                const [r, g, b] = blackbodyColor(Math.max(T, 2000));
                const br = 0.7 + Math.min((sizes ? sizes[s.i] : 5) * 0.03, 0.3);

                // Fuel stage overlay: modulate color to show evolutionary state
                const fuelStage = bodyData.fuel_stages ? bodyData.fuel_stages[s.i] : 0;
                const fuelFrac = bodyData.fuel_fractions ? bodyData.fuel_fractions[s.i] : 1.0;
                if (fuelStage === 1) {
                    // Red giant: force reddish color, larger sprite
                    c[j*3] = 1.0 * br; c[j*3+1] = 0.3 * br; c[j*3+2] = 0.05 * br;
                } else if (fuelStage >= 2 && fuelStage <= 4) {
                    // Late burning: blue-white, pulsing slightly
                    const pulse = 0.9 + 0.1 * Math.sin(this._time * 5 + s.i);
                    c[j*3] = 0.6 * br * pulse; c[j*3+1] = 0.7 * br * pulse; c[j*3+2] = 1.0 * br * pulse;
                } else if (fuelStage >= 5) {
                    // Iron core / dying: dim, flickering
                    const flicker = 0.3 + 0.7 * Math.random();
                    c[j*3] = 0.8 * br * flicker; c[j*3+1] = 0.2 * br * flicker; c[j*3+2] = 0.1 * br * flicker;
                } else {
                    // Normal main sequence
                    c[j*3] = r * br; c[j*3+1] = g * br; c[j*3+2] = b * br;
                }
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
            const ids = cloud.userData.ids;
            for (let j = 0; j < gas.length; j++) {
                const g = gas[j];
                p[j*3] = g.x; p[j*3+1] = g.y; p[j*3+2] = g.z;
                ids[j] = g.id;
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

        // -- Nebulae: giant structured dust clouds --
        if (this._showGas && nebulae.length > 0) {
            const cloud = this._ensureCloud('nebula', Math.max(nebulae.length, 600), 25.0, 0.20, THREE.AdditiveBlending, _gasTex, true);
            const p = cloud.geometry.attributes.position.array;
            const c = cloud.geometry.attributes.color.array;
            const ids = cloud.userData.ids;
            const s = cloud.geometry.attributes.size ? cloud.geometry.attributes.size.array : null;
            // The orientation angle for horizontal elongation
            const a = cloud.geometry.attributes.angle ? cloud.geometry.attributes.angle.array : null;
            // Radial distance to center
            const rad = cloud.geometry.attributes.radius ? cloud.geometry.attributes.radius.array : null;
            
            for (let j = 0; j < nebulae.length; j++) {
                const n = nebulae[j];
                p[j*3] = n.x; p[j*3+1] = n.y; p[j*3+2] = n.z;
                ids[j] = n.id;
                
                if (s) {
                    s[j] = sizes ? sizes[n.i] : 25.0; // Custom radii
                }
                if (a) {
                    // Orbital tangent is perpendicular to the radial vector
                    a[j] = Math.atan2(n.z, n.x); 
                }
                if (rad) {
                    rad[j] = Math.max(0.1, Math.sqrt(n.x*n.x + n.y*n.y + n.z*n.z)); // Export radial distance
                }
                
                // Deep space dust colors: crimson, dark purple, and gold
                const T = temperatures ? temperatures[n.i] : 4000;
                const t = Math.max(0, Math.min(1, Math.log10(T + 1) / 5));
                if (t < 0.4)      { c[j*3] = 0.35; c[j*3+1] = 0.05; c[j*3+2] = 0.15; }
                else if (t < 0.7) { c[j*3] = 0.2; c[j*3+1] = 0.1; c[j*3+2] = 0.3; }
                else              { c[j*3] = 0.4; c[j*3+1] = 0.2; c[j*3+2] = 0.05; }
            }
            cloud.geometry.attributes.position.needsUpdate = true;
            cloud.geometry.attributes.color.needsUpdate = true;
            if (s) cloud.geometry.attributes.size.needsUpdate = true;
            if (a) cloud.geometry.attributes.angle.needsUpdate = true;
            cloud.geometry.setDrawRange(0, nebulae.length);
            cloud.visible = true;
        } else if (this._nebulaCloud) this._nebulaCloud.visible = false;

        // -- Dark matter: ultra-faint violet revealing structure --
        if (this._showDM && dm.length > 0) {
            const cloud = this._ensureCloud('dm', Math.max(dm.length, 500), 4.0, 0.06, THREE.AdditiveBlending, _gasTex);
            const p = cloud.geometry.attributes.position.array;
            const c = cloud.geometry.attributes.color.array;
            const ids = cloud.userData.ids;
            for (let j = 0; j < dm.length; j++) {
                const d = dm[j];
                p[j*3] = d.x; p[j*3+1] = d.y; p[j*3+2] = d.z;
                ids[j] = d.id;
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
    // ================================================================
    // Black holes: event horizon + shader accretion disk + photon ring
    // ================================================================
    _updateBlackHoles(bhs, bodyData) {
        if (!this._bhMeshCache) this._bhMeshCache = new Map();
        
        // Track alive IDs to prune dead/merged black holes
        const aliveSet = new Set();
        for (const bh of bhs) aliveSet.add(bh.id);
        
        for (const [id, bundle] of this._bhMeshCache.entries()) {
            if (!aliveSet.has(id)) {
                for (const m of bundle.meshes) {
                    this._group.remove(m);
                    if (m.geometry) m.geometry.dispose();
                    if (m.material) {
                        if (m.material.map) m.material.map = null;
                        m.material.dispose();
                    }
                }
                this._bhMeshCache.delete(id);
                this._bhAge.delete(id);
            }
        }

        if (!this._showBH) {
            for (const bundle of this._bhMeshCache.values()) {
                for (const m of bundle.meshes) m.visible = false;
            }
            return;
        }

        for (const bh of bhs) {
            const mass = bodyData.sizes ? bodyData.sizes[bh.i] : 100;
            const rs = Math.max(0.6, Math.cbrt(mass) * 0.35);
            const bhPos = new THREE.Vector3(bh.x, bh.y, bh.z);

            if (!this._bhAge.has(bh.id)) this._bhAge.set(bh.id, this._time);
            const age = this._time - this._bhAge.get(bh.id);
            const fadeIn = Math.min(1.0, age / 5.0); 
            const growFactor = Math.min(1.0, age / 10.0); 
            const easeGrow = growFactor * growFactor * (3 - 2 * growFactor); 
            const innerR = rs * 1.05;
            const outerR = rs * (1.5 + 7.5 * easeGrow); // Toned down from 12.5

            let bundle = this._bhMeshCache.get(bh.id);
            
            if (!bundle) {
                // Construct unit-scale geometries exactly ONCE per Black Hole
                const sGeo = new THREE.SphereGeometry(1.0, 48, 48);
                const sMat = new THREE.MeshBasicMaterial({ color: 0x000000 });
                const sphere = new THREE.Mesh(sGeo, sMat);
                sphere.userData.id = bh.id;
                
                const coronaGeo = new THREE.SphereGeometry(1.15, 32, 32);
                const coronaMat = new THREE.MeshBasicMaterial({
                    color: 0x331100, transparent: true, opacity: 0.0,
                    blending: THREE.AdditiveBlending, depthWrite: false, side: THREE.BackSide
                });
                const corona = new THREE.Mesh(coronaGeo, coronaMat);

                // Use maximum max bounds for RingGeometry so it doesn't clip when expanded
                const diskGeo = new THREE.RingGeometry(1.05, 14.0, 128, 8);
                const diskMat = new THREE.ShaderMaterial({
                    vertexShader: DISK_VERT, fragmentShader: DISK_FRAG,
                    uniforms: {
                        time: { value: this._time },
                        innerRadius: { value: innerR },
                        outerRadius: { value: outerR },
                        bhPosition: { value: new THREE.Vector3() },
                        opacity: { value: 0 }
                    },
                    transparent: true, side: THREE.DoubleSide, blending: THREE.AdditiveBlending, depthWrite: false
                });
                const disk = new THREE.Mesh(diskGeo, diskMat);
                disk.rotation.x = Math.PI * 0.5;

                const disk2Geo = new THREE.RingGeometry(1.1, 10.0, 96, 4);
                const disk2Mat = new THREE.ShaderMaterial({
                    vertexShader: DISK_VERT, fragmentShader: DISK_FRAG,
                    uniforms: {
                        time: { value: this._time * 1.3 },
                        innerRadius: { value: innerR * 1.1 },
                        outerRadius: { value: outerR * 0.7 },
                        bhPosition: { value: new THREE.Vector3() },
                        opacity: { value: 0 }
                    },
                    transparent: true, side: THREE.DoubleSide, blending: THREE.AdditiveBlending, depthWrite: false
                });
                const disk2 = new THREE.Mesh(disk2Geo, disk2Mat);
                disk2.rotation.x = Math.PI * 0.5 + 0.08;
                disk2.rotation.z = 0.5;

                const ptsGeo = new THREE.BufferGeometry();
                ptsGeo.setAttribute('position', new THREE.BufferAttribute(new Float32Array([0,0,0]), 3));
                const einstein = new THREE.Points(ptsGeo, new THREE.PointsMaterial({
                    size: 1.0, color: 0x5533aa, transparent: true, opacity: 0.0,
                    blending: THREE.AdditiveBlending, depthWrite: false, sizeAttenuation: true, map: _haloTex
                }));

                const glow = new THREE.Points(ptsGeo, new THREE.PointsMaterial({
                    size: 1.0, color: 0xffaa44, transparent: true, opacity: 0.0,
                    blending: THREE.AdditiveBlending, depthWrite: false, sizeAttenuation: true, map: _haloTex
                }));

                const jetGeo = new THREE.CylinderGeometry(0.3, 0.3, 1.0, 16, 1, true);
                jetGeo.translate(0, 0.5, 0); // Base at the origin
                const jetMat = new THREE.ShaderMaterial({
                    vertexShader: JET_VERT, fragmentShader: JET_FRAG,
                    uniforms: { time: { value: this._time }, intensity: { value: 0 } },
                    transparent: true, side: THREE.DoubleSide, blending: THREE.AdditiveBlending, depthWrite: false
                });
                const jetUp = new THREE.Mesh(jetGeo, jetMat);
                const jetDown = new THREE.Mesh(jetGeo, jetMat.clone());
                jetDown.rotation.x = Math.PI;

                // Add to scene graph
                this._group.add(sphere, corona, disk, disk2, einstein, glow, jetUp, jetDown);
                
                bundle = { 
                    sphere, corona, disk, disk2, einstein, glow, jetUp, jetDown,
                    meshes: [sphere, corona, disk, disk2, einstein, glow, jetUp, jetDown] 
                };
                this._bhMeshCache.set(bh.id, bundle);
            }

            // Real-time Update Path (Zero Allocations)
            for (const m of bundle.meshes) m.visible = true;
            
            // Sync positions (Points meshes use absolute buffer pos, regular meshes use obj pos)
            bundle.sphere.position.copy(bhPos);
            bundle.corona.position.copy(bhPos);
            bundle.disk.position.copy(bhPos);
            bundle.disk2.position.copy(bhPos);
            bundle.einstein.geometry.attributes.position.array[0] = bhPos.x;
            bundle.einstein.geometry.attributes.position.array[1] = bhPos.y;
            bundle.einstein.geometry.attributes.position.array[2] = bhPos.z;
            bundle.einstein.geometry.attributes.position.needsUpdate = true;
            bundle.glow.geometry.attributes.position.array[0] = bhPos.x;
            bundle.glow.geometry.attributes.position.array[1] = bhPos.y;
            bundle.glow.geometry.attributes.position.array[2] = bhPos.z;
            bundle.glow.geometry.attributes.position.needsUpdate = true;
            bundle.jetUp.position.copy(bhPos);
            bundle.jetDown.position.copy(bhPos);
            bundle.glow.geometry.attributes.position.array[2] = bhPos.z;
            bundle.glow.geometry.attributes.position.needsUpdate = true;

            // Sync scales referencing `rs`
            bundle.sphere.scale.setScalar(rs);
            bundle.corona.scale.setScalar(rs);
            bundle.disk.scale.setScalar(rs);
            bundle.disk2.scale.setScalar(rs);

            // Sync shader uniforms
            if (this._showDisks) {
                bundle.disk.visible = true;
                bundle.disk2.visible = true;
                
                bundle.disk.material.uniforms.time.value = this._time;
                bundle.disk.material.uniforms.innerRadius.value = innerR;
                bundle.disk.material.uniforms.outerRadius.value = outerR;
                bundle.disk.material.uniforms.bhPosition.value.copy(bhPos);
                bundle.disk.material.uniforms.opacity.value = fadeIn * 0.75;
                
                bundle.disk2.material.uniforms.time.value = this._time * 1.3;
                bundle.disk2.material.uniforms.innerRadius.value = innerR * 1.1;
                bundle.disk2.material.uniforms.outerRadius.value = outerR * 0.7;
                bundle.disk2.material.uniforms.bhPosition.value.copy(bhPos);
                bundle.disk2.material.uniforms.opacity.value = fadeIn * 0.45;
            } else {
                bundle.disk.visible = false;
                bundle.disk2.visible = false;
            }

            // Sync procedural material values
            bundle.corona.material.opacity = 0.20 * fadeIn;
            bundle.einstein.material.opacity = 0.05 * fadeIn;
            bundle.einstein.material.size = rs * (4 + 14 * easeGrow); // Toned down from 24
            bundle.glow.material.opacity = 0.25 * fadeIn;
            bundle.glow.material.size = rs * (1 + 3 * easeGrow);

            // Fetch procedural jet intensity (packed loosely in luminosities array)
            const powerLevel = bodyData.luminosities ? bodyData.luminosities[bh.i] : 0;
            
            // Smoothly interpolate the rendering intensity so it draws out and fades organically
            bundle.jetIntensity = bundle.jetIntensity || 0;
            bundle.jetIntensity += (powerLevel - bundle.jetIntensity) * 0.05;
            
            const jetRenderIntensity = Math.min(bundle.jetIntensity * 0.05, 1.5) * fadeIn;
            
            // Jet dynamic drawing/flicker
            bundle.jetUp.material.uniforms.time.value = this._time;
            bundle.jetUp.material.uniforms.intensity.value = jetRenderIntensity;
            bundle.jetUp.scale.set(rs * 1.5, rs * (3 + jetRenderIntensity * 15), rs * 1.5);
            
            bundle.jetDown.material.uniforms.time.value = this._time;
            bundle.jetDown.material.uniforms.intensity.value = jetRenderIntensity;
            bundle.jetDown.scale.set(rs * 1.5, rs * (3 + jetRenderIntensity * 15), rs * 1.5);
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

    _ensureCloud(name, maxCount, defaultSize, opacity, blending, map, useSizes = false) {
        let cloud = name === 'star' ? this._starCloud : 
                    name === 'gas' ? this._gasCloud : 
                    name === 'nebula' ? this._nebulaCloud :
                    this._dmCloud;
                    
        if (!cloud || cloud.geometry.attributes.position.count < maxCount) {
            if (cloud) { this._group.remove(cloud); cloud.geometry.dispose(); cloud.material.dispose(); }
            
            const g = new THREE.BufferGeometry();
            g.setAttribute('position', new THREE.BufferAttribute(new Float32Array(maxCount * 3), 3));
            g.setAttribute('color', new THREE.BufferAttribute(new Float32Array(maxCount * 3), 3));
            
            let mat;
            // If the buffer geometry needs variable per-particle sizes, we must use a custom ShaderMaterial
            // because THREE.PointsMaterial size element is uniform unless heavily modified
            if (useSizes) {
                g.setAttribute('size', new THREE.BufferAttribute(new Float32Array(maxCount), 1));
                g.setAttribute('angle', new THREE.BufferAttribute(new Float32Array(maxCount), 1));
                g.setAttribute('radius', new THREE.BufferAttribute(new Float32Array(maxCount), 1));
                mat = new THREE.ShaderMaterial({
                    uniforms: {
                        color: { value: new THREE.Color(0xffffff) },
                        pointTexture: { value: map },
                        globalOpacity: { value: opacity }
                    },
                    vertexShader: `
                        attribute float size;
                        attribute float angle;
                        attribute float radius;
                        varying vec3 vColor;
                        varying float vAngle;
                        varying float vRad;
                        void main() {
                            vColor = color;
                            vAngle = angle;
                            vRad = radius;
                            vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
                            gl_PointSize = size * (300.0 / -mvPosition.z);
                            gl_Position = projectionMatrix * mvPosition;
                        }
                    `,
                    fragmentShader: `
                        uniform sampler2D pointTexture;
                        uniform float globalOpacity;
                        varying vec3 vColor;
                        varying float vAngle;
                        varying float vRad;
                        void main() {
                            vec2 uv = gl_PointCoord;
                            
                            // 1. Shift origin to center
                            uv -= 0.5;
                            
                            // 2. Rotate to orbital tangent
                            float c = cos(vAngle);
                            float s = sin(vAngle);
                            mat2 rMat = mat2(c, -s, s, c);
                            uv = rMat * uv;
                            
                            // 3. Dynamic spaghettification based on radial distance
                            float stretch = clamp(vRad / 30.0, 0.15, 0.8); // Relaxed extreme compression
                            uv.x *= stretch;
                            
                            // 4. Shift back
                            uv += 0.5;
                            
                            // Smoothly fade texture sampling at boundaries based purely on UV map radius
                            float distFromCenter = length(uv - 0.5);
                            float edgeFade = smoothstep(0.48, 0.20, distFromCenter);
                            
                            vec4 texColor = texture2D(pointTexture, gl_PointCoord); // Use original for the sprite, apply fade via alpha
                            gl_FragColor = vec4(vColor, globalOpacity * edgeFade) * texColor;
                        }
                    `,
                    blending: blending,
                    depthWrite: false,
                    depthTest: false,
                    transparent: true,
                    vertexColors: true
                });
            } else {
                mat = new THREE.PointsMaterial({
                    size: defaultSize,
                    map: map,
                    blending: blending,
                    depthWrite: false,
                    transparent: true,
                    opacity: opacity,
                    vertexColors: true,
                    sizeAttenuation: true
                });
            }
            
            cloud = new THREE.Points(g, mat);
            cloud.frustumCulled = false;
            cloud.name = 'cosmic-' + name;
            cloud.userData.ids = new Int32Array(maxCount);
            this._group.add(cloud);
            if (name === 'star') this._starCloud = cloud;
            else if (name === 'gas') this._gasCloud = cloud;
            else if (name === 'nebula') this._nebulaCloud = cloud;
            else this._dmCloud = cloud;
        }
        return cloud;
    }

    getInteractables() {
        const arr = [];
        if (this._starCloud && this._showStars) arr.push(this._starCloud);
        if (this._gasCloud && this._showGas) arr.push(this._gasCloud);
        if (this._nebulaCloud && this._showGas) arr.push(this._nebulaCloud);
        if (this._dmCloud && this._showDM) arr.push(this._dmCloud);
        if (this._bhMeshCache && this._showBH) {
            for (const bundle of this._bhMeshCache.values()) {
                if (bundle.meshes && bundle.meshes.length > 0) {
                    arr.push(bundle.meshes[0]); // sphere with userData.id
                }
            }
        }
        return arr;
    }

    _rng(seed) {
        let s = seed;
        return () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
    }
}
