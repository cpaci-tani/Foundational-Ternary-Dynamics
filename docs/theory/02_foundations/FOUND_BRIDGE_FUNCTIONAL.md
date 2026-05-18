# FOUND — The Bridge Functional: mass as functional on the master-quadratic root spectrum

**Tag:** [SELECTION]
**Date:** 2026-04-26
**LEDGER row:** FTD-0095
**Dependencies:** FTD-0001 (master quadratic), FTD-0028 (Moore Layer Theorem)
**Dependents:** FTD-0094 (L2 candidate identity)
**Status:** Structural commitment with [OPEN] derivation target.

---

## 1 · Mass-as-functional declaration

In the legacy presentation of the Standard Model, **mass** appears as a primitive monadic property — a particle has a rest mass, full stop. The relational character of mass is acknowledged only in degenerate cases (binding energy, anomalous mass corrections, the running quark mass).

This document commits FTD to the structural-realist alternative:

> **MASS IS NOT MONADIC.** Within FTD, mass is the value of a functional
>
>     M : Couplings → ℝ
>
> evaluated on the root spectrum of the master quadratic
>
>     x² − 16G*² x + 16G*³ = 0    (FTD-0001, [THEOREM]).

The root spectrum `(x₊, x₋) = (137.036, 3.024)` carries all available information about the two coupling sectors the lattice supports. Any mass scale that FTD assigns must, on this commitment, be a Sₙ-invariant functional of that spectrum, modulo the dimensional calibration `mass-unit ≡ m_e/K_B = 1 MeV/c²` (THEOREM_A_PHYS_NO_GO + FTD-0041).

This is the OSR (ontic structural realism) move — Worrall, Ladyman, Ross — applied locally to the FTD ontology. We do not claim it is true *of nature*; we claim it is true *of FTD as a model*, and we adopt it as the working ontology for the master-quadratic chain.

## 2 · Arithmetic-mean derivation target — [OPEN]

The L2 candidate identity (FTD-0094) implicitly selects the *arithmetic mean* as the bridge functional:

  M(x₊, x₋) = α · (x₊ + x₋) / 2 = 8αG*²    [CONJECTURE; 68.77 ppm vs CODATA m_e]

But the master-quadratic spectrum admits other Sₙ-invariant functionals:

| Functional | Value | Slogan |
|---|---|---|
| **Arithmetic mean** | (x₊ + x₋)/2 = 8G*² | "trace / 2" — what L2 chooses |
| Geometric mean | √(x₊·x₋) = 4G*^(3/2) | "discriminant root" |
| Harmonic mean | 2x₊x₋/(x₊+x₋) = 2G* | "Vieta product over trace" |
| Maximum / Minimum | x₊ or x₋ | sectoral, not symmetric |
| Polynomial trace power | (x₊^k + x₋^k)^(1/k) | one-parameter family |

Each gives a different mass formula. **L2 imports an unargued metaphysical commitment** — that the EM and color sectors contribute *additively and equally* to inertia. Without an independent derivation of why the arithmetic mean is the physically correct bridge functional, this is precisely Whitehead's *fallacy of misplaced concreteness*: taking a convenient mathematical operation (sum-then-halve) and treating it as ontologically given.

The following claim is therefore [OPEN]:

> **(Arithmetic-mean theorem, target).** Let `B : J → s` be the bridge operator
> mapping dispositional flux to actual state, defined on the BCC sub-stencil
> σ_BCC ⊂ Moore-26. Then the eigenvalues of `B` on the BCC band edge are
> `(α·x₊, α·x₋)`, and the manifestation-mass quantum equals their
> *arithmetic* mean.

Closure routes (preferred → speculative):

1. **Variational principle on σ_BCC.** Show that the lattice action restricted to σ_BCC has a saddle-point structure where x₊ and x₋ enter as the two stationary eigenvalues, and that the *trace* (not norm or product) is what couples to the mass current. Status: not attempted.

2. **'t Hooft beable interpretation.** Treat the two roots as the *two ontic states* of a single beable (master beable). Under the unbroken-phase equiprobability assumption,
   `⟨x⟩ = (x₊ + x₋)/2`
   is forced by the stationary measure of the beable. Status: a slogan; needs a formal beable model.

3. **Polynomial trace theorem.** Show that for any polynomial whose coefficients descend from Chowla–Selberg + CM-curve uniqueness, the *first elementary symmetric function* (Vieta trace) is the unique Sₙ-invariant functional that respects the regulator pairing. Status: speculative — requires a Beilinson regulator argument that does not currently exist for G*.

