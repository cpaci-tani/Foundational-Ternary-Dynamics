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
 *
 * Pass 0a (bridge foundations, 2026-09-09): this class now also owns —
 *   - `_stats`/`_totals` (cosmic-pass-stats.js): this-tick / cumulative
 *     instrumentation, surfaced via getPassStats().
 *   - `_toggles` (14 rule-level physics gates; see `_freshToggles()`),
 *     re-seeded from the loaded scenario's own `_enableSubgrid` /
 *     `_stellarEvolution` / `_hawkingEvaporation` choices by
 *     `_syncRuleTogglesFromScenario()`.
 *   - `_eventLog` (bounded ring, cap 200): getEventLog().
 *   - `_comBaseline`: the centre-of-mass captured at the end of
 *     setupScenario, for the comDrift diagnostic.
 *   - Live setters (setGravityScale, setSofteningScale,
 *     setSpeedLimitFactor, setClockGain, setExpansionEnabled,
 *     setSphAlpha, setSphBeta, setAdaptiveSmoothing) that write private
 *     fields consumed with `?? default` fallbacks elsewhere — see
 *     getRuntimeParams() for the full current set.
 * See .superpowers/sdd/2026-09-09-scale5-cosmic-instrument/progress.md
 * for the pass record and the rulings made where the plan's text and the
 * actual tick order/code did not line up cleanly.
 */

import {
    OMEGA_LAMBDA, OMEGA_MATTER, H0_LATTICE, LATTICE_TO_SOLAR_MASS,
} from '../constants.js';

import { runCosmicScenario } from './cosmic-scenarios/index.js';
import { computeCosmicForces, BH_N_THRESHOLD } from './cosmic-physics.js';
import { postCosmicUpdates } from './cosmic-postupdates.js';
import { isGasType, SPH } from './cosmic-sph.js';
import { createCosmicPassStats } from './cosmic-pass-stats.js';

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
//   Pass 0a: `setClockGain()` overrides this default live (`this._clockGain
//   ?? COSMIC_CLOCK_GAIN`); `setExpansionEnabled(false)` skips the
//   Friedmann step entirely for a tick, freezing a/H/z (still diagnostics-
//   only, so this changes no N-body dynamics either).
// COSMIC_A_MAX: soft display cap on a(t). In the de Sitter future a grows
//   without bound; capping keeps the readout finite (H, z stay meaningful
//   at the floor). Purely cosmetic — no dynamical effect.
const COSMIC_A_INIT = 0.05;
const COSMIC_CLOCK_GAIN = 40.0;
const COSMIC_A_MAX = 1000.0;

// Event log cap (Pass 0a). A bounded ring so a long-running session's
// event log cannot grow without bound; oldest entries drop silently.
const EVENT_LOG_CAP = 200;

// Hubble rate from the flat-ΛCDM Friedmann equation at scale factor `a`.
// H(a) = H0·√(Ω_M·a⁻³ + Ω_Λ). Returns lattice-unit H (pre clock-gain).
function _friedmannH(a, H0, omegaM, omegaL) {
    const inv_a3 = 1.0 / (a * a * a);
    return H0 * Math.sqrt(omegaM * inv_a3 + omegaL);
}

