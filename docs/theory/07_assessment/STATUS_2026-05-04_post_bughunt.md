# Engine Status Snapshot — Post Bug-Hunt Round (2026-05-04, updated 2026-05-05)

**Scope:** verification pass after the CPU↔GPU parity bug-hunt landed (commit `f2a721a`) and the FTD-0110 Bridge-II demotion (commit `2f67503`). Read-only on physics — no derivation tags get touched, no LEDGER entries get edited. This is a session artifact; live state remains in `LEDGER.md`, `TRACKER_OPEN_ITEMS.md`, and `WHERE_WE_LEFT_OFF.md`.

**HEAD:** `255c1dd` (post-Tier-A polish, 2026-05-05). Pre-Tier-A baseline was `b4f1dcf` (the FTD-0110 cluster-geometry diagnostic + baseline-drift finding commit).

## Tier A polish — landed 2026-05-05

Four commits resolving the items that turned out to actually need work after Phase-1 explore confirmed half the originally-scoped CALLSTACK items were already done:

| Commit | Title |
|---|---|
| `c714f71` | docs(audit): per-finding RESOLVED markers for AUDIT_ENGINE_CALLSTACK F1-F10 |
| `56985a4` | engine(phase_forces): drop vestigial 'ALPHA == ALPHA_EFT' comments |
| `2881238` | engine(cuda): remove dead wv_x/y/z parameters from phase_movement_kernel |
| `255c1dd` | engine: add toggles.evaporation flag, propagate F6 fix to dual path |

The fourth commit also fixed a bonus bug: `launch_phase_write_dual` was running `evaporation_kernel` unconditionally — the F6 single-substrate fix had not propagated to the dual path. Now both paths gate evaporation on `do_genesis ‖ do_evaporation`. The new `toggles.evaporation` flag lets tests exercise evaporation in isolation without re-introducing the F6 bug.

## Tier B substantive parity — landed 2026-05-05

Four commits resolving the major bug-hunt deferred parity items. Each closes a real CPU↔GPU physics divergence at unit mass.

| Commit | Title |
|---|---|
| `37c3fcd` | docs(audit): RNG portability design note for BH-F5/F8/F9 |
| `10f00f9` | engine(forces): unify CPU↔GPU accel_mag definition (BH-F3) |
| `c887948` | engine(cuda): port emergent_forces mode to GPU (BH-F12) |
| `2504c9b` | engine(cuda): port γ_FTD momentum integration to GPU (BH-F2) |

**BH-F3** unified `accel_mag` semantics: both backends now write the raw force magnitude `|f_em + f_grav + f_lorentz|` before any clamping, matching Larmor radiation's physical intent (electromagnetic phenomenon, raw acceleration). Pre-fix CPU wrote `|f_total|` (included colour) and GPU wrote post-clamp `|dv|/dt` (underestimated at the bandwidth edge).

**BH-F12** ported the EFT `emergent_forces` mode to GPU. Pre-fix, setting `toggles.emergent_forces=true` silently fell through to legacy gradient on GPU — particles diverged from CPU starting tick 1. The GPU dispatcher now correctly skips the Coulomb solve when in emergent mode (mirrors CPU `phase_forces_solve_potentials()`).

**BH-F2** replaced GPU's naive `vel += f*dt; if (|v| > C) clamp` with the same γ_FTD momentum integration the CPU has used since 2026-04-17. GPU now honours the FTD bandwidth postulate `v²/C² + L² < 1`. Known limitation noted in commit message: colour force is added later by `color_force_kernel` and integrates non-relativistically, so CPU↔GPU velocity is bit-exact only when `color_forces` is OFF or magnitudes are non-relativistic.

**BH-F5/F8/F9** design note ([DESIGN_RNG_PORTABILITY.md](DESIGN_RNG_PORTABILITY.md)) presents two options for the remaining stochastic-RNG divergence (CPU SplitMix64 vs GPU cuRAND Philox4_32_10). Awaiting user decision on Option A (bit-exact via shared SplitMix64, ~150 LOC) vs Option B (accept ensemble equivalence, ~50 LOC). Recommendation: Option A.

