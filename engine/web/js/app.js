/**
 * @file app.js
 * @brief FTD Web Dashboard — Main Application Controller
 *
 * [EXTENDED] Initializes all subsystems, manages the frame loop,
 * and wires up UI controls to the simulation bridge.
 */

import { appRegistry } from './core/registry.js';
import { Viewport } from './viewport.js?v=26';
import { FluxEnergyChart, ParticleChart } from './charts.js';
import { telemetryHub } from './telemetry-hub.js';
import { createInspectorAppRuntime } from './inspector/app-runtime.js?v=5';
import { initZoo, setEngineMode as setZooMode } from './zoo.js?v=3';
import { getCategories, getMoleculesByCategory } from './molecules.js';
import { debugLog } from './core/log.js';

// ── Scale Controllers (extracted from inline code) ─────────────────
import * as Scale0Controller from './scales/scale0/controller.js?v=44';
import * as Scale1Controller from './scales/scale1/controller.js?v=31';
import * as Scale2Controller from './scales/scale2/controller.js';
import * as Scale3Controller from './scales/scale3/controller.js';
import { AE_PHYSICS_SPECS } from './scales/scale2/scenario-registry.js';
// ── Phase 1-3: Ontic Observatory, Physics Fidelity, Aggregation Bridge
import * as Scale4Controller from './scales/scale4/controller.js';
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
import { AggregateDetector, EmergenceMonitor } from './aggregation-bridge.js?v=2';
import { createOnticPanel } from './ui/app-ontic.js';
import { BackgroundManager } from './backgrounds.js';
import { AppShell } from './ui/shell/app-shell.js?v=27';
import {
    initChartsPanel,
    initDiagnosticsPanel,
    initInteractionHierarchyPanel,
    initLagrangianPanel,
    initParticleLogPanel,
    initScenePanel,
    initTelemetryGridPanel,
} from './ui/panels/index.js';
import { floatingWindowManager } from './ui/components/floating-window/component.js?v=2';
import { initFluxSlicePanel } from './scales/scale0/ui/overlays/flux-slice-panel.js';
import { initWaveLabPanel } from './scales/scale0/ui/overlays/wave-lab-panel.js?v=2';
import { initP1ObservablesPanel } from './scales/scale0/ui/overlays/p1-observables-panel.js?v=3';
import { initConservationMicropanel } from './scales/scale0/ui/overlays/conservation-micropanel.js';
import { initSpectrumPanel } from './scales/scale0/ui/overlays/spectrum-panel.js';
import { initGravityPanel } from './scales/scale0/ui/overlays/gravity-panel.js?v=6';
import { initTimePanel } from './scales/scale0/ui/overlays/time-panel.js';
import { initThermoPanel } from './scales/scale0/ui/overlays/thermo-panel.js?v=3';
import { initDispersionPanel } from './scales/scale0/ui/overlays/dispersion-panel.js';
import { initKnotsPanel } from './scales/scale0/ui/overlays/knots-panel.js';
import { initScaleContextPanel } from './scales/scale0/ui/overlays/scale-context-panel.js?v=3';
import { initSettingsModal } from './ui/components/settings-modal/component.js?v=2';
// Wire / boot helpers extracted per refactoring-analyst RF-9 (partial).
import { wireKeyboard as wireKeyboardExternal } from './app-wire/keyboard.js';
import { showToast, loadProgress as _loadProgress } from './app-wire/status.js';
import { bootBridge } from './app-wire/bridge-boot.js?v=10';
import { sliderValueToSpeed, speedLabel } from './ui/components/play-bar/speed-scale.js';
import {
    captureScale1Checkpoint,
    importScale1Checkpoint,
    markScale1ReplayStart,
    restoreSavedScale1Checkpoint,
    serializeScale1Checkpoint,
    verifyScale1Replay,
} from './scales/scale1/checkpoint-replay.js?v=1';

debugLog('[FTD] App version 20260318a loaded (cache-busted)');

