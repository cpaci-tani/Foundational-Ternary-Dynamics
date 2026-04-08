/**
 * Scale 0 (Lattice) Controller
 * ────────────────────────────────────────────────────────────────────
 *
 * Owns the Scale 0 frame loop, scenario loading, and field visualization
 * state. Extracted from app.js to isolate lattice-specific logic into a
 * self-contained module that the main app delegates to.
 *
 * WHY THIS EXISTS:
 *   app.js grew to 4000+ lines with six scale-specific animation loops,
 *   scenario loaders, and visualization state all tangled together.
 *   Extracting each scale's controller lets us reason about (and test)
 *   each scale independently, and makes the main app a thin dispatcher.
 *
 * CONTEXT OBJECT (ctx):
 *   The caller passes a ctx object containing shared application state
 *   that this controller reads/writes. This avoids coupling to app.js
 *   module-level variables while still sharing the bridge, viewport, etc.
 *
 *   Required ctx properties:
 *     bridge          - WASMBridge or MockBridge instance
 *     viewport        - Viewport (Three.js) renderer
 *     running         - boolean, true when simulation is playing
 *     ticksPerFrame   - number, simulation ticks per render frame
 *     inspector       - Inspector panel instance
 *     diagnostics     - DiagnosticsPanel instance
 *     fluxEnergyChart - FluxEnergyChart instance
 *     particleChart   - ParticleChart instance
 *     lagrangianChart - LagrangianChart instance
 *     chartCharge     - Sparkline for charge balance (may be null)
 *     chartEBEnergy   - Sparkline for E-B energy (may be null)
 *     chartGauss      - Sparkline for Gauss violation (may be null)
 *     chartEntropy    - Sparkline for entropy (may be null)
 *     peTelemetry     - PETelemetryPanel (may be null)
 *     activeTab       - string, currently selected right-panel tab
 *     frameCount      - number, global frame counter
 *     dom             - object with cached DOM references (statusTick, etc.)
 *     updateOnticPanel  - function, refreshes the ontic panel
 *     updateHierarchyPanel - function, refreshes the hierarchy panel
 *     observatory     - OnticObservatory instance (may be null)
 */

import { MockBridge } from '../../wasm-bridge.js?v=20260318a';
import { computeStreamlines, generateEFieldSeeds, generateBFieldSeeds, generateGridSeeds } from '../../fieldlines.js?v=20260304q';
import { formatEnergy } from '../../units.js';
import { K_B, G_N, DAMPING, K_GENESIS } from '../../constants.js?v=20260305e';
import { SCALE0_TOGGLES, SCALE0_SCENARIO_OVERRIDES, LIGHT_SCENARIO_OVERRIDES } from '../../config/toggles.js';

// ── Field Visualization State (Scale 0 internal) ────────────────────
// These flags track which field overlays are active. They are module-private;
// the rest of the app queries them through getFieldState().
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

// ── Simulation Caches (Scale 0 internal) ────────────────────────────
let _fieldGrid = null;          // cached grid from generateGridXZ
let _srcParticlesBuf = [];      // reusable {x,y,z} array for field seed generation
let _fluxMock = null;           // MockBridge for Scale 0 flux visualization fallback
let _latticeNeedsUpload = true; // set true on scenario load / step / resume
let _tickAccumulator = 0;       // accumulates fractional ticks for sub-1 speed

// Default toggle array alias (matches app.js convention)
const DEFAULT_TOGGLES = SCALE0_TOGGLES;


// ── Internal Helpers ────────────────────────────────────────────────

/**
 * Recompute the cached OR of all field toggle flags.
 * Called after any field toggle changes to avoid per-frame OR chains.
 */
function _recomputeAnyFieldActive() {
    _anyFieldActive = _showEField || _showBField || _showPoynting ||
        _showDivField || _showFluxLines || _showForceVolume ||
        _showDualSubstrate || _showChirality || _showLight ||
        _showGravityField || _showDarkMatterHalo || _showDampingZones ||
        _showGenesisIsosurface;
}

/**
 * Format large numbers with K/M suffixes for the status bar.
 */
