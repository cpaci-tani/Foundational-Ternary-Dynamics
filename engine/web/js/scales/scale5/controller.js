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
 *   - Physics ticks + telemetry updates at ~30 Hz (every other rAF frame),
 *     rendering at ~60 Hz so OrbitControls stay smooth
 *   - Compact toolbar telemetry + controls panel cards
 */

import { CosmicRenderer } from '../../cosmic-renderer.js';
import { CosmicMockBridge } from '../../bridge/mock-scale5.js';
import { createStatusBarCache } from '../scale-utils.js';

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------

let _cosmicRenderer = null;    // CosmicRenderer instance (Three.js visuals)
let _cosmicBridge = null;      // CosmicMockBridge instance (N-body engine)
const _toolbarStatus = createStatusBarCache();
const _panelStatus = createStatusBarCache();

// ---------------------------------------------------------------------------
// animateCosmic  -- per-frame update (called from the main rAF loop)
// ---------------------------------------------------------------------------

/**
 * Per-frame cosmic animation. Drives physics ticks, renderer state updates,
 * and toolbar/panel telemetry from the main rAF loop. Physics and telemetry
 * update every other rAF frame (~30 Hz), matching the cadence of the
 * pre-B.1 setInterval; the viewport is rendered every rAF frame (~60 Hz)
 * so OrbitControls stay smooth.
 *
 * @param {object} ctx - Shared context from the main app:
 *   { viewport, running, ticksPerFrame, frameCount }
 */
export function animateCosmic(ctx) {
    const { viewport } = ctx;

    if (!_cosmicBridge || !_cosmicRenderer) {
        // Fallback: still render viewport so the scene isn't frozen
        viewport.render();
        return;
    }

    // Match the pre-B.1 30 Hz physics cadence by ticking on every other
    // rAF frame. Rendering still runs at full rAF rate below.
    const isPhysicsFrame = (ctx.frameCount & 1) === 0;

    if (isPhysicsFrame) {
        if (ctx.running) {
            _cosmicBridge.run(Math.max(1, Math.round(ctx.ticksPerFrame)));
        }

        const data = _cosmicBridge.getCosmicData();
        const diag = _cosmicBridge.getDiagnostics();
        _cosmicRenderer.update(data, diag);

        // Compact toolbar telemetry
        _toolbarStatus.update('cosmic-tb-bodies', diag.bodyCount + ' bodies');
        _toolbarStatus.update('cosmic-tb-tick', 'T ' + diag.tick);
        _toolbarStatus.update('cosmic-tb-hubble', 'H=' + diag.hubbleParameter.toFixed(4));

        // Controls panel cards
        const c = diag.countsByType || [];
        _panelStatus.update('cosmic-n-bodies', String(diag.bodyCount));
        _panelStatus.update('cosmic-tick', String(diag.tick));
        _panelStatus.update('cosmic-hubble', diag.hubbleParameter.toFixed(5));
        _panelStatus.update('cosmic-scale-factor', diag.scaleFactor.toFixed(5));
        _panelStatus.update('cosmic-n-dm', String(c[3] || 0));
        _panelStatus.update('cosmic-n-gas', String(c[4] || 0));
        _panelStatus.update('cosmic-n-stars', String(c[5] || 0));
        _panelStatus.update('cosmic-n-bh', String(c[2] || 0));
        _panelStatus.update('cosmic-ke', diag.totalKE.toExponential(2));
    }

    // Render every rAF frame so OrbitControls stay responsive
    viewport.render();
}

// ---------------------------------------------------------------------------
// loadCosmicScenario  -- set up a named cosmic scenario
// ---------------------------------------------------------------------------

/**
 * Initialize and load a cosmic scenario by name. Creates the CosmicMockBridge
 * and CosmicRenderer, configures camera for cosmic scale, and sets the
 * camera preset. Physics ticks, telemetry, and rendering are driven by
 * animateCosmic() on the main rAF loop.
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

    // Auto-play; animateCosmic() will advance physics from the main rAF loop.
    ctx.running = true;
    ctx.updatePlayButton();
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
 * Disposes the renderer and clears the bridge. Physics/render is driven
 * by animateCosmic() on the main rAF loop, which becomes a no-op once
 * _cosmicBridge is nulled out, so no interval cleanup is needed.
 *
 * @param {object} ctx - Shared context (used only to restore viewport state)
 */
export function resetScale5(ctx) {
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
