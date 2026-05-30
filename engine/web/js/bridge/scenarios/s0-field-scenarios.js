/**
 * S0Field scenarios — s0-field-* group.
 *
 * Extracted from bridge/scenarios/index.js as part of Wave 3 tickets 8-13
 * of the large-file refactor (docs/SPEC_REFACTOR_LARGE_FILES.md §4). This
 * is a pure move — scenario bodies preserved verbatim; only the outer
 * `if (name.startsWith('s0-field-')) { ... }` wrapper was lifted out
 * and replaced with an early `return false` when the prefix does not
 * match, plus `return true` at the tail to signal handled.
 *
 * Call pattern: `setupS0FieldScenario.call(mockBridge, name, ctx)`
 * where ctx = { N, mid, midF } are the precomputed lattice parameters.
 * Returns true if the scenario was handled, false otherwise.
 */

import { ALPHA, COULOMB_K_FORCE, K_B, C_SPEED } from '../../constants.js';

/**
 * @param {string} name - scenario identifier
 * @param {{N:number, mid:number, midF:number}} ctx - precomputed lattice params
 * @returns {boolean} true if handled
 */
export function setupS0FieldScenario(name, ctx) {
    if (!name.startsWith('s0-field-')) return false;
    const { N, mid, midF } = ctx;
            this._initFluxGrid();
            const mc  = Math.round(midF);

            switch (name) {
                case 's0-field-plane-wave': {
                    // Z-polarized plane wave propagating +x, wavelength N/4
                    const wl  = N / 4;
                    const amp = K_B * 2;
                    const k   = 2 * Math.PI / wl;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const phase = k * x;
                        const jz    = amp * Math.sin(phase);
                        const wz    = amp * Math.cos(phase) * C_SPEED;
                        if (Math.abs(jz) > 1e-12 || Math.abs(wz) > 1e-12) {
                            this._injectFlux(x, y, z, 0, 0, jz);
                            this._injectWaveVel(x, y, z, wz, 0, 0);
                        }
                    }
                    break;
                }

                case 's0-field-standing-wave': {
                    // Z-polarized standing wave along x, wavelength N/4
                    const wl  = N / 4;
                    const amp = K_B * 2;
                    const k   = 2 * Math.PI / wl;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const jz = amp * Math.sin(k * x);
                        if (Math.abs(jz) > 1e-12) {
                            this._injectFlux(x, y, z, 0, 0, jz);
                        }
                        // wave_vel = 0 for standing wave (no net propagation)
                    }
                    break;
                }

                case 's0-field-uniform-e': {
                    // Uniform E field in the +x direction.
                    // genesis=false (audit-2 2026-04-28): a static uniform
                    // E field shouldn't fill the entire lattice with
                    // manifested particles. Schwinger-effect pair production
                    // at this lattice scale is unphysical. Without this,
                    // 32767 particles (≈INT16_MAX cap) by t=30.
                    this._toggles.genesis = false;
                    const eMag = 0.1;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        // E = -wave_vel, so wave_vel = -E
                        this._injectWaveVel(x, y, z, -eMag, 0, 0);
                    }
                    break;
                }

                case 's0-field-uniform-b': {
                    // Uniform B field in +z via vector-potential-like flux
                    // B = curl(J), so J = (-Bz*y/2, Bz*x/2, 0) relative to center
                    const bMag = 0.05;
                    const half = midF;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const rx = x - half, ry = y - half;
                        const jx = -bMag * ry / 2;
                        const jy =  bMag * rx / 2;
                        if (Math.abs(jx) > 1e-12 || Math.abs(jy) > 1e-12) {
                            this._injectFlux(x, y, z, jx, jy, 0);
                        }
                    }
                    break;
                }

                case 's0-field-photon-pulse': {
                    // Gaussian-enveloped plane wave, z-polarized, propagating +x
                    const sigma  = Math.max(3, Math.floor(N / 8));
                    const amp    = K_B * 2;
                    const lambdaEff = 4 * sigma;
                    const k      = 2 * Math.PI / lambdaEff;
                    const cutR   = 3.0 * sigma;
                    const cutR2  = cutR * cutR;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const dx = x - mc, dy = y - mc, dz = z - mc;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > cutR2) continue;
                        const g     = Math.exp(-r2 / (2 * sigma * sigma));
                        if (g < 1e-6) continue;
                        const phase = k * dx;
                        const jz    = amp * g * Math.sin(phase);
                        const wz    = amp * g * Math.cos(phase) * C_SPEED;
                        this._injectFlux(x, y, z, 0, 0, jz);
                        this._injectWaveVel(x, y, z, wz, 0, 0);
                    }
                    break;
                }

                case 's0-field-electric-dipole': {
                    // Two charges along x-axis separated by N/8
                    const sep  = Math.max(2, Math.floor(N / 8));
                    const half = Math.floor(sep / 2);
                    const px   = mc + half, nx = mc - half;
                    this.injectParticle(px, mc, mc, +1);
                    this.injectParticle(nx, mc, mc, -1);
                    // Coulomb dressing: superposed 1/r^2 from both charges.
                    // Uses COULOMB_K_FORCE (= α/4π) named alias for convention
                    // attribution (audit P1-6 fix, 2026-05-27).
                    const amp = COULOMB_K_FORCE;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        let jx = 0, jy = 0, jz = 0;
                        // +1 charge at (px, mc, mc)
                        const dx1 = x - px, dy1 = y - mc, dz1 = z - mc;
                        const r2_1 = dx1*dx1 + dy1*dy1 + dz1*dz1 + 1;
                        const f1   = amp / r2_1;
                        jx += f1 * dx1; jy += f1 * dy1; jz += f1 * dz1;
                        // -1 charge at (nx, mc, mc)
                        const dx2 = x - nx, dy2 = y - mc, dz2 = z - mc;
                        const r2_2 = dx2*dx2 + dy2*dy2 + dz2*dz2 + 1;
                        const f2   = -amp / r2_2;
                        jx += f2 * dx2; jy += f2 * dy2; jz += f2 * dz2;
                        const mag = Math.sqrt(jx*jx + jy*jy + jz*jz);
                        if (mag > 1e-6) {
                            this._injectFlux(x, y, z, jx, jy, jz);
                        }
                    }
                    break;
                }

                case 's0-field-magnetic-dipole': {
                    // Current loop in the xy-plane, moment along z
                    const loopR = Math.max(3, Math.floor(N / 8));
                    const amp   = K_B;
                    // For each angular position, find nearest lattice sites
                    const nAngles = Math.max(36, loopR * 8);
                    for (let i = 0; i < nAngles; i++) {
                        const theta = 2 * Math.PI * i / nAngles;
                        const lx = Math.round(mc + loopR * Math.cos(theta));
                        const ly = Math.round(mc + loopR * Math.sin(theta));
                        // Tangent direction = (-sin(theta), cos(theta), 0)
                        const tx = -Math.sin(theta) * amp;
                        const ty =  Math.cos(theta) * amp;
                        // Stamp across all z slices at this (x,y)
                        for (let z = 0; z < N; z++) {
                            this._injectFlux(lx, ly, z, tx, ty, 0);
                        }
                    }
                    break;
                }

                case 's0-field-vortex-line': {
                    // Vortex along z-axis through center: J = (Gamma/2pi*r) * theta_hat
                    const gamma = K_B * 4;
                    const half  = midF;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const rx = x - half, ry = y - half;
                        const r  = Math.max(Math.sqrt(rx * rx + ry * ry), 1.0);
                        const mag = gamma / (2 * Math.PI * r);
                        if (mag < 1e-6) continue;
                        // Azimuthal: theta_hat = (-ry/r, rx/r, 0)
                        this._injectFlux(x, y, z, -mag * ry / r, mag * rx / r, 0);
                    }
                    break;
                }
            }
            return true;
}
