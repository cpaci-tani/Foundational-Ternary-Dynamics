/**
 * @file app.js
 * @brief FTD Web Dashboard — Main Application Controller
 *
 * [EXTENDED] Initializes all subsystems, manages the frame loop,
 * and wires up UI controls to the simulation bridge.
 */

import { wireToolbar } from './app-wire/toolbar.js';
import { wireParticleControls } from './app-wire/particle-controls.js';
import { wireAtomControls } from './app-wire/atom-controls.js';
import { wireViewportControls, syncViewControls } from './app-wire/viewport-controls.js';
import { wireSettings } from './app-wire/settings.js';
import { createPlaybackActions } from './app-wire/playback-actions.js';
import { LifetimeScope } from './ui/utils/lifetime-scope.js';
import { ObserverWorkspaceHost } from './observer/workspace-host.js';
import { suspendDashboardWork } from './core/dashboard-suspension.js';
import { appRegistry } from './core/registry.js';
import { Viewport } from './viewport.js?v=26';
import { FluxEnergyChart, ParticleChart } from './charts.js';
import { telemetryHub } from './telemetry-hub.js';
import { createScale0ValidityMonitor } from './scales/scale0/runtime/validity.js';
import { subscribeScale0Qualification } from './scales/scale0/state/store.js';
import { createInspectorAppRuntime } from './inspector/app-runtime.js?v=10';
import { initZoo, setEngineMode as setZooMode } from './zoo.js?v=3';
import { populateScale3ScenarioSelect, SCALE3_DEFAULT_SCENARIO } from './scales/scale3/scenario-registry.js';
import { debugLog } from './core/log.js';

// ── Scale Controllers (extracted from inline code) ─────────────────
import * as Scale0Controller from './scales/scale0/controller.js?v=44';
import * as Scale1Controller from './scales/scale1/controller.js?v=31';
import * as Scale2Controller from './scales/scale2/controller.js';
import * as Scale3Controller from './scales/scale3/controller.js';
// ── Phase 1-3: Ontic Observatory, Physics Fidelity, Aggregation Bridge
import * as Scale4Controller from './scales/scale4/controller.js?v=12';
import * as Scale5Controller from './scales/scale5/controller.js';
import * as Scale6Controller from './scales/scale6/controller.js';
import { applyScaleGridAxesDefaults } from './scales/scale-utils.js';
import { OnticObservatory } from './ontic-observatory.js';
import { K_B } from './constants.js';
// renderEnergyLevels, renderCrossSections, renderDecayRates, renderFcCard,
// renderObserverCard, renderOnticHierarchy, renderInfoDynamics moved to
// ui/app-ontic.js (Wave 2 ticket 7).
// ALPHA, G_STAR, VARPI, X_PLUS, X_MINUS, TICK_PHASES, K_B,
// K_GENESIS, C_SPEED, ONTIC_LAYERS, ONTIC_TOTAL_CONSTANTS
// now imported directly by ui/app-ontic.js.
import { createOnticPanel } from './ui/app-ontic.js';
import { BackgroundManager } from './backgrounds.js';
import { AppShell } from './ui/shell/app-shell.js?v=32';
import {
    initChartsPanel,
    initDiagnosticsPanel,
    initInteractionHierarchyPanel,
    initLagrangianPanel,
    initParticleLogPanel,
    initScenePanel,
    initTelemetryGridPanel,
} from './ui/panels/index.js?v=3';
import { isPanelLive } from './ui/panels/panel-visibility.js?v=2';
import { initFluxSlicePanel } from './scales/scale0/ui/overlays/flux-slice-panel.js';
import { initWaveLabPanel } from './scales/scale0/ui/overlays/wave-lab-panel.js?v=2';
import { initP1ObservablesPanel } from './scales/scale0/ui/overlays/p1-observables-panel.js?v=3';
import { initConservationMicropanel } from './scales/scale0/ui/overlays/conservation-micropanel.js';
import { initSpectrumPanel } from './scales/scale0/ui/overlays/spectrum-panel.js';
import { initGravityPanel } from './scales/scale0/ui/overlays/gravity-panel.js?v=6';
import { initFluidPanel } from './scales/scale0/ui/overlays/fluid-panel.js';
import { initTimePanel } from './scales/scale0/ui/overlays/time-panel.js';
import { initThermoPanel } from './scales/scale0/ui/overlays/thermo-panel.js?v=3';
import { initDispersionPanel } from './scales/scale0/ui/overlays/dispersion-panel.js';
import { initKnotsPanel } from './scales/scale0/ui/overlays/knots-panel.js';
import { initTransactionPanel } from './scales/scale0/ui/overlays/transaction-panel.js';
import { initScaleContextPanel } from './scales/scale0/ui/overlays/scale-context-panel.js?v=3';
// Wire / boot helpers extracted per refactoring-analyst RF-9 (partial).
import { wireKeyboard as wireKeyboardExternal } from './app-wire/keyboard.js';
import { showToast, loadProgress as _loadProgress } from './app-wire/status.js';
import { bootBridge, applyEngineStatusChip } from './app-wire/bridge-boot.js?v=10';
import { createBridge } from './bridge-init.js?v=10';
import { sliderValueToSpeed, speedLabel } from './ui/components/play-bar/speed-scale.js';

debugLog('[FTD] App version 20260318a loaded (cache-busted)');

// ── Application State ────────────────────────────────────────────────
let _initialized = false;
const appBindings = new LifetimeScope();
let appFrame = null;
let bridge = null;
// DEBUG: expose bridge globally for console inspection
Object.defineProperty(window, '_ftdBridge', { get() { return bridge; }, configurable: true });
let viewport = null;
let observerHost = null;
let appShell = null;
let scale0Validity = null;
let inspector = null;
let inspectorRuntime = null;
let diagnosticsPanel = null;
let chartsPanel = null;
let telemetryGridPanel = null;
let lagrangianPanel = null;
let interactionHierarchyPanel = null;
let particleLogPanel = null;
// Legacy chart instances (scale1/scale2 still push into these ring buffers).
let fluxEnergyChart = null;
let particleChart = null;

