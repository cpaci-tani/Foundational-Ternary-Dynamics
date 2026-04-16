/**
 * FTD Web Dashboard — Main Application Controller
 *
 * Initializes all subsystems, manages the frame loop,
 * and wires up UI controls to the simulation bridge.
 */

import { createBridge, MockBridge } from './wasm-bridge-dag.js';
import { tryNativeBridge } from './ws-bridge.js';
import { Viewport } from './viewport.js';
import { FluxEnergyChart, ParticleChart } from './charts.js';
import { DiagnosticsPanel, Sparkline } from './diagnostics.js';
import { LagrangianChart } from './lagrangian.js';
import { Inspector } from './inspector.js';
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
import { ONTIC_LAYERS, ONTIC_TOTAL_CONSTANTS, TICK_PHASES, ALPHA, K_B, K_GENESIS, G_N, DAMPING, G_STAR, VARPI, X_PLUS, X_MINUS, COS2_THETA_C, C_SPEED } from './constants.js';
// K_C, Y_REAL, Y_IMAG, THETA_C_DEG, C_MANDELBROT moved to Scale11Controller
import { AggregateDetector, ScaleBridgeVisualizer, EmergenceMonitor, renderAggregationTower, renderScaleBridge, renderEmergenceMonitor } from './aggregation-bridge.js';
import { BackgroundManager } from './backgrounds.js';
import { PETelemetryPanel } from './pe-telemetry.js';
// ConsciousnessEngine moved to Scale11Controller
import { addInfoTooltips } from './consciousness-pedagogy.js';
import { SCALE0_TOGGLES, SCALE2_TOGGLES, SCALE0_SCENARIO_OVERRIDES, LIGHT_SCENARIO_OVERRIDES } from './config/toggles.js';
import { QUANTUM_SCENARIO_DESCRIPTIONS, formatS0SeedMetadata } from './config/scenarios.js';
// CS_SCENARIO_DESCRIPTIONS moved to Scale11Controller
import { MeasurementAccumulator, QUANTUM_EXPERIMENTS, computeHistogram,
         exportCSV, exportJSON, copyToClipboard } from './quantum-lab.js';

debugLog('[FTD] App version 20260318a loaded (cache-busted)');

// ── Application State ────────────────────────────────────────────────
let _initialized = false;
let bridge = null;
// _savedBridge moved to Scale11Controller (bridge save/restore for consciousness mode)
// DEBUG: expose bridge globally for console inspection
Object.defineProperty(window, '_ftdBridge', { get() { return bridge; }, configurable: true });
let viewport = null;
let inspector = null;
let diagnostics = null;
let fluxEnergyChart = null;
let particleChart = null;
let lagrangianChart = null;
let chartCharge = null;
let chartEBEnergy = null;
let chartGauss = null;
let chartEntropy = null;
let peTelemetry = null;

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

// Quantum Lab state
let _quantumAccumulator = null;
let _quantumResults = null;

// Cached DOM / object refs for animate() hot path (avoid per-frame alloc)
let _symPanel = null;           // floating-symmetry-panel DOM element
let _symVec3 = null;            // reusable THREE.Vector3 for panel projection

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
        get inspector() { return inspector; },
        get diagnostics() { return diagnostics; },
        get fluxEnergyChart() { return fluxEnergyChart; },
        get particleChart() { return particleChart; },
        get lagrangianChart() { return lagrangianChart; },
        get chartCharge() { return chartCharge; },
        get chartEBEnergy() { return chartEBEnergy; },
        get chartGauss() { return chartGauss; },
        get chartEntropy() { return chartEntropy; },
        get peTelemetry() { return peTelemetry; },
        get running() { return running; },
        set running(v) { running = v; },
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
    };
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

// Default toggle states for Scale 0 scenarios (name, default, DOM element id)
// Toggle definitions imported from config/toggles.js
const DEFAULT_TOGGLES = SCALE0_TOGGLES;

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
        debugLog('[loading] Safety timeout dismissed overlay');
    }
}, 8000);

