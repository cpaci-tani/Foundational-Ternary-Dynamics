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
 * Physics preserved exactly from app_dag.js inline code:
 *   - N-body ticks per frame (adjustable via ctx.ticksPerFrame)
 *   - Hubble parameter H(t), scale factor a(t)
 *   - Omega_matter, Omega_Lambda density fractions
 *   - Camera presets per scenario (galaxy, overview, blackhole, merger, quasar)
 *   - Independent 30fps interval loop (avoids module caching issues)
 *   - Compact toolbar telemetry + controls panel cards
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
 * Per-frame cosmic animation. The setInterval in loadCosmicScenario handles
 * physics ticks and telemetry updates; this rAF callback just re-renders
 * the viewport so the camera controls stay responsive.
 *
 * @param {object} ctx - Shared context from the main app:
 *   { viewport, running }
 */
export function animateCosmic(ctx) {
    const { viewport } = ctx;

    if (!_cosmicBridge || !_cosmicRenderer) {
        // Fallback: still render viewport
        viewport.render();
        return;
    }

    // Update renderer with current state (ticks run in the setInterval)
    const data = _cosmicBridge.getCosmicData();
    const diag = _cosmicBridge.getDiagnostics();
    _cosmicRenderer.update(data, diag);

    // Toolbar telemetry is updated by the setInterval in loadCosmicScenario

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
 *   { viewport, running, ticksPerFrame, engineMode,
 *     _resetAllVisualState, updatePlayButton }
 * @param {string} scenarioName - Scenario key (default: 'cosmic-galaxy')
 */
export function loadCosmicScenario(ctx, scenarioName = 'cosmic-galaxy') {
    ctx._resetAllVisualState();
    ctx.running = false;
    ctx.updatePlayButton();

    const viewport = ctx.viewport;

    // Hide all non-cosmic visuals
    if (viewport) {
        viewport.toggleFluxVolume(false);
        viewport.toggleFluxSlice(false);
        viewport.toggleGrid(false);
        viewport.toggleAxes(false);
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
        'cosmic-super-cluster': 'overview',
        'cosmic-cluster': 'overview',
        'cosmic-web': 'overview',
        'cosmic-black-hole': 'blackhole',
        'cosmic-merger': 'merger',
        'cosmic-quasar': 'quasar',
        'cosmic-stellar-lifecycle': 'overview',
        'cosmic-ftd-collapse': 'overview'
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
        if (ctx.running) _cosmicBridge.run(Math.max(1, Math.round(ctx.ticksPerFrame)));
        const data = _cosmicBridge.getCosmicData();
        const diag = _cosmicBridge.getDiagnostics();
        _cosmicRenderer.update(data, diag);
        // Compact toolbar telemetry
        const _tb = (id, v) => { const e = document.getElementById(id); if (e) e.textContent = v; };
        _tb('cosmic-tb-bodies', diag.bodyCount + ' bodies');
        _tb('cosmic-tb-tick', 'T ' + diag.tick);
        _tb('cosmic-tb-hubble', 'H=' + diag.hubbleParameter.toFixed(4));
        // Controls panel cards
        const c = diag.countsByType || [];
        const _set = (id, v) => { const e = document.getElementById(id); if (e) e.textContent = v; };
        _set('cosmic-n-bodies', diag.bodyCount);
        _set('cosmic-tick', diag.tick);
        _set('cosmic-hubble', diag.hubbleParameter.toFixed(5));
        _set('cosmic-scale-factor', diag.scaleFactor.toFixed(5));
        _set('cosmic-n-dm', c[3] || 0);
        _set('cosmic-n-gas', c[4] || 0);
        _set('cosmic-n-stars', c[5] || 0);
        _set('cosmic-n-bh', c[2] || 0);
        _set('cosmic-ke', diag.totalKE.toExponential(2));
        viewport.render();
    }, 33); // 33ms = ~30fps; separate from rAF to decouple physics from render rate
}

// ---------------------------------------------------------------------------
// step  -- single-tick step (called from Step button)
// ---------------------------------------------------------------------------

/**
 * Advance the cosmic simulation by one tick and re-render.
 * Called when the user clicks the Step button while in cosmic mode.
 *
 * @param {object} ctx - Shared context: { viewport }
 */
export function step(ctx) {
    if (_cosmicBridge) {
        _cosmicBridge.run(1);
        if (_cosmicRenderer) {
            const data = _cosmicBridge.getCosmicData();
            _cosmicRenderer.update(data, _cosmicBridge.getDiagnostics());
            ctx.viewport.render();
        }
    }
}

// ---------------------------------------------------------------------------
// setCameraPreset  -- change camera view (called from camera selector)
// ---------------------------------------------------------------------------

/**
 * Set the camera preset for the cosmic renderer.
 *
 * @param {string} preset - Camera preset name
 */
export function setCameraPreset(preset) {
    if (_cosmicRenderer && _cosmicBridge) {
        const data = _cosmicBridge.getCosmicData();
        _cosmicRenderer.setCameraPreset(preset, data);
    }
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
    // Restore lattice particles visibility for other scales
    if (ctx && ctx.viewport && ctx.viewport.particles) {
        ctx.viewport.particles.visible = true;
    }
}
