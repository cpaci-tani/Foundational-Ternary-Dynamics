/**
 * FTD Web Dashboard — Main Application Controller
 *
 * Initializes all subsystems, manages the frame loop,
 * and wires up UI controls to the simulation bridge.
 */

import { createBridge, MockBridge } from './wasm-bridge.js?v=20260309c';
import { Viewport } from './viewport.js?v=20260309g';
import { FluxEnergyChart, ParticleChart } from './charts.js?v=20260304q';
import { DiagnosticsPanel } from './diagnostics.js?v=20260304q';
import { LagrangianChart } from './lagrangian.js?v=20260304q';
import { Inspector } from './inspector.js?v=20260304q';
import { initZoo, setEngineMode as setZooMode } from './zoo.js?v=20260304q';
import { getById } from './particle-catalog.js?v=20260304q';
import { allElements, tablePosition, elementSymbol, getElement } from './elements.js?v=20260304q';
import { getCategories, getMoleculesByCategory, getMolecule, loadMolecule } from './molecules.js?v=20260304q';
import { expandAEToOrbitalCloud, generateBondingCloud, electronConfig, slaterZeff, A0_DISPLAY, nuclearShellRadius } from './orbitals.js?v=20260309c';
import { atomicEnergy, periodicTableTotalEnergy, formatEnergy as formatEnergyAE } from './atomic-energy.js?v=20260304q';
import { formatEnergy, formatTemperature, formatVelocity } from './units.js';
import { generateGridXZ, samplePEField, samplePECoulombOnly, samplePEGravityField, makePECoulombFieldFn, sampleAEField } from './fields.js?v=20260304q';
import { computeStreamlines, generateEFieldSeeds, generateBFieldSeeds, generateGridSeeds } from './fieldlines.js?v=20260304q';

// ── Phase 1-3: Ontic Observatory, Physics Fidelity, Aggregation Bridge
import { OnticObservatory, renderFcCard, renderObserverCard, renderHierarchyTower as renderOnticHierarchy, renderInfoDynamics } from './ontic-observatory.js?v=20260304q';
import { renderEnergyLevels } from './spectroscopy.js?v=20260304q';
import { renderCrossSections } from './cross-sections.js?v=20260304q';
import { renderDecayRates } from './decay-rates.js?v=20260304q';
import { ONTIC_LAYERS, ONTIC_TOTAL_CONSTANTS, TICK_PHASES, ALPHA, K_B, K_GENESIS, G_N, DAMPING, G_STAR, VARPI, X_PLUS, X_MINUS, K_C, Y_REAL, Y_IMAG, THETA_C_DEG, C_MANDELBROT, COS2_THETA_C } from './constants.js?v=20260305e';
import { AggregateDetector, ScaleBridgeVisualizer, EmergenceMonitor, renderAggregationTower, renderScaleBridge, renderEmergenceMonitor } from './aggregation-bridge.js?v=20260304q';
import { BackgroundManager } from './backgrounds.js?v=20260304s';
import { PETelemetryPanel } from './pe-telemetry.js?v=20260304q';
import { ConsciousnessEngine } from './consciousness.js?v=20260305e';
import { ConsciousnessPedagogy, addInfoTooltips } from './consciousness-pedagogy.js?v=20260317a';

console.log('[FTD] App version 20260309f loaded (cache-busted)');

// ── Application State ────────────────────────────────────────────────
let _initialized = false;
let bridge = null;
// DEBUG: expose bridge globally for console inspection
Object.defineProperty(window, '_ftdBridge', { get() { return bridge; }, configurable: true });
let viewport = null;
let inspector = null;
let diagnostics = null;
let fluxEnergyChart = null;
let particleChart = null;
let lagrangianChart = null;
let peTelemetry = null;

let running = false;
let ticksPerFrame = 1;
let _tickAccumulator = 0; // accumulates fractional ticks for sub-1 speed
let activeTab = 'controls';
let frameCount = 0;
let lastFpsTime = performance.now();
let fpsDisplay = 0;
let engineMode = 'lattice'; // 'lattice', 'particles', 'atoms', 'molecules', or 'consciousness'
let _csEngine = null;          // ConsciousnessEngine instance (Scale 4)
let _csPedagogy = null;        // ConsciousnessPedagogy instance (Theory/Walkthrough panels)
let _csScenarioMeta = { name: '', domain: 'Real (k=16)', thetaMode: 'static', sloopDepth: 0, bellS: null };
let _mandelbrotZ_re = 0, _mandelbrotZ_im = 0, _mandelbrotIter = 0;
let _aeInitialEnergy = null; // for AE energy drift tracking
let _showBonds = true;
let _showOrbitalClouds = true; // orbital electron clouds in AE mode
let bgManager = null;          // BackgroundManager instance
let _prevLegendKey = '';        // cached element-set key for legend rebuild
let _showPEEField = false;      // E-field streamlines for Scale 1
let _showPEPotential = false;   // Coulomb potential heatmap for Scale 1
let _showPEGravField = false;   // Gravity field vectors for Scale 1
let _showPEForces = false;      // Per-particle net force arrows for Scale 1
let _showAEField = false;       // force field overlay for Scale 2
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
let _fieldGrid = null;          // cached grid from generateGridXZ
let _srcParticlesBuf = [];      // reusable {x,y,z} array for field seed generation
let _fluxMock = null;           // MockBridge for Scale 0 flux visualization fallback

// Field visualization state (Scale 0)
let _showEField = false;
let _showBField = false;
let _showPoynting = false;
let _showDivField = false;
let _showFluxLines = false;
let _showForceVolume = false;
let _showDualSubstrate = false;
let _showChirality = false;
let _showLight = false;
let _showGravityField = false;
let _showDarkMatterHalo = false;
let _showDampingZones = false;
let _showGenesisIsosurface = false;
let _fieldFrame = 0;            // throttle counter for field updates
let _fieldNeedsUpdate = false;  // force immediate field compute on toggle activation
let _anyFieldActive = false;    // cached OR of all field toggle flags

function _recomputeAnyFieldActive() {
    _anyFieldActive = _showEField || _showBField || _showPoynting ||
        _showDivField || _showFluxLines || _showForceVolume ||
        _showDualSubstrate || _showChirality || _showLight ||
        _showGravityField || _showDarkMatterHalo || _showDampingZones ||
        _showGenesisIsosurface;
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
// ── Consciousness Sub-tab Wiring ──────────────────────────────────────

const SCENARIO_DESCRIPTIONS = {
    'cs-threshold':      'Flux starts below the consciousness threshold K_C \u2248 3.60. As flux energy builds, the discriminant \u0394_k passes through zero, and roots transition from real (physics) through degenerate (measurement) to complex (consciousness). Watch the Domain indicator change.',
    'cs-high-coupling':  'Four-source flux interference with coupling and forces enabled. High flux density pushes well above K_C, producing strong consciousness intensity. The holographic figure becomes vivid as the observer\u2019s self-model stabilizes.',
    'cs-self-ref':       'Standing wave pattern: the observer meets itself. sLoop depth = 1 \u2014 a fixed point of the gap equation x\u00B2 = K(x \u2212 G*). The lattice determines its own coupling.',
    'cs-nested-sloop':   'Two orthogonal standing waves: aware of self-awareness. sLoop depth = 2. This is the algebraic expression of recursive self-referential closure.',
    'cs-chirality':      'Dual substrate with left/right asymmetric injection demonstrating parity violation. The chirality split mirrors the 3:1 alternating handedness of the dyadic Fourier shells.',
    'cs-boundary-orbit': 'Mandelbrot iteration at c = 1/G* \u2248 0.338, tracking the edge of chaos. The fixed points of z \u2192 z\u00B2 + c are exactly the consciousness roots y = 2.19 \u00B1 2.86i.',
    'cs-entangled':      'Full coupling with Bell parameter S = 2\u221A2 \u2248 2.83. All forces, genesis, and movement enabled. Demonstrates observer-lattice entanglement via complexification + sLoop coupling.',
    'cs-flow':           'Fast vortex pattern with effective \u03B8 < 52.54\u00B0 (object-dominant flow state). The holographic figure responds with rapid, outward-focused dynamics.',
    'cs-meditation':     'Gentle centered pulse with effective \u03B8 > 52.54\u00B0 (subject-dominant contemplative state). The observer turns inward, producing slow, resonant breathing patterns.',
};

function wireConsciousnessSubTabs() {
    const subtabs = document.querySelectorAll('.cs-subtab');
    const subpanels = document.querySelectorAll('.cs-subpanel');
    subtabs.forEach(st => {
        st.addEventListener('click', () => {
            subtabs.forEach(s => s.classList.remove('active'));
            st.classList.add('active');
            const target = st.dataset.cspanel;
            subpanels.forEach(sp => sp.classList.toggle('active', sp.id === target));
            if (_csPedagogy) {
                if (target === 'cs-theory') { _csPedagogy.show(); _csPedagogy.resize(); }
                else if (target === 'cs-walkthrough') { _csPedagogy.startWalkthrough(); _csPedagogy.show(); }
                else _csPedagogy.hide();
            }
        });
    });

    // Scenario description wiring
    const scenarioSel = document.getElementById('cs-scenario-select');
    if (scenarioSel) {
        scenarioSel.addEventListener('change', () => {
            const descEl = document.getElementById('cs-scenario-desc-text');
            if (descEl) descEl.textContent = SCENARIO_DESCRIPTIONS[scenarioSel.value] || '';
        });
    }

    // Walkthrough prev/next
    const prevBtn = document.getElementById('cs-walk-prev');
    const nextBtn = document.getElementById('cs-walk-next');
    if (prevBtn) prevBtn.addEventListener('click', () => {
        if (_csPedagogy) _csPedagogy.setWalkthroughStep((_csPedagogy._walkthroughStep || 0) - 1);
    });
    if (nextBtn) nextBtn.addEventListener('click', () => {
        if (_csPedagogy) _csPedagogy.setWalkthroughStep((_csPedagogy._walkthroughStep || 0) + 1);
    });
}

function _resetAllVisualState() {
    // ── Charts & telemetry ──
    clearCharts();
    if (peTelemetry) peTelemetry.clear();

    // ── Trail / orbit history ──
    _trailHistory.clear();
    _fieldGrid = null;
    if (viewport) {
        viewport.clearTrails();
        viewport.clearElementLabels();
    }

    // ── Scale 0: field visualization overlays ──
    _showEField = false;
    _showBField = false;
    _showPoynting = false;
    _showDivField = false;
    _showFluxLines = false;
    _showForceVolume = false;
    _showDualSubstrate = false;
    _showChirality = false;
    _showLight = false;
    _showGravityField = false;
    _showDarkMatterHalo = false;
    _showDampingZones = false;
    _showGenesisIsosurface = false;
    _fieldNeedsUpdate = false;
    _recomputeAnyFieldActive();

    // Deactivate Scale 0 field toggle buttons
    for (const id of [
        'toggle-e-field', 'toggle-b-field', 'toggle-poynting',
        'toggle-div-field', 'toggle-flux-lines', 'toggle-force-volume',
        'toggle-dual-substrate', 'toggle-chirality', 'toggle-light',
        'toggle-gravity-field', 'toggle-dark-halo', 'toggle-damping-zones',
        'toggle-genesis-iso',
    ]) {
        const btn = document.getElementById(id);
        if (btn) btn.classList.remove('active');
    }
    // Tell viewport to hide all field overlays
    if (viewport) {
        viewport.toggleEFieldLines(false);
        viewport.toggleBFieldLines(false);
        viewport.togglePoyntingVectors(false);
        viewport.toggleDivergenceField(false);
        viewport.toggleFluxStreamlines(false);
        viewport.toggleForceVolume(false);
        viewport.toggleDualFluxVolume(false);
        viewport.toggleChiralityField(false);
        viewport.toggleLightField(false);
        viewport.toggleGravityField(false);
        viewport.toggleDarkMatterHalo(false);
        viewport.toggleDampingZones(false);
        viewport.toggleGenesisIsosurface(false);
    }

    // ── Scale 1: PE overlay buttons ──
    _showPEEField = false;
    _showPEPotential = false;
    _showPEGravField = false;
    _showPEForces = false;
    _showVelocities = false;
    _showTrails = false;
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

    // ── Scale 2/3: AE field overlay ──
    _showAEField = false;
    const aeFieldBtn = document.getElementById('toggle-ae-field');
    if (aeFieldBtn) aeFieldBtn.classList.remove('active');

    // ── Scale 2/3: Enhanced atom/molecule visuals ──
    _showNucleusShells = true;
    _bondStyle = 'cylinders';
    _showShellBounds = false;
    _showOrbitalLobes = false;
    _showAEForceIonic = false;
    _showAEForceVdw = false;
    _showAEForceBond = false;
    _showAEForceNet = false;
    _forceFrame = 0;

    // Reset new visual toggle buttons
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

    // Tell viewport to reset enhanced visual objects
    if (viewport) {
        viewport.toggleNucleusShells(true);
        viewport.toggleBondCylinders(true);
        viewport.toggleOrbitalShells(false);
        viewport.toggleOrbitalLobes(false);
        viewport.toggleAEForceIonic(false);
        viewport.toggleAEForceVdw(false);
        viewport.toggleAEForceBond(false);
        viewport.toggleAEForceNet(false);
    }

    // ── AE energy drift reference ──
    _aeInitialEnergy = null;
}

// Default toggle states for Scale 0 scenarios (name, default, DOM element id)
const DEFAULT_TOGGLES = [
    ['wave_propagation', true,  't-wave'],
    ['coupling',         true,  't-coupling'],
    ['damping',          true,  't-damping'],
    ['genesis',          true,  't-genesis'],
    ['gauss_projection', true,  't-gauss'],
    ['forces',           true,  't-forces'],
    ['gravity',          false, 't-gravity'],
    ['movement',         true,  't-movement'],
    ['poisson_coulomb',  true,  't-poisson'],
    ['lorentz_force',    false, 't-lorentz'],
    ['selective_damping',false, 't-selective'],
    ['larmor_radiation', false, 't-larmor'],
    ['dual_substrate',   false, 't-dual'],
    ['confinement',      false, 't-confinement'],
];

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

// ── Cloud Rendering for PE Mode ─────────────────────────────────────
// Each particle is rendered as a Gaussian flux cloud, not a point.
// Cloud point count ~ mass (electron 0.511 MeV → 511 cloud points).
// Pre-allocated buffers avoid per-frame allocation.
const MAX_CLOUD_TOTAL = 100000;
const _cloudPos = new Float32Array(MAX_CLOUD_TOTAL * 3);
const _cloudCol = new Float32Array(MAX_CLOUD_TOTAL * 3);
const _cloudSize = new Float32Array(MAX_CLOUD_TOTAL);
const _cloudParticleMap = new Int32Array(MAX_CLOUD_TOTAL); // cloud point index → PE particle ID

// ── Trail History for PE Mode ────────────────────────────────────────
const TRAIL_MAX_LENGTH = 200;
const _trailHistory = new Map(); // particleId → { positions: Float32Array, head: int, length: int }
let _showVelocities = false;
let _showTrails = false;

// Cache pre-generated cloud templates per particle type
const _cloudTemplates = new Map();

function ensureCloudTemplate(catalogId, mass_mev) {
    if (_cloudTemplates.has(catalogId)) return _cloudTemplates.get(catalogId);

    // Point count scales with mass: electron (0.511 MeV) → 511 points
    // Power law so proton (938 MeV) → ~3000, not 938000
    const nRaw = Math.round(603 * Math.pow(mass_mev, 0.238));
    const n = Math.min(Math.max(nRaw, 50), 5000);

    // Cloud radius: lighter particles are more spread out (Compton-like)
    const radius = 2.0 + 3.0 * Math.pow(0.511 / mass_mev, 0.15);
    const sigma = radius / 2.5; // ~95% within radius

    const offsets = new Float32Array(n * 3);
    const brightness = new Float32Array(n);

    for (let i = 0; i < n; i++) {
        // Box-Muller for 3D Gaussian
        const u1 = Math.random() || 1e-10, u2 = Math.random();
        const u3 = Math.random() || 1e-10, u4 = Math.random();
        const sq1 = Math.sqrt(-2 * Math.log(u1));
        const sq3 = Math.sqrt(-2 * Math.log(u3));

        const ox = sq1 * Math.cos(2 * Math.PI * u2) * sigma;
        const oy = sq1 * Math.sin(2 * Math.PI * u2) * sigma;
        const oz = sq3 * Math.cos(2 * Math.PI * u4) * sigma;

        offsets[i * 3] = ox;
        offsets[i * 3 + 1] = oy;
        offsets[i * 3 + 2] = oz;

        const dist = Math.sqrt(ox * ox + oy * oy + oz * oz) / radius;
        brightness[i] = Math.exp(-dist * dist * 2.0); // Gaussian falloff
    }

    const tmpl = { n, radius, offsets, brightness };
    _cloudTemplates.set(catalogId, tmpl);
    return tmpl;
}

function expandPEToCloud(peData, typeMap, t) {
    const srcCount = peData.count;
    let out = 0;

    for (let i = 0; i < srcCount && out < MAX_CLOUD_TOTAL; i++) {
        const cx = peData.positions[i * 3];
        const cy = peData.positions[i * 3 + 1];
        const cz = peData.positions[i * 3 + 2];

        const pid = peData.ids ? peData.ids[i] : -1;
        const catId = typeMap ? typeMap.get(pid) : null;
        const p = catId ? getById(catId) : null;

        if (p) {
            const tmpl = ensureCloudTemplate(catId, p.mass_mev);
            const [cr, cg, cb] = p.display_color;
            const n = Math.min(tmpl.n, MAX_CLOUD_TOTAL - out);
            const wiggle = 0.15 * tmpl.radius; // 15% of cloud radius

            for (let j = 0; j < n; j++) {
                // Per-point sinusoidal perturbation for organic "breathing" motion
                // Golden angle phase spacing ensures adjacent points move independently
                const phase = j * 2.39996323;
                const fx = Math.sin(t * 1.7 + phase) * wiggle;
                const fy = Math.sin(t * 2.3 + phase * 1.3) * wiggle;
                const fz = Math.sin(t * 1.1 + phase * 0.7) * wiggle;

                _cloudPos[out * 3] = cx + tmpl.offsets[j * 3] + fx;
                _cloudPos[out * 3 + 1] = cy + tmpl.offsets[j * 3 + 1] + fy;
                _cloudPos[out * 3 + 2] = cz + tmpl.offsets[j * 3 + 2] + fz;

                const b = tmpl.brightness[j];
                _cloudCol[out * 3] = cr * b;
                _cloudCol[out * 3 + 1] = cg * b;
                _cloudCol[out * 3 + 2] = cb * b;

                _cloudSize[out] = 1.5 + b * 1.5; // 1.5 at edge → 3.0 at center
                _cloudParticleMap[out] = pid;
                out++;
            }
        } else {
            // Fallback: single point for untyped particles
            _cloudPos[out * 3] = cx;
            _cloudPos[out * 3 + 1] = cy;
            _cloudPos[out * 3 + 2] = cz;
            _cloudCol[out * 3] = 0.5;
            _cloudCol[out * 3 + 1] = 0.5;
            _cloudCol[out * 3 + 2] = 0.5;
            _cloudSize[out] = 3.0;
            _cloudParticleMap[out] = pid;
            out++;
        }
    }

    return { positions: _cloudPos, colors: _cloudCol, sizes: _cloudSize, count: out };
}

function updateTrailHistory(peData) {
    // Record current positions into circular buffers
    for (let i = 0; i < peData.count; i++) {
        const id = peData.ids[i];
        if (!_trailHistory.has(id)) {
            _trailHistory.set(id, {
                positions: new Float32Array(TRAIL_MAX_LENGTH * 3),
                head: 0, length: 0
            });
        }
        const trail = _trailHistory.get(id);
        const h = trail.head;
        trail.positions[h * 3] = peData.positions[i * 3];
        trail.positions[h * 3 + 1] = peData.positions[i * 3 + 1];
        trail.positions[h * 3 + 2] = peData.positions[i * 3 + 2];
        trail.head = (h + 1) % TRAIL_MAX_LENGTH;
        trail.length = Math.min(trail.length + 1, TRAIL_MAX_LENGTH);
    }
    // Remove trails for particles that no longer exist
    const activeIds = new Set();
    for (let i = 0; i < peData.count; i++) activeIds.add(peData.ids[i]);
    for (const [id] of _trailHistory) {
        if (!activeIds.has(id)) _trailHistory.delete(id);
    }
}

// ── Toast Notification System ────────────────────────────────────────
function showToast(msg, severity = 'info') {
    const container = document.getElementById('toast-container');
    if (!container) return;
    const toast = document.createElement('div');
    toast.className = `toast toast-${severity}`;
    toast.innerHTML = `<span>${msg}</span><button onclick="this.parentElement.remove()">&times;</button>`;
    container.appendChild(toast);
    setTimeout(() => { if (toast.parentElement) toast.remove(); }, 8000);
}

// ── Initialization ───────────────────────────────────────────────────
async function init() {
    if (_initialized) return;
    _initialized = true;

    // Cache frequently-accessed DOM elements for animation loops
    _cacheDOM();

    // Create bridge (tries WASM, falls back to mock)
    const latticeSize = parseInt(document.getElementById('lattice-size').value);
    bridge = await createBridge(latticeSize);

    // Update engine type indicator
    const engineEl = document.getElementById('status-engine');
    if (bridge.isWasm && bridge.ready) {
        engineEl.textContent = 'WASM Engine';
        engineEl.style.color = '#4ade80';
    } else {
        engineEl.textContent = 'Mock Engine';
        engineEl.style.color = '#fbbf24';
        showToast('WASM engine unavailable — running in Mock mode. Physics is approximate.', 'warning');
    }

    // Create 3D viewport
    const viewportContainer = document.getElementById('viewport');
    viewport = new Viewport(viewportContainer);
    viewport.setLatticeSize(latticeSize);

    // Create panels
    diagnostics = new DiagnosticsPanel();
    fluxEnergyChart = new FluxEnergyChart(document.getElementById('chart-flux-energy'));
    particleChart = new ParticleChart(document.getElementById('chart-particles'));
    lagrangianChart = new LagrangianChart(document.getElementById('chart-lagrangian'));
    inspector = new Inspector(viewport, bridge);
    peTelemetry = new PETelemetryPanel();

    // Populate constants table from WASM if available
    populateConstants();

    // Build element scenario dropdown (molecules are Scale 3 only)
    buildElementScenarios();
    buildScale3MoleculeDropdown();

    // Phase 1: Ontic Observatory
    observatory = new OnticObservatory();
    aggregateDetector = new AggregateDetector();
    scaleBridgeViz = new ScaleBridgeVisualizer();
    emergenceMonitor = new EmergenceMonitor(500);
    initOnticPhysicsHierarchy();

    // Wire up all UI controls
    wireToolbar();
    wireTabs();
    wireControls();
    wireViewportToggles();
    wireKeyboard();

    // Initialize Particle Zoo panel
    initZoo(bridge);

    // Enable flux volume visualization by default for Scale 0
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
        if (_fluxMock && _fluxMock.setBoundaryShape) _fluxMock.setBoundaryShape(shape);
        // Force immediate re-render so flux volume/particles clip to new boundary
        _latticeNeedsUpload = true;
    });

    // Reflective boundary toggle — when unchecked, flux/particles dissipate past boundary
    const reflectiveCheck = document.getElementById('reflective-boundary');
    reflectiveCheck.addEventListener('change', () => {
        const on = reflectiveCheck.checked;
        if (bridge && bridge.setReflectiveBoundary) bridge.setReflectiveBoundary(on);
        if (_fluxMock && _fluxMock.setReflectiveBoundary) _fluxMock.setReflectiveBoundary(on);
    });

    // Load default scenario (flux-pulse: pure substrate wave propagation)
    loadScenario('flux-pulse');

    // Start frame loop
    requestAnimationFrame(animate);
}

