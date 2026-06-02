# EXPLR — Ternary-Matrix BCC-Snap: Honest Negative Result

**Document type:** Exploratory test result (closed-negative)
**Status:** `[CLOSED NEGATIVE]` — under the pre-registered construction, iterates of `𝒟_T = A + B·Θ` do **not** snap to BCC primitive directions; the proposed mechanism does not survive the test as stated.
**Created:** 2026-05-23
**Pre-registration:** [`PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md`](PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md)
**Git tag (hash-lock):** `preregister-ternary-matrix-bcc-snap-v1` (commit `b077f39`, runner SHA256 `77c2fc6965b83d9392fbd0c8fa30fb39210cf1663346e49aa2e2ec42ef256c1d`)
**Runner:** `scripts/exploration/explore_ternary_matrix_iteration.py`
**Results:** `scripts/exploration/results/ternary_matrix_iteration_2026-05-23.{csv,md}`
**Provenance:** 2026-05-23 user-presented synthesis (steps 1–4: Borwein/Guillera → ternary matrix → BCC-snap → QM-collapse identification). This document tests **step 3** (BCC-snap); steps 1, 2 are scaffolding, step 4 is out of scope per `PREREG §4.4`.

---

## 0 · One-paragraph result

The pre-registered prediction (P1: at least one of 8 natural (A, B) pairs yields a dominant eigenvector at distance `< 10⁻⁶` from a BCC primitive direction `(±1, ±1, ±1)/√3` uniformly over 5 seeds) **fired Outcome D** per the manifest's outcome→tag map: 6 of 8 (A, B) pairs converge to non-BCC directions, and the remaining 2 pairs (the antisymmetric B-patterns under `A2 = diag(G*, ϖ, π)`) fail to converge within 500 steps because their dominant eigenvalues form a complex-conjugate pair. `[CLOSED NEGATIVE]` on the construction-as-stated. No FTD tag is promoted or demoted by this result; the LEDGER status of `x₊ = 1/α` (`[STRONGLY MOTIVATED CONJECTURE]`, FTD-0013) is unchanged.

---

## 1 · What was tested

The 2026-05-23 synthesis proposed promoting Guillera's scalar `𝒟 = a + b·ϑ_x` to a matrix operator `𝒟_T = A(x) + B·Θ` on a ternary carrier, and claimed that iterates "snap" onto BCC lattice nodes. The pre-registration pinned the construction at its smallest viable form (3×3 over ℝ³, `mpmath` 50-digit precision, normalized power iteration as the iteration rule):

