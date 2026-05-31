/**
 * CosmicMockBridge — JS-only N-body simulation for cosmic scale (Scale 5).
 *
 * Architecture follows Gadget-2 conventions:
 *   1. _computeForces():  gravity + hydro + external (NO mass changes)
 *   2. tick():            kick-drift-recompute-kick (Velocity Verlet)
 *   3. _postUpdates():    accretion, star formation, mergers, cleanup
 *
 * Softening: fixed per body type (energy-conserving).
 * Pairwise rule: eps = max(eps_i, eps_j) per Gadget-2 standard.
 *
 * Unit system: G = G_N = 0.01 (FTD ontic chain). Masses and distances
 * scaled so v_circular ~ O(1) for visual dynamics.
 *
 * Mass-unit note: body masses on this scale are stored in lattice units,
 * not solar masses. For UI display ("M = X M_sun") multiply by
 * `LATTICE_TO_SOLAR_MASS` (= 50, constants.js). As of the 2026-05-27
 * audit (P0-6) every `M☉`-labelled telemetry string in `_updateTelemetry`
 * applies this factor at the point of formatting; the raw `b.mass` values
 * elsewhere (forces, getCosmicData) remain in lattice units by design.
 *
 * Refactor note (MS5-1..3): scenario data generation moved to
 * ./cosmic-scenarios/, the force kernel to ./cosmic-physics.js, and
 * post-integration events to ./cosmic-postupdates.js. The class below
 * owns state, the tick schedule, telemetry, and the public API.
 */

import {
    OMEGA_LAMBDA, OMEGA_MATTER, H0_LATTICE, LATTICE_TO_SOLAR_MASS,
} from '../constants.js';

import { runCosmicScenario } from './cosmic-scenarios/index.js';
import { computeCosmicForces } from './cosmic-physics.js';
import { postCosmicUpdates } from './cosmic-postupdates.js';

// ── Friedmann / Hubble integration (audit P0-9, 2026-05-27) ─────────────
// Flat ΛCDM background: H(a)² = H0²·(Ω_M·a⁻³ + Ω_Λ), with a=1 "today".
// Ω_Λ = OMEGA_LAMBDA = 2/3 [PARAMETRIC — FTD-internal selection, NOT
// [THEOREM]; does not match Planck-2018 Ω_Λ ≈ 0.685; see constants.js
// :445-451]. Ω_M = OMEGA_MATTER = 1/3 [PARAMETRIC] is its complement and
// equals (DM_FRACTION + BARYON_FRACTION = 17/27 + 10/27 = 1)·OMEGA_MATTER,
// i.e. the full matter budget — the DM:baryon split partitions Ω_M but
// does not change the total that enters the Friedmann source.
//
// The integrator below replaces the historical static (_a = 1.0,
// _adot = 0.0) placeholder. a(t) is evolved forward by RK4 on
// da/dt = a·H(a) from an early-universe initial condition so the
// dashboard shows monotonic expansion with H decreasing toward the
// de Sitter floor H → H0·√Ω_Λ.
//
// COSMIC_A_INIT: scale factor at scenario start (a < 1 ⇒ room to expand;
//   z = 1/a − 1 = 19 at a = 0.05, a recognizable early-universe redshift).
// COSMIC_CLOCK_GAIN: [IMPOSED] display-only acceleration of the cosmic
//   clock. H0_LATTICE = 0.001 with per-tick dt ~ 0.01–0.05 gives a bare
//   H0·dt ~ 1e-5 — invisible on dashboard timescales. The cosmic-time
//   increment per tick is dtCosmic = dt · COSMIC_CLOCK_GAIN. GAIN = 40 is
//   calibrated so the universe crosses a = 1 ("today") around a few
//   hundred ticks and then stays in readable single/double digits for
//   thousands of ticks, with H smoothly relaxing to the de Sitter floor
//   H0·√Ω_Λ ≈ 8.2e-4 — i.e. the correct ΛCDM SHAPE of a(t) at a
//   comfortable viewing rate. This gain scales ONLY the cosmological
//   background clock (a, H, z diagnostics); it does NOT enter the N-body
//   force kernel or body kinematics, so scenario dynamics are unchanged.
// COSMIC_A_MAX: soft display cap on a(t). In the de Sitter future a grows
//   without bound; capping keeps the readout finite (H, z stay meaningful
//   at the floor). Purely cosmetic — no dynamical effect.
const COSMIC_A_INIT = 0.05;
const COSMIC_CLOCK_GAIN = 40.0;
const COSMIC_A_MAX = 1000.0;

