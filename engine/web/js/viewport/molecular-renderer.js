/**
 * Molecular renderer — Scale 2 (atoms) and Scale 3 (molecules).
 *
 * Owns every visual that rides on an atom or a bond:
 *   - bondLines         thin line segments, Scale 2 quick view
 *   - _bondCylinders    instanced thick cylinders, single/double/triple
 *   - _bondLight        directional light added for bond shading
 *   - _nucleusShells    empirical nuclear-extent envelopes scaled by A^(1/3)
 *   - _orbitalShells    translucent spheres, one per principal quantum n
 *   - _orbitalLobes     p/d/f lobes for the valence shell
 *   - _aeForce*        exact per-atom force-component arrows
 *   - _elementLabels    sprite-based chemical symbols
 *
 * Extracted from viewport.js as Wave 2 ticket 4 of the large-file refactor
 * (see engine/web/docs/INDEX.md). Every body is preserved
 * verbatim; the only structural change is that these visuals now live on
 * a MolecularRenderer instance that viewport.js composes rather than
 * inherits. Viewport retains a thin delegator for each method so external
 * callers see no API change.
 *
 * The renderer reads ONLY `this.scene` from its constructor argument (no
 * cross-section dependencies on `_halfN`, `_boundaryShape`, `_insideBoundary`,
 * or `_engineMode`) — confirmed during refactor scoping. That's what makes
 * this a LOW-risk extraction: the scene is the only shared concern.
 */

import * as THREE from 'three';

function writeVArrow(array, offset, px, py, pz, tx, ty, tz, minHead, maxHead) {
    const length = Math.hypot(tx - px, ty - py, tz - pz);
    const dx = (tx - px) / length, dy = (ty - py) / length, dz = (tz - pz) / length;
    let qx = -dy, qy = dx, qz = 0;
    let qmag = Math.hypot(qx, qy, qz);
    if (qmag < 1e-6) {
        qx = 0; qy = -dz; qz = dy;
        qmag = Math.hypot(qx, qy, qz);
    }
    qx /= qmag; qy /= qmag; qz /= qmag;
    const headLength = Math.min(maxHead, Math.max(minHead, length * 0.28));
    const headWidth = headLength * 0.48;
    const bx = tx - dx * headLength, by = ty - dy * headLength, bz = tz - dz * headLength;
    array[offset] = px; array[offset + 1] = py; array[offset + 2] = pz;
    array[offset + 3] = tx; array[offset + 4] = ty; array[offset + 5] = tz;
    array[offset + 6] = tx; array[offset + 7] = ty; array[offset + 8] = tz;
    array[offset + 9] = bx + qx * headWidth;
    array[offset + 10] = by + qy * headWidth;
    array[offset + 11] = bz + qz * headWidth;
    array[offset + 12] = tx; array[offset + 13] = ty; array[offset + 14] = tz;
    array[offset + 15] = bx - qx * headWidth;
    array[offset + 16] = by - qy * headWidth;
    array[offset + 17] = bz - qz * headWidth;
}

export class MolecularRenderer {
    constructor(scene) {
        this.scene = scene;
        this.bondLines = null;
        this._bondCylinders = null;
        this._bondLight = null;
        this._nucleusShells = null;
        this._orbitalShells = null;
        this._orbitalLobes = null;
        this._aeForceIonic = null;
        this._aeForceVdw = null;
        this._aeForceBond = null;
        this._aeForceHBond = null;
        this._aeForceAngle = null;
        this._aeForceDipole = null;
        this._aeForceNet = null;
        this._aeDipoles = null;
        this._hbondLines = null;
        this._hbondCapacity = 0;
        this._elementLabels = null;
        this._nuclearFlashes = null;
        this._nuclearHeat = null;
        this._nuclearRadiation = null;
        this._nuclearWavefronts = null;
        this._nuclearPackets = null;
        this._nuclearShockRings = null;
        this._nuclearTransportBoundary = null;
        this._nuclearLight = null;
        this._nuclearPresentationEvents = new Map();
        this._labelPool = null;
        this._canvasPool = [];
        // Optional function (Z) -> neutron count. External callers may
        // assign through `viewport._molRenderer._defaultNeutronCount`; if
        // left null the `updateNucleusShells` pathway falls back to the
        // Math.round(Z * 1.2) approximation used before extraction.
        this._defaultNeutronCount = null;
    }

    // ── Bond Lines (Scale 2 — Atom mode) ──────────────────────────────

