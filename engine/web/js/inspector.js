/**
 * Inspector Panel — click-to-inspect particle properties.
 *
 * Supports three modes:
 * - Scale 0 (Lattice): Raycasts against lattice particles, queries WASM for voxel data
 * - Scale 1 (Particles): Raycasts against cloud points, maps back to PE particle via
 *   cloud-to-particle map, queries bridge for per-particle telemetry
 * - Scale 2 (Atoms): Raycasts against atom/orbital cloud points, maps back to atom via
 *   cloud-to-atom map, queries bridge for per-atom telemetry + molecule info
 */

import * as THREE from 'three';
import { getById, chargeLabel, formatMass } from './particle-catalog.js';
import { getElement, elementSymbol, cpkColor } from './elements.js';
import { getMolecule } from './molecules.js';

const SPIN_LABELS = { 0: '--', 1: '+1/2 (up)', '-1': '-1/2 (down)' };
const COLOR_LABELS = { 0: 'colorless', 1: 'red', 2: 'green', 3: 'blue' };
const COLOR_CSS = { 0: '#9ca3af', 1: '#ef5350', 2: '#4ade80', 3: '#60a5fa' };

export class Inspector {
    constructor(viewport, bridge) {
        this.viewport = viewport;
        this.bridge = bridge;
        this.raycaster = new THREE.Raycaster();
        this.raycaster.params.Points.threshold = 2.0;
        this.mouse = new THREE.Vector2();
        this.selectedIndex = -1;
        this._selectedPos = null; // {x, y, z} lattice coords
        this._engineMode = 'lattice';

        // PE mode state
        this._cloudParticleMap = null; // Int32Array mapping cloud index → PE particle ID
        this._cloudCount = 0;
        this._peTypeMap = null;
        this._selectedPEParticleId = -1;

        // AE mode state (Scale 2)
        this._selectedAEAtomId = -1;
        this._aeAtomIds = null;        // Int32Array from atomData.ids
        this._aeCloudAtomMap = null;   // Int32Array from orbital expansion
        this._aeCloudMode = false;     // Whether orbital clouds are on
        this._aeCloudCount = 0;
        this._aePointCount = 0;        // Non-cloud atom point count
        this._currentMolId = null;     // Current molecule ID for info card

        // Scale 0 DOM elements
        this.emptyEl = document.getElementById('inspector-empty');
        this.contentEl = document.getElementById('inspector-content');

        // Scale 1 PE DOM elements
        this.peEmptyEl = document.getElementById('pe-inspector-empty');
        this.peContentEl = document.getElementById('pe-inspector-content');

        // Scale 2 AE DOM elements
        this.aeEmptyEl = document.getElementById('ae-inspector-empty');
        this.aeContentEl = document.getElementById('ae-inspector-content');
        this.aeMolInfoEl = document.getElementById('ae-mol-info');

        this.fields = {
            id:       document.getElementById('insp-id'),
            state:    document.getElementById('insp-state'),
            pos:      document.getElementById('insp-pos'),
            spin:     document.getElementById('insp-spin'),
            color:    document.getElementById('insp-color'),
            pair:     document.getElementById('insp-pair'),
            locked:   document.getElementById('insp-locked'),
            flux:     document.getElementById('insp-flux'),
            density:  document.getElementById('insp-density'),
            divj:     document.getElementById('insp-divj'),
            curl:     document.getElementById('insp-curl'),
            vel:      document.getElementById('insp-vel'),
            speed:    document.getElementById('insp-speed'),
            accel:    document.getElementById('insp-accel'),
            fCoulomb: document.getElementById('insp-f-coulomb'),
            fGravity: document.getElementById('insp-f-gravity'),
            eMag:     document.getElementById('insp-e-mag'),
            bMag:     document.getElementById('insp-b-mag'),
            fMagnetic:document.getElementById('insp-f-magnetic'),
            fStrong:  document.getElementById('insp-f-strong'),
            fExchange:document.getElementById('insp-f-exchange'),
        };

        this.peFields = {
            dot:      document.getElementById('pe-insp-dot'),
            name:     document.getElementById('pe-insp-name'),
            symbol:   document.getElementById('pe-insp-symbol'),
            id:       document.getElementById('pe-insp-id'),
            catalog:  document.getElementById('pe-insp-catalog'),
            mass:     document.getElementById('pe-insp-mass'),
            charge:   document.getElementById('pe-insp-charge'),
            locked:   document.getElementById('pe-insp-locked'),
            pos:      document.getElementById('pe-insp-pos'),
            vel:      document.getElementById('pe-insp-vel'),
            speed:    document.getElementById('pe-insp-speed'),
            ke:       document.getElementById('pe-insp-ke'),
            orbital:  document.getElementById('pe-insp-orbital'),
            nearest:  document.getElementById('pe-insp-nearest'),
            dist:     document.getElementById('pe-insp-dist'),
            fc:       document.getElementById('pe-insp-fc'),
            fnet:     document.getElementById('pe-insp-fnet'),
        };

        this.aeFields = {
            dot:         document.getElementById('ae-insp-dot'),
            name:        document.getElementById('ae-insp-name'),
            symbol:      document.getElementById('ae-insp-symbol'),
            id:          document.getElementById('ae-insp-id'),
            z:           document.getElementById('ae-insp-z'),
            charge:      document.getElementById('ae-insp-charge'),
            mass:        document.getElementById('ae-insp-mass'),
            locked:      document.getElementById('ae-insp-locked'),
            n:           document.getElementById('ae-insp-n'),
            a:           document.getElementById('ae-insp-a'),
            maxBonds:    document.getElementById('ae-insp-maxbonds'),
            pos:         document.getElementById('ae-insp-pos'),
            vel:         document.getElementById('ae-insp-vel'),
            speed:       document.getElementById('ae-insp-speed'),
            ke:          document.getElementById('ae-insp-ke'),
            fnet:        document.getElementById('ae-insp-fnet'),
            bonds:       document.getElementById('ae-insp-bonds'),
            nearest:     document.getElementById('ae-insp-nearest'),
            nearestDist: document.getElementById('ae-insp-nearest-dist'),
            sigma:       document.getElementById('ae-insp-sigma'),
            epsilon:     document.getElementById('ae-insp-epsilon'),
        };

        this.aeMolFields = {
            title:       document.getElementById('ae-mol-title'),
            desc:        document.getElementById('ae-mol-desc'),
            formula:     document.getElementById('ae-mol-formula'),
            category:    document.getElementById('ae-mol-category'),
            atomCount:   document.getElementById('ae-mol-atom-count'),
            composition: document.getElementById('ae-mol-composition'),
            bondCount:   document.getElementById('ae-mol-bond-count'),
            mass:        document.getElementById('ae-mol-mass'),
        };

        // Scenario info card (non-molecule scenarios)
        this.aeScenarioInfoEl = document.getElementById('ae-scenario-info');
        this.aeScenarioTitle = document.getElementById('ae-scenario-title');
        this.aeScenarioDesc = document.getElementById('ae-scenario-desc');
        this.aeScenarioFields = document.getElementById('ae-scenario-fields');

        // Click handler on viewport canvas
        const canvas = viewport.renderer.domElement;
        canvas.addEventListener('click', (e) => this._onClick(e));
    }

