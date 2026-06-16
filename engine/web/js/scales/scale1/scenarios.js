/**
 * Scale 1 — PE Scenario Loader
 * ────────────────────────────────────────────────────────────────────
 *
 * Extracted verbatim from scales/scale1/controller.js (ticket S1-1).
 * Houses the big `switch (name)` that spawns particles for each
 * pe-* scenario: atoms, exotic atoms, hadrons, boson pairs, scattering,
 * and the micro-black-hole gravity demo.
 *
 * All scenarios use fully dynamic particles. Orbit ICs come from
 * peApplyEquilibriumOrbit (force balance at t=0), with applyEquilibriumOrbitBatch
 * for multi-body groups (helium, binaries, BH ring). Composite nuclei
 * approximate (Z, A) as a single massive charged particle.
 *
 * This is a pure move — no scenario-body logic was changed. Only the
 * surrounding orchestration (bridge init, toggle defaults, UI checkbox
 * sync, resetAllVisualState call) remains in the controller.
 *
 * CONTRACT:
 *   setupPEScenario(name, ctx) is called AFTER the controller has:
 *     1. Verified bridge.initPE exists
 *     2. Called ctx.resetAllVisualState()
 *     3. Called bridge.initPE()
 *     4. Reset PE defaults (coulomb on, damping/gravity off, softening 0.1)
 *     5. Synced the pe-coulomb/pe-gravity/pe-damping checkboxes
 *     6. Pushed the slider dt into the bridge
 *
 *   The function spawns particles for the named scenario. For scenarios
 *   that override the default physics (currently only pe-micro-bh), it
 *   flips bridge toggles and updates checkboxes inline, and returns a
 *   BH-state hint the controller uses to arm Hawking emission.
 *
 *   Return value:
 *     { bhActive: boolean, bhHorizonR: number } — controller applies this
 *     to its module-level state. For non-BH scenarios: { bhActive: false }.
 *
 * CTX object:
 *   { bridge, viewport, constants: { me, mp, mmu, mn, mpi, mK, mtau, mW,
 *     mSig, mOmg, mDel, RE, BH_MASS, BH_TEST_MASS, BH_HORIZON_R, G_PE, C_SPEED } }
 *
 *   Initial orbit speeds come from peApplyEquilibriumOrbit (force balance at t=0).
 */

import {
    seedHydrogenLike,
    seedBinaryOrbit,
    seedAtomicIon,
    spawnCompositeNucleus,
    applyEquilibriumOrbitBatch,
} from './pe-dynamics.js';

const BASE_PHYSICS = Object.freeze({
    coulomb: true,
    gravity: false,
    damping: false,
    lorentz: false,
    exchange: false,
    strong: false,
    magnetic_dipole: false,
    spin_orbit: false,
    radiation: false,
    relativistic: false,
    relativistic_verlet: false,
    dt: 1.0,
    softening: 0.1,
});

const ATOMIC_OVERLAYS = Object.freeze({
    velocities: true,
    trails: true,
    efield: false,
    potential: true,
    gravityField: false,
    forces: true,
});

const SCATTERING_OVERLAYS = Object.freeze({
    velocities: true,
    trails: true,
    efield: true,
    potential: false,
    gravityField: false,
    forces: true,
});

const GRAVITY_OVERLAYS = Object.freeze({
    velocities: true,
    trails: true,
    efield: false,
    potential: false,
    gravityField: true,
    forceGravity: true,
    forceNet: true,
});

const CUSTOM_OVERLAYS = Object.freeze({
    velocities: false,
    trails: false,
    efield: false,
    potential: false,
    gravityField: false,
    forces: false,
});

const PRESET_OVERRIDES = Object.freeze({
    'pe-hydrogen-fine': {
        physics: { magnetic_dipole: true, spin_orbit: true },
        overlays: { ...ATOMIC_OVERLAYS, forces: true, forceNet: true },
    },
    'pe-scattering':          { overlays: SCATTERING_OVERLAYS },
    'pe-omega-scattering':    { overlays: SCATTERING_OVERLAYS },
    'pe-meson-scattering':    { overlays: SCATTERING_OVERLAYS },
    'pe-muon-scattering':     { overlays: SCATTERING_OVERLAYS },
    'pe-three-body':          { overlays: { ...ATOMIC_OVERLAYS, efield: true } },
    'pe-w-pair':              { physics: { relativistic: true, relativistic_verlet: true } },
    'pe-micro-bh': {
        physics: {
            coulomb: false,
            gravity: true,
            damping: false,
            softening: 1.0,
        },
        overlays: GRAVITY_OVERLAYS,
    },
    'pe-custom': {
        overlays: CUSTOM_OVERLAYS,
    },
});

