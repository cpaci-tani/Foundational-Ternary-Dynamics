# AUDIT — Algebraic Spine Deep Adversarial Audit (2026-06-24)

**Tag:** [AUDIT FINDING / TAG-HONESTY]
**Date:** 2026-06-24
**LEDGER row:** FTD-0318
**Scope:** the FTD algebraic spine (the theorem-grade pure-math core, `SPEC_ALGEBRAIC_SPINE.md` Theorems 1–9 + subsidiaries), independent of any physics interpretation.
**Outcome:** demotion-only corrections across 15 files. **ZERO promotions.** `x₊=1/α` stays `[SMC]` (FTD-0013); MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; FC-W stays an adopted `[AXIOM]` (FTD-0315). Golden gate untouched (docs + Python only).

This document is the provenance anchor that every inline `corrected 2026-06-24 spine audit` note in the corpus refers to.

---

## 1. Method

A multi-agent adversarial audit (extract → verify → adversarial → synthesis) that, for every load-bearing spine claim:

1. **Re-ran the actual proof script** and recorded its real output (not the docs' asserted "PASS").
2. **Independently recomputed every identity** this session with `mpmath` at dps = 60–150 (never recalled a value).
3. **Ran perspective-diverse hostile lenses** on each of the 8 load-bearing theorems (number-theory rigor, transcendence/Galois, statistical methodology, epistemic-tag honesty), defaulting to "refuted" under uncertainty.

All 12 clusters were verified; all 11 extant proof scripts ran (several only under `PYTHONUTF8=1`).

## 2. What is genuinely, unconditionally theorem-grade (survived every lens)

All reproduce to machine zero (dps 60–150):

- **G\* = Γ(1/4)/Γ(3/4) = Γ(1/4)²/(√2·π)** = 2.95867511918863889… (elementary Γ-reflection). Distinct from ϖ = 2.62205755… (the FTD-0117 typo is absent everywhere checked).
- **Master quadratic** x² − 16G\*²x + 16G\*³ = 0, roots **x₊ = 137.036171458…** (+1.2572 ppm vs 1/α), **x₋ = 3.023963916…**; Vieta exact.
- **Watson equality** G\*²/(2π) = Γ(1/4)⁴/(4π³) = 2·G_gauss² = 1.39320392968…
- **Harmonic tower** 1/y₊ + 1/y₋ = 1 (symbolic, all k, multiplier-independent) + the discriminant factorization.
- **BCC triple-cosine** identity; **AGM bridge** G\* = 2·G_gauss·√π.
- **Coefficient 16 = |Aut(E)|²** as a group-order *value* (E: y²=x³−x, Aut = μ₄; script 14/14 PASS).
- **|μ|=|disc| arithmetic uniqueness of ℚ(i)**.
- **Deligne exponents:** k = 9 (512) for the sum, k = 13 (8192) for the product — the "2^10" alternative is wrong by exactly ×2 / ×8 and does not appear in the live corpus.

## 3. Corrections applied (demotion-only)

### CRITICAL — a numerically false `[THEOREM]`, corrected
- **`DERIV_LFUNCTION_GSTAR_CONNECTION.md` §3.1 + summary item 1.** The substitution `G* = 4√(2/π)·L` (= 2.0921, internally inconsistent with the same doc's correct §1.2 `G* = 8L/√π` = 2.95868) yielded wrong coefficients `512/π` and `2048√2/π^(3/2)`. Corrected to `1024/π` and `8192/π^(3/2)` (recomputed: 16G\*² = 140.0601353744945…, 16G\*³ = 414.3924377227094…, both machine-exact; the √2 was an artifact). Now consistent with `DERIV_MASTER_QUADRATIC_CM_LVALUES.md`.

### MAJOR — overclaimed tags reconciled to proofs
| Claim | Was | Now | Why |
|---|---|---|---|
| Theorem 9 "**maximal**/canonical π-free subfield" (`SPEC_ALGEBRAIC_SPINE.md` §9/§0/header) | THEOREM (maximal) | THEOREM (π-free, cond. Chudnovsky); "maximal" struck | No maximality proof exists; ℚ(Γ(1/4)) is a larger π-free subfield (false as stated). The doc's own "What it does NOT claim" already conceded it. |
| **D=3 forcing** (SPEC §10, LEDGER FTD-0010, `DERIV_D3_FROM_AUTOMORPHISM.md`) | THEOREM | arithmetic uniqueness THEOREM / **dimension-forcing [SELECTION]** | LHS \|Aut\|²=16 is D-independent; RHS uses \|O_h\|/3=48/3, presupposing D=3 (circular). |
| **CM Theorem 3** d=−4 (SPEC §3) | "mathematically proven" | arithmetic \|μ\|=\|disc\| THEOREM / physics dual-match **[NUMERICAL FACT]** | Flips under the rational-multiplier criterion: (d=−3,q=3) lands at +0.9077 ppm, tighter than canonical. |
| **Phase J §7 general-L** (SPEC §7, `proof_phase_j_general_L.py`) | "DISCONFIRMED for general L" | THEOREM L=2 + **NUMERICAL EVIDENCE L=3** (spread 8.9e-16, re-run) + **OPEN/ambiguous L≥4** | The "DISCONFIRMED" was an overclaim in the negative direction; L=3 is ultralocal; L≥4 is Gauss-zero-mode masked. |
| **z_BCC·2 = 16** (LEDGER FTD-0007, `FOUND_DIMENSIONAL_COUNTING.md`) | THEOREM | **[SELECTION]** | Re-spelling of 2⁴; \|Aut(E)\|²=16 remains the load-bearing THEOREM. |
| **Watson "I₁"** (`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`) | "1.3932 = Watson's integral I₁" | "= G_BCC(0)" (BCC return Green's function; SC Watson self-energy ≈ 0.5054) | Mislabel; the equality W₃ = G\*²/(2π) is unaffected. |
| **Motivic uniqueness** (`proof_motivic_master_quadratic.py`) | THEOREM | numeric identities THEOREM / forcing-uniqueness **[SELECTION]** | A constraint+selection argument, not a uniqueness proof. |
| **Harmonic-tower citation** (`THEOREM_HARMONIC_INVARIANT_TOWER.md`) | "Lindemann–Weierstrass via Schneider 1941" | **Chudnovsky 1976** (Waldschmidt 2000 §1.4); A_k transcendence k≥4 → [CONDITIONAL THEOREM given Chudnovsky] | Schneider 1941 alone is insufficient for G\* transcendence. |

### Provenance / hygiene
- Dangling `THEOREM_D_EQUALS_3.md` repointed → `DERIV_D3_FROM_AUTOMORPHISM.md` (SPEC, LEDGER, `FOUND_TERNARY_STATE_FROM_I.md`, `WHERE_WE_LEFT_OFF.md`).
- `constants.py` ALPHA_INV: inline `[CONJECTURE / post-hoc fit]` caveat (value unchanged).
- `CLAUDE.md` "two verified fast routes to G\*" → one (Landen present; Guillera artifact absent from main).

## 4. Conditionality map

- **Chudnovsky 1976** (alg. indep. of π and Γ(1/4)) is the keystone import: it gates ℚ(G\*) π-freeness (Thm 9), A_k transcendence (k≥4), and the entire W-narrowing theorem (FTD-0314). It is an *established* theorem, so "CONDITIONAL THEOREM" is legitimate — but the clause must ride along in every mention (it had been stripped in compressed cross-refs).
- **Damerell–Shimura / BSD** special values (`L(Sym²E,1)=ϖ²/(8π)`, `L(E,1)=ϖ/4`) are imported, not re-derived in-repo — the Deligne identities are `[DERIVED-given-import]`. No in-repo L-value verification exists yet (no PARI/Sage).
- **Trivial-multiplier criterion** gates the CM d=−4 physics privilege.
- **L=2-only scope** for Phase J ultralocality (L=3 numerically holds; L≥4 open).

## 5. Flagged for owner / follow-up (resolved in subsequent rows)

- **FTD-0189 ID collision** — the polynomial look-elsewhere scan (the framework's single live α-structural-evidence) is cited corpus-wide as "FTD-0189" but that LEDGER id is the graviton-provenance audit; the scan has no dedicated row → resolved by **FTD-0319**.
- The scan's **"~4×10⁵:1 Bayes" is unsupported** by its runner (~19× scan-size factor) and its uniqueness is asymmetric-tolerance-conditioned (symmetric 1% gate → 32 dual-matchers / 11 constants).
- **FTD-0097 hash-lock** cosmetically broken (CODATA-2022→2018 comment edit), but the χ² figures DO reproduce (470.26 / 38.09) — the audit's interim "368.5" claim was wrong (caught by forced recomputation).
- The **~125 unscanned `[PARAMETRIC]` claims** (the deepest open methodological question) → addressed by the pre-registered rigidity-catalog scan (FTD-0320).

## 6. Honest verdict

The spine's load-bearing math is real and the numerics are pristine, but the genuine unconditional-THEOREM set is smaller than the "nine theorems" headline and pervasively conditional on Chudnovsky 1976. The corpus's own discipline largely held — most overclaims were headline-above-proof drift over docs that already carried honest caveats — plus one genuine numerical error (now fixed). After this audit, every tag matches its proof.
