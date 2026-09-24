# Foundation: The Master Quadratic — Bare Algebraic Structure

**Status:** [THEOREM] (all identities below are exact by Vieta; verified to machine precision)
**Purpose:** Strip the master quadratic to its bare mathematical content. What does the underlying math actually say — without physics, without interpretation, just algebra?
**Ledger row:** FTD-0082
**Test:** `engine/tests/test_master_quadratic_identities.cpp` (ctest `master_quadratic_identities`, all identities verified to 1e-15)
**Companion:** [FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md](FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md) — why the polynomial has this form

---

## 0. Executive summary

The master quadratic
$$ x^2 - 16 G^{*2} x + 16 G^{*3} = 0 $$
says, in bare algebraic terms: **there are two positive real numbers $x_+, x_-$ whose sum is $16 G^{*2}$ and whose product is $16 G^{*3}$.** By Vieta, these two conditions imply a third:

$$ \boxed{\ \frac{1}{x_+} + \frac{1}{x_-} = \frac{1}{G^*}\ } $$

**The harmonic identity $1/x_+ + 1/x_- = 1/G^*$ is the deepest algebraic content of the master quadratic.** Everything else is algebra on top of it.

Physics enters only through the identification $x_+ = 1/\alpha$ (FTD-0013, [STRONGLY MOTIVATED CONJECTURE]). Everything in this document is pure algebra in the roots $x_+$ and $x_-$.

---

## 1. The polynomial and its four equivalent forms

Starting from the standard form:
$$ x^2 - 16 G^{*2} x + 16 G^{*3} = 0 \tag{1} $$

this is algebraically equivalent to each of:

**Self-consistency form:**
$$ x = 16 G^{*2} \left(1 - \frac{G^*}{x}\right) \tag{2} $$

**Gap equation form:**
$$ x^2 = 16 G^{*2} (x - G^*) \tag{3} $$

**Normalized form** (with $w = x/(16 G^{*2})$, $\epsilon = 1/(16 G^*)$):
$$ w^2 - w + \epsilon = 0 \tag{4} $$

Each form emphasizes a different aspect:
- (1) factors readily into roots via the quadratic formula
- (2) is a fixed-point equation; its solutions are fixed points of $x \mapsto 16G^{*2}(1 - G^*/x)$
- (3) is the "gap equation" form: energy balance between a quadratic self-term $x^2$ and a linear lattice term $x - G^*$
- (4) is the cleanest: a generic $w^2 - w + \epsilon = 0$ with a single small parameter $\epsilon \approx 0.021$

The normalized form reveals the master quadratic is a **perturbation of the trivial $w^2 - w = 0$** (whose roots are $(0, 1)$) by the single scalar $\epsilon = 1/(16G^*)$.

## 2. The three Vieta identities

For any monic quadratic with roots $x_\pm$, Vieta gives two independent identities plus one derivable from them. For the master quadratic:

| # | Identity | Content |
|---|---|---|
| **V1** | $x_+ + x_- = 16 G^{*2}$ | Sum of roots = linear coefficient |
| **V2** | $x_+ \cdot x_- = 16 G^{*3}$ | Product of roots = constant term |
| **V3** | $\dfrac{1}{x_+} + \dfrac{1}{x_-} = \dfrac{1}{G^*}$ | Sum of reciprocals = $(V1)/(V2)$ |

V3 follows from V1/V2: $\dfrac{V1}{V2} = \dfrac{x_+ + x_-}{x_+ x_-} = \dfrac{1}{x_+} + \dfrac{1}{x_-} = \dfrac{16G^{*2}}{16G^{*3}} = \dfrac{1}{G^*}$.

**Numerical verification** (machine precision):

```
  1/x+ + 1/x−  = 0.337989120033643
  1/G*         = 0.337989120033642
  |diff|       = 8.3e-16     (= floating-point epsilon)
```

V3 is **exact**, not approximate. The apparent 0.05–0.13% near-identities like $\sqrt[3]{18} \approx \varpi$ and $\sqrt[3]{26} \approx G^*$ are real observations, but V3 is a Vieta theorem — it holds to infinite precision.

## 3. The three means form a geometric progression

The arithmetic, geometric, and harmonic means of the two roots are:

| Mean | Formula | Value |
|---|---|---|
| AM = $\tfrac{x_+ + x_-}{2}$ | $8 G^{*2}$ | 70.030 |
| GM = $\sqrt{x_+ \cdot x_-}$ | $4 G^{*3/2}$ | 20.357 |
| HM = $\dfrac{2 x_+ x_-}{x_+ + x_-}$ | $2 G^*$ | 5.917 |