    setBridge(bridge) {
        this.bridge = bridge;
    }

    setEngineMode(mode) {
        this._engineMode = mode;
        // Reset selection when switching modes
        this.selectedIndex = -1;
        this._selectedPos = null;
        this._selectedPEParticleId = -1;
        this._selectedAEAtomId = -1;
        this._hideLatticeInspector();
        this._hidePEInspector();
        this._hideAEInspector();
        // Hide molecule/scenario info when leaving atoms mode
        if (mode !== 'atoms') {
            if (this.aeMolInfoEl) this.aeMolInfoEl.style.display = 'none';
            if (this.aeScenarioInfoEl) this.aeScenarioInfoEl.style.display = 'none';
        }
    }

    setPEContext(cloudParticleMap, cloudCount, typeMap) {
        this._cloudParticleMap = cloudParticleMap;
        this._cloudCount = cloudCount;
        this._peTypeMap = typeMap;
    }

    setAEContext(atomData, cloudAtomMap, cloudMode) {
        this._aeAtomIds = atomData?.ids || null;
        this._aeCloudAtomMap = cloudAtomMap || null;
        this._aeCloudMode = cloudMode;
        this._aePointCount = atomData?.count || 0;
        if (cloudAtomMap) {
            // Cloud count is determined by the number of cloud points rendered
            // We use the viewport's particle count as the definitive cloud point count
            this._aeCloudCount = this.viewport.particles?.geometry?.getAttribute('position')?.count || 0;
        }
        // Update live bond count in molecule info card
        if (this.aeMolFields.bondCount && this._currentMolId && atomData) {
            this.aeMolFields.bondCount.textContent = Math.round((atomData.bondCount || 0) / 2);
        }
    }

