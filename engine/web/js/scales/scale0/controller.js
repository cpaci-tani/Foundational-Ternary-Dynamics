/**
 * Scale 0 (Lattice) Controller
 * ────────────────────────────────────────────────────────────────────
 *
 * Owns the Scale 0 frame loop, scenario loading, and field visualization
 * state. Extracted from app_dag.js to isolate lattice-specific logic into
 * a self-contained module that the main app delegates to.
 *
 * WHY THIS EXISTS:
 *   app_dag.js grew to 4000+ lines with six scale-specific animation loops,
 *   scenario loaders, and visualization state all tangled together.
 *   Extracting each scale's controller lets us reason about (and test)
 *   each scale independently, and makes the main app a thin dispatcher.
 *
 * CONTEXT OBJECT (ctx):
 *   The caller passes a ctx object containing shared application state
 *   that this controller reads/writes. This avoids coupling to app_dag.js
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
 *     resetAllVisualState - function, master visual state reset from app_dag
 */

import { MockBridge } from '../../wasm-bridge-dag.js';
import { computeStreamlines, generateEFieldSeeds, generateBFieldSeeds, generateGridSeeds } from '../../fieldlines.js';
import { formatEnergy } from '../../units.js';
import { K_B, G_N, DAMPING, K_GENESIS } from '../../constants.js';
import { SCALE0_TOGGLES, SCALE0_SCENARIO_OVERRIDES, LIGHT_SCENARIO_OVERRIDES } from '../../config/toggles.js';
import { createTickAccumulator, formatSI, throttleBySize } from '../scale-utils.js';

// ── Field Visualization State (Scale 0 internal) ────────────────────
// These flags track which field overlays are active. They are module-private;
// the rest of the app queries them through getFieldState().
let _showEField = false;
let _showBField = false;
let _showPoynting = false;
let _showDivField = false;
let _showFluxLines = false;
let _showForceEM = false;
let _showForceGravity = false;
let _showForceStrong = false;
let _showForceWeak = false;
let _showDualSubstrate = false;
let _showChirality = false;
let _showLight = false;
let _showDarkMatterHalo = false;
let _showDampingZones = false;
let _showGenesisIsosurface = false;
let _showConfinement = false;
let _fieldFrame = 0;            // throttle counter for field updates
let _fieldNeedsUpdate = false;  // force immediate field compute on toggle activation
let _anyFieldActive = false;    // cached OR of all field toggle flags

// ── Simulation Caches (Scale 0 internal) ────────────────────────────
let _fluxMock = null;           // MockBridge for Scale 0 flux visualization fallback
// True when the WASM bridge does NOT provide its own flux data, so the JS
// MockBridge must run a parallel wave simulation. Determined once per scenario
// load by probing bridge.getFluxVolume(). When false, _fluxMock.tick() is
// SKIPPED in the play loop -- this avoids running two full L^3 wave updates
// per tick when WASM is already doing the work (the dominant FPS killer).
let _useFluxMock = false;

/**
 * Return true when the JS MockBridge should supply flux data for rendering.
 *
 * Flux-prefixed scenarios always use the JS mock because the C++ setup_scenario
 * bakes a fixed sigma=3 and integer mid=N/2 — incorrect centering/sizing at
 * any N. The JS mock uses sigma=N/10 and midF=(N-1)/2 which are correct.
 * For all other scenarios we probe bridge.getFluxVolume(): if WASM returns a
 * non-empty volume the mock is redundant (and ticking it burns FPS).
 *
 * @param {object} bridge      - Active simulation bridge (WasmBridge or MockBridge)
 * @param {string} scenarioName - Current scenario identifier
 * @returns {boolean}
 */
function _shouldUseFluxMock(bridge, scenarioName) {
    if (scenarioName.startsWith('flux-')) return true;
    // SM seed scenarios are only implemented in MockBridge.setupScenario;
    // the C++ engine does not know about them, so force the JS flux mock.
    if (scenarioName.startsWith('s0-seed-')) return true;
    if (scenarioName.startsWith('s0-field-')) return true;
    try {
        const probe = bridge.getFluxVolume && bridge.getFluxVolume();
        return !(probe && probe.length > 0);
    } catch (_e) { return true; }
}