// Two-tier pause system:
//   `running`         — GLOBAL pause. When false, the entire RAF body is skipped:
//                       no physics, no rendering work, no flux mock animation.
//                       The single source of truth for "is anything moving?".
let running = false;
let ticksPerFrame = 1;
let _tickAccumulator = 0; // accumulates fractional ticks for sub-1 speed
let activeTab = 'controls';
let frameCount = 0;
let lastFpsTime = performance.now();
let fpsDisplay = 0;
// Valid engineMode values and their scale indices:
//   'lattice'       (Scale 0) — flux field + particle manifestation
//   'particles'     (Scale 1) — point-particle Coulomb/gravity (PE engine)
//   'atoms'         (Scale 2) — atomic engine with orbital clouds (AE engine)
//   'molecules'     (Scale 3) — same AE engine, molecule scenarios + bonding
//   'planetary'     (Scale 4) — N-body solar system (separate controller)
//   'cosmic'        (Scale 5) — galaxy/cluster simulation (CosmicRenderer)
// Transitions: switchEngineMode() is the SOLE entry point for mode changes.
let engineMode = 'lattice';
let _showOrbitalClouds = true; // orbital electron clouds in AE mode
let bgManager = null;          // BackgroundManager instance
let _prevLegendKey = '';        // cached element-set key for legend rebuild
// Scale 1 field viz flags (_showPEEField, _showPEPotential, _showPEGravField,
// _showPEForces) moved to scales/scale1/controller.js.
// Scale 2 field viz flag (_showAEField) moved to scales/scale2/controller.js.
// Enhanced atom/molecule visual state
let _showNucleusShells = true;    // strong force glow shells around nuclei
let _bondStyle = 'cylinders';     // 'cylinders' | 'lines' | 'off'
let _showShellBounds = false;     // translucent shell boundary spheres
let _showOrbitalLobes = false;    // p/d/f orbital lobe shapes
let _showAEForceIonic = false;    // Coulomb force arrows
let _showAEForceVdw = false;      // van der Waals force arrows
let _showAEForceBond = false;     // bond spring force arrows
let _showAEForceHBond = false;    // H-bond force arrows
let _showAEForceAngle = false;    // angle-strain force arrows
let _showAEForceDipole = false;   // dipole-dipole force arrows
let _showAEForceNet = false;      // net force arrows
let _forceFrame = 0;              // throttle: compute forces every 2nd frame
let _fieldParticleBuf = [];     // reusable {x,y,z} array for E/B field seeds (Scale 0)
let _aeLabelBuf = [];           // reusable label objects for AE element labels
const _aeLegendZSet = new Set(); // reusable Set for AE legend key computation
const _aeLegendZArr = [];        // reusable sorted array for AE legend key
// AE cloud merge buffers (_aeMergeCap/Pos/Col/Size) moved to
// scales/scale2/controller.js when the AE animator was extracted.
// Scale 0 field viz state (_dualLVecs, _dualRVecs, _chiralValues,
// _show*Field flags, _fieldFrame, _fieldNeedsUpdate, _anyFieldActive)
// lives in Scale0Controller. Read/write via the controller's exported
// getFieldState() / setFieldToggle() API. The fluxMock now lives entirely
// inside `state.fluxMock` (see scales/scale0/state/store.js); the legacy
// app-level `_fluxMock` global was retired with the harness migration
// cleanup and intercept removal.



// Black hole scenario state (Scale 1 only)
// [SELECTION] All BH constants are pedagogical choices for visualization,
// not derived from theory. See pe-micro-bh scenario for usage.
let _bhActive = false;
let _bhHawkingTick = 0;
const _BH_HAWKING_INTERVAL = 300;  // ticks between Hawking pair emissions
const _BH_HORIZON_R = 3.0;         // visual event horizon radius
const _BH_MASS = 5000;             // MeV (pedagogical, not physical)
const _BH_TEST_MASS = K_B;         // electron mass (MeV) for test particles

// _recomputeAnyFieldActive() removed — this logic lives inside
// Scale0Controller. The app.js copy was never called and referenced
// module-local flags that have since moved to the controller.

/**
 * Build a shared context object for scale controllers.
 * Uses getters/setters so controllers read/write the live module-level
 * variables (running, ticksPerFrame, engineMode) rather than snapshots.
 *
 * PERF (F-16): every field below is either a live getter/setter over module
 * state or a function reference that is stable for the app's lifetime — there
 * are NO per-call plain-value snapshots. The object is therefore built exactly
 * once and the same instance is returned on every call (including the per-frame
 * lattice/cosmic/meta animators). Getter semantics are preserved verbatim:
 * each property access still re-reads the live module variable, so consumers
 * never observe a stale snapshot. Eliminates one object + several closure
 * allocations per call.
 */
let _ctxSingleton = null;
function _makeCtx() {
    if (_ctxSingleton) return _ctxSingleton;
    _ctxSingleton = {
        get bridge() { return bridge; },
        get viewport() { return viewport; },
        get appShell() { return appShell; },
        get scale0Validity() { return scale0Validity; },
        get inspector() { return inspector; },
        // Exposed so scale controllers that own their own bridge (Scale 4
        // planetary, Scale 5 cosmic) can re-point the inspector via
        // inspectorRuntime.setBridge() instead of falling through to the
        // bare inspector handle (audit P1-1, 2026-05-27).
        get inspectorRuntime() { return inspectorRuntime; },
        get diagnosticsPanel() { return diagnosticsPanel; },
        get chartsPanel() { return chartsPanel; },
        get telemetryGridPanel() { return telemetryGridPanel; },
        get lagrangianPanel() { return lagrangianPanel; },
        get fluxEnergyChart() { return fluxEnergyChart; },
        get particleChart() { return particleChart; },
        get telemetryHub() { return telemetryHub; },
        get running() { return running; },
        set running(v) { running = v; },
        get ticksPerFrame() { return ticksPerFrame; },
        get engineMode() { return engineMode; },
        get presentationSuspended() { return !!observerHost?.suspended; },
        get activeTab() { return activeTab; },
        // Is a telemetry consumer actually rendered? A collapsed floating
        // panel has no visible consumer and must not keep GPU reductions alive.
        isPanelVisible: _isPanelVisibleFn,
        get frameCount() { return frameCount; },
        get dom() { return _dom; },
        updateOnticPanel:   () => onticPanel?.updateOnticPanel(),
        resetAllVisualState: _resetAllVisualState,
        _resetAllVisualState,
        updatePlayButton,
        pauseSimulation,
        togglePlay,
        applyTicksPerFrameFromSlider,
        applyBoundaryShape,
        applyReflectiveBoundary,
        applyFluxBoundaryMode,
        applyFluxPeriodicAxis,
        clearCharts,
    };
    return _ctxSingleton;
}

function pauseSimulation() {
    running = false;
    bridge?.cancelQueuedTicks?.();
    if (engineMode === 'lattice') {
        Scale0Controller.setPlaybackRunning(_makeCtx(), false);
    }
    updatePlayButton();
}

function applyTicksPerFrameFromSlider(value) {
    const slider = document.getElementById('ticks-per-frame');
    const display = document.getElementById('tpf-display');
    if (slider) slider.value = String(value);
    ticksPerFrame = sliderValueToSpeed(value);
    _tickAccumulator = 0;
    if (display) display.textContent = speedLabel(ticksPerFrame);
    if (engineMode === 'lattice') {
        Scale0Controller.setPlaybackSpeed(_makeCtx(), ticksPerFrame);
    } else if (bridge && typeof bridge.setTicksPerFrame === 'function') {
        bridge.setTicksPerFrame(ticksPerFrame);
    }
}

function applyBoundaryShape(shape) {
    const boundarySelect = document.getElementById('boundary-select');
    if (boundarySelect) boundarySelect.value = shape;
    viewport?.setBoundaryShape?.(shape);
    // Exactly one bridge owns live Scale-0 physics. Mirroring this write to the
    // idle main-thread bridge and worker created split state during fallback.
    const owner = Scale0Controller.getActivePhysicsOwner(_makeCtx());
    owner?.setBoundaryShape?.(shape);
    Scale0Controller.setLatticeNeedsUpload();
}

// 0 = Periodic, 1 = Reflective, 2 = Dispersal. Every law owns all six faces;
// the orientation axis is metadata for forward/lateral/vertical presentation.
function applyFluxBoundaryMode(mode) {
    const normalized = Number.isInteger(Number(mode)) && Number(mode) >= 0 && Number(mode) <= 2
        ? Number(mode)
        : 2;
    const sel = document.getElementById('flux-boundary-mode');
    if (sel) sel.value = String(normalized);
    // Exactly one physics owner receives a live boundary command. The idle
    // main-thread bridge is rebuilt/reconfigured if worker fallback is needed;
    // mirroring every UI input into it only created split ownership.
    const owner = Scale0Controller.getActivePhysicsOwner(_makeCtx());
    owner?.setFluxBoundaryMode?.(normalized);
    const axisSelect = document.getElementById('flux-periodic-axis');
    const periodicAxis = Math.max(0, Math.min(3,
        Math.trunc(Number(axisSelect?.value ?? 2))));
    // The viewport's legacy particle-wall flag must describe reflective mode
    // for direct user changes as well as scenario defaults.
    viewport?.setReflectiveBoundary?.(normalized === 1);
    viewport?.setBoundaryDynamics?.(normalized, periodicAxis);
    Scale0Controller.setLatticeNeedsUpload();
}

