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
            // genesis=false (audit 2026-04-28): a free EM wave should not
            // spontaneously pair-produce.
            this._toggles.genesis = false;
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

        case 's0-vacuum-electron-neutrino':
        case 's0-vacuum-muon-neutrino':
        case 's0-vacuum-tau-neutrino': {
            // Soft chirality-biased flux blob (no manifested core), with
            // per-flavor amplitude reflecting (m_ν / m_e) hierarchy. Masses
            // themselves are [OPEN] in FTD; amplitudes are [SELECTION].
            //
            //   ν_e : 1.0× baseline   (lightest; m_νe upper bound ≪ m_νμ)
            //   ν_μ : 1.3×            (suggests slight mass excess)
            //   ν_τ : 1.6×            (heaviest of the three)
            //
            // All three remain well below K_GENESIS so no spurious genesis.
            const boost =
                name === 's0-vacuum-tau-neutrino'  ? 1.6 :
                name === 's0-vacuum-muon-neutrino' ? 1.3 : 1.0;
            const sig = 2, eR = 6;
            for (let dz2 = -eR; dz2 <= eR; dz2++)
            for (let dy2 = -eR; dy2 <= eR; dy2++)
            for (let dx2 = -eR; dx2 <= eR; dx2++) {
                const r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
                if (r22 > eR*eR) continue;
                const gg = K_B * 0.3 * boost * Math.exp(-r22 / (2 * sig * sig));
                if (gg < 0.001) continue;
                this._injectFlux(mc+dx2, mc+dy2, mc+dz2, gg*0.55, gg*0.45, 0);
            }
            return true;
        }

        case 's0-vacuum-pion-neutral': {
            // π⁰: neutral meson, decays predominantly to 2γ (BR ≈ 98.8%).
            // Topology: charged-pion-style quark-antiquark dipole on x-axis,
            // BUT both vertices carry s=0 (void core). Flux dressing remains
            // ±-paired so the meson has zero net charge but nontrivial flux
            // structure that couples to the EM channel via Gauss constraint.
            //
            // Mass m_π0 = 135.0 MeV (vs m_π± = 139.6 MeV); the small splitting
            // is [OPEN] in FTD; here we use the charged-pion amplitude.
            const sp = Math.max(3, Math.floor(N / 8));
            const hf = Math.floor(sp / 2);
            // Use injectDressedParticle with state=0 — this gives a void core
            // at each vertex with the radial flux envelope still applied.
            injectDressedParticle(this, mc + hf, mc, mc, 0, +1, 1, 2, K_B * 0.5, true);
            injectDressedParticle(this, mc - hf, mc, mc, 0, -1, 1, 2, K_B * 0.5, true);
            return true;
        }

        case 's0-vacuum-kaon-charged': {
            // K±: charged meson, m_K± = 493.7 MeV ≈ 3.54 × m_π±.
            // Topology: charged-pion-style dipole with elevated amplitude.
            // The amplitude scaling here is [PARAMETRIC] — it reproduces the
            // K-mass via the FTD-0110 cluster-size↔mass map (A = 2·√(m/m_e))
            // rather than deriving it. Mass itself is [OPEN] in FTD.
            //
            //   K-amp / π-amp ≈ √(m_K / m_π) ≈ 1.88
            const sp = Math.max(3, Math.floor(N / 8));
            const hf = Math.floor(sp / 2);
            const kBoost = 1.88;
            injectDressedParticle(this, mc + hf, mc, mc, +1, +1, 1, 2, K_B * 0.5 * kBoost, true);
            injectDressedParticle(this, mc - hf, mc, mc, -1, -1, 1, 2, K_B * 0.5 * kBoost, true);
            return true;
        }

        default:
            return false;
    }
}
