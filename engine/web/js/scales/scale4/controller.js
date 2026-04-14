/**
 * Scale 4 (Planetary) Controller
 *
 * Owns the Scale 4 N-Body physics loop, scenario loading, and UI list mapping.
 * Extracted from app_dag.js to isolate planetary logic into a self-contained module.
 */

import { PlanetaryMockBridge } from '../../bridge/mock-scale4.js?v=20260408b';
import { PlanetaryRenderer } from '../../planetary-renderer.js?v=20260408b';
import { bus, EVENTS } from '../../core/event-bus.js';

let _planetaryBridge = null;
let _planetaryRenderer = null;

/**
 * animatePlanetary — Frame loop logic run by the specific interval.
 */
function _startPlanetaryLoop(ctx) {
    if (window._planetaryInterval) clearInterval(window._planetaryInterval);
    
    let planetAcc = 0;
    window._planetaryInterval = setInterval(() => {
        if (ctx.engineMode !== 'planetary') {
            clearInterval(window._planetaryInterval);
            return;
        }
        
        if (ctx.running) {
            planetAcc += ctx.ticksPerFrame || 1;
            const f = Math.floor(planetAcc);
            planetAcc -= f;
            if (f > 0 && _planetaryBridge) _planetaryBridge.run(f);
        }
        
        if (_planetaryBridge && _planetaryRenderer) {
            const currentData = _planetaryBridge.getPlanetaryData();
            _planetaryRenderer.update(currentData);
        }
        
        // Update DOM diagnostics and inspector state
        if (ctx.inspector) ctx.inspector.update();
        if (ctx.viewport) ctx.viewport.render();
    }, 16);
}

/**
 * loadScenario — Instantiate the simulation and bridge, bind UI.
 */
export function loadScenario(ctx, name = 'planetary-solar') {
    const { viewport, inspector } = ctx;

    // Isolate visualization overlays
    if (viewport) {
        viewport.toggleFluxVolume(false);
        viewport.toggleFluxSlice(false);
        viewport.toggleGrid(false);
        viewport.toggleAxes(false);
        if (viewport.particles) viewport.particles.visible = false;
    }

    _planetaryBridge = new PlanetaryMockBridge();
    _planetaryBridge.setupScenario(name);

    if (_planetaryRenderer) _planetaryRenderer.dispose();
    _planetaryRenderer = new PlanetaryRenderer(viewport.scene, viewport.camera, viewport.renderer);
    
    if (inspector) inspector.setPlanetaryContext(_planetaryBridge, _planetaryRenderer);

    // Camera Presets
    viewport.camera.near = 0.001;
    viewport.camera.far = 1000;
    viewport.camera.updateProjectionMatrix();
    viewport.controls.minDistance = 0.01;
    viewport.controls.maxDistance = 100;
    viewport.camera.position.set(0, 5, 10);
    viewport.camera.lookAt(0, 0, 0);
    viewport.controls.target.set(0, 0, 0);

    const data = _planetaryBridge.getPlanetaryData();
    _planetaryRenderer.update(data);

    _populateLayerList(ctx);
    _bindToggles();

    _startPlanetaryLoop(ctx);
}

function _populateLayerList(ctx) {
    const layerList = document.getElementById('planetary-layer-list');
    if (layerList && _planetaryBridge && _planetaryBridge._bodies) {
        layerList.innerHTML = '';
        _planetaryBridge._bodies.forEach((b) => {
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

function _bindToggles() {
    const optOrbits = document.getElementById('planetary-opt-orbits');
    if (optOrbits) {
        optOrbits.onchange = (e) => {
            if (_planetaryRenderer) _planetaryRenderer.setRenderOrbits(e.target.checked);
        };
    }
    const optAxes = document.getElementById('planetary-opt-axes');
    if (optAxes) {
        optAxes.onchange = (e) => {
            if (_planetaryRenderer) _planetaryRenderer.setRenderAxes(e.target.checked);
        };
    }
}

/**
 * step — Advance the planetary simulation by one tick (used by Step button).
 */
export function step() {
    if (_planetaryBridge) {
        _planetaryBridge.run(1);
        if (_planetaryRenderer) {
            const currentData = _planetaryBridge.getPlanetaryData();
            _planetaryRenderer.update(currentData);
        }
    }
}

/**
 * dispose — Clean up planetary renderer, bridge, and interval when leaving Scale 4.
 */
export function dispose(ctx) {
    if (window._planetaryInterval) {
        clearInterval(window._planetaryInterval);
        window._planetaryInterval = null;
    }
    if (_planetaryRenderer) {
        _planetaryRenderer.dispose();
        _planetaryRenderer = null;
    }
    _planetaryBridge = null;
    // Restore lattice particles visibility for other scales
    if (ctx && ctx.viewport && ctx.viewport.particles) {
        ctx.viewport.particles.visible = true;
    }
}

