/**
 * Light scenarios — light-* group.
 *
 * Extracted from bridge/scenarios/index.js as part of Wave 3 tickets 8-13
 * of the bridge modularization pass documented in engine/web/docs/INDEX.md. This
 * is a pure move — scenario bodies preserved verbatim; only the outer
 * `if (name.startsWith('light-')) { ... }` wrapper was lifted out
 * and replaced with an early `return false` when the prefix does not
 * match, plus `return true` at the tail to signal handled.
 *
 * Call pattern: `setupLightScenario.call(mockBridge, name, ctx)`
 * where ctx = { N, mid, midF } are the precomputed lattice parameters.
 * Returns true if the scenario was handled, false otherwise.
 */

import { C_SPEED } from '../../constants.js';
import {
    configureFreeWaveTerms,
    injectCoherentSlitPair,
    injectSheetPacketX,
    injectTransversePacketX,
} from './_helpers.js';

/**
 * @param {string} name - scenario identifier
 * @param {PhysicsHarness} harness - physics harness instance
 * @param {{N:number, mid:number, midF:number}} ctx - precomputed lattice params
 * @returns {boolean} true if handled
 */
export function setupLightScenario(name, harness, ctx) {
    if (!name.startsWith('light-')) return false;
    const { N, mid, midF, vox, sigma, band } = ctx;
    const sig = sigma;
    const mc = mid;
    const configureFreeWave = () => configureFreeWaveTerms(harness, true);
            const pi = Math.PI;
            const amp = 0.15;
            switch (name) {
                case 'light-rainbow': {
                    configureFreeWave();
                    // Three transverse harmonics. Propagation is along x, so
                    // x-polarization would be longitudinal and would be
                    // removed/distorted by the Gauss projection.
                    const waves = [
                        { n: 1, pol: 1 },  // red → y-polarized
                        { n: 3, pol: 2 },  // green → z-polarized
                        { n: 6, pol: 1 },  // blue → y-polarized
                    ];
                    for (const w of waves) {
                        const k = 2 * pi * w.n / N;
                        for (let x = 0; x < N; x++)
                        for (let y = 0; y < N; y++)
                        for (let z = 0; z < N; z++) {
                            const J_val = amp * Math.sin(k * x);
                            // Match the engine's kick-drift phase:
                            // W = -c D_x J - c^2 Lap(J)/2.
                            const halfSin = Math.sin(k / 2);
                            const wv_val = -C_SPEED * Math.sin(k) * amp * Math.cos(k * x)
                                + 2 * C_SPEED * C_SPEED * halfSin * halfSin * J_val;
                            const fv = [0, 0, 0], wv = [0, 0, 0];
                            fv[w.pol] = J_val;
                            wv[w.pol] = wv_val;
                            harness.injectFlux(x, y, z, fv[0], fv[1], fv[2]);
                            harness.injectWaveVel(x, y, z, wv[0], wv[1], wv[2]);
                        }
                    }
                    break;
                }
                case 'light-dipole': {
                    configureFreeWave();
                    injectTransversePacketX(harness, ctx, {
                        x0: midF - 2, y0: midF, z0: midF,
                        sigmaX: sig(2.5), sigmaT: sig(3), amp: 0.5, direction: -1,
                    });
                    injectTransversePacketX(harness, ctx, {
                        x0: midF + 2, y0: midF, z0: midF,
                        sigmaX: sig(2.5), sigmaT: sig(3), amp: 0.5, direction: +1,
                    });
                    break;
                }
                case 'light-two-slit': {
                    // Two coherent classical sources offset in y and
                    // propagating +x. There is no barrier/slit boundary and no
                    // single-particle quantum-interference claim.
                    configureFreeWave();
                    injectCoherentSlitPair(harness, ctx);
                    break;
                }
                case 'light-photon-race': {
                    // Dim vs bright Gaussian pulses — same speed (linearity)
                    configureFreeWave();
                    const raceSigma = sigma(3);
                    const raceHw = vox(2);
                    const x_start = vox(8);
                    const pAmps = [0.05, 0.5];
                    const transverseOffsets = [mid - vox(5), mid + vox(5)];
                    injectSheetPacketX(harness, ctx, {
                        x0: x_start, y0: transverseOffsets[0], sigmaX: raceSigma,
                        sigmaY: Math.max(1, raceHw), amp: pAmps[0], direction: +1,
                        polarizationAxis: 1,
                    });
                    injectSheetPacketX(harness, ctx, {
                        x0: x_start, y0: transverseOffsets[1], sigmaX: raceSigma,
                        sigmaY: Math.max(1, raceHw), amp: pAmps[1], direction: +1,
                        polarizationAxis: 2,
                    });
                    break;
                }

                default:
                    return false;
            }
            return true;
}
