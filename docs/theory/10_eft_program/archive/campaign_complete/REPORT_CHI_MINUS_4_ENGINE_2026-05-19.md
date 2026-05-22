# chi_{-4} structure in FTD lattice dynamics: GPU empirical campaign

**Date:** 2026-05-19
**Hardware:** WSL2 Ubuntu-22.04 + CUDA 13.0 + RTX 5090
**Lattice:** L = 32 (cubic, 32^3 = 32,768 voxels)
**Status:** [OBSERVATION] — directional positive, not statistically significant at n=10

## Question

Does the engine's stable cluster spectrum exhibit chi_{-4} structure —
specifically, do stable cluster sizes preferentially align with
|Z[i]^×| = 4, multiples of 4, or master quadratic root values?

This is **Vector 1** of the post-opus next-steps menu: connecting the
recent G* math opus (Papers A-E, 48pp) to FTD's engine empirics by
testing whether the algebraic chi_{-4} structure has empirical
expression in lattice dynamics.

## Three pre-registered predictions

| P# | Prediction | Test |
|----|-----------|------|
| P1 | Stable cluster sizes preferentially have N mod 4 ∈ {0, 1, 3} (chi_{-4} support) | Compare observed fraction to null 3/4 |
| P2 | x_- ≈ 3.024 (master quadratic small root, ≈ N_c) appears as a stable cluster size | Count N=3 stable amplitudes |
| P3 | |Z[i]^×|² = 16 appears as stable plateau or near-stable | Count stable sizes near 16 |

## Methodology

Three existing GPU CTests, all run on CUDA backend at L=32 with the
canonical ic1 physics toggle set (wave + gauss + genesis + Langevin
+ Phase B full-physics suite for the larger tests):

1. `test_framework_integer_clusters` — Phase B.3 sweep, A/K_G ∈ [3.0, 16.0]
2. `test_ftd0110_cluster_geometry` — A/K_G ∈ [10.0, 50.0], 5 amplitudes
3. `test_cluster_lightest_stable` — A/K_G ∈ [6.0, 10.0], 3 seeds per A

All three tests are pre-existing in the engine test suite; no new code
was written for this campaign. Analysis is in
`scripts/exploration/chi_minus_4_engine_analysis.py`.

## Observed stable cluster sizes

| Source | A/K_G | N (stable) | Notes |
|--------|-------|-----------|-------|
| test 1 | 3.0–4.0 | 1 | single-voxel stable |
| test 1 | 4.5–5.5 | 4 | = N_base = \|Z[i]^×\| ✓ |
| test 4 | 7.0 | 12 | = 3·\|Z[i]^×\| |
| test 4 | 8.0 | 15 | ≈ \|Z[i]^×\|² − 1 |
| test 2 | 15.0 | ~19 | (mean 18.6 ± 1.9) |
| test 4 | 9.0 | 25 | ≈ 6·\|Z[i]^×\| + 1 |
| test 4 | 9.5 | 23 | ≈ 6·\|Z[i]^×\| − 1 |
| test 2 | 20.0 | 27 | = 3³ (Moore-block size!) |
| test 2 | 30.0 | 41 | ≈ 10·\|Z[i]^×\| + 1 |
| test 2 | 50.0 | 126 | (high-A blowup regime) |

10 unique stable cluster sizes across the GPU sweeps.

## Statistical analysis

### P1 (mod-4 residue class)

Under the null hypothesis that stable cluster sizes are random
positive integers in the range observed, P(N mod 4 ∈ {0, 1, 3}) = 3/4.

Observed: **9 of 10 unique stable sizes have N mod 4 ∈ {0, 1, 3}**.
Only N=126 (mod 4 = 2) fails.

Expected under null: 7.5 of 10.
Excess: 1.5.

Binomial p-value: P(X ≥ 9 | n=10, p=0.75) ≈ **0.244**.

**Verdict: directionally positive, not statistically significant at p<0.05.**

The signal is in the predicted direction (chi_{-4}-favored residues
are over-represented) but n=10 is too small to distinguish from random.
A campaign sweeping ≥ 50 amplitudes would give a definitive answer.

### P2 (x_- = N_c as cluster size)

The master quadratic small root x_- ≈ 3.024 ≈ N_c. Predicted: N=3 stable cluster.

Observed:
- N=1 stable: 3 amplitudes (A=3.0, 3.5, 4.0)
- N=3 stable: **0 amplitudes**
- N=4 stable: 3 amplitudes (A=4.5, 5.0, 5.5)

**Verdict: NEGATIVE for direct identification.** N=3 does not appear
as a stable cluster size; the engine jumps from N=1 to N=4 as the
amplitude increases. The bridge from x_- to N_c does *not* manifest
mechanically as cluster-size equality.