    setCurrentMolecule(molId) {
        this._currentMolId = molId;
        this._updateMoleculeInfo(molId);
    }

    setScenarioInfo(info) {
        if (!this.aeScenarioInfoEl) return;
        if (!info) {
            this.aeScenarioInfoEl.style.display = 'none';
            return;
        }
        this.aeScenarioInfoEl.style.display = 'block';
        if (this.aeScenarioTitle) this.aeScenarioTitle.textContent = info.title || '--';
        if (this.aeScenarioDesc) this.aeScenarioDesc.textContent = info.desc || '';
        if (this.aeScenarioFields) {
            this.aeScenarioFields.innerHTML = '';
            for (const [label, value] of Object.entries(info.fields || {})) {
                const dt = document.createElement('dt');
                dt.textContent = label;
                const dd = document.createElement('dd');
                dd.textContent = value;
                this.aeScenarioFields.appendChild(dt);
                this.aeScenarioFields.appendChild(dd);
            }
        }
    }

    _onClick(e) {
        const canvas = this.viewport.renderer.domElement;
        const rect = canvas.getBoundingClientRect();
        this.mouse.x = ((e.clientX - rect.left) / rect.width) * 2 - 1;
        this.mouse.y = -((e.clientY - rect.top) / rect.height) * 2 + 1;

        this.raycaster.setFromCamera(this.mouse, this.viewport.camera);
        const intersects = this.raycaster.intersectObject(this.viewport.particles);

        if (this._engineMode === 'atoms') {
            this._onClickAE(intersects);
        } else if (this._engineMode === 'particles') {
            this._onClickPE(intersects);
        } else {
            this._onClickLattice(intersects);
        }
    }

    // ── Scale 0 (Lattice) ─────────────────────────────────────────────

    _onClickLattice(intersects) {
        if (intersects.length > 0) {
            this.selectedIndex = intersects[0].index;
            const posArr = this.viewport.particles.geometry.getAttribute('position').array;
            this._selectedPos = {
                x: Math.round(posArr[this.selectedIndex * 3]),
                y: Math.round(posArr[this.selectedIndex * 3 + 1]),
                z: Math.round(posArr[this.selectedIndex * 3 + 2]),
            };
            this._showLatticeInspector();
        } else {
            this.selectedIndex = -1;
            this._selectedPos = null;
            this._hideLatticeInspector();
        }
    }

    _showLatticeInspector() {
        if (this.emptyEl) this.emptyEl.style.display = 'none';
        if (this.contentEl) this.contentEl.style.display = 'block';
        this._updateLatticeFields();
    }

    _hideLatticeInspector() {
        if (this.emptyEl) this.emptyEl.style.display = 'block';
        if (this.contentEl) this.contentEl.style.display = 'none';
    }

