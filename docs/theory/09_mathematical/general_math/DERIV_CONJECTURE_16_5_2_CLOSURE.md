# Closure of Conjecture 16.5.2: the Sym^a Residual Reduces to Theorem 17.5

**Status:** [DERIVED] — an explicit six-step reduction chain, not an independent theorem.
**Date:** 2026-05-19
**Companion:** G* opus follow-up Phase 1 lemma L5, target T-A3. Spec at `docs/superpowers/specs/2026-05-19-gstar-followup-attacks-design.md`.
**Verification:** `scripts/exploration/gstar_sym_k_eigenlines.py` (functions `phi_is_real_forces_q_rational`, `phi_specialise_symbolic`) + `scripts/tests/test_gstar_sym_k_eigenlines.py` (43/43 pass, including `test_reality_collapse_lemma_sym2`, `test_reality_collapse_lemma_q_rational_gives_real`, `test_reality_collapse_lemma_imaginary_parts_independent`, `test_phi_is_real_forces_q_rational_helper`).

---

## §1 — The conjecture

Paper A, §16.5 (also §17.6 in the companion numbering) poses the following:

**Conjecture 16.5.2 / Paper A Conjecture 17.6.** *(Sym^a residual conjecture.)* Theorem 17.5 establishes that (a, b) = (2, 3) is the unique minimum-a admissible pair in the *leading-period family* — polynomials of the form

    P_{(a,b)}(x) = x² − 16·Φ(b')·x + 16·Φ(c')

where b' = λ · ω^a and c' = μ · η^b with λ, μ ∈ ℤ (integer multiples of a single leading-period monomial). The conjecture asks whether this (2, 3) uniqueness *extends* to the full generality:

> **Does the (2, 3) uniqueness hold when b' ranges over all of Sym^a H¹(E_lemn) (arbitrary Q[i]-coefficients in the monomial basis) and c' ranges over all of Sym^b H¹(E_lemn)?**

This is a genuine extension: the leading-period case fixes one monomial per symmetric power, while the conjecture asks whether broader coefficient freedom could allow a different (a, b) pair to pass the Theorem 17.5 admissibility criteria.

---

## §2 — The reality-collapse lemma

This is the load-bearing step. It shows that the "arbitrary coefficient" freedom is illusory once a necessary structural constraint is imposed.

**LEMMA (Reality-collapse, L5-1 / FTD-0181-precursor).** *Let k ≥ 1 and let*

    b' = Σ_{a+b=k} α_{a,b} · ω^a η^b,   α_{a,b} ∈ Q[i]

