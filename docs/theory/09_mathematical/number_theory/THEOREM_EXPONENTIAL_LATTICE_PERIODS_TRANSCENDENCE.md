# THEOREM — Exponential lattice periods are transcendental: E2's individual-transcendence sub-question closed (SC and BCC symbols), and the E2-full wall priced

**Tag:** **Theorem A (SC):** `[THEOREM — external, assembled]` (all inputs classical: Siegel 1929/Shidlovskii, Kolchin/Kovacic, Beukers 2006). **Theorem B (BCC):** `[THEOREM — assembled]`, one short module-theoretic lemma (simplicity + Frobenius reciprocity) not found stated in the literature — flagged for external review. **Wall pricing (§4):** `[SYNTHESIS]`. **Promotes nothing:** E2-full (independence from {π, Γ(1/4)}) stays `[OPEN]`.
**LEDGER id:** FTD-0378 · **Verifier:** [`scripts/proofs/proof_e2_exponential_periods_transcendence.py`](../../../../scripts/proofs/proof_e2_exponential_periods_transcendence.py) — identity links exact (residuals 0 at 40 dps); transcendence inputs **cited, not verified**.
**Corrects:** the "even individual transcendence appears open" line in [`REF_EXPORTED_PROBLEMS_E1_E2.md`](REF_EXPORTED_PROBLEMS_E1_E2.md) P2 and the E2 entry of [`ANALYSIS_E1_E2_TRANSCENDENCE_SOTA.md`](ANALYSIS_E1_E2_TRANSCENDENCE_SOTA.md) (FTD-0376); also re-casts P2's dim-G_M sentence (an upper bound cannot prove independence — §5).
**Provenance:** assembled 2026-07-09; two-lane adversarial workflow (adversarial: MIXED→repaired; pricing: CONFIRMED). The adversarial lane **caught a defect in the original BCC chain** (§3 hazard) before it entered the corpus. ⚠ **Assembled in an AI session; external human review required before outward citation — especially Theorem B's linear-independence lemma.**

---

## 0 · Headlines

> **Theorem A (SC symbol).** For every **nonzero algebraic** τ,
> $$H_{SC}(\tau) = \frac{1}{(2\pi)^3}\int_{[0,2\pi]^3} e^{-\tau\,(1-\frac{\cos k_1+\cos k_2+\cos k_3}{3})}\,d^3k \;=\; e^{-\tau}\,I_0(\tau/3)^3$$
> is **transcendental**. Moreover the value field is pinned exactly: `trdeg ℚ̄(e^τ, I₀(τ/3), I₀′(τ/3)) = 3`.

> **Theorem B (BCC symbol).** For every nonzero algebraic τ,
> $$H_{BCC}(\tau) = \frac{1}{(2\pi)^3}\int_{[0,2\pi]^3} e^{-\tau\,(1-\cos k_1\cos k_2\cos k_3)}\,d^3k \;=\; e^{-\tau}\cdot{}_2F_3\!\left(\tfrac12,\tfrac12;\,1,1,1;\,\tfrac{\tau^2}{4}\right)$$
> is **transcendental**.

These close the "more modest" sub-question of exported Problem 2 (E2) — *individual* transcendence, which FTD-0376 and the export doc recorded as open — for the two named cubic symbols, **unconditionally**, and for all algebraic (not just rational) τ ≠ 0. No statement of either result was found in the lattice-Green's-function / heat-kernel literature; the inputs are classical, so this is **assembly/packaging novelty only**. **E2-full — independence from {π, Γ(1/4)} — remains open** and is priced precisely in §4.

---

## 1 · Theorem A: the chain (all links classical)

