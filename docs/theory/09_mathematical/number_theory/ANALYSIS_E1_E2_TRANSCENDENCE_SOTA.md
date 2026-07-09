# ANALYSIS — E1 & E2: the transcendence state of the art and the precise price of the boundary

**Tag:** `[SYNTHESIS]` — a citation-verified state-of-the-art map for the two open independence assumptions (E1, E2) that the δ∉N boundary theorem imports, together with the verified closed form of the SC Watson constant and pre-registered numerics. **It solves neither problem** (no one can, without the named conjectures below); it *prices* them. Introduces no FTD theorem and **promotes no tag**.
**LEDGER id:** FTD-0376 · **Verifier:** [`scripts/proofs/proof_e1_sc_watson_gamma_class.py`](../../../../scripts/proofs/proof_e1_sc_watson_gamma_class.py) (SC closed form + the multiplicative-non-reducibility fact; exit 0).
**Sits with:** [`REF_EXPORTED_PROBLEMS_E1_E2.md`](REF_EXPORTED_PROBLEMS_E1_E2.md) (the FTD-free problem statements), the priced-import ledger ([`SPEC_IMPORT_LEDGER.md`](../../01_reference/SPEC_IMPORT_LEDGER.md), IMP-C3/C4), and the δ-independence verdict (FTD-0369 / [`THEOREM_RAMIFICATION_LOCUS.md`](../../02_foundations/THEOREM_RAMIFICATION_LOCUS.md), FTD-0370).

---

## 0 · The headline, priced

**δ∉N (FTD-0369/0370) rests on Chudnovsky 1976 (proven) + E1 + E2.** This note pins the price of E1 and E2 exactly:

> **E1 is a case of the Rohrlich–Lang conjecture / the Grothendieck period conjecture. E2 is a case of the Fresán–Jossen exponential period conjecture.** Both are famous, wide-open problems of mainstream transcendence theory. Therefore **δ∉N cannot be made unconditional without proving one of them** — this is not an FTD-specific gap.

Cross-domain point worth stating plainly: FTD's boundary import did **not** hit an idiosyncratic wall. The exact statement it needs — *periods of CM points of distinct imaginary-quadratic fields are algebraically independent* — is the exact statement transcendence theory itself cannot yet prove (Rohrlich–Lang), which is in turn a shadow of the Grothendieck period conjecture. The boundary is priced at a *recognized frontier*, not a private one. `[SYNTHESIS]`

---

## 1 · E1 — the transcendence degree of ℚ(π, Γ(1/4), W_SC, W_FCC)

**The constants.** The three cubic-lattice Watson return-Green's-function constants are CM periods:
- **BCC:** already Γ(1/4)-content — `W_BCC = Γ(1/4)⁴/(4π³)` (Watson 1939), the ℚ(i)/disc-−4 class.
- **SC:** `W_S = (√6/32π³)·Γ(1/24)Γ(5/24)Γ(7/24)Γ(11/24)` — **ℚ(ζ₂₄)-class (24th-division points)**. `[THEOREM — verified this session]` to 25 digits (§3, verifier).
- **FCC:** Γ(1/3)-content — the ℚ(√−3)/disc-−3 class (Glasser–Zucker 1977; Joyce). `[THEOREM — external, literature-attributed; not independently recomputed here]`.

**The obstruction map.**

| | value | status |
|---|---|---|
| proven floor | `trdeg ≥ 2` | `[THEOREM]` Chudnovsky 1976: {π, Γ(1/4)} algebraically independent (also {π, Γ(1/3)}) |
| next rung | `trdeg ≥ 3` | **`[OPEN]`** — even {π, Γ(1/4), Γ(1/3)} jointly is open (Waldschmidt Conj. 5.23) |
| conjectural value | `trdeg = 4` | **`[CONJECTURE]`** — Rohrlich–Lang / Grothendieck period conjecture |