    _updateLatticeFields() {
        if (!this._selectedPos) return;
        const { x, y, z } = this._selectedPos;

        const v = this.bridge.inspectVoxel(x, y, z);
        const f = this.bridge.getForceAt(x, y, z);

        if (v) {
            const stateLabel = v.state === 1 ? '+1 (positive)' : v.state === -1 ? '-1 (negative)' : '0 (void)';
            const stateColor = v.state === 1 ? '#4ade80' : v.state === -1 ? '#f87171' : '#9ca3af';
            this.fields.id.textContent = v.particleId >= 0 ? v.particleId : '--';
            this.fields.state.innerHTML = `<span style="color:${stateColor}">${stateLabel}</span>`;
            this.fields.pos.textContent = `(${x}, ${y}, ${z})`;
            this.fields.spin.textContent = v.spin === 1 ? '+1/2 (up)' : v.spin === -1 ? '-1/2 (down)' : '--';
            const cLabel = COLOR_LABELS[v.color] || '--';
            const cColor = COLOR_CSS[v.color] || '#9ca3af';
            this.fields.color.innerHTML = `<span style="color:${cColor}">${cLabel}</span>`;
            this.fields.pair.textContent = v.pairId >= 0 ? v.pairId : '--';
            this.fields.locked.textContent = v.locked ? 'Yes' : 'No';

            this.fields.flux.textContent = vec3Str(v.fluxX, v.fluxY, v.fluxZ);
            this.fields.density.textContent = v.density.toFixed(6);
            this.fields.divj.textContent = v.divJ.toFixed(6);
            this.fields.curl.textContent = vec3Str(v.curlX, v.curlY, v.curlZ);
            this.fields.vel.textContent = vec3Str(v.velX, v.velY, v.velZ);
            this.fields.speed.textContent = v.speed.toFixed(6);
            this.fields.accel.textContent = v.accelMag.toFixed(6);
            this.fields.eMag.textContent = v.Emag !== undefined ? fmtForce(v.Emag) : '--';
            this.fields.bMag.textContent = v.Bmag !== undefined ? fmtForce(v.Bmag) : '--';
        } else {
            this.fields.id.textContent = '--';
            this.fields.state.textContent = '--';
            this.fields.pos.textContent = `(${x}, ${y}, ${z})`;
            this.fields.spin.textContent = '--';
            this.fields.color.textContent = '--';
            this.fields.pair.textContent = '--';
            this.fields.locked.textContent = '--';
            this.fields.flux.textContent = '--';
            this.fields.density.textContent = '--';
            this.fields.divj.textContent = '--';
            this.fields.curl.textContent = '--';
            this.fields.vel.textContent = '--';
            this.fields.speed.textContent = '--';
            this.fields.accel.textContent = '--';
            this.fields.eMag.textContent = '--';
            this.fields.bMag.textContent = '--';
        }

        if (f) {
            this.fields.fCoulomb.textContent = fmtForce(f.coulombMag);
            this.fields.fGravity.textContent = fmtForce(f.gravityMag);
            this.fields.fMagnetic.textContent = fmtForce(f.magneticMag);
            this.fields.fStrong.textContent = fmtForce(f.strongMag);
            this.fields.fExchange.textContent = fmtForce(f.exchangeMag);
        } else {
            for (const key of ['fCoulomb', 'fGravity', 'fMagnetic', 'fStrong', 'fExchange']) {
                this.fields[key].textContent = '--';
            }
        }
    }

    // ── Scale 1 (Particles / PE) ──────────────────────────────────────

    _onClickPE(intersects) {
        if (intersects.length > 0 && this._cloudParticleMap) {
            const cloudIdx = intersects[0].index;
            if (cloudIdx < this._cloudCount) {
                this._selectedPEParticleId = this._cloudParticleMap[cloudIdx];
                this._showPEInspector();
                return;
            }
        }
        this._selectedPEParticleId = -1;
        this._hidePEInspector();
    }

    _showPEInspector() {
        if (this.peEmptyEl) this.peEmptyEl.style.display = 'none';
        if (this.peContentEl) this.peContentEl.style.display = 'block';
        this._updatePEFields();
    }

    _hidePEInspector() {
        if (this.peEmptyEl) this.peEmptyEl.style.display = 'block';
        if (this.peContentEl) this.peContentEl.style.display = 'none';
    }