function applyFluxPeriodicAxis(axis) {
    const normalized = Math.max(0, Math.min(3, Math.trunc(Number(axis) || 0)));
    const sel = document.getElementById('flux-periodic-axis');
    if (sel) sel.value = String(normalized);
    const owner = Scale0Controller.getActivePhysicsOwner(_makeCtx());
    owner?.setFluxPeriodicAxis?.(normalized);
    const mode = Math.max(0, Math.min(2,
        Math.trunc(Number(document.getElementById('flux-boundary-mode')?.value ?? 2))));
    viewport?.setBoundaryDynamics?.(mode, normalized);
    Scale0Controller.setLatticeNeedsUpload();
}

function applyReflectiveBoundary(on) {
    // Legacy path: map bool → flux boundary mode (on=Reflective/1, off=Dispersal/2)
    applyFluxBoundaryMode(on ? 1 : 2);
}

/**
 * Master visual state reset — called by EVERY scenario loader to prevent
 * state leakage between scenarios. Resets:
 *   - Scale 0 field visualization flags + buttons
 *   - Scale 1 PE overlay flags + buttons + dynamics buttons
 *   - Scale 1 velocity/trail flags + buttons
 *   - Scale 2 AE field overlay button
 *   - Charts, Lagrangian, diagnostics panel (hub-backed)
 *   - PE telemetry, trail history, field grid cache
 *   - Viewport overlays (trails, element labels, field visualizations)
 */

// Reset simulation data caches (always on scenario change) but PRESERVE visual toggles
function _resetSimCaches() {
    clearCharts();
    Scale1Controller.resetScale1(_makeCtx()); // clears trail history + cloud caches
    if (viewport) {
        viewport.clearTrails();
        viewport.clearElementLabels();
        viewport.clearMolecularMeshes();
        viewport.updateParticles({ count: 0 });
    }
    Scale0Controller.setLatticeNeedsUpload();
}

// Full visual reset — only called on ENGINE MODE SWITCH (scale change), not scenario change
function _resetAllVisualState() {
    _resetSimCaches();

    // ── Scale 0: delegate to controller for field state, buttons, viewport overlays ──
    Scale0Controller.resetScale0(_makeCtx());

    if (engineMode !== 'lattice') {
        if (viewport) {
            viewport.toggleFluxVolume(false);
            viewport.toggleFluxSlice(false);
        }
    }

    // ── Scale 1: PE overlay buttons (delegated to Scale1Controller) ──
    Scale1Controller.resetScale1(_makeCtx());
    for (const id of [
        'toggle-pe-efield', 'toggle-pe-potential', 'toggle-pe-field-battery',
        'toggle-pe-gravity-field',
        'toggle-pe-force-coulomb', 'toggle-pe-force-gravity',
        'toggle-pe-force-lorentz', 'toggle-pe-force-exchange',
        'toggle-pe-force-strong', 'toggle-pe-force-radiation',
        'toggle-pe-force-magnetic-dipole', 'toggle-pe-force-spin-orbit',
        'toggle-pe-force-net',
        'toggle-pe-system',
        'toggle-pe-admissibility', 'toggle-pe-provenance',
        'toggle-velocities', 'toggle-trails',
    ]) {
        const btn = document.getElementById(id);
        if (btn) btn.classList.remove('active');
    }
    if (viewport) {
        viewport.togglePEStreamlines(false);
        viewport.toggleFieldHeatmap(false);
        viewport.toggleFieldVectors(false);
        viewport.toggleGravityVectors(false);
        viewport.togglePEForceCoulomb(false);
        viewport.togglePEForceGravity(false);
        viewport.togglePEForceStrong(false);
        viewport.togglePEForceNet(false);
        viewport.togglePESystem(false);
        viewport.toggleVelocityVectors(false);
        viewport.toggleTrails(false);
    }

    // ── Scale 2/3: delegated to Scale2Controller ──
    Scale2Controller.resetScale2({ viewport });

    // Reset AE toggle buttons (DOM shared across scales, kept here)
    const aeFieldBtn2 = document.getElementById('toggle-ae-field');
    if (aeFieldBtn2) {
        aeFieldBtn2.classList.remove('active');
        aeFieldBtn2.setAttribute('aria-pressed', 'false');
    }
    for (const id of [
        'ae-show-clouds', 'ae-show-shells', 'ae-show-labels', 'ae-show-shell-bounds', 'ae-show-lobes',
        'ae-force-ionic', 'ae-force-vdw', 'ae-force-bond',
        'ae-force-hbond', 'ae-force-angle', 'ae-force-dipole', 'ae-force-net',
        'toggle-ae-velocities', 'toggle-ae-dipoles', 'toggle-ae-hbonds',
        'toggle-ae-nuclear-events', 'toggle-ae-radiation', 'toggle-ae-heat',
        'toggle-ae-nuclear-boundary',
    ]) {
        const el = document.getElementById(id);
        if (el) {
            if (el.type === 'checkbox') {
                el.checked = (id === 'ae-show-shells' || id === 'ae-show-clouds' || id === 'ae-show-labels');
            } else {
                el.classList.remove('active');
                el.setAttribute('aria-pressed', 'false');
            }
        }
    }
    const bondSelect = document.getElementById('bond-style-select');
    if (bondSelect) bondSelect.value = 'cylinders';
}

// Phase 1-3 state
let observatory = null;
let _physicsZ = 1; // current Z for physics tab
// Ontic panel provider (Wave 2 ticket 7) — bound to live-state getters so
// it reads bridge/engineMode/observatory/etc. via deps at call time.
let onticPanel = null;

// ── Cached DOM Elements (populated in init()) ──────────────────────
// Avoids repeated getElementById() calls in 60fps animation loops.
const _dom = {
    statusPtime: null, statusParticles: null,
    statusEnergy: null, statusDot: null, statusState: null,
    statusFps: null, aeLegend: null,
    aeDiagCount: null, aeDiagBonds: null, aeDiagKe: null,
    aeDiagEtotal: null, aeDiagPeIonic: null, aeDiagPeVdw: null,
    aeDiagPeBond: null, aeDiagTemp: null, aeDiagMomentum: null,
    aeDiagTick: null, aeDiagDrift: null,
    aeDiagMass: null, aeDiagNbe: null, aeDiagBa: null,
    aeDiagEbe: null, aeDiagMassKb: null,
};

function _cacheDOM() {
    _dom.statusPtime = document.getElementById('status-ptime');
    _dom.statusParticles = document.getElementById('status-particles');
    _dom.statusEnergy = document.getElementById('status-energy');
    _dom.statusDot = document.getElementById('status-dot');
    _dom.statusState = document.getElementById('status-state');
    _dom.statusFps = document.getElementById('status-fps');
    _dom.aeLegend = document.getElementById('ae-legend');
    _dom.aeDiagCount = document.getElementById('ae-diag-count');
    _dom.aeDiagBonds = document.getElementById('ae-diag-bonds');
    _dom.aeDiagKe = document.getElementById('ae-diag-ke');
    _dom.aeDiagEtotal = document.getElementById('ae-diag-etotal');
    _dom.aeDiagPeIonic = document.getElementById('ae-diag-pe-ionic');
    _dom.aeDiagPeVdw = document.getElementById('ae-diag-pe-vdw');
    _dom.aeDiagPeBond = document.getElementById('ae-diag-pe-bond');
    _dom.aeDiagTemp = document.getElementById('ae-diag-temp');
    _dom.aeDiagMomentum = document.getElementById('ae-diag-momentum');
    _dom.aeDiagTick = document.getElementById('ae-diag-tick');
    _dom.aeDiagDrift = document.getElementById('ae-diag-drift');
    _dom.aeDiagMass = document.getElementById('ae-diag-mass');
    _dom.aeDiagNbe = document.getElementById('ae-diag-nbe');
    _dom.aeDiagBa = document.getElementById('ae-diag-ba');
    _dom.aeDiagEbe = document.getElementById('ae-diag-ebe');
    _dom.aeDiagMassKb = document.getElementById('ae-diag-mass-kb');
}

