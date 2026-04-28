# engine/web/js/bridge/scenarios — Scenario library

**Purpose.** 84+ Scale-0 scenarios partitioned into 5 prefix-named domains.
Each scenario is a setup function that injects particles + flux into the
bridge to seed an initial condition.

## Public API

`runSetupScenario(name, ctx)` (from `index.js`) is the dispatcher; consumers
call this. The bridge calls it via `setupScenario(name)` (Scale 0
capability surface).

## Internal structure

| Group | File | Count | Domain |
|---|---|---:|---|
| `flux-*` | `flux-scenarios.js` | 20 | Substrate physics: pulses, dipoles, standing waves, vortices, dual substrate, baryon, meson, cyclotron, screening, thermalization, foam |
| `light-*` | `light-scenarios.js` | 4 | EM dynamics: rainbow, dipole radiation, two-slit, photon race |
| `quantum-*` | `quantum-scenarios.js` | 8 | QM phenomena: Born rule, double slit, tunnel, well, entanglement, Aharonov-Bohm, Casimir, Zeno |
| `s0-seed-*` | `s0-seed-scenarios.js` | 49 | SM particles: leptons, quarks, bosons, baryons, atoms, molecules, gauge / topological, gravity, observers |
| `s0-field-*` | `s0-field-scenarios.js` | 8 | Field configurations: plane waves, uniform E/B, photon pulse, dipoles, vortex |

Plus:
- `index.js` — dispatcher (chains the 5 group files)
- `_helpers.js` — shared primitives (`injectRadialEnvelope`, `injectParticleFull`, `injectDressedParticle`, `injectTriad`, `TRIAD_ANGLES`)

## Dependencies

- **Imports from**: `../../constants.js`, `_helpers.js`
- **Imported by**: `../wasm-bridge-dag.js` (the dispatcher is registered to MockBridge's `setupScenario` capability)
- **C++ mirror**: `engine/src/scenarios/<group>.cpp` (see `engine/src/scenarios/README.md` if exists, or grep for the C++ counterparts)

## Scenario contract

A scenario group file exports `setupXxxScenario(name, ctx)`:

1. Returns `true` if it handled the scenario; `false` if the prefix didn't match.
2. Throws on a known prefix with malformed scenario name (do not silently fall through).
3. Called with `.call(this, ...)` so the body has access to bridge mutation methods (`this.injectParticle`, `this._injectFlux`).
4. MUST use shared helpers from `_helpers.js` for radial envelopes and triad placement.
5. MUST mirror the C++ scenario body (positions, charges, locked flags). When drift is intentional (e.g., visual cue not matching strict physics), document it inline.

See [CONTRACTS.md §4](../../../../CONTRACTS.md#4--scenario-dispatch-contract) for the full contract.

## How to extend

### Adding a new scenario
1. Add a `case 'new-scenario-id':` to the relevant group file (matching the prefix).
2. Mirror the body in `engine/src/scenarios/<group>.cpp`.
3. Register in `engine/web/js/scales/scale0/scenario-registry.js`.
4. If non-default toggles are needed: add an entry to `SCALE0_SCENARIO_OVERRIDES` in `engine/web/js/config/toggles.js`. **DO NOT** mutate `this._toggles` directly inside the scenario body — the loader resets defaults after `setupScenario` runs.

### Adding a new scenario group (new prefix)
1. Create `<prefix>-scenarios.js` exporting `setupXxxScenario(name, ctx)`.
2. Add it to the dispatch chain in `index.js`.
3. Update [META_PROJECT_ATLAS.md](../../../../META_PROJECT_ATLAS.md) §2 directory tree to list the new group.

## Anti-patterns (do not do this)

- Direct `this._toggles.foo = true` inside a scenario body (gets reset by `applyToggleDefaults`)
- Bypass `_helpers.js` and re-implement triad geometry (drift risk vs C++ side)
- Forget to register the scenario in `scenario-registry.js` (the dashboard dropdown won't see it)

## Related docs

- [CONTRACTS.md §4](../../../../CONTRACTS.md#4--scenario-dispatch-contract)
- [docs/adr/0006-prefix-dispatch-scenarios.md](../../../../docs/adr/0006-prefix-dispatch-scenarios.md)
- [engine/web/docs/USER_GUIDE.md](../../../docs/USER_GUIDE.md) §scenarios
- C++ mirror: `engine/src/scenarios/`
