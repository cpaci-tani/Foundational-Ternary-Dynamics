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

> **[THEOREM 2 (tower discriminant factorization).]** `disc(M_k) = 2^{k+2} · G*^{k−1} · A_k` where `A_k := 2^{k−2} G*^{k−3} − 1` is the **level-`k` discriminant correction**. `A_3 = 1`; `A_k` is irrational for every `k ≥ 4` (since `G*` is transcendental).

A clean closed form for α at tree level follows from Theorem 1 applied to k = 4 (Vieta inversion of FTD-0001):

> **[DERIVED, re-statement of FTD-0001 in publication-grade form].**
> $$\alpha_{\text{tree}} \;=\; \frac{1}{2 G^*} \;-\; \frac{\sqrt{4 G^* - 1}}{4\,G^{*\,3/2}}\,.$$

Numerically `α_tree⁻¹ = 137.036171458…`, with the canonical 1.26 ppm tree-level residual against CODATA `α⁻¹ = 137.035999084` recovered exactly.

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

**Corollary (irrationality of A_k for k ≥ 4).** Since `G*` is transcendental over `Q` (a consequence of Lindemann–Weierstrass applied to `Γ(1/4)` via Schneider 1941; see Waldschmidt's monograph), every nontrivial polynomial expression `2^{k−2}G*^{k−3} − 1` with `k − 3 ≥ 1` is irrational. At `k = 3`, `A_3 = 1` is rational; the discriminant is `32 G*²` and `√disc(M_3) = 4 G* √2`, so the level-3 splitting field is `Q(G*, √2)` (a real quadratic extension of the field generated by `G*`). At every `k ≥ 4`, the splitting field properly contains `Q(G*, √A_k) = Q(G*, √(4G*−1))` for `k = 4`, and so on.

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
- CODATA 2022: `α⁻¹ = 137.035999084(21)` → tree residual = **1.258 ppm**

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

**The master quadratic `M(x) = x² − 16 G*² x + 16 G*³` (FTD-0001, Theorem 2 of SPEC_ALGEBRAIC_SPINE) is the level-4 instance of the (1+i)-tower of `G*`-normalized quadratics `M_k(x) = x² − 2^k G*^{k−2} x + 2^k G*^{k−1}`; this tower admits a single algebraic invariant `1/y_+ + 1/y_− = 1` (where `y_± := x_±/G*`) at every level [THEOREM 1, proved via Vieta + the `c_k = G* · b_k` normalization]; its discriminant factors as `disc(M_k) = 2^{k+2} G*^{k−1} A_k` with level-`k` correction `A_k = 2^{k−2} G*^{k−3} − 1` [THEOREM 2, by direct computation, with `A_k` irrational for `k ≥ 4` modulo Schneider's transcendence of `G*`]; the level-4 instance gives the closed-form tree-level identity `α = 1/(2G*) − √(4G* − 1)/(4 G*^{3/2})` [DERIVED, restatement of FTD-0001 in publication-grade form] reproducing CODATA's `α⁻¹` to the canonical 1.26 ppm; the dual physical-identification conjecture (`α ↔ 1/x_+`, `N_c ↔ x_−`) is unchanged in tag, the (1+i)-multiplier uniqueness and level-4 selection remain [OPEN], and the QFT-conformal-anomaly analogy is metaphor-only [CONJECTURE, exploratory] absent a formal regularization-class construction.**
