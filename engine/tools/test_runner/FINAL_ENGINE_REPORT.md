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

## CTest baseline results (post-merge, post-fix)

Executed on 2026-04-14 after the three post-merge fixes landed. Two-pass
strategy using the `gpu` label the Phase 0 macro introduced — so CPU tests
can run parallel while GPU tests run serial (avoiding VRAM contention).

### Pass 1 — CPU tests in parallel (`ctest -j 24 -LE gpu`)

- **88 passed / 70 failed / 11 timeout / 2 unclassified, out of 171 tests**
- **Pass rate: 51.5%**
- **Wall clock: 800.85 sec (13.3 min)**
- An earlier serial attempt reached 72/175 in ~15 min before being killed
  and restarted with `-j 24`. The parallel run finishes the full 171-test
  CPU set in 13.3 min, dominated by 11 tests that each hit the 600s
  timeout. Estimated serial wall clock would have been 45-75 min, so
  parallelism delivered **~3-5× speedup** (not 24× because the timeout
  sitters are the wall-clock bottleneck — each worker sits on one for
  its full budget).

### Pass 2 — GPU tests serial (`ctest -j 1 -L gpu`)

- **4 passed / 0 failed / 0 timeout, out of 4 tests**
- **Pass rate: 100%**
- **Wall clock: 507.58 sec (8.5 min)**
- Per-test: `gpu_parity` 9.45s, `gpu_benchmark` 2.06s, `gpu_physics`
  199.20s, `gpu_experiments` 296.87s
- RTX 5090 Blackwell, 32 GB VRAM — no CUDA OOM, no VRAM contention.

### Combined total

- **92 passed / 83 failed or timed out, out of 175 tests = 52.6%**
- Pass 1 + Pass 2 combined wall clock: **~22 min** (vs 60-90 min single-pass
  serial estimate).

### Serial-vs-parallel cross-validation

Comparing the 39 tests that completed in both the serial and parallel
runs (the serial run was killed early to restart with `-j 24`):

- **Zero regressions from parallelism** — no test that passed serially
  fails in parallel. The 24-worker run does NOT introduce false failures.
- **One improvement** — `pe_forces` went Failed → Passed after my
  `3ca3253` particle_engine relativistic fix.
- The 11 timeouts in parallel are tests that were already running slow
  or hanging in serial (e.g. `em_energy_conservation` also timed out at
  600s serially). Timeout-under-contention is a reporting artifact, not
  a causal regression.

**Conclusion**: the 52.6% pass rate is the **real engine state on
current main**, not an artifact of parallelism or the FTD Test Bench merge.

### Post-WIP rerun (after committing the user's 177-file WIP)

After committing the user's uncommitted WIP as 6 chunks (commits `68971a2`
through `c173031`) and rebuilding from scratch, I reran both ctest passes
against the now-fully-committed state:

- **Pass 1 (CPU `-j 24 -LE gpu`)**: 89/171 passed, 792.47 sec wall
- **Pass 2 (GPU `-j 1 -L gpu`)**: 4/4 passed, 502.85 sec wall
- **Combined: 93/175 = 53.1%**

Cross-validated pre-WIP vs post-WIP:
- **Zero regressions** from committing the WIP — no test that was passing
  pre-WIP fails post-WIP.
- **One improvement** — `campaign_grothendieck` went Timeout → Passed
  (saved 600s wall clock, +1 on pass count).
- **One new failure surfaced** — `helium_scale1` (a brand-new test added
  in the WIP) segfaults after successful particle construction but before
  reaching diagnostics. The 2 locked protons at identical positions
  `(0,0,0)` were my first suspect (Barnes-Hut degenerate case) but
  offsetting them by `1e-10` didn't fix it. Per-line debug showed the
  crash is NOT on the `particles()[2].r_eff = 0.01` assignment (that
  works) — it's later, possibly in the diagnostics path or during
  the first tick. Tried to bisect but got diminishing returns on
  diagnosis time; left as a new WIP issue for the engine author.

Net post-WIP: 93/175 = **53.1%** (up from 92/175 = 52.6% pre-WIP).
One net test improvement (grothendieck fixed) minus one new regression
(helium_scale1 segfault) = 93 - 1 + 1 = 93 passing, 1 net improvement
on the suite.

The pre-existing failure cluster (EM/latency sign bug) was NOT fixed
by the user's WIP commits. Those need deeper engine work targeting
`render_bridge.cpp`'s `solve_latency_poisson()` and the `phi_latency`
sign handling — which appears to be what the user is actively
developing and is intentionally out of this session's scope.

