# INDEX · Particle Physics Applications

**Tag:** [REFERENCE]
**Date:** 2026-05-22
**Status:** [REFERENCE] — local navigation index for `docs/theory/05_particles/`.
**Purpose:** This cluster applies the framework integers {3, 4, 7, 13} and the master quadratic to Standard Model particle content: lepton and quark mass ratios, electroweak boson masses, the absolute neutrino mass scale, the electron-mass formula, GPU-measured material emergence and color binding, and the division-algebra / octonionic origin of the four integers. Read it when you need a specific particle prediction and its honest epistemic status — most entries here mix genuine derivations with parametric insertions.

## Read first

1. [REF_PHYSICS_REFERENCE.md](REF_PHYSICS_REFERENCE.md) — integer-encoding survey + SM completeness audit; the orientation doc for the cluster.
2. [DERIV_COMPLETE_PARTICLE_PHYSICS.md](DERIV_COMPLETE_PARTICLE_PHYSICS.md) — full SM-observable roll-up with the ~35 [THEOREM] / ~50 [PARAMETRIC] / ~50+ [EXTERNAL] breakdown stated up front.
3. [DERIV_OCTONIONIC_STRUCTURE.md](DERIV_OCTONIONIC_STRUCTURE.md) — why {3, 4, 7, 13} arise from normed division algebras; x₊, x₋ = 70 ± 67 (Heegner).

## Masses & Standard Model spectrum

| File | Tag | Purpose |
|---|---|---|
| [DERIV_COMPLETE_PARTICLE_PHYSICS.md](DERIV_COMPLETE_PARTICLE_PHYSICS.md) | Mixed: ~35 [THEOREM] / ~50 [PARAMETRIC] / ~50+ [EXTERNAL] | SM observables from framework structure; explicit epistemic notice on which predictions are genuine vs imported. |
| [DERIV_ELECTRON_MASS_MOTIVATION.md](DERIV_ELECTRON_MASS_MOTIVATION.md) | [SELECTION] | Factor-by-factor motivation for m_e = M_P·√(2π)·(16/3)·α¹¹ (0.19%); combination not uniquely derived. |
| [PRED_ELECTROWEAK_MASSES.md](PRED_ELECTROWEAK_MASSES.md) | [THEOREM] (running + tree) + [PARAMETRIC] (radiative corr.) | M_Z to 0.02% and M_W to 0.5% from α, sin²θ_W = 3/13, v — zero free parameters. |
| [FOUND_DISCRETE_NATIVE_MASS_GENERATION.md](FOUND_DISCRETE_NATIVE_MASS_GENERATION.md) | [FOUNDATIONAL / OPERATIONAL] | Rest mass operationally defined as voxel cardinality, with linear-level scaling (FTD-0110) and point-group representation analysis. |
| [EXPLR_FTD_MASS_CHAIN.md](EXPLR_FTD_MASS_CHAIN.md) | [arithmetic synthesis — mixed status] | G* → master quadratic → m_e/m_p/m_n arithmetic chain. THEOREM spine; PARAMETRIC / COORDINATE-COINCIDENCE mass matches (mass-unit ≡ m_e; not engine-dynamical). Paired with red-team [AUDIT_MASS_CHAIN_REDTEAM.md](../07_assessment/AUDIT_MASS_CHAIN_REDTEAM.md). |
## Coupling constants & forces (Class C)

| File | Tag | Purpose |
|---|---|---|
| [SPEC_CLASS_C_CLUSTER_INTERACTION.md](../01_reference/SPEC_CLASS_C_CLUSTER_INTERACTION.md) | [INFRASTRUCTURE SPEC] | Cluster-cluster interaction and coupling readout spec (protocol; engine implementation TBD). |

## Color, binding & engine measurements

| File | Tag | Purpose |
|---|---|---|
| [DERIV_MATERIAL_EMERGENCE_FROM_LATTICE.md](DERIV_MATERIAL_EMERGENCE_FROM_LATTICE.md) | [MEASURED] on GPU (FTD-0076) | Smallest particle that emerges from the native genesis rule; quantum-number classification against the SM. |
| [DERIV_COLOR_BINDING_STRUCTURE_AND_ME_STATUS.md](DERIV_COLOR_BINDING_STRUCTURE_AND_ME_STATUS.md) | [MEASURED] (binding) + [SELECTION] (m_e unchanged) (FTD-0077) | 3-quark RGB binding test, SU(3) color-transformation check, and why m_e cannot yet be promoted to [THEOREM]. |

## Algebraic foundations & reference

| File | Tag | Purpose |
|---|---|---|
| [DERIV_OCTONIONIC_STRUCTURE.md](DERIV_OCTONIONIC_STRUCTURE.md) | consolidated derivation | Division algebras and Heegner numbers behind {3, 4, 7, 13}; master-quadratic roots as 70 ± 67. |
| [DERIV_MATERIAL_EMERGENCE_FROM_LATTICE.md](DERIV_MATERIAL_EMERGENCE_FROM_LATTICE.md) | — | (See above — listed under engine measurements.) |
| [REF_PHYSICS_REFERENCE.md](REF_PHYSICS_REFERENCE.md) | [REFERENCE] | Integer-encoding catalog + SM completeness matrix; consolidates the former REF_PHYSICS_ENCODINGS + COMPLETENESS_MATRIX docs. |

---

## Engine-native overlay primitives — candidate reconstructions (2026-05-22)

Pure header-only theory overlays on the existing lattice engine, tagged `[CANDIDATE RECONSTRUCTION]` explicitly — diagnostic instruments, not theorems. No `RenderBridge` touch; the golden-tick hash is preserved.

| File | Tag | Purpose |
|---|---|---|
| [EXPLR_GENERATION_GRAPH_GAMMA_D.md](EXPLR_GENERATION_GRAPH_GAMMA_D.md) | [CANDIDATE RECONSTRUCTION] | Γ_F(d) K₃ triangle on `(q*^{d+1}, 1, q*^d)` with phase `φ=π+π/d`; CKM-shape overlap matrix as diagnostic only (NOT asserted vs experimental CKM). FTD-0196. |

---

10 active docs in this cluster (+ 2 archived).
