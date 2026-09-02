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
import { makeRingTexture, makeTextTexture, makeBillboardSprite } from '../scales/scale1/overlay-billboards.js';
import {
    DEFAULT_TRAIL_SETTINGS,
    TRAIL_HISTORY_CAPACITY,
    normalizeTrailSettings,
    trailRetentionAlpha,
} from '../scales/scale1/trail-settings.js?v=2';

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

function writeEnergyHeatColor(target, offset, normalized, fade) {
    const t = Math.max(0, Math.min(1, normalized));
    let r;
    let g;
    let b;
    if (t < 1 / 3) {
        const u = t * 3;
        r = 0.11 + (0.13 - 0.11) * u;
        g = 0.30 + (0.83 - 0.30) * u;
        b = 0.85 + (0.93 - 0.85) * u;
    } else if (t < 2 / 3) {
        const u = (t - 1 / 3) * 3;
        r = 0.13 + (0.98 - 0.13) * u;
        g = 0.83 + (0.80 - 0.83) * u;
        b = 0.93 + (0.08 - 0.93) * u;
    } else {
        const u = (t - 2 / 3) * 3;
        r = 0.98 + (0.94 - 0.98) * u;
        g = 0.80 + (0.20 - 0.80) * u;
        b = 0.08 + (0.18 - 0.08) * u;
    }
    target[offset] = r * fade;
    target[offset + 1] = g * fade;
    target[offset + 2] = b * fade;
}