// ── Reusable particle position buffer for field seed generation ─────
// Avoids allocating {x,y,z} objects per particle per frame in E/B field paths.
function _fillFieldParticleBuf(pData) {
    while (_fieldParticleBuf.length < pData.count) _fieldParticleBuf.push({ x: 0, y: 0, z: 0 });
    _fieldParticleBuf.length = pData.count;
    for (let i = 0; i < pData.count; i++) {
        _fieldParticleBuf[i].x = pData.positions[i * 3];
        _fieldParticleBuf[i].y = pData.positions[i * 3 + 1];
        _fieldParticleBuf[i].z = pData.positions[i * 3 + 2];
    }
}

// ── Scale 1 PE cloud/trail code REMOVED ──────────────────────────────
// ensureCloudTemplate, expandPEToCloud, updateTrailHistory, _trailActiveIds
// all moved to Scale1Controller. See engine/web/js/scales/scale1/controller.js

// ── Toast Notification System ────────────────────────────────────────
// Leaf modules (scenario-loader, scale0 toolbar) reach the toast system via
// this window hook — they must not import app.js (CONTRACTS §3 Rule 1).
window.showToast = showToast;
appBindings.on(window, 'ftd:engine-error', event => {
    const detail = event.detail || {};
    const message = detail.error || 'The native engine rejected a command.';
    scale0Validity?.runtimeFailure(message);
    showToast(message, 'error');
    // A quarantined (restart-required) native engine is unrecoverable by
    // reconnecting — a desktop host gets its own clean restart flow via the
    // postMessage below; a plain browser tab has no such host, so fall back
    // to WASM live instead of leaving the dashboard silently dead.
    if (detail.restartRequired && !window.chrome?.webview) {
        fallBackToWasm('restart-required');
    }
    window.chrome?.webview?.postMessage?.({
        type: 'engine-error',
        message,
        // A timed-out CUDA fence cannot be made safe by destroying its live
        // buffers in-process. Let the desktop host present its clean WSL
        // engine restart flow instead of leaving the dashboard reconnecting
        // forever to a deliberately quarantined server.
        restartRequired: !!detail.restartRequired,
    });
});
appBindings.on(window, 'ftd:engine-progress', event => {
    const detail = event.detail || {};
    window.chrome?.webview?.postMessage?.({
        type: 'engine-progress',
        operation: detail.operation || 'operation',
        phase: detail.phase || 'working',
        size: Number(detail.size) || 0,
    });
});

// ── Native-engine loss: live fallback to WASM ────────────────────────
// The native bridge (ws_server.exe) can go away two ways while a page is
// actively using it: voluntarily — this page's own "Switch to WASM" button,
// or the machine-wide "Stop all engine servers" button, both in
// gpu-server-card.js, which dispatch 'ftd:gpu-server-stopped' on success —
// or involuntarily, via the restartRequired branch above. Either way this
// is the one place that swaps the shared `bridge` back to an in-thread
// WasmBridge, live, with no page reload.
//
// Ordinary transient disconnects are deliberately NOT handled here:
// WebSocketBridge's exponential-backoff reconnect keeps retrying forever by
// design (a native engine can be briefly restarted mid-session — see its
// _scheduleReconnect doc comment); this only fires once the native side is
// *confirmed* gone for good, not on every dropped socket.
let _fallingBackToWasm = false;

async function fallBackToWasm(reason) {
    if (_fallingBackToWasm || !bridge || bridge.isWasm) return;
    _fallingBackToWasm = true;
    debugLog(`[fallBackToWasm] Switching to WASM (${reason})`);
    try {
        const latticeSize = bridge.latticeSize
            || parseInt(document.getElementById('lattice-size')?.value, 10) || 33;
        try { bridge.dispose?.(); } catch (_e) { /* already torn down */ }
        bridge = await createBridge(latticeSize);
        appRegistry.register('activeBridge', bridge);
        inspectorRuntime?.setBridge(bridge);
        applyEngineStatusChip(bridge);
        showToast('GPU engine unavailable — switched to the in-browser WASM engine. '
            + 'Use "Reload & connect" in the GPU card to go back.', 'info');

        // Re-arm whichever scale is actually visible so it has something to
        // simulate on the new bridge; the other scales pick up the fresh
        // `bridge` on their own next per-frame ctx build (_buildScale1Ctx /
        // _buildScale2Ctx / _makeCtx's live getters), same as a scale switch.
        // Scale 4/5/6 own their own bridge independent of this one and are
        // unaffected. Toggles reset to the scenario's defaults, matching
        // what already happens on an ordinary scale switch — there is no
        // enumerable toggle list to snapshot and replay generically.
        if (engineMode === 'lattice') {
            const scenario = document.getElementById('scenario-select')?.value || 'flux-pulse';
            Scale0Controller.loadScenario(_makeCtx(), scenario);
        } else if (engineMode === 'particles') {
            loadPEScenario(document.getElementById('pe-scenario-select')?.value || 's1-native-m3-replay');
        } else if (engineMode === 'atoms') {
            loadAEScenario(document.getElementById('ae-scenario-select')?.value || 'ae-hydrogen-atom');
        } else if (engineMode === 'molecules') {
            loadMoleculeScenario(document.getElementById('mol-scenario-select')?.value || 'mol-water');
        }
        Scale0Controller.setLatticeNeedsUpload();
    } catch (err) {
        console.error('[fallBackToWasm] Failed to create WASM bridge:', err);
        showToast('GPU engine is gone and the WASM fallback failed to start ('
            + err.message + '). Reload the page.', 'error');
    } finally {
        _fallingBackToWasm = false;
    }
}
appBindings.on(window, 'ftd:gpu-server-stopped', () => { fallBackToWasm('stopped-by-user'); });

// ── Initialization ───────────────────────────────────────────────────
// Safety timeout: dismiss loading overlay after 8000ms even if init() hangs
// (e.g. WASM compilation stalls, WebGL context fails). This prevents the user
// from being stuck on a blank screen.
appBindings.timeout(() => {
    const lo = document.getElementById('loading-overlay');
    if (lo && !lo.classList.contains('hidden')) {
        lo.classList.add('hidden');
        appBindings.timeout(() => lo.classList.add('removed'), 350);
        debugLog('[loading] Safety timeout dismissed overlay');
    }
}, 8000);

