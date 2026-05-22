# INDEX · Structural & Geometric Principles

**Tag:** [REFERENCE]
**Date:** 2026-05-22
**Status:** [REFERENCE] — local navigation index for `docs/theory/08_structural/`.
**Purpose:** This cluster derives FTD's geometric and combinatorial backbone from the cubic lattice: the Moore-neighborhood polyhedral decomposition, the BCC multiplicative structure that unifies the Watson identity with SU(3), the cuboctahedral origin of the framework integers {3,4,7,13}, the coefficient 16, the two-layer flux/state ontology, and the trit information-theoretic lens on G*. Read this cluster to understand *why* FTD's numbers are lattice geometry rather than fits.

---

## Read first

Newcomers should read these in order:

1. [THEOREM_MOORE_LAYER_DECOMPOSITION.md](THEOREM_MOORE_LAYER_DECOMPOSITION.md) — the master combinatorial theorem: SM gauge groups + particle content from the Moore neighborhood.
2. [DERIV_CUBOCTAHEDRAL_INTEGERS.md](DERIV_CUBOCTAHEDRAL_INTEGERS.md) — the four framework integers {3,4,7,13} as cuboctahedral geometry.
3. [DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md](DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md) — why the gap-equation coefficient and SU(3) share one BCC origin.
4. [DERIV_DUAL_DERIVATION_OF_16.md](DERIV_DUAL_DERIVATION_OF_16.md) — the master-quadratic coefficient 16 from two independent routes.

---

## Moore-neighborhood decomposition

The polyhedral structure of the 26-neighbor Moore neighborhood and the gauge / particle content it generates.

| File | Tag | Purpose |
|---|---|---|
| [THEOREM_MOORE_LAYER_DECOMPOSITION.md](THEOREM_MOORE_LAYER_DECOMPOSITION.md) | [THEOREM] / [SELECTION] | Moore neighborhood decomposes into D polyhedral layers → U(1)×SU(2)×SU(3), 3 generations, 4 particles, 17 dark states. |
| [DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md](DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md) | structural theorem + [SELECTION] | The BCC cosine-product eigenvalue unifies the Watson integral (gap coefficient) with the SU(3) color group. |

---

## Framework integers from lattice geometry

How {N_c, N_base, b_3, N_eff} and the coefficient 16 arise as structural invariants of ℤ³.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_CUBOCTAHEDRAL_INTEGERS.md](DERIV_CUBOCTAHEDRAL_INTEGERS.md) | [THEOREM] | {3,4,7,13} uniquely determined by cuboctahedron geometry; resolves the integer-circularity problem. |
| [DERIV_EXISTENTIAL_UNIT.md](DERIV_EXISTENTIAL_UNIT.md) | [THEOREM] §1–8, [CONJECTURE] §9 | The 3³ lattice as minimal complete lattice; integers sum to N_c³, selecting N_c = 3. |
| [DERIV_DUAL_DERIVATION_OF_16.md](DERIV_DUAL_DERIVATION_OF_16.md) | [THEOREM] | Coefficient 16 from \|Aut(E_i)\|² and the O_h axis-stabilizer index — same number, two routes. |
| [DERIV_STABILIZER_DECOMPOSITION.md](DERIV_STABILIZER_DECOMPOSITION.md) | [THEOREM] | Stab_{O_h}(e₃) ≅ D₄ × ℤ/2; bridges CM-curve Aut(E_i) to cubic lattice symmetry. |

---

## Bound-state cluster geometry

The geometric interpretation of the engine's emergent 25-voxel cluster (FTD-0107).

| File | Tag | Purpose |
|---|---|---|
| [EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md](EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md) | [EXPLORATORY] (§3 hypothesis refuted) | 25 = 2nd centered octahedral number; the L¹-ball-radius-2 topology prediction was refuted by the engine. |
| [EXPLR_OCTAHEDRAL_BOUND_STATES.md](EXPLR_OCTAHEDRAL_BOUND_STATES.md) | [EXPLORATORY] (duality reading refuted) | Volumetric properties of the octahedral cluster family; the polytope-duality interpretation was refuted. |

---

## Two-layer ontology & information theory

The flux/state duality and the information-theoretic lens on G*.

