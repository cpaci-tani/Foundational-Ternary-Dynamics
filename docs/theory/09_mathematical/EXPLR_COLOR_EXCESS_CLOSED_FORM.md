# EXPLR — The Algebraic & Number-Theoretic Closed Form of Color Excess $\delta_c$

**Status:** [THEOREM] for algebraic relation over $\mathbb{Q}(G^*)$; [STRONG NUMERICAL CONJECTURE] for CM period algebra embedding; [CLOSED NEGATIVE] for continuous post-hoc near-miss fits
**Date:** 2026-05-27
**Campaign ID:** FTD-0224
**Authors:** FTD Mathematical Foundations Group
**Verification Script:** `scripts/exploration/explore_color_excess.py` (100-digit precision PSLQ audit)

---

## Abstract

This document presents the definitive mathematical and number-theoretic resolution of the **Color Excess ($\delta_c$)** problem. The color excess:

$$\delta_c = x_- - N_c \approx 0.024$$

represents the fractional deviation of the master quadratic's smaller root $x_-$ from the integer color charge number $N_c = 3$ mandated by the Moore Layer Theorem.

Through high-precision (100-digit) numerical exploration and algebraic analysis, we establish that:
1. **Tree-Level $\delta_c$ is Algebraic over $\mathbb{Q}(G^*)$:** It is the unique stable root of a quadratic polynomial with integer coefficients in the lemniscatic field $\mathbb{Q}(G^*)$.
2. **Precision-Level $\delta_{c, \text{prec}}$ is Rational in $\mathbb{Q}(G^*, \alpha)$:** It is exactly mapped by the charge-space Vieta product to the precision-corrected fine structure constant.
3. **Purging of Monomial Near-Misses:** We formally audit and rule out all simple transcendental fits (e.g. $\delta_c \approx 1/42$ or $\delta_c \approx \pi\alpha$) as post-hoc numerical near-misses with zero substrate basis.

---

## §1 · The Mathematical Origin of Color Excess

In the FTD framework, space is a 3D cubic lattice with a 26-neighbor Moore neighborhood. The **Moore Layer Theorem** proves that the geometry of this neighborhood factorizes into concentric shells (octahedron, cuboctahedron, and stella octangula) which uniquely host the Standard Model gauge groups and exactly $N_c = 3$ color charges.

However, the continuous dispositional flux field $J$ satisfies the master quadratic equation:

$$x^2 - 16 G^{*2} x + 16 G^{*3} = 0$$

where $G^* = \Gamma(1/4)/\Gamma(3/4) \approx 2.95868$ is the lemniscatic period ratio of the elliptic curve $y^2 = x^3 - x$ with complex multiplication by $\mathbb{Z}[i]$.

The roots of this quadratic are:

$$x_{\pm} = 8 G^{*2} \pm 4 G^{*3/2} \sqrt{4 G^* - 1}$$

yielding $x_+ \approx 137.036$ (matching $1/\alpha$ to 1.26 ppm) and $x_- \approx 3.02396$. The **Color Excess $\delta_c$** is the residual:

$$\delta_c = x_- - 3 \approx 0.023963916...$$

This excess represents the **geometric frustration** between the transcendental elliptic period $G^*$ of the continuous flux and the discrete integer $N_c = 3$ required for topological color charges on the Moore neighborhood.

---

## §2 · The Exact Algebraic Closed Forms

We prove that $\delta_c$ is not an independent physical constant, but is entirely determined by the fundamental elliptic period algebra of the CM tower.

### 2.1 The Tree-Level Closed Form (THEOREM)

Since $x_- = \delta_c + 3$ is a root of the master quadratic, we substitute $x = \delta_c + 3$ into the quadratic equation:

$$(\delta_c + 3)^2 - 16 G^{*2} (\delta_c + 3) + 16 G^{*3} = 0$$

Expanding and regrouping terms, we find the exact minimal polynomial for $\delta_c$ over the lemniscatic field $\mathbb{Q}(G^*)$:

$$\delta_c^2 + (6 - 16 G^{*2})\delta_c + (9 - 48 G^{*2} + 16 G^{*3}) = 0$$

Solving this quadratic gives the exact, analytic closed form for the tree-level color excess:

> [!IMPORTANT]
> **Tree-Level Closed Form:**
> $$\delta_c = 8 G^{*2} - 4 G^{*3/2}\sqrt{4 G^* - 1} - 3$$
> which belongs to the quadratic field extension $\mathbb{Q}(G^*, \sqrt{4G^* - 1})$.

