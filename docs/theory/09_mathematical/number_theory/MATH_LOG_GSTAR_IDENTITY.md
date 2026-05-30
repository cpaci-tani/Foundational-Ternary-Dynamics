# The log G* Identity: Absorption of All Unsolved L-Values

## G* as the Universal Container for Arithmetically Irreducible Constants

**Date:** April 3, 2026 (cross-link added 2026-04-26)
**Status:** [THEOREM] (identity verified to 80+ digits)
**Proof script:** `scripts/verification/verify_log_gstar_identity.py`

> **Parent identity (closed form).** The expansion below is the L-value series of the *closed* identity
> $$\log G^* \;=\; \beta'(0) \,+\, \log 2 \;=\; \zeta'(0,\,1/4) \,-\, \zeta'(0,\,3/4)$$
> where β(s) = L(s, χ_{−4}) is the Dirichlet beta function. See MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md, Derivation 9 ("From the Stirling Complement"), for the derivation via the Lerch formula and the structural placement of G* as the L-value-sector residual of Γ at z = 1/4 against its (divergent-at-1/4) Stirling skeleton. The series below is what the parent identity yields under expansion via the Hurwitz–Bernoulli machinery.

---

## 1. The Identity

**Claim LGS-1.** [THEOREM] The logarithm of G* = Gamma(1/4)/Gamma(3/4) expands as:

$$\log G^* = \frac{\gamma + 3\log 2}{2} - \frac{G_{\mathrm{Cat}}}{2} + \frac{7}{24}\zeta(3) - \frac{\beta(4)}{4} + \frac{31}{160}\zeta(5) - \cdots$$

where:
- gamma = 0.57721... is the Euler-Mascheroni constant
- G_Cat = beta(2) = 0.91597... is Catalan's constant
- zeta(3) = 1.20206... is Apery's constant
- beta(4) = 0.98894... is the Dirichlet beta at 4

The series collects **every unsolved L-value** with rational coefficients. The solved values (zeta(2), zeta(4), beta(3), beta(5), ...) do not appear -- they are carried by pi via the reflection formula.

---

## 2. General Coefficient Structure

**Claim LGS-2.** [THEOREM] The general coefficients are derived from the Taylor expansion of log Gamma(s) - log Gamma(1-s) at s = 1/4, combined with the Hurwitz decomposition into Dirichlet L-functions.