1. **Factorization (elementary, exact).** `e^{(τ/3)Σcos kᵢ}` factors, and `(2π)⁻¹∫e^{x cos k}dk = I₀(x)`, so `H_SC(τ) = e^{-τ}I₀(τ/3)³`.
2. **E-functions.** `I₀(z/3) = Σ (z²/36)^m/(m!)²` is a strict Siegel E-function (rational coefficients, height O(m); Siegel 1949 covers all pFq-type series with rational parameters and algebraic scaling); `e^z` likewise. The joint system for `(e^z, I₀(z/3), I₀′(z/3))` is `Y′ = AY` over ℚ(z) with `T(z) = z` — **singular only at 0**.
3. **Functional independence over ℚ̄(z).** The modified Bessel (ν=0) equation has Picard–Vessiot group **SL₂** (Wronskian rational ⇒ G ⊆ SL₂; irreducible and non-Liouvillian for ν ∉ ½+ℤ — Kolchin 1968; Kovacic 1986's worked Bessel example). SL₂ has no 𝔾_m quotient, so no `e^{cz}` lies in the Bessel PV field; the joint group is **SL₂ × 𝔾_m**, giving functional trdeg 3.
4. **Siegel–Shidlovskii (Beukers 2006, Thm 1.1).** At any algebraic ξ with `ξ·T(ξ) ≠ 0` — here **all algebraic τ ≠ 0**, negative included — the *values* inherit the functional transcendence degree: `trdeg ℚ̄(e^τ, I₀(τ/3), I₀′(τ/3)) = 3`.
5. **Transfer.** If `H = e^{-τ}I₀³` were algebraic, `I₀(τ/3)³ − H·e^τ = 0` would be a nontrivial polynomial relation between two of the three independent values. ∎

*(A subtlety handled: the classical value-level statement "e^α, J₀(α), J₀′(α) independent" is same-argument; the chain avoids needing it by putting the **rescaled** function I₀(z/3) into the system and evaluating at ξ = τ.)*

---

## 2 · Theorem B: the linear route

The series rewrite (exact, verifier L3): `(2π)⁻³∫e^{τc₁c₂c₃} = Σ_m [\binom{2m}{m}/4^m]³ τ^{2m}/(2m)! = {}_2F_3(½,½;1,1,1;τ²/4) =: F(τ)` — a strict hypergeometric E-function of Katz type (2,4), operator `Hyp((t+½)², t⁴)` (order 4).

The relation to exclude is only **linear**: `H_BCC(τ) = v ∈ ℚ̄` means `F(τ) − v·e^τ = 0`. So the **linear** Siegel–Shidlovskii (Beukers Cor. 1.4) suffices, needing only ℚ̄(z)-**linear** independence of `{e^z, F, F′, F″, F‴}`:
- **Simplicity.** `Hyp((t+½)², t⁴)` is simple by Katz's criterion (no root of P congruent mod ℤ to a root of Q: `−½ − 0 ∉ ℤ`), so the minimal operator has full order 4, and linear independence descends through the quadratic pullback `z ↦ τ²/4`.
- **No rank-1 submodule.** `e^τ` in the span would give a rank-1 submodule N of the pullback; Frobenius reciprocity for the finite étale double cover gives `0 ≠ Hom([2]_*N, H)` with `[2]_*N` of rank 2 mapping nonzero into a **simple** rank-4 module — impossible. *(Numerical cross-check: `H_BCC(τ)·τ^{3/2} → ≈0.254`, exponent −3/2 ∉ ℤ, independently excluding any rational-multiple-of-`e^τ` element.)*
- Beukers Cor. 1.4 at `ξ = τ ≠ 0` then gives ℚ̄-linear independence of the five values, so `F(τ) ≠ v·e^τ`. ∎

## 3 · ⚠ The SO₄ hazard (recorded so it is never "cited")

The *original* proposed chain for B — cite the Katz/Duval–Mitschi differential-Galois classification and claim **full** functional independence of `{e^z, F, F′, F″, F‴}` — is **defective**: Katz's type-(2,4) classification leaves `G^{o,der} ∈ {SL₄, SO₄, Sp₄}`; self-duality of the module (a = {½,½}, b = {1,1,1,1} duality-stable) already excludes SL₄; and **if the group is SO₄, full independence is FALSE** (F and its derivatives satisfy a genuine quadratic relation over ℚ̄(z)). The Duval–Mitschi table entry for this parameter point was not verifiable. **Never cite full algebraic independence for the BCC block** unless someone actually pins Sp₄ vs SO₄; the linear route of §2 avoids the question entirely. *(Caught adversarially before entering the corpus — the review process working as designed.)*

---

## 4 · E2-full, priced precisely `[SYNTHESIS]`

What remains open, in decreasing strength (all unconditional-status):
- **(a)** `trdeg ℚ(π, Γ(1/4), e^τ, I₀(τ/3), I₀′(τ/3)) = 5` — the natural full target. This **formally contains** "e^τ and π algebraically independent" as a sub-statement, which for rational τ ≠ 0 is **equivalent to the famously open (e, π) problem** (even the irrationality of e+π is open) — and requires an I₀-vs-π crossing besides, for which **no technology exists at all**.
- **(b)** The literal E2-full: `H_σ(τ)` transcendental over ℚ(π, Γ(1/4)). **Logical care (lane-verified): there is NO formal implication in either direction between (b) and the (e, π) problem** — if `e^τ` were algebraic over ℚ(π), H could still be transcendental over ℚ(π) via the I₀³ factor, and a relation P(H, π) = 0 does not isolate `e^τ`. The correct statement: *not formally comparable, but behind the same wall* — every available strategy passes through (and beyond) (e, π).
- **(c)** Even `H_SC(τ)` transcendental over ℚ(π) alone: open.

**Conditional closure:** a **single instance of the Fresán–Jossen exponential period conjecture** (Conj. 8.2.6/8.2.8) for the joint motive — (exponential SC motive, an `H³(𝔾_m³, τσ)` per F–J §12.7/§12.4.5, built from Bessel motives with `G = GL₂`, Prop. 12.4.3) ⊗ (lemniscatic CM motive) — suffices, modulo a finite group-theory computation. **Schanuel's conjecture is insufficient and incomparable** for this instance (it constrains only exponentials, saying nothing about Bessel values). **No unconditional result mixing any E-function value with π or Γ(1/4) exists** — F–J themselves state Bessel-vs-2πi (Conj. 12.4.4) as open; Nesterenko's {π, e^π, Γ(1/4)} remains the unique e-π-type crossing and its quasi-modular mechanism does not transport to Bessel/lattice E-values.

## 5 · The dim-G_M sub-question: resolved for the value block, and one correction

The export doc asked to "identify the exponential motive M and compute dim G_M for an unconditional upper bound [that] may separate H_σ(τ) from {π, Γ(1/4)}."
- **Resolved (value block):** the motive is identified (F–J §12.7 template), and via the André/Fresán–Jossen dictionary between E-functions and exponential motives, Beukers' refined Siegel–Shidlovskii **is** the numerical period-conjecture statement restricted to the rapid-decay value block: for the SC symbol, `trdeg = 3` **exactly, unconditionally** (Theorem A step 4). Not resolved for the **full** period matrix (the K-Bessel/2πi rows — open even for a single Bessel motive, F–J Conj. 12.4.4).
- **Correction (logical miscast):** an **upper** bound on trdeg can never *prove* independence — that is a **lower**-bound statement — and the only unconditional lower-bound technology (Siegel–Shidlovskii/Beukers) works over ℚ̄ and is **blind to π and Γ(1/4) by construction**. The export doc's "may separate without the conjecture" hope is withdrawn.

---

## 6 · The completed E1/E2 pattern

With FTD-0377 and this note, both exported problems now have the same honest shape:

| | tractable floor | status | the wall | priced at |
|---|---|---|---|---|
| **E1** | per-constant: {π, W} indep. for each Watson constant (discs −4/−3/−24) | **CLOSED** (FTD-0377) | cross-disc joint independence | **multi-curve Chudnovsky** < Rohrlich–Lang (marker: Bertrand 1997) |
| **E2** | per-value: H_σ(τ) transcendental (SC, BCC; all algebraic τ≠0) | **CLOSED** (this note) | independence from {π, Γ(1/4)} | **exponential period conjecture** instance; behind the (e,π) wall |

δ∉N's conditionality is unchanged in *strength* but now maximally *named*: Chudnovsky (proven) + multi-curve Chudnovsky (E1, open) + the F–J exponential-period instance (E2, open). No tag moves: x₊=1/α `[SMC]`, FC-W `[AXIOM]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`.

## 7 · References

- C. L. Siegel, *Über einige Anwendungen diophantischer Approximationen* (1929); *Transcendental Numbers* (1949 lectures).
- A. B. Shidlovskii, *Transcendental Numbers*, de Gruyter (1989), Ch. 4.4.
- F. Beukers, *A refined version of the Siegel–Shidlovskii theorem*, Ann. of Math. **163** (2006) 369–379 — Thm 1.1 and Cor. 1.4.
- E. R. Kolchin, *Algebraic groups and algebraic dependence*, Amer. J. Math. (1968); J. Kovacic (1986) — Bessel PV group SL₂.
- N. M. Katz, *Exponential Sums and Differential Equations* — simplicity criterion; type-(p,q) Galois classification (Thm 3.6/Prop 4.0.1). A. Duval, C. Mitschi, Pacific J. Math. **138** (1989); C. Mitschi, Pacific J. Math. **176** (1996).
- Y. André, *Séries Gevrey de type arithmétique I/II*, Ann. of Math. **151** (2000) — E-operators / exponential-motive dictionary.
- J. Fresán, P. Jossen, *Exponential Motives* — §3.4, §4.2, Conj. 8.2.6/8.2.8, Prop. 8.3.1, §12.3–12.7 (Bessel motives, Prop. 12.4.3, Conj. 12.4.4, Conj. 12.6.5, Prop. 12.1.4).
- Yu. V. Nesterenko, Sb. Math. **187** (1996) — the unique unconditional e-π crossing.
- M. Waldschmidt, *Diophantine Approximation on Linear Algebraic Groups* (2000) — (e, π) open-problem status.

## 8 · Honest boundary

Both theorems are **assemblies**; the inputs are classical and the packaging is the contribution. Theorem B contains one short original lemma (the §2 no-rank-1-submodule argument) produced and panel-verified in-session but **not found stated in the literature — it is the single item most needing external eyes**. E2-full remains open and is not touched. Golden hash untouched (docs + one verifier). **External human number-theory review required before outward citation.**
