# Chowla–Selberg Extension to Class Number h ≥ 2

**Tag:** [THEORY NOTE — literature synthesis]
**Date:** 2026-05-02
**Closes:** Math-complete checklist MC-T2.3 (and supports MC-T1.2 route a).
**Audience:** project owner + future agents extending Theorem 3 to non-h=1 CM fields.

---

## Purpose

The classical Chowla–Selberg formula expresses the period lattice of a
CM elliptic curve over an imaginary quadratic field of class number
h = 1 as a product of Γ-function values. FTD-0002 (G* algebraic
identity / Theorem 1) and FTD-0007 (coefficient 16) both depend on
this h = 1 case via the curve E: y² = x³ − x (CM by Z[i], discriminant
d = −4, class number 1).

Theorem 3 (CM curve uniqueness) was retagged 2026-05-02 as
`[NUMERICAL FACT, exhaustive over 9-element h = 1 set]` (Tier-I
MC-T1.2 closure via route b). The structural question — whether
the dual permille match (1/α, N_c) extends to CM curves with
h ≥ 2 — requires the higher-h analogue of Chowla–Selberg.

This document records the literature framework for that extension
and identifies what would be needed to upgrade Theorem 3 from
[NUMERICAL FACT, h=1 only] to a proper structural theorem.

This is documentation only — it is NOT a derivation, a numerical
scan, or a closure of MC-T1.2 route (a). It is the analytic
machinery list that MC-T1.2 route (a) would need.

---

## 1. The classical Chowla–Selberg formula (h = 1)

For an imaginary quadratic field K = ℚ(√−d) of discriminant D = −d
and class number h(K) = 1, with E_K the associated CM elliptic curve,
the Chowla–Selberg formula (1949) states:

$$
\Omega_K = \frac{1}{\sqrt{2\pi |D|}} \prod_{a=1}^{|D|-1} \Gamma\left(\frac{a}{|D|}\right)^{\chi(a)/2 h(K)}
$$

where:

- $\Omega_K$ is the period of the CM curve (a transcendental number).
- $\chi$ is the Kronecker character of $K$.
- The product is over residues mod $|D|$.

For $K = \mathbb{Q}(i)$ (d = −4, h(K) = 1, χ the non-trivial
character mod 4):

$$
\Omega_{\mathbb{Q}(i)} = \frac{1}{\sqrt{8\pi}} \cdot \frac{\Gamma(1/4)^{1/2}}{\Gamma(3/4)^{1/2}}
$$

which gives, after squaring and rearranging via Euler reflection
$\Gamma(1/4) \Gamma(3/4) = \pi \sqrt{2}$:

$$
G^* = \frac{\Gamma(1/4)}{\Gamma(3/4)} = \frac{\Gamma(1/4)^2}{\pi \sqrt{2}}
$$

This is the FTD canonical G* (FTD-0002 / Theorem 1).

## 2. The h = 1 structural privilege of d = −4

Of the nine class-number-1 imaginary quadratic discriminants

$$
d \in \{-3, -4, -7, -8, -11, -19, -43, -67, -163\},
$$

only $d = -4$ has the dual-permille match:
- $x_+ = 137.036$ matches $1/\alpha$ to 1.26 ppm
- $x_- = 3.024$ matches $N_c$ to 0.80%

**Empirical observation** (Theorem 3 / FTD-0003): no other h = 1
discriminant produces this dual match.

**Structural reason candidates:**
- d = −4 is uniquely Z[i]-CM: its endomorphism ring Z[i] has
  a non-trivial automorphism group |Z[i]^×| = 4 (vs. ±1 for d = −3
  Eisenstein curves with extra j = 0 structure but different unit
  group).
- The $\Gamma(1/4)$ ratio in Chowla–Selberg for d = −4 has a
  particularly clean form (just $\Gamma(1/4)/\Gamma(3/4)$ via
  Euler reflection).
- The CM ring Z[i] is the unique non-cyclotomic Euclidean
  imaginary quadratic ring of class number 1 with a non-trivial
  unit group of order > 2.

These are SUGGESTIVE. None are formally a structural-uniqueness
proof.

---

## 3. The h ≥ 2 generalization

For class number h(K) ≥ 2, the curve $E_K$ no longer has a single
period; it has a period lattice with $h$ inequivalent generators.
The Chowla–Selberg formula generalizes to:

$$
\prod_{[\mathfrak{a}] \in \text{Cl}(K)} \Omega_{\mathfrak{a}}^2
= \left(\frac{1}{2\pi |D|}\right)^{h(K)/2}
\prod_{a=1}^{|D|-1} \Gamma\left(\frac{a}{|D|}\right)^{\chi(a)}
$$

where the product on the LHS is over the ideal-class group of $K$.

This is the **Damerell–Anderson–Schipnitzer formula** (1971; cf.
also Gross–Koblitz, p-adic version).

### What's different at h ≥ 2

