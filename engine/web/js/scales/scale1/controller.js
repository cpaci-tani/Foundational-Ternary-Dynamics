/**
 * Scale 1 (Particles) Controller
 *
 * Extracted from app_dag.js to isolate the Particle Engine (PE) frame loop,
 * scenario loader, cloud rendering, trail history, and field overlay logic.
 *
 * Owns all PE-specific state internally:
 *   - Cloud templates and cloud-to-particle mapping
 *   - Trail history circular buffers
 *   - Field toggle flags (E-field, potential, gravity, forces)
 *   - Velocity/trail display flags
 *   - Black hole scenario state
 *   - Field grid cache and source particle buffer
 *
 * Exports:
 *   animatePE(ctx)             — per-frame update (app_dag.js lines ~1052-1207)
 *   loadPEScenario(ctx, name)  — scenario setup  (app_dag.js lines ~3085-3423)
 *   resetScale1(ctx)           — clear PE-specific state for mode switch
 *
 * The ctx object provides shared app resources:
 *   { bridge, viewport, running, ticksPerFrame, tickAccumulator,
 *     inspector, fluxEnergyChart, particleChart, peTelemetry,
 *     activeTab, frameCount, dom,
 *     updateOnticPanel, updateHierarchyPanel }
 *
 * ---------------------------------------------------------------
 * DELEGATION STUBS: after wiring this controller into app_dag.js,
 * the following app_dag.js functions should become thin wrappers:
 *
 *   function animatePE(now) {
 *       return scale1Controller.animatePE({ ...ctx, now });
 *   }
 *
 *   function loadPEScenario(name) {
 *       return scale1Controller.loadPEScenario(ctx, name);
 *   }
 *
 * And the following app_dag.js code blocks can be removed:
 *   - Cloud rendering section   (lines ~397-536)
 *   - PE state variables         (lines ~82-85, 119-126, 405-414)
 *   - expandPEToCloud()          (lines ~455-510)
 *   - ensureCloudTemplate()      (lines ~416-453)
 *   - updateTrailHistory()       (lines ~512-535)
 * ---------------------------------------------------------------
 */

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


// =====================================================================
// PE-Specific Module State
// =====================================================================

// -- Cloud rendering buffers (pre-allocated, reused every frame) ------
//    Each PE particle is rendered as a Gaussian flux cloud, not a point.
//    Cloud point count ~ mass (electron 0.511 MeV -> 511 cloud points).
const MAX_CLOUD_TOTAL = 100000;
const _cloudPos  = new Float32Array(MAX_CLOUD_TOTAL * 3);
const _cloudCol  = new Float32Array(MAX_CLOUD_TOTAL * 3);
const _cloudSize = new Float32Array(MAX_CLOUD_TOTAL);
const _cloudParticleMap = new Int32Array(MAX_CLOUD_TOTAL); // cloud index -> PE particle ID

// -- Cloud template cache (one per particle catalog ID) ---------------
const _cloudTemplates = new Map();

// -- Trail history (circular buffers per particle) --------------------
const TRAIL_MAX_LENGTH = 200;
const _trailHistory = new Map(); // particleId -> { positions: Float32Array, head, length }

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
let _lastCloudTime = -1;           // cached `t` for cloud breathing; skip recompute when paused
let _lastCloudData = null;         // cached cloud output when paused


// =====================================================================
// Internal Helpers
// =====================================================================

// formatNumber helper removed -- use formatSI from scale-utils.js instead

/**
 * Generate (or retrieve cached) a Gaussian cloud template for a given
 * particle type.  Point count scales sub-linearly with mass so heavier
 * particles get denser clouds without blowing the budget.
 *
 * Template fields: { n, radius, offsets: Float32Array, brightness: Float32Array }
 *
 * Originally app_dag.js lines ~416-453.
 */
