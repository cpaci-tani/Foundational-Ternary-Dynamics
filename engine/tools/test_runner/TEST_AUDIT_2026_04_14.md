# FTD Engine Test Audit — April 14, 2026

**Status**: Authoritative snapshot. Supersedes the narratives in
`HANDOFF.md`, `FINAL_ENGINE_REPORT.md`, and `AUDIT_LATENCY_2026_04_14.md`
for "what is true right now". Those three docs remain as historical
snapshots of earlier phases.

**Scope**: Full inventory and classification of the FTD engine test
suite after the Phase 0–8 test bench merge and the April 14 latency +
GPU-label audits. Produced as the opening of the "audit the tests
and fix them" session.

**Session plan reference**: `C:\Users\cpaci\.claude\plans\async-percolating-moler.md`

---

## 1. Baseline snapshot (post-Wave 1)

Wave 1 (commit `8d7ed60`): serialized 11 GPU-heavy tests by appending
them to `_ftd_gpu_tests` so `ctest -L gpu` runs them sequentially
instead of contending on one RTX 5090. No source changes, only the
CMakeLists.txt GPU-label list.

### Registration counts (`ctest --show-only=json-v1`)

| Metric | Value |
|---|---|
| Total registered CTest tests | **175** |
| Source files in `engine/tests/` | **185** (117 `test_*.cpp` + 63 `campaign_*.cpp` + 5 `benchmark_*.cpp` + 0 `*.cu`) |
| Source-file / ctest delta | **10** (see §5 for reconciliation) |
| `ftd_add_test()` adopters | **3** of 175 (1.7%) |
| `ftd::test::*` NDJSON adopters | **3** of 185 (1.6%) |

### Label distribution (post-Wave 1)

| Label | Count | Notes |
|---|---|---|
| `unit` | 100 | Auto-applied by `ftd_add_test` to `test_*`; legacy tests via `set_property APPEND` block |
| `campaign` | 49 | Same pattern for `campaign_*` |
| `gpu` | **15** (was 4 pre-Wave-1) | 4 explicit `gpu_*` + 11 latency-dependent EM/dispersion/DS tests serialized by Wave 1 |
| `foundation` | 10 | Foundational physics: constants, lattice, discrete_operators, etc. |
| `scale2` | 9 | Atomic engine: atom_engine, atom_scale_bridge, ae_* |
| `lagrangian` | 5 | variational_coulomb, magnetic_lagrangian, dissipation, action_stationarity, lagrangian |
| `scale1` | 4 | Particle engine: particle_engine, pe_forces, campaign_pe_fine_structure, particle_toggles |

### Test timing (Wave 1 baseline)

| Pass | Mode | Tests | Wall clock | Passed | Failed | Segfault |
|---|---|---|---|---|---|---|
| **Pass 1 (CPU)** | `ctest -j 24 -LE gpu` | 160 | 379.94 sec (6.3 min) | **89** | 69 | 2 |
| **Pass 2 (GPU)** | `ctest -j 1 -L gpu` | 15 | pending — see §1.1 | pending | pending | pending |

**Pass 1 speedup vs pre-Wave-1**: 790 sec → 380 sec = **52% faster** (the
11 removed timeouts were the wall-clock bottleneck — each worker sat
on one for its full 600s budget).

**Pass 1 pass count unchanged** (89 in both runs): Wave 1 is pure
metadata, no physics or test-source changes. The 11 relocated tests
simply moved from CPU queue to GPU queue; their pass/fail status will
be known after Pass 2.

### 1.1 Pass 2 (GPU serial) — TO BE FILLED IN

After Pass 2 completes, this section will contain:
- 15-test pass/fail breakdown
- Wall clock
- Per-test duration (interesting since these were the tests previously hitting 600s)
- **Combined total** and **pass rate** vs 175

**Expected**: ~4 explicit GPU tests still pass (100%). Of the 11 newly-serialized
tests, serial reruns during the latency audit showed they complete in
141–460 sec with real physics failures (not hangs). One (`campaign_grothendieck`)
previously passed when run serially — potential free `+1`.

---

## 2. Full test inventory by family

This section enumerates every source file in `engine/tests/` and assigns
it to a Phase 1 consolidation family (or "other").

### 2.1 Already-consolidated POC families (commit `6469a33`)