    _updatePEFields() {
        if (this._selectedPEParticleId < 0) return;

        const data = this.bridge.peInspectParticle(this._selectedPEParticleId);
        if (!data) {
            this._selectedPEParticleId = -1;
            this._hidePEInspector();
            return;
        }

        // Look up catalog entry
        const catId = this._peTypeMap ? this._peTypeMap.get(data.id) : null;
        const cat = catId ? getById(catId) : null;

        // Identity
        if (cat) {
            const [r, g, b] = cat.display_color;
            this.peFields.dot.style.background =
                `rgb(${Math.round(r * 255)},${Math.round(g * 255)},${Math.round(b * 255)})`;
            this.peFields.name.textContent = cat.name;
            this.peFields.symbol.textContent = cat.symbol;
            this.peFields.catalog.textContent = catId;
            this.peFields.mass.textContent = formatMass(cat.mass_mev);
            this.peFields.charge.textContent = chargeLabel(cat.charge);
        } else {
            this.peFields.dot.style.background = '#9ca3af';
            this.peFields.name.textContent = 'Unknown';
            this.peFields.symbol.textContent = '?';
            this.peFields.catalog.textContent = catId || '--';
            this.peFields.mass.textContent = data.mass.toFixed(3) + ' MeV';
            this.peFields.charge.textContent = data.charge > 0 ? '+' + data.charge : data.charge.toString();
        }
        this.peFields.id.textContent = data.id;
        this.peFields.locked.textContent = data.locked ? 'Yes (fixed)' : 'No';

        // Dynamics
        this.peFields.pos.textContent = `(${data.x.toFixed(2)}, ${data.y.toFixed(2)}, ${data.z.toFixed(2)})`;
        this.peFields.vel.textContent = `(${data.vx.toFixed(4)}, ${data.vy.toFixed(4)}, ${data.vz.toFixed(4)})`;
        this.peFields.speed.textContent = data.speed.toFixed(6);
        this.peFields.ke.textContent = data.ke.toFixed(6);
        this.peFields.orbital.textContent = data.orbitalR >= 0 ? data.orbitalR.toFixed(3) : '--';

        // Interactions
        if (data.nearestId >= 0) {
            const nearCatId = this._peTypeMap ? this._peTypeMap.get(data.nearestId) : null;
            const nearCat = nearCatId ? getById(nearCatId) : null;
            this.peFields.nearest.textContent = nearCat ? nearCat.name : `#${data.nearestId}`;
            this.peFields.dist.textContent = data.nearestDist.toFixed(3);
            this.peFields.fc.textContent = fmtForce(data.fCoulombNearest);
        } else {
            this.peFields.nearest.textContent = '--';
            this.peFields.dist.textContent = '--';
            this.peFields.fc.textContent = '--';
        }
        this.peFields.fnet.textContent = fmtForce(data.fNetMag);
    }

    // ── Scale 2 (Atoms / AE) ─────────────────────────────────────────

    _onClickAE(intersects) {
        if (intersects.length > 0) {
            const hitIdx = intersects[0].index;
            let atomArrayIdx = -1;

            if (this._aeCloudMode && this._aeCloudAtomMap) {
                // Orbital cloud mode: map cloud point → atom array index
                atomArrayIdx = this._aeCloudAtomMap[hitIdx];
            } else {
                // Direct atom point mode: hitIdx IS the atom array index
                atomArrayIdx = hitIdx;
            }

            // Map atom array index → atom ID via the ids array
            if (atomArrayIdx >= 0 && this._aeAtomIds && atomArrayIdx < this._aePointCount) {
                this._selectedAEAtomId = this._aeAtomIds[atomArrayIdx];
                this._showAEInspector();
                return;
            }
        }
        this._selectedAEAtomId = -1;
        this._hideAEInspector();
    }

    _showAEInspector() {
        if (this.aeEmptyEl) this.aeEmptyEl.style.display = 'none';
        if (this.aeContentEl) this.aeContentEl.style.display = 'block';
        this._updateAEFields();
    }

    _hideAEInspector() {
        if (this.aeEmptyEl) this.aeEmptyEl.style.display = 'block';
        if (this.aeContentEl) this.aeContentEl.style.display = 'none';
    }

