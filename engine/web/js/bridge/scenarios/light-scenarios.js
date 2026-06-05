/**
 * Light scenarios — light-* group.
 *
 * Extracted from bridge/scenarios/index.js as part of Wave 3 tickets 8-13
 * of the large-file refactor (docs/SPEC_REFACTOR_LARGE_FILES.md §4). This
 * is a pure move — scenario bodies preserved verbatim; only the outer
 * `if (name.startsWith('light-')) { ... }` wrapper was lifted out
 * and replaced with an early `return false` when the prefix does not
 * match, plus `return true` at the tail to signal handled.
 *
 * Call pattern: `setupLightScenario.call(mockBridge, name, ctx)`
 * where ctx = { N, mid, midF } are the precomputed lattice parameters.
 * Returns true if the scenario was handled, false otherwise.
 */

import { K_B, C_SPEED } from '../../constants.js';

/**
 * @param {string} name - scenario identifier
 * @param {PhysicsHarness} harness - physics harness instance
 * @param {{N:number, mid:number, midF:number}} ctx - precomputed lattice params
 * @returns {boolean} true if handled
 */
export function setupLightScenario(name, harness, ctx) {
    if (!name.startsWith('light-')) return false;
    const { N, mid, midF } = ctx;
            const pi = Math.PI;
            const amp = 0.15;
            switch (name) {
                case 'light-rainbow': {
                    // Three traveling waves: red (n=1,y), green (n=3,z), blue (n=6,x)
                    const waves = [
                        { n: 1, pol: 1 },  // red → y-polarized
                        { n: 3, pol: 2 },  // green → z-polarized
                        { n: 6, pol: 0 },  // blue → x-polarized
                    ];
                    for (const w of waves) {
                        const k = 2 * pi * w.n / N;
                        const omega = 2 * C_SPEED * Math.sin(k / 2);
                        for (let x = 0; x < N; x++)
                        for (let y = 0; y < N; y++)
                        for (let z = 0; z < N; z++) {
                            const J_val = amp * Math.sin(k * x);
                            const wv_val = -omega * amp * Math.cos(k * x);
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
                    // Gaussian z-directed pulse → sin²θ radiation.
                    // genesis=false (audit-2 2026-04-28): classical EM
                    // dipole radiation is not a pair-producer; the
                    // wave evolution would otherwise manifest ~29k
                    // particles by t=200.
                    harness.setToggle('genesis', false);
                    const sigma = 3;
                    const dAmp = 0.5;
                    for (let x = 0; x < N; x++)
                    for (let y = 0; y < N; y++)
                    for (let z = 0; z < N; z++) {
                        const dx = x - mid, dy = y - mid, dz = z - mid;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const g = dAmp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (g < 1e-6) continue;
                        harness.injectFlux(x, y, z, 0, 0, g);
                        harness.injectWaveVel(x, y, z, 0, 0, g);
                    }
                    break;
                }
                case 'light-two-slit': {
                    // Two coherent line sources offset in y, propagating in +x.
                    // genesis=false (audit-2 2026-04-28): classical
                    // double-slit interference; should NOT manifest
                    // particles. Without this, ~31k particles by t=200.
                    harness.setToggle('genesis', false);
                    const sigma = 2;
                    const sAmp = 0.3;
                    const slit_sep = Math.floor(N / 6);
                    const slit_x = Math.floor(N / 4);
                    const slit_ys = [mid - slit_sep, mid + slit_sep];
                    for (const sy of slit_ys) {
                        for (let z = 0; z < N; z++)
                        for (let dy = -4; dy <= 4; dy++)
                        for (let dx = -4; dx <= 4; dx++) {
                            const r2 = dx * dx + dy * dy;
                            const g = sAmp * Math.exp(-r2 / (2 * sigma * sigma));
                            if (g < 1e-6) continue;
                            const px = slit_x + dx, py = sy + dy;
                            if (px < 0 || px >= N || py < 0 || py >= N) continue;
                            harness.injectFlux(px, py, z, 0, 0, g);
                            harness.injectWaveVel(px, py, z, g, 0, 0); // propagate +x
                        }
                    }
                    break;
                }
                case 'light-photon-race': {
                    // Dim vs bright Gaussian pulses — same speed (linearity)
                    const sigma = 3;
                    const x_start = Math.floor(N / 4);
                    const pAmps = [0.05, 0.5];
                    const y_offsets = [mid - Math.floor(N / 6), mid + Math.floor(N / 6)];
                    for (let p = 0; p < 2; p++) {
                        for (let x = 0; x < N; x++) {
                            const dx = x - x_start;
                            const g = pAmps[p] * Math.exp(-dx * dx / (2 * sigma * sigma));
                            if (g < 1e-8) continue;
                            for (let y = y_offsets[p] - 2; y <= y_offsets[p] + 2; y++)
                            for (let z = mid - 2; z <= mid + 2; z++) {
                                if (y < 0 || y >= N || z < 0 || z >= N) continue;
                                harness.injectFlux(x, y, z, 0, 0, g);
                                harness.injectWaveVel(x, y, z, 0, 0, g); // outgoing +x
                            }
                        }
                    }
                    break;
                }
            }
            return true;
}
