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
import { MAX_PARTICLES, PE_VIS_BOUNDARY_R } from './constants.js';

// Custom particle shaders — centralized in viewport/shaders.js (D-1).
import { PARTICLE_VERT, PARTICLE_FRAG, PARTICLE_SHADER_UNIFORMS } from './shaders.js';

// SU(3)-labeled color-charge palette, keyed off the real genesis-assigned
// Voxel::color field (0=colorless, 1=red, 2=green, 3=blue). Deliberately
// distinct from particle-catalog.js's display_color (a fixed per-species
// branding color) — this is a per-instance physical label, not a species
// identity. FTD 'color' is a C3-symmetric discrete axis label, not SU(3)
// gauge charge (LEDGER FTD-0077); the toggle name says "color charge" to
// match the physics term while the tooltip carries that caveat.
const COLOR_CHARGE_PALETTE = [
    [0.55, 0.55, 0.55], // 0: colorless
    [0.90, 0.25, 0.25], // 1: red
    [0.25, 0.85, 0.35], // 2: green
    [0.30, 0.45, 0.95], // 3: blue
];


export class ViewportParticleRenderer {
    constructor({
        scene,
        latticeSize,
        halfN,
        insideBoundary,
        getBoundaryShape,
        getBoundaryMode,
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
        this._getBoundaryMode = getBoundaryMode || (() => 'lattice');
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
        this.spinVectors = null;
        this.trails = null;
        this._particleForces = null;
        this._peSystem = null;
        this._voxelDebug = null;      // Scale-1 promotion-source ghost layer
        this._voxelDebugWarned = false;

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
        const manifestPhases = new Float32Array(MAX_PARTICLES);
        const manifestRates = new Float32Array(MAX_PARTICLES);

        const posAttr = new THREE.BufferAttribute(positions, 3);
        const colAttr = new THREE.BufferAttribute(colors, 3);
        const sizeAttr = new THREE.BufferAttribute(sizes, 1);
        const phaseAttr = new THREE.BufferAttribute(manifestPhases, 1);
        const rateAttr = new THREE.BufferAttribute(manifestRates, 1);
        posAttr.setUsage(THREE.DynamicDrawUsage);
        colAttr.setUsage(THREE.DynamicDrawUsage);
        sizeAttr.setUsage(THREE.DynamicDrawUsage);
        phaseAttr.setUsage(THREE.DynamicDrawUsage);
        rateAttr.setUsage(THREE.DynamicDrawUsage);
        geometry.setAttribute('position', posAttr);
        geometry.setAttribute('particleColor', colAttr);
        geometry.setAttribute('size', sizeAttr);
        geometry.setAttribute('manifestPhase', phaseAttr);
        geometry.setAttribute('manifestRate', rateAttr);
        geometry.setDrawRange(0, 0);

        const material = new THREE.ShaderMaterial({
            uniforms: {
                ...PARTICLE_SHADER_UNIFORMS,
                uOpacity: { value: 0.9 },
                uGlow: { value: 0.15 },
            },
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

    // ── Voxel debug ghost layer (Scale-1 promotion source view) ─────────
    // A SEPARATE small Points mesh — never multiplexed onto the shared
    // particle mesh, whose attributes updateParticles() owns. Shows the
    // per-voxel coarsenToParticles snapshot behind the promoted cluster
    // particles ([IMPOSED] display layer; static snapshot, no per-frame cost).
    _buildVoxelDebugLayer() {
        const MAX_VOXEL_GHOSTS = 50000;
        const geo = new THREE.BufferGeometry();
        const pos = new THREE.BufferAttribute(new Float32Array(MAX_VOXEL_GHOSTS * 3), 3);
        const col = new THREE.BufferAttribute(new Float32Array(MAX_VOXEL_GHOSTS * 3), 3);
        pos.setUsage(THREE.DynamicDrawUsage);
        col.setUsage(THREE.DynamicDrawUsage);
        geo.setAttribute('position', pos);
        geo.setAttribute('color', col);
        geo.setDrawRange(0, 0);
        const mat = new THREE.PointsMaterial({
            size: 1.4, vertexColors: true, transparent: true, opacity: 0.35,
            depthWrite: false, sizeAttenuation: true,
        });
        this._voxelDebug = new THREE.Points(geo, mat);
        this._voxelDebug.visible = false;
        this._voxelDebug.frustumCulled = false;
        this._scene.add(this._voxelDebug);
        this._voxelDebugMax = MAX_VOXEL_GHOSTS;
    }

    /**
     * Fill the ghost layer from a coarsenToParticles snapshot.
     * Positions are lattice coords; mapped to the PE origin frame with the
     * same center/scale the promotion mapping used.
     */
    updateVoxelDebugLayer(coarsen, latticeSize, displayScale = 1) {
        if (!coarsen || !coarsen.count) { this.toggleVoxelDebugLayer(false); return; }
        if (!this._voxelDebug) this._buildVoxelDebugLayer();
        const geo = this._voxelDebug.geometry;
        const pos = geo.getAttribute('position');
        const col = geo.getAttribute('color');
        const center = (latticeSize - 1) / 2;
        let n = coarsen.count;
        if (n > this._voxelDebugMax) {
            if (!this._voxelDebugWarned) {
                console.warn(`[Viewport] voxel debug layer clamped to ${this._voxelDebugMax}`
                    + ` of ${n} voxel records`);
                this._voxelDebugWarned = true;
            }
            n = this._voxelDebugMax;
        }
        for (let i = 0; i < n; i++) {
            pos.array[i * 3] = (coarsen.positions[i * 3] - center) * displayScale;
            pos.array[i * 3 + 1] = (coarsen.positions[i * 3 + 1] - center) * displayScale;
            pos.array[i * 3 + 2] = (coarsen.positions[i * 3 + 2] - center) * displayScale;
            if (coarsen.charges[i] > 0) {
                col.array[i * 3] = 0.22; col.array[i * 3 + 1] = 0.55; col.array[i * 3 + 2] = 0.33;
            } else {
                col.array[i * 3] = 0.58; col.array[i * 3 + 1] = 0.28; col.array[i * 3 + 2] = 0.28;
            }
        }
        pos.needsUpdate = true;
        col.needsUpdate = true;
        geo.setDrawRange(0, n);
    }

    toggleVoxelDebugLayer(on) {
        if (!this._voxelDebug) {
            if (!on) return;
            this._buildVoxelDebugLayer();
        }
        this._voxelDebug.visible = !!on;
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
        const scale = 40; // world-units per unit velocity (β-colored below)

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

    // ── Spin axis vectors (PE mode) ───────────────────────────────────
    _buildSpinVectors() {
        const MAX_VEC = 200;
        const vertices = new Float32Array(MAX_VEC * 2 * 3);
        const colors = new Float32Array(MAX_VEC * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.75,
        });
        this.spinVectors = new THREE.LineSegments(geo, mat);
        this.spinVectors.frustumCulled = false;
        this.spinVectors.visible = false;
        this._scene.add(this.spinVectors);
    }

    updateSpinVectors(positions, spinAxes, spins, count) {
        if (!this.spinVectors) this._buildSpinVectors();
        if (!spinAxes || !spins) return;

        const posAttr = this.spinVectors.geometry.getAttribute('position');
        const colAttr = this.spinVectors.geometry.getAttribute('color');
        const maxLines = posAttr.array.length / 6;
        const n = Math.min(count, maxLines);
        const scale = 2.8;
        let drawn = 0;

        for (let i = 0; i < n; i++) {
            const s = spins[i];
            if (!s) continue;
            const i3 = i * 3;
            let sx = spinAxes[i3];
            let sy = spinAxes[i3 + 1];
            let sz = spinAxes[i3 + 2];
            let smag = Math.sqrt(sx * sx + sy * sy + sz * sz);
            if (smag < 1e-8) {
                sz = s > 0 ? 1 : -1;
                sx = 0;
                sy = 0;
                smag = 1;
            }
            const len = scale * smag;
            const ux = sx / smag;
            const uy = sy / smag;
            const uz = sz / smag;

            const px = positions[i3];
            const py = positions[i3 + 1];
            const pz = positions[i3 + 2];
            const li = drawn;

            posAttr.array[li * 6] = px;
            posAttr.array[li * 6 + 1] = py;
            posAttr.array[li * 6 + 2] = pz;
            posAttr.array[li * 6 + 3] = px + ux * len;
            posAttr.array[li * 6 + 4] = py + uy * len;
            posAttr.array[li * 6 + 5] = pz + uz * len;

            const r = 0.72;
            const g = 0.45;
            const b = 0.95;
            colAttr.array[li * 6] = r * 0.45;
            colAttr.array[li * 6 + 1] = g * 0.45;
            colAttr.array[li * 6 + 2] = b * 0.45;
            colAttr.array[li * 6 + 3] = r;
            colAttr.array[li * 6 + 4] = g;
            colAttr.array[li * 6 + 5] = b;
            drawn++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this.spinVectors.geometry.setDrawRange(0, drawn * 2);
    }

    toggleSpinVectors(on) {
        if (!this.spinVectors) this._buildSpinVectors();
        this.spinVectors.visible = on;
        if (!on) this.spinVectors.geometry.setDrawRange(0, 0);
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
            const maxLen = 200;
            const start = trail.length < maxLen ? 0 : trail.head;
            const speeds = trail.speeds;

            for (let j = 0; j < len - 1 && seg < maxSegments; j++) {
                const idx0 = (start + j) % maxLen;
                const idx1 = (start + j + 1) % maxLen;

                posAttr.array[seg * 6] = trail.positions[idx0 * 3];
                posAttr.array[seg * 6 + 1] = trail.positions[idx0 * 3 + 1];
                posAttr.array[seg * 6 + 2] = trail.positions[idx0 * 3 + 2];
                posAttr.array[seg * 6 + 3] = trail.positions[idx1 * 3];
                posAttr.array[seg * 6 + 4] = trail.positions[idx1 * 3 + 1];
                posAttr.array[seg * 6 + 5] = trail.positions[idx1 * 3 + 2];

                const fade = (j + 1) / len;
                const spd = speeds ? (speeds[idx1] || 0) : 0;
                const beta = Math.min(spd / C_SPEED, 1.0);
                const speedBoost = 0.65 + 0.35 * beta;
                colAttr.array[seg * 6] = cr * fade * 0.7 * speedBoost;
                colAttr.array[seg * 6 + 1] = cg * fade * 0.7 * speedBoost;
                colAttr.array[seg * 6 + 2] = cb * fade * 0.7 * speedBoost;
                colAttr.array[seg * 6 + 3] = cr * fade * speedBoost;
                colAttr.array[seg * 6 + 4] = cg * fade * speedBoost;
                colAttr.array[seg * 6 + 5] = cb * fade * speedBoost;

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

    // ── Per-Particle Force Arrows (decomposed: Coulomb / gravity / strong / net) ──
    _buildPEForceArrows() {
        const MAX = 200;
        const makeSet = (color, dashed = false) => {
            const vertices = new Float32Array(MAX * 6);
            const geo = new THREE.BufferGeometry();
            geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
            geo.setDrawRange(0, 0);
            const mat = dashed
                ? new THREE.LineDashedMaterial({
                    color, transparent: true, opacity: 0.9, dashSize: 1.5, gapSize: 1.0,
                  })
                : new THREE.LineBasicMaterial({
                    color, transparent: true, opacity: 0.85,
                  });
            const lines = new THREE.LineSegments(geo, mat);
            lines.frustumCulled = false;
            lines.visible = false;
            this._scene.add(lines);
            return lines;
        };
        this._peForceCoulomb = makeSet(0xff4444);
        this._peForceGravity = makeSet(0x94a3b8);
        this._peForceStrong = makeSet(0xff1744, /* dashed */ true);
        this._peForceNet = makeSet(0x44cc66);
        // Legacy alias — net force layer
        this._particleForces = this._peForceNet;
    }

    _updatePEForceArrowSet(lines, positions, forces, count, maxForce, visGain = 1.0) {
        if (!lines) return;
        const posAttr = lines.geometry.getAttribute('position');
        const n = Math.min(count, 200);
        const arrowScale = 12.0;
        for (let i = 0; i < n; i++) {
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            const fx = forces[i * 3] * visGain;
            const fy = forces[i * 3 + 1] * visGain;
            const fz = forces[i * 3 + 2] * visGain;
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            posAttr.array[i * 6] = px;
            posAttr.array[i * 6 + 1] = py;
            posAttr.array[i * 6 + 2] = pz;
            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce * visGain + 1e-20) * 10) : 0;
            posAttr.array[i * 6 + 3] = px + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[i * 6 + 4] = py + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[i * 6 + 5] = pz + (mag > 1e-20 ? fz / mag * scale : 0);
        }
        posAttr.needsUpdate = true;
        if (lines.material.isLineDashedMaterial) lines.computeLineDistances();
        lines.geometry.setDrawRange(0, n * 2);
    }

    updatePEForceDecomposition(decomp, gravityVisGain = 1.0) {
        if (!this._peForceCoulomb) this._buildPEForceArrows();
        if (!decomp || decomp.count === 0) {
            for (const layer of [this._peForceCoulomb, this._peForceGravity, this._peForceStrong, this._peForceNet]) {
                layer.geometry.setDrawRange(0, 0);
            }
            return;
        }
        const { positions, count } = decomp;
        this._updatePEForceArrowSet(this._peForceCoulomb, positions, decomp.coulomb, count, decomp.maxCoulomb);
        this._updatePEForceArrowSet(this._peForceGravity, positions, decomp.gravity, count, decomp.maxGravity, gravityVisGain);
        this._updatePEForceArrowSet(this._peForceStrong, positions, decomp.strong, count, decomp.maxStrong);
        this._updatePEForceArrowSet(this._peForceNet, positions, decomp.net, count, decomp.maxNet);
    }

    togglePEForceCoulomb(on) { if (!this._peForceCoulomb) this._buildPEForceArrows(); this._peForceCoulomb.visible = on; if (!on) this._peForceCoulomb.geometry.setDrawRange(0, 0); }
    togglePEForceGravity(on) { if (!this._peForceGravity) this._buildPEForceArrows(); this._peForceGravity.visible = on; if (!on) this._peForceGravity.geometry.setDrawRange(0, 0); }
    togglePEForceStrong(on)  { if (!this._peForceStrong)  this._buildPEForceArrows(); this._peForceStrong.visible = on;  if (!on) this._peForceStrong.geometry.setDrawRange(0, 0); }
    togglePEForceNet(on)     { if (!this._peForceNet)     this._buildPEForceArrows(); this._peForceNet.visible = on;     if (!on) this._peForceNet.geometry.setDrawRange(0, 0); }

    // Legacy net-force API (delegates to F_net layer)
    _buildParticleForces() { this._buildPEForceArrows(); }

    updateParticleForces(positions, forces, count, maxForce) {
        if (!this._peForceNet) this._buildPEForceArrows();
        this._updatePEForceArrowSet(this._peForceNet, positions, forces, count, maxForce);
    }

    toggleParticleForces(on) { this.togglePEForceNet(on); }

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
        const phaseAttr = geo.getAttribute('manifestPhase');
        const rateAttr = geo.getAttribute('manifestRate');
        const hasManifest = !!(data.phases && data.rates);
        const colorByColorCharge = this.visualSettings.colorByColorCharge && !!data.colorCharge;

        const rawCount = Math.min(data.count, MAX_PARTICLES);

        // Clip particles to current boundary shape
        const _bs = this._getBoundaryShape();
        const needsClip = _bs && _bs !== 'none' && _bs !== 'cube';
        let count = 0;
        const originMode = this._getBoundaryMode() === 'origin';
        for (let i = 0; i < rawCount; i++) {
            const px = data.positions[i * 3];
            const py = data.positions[i * 3 + 1];
            const pz = data.positions[i * 3 + 2];
            if (needsClip) {
                let nx, ny, nz;
                if (originMode) {
                    const R = PE_VIS_BOUNDARY_R;
                    nx = px / R;
                    ny = py / R;
                    nz = pz / R;
                } else {
                    const center = this._latticeSize / 2;
                    const radius = this._latticeSize / 2;
                    nx = (px - center) / radius;
                    ny = (py - center) / radius;
                    nz = (pz - center) / radius;
                }
                if (!this._insideBoundary(nx, ny, nz)) continue;
            }
            posAttr.array[count * 3] = px;
            posAttr.array[count * 3 + 1] = py;
            posAttr.array[count * 3 + 2] = pz;
            if (colorByColorCharge) {
                const label = data.colorCharge[i] | 0;
                const [cr, cg, cb] = COLOR_CHARGE_PALETTE[label] ?? COLOR_CHARGE_PALETTE[0];
                colAttr.array[count * 3] = cr;
                colAttr.array[count * 3 + 1] = cg;
                colAttr.array[count * 3 + 2] = cb;
            } else {
                colAttr.array[count * 3] = data.colors[i * 3];
                colAttr.array[count * 3 + 1] = data.colors[i * 3 + 1];
                colAttr.array[count * 3 + 2] = data.colors[i * 3 + 2];
            }
            sizeAttr.array[count] = (data.sizes[i] ?? 3.0) * this.visualSettings.globalScale;
            if (hasManifest) {
                phaseAttr.array[count] = data.phases[i];
                rateAttr.array[count] = data.rates[i];
            } else {
                phaseAttr.array[count] = 0;
                rateAttr.array[count] = 0;
            }
            count++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        sizeAttr.needsUpdate = true;
        if (phaseAttr) phaseAttr.needsUpdate = true;
        if (rateAttr) rateAttr.needsUpdate = true;

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

    /** GPU-driven ternary manifestation blink (Scale 1 PE clouds). */
    setManifestation(enabled, timeSec, fill = 0.40) {
        const mat = this.particles?.material;
        if (!mat?.uniforms) return;
        mat.uniforms.uManifestEnabled.value = enabled ? 1.0 : 0.0;
        mat.uniforms.uManifestTime.value = timeSec;
        mat.uniforms.uManifestThresh.value = Math.sin(Math.PI * (1 - 2 * fill) / 2);
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
        disposeMesh(this.spinVectors);
        disposeMesh(this.trails);
        disposeMesh(this._particleForces);
        disposeMesh(this._peSystem);
        disposeMesh(this._voxelDebug);

        this.particles = null;
        this.velocityVectors = null;
        this.spinVectors = null;
        this.trails = null;
        this._particleForces = null;
        this._peSystem = null;
    }
}
