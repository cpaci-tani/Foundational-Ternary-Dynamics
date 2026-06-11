# EXPLR — Generation Graph Γ_F(d) and CKM-Shape Overlap (Candidate Reconstruction)

**Tag:** `[CANDIDATE RECONSTRUCTION]` — explicitly **NOT** a theorem. Equating the overlap magnitudes with the experimental CKM matrix is `[CONJECTURE]` at best, and is not asserted by this module or its test.
**Date:** 2026-05-22
**LEDGER:** FTD-0196
**Engine primitive:** [`../../../engine/include/ftd/generation_graph.h`](../../../engine/include/ftd/generation_graph.h)
**Constructive instrument:** [`../../../engine/tests/test_generation_graph.cpp`](../../../engine/tests/test_generation_graph.cpp) — `ctest -R "^generation_graph$"` passes 1/1; **only structural sanity asserted**, candidate overlap magnitudes printed informationally.
**Status:** Module is a diagnostic instrument for a candidate structural reading. GTCA F1/F10 applies — a structural resemblance is not a derivation.

---

## §1 — What is claimed (and what is NOT)

`[CANDIDATE RECONSTRUCTION].` The Γ_F(d) construction (§2) is **one** candidate reading of the three-generation flavour mixing pattern using only ontic-chain inputs (G\* and the arithmetic root `q*`). Its eigenvectors' overlap matrix has the **shape** of the CKM matrix (diagonally dominant 3×3 unitary). Whether that shape genuinely matches the empirical CKM matrix is a separate, downstream `[CONJECTURE]` and would require:

- a derivation of the K_3 edge-weight + Wilson-loop-phase distribution from substrate dynamics (this module uses the canonical PLAN_03 graph-Laplacian form, with the Wilson-loop phase on the (0,2) edge only — a `[SELECTION]` of one specific construction, not a derivation);
- a derivation that `d_U = 3` and `d_D = 2` are physically forced (currently `[SELECTION]`);
- physical interpretation as the experimental CKM matrix — `[CONJECTURE]`. The C++ test now **reproduces the owner's Python-prototype overlap to machine precision** (max `|Δ| ≈ 4×10⁻⁷`, Frobenius `≈ 8×10⁻⁷`, see §4), but agreement between the *prototype* and the empirical CKM matrix is still on the conjecture side of the line.

None of these is delivered here. This is `[CANDIDATE RECONSTRUCTION]`, not `[THEOREM]`, not `[DERIVED]`, not `[STRONGLY MOTIVATED CONJECTURE]`.

`[CONFIRMED].` What the engine test does verify (and what this doc asserts):

- the construction is internally consistent (Hermitian Γ_F(d), real spectrum from the cubic formula, orthonormal eigenvectors, eigenvalue equation `H·v = λ·v`);
- the overlap-as-mixing-matrix is unitary (`V·V† = I` to `10^{−10}`).

These are bookkeeping facts about a small Hermitian system; they license the construction as a diagnostic instrument, nothing more.

---

## §2 — The Γ_F(d) construction

Let `G* = ftd::ontic::G_STAR ≈ 2.958675119188639` (canonical engine-side, [`../../../engine/include/ftd/ontic/lemniscate.h`](../../../engine/include/ftd/ontic/lemniscate.h)). Define

    q*  =  ( G* − √(G*² − 4) ) / 2                  (eq. 1)

= the smaller root of `x² − G* · x + 1 = 0`. Numerically `q* ≈ 0.389181783924403`; the other root `1/q* = G* − q* ≈ 2.5695` (their product is 1). `q*² − G*·q* + 1` evaluates to `2.22 × 10^{−16}` in the test — machine zero.

Define `Γ_F(d)` as the 3×3 Hermitian **weighted graph Laplacian** of the K_3 triangle on three vertices `(0, 1, 2)`, with edge weights and a single Wilson-loop phase on the closing edge `(0, 2)`:

    edge (0,1) weight   w_12  =  q*^{d+1}
    edge (1,2) weight   w_23  =  1
    edge (0,2) weight   w_13  =  q*^d
    Wilson-loop phase   φ(d)  =  π + π/d         (carried by (0,2) only)

    L_00 = w_12 + w_13       L_01 = −w_12              L_02 = −w_13 · e^(+iφ)
    L_10 = −w_12             L_11 = w_12 + w_23        L_12 = −w_23                 (eq. 2)
    L_20 = −w_13 · e^(−iφ)   L_21 = −w_23              L_22 = w_13 + w_23

