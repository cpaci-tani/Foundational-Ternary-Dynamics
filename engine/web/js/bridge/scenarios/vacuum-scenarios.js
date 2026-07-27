/**
 * Vacuum Particle Scenarios — s0-vacuum-* group.
 *
 * Curated single-particle-in-vacuum showcases. See
 * engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md for the contract:
 * each scenario applies a uniform vacuum environment, then injects one
 * particle at the lattice center with no extras (no other particles,
 * no background field, no boundary tweaks beyond engine defaults).
 *
 * 10 of the 15 cases are wrappers around existing s0-seed-* injectors;
 * 5 are net-new in this file: 3 neutrino flavors + π⁰ + K±.
 *
 * Call pattern: `setupVacuumScenario(name, harness, ctx)` — identical to the other
 * five scenario group files; `harness` is the PhysicsHarness and `ctx = { N, mid,
 * midF }`. (Was previously declared `(name, ctx)` with a stray `this`/`harness`
 * mix — a refactor inconsistency that threw a ReferenceError on the MockBridge
 * fallback path; fixed 2026-06-05, health audit §A.4.)
 * Returns true if handled, false otherwise.
 */

import { K_B } from '../../constants.js';
import {
    applyVacuumEnvironment,
    injectRadialEnvelope,
    injectParticleFull,
    injectDressedParticle,
    injectTriad,
    injectPlanePacketX,
    injectTransversePacketX,
    configureFreeWaveTerms,
    configureUnlockedCompositeTerms,
} from './_helpers.js';

/**
 * @param {string} name  scenario identifier (must start with 's0-vacuum-')
 * @param {PhysicsHarness} harness  physics harness instance
 * @param {{N:number, mid:number, midF:number}} ctx
 * @returns {boolean} true iff handled
 */
