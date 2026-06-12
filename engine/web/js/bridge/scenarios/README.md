# engine/web/js/bridge/scenarios — Scenario library

**Purpose.** Scale-0 scenarios partitioned into 6 prefix-named domains.
Each scenario is a setup function that injects particles + flux into the
bridge to seed an initial condition.

> **Architecture references:** the full Scale 0 subsystem (definition layers,
> lifecycle, contracts) is specified in
> [engine/web/docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md](../../../docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md).
> The broader cross-scale scenario map lives at
> [engine/SCENARIO_ARCHITECTURE.md](../../../../SCENARIO_ARCHITECTURE.md).

## Public API

`runSetupScenario(name, harness)` (from `index.js`) is the dispatcher; consumers
call this. The bridge calls it via `setupScenario(name)` (Scale 0
capability surface). When no harness is passed, the dispatcher builds a thin
`this`-bound fallback over the MockBridge.

## Internal structure

| Group | File | Domain |
|---|---|---|
| `flux-*` | `flux-scenarios.js` | Substrate physics: pulses, dipoles, standing waves, vortices, dual substrate, baryon, meson, cyclotron, screening, thermalization, foam |
| `light-*` | `light-scenarios.js` | EM dynamics: rainbow, dipole radiation, two-slit, photon race |
| `quantum-*` | `quantum-scenarios.js` | QM phenomena: Born rule, double slit, eraser, tunnel, well, entanglement, Aharonov-Bohm, Casimir, Zeno |
| `s0-vacuum-*` | `vacuum-scenarios.js` | Single particle in vacuum: leptons, neutrinos, gauge bosons, baryons, mesons |
| `s0-seed-*` | `s0-seed-scenarios.js` | SM particles: quarks, bosons, atoms, molecules, gauge / topological, gravity, observers, emergent clusters |
| `s0-field-*` | `s0-field-scenarios.js` | Field configurations: plane waves, uniform E/B, photon pulse, FTD-0253 spacetime forcing, dipoles, vortex |

Re-derive inventory counts from source via `tests/scenario-parity.spec.js`;
it is the drift guard.

Plus:
- `index.js` — dispatcher (chains the 6 group files in prefix order)
- `_helpers.js` — shared primitives (`injectRadialEnvelope`, `injectParticleFull`, `injectDressedParticle`, `injectTriad`, `applyVacuumEnvironment`, `TRIAD_ANGLES`)

## Dependencies

- **Imports from**: `../../constants.js`, `_helpers.js`
- **Imported by**: `../mock-bridge.js` (the dispatcher is registered to MockBridge's `setupScenario` capability; `bridge-init.js` is now a 42-LOC re-export shim post-Phase 2 split)
- **C++ mirror**: `engine/src/scenarios/<group>.cpp`

## Scenario contract

A scenario group file exports `setupXxxScenario(name, harness, ctx)`:

1. Returns `true` if it handled the scenario; `false` if the prefix didn't match.
2. Throws on a known prefix with malformed scenario name (do not silently fall through).
3. Receives a `harness` (2nd arg) exposing injection methods (`harness.injectParticle`, `harness.injectFlux`, `harness.injectWaveVel`); `ctx = {N, mid, midF}` (3rd arg) carries precomputed lattice-center params. On the legacy in-thread MockBridge fallback the body is `.call`'d with the bridge as `this`.
4. MUST use shared helpers from `_helpers.js` for radial envelopes and triad placement.
5. MUST mirror the C++ scenario body (positions, charges, locked flags). When drift is intentional (e.g., visual cue not matching strict physics), document it inline.

See [CONTRACTS.md §4](../../../../../CONTRACTS.md#4--scenario-dispatch-contract) for the cross-module contract.

## How to extend

### Adding a new scenario
1. Add a `case 'new-scenario-id':` to the relevant group file (matching the prefix).
2. Mirror the body in `engine/src/scenarios/<group>.cpp`.
3. Register in `engine/web/js/scales/scale0/scenario-registry.js`.
4. If non-default toggles are needed: add an entry to `SCALE0_SCENARIO_OVERRIDES` in `engine/web/js/config/toggles.js`. Prefer this for declarative, user-visible defaults. Scenario bodies may still make local setup calls for a physical seed, but they should not hide persistent UI policy in bridge internals.

### Adding a new scenario group (new prefix)
1. Create `<prefix>-scenarios.js` exporting `setupXxxScenario(name, harness, ctx)`.
2. Add it to the dispatch chain in `index.js`.
3. Update [META_PROJECT_ATLAS.md](../../../../../META_PROJECT_ATLAS.md) §2 directory tree to list the new group.

## Anti-patterns (do not do this)

- Direct `this._toggles.foo = true` inside a scenario body for UI policy. Use `SCALE0_SCENARIO_OVERRIDES` so scenario defaults stay visible to the loader, dashboard, and tests.
- Bypass `_helpers.js` and re-implement triad geometry (drift risk vs C++ side)
- Forget to register the scenario in `scenario-registry.js` (the dashboard dropdown won't see it; the parity guard now also checks this)

## Related docs

- [engine/web/docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md](../../../docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md) — architecture
- [engine/SCENARIO_ARCHITECTURE.md](../../../../SCENARIO_ARCHITECTURE.md) — cross-scale scenario architecture
- [CONTRACTS.md §4](../../../../../CONTRACTS.md#4--scenario-dispatch-contract) — cross-module contract
- [engine/web/docs/USER_GUIDE.md](../../../docs/USER_GUIDE.md) §scenarios
- C++ mirror: `engine/src/scenarios/`