function formatNumber(n) {
    if (n >= 1000000) return (n / 1000000).toFixed(1) + 'M';
    if (n >= 1000) return (n / 1000).toFixed(1) + 'K';
    return n.toString();
}


// ═══════════════════════════════════════════════════════════════════
//  PUBLIC API
// ═══════════════════════════════════════════════════════════════════

/**
 * animateLattice — Scale 0 frame loop.
 *
 * Called once per requestAnimationFrame when the engine is in lattice mode.
 * Handles: tick advancement, particle/flux GPU upload, field visualization
 * updates (E/B/Poynting/divergence/flux/force/gravity/dark matter/damping/
 * genesis/dual substrate/chirality/light), viewport rendering, diagnostics
 * collection, and chart/panel updates.
 *
 * @param {object} ctx - Application context (see module header for shape)
 */
export function animateLattice(ctx) {
    const { bridge, viewport } = ctx;
    const L = bridge.latticeSize || 32;

    // ── Tick simulation if running ──────────────────────────────────
    // For large lattices (L>48), throttle to avoid frame drops in JS MockBridge.
    // The wave equation is O(L^3 * 18) per tick — at L=64 that's ~14M ops.
    if (ctx.running) {
        _tickAccumulator += ctx.ticksPerFrame;
        const wholeTicks = Math.floor(_tickAccumulator);
        _tickAccumulator -= wholeTicks;

        // Throttle: large lattices get fewer ticks per frame
        const maxTicksPerFrame = L > 48 ? 1 : (L > 32 ? 2 : wholeTicks);
        const ticksToRun = Math.min(wholeTicks, maxTicksPerFrame);

        for (let i = 0; i < ticksToRun; i++) {
            bridge.tick();
            if (_fluxMock) _fluxMock.tick();
        }
        _latticeNeedsUpload = true;
    }

    // ── GPU buffer upload (only when data changed) ──────────────────
    // For large lattices, throttle volume updates to every Nth frame
    const volUpdateInterval = L > 48 ? 3 : 1;
    if (_latticeNeedsUpload && (ctx.frameCount % volUpdateInterval === 0)) {
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
        const Lvol = bridge.latticeSize || 32;
        if (viewport.showFlux) {
            // Try WASM first, fall back to MockBridge JS wave sim
            let vol = bridge.getFluxVolume();
            if ((!vol || vol.length === 0) && _fluxMock) {
                vol = _fluxMock.getFluxVolume();
            }
            if (vol && vol.length > 0) {
                viewport.updateFluxVolume(vol, Lvol);
            }
        }
        if (viewport.showHeatmap) {
            const sliceIdx = Math.floor(Lvol / 2);
            let slice = bridge.getFluxSlice(1, sliceIdx);
            if ((!slice || slice.length === 0) && _fluxMock) {
                slice = _fluxMock.getFluxSlice(1, sliceIdx);
            }
            if (slice && slice.length > 0) {
                viewport.updateFluxSlice(slice, Lvol, 1, sliceIdx);
            }
        }

        _latticeNeedsUpload = false;
    }

    // ── Field visualization updates (independent of lattice upload) ──
    // Runs every frame but self-throttles to every 3rd frame for perf.
    // _fieldNeedsUpdate bypasses throttle for immediate response on toggle.
    _fieldFrame++;

    const fieldThrottle = L > 48 ? 6 : 3;
    if (_anyFieldActive && (_fieldNeedsUpdate || _fieldFrame % fieldThrottle === 0)) {
        _fieldNeedsUpdate = false;
        const fieldBridge = _fluxMock || bridge;
        const stride = L > 48 ? 6 : L > 32 ? 4 : 2;

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

        // Light field (|Poynting| glow -- reuses Poynting data if already fetched)
        if (_showLight) {
            const sData = fieldBridge.getPoyntingSampled(stride);
            if (sData.count > 0) viewport.updateLightField(sData);
        }
    }

    viewport.render();

    // ── Diagnostics (throttled to every 3rd frame for perf) ─────────
    if (ctx.frameCount % 3 === 0) {
        // Primary diagnostics from the WASM bridge (authoritative for particles,
        // energy, tick count). Fall back to MockBridge only when WASM has no
        // manifested particles AND the mock has flux data (JS-only wave demos).
        const wasmDiag = bridge.getDiagnostics();
        const mockDiag = _fluxMock ? _fluxMock.getDiagnostics() : null;
        const diag = (mockDiag && !wasmDiag.manifested && mockDiag.totalFlux > 0)
            ? { ...mockDiag, tick: wasmDiag.tick }
            : wasmDiag;

        // Update status bar
        ctx.dom.statusTick.textContent = formatNumber(diag.tick);
        if (diag.physicalTime !== undefined) {
            ctx.dom.statusPtime.textContent = formatNumber(Math.round(diag.physicalTime));
        } else {
            ctx.dom.statusPtime.textContent = formatNumber(diag.tick);
        }
        // Scale 0 shows flux stats; manifested count should be 0 for flux-only scenarios
        ctx.dom.statusParticles.textContent = diag.manifested || 0;
        ctx.dom.statusEnergy.textContent = formatEnergy(diag.totalEnergy, 0).text;

        // Update status dot
        if (ctx.running) {
            ctx.dom.statusDot.classList.remove('idle');
            ctx.dom.statusState.textContent = 'Running';
        } else {
            ctx.dom.statusDot.classList.add('idle');
            ctx.dom.statusState.textContent = 'Idle';
        }

        // Always accumulate data for all panels
        ctx.diagnostics.update(diag);
        ctx.fluxEnergyChart.push(diag);
        ctx.particleChart.push(diag);
        // Push to additional charts
        if (ctx.chartCharge) ctx.chartCharge.push(diag.chargeBalance || 0);
        if (ctx.chartEntropy) ctx.chartEntropy.push(diag.entropy || 0);

        const lag = _fluxMock ? _fluxMock.getLagrangian() : bridge.getLagrangian();
        ctx.lagrangianChart.push(lag);

        // Update active panel visuals
        switch (ctx.activeTab) {
            case 'diagnostics':
                ctx.diagnostics.drawSparklines();
                if (ctx.peTelemetry) ctx.peTelemetry.drawCharts();
                const ea = _fluxMock ? _fluxMock.getEnergyAudit() : bridge.getEnergyAudit();
                ctx.diagnostics.updateEnergyAudit(ea);
                break;
            case 'charts': {
                ctx.fluxEnergyChart.draw();
                ctx.particleChart.draw();
                const eaC = _fluxMock ? _fluxMock.getEnergyAudit() : bridge.getEnergyAudit();
                if (eaC) {
                    if (ctx.chartEBEnergy) { ctx.chartEBEnergy.push((eaC.EFieldEnergy || eaC.eFieldEnergy || 0) - (eaC.BFieldEnergy || eaC.bFieldEnergy || 0)); ctx.chartEBEnergy.draw('#a78bfa'); }
                    if (ctx.chartGauss) { ctx.chartGauss.push(eaC.gaussViolation || 0); ctx.chartGauss.draw('#fbbf24'); }
                }
                if (ctx.chartCharge) ctx.chartCharge.draw('#4ade80');
                if (ctx.chartEntropy) ctx.chartEntropy.draw('#60a5fa');
                break;
            }
            case 'lagrangian':
                ctx.lagrangianChart.draw();
                break;
            case 'inspector':
                ctx.inspector.update();
                break;
            case 'ontic':
                ctx.updateOnticPanel();
                break;
            case 'hierarchy':
                ctx.updateHierarchyPanel();
                break;
        }
    }
}


