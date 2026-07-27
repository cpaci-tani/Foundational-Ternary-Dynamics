/**
 * S0Field scenarios — s0-field-* group.
 *
 * Extracted from bridge/scenarios/index.js as part of Wave 3 tickets 8-13
 * of the bridge modularization pass documented in engine/web/docs/INDEX.md. This
 * is a pure move — scenario bodies preserved verbatim; only the outer
 * `if (name.startsWith('s0-field-')) { ... }` wrapper was lifted out
 * and replaced with an early `return false` when the prefix does not
 * match, plus `return true` at the tail to signal handled.
 *
 * Call pattern: `setupS0FieldScenario.call(mockBridge, name, ctx)`
 * where ctx = { N, mid, midF } are the precomputed lattice parameters.
 * Returns true if the scenario was handled, false otherwise.
 */

import { COULOMB_K_FORCE, K_B, C_SPEED } from '../../constants.js';
import {
    injectPlaneHarmonicX,
    injectStandingHarmonicX,
    injectTransversePacketX,
    configureFreeWaveTerms,
    configureStaticSeedTerms,
    configureLockedCoupledFieldTerms,
    configureEmergentRecoilTerms,
} from './_helpers.js';
import {
    LIGHT_LATTICE_WAVE_SCENARIO_ID,
    RF_LATTICE_WAVE_SCENARIO_ID,
    SOUND_LATTICE_WAVE_SCENARIO_ID,
    SOUND_COLLISION_SCENARIO_ID,
    seedSpectrumComparator,
} from './spectrum-comparator.js';

/**
 * @param {string} name - scenario identifier
 * @param {PhysicsHarness} harness - physics harness instance
 * @param {{N:number, mid:number, midF:number}} ctx - precomputed lattice params
 * @returns {boolean} true if handled
 */
