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
 * Physics preserved exactly from the main dashboard controller:
 *   - Camera preset: position (5, 3.5, 5), target origin
 *   - Toggle map: 13 geometric layers (center, oct, cuboct, cube,
 *     tetra+/-, BCC/FCC, gerade/ungerade, connections, axes, mirrors,
 *     labels, auto-rotate)
 *   - Info panel built via buildMetaInfoPanel (pedagogy)
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { MetaUnit } from '../../meta-unit.js';
import { buildMetaInfoPanel, buildSiteInspectPanel } from '../../meta-pedagogy.js';
import { debugLog } from '../../core/log.js';

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------

class Scale6LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
        this.metaUnit = null;
    }

    mount(ctx) {
        // Standard setup placeholder
    }

    loadMetaScenario(ctx) {
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
        if (this.metaUnit) {
            if (this.metaUnit.dispose) this.metaUnit.dispose();
            this.metaUnit = null;
        }
        if (viewport.scene) {
            this.metaUnit = new MetaUnit(viewport.scene, viewport.camera, viewport.renderer);
            this.trackThreeObject(this.metaUnit);
        }

        // Build the info panel (pass metaUnit so panel buttons can drive the 3D view)
        const infoContainer = document.getElementById('meta-info-panel');
        if (infoContainer) {
            buildMetaInfoPanel(infoContainer, this.metaUnit);
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
            if (el && this.metaUnit) {
                this.bindEvent(el, 'click', () => {
                    el.classList.toggle('active');
                    if (this.metaUnit) {
                        this.metaUnit[method](el.classList.contains('active'));
                    }
                });
                // Apply initial state from button's active class
                this.metaUnit[method](el.classList.contains('active'));
            }
        }

        // Auto-select the Meta tab in the sidebar
        const metaTab = document.querySelector('.tab[data-panel="meta-info"]');
        if (metaTab) metaTab.click();

        debugLog('[FTD] Meta Scale: Existential Unit loaded');
    }

    update(dt) {
        if (this.metaUnit && this.metaUnit.update) {
            this.metaUnit.update(dt);
        }
    }

    destroy(ctx) {
        super.destroy(ctx);
        if (this.metaUnit) {
            if (this.metaUnit.dispose) this.metaUnit.dispose();
            this.metaUnit = null;
        }
        // Restore lattice particles visibility for other scales
        if (ctx && ctx.viewport && ctx.viewport.particles) {
            ctx.viewport.particles.visible = true;
        }
    }
}

const _lifecycleController = new Scale6LifecycleController();

export function mount(ctx) {
    _lifecycleController.mount(ctx);
}

export function destroy(ctx) {
    _lifecycleController.destroy(ctx);
}

export function loadMetaScenario(ctx) {
    _lifecycleController.loadMetaScenario(ctx);
}

export function updateMeta(ctx, dt) {
    _lifecycleController.update(dt);
    if (ctx && ctx.viewport) ctx.viewport.render();
}

export function step(ctx) {
    _lifecycleController.update(1 / 60);
    if (ctx && ctx.viewport) ctx.viewport.render();
}

export function resetScale6(ctx) {
    _lifecycleController.destroy(ctx);
}