// Hubble rate from the flat-ΛCDM Friedmann equation at scale factor `a`.
// H(a) = H0·√(Ω_M·a⁻³ + Ω_Λ). Returns lattice-unit H (pre clock-gain).
function _friedmannH(a, H0, omegaM, omegaL) {
    const inv_a3 = 1.0 / (a * a * a);
    return H0 * Math.sqrt(omegaM * inv_a3 + omegaL);
}

export class CosmicMockBridge {
    constructor() {
        this._bodies = [];
        this._tick = 0;
        this._nextId = 0;
        this._dt = 0.01;
        this._H0 = H0_LATTICE;
        // Flat-ΛCDM background state (audit P0-9). Initialized from the
        // Friedmann equation rather than left static; _resetFriedmann()
        // is the single source of truth so constructor + setupScenario agree.
        this._omegaM = OMEGA_MATTER;       // [PARAMETRIC] 1/3
        this._omegaL = OMEGA_LAMBDA;       // [PARAMETRIC] 2/3
        this._a = COSMIC_A_INIT; this._adot = 0.0; this._H = this._H0; this._z = 0.0;
        this._resetFriedmann();            // authoritative init of a/adot/H/z
        this._boxSize = 200;
        this._softening = 5.0; // base softening (used as fallback)
        this._gwEvents = [];
        this._t_cosmic = 0.0;
        this._enableSubgrid = false;
        this._stellarEvolution = false;
        this._hawkingEvaporation = false;
    }

    static TYPE = {
        DARK_ENERGY: -3, QUASAR: -2, BLACK_HOLE: -1,
        DARK_MATTER: 0, GAS: 1, STAR: 2,
        NEUTRON_STAR: 3, NEBULA: 4, WHITE_DWARF: 5
    };

    addBody(type, mass, x, y, z, vx = 0, vy = 0, vz = 0, temp = 0) {
        const id = this._nextId++;
        this._bodies.push({
            id, type, mass,
            x, y, z, vx, vy, vz,
            ax: 0, ay: 0, az: 0,
            temperature: temp,
            internal_energy: Math.max(temp * 0.001, 0.01),
            density: 0, pressure: 0,
            luminosity: type === 2 ? Math.pow(mass, 3.5) : 0,
            radius: Math.cbrt(mass) * 0.1,
            tidal_stretch: 0, // 0 = normal, grows toward 1.0 as star is disrupted
            original_mass: mass, // for tracking how much has been shed
            fuel_fraction: type === 2 ? 1.0 : 0, // 1.0 = full fuel, 0.0 = exhausted
            fuel_stage: 0,       // 0=H, 1=He, 2=C, 3=O, 4=Si, 5=Fe (dead)
            budget_income: 0,    // fusion energy per tick (for budget overlay)
            budget_expense: 0,   // gravitational + radiation drain per tick
            age: 0,              // ticks since creation
        });
        return id;
    }

    // Plummer sphere enclosed mass: M(r) = M * r^3 / (r^2 + a^2)^(3/2)
    _enclosedMass(r, M_total, rs) {
        return M_total * r * r * r / Math.pow(r * r + rs * rs, 1.5);
    }

    // ================================================================
    // FRIEDMANN / HUBBLE BACKGROUND (audit P0-9)
    // ================================================================
    // Flat ΛCDM: H(a)² = H0²·(Ω_M·a⁻³ + Ω_Λ); a(t) integrated by RK4 on
    // da/dt = a·H(a). Deterministic (no RNG). Diagnostics-only: the
    // background a/H/z are reported to telemetry but are intentionally NOT
    // fed back into the N-body force kernel, so enabling this changes no
    // scenario dynamics (see header note on COSMIC_CLOCK_GAIN).

    /** Reset the background to the early-universe IC. Single source of
     *  truth shared by the constructor and setupScenario(). */
    _resetFriedmann() {
        this._a = COSMIC_A_INIT;
        this._H = _friedmannH(this._a, this._H0, this._omegaM, this._omegaL);
        this._adot = this._a * this._H;
        this._z = 1.0 / this._a - 1.0;
    }