function ensureCloudTemplate(catalogId, mass_mev) {
    if (_cloudTemplates.has(catalogId)) return _cloudTemplates.get(catalogId);

    // Point count: electron (0.511 MeV) -> 511 pts; proton (938) -> ~3000
    const nRaw = Math.round(603 * Math.pow(mass_mev, 0.238));
    const n = Math.min(Math.max(nRaw, 50), 5000);

    // Cloud radius: lighter particles are more spread out (Compton-like)
    const radius = 2.0 + 3.0 * Math.pow(K_B / mass_mev, 0.15);
    const sigma = radius / 2.5; // ~95% within radius

    const offsets    = new Float32Array(n * 3);
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

        offsets[i * 3]     = ox;
        offsets[i * 3 + 1] = oy;
        offsets[i * 3 + 2] = oz;

        const dist = Math.sqrt(ox * ox + oy * oy + oz * oz) / radius;
        brightness[i] = Math.exp(-dist * dist * 2.0); // Gaussian falloff
    }

    const tmpl = { n, radius, offsets, brightness };
    _cloudTemplates.set(catalogId, tmpl);
    return tmpl;
}

/**
 * Expand PE particle centers into flux cloud point data suitable for
 * the viewport point cloud renderer.
 *
 * Each particle is replaced by N Gaussian-distributed cloud points with
 * per-frame sinusoidal "breathing" motion for organic visual quality.
 *
 * Returns { positions, colors, sizes, count } referencing the module-level
 * pre-allocated buffers (zero-copy for the viewport).
 *
 * Originally app_dag.js lines ~455-510.
 */
function expandPEToCloud(peData, typeMap, t) {
    const srcCount = peData.count;
    let out = 0;

    for (let i = 0; i < srcCount && out < MAX_CLOUD_TOTAL; i++) {
        const cx = peData.positions[i * 3];
        const cy = peData.positions[i * 3 + 1];
        const cz = peData.positions[i * 3 + 2];

        const pid   = peData.ids ? peData.ids[i] : -1;
        const catId = typeMap ? typeMap.get(pid) : null;
        const p     = catId ? getById(catId) : null;

        if (p) {
            const tmpl = ensureCloudTemplate(catId, p.mass_mev);
            const [cr, cg, cb] = p.display_color;
            const n = Math.min(tmpl.n, MAX_CLOUD_TOTAL - out);
            const wiggle = 0.15 * tmpl.radius; // 15% of cloud radius

            for (let j = 0; j < n; j++) {
                // Per-point sinusoidal perturbation for organic "breathing" motion.
                // Golden angle phase spacing ensures adjacent points move independently.
                const phase = j * 2.39996323;
                const fx = Math.sin(t * 1.7 + phase) * wiggle;
                const fy = Math.sin(t * 2.3 + phase * 1.3) * wiggle;
                const fz = Math.sin(t * 1.1 + phase * 0.7) * wiggle;

                _cloudPos[out * 3]     = cx + tmpl.offsets[j * 3]     + fx;
                _cloudPos[out * 3 + 1] = cy + tmpl.offsets[j * 3 + 1] + fy;
                _cloudPos[out * 3 + 2] = cz + tmpl.offsets[j * 3 + 2] + fz;

                const b = tmpl.brightness[j];
                _cloudCol[out * 3]     = cr * b;
                _cloudCol[out * 3 + 1] = cg * b;
                _cloudCol[out * 3 + 2] = cb * b;

                _cloudSize[out] = 1.5 + b * 1.5; // 1.5 at edge -> 3.0 at center
                _cloudParticleMap[out] = pid;
                out++;
            }
        } else {
            // Fallback: single point for untyped particles
            _cloudPos[out * 3]     = cx;
            _cloudPos[out * 3 + 1] = cy;
            _cloudPos[out * 3 + 2] = cz;
            _cloudCol[out * 3]     = 0.5;
            _cloudCol[out * 3 + 1] = 0.5;
            _cloudCol[out * 3 + 2] = 0.5;
            _cloudSize[out] = 3.0;
            _cloudParticleMap[out] = pid;
            out++;
        }
    }

    return { positions: _cloudPos, colors: _cloudCol, sizes: _cloudSize, count: out };
}

