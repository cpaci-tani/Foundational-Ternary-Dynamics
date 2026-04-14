/**
 * Scale 6 — Meta Controller
 *
 * Manages the meta scale: the 3^3 = 27-site "existential unit" that
 * visualizes the Moore neighborhood decomposition (octahedron +
 * cuboctahedron + stella octangula = SC + FCC + BCC).
 *
 * This is the highest scale — it shows the geometric skeleton from which
 * gauge groups, generations, and dark states emerge.  The MetaUnit class
 * handles all Three.js geometry; this controller wires the DOM toggles
 * and the info/inspect pedagogy panels.
 *
 * Physics preserved exactly from app.js:
 *   - Camera preset: position (5, 3.5, 5), target origin
 *   - Toggle map: 13 geometric layers (center, oct, cuboct, cube,
 *     tetra+/-, BCC/FCC, gerade/ungerade, connections, axes, mirrors,
 *     labels, auto-rotate)
 *   - Info panel built via buildMetaInfoPanel (pedagogy)
 */

import { MetaUnit } from '../../meta-unit.js?v=20260405a';
import { buildMetaInfoPanel, buildSiteInspectPanel } from '../../meta-pedagogy.js?v=20260405a';

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------

let metaUnit = null;   // MetaUnit instance (Three.js group in scene)

// ---------------------------------------------------------------------------
// loadMetaScenario  -- initialize the meta scale view
// ---------------------------------------------------------------------------

/**
 * Initialize the Meta Scale: 3^3 existential unit visualization.
 * Hides all Scale 0 visuals, positions the camera, creates the MetaUnit,
 * builds the pedagogy info panel, and wires 13 geometric toggle buttons.
 *
 * @param {object} ctx - Shared context:
 *   { bridge, viewport, running, updatePlayButton, _resetAllVisualState }
 */
export function loadMetaScenario(ctx) {
    ctx._resetAllVisualState();
    ctx.running = false;
    ctx.updatePlayButton();

    const viewport = ctx.viewport;

    // Hide ALL Scale 0 visuals -- flux volume, wireframe, particles, fields
    if (viewport) {
        viewport.toggleFluxVolume(false);
        viewport.toggleFluxSlice(false);
        viewport.toggleGrid(false);
        // Clear existing particle cloud
        if (viewport.particles) viewport.particles.visible = false;
        // Hide any field lines
        viewport.toggleEFieldLines(false);
        viewport.toggleBFieldLines(false);
    }
    const fvBtn = document.getElementById('toggle-flux-volume');
    if (fvBtn) fvBtn.classList.remove('active');
    const gridBtn = document.getElementById('toggle-grid');
    if (gridBtn) gridBtn.classList.remove('active');

    // Position camera for meta view (close-up, centered on origin)
    if (viewport.camera && viewport.controls) {
        viewport.controls.target.set(0, 0, 0);
        viewport.camera.position.set(5, 3.5, 5);
        viewport.controls.update();
    }

    // Dispose stale MetaUnit and recreate
    if (metaUnit) {
        if (metaUnit.dispose) metaUnit.dispose();
        metaUnit = null;
    }
    if (viewport.scene) {
        metaUnit = new MetaUnit(viewport.scene, viewport.camera, viewport.renderer);
    }

    // Build the info panel (pass metaUnit so panel buttons can drive the 3D view)
    const infoContainer = document.getElementById('meta-info-panel');
    if (infoContainer) {
        buildMetaInfoPanel(infoContainer, metaUnit);
    }

    // Wire up meta toggles -- each button toggles a geometric layer
    const toggleIds = [
        ['meta-toggle-center', 'toggleCenter'],
        ['meta-toggle-oct', 'toggleOctahedron'],
        ['meta-toggle-cuboct', 'toggleCuboctahedron'],
        ['meta-toggle-cube', 'toggleCube'],
        ['meta-toggle-tetra-plus', 'toggleTetraPlus'],
        ['meta-toggle-tetra-minus', 'toggleTetraMinus'],
        ['meta-toggle-bcc-fcc', 'toggleBCCFCC'],
        ['meta-toggle-gerade', 'toggleGeradeUngerade'],
        ['meta-toggle-connections', 'toggleConnections'],
        ['meta-toggle-axes', 'toggleRotationAxes'],
        ['meta-toggle-mirrors', 'toggleMirrorPlanes'],
        ['meta-toggle-labels', 'toggleFrameworkLabels'],
        ['meta-toggle-rotate', 'toggleAutoRotate'],
    ];
    for (const [elId, method] of toggleIds) {
        const el = document.getElementById(elId);
        if (el && metaUnit) {
            el.onclick = () => {
                el.classList.toggle('active');
                metaUnit[method](el.classList.contains('active'));
            };
            // Apply initial state from button's active class
            metaUnit[method](el.classList.contains('active'));
        }
    }

    // Auto-select the Meta tab in the sidebar
    const metaTab = document.querySelector('.tab[data-panel="meta-info"]');
    if (metaTab) metaTab.click();

    console.log('[FTD] Meta Scale: Existential Unit loaded');
}

// ---------------------------------------------------------------------------
// updateMeta  -- per-frame update (called from the main rAF loop)
// ---------------------------------------------------------------------------

/**
 * Per-frame meta update. Drives auto-rotate and any future per-tick
 * animations. Called from the main rAF loop.
 *
 * @param {object} ctx - Shared context: { viewport }
 * @param {number} dt - Time delta in seconds since last frame
 */
export function updateMeta(ctx, dt) {
    if (metaUnit && metaUnit.update) {
        metaUnit.update(dt);
    }
    if (ctx && ctx.viewport) ctx.viewport.render();
}

// ---------------------------------------------------------------------------
// step  -- single-tick step (called from Step button)
// ---------------------------------------------------------------------------

/**
 * Advance the meta visualization by one frame step and re-render.
 * Meta scale has no simulation; this just updates auto-rotate.
 *
 * @param {object} ctx - Shared context: { viewport }
 */
export function step(ctx) {
    if (metaUnit && metaUnit.update) metaUnit.update(1 / 60);
    if (ctx && ctx.viewport) ctx.viewport.render();
}

// ---------------------------------------------------------------------------
// resetScale6  -- tear down meta state on scale switch
// ---------------------------------------------------------------------------

/**
 * Clean up Scale 6 state when leaving meta mode.
 * Disposes the MetaUnit so it can be recreated fresh on return.
 *
 * @param {object} ctx - Shared context (unused for now, reserved for future cleanup)
 */
export function resetScale6(ctx) {
    if (metaUnit) {
        if (metaUnit.dispose) metaUnit.dispose();
        metaUnit = null;
    }
    // Restore lattice particles visibility for other scales
    if (ctx && ctx.viewport && ctx.viewport.particles) {
        ctx.viewport.particles.visible = true;
    }
}