**Why the gap 2→4 is entirely open, precisely:**
- **Chudnovsky 1976 `[THEOREM]`** proves {π, Γ(1/4)} and {π, Γ(1/3)} each algebraically independent (via periods/quasi-periods of `y²=x³−x`, j=1728, and `y²=x³−1`, j=0). This is the *only* known transcendence proof for either Γ-value, and it already gives the stronger independence over ℚ(π).
- **Nesterenko 1996 `[THEOREM]`** strengthens each to a trdeg-3 triple — {Γ(1/4), π, eᵖ} and {Γ(1/3), π, e^{π√3}} — but the third generator is a **same-field exponential**, *not* the other Γ-value. So the two triples share only π and their conjunction does **not** give {π, Γ(1/4), Γ(1/3)}.
- **The joint two-Γ statement `[OPEN]`** is Waldschmidt Conjecture 5.23 ("π, Γ(1/3), Γ(1/4) are algebraically independent"), following from Rohrlich–Lang; one cannot even prove Γ(1/4) or Γ(1/3) transcendental *over ℚ* without the stronger transcendence over ℚ(π).
- **The SC constant — CORRECTED 2026-07-09 (FTD-0377):** the *individual* Γ(1/24)-class values remain unproven transcendental (Fermat genus > 1 / higher-dimensional CM abelian-variety obstruction, as for Γ(1/5)) — **but this does not transfer to W_SC itself.** The specific product Γ(1/24)Γ(5/24)Γ(7/24)Γ(11/24) is exactly the χ₋₂₄ Chowla–Selberg combination, which collapses to the disc −24 (h=2) CM period: `W_S = (3√6/2)·Ω₋₂₄/π²` exactly, so **{π, W_SC} are algebraically independent unconditionally** (Chudnovsky via the in-print Ω_D statement, Zudilin arXiv:2508.17738). See [`THEOREM_WATSON_SC_TRANSCENDENCE.md`](THEOREM_WATSON_SC_TRANSCENDENCE.md): **all three Watson constants are per-constant closed**; E1's whole open content is the *cross-disc joint* independence — the **multi-curve Chudnovsky**, strictly weaker than full Rohrlich–Lang.
- **The relation-structure conjectures are open `[CONJECTURE]`:** Rohrlich's multiplicative conjecture and the stronger Rohrlich–Lang conjecture (Waldschmidt Conj. 22) generate all Γ-value relations from distribution + reflection + oddness; **Lang's conjecture** (`trdeg = 1 + φ(n)/2` for the field of Γ(a/n)) is proven **only for n = 3, 4, 6** (Chudnovsky + Chowla–Selberg; Fresán–Jossen Conj. 1.3.4). "Distinct-CM-field periods are independent" is a *consequence* of Rohrlich–Lang + GPC (André §24.6), not a theorem; GPC for CM abelian varieties is "still widely open" (Gao–Ullmo 2024), with only fragments proven (Bost–Charles prove GPC¹ = divisor classes only; the single-CM-elliptic-curve case is Chudnovsky).

**Verdict (amended 2026-07-09, FTD-0377):** `[SYNTHESIS]` E1's joint field admits **no unconditional progress above the trdeg-2 floor** — but the floor itself is now **per-constant closed** ({π, W} independent for each Watson constant individually; FTD-0377), and the open content is precisely the **multi-curve Chudnovsky** (strictly weaker than the full Rohrlich–Lang this note originally priced it at): proven single-curve < multi-curve (OPEN, = E1) < Rohrlich–Lang < GPC.

---

## 2 · E2 — transcendence of the exponential lattice periods H_σ(τ)

`H_σ(τ) = (2π)⁻³ ∫_{[0,2π]³} e^{−τσ(k)} d³k` (rational τ > 0, cubic-lattice symbol σ) — exponential periods mixing e-content with Γ/π-periods.

