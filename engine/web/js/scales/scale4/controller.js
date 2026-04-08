/**
 * Scale 4 — Consciousness Controller
 *
 * Manages the consciousness scale: holographic visualization driven by
 * lattice flux data, Mandelbrot boundary-orbit tracking, and pedagogy
 * panels (theory walkthrough, scenario descriptions).
 *
 * Physics preserved exactly from app.js:
 *   - Flux ratio:      peak |J| / K_C
 *   - Effective theta:  object (0.7*theta_c) / subject (1.3*theta_c) / dynamic
 *   - Domain class:     Real (k=16) / Degenerate / Complex (k=1/2)
 *   - y_eff scaling:    Y_REAL, Y_IMAG scaled by flux ratio
 *   - Consciousness I:  |y_eff| - K_C  (positive = complex domain)
 *   - Mandelbrot iter:  z -> z^2 + c,  c = 1/G* + perturbation
 */

import { ConsciousnessEngine } from '../../consciousness.js?v=20260305e';
import { ConsciousnessPedagogy, addInfoTooltips } from '../../consciousness-pedagogy.js?v=20260317a';
import { CS_SCENARIO_DESCRIPTIONS } from '../../config/scenarios.js';
import { MockBridge } from '../../wasm-bridge.js?v=20260318a';
import {
    K_C, Y_REAL, Y_IMAG, THETA_C_DEG, C_MANDELBROT, K_B
} from '../../constants.js?v=20260305e';

// ---------------------------------------------------------------------------
// Module-level state (one instance per session, mirrors app.js globals)
// ---------------------------------------------------------------------------

let _csEngine = null;          // ConsciousnessEngine (hologram + audio)
let _csPedagogy = null;        // ConsciousnessPedagogy (theory + walkthrough panels)
let _csScenarioMeta = {        // Scenario metadata for diagnostics
    name: '', domain: 'Real (k=16)', thetaMode: 'static', sloopDepth: 0, bellS: null
};

// Mandelbrot boundary-orbit state (z -> z^2 + c iteration)
let _mandelbrotZ_re = 0;
let _mandelbrotZ_im = 0;
let _mandelbrotIter = 0;

// Scenario descriptions lookup (from config/scenarios.js)
const SCENARIO_DESCRIPTIONS = CS_SCENARIO_DESCRIPTIONS;

// ---------------------------------------------------------------------------
// wireConsciousnessSubTabs  -- DOM wiring for Theory / Walkthrough / Scenarios
// ---------------------------------------------------------------------------

/**
 * Wire the sub-tab bar inside the Scale 4 sidebar panel.
 * Handles: Theory <-> Walkthrough <-> Scenarios tab switching,
 * scenario description updates, walkthrough prev/next navigation.
 */