export function setupS0FieldScenario(name, harness, ctx) {
    if (!name.startsWith('s0-field-')) return false;
    const { N, mid, midF, vox, sigma, band } = ctx;
    const sig = sigma;
    const mc = mid;
    const configureExactFreeWave = () => {
        for (const [key, value] of [
            ['wave_propagation', true], ['coupling', false], ['damping', false],
            ['selective_damping', false], ['genesis', false], ['evaporation', false],
            ['gauss_projection', false], ['forces', false], ['gravity', false],
            ['movement', false], ['poisson_coulomb', false], ['lorentz_force', false],
            ['larmor_radiation', false], ['dual_substrate', false],
            ['color_forces', false], ['strong_force', false], ['confinement', false],
            ['exchange_force', false], ['weak_transmutation', false],
        ]) harness.setToggle(key, value);
    };

            switch (name) {
                case 's0-field-plane-wave': {
                    // Exact z-polarized n=4 eigenmode propagating +x under
                    // the production kick-drift map.
                    configureExactFreeWave();
                    injectPlaneHarmonicX(harness, ctx, {
                        modeN: 4, amp: K_B * 2, direction: +1,
                    });
                    break;
                }

                case 's0-field-standing-wave': {
                    // Exact z-polarized n=4 standing eigenmode, including the
                    // pre-kick temporal stagger required by the engine map.
                    configureExactFreeWave();
                    injectStandingHarmonicX(harness, ctx, {
                        modeN: 4, amp: K_B * 2,
                    });
                    break;
                }

                case 's0-field-uniform-e': {
                    configureStaticSeedTerms(harness);
                    // Uniform E field in the +x direction.
                    // genesis=false (audit-2 2026-04-28): a static uniform
                    // E field shouldn't fill the entire lattice with
                    // manifested particles. Schwinger-effect pair production
                    // at this lattice scale is unphysical. Without this,
                    // 32767 particles (≈INT16_MAX cap) by t=30.
                    harness.setToggle('genesis', false);
                    const eMag = 0.1;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        // E = -wave_vel, so wave_vel = -E
                        harness.injectWaveVel(x, y, z, -eMag, 0, 0);
                    }
                    break;
                }

                case 's0-field-uniform-b': {
                    configureStaticSeedTerms(harness);
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
                            harness.injectFlux(x, y, z, jx, jy, 0);
                        }
                    }
                    break;
                }

                case 's0-field-photon-pulse': {
                    const pulseSigma = sig(4);
                    configureFreeWaveTerms(harness, true);
                    injectTransversePacketX(harness, ctx, {
                        x0: mc, y0: mc, z0: mc,
                        sigmaX: pulseSigma, sigmaT: Math.max(6, N / 4),
                        amp: K_B * 2, direction: +1,
                        carrierK: 2 * Math.PI / (4 * pulseSigma),
                    });
                    break;
                }

                case RF_LATTICE_WAVE_SCENARIO_ID:
                case LIGHT_LATTICE_WAVE_SCENARIO_ID:
                case SOUND_LATTICE_WAVE_SCENARIO_ID:
                case SOUND_COLLISION_SCENARIO_ID: {
                    seedSpectrumComparator(harness, ctx, name);
                    break;
                }

                case 's0-field-thomson-scattering': {
                    // Locked-source superposition null. The native four-arm
                    // campaign detects no interaction residual or recoil.
                    configureLockedCoupledFieldTerms(harness);

                    harness.injectParticle(mc, mc, mc, -1, { locked: true, spin: -1, color: 0 });
                    const modeN = 4;
                    const amp = 0.05;
                    const k = 2 * Math.PI * modeN / N;
                    const omega = 2 * Math.asin(C_SPEED * Math.abs(Math.sin(k / 2)));
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const jy = amp * Math.sin(k * x);
                        const wy = amp * ((1 - Math.cos(omega)) * Math.sin(k*x)
                            - Math.sin(omega) * Math.cos(k*x));
                        if (Math.abs(jy) > 1e-12) harness.injectFlux(x, y, z, 0, jy, 0);
                        if (Math.abs(wy) > 1e-12) harness.injectWaveVel(x, y, z, 0, wy, 0);
                    }
                    break;
                }

                case 's0-field-thomson-unlocked-recoil': {
                    // Native flux-gradient recoil probe. This does not claim
                    // a Thomson cross section or identify the marker as a
                    // physical electron.
                    configureEmergentRecoilTerms(harness);

                    harness.injectParticle(mc, mc, mc, -1, { locked: false, spin: -1, color: 0 });
                    const modeN = 4;
                    const amp = 0.05;
                    const k = 2 * Math.PI * modeN / N;
                    const omega = 2 * Math.asin(C_SPEED * Math.abs(Math.sin(k / 2)));
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const jy = amp * Math.sin(k * x);
                        const wy = amp * ((1 - Math.cos(omega)) * Math.sin(k*x)
                            - Math.sin(omega) * Math.cos(k*x));
                        if (Math.abs(jy) > 1e-12) harness.injectFlux(x, y, z, 0, jy, 0);
                        if (Math.abs(wy) > 1e-12) harness.injectWaveVel(x, y, z, 0, wy, 0);
                    }
                    break;
                }

                case 's0-field-spacetime-forcing-boundary': {
                    // Native production-wave point response. The diffusion
                    // comparison in the legacy demo is counterfactual only.
                    configureFreeWaveTerms(harness, false);
                    harness.injectFlux(mc, mc, mc, 0, 0, 1.0);
                    harness.injectWaveVel(mc, mc, mc, 0, 0, 1.0);
                    break;
                }

                case 's0-field-electric-dipole': {
                    // Imposed softened opposite-source Coulomb-shaped flux.
                    configureStaticSeedTerms(harness);
                    // Two ternary markers along x-axis separated by N/8.
                    const sep  = vox(4);
                    const half = Math.floor(sep / 2);
                    const px   = mc + half, nx = mc - half;
                    harness.injectParticle(px, mc, mc, +1);
                    harness.injectParticle(nx, mc, mc, -1);
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
                        const f1   = amp / Math.pow(r2_1, 1.5);
                        jx += f1 * dx1; jy += f1 * dy1; jz += f1 * dz1;
                        // -1 charge at (nx, mc, mc)
                        const dx2 = x - nx, dy2 = y - mc, dz2 = z - mc;
                        const r2_2 = dx2*dx2 + dy2*dy2 + dz2*dz2 + 1;
                        const f2   = -amp / Math.pow(r2_2, 1.5);
                        jx += f2 * dx2; jy += f2 * dy2; jz += f2 * dz2;
                        const mag = Math.sqrt(jx*jx + jy*jy + jz*jz);
                        if (mag > 1e-6) {
                            harness.injectFlux(x, y, z, jx, jy, jz);
                        }
                    }
                    break;
                }

                case 's0-field-magnetic-dipole': {
                    // Softened dipole vector potential A = mu x r /
                    // (r^2+a^2)^(3/2), mu parallel to +z.
                    configureStaticSeedTerms(harness);
                    const half = midF;
                    const muAmp = K_B / (4 * Math.PI);
                    for (let z=0;z<N;z++) for (let y=0;y<N;y++) for (let x=0;x<N;x++) {
                        const rx=x-half, ry=y-half, rz=z-half;
                        const denom=Math.pow(rx*rx+ry*ry+rz*rz+1,1.5);
                        const ax=-muAmp*ry/denom, ay=muAmp*rx/denom;
                        if(Math.hypot(ax,ay)>1e-8) harness.injectFlux(x,y,z,ax,ay,0);
                    }
                    break;
                }

                case 's0-field-vortex-line': {
                    // Imposed azimuthal 1/r profile; no physical vortex identity.
                    configureStaticSeedTerms(harness);
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
                        harness.injectFlux(x, y, z, -mag * ry / r, mag * rx / r, 0);
                    }
                    break;
                }
            }
            return true;
}
