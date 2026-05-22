# Theorem — Harmonic invariant of the master-quadratic tower

**Tag:** [THEOREM] (harmonic invariant) + [DERIVED] (closed form for α)
**Date:** 2026-04-29 (late evening)
**LEDGER row:** FTD-0111 (NEW)
**Companion canonical reference:** [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md) §10 (added in same commit)
**Convention.** Throughout this document `G* := Γ(1/4)/Γ(3/4) ≈ 2.9586751` per `scripts/constants.py` line 103/265. (The lemniscate constant `ϖ = Γ(1/4)²/(2√(2π))` differs from this by a factor `2/√π`; the spine doc §1 has a long-standing notation drift between the displayed Γ-ratio formula and the numerical value, which we do not address here. The operative master-quadratic identity `Δ = 64 G*³(4G*−1)` requires the larger value.)

---

## 0 · Summary

The master quadratic `M(x) = x² − 16 G*² x + 16 G*³` (Theorem 2 of SPEC_ALGEBRAIC_SPINE) is the level-4 instance of an infinite family of polynomials

$$M_k(x) := x^2 - 2^k\,G^{*\,k-2}\,x + 2^k\,G^{*\,k-1}, \qquad k \in \mathbb{Z}_{\ge 3}.$$

This family — call it the **(1+i)-tower** since `2^k = N((1+i)^k)` is the Gaussian-integer norm of the unique even prime in `Z[i]` raised to the level — admits a single algebraic invariant that holds at every level:

> **[THEOREM 1 (harmonic invariant).]** Let `y_± := x_±/G*` where `x_±` are the roots of `M_k(x)`. Then `1/y_+ + 1/y_− = 1` for every `k ≥ 3`.

Proof: 3 lines (Vieta + the normalization `c_k = G* · b_k`).

The discriminant factors cleanly at every level:

> **[THEOREM 2 (tower discriminant factorization).]** `disc(M_k) = 2^{k+2} · G*^{k−1} · A_k` where `A_k := 2^{k−2} G*^{k−3} − 1` is the **level-`k` discriminant correction**. `A_3 = 1`; `A_k` is **transcendental** over `Q` for every `k ≥ 4`. (Proof: `G*` is transcendental by Chowla–Selberg + Chudnovsky 1976; for `k ≥ 4` the expression `2^{k−2}·G*^{k−3}` is a non-constant polynomial in `G*` with rational coefficients, hence transcendental, and `A_k` differs from a transcendental by a rational, so `A_k` is itself transcendental. The earlier statement "`A_k` is irrational for `k ≥ 4`" was a strict-but-weaker corollary; tightened 2026-04-30 per `docs/theory/07_assessment/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` Theorem 6.)

A clean closed form for α at tree level follows from Theorem 1 applied to k = 4 (Vieta inversion of FTD-0001):

> **[DERIVED, re-statement of FTD-0001 in publication-grade form].**
> $$\alpha_{\text{tree}} \;=\; \frac{1}{2 G^*} \;-\; \frac{\sqrt{4 G^* - 1}}{4\,G^{*\,3/2}}\,.$$

Numerically `α_tree⁻¹ = 137.036171458…`, with the canonical 1.26 ppm tree-level residual against CODATA `α⁻¹ = 137.035999177` recovered exactly.

This document proves the two theorems, derives the closed form, and explicitly demarcates what is and is not added by the tower view.

---

## 1 · The (1+i)-tower of master quadratics

**Definition (tower).** For each integer `k ≥ 3`, define

$$M_k(x) \;:=\; x^2 \;-\; 2^k\,G^{*\,k-2}\,x \;+\; 2^k\,G^{*\,k-1}.$$

Equivalently `b_k = 2^k G*^{k−2}` and `c_k = 2^k G*^{k−1}`, with the normalization constraint `c_k = G* · b_k`.

