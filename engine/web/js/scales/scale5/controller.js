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

import { BaseLifecycleController } from '../../lifecycle.js';
import { CosmicRenderer } from '../../cosmic-renderer.js';
import { CosmicMockBridge } from '../../bridge/mock-scale5.js';
import { createStatusBarCache } from '../scale-utils.js';

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------

class Scale5LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
        this.bridge = null;
        this.renderer = null;
    }

    mount(ctx) {
        // Standard setup placeholder
    }

    loadCosmicScenario(ctx, scenarioName = 'cosmic-galaxy') {
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
        this.bridge = new CosmicMockBridge();
        this.bridge.setupScenario(scenarioName);

        // Create cosmic renderer
        if (this.renderer) {
            this.renderer.dispose();
        }
        this.renderer = new CosmicRenderer(viewport.scene, viewport.camera, viewport.renderer);
        this.trackThreeObject(this.renderer);

        // Configure camera for cosmic scale
        viewport.camera.near = 0.1;
        viewport.camera.far = 50000;
        viewport.camera.updateProjectionMatrix();
        viewport.controls.minDistance = 5;
        viewport.controls.maxDistance = 5000;

        // Initial render
        const data = this.bridge.getCosmicData();
        this.renderer.update(data, this.bridge.getDiagnostics());

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
        this.renderer.setCameraPreset(presetMap[scenarioName] || 'overview', data);

        // Auto-play
        ctx.running = true;
        ctx.updatePlayButton();
    }

    step(ctx) {
        if (this.bridge) {
            this.bridge.run(1);
            if (this.renderer) {
                const data = this.bridge.getCosmicData();
                this.renderer.update(data, this.bridge.getDiagnostics());
                ctx.viewport.render();
            }
        }
    }

    setCameraPreset(preset) {
        if (this.renderer && this.bridge) {
            const data = this.bridge.getCosmicData();
            this.renderer.setCameraPreset(preset, data);
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

const _lifecycleController = new Scale5LifecycleController();

const _toolbarStatus = createStatusBarCache();
const _panelStatus = createStatusBarCache();

export function mount(ctx) {
    _lifecycleController.mount(ctx);
}

export function destroy(ctx) {
    _lifecycleController.destroy(ctx);
}

export function animateCosmic(ctx) {
    const { viewport } = ctx;
    const bridge = _lifecycleController.bridge;
    const renderer = _lifecycleController.renderer;

    if (!bridge || !renderer) {
        // Fallback: still render viewport so the scene isn't frozen
        viewport.render();
        return;
    }

    // Match the pre-B.1 30 Hz physics cadence by ticking on every other
    // rAF frame. Rendering still runs at full rAF rate below.
    const isPhysicsFrame = (ctx.frameCount & 1) === 0;

    if (isPhysicsFrame) {
        if (ctx.running) {
            bridge.run(Math.max(1, Math.round(ctx.ticksPerFrame)));
        }

        const data = bridge.getCosmicData();
        const diag = bridge.getDiagnostics();
        renderer.update(data, diag);

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

export function loadCosmicScenario(ctx, scenarioName = 'cosmic-galaxy') {
    _lifecycleController.loadCosmicScenario(ctx, scenarioName);
}

export function step(ctx) {
    _lifecycleController.step(ctx);
}

export function setCameraPreset(preset) {
    _lifecycleController.setCameraPreset(preset);
}

export function resetScale5(ctx) {
    _lifecycleController.destroy(ctx);
}