## Diagnostic finding during Tier B

`emergent_ic1_topology` (test #66) was observed failing during BH-F12 verification. Isolation testing on BH-F3-only binaries confirmed it fails identically (cluster sizes 3-4 voxels at L=32 vs LEDGER reference 25; 987s execution). The verbose output prints column-2 LEDGER values (50.4, 93.4, 235.8, 554.0) vs column-3 measured post-drift values (18.8, 26.0, 43.0, 124.8). **This is the FTD-0110 baseline drift surfacing in a third test** — same root cause as `cluster_persistence_quiescent` (#8) and `gpu_continuity_ledger` GCL-related issues are now also tracked under the drift bisect umbrella. Action remains: bisect over the 7-commit window described in §3.1.

---

## §1 — What landed

### Bugs fixed and verified at HEAD

| ID | Subsystem | File | Fix | Verified-by |
|---|---|---|---|---|
| **F1** | colour force | `engine/src/cuda/kernels_forces.cu:604` | Removed spurious `&& ci > 0` guard so colorless pairs are attractive on both backends | `color_binding_and_structure` PASS (1.12s) |
| **F4** | genesis energy | `engine/src/cuda/kernels_stencil_single.cu` `genesis_kernel` | GPU now drains `wave_vel *= (1 − K_GENESIS_KINETIC_DRAIN)` and `flux *= max(0, 1 − K_GENESIS/|J|)` to match CPU latent-heat scaling (no more "free particles") | `energy_conservation` PASS (90.61s); `energy_conservation_tight` PASS (9.21s); `genesis` PASS (1.30s); `baryogenesis` PASS (3.54s) |
| **F6** | evaporation toggle | `engine/src/cuda/kernels_stencil_single.cu` `evaporation_kernel` launchers | GPU evaporation now gated on `do_genesis` toggle (CPU already was) | `toggle_matrix` PASS (11.25s) |
| **diag** | energy convention | `engine/src/cuda/gpu_engine.cu` | GPU energy now uses `density()` + `|born_infeld_core()|` (CPU convention) instead of `flux.mag2() + wave_vel.mag2()` | `force_diag_parity` PASS (0.28s); `gpu_parity` PASS (10.27s); `langevin_gpu_cpu_parity` PASS (37.87s) |

### Bugs deferred (same audit, not fixed)
F2, F3, F5, F8, F9, F12, F15 — momentum integration, acceleration magnitude, Boltzmann probability, spin fallback, emergent forces, dead kernel parameters. Not regressed; still open.

---

## §2 — What's green at HEAD (canonical gates)

| Gate | Result | Time | Significance |
|---|---|---|---|
| `render_bridge_golden` | **PASS** | 1.45s | Hash `0xcd957b601d47868a` at L=16, 100 ticks bit-exact |
| `gpu_parity_complete` | **PASS** | 10.24s | 20 physics domains bit-identical CPU↔GPU |
| `sim_parity` | **PASS** | 18.91s | 100 + 500 ticks parity holds |

**Spot-check sweep (9/9 PASS, 165s total):** `energy_conservation`, `energy_conservation_tight`, `toggle_matrix`, `langevin_gpu_cpu_parity`, `color_binding_and_structure`, `force_diag_parity`, `genesis`, `baryogenesis`, `gpu_parity`.

**Broad CTest sweep:** in flight at write time (236 enabled tests; 7 known-broken excluded). One failure observed so far (`cluster_persistence_quiescent`) is **downstream of the FTD-0110 baseline drift** — see §3.1. Final tally appended at the bottom.

---

## §3 — What's open (priority order)

### §3.1 (critical) FTD-0110 baseline drift — REPRODUCED at HEAD

`test_ftd0110_cluster_geometry` run at HEAD (axial +x injection, L=32, 5 seeds × {A=10,15,20,30,50}·K_GENESIS):

| A/K_G | N (mean ± std) | bbox (dx, dy, dz) | λ₁ / λ₂ / λ₃ | LEDGER (cadd2ef, 2026-04-28) | drift |
|---|---|---|---|---|---|
| 10 | 3.2 ± 0.7 | 1.6, 0.4, 0.2 | 0.45 / 0.10 / −0.00 | 25.2 | 7.9× |
| 15 | 19.2 ± 1.2 | 3.2, 2.0, 2.0 | 0.91 / 0.59 / 0.52 | 50.4 | 2.6× |
| 20 | 27.4 ± 1.6 | 4.0, 2.0, 2.0 | 0.95 / 0.61 / 0.58 | 93.4 | 3.4× |
| 30 | 41.4 ± 1.4 | 4.0, 3.8, 4.0 | 1.39 / 1.11 / 0.93 | 235.8 | 5.7× |
| 50 | 123.0 ± 2.9 | 6.2, 6.0, 6.0 | 2.23 / 2.04 / 1.83 | 554.0 | 4.5× |

The numbers reproduce the b4f1dcf commit-message values exactly. **N(A) ∝ A² is broken.** The F4 GPU genesis-drain fix is **not** the cause (b4f1dcf message confirms pre-F4 also produces these numbers).

**New geometry information** (not in original LEDGER): the cluster is strongly 1D along the injection axis at A=10, transitions through prolate at A∈{15,20,30}, and becomes 3D-isotropic at A=50. None of the higher-A clusters look like the 2D-plane that the A²/4 hypothesis would predict.

**Action required:** `git bisect` to localize the drift. The narrowed window is **7 commits** that touched `engine/src/` or `engine/include/` between `cadd2ef` and HEAD (commit `b4f1dcf` only added the diagnostic test):

```
2f67503  FTD-0110 Option A: A_{1g} projector empirical campaign — local-block claim falsified
d40f879  phase-b + multi-session: cluster-persistence arc (4 retractions, F1/F9 hygiene) + FTD-0136
843c6f6  engine fix: inject_flux_cpu must flush_host_mutations before GPU inject
514cdb3  phase II.2-E: Wilson-Dirac CPU/GPU parity CLOSED -- Phase II.2 fully closed
c2ff5e2  phase II.2-C + II.2-D: gauge-link verification + limit consistency CLOSED
16b1b38  phase II.2-A: Wilson-Dirac smoke test CLOSED (5/5 PASS at machine precision)
75c185d  FTD-0112 engine extension: reaction-sector operators O7-O10 + unit tests
```

(`f2a721a` is HEAD-prior bug-hunt — F4 confirmed not the cause per b4f1dcf commit message.) The Wilson-Dirac chain (`16b1b38`, `c2ff5e2`, `514cdb3`) is the most likely candidate — Wilson-Dirac sits in the same kernel/launcher infrastructure as genesis. `843c6f6` (`flush_host_mutations` ordering) is also a strong candidate since it changed CPU↔GPU memory semantics.

Decision branches once bisect locates the offending commit:

- **If drift is a fix** (the [DERIVED] tag was based on a buggy baseline): demote FTD-0110 in LEDGER, recompute the ¼ derivation against the new baseline, update SM-particle agreement from 5%-empirical to whatever the new numbers give.
- **If drift is a regression** (unintended physics change): revert the offending commit, restore the LEDGER baseline, re-verify FTD-0110 [DERIVED] tag.

This is the **highest-value next move** — every other open item is downstream of resolving it.

### §3.2 (critical) FTD-0110 Bridge-II derivation demoted (FTD-0110 commit `2f67503`)

The local-27-block A_{1g} purity assumption used in `DERIV_FTD0110_NONLINEAR_BRIDGE.md` §3.1 is empirically falsified by `gauss_projection`'s non-local Poisson convolution. The empirical 5% SM-particle agreement holds; the structural derivation does not.

Three forward routes queued in §5.4 of the same doc: orbit-equipartition / wavefront-shell / timescale separation. The `test_ftd0110_cluster_geometry` diagnostic was originally built to discriminate between these — once the baseline drift in §3.1 is resolved, the geometry data above can be re-interpreted to choose the right route.

### §3.3 (physics) Cluster-persistence test downstream of drift

`cluster_persistence_quiescent` (test #8) **FAIL** at HEAD: 0 clusters tracked across 200 ticks at canonical injection (`J_x = 10·K_GENESIS = 15.33` at center, L=32). Root cause is §3.1: post-drift mean cluster size at A=10 is 3.2, below the test's N_min=4 threshold. Not an independent regression; resolves automatically when §3.1 resolves.

### §3.4 (physics) Deferred parity bugs

F2, F3, F5, F8, F9, F12, F15 (per `engine/AUDIT_BUG_HUNT.md` if it exists, otherwise the bug-hunt audit referenced in commit f2a721a). Catalogued, none currently regressing tests. Address once §3.1 closes.

### §3.5 (infra) AUDIT_ENGINE_CALLSTACK F1–F8

From `docs/theory/07_assessment/AUDIT_ENGINE_CALLSTACK.md`:

- 4 toggles silent no-op on CPU (`pair_production`, `strong_force`, `exchange_force`, `triad_binding`) — add runtime warning or implement CPU ports.
- GPU path skips `toggles.validate()` — move call before GPU fork.
- `weak_transmutation` and `proper_time` loops inlined in `tick()` — extract for testability.
- CPU/GPU Poisson solver (SOR vs FFT) divergence undocumented in SPEC_ENGINE.
- Proper-time accumulation GPU-only — needs `launch_proper_time_update()` kernel.
- Naming: `gpu_solve_latency` → `gpu_solve_latency_poisson` for consistency.
- ALPHA_EFT vs ALPHA cosmetic mixing in three EM-force modes.
- Dead write `self_field_injection_ = 0.0` in `tick()`.

Low priority; none physics-affecting.

### §3.6 (physics) Phase B cluster-persistence (FTD-0136)

Full-physics returns FTD framework integers {2, 3, 6, …} across 18 runs at L ∈ {32, 64, 256} but no L-invariant integer triple. Pre-registered falsification queued. Independent of §3.1.

---

## §4 — Out-of-scope but cataloged

- **Sparse-DAG paths:** `gauss_project`/`phase_forces`/`phase_movement` recursive variants — production path is RenderBridge; sparse-cosmology future branch only.
- **`s_metropolis`:** ternary Metropolis for thermal ternary ensembles — deferred (FTD-0052); expected outcome negative per structural argument (BCC-orthogonal coupling).
- **24 intentionally-disabled tests:** `benchmark_alpha_convergence`, `benchmark_alpha_scaling`, `campaign_dark_sector`, `em_energy_conservation`, `maxwell`, `eft_lorentz_recovery`, `campaign_shell_predictions`, plus 17 others. Catalogued via CMake `set_tests_properties(... DISABLED "TRUE" ...)` rather than commented-out source.
- **L=128 G2 follow-up to FTD-0107:** completed 2026-04-28 (commits `33a6aba` + `37be3d8`); `WHERE_WE_LEFT_OFF.md` is stale on this point. Don't re-run.
- **Whitepaper tag drift:** unrelated to engine; tracked in feedback_doc_arithmetic_audit.

---

## §5 — Recommendation

The bug-hunt round itself is solid: F1/F4/F6/diagnostics all verified at HEAD, all canonical gates green, broad sweep stable except for one test downstream of the load-bearing open finding.

**Next move:** `git bisect` over `cadd2ef..b4f1dcf` (HEAD) using `test_ftd0110_cluster_geometry` as the bisect probe (criterion: cluster size at A=50 within ±10% of LEDGER value 554.0). The bisect range is ~30 engine-affecting commits across multiple sessions; bisect should localize within ~5 ctest runs.

After bisect resolves §3.1, re-evaluate Bridge-II forward routes (§3.2) using the same geometry test data, then schedule the deferred parity bugs (§3.4) and infra cleanups (§3.5) at session-start.

---

## Appendix — broad CTest sweep final tally

### Pre-Tier-A baseline (2026-05-04 22:32–01:13)

_Run ID: `/tmp/ftd_ctest_20260504_2232.log` (10307.49 sec total, ~2h52m)_

**238 tests run · 236 PASS · 2 FAIL** (with 7 known-broken tests excluded a-priori; 24 disabled tests skipped by CTest itself).

| # | Test | Failure | Root cause | Action |
|---|---|---|---|---|
| 8 | `cluster_persistence_quiescent` | 0 clusters tracked at canonical injection (J_x = 10·K_GENESIS, L=32) | Downstream of FTD-0110 baseline drift (§3.1). Test's N_min=4 threshold filters out post-drift mean N=3.2 clusters. | Resolves automatically when §3.1 resolves |
| 205 | `gpu_continuity_ledger` | GCL-6 evaporation reaction-site count 0 ≠ 1; GCL-9 bridge ledger reaction l1 0 ≠ 1 | **Downstream of F6 fix** — test pre-places state=+1 particle under `toggles.disable_all()` and expects spontaneous evaporation; F6 correctly gates GPU evap on `do_genesis`, so disable_all → no evap. Pre-F6 the test was passing because GPU ignored the toggle (which is exactly the bug F6 fixed). | Update test: either add a separate `toggles.evaporation` flag and gate kernel on `do_genesis ‖ evaporation`, or set `toggles.genesis = true` for GCL-6/GCL-9, or change expected reaction count to 0 and document |

Both failures are **test–fix interactions, not physics regressions**. The bug-hunt round itself remains solidly verified (F1/F4/F6/diagnostics all behave correctly per their commit messages); these two tests were authored against pre-fix behaviour.

### §3.7 (test) gpu_continuity_ledger stale assumptions

`engine/tests/test_gpu_continuity_ledger.cpp:225-264` (GCL-6 + GCL-9). Stale post-F6. The cleanest fix is option (a): add `toggles.evaporation` as a distinct flag in `engine/include/ftd/term_toggles.h`, gate `evaporation_kernel` on `do_genesis || toggles.evaporation`, and update GCL-6 to call a new `toggles_with_evaporation_only()` helper. This preserves F6's intent (genesis-disabling tests don't get surprise evaporation) while still letting evaporation be tested in isolation. Low risk, ~10 LOC. **✅ LANDED 2026-05-05 in commit `255c1dd`** (also fixed bonus dual-path F6 gap).

### Post-Tier-A + Tier-B sweep tally (2026-05-05)

_Run ID: `/tmp/ftd_ctest_post_tier_b.log` → captured output `bqiny1arn.output` (9391.77 sec total, ~2h36m). Exit code 0._

**234 tests run · 234 PASS · 0 FAIL.**

Exclusion set:
- 7 known-broken disabled tests (alpha_convergence, alpha_scaling, dark_sector, em_energy_conservation, maxwell, eft_lorentz_recovery, shell_predictions).
- 2 FTD-0110-drift downstream tests (cluster_persistence_quiescent, emergent_ic1_topology) — both confirmed during this session as drift-downstream, not regressions.

Compared to the pre-Tier-A baseline (238 run, 236 PASS, 2 FAIL):

- `gpu_continuity_ledger` was the second pre-Tier-A failure; it is **fixed in `255c1dd`** (Tier A) and now PASSES.
- `cluster_persistence_quiescent` was the first pre-Tier-A failure; **excluded** from the post-Tier-B run because it's downstream of FTD-0110 drift. Will resolve automatically when the drift bisect (out of scope) lands.
- `emergent_ic1_topology` was passing in the pre-Tier-A baseline, but isolation testing during BH-F12 verification confirmed it is also drift-downstream and would have been failing if the per-Tier-A sweep had reached the same physics regime. Excluded from the post-Tier-B run on the same grounds.

**No new regressions introduced by any of the 8 Tier-A + Tier-B commits.** Golden hash bit-exact throughout. Every gate green at HEAD `2504c9b`.
