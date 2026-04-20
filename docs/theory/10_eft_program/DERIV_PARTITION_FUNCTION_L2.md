# DERIV — Explicit Partition Function on L=2 (Phase J)

**Tag:** [THEOREM] for the computation itself, [OPEN FINDING] for the
interpretation.
**Status:** first explicit FTD partition-function computation (noted as
"Priority #1" in project memory; never attempted before).
**Date:** 2026-04-19
**Script:** `scripts/proofs/partition_function_L2.py`
**Trigger:** user requested a derivation from lattice first principles,
consulting theory docs first.

---

## 1 · Goal

Compute the FTD partition function `Z = ∫ DJ exp(-S_E[J, s])` explicitly
on the smallest nontrivial periodic lattice (L = 2, i.e. 2×2×2 = 8
voxels), using the action specified in
[`SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) §3.3.
Ask: does classical extremisation of S_E fix g_c from first principles?

## 2 · Setup

### 2.1 · Action (static sector)

From `SPEC_FTD_LAGRANGIAN.md` §3.3, the matter Lagrangian is

```
  L_matter = −K_B √((f² − v²)/f) − g_c·s·(∇·J) − λ_G·(∇·J − ρ)²
```

With `v = 0` (static), `f = 1` (weak gravity), `ρ = s` (engine
convention, verified in `poisson_solvers.cpp:123`), and adding the
field-sector expansion `−(c²/2)|∇J|²` from the Born-Infeld core's
weak-field expansion (per SPEC §3.6 items 5–6), the Euclidean static
action becomes

```
  S_E[J, s] = (c²/2) Σ_v |∇J(v)|²        [field-gradient term]
            + g_c   Σ_v s_v (∇·J)(v)     [state-flux coupling]
            + λ_G   Σ_v (∇·J − s)²       [Gauss constraint]
```

In the λ_G → ∞ limit, the constraint `∇·J = s` is exactly enforced.

### 2.2 · Enumeration

For L = 2 with `s ∈ {−1, 0, +1}` per voxel, there are `3^8 = 6561`
possible state configurations. Periodic boundary conditions force
`Σ s = 0` (since `∇·J = s` integrated over the torus must vanish),
reducing to **1107 charge-neutral configurations**.

## 3 · Key identity (Parseval)

For `J = −∇φ` with `∇²φ = −s` on the torus:

```
  ∫ |∇J|² = ∫ |Hessian(φ)|² = ∫ s²   (Parseval + Fourier)
```

In Fourier space: `(∂_i ∂_j φ)(k) = −k_i k_j φ̂(k)`, and
`Σ_{ij} (k_i k_j)² = (k²)²`, while `|φ̂|² = |ŝ|²/(k²)²`. So
`∫ |∇J|² = ∫ (k²)² · |ŝ|²/(k²)² dk = ∫ |ŝ|² = ∫ s²`.

On the lattice this holds up to discrete corrections; I verified
numerically on L = 2 that `∫ |∇J|² = Σ s²` exactly for every config.

## 4 · Critical result

After enforcing the Gauss constraint `∇·J = s`:

```
  S_E[J_min, s] = (c²/2) · Σ s²  +  g_c · Σ s²
                = (c²/2 + g_c) · N_manifested
```

where `N_manifested = Σ s_v² = Σ 1[s_v ≠ 0]` is just the count of
non-void voxels.

**S_E depends only on the number of charges, not on their placement.**

### 4.1 · Empirical verification (L = 2, g_c = 1)

Two dipole configurations:

| Placement | Separation | S_E | Σ\|J\|² (engine diagnostic) |
|---|:--:|:--:|:--:|
| Dipole: +1 at (0,0,0), −1 at (0,0,1) | 1 | 2.333 | 0.292 |
| Dipole: +1 at (0,0,0), −1 at (1,1,1) | √3 | 2.333 | 0.417 |
| **Difference** | | **0** | **0.125** |

The analytical action `S_E` is identical for the two placements — the
Lagrangian distinguishes nothing about the relative positions of
charges. The engine's `Σ|J|²` diagnostic (the classical EM field
energy, × 2 per the Phase G convention audit) DOES distinguish.

## 5 · Structural consequence

**The FTD analytical action as written in SPEC_FTD_LAGRANGIAN.md
contains no Coulomb interaction between static charges.** What the
engine exhibits as Coulomb-like dynamics comes from mechanisms that are
NOT in the analytical action:

1. **`Σ|J|²` energy diagnostic** (engine's `field_energy`): mathematically
   `∫|∇φ|²` = classical EM field energy, giving Phase G's geometric
   Coulomb `2·r·G_L(r)`. This is a *diagnostic*, not part of `S_E`.

2. **`solve_coulomb_poisson()`**: a separate Poisson solve
   `∇²φ_Coulomb = −s`, followed by explicit force `F = −α·s·∇φ_Coulomb`
   with hardcoded α. Used by the `poisson_coulomb` toggle. Inserts α
   parametrically.

3. **`emergent_forces` toggle**: computes force from flux gradient,
   matching Phase G geometric Coulomb.

None of (1), (2), (3) is derived from `S_E`. Mechanism (1) is a
parallel energy bookkeeping; (2) is an additional solve with α
inserted; (3) is an alternative force computation.

## 6 · Implication for first-principles g_c

Since `S_E` is independent of charge placement, **classical
extremisation of `S_E` with respect to charge positions does not fix
g_c**. You cannot derive g_c from "what value makes the action
consistent" — any value works equally well.

This is the **explicit confirmation** of Phase I Option 2's theoretical
argument (Mechanism A ruled out: no topological quantisation) extended
to Mechanism C (self-consistent fixed point: no variational fixed
point exists in `S_E`).

Mechanism B (lattice-to-continuum matching) remains the only viable
route — but it requires promoting the classical action to a quantum
path integral with explicit UV regulator and computing the 1-loop
Wilson coefficient relating the bare lattice g_c to the continuum
renormalised coupling. That's a separate program, not a straightforward
classical computation.

## 7 · What this DOES derive from first principles

The Phase J computation is not zero-result. It establishes:

1. **Lattice Green's function `G_L(r)`** on L=2 has 4 distinct values
   by cubic symmetry: G_L(0,0,0) = 0.151, G_L(1,0,0) = 0.00521,
   G_L(1,1,0) = −0.0365, G_L(1,1,1) = −0.0573. These are computable
   from first principles (7-pt Laplacian eigenvalues), no α required.

2. **Neutral-config count** on L=2 is 1107 out of 3⁸ = 6561 — a pure
   lattice-combinatoric result from the ternary alphabet plus zero-sum
   constraint.

3. **Parseval identity** `∫|∇J|² = ∫s²` for J = −∇φ solving
   ∇²φ = −s. Exact in the continuum; verified on L=2.

4. **Ultralocality of the analytical action**: `S_E` is a local
   functional of s (counts manifestations), not a pairwise functional.
   This is a rigorous structural statement about the FTD Lagrangian.

These are all [THEOREM] at the level of the lattice combinatorics and
the FTD action's structure. They do NOT derive α.

## 8 · Recommended epistemic updates

This computation tightens the Phase I audit:

- **Mechanism C (self-consistent fixed point)** in
  `OPEN_GC_FROM_FIRST_PRINCIPLES.md` §2.3 should be flagged **RULED
  OUT** at the classical level (with this caveat: a quantum path
  integral with explicit UV regulator and fluctuation determinants
  might still produce a fixed point. That's beyond classical
  extremisation).

- **The gap-equation narrative** in
  `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` needs explicit
  acknowledgement that the "self-consistency" operational rule is not
  derivable from the FTD action by minimisation alone. The
  "one-loop effective coupling" claim assumes a path integral that
  has never been constructed.

- **The master quadratic's dual-prediction** (x_+ ↔ 1/α, x_- ↔ N_c)
  remains the primary evidence for the lattice's relevance to α. It
  lives entirely in the motivic/algebraic structure (Watson identity,
  CM curve periods, Moore-neighbourhood integers) — NOT in the
  dynamical action.

## 9 · Reproducibility

```
scripts/proofs/partition_function_L2.py    # this script (600 lines)
docs/theory/01_reference/SPEC_FTD_LAGRANGIAN.md  # source action
engine/src/poisson_solvers.cpp:123          # engine's Gauss implementation
```

Run:
```bash
PYTHONIOENCODING=utf-8 python scripts/proofs/partition_function_L2.py
```

## 10 · One-paragraph summary

We computed the FTD partition function explicitly on the 2×2×2
periodic torus with action as specified in SPEC_FTD_LAGRANGIAN.md §3.3.
Under the Gauss constraint `∇·J = s`, the action's value depends only
on the number of manifested voxels, not on their placement — the
Lagrangian is *ultralocal* in the state field. This means **classical
extremisation of S_E cannot fix the coupling g_c**, and the Coulomb
physics that the engine exhibits (including the Phase G geometric
α_r = 2·r·G_L(r)) does NOT come from the analytical action. It comes
from parallel diagnostics and separate Poisson solves. The
"first-principles derivation of α from the lattice action alone" is
not achievable within the current FTD formulation; additional input
(quantum path integral with fluctuation determinants, or acceptance
of the master quadratic's algebraic match as primary) is required.