| File | Tag | Purpose |
|---|---|---|
| [EXPLR_LOOP_GRID_DUALITY.md](EXPLR_LOOP_GRID_DUALITY.md) | [SELECTION] | The continuous flux (Loop) vs discrete state (Grid) layers as a fundamental continuous/discrete duality. |
| [EXPLR_TRIT_INFORMATION_THEORY.md](EXPLR_TRIT_INFORMATION_THEORY.md) | Foundational extension | G* = √(2π)·ϑ₃(e^−π)² ties geometric self-reference to spectral (Fourier) self-duality. |
| [EXPLR_PHASE_LATTICE_MOORE.md](EXPLR_PHASE_LATTICE_MOORE.md) | [THEOREM]/[SELECTION]/[CONJECTURE] | The phase lattice {π,ϖ,G*}³ on (S¹)³ as a phase-space avatar of the Moore neighborhood. |

---

## Pre-registrations & audit results

Hash-locked pre-registrations of structural audits in this cluster — the prose is fixed and SHA256-pinned before any matching is attempted — and the audit results that execute them (registry: [`../10_eft_program/REF_PREREGISTER_MANIFEST.md`](../10_eft_program/REF_PREREGISTER_MANIFEST.md)).

| File | Tag | Purpose |
|---|---|---|
| [PREREG_FINITE_NEUTRAL_LOCK_v1.md](PREREG_FINITE_NEUTRAL_LOCK_v1.md) | [PRE-REGISTRATION] | Q10 finite-neutral-lock audit (FTD-0190): the minimal finite closure object that locks an internal two-state opposition sector while leaving a massless U(1)-like neutral readout — and whether its continuum shadow carries the electroweak doublet (1,2)_{1/2}. Desk audit of the frozen structural inventory; no numerical search. |
| [AUDIT_FINITE_NEUTRAL_LOCK.md](AUDIT_FINITE_NEUTRAL_LOCK.md) | [AUDIT FINDING] | Result of the Q10 audit (FTD-0190): **UNDERDETERMINED**. The catalog supplies every ingredient of the finite neutral-lock skeleton — a two-state opposition with a derivable ±½ doublet normalisation from ℤ[i]^×≅ℤ₄, a rank-1 U(1)-shadow, colour-singlet compatibility, hypercharge forced — but not a *forced* rank-2→rank-1 assembly. No falsifier fired. Verifier: `scripts/proofs/audit_finite_neutral_lock.py`. |
| [PREREG_COLOUR_SINGLET_RANK_v1.md](PREREG_COLOUR_SINGLET_RANK_v1.md) | [PRE-REGISTRATION] | Q11 colour-singlet rank audit (FTD-0191): is FTD's colour-singlet, internal abelian rank forced to exactly 2 (the electroweak rank SU(2)×U(1))? The successor question that decides Q10 — FOUND lifts FTD-0190, CLOSED-NEGATIVE closes it. Cyclic-subgroup enumeration of the frozen catalog; no numerical search. |
| [AUDIT_COLOUR_SINGLET_RANK.md](AUDIT_COLOUR_SINGLET_RANK.md) | [AUDIT FINDING] | Result of the Q11 audit (FTD-0191): **UNDERDETERMINED**; Q10 stays UNDERDETERMINED. The catalog forces exactly one colour-singlet internal rank-1 U(1)-shadow (ℤ[i]^×); the second (the weak SU(2) Cartan on φ=J_L−J_R) is only [SELECTION]-grade. Sharpens Q10: the electroweak-rank question reduces to one named [SELECTION] — the "Q12" target. Verifier: `scripts/proofs/audit_colour_singlet_rank.py`. |
| [PREREG_WEAK_SU2_PROVENANCE_v1.md](PREREG_WEAK_SU2_PROVENANCE_v1.md) | [PRE-REGISTRATION] | Q12 weak-SU(2) provenance audit (FTD-0192): is the weak SU(2) on φ=J_L−J_R a genuine derivation or a count-match? The terminating step of the Q10→Q11→Q12 chain — GENUINE lifts FTD-0190+0191 to FOUND, COUNT-MATCH closes FTD-0190 negative. A provenance audit of the existing derivation `DERIV_LATTICE_SU2_WEAK.md`. |

---

16 active docs in this cluster (+ 0 archived).