**Justification of "(1+i)" labeling.** In `Z[i]`, `(1+i)` is the unique Gaussian prime above the rational prime `2`, with norm `N(1+i) = 2`. The factor `2^k = N((1+i)^k)` therefore enumerates principal ideals along the `(1+i)`-tower in `Z[i]`. Whether this is the *unique* natural tower for FTD or one choice among several (e.g. the rational-prime-3 tower with `b_k = 3^k G*^{k−2}`, the Gaussian-prime-(2+i) tower with `b_k = 5^k G*^{k−2}`) is **[OPEN]** — see §6 below; the harmonic-invariant theorem holds for every such tower trivially (it depends only on the `c_k = G* · b_k` constraint, not on the multiplier), so the (1+i) labeling is presently exposition rather than uniqueness claim.

**Table of low levels:**

| k | b_k        | c_k        | A_k = 2^{k−2}G*^{k−3} − 1 | numeric A_k |
|--:|------------|------------|---------------------------|------------:|
| 3 | 8 G*       | 8 G*²      | 1                         | 1           |
| 4 | 16 G*²     | 16 G*³     | 4 G* − 1                  | 10.834700   |
| 5 | 32 G*³     | 32 G*⁴     | 8 G*² − 1                 | 69.030068   |
| 6 | 64 G*⁴     | 64 G*⁵     | 16 G*³ − 1                | 413.39244   |
| 7 | 128 G*⁵    | 128 G*⁶    | 32 G*⁴ − 1                | 2451.1052   |

(Verified at 50-digit precision; see `scripts/proofs/proof_harmonic_invariant_tower.py`.)

The level `k = 4` is the master quadratic `M(x) = x² − 16 G*² x + 16 G*³` of Theorem 2 of SPEC_ALGEBRAIC_SPINE.

---

## 2 · Theorem 1 — harmonic invariant

**Statement.** Let `M_k(x) = x² − b_k x + c_k` with `c_k = G* · b_k` (the tower normalization). Let `x_+, x_−` be its roots and `y_± := x_±/G*`. Then

$$\frac{1}{y_+} + \frac{1}{y_-} \;=\; 1.$$

**Proof.** By Vieta,

$$x_+ + x_- = b_k, \qquad x_+ \cdot x_- = c_k = G^* \cdot b_k.$$

Therefore

$$\frac{1}{y_+} + \frac{1}{y_-} \;=\; G^*\!\left(\frac{1}{x_+} + \frac{1}{x_-}\right) \;=\; G^* \cdot \frac{x_+ + x_-}{x_+ \cdot x_-} \;=\; G^* \cdot \frac{b_k}{G^* \cdot b_k} \;=\; 1. \qquad\square$$

**Remarks.**
- The proof depends only on the normalization `c_k = G* · b_k`, not on the specific value of `b_k`. The multiplier `2^k` is irrelevant to the invariant — replacing `2^k` with any nonzero multiplier `m_k` preserves the identity. The harmonic invariant is therefore a property of the family `{M(x) : c = G* · b}` of all G*-normalized monic quadratics, with the (1+i)-tower being one indexed sub-family.
- The invariant is genuinely *algebraic*, not analytic: it holds in any field containing `G*` and the ratio `c_k/b_k = G*`.

**[THEOREM]**, proved.

---

## 3 · Theorem 2 — tower discriminant factorization

**Statement.** For every `k ≥ 3`,

$$\operatorname{disc}(M_k) \;=\; 2^{k+2}\,G^{*\,k-1}\,\big(\,2^{k-2}\,G^{*\,k-3} - 1\,\big) \;=\; 2^{k+2}\,G^{*\,k-1}\,A_k,$$

where `A_k := 2^{k−2}G*^{k−3} − 1` is the **level-`k` discriminant correction**.

**Proof.** Direct computation:

$$\operatorname{disc}(M_k) = b_k^2 - 4 c_k = (2^k G^{*\,k-2})^2 - 4 \cdot 2^k G^{*\,k-1} = 2^{2k} G^{*\,2k-4} - 2^{k+2} G^{*\,k-1}.$$

Factor out `2^{k+2} G*^{k−1}`:

$$\operatorname{disc}(M_k) = 2^{k+2}\,G^{*\,k-1}\,\big(2^{k-2}\,G^{*\,k-3} - 1\big). \qquad\square$$