let _latticeNeedsUpload = true; // set true on scenario load / step / resume
// Fractional-tick accumulator (sub-1 speed support) — uses shared helper
// from scale-utils.js so every scale controller shares the same semantics.
const _tickAcc = createTickAccumulator();

// ── Reusable buffers for field visualization (avoid per-frame alloc) ─
let _fieldParticleBuf = [];     // reusable {x,y,z} array for E/B field seeds
let _dualLVecs = null;          // dual substrate left-chirality vectors
let _dualRVecs = null;          // dual substrate right-chirality vectors
let _chiralValues = null;       // chirality scalar values
let _weakValues = null;         // weak force scalar values (chirality-based)

// Default toggle array alias (matches app_dag.js convention)
const DEFAULT_TOGGLES = SCALE0_TOGGLES;

// Dual-substrate L/R chirality factor: delta = sqrt((4G*-1)/(4G*)).
// Hoisted to module scope so the per-frame field block doesn't redeclare it.
const DUAL_DELTA = 0.9568;


// ── Internal Helpers ────────────────────────────────────────────────

/**
 * Recompute the cached OR of all field toggle flags.
 * Called after any field toggle changes to avoid per-frame OR chains.
 */
function _recomputeAnyFieldActive() {
    _anyFieldActive = _showEField || _showBField || _showPoynting ||
        _showDivField || _showFluxLines || _showForceEM ||
        _showForceGravity || _showForceStrong || _showForceWeak ||
        _showDualSubstrate || _showChirality || _showLight ||
        _showDarkMatterHalo || _showDampingZones ||
        _showGenesisIsosurface || _showConfinement;
}

// formatNumber helper removed -- use formatSI from scale-utils.js instead
// (it covers K/M/G/T with two-decimal precision).

/**
 * Reuse _fieldParticleBuf to avoid per-frame {x,y,z} object allocation
 * in E/B field seed generation paths.
 */
function _fillFieldParticleBuf(pData) {
    while (_fieldParticleBuf.length < pData.count) _fieldParticleBuf.push({ x: 0, y: 0, z: 0 });
    _fieldParticleBuf.length = pData.count;
    for (let i = 0; i < pData.count; i++) {
        _fieldParticleBuf[i].x = pData.positions[i * 3];
        _fieldParticleBuf[i].y = pData.positions[i * 3 + 1];
        _fieldParticleBuf[i].z = pData.positions[i * 3 + 2];
    }
}


// ═══════════════════════════════════════════════════════════════════
//  PUBLIC API
// ═══════════════════════════════════════════════════════════════════

