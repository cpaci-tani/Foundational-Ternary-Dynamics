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
 * Call pattern: `setupVacuumScenario.call(mockBridge, name, ctx)`
 * where ctx = { N, mid, midF }.
 * Returns true if handled, false otherwise.
 */

import { K_B } from '../../constants.js';
import {
    applyVacuumEnvironment,
    injectRadialEnvelope,
    injectParticleFull,
    injectDressedParticle,
    injectTriad,
} from './_helpers.js';

/**
 * @param {string} name  scenario identifier (must start with 's0-vacuum-')
 * @param {{N:number, mid:number, midF:number}} ctx
 * @returns {boolean} true iff handled
 */
export function setupVacuumScenario(name, ctx) {
    if (!name.startsWith('s0-vacuum-')) return false;
    const { N, mid, midF } = ctx;
    const mc = Math.round(midF);

    this._initFluxGrid();
    applyVacuumEnvironment(this, ctx);

    switch (name) {
        case 's0-vacuum-electron': {
            // Mirror of s0-seed-electron — unit negative charge + radial-inward
            // flux envelope at scale K_B. Vacuum: nothing else in the lattice.
            this.injectParticle(mc, mc, mc, -1);
            const envR = Math.max(3, Math.floor(N / 6));
            injectRadialEnvelope(this, midF, midF, midF, -1, envR / 2, K_B * 1.5,
                { radius: envR, minR2: 0.25 });
            return true;
        }

        case 's0-vacuum-muon':
        case 's0-vacuum-tau': {
            // Mirror of s0-seed-{muon,tau} — same topology as electron, larger
            // amplitude. Mass ratios μ/e=207 and τ/e=3477 are [THEOREM] from
            // framework integers but have no spatial form; envelope amplitude
            // is a [SELECTION] visualization cue.
            const boost = (name === 's0-vacuum-tau') ? 2.25 : 1.80;
            this.injectParticle(mc, mc, mc, -1);
            const envR = Math.max(3, Math.floor(N / 6));
            injectRadialEnvelope(this, midF, midF, midF, -1, envR / 2, K_B * boost,
                { radius: envR, minR2: 0.25 });
            return true;
        }

        case 's0-vacuum-photon': {
            // Mirror of s0-seed-photon — J_z-polarized Gaussian pulse
            // propagating +x. c = 1/√3 [THEOREM] from cubic-lattice CFL.
            const sigma = 3;
            const pAmp = K_B * 2;
            const pStartX = Math.max(4, Math.floor(N / 4));
            const halfR = 8;
            for (let z = 0; z < N; z++)
            for (let y = 0; y < N; y++)
            for (let dx = -halfR; dx <= halfR; dx++) {
                const x = pStartX + dx;
                if (x < 0 || x >= N) continue;
                const dy = y - midF, dz = z - midF;
                const r2 = dx * dx + dy * dy + dz * dz;
                const g = pAmp * Math.exp(-r2 / (2 * sigma * sigma));
                if (g < 1e-6) continue;
                this._injectFlux(x, y, z, 0, 0, g);
                this._injectWaveVel(x, y, z, g, 0, 0);
            }
            return true;
        }

        case 's0-vacuum-w-boson': {
            // Mirror of s0-seed-w-boson — charged (s=+1) localized lump
            // with chirality bias on Jx (left-handed coupling).
            injectParticleFull(this, mc, mc, mc, +1, { spin: +1 });
            injectRadialEnvelope(this, mc, mc, mc, +1, 1.8, K_B * 1.6,
                { radius: 5, axisBias: [1.3, 1, 1] });
            return true;
        }

        case 's0-vacuum-z-boson': {
            // Mirror of s0-seed-z-boson — neutral, balanced inward envelope.
            injectRadialEnvelope(this, mc, mc, mc, -1, 2.0, K_B * 1.8, { radius: 6 });
            return true;
        }

        case 's0-vacuum-higgs': {
            // Mirror of s0-seed-higgs-boson — scalar isotropic flux lump,
            // no manifested core (Higgs is a field, not a state-particle).
            const hSig = 2.0, hR = 6, hAmp = K_B * 1.2;
            for (let dz = -hR; dz <= hR; dz++)
            for (let dy = -hR; dy <= hR; dy++)
            for (let dx = -hR; dx <= hR; dx++) {
                const r2 = dx*dx + dy*dy + dz*dz;
                if (r2 === 0 || r2 > hR*hR) continue;
                const g = hAmp * Math.exp(-r2 / (2 * hSig * hSig));
                if (g < 1e-3) continue;
                const iso = g / Math.sqrt(3);
                this._injectFlux(mc+dx, mc+dy, mc+dz, iso, iso, iso);
            }
            return true;
        }

        case 's0-vacuum-proton': {
            // Mirror of s0-seed-proton-l4 — 3 vertices on equilateral
            // triangle, charges [+1,+1,-1], colors [1,2,3].
            const bR = Math.max(2, Math.floor(N / 8));
            injectTriad(this, mc, mc, mc, [+1, +1, -1], [1, 2, 3], bR);
            return true;
        }

        case 's0-vacuum-neutron': {
            // Mirror of s0-seed-neutron — same triad geometry as proton,
            // charges [+1,-1,-1] (net 0).
            const bR = Math.max(2, Math.floor(N / 8));
            injectTriad(this, mc, mc, mc, [+1, -1, -1], [1, 2, 3], bR);
            return true;
        }

        case 's0-vacuum-pion-charged': {
            // Mirror of s0-seed-pion — quark-antiquark dipole on x-axis.
            const sp = Math.max(3, Math.floor(N / 8));
            const hf = Math.floor(sp / 2);
            injectDressedParticle(this, mc + hf, mc, mc, +1, +1, 1, 2, K_B * 0.5, true);
            injectDressedParticle(this, mc - hf, mc, mc, -1, -1, 1, 2, K_B * 0.5, true);
            return true;
        }

        // ── Net-new scenarios filled in by Task 3 ────────────────────
        case 's0-vacuum-electron-neutrino':
        case 's0-vacuum-muon-neutrino':
        case 's0-vacuum-tau-neutrino':
        case 's0-vacuum-pion-neutral':
        case 's0-vacuum-kaon-charged':
            return true;  // Will be filled in by Task 3.

        default:
            return false;
    }
}
