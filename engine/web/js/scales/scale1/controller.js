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
import { formatEnergy } from '../../units.js';
import {
    generateGridXZ, samplePECoulombOnly,
    samplePEGravityField, makePECoulombFieldFn
} from '../../fields.js';
import {
    computeStreamlines, generateEFieldSeeds
} from '../../fieldlines.js';
import {
    C_SPEED, G_PE, K_B, GRAVITY_VIS_GAIN,
    M_P_PHYS, M_MU_PHYS, M_N_PHYS, M_PI_CH_PHYS, M_K_CH_PHYS,
    M_TAU_PHYS, M_W_PHYS, M_SIGMA_PHYS, M_OMEGA_PHYS, M_DELTA_PHYS
} from '../../constants.js';
import { createTickAccumulator, formatSI } from '../scale-utils.js';
import { Scale1ControlsComponent } from './ui/controls/component.js';
import {
    expandPEToCloud, buildPEManifestBlinkRate, updateTrailHistory,
    getCloudParticleMap, getTrailHistory, clearCloudAndTrails, MANIFEST_FILL
} from './pe-cloud-expander.js';
import { setupPEScenario, getPEScenarioPreset } from './scenarios.js';
import { telemetryHub } from '../../telemetry-hub.js';


// =====================================================================
// PE-Specific Module State
// =====================================================================

// -- Field overlay toggle flags ---------------------------------------
let _showPEEField    = false;   // E-field streamlines
let _showPEPotential = false;   // Coulomb potential heatmap
let _showPEGravField = false;   // Gravity field vectors (grid)
let _showPEForceCoulomb = false;
let _showPEForceGravity = false;
let _showPEForceStrong  = false;
let _showPEForceNet     = false;   // Net per-particle force arrows
let _showPESystem    = false;   // System observables: CoM + momentum + ang. mom.
let _showVelocities  = false;   // Velocity vectors overlay
let _showTrails      = false;   // Orbit trail lines

// -- Field computation cache ------------------------------------------
let _fieldGrid       = null;    // cached grid from generateGridXZ
const _srcParticlesBuf = [];    // reusable {x,y,z} array for field seed generation

// -- Black hole scenario state ----------------------------------------
let _bhActive       = false;
let _bhHawkingTick  = 0;
// [IMPOSED] pedagogical toy values — the micro-BH demo is Newtonian
// gravity + a visual horizon/emission cadence, NOT a GR solver (see
// USER_GUIDE §Scale 1). Mass/horizon/interval chosen for legibility.
const _BH_HAWKING_INTERVAL = 300;
const _BH_HORIZON_R = 3.0;
const _BH_MASS      = 5000;
const _BH_TEST_MASS = K_B;   // electron mass (MeV) — Hawking test particle

// -- Tick accumulator (sub-1 speed fractional ticks, shared helper) ----
const _tickAcc = createTickAccumulator();

// -- Paused-state dedup (avoid redundant work when simulation idle) ----
let _statusCache = { tick: '', ptime: '', particles: '', energy: '', state: '' };


// =====================================================================
// Exported: Field Toggle Setters + cloud/trail accessors
// =====================================================================

export function setPEEField(on)    { _showPEEField    = on; }
export function setPEPotential(on) { _showPEPotential = on; }
export function setPEGravField(on)    { _showPEGravField = on; }
export function setPEForceCoulomb(on)  { _showPEForceCoulomb = on; }
export function setPEForceGravity(on)  { _showPEForceGravity = on; }
export function setPEForceStrong(on)   { _showPEForceStrong = on; }
export function setPEForceNet(on)      { _showPEForceNet = on; }
/** @deprecated Use setPEForceNet — kept for callers that still say "forces". */
export function setPEForces(on)        { setPEForceNet(on); }
export function setPESystem(on)    { _showPESystem    = on; }
export function setVelocities(on)  { _showVelocities  = on; }
export function setTrails(on)      { _showTrails      = on; }

// Re-export cloud/trail accessors so external consumers have a single
// import surface (controller.js) and need not know about the split file.
export { getCloudParticleMap, getTrailHistory };

function setButtonActive(id, on) {
    const btn = document.getElementById(id);
    if (!btn) return;
    btn.classList.toggle('active', !!on);
    btn.setAttribute('aria-pressed', on ? 'true' : 'false');
}

