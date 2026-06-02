# engine/tests — C++ test suite

**Purpose.** ~250 C++ unit tests, benchmarks, and measurement campaigns
for the FTD engine. Built via CMake; run via CTest.

**Registration audit (2026-05-27 cleanup).** All 267 `.cpp`/`.cu` files in this directory are registered in `engine/CMakeLists.txt` via either `ftd_add_test(...)` or `add_executable(...)`. Zero orphan files (tests that compile-fail-silently because no build rule references them). This invariant is intended to hold across future PRs: a new test file should arrive with its CMake registration in the same commit. If the audit ever drifts, see Commit 5 of the 2026-05-27 engine cleanup batch for the verification methodology (strict grep for `(?:ftd_add_test|add_executable|target_sources)\b[^)]*tests/<file>` plus a permissive `tests/<file>` fallback).

## Public API

Tests are individual `.cpp` files registered via the `ftd_add_test` macro
in `engine/CMakeLists.txt`. Each test is a `main()` that uses the
`ftd::test::*` API from `engine/include/ftd/test_telemetry.h`.

```cpp
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_xxx");
    ftd::test::section("Subsection");
    ftd::RenderBridge rb(16);
    // ... setup, tick, assert ...
    ftd::test::check("C1: ...", condition);
    ftd::test::check_close("C2: ...", computed, expected, 1e-9);
    return ftd::test::finalize();  // returns failure count
}
```

