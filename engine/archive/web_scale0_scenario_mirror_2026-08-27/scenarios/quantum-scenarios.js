/**
 * Quantum scenarios — quantum-* group.
 *
 * Extracted from bridge/scenarios/index.js as part of Wave 3 tickets 8-13
 * of the bridge modularization pass documented in engine/web/docs/INDEX.md. This
 * is a pure move — scenario bodies preserved verbatim; only the outer
 * `if (name.startsWith('quantum-')) { ... }` wrapper was lifted out
 * and replaced with an early `return false` when the prefix does not
 * match, plus `return true` at the tail to signal handled.
 *
 * Call pattern: `setupQuantumScenario.call(mockBridge, name, ctx)`
 * where ctx = { N, mid, midF } are the precomputed lattice parameters.
 * Returns true if the scenario was handled, false otherwise.
 */

import { K_B, K_GENESIS } from '../../constants.js';
import {
    injectCoherentSlitPair,
    injectLockedBarrierWall,
    injectLockedYZPlane,
    injectParticleFull,
    injectTransversePacketX,
    configureGenesisGateTerms,
    configureFreeWaveTerms,
    injectPlaneHarmonicX,
} from './_helpers.js';

/**
 * @param {string} name - scenario identifier
 * @param {PhysicsHarness} harness - physics harness instance
 * @param {{N:number, mid:number, midF:number}} ctx - precomputed lattice params
 * @returns {boolean} true if handled
 */