/**
 * loadScenario — Load a Scale 0 scenario by name.
 *
 * Resets visual state, configures the bridge and MockBridge for the
 * named scenario, applies default toggle states then scenario-specific
 * overrides, syncs toggle UI, and marks overridden toggles visually.
 *
 * @param {object} ctx - Application context
 * @param {string} name - Scenario identifier (e.g. 'flux-dipole', 'light-plane')
 */
export function loadScenario(ctx, name) {
    const { bridge, viewport } = ctx;

    resetScale0(ctx);
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
    _syncComboSliders(bridge);

    _latticeNeedsUpload = true;
}


/**
 * resetScale0 — Reset all Scale 0 visual and simulation state.
 *
 * Called on scenario load and on scale switch. Clears field overlays,
 * resets toggle buttons, hides viewport field layers, and clears caches.
 *
 * @param {object} ctx - Application context
 */
export function resetScale0(ctx) {
    const { viewport } = ctx;

    // Clear simulation data caches
    _fieldGrid = null;
    _fieldNeedsUpdate = true;
    _latticeNeedsUpload = true;

    // Reset flux slice OFF, keep flux volume and grid ON (defaults)
    if (viewport) {
        viewport.toggleFluxVolume(true);  // default ON
        viewport.toggleFluxSlice(false);
    }
    const fvBtn = document.getElementById('toggle-flux-volume');
    if (fvBtn) fvBtn.classList.add('active');
    const fsBtn = document.getElementById('toggle-flux-slice');
    if (fsBtn) fsBtn.classList.remove('active');

    // Reset all field visualization flags
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

    // Reset tick accumulator
    _tickAccumulator = 0;
}


