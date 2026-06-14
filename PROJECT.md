# Project: FTD Scale-0 Scenario Comments & Verification

## Architecture
- **Configuration**: `engine/config/scenarios/scale0.json` (defines 36 scenario IDs).
- **JS Registry**: `engine/web/js/scales/scale0/scenario-registry.js` (registers Scale-0 scenarios for the web UI).
- **C++ Implementations**: `engine/src/scenarios/*.cpp` (implements the backend logic, primarily `flux.cpp`, `light.cpp`, `quantum.cpp`, `s0_field.cpp`, `s0_seed.cpp`, and `vacuum.cpp`, dispatched via `scenarios.cpp`).

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| M1 | Exploration & Analysis | Map all 36 scenario IDs in `scale0.json` to C++ and JS locations, analyze logic and identify discrepancies or no-ops | none | PLANNED |
| M2 | Comment Implementation | Add descriptive comments stating physical purpose, IC parameters, and expected behavior to C++ implementation files | M1 | PLANNED |
| M3 | Coverage Validation Script | Write and run a script confirming every scenario ID in `scale0.json` has a descriptive comment in C++ | M2 | PLANNED |
| M4 | Compilation & CTests | Compile C++ engine and run CTests to ensure no regressions or warnings | M2 | PLANNED |

## Interface Contracts
- **C++ Dispatch**: `dispatch_scenario(RenderBridge&, const std::string& name)` returns true/false.
- **JS Setup**: `load(harness, params)` in `scenario-registry.js` calls `harness.setupScenario`.