Diagonals are vertex degrees (sum of incident edge weights); off-diagonals are minus the edge weights; the Wilson-loop holonomy `e^{i 3φ}` distinguishes `Γ_F(d)` for different `d`. This is the canonical PLAN_03 form — a standard weighted graph Laplacian, **not** a uniform-phase adjacency matrix.

Identify `Γ_U := Γ_F(3)` ("up-type") and `Γ_D := Γ_F(2)` ("down-type"). Their eigenvectors `|U_i⟩`, `|D_j⟩` give the CKM-shape overlap

    V_{ij}  =  ⟨U_i | D_j⟩                            (eq. 3)

with magnitudes `|V_{ij}|` reported by the test.

---

## §3 — What is verified (structural sanity)

The test [`engine/tests/test_generation_graph.cpp`](../../../engine/tests/test_generation_graph.cpp) asserts only structural facts:

| Sub-test | What is asserted |
|---|---|
| T1 | `q*` is real in `(0, 1)`; satisfies `q*² − G*·q* + 1 = 0` to `< 10^{−14}`. |
| T2 | `Γ_F(d)` is Hermitian for `d ∈ {2, 3}` to `< 10^{−14}`. |
| T3 | Cubic-formula eigenvalues are real; `tr(H) = Σλ_i` and `det(H) = Π λ_i` to `< 10^{−12}` (consistency). |
| T4 | Eigenvectors are orthonormal: `⟨v_i | v_j⟩ = δ_{ij}` to `< 10^{−10}`. |
| T5 | Eigenvalue equation: `\|H v_k − λ_k v_k\| < 10^{−10}`. |
| T6 | Overlap unitarity: `V · V† = I` to `< 10^{−10}`. |
| T7 | **INFORMATIONAL** — prints `|V_{ij}|`, the owner-supplied candidate target, and the absolute deviation. No assertion in T7 itself. |
| T8 | PLAN_03 rule check: `generation_weights(d)` returns `(w_12, w_23, w_13, φ) = (q*^{d+1}, 1, q*^d, π+π/d)` to `< 10^{−14}`. |
| T9 | PLAN_03 target-tolerance: `\|V_{ij} − target_{ij}\| ≤ 5 × 10⁻⁴` for all `(i, j)` (canonical PLAN_03 tolerance for first implementation; **actual: max `\|Δ\| ≈ 4 × 10⁻⁷`, Frobenius `≈ 8 × 10⁻⁷`** — three orders of magnitude tighter). |

All eight asserted sub-tests pass at machine precision.

---

## §4 — Computed values (informational)

Numerical output from the test:

```
Γ_F(3) spectrum:  0.0195   0.1018   2.0425   (sorted ascending)
Γ_F(2) spectrum:  0.0305   0.2768   2.1135   (sorted ascending)

|V_{ij}| (computed by the C++ test):
    0.973536   0.228440   0.006537
    0.228336   0.972678   0.041952
    0.009485   0.041385   0.999098

owner-supplied candidate target (Python prototype):
    0.973536   0.228440   0.006537
    0.228336   0.972678   0.041952
    0.009485   0.041385   0.999098

abs deviation:  < 10⁻⁶ at every entry
Frobenius deviation = 7.83 × 10⁻⁷
max |Δ|             = 4.09 × 10⁻⁷
```

