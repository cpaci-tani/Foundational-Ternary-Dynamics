# Realizability Lower Bounds for the Native Closure N — What the Substrate *Does* Reach

**Status:** [DERIVED — schema-level] (realizations) + [DERIVED — conditional on E1] (strict lower bound) + [SYNTHESIS] (the sandwich) · **Verifier:** `scripts/proofs/proof_b1_realizability.py` (6/6 PASS) · **Date:** 2026-07-05
**Program:** Clause-2/3 boundary program, stage **B1** — the last chartered stage (plan `let-s-plan-a-comprehensive-calm-dijkstra.md`). Program-internal under FTD-0368; no new LEDGER id (maintenance-log line).
**Frozen definition tested against:** `PREREG_DELTA_IND_CLOSURE_DEFINITION_v1.md` (lock `63e9c506`), **verbatim** — B1 is a membership computation against the *already-frozen* N_dyn (D1–D4) and N_calc, with the v1.1 D2 symbol scope {σ₁₈ default, BCC, 7-point SC} binding. No clause is amended; FCC stays a v2 symbol and is honestly excluded here.

---

## 1. The question: N from below

FTD-0369 proved δ = √(G\*(4G\*−1)) lies **outside** the native closure N (conditional on E0 + E\*), and FTD-0370 generalized that to a whole excluded √-family. Those are *upper* bounds on N's reach — statements of what it cannot manufacture. B1 is the mirror discipline: exhibit explicit **D1–D4-admissible schemas** whose admissible limits are **named constants inside N** — a *lower* bound. The two together bracket N, and the bracket is where the content is: N is not small, and knowing *how large* it is sharpens what δ's exclusion means.

"Realizable" here is used in the frozen definition's exact sense: c ∈ N_dyn iff there is a uniform (D1), linear-sector (D2, in-scope symbol), canonical-source (D3), polynomial-modulus (D4) native schema (a_L) with a_L → c. Certification is structural admissibility of the schema + the existence of the classical Watson/Green limit; the numerics (below) are the D4 **rate witness**, [EXTERNAL]-tagged exactly as the S2 anchors are.

## 2. The realizability ledger

Each schema is the origin value of the zero-mode-excluded lattice Green's function for a frozen-scope symbol, G^X_L(0) = (1/L³) Σ′_k 1/σ_X(k). All three verified (`proof_b1_realizability.py`):

| # | target constant | symbol σ_X(k) | D1–D4 schema | limit value | value class | tag |
|---|---|---|---|---|---|---|
| R1 | G\*²/(2π) ≈ 1.393204 | BCC: 1 − ∏cos | odd-L ladder, unit source (S2 anchor, re-verified) | 1/L-extrapolant 1.393206 (err 2×10⁻⁶) | **∈ ℚ(G\*,π)** (no field extension) | [DERIVED — schema] |
| R2 | W_S/2 ≈ 0.252731 | SC: 6 − 2Σcos | all-L, unit source | 1/L-extrapolant 0.252731 (err 5×10⁻⁷) | **Γ(1/24)-class** — *outside* ℚ(G\*,π) [Glasser–Zucker] | [DERIVED — schema] |
| R3 | W₁₈ ≈ 1.2679 | σ₁₈: 1 − (1/6)Σcos − (1/6)Σcos·cos (**the engine default**) | all-L, unit source | 1/L-extrapolant 1.267936 (err 4×10⁻⁵ vs AUDIT_LINK8) | **period of an IRREDUCIBLE order-4, degree-12 ODE** (FTD-0372; Sage/ore_algebra: no factor, not a symmetric cube ⇒ **NOT a classical elliptic Γ-quotient** — a genuinely new order-4 period, outside ℚ(G\*,π)) | [DERIVED — schema] |
| R4 | π | — | N_calc base generator; **and** cross-route: G\*²/(2π) ∈ N_dyn ∧ G\* ∈ N ⇒ π ∈ N | exact | ∈ ℚ(G\*,π) | [DERIVED] |

**Finite-L algebraicity (Lemma 0 witness).** At L=3 every schema value is an *exact rational* — G^BCC_3(0) = 244/243, G^SC_3(0) = 44/243, G^σ₁₈_3(0) = 232/243 — confirming Lemma 0's "finite dynamics is transcendence-inert" for the full B1 symbol set; the transcendence enters only at the D4 limit, exactly where the definition puts it.

