/**
 * Scale 5 — Cosmic Controller
 *
 * Manages the cosmic scale: N-body gravitational simulation with
 * Hubble expansion, dark matter, and cosmological diagnostics.
 *
 * The CosmicMockBridge provides a JS-only N-body engine with FTD-derived
 * constants (G_N, dark energy fraction). The CosmicRenderer visualizes
 * bodies as point clouds with type-coded colors (stars, gas, dark matter,
 * black holes).
 *
 * Physics preserved exactly from app.js:
 *   - N-body ticks per frame (adjustable, default 5 in rAF / 3 in interval)
 *   - Hubble parameter H(t), scale factor a(t)
 *   - Omega_matter, Omega_Lambda density fractions
 *   - Camera presets per scenario (galaxy, overview, blackhole, merger, quasar)
 *   - Independent 30fps interval loop (avoids module caching issues)
 */

import { CosmicRenderer } from '../../cosmic-renderer.js';
import { CosmicMockBridge } from '../../wasm-bridge.js?v=20260318a';

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------

let _cosmicRenderer = null;    // CosmicRenderer instance (Three.js visuals)
let _cosmicBridge = null;      // CosmicMockBridge instance (N-body engine)

// ---------------------------------------------------------------------------
// animateCosmic  -- per-frame update (called from the main rAF loop)
// ---------------------------------------------------------------------------

/**
 * Per-frame cosmic animation. Runs N-body ticks, updates the renderer
 * with current body positions and cosmological diagnostics, then renders.
 *
 * @param {object} ctx - Shared context from the main app:
 *   { bridge, viewport, running, ticksPerFrame }
 */
export function animateCosmic(ctx) {
    const { viewport, running } = ctx;

    if (!_cosmicBridge || !_cosmicRenderer) {
        // Fallback: still render viewport
        viewport.render();
        return;
    }

    if (running) {
        // Run N-body ticks per frame (adjustable)
        const ticksPerFrame = 5;
        _cosmicBridge.run(ticksPerFrame);
    }

    // Update renderer with current state
    const data = _cosmicBridge.getCosmicData();
    const diag = _cosmicBridge.getDiagnostics();
    _cosmicRenderer.update(data, diag);

    // Update diagnostics display
    const cosmicDiagEl = document.getElementById('cosmic-diagnostics');
    if (cosmicDiagEl) {
        cosmicDiagEl.innerHTML = `
            <div>Tick: ${diag.tick}  Bodies: ${diag.bodyCount}</div>
            <div>H(t): ${diag.hubbleParameter.toFixed(4)}  a(t): ${diag.scaleFactor.toFixed(4)}</div>
            <div>&Omega;<sub>m</sub>: ${diag.omegaMatter.toFixed(3)}  &Omega;<sub>&Lambda;</sub>: ${diag.omegaLambda.toFixed(3)}</div>
            <div>Total Mass: ${diag.totalMass.toExponential(2)}  KE: ${diag.totalKE.toExponential(2)}</div>
        `;
    }

    // Render using standard viewport (post-processing added later)
    viewport.render();
}

// ---------------------------------------------------------------------------
// loadCosmicScenario  -- set up a named cosmic scenario
// ---------------------------------------------------------------------------

/**
 * Initialize and load a cosmic scenario by name. Creates the CosmicMockBridge
 * and CosmicRenderer, configures camera for cosmic scale, sets camera preset,
 * and starts the independent 30fps interval loop.
 *
 * @param {object} ctx - Shared context:
 *   { bridge, viewport, running, updatePlayButton, _resetAllVisualState, engineMode }
 * @param {string} scenarioName - Scenario key (default: 'cosmic-galaxy')
 */