- **Individual transcendence — SPLIT, and partly CLOSED 2026-07-09 (FTD-0378):** transcendence **over ℚ** is now a theorem for the SC and BCC symbols, for all algebraic τ ≠ 0 — `H_SC(τ) = e^{−τ}I₀(τ/3)³` falls to Siegel–Shidlovskii (Bessel E-functions; value-field trdeg exactly 3), and `H_BCC(τ) = e^{−τ}·₂F₃(½,½;1,1,1;τ²/4)` to the linear Siegel–Shidlovskii (Beukers Cor. 1.4) — see [`THEOREM_EXPONENTIAL_LATTICE_PERIODS_TRANSCENDENCE.md`](THEOREM_EXPONENTIAL_LATTICE_PERIODS_TRANSCENDENCE.md). Transcendence **over ℚ(π, Γ(1/4))** — the E2-full statement — remains `[OPEN]`: not formally comparable to the (e,π) problem, but behind the same wall (the natural route formally contains it), and no unconditional E-value-vs-π crossing exists. Nesterenko-type methods reach only *same-field* exponentials. `[SYNTHESIS]`
- **The gating conjecture is the Fresán–Jossen exponential period conjecture** (Conj. 8.2.6 / numerical form 1.3.2): for an exponential motive M, `trdeg ℚ(periods of M) = dim G_M`. It is explicitly a `[CONJECTURE]` — the exponential-motives analogue of GPC — under which independence of H_σ(τ) from {π, Γ(1/4)} follows. (Schanuel's conjecture may suffice for sub-cases; whether it is strictly weaker here is one of the exported sub-questions below.)
- **The dim-G_M handle — resolved for the value block, and CORRECTED (FTD-0378):** the exponential motive is identified (Fresán–Jossen §12.7 template; Bessel motives, G = GL₂), and via the André/F–J E-function dictionary the Siegel–Shidlovskii value-field statement **is** the numerical period conjecture restricted to the rapid-decay block — trdeg exactly 3 for the SC symbol, **unconditional**. But the original hope here was logically miscast: an **upper** bound on trdeg can never *prove* independence (a lower-bound statement), and the unconditional lower-bound technology works over ℚ̄ only — blind to π and Γ(1/4) by construction. Withdrawn.

**Verdict (amended 2026-07-09, FTD-0378):** `[SYNTHESIS]` E2's individual-transcendence floor is **closed** (SC and BCC symbols, unconditionally); E2-full (independence from {π, Γ(1/4)}) is gated behind a single instance of the exponential period conjecture, with the (e,π) problem embedded in the natural route — mirroring E1's shape: floor closed, wall named.

---

## 3 · Pre-registered numerics (what computation can and cannot say)

Two facts were computed this session (verifier `proof_e1_sc_watson_gamma_class.py`, exit 0):

1. **`[THEOREM — verified]`** `W_S = 3·∫₀^∞ e^{−3t}I₀(t)³ dt = (√6/32π³)·Γ(1/24)Γ(5/24)Γ(7/24)Γ(11/24)` to 25 digits — confirming the SC constant's ℚ(ζ₂₄)-class content.
2. **`[NUMERICAL FACT — multiplicative-only, height ≤ 10⁶]`** the SC Γ-product `Γ(1/24)Γ(5/24)Γ(7/24)Γ(11/24)` is **not** multiplicatively expressible via {Γ(1/3), Γ(1/4), π, √2, √3} — PSLQ finds no integer relation among their logs up to coefficient height 10⁶. Basis fixed *before* the search (an absence-of-relation test, not a near-miss search).

**The honest limit of the numerics (mandatory):** PSLQ-on-logs detects only *multiplicative* relations. **Algebraic** independence — what E1 actually asks — is strictly stronger (two numbers can be multiplicatively independent yet algebraically dependent), and **no numerical method can establish it.** Fact 2 rules out only the simplest reduction, to a stated height. It is consistent with the conjectural trdeg 4; it is *not* evidence of it. `[SYNTHESIS]`

---

## 4 · What this does to the boundary price

- **IMP-C3 (E1)** is re-priced from "open" to **"open — a case of Rohrlich–Lang / GPC (Waldschmidt Conj. 5.23; Lang proven only n=3,4,6)."**
- **IMP-C4 (E\*/E\*\*, subsuming E2)** is re-priced to name **the Fresán–Jossen exponential period conjecture** for the exponential-period cases.
- **δ∉N (FTD-0369/0370)** is therefore, stated precisely: **`[THEOREM — conditional on: Chudnovsky 1976 (proven) + Rohrlich–Lang/GPC (E1, open) + the exponential period conjecture (E2, open)]`.** No tag moves: x₊=1/α stays `[SMC]`, FC-W stays `[AXIOM]`, MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`. This *sharpens* the price; it does not close or reopen anything.

---

## 5 · Refined exported sub-questions (the tractable edges)

These narrow the exported problems to where progress might actually be found — offered as mathematics (attribution-not-endorsement):

1. **E1 sub-question.** The SC constant's Γ(1/24)-content corresponds to a CM abelian variety with multiplication by ℚ(ζ₂₄). Is there a conditional-but-*sharper* reduction — e.g. via that specific abelian variety — placing the SC constant's trdeg contribution under a more tractable hypothesis than full Rohrlich–Lang?
2. **E2 sub-question.** Is there a concrete exponential motive M whose periods are exactly the H_σ(τ), so that `dim G_M` is computable and gives an **unconditional upper bound** on trdeg — possibly separating H_σ(τ) from {π, Γ(1/4)} without the exponential period conjecture? And would Schanuel *alone* suffice for the independence, i.e. is it strictly weaker than Conj. 8.2.6 here?

---

## 6 · References (citation-verified this session)

- G. V. Chudnovsky (1976) — algebraic independence of {π, Γ(1/4)} and of {π, Γ(1/3)}; the single-CM-elliptic-curve case of GPC.
- Yu. V. Nesterenko (1996) — algebraic independence of {q, E₂, E₄, E₆}(q); specializes to {Γ(1/4), π, eᵖ} and {Γ(1/3), π, e^{π√3}}.
- M. Waldschmidt, *Transcendence of Periods: the State of the Art* and the AWS Lecture 5 notes — Theorems 14/17, Conjectures 5.23 (joint Γ), 22 (Rohrlich–Lang); the "denominator dividing 6" transcendence ceiling.
- J. Fresán, P. Jossen, *Exponential Motives* — Conj. 1.3.4 (Lang, proven only n=3,4,6), Conj. 8.2.6 / 1.3.2 (exponential period conjecture), the unconditional `trdeg ≤ dim G_M`.
- J.-B. Bost, F. Charles (arXiv:1307.1045) — GPC as a de Rham–Betti conjecture; GPC¹ (divisor classes) proven for all abelian varieties.
- Z. Gao, E. Ullmo (arXiv:2411.12249, 2024) — "Grothendieck's period conjecture for CM abelian varieties is still widely open."
- W. Zudilin (arXiv:2508.17738, 2025) — field-uniform restatement of the Chudnovsky/Nesterenko periods results.
- Watson (1939); Glasser–Zucker (1977, with SC erratum history); Joyce; Guttmann (2010 survey) — the SC/FCC Watson closed forms.

---

## 7 · Honest boundary

This is a `[SYNTHESIS]` map of external mathematics + a verified closed form + pre-registered numerics; **it proves nothing new and solves neither E1 nor E2.** The citations rest on secondary surveys (Waldschmidt, Fresán–Jossen) reconstructed and cross-checked, not on the primary Chudnovsky/Nesterenko papers read line by line; the specific conjecture numbers should be confirmed against a current edition before external citation. The value delivered is the *precise pricing of the boundary* — the Number-One-Goal "mark and price" face at its intended altitude.