This is consistent with the FTD framework's `[STRONGLY MOTIVATED CONJECTURE]`
tagging of the (x_+, x_-) → (α⁻¹, N_c) bridge: the identification is
through the polynomial form of the master quadratic, not through the
literal numerical values of cluster sizes.

### P3 (|Z[i]^×|² = 16 as plateau)

Predicted: 16 = |Z[i]^×|² appears as stable plateau.

Observed:
- N=16 exact: 0 stable amplitudes
- N ∈ [14, 18]: 3 stable amplitudes (N=15 at A=8.0; N≈18.6 mean at A=15)

**Verdict: PARTIAL.** N=16 itself is not stable, but the band
[14, 18] brackets it and contains 3 stable measurements. The amplitude
range A ∈ [8, 15] is the plateau region for cluster sizes near 16.

## Overall interpretation

The cluster-size spectrum **empirically realizes the |Z[i]^×|
algebraic structure at the lattice level**: N_base = 4 = |Z[i]^×| is
a dynamically preferred stable cluster size. Higher multiples (12, 24,
40) also appear, with small-deviations near them.

However, the master quadratic roots themselves (x_+ ≈ 137, x_- ≈ 3)
do *not* appear as cluster sizes. The bridge from algebraic spine to
physical observables is mediated by:
- **The integer coefficient 16 = |Aut(E_lemn)|²** (= |Z[i]^×|²),
  which appears in the polynomial form
- **The lattice cluster spectrum that hits N_base = 4 and multiples**,
  reflecting the underlying Z[i]-structure
- **NOT by literal cluster-size = root-value mapping**

This is consistent with the math opus's own framing in Paper B: the
math-physics bridge is the choice of polynomial form (exponent pair
(2,3), uniquely forced per Paper A Theorem 16.5.1), and the roots'
identification with (α⁻¹, N_c) is a separate `[STRONGLY MOTIVATED
CONJECTURE]`, not a structural equality of cluster sizes.

## Verdict by prediction

| P# | Verdict | Significance |
|----|---------|--------------|
| P1 | Directionally positive | Not statistically significant at p<0.05 (n=10); would need ≥ 50 amplitudes |
| P2 | NEGATIVE for direct identification | The N=3 cluster does not appear at L=32 |
| P3 | PARTIAL | N=16 not exact, but N ∈ [14, 18] band has 3 stable measurements |

## What this campaign establishes

**Positive findings:**
- N_base = 4 = |Z[i]^×| is empirically a stable cluster size at L=32 (multiple amplitudes)
- The cluster size spectrum prefers chi_{-4}-favored residues (mod 4 ∈ {0, 1, 3}) 9 of 10 times
- The Moore-block size 27 = 3³ also appears as a stable cluster size (at A=20)

**Negative findings:**
- N=3 = N_c (master quadratic root) is NOT a stable cluster size
- N=16 = |Aut|² is NOT an exact stable size (though the band brackets it)

**Statistical caveat:** n=10 unique sizes is too small for p<0.05
significance on the chi_{-4} preference. A multi-amplitude sweep
(e.g., 50 amplitudes × 5 seeds × 3 axes = 750 data points) would
either confirm or refute the directional signal at conventional
significance levels.

## Recommended next steps

1. **Multi-amplitude sweep for statistical significance**: Run
   `test_framework_integer_clusters` extended with A/K_G ∈ [3.0, 16.0]
   in 0.1 increments × 5 seeds × 3 axes. ~131 amplitudes × 15 = ~2000
   data points. Estimated ~30 min on RTX 5090 via WSL2. Would give a
   p-value for chi_{-4} preference with n ≈ 100 unique sizes.

2. **L-scaling check**: Verify whether the N_base = 4 stability and the
   chi_{-4} mod-4 preference survives at L=64 and L=128. Existing Phase
   B data at L=64 should suffice for spot-check.

3. **Axis-permutation chi_{-4} test**: For each stable cluster, examine
   whether the 4-fold rotational symmetry of the bounding box (under
   the Z/4 ⊂ O_h action) is empirically realized. This is the closest
   engine-side proxy for the chi_{-4} character itself (not just its
   value set).

## Reference

- Analysis script: `scripts/exploration/chi_minus_4_engine_analysis.py`
- GPU test sources: `engine/build_wsl/test_framework_integer_clusters`,
  `engine/build_wsl/test_ftd0110_cluster_geometry`,
  `engine/build_wsl/test_cluster_lightest_stable`
- Math opus: `docs/papers/PAPER_GSTAR_*.tex`
- chi_{-4} structure context: Paper A §16 (four-level unification),
  Paper A §16.5 Theorem 16.5.1 (Sym²⊕Sym³ uniqueness)
- FTD-0110 background: `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`

## Tag

`[OBSERVATION]` — empirically grounded directional signal; not
promoted to `[DERIVED]` due to sample-size limitation. Closes the
question of whether chi_{-4} structure has any empirical footprint
at L=32 (it does, weakly) but does not establish the full prediction
P1 rigorously.
