# DERIV — Branch Holonomy Gap on a Periodic Torus

**Tag:** [THEOREM]
**Date:** 2026-05-22
**LEDGER:** FTD-0194
**Engine primitive:** [`../../../engine/include/ftd/branch_holonomy.h`](../../../engine/include/ftd/branch_holonomy.h)
**Constructive verification:** [`../../../engine/tests/test_branch_holonomy_gap.cpp`](../../../engine/tests/test_branch_holonomy_gap.cpp) — `ctest -R "^branch_holonomy_gap$"` passes 1/1, all six sub-tests green at machine precision (λ_min match to 1e-12).
**Status:** `[THEOREM]` — finite-group / linear-algebra theorem about the Z_2-twisted Laplacian on a finite cyclic group; equivalent to the antiperiodic-boundary-condition spectrum on a periodic ring.

---

## §1 — Statement

> On a periodic 1D ring of `N ≥ 2` sites, equip each edge `e_i = (i, (i+1) mod N)` with a sign `σ_i ∈ {+1, −1}`. Define the signed difference operator `D_σ` mapping vertex functions to edge functions by `(D_σ U)_e_i = U_{(i+1) mod N} − σ_i · U_i` and the signed graph Laplacian `L_σ = D_σ^T D_σ` (symmetric, positive semi-definite). Let the **Z_2 holonomy** be `H = ∏_i σ_i ∈ {+1, −1}`.
>
> **Theorem (eq. 1).** If `H = −1` (any odd number of edge flips), then
>
>     λ_min(L_σ)  =  4 · sin²( π / (2N) ).

The spectrum depends only on `H`, not on which specific edges carry the sign flips (gauge equivalence — proof §3.2). For `H = +1` the spectrum is the standard periodic Laplacian's `4 sin²(πk/N)`, `k = 0, …, N−1`, with `λ_min = 0` (constants in the kernel).

---

## §2 — Setup

The signed Laplacian on the ring is N×N, tridiagonal with periodic wrap-around:

| | column `i−1` | column `i` | column `i+1` | else |
|---|---|---|---|---|
| row `i` | `−σ_{i−1}` | `2` | `−σ_i` | `0` |

(All indices mod N.) Sum of contributions: each edge `e_i` adds `+1` to `L_{ii}` and `L_{i+1,i+1}` and `−σ_i` to `L_{i,i+1}` and `L_{i+1,i}`. Hermitian / symmetric by construction.

A Z_2 "branch twist" `H = −1` is exactly the lattice analogue of an antiperiodic boundary condition: a wave-function picks up a sign factor `−1` once around the ring.

---

## §3 — Proof

### §3.1 Spectrum in the twisted sector

Diagonalise via Fourier modes. Without sign flips (`H = +1`) the eigenvectors are `e^{i k j}`, `k ∈ {2πm/N : m = 0, …, N−1}`, with eigenvalues `λ(k) = 2 − 2 cos k = 4 sin²(k/2)`. The lowest is `k = 0`, giving `λ_min = 0`.

With one sign flip on edge `e_0` (i.e. `σ_0 = −1`, all other `σ_i = +1`), the eigenvalue problem is `2 U_i − U_{i−1} − U_{i+1} = λ U_i` for `i ≠ 0` and `2 U_0 + U_{−1} − U_1 = λ U_0` (the `−σ_{−1} = +1` and `−σ_0 = +1` give the modified row). The ansatz `U_i = e^{i k i}` works iff `e^{i k N} = −1`, i.e.,

    k  ∈  {π (2m + 1) / N  :  m = 0, 1, …, N−1}.

These are the **half-integer** Brillouin-zone momenta — exactly the antiperiodic-BC quantisation. The eigenvalues become

    λ_m  =  4 · sin²( π (2m + 1) / (2N) ),    m = 0, 1, …, N−1.

The lowest is at `m = 0`:

    λ_min  =  4 · sin²( π / (2N) ).     ∎

### §3.2 Gauge equivalence (only `H` matters)

Two sign configurations with the same `H = ∏_i σ_i` are related by a vertex Z_2 gauge transformation `U_j ↦ ε_j U_j`, `ε_j ∈ {+1, −1}`, which induces `σ_i ↦ ε_i ε_{i+1} σ_i`. Choosing `ε_j = ∏_{k < j} σ_k` (cumulative product) propagates a single flip's effect to a desired edge; the spectrum of `L_σ` is invariant under this transformation (it is unitary equivalence). Hence the spectrum depends only on `H`, not on the flip locations.

