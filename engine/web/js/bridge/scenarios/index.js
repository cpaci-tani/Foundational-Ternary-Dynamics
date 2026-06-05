/**
 * Scale-0 / Scale-1 / Scale-2 scenario dispatcher — MockBridge side.
 *
 * Wave 3 tickets 8-13 of the large-file refactor (docs/SPEC_REFACTOR_LARGE_FILES.md §4).
 *
 * The 86-scenario switch body originally lived inline in MockBridge. It was
 * first lifted into this single file as a monolith (to de-risk the move),
 * then split into 5 prefix-based group files:
 *
 *   flux-scenarios.js      — 20 flux-* scenarios (Scale 0 substrate, QCD,
 *                             cyclotron, screening, triad, thermalization,
 *                             vacuum foam, experiment-suite cases)
 *   light-scenarios.js     — 4 light-* scenarios (rainbow, dipole, two-slit,
 *                             photon-race)
 *   quantum-scenarios.js   — 8 quantum-* scenarios (born, double-slit, tunnel,
 *                             well, entangle, aharonov-bohm, casimir, zeno)
 *   s0-seed-scenarios.js   — 49 s0-seed-* scenarios (SM particles, Moore
 *                             geometries, quarks, gauge bosons, gravity,
 *                             reference frame context/observer seeds)
 *   s0-field-scenarios.js  — 8 s0-field-* scenarios (plane/standing waves,
 *                             uniform E/B, electric/magnetic dipoles, vortex)
 *
 * Each group exports a `setupXxxScenario(name, ctx)` function that:
 *   - Returns false if the name prefix does not match (dispatcher falls through)
 *   - Returns true if it handled the scenario
 *
 * Context object `ctx = { N, mid, midF }` is precomputed once here and passed
 * to each group to avoid recomputing the lattice-center parameters.
 *
 * CONTRACT — `this` binding via `.call(bridgeInstance, name, ctx)` is
 * mandatory. Every `this.reset()`, `this._initFluxGrid()`,
 * `this.injectParticle(...)`, etc., inside group files binds to the
 * MockBridge instance.
 */

import { setupFluxScenario }    from './flux-scenarios.js';
import { setupLightScenario }   from './light-scenarios.js';
import { setupQuantumScenario } from './quantum-scenarios.js';
import { setupVacuumScenario }  from './vacuum-scenarios.js';
import { setupS0SeedScenario }  from './s0-seed-scenarios.js';
import { setupS0FieldScenario } from './s0-field-scenarios.js';

/**
 * Dispatcher: executes a scenario by name by trying each group in order.
 * Call via `runSetupScenario(name, harness)`.
 *
 * @param {string} name - scenario identifier (flux-*, light-*, quantum-*,
 *   s0-seed-*, s0-field-*, or 'empty')
 * @param {PhysicsHarness} harness - the active physics harness
 */
export function runSetupScenario(name, harness = null) {
    const bridge = harness?.bridge ?? this;
    const N = harness ? harness.getLatticeSize() : bridge.latticeSize;
    const mid = Math.floor(N / 2);
    // True float lattice center.  For even N this is N/2 - 0.5 (between two voxels).
    // Gaussian loops anchored at midF produce a distribution whose visual centroid
    // sits exactly at N/2 — matching the wireframe center — for ALL N.
    const midF = (N - 1) / 2;
    if (harness) harness.reset();
    else bridge.reset();

    // 'empty' is equivalent to "just reset" — handle before the prefix chain
    // so it short-circuits cleanly (no group file owns the 'empty' prefix).
    // Unknown names also fall through the whole chain and silently no-op,
    // which matches the pre-refactor behavior.
    if (name === 'empty') return;

    // Try each group in order. First matching prefix wins; stops immediately.
    const ctx = { N, mid, midF };
    const scenarioHarness = harness ?? {
        bridge,
        setToggle: (key, value) => bridge.setToggle?.(key, value),
        injectFlux: (x, y, z, fx, fy, fz) => bridge._injectFlux?.(x, y, z, fx, fy, fz),
        injectWaveVel: (x, y, z, vx, vy, vz) => bridge._injectWaveVel?.(x, y, z, vx, vy, vz),
        injectParticle: (x, y, z, state, opts = {}) => {
            bridge.injectParticle?.(x, y, z, state);
            const last = bridge._particles?.[bridge._particles.length - 1];
            if (!last) return null;
            if (Number.isFinite(opts.spin)) last.spin = opts.spin;
            if (Number.isFinite(opts.color)) last.color = opts.color;
            if (opts.locked) last.locked = true;
            return last;
        },
    };
    if (setupFluxScenario(name, scenarioHarness, ctx))    return;
    if (setupLightScenario(name, scenarioHarness, ctx))   return;
    if (setupQuantumScenario(name, scenarioHarness, ctx)) return;
    if (setupVacuumScenario(name, scenarioHarness, ctx))  return;
    if (setupS0SeedScenario(name, scenarioHarness, ctx))  return;
    if (setupS0FieldScenario(name, scenarioHarness, ctx)) return;
}
