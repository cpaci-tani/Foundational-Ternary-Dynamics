/**
 * Scale 1 (Particles) Controller
 *
 * Extracted from app.js to isolate the Particle Engine (PE) frame loop,
 * scenario loader, cloud rendering, trail history, and field overlay logic.
 *
 * Owns all PE-specific state internally:
 *   - Field toggle flags (E-field, potential, gravity, forces)
 *   - Velocity/trail display flags
 *   - Black hole scenario state (Hawking tick counter)
 *   - Field grid cache and source particle buffer
 *
 * Delegates to sibling modules:
 *   ./pe-cloud-expander.js — cloud buffers, templates, trail history
 *   ./scenarios.js         — pe-* scenario setup (big switch)
 *
 * Exports:
 *   animatePE(ctx)             — per-frame update
 *   loadPEScenario(ctx, name)  — scenario setup
 *   resetScale1(ctx)           — clear PE-specific state for mode switch
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { getById } from '../../particle-catalog.js';
import { formatEnergy } from '../../units.js';
import {
    generateGridXZ, samplePECoulombOnly,
    samplePEGravityField, makePECoulombFieldFn
} from '../../fields.js';
import {
    computeStreamlines, generateEFieldSeeds
} from '../../fieldlines.js';
import {
    ALPHA, C_SPEED, G_N, K_B,
    M_P_PHYS, M_MU_PHYS, M_N_PHYS, M_PI_CH_PHYS, M_K_CH_PHYS,
    M_TAU_PHYS, M_W_PHYS, M_SIGMA_PHYS, M_OMEGA_PHYS, M_DELTA_PHYS
} from '../../constants.js';
import { createTickAccumulator, formatSI } from '../scale-utils.js';
import { Scale1ControlsComponent } from './ui/controls/component.js';
import {
    expandPEToCloud, updateTrailHistory,
    getCloudParticleMap, getTrailHistory, clearCloudAndTrails
} from './pe-cloud-expander.js';
import { setupPEScenario } from './scenarios.js';
import { telemetryHub } from '../../telemetry-hub.js';


// =====================================================================
// PE-Specific Module State
// =====================================================================

// -- Field overlay toggle flags ---------------------------------------
let _showPEEField    = false;   // E-field streamlines
let _showPEPotential = false;   // Coulomb potential heatmap
let _showPEGravField = false;   // Gravity field vectors
let _showPEForces    = false;   // Per-particle net force arrows
let _showVelocities  = false;   // Velocity vectors overlay
let _showTrails      = false;   // Orbit trail lines

// -- Field computation cache ------------------------------------------
let _fieldGrid       = null;    // cached grid from generateGridXZ
const _srcParticlesBuf = [];    // reusable {x,y,z} array for field seed generation

// -- Black hole scenario state ----------------------------------------
let _bhActive       = false;
let _bhHawkingTick  = 0;
const _BH_HAWKING_INTERVAL = 300;
const _BH_HORIZON_R = 3.0;
const _BH_MASS      = 5000;
const _BH_TEST_MASS = K_B;   // electron mass (MeV) — Hawking test particle

// -- Tick accumulator (sub-1 speed fractional ticks, shared helper) ----
const _tickAcc = createTickAccumulator();

// -- Paused-state dedup (avoid redundant work when simulation idle) ----
let _statusCache = { tick: '', ptime: '', particles: '', energy: '', state: '' };
let _diagPushedWhilePaused = false;
let _lastCloudData = null;         // cached cloud output when paused


// =====================================================================
// Exported: Field Toggle Setters + cloud/trail accessors
// =====================================================================

export function setPEEField(on)    { _showPEEField    = on; }
export function setPEPotential(on) { _showPEPotential = on; }
export function setPEGravField(on) { _showPEGravField = on; }
export function setPEForces(on)    { _showPEForces    = on; }
export function setVelocities(on)  { _showVelocities  = on; }
export function setTrails(on)      { _showTrails      = on; }

// Re-export cloud/trail accessors so external consumers have a single
// import surface (controller.js) and need not know about the split file.
export { getCloudParticleMap, getTrailHistory };


// =====================================================================
// Exported: resetScale1(ctx)
// =====================================================================

/**
 * Reset PE-internal state for a clean mode switch.
 */
