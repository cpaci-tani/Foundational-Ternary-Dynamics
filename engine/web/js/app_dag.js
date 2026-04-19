/**
 * @file app_dag.js
 * @brief FTD Web Dashboard — Main Application Controller
 *
 * [EXTENDED] Initializes all subsystems, manages the frame loop,
 * and wires up UI controls to the simulation bridge.
 */

import { createBridge, MockBridge } from './wasm-bridge-dag.js';
import { tryNativeBridge } from './ws-bridge.js';
import { Viewport } from './viewport.js';
import { DiagnosticsPanel, Sparkline } from './diagnostics.js';
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
import * as Scale11Controller from './scales/scale11/controller.js';
import { OnticObservatory, renderFcCard, renderObserverCard, renderHierarchyTower as renderOnticHierarchy, renderInfoDynamics } from './ontic-observatory.js';
import { renderEnergyLevels } from './spectroscopy.js';
import { renderCrossSections } from './cross-sections.js';
import { renderDecayRates } from './decay-rates.js';
import { ONTIC_LAYERS, ONTIC_TOTAL_CONSTANTS, TICK_PHASES, ALPHA, K_B, K_GENESIS, G_N, G_STAR, VARPI, X_PLUS, X_MINUS, COS2_THETA_C, C_SPEED } from './constants.js';
// K_C, Y_REAL, Y_IMAG, THETA_C_DEG, C_MANDELBROT moved to Scale11Controller
import { AggregateDetector, ScaleBridgeVisualizer, EmergenceMonitor, renderAggregationTower, renderScaleBridge, renderEmergenceMonitor } from './aggregation-bridge.js';
import { BackgroundManager } from './backgrounds.js';
import { PETelemetryPanel } from './pe-telemetry.js';
// ConsciousnessEngine moved to Scale11Controller
import { addInfoTooltips } from './consciousness-pedagogy.js';
// SCALE0_TOGGLES/SCALE2_TOGGLES/SCALE0_SCENARIO_OVERRIDES/LIGHT_SCENARIO_OVERRIDES
// are now imported by the owning scale controllers (scale0 via ui/controls/wire.js).
// CS_SCENARIO_DESCRIPTIONS moved to Scale11Controller
import { initVerifyPanel } from './verify-panel/component.js';
import { AppShell } from './ui/shell/app-shell.js';
import { initDiagnosticsPanel, initChartsPanel, initLagrangianPanel, initConsciousnessPanel } from './ui/panels/index.js';
import { initSettingsModal } from './ui/components/settings-modal/component.js';

debugLog('[FTD] App version 20260318a loaded (cache-busted)');

// ── Application State ────────────────────────────────────────────────
let _initialized = false;
let bridge = null;
// _savedBridge moved to Scale11Controller (bridge save/restore for consciousness mode)
// DEBUG: expose bridge globally for console inspection
Object.defineProperty(window, '_ftdBridge', { get() { return bridge; }, configurable: true });
let viewport = null;
let appShell = null;
let inspector = null;
let inspectorRuntime = null;
let diagnostics = null;
let diagnosticsPanel = null;
let chartsPanel = null;
let lagrangianPanel = null;
// Legacy chart instances (scale1/scale2 still push into these ring buffers).
let fluxEnergyChart = null;
let particleChart = null;
let peTelemetry = null;

// Two-tier pause system:
//   `running`         — GLOBAL pause. When false, the entire RAF body is skipped:
//                       no physics, no rendering work, no flux mock animation.
//                       The single source of truth for "is anything moving?".
//   `scenarioRunning` — SCENARIO pause. When false but `running` is true, the
//                       scenario tick (mainScale0.tickScale0 / peTick / aeTick)
//                       is skipped, but the flux mock continues animating and
//                       overlays/render continue updating. Lets the user freeze
//                       scenario-specific dynamics while watching residual field
//                       motion. Cannot be ON when `running` is OFF.
//   `globalTick`      — wall-clock frame counter that advances every animate()
//                       call where `running` is true. Independent of scenario
//                       ticks (which throttle, can advance multiple per frame).
let running = false;
let scenarioRunning = true;
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
//   'consciousness' (Scale 11) — flux-driven hologram visualization
//   'meta'          (Scale 12) — 3^3 existential unit (MetaUnit)
// Transitions: switchEngineMode() is the SOLE entry point for mode changes.
let engineMode = 'lattice';
// _csEngine, _csPedagogy, _csScenarioMeta, _mandelbrotZ_* moved to Scale11Controller
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
// getFieldState() / setFieldToggle() API. The _fluxMock instance below
// is the ONLY piece of Scale 0 state that app_dag.js still owns; it is
// used by the lattice-scenario wiring at ~line 2250 below.
let _fluxMock = null;           // MockBridge for Scale 0 flux visualization fallback

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
// Scale0Controller. The app_dag.js copy was never called and referenced
// module-local flags that have since moved to the controller.