async function init() {
    if (_initialized) return;
    _initialized = true;

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
    diagnostics = new DiagnosticsPanel();
    fluxEnergyChart = new FluxEnergyChart(document.getElementById('chart-flux-energy'));
    particleChart = new ParticleChart(document.getElementById('chart-particles'));
    lagrangianChart = new LagrangianChart(document.getElementById('chart-lagrangian'));
    // Additional chart sparklines
    const ccEl = document.getElementById('chart-charge');
    if (ccEl) chartCharge = new Sparkline(ccEl);
    const ebEl = document.getElementById('chart-eb-energy');
    if (ebEl) chartEBEnergy = new Sparkline(ebEl);
    const cgEl = document.getElementById('chart-gauss');
    if (cgEl) chartGauss = new Sparkline(cgEl);
    const ceEl = document.getElementById('chart-entropy');
    if (ceEl) chartEntropy = new Sparkline(ceEl);
    inspector = new Inspector(viewport, bridge);
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
    wireToolbar();
    wireTabs();
    wireControls();
    wireViewportToggles();
    wireQuantumLab();
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

    // Initialize boundary shape selector
    const boundarySelect = document.getElementById('boundary-select');
    boundarySelect.addEventListener('change', () => {
        const shape = boundarySelect.value;
        viewport.setBoundaryShape(shape);
        // Propagate to all simulation bridges for containment
        if (bridge && bridge.setBoundaryShape) bridge.setBoundaryShape(shape);
        const fm = Scale0Controller.getFluxMock();
        if (fm && fm.setBoundaryShape) fm.setBoundaryShape(shape);
        // Force immediate re-render so flux volume/particles clip to new boundary
        Scale0Controller.setLatticeNeedsUpload();
    });

    // Reflective boundary toggle — when unchecked, flux/particles dissipate past boundary
    const reflectiveCheck = document.getElementById('reflective-boundary');
    reflectiveCheck.addEventListener('change', () => {
        const on = reflectiveCheck.checked;
        if (bridge && bridge.setReflectiveBoundary) bridge.setReflectiveBoundary(on);
        const fm = Scale0Controller.getFluxMock();
        if (fm && fm.setReflectiveBoundary) fm.setReflectiveBoundary(on);
    });

    _loadProgress(95, 'Loading scenario...');

    // Load default scenario (flux-pulse: pure substrate wave propagation)
    Scale0Controller.loadScenario(_makeCtx(), 'flux-pulse');

    // Done — dismiss loading overlay
    _loadProgress(100, 'Ready');
    setTimeout(() => {
        const lo = document.getElementById('loading-overlay');
        if (lo) lo.classList.add('hidden');
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

    // Update floating UI tracking (DOM ref + Vector3 cached to avoid per-frame alloc)
    if (!_symPanel) _symPanel = document.getElementById('floating-symmetry-panel');
    if (_symPanel && _symPanel.style.display === 'block' && typeof inspector !== 'undefined' && inspector._selectedPos && typeof viewport !== 'undefined' && viewport.camera) {
        if (!_symVec3) {
            const V3 = (typeof THREE !== 'undefined') ? THREE.Vector3 : (window.THREE ? window.THREE.Vector3 : null);
            if (V3) _symVec3 = new V3();
        }
        if (_symVec3) {
            const pos = inspector._selectedPos;
            _symVec3.set(pos.x, pos.y, pos.z);
            _symVec3.project(viewport.camera);

            const halfW = window.innerWidth / 2;
            const halfH = window.innerHeight / 2;

            const xOffset = (_symVec3.x * halfW) + halfW;
            const yOffset = -(_symVec3.y * halfH) + halfH;

            if (_symVec3.z < 1) {
                _symPanel.style.left = `${xOffset + 20}px`;
                _symPanel.style.top = `${yOffset - 20}px`;
            } else {
                _symPanel.style.left = '-9999px'; // Behind camera
            }
        }
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

// animateLattice -- REMOVED: delegated to Scale0Controller.animateLattice(ctx)
// See engine/web/js/scales/scale0/controller.js for the extracted code.

// ── Scale 1/2/3 Context Builders ────────────────────────────────────
function _buildScale1Ctx(now) {
    return {
        bridge, viewport, running, ticksPerFrame, inspector,
        fluxEnergyChart, particleChart, peTelemetry,
        activeTab, frameCount, dom: _dom, now,
        updateOnticPanel, updateHierarchyPanel,
    };
}

function _buildScale2Ctx(now) {
    return {
        bridge, viewport, running, ticksPerFrame, inspector,
        fluxEnergyChart, particleChart,
        activeTab, frameCount, dom: _dom, now,
        updatePlayButton, updateOnticPanel, updateHierarchyPanel,
        resetAllVisualState: _resetAllVisualState,
        setRunning: (v) => { running = v; },
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
            bridge.tick();
            const fm1 = Scale0Controller.getFluxMock();
            if (fm1) fm1.tick();
            Scale0Controller.setLatticeNeedsUpload();
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
            const scenario = document.getElementById('scenario-select').value;
            Scale0Controller.loadScenario(_makeCtx(), scenario);
        }
    });

    // Scenario select
    document.getElementById('scenario-select').addEventListener('change', (e) => {
        running = false;
        updatePlayButton();
        Scale0Controller.loadScenario(_makeCtx(), e.target.value);
        // SM seed scenarios: show epistemic-tag panel
        const latDesc = document.getElementById('lat-scenario-desc');
        const latDescText = document.getElementById('lat-scenario-desc-text');
        if (latDesc && latDescText) {
            const meta = formatS0SeedMetadata(e.target.value);
            if (meta) {
                latDescText.textContent = meta;
                latDesc.style.display = '';
                latDesc.open = true;
            } else {
                latDesc.style.display = 'none';
                latDesc.open = false;
            }
        }
        // Sync Quantum Lab experiment selector when a quantum-* scenario is picked
        if (e.target.value.startsWith('quantum-')) {
            const qlabSel = document.getElementById('qlab-experiment');
            if (qlabSel) qlabSel.value = e.target.value;
            const descEl = document.getElementById('qlab-description');
            if (descEl && QUANTUM_SCENARIO_DESCRIPTIONS[e.target.value]) {
                descEl.textContent = QUANTUM_SCENARIO_DESCRIPTIONS[e.target.value];
            }
            _switchToQuantumLabTab();
        }
    });

    // Lattice size — delegates to Scale0Controller.resizeLattice which
    // resizes the bridge + viewport while PRESERVING toggles, sliders,
    // charts, play state, and camera. The current scenario is re-injected
    // at the new size so the lattice is consistent with the dropdown.
    document.getElementById('lattice-size').addEventListener('change', (e) => {
        const size = parseInt(e.target.value);
        Scale0Controller.resizeLattice(_makeCtx(), size);
    });

    // Speed slider: 0..100 maps to ticks-per-frame via piecewise curve:
    //   [0..50]  exponential: 10^((s-50)/25) — gives 0.01..1.0 tpf
    //   [50..100] linear: 1.0 + (s-50)/50   — gives 1.0..2.0 tpf
    // Sub-1 tpf uses _tickAccumulator for fractional tick accumulation.
    // Planetary mode overrides with 4 orders of magnitude (0.01x..100x).
    const slider = document.getElementById('ticks-per-frame');
    const display = document.getElementById('tpf-display');
    function _sliderToSpeed(s) {
        if (engineMode === 'planetary') {
            // 4 orders of magnitude: 0.01x to 100.0x
            return Math.pow(10, (s - 50) / 25);
        }
        if (s <= 50) {
            // Exponential: 0.01 at s=0, 1.0 at s=50
            return Math.pow(10, (s - 50) / 25);
        }
        // Linear: 1.0 at s=50, 2.0 at s=100
        return 1.0 + (s - 50) / 50;
    }
    function _speedLabel(tpf) {
        if (tpf < 0.1) return tpf.toFixed(2);
        if (tpf < 1) return tpf.toFixed(1);
        return tpf.toFixed(1);
    }
    // Initialize from default slider value
    ticksPerFrame = _sliderToSpeed(parseFloat(slider.value));
    display.textContent = _speedLabel(ticksPerFrame);
    slider.addEventListener('input', () => {
        ticksPerFrame = _sliderToSpeed(parseFloat(slider.value));
        _tickAccumulator = 0;
        display.textContent = _speedLabel(ticksPerFrame);
    });

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
    const tabs = document.querySelectorAll('#tab-bar .tab');
    const panels = document.querySelectorAll('#panel-area .panel');

    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            const target = tab.dataset.panel;
            activeTab = target;

            // Auto-expand if panels are collapsed
            const app = document.getElementById('app');
            if (app.classList.contains('panels-collapsed')) {
                app.classList.remove('panels-collapsed');
                const toggleBtn = document.getElementById('btn-panel-toggle');
                if (toggleBtn) { toggleBtn.innerHTML = '&#9660;'; toggleBtn.title = 'Collapse panels'; }
                if (viewport && viewport.resize) setTimeout(() => viewport.resize(), 250);
            }

            tabs.forEach(t => {
                t.classList.remove('active');
                t.setAttribute('aria-selected', 'false');
                t.setAttribute('tabindex', '-1');
            });
            tab.classList.add('active');
            tab.setAttribute('aria-selected', 'true');
            tab.setAttribute('tabindex', '0');

            panels.forEach(p => {
                p.classList.toggle('active', p.id === `panel-${target}`);
            });

            // Redraw charts when their tab becomes visible
            if (target === 'charts') {
                fluxEnergyChart.draw();
                particleChart.draw();
            } else if (target === 'lagrangian') {
                lagrangianChart.draw();
            } else if (target === 'diagnostics') {
                diagnostics.drawSparklines();
                if (peTelemetry) peTelemetry.drawCharts();
            } else if (target === 'ontic') {
                updateOnticPanel();
            } else if (target === 'physics') {
                // Re-render physics content (already static, but Z may have changed)
                const energyEl = document.getElementById('physics-energy-levels');
                if (energyEl) renderEnergyLevels(_physicsZ, energyEl);
            } else if (target === 'hierarchy') {
                updateHierarchyPanel();
            }
        });
    });

    // Panel collapse/expand toggle
    const toggleBtn = document.getElementById('btn-panel-toggle');
    if (toggleBtn) {
        const app = document.getElementById('app');
        // Load cached state, default to collapsed
        const cached = localStorage.getItem('ftd-panels-collapsed');
        const isCollapsed = cached !== null ? cached === 'true' : true;
        
        if (isCollapsed) {
            app.classList.add('panels-collapsed');
            toggleBtn.innerHTML = '&#9650;';
            toggleBtn.title = 'Expand panels';
        } else {
            app.classList.remove('panels-collapsed');
            toggleBtn.innerHTML = '&#9660;';
            toggleBtn.title = 'Collapse panels';
        }

        toggleBtn.addEventListener('click', () => {
            const collapsed = app.classList.toggle('panels-collapsed');
            localStorage.setItem('ftd-panels-collapsed', collapsed);
            toggleBtn.innerHTML = collapsed ? '&#9650;' : '&#9660;';
            toggleBtn.title = collapsed ? 'Expand panels' : 'Collapse panels';
            // Notify Three.js viewport of resize
            if (viewport && viewport.resize) setTimeout(() => viewport.resize(), 250);
        });
    }

    // Custom Top-Center Panel Resizer
    const panelResizer = document.getElementById('panel-resizer');
    const panelAreaElement = document.getElementById('panel-area');
    if (panelResizer && panelAreaElement) {
        let isDraggingPanel = false;
        let startY = 0;
        let startHeight = 0;

        panelResizer.addEventListener('mousedown', (e) => {
            isDraggingPanel = true;
            startY = e.clientY;
            startHeight = panelAreaElement.getBoundingClientRect().height;
            document.body.style.cursor = 'ns-resize';
            e.preventDefault();
        });

        document.addEventListener('mousemove', (e) => {
            if (!isDraggingPanel) return;
            const dy = startY - e.clientY;
            const newHeight = startHeight + dy;
            panelAreaElement.style.height = `${newHeight}px`;
        });

        document.addEventListener('mouseup', () => {
            if (isDraggingPanel) {
                isDraggingPanel = false;
                document.body.style.cursor = '';
            }
        });
    }
}