class Scale1LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
    }

    mount(ctx) {
        // Standard setup placeholder
    }

    destroy(ctx) {
        super.destroy(ctx);
        _resetScale1Internal(ctx);
    }
}

const _lifecycleController = new Scale1LifecycleController();

export function mount(ctx) {
    _lifecycleController.mount(ctx);
}

export function destroy(ctx) {
    _lifecycleController.destroy(ctx);
}

export function resetScale1(ctx) {
    _lifecycleController.destroy(ctx);
}

function _resetScale1Internal(ctx) {
    const { viewport } = ctx;

    // Clear cloud templates and trail history (owned by pe-cloud-expander)
    clearCloudAndTrails();

    // Reset field overlays
    _showPEEField    = false;
    _showPEPotential = false;
    _showPEGravField = false;
    _showPEForces    = false;
    _showVelocities  = false;
    _showTrails      = false;

    // Reset field computation cache
    _fieldGrid = null;
    _srcParticlesBuf.length = 0;

    // Reset black hole state
    _bhActive      = false;
    _bhHawkingTick = 0;

    // Reset tick accumulator
    _tickAcc.reset();

    // Clear paused-state caches
    _statusCache = { tick: '', ptime: '', particles: '', energy: '', state: '' };
    _diagPushedWhilePaused = false;
    _lastCloudData = null;

    // Clear viewport overlays if available
    if (viewport) {
        viewport.togglePEStreamlines(false);
        viewport.toggleFieldHeatmap(false);
        viewport.toggleFieldVectors(false);
        viewport.toggleGravityVectors(false);
        viewport.toggleParticleForces(false);
        viewport.toggleVelocityVectors(false);
        viewport.toggleTrails(false);
    }
}


// =====================================================================
// Exported: animatePE(ctx)
// =====================================================================