/**
 * Build a shared context object for scale controllers.
 * Uses getters/setters so controllers read/write the live module-level
 * variables (running, ticksPerFrame, engineMode) rather than snapshots.
 */
function _makeCtx() {
    return {
        get bridge() { return bridge; },
        get viewport() { return viewport; },
        get appShell() { return appShell; },
        get inspector() { return inspector; },
        get diagnostics() { return diagnostics; },
        get diagnosticsPanel() { return diagnosticsPanel; },
        get chartsPanel() { return chartsPanel; },
        get lagrangianPanel() { return lagrangianPanel; },
        get fluxEnergyChart() { return fluxEnergyChart; },
        get particleChart() { return particleChart; },
        get peTelemetry() { return peTelemetry; },
        get telemetryHub() { return telemetryHub; },
        get running() { return running; },
        set running(v) { running = v; },
        // Scenario pause is a sub-state of global pause. When global is off,
        // scenarioRunning effectively reads as false even if its raw value is true.
        get scenarioRunning() { return running && scenarioRunning; },
        set scenarioRunning(v) { scenarioRunning = !!v; updateLocalPlayButton(); },
        get globalTick() { return globalTick; },
        get ticksPerFrame() { return ticksPerFrame; },
        get engineMode() { return engineMode; },
        get activeTab() { return activeTab; },
        get frameCount() { return frameCount; },
        get dom() { return _dom; },
        updateOnticPanel,
        updateHierarchyPanel,
        resetAllVisualState: _resetAllVisualState,
        _resetAllVisualState,
        updatePlayButton,
        pauseSimulation,
        applyTicksPerFrameFromSlider,
        applyBoundaryShape,
        applyReflectiveBoundary,
        clearCharts,
    };
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
// wireConsciousnessSubTabs moved to Scale11Controller.wireSubTabs()

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

// ── Cached DOM Elements (populated in init()) ──────────────────────
// Avoids repeated getElementById() calls in 60fps animation loops.
const _dom = {
    statusTick: null, statusPtime: null, statusParticles: null,
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
    _dom.statusTick = document.getElementById('status-tick');
    _dom.statusGlobalTick = document.getElementById('status-global-tick');
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

    // 1. Try native GPU engine
    debugLog('[init] Trying native GPU engine on ws://localhost:9100...');
    try {
        bridge = await tryNativeBridge(latticeSize);
    } catch (e) {
        console.warn('[init] Native GPU bridge error:', e);
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

    // Create 3D viewport
    _loadProgress(40, 'Building 3D viewport...');
    const viewportContainer = document.getElementById('viewport');
    viewport = new Viewport(viewportContainer);
    viewport.setLatticeSize(latticeSize);

    _loadProgress(50, 'Creating panels...');
    // Initialize panel component wrappers (Phase 4)
    diagnosticsPanel = initDiagnosticsPanel();
    chartsPanel = initChartsPanel();
    lagrangianPanel = initLagrangianPanel();
    initConsciousnessPanel();
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

    // Populate constants table from WASM if available
    populateConstants();

    // Build element scenario dropdown (molecules are Scale 3 only)
    buildElementScenarios();
    buildScale3MoleculeDropdown();

    _loadProgress(60, 'Initializing observatory...');
    observatory = new OnticObservatory();
    aggregateDetector = new AggregateDetector();
    scaleBridgeViz = new ScaleBridgeVisualizer();
    emergenceMonitor = new EmergenceMonitor(500);
    initOnticPhysicsHierarchy();

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
// NOTE: 'planetary' mode is a no-op here; it runs via setInterval
// inside Scale4Controller.loadScenario. All other modes (including
// 'cosmic' after Phase B.1) drive physics + render from this rAF loop.
function animate(now) {
    requestAnimationFrame(animate);

    // Global tick = wall-clock frames since global play resumed. Always advances
    // when global is running, regardless of scenario pause or per-scale tick
    // throttling. Updated to the status bar at the same cadence as FPS.
    if (running) globalTick++;

    if (engineMode === 'meta') {
        Scale6Controller.updateMeta(_makeCtx(), 1 / 60);
    } else if (engineMode === 'cosmic') {
        Scale5Controller.animateCosmic(_makeCtx());
    } else if (engineMode === 'consciousness') {
        Scale11Controller.animateConsciousness(_makeScale11Ctx(), now);
    } else if (engineMode === 'atoms' || engineMode === 'molecules') {
        animateAE(now);
    } else if (engineMode === 'particles') {
        animatePE(now);
    } else if (engineMode === 'planetary') {
        // Handled via _planetaryInterval in loadPlanetaryScenario
    } else {
        Scale0Controller.animateLattice(_makeCtx());
    }

    // Animate environment background
    if (bgManager) bgManager.update(1 / 60);

    inspectorRuntime?.updateFloatingPanels();

    // FPS counter
    frameCount++;
    if (now - lastFpsTime >= 1000) {
        fpsDisplay = frameCount;
        frameCount = 0;
        lastFpsTime = now;
        if (_dom.statusFps) _dom.statusFps.textContent = fpsDisplay;
        // Global tick refreshed at FPS cadence (no need to write to DOM 60×/s).
        if (_dom.statusGlobalTick) _dom.statusGlobalTick.textContent = globalTick;
    }
}

// animateLattice -- REMOVED: delegated to Scale0Controller.animateLattice(ctx)
// See engine/web/js/scales/scale0/controller.js for the extracted code.

// ── Scale 1/2/3 Context Builders ────────────────────────────────────
function _buildScale1Ctx(now) {
    return {
        bridge, viewport, running,
        // Effective scenarioRunning: false whenever global is paused.
        scenarioRunning: running && scenarioRunning,
        ticksPerFrame, inspector,
        fluxEnergyChart, particleChart, peTelemetry,
        activeTab, frameCount, dom: _dom, now,
        updateOnticPanel, updateHierarchyPanel,
    };
}

function _buildScale2Ctx(now) {
    return {
        bridge, viewport, running,
        scenarioRunning: running && scenarioRunning,
        ticksPerFrame, inspector,
        fluxEnergyChart, particleChart,
        activeTab, frameCount, dom: _dom, now,
        updatePlayButton, updateOnticPanel, updateHierarchyPanel,
        resetAllVisualState: _resetAllVisualState,
        setRunning: (v) => { running = v; updateLocalPlayButton(); },
        engineMode,
    };
}

function _makeScale11Ctx() {
    return {
        get bridge() { return bridge; },
        set bridge(v) { bridge = v; },
        get viewport() { return viewport; },
        get running() { return running; },
        set running(v) { running = v; },
        get ticksPerFrame() { return ticksPerFrame; },
        get engineMode() { return engineMode; },
        MockBridge,
        _resetAllVisualState,
        addInfoTooltips,
        updatePlayButton,
    };
}

function animatePE(now) {
    Scale1Controller.animatePE(_buildScale1Ctx(now));
}

function animateAE(now) {
    Scale2Controller.animateAE(_buildScale2Ctx(now));
}

// (Inline animatePE, animateAE, updateAtomicEnergyDisplay, formatSI removed
//  -- now in Scale1Controller and Scale2Controller)

// ── Constants Table ─────────────────────────────────────────────────
function populateConstants() {
    const c = bridge.getConstants();
    if (!c) return;

    const set = (id, val, decimals = 7) => {
        const el = document.getElementById(id);
        if (el) el.textContent = typeof val === 'number' ? val.toFixed(decimals) : val;
    };

    set('const-gstar', c.G_STAR);
    set('const-alpha-inv', c.ALPHA_INV);
    set('const-alpha', c.ALPHA);
    set('const-kb', c.K_B);
    set('const-gn', c.G_N);
    set('const-gc', c.G_C);
    set('const-nc', c.N_C, 0);
    set('const-neff', c.N_EFF, 0);
}

// ── Toolbar Wiring ───────────────────────────────────────────────────
function wireToolbar() {
    // Play/Pause
    document.getElementById('btn-play').addEventListener('click', togglePlay);
    // Scenario Play/Pause — independent of global pause (button is disabled
    // when global is off; see updateLocalPlayButton).
    const scenBtn = document.getElementById('btn-local-play');
    if (scenBtn) scenBtn.addEventListener('click', toggleScenarioPlay);
    // Initial sync (so disabled state shows on load before user clicks anything).
    updateLocalPlayButton();

    // Step
    document.getElementById('btn-step').addEventListener('click', () => {
        running = false;
        updatePlayButton();
        if (engineMode === 'consciousness') {
            Scale11Controller.step(_makeScale11Ctx());
        } else if (engineMode === 'atoms' || engineMode === 'molecules') {
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
        } else if (engineMode === 'consciousness') {
            loadConsciousnessScenario(document.getElementById('cs-scenario-select').value);
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
            Scale4Controller.loadScenario({ viewport, inspector, running, ticksPerFrame, engineMode }, e.target.value);
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

    // Consciousness scenario selector
    const csSelect = document.getElementById('cs-scenario-select');
    if (csSelect) {
        csSelect.addEventListener('change', (e) => {
            running = false;
            updatePlayButton();
            loadConsciousnessScenario(e.target.value);
        });
    }

    // Consciousness figure type selector
    const csFigure = document.getElementById('cs-figure-select');
    if (csFigure) {
        csFigure.addEventListener('change', (e) => {
            Scale11Controller.setFigureType(e.target.value);
        });
    }

    // Consciousness audio toggle
    const csAudio = document.getElementById('cs-audio');
    if (csAudio) {
        csAudio.addEventListener('change', (e) => {
            if (e.target.checked) {
                const scenarioName = Scale11Controller.getScenarioMeta()?.name || 'cs-custom';
                Scale11Controller.enableAudio(scenarioName);
            } else {
                Scale11Controller.disableAudio();
            }
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
        } else if (target === 'lagrangian') {
            lagrangianPanel?.update();
        } else if (target === 'diagnostics') {
            diagnosticsPanel?.update();
            diagnostics.drawSparklines();
            if (peTelemetry) peTelemetry.drawCharts();
        } else if (target === 'physics') {
            const energyEl = document.getElementById('physics-energy-levels');
            if (energyEl) renderEnergyLevels(_physicsZ, energyEl);
        } else if (target === 'hierarchy') {
            updateHierarchyPanel();
        }
    };

    appShell?.bindPanelDock({
        activeTab,
        onTabActivated: handlePanelActivated,
    });
}

// ── Consciousness Mode (Scale 11) — delegated to Scale11Controller ───

function loadConsciousnessScenario(name) {
    Scale11Controller.loadConsciousnessScenario(_makeScale11Ctx(), name);
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
        if (el) el.addEventListener('change', () => setter(el.checked));
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
            const cb = document.getElementById('pe-gravity-check');
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
            const cb = document.getElementById('pe-damping-check');
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
function wireKeyboard() {
    document.addEventListener('keydown', (e) => {
        // Ignore if typing in an input
        if (e.target.tagName === 'INPUT' || e.target.tagName === 'SELECT') return;

        switch (e.key.toLowerCase()) {
            case ' ':
                e.preventDefault();
                // Shift+Space toggles scenario pause; plain Space toggles global.
                if (e.shiftKey) toggleScenarioPlay();
                else togglePlay();
                break;
            case 's':
                running = false;
                updatePlayButton();
                if (engineMode === 'consciousness') {
                    // Consciousness ticks both flux and CS engine
                    bridge.tick();
                    { const fm = Scale0Controller.getFluxMock(); if (fm) fm.tick(); }
                } else if (engineMode === 'atoms' || engineMode === 'molecules') {
                    bridge.aeTick();
                } else if (engineMode === 'particles') {
                    bridge.peTick();
                } else {
                    Scale0Controller.step(_makeCtx());
                }
                break;
            case 'r':
                running = false;
                updatePlayButton();
                if (engineMode === 'consciousness') {
                    loadConsciousnessScenario(document.getElementById('cs-scenario-select').value);
                } else if (engineMode === 'molecules') {
                    loadMoleculeScenario(document.getElementById('mol-scenario-select').value);
                } else if (engineMode === 'atoms') {
                    loadAEScenario(document.getElementById('ae-scenario-select').value);
                } else if (engineMode === 'particles') {
                    loadPEScenario(document.getElementById('pe-scenario-select').value);
                } else {
                    Scale0Controller.reset(_makeCtx());
                }
                break;
        }

        // Field visualization shortcuts (1-8) — Scale 0 only
        if (engineMode === 'lattice') {
            if (Scale0Controller.handleShortcutKey(e.key)) e.preventDefault();
        }
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
        root.style.setProperty('--ui-scale', s);
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
    if (btnOpen && modal) btnOpen.addEventListener('click', () => { modal.style.display = 'flex'; });
    if (btnClose && modal) btnClose.addEventListener('click', () => { modal.style.display = 'none'; });
    if (modal) modal.addEventListener('click', (e) => { if (e.target === modal) modal.style.display = 'none'; });
    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && modal?.style.display === 'flex') modal.style.display = 'none';
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
//   4. Dispose old scale resources (consciousness engine, cosmic renderer, planetary)
//   5. Call the new scale's scenario loader
// Rapid switching is safe because step 1 halts ticking before any teardown,
// and each loader calls _resetAllVisualState() which clears all prior state.
function switchEngineMode(mode) {
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
    app.classList.toggle('mode-consciousness', mode === 'consciousness');
    app.classList.toggle('mode-cosmic', mode === 'cosmic');
    app.classList.toggle('mode-meta', mode === 'meta');

    // If the active tab is hidden for this scale, fall back to Controls
    const scaleIndex = { lattice: '0', particles: '1', atoms: '2', molecules: '3', planetary: '4', cosmic: '5', meta: '12', consciousness: '11' }[mode];
    if (appShell) appShell.setActiveScale(scaleIndex);
    else app.setAttribute('data-active-scale', scaleIndex);

    // Free JS flux sim when leaving Scale 0 (before loadXxx resets visual state)
    if (mode !== 'lattice') Scale0Controller.exit(_makeCtx());

    // Keep mode-dependent inspector, viewport, and zoo state in sync.
    inspectorRuntime?.syncMode(mode);

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

    // Cleanup consciousness engine when leaving Scale 11
    if (mode !== 'consciousness') {
        Scale11Controller.resetScale11(_makeScale11Ctx());
    }

    // Cleanup cosmic renderer when leaving Scale 5
    if (mode !== 'cosmic') {
        Scale5Controller.resetScale5(_makeCtx());
    }

    // Cleanup planetary renderer when leaving Scale 4
    if (mode !== 'planetary') {
        Scale4Controller.dispose({ viewport, inspector, running, ticksPerFrame, engineMode });
    }

    // Cleanup meta unit when leaving Scale 12
    if (mode !== 'meta') {
        Scale6Controller.resetScale6(_makeCtx());
    }

    if (mode === 'lattice') {
        const scenario = document.getElementById('scenario-select')?.value || 'flux-pulse';
        Scale0Controller.enter(_makeCtx());
        Scale0Controller.loadScenario(_makeCtx(), scenario);
    } else if (mode === 'particles') {
        loadPEScenario(document.getElementById('pe-scenario-select')?.value || 'pe-hydrogen');
    } else if (mode === 'atoms') {
        loadAEScenario(document.getElementById('ae-scenario-select')?.value || 'ae-crystal');
    } else if (mode === 'molecules') {
        loadMoleculeScenario(document.getElementById('mol-scenario-select')?.value || 'mol-water');
    } else if (mode === 'planetary') {
        Scale4Controller.loadScenario({ viewport, inspector, running, ticksPerFrame, engineMode }, document.getElementById('planetary-scenario-select')?.value || 'planetary-solar');
    } else if (mode === 'cosmic') {
        Scale5Controller.loadCosmicScenario(_makeCtx(), document.getElementById('cosmic-scenario-select')?.value || 'cosmic-galaxy');
    } else if (mode === 'meta') {
        Scale6Controller.loadMetaScenario(_makeCtx());
    } else if (mode === 'consciousness') {
        loadConsciousnessScenario(document.getElementById('cs-scenario-select')?.value || 'cs-threshold');
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
    // The scenario button's enabled-state depends on global pause; refresh it
    // so users immediately see whether they can toggle scenario independently.
    updateLocalPlayButton();
}

function toggleScenarioPlay() {
    // Scenario play is meaningless when global is paused — silently no-op so
    // the click doesn't visually flicker the icon.
    if (!running) return;
    scenarioRunning = !scenarioRunning;
    updateLocalPlayButton();
}

function updatePlayButton() {
    const btn = document.getElementById('btn-play');
    if (!btn) return;
    btn.innerHTML = running ? '&#9208;' : '&#9654;'; // ⏸ / ▶
    btn.dataset.paused = running ? 'false' : 'true';
}

function updateLocalPlayButton() {
    const btn = document.getElementById('btn-local-play');
    if (!btn) return;
    const effective = running && scenarioRunning;
    btn.innerHTML = effective ? '&#9209;' : '&#9655;'; // ⏹ / ▷
    if (!running) {
        btn.dataset.state = 'global-paused';
    } else if (!scenarioRunning) {
        btn.dataset.state = 'local-paused-global-running';
    } else {
        btn.dataset.state = 'running';
    }
}

function clearCharts() {
    // Reset the hub's ring buffers for Scale 0 — charts share these buffers, so
    // clearing at the hub level is sufficient; uPlot instances redraw from the
    // cleared buffers on the next update().
    telemetryHub.resetScale(0);
    if (diagnostics) diagnostics.clear();
}

function formatNumber(n) {
    if (n >= 1000000) return (n / 1000000).toFixed(1) + 'M';
    if (n >= 1000) return (n / 1000).toFixed(1) + 'K';
    return n.toString();
}

// ── Phase 1-3: Ontic / Physics / Hierarchy Initialization ────────
function initOnticPhysicsHierarchy() {
    // Phase 2: Render static physics content
    const energyEl = document.getElementById('physics-energy-levels');
    if (energyEl) renderEnergyLevels(_physicsZ, energyEl);

    const csEl = document.getElementById('physics-cross-sections');
    if (csEl) renderCrossSections(csEl);

    const drEl = document.getElementById('physics-decay-rates');
    if (drEl) renderDecayRates(drEl);

    // Ontic chain constants summary card
    const constEl = document.getElementById('physics-constants');
    if (constEl) renderOnticChainSummary(constEl);

    // Physics Z slider
    const zSlider = document.getElementById('physics-z-slider');
    const zValue = document.getElementById('physics-z-value');
    if (zSlider) {
        zSlider.addEventListener('input', () => {
            _physicsZ = parseInt(zSlider.value);
            zValue.textContent = `Z=${_physicsZ}`;
            if (energyEl) renderEnergyLevels(_physicsZ, energyEl);
        });
    }

    // Initial render of ontic + hierarchy panels
    updateOnticPanel();
    updateHierarchyPanel();
}

function renderOnticChainSummary(container) {
    let rows = '';
    const constants = [
        ['G*', G_STAR.toFixed(10), 'Universal render bridge'],
        ['ϖ', VARPI.toFixed(10), 'Lemniscate constant'],
        ['1/α', X_PLUS.toFixed(7), 'Fine structure inverse'],
        ['x₋', X_MINUS.toFixed(7), '≈ N_c (color charges)'],
        ['α', ALPHA.toFixed(10), 'Fine structure constant'],
        ['K_B', K_B + ' MeV', 'Electron mass / threshold'],
    ];
    for (const [sym, val, desc] of constants) {
        rows += `<div style="display:flex;justify-content:space-between;padding:2px 0;border-bottom:1px solid var(--bg-card)">
            <span style="color:var(--accent)">${sym}</span>
            <span style="color:var(--text-primary);font-family:var(--font-mono);font-size:10px">${val}</span>
        </div>`;
    }
    container.innerHTML = `
        <div class="card-title">Ontic Chain Constants</div>
        ${rows}
        <div style="margin-top:4px;font-size:9px;color:var(--text-muted)">
            ${ONTIC_TOTAL_CONSTANTS} constants across ${ONTIC_LAYERS.length} layers.
            Inputs: D=3 + ϖ → all physics.
        </div>`;
}

function updateOnticPanel() {
    if (!observatory) return;
    const fcCard = document.getElementById('ontic-fc-card');
    const obsCard = document.getElementById('ontic-observer-card');
    const hierCard = document.getElementById('ontic-hierarchy-card');
    const infoCard = document.getElementById('ontic-info-card');
    if (!fcCard && !obsCard && !hierCard && !infoCard) return;

    // Build diagnostics data from current engine state and update observatory
    const diagData = getOnticDiagnostics();
    const scaleIdx = diagData.scale || 0;
    const rawDiag = getRawDiagnostics();
    observatory.update(rawDiag, scaleIdx, diagData.tick);
    if (fcCard) renderFcCard(observatory, fcCard);
    if (obsCard) renderObserverCard(observatory, obsCard);
    if (hierCard) renderOnticHierarchy(observatory, hierCard);
    if (infoCard) renderInfoDynamics(observatory, infoCard);
}

function updateHierarchyPanel() {
    if (!aggregateDetector || !emergenceMonitor) return;

    const diagData = getOnticDiagnostics();

    // Record emergence data
    emergenceMonitor.record(diagData);

    // Aggregation tower
    const { levels, details } = aggregateDetector.detect(diagData);
    const towerEl = document.getElementById('hierarchy-tower');
    if (towerEl) renderAggregationTower(levels, details, towerEl);

    // Scale bridge
    const scaleIdx = engineMode === 'atoms' ? 2 : engineMode === 'particles' ? 1 : 0;
    const bridgeEl = document.getElementById('hierarchy-bridge');
    if (bridgeEl) renderScaleBridge(scaleIdx, diagData, bridgeEl);

    // Emergence monitor
    const emergeEl = document.getElementById('hierarchy-emergence');
    if (emergeEl) renderEmergenceMonitor(emergenceMonitor.getTrajectory(), emergeEl);
}

/**
 * Get raw bridge diagnostics for the current engine mode.
 * Used by OnticObservatory.update(diag, scale, tick).
 */
function getRawDiagnostics() {
    try {
        if (engineMode === 'atoms' || engineMode === 'molecules') {
            const d = bridge.aeGetDiagnostics();
            return { count: d.atomCount, totalEnergy: d.totalEnergy, bondCount: d.bondCount, maxSep: 0 };
        } else if (engineMode === 'particles') {
            const d = bridge.peGetDiagnostics();
            return { count: d.particleCount, totalEnergy: d.totalEnergy, maxSep: 0 };
        } else {
            return bridge.getDiagnostics();
        }
    } catch {
        return { manifested: 0, totalFlux: 0, totalEnergy: 0, locked: 0 };
    }
}

/**
 * Extract unified diagnostics data from the current engine mode.
 * Used by OnticObservatory and AggregateDetector.
 */
function getOnticDiagnostics() {
    try {
        if (engineMode === 'atoms' || engineMode === 'molecules') {
            const diag = bridge.aeGetDiagnostics();
            const scaleNum = engineMode === 'molecules' ? 3 : 2;
            const selectId = engineMode === 'molecules' ? 'mol-scenario-select' : 'ae-scenario-select';
            const defaultName = engineMode === 'molecules' ? 'mol-h2' : 'ae-custom';
            return {
                tick: diag.tick,
                particleCount: diag.atomCount,
                boundCount: diag.bondCount,
                latticeSize: 64,
                spatialExtent: diag.atomCount > 1 ? 0.3 : 0.0,
                totalEnergy: diag.totalEnergy,
                relaxTime: 100,
                scale: scaleNum,
                scenarioName: document.getElementById(selectId)?.value || defaultName,
            };
        } else if (engineMode === 'particles') {
            const diag = bridge.peGetDiagnostics();
            return {
                tick: diag.tick,
                particleCount: diag.particleCount,
                boundCount: 0,
                latticeSize: 64,
                spatialExtent: diag.particleCount > 1 ? 0.2 : 0.0,
                totalEnergy: diag.totalEnergy,
                relaxTime: 200,
                scale: 1,
                scenarioName: document.getElementById('pe-scenario-select')?.value || 'pe-custom',
            };
        } else {
            const diag = bridge.getDiagnostics();
            return {
                tick: diag.tick,
                particleCount: diag.manifested,
                boundCount: diag.locked || 0,
                latticeSize: bridge.latticeSize || 32,
                spatialExtent: diag.manifested > 0 ? 0.15 : 0.0,
                totalEnergy: diag.totalEnergy,
                relaxTime: 500,
                scale: 0,
                scenarioName: document.getElementById('scenario-select')?.value || 'pair',
            };
        }
    } catch {
        return {
            tick: 0, particleCount: 0, boundCount: 0, latticeSize: 32,
            spatialExtent: 0, totalEnergy: 0, relaxTime: 100, scale: 0,
            scenarioName: 'Empty',
        };
    }
}

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