// ── Consciousness Mode (Scale 11) — delegated to Scale11Controller ───

function loadConsciousnessScenario(name) {
    Scale11Controller.loadConsciousnessScenario(_makeScale11Ctx(), name);
}

// ── Controls Panel Wiring ────────────────────────────────────────────
function wireControls() {
    // Physics toggles
    const toggleMap = {
        't-wave': 'wave_propagation',
        't-coupling': 'coupling',
        't-damping': 'damping',
        't-genesis': 'genesis',
        't-gauss': 'gauss_projection',
        't-forces': 'forces',
        't-poisson': 'poisson_coulomb',
        't-movement': 'movement',
        't-lorentz': 'lorentz_force',
        't-gravity': 'gravity',
        't-selective': 'selective_damping',
        't-larmor': 'larmor_radiation',
        't-dual': 'dual_substrate',
        't-confinement': 'confinement',
    };

    for (const [elId, toggleName] of Object.entries(toggleMap)) {
        const el = document.getElementById(elId);
        if (el) {
            el.addEventListener('change', () => {
                bridge.setToggle(toggleName, el.checked);
                const row = el.closest('.toggle-row');
                if (row) row.classList.remove('scenario-override');
            });
        }
    }

    // ── Combo Panel: Injection ──────────────────────────────────────
    let _injState = 1;
    const injPos = document.getElementById('inj-state-pos');
    const injNeg = document.getElementById('inj-state-neg');
    if (injPos && injNeg) {
        injPos.addEventListener('click', () => {
            _injState = 1;
            injPos.classList.add('active');
            injNeg.classList.remove('active');
        });
        injNeg.addEventListener('click', () => {
            _injState = -1;
            injNeg.classList.add('active');
            injPos.classList.remove('active');
        });
    }

    function _getInjPos() {
        return {
            x: parseInt(document.getElementById('inj-x').value) || 0,
            y: parseInt(document.getElementById('inj-y').value) || 0,
            z: parseInt(document.getElementById('inj-z').value) || 0,
            state: _injState
        };
    }

    document.getElementById('btn-center').addEventListener('click', () => {
        const half = Math.floor(bridge.latticeSize / 2);
        document.getElementById('inj-x').value = half;
        document.getElementById('inj-y').value = half;
        document.getElementById('inj-z').value = half;
    });

    document.getElementById('btn-random').addEventListener('click', () => {
        const L = bridge.latticeSize || 32;
        const rand = () => 2 + Math.floor(Math.random() * (L - 4));
        document.getElementById('inj-x').value = rand();
        document.getElementById('inj-y').value = rand();
        document.getElementById('inj-z').value = rand();
    });

    document.getElementById('btn-inject').addEventListener('click', () => {
        const { x, y, z, state } = _getInjPos();
        // Use wavepacket injection: bare point particles have zero flux and
        // are immediately evaporated by the neighborhood-energy check in
        // phase_write (local_energy=0 < K_B²×EVAP_THRESHOLD).
        // Wavepacket gives the particle a Gaussian flux envelope so it
        // survives and the self-field can stabilise.
        bridge.injectWavepacket(x, y, z, state);
        Scale0Controller.setLatticeNeedsUpload();
    });

    document.getElementById('btn-inject-wave').addEventListener('click', () => {
        const { x, y, z, state } = _getInjPos();
        bridge.injectWavepacket(x, y, z, state);
        Scale0Controller.setLatticeNeedsUpload();
    });

    document.getElementById('btn-inject-flux').addEventListener('click', () => {
        const { x, y, z } = _getInjPos();
        const kb = (bridge.getParam && bridge.getParam('kb')) || K_B;
        bridge.injectFlux(x, y, z, kb * 0.8, 0, 0);
        Scale0Controller.setLatticeNeedsUpload();
    });

    document.getElementById('btn-inject-pair').addEventListener('click', () => {
        const { x, y, z } = _getInjPos();
        const kb = (bridge.getParam && bridge.getParam('kb')) || K_B;
        bridge.createEntangledPair(x, y, z, kb, 0, 0);
        Scale0Controller.setLatticeNeedsUpload();
    });

    // ── Combo Panel: Parameter Sliders ──────────────────────────────
    const comboSliders = [
        { id: 'combo-kb', valId: 'combo-kb-val', param: 'kb', fmt: 3 },
        { id: 'combo-gn', valId: 'combo-gn-val', param: 'gn', fmt: 3 },
        { id: 'combo-damp', valId: 'combo-damp-val', param: 'damping', fmt: 3 },
    ];
    for (const s of comboSliders) {
        const slider = document.getElementById(s.id);
        const display = document.getElementById(s.valId);
        if (!slider || !display) continue;
        if (bridge.isWasm) {
            slider.disabled = true;
            slider.title = 'Read-only in WASM mode';
            slider.style.opacity = '0.4';
        }
        slider.addEventListener('input', () => {
            const val = parseFloat(slider.value);
            display.textContent = val.toFixed(s.fmt);
            if (!bridge.isWasm && bridge.setParam) {
                bridge.setParam(s.param, val);
            }
        });
    }

    // ── Visuals Panel: Shape, Opacity, Size ─────────────────────────
    const SHAPE_INDEX = { circle: 0, square: 1, diamond: 2, star: 3, triangle: 4, hexagon: 5, ring: 6, cross: 7 };

    const shapeSelect = document.getElementById('particle-shape');
    if (shapeSelect) shapeSelect.addEventListener('change', () => {
        viewport.setPointShape(SHAPE_INDEX[shapeSelect.value] || 0);
    });

    // Opacity slider
    const opacitySlider = document.getElementById('size-opacity');
    const opacityDisplay = document.getElementById('size-opacity-val');
    if (opacitySlider && opacityDisplay) {
        opacitySlider.addEventListener('input', () => {
            const val = parseFloat(opacitySlider.value);
            opacityDisplay.textContent = Math.round(val * 100) + '%';
            viewport.setOpacity(val);
            if (bridge._visualSettings) bridge._visualSettings.opacity = val;
        });
    }

    // Size sliders
    const sizeSliders = [
        { id: 'size-global', valId: 'size-global-val', key: 'globalScale', suffix: 'x', fmt: 1 },
        { id: 'size-manifested', valId: 'size-manifested-val', key: 'manifestedSize', suffix: '', fmt: 0 },
        { id: 'size-void', valId: 'size-void-val', key: 'voidSize', suffix: '', fmt: 0 },
    ];
    for (const s of sizeSliders) {
        const slider = document.getElementById(s.id);
        const display = document.getElementById(s.valId);
        if (!slider || !display) continue;
        slider.addEventListener('input', () => {
            const val = parseFloat(slider.value);
            display.textContent = val.toFixed(s.fmt) + s.suffix;
            viewport.visualSettings[s.key] = val;
            if (bridge._visualSettings) bridge._visualSettings[s.key] = val;
        });
    }

    // Share visual settings with bridge
    bridge._visualSettings = viewport.visualSettings;

    // ── Combo Panel: Field Actions ──────────────────────────────────
    document.getElementById('btn-clear-field').addEventListener('click', () => {
        if (bridge.clearField) {
            bridge.clearField();
        } else {
            bridge.reset(bridge.latticeSize);
            viewport.setLatticeSize(bridge.latticeSize);
            clearCharts();
        }
        Scale0Controller.setLatticeNeedsUpload();
    });

    document.getElementById('btn-random-flux').addEventListener('click', () => {
        if (bridge.seedRandomFlux) {
            bridge.seedRandomFlux();
        }
        Scale0Controller.setLatticeNeedsUpload();
    });

    // Quick actions
    const btnEnableAll = document.getElementById('btn-enable-all');
    if (btnEnableAll) btnEnableAll.addEventListener('click', () => {
        for (const [elId] of Object.entries(toggleMap)) {
            const el = document.getElementById(elId);
            if (el) { el.checked = true; bridge.setToggle(toggleMap[elId], true); }
        }
    });

    const btnDisableAll = document.getElementById('btn-disable-all');
    if (btnDisableAll) btnDisableAll.addEventListener('click', () => {
        for (const [elId] of Object.entries(toggleMap)) {
            const el = document.getElementById(elId);
            if (el) { el.checked = false; bridge.setToggle(toggleMap[elId], false); }
        }
    });

    const btnClearParticles = document.getElementById('btn-clear-particles');
    if (btnClearParticles) btnClearParticles.addEventListener('click', () => {
        bridge.reset(bridge.latticeSize);
        viewport.setLatticeSize(bridge.latticeSize);
        clearCharts();
    });

    // ── Flux Volume Controls ──
    const fluxShapeSelect = document.getElementById('flux-shape-select');
    if (fluxShapeSelect) {
        fluxShapeSelect.addEventListener('change', () => {
            viewport.setFluxShape(parseInt(fluxShapeSelect.value));
        });
    }

    const fluxOpacitySlider = document.getElementById('flux-opacity');
    const fluxOpacityVal = document.getElementById('flux-opacity-val');
    if (fluxOpacitySlider) {
        fluxOpacitySlider.addEventListener('input', () => {
            const v = parseFloat(fluxOpacitySlider.value);
            fluxOpacityVal.textContent = v.toFixed(2);
            viewport.setFluxOpacity(v);
        });
    }

    const fluxScaleSlider = document.getElementById('flux-point-scale');
    const fluxScaleVal = document.getElementById('flux-point-scale-val');
    if (fluxScaleSlider) {
        fluxScaleSlider.addEventListener('input', () => {
            const v = parseFloat(fluxScaleSlider.value);
            fluxScaleVal.textContent = v.toFixed(1);
            viewport.setFluxPointScale(v);
            Scale0Controller.setLatticeNeedsUpload(); // force re-render with new scale
        });
    }

    const fluxThreshSlider = document.getElementById('flux-threshold');
    const fluxThreshVal = document.getElementById('flux-threshold-val');
    if (fluxThreshSlider) {
        fluxThreshSlider.addEventListener('input', () => {
            const v = parseFloat(fluxThreshSlider.value);
            fluxThreshVal.textContent = v.toFixed(3);
            viewport.setFluxThreshold(v);
            Scale0Controller.setLatticeNeedsUpload(); // force re-render with new threshold
        });
    }

    const fluxScenarioScaleSlider = document.getElementById('flux-scenario-scale');
    const fluxScenarioScaleVal = document.getElementById('flux-scenario-scale-val');
    if (fluxScenarioScaleSlider) {
        fluxScenarioScaleSlider.addEventListener('input', () => {
            const v = parseFloat(fluxScenarioScaleSlider.value);
            fluxScenarioScaleVal.textContent = v.toFixed(1);
            viewport.setScenarioScale(v);
        });
    }

    // Scale 0 dt slider
    const s0DtSlider = document.getElementById('s0-dt-slider');
    const s0DtValue = document.getElementById('s0-dt-value');
    if (s0DtSlider) {
        s0DtSlider.addEventListener('input', () => {
            const dt = parseFloat(s0DtSlider.value);
            s0DtValue.textContent = dt.toFixed(1);
            bridge.setDt(dt);
        });
    }

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

    const fluxVolBtn = document.getElementById('toggle-flux-volume');
    if (fluxVolBtn) {
        fluxVolBtn.addEventListener('click', () => {
            fluxVolBtn.classList.toggle('active');
            viewport.toggleFluxVolume(fluxVolBtn.classList.contains('active'));
            Scale0Controller.setLatticeNeedsUpload(); // force re-render on toggle
        });
    }

    const fluxSliceBtn = document.getElementById('toggle-flux-slice');
    if (fluxSliceBtn) {
        fluxSliceBtn.addEventListener('click', () => {
            fluxSliceBtn.classList.toggle('active');
            viewport.toggleFluxSlice(fluxSliceBtn.classList.contains('active'));
            Scale0Controller.setLatticeNeedsUpload();
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

    // ── Field visualization toggles (Scale 0) ───────────────────────
    // Each entry: [button-id, controller-field-key, viewport-toggle-method]
    const fieldToggles = [
        ['toggle-e-field', 'showEField', (on) => viewport.toggleEFieldLines(on)],
        ['toggle-b-field', 'showBField', (on) => viewport.toggleBFieldLines(on)],
        ['toggle-poynting', 'showPoynting', (on) => viewport.togglePoyntingVectors(on)],
        ['toggle-div-field', 'showDivField', (on) => viewport.toggleDivergenceField(on)],
        ['toggle-flux-lines', 'showFluxLines', (on) => viewport.toggleFluxStreamlines(on)],
        ['toggle-force-em', 'showForceEM', (on) => viewport.showEMForce(on)],
        ['toggle-force-gravity', 'showForceGravity', (on) => viewport.showGravityForce(on)],
        ['toggle-force-strong', 'showForceStrong', (on) => viewport.showStrongForce(on)],
        ['toggle-force-weak', 'showForceWeak', (on) => viewport.showWeakField(on)],
        ['toggle-dual-substrate', 'showDualSubstrate', (on) => viewport.toggleDualFluxVolume(on)],
        ['toggle-chirality', 'showChirality', (on) => viewport.toggleChiralityField(on)],
        ['toggle-light', 'showLight', (on) => viewport.toggleLightField(on)],
        ['toggle-dark-halo', 'showDarkMatterHalo', (on) => viewport.toggleDarkMatterHalo(on)],
        ['toggle-damping-zones', 'showDampingZones', (on) => viewport.toggleDampingZones(on)],
        ['toggle-genesis-iso', 'showGenesisIsosurface', (on) => viewport.toggleGenesisIsosurface(on)],
        ['toggle-confinement', 'showConfinement', (on) => viewport.toggleConfinement(on)],
    ];
    for (const [id, fieldKey, viewportToggle] of fieldToggles) {
        const btn = document.getElementById(id);
        if (btn) {
            btn.addEventListener('click', () => {
                btn.classList.toggle('active');
                const on = btn.classList.contains('active');
                Scale0Controller.setFieldToggle(fieldKey, on);
                viewportToggle(on);
                Scale0Controller.setLatticeNeedsUpload(); // force immediate re-render on any toggle
            });
        }
    }
}

// ── Quantum Lab Wiring ──────────────────────────────────────────────

/**
 * Programmatically switch to the Quantum Lab tab. Reuses the same logic
 * as wireTabs() so activeTab/panel state stays consistent.
 */
function _switchToQuantumLabTab() {
    const tab = document.querySelector('#tab-bar .tab[data-panel="quantum-lab"]');
    if (tab) tab.click();
}

/**
 * Wire all Quantum Lab panel controls: experiment selector, run/abort,
 * export buttons, and progress bar. Called once from init().
 */
function wireQuantumLab() {
    const experimentSel = document.getElementById('qlab-experiment');
    const descriptionEl = document.getElementById('qlab-description');
    const runBtn        = document.getElementById('qlab-run');
    const abortBtn      = document.getElementById('qlab-abort');
    const progressWrap  = document.getElementById('qlab-progress-wrap');
    const progressBar   = document.getElementById('qlab-progress-bar');
    const progressLabel = document.getElementById('qlab-progress-label');
    const progressPct   = document.getElementById('qlab-progress-pct');
    const csvBtn        = document.getElementById('qlab-export-csv');
    const jsonBtn       = document.getElementById('qlab-export-json');
    const copyBtn       = document.getElementById('qlab-copy');

    if (!experimentSel || !runBtn) return; // guard: panel not in DOM

    // A. Experiment selector → update description + sync main dropdown
    experimentSel.addEventListener('change', () => {
        const value = experimentSel.value;
        if (descriptionEl && QUANTUM_SCENARIO_DESCRIPTIONS[value]) {
            descriptionEl.textContent = QUANTUM_SCENARIO_DESCRIPTIONS[value];
        }
        // Sync main scenario dropdown and load the scenario
        const mainSel = document.getElementById('scenario-select');
        if (mainSel) {
            mainSel.value = value;
        }
        Scale0Controller.loadScenario(_makeCtx(), value);
        _switchToQuantumLabTab();
    });

    // Initialize description from whatever is selected on load
    if (descriptionEl && QUANTUM_SCENARIO_DESCRIPTIONS[experimentSel.value]) {
        descriptionEl.textContent = QUANTUM_SCENARIO_DESCRIPTIONS[experimentSel.value];
    }

    // C. Run button
    runBtn.addEventListener('click', async () => {
        const experimentName = experimentSel.value;
        const experiment = QUANTUM_EXPERIMENTS[experimentName];
        if (!experiment) return;

        const totalTrials  = parseInt(document.getElementById('qlab-trials').value) || experiment.defaultTrials;
        const ticksPerTrial = parseInt(document.getElementById('qlab-ticks').value) || experiment.defaultTicks;

        // Pause simulation
        running = false;
        updatePlayButton();

        // Show progress bar, hide Run, show Abort
        progressWrap.style.display = '';
        runBtn.style.display = 'none';
        abortBtn.style.display = '';
        csvBtn.disabled = true;
        jsonBtn.disabled = true;
        copyBtn.disabled = true;

        // Reset progress
        progressBar.style.width = '0%';
        progressLabel.textContent = 'Trial 0 / ' + totalTrials;
        progressPct.textContent = '0%';

        // Create accumulator and configure
        _quantumAccumulator = new MeasurementAccumulator();
        _quantumAccumulator.configure({
            scenarioName: experimentName,
            totalTrials: totalTrials,
            ticksPerTrial: ticksPerTrial,
            measureFn: experiment.measureFn,
            resetFn: experiment.resetFn,
        });

        try {
            await _quantumAccumulator.runAll(bridge, {
                onProgress(i, total) {
                    const pct = Math.round((i / total) * 100);
                    progressBar.style.width = pct + '%';
                    progressLabel.textContent = 'Trial ' + i + ' / ' + total;
                    progressPct.textContent = pct + '%';
                },
                onComplete(results) {
                    _quantumResults = results;

                    // Draw histogram
                    drawQuantumHistogram(results, experimentName);

                    // Update statistics
                    const stats = _quantumAccumulator.getStatistics();
                    if (stats) {
                        document.getElementById('qlab-stat-n').textContent = stats.n;
                        document.getElementById('qlab-stat-mean').textContent = isNaN(stats.mean) ? '--' : stats.mean.toFixed(4);
                        document.getElementById('qlab-stat-std').textContent = isNaN(stats.std) ? '--' : stats.std.toFixed(4);
                        document.getElementById('qlab-stat-min').textContent = isNaN(stats.min) ? '--' : stats.min.toFixed(4);
                        document.getElementById('qlab-stat-max').textContent = isNaN(stats.max) ? '--' : stats.max.toFixed(4);
                    } else {
                        // Object-type results — use analyseFn summary
                        document.getElementById('qlab-stat-n').textContent = results.length;
                        document.getElementById('qlab-stat-mean').textContent = '--';
                        document.getElementById('qlab-stat-std').textContent = '--';
                        document.getElementById('qlab-stat-min').textContent = '--';
                        document.getElementById('qlab-stat-max').textContent = '--';
                    }
                },
            });
        } catch (err) {
            console.error('Quantum Lab experiment error:', err);
        } finally {
            // Restore UI regardless of success or error
            progressWrap.style.display = 'none';
            runBtn.style.display = '';
            abortBtn.style.display = 'none';
            csvBtn.disabled = !_quantumResults;
            jsonBtn.disabled = !_quantumResults;
            copyBtn.disabled = !_quantumResults;

            // Refresh the 3D viewport to reflect final simulation state
            Scale0Controller.setLatticeNeedsUpload();
            const particleData = bridge.getParticleData();
            if (particleData) viewport.updateParticles(particleData);
        }
    });

    // D. Abort button
    abortBtn.addEventListener('click', () => {
        if (_quantumAccumulator) _quantumAccumulator.abort();
        progressWrap.style.display = 'none';
        runBtn.style.display = '';
        abortBtn.style.display = 'none';
    });

    // E. Export buttons
    csvBtn.addEventListener('click', () => {
        if (!_quantumResults) return;
        const experimentName = experimentSel.value;
        const experiment = QUANTUM_EXPERIMENTS[experimentName];
        if (!experiment) return;
        exportCSV(experiment.columns, _quantumResults, 'ftd-' + experimentName + '.csv');
    });

    jsonBtn.addEventListener('click', () => {
        if (!_quantumResults) return;
        const experimentName = experimentSel.value;
        const experiment = QUANTUM_EXPERIMENTS[experimentName];
        if (!experiment) return;
        exportJSON(
            { scenario: experimentName, latticeSize: bridge.latticeSize, timestamp: Date.now() },
            _quantumResults,
            'ftd-' + experimentName + '.json'
        );
    });

    copyBtn.addEventListener('click', () => {
        if (!_quantumResults) return;
        const experimentName = experimentSel.value;
        const experiment = QUANTUM_EXPERIMENTS[experimentName];
        if (!experiment) return;
        copyToClipboard(experiment.columns, _quantumResults);
    });
}

/**
 * Render a bar-chart histogram on the #qlab-histogram canvas.
 *
 * @param {Array} results        - Raw measurement results from the accumulator
 * @param {string} experimentName - Key into QUANTUM_EXPERIMENTS
 */
function drawQuantumHistogram(results, experimentName) {
    const canvas = document.getElementById('qlab-histogram');
    if (!canvas) return;
    const experiment = QUANTUM_EXPERIMENTS[experimentName];
    if (!experiment) return;

    // Extract numeric values for the histogram.
    // If results are plain numbers, use them directly.
    // If they are arrays/objects (e.g., Born rule returns arrays of {r, fluxDensity}),
    // flatten to the first column's values.
    let values;
    if (results.length > 0 && typeof results[0] === 'number') {
        values = results;
    } else if (results.length > 0 && Array.isArray(results[0])) {
        // Array-of-arrays: flatten and extract first numeric field
        values = [];
        for (const trial of results) {
            if (!Array.isArray(trial)) continue;
            for (const item of trial) {
                if (typeof item === 'number') { values.push(item); }
                else if (item && typeof item === 'object') {
                    // Use the first column key
                    const key = experiment.columns[0];
                    if (key && typeof item[key] === 'number') values.push(item[key]);
                    else {
                        // Try common field names
                        const v = item.r ?? item.y ?? item.x ?? item.value ?? item.intensity;
                        if (typeof v === 'number') values.push(v);
                    }
                }
            }
        }
    } else if (results.length > 0 && typeof results[0] === 'object') {
        // Array of objects (one per trial)
        values = [];
        const key = experiment.columns[0];
        for (const item of results) {
            if (key && typeof item[key] === 'number') values.push(item[key]);
            else {
                const v = item.r ?? item.y ?? item.x ?? item.value ?? item.intensity;
                if (typeof v === 'number') values.push(v);
            }
        }
    } else {
        values = [];
    }

    if (values.length === 0) return;

    const hist = computeHistogram(values, 25);
    const { edges, counts, binWidth } = hist;
    const maxCount = Math.max(1, ...counts);

    // DPR-aware canvas sizing
    const dpr = window.devicePixelRatio || 1;
    const cssW = canvas.clientWidth;
    const cssH = canvas.clientHeight;
    canvas.width = cssW * dpr;
    canvas.height = cssH * dpr;
    const ctx = canvas.getContext('2d');
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);

    // Layout
    const padLeft = 40, padRight = 10, padTop = 10, padBottom = 28;
    const plotW = cssW - padLeft - padRight;
    const plotH = cssH - padTop - padBottom;

    // Background
    ctx.fillStyle = '#1a1f2e';
    ctx.fillRect(0, 0, cssW, cssH);

    // Bars
    const barW = plotW / counts.length;
    ctx.fillStyle = '#00e5ff';
    for (let i = 0; i < counts.length; i++) {
        const barH = (counts[i] / maxCount) * plotH;
        ctx.fillRect(padLeft + i * barW + 1, padTop + plotH - barH, barW - 2, barH);
    }

    // Y-axis labels (5 ticks)
    ctx.fillStyle = '#667';
    ctx.font = '9px monospace';
    ctx.textAlign = 'right';
    for (let t = 0; t <= 4; t++) {
        const val = Math.round((t / 4) * maxCount);
        const y = padTop + plotH - (t / 4) * plotH;
        ctx.fillText(val, padLeft - 4, y + 3);
    }

    // X-axis labels (5-6 evenly spaced bin edges)
    ctx.textAlign = 'center';
    const labelCount = Math.min(6, edges.length);
    const step = Math.max(1, Math.floor(edges.length / labelCount));
    for (let i = 0; i < edges.length; i += step) {
        const x = padLeft + (i / (edges.length - 1)) * plotW;
        ctx.fillText(edges[i].toFixed(1), x, cssH - 4);
    }

    // Expected curve overlay (if experiment provides one)
    if (experiment.expectedCurve) {
        ctx.beginPath();
        ctx.strokeStyle = '#ffd700'; // gold
        ctx.lineWidth = 2;
        const xMin = edges[0];
        const xMax = edges[edges.length - 1];
        const steps = 100;
        // Normalise the expected curve to match histogram peak
        let curveMax = 0;
        for (let s = 0; s <= steps; s++) {
            const xv = xMin + (s / steps) * (xMax - xMin);
            const yv = experiment.expectedCurve(xv);
            if (yv > curveMax) curveMax = yv;
        }
        const curveScale = curveMax > 0 ? maxCount / curveMax : 1;
        for (let s = 0; s <= steps; s++) {
            const xv = xMin + (s / steps) * (xMax - xMin);
            const yv = experiment.expectedCurve(xv) * curveScale;
            const px = padLeft + (s / steps) * plotW;
            const py = padTop + plotH - (yv / maxCount) * plotH;
            if (s === 0) ctx.moveTo(px, py);
            else ctx.lineTo(px, py);
        }
        ctx.stroke();
    }

    // Update chart title
    const titleEl = document.getElementById('qlab-chart-title');
    if (titleEl && experiment.label) {
        titleEl.textContent = experiment.label + ' Distribution';
    }
}

// ── Keyboard Shortcuts ───────────────────────────────────────────────
function wireKeyboard() {
    document.addEventListener('keydown', (e) => {
        // Ignore if typing in an input
        if (e.target.tagName === 'INPUT' || e.target.tagName === 'SELECT') return;

        switch (e.key.toLowerCase()) {
            case ' ':
                e.preventDefault();
                togglePlay();
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
                    bridge.tick();
                    { const fm = Scale0Controller.getFluxMock(); if (fm) fm.tick(); }
                    Scale0Controller.setLatticeNeedsUpload();
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
                    const scenario = document.getElementById('scenario-select').value;
                    Scale0Controller.loadScenario(_makeCtx(), scenario);
                }
                break;
        }

        // Field visualization shortcuts (1-8) — Scale 0 only
        if (engineMode === 'lattice') {
            const fieldKeys = {
                '1': 'toggle-e-field',
                '2': 'toggle-b-field',
                '3': 'toggle-poynting',
                '4': 'toggle-div-field',
                '5': 'toggle-flux-lines',
                '6': 'toggle-force-em',
                '7': 'toggle-dual-substrate',
                '8': 'toggle-chirality',
                '9': 'toggle-light',
            };
            const btnId = fieldKeys[e.key];
            if (btnId) {
                const btn = document.getElementById(btnId);
                if (btn) btn.click();
            }
        }
    });
}

// ── Settings Modal ──────────────────────────────────────────────────
{
    const modal = document.getElementById('settings-modal');
    const btnOpen = document.getElementById('btn-settings');
    const btnClose = document.getElementById('settings-close');
    const slider = document.getElementById('settings-ui-scale');
    const valDisplay = document.getElementById('settings-scale-val');
    const btnReset = document.getElementById('settings-reset');

    // ── Scale ──
    function applyScale(s) {
        document.documentElement.style.setProperty('--ui-scale', s);
        if (slider) slider.value = s;
        if (valDisplay) valDisplay.textContent = Math.round(s * 100) + '%';
        document.querySelectorAll('.settings-preset').forEach(b => {
            b.classList.toggle('active', Math.abs(parseFloat(b.dataset.scale) - s) < 0.01);
        });
        try { localStorage.setItem('ftd-ui-scale', String(s)); } catch (e) { }
        if (viewport && viewport.resize) setTimeout(() => viewport.resize(), 100);
    }

    // ── Theme ──
    function applyTheme(name) {
        if (name === 'default') {
            document.documentElement.removeAttribute('data-theme');
        } else {
            document.documentElement.setAttribute('data-theme', name);
        }
        document.querySelectorAll('.theme-swatch').forEach(sw => {
            sw.classList.toggle('active', sw.dataset.theme === name);
        });
        try { localStorage.setItem('ftd-theme', name); } catch (e) { }
    }

    // ── Load saved settings ──
    try {
        const savedScale = localStorage.getItem('ftd-ui-scale');
        if (savedScale) applyScale(parseFloat(savedScale));
        const savedTheme = localStorage.getItem('ftd-theme');
        if (savedTheme) applyTheme(savedTheme);
    } catch (e) { }

    // ── Modal open/close ──
    if (btnOpen && modal) btnOpen.addEventListener('click', () => { modal.style.display = 'flex'; });
    if (btnClose && modal) btnClose.addEventListener('click', () => { modal.style.display = 'none'; });
    if (modal) modal.addEventListener('click', (e) => { if (e.target === modal) modal.style.display = 'none'; });

    // ── Scale controls ──
    if (slider) slider.addEventListener('input', () => applyScale(parseFloat(slider.value)));
    document.querySelectorAll('.settings-preset').forEach(btn => {
        btn.addEventListener('click', () => applyScale(parseFloat(btn.dataset.scale)));
    });

    // ── Theme controls ──
    document.querySelectorAll('.theme-swatch').forEach(sw => {
        sw.addEventListener('click', () => applyTheme(sw.dataset.theme));
    });

    // ── Reset all ──
    if (btnReset) {
        btnReset.addEventListener('click', () => {
            applyScale(1.1);
            applyTheme('default');
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
    app.setAttribute('data-active-scale', scaleIndex);

    // Dynamically filter tabs based strictly on data-scales
    document.querySelectorAll('#tab-bar .tab').forEach(tab => {
        if (tab.dataset.scales) {
            if (tab.dataset.scales.split(',').includes(scaleIndex)) tab.style.display = '';
            else tab.style.display = 'none';
        } else {
            tab.style.display = '';
        }
    });

    const activeTabEl = document.querySelector('#tab-bar .tab.active');
    if (activeTabEl && activeTabEl.style.display === 'none') {
        const controlsTab = document.querySelector('#tab-bar .tab[data-panel="controls"]');
        if (controlsTab) controlsTab.click();
    }

    // Free JS flux sim when leaving Scale 0 (before loadXxx resets visual state)
    if (mode !== 'lattice') Scale0Controller.clearFluxMock();

    // Tell inspector, viewport, and zoo panel about mode change
    if (inspector) inspector.setEngineMode(mode);
    if (viewport) viewport.setEngineMode(mode);
    setZooMode(mode);

    const tpfSlider = document.getElementById('ticks-per-frame');
    if (tpfSlider) tpfSlider.dispatchEvent(new Event('input'));

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
        const scenario = document.getElementById('scenario-select')?.value || 'stable-universe';
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
}

function updatePlayButton() {
    const btn = document.getElementById('btn-play');
    btn.classList.toggle('active', running);
    btn.innerHTML = running ? '&#9646;&#9646;' : '&#9654;';
}

// DEAD: loadScenario, _markScenarioOverrides, _syncComboSliders
// Now delegated to Scale0Controller.loadScenario(ctx, name)
// See engine/web/js/scales/scale0/controller.js
function loadScenario(name) {
    _resetAllVisualState();
    bridge.setupScenario(name);

    // Create/reset MockBridge for JS-side flux visualization (fallback when WASM
    // doesn't have getFluxVolume, or for the parallel JS wave equation demo)
    const L = bridge.latticeSize || 32;
    _fluxMock = new MockBridge(L);
    // Sync boundary shape and reflective setting to new mock bridge
    const boundaryEl = document.getElementById('boundary-select');
    if (boundaryEl) _fluxMock.setBoundaryShape(boundaryEl.value);
    const reflEl = document.getElementById('reflective-boundary');
    if (reflEl) _fluxMock.setReflectiveBoundary(reflEl.checked);
    _fluxMock.setupScenario(name);

    // Reset ALL toggles to defaults before applying scenario-specific overrides.
    // This prevents state leakage between scenarios (e.g., gravity staying ON).
    for (const [key, val, elId] of DEFAULT_TOGGLES) {
        bridge.setToggle(key, val);
        const el = document.getElementById(elId);
        if (el) el.checked = val;
    }

    // Scenario-specific toggle overrides (data-driven from config/toggles.js)
    const overrides = SCALE0_SCENARIO_OVERRIDES[name];
    if (overrides) {
        for (const [key, val, elId] of overrides) {
            bridge.setToggle(key, val);
            const el = document.getElementById(elId);
            if (el) el.checked = val;
        }
    }

    // Light scenarios: pure EM wave propagation (no matter coupling)
    if (name.startsWith('light-')) {
        for (const [key, val, elId] of LIGHT_SCENARIO_OVERRIDES) {
            bridge.setToggle(key, val);
            const el = document.getElementById(elId);
            if (el) el.checked = val;
        }
    }

    // Sync all toggle states to MockBridge from HTML (single source of truth)
    if (_fluxMock) {
        for (const [key, , elId] of DEFAULT_TOGGLES) {
            const el = document.getElementById(elId);
            if (el) _fluxMock.setToggle(key, el.checked);
        }
    }

    // Mark toggles that differ from defaults after scenario overrides
    _markScenarioOverrides();

    // Resync combo panel sliders to bridge defaults after reset
    _syncComboSliders();

    Scale0Controller.setLatticeNeedsUpload();
}

function _markScenarioOverrides() {
    const advDetails = document.querySelector('.toggle-advanced');
    let advNeedsOpen = false;
    for (const [, defaultVal, elId] of DEFAULT_TOGGLES) {
        const el = document.getElementById(elId);
        if (!el) continue;
        const row = el.closest('.toggle-row');
        if (!row) continue;
        if (el.checked !== defaultVal) {
            row.classList.add('scenario-override');
            if (advDetails && advDetails.contains(el)) advNeedsOpen = true;
        } else {
            row.classList.remove('scenario-override');
        }
    }
    if (advNeedsOpen && advDetails) advDetails.open = true;
}

function _syncComboSliders() {
    const defaults = { kb: K_B, gn: G_N, damping: DAMPING };
    const map = [
        { id: 'combo-kb', valId: 'combo-kb-val', param: 'kb', fmt: 3 },
        { id: 'combo-gn', valId: 'combo-gn-val', param: 'gn', fmt: 3 },
        { id: 'combo-damp', valId: 'combo-damp-val', param: 'damping', fmt: 3 },
    ];
    for (const s of map) {
        const slider = document.getElementById(s.id);
        const display = document.getElementById(s.valId);
        if (!slider || !display) continue;
        const val = bridge?.getParam ? bridge.getParam(s.param) : defaults[s.param];
        if (val != null) {
            slider.value = val;
            display.textContent = val.toFixed(s.fmt);
        }
    }
}

function clearCharts() {
    if (fluxEnergyChart) fluxEnergyChart.clear();
    if (particleChart) particleChart.clear();
    if (lagrangianChart) lagrangianChart.clear();
    if (diagnostics) diagnostics.clear();
    if (chartCharge) chartCharge.clear();
    if (chartEBEnergy) chartEBEnergy.clear();
    if (chartGauss) chartGauss.clear();
    if (chartEntropy) chartEntropy.clear();
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

    // Build diagnostics data from current engine state and update observatory
    const diagData = getOnticDiagnostics();
    const scaleIdx = diagData.scale || 0;
    const rawDiag = getRawDiagnostics();
    observatory.update(rawDiag, scaleIdx, diagData.tick);

    const fcCard = document.getElementById('ontic-fc-card');
    if (fcCard) renderFcCard(observatory, fcCard);

    const obsCard = document.getElementById('ontic-observer-card');
    if (obsCard) renderObserverCard(observatory, obsCard);

    const hierCard = document.getElementById('ontic-hierarchy-card');
    if (hierCard) renderOnticHierarchy(observatory, hierCard);

    const infoCard = document.getElementById('ontic-info-card');
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
