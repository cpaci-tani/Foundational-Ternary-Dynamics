# FTD Test Bench — Final Engine Report

Closing report for the "finish strong with the engine" session. Covers the
state of `main` after the FTD Test Bench merge, the full-engine build +
test results, known failure categories, and what's ready to hand off.

## Tree of commits since the merge base

```
d7d32e4  Fix CMakeLists LABELS overwrite: stack category labels via set_property APPEND
3ca3253  Fix particle_engine::compute_force: apply relativistic + radiation post-processing
ec84346  Fix benchmark_wilson_loops: RenderBridge non-copyable under CUDA
fbec0a2  Merge branch 'worktree-test-runner-unified' — FTD Test Bench
4b98f43  Add HANDOFF.md — branch handoff summary for the FTD Test Bench worktree
6469a33  Phase 1+2b POC: consolidate tritium + pe_* families with telemetry instrumentation
d0f558a  Add engine/_verify_final.bat — end-to-end verification helper
7d93eb0  Phase 8a: PyTorch canonical imports + top 5 hot script conversions
f8844b7  Phase 7: Retire legacy web test dashboard (SSE + Python)
b454a80  Phase 6: SQLite HistoryDb — run persistence + regression detection
e450c96  Phase 5: TelemetryCharts — live multi-trace scalar telemetry from NDJSON
3403865  Phase 4: LatticeViewer QOpenGLWidget — live 3D lattice from NDJSON snapshots
f87c7c5  Phase 3: Qt6 test runner scaffold — MainWindow + smart dispatcher
3801e49  Phase 2a: ftd::test NDJSON telemetry library + self-test
e86513f  Phase 0: ftd_add_test macro + CUDA default via AUTO probe
009c9ee  docs: Sync all meta-documentation with v2.14 / v5.30 / April 13 session  ← merge base from user's main
```

## Build status

Full CUDA ALL_BUILD target against `engine/build_strong/`:
- Qt 6.10.2 msvc2022_64 auto-detected
- CUDA 13.0 auto-detected, architectures `89;120` (Ada + Blackwell / RTX 5090)
- **187 executables produced** (includes all engine tests + `ftd_test_runner.exe`
  at `engine/build_strong/tools/test_runner/Release/ftd_test_runner.exe`)
- Build time on the user's RTX 5090 + VS 2026: ~10 minutes clean, ~2s incremental

Build verified after three fixes discovered by the full-build + ctest sweep:

1. **`ec84346` — `benchmark_wilson_loops.cpp`**: `RenderBridge` has deleted
   copy constructor under CUDA (owns `std::unique_ptr<gpu::GpuEngine>` member,
   and the user-declared destructor suppresses implicit move generation).
   `setup_color_field` was returning by value, which required a copy. Fix:
   convert to out-parameter reference. Only CUDA builds were affected
   (CPU-only builds masked the issue before main's EFT commit enabled CUDA).

2. **`3ca3253` — `particle_engine.cpp`**: Main's commit `a4c75e4` (EFT
   reconstruction) refactored the particle force pipeline and accidentally
   moved the radiation reaction + relativistic correction post-processing
   into `compute_all_forces()` only. Direct `compute_force(i)` calls (used
   by `test_pe_forces` and similar) saw the raw pairwise sum without the
   `1/gamma` correction. Caught by `test_pe_forces` RE1/RE3/RE4 failing
   with `gamma=1.1547 expected_ratio=0.866025 actual=1 err=0.154701`.
   Fix: duplicate the post-processing block into `compute_force(i)`.
   Changes only `engine/src/particle_engine.cpp` (the .h is in user WIP
   and deliberately left untouched to avoid merge conflict).
   Post-fix: `test_pe_forces` goes from 5 failures to ALL PASS (46/46
   assertions across 7 sections: exchange, lorentz, magnetic_dipole,
   radiation, relativistic, spin_orbit, strong).

3. **`d7d32e4` — `engine/CMakeLists.txt`**: The five category LABELS
   blocks (unit/campaign/foundation/lagrangian/scale1/scale2) used
   `set_tests_properties(... PROPERTIES LABELS "foo")` with replacement
   semantics, so a test appearing in multiple blocks only kept the LAST
   label assigned. `pe_forces` ended up with `['scale1']` when it should
   have had `['unit', 'scale1']`; same for `particle_engine`, `lagrangian`,
   `magnetic_lagrangian`, etc. Was flagged in HANDOFF.md as a pre-existing
   one-liner. Fix: convert all five LABELS blocks from `set_tests_properties`
   to `set_property(TEST ... APPEND PROPERTY LABELS ...)`. TIMEOUT blocks
   stay as `set_tests_properties` (TIMEOUT is a scalar, replacement is
   correct). GPU label block already used APPEND from Phase 0. Post-fix
   verification via `ctest --show-only=json-v1`:

   ```
   pe_forces           labels=['scale1', 'unit']
   particle_engine     labels=['scale1', 'unit']
   lagrangian          labels=['lagrangian', 'unit']
   magnetic_lagrangian labels=['foundation', 'lagrangian', 'unit']
   ```

