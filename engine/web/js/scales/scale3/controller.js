/**
 * Scale 3 (Molecules) Controller
 * ────────────────────────────────────────────────────────────────────
 *
 * Owns molecule scenario loading for Scale 3.  Scale 3 shares the same
 * AtomEngine (AE) as Scale 2 -- the only difference is that Scale 3
 * loads pre-built molecular structures (H2O, ethanol, caffeine, etc.)
 * rather than individual atoms.  The render loop is identical and reused
 * from the Scale 2 controller's animateAE().
 *
 * WHY THIS IS SEPARATE FROM SCALE 2:
 *   Scale 2 (Atoms) loads individual elements and atomic clusters.
 *   Scale 3 (Molecules) loads multi-atom molecules from a data-driven
 *   library (molecules.js), with pre-bonding, stability dry-runs, and
 *   molecule-specific camera distances.  Keeping them in separate files
 *   mirrors the UI's scale selector and lets each controller own its
 *   scenario logic independently.
 *
 * SHARED RESOURCES:
 *   - animateAE() from Scale 2 handles the render loop for both scales
 *   - The AE bridge methods (aeTick, aeGetAtomData, etc.) are identical
 *   - Visual toggle state lives in Scale 2's module and is shared
 *
 * CONTEXT OBJECT (ctx):
 *   Same shape as Scale 2 -- see scale2/controller.js header.
 *
 *   Additional ctx property used here:
 *     resetAllVisualState - function, master cross-scale visual reset
 *
 * EXPORTS:
 *   loadMoleculeScenario(ctx, name) - molecule scenario setup
 *   resetScale3(ctx)                - clear Scale 3-specific state
 *
 * ---------------------------------------------------------------
 * DELEGATION STUB: after wiring into app.js:
 *
 *   function loadMoleculeScenario(name) {
 *       return scale3.loadMoleculeScenario(ctx, name);
 *   }
 * ---------------------------------------------------------------
 */

import { getMolecule, loadMolecule } from '../../molecules.js';
import { SCALE2_TOGGLES } from '../../config/toggles.js';
import { animateAE, syncAEParams } from '../scale2/controller.js';
import { Scale3ControlsComponent } from './ui/controls/component.js';


// =====================================================================
import { syncAEParamsFromUI as _syncAEParamsFromUIInternal, resetAETogglesToDefaults as _resetAETogglesToDefaults } from '../scale-utils.js';



// =====================================================================
// Module State
// =====================================================================

// Scale 3 tracks initial energy for drift monitoring (same purpose as
// Scale 2's _aeInitialEnergy, but scoped to molecule scenarios).
let _aeInitialEnergy = null;


// =====================================================================
// Exported: resetScale3(ctx)
// =====================================================================
/**
 * Clear Scale 3-specific state for a clean mode switch.
 *
 * Scale 3 shares most visual state with Scale 2 (orbital clouds, bond
 * rendering, force arrows, etc.), so resetScale2() handles those.
 * This function clears molecule-specific state only.
 */
export function resetScale3(ctx) {
    _aeInitialEnergy = null;

    // Clear molecule info from inspector
    if (ctx.inspector) ctx.inspector.setCurrentMolecule(null);
}


// =====================================================================
// Exported: loadMoleculeScenario(ctx, name)
// =====================================================================
// Set up a molecule scenario in the AE engine.
// Originally app.js lines ~3913-3992.
//
// Handles:
//   - Data-driven molecular library (molecules.js): H2, H2O, ethanol, etc.
//   - Pre-bonding: establishes covalent bonds BEFORE the first tick
//     to prevent explosive LJ repulsion from atoms placed at bond distances
//   - Stability dry-run: one-tick test to detect unstable geometries
//   - NaCl crystal lattice (special scenario)
//   - Custom empty scenario
//
// The render loop is delegated to animateAE() from Scale 2.
//
// NOTE: ctx.resetAllVisualState() is called first to clear cross-scale
// visual state.  That function lives in app.js.