| Family | Consolidated file | LOC | Sections | Check sites | Status |
|---|---|---|---|---|---|
| Tritium algebra (ex-7 `test_trit_*.cpp`) | `test_tritium_algebra.cpp` | 847 | 7 | 172 | PASS (Wave 1) |
| Particle-engine forces (ex-7 `test_pe_*.cpp`) | `test_pe_forces.cpp` | 818 | 7 | 46 | PASS (Wave 1, after `3ca3253` relativistic fix) |

### 2.2 Remaining Wave-4 consolidation families

Each row is a target for the Wave 4 sweep. "Status" is the per-test
pass/fail state in Wave 1 Pass 1.

| # | Family | Files on disk (verified Apr 14) | Target | Wave 1 status |
|---|---|---|---|---|
| 1 | **Energy conservation** | `test_energy`, `test_energy_conservation`, `test_energy_tracking` (3) | `test_energy_conservation.cpp` (self-ref) | 2 fail (energy_conservation, energy_tracking) + 1 pass (energy) |
| 2 | **Gauss** | `test_gauss`, `test_gauss_convergence` (2) | `test_gauss.cpp` (self-ref) | 1 fail (gauss) + 1 pass (gauss_convergence) |
| 3 | **Lorentz + Magnetic** | `test_lorentz`, `test_lorentz_force`, `test_lorentz_invariance`, `test_magnetic`, `test_magnetic_lagrangian` (5) | `test_lorentz.cpp` (self-ref) | 3 fail (lorentz_force, lorentz_invariance, magnetic — GPU), 1 pass (magnetic_lagrangian), 1 pass (lorentz) |
| 4 | **ae_* atom engine forces** | `test_ae_angle_strain`, `test_ae_dipole`, `test_ae_electronegativity`, `test_ae_hbonds`, `test_ae_thermostat` (5) | `test_atom_engine_forces.cpp` (NEW) | 2 fail (ae_angle_strain, ae_dipole), 1 SegFault (ae_thermostat), 2 pass (ae_electronegativity, ae_hbonds) |
| 5 | **Quantum correlations** | `test_entanglement`, `campaign_epr_correlation`, `campaign_bell_substrate` (3) | `campaign_quantum_correlations.cpp` (NEW) | 1 fail (entanglement), 2 pass |
| 6 | **Wave dynamics** | `test_wave_speed`, `test_interference`, `campaign_wave_isotropy`, `campaign_two_slit` (4) | `campaign_wave_dynamics.cpp` (NEW) | All pass; note `test_wave_collapse` (separate, fails) |
| 7 | **Dispersion** | `test_dispersion_relation`, `campaign_dispersion`, `campaign_dispersion_convergence` (3) | `campaign_dispersion.cpp` (self-ref) | All 3 are GPU-labeled now; see §1.1 |
| 8 | **Coulomb force law** | `test_poisson_coulomb`, `campaign_coulomb_convergence`, `campaign_force_law`, `campaign_poisson_force_law`, `campaign_poisson_binding` (5) | `campaign_coulomb_force_law.cpp` (NEW) | 4 fail + 1 pass (coulomb_convergence) |
| 9 | **Hydrogen spectrum** | `test_hydrogen_scale1`, `test_hydrogen_em_only`, `test_hydrogen_spectrum_scale1`, `campaign_poisson_hydrogen`, `campaign_hydrogen_spectrum` (5) | `campaign_hydrogen_spectrum.cpp` (self-ref — two-step commit) | 2 fail (hydrogen_em_only, poisson_hydrogen), 3 pass |
| 10 | **QCD forces** (user-confirmed: 5 real campaigns on disk) | `campaign_color_force`, `campaign_color_neutral`, `campaign_confinement`, `campaign_baryon_formation`, `campaign_gluon_dynamics` (5) | `campaign_qcd_forces.cpp` (NEW) | 4 fail (color_force, color_neutral, confinement, gluon_dynamics) + 1 ? (baryon_formation) |
| 11 | **Dark sector** | `campaign_dark_sector` + 6 × `campaign_ds_*` (correlation_function, information_cascade, phase_recovery, ternary_detector, void_classification, vortex_lines) = **7 files** | `campaign_dark_sector.cpp` (self-ref — 7-section merge) | 3 fail (dark_sector, ds_information_cascade, ds_ternary_detector) + 2 GPU-labeled (ds_vortex_lines, ds_correlation_function) + 2 pass (phase_recovery, void_classification) |

**Total old files in families**: 2 (POC done) + 46 (Wave 4 target) = **48 files**
**Total new files**: 2 (POC) + 11 (Wave 4 target) = **13 files**
**Net reduction when Wave 4 completes**: 48 → 13 = **−35 files**

