/**
 * Scale-0 scenario dispatcher — JS parity mirror of engine/src/scenarios/*.
 *
 * LIVE dashboard loads use the C++ library via WASM (WasmBridge /
 * WasmBridgeProxy). This JS tree is kept for name-parity CI and as a
 * reference twin — not the production seed path.
 *
 * The 86-scenario switch body originally lived inline in MockBridge. It was
 * first lifted into this single file as a monolith (to de-risk the move),
 * then split into 6 prefix-based group files:
 *
 *   flux-scenarios.js      — 20 flux-* scenarios (Scale 0 substrate, QCD,
 *                             cyclotron, screening, triad, thermalization,
 *                             vacuum foam, experiment-suite cases)
 *   light-scenarios.js     — 4 light-* scenarios (rainbow, dipole, two-slit,
 *                             photon-race)
 *   quantum-scenarios.js   — 8 quantum-* scenarios (born, double-slit, tunnel,
 *                             well, entangle, aharonov-bohm, casimir, zeno)
 *   vacuum-scenarios.js    — s0-vacuum-* single-particle vacuum scenarios
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
 * Context object `ctx = { N, mid, midF, vox, sigma, band, … }` is precomputed
 * once here. Physics geometry uses the reference-lattice helpers so scenarios
 * keep the same physical size when L is changed (separate from visual scaling).
 *
 * Returns true iff a group handled `name`; false if unknown (never silent success).
 */

import { setupFluxScenario }    from './flux-scenarios.js';
import { setupLightScenario }   from './light-scenarios.js';
import { setupQuantumScenario } from './quantum-scenarios.js';
import { setupVacuumScenario }  from './vacuum-scenarios.js';
import { setupS0SeedScenario }  from './s0-seed-scenarios.js';
import { setupS0FieldScenario } from './s0-field-scenarios.js';
import { createPhysicsLatticeHelpers } from './physics-lattice.js';
import { createScenarioHarness, configureStaticSeedTerms } from './_helpers.js';

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

    // 'empty' is the null-control baseline: reset, then isolate every production
    // phase so the dashboard does not leave the full default stack armed on a
    // zero field. No group file owns the 'empty' prefix.
    if (name === 'empty') {
        const scenarioHarness = harness ?? createScenarioHarness(bridge);
        configureStaticSeedTerms(scenarioHarness);
        return true;
    }

    // Try each group in order. First matching prefix wins; stops immediately.
    const ctx = { N, mid, midF, ...createPhysicsLatticeHelpers(N) };
    const scenarioHarness = harness ?? createScenarioHarness(bridge);
    if (typeof scenarioHarness.initFluxGrid === 'function') scenarioHarness.initFluxGrid();
    if (setupFluxScenario(name, scenarioHarness, ctx))    return true;
    if (setupLightScenario(name, scenarioHarness, ctx))   return true;
    if (setupQuantumScenario(name, scenarioHarness, ctx)) return true;
    if (setupVacuumScenario(name, scenarioHarness, ctx))  return true;
    if (setupS0SeedScenario(name, scenarioHarness, ctx))  return true;
    if (setupS0FieldScenario(name, scenarioHarness, ctx)) return true;

    console.warn('[scenarios] unknown scenario id (JS parity mirror):', name);
    return false;
}
