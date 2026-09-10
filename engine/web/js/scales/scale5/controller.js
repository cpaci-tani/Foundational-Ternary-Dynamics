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
import { syncScale5Toggles } from './ui/toggle-sync.js';
import { Scale5ControlsComponent } from './ui/controls/component.js';
import { syncScale5Overlays } from './ui/overlays/component.js';
import { isPanelLive } from '../../ui/panels/panel-visibility.js';

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
 * Pass B: an optional 5th `edges` argument (the bin-edge array
 * `_computeGasProfile()` already returns and this function previously
 * ignored) reserves a 16px bottom margin and prints the first/last bin
 * edge there, so the two blind bar charts gain an axis scale. `edges` is
 * optional so a caller with no profile data yet can still clear the canvas.
 *
 * @param {HTMLCanvasElement|null} canvas
 * @param {Float64Array|number[]|undefined} values
 * @param {string} color
 * @param {'nonneg'|'signed'} mode
 * @param {Float64Array|number[]|undefined} [edges]
 */
function _drawGasProfileBars(canvas, values, color, mode, edges) {
    if (!canvas) return;
    const ctx2d = canvas.getContext('2d');
    if (!ctx2d) return;
    const w = canvas.width, h = canvas.height;
    ctx2d.clearRect(0, 0, w, h);
    if (!values || values.length === 0) return;

    // 16px font floor (project styling rule) reserves the same 16px for the
    // label row, so the axis text is never sub-floor and never overlaps bars.
    const labelH = 16;
    const plotH = h - labelH;

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
        const midY = plotH / 2;
        ctx2d.strokeStyle = '#3a4a6a';
        ctx2d.lineWidth = 1;
        ctx2d.beginPath();
        ctx2d.moveTo(0, midY);
        ctx2d.lineTo(w, midY);
        ctx2d.stroke();
        for (let i = 0; i < n; i++) {
            const barH = (values[i] / maxAbs) * (plotH / 2 - 2);
            const y = barH >= 0 ? midY - barH : midY;
            ctx2d.fillRect(i * barW, y, Math.max(1, barW - 1), Math.abs(barH));
        }
    } else {
        for (let i = 0; i < n; i++) {
            const barH = (values[i] / maxAbs) * (plotH - 4);
            ctx2d.fillRect(i * barW, plotH - barH, Math.max(1, barW - 1), barH);
        }
    }

    if (edges && edges.length >= 2) {
        ctx2d.fillStyle = '#8a9bbf';
        ctx2d.font = '16px sans-serif';
        ctx2d.textBaseline = 'bottom';
        ctx2d.textAlign = 'left';
        ctx2d.fillText(edges[0].toFixed(1), 1, h);
        ctx2d.textAlign = 'right';
        ctx2d.fillText(edges[edges.length - 1].toFixed(1), w - 1, h);
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
        // Called by app.js's generic scale-switch dispatcher (`nextController
        // .mount(ctx)`) whenever a switch INTO Scale 5 happens. The controls
        // card must also be (re-)mounted here, not only from
        // loadCosmicScenario() below, because a future caller could reach
        // mount() without immediately loading a scenario.
        this._mountControls(ctx);
    }

    /** Mount the Scale 5 controls card (Pass 0b) and bind its one
     *  interactive control. Called from BOTH mount() and the top of
     *  loadCosmicScenario() (the latter is reachable without the former —
     *  every scenario switch calls it directly), so both the card creation
     *  (Scale5ControlsComponent.init(), reconciled by a stable
     *  data-scale5-control-card key) and this method itself must be
     *  idempotent on repeated calls. */
    _mountControls(ctx) {
        const controlsPanel = document.getElementById('panel-controls');
        if (controlsPanel) new Scale5ControlsComponent(controlsPanel).init();

        // The "Scenario defaults" button lives in DOM that persists across
        // scale switches (hidden via .scale5-only CSS, like Scale 4's
        // toolbar toggles) — bindEvent()/destroy() below unbind it whenever
        // this controller tears down, so a re-entry must re-bind. Guard with
        // a dataset flag exactly as scale4/controller.js:710-728 does for
        // its persistent toolbar checkboxes.
        const resetBtn = document.getElementById('cosmic-ctrl-reset-toggles');
        if (resetBtn && !resetBtn.dataset.s5CtrlBound) {
            this.bindEvent(resetBtn, 'click', () => {
                if (!this.bridge) return;
                this.bridge._syncRuleTogglesFromScenario();
                syncScale5Toggles(this.bridge);
            });
            resetBtn.dataset.s5CtrlBound = '1';
        }

        // Gas card (Pass B): the SPH and legacy-repulsion checkboxes are
        // bound by Scale5ControlsComponent.init() itself, through the
        // shared toggle-sync module (they need no `this.bridge` closure —
        // see ui/toggle-sync.js). Only the two viscosity sliders and the
        // adaptive-smoothing checkbox are bound here, since they have no
        // second UI surface and no SCALE5_TOGGLES registry entry (they are
        // bridge fields/live setters, not rule toggles — Ruling P3). Lazy
        // `this.bridge` closures, same reasoning as resetBtn above: binding
        // happens once, before a fresh bridge necessarily exists yet.
        const alphaInput = document.getElementById('cosmic-gas-alpha');
        const alphaValue = document.getElementById('cosmic-gas-alpha-value');
        if (alphaInput && !alphaInput.dataset.s5CtrlBound) {
            this.bindEvent(alphaInput, 'input', () => {
                const v = Number(alphaInput.value);
                this.bridge?.setSphAlpha?.(v);
                if (alphaValue) alphaValue.textContent = v.toFixed(1);
            });
            alphaInput.dataset.s5CtrlBound = '1';
        }

        const betaInput = document.getElementById('cosmic-gas-beta');
        const betaValue = document.getElementById('cosmic-gas-beta-value');
        if (betaInput && !betaInput.dataset.s5CtrlBound) {
            this.bindEvent(betaInput, 'input', () => {
                const v = Number(betaInput.value);
                this.bridge?.setSphBeta?.(v);
                if (betaValue) betaValue.textContent = v.toFixed(1);
            });
            betaInput.dataset.s5CtrlBound = '1';
        }

        const adaptiveInput = document.getElementById('cosmic-gas-adaptive-h');
        if (adaptiveInput && !adaptiveInput.dataset.s5CtrlBound) {
            this.bindEvent(adaptiveInput, 'change', () => {
                this.bridge?.setAdaptiveSmoothing?.(adaptiveInput.checked);
            });
            adaptiveInput.dataset.s5CtrlBound = '1';
        }

        // Dynamics card (Pass A): the speed_limit checkbox is bound by
        // Scale5ControlsComponent.init() itself through the shared
        // toggle-sync module (same shape as the Gas card's sph_monaghan/
        // legacy_gas_repulsion checkboxes above). The four sliders below are
        // bridge fields/live setters with no second UI surface and no
        // SCALE5_TOGGLES registry entry, so they are bound directly here —
        // identical reasoning to the Gas card's alpha/beta/adaptive-h
        // sliders just above. Lazy `this.bridge` closures for the same
        // reason: binding happens once, before a fresh bridge necessarily
        // exists yet.
        const gravityInput = document.getElementById('cosmic-dynamics-gravity');
        const gravityValue = document.getElementById('cosmic-dynamics-gravity-value');
        if (gravityInput && !gravityInput.dataset.s5CtrlBound) {
            this.bindEvent(gravityInput, 'input', () => {
                const v = Number(gravityInput.value);
                this.bridge?.setGravityScale?.(v);
                if (gravityValue) gravityValue.textContent = v.toFixed(2);
            });
            gravityInput.dataset.s5CtrlBound = '1';
        }

        const softeningInput = document.getElementById('cosmic-dynamics-softening');
        const softeningValue = document.getElementById('cosmic-dynamics-softening-value');
        if (softeningInput && !softeningInput.dataset.s5CtrlBound) {
            this.bindEvent(softeningInput, 'input', () => {
                const v = Number(softeningInput.value);
                this.bridge?.setSofteningScale?.(v);
                if (softeningValue) softeningValue.textContent = v.toFixed(2);
            });
            softeningInput.dataset.s5CtrlBound = '1';
        }

        const dtInput = document.getElementById('cosmic-dynamics-dt');
        const dtValue = document.getElementById('cosmic-dynamics-dt-value');
        if (dtInput && !dtInput.dataset.s5CtrlBound) {
            this.bindEvent(dtInput, 'input', () => {
                const v = Number(dtInput.value);
                this.bridge?.setDt?.(v);
                if (dtValue) dtValue.textContent = v.toFixed(3);
            });
            dtInput.dataset.s5CtrlBound = '1';
        }

        const speedLimitFactorInput = document.getElementById('cosmic-dynamics-speed-limit-factor');
        const speedLimitFactorValue = document.getElementById('cosmic-dynamics-speed-limit-factor-value');
        if (speedLimitFactorInput && !speedLimitFactorInput.dataset.s5CtrlBound) {
            this.bindEvent(speedLimitFactorInput, 'input', () => {
                const v = Number(speedLimitFactorInput.value);
                this.bridge?.setSpeedLimitFactor?.(v);
                if (speedLimitFactorValue) speedLimitFactorValue.textContent = v.toFixed(2);
            });
            speedLimitFactorInput.dataset.s5CtrlBound = '1';
        }
    }

    /** Reflect the fresh bridge's runtime SPH params (Pass B: alpha, beta,
     *  adaptive smoothing) onto the Gas card. No scenario currently
     *  overrides these — every fresh CosmicMockBridge starts at the
     *  SPH.ALPHA/SPH.BETA/true defaults (getRuntimeParams()) — so this
     *  keeps a slider a user dragged on a prior scenario from silently
     *  misrepresenting the new bridge's actual state. Called from
     *  loadCosmicScenario() alongside syncScale5Toggles(this.bridge). */
    _syncGasControlsFromBridge() {
        if (!this.bridge?.getRuntimeParams) return;
        const params = this.bridge.getRuntimeParams();
        const alphaInput = document.getElementById('cosmic-gas-alpha');
        const alphaValue = document.getElementById('cosmic-gas-alpha-value');
        if (alphaInput) alphaInput.value = String(params.sphAlpha);
        if (alphaValue) alphaValue.textContent = params.sphAlpha.toFixed(1);
        const betaInput = document.getElementById('cosmic-gas-beta');
        const betaValue = document.getElementById('cosmic-gas-beta-value');
        if (betaInput) betaInput.value = String(params.sphBeta);
        if (betaValue) betaValue.textContent = params.sphBeta.toFixed(1);
        const adaptiveInput = document.getElementById('cosmic-gas-adaptive-h');
        if (adaptiveInput) adaptiveInput.checked = !!params.adaptiveSmoothing;
    }

    /** Reflect the fresh bridge's runtime dynamics params (Pass A: gravity
     *  scale, softening scale, dt, speed-limit factor) onto the Dynamics
     *  card, mirroring _syncGasControlsFromBridge above exactly — same
     *  reasoning: no scenario currently overrides these live-setter fields
     *  (getRuntimeParams() always reports their `?? 1`/base defaults on a
     *  fresh bridge), so this keeps a slider dragged on a prior scenario
     *  from silently misrepresenting the new bridge's actual state. Called
     *  from loadCosmicScenario() alongside _syncGasControlsFromBridge(). */
    _syncDynamicsControlsFromBridge() {
        if (!this.bridge?.getRuntimeParams) return;
        const params = this.bridge.getRuntimeParams();
        const gravityInput = document.getElementById('cosmic-dynamics-gravity');
        const gravityValue = document.getElementById('cosmic-dynamics-gravity-value');
        if (gravityInput) gravityInput.value = String(params.gravityScale);
        if (gravityValue) gravityValue.textContent = params.gravityScale.toFixed(2);
        const softeningInput = document.getElementById('cosmic-dynamics-softening');
        const softeningValue = document.getElementById('cosmic-dynamics-softening-value');
        if (softeningInput) softeningInput.value = String(params.softeningScale);
        if (softeningValue) softeningValue.textContent = params.softeningScale.toFixed(2);
        const dtInput = document.getElementById('cosmic-dynamics-dt');
        const dtValue = document.getElementById('cosmic-dynamics-dt-value');
        if (dtInput) dtInput.value = String(params.dt);
        if (dtValue) dtValue.textContent = params.dt.toFixed(3);
        const speedLimitFactorInput = document.getElementById('cosmic-dynamics-speed-limit-factor');
        const speedLimitFactorValue = document.getElementById('cosmic-dynamics-speed-limit-factor-value');
        if (speedLimitFactorInput) speedLimitFactorInput.value = String(params.speedLimitFactor);
        if (speedLimitFactorValue) speedLimitFactorValue.textContent = params.speedLimitFactor.toFixed(2);
    }

    loadCosmicScenario(ctx, scenarioName = 'cosmic-galaxy') {
        this._mountControls(ctx);
        _tickAcc.reset();
        telemetryHub.resetScale(5);
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
        this._syncGasControlsFromBridge();
        this._syncDynamicsControlsFromBridge();

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

        // Rebind the Gas overlay's colour-by/smoothing-circle controls to
        // THIS fresh renderer (Ruling P1): ViewportOverlaysComponent.init()
        // builds the overlay DOM once at boot and never rebuilds it, while
        // the renderer above is recreated on every scenario load — without
        // this call the overlay controls would keep driving a disposed
        // renderer after the first scenario switch.
        syncScale5Overlays(this.renderer);

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
        // super.destroy() removed the "Scenario defaults" button's listener,
        // but the button itself persists in the DOM (hidden via
        // .scale5-only), so clear the bind-guard flag too — otherwise
        // re-entering Scale 5 would see s5CtrlBound and skip re-binding,
        // leaving the button dead (scale4/controller.js:710-728 precedent).
        const resetBtn = document.getElementById('cosmic-ctrl-reset-toggles');
        if (resetBtn) delete resetBtn.dataset.s5CtrlBound;
        const alphaInput = document.getElementById('cosmic-gas-alpha');
        if (alphaInput) delete alphaInput.dataset.s5CtrlBound;
        const betaInput = document.getElementById('cosmic-gas-beta');
        if (betaInput) delete betaInput.dataset.s5CtrlBound;
        const adaptiveInput = document.getElementById('cosmic-gas-adaptive-h');
        if (adaptiveInput) delete adaptiveInput.dataset.s5CtrlBound;
        const gravityInput = document.getElementById('cosmic-dynamics-gravity');
        if (gravityInput) delete gravityInput.dataset.s5CtrlBound;
        const softeningInput = document.getElementById('cosmic-dynamics-softening');
        if (softeningInput) delete softeningInput.dataset.s5CtrlBound;
        const dtInput = document.getElementById('cosmic-dynamics-dt');
        if (dtInput) delete dtInput.dataset.s5CtrlBound;
        const speedLimitFactorInput = document.getElementById('cosmic-dynamics-speed-limit-factor');
        if (speedLimitFactorInput) delete speedLimitFactorInput.dataset.s5CtrlBound;
        if (this.renderer) {
            this.renderer.dispose();
            this.renderer = null;
        }
        this.bridge = null;
        syncScale5Toggles(null); // stop the toolbar checkbox driving a destroyed bridge
        syncScale5Overlays(null); // stop the overlay controls driving a disposed renderer
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

// Resolved once and reused, the same way _panelStatus above avoids
// re-querying the DOM on every physics frame (M15b) — the card is part of
// the static panel template and is not recreated on a scale mount/destroy.
let _profileCardEl = null;
function getProfileCardEl() {
    if (_profileCardEl === null) _profileCardEl = document.getElementById('cosmic-gas-profile-card');
    return _profileCardEl;
}

// Same memoization pattern for the three panels whose liveness gates the
// energy audit (Pass 0b): resolved once and reused rather than three
// getElementById calls on every physics frame (~30 Hz).
let _diagPanelEl = null;
function getDiagPanelEl() {
    if (_diagPanelEl === null) _diagPanelEl = document.getElementById('panel-diagnostics');
    return _diagPanelEl;
}
let _chartsPanelEl = null;
function getChartsPanelEl() {
    if (_chartsPanelEl === null) _chartsPanelEl = document.getElementById('panel-charts');
    return _chartsPanelEl;
}
let _telemetryGridPanelEl = null;
function getTelemetryGridPanelEl() {
    if (_telemetryGridPanelEl === null) _telemetryGridPanelEl = document.getElementById('panel-telemetry-grid');
    return _telemetryGridPanelEl;
}

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
        // Pass 0b: the softened-potential-energy audit (cosmic-physics.js,
        // Pass 0a) is O(N^2) and gated behind bridge._wantEnergyAudit — pay
        // for it only while a consumer of `pe`/`peAvailable` is actually on
        // screen (Diagnostics, Charts, or the Telemetry Grid tab, docked or
        // floated-and-uncollapsed per isPanelLive), so a user watching only
        // the viewport pays nothing for it.
        bridge._wantEnergyAudit = isPanelLive(getDiagPanelEl())
            || isPanelLive(getChartsPanelEl())
            || isPanelLive(getTelemetryGridPanelEl());

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
        const profileCard = getProfileCardEl();
        if (profile) {
            if (profileCard) profileCard.hidden = false;
            _panelStatus.update('cosmic-profile-axis', profile.axis);
            _panelStatus.update('cosmic-thermal', profile.thermal.toExponential(3));
            _panelStatus.update('cosmic-kinetic', profile.kinetic.toExponential(3));
            _drawGasProfileBars(document.getElementById('cosmic-profile-density'), profile.density, '#4ade80', 'nonneg', profile.edges);
            _drawGasProfileBars(document.getElementById('cosmic-profile-velocity'), profile.velocity, '#42a5f5', 'signed', profile.edges);
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
