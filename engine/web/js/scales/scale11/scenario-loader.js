/**
 * Scale 11 — Consciousness Scenario Loader
 * ────────────────────────────────────────────────────────────────────
 *
 * Extracted verbatim from scales/scale11/controller.js (ticket S11-1).
 * Houses the big `switch (name)` that arms flux patterns, toggle
 * overrides, and scenario metadata for each cs-* scenario.
 *
 * This is a pure move — no scenario body was changed. The controller
 * handles MockBridge creation, pedagogy wiring, base flux toggles, and
 * DOM diagnostic updates before and after this function runs.
 *
 * CONTRACT:
 *   setupConsciousnessScenario(name, bridge) configures the flux-only
 *   bridge for the named scenario and returns the scenario metadata
 *   the controller stores in _csScenarioMeta.
 *
 *   The caller has already:
 *     1. Reset Mandelbrot iteration state + tick accumulator
 *     2. Pushed base flux toggles (wave_propagation on, coupling/
 *        forces/genesis/gauss_projection/gravity/movement/
 *        dual_substrate off, damping on)
 *
 *   Return value:
 *     { name, domain, thetaMode, sloopDepth, bellS }
 */

import { K_B } from '../../constants.js';


/**
 * Run a consciousness scenario's setup body.
 * @param {string} name - scenario key (cs-*)
 * @param {object} bridge - flux-only MockBridge instance
 * @returns {{name:string, domain:string, thetaMode:string,
 *            sloopDepth:number, bellS:(number|null)}}
 */
export function setupConsciousnessScenario(name, bridge) {
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
            return { name, domain: 'Real (k=16)', thetaMode: 'dynamic',
                     sloopDepth: 0, bellS: null };
        }
        case 'cs-high-coupling': {
            // 4-source interference + coupling + forces (psychedelic high-flux state)
            bridge.setToggle('coupling', true);
            bridge.setToggle('forces',   true);
            bridge.setupScenario('flux-interference');
            return { name, domain: 'Complex (k=\u00BD)', thetaMode: 'dynamic',
                     sloopDepth: 0, bellS: null };
        }
        case 'cs-self-ref': {
            // Standing wave = observer meeting itself (sLoop depth 1)
            bridge.setupScenario('flux-standing');
            return { name, domain: 'Complex (k=\u00BD)', thetaMode: 'static',
                     sloopDepth: 1, bellS: null };
        }
        case 'cs-nested-sloop': {
            // Two orthogonal standing waves = self-aware of self-awareness (sLoop depth 2)
            bridge.setupScenario('flux-nested-standing');
            return { name, domain: 'Complex (k=\u00BD)', thetaMode: 'static',
                     sloopDepth: 2, bellS: null };
        }
        case 'cs-chirality': {
            // Dual substrate with asymmetric L/R injection
            bridge.setToggle('dual_substrate', true);
            bridge.setupScenario('flux-dual-substrate');
            return { name, domain: 'Complex (k=\u00BD)', thetaMode: 'dynamic',
                     sloopDepth: 1, bellS: null };
        }
        case 'cs-boundary-orbit': {
            // Mandelbrot c = 1/G* iteration tracking
            bridge.setupScenario('flux-soliton');
            return { name, domain: 'Degenerate', thetaMode: 'dynamic',
                     sloopDepth: 1, bellS: null };
        }
        case 'cs-entangled': {
            // Full coupling: dipole + genesis + forces + movement
            bridge.setToggle('coupling', true);
            bridge.setToggle('genesis',  true);
            bridge.setToggle('forces',   true);
            bridge.setToggle('movement', true);
            bridge.setupScenario('flux-dipole');
            return { name, domain: 'Complex (k=\u00BD)', thetaMode: 'dynamic',
                     sloopDepth: 1, bellS: 2.0 };
        }
        case 'cs-flow': {
            // Fast vortex pattern, theta < 52.54 (object-dominant flow state)
            bridge.setupScenario('flux-vortex');
            return { name, domain: 'Complex (k=\u00BD)', thetaMode: 'object',
                     sloopDepth: 0, bellS: null };
        }
        case 'cs-meditation': {
            // Gentle centered pulse, theta > 52.54 (subject-dominant meditation)
            bridge.setupScenario('flux-pulse');
            return { name, domain: 'Complex (k=\u00BD)', thetaMode: 'subject',
                     sloopDepth: 0, bellS: null };
        }
        case 'cs-custom':
        default: {
            bridge.setupScenario('empty');
            return { name: 'cs-custom', domain: '--', thetaMode: 'static',
                     sloopDepth: 0, bellS: null };
        }
    }
}