    /** Advance the scale factor by one tick using RK4 on da/dt = a·H(a).
     *  `dtCosmic` is the cosmic-clock increment for this tick. */
    _stepFriedmann(dtCosmic) {
        const H0 = this._H0, oM = this._omegaM, oL = this._omegaL;
        // da/dt = a · H(a)
        const f = (a) => a * _friedmannH(a, H0, oM, oL);
        const a0 = this._a;
        const k1 = f(a0);
        const k2 = f(a0 + 0.5 * dtCosmic * k1);
        const k3 = f(a0 + 0.5 * dtCosmic * k2);
        const k4 = f(a0 + dtCosmic * k3);
        let a = a0 + (dtCosmic / 6.0) * (k1 + 2 * k2 + 2 * k3 + k4);
        // Guard: a is strictly positive and monotonically increasing for
        // Ω_M, Ω_Λ ≥ 0; clamp the floor against any FP underflow, and the
        // ceiling with COSMIC_A_MAX (cosmetic — keeps the readout finite).
        if (!(a > 1e-6)) a = 1e-6;
        if (a > COSMIC_A_MAX) a = COSMIC_A_MAX;
        this._a = a;
        this._H = _friedmannH(a, H0, oM, oL);
        this._adot = a * this._H;
        this._z = 1.0 / a - 1.0;
    }

    // ================================================================
    // SCENARIOS — delegated to ./cosmic-scenarios/
    // ================================================================

    setupScenario(name) {
        this._bodies = [];
        this._nextId = 0;
        this._tick = 0;
        this._resetFriedmann();   // a(t)/H(t) back to early-universe IC (P0-9)
        this._gwEvents = [];
        this._t_cosmic = 0.0;
        this._scenarioName = name;
        this._customTelemetry = {};
        this._stellarEvolution = false;
        this._hawkingEvaporation = false;

        const rng = this._rng(42);
        const PI2 = Math.PI * 2;
        // Box-Muller for Gaussian random numbers (needed for z-dispersion)
        const randn = () => Math.sqrt(-2 * Math.log(rng() + 1e-10)) * Math.cos(PI2 * rng());

        const ctx = { T: CosmicMockBridge.TYPE, rng, randn, PI2 };
        runCosmicScenario.call(this, name, ctx);
    }

    // ================================================================
    // FORCE COMPUTATION — delegated to ./cosmic-physics.js
    // ================================================================

    _computeForces() {
        computeCosmicForces.call(this, CosmicMockBridge.TYPE);
    }

    // ================================================================
    // POST-INTEGRATION UPDATES — delegated to ./cosmic-postupdates.js
    // ================================================================

    _postUpdates() {
        postCosmicUpdates.call(this, CosmicMockBridge.TYPE);
    }

    // ================================================================
    // CUSTOM SCENARIO TELEMETRY
    // ================================================================

