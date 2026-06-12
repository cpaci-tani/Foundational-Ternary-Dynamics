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

/**
 * @param {string} name - scenario identifier
 * @param {PhysicsHarness} harness - physics harness instance
 * @param {{N:number, mid:number, midF:number}} ctx - precomputed lattice params
 * @returns {boolean} true if handled
 */
export function setupQuantumScenario(name, harness, ctx) {
    if (!name.startsWith('quantum-')) return false;
    const { N, mid } = ctx;

            switch (name) {
                case 'quantum-born-rule': {
                    // Random-phase Gaussian flux pulse → Born rule P = |ψ|² statistics
                    const sigma = N / 8;
                    const amp = K_B * 2;
                    const theta = Math.random() * 2 * Math.PI;
                    const pulseR = Math.min(Math.ceil(sigma * 3), mid - 1);
                    for (let dz = -pulseR; dz <= pulseR; dz++)
                    for (let dy = -pulseR; dy <= pulseR; dy++)
                    for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (val > 0.001) {
                            harness.injectFlux(mid + dx, mid + dy, mid + dz,
                                val * Math.cos(theta), val * Math.sin(theta), 0);
                        }
                    }
                    harness.setToggle('genesis', true);
                    break;
                }
                case 'quantum-double-slit': {
                    // Two coherent line sources with genesis → interference + manifestation
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
                    harness.setToggle('genesis', true);
                    harness.setToggle('coupling', false);
                    break;
                }
                case 'quantum-eraser': {
                    // Quantum Eraser: Coherent slits marked orthogonally
                    const sigma = 2;
                    const sAmp = 0.3;
                    const slit_sep = Math.floor(N / 6);
                    const slit_x = Math.floor(N / 4);

                    // Slit 1: y-polarized
                    const sy1 = mid - slit_sep;
                    for (let z = 0; z < N; z++)
                    for (let dy = -4; dy <= 4; dy++)
                    for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy;
                        const g = sAmp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (g < 1e-6) continue;
                        const px = slit_x + dx, py = sy1 + dy;
                        if (px < 0 || px >= N || py < 0 || py >= N) continue;
                        harness.injectFlux(px, py, z, 0, g, 0); // y-polarized
                        harness.injectWaveVel(px, py, z, g, 0, 0); // propagate +x
                    }

                    // Slit 2: z-polarized
                    const sy2 = mid + slit_sep;
                    for (let z = 0; z < N; z++)
                    for (let dy = -4; dy <= 4; dy++)
                    for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy;
                        const g = sAmp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (g < 1e-6) continue;
                        const px = slit_x + dx, py = sy2 + dy;
                        if (px < 0 || px >= N || py < 0 || py >= N) continue;
                        harness.injectFlux(px, py, z, 0, 0, g); // z-polarized
                        harness.injectWaveVel(px, py, z, g, 0, 0); // propagate +x
                    }

                    // Diagonal eraser (y=z polarizer) at x = N/2
                    const eraserX = Math.floor(N / 2);
                    for (let y = 0; y < N; y++) {
                        for (let z = 0; z < N; z++) {
                            // Place locked particles along the y + z diagonal to form parallel conducting wires
                            if ((y + z) % 2 === 0) {
                                harness.injectParticle(eraserX, y, z, 1);
                                harness.bridge._particles[harness.bridge._particles.length - 1].locked = true;
                            }
                        }
                    }

                    harness.setToggle('genesis', true);
                    harness.setToggle('coupling', false);
                    break;
                }
                case 'quantum-tunnel': {
                    // Gaussian flux packet → barrier of locked particles → tunneling.
                    // genesis=false (audit-2 2026-04-28): the flux packet
                    // should TUNNEL through the barrier, not pair-produce.
                    // The 3072 initial particles are the locked barrier
                    // (32×32×W=3 wall) and stay constant; the wave was
                    // otherwise manifesting ~28k by t=200.
                    harness.setToggle('genesis', false);
                    const sigma = N / 12;
                    const amp = K_B * 2;
                    const packetX = Math.floor(N / 4);
                    const pulseR = Math.min(Math.ceil(sigma * 3), mid - 1);
                    // Gaussian flux packet propagating +x
                    for (let dz = -pulseR; dz <= pulseR; dz++)
                    for (let dy = -pulseR; dy <= pulseR; dy++)
                    for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (val > 0.001) {
                            const x = packetX + dx, y = mid + dy, z = mid + dz;
                            if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) {
                                harness.injectFlux(x, y, z, val, 0, 0);
                                harness.injectWaveVel(x, y, z, val, 0, 0); // +x propagation
                            }
                        }
                    }
                    // Barrier: locked +1 particles across y-z plane
                    const W = harness.bridge._quantumBarrierWidth || 3;
                    for (let y = 0; y < N; y++)
                    for (let z = 0; z < N; z++)
                    for (let dx = 0; dx < W; dx++) {
                        harness.injectParticle(mid + dx, y, z, 1);
                        harness.bridge._particles[harness.bridge._particles.length - 1].locked = true;
                    }
                    break;
                }
                case 'quantum-well': {
                    // Reflective walls + broadband standing waves → energy quantization
                    const wallA = Math.floor(N / 4);
                    const wallB = Math.floor(3 * N / 4);
                    const boxLength = wallB - wallA;
                    // Reflective walls: locked +1 particles across y-z planes
                    for (let y = 0; y < N; y++)
                    for (let z = 0; z < N; z++) {
                        harness.injectParticle(wallA, y, z, 1);
                        harness.bridge._particles[harness.bridge._particles.length - 1].locked = true;
                        harness.injectParticle(wallB, y, z, 1);
                        harness.bridge._particles[harness.bridge._particles.length - 1].locked = true;
                    }
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
                    harness.setToggle('genesis', false);
                    harness.setToggle('damping', false);
                    break;
                }
                case 'quantum-entangle': {
                    // Super-threshold flux burst → pair genesis + correlation tracking
                    const bigAmp = K_GENESIS * 5;
                    for (let dz = -4; dz <= 4; dz++)
                    for (let dy = -4; dy <= 4; dy++)
                    for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = bigAmp * Math.exp(-r2 / (2 * 6));
                        if (val > 0.001) {
                            harness.injectFlux(mid + dx, mid + dy, mid + dz, val, val, val);
                        }
                    }
                    harness.setToggle('genesis', true);
                    harness.bridge._quantumExperimentMode = 'entangle';
                    break;
                }
                case 'quantum-aharonov-bohm': {
                    // Solenoid flux tube + two packets passing on opposite sides.
                    // genesis=false (audit-2 2026-04-28): the A-B effect is a
                    // *gauge-phase* phenomenon — the packets should NOT
                    // pair-produce while traversing the solenoid. Without
                    // this, ~31k particles by t=200.
                    harness.setToggle('genesis', false);
                    const R = Math.floor(N / 8);
                    // Confined flux tube along z at center (solenoid)
                    for (let z = 0; z < N; z++)
                    for (let dy = -R; dy <= R; dy++)
                    for (let dx = -R; dx <= R; dx++) {
                        if (dx * dx + dy * dy > R * R) continue;
                        harness.injectFlux(mid + dx, mid + dy, z, 0, 0, K_B * 0.5);
                    }
                    // Packet A: above solenoid, propagating +x
                    const pSigma = 3;
                    const pAmp = K_B * 2;
                    const pStartX = Math.floor(N / 4);
                    for (let dz = -pSigma; dz <= pSigma; dz++)
                    for (let dy = -pSigma; dy <= pSigma; dy++)
                    for (let dx = -pSigma; dx <= pSigma; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = pAmp * Math.exp(-r2 / (2 * pSigma * pSigma));
                        if (val > 0.001) {
                            // Packet A: y = mid + R + 2
                            const ayPos = mid + R + 2 + dy;
                            if (pStartX + dx >= 0 && pStartX + dx < N && ayPos >= 0 && ayPos < N && mid + dz >= 0 && mid + dz < N) {
                                harness.injectFlux(pStartX + dx, ayPos, mid + dz, val, 0, 0);
                                harness.injectWaveVel(pStartX + dx, ayPos, mid + dz, val, 0, 0);
                            }
                            // Packet B: y = mid - R - 2
                            const byPos = mid - R - 2 + dy;
                            if (pStartX + dx >= 0 && pStartX + dx < N && byPos >= 0 && byPos < N && mid + dz >= 0 && mid + dz < N) {
                                harness.injectFlux(pStartX + dx, byPos, mid + dz, val, 0, 0);
                                harness.injectWaveVel(pStartX + dx, byPos, mid + dz, val, 0, 0);
                            }
                        }
                    }
                    break;
                }
                case 'quantum-casimir': {
                    // Two parallel plates + vacuum fluctuation noise → Casimir effect
                    const d = harness.bridge._quantumCasimirSep || 6;
                    const plateA = mid - Math.floor(d / 2);
                    const plateB = mid + Math.floor(d / 2);
                    // Locked +1 particles forming two plates across y-z
                    for (let y = 0; y < N; y++)
                    for (let z = 0; z < N; z++) {
                        harness.injectParticle(plateA, y, z, 1);
                        harness.bridge._particles[harness.bridge._particles.length - 1].locked = true;
                        harness.injectParticle(plateB, y, z, 1);
                        harness.bridge._particles[harness.bridge._particles.length - 1].locked = true;
                    }
                    // Fill entire lattice with low-amplitude random flux (vacuum foam)
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        harness.injectFlux(x, y, z,
                            (Math.random() - 0.5) * K_B * 0.3,
                            (Math.random() - 0.5) * K_B * 0.3,
                            (Math.random() - 0.5) * K_B * 0.3);
                    }
                    harness.setToggle('genesis', false);
                    break;
                }
                case 'quantum-zeno': {
                    // Near-threshold flux → genesis + frequent measurement suppresses decay
                    const sigma = N / 10;
                    const amp = K_GENESIS * 1.2;
                    const pulseR = Math.min(Math.ceil(sigma * 3), mid - 1);
                    for (let dz = -pulseR; dz <= pulseR; dz++)
                    for (let dy = -pulseR; dy <= pulseR; dy++)
                    for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (val > 0.001) {
                            harness.injectFlux(mid + dx, mid + dy, mid + dz, val, val, val);
                        }
                    }
                    harness.setToggle('genesis', true);
                    harness.bridge._quantumZenoInterval = harness.bridge._quantumZenoInterval || 10;
                    harness.bridge._quantumZenoMode = true;
                    break;
                }
            }
            return true;
}
