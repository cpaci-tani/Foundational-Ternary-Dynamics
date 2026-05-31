/**
 * Scale 4 (Planetary) Controller
 *
 * Owns the Scale 4 N-Body physics loop, scenario loading, and UI list mapping.
 * Extracted from app.js to isolate planetary logic into a self-contained module.
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { PlanetaryMockBridge } from '../../bridge/mock-scale4.js';
import { PlanetaryRenderer } from '../../planetary-renderer.js';

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

        // Save prior camera/controls state so destroy() can restore for other scales
        // (audit P1-8 fix, 2026-05-27).
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
        // gravity-mode change) WITHOUT an intervening destroy(), so clear any
        // prior loop first — otherwise intervals stack and the sim runs N× too
        // fast after N reloads.
        if (this._planetaryIntervalId != null) {
            clearInterval(this._planetaryIntervalId);
            this._planetaryIntervalId = null;
        }

        let planetAcc = 0;
        // NOTE (P0-5): ctx is the live getter object from app.js _makeCtx(), so
        // ctx.running / ctx.engineMode / ctx.ticksPerFrame are read fresh each
        // tick — NOT captured snapshots. This is what makes pause work after
        // load. Do not destructure these into locals here.
        this._planetaryIntervalId = this.setInterval(() => {
            // Guard: if we've switched away from planetary, idle. (destroy()
            // also clears this interval; this is belt-and-suspenders for the
            // in-place-reload window.)
            if (ctx.engineMode !== 'planetary') {
                return;
            }

            if (ctx.running) {
                planetAcc += ctx.ticksPerFrame || 1;
                const f = Math.floor(planetAcc);
                planetAcc -= f;
                if (f > 0 && this.bridge) this.bridge.run(f);
            }

            if (this.bridge && this.renderer) {
                const currentData = this.bridge.getPlanetaryData();
                this.renderer.update(currentData);
            }

            // Update DOM diagnostics and inspector state
            if (ctx.inspector) ctx.inspector.update();
            if (ctx.viewport) ctx.viewport.render();
        }, 16);
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
        super.destroy(ctx);
        // super.destroy() cleared the tracked loop interval; drop our handle so
        // a later re-mount starts a fresh one.
        this._planetaryIntervalId = null;
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
        // Restore camera/controls (audit P1-8 fix, 2026-05-27)
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

