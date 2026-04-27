/**
 * Flux scenarios — flux-* group.
 *
 * Extracted from bridge/scenarios/index.js as part of Wave 3 tickets 8-13
 * of the large-file refactor (docs/SPEC_REFACTOR_LARGE_FILES.md §4). This
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
import { TRIAD_ANGLES } from './_helpers.js';

/**
 * @param {string} name - scenario identifier
 * @param {{N:number, mid:number, midF:number}} ctx - precomputed lattice params
 * @returns {boolean} true if handled
 */
export function setupFluxScenario(name, ctx) {
    if (!name.startsWith('flux-')) return false;
    const { N, mid, midF } = ctx;
            this._initFluxGrid();
            const sigma = N / 10;
            const amp = K_B * 2;

            switch (name) {
                case 'flux-pulse': {
                    // Gaussian pulse — loop anchored at midF so visual centroid = N/2 exactly
                    const pulseR = Math.min(Math.ceil(sigma * 3), Math.floor(midF));
                    const pLo = Math.floor(midF) - pulseR, pHi = Math.ceil(midF) + pulseR;
                    for (let z = pLo; z <= pHi; z++) for (let y = pLo; y <= pHi; y++) for (let x = pLo; x <= pHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (val > 0.001) this._injectFlux(x, y, z, val, 0, 0);
                    }
                    break;
                }
                case 'flux-dipole': {
                    // Two opposite flux injections — poles symmetric about midF
                    const off = Math.floor(N / 4);
                    const pLx = Math.floor(midF) - off, pRx = Math.ceil(midF) + off;
                    const yzLo = Math.floor(midF) - 4, yzHi = Math.ceil(midF) + 4;
                    for (let z = yzLo; z <= yzHi; z++) for (let y = yzLo; y <= yzHi; y++) for (let dx = -4; dx <= 4; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * 9));
                        if (val > 0.001) {
                            this._injectFlux(pLx + dx, y, z, val, val * 0.5, 0);
                            this._injectFlux(pRx + dx, y, z, -val, -val * 0.5, 0);
                        }
                    }
                    break;
                }
                case 'flux-standing': {
                    // Counter-propagating pulses along X — poles symmetric about midF
                    const off = Math.floor(N / 3);
                    const pLx = Math.floor(midF) - off, pRx = Math.ceil(midF) + off;
                    const yzLo = Math.floor(midF) - 4, yzHi = Math.ceil(midF) + 4;
                    for (let z = yzLo; z <= yzHi; z++) for (let y = yzLo; y <= yzHi; y++) for (let dx = -4; dx <= 4; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * 9));
                        if (val > 0.001) {
                            this._injectFlux(pLx + dx, y, z, val, 0, 0);
                            this._injectFlux(pRx + dx, y, z, val, 0, 0);
                        }
                    }
                    break;
                }
                case 'flux-soliton': {
                    // Large amplitude nonlinear pulse — centered at midF
                    const sLo = Math.floor(midF) - 3, sHi = Math.ceil(midF) + 3;
                    for (let z = sLo; z <= sHi; z++) for (let y = sLo; y <= sHi; y++) for (let x = sLo; x <= sHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 10 * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) this._injectFlux(x, y, z, val, val, 0);
                    }
                    break;
                }
                case 'flux-cascade': {
                    // Above genesis threshold — centered at midF
                    const bigAmp = K_GENESIS * 3;
                    const cLo = Math.floor(midF) - 3, cHi = Math.ceil(midF) + 3;
                    for (let z = cLo; z <= cHi; z++) for (let y = cLo; y <= cHi; y++) for (let x = cLo; x <= cHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = bigAmp * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) this._injectFlux(x, y, z, val, 0, val * 0.5);
                    }
                    break;
                }
                case 'flux-annihilation': {
                    // Two matter-antimatter pairs on collision courses — symmetric about midF
                    const off = Math.floor(N / 3);
                    const pL = Math.floor(midF) - off, pR = Math.ceil(midF) + off;
                    const mc = Math.round(midF); // nearest integer to true center
                    // X-axis pair
                    this.injectParticle(pL, mc, mc, 1);
                    this.injectParticle(pR, mc, mc, -1);
                    // Z-axis pair
                    this.injectParticle(mc, mc, pL, -1);
                    this.injectParticle(mc, mc, pR, 1);
                    // Strong flux kicks toward center for dramatic head-on collisions
                    const pushAmp = amp * 2;
                    const kLo = Math.floor(midF) - 3, kHi = Math.ceil(midF) + 3;
                    for (let z = kLo; z <= kHi; z++) for (let y = kLo; y <= kHi; y++) for (let x = kLo; x <= kHi; x++) {
                        const dy = y - midF, dz = z - midF;
                        // X-axis pair kicks
                        const dxL = x - pL, dxR = x - pR;
                        const r2L = dxL*dxL + dy*dy + dz*dz;
                        const r2R = dxR*dxR + dy*dy + dz*dz;
                        const valL = pushAmp * Math.exp(-r2L / (2 * 4));
                        const valR = pushAmp * Math.exp(-r2R / (2 * 4));
                        if (valL > 0.001) this._injectFlux(x, y, z, valL, 0, 0);
                        if (valR > 0.001) this._injectFlux(x, y, z, -valR, 0, 0);
                        // Z-axis pair kicks
                        const dzL = z - pL, dzR = z - pR;
                        const dx0 = x - mc;
                        const r2ZL = dx0*dx0 + dy*dy + dzL*dzL;
                        const r2ZR = dx0*dx0 + dy*dy + dzR*dzR;
                        const valZL = pushAmp * Math.exp(-r2ZL / (2 * 4));
                        const valZR = pushAmp * Math.exp(-r2ZR / (2 * 4));
                        if (valZL > 0.001) this._injectFlux(x, y, z, 0, 0, valZL);
                        if (valZR > 0.001) this._injectFlux(x, y, z, 0, 0, -valZR);
                    }
                    break;
                }
                case 'flux-pair-production': {
                    // Super-threshold flux burst — centered at midF
                    const bigAmp = K_GENESIS * 5;
                    const ppLo = Math.floor(midF) - 4, ppHi = Math.ceil(midF) + 4;
                    for (let z = ppLo; z <= ppHi; z++) for (let y = ppLo; y <= ppHi; y++) for (let x = ppLo; x <= ppHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = bigAmp * Math.exp(-r2 / (2 * 6));
                        if (val > 0.001) this._injectFlux(x, y, z, val, val * 0.7, val * 0.3);
                    }
                    break;
                }
                case 'flux-interference': {
                    // 4 coherent sources — symmetric about midF in X and Z
                    const q = Math.floor(N / 4);
                    const qL = Math.floor(midF) - q, qR = Math.ceil(midF) + q;
                    const mc = Math.round(midF);
                    const sources = [
                        [qL, mc, qL], [qR, mc, qL],
                        [qL, mc, qR], [qR, mc, qR],
                    ];
                    for (const [sx, sy, sz] of sources) {
                        for (let dz = -4; dz <= 4; dz++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = amp * 1.5 * Math.exp(-r2 / (2 * 6));
                            if (val > 0.001) this._injectFlux(sx + dx, sy + dy, sz + dz, val, 0, 0);
                        }
                    }
                    break;
                }
                case 'flux-vortex': {
                    // Circular-polarized flux ring — centered at midF
                    const vRadius = Math.floor(N / 5);
                    const nV = 24;
                    const mc = Math.round(midF);
                    for (let i = 0; i < nV; i++) {
                        const angle = (2 * Math.PI * i) / nV;
                        const rx = Math.round(midF + vRadius * Math.cos(angle));
                        const rz = Math.round(midF + vRadius * Math.sin(angle));
                        const tX = -Math.sin(angle) * amp * 2;
                        const tZ = Math.cos(angle) * amp * 2;
                        const tY = amp * 0.5;
                        this._injectFlux(rx, mc, rz, tX, tY, tZ);
                        this._injectFlux(rx, mc + 1, rz, tX * 0.5, tY * 0.5, tZ * 0.5);
                        this._injectFlux(rx, mc - 1, rz, tX * 0.5, -tY * 0.5, tZ * 0.5);
                    }
                    break;
                }
                case 'flux-dual-substrate': {
                    // L/R chirality demo — poles symmetric about midF
                    const off = Math.floor(N / 4);
                    const pLx = Math.floor(midF) - off, pRx = Math.ceil(midF) + off;
                    const yzLo = Math.floor(midF) - 5, yzHi = Math.ceil(midF) + 5;
                    for (let z = yzLo; z <= yzHi; z++) for (let y = yzLo; y <= yzHi; y++) for (let dx = -5; dx <= 5; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 1.5 * Math.exp(-r2 / (2 * 8));
                        if (val > 0.001) {
                            this._injectFlux(pLx + dx, y, z, val, val * 0.5, -val * 0.3);
                            this._injectFlux(pRx + dx, y, z, val, -val * 0.5, val * 0.3);
                        }
                    }
                    break;
                }
                case 'flux-random-genesis': {
                    // Random super-threshold flux patches → stochastic particle creation
                    const nPatches = 8;
                    const threshold = K_GENESIS * 2.5;
                    for (let p = 0; p < nPatches; p++) {
                        const cx = Math.floor(Math.random() * (N - 8)) + 4;
                        const cy = Math.floor(Math.random() * (N - 8)) + 4;
                        const cz = Math.floor(Math.random() * (N - 8)) + 4;
                        const pAmp = threshold * (0.8 + Math.random() * 0.8);
                        for (let dz = -2; dz <= 2; dz++) for (let dy = -2; dy <= 2; dy++) for (let dx = -2; dx <= 2; dx++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = pAmp * Math.exp(-r2 / (2 * 3));
                            if (val > 0.001) {
                                const sx = (Math.random() - 0.5) * val;
                                const sy = (Math.random() - 0.5) * val;
                                const sz = (Math.random() - 0.5) * val;
                                this._injectFlux(cx + dx, cy + dy, cz + dz, sx, sy, sz);
                            }
                        }
                    }
                    break;
                }

                // ── QCD Scenarios ──
                case 'flux-meson': {
                    // Quark-antiquark bound state — poles symmetric about midF
                    const mOff = Math.max(2, Math.floor(N / 8));
                    const mDress = Math.max(2, Math.floor(N / 10));
                    const mL = Math.floor(midF) - mOff, mR = Math.ceil(midF) + mOff;
                    const mc = Math.round(midF);
                    this.injectParticle(mL, mc, mc, 1);
                    this.injectParticle(mR, mc, mc, -1);
                    const mpIdx = this._particles.length;
                    this._particles[mpIdx - 2].vy = 0.05;
                    this._particles[mpIdx - 1].vy = -0.05;
                    const mesonAmp = K_B * 1.5;
                    const mSigma2 = mDress * mDress;
                    const myzLo = Math.floor(midF) - mDress, myzHi = Math.ceil(midF) + mDress;
                    for (let z = myzLo; z <= myzHi; z++) for (let y = myzLo; y <= myzHi; y++) for (let dx = -mDress; dx <= mDress; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = mesonAmp * Math.exp(-r2 / (2 * mSigma2));
                        if (val > 0.001) {
                            this._injectFlux(mL + dx, y, z, val, 0, 0);
                            this._injectFlux(mR + dx, y, z, -val, 0, 0);
                        }
                    }
                    break;
                }
                case 'flux-string-breaking': {
                    // Confinement string snap — poles symmetric about midF
                    const sbOff = Math.max(2, Math.floor(N / 10));
                    const sbDress = Math.max(2, Math.floor(N / 8));
                    const sbL = Math.floor(midF) - sbOff, sbR = Math.ceil(midF) + sbOff;
                    const mc = Math.round(midF);
                    this.injectParticle(sbL, mc, mc, 1);
                    this.injectParticle(sbR, mc, mc, -1);
                    const sbIdx = this._particles.length;
                    this._particles[sbIdx - 2].vx = -0.3;
                    this._particles[sbIdx - 1].vx = 0.3;
                    // High flux at true center for genesis when string snaps
                    const sbAmp = K_B * 3;
                    const sbLo = Math.floor(midF) - sbDress, sbHi = Math.ceil(midF) + sbDress;
                    for (let z = sbLo; z <= sbHi; z++) for (let y = sbLo; y <= sbHi; y++) for (let x = sbLo; x <= sbHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = sbAmp * Math.exp(-r2 / (2 * sbDress));
                        if (val > 0.001) this._injectFlux(x, y, z, val, val * 0.3, 0);
                    }
                    break;
                }
                case 'flux-baryon': {
                    // Three-quark equilateral triangle — centered at midF
                    const bR = Math.floor(N / 6);
                    const mc = Math.round(midF);
                    for (let k = 0; k < 3; k++) {
                        const angle = TRIAD_ANGLES[k];
                        const bx = Math.round(midF + bR * Math.cos(angle));
                        const bz = Math.round(midF + bR * Math.sin(angle));
                        this.injectParticle(bx, mc, bz, 1);
                        const bidx = this._particles.length - 1;
                        this._particles[bidx].vx = -0.04 * Math.sin(angle);
                        this._particles[bidx].vz = 0.04 * Math.cos(angle);
                    }
                    const bSea = Math.max(1, Math.floor(bR / 2));
                    this.injectParticle(mc + bSea, mc + bSea, mc, -1);
                    // Light flux dressing centered at midF
                    const bLo = Math.floor(midF) - 3, bHi = Math.ceil(midF) + 3;
                    for (let z = bLo; z <= bHi; z++) for (let y = bLo; y <= bHi; y++) for (let x = bLo; x <= bHi; x++) {
                        const dx = x - midF, dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 0.5 * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) this._injectFlux(x, y, z, val, 0, val * 0.3);
                    }
                    break;
                }

                case 'flux-nested-standing': {
                    // Two orthogonal standing wave pairs — all poles symmetric about midF
                    const offX = Math.floor(N / 3);
                    const offZ = Math.floor(N / 4);
                    const xL = Math.floor(midF) - offX, xR = Math.ceil(midF) + offX;
                    const zL = Math.floor(midF) - offZ, zR = Math.ceil(midF) + offZ;
                    const yzLo = Math.floor(midF) - 4, yzHi = Math.ceil(midF) + 4;
                    for (let z = yzLo; z <= yzHi; z++) for (let y = yzLo; y <= yzHi; y++) for (let dx = -4; dx <= 4; dx++) {
                        const dy = y - midF, dz = z - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * 9));
                        if (val > 0.001) {
                            this._injectFlux(xL + dx, y, z, val, 0, 0);
                            this._injectFlux(xR + dx, y, z, val, 0, 0);
                        }
                    }
                    for (let x = yzLo; x <= yzHi; x++) for (let y = yzLo; y <= yzHi; y++) for (let dz = -4; dz <= 4; dz++) {
                        const dx = x - midF, dy = y - midF;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * 9));
                        if (val > 0.001) {
                            this._injectFlux(x, y, zL + dz, 0, 0, val);
                            this._injectFlux(x, y, zR + dz, 0, 0, val);
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
                        this._injectFlux(x, y, z, -bAmp * cy * 0.05, bAmp * cx * 0.05, 0);
                    }
                    // Charged particle with velocity in +x
                    this.injectParticle(mid, mid, mid, 1);
                    for (let d = -3; d <= 3; d++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                        const r2 = dx * dx + dy * dy + d * d;
                        const val = amp * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) {
                            this._injectFlux(mid + dx, mid + dy, mid + d, val * 0.5, 0, 0);
                        }
                    }
                    break;
                }

                case 'flux-screening': {
                    // Charge screening: central +1 surrounded by 6 opposite charges
                    // (from test_gpu_experiments GP-EXP-SCREENING / Debye-Hückel)
                    const shellR = Math.floor(N / 5);
                    this.injectParticle(mid, mid, mid, 1);
                    // 6 screening charges on face-axes
                    const scOffsets = [
                        [shellR, 0, 0], [-shellR, 0, 0],
                        [0, shellR, 0], [0, -shellR, 0],
                        [0, 0, shellR], [0, 0, -shellR],
                    ];
                    for (const [ox, oy, oz] of scOffsets) {
                        this.injectParticle(mid + ox, mid + oy, mid + oz, -1);
                    }
                    // Seed flux dressing around central charge (scales with L)
                    const scDress = Math.max(3, Math.floor(shellR * 0.8));
                    const scDress2 = scDress * scDress;
                    for (let dz = -scDress; dz <= scDress; dz++) for (let dy = -scDress; dy <= scDress; dy++) for (let dx = -scDress; dx <= scDress; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 === 0 || r2 > scDress2) continue;
                        const r = Math.sqrt(r2);
                        const val = amp * 0.5 / r;
                        this._injectFlux(mid + dx, mid + dy, mid + dz, val * dx / r, val * dy / r, val * dz / r);
                    }
                    break;
                }

                case 'flux-triad': {
                    // Triad formation: 3 same-sign particles in equilateral triangle
                    // (from campaign_triad_binding / campaign_baryon_formation)
                    const tR = Math.floor(N / 6);
                    for (const angle of TRIAD_ANGLES) {
                        const px = mid + Math.round(tR * Math.cos(angle));
                        const pz = mid + Math.round(tR * Math.sin(angle));
                        this.injectParticle(px, mid, pz, 1);
                        // Flux kick toward center (binding)
                        for (let dx = -3; dx <= 3; dx++) for (let dy = -3; dy <= 3; dy++) for (let dz = -3; dz <= 3; dz++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = amp * 0.5 * Math.exp(-r2 / (2 * 4));
                            if (val > 0.001) {
                                const toCX = (mid - (px + dx));
                                const toCZ = (mid - (pz + dz));
                                const dist = Math.sqrt(toCX * toCX + toCZ * toCZ) || 1;
                                this._injectFlux(px + dx, mid + dy, pz + dz,
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
                    for (let dz = -4; dz <= 4; dz++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = thermAmp * Math.exp(-r2 / (2 * 6));
                        if (val > 0.001) {
                            // Random flux directions for maximum entropy growth
                            const rx = (Math.random() - 0.5) * 2;
                            const ry = (Math.random() - 0.5) * 2;
                            const rz2 = (Math.random() - 0.5) * 2;
                            const rLen = Math.sqrt(rx * rx + ry * ry + rz2 * rz2) || 1;
                            this._injectFlux(corner + dx, corner + dy, corner + dz,
                                val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
                        }
                    }
                    break;
                }


                case 'flux-vacuum-foam': {
                    // Near-threshold flux everywhere → spontaneous pair creation/annihilation
                    const foamR = Math.floor(N / 3);
                    const foamBase = K_B * 0.9;
                    const foamVar = K_B * 0.4;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const dx = x - mid, dy = y - mid, dz = z - mid;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > foamR * foamR) continue;
                        const r = Math.sqrt(r2);
                        const envelope = Math.exp(-r2 / (2 * foamR * foamR * 0.5));
                        const val = (foamBase + foamVar * Math.random()) * envelope;
                        // Random flux direction
                        const rx = (Math.random() - 0.5) * 2;
                        const ry = (Math.random() - 0.5) * 2;
                        const rz2 = (Math.random() - 0.5) * 2;
                        const rLen = Math.sqrt(rx * rx + ry * ry + rz2 * rz2) || 1;
                        this._injectFlux(x, y, z, val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
                    }
                    break;
                }
            }
            return true;
}
