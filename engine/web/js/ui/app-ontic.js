/**
 * Ontic Observatory / Physics / Hierarchy panel glue.
 *
 * Extracted from `app.js` as Wave 2 ticket 7 of the large-file refactor
 * (see engine/web/docs/INDEX.md). This is a move, not a rewrite —
 * function bodies preserved verbatim; the only structural change is that
 * accesses to `app`-scope mutable state (bridge, engineMode, observatory,
 * aggregateDetector, emergenceMonitor, _physicsZ) go through a getter bag.
 *
 * DEPS CONTRACT — caller must pass a bag providing live access to the
 * mutable variables that live in app.js module scope:
 *   {
 *     getBridge():            Bridge,              // simulation bridge (mock or wasm)
 *     getEngineMode():        'cosmic' | 'particles' | 'atoms' | 'molecules' | 'lattice' | ...
 *     getObservatory():       OnticObservatory | null,
 *     getAggregateDetector(): AggregateDetector | null,
 *     getEmergenceMonitor():  EmergenceMonitor | null,
 *     getPhysicsZ():          number,
 *     setPhysicsZ(z):         void,
 *   }
 */

import { renderEnergyLevels } from '../spectroscopy.js';
import { renderCrossSections } from '../cross-sections.js';
import { renderDecayRates } from '../decay-rates.js';
import {
    renderFcCard, renderObserverCard, renderHierarchyTower as renderOnticHierarchy,
    renderInfoDynamics
} from '../ontic-observatory.js';
import {
    ONTIC_LAYERS, ONTIC_TOTAL_CONSTANTS,
    ALPHA, K_B, G_STAR, VARPI, X_PLUS, X_MINUS,
} from '../constants.js';

/**
 * Build the ontic-panel provider bound to the given deps.
 * @returns { populateConstants, initOnticPhysicsHierarchy,
 *            renderOnticChainSummary, updateOnticPanel,
 *            getOnticDiagnostics, getRawDiagnostics }
 */
export function createOnticPanel(deps) {
    const {
        getBridge, getEngineMode,
        getObservatory,
        getPhysicsZ, setPhysicsZ,
    } = deps;

    // ── Constants Table ─────────────────────────────────────────────────
    function populateConstants() {
        const bridge = getBridge();
        const c = bridge?.getConstants?.();
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

    // ── Phase 1-3: Ontic / Physics / Hierarchy Initialization ────────
    function initOnticPhysicsHierarchy() {
        // Phase 2: Render static physics content
        const energyEl = document.getElementById('physics-energy-levels');
        if (energyEl) renderEnergyLevels(getPhysicsZ(), energyEl);

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
                const z = parseInt(zSlider.value);
                setPhysicsZ(z);
                if (zValue) zValue.textContent = `Z=${z}`;
                if (energyEl) renderEnergyLevels(z, energyEl);
            });
        }

        // Initial render of ontic panels
        updateOnticPanel();
    }

    function renderOnticChainSummary(container) {
        let rows = '';
        const constants = [
            ['G*', G_STAR.toFixed(10), 'Universal render bridge'],
            ['ϖ', VARPI.toFixed(10), 'Lemniscate constant'],
            ['1/α', X_PLUS.toFixed(7), 'Fine structure inverse'],
            ['x₋', X_MINUS.toFixed(7), 'Smaller master-quadratic root; identification ↔ N_c RETIRED (Cleanup Taxonomy v1.4 §5)'],
            ['α', ALPHA.toFixed(10), 'Fine structure constant'],
            ['K_B', K_B + ' MeV', 'Electron mass / threshold'],
        ];
        for (const [sym, val, _desc] of constants) {
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
        const observatory = getObservatory();
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

    /**
     * Get raw bridge diagnostics for the current engine mode.
     * Used by OnticObservatory.update(diag, scale, tick).
     */
    function getRawDiagnostics() {
        const bridge = getBridge();
        const engineMode = getEngineMode();
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
        const bridge = getBridge();
        const engineMode = getEngineMode();
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

    /**
     * Re-render the physics energy-levels card (called on tab activation to
     * refresh for the current Z selection). The other physics cards
     * (cross sections, decay rates, ontic chain summary) are static after
     * initial render so they're not re-computed here.
     */
    function refreshPhysicsPanel() {
        const energyEl = document.getElementById('physics-energy-levels');
        if (energyEl) renderEnergyLevels(getPhysicsZ(), energyEl);
    }

    return {
        populateConstants,
        initOnticPhysicsHierarchy,
        renderOnticChainSummary,
        updateOnticPanel,
        getOnticDiagnostics,
        getRawDiagnostics,
        refreshPhysicsPanel,
    };
}