The three means are in **geometric progression**:

$$ \frac{\text{AM}}{\text{GM}} = \frac{\text{GM}}{\text{HM}} = 2\sqrt{G^*} \approx 3.440 $$

The standard identity $\text{AM} \cdot \text{HM} = \text{GM}^2$ holds trivially (true for any two reals). The non-trivial content is that the common ratio is **$2\sqrt{G^*}$** — this is the master quadratic's arithmetic imprint on the mean hierarchy.

**Why three means in GP matters:** it means the two roots $x_\pm$ are structured as "a pair whose means scale by $2\sqrt{G^*}$ per level." This is the geometric signature of the pair $(x_+, x_-)$ being determined by $G^*$ alone.

## 4. The normalized-form perturbation theory

In normalized form $w^2 - w + \epsilon = 0$ with $\epsilon = 1/(16 G^*) \approx 0.0211$:

**Exact roots:**
$$ w_\pm = \frac{1 \pm \sqrt{1 - 4\epsilon}}{2} $$

**Expansion for small $\epsilon$:**

$$ w_+ = 1 - \epsilon - \epsilon^2 - 2\epsilon^3 - 5\epsilon^4 - \ldots $$
$$ w_- = \epsilon + \epsilon^2 + 2\epsilon^3 + 5\epsilon^4 + \ldots $$

(Coefficients are the Catalan numbers $C_{n-1}$: 1, 1, 2, 5, 14, ...)

Converting back to $x = 16 G^{*2} w$:

$$ x_+ = 16 G^{*2}\left(1 - \epsilon - \epsilon^2 - 2\epsilon^3 - \ldots\right) = 16 G^{*2} - G^* - \frac{1}{16} - \ldots $$

$$ x_- = 16 G^{*2}\left(\epsilon + \epsilon^2 + 2\epsilon^3 + \ldots\right) = G^* + \frac{1}{16} + \ldots $$

**What this reveals:**
- $x_+$ is dominated by $16 G^{*2} \approx 140$ with a $-G^*$ correction
- $x_-$ is dominated by $G^* \approx 2.96$ with a $+1/16$ correction (bringing it to $\approx 3.02$)
- Subsequent corrections are geometrically smaller by factor $\epsilon \approx 1/47$

**Leading-order statement:** $x_+ \approx 16 G^{*2}$ and $x_- \approx G^*$.

## 5. The companion identity

From V2, the reciprocals of the roots have product
$$ \frac{1}{x_+} \cdot \frac{1}{x_-} = \frac{1}{x_+ x_-} = \frac{1}{16 G^{*3}} $$

So $1/x_+$ and $1/x_-$ are two numbers with:
- **Sum** $= 1/G^*$ (from V3)
- **Product** $= 1/(16 G^{*3})$ (from V2)

Therefore:
$$ \frac{1}{x_+} + \frac{1}{x_-} = \frac{1}{G^*} \quad \text{AND} \quad \frac{1}{x_+} \cdot \frac{1}{x_-} = \frac{1}{16 G^{*3}} $$

The two numbers $1/x_+$ and $1/x_-$ are **themselves Vieta-paired** with a different quadratic:

$$ y^2 - \frac{1}{G^*} y + \frac{1}{16 G^{*3}} = 0 $$

This is the reciprocal-form master quadratic. Its roots are $(1/x_+, 1/x_-)$ rather than $(x_+, x_-)$.

## 6. The two master quadratics

There are two natural quadratics encoding the same content:

**Primary master quadratic** (large-number form):
$$ x^2 - 16 G^{*2} x + 16 G^{*3} = 0 \quad \Rightarrow \quad \text{roots } \{x_+, x_-\} $$

**Reciprocal master quadratic** (small-number form):
$$ y^2 - \frac{1}{G^*} y + \frac{1}{16 G^{*3}} = 0 \quad \Rightarrow \quad \{y_+, y_-\} = \{1/x_-, 1/x_+\} $$

These two polynomials are obtained from each other by the substitution $y = 1/x$ (modulo an overall scale). The "primary" is scaled by $16 G^{*3}$; the "reciprocal" is scaled by $1$.