function setCheckbox(id, on) {
    const el = document.getElementById(id);
    if (el) el.checked = !!on;
}

function setSliderValue(id, value, digits) {
    const slider = document.getElementById(id);
    if (!slider || value === undefined || value === null) return;
    slider.value = String(value);
    const valueEl = document.getElementById(id.replace('-slider', '-value'));
    if (valueEl) valueEl.textContent = Number(value).toFixed(digits);
}

function applyPEPhysicsPreset(bridge, preset) {
    const p = preset.physics || {};
    bridge.peSetCoulomb?.(!!p.coulomb);
    bridge.peSetGravity?.(!!p.gravity);
    bridge.peSetDamping?.(!!p.damping);
    bridge.peSetLorentz?.(!!p.lorentz);
    bridge.peSetExchange?.(!!p.exchange);
    bridge.peSetStrong?.(!!p.strong);
    bridge.peSetMagneticDipole?.(!!p.magnetic_dipole);
    bridge.peSetSpinOrbit?.(!!p.spin_orbit);
    bridge.peSetRadiation?.(!!p.radiation);
    bridge.peSetRelativistic?.(!!p.relativistic);
    bridge.peSetRelativisticVerlet?.(!!p.relativistic_verlet);

    if (p.dt !== undefined) {
        bridge.peSetDt?.(p.dt);
        setSliderValue('pe-dt-slider', p.dt, 1);
    }
    if (p.softening !== undefined) {
        bridge.peSetSoftening?.(p.softening);
        setSliderValue('pe-soft-slider', p.softening, 2);
    }

    setCheckbox('pe-coulomb', p.coulomb);
    setCheckbox('pe-gravity', p.gravity);
    setCheckbox('pe-damping', p.damping);
    setCheckbox('pe-lorentz-p', p.lorentz);
    setCheckbox('pe-exchange', p.exchange);
    setCheckbox('pe-strong', p.strong);
    setCheckbox('pe-magnetic-dipole', p.magnetic_dipole);
    setCheckbox('pe-spin-orbit', p.spin_orbit);
    setCheckbox('pe-radiation', p.radiation);
    setCheckbox('pe-relativistic', p.relativistic);
    setButtonActive('toggle-pe-gravity', p.gravity);
    setButtonActive('toggle-pe-damping', p.damping);
}

function applyPEOverlayPreset(viewport, preset) {
    const o = preset.overlays || {};
    _showVelocities = !!o.velocities;
    _showTrails = !!o.trails;
    _showPEEField = !!o.efield;
    _showPEPotential = !!o.potential;
    _showPEGravField = !!o.gravityField;
    _showPEForceCoulomb = !!(o.forceCoulomb ?? o.forces);
    _showPEForceGravity = !!o.forceGravity;
    _showPEForceStrong  = !!o.forceStrong;
    _showPEForceNet     = !!(o.forceNet ?? o.forces);
    _showPESystem = !!o.system;

    setButtonActive('toggle-velocities', _showVelocities);
    setButtonActive('toggle-trails', _showTrails);
    setButtonActive('toggle-pe-efield', _showPEEField);
    setButtonActive('toggle-pe-potential', _showPEPotential);
    setButtonActive('toggle-pe-gravity-field', _showPEGravField);
    setButtonActive('toggle-pe-force-coulomb', _showPEForceCoulomb);
    setButtonActive('toggle-pe-force-gravity', _showPEForceGravity);
    setButtonActive('toggle-pe-force-strong', _showPEForceStrong);
    setButtonActive('toggle-pe-force-net', _showPEForceNet);
    setButtonActive('toggle-pe-system', _showPESystem);

    if (!viewport) return;
    viewport.toggleVelocityVectors(_showVelocities);
    viewport.toggleTrails(_showTrails);
    viewport.togglePEStreamlines(_showPEEField);
    viewport.toggleFieldHeatmap(_showPEPotential);
    viewport.toggleFieldVectors(_showPEPotential);
    viewport.toggleGravityVectors(_showPEGravField);
    viewport.togglePEForceCoulomb(_showPEForceCoulomb);
    viewport.togglePEForceGravity(_showPEForceGravity);
    viewport.togglePEForceStrong(_showPEForceStrong);
    viewport.togglePEForceNet(_showPEForceNet);
    viewport.togglePESystem(_showPESystem);
}


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
    _showPEForceCoulomb = false;
    _showPEForceGravity = false;
    _showPEForceStrong  = false;
    _showPEForceNet     = false;
    _showPESystem    = false;
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

    // Clear viewport overlays if available
    if (viewport) {
        viewport.togglePEStreamlines(false);
        viewport.toggleFieldHeatmap(false);
        viewport.toggleFieldVectors(false);
        viewport.toggleGravityVectors(false);
        viewport.toggleParticleForces(false);
        viewport.togglePESystem(false);
        viewport.toggleVelocityVectors(false);
        viewport.toggleTrails(false);
        viewport.toggleSpinVectors?.(false);
        if (viewport.setPEManifestation) viewport.setPEManifestation(false, 0);
    }
}


