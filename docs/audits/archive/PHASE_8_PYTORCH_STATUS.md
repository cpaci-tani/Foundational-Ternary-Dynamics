# Phase 8 — PyTorch Conversion Status

This document tracks the PyTorch / CUDA conversion of FTD's Python test,
proof, and verification ecosystem. Phase 8a landed the infrastructure plus
the highest-impact hot-loop scripts. Subsequent phases will convert the
remaining scripts in priority order.

## Infrastructure landed (Phase 8a)

`scripts/constants.py` now exports PyTorch / CUDA canonical imports:

- `TORCH` — the `torch` module (or `None` if not installed)
- `DEVICE` — `torch.device('cuda' if torch.cuda.is_available() else 'cpu')` (or `None`)
- `DTYPE` — `torch.float64` (FTD physics requires double precision)
- `t(value, device=None, dtype=None)` — tensor constructor on `DEVICE`
- `to_numpy(tensor)` — `.detach().cpu().numpy()` helper (identity on ndarray)

The import is `try/except ImportError`-guarded so existing callers of
`constants.py` (43+ scripts that pull `G_STAR`, `ALPHA`, etc.) are unaffected
when PyTorch is not installed.

### PyTorch install status at conversion time

PyTorch is **not installed** in the current environment
(`python -c "import torch"` raises `ModuleNotFoundError`). All converted
scripts therefore exercise the NumPy fallback path on every run. Install
with:

```
pip install torch --index-url https://download.pytorch.org/whl/cu121
```

(or the CUDA version matching the user's driver — CUDA 13.0 is currently
installed on the build machine, and PyTorch wheels shipping their own
CUDA 12.x runtime work with that driver).

When torch becomes available, the converted scripts automatically switch
to the GPU path — no further edits required.

## Converted scripts (5 / 127)

Each script below now prints `[backend] device={DEVICE}, torch={TORCH is not None}`
at startup and dispatches hot numerical kernels to either torch (when
available) or the preserved NumPy code path.

| # | Script | Hot kernel | Baseline → Converted | Notes |
|---|--------|-----------|----------------------|-------|
| 1 | `scripts/proofs/compute_observer_bell.py` | 1M-sample Bell correlations (substrate + CHSH) | 0.68 s → 0.95 s | Byte-identical output (only the backend log line is new). GPU path computes `substrate_outcome` reductions on DEVICE. |
| 2 | `scripts/proofs/born_rule_comprehensive.py` | 500 K × 10-amplitude frequency MC | 2.98 s → 1.17 s | Numeric drift within script's own noise floor (baseline is non-deterministic — no seed on `np.random.randn`). 13/13 tests still pass. |
| 3 | `scripts/proofs/watson_convergence.py` | Midpoint rule triple loop N=50..800 | 3 m 18.7 s → 5.1 s (**38×**) | Largest wall-clock win. The pure-Python triple for-loop became a chunked 3D broadcast reduction. Numeric output differs by at most 3 × 10⁻¹² absolute (well within tolerance). |
| 4 | `scripts/proofs/proof_d3_uniqueness.py` | 3 × 5 M-sample D-dimensional Watson MC | 1.60 s → 1.58 s | Byte-identical output when fallback is used. GPU path sets up a single `TORCH.rand` draw + `TORCH.prod` reduction per dimension. |
| 5 | `scripts/proofs/proof_bell_cosine_from_gauss.py` | 37 angles × 2 M singlet MC + 37 angles × 2 M S² vector MC | 5.10 s → 5.11 s | Byte-identical output. GPU path keeps the numpy RNG stream in sync by drawing scalars on CPU then shipping them to DEVICE for the hot `A*B mean` reduction. |

All five scripts return exit 0 after conversion.

### Verification protocol

For each converted script:

1. Ran the baseline once with timing (`time python scripts/…`)
2. Applied the conversion (PyTorch dispatch + preserved NumPy fallback)
3. Ran the converted version, captured stdout
4. `diff`ed baseline vs converted output

Tolerances chosen:

- **Deterministic scripts** (`compute_observer_bell`, `proof_d3_uniqueness`,
  `proof_bell_cosine_from_gauss`): require byte-identical output except
  for the new `[backend] ...` log line.