// ── Frame Loop ───────────────────────────────────────────────────────
function animate(now) {
    requestAnimationFrame(animate);

    if (engineMode === 'consciousness') {
        animateConsciousness(now);
    } else if (engineMode === 'atoms' || engineMode === 'molecules') {
        animateAE(now);
    } else if (engineMode === 'particles') {
        animatePE(now);
    } else {
        animateLattice(now);
    }

    // Animate environment background
    if (bgManager) bgManager.update(1 / 60);

    // FPS counter
    frameCount++;
    if (now - lastFpsTime >= 1000) {
        fpsDisplay = frameCount;
        frameCount = 0;
        lastFpsTime = now;
        _dom.statusFps.textContent = fpsDisplay;
    }
}

let _latticeNeedsUpload = true; // set true on scenario load / step / resume

function animateLattice(now) {
    // Tick simulation if running
    if (running) {
        _tickAccumulator += ticksPerFrame;
        const wholeTicks = Math.floor(_tickAccumulator);
        _tickAccumulator -= wholeTicks;
        for (let i = 0; i < wholeTicks; i++) {
            bridge.tick();
            // Tick parallel JS flux + particle simulation for visualization
            if (_fluxMock) _fluxMock.tick();
        }
        _latticeNeedsUpload = true;
    }

    // Only re-upload GPU buffers when data has actually changed
    if (_latticeNeedsUpload) {
        let particleData = bridge.getParticleData();
        // Fall back to MockBridge particles when main bridge has none (JS-only scenarios)
        if ((!particleData || particleData.count === 0) && _fluxMock) {
            const mockPD = _fluxMock.getParticleData();
            if (mockPD && mockPD.count > 0) particleData = mockPD;
        }
        viewport.updateParticles(particleData);

        // Confinement string rendering
        if (_fluxMock && _fluxMock._toggles.confinement) {
            viewport.updateConfinementStrings(_fluxMock._particles, bridge.latticeSize || 32);
            viewport.toggleConfinementStrings(true);
        } else {
            viewport.toggleConfinementStrings(false);
        }

        // Flux volume/slice rendering for Scale 0
        const L = bridge.latticeSize || 32;
        if (viewport.showFlux) {
            // Try WASM first, fall back to MockBridge JS wave sim
            let vol = bridge.getFluxVolume();
            if ((!vol || vol.length === 0) && _fluxMock) {
                vol = _fluxMock.getFluxVolume();
            }
            if (vol && vol.length > 0) {
                viewport.updateFluxVolume(vol, L);
            }
        }
        if (viewport.showHeatmap) {
            const sliceIdx = Math.floor(L / 2);
            let slice = bridge.getFluxSlice(1, sliceIdx);
            if ((!slice || slice.length === 0) && _fluxMock) {
                slice = _fluxMock.getFluxSlice(1, sliceIdx);
            }
            if (slice && slice.length > 0) {
                viewport.updateFluxSlice(slice, L, 1, sliceIdx);
            }
        }

        _latticeNeedsUpload = false;
    }

    // ── Field visualization updates (independent of lattice upload) ──
    // Runs every frame but self-throttles to every 3rd frame for perf.
    // _fieldNeedsUpdate bypasses throttle for immediate response on toggle.
    _fieldFrame++;

    if (_anyFieldActive && (_fieldNeedsUpdate || _fieldFrame % 3 === 0)) {
        _fieldNeedsUpdate = false;
        const fieldBridge = _fluxMock || bridge;
        const L = bridge.latticeSize || 32;
        const stride = L > 32 ? 4 : 2;

        // E-field streamlines
        if (_showEField) {
            const eData = fieldBridge.getEFieldSampled(stride);
            if (eData.count > 0) {
                const pData = bridge.getParticleData();
                const particles = [];
                for (let i = 0; i < pData.count; i++) {
                    particles.push({ x: pData.positions[i*3], y: pData.positions[i*3+1], z: pData.positions[i*3+2] });
                }
                const seeds = particles.length > 0 ? generateEFieldSeeds(particles, 2, 120) : generateGridSeeds(L, 8, 120);
                const lines = computeStreamlines(eData, seeds, { N: L, stride, maxSteps: 80, stepSize: 0.6 });
                viewport.updateEFieldLines(lines);
            }
        }

        // B-field streamlines
        if (_showBField) {
            const bData = fieldBridge.getBFieldSampled(stride);
            if (bData.count > 0) {
                const pData = bridge.getParticleData();
                const particles = [];
                for (let i = 0; i < pData.count; i++) {
                    particles.push({ x: pData.positions[i*3], y: pData.positions[i*3+1], z: pData.positions[i*3+2] });
                }
                const seeds = particles.length > 0 ? generateBFieldSeeds(particles, 4, 120) : generateGridSeeds(L, 8, 120);
                const lines = computeStreamlines(bData, seeds, { N: L, stride, maxSteps: 150, stepSize: 0.5, bidirectional: false });
                viewport.updateBFieldLines(lines);
            }
        }

        // Poynting vectors
        if (_showPoynting) {
            const sData = fieldBridge.getPoyntingSampled(stride);
            if (sData.count > 0) viewport.updatePoyntingVectors(sData);
        }

        // Divergence field
        if (_showDivField) {
            const divData = fieldBridge.getDivJSampled(stride);
            if (divData.count > 0) viewport.updateDivergenceField(divData);
        }

        // Flux streamlines
        if (_showFluxLines) {
            const jData = fieldBridge.getFluxVectorSampled(stride);
            if (jData.count > 0) {
                const seeds = generateGridSeeds(L, 8, 150);
                const lines = computeStreamlines(jData, seeds, { N: L, stride, maxSteps: 80, stepSize: 0.5 });
                let maxFlux = 0;
                for (let i = 0; i < jData.count; i++) {
                    const m = Math.sqrt(jData.vectors[i*3]**2 + jData.vectors[i*3+1]**2 + jData.vectors[i*3+2]**2);
                    if (m > maxFlux) maxFlux = m;
                }
                viewport.updateFluxStreamlines(lines, maxFlux);
            }
        }

        // Force volume
        if (_showForceVolume) {
            const fData = fieldBridge.getForceFieldSampled(stride);
            if (fData.count > 0) viewport.updateForceVolume(fData);
        }

        // Gravity field (density gradient)
        if (_showGravityField) {
            const gData = fieldBridge.getGravityFieldSampled(stride);
            if (gData.count > 0) viewport.updateGravityField(gData);
        }

        // Dark matter halo (sub-threshold flux envelope)
        if (_showDarkMatterHalo && fieldBridge._fluxJ) {
            const N = fieldBridge.latticeSize;
            const total = N * N * N;
            // Compute per-voxel flux magnitude for the halo overlay
            if (!fieldBridge._fluxMagBuf || fieldBridge._fluxMagBuf.length !== total) {
                fieldBridge._fluxMagBuf = new Float32Array(total);
            }
            const J = fieldBridge._fluxJ;
            for (let i = 0; i < total; i++) {
                const jx = J[i*3], jy = J[i*3+1], jz = J[i*3+2];
                fieldBridge._fluxMagBuf[i] = Math.sqrt(jx*jx + jy*jy + jz*jz);
            }
            viewport.updateDarkMatterHalo(fieldBridge._particles, fieldBridge._fluxMagBuf, N);
        }

        // Selective damping zones (wireframe cubes around damped voxels)
        if (_showDampingZones) {
            viewport.updateDampingZones(fieldBridge._particles, fieldBridge.latticeSize);
        }

        // Genesis threshold isosurface (birth boundary)
        if (_showGenesisIsosurface && fieldBridge._fluxJ) {
            const N = fieldBridge.latticeSize;
            const total = N * N * N;
            if (!fieldBridge._fluxMagBuf || fieldBridge._fluxMagBuf.length !== total) {
                fieldBridge._fluxMagBuf = new Float32Array(total);
            }
            const J = fieldBridge._fluxJ;
            for (let i = 0; i < total; i++) {
                const jx = J[i*3], jy = J[i*3+1], jz = J[i*3+2];
                fieldBridge._fluxMagBuf[i] = Math.sqrt(jx*jx + jy*jy + jz*jz);
            }
            viewport.updateGenesisIsosurface(fieldBridge._fluxMagBuf, N, K_GENESIS);
        }

        // Dual substrate (uses flux data split into L/R via delta)
        if (_showDualSubstrate) {
            const jData = fieldBridge.getFluxVectorSampled(stride);
            if (jData.count > 0) {
                const DELTA = 0.9568;
                const lFactor = (1 + DELTA) / 2;
                const rFactor = (1 - DELTA) / 2;
                const lVecs = new Float32Array(jData.vectors.length);
                const rVecs = new Float32Array(jData.vectors.length);
                for (let i = 0; i < jData.vectors.length; i++) {
                    lVecs[i] = jData.vectors[i] * lFactor;
                    rVecs[i] = jData.vectors[i] * rFactor;
                }
                viewport.updateDualFluxVolume(
                    { positions: jData.positions, vectors: lVecs, count: jData.count },
                    { positions: jData.positions, vectors: rVecs, count: jData.count }
                );
            }
        }

        // Chirality (|J_L| - |J_R| as scalar field)
        if (_showChirality) {
            const jData = fieldBridge.getFluxVectorSampled(stride);
            if (jData.count > 0) {
                const DELTA = 0.9568;
                const lF = (1 + DELTA) / 2, rF = (1 - DELTA) / 2;
                const values = new Float32Array(jData.count);
                for (let i = 0; i < jData.count; i++) {
                    const jx = jData.vectors[i*3], jy = jData.vectors[i*3+1], jz = jData.vectors[i*3+2];
                    const mag = Math.sqrt(jx*jx + jy*jy + jz*jz);
                    values[i] = mag * (lF - rF);
                }
                viewport.updateChiralityField({ positions: jData.positions, values, count: jData.count });
            }
        }

        // Light field (|Poynting| glow — reuses Poynting data if already fetched)
        if (_showLight) {
            const sData = fieldBridge.getPoyntingSampled(stride);
            if (sData.count > 0) viewport.updateLightField(sData);
        }
    }

    viewport.render();

    // Get diagnostics (throttle updates to every 3 frames for perf)
    if (frameCount % 3 === 0) {
        // Primary diagnostics from the WASM bridge (authoritative for particles,
        // energy, tick count). Fall back to MockBridge only when WASM has no
        // manifested particles AND the mock has flux data (JS-only wave demos).
        const wasmDiag = bridge.getDiagnostics();
        const mockDiag = _fluxMock ? _fluxMock.getDiagnostics() : null;
        const diag = (mockDiag && !wasmDiag.manifested && mockDiag.totalFlux > 0)
            ? { ...mockDiag, tick: wasmDiag.tick }
            : wasmDiag;

        // Update status bar
        _dom.statusTick.textContent = formatNumber(diag.tick);
        if (diag.physicalTime !== undefined) {
            _dom.statusPtime.textContent = formatNumber(Math.round(diag.physicalTime));
        } else {
            _dom.statusPtime.textContent = formatNumber(diag.tick);
        }
        // Scale 0 shows flux stats; manifested count should be 0 for flux-only scenarios
        _dom.statusParticles.textContent = diag.manifested || 0;
        _dom.statusEnergy.textContent = formatEnergy(diag.totalEnergy, 0).text;

        // Update status dot
        if (running) {
            _dom.statusDot.classList.remove('idle');
            _dom.statusState.textContent = 'Running';
        } else {
            _dom.statusDot.classList.add('idle');
            _dom.statusState.textContent = 'Idle';
        }

        // Always accumulate data for all panels
        diagnostics.update(diag);
        fluxEnergyChart.push(diag);
        particleChart.push(diag);

        const lag = _fluxMock ? _fluxMock.getLagrangian() : bridge.getLagrangian();
        lagrangianChart.push(lag);

        // Update active panel visuals
        switch (activeTab) {
            case 'diagnostics':
                diagnostics.drawSparklines();
                if (peTelemetry) peTelemetry.drawCharts();
                const ea = _fluxMock ? _fluxMock.getEnergyAudit() : bridge.getEnergyAudit();
                diagnostics.updateEnergyAudit(ea);
                break;
            case 'charts':
                fluxEnergyChart.draw();
                particleChart.draw();
                break;
            case 'lagrangian':
                lagrangianChart.draw();
                break;
            case 'inspector':
                inspector.update();
                break;
            case 'ontic':
                updateOnticPanel();
                break;
            case 'hierarchy':
                updateHierarchyPanel();
                break;
        }
    }
}