    _updateAEFields() {
        if (this._selectedAEAtomId < 0) return;

        const data = this.bridge.aeInspectAtom(this._selectedAEAtomId);
        if (!data) {
            this._selectedAEAtomId = -1;
            this._hideAEInspector();
            return;
        }

        // Element info from elements.js
        const el = getElement(data.Z);
        const sym = elementSymbol(data.Z);
        const color = cpkColor(data.Z);

        // Element card
        if (this.aeFields.dot) {
            this.aeFields.dot.style.background =
                `rgb(${Math.round(color[0] * 255)},${Math.round(color[1] * 255)},${Math.round(color[2] * 255)})`;
        }
        if (this.aeFields.name) this.aeFields.name.textContent = el ? el.name : `Z=${data.Z}`;
        if (this.aeFields.symbol) this.aeFields.symbol.textContent = sym;
        if (this.aeFields.id) this.aeFields.id.textContent = data.id;
        if (this.aeFields.z) this.aeFields.z.textContent = data.Z;
        if (this.aeFields.charge) {
            this.aeFields.charge.textContent = data.charge === 0 ? '0' :
                (data.charge > 0 ? '+' + data.charge : data.charge.toString());
        }
        if (this.aeFields.mass) this.aeFields.mass.textContent = data.mass.toFixed(3) + ' AMU';
        if (this.aeFields.locked) this.aeFields.locked.textContent = data.locked ? 'Yes' : 'No';
        if (this.aeFields.n) this.aeFields.n.textContent = data.N;
        if (this.aeFields.a) this.aeFields.a.textContent = data.Z + data.N;
        if (this.aeFields.maxBonds) this.aeFields.maxBonds.textContent = data.maxBonds;

        // Dynamics card
        if (this.aeFields.pos) {
            this.aeFields.pos.textContent = `(${data.x.toFixed(3)}, ${data.y.toFixed(3)}, ${data.z.toFixed(3)})`;
        }
        if (this.aeFields.vel) {
            this.aeFields.vel.textContent = `(${data.vx.toFixed(4)}, ${data.vy.toFixed(4)}, ${data.vz.toFixed(4)})`;
        }
        if (this.aeFields.speed) this.aeFields.speed.textContent = data.speed.toFixed(6);
        if (this.aeFields.ke) this.aeFields.ke.textContent = data.ke.toFixed(6);
        if (this.aeFields.fnet) this.aeFields.fnet.textContent = fmtForce(data.fNetMag);

        // Bonds list (dynamic)
        if (this.aeFields.bonds) {
            this._buildBondsList(data.bonds);
        }

        // Nearest non-bonded neighbor
        if (this.aeFields.nearest) {
            if (data.nearestId >= 0) {
                const nearEl = getElement(data.nearestZ);
                const nearSym = elementSymbol(data.nearestZ);
                this.aeFields.nearest.textContent = nearEl ? `${nearSym} (#${data.nearestId})` : `#${data.nearestId}`;
            } else {
                this.aeFields.nearest.textContent = '--';
            }
        }
        if (this.aeFields.nearestDist) {
            this.aeFields.nearestDist.textContent =
                data.nearestId >= 0 ? data.nearestDist.toFixed(3) : '--';
        }

        // vdW parameters
        if (this.aeFields.sigma) this.aeFields.sigma.textContent = data.sigma.toFixed(3);
        if (this.aeFields.epsilon) this.aeFields.epsilon.textContent = data.epsilon.toFixed(6);
    }

