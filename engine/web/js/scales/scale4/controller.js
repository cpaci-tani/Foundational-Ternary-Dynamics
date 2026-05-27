/**
 * Scale 4 (Planetary) Controller
 *
 * Owns the Scale 4 N-Body physics loop, scenario loading, and UI list mapping.
 * Extracted from app_dag.js to isolate planetary logic into a self-contained module.
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { PlanetaryMockBridge } from '../../bridge/mock-scale4.js';
import { PlanetaryRenderer } from '../../planetary-renderer.js';

class Scale4LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
        this.bridge = null;
        this.renderer = null;
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
        this.bridge.setupScenario(name);

        if (this.renderer) {
            this.renderer.dispose();
        }
        this.renderer = new PlanetaryRenderer(viewport.scene, viewport.camera, viewport.renderer);
        this.trackThreeObject(this.renderer);
        
        if (inspector) inspector.setPlanetaryContext(this.bridge, this.renderer);

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
        this._bindToggles();

        this._startPlanetaryLoop(ctx);
    }

    _startPlanetaryLoop(ctx) {
        let planetAcc = 0;
        this.setInterval(() => {
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

    _bindToggles() {
        const optOrbits = document.getElementById('planetary-opt-orbits');
        if (optOrbits) {
            this.bindEvent(optOrbits, 'change', (e) => {
                if (this.renderer) this.renderer.setRenderOrbits(e.target.checked);
            });
        }
        const optAxes = document.getElementById('planetary-opt-axes');
        if (optAxes) {
            this.bindEvent(optAxes, 'change', (e) => {
                if (this.renderer) this.renderer.setRenderAxes(e.target.checked);
            });
        }
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
        if (this.renderer) {
            this.renderer.dispose();
            this.renderer = null;
        }
        this.bridge = null;
        // Restore lattice particles visibility for other scales
        if (ctx && ctx.viewport && ctx.viewport.particles) {
            ctx.viewport.particles.visible = true;
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