## Runner pipeline verification (without GUI launch)

The Qt runner cannot be launched headlessly (requires a display), but the
underlying data pipeline was verified end-to-end:

### 1. CTest enumeration (what `TestModel` reads at startup)

```
ctest --test-dir engine/build_strong --show-only=json-v1 -C Release
```

**Result:** 175 tests registered, label distribution:
- `unit=76  campaign=47  scale2=9  foundation=8  lagrangian=5  scale1=4  gpu=4`

All Phase 0-6 + POC tests present:
- `constants`, `tritium_algebra`, `pe_forces`, `telemetry_selftest` — from my branch
- `gpu_physics`, `campaign_gluon_dynamics`, `einstein_equations` — from main's new benchmarks

### 2. NDJSON protocol (what `NdjsonParser` consumes for instrumented tests)

```
FTD_TEST_TELEMETRY=1 engine/build_strong/Release/test_telemetry_selftest.exe
```

**Result:** 26 lines of valid JSON, 7 event types:
- `start=1, section=5, check=5, metric=10, tick=3, snapshot=1, end=1`

Every line round-trips through `json.loads` cleanly — the Qt
`QJsonDocument::fromJson` equivalent will handle them identically.

### 3. Regex fallback (what `NdjsonParser` does for non-instrumented tests)

```
engine/build_strong/Release/test_constants.exe
```

**Result:** 74 output lines, 47 matched by the `^\s{2,}(PASS|FAIL)\s{2}(.+)`
regex fallback (all 47 passing). 0 failures detected.

### 4. Smart dispatcher hints

The CMake `ftd_add_test()` macro auto-labels GPU-heavy tests with the
`gpu` CTest label. The runner's `SmartDispatcher` reads this label to
route tests into the serial GPU queue vs the parallel CPU queue.
4 GPU-labeled tests: `gpu_parity`, `gpu_benchmark`, `gpu_physics`,
`gpu_experiments`.

## CTest baseline results

**TO BE FILLED IN ONCE CTEST COMPLETES.**

## Known pre-existing failures

From the Apr 13 run (pre-merge), 40 tests were failing. These are documented
in `engine/tests/README_SCIENTIFIC_STATUS.md` and `engine/tests/TEST_DEVIATION_MAP.md`
and are intentionally out of scope for the FTD Test Bench plan. They should
still appear as failing in the runner (making them more visible).

Categories observed in the first 26 tests of this session's ctest run:

- `thermodynamics` — failed (pre-existing)
- `lagrangian` — failed (pre-existing, possibly related to latency field)
- `dual_substrate`, `genesis`, `gravity_dynamics`, `annihilation_conservation`,
  `wave_collapse`, `gauge`, `momentum` — all failing

The `flux_mediated` / `latency` / `dual_substrate` failures are documented
as a "latency field sign" bug in `README_SCIENTIFIC_STATUS.md`.

## Ready to use

Build helpers (all live in `engine/`):
- `_build_runner.bat`  — builds `ftd_test_runner.exe` only (fast: ~2-30s incremental)
- `_build_all.bat`     — builds the full engine ALL_BUILD (slow: ~10 min clean)
- `_build_wilson.bat`  — builds just `ftd_wilson_loops` (used to verify the copy fix)
- `_verify_final.bat`  — smoke-tests the runner + telemetry selftest

Runner location: `engine/build_strong/tools/test_runner/Release/ftd_test_runner.exe`

To launch:

```bash
engine/build_strong/tools/test_runner/Release/ftd_test_runner.exe --build-dir engine/build_strong
```

## Deferred work (consistent with HANDOFF.md)

1. **Phase 1 — 12 more consolidation families** (Coulomb, wave, hydrogen,
   ae_*, energy, Lorentz, dispersion, magnetic, gauss, QCD, quantum
   correlations, dark sector). POC pattern is proven; each family is
   ~30-60 min of mechanical work.

2. **Phase 2b — ~150 more test instrumentation** sweeps to give every
   test the full live 3D lattice viz / streaming charts experience.

3. **Phase 8b — more Python PyTorch conversions** (122 scripts remaining).
   A parallel subagent is running during this session.

4. **Pre-existing engine failures** (~40 tests) — out of scope for the
   test bench work, but now visible via the runner.

5. **GUI smoke test** — launch the runner on the user's machine and click
   through each tab to verify the UI actually works (visual verification
   only; compile + link + data pipeline are already validated).

6. **Polish items** from HANDOFF.md:
   - `pe_forces` CTest label is `['scale1']` not `['unit']` due to a
     pre-existing set_tests_properties pattern using replacement
     semantics. One-line fix.
   - Tritium consolidated file at 847 LOC is 5.9% over the soft cap.
     Optional split.
   - `windeployqt` for portable Qt6 DLL bundling.
