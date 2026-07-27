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

import { C_SPEED, K_B, K_GENESIS, K_MANIFEST } from '../../constants.js';
import {
    TRIAD_ANGLES,
    configureFreeWaveTerms,
    configureStaticSeedTerms,
    configureFreeMovementTerms,
    configureGenesisGateTerms,
    configurePairProductionTerms,
    configureAnnihilationTerms,
    configureLorentzOrbitTerms,
    injectParticleFull,
    injectTransversePacketX,
} from './_helpers.js';

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
                    // Isolated divergence-free packet used to characterize the
                    // finite-box boundary operators. No particle or EM identity.
                    configureFreeWaveTerms(harness, false);
                    const sx = Math.max(3, N / 16);
                    injectTransversePacketX(harness, ctx, {
                        x0: N / 3, y0: midF, z0: midF,
                        sigmaX: sx, sigmaT: sx, amp: K_B * 0.5,
                        direction: +1, carrierK: 2 * Math.PI / (4 * sx),
                    });
                    break;
                }
                case 'flux-dipole': {
                    // Antisymmetric pair of Gaussian vector-wave blobs.
                    configureFreeWaveTerms(harness, false);
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
                            harness.injectWaveVel(pLx + dx, y, z, val, val * 0.5, 0);
                            harness.injectFlux(pRx + dx, y, z, -val, -val * 0.5, 0);
                            harness.injectWaveVel(pRx + dx, y, z, -val, -val * 0.5, 0);
                        }
                    }
                    break;
                }
                case 'flux-standing': {
                    // Reflection-even, zero-initial-momentum broadband wave pair.
                    configureFreeWaveTerms(harness, false);
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
                    // High-amplitude packet dispersion diagnostic; no soliton term exists.
                    for (const [key, value] of [
                        ['wave_propagation', true], ['coupling', false], ['damping', false],
                        ['selective_damping', false], ['genesis', false],
                        ['gauss_projection', true], ['forces', false], ['movement', false],
                    ]) harness.setToggle(key, value);
                    injectTransversePacketX(harness, ctx, {
                        x0: midF, y0: midF, z0: midF,
                        sigmaX: sigma(2), sigmaT: sigma(2), amp: amp * 2, direction: +1,
                    });
                    break;
                }
                case 'flux-cascade': {
                    configureGenesisGateTerms(harness);
                    // Legacy supercritical Gaussian seed. Branching, outward
                    // recruitment, and pair production are not qualified.
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
                    // Exact native collision-removal probe. No wave propagation:
                    // only pre-existing flux is redistributed to face neighbours.
                    configureAnnihilationTerms(harness);
                    const mc = Math.floor(N / 2);
                    injectParticleFull(harness, mc - 1, mc, mc, +1, { vx: C_SPEED });
                    injectParticleFull(harness, mc, mc, mc, -1);
                    harness.injectFlux(mc - 1, mc, mc, 0, +K_B, 0);
                    harness.injectFlux(mc, mc, mc, 0, -K_B, 0);
                    break;
                }
                case 'flux-pair-production': {
                    // Isolated p=1/2 cohort for the selected native pair rule.
                    configurePairProductionTerms(harness);
                    const pairAmp = K_GENESIS + K_MANIFEST * Math.log(2);
                    for (let z = 2; z < N - 2; z += 3)
                    for (let y = 2; y < N - 2; y += 3)
                    for (let x = 2; x + 1 < N - 2; x += 3)
                        harness.injectFlux(x, y, z, pairAmp, 0, 0);
                    break;
                }
                case 'flux-interference': {
                    // Four-lobe reflection-symmetric broadband wave field.
                    configureFreeWaveTerms(harness, false);
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
                    // Exact discrete helical ring; no spin identification.
                    configureStaticSeedTerms(harness);
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
                    // Mirror-polarized pair. The dual_substrate operator is not
                    // engaged, so this is a vector-wave parity probe only.
                    configureFreeWaveTerms(harness, false);
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
                            harness.injectWaveVel(pLx + dx, y, z, val, val * 0.5, -val * 0.3);
                            harness.injectFlux(pRx + dx, y, z, val, -val * 0.5, val * 0.3);
                            harness.injectWaveVel(pRx + dx, y, z, val, -val * 0.5, val * 0.3);
                        }
                    }
                    break;
                }
                case 'flux-random-genesis': {
                    configureGenesisGateTerms(harness);
                    // Unqualified random super-threshold genesis setup. Native
                    // genesis creates individual states, not correlated pairs.
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

                case 'flux-genesis-between-gates': {
                    // FTD-0388 one-tick gate discriminator — three exact initial
                    // uniform-|J| cohorts along x:
                    // 1.5160 (below K_GENESIS = 3·W_SC = 1.5163860591519780, adopted
                    // 2026-07-17), 1.5250 (between the new gate and the retired
                    // 3·K_B = 1.533 gate), 1.5340 (above both). Only the initial
                    // decision sees the exact amplitudes: accepted genesis drains
                    // flux and the master rule also contains evaporation.
                    // The compiled first-tick hazards are 0 / 0.0168973 / 0.034247.
                    // No later frozen-cohort or independent-trial claim is made.
                    configureGenesisGateTerms(harness);
                    const bandAmp = [1.5160, 1.5250, 1.5340];
                    if (!(bandAmp[0] < K_GENESIS && K_GENESIS < bandAmp[1])) {
                        console.warn('[flux-genesis-between-gates] bands no longer straddle ' +
                            `K_GENESIS = ${K_GENESIS} — re-band this scenario (and the C++ twin)`);
                    }
                    const x1 = 1 + Math.floor((N - 2) / 3);
                    const x2 = 1 + Math.floor((2 * (N - 2)) / 3);
                    for (let x = 1; x < N - 1; x++) {
                        if (x === x1 || x === x2) continue; // 1-plane visual separators
                        const b = (x < x1) ? 0 : (x < x2) ? 1 : 2;
                        for (let z = 1; z < N - 1; z++) for (let y = 1; y < N - 1; y++)
                            harness.injectFlux(x, y, z, bandAmp[b], 0, 0);
                    }
                    break;
                }

                // ── QCD Scenarios ──
                case 'flux-meson': {
                    // Counter-moving opposite-state transport probe. No color
                    // labels, confinement operator, or meson identity.
                    configureFreeMovementTerms(harness);
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
                    configureFreeMovementTerms(harness);
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
                    configureFreeMovementTerms(harness);
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
                    // Orthogonal reflection-even broadband wave pairs.
                    configureFreeWaveTerms(harness, false);
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
                    // Imposed uniform-curl vector potential plus the selected
                    // native Lorentz response. No EM-emergence claim.
                    configureLorentzOrbitTerms(harness);
                    // alpha*B*dt < 0.01 keeps the unit-tick orbit resolved.
                    const imposedBz = 1.0;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const cx = x - midF, cy = y - midF;
                        harness.injectFlux(x, y, z,
                            -0.5 * imposedBz * cy,
                             0.5 * imposedBz * cx, 0);
                    }
                    harness.injectParticle(mid, mid, mid, +1, {
                        vx: 0.12, vy: 0, vz: 0,
                    });
                    break;
                }

                case 'flux-screening': {
                    // Prepared non-neutral octahedral polarity shell.
                    configureStaticSeedTerms(harness);
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
                    // Prepared threefold polarity seed with imposed inward flux.
                    configureStaticSeedTerms(harness);
                    const tR = vox(5);
                    for (const angle of TRIAD_ANGLES) {
                        const px = mid + Math.round(tR * Math.cos(angle));
                        const pz = mid + Math.round(tR * Math.sin(angle));
                        harness.injectParticle(px, mid, pz, 1);
                        // Inward flux dressing (initial data only).
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
                    // Localized random-wave mixing; no thermostat or entropy claim.
                    configureFreeWaveTerms(harness, false);
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
                    // Finite random-wave ball; no ongoing quantum/noise source.
                    configureFreeWaveTerms(harness, false);
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
                    // Selected finite periodic random-wave bath. It tests the
                    // source-free kick-drift invariant, not quantum zero-point
                    // energy, a ground state, or the QFT 1/2 hbar omega term.
                    configureFreeWaveTerms(harness, false);
                    const zpeAmp = K_B * 0.3;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const jx = (Math.random() - 0.5) * zpeAmp;
                        const jy = (Math.random() - 0.5) * zpeAmp;
                        const jz = (Math.random() - 0.5) * zpeAmp;
                        harness.injectFlux(x, y, z, jx, jy, jz);
                        harness.injectWaveVel(x, y, z, jx, jy, jz);
                    }
                    break;
                }
            }
            return true;
}
