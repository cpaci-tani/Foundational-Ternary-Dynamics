/**
 * Scale 11 (Consciousness) Controller
 * ────────────────────────────────────────────────────────────────────
 *
 * Owns the Scale 11 consciousness visualization loop, scenario loading,
 * pedagogy sub-tab wiring, and Mandelbrot boundary-orbit tracking.
 * Extracted from app_dag.js to isolate consciousness-specific logic
 * into a self-contained module.
 *
 * WHY THIS EXISTS:
 *   The consciousness scale had ~340 lines inline in app_dag.js covering
 *   animateConsciousness(), loadConsciousnessScenario(), and
 *   wireConsciousnessSubTabs(). Extracting them follows the pattern
 *   established by Scale 4 (planetary) and Scale 5 (cosmic) controllers.
 *
 * CONTEXT OBJECT (ctx):
 *   The caller passes a ctx object containing shared application state.
 *   Required properties:
 *     bridge           - WASMBridge or MockBridge instance
 *     viewport         - Viewport (Three.js) renderer
 *     running          - boolean, true when simulation is playing
 *     ticksPerFrame    - number, simulation ticks per render frame
 *     engineMode       - string, current scale mode
 *     MockBridge       - MockBridge constructor (for creating flux-only bridge)
 *     _resetAllVisualState - function, clears all visual overlays
 *     addInfoTooltips  - function, wires info tooltip icons
 *
 * BRIDGE SAVE/RESTORE:
 *   Consciousness mode replaces the real bridge with a flux-only MockBridge.
 *   The original bridge is saved in _savedBridge and restored in resetScale11()
 *   when leaving consciousness mode. This is critical -- without it, returning
 *   to other scales would leave a dead bridge reference.
 */

import { ConsciousnessEngine } from '../../consciousness.js';
import { ConsciousnessPedagogy, addInfoTooltips } from '../../consciousness-pedagogy.js';
import {
    K_B, K_C, Y_REAL, Y_IMAG, THETA_C_DEG, C_MANDELBROT
} from '../../constants.js';
import { CS_SCENARIO_DESCRIPTIONS } from '../../config/scenarios.js';
import { createListenerBag, createTickAccumulator } from '../scale-utils.js';

// ── Module State ────────────────────────────────────────────────────

let _csEngine    = null;   // ConsciousnessEngine instance
let _csPedagogy  = null;   // ConsciousnessPedagogy instance (Theory/Walkthrough panels)
let _savedBridge = null;   // Holds the real bridge while consciousness uses MockBridge

// Scenario metadata (domain, theta mode, sLoop depth, Bell S-value)
let _csScenarioMeta = {
    name: '', domain: 'Real (k=16)', thetaMode: 'static', sloopDepth: 0, bellS: null
};

// Mandelbrot boundary-orbit iteration state
let _mandelbrotZ_re = 0;
let _mandelbrotZ_im = 0;
let _mandelbrotIter = 0;

// Tick accumulator for fractional-tick support (shared helper)
const _tickAcc = createTickAccumulator();

// DOM listener bag -- cleared on resetScale11 so re-entering consciousness
// mode doesn't leak click/change handlers on .cs-subtab / #cs-* controls.
let _subTabListeners = createListenerBag();

// Scenario descriptions alias
const SCENARIO_DESCRIPTIONS = CS_SCENARIO_DESCRIPTIONS;

// ── animateConsciousness ────────────────────────────────────────────

/**
 * Per-frame consciousness animation. Ticks the underlying lattice for
 * flux data, computes consciousness diagnostics (flux ratio, effective
 * theta, domain classification, Mandelbrot orbit), updates the
 * ConsciousnessEngine visuals and audio, and refreshes DOM elements.
 *
 * Called from the main rAF loop when engineMode === 'consciousness'.
 *
 * @param {object} ctx - Shared context from the main app
 * @param {number} now - Current timestamp from requestAnimationFrame
 */
