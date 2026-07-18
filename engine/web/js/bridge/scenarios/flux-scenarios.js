/**
 * Flux scenarios — flux-* group.
 *
 * Extracted from bridge/scenarios/index.js as part of Wave 3 tickets 8-13
 * of the bridge modularization pass documented in engine/web/docs/INDEX.md. This
 * is a pure move — scenario bodies preserved verbatim; only the outer
 * `if (name.startsWith('flux-')) { ... }` wrapper was lifted out
 * and replaced with an early `return false` when the prefix does not
 * match, plus `return true` at the tail to signal handled.
 *
 * Call pattern: `setupFluxScenario.call(mockBridge, name, ctx)`
 * where ctx = { N, mid, midF } are the precomputed lattice parameters.
 * Returns true if the scenario was handled, false otherwise.
 */

import { K_B, K_GENESIS } from '../../constants.js';
import { TRIAD_ANGLES, injectParticleFull } from './_helpers.js';

/**
 * @param {string} name - scenario identifier
 * @param {PhysicsHarness} harness - physics harness instance
 * @param {{N:number, mid:number, midF:number, vox:function, sigma:function, band:function}} ctx
 * @returns {boolean} true if handled
 */
export function setupFluxScenario(name, harness, ctx) {
    if (!name.startsWith('flux-')) return false;
    const { N, mid, midF, vox, sigma, band } = ctx;
    const sig = sigma;
    const mc = mid;
            const pulseSigma = sigma(3.3);
            const amp = K_B * 2;

            switch (name) {
                case 'flux-pulse': {
                    // Gaussian pulse — loop anchored at midF so visual centroid = N/2 exactly
                    const pulseR = Math.min(Math.ceil(pulseSigma * 3), Math.floor(midF));
                    const pLo = Math.floor(midF) - pulseR, pHi = Math.ceil(midF) + pulseR;
                    for (let z = pLo; z <= pHi; z++) for (let y = pLo; y <= pHi; y++) for (let x = pLo; x <= pHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * pulseSigma * pulseSigma));
                        if (val > 0.001) harness.injectFlux(x, y, z, val, 0, 0);
                    }
                    break;
                }
                case 'flux-dipole': {
                    // Two opposite flux injections — poles symmetric about midF
                    const off = vox(8);
                    const pLx = Math.floor(midF) - off, pRx = Math.ceil(midF) + off;
                    const poleHw = vox(4);
                    const poleSig = sigma(3);
                    const { lo: yzLo, hi: yzHi } = band(midF, 4);
                    for (let z = yzLo; z <= yzHi; z++) for (let y = yzLo; y <= yzHi; y++) for (let dx = -poleHw; dx <= poleHw; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * poleSig * poleSig));
                        if (val > 0.001) {
                            harness.injectFlux(pLx + dx, y, z, val, val * 0.5, 0);
                            harness.injectFlux(pRx + dx, y, z, -val, -val * 0.5, 0);
                        }
                    }
                    break;
                }
                case 'flux-standing': {
                    // Counter-propagating pulses along X — poles symmetric about midF
                    const off = vox(11);
                    const pLx = Math.floor(midF) - off, pRx = Math.ceil(midF) + off;
                    const poleHw = vox(4);
                    const poleSig = sigma(3);
                    const { lo: yzLo, hi: yzHi } = band(midF, 4);
                    for (let z = yzLo; z <= yzHi; z++) for (let y = yzLo; y <= yzHi; y++) for (let dx = -poleHw; dx <= poleHw; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * poleSig * poleSig));
                        if (val > 0.001) {
                            harness.injectFlux(pLx + dx, y, z, val, 0, 0);
                            harness.injectFlux(pRx + dx, y, z, val, 0, 0);
                        }
                    }
                    break;
                }
                case 'flux-soliton': {
                    // Large amplitude nonlinear pulse — centered at midF.
                    // genesis=false (audit-2 2026-04-28): a soliton is a
                    // *non-dispersive localized wave*, not a pair-producer;
                    // the high amp * 10 exceeded K_GENESIS as the wave
                    // evolved, manifesting ~28k particles by t=200.
                    harness.setToggle('genesis', false);
                    const coreSig = sigma(2);
                    const { lo: sLo, hi: sHi } = band(midF, 3);
                    for (let z = sLo; z <= sHi; z++) for (let y = sLo; y <= sHi; y++) for (let x = sLo; x <= sHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 10 * Math.exp(-r2 / (2 * coreSig * coreSig));
                        if (val > 0.001) harness.injectFlux(x, y, z, val, val, 0);
                    }
                    break;
                }
                case 'flux-cascade': {
                    // Above genesis threshold — centered at midF
                    const bigAmp = K_GENESIS * 3;
                    const coreSig = sigma(2);
                    const { lo: cLo, hi: cHi } = band(midF, 3);
                    for (let z = cLo; z <= cHi; z++) for (let y = cLo; y <= cHi; y++) for (let x = cLo; x <= cHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = bigAmp * Math.exp(-r2 / (2 * coreSig * coreSig));
                        if (val > 0.001) harness.injectFlux(x, y, z, val, 0, val * 0.5);
                    }
                    break;
                }
                case 'flux-annihilation': {
                    // Two matter-antimatter pairs on collision courses — symmetric about midF
                    const off = vox(11);
                    const pL = Math.floor(midF) - off, pR = Math.ceil(midF) + off;
                    const mc = Math.round(midF); // nearest integer to true center
                    // X-axis pair
                    harness.injectParticle(pL, mc, mc, 1);
                    harness.injectParticle(pR, mc, mc, -1);
                    // Z-axis pair
                    harness.injectParticle(mc, mc, pL, -1);
                    harness.injectParticle(mc, mc, pR, 1);
                    // Strong flux kicks toward center for dramatic head-on collisions
                    const pushAmp = amp * 2;
                    const kickSig = sigma(2);
                    const { lo: kLo, hi: kHi } = band(midF, 3);
                    for (let z = kLo; z <= kHi; z++) for (let y = kLo; y <= kHi; y++) for (let x = kLo; x <= kHi; x++) {
                        const dy = y - midF, dz = z - midF;
                        // X-axis pair kicks
                        const dxL = x - pL, dxR = x - pR;
                        const r2L = dxL*dxL + dy*dy + dz*dz;
                        const r2R = dxR*dxR + dy*dy + dz*dz;
                        const valL = pushAmp * Math.exp(-r2L / (2 * kickSig * kickSig));
                        const valR = pushAmp * Math.exp(-r2R / (2 * kickSig * kickSig));
                        if (valL > 0.001) harness.injectFlux(x, y, z, valL, 0, 0);
                        if (valR > 0.001) harness.injectFlux(x, y, z, -valR, 0, 0);
                        // Z-axis pair kicks
                        const dzL = z - pL, dzR = z - pR;
                        const dx0 = x - mc;
                        const r2ZL = dx0*dx0 + dy*dy + dzL*dzL;
                        const r2ZR = dx0*dx0 + dy*dy + dzR*dzR;
                        const valZL = pushAmp * Math.exp(-r2ZL / (2 * kickSig * kickSig));
                        const valZR = pushAmp * Math.exp(-r2ZR / (2 * kickSig * kickSig));
                        if (valZL > 0.001) harness.injectFlux(x, y, z, 0, 0, valZL);
                        if (valZR > 0.001) harness.injectFlux(x, y, z, 0, 0, -valZR);
                    }
                    break;
                }
                case 'flux-pair-production': {
                    // Super-threshold flux burst — centered at midF
                    const bigAmp = K_GENESIS * 5;
                    const burstSig = sigma(Math.sqrt(6));
                    const { lo: ppLo, hi: ppHi } = band(midF, 4);
                    for (let z = ppLo; z <= ppHi; z++) for (let y = ppLo; y <= ppHi; y++) for (let x = ppLo; x <= ppHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = bigAmp * Math.exp(-r2 / (2 * burstSig * burstSig));
                        if (val > 0.001) harness.injectFlux(x, y, z, val, val * 0.7, val * 0.3);
                    }
                    break;
                }
                case 'flux-interference': {
                    // 4 coherent sources — symmetric about midF in X and Z
                    const q = vox(8);
                    const qL = Math.floor(midF) - q, qR = Math.ceil(midF) + q;
                    const mc = Math.round(midF);
                    const sources = [
                        [qL, mc, qL], [qR, mc, qL],
                        [qL, mc, qR], [qR, mc, qR],
                    ];
                    for (const [sx, sy, sz] of sources) {
                        const srcHw = vox(4);
                        const srcSig = sigma(Math.sqrt(6));
                        for (let dz = -srcHw; dz <= srcHw; dz++) for (let dy = -srcHw; dy <= srcHw; dy++) for (let dx = -srcHw; dx <= srcHw; dx++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = amp * 1.5 * Math.exp(-r2 / (2 * srcSig * srcSig));
                            if (val > 0.001) harness.injectFlux(sx + dx, sy + dy, sz + dz, val, 0, 0);
                        }
                    }
                    break;
                }
                case 'flux-vortex': {
                    // Circular-polarized flux ring — centered at midF
                    const vRadius = vox(6);
                    const nV = 24;
                    const mc = Math.round(midF);
                    for (let i = 0; i < nV; i++) {
                        const angle = (2 * Math.PI * i) / nV;
                        const rx = Math.round(midF + vRadius * Math.cos(angle));
                        const rz = Math.round(midF + vRadius * Math.sin(angle));
                        const tX = -Math.sin(angle) * amp * 2;
                        const tZ = Math.cos(angle) * amp * 2;
                        const tY = amp * 0.5;
                        harness.injectFlux(rx, mc, rz, tX, tY, tZ);
                        harness.injectFlux(rx, mc + 1, rz, tX * 0.5, tY * 0.5, tZ * 0.5);
                        harness.injectFlux(rx, mc - 1, rz, tX * 0.5, -tY * 0.5, tZ * 0.5);
                    }
                    break;
                }
                case 'flux-dual-substrate': {
                    // L/R chirality demo — poles symmetric about midF
                    const off = vox(8);
                    const pLx = Math.floor(midF) - off, pRx = Math.ceil(midF) + off;
                    const poleHw = vox(5);
                    const poleSig = sigma(Math.sqrt(8));
                    const { lo: yzLo, hi: yzHi } = band(midF, 5);
                    for (let z = yzLo; z <= yzHi; z++) for (let y = yzLo; y <= yzHi; y++) for (let dx = -poleHw; dx <= poleHw; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 1.5 * Math.exp(-r2 / (2 * poleSig * poleSig));
                        if (val > 0.001) {
                            harness.injectFlux(pLx + dx, y, z, val, val * 0.5, -val * 0.3);
                            harness.injectFlux(pRx + dx, y, z, val, -val * 0.5, val * 0.3);
                        }
                    }
                    break;
                }
                case 'flux-random-genesis': {
                    // Random super-threshold flux patches → stochastic particle creation
                    const nPatches = 8;
                    const threshold = K_GENESIS * 2.5;
                    const margin = vox(4);
                    const patchSpan = vox(8);
                    const patchHw = vox(2);
                    const patchSig = sigma(Math.sqrt(3));
                    for (let p = 0; p < nPatches; p++) {
                        const cx = Math.floor(Math.random() * (N - patchSpan)) + margin;
                        const cy = Math.floor(Math.random() * (N - patchSpan)) + margin;
                        const cz = Math.floor(Math.random() * (N - patchSpan)) + margin;
                        const pAmp = threshold * (0.8 + Math.random() * 0.8);
                        for (let dz = -patchHw; dz <= patchHw; dz++) for (let dy = -patchHw; dy <= patchHw; dy++) for (let dx = -patchHw; dx <= patchHw; dx++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = pAmp * Math.exp(-r2 / (2 * patchSig * patchSig));
                            if (val > 0.001) {
                                const sx = (Math.random() - 0.5) * val;
                                const sy = (Math.random() - 0.5) * val;
                                const sz = (Math.random() - 0.5) * val;
                                harness.injectFlux(cx + dx, cy + dy, cz + dz, sx, sy, sz);
                            }
                        }
                    }
                    break;
                }

                // ── QCD Scenarios ──
                case 'flux-meson': {
                    // Quark-antiquark bound state — poles symmetric about midF
                    const mOff = vox(4);
                    const mDress = vox(3);
                    const mL = Math.floor(midF) - mOff, mR = Math.ceil(midF) + mOff;
                    const mc = Math.round(midF);
                    injectParticleFull(harness, mL, mc, mc, 1, { vy: 0.05 });
                    injectParticleFull(harness, mR, mc, mc, -1, { vy: -0.05 });
                    const mesonAmp = K_B * 1.5;
                    const mSigma2 = mDress * mDress;
                    const myzLo = Math.floor(midF) - mDress, myzHi = Math.ceil(midF) + mDress;
                    for (let z = myzLo; z <= myzHi; z++) for (let y = myzLo; y <= myzHi; y++) for (let dx = -mDress; dx <= mDress; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = mesonAmp * Math.exp(-r2 / (2 * mSigma2));
                        if (val > 0.001) {
                            harness.injectFlux(mL + dx, y, z, val, 0, 0);
                            harness.injectFlux(mR + dx, y, z, -val, 0, 0);
                        }
                    }
                    break;
                }
                case 'flux-string-breaking': {
                    // Confinement string snap — poles symmetric about midF
                    const sbOff = vox(3);
                    const sbDress = vox(4);
                    const sbL = Math.floor(midF) - sbOff, sbR = Math.ceil(midF) + sbOff;
                    const mc = Math.round(midF);
                    injectParticleFull(harness, sbL, mc, mc, 1, { vx: -0.3 });
                    injectParticleFull(harness, sbR, mc, mc, -1, { vx: 0.3 });
                    // High flux at true center for genesis when string snaps
                    const sbAmp = K_B * 3;
                    const sbLo = Math.floor(midF) - sbDress, sbHi = Math.ceil(midF) + sbDress;
                    for (let z = sbLo; z <= sbHi; z++) for (let y = sbLo; y <= sbHi; y++) for (let x = sbLo; x <= sbHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = sbAmp * Math.exp(-r2 / (2 * sbDress));
                        if (val > 0.001) harness.injectFlux(x, y, z, val, val * 0.3, 0);
                    }
                    break;
                }
                case 'flux-baryon': {
                    // Three-quark equilateral triangle — centered at midF
                    const bR = vox(5);
                    const mc = Math.round(midF);
                    for (let k = 0; k < 3; k++) {
                        const angle = TRIAD_ANGLES[k];
                        const bx = Math.round(midF + bR * Math.cos(angle));
                        const bz = Math.round(midF + bR * Math.sin(angle));
                        injectParticleFull(harness, bx, mc, bz, 1, {
                            vx: -0.04 * Math.sin(angle),
                            vz: 0.04 * Math.cos(angle),
                        });
                    }
                    const bSea = Math.max(1, Math.floor(bR / 2));
                    harness.injectParticle(mc + bSea, mc + bSea, mc, -1);
                    // Light flux dressing centered at midF
                    const dressSig = sigma(2);
                    const { lo: bLo, hi: bHi } = band(midF, 3);
                    for (let z = bLo; z <= bHi; z++) for (let y = bLo; y <= bHi; y++) for (let x = bLo; x <= bHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 0.5 * Math.exp(-r2 / (2 * dressSig * dressSig));
                        if (val > 0.001) harness.injectFlux(x, y, z, val, 0, val * 0.3);
                    }
                    break;
                }

                case 'flux-nested-standing': {
                    // Two orthogonal standing wave pairs — all poles symmetric about midF
                    const offX = vox(11);
                    const offZ = vox(8);
                    const xL = Math.floor(midF) - offX, xR = Math.ceil(midF) + offX;
                    const zL = Math.floor(midF) - offZ, zR = Math.ceil(midF) + offZ;
                    const poleHw = vox(4);
                    const poleSig = sigma(3);
                    const { lo: yzLo, hi: yzHi } = band(midF, 4);
                    for (let z = yzLo; z <= yzHi; z++) for (let y = yzLo; y <= yzHi; y++) for (let dx = -poleHw; dx <= poleHw; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * poleSig * poleSig));
                        if (val > 0.001) {
                            harness.injectFlux(xL + dx, y, z, val, 0, 0);
                            harness.injectFlux(xR + dx, y, z, val, 0, 0);
                        }
                    }
                    for (let x = yzLo; x <= yzHi; x++) for (let y = yzLo; y <= yzHi; y++) for (let dz = -poleHw; dz <= poleHw; dz++) {
                        const dx = x - midF, dy = y - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * poleSig * poleSig));
                        if (val > 0.001) {
                            harness.injectFlux(x, y, zL + dz, 0, 0, val);
                            harness.injectFlux(x, y, zR + dz, 0, 0, val);
                        }
                    }
                    break;
                }

                // ── Experiment scenarios (from test suite) ──

                case 'flux-cyclotron': {
                    // Cyclotron motion: uniform B-field (curl of J) + charged particle
                    // (from test_gpu_experiments GP-EXP-CYCLOTRON)
                    // Create background B-field along z by injecting circular flux in xy-plane
                    const bAmp = amp * 0.15;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        // J = B × r / 2 for uniform B_z → J_x = -B*y/2, J_y = +B*x/2
                        const cx = x - mid, cy = y - mid;
                        harness.injectFlux(x, y, z, -bAmp * cy * 0.05, bAmp * cx * 0.05, 0);
                    }
                    // Charged particle with velocity in +x
                    harness.injectParticle(mid, mid, mid, 1);
                    const dressHw = vox(3);
                    const dressSig = sigma(2);
                    for (let d = -dressHw; d <= dressHw; d++) for (let dy = -dressHw; dy <= dressHw; dy++) for (let dx = -dressHw; dx <= dressHw; dx++) {
                        const r2 = dx * dx + dy * dy + d * d;
                        const val = amp * Math.exp(-r2 / (2 * dressSig * dressSig));
                        if (val > 0.001) {
                            harness.injectFlux(mid + dx, mid + dy, mid + d, val * 0.5, 0, 0);
                        }
                    }
                    break;
                }

                case 'flux-screening': {
                    // Charge screening: central +1 surrounded by 6 opposite charges
                    // (from test_gpu_experiments GP-EXP-SCREENING / Debye-Hückel)
                    const shellR = vox(6);
                    harness.injectParticle(mid, mid, mid, 1);
                    // 6 screening charges on face-axes
                    const scOffsets = [
                        [shellR, 0, 0], [-shellR, 0, 0],
                        [0, shellR, 0], [0, -shellR, 0],
                        [0, 0, shellR], [0, 0, -shellR],
                    ];
                    for (const [ox, oy, oz] of scOffsets) {
                        harness.injectParticle(mid + ox, mid + oy, mid + oz, -1);
                    }
                    // Seed flux dressing around central charge (scales with L)
                    const scDress = Math.max(3, Math.floor(shellR * 0.8));
                    const scDress2 = scDress * scDress;
                    for (let dz = -scDress; dz <= scDress; dz++) for (let dy = -scDress; dy <= scDress; dy++) for (let dx = -scDress; dx <= scDress; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 === 0 || r2 > scDress2) continue;
                        const r = Math.sqrt(r2);
                        const val = amp * 0.5 / r;
                        harness.injectFlux(mid + dx, mid + dy, mid + dz, val * dx / r, val * dy / r, val * dz / r);
                    }
                    break;
                }

                case 'flux-triad': {
                    // Triad formation: 3 same-sign particles in equilateral triangle
                    // (from campaign_triad_binding / campaign_baryon_formation)
                    const tR = vox(5);
                    for (const angle of TRIAD_ANGLES) {
                        const px = mid + Math.round(tR * Math.cos(angle));
                        const pz = mid + Math.round(tR * Math.sin(angle));
                        harness.injectParticle(px, mid, pz, 1);
                        // Flux kick toward center (binding)
                        const bindHw = vox(3);
                        const bindSig = sigma(2);
                        for (let dx = -bindHw; dx <= bindHw; dx++) for (let dy = -bindHw; dy <= bindHw; dy++) for (let dz = -bindHw; dz <= bindHw; dz++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = amp * 0.5 * Math.exp(-r2 / (2 * bindSig * bindSig));
                            if (val > 0.001) {
                                const toCX = (mid - (px + dx));
                                const toCZ = (mid - (pz + dz));
                                const dist = Math.sqrt(toCX * toCX + toCZ * toCZ) || 1;
                                harness.injectFlux(px + dx, mid + dy, pz + dz,
                                    val * toCX / dist, 0, val * toCZ / dist);
                            }
                        }
                    }
                    break;
                }

                case 'flux-thermalization': {
                    // Thermalization: concentrated energy in one corner → watch it spread
                    // (from test_thermodynamics — entropy increase demo)
                    const corner = Math.floor(N / 4);
                    const thermAmp = amp * 3;
                    const thermSig = sigma(Math.sqrt(6));
                    const thermHw = vox(4);
                    for (let dz = -thermHw; dz <= thermHw; dz++) for (let dy = -thermHw; dy <= thermHw; dy++) for (let dx = -thermHw; dx <= thermHw; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = thermAmp * Math.exp(-r2 / (2 * thermSig * thermSig));
                        if (val > 0.001) {
                            // Random flux directions for maximum entropy growth
                            const rx = (Math.random() - 0.5) * 2;
                            const ry = (Math.random() - 0.5) * 2;
                            const rz2 = (Math.random() - 0.5) * 2;
                            const rLen = Math.sqrt(rx * rx + ry * ry + rz2 * rz2) || 1;
                            harness.injectFlux(corner + dx, corner + dy, corner + dz,
                                val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
                        }
                    }
                    break;
                }


                case 'flux-vacuum-foam': {
                    // Near-threshold flux everywhere → spontaneous pair creation/annihilation
                    const foamR = vox(11);
                    const foamBase = K_B * 0.9;
                    const foamVar = K_B * 0.4;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const dx = x - mid, dy = y - mid, dz = z - mid;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > foamR * foamR) continue;
                        const envelope = Math.exp(-r2 / (2 * foamR * foamR * 0.5));
                        const val = (foamBase + foamVar * Math.random()) * envelope;
                        // Random flux direction
                        const rx = (Math.random() - 0.5) * 2;
                        const ry = (Math.random() - 0.5) * 2;
                        const rz2 = (Math.random() - 0.5) * 2;
                        const rLen = Math.sqrt(rx * rx + ry * ry + rz2 * rz2) || 1;
                        harness.injectFlux(x, y, z, val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
                    }
                    break;
                }

                case 'flux-zero-point': {
                    // Zero-Point Energy — the irreducible ground-state floor.
                    // Uniform LOW-amplitude random flux across the WHOLE lattice
                    // (no envelope, no sphere), at the same 0.3·K_B the "Random
                    // Flux" action uses — magnitude ≈ 0.08, ~20× below
                    // K_GENESIS (= N_c·K_MANIFEST = 1.5164, FTD-0388), so nothing can manifest.
                    // With genesis + damping both OFF (config/toggles.js), the
                    // energy-conserving wave dynamics keep this jittering
                    // indefinitely: a persistent non-zero energy floor that never
                    // relaxes to exactly zero (watch the energy-audit / Lagrangian
                    // overlays) and — unlike flux-vacuum-foam — never produces a
                    // particle. Pedagogical lattice illustration, NOT a derivation
                    // of the QFT ½ℏω vacuum energy. Amplitude is a [SELECTION].
                    const zpeAmp = K_B * 0.3;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        harness.injectFlux(x, y, z,
                            (Math.random() - 0.5) * zpeAmp,
                            (Math.random() - 0.5) * zpeAmp,
                            (Math.random() - 0.5) * zpeAmp);
                    }
                    break;
                }
            }
            return true;
}