/**
 * Record current particle positions into per-particle circular trail buffers.
 * Prunes trails for particles that no longer exist.
 *
 * Originally app_dag.js lines ~512-535.
 */
function updateTrailHistory(peData) {
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
        trail.positions[h * 3]     = peData.positions[i * 3];
        trail.positions[h * 3 + 1] = peData.positions[i * 3 + 1];
        trail.positions[h * 3 + 2] = peData.positions[i * 3 + 2];
        trail.head   = (h + 1) % TRAIL_MAX_LENGTH;
        trail.length = Math.min(trail.length + 1, TRAIL_MAX_LENGTH);
    }

    // Remove trails for particles that no longer exist
    const activeIds = new Set();
    for (let i = 0; i < peData.count; i++) activeIds.add(peData.ids[i]);
    for (const [id] of _trailHistory) {
        if (!activeIds.has(id)) _trailHistory.delete(id);
    }
}


// =====================================================================
// Exported: Field Toggle Setters
// =====================================================================
// These are called by the UI wiring code in app_dag.js when the user clicks
// Scale 1 overlay buttons.  They update module-internal toggle state.

export function setPEEField(on)    { _showPEEField    = on; }
export function setPEPotential(on) { _showPEPotential = on; }
export function setPEGravField(on) { _showPEGravField = on; }
export function setPEForces(on)    { _showPEForces    = on; }
export function setVelocities(on)  { _showVelocities  = on; }
export function setTrails(on)      { _showTrails      = on; }

/** Read-only access to the cloud-to-particle mapping array. */
export function getCloudParticleMap() { return _cloudParticleMap; }

/** Read-only access to trail history for external consumers. */
export function getTrailHistory()    { return _trailHistory; }


// =====================================================================
// Exported: resetScale1(ctx)
// =====================================================================
// Clear all PE-specific internal state.  Called on engine mode switch
// (not on scenario change within Scale 1 -- that uses _resetAllVisualState
// in app_dag.js which delegates to the toggle resets below).

/**
 * Reset PE-internal state for a clean mode switch.
 * ctx.viewport is used to clear viewport overlays.
 *
 * NOTE: The visual toggle button DOM updates (removing .active class)
 * remain in app_dag.js _resetAllVisualState() for now, because the DOM
 * elements are shared across scales and managed by the central reset.
 */
export function resetScale1(ctx) {
    const { viewport } = ctx;

    // Clear cloud template cache (forces re-generation on next load)
    _cloudTemplates.clear();

    // Clear trail history
    _trailHistory.clear();

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
// Per-frame update for Scale 1 (Particles).
// Originally app_dag.js lines ~1052-1207.
//
// Responsibilities:
//   1. Tick the PE simulation (accumulator handles sub-1 speeds)
//   2. Hawking emission for black hole scenario
//   3. Expand particle centers into flux clouds
//   4. Update velocity vectors and orbit trails
//   5. Compute and display field overlays (Coulomb, gravity, forces)
//   6. Render the viewport
//   7. Update diagnostics, charts, and panels (throttled to every 3rd frame)

export function animatePE(ctx) {
    const {
        bridge, viewport, running, ticksPerFrame, inspector,
        fluxEnergyChart, particleChart, peTelemetry,
        activeTab, frameCount, dom, now,
        updateOnticPanel, updateHierarchyPanel
    } = ctx;

    // ── 1. Tick PE simulation if running ────────────────────────────
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
    // PERF: When paused, skip the expensive cloud expansion (up to 100K
    // sin/cos per frame for breathing animation).  Reuse cached output
    // until the simulation resumes or particle data changes.
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
        inspector.setPEContext(_cloudParticleMap, cloud.count, typeMap);
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
        viewport.updateTrails(_trailHistory, typeMap);
    }

    // ── 5. PE Field Overlays (individual force decomposition) ───────
    // PERF: Skip field recomputation when paused — particle positions haven't changed.

    // Coulomb potential heatmap + force vectors (XZ plane)
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
        // Convert flat positions array to {x,y,z} objects for generateEFieldSeeds.
        // Reuse buffer -- resize only when particle count changes.
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
    // PERF: When paused, data is identical — push once then skip.
    if (frameCount % 3 === 0 && (running || !_diagPushedWhilePaused)) {
        const diag = bridge.peGetDiagnostics();

        // Update status bar with dedup (avoids DOM thrash when paused)
        const sTick = formatSI(diag.tick);
        const sParticles = String(diag.particleCount);
        const sEnergy = formatEnergy(diag.totalEnergy, 1).text;
        const sState = running ? 'Running' : 'Idle';

        if (_statusCache.tick !== sTick) { dom.statusTick.textContent = sTick; dom.statusPtime.textContent = sTick; _statusCache.tick = sTick; }
        if (_statusCache.particles !== sParticles) { dom.statusParticles.textContent = sParticles; _statusCache.particles = sParticles; }
        if (_statusCache.energy !== sEnergy) { dom.statusEnergy.textContent = sEnergy; _statusCache.energy = sEnergy; }
        if (_statusCache.state !== sState) {
            dom.statusState.textContent = sState;
            _statusCache.state = sState;
            if (running) dom.statusDot.classList.remove('idle');
            else dom.statusDot.classList.add('idle');
        }

        // Update PE telemetry panel
        const ext = bridge.peGetExtendedData();
        if (peTelemetry) peTelemetry.update(diag, ext);

        // Feed charts with adapted data structure
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

        // Track paused-state dedup
        if (!running) _diagPushedWhilePaused = true;
        else _diagPushedWhilePaused = false;

        // Update active side-panel visuals
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
            case 'hierarchy':
                updateHierarchyPanel();
                break;
        }
    }
}