export function animateConsciousness(ctx, now) {
    const { viewport, running, ticksPerFrame } = ctx;
    const bridge = ctx.bridge;

    // Tick the underlying lattice for flux data
    if (running && bridge) {
        const wholeTicks = _tickAcc.accumulate(ticksPerFrame);
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
        const waveE  = audit.waveEnergy  || 0;

        // Normalize energies (rough -- scale to 0-1 range)
        const maxE  = Math.max(fieldE, 1);
        const nFlux = Math.min(fieldE / maxE, 1);
        const nWave = Math.min(waveE / Math.max(waveE, 1), 1);

        // Compute variance from diagnostic if available
        let variance       = 0.1;
        let curlMag        = 0.05;
        let centralDensity = 0.3;
        let polarity       = 0;

        // Use energy audit fields if present
        if (audit.chargeTotal !== undefined) {
            polarity = Math.max(-1, Math.min(1, audit.chargeTotal / 10));
        }

        // Estimate variance from difference between max and mean
        if (fieldE > 0) {
            variance       = Math.min(1, fieldE / 50);
            curlMag        = Math.min(1, waveE  / 20);
            centralDensity = Math.min(1, fieldE / 30);
        }

        // ── Dynamic Consciousness Diagnostics ──────────────────────

        // 1. Flux ratio: peak flux / K_C
        const peakFlux  = Math.sqrt(fieldE + waveE);  // proxy for peak |J|
        const fluxRatio = K_C > 0 ? peakFlux / K_C : 0;

        // 2. Effective theta: object-dominant (low theta) vs subject-dominant (high theta)
        let effTheta;
        if (_csScenarioMeta.thetaMode === 'object') {
            effTheta = THETA_C_DEG * 0.7;   // flow state -- below critical angle
        } else if (_csScenarioMeta.thetaMode === 'subject') {
            effTheta = THETA_C_DEG * 1.3;   // meditation -- above critical angle
        } else if (_csScenarioMeta.thetaMode === 'dynamic') {
            // Dynamic: high wave energy -> lower theta (object),
            //          high field energy -> higher theta (subject)
            const totalE    = fieldE + waveE + 0.001;
            const fieldFrac = fieldE / totalE;
            effTheta = THETA_C_DEG * (0.5 + fieldFrac);  // range ~26 deg - 52+ deg
        } else {
            effTheta = THETA_C_DEG;  // static: exactly the theory value
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
            domainLabel = fluxRatio >= 1.0
                ? 'Complex (k=\u00BD)'
                : fluxRatio >= 0.5
                    ? 'Degenerate'
                    : 'Real (k=16)';
        }

        // 4. Effective y: scale Y_REAL and Y_IMAG by flux ratio
        const yScale   = Math.min(fluxRatio, 2.0);
        const yRealEff = Y_REAL * yScale;
        const yImagEff = Y_IMAG * yScale;
        const yMag     = Math.sqrt(yRealEff * yRealEff + yImagEff * yImagEff);

        // 5. Consciousness intensity: |y_eff| - K_C
        const consciousnessI = yMag - K_C;

        // 6. Mandelbrot iteration (boundary-orbit scenario)
        let mandelbrotDisplay = `c=${C_MANDELBROT.toFixed(3)}`;
        if (_csScenarioMeta.name === 'cs-boundary-orbit') {
            // One z -> z^2 + c iteration per frame, c = C_MANDELBROT + tiny flux perturbation
            const c_re  = C_MANDELBROT + (fluxRatio - 1.0) * 0.001;
            const c_im  = 0;
            const new_re = _mandelbrotZ_re * _mandelbrotZ_re
                         - _mandelbrotZ_im * _mandelbrotZ_im + c_re;
            const new_im = 2 * _mandelbrotZ_re * _mandelbrotZ_im + c_im;
            _mandelbrotZ_re = new_re;
            _mandelbrotZ_im = new_im;
            _mandelbrotIter++;
            const zMag = Math.sqrt(
                _mandelbrotZ_re * _mandelbrotZ_re + _mandelbrotZ_im * _mandelbrotZ_im
            );
            // Reset if escaped (|z| > 2)
            if (zMag > 2) {
                _mandelbrotZ_re  = 0;
                _mandelbrotZ_im  = 0;
                _mandelbrotIter  = 0;
            }
            mandelbrotDisplay = `|z|=${zMag.toFixed(3)}`;
        }

        // ── Update Consciousness Engine (visual + audio) ───────────

        _csEngine.update({
            fluxEnergy:     nFlux,
            waveEnergy:     nWave,
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

        // ── DOM Updates ────────────────────────────────────────────

        // Row 2: Dynamic measurements
        const effThetaEl = document.getElementById('cs-diag-eff-theta');
        if (effThetaEl) {
            effThetaEl.textContent = `${effTheta.toFixed(1)}\u00B0`;
            effThetaEl.style.color = effTheta < THETA_C_DEG
                ? 'var(--accent)' : 'var(--consciousness-primary)';
        }

        const fluxRatioEl = document.getElementById('cs-diag-flux-ratio');
        if (fluxRatioEl) {
            fluxRatioEl.textContent = fluxRatio.toFixed(3);
            fluxRatioEl.style.color = fluxRatio >= 1.0
                ? 'var(--consciousness-primary)' : 'var(--text-muted)';
        }

        const domainEl = document.getElementById('cs-diag-domain');
        if (domainEl) {
            domainEl.textContent = domainLabel;
            domainEl.style.color = domainLabel.includes('Complex')
                ? 'var(--consciousness-primary)'
                : domainLabel.includes('Degenerate')
                    ? 'var(--warning)' : 'var(--text-muted)';
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

// ── loadConsciousnessScenario ───────────────────────────────────────

/**
 * Initialize and load a consciousness scenario by name.
 *
 * Creates the ConsciousnessEngine and ConsciousnessPedagogy on first call.
 * Replaces the real bridge with a flux-only MockBridge (saving the original
 * in _savedBridge for restoration in resetScale11). Configures lattice
 * toggles for flux-only mode, sets up scenario-specific flux patterns and
 * metadata, and wires the figure-type selector and audio toggle.
 *
 * @param {object} ctx  - Shared context from the main app
 * @param {string} name - Scenario key (default: 'cs-threshold')
 */
export function loadConsciousnessScenario(ctx, name = 'cs-threshold') {
    const { viewport, MockBridge } = ctx;

    ctx._resetAllVisualState();

    // Initialize ConsciousnessEngine if not yet created
    if (!_csEngine && viewport) {
        _csEngine = new ConsciousnessEngine(viewport.scene);
    }

    // Initialize pedagogy panels and info tooltips
    if (!_csPedagogy) {
        _csPedagogy = new ConsciousnessPedagogy();
        if (ctx.addInfoTooltips) ctx.addInfoTooltips();
        wireSubTabs(ctx);
    }

    // Create a flux-only MockBridge for lattice dynamics.
    // Save the real bridge so it can be restored when leaving consciousness mode.
    if (!ctx.bridge || !(ctx.bridge instanceof MockBridge)) {
        _savedBridge = ctx.bridge;
        ctx.bridge = new MockBridge(32);
    }

    const bridge = ctx.bridge;

    // Reset Mandelbrot iteration state
    _mandelbrotZ_re  = 0;
    _mandelbrotZ_im  = 0;
    _mandelbrotIter  = 0;
    _tickAcc.reset();

    // Base toggles: flux-only mode (no particles, no forces -- just waves)
    bridge.setToggle('wave_propagation', true);
    bridge.setToggle('coupling',        false);
    bridge.setToggle('damping',         true);
    bridge.setToggle('genesis',         false);
    bridge.setToggle('gauss_projection', false);
    bridge.setToggle('forces',          false);
    bridge.setToggle('gravity',         false);
    bridge.setToggle('movement',        false);
    bridge.setToggle('dual_substrate',  false);

    // Set up scenario-specific flux patterns and toggle overrides
    switch (name) {
        case 'cs-threshold': {
            // Start below K_C with low-amplitude Gaussian, gradually build
            // to cross real -> complex boundary
            const csMid    = Math.floor((bridge.latticeSize || 32) / 2);
            const csSubAmp = K_B * 0.3;  // 0.511 * 0.3
            const csSigma  = 4;
            for (let dz = -6; dz <= 6; dz++) {
                for (let dy = -6; dy <= 6; dy++) {
                    for (let dx = -6; dx <= 6; dx++) {
                        const r2  = dx * dx + dy * dy + dz * dz;
                        const val = csSubAmp * Math.exp(-r2 / (2 * csSigma * csSigma));
                        if (val > 0.001) {
                            bridge.injectFlux(csMid + dx, csMid + dy, csMid + dz, val, 0, 0);
                        }
                    }
                }
            }
            _csScenarioMeta = {
                name, domain: 'Real (k=16)', thetaMode: 'dynamic',
                sloopDepth: 0, bellS: null
            };
            break;
        }
        case 'cs-high-coupling': {
            // 4-source interference + coupling + forces (psychedelic high-flux state)
            bridge.setToggle('coupling', true);
            bridge.setToggle('forces',   true);
            bridge.setupScenario('flux-interference');
            _csScenarioMeta = {
                name, domain: 'Complex (k=\u00BD)', thetaMode: 'dynamic',
                sloopDepth: 0, bellS: null
            };
            break;
        }
        case 'cs-self-ref': {
            // Standing wave = observer meeting itself (sLoop depth 1)
            bridge.setupScenario('flux-standing');
            _csScenarioMeta = {
                name, domain: 'Complex (k=\u00BD)', thetaMode: 'static',
                sloopDepth: 1, bellS: null
            };
            break;
        }
        case 'cs-nested-sloop': {
            // Two orthogonal standing waves = self-aware of self-awareness (sLoop depth 2)
            bridge.setupScenario('flux-nested-standing');
            _csScenarioMeta = {
                name, domain: 'Complex (k=\u00BD)', thetaMode: 'static',
                sloopDepth: 2, bellS: null
            };
            break;
        }
        case 'cs-chirality': {
            // Dual substrate with asymmetric L/R injection
            bridge.setToggle('dual_substrate', true);
            bridge.setupScenario('flux-dual-substrate');
            _csScenarioMeta = {
                name, domain: 'Complex (k=\u00BD)', thetaMode: 'dynamic',
                sloopDepth: 1, bellS: null
            };
            break;
        }
        case 'cs-boundary-orbit': {
            // Mandelbrot c = 1/G* iteration tracking
            bridge.setupScenario('flux-soliton');
            _csScenarioMeta = {
                name, domain: 'Degenerate', thetaMode: 'dynamic',
                sloopDepth: 1, bellS: null
            };
            break;
        }
        case 'cs-entangled': {
            // Full coupling: dipole + genesis + forces + movement
            bridge.setToggle('coupling', true);
            bridge.setToggle('genesis',  true);
            bridge.setToggle('forces',   true);
            bridge.setToggle('movement', true);
            bridge.setupScenario('flux-dipole');
            _csScenarioMeta = {
                name, domain: 'Complex (k=\u00BD)', thetaMode: 'dynamic',
                sloopDepth: 1, bellS: 2.0
            };
            break;
        }
        case 'cs-flow': {
            // Fast vortex pattern, theta < 52.54 (object-dominant flow state)
            bridge.setupScenario('flux-vortex');
            _csScenarioMeta = {
                name, domain: 'Complex (k=\u00BD)', thetaMode: 'object',
                sloopDepth: 0, bellS: null
            };
            break;
        }
        case 'cs-meditation': {
            // Gentle centered pulse, theta > 52.54 (subject-dominant meditation)
            bridge.setupScenario('flux-pulse');
            _csScenarioMeta = {
                name, domain: 'Complex (k=\u00BD)', thetaMode: 'subject',
                sloopDepth: 0, bellS: null
            };
            break;
        }
        case 'cs-custom':
        default: {
            bridge.setupScenario('empty');
            _csScenarioMeta = {
                name: 'cs-custom', domain: '--', thetaMode: 'static',
                sloopDepth: 0, bellS: null
            };
            break;
        }
    }

    // Update static diagnostics from scenario metadata
    const sloopEl = document.getElementById('cs-diag-sloop');
    if (sloopEl) sloopEl.textContent = _csScenarioMeta.sloopDepth;

    const bellEl = document.getElementById('cs-diag-bell');
    if (bellEl) {
        bellEl.textContent = _csScenarioMeta.bellS !== null
            ? `S=${_csScenarioMeta.bellS.toFixed(1)}` : '--';
    }

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

// ── wireSubTabs ─────────────────────────────────────────────────────

/**
 * Wire consciousness pedagogy sub-tab navigation (Theory, Walkthrough,
 * Diagnostics) and scenario description updates.
 *
 * All listeners are attached via the module-level _subTabListeners bag
 * so that resetScale11 can remove them cleanly. Re-entering consciousness
 * mode simply clears the bag and re-wires, preventing listener leaks.
 *
 * @param {object} ctx - Shared context (unused currently, reserved)
 */
export function wireSubTabs(ctx) {
    // Clear any previously-attached listeners from a prior consciousness
    // session (safety net if resetScale11 was skipped somehow).
    _subTabListeners.clear();

    const subtabs   = document.querySelectorAll('.cs-subtab');
    const subpanels = document.querySelectorAll('.cs-subpanel');

    subtabs.forEach(st => {
        _subTabListeners.on(st, 'click', () => {
            subtabs.forEach(s => s.classList.remove('active'));
            st.classList.add('active');
            const target = st.dataset.cspanel;
            subpanels.forEach(sp => sp.classList.toggle('active', sp.id === target));

            if (_csPedagogy) {
                if (target === 'cs-theory') {
                    _csPedagogy.show();
                    _csPedagogy.resize();
                } else if (target === 'cs-walkthrough') {
                    _csPedagogy.startWalkthrough();
                    _csPedagogy.show();
                } else {
                    _csPedagogy.hide();
                }
            }
        });
    });

    // Scenario description wiring
    const scenarioSel = document.getElementById('cs-scenario-select');
    _subTabListeners.on(scenarioSel, 'change', () => {
        const descEl = document.getElementById('cs-scenario-desc-text');
        if (descEl) descEl.textContent = SCENARIO_DESCRIPTIONS[scenarioSel.value] || '';
    });

    // Walkthrough prev/next
    _subTabListeners.on(document.getElementById('cs-walk-prev'), 'click', () => {
        if (_csPedagogy) {
            _csPedagogy.setWalkthroughStep((_csPedagogy._walkthroughStep || 0) - 1);
        }
    });
    _subTabListeners.on(document.getElementById('cs-walk-next'), 'click', () => {
        if (_csPedagogy) {
            _csPedagogy.setWalkthroughStep((_csPedagogy._walkthroughStep || 0) + 1);
        }
    });
}

// ── step ────────────────────────────────────────────────────────────

/**
 * Advance the consciousness simulation by one tick (used by Step button / 's' key).
 * Ticks both the flux bridge and the consciousness engine.
 *
 * @param {object} ctx - Shared context
 */
export function step(ctx) {
    if (ctx.bridge) {
        ctx.bridge.tick();
    }
}

// ── resetScale11 ────────────────────────────────────────────────────

/**
 * Clean up Scale 11 state when leaving consciousness mode.
 * Disposes the ConsciousnessEngine, restores the original bridge,
 * and resets iteration state. Leaves _csPedagogy and its wired-up
 * DOM listeners alive for the page lifetime — see the comment below.
 *
 * @param {object} ctx - Shared context
 */
export function resetScale11(ctx) {
    if (_csEngine) {
        _csEngine.dispose();
        _csEngine = null;
    }

    // NOTE: _csPedagogy is intentionally NOT nulled here (Phase B.2 fix).
    // Its constructor attaches 'input'/'click' listeners to persistent
    // DOM nodes (_kSlider, _betaSlider, filter canvas) that have no
    // corresponding removeEventListener path. Re-creating the instance
    // on every consciousness re-entry would leak a fresh set of listeners
    // each cycle. Keeping the instance alive for the page lifetime is
    // both cheaper and leak-free; nothing about the pedagogy state needs
    // to be torn down between scale switches.
    //
    // For the same reason we do NOT call _subTabListeners.clear() here:
    // wireSubTabs only runs once (gated by `if (!_csPedagogy)` in
    // loadConsciousnessScenario), so the bag holds exactly one set of
    // listeners for the page lifetime.

    // Restore the real bridge that was saved when entering consciousness mode
    if (_savedBridge) {
        ctx.bridge = _savedBridge;
        _savedBridge = null;
    }

    // Reset iteration state
    _mandelbrotZ_re  = 0;
    _mandelbrotZ_im  = 0;
    _mandelbrotIter  = 0;
    _tickAcc.reset();
    _csScenarioMeta  = {
        name: '', domain: 'Real (k=16)', thetaMode: 'static',
        sloopDepth: 0, bellS: null
    };

    // Restore lattice particles visibility for other scales
    if (ctx && ctx.viewport && ctx.viewport.particles) {
        ctx.viewport.particles.visible = true;
    }
}

// ── External control accessors ─────────────────────────────────────

/**
 * Set the consciousness figure type (hologram, waveform, etc.)
 * Called from app_dag.js event handlers.
 */
export function setFigureType(type) {
    if (_csEngine) _csEngine.setFigureType(type);
}

/**
 * Enable consciousness audio with the given scenario name.
 */
export function enableAudio(scenarioName) {
    if (_csEngine) _csEngine.enableAudio(scenarioName);
}

/**
 * Disable consciousness audio.
 */
export function disableAudio() {
    if (_csEngine) _csEngine.disableAudio();
}

/**
 * Get the current scenario metadata (for event handlers that need the name).
 */
export function getScenarioMeta() {
    return _csScenarioMeta;
}