// =====================================================================
// System observables helper
// =====================================================================

/**
 * Compute system-level observables from a PE particle frame.
 *
 * Returns the mass-weighted center of mass, the total momentum
 * p = Σ mᵢ vᵢ, and the angular momentum about the center of mass
 * L = Σ mᵢ (rᵢ − r_cm) × (vᵢ − v_cm). Using velocities relative to the
 * CoM makes L the intrinsic orbital-plane normal, independent of any
 * bulk drift of the system. Computed from the particle frame directly so
 * it is backend-robust (WASM diagnostics may not populate momentum
 * components).
 *
 * @param {{positions:Float32Array, velocities:Float32Array, masses:Float64Array, count:number}} peData
 * @returns {{com:number[], p:number[], l:number[]}}
 */
function computeSystemVectors(peData) {
    const { positions, velocities, masses, count } = peData;
    let M = 0, cx = 0, cy = 0, cz = 0, px = 0, py = 0, pz = 0;
    for (let i = 0; i < count; i++) {
        const m = masses[i];
        M += m;
        cx += m * positions[i * 3];
        cy += m * positions[i * 3 + 1];
        cz += m * positions[i * 3 + 2];
        px += m * velocities[i * 3];
        py += m * velocities[i * 3 + 1];
        pz += m * velocities[i * 3 + 2];
    }
    if (M <= 0) return { com: [0, 0, 0], p: [0, 0, 0], l: [0, 0, 0] };
    cx /= M; cy /= M; cz /= M;
    const vcx = px / M, vcy = py / M, vcz = pz / M;
    let lx = 0, ly = 0, lz = 0;
    for (let i = 0; i < count; i++) {
        const m = masses[i];
        const rx = positions[i * 3]     - cx;
        const ry = positions[i * 3 + 1] - cy;
        const rz = positions[i * 3 + 2] - cz;
        const wx = velocities[i * 3]     - vcx;
        const wy = velocities[i * 3 + 1] - vcy;
        const wz = velocities[i * 3 + 2] - vcz;
        lx += m * (ry * wz - rz * wy);
        ly += m * (rz * wx - rx * wz);
        lz += m * (rx * wy - ry * wx);
    }
    return { com: [cx, cy, cz], p: [px, py, pz], l: [lx, ly, lz] };
}


// =====================================================================
// Exported: animatePE(ctx)
// =====================================================================

