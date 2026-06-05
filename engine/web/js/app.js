/**
 * @file app.js
 * @brief FTD Web Dashboard — Main Application Controller
 *
 * [EXTENDED] Initializes all subsystems, manages the frame loop,
 * and wires up UI controls to the simulation bridge.
 */

import { createBridge, MockBridge } from './bridge-init.js';
// PhysicsHarness factory — lazily attached per-bridge by panels and
// overlays that need the canonical read/write surface.
import { getPhysicsHarness } from './physics/index.js';
import { tryNativeBridge } from './ws-bridge.js';
import { Viewport } from './viewport.js';
import { DiagnosticsPanel } from './diagnostics.js';
import { FluxEnergyChart, ParticleChart } from './charts.js';
import { telemetryHub } from './telemetry-hub.js';
import { createInspectorAppRuntime } from './inspector/app-runtime.js';
import { initZoo, setEngineMode as setZooMode } from './zoo.js';
import { getElement } from './elements.js';
import { getCategories, getMoleculesByCategory, getMolecule, loadMolecule } from './molecules.js';
import { atomicEnergy } from './atomic-energy.js';
import { formatEnergy } from './units.js';
import { debugLog } from './core/log.js';

// ── Scale Controllers (extracted from inline code) ─────────────────
import * as Scale0Controller from './scales/scale0/controller.js';
import * as Scale1Controller from './scales/scale1/controller.js';
import * as Scale2Controller from './scales/scale2/controller.js';
import * as Scale3Controller from './scales/scale3/controller.js';
// ── Phase 1-3: Ontic Observatory, Physics Fidelity, Aggregation Bridge
import * as Scale4Controller from './scales/scale4/controller.js';
import * as Scale5Controller from './scales/scale5/controller.js';
import * as Scale6Controller from './scales/scale6/controller.js';
import { OnticObservatory } from './ontic-observatory.js';
// renderEnergyLevels, renderCrossSections, renderDecayRates, renderFcCard,
// renderObserverCard, renderOnticHierarchy, renderInfoDynamics moved to
// ui/app-ontic.js (Wave 2 ticket 7).
import { TICK_PHASES, K_B, K_GENESIS, C_SPEED } from './constants.js';
// ALPHA, G_STAR, VARPI, X_PLUS, X_MINUS, ONTIC_LAYERS, ONTIC_TOTAL_CONSTANTS
// now imported directly by ui/app-ontic.js.
import { AggregateDetector, ScaleBridgeVisualizer, EmergenceMonitor } from './aggregation-bridge.js';
import { createOnticPanel } from './ui/app-ontic.js';
import { BackgroundManager } from './backgrounds.js';
import { PETelemetryPanel } from './pe-telemetry.js';
import { initVerifyPanel } from './verify-panel/component.js';
import { AppShell } from './ui/shell/app-shell.js';
import { initDiagnosticsPanel, initChartsPanel, initLagrangianPanel, initScenePanel, initTelemetryGridPanel } from './ui/panels/index.js';
import { floatingWindowManager } from './ui/components/floating-window/component.js';
import { initFluxSlicePanel } from './scales/scale0/ui/overlays/flux-slice-panel.js';
import { initP1ObservablesPanel } from './scales/scale0/ui/overlays/p1-observables-panel.js';
import { initConservationMicropanel } from './scales/scale0/ui/overlays/conservation-micropanel.js';
import { initSpectrumPanel } from './scales/scale0/ui/overlays/spectrum-panel.js';
import { initSettingsModal } from './ui/components/settings-modal/component.js';
// Keyboard shortcut handler extracted per refactoring-analyst RF-9 (partial).
import { wireKeyboard as wireKeyboardExternal } from './app-wire/keyboard.js';

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
let diagnostics = null;
let diagnosticsPanel = null;
let chartsPanel = null;
let telemetryGridPanel = null;
let lagrangianPanel = null;
// Legacy chart instances (scale1/scale2 still push into these ring buffers).
let fluxEnergyChart = null;
let particleChart = null;
let peTelemetry = null;

// Two-tier pause system:
//   `running`         — GLOBAL pause. When false, the entire RAF body is skipped:
//                       no physics, no rendering work, no flux mock animation.
//                       The single source of truth for "is anything moving?".
//   `globalTick`      — wall-clock frame counter that advances every animate()
//                       call where `running` is true. Independent of the engine
//                       tick (which throttles, can advance multiple per frame).
let running = false;
let globalTick = 0;
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
//   'meta'          (Scale 12) — 3^3 existential unit (MetaUnit)
// Transitions: switchEngineMode() is the SOLE entry point for mode changes.
let engineMode = 'lattice';
let _aeInitialEnergy = null; // for AE energy drift tracking
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