export function loadCosmicScenario(ctx, scenarioName = 'cosmic-galaxy') {
    ctx._resetAllVisualState();
    ctx.running = false;
    ctx.updatePlayButton();

    const viewport = ctx.viewport;

    // Hide all Scale 0 visuals
    if (viewport) {
        viewport.toggleFluxVolume(false);
        viewport.toggleFluxSlice(false);
        viewport.toggleGrid(false);
        if (viewport.particles) viewport.particles.visible = false;
    }

    // Create cosmic bridge (JS-only mock for now)
    _cosmicBridge = new CosmicMockBridge();
    _cosmicBridge.setupScenario(scenarioName);

    // Create cosmic renderer
    if (_cosmicRenderer) _cosmicRenderer.dispose();
    _cosmicRenderer = new CosmicRenderer(viewport.scene, viewport.camera, viewport.renderer);

    // Configure camera for cosmic scale
    viewport.camera.near = 0.1;
    viewport.camera.far = 50000;
    viewport.camera.updateProjectionMatrix();
    viewport.controls.minDistance = 5;
    viewport.controls.maxDistance = 5000;

    // Initial render
    const data = _cosmicBridge.getCosmicData();
    _cosmicRenderer.update(data, _cosmicBridge.getDiagnostics());

    // Set camera preset based on scenario
    const presetMap = {
        'cosmic-galaxy': 'galaxy',
        'cosmic-cluster': 'overview',
        'cosmic-web': 'overview',
        'cosmic-black-hole': 'blackhole',
        'cosmic-merger': 'merger',
        'cosmic-quasar': 'quasar'
    };
    _cosmicRenderer.setCameraPreset(presetMap[scenarioName] || 'overview', data);

    // Auto-play
    ctx.running = true;
    ctx.updatePlayButton();

    // Cosmic frame loop (independent of rAF to avoid module caching issues)
    if (window._cosmicInterval) clearInterval(window._cosmicInterval);
    window._cosmicInterval = setInterval(() => {
        if (ctx.engineMode !== 'cosmic' || !_cosmicBridge || !_cosmicRenderer) {
            clearInterval(window._cosmicInterval);
            window._cosmicInterval = null;
            return;
        }
        if (ctx.running) _cosmicBridge.run(3);
        const data = _cosmicBridge.getCosmicData();
        const diag = _cosmicBridge.getDiagnostics();
        _cosmicRenderer.update(data, diag);
        // Toolbar diagnostics
        const el = document.getElementById('cosmic-diagnostics');
        if (el) {
            el.innerHTML = `<div>Tick: ${diag.tick}  Bodies: ${diag.bodyCount}</div>`
                + `<div>H(t): ${diag.hubbleParameter.toFixed(4)}  a(t): ${diag.scaleFactor.toFixed(4)}</div>`
                + `<div>\u03A9<sub>m</sub>: ${diag.omegaMatter.toFixed(3)}  \u03A9<sub>\u039B</sub>: ${diag.omegaLambda.toFixed(3)}</div>`
                + `<div>Mass: ${diag.totalMass.toExponential(2)}  KE: ${diag.totalKE.toExponential(2)}</div>`;
        }
        // Panel diagnostics
        const pd = document.getElementById('cosmic-panel-diagnostics');
        if (pd) {
            const c = diag.countsByType || [];
            pd.innerHTML = `<div>Tick: ${diag.tick} | Bodies: ${diag.bodyCount}</div>`
                + `<div>DM: ${c[3]||0} | Gas: ${c[4]||0} | Stars: ${c[5]||0} | BH: ${c[2]||0}</div>`
                + `<div>H(t) = ${diag.hubbleParameter.toFixed(5)} | a(t) = ${diag.scaleFactor.toFixed(5)}</div>`
                + `<div>\u03A9<sub>m</sub> = ${diag.omegaMatter.toFixed(4)} | \u03A9<sub>\u039B</sub> = ${diag.omegaLambda.toFixed(4)}</div>`
                + `<div>Total mass: ${diag.totalMass.toExponential(3)}</div>`
                + `<div>Kinetic energy: ${diag.totalKE.toExponential(3)}</div>`;
        }
        viewport.render();
    }, 33); // ~30fps
}

// ---------------------------------------------------------------------------
// resetScale5  -- tear down cosmic state on scale switch
// ---------------------------------------------------------------------------

/**
 * Clean up Scale 5 state when leaving cosmic mode.
 * Stops the interval loop, disposes the renderer, and clears the bridge.
 *
 * @param {object} ctx - Shared context (unused for now, reserved for future cleanup)
 */
export function resetScale5(ctx) {
    if (window._cosmicInterval) {
        clearInterval(window._cosmicInterval);
        window._cosmicInterval = null;
    }
    if (_cosmicRenderer) {
        _cosmicRenderer.dispose();
        _cosmicRenderer = null;
    }
    _cosmicBridge = null;
}