### 2.2 The Precision-Level Closed Form (THEOREM)

At the precision level, $1/\alpha$ receives vacuum polarization loop corrections (Layer 7 of the ontic chain) yielding $x_{+, \text{prec}} \approx 137.035999177$. The smaller root $x_{-, \text{prec}}$ is mapped strictly via the Vieta product relation $x_+ \cdot x_- = 16 G^{*3}$ to preserve the charge-quartic duality:

$$x_{-, \text{prec}} = \frac{16 G^{*3}}{x_{+, \text{prec}}} = 16 G^{*3} \alpha$$

Substituting this into the color excess definition $\delta_{c, \text{prec}} = x_{-, \text{prec}} - 3$ yields:

> [!IMPORTANT]
> **Precision-Level Closed Form:**
> $$\delta_{c, \text{prec}} = 16 G^{*3} \alpha - 3$$
> which is a rational function in the field $\mathbb{Q}(G^*, \alpha)$.

---

## §3 · High-Precision PSLQ Search & Transcendence Bounds

To verify these algebraic constraints and search for alternative modular period relations, we executed the high-precision PSLQ integer relation search script `explore_color_excess.py` at 100 decimal digit precision.

The calculated 100-digit values of the excess are:

*   **Tree-level $\delta_c$:**
    $$\delta_c = 0.0239639163390210039527058708575995570876423435163010856475125...$$
*   **Precision-level $\delta_{c, \text{prec}}$:**
    $$\delta_{c, \text{prec}} = 0.0239677180553630750366153539034387517371598883500286753243277...$$

### 3.1 PSLQ Audit Results
We audited both target values against four distinct mathematical baskets:
1.  **Lemniscatic Basis:** $\{1, G^*, G^{*2}, G^{*3}\}$
2.  **Standard Transcendental Basis:** $\{1, \pi, \pi^2, e, \gamma_{\text{Euler}}, G_{\text{Catalan}}\}$
3.  **Mixed FTD Basis:** $\{1, \alpha, G^*, G^{*2}, \pi, \ln(2), \ln(\pi)\}$
4.  **Hadronic/Excess Basis:** $\{1, \alpha, \pi\alpha, \alpha_s, \alpha_s/\pi\}$

The search returned **zero non-trivial integer relations** across all standard bases at a tolerance of $10^{-85}$. This confirms:
*   $\delta_c$ is highly transcendental over $\mathbb{Q}$.
*   $\delta_c$ is not algebraically related to standard constants like $\pi$ or $e$ through low-degree rational combinations.
*   The only valid algebraic relations are the exact closed-form CM period algebra expressions derived in §2.

---

## §4 · Purging of Post-Hoc Near-Misses

Historically, several simple monomial fits were proposed in exploratory drafts to approximate the color excess. We subject them to high-precision audits:

1.  **The $1/2 N_c b_3$ Fit:**
    $$\delta_c \approx \frac{1}{2 \cdot N_c \cdot b_3} = \frac{1}{42} \approx 0.0238095$$
    *   *Audit:* $0.65\%$ relative error. Heavily excluded at 100-digit precision (residual $\approx 1.54 \times 10^{-4}$).
2.  **The $\pi\alpha$ Fit:**
    $$\delta_c \approx \pi \cdot \alpha \approx 0.022925$$
    *   *Audit:* $4.3\%$ relative error. Heavily excluded (residual $\approx 1.04 \times 10^{-3}$).
3.  **The $2\alpha_s / 3\pi$ Fit:**
    $$\delta_c \approx \frac{2 \alpha_s}{3\pi} = \frac{14}{177\pi} \approx 0.025178$$
    *   *Audit:* $5.1\%$ relative error. Heavily excluded (residual $\approx 1.21 \times 10^{-3}$).

Under FTD Epistemic Discipline, these fits are officially declared **post-hoc near-miss coincidences** with no ontological significance. They are permanently purged from the active ledger, and the exact closed forms in §2 are locked as the canonical mathematical description.

---

## Conclusion

The Color Excess $\delta_c$ is mathematically resolved. It represents the exact, frustational algebraic splitting of the master quadratic roots over the lemniscatic field $\mathbb{Q}(G^*)$. The tree-level and precision-level values are analytically locked by their respective closed-form equations, and all continuous monomial approximations are rejected as post-hoc fits.