// =====================================================================
// Exported: loadPEScenario(ctx, name)
// =====================================================================
// Set up a specific PE scenario by name.
// Initializes the PE bridge, configures physics toggles, and spawns
// particles with appropriate masses, charges, and orbital velocities.
//
// Originally app_dag.js lines ~3085-3423.
//
// NOTE: ctx.resetAllVisualState() is called first to clear all cross-scale
// visual state.  That function lives in app_dag.js because it touches DOM
// elements and viewport toggles shared across all scales.

export function loadPEScenario(ctx, name) {
    const { bridge, viewport } = ctx;

    if (!bridge.initPE) return;

// Delegate to app_dag.js master reset (clears charts, trails, field cache,
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
    // - Coulomb ON (electromagnetic binding)
    // - Damping OFF (Larmor radiation is Scale 0 only)
    // - Gravity OFF (G_N=0.01 is lattice-scale; real alpha_G ~ 6e-39)
    // - Low softening so Coulomb force is strong enough for bound orbits
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

    // ── Orbital velocity helper ─────────────────────────────────────
    // Plummer force: F = alpha * |Q| * r / (4pi * (r^2 + soft^2)^(3/2))
    // Equilibrium:   m * v^2 / r = F
    //            ->  v = sqrt(alpha * |Q| * r / (4pi * m * (r^2 + soft^2)))
    const ALPHA_PE = ALPHA; // use imported ALPHA (full precision 1/137.036...)
    const soft2 = 0.01;  // 0.1^2
    const orbitalV = (m, r, Q = 1) =>
        Math.sqrt(ALPHA_PE * Q * r / (4 * Math.PI * m * (r * r + soft2)));

    // ── Particle masses (MeV) ───────────────────────────────────────
    // `me` uses K_B (the FTD-derived electron mass scale, which equals
    // the PDG value 0.511 by construction). All remaining masses are
    // PDG experimental reference values from constants.js; they are
    // intentionally NOT unified with the FTD-derived M_PROTON etc.
    // because the framework scale (M_PROTON ≈ 1798 MeV) differs from
    // the physical scale (M_P_PHYS ≈ 938 MeV) by design (CLAUDE.md).
    const me   = K_B;           // electron (FTD = PDG = 0.511 MeV)
    const mp   = M_P_PHYS;      // proton
    const mmu  = M_MU_PHYS;     // muon
    const mn   = M_N_PHYS;      // neutron
    const mpi  = M_PI_CH_PHYS;  // charged pion
    const mK   = M_K_CH_PHYS;   // charged kaon
    const mtau = M_TAU_PHYS;    // tau
    const mW   = M_W_PHYS;      // W boson
    const mSig = M_SIGMA_PHYS;  // Sigma+
    const mOmg = M_OMEGA_PHYS;  // Omega-
    const mDel = M_DELTA_PHYS;  // Delta++
    const RE   = 0.1;           // effective radius (tiny -- no false annihilation)

    // ── Scenario switch ─────────────────────────────────────────────
    switch (name) {

        // -- Hydrogen: locked proton + orbiting electron ─────────────
        case 'pe-hydrogen': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // -- Helium: locked He nucleus (2p+2n), 2 orbiting electrons ─
        case 'pe-helium': {
            const r = 4;
            const v = orbitalV(me, r, 2); // Q=2
            bridge.peAddLockedParticle('proton', 1, 0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('proton', 1, -0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, 0, 0.3, 0, mn, RE);
            bridge.peAddLockedParticle('neutron', 0, 0, -0.3, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('electron', -1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }

        // -- Positronium: e+/e- orbiting common center of mass ───────
        case 'pe-positronium': {
            const r = 5;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r / (4 * Math.PI * me * (sep * sep + soft2)));
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('positron', 1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }

        // -- Muonium: locked mu+ + orbiting electron ─────────────────
        case 'pe-muonium': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('mu_plus', 1, 0, 0, 0, mmu, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // -- Rutherford scattering: proton + electron approach ───────
        case 'pe-scattering': {
            const v_app = 0.005;
            bridge.peAddParticle('proton', 1, -15, 0, 0, v_app, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, 15, 3, 0, -v_app * 10, 0, 0, me, RE);
            break;
        }

        // -- Three-body: 2 locked protons + 1 electron ───────────────
        case 'pe-three-body': {
            const r = 8;
            const v = orbitalV(me, r, 2); // total Q=2
            bridge.peAddLockedParticle('proton', 1, -3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('proton', 1, 3, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, 0, r, 0, v, 0, 0, me, RE);
            break;
        }

        // -- Deuteron: locked (p+n) + orbiting electron ──────────────
        case 'pe-deuteron': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('proton', 1, 0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, -0.3, 0, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // ── Lepton scenarios ────────────────────────────────────────

        // -- True muonium: mu+/mu- bound state ───────────────────────
        case 'pe-true-muonium': {
            const r = 3;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r / (4 * Math.PI * mmu * (sep * sep + soft2)));
            bridge.peAddParticle('antimuon', 1, r, 0, 0, 0, v, 0, mmu, RE);
            bridge.peAddParticle('muon', -1, -r, 0, 0, 0, -v, 0, mmu, RE);
            break;
        }

        // -- Tauonium: tau+/tau- bound state (tight orbit) ───────────
        case 'pe-tauonium': {
            const r = 2;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r / (4 * Math.PI * mtau * (sep * sep + soft2)));
            bridge.peAddParticle('antitau', 1, r, 0, 0, 0, v, 0, mtau, RE);
            bridge.peAddParticle('tau', -1, -r, 0, 0, 0, -v, 0, mtau, RE);
            break;
        }

        // -- Tauonic hydrogen: locked proton + orbiting tau- ─────────
        case 'pe-tau-atom': {
            const r = 1.5;
            const v = orbitalV(mtau, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('tau', -1, r, 0, 0, 0, v, 0, mtau, RE);
            break;
        }

        // ── Exotic atom scenarios ───────────────────────────────────

        // -- Pionic hydrogen: pi- orbiting locked proton ─────────────
        case 'pe-pionic-hydrogen': {
            const r = 4;
            const v = orbitalV(mpi, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('pion_minus', -1, r, 0, 0, 0, v, 0, mpi, RE);
            break;
        }

        // -- Kaonic hydrogen: K- orbiting locked proton ──────────────
        case 'pe-kaonic-hydrogen': {
            const r = 4;
            const v = orbitalV(mK, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('kaon_minus', -1, r, 0, 0, 0, v, 0, mK, RE);
            break;
        }

        // -- Sigma+ atom: electron orbiting locked Sigma+ ────────────
        case 'pe-sigma-plus-atom': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('sigma_plus', 1, 0, 0, 0, mSig, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // -- Protonium: p/p-bar orbiting common center of mass ───────
        case 'pe-antiprotonic-hydrogen': {
            const r = 3;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r / (4 * Math.PI * mp * (sep * sep + soft2)));
            bridge.peAddParticle('proton', 1, r, 0, 0, 0, v, 0, mp, RE);
            bridge.peAddParticle('antiproton', -1, -r, 0, 0, 0, -v, 0, mp, RE);
            break;
        }

        // ── Hadron scenarios ────────────────────────────────────────

        // -- Pionium: pi+/pi- Coulomb bound state ────────────────────
        case 'pe-pion-orbit': {
            const r = 4;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r / (4 * Math.PI * mpi * (sep * sep + soft2)));
            bridge.peAddParticle('pion_plus', 1, r, 0, 0, 0, v, 0, mpi, RE);
            bridge.peAddParticle('pion_minus', -1, -r, 0, 0, 0, -v, 0, mpi, RE);
            break;
        }

        // -- Kaonium: K+/K- Coulomb bound state ──────────────────────
        case 'pe-kaon-pair': {
            const r = 4;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r / (4 * Math.PI * mK * (sep * sep + soft2)));
            bridge.peAddParticle('kaon_plus', 1, r, 0, 0, 0, v, 0, mK, RE);
            bridge.peAddParticle('kaon_minus', -1, -r, 0, 0, 0, -v, 0, mK, RE);
            break;
        }

        // -- Delta++ system: 2 locked +1 charges + 2 electrons ───────
        case 'pe-delta-system': {
            const r = 4;
            const v = orbitalV(me, r, 2);
            bridge.peAddLockedParticle('delta_pp_a', 1, 0.3, 0, 0, mDel / 2, RE);
            bridge.peAddLockedParticle('delta_pp_b', 1, -0.3, 0, 0, mDel / 2, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('electron', -1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }

        // -- Omega- scattering: locked Omega- + approaching positron ─
        case 'pe-omega-scattering': {
            const v_app = 0.004;
            bridge.peAddLockedParticle('omega_minus', -1, 0, 0, 0, mOmg, RE);
            bridge.peAddParticle('positron', 1, -15, 2, 0, v_app, 0, 0, me, RE);
            break;
        }

        // ── Nuclear scenarios ───────────────────────────────────────

        // -- Tritium: locked (p+n+n) nucleus + orbiting electron ─────
        case 'pe-tritium': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0.3, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, 0.3, -0.2, 0, mn, RE);
            bridge.peAddLockedParticle('neutron', 0, -0.3, -0.2, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // -- Helion / He-3: locked (2p+n) + 2 orbiting electrons ─────
        case 'pe-helion': {
            const r = 4;
            const v = orbitalV(me, r, 2);
            bridge.peAddLockedParticle('proton', 1, 0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('proton', 1, -0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, 0, 0.3, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('electron', -1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }

        // ── Boson scenarios ─────────────────────────────────────────

        // -- W+/W- pair in mutual Coulomb orbit ──────────────────────
        case 'pe-w-pair': {
            const r = 2;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r / (4 * Math.PI * mW * (sep * sep + soft2)));
            bridge.peAddParticle('w_plus', 1, r, 0, 0, 0, v, 0, mW, RE);
            bridge.peAddParticle('w_minus', -1, -r, 0, 0, 0, -v, 0, mW, RE);
            break;
        }

        // ── Scattering scenarios ────────────────────────────────────

        // -- Meson scattering: pi+ approaching locked proton (repulsive)
        case 'pe-meson-scattering': {
            const v_app = 0.006;
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('pion_plus', 1, -15, 2, 0, v_app, 0, 0, mpi, RE);
            break;
        }

        // -- Muon scattering: mu- approaching locked proton (attractive)
        case 'pe-muon-scattering': {
            const v_app = 0.008;
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('muon', -1, -15, 2, 0, v_app, 0, 0, mmu, RE);
            break;
        }

        // ── Gravity scenarios ───────────────────────────────────────

        // -- Micro Black Hole: FTD lattice accretion demo ────────────
        // [SELECTION] M_BH=5000 MeV, radii, Hawking rate are pedagogical choices
        // [EMERGENT] C_SPEED cap creates inspiral zone at r < ~10
        case 'pe-micro-bh': {
            // Override: gravity dominates, no Coulomb
            bridge.peSetCoulomb(false);
            bridge.peSetGravity(true);
            bridge.peSetDamping(false);
            bridge.peSetSoftening(1.0);
            const peCoulombEl2 = document.getElementById('pe-coulomb');
            const peGravityEl2 = document.getElementById('pe-gravity');
            const peDampingEl2 = document.getElementById('pe-damping');
            if (peCoulombEl2) peCoulombEl2.checked = false;
            if (peGravityEl2) peGravityEl2.checked = true;
            if (peDampingEl2) peDampingEl2.checked = false;

            // BH locked at origin -- neutral, enormous mass
            bridge.peAddLockedParticle('neutron', 0, 0, 0, 0, _BH_MASS, 0.5);

            // Gravity-only orbital velocity with Plummer softening
            const soft2_bh = 1.0;
            const gravOrbitalV = (r) =>
                Math.sqrt(G_N * _BH_MASS * r / Math.pow(r * r + soft2_bh, 1.5));

            // ZONE 1: Inspiral donors at r=8 (v_circ > C_SPEED -- will spiral in)
            const r_fall = 8, v_fall = 0.45;
            const angles_fall = [0, Math.PI / 2, Math.PI, 3 * Math.PI / 2];
            for (const a of angles_fall) {
                bridge.peAddParticle('neutron', 0,
                    r_fall * Math.cos(a), 0, r_fall * Math.sin(a),
                    -v_fall * Math.sin(a), 0, v_fall * Math.cos(a),
                    _BH_TEST_MASS, 0.1);
            }

            // ZONE 2: Accretion ring at r=16 (v_circ < C_SPEED -- stable orbits)
            const r_ring = 16;
            const v_ring = Math.min(gravOrbitalV(r_ring) * 0.92, C_SPEED * 0.92);
            const nRing = 8;
            for (let i = 0; i < nRing; i++) {
                const a = (i / nRing) * 2 * Math.PI;
                bridge.peAddParticle('neutron', 0,
                    r_ring * Math.cos(a), 0, r_ring * Math.sin(a),
                    -v_ring * Math.sin(a), 0, v_ring * Math.cos(a),
                    _BH_TEST_MASS, 0.1);
            }

            // ZONE 3: Far escapers at r=26 (slightly super-circular)
            const r_far = 26;
            const v_far = gravOrbitalV(r_far) * 1.05;
            bridge.peAddParticle('neutron', 0,
                r_far, 0, 0, 0, 0, v_far, _BH_TEST_MASS, 0.1);
            bridge.peAddParticle('neutron', 0,
                -r_far, 0, 0, 0, 0, -v_far, _BH_TEST_MASS, 0.1);

            // Activate Hawking emission + event horizon visual
            _bhActive      = true;
            _bhHawkingTick = 0;
            if (viewport && viewport.setEventHorizon) {
                viewport.setEventHorizon(true, _BH_HORIZON_R);
            }
            break;
        }

        // -- Custom / Empty: user injects manually via Zoo or controls
        case 'pe-custom':
        default:
            break;
    }
}


// =====================================================================
// Exported: bindScale1ControlsUI()
// =====================================================================
// Mount the Scale 1 control card into the controls panel.
// Called once during app startup after the DOM is ready.

export function bindScale1ControlsUI() {
    const controlsPanel = document.getElementById('panel-controls');
    if (controlsPanel) new Scale1ControlsComponent(controlsPanel).init();
}