async function init() {
    if (_initialized) return;
    _initialized = true;

    appShell = new AppShell({
        app: document.getElementById('app'),
        onViewportResize: () => viewport?.resize?.(),
        onDestroy: () => {
            pauseSimulation();
            if (appFrame !== null) cancelAnimationFrame(appFrame);
            appFrame = null;
            appBindings.dispose();
        },
    }).init();

    scale0Validity = createScale0ValidityMonitor(appShell.topbar?.validity);
    // App lifetime subscription: pagehide can enter BFCache, whose restored page
    // still needs load/reset notifications. A full navigation discards this page.
    appBindings.defer(subscribeScale0Qualification(snapshot => scale0Validity.setQualification(snapshot)));

    _loadProgress(5, 'Caching DOM...');
    _cacheDOM();

    _loadProgress(10, 'Probing GPU engine...');
    const latticeSelect = document.getElementById('lattice-size');
    const requestedLattice = Number(new URLSearchParams(window.location.search).get('lattice'));
    if (latticeSelect && Number.isInteger(requestedLattice)
        && requestedLattice >= 4 && requestedLattice <= 256) {
        if (![...latticeSelect.options].some(option => Number(option.value) === requestedLattice)) {
            latticeSelect.add(new Option(String(requestedLattice), String(requestedLattice)));
        }
        latticeSelect.value = String(requestedLattice);
    }
    const latticeSize = parseInt(latticeSelect.value);
    bridge = await bootBridge(latticeSize, { showToast, loadProgress: _loadProgress });
    appRegistry.register('activeBridge', bridge);

    // Create 3D viewport
    _loadProgress(40, 'Building 3D viewport...');
    const viewportContainer = document.getElementById('viewport');
    viewport = new Viewport(viewportContainer);
    viewport.setLatticeSize(latticeSize);
    appRegistry.register('viewport', viewport);
    observerHost = new ObserverWorkspaceHost({
        app: document.getElementById('app'),
        button: document.getElementById('btn-observer-workspace'),
        viewport,
        getMode: () => engineMode,
        getRunning: () => running,
        getReturnFocus: () => document.getElementById('simulation-menu-trigger'),
        suspendDashboard: () => suspendDashboardWork([
            Scale0Controller.getActivePhysicsOwner(_makeCtx()), bridge,
        ], { settleAnalysis: () => Scale0Controller.settleBackgroundAnalysis() }),
        setRunning: value => {
            if (!value) pauseSimulation();
            else {
                running = true;
                if (engineMode === 'lattice') Scale0Controller.setPlaybackRunning(_makeCtx(), true);
                updatePlayButton();
            }
        },
        readLattice: () => {
            const owner = engineMode === 'lattice' ? Scale0Controller.getActivePhysicsOwner(_makeCtx()) : null;
            return { owner, mode: engineMode, scenario: Scale0Controller.getCurrentScenarioId() };
        },
    });
    appBindings.defer(() => observerHost?.dispose());
    appRegistry.register('observerHost', observerHost);
    // The assistant loads independently; inference never becomes a frame-loop dependency.
    void import('./assistant/bootstrap.js').then(({ createAssistant }) => createAssistant({
        getCtx: _makeCtx, isLattice: () => engineMode === 'lattice', loadLatticeScenario: Scale0Controller.loadScenario,
        host: observerHost, registry: appRegistry, app: document.getElementById('app'),
    })).then(assistant => appBindings.defer(() => assistant.dispose()))
        .catch(error => console.warn('JEV console unavailable:', error.message));


    _loadProgress(50, 'Creating panels...');
    // Initialize panel component wrappers (Phase 4)
    diagnosticsPanel = initDiagnosticsPanel();
    chartsPanel = initChartsPanel();
    telemetryGridPanel = initTelemetryGridPanel();
    lagrangianPanel = initLagrangianPanel();
    interactionHierarchyPanel = initInteractionHierarchyPanel();
    particleLogPanel = initParticleLogPanel();
    initFluxSlicePanel();
    initWaveLabPanel();
    initP1ObservablesPanel();
    initConservationMicropanel();
    initSpectrumPanel();
    initGravityPanel();
    initFluidPanel();
    initTimePanel();
    initThermoPanel();
    initDispersionPanel();
    initKnotsPanel();
    initTransactionPanel();
    initScaleContextPanel();
    appRegistry.register('panel:fluxSlice', window.__ftdFluxSlicePanel);
    appRegistry.register('panel:waveLab', window.__ftdWaveLabPanel);
    appRegistry.register('panel:p1Observables', window.__ftdP1Panel);
    appRegistry.register('panel:conservation', window.__ftdConservationPanel);
    appRegistry.register('panel:spectrum', window.__ftdSpectrumPanel);
    appRegistry.register('panel:gravity', window.__ftdGravityPanel);
    appRegistry.register('panel:time', window.__ftdTimePanel);
    appRegistry.register('panel:thermo', window.__ftdThermoPanel);
    appRegistry.register('panel:dispersion', window.__ftdDispersionPanel);
    appRegistry.register('panel:knots', window.__ftdKnotsPanel);
    appRegistry.register('panel:transactions', window.__ftdTransactionPanel);
    // Scene panel — curated render controls (FOV / exposure / bloom / fog / ...).
    // Scales 0–3 only (gated by panel-registry); unmounted cleanly when
    // the user switches to a separate-renderer scale like 4/5/12.
    initScenePanel({
        panelArea: document.getElementById('panel-area'),
        viewport,
        backgroundManager: bgManager,
    });
    // Scale 0 charts + Lagrangian now own their own uPlot instances via
    // ChartsPanelComponent and LagrangianPanelComponent. Legacy
    // FluxEnergyChart / ParticleChart are retained (with null canvases)
    // because scale1 / scale2 controllers still push frame data into them;
    // they render nothing but keep the ring buffers populated so any later
    // re-consumer sees history. (panels redesign 2026-04)
    fluxEnergyChart = new FluxEnergyChart(null, {
        fluxBuf:   telemetryHub.flux,
        energyBuf: telemetryHub.energy,
    });
    particleChart = new ParticleChart(null, {
        totalBuf: telemetryHub.manifested,
        posBuf:   telemetryHub.positive,
        negBuf:   telemetryHub.negative,
    });
    inspectorRuntime = createInspectorAppRuntime({ viewport, bridge, setZooMode });
    inspector = inspectorRuntime.inspector;
    interactionHierarchyPanel?.setInspector?.(inspector);

    // Build ontic-panel provider with live-state getters (Wave 2 ticket 7).
    // observatory is created below; the getters tolerate nulls so
    // populateConstants() can run first.
    onticPanel = createOnticPanel({
        getBridge:            () => bridge,
        getEngineMode:        () => engineMode,
        getObservatory:       () => observatory,
        getPhysicsZ:          () => _physicsZ,
        setPhysicsZ:          (z) => { _physicsZ = z; },
    });

    // Populate constants table from WASM if available
    onticPanel.populateConstants();

    // Scale 2 scenario select is populated in createScale2ScenarioToolbarGroup().
    buildScale3MoleculeDropdown();

    _loadProgress(60, 'Initializing observatory...');
    observatory = new OnticObservatory();
    onticPanel.initOnticPhysicsHierarchy();

    _loadProgress(70, 'Wiring controls...');
    // Play bar owns the playback buttons (play/step/reset/speed).
    // Mount it before wireToolbar so those button IDs exist in the DOM
    // when the toolbar wirer attaches its listeners.
    Scale0Controller.mountScale0PlaybackUI();
    appBindings.defer(wireToolbar(_makeCtx(), playbackActions));
    wireTabs();
    // Scale controllers own their own UI wiring (controls panel cards, event
    // handlers). The application binders own shared transport, viewport, and
    // particle/atom controls through live context getters.
    Scale0Controller.bindUI(_makeCtx());
    // The cards persist across scale switches; release them only with the app.
    appBindings.defer(() => _makeCtx().disposeScale0Controls?.());
    Scale1Controller.bindScale1ControlsUI();
    Scale2Controller.bindScale2ControlsUI();
    Scale3Controller.bindScale3ControlsUI();
    appBindings.defer(wireParticleControls(_makeCtx(), { loadPEScenario }));
    appBindings.defer(wireAtomControls(_makeCtx(), { loadAEScenario }));
    appBindings.defer(wireViewportControls(_makeCtx()));
    appBindings.defer(wireKeyboard());

    // Cold-boot lifecycle mount (pagehide → exitScale0, overlay panel
    // idempotent init, prime-tick button). switchEngineMode already mounts
    // on every scale re-entry; without this, the first lattice session never
    // registered pagehide cleanup until the user left and returned.
    Scale0Controller.mount(_makeCtx());

    // ── Wire Immersive Mode (UI Toggle) ──
    const btnToggleUI = document.getElementById('btn-toggle-ui');
    if (btnToggleUI) {
        const eyeOpenSVG = `<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg>`;
        const eyeClosedSVG = `<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-10-7-10-7a19.45 19.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 10 7 10 7a19.5 19.5 0 0 1-2.58 3.73M1 1l22 22"></path></svg>`;

        const updateToggleButton = (isHidden) => {
            if (isHidden) {
                btnToggleUI.innerHTML = eyeOpenSVG;
                btnToggleUI.title = 'Show UI (Ctrl+U)';
            } else {
                btnToggleUI.innerHTML = eyeClosedSVG;
                btnToggleUI.title = 'Hide UI (Ctrl+U)';
            }
        };

        // Initial state is visible (closed eye icon represents hide action)
        updateToggleButton(false);

        const toggleUI = () => {
            const isHidden = document.documentElement.classList.toggle('ui-hidden');
            updateToggleButton(isHidden);
            
            // Force WebGL renderer resize and camera update
            appBindings.timeout(() => {
                window.dispatchEvent(new Event('resize'));
            }, 50);
        };

        appBindings.on(btnToggleUI, 'click', toggleUI);

        const btnShowUI = document.getElementById('btn-show-ui');
        if (btnShowUI) {
            appBindings.on(btnShowUI, 'click', toggleUI);
        }

        // Bind keyboard shortcut (Ctrl+U)
        appBindings.on(document, 'keydown', (e) => {
            if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'u') {
                e.preventDefault();
                toggleUI();
            }
        });
    }

    _loadProgress(80, 'Loading particle zoo...');
    initZoo(bridge);

    _loadProgress(85, 'Configuring viewport...');
    viewport.toggleFluxVolume(true);

    // Initialize environment backgrounds
    bgManager = new BackgroundManager(viewport.scene);
    const bgSelect = document.getElementById('bg-select');
    bgManager.set(bgSelect.value, viewport.renderer);
    appBindings.on(bgSelect, 'change', () => bgManager.set(bgSelect.value, viewport.renderer));

    _loadProgress(95, 'Loading scenario...');

    // Load the selector's actual value. Browser/WebView form restoration may
    // retain a non-default scenario without emitting `change`; the controller
    // explicitly reconciles it again on pageshow and native reconnect.
    Scale0Controller.loadSelectedScenario(_makeCtx());

    // Done — dismiss loading overlay
    _loadProgress(100, 'Ready');
    if (appShell) appShell.setReady();
    appBindings.timeout(() => {
        const lo = document.getElementById('loading-overlay');
        if (lo) {
            lo.classList.add('hidden');
            appBindings.timeout(() => lo.classList.add('removed'), 350);
        }
    }, 400); // brief pause at 100% so user sees completion

    // Start frame loop
    appFrame = requestAnimationFrame(animate);
}

