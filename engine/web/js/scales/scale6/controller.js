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
 *     tetra+/-, BCC/FCC, orbit_rep/antipode, connections, axes, mirrors,
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

        // Capture the pre-Scale-6 camera/controls state so destroy() can restore
        // it for other scales (audit W1-1). The `!this._saved*` guard mirrors the
        // Scale-4 reference (scale4/controller.js ~L71-84): loadMetaScenario()
        // can re-run on an in-place reload WITHOUT an intervening destroy(), and
        // by then position/target have already been moved to the meta preset.
        // Capturing only on the first load (when _saved* is null) keeps the
        // originals pristine; destroy() nulls them so a later re-entry re-captures
        // a fresh baseline. Vectors are cloned so later camera motion (e.g.
        // auto-rotate / user orbit) doesn't mutate the saved snapshot. near/far
        // and minDistance/maxDistance are captured even though the meta preset
        // below currently leaves them untouched: this keeps the snapshot complete
        // and the restore self-contained (far + maxDistance are the load-bearing
        // restores the shared mode-switch path never re-derives — see destroy()).
        if (viewport && viewport.camera && !this._savedCamera) {
            this._savedCamera = {
                near: viewport.camera.near,
                far: viewport.camera.far,
                position: viewport.camera.position.clone(),
            };
        }
        if (viewport && viewport.controls && !this._savedControls) {
            this._savedControls = {
                minDistance: viewport.controls.minDistance,
                maxDistance: viewport.controls.maxDistance,
                target: viewport.controls.target.clone(),
            };
        }

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
            ['meta-toggle-gerade', 'toggleInversionDomain'],
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
        // Restore camera/controls clip planes + zoom limits + framing to their
        // pre-Scale-6 values (audit W1-1; originals captured in
        // loadMetaScenario). Mirrors the Scale-4 reference
        // (scale4/controller.js ~L318-340).
        //
        // Division of labour with viewport.setEngineMode() (called downstream
        // via the inspector/mode-sync path AFTER this destroy()): that shared
        // path re-derives camera.near, controls.minDistance, and re-centres
        // position/target for the destination scale — but it NEVER touches
        // camera.far or controls.maxDistance. far + maxDistance are therefore
        // the load-bearing restores here; near/position/min/target are restored
        // too (harmless — setEngineMode overwrites them) so this teardown stays
        // self-contained and Meta→Lattice leaves the lattice camera correct.
        if (ctx && ctx.viewport) {
            const viewport = ctx.viewport;
            if (this._savedCamera && viewport.camera) {
                viewport.camera.near = this._savedCamera.near;
                viewport.camera.far = this._savedCamera.far;
                viewport.camera.position.copy(this._savedCamera.position);
                viewport.camera.updateProjectionMatrix();
                this._savedCamera = null;
            }
            if (this._savedControls && viewport.controls) {
                viewport.controls.minDistance = this._savedControls.minDistance;
                viewport.controls.maxDistance = this._savedControls.maxDistance;
                viewport.controls.target.copy(this._savedControls.target);
                // Push the restored target/limits into OrbitControls now rather
                // than relying on a later controls.update() from whatever scale
                // mounts next — keeps the restore correct even if call order
                // changes or the destination skips its own update().
                if (typeof viewport.controls.update === 'function') {
                    viewport.controls.update();
                }
                this._savedControls = null;
            }
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