    _updateTelemetry() {
        const name = this._scenarioName;
        const T = CosmicMockBridge.TYPE;
        const isBH = (t) => t === T.BLACK_HOLE || t === T.QUASAR;
        const isStar = (t) => t === T.STAR || t === T.NEUTRON_STAR || t === T.WHITE_DWARF;

        const tel = {};

        if (name === 'cosmic-merger' || name === 'cosmic-binary-agn') {
            const bhs = this._bodies.filter(b => isBH(b.type)).sort((a, b) => b.mass - a.mass);
            if (bhs.length >= 2) {
                const dx = bhs[0].x - bhs[1].x, dy = bhs[0].y - bhs[1].y, dz = bhs[0].z - bhs[1].z;
                const sep = Math.sqrt(dx * dx + dy * dy + dz * dz);
                tel['Core Separation'] = sep.toFixed(2) + ' lu';
                if (name === 'cosmic-binary-agn') {
                    tel['Peak Jet Power'] = Math.max(bhs[0].luminosity || 0, bhs[1].luminosity || 0).toExponential(2) + ' EJ/s';
                }
            } else if (bhs.length === 1) {
                tel['Status'] = 'Merger Complete';
                tel['Singularity Mass'] = (bhs[0].mass * LATTICE_TO_SOLAR_MASS).toFixed(1) + ' M\u2299';
            }
        } else if (name === 'cosmic-cartwheel-collision') {
            const bhs = this._bodies.filter(b => isBH(b.type));
            if (bhs.length >= 2) {
                const dx = bhs[0].x - bhs[1].x, dy = bhs[0].y - bhs[1].y, dz = bhs[0].z - bhs[1].z;
                const sep = Math.sqrt(dx * dx + dy * dy + dz * dz);
                tel['Bullet Distance'] = sep.toFixed(1) + ' lu';
                tel['Impact Phase'] = (bhs[1].y > 0) ? 'Post-Collision' : 'Approach';
            }
        } else if (name === 'cosmic-globular-cluster') {
            let coreStars = 0;
            let M_core = 0;
            for (const b of this._bodies) {
                if (isStar(b.type)) {
                    if (b.x * b.x + b.y * b.y + b.z * b.z < 100) { // core radius squared
                        coreStars++;
                        M_core += b.mass;
                    }
                }
            }
            tel['Core Population (r<10)'] = coreStars;
            tel['Core Density'] = ((M_core * LATTICE_TO_SOLAR_MASS) / (4 / 3 * Math.PI * 1000)).toExponential(2) + ' M\u2299/lu\u00B3';
        } else if (name === 'cosmic-stellar-lifecycle') {
            const wd = this._bodies.filter(b => b.type === T.WHITE_DWARF || b.type === T.NEUTRON_STAR).length;
            const bh = this._bodies.filter(b => b.type === T.BLACK_HOLE).length;
            tel['Deceased Stars'] = wd;
            tel['Supernova Remnants (BH)'] = bh;
        } else if (name === 'cosmic-black-hole') {
            const bh = this._bodies.find(b => isBH(b.type));
            if (bh) {
                tel['BH Mass'] = (bh.mass * LATTICE_TO_SOLAR_MASS).toFixed(2) + ' M\u2299';
                tel['Accretion Disk Lum'] = (bh.luminosity || 0).toExponential(2) + ' W';
            }
        } else if (name === 'cosmic-ftd-collapse') {
            const bh = this._bodies.find(b => isBH(b.type));
            if (bh) {
                tel['Status'] = 'Collapsed (Singularity Born)';
                tel['BH Mass'] = (bh.mass * LATTICE_TO_SOLAR_MASS).toFixed(1) + ' M\u2299';
            } else {
                tel['Status'] = 'Pre-Collapse (Increasing Density)';
            }
        } else if (name === 'cosmic-web') {
            const nodes = this._bodies.filter(b => isBH(b.type)).length;
            const gas = this._bodies.filter(b => b.type === T.NEBULA).length;
            tel['Stable Anchor Nodes'] = nodes;
            tel['Filament Gas Clumps'] = gas;
        }

        this._customTelemetry = tel;
    }

    // ================================================================
    // TICK — Velocity Verlet (Gadget-2 kick-drift-kick)
    // ================================================================

    tick() {
        const n = this._bodies.length;
        if (n === 0) return;
        const dt = this._dt;

        // Step 1: half-kick with CURRENT accelerations
        for (const b of this._bodies) {
            b.vx += 0.5 * dt * b.ax;
            b.vy += 0.5 * dt * b.ay;
            b.vz += 0.5 * dt * b.az;
        }
        // Step 2: drift
        for (const b of this._bodies) {
            b.x += dt * b.vx;
            b.y += dt * b.vy;
            b.z += dt * b.vz;
        }
        // Step 3: recompute forces at NEW positions
        this._computeForces();
        // Step 4: second half-kick with FRESH forces
        for (const b of this._bodies) {
            b.vx += 0.5 * dt * b.ax;
            b.vy += 0.5 * dt * b.ay;
            b.vz += 0.5 * dt * b.az;
        }
        // Step 5: post-integration updates (mass changes, mergers, cleanup)
        this._postUpdates();

        // Step 6: advance the ΛCDM background a(t)/H(t) (audit P0-9).
        // dtCosmic = dt · GAIN (the visual-accelerated cosmic clock);
        // _friedmannH already carries H0, so H0 is NOT multiplied in here.
        // Diagnostics-only — does not perturb the N-body integration above.
        this._stepFriedmann(dt * COSMIC_CLOCK_GAIN);

        this._t_cosmic += dt;
        this._tick++;
    }

