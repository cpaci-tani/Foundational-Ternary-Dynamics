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
import { updateInspectorChrome, resetInspectorSelection } from './inspector/chrome.js';
import { collectInspectorDom } from './inspector/dom-bindings.js';
import { bindInspectorPointerControls } from './inspector/pointer-controller.js';
import {
    handleLatticeClick,
    showLatticeInspector,
    hideLatticeInspector,
    updateLatticeFields,
} from './inspector/scales/lattice.js';
import {
    handlePEClick,
    showPEInspector,
    hidePEInspector,
    updatePEFields,
} from './inspector/scales/particles.js';
import {
    handleAEClick,
    showAEInspector,
    hideAEInspector,
    updateAEFields,
    buildAEBondsList,
    updateAEMoleculeInfo,
    setAEScenarioInfo,
} from './inspector/scales/atoms.js';
import {
    handlePlanetaryClick,
    showPlanetaryInspector,
    hidePlanetaryInspector,
    updatePlanetaryFields,
} from './inspector/scales/planetary.js';
import {
    handleCosmicClick,
    showCosmicInspector,
    hideCosmicInspector,
    updateCosmicFields,
} from './inspector/scales/cosmic.js';

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

        // Scale 4 Planetary DOM elements
        this._selectedPlanetaryId = -1;
        this._planetaryRenderer = null;

        // Scale 5 Cosmic DOM elements
        this._selectedCosmicId = -1;
        this._cosmicRenderer = null;
        this._dragThresholdPx = 6;
        Object.assign(this, collectInspectorDom());

        // Focus Voxel button
        const btnFocus = this.focusSelectionBtn;
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
        if (this.clearSelectionBtn) {
            this.clearSelectionBtn.addEventListener('click', () => this.clearSelection());
        }

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

        this._releasePointerControls = bindInspectorPointerControls({
            viewport,
            dragThresholdPx: this._dragThresholdPx,
            onClick: (e) => this._onClick(e),
            onEscape: () => this.clearSelection(),
        });
        
        const threshInput = document.getElementById('raycast-threshold');
        if (threshInput) {
            threshInput.addEventListener('input', (e) => {
                this.raycaster.params.Points.threshold = parseFloat(e.target.value);
                const valDisplay = document.getElementById('raycast-threshold-val');
                if (valDisplay) valDisplay.textContent = e.target.value;
            });
        }
        this._updateInspectorChrome();
    }

    setBridge(bridge) {
        this.bridge = bridge;
    }

    getSelectedLatticePosition() {
        return this._selectedPos;
    }

    clearSelection() {
        resetInspectorSelection(this);
        this._hideLatticeInspector();
        this._hidePEInspector();
        this._hideAEInspector();
        this._hidePlanetaryInspector();
        this._hideCosmicInspector();
    }

    _updateInspectorChrome() {
        updateInspectorChrome(this);
    }

    setEngineMode(mode) {
        this._engineMode = mode;
        // Reset selection when switching modes
        resetInspectorSelection(this);
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
        this._updateInspectorChrome();
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
        updateAEMoleculeInfo(this, molId);
    }

    setScenarioInfo(info) {
        setAEScenarioInfo(this, info);
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
                    handleCosmicClick(this, cosmicIntersects);
                } else {
                    handleCosmicClick(this, []);
                }
            }
        } else if (this._engineMode === 'planetary') {
            if (this._planetaryRenderer) {
                const interactables = this._planetaryRenderer.getInteractables();
                if (interactables.length > 0) {
                    const planIntersects = this.raycaster.intersectObjects(interactables, false);
                    handlePlanetaryClick(this, planIntersects);
                } else {
                    handlePlanetaryClick(this, []);
                }
            }
        } else if (this._engineMode === 'atoms') {
            handleAEClick(this, intersects);
        } else if (this._engineMode === 'particles') {
            handlePEClick(this, intersects);
        } else {
            handleLatticeClick(this, intersects);
        }
    }

    // ── Scale 0 (Lattice) ─────────────────────────────────────────────

    _onClickLattice(intersects) {
        handleLatticeClick(this, intersects);
    }

    _showLatticeInspector() {
        showLatticeInspector(this);
    }

    _hideLatticeInspector() {
        hideLatticeInspector(this);
    }

    _updateLatticeFields() {
        updateLatticeFields(this);
    }

    // ── Scale 1 (Particles / PE) ──────────────────────────────────────

    _onClickPE(intersects) {
        handlePEClick(this, intersects);
    }

    _showPEInspector() {
        showPEInspector(this);
    }

    _hidePEInspector() {
        hidePEInspector(this);
    }

    _updatePEFields() {
        updatePEFields(this);
    }

    // ── Scale 2 (Atoms / AE) ─────────────────────────────────────────

    _onClickAE(intersects) {
        handleAEClick(this, intersects);
    }

    _showAEInspector() {
        showAEInspector(this);
    }

    _hideAEInspector() {
        hideAEInspector(this);
    }

    _updateAEFields() {
        updateAEFields(this);
    }

    _buildBondsList(bonds) {
        buildAEBondsList(this, bonds);
    }

    _updateMoleculeInfo(molId) {
        updateAEMoleculeInfo(this, molId);
    }

    // ── Scale 4 (Planetary) ──────────────────────────────────────────

    _onClickPlanetary(intersects) {
        handlePlanetaryClick(this, intersects);
    }

    _showPlanetaryInspector() {
        showPlanetaryInspector(this);
    }

    _hidePlanetaryInspector() {
        hidePlanetaryInspector(this);
    }

    _updatePlanetaryFields() {
        updatePlanetaryFields(this);
    }

    // ── Scale 5 (Cosmic) ─────────────────────────────────────────────
    
    _onClickCosmic(intersects) {
        handleCosmicClick(this, intersects);
    }

    _showCosmicInspector() {
        showCosmicInspector(this);
    }

    _hideCosmicInspector() {
        hideCosmicInspector(this);
    }

    _updateCosmicFields() {
        updateCosmicFields(this);
    }

    // ── Per-frame update ──────────────────────────────────────────────

    /** Call each frame to keep display updated if particle moved */
    update() {
        if (this._engineMode === 'cosmic') {
            if (this._selectedCosmicId >= 0 && this.cosmicContentEl && this.cosmicContentEl.style.display === 'block') {
                updateCosmicFields(this);
            }
        } else if (this._engineMode === 'atoms') {
            if (this._selectedAEAtomId >= 0 && this.aeContentEl &&
                this.aeContentEl.style.display === 'block') {
                updateAEFields(this);
            }
        } else if (this._engineMode === 'particles') {
            if (this._selectedPEParticleId >= 0 && this.peContentEl &&
                this.peContentEl.style.display === 'block') {
                updatePEFields(this);
            }
        } else if (this._engineMode === 'planetary') {
            if (this._selectedPlanetaryId >= 0 && this.planetaryContentEl &&
                this.planetaryContentEl.style.display === 'block') {
                updatePlanetaryFields(this);
            }
        } else {
            if (this._selectedPos && this.contentEl &&
                this.contentEl.style.display === 'block') {
                updateLatticeFields(this);
            }
        }
    }
}