### 2.3 Corrections to HANDOFF.md family table

| Claim in HANDOFF.md | Reality on disk |
|---|---|
| `ae_*` = "6 files" | Disk shows 5 `test_ae_*` + 1 `campaign_ae_water`. If `campaign_ae_water` counts, it's 6 total but belongs in a different scale2 cohort. Wave 4 merges the 5 `test_ae_*`. |
| QCD = `campaign_color_force` + `campaign_strong_force` + `campaign_exchange_force` | `campaign_strong_force.cpp` and `campaign_exchange_force.cpp` **do not exist**. User-confirmed replacement: the 5 real files above. |
| Dark sector = "7× `campaign_ds_*`" | Only **6** `campaign_ds_*.cpp` exist; the 7th file is `campaign_dark_sector.cpp` (different name). Wave 4 merges all 7. |
| Hydrogen target = `campaign_hydrogen_spectrum.cpp` | Target name collides with an existing source file (one of the 5 inputs). Requires two-step or in-place overwrite commit. |
| `campaign_hydrogen_binding.cpp` | Not in HANDOFF's family list but exists on disk and currently fails. Decision: fold into Hydrogen family (6 files → 1) OR leave separate. Deferred to Wave 4. |
| Coulomb force law = 4 files | `campaign_poisson_binding.cpp` also fails and is topically Coulomb/Poisson-adjacent. This doc adds it as family member #5 (subject to Wave 4 review). |

---

## 3. Failure classification — every non-passing test in exactly one bucket

### 3.1 CPU Pass 1 non-passing (71 total)

#### Bucket A: Latency-downstream EM / Poisson / gravity (16 tests)
Root cause: EM/gravity pipeline is downstream of the latency Poisson
solver. AUDIT_LATENCY_2026_04_14.md fixed 5 tests via `force_cpu()` +
sign convention flip; more tests likely need the same pattern.
**Wave 3.2** (force_cpu() holdouts) and **Wave 4** (per-family fixes) target these.

- `gauss`, `gravity_dynamics`, `intervoxel_coupling`, `einstein_equations`
- `em_fields`, `larmor`, `light`, `thomson_scattering`
- `campaign_gravity_profile`, `campaign_gravity_hierarchy`
- `campaign_poisson_force_law`, `campaign_poisson_binding`, `campaign_poisson_hydrogen`, `campaign_force_law`
- `poisson_coulomb`, `hydrogen_em_only`

#### Bucket B: Real physics failures in EM / SM / QM / QCD sectors (31 tests)
Require per-family engine-expert / ftd-lead-physicist diagnosis in
Wave 4. These are legitimate unresolved physics bugs or sign/normalization
drifts, not infrastructure.

- **Particle/atomic forces (10)**: `lorentz_force`, `lorentz_invariance`, `momentum`, `selective_damping`, `wavepacket`, `vortex`, `portable_field`, `bridge_dynamics`, `dual_substrate`, `multiscale_bridge`
- **SM / heavy physics (11)**: `electroweak`, `higgs_mechanism`, `wz_mass`, `flavor_physics`, `campaign_hydrogen_binding`, `campaign_shell_predictions`, `campaign_sm_observables`, `campaign_color_force`, `campaign_color_neutral`, `campaign_confinement`, `campaign_gluon_dynamics`
- **QM / dark sector (6)**: `bell_aggregate`, `born_rule_ensemble`, `entanglement`, `hilbert`, `ensemble`, `campaign_dark_sector`, `campaign_ds_information_cascade`, `campaign_ds_ternary_detector`
- **Other (4)**: `thermodynamics`, `logic_engine`, `gauge`, `wave_collapse`

#### Bucket C: Atomic engine (ae_*) and related (5 tests)
- `ae_angle_strain`, `ae_dipole` (failures) — Wave 4 family #4
- `ae_thermostat` — **PRE-EXISTING SEGFAULT** (not a Wave 1 regression — confirmed by `audit_fresh_cpu.log`)
- `campaign_ae_water` — separate scale2 test, not in Wave 4 family
- `energy_conservation`, `energy_tracking` — Wave 4 family #1

#### Bucket D: Benchmark-assertion-subset failures (9 tests)
Tests whose internal assertions are mostly passing but some fail; the
binary exits non-zero overall. The 5 `benchmark_*.cpp` files themselves
(Wilson, engine_theory, emergent_alpha, budget, BH thermo) are **NOT**
in ctest (Wave 3.1 registers them).