function animatePE(now) {
    // Tick PE simulation if running
    if (running) {
        _tickAccumulator += ticksPerFrame;
        const wholeTicks = Math.floor(_tickAccumulator);
        _tickAccumulator -= wholeTicks;
        for (let i = 0; i < wholeTicks; i++) {
            bridge.peTick();
        }
    }

    // Get particle centers, expand each into a flux cloud with animated motion
    const peData = bridge.peGetParticleData();
    const typeMap = bridge.peGetParticleTypes();
    const t = now * 0.001; // seconds for smooth animation
    const cloud = expandPEToCloud(peData, typeMap, t);
    viewport.updateParticles(cloud);

    // Update inspector with cloud-to-particle mapping
    if (inspector) {
        inspector.setPEContext(_cloudParticleMap, cloud.count, typeMap);
    }

    // Update velocity vectors overlay
    if (_showVelocities && peData.count > 0) {
        viewport.updateVelocityVectors(peData.positions, peData.velocities, peData.count);
    }

    // Update orbit trails
    if (running && peData.count > 0) {
        updateTrailHistory(peData);
    }
    if (_showTrails) {
        viewport.updateTrails(_trailHistory, typeMap);
    }

    // ── PE Field Overlays (individual force decomposition) ──────────

    // Coulomb potential heatmap + force vectors (XZ plane)
    if (_showPEPotential && peData.count > 0) {
        if (!_fieldGrid) _fieldGrid = generateGridXZ(25, 20);
        const src = bridge.peGetFieldSources();
        const field = samplePECoulombOnly(src, _fieldGrid.positions, _fieldGrid.count);
        viewport.updateFieldHeatmap(_fieldGrid.positions, field.potentials, _fieldGrid.count, field.maxPotential);
        viewport.updateFieldVectors(_fieldGrid.positions, field.forces, _fieldGrid.count, field.maxForce, 8.0);
    }

    // Coulomb E-field streamlines (3D, throttled every 5 frames)
    if (_showPEEField && peData.count > 0 && frameCount % 5 === 0) {
        const src = bridge.peGetFieldSources();
        const fieldFn = makePECoulombFieldFn(src, 0.5);
        // Convert flat positions array to {x,y,z} objects for generateEFieldSeeds
        // Reuse buffer — resize only when particle count changes
        while (_srcParticlesBuf.length < src.count) _srcParticlesBuf.push({ x: 0, y: 0, z: 0 });
        _srcParticlesBuf.length = src.count;
        for (let i = 0; i < src.count; i++) {
            _srcParticlesBuf[i].x = src.positions[i*3];
            _srcParticlesBuf[i].y = src.positions[i*3+1];
            _srcParticlesBuf[i].z = src.positions[i*3+2];
        }
        const seeds = generateEFieldSeeds(_srcParticlesBuf, 3, 100);
        const lines = computeStreamlines({ fieldFn }, seeds, { maxSteps: 80, stepSize: 0.5, bounds: 30 });
        viewport.updatePEStreamlines(lines);
    }

    // Gravity field vectors (XZ plane)
    if (_showPEGravField && peData.count > 0) {
        if (!_fieldGrid) _fieldGrid = generateGridXZ(25, 20);
        const src = bridge.peGetFieldSources();
        const field = samplePEGravityField(src, _fieldGrid.positions, _fieldGrid.count);
        viewport.updateGravityVectors(_fieldGrid.positions, field.forces, _fieldGrid.count, field.maxForce);
    }

    // Per-particle net force arrows
    if (_showPEForces && peData.count > 0) {
        const fd = bridge.peGetForces();
        viewport.updateParticleForces(fd.positions, fd.forces, fd.count, fd.maxForce);
    }

    viewport.render();

    // PE diagnostics (throttled)
    if (frameCount % 3 === 0) {
        const diag = bridge.peGetDiagnostics();

        // Update status bar with PE-specific info
        _dom.statusTick.textContent = formatNumber(diag.tick);
        _dom.statusPtime.textContent = formatNumber(diag.tick);
        _dom.statusParticles.textContent = diag.particleCount;
        _dom.statusEnergy.textContent = formatEnergy(diag.totalEnergy, 1).text;

        // Update status dot
        if (running) {
            _dom.statusDot.classList.remove('idle');
            _dom.statusState.textContent = 'Running';
        } else {
            _dom.statusDot.classList.add('idle');
            _dom.statusState.textContent = 'Idle';
        }

        // Update PE telemetry panel
        const ext = bridge.peGetExtendedData();
        if (peTelemetry) peTelemetry.update(diag, ext);

        // Feed charts with adapted data
        const diagAdapted = {
            tick: diag.tick,
            manifested: diag.particleCount,
            positive: 0, negative: 0,
            totalFlux: 0, totalEnergy: diag.totalEnergy,
            fieldEnergy: diag.totalPE,
            kineticEnergy: diag.totalKE,
            peFlux: diag.totalPE,
        };
        fluxEnergyChart.push(diagAdapted);
        particleChart.push(diagAdapted);

        // Update active panel visuals
        switch (activeTab) {
            case 'charts':
                fluxEnergyChart.draw();
                particleChart.draw();
                break;
            case 'inspector':
                inspector.update();
                break;
            case 'ontic':
                updateOnticPanel();
                break;
            case 'hierarchy':
                updateHierarchyPanel();
                break;
        }
    }
}

