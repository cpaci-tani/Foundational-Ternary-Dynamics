# The L² Wall as Closure-Conservation — Polyhedrality Is Conserved

**Status:** [DERIVED — formalization] + [SYNTHESIS] of FTD-0208 v3 · **Verifier:** `scripts/proofs/proof_l2_closure_recast.py` (5/5 PASS) · **Date:** 2026-07-05
**Program:** Clause-2/3 boundary program, stage A3 (template instance; plan `let-s-plan-a-comprehensive-calm-dijkstra.md`)
**Result of record:** `docs/theory/10_eft_program/archive/closed_negative/ANALYSIS_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md` (FTD-0208 v3). This note **re-states** that closed negative as a conservation law; it does not amend, re-open, or extend its verdict. Per the template-degeneration gate declared in the program plan: the content beyond v3 is the invariant *formulation* (conserved quantity + wholesale SO(3) exclusion), not a new impossibility — hence [SYNTHESIS] with a [DERIVED — formalization] core, and **no new LEDGER id** (maintenance-log line only).

---

## 1. The object: the budget-combination closure **B**

FTD-0208 v3 inventoried the Scale-0 primitives available for combining the motion budget (speed fraction v) with the clock budget (dτ/dt): coordinate magnitudes, addition, binary max, and positive rational scaling. Define

> **B** = the closure of { |x₁|, |x₂|, |x₃| } (and the budget components built from them) under { +, max, ℚ⁺· }.

**Scope guard.** B is *not* the frozen native closure N of `PREREG_DELTA_IND_CLOSURE_DEFINITION_v1.md` (D1–D4); it is a separate, small, self-contained structure — v3's primitive inventory abstracted. No claim in this note transfers to N, and nothing here touches the frozen verdict map. (Same discipline as the grade-closure note `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` §1.)

Every generator is an O_h-invariant piecewise-ℚ-linear (polyhedral) form, and each operation preserves that property (verifier H5: exact local linearity on a declared cell; O_h-invariance of representatives under the group generators in H3). Hence:

> **Bookkeeping lemma.** Every member of B is an O_h-invariant piecewise-ℚ-linear form. *Polyhedrality is conserved by native budget combination.* [DERIVED — schema-level; H3 + H5]

## 2. The conserved invariant, in working form

A polyhedral norm's unit ball is a polytope; in dimension 3 its boundary contains 2-faces, so it always admits **non-parallel additive-equality pairs**: distinct u ∦ v with ‖u+v‖ = ‖u‖ + ‖v‖ (two points of one face). Verified witnesses (H2, exact): L¹ with u=(1,0,0), v=(0,1,0); L∞ with u=(1,1,0), v=(1,−1,0); and the native budget law v + dτ/dt ≤ 1 itself, which is additive outright. Equivalently: **no member of B is strictly convex.**

The Euclidean form is the opposite case. The Lagrange identity
(u·u)(v·v) − (u·v)² = |u×v|² (H1, symbolic) forces equality in the triangle inequality **only** for parallel vectors: L² is strictly convex. A remark strengthening this from "not globally equal" to "not even locally reachable": r = √(x²+y²+z²) has Hessian (I − x̂x̂ᵀ)/r of rank 2 everywhere off the origin, so it is nowhere locally linear — a piecewise-linear form cannot agree with it on any open cone. [THEOREM — classical, one-line proof stated]

## 3. The wall

> **Theorem (L² wall, conservation form).** No member of B is a strictly convex form; in particular c·L² ∉ B for any c > 0. Moreover, since a norm invariant under all of SO(3) is necessarily a multiple of the Euclidean norm (radial reduction + homogeneity; the forcing rotation is exhibited exactly in H4: an explicit R ∈ SO(3), RᵀR = I, det R = 1, mapping e₁ to (1,1,1)/√3), **B contains no SO(3)-invariant form at all**. [DERIVED — formalization of FTD-0208 v3; H1–H5]

The Pythagorean clock budget (dτ/dt)² + v² = 1 *is* an SO(3)-invariant (strictly convex) budget form. So it is unreachable from the Scale-0 primitives — which is exactly v3's closed-negative, now stated as a conservation law:

> **The substrate conserves polyhedrality; sphericity cannot be manufactured.** The quadratic budget is an *import* — the clock hypothesis (`SPEC_FTD_LAGRANGIAN.md` §4.3; Arc B P2 prereg `preregister-clock-hypothesis-derivation-v1`) is the flagged interpretive step that imports it, and this note prices what the import buys: strict convexity / SO(3), the properties the native closure provably lacks.

O_h-invariance is *not* the obstruction — H3 shows L¹ is fully O_h-invariant while assigning 1 vs √3 to two vectors of equal Euclidean length. The obstruction is finer: O_h admits polyhedral invariants; SO(3) admits only the sphere.

## 4. The conserved-charge table, extended

With this note the boundary program's conservation-law inventory (see `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` §5, `THEOREM_RAMIFICATION_LOCUS.md`) reads:

| # | conserved charge | closure it guards | the wall it prices | source |
|---|---|---|---|---|
| 1 | finite-horizon algebraicity | N (frozen D1–D4) | transcendence needs a limit schema | Lemma 0, `FOUND_FINITE_HORIZON_ALGEBRAICITY.md` |
| 2 | (4t−1)-square-class parity / ramification locus | N | δ and the whole √(affine-composite) family | FTD-0369 / FTD-0370 |
| 3 | dimension grade 0 | N + spec rules | a_phys, K_B calibrations | `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` |
| 4 | **polyhedrality** (this note) | **B** (budget sector) | the L²/SO(3) clock budget (clock hypothesis) | FTD-0208 v3, recast here |

Charge 4 differs from 1–3 in domain: it guards the *budget/metric sector*, not the constant sector — the table entry records that the two closures are distinct structures.

## 5. Falsifiers

- **F1 (inventory gap — the real risk).** The conservation claim is exactly as strong as v3's primitive inventory. A native Scale-0 operation *outside* { +, max, ℚ⁺· } that produces curvature — e.g. a spec rule that effectively computes a quadratic combination of budget components — would break conservation. Any future spec/toggle audit that finds such an operation falsifies this note (and re-opens v3).
- **F2 (formal gap).** An O_h-invariant strictly convex form constructed from the declared generators would contradict the bookkeeping lemma; exhibiting one falsifies the formalization directly.

## 6. Cross-references

FTD-0208 v3 (result of record) · `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` (A2 template sibling) · `THEOREM_RAMIFICATION_LOCUS.md` (FTD-0370 flagship) · `FOUND_FINITE_HORIZON_ALGEBRAICITY.md` (Lemma 0) · `SPEC_ALPHA_READOUT_CONTRACT.md` §2.5 (ramification checkpoint) · `DERIV_NEWTON_FROM_SUBSTRATE.md` + `SPEC_FTD_LAGRANGIAN.md` §4.3 (the clock-hypothesis import this wall prices) · `FOUND_MODULUS_ARGUMENT_FRONTIER.md` (FTD-0336: forced/modulus vs chosen/argument — the quadratic budget is an argument-side import).

**Standing invariants:** x₊ = 1/α stays [SMC]; MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; FC-W stays [AXIOM]; no tag in any cited document moves.