/**
 * getFieldState — Read-only access to the current field visualization flags.
 *
 * Returns a snapshot object (not a live reference). Used by UI wiring code
 * in app.js to check which overlays are active.
 *
 * @returns {object} Current field toggle states and the fluxMock reference
 */
export function getFieldState() {
    return {
        showEField: _showEField,
        showBField: _showBField,
        showPoynting: _showPoynting,
        showDivField: _showDivField,
        showFluxLines: _showFluxLines,
        showForceVolume: _showForceVolume,
        showDualSubstrate: _showDualSubstrate,
        showChirality: _showChirality,
        showLight: _showLight,
        showGravityField: _showGravityField,
        showDarkMatterHalo: _showDarkMatterHalo,
        showDampingZones: _showDampingZones,
        showGenesisIsosurface: _showGenesisIsosurface,
        anyFieldActive: _anyFieldActive,
        fieldNeedsUpdate: _fieldNeedsUpdate,
        fluxMock: _fluxMock,
        fieldGrid: _fieldGrid,
    };
}


/**
 * setFieldToggle — Set a single field visualization flag by key.
 *
 * Used by UI event handlers to toggle individual overlays.
 * Automatically recomputes the _anyFieldActive cache.
 *
 * @param {string} key - Field name (e.g. 'showEField', 'showBField')
 * @param {boolean} value - New state
 */
export function setFieldToggle(key, value) {
    switch (key) {
        case 'showEField': _showEField = value; break;
        case 'showBField': _showBField = value; break;
        case 'showPoynting': _showPoynting = value; break;
        case 'showDivField': _showDivField = value; break;
        case 'showFluxLines': _showFluxLines = value; break;
        case 'showForceVolume': _showForceVolume = value; break;
        case 'showDualSubstrate': _showDualSubstrate = value; break;
        case 'showChirality': _showChirality = value; break;
        case 'showLight': _showLight = value; break;
        case 'showGravityField': _showGravityField = value; break;
        case 'showDarkMatterHalo': _showDarkMatterHalo = value; break;
        case 'showDampingZones': _showDampingZones = value; break;
        case 'showGenesisIsosurface': _showGenesisIsosurface = value; break;
    }
    _recomputeAnyFieldActive();
    if (value) _fieldNeedsUpdate = true;
}


/**
 * setLatticeNeedsUpload — Flag that lattice GPU buffers need refresh.
 * Called by app.js when simulation steps or resumes.
 */
export function setLatticeNeedsUpload() {
    _latticeNeedsUpload = true;
}


/**
 * getFluxMock — Access the current MockBridge instance.
 * Needed by app.js for toggle sync and boundary changes.
 * @returns {MockBridge|null}
 */
export function getFluxMock() {
    return _fluxMock;
}


// ── Private Helpers ─────────────────────────────────────────────────

/**
 * Mark toggle rows that differ from defaults with a visual indicator.
 * Opens the advanced section if any advanced toggle was overridden.
 */
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

/**
 * Resync combo panel slider values to current bridge parameters.
 * Called after scenario load resets parameters to defaults.
 */
function _syncComboSliders(bridge) {
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