If none of these closes within a structurally consistent scope, the arithmetic-mean rule is demoted to [PARAMETRIC] and the Bridge Functional ontology survives only at the *typed-functional* level (M is some unspecified Sₙ-invariant functional).

## 3 · 't Hooft beable equivalence path

Gerard 't Hooft's "cellular automaton interpretation of quantum mechanics" treats the ontic substrate as deterministic, with quantum superposition emerging from the stationary measure of unobserved variables. Translated to FTD:

- The two master-quadratic roots `(x₊, x₋)` are the two *beable states* of an unbroken phase.
- In equilibrium, the stationary measure puts equal weight on both — neither is preferred.
- The expectation `⟨x⟩` of the beable's eigenvalue under that measure is
  `⟨x⟩ = (x₊ + x₋)/2` ,
  which is the arithmetic mean by the symmetry of the measure.
- Mass, on this reading, is `α · ⟨x⟩` — the EM-coupling-rescaled stationary expectation.

This is structurally suggestive. It does **not** constitute a derivation. It does, however, give the arithmetic-mean rule a physical pedigree (equilibrium of a beable) rather than an aesthetic one (Vieta trace looks pretty). Whether the unbroken-phase equiprobability assumption is actually correct is a separate open question.

Cross-references:
- 't Hooft, *The Cellular Automaton Interpretation of Quantum Mechanics* (Springer, 2016).
- `docs/theory/03_derivations/DERIV_QM_FROM_LATTICE.md` — FTD's own QM-from-lattice route, which shares structural assumptions with 't Hooft.

## 4 · Slogan upgrade

Previous slogan in `docs/theory/02_foundations/FOUND_MASTER_QUADRATIC_*` (where present): *"the master quadratic predicts α and N_c"*.

Adopted in this document, conditional on the arithmetic-mean derivation closing positive:

> **"Mass is the stationary expectation of the master beable, computed by Vieta."**

This slogan is preferable because:
- It names the bridge functional explicitly (Vieta trace).
- It locates mass in the bridge between dispositional flux (J) and actual state (s) — the right ontological tier.
- It is honestly conditional — both on the bridge operator existing and on the beable interpretation surviving.

Until the [OPEN] in §2 closes, the slogan is provisional. Until then, the prior phrasing should remain in published material, with the new slogan reserved for the post-closure documents.

## 5 · Cross-references

- `THEOREM_MOORE_LAYER_DECOMPOSITION.md` (FTD-0028): polyhedral decomposition that gives U(1) × SU(2) × SU(3) and the BCC sub-stencil.
- `docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md`: master quadratic chain.
- `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`: master quadratic algebraic identity.
- `docs/theory/10_eft_program/archive/closed_negative/DERIV_MECHANISM_C_GC_BCC_BRIDGE.md` (FTD-0093, closed negative): structural derivation attempt for the BCC bridge operator.
- `docs/theory/10_eft_program/PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md` (FTD-0093, closed negative): falsifier.
- `docs/theory/10_eft_program/OPEN_MU_FROM_LP_MISSING_ARROW.md` (FTD-0096): the calibration-side broken arrow.

## 6 · What this document does NOT claim

- It does **not** claim the arithmetic-mean rule is derived. It is asserted; the derivation is [OPEN] (§2).
- It does **not** claim mass is *actually* relational in nature. It claims mass-as-functional is the working ontology *within FTD*.
- It does **not** retire the previous monadic-mass slogans across the manuscript portfolio. Those remain valid until the [OPEN] closes.
- It does **not** specify a unique bridge operator B. §2 names it as a [target], not an extant object.
- It does **not** entail any commitment to equiprobability of (x₊, x₋) outside the unbroken-phase context where 't Hooft beable interpretation applies.

## 7 · Status summary

| Claim | Status | Note |
|---|---|---|
| Mass is a functional `M : Couplings → ℝ` (typed) | [SELECTION] | Adopted commitment |
| Master-quadratic spectrum is the input to M | [SELECTION] | Follows FTD-0001 |
| The functional is the arithmetic mean | [OPEN] | §2 — derivation target |
| 't Hooft beable equivalence | [CONJECTURE] | §3 — slogan-level |
| Vieta-trace slogan upgrade | conditional | §4 — gated by §2 closure |
