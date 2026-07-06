# THEOREM — The ramification locus of the native closure: the substrate ramifies only where it lives

**Tag:** R1 (hull) **[THEOREM — conditional on Chudnovsky 1976 only]**; R2 (full N) **[THEOREM — conditional on E0 + E\*\*]** with E\*\* an open, named, uniform assumption strengthening FTD-0369's amended E\*; framing readings [coherent-interpretation].
**LEDGER id:** FTD-0370 (the Clause-2/3 flagship, A4 = B2).
**Verification:** `scripts/proofs/proof_ramification_locus.py` (7/7; the divisor and radicand lists of §3 are declared here and fixed before the sweep — exclusions only, nothing searched for).
**Audience:** the boundary program; anyone asking "is the δ-wall special, or one of many?"

---

## §1 — Statement

Work in the E0 model (Chudnovsky 1976): t = G\*, u = π independent transcendentals over ℚ̄; places of the t-line are t = c (c ∈ ℚ̄) and t = ∞, with u a t-unit everywhere.

> **R1 (the hull ramifies only at its coordinates).** The hull Ñ = ℚ̄(s, w), s² = t, w⁴ = u, is a Kummer extension whose radicand divisors are supported on the coordinate places — div_t(t) = {0: +1, ∞: −1}, div_t(u) = ∅ — so **Ram_t(Ñ) = {0, ∞}**. Consequently, for every c ≠ 0 the valuation v_{t−c} extends to Ñ with value group ℤ, and **√f ∉ Ñ for every f ∈ ℚ̄(t, u) with odd v_{t−c}(f) at any non-coordinate place** — the *entire* √(affine-composite) family is excluded at once: √(4t−1), √(t+1), √(2t−1), √(t²+1) (places ±i ∈ ℚ̄), and δ = √(t(4t−1)) among them. [THEOREM — E0 only; pure Kummer bookkeeping, verified R1a/R1b]

> **R2 (the same locus for all of N, conditionally).** Let **E\*\*** (uniform unramifiedness) be: *the compositum ⟨N_calc, Frac(𝕍\*)⟩ is unramified over every place t = c, c ≠ 0.* Under E0 + E\*\*: **Ram_t(N) ⊆ {0, ∞}**, and the whole exclusion family of R1 transfers to N — δ and every √(affine-composite) lie outside N and outside every √-unit tower over it. E\*\* is the family-quantified *uniform* strengthening of FTD-0369's amended E\* (which is exactly the c = 1/4 slice); per-target minimal assumptions E\*(c) are available for any single family member. R2 inherits every A0-audit amendment of FTD-0369 (the E0 + E\* package discipline; the m=1 BCC restriction; the suspended retirement). [THEOREM — conditional on E0 + E\*\*]

## §2 — What this changes

**δ is de-specialized.** Before: the α-wall was one theorem about one surd. Now: the hull provably ramifies *only over its own coordinate monomials* — the places where the natives' roots (√G\*, π^{1/4}) actually live — and every branch-point off the coordinate axes is unreachable. The verified family already contains places {1/4, 1/2, −1, ±i}: the α-wall (c = 1/4) is the physically-pointed instance of a **coordinate-ramification law**, not an isolated obstruction. In the goal's vocabulary: clause 2's boundary is no longer a wall but a *locus* — "the substrate ramifies only where it lives," and every import of argument-type √-data is now indexed by the place it would have to ramify. [coherent-interpretation for the slogan; the mathematics is §1]

**The three charges gain a geometric parent.** The (4t−1)-parity charge of FTD-0369 is the c = 1/4 slice of R1/R2; the conserved-quantity table of `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` §3 should be read with this note as the parity row's general form.

## §3 — The declared lists (fixed before the sweep)

Radicands: {t(4t−1), 4t−1, t+1, 2t−1, (t+1)(4t−1), t²+1, t³(4t−1)}. Hull rows: the 13 documented monomials of FTD-0353 §2.2. The verifier computes each radicand's square-free part, its non-coordinate odd-valuation places, and the hull rows' unit status at every such place. Nothing outside these lists is claimed; extending the lists is a doc edit that precedes any re-run.

## §4 — Honesty ledger

- R1 is unconditional beyond E0 and **purely field-theoretic**: it makes no dynamical claim and is independent of the D2-scope and m=1 questions (it is a statement about Ñ, whose membership facts are FTD-0353's).
- R2's E\*\* is **strictly stronger** than the S3 package's E\* and is open; its negation at any place c is a place-indexed generalization of the FTD-0353 §8 shared falsifier (a native, forced output whose square-class carries (t−c)). One falsifier schema now covers the whole family.
- Nothing here re-awards the suspended BCC-sector retirement, touches x₊ = 1/α [SMC], or closes MC-T4.3 — the locus theorem sharpens the wall's geometry; the two doors of Lemma 0 (limits, assignments) remain the only entrances.
- **Falsifier (R1):** an element of Ñ whose square is a ℚ̄(t,u)-function with odd valuation off the coordinates (would contradict Kummer theory — i.e., an arithmetic error here, checkable). **Falsifier (R2):** the place-indexed E\*\* negation above.

## §5 — Cross-references

`ANALYSIS_DELTA_IND_CLOSURE_v1.md` (FTD-0369, as amended — the c = 1/4 instance and the E\*-discipline R2 extends); `THEOREM_VALUATION_4GSTAR_MINUS_1.md` (FTD-0353 — the hull membership facts + §8 falsifier); `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` §3 (the conserved-charge table this generalizes); `FOUND_SQUARE_ROOT_AS_ACT.md` (FTD-0340 — what a branch-choice is); `scripts/proofs/proof_ramification_locus.py`.