### §3.3 3D corollary

On a 3D torus `N×N×N` with a single Z_2 twist along the `x`-cycle, the eigenvalues are `4 [sin²(k_x/2) + sin²(k_y/2) + sin²(k_z/2)]` with `k_x ∈ {π(2m+1)/N}` (antiperiodic) and `k_y, k_z ∈ {2πn/N}` (periodic). The minimum is at `k_x = π/N`, `k_y = k_z = 0`, giving exactly the same `λ_min = 4 sin²(π/(2N))`. The 1D primitive captures the 3D gap.

---

## §4 — Constructive verification

`engine/tests/test_branch_holonomy_gap.cpp` is the constructive proof at four discrete `N`-values. It uses an inline cyclic-Jacobi eigensolver on the explicit `N×N` matrix and compares the full spectrum to the closed-form (eq. 1 plus the trivial-sector form):

| Sub-test | What it checks |
|---|---|
| T1 | Trivial holonomy (`H = +1`): spectrum matches `{4 sin²(πk/N)}_{k}`; `λ_min = 0` (constants in kernel). |
| T2 | Z_2 twist (`H = −1`): spectrum matches `{4 sin²(π(2m+1)/(2N))}_{m}`; **`λ_min = 4 sin²(π/(2N))`** to 1e-12. |
| T3 | Gauge equivalence: flipping edge 0, 3, or 7 (in `N = 8`) gives the identical spectrum. |
| T4 | Parity: any odd number of flips → twisted spectrum; any even → trivial spectrum. |
| T5 | `apply(U) == build_matrix() * U` to round-off. |
| T6 | Input validation: `N < 2`, wrong vector length, and signs `∉ {±1}` all throw. |

`N ∈ {4, 8, 16, 32}` covered for T1 and T2. Sample (N = 32 twisted): numerical `0.00963054665561`, theorem `0.00963054665561` — match to all 12 printed digits.

---

## §5 — Scope and what this is

This is a finite-group / linear-algebra theorem about the Z_2-bundle Laplacian on a discrete cyclic graph. It is `[THEOREM]` because the Fourier-mode quantisation argument is exact and the closed form is a direct consequence.

The header is a **pure overlay** on the engine's existing lattice infrastructure (it does not touch `RenderBridge`, the tick cycle, or any physics toggle; the golden-tick hash is preserved). The test is `unit`/`native`-labelled and runs in ~10 ms with no GPU dependency.

The gap formula has downstream physical readings (antiperiodic fermionic boundary conditions on a torus, vortex defect energy in `O(2)`/`U(1)` models, etc.) — those readings are separate work and are not promoted by this theorem on its own.

---

## §6 — Cross-references

- **Engine primitive:** [`../../../engine/include/ftd/branch_holonomy.h`](../../../engine/include/ftd/branch_holonomy.h) — `SignedRing1D` class, `apply()` / `build_matrix()`, `torus_branch_twist_gap_1d(N)`, `twisted_ring_spectrum_closed_form(N)`, `trivial_ring_spectrum_closed_form(N)`.
- **Test:** [`../../../engine/tests/test_branch_holonomy_gap.cpp`](../../../engine/tests/test_branch_holonomy_gap.cpp).
- **CMake:** registered as `ftd_add_test(test_branch_holonomy_gap … LABELS native)` in `engine/CMakeLists.txt`, adjacent to `test_sublattice_laplacian` (same conceptual cluster of signed / sub-stencil Laplacians).
- **LEDGER:** FTD-0194 (`docs/theory/07_assessment/LEDGER.md`).
- **Sibling overlays** (same campaign): [`DERIV_Z3_CENTER_GRAPH_CLOSURE.md`](../standard_model/DERIV_Z3_CENTER_GRAPH_CLOSURE.md) (Z_3 color-center closure, FTD-0195), [`../05_particles/EXPLR_GENERATION_GRAPH_GAMMA_D.md`](../05_particles/EXPLR_GENERATION_GRAPH_GAMMA_D.md) (generation graph `Γ_F(d)`, `[CANDIDATE RECONSTRUCTION]`, FTD-0196).

---

*Module 1 of the three-module engine-native overlay pivot (commit `5b06324`). No `RenderBridge` change, no physics toggle wired, no spine claim promoted or demoted.*