// ── Application State ────────────────────────────────────────────────
let _initialized = false;
let bridge = null;
// DEBUG: expose bridge globally for console inspection
Object.defineProperty(window, '_ftdBridge', { get() { return bridge; }, configurable: true });
let viewport = null;
let appShell = null;
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
let _showBonds = true;
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
let aggregateDetector = null;
let emergenceMonitor = null;
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
window.addEventListener('ftd:engine-error', event => {
    const detail = event.detail || {};
    const message = detail.error || 'The native engine rejected a command.';
    showToast(message, 'error');
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
window.addEventListener('ftd:engine-progress', event => {
    const detail = event.detail || {};
    window.chrome?.webview?.postMessage?.({
        type: 'engine-progress',
        operation: detail.operation || 'operation',
        phase: detail.phase || 'working',
        size: Number(detail.size) || 0,
    });
});

// ── Initialization ───────────────────────────────────────────────────
// Safety timeout: dismiss loading overlay after 8000ms even if init() hangs
// (e.g. WASM compilation stalls, WebGL context fails). This prevents the user
// from being stuck on a blank screen.
setTimeout(() => {
    const lo = document.getElementById('loading-overlay');
    if (lo && !lo.classList.contains('hidden')) {
        lo.classList.add('hidden');
        setTimeout(() => lo.classList.add('removed'), 350);
        debugLog('[loading] Safety timeout dismissed overlay');
    }
}, 8000);

async function init() {
    if (_initialized) return;
    _initialized = true;

    appShell = new AppShell({
        app: document.getElementById('app'),
        onViewportResize: () => viewport?.resize?.(),
    }).init();

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
    initTimePanel();
    initThermoPanel();
    initDispersionPanel();
    initKnotsPanel();
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
    // observatory/aggregateDetector/emergenceMonitor are created below; the
    // getters tolerate nulls so populateConstants() can run first.
    onticPanel = createOnticPanel({
        getBridge:            () => bridge,
        getEngineMode:        () => engineMode,
        getObservatory:       () => observatory,
        getAggregateDetector: () => aggregateDetector,
        getEmergenceMonitor:  () => emergenceMonitor,
        getPhysicsZ:          () => _physicsZ,
        setPhysicsZ:          (z) => { _physicsZ = z; },
    });

    // Populate constants table from WASM if available
    onticPanel.populateConstants();

    // Scale 2 scenario select is populated in createScale2ScenarioToolbarGroup().
    buildScale3MoleculeDropdown();

    _loadProgress(60, 'Initializing observatory...');
    observatory = new OnticObservatory();
    aggregateDetector = new AggregateDetector();
    emergenceMonitor = new EmergenceMonitor(500);
    onticPanel.initOnticPhysicsHierarchy();

    _loadProgress(70, 'Wiring controls...');
    // Play bar owns the playback buttons (play/step/reset/speed).
    // Mount it before wireToolbar so those button IDs exist in the DOM
    // when the toolbar wirer attaches its listeners.
    Scale0Controller.mountScale0PlaybackUI();
    wireToolbar();
    wireTabs();
    // Scale controllers own their own UI wiring (controls panel cards, event
    // handlers). wireControls() below only handles Scale 1/2/3 legacy wiring
    // that hasn't yet migrated.
    Scale0Controller.bindUI(_makeCtx());
    Scale1Controller.bindScale1ControlsUI();
    Scale2Controller.bindScale2ControlsUI();
    Scale3Controller.bindScale3ControlsUI();
    wireControls();
    wireViewportToggles();
    wireKeyboard();

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
            setTimeout(() => {
                window.dispatchEvent(new Event('resize'));
            }, 50);
        };

        btnToggleUI.addEventListener('click', toggleUI);

        const btnShowUI = document.getElementById('btn-show-ui');
        if (btnShowUI) {
            btnShowUI.addEventListener('click', toggleUI);
        }

        // Bind keyboard shortcut (Ctrl+U)
        document.addEventListener('keydown', (e) => {
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
    bgSelect.addEventListener('change', () => bgManager.set(bgSelect.value, viewport.renderer));

    _loadProgress(95, 'Loading scenario...');

    // Load the selector's actual value. Browser/WebView form restoration may
    // retain a non-default scenario without emitting `change`; the controller
    // explicitly reconciles it again on pageshow and native reconnect.
    Scale0Controller.loadSelectedScenario(_makeCtx());

    // Done — dismiss loading overlay
    _loadProgress(100, 'Ready');
    if (appShell) appShell.setReady();
    setTimeout(() => {
        const lo = document.getElementById('loading-overlay');
        if (lo) {
            lo.classList.add('hidden');
            setTimeout(() => lo.classList.add('removed'), 350);
        }
    }, 400); // brief pause at 100% so user sees completion

    // Start frame loop
    requestAnimationFrame(animate);
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
    requestAnimationFrame(animate);

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
        Scale0Controller.animateLattice(_makeCtx());
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
    lagrangian: 250,
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
    if (document.documentElement.classList.contains('ui-hidden')) return false;
    if (activeTab === panelId) {
        return !document.getElementById('app')?.classList.contains('panels-collapsed');
    }
    const floating = floatingWindowManager.getWindow(panelId);
    return !!floating && !floating.isCollapsed;
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
function wireToolbar() {
    // Play/Pause
    document.getElementById('btn-play').addEventListener('click', togglePlay);

    // Step
    document.getElementById('btn-step').addEventListener('click', () => {
        pauseSimulation();
        if (engineMode === 'atoms' || engineMode === 'molecules') {
            bridge.aeTick();
        } else if (engineMode === 'particles') {
            bridge.peTick();
        } else if (engineMode === 'cosmic') {
            Scale5Controller.step(_makeCtx());
        } else if (engineMode === 'planetary') {
            // Step the planetary bridge one tick via Scale4Controller
            Scale4Controller.step();
        } else if (engineMode === 'meta') {
            // No-op: MetaUnit has no tick-based physics to step.
        } else {
            Scale0Controller.step(_makeCtx());
        }
    });

    // Reset
    document.getElementById('btn-reset').addEventListener('click', () => {
        pauseSimulation();
        if (engineMode === 'cosmic') {
            Scale5Controller.loadCosmicScenario(_makeCtx(), document.getElementById('cosmic-scenario-select')?.value || 'cosmic-galaxy');
        } else if (engineMode === 'planetary') {
            Scale4Controller.loadScenario(_makeCtx(), document.getElementById('planetary-scenario-select')?.value || 'planetary-solar');
        } else if (engineMode === 'molecules') {
            loadMoleculeScenario(document.getElementById('mol-scenario-select').value);
        } else if (engineMode === 'atoms') {
            loadAEScenario(document.getElementById('ae-scenario-select').value);
        } else if (engineMode === 'particles') {
            loadPEScenario(document.getElementById('pe-scenario-select').value);
        } else if (engineMode === 'meta') {
            Scale6Controller.loadScenario(_makeCtx());
        } else {
            Scale0Controller.reset(_makeCtx());
        }
    });

    const slider = document.getElementById('ticks-per-frame');
    applyTicksPerFrameFromSlider(slider.value);
    let speedInputRaf = null;
    slider.addEventListener('input', () => {
        if (speedInputRaf !== null) return;
        speedInputRaf = requestAnimationFrame(() => {
            speedInputRaf = null;
            applyTicksPerFrameFromSlider(slider.value);
        });
    });

    // Engine mode selector (Scale 0 / Scale 1)
    document.getElementById('engine-mode').addEventListener('change', (e) => {
        pauseSimulation();
        switchEngineMode(e.target.value);
    });

    // PE scenario selector
    document.getElementById('pe-scenario-select').addEventListener('change', (e) => {
        running = false;
        updatePlayButton();
        loadPEScenario(e.target.value);
    });
    // AE scenario selector
    document.getElementById('ae-scenario-select').addEventListener('change', (e) => {
        running = false;
        updatePlayButton();
        loadAEScenario(e.target.value);
    });

    // Scale 3 molecule scenario selector
    const molSelect = document.getElementById('mol-scenario-select');
    if (molSelect) {
        molSelect.addEventListener('change', (e) => {
            running = false;
            updatePlayButton();
            loadMoleculeScenario(e.target.value);
        });
    }

    // Planetary scenario selector
    const planetarySelect = document.getElementById('planetary-scenario-select');
    if (planetarySelect) {
        planetarySelect.addEventListener('change', (e) => {
            running = false;
            updatePlayButton();
            // Pass live ctx (with getters) so the rafCoordinator loop callback reads live running/engineMode (audit P0-5 fix, 2026-05-27).
            Scale4Controller.loadScenario(_makeCtx(), e.target.value);
        });
    }

    // Cosmic scenario selector
    const cosmicSelect = document.getElementById('cosmic-scenario-select');
    if (cosmicSelect) {
        cosmicSelect.addEventListener('change', (e) => {
            running = false;
            updatePlayButton();
            Scale5Controller.loadCosmicScenario(_makeCtx(), e.target.value);
        });
    }
    // Cosmic camera selector
    const cosmicCamera = document.getElementById('cosmic-camera-select');
    if (cosmicCamera) {
        cosmicCamera.addEventListener('change', (e) => {
            Scale5Controller.setCameraPreset(e.target.value);
        });
    }

    // Orbital cloud toggles (Scale 2 and Scale 3)
    const cloudToggle = document.getElementById('ae-show-clouds');
    if (cloudToggle) {
        cloudToggle.addEventListener('change', (e) => {
            Scale2Controller.setAEVisualToggle('showOrbitalClouds', e.target.checked);
        });
    }
    const molCloudToggle = document.getElementById('mol-show-clouds');
    if (molCloudToggle) {
        molCloudToggle.addEventListener('change', (e) => {
            Scale2Controller.setAEVisualToggle('showOrbitalClouds', e.target.checked);
        });
    }

    // ── Enhanced atom/molecule visual controls ──

    // Nucleus shells (strong force glow)
    const shellToggle = document.getElementById('ae-show-shells');
    if (shellToggle) {
        shellToggle.addEventListener('change', (e) => {
            Scale2Controller.setAEVisualToggle('showNucleusShells', e.target.checked);
            viewport.toggleNucleusShells(e.target.checked);
        });
    }

    const labelToggle = document.getElementById('ae-show-labels');
    if (labelToggle) {
        labelToggle.addEventListener('change', (e) => {
            Scale2Controller.setAEVisualToggle('showElementLabels', e.target.checked);
            viewport.toggleElementLabels(e.target.checked);
        });
    }

    // Shell boundary spheres
    const shellBoundsToggle = document.getElementById('ae-show-shell-bounds');
    if (shellBoundsToggle) {
        shellBoundsToggle.addEventListener('change', (e) => {
            Scale2Controller.setAEVisualToggle('showShellBounds', e.target.checked);
            viewport.toggleOrbitalShells(e.target.checked);
        });
    }

    // Orbital lobes
    const lobeToggle = document.getElementById('ae-show-lobes');
    if (lobeToggle) {
        lobeToggle.addEventListener('change', (e) => {
            Scale2Controller.setAEVisualToggle('showOrbitalLobes', e.target.checked);
            viewport.toggleOrbitalLobes(e.target.checked);
        });
    }

    // Bond style selector
    const bondStyleSelect = document.getElementById('bond-style-select');
    if (bondStyleSelect) {
        bondStyleSelect.addEventListener('change', (e) => {
            Scale2Controller.setAEVisualToggle('bondStyle', e.target.value);
            viewport.toggleBondCylinders(e.target.value === 'cylinders');
            viewport.toggleBondLines(e.target.value === 'lines');
        });
    }

    // Force arrow toggles
    const forceToggles = [
        ['ae-force-ionic', '_showAEForceIonic', 'toggleAEForceIonic'],
        ['ae-force-vdw', '_showAEForceVdw', 'toggleAEForceVdw'],
        ['ae-force-bond', '_showAEForceBond', 'toggleAEForceBond'],
        ['ae-force-hbond', '_showAEForceHBond', 'toggleAEForceHBond'],
        ['ae-force-angle', '_showAEForceAngle', 'toggleAEForceAngle'],
        ['ae-force-dipole', '_showAEForceDipole', 'toggleAEForceDipole'],
        ['ae-force-net', '_showAEForceNet', 'toggleAEForceNet'],
    ];
    for (const [id, flag, method] of forceToggles) {
        const btn = document.getElementById(id);
        if (btn) {
            btn.addEventListener('click', () => {
                const isActive = btn.classList.toggle('active');
                btn.setAttribute('aria-pressed', isActive ? 'true' : 'false');
                switch (flag) {
                    case '_showAEForceIonic': Scale2Controller.setAEVisualToggle('showAEForceIonic', isActive); break;
                    case '_showAEForceVdw': Scale2Controller.setAEVisualToggle('showAEForceVdw', isActive); break;
                    case '_showAEForceBond': Scale2Controller.setAEVisualToggle('showAEForceBond', isActive); break;
                    case '_showAEForceHBond': Scale2Controller.setAEVisualToggle('showAEForceHBond', isActive); break;
                    case '_showAEForceAngle': Scale2Controller.setAEVisualToggle('showAEForceAngle', isActive); break;
                    case '_showAEForceDipole': Scale2Controller.setAEVisualToggle('showAEForceDipole', isActive); break;
                    case '_showAEForceNet': Scale2Controller.setAEVisualToggle('showAEForceNet', isActive); break;
                }
                viewport[method](isActive);
            });
        }
    }
}

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
function wireControls() {
    // PE controls — every row carries the exact native registry toggle key.
    // This avoids a second hand-maintained setter map in the browser.
    for (const el of document.querySelectorAll('[data-pe-toggle]')) {
        el.addEventListener('change', () => {
            const accepted = bridge.peSetToggle?.(el.dataset.peToggle, el.checked);
            if (accepted === false) {
                el.checked = !!bridge.peGetToggle?.(el.dataset.peToggle);
            }
            Scale1Controller.markPhysicsProfileModified();
        });
    }

    for (const button of document.querySelectorAll('[data-pe-profile]')) {
        button.addEventListener('click', () => {
            Scale1Controller.applyPhysicsProfile(bridge, button.dataset.peProfile);
        });
    }

    // PE sliders
    const dtSlider = document.getElementById('pe-dt-slider');
    const dtValue = document.getElementById('pe-dt-value');
    if (dtSlider) {
        dtSlider.addEventListener('input', () => {
            const dt = parseFloat(dtSlider.value);
            dtValue.textContent = dt.toFixed(1);
            bridge.peSetDt(dt);
            Scale1Controller.markObservationDirty();
        });
    }

    const softSlider = document.getElementById('pe-soft-slider');
    const softValue = document.getElementById('pe-soft-value');
    if (softSlider) {
        softSlider.addEventListener('input', () => {
            const s = parseFloat(softSlider.value);
            softValue.textContent = s.toFixed(2);
            bridge.peSetSoftening(s);
            Scale1Controller.markObservationDirty();
            telemetryHub.setScale1Runtime({ softening: s });
        });
    }

    // Trajectory history is presentation-only and tick-aligned. The generic
    // data key keeps all visual history controls on one controller contract.
    for (const input of document.querySelectorAll('[data-pe-trail-setting]')) {
        input.addEventListener('input', () => {
            Scale1Controller.setTrailSettings({
                [input.dataset.peTrailSetting]: parseFloat(input.value),
            });
        });
    }
    for (const button of document.querySelectorAll('[data-pe-trail-mode]')) {
        button.addEventListener('click', () => {
            Scale1Controller.setTrailSettings({ renderMode: button.dataset.peTrailMode });
        });
    }
    document.getElementById('btn-pe-trail-reset')?.addEventListener('click', () => {
        Scale1Controller.resetTrailSettings();
    });

    document.getElementById('btn-pe-clear').addEventListener('click', () => {
        running = false;
        updatePlayButton();
        loadPEScenario(document.getElementById('pe-scenario-select').value);
    });

    const checkpointStatus = document.getElementById('pe-checkpoint-status');
    const setCheckpointStatus = (message, failed = false) => {
        if (checkpointStatus) {
            checkpointStatus.textContent = message;
            checkpointStatus.dataset.status = failed ? 'error' : 'ready';
        }
    };
    const afterCheckpointMutation = () => {
        Scale1Controller.markObservationDirty();
        telemetryHub.resetScale1?.();
    };
    document.getElementById('btn-pe-checkpoint-save')?.addEventListener('click', () => {
        try {
            const result = captureScale1Checkpoint(bridge);
            setCheckpointStatus(`Captured tick ${result.tick} · ${result.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    document.getElementById('btn-pe-checkpoint-restore')?.addEventListener('click', () => {
        try {
            running = false;
            updatePlayButton();
            const result = restoreSavedScale1Checkpoint(bridge);
            afterCheckpointMutation();
            setCheckpointStatus(`Restored tick ${result.tick} · ${result.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    document.getElementById('btn-pe-checkpoint-export')?.addEventListener('click', () => {
        try {
            const captured = captureScale1Checkpoint(bridge);
            const blob = new Blob([serializeScale1Checkpoint()], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const anchor = document.createElement('a');
            anchor.href = url;
            anchor.download = `ftd-scale1-tick-${captured.tick}.json`;
            anchor.click();
            setTimeout(() => URL.revokeObjectURL(url), 0);
            setCheckpointStatus(`Exported tick ${captured.tick} · ${captured.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    const checkpointFile = document.getElementById('pe-checkpoint-file');
    document.getElementById('btn-pe-checkpoint-import')?.addEventListener('click', () => {
        checkpointFile?.click();
    });
    checkpointFile?.addEventListener('change', async () => {
        const file = checkpointFile.files?.[0];
        if (!file) return;
        try {
            running = false;
            updatePlayButton();
            const result = importScale1Checkpoint(await file.text(), bridge);
            afterCheckpointMutation();
            setCheckpointStatus(`Imported tick ${result.tick} · ${result.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        } finally {
            checkpointFile.value = '';
        }
    });
    document.getElementById('btn-pe-replay-mark')?.addEventListener('click', () => {
        try {
            const result = markScale1ReplayStart(bridge);
            setCheckpointStatus(`Replay start marked at tick ${result.tick} · ${result.digest}`);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    document.getElementById('btn-pe-replay-verify')?.addEventListener('click', async () => {
        try {
            running = false;
            updatePlayButton();
            setCheckpointStatus('Replaying the marked segment…');
            const result = await verifyScale1Replay(bridge);
            afterCheckpointMutation();
            setCheckpointStatus(result.match
                ? `Replay matched ${result.ticks} ticks · ${result.actualDigest}`
                : `Replay mismatch · expected ${result.expectedDigest}, got ${result.actualDigest}`,
            !result.match);
        } catch (error) {
            setCheckpointStatus(error.message, true);
        }
    });
    const updateFieldBatteryStatus = () => {
        const snapshot = bridge.peGetFinitePortBatterySnapshot?.();
        const status = document.getElementById('pe-field-battery-status');
        if (status && snapshot) {
            status.textContent = `Layer ${snapshot.acceptedLayers}/${snapshot.capacity} · `
                + `E ${Number(snapshot.totalBookedEnergy).toPrecision(6)}`;
        }
        Scale1Controller.markObservationDirty();
    };
    document.getElementById('btn-pe-field-battery-step')?.addEventListener('click', () => {
        if (!bridge.peStepFinitePortBattery?.()) {
            showToast('Finite ready-port capacity is exhausted.', 'info');
        }
        updateFieldBatteryStatus();
    });
    document.getElementById('btn-pe-field-battery-reverse')?.addEventListener('click', () => {
        if (!bridge.peReverseFinitePortBattery?.()) {
            showToast('No accepted field layer is available to reverse.', 'info');
        }
        updateFieldBatteryStatus();
    });

    // AE controls — force & dynamics toggles
    for (const spec of AE_PHYSICS_SPECS) {
        const el = document.getElementById(spec.elementId);
        if (!el) continue;
        el.addEventListener('change', () => bridge[spec.setter]?.(el.checked));
    }

    // AE sliders
    const aeDtSlider = document.getElementById('ae-dt-slider');
    const aeDtValue = document.getElementById('ae-dt-value');
    if (aeDtSlider) {
        aeDtSlider.addEventListener('input', () => {
            const dt = parseFloat(aeDtSlider.value);
            aeDtValue.textContent = dt.toFixed(3);
            bridge.aeSetDt(dt);
        });
    }

    const aeSoftSlider = document.getElementById('ae-soft-slider');
    const aeSoftValue = document.getElementById('ae-soft-value');
    if (aeSoftSlider) {
        aeSoftSlider.addEventListener('input', () => {
            const s = parseFloat(aeSoftSlider.value);
            aeSoftValue.textContent = s.toFixed(2);
            bridge.aeSetSoftening(s);
        });
    }

    // Dynamic Scale-2 nuclear laboratory. These controls mutate the live
    // environment; they never reload or branch on the selected scenario.
    const nuclearPatch = (patch) => bridge.aeSetNuclearEnvironment?.(patch);
    const bindNuclearRange = (id, valueId, key, format) => {
        const input = document.getElementById(id);
        const value = document.getElementById(valueId);
        input?.addEventListener('input', () => {
            const numeric = Number(input.value);
            if (value) value.textContent = format(numeric);
            nuclearPatch({ [key]: numeric });
        });
    };
    bindNuclearRange('ae-nuclear-reactivity', 'ae-nuclear-reactivity-value', 'reactivityScale', v => v.toFixed(1));
    bindNuclearRange('ae-nuclear-collision-radius', 'ae-nuclear-collision-radius-value', 'collisionRadiusScale', v => `${v.toFixed(2)}×`);
    bindNuclearRange('ae-nuclear-transport-radius', 'ae-nuclear-transport-radius-value', 'transportRadius', v => `${v.toFixed(0)} lu`);
    bindNuclearRange('ae-nuclear-moderator', 'ae-nuclear-moderator-value', 'moderatorStrength', v => v.toFixed(2));
    bindNuclearRange('ae-nuclear-absorber', 'ae-nuclear-absorber-value', 'absorberStrength', v => v.toFixed(2));
    bindNuclearRange('ae-nuclear-source-rate', 'ae-nuclear-source-rate-value', 'sourceRate', v => `${v.toFixed(2)}/tick`);
    document.getElementById('ae-nuclear-boundary')?.addEventListener('change', event =>
        nuclearPatch({ boundaryMode: event.currentTarget.value }));
    document.getElementById('ae-nuclear-source-energy')?.addEventListener('change', event =>
        nuclearPatch({ sourceEnergyMeV: Number(event.currentTarget.value) }));
    document.getElementById('ae-nuclear-source-enabled')?.addEventListener('change', event =>
        nuclearPatch({ sourceEnabled: event.currentTarget.checked }));
    document.getElementById('ae-nuclear-channel')?.addEventListener('change', event => {
        const channel = event.currentTarget.value;
        if (!channel) {
            bridge.aeConfigureNuclearReaction?.('');
            return;
        }
        bridge.aeConfigureNuclearReaction?.({
            channel,
            mode: 'sandbox',
            eventLimit: 100000,
            seed: 0x5eed235,
        });
        const valueOf = id => Number(document.getElementById(id)?.value);
        nuclearPatch({
            reactivityScale: valueOf('ae-nuclear-reactivity'),
            collisionRadiusScale: valueOf('ae-nuclear-collision-radius'),
            transportRadius: valueOf('ae-nuclear-transport-radius'),
            boundaryMode: document.getElementById('ae-nuclear-boundary')?.value,
            moderatorStrength: valueOf('ae-nuclear-moderator'),
            absorberStrength: valueOf('ae-nuclear-absorber'),
            sourceRate: valueOf('ae-nuclear-source-rate'),
            sourceEnergyMeV: valueOf('ae-nuclear-source-energy'),
            sourceEnabled: !!document.getElementById('ae-nuclear-source-enabled')?.checked,
        });
    });
    for (const [id, kind] of [
        ['btn-ae-inject-neutron', 'neutron'],
        ['btn-ae-inject-dt', 'dt-pair'],
        ['btn-ae-inject-u235', 'u235'],
    ]) {
        document.getElementById(id)?.addEventListener('click', () => {
            if (bridge.aeInjectNuclearParticle?.(kind) === false) {
                showToast('Select a nuclear channel before injecting reactants.', 'info');
            }
        });
    }

    document.getElementById('btn-ae-clear').addEventListener('click', () => {
        running = false;
        updatePlayButton();
        loadAEScenario(document.getElementById('ae-scenario-select').value);
    });
}

// ── Viewport Toggle Wiring ───────────────────────────────────────────
function wireViewportToggles() {
    const setToggleState = (button, on) => {
        button.classList.toggle('active', on);
        button.setAttribute('aria-pressed', on ? 'true' : 'false');
    };

    // Universal toggles (visible on all scales)
    const axesBtn = document.getElementById('toggle-axes');
    if (axesBtn) {
        axesBtn.addEventListener('click', () => {
            const on = !axesBtn.classList.contains('active');
            setToggleState(axesBtn, on);
            viewport.toggleAxes(on);
        });
    }
    // Grid button also controls the wireframe (lattice boundary box) at Scale 0
    const gridBtn = document.getElementById('toggle-grid');
    if (gridBtn) {
        gridBtn.addEventListener('click', () => {
            const on = !gridBtn.classList.contains('active');
            setToggleState(gridBtn, on);
            viewport.toggleGrid(on);
            viewport.toggleWireframe(on);
        });
    }

    const orientationBtn = document.getElementById('toggle-boundary-orientation');
    if (orientationBtn) {
        orientationBtn.addEventListener('click', () => {
            const on = !orientationBtn.classList.contains('active');
            setToggleState(orientationBtn, on);
            viewport.toggleBoundaryOrientation(on);
        });
    }

    const clockBtn = document.getElementById('toggle-global-clock');
    if (clockBtn) {
        clockBtn.addEventListener('click', () => {
            const on = !clockBtn.classList.contains('active');
            setToggleState(clockBtn, on);
            viewport.toggleGlobalClock(on);
        });
    }

    // Camera preset buttons — Scale-0-specific camera viewpoints. Each
    // button snaps the orbit camera to a named direction; "Fit" zooms to
    // frame the active flux volume. Buttons hidden on non-lattice scales
    // via the .scale0-only class on their container.
    for (const btn of document.querySelectorAll('[data-cam-preset]')) {
        const preset = btn.getAttribute('data-cam-preset');
        btn.addEventListener('click', () => {
            if (!viewport) return;
            if (preset === 'fit') {
                viewport.zoomToFit?.();
            } else {
                viewport.setCameraPreset?.(preset);
            }
            // Transient visual pulse so the user sees the preset was applied,
            // without leaving any button stuck in an active state (these
            // are momentary actions, not persistent toggles).
            btn.classList.add('status-preset-flash');
            setTimeout(() => btn.classList.remove('status-preset-flash'), 260);
        });
    }


    // PE mode visual overlay toggles (delegated to Scale1Controller)
    const velBtn = document.getElementById('toggle-velocities');
    if (velBtn) velBtn.addEventListener('click', () => {
        velBtn.classList.toggle('active');
        const on = velBtn.classList.contains('active');
        velBtn.setAttribute('aria-pressed', on ? 'true' : 'false');
        Scale1Controller.setVelocities(on);
        viewport.toggleVelocityVectors(on);
    });

    const trailBtn = document.getElementById('toggle-trails');
    if (trailBtn) trailBtn.addEventListener('click', () => {
        trailBtn.classList.toggle('active');
        const trailOn = trailBtn.classList.contains('active');
        trailBtn.setAttribute('aria-pressed', trailOn ? 'true' : 'false');
        Scale1Controller.setTrails(trailOn);
        viewport.toggleTrails(trailOn);
    });

    // PE field overlay toggles (delegated to Scale1Controller)
    const peFieldToggles = [
        ['toggle-pe-efield', (on) => { Scale1Controller.setPEEField(on); viewport.togglePEStreamlines(on); }],
        ['toggle-pe-potential', (on) => {
            Scale1Controller.setPEPotential(on);
            const active = Scale1Controller.isPEFieldSurfaceActive();
            viewport.toggleFieldHeatmap(active); viewport.toggleFieldVectors(active);
        }],
        ['toggle-pe-field-battery', (on) => {
            Scale1Controller.setPEFieldBattery(on);
            const active = Scale1Controller.isPEFieldSurfaceActive();
            viewport.toggleFieldHeatmap(active); viewport.toggleFieldVectors(active);
        }],
        ['toggle-pe-gravity-field', (on) => { Scale1Controller.setPEGravField(on); viewport.toggleGravityVectors(on); }],
        ['toggle-pe-force-coulomb', (on) => { Scale1Controller.setPEForceCoulomb(on); viewport.togglePEForceCoulomb(on); }],
        ['toggle-pe-force-gravity', (on) => { Scale1Controller.setPEForceGravity(on); viewport.togglePEForceGravity(on); }],
        ['toggle-pe-force-lorentz', (on) => { Scale1Controller.setPEForceLorentz(on); viewport.togglePEForceLorentz(on); }],
        ['toggle-pe-force-exchange', (on) => { Scale1Controller.setPEForceExchange(on); viewport.togglePEForceExchange(on); }],
        ['toggle-pe-force-strong', (on) => { Scale1Controller.setPEForceStrong(on); viewport.togglePEForceStrong(on); }],
        ['toggle-pe-force-radiation', (on) => { Scale1Controller.setPEForceRadiation(on); viewport.togglePEForceRadiation(on); }],
        ['toggle-pe-force-magnetic-dipole', (on) => { Scale1Controller.setPEForceMagneticDipole(on); viewport.togglePEForceMagneticDipole(on); }],
        ['toggle-pe-force-spin-orbit', (on) => { Scale1Controller.setPEForceSpinOrbit(on); viewport.togglePEForceSpinOrbit(on); }],
        ['toggle-pe-force-net', (on) => { Scale1Controller.setPEForceNet(on); viewport.togglePEForceNet(on); }],
        ['toggle-pe-system', (on) => { Scale1Controller.setPESystem(on); viewport.togglePESystem(on); }],
        ['toggle-pe-admissibility', (on) => { Scale1Controller.setAdmissibilityRing(on); viewport.toggleAdmissibilityRings(on); }],
        ['toggle-pe-provenance', (on) => { Scale1Controller.setProvenanceLabel(on); viewport.toggleProvenanceLabels(on); }],
    ];
    for (const [id, handler] of peFieldToggles) {
        const btn = document.getElementById(id);
        if (btn) {
            btn.addEventListener('click', () => {
                btn.classList.toggle('active');
                const on = btn.classList.contains('active');
                btn.setAttribute('aria-pressed', on ? 'true' : 'false');
                handler(on);
            });
        }
    }

    // AE field overlay toggle
    const aeFieldBtn = document.getElementById('toggle-ae-field');
    if (aeFieldBtn) {
        aeFieldBtn.addEventListener('click', () => {
            aeFieldBtn.classList.toggle('active');
            const aeFieldOn = aeFieldBtn.classList.contains('active');
            aeFieldBtn.setAttribute('aria-pressed', aeFieldOn ? 'true' : 'false');
            Scale2Controller.setAEVisualToggle('showAEField', aeFieldOn);
            viewport.toggleFieldHeatmap(aeFieldOn);
            viewport.toggleFieldVectors(aeFieldOn);
        });
    }

    // AE kinetic/electrostatic structure overlays (Scale 2 deep pass):
    // velocity vectors, dipole arrows, dashed H-bond lines. Flags drive
    // per-frame updates in Scale2Controller.animateAE; the viewport toggle
    // controls layer visibility immediately.
    const aeStructureToggles = [
        ['toggle-ae-velocities', 'showAEVelocities', (on) => viewport.toggleVelocityVectors(on)],
        ['toggle-ae-dipoles', 'showAEDipoles', (on) => viewport.toggleAEDipoles(on)],
        ['toggle-ae-hbonds', 'showAEHBondLines', (on) => viewport.toggleHBondLines(on)],
        ['toggle-ae-nuclear-events', 'showAENuclearEvents', (on) => viewport.toggleNuclearEvents?.(on)],
        ['toggle-ae-radiation', 'showAERadiation', (on) => viewport.toggleNuclearRadiation?.(on)],
        ['toggle-ae-heat', 'showAEHeat', (on) => viewport.toggleNuclearHeat?.(on)],
        ['toggle-ae-nuclear-boundary', 'showAENuclearBoundary', (on) => viewport.toggleNuclearBoundary?.(on)],
    ];
    for (const [btnId, flagKey, vpToggle] of aeStructureToggles) {
        const btn = document.getElementById(btnId);
        if (btn) {
            btn.addEventListener('click', () => {
                const isOn = btn.classList.toggle('active');
                btn.setAttribute('aria-pressed', isOn ? 'true' : 'false');
                Scale2Controller.setAEVisualToggle(flagKey, isOn);
                vpToggle(isOn);
            });
        }
    }

    // Scale 3 bonds toggle
    const molBondBtn = document.getElementById('toggle-mol-bonds');
    if (molBondBtn) {
        molBondBtn.addEventListener('click', () => {
            molBondBtn.classList.toggle('active');
            _showBonds = molBondBtn.classList.contains('active');
            molBondBtn.setAttribute('aria-pressed', _showBonds ? 'true' : 'false');
            viewport.toggleBondLines(_showBonds);
        });
    }

}




// ── Keyboard Shortcuts ───────────────────────────────────────────────
// Keyboard shortcut handler — body extracted to app-wire/keyboard.js. This
// thin wrapper provides the live-state getters + mode-specific step/reload
// callbacks the extracted module needs.
function wireKeyboard() {
    wireKeyboardExternal({
        getEngineMode: () => engineMode,
        getBridge: () => bridge,
        pauseSimulation,
        togglePlay,
        stepScenario: () => {
            if (engineMode === 'atoms' || engineMode === 'molecules') {
                bridge.aeTick();
            } else if (engineMode === 'particles') {
                bridge.peTick();
            } else if (engineMode === 'cosmic') {
                Scale5Controller.step(_makeCtx());
            } else if (engineMode === 'planetary') {
                Scale4Controller.step();
            } else if (engineMode === 'meta') {
                // MetaUnit has no tick-based physics to step.
            } else {
                Scale0Controller.step(_makeCtx());
            }
        },
        reloadScenario: () => {
            if (engineMode === 'molecules') {
                loadMoleculeScenario(document.getElementById('mol-scenario-select').value);
            } else if (engineMode === 'atoms') {
                loadAEScenario(document.getElementById('ae-scenario-select').value);
            } else if (engineMode === 'particles') {
                loadPEScenario(document.getElementById('pe-scenario-select').value);
            } else if (engineMode === 'cosmic') {
                Scale5Controller.loadCosmicScenario(_makeCtx(), document.getElementById('cosmic-scenario-select')?.value || 'cosmic-galaxy');
            } else if (engineMode === 'planetary') {
                Scale4Controller.loadScenario(_makeCtx(), document.getElementById('planetary-scenario-select')?.value || 'planetary-solar');
            } else if (engineMode === 'meta') {
                Scale6Controller.loadScenario(_makeCtx());
            } else {
                Scale0Controller.reset(_makeCtx());
            }
        },
        Scale0Controller,
    });
}

// ── Settings Modal ──────────────────────────────────────────────────
{
    initSettingsModal();
    const root = document.documentElement;
    const modal = document.getElementById('settings-modal');
    const btnOpen = document.getElementById('btn-settings');
    const btnClose = document.getElementById('settings-close');
    const slider = document.getElementById('settings-ui-scale');
    const valDisplay = document.getElementById('settings-scale-val');
    const glassToggle = document.getElementById('settings-glass-enabled');
    const glassThicknessSlider = document.getElementById('settings-glass-thickness');
    const glassThicknessValue = document.getElementById('settings-glass-thickness-val');
    const glassControls = document.getElementById('settings-glass-controls');
    const btnReset = document.getElementById('settings-reset');
    const settingsButtons = Array.from(document.querySelectorAll('[data-setting][data-value]'));

    const DEFAULT_SETTINGS = Object.freeze({
        scale: 1.0,
        theme: 'default',
        glass: 'off',
        glassThickness: 16,
        density: 'comfortable',
        panelWidth: 'standard',
        tooltips: 'on',
        statusBar: 'shown',
    });

    const STORAGE_KEYS = Object.freeze({
        scale: 'ftd-ui-scale',
        theme: 'ftd-theme',
        glass: 'ftd-glassmorphism',
        glassThickness: 'ftd-glass-thickness',
        density: 'ftd-density',
        panelWidth: 'ftd-panel-width',
        tooltips: 'ftd-tooltips',
        statusBar: 'ftd-status-bar',
    });

    function setChoiceGroup(settingName, value) {
        settingsButtons.forEach((button) => {
            button.classList.toggle(
                'active',
                button.dataset.setting === settingName && button.dataset.value === value,
            );
        });
    }

    function persist(key, value) {
        try { localStorage.setItem(key, String(value)); } catch (e) { }
    }

    const GLASS_THICKNESS_MIN = 4;
    const GLASS_THICKNESS_MAX = 32;
    let glassThicknessFrame = 0;
    let pendingGlassThickness = null;

    function normalizeGlassThickness(value) {
        const parsed = Number(value);
        if (!Number.isFinite(parsed)) return DEFAULT_SETTINGS.glassThickness;
        return Math.min(GLASS_THICKNESS_MAX, Math.max(GLASS_THICKNESS_MIN, Math.round(parsed)));
    }

    function updateGlassThicknessDisplay(thickness) {
        if (glassThicknessSlider) glassThicknessSlider.value = String(thickness);
        if (glassThicknessValue) glassThicknessValue.textContent = `${thickness} px`;
    }

    function applyGlassThickness(value) {
        if (glassThicknessFrame) cancelAnimationFrame(glassThicknessFrame);
        glassThicknessFrame = 0;
        pendingGlassThickness = null;
        const thickness = normalizeGlassThickness(value);
        root.style.setProperty('--glass-thickness', `${thickness}px`);
        root.style.setProperty('--glass-blur-low', `${thickness / 2}px`);
        root.style.setProperty('--glass-blur-mid', `${thickness}px`);
        root.style.setProperty('--glass-blur-high', `${thickness * 1.5}px`);
        updateGlassThicknessDisplay(thickness);
        persist(STORAGE_KEYS.glassThickness, thickness);
    }

    function queueGlassThickness(value) {
        pendingGlassThickness = normalizeGlassThickness(value);
        updateGlassThicknessDisplay(pendingGlassThickness);
        if (glassThicknessFrame) return;
        glassThicknessFrame = requestAnimationFrame(() => {
            const thickness = pendingGlassThickness;
            glassThicknessFrame = 0;
            pendingGlassThickness = null;
            applyGlassThickness(thickness);
        });
    }

    function applyGlassMode(value) {
        const mode = value === 'on' ? 'on' : 'off';
        const enabled = mode === 'on';
        root.dataset.glass = mode;
        if (glassToggle) {
            glassToggle.checked = enabled;
            glassToggle.setAttribute('aria-checked', enabled ? 'true' : 'false');
        }
        if (glassThicknessSlider) glassThicknessSlider.disabled = !enabled;
        if (glassControls) {
            glassControls.classList.toggle('is-disabled', !enabled);
            glassControls.setAttribute('aria-disabled', enabled ? 'false' : 'true');
        }
        persist(STORAGE_KEYS.glass, mode);
    }

    // ── Scale ──
    function applyScale(s) {
        // Write the USER knob (--ui-scale-base). The effective --ui-scale is
        // derived in tokens.css (= base) and may be multiplied per-breakpoint
        // in responsive.css (mobile = base × 1.2) without losing this setting.
        root.style.setProperty('--ui-scale-base', s);
        if (slider) slider.value = s;
        if (valDisplay) valDisplay.textContent = Math.round(s * 100) + '%';
        document.querySelectorAll('.settings-preset').forEach(b => {
            b.classList.toggle('active', Math.abs(parseFloat(b.dataset.scale) - s) < 0.01);
        });
        if (root.dataset.statusBar === 'hidden') {
            root.style.setProperty('--status-bar-offset', '0px');
        } else {
            root.style.setProperty('--status-bar-offset', 'calc(28px * var(--ui-scale))');
        }
        persist(STORAGE_KEYS.scale, s);
        if (viewport && viewport.resize) setTimeout(() => viewport.resize(), 100);
    }

    // ── Theme ──
    let themeReleaseRaf = 0;
    function applyTheme(name) {
        root.dataset.themeChanging = 'true';
        if (themeReleaseRaf) cancelAnimationFrame(themeReleaseRaf);
        if (name === 'default') {
            root.removeAttribute('data-theme');
        } else {
            root.setAttribute('data-theme', name);
        }
        document.querySelectorAll('.theme-swatch').forEach(sw => {
            const on = sw.dataset.theme === name;
            sw.classList.toggle('active', on);
            sw.setAttribute('aria-checked', on ? 'true' : 'false');
            sw.tabIndex = on ? 0 : -1;
        });
        persist(STORAGE_KEYS.theme, name);
        themeReleaseRaf = requestAnimationFrame(() => {
            themeReleaseRaf = requestAnimationFrame(() => {
                delete root.dataset.themeChanging;
                themeReleaseRaf = 0;
            });
        });
    }



    function applyDensity(mode) {
        root.dataset.density = mode;
        setChoiceGroup('density', mode);
        persist(STORAGE_KEYS.density, mode);
    }

    function applyPanelWidth(mode) {
        root.dataset.panelWidth = mode;
        setChoiceGroup('panel-width', mode);
        if (viewport && viewport.resize) setTimeout(() => viewport.resize(), 80);
        persist(STORAGE_KEYS.panelWidth, mode);
    }

    function applyTooltipMode(mode) {
        root.dataset.tooltips = mode;
        if (mode === 'off') document.getElementById('ui-tooltip')?.setAttribute('hidden', '');
        setChoiceGroup('tooltips', mode);
        persist(STORAGE_KEYS.tooltips, mode);
    }

    function applyStatusBar(mode) {
        root.dataset.statusBar = mode;
        root.style.setProperty(
            '--status-bar-offset',
            mode === 'hidden' ? '0px' : 'calc(28px * var(--ui-scale))',
        );
        if (viewport && viewport.resize) setTimeout(() => viewport.resize(), 80);
        setChoiceGroup('status-bar', mode);
        persist(STORAGE_KEYS.statusBar, mode);
    }

    // ── Load saved settings ──
    try {
        const savedScale = localStorage.getItem(STORAGE_KEYS.scale);
        applyScale(savedScale ? parseFloat(savedScale) : DEFAULT_SETTINGS.scale);
        applyTheme(localStorage.getItem(STORAGE_KEYS.theme) || DEFAULT_SETTINGS.theme);
        const savedGlassThickness = localStorage.getItem(STORAGE_KEYS.glassThickness);
        applyGlassThickness(savedGlassThickness ?? DEFAULT_SETTINGS.glassThickness);
        applyGlassMode(localStorage.getItem(STORAGE_KEYS.glass) || DEFAULT_SETTINGS.glass);
        applyDensity(localStorage.getItem(STORAGE_KEYS.density) || DEFAULT_SETTINGS.density);
        applyPanelWidth(localStorage.getItem(STORAGE_KEYS.panelWidth) || DEFAULT_SETTINGS.panelWidth);
        applyTooltipMode(localStorage.getItem(STORAGE_KEYS.tooltips) || DEFAULT_SETTINGS.tooltips);
        applyStatusBar(localStorage.getItem(STORAGE_KEYS.statusBar) || DEFAULT_SETTINGS.statusBar);
    } catch (e) { }

    // ── Modal open/close ──
    if (btnOpen && modal) btnOpen.addEventListener('click', () => { modal.classList.add('visible'); });
    if (btnClose && modal) btnClose.addEventListener('click', () => { modal.classList.remove('visible'); });
    if (modal) modal.addEventListener('click', (e) => { if (e.target === modal) modal.classList.remove('visible'); });
    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && modal?.classList.contains('visible')) modal.classList.remove('visible');
    });

    // ── Scale controls ──
    if (slider) slider.addEventListener('input', () => applyScale(parseFloat(slider.value)));
    document.querySelectorAll('.settings-preset').forEach(btn => {
        btn.addEventListener('click', () => applyScale(parseFloat(btn.dataset.scale)));
    });

    // ── Theme controls ──
    document.querySelectorAll('.theme-swatch').forEach(sw => {
        sw.addEventListener('click', () => applyTheme(sw.dataset.theme));
        sw.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') {
                e.preventDefault();
                applyTheme(sw.dataset.theme);
                return;
            }
            if (e.key !== 'ArrowRight' && e.key !== 'ArrowLeft'
                && e.key !== 'ArrowDown' && e.key !== 'ArrowUp') return;
            e.preventDefault();
            const all = [...document.querySelectorAll('.theme-swatch')];
            const i = all.indexOf(sw);
            if (i < 0) return;
            const dir = (e.key === 'ArrowRight' || e.key === 'ArrowDown') ? 1 : -1;
            const next = all[(i + dir + all.length) % all.length];
            applyTheme(next.dataset.theme);
            next.focus();
        });
    });

    // ── Glass controls ──
    if (glassToggle) {
        glassToggle.addEventListener('change', () => applyGlassMode(glassToggle.checked ? 'on' : 'off'));
    }
    if (glassThicknessSlider) {
        glassThicknessSlider.addEventListener('input', () => queueGlassThickness(glassThicknessSlider.value));
        // Pointer release / keyboard commit flushes synchronously so an
        // immediate reload cannot strand the final value in a queued frame.
        glassThicknessSlider.addEventListener('change', () => applyGlassThickness(glassThicknessSlider.value));
    }

    // ── Other preference controls ──
    settingsButtons.forEach((button) => {
        button.addEventListener('click', () => {
            const setting = button.dataset.setting;
            const value = button.dataset.value;
            if (!setting || !value) return;
            if (setting === 'density') applyDensity(value);
            else if (setting === 'panel-width') applyPanelWidth(value);
            else if (setting === 'tooltips') applyTooltipMode(value);
            else if (setting === 'status-bar') applyStatusBar(value);
        });
    });

    // ── Reset all ──
    if (btnReset) {
        btnReset.addEventListener('click', () => {
            applyScale(DEFAULT_SETTINGS.scale);
            applyTheme(DEFAULT_SETTINGS.theme);
            applyGlassThickness(DEFAULT_SETTINGS.glassThickness);
            applyGlassMode(DEFAULT_SETTINGS.glass);
            applyDensity(DEFAULT_SETTINGS.density);
            applyPanelWidth(DEFAULT_SETTINGS.panelWidth);
            applyTooltipMode(DEFAULT_SETTINGS.tooltips);
            applyStatusBar(DEFAULT_SETTINGS.statusBar);
        });
    }
}

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

function switchEngineMode(mode) {
    // 1. Uniform Lifecycle: Teardown previous controller
    const prevController = CONTROLLERS[engineMode];
    if (prevController && typeof prevController.destroy === 'function') {
        prevController.destroy(_makeCtx());
    }

    engineMode = mode;

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
    // Scales 0-3 (lattice/particles/atoms/molecules) all share the
    // app-level `bridge`; restore it here so that returning from a
    // self-bridged scale (Scale 4 planetary / Scale 5 cosmic, which swap
    // in their own bridge during loadScenario) does not leave the
    // inspector querying a stale planetary/cosmic backend. Scales 4/5
    // overwrite this with their own bridge later in their loaders, so the
    // guard avoids clobbering them.
    if (mode === 'lattice' || mode === 'particles'
        || mode === 'atoms' || mode === 'molecules') {
        // Worker-hosted Scale-0 scenarios run a SEPARATE RenderBridge inside the
        // worker that is never ticked on the main thread. Pointing the inspector
        // at the app-level direct `bridge` in that case shows frozen tick-0
        // numbers. Route through the active physics owner instead: when a worker
        // proxy (fluxMock with isWorker) is live, use it. With M7's
        // null-returning proxy reads, the inspector now honestly shows "no data"
        // on the worker path rather than frozen-wrong values (intended
        // degradation; true worker-backed inspect is a Phase-2 follow-up).
        const fluxMock = Scale0Controller.getFluxMock();
        const activeBridge = (fluxMock && fluxMock.isWorker) ? fluxMock : bridge;
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
    if (!select) return;

    // Clear existing options
    select.innerHTML = '';

    // Add molecule categories from data-driven library
    for (const cat of getCategories()) {
        const mols = getMoleculesByCategory(cat.id);
        if (!mols.length) continue;
        const group = document.createElement('optgroup');
        group.label = cat.label;
        for (const mol of mols) {
            const opt = document.createElement('option');
            opt.value = `mol-${mol.id}`;
            const cleanFormula = mol.formula.replace(/<[^>]+>/g, '');
            opt.textContent = `${cleanFormula} ${mol.name}`;
            group.appendChild(opt);
        }
        select.appendChild(group);
    }

    // Add special entries
    const specialGroup = document.createElement('optgroup');
    specialGroup.label = 'Special';
    const crystalOpt = document.createElement('option');
    crystalOpt.value = 'mol-crystal';
    crystalOpt.textContent = 'NaCl Crystal (3\u00d73\u00d73)';
    specialGroup.appendChild(crystalOpt);
    const customOpt = document.createElement('option');
    customOpt.value = 'mol-custom';
    customOpt.textContent = 'Custom';
    specialGroup.appendChild(customOpt);
    select.appendChild(specialGroup);

    // Default to H2
    select.value = 'mol-h2';
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

// ── Launch ───────────────────────────────────────────────────────────
init().catch(err => {
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
