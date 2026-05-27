/**
 * @file engine/web/js/viewport/particle-renderer.js
 * @purpose Owns particle positions, trails, velocity vectors, per-particle
 *          force vectors for the Scale-0 lattice dashboard. One of 4
 *          sub-renderers extracted from the monolithic Viewport class
 *          in Phase 3 of the refactor sweep.
 * @consumers engine/web/js/viewport.js (composes this via constructor)
 * @contract CONTRACTS.md §2 (Capability Factory Contract — applies to
 *          any sub-renderer with onLatticeSizeChanged/dispose lifecycle)
 * @related ./flux-renderer.js (3b, sibling — Phase 3b precedent),
 *          ./scene-core.js (3a, future), ./field-renderer.js (3c, future,
 *          will own _writeArrowFieldIntoMesh which this module receives
 *          as a callback), ./REFACTOR_MAP.md (extraction guide)
 *
 * Phase 3d of the refactor sweep. Atom/bond/orbital rendering remains
 * delegated to MolecularRenderer (already extracted) via thin forwarders
 * on the Viewport orchestrator — those are NOT moved here.
 */

import * as THREE from 'three';
import { getById } from '../particle-catalog.js';
import { K_B } from '../constants.js';

// Pre-allocated buffer sizes. Particle buffer is fixed at init to avoid
// dynamic GPU reallocation; draw range controls visible count each frame.
const MAX_PARTICLES = 100000;

// Custom particle shaders (duplicated here so the module is self-contained).
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

import { PARTICLE_FRAG } from './shaders.js';


export class ViewportParticleRenderer {
    constructor({
        scene,
        latticeSize,
        halfN,
        insideBoundary,
        getBoundaryShape,
        visualSettings,
        writeArrowFieldIntoMesh,
    }) {
        this._scene = scene;
        this._latticeSize = latticeSize;
        this._halfN = halfN;
        this._insideBoundary = insideBoundary;
        // Live getter so boundary-shape changes on the orchestrator propagate
        // here without a setter call (mirrors the pre-extraction direct read
        // of `this._boundaryShape` in updateParticles' fast-path check).
        this._getBoundaryShape = getBoundaryShape || (() => 'cube');
        // visualSettings is shared with the Viewport orchestrator: both sides
        // hold a reference to the SAME object so opacity changes from
        // setOpacity propagate to readers on either side without explicit
        // syncing.
        this.visualSettings = visualSettings;
        this._writeArrowFieldIntoMesh = writeArrowFieldIntoMesh;

        // State owned by ParticleRenderer (moved verbatim from Viewport's
        // constructor + lazy builders).
        this.particles = null;
        this.velocityVectors = null;
        this.trails = null;
        this._particleForces = null;

        // Build the main particle Points mesh eagerly (mirrors the
        // pre-extraction behaviour of `this._initParticles()` being called
        // from Viewport's constructor).
        this._initParticles();
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
        this._scene.add(this.particles);
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
        this._scene.add(this.velocityVectors);
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
        this._scene.add(this.trails);
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
        this._scene.add(this._particleForces);
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

    // ── Main per-frame particle update ────────────────────────────────
    updateParticles(data) {
        const geo = this.particles.geometry;
        const posAttr = geo.getAttribute('position');
        const colAttr = geo.getAttribute('particleColor');
        const sizeAttr = geo.getAttribute('size');

        const rawCount = Math.min(data.count, MAX_PARTICLES);

        // Clip particles to current boundary shape
        const halfN = this._halfN;
        const _bs = this._getBoundaryShape();
        const needsClip = _bs && _bs !== 'none' && _bs !== 'cube';
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

    // ── Lifecycle ─────────────────────────────────────────────────────
    onLatticeSizeChanged(size, halfN) {
        // Particle, velocity-vectors, trails, and particle-forces buffers are
        // all sized by particle count caps (MAX_PARTICLES, MAX_VEC=200,
        // MAX_SEGMENTS=10000, MAX_PFORCES=200) — none scale with the lattice
        // dimension. So a lattice resize doesn't require rebuilding any of
        // these meshes.
        //
        // We DO need to refresh the cached lattice geometry used by
        // updateParticles' boundary-clip math, so subsequent frames clip
        // against the new half-lattice extent.
        this._latticeSize = size;
        this._halfN = halfN;
    }

    dispose() {
        const disposeMesh = (obj) => {
            if (!obj) return;
            this._scene.remove(obj);
            if (obj.geometry) obj.geometry.dispose();
            if (obj.material) {
                if (obj.material.map) obj.material.map.dispose();
                obj.material.dispose();
            }
        };

        disposeMesh(this.particles);
        disposeMesh(this.velocityVectors);
        disposeMesh(this.trails);
        disposeMesh(this._particleForces);

        this.particles = null;
        this.velocityVectors = null;
        this.trails = null;
        this._particleForces = null;
    }
}
