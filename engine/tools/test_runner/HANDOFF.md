# FTD Test Bench — Branch Handoff

> **Superseded for current state** by [`TEST_AUDIT_2026_04_14.md`](TEST_AUDIT_2026_04_14.md).
> Retained as the historical snapshot of the Phase 0–8 branch merge.
> Family tables in this doc contain errors (QCD phantom files, ae_* count, dark sector grouping) — see the audit for corrections.

**Branch:** `worktree-test-runner-unified`
**Worktree:** `C:\Users\cpaci\Desktop\ftd\.claude\worktrees\test-runner-unified`
**Merge base:** `c0f7d3a` (main HEAD at time of worktree creation)
**Plan file:** `C:\Users\cpaci\.claude\plans\concurrent-watching-crane.md`

This branch contains the FTD Test Bench unified test runner work, implemented
across ten commits according to the approved plan. The goal was a native Qt6
Windows application that replaces the scattered CTest / pytest / `run_tests_live.py`
surfaces with a single pane of glass: batch test selection, live output,
smart GPU/CPU dispatch, live 3D lattice visualization during runs, streaming
telemetry charts, and SQLite-backed run history with regression detection.

## What landed

| # | Commit | Phase | Summary |
|---|--------|-------|---------|
| 1 | `e86513f` | 0 | `ftd_add_test` CMake macro + `FTD_ENABLE_CUDA=AUTO` default via `find_package(CUDAToolkit)` probe, with graceful CPU-only fallback. Arch default `89;120` set before `enable_language(CUDA)`. New helper `engine/cmake/FtdAddTest.cmake`. |
| 2 | `3801e49` | 2a | `engine/include/ftd/test_telemetry.h` header-only NDJSON API: `init`, `section`, `check`, `check_close`, `metric`, `tick` (with extras), `snapshot` (base64 int8 voxels), `finalize`. Zero-overhead when `FTD_TEST_TELEMETRY` env var unset (human-readable mode byte-identical to the legacy pattern). New self-test `test_telemetry_selftest.cpp` verifies both modes. |
| 3 | `f87c7c5` | 3 | Qt6 runner scaffold at `engine/tools/test_runner/` (13 classes, ~2127 LOC): `MainWindow`, `TestModel` (reads `ctest --show-only=json-v1`, ports `CATEGORY_RULES` from the retired `run_tests_live.py`, extracts descriptions from `/** Test: ... */` source headers, supports GPU badge from CTest label), `TestRunner` (QProcess-per-test subprocess launcher, sets `FTD_TEST_TELEMETRY=1` in env), `NdjsonParser` (incremental line parser with regex fallback for non-instrumented tests), `SmartDispatcher` (serial GPU queue + parallel CPU queue, N-1 workers), `OutputPanel` (interleaved per-test output view). CMake integration: `FTD_BUILD_TEST_RUNNER=AUTO` + `find_package(Qt6 QUIET)`. Placeholders for Phases 4-6 tabs. |
| 4 | `3403865` | 4 | `LatticeViewer` QOpenGLWidget (~752 LOC for class + shaders): 300-frame ring buffer of `LatticeFrame`, `ingestSnapshot()` decodes base64 int8 voxel grids from Phase 2a snapshot events, `paintGL` renders voxels as instanced points with antialiased disc fragment shader (state → color), draws axis bbox. Arcball camera with left-drag rotate, middle-drag pan, wheel zoom. Pause / scrub-offset API. `FieldLines.{h,cpp}` stub with TODO for full RK4 port from `engine/web/js/fieldlines.js`. Replaces the Phase 3 placeholder in the Live Lattice tab. |
| 5 | `e450c96` | 5 | `TelemetryCharts` widget using Qt6 QtCharts (144 + 450 LOC): auto-discovers metric series from both `metric` and `tick` NDJSON events, per-test chart sets in a `QStackedWidget` with `QComboBox` selector, 1000-sample circular buffers per `QLineSeries`, auto-scaling X/Y axes with incremental min/max and periodic rescan. Static chart grouping rules (Energy / Gauss violation / Forces / Counts / Performance) merge related metrics into shared charts. `loadExpectedValues(testName)` reads optional `engine/tests/expected/<test>.json` sidecar for horizontal reference lines (silent when absent). `OutputPanel`'s 1:50 metric sampling throttle removed — charts now receive every data point; OutputPanel logs only the first occurrence per `(test,metric)` pair. |
| 6 | `b454a80` | 6 | `HistoryDb` (SQLite via Qt6::Sql, 161 + 483 LOC): auto-migrating schema (`runs`, `test_results`, `schema_version`), run-scoped transactions wrap per-test inserts for throughput, `startRun`/`recordResult`/`finishRun` lifecycle, `findRegressions` (pass→fail flips since last run), `diffRuns`. `HistoryTab` UI (78 + 447 LOC): `QTableView` of past runs, click-to-show details, two-row Diff button, per-test trend chart. MainWindow: replaces History tab placeholder, instantiates `HistoryDb` at `applicationDirPath()/runs.sqlite`, captures git short SHA via `git rev-parse --short HEAD` subprocess (500ms timeouts), 10-second regression toast in the status bar. |
| 7 | `f8844b7` | 7 | **Retired** the legacy web test dashboard. Deleted `engine/web/tests.html`, `engine/web/js/tests.js` (535 LOC), `engine/run_tests_live.py` (374 LOC). Updated historical comments in `TestModel.{h,cpp}` and `README.md` to note the retirement. Updated `engine/tools/AUDIT_PLAN.md` lines 6.15/6.16 to mark the legacy dashboard as retired and point at the Qt runner. Net diff: +55/-1412 lines — meaningful dead-code removal. Also added a local `.gitignore` at the worktree root mirroring the main repo's ignore rules (the main repo's `.gitignore` is an uncommitted local modification, so the worktree didn't inherit any ignore rules by default — a `git add -A` accident was caught and reset). |
| 8 | `7d93eb0` | 8a | PyTorch canonical imports added to `scripts/constants.py` (`DEVICE`, `DTYPE`, `t()`, `to_numpy()`, `TORCH` — behind `try/except ImportError` so scripts that don't use them are unaffected). Converted 5 hot scripts: `compute_observer_bell.py`, `born_rule_comprehensive.py`, `watson_convergence.py`, `proof_d3_uniqueness.py`, `proof_bell_cosine_from_gauss.py`. **`watson_convergence.py` ran 38× faster** (3m 18.7s → 5.1s) from replacing a pure-Python triple-nested for-loop (512M iterations) with a chunked 3D broadcast — speedup realized via the NumPy fallback alone, before any GPU involvement. `proof_confinement_wilson.py` skipped (no hot loop, pure analytic). `look_elsewhere_monte_carlo.py` skipped (pre-existing IndexError bug in baseline unrelated to conversion). Status doc at `scripts/PHASE_8_PYTORCH_STATUS.md`. |
| 9 | `d0f558a` | — | `engine/_verify_final.bat` end-to-end verification helper: builds Qt runner + telemetry selftest, runs the selftest in both modes, captures `ctest --show-only=json-v1`. Post-Phase-8a run: 178 tests registered, selftest emits 26 NDJSON lines, human-readable ALL PASS. |
| 10 | `6469a33` | 1+2b POC | Consolidated two test families as a proof-of-concept for the full Phase 1 sweep. **tritium: 7 files (1104 LOC) → 1 file (847 LOC)**, 23% reduction. **pe_*: 7 files (1060 LOC) → 1 file (818 LOC)**, 23% reduction. Both use the Phase 2a `ftd::test` API (172 + 46 = 218 check sites). Parity verified: pe_* has strict runtime 41/41 PASS match; tritium has 172 static CHECK→check site match (tritium's legacy CHECK macro was silent on pass, no runtime PASS count to diff). Builds cleanly, `ctest -R '^(pe_forces\|tritium_algebra)$'` → 2/2, both modes exit 0. |

**Total diff vs `c0f7d3a` merge base:** 62 files changed, +8045 / -3687 lines (net +4358). Ten commits.

## How to build and run

Everything runs from the **worktree root** (`.claude/worktrees/test-runner-unified`).
The user's main checkout at `C:\Users\cpaci\Desktop\ftd\engine\` is untouched.

### Build the Qt runner

```
cmd.exe //c "engine\\_build_runner.bat"
```

This helper sources `vcvars64.bat` via `vswhere`, copies CUDA MSBuild extensions
if missing, configures with `-DFTD_ENABLE_CUDA=ON -DCMAKE_PREFIX_PATH=C:/Qt/6.10.2/msvc2022_64`,
and builds the `ftd_test_runner` target via MSBuild.

Output: `engine/build_runner/tools/test_runner/Release/ftd_test_runner.exe` (~435 KB).

### End-to-end verification (also builds telemetry selftest, runs it in both modes)

```
cmd.exe //c "engine\\_verify_final.bat"
```

Output: `engine/build_final/` with both binaries, NDJSON capture, and CTest enumeration.

### Launch the runner

```
engine/build_runner/tools/test_runner/Release/ftd_test_runner.exe --build-dir engine/build_runner
```

The `--build-dir` flag tells `TestModel` where to run `ctest --show-only=json-v1`
to enumerate tests. Defaults to `engine/build` if omitted.

### Disable CUDA or the runner (if needed)

- `cmake -DFTD_ENABLE_CUDA=OFF ...` — forces CPU build
- `cmake -DFTD_BUILD_TEST_RUNNER=OFF ...` — skips the runner target entirely

## What's explicitly deferred (not in this branch)

The plan had additional work that was intentionally scoped OUT of this session
to land a shippable core deliverable first. These are documented follow-ups, not
regressions.

### Phase 1 (full sweep) — 12 more consolidation families

The POC (commit 10) consolidated 2 of the 14 families identified in the plan.
Remaining:

| Family | Old files | New file | Estimated reduction |
|--------|-----------|----------|---------------------|
| Coulomb force law | `test_poisson_coulomb`, `campaign_coulomb_convergence`, `campaign_force_law`, `campaign_poisson_force_law` | `campaign_coulomb_force_law.cpp` | 3 files |
| Wave dynamics | `test_wave_speed`, `test_interference`, `campaign_wave_isotropy`, `campaign_two_slit` | `campaign_wave_dynamics.cpp` | 3 files |
| Hydrogen spectrum | `test_hydrogen_scale1`, `test_hydrogen_em_only`, `campaign_poisson_hydrogen`, `campaign_hydrogen_spectrum`, `test_hydrogen_spectrum_scale1` | `campaign_hydrogen_spectrum.cpp` | 4 files |
| `ae_*` atom engine forces | 6 files | `test_atom_engine_forces.cpp` | 5 files |
| Energy conservation | `test_energy`, `test_energy_conservation`, `test_energy_tracking` | `test_energy_conservation.cpp` | 2 files |
| Lorentz family | `test_lorentz`, `test_lorentz_invariance`, `test_lorentz_force` | `test_lorentz.cpp` | 2 files |
| Dispersion | `test_dispersion_relation`, `campaign_dispersion`, `campaign_dispersion_convergence` | `campaign_dispersion.cpp` | 2 files |
| Magnetic | `test_magnetic`, `test_magnetic_lagrangian` (overlap with lorentz family) | folded into Lorentz | 1 file |
| Gauss/Poisson | `test_gauss`, `test_gauss_convergence` | `test_gauss.cpp` (L sweep) | 1 file |
| QCD forces | `campaign_color_force`, `campaign_strong_force`, `campaign_exchange_force` | `campaign_qcd_forces.cpp` | 2 files |
| Quantum correlations | `test_entanglement`, `campaign_epr_correlation`, `campaign_bell_substrate` | `campaign_quantum_correlations.cpp` | 2 files |
| Dark sector | 7× `campaign_ds_*` | `campaign_dark_sector.cpp` | 6 files |

Execution pattern is well-established by the POC commit: read family, create
new consolidated file with `ftd::test::section()` per original, `ftd::test::check()`
replacing legacy `check()`, parity-verify old vs new PASS counts, delete old
files, update `CMakeLists.txt` (remove old entries, add `ftd_add_test(...)`),
build, run both modes, commit with parity table in the message. Each family is
~30-60 minutes of focused work.

### Phase 2b (instrumentation sweep) — ~150 tests not yet converted

The POC instrumented the two consolidated families (218 check sites total). The
remaining ~150 pre-existing tests still emit their own hand-written `check()` /
`check_close()` macros in their `main()` functions. The runner HAS a fallback
path in `NdjsonParser` that regex-parses `"  PASS  name"` / `"  FAIL  name"`
lines, so non-instrumented tests still run and appear in the runner correctly,
just without the rich tick/metric/snapshot stream. Converting the rest is a
pure upside — enables live lattice viz and telemetry charts for every test —
but is not a blocker.

The mechanical transformation per test is:
```cpp
// Before
#include <cmath>
int failures = 0;
void check(const char* name, bool ok) { /* ~8 lines */ }
void check_close(const char* name, double a, double b, double tol) { /* ~10 lines */ }
int main() {
    // ... test body with check() / check_close() calls
    return failures > 0 ? 1 : 0;
}

// After
#include "ftd/test_telemetry.h"
int main() {
    ftd::test::init("test_X");
    // ... unchanged test body
    //     check(...) → ftd::test::check(...)
    //     check_close(...) → ftd::test::check_close(...)
    return ftd::test::finalize();
}
```

~30 LOC removed per test × 150 tests = **~4500 LOC of boilerplate** waiting to
be shed by a sweep.

### Phase 8b+ (remaining 122 Python scripts)

Phase 8a converted the 5 hottest MC / high-iteration scripts. The tracking doc
`scripts/PHASE_8_PYTORCH_STATUS.md` lists the next priorities:

- Remaining `scripts/proofs/*.py` with hot numerical loops
- `scripts/verification/*.py` Dirichlet sums (`verify_anti_correlation.py`,
  `verify_log_gstar_identity.py` each have `range(100000)` mpmath loops)
- `scripts/tests/comprehensive/test_tier4_simulation.py` lattice dynamics —
  the single biggest parallel-safe win in the Python suite
- `scripts/experiments/` lattice analysis files (`lattice_analysis/analyze_*.py`)

PyTorch is **NOT INSTALLED** in the user's environment as of Phase 8a. The
converted scripts currently run the NumPy fallback path on every invocation.
Installing PyTorch (`pip install torch --index-url https://download.pytorch.org/whl/cu121`)
would unlock the GPU speedup for the converted 5 plus anything future.

### Long-standing engine bugs (documented, not fixed)

None of the Phase 0-7 work touches engine core code. The 40 pre-existing
CTest failures from the Apr 13 run are still present; the runner just
surfaces them more clearly. The plan explicitly said "the 40 currently-
failing CTests stay failing". Fixing them is separate work and shouldn't
block merging this branch.

## How to merge (options)

Zero-conflict path: this branch was cut from `c0f7d3a`, which is BEFORE
the user's local uncommitted engine work. The user's uncommitted changes
in the main workspace (`engine/CMakeLists.txt`, `engine/include/ftd/`,
`engine/src/`, `engine/include/ftd/barnes_hut.h`, `engine/include/ftd/dag_*.h`,
`engine/src/dag_engine.cpp`) will need to be reconciled when merging.

### Option A: Merge the whole branch now

```
git checkout main
git merge worktree-test-runner-unified
```

Conflicts expected in `engine/CMakeLists.txt` (I edited the top section for
CUDA default and the test registration; the user edited other sections for
the dag_engine work). Straightforward to resolve manually — the changes are
in disjoint parts of the file.

### Option B: Cherry-pick per-phase

Each phase is a clean commit. You can cherry-pick just Phases 0, 2a, 3-6 (the
Qt runner core) and leave Phases 7, 8a, 1+2b POC for later:

```
git cherry-pick e86513f 3801e49 f87c7c5 3403865 e450c96 b454a80
```

Then build with `engine/_build_runner.bat` to verify on top of the user's
main branch state.

### Option C: Keep the branch alive, merge later

Leave the worktree in place while you continue other work on main. Return
to finish Phase 1/2b/8b as a follow-up session. The worktree is self-contained.

## Known issues / caveats

1. **No GUI smoke test** — none of the agents launched the runner executable.
   Compile + link success and the telemetry selftest (human + NDJSON) are the
   verification surface. A five-minute manual smoke test on the user's machine
   would close this gap.

2. **GPU architecture hardcoded to 89;120** — matches the user's RTX 5090
   (Blackwell SM 120). Ada (89) is kept for forward compat with RTX 4060-4090.
   Older GPUs (Turing 75, Ampere 86) need a cmake override:
   `-DCMAKE_CUDA_ARCHITECTURES="86;89;120"`.

3. **`pe_forces` label is `['scale1']` not `['unit']`** — the existing
   `set_tests_properties(... PROPERTIES LABELS "scale1")` block in
   `engine/CMakeLists.txt` uses replacement semantics, clobbering the
   `ftd_add_test` macro's auto-applied `"unit"` label. This is a pre-existing
   convention (same issue affects `particle_engine`). Fix is a one-liner:
   change the LABELS block to `set_property(TEST ... APPEND PROPERTY LABELS "scale1")`.
   Intentionally out of scope for the POC commit.

4. **Tritium consolidated file is 847 LOC** — 5.9% over the spec's 800-LOC
   soft cap. Preserving every assertion verbatim was a hard constraint that
   outranked the soft cap. Splitting into two files (`algebra_core` +
   `algebra_linalg`) is a straightforward follow-up if the size matters.

5. **`look_elsewhere_monte_carlo.py` has a pre-existing IndexError** in its
   NumPy baseline when run with 1M samples. Unrelated to Phase 8a conversion;
   documented in the PyTorch status doc. Needs a separate fix.

6. **`born_rule_comprehensive.py` lacks `np.random.seed(...)`** at module
   load. Two consecutive runs produce different numerical values — not
   deterministic by design. The Phase 8a conversion preserves this
   non-determinism; golden-file parity was checked on structure (13/13 tests
   pass both sides), not exact values.

7. **Runner executable is 435 KB** — reasonable for a Qt6 Widgets app with
   Charts, OpenGL, and Sql modules. Qt DLLs need to be on PATH at launch
   time; `windeployqt` would bundle them for distribution but that's not
   configured (development-time only).

## Plan reference

Full plan with phase-by-phase design: `C:\Users\cpaci\.claude\plans\concurrent-watching-crane.md`

## Artifact locations (reference)

- **Runner source tree:** `engine/tools/test_runner/` — 15 .h/.cpp files + CMakeLists.txt + README.md
- **Telemetry library:** `engine/include/ftd/test_telemetry.h`
- **CMake macro:** `engine/cmake/FtdAddTest.cmake`
- **Build helper:** `engine/_build_runner.bat`
- **Verification helper:** `engine/_verify_final.bat`
- **PyTorch status doc:** `scripts/PHASE_8_PYTORCH_STATUS.md`
- **This handoff doc:** `engine/tools/test_runner/HANDOFF.md`
- **Plan file (outside worktree):** `C:\Users\cpaci\.claude\plans\concurrent-watching-crane.md`