// ── Frame Loop ───────────────────────────────────────────────────────
// Main rAF loop — dispatches to the mode-specific animator.
// Always schedules next frame first (unconditional rAF) so the loop
// never stalls, even if a mode-specific function throws.
// NOTE: 'planetary' mode is a no-op here; it runs via its own
// rafCoordinator subscription ('scale4-planetary-loop') set up in
// Scale4Controller.loadScenario. All other modes (including 'cosmic'
// after Phase B.1) drive physics + render from this rAF loop.
function animate(now) {
    if (appBindings.disposed) return;
    appFrame = requestAnimationFrame(animate);

    if (observerHost?.suspended) {
        observerHost.frame(now);
        return;
    }

    if (engineMode === 'cosmic') {
        Scale5Controller.animateCosmic(_makeCtx());
    } else if (engineMode === 'atoms' || engineMode === 'molecules') {
        animateAE(now);
    } else if (engineMode === 'particles') {
        animatePE(now);
    } else if (engineMode === 'planetary') {
        // Handled via the rafCoordinator 'scale4-planetary-loop'
        // subscription created in Scale4Controller.loadScenario.
    } else if (engineMode === 'meta') {
        // Handled via the rafCoordinator 'scale6-meta-loop' subscription
        // created in Scale6Controller.loadScenario (same pattern as
        // planetary above — MetaUnit has no physics tick, only auto-rotate
        // + label repositioning, so it self-drives at its own cadence).
    } else {
        try {
            Scale0Controller.animateLattice(_makeCtx());
        } catch (error) {
            // Observe direct WASM/frame failures without changing propagation,
            // scheduling or the engine's existing stop/rollback behavior.
            try { scale0Validity?.runtimeFailure(error); } catch { /* retain original error */ }
            throw error;
        }
    }

    // Animate environment background
    if (bgManager) bgManager.update(1 / 60);

    // Update active docked panels or floated windows in real-time. Scale 0
    // owns its active telemetry panel updates on the same cadence as its
    // telemetry collection; app.js still services floated panels and panels in
    // the other engines.
    if (_shouldAppUpdatePanel('telemetry-grid', now)) {
        telemetryGridPanel?.update();
    }
    if (_shouldAppUpdatePanel('charts', now)) {
        chartsPanel?.update();
    }
    if (_shouldAppUpdatePanel('diagnostics', now)) {
        diagnosticsPanel?.update();
    }
    if (_shouldAppUpdatePanel('lagrangian', now)) {
        lagrangianPanel?.update();
    }
    if (_shouldAppUpdatePanel('particle-log', now)) {
        particleLogPanel?.update();
    }
    if (_shouldAppUpdatePanel('interaction-hierarchy', now)) {
        interactionHierarchyPanel?.update();
    }

    // FPS counter
    frameCount++;
    if (now - lastFpsTime >= 1000) {
        fpsDisplay = frameCount;
        frameCount = 0;
        lastFpsTime = now;
        if (_dom.statusFps) _dom.statusFps.textContent = fpsDisplay;
    }
}

const _panelUpdateIntervalMs = Object.freeze({
    // Components are source-stamp dirty-gated, so a 30 Hz presentation pass
    // does not redraw unchanged telemetry. It does ensure floated panels and
    // Scales 1–3 consume a newly completed sample within one display frame
    // instead of waiting behind the former 100 ms (10 Hz) cap.
    diagnostics: 33,
    charts: 33,
    // The grid's visible sparklines must consume every published telemetry
    // sample. 125 ms made them redraw at ~8 Hz while Scale 0 publishes at
    // display-refresh / 3 (~20-24 Hz), producing the visibly stepped motion
    // captured in the 2026-08-28 audit video. Floated and non-Scale-0 grids
    // remain bounded by the component's matching ~30 Hz render cap.
    'telemetry-grid': 33,
    // Lagrangian plots are already source-stamp gated. A floated panel must
    // receive completed observations on the next display frame, just like its
    // docked worker-backed counterpart; 250 ms limited every plot to 4 Hz.
    lagrangian: 0,
    'interaction-hierarchy': 100,
    'particle-log': 100,
});
const _panelLastUpdateAt = new Map();

function _shouldAppUpdatePanel(panelId, now = performance.now()) {
    if (!_isPanelVisibleFn(panelId)) return false;
    const scale0Owned = engineMode === 'lattice' &&
        (panelId === 'charts' || panelId === 'diagnostics' ||
            panelId === 'telemetry-grid' || panelId === 'lagrangian') &&
        activeTab === panelId;
    if (scale0Owned) return false;
    const interval = _panelUpdateIntervalMs[panelId] ?? 100;
    const last = _panelLastUpdateAt.get(panelId) ?? Number.NEGATIVE_INFINITY;
    if (now - last < interval) return false;
    _panelLastUpdateAt.set(panelId, now);
    return true;
}

