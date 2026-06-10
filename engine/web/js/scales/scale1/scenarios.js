/**
 * Scale 1 — PE Scenario Loader
 * ────────────────────────────────────────────────────────────────────
 *
 * Extracted verbatim from scales/scale1/controller.js (ticket S1-1).
 * Houses the big `switch (name)` that spawns particles for each
 * pe-* scenario: atoms, exotic atoms, hadrons, boson pairs, scattering,
 * and the micro-black-hole gravity demo.
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
 *     mSig, mOmg, mDel, RE, ALPHA_PE, soft2, orbitalV, BH_MASS,
 *     BH_TEST_MASS, BH_HORIZON_R, G_N, C_SPEED } }
 *
 *   The particle masses and orbitalV helper are built in the controller
 *   (to keep imports local to their users) and threaded through here.
 */

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
    forces: true,
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
        RE, ALPHA_PE, soft2, orbitalV,
        BH_MASS, BH_TEST_MASS, BH_HORIZON_R, G_N, C_SPEED
    } = constants;

    switch (name) {

        // -- Hydrogen: locked proton + orbiting electron ─────────────
        case 'pe-hydrogen': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // -- Helium: locked He nucleus (2p+2n), 2 orbiting electrons ─
        case 'pe-helium': {
            const r = 4;
            const v = orbitalV(me, r, 2); // Q=2
            bridge.peAddLockedParticle('proton', 1, 0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('proton', 1, -0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, 0, 0.3, 0, mn, RE);
            bridge.peAddLockedParticle('neutron', 0, 0, -0.3, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('electron', -1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }

        // -- Positronium: e+/e- orbiting common center of mass ───────
        case 'pe-positronium': {
            const r = 5;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r * sep / (4 * Math.PI * me * Math.pow(sep * sep + soft2, 1.5)));
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('positron', 1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }

        // -- Muonium: locked mu+ + orbiting electron ─────────────────
        case 'pe-muonium': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('mu_plus', 1, 0, 0, 0, mmu, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // -- Rutherford scattering: proton + electron approach ───────
        case 'pe-scattering': {
            const v_app = 0.005;
            bridge.peAddParticle('proton', 1, -15, 0, 0, v_app, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, 15, 3, 0, -v_app * 10, 0, 0, me, RE);
            break;
        }

        // -- Three-body: 2 locked protons + 1 electron ───────────────
        case 'pe-three-body': {
            const r = 8;
            const v = orbitalV(me, r, 2); // total Q=2
            bridge.peAddLockedParticle('proton', 1, -3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('proton', 1, 3, 0, 0, mp, RE);
            bridge.peAddParticle('electron', -1, 0, r, 0, v, 0, 0, me, RE);
            break;
        }

        // -- Deuteron: locked (p+n) + orbiting electron ──────────────
        case 'pe-deuteron': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('proton', 1, 0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, -0.3, 0, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // ── Lepton scenarios ────────────────────────────────────────

        // -- True muonium: mu+/mu- bound state ───────────────────────
        case 'pe-true-muonium': {
            const r = 3;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r * sep / (4 * Math.PI * mmu * Math.pow(sep * sep + soft2, 1.5)));
            bridge.peAddParticle('antimuon', 1, r, 0, 0, 0, v, 0, mmu, RE);
            bridge.peAddParticle('muon', -1, -r, 0, 0, 0, -v, 0, mmu, RE);
            break;
        }

        // -- Tauonium: tau+/tau- bound state (tight orbit) ───────────
        case 'pe-tauonium': {
            const r = 2;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r * sep / (4 * Math.PI * mtau * Math.pow(sep * sep + soft2, 1.5)));
            bridge.peAddParticle('antitau', 1, r, 0, 0, 0, v, 0, mtau, RE);
            bridge.peAddParticle('tau', -1, -r, 0, 0, 0, -v, 0, mtau, RE);
            break;
        }

        // -- Tauonic hydrogen: locked proton + orbiting tau- ─────────
        case 'pe-tau-atom': {
            const r = 1.5;
            const v = orbitalV(mtau, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('tau', -1, r, 0, 0, 0, v, 0, mtau, RE);
            break;
        }

        // ── Exotic atom scenarios ───────────────────────────────────

        // -- Pionic hydrogen: pi- orbiting locked proton ─────────────
        case 'pe-pionic-hydrogen': {
            const r = 4;
            const v = orbitalV(mpi, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('pion_minus', -1, r, 0, 0, 0, v, 0, mpi, RE);
            break;
        }

        // -- Kaonic hydrogen: K- orbiting locked proton ──────────────
        case 'pe-kaonic-hydrogen': {
            const r = 4;
            const v = orbitalV(mK, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('kaon_minus', -1, r, 0, 0, 0, v, 0, mK, RE);
            break;
        }

        // -- Sigma+ atom: electron orbiting locked Sigma+ ────────────
        case 'pe-sigma-plus-atom': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('sigma_plus', 1, 0, 0, 0, mSig, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // -- Protonium: p/p-bar orbiting common center of mass ───────
        case 'pe-antiprotonic-hydrogen': {
            const r = 3;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r * sep / (4 * Math.PI * mp * Math.pow(sep * sep + soft2, 1.5)));
            bridge.peAddParticle('proton', 1, r, 0, 0, 0, v, 0, mp, RE);
            bridge.peAddParticle('antiproton', -1, -r, 0, 0, 0, -v, 0, mp, RE);
            break;
        }

        // ── Hadron scenarios ────────────────────────────────────────

        // -- Pionium: pi+/pi- Coulomb bound state ────────────────────
        case 'pe-pion-orbit': {
            const r = 4;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r * sep / (4 * Math.PI * mpi * Math.pow(sep * sep + soft2, 1.5)));
            bridge.peAddParticle('pion_plus', 1, r, 0, 0, 0, v, 0, mpi, RE);
            bridge.peAddParticle('pion_minus', -1, -r, 0, 0, 0, -v, 0, mpi, RE);
            break;
        }

        // -- Kaonium: K+/K- Coulomb bound state ──────────────────────
        case 'pe-kaon-pair': {
            const r = 4;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r * sep / (4 * Math.PI * mK * Math.pow(sep * sep + soft2, 1.5)));
            bridge.peAddParticle('kaon_plus', 1, r, 0, 0, 0, v, 0, mK, RE);
            bridge.peAddParticle('kaon_minus', -1, -r, 0, 0, 0, -v, 0, mK, RE);
            break;
        }

        // -- Delta++ system: 2 locked +1 charges + 2 electrons ───────
        case 'pe-delta-system': {
            const r = 4;
            const v = orbitalV(me, r, 2);
            bridge.peAddLockedParticle('delta_pp_a', 1, 0.3, 0, 0, mDel / 2, RE);
            bridge.peAddLockedParticle('delta_pp_b', 1, -0.3, 0, 0, mDel / 2, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('electron', -1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }

        // -- Omega- scattering: locked Omega- + approaching positron ─
        case 'pe-omega-scattering': {
            const v_app = 0.004;
            bridge.peAddLockedParticle('omega_minus', -1, 0, 0, 0, mOmg, RE);
            bridge.peAddParticle('positron', 1, -15, 2, 0, v_app, 0, 0, me, RE);
            break;
        }

        // ── Nuclear scenarios ───────────────────────────────────────

        // -- Tritium: locked (p+n+n) nucleus + orbiting electron ─────
        case 'pe-tritium': {
            const r = 5;
            const v = orbitalV(me, r);
            bridge.peAddLockedParticle('proton', 1, 0, 0.3, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, 0.3, -0.2, 0, mn, RE);
            bridge.peAddLockedParticle('neutron', 0, -0.3, -0.2, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            break;
        }

        // -- Helion / He-3: locked (2p+n) + 2 orbiting electrons ─────
        case 'pe-helion': {
            const r = 4;
            const v = orbitalV(me, r, 2);
            bridge.peAddLockedParticle('proton', 1, 0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('proton', 1, -0.3, 0, 0, mp, RE);
            bridge.peAddLockedParticle('neutron', 0, 0, 0.3, 0, mn, RE);
            bridge.peAddParticle('electron', -1, r, 0, 0, 0, v, 0, me, RE);
            bridge.peAddParticle('electron', -1, -r, 0, 0, 0, -v, 0, me, RE);
            break;
        }

        // ── Boson scenarios ─────────────────────────────────────────

        // -- W+/W- pair in mutual Coulomb orbit ──────────────────────
        case 'pe-w-pair': {
            const r = 2;
            const sep = 2 * r;
            const v = Math.sqrt(ALPHA_PE * r * sep / (4 * Math.PI * mW * Math.pow(sep * sep + soft2, 1.5)));
            bridge.peAddParticle('w_plus', 1, r, 0, 0, 0, v, 0, mW, RE);
            bridge.peAddParticle('w_minus', -1, -r, 0, 0, 0, -v, 0, mW, RE);
            break;
        }

        // ── Scattering scenarios ────────────────────────────────────

        // -- Meson scattering: pi+ approaching locked proton (repulsive)
        case 'pe-meson-scattering': {
            const v_app = 0.006;
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('pion_plus', 1, -15, 2, 0, v_app, 0, 0, mpi, RE);
            break;
        }

        // -- Muon scattering: mu- approaching locked proton (attractive)
        case 'pe-muon-scattering': {
            const v_app = 0.008;
            bridge.peAddLockedParticle('proton', 1, 0, 0, 0, mp, RE);
            bridge.peAddParticle('muon', -1, -15, 2, 0, v_app, 0, 0, mmu, RE);
            break;
        }

        // ── Gravity scenarios ───────────────────────────────────────

        // -- Micro Black Hole: FTD lattice accretion demo ────────────
        // [SELECTION] M_BH=5000 MeV, radii, Hawking rate are pedagogical choices
        // [EMERGENT] C_SPEED cap creates inspiral zone at r < ~10
        case 'pe-micro-bh': {
            // BH locked at origin -- neutral, enormous mass
            bridge.peAddLockedParticle('neutron', 0, 0, 0, 0, BH_MASS, 0.5);

            // Gravity-only orbital velocity with Plummer softening
            const soft2_bh = 1.0;
            const gravOrbitalV = (r) =>
                Math.sqrt(G_N * BH_MASS * r * r / Math.pow(r * r + soft2_bh, 1.5));

            // ZONE 1: Inspiral donors at r=8 (v_circ > C_SPEED -- will spiral in)
            const r_fall = 8, v_fall = 0.45;
            const angles_fall = [0, Math.PI / 2, Math.PI, 3 * Math.PI / 2];
            for (const a of angles_fall) {
                bridge.peAddParticle('neutron', 0,
                    r_fall * Math.cos(a), 0, r_fall * Math.sin(a),
                    -v_fall * Math.sin(a), 0, v_fall * Math.cos(a),
                    BH_TEST_MASS, 0.1);
            }

            // ZONE 2: Accretion ring at r=16 (v_circ < C_SPEED -- stable orbits)
            const r_ring = 16;
            const v_ring = Math.min(gravOrbitalV(r_ring) * 0.92, C_SPEED * 0.92);
            const nRing = 8;
            for (let i = 0; i < nRing; i++) {
                const a = (i / nRing) * 2 * Math.PI;
                bridge.peAddParticle('neutron', 0,
                    r_ring * Math.cos(a), 0, r_ring * Math.sin(a),
                    -v_ring * Math.sin(a), 0, v_ring * Math.cos(a),
                    BH_TEST_MASS, 0.1);
            }

            // ZONE 3: Far escapers at r=26 (slightly super-circular)
            const r_far = 26;
            const v_far = gravOrbitalV(r_far) * 1.05;
            bridge.peAddParticle('neutron', 0,
                r_far, 0, 0, 0, 0, v_far, BH_TEST_MASS, 0.1);
            bridge.peAddParticle('neutron', 0,
                -r_far, 0, 0, 0, 0, -v_far, BH_TEST_MASS, 0.1);

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