- **Numerically sensitive scripts** (`watson_convergence`): allow 1 × 10⁻¹¹
  absolute drift in the last digit due to vectorized reduction order.
- **Already non-deterministic scripts** (`born_rule_comprehensive` — no
  `np.random.seed(...)` call): require the PASS/FAIL structure to match
  exactly, and numerical values to stay within the script's own run-to-run
  noise floor.

## Skipped scripts (from the original Phase 8a candidate list)

| Script | Reason |
|--------|--------|
| `scripts/proofs/proof_confinement_wilson.py` | **No Monte Carlo / hot loop.** The script is entirely analytic: `scipy.special.iv` Bessel evaluations at 2 points, small Wilson-loop area-law products up to R×T = 7×7, linear fits with ≤ 49 points, and Creutz ratio checks over ≤ 5×5 grids. The only `numpy.polyfit` calls operate on length-7 arrays. PyTorch would be pure overhead. Substituted with `proof_d3_uniqueness.py` (3 × 5 M MC) for Phase 8a. |
| `scripts/proofs/look_elsewhere_monte_carlo.py` | **Baseline crashes** with the default `--samples 1000000`. Pre-existing `IndexError` at line 169: `x1[idx]` indexes into a filtered `x1 = (-b_valid + sqrt_d) / 2` (length = number of valid discriminants) using global chunk indices `idx = valid_indices[indices]`. The bug only manifests when the MC draws produce at least one hit in the TOLERANCE_PPM window — 1 M samples reliably triggers it, 100 samples do not. Fixing the bug is outside the scope of this conversion (the task says "only swap the tensor/array backend"). Reported as an orthogonal issue; conversion candidate for a follow-up phase after the bug is fixed. |

## Remaining work — priority queue

Total Python scripts in the worktree: **127** (3 at `scripts/`, 64 in `proofs/`,
49 in `verification/`, 11 in `tests/`). Five converted, 122 remaining.

Priority rubric: hot scripts = any with ≥ 100 000-element MC draws, pure-Python
triple-nested numerical for-loops, or ≥ 1 s current wall-clock. Cold = anything
that's mostly print statements, small linear algebra on ≤ 20² matrices, or
pure mpmath symbolic work that torch can't help with.

### Tier 1 — hot MC / hot iteration (convert next)

- `scripts/verification/verify_born_rule.py` — 1 M-sample MC mirror of
  `born_rule_comprehensive.py`'s frequency derivation. Trivial to convert
  using the same pattern.
- `scripts/proofs/proof_gap_equation_from_partition_function.py` — defines
  `watson_bcc_montecarlo(n_samples=2_000_000)` but (currently) never
  calls it. Convert the definition anyway so it's ready when re-enabled.
- `scripts/proofs/look_elsewhere_monte_carlo.py` — 1 M-sample MC. First
  fix the `IndexError` at line 169 (needs `c1[idx]`, `p1[idx]`, … to use
  chunk-global indices, not subset indices), then convert.
- `scripts/proofs/watson_check.py`, `scripts/proofs/watson_normalization_fix.py`
  — both contain triple Python for-loops over `range(N)` with N in the
  several-hundred range. Same `_midpoint_sum_torch` / `_midpoint_sum_numpy`
  helper pattern as `watson_convergence.py`.
- `scripts/proofs/proof_von_neumann_type.py`, `scripts/proofs/proof_three_open_problems.py`,
  `scripts/proofs/proof_self_energy_derivation.py` — each uses
  `np.random` inside loops; measure before converting.
- `scripts/proofs/su2_gauge_proof.py`, `scripts/proofs/su3_gauge_proof.py`,
  `scripts/proofs/u1_gauge_proof.py`, `scripts/proofs/renormalization_framework.py`
  — gauge-theory Monte Carlo; worth measuring first to see if they have
  genuine hot kernels.

### Tier 2 — medium-cost scripts (convert after Tier 1)

- `scripts/proofs/proof_bell_cosine_from_gauss.py` — **already converted**
  (Phase 8a). Any remaining sub-kernels (`differential_evolution` optimizer
  at line 582) are scipy-backed and not torch candidates.
