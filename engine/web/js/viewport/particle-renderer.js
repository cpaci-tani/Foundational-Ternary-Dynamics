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
import { K_B, C_SPEED } from '../constants.js';

// Pre-allocated buffer size — centralized in viewport/constants.js (D-6).
import { MAX_PARTICLES } from './constants.js';

// Custom particle shaders — centralized in viewport/shaders.js (D-1).
import { PARTICLE_VERT, PARTICLE_FRAG } from './shaders.js';


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
        this._peSystem = null;

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
            uniforms: { shapeType: { value: 0 }, uOpacity: { value: 0.9 }, uGlow: { value: 0.15 } },
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

            // Color by causal saturation β = |v| / c_lattice (c = 1/√3). The
            // ternary lattice caps signal speed at C_SPEED, so the ramp reads
            // off how close a particle sits to the FTD causal limit:
            //   green (slow) → yellow → orange → red → white (pinned at the cap).
            const speed = Math.sqrt(vx * vx + vy * vy + vz * vz);
            const beta = Math.min(speed / C_SPEED, 1.0);
            let r, g, b;
            if (beta < 0.5) {
                const t = beta / 0.5;            // green → yellow
                r = 0.25 + 0.75 * t; g = 0.90; b = 0.25;
            } else if (beta < 0.85) {
                const t = (beta - 0.5) / 0.35;   // yellow → orange
                r = 1.0; g = 0.90 - 0.55 * t; b = 0.20;
            } else if (beta < 0.985) {
                const t = (beta - 0.85) / 0.135; // orange → red
                r = 1.0; g = 0.35 - 0.35 * t; b = 0.20 - 0.20 * t;
            } else {
                const t = (beta - 0.985) / 0.015; // red → white (at the cap)
                r = 1.0; g = 0.90 * t; b = 0.90 * t;
            }
            // Tail dimmer, tip at full intensity → direction reads clearly.
            colAttr.array[i * 6] = r * 0.5;
            colAttr.array[i * 6 + 1] = g * 0.5;
            colAttr.array[i * 6 + 2] = b * 0.5;
            colAttr.array[i * 6 + 3] = r;
            colAttr.array[i * 6 + 4] = g;
            colAttr.array[i * 6 + 5] = b;
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

    // ── System Observables (center of mass + p + L) ───────────────────
    // A single LineSegments mesh carrying the system-level conserved
    // quantities: a center-of-mass cross plus two direction arrows for the
    // total momentum p (cyan) and the angular-momentum axis L (magenta).
    // These complement the per-particle/field overlays and the conservation
    // telemetry (|p|, |L| read out numerically in the Diagnostics panel).
    _buildPESystem() {
        const MAX_SEG = 8; // 3 cross + p + L, with headroom
        const vertices = new Float32Array(MAX_SEG * 2 * 3);
        const colors = new Float32Array(MAX_SEG * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.9,
        });
        this._peSystem = new THREE.LineSegments(geo, mat);
        this._peSystem.frustumCulled = false; // dynamic geo — see velocityVectors
        this._peSystem.visible = false;
        this._scene.add(this._peSystem);
    }

    // com/p/l are length-3 arrays [x,y,z]. p and L are drawn as fixed-length
    // direction arrows (orientation, not magnitude — the numeric values live
    // in telemetry); each collapses to a zero-length, invisible segment when
    // its magnitude is ~0 (e.g. p≈0 for a system at rest in its own frame,
    // or L≈0 for a single particle).
    updatePESystem(com, p, l) {
        if (!this._peSystem) this._buildPESystem();
        const posAttr = this._peSystem.geometry.getAttribute('position');
        const colAttr = this._peSystem.geometry.getAttribute('color');
        if (!com) { this._peSystem.geometry.setDrawRange(0, 0); return; }

        const cx = com[0], cy = com[1], cz = com[2];
        const CROSS = 2.0;   // half-length of the center-of-mass cross
        const ARROW = 10.0;  // visual length of the p / L direction arrows

        const setSeg = (s, x0, y0, z0, x1, y1, z1, tr, tg, tb, hr, hg, hb) => {
            posAttr.array[s * 6]     = x0; posAttr.array[s * 6 + 1] = y0; posAttr.array[s * 6 + 2] = z0;
            posAttr.array[s * 6 + 3] = x1; posAttr.array[s * 6 + 4] = y1; posAttr.array[s * 6 + 5] = z1;
            colAttr.array[s * 6]     = tr; colAttr.array[s * 6 + 1] = tg; colAttr.array[s * 6 + 2] = tb;
            colAttr.array[s * 6 + 3] = hr; colAttr.array[s * 6 + 4] = hg; colAttr.array[s * 6 + 5] = hb;
        };

        // Center-of-mass cross (3 axis-aligned segments, light gray).
        const gr = 0.80, gg = 0.85, gb = 0.95;
        setSeg(0, cx - CROSS, cy, cz, cx + CROSS, cy, cz, gr, gg, gb, gr, gg, gb);
        setSeg(1, cx, cy - CROSS, cz, cx, cy + CROSS, cz, gr, gg, gb, gr, gg, gb);
        setSeg(2, cx, cy, cz - CROSS, cx, cy, cz + CROSS, gr, gg, gb, gr, gg, gb);

        // Total momentum p (cyan), tail dim → tip bright.
        const pm = p ? Math.sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]) : 0;
        if (pm > 1e-9) {
            const ux = p[0] / pm, uy = p[1] / pm, uz = p[2] / pm;
            setSeg(3, cx, cy, cz, cx + ux * ARROW, cy + uy * ARROW, cz + uz * ARROW,
                0.13, 0.39, 0.50, 0.27, 0.78, 1.0);
        } else {
            setSeg(3, cx, cy, cz, cx, cy, cz, 0, 0, 0, 0, 0, 0);
        }

        // Angular-momentum axis L (magenta) — orbital-plane normal through CoM.
        const lm = l ? Math.sqrt(l[0] * l[0] + l[1] * l[1] + l[2] * l[2]) : 0;
        if (lm > 1e-9) {
            const ux = l[0] / lm, uy = l[1] / lm, uz = l[2] / lm;
            setSeg(4, cx, cy, cz, cx + ux * ARROW, cy + uy * ARROW, cz + uz * ARROW,
                0.42, 0.24, 0.47, 0.84, 0.48, 0.94);
        } else {
            setSeg(4, cx, cy, cz, cx, cy, cz, 0, 0, 0, 0, 0, 0);
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this._peSystem.geometry.setDrawRange(0, 5 * 2);
    }

    togglePESystem(on) {
        if (!this._peSystem) this._buildPESystem();
        this._peSystem.visible = on;
        if (!on) this._peSystem.geometry.setDrawRange(0, 0);
    }

    // ── Main per-frame particle update ────────────────────────────────
    updateParticles(data) {
        const geo = this.particles.geometry;
        const posAttr = geo.getAttribute('position');
        const colAttr = geo.getAttribute('particleColor');
        const sizeAttr = geo.getAttribute('size');

        const rawCount = Math.min(data.count, MAX_PARTICLES);

        // Clip particles to current boundary shape
        const _bs = this._getBoundaryShape();
        const needsClip = _bs && _bs !== 'none' && _bs !== 'cube';
        let count = 0;
        for (let i = 0; i < rawCount; i++) {
            const px = data.positions[i * 3];
            const py = data.positions[i * 3 + 1];
            const pz = data.positions[i * 3 + 2];
            if (needsClip) {
                const center = this._latticeSize / 2;
                const radius = this._latticeSize / 2;
                const nx = (px - center) / radius;
                const ny = (py - center) / radius;
                const nz = (pz - center) / radius;
                if (!this._insideBoundary(nx, ny, nz)) continue;
            }
            posAttr.array[count * 3] = px;
            posAttr.array[count * 3 + 1] = py;
            posAttr.array[count * 3 + 2] = pz;
            colAttr.array[count * 3] = data.colors[i * 3];
            colAttr.array[count * 3 + 1] = data.colors[i * 3 + 1];
            colAttr.array[count * 3 + 2] = data.colors[i * 3 + 2];
            // Per-polarity size: detect from color (green=+1, red=-1, blue=void)
            const cr = data.colors[i * 3], cg = data.colors[i * 3 + 1];
            let baseSize;
            if (cg > 0.6 && cr < 0.6) {
                baseSize = this.visualSettings.positiveSize ?? data.sizes[i];
            } else if (cr > 0.6 && cg < 0.6) {
                baseSize = this.visualSettings.negativeSize ?? data.sizes[i];
            } else {
                baseSize = data.sizes[i];
            }
            sizeAttr.array[count] = baseSize * this.visualSettings.globalScale;
            count++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;

        geo.setDrawRange(0, count);
    }

    // Dynamic reactive size update for sliders when paused/running
    updateParticleSizes() {
        if (!this.particles) return;
        const geo = this.particles.geometry;
        const colAttr = geo.getAttribute('particleColor');
        const sizeAttr = geo.getAttribute('size');
        if (!colAttr || !sizeAttr) return;

        const count = geo.drawRange.count;
        for (let i = 0; i < count; i++) {
            const cr = colAttr.array[i * 3], cg = colAttr.array[i * 3 + 1];
            let baseSize;
            if (cg > 0.6 && cr < 0.6) {
                baseSize = this.visualSettings.positiveSize;
            } else if (cr > 0.6 && cg < 0.6) {
                baseSize = this.visualSettings.negativeSize;
            } else {
                continue; // Skip void or other particles where size isn't managed by pos/neg size controls
            }
            sizeAttr.array[i] = baseSize * this.visualSettings.globalScale;
        }
        sizeAttr.needsUpdate = true;
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

    setGlow(val) {
        if (this.particles && this.particles.material.uniforms) {
            this.particles.material.uniforms.uGlow.value = val;
        }
        this.visualSettings.glowIntensity = val;
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
        disposeMesh(this._peSystem);

        this.particles = null;
        this.velocityVectors = null;
        this.trails = null;
        this._particleForces = null;
        this._peSystem = null;
    }
}