export function animatePE(ctx) {
    const {
        bridge, viewport, running, ticksPerFrame, inspector,
        fluxEnergyChart, particleChart, peTelemetry,
        activeTab, frameCount, dom, now,
        updateOnticPanel
    } = ctx;

    // ── 1. Tick PE simulation while running ──────────────────────────
    if (running) {
        const wholeTicks = _tickAcc.accumulate(ticksPerFrame);
        for (let i = 0; i < wholeTicks; i++) {
            bridge.peTick();
        }

        // ── 2. Hawking-analogue pair emission for micro black hole ──
        if (_bhActive) {
            _bhHawkingTick += wholeTicks;
            if (_bhHawkingTick >= _BH_HAWKING_INTERVAL) {
                _bhHawkingTick = 0;
                const phi = Math.random() * 2 * Math.PI;
                const r_emit = _BH_HORIZON_R + 0.5;
                const px = r_emit * Math.cos(phi);
                const pz = r_emit * Math.sin(phi);
                const v_out = C_SPEED * 0.60;
                // Escaping particle (red, e-) -- radially outward
                bridge.peAddParticle('electron', -1, px, 0, pz,
                    v_out * Math.cos(phi), 0, v_out * Math.sin(phi),
                    _BH_TEST_MASS, 0.1);
                // In-falling partner (green, e+) -- antipodal, slow
                bridge.peAddParticle('positron', 1, -px, 0, -pz,
                    v_out * 0.3 * Math.cos(phi), 0, v_out * 0.3 * Math.sin(phi),
                    _BH_TEST_MASS, 0.1);
            }
        }
    }

    // ── 3. Cloud expansion: particle centers -> flux cloud points ───
    const peData  = bridge.peGetParticleData();
    const typeMap = bridge.peGetParticleTypes();
    const t       = now * 0.001; // seconds for smooth animation
    let cloud;
    if (!running && _lastCloudData && _lastCloudData.count > 0) {
        cloud = _lastCloudData;  // reuse cached cloud — positions haven't changed
    } else {
        cloud = expandPEToCloud(peData, typeMap, t);
        _lastCloudData = cloud;
    }
    viewport.updateParticles(cloud);

    // Update inspector with cloud-to-particle mapping
    if (inspector) {
        inspector.setPEContext(getCloudParticleMap(), cloud.count, typeMap);
    }

    // ── 4. Velocity vectors overlay ─────────────────────────────────
    if (_showVelocities && peData.count > 0) {
        viewport.updateVelocityVectors(peData.positions, peData.velocities, peData.count);
    }

    // ── 4b. Orbit trails ────────────────────────────────────────────
    if (running && peData.count > 0) {
        updateTrailHistory(peData);
    }
    if (_showTrails) {
        viewport.updateTrails(getTrailHistory(), typeMap);
    }

    // ── 5. PE Field Overlays (individual force decomposition) ───────
    if (_showPEPotential && running && peData.count > 0) {
        if (!_fieldGrid) _fieldGrid = generateGridXZ(25, 20);
        const src   = bridge.peGetFieldSources();
        const field = samplePECoulombOnly(src, _fieldGrid.positions, _fieldGrid.count);
        viewport.updateFieldHeatmap(
            _fieldGrid.positions, field.potentials, _fieldGrid.count, field.maxPotential);
        viewport.updateFieldVectors(
            _fieldGrid.positions, field.forces, _fieldGrid.count, field.maxForce, 8.0);
    }

    // Coulomb E-field streamlines (3D, throttled every 5 frames)
    if (_showPEEField && running && peData.count > 0 && frameCount % 5 === 0) {
        const src     = bridge.peGetFieldSources();
        const fieldFn = makePECoulombFieldFn(src, 0.5);
        while (_srcParticlesBuf.length < src.count) _srcParticlesBuf.push({ x: 0, y: 0, z: 0 });
        _srcParticlesBuf.length = src.count;
        for (let i = 0; i < src.count; i++) {
            _srcParticlesBuf[i].x = src.positions[i * 3];
            _srcParticlesBuf[i].y = src.positions[i * 3 + 1];
            _srcParticlesBuf[i].z = src.positions[i * 3 + 2];
        }
        const seeds = generateEFieldSeeds(_srcParticlesBuf, 3, 100);
        const lines = computeStreamlines({ fieldFn }, seeds, {
            maxSteps: 80, stepSize: 0.5, bounds: 30
        });
        viewport.updatePEStreamlines(lines);
    }

    // Gravity field vectors (XZ plane)
    if (_showPEGravField && running && peData.count > 0) {
        if (!_fieldGrid) _fieldGrid = generateGridXZ(25, 20);
        const src   = bridge.peGetFieldSources();
        const field = samplePEGravityField(src, _fieldGrid.positions, _fieldGrid.count);
        viewport.updateGravityVectors(
            _fieldGrid.positions, field.forces, _fieldGrid.count, field.maxForce);
    }

    // Per-particle net force arrows
    if (_showPEForces && running && peData.count > 0) {
        const fd = bridge.peGetForces();
        viewport.updateParticleForces(fd.positions, fd.forces, fd.count, fd.maxForce);
    }

    // ── 6. Render ───────────────────────────────────────────────────
    viewport.render();

    // ── 7. PE diagnostics (throttled to every 3rd frame) ────────────
    if (frameCount % 3 === 0 && (running || !_diagPushedWhilePaused)) {
        const diag = telemetryHub.collectScale1(bridge);
        const ext = telemetryHub.collectScale1Extended(bridge);

        if (diag) {
            const sTick = formatSI(diag.tick);
            const sParticles = String(diag.particleCount);
            const sEnergy = formatEnergy(diag.totalEnergy, 1).text;
            const sState = running ? 'Running' : 'Idle';

            if (_statusCache.tick !== sTick) { dom.statusPtime.textContent = sTick; _statusCache.tick = sTick; }
            if (_statusCache.particles !== sParticles) { dom.statusParticles.textContent = sParticles; _statusCache.particles = sParticles; }
            if (_statusCache.energy !== sEnergy) { dom.statusEnergy.textContent = sEnergy; _statusCache.energy = sEnergy; }
            if (_statusCache.state !== sState) {
                dom.statusState.textContent = sState;
                _statusCache.state = sState;
                if (running) dom.statusDot.classList.remove('idle');
                else dom.statusDot.classList.add('idle');
            }

            if (peTelemetry) peTelemetry.update(diag, ext);

            const diagAdapted = {
                tick:          diag.tick,
                manifested:    diag.particleCount,
                positive: 0,  negative: 0,
                totalFlux: 0, totalEnergy: diag.totalEnergy,
                fieldEnergy:   diag.totalPE,
                kineticEnergy: diag.totalKE,
                peFlux:        diag.totalPE,
            };
            fluxEnergyChart.push(diagAdapted);
            particleChart.push(diagAdapted);
        }

        if (!running) _diagPushedWhilePaused = true;
        else _diagPushedWhilePaused = false;

        switch (activeTab) {
            case 'diagnostics':
                if (peTelemetry) peTelemetry.drawCharts();
                break;
            case 'charts':
                fluxEnergyChart.draw();
                particleChart.draw();
                break;
            case 'inspector':
                inspector.update();
                break;
        }
    }
}