    _buildBondLines() {
        const MAX_BONDS = 1024;
        const vertices = new Float32Array(MAX_BONDS * 2 * 3);
        const colors = new Float32Array(MAX_BONDS * 2 * 3);
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.8,
        });
        this.bondLines = new THREE.LineSegments(geo, mat);
        this.bondLines.frustumCulled = false; // dynamic geo — see _eFieldLines
        this.bondLines.visible = true;
        this.scene.add(this.bondLines);
    }

    updateBondLines(atomData) {
        if (!this.bondLines) this._buildBondLines();
        if (!atomData || !atomData.bonds || atomData.bondCount === 0) {
            this.bondLines.geometry.setDrawRange(0, 0);
            return;
        }

        const posAttr = this.bondLines.geometry.getAttribute('position');
        const colAttr = this.bondLines.geometry.getAttribute('color');
        const maxBonds = posAttr.array.length / 6;
        const n = Math.min(atomData.bondCount, maxBonds);
        const idToIdx = new Map();
        for (let i = 0; i < atomData.count; i++) idToIdx.set(atomData.ids[i], i);
        let drawn = 0;

        for (let b = 0; b < n; b++) {
            // Bonds are stored by stable atom ID, not by the atom's current
            // array slot. IDs become sparse after reactions/removals.
            const idxA = idToIdx.get(atomData.bonds[b * 2]);
            const idxB = idToIdx.get(atomData.bonds[b * 2 + 1]);
            if (idxA === undefined || idxB === undefined) continue;
            const offset = drawn * 6;

            // Start vertex (atom A position)
            posAttr.array[offset] = atomData.positions[idxA * 3];
            posAttr.array[offset + 1] = atomData.positions[idxA * 3 + 1];
            posAttr.array[offset + 2] = atomData.positions[idxA * 3 + 2];
            // End vertex (atom B position)
            posAttr.array[offset + 3] = atomData.positions[idxB * 3];
            posAttr.array[offset + 4] = atomData.positions[idxB * 3 + 1];
            posAttr.array[offset + 5] = atomData.positions[idxB * 3 + 2];

            // Bond color: blend the two atom colors
            const rA = atomData.colors[idxA * 3], gA = atomData.colors[idxA * 3 + 1], bA = atomData.colors[idxA * 3 + 2];
            const rB = atomData.colors[idxB * 3], gB = atomData.colors[idxB * 3 + 1], bB = atomData.colors[idxB * 3 + 2];
            colAttr.array[offset] = rA;
            colAttr.array[offset + 1] = gA;
            colAttr.array[offset + 2] = bA;
            colAttr.array[offset + 3] = rB;
            colAttr.array[offset + 4] = gB;
            colAttr.array[offset + 5] = bB;
            drawn++;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this.bondLines.geometry.setDrawRange(0, drawn * 2);
    }

    toggleBondLines(on) {
        if (!this.bondLines) this._buildBondLines();
        this.bondLines.visible = on;
        if (!on) this.bondLines.geometry.setDrawRange(0, 0);
    }

    // ── Nuclear extent envelopes (empirical A^(1/3) display) ──────────

    _buildNucleusShells() {
        const maxShells = 512;
        const geo = new THREE.SphereGeometry(1, 16, 12);
        const mat = new THREE.MeshBasicMaterial({
            color: 0xff6633, transparent: true, opacity: 0.12,
            blending: THREE.AdditiveBlending, depthWrite: false, side: THREE.DoubleSide,
        });
        this._nucleusShells = new THREE.InstancedMesh(geo, mat, maxShells);
        this._nucleusShells.count = 0;
        this._nucleusShells.visible = true;
        this._nucleusShells.renderOrder = -2;
        this.scene.add(this._nucleusShells);
    }

    updateNucleusShells(atomData) {
        if (!this._nucleusShells) this._buildNucleusShells();
        if (!atomData || atomData.count === 0) { this._nucleusShells.count = 0; return; }
        const n = Math.min(atomData.count, 512);
        const mat4 = new THREE.Matrix4();
        const populationScale = atomData.count > 16 ? 0.25 : 1;
        this._nucleusShells.material.opacity = atomData.count > 16 ? 0.065 : 0.12;
        for (let i = 0; i < n; i++) {
            const Z = atomData.atomicNums[i];
            const N_neutrons = atomData.neutronCounts?.[i]
                ?? (this._defaultNeutronCount ? this._defaultNeutronCount(Z) : Math.round(Z * 1.2));
            const A = Z + N_neutrons;
            const radius = 0.5 * Math.cbrt(Math.max(A, 1)) * 1.8 * populationScale;
            mat4.makeScale(radius, radius, radius);
            mat4.setPosition(atomData.positions[i * 3], atomData.positions[i * 3 + 1], atomData.positions[i * 3 + 2]);
            this._nucleusShells.setMatrixAt(i, mat4);
        }
        this._nucleusShells.count = n;
        this._nucleusShells.instanceMatrix.needsUpdate = true;
    }

    toggleNucleusShells(on) {
        if (!this._nucleusShells) this._buildNucleusShells();
        this._nucleusShells.visible = on;
    }

    // ── Effective nuclear event / transport visuals ─────────────────

    _buildNuclearEffects() {
        const sphere = new THREE.SphereGeometry(1, 20, 14);
        this._nuclearFlashes = new THREE.InstancedMesh(sphere, new THREE.MeshBasicMaterial({
            color: 0xfff6c7, transparent: true, opacity: 0.4,
            blending: THREE.AdditiveBlending, depthWrite: false, depthTest: false,
        }), 256);
        this._nuclearFlashes.count = 0;
        this._nuclearFlashes.frustumCulled = false;
        this._nuclearFlashes.renderOrder = 12;
        this.scene.add(this._nuclearFlashes);

        this._nuclearHeat = new THREE.InstancedMesh(sphere.clone(), new THREE.MeshBasicMaterial({
            color: 0xffffff, vertexColors: true, transparent: true, opacity: 0.075,
            blending: THREE.AdditiveBlending, depthWrite: false, depthTest: true,
        }), 256);
        this._nuclearHeat.count = 0;
        this._nuclearHeat.frustumCulled = false;
        this._nuclearHeat.renderOrder = 7;
        this.scene.add(this._nuclearHeat);

        // A wire sphere is a qualitative carrier front, not a claim that the
        // event is isotropic at resolved nuclear scales. Distinct expansion
        // rates make prompt gamma and neutron transport readable at a glance.
        this._nuclearWavefronts = new THREE.InstancedMesh(
            new THREE.SphereGeometry(1, 16, 10),
            new THREE.MeshBasicMaterial({
                color: 0xffffff, vertexColors: true, wireframe: true,
                transparent: true, opacity: 0.24,
                blending: THREE.AdditiveBlending, depthWrite: false, depthTest: false,
            }),
            768,
        );
        this._nuclearWavefronts.count = 0;
        this._nuclearWavefronts.frustumCulled = false;
        this._nuclearWavefronts.renderOrder = 9;
        this.scene.add(this._nuclearWavefronts);

        this._nuclearPackets = new THREE.InstancedMesh(
            new THREE.SphereGeometry(1, 10, 8),
            new THREE.MeshBasicMaterial({
                color: 0xffffff, vertexColors: true, transparent: true, opacity: 0.96,
                blending: THREE.AdditiveBlending, depthWrite: false, depthTest: false,
            }),
            4096,
        );
        this._nuclearPackets.count = 0;
        this._nuclearPackets.frustumCulled = false;
        this._nuclearPackets.renderOrder = 13;
        this.scene.add(this._nuclearPackets);

        this._nuclearShockRings = new THREE.InstancedMesh(
            new THREE.TorusGeometry(1, 0.055, 6, 36),
            new THREE.MeshBasicMaterial({
                color: 0xffffff, vertexColors: true, transparent: true, opacity: 0.52,
                blending: THREE.AdditiveBlending, depthWrite: false, depthTest: false,
            }),
            512,
        );
        this._nuclearShockRings.count = 0;
        this._nuclearShockRings.frustumCulled = false;
        this._nuclearShockRings.renderOrder = 11;
        this.scene.add(this._nuclearShockRings);

        this._nuclearLight = new THREE.PointLight(0xffb55c, 0, 54, 2);
        this._nuclearLight.visible = false;
        this.scene.add(this._nuclearLight);

        this._nuclearTransportBoundary = new THREE.LineSegments(
            new THREE.WireframeGeometry(new THREE.SphereGeometry(1, 20, 12)),
            new THREE.LineBasicMaterial({ color: 0x67e8f9, transparent: true, opacity: 0.13, depthWrite: false }),
        );
        this._nuclearTransportBoundary.visible = false;
        this._nuclearTransportBoundary.renderOrder = 3;
        this.scene.add(this._nuclearTransportBoundary);

        const geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.Float32BufferAttribute(new Float32Array(4096 * 6), 3));
        geometry.setAttribute('color', new THREE.Float32BufferAttribute(new Float32Array(4096 * 6), 3));
        geometry.setDrawRange(0, 0);
        this._nuclearRadiation = new THREE.LineSegments(geometry, new THREE.LineBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.5,
            blending: THREE.AdditiveBlending, depthWrite: false, depthTest: false,
        }));
        this._nuclearRadiation.frustumCulled = false;
        this._nuclearRadiation.renderOrder = 10;
        this.scene.add(this._nuclearRadiation);
    }

    updateNuclearEffects(data, {
        events = false, radiation = false, heat = false, boundary = false,
    } = {}) {
        if (!this._nuclearFlashes) this._buildNuclearEffects();
        const effects = data?.effects || [];
        if (boundary && data?.transportRadius > 0) {
            this._nuclearTransportBoundary.visible = true;
            this._nuclearTransportBoundary.scale.setScalar(data.transportRadius);
            this._nuclearTransportBoundary.material.color.setHex(
                data.boundaryMode === 'reflect' ? 0xfbbf24 : 0x67e8f9,
            );
        } else {
            this._nuclearTransportBoundary.visible = false;
        }
        const activePresentationKeys = new Set();
        if (effects.length === 0) this._nuclearPresentationEvents.clear();
        const nowMs = typeof performance !== 'undefined' ? performance.now() : Date.now();
        const matrix = new THREE.Matrix4();
        const positionV = new THREE.Vector3();
        const scaleV = new THREE.Vector3();
        const axisV = new THREE.Vector3();
        const zAxis = new THREE.Vector3(0, 0, 1);
        const quaternion = new THREE.Quaternion();
        const heatColor = new THREE.Color();
        const carrierColor = new THREE.Color();
        let flashCount = 0, heatCount = 0, rayCount = 0;
        let wavefrontCount = 0, packetCount = 0, shockCount = 0;
        let lightStrength = 0;
        const position = this._nuclearRadiation.geometry.getAttribute('position');
        const color = this._nuclearRadiation.geometry.getAttribute('color');
        const maxRays = position.array.length / 6;
        const directions = [
            [1, 0.22, 0.08], [-0.38, 0.92, 0.18], [0.12, -0.26, 1],
            [-0.84, -0.32, 0.45], [0.5, 0.18, -0.88], [0.22, -0.94, -0.28],
            [0.72, 0.62, 0.36], [-0.66, 0.54, -0.52], [0.36, -0.72, 0.64],
            [-0.18, -0.58, -0.82], [0.9, -0.42, 0.24], [-0.52, -0.12, 0.92],
        ];
        for (let i = 0; i < Math.min(effects.length, 256); i++) {
            const event = effects[i];
            const generation = Math.max(0, Number(event.generation) || 0);
            const presentationKey = `${event.ordinal ?? i}:${event.tick}:${generation}`;
            activePresentationKeys.add(presentationKey);
            if (!this._nuclearPresentationEvents.has(presentationKey)) {
                this._nuclearPresentationEvents.set(presentationKey, nowMs);
            }
            // The physics engine may process hundreds of exact ticks between
            // two rendered frames. Start each visible pulse when the event is
            // first observed, never later: cumulative released energy and the
            // ignition pulse therefore advance together. Only carrier/heat
            // aftermath continues after the final release has been booked.
            const age = (nowMs - this._nuclearPresentationEvents.get(presentationKey)) / (1000 / 30);
            const weight = Math.max(1, Number(event.weight) || 1);
            const microscopicMeV = Math.max(0, Number(event.totalMeV) || 0) / weight;
            // Physical event energy and represented population affect only
            // presentation scale. The energy ledger itself remains unchanged.
            const eventScale = 0.95
                + Math.min(0.85, Math.log10(1 + microscopicMeV) * 0.34)
                + Math.min(0.75, Math.log10(weight) / 24);
            const generationHue = (0.075 + Math.min(0.12, generation * 0.018)) % 1;

            if (events && age <= 24) {
                const t = Math.min(1, age / 24);
                const radius = eventScale * (0.35 + 1.9 * Math.sin(Math.PI * t) * Math.exp(-0.7 * t));
                matrix.makeScale(radius, radius, radius);
                matrix.setPosition(event.x, event.y, event.z);
                this._nuclearFlashes.setMatrixAt(flashCount++, matrix);
                const candidateLight = eventScale * (1 - t) * 12;
                if (candidateLight > lightStrength) {
                    lightStrength = candidateLight;
                    this._nuclearLight.position.set(event.x, event.y, event.z);
                    this._nuclearLight.color.setHSL(generationHue, 0.95, 0.68);
                }
            }
            const depositedFraction = Math.max(0, Math.min(1, Number(event.depositedFraction) || 0));
            if (heat && depositedFraction > 1e-6 && age <= 240 && heatCount < 256) {
                const pulse = 1 + 0.075 * Math.sin(age * 0.28 + i * 1.7);
                const radius = eventScale * (0.45 + 2.65 * Math.cbrt(depositedFraction)) * pulse;
                matrix.makeScale(radius, radius, radius);
                matrix.setPosition(event.x, event.y, event.z);
                this._nuclearHeat.setMatrixAt(heatCount, matrix);
                heatColor.setHSL(generationHue, 0.95, Math.max(0.4, 0.62 - age / 1100));
                this._nuclearHeat.setColorAt(heatCount++, heatColor);
            }

            // Two reaction-plane rings make the collision axis and ignition
            // point readable without pretending to resolve a nuclear surface.
            if (events && age <= 48 && shockCount + 1 < 512) {
                axisV.set(event.axisX ?? 1, event.axisY ?? 0, event.axisZ ?? 0).normalize();
                quaternion.setFromUnitVectors(zAxis, axisV);
                for (let ring = 0; ring < 2; ring++) {
                    const radius = eventScale * (0.55 + age * (0.11 + ring * 0.045));
                    matrix.compose(
                        positionV.set(event.x, event.y, event.z),
                        quaternion,
                        scaleV.set(radius, radius, radius),
                    );
                    this._nuclearShockRings.setMatrixAt(shockCount, matrix);
                    carrierColor.setHSL(generationHue + ring * 0.025, 1, ring ? 0.66 : 0.78);
                    this._nuclearShockRings.setColorAt(shockCount++, carrierColor);
                }
            }

            if (radiation) {
                const carriers = [];
                if ((event.neutronMeV || 0) > 0 && age <= 112) {
                    const neutronDirections = Array.isArray(event.neutronDirections)
                        ? event.neutronDirections.filter(direction =>
                            direction && [direction.x, direction.y, direction.z].every(Number.isFinite))
                        : [];
                    carriers.push({
                        speed: 0.17, color: 0x55f4ef,
                        directions: neutronDirections,
                        packets: neutronDirections.length,
                        phase: 0.0,
                    });
                }
                if ((event.gammaMeV || 0) > 0 && age <= 72) {
                    carriers.push({
                        speed: 0.29, color: 0xf472ff,
                        directions: null, packets: 6, phase: 0.45,
                    });
                }
                for (const carrier of carriers) {
                    const frontRadius = eventScale * (0.65 + age * carrier.speed);
                    if (wavefrontCount < 768) {
                        matrix.makeScale(frontRadius, frontRadius, frontRadius);
                        matrix.setPosition(event.x, event.y, event.z);
                        this._nuclearWavefronts.setMatrixAt(wavefrontCount, matrix);
                        carrierColor.setHex(carrier.color);
                        this._nuclearWavefronts.setColorAt(wavefrontCount++, carrierColor);
                    }
                    for (let p = 0; p < carrier.packets && rayCount < maxRays && packetCount < 4096; p++) {
                        const actual = carrier.directions?.[p];
                        const base = actual
                            ? [actual.x, actual.y, actual.z]
                            : directions[(i * 5 + p + Math.round(carrier.phase * 10)) % directions.length];
                        const mag = Math.hypot(base[0], base[1], base[2]) || 1;
                        const dx = base[0] / mag, dy = base[1] / mag, dz = base[2] / mag;
                        const wobble = 0.94 + 0.06 * Math.sin(age * 0.22 + p * 2.1);
                        const packetRadius = frontRadius * wobble;
                        const trailStart = Math.max(0, packetRadius - eventScale * 1.35);
                        const offset = rayCount * 6;
                        position.array[offset] = event.x + dx * trailStart;
                        position.array[offset + 1] = event.y + dy * trailStart;
                        position.array[offset + 2] = event.z + dz * trailStart;
                        position.array[offset + 3] = event.x + dx * packetRadius;
                        position.array[offset + 4] = event.y + dy * packetRadius;
                        position.array[offset + 5] = event.z + dz * packetRadius;
                        carrierColor.setHex(carrier.color);
                        const rgb = [carrierColor.r, carrierColor.g, carrierColor.b];
                        color.array.set(rgb, offset);
                        color.array.set(rgb, offset + 3);
                        rayCount++;

                        const packetSize = carrier.speed > 0.2 ? 0.23 : 0.34;
                        matrix.makeScale(packetSize, packetSize, packetSize);
                        matrix.setPosition(
                            event.x + dx * packetRadius,
                            event.y + dy * packetRadius,
                            event.z + dz * packetRadius,
                        );
                        this._nuclearPackets.setMatrixAt(packetCount, matrix);
                        this._nuclearPackets.setColorAt(packetCount++, carrierColor);
                    }
                }
            }
        }
        for (const key of this._nuclearPresentationEvents.keys()) {
            if (!activePresentationKeys.has(key)) this._nuclearPresentationEvents.delete(key);
        }
        for (const flight of data?.flights || []) {
            if (!radiation || rayCount >= maxRays || packetCount >= 4096) break;
            const t = Math.max(0, Math.min(1, Number(flight.progress) || 0));
            const p = t * t * (3 - 2 * t);
            const x = flight.x0 + (flight.x1 - flight.x0) * p;
            const y = flight.y0 + (flight.y1 - flight.y0) * p;
            const z = flight.z0 + (flight.z1 - flight.z0) * p;
            const trailP = Math.max(0, p - 0.2);
            const offset = rayCount * 6;
            position.array[offset] = flight.x0 + (flight.x1 - flight.x0) * trailP;
            position.array[offset + 1] = flight.y0 + (flight.y1 - flight.y0) * trailP;
            position.array[offset + 2] = flight.z0 + (flight.z1 - flight.z0) * trailP;
            position.array[offset + 3] = x;
            position.array[offset + 4] = y;
            position.array[offset + 5] = z;
            color.array.set([0.28, 1, 0.72], offset);
            color.array.set([0.28, 1, 0.72], offset + 3);
            rayCount++;

            const packetSize = 0.34 + Math.min(0.16, (flight.generation || 0) * 0.025);
            matrix.makeScale(packetSize, packetSize, packetSize);
            matrix.setPosition(x, y, z);
            this._nuclearPackets.setMatrixAt(packetCount, matrix);
            carrierColor.setHex(0x49ffb8);
            this._nuclearPackets.setColorAt(packetCount++, carrierColor);
        }
        this._nuclearFlashes.count = flashCount;
        this._nuclearHeat.count = heatCount;
        this._nuclearWavefronts.count = wavefrontCount;
        this._nuclearPackets.count = packetCount;
        this._nuclearShockRings.count = shockCount;
        this._nuclearFlashes.instanceMatrix.needsUpdate = true;
        this._nuclearHeat.instanceMatrix.needsUpdate = true;
        this._nuclearWavefronts.instanceMatrix.needsUpdate = true;
        this._nuclearPackets.instanceMatrix.needsUpdate = true;
        this._nuclearShockRings.instanceMatrix.needsUpdate = true;
        if (this._nuclearHeat.instanceColor) this._nuclearHeat.instanceColor.needsUpdate = true;
        if (this._nuclearWavefronts.instanceColor) this._nuclearWavefronts.instanceColor.needsUpdate = true;
        if (this._nuclearPackets.instanceColor) this._nuclearPackets.instanceColor.needsUpdate = true;
        if (this._nuclearShockRings.instanceColor) this._nuclearShockRings.instanceColor.needsUpdate = true;
        position.needsUpdate = true;
        color.needsUpdate = true;
        this._nuclearRadiation.geometry.setDrawRange(0, rayCount * 2);
        this._nuclearFlashes.visible = !!events && flashCount > 0;
        this._nuclearHeat.visible = !!heat && heatCount > 0;
        this._nuclearRadiation.visible = !!radiation && rayCount > 0;
        this._nuclearWavefronts.visible = !!radiation && wavefrontCount > 0;
        this._nuclearPackets.visible = !!radiation && packetCount > 0;
        this._nuclearShockRings.visible = !!events && shockCount > 0;
        this._nuclearLight.intensity = events ? Math.min(22, lightStrength) : 0;
        this._nuclearLight.visible = !!events && lightStrength > 0;
    }

    toggleNuclearEvents(on) {
        if (!this._nuclearFlashes) this._buildNuclearEffects();
        this._nuclearFlashes.visible = !!on && this._nuclearFlashes.count > 0;
        this._nuclearShockRings.visible = !!on && this._nuclearShockRings.count > 0;
        this._nuclearLight.visible = !!on && this._nuclearLight.intensity > 0;
    }

    toggleNuclearRadiation(on) {
        if (!this._nuclearRadiation) this._buildNuclearEffects();
        this._nuclearRadiation.visible = !!on && this._nuclearRadiation.geometry.drawRange.count > 0;
        this._nuclearWavefronts.visible = !!on && this._nuclearWavefronts.count > 0;
        this._nuclearPackets.visible = !!on && this._nuclearPackets.count > 0;
    }

    toggleNuclearHeat(on) {
        if (!this._nuclearHeat) this._buildNuclearEffects();
        this._nuclearHeat.visible = !!on && this._nuclearHeat.count > 0;
    }

    toggleNuclearBoundary(on) {
        if (!this._nuclearTransportBoundary) this._buildNuclearEffects();
        this._nuclearTransportBoundary.visible = !!on && this._nuclearTransportBoundary.scale.x > 0;
    }

    // ── Bond Cylinders (thick styled bonds) ────────────────────────────

    _buildBondCylinders() {
        const maxInstances = 1500;
        const geo = new THREE.CylinderGeometry(1, 1, 1, 8);
        geo.translate(0, 0.5, 0); // pivot at base so scaling works from one end
        const mat = new THREE.MeshLambertMaterial({
            color: 0xffffff, transparent: true, opacity: 0.85,
        });
        this._bondCylinders = new THREE.InstancedMesh(geo, mat, maxInstances);
        this._bondCylinders.count = 0;
        this._bondCylinders.visible = true;
        this.scene.add(this._bondCylinders);

        // Add directional light for bond shading (only active in atoms/molecules)
        this._bondLight = new THREE.DirectionalLight(0xffffff, 0.4);
        this._bondLight.position.set(10, 20, 10);
        this._bondLight.visible = true;
        this.scene.add(this._bondLight);
    }

    // Renders covalent bonds as oriented cylinders. Single/double/triple bonds
    // use 1/2/3 parallel cylinders respectively. Each bond creates new Vector3
    // temporaries -- acceptable because atom counts are typically <200.
    updateBondCylinders(atomData) {
        if (!this._bondCylinders) this._buildBondCylinders();
        if (!atomData || atomData.bondCount === 0) { this._bondCylinders.count = 0; return; }

        // Build id→index lookup
        const idToIdx = new Map();
        for (let i = 0; i < atomData.count; i++) idToIdx.set(atomData.ids[i], i);

        // Per-update scratch, reused across every bond/cylinder so the loop
        // below allocates nothing (it can emit up to 1500 cylinders/frame and
        // used to `new` ~10 THREE objects per bond).
        const mat4 = new THREE.Matrix4();
        const up = new THREE.Vector3(0, 1, 0);
        const dir = new THREE.Vector3();
        const quat = new THREE.Quaternion();
        const color = new THREE.Color();
        const cA = new THREE.Color();
        const cB = new THREE.Color();
        const perp = new THREE.Vector3();
        const perp2 = new THREE.Vector3();
        const posV = new THREE.Vector3();
        const scaleV = new THREE.Vector3();
        const AXIS_Z = new THREE.Vector3(0, 0, 1);
        const AXIS_X = new THREE.Vector3(1, 0, 0);
        let instIdx = 0;

        for (let b = 0; b < atomData.bondCount && instIdx < 1500; b++) {
            const idA = atomData.bonds[b * 2];
            const idB = atomData.bonds[b * 2 + 1];
            const iA = idToIdx.get(idA), iB = idToIdx.get(idB);
            if (iA === undefined || iB === undefined) continue;

            const ax = atomData.positions[iA * 3], ay = atomData.positions[iA * 3 + 1], az = atomData.positions[iA * 3 + 2];
            const bx = atomData.positions[iB * 3], by = atomData.positions[iB * 3 + 1], bz = atomData.positions[iB * 3 + 2];
            const dx = bx - ax, dy = by - ay, dz = bz - az;
            const bondLen = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (bondLen < 1e-10) continue;

            dir.set(dx, dy, dz).normalize();
            quat.setFromUnitVectors(up, dir);

            // Color: blend CPK colors of bonded atoms. .setRGB(r,g,b) matches
            // `new THREE.Color(r,g,b)` exactly (both use the working space).
            cA.setRGB(atomData.colors[iA * 3], atomData.colors[iA * 3 + 1], atomData.colors[iA * 3 + 2]);
            cB.setRGB(atomData.colors[iB * 3], atomData.colors[iB * 3 + 1], atomData.colors[iB * 3 + 2]);
            color.copy(cA).lerp(cB, 0.5);

            const order = atomData.bondOrders ? atomData.bondOrders[b] : 1;
            // Aromatic bonds carry the sentinel order 1.5 (P0-13): render them
            // as one full bond plus a thinner parallel "delocalised" cylinder,
            // visually between a single and a hard double.
            const isAromatic = order >= 1.5 && order < 2;

            if (isAromatic) {
                // Aromatic: full-width main cylinder + thin offset companion.
                perp.crossVectors(dir, AXIS_Z);
                if (perp.lengthSq() < 0.001) perp.crossVectors(dir, AXIS_X);
                perp.normalize().multiplyScalar(0.16);
                // Main bond (centred).
                mat4.compose(posV.set(ax, ay, az), quat, scaleV.set(0.15, bondLen, 0.15));
                this._bondCylinders.setMatrixAt(instIdx, mat4);
                this._bondCylinders.setColorAt(instIdx, color);
                instIdx++;
                // Thin delocalised companion (offset to one side).
                if (instIdx < 1500) {
                    const ox = ax + perp.x, oy = ay + perp.y, oz = az + perp.z;
                    mat4.compose(posV.set(ox, oy, oz), quat, scaleV.set(0.07, bondLen, 0.07));
                    this._bondCylinders.setMatrixAt(instIdx, mat4);
                    this._bondCylinders.setColorAt(instIdx, color);
                    instIdx++;
                }
            } else if (order < 2) {
                // Single bond: 1 cylinder, radius 0.15
                mat4.compose(posV.set(ax, ay, az), quat, scaleV.set(0.15, bondLen, 0.15));
                this._bondCylinders.setMatrixAt(instIdx, mat4);
                this._bondCylinders.setColorAt(instIdx, color);
                instIdx++;
            } else if (order < 3) {
                // Double bond: 2 parallel cylinders offset ±0.18
                perp.crossVectors(dir, AXIS_Z);
                if (perp.lengthSq() < 0.001) perp.crossVectors(dir, AXIS_X);
                perp.normalize().multiplyScalar(0.18);
                for (let s = -1; s <= 1; s += 2) {
                    const ox = ax + perp.x * s, oy = ay + perp.y * s, oz = az + perp.z * s;
                    mat4.compose(posV.set(ox, oy, oz), quat, scaleV.set(0.12, bondLen, 0.12));
                    if (instIdx < 1500) {
                        this._bondCylinders.setMatrixAt(instIdx, mat4);
                        this._bondCylinders.setColorAt(instIdx, color);
                        instIdx++;
                    }
                }
            } else if (order >= 3) {
                // Triple bond: 3 cylinders in triangle arrangement
                perp.crossVectors(dir, AXIS_Z);
                if (perp.lengthSq() < 0.001) perp.crossVectors(dir, AXIS_X);
                perp.normalize();
                perp2.crossVectors(dir, perp).normalize();
                const angles = [0, 2 * Math.PI / 3, 4 * Math.PI / 3];
                for (const angle of angles) {
                    const offX = Math.cos(angle) * 0.2, offY = Math.sin(angle) * 0.2;
                    const ox = ax + perp.x * offX + perp2.x * offY;
                    const oy = ay + perp.y * offX + perp2.y * offY;
                    const oz = az + perp.z * offX + perp2.z * offY;
                    mat4.compose(posV.set(ox, oy, oz), quat, scaleV.set(0.10, bondLen, 0.10));
                    if (instIdx < 1500) {
                        this._bondCylinders.setMatrixAt(instIdx, mat4);
                        this._bondCylinders.setColorAt(instIdx, color);
                        instIdx++;
                    }
                }
            }
        }

        this._bondCylinders.count = instIdx;
        this._bondCylinders.instanceMatrix.needsUpdate = true;
        if (this._bondCylinders.instanceColor) this._bondCylinders.instanceColor.needsUpdate = true;
    }

    toggleBondCylinders(on) {
        if (!this._bondCylinders) this._buildBondCylinders();
        this._bondCylinders.visible = on;
        if (this._bondLight) this._bondLight.visible = on;
    }

    // ── Orbital Shell Boundaries (translucent spheres per n) ──────────

    _buildOrbitalShells() {
        const maxShells = 1024;
        const geo = new THREE.SphereGeometry(1, 24, 16);
        const mat = new THREE.MeshBasicMaterial({
            color: 0x66bfff, transparent: true, opacity: 0.05,
            depthWrite: false, side: THREE.DoubleSide,
        });
        this._orbitalShells = new THREE.InstancedMesh(geo, mat, maxShells);
        this._orbitalShells.count = 0;
        this._orbitalShells.visible = false; // default OFF
        this._orbitalShells.renderOrder = -3;
        this.scene.add(this._orbitalShells);
    }

    updateOrbitalShells(atomData, electronConfigFn, slaterZeffFn, a0Display) {
        if (!this._orbitalShells) this._buildOrbitalShells();
        if (!atomData || atomData.count === 0 || !electronConfigFn) {
            this._orbitalShells.count = 0;
            return;
        }

        const mat4 = new THREE.Matrix4();
        const shellColors = {
            1: new THREE.Color(0x66bfff),  // blue
            2: new THREE.Color(0x4de673),  // green
            3: new THREE.Color(0xffb333),  // orange
            4: new THREE.Color(0xd94db3),  // pink
        };
        let instIdx = 0;

        for (let i = 0; i < atomData.count && instIdx < 1024; i++) {
            const Z = atomData.atomicNums[i];
            const config = electronConfigFn(Z);
            const seenN = new Set();
            for (const sub of config) {
                if (seenN.has(sub.n)) continue;
                seenN.add(sub.n);
                const zEff = slaterZeffFn(Z, sub.n, sub.l);
                const radius = (sub.n * sub.n / zEff) * a0Display;
                const cx = atomData.positions[i * 3];
                const cy = atomData.positions[i * 3 + 1];
                const cz = atomData.positions[i * 3 + 2];

                mat4.makeScale(radius, radius, radius);
                mat4.setPosition(cx, cy, cz);
                this._orbitalShells.setMatrixAt(instIdx, mat4);

                const col = shellColors[Math.min(sub.n, 4)] || shellColors[4];
                this._orbitalShells.setColorAt(instIdx, col);
                instIdx++;
                if (instIdx >= 1024) break;
            }
        }

        this._orbitalShells.count = instIdx;
        this._orbitalShells.instanceMatrix.needsUpdate = true;
        if (this._orbitalShells.instanceColor) this._orbitalShells.instanceColor.needsUpdate = true;
    }

    toggleOrbitalShells(on) {
        if (!this._orbitalShells) this._buildOrbitalShells();
        this._orbitalShells.visible = on;
    }

    // ── Orbital Lobes (p/d/f shaped meshes) ───────────────────────────

    _buildOrbitalLobes() {
        const maxLobes = 2000;
        // Elongated ellipsoid for p-orbital lobe shape
        const baseSphere = new THREE.SphereGeometry(1, 12, 8);
        const pos = baseSphere.attributes.position;
        for (let i = 0; i < pos.count; i++) {
            const x = pos.getX(i), y = pos.getY(i), z = pos.getZ(i);
            pos.setXYZ(i, x * 0.5, y * 1.6, z * 0.5); // elongated along Y
        }
        pos.needsUpdate = true;
        baseSphere.computeVertexNormals();

        const mat = new THREE.MeshBasicMaterial({
            color: 0xffffff, transparent: true, opacity: 0.08,
            depthWrite: false, side: THREE.DoubleSide,
            blending: THREE.AdditiveBlending,
        });
        this._orbitalLobes = new THREE.InstancedMesh(baseSphere, mat, maxLobes);
        this._orbitalLobes.count = 0;
        this._orbitalLobes.visible = false; // default OFF
        this._orbitalLobes.renderOrder = -4;
        this.scene.add(this._orbitalLobes);
    }

    updateOrbitalLobes(atomData, electronConfigFn, slaterZeffFn, a0Display) {
        if (!this._orbitalLobes) this._buildOrbitalLobes();
        if (!atomData || atomData.count === 0 || !electronConfigFn) {
            this._orbitalLobes.count = 0;
            return;
        }

        const mat4 = new THREE.Matrix4();
        const lobeColors = {
            1: new THREE.Color(0x30ee55), // p — green
            2: new THREE.Color(0xffaa22), // d — gold
            3: new THREE.Color(0xdd44bb), // f — magenta
        };
        let instIdx = 0;

        for (let i = 0; i < atomData.count && instIdx < 2000; i++) {
            const Z = atomData.atomicNums[i];
            const config = electronConfigFn(Z);
            const maxN = Math.max(...config.map(s => s.n));
            const cx = atomData.positions[i * 3];
            const cy = atomData.positions[i * 3 + 1];
            const cz = atomData.positions[i * 3 + 2];

            // Only show lobes for valence shell (outermost occupied orbitals)
            for (const sub of config) {
                if (sub.l === 0) continue; // s-orbitals are spherical (no lobes)
                const isValence = (sub.n === maxN) || (sub.n === maxN - 1 && sub.l >= 2);
                if (!isValence) continue;

                const zEff = slaterZeffFn(Z, sub.n, sub.l);
                const radius = (sub.n * sub.n / zEff) * a0Display * 0.6;
                const col = lobeColors[sub.l] || lobeColors[3];

                // Generate lobe orientations based on l
                const axes = this._getLobeAxes(sub.l);
                for (const axis of axes) {
                    if (instIdx >= 2000) break;
                    // Place lobe: scale by radius, rotate to axis orientation, translate to atom
                    const quat = new THREE.Quaternion();
                    const up = new THREE.Vector3(0, 1, 0);
                    const target = new THREE.Vector3(axis[0], axis[1], axis[2]);
                    quat.setFromUnitVectors(up, target.normalize());

                    mat4.compose(
                        new THREE.Vector3(cx, cy, cz),
                        quat,
                        new THREE.Vector3(radius * 0.5, radius, radius * 0.5)
                    );
                    this._orbitalLobes.setMatrixAt(instIdx, mat4);
                    this._orbitalLobes.setColorAt(instIdx, col);
                    instIdx++;

                    // Mirror lobe (opposite direction)
                    if (instIdx >= 2000) break;
                    target.negate();
                    quat.setFromUnitVectors(up, target.normalize());
                    mat4.compose(
                        new THREE.Vector3(cx, cy, cz),
                        quat,
                        new THREE.Vector3(radius * 0.5, radius, radius * 0.5)
                    );
                    this._orbitalLobes.setMatrixAt(instIdx, mat4);
                    this._orbitalLobes.setColorAt(instIdx, col);
                    instIdx++;
                }
            }
        }

        this._orbitalLobes.count = instIdx;
        this._orbitalLobes.instanceMatrix.needsUpdate = true;
        if (this._orbitalLobes.instanceColor) this._orbitalLobes.instanceColor.needsUpdate = true;
    }

    _getLobeAxes(l) {
        if (l === 1) {
            // p-orbitals: px, py, pz
            return [[1, 0, 0], [0, 1, 0], [0, 0, 1]];
        } else if (l === 2) {
            // d-orbitals: dz², dxz, dyz, dx²-y², dxy (simplified to 4 main axes)
            return [[1, 0, 0], [0, 1, 0], [0, 0, 1], [0.707, 0.707, 0]];
        } else {
            // f-orbitals: 6 axes for symmetry
            return [[1, 0, 0], [0, 1, 0], [0, 0, 1], [0.707, 0.707, 0], [0.707, 0, 0.707], [0, 0.707, 0.707]];
        }
    }

    toggleOrbitalLobes(on) {
        if (!this._orbitalLobes) this._buildOrbitalLobes();
        this._orbitalLobes.visible = on;
    }

    // ── Per-Atom Force Arrows ─────────────────────────────────────────

    _buildAEForceArrows() {
        const maxAtoms = 512;
        const createArrowSet = (color) => {
            // Shaft plus a two-segment V head: direction must remain readable
            // even for single-color component layers.
            const vertices = new Float32Array(maxAtoms * 18);
            const geo = new THREE.BufferGeometry();
            geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
            geo.setDrawRange(0, 0);
            const mat = new THREE.LineBasicMaterial({ color, linewidth: 2, transparent: true, opacity: 0.8 });
            const lines = new THREE.LineSegments(geo, mat);
            lines.visible = false;
            this.scene.add(lines);
            return lines;
        };

        this._aeForceIonic = createArrowSet(0xff4444); // red for Coulomb
        this._aeForceVdw = createArrowSet(0x44ff44); // green for vdW
        this._aeForceBond = createArrowSet(0xff8844); // orange for bond
        this._aeForceHBond = createArrowSet(0x38bdf8); // blue for H-bond
        this._aeForceAngle = createArrowSet(0xfacc15); // yellow for angle strain
        this._aeForceDipole = createArrowSet(0xe879f9); // magenta for dipole force
        this._aeForceNet = createArrowSet(0xffffff); // white for net
    }

    updateAEForces(positions, forceData, count) {
        if (!this._aeForceIonic) this._buildAEForceArrows();
        if (!forceData || count === 0) {
            [this._aeForceIonic, this._aeForceVdw, this._aeForceBond,
                this._aeForceHBond, this._aeForceAngle, this._aeForceDipole,
                this._aeForceNet].forEach(l => l.geometry.setDrawRange(0, 0));
            return;
        }

        const scale = 8.0; // visual scale factor for force arrows
        const n = Math.min(count, 512);

        const updateArrows = (lines, forceArr) => {
            const posAttr = lines.geometry.getAttribute('position');
            let drawn = 0;
            for (let i = 0; i < n; i++) {
                const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
                const fx = forceArr[i * 3], fy = forceArr[i * 3 + 1], fz = forceArr[i * 3 + 2];

                // Log-compress force magnitude for visibility
                const fmag = Math.sqrt(fx * fx + fy * fy + fz * fz);
                if (fmag <= 1e-10) continue;
                const logScale = scale * Math.log1p(fmag) / fmag;
                const tx = px + fx * logScale;
                const ty = py + fy * logScale;
                const tz = pz + fz * logScale;
                writeVArrow(posAttr.array, drawn * 18, px, py, pz, tx, ty, tz, 0.16, 0.75);
                drawn++;
            }
            posAttr.needsUpdate = true;
            lines.geometry.setDrawRange(0, drawn * 6);
        };

        updateArrows(this._aeForceIonic, forceData.ionic);
        updateArrows(this._aeForceVdw, forceData.vdw);
        updateArrows(this._aeForceBond, forceData.bond);
        updateArrows(this._aeForceHBond, forceData.hbond);
        updateArrows(this._aeForceAngle, forceData.angle);
        updateArrows(this._aeForceDipole, forceData.dipole);
        updateArrows(this._aeForceNet, forceData.net);
    }

    toggleAEForceIonic(on) { if (!this._aeForceIonic) this._buildAEForceArrows(); this._aeForceIonic.visible = on; }
    toggleAEForceVdw(on)   { if (!this._aeForceVdw)   this._buildAEForceArrows(); this._aeForceVdw.visible = on; }
    toggleAEForceBond(on)  { if (!this._aeForceBond)  this._buildAEForceArrows(); this._aeForceBond.visible = on; }
    toggleAEForceHBond(on) { if (!this._aeForceHBond) this._buildAEForceArrows(); this._aeForceHBond.visible = on; }
    toggleAEForceAngle(on) { if (!this._aeForceAngle) this._buildAEForceArrows(); this._aeForceAngle.visible = on; }
    toggleAEForceDipole(on){ if (!this._aeForceDipole)this._buildAEForceArrows(); this._aeForceDipole.visible = on; }
    toggleAEForceNet(on)   { if (!this._aeForceNet)   this._buildAEForceArrows(); this._aeForceNet.visible = on; }

    // ── Per-Atom Dipole-Moment Arrows ─────────────────────────────────

    _buildAEDipoleArrows() {
        const maxAtoms = 512;
        const vertices = new Float32Array(maxAtoms * 18); // shaft + V head
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineBasicMaterial({ color: 0xe879f9, linewidth: 2, transparent: true, opacity: 0.85 });
        this._aeDipoles = new THREE.LineSegments(geo, mat);
        this._aeDipoles.frustumCulled = false;
        this._aeDipoles.visible = false;
        this.scene.add(this._aeDipoles);
    }

    updateAEDipoles(positions, dipoles, count) {
        if (!this._aeDipoles) this._buildAEDipoleArrows();
        if (!dipoles || count === 0) {
            this._aeDipoles.geometry.setDrawRange(0, 0);
            return;
        }
        const scale = 2.0; // visual scale; dipole magnitudes are O(bond length · Δχ)
        const n = Math.min(count, 512);
        const posAttr = this._aeDipoles.geometry.getAttribute('position');
        let drawn = 0;
        for (let i = 0; i < n; i++) {
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            const mx = dipoles[i * 3], my = dipoles[i * 3 + 1], mz = dipoles[i * 3 + 2];
            const mag = Math.sqrt(mx * mx + my * my + mz * mz);
            if (mag <= 1e-10) continue;
            // Log-compress like the force arrows so large dipoles stay on screen
            const k = scale * Math.log1p(mag) / mag;
            const tx = px + mx * k, ty = py + my * k, tz = pz + mz * k;
            writeVArrow(posAttr.array, drawn * 18, px, py, pz, tx, ty, tz, 0.14, 0.6);
            drawn++;
        }
        posAttr.needsUpdate = true;
        this._aeDipoles.geometry.setDrawRange(0, drawn * 6);
    }

    toggleAEDipoles(on) {
        if (!this._aeDipoles) this._buildAEDipoleArrows();
        this._aeDipoles.visible = on;
        if (!on) this._aeDipoles.geometry.setDrawRange(0, 0);
    }

    // ── Hydrogen-Bond Dashed Lines (donor-H···acceptor) ───────────────

    _buildHBondLines(requiredPairs = 256) {
        const oldLines = this._hbondLines;
        const visible = oldLines?.visible ?? false;
        if (oldLines) {
            this.scene.remove(oldLines);
            oldLines.geometry.dispose();
            oldLines.material.dispose();
        }
        // Grow only when the live geometry demands it. H-bond eligibility is
        // pairwise, so a fixed 256-line pool could silently discard otherwise
        // valid overlay records even while the engine continued computing them.
        const capacity = Math.max(256, 2 ** Math.ceil(Math.log2(Math.max(1, requiredPairs))));
        const vertices = new Float32Array(capacity * 6); // 2 verts per pair
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineDashedMaterial({
            color: 0x7dd3fc, dashSize: 0.35, gapSize: 0.25,
            transparent: true, opacity: 0.85,
        });
        this._hbondLines = new THREE.LineSegments(geo, mat);
        this._hbondLines.frustumCulled = false;
        this._hbondLines.visible = visible;
        this._hbondCapacity = capacity;
        this.scene.add(this._hbondLines);
    }

    updateHBondLines(segments, count) {
        if (!this._hbondLines) this._buildHBondLines();
        const requested = Math.max(0, Math.floor(Number(count) || 0));
        if (requested > this._hbondCapacity) this._buildHBondLines(requested);
        const n = Math.min(requested, this._hbondCapacity);
        if (!segments || n === 0) {
            this._hbondLines.geometry.setDrawRange(0, 0);
            return;
        }
        const posAttr = this._hbondLines.geometry.getAttribute('position');
        posAttr.array.set(segments.subarray(0, n * 6));
        posAttr.needsUpdate = true;
        this._hbondLines.geometry.setDrawRange(0, n * 2);
        // LineDashedMaterial requires per-vertex line distances or nothing renders.
        this._hbondLines.computeLineDistances();
    }

    toggleHBondLines(on) {
        if (!this._hbondLines) this._buildHBondLines();
        this._hbondLines.visible = on;
        if (!on) this._hbondLines.geometry.setDrawRange(0, 0);
    }

    /**
     * Bulk visibility — called by Viewport.setEngineMode's hideAllOverlays().
     * Touches every mesh/group this renderer owns. Nulls left uncreated stay null.
     */
    setAllVisible(on) {
        if (this.bondLines) this.bondLines.visible = on;
        if (this._bondCylinders) this._bondCylinders.visible = on;
        if (this._bondLight) this._bondLight.visible = on;
        if (this._nucleusShells) this._nucleusShells.visible = on;
        if (this._orbitalShells) this._orbitalShells.visible = on;
        if (this._orbitalLobes) this._orbitalLobes.visible = on;
        if (this._elementLabels) this._elementLabels.visible = on;
        if (this._aeForceIonic) this._aeForceIonic.visible = on;
        if (this._aeForceVdw) this._aeForceVdw.visible = on;
        if (this._aeForceBond) this._aeForceBond.visible = on;
        if (this._aeForceHBond) this._aeForceHBond.visible = on;
        if (this._aeForceAngle) this._aeForceAngle.visible = on;
        if (this._aeForceDipole) this._aeForceDipole.visible = on;
        if (this._aeForceNet) this._aeForceNet.visible = on;
        if (this._aeDipoles) this._aeDipoles.visible = on;
        if (this._hbondLines) this._hbondLines.visible = on;
        if (this._nuclearFlashes) this._nuclearFlashes.visible = on;
        if (this._nuclearHeat) this._nuclearHeat.visible = on;
        if (this._nuclearRadiation) this._nuclearRadiation.visible = on;
        if (this._nuclearWavefronts) this._nuclearWavefronts.visible = on;
        if (this._nuclearPackets) this._nuclearPackets.visible = on;
        if (this._nuclearShockRings) this._nuclearShockRings.visible = on;
        if (this._nuclearTransportBoundary) this._nuclearTransportBoundary.visible = on;
        if (this._nuclearLight) this._nuclearLight.visible = on;
    }

    /**
     * Atom/molecule subset of visibility — toggled when entering atoms/molecules mode.
     * Scale 1 (PE) is NOT atom-mode so bondCylinders/bondLight/nucleusShells/labels stay off.
     */
    setAtomMolVisible(on) {
        if (this._bondCylinders) this._bondCylinders.visible = on;
        if (this._bondLight) this._bondLight.visible = on;
        if (this._nucleusShells) this._nucleusShells.visible = on;
        if (this._elementLabels) this._elementLabels.visible = on;
    }

    // ── Element Labels (Scale 2 — Atom mode) ──────────────────────────
    // Sprite-based text labels that always face the camera. Each label
    // is a canvas-textured sprite positioned at the atom center.

    _makeTextSprite(text, color = '#ffffff', fontSize = 48) {
        let canvas;
        if (this._canvasPool && this._canvasPool.length > 0) {
            canvas = this._canvasPool.pop();
        } else {
            canvas = document.createElement('canvas');
            canvas.width = 128;
            canvas.height = 64;
        }
        const ctx = canvas.getContext('2d');
        ctx.clearRect(0, 0, 128, 64);
        ctx.font = `bold ${fontSize}px 'Inter', 'Segoe UI', sans-serif`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        // Outline for readability
        ctx.strokeStyle = '#000000';
        ctx.lineWidth = 4;
        ctx.strokeText(text, 64, 32);
        ctx.fillStyle = color;
        ctx.fillText(text, 64, 32);
        const texture = new THREE.CanvasTexture(canvas);
        texture.minFilter = THREE.LinearFilter;
        const mat = new THREE.SpriteMaterial({ map: texture, transparent: true, depthTest: false });
        const sprite = new THREE.Sprite(mat);
        sprite.scale.set(2.8, 1.4, 1);
        return sprite;
    }

    /**
     * Update element labels — creates/recycles sprites to match atom data.
     * @param {Array<{x,y,z,symbol,color}>} labels — array of label descriptors
     */
    updateElementLabels(labels) {
        if (!this._elementLabels) {
            this._elementLabels = new THREE.Group();
            this._elementLabels.visible = true;
            this.scene.add(this._elementLabels);
            this._labelPool = [];
        }

        const group = this._elementLabels;
        const pool = this._labelPool;
        const needed = labels ? labels.length : 0;

        // Hide excess sprites
        for (let i = needed; i < pool.length; i++) {
            pool[i].visible = false;
        }

        if (!labels) return;

        for (let i = 0; i < needed; i++) {
            const lb = labels[i];
            let sprite;
            if (i < pool.length) {
                sprite = pool[i];
                // Update texture if symbol changed
                if (sprite._symbol !== lb.symbol) {
                    if (this._canvasPool && sprite.material.map.image) this._canvasPool.push(sprite.material.map.image);
                    sprite.material.map.dispose();
                    sprite.material.dispose();
                    const newSprite = this._makeTextSprite(lb.symbol, lb.color || '#ffffff');
                    newSprite._symbol = lb.symbol;
                    // Replace in pool and group
                    group.remove(sprite);
                    pool[i] = newSprite;
                    group.add(newSprite);
                    sprite = newSprite;
                }
            } else {
                sprite = this._makeTextSprite(lb.symbol, lb.color || '#ffffff');
                sprite._symbol = lb.symbol;
                pool.push(sprite);
                group.add(sprite);
            }
            sprite.position.set(lb.x, lb.y + 1.8, lb.z); // offset above atom center
            sprite.visible = true;
        }
    }

    toggleElementLabels(on) {
        if (this._elementLabels) this._elementLabels.visible = on;
    }

    clearElementLabels() {
        if (!this._elementLabels) return;
        for (const sprite of this._labelPool) {
            if (this._canvasPool && sprite.material.map.image) this._canvasPool.push(sprite.material.map.image);
            sprite.material.map.dispose();
            sprite.material.dispose();
        }
        this.scene.remove(this._elementLabels);
        this._elementLabels = null;
        this._labelPool = [];
    }

    /**
     * Reset draw ranges / instance counts to 0 on every owned visual.
     * Called by viewport.clearMolecularMeshes() at scenario boundaries.
     */
    clearMolecularMeshes() {
        if (this._bondCylinders) this._bondCylinders.count = 0;
        if (this.bondLines) this.bondLines.geometry.setDrawRange(0, 0);
        if (this._nucleusShells) this._nucleusShells.count = 0;
        if (this._orbitalShells) this._orbitalShells.count = 0;
        if (this._orbitalLobes) this._orbitalLobes.count = 0;
        if (this._nuclearFlashes) this._nuclearFlashes.count = 0;
        if (this._nuclearHeat) this._nuclearHeat.count = 0;
        if (this._nuclearRadiation) this._nuclearRadiation.geometry.setDrawRange(0, 0);
        if (this._nuclearWavefronts) this._nuclearWavefronts.count = 0;
        if (this._nuclearPackets) this._nuclearPackets.count = 0;
        if (this._nuclearShockRings) this._nuclearShockRings.count = 0;
        if (this._nuclearTransportBoundary) this._nuclearTransportBoundary.visible = false;
        if (this._nuclearLight) { this._nuclearLight.intensity = 0; this._nuclearLight.visible = false; }
        this._nuclearPresentationEvents.clear();
        if (this._aeForceIonic) {
            [this._aeForceIonic, this._aeForceVdw, this._aeForceBond,
                this._aeForceHBond, this._aeForceAngle, this._aeForceDipole,
                this._aeForceNet]
                .forEach(l => l.geometry.setDrawRange(0, 0));
        }
        if (this._aeDipoles) this._aeDipoles.geometry.setDrawRange(0, 0);
        if (this._hbondLines) this._hbondLines.geometry.setDrawRange(0, 0);
    }

    /**
     * Tear down every mesh + material + texture this renderer owns, and
     * remove them from the scene. Called from viewport.dispose().
     */
    dispose() {
        const scene = this.scene;
        const disposeMesh = (obj) => {
            if (!obj) return;
            scene.remove(obj);
            if (obj.geometry) obj.geometry.dispose();
            if (obj.material) {
                if (obj.material.map) obj.material.map.dispose();
                obj.material.dispose();
            }
        };
        disposeMesh(this.bondLines);         this.bondLines = null;
        disposeMesh(this._bondCylinders);    this._bondCylinders = null;
        if (this._bondLight) { scene.remove(this._bondLight); this._bondLight = null; }
        disposeMesh(this._nucleusShells);    this._nucleusShells = null;
        disposeMesh(this._orbitalShells);    this._orbitalShells = null;
        disposeMesh(this._orbitalLobes);     this._orbitalLobes = null;
        disposeMesh(this._aeForceIonic);     this._aeForceIonic = null;
        disposeMesh(this._aeForceVdw);       this._aeForceVdw = null;
        disposeMesh(this._aeForceBond);      this._aeForceBond = null;
        disposeMesh(this._aeForceHBond);     this._aeForceHBond = null;
        disposeMesh(this._aeForceAngle);     this._aeForceAngle = null;
        disposeMesh(this._aeForceDipole);    this._aeForceDipole = null;
        disposeMesh(this._aeForceNet);       this._aeForceNet = null;
        disposeMesh(this._aeDipoles);        this._aeDipoles = null;
        disposeMesh(this._hbondLines);       this._hbondLines = null;
        this._hbondCapacity = 0;
        disposeMesh(this._nuclearFlashes);   this._nuclearFlashes = null;
        disposeMesh(this._nuclearHeat);      this._nuclearHeat = null;
        disposeMesh(this._nuclearRadiation); this._nuclearRadiation = null;
        disposeMesh(this._nuclearWavefronts); this._nuclearWavefronts = null;
        disposeMesh(this._nuclearPackets);    this._nuclearPackets = null;
        disposeMesh(this._nuclearShockRings); this._nuclearShockRings = null;
        disposeMesh(this._nuclearTransportBoundary); this._nuclearTransportBoundary = null;
        if (this._nuclearLight) { scene.remove(this._nuclearLight); this._nuclearLight = null; }
        this._nuclearPresentationEvents.clear();
        this.clearElementLabels();
    }
}