export function setupQuantumScenario(name, harness, ctx) {
    if (!name.startsWith('quantum-')) return false;
    const { N, mid, midF, vox, sigma, band } = ctx;
    const sig = sigma;
    const mc = mid;
    const configureFreeWave = () => {
        for (const [key, value] of [
            ['wave_propagation', true], ['coupling', false], ['damping', false],
            ['selective_damping', false], ['genesis', false],
            ['gauss_projection', true], ['forces', false], ['movement', false],
            ['gravity', false], ['poisson_coulomb', false], ['lorentz_force', false],
            ['dual_substrate', false], ['weak_transmutation', false],
            ['color_forces', false], ['strong_force', false], ['confinement', false],
            ['exchange_force', false], ['larmor_radiation', false], ['evaporation', false],
        ]) harness.setToggle(key, value);
    };

            switch (name) {
                case 'quantum-born-rule': {
                    // Fixed-orientation Gaussian J/W pulse -> isolated native
                    // threshold/excess genesis response. No Born-law claim.
                    configureGenesisGateTerms(harness);
                    const bornSigma = sigma(4.125);
                    const amp = K_B * 2;
                    const theta = Math.PI / 7;
                    const pulseR = Math.min(Math.ceil(bornSigma * 3), mid - 1);
                    for (let dz = -pulseR; dz <= pulseR; dz++)
                    for (let dy = -pulseR; dy <= pulseR; dy++)
                    for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * bornSigma * bornSigma));
                        if (val > 0.001) {
                            const jx = val * Math.cos(theta);
                            const jy = val * Math.sin(theta);
                            harness.injectFlux(mid + dx, mid + dy, mid + dz, jx, jy, 0);
                            harness.injectWaveVel(mid + dx, mid + dy, mid + dz, jx, jy, 0);
                        }
                    }
                    break;
                }
                case 'quantum-double-slit': {
                    configureFreeWave();
                    injectCoherentSlitPair(harness, ctx);
                    break;
                }
                case 'quantum-eraser': {
                    configureFreeWave();
                    injectCoherentSlitPair(harness, ctx);
                    injectLockedYZPlane(harness, Math.floor(N / 2), N, { parity: 'even' });
                    harness.setToggle('coupling', true);
                    break;
                }
                case 'quantum-tunnel': {
                    // Native state-wall transmission/null test (not a Schrödinger barrier).
                    configureFreeWave();
                    harness.setToggle('coupling', true);
                    const tunnelSigma = sigma(2.75);
                    const packetX = vox(8);
                    injectTransversePacketX(harness, ctx, {
                        x0: packetX, y0: mid, z0: mid,
                        sigmaX: tunnelSigma, sigmaT: tunnelSigma,
                        amp: K_B * 0.5, direction: +1,
                    });
                    // Barrier: locked +1 particles across y-z plane
                    const W = harness.bridge._quantumBarrierWidth || vox(3);
                    injectLockedBarrierWall(harness, mid, N, W, 1);
                    break;
                }
                case 'quantum-well': {
                    // Marker planes are not Gauss charge sheets or wave
                    // boundaries; isolate the unprojected native wave map.
                    configureFreeWaveTerms(harness, false);
                    // Imposed broadband standing basis between marker walls.
                    const wallA = Math.floor(N / 4);
                    const wallB = Math.floor(3 * N / 4);
                    const boxLength = wallB - wallA;
                    injectLockedYZPlane(harness, wallA, N);
                    injectLockedYZPlane(harness, wallB, N);
                    // Broadband flux between walls: modes n=1..8
                    for (let n = 1; n <= 8; n++) {
                        const amp_n = K_B * 0.5 / n;
                        for (let x = wallA + 1; x < wallB; x++)
                        for (let y = 0; y < N; y++)
                        for (let z = 0; z < N; z++) {
                            const val = amp_n * Math.sin(n * Math.PI * (x - wallA) / boxLength);
                            if (Math.abs(val) > 1e-6) {
                                harness.injectFlux(x, y, z, 0, val, 0);
                            }
                        }
                    }
                    break;
                }
                case 'quantum-entangle': {
                    // Native tagged anti-correlated pair; classical correlation, not Bell entanglement.
                    harness.createEntangledPair(mid, mid, mid, 0, 0, K_B);
                    harness.setToggle('genesis', false);
                    harness.setToggle('evaporation', false);
                    harness.setToggle('movement', false);
                    break;
                }
                case 'quantum-aharonov-bohm': {
                    // Solenoid flux tube + two packets passing on opposite sides.
                    // genesis=false (audit-2 2026-04-28): the A-B effect is a
                    // *gauge-phase* phenomenon — the packets should NOT
                    // pair-produce while traversing the solenoid. Without
                    // this, ~31k particles by t=200.
                    configureFreeWave();
                    const R = vox(4);
                    // Confined flux tube along z at center (solenoid)
                    for (let z = 0; z < N; z++)
                    for (let dy = -R; dy <= R; dy++)
                    for (let dx = -R; dx <= R; dx++) {
                        if (dx * dx + dy * dy > R * R) continue;
                        harness.injectFlux(mid + dx, mid + dy, z, 0, 0, K_B * 0.5);
                    }
                    // Packet A: above solenoid, propagating +x
                    const pSigma = sigma(3);
                    const pGap = vox(2);
                    const pStartX = vox(8);
                    injectTransversePacketX(harness, ctx, {
                        x0: pStartX, y0: mid + R + pGap, z0: mid,
                        sigmaX: pSigma, sigmaT: pSigma, amp: K_B * 0.5, direction: +1,
                    });
                    injectTransversePacketX(harness, ctx, {
                        x0: pStartX, y0: mid - R - pGap, z0: mid,
                        sigmaX: pSigma, sigmaT: pSigma, amp: K_B * 0.5, direction: +1,
                    });
                    break;
                }
                case 'quantum-casimir': {
                    // Two inert marker planes plus a reproducible transverse
                    // eigenmode. This is a plate-transparency null, not vacuum
                    // fluctuations or a Casimir-force calculation.
                    configureFreeWaveTerms(harness, false);
                    const d = harness.bridge._quantumCasimirSep || vox(6);
                    const plateA = mid - Math.floor(d / 2);
                    const plateB = mid + Math.floor(d / 2);
                    injectLockedYZPlane(harness, plateA, N);
                    injectLockedYZPlane(harness, plateB, N);
                    injectPlaneHarmonicX(harness, ctx, {
                        modeN: 4, amp: 0.05, direction: +1,
                    });
                    break;
                }
                case 'quantum-zeno': {
                    // Unobserved near-threshold control; no measurement operator is present.
                    configureGenesisGateTerms(harness);
                    const zenoSigma = sigma(3.3);
                    const amp = K_GENESIS * 1.2;
                    const pulseR = Math.min(Math.ceil(zenoSigma * 3), mid - 1);
                    for (let dz = -pulseR; dz <= pulseR; dz++)
                    for (let dy = -pulseR; dy <= pulseR; dy++)
                    for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * zenoSigma * zenoSigma));
                        if (val > 0.001) {
                            harness.injectFlux(mid + dx, mid + dy, mid + dz, val, val, val);
                            harness.injectWaveVel(mid + dx, mid + dy, mid + dz, val, val, val);
                        }
                    }
                    break;
                }

                default:
                    return false;
            }
            return true;
}