// =====================================================================
// Exported: loadPEScenario(ctx, name)
// =====================================================================

export function loadPEScenario(ctx, name) {
    const { bridge, viewport } = ctx;

    if (!bridge.initPE) return;

    // Delegate to app.js master reset (clears charts, trails, field cache,
    // and resets all toggle buttons across all scales)
    ctx.resetAllVisualState();

    bridge.initPE();

    // Reset black hole state from any prior scenario
    _bhActive      = false;
    _bhHawkingTick = 0;
    if (viewport && viewport.setEventHorizon) {
        viewport.setEventHorizon(false, 0);
    }

    // ── PE physics defaults for atomic-scale simulations ────────────
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

    // Sync dt to slider value
    const dtSlider = document.getElementById('pe-dt-slider');
    if (dtSlider) bridge.peSetDt(parseFloat(dtSlider.value));

    // ── Orbital velocity helper ─────────────────────────────────────
    // Plummer force: F = alpha * |Q| * r / (4pi * (r^2 + soft^2)^(3/2))
    // Equilibrium:   m * v^2 / r = F
    //            ->  v = sqrt(alpha * |Q| * r / (4pi * m * (r^2 + soft^2)))
    const ALPHA_PE = ALPHA;
    const soft2 = 0.01;  // 0.1^2
    const orbitalV = (m, r, Q = 1) =>
        Math.sqrt(ALPHA_PE * Q * r / (4 * Math.PI * m * (r * r + soft2)));

    // ── Particle masses (MeV) ───────────────────────────────────────
    const constants = {
        me:   K_B,           mp:   M_P_PHYS,
        mmu:  M_MU_PHYS,     mn:   M_N_PHYS,
        mpi:  M_PI_CH_PHYS,  mK:   M_K_CH_PHYS,
        mtau: M_TAU_PHYS,    mW:   M_W_PHYS,
        mSig: M_SIGMA_PHYS,  mOmg: M_OMEGA_PHYS,
        mDel: M_DELTA_PHYS,  RE:   0.1,
        ALPHA_PE, soft2, orbitalV,
        BH_MASS: _BH_MASS, BH_TEST_MASS: _BH_TEST_MASS,
        BH_HORIZON_R: _BH_HORIZON_R,
        G_N, C_SPEED,
    };

    const result = setupPEScenario(name, { bridge, viewport, constants });

    // Apply BH state hint from scenario (only pe-micro-bh sets bhActive=true)
    if (result && result.bhActive) {
        _bhActive      = true;
        _bhHawkingTick = 0;
    }
}


// =====================================================================
// Exported: bindScale1ControlsUI()
// =====================================================================

export function bindScale1ControlsUI() {
    const controlsPanel = document.getElementById('panel-controls');
    if (controlsPanel) new Scale1ControlsComponent(controlsPanel).init();
}