See [CONTRACTS.md §8](../../CONTRACTS.md#8--test-telemetry-contract) for the
telemetry API contract.

## Internal structure

| Pattern | Role | Examples |
|---|---|---|
| `test_*.cpp` | Standalone unit tests (1 per concern) | `test_constants.cpp`, `test_lorentz.cpp`, `test_audit_regression.cpp` |
| `benchmark_*.cpp` | Engine-theory bridge tests (numerical comparison) | `benchmark_engine_theory.cpp`, `benchmark_emergent_alpha.cpp`, `benchmark_black_hole_thermo.cpp` |
| `campaign_*.cpp` | Long-running measurement campaigns | `campaign_gluon_dynamics.cpp`, `campaign_wigner.cpp`, `campaign_lorentz_measure.cpp` |
| `support/` | Shared fixtures and telemetry impl (post-Phase 7, commits 2db67ca…87158ae) — see "Shared infrastructure" below |

### Engine-flawless audit additions (2026-06-01)

The engine-flawless lifecycle/callstack audit added three focused tests
(branch `flawless-engine-2026-06-01`):

| Test | Concern | Labels |
|---|---|---|
| `test_conservation_profile` | Energy-conservation + Gauss-constraint profile (pins the non-variational Gauss projection operator `J -= ∇φ` as the conservation leak; iteration-independent ~5e-3 RMS stencil floor) | `conservation`, `unit` |
| `test_tick_phase_order` | Tick phase-order regression (read → write → gauss_project → forces → movement) | `lifecycle`, `unit` |
| `test_engine_lifecycle` | ScaleEngine `clear()` / RAII teardown | `lifecycle`, `unit` |

## How to run

```bash
# Build
cmake -S engine -B engine/build && cmake --build engine/build --config Release

# Full pass
cd engine/build && ctest --output-on-failure -C Release

# Single test
engine/build/Release/test_audit_regression.exe

# Subset by name pattern
ctest -R "lorentz" --output-on-failure

# Subset by label (Phase 7 — live)
ctest -L unit       # 147 fast tests
ctest -L physics    # energy/Coulomb/locked-particle/absorbing-BC suite
ctest -L golden     # bit-exact regression gate (test_render_bridge_golden)
ctest -L slow       # multi-tick scenarios / perf
ctest -L gpu        # CUDA — route via WSL2
```

## CTest labels (Phase 7, live)

Every test now carries at least one label:

| Label | Coverage | Wall time |
|---|---|---|
| `unit` | Pure unit tests; no GPU; <1s each (147 tests) | ~30s total |
| `physics` | Energy conservation, Coulomb, locked particle, absorbing BC | ~2 min |
| `golden` | Bit-exact regression vs frozen byte-hash (`test_render_bridge_golden`, hash `0xcd957b601d47868a`) | <10s |
| `slow` | Multi-tick scenarios, perf-sensitive | 1-5 min each |
| `gpu` | Requires CUDA; route via WSL2 | varies |

The golden-tick gate is the bit-exact regression for any
physics-touching extraction (Phase 4 phase decomposition, Phase 5 CUDA
stencil split, etc.). See ADR-0012.

## Shared infrastructure

Phase 7 split (commits 2db67ca…87158ae): the test telemetry header is
now declarations-only; the implementation lives in a static support
library auto-linked to every test.

| File | LOC | Role |
|---|---:|---|
| `engine/include/ftd/test_telemetry.h` | 154 | Declarations only (was 412 LOC header-only) |
| `engine/tests/support/test_telemetry.cpp` | 312 | Implementation (compiled once, not per-TU) |
| `engine/tests/support/bridge_fixtures.h` / `.cpp` | — | Shared bridge fixtures: `ToggleProfile { Logic6, LogicOnly, FullEM, FullSM, Custom }`, `make_bridge(L, profile, seed=42, force_cpu=true)`, `run_for(rb, n)`, `inject_particle_at_center(rb, state, v)`, `assert_energy_conserved(rb, n_ticks, eps_rel)` |

The CMake target `ftd_test_support` is auto-linked into every test
registered through `ftd_add_test`; tests no longer duplicate
boilerplate setup.

`engine/include/ftd/test_telemetry_snapshot.h` — RenderBridge-aware
snapshot encoder (separate header to avoid circular include); used by
`test_render_bridge_golden` for the bit-exact regression hash.

## How to extend

### Adding a new test
1. Create `test_<name>.cpp` in this directory.
2. Use the standard skeleton (see Public API above). Prefer
   `support/bridge_fixtures.h` helpers over re-rolling setup.
3. Register in `engine/CMakeLists.txt` via the `ftd_add_test` macro,
   passing one or more LABELS (`unit` / `physics` / `golden` / `slow` /
   `gpu`). The `ftd_test_support` library is auto-linked.

### Adding a benchmark or campaign
- Same as above, but use `benchmark_` or `campaign_` prefix.
- Campaigns that take >5 min should set `set_tests_properties(... PROPERTIES TIMEOUT 600)` and carry the `slow` label.
- GPU campaigns: tag with the `gpu` label; document WSL2-only execution.

### Touching physics code
Any extraction or refactor that could perturb tick output MUST keep
`test_render_bridge_golden` green (hash `0xcd957b601d47868a`). Run
`ctest -L golden` before claiming Phase-X-complete. See ADR-0012.

## Invariants

- Each test exits 0 on success, non-zero on failure (use `ftd::test::finalize()` return)
- Tests do NOT depend on filesystem state from other tests
- Tests do NOT mutate `engine/results/` (campaigns may write to a tmpdir)
- GPU tests detect device absence and skip cleanly with a recorded reason

## Reference exemplars

- `test_constants.cpp` — pure constants audit (no RenderBridge needed)
- `test_audit_regression.cpp` — full RenderBridge with multiple sections (energy convention, locked particle, dual substrate, Laplacian sum-rule, Coulomb PE)
- `test_lorentz.cpp` — multi-section physics test with `force_diag_at` accessors

## Related docs

- [CONTRACTS.md §8](../../CONTRACTS.md#8--test-telemetry-contract)
- [engine/SPEC_ENGINE.md](../SPEC_ENGINE.md)
- [docs/adr/0008-r1-r5-phase-extraction.md](../../docs/adr/0008-r1-r5-phase-extraction.md)
- [docs/adr/0012-golden-tick-regression-gate.md](../../docs/adr/0012-golden-tick-regression-gate.md) — bit-exact regression gate
- [META_PROJECT_ATLAS.md](../../META_PROJECT_ATLAS.md) §10 (run-all commands)
- Test-time API: `engine/include/ftd/test_telemetry.h` (declarations) + `support/test_telemetry.cpp` (impl)
- Shared fixtures: `engine/tests/support/bridge_fixtures.{h,cpp}`
