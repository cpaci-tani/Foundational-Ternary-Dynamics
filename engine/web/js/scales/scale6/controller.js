/**
 * Scale 6 (Meta / Existential Unit) Controller
 * ────────────────────────────────────────────────────────────────────
 *
 * Reconnects the previously-orphaned MetaUnit module (the 27-site Moore
 * neighborhood decomposition — octahedron/cuboctahedron/stella octangula)
 * as a genuine, mountable scale, following the BaseLifecycleController
 * pattern used by scale0-5.
 *
 * The pedagogy panel uses the shared instrument-panel lifecycle owner. Its
 * responsive placement is CSS-owned and consumes the shell safe-area
 * contract rather than an inline fixed-width style island.
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
import { mountInstrumentPanel } from '../../ui/utils/instrument-panel.js';
import { hideScale0Overlays, saveScaleCameraState, restoreScaleCameraState } from '../scale-utils.js';

const META_LOOP_ID = 'scale6-meta-loop';
const META_LOOP_HZ = 30;
const PANEL_ID = 'scale6-meta-panel';

class Scale6LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
        this.metaUnit = null;
        this._panelOwner = null;
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

        this._disposeMetaUnit();
        this.metaUnit = new MetaUnit(viewport.scene, viewport.camera, viewport.renderer);

        // Camera framing — capture pre-Scale-6 state once (mirrors Scale 4's
        // P1-8a restore pattern) so destroy() can put other scales' camera
        // back exactly where they were.
        saveScaleCameraState(this, viewport);
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
        // Shell-level portal: viewport establishes its own stacking context, so
        // short/mobile instrument panels mounted inside it cannot rise above
        // transport and dock siblings even with a larger z-index.
        const host = document.getElementById('app') || document.body;
        this._panelOwner?.dispose();
        this._panelOwner = mountInstrumentPanel({
            id: PANEL_ID,
            host,
            className: 'meta-floating-panel',
            label: 'Existential unit details',
            build: (panel) => buildMetaInfoPanel(panel, this.metaUnit),
            collapsible: true,
        });
        this._panelEl = this._panelOwner.element;
    }

    _bindClickInspector(ctx) {
        const { viewport } = ctx;
        const canvas = viewport?.renderer?.domElement;
        if (!canvas) return;

        this._raycaster = new THREE.Raycaster();
        const ndc = new THREE.Vector2();

        if (this._pointerDownHandler) return;
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
                if (ctx.presentationSuspended) return;
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

    _disposeMetaUnit() {
        if (!this.metaUnit) return;
        this.metaUnit.dispose();
        this.metaUnit = null;
    }

    destroy(ctx) {
        this._stopLoop();
        super.destroy(ctx); // unbinds the pointerdown listener tracked via bindEvent
        this._pointerDownHandler = null;
        this._raycaster = null;

        this._panelOwner?.dispose();
        this._panelOwner = null;
        this._panelEl = null;
        this._disposeMetaUnit();

        if (ctx && ctx.viewport) {
            restoreScaleCameraState(this, ctx.viewport);
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

/** MetaUnit has no independent axes/grid/clock render layers. */
export function getViewControlCapabilities() {
    return { axes: false, grid: false, boundaryOrientation: false, globalClock: false };
}

export function getViewControlState() {
    return { axes: false, grid: false };
}
