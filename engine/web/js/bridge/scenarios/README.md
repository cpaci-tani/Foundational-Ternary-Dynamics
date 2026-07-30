# engine/web/js/bridge/scenarios — JS parity mirror

**Purpose.** Name-level twin of `engine/src/scenarios/*.cpp` for Playwright
parity CI (`tests/scenario-parity.spec.js`). Partitioned into 6 prefix-named
domains.

> **Not the live seed path.** The dashboard loads Scale-0 scenarios through
> WASM → `ftd::dispatch_scenario` (`WasmBridge` / `WasmBridgeProxy`). Do not
> wire this tree back into production loads without a deliberate architecture
> change.

> **Architecture references:**
> [SPEC_SCALE0_SCENARIO_ARCHITECTURE.md](../../../docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md),
> [engine/SCENARIO_ARCHITECTURE.md](../../../../SCENARIO_ARCHITECTURE.md).

## Public API

`runSetupScenario(name, harness)` returns `true` if a group handled `name`,
else `false` (and warns). Used only by the JS parity / MockBridge-era path.

## Internal structure

| Group | File | Domain |
|---|---|---|
| `flux-*` | `flux-scenarios.js` | Substrate physics |
| `light-*` | `light-scenarios.js` | EM / light demos |
| `quantum-*` | `quantum-scenarios.js` | QM pedagogy demos |
| `s0-vacuum-*` | `vacuum-scenarios.js` | Vacuum particle seeds |
| `s0-seed-*` | `s0-seed-scenarios.js` | SM / Moore / gravity / emergent |
| `s0-field-*` | `s0-field-scenarios.js` | Field configurations |

Plus `_helpers.js` (mirrors `engine/src/scenarios/_helpers.h`) and
`physics-lattice.js` (`PHYSICS_REFERENCE_L = 33`).

## Drift policy

Every IC change must update **C++ first** (live path), then the JS case of the
same id. Parity CI fails on name drift; it does not prove semantic equality.