- `campaign_aggregate_interaction`, `campaign_weak_transmutation`, `campaign_parity_violation`
- `campaign_triad_binding`, `campaign_multiscale_pipeline`
- `confinement_test`, `triad_confinement`, `asymptotic_freedom`, `baryogenesis`

#### Bucket E: TEST_DEVIATION_MAP documented deviations (6 tests)
Per `engine/tests/TEST_DEVIATION_MAP.md`, these are by-design divergences
from SM expectations; they would require reframing the assertions to
pass, not physics fixes. They should be surfaced in this audit as
"documented deviation" and **NOT** force-fixed in Wave 4.

Candidates (needs per-test verification during Wave 4):
- `lagrangian` (action stationarity derivation) — deviation map §10
- `action_stationarity` (same)
- `annihilation_conservation` (TDM §2 anti-correlation reframing)
- `genesis`, `campaign_gauge_dynamics`, `campaign_gauge_constraint` (emergent/phenomenological)

**Note**: TDM entries reference GPU test assertion IDs (GPC-*, HERTZ-*, GP-*)
not the CPU ctest binary names. The mapping is approximate; during Wave 4
each candidate should be verified against TDM before deciding fix vs. reframe.

#### Bucket F: New WIP regression (1 test)
- `helium_scale1` — segfault at `Test #54`, 0.56 sec. Wave 3.3 deep triage.

### 3.2 GPU Pass 2 non-passing
Pending — section 1.1 will be filled in after Pass 2 completes.