The fact that the reciprocal form has coefficients $1/G^*$ and $1/(16 G^{*3})$ — simple inverses of the primary's coefficients — reflects that the polynomial is **invariant under the reflection $x \leftrightarrow 16 G^{*3}/x$** up to scale.

Specifically, if $x$ is a root, then $16 G^{*3}/x$ is also a root (by Vieta product). Check: $16 G^{*3}/x_+ = 16 \cdot 25.9/137.04 = 3.024 = x_-$ ✓.

**The two roots are reciprocals of each other (modulo $16G^{*3}$).**

## 7. What the master quadratic does NOT say

The polynomial contains no physics. Specifically, it does not determine:

- **Whether a root is a physical constant**: the identification $x_+ = 1/\alpha$ (FTD-0013) rests on numerical proximity, $x_+ \approx 137.036 \approx 1/\alpha$.
- **Whether these are the "right" physical constants at all**: that's the identification step.
- **Why the polynomial should exist at all**: that's the motivation chain (FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md).
- **Any specific tick-dynamics, lattice stencil, or simulation rule**: those are Axiom Zero postulates, independent of the master quadratic.

The polynomial is a **pure algebraic relation** between three objects: two roots and one parameter $G^*$. Its physical interpretation is a separate layer, and the present document is careful to keep them distinct.

## 8. The entire content in one table

| Name | Form | Content |
|---|---|---|
| Standard form | $x^2 - 16 G^{*2} x + 16 G^{*3} = 0$ | Monic polynomial with roots $x_\pm$ |
| Vieta V1 (sum) | $x_+ + x_- = 16 G^{*2}$ | Two numbers summing to $16 G^{*2}$ |
| Vieta V2 (product) | $x_+ \cdot x_- = 16 G^{*3}$ | ...with product $16 G^{*3}$ |
| **Vieta V3 (reciprocals)** | $\mathbf{1/x_+ + 1/x_- = 1/G^*}$ | ...reciprocal sum $1/G^*$ [the cleanest form] |
| Companion | $(1/x_+)(1/x_-) = 1/(16 G^{*3})$ | Reciprocal product |
| Normalized | $w^2 - w + \epsilon = 0$, $\epsilon = 1/(16G^*)$ | Small-parameter perturbation of $(w=1, w=0)$ |
| Means | AM : GM : HM = $8 G^{*2} : 4 G^{*3/2} : 2 G^*$ | Geometric progression with ratio $2\sqrt{G^*}$ |
| Reflection | $x \leftrightarrow 16 G^{*3}/x$ | Roots are images of each other under reciprocation-and-scale |
| Reciprocal quadratic | $y^2 - y/G^* + 1/(16 G^{*3}) = 0$ | Roots $1/x_+$ and $1/x_-$ |

## 9. Epistemic tags

| Piece | Tag |
|---|---|
| All Vieta identities V1, V2, V3 | **[THEOREM]** (exact by definition) |
| Three-means geometric progression | **[THEOREM]** (derivable from Vieta) |
| Normalized form $w^2 - w + \epsilon = 0$ | **[THEOREM]** (algebraic rearrangement) |
| Small-$\epsilon$ expansion with Catalan coefficients | **[THEOREM]** (generating function of $w_\pm$) |
| Reflection symmetry $x \leftrightarrow 16 G^{*3}/x$ | **[THEOREM]** (immediate from Vieta product) |
| Physics identification $x_+ = 1/\alpha$ | [STRONGLY MOTIVATED CONJECTURE] (FTD-0013) |

**All of §1–6 is exact algebra.** No physics is invoked. Physics enters only through the identification $x_+ = 1/\alpha$.

## 10. What this decomposition accomplishes

**Surface form:** the master quadratic can be read as a specific polynomial with numerical roots 137.036 and 3.024.

**Bare structure:** the polynomial is the algebraic encoding of a single relation

$$ \frac{1}{x_+} + \frac{1}{x_-} = \frac{1}{G^*} $$

Everything else — the factor 16, the two roots, the geometric-progression means, the normalized form — is consequence of this single identity combined with the product relation $(1/x_+)(1/x_-) = 1/(16 G^{*3})$.

**This is the master quadratic, unpacked.** The algebra is simple; the content is one line.

---

*Breaks the master quadratic into its bare algebraic parts, identifies the harmonic identity $1/x_+ + 1/x_- = 1/G^*$ as the single deepest statement, verifies all identities to machine precision via `test_master_quadratic_identities`, and preserves the distinction between pure algebra (exact) and physical identification ($x_+ = 1/\alpha$).*