export function getPEScenarioPreset(name) {
    const override = PRESET_OVERRIDES[name] || {};
    return {
        physics: { ...BASE_PHYSICS, ...(override.physics || {}) },
        overlays: { ...ATOMIC_OVERLAYS, ...(override.overlays || {}) },
        status: override.status || 'Scale 1 continuous N-body demo',
    };
}


/**
 * Execute the scenario body for the given name.
 * @param {string} name - scenario identifier (pe-*)
 * @param {object} ctx  - shared context (see file header)
 * @returns {{bhActive: boolean, bhHorizonR?: number}}
 */
export function setupPEScenario(name, ctx) {
    const { bridge, viewport, constants } = ctx;
    const {
        me, mp, mmu, mn, mpi, mK, mtau, mW, mSig, mOmg, mDel,
        RE, BH_MASS, BH_TEST_MASS, BH_HORIZON_R, G_PE, C_SPEED
    } = constants;

    switch (name) {

        // -- Hydrogen: dynamic proton + electron (force-derived orbit) ─
        case 'pe-hydrogen': {
            seedHydrogenLike(bridge, {
                r: 5, nucleusCatalog: 'proton', nucleusCharge: 1, nucleusMass: mp,
                leptonCatalog: 'electron', leptonCharge: -1, leptonMass: me, RE,
            });
            break;
        }

        // -- Hydrogen + tilted spins: magnetic dipole / spin-orbit demo ─
        case 'pe-hydrogen-fine': {
            const { nucleusId, leptonId } = seedHydrogenLike(bridge, {
                r: 5, nucleusCatalog: 'proton', nucleusCharge: 1, nucleusMass: mp,
                leptonCatalog: 'electron', leptonCharge: -1, leptonMass: me, RE,
            });
            bridge.peSetSpinAxis?.(nucleusId, 0.707, 0.707, 0);
            bridge.peSetSpinAxis?.(leptonId, 0, 0.866, 0.5);
            break;
        }

        // -- Helium: composite nucleus (Z=2, A=4) + 2 electrons ───────
        case 'pe-helium': {
            seedAtomicIon(bridge, { Z: 2, A: 4, mp, me, RE, r: 4, electrons: 2 });
            break;
        }

        // -- Positronium: e+/e- mutual orbit (force-derived) ───────────
        case 'pe-positronium': {
            seedBinaryOrbit(bridge, {
                catalogA: 'electron', chargeA: -1, massA: me,
                catalogB: 'positron', chargeB: 1, massB: me,
                separation: 10, RE,
            });
            break;
        }

        // -- Muonium: dynamic μ⁺ + electron ──────────────────────────
        case 'pe-muonium': {
            seedHydrogenLike(bridge, {
                r: 5, nucleusCatalog: 'antimuon', nucleusCharge: 1, nucleusMass: mmu,
                leptonCatalog: 'electron', leptonCharge: -1, leptonMass: me, RE,
            });
            break;
        }

        // -- Rutherford scattering: proton + electron approach ───────
        case 'pe-scattering': {
            const v_app = 0.005;
            bridge.peAddParticle('proton', 1, -15, 0, 0, v_app, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, 15, 3, 0, -v_app * 10, 0, 0, me, RE);
            break;
        }

        // -- Three-body: composite Z=2 nucleus + 1 electron ───────────
        case 'pe-three-body': {
            const r = 8;
            spawnCompositeNucleus(bridge, 2, 2, mp, RE);
            const eid = bridge.peAddParticle('electron', -1, 0, r, 0, 0, 0, 0, me, RE);
            bridge.peApplyEquilibriumOrbit(eid, { tangent: [1, 0, 0] });
            break;
        }

        // -- Deuteron: composite (p+n) + electron ────────────────────
        case 'pe-deuteron': {
            seedHydrogenLike(bridge, {
                r: 5, nucleusCatalog: 'proton', nucleusCharge: 1, nucleusMass: mp + mn,
                leptonCatalog: 'electron', leptonCharge: -1, leptonMass: me, RE,
            });
            break;
        }

        // ── Lepton scenarios ────────────────────────────────────────

        // -- True muonium: μ⁺/μ⁻ mutual orbit ────────────────────────
        case 'pe-true-muonium': {
            seedBinaryOrbit(bridge, {
                catalogA: 'antimuon', chargeA: 1, massA: mmu,
                catalogB: 'muon', chargeB: -1, massB: mmu,
                separation: 6, RE,
            });
            break;
        }

        // -- Tauonium: τ⁺/τ⁻ mutual orbit ────────────────────────────
        case 'pe-tauonium': {
            seedBinaryOrbit(bridge, {
                catalogA: 'antitau', chargeA: 1, massA: mtau,
                catalogB: 'tau', chargeB: -1, massB: mtau,
                separation: 4, RE,
            });
            break;
        }

        // -- Tauonic hydrogen: dynamic proton + τ⁻ ───────────────────
        case 'pe-tau-atom': {
            seedHydrogenLike(bridge, {
                r: 1.5, nucleusCatalog: 'proton', nucleusCharge: 1, nucleusMass: mp,
                leptonCatalog: 'tau', leptonCharge: -1, leptonMass: mtau, RE,
            });
            break;
        }

        // ── Exotic atom scenarios ───────────────────────────────────

        // -- Pionic hydrogen ─────────────────────────────────────────
        case 'pe-pionic-hydrogen': {
            seedHydrogenLike(bridge, {
                r: 4, nucleusCatalog: 'proton', nucleusCharge: 1, nucleusMass: mp,
                leptonCatalog: 'pion_minus', leptonCharge: -1, leptonMass: mpi, RE,
            });
            break;
        }

        // -- Kaonic hydrogen ─────────────────────────────────────────
        case 'pe-kaonic-hydrogen': {
            seedHydrogenLike(bridge, {
                r: 4, nucleusCatalog: 'proton', nucleusCharge: 1, nucleusMass: mp,
                leptonCatalog: 'kaon_minus', leptonCharge: -1, leptonMass: mK, RE,
            });
            break;
        }

        // -- Sigma+ atom ───────────────────────────────────────────────
        case 'pe-sigma-plus-atom': {
            seedHydrogenLike(bridge, {
                r: 5, nucleusCatalog: 'sigma_plus', nucleusCharge: 1, nucleusMass: mSig,
                leptonCatalog: 'electron', leptonCharge: -1, leptonMass: me, RE,
            });
            break;
        }

        // -- Protonium: p/p̄ mutual orbit ─────────────────────────────
        case 'pe-antiprotonic-hydrogen': {
            seedBinaryOrbit(bridge, {
                catalogA: 'proton', chargeA: 1, massA: mp,
                catalogB: 'antiproton', chargeB: -1, massB: mp,
                separation: 6, RE,
            });
            break;
        }

        // ── Hadron scenarios ────────────────────────────────────────

        // -- Pionium: π⁺/π⁻ mutual orbit ─────────────────────────────
        case 'pe-pion-orbit': {
            seedBinaryOrbit(bridge, {
                catalogA: 'pion_plus', chargeA: 1, massA: mpi,
                catalogB: 'pion_minus', chargeB: -1, massB: mpi,
                separation: 8, RE,
            });
            break;
        }

        // -- Kaonium: K⁺/K⁻ mutual orbit ─────────────────────────────
        case 'pe-kaon-pair': {
            seedBinaryOrbit(bridge, {
                catalogA: 'kaon_plus', chargeA: 1, massA: mK,
                catalogB: 'kaon_minus', chargeB: -1, massB: mK,
                separation: 8, RE,
            });
            break;
        }

        // -- Delta++ system: dynamic Δ⁺⁺ + 2 electrons ───────────────
        case 'pe-delta-system': {
            seedAtomicIon(bridge, {
                Z: 2, A: 4, mp, me, RE, r: 4, electrons: 2,
                nucleusCatalog: 'delta_pp', nucleusMass: mDel,
            });
            break;
        }

        // -- Omega- scattering: dynamic Ω⁻ + approaching positron ───
        case 'pe-omega-scattering': {
            const v_app = 0.004;
            bridge.peAddParticle('omega_minus', -1, 0, 0, 0, 0, 0, 0, mOmg, RE);
            bridge.peAddParticle('positron', 1, -15, 2, 0, v_app, 0, 0, me, RE);
            break;
        }

        // ── Nuclear scenarios ───────────────────────────────────────

        // -- Tritium: composite (p+2n) + electron ─────────────────────
        case 'pe-tritium': {
            seedHydrogenLike(bridge, {
                r: 5, nucleusCatalog: 'proton', nucleusCharge: 1, nucleusMass: mp + 2 * mn,
                leptonCatalog: 'electron', leptonCharge: -1, leptonMass: me, RE,
            });
            break;
        }

        // -- Helion / He-3: composite (2p+n) + 2 electrons ─────────
        case 'pe-helion': {
            seedAtomicIon(bridge, { Z: 2, A: 3, mp, me, RE, r: 4, electrons: 2 });
            break;
        }

        // ── Boson scenarios ─────────────────────────────────────────

        // -- W+/W- pair: force-derived mutual orbit ──────────────────
        case 'pe-w-pair': {
            seedBinaryOrbit(bridge, {
                catalogA: 'w_plus', chargeA: 1, massA: mW,
                catalogB: 'w_minus', chargeB: -1, massB: mW,
                separation: 4, RE,
            });
            break;
        }

        // ── Scattering scenarios ────────────────────────────────────

        // -- Meson scattering: dynamic proton + approaching π⁺ ───────
        case 'pe-meson-scattering': {
            const v_app = 0.006;
            bridge.peAddParticle('proton', 1, 0, 0, 0, 0, 0, 0, mp, RE);
            bridge.peAddParticle('pion_plus', 1, -15, 2, 0, v_app, 0, 0, mpi, RE);
            break;
        }

        // -- Muon scattering: dynamic proton + approaching μ⁻ ────────
        case 'pe-muon-scattering': {
            const v_app = 0.008;
            bridge.peAddParticle('proton', 1, 0, 0, 0, 0, 0, 0, mp, RE);
            bridge.peAddParticle('muon', -1, -15, 2, 0, v_app, 0, 0, mmu, RE);
            break;
        }

        // ── Gravity scenarios ───────────────────────────────────────

        // -- Micro Black Hole: physical α_G gravity (FTD-0131) ─────
        // With G_PE = G_DERIVED, inspiral/accretion is unobservable on any
        // tick budget — dynamics are negligible. Scenario kept so gravity PE
        // and coupling telemetry expose the true ~1.75e-45 hierarchy.
        case 'pe-micro-bh': {
            // Super-massive dynamic anchor (moves negligibly under G_PE)
            bridge.peAddParticle('neutron', 0, 0, 0, 0, 0, 0, 0, BH_MASS, 0.5);

            const r_fall = 8, v_fall = 0.45;
            const angles_fall = [0, Math.PI / 2, Math.PI, 3 * Math.PI / 2];
            for (const a of angles_fall) {
                bridge.peAddParticle('neutron', 0,
                    r_fall * Math.cos(a), 0, r_fall * Math.sin(a),
                    -v_fall * Math.sin(a), 0, v_fall * Math.cos(a),
                    BH_TEST_MASS, 0.1);
            }

            const r_ring = 16;
            const nRing = 8;
            const orbitSpecs = [];
            for (let i = 0; i < nRing; i++) {
                const a = (i / nRing) * 2 * Math.PI;
                const pid = bridge.peAddParticle('neutron', 0,
                    r_ring * Math.cos(a), 0, r_ring * Math.sin(a),
                    0, 0, 0, BH_TEST_MASS, 0.1);
                orbitSpecs.push({
                    particleId: pid,
                    center: [0, 0, 0],
                    tangent: [-Math.sin(a), 0, Math.cos(a)],
                });
            }

            const r_far = 26;
            const farId = bridge.peAddParticle('neutron', 0, r_far, 0, 0, 0, 0, 0, BH_TEST_MASS, 0.1);
            orbitSpecs.push({ particleId: farId, tangent: [0, 0, 1] });
            const farId2 = bridge.peAddParticle('neutron', 0, -r_far, 0, 0, 0, 0, 0, BH_TEST_MASS, 0.1);
            orbitSpecs.push({ particleId: farId2, tangent: [0, 0, -1] });

            applyEquilibriumOrbitBatch(bridge, orbitSpecs);
            bridge.peScaleVelocity(farId, 1.05);
            bridge.peScaleVelocity(farId2, 1.05);

            // Tell the controller to activate Hawking emission + event horizon visual
            if (viewport && viewport.setEventHorizon) {
                viewport.setEventHorizon(true, BH_HORIZON_R);
            }
            return { bhActive: true, bhHorizonR: BH_HORIZON_R };
        }

        // -- Custom / Empty: user injects manually via Zoo or controls
        case 'pe-custom':
        default:
            break;
    }

    return { bhActive: false };
}