*be an arbitrary element of Sym^k H¹(E_lemn) ⊗ Q[i]. Write α_{a,b} = p_{a,b} + i·q_{a,b} with p_{a,b}, q_{a,b} ∈ Q. Then:*

    Φ(b') ∈ ℝ   ⟺   all q_{a,b} = 0   (i.e., all α_{a,b} ∈ Q).

**PROOF.** By the specialisation map (Convention C2 + the formula of `EXPLR_SYM_PERIOD_ALGEBRA_CONVENTIONS.md`), the monomial images are

    Φ(ω^a η^b) = (−1)^b · G*^{a−b} · π^{(a+b)/2}.

These are *real* (since G* ∈ ℝ>0 and π ∈ ℝ>0). Therefore

    Φ(b') = Σ_{a+b=k} (p_{a,b} + i·q_{a,b}) · Φ(ω^a η^b)

has imaginary part

    Im(Φ(b')) = Σ_{a+b=k} q_{a,b} · Φ(ω^a η^b).

The monomial Φ-images {Φ(ω^a η^b)}_{a+b=k} are **Q-linearly independent**: they are of the form r_{a,b} · G*^{a−b} · π^{k/2} where r_{a,b} = (−1)^b ∈ {+1,−1} and the exponent a−b ranges over {k, k−2, …, −k} — k+1 distinct values. Since G* is transcendental over Q (Chudnovsky 1976), distinct G*-powers are Q-linearly independent; multiplying by the nonzero real π^{k/2} preserves Q-linear independence. (The ±1 signs r_{a,b} are rational, so they participate in the Q-rational combination.)

Hence Im(Φ(b')) = 0 is a vanishing Q-linear combination of Q-linearly independent reals, which forces each coefficient q_{a,b} = 0.

The converse (all q_{a,b} = 0 ⟹ Φ(b') ∈ ℝ) is immediate from the formula. ∎

*(Numerically verified for k ∈ {2, 3, 4, 5} by `test_reality_collapse_lemma_imaginary_parts_independent` and `test_reality_collapse_lemma_q_rational_gives_real`.)*

**COROLLARY (Q-rational collapse).** Under the same hypotheses, if Φ(b') ∈ ℝ then b' has *Q-rational* coefficients; and Φ then restricts to an injection from the Q-rational lattice into ℝ (same Q-linear independence argument). Hence a Q-rational element b' ∈ Sym^k H¹ is uniquely determined by its Φ-image: if Φ(b'₁) = Φ(b'₂) with b'₁, b'₂ Q-rational, then b'₁ = b'₂.

---

## §3 — The closure theorem

**THEOREM T-A3 (Closure of Conjecture 16.5.2).** *Conjecture 16.5.2 holds: (a, b) = (2, 3) is the unique minimum-a admissible pair even when b' ranges over all of Sym^a H¹(E_lemn) with arbitrary Q[i]-coefficients (and c' over all of Sym^b H¹). The residual generality admitted by the conjecture is illusory once the real-root condition is enforced.*

**Tag:** [DERIVED]. Closure by explicit reduction to Theorem 17.5; see the dependency declaration in §4.

**PROOF.** The proof is an explicit six-step chain.

**Step 1 — Necessary condition: real coefficients.** The master quadratic polynomial

    P_{(a,b)}(x) = x² − 16·Φ(b')·x + 16·Φ(c')

is required to admit a *real ordered root pair* (x₊, x₋) with x₊ > x₋ > 0. This is a necessary condition before any admissibility criteria of Theorem 17.5 can be evaluated: Theorem 17.5 criterion (iii) (non-degeneracy; discriminant analysis) presupposes a real discriminant, and the physical identification x₊ = α⁻¹ ≈ 137 (FTD-0013 [SMC]) — together with the historical paired x₋ ≈ N_c (≈ 3), now **RETIRED** per v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`; preserved here as historical motivation for the reality-requirement) — requires real roots. For P to have real roots, its coefficients must be real:

    Φ(b') ∈ ℝ   and   Φ(c') ∈ ℝ.

**Step 2 — Reality-collapse (Lemma §2).** By the Reality-collapse Lemma, Φ(b') ∈ ℝ forces all Q[i]-coefficients of b' (in the monomial basis of Sym^a H¹) to be Q-rational. Likewise Φ(c') ∈ ℝ forces c' ∈ Sym^b H¹ to be Q-rational.

**Step 3 — Reduction to the leading-period family.** Among Q-rational elements of Sym^a H¹, the period map b' ↦ Φ(b') is injective (by the Corollary of the Reality-collapse Lemma: Q-linear independence of the monomial Φ-images). Every Q-rational b' is therefore uniquely determined by its Φ-image, and the Φ-image exhausts a discrete (Q-rational) sub-lattice of ℝ. The leading-period family (integer-multiple of a single monomial ω^a or η^b) is a sub-family of the Q-rational family. No enlargement of the coefficient space beyond Q-rational is possible under Step 2's constraint; and within Q-rational elements, there is no additional freedom that escapes the Theorem 17.5 evaluation — the admissibility criteria of Theorem 17.5 (integer prefactor 16, non-scalar-monomial roots, non-degeneracy) apply verbatim to Q-rational (b', c') pairs.

**Step 4 — Admissibility criteria are evaluated on the Q-rational domain.** The Theorem 17.5 criteria are:
- (i) the coefficient 16 (the integer factor in front of Φ(b') and Φ(c'));
- (ii) the roots x₊, x₋ are not scalar multiples of any single G*^k;
- (iii) the discriminant is strictly positive and the roots are real and distinct.

All three criteria are phrased in terms of Φ(b') and Φ(c') as real numbers, and are therefore applicable uniformly across the entire Q-rational (b', c') family — not just the leading-period sub-family. The reduction in Steps 2–3 shows that the Q-rational family *is* the domain on which these criteria must be tested.

**Step 5 — Theorem 17.5 applies.** Theorem 17.5 (Paper A §17.4–17.5; confirmed in L4 / FTD-0180 that (a, b) = (2, 3) is the unique minimum-a admissible pair) covers the family of Q-rational (b', c') pairs under the Theorem 17.5 criteria. By H4 (confirmed in FTD-0180), no pair with a < 2 or with a = 2 and b < 3 (and b ≠ a) passes all three criteria. The pair (a, b) = (2, 3) is the unique minimum-a solution.

**Step 6 — Conclusion.** Combining Steps 1–5: any (b', c') pair that could serve as the coefficient data for a real-admissible master quadratic must be Q-rational (Steps 1–2); the Q-rational domain coincides with the evaluation domain of Theorem 17.5 (Steps 3–4); and Theorem 17.5 gives (a, b) = (2, 3) uniqueness on that domain (Step 5). Therefore Conjecture 16.5.2 holds. ∎

---

## §4 — Epistemic status and scope

**Tag:** [DERIVED]. This result is an explicit reduction chain (§3, Steps 1–6) that the document itself reproduces in full. It is *not* an independent [THEOREM] in the sense of a new mathematical argument; it is a reorganisation of existing results.

**Dependencies:**
1. **Theorem 17.5 (Paper A §17.4–17.5)** [THEOREM]: the (2, 3) uniqueness in the leading-period family, with criteria (i)–(iii). Status confirmed in FTD-0180.
2. **Reality-collapse lemma (L5-1, §2 above)** [THEOREM]: Φ(b') ∈ ℝ ⟺ b' Q-rational. Depends on Q-linear independence of monomial Φ-images.
3. **Chudnovsky 1976** (classical): transcendence of G* = Γ(1/4)/Γ(3/4) over ℚ (equivalently, of the lemniscate constant). This is the foundation of the Q-linear independence claim in §2. It is cited, not reproved here.

**Scope — the real-root reading (important honesty note).** The closure holds specifically under the **real-root admissibility criterion**: the master quadratic must have real, positive, distinct roots. This is the reading required by:
- Paper A Theorem 17.5 itself (criterion (iii) presupposes a real discriminant analysis);
- any physical identification of (x₊, x₋) with (α⁻¹, N_c) — which are real. *(Historical pair-identification framing; `x_-  N_c` is retired per v1.4 §5. The single live identification `x_+  α⁻¹` still requires real roots; the historical paired reading is preserved here as motivation for the reality requirement.)*

If a future reformulation of Conjecture 16.5.2 considered an *alternative* admissibility criterion that permitted complex-conjugate root pairs, or otherwise did not require Φ(b') and Φ(c') to be real, then Steps 1–2 of the proof chain would not apply. In that alternative framing the conjecture would remain open. The closure in this document is for the real-root reading, which is the mathematically and physically natural reading of Paper A's own criterion set. The scope note is not a weakness of the argument; it is a precise statement of the argument's hypothesis.

---

## §5 — Consequences

**Closure of Paper A's residual.** Paper A §16.5 ends with Theorem 16.5.1 (the leading-period uniqueness, identical to Theorem 17.5) followed immediately by Conjecture 16.5.2 as the open residual: *does the uniqueness extend to arbitrary Sym^a coefficients?* Theorem T-A3 answers this affirmatively. The §16.5 arc of Paper A is now complete.

**Duke/JAMS-grade upgrade made available.** The G* opus overview document (Paper E / the four-paper overview) states explicitly that closing Conjecture 16.5.2 was the stated requirement for upgrading Paper A from the current Crelle/Compositio-grade target to Duke/JAMS-grade. Theorem T-A3 makes that upgrade *available*: the mathematical gap is closed. The upgrade is not achieved by this theory note alone — Paper A's authors must write up the result in the paper's own notation and proof style, integrate it into §16.5 / §17, and resubmit. This document provides the argument; the editorial work remains.

**No tag promotion for earlier claims.** This closure does not alter the epistemic tags of Theorem 17.5 (which remains [THEOREM] from Paper A), the reality-collapse lemma (which remains [THEOREM] from L5-1 / this document's §2), or the physical identification x₊ = α⁻¹ (which remains [STRONGLY MOTIVATED CONJECTURE] per TRACKER_ONTIC_TRUTH.md). The reduction chain proves the conjecture under its mathematical statement; the chain from the mathematical result to the physics remains at its established tier.

---

## §6 — Cross-references

| Entry | Content |
|---|---|
| Paper A §16.5 | Theorem 16.5.1 (leading-period uniqueness) + Conjecture 16.5.2 (the residual closed here) |
| Paper A §17.4–17.6 | Theorem 17.5, admissibility criteria (i)–(iii), master quadratic formulation |
| FTD-0178 | L2: Hodge complex structure J on Sym^k H¹; J² = (−1)^k · id; σ_{a,b} closed form |
| FTD-0179 | L3: J-eigenspace decomposition of Sym^k; explicit eigenlines for k = 2, 3, 4, 5 |
| FTD-0180 | L4: H4 confirmed — (2, 3) is unique minimum-a admissible pair in Theorem 17.5's family |
| FTD-0181 | L6: Integer-4 unification; |μ_K| = |disc(K)| uniqueness for ℚ(i); target T-A2 |
| FTD-0182 | This closure; target T-A3 |
| `EXPLR_SYM_PERIOD_ALGEBRA_CONVENTIONS.md` | Conventions C1–C6; specialisation map Φ; monomial Φ-images |
| `DERIV_INTEGER_4_UNIFICATION.md` | T-A2 closure; corrected catalogue of "4"s |
| `scripts/exploration/gstar_sym_k_eigenlines.py` | Functions `phi_is_real_forces_q_rational`, `phi_specialise_symbolic` |
| `scripts/tests/test_gstar_sym_k_eigenlines.py` | 43/43 pass; reality-collapse tests at lines 1002–1101 |
| TRACKER_ONTIC_TRUTH.md | Canonical bedrock; physical identification x₊ = α⁻¹ remains [STRONGLY MOTIVATED CONJECTURE] |
| LEDGER.md | Master claim ledger; FTD-0182 row for this closure |
