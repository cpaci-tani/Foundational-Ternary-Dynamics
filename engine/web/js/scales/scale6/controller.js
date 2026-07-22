/**
 * Scale 6 (Meta / Existential Unit) Controller
 * ────────────────────────────────────────────────────────────────────
 *
 * Reconnects the previously-orphaned MetaUnit module (the 27-site Moore
 * neighborhood decomposition — octahedron/cuboctahedron/stella octangula)
 * as a genuine, mountable scale, following the BaseLifecycleController
 * pattern used by scale0-5.
 *
 * Design choice (2026-07-14): rather than wiring into the panel-dock/tab
 * subsystem (a large, separate area with its own visibility rules that
 * every other scale's panels participate in), this controller follows the
 * simpler self-contained floating-panel pattern already established by
 * scales/scale0/ui/overlays/genesis-burst-panel.js — the pedagogy panel is
 * appended directly to the viewport on mount and removed on destroy. This
 * delivers the same functional outcome (a real, interactive, clickable
 * Scale-6 exhibit) without touching the tab-dock system other scales share.
 *
 * MetaUnit itself needs only (scene, camera, renderer) — no bridge, no
 * physics tick loop. Self-driven via the shared rafCoordinator (matching
 * Scale 4's precedent: 'planetary' is a no-op in app.js's central animate()
 * and instead runs its own subscription — see scales/scale4/controller.js).
 */

import * as THREE from 'three';
import { BaseLifecycleController } from '../../lifecycle.js';
import { MetaUnit } from '../../meta-unit.js';
import { buildMetaInfoPanel, buildSiteInspectPanel } from '../../meta-pedagogy.js';
import { rafCoordinator } from '../../lib/raf-coordinator.js';
import { hideScale0Overlays } from '../scale-utils.js';

const META_LOOP_ID = 'scale6-meta-loop';
const META_LOOP_HZ = 30;
const PANEL_ID = 'scale6-meta-panel';

class Scale6LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
        this.metaUnit = null;
        this._panelEl = null;
        this._raycaster = null;
        this._pointerDownHandler = null;
        this._savedCamera = null;
        this._savedControls = null;
    }

    mount(ctx) {
        // Standard setup placeholder — real work happens in loadScenario,
        // matching the scale4/scale5 convention (mount is a no-op; the
        // scenario loader does the heavy lifting and is re-entrant).
    }

    loadScenario(ctx) {
        const { viewport } = ctx;
        if (!viewport) return;

        // Hide Scale-0 lattice overlays (flux volume/slice, grid/axes, the
        // manifested-particle cloud) so nothing from a prior lattice session
        // visually clutters the meta-unit exhibit.
        hideScale0Overlays(viewport);

        if (this.metaUnit) {
            this.metaUnit.dispose();
            this.metaUnit = null;
        }
        this.metaUnit = new MetaUnit(viewport.scene, viewport.camera, viewport.renderer);
        this.trackThreeObject(this.metaUnit._root);

        // Camera framing — capture pre-Scale-6 state once (mirrors Scale 4's
        // P1-8a restore pattern) so destroy() can put other scales' camera
        // back exactly where they were.
        if (viewport.camera && !this._savedCamera) {
            this._savedCamera = {
                near: viewport.camera.near,
                far: viewport.camera.far,
                position: viewport.camera.position.clone(),
            };
        }
        if (viewport.controls && !this._savedControls) {
            this._savedControls = {
                minDistance: viewport.controls.minDistance,
                maxDistance: viewport.controls.maxDistance,
                target: viewport.controls.target.clone(),
            };
        }
        // The 27-site unit is centered at local (0,0,0) (meta-unit.js's own
        // MetaUnit._root origin — confirmed by inspection, not (1,1,1) as an
        // earlier draft of this file assumed) — frame it directly.
        viewport.camera.near = 0.01;
        viewport.camera.far = 200;
        viewport.camera.updateProjectionMatrix();
        viewport.camera.position.set(5, 4, 7);
        viewport.camera.lookAt(0, 0, 0);
        if (viewport.controls) {
            viewport.controls.minDistance = 1;
            viewport.controls.maxDistance = 60;
            viewport.controls.target.set(0, 0, 0);
            if (typeof viewport.controls.update === 'function') viewport.controls.update();
        }

        this._mountPanel(ctx);
        this._bindClickInspector(ctx);
        this._startLoop(ctx);
    }

    _mountPanel(ctx) {
        const host = document.getElementById('viewport') || document.body;
        document.getElementById(PANEL_ID)?.remove();
        const panel = document.createElement('div');
        panel.id = PANEL_ID;
        panel.className = 'meta-floating-panel';
        panel.style.cssText =
            'position:absolute; top:12px; left:12px; z-index:40; width:300px; ' +
            'max-height:calc(100% - 24px); overflow-y:auto; border-radius:12px; ' +
            'font-family:var(--font-sans,sans-serif); font-size:12px; ' +
            'background:var(--color-background-primary,rgba(20,20,24,0.92)); ' +
            'border:0.5px solid var(--color-border-secondary,rgba(255,255,255,0.25)); ' +
            'color:var(--color-text-primary,#eee); box-shadow:0 2px 12px rgba(0,0,0,0.3);';
        host.appendChild(panel);
        this._panelEl = panel;
        buildMetaInfoPanel(panel, this.metaUnit);
    }

    _bindClickInspector(ctx) {
        const { viewport } = ctx;
        const canvas = viewport?.renderer?.domElement;
        if (!canvas) return;

        this._raycaster = new THREE.Raycaster();
        const ndc = new THREE.Vector2();

        this._pointerDownHandler = (ev) => {
            const rect = canvas.getBoundingClientRect();
            ndc.x = ((ev.clientX - rect.left) / rect.width) * 2 - 1;
            ndc.y = -((ev.clientY - rect.top) / rect.height) * 2 + 1;
            this._raycaster.setFromCamera(ndc, viewport.camera);
            const siteInfo = this.metaUnit?.inspectSite(this._raycaster) ?? null;
            if (siteInfo && this._panelEl) buildSiteInspectPanel(this._panelEl, siteInfo);
        };
        this.bindEvent(canvas, 'pointerdown', this._pointerDownHandler);
    }

    _startLoop(ctx) {
        this._stopLoop();
        let lastT = null;
        this._loopSub = rafCoordinator.subscribe(META_LOOP_ID, {
            hz: META_LOOP_HZ,
            cb: () => {
                if (ctx.engineMode !== 'meta') return;
                const now = performance.now();
                const dt = lastT === null ? 0 : (now - lastT) / 1000;
                lastT = now;
                if (this.metaUnit) this.metaUnit.update(dt);
                if (ctx.viewport) ctx.viewport.render();
            },
        });
    }

    _stopLoop() {
        if (this._loopSub) {
            this._loopSub.unsubscribe();
            this._loopSub = null;
        }
    }

    destroy(ctx) {
        this._stopLoop();
        super.destroy(ctx); // unbinds the pointerdown listener tracked via bindEvent
        this._pointerDownHandler = null;
        this._raycaster = null;

        if (this._panelEl) {
            this._panelEl.remove();
            this._panelEl = null;
        }
        if (this.metaUnit) {
            this.metaUnit.dispose();
            this.metaUnit = null;
        }

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
                if (typeof viewport.controls.update === 'function') viewport.controls.update();
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

export function loadScenario(ctx) {
    _lifecycleController.loadScenario(ctx);
}