- **Θ** = `diag(1, 2, 3)` (the discrete Euler operator; ℝ³ projection of Guillera's `ϑ_x = x·d/dx`). `[DEFINITION]`
- **A**: two candidates, `A1 = diag(G*, G*², G*³)` and `A2 = diag(G*, ϖ, π)`. `[DEFINITION]`
- **B**: four natural BCC-sign-pattern candidates `B1..B4`, plus a `B=0` control and 10 randomized-B controls. `[DEFINITION]`
- **Iteration:** `v_{k+1} = (A + B·Θ)·v_k / ||(A + B·Θ)·v_k||`, terminating at `||v_{k+1} − v_k|| < 10⁻¹²` or 500 steps. `[DEFINITION]`

This iteration converges (generically) to the dominant eigenvector of `A + B·Θ` in projective space, modulo antipodal identification. The BCC-snap claim therefore reduces to: *does the dominant eigenvector of one of the eight natural (A, B) pairs lie on a BCC primitive direction?*

---

## 2 · What the run produced

The full per-row data are in [`ternary_matrix_iteration_2026-05-23.csv`](../../../scripts/exploration/results/ternary_matrix_iteration_2026-05-23.csv) (150 rows). The per-(A, B) summary:

| A | B | seeds converged | mean `d_BCC` | mean `d_axis` | snaps to BCC (`d_BCC < 10⁻⁶`) |
|---|---|---|---|---|---|
| A1 (G* powers) | B1 (pos sym) | 5/5 | 7.16×10⁻¹ | 2.24×10⁻¹ | 0/5 |
| A1 | B2 (asym BCC) | 5/5 | 7.20×10⁻¹ | 2.19×10⁻¹ | 0/5 |
| A1 | B3 (cyclic antisym) | 5/5 | 7.28×10⁻¹ | 2.17×10⁻¹ | 0/5 |
| A1 | B4 (Toeplitz sign) | 5/5 | 7.17×10⁻¹ | 2.23×10⁻¹ | 0/5 |
| A2 (G*, ϖ, π) | B1 | 5/5 | 1.42×10⁻¹ | 7.93×10⁻¹ | 0/5 |
| A2 | B2 | 5/5 | 5.64×10⁻¹ | 4.41×10⁻¹ | 0/5 |
| A2 | B3 | **0/5** | — | — | 0/5 |
| A2 | B4 | **0/5** | — | — | 0/5 |

Controls:

- `B=0`: 10/10 converge to a pure axis direction (`±e_3` for A1; `±e_3` for A2). `d_axis ≲ 10⁻¹³`, `d_BCC ≈ 0.92`. The eigenvector is the basis vector aligned with the largest-modulus diagonal entry of `A`. `[NUMERICAL FACT]`
- Random-B (100 runs across 10 random sign patterns × 5 seeds × 2 A): 0 of 100 satisfies `d_BCC < 10⁻⁶`. The BCC-snap is not a generic feature of off-diagonal coupling on this carrier. `[NUMERICAL FACT]`

The pre-registered outcome → tag map (`PREREG §4.3`) fires **Outcome D** because at least one natural (A, B) fails to converge (A2 × B3, A2 × B4). Read more carefully:

- **6 of 8** primary pairs converge but **not to BCC**. The dominant eigenvector lies strongly biased toward the largest-diagonal-entry axis of A: for A1, near `(0.14, 0.17, 0.97)` — close to `e_3`; for A2 + B1, near `(0.69, 0.53, 0.49)`. Off-diagonal coupling perturbs the eigenvector slightly off-axis but does **not** rotate it onto `(1,1,1)/√3`. This corresponds substantively to **Outcome B** of the manifest (convergence to a non-BCC attractor).
- **2 of 8** primary pairs (the antisymmetric B-patterns under A2) fail to converge: their `A + B·Θ` has a complex-conjugate dominant eigenvalue pair, so power iteration rotates in a 2D invariant subspace without contracting to a single direction. This is **Outcome D**.

The combined honest reading: the proposal as constructed yields **Outcome B-with-D-subset**, both falsifying the pre-registered prediction P1.

---

## 3 · Why the construction does not produce a BCC snap

`[STRUCTURAL OBSERVATION].` The diagonal A in both candidate forms is strictly diagonal-dominant: `A1` has entries `(G*, G*², G*³) ≈ (2.96, 8.75, 25.91)`, and `A2` has entries `(G*, ϖ, π) ≈ (2.96, 2.62, 3.14)`. The off-diagonal coupling `B·Θ` has entries of order `O(3)` (because `Θ = diag(1,2,3)` and B has entries in `{−1, 0, +1}`). For A1 this is roughly 1:10 sub-dominant; the dominant eigenvector is therefore close to `e_3`, the basis vector of the largest A-entry. Power iteration finds this dominant eigenvector and the BCC-symmetric direction `(1,1,1)/√3` is not it.

`[STRUCTURAL OBSERVATION].` The cyclic-antisymmetric `B3 = [[0,1,−1],[−1,0,1],[1,−1,0]]` is the matrix of the cross-product `v ↦ (1,1,1)/√3 × v`; its eigenvalues are `0, ±i√3` with kernel along `(1,1,1)/√3`. In the small-A limit (A → 0), `B3·Θ` does have a real eigenvalue 0 with eigenvector close to `(1,1,1)/√3`, and BCC-snap would obtain in that limit. But the test holds A at its FTD-canonical magnitude, where the diagonal-dominance breaks the BCC kernel: B3·Θ's complex-conjugate ±i√3·{2,3,1} eigenvalues plus A's diagonal entries combine to give eigenvalues of `A + B3·Θ` that are also a complex-conjugate pair near the diagonal of A. Power iteration then rotates in a 2D subspace and never converges. Hence Outcome D under (A2, B3) and (A2, B4).

`[CONJECTURE].` A construction where A and B are commensurate in magnitude — for example, taking A scaled by a small dimensionless coupling, or replacing the additive `A + B·Θ` with a multiplicative `A^{1/2}·(I + B·Θ)·A^{1/2}` form — might recover BCC alignment. This is not pre-registered, and any positive result in that family would require its own pre-registration and re-run. It is recorded here only so the closed-negative scope is precise: *under the additive form with diagonal-dominant A as committed in §2.2 of the manifest*, BCC-snap does not occur.

---

## 4 · What is **not** falsified by this run

- **The master quadratic and its algebraic-spine theorems** (`SPEC_ALGEBRAIC_SPINE.md` §§1–9). `[THEOREM]` status unchanged.
- **G* = Γ(1/4)/Γ(3/4)** as an algebraic identity (FTD-0001). `[THEOREM]` unchanged.
- **`x₊ = 1/α`** (FTD-0013). `[STRONGLY MOTIVATED CONJECTURE]` unchanged. This test bears on a proposed *mechanism* (matrix-iteration → BCC), not on the central conjecture.
- **The BCC multiplicative structure** (`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`). `[THEOREM]` unchanged. The eigenvalue `σ_BCC(k) = 1 − cos k₁ cos k₂ cos k₃` is a momentum-space scalar; this test was about *position-space matrix iteration*, a different object.
- **The Z[i]-module structure on Z[BCC] ⊗ ℚ** (FTD-0122, Paper B). `[DERIVED for Roles 1+3] / [NO-GO for Roles 2+4]` unchanged.
- **The two Guillera-related G*-computation routes** (`proof_quartic_quarter_constants.py`, `proof_landen_gstar_compression.py`). `[FACT / verified at >130 digits]` unchanged.
- **The Guillera fence** (`REF_GUILLERA_CORPUS_MAP.md` §0). Unchanged. This test claimed no Guillera-validates-FTD bridge, and the negative result reinforces — not undermines — that fence.

---

## 5 · What this rules out

`[CLOSED NEGATIVE].` The specific construction `𝒟_T = A + B·Θ` on `ℝ³` with `A ∈ {diag(G*, G*², G*³), diag(G*, ϖ, π)}`, `B` one of four natural BCC-sign-pattern 3×3 matrices, `Θ = diag(1, 2, 3)`, under normalized power iteration, does **not** produce iterates that snap to BCC primitive directions. Both halves of the prediction fail:

- The off-diagonal `B·Θ` coupling does not rotate the dominant eigenvector onto a BCC direction (Outcome B for 6/8 pairs).
- The antisymmetric B patterns introduce complex-conjugate dominant eigenvalues, blocking convergence entirely (Outcome D for 2/8 pairs).

`[METHODOLOGICAL].` Future variants of this proposal — multiplicative forms, A scaled by small couplings, function-space carriers, nonlinear A(v_k), or non-power-iteration dynamics — need a fresh pre-registration (`v2`) and a fresh hash-lock before any of their results can count.

---

## 6 · Items still out of scope (per PREREG §4.4)

The pre-registration explicitly fenced these off this run:

- The identification `x₊ = 1/α` (FTD-0013) — unchanged.
- The collapse-mechanism question — canonical proposal remains `DERIV_COLLAPSE_MECHANISM.md` (Softplus/Lindblad/Type III₁→I); the user-presented "BCC-fractal-replaces-collapse" identification was never tested here and cannot be retro-credited to this run.
- "Gravity is the quartic folding of mathematical tension" — overclaim per LEDGER (gravity is `[PARTIAL]` per FTD-0131); not in scope.
- The Weierstrass-class fractal limit — adjacent and testable in a separate experiment (basin-of-attraction analysis, not iterate-convergence analysis); not in scope.

---

## 7 · Cross-references

- [`PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md`](PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md) — the pre-registration manifest (locked construction, prediction, outcome map).
- [`REF_GUILLERA_CORPUS_MAP.md`](../general_math/REF_GUILLERA_CORPUS_MAP.md) — Guillera fence (§0); explanation of why this test does not constitute a Guillera↔physics bridge regardless of outcome.
- [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md) — canonical BCC formalism; the BCC eigenvalue `σ_BCC(k)` is a *momentum-space scalar*, not a position-space matrix iteration. This run did not test that result; it tested a different object the user proposed.
- [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md) — Theorems 1–9 (unaffected).
- [`DERIV_COLLAPSE_MECHANISM.md`](../06_reference_frames_and_measurement/DERIV_COLLAPSE_MECHANISM.md) — canonical collapse proposal; the present run does not engage it.
- [`LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) — see new row FTD-0197.
- [`EXPLR_3X3_MIXING_NEGATIVE.md`](../general_math/EXPLR_3X3_MIXING_NEGATIVE.md) — sibling 3×3 closed-negative document; same epistemic pattern (a 3×3 promotion that does not extend a 2×2 structure cleanly).

---

## 8 · Bookkeeping

- **Pre-registration discipline:** fully respected. Construction, prediction, outcome map, and seeds were frozen before the runner executed.
- **No tag promoted:** no FTD theorem, derivation, or conjecture is upgraded by this result.
- **No claim demoted:** the proposed BCC-snap was never tagged above `[PROPOSAL]`; it now closes negative under the pre-registered construction.
- **Engine touched:** none.
- **Manuscript touched:** none.
- **Paper touched:** none.
- **Result lives in:** this file + LEDGER FTD-0197 + the CSV/MD result artifacts. Nowhere else.