/**
 * animateLattice -- Scale 0 frame loop.
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
    // Wave equation is O(L^3 x 18) per tick:
    //   L=32: ~1.8M ops, L=64: ~14M, L=96: ~48M, L=128: ~113M
    if (ctx.running) {
        const wholeTicks = _tickAcc.accumulate(ctx.ticksPerFrame);

        // Throttle: large lattices get fewer ticks per frame.
        // Inlined (was throttleBySize) to avoid a per-frame array literal alloc.
        const maxTicksPerFrame = L > 96 ? 1 : (L > 48 ? 1 : (L > 32 ? 2 : wholeTicks));
        const ticksToRun = Math.min(wholeTicks, maxTicksPerFrame);

        // PERF: only tick the JS MockBridge when its data is actually consumed.
        // Pre-fix this ran a full L^3 wave update per tick *in addition* to
        // bridge.tick() unconditionally, dominating the play-time CPU budget.
        // Two consumers force the mock to stay live:
        //   (1) WASM has no flux of its own (_useFluxMock true)
        //   (2) Dark-matter-halo / genesis-isosurface overlays read _fluxJ
        //       directly (legacy coupling — see RF-09 in audit)
        const _tickMock = _fluxMock && (_useFluxMock || _showDarkMatterHalo || _showGenesisIsosurface);
        for (let i = 0; i < ticksToRun; i++) {
            bridge.tick();
            if (_tickMock) _fluxMock.tick();
        }
        _latticeNeedsUpload = true;
    }

    // ── GPU buffer upload (only when data changed) ──────────────────
    // Volume upload throttle: L=128 -> every 6th frame, L=96 -> every 4th, L>48 -> every 3rd
    // Inlined (was throttleBySize) to avoid per-frame array literal alloc.
    const volUpdateInterval = L > 96 ? 6 : (L > 64 ? 4 : (L > 48 ? 3 : 1));
    if (_latticeNeedsUpload && (ctx.frameCount % volUpdateInterval === 0)) {
        let particleData = bridge.getParticleData();
        // Fall back to MockBridge particles when main bridge has none (JS-only scenarios)
        if (_useFluxMock && (!particleData || particleData.count === 0)) {
            const mockPD = _fluxMock.getParticleData();
            if (mockPD && mockPD.count > 0) particleData = mockPD;
        }
        viewport.updateParticles(particleData);

        // Confinement strings updates via fieldBridge / MockBridge state evaluation
        if (typeof _showConfinement !== 'undefined' && _showConfinement) {
            viewport.updateConfinementStrings(bridge);
        }

        // Flux volume/slice rendering for Scale 0
        // When _useFluxMock is set (e.g. flux scenarios where WASM sigma/centering
        // is wrong), query the JS mock first; otherwise prefer WASM and fall back.
        if (viewport.showFlux) {
            let vol;
            if (_useFluxMock && _fluxMock) {
                vol = _fluxMock.getFluxVolume();
                if (!vol || vol.length === 0) vol = bridge.getFluxVolume();
            } else {
                vol = bridge.getFluxVolume();
                if ((!vol || vol.length === 0) && _fluxMock) vol = _fluxMock.getFluxVolume();
            }
            if (vol && vol.length > 0) {
                viewport.updateFluxVolume(vol, L);
            }
        }
        if (viewport.showHeatmap) {
            const sliceIdx = Math.floor(L / 2);
            let slice;
            if (_useFluxMock && _fluxMock) {
                slice = _fluxMock.getFluxSlice(1, sliceIdx);
                if (!slice || slice.length === 0) slice = bridge.getFluxSlice(1, sliceIdx);
            } else {
                slice = bridge.getFluxSlice(1, sliceIdx);
                if ((!slice || slice.length === 0) && _fluxMock) slice = _fluxMock.getFluxSlice(1, sliceIdx);
            }
            if (slice && slice.length > 0) {
                viewport.updateFluxSlice(slice, L, 1, sliceIdx);
            }
        }

        _latticeNeedsUpload = false;
    }

    // ── Field visualization updates (independent of lattice upload) ──
    // Runs every frame but self-throttles to every Nth frame for perf.
    // _fieldNeedsUpdate bypasses throttle for immediate response on toggle.
    _fieldFrame++;

    // Field overlay throttle: larger lattices update less frequently
    const fieldThrottle = L > 96 ? 12 : (L > 48 ? 6 : 3);
    if (_anyFieldActive && (_fieldNeedsUpdate || _fieldFrame % fieldThrottle === 0)) {
        _fieldNeedsUpdate = false;
        // fieldBridge is the source for *public* sampled-field getters
        // (getEFieldSampled, getFluxVectorSampled, etc.). Prefer the WASM
        // bridge when it has flux; fall back to the JS mock for JS-only
        // scenarios. mockSource is the source for the legacy *private*
        // _fluxJ peek used by halo/genesis-iso branches -- always the mock.
        const fieldBridge = _useFluxMock ? _fluxMock : bridge;
        const mockSource = _fluxMock;
        // Field sampling stride: L=128->8, L>48->6, L>32->4, else 2
        const stride = L > 96 ? 8 : (L > 48 ? 6 : (L > 32 ? 4 : 2));
        // Scale streamline max steps with lattice size so lines remain
        // visible at L=64/128 (base tuned for L=32).
        const stepsScale = Math.ceil(L / 32);
        // Adaptive seed spacing: clamp to [2, 8] so small lattices get enough seeds
        const seedSpacing = Math.max(2, Math.min(8, Math.floor(L / 4)));

        // PERF: cache sampled-field results across overlay branches.
        // Without these, getFluxVectorSampled() ran up to 3x per frame
        // (flux-lines + dual-substrate + chirality) and getPoyntingSampled()
        // ran up to 2x (poynting + light), each rebuilding the entire
        // sampled grid from scratch. Each fetch is O(N^3 / stride^3).
        let _jDataCache = null;
        const _needFluxVec = _showFluxLines || _showDualSubstrate || _showChirality || _showForceWeak;
        if (_needFluxVec) _jDataCache = fieldBridge.getFluxVectorSampled(stride);
        let _sDataCache = null;
        const _needPoynting = _showPoynting || _showLight;
        if (_needPoynting) _sDataCache = fieldBridge.getPoyntingSampled(stride);

        // E-field streamlines
        if (_showEField) {
            const eData = fieldBridge.getEFieldSampled(stride);
            if (eData.count > 0) {
                const pData = bridge.getParticleData();
                // Reuse _fieldParticleBuf to avoid per-frame {x,y,z} object alloc
                _fillFieldParticleBuf(pData);
                const seeds = pData.count > 0 ? generateEFieldSeeds(_fieldParticleBuf, 2, 120) : generateGridSeeds(L, seedSpacing, 120);
                const lines = computeStreamlines(eData, seeds, { N: L, stride, maxSteps: 80 * stepsScale, stepSize: 0.6 });
                viewport.updateEFieldLines(lines);
            }
        }

        // B-field streamlines
        if (_showBField) {
            const bData = fieldBridge.getBFieldSampled(stride);
            if (bData.count > 0) {
                const pData = bridge.getParticleData();
                // Reuse _fieldParticleBuf to avoid per-frame {x,y,z} object alloc
                _fillFieldParticleBuf(pData);
                const seeds = pData.count > 0 ? generateBFieldSeeds(_fieldParticleBuf, 4, 120) : generateGridSeeds(L, seedSpacing, 120);
                const lines = computeStreamlines(bData, seeds, { N: L, stride, maxSteps: 150 * stepsScale, stepSize: 0.5, bidirectional: false });
                viewport.updateBFieldLines(lines);
            }
        }

        // Poynting vectors (uses cached _sDataCache)
        if (_showPoynting && _sDataCache && _sDataCache.count > 0) {
            viewport.updatePoyntingVectors(_sDataCache);
        }

        // Divergence field
        if (_showDivField) {
            const divData = fieldBridge.getDivJSampled(stride);
            if (divData.count > 0) viewport.updateDivergenceField(divData);
        }

        // Flux streamlines (uses cached _jDataCache)
        if (_showFluxLines && _jDataCache && _jDataCache.count > 0) {
            const seeds = generateGridSeeds(L, seedSpacing, 150);
            const lines = computeStreamlines(_jDataCache, seeds, { N: L, stride, maxSteps: 80 * stepsScale, stepSize: 0.5 });
            let maxFlux = 0;
            for (let i = 0; i < _jDataCache.count; i++) {
                const m = Math.sqrt(_jDataCache.vectors[i * 3] ** 2 + _jDataCache.vectors[i * 3 + 1] ** 2 + _jDataCache.vectors[i * 3 + 2] ** 2);
                if (m > maxFlux) maxFlux = m;
            }
            viewport.updateFluxStreamlines(lines, maxFlux);
        }

        // EM force (cyan arrows — Coulomb from particles)
        if (_showForceEM) {
            const emData = fieldBridge.getEMForceField(stride);
            if (emData.count > 0) viewport.updateEMForceField(emData);
        }

        // Gravity force (amber arrows — density gradient)
        if (_showForceGravity) {
            const gData = fieldBridge.getGravityForceField(stride);
            if (gData.count > 0) viewport.updateGravityForceField(gData);
        }

        // Strong force (red arrows — confinement/color force)
        if (_showForceStrong) {
            const sData = fieldBridge.getStrongForceField(stride);
            if (sData.count > 0) viewport.updateStrongForceField(sData);
        }

        // Weak force (purple points — chirality-based overlay)
        if (_showForceWeak && _jDataCache && _jDataCache.count > 0) {
            const lMinusR = DUAL_DELTA;
            if (!_weakValues || _weakValues.length < _jDataCache.count) {
                _weakValues = new Float32Array(_jDataCache.count);
            }
            for (let i = 0; i < _jDataCache.count; i++) {
                const jx = _jDataCache.vectors[i * 3], jy = _jDataCache.vectors[i * 3 + 1], jz = _jDataCache.vectors[i * 3 + 2];
                const mag = Math.sqrt(jx * jx + jy * jy + jz * jz);
                _weakValues[i] = mag * lMinusR;
            }
            viewport.updateWeakField({ positions: _jDataCache.positions, values: _weakValues, count: _jDataCache.count });
        }

        // Dark matter halo (sub-threshold flux envelope)
        // Reads MockBridge private state directly — leave coupling smell
        // alone for now (RF-09 in audit), but always source from mockSource
        // since the WASM bridge has no _fluxJ getter.
        if (_showDarkMatterHalo && mockSource && mockSource._fluxJ) {
            const N = mockSource.latticeSize;
            if (mockSource._updateFluxMag) mockSource._updateFluxMag();
            const mag = mockSource._fluxMag;
            if (mag) viewport.updateDarkMatterHalo(mockSource._particles, mag, N);
        }

        // Selective damping zones (wireframe cubes around damped voxels)
        if (_showDampingZones && mockSource) {
            viewport.updateDampingZones(mockSource._particles, mockSource.latticeSize);
        }

        // Genesis threshold isosurface (birth boundary) — same coupling caveat
        if (_showGenesisIsosurface && mockSource && mockSource._fluxJ) {
            const N = mockSource.latticeSize;
            if (mockSource._updateFluxMag) mockSource._updateFluxMag();
            const mag = mockSource._fluxMag;
            if (mag) viewport.updateGenesisIsosurface(mag, N, K_GENESIS);
        }

        // Dual substrate (uses cached _jDataCache split into L/R via delta)
        if (_showDualSubstrate && _jDataCache && _jDataCache.count > 0) {
            const lFactor = (1 + DUAL_DELTA) / 2;
            const rFactor = (1 - DUAL_DELTA) / 2;
            const vecLen = _jDataCache.vectors.length;
            if (!_dualLVecs || _dualLVecs.length < vecLen) {
                _dualLVecs = new Float32Array(vecLen);
                _dualRVecs = new Float32Array(vecLen);
            }
            for (let i = 0; i < vecLen; i++) {
                _dualLVecs[i] = _jDataCache.vectors[i] * lFactor;
                _dualRVecs[i] = _jDataCache.vectors[i] * rFactor;
            }
            viewport.updateDualFluxVolume(
                { positions: _jDataCache.positions, vectors: _dualLVecs, count: _jDataCache.count },
                { positions: _jDataCache.positions, vectors: _dualRVecs, count: _jDataCache.count }
            );
        }

        // Chirality (|J_L| - |J_R| as scalar field) — uses cached _jDataCache
        if (_showChirality && _jDataCache && _jDataCache.count > 0) {
            const lMinusR = DUAL_DELTA;  // (1+d)/2 - (1-d)/2 = d
            if (!_chiralValues || _chiralValues.length < _jDataCache.count) {
                _chiralValues = new Float32Array(_jDataCache.count);
            }
            for (let i = 0; i < _jDataCache.count; i++) {
                const jx = _jDataCache.vectors[i * 3], jy = _jDataCache.vectors[i * 3 + 1], jz = _jDataCache.vectors[i * 3 + 2];
                const mag = Math.sqrt(jx * jx + jy * jy + jz * jz);
                _chiralValues[i] = mag * lMinusR;
            }
            viewport.updateChiralityField({ positions: _jDataCache.positions, values: _chiralValues, count: _jDataCache.count });
        }

        // Light field (|Poynting| glow) — uses cached _sDataCache
        if (_showLight && _sDataCache && _sDataCache.count > 0) {
            viewport.updateLightField(_sDataCache);
        }
    }

    viewport.render();

    // ── Diagnostics (throttled to every 3rd frame for perf) ─────────
    if (ctx.frameCount % 3 === 0) {
        // Primary diagnostics from the WASM bridge (authoritative for particles,
        // energy, tick count). Fall back to MockBridge only when WASM has no
        // manifested particles AND the mock has flux data (JS-only wave demos).
        // PERF: only consult mock when WASM doesn't own the flux data, so JS
        // diagnostics aren't re-walked at 20Hz when WASM is authoritative.
        const wasmDiag = bridge.getDiagnostics();
        const mockDiag = _useFluxMock ? _fluxMock.getDiagnostics() : null;
        const diag = (mockDiag && !wasmDiag.manifested && mockDiag.totalFlux > 0)
            ? { ...mockDiag, tick: wasmDiag.tick }
            : wasmDiag;

        // Update status bar
        ctx.dom.statusTick.textContent = formatSI(diag.tick);
        if (diag.physicalTime !== undefined) {
            ctx.dom.statusPtime.textContent = formatSI(Math.round(diag.physicalTime));
        } else {
            ctx.dom.statusPtime.textContent = formatSI(diag.tick);
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

        const lag = _useFluxMock ? _fluxMock.getLagrangian() : bridge.getLagrangian();
        ctx.lagrangianChart.push(lag);

        // Update active panel visuals
        switch (ctx.activeTab) {
            case 'diagnostics':
                ctx.diagnostics.drawSparklines();
                if (ctx.peTelemetry) ctx.peTelemetry.drawCharts();
                const ea = _useFluxMock ? _fluxMock.getEnergyAudit() : bridge.getEnergyAudit();
                ctx.diagnostics.updateEnergyAudit(ea);
                break;
            case 'charts': {
                ctx.fluxEnergyChart.draw();
                ctx.particleChart.draw();
                const eaC = _useFluxMock ? _fluxMock.getEnergyAudit() : bridge.getEnergyAudit();
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
 * loadScenario -- Load a Scale 0 scenario by name.
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

    // Use master visual state reset from app_dag (handles ALL scales)
    ctx.resetAllVisualState();

    // Reset auxiliary settings to defaults (speed, boundary, view toggles).
    // Per-spec: "Changing scenarios should reset ALL values/settings."
    _resetAuxiliarySettings();

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

    _useFluxMock = _shouldUseFluxMock(bridge, name);

    // Mark toggles that differ from defaults after scenario overrides
    _markScenarioOverrides();

    // Resync combo panel sliders to bridge defaults after reset
    _syncComboSliders(bridge);

    _latticeNeedsUpload = true;
}


/**
 * resizeLattice -- Change the lattice size WITHOUT resetting state.
 *
 * The WASM RenderBridge is fixed-size, so resizing requires recreating it.
 * Unlike loadScenario(), this function:
 *   - PRESERVES all toggle states, slider values, charts, play state, camera env
 *   - Re-injects the CURRENT scenario into the new bridge (so the lattice has
 *     a sensible flux pattern at the new size)
 *   - Re-syncs the existing toggle state (read from HTML) to both bridges
 *
 * Per-spec: "Changing size should immediately set the size [for] the flux
 * volume and all overlays and NOT reset the state of the simulation."
 *
 * Note: viewport.setLatticeSize() rebuilds the flux volume geometry and
 * recenters the lattice mode camera, both of which are size-dependent.
 *
 * @param {object} ctx - Application context
 * @param {number} newSize - New lattice side length
 */
