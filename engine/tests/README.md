# engine/tests — C++ test suite

**Purpose.** ~250 C++ unit tests, benchmarks, and measurement campaigns
for the FTD engine. Built via CMake; run via CTest.

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
| `campaign_*.cpp` | Long-running measurement campaigns | `campaign_emergent_spectrum_2026-04-27.cpp`, `campaign_gluon_dynamics.cpp`, `campaign_wigner.cpp` |
| `support/` | (Phase 7 will add) Shared fixtures: `make_bridge`, `run_for`, `assert_energy_conserved` | TBD |

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

# (Phase 7 target) Subset by label
ctest -L unit
ctest -L physics
ctest -L golden
```

## CTest labels (Phase 7 target)

After Phase 7 lands, every test will carry at least one label:

| Label | Coverage | Wall time |
|---|---|---|
| `unit` | Pure unit tests; no GPU; <1s each | ~30s total |
| `physics` | Energy conservation, Coulomb, locked particle, absorbing BC | ~2 min |
| `golden` | Bit-exact regression vs frozen byte-hash output | <10s |
| `slow` | Multi-tick scenarios, perf-sensitive | 1-5 min each |
| `gpu` | Requires CUDA; route via WSL2 | varies |

## Shared infrastructure

`engine/include/ftd/test_telemetry.h` — declarations + (currently) inline
impl. Phase 7 will split impl to `engine/tests/support/test_telemetry.cpp`
to stop recompiling 412 LOC across 155 TUs.

`engine/include/ftd/test_telemetry_snapshot.h` — RenderBridge-aware
snapshot encoder (separate header to avoid circular include).

## How to extend

### Adding a new test
1. Create `test_<name>.cpp` in this directory.
2. Use the standard skeleton (see Public API above).
3. Register in `engine/CMakeLists.txt`:
   ```cmake
   add_executable(test_<name> tests/test_<name>.cpp)
   target_link_libraries(test_<name> ftd_core)
   add_test(NAME <name> COMMAND test_<name>)
   ```
   (Phase 7 will introduce `ftd_add_test(<name> SOURCES ... LABELS ...)` macro.)

### Adding a benchmark or campaign
- Same as above, but use `benchmark_` or `campaign_` prefix.
- Campaigns that take >5 min should set `set_tests_properties(... PROPERTIES TIMEOUT 600)`.
- GPU campaigns: tag with the `gpu` label (Phase 7); document WSL2-only execution.

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
- [META_PROJECT_ATLAS.md](../../META_PROJECT_ATLAS.md) §10 (run-all commands)
- Test-time API: `engine/include/ftd/test_telemetry.h`
