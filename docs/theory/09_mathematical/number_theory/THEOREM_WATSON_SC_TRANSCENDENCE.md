# THEOREM — {π, W_SC} are algebraically independent: the disc −24 reduction, the per-constant closure of E1's floor, and the multi-curve wall

**Tag:** `[THEOREM — external, assembled]` for the headline result (every load-bearing link is a published theorem or elementary exact algebra; no numerical step is load-bearing) **+ `[SYNTHESIS]`** for the wall map (§4). **Promotes nothing**: E1's *joint* independence stays `[OPEN]`; δ∉N's conditionality is *refined*, not reduced.
**LEDGER id:** FTD-0377 · **Verifier:** [`scripts/proofs/proof_watson_sc_transcendence.py`](../../../../scripts/proofs/proof_watson_sc_transcendence.py) — 11/11 links at 100 dps (several exact); transcendence inputs **cited, not verified**.
**Corrects:** the "SC is worst-off" reading in [`ANALYSIS_E1_E2_TRANSCENDENCE_SOTA.md`](ANALYSIS_E1_E2_TRANSCENDENCE_SOTA.md) (FTD-0376) — see §3.
**Provenance:** assembled 2026-07-09 from a pre-registered PSLQ discovery pass (identity confirmed classical) + a three-lane adversarial workflow (literature / wall / adversarial — all lanes CONFIRMED). ⚠ **Assembled inside an AI session; needs external human number-theory review before outward citation.**

---

## 0 · Headline

> **Theorem (assembled).** Let `W_S = (3/π³)∫_{[0,π]³} dV/(3 − cos k₁ − cos k₂ − cos k₃) = 1.5163860591…` be the simple-cubic Watson constant (Watson's normalization; the bare torus mean is `W_S/3`). Then **π and W_S are algebraically independent over ℚ** — in particular W_S is transcendental — and moreover **trdeg ℚ(π, W_S, e^{π√6}) = 3**.

The same holds, with discriminants −3 and −4, for the FCC and BCC Watson constants. **All three Watson constants are therefore individually resolved**; the *only* remaining open content of E1 is the *joint* independence across discriminants (§4).

To our knowledge — after a targeted sweep of the lattice-Green's-function literature (Zucker's 2011 "70+ Years of the Watson Integrals" survey, Glasser–Zucker 1977, Guttmann 2010, Zudilin 2025) — **this corollary has never been stated for the Watson constants**, although every ingredient is classical and the general Ω_D statement is in print. It is expert-folklore-grade, not new mathematics; the assembly and the explicit reduction constant appear to be new.

---

## 1 · The proof chain (all links published theorems or exact elementary algebra)

Let `P₊ = Γ(1/24)Γ(5/24)Γ(7/24)Γ(11/24)` and `P₋ = Γ(13/24)Γ(17/24)Γ(19/24)Γ(23/24)`. The Kronecker character χ₋₂₄ is +1 exactly on {1,5,7,11} mod 24 — the SC product runs over precisely the χ₋₂₄-positive residues.