### 3.3 Pre-existing segfaults not blocked by Wave 4
- `helium_scale1` (Wave 3.3 will triage)
- `ae_thermostat` (Wave 4 family #4 will handle as part of the consolidation; if physics fix is nontrivial, split into a dedicated commit)

---

## 4. Instrumentation adoption (`ftd::test::*` NDJSON API)

### 4.1 Adopters (post-POC, pre-Wave-4)

| File | Check sites | Sections | Used by |
|---|---|---|---|
| `test_telemetry_selftest.cpp` | 5 | 5 | Phase 2a self-test (commit `3801e49`) |
| `test_tritium_algebra.cpp` | 172 | 7 | Phase 1+2b POC (commit `6469a33`) |
| `test_pe_forces.cpp` | 46 | 7 | Phase 1+2b POC (commit `6469a33`) |

**Total: 3/185 files = 1.6%**.

### 4.2 Wave 4 target adoption

After Wave 4 completes, adoption will be:
- 3 existing + 11 new consolidated files (Wave 4 targets) = **14 adopters**
- Percentage: 14/~150 (post-Wave-4 file count) ≈ **9.3%**
- Plus Wave 3.6 optional: `test_latency_field.cpp` (+1)

The remaining ~135 tests (Phase 2b sweep) are deferred per the approved plan.

---

## 5. Orphan reconciliation — where do the 10 extra source files go?

`185 source files − 175 ctest entries = 10`. None are "true orphans";
all have explanations:

| Bucket | Count | Files / pattern |
|---|---|---|
| **Benchmarks** (`add_executable` but no `add_test`) | **5** | `benchmark_engine_theory`, `benchmark_emergent_alpha`, `benchmark_budget_equation`, `benchmark_black_hole_thermo`, `benchmark_wilson_loops` — Wave 3.1 registers them |
| **POC inputs** (already consolidated and deleted on disk) | 0 | The 7 `test_trit_*.cpp` + 7 `test_pe_*.cpp` files are already gone; confirmed in §2.1 |
| **Conditional compilation / CUDA-only** | ~5 | `gpu_parity`, `gpu_benchmark`, `gpu_physics`, `gpu_experiments`, possibly others — registered only under `FTD_ENABLE_CUDA` |

**Conclusion**: Zero true orphans. Every source file on disk is either
registered in ctest or explicitly excluded. Wave 3.1 closes the
benchmark gap.

---

## 6. Infrastructure findings feeding Wave 3

### 6.1 Benchmark registration gap (Wave 3.1)
5 `benchmark_*.cpp` files build executables but never run in ctest.
Converting the `add_executable` trio at `engine/CMakeLists.txt:460–473`
to `ftd_add_test(... CTEST_NAME benchmark_... TIMEOUT 1800)` fixes this
and auto-applies the `benchmark` label.

### 6.2 force_cpu() holdouts (Wave 3.2)
Bucket A in §3.1 lists 16 latency-downstream candidates. Grep reveals
`RenderBridge` construction sites. The AUDIT_LATENCY_2026_04_14.md
pattern (calling `rb->force_cpu()` after construction) fixed 5 known
targets; remaining candidates that need verification:

- `test_gravity_dynamics`, `test_intervoxel_coupling`, `test_gauss`
- `test_em_fields`, `test_larmor`, `test_light`, `test_thomson_scattering`
- `campaign_gravity_profile`, `campaign_gravity_hierarchy`
- `campaign_poisson_*` (4 tests)

Expected impact: up to +4 tests moving from Fail to Pass.

### 6.3 LABELS APPEND status
Commit `d7d32e4` fixed all 5 category blocks (unit/campaign/foundation/lagrangian/scale1/scale2)
to use `set_property APPEND`. **No holdouts remain.** Verified in §1
label distribution — `pe_forces` shows `['scale1', 'unit']`, `lagrangian`
shows `['lagrangian', 'unit']`, etc.

### 6.4 helium_scale1 (Wave 3.3)
- Entry point: `Test #54` in Wave 1 Pass 1, segfault in 0.56 sec
- Known: crash is past `particles()[2].r_eff = 0.01`
- Known: Barnes-Hut `(0,0,0)` 2-proton degeneracy ruled out (offsetting by `1e-10` didn't fix it)
- Unknown: whether crash is in diagnostics path or in first `tick()`
- Strategy: stderr bisect through main(), then engine-expert subagent if past diagnostics

### 6.5 ae_thermostat pre-existing segfault
Not previously documented. Should be folded into Wave 4 family #4
(`ae_*` atom engine forces) as a separate triage step before or after
the consolidation.

---

## 7. Corrected family table — superseded claims

See §2.2 and §2.3 above. The principal HANDOFF.md errors:

1. QCD family files that don't exist on disk (`campaign_strong_force`, `campaign_exchange_force`)
2. ae_* count: "6 files" should be "5 `test_ae_*`" (plus `campaign_ae_water` handled separately)
3. Dark sector: "7× `campaign_ds_*`" should be "6 `campaign_ds_*` + 1 `campaign_dark_sector`"
4. `campaign_poisson_binding.cpp` is Coulomb-family adjacent but not listed
5. `campaign_hydrogen_binding.cpp` exists and fails but isn't in any HANDOFF family
6. Hydrogen target name collision (source = target)

---

## 8. Index of superseded / historical docs

These three files remain on disk as historical snapshots. They each get
a 1-line header note pointing here.

| Doc | Phase | What it captures |
|---|---|---|
| `HANDOFF.md` | Phase 0–8 + POC | Branch handoff for the test-runner worktree merge. Lists what was shipped, what was deferred, family table (with the errors now corrected in §2.3). |
| `FINAL_ENGINE_REPORT.md` | Post-WIP merge | Post-merge ctest baseline (92/175, then 93/175, then 94/175 post-audit). Failure categorization by physics sector. |
| `AUDIT_LATENCY_2026_04_14.md` | Latency audit | Root-cause analysis of the latency-field sign bug + GPU-contention false-timeouts. Documents the 5 `force_cpu()` fixes applied. |

---

## 9. Session checkpoint log

This section tracks the "audit the tests and fix them" session progress.
Updated after each wave.

### Wave 1 (commit `8d7ed60`)
- Committed the staged `_ftd_gpu_tests` append (11 new labels)
- Label count: 4 → 15
- Pass 1 wall clock: 790s → 380s (52% faster)
- Pass count: unchanged (89 on CPU side)
- Pass 2 results: TO BE FILLED IN

### Wave 2 (this commit)
- Wrote `TEST_AUDIT_2026_04_14.md`
- Added superseded headers to HANDOFF / FINAL_ENGINE_REPORT / AUDIT_LATENCY

### Wave 3 (pending)
- 3.1: benchmark registration (+5 ctest entries)
- 3.2: force_cpu() holdouts
- 3.3: helium_scale1 deep triage
- 3.4/3.5/3.6: timeout, label check, optional instrumentation

### Wave 4 (pending)
- 11 family consolidations, one commit each
- Consolidate-and-fix-physics in same commit for Bucket A and Bucket B tests

---

**Document produced**: 2026-04-14, during Wave 2 of the
"audit the tests and fix them" session.
**Data sources**: `ctest --show-only=json-v1`, `/tmp/ctest_wave1_cpu.log`,
`engine/tools/test_runner/{HANDOFF,FINAL_ENGINE_REPORT,AUDIT_LATENCY_2026_04_14}.md`,
direct filesystem inventory of `engine/tests/`.