// Uniform bin edges [lo, hi] split into nBins — shared by the gas-lab
// profile channel below (Task 5).
function _linspace(lo, hi, nBins) {
    const edges = new Float64Array(nBins + 1);
    const step = (hi - lo) / nBins;
    for (let i = 0; i <= nBins; i++) edges[i] = lo + i * step;
    return edges;
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
        // Scale 5 physics-term toggles (mirrors Scale 0's per-term dashboard
        // pattern). See _freshToggles() for the full 14-key registry.
        this._toggles = this._freshToggles();
        // Pass 0a: per-tick / cumulative instrumentation (cosmic-pass-stats.js).
        this._stats = createCosmicPassStats();
        this._totals = createCosmicPassStats();
        this._eventLog = [];
        this._comBaseline = null;
        this._pe = NaN;
        this._peAvailable = false;
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
            h: Math.cbrt(mass) * 0.2, sound: 0, du: 0, // Monaghan SPH state (cosmic-sph.js)
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
    // GAS-LAB AXIS PROFILE (Task 5; [IMPOSED effective gas dynamics])
    // ================================================================
    // Diagnostic-only binning of the gas population (GAS + NEBULA) along
    // one axis, for the "Gas profile" panel card. `edges` is a uniform
    // Float64Array of bin boundaries (length nBins+1, from _linspace).
    // `coordFn(b)` returns { coord, vRadial } for one gas body: `coord` is
    // the binning coordinate, `vRadial` the velocity component reported
    // per bin (radial for a spherical/cylindrical 'r' axis, vx for the
    // 'x' axis). Density uses a spherical-shell volume for axis 'r' and a
    // fixed reference-disc slab volume for axis 'x' — a display
    // convention for this imported, non-derivational lab, not a physical
    // normalization.
    _computeGasProfile(axis, edges, coordFn) {
        const T = CosmicMockBridge.TYPE;
        const nBins = edges.length - 1;
        const density = new Float64Array(nBins);
        const velocity = new Float64Array(nBins);
        const velCount = new Int32Array(nBins);
        let thermal = 0, kinetic = 0, gasCount = 0;
        const binWidth = (edges[nBins] - edges[0]) / nBins;
        const crossSectionArea = Math.PI * (this._boxSize / 4) * (this._boxSize / 4);

        for (const b of this._bodies) {
            if (!isGasType(b.type, T)) continue;
            gasCount++;
            thermal += b.mass * (b.internal_energy || 0);
            kinetic += 0.5 * b.mass * (b.vx * b.vx + b.vy * b.vy + b.vz * b.vz);

            const { coord, vRadial } = coordFn(b);
            if (!(coord >= edges[0]) || coord >= edges[nBins]) continue;
            let idx = Math.floor((coord - edges[0]) / binWidth);
            if (idx < 0) idx = 0;
            if (idx >= nBins) idx = nBins - 1;

            const binVolume = (axis === 'r')
                ? (4 / 3) * Math.PI * (Math.pow(edges[idx + 1], 3) - Math.pow(edges[idx], 3))
                : crossSectionArea * binWidth;
            density[idx] += b.mass / binVolume;
            velocity[idx] += vRadial;
            velCount[idx]++;
        }
        for (let i = 0; i < nBins; i++) {
            if (velCount[i] > 0) velocity[i] /= velCount[i];
        }
        return { axis, edges: Float64Array.from(edges), density, velocity, thermal, kinetic, gasCount };
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
    // PASS 0a — TOGGLE REGISTRY / RULE-TOGGLE SYNC
    // ================================================================
    // 14 bridge rule-toggles, each read at its call site with the "absent
    // key means on" convention (`this._toggles?.<key> !== false`). A
    // thorough audit of cosmic-physics.js/cosmic-postupdates.js/
    // cosmic-sph.js found exactly this many genuinely distinct
    // phenomenological rule blocks; see the SDD ledger for the count
    // ruling against the plan's aspirational "16 bridge rule-toggles"
    // Surface-Budget figure (which is a target for the FULL Pass 0-D
    // plan, not a literal per-pass enumeration).
    _freshToggles() {
        return {
            sph_monaghan: false,
            gas_cooling: true,
            radiation_pressure: true,
            tidal_stretch: true,
            tidal_disruption: true,
            legacy_gas_repulsion: true,
            star_formation: true,
            bondi_accretion: true,
            horizon_absorption: true,
            mergers: true,
            emergent_black_holes: true,
            stellar_evolution: false,
            hawking_evaporation: false,
            speed_limit: true,
        };
    }

    /** Seed the rule toggles from the just-loaded scenario's OWN choices
     *  (_enableSubgrid / _stellarEvolution / _hawkingEvaporation), called
     *  AFTER runCosmicScenario returns so a scenario's setup function has
     *  already run. Never touches sph_monaghan — the three gas-laboratory
     *  scenarios (cosmic-scenarios/gas.js) set that directly on
     *  `this._toggles` during their own setup and must survive this sync
     *  unmodified (cosmic-sph.node.test.mjs:404-415, M9). */
    _syncRuleTogglesFromScenario() {
        const subgrid = !!this._enableSubgrid;
        this._toggles.gas_cooling = subgrid;
        this._toggles.radiation_pressure = subgrid;
        this._toggles.tidal_stretch = subgrid;
        this._toggles.legacy_gas_repulsion = subgrid;
        this._toggles.star_formation = subgrid;
        this._toggles.bondi_accretion = subgrid;
        // I3 fix: emergent_black_holes is AND-gated with _enableSubgrid in
        // postCosmicUpdates (Pass D, Ruling D-1), exactly like its five
        // siblings above -- it used to keep its _freshToggles() `true`
        // default here regardless of subgrid, so it was the only rule that
        // rendered CHECKED on the twelve _enableSubgrid = false scenarios
        // where it can never fire (a checked box that cannot fire is the
        // same class of problem as a wrong tooltip). Seed it the same way.
        this._toggles.emergent_black_holes = subgrid;
        this._toggles.stellar_evolution = !!this._stellarEvolution;
        this._toggles.hawking_evaporation = !!this._hawkingEvaporation;
        // horizon_absorption / mergers / tidal_disruption / speed_limit
        // are NOT scenario-conditional today (they run regardless of
        // _enableSubgrid in the existing code) so they keep their
        // _freshToggles() default here.
    }

    /** Centre of mass captured once, at the end of setupScenario, as the
     *  baseline for the comDrift diagnostic in getDiagnostics(). */
    _captureComBaseline() {
        let totalMass = 0, cx = 0, cy = 0, cz = 0;
        for (const b of this._bodies) {
            totalMass += b.mass;
            cx += b.mass * b.x; cy += b.mass * b.y; cz += b.mass * b.z;
        }
        this._comBaseline = totalMass > 0
            ? { x: cx / totalMass, y: cy / totalMass, z: cz / totalMass }
            : { x: 0, y: 0, z: 0 };
    }

    /** Append one entry to the bounded event log (cap EVENT_LOG_CAP).
     *  Called from cosmic-postupdates.js at each of the seven event
     *  sites named in the plan: star_formed, merger, supernova,
     *  evaporation, tidal_disruption, emergent_black_hole,
     *  horizon_absorption. */
    _pushEvent(kind, detail) {
        this._eventLog.push({ tick: this._tick, kind, detail });
        if (this._eventLog.length > EVENT_LOG_CAP) this._eventLog.shift();
    }

    /** Fold this tick's `_stats` into the cumulative `_totals`. Called
     *  once per tick from _postUpdates(), after postCosmicUpdates
     *  returns (so both cosmic-physics.js's and cosmic-postupdates.js's
     *  contributions for THIS tick are already in `_stats`). See
     *  cosmic-pass-stats.js's module header for which convention
     *  ("running sum" / "running max" / "latest-known snapshot") applies
     *  to which field. */
    _accumulateTotals() {
        const s = this._stats, t = this._totals;
        // Running sums (event counts + masses).
        t.starsFormed += s.starsFormed;
        t.bondiAccretedMass += s.bondiAccretedMass;
        t.horizonAbsorptions += s.horizonAbsorptions;
        t.horizonAbsorbedMass += s.horizonAbsorbedMass;
        t.emergentBHFormations += s.emergentBHFormations;
        t.mergers += s.mergers;
        t.gwMassLost += s.gwMassLost;
        t.tidalDisruptions += s.tidalDisruptions;
        t.tidalShedMass += s.tidalShedMass;
        t.supernovae += s.supernovae;
        t.ejectaMass += s.ejectaMass;
        t.evaporations += s.evaporations;
        t.hawkingMassLost += s.hawkingMassLost;
        t.speedLimitClamps += s.speedLimitClamps;
        t.bodiesCulled += s.bodiesCulled;
        t.coolingCount += s.coolingCount;

        // Running all-time maxima.
        if (s.maxRadiationAccel > t.maxRadiationAccel) t.maxRadiationAccel = s.maxRadiationAccel;
        if (s.maxTidalAccel > t.maxTidalAccel) t.maxTidalAccel = s.maxTidalAccel;
        if (s.coolingMax > t.coolingMax) t.coolingMax = s.coolingMax;
        if (s.hawkingMaxT > t.hawkingMaxT) t.hawkingMaxT = s.hawkingMaxT;
        if (s.speedLimitMaxFactor > t.speedLimitMaxFactor) t.speedLimitMaxFactor = s.speedLimitMaxFactor;
        if (s.sphPairCount > t.sphPairCount) t.sphPairCount = s.sphPairCount;
        if (s.neighborMax > t.neighborMax) t.neighborMax = s.neighborMax;
        if (s.hMax > t.hMax) t.hMax = s.hMax;
        if (s.rhoMax > t.rhoMax) t.rhoMax = s.rhoMax;
        if (s.pMax > t.pMax) t.pMax = s.pMax;
        if (s.cMax > t.cMax) t.cMax = s.cMax;
        if (s.emergentBHEnclosedMass > t.emergentBHEnclosedMass) t.emergentBHEnclosedMass = s.emergentBHEnclosedMass;

        // Latest-known snapshot (no lossless cumulative form for a mean,
        // a min extracted from a per-tick sample, or an enum).
        t.solverPath = s.solverPath;
        t.solverThreshold = s.solverThreshold;
        t.sphGasCount = s.sphGasCount;
        t.neighborMin = s.neighborMin; t.neighborMean = s.neighborMean;
        t.hMin = s.hMin; t.hMean = s.hMean;
        t.rhoMin = s.rhoMin;
        t.pMin = s.pMin;
        t.cMin = s.cMin;
        t.thermal = s.thermal; t.kinetic = s.kinetic;
        t.coolingMean = s.coolingMean;
        t.emergentBHVesc = s.emergentBHVesc;
        t.emergentBHThreshold = s.emergentBHThreshold;
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
        this._toggles = this._freshToggles();
        this._stats = createCosmicPassStats();
        this._totals = createCosmicPassStats();
        this._eventLog = [];
        this._pe = NaN;
        this._peAvailable = false;

        const rng = this._rng(42);
        const PI2 = Math.PI * 2;
        // Box-Muller for Gaussian random numbers (needed for z-dispersion)
        const randn = () => Math.sqrt(-2 * Math.log(rng() + 1e-10)) * Math.cos(PI2 * rng());

        const ctx = { T: CosmicMockBridge.TYPE, rng, randn, PI2 };
        runCosmicScenario.call(this, name, ctx);

        // Seed the rule toggles from what the scenario just set (0.6/Pass 0a).
        this._syncRuleTogglesFromScenario();
        // Baseline for the comDrift diagnostic.
        this._captureComBaseline();
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
        this._accumulateTotals();
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
        // Reset here so every scenario other than the three gas labs below
        // reports no profile (Task 5). getDiagnostics() falls back to null
        // via `?? null` before the first tick runs this method anyway.
        this._customProfiles = null;

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
                tel['Singularity Mass'] = (bhs[0].mass * LATTICE_TO_SOLAR_MASS).toFixed(1) + ' M⊙';
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
            tel['Core Density'] = ((M_core * LATTICE_TO_SOLAR_MASS) / (4 / 3 * Math.PI * 1000)).toExponential(2) + ' M⊙/lu³';
        } else if (name === 'cosmic-stellar-lifecycle') {
            const wd = this._bodies.filter(b => b.type === T.WHITE_DWARF || b.type === T.NEUTRON_STAR).length;
            const bh = this._bodies.filter(b => b.type === T.BLACK_HOLE).length;
            tel['Deceased Stars'] = wd;
            tel['Supernova Remnants (BH)'] = bh;
        } else if (name === 'cosmic-black-hole') {
            const bh = this._bodies.find(b => isBH(b.type));
            if (bh) {
                tel['BH Mass'] = (bh.mass * LATTICE_TO_SOLAR_MASS).toFixed(2) + ' M⊙';
                tel['Accretion Disk Lum'] = (bh.luminosity || 0).toExponential(2) + ' W';
            }
        } else if (name === 'cosmic-ftd-collapse') {
            const bh = this._bodies.find(b => isBH(b.type));
            if (bh) {
                tel['Status'] = 'Collapsed (Singularity Born)';
                tel['BH Mass'] = (bh.mass * LATTICE_TO_SOLAR_MASS).toFixed(1) + ' M⊙';
            } else {
                tel['Status'] = 'Pre-Collapse (Increasing Density)';
            }
        } else if (name === 'cosmic-web') {
            const nodes = this._bodies.filter(b => isBH(b.type)).length;
            const gas = this._bodies.filter(b => b.type === T.NEBULA).length;
            tel['Stable Anchor Nodes'] = nodes;
            tel['Filament Gas Clumps'] = gas;
        } else if (name === 'cosmic-gas-collapse') {
            const profile = this._computeGasProfile('r', _linspace(0, 40, 20), (b) => {
                const r = Math.sqrt(b.x * b.x + b.y * b.y + b.z * b.z);
                return { coord: r, vRadial: r > 1e-9 ? (b.x * b.vx + b.y * b.vy + b.z * b.vz) / r : 0 };
            });
            this._customProfiles = profile;
            tel['Gas'] = profile.gasCount;
            tel['Thermal E'] = profile.thermal.toExponential(2);
            tel['Kinetic E'] = profile.kinetic.toExponential(2);
        } else if (name === 'cosmic-gas-cloud-collision') {
            const profile = this._computeGasProfile('x', _linspace(-45, 45, 30), (b) => ({
                coord: b.x, vRadial: b.vx,
            }));
            this._customProfiles = profile;
            tel['Gas'] = profile.gasCount;
            tel['Thermal E'] = profile.thermal.toExponential(2);
            tel['Kinetic E'] = profile.kinetic.toExponential(2);
        } else if (name === 'cosmic-gas-rotating-disk') {
            // Cylindrical radius in the disk plane (x-z; y is height —
            // matches galaxies.js's disk convention), NOT the full 3D r.
            const profile = this._computeGasProfile('r', _linspace(0, 40, 20), (b) => {
                const r = Math.sqrt(b.x * b.x + b.z * b.z);
                return { coord: r, vRadial: r > 1e-9 ? (b.x * b.vx + b.z * b.vz) / r : 0 };
            });
            this._customProfiles = profile;
            tel['Gas'] = profile.gasCount;
            tel['Thermal E'] = profile.thermal.toExponential(2);
            tel['Kinetic E'] = profile.kinetic.toExponential(2);
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
        // Pass 0a: setExpansionEnabled(false) skips this step entirely
        // (freezing a/H/z); setClockGain(v) overrides the default GAIN.
        if (this._expansionEnabled ?? true) {
            this._stepFriedmann(dt * (this._clockGain ?? COSMIC_CLOCK_GAIN));
        }

        this._t_cosmic += dt;
        this._tick++;
    }

    run(nTicks) { for (let i = 0; i < nTicks; i++) this.tick(); }

    // ================================================================
    // DATA OUTPUT
    // ================================================================

    getCosmicData() {
        const n = this._bodies.length;
        // F-3 (audit 2026-05-27): reuse persistent typed-array buffers across
        // physics frames instead of allocating 9 fresh arrays sized N every
        // tick (~2.4 MB/s GC churn at N≈2600). The returned object is consumed
        // synchronously by renderer.update() and never retained across calls
        // (see scale5/controller.js), so a reused (next-frame-overwritten)
        // buffer set is safe. Buffers are reallocated only when N changes, so
        // their `.length` stays exactly n·{1,3} — bit-identical output values.
        const buf = (this._dataBuf && this._dataBuf.n === n)
            ? this._dataBuf
            : (this._dataBuf = {
                n,
                positions: new Float32Array(n * 3),
                types: new Int8Array(n),
                temperatures: new Float32Array(n),
                sizes: new Float32Array(n),
                // True body mass in lattice units (audit P0-7). `sizes` is a
                // radius-like field (b.radius || cbrt(mass)·…); the BH renderer
                // needs the actual mass to draw a Schwarzschild-inspired
                // radius proxy linear in M rather than ∝ M^(1/3).
                masses: new Float32Array(n),
                densities: new Float32Array(n),
                luminosities: new Float32Array(n),
                stretches: new Float32Array(n),
                ids: new Int32Array(n), // stable body IDs (survive index shifts)
                fuel_stages: new Int8Array(n),
                fuel_fractions: new Float32Array(n),
                // Pass B (colour-by / smoothing-length-circle overlays,
                // cosmic-renderer.js): velocities is the FULL 3-vector, not
                // just |v| — a speed-only buffer would need widening again
                // the moment a velocity-vector overlay (Pass A) wanted the
                // direction too, so the richer datum is packed once here;
                // the colour-by ramp derives magnitude on demand (N sqrt's
                // per frame at N~2600, negligible). smoothingLengths is the
                // SPH adaptive smoothing length `h` (0 for non-gas bodies).
                velocities: new Float32Array(n * 3),
                smoothingLengths: new Float32Array(n),
            });

        const positions = buf.positions;
        const types = buf.types;
        const temperatures = buf.temperatures;
        const sizes = buf.sizes;
        const masses = buf.masses;
        const densities = buf.densities;
        const luminosities = buf.luminosities;
        const stretches = buf.stretches;
        const ids = buf.ids;
        const fuel_stages = buf.fuel_stages;
        const fuel_fractions = buf.fuel_fractions;
        const velocities = buf.velocities;
        const smoothingLengths = buf.smoothingLengths;

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
            velocities[i * 3] = b.vx; velocities[i * 3 + 1] = b.vy; velocities[i * 3 + 2] = b.vz;
            smoothingLengths[i] = b.h || 0;
        }
        return {
            positions, types, temperatures, sizes, masses, densities, luminosities,
            stretches, ids, fuel_stages, fuel_fractions, velocities, smoothingLengths,
            count: n,
        };
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
        let totalMass = 0, totalKE = 0, totalThermal = 0;
        const counts = new Array(9).fill(0);
        const massByType = new Array(9).fill(0);
        let dmMass = 0;
        // Pass 0a: momentum, centre-of-mass and angular-momentum
        // accumulators, folded into the SAME O(N) loop this method already
        // runs for totalMass/totalKE.
        let px = 0, py = 0, pz = 0;
        let cxSum = 0, cySum = 0, czSum = 0;
        let lx = 0, ly = 0, lz = 0;
        const TYPE = CosmicMockBridge.TYPE;
        for (const b of this._bodies) {
            const vx = b.vx, vy = b.vy, vz = b.vz;
            totalMass += b.mass;
            totalKE += 0.5 * b.mass * (vx * vx + vy * vy + vz * vz);
            const idx = b.type + 3;
            if (idx >= 0 && idx < 9) { counts[idx]++; massByType[idx] += b.mass; }
            if (b.type === TYPE.DARK_MATTER) dmMass += b.mass;
            // Total thermal (internal) energy of gas bodies — the SPH
            // energy-equation counterpart to totalKE above (audit: Task 4,
            // sph_monaghan). Zero when no gas bodies are present regardless
            // of the toggle, so this is a harmless addition on old scenarios.
            if (isGasType(b.type, TYPE)) totalThermal += b.mass * (b.internal_energy || 0);

            px += b.mass * vx; py += b.mass * vy; pz += b.mass * vz;
            cxSum += b.mass * b.x; cySum += b.mass * b.y; czSum += b.mass * b.z;
            lx += b.mass * (b.y * vz - b.z * vy);
            ly += b.mass * (b.z * vx - b.x * vz);
            lz += b.mass * (b.x * vy - b.y * vx);
        }
        const comX = totalMass > 0 ? cxSum / totalMass : 0;
        const comY = totalMass > 0 ? cySum / totalMass : 0;
        const comZ = totalMass > 0 ? czSum / totalMass : 0;
        let comDrift = 0;
        if (this._comBaseline) {
            const ddx = comX - this._comBaseline.x;
            const ddy = comY - this._comBaseline.y;
            const ddz = comZ - this._comBaseline.z;
            comDrift = Math.sqrt(ddx * ddx + ddy * ddy + ddz * ddz);
        }
        // Pass A: system radius — max distance from the instantaneous centre
        // of mass to any body, for the Gravity & Dynamics diagnostics row.
        // comX/comY/comZ above are only known once the loop over
        // this._bodies above has finished, so this is a genuine second O(N)
        // pass (not folded into the first) — cheap next to the O(N^2)/
        // Barnes-Hut force solve already run this tick at these body counts.
        let systemRadius2 = 0;
        for (const b of this._bodies) {
            const dx = b.x - comX, dy = b.y - comY, dz = b.z - comZ;
            const d2 = dx * dx + dy * dy + dz * dz;
            if (d2 > systemRadius2) systemRadius2 = d2;
        }
        const systemRadius = Math.sqrt(systemRadius2);
        return {
            tick: this._tick, bodyCount: this._bodies.length,
            countsByType: counts, totalMass, totalKE, dmMass, totalThermal,
            // Live ΛCDM background (audit P0-9): _H and _a are integrated
            // each tick by _stepFriedmann, no longer the static H0/1.0.
            // hubbleParameter is the present (visual-clock) Hubble rate;
            // hubble0 keeps the H0 anchor available for reference.
            hubbleParameter: this._H, scaleFactor: this._a,
            redshift: this._z, hubble0: this._H0,
            omegaMatter: this._omegaM, omegaLambda: this._omegaL,
            customTelemetry: this._customTelemetry,
            toggles: { ...this._toggles },
            // Set by gas-laboratory scenarios (Task 5); absent otherwise.
            customProfiles: this._customProfiles ?? null,
            // Pass 0a additions below. `pe` is the SOFTENED potential
            // energy on the gravity kernel's own convention (same eps as
            // the force), NOT the unsoftened physical potential energy —
            // see cosmic-physics.js. NaN/peAvailable=false when the audit
            // was not requested (this._wantEnergyAudit falsy) or the
            // Barnes-Hut branch ran (no pairwise loop exists there).
            pe: this._pe ?? NaN,
            peAvailable: this._peAvailable ?? false,
            momentum: { x: px, y: py, z: pz },
            angMom: { x: lx, y: ly, z: lz },
            comX, comY, comZ,
            comDrift,
            systemRadius,
            massByType,
        };
    }

    setDt(dt) { this._dt = dt; }
    getDt() { return this._dt; }
    clear() { this._bodies = []; this._tick = 0; this._nextId = 0; }

    // ================================================================
    // PASS 0a — RUNTIME ACCESSORS
    // ================================================================

    /** Snapshot of every live-tunable runtime parameter, at its current
     *  effective value (the default when no live override is set). */
    getRuntimeParams() {
        return {
            dt: this._dt,
            softening: this._softening,
            boxSize: this._boxSize,
            gravityScale: this._gravityScale ?? 1,
            softeningScale: this._softeningScale ?? 1,
            speedLimitFactor: this._speedLimitFactor ?? 1,
            clockGain: this._clockGain ?? COSMIC_CLOCK_GAIN,
            sphAlpha: this._sphAlpha ?? SPH.ALPHA,
            sphBeta: this._sphBeta ?? SPH.BETA,
            adaptiveSmoothing: this._adaptiveSmoothing !== false,
            expansionEnabled: this._expansionEnabled ?? true,
            scenarioName: this._scenarioName,
            enableSubgrid: !!this._enableSubgrid,
            stellarEvolution: !!this._stellarEvolution,
            hawkingEvaporation: !!this._hawkingEvaporation,
            solverPath: this._stats ? this._stats.solverPath : 'direct',
            bhThreshold: BH_N_THRESHOLD,
        };
    }

    /** This-tick stats (cosmic-pass-stats.js) plus the cumulative totals
     *  nested under `totals`. */
    getPassStats() {
        return { ...this._stats, totals: { ...this._totals } };
    }

    /** A shallow copy of the bounded event log (cap EVENT_LOG_CAP),
     *  oldest first. */
    getEventLog() {
        return this._eventLog.slice();
    }

    // ================================================================
    // PASS 0a — LIVE SETTERS (applied silently; no provenance banner,
    // no run-modified marking — owner decision, plan header).
    // ================================================================

    /** Multiplier on G_N in the gravity kernel (cosmic-physics.js:82-ish,
     *  `G_N * (this._gravityScale ?? 1)`). */
    setGravityScale(v) { this._gravityScale = v; }

    /** Multiplier on the per-type softening table (cosmic-physics.js's
     *  SoA-flatten loop). */
    setSofteningScale(v) { this._softeningScale = v; }

    /** Multiplier on the lattice speed of light used by the speed-limit
     *  clamp (cosmic-postupdates.js enforceCosmicSpeedLimit). */
    setSpeedLimitFactor(v) { this._speedLimitFactor = v; }

    /** Overrides COSMIC_CLOCK_GAIN for the Friedmann background step
     *  (display-only; never touches the N-body force kernel). */
    setClockGain(v) { this._clockGain = v; }

    /** Toggling this off freezes the ΛCDM background (a/H/z) without
     *  perturbing N-body dynamics (the Friedmann step is diagnostics-only
     *  even when enabled). */
    setExpansionEnabled(v) { this._expansionEnabled = !!v; }

    /** Overrides the frozen SPH.ALPHA artificial-viscosity coefficient
     *  read by cosmic-sph.js (the frozen SPH object itself is left
     *  alone — cosmic-sph.node.test.mjs:127-133 pins its values). */
    setSphAlpha(v) { this._sphAlpha = v; }

    /** Overrides the frozen SPH.BETA artificial-viscosity coefficient
     *  (see setSphAlpha). */
    setSphBeta(v) { this._sphBeta = v; }

    /** false freezes the SPH adaptive smoothing-length write-back
     *  (cosmic-sph.js); absent/true leaves it on. */
    setAdaptiveSmoothing(v) { this._adaptiveSmoothing = !!v; }

    // ================================================================
    // PHYSICS-TERM TOGGLES (mirrors the Scale 0 dashboard pattern)
    // ================================================================

    /** @param {string} key @param {boolean} value */
    setToggle(key, value) {
        if (!(key in this._toggles)) throw new Error('unknown toggle ' + key);
        this._toggles[key] = !!value;
    }

    /** @param {string} key @returns {boolean} */
    getToggle(key) {
        return this._toggles[key];
    }

    _rng(seed) {
        let s = seed;
        return () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
    }
}
