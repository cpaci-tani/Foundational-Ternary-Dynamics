/**
 * Scale 5 — Cosmic Controller
 *
 * Manages the cosmic scale: N-body gravitational simulation with
 * Hubble expansion, dark matter, and cosmological diagnostics.
 *
 * The CosmicMockBridge provides a JS-only N-body engine with FTD-internal
 * constants (G_N [IMPOSED]; Ω_Λ = 2/3 is an engine [CONJECTURE], NOT a
 * derived dark-energy density — it does not match the observed Ω_Λ ≈ 0.685).
 * The CosmicRenderer visualizes
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
import { createStatusBarCache, hideScale0Overlays, createTickAccumulator, saveScaleCameraState, restoreScaleCameraState } from '../scale-utils.js';
import { telemetryHub } from '../../telemetry-hub.js';
import { syncScale5Toggles } from './ui/toolbar/component.js';

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Max scenario-telemetry pairs to show in the compact toolbar strip. The
// bridge emits at most two keys per scenario today; the cap guards the
// always-visible strip against an unexpectedly large map.
const COSMIC_TELEMETRY_MAX_PAIRS = 3;

/**
 * Flatten the bridge's per-scenario telemetry map into one compact toolbar
 * string, e.g. {'Core Separation': '12.3 lu', 'Status': 'Approach'} →
 * "Core Separation: 12.3 lu · Status: Approach". Returns '' for an empty or
 * missing map so the strip entry simply stays blank between scenarios.
 *
 * @param {Object<string,string|number>|null|undefined} tel
 * @returns {string}
 */
function formatCosmicTelemetry(tel) {
    if (!tel) return '';
    const keys = Object.keys(tel);
    if (keys.length === 0) return '';
    return keys
        .slice(0, COSMIC_TELEMETRY_MAX_PAIRS)
        .map((k) => k + ': ' + tel[k])
        .join(' · ');
}

/**
 * Draw one bar-chart row for the gas-lab profile card (Task 5). `mode`
 * 'nonneg' draws bars up from the bottom (density, always >= 0); 'signed'
 * draws bars from a mid-line, positive up / negative down (radial or x
 * velocity, which can be either sign). Normalizes to the series' own max
 * absolute value each call — a display convenience, not a physical scale.
 *
 * @param {HTMLCanvasElement|null} canvas
 * @param {Float64Array|number[]|undefined} values
 * @param {string} color
 * @param {'nonneg'|'signed'} mode
 */
function _drawGasProfileBars(canvas, values, color, mode) {
    if (!canvas) return;
    const ctx2d = canvas.getContext('2d');
    if (!ctx2d) return;
    const w = canvas.width, h = canvas.height;
    ctx2d.clearRect(0, 0, w, h);
    if (!values || values.length === 0) return;

    let maxAbs = 0;
    for (let i = 0; i < values.length; i++) {
        const a = Math.abs(values[i]);
        if (a > maxAbs) maxAbs = a;
    }
    if (!(maxAbs > 0)) return;

    const n = values.length;
    const barW = w / n;
    ctx2d.fillStyle = color;

    if (mode === 'signed') {
        const midY = h / 2;
        ctx2d.strokeStyle = '#3a4a6a';
        ctx2d.lineWidth = 1;
        ctx2d.beginPath();
        ctx2d.moveTo(0, midY);
        ctx2d.lineTo(w, midY);
        ctx2d.stroke();
        for (let i = 0; i < n; i++) {
            const barH = (values[i] / maxAbs) * (h / 2 - 2);
            const y = barH >= 0 ? midY - barH : midY;
            ctx2d.fillRect(i * barW, y, Math.max(1, barW - 1), Math.abs(barH));
        }
    } else {
        for (let i = 0; i < n; i++) {
            const barH = (values[i] / maxAbs) * (h - 4);
            ctx2d.fillRect(i * barW, h - barH, Math.max(1, barW - 1), barH);
        }
    }
}

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------