export function wireConsciousnessSubTabs() {
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

// ---------------------------------------------------------------------------
// animateConsciousness  -- per-frame update (called from the main rAF loop)
// ---------------------------------------------------------------------------

/**
 * Per-frame consciousness animation. Ticks the lattice, extracts flux
 * diagnostics, drives the hologram engine, and updates DOM readouts.
 *
 * @param {object} ctx - Shared context from the main app:
 *   { bridge, viewport, running, ticksPerFrame, _tickAccumulator }
 */
export function animateConsciousness(ctx) {
    const { viewport, running } = ctx;
    let bridge = ctx.bridge;

    // Tick the underlying lattice for flux data
    if (running && bridge) {
        ctx._tickAccumulator += ctx.ticksPerFrame;
        const wholeTicks = Math.floor(ctx._tickAccumulator);
        ctx._tickAccumulator -= wholeTicks;
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
        // Normalize energies (rough -- scale to 0-1 range)
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

        // -- Dynamic Consciousness Diagnostics --
        // (computed before engine update so audio can use them)

        // 1. Flux ratio: peak flux / K_C
        const peakFlux = Math.sqrt(fieldE + waveE); // proxy for peak |J|
        const fluxRatio = K_C > 0 ? peakFlux / K_C : 0;

        // 2. Effective theta: object-dominant (low theta) vs subject-dominant (high theta)
        let effTheta;
        if (_csScenarioMeta.thetaMode === 'object') {
            effTheta = THETA_C_DEG * 0.7; // flow state -- below critical angle
        } else if (_csScenarioMeta.thetaMode === 'subject') {
            effTheta = THETA_C_DEG * 1.3; // meditation -- above critical angle
        } else if (_csScenarioMeta.thetaMode === 'dynamic') {
            // Dynamic: high wave energy -> lower theta (object), high field energy -> higher theta (subject)
            const totalE = fieldE + waveE + 0.001;
            const fieldFrac = fieldE / totalE;
            effTheta = THETA_C_DEG * (0.5 + fieldFrac); // range ~26 deg -- 52 deg+
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
            domainLabel = 'Complex (k=\u00BD)';
        }
        // Override with scenario meta for boundary-orbit
        if (_csScenarioMeta.name === 'cs-threshold') {
            domainLabel = fluxRatio >= 1.0 ? 'Complex (k=\u00BD)' : fluxRatio >= 0.5 ? 'Degenerate' : 'Real (k=16)';
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
            // One z -> z^2 + c iteration per frame, c = C_MANDELBROT + tiny flux perturbation
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

        // -- Update Consciousness Engine (visual + audio) --

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
            mandelbrotZ: Math.sqrt(_mandelbrotZ_re ** 2 + _mandelbrotZ_im ** 2),
        });

        // Update pedagogy panels with live engine data
        if (_csPedagogy) {
            _csPedagogy.update({ fluxRatio, effTheta, consciousnessI });
        }

        // -- DOM Updates --

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

// ---------------------------------------------------------------------------
// loadConsciousnessScenario  -- set up a named consciousness scenario
// ---------------------------------------------------------------------------

/**
 * Initialize and load a consciousness scenario by name.
 * Creates the ConsciousnessEngine and MockBridge if needed, resets Mandelbrot
 * state, configures toggle overrides, and injects scenario-specific flux.
 *
 * @param {object} ctx - Shared context: { bridge, viewport, _resetAllVisualState }
 * @param {string} name - Scenario key (e.g. 'cs-threshold', 'cs-boundary-orbit')
 */
export function loadConsciousnessScenario(ctx, name) {
    ctx._resetAllVisualState();

    const viewport = ctx.viewport;

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
    let bridge = ctx.bridge;
    if (!bridge || !(bridge instanceof MockBridge)) {
        bridge = new MockBridge(32);
        ctx.bridge = bridge;
    }

    // Reset Mandelbrot iteration state
    _mandelbrotZ_re = 0; _mandelbrotZ_im = 0; _mandelbrotIter = 0;

    // Base toggles: flux-only mode (no particles, no forces -- just waves)
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
            // Start below K_C with low-amplitude Gaussian, gradually build to cross real->complex boundary
            const csMid = Math.floor((bridge.latticeSize || 32) / 2);
            const csSubAmp = K_B * 0.3;
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
            _csScenarioMeta = { name, domain: 'Complex (k=\u00BD)', thetaMode: 'dynamic', sloopDepth: 0, bellS: null };
            break;
        }
        case 'cs-self-ref': {
            // Standing wave = observer meeting itself (sLoop depth 1)
            bridge.setupScenario('flux-standing');
            _csScenarioMeta = { name, domain: 'Complex (k=\u00BD)', thetaMode: 'static', sloopDepth: 1, bellS: null };
            break;
        }
        case 'cs-nested-sloop': {
            // Two orthogonal standing waves = self-aware of self-awareness (sLoop depth 2)
            bridge.setupScenario('flux-nested-standing');
            _csScenarioMeta = { name, domain: 'Complex (k=\u00BD)', thetaMode: 'static', sloopDepth: 2, bellS: null };
            break;
        }
        case 'cs-chirality': {
            // Dual substrate with asymmetric L/R injection
            bridge.setToggle('dual_substrate', true);
            bridge.setupScenario('flux-dual-substrate');
            _csScenarioMeta = { name, domain: 'Complex (k=\u00BD)', thetaMode: 'dynamic', sloopDepth: 1, bellS: null };
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
            _csScenarioMeta = { name, domain: 'Complex (k=\u00BD)', thetaMode: 'dynamic', sloopDepth: 1, bellS: 2.0 };
            break;
        }
        case 'cs-flow': {
            // Fast vortex pattern, theta < 52.54 (object-dominant flow state)
            bridge.setupScenario('flux-vortex');
            _csScenarioMeta = { name, domain: 'Complex (k=\u00BD)', thetaMode: 'object', sloopDepth: 0, bellS: null };
            break;
        }
        case 'cs-meditation': {
            // Gentle centered pulse, theta > 52.54 (subject-dominant meditation)
            bridge.setupScenario('flux-pulse');
            _csScenarioMeta = { name, domain: 'Complex (k=\u00BD)', thetaMode: 'subject', sloopDepth: 0, bellS: null };
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

    // Wire audio toggle -- pass scenario name so each gets unique sound
    const audioChk = document.getElementById('cs-audio');
    if (audioChk && _csEngine) {
        if (audioChk.checked) _csEngine.enableAudio(name);
        else _csEngine.disableAudio();
    }
}

// ---------------------------------------------------------------------------
// resetScale4  -- tear down consciousness state on scale switch
// ---------------------------------------------------------------------------

/**
 * Clean up Scale 4 state when leaving consciousness mode.
 * Disposes the hologram engine and pedagogy instance so they can be
 * recreated fresh if the user returns to Scale 4.
 *
 * @param {object} ctx - Shared context (unused for now, reserved for future cleanup)
 */
export function resetScale4(ctx) {
    if (_csEngine) {
        if (_csEngine.disableAudio) _csEngine.disableAudio();
        if (_csEngine.dispose) _csEngine.dispose();
        _csEngine = null;
    }
    _csPedagogy = null;
    _csScenarioMeta = { name: '', domain: 'Real (k=16)', thetaMode: 'static', sloopDepth: 0, bellS: null };
    _mandelbrotZ_re = 0;
    _mandelbrotZ_im = 0;
    _mandelbrotIter = 0;
}