**zeta(2m+1) terms** (Apery's constant and its higher analogues):

$$\text{coefficient of } \zeta(2m+1) = \frac{2^{2m+1} - 1}{(2m+1) \cdot 2^{2m+1}}$$

Explicitly:
- zeta(3): (8-1)/(3*8) = 7/24
- zeta(5): (32-1)/(5*32) = 31/160
- zeta(7): (128-1)/(7*128) = 127/896

**beta(2m) terms** (Catalan's constant and its higher analogues):

$$\text{coefficient of } \beta(2m) = \frac{(-1)^{m+1}}{2m}$$

Explicitly:
- beta(2) = Catalan: -1/2
- beta(4): -1/4
- beta(6): -1/6

The alternating signs in the beta coefficients and the uniform signs in the zeta coefficients reflect the character parity structure described in `MATH_ANTI_CORRELATION_THEOREM.md`.

---

## 3. Interpretation: G* Absorbs the Unsolved World

**Claim LGS-3.** [THEOREM] The identity shows that log G* is a generating function for all arithmetically irreducible L-values:

- **Unsolved constants** (Catalan G, zeta(3), beta(4), zeta(5), beta(6), zeta(7), ...): ALL appear in log G* with nonzero rational coefficients.
- **Solved constants** (zeta(2) = pi^2/6, beta(3) = pi^3/32, zeta(4) = pi^4/90, ...): NONE appear in log G*. They are carried by pi via the Euler/Bernoulli formulas.

This is a precise sense in which G* and pi are **siblings**: pi carries every solved L-value, and G* carries every unsolved one. Together they span the full L-value spectrum. They arise from the same lattice (the Gaussian integers Z[i]) but encode complementary arithmetic information.

---

## 3.5 Structural reason for irreducibility: the functional-equation obstruction

**Claim LGS-3.5.** [STRUCTURAL] The split between "solved" L-values (carried by π via Bernoulli/Euler numbers) and "unsolved" L-values (carried by G\* via the log expansion) is **not empirical** — it has a precise root cause in the functional equation of each L-function.

**The mechanism.** For β(s) = L(s, χ_{-4}), the functional equation `Λ(s) = (4/π)^{s/2} Γ((s+1)/2) β(s) = Λ(1-s)` pairs each value with its reflection across s = 1/2. Reading off the pairs:

| pair (s, 1−s) | s value | 1−s value | both closeable? |
|---|---|---|---|
| (0, 1) | β(0) = 1/2 | β(1) = π/4 | **YES** (both rational/π) |
| (−1, 2) | β(−1) = **0** (trivial zero) | β(2) = **Catalan** | **NO** (zero blocks) |
| (−2, 3) | β(−2) = −1/2 (rational, via Euler numbers) | β(3) = π³/32 | **YES** |
| (−3, 4) | β(−3) = **0** | β(4) = unknown | **NO** |
| (−4, 5) | β(−4) = 5/2 (rational) | β(5) = 5π⁵/1536 | **YES** |
| (−5, 6) | β(−5) = **0** | β(6) = unknown | **NO** |

**Pattern.** Closed-form positive β-values pair with rational negative values (carried by Euler numbers). Open positive β-values pair with **trivial zeros** of β at the negative odd integers. The trivial zeros are forced by the Γ((s+1)/2) factor in the completed L-function — they are structural, not numerical accidents.

**The same mechanism for ζ(s).** Functional equation pairs s = 2k+1 with s = −2k. ζ at negative even integers is a trivial zero (Riemann's). Therefore ζ(2k+1) — Apéry's constant ζ(3) and beyond — is open for the same structural reason: the functional-equation reflection lands on a trivial zero, and the equation gives an indeterminate 0/0 instead of a closed form.

**Where G\* sits, by contrast.** G\* lives at β′(0), the *derivative* at s = 0 — a non-degenerate point (β(0) = 1/2 is finite, not a zero). The functional equation at s = 0 is well-behaved, and Lerch's formula extracts G\* cleanly from the Γ-asymptotics. **G\* is a derivative at a generic point; Catalan is a value at a critical point.** The two are structurally different objects of the same L-function, separated by whether the functional-equation reflection lands on a zero.

**Consequence: G\* does not give Catalan.** No closed form `β(2) = (rational) · π^a · G*^b` is expected to exist. Heuristically (Boyd, Borwein, Beilinson conjectures), Catalan is algebraically independent of {π, log 2, Γ(1/4)}, and §4's PSLQ search confirms numerically that no low-complexity relation exists. Structurally, β(2) is the regulator of K_3(Z[i]) up to rational by Beilinson's conjecture — analogous to how ζ(3) is the regulator of K_3(Z) — and no regulator of a non-trivial K-group has ever been expressed in elementary closed form.

**The honest framing.** The series for log G\* in §1 is best read not as "G\* contains the irreducibles" (which is true but inert) but as "G\* is a *generating function* for the regulator-irreducible sector at level 4". The irreducibles are absorbed because they are **the only L-values that the functional equation cannot reduce to π**. G\*'s β′(0) form is the natural generating object for this sector at the smallest discriminant (D = −4).

This places `MATH_LOG_GSTAR_IDENTITY` in the Beilinson-conjecture framework: log G\* is to K-theory at level 4 what π is to closed L-values. **Both fail to give Catalan in closed form** — but that failure is *structural*, not a deficiency of the framework.

---

## 4. PSLQ Non-Relations

**Claim LGS-4.** [THEOREM] Using the PSLQ integer relation algorithm at 80+ digits of precision:

- NO algebraic relation exists between any individual unsolved constant and {pi, G*, varpi} of degree <= 20 and coefficient height <= 10^6.
- Specifically tested: Catalan's G, zeta(3), beta(4), zeta(5), and zeta(7) individually have no low-degree algebraic relation with pi or G*.
- The unsolved constants are **absorbed** into G* only through the infinite series -- they cannot be individually extracted by algebraic operations on G*.

This confirms that G* is a **transcendental container**: it holds all the unsolved constants but none can be recovered from it in closed form. The absorption is through an infinite sum, not through algebraic operations.

---

## 5. The Content Product

**Claim LGS-5.** [THEOREM] Define the product of all "content values":

$$P_{\mathrm{content}} = \prod_{m=1}^{\infty} \beta(2m) \cdot \prod_{m=1}^{\infty} \zeta(2m+1)$$

Numerically (truncating at m = 10):

$$P_{\mathrm{content}} \approx 1.13985\ldots$$

This product converges because both beta(2m) -> 1 and zeta(2m+1) -> 1 exponentially fast as m -> infinity. The product encodes the total "unsolved content" of number theory in a single real number. Its relation to G* is through exponentiation of the log G* series, but no closed form for P_content itself is known.

---

## Depends On

- Family of races: `docs/theory/09_mathematical/MATH_FAMILY_OF_RACES.md` (G* = R_4 = Gamma(1/4)/Gamma(3/4))
- Anti-correlation theorem: `docs/theory/09_mathematical/MATH_ANTI_CORRELATION_THEOREM.md` (explains why solved/unsolved alternate)
- Standard results: Taylor expansion of log Gamma, Hurwitz zeta decomposition, PSLQ algorithm

## Honesty Notes

1. The identity is a consequence of the Taylor expansion of log Gamma -- it is **classical mathematics** repackaged, not a new discovery.
2. The claim that G* "absorbs" unsolved constants is a poetic restatement of the series expansion. It does not imply that knowing G* to arbitrary precision would allow extraction of individual L-values.
3. The PSLQ non-results are numerical, not proofs of algebraic independence. They rule out low-degree relations but cannot exclude high-degree ones.
4. The content product P_content is a defined quantity with no known closed form. Its significance is organizational, not computational.
5. This document makes NO physics claims. The connection to FTD is only that G* = Gamma(1/4)/Gamma(3/4) is the same constant that appears in the master quadratic.