    _buildBondsList(bonds) {
        const container = this.aeFields.bonds;
        if (!container) return;
        container.innerHTML = '';

        if (!bonds || bonds.length === 0) {
            const dt = document.createElement('dt');
            dt.textContent = 'Bonds';
            const dd = document.createElement('dd');
            dd.textContent = 'None';
            container.appendChild(dt);
            container.appendChild(dd);
            return;
        }

        for (let i = 0; i < bonds.length; i++) {
            const b = bonds[i];
            const partnerSym = elementSymbol(b.partnerZ);
            const orderSym = b.order === 1 ? '\u2014' : b.order === 2 ? '\u2550' : b.order === 3 ? '\u2261' : `\u00d7${b.order}`;

            const dt = document.createElement('dt');
            dt.textContent = `Bond ${i + 1}`;
            const dd = document.createElement('dd');
            dd.textContent = `${partnerSym} #${b.partnerId} ${orderSym}`;
            container.appendChild(dt);
            container.appendChild(dd);

            const dtDist = document.createElement('dt');
            dtDist.textContent = '';  // sub-entry
            const ddDist = document.createElement('dd');
            ddDist.textContent = `d=${b.dist.toFixed(3)}, r\u2080=${b.r_eq.toFixed(3)}`;
            container.appendChild(dtDist);
            container.appendChild(ddDist);
        }
    }

    _updateMoleculeInfo(molId) {
        if (!this.aeMolInfoEl) return;

        if (!molId) {
            this.aeMolInfoEl.style.display = 'none';
            return;
        }

        const mol = getMolecule(molId);
        if (!mol) {
            this.aeMolInfoEl.style.display = 'none';
            return;
        }

        this.aeMolInfoEl.style.display = 'block';

        if (this.aeMolFields.title) this.aeMolFields.title.textContent = mol.name;
        if (this.aeMolFields.desc) this.aeMolFields.desc.textContent = mol.description || '--';
        if (this.aeMolFields.formula) this.aeMolFields.formula.innerHTML = mol.formula || '--';
        if (this.aeMolFields.category) this.aeMolFields.category.textContent = mol.category || '--';

        const atoms = mol.atoms || [];
        if (this.aeMolFields.atomCount) this.aeMolFields.atomCount.textContent = atoms.length;

        // Build composition string e.g. "6C + 6H"
        if (this.aeMolFields.composition) {
            const counts = {};
            for (const a of atoms) {
                const sym = elementSymbol(a.Z);
                counts[sym] = (counts[sym] || 0) + 1;
            }
            // Hill system: C first, H second, rest alphabetical
            const comp = Object.entries(counts)
                .sort((a, b) => {
                    if (a[0] === 'C') return -1;
                    if (b[0] === 'C') return 1;
                    if (a[0] === 'H') return -1;
                    if (b[0] === 'H') return 1;
                    return a[0].localeCompare(b[0]);
                })
                .map(([sym, n]) => `${n}${sym}`)
                .join(' + ');
            this.aeMolFields.composition.textContent = comp || '--';
        }

        // Bond count: set to '--' initially; live count updated by setAEContext()
        if (this.aeMolFields.bondCount) {
            this.aeMolFields.bondCount.textContent = '--';
        }

        // Total mass
        if (this.aeMolFields.mass) {
            let totalMass = 0;
            for (const a of atoms) {
                const el = getElement(a.Z);
                const N = el ? el.neutrons : 0;
                totalMass += a.Z + N * 1.001;
            }
            this.aeMolFields.mass.textContent = totalMass.toFixed(2) + ' AMU';
        }
    }

    // ── Per-frame update ──────────────────────────────────────────────

    /** Call each frame to keep display updated if particle moved */
    update() {
        if (this._engineMode === 'atoms') {
            if (this._selectedAEAtomId >= 0 && this.aeContentEl &&
                this.aeContentEl.style.display === 'block') {
                this._updateAEFields();
            }
        } else if (this._engineMode === 'particles') {
            if (this._selectedPEParticleId >= 0 && this.peContentEl &&
                this.peContentEl.style.display === 'block') {
                this._updatePEFields();
            }
        } else {
            if (this.selectedIndex >= 0 && this.contentEl &&
                this.contentEl.style.display === 'block') {
                this._updateLatticeFields();
            }
        }
    }
}

function vec3Str(x, y, z) {
    return `(${x.toFixed(4)}, ${y.toFixed(4)}, ${z.toFixed(4)})`;
}

function fmtForce(mag) {
    if (mag === 0) return '0';
    if (mag < 0.0001) return mag.toExponential(2);
    return mag.toFixed(6);
}