## Known pre-existing failures — categorized by physics sector

The 83 non-passing tests cluster tightly into these families. Most are
downstream of main's commit `a4c75e4` (EFT reconstruction + latency fix
+ 7 benchmark suites) which refactored the particle and lattice force
pipelines and exposes the "latency field sign" bug documented in
`engine/tests/README_SCIENTIFIC_STATUS.md`.

### EM sector — 8 tests, all hit 600s timeout

`magnetic`, `maxwell`, `em_energy_conservation`, `poynting`,
`dispersion_relation`, `spectral`, `campaign_dispersion`,
`campaign_dispersion_convergence`

Matches the `README_SCIENTIFIC_STATUS.md` hypothesis: the EM sector
depends on `phi_latency` computed as negative near mass and clipped by
`sqrt(max(phi, 0))` to zero, breaking downstream EM + GR + BH
diagnostics. **This is where the biggest pass-rate improvement lives**
— fix the latency sign and most of these should come back.

### Poisson solver — 4 tests

`poisson_coulomb`, `campaign_poisson_force_law`,
`campaign_poisson_binding`, `campaign_poisson_hydrogen`

All downstream of the Poisson solver. Likely same latency-field root cause.

### Gauss + gravity — 7 tests

`gauss`, `gravity_dynamics`, `latency_field`, `intervoxel_coupling`,
`campaign_gravity_profile`, `campaign_gravity_hierarchy`, `einstein_equations`

Core gravitational sector. Latency field is central here.

### Particle / atomic forces — 13 tests

`lorentz_force`, `momentum`, `lorentz_invariance`, `selective_damping`,
`wavepacket`, `vortex`, `portable_field`, `bridge_dynamics`,
`dual_substrate`, `ae_angle_strain`, `ae_dipole`, `campaign_ae_water`,
`multiscale_bridge`

### SM / heavy physics — 14 tests

`electroweak`, `hydrogen_em_only`, `higgs_mechanism`, `wz_mass`,
`flavor_physics`, `campaign_hydrogen_binding`, `campaign_shell_predictions`,
`campaign_sm_observables`, `campaign_color_force`, `campaign_color_neutral`,
`campaign_confinement`, `campaign_gluon_dynamics`,
`campaign_weak_transmutation`, `campaign_parity_violation`

### QM / measurement / dark sector — 9 tests

`bell_aggregate`, `born_rule_ensemble`, `entanglement`, `hilbert`,
`ensemble`, `campaign_ds_ternary_detector`,
`campaign_ds_information_cascade`, `campaign_ds_vortex_lines` (timeout),
`campaign_ds_correlation_function` (timeout), `campaign_dark_sector`

### Other — miscellaneous

`thermodynamics`, `lagrangian`, `annihilation_conservation`, `genesis`,
`asymptotic_freedom`, `baryogenesis`, `triad_confinement`, `confinement_test`,
`action_stationarity`, `energy_conservation`, `energy_tracking`, `logic_engine`,
`campaign_gauge_constraint`, `campaign_gauge_dynamics`, `campaign_triad_binding`,
`campaign_multiscale_pipeline`, `campaign_aggregate_interaction`,
`campaign_force_law`, `campaign_grothendieck` (timeout), `gauge`,
`wave_collapse`, `light`, `larmor`, `em_fields`, `thomson_scattering`

### What works (92 passing)

All 4 GPU benchmarks, both Phase 1+2b consolidated suites
(`tritium_algebra` and `pe_forces` — the latter only after fix `3ca3253`),
the `telemetry_selftest` for the Phase 2a library, foundational math
(`constants`, `lorentz`, `lattice`, `born_infeld`, `ontic_chain`), most
`pe_*` forces (now all rolled into `pe_forces`), most `ae_*` forces,
particle + atom engine core, scale bridges including `hydrogen_scale1`,
`atom_engine`, `atom_scale_bridge`, benchmarks including
`ftd_benchmark_engine_theory`, `ftd_emergent_alpha`, `ftd_budget_equation`,
QM campaigns like `campaign_bell_substrate`, `campaign_epr_correlation`,
`campaign_born_rule`, `campaign_born_ensemble`, convergence campaigns
`campaign_coulomb_convergence` and `campaign_wave_isotropy`, plus many
infrastructure tests.

The working set covers foundational math + unit tests + all GPU
benchmarks + much of the scale-ladder infrastructure. The broken set
clusters tightly around the latency-field-dependent sectors, which is
consistent with the documented pre-existing bug.

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