const _tickAcc = createTickAccumulator();

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
        _tickAcc.reset();
        ctx._resetAllVisualState();
        ctx.running = false;
        ctx.updatePlayButton();

        const viewport = ctx.viewport;

        // Hide all non-cosmic visuals
        hideScale0Overlays(viewport);

        // Create cosmic bridge (JS-only mock for now)
        this.bridge = new CosmicMockBridge();
        this.bridge.setupScenario(scenarioName);

        // Reflect this bridge's toggle state (e.g. a gas laboratory setting
        // `this._toggles.sph_monaghan = true` in its scenario setup) onto
        // the toolbar checkbox, and make the checkbox drive THIS bridge
        // going forward (Task 4, sph_monaghan; see toolbar/component.js).
        syncScale5Toggles(this.bridge);

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

        // Hand the cosmic bridge + renderer to the inspector so body-click
        // inspection works (audit §E item (a), 2026-05-31). The inspector's
        // cosmic raycast path (inspector.js) is gated on `_cosmicRenderer`
        // being set via setCosmicContext(); without this call the producer
        // `bridge.cosmicInspectBody()` and the whole cosmic-inspector panel
        // (already built in inspector/scales/cosmic.js + panel template) were
        // unreachable — clicks never resolved to a body. syncMode('cosmic')
        // already sets _engineMode='cosmic'; this supplies getInteractables().
        if (ctx.inspector?.setCosmicContext) {
            ctx.inspector.setCosmicContext(this.bridge, this.renderer);
        }

        // Save prior camera state for destroy() to restore (audit P1-8, 2026-05-27)
        saveScaleCameraState(this, viewport);

        // Configure camera for cosmic scale
        viewport.camera.near = 0.1;
        viewport.camera.far = 50000;
        viewport.camera.updateProjectionMatrix();
        viewport.controls.minDistance = 5;
        viewport.controls.maxDistance = 100000000;

        // Initial render
        const data = this.bridge.getCosmicData();
        this.renderer.update(data, this.bridge.getDiagnostics());

        // Set camera preset based on scenario. Note: the 'quasar' camera
        // preset was orphaned — neither the scenario `<select>` nor the
        // camera `<select>` ever offered it, and this map never dispatched
        // to it. Its dead entry in CosmicRenderer.setCameraPreset() was
        // removed 2026-05-31 (audit §E item (c)). Binary AGN
        // ('cosmic-binary-agn') covers the quasar use case visually.
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
        syncScale5Toggles(null); // stop the toolbar checkbox driving a destroyed bridge
        // Restore lattice particles visibility for other scales
        if (ctx && ctx.viewport && ctx.viewport.particles) {
            ctx.viewport.particles.visible = true;
        }
        // Restore camera/controls (audit P1-8 fix, 2026-05-27)
        if (ctx && ctx.viewport) {
            restoreScaleCameraState(this, ctx.viewport);
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
            const wholeTicks = _tickAcc.accumulate(ctx.ticksPerFrame);
            if (wholeTicks > 0) {
                bridge.run(wholeTicks);
            }
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
        // Scenario-specific telemetry (audit §E item (b), 2026-05-31). The
        // bridge computes per-scenario readouts each tick (_updateTelemetry →
        // _customTelemetry, e.g. "Core Separation", "BH Mass", "Status") and
        // returns them as diag.customTelemetry, but nothing displayed them.
        // Flatten the {key: value} map into one compact strip entry; the
        // status cache skips the DOM write when the string is unchanged.
        _toolbarStatus.update('cosmic-tb-scenario', formatCosmicTelemetry(diag.customTelemetry));

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

        // Gas laboratory axis-profile card (Task 5; [IMPOSED effective gas
        // dynamics] — see the card title). Only the three gas labs
        // populate diag.customProfiles; every other scenario leaves it
        // null and the card stays hidden.
        const profile = diag.customProfiles;
        const profileCard = document.getElementById('cosmic-gas-profile-card');
        if (profile) {
            if (profileCard) profileCard.hidden = false;
            _panelStatus.update('cosmic-profile-axis', profile.axis);
            _panelStatus.update('cosmic-thermal', profile.thermal.toExponential(3));
            _panelStatus.update('cosmic-kinetic', profile.kinetic.toExponential(3));
            _drawGasProfileBars(document.getElementById('cosmic-profile-density'), profile.density, '#4ade80', 'nonneg');
            _drawGasProfileBars(document.getElementById('cosmic-profile-velocity'), profile.velocity, '#42a5f5', 'signed');
        } else if (profileCard) {
            profileCard.hidden = true;
        }
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