// animateLattice -- REMOVED: delegated to Scale0Controller.animateLattice(ctx)
// See engine/web/js/scales/scale0/controller.js for the extracted code.

// ── Scale 1/2/3 Context Builders ────────────────────────────────────
// PERF (F-16): animatePE/animateAE run every animation frame, and their
// consumers (Scale1Controller.animatePE / Scale2Controller.animateAE)
// destructure the entire ctx synchronously at the top of the call and never
// retain or re-read it. We therefore reuse one persistent ctx object per scale,
// built once with its stable function references, and refresh ONLY the volatile
// fields in place on each call. This is byte-identical to the previous
// per-frame object literals (which snapshotted the same values by value at the
// same instant), while eliminating one object + the per-frame closure
// allocations. The function references below are app-lifetime-stable, so they
// are captured once; they are NOT getters because the previous literals were
// not getters — the snapshot point is "the moment the builder runs", and that
// is exactly when these fields are refreshed, immediately before the consumer
// destructures them within the same frame.
const _scale1Ctx = {
    bridge: null, viewport: null, running: false,
    ticksPerFrame: 1, inspector: null,
    fluxEnergyChart: null, particleChart: null,
    activeTab: null, frameCount: 0, dom: _dom, now: 0,
    telemetryHub: null, engineMode: null,
    isPanelVisible: null, resetAllVisualState: null,
    updateOnticPanel:   () => onticPanel?.updateOnticPanel(),
};

function _buildScale1Ctx(now) {
    const c = _scale1Ctx;
    c.bridge = bridge;
    c.viewport = viewport;
    c.running = running;
    c.ticksPerFrame = ticksPerFrame;
    c.inspector = inspector;
    c.fluxEnergyChart = fluxEnergyChart;
    c.particleChart = particleChart;
    c.activeTab = activeTab;
    c.frameCount = frameCount;
    c.dom = _dom;
    c.now = now;
    // Ctx-shape consolidation (2026-07-29 revision): the per-frame ctx now
    // carries the same load-bearing members the full _makeCtx() has, so the
    // controller sees ONE shape everywhere (CONTRACTS §3).
    c.telemetryHub = telemetryHub;
    c.engineMode = engineMode;
    c.isPanelVisible = _isPanelVisibleFn;
    c.resetAllVisualState = _resetAllVisualState;
    return c;
}

// Same predicate _makeCtx() exposes, hoisted so the per-frame ctx builder
// doesn't allocate a closure every frame.
const _isPanelVisibleFn = (panelId) => {
    return isPanelLive(document.getElementById(`panel-${panelId}`));
};

const _scale2Ctx = {
    bridge: null, viewport: null, running: false,
    ticksPerFrame: 1, inspector: null,
    fluxEnergyChart: null, particleChart: null,
    activeTab: null, frameCount: 0, dom: _dom, now: 0,
    updatePlayButton,
    updateOnticPanel:   () => onticPanel?.updateOnticPanel(),
    resetAllVisualState: _resetAllVisualState,
    setRunning: (v) => { running = v; },
    engineMode: null,
};

function _buildScale2Ctx(now) {
    const c = _scale2Ctx;
    c.bridge = bridge;
    c.viewport = viewport;
    c.running = running;
    c.ticksPerFrame = ticksPerFrame;
    c.inspector = inspector;
    c.fluxEnergyChart = fluxEnergyChart;
    c.particleChart = particleChart;
    c.activeTab = activeTab;
    c.frameCount = frameCount;
    c.dom = _dom;
    c.now = now;
    c.engineMode = engineMode;
    return c;
}

function animatePE(now) {
    Scale1Controller.animatePE(_buildScale1Ctx(now));
}

function animateAE(now) {
    Scale2Controller.animateAE(_buildScale2Ctx(now));
}

// (Inline animatePE, animateAE, updateAtomicEnergyDisplay, formatSI removed
//  -- now in Scale1Controller and Scale2Controller)

// populateConstants moved to ui/app-ontic.js (Wave 2 ticket 7).

// ── Toolbar Wiring ───────────────────────────────────────────────────


// ── Tab System ───────────────────────────────────────────────────────
// Wires sidebar tab buttons to show/hide corresponding panels.
// Tab visibility is further filtered by switchEngineMode() using
// data-scales attributes so only scale-relevant tabs appear.
// Also wires the panel collapse toggle and the drag-to-resize handle.
function wireTabs() {
    const handlePanelActivated = (target) => {
        activeTab = target;
        const tabLabel = appShell?.getPanelLabel(target) || 'Controls';
        appShell?.setActivePanelTitle(tabLabel);

        if (target === 'charts') {
            chartsPanel?.update();
        } else if (target === 'telemetry-grid') {
            telemetryGridPanel?.update();
        } else if (target === 'lagrangian') {
            lagrangianPanel?.update();
        } else if (target === 'diagnostics') {
            diagnosticsPanel?.update();
        } else if (target === 'physics') {
            onticPanel?.refreshPhysicsPanel();
        }
    };

    appShell?.bindPanelDock({
        activeTab,
        onTabActivated: handlePanelActivated,
    });
}

// ── Controls Panel Wiring ────────────────────────────────────────────
// Scale 0 controls (physics toggles, injection, parameter sliders, flux
// volume, field actions) are wired by Scale0Controller.bindUI via
// js/scales/scale0/ui/controls/wire.js. This function now handles only
// Scale 1 (PE) and Scale 2/3 (AE) controls.


// ── Viewport Toggle Wiring ───────────────────────────────────────────





// ── Keyboard Shortcuts ───────────────────────────────────────────────
// Keyboard shortcut handler — body extracted to app-wire/keyboard.js. This
// thin wrapper provides the live-state getters + mode-specific step/reload
// callbacks the extracted module needs.
function wireKeyboard() {
    return wireKeyboardExternal({
        getEngineMode: () => engineMode,
        getBridge: () => bridge,
        pauseSimulation,
        togglePlay,
        stepScenario: playbackActions.step,
        reloadScenario: playbackActions.reset,
        Scale0Controller,
    });
}

// Settings read the current viewport when used, including after bridge/mode changes.
const disposeSettings = wireSettings({ getViewport: () => viewport });
appBindings.defer(disposeSettings);

// ── Engine Mode Switching ────────────────────────────────────────────
// SOLE entry point for scale transitions. Sequence:
//   1. Stop simulation (running = false)
//   2. Update CSS classes for panel/control visibility
//   3. Filter tab bar to show only tabs valid for this scale
//   4. Dispose old scale resources (cosmic renderer, planetary, etc.)
//   5. Call the new scale's scenario loader
// Rapid switching is safe because step 1 halts ticking before any teardown,
// and each loader calls _resetAllVisualState() which clears all prior state.
const CONTROLLERS = {
    lattice: Scale0Controller,
    particles: Scale1Controller,
    atoms: Scale2Controller,
    molecules: Scale3Controller,
    planetary: Scale4Controller,
    cosmic: Scale5Controller,
    meta: Scale6Controller
};

