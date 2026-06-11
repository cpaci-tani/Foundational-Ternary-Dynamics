# Integer-4 Unification: The Lemniscatic Catalogue and the |μ| = |disc| Coincidence

**Status:** [THEOREM] for the classification + the |μ_K| = |disc(K)| uniqueness; the claim that the catalogue co-occurrences are "non-coincidental" is [DERIVED] (an explicit functorial-origin chain, not an independent theorem).
**Date:** 2026-05-19
**Companion:** G* opus follow-up Phase 1, lemma L6, target T-A2. Spec at `docs/superpowers/specs/2026-05-19-gstar-followup-attacks-design.md`.
**Verification:** `scripts/exploration/gstar_sym_k_eigenlines.py` (functions `unit_group_order_imag_quad`, `discriminant_imag_quad`, `classify_catalogue_4`) + `scripts/tests/test_gstar_sym_k_eigenlines.py` (4 tests: `test_mu_disc_coincidence_unique_to_Q_i` and 3 others). 39/39 tests pass.

---

## §1 — Motivation and the spec-draft error

The FTD lemniscatic sector exhibits a recurring integer 4 (and its square, 16) across multiple independent constructions — LEDGER entries FTD-0008, FTD-0110, FTD-0111, FTD-0122, FTD-0127, FTD-0128, Paper A §6, and Paper A §17. The G* opus follow-up spec drafted a target T-A2 claiming all these 4's are "instances of dim_ℤ(M) for a single rank-2 free ℤ[i]-module M ≅ ℤ[i]²". That draft statement is **incorrect** on two counts:

1. The draft catalogue entry "rank_ℤ(H¹(E_lemn)) = 4" is a factual error. H¹ of an elliptic curve (a genus-1 curve, topologically a 2-torus) has rank 2 over ℤ, equivalently rank 1 over ℤ[i]. The correct value is **2**, not 4.

2. The remaining 4's are NOT all dim_ℤ of one module — they are genuinely distinct arithmetic invariants. This document gives the correct classification.

---

## §2 — The corrected catalogue: three classes

Every "4" in the lemniscatic catalogue belongs to exactly one of three classes:

**Class (a) — unit-derived.**
|ℤ[i]^×| = |Aut_geom(E_lemn)| = |μ₄| = 4, where μ₄ is the group of 4th roots of unity. The associated 16 = |μ₄|² is the master-quadratic integer coefficient.

*Source:* ℚ(i) contains exactly the 4th roots of unity, and the geometric automorphism group of the CM elliptic curve E_lemn equals the unit group of its endomorphism ring (Aut(E_lemn) = ℤ[i]^× — a standard fact for CM elliptic curves). The four units {1, −1, i, −i} correspond exactly to the four geometric automorphisms.

**Class (b) — discriminant-derived.**
The conductor of the Kronecker character χ_{−4} equals |disc(ℚ(i))| = 4; the (1+i)-tower level 4 equals |(1+i)⁴| = |−4| = |disc(ℚ(i))|.

*Source:* The prime 2 ramifies in ℤ[i] as 2 = −i·(1+i)², giving disc(ℚ(i)) = −4. The conductor of the Kronecker character of an imaginary quadratic field equals the absolute value of its discriminant. So χ_{−4} has conductor 4 = |disc(ℚ(i))|.

**Class (c) — module-rank.**
dim_ℤ(V_complex) = dim_ℤ(ℤ[i]²) = 4 = 2·[ℚ(i):ℚ], where V_complex is the complex-structure-carrying sub-representation in the BCC decomposition (FTD-0122).

*Source:* V_complex is a rank-2 module over ℤ[i], and ℤ[i] is rank 2 over ℤ, so dim_ℤ = 2·2 = 4. **Note:** this class-(c) "4" is GENERIC to every imaginary quadratic CM field — it equals 4 for ℚ(ρ) as well — and is therefore not part of the ℚ(i)-specific phenomenon.

---

## §3 — The load-bearing structural fact: |μ_K| = |disc(K)| characterises ℚ(i)

**THEOREM (uniqueness of the lemniscatic coincidence).** Among all imaginary quadratic fields K = ℚ(√−d) (d a positive squarefree integer), the field K = ℚ(i) (d = 1) is the unique one satisfying

    |μ_K| = |disc(K)|.