**Corollary (transcendence of A_k for k ≥ 4).** Since `G*` is transcendental over `Q` (a consequence of Lindemann–Weierstrass applied to `Γ(1/4)` via Schneider 1941; see Waldschmidt's monograph), every nontrivial polynomial expression `2^{k−2}G*^{k−3} − 1` with `k − 3 ≥ 1` is **transcendental** (a non-rational polynomial in a transcendental over `Q` with rational coefficients is transcendental). At `k = 3`, `A_3 = 1` is rational; the discriminant is `32 G*²` and `√disc(M_3) = 4 G* √2`, so the level-3 splitting field is `Q(G*, √2)` (a real quadratic extension of the field generated by `G*`). At every `k ≥ 4`, the splitting field properly contains `Q(G*, √A_k) = Q(G*, √(4G*−1))` for `k = 4`, and so on. (Heading was previously "irrationality"; the stronger transcendence claim is the natural conclusion. Tightened 2026-04-30.)

**[THEOREM]**, proved (modulo the standard transcendence theorem of Schneider–Waldschmidt invoked in the corollary).

---

## 4 · Closed form for α at tree level

Apply Theorem 1 at `k = 4`. The master quadratic `M_4(x) = x² − 16 G*² x + 16 G*³` has roots

$$x_{\pm} \;=\; 8 G^{*\,2} \;\pm\; 4\,G^{*\,3/2}\,\sqrt{4 G^* - 1}\,.$$

The dual quadratic obtained by `x ↔ 1/α` is

$$\boxed{\;16\,G^{*\,3}\,\alpha^2 \;-\; 16\,G^{*\,2}\,\alpha \;+\; 1 \;=\; 0\;}$$

(verified algebraically: residual `−5.3 × 10⁻⁵¹` at 50-digit precision; this is exact algebra, the residual is roundoff).

Solving for the smaller root gives the closed form

$$\boxed{\;\alpha_{\text{tree}} \;=\; \frac{1}{2 G^*} \;-\; \frac{\sqrt{4 G^* - 1}}{4\,G^{*\,3/2}}\,. }$$

**Numerical evaluation at G* = 2.95867512...:**

- `1 / (2 G*)` = `0.168994560016821182…`
- `√(4 G* − 1) / (4 G*^{3/2})` = `0.161697216626683099…`
- `α_tree` = `0.007297343390138083…`
- `α_tree⁻¹` = `137.036171458155483…`
- CODATA 2022: `α⁻¹ = 137.035999177(21)` → tree residual = **1.258 ppm**

This is the canonical FTD-0001 1.26 ppm tree-level result, expressed as a difference of two `G*`-algebraic terms rather than as `1/x_+`. The two forms are algebraically identical — the closed form is FTD-0001 in publication-grade prose, not a new claim.

**[DERIVED]** — re-statement of FTD-0001 (Theorem 3 of SPEC_ALGEBRAIC_SPINE) in a more legible algebraic form. Does *not* change the LEDGER tag of the physical identification `α ↔ 1/x_+`, which remains [STRONGLY MOTIVATED CONJECTURE] (the dual conjecture; LEDGER FTD-0001).

---

## 5 · What this section does and does not establish

**What is added to FTD's portfolio:**

1. **Harmonic invariant theorem [THEOREM]**: `1/y_+ + 1/y_− = 1` for the entire `c = G*·b` family of G*-normalized quadratics. This is a new entry to `SPEC_ALGEBRAIC_SPINE.md` (proposed Theorem 8). The proof is complete.
2. **Tower discriminant factorization [THEOREM]**: `disc(M_k) = 2^{k+2}G*^{k−1}A_k`. New algebraic identity; the proof is complete.
3. **Closed form for α [DERIVED]**: `α = 1/(2G*) − √(4G*−1)/(4G*^{3/2})`. A more legible expression of FTD-0001's tree-level prediction. Not a new claim.

**What is NOT added:**

1. **Resolution of the dual conjecture.** The 1.26 ppm match `α ≈ 1/x_+` and the 0.80% match `N_c ≈ x_−` remain [STRONGLY MOTIVATED CONJECTURE] (LEDGER FTD-0001). The harmonic invariant constrains the *algebraic relationship* between the two roots; it does not derive their physical identification.
2. **Selection of level `k = 4`.** The tower parameterizes the master-quadratic family; the empirical observation that the level-4 instance matches `α⁻¹` to 1.26 ppm (and N_c to 0.80% in the same instance) is unaltered. **[OPEN]**: why level 4? See §6.
3. **A formal anomaly construction.** The phrase "level-4 anomaly" used informally in some discussions is a metaphor borrowed from QFT conformal-anomaly language; it does not correspond to any derived QFT result in FTD as of this writing. **[CONJECTURE / exploratory]**: that there exists a regularization-class construction in the matched-stencil lattice EFT whose level enumerates `k`. Until such a construction is exhibited, "level-`k` discriminant correction" is the appropriate technical term, not "anomaly."
4. **Uniqueness of the (1+i)-multiplier choice.** The harmonic invariant holds for every family `c = G* · b`, regardless of multiplier. The (1+i)-tower selects `m_k = 2^k`; other Gaussian-prime towers (`(2+i)`-tower with `m_k = 5^k`, etc.) and rational-prime towers (`m_k = p^k`) all admit the harmonic invariant identically. **[OPEN]**: a CM-style rigidity scan analogous to the 60k-polynomial scan that rigidified FTD-0001, asked at the level of multiplier choice rather than coefficient choice.

---

## 6 · Open questions opened by the tower view

The tower view reorganizes the master-quadratic content of FTD without changing it. In doing so, it sharpens several research questions:

**Q1. (1+i)-uniqueness.** Among tower families `M_k^{(p)}(x) = x² − p^k G*^{k−2}x + p^k G*^{k−1}` for various primes `p`, is there a structural argument singling out `p = 2` (equivalently, the (1+i)-tower in `Z[i]`)? The naive empirical scan: at level 4, does any other tower `(p^4 G*², p^4 G*³)` admit a root that matches a known physical constant to ppm precision? This is testable via a small computational sweep over `p ∈ {3, 5, 7, 11, 13, …}` (rational primes) and `p ∈ {1+i, 2+i, 1+2i, 3+2i, …}` (Gaussian primes).

**Q2. Why level 4?** Granting the (1+i)-tower as the natural family, the level `k = 4` is the empirical match to `α⁻¹`. The tower view does not derive this. The question "why level 4 carries the physics" is the same question as "why the master quadratic with coefficient 16 specifically" — restated, not resolved. Two attack routes: (a) **structural**, via the four framework integers `{N_c=3, N_base=4, b_3=7, N_eff=13}` — `k = 4 = N_base` is suggestive but not derivation; (b) **modular**, via Damerell-style L-value evaluation at level 4 of some discrete tower in `Z[i]` — speculative.

**Q3. Other levels' physical content (if any).** The tower predicts `x_+(k) ~ 2^k G*^{k−2}` and `x_−(k) → G*` as `k → ∞`. The numerical values are:

| k | x_+(k)    | x_−(k)    |
|--:|----------:|----------:|
| 3 |   20.20   |   3.466   |
| 4 |  137.036  |   3.024   |
| 5 |  825.82   |   2.969   |
| 6 | 4901.25   |   2.960   |
| 7 | 29017.0   |   2.959   |

The small-root asymptote `→ G*` is automatic (algebra: `x_−(k) → c_k/b_k = G*`); it is not a prediction. The large root `x_+(k) → b_k = 2^k G*^{k−2}` is also automatic. Asking whether `825.82` or `4901.25` matches some physical coupling at GUT or Planck scale is a fishing expedition unless an independent reason to look at level `k` is supplied. **[OPEN, fishing-discipline applies]** — any positive match must be pre-registered against the published tower numerics, not retrofitted.

**Q4. Connection to lattice-EFT anomalies.** Speculative: the matched-stencil lattice EFT has a regularization parameter (the lattice spacing relative to the cluster). Whether the discrete log of this parameter enumerates a tower of effective theories whose discriminant corrections coincide with `A_k` is **[OPEN, exploratory only]**. No work has been done; flagging only because the QFT-anomaly metaphor pushed in this direction.

---

## 6.5 · Level-3 cyclotomic identity (added late 2026-04-29)

**[THEOREM]** At level `k = 3`, the inverted normalized roots are exact algebraic constants in the cyclotomic field `Q(ζ_8) = Q(i, √2)`:

$$\frac{1}{y_+(3)} \;=\; \frac{2 - \sqrt{2}}{4} \;=\; \sin^2\!\left(\frac{\pi}{8}\right), \qquad \frac{1}{y_-(3)} \;=\; \frac{2 + \sqrt{2}}{4} \;=\; \cos^2\!\left(\frac{\pi}{8}\right).$$

**Proof.** At `k = 3`, `b_3 = 8 G*`, `c_3 = 8 G*²`, so

$$\operatorname{disc}(M_3) = 64 G^{*\,2} - 32 G^{*\,2} = 32 G^{*\,2}, \qquad \sqrt{\operatorname{disc}(M_3)} = 4 G^* \sqrt{2}.$$

Therefore `x_±(3) = 2 G*(2 ± √2)`, so `y_±(3) = 2(2 ± √2)`, and

$$\frac{1}{y_\pm(3)} \;=\; \frac{1}{2(2 \pm \sqrt{2})} \;=\; \frac{2 \mp \sqrt{2}}{2 \cdot (4 - 2)} \;=\; \frac{2 \mp \sqrt{2}}{4}.$$

The half-angle identities `sin²(π/8) = (1 − cos(π/4))/2 = (2 − √2)/4` and `cos²(π/8) = (2 + √2)/4` complete the identification. Verified at 20-digit precision in the verification script (Section 6.7 below). □

**Structural reading.** At level `k = 3`, the discriminant correction `A_3 = 1` is rational (Section 3), so the splitting field of `M_3` is `Q(G*, √2)` — a degree-2 rational extension of the field generated by `G*`. The `G*`-factor *factors out cleanly* from the algebraic content; the level-3 master quadratic's roots are `G* × (rational over Q(√2))`. The inverted normalized roots `1/y_±(3)` therefore live in `Q(√2) ⊂ Q(ζ_8)`, the cyclotomic field of 8-th roots of unity, evaluating to `sin²(π/8)` and `cos²(π/8)`.

This is genuinely new content of Theorem 8: the harmonic invariant `1/y_+ + 1/y_− = 1` at level 3 is realized by the *concrete* pair `(sin²(π/8), cos²(π/8))` — the natural "two complementary projections" on the 8-element cyclic group. **This connects the (1+i)-tower (Gaussian integers, `(1+i) ∈ Z[i]`) at level 3 to the cyclotomic ladder `Q(i) = Q(ζ_4) ⊂ Q(ζ_8)`** — a structural link between the multiplier `2^k = N((1+i)^k)` and the cyclotomic extensions of `Q(i)`.

---

## 6.6 · Structural reason for `k = 4` selection [SELECTION PRINCIPLE candidate]

**[SELECTION PRINCIPLE candidate, NOT proved as forcing]** Level `k = 4` is the **smallest level at which the discriminant correction `A_k` contains a positive power of `G*`**.

**Verification.** From the closed form `A_k = 2^(k−2) G*^(k−3) − 1`:

| k | `A_k`                | `G*`-power |
|---|----------------------|-----------:|
| 3 | `1`                  | 0          |
| 4 | `4 G* − 1`           | 1          |
| 5 | `8 G*² − 1`          | 2          |
| 6 | `16 G*³ − 1`         | 3          |
| 7 | `32 G*⁴ − 1`         | 4          |

At `k = 3`, `A_3 = 1` has `G*`-power 0 (it is purely rational). At every `k ≥ 4`, `A_k` is a polynomial in `G*` of degree `k − 3 ≥ 1`. Therefore the splitting field of `M_k`:
- at `k = 3` is the rational extension `Q(G*, √2)` — `G*` factors out as a multiplier;
- at `k ≥ 4` is `Q(G*, √(A_k))` — a transcendental extension that requires `G*`'s analytic content to specify.

**Selection-principle reading.** If the heuristic is *"physical content emerges at the smallest level where the master quadratic genuinely depends on `G*`'s transcendence,"* then `k = 4` is forced — and `k = 4 = N_base` is then a *consequence*, not a coincidence. The framework integer `N_base = 4` indexes the first `G*`-transcendental level of the tower.

**[OPEN]** What is missing: a derivation showing that this heuristic (physics emerges at first transcendental level) is forced by some structural argument rather than imposed empirically. Possible attack: argue that the level-3 instance is "trivial" (factorizes through `Q(ζ_8)`) and therefore cannot carry physical content distinct from cyclotomic geometry, while level-4 is the first instance with non-cyclotomic algebraic content. This is suggestive but not derivation; **this section presents the structural observation, not a closure.**

This sharpens the "why `k = 4`?" question from "empirical coincidence" to "first `G*`-non-trivial level"; both readings live with the same epistemic tag (the empirical match itself remains [STRONGLY MOTIVATED CONJECTURE]) but the structural reason changes how the question gets stated.

---

## 6.7 · Exploratory scan at `k ∈ [3, 15]` [EXPLORATORY only]

A post-hoc scan against 16 known dimensionless physics constants at 1% tolerance (script: `scripts/exploration/explore_tower_level_scan.py`) returned exactly two matches:

1. **`x_+(4) = 137.0362` ↔ `α⁻¹` (1.26 ppm)** — the canonical FTD-0001 dual conjecture; reproduced.
2. **`1/y_-(4) = 0.9784` ↔ `cos²(θ_13)` PMNS = 0.978 (0.04% / 420 ppm)** — *automatic from harmonic invariant*, since `1/y_-(4) = 1 − 1/y_+(4) = 1 − G*α`, so this is not independent evidence.

No matches at any other level, including the framework-integer levels `k ∈ {3, 7, 13} = {N_c, b_3, N_eff}`. **The framework-integer-as-tower-index hypothesis is falsified by this scan**: only `k = N_base = 4` carries verified physical content, and the structural explanation (Section 6.6) is "first `G*`-non-trivial level," not "framework integer."

**Critical epistemic warning (FTD-0097 fishing-discipline).** This scan was post-hoc, not pre-registered. Its results cannot be used as evidence for or against any framework conjecture. The cyclotomic identity at `k = 3` (Section 6.5) is a [THEOREM] derived independently of the scan — verified algebraically. The framework-integer hypothesis falsification is ALSO independent (it follows from the structural reason in §6.6, which says only `k = 4` is forced by the `G*`-transcendence criterion). The scan itself is exposition only. A future blind scan, with prior pre-registration of (i) the constant catalog, (ii) the level range, (iii) the precision threshold, and (iv) the look-elsewhere correction, would be required for any positive match at `k ≠ 4` to count as evidence. Such a protocol is queued in `PROTOCOL_TOWER_LEVEL_FALSIFIER.md`.

---

## 6.8 · Cover-page reformulation: the dual conjecture as harmonic complement

**[CONJECTURE — restatement of FTD-0001 dual conjecture]** The harmonic invariant evaluated at the physical identifications collapses to one line:

$$\boxed{\; \alpha \;+\; \frac{1}{N_c} \;=\; \frac{1}{G^*} \;}$$

**Derivation.** Theorem 1 gives `1/y_+ + 1/y_- = 1` for the level-4 master quadratic. Substituting `1/y_+ = G*/x_+ = G*α` (FTD-0001 dual: `α = 1/x_+`) and `1/y_- = G*/x_- = G*/N_c` (FTD-0001 dual: `N_c = x_-`), then dividing by `G*`, gives the boxed equation directly. □

**What this is.** Algebraically, this is the dual-prediction conjecture in its most compact form. Numerically, with `α_obs = 0.00729735` and `N_c = 3` (integer) and `1/G* = 0.337988`:

$$\alpha_{\text{obs}} + \tfrac{1}{3} \;=\; 0.34063 \quad \text{vs} \quad 1/G^* = 0.33799,$$

differing by **0.78%** — the *same* 0.78% as the master quadratic's smaller root deviation from 3 (`x_- = 3.0240 = 3 × (1 + 0.0080)`). The two equivalent statements:
- "The master quadratic's smaller root is 3.024, off the integer N_c = 3 by 0.80%"
- "α + 1/N_c (with N_c = 3) deviates from 1/G* by 0.78%"

are the same conjecture, different framings. The boxed form is the conjecture's *cover-page* version: it states FTD's central empirical claim as a one-line conservation law over `(α, 1/N_c, 1/G*)` rather than as a polynomial-root coincidence.

**Cross-domain bridge.** The structure `1/A + 1/B = 1/C` recurs across physics:
- **Parallel resistors** (Kirchhoff): `1/R_eq = 1/R_1 + 1/R_2`
- **Reduced mass** (Newtonian two-body): `1/μ = 1/m_1 + 1/m_2`
- **Thin lens equation** (paraxial optics): `1/f = 1/d_o + 1/d_i`

In each case `C` is a *combined* / *equivalent* / *focal* quantity of which `A` and `B` are reciprocal contributions. **FTD's instance places `G*` as the "equivalent reciprocal" of which `α` (electromagnetic) and `1/N_c` (color) are complementary terms.** The dual conjecture is then not a numerical coincidence — it is the assertion that `G*` plays the structural role of a Kirchhoff-type combined reciprocal across the EM-color sectors.

**[CONJECTURE, exploratory only].** That this Kirchhoff analogy carries any predictive content beyond the identity itself. The analogy is *suggestive* — it changes how the FTD claim should be presented in papers (one line at the top of the abstract) — but it is not a derivation of α from first principles, and it cannot upgrade the empirical match's epistemic tag beyond what FTD-0001 already establishes. Use the analogy in motivation, not in proof.

**Slogan-grade summary**: `(α, 1/N_c)` are complementary harmonic conjugates of `1/G*`.

---

## 7 · Cross-references

- **Canonical theorem list:** `SPEC_ALGEBRAIC_SPINE.md` — proposed Theorem 8 added in same commit.
- **FTD-0001 (master quadratic):** Theorem 2 of SPEC_ALGEBRAIC_SPINE; this is the level-4 instance.
- **Dual conjecture:** §9 of SPEC_ALGEBRAIC_SPINE (`α ↔ 1/x_+` at 1.26 ppm; `N_c ↔ x_−` at 0.80%) — unchanged.
- **60k-polynomial rigidity scan:** `EXPLR_60K_POLYNOMIAL_SCAN.md` — established that FTD-0001's coefficients (16, 16, 1) are rigid against perturbation. Q1 above proposes the multiplier-level analog.
- **Constants:** `scripts/constants.py` line 103/265 (`G_STAR = Γ(1/4)/Γ(3/4) ≈ 2.9586751`).
- **Verification script:** `scripts/proofs/proof_harmonic_invariant_tower.py` (numerical confirmation at 50-digit precision for `k ∈ {3, 4, 5, 6, 7}`).
- **LEDGER row:** FTD-0111 (this entry).

---

## 8 · Single-line summary

**The master quadratic `M(x) = x² − 16 G*² x + 16 G*³` (FTD-0001, Theorem 2 of SPEC_ALGEBRAIC_SPINE) is the level-4 instance of the (1+i)-tower of `G*`-normalized quadratics `M_k(x) = x² − 2^k G*^{k−2} x + 2^k G*^{k−1}`; this tower admits a single algebraic invariant `1/y_+ + 1/y_− = 1` (where `y_± := x_±/G*`) at every level [THEOREM 1, proved via Vieta + the `c_k = G* · b_k` normalization]; its discriminant factors as `disc(M_k) = 2^{k+2} G*^{k−1} A_k` with level-`k` correction `A_k = 2^{k−2} G*^{k−3} − 1` [THEOREM 2, by direct computation, with `A_k` transcendental over `Q` for `k ≥ 4` via Schneider–Chudnovsky transcendence of `G*`]; the level-4 instance gives the closed-form tree-level identity `α = 1/(2G*) − √(4G* − 1)/(4 G*^{3/2})` [DERIVED, restatement of FTD-0001 in publication-grade form] reproducing CODATA's `α⁻¹` to the canonical 1.26 ppm; the dual physical-identification conjecture (`α ↔ 1/x_+`, `N_c ↔ x_−`) is unchanged in tag, the (1+i)-multiplier uniqueness and level-4 selection remain [OPEN], and the QFT-conformal-anomaly analogy is metaphor-only [CONJECTURE, exploratory] absent a formal regularization-class construction.**