    run(nTicks) { for (let i = 0; i < nTicks; i++) this.tick(); }

    // ================================================================
    // DATA OUTPUT
    // ================================================================

    getCosmicData() {
        const n = this._bodies.length;
        const positions = new Float32Array(n * 3);
        const types = new Int8Array(n);
        const temperatures = new Float32Array(n);
        const sizes = new Float32Array(n);
        // True body mass in lattice units (audit P0-7). `sizes` is a
        // radius-like field (b.radius || cbrt(mass)·…); the BH renderer
        // needs the actual mass to draw a Schwarzschild horizon r_s = 2 G_N M
        // that is LINEAR in M rather than the historical ∝ M^(1/3).
        const masses = new Float32Array(n);
        const densities = new Float32Array(n);
        const luminosities = new Float32Array(n);
        const stretches = new Float32Array(n);
        const ids = new Int32Array(n); // stable body IDs (survive index shifts)
        const fuel_stages = new Int8Array(n);
        const fuel_fractions = new Float32Array(n);

        for (let i = 0; i < n; i++) {
            const b = this._bodies[i];
            positions[i * 3] = b.x; positions[i * 3 + 1] = b.y; positions[i * 3 + 2] = b.z;
            types[i] = b.type;
            ids[i] = b.id;
            const stretch = b.tidal_stretch || 0;
            temperatures[i] = b.temperature + stretch * 15000;
            // Radius override if present, else fallback to mass-based
            sizes[i] = b.radius || (Math.cbrt(b.mass) * (1 + stretch * 2));
            masses[i] = b.mass;
            densities[i] = b.density || 0.1;
            // Note: For BH, luminosity holds the Jet Intensity gauge!
            luminosities[i] = b.luminosity * (1 - stretch * 0.5);
            stretches[i] = stretch;
            fuel_stages[i] = b.fuel_stage || 0;
            fuel_fractions[i] = b.fuel_fraction != null ? b.fuel_fraction : 1.0;
        }
        return { positions, types, temperatures, sizes, masses, densities, luminosities, stretches, ids, fuel_stages, fuel_fractions, count: n };
    }

    cosmicInspectBody(id) {
        const b = this._bodies.find(x => x.id === id);
        if (!b) return null;
        return {
            id: b.id, type: b.type, mass: b.mass,
            x: b.x, y: b.y, z: b.z, vx: b.vx, vy: b.vy, vz: b.vz,
            speed: Math.sqrt(b.vx * b.vx + b.vy * b.vy + b.vz * b.vz),
            temperature: b.temperature || 0,
            luminosity: b.luminosity || 0,
            fuel_fraction: b.fuel_fraction != null ? b.fuel_fraction : 1.0,
            fuel_stage: b.fuel_stage || 0,
            age: b.age || 0,
            radius: b.radius || (Math.cbrt(b.mass) * (1 + (b.tidal_stretch || 0) * 2))
        };
    }

    getDiagnostics() {
        let totalMass = 0, totalKE = 0;
        const counts = new Array(9).fill(0);
        for (const b of this._bodies) {
            totalMass += b.mass;
            totalKE += 0.5 * b.mass * (b.vx * b.vx + b.vy * b.vy + b.vz * b.vz);
            const idx = b.type + 3;
            if (idx >= 0 && idx < 9) counts[idx]++;
        }
        return {
            tick: this._tick, bodyCount: this._bodies.length,
            countsByType: counts, totalMass, totalKE,
            // Live ΛCDM background (audit P0-9): _H and _a are integrated
            // each tick by _stepFriedmann, no longer the static H0/1.0.
            // hubbleParameter is the present (visual-clock) Hubble rate;
            // hubble0 keeps the H0 anchor available for reference.
            hubbleParameter: this._H, scaleFactor: this._a,
            redshift: this._z, hubble0: this._H0,
            omegaMatter: this._omegaM, omegaLambda: this._omegaL,
            customTelemetry: this._customTelemetry
        };
    }

    setDt(dt) { this._dt = dt; }
    getDt() { return this._dt; }
    clear() { this._bodies = []; this._tick = 0; this._nextId = 0; }

    _rng(seed) {
        let s = seed;
        return () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
    }
}