export function resizeLattice(ctx, newSize) {
    const { bridge, viewport } = ctx;

    // Capture current scenario name BEFORE we touch anything
    const scenarioEl = document.getElementById('scenario-select');
    const scenarioName = scenarioEl ? scenarioEl.value : 'flux-pulse';

    // ── PRE-FLIGHT MEMORY CHECK ─────────────────────────────────────
    // Voxel struct is 232 bytes. Estimate the new bridge's WASM heap
    // requirement and refuse if it would exceed MAXIMUM_MEMORY (2 GB),
    // which would otherwise abort() the WASM module catastrophically.
    //
    //   bytes ≈ N³ × ~330  (voxels=232 + force_diag=96 + delta_j×3+
    //                       phi×4 + various small buffers, all per-voxel)
    const VOXEL_BYTES_TOTAL = 330;
    const MAX_WASM_MEMORY = 2 * 1024 * 1024 * 1024; // 2 GB ceiling
    const SAFETY_FACTOR = 1.3;  // leave headroom for staging buffers + JS
    const projectedBytes = Math.ceil(newSize ** 3 * VOXEL_BYTES_TOTAL * SAFETY_FACTOR);
    if (projectedBytes >= MAX_WASM_MEMORY) {
        const projGB = (projectedBytes / 1024 / 1024 / 1024).toFixed(2);
        const msg = `L=${newSize} would need ~${projGB} GB of WASM heap (max 2 GB). Refusing to resize.`;
        if (typeof window.showToast === 'function') {
            window.showToast(msg, 'error');
        } else {
            console.warn('[Scale0] ' + msg);
        }
        // Revert the dropdown to the current bridge size
        const sizeEl = document.getElementById('lattice-size');
        if (sizeEl && bridge.latticeSize) sizeEl.value = String(bridge.latticeSize);
        return;
    }

    // Resize the WASM RenderBridge: set the new size on the wrapper and
    // call setupScenario, which internally does ONE reset() at the new
    // size + applies the scenario. (Calling bridge.reset(newSize) AND
    // bridge.setupScenario(name) in sequence triggers a *double* reset
    // because setupScenario itself starts with this.reset() — that doubles
    // the WASM allocation churn and risks OOM under memory pressure.)
    // The simulation tick counter and per-voxel state are necessarily lost
    // because RenderBridge has no in-place resize.
    bridge.latticeSize = newSize;
    bridge.setupScenario(scenarioName);
    viewport.setLatticeSize(newSize);

    // setLatticeSize() removes the old flux volume geometry. Force an immediate
    // rebuild at the new size so it's visible on this frame rather than waiting
    // 1-3 frames for the lazy updateFluxVolume() path to run.
    // toggleFluxVolume preserves the current showFlux state (no visual toggle).
    viewport.toggleFluxVolume(viewport.showFlux);

    // Re-create the JS MockBridge at the new size (per-size instance)
    _fluxMock = new MockBridge(newSize);

    // Sync boundary settings (preserve the user's current choice)
    const boundaryEl = document.getElementById('boundary-select');
    if (boundaryEl) _fluxMock.setBoundaryShape(boundaryEl.value);
    const reflEl = document.getElementById('reflective-boundary');
    if (reflEl) _fluxMock.setReflectiveBoundary(reflEl.checked);
    _fluxMock.setupScenario(scenarioName);

    // CRITICAL: re-sync the EXISTING toggle states (from HTML) to the new
    // bridge. Do NOT reset toggles to defaults — the user may have configured
    // them and resizing should preserve their choices.
    for (const [key, , elId] of DEFAULT_TOGGLES) {
        const el = document.getElementById(elId);
        if (el) {
            bridge.setToggle(key, el.checked);
            _fluxMock.setToggle(key, el.checked);
        }
    }

    _useFluxMock = _shouldUseFluxMock(bridge, scenarioName);

    _latticeNeedsUpload = true;
    _fieldNeedsUpdate = true;  // field overlays should refresh next frame
    _tickAcc.reset();          // tick fractional accumulator restart is harmless

    // Deliberately NOT done here:
    //   - clearCharts()          (charts preserve play history across resize)
    //   - resetAllVisualState()  (no overlay button reset)
    //   - touching ctx.running   (preserve play/pause)
    //   - resetting sliders      (preserve speed/kb/gn/damping)
}