1. **(Glasser–Zucker 1977, as corrected; Zucker 2011 eq. (3.6) — ×3 for Watson's normalization.)** `W_S = (√6/32π³)·P₊`. *(The 1977 PNAS original famously omitted a factor 384π; always cite the corrected form.)*
2. **(Elementary, exact.)** `sin(π/24)sin(5π/24)sin(7π/24)sin(11π/24) = 1/16`: pairing, `sin(π/24)sin(11π/24) = cos(5π/12)/2` and `sin(5π/24)sin(7π/24) = cos(π/12)/2`, so the product is `cos(5π/12)cos(π/12)/4 = sin(π/12)cos(π/12)/4 = sin(π/6)/8 = 1/16`. By reflection, `P₊·P₋ = π⁴/(1/16) = 16π⁴`.
3. **(Chowla–Selberg, disc −24: h = 2, w = 2.)** In Zudilin's normalization `Ω_D := (2π/|D|)·(∏_a Γ(a/|D|)^{χ_D(a)})^{1/h}`, steps 1–2 give **`Ω₋₂₄ = (π/12)·√(P₊/P₋) = P₊/(48π)`**, hence the exact reduction
$$\boxed{\;W_S \;=\; \frac{3\sqrt6}{2}\cdot\frac{\Omega_{-24}}{\pi^{2}}\;}$$
(verified to 100 dps, residual exactly 0).
4. **(Chudnovsky 1976; the Ω_D corollary stated in print in Zudilin, arXiv:2508.17738.)** For **any** D < 0, `{Ω_D, π}` are algebraically independent over ℚ. **(Nesterenko 1996.)** For any D < 0, D ≡ 0,1 (mod 4): `{Ω_D, π, e^{π√|D|}}` are algebraically independent. D = −24 qualifies.
5. **(Transfer lemma — elementary.)** If `W = c·π^m·Ω^n` with `c ∈ ℚ̄^×`, integers m, n, n ≠ 0, then a nontrivial `Q(W, π) = 0` over ℚ becomes `Σ c_{ij}c^iΩ^{ni}π^{mi+j} = 0` with the exponent map `(i,j) ↦ (ni, mi+j)` **injective**, contradicting step 4; trdeg is insensitive to the algebraic base extension ℚ → ℚ̄. Applied with `(c, m, n) = (3√6/2, −2, 1)`. ∎

**Classical scaffolding (dispensable but pinned):** Watson 1939 proved `W_S = [4(18+12√2−10√3−7√6)/π²]·K²(k₆)` with `k₆ = (2−√3)(√3−√2)` the 6th singular value (`K′/K = √6`, τ = i√6, CM by ℤ[√−6] — the **maximal** order of disc −24); Zucker 1977 converted `K²(k₆)` to Γ-values. Equivalently `P₊ = 384(1+√2)·k₆·π·K(k₆)²` (the pre-registered PSLQ discovery form of this session, confirmed classical). The general Chowla–Selberg exponent on the period is **4h/w** (disc −24: 4·2/2 = 4; disc −4: 4·1/4 = 1; disc −3: 4·1/6 = 2/3).

---

## 2 · The uniform picture — all three Watson constants

| lattice | constant | closed form | disc | h | `{π, W}` |
|---|---|---|---|---|---|
| BCC | `W_B = Γ(1/4)⁴/4π³ = G*²/2π` | Watson 1939 | **−4** | 1 | **independent** (Chudnovsky) |
| FCC | `W_F = (√3/π²)K²(k₃) = 3Γ(1/3)⁶/2^{14/3}π⁴` | Watson 1939 | **−3** | 1 | **independent** (Chudnovsky) |
| SC | `W_S = (√6/32π³)P₊ = (3√6/2)Ω₋₂₄/π²` | GZ 1977 (corr.) / **this note** | **−24** | 2 | **independent** (this note, §1) |

Each Watson constant is an explicit algebraic multiple of `Ω_D^{…}/π^{…}` for its own discriminant; each is individually transcendental and algebraically independent of π; Nesterenko adds the same-field exponential (`e^{2π}, e^{π√3}, e^{π√6}` respectively) for trdeg 3 per constant. **E1's floor is therefore fully "per-constant closed"; everything open is cross-disc.**

---

## 3 · Correction to FTD-0376

FTD-0376's obstruction map said the SC constant is "worse" because Γ(1/24)-class values are not even individually proven transcendental. That is true **of the individual Γ(1/24)** — and remains true — but it does **not** transfer to W_S: the specific *product* `P₊` is exactly the χ₋₂₄ Chowla–Selberg combination, which collapses (§1, steps 2–3) to the h = 2 CM period. **W_S itself is transcendental, unconditionally.** The corrected map: no Watson constant is individually open; the floor of E1's field is trdeg ≥ 2 with *every single generator* resolved against π; the gap 2 → 4 is purely the cross-disc joint independence.

---

## 4 · The wall, precisely (multi-curve Chudnovsky) `[SYNTHESIS]`

E1 restates exactly as: **`trdeg ℚ(π, Ω₋₄, Ω₋₃, Ω₋₂₄) = 4`** (each Watson constant generates, over ℚ̄ and up to π-powers and quadratic extensions, the same field as its Ω_D). The Grothendieck period conjecture predicts exactly 4: for the abelian threefold `E₋₄ × E₋₃ × E₋₂₄` (pairwise non-isogenous, distinct CM fields), `dim MT = 2+2+2−2 = 4` (fiber product over the shared weight 𝔾_m; cross-checked against Bertolin's elliptico-toric formula). The first new rung, trdeg ≥ 3, needs any single cross-disc pair.

**The hierarchy (all inclusions strict):**
$$\text{single-curve Chudnovsky (PROVEN)} \;<\; \textbf{multi-curve Chudnovsky (OPEN)} \;<\; \text{Rohrlich–Lang (OPEN)} \;<\; \text{André/GPC (OPEN)}$$
Multi-curve Chudnovsky does *not* imply Rohrlich–Lang (it says nothing about Γ(1/5) — abelian-surface periods — or the universal distribution structure), so pricing E1 here is a genuine sharpening over FTD-0376's R–L pricing.

**Obstruction map (what is available vs missing):**
- Zero/multiplicity estimates on commutative algebraic groups: **available** (Philippon 1986; Wüstholz 1989) — *not* the wall.
- One-point modular multiplicity estimate: **available** (Nesterenko 1996) — powers the only unconditional trdeg-3 mechanism; intrinsically one-orbit.
- **Missing (a): cross-curve coupling in the auxiliary construction.** For non-isogenous CM curves `End(E₁×E₂) = End(E₁) ⊕ End(E₂)` — no cross endomorphisms — so the Siegel-lemma parameter count on `𝔾_a² × Ẽ₁ × Ẽ₂` reproduces the single-curve codim-1 output ("at least 2 of…"), which one curve already gives.
- **Missing (b): the codim-2 Gelfond elimination.** Certifying trdeg ≥ 3 needs a near-optimal *measure* of algebraic independence for the first pair as input; the known measures (Philibert; Philippon–Bruiltet, with a ~10³²⁶ constant) are orders of magnitude too weak. Tosi (arXiv:2309.02800) names this bottleneck verbatim.
- **Missing (c): a two-point multiplicity estimate for the doubled Ramanujan system.** Nesterenko's D-stable-ideal classification fails on the product (modular correspondences `Φ_n(J₁,J₂) = 0` supply infinitely many invariant subvarieties). The canonical named marker is **Bertrand's conjecture** (Ramanujan J. 1, 1997): `J(α₁), J(α₂)` algebraically independent for multiplicatively independent algebraic nome — even this qualitative two-point trdeg-2 statement is open. (There is no conjecture literally named "multi-point Nesterenko"; the full multi-curve prediction is André's generalized period conjecture / Bertolin's elliptico-toric conjecture.)
- **Partial-results inventory (none cross fields):** Vasil'ev 1996 / Grinspan 2002 (`≥ 2 of {π, Γ(1/5), Γ(2/5)}`) live over the single field ℚ(ζ₅); Philippon–Wüstholz elliptic Lindemann–Weierstrass is one-curve at algebraic points, never periods of two curves; Diaz's large-trdeg results are exponential-setting only. **No unconditional trdeg ≥ 3 across two distinct CM fields exists.**

**Consequence for the boundary price:** the E1 leg of δ∉N is re-priced from "a case of Rohrlich–Lang / GPC" to the sharper **"a case of multi-curve Chudnovsky — strictly weaker than Rohrlich–Lang"**; with this note's theorem, the E1 field's *generators* are all individually closed and the import is purely the cross-disc coupling. No tag moves: δ∉N stays `[THEOREM — conditional]` on the same (now more precisely named) open inputs; x₊ = 1/α stays `[SMC]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`.

---

## 5 · References

- G. N. Watson, *Three triple integrals*, Quart. J. Math. Oxford **10** (1939) 266–276.
- M. L. Glasser, I. J. Zucker, *Extended Watson integrals for the cubic lattices*, PNAS **74** (1977) 1800–1801 — **as corrected** (the published form omits a factor 384π; see Zucker 2011 eq. (3.6)).
- I. J. Zucker, *70+ Years of the Watson Integrals*, J. Stat. Phys. **145** (2011) 591–612 — eqs. (1.8), (1.9), (3.5)–(3.7); the erratum history.
- I. J. Zucker, Math. Proc. Camb. Phil. Soc. **82** (1977) 111–118 — `K[N]` in Γ-values via Selberg–Chowla.
- S. Chowla, A. Selberg, J. reine angew. Math. **227** (1967) 86–110.
- G. V. Chudnovsky, Dokl. Akad. Nauk Ukrain. SSR Ser. A **8** (1976) 698–701.
- Yu. V. Nesterenko, Sb. Math. **187** (1996) 1319–1348.
- W. Zudilin, *Linear independence measures for Chowla–Selberg periods*, arXiv:2508.17738; RIMS Kôkyûroku **2340** (2026) 130–134 — the in-print `{Ω_D, π}` statement (its quantitative Theorem 1 covers D ∈ {−148,−232,−267,−163} only — **not** −24).
- D. Bertrand, Ramanujan J. **1** (1997) — the two-point J-value conjecture. C. Bertolin, J. Number Theory **97** (2002) — elliptico-toric conjecture. P. Philippon, Bull. SMF **114** (1986); G. Wüstholz, Ann. Math. **129** (1989) — zero estimates. Vasil'ev (1996); P. Grinspan, J. Number Theory **94** (2002) 136–176; R. Tosi, arXiv:2309.02800.
- J. M. Borwein, I. J. Zucker, IMA J. Numer. Anal. **12** (1992) 519–526 — `Γ(1/24)Γ(11/24)/[Γ(5/24)Γ(7/24)] = √3(2+√3)^{1/2}`.

---

## 6 · Honest boundary

The theorem is an **assembly** of published results; its value is (i) the explicit reduction constant `3√6/2` and the one-line chain, (ii) closing the per-constant floor of E1 (correcting FTD-0376), and (iii) the sharpened wall pricing. It does **not** touch the joint (cross-disc) independence — the actual open content of E1 — and it makes **no** progress on Rohrlich–Lang itself. Watson's 1939 formulas were verified against Zucker's 2011 survey (the primary is paywalled) with every quoted formula independently recomputed at 50–140 dps. Produced in an AI session with a three-lane adversarial panel; **external human review is required before any outward citation of the theorem.**