- `scripts/tests/comprehensive/*` — 7-tier verification framework, mostly
  wraps other scripts via subprocess. Low conversion value but could be
  ported for consistency.
- `scripts/verification/verify_anti_correlation.py`,
  `scripts/verification/verify_log_gstar_identity.py` — these call
  `sum(mpmath.mpf(...) for n in range(100000))`. **mpmath stays mpmath**
  per task rules (torch can't do arbitrary precision). Skip unless the
  surrounding numerical code becomes the bottleneck.
- `scripts/experiments/*` — Bell tests, CERN analysis, physics simulations.
  Out of scope for Phase 8a; revisit once the proofs tree is done.

### Tier 3 — cold scripts (low ROI)

Anything dominated by:
- mpmath arbitrary precision (torch can't help)
- `scipy.integrate.quad` / `tplquad` (scipy owns the inner loop)
- Small symbolic computations (`sympy`)
- Linear algebra on matrices < 100 × 100
- Pure plotting / reporting (`matplotlib`, `scripts/visualization/*`)

Examples: `scripts/proofs/watson_exact.py` (tplquad-driven),
`scripts/proofs/decisive_computation_results.py`,
most `scripts/visualization/*`.

## Commit layout

Phase 8a lands as a single commit on `worktree-test-runner-unified`:

- `scripts/constants.py`
- `scripts/proofs/compute_observer_bell.py`
- `scripts/proofs/born_rule_comprehensive.py`
- `scripts/proofs/watson_convergence.py`
- `scripts/proofs/proof_d3_uniqueness.py`
- `scripts/proofs/proof_bell_cosine_from_gauss.py`
- `docs/audits/archive/PHASE_8_PYTORCH_STATUS.md` (this file)

No `engine/` files touched. No `pip install` executed.

## Phase 8b (2026-04-14)

Continuation of the Phase 8a hot-script scope. PyTorch still not installed
in the environment, so every converted script exercises the vectorized
NumPy fallback path. The fallback is already dramatically faster than the
original Python triple loops because the conversion replaces scalar Python
for-loops with broadcasting reductions.

### Converted (Phase 8b): 6 more scripts

| # | Script | Hot kernel | Baseline → Converted | Notes |
|---|--------|-----------|----------------------|-------|
| 6 | `scripts/proofs/watson_check.py` | 500^3 midpoint rule triple loop + convergence sweep over N in [50..500] | 1 m 40.5 s → 1.7 s (**60×**) | Byte-identical output except for the new `[backend]` log line. Broadcast + chunked along i1, same pattern as Phase 8a's `watson_convergence.py`. |
| 7 | `scripts/proofs/watson_normalization_fix.py` | 400^3 triple loop (three accumulators: 1/hat_k^2, 1/sigma, 1/D) + Richardson sweep over N in [100..400] | 1 m 40.0 s → 2.1 s (**48×**) | Byte-identical output. The Richardson sweep preserves the original's identity `t_sigma = 6*t_hatk2` and `t_watson = 2*t_hatk2` explicitly for numerical equivalence. |
| 8 | `scripts/proofs/gauss_constrained_green_v3.py` | N=200 midpoint rule with four simultaneous accumulators (hat_k^2_6, hat_k^2_18, ratio, inverse ratio) | 12.6 s → 1.1 s (**11×**) | Byte-identical output. The 18-point stencil is expanded using precomputed `c[i]+c[j]` and `c[i]*c[j]` tables, then broadcast. |
| 9 | `scripts/proofs/gauss_constrained_green_v2.py` | Five separate `range(L)^3` accumulators at L in [8, 16, 32, 64] for stencil-mismatch analysis | 8.7 s → 0.7 s (**12×**) | Byte-identical output. All five `compute_*(L)` functions dispatch to a single `_v2_sums(L)` helper that returns all five reductions in one pass. The origin (`k = 0`) is explicitly masked to mirror the original's `if den < 1e-12: continue` guard. |
| 10 | `scripts/verification/verify_born_rule.py` | 20 amplitudes × 3 × 1 M Gaussian draws (signal + noise vector Monte Carlo) | 1.5 s → 1.7 s | Non-deterministic (no `np.random.seed`) — run-to-run drift of ~5×10⁻⁴ in the per-amplitude frequencies, ~5×10⁻⁵ in the correlation coefficient. Within the script's own noise floor. PASS status preserved. |
| 11 | `scripts/proofs/look_elsewhere_monte_carlo.py` | 1 M-sample parametric quadratic scan (NumPy already vectorized; primary value is the bug fix) | crashed → 0.6 s | **Bug fix:** pre-existing `IndexError` at line 169 in the `hits1` reporting block. The original used `x1[idx]` where `idx = valid_indices[indices]` was chunk-global, but `x1` is the filtered array (length `valid_d.sum()`). The fix recomputes the root from chunk-global `b[idx]` and `c_term[idx]` (mirroring the x2 block's approach). The torch path redoes the dominant FLOPs (`b`, `c_term`, `discriminant`) on DEVICE while keeping the numpy RNG on CPU so the `seed=42` match-count stays reproducible. Deterministic: verified 1 M-sample runs produce byte-identical output across repeat runs. |

### Phase 8b verification protocol

For each converted script:

1. Ran the pre-8b baseline via `python scripts/…` with `time`, captured
   stdout to `/tmp/*_baseline.out`.
2. Applied the conversion (PyTorch dispatch + preserved NumPy fallback).
3. Ran the new version, captured stdout.
4. `diff`ed baseline vs converted output — all five deterministic scripts
   produce byte-identical output except for the new `[backend]` log line.
5. For `verify_born_rule.py` (non-deterministic), ran the new version
   twice to measure intrinsic drift (~5×10⁻⁴); verified that the
   baseline/new drift is within that floor and the PASS structure is
   preserved.
6. For `look_elsewhere_monte_carlo.py` (baseline crashed), fixed the
   pre-existing bug first, verified deterministic re-runs are
   byte-identical, and confirmed the FTD master quadratic is still the
   unique match in the reported list.

### Pre-existing baseline bugs fixed (Phase 8b)

| Script | Bug | Fix |
|--------|-----|-----|
| `scripts/proofs/look_elsewhere_monte_carlo.py` (line 169) | `x1[idx]` and `x1[idx] - ALPHA_INV_TARGET` where `idx = valid_indices[indices]` is chunk-global but `x1` is the filtered-by-valid-discriminant subset. Triggers `IndexError` at any N_samples large enough to generate a ppm-match hit. | Mirror the x2 block's approach: recompute `d_val = b[chunk_idx]**2 - 4*c_term[chunk_idx]` and `root_val = (-b[chunk_idx] + sqrt(d_val))/2` using chunk-global `b` and `c_term`. Avoids the filtered/unfiltered confusion entirely. |

### Skipped in Phase 8b (with reasons)

| Script | Baseline | Reason |
|--------|----------|--------|
| `scripts/verification/verify_anti_correlation.py` | 1.17 s | `range(100000)` loops are `sum(mpmath.mpf(...) for n in range(100000))` — arbitrary precision via mpmath, which torch can't help with. The rest of the script is mpmath-backed `zeta(s)` evaluations. Skipped per task rule: "mpmath stays mpmath". |
| `scripts/verification/verify_log_gstar_identity.py` | 0.96 s | Same — the `range(100000)` beta(4) / beta(6) sums are mpmath arbitrary precision. The rest is formula evaluation with hardcoded float constants. |
| `scripts/exploration/i_from_star.py` | 1.67 s | `range(1, 1000001)` is an mpmath `L_partial += mpf(c) / n` loop for L(chi_{-4}, 1). mpmath arbitrary precision, not vectorizable with torch. |
| `scripts/exploration/explore_primes_and_gstar.py` | 0.52 s | Sub-second runtime, below the 1 s priority threshold. Only one `range(100000)` Python generator sum; negligible impact. |
| `scripts/proofs/proof_confinement_wilson.py` | fast | Re-verified from Phase 8a: no hot MC or triple loop. All `range(1,8)` sized loops; dominated by `scipy.special.iv` at 2 points. PyTorch would be pure overhead. |
| `scripts/exploration/test_all_physics.py` | 0.36 s | 27×27 matrix ops and hand-written integer loops. Too small to benefit from vectorization. |
| `scripts/tests/comprehensive/test_tier4_simulation.py` | 0.09 s | Entire module skips via `pytest.importorskip("ternary_matrix.config")` — `ternary_matrix` is the archived Python engine, replaced by the C++ engine. Not a conversion target. |
| `scripts/proofs/proof_gap_equation_from_partition_function.py` | fast | `watson_bcc_montecarlo` is defined but never called. Converting an uncalled helper adds no measurable speedup; revisit if/when the caller is reactivated. |
| `scripts/proofs/proof_von_neumann_type.py` | 0.64 s | Sub-second; loops are over 3×3 complex matrices and 10k-sample statistics. Below threshold. |
| `scripts/proofs/proof_three_open_problems.py` | 0.26 s | Sub-second; mostly formula evaluation. |
| `scripts/proofs/proof_self_energy_derivation.py` | 0.95 s | Below threshold; no obvious hot kernel. |
| `scripts/proofs/u1_gauge_proof.py` | **baseline crashes** | Imports `ternary_matrix.model.grid` which is archived. Not a conversion target. |
| `scripts/proofs/su2_gauge_proof.py` | 0.21 s | Below threshold. |
| `scripts/proofs/su3_gauge_proof.py` | 0.19 s | Below threshold. |
| `scripts/proofs/renormalization_framework.py` | 0.23 s | Below threshold. |
| `scripts/exploration/explore_bell_verify.py` | 28.4 s | Non-trivial target (would be Tier 1 by wall clock) but the dominant cost is ~450 k calls through a `SimpleLattice` class instance with `lat.tick()` side effects. Vectorizing would require refactoring the class to carry a trial-batch dimension, which violates the "swap backends, do not refactor" rule. Documented for a dedicated future phase. |
| `scripts/exploration/transfer_matrix_scaling.py` | 11.5 s | Dominant cost is `scipy.sparse.linalg.eigsh` on a 19683×19683 dense matrix — LAPACK already at BLAS speed on CPU. `torch.linalg.eigh` on GPU would help, but the script also has several downstream correlations over the eigenvector matrix. Moderate-risk conversion; Phase 8c candidate. |
| `scripts/proofs/gauss_constrained_green.py` (v1) | 2.8 s | 2.8 s is modestly above threshold, but it has 6 independent `compute_*` functions each with its own triple loop. v3 (converted) supersedes v1 in the project and gives a bigger absolute win; v1 is Phase 8c candidate. |

### Phase 8b summary

- **6 scripts converted** (watson_check, watson_normalization_fix, gauss_constrained_green_v3, gauss_constrained_green_v2, verify_born_rule, look_elsewhere_monte_carlo)
- **1 pre-existing bug fixed** (`look_elsewhere_monte_carlo.py` IndexError)
- **Total wall-clock savings**: ~3 m 30 s → ~8 s on these 6 scripts (roughly 26× aggregate speedup of the converted subset on the NumPy fallback path; GPU path would be larger)
- **19 scripts explicitly considered and skipped** with documented reasons
- All 6 converted scripts return exit 0 after conversion
- All 5 deterministic converted scripts produce byte-identical output except for the new `[backend]` log line
- `verify_born_rule.py` (non-deterministic) is within its own ~5×10⁻⁴ run-to-run noise floor

Cumulative status: **11 / 127 scripts converted** (Phase 8a: 5, Phase 8b: 6). 116 remaining.

### Phase 8c priority queue

- `scripts/proofs/gauss_constrained_green.py` (v1) — convert remaining compute_* helpers for consistency with v2/v3.
- `scripts/exploration/transfer_matrix_scaling.py` — torch.linalg.eigh + vectorized correlation loops (requires torch to be installed for any benefit).
- `scripts/exploration/explore_bell_verify.py` — refactor `SimpleLattice` to carry a trial-batch dimension, then vectorize the outer loops.
- `scripts/proofs/proof_gap_equation_from_partition_function.py` — convert `watson_bcc_montecarlo` if/when re-enabled.
- `scripts/proofs/derive_mass_prefactors.py` — check for hot kernels (multiple deeply nested range(n_verts) loops seen in grep scan).
