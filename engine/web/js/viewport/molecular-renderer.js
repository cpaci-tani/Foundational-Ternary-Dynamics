/**
 * Molecular renderer — Scale 2 (atoms) and Scale 3 (molecules).
 *
 * Owns every visual that rides on an atom or a bond:
 *   - bondLines         thin line segments, Scale 2 quick view
 *   - _bondCylinders    instanced thick cylinders, single/double/triple
 *   - _bondLight        directional light added for bond shading
 *   - _nucleusShells    strong-force glow spheres scaled by A^(1/3)
 *   - _orbitalShells    translucent spheres, one per principal quantum n
 *   - _orbitalLobes     p/d/f lobes for the valence shell
 *   - _aeForceIonic/Vdw/Bond/Net  per-atom force arrows
 *   - _elementLabels    sprite-based chemical symbols
 *
 * Extracted from viewport.js as Wave 2 ticket 4 of the large-file refactor
 * (see docs/SPEC_REFACTOR_LARGE_FILES.md §5). Every body is preserved
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
        this._aeForceNet = null;
        this._aeDipoles = null;
        this._hbondLines = null;
        this._elementLabels = null;
        this._labelPool = null;
        // Optional function (Z) -> neutron count. External callers may
        // assign through `viewport._molRenderer._defaultNeutronCount`; if
        // left null the `updateNucleusShells` pathway falls back to the
        // Math.round(Z * 1.2) approximation used before extraction.
        this._defaultNeutronCount = null;
    }

    // ── Bond Lines (Scale 2 — Atom mode) ──────────────────────────────

    _buildBondLines() {
        const MAX_BONDS = 500;
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

        for (let b = 0; b < n; b++) {
            const idxA = atomData.bonds[b * 2];
            const idxB = atomData.bonds[b * 2 + 1];

            // Start vertex (atom A position)
            posAttr.array[b * 6] = atomData.positions[idxA * 3];
            posAttr.array[b * 6 + 1] = atomData.positions[idxA * 3 + 1];
            posAttr.array[b * 6 + 2] = atomData.positions[idxA * 3 + 2];
            // End vertex (atom B position)
            posAttr.array[b * 6 + 3] = atomData.positions[idxB * 3];
            posAttr.array[b * 6 + 4] = atomData.positions[idxB * 3 + 1];
            posAttr.array[b * 6 + 5] = atomData.positions[idxB * 3 + 2];

            // Bond color: blend the two atom colors
            const rA = atomData.colors[idxA * 3], gA = atomData.colors[idxA * 3 + 1], bA = atomData.colors[idxA * 3 + 2];
            const rB = atomData.colors[idxB * 3], gB = atomData.colors[idxB * 3 + 1], bB = atomData.colors[idxB * 3 + 2];
            colAttr.array[b * 6] = rA;
            colAttr.array[b * 6 + 1] = gA;
            colAttr.array[b * 6 + 2] = bA;
            colAttr.array[b * 6 + 3] = rB;
            colAttr.array[b * 6 + 4] = gB;
            colAttr.array[b * 6 + 5] = bB;
        }

        posAttr.needsUpdate = true;
        colAttr.needsUpdate = true;
        this.bondLines.geometry.setDrawRange(0, n * 2);
    }

    toggleBondLines(on) {
        if (!this.bondLines) this._buildBondLines();
        this.bondLines.visible = on;
        if (!on) this.bondLines.geometry.setDrawRange(0, 0);
    }

    // ── Nucleus Shells (strong force glow spheres) ─────────────────────

    _buildNucleusShells() {
        const maxShells = 100;
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
        const n = Math.min(atomData.count, 100);
        const mat4 = new THREE.Matrix4();
        for (let i = 0; i < n; i++) {
            const Z = atomData.atomicNums[i];
            const N_neutrons = this._defaultNeutronCount ? this._defaultNeutronCount(Z) : Math.round(Z * 1.2);
            const A = Z + N_neutrons;
            const radius = 0.5 * Math.cbrt(Math.max(A, 1)) * 1.8;
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

        const mat4 = new THREE.Matrix4();
        const up = new THREE.Vector3(0, 1, 0);
        const dir = new THREE.Vector3();
        const quat = new THREE.Quaternion();
        const color = new THREE.Color();
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

            // Color: blend CPK colors of bonded atoms
            const cA = new THREE.Color(atomData.colors[iA * 3], atomData.colors[iA * 3 + 1], atomData.colors[iA * 3 + 2]);
            const cB = new THREE.Color(atomData.colors[iB * 3], atomData.colors[iB * 3 + 1], atomData.colors[iB * 3 + 2]);
            color.copy(cA).lerp(cB, 0.5);

            const order = atomData.bondOrders ? atomData.bondOrders[b] : 1;
            // Aromatic bonds carry the sentinel order 1.5 (P0-13): render them
            // as one full bond plus a thinner parallel "delocalised" cylinder,
            // visually between a single and a hard double.
            const isAromatic = order >= 1.5 && order < 2;

            if (isAromatic) {
                // Aromatic: full-width main cylinder + thin offset companion.
                const perp = new THREE.Vector3().crossVectors(dir, new THREE.Vector3(0, 0, 1));
                if (perp.lengthSq() < 0.001) perp.crossVectors(dir, new THREE.Vector3(1, 0, 0));
                perp.normalize().multiplyScalar(0.16);
                // Main bond (centred).
                mat4.compose(new THREE.Vector3(ax, ay, az), quat, new THREE.Vector3(0.15, bondLen, 0.15));
                this._bondCylinders.setMatrixAt(instIdx, mat4);
                this._bondCylinders.setColorAt(instIdx, color);
                instIdx++;
                // Thin delocalised companion (offset to one side).
                if (instIdx < 1500) {
                    const ox = ax + perp.x, oy = ay + perp.y, oz = az + perp.z;
                    mat4.compose(new THREE.Vector3(ox, oy, oz), quat, new THREE.Vector3(0.07, bondLen, 0.07));
                    this._bondCylinders.setMatrixAt(instIdx, mat4);
                    this._bondCylinders.setColorAt(instIdx, color);
                    instIdx++;
                }
            } else if (order < 2) {
                // Single bond: 1 cylinder, radius 0.15
                mat4.compose(new THREE.Vector3(ax, ay, az), quat, new THREE.Vector3(0.15, bondLen, 0.15));
                this._bondCylinders.setMatrixAt(instIdx, mat4);
                this._bondCylinders.setColorAt(instIdx, color);
                instIdx++;
            } else if (order < 3) {
                // Double bond: 2 parallel cylinders offset ±0.18
                const perp = new THREE.Vector3().crossVectors(dir, new THREE.Vector3(0, 0, 1));
                if (perp.lengthSq() < 0.001) perp.crossVectors(dir, new THREE.Vector3(1, 0, 0));
                perp.normalize().multiplyScalar(0.18);
                for (let s = -1; s <= 1; s += 2) {
                    const ox = ax + perp.x * s, oy = ay + perp.y * s, oz = az + perp.z * s;
                    mat4.compose(new THREE.Vector3(ox, oy, oz), quat, new THREE.Vector3(0.12, bondLen, 0.12));
                    if (instIdx < 1500) {
                        this._bondCylinders.setMatrixAt(instIdx, mat4);
                        this._bondCylinders.setColorAt(instIdx, color);
                        instIdx++;
                    }
                }
            } else if (order >= 3) {
                // Triple bond: 3 cylinders in triangle arrangement
                const perp = new THREE.Vector3().crossVectors(dir, new THREE.Vector3(0, 0, 1));
                if (perp.lengthSq() < 0.001) perp.crossVectors(dir, new THREE.Vector3(1, 0, 0));
                perp.normalize();
                const perp2 = new THREE.Vector3().crossVectors(dir, perp).normalize();
                const angles = [0, 2 * Math.PI / 3, 4 * Math.PI / 3];
                for (const angle of angles) {
                    const offX = Math.cos(angle) * 0.2, offY = Math.sin(angle) * 0.2;
                    const ox = ax + perp.x * offX + perp2.x * offY;
                    const oy = ay + perp.y * offX + perp2.y * offY;
                    const oz = az + perp.z * offX + perp2.z * offY;
                    mat4.compose(new THREE.Vector3(ox, oy, oz), quat, new THREE.Vector3(0.10, bondLen, 0.10));
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
        const maxShells = 200;
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

        for (let i = 0; i < atomData.count && instIdx < 200; i++) {
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
                if (instIdx >= 200) break;
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
        const maxAtoms = 200;
        const createArrowSet = (color) => {
            const vertices = new Float32Array(maxAtoms * 6); // 2 verts per atom
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
        this._aeForceNet = createArrowSet(0xffffff); // white for net
    }

    updateAEForces(positions, forceData, count) {
        if (!this._aeForceIonic) this._buildAEForceArrows();
        if (!forceData || count === 0) {
            [this._aeForceIonic, this._aeForceVdw, this._aeForceBond, this._aeForceNet].forEach(l => l.geometry.setDrawRange(0, 0));
            return;
        }

        const scale = 8.0; // visual scale factor for force arrows
        const n = Math.min(count, 200);

        const updateArrows = (lines, forceArr) => {
            const posAttr = lines.geometry.getAttribute('position');
            for (let i = 0; i < n; i++) {
                const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
                const fx = forceArr[i * 3], fy = forceArr[i * 3 + 1], fz = forceArr[i * 3 + 2];

                // Log-compress force magnitude for visibility
                const fmag = Math.sqrt(fx * fx + fy * fy + fz * fz);
                const logScale = fmag > 1e-10 ? scale * Math.log1p(fmag) / fmag : 0;

                posAttr.array[i * 6] = px;
                posAttr.array[i * 6 + 1] = py;
                posAttr.array[i * 6 + 2] = pz;
                posAttr.array[i * 6 + 3] = px + fx * logScale;
                posAttr.array[i * 6 + 4] = py + fy * logScale;
                posAttr.array[i * 6 + 5] = pz + fz * logScale;
            }
            posAttr.needsUpdate = true;
            lines.geometry.setDrawRange(0, n * 2);
        };

        updateArrows(this._aeForceIonic, forceData.ionic);
        updateArrows(this._aeForceVdw, forceData.vdw);
        updateArrows(this._aeForceBond, forceData.bond);
        updateArrows(this._aeForceNet, forceData.net);
    }

    toggleAEForceIonic(on) { if (!this._aeForceIonic) this._buildAEForceArrows(); this._aeForceIonic.visible = on; }
    toggleAEForceVdw(on)   { if (!this._aeForceVdw)   this._buildAEForceArrows(); this._aeForceVdw.visible = on; }
    toggleAEForceBond(on)  { if (!this._aeForceBond)  this._buildAEForceArrows(); this._aeForceBond.visible = on; }
    toggleAEForceNet(on)   { if (!this._aeForceNet)   this._buildAEForceArrows(); this._aeForceNet.visible = on; }

    // ── Per-Atom Dipole-Moment Arrows ─────────────────────────────────

    _buildAEDipoleArrows() {
        const maxAtoms = 200;
        const vertices = new Float32Array(maxAtoms * 6); // 2 verts per atom
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
        const n = Math.min(count, 200);
        const posAttr = this._aeDipoles.geometry.getAttribute('position');
        for (let i = 0; i < n; i++) {
            const px = positions[i * 3], py = positions[i * 3 + 1], pz = positions[i * 3 + 2];
            const mx = dipoles[i * 3], my = dipoles[i * 3 + 1], mz = dipoles[i * 3 + 2];
            const mag = Math.sqrt(mx * mx + my * my + mz * mz);
            // Log-compress like the force arrows so large dipoles stay on screen
            const k = mag > 1e-10 ? scale * Math.log1p(mag) / mag : 0;
            posAttr.array[i * 6]     = px;
            posAttr.array[i * 6 + 1] = py;
            posAttr.array[i * 6 + 2] = pz;
            posAttr.array[i * 6 + 3] = px + mx * k;
            posAttr.array[i * 6 + 4] = py + my * k;
            posAttr.array[i * 6 + 5] = pz + mz * k;
        }
        posAttr.needsUpdate = true;
        this._aeDipoles.geometry.setDrawRange(0, n * 2);
    }

    toggleAEDipoles(on) {
        if (!this._aeDipoles) this._buildAEDipoleArrows();
        this._aeDipoles.visible = on;
        if (!on) this._aeDipoles.geometry.setDrawRange(0, 0);
    }

    // ── Hydrogen-Bond Dashed Lines (donor-H···acceptor) ───────────────

    _buildHBondLines() {
        const MAX_PAIRS = 256;
        const vertices = new Float32Array(MAX_PAIRS * 6); // 2 verts per pair
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geo.setDrawRange(0, 0);
        const mat = new THREE.LineDashedMaterial({
            color: 0x7dd3fc, dashSize: 0.35, gapSize: 0.25,
            transparent: true, opacity: 0.85,
        });
        this._hbondLines = new THREE.LineSegments(geo, mat);
        this._hbondLines.frustumCulled = false;
        this._hbondLines.visible = false;
        this.scene.add(this._hbondLines);
    }

    updateHBondLines(segments, count) {
        if (!this._hbondLines) this._buildHBondLines();
        const n = Math.min(count || 0, 256);
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
        if (this._aeForceNet) this._aeForceNet.visible = on;
        if (this._aeDipoles) this._aeDipoles.visible = on;
        if (this._hbondLines) this._hbondLines.visible = on;
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
        const canvas = document.createElement('canvas');
        canvas.width = 128;
        canvas.height = 64;
        const ctx = canvas.getContext('2d');
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
        sprite.scale.set(4, 2, 1);
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
            sprite.position.set(lb.x, lb.y + 2.5, lb.z); // offset above atom center
            sprite.visible = true;
        }
    }

    toggleElementLabels(on) {
        if (this._elementLabels) this._elementLabels.visible = on;
    }

    clearElementLabels() {
        if (!this._elementLabels) return;
        for (const sprite of this._labelPool) {
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
        if (this._aeForceIonic) {
            [this._aeForceIonic, this._aeForceVdw, this._aeForceBond, this._aeForceNet]
                .forEach(l => l.geometry.setDrawRange(0, 0));
        }
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
        disposeMesh(this._aeForceNet);       this._aeForceNet = null;
        this.clearElementLabels();
    }
}