The **canonical PLAN_03 K_3 graph-Laplacian form reproduces the owner's Python-prototype overlap exactly** — to round-off precision, three orders of magnitude tighter than PLAN_03's own first-implementation tolerance of `5 × 10⁻⁴`. The CKM-shape diagonal dominance is genuine (V_{ii} ≈ 0.97–0.999) and the small mixing pattern is recovered (V_{ub}, V_{td} both < 10⁻². The C++ implementation is a faithful port of the Python prototype.

This says nothing new about whether the construction matches the **experimental** CKM matrix — that comparison is the separate `[CONJECTURE]` from §1. What is now confirmed: the C++ and Python implementations of the same candidate agree to machine precision.

---

## §5 — Honest tagging of the components

| Component | Tag | Why |
|---|---|---|
| `Γ_F(d)` matrix construction (eq. 2) | `[SELECTION]` of one specific construction | The canonical PLAN_03 graph-Laplacian (phase only on the (0,2) Wilson-loop closure edge) is one of several admissible K_3 forms; selecting it is convention, not derivation. |
| `q* = (G* − √(G*²−4))/2` (eq. 1) | `[DERIVED]` from G\* via a quadratic root | Standard arithmetic; G\* is the substrate input (`ftd::ontic::G_STAR`). |
| Real, sorted eigenvalues from cubic | `[THEOREM]` (Smith-1961 stable cubic for Hermitian 3×3) | Standard linear algebra. |
| Orthonormal eigenvectors / unitarity of `V` | `[THEOREM]` (Hermitian linear algebra) | Standard. |
| `Γ_F(d)` → flavour generations | `[SELECTION]` (the labelling `Γ_U` = Γ_F(3), `Γ_D` = Γ_F(2)) | Interpretation, not derivation. |
| `\|V_{ij}\|`  experimental CKM | `[CANDIDATE RECONSTRUCTION]` overall; the equality is `[CONJECTURE]` at best | A structural resemblance is not a derivation (GTCA F1/F10). |

The LEDGER row (FTD-0196) records: the `Γ_F(d)` flavour graph as `[CANDIDATE RECONSTRUCTION]`, and the CKM-like overlap as `[RECONSTRUCTION / diagnostic]`.

---

## §6 — Scope

This module is a **pure header-only overlay** on the existing engine — no `RenderBridge` touch, no physics-toggle wiring, no engine state modified. The golden-tick hash is preserved. The test is `unit`/`native`-labelled and runs in ~10 ms with no GPU dependency.

The eigensolver (Smith-1961 stable cubic + cross-product null vectors) is a general-purpose 3×3 Hermitian utility usable by any downstream FTD-native theory module that needs small Hermitian spectra without an external LAPACK dependency.

---

## §7 — Cross-references

- **Engine primitive:** [`../../../engine/include/ftd/generation_graph.h`](../../../engine/include/ftd/generation_graph.h) — `q_star()`, `GenerationWeights` struct, `generation_weights(d)`, `gamma_F_from_weights(w)`, `gamma_F(d)` (canonical PLAN_03 graph-Laplacian form), `hermitian_eigenvalues_3x3(H)`, `hermitian_eigendecomposition_3x3(H)`, `overlap_magnitudes(U, D)`.
- **Constants source:** [`../../../engine/include/ftd/ontic/lemniscate.h`](../../../engine/include/ftd/ontic/lemniscate.h) — `ftd::ontic::G_STAR` (canonical engine-side `G*`; no local duplicate).
- **Reused matrix helpers:** [`../../../engine/include/ftd/color_center.h`](../../../engine/include/ftd/color_center.h) — `ComplexMatrix3`, `matrix_multiply3`, `matrix_close3`, `identity3`, `trace3` (same campaign — Module 2).
- **Test:** [`../../../engine/tests/test_generation_graph.cpp`](../../../engine/tests/test_generation_graph.cpp).
- **CMake:** `ftd_add_test(test_generation_graph … LABELS native)` in `engine/CMakeLists.txt`.
- **LEDGER:** FTD-0196 (`docs/theory/07_assessment/core_ledgers/LEDGER.md`).
- **Sibling overlays** (same campaign): [`../03_derivations/DERIV_BRANCH_HOLONOMY_GAP.md`](../03_derivations/DERIV_BRANCH_HOLONOMY_GAP.md) (branch holonomy `[THEOREM]`, FTD-0194), [`../03_derivations/DERIV_Z3_CENTER_GRAPH_CLOSURE.md`](../03_derivations/DERIV_Z3_CENTER_GRAPH_CLOSURE.md) (Z_3 closure `[THEOREM]`, FTD-0195).

---

*Module 3 of the three-module engine-native overlay pivot (commit `92b7349`). `[CANDIDATE RECONSTRUCTION]` throughout — no spine claim promoted, no `[THEOREM]` tag applied to any flavour-physics claim. The instrument is the deliverable; the candidate match is the diagnostic.*
