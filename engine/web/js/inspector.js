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
import {
    formatPosition, formatVec3, formatVelocity, formatForce,
    formatEnergy, formatLength, formatDensity, formatDivergence,
    formatField_E, formatField_B, formatFlux,
    BOHR_RADIUS_ANGSTROM,
} from './units.js';

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

        // Scale 4 Planetary DOM elements
        this.planetaryEmptyEl = document.getElementById('planetary-inspector-empty');
        this.planetaryContentEl = document.getElementById('planetary-inspector-content');
        this._selectedPlanetaryId = -1;
        this._planetaryRenderer = null;

        // Scale 5 Cosmic DOM elements
        this.cosmicEmptyEl = document.getElementById('cosmic-inspector-empty');
        this.cosmicContentEl = document.getElementById('cosmic-inspector-content');
        this._selectedCosmicId = -1;
        this._cosmicRenderer = null;

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

        this.cosmicFields = {
            dot:        document.getElementById('cosmic-insp-dot'),
            type:       document.getElementById('cosmic-insp-type'),
            id:         document.getElementById('cosmic-insp-id'),
            mass:       document.getElementById('cosmic-insp-mass'),
            radius:     document.getElementById('cosmic-insp-radius'),
            age:        document.getElementById('cosmic-insp-age'),
            temp:       document.getElementById('cosmic-insp-temp'),
            lum:        document.getElementById('cosmic-insp-lum'),
            pos:        document.getElementById('cosmic-insp-pos'),
            vel:        document.getElementById('cosmic-insp-vel'),
            speed:      document.getElementById('cosmic-insp-speed'),
            fuelFrac:   document.getElementById('cosmic-insp-fuel-frac'),
            fuelStage:  document.getElementById('cosmic-insp-fuel-stage')
        };

        this.planetaryFields = {
            dot:    document.getElementById('planetary-insp-dot'),
            type:   document.getElementById('planetary-insp-type'),
            id:     document.getElementById('planetary-insp-id'),
            mass:   document.getElementById('planetary-insp-mass'),
            temp:   document.getElementById('planetary-insp-temp'),
            biome:  document.getElementById('planetary-insp-biome'),
            pos:    document.getElementById('planetary-insp-pos'),
            vel:    document.getElementById('planetary-insp-vel'),
            speed:  document.getElementById('planetary-insp-speed')
        };

        // Focus Voxel button
        const btnFocus = document.getElementById('btn-focus-voxel');
        if (btnFocus) {
            btnFocus.addEventListener('click', () => {
                if (this._selectedPos && this.viewport && this.viewport.controls) {
                    const {x, y, z} = this._selectedPos;
                    this.viewport.controls.target.set(x, y, z);
                    const dist = 15;
                    const currPos = this.viewport.camera.position.clone();
                    const targetPos = new THREE.Vector3(x, y, z);
                    const dir = currPos.sub(targetPos).normalize().multiplyScalar(dist);
                    this.viewport.camera.position.copy(targetPos.clone().add(dir));
                }
            });
        }

        // Scenario info card (non-molecule scenarios)
        this.aeScenarioInfoEl = document.getElementById('ae-scenario-info');
        
        // Setup Symmetry Panel toggle listeners
        const symIds = ['sym-u1', 'sym-su2', 'sym-su3'];
        symIds.forEach(id => {
            const el = document.getElementById(id);
            if (el) el.addEventListener('change', () => {
                if (this._selectedPos && this.viewport) {
                    const u1 = document.getElementById('sym-u1').checked;
                    const su2 = document.getElementById('sym-su2').checked;
                    const su3 = document.getElementById('sym-su3').checked;
                    this.viewport.setSymmetryHighlights(this._selectedPos.x, this._selectedPos.y, this._selectedPos.z, u1, su2, su3);
                }
            });
        });
        this.aeScenarioTitle = document.getElementById('ae-scenario-title');
        this.aeScenarioDesc = document.getElementById('ae-scenario-desc');
        this.aeScenarioFields = document.getElementById('ae-scenario-fields');

        // Coordinate manual adjustments
        ['x', 'y', 'z'].forEach(axis => {
            const el = document.getElementById(`insp-pos-${axis}`);
            if (el) {
                el.addEventListener('change', (e) => {
                    if (!this._selectedPos) this._selectedPos = { x: 0, y: 0, z: 0 };
                    this._selectedPos[axis] = parseInt(e.target.value) || 0;
                    this._showLatticeInspector(); // Re-trigger inspector and highlight graphics
                });
            }
        });

        // Click handler on viewport canvas (double click to avoid dragging deselects)
        const canvas = viewport.renderer.domElement;
        canvas.addEventListener('dblclick', (e) => this._onClick(e));
        
        const threshInput = document.getElementById('raycast-threshold');
        if (threshInput) {
            threshInput.addEventListener('input', (e) => {
                this.raycaster.params.Points.threshold = parseFloat(e.target.value);
                const valDisplay = document.getElementById('raycast-threshold-val');
                if (valDisplay) valDisplay.textContent = e.target.value;
            });
        }
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
        this._selectedPlanetaryId = -1;
        this._selectedCosmicId = -1;
        this._hideLatticeInspector();
        this._hidePEInspector();
        this._hideAEInspector();
        this._hidePlanetaryInspector();
        this._hideCosmicInspector();
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

    setPlanetaryContext(bridge, renderer) {
        this.bridge = bridge;
        this._planetaryRenderer = renderer;
    }

    setCosmicContext(bridge, renderer) {
        this.bridge = bridge;
        this._cosmicRenderer = renderer;
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
        // Convert screen coords to NDC (-1..1) for Three.js raycaster
        this.mouse.x = ((e.clientX - rect.left) / rect.width) * 2 - 1;
        this.mouse.y = -((e.clientY - rect.top) / rect.height) * 2 + 1;

        this.raycaster.setFromCamera(this.mouse, this.viewport.camera);

        // Raycasting strategy varies by scale:
        // - Lattice (Scale 0): test particles, flux volume, and invisible
        //   voidBox (catches clicks on empty lattice space).
        // - PE/AE (Scale 1/2): test only the shared particle point cloud.
        // - Cosmic/Planetary (Scale 4/5): test dedicated mesh interactables.
        let intersects = [];
        if (this._engineMode === 'lattice') {
            const targets = [];
            if (this.viewport.particles) targets.push(this.viewport.particles);
            if (this.viewport._fluxVolume) targets.push(this.viewport._fluxVolume);
            if (this.viewport._voidBox) targets.push(this.viewport._voidBox);
            intersects = this.raycaster.intersectObjects(targets, false);
        } else if (this.viewport.particles) {
            intersects = [this.raycaster.intersectObject(this.viewport.particles)].flat();
        }

        if (this._engineMode === 'cosmic') {
            if (this._cosmicRenderer) {
                const interactables = this._cosmicRenderer.getInteractables();
                if (interactables.length > 0) {
                    const cosmicIntersects = this.raycaster.intersectObjects(interactables, false);
                    this._onClickCosmic(cosmicIntersects);
                } else {
                    this._onClickCosmic([]);
                }
            }
        } else if (this._engineMode === 'planetary') {
            if (this._planetaryRenderer) {
                const interactables = this._planetaryRenderer.getInteractables();
                if (interactables.length > 0) {
                    const planIntersects = this.raycaster.intersectObjects(interactables, false);
                    this._onClickPlanetary(planIntersects);
                } else {
                    this._onClickPlanetary([]);
                }
            }
        } else if (this._engineMode === 'atoms') {
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
            // Prioritize threshold-visible voxels over the empty boundary box
            let hit = intersects.find(h => h.object !== this.viewport._voidBox);
            
            // If the user deliberately requests only threshold-visible voxels, ignore the voidBox entirely
            if (!hit) {
                this.selectedIndex = -1;
                this._selectedPos = null;
                this._hideLatticeInspector();
                return;
            }
            if (hit.object === this.viewport._voidBox) {
                this.selectedIndex = -1;
                this._selectedPos = {
                    x: Math.round(hit.point.x),
                    y: Math.round(hit.point.y),
                    z: Math.round(hit.point.z)
                };
            } else {
                this.selectedIndex = hit.index;
                const posArr = hit.object.geometry.getAttribute('position').array;
                this._selectedPos = {
                    x: Math.round(posArr[this.selectedIndex * 3]),
                    y: Math.round(posArr[this.selectedIndex * 3 + 1]),
                    z: Math.round(posArr[this.selectedIndex * 3 + 2]),
                };
            }
            const L = this.bridge.latticeSize || 32;
            this._selectedPos.x = Math.max(0, Math.min(L-1, this._selectedPos.x));
            this._selectedPos.y = Math.max(0, Math.min(L-1, this._selectedPos.y));
            this._selectedPos.z = Math.max(0, Math.min(L-1, this._selectedPos.z));
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
        
        if (this.viewport && this.viewport.setVoxelHighlight && this._selectedPos) {
            this.viewport.setVoxelHighlight(this._selectedPos.x, this._selectedPos.y, this._selectedPos.z, true);
            // Re-trigger symmetry highlight rendering using the existing toggle states
            const u1 = document.getElementById('sym-u1')?.checked || false;
            const su2 = document.getElementById('sym-su2')?.checked || false;
            const su3 = document.getElementById('sym-su3')?.checked || false;
            this.viewport.setSymmetryHighlights(this._selectedPos.x, this._selectedPos.y, this._selectedPos.z, u1, su2, su3);
        }
        
        const symPanel = document.getElementById('floating-symmetry-panel');
        if (symPanel) symPanel.style.display = 'block';
        this._updateLatticeFields();
    }

    _hideLatticeInspector() {
        if (this.emptyEl) this.emptyEl.style.display = 'block';
        if (this.contentEl) this.contentEl.style.display = 'none';
        if (this.viewport && this.viewport.setVoxelHighlight) {
            this.viewport.setVoxelHighlight(0, 0, 0, false);
            this.viewport.setSymmetryHighlights(0, 0, 0, false, false, false);
        }
        const symPanel = document.getElementById('floating-symmetry-panel');
        if (symPanel) symPanel.style.display = 'none';
    }

    _updateLatticeFields() {
        if (!this._selectedPos) return;
        const { x, y, z } = this._selectedPos;

        let v = null;
        let f = null;
        if (typeof this.bridge.inspectVoxel === 'function') v = this.bridge.inspectVoxel(x, y, z);
        if (typeof this.bridge.getForceAt === 'function') f = this.bridge.getForceAt(x, y, z);
        
        // If engine doesn't support inspection (e.g. MockBridge), generate fallback null states
        if (!v && typeof this.bridge.inspectVoxel !== 'function') {
            v = { state: 0, particleId: -1, spin: 0, color: 0, pairId: -1, locked: false, fluxX:0, fluxY:0, fluxZ:0, density:0, divJ:0, curlX:0, curlY:0, curlZ:0, velX:0, velY:0, velZ:0, speed:0, accelMag:0 };
        }

        if (v) {
            const stateLabel = v.state === 1 ? '+1 (positive)' : v.state === -1 ? '-1 (negative)' : '0 (void)';
            const stateColor = v.state === 1 ? '#4ade80' : v.state === -1 ? '#f87171' : '#9ca3af';
            this.fields.id.textContent = v.particleId >= 0 ? v.particleId : '--';
            this.fields.state.innerHTML = `<span style="color:${stateColor}">${stateLabel}</span>`;
            // Populate position input fields (don't overwrite textContent which destroys <input> children)
            const posXEl = document.getElementById('insp-pos-x');
            const posYEl = document.getElementById('insp-pos-y');
            const posZEl = document.getElementById('insp-pos-z');
            if (posXEl) posXEl.value = x;
            if (posYEl) posYEl.value = y;
            if (posZEl) posZEl.value = z;
            this.fields.spin.textContent = v.spin === 1 ? '+1/2 (up)' : v.spin === -1 ? '-1/2 (down)' : '--';
            const cLabel = COLOR_LABELS[v.color] || '--';
            const cColor = COLOR_CSS[v.color] || '#9ca3af';
            this.fields.color.innerHTML = `<span style="color:${cColor}">${cLabel}</span>`;
            this.fields.pair.textContent = v.pairId >= 0 ? v.pairId : '--';
            this.fields.locked.textContent = v.locked ? 'Yes' : 'No';

            this.fields.flux.textContent = formatVec3(v.fluxX, v.fluxY, v.fluxZ, 'flux', 0);
            this.fields.density.textContent = formatDensity(v.density, 0).text;
            this.fields.divj.textContent = formatDivergence(v.divJ, 0).text;
            this.fields.curl.textContent = formatVec3(v.curlX, v.curlY, v.curlZ, 'curl', 0);
            this.fields.vel.textContent = formatVec3(v.velX, v.velY, v.velZ, 'velocity', 0);
            this.fields.speed.textContent = formatVelocity(v.speed, 0).text;
            this.fields.accel.textContent = formatForce(v.accelMag, 0).text;
            this.fields.eMag.textContent = v.Emag !== undefined ? formatField_E(v.Emag, 0).text : '--';
            this.fields.bMag.textContent = v.Bmag !== undefined ? formatField_B(v.Bmag, 0).text : '--';
        } else {
            this.fields.id.textContent = '--';
            this.fields.state.textContent = '--';
            const posXEl2 = document.getElementById('insp-pos-x');
            const posYEl2 = document.getElementById('insp-pos-y');
            const posZEl2 = document.getElementById('insp-pos-z');
            if (posXEl2) posXEl2.value = x;
            if (posYEl2) posYEl2.value = y;
            if (posZEl2) posZEl2.value = z;
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
            this.fields.fCoulomb.textContent = formatForce(f.coulombMag, 0).text;
            this.fields.fGravity.textContent = formatForce(f.gravityMag, 0).text;
            this.fields.fMagnetic.textContent = formatForce(f.magneticMag, 0).text;
            this.fields.fStrong.textContent = formatForce(f.strongMag, 0).text;
            this.fields.fExchange.textContent = formatForce(f.exchangeMag, 0).text;
        } else {
            for (const key of ['fCoulomb', 'fGravity', 'fMagnetic', 'fStrong', 'fExchange']) {
                this.fields[key].textContent = '--';
            }
        }
        
        // Update Moore Neighborhood
        const mooreGrid = document.getElementById('insp-moore-grid');
        if (mooreGrid && typeof this.bridge.inspectVoxel === 'function') {
            const L = this.bridge.latticeSize || 64;
            let html = '';
            for (let dz = -1; dz <= 1; dz++) {
                html += '<div style="display:inline-block; margin: 0 8px;">';
                html += `<div style="color:var(--text-muted);font-size:10px;margin-bottom:6px">Z${dz === 0 ? '' : (dz > 0 ? '+'+dz : dz)}</div>`;
                for (let dy = 1; dy >= -1; dy--) {
                    html += '<div style="display:flex;gap:4px;margin-bottom:4px">';
                    for (let dx = -1; dx <= 1; dx++) {
                        const nX = (x + dx + L) % L;
                        const nY = (y + dy + L) % L;
                        const nZ = (z + dz + L) % L;
                        let nV = this.bridge.inspectVoxel(nX, nY, nZ);
                        let symbol = '·';
                        let color = '#475569';
                        let bg = '#0f172a'; // darker background for empty
                        let borderStyle = 'border:1px solid #334155;';
                        
                        if (nV && nV.state === 1) { 
                            symbol = '+'; 
                            color = '#4ade80';
                            bg = 'rgba(74, 222, 128, 0.15)';
                        } else if (nV && nV.state === -1) { 
                            symbol = '-'; 
                            color = '#f87171';
                            bg = 'rgba(248, 113, 113, 0.15)';
                        } else if (nV && nV.state === 0) {
                            // Empty state dynamics (flux heatmap)
                            let fx = nV.fluxX || 0;
                            let fy = nV.fluxY || 0;
                            let fz = nV.fluxZ || 0;
                            let fluxMag = Math.sqrt(fx*fx + fy*fy + fz*fz);
                            if (fluxMag > 0.001) {
                                let intensity = Math.min(1.0, fluxMag * 2.0);
                                bg = `rgba(56, 189, 248, ${intensity * 0.4})`; // sky blue glow
                                color = `rgba(125, 211, 252, ${0.4 + intensity * 0.6})`;
                                if (fluxMag > 0.1) {
                                    symbol = '~'; // indicate strong rippling flux
                                }
                            }
                        }
                        
                        // Highlight center focus point
                        const isCenter = (dx === 0 && dy === 0 && dz === 0);
                        if (isCenter) {
                            borderStyle = 'border:1px solid #94a3b8;';
                            if (bg === '#0f172a') bg = '#1e293b'; // slightly lighter if entirely empty
                        }
                        
                        html += `<div style="width:18px;height:18px;line-height:16px;background:${bg};${borderStyle}border-radius:2px;color:${color};transition:background 0.1s;overflow:hidden">${symbol}</div>`;
                    }
                    html += '</div>';
                }
                html += '</div>';
            }
            mooreGrid.innerHTML = html;
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
        this.peFields.pos.textContent = formatPosition(data.x, data.y, data.z, 1);
        this.peFields.vel.textContent = formatVec3(data.vx, data.vy, data.vz, 'velocity', 1);
        this.peFields.speed.textContent = formatVelocity(data.speed, 1).text;
        this.peFields.ke.textContent = formatEnergy(data.ke, 1).text;
        this.peFields.orbital.textContent = data.orbitalR >= 0 ? formatLength(data.orbitalR, 1).text : '--';

        // Interactions
        if (data.nearestId >= 0) {
            const nearCatId = this._peTypeMap ? this._peTypeMap.get(data.nearestId) : null;
            const nearCat = nearCatId ? getById(nearCatId) : null;
            this.peFields.nearest.textContent = nearCat ? nearCat.name : `#${data.nearestId}`;
            this.peFields.dist.textContent = formatLength(data.nearestDist, 1).text;
            this.peFields.fc.textContent = formatForce(data.fCoulombNearest, 1).text;
        } else {
            this.peFields.nearest.textContent = '--';
            this.peFields.dist.textContent = '--';
            this.peFields.fc.textContent = '--';
        }
        this.peFields.fnet.textContent = formatForce(data.fNetMag, 1).text;
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
            this.aeFields.pos.textContent = formatPosition(data.x, data.y, data.z, 2);
        }
        if (this.aeFields.vel) {
            this.aeFields.vel.textContent = formatVec3(data.vx, data.vy, data.vz, 'velocity', 2);
        }
        if (this.aeFields.speed) this.aeFields.speed.textContent = formatVelocity(data.speed, 2).text;
        if (this.aeFields.ke) this.aeFields.ke.textContent = formatEnergy(data.ke, 2).text;
        if (this.aeFields.fnet) this.aeFields.fnet.textContent = formatForce(data.fNetMag, 2).text;

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
                data.nearestId >= 0 ? formatLength(data.nearestDist, 2).text : '--';
        }

        // vdW parameters
        if (this.aeFields.sigma) this.aeFields.sigma.textContent = formatLength(data.sigma, 2).text;
        if (this.aeFields.epsilon) this.aeFields.epsilon.textContent = formatEnergy(data.epsilon, 2).text;
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
            const dStr = formatLength(b.dist, 2).text;
            const rStr = formatLength(b.r_eq, 2).text;
            ddDist.textContent = `d=${dStr}, r\u2080=${rStr}`;
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

    // ── Scale 4 (Planetary) ──────────────────────────────────────────

    _onClickPlanetary(intersects) {
        if (intersects.length > 0) {
            const mesh = intersects[0].object;
            this._selectedPlanetaryId = mesh.userData.id;
            this._showPlanetaryInspector();
        } else {
            this._selectedPlanetaryId = -1;
            this._hidePlanetaryInspector();
        }
    }

    _showPlanetaryInspector() {
        if (this.planetaryEmptyEl) this.planetaryEmptyEl.style.display = 'none';
        if (this.planetaryContentEl) this.planetaryContentEl.style.display = 'block';
        this._updatePlanetaryFields();
    }

    _hidePlanetaryInspector() {
        if (this.planetaryEmptyEl) this.planetaryEmptyEl.style.display = 'block';
        if (this.planetaryContentEl) this.planetaryContentEl.style.display = 'none';
    }

    _updatePlanetaryFields() {
        if (this._selectedPlanetaryId === -1 || !this.bridge) return;

        const data = this.bridge.getPlanetaryData();
        if (!data || !data.buffer) return;

        let index = -1;
        for (let i = 0; i < data.count; i++) {
            if (data.buffer[i * 16 + 6] === this._selectedPlanetaryId) {
                index = i;
                break;
            }
        }
        if (index === -1) {
            this._hidePlanetaryInspector();
            return;
        }

        const off = index * 16;
        const x = data.buffer[off + 0];
        const y = data.buffer[off + 1];
        const z = data.buffer[off + 2];
        const type = data.buffer[off + 3];
        const mass = data.buffer[off + 4];
        
        const vx = data.buffer[off + 8];
        const vy = data.buffer[off + 9];
        const vz = data.buffer[off + 10];

        const speed = Math.sqrt(vx*vx + vy*vy + vz*vz);

        // Find star for temperature heuristics
        let starPos = {x:0, y:0, z:0};
        for (let i = 0; i < data.count; i++) {
            if (data.buffer[i * 16 + 3] === 0) {
                starPos = {x: data.buffer[i*16+0], y: data.buffer[i*16+1], z: data.buffer[i*16+2]};
                break;
            }
        }
        
        let d = Math.sqrt(Math.pow(x - starPos.x, 2) + Math.pow(y - starPos.y, 2) + Math.pow(z - starPos.z, 2));
        let uTemp = 0.0;
        let biome = "Deep Space";
        
        if (type === 0) {
            this.planetaryFields.type.textContent = "Host Star";
            this.planetaryFields.dot.style.background = '#facc15';
            biome = "Stellar Plasma";
        } else {
            this.planetaryFields.type.textContent = "Rocky Exoplanet";
            this.planetaryFields.dot.style.background = '#4ade80';
            
            if (d < 0.5) { uTemp = 1.0; biome = "Lava World"; }
            else if (d > 2.0) { uTemp = -1.0; biome = "Ice World"; }
            else { uTemp = (1.25 - d); biome = "Temperate Earthlike"; }
            
            if (type === 2) {
                this.planetaryFields.type.textContent = "Gas Giant";
                this.planetaryFields.dot.style.background = '#38bdf8';
                biome = "Gas/Fluid Envelope";
            }
        }

        this.planetaryFields.id.textContent = this._selectedPlanetaryId;
        this.planetaryFields.mass.textContent = typeof mass === 'number' ? mass.toFixed(4) + ' M☉' : mass;
        
        // Pseudo temp mapping
        let tK = 280 + (uTemp * 500); 
        this.planetaryFields.temp.textContent = Math.round(tK).toString();
        this.planetaryFields.biome.textContent = biome;

        this.planetaryFields.pos.textContent = `(${x.toFixed(4)}, ${y.toFixed(4)}, ${z.toFixed(4)})`;
        this.planetaryFields.vel.textContent = `(${vx.toFixed(4)}, ${vy.toFixed(4)}, ${vz.toFixed(4)})`;
        this.planetaryFields.speed.textContent = `${speed.toFixed(6)} AU/t`;
    }

    // ── Scale 5 (Cosmic) ─────────────────────────────────────────────
    
    _onClickCosmic(intersects) {
        if (intersects.length > 0) {
            const hit = intersects[0];
            let rawId = -1;
            
            // Extract the physics ID from the intersected hit
            if (hit.object.userData && hit.object.userData.ids) {
                // PointCloud geometry hit
                const geoIdx = hit.index;
                rawId = hit.object.userData.ids[geoIdx];
            } else if (hit.object.userData && hit.object.userData.id !== undefined) {
                // Solid Mesh geometry hit (like a Black Hole)
                rawId = hit.object.userData.id;
            }

            if (rawId >= 0) {
                this._selectedCosmicId = rawId;
                this._showCosmicInspector();
                return;
            }
        }
        this._selectedCosmicId = -1;
        this._hideCosmicInspector();
    }

    _showCosmicInspector() {
        if (this.cosmicEmptyEl) this.cosmicEmptyEl.style.display = 'none';
        if (this.cosmicContentEl) this.cosmicContentEl.style.display = 'block';
        this._updateCosmicFields();
    }

    _hideCosmicInspector() {
        if (this.cosmicEmptyEl) this.cosmicEmptyEl.style.display = 'block';
        if (this.cosmicContentEl) this.cosmicContentEl.style.display = 'none';
    }

    _updateCosmicFields() {
        if (this._selectedCosmicId < 0 || !this.bridge.cosmicInspectBody) return;
        const b = this.bridge.cosmicInspectBody(this._selectedCosmicId);
        if (!b) {
            this._hideCosmicInspector();
            return;
        }

        const typeNames = {
            '-3': 'Dark Energy', '-2': 'Quasar', '-1': 'Black Hole',
            '0': 'Dark Matter', '1': 'Gas Cloud', '2': 'Star',
            '3': 'Neutron Star', '4': 'Nebula', '5': 'White Dwarf'
        };
        const colors = {
            '-3': '#5b21b6', '-2': '#facc15', '-1': '#000000',
            '0': '#7c3aed', '1': '#38bdf8', '2': '#fbbf24',
            '3': '#94a3b8', '4': '#f472b6', '5': '#f8fafc'
        };

        if (this.cosmicFields.type) this.cosmicFields.type.textContent = typeNames[b.type] || 'Unknown';
        if (this.cosmicFields.dot) {
            this.cosmicFields.dot.style.background = colors[b.type] || '#ccc';
            if (b.type === -1) this.cosmicFields.dot.style.border = '1px solid #aaa'; // edge for BH
            else this.cosmicFields.dot.style.border = 'none';
        }

        if (this.cosmicFields.id) this.cosmicFields.id.textContent = b.id;
        
        // Mass string
        let massStr = '';
        if (b.mass >= 1e6) { massStr = (b.mass / 1e6).toFixed(2) + ' M\u2609'; }
        else if (b.mass < 1) { massStr = b.mass.toFixed(4) + ' M\u2609'; }
        else { massStr = b.mass.toFixed(1) + ' M\u2609'; }
        
        if (this.cosmicFields.mass) this.cosmicFields.mass.textContent = massStr;
        if (this.cosmicFields.radius) this.cosmicFields.radius.textContent = b.radius.toFixed(2) + ' R\u2609';
        if (this.cosmicFields.age) this.cosmicFields.age.textContent = b.age > 0 ? (b.age * 0.1).toFixed(1) + ' Myrs' : '--';
        if (this.cosmicFields.temp) this.cosmicFields.temp.textContent = b.temperature > 0 ? Math.round(b.temperature).toLocaleString() + ' K' : '--';
        if (this.cosmicFields.lum) this.cosmicFields.lum.textContent = b.luminosity > 0 ? b.luminosity.toExponential(2) + ' L\u2609' : '--';
        
        if (this.cosmicFields.pos) this.cosmicFields.pos.textContent = `(${b.x.toFixed(1)}, ${b.y.toFixed(1)}, ${b.z.toFixed(1)})`;
        if (this.cosmicFields.vel) this.cosmicFields.vel.textContent = `(${b.vx.toFixed(2)}, ${b.vy.toFixed(2)}, ${b.vz.toFixed(2)})`;
        if (this.cosmicFields.speed) this.cosmicFields.speed.textContent = b.speed.toFixed(2) + ' km/s';

        if (this.cosmicFields.fuelFrac) this.cosmicFields.fuelFrac.textContent = (b.fuel_fraction * 100).toFixed(1) + '%';
        
        const phaseNames = ['Protostar', 'Red Giant', 'Core He Burn', 'AGB', 'Pre-SN', 'Core Collapse'];
        if (this.cosmicFields.fuelStage) {
            if (b.type === 2) {
                this.cosmicFields.fuelStage.textContent = phaseNames[b.fuel_stage] || 'Main Sequence';
            } else {
                this.cosmicFields.fuelStage.textContent = '--';
            }
        }
    }

    // ── Per-frame update ──────────────────────────────────────────────

    /** Call each frame to keep display updated if particle moved */
    update() {
        if (this._engineMode === 'cosmic') {
            if (this._selectedCosmicId >= 0 && this.cosmicContentEl && this.cosmicContentEl.style.display === 'block') {
                this._updateCosmicFields();
            }
        } else if (this._engineMode === 'atoms') {
            if (this._selectedAEAtomId >= 0 && this.aeContentEl &&
                this.aeContentEl.style.display === 'block') {
                this._updateAEFields();
            }
        } else if (this._engineMode === 'particles') {
            if (this._selectedPEParticleId >= 0 && this.peContentEl &&
                this.peContentEl.style.display === 'block') {
                this._updatePEFields();
            }
        } else if (this._engineMode === 'planetary') {
            if (this._selectedPlanetaryId >= 0 && this.planetaryContentEl &&
                this.planetaryContentEl.style.display === 'block') {
                this._updatePlanetaryFields();
            }
        } else {
            if (this._selectedPos && this.contentEl &&
                this.contentEl.style.display === 'block') {
                this._updateLatticeFields();
            }
        }
    }
}

// Legacy formatting helpers -- superseded by units.js formatPosition/formatForce.
// Retained for backward compatibility with any external callers.
function vec3Str(x, y, z) {
    return `(${x.toFixed(4)}, ${y.toFixed(4)}, ${z.toFixed(4)})`;
}

function fmtForce(mag) {
    if (mag === 0) return '0';
    if (mag < 0.0001) return mag.toExponential(2);
    return mag.toFixed(6);
}