let modeSwitchGeneration = 0;
async function switchEngineMode(mode) {
    const generation = ++modeSwitchGeneration;
    if (observerHost?.suspended || observerHost?.pending || observerHost?.exiting) {
        await observerHost.exit();
        if (generation !== modeSwitchGeneration) return;
    }
    // 1. Uniform Lifecycle: Teardown previous controller
    const prevController = CONTROLLERS[engineMode];
    if (prevController && typeof prevController.destroy === 'function') {
        prevController.destroy(_makeCtx());
    }

    engineMode = mode;
    scale0Validity?.setMode(mode);

    // Stop simulation on mode switch — prevents leftover play state
    // from a previous mode causing immediate ticking in the new mode
    running = false;
    updatePlayButton();

    // Toggle CSS mode class on root — drives all scale0-only / scale1-only / scale2-only / scale3-only visibility
    const app = document.getElementById('app');
    app.classList.toggle('mode-lattice', mode === 'lattice');
    app.classList.toggle('mode-particles', mode === 'particles');
    app.classList.toggle('mode-atoms', mode === 'atoms');
    app.classList.toggle('mode-molecules', mode === 'molecules');
    app.classList.toggle('mode-planetary', mode === 'planetary');
    app.classList.toggle('mode-cosmic', mode === 'cosmic');
    app.classList.toggle('mode-meta', mode === 'meta');

    // If the active tab is hidden for this scale, fall back to Controls
    const scaleIndex = { lattice: '0', particles: '1', atoms: '2', molecules: '3', planetary: '4', cosmic: '5', meta: '6' }[mode];
    if (appShell) appShell.setActiveScale(scaleIndex);
    else app.setAttribute('data-active-scale', scaleIndex);

    // Keep mode-dependent inspector, viewport, and zoo state in sync.
    inspectorRuntime?.syncMode(mode);

    // Scales 1–5: grid/axes off by default; Scale 0 restores both (+ wireframe).
    applyScaleGridAxesDefaults(viewport, mode);

    // Re-point the inspector at the active scale's bridge (audit P1-1).
    // Scales 1-3 share the app-level bridge; Scale 0 may own a worker.
    // Restore the selected owner here so that returning from a
    // self-bridged scale (Scale 4 planetary / Scale 5 cosmic, which swap
    // in their own bridge during loadScenario) does not leave the
    // inspector querying a stale planetary/cosmic backend. Scales 4/5
    // overwrite this with their own bridge later in their loaders, so the
    // guard avoids clobbering them.
    if (mode === 'lattice' || mode === 'particles'
        || mode === 'atoms' || mode === 'molecules') {
        // The Scale-0 loader updates this again when it selects a new owner.
        // A retained lattice worker must never answer particle/atom queries.
        const fluxMock = Scale0Controller.getFluxMock();
        const activeBridge = (mode === 'lattice' && fluxMock?.isWorker) ? fluxMock : bridge;
        inspectorRuntime?.setBridge(activeBridge);
    }

    const tpfSlider = document.getElementById('ticks-per-frame');
    if (tpfSlider) applyTicksPerFrameFromSlider(tpfSlider.value);

    // 2. Uniform Lifecycle: Mount the next controller
    const nextController = CONTROLLERS[mode];
    if (nextController && typeof nextController.mount === 'function') {
        nextController.mount(_makeCtx());
    }

    if (mode === 'lattice') {
        const scenario = document.getElementById('scenario-select')?.value || 'flux-pulse';
        Scale0Controller.loadScenario(_makeCtx(), scenario);
    } else if (mode === 'particles') {
        loadPEScenario(document.getElementById('pe-scenario-select')?.value || 's1-native-m3-replay');
    } else if (mode === 'atoms') {
        loadAEScenario(document.getElementById('ae-scenario-select')?.value || 'ae-hydrogen-atom');
    } else if (mode === 'molecules') {
        loadMoleculeScenario(document.getElementById('mol-scenario-select')?.value || 'mol-water');
    } else if (mode === 'planetary') {
        Scale4Controller.loadScenario(_makeCtx(), document.getElementById('planetary-scenario-select')?.value || 'planetary-solar');
    } else if (mode === 'cosmic') {
        Scale5Controller.loadCosmicScenario(_makeCtx(), document.getElementById('cosmic-scenario-select')?.value || 'cosmic-galaxy');
    } else if (mode === 'meta') {
        Scale6Controller.loadScenario(_makeCtx());
    }

    Scale0Controller.setLatticeNeedsUpload();
    syncViewControls(_makeCtx());
    frameCount = 0;
}


function loadPEScenario(name) {
    Scale1Controller.loadPEScenario(_makeCtx(), name);
}


// ── Atom Engine Scenarios ────────────────────────────────────────────
function loadAEScenario(name) {
    Scale2Controller.loadAEScenario({ bridge, viewport, inspector, resetAllVisualState: _resetAllVisualState }, name);
}


// AE toggle helpers moved to Scale2Controller; scenario profiles now come from
// the canonical Scale 2 registry rather than an imperative Phase-3 helper.
function _syncAEParamsFromUI() {
    Scale2Controller.syncAEParams({ bridge });
}


// ── Scale 3: Molecule Scenario Loader (delegated to Scale3Controller) ──
function loadMoleculeScenario(name) {
    Scale3Controller.loadMoleculeScenario({ bridge, viewport, inspector, resetAllVisualState: _resetAllVisualState }, name);
}


// ── Build Scale 3 Molecule Dropdown ──────────────────────────────────
function buildScale3MoleculeDropdown() {
    const select = document.getElementById('mol-scenario-select');
    populateScale3ScenarioSelect(select, SCALE3_DEFAULT_SCENARIO);
}

// ── Helpers ──────────────────────────────────────────────────────────
function togglePlay() {
    if (running) {
        pauseSimulation();
        return;
    }
    running = true;
    if (engineMode === 'lattice') {
        Scale0Controller.setPlaybackRunning(_makeCtx(), true);
    }
    updatePlayButton();
}

function updatePlayButton() {
    const step = document.getElementById('btn-step');
    if (step) {
        step.disabled = engineMode === 'meta';
        step.title = step.disabled ? 'This geometry view has no tick-based simulation to step.' : 'Step one tick (S)';
    }
    const btn = document.getElementById('btn-play');
    if (!btn) return;
    const paused = running ? 'false' : 'true';
    const glyph = running ? '\u23F8' : '\u25B6';
    if (btn.dataset.paused === paused && btn.textContent === glyph) return;
    btn.textContent = glyph;
    btn.dataset.paused = paused;
}

function clearCharts() {
    // Reset the hub's ring buffers for all scales — charts share these buffers, so
    // clearing at the hub level is sufficient; uPlot instances redraw from the
    // cleared buffers on the next update().
    telemetryHub.resetAll();
}

// ── Phase 1-3: Ontic / Physics / Hierarchy ────────
// Moved to ui/app-ontic.js as Wave 2 ticket 7 of the large-file refactor.
// Call via onticPanel.initOnticPhysicsHierarchy / updateOnticPanel /
// refreshPhysicsPanel / getOnticDiagnostics /
// getRawDiagnostics / renderOnticChainSummary. See
// Bridge modularization provenance is cataloged in docs/INDEX.md.

const playbackActions = createPlaybackActions(_makeCtx(), {
    lattice: Scale0Controller, planetary: Scale4Controller,
    cosmic: Scale5Controller, meta: Scale6Controller,
}, { switchEngineMode, loadPEScenario, loadAEScenario, loadMoleculeScenario });

// ── Launch ───────────────────────────────────────────────────────────
init().catch(err => {
    scale0Validity?.runtimeFailure(err);
    console.error('FTD Dashboard initialization failed:', err);
    // Show full-screen error overlay so user isn't staring at a blank page
    const overlay = document.createElement('div');
    overlay.id = 'error-overlay';
    overlay.innerHTML = `
        <div class="error-box">
            <h2>Initialization Failed</h2>
            <p>The FTD Dashboard could not start. This may be caused by a missing
               WebGL context, a CDN failure, or an unsupported browser.</p>
            <p>Check the browser console (F12) for details.</p>
            <code>${String(err).replace(/</g, '&lt;')}</code>
        </div>`;
    document.body.appendChild(overlay);
});