For ℚ(i): |μ_K| = 4 and |disc(K)| = 4. For every other imaginary quadratic field, |μ_K| ≠ |disc(K)|.

**PROOF.**

The unit group order is determined classically:
- |μ_K| = 4 if d = 1 (K = ℚ(i)),
- |μ_K| = 6 if d = 3 (K = ℚ(ρ), the Eisenstein field),
- |μ_K| = 2 for all other squarefree d ≥ 2.

The discriminant satisfies:
- |disc(K)| = 4d if d ≡ 1, 2 (mod 4),
- |disc(K)| = d   if d ≡ 3 (mod 4).

In particular |disc(K)| ≥ 3 for every imaginary quadratic field, with |disc(K)| = 3 only for d = 3, and |disc(K)| = 4 only for d = 1.

We check each case of |μ_K|:

*Case |μ_K| = 2 (all d ≥ 2 except d = 3):* The coincidence requires |disc(K)| = 2. But |disc(K)| ≥ 3, so this is impossible.

*Case |μ_K| = 4 (d = 1, K = ℚ(i)):* |disc(ℚ(i))| = 4·1 = 4 = |μ_K|. ✓ Coincidence holds.

*Case |μ_K| = 6 (d = 3, K = ℚ(ρ)):* |disc(ℚ(ρ))| = 3 ≠ 6.

Hence d = 1 is the unique solution. ∎

*(Numerically verified for all squarefree d ∈ [1, 200] by `test_mu_disc_coincidence_unique_to_Q_i`.)*

---

## §4 — The unification statement (corrected T-A2)

**COROLLARY (Integer-4 unification, corrected T-A2).** Every "4" in the lemniscatic catalogue is a functorial invariant of the number field ℚ(i):

- Classes (a) and (b) are the two fundamental arithmetic invariants |μ_K| and |disc(K)| of K = ℚ(i).
- These two invariants **coincide** (both equal 4) precisely because K = ℚ(i) is the unique imaginary quadratic field with |μ_K| = |disc(K)| (§3).
- Class (c) is the generic module-rank 2·[K:ℚ] = 4, common to all imaginary quadratic CM fields and not specific to ℚ(i).

The lemniscatic catalogue therefore shows a UNIFORM 4 (rather than a mix of integers) precisely because ℚ(i) is the field where the unit-order and discriminant coincide. The 16 = |μ₄|² is the squared unit-order.

---

## §5 — What this does and does not establish

**It DOES:**
- Give a correct classification of every catalogue 4 into exactly one of three classes.
- Prove the |μ_K| = |disc(K)| uniqueness of ℚ(i) as an imaginary quadratic field — the theorem that explains WHY the lemniscatic sector exhibits a uniform 4.
- Correct the spec-draft entry: rank_ℤ(H¹(E_lemn)) = 4 is wrong; the true value is **2** (rank 2 over ℤ, rank 1 over ℤ[i]).

**It does NOT:**
- Claim all 4's are "the same 4" in a single-functor sense — the spec-draft over-claim. Class (c) is a genuinely different (and generic) 4 that merely also equals 4.
- Provide a new derivation of α = 1/x_+; the empirical identification remains [STRONGLY MOTIVATED CONJECTURE] as classified in TRACKER_ONTIC_TRUTH.md.

---

## §6 — Cross-references

| Entry | Content |
|---|---|
| FTD-0008 | Moore-neighbourhood integers; N_base = 4 first appears |
| FTD-0110 | Clustermass identification; N_base from O_h A_{1g} multiplicity |
| FTD-0111 | (1+i)-tower harmonic invariant; level k = 4 |
| FTD-0122 | BCC complex-structure theorem; V_complex ≅ ℤ[i]² |
| FTD-0127 | G* parity-twist; χ_{−4} conductor 4 |
| FTD-0128 | Ternary state from i²; |ℤ[i]^×| = 4 |
| Paper A §6 | Master quadratic, coefficient 16 |
| Paper A §17 | χ_{−4} four-level unification |
| LEDGER FTD-0181 | This theorem |
| `scripts/exploration/gstar_sym_k_eigenlines.py` | Verification functions |
| `scripts/tests/test_gstar_sym_k_eigenlines.py` | 4 tests; 39/39 pass |