**Why R2 is load-bearing.** W_S is the classical simple-cubic Watson integral, evaluated by Glasser–Zucker in the Γ(1/24)-class — a period of a *different* CM field than G\* (Γ(1/4)-class). It is realized here *unconditionally* (the SC schema is D1–D4-admissible and in the frozen D2 scope, and its limit is Watson's theorem). Its being *outside* ℚ(G\*,π) is the separate open statement E1 — see §3.

**Why R3 matters.** The σ₁₈ symbol is the spec's **own default** linear operator (`AUDIT_LINK8_CLOSURE.md` §2). Its Green's constant W₁₈ is native by construction — a member of N_dyn — and its arithmetic is now **fully charted**: W₁₈ is the period of an explicit **order-4, degree-12** ODE (FTD-0372, `EXPLR_STENCIL_SPECTRUM.md` §2) that is **irreducible over ℚ̄(z)** (Sage/ore_algebra) and **not a symmetric power** of a second-order operator (exact exponent obstruction). So — unlike the sibling SC/FCC/BCC lattice constants, which are all classical CM Γ-classes — **W₁₈ does not reduce to a classical elliptic Γ-quotient; it is a genuinely new order-4 period.** N therefore contains a member whose arithmetic is not the substrate's "own" G\*-class *nor* a classical Watson Γ-class, but a new higher period. That is the honest shape of the boundary from the inside: the default stencil's constant is arithmetically novel — the substrate's isotropized mixture manufactures something the pure lattices do not.

## 3. The lower bound

**Unconditional.** N is a field containing N_calc's base ℚ(G\*,π) and the three realized limits:

> **N ⊇ ℚ(G\*, π, W_S, W₁₈).** [DERIVED — schema-level; R1–R4, field closure]

**Conditional (strictness).** The realized SC value W_S is Γ(1/24)-class. The statement **W_S ∉ ℚ(G\*, π)** — that the simple-cubic Watson period is not a rational function of the lemniscatic G\* and π — is a transcendence statement believed true and open (it is exactly the weak form of **E1**, the SC/FCC-independence assumption on which FTD-0369's conditionality rests). Under it:

> **N ⊋ ℚ(G\*, π), conditional on E1.** [DERIVED — conditional on E1]

The point of tagging it this way: the *same* E1 that keeps FTD-0369 from being unconditional is what certifies N is strictly larger than its base field. B1 and FTD-0369 are two consequences of one open independence.

**The sandwich [SYNTHESIS].** Combining the three registered results:

> ℚ(G\*, π) ⊊ N (mod E1, this note) — yet — δ ∉ N (mod E\*, FTD-0369), with the whole √(affine-composite) family excluded (FTD-0370).

So N is a **large period ring that specifically dodges the (4G\*−1) square class**. Its exclusion of δ is not the poverty of a small closure; it is the precise arithmetic selectivity of a rich one. This is FTD-0370's ramification-locus law seen from the membership side: N reaches the CM periods of the lattice symbols (BCC/SC, and the uncharted σ₁₈), and ramifies only over its own coordinate places — so a surd branched at 4G\*−1, off those places, stays out no matter how much period content N accumulates.

## 4. Honest non-realizations (Tier C — what B1 does *not* place inside N)

- **δ = √(G\*(4G\*−1)).** Conjecturally ∉ N (FTD-0369). B1 does **not** attempt to realize it — exhibiting a schema whose limit generates δ is precisely S3's REFUTED branch, which is banned to fish for (prereg B1/B5) and would be an outcome-symmetric revolution requiring the FTD-0314 §4 forced-vs-hand-placed adjudication, not a B1 construction.
- **W_FCC (Γ(1/3)-class).** The FCC symbol is **outside the frozen D2 scope** (v1.1 note: σ₁₈ + BCC + 7-point SC only). W_FCC is realizable only under a v2 scope enlargement; claiming it in N under v1 would be the exact scope-drift the freeze exists to prevent. Recorded as v2.
- **Nonlinear-sector constants.** D2 is linear-sector only; any constant requiring rule-4/rule-6 thresholds is a v2 (properness-rung) target.

## 5. Falsifiers

- **F1 (realization error).** Any schema in the ledger whose limit is shown *not* to be its stated constant (a corrected Watson value, or a demonstration that the 1/L extrapolation masks a different limit) falsifies that row. The exact finite-L rationals + the classical limit theorems make this remote, but it is the operative check.
- **F2 (E1 collapse).** If W_S ∈ ℚ(G\*, π) were proven (E1 false in this weak form), the strict lower bound N ⊋ ℚ(G\*,π) would lose its only support *via this route* — and, symmetrically, FTD-0369's SC-sector conditionality would need re-adjudication. The two stand or fall together on E1.
- **F3 (scope drift).** Using FCC or any non-frozen symbol to inflate the lower bound voids the result under ban B2; the verifier asserts FCC is not used (R6).

## 6. Cross-references

`PREREG_DELTA_IND_CLOSURE_DEFINITION_v1.md` (the frozen N — D1–D4, N_calc, v1.1 scope) · `proof_s2_adequacy_anchors.py` (the BCC/Phase-G machinery reused) · `ANALYSIS_DELTA_IND_CLOSURE_v1.md` (FTD-0369 — the upper bound δ ∉ N, and E1) · `THEOREM_RAMIFICATION_LOCUS.md` (FTD-0370 — the ramification law this note mirrors from inside) · `EXPLR_STENCIL_SPECTRUM.md` (B0 — W₁₈'s holonomic-but-large status, the R3 constant) · `FOUND_FINITE_HORIZON_ALGEBRAICITY.md` (Lemma 0 — the finite-L rationals) · `REF_EXPORTED_PROBLEMS_E1_E2.md` (P1 = E1, the assumption this note's strictness rests on) · `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` + `FOUND_L2_CLOSURE_RECAST.md` (the sibling boundary-program conserved charges).

**Standing invariants:** x₊ = 1/α stays [SMC]; MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; FC-W stays [AXIOM]; no tag in any cited document moves. B1 introduces no conjecture and no new id — it is the positive-side formalization the program was chartered to deliver.