export function animatePE(ctx) {
    const {
        bridge, viewport, running, ticksPerFrame, inspector,
        fluxEnergyChart, particleChart, peTelemetry,
        activeTab, frameCount, dom, now
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

    // ── 3. Cloud expansion: fixed boundary + balanced manifestation blink ─
    const peData  = bridge.peGetParticleData();
    const typeMap = bridge.peGetParticleTypes();
    const forceData = bridge.peGetForces?.() ?? null;
    const blinkRate = buildPEManifestBlinkRate(peData, forceData);
    const frameSec = typeof now === 'number' ? now * 0.001 : performance.now() * 0.001;
    const cloud = expandPEToCloud(peData, typeMap, { blinkRate, frameSec });
    viewport.updateParticles(cloud);

    if (viewport.setPEManifestation) {
        viewport.setPEManifestation(true, frameSec, MANIFEST_FILL);
    }

    if (peData.spinAxes && peData.spins && peData.count > 0) {
        viewport.updateSpinVectors?.(
            peData.positions, peData.spinAxes, peData.spins, peData.count);
        viewport.toggleSpinVectors?.(true);
    } else {
        viewport.toggleSpinVectors?.(false);
    }

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
    if (_showPEPotential && peData.count > 0) {
        if (!_fieldGrid) _fieldGrid = generateGridXZ(25, 20);
        const src   = bridge.peGetFieldSources();
        const field = samplePECoulombOnly(src, _fieldGrid.positions, _fieldGrid.count);
        viewport.updateFieldHeatmap(
            _fieldGrid.positions, field.potentials, _fieldGrid.count, field.maxPotential);
        viewport.updateFieldVectors(
            _fieldGrid.positions, field.forces, _fieldGrid.count, field.maxForce, 8.0);
    }

    // Coulomb E-field streamlines (3D, throttled every 5 frames)
    const refreshStreamlines = running ? frameCount % 5 === 0 : (frameCount % 30 === 0);
    if (_showPEEField && peData.count > 0 && refreshStreamlines) {
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
    if (_showPEGravField && peData.count > 0) {
        if (!_fieldGrid) _fieldGrid = generateGridXZ(25, 20);
        const src   = bridge.peGetFieldSources();
        const field = samplePEGravityField(src, _fieldGrid.positions, _fieldGrid.count);
        viewport.updateGravityVectors(
            _fieldGrid.positions, field.forces, _fieldGrid.count, field.maxForce);
    }

    // Per-particle decomposed force arrows (F_C / F_g / F_S / F_net)
    const anyPEForce = _showPEForceCoulomb || _showPEForceGravity || _showPEForceStrong || _showPEForceNet;
    if (anyPEForce && peData.count > 0) {
        const decomp = bridge.peGetForceDecomposition?.() ?? null;
        if (decomp) {
            viewport.updatePEForceDecomposition(decomp, GRAVITY_VIS_GAIN);
        }
    }

    // System observables: center of mass + total momentum p + ang.-mom. axis L
    if (_showPESystem && peData.count > 0) {
        const sys = computeSystemVectors(peData);
        viewport.updatePESystem(sys.com, sys.p, sys.l);
    }

    // ── 6. Render ───────────────────────────────────────────────────
    viewport.render();

    // ── 7. PE diagnostics (throttled to every 3rd frame) ────────────
    if (frameCount % 3 === 0) {
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
    const preset = getPEScenarioPreset(name);

    // Delegate to app.js master reset (clears charts, trails, field cache,
    // and resets all toggle buttons across all scales)
    ctx.resetAllVisualState();

    bridge.initPE();

    // Re-baseline hub telemetry (pe* ring buffers + _peInitialEnergy drift
    // reference) so the new scenario doesn't inherit the previous one's
    // energy baseline (mirrors loadAEScenario's resetScale(2) wiring).
    telemetryHub.resetScale(1);

    // Reset black hole state from any prior scenario
    _bhActive      = false;
    _bhHawkingTick = 0;
    if (viewport && viewport.setEventHorizon) {
        viewport.setEventHorizon(false, 0);
    }

    applyPEPhysicsPreset(bridge, preset);

    // ── Particle masses (MeV) ───────────────────────────────────────
    const constants = {
        me:   K_B,           mp:   M_P_PHYS,
        mmu:  M_MU_PHYS,     mn:   M_N_PHYS,
        mpi:  M_PI_CH_PHYS,  mK:   M_K_CH_PHYS,
        mtau: M_TAU_PHYS,    mW:   M_W_PHYS,
        mSig: M_SIGMA_PHYS,  mOmg: M_OMEGA_PHYS,
        mDel: M_DELTA_PHYS,  RE:   0.1,
        BH_MASS: _BH_MASS, BH_TEST_MASS: _BH_TEST_MASS,
        BH_HORIZON_R: _BH_HORIZON_R,
        G_PE, C_SPEED,
    };

    const result = setupPEScenario(name, { bridge, viewport, constants });
    applyPEOverlayPreset(viewport, preset);

    // Soft circles + shader manifestation (void slots stay as faint ghosts)
    if (viewport?.setParticleShape) viewport.setParticleShape(0);
    if (viewport?.setParticleGlow) viewport.setParticleGlow(0.28);
    if (viewport?.setPEManifestation) viewport.setPEManifestation(true, 0, MANIFEST_FILL);

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