// Verification Lab (replaces the legacy Quantum Lab panel)
let _verifLabComponent = null;

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
        get diagnostics() { return diagnostics; },
        get diagnosticsPanel() { return diagnosticsPanel; },
        get chartsPanel() { return chartsPanel; },
        get telemetryGridPanel() { return telemetryGridPanel; },
        get lagrangianPanel() { return lagrangianPanel; },
        get fluxEnergyChart() { return fluxEnergyChart; },
        get particleChart() { return particleChart; },
        get peTelemetry() { return peTelemetry; },
        get telemetryHub() { return telemetryHub; },
        get running() { return running; },
        set running(v) { running = v; },
        get globalTick() { return globalTick; },
        get ticksPerFrame() { return ticksPerFrame; },
        get engineMode() { return engineMode; },
        get activeTab() { return activeTab; },
        get frameCount() { return frameCount; },
        get dom() { return _dom; },
        updateOnticPanel:   () => onticPanel?.updateOnticPanel(),
        updateHierarchyPanel: () => onticPanel?.updateHierarchyPanel(),
        resetAllVisualState: _resetAllVisualState,
        _resetAllVisualState,
        updatePlayButton,
        pauseSimulation,
        applyTicksPerFrameFromSlider,
        applyBoundaryShape,
        applyReflectiveBoundary,
        clearCharts,
    };
    return _ctxSingleton;
}

function pauseSimulation() {
    running = false;
    updatePlayButton();
}

function sliderValueToSpeed(s, modeValue = engineMode) {
    if (modeValue === 'planetary') return Math.pow(10, (s - 50) / 25);
    if (s <= 50) return Math.pow(10, (s - 50) / 25);
    return 1.0 + (s - 50) / 50;
}

function speedLabel(tpf) {
    if (tpf < 0.1) return tpf.toFixed(2);
    if (tpf < 1) return tpf.toFixed(1);
    return tpf.toFixed(1);
}

function applyTicksPerFrameFromSlider(value) {
    const slider = document.getElementById('ticks-per-frame');
    const display = document.getElementById('tpf-display');
    if (slider) slider.value = String(value);
    ticksPerFrame = sliderValueToSpeed(parseFloat(value), engineMode);
    _tickAccumulator = 0;
    if (display) display.textContent = speedLabel(ticksPerFrame);
}

function applyBoundaryShape(shape) {
    const boundarySelect = document.getElementById('boundary-select');
    if (boundarySelect) boundarySelect.value = shape;
    viewport?.setBoundaryShape?.(shape);
    if (bridge?.setBoundaryShape) bridge.setBoundaryShape(shape);
    const fm = Scale0Controller.getFluxMock();
    if (fm?.setBoundaryShape) fm.setBoundaryShape(shape);
    Scale0Controller.setLatticeNeedsUpload();
}

function applyReflectiveBoundary(on) {
    const reflectiveBtn = document.getElementById('toggle-reflective');
    if (reflectiveBtn) reflectiveBtn.classList.toggle('active', !!on);
    if (bridge?.setReflectiveBoundary) bridge.setReflectiveBoundary(on);
    const fm = Scale0Controller.getFluxMock();
    if (fm?.setReflectiveBoundary) fm.setReflectiveBoundary(on);
    if (viewport?.setReflectiveBoundary) viewport.setReflectiveBoundary(on);
    Scale0Controller.setLatticeNeedsUpload();
}

/**
 * Master visual state reset — called by EVERY scenario loader to prevent
 * state leakage between scenarios. Resets:
 *   - Scale 0 field visualization flags + buttons
 *   - Scale 1 PE overlay flags + buttons + dynamics buttons
 *   - Scale 1 velocity/trail flags + buttons
 *   - Scale 2 AE field overlay button
 *   - Charts, Lagrangian, diagnostics sparklines
 *   - PE telemetry, trail history, field grid cache
 *   - Viewport overlays (trails, element labels, field visualizations)
 */