export function setupVacuumScenario(name, harness, ctx) {
    if (!name.startsWith('s0-vacuum-')) return false;
    const { N, mid, midF, vox, sigma, band } = ctx;
    const sig = sigma;
    const mc = mid;
    applyVacuumEnvironment(harness, ctx);

    switch (name) {
        case 's0-vacuum-electron': {
            // One inert negative marker plus a selected inward radial vector
            // template. No charge coupling, mass pole, spinor, or electron
            // observable is present.
            configureFreeWaveTerms(harness, false);
            harness.injectParticle(mc, mc, mc, -1);
            const envR = vox(5);
            injectRadialEnvelope(harness, midF, midF, midF, -1, sig(2.5), K_B * 1.5,
                { radius: envR, minR2: 0.25 });
            return true;
        }

        case 's0-vacuum-muon':
        case 's0-vacuum-tau': {
            // Exact 1.2x/1.5x amplitude copies of the electron-labelled vector
            // template. No generation or mass distinction is encoded.
            configureFreeWaveTerms(harness, false);
            const boost = (name === 's0-vacuum-tau') ? 2.25 : 1.80;
            harness.injectParticle(mc, mc, mc, -1);
            const envR = vox(5);
            injectRadialEnvelope(harness, midF, midF, midF, -1, sig(2.5), K_B * boost,
                { radius: envR, minR2: 0.25 });
            return true;
        }

        case 's0-vacuum-photon': {
            // Mirror of s0-seed-photon — J_z-polarized Gaussian pulse
            // propagating +x. c = 1/√3 [THEOREM] from cubic-lattice CFL.
            // genesis=false (audit 2026-04-28): a free EM wave should not
            // spontaneously pair-produce.
            for (const [key, value] of [
                ['wave_propagation', true], ['coupling', false], ['damping', false],
                ['selective_damping', false], ['genesis', false],
                ['gauss_projection', true], ['forces', false], ['movement', false],
            ]) harness.setToggle(key, value);
            injectPlanePacketX(harness, ctx, {
                x0: vox(8), sigmaX: sig(3), amp: K_B * 0.5, direction: +1,
            });
            return true;
        }

        case 's0-vacuum-w-boson': {
            // One inert positive marker and an anisotropic vector template;
            // there is no weak charge, chirality coupling, or W observable.
            configureFreeWaveTerms(harness, false);
            injectParticleFull(harness, mc, mc, mc, +1, { spin: +1 });
            injectRadialEnvelope(harness, mc, mc, mc, +1, sig(1.8), K_B * 1.6,
                { radius: vox(5), axisBias: [1.3, 1, 1] });
            return true;
        }

        case 's0-vacuum-z-boson': {
            // Unmanifested inward radial vector template; no neutral current,
            // mass pole, polarization representation, or Z observable.
            configureFreeWaveTerms(harness, false);
            injectRadialEnvelope(harness, mc, mc, mc, -1, sig(2.0), K_B * 1.8, { radius: vox(6) });
            return true;
        }

        case 's0-vacuum-higgs': {
            // Equal-component three-vector blob. It is not a scalar and has
            // no Higgs potential, mass pole, symmetry breaking, or decay.
            configureFreeWaveTerms(harness, false);
            const hSig = sig(2.0), hR = vox(6), hAmp = K_B * 1.2;
            for (let dz = -hR; dz <= hR; dz++)
            for (let dy = -hR; dy <= hR; dy++)
            for (let dx = -hR; dx <= hR; dx++) {
                const r2 = dx*dx + dy*dy + dz*dz;
                if (r2 === 0 || r2 > hR*hR) continue;
                const g = hAmp * Math.exp(-r2 / (2 * hSig * hSig));
                if (g < 1e-3) continue;
                const iso = g / Math.sqrt(3);
                harness.injectFlux(mc+dx, mc+dy, mc+dz, iso, iso, iso);
            }
            return true;
        }

        case 's0-vacuum-proton': {
            configureUnlockedCompositeTerms(harness);
            // Unlocked 3-site selected-color candidate. Stability is measured;
            // proton identity is not encoded by this initialization.
            const bR = vox(4);
            injectTriad(harness, mc, mc, mc, [+1, +1, -1], [1, 2, 3], bR, false);
            return true;
        }

        case 's0-vacuum-neutron': {
            configureUnlockedCompositeTerms(harness);
            // Same geometry with a different imposed polarity pattern.
            const bR = vox(4);
            injectTriad(harness, mc, mc, mc, [+1, -1, -1], [1, 2, 3], bR, false);
            return true;
        }

        case 's0-vacuum-pion-charged': {
            configureUnlockedCompositeTerms(harness);
            // Unlocked opposite-polarity selected-color pair.
            const sp = vox(4);
            const hf = Math.floor(sp / 2);
            injectDressedParticle(harness, mc + hf, mc, mc, +1, +1, 1, 2, K_B * 0.5, false);
            injectDressedParticle(harness, mc - hf, mc, mc, -1, -1, 2, 2, K_B * 0.5, false);
            return true;
        }

        case 's0-vacuum-electron-neutrino':
        case 's0-vacuum-muon-neutrino':
        case 's0-vacuum-tau-neutrino': {
            // One neutral native packet at imposed amplitude multipliers
            // 1.0/1.3/1.6.  There is no flavor label, mass term, oscillation,
            // weak interaction, or neutrino-identifying observable here.
            const boost =
                name === 's0-vacuum-tau-neutrino'  ? 1.6 :
                name === 's0-vacuum-muon-neutrino' ? 1.3 : 1.0;
            configureFreeWaveTerms(harness, true);
            injectTransversePacketX(harness, ctx, {
                x0: vox(8), y0: midF, z0: midF, sigmaX: sig(2.5), sigmaT: Math.max(5, N / 5),
                amp: K_B * 0.3 * boost, direction: +1,
                carrierK: 2 * Math.PI / Math.max(8, N / 3),
            });
            return true;
        }

        case 's0-vacuum-pion-neutral': {
            configureUnlockedCompositeTerms(harness);
            // This is currently bit-identical to the charged-pion-labelled
            // setup; there is no neutral-pion-specific degree of freedom.
            const sp = vox(4);
            const hf = Math.floor(sp / 2);
            injectDressedParticle(harness, mc + hf, mc, mc, +1, +1, 1, 2, K_B * 0.5, false);
            injectDressedParticle(harness, mc - hf, mc, mc, -1, -1, 2, 2, K_B * 0.5, false);
            return true;
        }

        case 's0-vacuum-kaon-charged': {
            configureUnlockedCompositeTerms(harness);
            // Same selected pair geometry with an imposed 1.88 dressing boost;
            // no kaon mass or flavor mechanism is inferred.
            const sp = vox(4);
            const hf = Math.floor(sp / 2);
            const kBoost = 1.88;
            injectDressedParticle(harness, mc + hf, mc, mc, +1, +1, 1, 2, K_B * 0.5 * kBoost, false);
            injectDressedParticle(harness, mc - hf, mc, mc, -1, -1, 2, 2, K_B * 0.5 * kBoost, false);
            return true;
        }

        default:
            return false;
    }
}