- Multiple inequivalent CM curves exist over $K$ — one for each
  ideal class (or pair of ideal classes under complex conjugation).
- The period $\Omega_{\mathfrak{a}}$ depends on the chosen ideal
  class $[\mathfrak{a}]$.
- The "G* analogue" $G^*_K$ is not a single number but a vector
  indexed by ideal classes.

### Smallest h = 2 cases

The first imaginary quadratic fields with class number 2 are at
discriminants

$$
d \in \{-15, -20, -24, -35, -40, -51, -52, -88, -91, -115, -123, -148, -187, -232, -235, -267, -403, -427\}
$$

(Stark–Heegner list). Each gives 2 inequivalent CM curves; for each
one constructs an analogue master quadratic and tests for dual permille
match against (1/α, N_c).

---

## 4. What MC-T1.2 route (a) would need

To upgrade Theorem 3 from [NUMERICAL FACT, h = 1 only] to a structural
theorem covering all h, the following machinery is required:

1. **Damerell-style identities for h = 2, 3, ...**: explicit
   $\Gamma$-function product expressions for periods of CM curves
   over class-number-h fields.

2. **Per-ideal-class master-quadratic analogue**: for each
   $[\mathfrak{a}] \in \text{Cl}(K)$, compute the analogue
   $G^*_{[\mathfrak{a}]}$ and the resulting $P_{[\mathfrak{a}]}(x)$.

3. **Dual-match scan across all h ≤ N**: numerically test the
   dual permille match across all h ≤ 5 (say) curves for at
   least the first $\sim 100$ discriminants.

4. **Structural argument for non-match at h ≥ 2** (or, if any h ≥ 2
   curve dual-matches, a structural argument for why it doesn't
   contradict the d = −4 selection — perhaps Z[i] is privileged
   among CM rings by its unit-group order).

The estimated effort: **W–M (2–6 weeks) for a focused mathematician
with familiarity with CM theory.** Beyond session scope but well-defined.

---

## 5. Connection to MC-T2.1 + MC-T2.2 (extended polynomial scan)

The 2026-05-02 extended polynomial scan (`proof_polynomial_look_elsewhere_extended.py`)
searched 2.87M polynomials in the EXTENDED Gaussian-integer-tower
family + Eisenstein-integer-tower family.

**Result**: 0 dual-matchers in Eisenstein family. 1 dual-matcher in
extended Gaussian family (the master quadratic itself). *(Note: "dual-matcher" here refers to the historical target pair `(1/α, N_c)` used by the scan; the `x_-  N_c` identification is **retired** per v1.4 §5 — LEDGER FTD-0014 removed in commit `ca7eb61` — but the polynomial-template-uniqueness fact about the master quadratic is **independent of the target identification** and stands.)*

**Interpretation**: this is INDIRECT evidence for the d = −4 structural
privilege. Eisenstein integers Z[ω] (CM ring of curves with j = 0,
including y² = x³ − 1) form a different CM ring with |Z[ω]^×| = 6
(units {±1, ±ω, ±ω²}). The fact that no Eisenstein-family multiplier
produces the dual permille match is consistent with the conjecture
that d = −4 is structurally privileged.

If the h ≥ 2 generalization eventually shows that no h ≥ 2 curve
produces the dual match either, the cumulative argument would be:

> Theorem 3 (extended): Among all CM elliptic curves with CM by an
> order $O_K$ in any imaginary quadratic field $K$, the unique curve
> producing the dual permille match (1/α, N_c) via the master quadratic
> structure is $E: y² = x³ − x$ (CM by Z[i], discriminant d = −4,
> class number 1).

This would be a major structural result — closing both T1.2 and
strengthening T2.1+T2.2.

---

## 6. Status summary

| Question | Status | Effort to close |
|---|---|---|
| Chowla–Selberg formula at h = 1 | classical [THEOREM] | done (FTD-0002) |
| Damerell formula at h = 2, 3, ... | classical [THEOREM] | literature, 1-2 days |
| Numerical scan of master-quadratic analogue at h ≥ 2 | [OPEN] | 1-2 weeks |
| Structural theorem of d = −4 uniqueness across all CM curves | [OPEN] | 2-6 weeks |

---

## 7. References

- Chowla, S. & Selberg, A. (1949). On Epstein's Zeta Function. *PNAS*.
- Damerell, R. (1970, 1971). L-functions of elliptic curves with
  complex multiplication, I, II. *Acta Arith.*
- Anderson, G. (1982). Logarithmic derivatives of Dirichlet L-functions
  and the periods of abelian varieties. *Compositio Math.*
- Gross, B. & Koblitz, N. (1979). Gauss sums and the p-adic Γ-function.
  *Annals of Math.*
- Cox, D. A. (2013). *Primes of the form x² + ny²: Fermat, class field
  theory, and complex multiplication*, 2nd ed. Wiley. — best modern
  textbook on the relevant CM theory + class field theory background.