function animateAE(now) {
    // Tick AE simulation if running
    if (running) {
        _tickAccumulator += ticksPerFrame;
        const wholeTicks = Math.floor(_tickAccumulator);
        _tickAccumulator -= wholeTicks;
        for (let i = 0; i < wholeTicks; i++) {
            try {
                bridge.aeTick();
            } catch (e) {
                console.error('[FTD] aeTick exception:', e);
                running = false;
                updatePlayButton();
                return;
            }
        }
    }

    // Get atom data (positions, colors, sizes — same format as particles)
    const atomData = bridge.aeGetAtomData();

    // Debug: detect problems (auto-pauses if detected)
    if (running && atomData.count > 0) {
        for (let i = 0; i < atomData.count; i++) {
            const x = atomData.positions[i * 3];
            const y = atomData.positions[i * 3 + 1];
            const z = atomData.positions[i * 3 + 2];
            if (!isFinite(x) || !isFinite(y) || !isFinite(z)) {
                console.error(`[FTD] Atom ${i} has non-finite position: (${x}, ${y}, ${z})`);
                running = false;
                updatePlayButton();
                break;
            }
            if (Math.abs(x) > 1e4 || Math.abs(y) > 1e4 || Math.abs(z) > 1e4) {
                console.warn(`[FTD] Atom ${i} flew off: (${x.toFixed(1)}, ${y.toFixed(1)}, ${z.toFixed(1)})`);
                running = false;
                updatePlayButton();
                break;
            }
        }
    }
    if (atomData.count === 0 && engineMode === 'molecules') {
        console.warn('[FTD] No atoms in molecule data — bridge may have been re-initialized');
    }

    // Render as orbital electron clouds or plain atom points
    let cloudData = null;
    if (_showOrbitalClouds && atomData.count > 0 && atomData.atomicNums) {
        const t = now * 0.001; // seconds for breathing animation
        cloudData = expandAEToOrbitalCloud(atomData, t);

        // Merge bonding electron clouds into particle data
        if (_bondStyle !== 'off' && atomData.bondCount > 0) {
            const bondCloud = generateBondingCloud(atomData);
            if (bondCloud.count > 0) {
                // Merge bond cloud points into cloud data
                const mergedCount = cloudData.count + bondCloud.count;
                const mp = new Float32Array(mergedCount * 3);
                const mc = new Float32Array(mergedCount * 3);
                const ms = new Float32Array(mergedCount);
                mp.set(cloudData.positions.subarray(0, cloudData.count * 3));
                mc.set(cloudData.colors.subarray(0, cloudData.count * 3));
                ms.set(cloudData.sizes.subarray(0, cloudData.count));
                mp.set(bondCloud.positions.subarray(0, bondCloud.count * 3), cloudData.count * 3);
                mc.set(bondCloud.colors.subarray(0, bondCloud.count * 3), cloudData.count * 3);
                ms.set(bondCloud.sizes.subarray(0, bondCloud.count), cloudData.count);
                cloudData = { positions: mp, colors: mc, sizes: ms, count: mergedCount };
            }
        }

        viewport.updateParticles(cloudData);
    } else {
        viewport.updateParticles(atomData);
    }

    // Pass AE context to inspector for click-to-inspect
    if (inspector) {
        if (_showOrbitalClouds && cloudData?.atomMap) {
            inspector.setAEContext(atomData, cloudData.atomMap, true);
        } else {
            inspector.setAEContext(atomData, null, false);
        }
    }

    // Update bond rendering
    if (_bondStyle === 'cylinders' && atomData.bondCount > 0) {
        viewport.updateBondCylinders(atomData);
        viewport.toggleBondCylinders(true);
        viewport.toggleBondLines(false);
    } else if (_bondStyle === 'lines' && atomData.bondCount > 0) {
        viewport.updateBondLines(atomData);
        viewport.toggleBondLines(true);
        viewport.toggleBondCylinders(false);
    } else {
        viewport.toggleBondCylinders(false);
        viewport.toggleBondLines(false);
    }

    // Update nucleus shells (strong force glow)
    if (_showNucleusShells && atomData.count > 0) {
        viewport.updateNucleusShells(atomData);
    }

    // Update orbital shell boundaries
    if (_showShellBounds && atomData.count > 0) {
        viewport.updateOrbitalShells(atomData, electronConfig, slaterZeff, A0_DISPLAY);
        viewport.toggleOrbitalShells(true);
    }

    // Update orbital lobes
    if (_showOrbitalLobes && atomData.count > 0) {
        viewport.updateOrbitalLobes(atomData, electronConfig, slaterZeff, A0_DISPLAY);
        viewport.toggleOrbitalLobes(true);
    }

    // Update per-atom force arrows (every 2nd frame for performance)
    const anyForce = _showAEForceIonic || _showAEForceVdw || _showAEForceBond || _showAEForceNet;
    if (anyForce && atomData.count > 0) {
        _forceFrame++;
        if (_forceFrame % 2 === 0) {
            const forceData = bridge.aeGetForceDecomposition();
            viewport.updateAEForces(atomData.positions, forceData, forceData.count);
        }
    }

    // Update element labels (always show for atoms)
    if (atomData.count > 0 && atomData.atomicNums) {
        const labels = [];
        for (let i = 0; i < atomData.count; i++) {
            const Z = atomData.atomicNums[i];
            const sym = elementSymbol(Z);
            // Convert CPK color to CSS hex for canvas rendering
            const r = Math.round(atomData.colors[i * 3] * 255);
            const g = Math.round(atomData.colors[i * 3 + 1] * 255);
            const b = Math.round(atomData.colors[i * 3 + 2] * 255);
            // Use white text unless atom is very light-colored
            const lum = 0.299 * r + 0.587 * g + 0.114 * b;
            const hexColor = lum > 200 ? '#aaaaaa' : '#ffffff';
            labels.push({
                x: atomData.positions[i * 3],
                y: atomData.positions[i * 3 + 1],
                z: atomData.positions[i * 3 + 2],
                symbol: sym,
                color: hexColor,
            });
        }
        viewport.updateElementLabels(labels);
    } else {
        viewport.updateElementLabels(null);
    }

    // Update element legend (only rebuild when set of elements changes)
    if (_dom.aeLegend && atomData.count > 0 && atomData.atomicNums) {
        const zSet = new Set();
        for (let i = 0; i < atomData.count; i++) zSet.add(atomData.atomicNums[i]);
        const key = [...zSet].sort((a, b) => a - b).join(',') + (_showOrbitalClouds ? '+c' : '');
        if (key !== _prevLegendKey) {
            _prevLegendKey = key;
            let html = '<div class="ae-legend-header">Elements</div>';
            for (const Z of [...zSet].sort((a, b) => a - b)) {
                const el = getElement(Z);
                const [r, g, b] = el.color;
                const hex = `#${(r * 255 | 0).toString(16).padStart(2, '0')}${(g * 255 | 0).toString(16).padStart(2, '0')}${(b * 255 | 0).toString(16).padStart(2, '0')}`;
                html += `<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:${hex}"></span><span class="ae-legend-sym">${el.symbol}</span><span class="ae-legend-name">${el.name}</span></div>`;
            }
            if (_showOrbitalClouds) {
                html += '<div class="ae-legend-sep"></div><div class="ae-legend-header">Substructure</div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#ff4d33"></span><span class="ae-legend-name">Protons</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#4d80e6"></span><span class="ae-legend-name">Neutrons</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#66bfff"></span><span class="ae-legend-name">s orbitals</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#4de673"></span><span class="ae-legend-name">p orbitals</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#ffb333"></span><span class="ae-legend-name">d orbitals</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#d94db3"></span><span class="ae-legend-name">f orbitals</span></div>';
            }
            _dom.aeLegend.innerHTML = html;
        }
    } else if (_dom.aeLegend) {
        if (_prevLegendKey !== '') { _dom.aeLegend.innerHTML = ''; _prevLegendKey = ''; }
    }

    // Update force field overlay (heatmap + vectors)
    if (_showAEField && atomData.count > 0) {
        // Auto-compute grid extent from atom bounding box
        let maxR = 5;
        for (let i = 0; i < atomData.count; i++) {
            const ax = Math.abs(atomData.positions[i * 3]);
            const az = Math.abs(atomData.positions[i * 3 + 2]);
            if (ax > maxR) maxR = ax;
            if (az > maxR) maxR = az;
        }
        const extent = maxR + 5; // padding around atoms
        if (!_fieldGrid || Math.abs(_fieldGrid.extent - extent) > 1) {
            _fieldGrid = generateGridXZ(extent, 20);
        }
        const src = bridge.aeGetFieldSources();
        const field = sampleAEField(src, _fieldGrid.positions, _fieldGrid.count);
        viewport.updateFieldHeatmap(_fieldGrid.positions, field.potentials, _fieldGrid.count, field.maxPotential);
        viewport.updateFieldVectors(_fieldGrid.positions, field.forces, _fieldGrid.count, field.maxForce, 3.0);
    }

    viewport.render();

    // AE diagnostics (throttled)
    if (frameCount % 3 === 0) {
        const diag = bridge.aeGetDiagnostics();

        // Update status bar
        _dom.statusTick.textContent = formatNumber(diag.tick);
        _dom.statusPtime.textContent = formatNumber(diag.tick);
        _dom.statusParticles.textContent = diag.atomCount;
        _dom.statusEnergy.textContent = formatEnergy(diag.totalEnergy, 2).text;

        // Update status dot
        if (running) {
            _dom.statusDot.classList.remove('idle');
            _dom.statusState.textContent = 'Running';
        } else {
            _dom.statusDot.classList.add('idle');
            _dom.statusState.textContent = 'Idle';
        }

        // Update AE diagnostic cards
        _dom.aeDiagCount.textContent = diag.atomCount;
        _dom.aeDiagBonds.textContent = diag.bondCount;
        _dom.aeDiagKe.textContent = formatEnergy(diag.totalKE, 2).text;
        _dom.aeDiagEtotal.textContent = formatEnergy(diag.totalEnergy, 2).text;
        _dom.aeDiagPeIonic.textContent = formatEnergy(diag.totalPEIonic, 2).text;
        _dom.aeDiagPeVdw.textContent = formatEnergy(diag.totalPEVdw, 2).text;
        _dom.aeDiagPeBond.textContent = formatEnergy(diag.totalPEBond, 2).text;
        _dom.aeDiagTemp.textContent = formatTemperature(diag.temperature, 2).text;
        const pMag = Math.sqrt(diag.momentumX ** 2 + diag.momentumY ** 2 + diag.momentumZ ** 2);
        _dom.aeDiagMomentum.textContent = pMag.toFixed(6) + ' AMU\u00b7\u00c5/step';
        _dom.aeDiagTick.textContent = formatNumber(diag.tick);

        // Energy drift tracking (reference captured at load time; fallback here for safety)
        if (_aeInitialEnergy === null && diag.totalEnergy !== 0) {
            _aeInitialEnergy = diag.totalEnergy;
        }
        if (_aeInitialEnergy !== null) {
            const drift = ((diag.totalEnergy - _aeInitialEnergy) / Math.abs(_aeInitialEnergy)) * 100;
            _dom.aeDiagDrift.textContent = drift.toFixed(4) + '%';
        }

        // ── Atomic energy physics display ──
        updateAtomicEnergyDisplay(atomData);

        // Feed charts with adapted data
        const diagAdapted = {
            tick: diag.tick,
            manifested: diag.atomCount,
            positive: 0, negative: 0,
            totalFlux: 0, totalEnergy: diag.totalEnergy,
            fieldEnergy: diag.totalPEIonic + diag.totalPEVdw + diag.totalPEBond,
            kineticEnergy: diag.totalKE,
            peFlux: diag.totalPEIonic,
        };
        fluxEnergyChart.push(diagAdapted);
        particleChart.push(diagAdapted);

        // Update active panel visuals
        switch (activeTab) {
            case 'charts':
                fluxEnergyChart.draw();
                particleChart.draw();
                break;
            case 'inspector':
                inspector.update();
                break;
            case 'ontic':
                updateOnticPanel();
                break;
            case 'hierarchy':
                updateHierarchyPanel();
                break;
        }
    }
}

/**
 * Update the atomic energy display cards based on current atom data.
 * Shows nuclear binding energy, B/A, electron binding, and FTD mass for
 * single elements or the entire periodic table.
 */
function updateAtomicEnergyDisplay(atomData) {
    if (!_dom.aeDiagMass || !atomData || atomData.count === 0) return;

    if (atomData.count === 1 && atomData.atomicNums) {
        // ── Single element ──
        const Z = atomData.atomicNums[0];
        const e = atomicEnergy(Z);
        _dom.aeDiagMass.textContent = formatEnergyAE(e.massEnergy);
        _dom.aeDiagNbe.textContent = formatEnergyAE(e.bindingEnergy);
        _dom.aeDiagBa.textContent = e.bindingPerNucleon.toFixed(4) + ' MeV';
        _dom.aeDiagEbe.textContent = (e.electronBinding / 1000).toFixed(2) + ' keV';
        _dom.aeDiagMassKb.textContent = formatSI(e.massInKB) + ' k\u0299';
    } else if (atomData.atomicNums) {
        // ── Multiple elements (periodic table or molecule) ──
        let totalMass = 0, totalBE = 0, totalNucleons = 0, totalEBE = 0;
        for (let i = 0; i < atomData.count; i++) {
            const Z = atomData.atomicNums[i];
            const e = atomicEnergy(Z);
            totalMass += e.massEnergy;
            totalBE += e.bindingEnergy;
            totalNucleons += e.massNumber;
            totalEBE += e.electronBinding;
        }
        const avgBA = totalNucleons > 0 ? totalBE / totalNucleons : 0;
        _dom.aeDiagMass.textContent = formatEnergyAE(totalMass);
        _dom.aeDiagNbe.textContent = formatEnergyAE(totalBE);
        _dom.aeDiagBa.textContent = avgBA.toFixed(4) + ' MeV';
        _dom.aeDiagEbe.textContent = (totalEBE / 1e6).toFixed(2) + ' MeV';
        _dom.aeDiagMassKb.textContent = formatSI(totalMass / 0.51099895);
    }
}

/** Format a large number with SI suffix. */
function formatSI(n) {
    if (Math.abs(n) >= 1e12) return (n / 1e12).toFixed(2) + 'T';
    if (Math.abs(n) >= 1e9) return (n / 1e9).toFixed(2) + 'G';
    if (Math.abs(n) >= 1e6) return (n / 1e6).toFixed(2) + 'M';
    if (Math.abs(n) >= 1e3) return (n / 1e3).toFixed(2) + 'K';
    return n.toFixed(2);
}

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
            bridge.tick();
            if (_fluxMock) _fluxMock.tick();
        } else if (engineMode === 'atoms' || engineMode === 'molecules') {
            bridge.aeTick();
        } else if (engineMode === 'particles') {
            bridge.peTick();
        } else {
            bridge.tick();
            if (_fluxMock) _fluxMock.tick();
            _latticeNeedsUpload = true;
        }
    });

    // Reset
    document.getElementById('btn-reset').addEventListener('click', () => {
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
            loadScenario(scenario);
        }
    });

    // Scenario select
    document.getElementById('scenario-select').addEventListener('change', (e) => {
        running = false;
        updatePlayButton();
        loadScenario(e.target.value);
    });

    // Lattice size
    document.getElementById('lattice-size').addEventListener('change', (e) => {
        const size = parseInt(e.target.value);
        bridge.reset(size);
        viewport.setLatticeSize(size);
        clearCharts();
        const scenario = document.getElementById('scenario-select').value;
        loadScenario(scenario);
    });

    // Speed slider: 0..100 maps to ticks-per-frame via exponential+linear curve
    // 0→0.01 tpf (discrete), 50→1.0 tpf (smooth 60fps), 100→2.0 tpf (2× speed)
    const slider = document.getElementById('ticks-per-frame');
    const display = document.getElementById('tpf-display');
    function _sliderToSpeed(s) {
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
            if (_csEngine) _csEngine.setFigureType(e.target.value);
        });
    }

    // Consciousness audio toggle
    const csAudio = document.getElementById('cs-audio');
    if (csAudio) {
        csAudio.addEventListener('change', (e) => {
            if (_csEngine) {
                if (e.target.checked) {
                    const scenarioName = _csScenarioMeta?.name || 'cs-custom';
                    _csEngine.enableAudio(scenarioName);
                } else {
                    _csEngine.disableAudio();
                }
            }
        });
    }

    // Orbital cloud toggles (Scale 2 and Scale 3)
    const cloudToggle = document.getElementById('ae-show-clouds');
    if (cloudToggle) {
        cloudToggle.addEventListener('change', (e) => {
            _showOrbitalClouds = e.target.checked;
        });
    }
    const molCloudToggle = document.getElementById('mol-show-clouds');
    if (molCloudToggle) {
        molCloudToggle.addEventListener('change', (e) => {
            _showOrbitalClouds = e.target.checked;
        });
    }

    // ── Enhanced atom/molecule visual controls ──

    // Nucleus shells (strong force glow)
    const shellToggle = document.getElementById('ae-show-shells');
    if (shellToggle) {
        shellToggle.addEventListener('change', (e) => {
            _showNucleusShells = e.target.checked;
            viewport.toggleNucleusShells(_showNucleusShells);
        });
    }

    // Shell boundary spheres
    const shellBoundsToggle = document.getElementById('ae-show-shell-bounds');
    if (shellBoundsToggle) {
        shellBoundsToggle.addEventListener('change', (e) => {
            _showShellBounds = e.target.checked;
            viewport.toggleOrbitalShells(_showShellBounds);
        });
    }

    // Orbital lobes
    const lobeToggle = document.getElementById('ae-show-lobes');
    if (lobeToggle) {
        lobeToggle.addEventListener('change', (e) => {
            _showOrbitalLobes = e.target.checked;
            viewport.toggleOrbitalLobes(_showOrbitalLobes);
        });
    }

    // Bond style selector
    const bondStyleSelect = document.getElementById('bond-style-select');
    if (bondStyleSelect) {
        bondStyleSelect.addEventListener('change', (e) => {
            _bondStyle = e.target.value;
            viewport.toggleBondCylinders(_bondStyle === 'cylinders');
            viewport.toggleBondLines(_bondStyle === 'lines');
        });
    }

    // Force arrow toggles
    const forceToggles = [
        ['ae-force-ionic', '_showAEForceIonic', 'toggleAEForceIonic'],
        ['ae-force-vdw',   '_showAEForceVdw',   'toggleAEForceVdw'],
        ['ae-force-bond',  '_showAEForceBond',   'toggleAEForceBond'],
        ['ae-force-net',   '_showAEForceNet',    'toggleAEForceNet'],
    ];
    for (const [id, flag, method] of forceToggles) {
        const btn = document.getElementById(id);
        if (btn) {
            btn.addEventListener('click', () => {
                const isActive = btn.classList.toggle('active');
                switch (flag) {
                    case '_showAEForceIonic': _showAEForceIonic = isActive; break;
                    case '_showAEForceVdw':   _showAEForceVdw = isActive; break;
                    case '_showAEForceBond':  _showAEForceBond = isActive; break;
                    case '_showAEForceNet':   _showAEForceNet = isActive; break;
                }
                viewport[method](isActive);
            });
        }
    }
}

// ── Tab System ───────────────────────────────────────────────────────
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

            tabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');

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
        toggleBtn.addEventListener('click', () => {
            const app = document.getElementById('app');
            const collapsed = app.classList.toggle('panels-collapsed');
            toggleBtn.innerHTML = collapsed ? '&#9650;' : '&#9660;';
            toggleBtn.title = collapsed ? 'Expand panels' : 'Collapse panels';
            // Notify Three.js viewport of resize
            if (viewport && viewport.resize) setTimeout(() => viewport.resize(), 250);
        });
    }
}

// ── Consciousness Mode (Scale 4) ─────────────────────────────────────