/**
 * resetScale0 -- Reset all Scale 0 field visualization state.
 *
 * Called on scenario load and on scale switch. Clears field overlays,
 * resets toggle buttons, hides viewport field layers, and clears caches.
 *
 * @param {object} ctx - Application context
 */
export function resetScale0(ctx) {
    const { viewport } = ctx;

    // Clear simulation data caches
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
    _showForceEM = false;
    _showForceGravity = false;
    _showForceStrong = false;
    _showForceWeak = false;
    _showDualSubstrate = false;
    _showChirality = false;
    _showLight = false;
    _showDarkMatterHalo = false;
    _showDampingZones = false;
    _showGenesisIsosurface = false;
    _showConfinement = false;
    _fieldNeedsUpdate = false;
    _recomputeAnyFieldActive();

    // Deactivate Scale 0 field toggle buttons
    for (const id of [
        'toggle-e-field', 'toggle-b-field', 'toggle-poynting',
        'toggle-div-field', 'toggle-flux-lines',
        'toggle-force-em', 'toggle-force-gravity', 'toggle-force-strong', 'toggle-force-weak',
        'toggle-dual-substrate', 'toggle-chirality', 'toggle-light',
        'toggle-dark-halo', 'toggle-damping-zones',
        'toggle-genesis-iso', 'toggle-confinement',
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
        viewport.showEMForce(false);
        viewport.showGravityForce(false);
        viewport.showStrongForce(false);
        viewport.showWeakField(false);
        viewport.toggleDualFluxVolume(false);
        viewport.toggleChiralityField(false);
        viewport.toggleLightField(false);
        viewport.toggleDarkMatterHalo(false);
        viewport.toggleDampingZones(false);
        viewport.toggleGenesisIsosurface(false);
        viewport.toggleConfinement(false);
    }

    // Reset tick accumulator
    _tickAcc.reset();
}


/**
 * getFieldState -- Read-only access to the current field visualization flags.
 *
 * Returns a snapshot object (not a live reference). Used by UI wiring code
 * in app_dag.js to check which overlays are active.
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
        showForceEM: _showForceEM,
        showForceGravity: _showForceGravity,
        showForceStrong: _showForceStrong,
        showForceWeak: _showForceWeak,
        showDualSubstrate: _showDualSubstrate,
        showChirality: _showChirality,
        showLight: _showLight,
        showDarkMatterHalo: _showDarkMatterHalo,
        showDampingZones: _showDampingZones,
        showGenesisIsosurface: _showGenesisIsosurface,
        showConfinement: _showConfinement,
        anyFieldActive: _anyFieldActive,
        fieldNeedsUpdate: _fieldNeedsUpdate,
        fluxMock: _fluxMock,
    };
}


/**
 * setFieldToggle -- Set a single field visualization flag by key.
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
        case 'showForceEM': _showForceEM = value; break;
        case 'showForceGravity': _showForceGravity = value; break;
        case 'showForceStrong': _showForceStrong = value; break;
        case 'showForceWeak': _showForceWeak = value; break;
        case 'showDualSubstrate': _showDualSubstrate = value; break;
        case 'showChirality': _showChirality = value; break;
        case 'showLight': _showLight = value; break;
        case 'showDarkMatterHalo': _showDarkMatterHalo = value; break;
        case 'showDampingZones': _showDampingZones = value; break;
        case 'showGenesisIsosurface': _showGenesisIsosurface = value; break;
        case 'showConfinement': _showConfinement = value; break;
    }
    _recomputeAnyFieldActive();
    if (value) _fieldNeedsUpdate = true;
}


/**
 * setLatticeNeedsUpload -- Flag that lattice GPU buffers need refresh.
 * Called by app_dag.js when simulation steps or resumes.
 */
export function setLatticeNeedsUpload() {
    _latticeNeedsUpload = true;
}


/**
 * getFluxMock -- Access the current MockBridge instance.
 * Needed by app_dag.js for toggle sync and boundary changes.
 * @returns {MockBridge|null}
 */
export function getFluxMock() {
    return _fluxMock;
}


/**
 * clearFluxMock -- Clear the MockBridge reference.
 * Called when leaving Scale 0 to free memory.
 */
export function clearFluxMock() {
    _fluxMock = null;
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
 * Reset Scale 0 auxiliary settings to their defaults: speed slider,
 * boundary shape, reflective boundary, flux volume/slice view toggles.
 *
 * Called from loadScenario as part of the "ALL values/settings" reset
 * mandate. The Scale 0 toggles + combo sliders are handled separately
 * by the existing reset paths in loadScenario itself.
 *
 * Each setting is set via .value/.checked AND a 'change'/'input' event
 * is dispatched so that downstream listeners (which actually push the
 * value into the WASM bridge / mock) take effect.
 */
function _resetAuxiliarySettings() {
    // Speed slider — default 50 maps to ticksPerFrame = 1.0 via the
    // piecewise curve in app_dag.js (see _sliderToSpeed).
    const speed = document.getElementById('ticks-per-frame');
    if (speed && speed.value !== '50') {
        speed.value = '50';
        speed.dispatchEvent(new Event('input'));
    }

    // Boundary shape — default 'cube' (no clipping)
    const boundary = document.getElementById('boundary-select');
    if (boundary && boundary.value !== 'cube') {
        boundary.value = 'cube';
        boundary.dispatchEvent(new Event('change'));
    }

    // Reflective boundary — default ON
    const refl = document.getElementById('reflective-boundary');
    if (refl && !refl.checked) {
        refl.checked = true;
        refl.dispatchEvent(new Event('change'));
    }

    // Flux Volume view toggle — default ON
    const fvBtn = document.getElementById('toggle-flux-volume');
    if (fvBtn && !fvBtn.classList.contains('active')) {
        fvBtn.click();  // toggles .active and pushes to viewport
    }

    // Flux Slice view toggle — default OFF
    const fsBtn = document.getElementById('toggle-flux-slice');
    if (fsBtn && fsBtn.classList.contains('active')) {
        fsBtn.click();
    }
}

/**
 * Resync combo panel slider values to current bridge parameters.
 * Called after scenario load resets parameters to defaults.
 */
function _syncComboSliders(bridge) {
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