export class ViewportParticleRenderer {
    constructor({
        scene,
        latticeSize,
        halfN,
        insideBoundary,
        getBoundaryShape,
        getBoundaryMode,
        getEngineMode,
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
        this._getEngineMode = getEngineMode || (() => 'lattice');
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
        this._admissibilityRings = null;
        this._provenanceLabels = null;
        this._peScenarioVisual = null;
        this._peInspectionFocus = null;
        this._peInspectionIds = null;
        // CPU-only sign cache for the rendered slots. Size controls must remain
        // correct when the color-charge overlay replaces decorative sign colors.
        this._particleSigns = new Int8Array(MAX_PARTICLES);

        // Build the main particle Points mesh eagerly (mirrors the
        // pre-extraction behaviour of `this._initParticles()` being called
        // from Viewport's constructor).
        this._initParticles();
    }

    setPEInspectionFocus(focus) {
        this._peInspectionFocus = focus || null;
        this._peInspectionIds = focus ? new Set(focus.particleIds || []) : null;
        for (const mesh of [
            this.velocityVectors, this.spinVectors, this.trails,
            this._peForceCoulomb, this._peForceGravity, this._peForceLorentz,
            this._peForceExchange, this._peForceStrong, this._peForceRadiation,
            this._peForceMagneticDipole, this._peForceSpinOrbit, this._peForceNet,
            this._peSystem,
        ]) {
            mesh?.geometry?.setDrawRange(0, 0);
        }
        if (this._admissibilityRings) {
            while (this._admissibilityRings.children.length) {
                const child = this._admissibilityRings.children[0];
                child.material?.dispose();
                this._admissibilityRings.remove(child);
            }
        }
        if (this._provenanceLabels) {
            while (this._provenanceLabels.children.length) {
                const child = this._provenanceLabels.children[0];
                child.material?.map?.dispose();
                child.material?.dispose();
                this._provenanceLabels.remove(child);
            }
        }
    }

    _matchesPEInspection(ids, index) {
        if (!this._peInspectionIds) return true;
        return !!ids && this._peInspectionIds.has(Number(ids[index]));
    }

    clearPEScenarioVisual() {
        const root = this._peScenarioVisual;
        if (!root) return;
        root.traverse((object) => {
            object.geometry?.dispose?.();
            const materials = Array.isArray(object.material)
                ? object.material : (object.material ? [object.material] : []);
            for (const material of materials) {
                material.map?.dispose?.();
                material.dispose?.();
            }
        });
        this._scene.remove(root);
        this._peScenarioVisual = null;
    }

    /**
     * Scenario-owned presentation geometry. This never contributes forces or
     * energy; the actual source and probe records remain ParticleEngine data.
     */
    setPEScenarioVisual(spec = null) {
        this.clearPEScenarioVisual();
        if (spec?.type !== 'open-terminal-battery') return;

        const length = Math.max(1, Number(spec.length) || 24);
        const height = Math.max(1, Number(spec.height) || 9);
        const depth = Math.max(1, Number(spec.depth) || 8);
        const positiveEndX = Number(spec.positiveEndX) || -length / 2;
        const negativeEndX = Number(spec.negativeEndX) || length / 2;
        const portWidth = Math.min(depth * 0.72, Math.max(0.5, Number(spec.portWidth) || 2.7));
        const portHeight = Math.min(height * 0.72, Math.max(0.5, Number(spec.portHeight) || 2.7));
        const group = new THREE.Group();
        group.name = 'pe-open-terminal-battery';
        group.userData.scenarioVisualType = spec.type;
        group.userData.presentationOnly = true;
        group.userData.physicalConstraint = 'native-perfect-insulator';
        group.userData.portCount = 2;

        const wallMaterial = new THREE.MeshBasicMaterial({
                color: 0x243147, transparent: true, opacity: 0.24,
                depthWrite: false, side: THREE.DoubleSide,
            });
        const addWall = (name, dimensions, position) => {
            const wall = new THREE.Mesh(new THREE.BoxGeometry(...dimensions), wallMaterial.clone());
            wall.name = name;
            wall.position.set(...position);
            group.add(wall);
        };
        const wallThickness = 0.14;
        addWall('battery-wall-top', [length, wallThickness, depth], [0, height / 2, 0]);
        addWall('battery-wall-bottom', [length, wallThickness, depth], [0, -height / 2, 0]);
        addWall('battery-wall-front', [length, height, wallThickness], [0, 0, depth / 2]);
        addWall('battery-wall-back', [length, height, wallThickness], [0, 0, -depth / 2]);
        wallMaterial.dispose();

        const edgeSource = new THREE.BoxGeometry(length, height, depth);
        const shellEdges = new THREE.LineSegments(
            new THREE.EdgesGeometry(edgeSource),
            new THREE.LineBasicMaterial({
                color: 0x9fb5d2, transparent: true, opacity: 0.82,
            }),
        );
        edgeSource.dispose();
        shellEdges.name = 'battery-shell-edges';
        group.add(shellEdges);

        const addTerminal = (x, color, label, name, electronDirection) => {
            const terminal = new THREE.Group();
            terminal.name = name;
            terminal.position.x = x;
            terminal.userData.aperture = { width: portWidth, height: portHeight };
            terminal.userData.carrier = 'electron';
            terminal.userData.electronDirection = electronDirection;
            const frameHeight = height * 0.9;
            const frameDepth = depth * 0.9;
            const barY = Math.max(0.1, (frameHeight - portHeight) / 2);
            const barZ = Math.max(0.1, (frameDepth - portWidth) / 2);
            const material = new THREE.MeshBasicMaterial({
                color, transparent: true, opacity: 0.42, depthWrite: false,
            });
            const addBar = (barName, dimensions, y, z) => {
                const geometry = new THREE.BoxGeometry(...dimensions);
                const bar = new THREE.Mesh(geometry, material.clone());
                bar.name = `${name}-${barName}`;
                bar.position.set(0, y, z);
                terminal.add(bar);
                const rim = new THREE.LineSegments(
                    new THREE.EdgesGeometry(geometry),
                    new THREE.LineBasicMaterial({ color, transparent: true, opacity: 0.92 }),
                );
                rim.position.copy(bar.position);
                rim.name = `${name}-${barName}-rim`;
                terminal.add(rim);
            };
            addBar('top', [0.7, barY, frameDepth], portHeight / 2 + barY / 2, 0);
            addBar('bottom', [0.7, barY, frameDepth], -(portHeight / 2 + barY / 2), 0);
            addBar('front', [0.7, portHeight, barZ], 0, portWidth / 2 + barZ / 2);
            addBar('back', [0.7, portHeight, barZ], 0, -(portWidth / 2 + barZ / 2));
            material.dispose();
            group.add(terminal);

            const texture = makeTextTexture(label, {
                color: `#${new THREE.Color(color).getHexString()}`, fontPx: 30,
            });
            const sprite = makeBillboardSprite(texture, 3.0);
            sprite.scale.set(4.2, 1.05, 1);
            sprite.position.set(x, height / 2 + 1.25, depth / 2 + 0.35);
            sprite.name = `${name}-label`;
            group.add(sprite);
        };
        addTerminal(positiveEndX, 0xff5d73, 'e⁻ IN · +', 'positive-terminal', 'in');
        addTerminal(negativeEndX, 0x58d6ff, '− · e⁻ OUT', 'negative-terminal', 'out');

        this._peScenarioVisual = group;
        this._scene.add(group);
    }

    togglePEScenarioVisual(on) {
        if (this._peScenarioVisual) this._peScenarioVisual.visible = !!on;
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
                shapeType: { value: this.visualSettings.particleShape ?? 0 },
                uOpacity: { value: this.visualSettings.particleOpacity ?? 0.9 },
                uGlow: { value: this.visualSettings.glowIntensity ?? 0.15 },
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

    // ── Admissibility ring overlay ──────────────────────────────────────
    // A THREE.Group of billboard sprites, one ring per qualified native
    // record. Rebuilt each call; record counts are small, so a full rebuild
    // is cheap next to particle pooling.
    _buildAdmissibilityRings() {
        this._admissibilityRings = new THREE.Group();
        this._admissibilityRings.visible = false;
        this._ringTexAdmissible = makeRingTexture({ color: '#4ade80', dashed: false });
        this._ringTexMarginal = makeRingTexture({ color: '#fbbf24', dashed: true });
        this._scene.add(this._admissibilityRings);
    }

    /**
     * @param {{positions:Float32Array|Float64Array, count:number}} peData
     * @param {Map<number, object>} seedById - Scale-1 snapshot display records
     * @param {Int32Array|number[]} ids - peData-aligned native particle ids
     */
    updateAdmissibilityRings(peData, seedById, ids) {
        if (!this._admissibilityRings) this._buildAdmissibilityRings();
        const group = this._admissibilityRings;
        while (group.children.length) {
            const child = group.children[0];
            child.material?.dispose();
            group.remove(child);
        }
        if (!seedById || seedById.size === 0) return;
        for (let i = 0; i < peData.count; i++) {
            if (!this._matchesPEInspection(ids, i)) continue;
            const seed = seedById.get(ids[i]);
            if (!seed) continue;
            const tex = seed.admissible ? this._ringTexAdmissible : this._ringTexMarginal;
            const sprite = makeBillboardSprite(tex, 2.5);
            sprite.position.set(
                peData.positions[i * 3], peData.positions[i * 3 + 1], peData.positions[i * 3 + 2]);
            group.add(sprite);
        }
    }

    toggleAdmissibilityRings(on) {
        if (!this._admissibilityRings) this._buildAdmissibilityRings();
        this._admissibilityRings.visible = on;
    }

    // ── Native-record provenance label overlay ──────────────────────────
    // Unlike the admissibility rings (2 cached ring textures), provenance
    // labels are per-particle unique text — each sprite's texture is built
    // and disposed on every rebuild rather than cached/reused.
    _buildProvenanceLabels() {
        this._provenanceLabels = new THREE.Group();
        this._provenanceLabels.visible = false;
        this._scene.add(this._provenanceLabels);
    }

    /**
     * @param {{positions:Float32Array|Float64Array, count:number}} peData
     * @param {Map<number, object>} seedById - Scale-1 snapshot display records
     * @param {Int32Array|number[]} ids - peData-aligned native particle ids
     */
    updateProvenanceLabels(peData, seedById, ids) {
        if (!this._provenanceLabels) this._buildProvenanceLabels();
        const group = this._provenanceLabels;
        while (group.children.length) {
            const child = group.children[0];
            child.material.map?.dispose();
            child.material.dispose();
            group.remove(child);
        }
        if (!seedById || seedById.size === 0) return;
        for (let i = 0; i < peData.count; i++) {
            if (!this._matchesPEInspection(ids, i)) continue;
            const seed = seedById.get(ids[i]);
            if (!seed) continue;
            const tex = makeTextTexture(seed.label || `#${seed.clusterId} N=${seed.size}`);
            const sprite = makeBillboardSprite(tex, 3.0);
            sprite.position.set(
                peData.positions[i * 3],
                peData.positions[i * 3 + 1] + 2.0, // offset above the particle
                peData.positions[i * 3 + 2]);
            group.add(sprite);
        }
    }

    toggleProvenanceLabels(on) {
        if (!this._provenanceLabels) this._buildProvenanceLabels();
        this._provenanceLabels.visible = on;
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

    updateVelocityVectors(positions, velocities, count, ids = null) {
        if (!this.velocityVectors) this._buildVelocityVectors();
        if (!velocities) return;

        const posAttr = this.velocityVectors.geometry.getAttribute('position');
        const colAttr = this.velocityVectors.geometry.getAttribute('color');
        const maxLines = posAttr.array.length / 6;
        const n = Math.min(count, maxLines);
        const scale = 40; // world-units per unit velocity (β-colored below)

        let drawn = 0;
        for (let i = 0; i < n; i++) {
            if (!this._matchesPEInspection(ids, i)) continue;
            const line = drawn++;
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            const vx = velocities[i * 3], vy = velocities[i * 3 + 1], vz = velocities[i * 3 + 2];

            // Start point (particle center)
            posAttr.array[line * 6] = px;
            posAttr.array[line * 6 + 1] = py;
            posAttr.array[line * 6 + 2] = pz;
            // End point (position + velocity * scale)
            posAttr.array[line * 6 + 3] = px + vx * scale;
            posAttr.array[line * 6 + 4] = py + vy * scale;
            posAttr.array[line * 6 + 5] = pz + vz * scale;

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
            colAttr.array[line * 6] = r * 0.5;
            colAttr.array[line * 6 + 1] = g * 0.5;
            colAttr.array[line * 6 + 2] = b * 0.5;
            colAttr.array[line * 6 + 3] = r;
            colAttr.array[line * 6 + 4] = g;
            colAttr.array[line * 6 + 5] = b;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this.velocityVectors.geometry.setDrawRange(0, drawn * 2);
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

    updateSpinVectors(positions, spinAxes, spins, count, ids = null) {
        if (!this.spinVectors) this._buildSpinVectors();
        if (!spinAxes || !spins) return;

        const posAttr = this.spinVectors.geometry.getAttribute('position');
        const colAttr = this.spinVectors.geometry.getAttribute('color');
        const maxLines = posAttr.array.length / 6;
        const n = Math.min(count, maxLines);
        const scale = 2.8;
        let drawn = 0;

        for (let i = 0; i < n; i++) {
            if (!this._matchesPEInspection(ids, i)) continue;
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

    // ── Trajectory history (PE mode overlay) ─────────────────────────
    _buildTrails(renderMode = DEFAULT_TRAIL_SETTINGS.renderMode) {
        const breadcrumbs = renderMode === 'breadcrumbs';
        const initialUnits = 50 * 200;
        const verticesPerUnit = breadcrumbs ? 1 : 2;
        const vertices = new Float32Array(initialUnits * verticesPerUnit * 3);
        const colors = new Float32Array(initialUnits * verticesPerUnit * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = breadcrumbs
            ? new THREE.PointsMaterial({
                vertexColors: true,
                transparent: true,
                opacity: DEFAULT_TRAIL_SETTINGS.opacity,
                size: DEFAULT_TRAIL_SETTINGS.pointSize,
                sizeAttenuation: true,
                depthWrite: false,
                blending: THREE.AdditiveBlending,
            })
            : new THREE.LineBasicMaterial({
                vertexColors: true,
                transparent: true,
                opacity: DEFAULT_TRAIL_SETTINGS.opacity,
                depthWrite: false,
            });
        this.trails = breadcrumbs ? new THREE.Points(geo, mat) : new THREE.LineSegments(geo, mat);
        this.trails.userData.trailMode = renderMode;
        this.trails.frustumCulled = false;
        this.trails.visible = false;
        this._scene.add(this.trails);
    }

    _ensureTrailMode(renderMode) {
        if (this.trails?.userData?.trailMode === renderMode) return;
        const visible = this.trails?.visible ?? false;
        if (this.trails) {
            this._scene.remove(this.trails);
            this.trails.geometry?.dispose();
            this.trails.material?.dispose();
        }
        this.trails = null;
        this._buildTrails(renderMode);
        this.trails.visible = visible;
    }

    _ensureTrailCapacity(requiredUnits) {
        const position = this.trails.geometry.getAttribute('position');
        const verticesPerUnit = this.trails.isPoints ? 1 : 2;
        const currentUnits = Math.floor(position.count / verticesPerUnit);
        if (currentUnits >= requiredUnits) {
            if (requiredUnits * 4 < currentUnits) {
                this._trailLowUsageFrames = (this._trailLowUsageFrames || 0) + 1;
                if (this._trailLowUsageFrames >= 120) {
                    this._resizeTrailGeometry(Math.max(64, Math.ceil(requiredUnits * 1.6)));
                    this._trailLowUsageFrames = 0;
                }
            } else {
                this._trailLowUsageFrames = 0;
            }
            return;
        }
        this._trailLowUsageFrames = 0;
        let capacity = Math.max(1, currentUnits);
        while (capacity < requiredUnits) capacity = Math.ceil(capacity * 1.6);
        this._resizeTrailGeometry(capacity);
    }

    _resizeTrailGeometry(capacity) {
        const verticesPerUnit = this.trails.isPoints ? 1 : 2;
        const vertexCount = capacity * verticesPerUnit;
        this.trails.geometry.setAttribute(
            'position', new THREE.Float32BufferAttribute(new Float32Array(vertexCount * 3), 3));
        this.trails.geometry.setAttribute(
            'color', new THREE.Float32BufferAttribute(new Float32Array(vertexCount * 3), 3));
    }

    updateTrails(trailHistory, typeMap, candidateSettings, currentTick) {
        const settings = normalizeTrailSettings(candidateSettings);
        this._ensureTrailMode(settings.renderMode);
        const emptyStats = {
            mode: settings.renderMode,
            drawn: 0,
            minEnergyDensity: 0,
            maxEnergyDensity: 0,
        };
        if (!trailHistory || trailHistory.size === 0) {
            this.trails.geometry.setDrawRange(0, 0);
            return emptyStats;
        }

        const tick = Number.isFinite(Number(currentTick)) ? Number(currentTick) : 0;
        const oldestTick = tick - settings.historyTicks;
        this.trails.material.opacity = settings.opacity;
        if (this.trails.isPoints) this.trails.material.size = settings.pointSize;

        let requiredUnits = 0;
        let minEnergyDensity = Number.POSITIVE_INFINITY;
        let maxEnergyDensity = 0;
        for (const [particleId, trail] of trailHistory) {
            if (this._peInspectionIds && !this._peInspectionIds.has(Number(particleId))) continue;
            const len = trail.length;
            const capacity = trail.capacity || TRAIL_HISTORY_CAPACITY;
            const start = len < capacity ? 0 : trail.head;
            let visibleSamples = 0;
            for (let j = 0; j < len; j++) {
                const idx = (start + j) % capacity;
                if (trail.ticks?.[idx] < oldestTick) continue;
                visibleSamples++;
                const density = Math.max(0, Number(trail.energyDensities?.[idx]) || 0);
                minEnergyDensity = Math.min(minEnergyDensity, density);
                maxEnergyDensity = Math.max(maxEnergyDensity, density);
            }
            requiredUnits += this.trails.isPoints
                ? visibleSamples : Math.max(0, visibleSamples - 1);
        }
        this._ensureTrailCapacity(requiredUnits);

        const posAttr = this.trails.geometry.getAttribute('position');
        const colAttr = this.trails.geometry.getAttribute('color');
        const maxUnits = Math.floor(posAttr.count / (this.trails.isPoints ? 1 : 2));
        const minLogEnergy = Math.log1p(Number.isFinite(minEnergyDensity) ? minEnergyDensity : 0);
        const maxLogEnergy = Math.log1p(maxEnergyDensity);
        const logEnergySpan = Math.max(1e-12, maxLogEnergy - minLogEnergy);
        let drawn = 0;

        const sampleTick = (trail, idx, fallback) => trail.ticks
            ? trail.ticks[idx] : fallback;
        const ageFade = (sample) => {
            const normalizedAge = Math.max(0, Math.min(
                1, (sample - oldestTick) / Math.max(settings.historyTicks, 1)));
            return 0.06 + 0.94 * Math.pow(normalizedAge, settings.fadeExponent);
        };

        for (const [particleId, trail] of trailHistory) {
            if (this._peInspectionIds && !this._peInspectionIds.has(Number(particleId))) continue;
            if (trail.length < 1) continue;

            const catId = typeMap ? typeMap.get(particleId) : null;
            const cat = catId ? getById(catId) : null;
            const cr = cat ? cat.display_color[0] : 0.5;
            const cg = cat ? cat.display_color[1] : 0.5;
            const cb = cat ? cat.display_color[2] : 0.5;
            const len = trail.length;
            const capacity = trail.capacity || TRAIL_HISTORY_CAPACITY;
            const start = len < capacity ? 0 : trail.head;
            const retention = trailRetentionAlpha(trail, tick, settings);
            if (retention <= 0) continue;

            if (this.trails.isPoints) {
                for (let j = 0; j < len && drawn < maxUnits; j++) {
                    const idx = (start + j) % capacity;
                    const stamp = sampleTick(trail, idx, tick - (len - 1 - j));
                    if (stamp < oldestTick) continue;
                    posAttr.array[drawn * 3] = trail.positions[idx * 3];
                    posAttr.array[drawn * 3 + 1] = trail.positions[idx * 3 + 1];
                    posAttr.array[drawn * 3 + 2] = trail.positions[idx * 3 + 2];
                    const speed = trail.speeds?.[idx] || 0;
                    const speedBoost = 0.65 + 0.35 * Math.min(speed / C_SPEED, 1);
                    const fade = ageFade(stamp) * retention * speedBoost;
                    colAttr.array[drawn * 3] = cr * fade;
                    colAttr.array[drawn * 3 + 1] = cg * fade;
                    colAttr.array[drawn * 3 + 2] = cb * fade;
                    drawn++;
                }
                continue;
            }

            for (let j = 1; j < len && drawn < maxUnits; j++) {
                const idx0 = (start + j - 1) % capacity;
                const idx1 = (start + j) % capacity;
                const stamp0 = sampleTick(trail, idx0, tick - (len - j));
                const stamp1 = sampleTick(trail, idx1, tick - (len - 1 - j));
                if (stamp0 < oldestTick || stamp1 < oldestTick) continue;
                const offset = drawn * 6;
                posAttr.array[offset] = trail.positions[idx0 * 3];
                posAttr.array[offset + 1] = trail.positions[idx0 * 3 + 1];
                posAttr.array[offset + 2] = trail.positions[idx0 * 3 + 2];
                posAttr.array[offset + 3] = trail.positions[idx1 * 3];
                posAttr.array[offset + 4] = trail.positions[idx1 * 3 + 1];
                posAttr.array[offset + 5] = trail.positions[idx1 * 3 + 2];
                const fade0 = ageFade(stamp0) * retention;
                const fade1 = ageFade(stamp1) * retention;
                if (settings.renderMode === 'energy') {
                    const density0 = Math.max(0, Number(trail.energyDensities?.[idx0]) || 0);
                    const density1 = Math.max(0, Number(trail.energyDensities?.[idx1]) || 0);
                    writeEnergyHeatColor(
                        colAttr.array, offset,
                        (Math.log1p(density0) - minLogEnergy) / logEnergySpan,
                        fade0);
                    writeEnergyHeatColor(
                        colAttr.array, offset + 3,
                        (Math.log1p(density1) - minLogEnergy) / logEnergySpan,
                        fade1);
                } else {
                    colAttr.array[offset] = cr * fade0;
                    colAttr.array[offset + 1] = cg * fade0;
                    colAttr.array[offset + 2] = cb * fade0;
                    colAttr.array[offset + 3] = cr * fade1;
                    colAttr.array[offset + 4] = cg * fade1;
                    colAttr.array[offset + 5] = cb * fade1;
                }
                drawn++;
            }
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this.trails.geometry.setDrawRange(0, drawn * (this.trails.isPoints ? 1 : 2));
        return {
            mode: settings.renderMode,
            drawn,
            minEnergyDensity: Number.isFinite(minEnergyDensity) ? minEnergyDensity : 0,
            maxEnergyDensity,
        };
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

    // ── Per-Particle Force Arrows (all native decomposition channels) ──
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
        this._peForceLorentz = makeSet(0x22d3ee);
        this._peForceExchange = makeSet(0xc084fc, /* dashed */ true);
        this._peForceStrong = makeSet(0xff1744, /* dashed */ true);
        this._peForceRadiation = makeSet(0xfbbf24, /* dashed */ true);
        this._peForceMagneticDipole = makeSet(0x60a5fa);
        this._peForceSpinOrbit = makeSet(0xfb923c);
        this._peForceNet = makeSet(0x44cc66);
        // Legacy alias — net force layer
        this._particleForces = this._peForceNet;
    }

    _updatePEForceArrowSet(lines, positions, forces, count, maxForce, visGain = 1.0, ids = null) {
        if (!lines) return;
        const posAttr = lines.geometry.getAttribute('position');
        const n = Math.min(count, 200);
        const arrowScale = 12.0;
        let drawn = 0;
        for (let i = 0; i < n; i++) {
            if (!this._matchesPEInspection(ids, i)) continue;
            const line = drawn++;
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            const fx = forces[i * 3] * visGain;
            const fy = forces[i * 3 + 1] * visGain;
            const fz = forces[i * 3 + 2] * visGain;
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            posAttr.array[line * 6] = px;
            posAttr.array[line * 6 + 1] = py;
            posAttr.array[line * 6 + 2] = pz;
            const scale = mag > 1e-20 ? arrowScale * Math.log(1 + mag / (maxForce * visGain + 1e-20) * 10) : 0;
            posAttr.array[line * 6 + 3] = px + (mag > 1e-20 ? fx / mag * scale : 0);
            posAttr.array[line * 6 + 4] = py + (mag > 1e-20 ? fy / mag * scale : 0);
            posAttr.array[line * 6 + 5] = pz + (mag > 1e-20 ? fz / mag * scale : 0);
        }
        posAttr.needsUpdate = true;
        if (lines.material.isLineDashedMaterial) lines.computeLineDistances();
        lines.geometry.setDrawRange(0, drawn * 2);
    }

    updatePEForceDecomposition(decomp, gravityVisGain = 1.0, ids = null) {
        if (!this._peForceCoulomb) this._buildPEForceArrows();
        if (!decomp || decomp.count === 0) {
            for (const layer of [
                this._peForceCoulomb, this._peForceGravity, this._peForceLorentz,
                this._peForceExchange, this._peForceStrong, this._peForceRadiation,
                this._peForceMagneticDipole, this._peForceSpinOrbit, this._peForceNet,
            ]) {
                layer.geometry.setDrawRange(0, 0);
            }
            return;
        }
        const { positions, count } = decomp;
        this._updatePEForceArrowSet(this._peForceCoulomb, positions, decomp.coulomb, count, decomp.maxCoulomb, 1.0, ids);
        this._updatePEForceArrowSet(this._peForceGravity, positions, decomp.gravity, count, decomp.maxGravity, gravityVisGain, ids);
        this._updatePEForceArrowSet(this._peForceLorentz, positions, decomp.lorentz, count, decomp.maxLorentz, 1.0, ids);
        this._updatePEForceArrowSet(this._peForceExchange, positions, decomp.exchange, count, decomp.maxExchange, 1.0, ids);
        this._updatePEForceArrowSet(this._peForceStrong, positions, decomp.strong, count, decomp.maxStrong, 1.0, ids);
        this._updatePEForceArrowSet(this._peForceRadiation, positions, decomp.radiation, count, decomp.maxRadiation, 1.0, ids);
        this._updatePEForceArrowSet(this._peForceMagneticDipole, positions, decomp.magnetic_dipole, count, decomp.maxMagneticDipole, 1.0, ids);
        this._updatePEForceArrowSet(this._peForceSpinOrbit, positions, decomp.spin_orbit, count, decomp.maxSpinOrbit, 1.0, ids);
        this._updatePEForceArrowSet(this._peForceNet, positions, decomp.net, count, decomp.maxNet, 1.0, ids);
    }

    togglePEForceCoulomb(on) { if (!this._peForceCoulomb) this._buildPEForceArrows(); this._peForceCoulomb.visible = on; if (!on) this._peForceCoulomb.geometry.setDrawRange(0, 0); }
    togglePEForceGravity(on) { if (!this._peForceGravity) this._buildPEForceArrows(); this._peForceGravity.visible = on; if (!on) this._peForceGravity.geometry.setDrawRange(0, 0); }
    togglePEForceLorentz(on) { if (!this._peForceLorentz) this._buildPEForceArrows(); this._peForceLorentz.visible = on; if (!on) this._peForceLorentz.geometry.setDrawRange(0, 0); }
    togglePEForceExchange(on) { if (!this._peForceExchange) this._buildPEForceArrows(); this._peForceExchange.visible = on; if (!on) this._peForceExchange.geometry.setDrawRange(0, 0); }
    togglePEForceStrong(on)  { if (!this._peForceStrong)  this._buildPEForceArrows(); this._peForceStrong.visible = on;  if (!on) this._peForceStrong.geometry.setDrawRange(0, 0); }
    togglePEForceRadiation(on) { if (!this._peForceRadiation) this._buildPEForceArrows(); this._peForceRadiation.visible = on; if (!on) this._peForceRadiation.geometry.setDrawRange(0, 0); }
    togglePEForceMagneticDipole(on) { if (!this._peForceMagneticDipole) this._buildPEForceArrows(); this._peForceMagneticDipole.visible = on; if (!on) this._peForceMagneticDipole.geometry.setDrawRange(0, 0); }
    togglePEForceSpinOrbit(on) { if (!this._peForceSpinOrbit) this._buildPEForceArrows(); this._peForceSpinOrbit.visible = on; if (!on) this._peForceSpinOrbit.geometry.setDrawRange(0, 0); }
    togglePEForceNet(on)     { if (!this._peForceNet)     this._buildPEForceArrows(); this._peForceNet.visible = on;     if (!on) this._peForceNet.geometry.setDrawRange(0, 0); }

    // Legacy net-force API (delegates to F_net layer)
    _buildParticleForces() { this._buildPEForceArrows(); }

    updateParticleForces(positions, forces, count, maxForce, ids = null) {
        if (!this._peForceNet) this._buildPEForceArrows();
        this._updatePEForceArrowSet(this._peForceNet, positions, forces, count, maxForce, 1.0, ids);
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
        const latticeMode = this._getEngineMode() === 'lattice';

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
            const sourceR = data.colors[i * 3];
            const sourceG = data.colors[i * 3 + 1];
            const sign = sourceG > sourceR ? 1 : (sourceR > sourceG ? -1 : 0);
            this._particleSigns[count] = sign;
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
            const size = latticeMode && sign !== 0
                ? (sign > 0 ? this.visualSettings.positiveSize : this.visualSettings.negativeSize)
                : (data.sizes[i] ?? 3.0);
            sizeAttr.array[count] = size * this.visualSettings.globalScale;
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
        const sizeAttr = geo.getAttribute('size');
        if (!sizeAttr) return;

        const count = geo.drawRange.count;
        if (count === 0) return;
        let changed = false;
        for (let i = 0; i < count; i++) {
            const sign = this._particleSigns[i];
            if (sign === 0) continue;
            const baseSize = sign > 0
                ? this.visualSettings.positiveSize
                : this.visualSettings.negativeSize;
            const next = baseSize * this.visualSettings.globalScale;
            if (sizeAttr.array[i] === next) continue;
            sizeAttr.array[i] = next;
            changed = true;
        }
        if (changed) sizeAttr.needsUpdate = true;
    }


    // ── Particle shape and opacity ──────────────────────────────────
    setPointShape(shapeIndex) {
        const shape = shapeIndex | 0;
        if (shape < 0 || shape > 7 || this.visualSettings.particleShape === shape) return;
        this.visualSettings.particleShape = shape;
        if (this.particles && this.particles.material.uniforms) {
            this.particles.material.uniforms.shapeType.value = shape;
        }
    }

    setOpacity(val) {
        if (this.visualSettings.particleOpacity === val) return;
        if (this.particles && this.particles.material.uniforms) {
            this.particles.material.uniforms.uOpacity.value = val;
        }
        this.visualSettings.particleOpacity = val;
        this.visualSettings.opacity = val;
    }

    setGlow(val) {
        if (this.visualSettings.glowIntensity === val) return;
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
        this.clearPEScenarioVisual();
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
        // Dispose every PE force-arrow layer, not just _peForceNet (which
        // _particleForces aliases), so no geometry/material survives teardown.
        disposeMesh(this._peForceCoulomb);
        disposeMesh(this._peForceGravity);
        disposeMesh(this._peForceLorentz);
        disposeMesh(this._peForceExchange);
        disposeMesh(this._peForceStrong);
        disposeMesh(this._peForceRadiation);
        disposeMesh(this._peForceMagneticDipole);
        disposeMesh(this._peForceSpinOrbit);
        disposeMesh(this._peForceNet);
        this._peForceCoulomb = this._peForceGravity = this._peForceLorentz = null;
        this._peForceExchange = this._peForceStrong = this._peForceRadiation = null;
        this._peForceMagneticDipole = this._peForceSpinOrbit = null;
        this._peForceNet = this._particleForces = null;
        disposeMesh(this._peSystem);
        if (this._admissibilityRings) {
            while (this._admissibilityRings.children.length) {
                disposeMesh(this._admissibilityRings.children.pop());
            }
            this._scene.remove(this._admissibilityRings);
        }
        if (this._ringTexAdmissible) this._ringTexAdmissible.dispose();
        if (this._ringTexMarginal) this._ringTexMarginal.dispose();

        if (this._provenanceLabels) {
            while (this._provenanceLabels.children.length) {
                disposeMesh(this._provenanceLabels.children.pop());
            }
            this._scene.remove(this._provenanceLabels);
        }

        this.particles = null;
        this.velocityVectors = null;
        this.spinVectors = null;
        this.trails = null;
        this._particleForces = null;
        this._peSystem = null;
        this._peScenarioVisual = null;
    }
}
