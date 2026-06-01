/**
 * Scale 4 (Planetary) Controller
 *
 * Owns the Scale 4 N-Body physics loop, scenario loading, and UI list mapping.
 * Extracted from app.js to isolate planetary logic into a self-contained module.
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { PlanetaryMockBridge } from '../../bridge/mock-scale4.js';
import { PlanetaryRenderer } from '../../planetary-renderer.js';
import { rafCoordinator } from '../../lib/raf-coordinator.js';

// F-6: drive the planetary loop from the shared rAF coordinator instead of
// setInterval(…, 16). 60 Hz matches the old ~16 ms cadence (1000/16 ≈ 62.5),
// and because 60 ≥ the coordinator's VISIBILITY_PAUSE_THRESHOLD_HZ (30) the
// loop keeps advancing when the tab is backgrounded — exactly the opposite of
// setInterval, which throttles to ~1 Hz and lets the sim drift. Integration is
// frame-counted (accumulate ticksPerFrame → bridge.run(f) → 100 fixed-dt
// Velocity-Verlet substeps), NOT wall-clock-integrated, so swapping the timer
// source leaves trajectories bit-identical.
const PLANETARY_LOOP_HZ = 60;
const PLANETARY_LOOP_ID = 'scale4-planetary-loop';

class Scale4LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
        this.bridge = null;
        this.renderer = null;
        // Gravity-constant mode for Scale 4 (P0-1). 'decorative' = slow lattice
        // G (default, preserves prior UX); 'physical' = Keplerian 4π². Survives
        // bridge recreation on scenario reload; mirrored by #planetary-gravity-mode.
        this._gravityMode = 'decorative';
    }

    mount(ctx) {
        // Standard setup placeholder
    }

    loadScenario(ctx, name = 'planetary-solar') {
        const { viewport, inspector } = ctx;

        // Isolate visualization overlays
        if (viewport) {
            viewport.toggleFluxVolume(false);
            viewport.toggleFluxSlice(false);
            viewport.toggleGrid(false);
            viewport.toggleAxes(false);
            if (viewport.particles) viewport.particles.visible = false;
        }

        this.bridge = new PlanetaryMockBridge();
        // Persist the user's gravity-mode choice across the bridge recreate that
        // happens on every (re)load. A fresh bridge defaults to 'decorative';
        // re-apply the remembered mode BEFORE setupScenario so initial
        // velocities are generated with the correct G (P0-1).
        if (this._gravityMode) this.bridge.setGravityMode(this._gravityMode);
        this.bridge.setupScenario(name);

        if (this.renderer) {
            this.renderer.dispose();
        }
        this.renderer = new PlanetaryRenderer(viewport.scene, viewport.camera, viewport.renderer);
        this.trackThreeObject(this.renderer);

        if (inspector) inspector.setPlanetaryContext(this.bridge, this.renderer);

        // Capture the pre-Scale-4 camera/controls state so destroy() can restore
        // it for other scales (audit P1-8a). The `!this._saved*` guard is
        // load-bearing: loadScenario() re-runs on every in-place reload
        // (scenario change, gravity-mode change) WITHOUT an intervening
        // destroy(), and by then near/far/min/max have already been narrowed
        // below. Capturing only on the first load (when _saved* is null) keeps
        // the originals pristine; destroy() nulls them so a later re-entry
        // re-captures a fresh baseline. Vectors are cloned so later camera
        // motion doesn't mutate the saved snapshot.
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

        // Camera Presets
        viewport.camera.near = 0.001;
        viewport.camera.far = 1000;
        viewport.camera.updateProjectionMatrix();
        viewport.controls.minDistance = 0.01;
        viewport.controls.maxDistance = 100;
        viewport.camera.position.set(0, 5, 10);
        viewport.camera.lookAt(0, 0, 0);
        viewport.controls.target.set(0, 0, 0);

        const data = this.bridge.getPlanetaryData();
        this.renderer.update(data);

        this._populateLayerList(ctx);
        this._bindToggles(ctx);

        this._startPlanetaryLoop(ctx);
    }

    _startPlanetaryLoop(ctx) {
        // loadScenario() runs again on every in-place reload (scenario change,
        // gravity-mode change) WITHOUT an intervening destroy(), so tear down
        // any prior loop first — otherwise subscriptions stack and the sim runs
        // N× too fast after N reloads. (Wave-1 invariant, preserved for the rAF
        // path: rafCoordinator.subscribe with a duplicate id only replaces the
        // entry, but unsubscribing first keeps the contract explicit and lets
        // destroy() share one teardown helper.)
        this._stopPlanetaryLoop();

        // Per-frame fractional-tick accumulator (was a closure local under
        // setInterval; promoted to an instance field so a scenario reload that
        // re-subscribes does not silently inherit a stale partial tick).
        this._planetAcc = 0;

        // NOTE (P0-5): ctx is the live getter object from app.js _makeCtx(), so
        // ctx.running / ctx.engineMode / ctx.ticksPerFrame are read fresh each
        // frame — NOT captured snapshots. This is what makes pause work after
        // load. Do not destructure these into locals here.
        //
        // F-6: subscribe to the shared rAF coordinator at 60 Hz in place of
        // setInterval(…, 16). The body is unchanged, so the integration is
        // identical: same accumulation of ticksPerFrame, same bridge.run(f)
        // (100 fixed-dt Velocity-Verlet substeps per visual tick), same dt.
        this._planetaryLoopSub = rafCoordinator.subscribe(PLANETARY_LOOP_ID, {
            hz: PLANETARY_LOOP_HZ,
            cb: () => {
                // Guard: if we've switched away from planetary, idle.
                // (_stopPlanetaryLoop() in destroy() also drops this
                // subscription; this is belt-and-suspenders for the
                // in-place-reload window.)
                if (ctx.engineMode !== 'planetary') {
                    return;
                }

                if (ctx.running) {
                    this._planetAcc += ctx.ticksPerFrame || 1;
                    const f = Math.floor(this._planetAcc);
                    this._planetAcc -= f;
                    if (f > 0 && this.bridge) this.bridge.run(f);
                }

                if (this.bridge && this.renderer) {
                    const currentData = this.bridge.getPlanetaryData();
                    this.renderer.update(currentData);
                }

                // Update DOM diagnostics and inspector state
                if (ctx.inspector) ctx.inspector.update();
                if (ctx.viewport) ctx.viewport.render();
            },
        });
    }

    /**
     * Tear down the planetary frame loop. Idempotent; safe to call when no
     * loop is active. (F-6: replaces the Wave-1 clearInterval teardown.)
     */
    _stopPlanetaryLoop() {
        if (this._planetaryLoopSub) {
            this._planetaryLoopSub.unsubscribe();
            this._planetaryLoopSub = null;
        }
    }

    _populateLayerList(ctx) {
        const layerList = document.getElementById('planetary-layer-list');
        if (layerList && this.bridge && this.bridge._bodies) {
            layerList.innerHTML = '';
            this.bridge._bodies.forEach((b) => {
                const li = document.createElement('li');
                li.style.padding = '4px 8px';
                li.style.borderBottom = '1px solid #1a1a2e';
                li.style.fontSize = '12px';
                li.style.cursor = 'pointer';
                
                let name = b.type === 0 ? "Host Star" : (b.type === 2 ? "Gas Giant" : "Rocky Planet");
                li.textContent = `ID ${b.id}: ${name} [Mass: ${b.mass.toFixed(4)}]`;

                li.onmouseenter = () => { li.style.background = '#1a1a2e'; };
                li.onmouseleave = () => { li.style.background = 'transparent'; };
                li.onclick = () => {
                    if (ctx.inspector) {
                        ctx.inspector.setEngineMode('planetary');
                        ctx.inspector._selectedPlanetaryId = b.id;
                        const btn = document.querySelector('.tab[data-panel="inspector"]');
                        if (btn) btn.click();
                        ctx.inspector._showPlanetaryInspector();
                    }
                };
                layerList.appendChild(li);
            });
        }
    }

    _bindToggles(ctx) {
        // loadScenario() can run repeatedly without an intervening destroy()
        // (the scenario <select> reloads in place), so guard each binding with
        // a dataset flag to avoid stacking duplicate listeners.
        //
        // NOTE (audit §E / P1-8 dead-UI): #planetary-opt-orbits and
        // #planetary-opt-axes are NOT dead. The audit checked index.html only;
        // these checkboxes are rendered at runtime by getPlanetaryPanelTemplate()
        // in js/ui/components/panel-resources/template.js (the "Visualization
        // Overlays" card of #panel-planetary), alongside #planetary-layer-list
        // which _populateLayerList() fills. Both bindings drive real renderer
        // behaviour — setRenderOrbits() toggles orbit-line visibility,
        // setRenderAxes() lazily builds + toggles a per-mesh AxesHelper
        // (planetary-renderer.js). Keep them. Do NOT also render these IDs in the
        // Scale-4 toolbar template: duplicate element IDs would split the toggle
        // state across two checkboxes and getElementById would bind only the
        // first.
        const optOrbits = document.getElementById('planetary-opt-orbits');
        if (optOrbits && !optOrbits.dataset.s4Bound) {
            optOrbits.dataset.s4Bound = '1';
            this.bindEvent(optOrbits, 'change', (e) => {
                if (this.renderer) this.renderer.setRenderOrbits(e.target.checked);
            });
        }
        const optAxes = document.getElementById('planetary-opt-axes');
        if (optAxes && !optAxes.dataset.s4Bound) {
            optAxes.dataset.s4Bound = '1';
            this.bindEvent(optAxes, 'change', (e) => {
                if (this.renderer) this.renderer.setRenderAxes(e.target.checked);
            });
        }

        // Gravity-constant mode toggle (P0-1). this._gravityMode is the
        // controller-level source of truth (the bridge is recreated on every
        // reload, so the choice must survive at controller scope). On change,
        // remember the mode and reload the current scenario so initial
        // velocities are regenerated for the new G — orbits run ~63× faster in
        // 'physical' (Keplerian) vs 'decorative' (slow lattice-G default).
        const gravSel = document.getElementById('planetary-gravity-mode');
        if (gravSel && !gravSel.dataset.s4Bound) {
            gravSel.dataset.s4Bound = '1';
            this.bindEvent(gravSel, 'change', (e) => {
                this._gravityMode = e.target.value;
                const scenario = document.getElementById('planetary-scenario-select')?.value
                    || this.bridge?._scenarioName
                    || 'planetary-solar';
                this.loadScenario(ctx, scenario);
            });
        }
        // Keep the visible selection in sync with the active mode across both
        // first mount and in-place scenario reloads.
        if (gravSel) gravSel.value = this._gravityMode;

        this._updateOverlayStatus();
    }

    /**
     * Update the viewport overlay status line so the UI does not assert AU/yr
     * timing fidelity while in 'decorative' mode (P0-1). In 'physical' mode the
     * Keplerian AU/M☉/yr timing is faithful, so the label says so.
     */
    _updateOverlayStatus() {
        const statusEl = document.getElementById('planetary-overlay-status');
        if (!statusEl) return;
        statusEl.textContent = this._gravityMode === 'physical'
            ? 'Orbital mechanics — Physical (Keplerian AU/M☉/yr; Earth year = 1 sim yr)'
            : 'Orbital mechanics — Decorative (visual cadence; not AU/yr-faithful)';
    }

    step() {
        if (this.bridge) {
            this.bridge.run(1);
            if (this.renderer) {
                const currentData = this.bridge.getPlanetaryData();
                this.renderer.update(currentData);
            }
        }
    }

    destroy(ctx) {
        // F-6: the frame loop is now an rAF-coordinator subscription, not a
        // tracked setInterval, so super.destroy() no longer clears it. Drop the
        // subscription explicitly before the base teardown so a later re-mount
        // starts a fresh one and the coordinator stops its rAF when no other
        // subscribers remain.
        this._stopPlanetaryLoop();
        super.destroy(ctx);
        // The toolbar toggle elements persist in the DOM across scale switches
        // (hidden via .scale4-only). super.destroy() removed their listeners, so
        // clear the bind-guard flags too; otherwise re-entering Scale 4 would
        // see s4Bound and skip re-binding, leaving the toggles dead.
        ['planetary-opt-orbits', 'planetary-opt-axes', 'planetary-gravity-mode'].forEach((id) => {
            const el = document.getElementById(id);
            if (el) delete el.dataset.s4Bound;
        });
        if (this.renderer) {
            this.renderer.dispose();
            this.renderer = null;
        }
        this.bridge = null;
        // Restore lattice particles visibility for other scales
        if (ctx && ctx.viewport && ctx.viewport.particles) {
            ctx.viewport.particles.visible = true;
        }
        // Restore camera/controls clip planes + zoom limits to their
        // pre-Scale-4 values (audit P1-8a; originals captured in loadScenario).
        //
        // Division of labour with viewport.setEngineMode() (called downstream
        // via inspectorRuntime.syncMode AFTER this destroy()): that path already
        // re-derives camera.near, controls.minDistance, and re-centres
        // position/target for the destination scale — but it NEVER touches
        // camera.far or controls.maxDistance. Scale 4 narrows both
        // (far 2000→1000, maxDistance 500→100), so without this restore a
        // Scale 4→0 switch leaves the lattice with a 1000-unit far plane and a
        // 100-unit zoom cap → far geometry culled / z-fighting. far + maxDistance
        // are therefore the load-bearing restores here; near/position/min/target
        // are restored too (harmless — setEngineMode overwrites them) so this
        // teardown stays self-contained.
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

const _lifecycleController = new Scale4LifecycleController();

export function mount(ctx) {
    _lifecycleController.mount(ctx);
}

export function destroy(ctx) {
    _lifecycleController.destroy(ctx);
}

export function loadScenario(ctx, name = 'planetary-solar') {
    _lifecycleController.loadScenario(ctx, name);
}

export function step() {
    _lifecycleController.step();
}

export function dispose(ctx) {
    _lifecycleController.destroy(ctx);
}

