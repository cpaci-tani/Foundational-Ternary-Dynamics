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
 * Physics preserved exactly from app.js inline code:
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
import { telemetryHub } from '../../telemetry-hub.js';

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

        // Inform the inspector about the cosmic bridge so it can route
        // queries to the right backend (audit P1-1 fix, 2026-05-27).
        // Without this, the inspector keeps a stale reference to the
        // global lattice bridge across all Scale 5 inspection.
        if (ctx.inspectorRuntime?.setBridge) {
            ctx.inspectorRuntime.setBridge(this.bridge);
        } else if (ctx.inspector?.setBridge) {
            ctx.inspector.setBridge(this.bridge);
        }

        // Create cosmic renderer
        if (this.renderer) {
            this.renderer.dispose();
        }
        this.renderer = new CosmicRenderer(viewport.scene, viewport.camera, viewport.renderer);
        this.trackThreeObject(this.renderer);

        // Save prior camera state for destroy() to restore (audit P1-8, 2026-05-27)
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

        // Configure camera for cosmic scale
        viewport.camera.near = 0.1;
        viewport.camera.far = 50000;
        viewport.camera.updateProjectionMatrix();
        viewport.controls.minDistance = 5;
        viewport.controls.maxDistance = 5000;

        // Initial render
        const data = this.bridge.getCosmicData();
        this.renderer.update(data, this.bridge.getDiagnostics());

        // Set camera preset based on scenario. Note: 'cosmic-quasar' /
        // 'quasar' preset was removed 2026-05-27 (audit W10) — the
        // toolbar `<select>` never offered a quasar scenario, and the
        // camera selector has no quasar option either, so the entry
        // was unreachable. Binary AGN ('cosmic-binary-agn') exists and
        // covers the quasar use case visually.
        const presetMap = {
            'cosmic-galaxy': 'galaxy',
            'cosmic-super-cluster': 'overview',
            'cosmic-cluster': 'overview',
            'cosmic-web': 'overview',
            'cosmic-black-hole': 'blackhole',
            'cosmic-merger': 'merger',
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
        const diag = telemetryHub.collectScale5(bridge) || bridge.getDiagnostics();
        renderer.update(data, diag);

        // Compact toolbar telemetry. Hubble parameter is now the LIVE
        // ΛCDM rate H(a) integrated each tick by the Friedmann solver
        // (audit P0-9 implemented 2026-05-27) — H decreases as a(t) grows.
        _toolbarStatus.update('cosmic-tb-bodies', diag.bodyCount + ' bodies');
        _toolbarStatus.update('cosmic-tb-tick', 'T ' + diag.tick);
        _toolbarStatus.update('cosmic-tb-hubble', 'H=' + diag.hubbleParameter.toFixed(4));

        // Controls panel cards
        const c = diag.countsByType || [];
        _panelStatus.update('cosmic-n-bodies', String(diag.bodyCount));
        _panelStatus.update('cosmic-tick', String(diag.tick));
        _panelStatus.update('cosmic-hubble', diag.hubbleParameter.toFixed(5));
        _panelStatus.update('cosmic-scale-factor', diag.scaleFactor.toFixed(4));
        // Redshift z = 1/a − 1, live from the Friedmann solver (audit P0-9).
        if (diag.redshift != null) {
            _panelStatus.update('cosmic-redshift', diag.redshift.toFixed(3));
        }
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
