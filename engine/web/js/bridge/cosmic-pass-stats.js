/**
 * cosmic-pass-stats.js — per-tick / cumulative instrumentation for the
 * Scale-5 cosmic bridge (Pass 0a, bridge-side foundations).
 *
 * `createCosmicPassStats()` returns ONE flat, preallocated, mutable object
 * shape. `CosmicMockBridge` (mock-scale5.js) holds two independent
 * instances of it:
 *
 *   - `this._stats`  — THIS-TICK values. Reset once per tick (at the top
 *     of `computeCosmicForces`, cosmic-physics.js — see the ledger note
 *     below for why that is the reset point rather than the top of
 *     `postCosmicUpdates`), then written by both cosmic-physics.js
 *     (solver path, SPH/cooling/radiation/tidal-force stats) and
 *     cosmic-postupdates.js (event counts + masses) during the SAME tick.
 *   - `this._totals` — cumulative across the scenario's lifetime. Never
 *     has `.reset()` called on it during ticking; only re-created (fresh
 *     zeros) on the next `setupScenario()` call, same as `this._stats`.
 *     `CosmicMockBridge._accumulateTotals()` folds `_stats` into `_totals`
 *     once per tick, after `postCosmicUpdates` returns.
 *
 * Ledger note (deviation from the plan's literal wording, recorded in
 * .superpowers/sdd/2026-09-09-scale5-cosmic-instrument/progress.md): the
 * plan says "reset at the top of postCosmicUpdates". Because
 * computeCosmicForces runs BEFORE postCosmicUpdates within the SAME tick
 * (see CosmicMockBridge.tick()) and both write into this ONE shared
 * object, a reset placed at the top of postCosmicUpdates would erase the
 * solver-path/SPH/cooling/radiation/tidal-force fields that
 * computeCosmicForces had just written a few statements earlier THIS SAME
 * TICK — before any external caller ever gets to read them. The reset is
 * therefore placed at the top of computeCosmicForces instead (the first
 * thing to run in the tick), which delivers the same "cleared before this
 * tick's writes begin" semantics without the same-tick self-erasure.
 *
 * Every quantity here was already computed somewhere in the tick and
 * simply discarded before this pass; retaining it is an O(1) accumulation
 * inside a loop that already runs (no new O(N) or O(N^2) work is added by
 * this module itself).
 *
 * `_totals`' fields do not all mean "running sum" — a mean or an enum has
 * no lossless cumulative form. `CosmicMockBridge._accumulateTotals()`
 * picks, per field, the convention that makes sense:
 *   - COUNT / MASS event fields             -> running sum.
 *   - `...Max` / `...MaxT` / `...MaxFactor`  -> all-time running max.
 *   - MIN fields, gauges, means, enums       -> latest-known snapshot
 *                                              (mirrors this tick's value).
 * See the per-field comment at each `_accumulateTotals()` line
 * (mock-scale5.js) for which convention applies to which field.
 */
export function createCosmicPassStats() {
    return {
        // Gravity solver (cosmic-physics.js:161 direct vs :189 Barnes–Hut).
        solverPath: 'direct',       // 'direct' | 'barnes-hut'
        solverThreshold: 0,         // BH_N_THRESHOLD, the body-count gate

        // SPH pass (cosmic-sph.js computeSphForces).
        sphGasCount: 0,
        sphPairCount: 0,
        neighborMin: 0, neighborMean: 0, neighborMax: 0,
        hMin: 0, hMean: 0, hMax: 0,
        rhoMin: 0, rhoMax: 0,
        pMin: 0, pMax: 0,
        // Pass B: sound-speed range and gas-only thermal/kinetic energy,
        // all read off the SAME computeSphForces() return as the fields
        // above (see that function's Pass B header note).
        cMin: 0, cMax: 0,
        thermal: 0, kinetic: 0,

        // Sub-grid gas cooling (cosmic-physics.js, gas_cooling gate).
        coolingMean: 0, coolingMax: 0, coolingCount: 0,

        // Stellar radiation pressure on gas (radiation_pressure gate).
        maxRadiationAccel: 0,

        // Tidal spaghettification FORCE (tidal_stretch gate).
        maxTidalAccel: 0,

        // Subgrid star formation (star_formation gate).
        starsFormed: 0,

        // Bondi accretion (bondi_accretion gate).
        bondiAccretedMass: 0,

        // Event-horizon absorption (horizon_absorption gate).
        horizonAbsorptions: 0,
        horizonAbsorbedMass: 0,

        // Emergent BH formation (emergent_black_holes gate).
        emergentBHEnclosedMass: 0,
        emergentBHVesc: 0,
        emergentBHThreshold: 0,
        emergentBHFormations: 0,

        // BH-BH mergers (mergers gate).
        mergers: 0,
        gwMassLost: 0,

        // Tidal disruption / mass shedding (tidal_disruption gate).
        tidalDisruptions: 0,
        tidalShedMass: 0,

        // Stellar death-sequence ejecta (stellar_evolution gate).
        supernovae: 0,
        ejectaMass: 0,

        // Hawking evaporation (hawking_evaporation gate).
        evaporations: 0,
        hawkingMassLost: 0,
        hawkingMaxT: 0,

        // Speed-limit clamp (speed_limit gate).
        speedLimitClamps: 0,
        speedLimitMaxFactor: 0,

        // Cleanup (mass > 0.01 filter; not toggle-gated).
        bodiesCulled: 0,

        reset() {
            this.solverPath = 'direct';
            this.solverThreshold = 0;
            this.sphGasCount = 0; this.sphPairCount = 0;
            this.neighborMin = 0; this.neighborMean = 0; this.neighborMax = 0;
            this.hMin = 0; this.hMean = 0; this.hMax = 0;
            this.rhoMin = 0; this.rhoMax = 0;
            this.pMin = 0; this.pMax = 0;
            this.cMin = 0; this.cMax = 0;
            this.thermal = 0; this.kinetic = 0;
            this.coolingMean = 0; this.coolingMax = 0; this.coolingCount = 0;
            this.maxRadiationAccel = 0;
            this.maxTidalAccel = 0;
            this.starsFormed = 0;
            this.bondiAccretedMass = 0;
            this.horizonAbsorptions = 0; this.horizonAbsorbedMass = 0;
            this.emergentBHEnclosedMass = 0; this.emergentBHVesc = 0;
            this.emergentBHThreshold = 0; this.emergentBHFormations = 0;
            this.mergers = 0; this.gwMassLost = 0;
            this.tidalDisruptions = 0; this.tidalShedMass = 0;
            this.supernovae = 0; this.ejectaMass = 0;
            this.evaporations = 0; this.hawkingMassLost = 0; this.hawkingMaxT = 0;
            this.speedLimitClamps = 0; this.speedLimitMaxFactor = 0;
            this.bodiesCulled = 0;
        },
    };
}