// Reset simulation data caches (always on scenario change) but PRESERVE visual toggles
function _resetSimCaches() {
    clearCharts();
    if (peTelemetry) peTelemetry.clear();
    Scale1Controller.resetScale1({ viewport }); // clears trail history + cloud caches
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
    Scale1Controller.resetScale1({ viewport });
    for (const id of [
        'toggle-pe-efield', 'toggle-pe-potential',
        'toggle-pe-gravity-field', 'toggle-pe-forces',
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
        viewport.toggleParticleForces(false);
        viewport.toggleVelocityVectors(false);
        viewport.toggleTrails(false);
    }

    // PE dynamics buttons (gravity/damping are OFF by default)
    for (const id of ['toggle-pe-gravity', 'toggle-pe-damping']) {
        const btn = document.getElementById(id);
        if (btn) btn.classList.remove('active');
    }

    // ── Scale 2/3: delegated to Scale2Controller ──
    Scale2Controller.resetScale2({ viewport });

    // Reset AE toggle buttons (DOM shared across scales, kept here)
    const aeFieldBtn2 = document.getElementById('toggle-ae-field');
    if (aeFieldBtn2) aeFieldBtn2.classList.remove('active');
    for (const id of [
        'ae-show-shells', 'ae-show-shell-bounds', 'ae-show-lobes',
        'ae-force-ionic', 'ae-force-vdw', 'ae-force-bond', 'ae-force-net',
    ]) {
        const el = document.getElementById(id);
        if (el) {
            if (el.type === 'checkbox') el.checked = (id === 'ae-show-shells');
            else el.classList.remove('active');
        }
    }
    const bondSelect = document.getElementById('bond-style-select');
    if (bondSelect) bondSelect.value = 'cylinders';

    // ── AE energy drift reference ──
    _aeInitialEnergy = null;
}

// Phase 1-3 state
let observatory = null;
let aggregateDetector = null;
let scaleBridgeViz = null;
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
function showToast(msg, severity = 'info') {
    const container = document.getElementById('toast-container');
    if (!container) return;
    const toast = document.createElement('div');
    toast.className = `toast toast-${severity}`;
    const span = document.createElement('span');
    span.textContent = msg;
    const btn = document.createElement('button');
    btn.textContent = '\u00d7';
    btn.addEventListener('click', () => toast.remove());
    toast.appendChild(span);
    toast.appendChild(btn);
    container.appendChild(toast);
    setTimeout(() => { if (toast.parentElement) toast.remove(); }, 8000);
}

// ── Loading Progress ─────────────────────────────────────────────────
function _loadProgress(pct, msg) {
    const bar = document.getElementById('load-bar');
    const status = document.getElementById('load-status');
    if (bar) bar.style.width = pct + '%';
    if (status) status.textContent = msg;
}

// ── Initialization ───────────────────────────────────────────────────
// Safety timeout: dismiss loading overlay after 8000ms even if init() hangs
// (e.g. WASM compilation stalls, WebGL context fails). This prevents the user
// from being stuck on a blank screen — the dashboard will render in mock mode.
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
    const latticeSize = parseInt(document.getElementById('lattice-size').value);
    const engineEl = document.getElementById('status-engine');
    const computeEl = document.getElementById('status-compute');

    const urlParams = new URLSearchParams(window.location.search);
    const forceMock = urlParams.get('engine') === 'mock';

    if (!forceMock) {
        // 1. Try native GPU engine
        debugLog('[init] Trying native GPU engine on ws://127.0.0.1:9100...');
        try {
            bridge = await tryNativeBridge(latticeSize);
        } catch (e) {
            console.warn('[init] Native GPU bridge error:', e);
            bridge = null;
        }
    } else {
        debugLog('[init] Skipping native GPU: forceMock active');
        bridge = null;
    }

    debugLog('[init] Native bridge result:', bridge ? 'connected' : 'unavailable');
    if (bridge && bridge.ready) {
        _loadProgress(30, 'GPU engine connected');
        engineEl.textContent = 'Native Engine';
        engineEl.style.color = '#c084fc';
        computeEl.textContent = bridge.isNativeGPU ? 'GPU' : 'CPU';
        computeEl.style.color = bridge.isNativeGPU ? '#4ade80' : '#60a5fa';
        computeEl.title = bridge.isNativeGPU
            ? 'Connected to native GPU engine (CUDA)'
            : 'Connected to native CPU engine';
        showToast('Native GPU engine connected — full CUDA acceleration active.', 'success');
    } else {
        if (forceMock) {
            _loadProgress(30, 'Mock engine (forced)');
            bridge = new MockBridge(latticeSize);
            engineEl.textContent = 'Mock Engine';
            engineEl.style.color = '#fbbf24';
            computeEl.textContent = 'CPU';
            computeEl.style.color = '#667';
            showToast('Running in forced Mock mode.', 'warning');
        } else {
            _loadProgress(20, 'Compiling WASM engine...');
            bridge = await createBridge(latticeSize);
            if (bridge.isWasm && bridge.ready) {
                _loadProgress(30, 'WASM engine ready');
                engineEl.textContent = 'WASM Engine';
                engineEl.style.color = '#4ade80';
                computeEl.textContent = 'CPU';
                computeEl.style.color = '#60a5fa';
                computeEl.title = 'Browser WASM runs on CPU. Start ws_server.exe for GPU.';
            } else {
                _loadProgress(30, 'Mock engine (fallback)');
                engineEl.textContent = 'Mock Engine';
                engineEl.style.color = '#fbbf24';
                computeEl.textContent = 'CPU';
                computeEl.style.color = '#667';
                showToast('No engine available — running in Mock mode. Start ws_server.exe for GPU.', 'warning');
            }
        }
    }

    // Create 3D viewport
    _loadProgress(40, 'Building 3D viewport...');
    const viewportContainer = document.getElementById('viewport');
    viewport = new Viewport(viewportContainer);
    viewport.setLatticeSize(latticeSize);

    _loadProgress(50, 'Creating panels...');
    // Initialize panel component wrappers (Phase 4)
    diagnosticsPanel = initDiagnosticsPanel();
    chartsPanel = initChartsPanel();
    telemetryGridPanel = initTelemetryGridPanel();
    lagrangianPanel = initLagrangianPanel();
    initFluxSlicePanel();
    initP1ObservablesPanel();
    initConservationMicropanel();
    initSpectrumPanel();
    // Scene panel — curated render controls (FOV / exposure / bloom / fog / ...).
    // Scales 0–3 only (gated by panel-registry); unmounted cleanly when
    // the user switches to a separate-renderer scale like 4/5/12.
    initScenePanel({
        panelArea: document.getElementById('panel-area'),
        viewport,
        backgroundManager: bgManager,
    });
    diagnostics = new DiagnosticsPanel();
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
    peTelemetry = new PETelemetryPanel();

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

    // Build element scenario dropdown (molecules are Scale 3 only)
    buildElementScenarios();
    buildScale3MoleculeDropdown();

    _loadProgress(60, 'Initializing observatory...');
    observatory = new OnticObservatory();
    aggregateDetector = new AggregateDetector();
    scaleBridgeViz = new ScaleBridgeVisualizer();
    emergenceMonitor = new EmergenceMonitor(500);
    onticPanel.initOnticPhysicsHierarchy();

    _loadProgress(70, 'Wiring controls...');
    // Scrub bar owns the playback buttons (play/local/step/reset/speed).
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
    wireVerificationLab();
    wireKeyboard();

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

    // Load default scenario (flux-pulse: pure substrate wave propagation)
    Scale0Controller.loadScenario(_makeCtx(), 'flux-pulse');

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

    // Global tick = wall-clock frames since global play resumed. Always advances
    // when global is running, regardless of scenario pause or per-scale tick
    // throttling. Feeds the flux-slice panel via ctx.globalTick (it has no
    // status-bar readout — the bar shows physical time "T:" only).
    if (running) globalTick++;

    if (engineMode === 'meta') {
        Scale6Controller.updateMeta(_makeCtx(), 1 / 60);
    } else if (engineMode === 'cosmic') {
        Scale5Controller.animateCosmic(_makeCtx());
    } else if (engineMode === 'atoms' || engineMode === 'molecules') {
        animateAE(now);
    } else if (engineMode === 'particles') {
        animatePE(now);
    } else if (engineMode === 'planetary') {
        // Handled via the rafCoordinator 'scale4-planetary-loop'
        // subscription created in Scale4Controller.loadScenario.
    } else {
        Scale0Controller.animateLattice(_makeCtx());
    }

    // Animate environment background
    if (bgManager) bgManager.update(1 / 60);

    inspectorRuntime?.updateFloatingPanels();

    // Update active docked panels or floated windows in real-time. Scale 0
    // owns its active telemetry panel updates on the same cadence as its
    // telemetry collection; app.js still services floated panels and panels in
    // the other engines.
    if (_shouldAppUpdatePanel('telemetry-grid')) {
        telemetryGridPanel?.update();
    }
    if (_shouldAppUpdatePanel('charts')) {
        chartsPanel?.update();
    }
    if (_shouldAppUpdatePanel('diagnostics')) {
        diagnosticsPanel?.update();
    }
    if (_shouldAppUpdatePanel('lagrangian')) {
        lagrangianPanel?.update();
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

function _shouldAppUpdatePanel(panelId) {
    const visible = activeTab === panelId || floatingWindowManager.has(panelId);
    if (!visible) return false;
    const scale0Owned = engineMode === 'lattice' &&
        (panelId === 'charts' || panelId === 'diagnostics' || panelId === 'lagrangian') &&
        activeTab === panelId;
    return !scale0Owned;
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
    fluxEnergyChart: null, particleChart: null, peTelemetry: null,
    activeTab: null, frameCount: 0, dom: _dom, now: 0,
    updateOnticPanel:   () => onticPanel?.updateOnticPanel(),
    updateHierarchyPanel: () => onticPanel?.updateHierarchyPanel(),
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
    c.peTelemetry = peTelemetry;
    c.activeTab = activeTab;
    c.frameCount = frameCount;
    c.dom = _dom;
    c.now = now;
    return c;
}

const _scale2Ctx = {
    bridge: null, viewport: null, running: false,
    ticksPerFrame: 1, inspector: null,
    fluxEnergyChart: null, particleChart: null,
    activeTab: null, frameCount: 0, dom: _dom, now: 0,
    updatePlayButton,
    updateOnticPanel:   () => onticPanel?.updateOnticPanel(),
    updateHierarchyPanel: () => onticPanel?.updateHierarchyPanel(),
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
        running = false;
        updatePlayButton();
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
            Scale6Controller.step(_makeCtx());
        } else {
            Scale0Controller.step(_makeCtx());
        }
    });

    // Reset
    document.getElementById('btn-reset').addEventListener('click', () => {
        running = false;
        updatePlayButton();
        if (engineMode === 'cosmic') {
            Scale5Controller.loadCosmicScenario(_makeCtx(), document.getElementById('cosmic-scenario-select')?.value || 'cosmic-galaxy');
        } else if (engineMode === 'meta') {
            Scale6Controller.loadMetaScenario(_makeCtx());
        } else if (engineMode === 'molecules') {
            loadMoleculeScenario(document.getElementById('mol-scenario-select').value);
        } else if (engineMode === 'atoms') {
            loadAEScenario(document.getElementById('ae-scenario-select').value);
        } else if (engineMode === 'particles') {
            loadPEScenario(document.getElementById('pe-scenario-select').value);
        } else {
            Scale0Controller.reset(_makeCtx());
        }
    });

    const slider = document.getElementById('ticks-per-frame');
    applyTicksPerFrameFromSlider(slider.value);
    slider.addEventListener('input', () => applyTicksPerFrameFromSlider(slider.value));

    // Engine mode selector (Scale 0 / Scale 1)
    document.getElementById('engine-mode').addEventListener('change', (e) => {
        running = false;
        updatePlayButton();
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
        ['ae-force-net', '_showAEForceNet', 'toggleAEForceNet'],
    ];
    for (const [id, flag, method] of forceToggles) {
        const btn = document.getElementById(id);
        if (btn) {
            btn.addEventListener('click', () => {
                const isActive = btn.classList.toggle('active');
                switch (flag) {
                    case '_showAEForceIonic': Scale2Controller.setAEVisualToggle('showAEForceIonic', isActive); break;
                    case '_showAEForceVdw': Scale2Controller.setAEVisualToggle('showAEForceVdw', isActive); break;
                    case '_showAEForceBond': Scale2Controller.setAEVisualToggle('showAEForceBond', isActive); break;
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
            diagnostics.drawSparklines();
            if (peTelemetry) peTelemetry.drawCharts();
        } else if (target === 'physics') {
            onticPanel?.refreshPhysicsPanel();
        } else if (target === 'hierarchy') {
            onticPanel?.updateHierarchyPanel();
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
    // PE controls — force & dynamics toggles
    const peToggleMap = {
        'pe-coulomb': (v) => bridge.peSetCoulomb(v),
        'pe-gravity': (v) => bridge.peSetGravity(v),
        'pe-damping': (v) => bridge.peSetDamping(v),
        'pe-lorentz-p': (v) => bridge.peSetLorentz(v),
        'pe-exchange': (v) => bridge.peSetExchange(v),
        'pe-strong': (v) => bridge.peSetStrong(v),
        'pe-magnetic-dipole': (v) => bridge.peSetMagneticDipole(v),
        'pe-spin-orbit': (v) => bridge.peSetSpinOrbit(v),
        'pe-radiation': (v) => bridge.peSetRadiation(v),
        'pe-relativistic': (v) => bridge.peSetRelativistic(v),
    };
    for (const [elId, setter] of Object.entries(peToggleMap)) {
        const el = document.getElementById(elId);
        if (el) {
            el.addEventListener('change', () => {
                const checked = el.checked;
                setter(checked);
                if (elId === 'pe-gravity') {
                    const btn = document.getElementById('toggle-pe-gravity');
                    if (btn) {
                        if (checked) btn.classList.add('active');
                        else btn.classList.remove('active');
                    }
                } else if (elId === 'pe-damping') {
                    const btn = document.getElementById('toggle-pe-damping');
                    if (btn) {
                        if (checked) btn.classList.add('active');
                        else btn.classList.remove('active');
                    }
                }
            });
        }
    }

    // PE sliders
    const dtSlider = document.getElementById('pe-dt-slider');
    const dtValue = document.getElementById('pe-dt-value');
    if (dtSlider) {
        dtSlider.addEventListener('input', () => {
            const dt = parseFloat(dtSlider.value);
            dtValue.textContent = dt.toFixed(1);
            bridge.peSetDt(dt);
        });
    }

    const softSlider = document.getElementById('pe-soft-slider');
    const softValue = document.getElementById('pe-soft-value');
    if (softSlider) {
        softSlider.addEventListener('input', () => {
            const s = parseFloat(softSlider.value);
            softValue.textContent = s.toFixed(2);
            bridge.peSetSoftening(s);
        });
    }

    document.getElementById('btn-pe-clear').addEventListener('click', () => {
        running = false;
        updatePlayButton();
        loadPEScenario(document.getElementById('pe-scenario-select').value);
    });

    // AE controls — force & dynamics toggles
    const aeToggleMap = {
        'ae-ionic': (v) => bridge.aeSetIonic(v),
        'ae-vdw': (v) => bridge.aeSetVdw(v),
        'ae-bonds-force': (v) => bridge.aeSetBondsForce(v),
        'ae-bonding': (v) => bridge.aeSetBonding(v),
        'ae-damping': (v) => bridge.aeSetDamping(v),
        'ae-speed-limit': (v) => bridge.aeSetSpeedLimit(v),
    };
    for (const [elId, setter] of Object.entries(aeToggleMap)) {
        const el = document.getElementById(elId);
        if (el) el.addEventListener('change', () => setter(el.checked));
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

    document.getElementById('btn-ae-clear').addEventListener('click', () => {
        running = false;
        updatePlayButton();
        loadAEScenario(document.getElementById('ae-scenario-select').value);
    });
}

// ── Viewport Toggle Wiring ───────────────────────────────────────────
function wireViewportToggles() {
    // Universal toggles (visible on all scales)
    const axesBtn = document.getElementById('toggle-axes');
    if (axesBtn) {
        axesBtn.addEventListener('click', () => {
            axesBtn.classList.toggle('active');
            viewport.toggleAxes(axesBtn.classList.contains('active'));
        });
    }
    // Grid button also controls the wireframe (lattice boundary box) at Scale 0
    const gridBtn = document.getElementById('toggle-grid');
    if (gridBtn) {
        gridBtn.addEventListener('click', () => {
            gridBtn.classList.toggle('active');
            const on = gridBtn.classList.contains('active');
            viewport.toggleGrid(on);
            viewport.toggleWireframe(on);
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
            btn.classList.add('vcp-preset-flash');
            setTimeout(() => btn.classList.remove('vcp-preset-flash'), 260);
        });
    }


    // PE mode visual overlay toggles (delegated to Scale1Controller)
    const velBtn = document.getElementById('toggle-velocities');
    if (velBtn) velBtn.addEventListener('click', () => {
        velBtn.classList.toggle('active');
        const on = velBtn.classList.contains('active');
        Scale1Controller.setVelocities(on);
        viewport.toggleVelocityVectors(on);
    });

    const trailBtn = document.getElementById('toggle-trails');
    if (trailBtn) trailBtn.addEventListener('click', () => {
        trailBtn.classList.toggle('active');
        const trailOn = trailBtn.classList.contains('active');
        Scale1Controller.setTrails(trailOn);
        viewport.toggleTrails(trailOn);
    });

    // PE field overlay toggles (delegated to Scale1Controller)
    const peFieldToggles = [
        ['toggle-pe-efield', (on) => { Scale1Controller.setPEEField(on); viewport.togglePEStreamlines(on); }],
        ['toggle-pe-potential', (on) => { Scale1Controller.setPEPotential(on); viewport.toggleFieldHeatmap(on); viewport.toggleFieldVectors(on); }],
        ['toggle-pe-gravity-field', (on) => { Scale1Controller.setPEGravField(on); viewport.toggleGravityVectors(on); }],
        ['toggle-pe-forces', (on) => { Scale1Controller.setPEForces(on); viewport.toggleParticleForces(on); }],
    ];
    for (const [id, handler] of peFieldToggles) {
        const btn = document.getElementById(id);
        if (btn) {
            btn.addEventListener('click', () => {
                btn.classList.toggle('active');
                handler(btn.classList.contains('active'));
            });
        }
    }

    // PE dynamics toggles (affect simulation behavior)
    const peGravBtn = document.getElementById('toggle-pe-gravity');
    if (peGravBtn) {
        peGravBtn.addEventListener('click', () => {
            peGravBtn.classList.toggle('active');
            const on = peGravBtn.classList.contains('active');
            bridge.peSetGravity(on);
            // Sync sidebar checkbox if it exists
            const cb = document.getElementById('pe-gravity');
            if (cb) cb.checked = on;
        });
    }

    const peDampBtn = document.getElementById('toggle-pe-damping');
    if (peDampBtn) {
        peDampBtn.addEventListener('click', () => {
            peDampBtn.classList.toggle('active');
            const on = peDampBtn.classList.contains('active');
            bridge.peSetDamping(on);
            // Sync sidebar checkbox if it exists
            const cb = document.getElementById('pe-damping');
            if (cb) cb.checked = on;
        });
    }

    // AE field overlay toggle
    const aeFieldBtn = document.getElementById('toggle-ae-field');
    if (aeFieldBtn) {
        aeFieldBtn.addEventListener('click', () => {
            aeFieldBtn.classList.toggle('active');
            const aeFieldOn = aeFieldBtn.classList.contains('active');
            Scale2Controller.setAEVisualToggle('showAEField', aeFieldOn);
            viewport.toggleFieldHeatmap(aeFieldOn);
            viewport.toggleFieldVectors(aeFieldOn);
        });
    }

    // Scale 3 bonds toggle
    const molBondBtn = document.getElementById('toggle-mol-bonds');
    if (molBondBtn) {
        molBondBtn.addEventListener('click', () => {
            molBondBtn.classList.toggle('active');
            _showBonds = molBondBtn.classList.contains('active');
            viewport.toggleBondLines(_showBonds);
        });
    }

}

// ── Verification Lab Wiring (replaces legacy Quantum Lab) ─────────

/** Programmatically switch to the Verify tab. */
function _switchToVerifyTab() {
    if (appShell) appShell.activatePanel('verification-lab');
    else {
        const tab = document.querySelector('#tab-bar .tab[data-panel="verification-lab"]');
        if (tab) tab.click();
    }
}

/** Initialise the Verify evidence-scoreboard panel (see js/verify-panel/). */
function wireVerificationLab() {
    const panelArea = document.getElementById('panel-area');
    if (!panelArea) return;
    if (_verifLabComponent) return;
    _verifLabComponent = initVerifyPanel({ panelArea });
}


// ── Keyboard Shortcuts ───────────────────────────────────────────────
// Keyboard shortcut handler — body extracted to app-wire/keyboard.js. This
// thin wrapper provides the live-state getters + mode-specific step/reload
// callbacks the extracted module needs.
function wireKeyboard() {
    wireKeyboardExternal({
        getEngineMode: () => engineMode,
        getBridge: () => bridge,
        setRunning: (v) => { running = v; },
        updatePlayButton,
        togglePlay,
        stepScenario: () => {
            if (engineMode === 'atoms' || engineMode === 'molecules') {
                bridge.aeTick();
            } else if (engineMode === 'particles') {
                bridge.peTick();
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
    const btnReset = document.getElementById('settings-reset');
    const settingsButtons = Array.from(document.querySelectorAll('[data-setting][data-value]'));

    const DEFAULT_SETTINGS = Object.freeze({
        scale: 1.0,
        theme: 'default',
        motion: 'system',
        density: 'comfortable',
        panelWidth: 'standard',
        tooltips: 'on',
        statusBar: 'shown',
    });

    const STORAGE_KEYS = Object.freeze({
        scale: 'ftd-ui-scale',
        theme: 'ftd-theme',
        motion: 'ftd-motion',
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
    function applyTheme(name) {
        if (name === 'default') {
            root.removeAttribute('data-theme');
        } else {
            root.setAttribute('data-theme', name);
        }
        document.querySelectorAll('.theme-swatch').forEach(sw => {
            sw.classList.toggle('active', sw.dataset.theme === name);
        });
        persist(STORAGE_KEYS.theme, name);
    }

    function applyMotion(mode) {
        if (mode === 'system') {
            root.removeAttribute('data-motion');
        } else {
            root.dataset.motion = mode;
        }
        setChoiceGroup('motion', mode);
        persist(STORAGE_KEYS.motion, mode);
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
        applyMotion(localStorage.getItem(STORAGE_KEYS.motion) || DEFAULT_SETTINGS.motion);
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
    });

    // ── Other preference controls ──
    settingsButtons.forEach((button) => {
        button.addEventListener('click', () => {
            const setting = button.dataset.setting;
            const value = button.dataset.value;
            if (!setting || !value) return;
            if (setting === 'motion') applyMotion(value);
            else if (setting === 'density') applyDensity(value);
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
            applyMotion(DEFAULT_SETTINGS.motion);
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
    const scaleIndex = { lattice: '0', particles: '1', atoms: '2', molecules: '3', planetary: '4', cosmic: '5', meta: '12' }[mode];
    if (appShell) appShell.setActiveScale(scaleIndex);
    else app.setAttribute('data-active-scale', scaleIndex);

    // Keep mode-dependent inspector, viewport, and zoo state in sync.
    inspectorRuntime?.syncMode(mode);

    // Re-point the inspector at the active scale's bridge (audit P1-1).
    // Scales 0-3 (lattice/particles/atoms/molecules) all share the
    // app-level `bridge`; restore it here so that returning from a
    // self-bridged scale (Scale 4 planetary / Scale 5 cosmic, which swap
    // in their own bridge during loadScenario) does not leave the
    // inspector querying a stale planetary/cosmic backend. Scales 4/5/6
    // overwrite this with their own bridge later in their loaders, so the
    // guard avoids clobbering them.
    if (mode === 'lattice' || mode === 'particles'
        || mode === 'atoms' || mode === 'molecules') {
        inspectorRuntime?.setBridge(bridge);
    }

    const tpfSlider = document.getElementById('ticks-per-frame');
    if (tpfSlider) applyTicksPerFrameFromSlider(tpfSlider.value);

    // Disable universal grid / axes for planetary physics which has local overlays
    if (mode === 'planetary') {
        viewport.toggleGrid(false);
        viewport.toggleAxes(false);
        const gridBtn = document.getElementById('toggle-grid');
        if (gridBtn) gridBtn.classList.remove('active');
        const axesBtn = document.getElementById('toggle-axes');
        if (axesBtn) axesBtn.classList.remove('active');
    }

    // 2. Uniform Lifecycle: Mount the next controller
    const nextController = CONTROLLERS[mode];
    if (nextController && typeof nextController.mount === 'function') {
        nextController.mount(_makeCtx());
    }

    if (mode === 'lattice') {
        const scenario = document.getElementById('scenario-select')?.value || 'flux-pulse';
        Scale0Controller.loadScenario(_makeCtx(), scenario);
    } else if (mode === 'particles') {
        loadPEScenario(document.getElementById('pe-scenario-select')?.value || 'pe-hydrogen');
    } else if (mode === 'atoms') {
        loadAEScenario(document.getElementById('ae-scenario-select')?.value || 'ae-crystal');
    } else if (mode === 'molecules') {
        loadMoleculeScenario(document.getElementById('mol-scenario-select')?.value || 'mol-water');
    } else if (mode === 'planetary') {
        Scale4Controller.loadScenario(_makeCtx(), document.getElementById('planetary-scenario-select')?.value || 'planetary-solar');
    } else if (mode === 'cosmic') {
        Scale5Controller.loadCosmicScenario(_makeCtx(), document.getElementById('cosmic-scenario-select')?.value || 'cosmic-galaxy');
    } else if (mode === 'meta') {
        Scale6Controller.loadMetaScenario(_makeCtx());
    }

    Scale0Controller.setLatticeNeedsUpload();
    frameCount = 0;
}


function loadPEScenario(name) {
    Scale1Controller.loadPEScenario({ bridge, viewport, resetAllVisualState: _resetAllVisualState, inspector }, name);
}


// ── Atom Engine Scenarios ────────────────────────────────────────────
function loadAEScenario(name) {
    Scale2Controller.loadAEScenario({ bridge, viewport, inspector, resetAllVisualState: _resetAllVisualState }, name);
}


// ── Build Element Scenarios in Dropdown ──────────────────────────────
function buildElementScenarios() {
    const select = document.getElementById('ae-scenario-select');
    if (!select) return;

    const periods = [
        { label: 'Period 1', start: 1, end: 2 },
        { label: 'Period 2', start: 3, end: 10 },
        { label: 'Period 3', start: 11, end: 18 },
        { label: 'Period 4', start: 19, end: 36 },
        { label: 'Period 5', start: 37, end: 54 },
        { label: 'Period 6', start: 55, end: 86 },
        { label: 'Period 7', start: 87, end: 118 },
    ];

    // Insert element optgroups before the Special optgroup
    const specialGroup = select.querySelector('optgroup[label="Special"]');

    for (const p of periods) {
        const group = document.createElement('optgroup');
        group.label = p.label;

        for (let Z = p.start; Z <= p.end; Z++) {
            const el = getElement(Z);
            if (!el) continue;
            const opt = document.createElement('option');
            opt.value = `ae-el-${Z}`;
            opt.textContent = `${Z} ${el.symbol} \u2014 ${el.name}`;
            group.appendChild(opt);
        }

        select.insertBefore(group, specialGroup);
    }

    // Default to Hydrogen
    select.value = 'ae-el-1';
}

// AE toggle helpers (_syncAEParamsFromUI, _resetAETogglesToDefaults, _aeSetPhase3) moved to Scale2Controller
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
            opt.textContent = `${mol.formula} ${mol.name}`;
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
    running = !running;
    updatePlayButton();
}

function updatePlayButton() {
    const btn = document.getElementById('btn-play');
    if (!btn) return;
    btn.innerHTML = running ? '&#9208;' : '&#9654;'; // ⏸ / ▶
    btn.dataset.paused = running ? 'false' : 'true';
}

function clearCharts() {
    // Reset the hub's ring buffers for Scale 0 — charts share these buffers, so
    // clearing at the hub level is sufficient; uPlot instances redraw from the
    // cleared buffers on the next update().
    telemetryHub.resetScale(0);
    if (diagnostics) diagnostics.clear();
}

// ── Phase 1-3: Ontic / Physics / Hierarchy ────────
// Moved to ui/app-ontic.js as Wave 2 ticket 7 of the large-file refactor.
// Call via onticPanel.initOnticPhysicsHierarchy / updateOnticPanel /
// updateHierarchyPanel / refreshPhysicsPanel / getOnticDiagnostics /
// getRawDiagnostics / renderOnticChainSummary. See
// docs/SPEC_REFACTOR_LARGE_FILES.md §4.

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