export function loadMoleculeScenario(ctx, name) {
    const { bridge, viewport, inspector } = ctx;

    if (!bridge.initAE) return;
    ctx.resetAllVisualState();
    bridge.initAE();

    // Reset toggles to defaults (bonding ON for molecules) then sync sliders
    _resetAETogglesToDefaults(bridge);
    _syncAEParamsFromUIInternal(bridge);

    // ── Data-driven molecular library ──────────────────────────────
    const molId = name.startsWith('mol-') ? name.slice(4) : null;
    if (molId && loadMolecule(bridge, molId)) {
        // Pre-bond: establish covalent bonds BEFORE the first tick.
        // Without this, atoms placed at covalent bond distances (inside the
        // LJ wall) experience explosive repulsive forces on the first tick.
        if (bridge.aePreBond) bridge.aePreBond();

        // Stability check: one-tick dry run to detect explosions
        const preData = bridge.aeGetAtomData();
        bridge.aeTick();
        const postData = bridge.aeGetAtomData();
        let maxDisp = 0;
        for (let i = 0; i < postData.count; i++) {
            const dx = postData.positions[i*3]   - preData.positions[i*3];
            const dy = postData.positions[i*3+1] - preData.positions[i*3+1];
            const dz = postData.positions[i*3+2] - preData.positions[i*3+2];
            maxDisp = Math.max(maxDisp, Math.sqrt(dx*dx + dy*dy + dz*dz));
        }
        if (maxDisp > 1) console.warn(`[FTD] ${molId}: UNSTABLE \u2014 max displacement ${maxDisp.toFixed(4)}`);

        // Reset to initial state (the dry-run consumed one tick)
        bridge.initAE();
        _syncAEParamsFromUIInternal(bridge);
        loadMolecule(bridge, molId);
        if (bridge.aePreBond) bridge.aePreBond();

        if (inspector) { inspector.setScenarioInfo(null); inspector.setCurrentMolecule(molId); }
        const mol = getMolecule(molId);
        if (mol?.cameraDistance && viewport) {
            viewport.controls.target.set(0, 0, 0);
            viewport.camera.position.set(0, 0, mol.cameraDistance);
            viewport.controls.update();
        }
        // Capture initial energy reference for drift tracking
        const initDiag = bridge.aeGetDiagnostics();
        if (initDiag.totalEnergy !== 0) _aeInitialEnergy = initDiag.totalEnergy;
        return;
    }

    // ── Fallback: crystal and custom scenarios ─────────────────────
    if (name === 'mol-crystal') {
        // NaCl 3x3x3 crystal lattice
        const sp = 7.5;
        for (let ix = 0; ix < 3; ix++) {
            for (let iy = 0; iy < 3; iy++) {
                for (let iz = 0; iz < 3; iz++) {
                    const x = (ix - 1) * sp;
                    const y = (iy - 1) * sp;
                    const z = (iz - 1) * sp;
                    if ((ix + iy + iz) % 2 === 0) {
                        bridge.aeAddAtom(11, x, y, z, 0, 0, 0, 1);
                    } else {
                        bridge.aeAddAtom(17, x, y, z, 0, 0, 0, -1);
                    }
                }
            }
        }
        if (bridge.aePreBond) bridge.aePreBond();
        if (inspector) inspector.setScenarioInfo({
            title: 'NaCl Ionic Crystal',
            desc: '3\u00d73\u00d73 rock salt lattice \u2014 alternating Na\u207a and Cl\u207b ions',
            fields: { 'Structure': 'FCC (rock salt)', 'Bonding': 'Ionic' }
        });
    }
    // mol-custom: empty, user builds manually

    // Capture initial energy reference for drift tracking
    const initDiag = bridge.aeGetDiagnostics();
    if (initDiag.totalEnergy !== 0) _aeInitialEnergy = initDiag.totalEnergy;
}

// Re-export animateAE so app.js can import it from either scale module.
// Scale 3 uses the exact same render loop as Scale 2 -- no separate
// animation function is needed.
export { animateAE } from '../scale2/controller.js';


// =====================================================================
// Exported: bindScale3ControlsUI()
// =====================================================================
// Mount Scale 3 control cards into the controls panel.
// Called once during app startup after the DOM is ready.

export function bindScale3ControlsUI() {
    const controlsPanel = document.getElementById('panel-controls');
    if (controlsPanel) new Scale3ControlsComponent(controlsPanel).init();
}

export function mount(ctx) {
    // standard placeholder
}

export function destroy(ctx) {
    resetScale3(ctx);
}
