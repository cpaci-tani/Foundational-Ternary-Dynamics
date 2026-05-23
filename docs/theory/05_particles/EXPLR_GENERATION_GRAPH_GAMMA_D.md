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

- a derivation of the K_3 edge-phase distribution from substrate dynamics (this module assumes uniform `e^{iφ}` on the upper triangle — a convention choice, not a derivation);
- a derivation of the eigenvector-ordering convention;
- agreement at machine precision with the experimental CKM magnitudes (the current Frobenius deviation is `≈ 0.36`, see §4) — at minimum a tightening of the convention is required.

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

Define `Γ_F(d)` as the 3×3 Hermitian matrix on three vertices `(0, 1, 2)`:

    Γ_F(d)_{kk}   = q*^{d+1},  1,  q*^d         for k = 0, 1, 2
    Γ_F(d)_{k≠l} = e^{iφ(d)}   in upper triangle  (k < l)
                   e^{−iφ(d)}  in lower triangle  (k > l)
    φ(d)          = π + π/d                              (eq. 2)

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
| T7 | **INFORMATIONAL** — prints `|V_{ij}|`, the owner-supplied candidate target, and the absolute deviation. **No assertion**. |

All six asserted sub-tests pass at machine precision.

---

## §4 — Computed values (informational)

Numerical output from the test:

```
Γ_F(3) spectrum:  −1.585  0.651  2.017
Γ_F(2) spectrum:  −1.397  0.388  2.219

|V_{ij}| (computed):
    0.958533   0.209116   0.193609
    0.249749   0.943537   0.217631
    0.137257   0.256920   0.956636

owner-supplied candidate target (CKM-shape, NOT asserted):
    0.973536   0.228440   0.006537
    0.228336   0.972678   0.041952
    0.009485   0.041385   0.999098

abs deviation:
    0.015003   0.019324   0.187072
    0.021413   0.029141   0.175679
    0.127772   0.215535   0.042462

Frobenius deviation = 0.363799
```

The **diagonal-dominant CKM-shape is recovered structurally** (V_{ii} all ≈ 0.94–0.96, V_{ij≠i} all < 0.26). The precise off-diagonal magnitudes depend on the K_3 edge-phase distribution convention — this module uses uniform `e^{iφ}` on the upper triangle, which is one of several admissible choices; matching the owner's Python prototype's convention would change the off-diagonals. Tightening the convention is `[OPEN]` follow-up work, not in scope here.

---

## §5 — Honest tagging of the components

| Component | Tag | Why |
|---|---|---|
| `Γ_F(d)` matrix construction (eq. 2) | `[CONSTRUCTION]` (pure choice of parametrisation) | The K_3 + uniform-phase form is one admissible convention; not unique. |
| `q* = (G* − √(G*²−4))/2` (eq. 1) | `[DERIVED]` from G\* via a quadratic root | Standard arithmetic; G\* is the substrate input (`ftd::ontic::G_STAR`). |
| Real, sorted eigenvalues from cubic | `[THEOREM]` (Smith-1961 stable cubic for Hermitian 3×3) | Standard linear algebra. |
| Orthonormal eigenvectors / unitarity of `V` | `[THEOREM]` (Hermitian linear algebra) | Standard. |
| `Γ_F(d)` → flavour generations | `[SELECTION]` (the labelling `Γ_U` = Γ_F(3), `Γ_D` = Γ_F(2)) | Interpretation, not derivation. |
| `\|V_{ij}\|` ↔ experimental CKM | `[CANDIDATE RECONSTRUCTION]` overall; the equality is `[CONJECTURE]` at best | A structural resemblance is not a derivation (GTCA F1/F10). |

The LEDGER row (FTD-0196) records: the `Γ_F(d)` flavour graph as `[CANDIDATE RECONSTRUCTION]`, and the CKM-like overlap as `[RECONSTRUCTION / diagnostic]`.

---

## §6 — Scope

This module is a **pure header-only overlay** on the existing engine — no `RenderBridge` touch, no physics-toggle wiring, no engine state modified. The golden-tick hash is preserved. The test is `unit`/`native`-labelled and runs in ~10 ms with no GPU dependency.

The eigensolver (Smith-1961 stable cubic + cross-product null vectors) is a general-purpose 3×3 Hermitian utility usable by any downstream FTD-native theory module that needs small Hermitian spectra without an external LAPACK dependency.

---

## §7 — Cross-references

- **Engine primitive:** [`../../../engine/include/ftd/generation_graph.h`](../../../engine/include/ftd/generation_graph.h) — `q_star()`, `gamma_F(d)`, `hermitian_eigenvalues_3x3(H)`, `hermitian_eigendecomposition_3x3(H)`, `overlap_magnitudes(U, D)`.
- **Constants source:** [`../../../engine/include/ftd/ontic/lemniscate.h`](../../../engine/include/ftd/ontic/lemniscate.h) — `ftd::ontic::G_STAR` (canonical engine-side `G*`; no local duplicate).
- **Reused matrix helpers:** [`../../../engine/include/ftd/color_center.h`](../../../engine/include/ftd/color_center.h) — `ComplexMatrix3`, `matrix_multiply3`, `matrix_close3`, `identity3`, `trace3` (same campaign — Module 2).
- **Test:** [`../../../engine/tests/test_generation_graph.cpp`](../../../engine/tests/test_generation_graph.cpp).
- **CMake:** `ftd_add_test(test_generation_graph … LABELS native)` in `engine/CMakeLists.txt`.
- **LEDGER:** FTD-0196 (`docs/theory/07_assessment/LEDGER.md`).
- **Sibling overlays** (same campaign): [`../03_derivations/DERIV_BRANCH_HOLONOMY_GAP.md`](../03_derivations/DERIV_BRANCH_HOLONOMY_GAP.md) (branch holonomy `[THEOREM]`, FTD-0194), [`../03_derivations/DERIV_Z3_CENTER_GRAPH_CLOSURE.md`](../03_derivations/DERIV_Z3_CENTER_GRAPH_CLOSURE.md) (Z_3 closure `[THEOREM]`, FTD-0195).

---

*Module 3 of the three-module engine-native overlay pivot (commit `92b7349`). `[CANDIDATE RECONSTRUCTION]` throughout — no spine claim promoted, no `[THEOREM]` tag applied to any flavour-physics claim. The instrument is the deliverable; the candidate match is the diagnostic.*