function animateConsciousness(now) {
    // Tick the underlying lattice for flux data
    if (running && bridge) {
        _tickAccumulator += ticksPerFrame;
        const wholeTicks = Math.floor(_tickAccumulator);
        _tickAccumulator -= wholeTicks;
        for (let i = 0; i < wholeTicks; i++) {
            // Per-tick threshold injection: gradually build flux toward K_C
            if (_csScenarioMeta.name === 'cs-threshold') {
                const mid = Math.floor((bridge.latticeSize || 32) / 2);
                bridge.injectFlux(mid, mid, mid, 0.005, 0.003, 0.001);
            }
            bridge.tick();
        }
    }

    // Extract flux data for driving the hologram + diagnostics
    if (_csEngine && bridge) {
        const audit = bridge.getEnergyAudit ? bridge.getEnergyAudit() : {};
        const fieldE = audit.fieldEnergy || 0;
        const waveE = audit.waveEnergy || 0;
        // Normalize energies (rough — scale to 0-1 range)
        const maxE = Math.max(fieldE, 1);
        const nFlux = Math.min(fieldE / maxE, 1);
        const nWave = Math.min(waveE / Math.max(waveE, 1), 1);

        // Compute variance from diagnostic if available
        let variance = 0.1;
        let curlMag = 0.05;
        let centralDensity = 0.3;
        let polarity = 0;

        // Use energy audit fields if present
        if (audit.chargeTotal !== undefined) {
            polarity = Math.max(-1, Math.min(1, audit.chargeTotal / 10));
        }

        // Estimate variance from difference between max and mean
        if (fieldE > 0) {
            variance = Math.min(1, fieldE / 50);
            curlMag = Math.min(1, waveE / 20);
            centralDensity = Math.min(1, fieldE / 30);
        }

        // ── Dynamic Consciousness Diagnostics ──────────────────────
        // (computed before engine update so audio can use them)

        // 1. Flux ratio: peak flux / K_C
        const peakFlux = Math.sqrt(fieldE + waveE); // proxy for peak |J|
        const fluxRatio = K_C > 0 ? peakFlux / K_C : 0;

        // 2. Effective theta: object-dominant (low θ) vs subject-dominant (high θ)
        let effTheta;
        if (_csScenarioMeta.thetaMode === 'object') {
            effTheta = THETA_C_DEG * 0.7; // flow state — below critical angle
        } else if (_csScenarioMeta.thetaMode === 'subject') {
            effTheta = THETA_C_DEG * 1.3; // meditation — above critical angle
        } else if (_csScenarioMeta.thetaMode === 'dynamic') {
            // Dynamic: high wave energy → lower θ (object), high field energy → higher θ (subject)
            const totalE = fieldE + waveE + 0.001;
            const fieldFrac = fieldE / totalE;
            effTheta = THETA_C_DEG * (0.5 + fieldFrac); // range ~26°–52°+
        } else {
            effTheta = THETA_C_DEG; // static: exactly the theory value
        }

        // 3. Domain classification based on flux level
        let domainLabel;
        if (fluxRatio < 0.5) {
            domainLabel = 'Real (k=16)';
        } else if (fluxRatio < 1.0) {
            domainLabel = 'Degenerate';
        } else {
            domainLabel = 'Complex (k=½)';
        }
        // Override with scenario meta for boundary-orbit
        if (_csScenarioMeta.name === 'cs-threshold') {
            domainLabel = fluxRatio >= 1.0 ? 'Complex (k=½)' : fluxRatio >= 0.5 ? 'Degenerate' : 'Real (k=16)';
        }

        // 4. Effective y: scale Y_REAL and Y_IMAG by flux ratio
        const yScale = Math.min(fluxRatio, 2.0);
        const yRealEff = Y_REAL * yScale;
        const yImagEff = Y_IMAG * yScale;
        const yMag = Math.sqrt(yRealEff * yRealEff + yImagEff * yImagEff);

        // 5. Consciousness intensity: |y_eff| - K_C
        const consciousnessI = yMag - K_C;

        // 6. Mandelbrot iteration (boundary-orbit scenario)
        let mandelbrotDisplay = `c=${C_MANDELBROT.toFixed(3)}`;
        if (_csScenarioMeta.name === 'cs-boundary-orbit') {
            // One z→z²+c iteration per frame, c = C_MANDELBROT + tiny flux perturbation
            const c_re = C_MANDELBROT + (fluxRatio - 1.0) * 0.001;
            const c_im = 0;
            const new_re = _mandelbrotZ_re * _mandelbrotZ_re - _mandelbrotZ_im * _mandelbrotZ_im + c_re;
            const new_im = 2 * _mandelbrotZ_re * _mandelbrotZ_im + c_im;
            _mandelbrotZ_re = new_re;
            _mandelbrotZ_im = new_im;
            _mandelbrotIter++;
            const zMag = Math.sqrt(_mandelbrotZ_re * _mandelbrotZ_re + _mandelbrotZ_im * _mandelbrotZ_im);
            // Reset if escaped (|z| > 2)
            if (zMag > 2) { _mandelbrotZ_re = 0; _mandelbrotZ_im = 0; _mandelbrotIter = 0; }
            mandelbrotDisplay = `|z|=${zMag.toFixed(3)}`;
        }

        // ── Update Consciousness Engine (visual + audio) ─────────

        _csEngine.update({
            fluxEnergy: nFlux,
            waveEnergy: nWave,
            variance,
            curlMag,
            centralDensity,
            polarity,
            // Consciousness diagnostics for audio modulation
            fluxRatio,
            effTheta,
            consciousnessI,
            mandelbrotZ: Math.sqrt(_mandelbrotZ_re**2 + _mandelbrotZ_im**2),
        });

        // Update pedagogy panels with live engine data
        if (_csPedagogy) {
            _csPedagogy.update({ fluxRatio, effTheta, consciousnessI });
        }

        // ── DOM Updates ────────────────────────────────────────────

        // Row 2: Dynamic measurements
        const effThetaEl = document.getElementById('cs-diag-eff-theta');
        if (effThetaEl) {
            effThetaEl.textContent = `${effTheta.toFixed(1)}\u00B0`;
            effThetaEl.style.color = effTheta < THETA_C_DEG ? 'var(--accent)' : 'var(--consciousness-primary)';
        }

        const fluxRatioEl = document.getElementById('cs-diag-flux-ratio');
        if (fluxRatioEl) {
            fluxRatioEl.textContent = fluxRatio.toFixed(3);
            fluxRatioEl.style.color = fluxRatio >= 1.0 ? 'var(--consciousness-primary)' : 'var(--text-muted)';
        }

        const domainEl = document.getElementById('cs-diag-domain');
        if (domainEl) {
            domainEl.textContent = domainLabel;
            domainEl.style.color = domainLabel.includes('Complex') ? 'var(--consciousness-primary)' :
                                   domainLabel.includes('Degenerate') ? 'var(--warning)' : 'var(--text-muted)';
        }

        // Row 3: Consciousness metrics
        const yrEl = document.getElementById('cs-diag-yreal');
        if (yrEl) yrEl.textContent = yRealEff.toFixed(3);

        const yiEl = document.getElementById('cs-diag-yimag');
        if (yiEl) yiEl.textContent = `${yImagEff.toFixed(3)}i`;

        const intEl = document.getElementById('cs-diag-intensity');
        if (intEl) {
            intEl.textContent = consciousnessI.toFixed(3);
            intEl.style.color = consciousnessI > 0 ? '#00ff88' : '#ff4444';
        }

        const mandEl = document.getElementById('cs-diag-mandelbrot');
        if (mandEl) mandEl.textContent = mandelbrotDisplay;
    }

    // Render
    viewport.render();
}

function loadConsciousnessScenario(name) {
    _resetAllVisualState();

    // Initialize ConsciousnessEngine if not yet created
    if (!_csEngine && viewport) {
        _csEngine = new ConsciousnessEngine(viewport.scene);
    }

    // Initialize pedagogy panels and info tooltips
    if (!_csPedagogy) {
        _csPedagogy = new ConsciousnessPedagogy();
        addInfoTooltips();
        wireConsciousnessSubTabs();
    }

    // Create a flux-only MockBridge for lattice dynamics
    if (!bridge || !(bridge instanceof MockBridge)) {
        bridge = new MockBridge(32);
    }

    // Reset Mandelbrot iteration state
    _mandelbrotZ_re = 0; _mandelbrotZ_im = 0; _mandelbrotIter = 0;

    // Base toggles: flux-only mode (no particles, no forces — just waves)
    bridge.setToggle('wave_propagation', true);
    bridge.setToggle('coupling', false);
    bridge.setToggle('damping', true);
    bridge.setToggle('genesis', false);
    bridge.setToggle('gauss_projection', false);
    bridge.setToggle('forces', false);
    bridge.setToggle('gravity', false);
    bridge.setToggle('movement', false);
    bridge.setToggle('dual_substrate', false);

    // Set up scenario-specific flux patterns and toggle overrides
    switch (name) {
        case 'cs-threshold': {
            // Start below K_C with low-amplitude Gaussian, gradually build to cross real→complex boundary
            const csMid = Math.floor((bridge.latticeSize || 32) / 2);
            const csSubAmp = 0.511 * 0.3; // K_B * 0.3
            const csSigma = 4;
            for (let dz = -6; dz <= 6; dz++) for (let dy = -6; dy <= 6; dy++) for (let dx = -6; dx <= 6; dx++) {
                const r2 = dx * dx + dy * dy + dz * dz;
                const val = csSubAmp * Math.exp(-r2 / (2 * csSigma * csSigma));
                if (val > 0.001) bridge.injectFlux(csMid + dx, csMid + dy, csMid + dz, val, 0, 0);
            }
            _csScenarioMeta = { name, domain: 'Real (k=16)', thetaMode: 'dynamic', sloopDepth: 0, bellS: null };
            break;
        }
        case 'cs-high-coupling': {
            // 4-source interference + coupling + forces (psychedelic high-flux state)
            bridge.setToggle('coupling', true);
            bridge.setToggle('forces', true);
            bridge.setupScenario('flux-interference');
            _csScenarioMeta = { name, domain: 'Complex (k=½)', thetaMode: 'dynamic', sloopDepth: 0, bellS: null };
            break;
        }
        case 'cs-self-ref': {
            // Standing wave = observer meeting itself (sLoop depth 1)
            bridge.setupScenario('flux-standing');
            _csScenarioMeta = { name, domain: 'Complex (k=½)', thetaMode: 'static', sloopDepth: 1, bellS: null };
            break;
        }
        case 'cs-nested-sloop': {
            // Two orthogonal standing waves = self-aware of self-awareness (sLoop depth 2)
            bridge.setupScenario('flux-nested-standing');
            _csScenarioMeta = { name, domain: 'Complex (k=½)', thetaMode: 'static', sloopDepth: 2, bellS: null };
            break;
        }
        case 'cs-chirality': {
            // Dual substrate with asymmetric L/R injection
            bridge.setToggle('dual_substrate', true);
            bridge.setupScenario('flux-dual-substrate');
            _csScenarioMeta = { name, domain: 'Complex (k=½)', thetaMode: 'dynamic', sloopDepth: 1, bellS: null };
            break;
        }
        case 'cs-boundary-orbit': {
            // Mandelbrot c=1/G* iteration tracking
            bridge.setupScenario('flux-soliton');
            _csScenarioMeta = { name, domain: 'Degenerate', thetaMode: 'dynamic', sloopDepth: 1, bellS: null };
            break;
        }
        case 'cs-entangled': {
            // Full coupling: dipole + genesis + forces + movement
            bridge.setToggle('coupling', true);
            bridge.setToggle('genesis', true);
            bridge.setToggle('forces', true);
            bridge.setToggle('movement', true);
            bridge.setupScenario('flux-dipole');
            _csScenarioMeta = { name, domain: 'Complex (k=½)', thetaMode: 'dynamic', sloopDepth: 1, bellS: 2.0 };
            break;
        }
        case 'cs-flow': {
            // Fast vortex pattern, theta < 52.54 (object-dominant flow state)
            bridge.setupScenario('flux-vortex');
            _csScenarioMeta = { name, domain: 'Complex (k=½)', thetaMode: 'object', sloopDepth: 0, bellS: null };
            break;
        }
        case 'cs-meditation': {
            // Gentle centered pulse, theta > 52.54 (subject-dominant meditation)
            bridge.setupScenario('flux-pulse');
            _csScenarioMeta = { name, domain: 'Complex (k=½)', thetaMode: 'subject', sloopDepth: 0, bellS: null };
            break;
        }
        case 'cs-custom':
        default: {
            bridge.setupScenario('empty');
            _csScenarioMeta = { name: 'cs-custom', domain: '--', thetaMode: 'static', sloopDepth: 0, bellS: null };
            break;
        }
    }

    // Update static diagnostics from scenario metadata
    const sloopEl = document.getElementById('cs-diag-sloop');
    if (sloopEl) sloopEl.textContent = _csScenarioMeta.sloopDepth;
    const bellEl = document.getElementById('cs-diag-bell');
    if (bellEl) bellEl.textContent = _csScenarioMeta.bellS !== null ? `S=${_csScenarioMeta.bellS.toFixed(1)}` : '--';

    // Wire figure type selector
    const figSel = document.getElementById('cs-figure-select');
    if (figSel && _csEngine) {
        _csEngine.setFigureType(figSel.value);
    }

    // Wire audio toggle — pass scenario name so each gets unique sound
    const audioChk = document.getElementById('cs-audio');
    if (audioChk && _csEngine) {
        if (audioChk.checked) _csEngine.enableAudio(name);
        else _csEngine.disableAudio();
    }
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
        _latticeNeedsUpload = true;
    });

    document.getElementById('btn-inject-wave').addEventListener('click', () => {
        const { x, y, z, state } = _getInjPos();
        bridge.injectWavepacket(x, y, z, state);
        _latticeNeedsUpload = true;
    });

    document.getElementById('btn-inject-flux').addEventListener('click', () => {
        const { x, y, z } = _getInjPos();
        const kb = (bridge.getParam && bridge.getParam('kb')) || 0.511;
        bridge.injectFlux(x, y, z, kb * 0.8, 0, 0);
        _latticeNeedsUpload = true;
    });

    document.getElementById('btn-inject-pair').addEventListener('click', () => {
        const { x, y, z } = _getInjPos();
        const kb = (bridge.getParam && bridge.getParam('kb')) || 0.511;
        bridge.createEntangledPair(x, y, z, kb, 0, 0);
        _latticeNeedsUpload = true;
    });

    // ── Combo Panel: Parameter Sliders ──────────────────────────────
    const comboSliders = [
        { id: 'combo-kb',   valId: 'combo-kb-val',   param: 'kb',      fmt: 3 },
        { id: 'combo-gn',   valId: 'combo-gn-val',   param: 'gn',      fmt: 3 },
        { id: 'combo-damp', valId: 'combo-damp-val',  param: 'damping', fmt: 3 },
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
        _latticeNeedsUpload = true;
    });

    document.getElementById('btn-random-flux').addEventListener('click', () => {
        if (bridge.seedRandomFlux) {
            bridge.seedRandomFlux();
        }
        _latticeNeedsUpload = true;
    });

    // Quick actions
    document.getElementById('btn-enable-all').addEventListener('click', () => {
        for (const [elId] of Object.entries(toggleMap)) {
            const el = document.getElementById(elId);
            if (el) { el.checked = true; bridge.setToggle(toggleMap[elId], true); }
        }
    });

    document.getElementById('btn-disable-all').addEventListener('click', () => {
        for (const [elId] of Object.entries(toggleMap)) {
            const el = document.getElementById(elId);
            if (el) { el.checked = false; bridge.setToggle(toggleMap[elId], false); }
        }
    });

    document.getElementById('btn-clear-particles').addEventListener('click', () => {
        bridge.reset(bridge.latticeSize);
        viewport.setLatticeSize(bridge.latticeSize);
        clearCharts();
    });

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
        'pe-coulomb':         (v) => bridge.peSetCoulomb(v),
        'pe-gravity':         (v) => bridge.peSetGravity(v),
        'pe-damping':         (v) => bridge.peSetDamping(v),
        'pe-lorentz-p':       (v) => bridge.peSetLorentz(v),
        'pe-exchange':        (v) => bridge.peSetExchange(v),
        'pe-strong':          (v) => bridge.peSetStrong(v),
        'pe-magnetic-dipole': (v) => bridge.peSetMagneticDipole(v),
        'pe-spin-orbit':      (v) => bridge.peSetSpinOrbit(v),
        'pe-radiation':       (v) => bridge.peSetRadiation(v),
        'pe-relativistic':    (v) => bridge.peSetRelativistic(v),
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
        'ae-ionic':       (v) => bridge.aeSetIonic(v),
        'ae-vdw':         (v) => bridge.aeSetVdw(v),
        'ae-bonds-force': (v) => bridge.aeSetBondsForce(v),
        'ae-bonding':     (v) => bridge.aeSetBonding(v),
        'ae-damping':     (v) => bridge.aeSetDamping(v),
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
    const wireBtn = document.getElementById('toggle-wireframe');
    wireBtn.addEventListener('click', () => {
        wireBtn.classList.toggle('active');
        viewport.toggleWireframe(wireBtn.classList.contains('active'));
        _latticeNeedsUpload = true;
        // Keep universal Grid toggle in sync (wireframe IS the grid at Scale 0)
        const gb = document.getElementById('toggle-grid');
        if (gb) gb.classList.toggle('active', wireBtn.classList.contains('active'));
    });

    // Universal toggles (visible on all scales)
    const axesBtn = document.getElementById('toggle-axes');
    if (axesBtn) {
        axesBtn.addEventListener('click', () => {
            axesBtn.classList.toggle('active');
            viewport.toggleAxes(axesBtn.classList.contains('active'));
        });
    }
    const gridBtn = document.getElementById('toggle-grid');
    if (gridBtn) {
        gridBtn.addEventListener('click', () => {
            gridBtn.classList.toggle('active');
            viewport.toggleGrid(gridBtn.classList.contains('active'));
            // Keep Scale 0 wireframe button in sync when grid toggle changes it
            if (engineMode === 'lattice') {
                const wb = document.getElementById('toggle-wireframe');
                if (wb) wb.classList.toggle('active', gridBtn.classList.contains('active'));
            }
        });
    }

    const fluxVolBtn = document.getElementById('toggle-flux-volume');
    if (fluxVolBtn) {
        fluxVolBtn.addEventListener('click', () => {
            fluxVolBtn.classList.toggle('active');
            viewport.toggleFluxVolume(fluxVolBtn.classList.contains('active'));
            _latticeNeedsUpload = true; // force re-render on toggle
        });
    }

    const fluxSliceBtn = document.getElementById('toggle-flux-slice');
    if (fluxSliceBtn) {
        fluxSliceBtn.addEventListener('click', () => {
            fluxSliceBtn.classList.toggle('active');
            viewport.toggleFluxSlice(fluxSliceBtn.classList.contains('active'));
            _latticeNeedsUpload = true;
        });
    }

    // PE mode visual overlay toggles
    const velBtn = document.getElementById('toggle-velocities');
    velBtn.addEventListener('click', () => {
        velBtn.classList.toggle('active');
        _showVelocities = velBtn.classList.contains('active');
        viewport.toggleVelocityVectors(_showVelocities);
    });

    const trailBtn = document.getElementById('toggle-trails');
    trailBtn.addEventListener('click', () => {
        trailBtn.classList.toggle('active');
        _showTrails = trailBtn.classList.contains('active');
        viewport.toggleTrails(_showTrails);
    });

    // PE field overlay toggles (individual force decomposition)
    const peFieldToggles = [
        ['toggle-pe-efield',        (on) => { _showPEEField = on; viewport.togglePEStreamlines(on); }],
        ['toggle-pe-potential',     (on) => { _showPEPotential = on; viewport.toggleFieldHeatmap(on); viewport.toggleFieldVectors(on); }],
        ['toggle-pe-gravity-field', (on) => { _showPEGravField = on; viewport.toggleGravityVectors(on); }],
        ['toggle-pe-forces',        (on) => { _showPEForces = on; viewport.toggleParticleForces(on); }],
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
            _showAEField = aeFieldBtn.classList.contains('active');
            viewport.toggleFieldHeatmap(_showAEField);
            viewport.toggleFieldVectors(_showAEField);
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
    const fieldToggles = [
        ['toggle-e-field',        '_showEField',        (on) => { _showEField = on; viewport.toggleEFieldLines(on); _fieldNeedsUpdate = true; }],
        ['toggle-b-field',        '_showBField',        (on) => { _showBField = on; viewport.toggleBFieldLines(on); _fieldNeedsUpdate = true; }],
        ['toggle-poynting',       '_showPoynting',      (on) => { _showPoynting = on; viewport.togglePoyntingVectors(on); _fieldNeedsUpdate = true; }],
        ['toggle-div-field',      '_showDivField',      (on) => { _showDivField = on; viewport.toggleDivergenceField(on); _fieldNeedsUpdate = true; }],
        ['toggle-flux-lines',     '_showFluxLines',     (on) => { _showFluxLines = on; viewport.toggleFluxStreamlines(on); _fieldNeedsUpdate = true; }],
        ['toggle-force-volume',   '_showForceVolume',   (on) => { _showForceVolume = on; viewport.toggleForceVolume(on); _fieldNeedsUpdate = true; }],
        ['toggle-dual-substrate', '_showDualSubstrate', (on) => { _showDualSubstrate = on; viewport.toggleDualFluxVolume(on); _fieldNeedsUpdate = true; }],
        ['toggle-chirality',      '_showChirality',     (on) => { _showChirality = on; viewport.toggleChiralityField(on); _fieldNeedsUpdate = true; }],
        ['toggle-light',          '_showLight',         (on) => { _showLight = on; viewport.toggleLightField(on); _fieldNeedsUpdate = true; }],
        ['toggle-gravity-field',  '_showGravityField',  (on) => { _showGravityField = on; viewport.toggleGravityField(on); _fieldNeedsUpdate = true; }],
        ['toggle-dark-halo',      '_showDarkMatterHalo',(on) => { _showDarkMatterHalo = on; viewport.toggleDarkMatterHalo(on); _fieldNeedsUpdate = true; }],
        ['toggle-damping-zones',  '_showDampingZones',  (on) => { _showDampingZones = on; viewport.toggleDampingZones(on); _fieldNeedsUpdate = true; }],
        ['toggle-genesis-iso',    '_showGenesisIsosurface', (on) => { _showGenesisIsosurface = on; viewport.toggleGenesisIsosurface(on); _fieldNeedsUpdate = true; }],
    ];
    for (const [id, , handler] of fieldToggles) {
        const btn = document.getElementById(id);
        if (btn) {
            btn.addEventListener('click', () => {
                btn.classList.toggle('active');
                handler(btn.classList.contains('active'));
                _recomputeAnyFieldActive();
                _latticeNeedsUpload = true; // force immediate re-render on any toggle
            });
        }
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
                    if (_fluxMock) _fluxMock.tick();
                } else if (engineMode === 'atoms' || engineMode === 'molecules') {
                    bridge.aeTick();
                } else if (engineMode === 'particles') {
                    bridge.peTick();
                } else {
                    bridge.tick();
                    if (_fluxMock) _fluxMock.tick();
                    _latticeNeedsUpload = true;
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
                    loadScenario(scenario);
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
                '6': 'toggle-force-volume',
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

// ── Engine Mode Switching ────────────────────────────────────────────
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
    app.classList.toggle('mode-consciousness', mode === 'consciousness');

    // If the active tab is hidden for this scale, fall back to Controls
    const activeTabEl = document.querySelector('#tab-bar .tab.active');
    if (activeTabEl) {
        const scales = activeTabEl.dataset.scales;
        const scaleIndex = { lattice: '0', particles: '1', atoms: '2', molecules: '3', consciousness: '4' }[mode];
        if (scales && !scales.split(',').includes(scaleIndex)) {
            // Current tab not available at this scale — click Controls
            const controlsTab = document.querySelector('#tab-bar .tab[data-panel="controls"]');
            if (controlsTab) controlsTab.click();
        }
    }

    // Free JS flux sim when leaving Scale 0 (before loadXxx resets visual state)
    if (mode !== 'lattice') _fluxMock = null;

    // Tell viewport to switch reference frame
    viewport.setEngineMode(mode);

    // Tell inspector and zoo panel about mode change
    if (inspector) inspector.setEngineMode(mode);
    setZooMode(mode);

    // Cleanup consciousness engine when leaving Scale 4
    if (mode !== 'consciousness' && _csEngine) {
        _csEngine.dispose();
        _csEngine = null;
    }

    // Initialize the appropriate engine (each loadXxx calls _resetAllVisualState)
    if (mode === 'consciousness') {
        loadConsciousnessScenario(document.getElementById('cs-scenario-select').value);
    } else if (mode === 'molecules') {
        // Ensure bond lines are visible (atoms mode hides them)
        viewport.toggleBondLines(true);
        loadMoleculeScenario(document.getElementById('mol-scenario-select').value);
    } else if (mode === 'atoms') {
        // Clear bond lines from previous molecule session
        viewport.toggleBondLines(false);
        loadAEScenario(document.getElementById('ae-scenario-select').value);
    } else if (mode === 'particles') {
        loadPEScenario(document.getElementById('pe-scenario-select').value);
    } else {
        // Back to lattice — reload current lattice scenario
        // Enable flux volume by default for Scale 0
        const fluxVolBtn = document.getElementById('toggle-flux-volume');
        if (fluxVolBtn && !fluxVolBtn.classList.contains('active')) {
            fluxVolBtn.classList.add('active');
        }
        viewport.toggleFluxVolume(true);
        const scenario = document.getElementById('scenario-select').value;
        loadScenario(scenario);
    }
}

function loadPEScenario(name) {
    if (!bridge.initPE) return;
    _resetAllVisualState();
    bridge.initPE();

    // PE physics setup for atomic-scale simulations:
    // - Disable damping (Larmor radiation is Scale 0 only)
    // - Disable gravity (G_N=0.01 is lattice-scale; real α_G≈6e-39, negligible at atomic scale)
    // - Reduce softening so Coulomb force is strong enough for bound orbits
    // - Use tiny r_eff so particles don't annihilate at orbital distances
    bridge.peSetCoulomb(true);
    bridge.peSetDamping(false);
    bridge.peSetGravity(false);
    bridge.peSetSoftening(0.1);
    // Sync toggle checkboxes to match engine state
    const peCoulombEl = document.getElementById('pe-coulomb');
    const peGravityEl = document.getElementById('pe-gravity');
    const peDampingEl = document.getElementById('pe-damping');
    if (peCoulombEl) peCoulombEl.checked = true;
    if (peGravityEl) peGravityEl.checked = false;
    if (peDampingEl) peDampingEl.checked = false;
    // Sync dt to slider value (initPE defaults to 0.005 but ensure it matches UI)
    const dtSlider = document.getElementById('pe-dt-slider');
    if (dtSlider) bridge.peSetDt(parseFloat(dtSlider.value));

    // Orbital velocity for circular orbit around a central charge Q:
    //   Plummer force: F = α·|Q|·r / (4π·(r² + soft²)^(3/2))
    //   Equilibrium:   m·v²/r = F  →  v = sqrt(α·|Q|·r² / (4π·m·(r²+soft²)^(3/2)))
    //   For soft << r this simplifies to v ≈ sqrt(α·|Q|·r / (4π·m·(r²+soft²)))
    const ALPHA = 0.00729;
    const soft2 = 0.01;  // 0.1²
    const orbitalV = (m, r, Q = 1) =>
        Math.sqrt(ALPHA * Q * r / (4 * Math.PI * m * (r * r + soft2)));

    const me = 0.511;    // electron mass MeV
    const mp = 938.272;  // proton mass MeV
    const mmu = 105.658;  // muon mass MeV
    const mn = 939.565;  // neutron mass MeV
    const mpi  = 139.57;   // charged pion mass MeV
    const mK   = 493.68;   // charged kaon mass MeV
    const mtau = 1776.86;  // tau mass MeV
    const mW   = 80377.0;  // W boson mass MeV
    const mSig = 1189.4;   // Sigma+ mass MeV
    const mOmg = 1672.5;   // Omega- mass MeV
    const mDel = 1232.0;   // Delta++ mass MeV
    const RE = 0.1;      // effective radius for PE (tiny — no false annihilation)

    switch (name) {
        case 'pe-hydrogen': {
            // Locked proton at origin, electron in circular orbit at r=5
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }
        case 'pe-helium': {
            // Locked He nucleus (2p + 2n), 2 electrons orbiting
            const r = 4;
            const v = orbitalV(me, r, 2); // Q=2 for helium
            bridge.peAddLockedParticle('proton', 1, 0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('proton', 1, -0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, 0, 0.3, 0, mn, RE);
            bridge.peAddLockedParticle('neutron', 0, 0, -0.3, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('electron', -1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }
        case 'pe-positronium': {
            // e+ and e- orbiting their common center of mass
            // Each at distance r from center (separation = 2r)
            // Reduced mass = me/2, force = α/(4π·(2r)²)
            // v = sqrt(α·r / (4π·me·((2r)² + soft²)))
            const r = 5;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA * r / (4 * Math.PI * me * (sep * sep + soft2)));
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('positron', 1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }
        case 'pe-muonium': {
            // Locked mu+ at origin, e- orbiting at r=5
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('mu_plus', 1, 0, 0, 0, mmu, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }
        case 'pe-scattering': {
            // Proton and electron approaching with impact parameter b=3
            const v_app = 0.005;
            bridge.peAddParticle('proton', 1, -15, 0, 0, v_app, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, 15, 3, 0, -v_app * 10, 0, 0, me, RE);
            break;
        }
        case 'pe-three-body': {
            // Two protons separated + one electron between them
            const r = 8;
            const v = orbitalV(me, r, 2); // total charge Q=2
            bridge.peAddLockedParticle('proton', 1, -3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('proton', 1, 3, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, 0, r, 0, v, 0, 0, me, RE);
            break;
        }
        case 'pe-deuteron': {
            // Locked deuteron (p + n) + electron orbiting
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('proton', 1, 0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, -0.3, 0, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }
        // ── Lepton scenarios (new) ─────────────────────────────────
        case 'pe-true-muonium': {
            // μ⁺μ⁻ orbiting their common center of mass
            const r = 3;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA * r / (4 * Math.PI * mmu * (sep * sep + soft2)));
            bridge.peAddParticle('antimuon', 1, r, 0, 0, 0, v, 0, mmu, RE);
            bridge.peAddParticle('muon', -1, -r, 0, 0, 0, -v, 0, mmu, RE);
            break;
        }
        case 'pe-tauonium': {
            // τ⁺τ⁻ orbiting their common center of mass (tight orbit)
            const r = 2;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA * r / (4 * Math.PI * mtau * (sep * sep + soft2)));
            bridge.peAddParticle('antitau', 1, r, 0, 0, 0, v, 0, mtau, RE);
            bridge.peAddParticle('tau', -1, -r, 0, 0, 0, -v, 0, mtau, RE);
            break;
        }
        case 'pe-tau-atom': {
            // Tauonic hydrogen: locked proton, τ⁻ in tight orbit
            const r = 1.5;
            const v = orbitalV(mtau, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('tau', -1, r, 0, 0, 0, v, 0, mtau, RE);
            break;
        }

        // ── Exotic atom scenarios ─────────────────────────────────
        case 'pe-pionic-hydrogen': {
            // Pionic hydrogen: π⁻ orbiting locked proton
            const r = 4;
            const v = orbitalV(mpi, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('pion_minus', -1, r, 0, 0, 0, v, 0, mpi, RE);
            break;
        }
        case 'pe-kaonic-hydrogen': {
            // Kaonic hydrogen: K⁻ orbiting locked proton
            const r = 4;
            const v = orbitalV(mK, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('kaon_minus', -1, r, 0, 0, 0, v, 0, mK, RE);
            break;
        }
        case 'pe-sigma-plus-atom': {
            // Σ⁺ atom: electron orbiting locked Sigma+
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('sigma_plus', 1, 0, 0, 0, mSig, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }
        case 'pe-antiprotonic-hydrogen': {
            // Protonium: p and p̄ orbiting common center of mass
            const r = 3;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA * r / (4 * Math.PI * mp * (sep * sep + soft2)));
            bridge.peAddParticle('proton', 1, r, 0, 0, 0, v, 0, mp, RE);
            bridge.peAddParticle('antiproton', -1, -r, 0, 0, 0, -v, 0, mp, RE);
            break;
        }

        // ── Hadron scenarios ──────────────────────────────────────
        case 'pe-pion-orbit': {
            // Pionium: π⁺π⁻ Coulomb bound state
            const r = 4;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA * r / (4 * Math.PI * mpi * (sep * sep + soft2)));
            bridge.peAddParticle('pion_plus', 1, r, 0, 0, 0, v, 0, mpi, RE);
            bridge.peAddParticle('pion_minus', -1, -r, 0, 0, 0, -v, 0, mpi, RE);
            break;
        }
        case 'pe-kaon-pair': {
            // Kaonium: K⁺K⁻ Coulomb bound state
            const r = 4;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA * r / (4 * Math.PI * mK * (sep * sep + soft2)));
            bridge.peAddParticle('kaon_plus', 1, r, 0, 0, 0, v, 0, mK, RE);
            bridge.peAddParticle('kaon_minus', -1, -r, 0, 0, 0, -v, 0, mK, RE);
            break;
        }
        case 'pe-delta-system': {
            // Δ⁺⁺ system: two locked +1 charges (int8_t constraint) + 2 electrons
            const r = 4;
            const v = orbitalV(me, r, 2);
            bridge.peAddLockedParticle('delta_pp_a', 1, 0.3, 0, 0, mDel / 2, RE);
            bridge.peAddLockedParticle('delta_pp_b', 1, -0.3, 0, 0, mDel / 2, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('electron', -1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }
        case 'pe-omega-scattering': {
            // Ω⁻ locked at origin, positron approaching with impact parameter
            const v_app = 0.004;
            bridge.peAddLockedParticle('omega_minus', -1, 0, 0, 0, mOmg, RE);
            bridge.peAddParticle('positron', 1, -15, 2, 0, v_app, 0, 0, me, RE);
            break;
        }

        // ── Nuclear scenarios ─────────────────────────────────────
        case 'pe-tritium': {
            // Tritium: locked p + n + n nucleus, electron orbiting
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0.3, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, 0.3, -0.2, 0, mn, RE);
            bridge.peAddLockedParticle('neutron', 0, -0.3, -0.2, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }
        case 'pe-helion': {
            // Helion / He-3: locked 2p + n nucleus, 2 electrons orbiting
            const r = 4;
            const v = orbitalV(me, r, 2);
            bridge.peAddLockedParticle('proton', 1, 0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('proton', 1, -0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, 0, 0.3, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('electron', -1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }

        // ── Boson scenarios ───────────────────────────────────────
        case 'pe-w-pair': {
            // W⁺W⁻ pair in mutual Coulomb orbit
            const r = 2;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA * r / (4 * Math.PI * mW * (sep * sep + soft2)));
            bridge.peAddParticle('w_plus', 1, r, 0, 0, 0, v, 0, mW, RE);
            bridge.peAddParticle('w_minus', -1, -r, 0, 0, 0, -v, 0, mW, RE);
            break;
        }

        // ── Scattering scenarios (new) ────────────────────────────
        case 'pe-meson-scattering': {
            // π⁺ approaching locked proton (repulsive Coulomb)
            const v_app = 0.006;
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('pion_plus', 1, -15, 2, 0, v_app, 0, 0, mpi, RE);
            break;
        }
        case 'pe-muon-scattering': {
            // μ⁻ approaching locked proton (attractive Coulomb)
            const v_app = 0.008;
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('muon', -1, -15, 2, 0, v_app, 0, 0, mmu, RE);
            break;
        }

        case 'pe-custom':
        default:
            // Empty — user injects manually via Zoo or controls
            break;
    }
}

// ── Atom Engine Scenarios ────────────────────────────────────────────
function loadAEScenario(name) {
    if (!bridge.initAE) return;
    _resetAllVisualState();
    bridge.initAE();

    // Reset all AE toggles to defaults, then sync sliders from UI
    _resetAETogglesToDefaults();
    _syncAEParamsFromUI();
    // Scale 2 override: no auto-bonding for individual atoms
    if (bridge.aeSetBonding) bridge.aeSetBonding(false);
    const bondEl = document.getElementById('ae-bonding');
    if (bondEl) bondEl.checked = false;

    // Clear molecule info (molecules are Scale 3 only)
    if (inspector) inspector.setCurrentMolecule(null);

    // ── Procedural scenarios (periodic table, elements, custom) ──
    const S = 5;   // typical spacing (in Bohr radii)

    switch (name) {
        case 'ae-periodic': {
            // Full 118-element periodic table in standard 18-column layout
            // Rows 1-7: main table, Row 8: lanthanides, Row 9: actinides
            const gap = S * 1.2;
            const elements = allElements();
            for (const el of elements) {
                const pos = tablePosition(el.Z);
                if (!pos) continue;
                // Center the 18-column table, rows go downward (y decreasing)
                // Add extra gap before f-block rows (8,9) to visually separate them
                let rowY = pos.row;
                if (pos.row >= 8) rowY = pos.row + 0.5; // extra half-row gap
                const x = (pos.col - 9.5) * gap;
                const y = (1 - rowY) * gap;
                bridge.aeAddLockedAtom(el.Z, x, y, 0);
            }
            if (inspector) inspector.setScenarioInfo({
                title: 'Periodic Table',
                desc: 'All 118 elements in standard layout \u2014 atoms locked, no dynamics',
                fields: {
                    'Elements': '118',
                    'Layout': '18-column standard',
                    'State': 'All locked (static display)',
                }
            });
            // Zoom camera out to see the full table
            if (viewport) {
                const centerY = -gap * 4; // center of the 9-row table
                viewport.controls.target.set(0, centerY, 0);
                viewport.camera.position.set(0, centerY, 100);
                viewport.controls.update();
            }
            break;
        }
        // ══════════════════════════════════════════════════════════════
        // NOBLE GAS CLUSTERS — vdW only (no bonding, no ionic)
        // ══════════════════════════════════════════════════════════════
        case 'ae-he-cluster': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(false);   document.getElementById('ae-ionic').checked = false;
            const S = 5.5;
            const hex = [[0,0,0],[S,0,0],[S*0.5,S*0.866,0],
                         [0,0,S],[S,0,S],[S*0.5,S*0.866,S]];
            for (const [x, y, z] of hex)
                bridge.aeAddAtom(2, x - S*0.5, y - S*0.3, z - S*0.5,
                    (Math.random()-0.5)*0.2, (Math.random()-0.5)*0.2, (Math.random()-0.5)*0.2, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'Helium Cluster',
                desc: 'Six He atoms — van der Waals (LJ 12-6) only. Watch them settle.',
                fields: { 'Atoms': '6 × He', 'Force': 'vdW only', 'Bonding': 'None (noble gas)' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 35); viewport.controls.update(); }
            break;
        }
        case 'ae-ar-cluster': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(false);   document.getElementById('ae-ionic').checked = false;
            const S = 6.0;
            for (let ix = 0; ix < 2; ix++) for (let iy = 0; iy < 2; iy++) for (let iz = 0; iz < 2; iz++)
                bridge.aeAddAtom(18, (ix-0.5)*S, (iy-0.5)*S, (iz-0.5)*S,
                    (Math.random()-0.5)*0.15, (Math.random()-0.5)*0.15, (Math.random()-0.5)*0.15, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'Argon Cluster',
                desc: 'Eight Ar atoms in a cube — vdW condensation dynamics.',
                fields: { 'Atoms': '8 × Ar', 'Force': 'vdW only', 'Layout': '2×2×2 cube' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 35); viewport.controls.update(); }
            break;
        }
        case 'ae-noble-mix': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(false);   document.getElementById('ae-ionic').checked = false;
            bridge.aeAddAtom(2, -12, 0, 0, 0.1, 0, 0, 0);
            bridge.aeAddAtom(2, -8, 0, 0, -0.1, 0, 0, 0);
            bridge.aeAddAtom(10, -2, 0, 0, 0.1, 0, 0, 0);
            bridge.aeAddAtom(10, 2, 0, 0, -0.1, 0, 0, 0);
            bridge.aeAddAtom(18, 7, 0, 0, 0.1, 0, 0, 0);
            bridge.aeAddAtom(18, 12, 0, 0, -0.1, 0, 0, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'Noble Gas Mix',
                desc: 'He + Ne + Ar — different sizes interact via vdW only.',
                fields: { 'Atoms': '2 He + 2 Ne + 2 Ar', 'Force': 'vdW only' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 45); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // IONIC FORMATION — Coulomb-driven, no covalent bonding
        // ══════════════════════════════════════════════════════════════
        case 'ae-nacl-form': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeAddAtom(11, -12, 0, 0, 0.15, 0, 0, 1);   // Na+
            bridge.aeAddAtom(17, 12, 0, 0, -0.15, 0, 0, -1);  // Cl-
            if (inspector) inspector.setScenarioInfo({ title: 'NaCl Formation',
                desc: 'Na⁺ and Cl⁻ attract via Coulomb force — ionic bond formation.',
                fields: { 'Atoms': 'Na⁺ + Cl⁻', 'Force': 'Ionic (Coulomb)', 'Bonding': 'None (ionic)' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 40); viewport.controls.update(); }
            break;
        }
        case 'ae-nacl-lattice': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetBondsForce(false); document.getElementById('ae-bonds-force').checked = false;
            const sp = 7.5;
            for (let ix = 0; ix < 3; ix++) for (let iy = 0; iy < 3; iy++) {
                const charge = ((ix + iy) % 2 === 0) ? 1 : -1;
                const Z = charge === 1 ? 11 : 17;
                bridge.aeAddAtom(Z, (ix-1)*sp, (iy-1)*sp, 0, 0, 0, 0, charge);
            }
            if (inspector) inspector.setScenarioInfo({ title: 'NaCl 3×3 Lattice',
                desc: 'Ionic crystal lattice — alternating Na⁺/Cl⁻ held by Coulomb.',
                fields: { 'Atoms': '9 (Na⁺/Cl⁻ alternating)', 'Layout': '3×3 grid', 'Force': 'Ionic + vdW' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 45); viewport.controls.update(); }
            break;
        }
        case 'ae-mgf2': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeAddAtom(12, 0, 0, 0, 0, 0, 0, 2);     // Mg2+
            bridge.aeAddAtom(9, -15, 0, 0, 0.2, 0, 0, -1);  // F-
            bridge.aeAddAtom(9, 15, 0, 0, -0.2, 0, 0, -1);  // F-
            if (inspector) inspector.setScenarioInfo({ title: 'MgF₂ Formation',
                desc: 'Mg²⁺ attracts two F⁻ ions — ionic bond formation.',
                fields: { 'Atoms': 'Mg²⁺ + 2 F⁻', 'Force': 'Ionic (Coulomb)' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 45); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // COVALENT FORMATION — watch bonds form via auto-bonding
        // ══════════════════════════════════════════════════════════════
        case 'ae-h2-form': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            bridge.aeAddAtom(1, -7, 0, 0, 0.08, 0, 0, 0);
            bridge.aeAddAtom(1, 7, 0, 0, -0.08, 0, 0, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'H₂ Formation',
                desc: 'Two hydrogen atoms approach — vdW attracts, bond forms at r < 4.8.',
                fields: { 'Atoms': '2 × H', 'Force': 'vdW + auto-bond', 'Threshold': '1.2 × σ_avg ≈ 4.8' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 25); viewport.controls.update(); }
            break;
        }
        case 'ae-o2-form': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            bridge.aeAddAtom(8, -5, 0, 0, 0.06, 0, 0, 0);
            bridge.aeAddAtom(8, 5, 0, 0, -0.06, 0, 0, 0);
            _aeSetPhase3({ angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'O₂ Formation',
                desc: 'Two oxygen atoms approach and bond — double bond forms.',
                fields: { 'Atoms': '2 × O', 'Force': 'vdW + auto-bond + angle strain' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 25); viewport.controls.update(); }
            break;
        }
        case 'ae-ch4-form': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            _aeSetPhase3({ angle: true });
            const d = 9, t = 1 / Math.sqrt(3);
            bridge.aeAddAtom(6, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, d*t, d*t, d*t, -0.05, -0.05, -0.05, 0);
            bridge.aeAddAtom(1, d*t, -d*t, -d*t, -0.05, 0.05, 0.05, 0);
            bridge.aeAddAtom(1, -d*t, d*t, -d*t, 0.05, -0.05, 0.05, 0);
            bridge.aeAddAtom(1, -d*t, -d*t, d*t, 0.05, 0.05, -0.05, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'CH₄ Assembly',
                desc: 'Carbon + 4 hydrogens approach — bonds form, angle strain drives tetrahedral.',
                fields: { 'Atoms': 'C + 4H', 'Target': '109.47° tetrahedral', 'Force': 'vdW + bond + angle' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 30); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // H-BONDING — pre-formed water molecules with hydrogen bonds
        // ══════════════════════════════════════════════════════════════
        case 'ae-water-dimer': {
            const ang = 104.5 * Math.PI / 180;
            const rOH = 3.4;
            // Molecule 1 (left)
            bridge.aeAddAtom(8, -7, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, -7 + rOH, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, -7 + rOH*Math.cos(ang), rOH*Math.sin(ang), 0, 0, 0, 0, 0);
            // Molecule 2 (right, rotated so O faces mol1's H)
            bridge.aeAddAtom(8, 7, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 7 - rOH, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 7 - rOH*Math.cos(ang), -rOH*Math.sin(ang), 0, 0, 0, 0, 0);
            // Pre-bond to establish O-H covalent bonds
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            _aeSetPhase3({ hbonds: true, angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'Water Dimer',
                desc: 'Two H₂O molecules — H-bond attracts them. First Phase 3 demo!',
                fields: { 'Atoms': '6 (2 × H₂O)', 'Force': 'Bond + H-bond + angle strain', 'H-bond': 'LJ 10-12 + angular' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 35); viewport.controls.update(); }
            break;
        }
        case 'ae-water-cluster': {
            const ang = 104.5 * Math.PI / 180;
            const rOH = 3.4;
            const N_mol = 5, R_ring = 16;
            for (let m = 0; m < N_mol; m++) {
                const theta = (2 * Math.PI * m) / N_mol;
                const ox = R_ring * Math.cos(theta), oy = R_ring * Math.sin(theta);
                bridge.aeAddAtom(8, ox, oy, 0, 0, 0, 0, 0);
                // H1 pointing toward next molecule (H-bond donor)
                const tn = (2 * Math.PI * (m + 1)) / N_mol;
                const dnx = Math.cos(tn) - Math.cos(theta), dny = Math.sin(tn) - Math.sin(theta);
                const dn = Math.sqrt(dnx*dnx + dny*dny);
                bridge.aeAddAtom(1, ox + rOH*dnx/dn, oy + rOH*dny/dn, 0, 0, 0, 0, 0);
                // H2 at HOH angle
                const px = -dny/dn, py = dnx/dn;
                const h2x = Math.cos(ang)*dnx/dn + Math.sin(ang)*px;
                const h2y = Math.cos(ang)*dny/dn + Math.sin(ang)*py;
                bridge.aeAddAtom(1, ox + rOH*h2x, oy + rOH*h2y, 0, 0, 0, 0, 0);
            }
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            _aeSetPhase3({ hbonds: true, angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'Water Pentamer',
                desc: 'Five H₂O molecules in a ring — H-bond network demonstration.',
                fields: { 'Atoms': '15 (5 × H₂O)', 'Force': 'Bond + H-bond + angle', 'Pattern': 'Cyclic H-bond ring' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 55); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // VSEPR GEOMETRY — start at wrong angle, watch relaxation
        // ══════════════════════════════════════════════════════════════
        case 'ae-vsepr-linear': {
            // CO₂: start bent at 90°, should relax to 180° (linear)
            // C-O sigma_avg ≈ 2.10, threshold ≈ 2.52; use r=2.0 for safe margin
            bridge.aeAddAtom(6, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(8, 2.0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(8, 0, 2.0, 0, 0, 0, 0, 0);  // 90° to start
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            _aeSetPhase3({ angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'CO₂ VSEPR',
                desc: 'CO₂ starts bent (90°) — angle strain drives it to linear (180°).',
                fields: { 'Atoms': 'C + 2O', 'Start': '90°', 'Target': '180° (linear)', 'Steric #': '2' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 20); viewport.controls.update(); }
            break;
        }
        case 'ae-vsepr-tetrahedral': {
            // CH₄: start at 90° (cubic), should relax to 109.47°
            const d = 3.5;
            bridge.aeAddAtom(6, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, d, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, -d, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 0, d, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 0, 0, d, 0, 0, 0, 0);
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            _aeSetPhase3({ angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'CH₄ VSEPR',
                desc: 'CH₄ starts at 90° — angle strain relaxes to 109.47° tetrahedral.',
                fields: { 'Atoms': 'C + 4H', 'Start': '90°', 'Target': '109.47°', 'Steric #': '4' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 20); viewport.controls.update(); }
            break;
        }
        case 'ae-vsepr-bent': {
            // H₂O: start at 150° (too wide), should relax to 104.5°
            const r = 3.4;
            const theta0 = 150 * Math.PI / 180;
            bridge.aeAddAtom(8, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, r, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, r*Math.cos(theta0), r*Math.sin(theta0), 0, 0, 0, 0, 0);
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            _aeSetPhase3({ angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'H₂O VSEPR',
                desc: 'H₂O starts at 150° — lone pairs drive H-O-H toward 104.5° bent.',
                fields: { 'Atoms': 'O + 2H', 'Start': '150°', 'Target': '104.5°', 'Lone pairs': '2' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 20); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // THERMAL DYNAMICS — thermostat + gas kinetics
        // ══════════════════════════════════════════════════════════════
        case 'ae-thermal-gas': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(false);   document.getElementById('ae-ionic').checked = false;
            _aeSetPhase3({ thermostat: true, temp: 1.0 });
            const L = 15;
            for (let n = 0; n < 12; n++) {
                const x = (Math.random()-0.5)*2*L, y = (Math.random()-0.5)*2*L, z = (Math.random()-0.5)*2*L;
                const speed = 0.3 + Math.random()*0.5;
                const phi = Math.random()*2*Math.PI, th = Math.acos(2*Math.random()-1);
                bridge.aeAddAtom(18, x, y, z,
                    speed*Math.sin(th)*Math.cos(phi), speed*Math.sin(th)*Math.sin(phi), speed*Math.cos(th), 0);
            }
            if (inspector) inspector.setScenarioInfo({ title: 'Thermal Gas',
                desc: '12 Ar atoms with Berendsen thermostat — temperature stabilizes at T=1.',
                fields: { 'Atoms': '12 × Ar', 'Force': 'vdW only', 'Thermostat': 'ON (T=1.0)' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 55); viewport.controls.update(); }
            break;
        }
        case 'ae-collision': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeAddAtom(18, -20, 0, 0, 0.4, 0, 0, 0);
            bridge.aeAddAtom(18, 20, 0, 0, -0.4, 0, 0, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'Head-On Collision',
                desc: 'Two Ar atoms approach at speed — LJ repulsion at short range.',
                fields: { 'Atoms': '2 × Ar', 'Force': 'vdW (LJ 12-6)', 'Speed': '0.4 each' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 50); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // METALLIC CLUSTERS — multi-atom bonding
        // ══════════════════════════════════════════════════════════════
        case 'ae-fe-bcc': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            // Fe sigma ≈ 1.35, bond threshold ≈ 1.62; BCC center-corner = a*√3/2
            // Need a*√3/2 < 1.62 → a < 1.87. Use a = 0.9 so center-corner ≈ 1.56
            const a = 0.9;
            // BCC: 8 corners + 1 center
            for (let ix = -1; ix <= 1; ix += 2)
                for (let iy = -1; iy <= 1; iy += 2)
                    for (let iz = -1; iz <= 1; iz += 2)
                        bridge.aeAddAtom(26, ix*a, iy*a, iz*a, 0, 0, 0, 0);
            bridge.aeAddAtom(26, 0, 0, 0, 0, 0, 0, 0);
            bridge.aePreBond();
            if (inspector) inspector.setScenarioInfo({ title: 'Fe BCC Cluster',
                desc: 'Iron atoms in body-centered cubic arrangement — metallic bonding.',
                fields: { 'Atoms': '9 × Fe', 'Layout': 'BCC (8 corners + center)', 'Force': 'vdW + bond' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 15); viewport.controls.update(); }
            break;
        }
        case 'ae-cu-fcc': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            // Cu sigma ≈ 1.30, bond threshold ≈ 1.56; use a = 1.5 for nearest-neighbor
            const a = 1.5;
            bridge.aeAddAtom(29, 0, 0, 0, 0, 0, 0, 0);    // center
            bridge.aeAddAtom(29, a, 0, 0, 0, 0, 0, 0);     // +x
            bridge.aeAddAtom(29, -a, 0, 0, 0, 0, 0, 0);    // -x
            bridge.aeAddAtom(29, 0, a, 0, 0, 0, 0, 0);     // +y
            bridge.aeAddAtom(29, 0, -a, 0, 0, 0, 0, 0);    // -y
            bridge.aeAddAtom(29, 0, 0, a, 0, 0, 0, 0);     // +z
            bridge.aeAddAtom(29, 0, 0, -a, 0, 0, 0, 0);    // -z
            bridge.aePreBond();
            if (inspector) inspector.setScenarioInfo({ title: 'Cu FCC Seed',
                desc: 'Copper atoms in face-centered cubic seed — nearest-neighbor bonding.',
                fields: { 'Atoms': '7 × Cu', 'Layout': 'FCC (center + 6 face)', 'Force': 'vdW + bond' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 15); viewport.controls.update(); }
            break;
        }

        case 'ae-custom':
            if (inspector) inspector.setScenarioInfo(null);
            break;

        default: {
            // Handle individual element scenarios: ae-el-1 through ae-el-118
            const isElement = name.startsWith('ae-el-');
            if (isElement) {
                const Z = parseInt(name.slice(6));
                bridge.aeAddLockedAtom(Z, 0, 0, 0);
                const el = getElement(Z);
                if (inspector && el) {
                    const N = el.neutrons || 0;
                    const mass = (Z + N * 1.001).toFixed(2);
                    const period = el.row <= 7 ? el.row : (el.row === 8 ? '6 (Ln)' : '7 (An)');
                    inspector.setScenarioInfo({
                        title: el.name,
                        desc: `Isolated ${el.name} atom (Z = ${Z})`,
                        fields: {
                            'Symbol': el.symbol,
                            'Z': Z,
                            'Period': period,
                            'Group': el.col,
                            'Mass': mass + ' AMU',
                            'Max Bonds': el.maxBonds,
                        }
                    });
                }
                // Camera distance scaled to atom size (heavier → more shells → zoom out)
                if (viewport) {
                    const dist = Z > 54 ? 50 : Z > 36 ? 40 : Z > 18 ? 30 : 20;
                    viewport.controls.target.set(0, 0, 0);
                    viewport.camera.position.set(0, 0, dist);
                    viewport.controls.update();
                }
            }
            break;
        }
    }

    // Capture initial energy reference for drift tracking (before first tick)
    const initDiag = bridge.aeGetDiagnostics();
    if (initDiag.totalEnergy !== 0) _aeInitialEnergy = initDiag.totalEnergy;
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

// Sync AE params from UI sliders (shared helper for Scale 2/3 loaders)
// Default AE toggle states: [elId, defaultChecked, setterName]
const AE_DEFAULT_TOGGLES = [
    ['ae-ionic', true, 'aeSetIonic'],
    ['ae-vdw', true, 'aeSetVdw'],
    ['ae-bonds-force', true, 'aeSetBondsForce'],
    ['ae-bonding', true, 'aeSetBonding'],
    ['ae-damping', false, 'aeSetDamping'],
    ['ae-speed-limit', true, 'aeSetSpeedLimit'],
    // Phase 3 (all off by default — scenarios enable as needed)
    ['ae-hbonds', false, 'aeSetHBonds'],
    ['ae-angle', false, 'aeSetAngleStrain'],
    ['ae-dipole', false, 'aeSetDipoleDipole'],
    ['ae-thermostat', false, 'aeSetThermostat'],
    ['ae-electronegativity', false, 'aeSetElectronegativity'],
];

function _syncAEParamsFromUI() {
    const dtEl = document.getElementById('ae-dt-slider');
    if (dtEl) bridge.aeSetDt(parseFloat(dtEl.value));
    const softEl = document.getElementById('ae-soft-slider');
    if (softEl) bridge.aeSetSoftening(parseFloat(softEl.value));
    // Sync all AE toggles from checkboxes
    for (const [elId, , setter] of AE_DEFAULT_TOGGLES) {
        const el = document.getElementById(elId);
        if (el && bridge[setter]) bridge[setter](el.checked);
    }
}

function _resetAETogglesToDefaults() {
    for (const [elId, defaultVal, setter] of AE_DEFAULT_TOGGLES) {
        const el = document.getElementById(elId);
        if (el) el.checked = defaultVal;
        if (bridge[setter]) bridge[setter](defaultVal);
    }
}

// Helper: enable Phase 3 forces for specific scenarios and sync UI checkboxes
function _aeSetPhase3(flags) {
    const map = {
        hbonds: ['ae-hbonds', 'aeSetHBonds'],
        angle: ['ae-angle', 'aeSetAngleStrain'],
        dipole: ['ae-dipole', 'aeSetDipoleDipole'],
        thermostat: ['ae-thermostat', 'aeSetThermostat'],
        elec: ['ae-electronegativity', 'aeSetElectronegativity'],
    };
    for (const [key, [elId, setter]] of Object.entries(map)) {
        if (flags[key] !== undefined && bridge[setter]) {
            bridge[setter](flags[key]);
            const el = document.getElementById(elId);
            if (el) el.checked = flags[key];
        }
    }
    if (flags.temp !== undefined && bridge.aeSetThermostatTemp) bridge.aeSetThermostatTemp(flags.temp);
}

// ── Scale 3: Molecule Scenario Loader ────────────────────────────────
// Uses same AtomEngine as Scale 2 but only molecule scenarios.
function loadMoleculeScenario(name) {
    if (!bridge.initAE) return;
    _resetAllVisualState();
    bridge.initAE();

    // Reset toggles to defaults (bonding ON for molecules) then sync sliders
    _resetAETogglesToDefaults();
    _syncAEParamsFromUI();

    // Data-driven molecular library
    const molId = name.startsWith('mol-') ? name.slice(4) : null;
    if (molId && loadMolecule(bridge, molId)) {
        // Pre-bond: establish covalent bonds BEFORE the first tick.
        // Without this, atoms placed at covalent bond distances (inside the
        // LJ wall) experience explosive repulsive forces on the first tick.
        if (bridge.aePreBond) bridge.aePreBond();

        // Stability check: one-tick dry run
        const preData = bridge.aeGetAtomData();
        bridge.aeTick();
        const postData = bridge.aeGetAtomData();
        let maxDisp = 0;
        for (let i = 0; i < postData.count; i++) {
            const dx = postData.positions[i*3] - preData.positions[i*3];
            const dy = postData.positions[i*3+1] - preData.positions[i*3+1];
            const dz = postData.positions[i*3+2] - preData.positions[i*3+2];
            maxDisp = Math.max(maxDisp, Math.sqrt(dx*dx + dy*dy + dz*dz));
        }
        if (maxDisp > 1) console.warn(`[FTD] ${molId}: UNSTABLE — max displacement ${maxDisp.toFixed(4)}`);

        // Reset to initial state (the dry-run consumed one tick)
        bridge.initAE();
        _syncAEParamsFromUI();
        loadMolecule(bridge, molId);
        if (bridge.aePreBond) bridge.aePreBond();

        if (inspector) { inspector.setScenarioInfo(null); inspector.setCurrentMolecule(molId); }
        const mol = getMolecule(molId);
        if (mol?.cameraDistance && viewport) {
            viewport.controls.target.set(0, 0, 0);
            viewport.camera.position.set(0, 0, mol.cameraDistance);
            viewport.controls.update();
        }
        // Capture initial energy reference for drift tracking
        const initDiag = bridge.aeGetDiagnostics();
        if (initDiag.totalEnergy !== 0) _aeInitialEnergy = initDiag.totalEnergy;
        return;
    }

    // Fallback: crystal and custom
    if (name === 'mol-crystal') {
        // NaCl 3x3x3 crystal lattice
        const sp = 7.5;
        for (let ix = 0; ix < 3; ix++) {
            for (let iy = 0; iy < 3; iy++) {
                for (let iz = 0; iz < 3; iz++) {
                    const x = (ix - 1) * sp;
                    const y = (iy - 1) * sp;
                    const z = (iz - 1) * sp;
                    if ((ix + iy + iz) % 2 === 0) {
                        bridge.aeAddAtom(11, x, y, z, 0, 0, 0, 1);
                    } else {
                        bridge.aeAddAtom(17, x, y, z, 0, 0, 0, -1);
                    }
                }
            }
        }
        if (bridge.aePreBond) bridge.aePreBond();
        if (inspector) inspector.setScenarioInfo({
            title: 'NaCl Ionic Crystal',
            desc: '3\u00d73\u00d73 rock salt lattice \u2014 alternating Na\u207a and Cl\u207b ions',
            fields: { 'Structure': 'FCC (rock salt)', 'Bonding': 'Ionic' }
        });
    }
    // mol-custom: empty, user builds manually

    // Capture initial energy reference for drift tracking
    const initDiag = bridge.aeGetDiagnostics();
    if (initDiag.totalEnergy !== 0) _aeInitialEnergy = initDiag.totalEnergy;
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
            opt.innerHTML = `${mol.formula} ${mol.name}`;
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

    // Scenario-specific toggle overrides
    if (name === 'flux-dual-substrate') {
        bridge.setToggle('dual_substrate', true);
        const el = document.getElementById('t-dual');
        if (el) el.checked = true;
    }
    if (name === 'flux-cosmic-web' || name === 'flux-gravitational-wave' ||
        name === 'flux-triad' || name === 'flux-baryon') {
        bridge.setToggle('gravity', true);
        const el = document.getElementById('t-gravity');
        if (el) el.checked = true;
    }
    if (name === 'flux-cyclotron') {
        bridge.setToggle('lorentz_force', true);
        const el = document.getElementById('t-lorentz');
        if (el) el.checked = true;
    }
    // QCD scenarios: enable confinement
    if (name === 'flux-meson' || name === 'flux-baryon') {
        bridge.setToggle('confinement', true);
        bridge.setToggle('genesis', false);
        const cEl = document.getElementById('t-confinement');
        if (cEl) cEl.checked = true;
        const genEl = document.getElementById('t-genesis');
        if (genEl) genEl.checked = false;
    }
    if (name === 'flux-string-breaking') {
        bridge.setToggle('confinement', true);
        bridge.setToggle('genesis', true);
        const cEl = document.getElementById('t-confinement');
        if (cEl) cEl.checked = true;
        const genEl = document.getElementById('t-genesis');
        if (genEl) genEl.checked = true;
    }
    if (name === 'flux-dark-matter') {
        bridge.setToggle('gravity', true);
        bridge.setToggle('genesis', false);
        const gEl = document.getElementById('t-gravity');
        if (gEl) gEl.checked = true;
        const genEl = document.getElementById('t-genesis');
        if (genEl) genEl.checked = false;
    }
    if (name === 'flux-baryogenesis') {
        bridge.setToggle('genesis', true);
        bridge.setToggle('gravity', true);
        const genEl = document.getElementById('t-genesis');
        if (genEl) genEl.checked = true;
        const gEl = document.getElementById('t-gravity');
        if (gEl) gEl.checked = true;
    }
    if (name === 'flux-vacuum-foam') {
        bridge.setToggle('genesis', true);
        bridge.setToggle('damping', true);
        const genEl = document.getElementById('t-genesis');
        if (genEl) genEl.checked = true;
        const dEl = document.getElementById('t-damping');
        if (dEl) dEl.checked = true;
    }

    // Light scenarios: wave propagation + selective damping only (pure EM)
    if (name.startsWith('light-')) {
        const lightOverrides = [
            ['selective_damping', true, 't-selective'],
            ['coupling',   false, 't-coupling'],
            ['damping',    false, 't-damping'],
            ['genesis',    false, 't-genesis'],
            ['gauss_projection', false, 't-gauss'],
            ['forces',     false, 't-forces'],
            ['movement',   false, 't-movement'],
            ['poisson_coulomb', false, 't-poisson'],
        ];
        for (const [key, val, elId] of lightOverrides) {
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

    _latticeNeedsUpload = true;
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
        { id: 'combo-kb',   valId: 'combo-kb-val',   param: 'kb',      fmt: 3 },
        { id: 'combo-gn',   valId: 'combo-gn-val',   param: 'gn',      fmt: 3 },
        { id: 'combo-damp', valId: 'combo-damp-val',  param: 'damping', fmt: 3 },
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
