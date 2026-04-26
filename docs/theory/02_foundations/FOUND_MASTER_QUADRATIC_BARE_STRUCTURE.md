# Foundation: The Master Quadratic — Bare Algebraic Structure

**Date:** 2026-04-24
**Status:** [THEOREM] (all identities below are exact by Vieta; verified to machine precision)
**Purpose:** Strip the master quadratic to its bare mathematical content. What does the underlying math actually say — without physics, without interpretation, just algebra?
**Ledger row:** FTD-0082
**Test:** `engine/tests/test_master_quadratic_identities.cpp` (ctest `master_quadratic_identities`, all identities verified to 1e-15)
**Companion:** [FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md](FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md) — why the polynomial has this form

---

## 0. Executive summary

The master quadratic
$$ x^2 - 16 G^{*2} x + 16 G^{*3} = 0 $$
says, in bare algebraic terms: **there are two positive real numbers $x_+, x_-$ whose sum is $16 G^{*2}$ and whose product is $16 G^{*3}$.** By Vieta, these two conditions are equivalent to a third:

$$ \boxed{\ \frac{1}{x_+} + \frac{1}{x_-} = \frac{1}{G^*}\ } $$

Under the physical identification (SP4) $x_+ = 1/\alpha$ and $x_- = N_c$, this becomes:

$$ \boxed{\ \alpha + \frac{1}{N_c} = \frac{1}{G^*}\ } $$

**This single line is the deepest content of the master quadratic.** Everything else is algebra on top of it.

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
  α + 1/x−  = 0.337989120033643
  1/G*      = 0.337989120033642
  |diff|    = 8.3e-16     (= floating-point epsilon)
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

$$ \frac{1}{\alpha} = x_+ = 16 G^{*2} - G^* - \frac{1}{16} - \frac{1}{256 G^*} - \ldots $$

$$ N_c = x_- = G^* + \frac{1}{16} + \frac{1}{256 G^*} + \frac{2}{4096 G^{*2}} + \ldots $$

**What this reveals:**
- $1/\alpha$ is dominated by $16 G^{*2} \approx 140$ with a $-G^*$ correction
- $N_c$ is dominated by $G^* \approx 2.96$ with a $+1/16$ correction (bringing it to $\approx 3.02$)
- Subsequent corrections are geometrically smaller by factor $\epsilon \approx 1/47$

**Leading-order statement:** $1/\alpha \approx 16 G^{*2}$ and $N_c \approx G^*$. The integer "3" for color count is $G^*$ rounded up plus a small lattice correction.

## 5. The single deepest identity (under SP4)

Accepting the physical identification $x_+ = 1/\alpha$, $x_- = N_c$ (selection principle SP4), the master quadratic's content compresses to:

$$ \boxed{\ \alpha + \frac{1}{N_c} = \frac{1}{G^*}\ } $$

**This is the entire physical content of the master quadratic in one line.**

### 5.1 What it says

The fine-structure constant $\alpha$ is the gap between $1/G^*$ and $1/N_c$:

$$ \alpha = \frac{1}{G^*} - \frac{1}{N_c} = \frac{N_c - G^*}{N_c \cdot G^*} $$

Since $N_c \approx 3.024$ and $G^* \approx 2.959$ differ by $\approx 0.065$, and their product is $\approx 8.94$:

$$ \alpha \approx \frac{0.065}{8.94} \approx 0.00727 $$

Matching experimental $\alpha = 0.00730$ to within 0.4%. (Under SP4 strict, $N_c = x_-$ exactly, and the identity is exact to machine precision.)

### 5.2 What it says physically

**The EM coupling $\alpha$ is the arithmetic residue between inverse color count and the lemniscatic constant's reciprocal.** EM and QCD are not independent — they are the two complementary shares of the lattice's intrinsic $1/G^*$.

If $G^*$ is the "total arithmetic scale" of the Gaussian-integer lattice and $1/N_c$ is QCD's share, then $\alpha$ is EM's share. Their sum is the whole.

### 5.3 The companion identity

From V2:
$$ \alpha \cdot N_c = \frac{1}{16 G^{*3}} $$

So $\alpha$ and $1/N_c$ are two numbers with:
- **Sum** $= 1/G^*$ (from V3)
- **Product** $= \alpha/N_c = 1/(16 G^{*3} \cdot N_c^2) $ — wait, let me recompute. $\alpha \cdot N_c = 1/(x_+ x_-) \cdot x_- \cdot x_- = x_-/x_+$... hmm not clean. Let me just use Vieta directly.

$(1/x_+)(1/x_-) = 1/(x_+ x_-) = 1/(16 G^{*3})$

So $\alpha \cdot (1/N_c) = 1/(16 G^{*3})$.

Therefore:
$$ \alpha + \frac{1}{N_c} = \frac{1}{G^*} \quad \text{AND} \quad \alpha \cdot \frac{1}{N_c} = \frac{1}{16 G^{*3}} $$

The two numbers $\alpha$ and $1/N_c$ are **themselves Vieta-paired** with a different quadratic:

$$ y^2 - \frac{1}{G^*} y + \frac{1}{16 G^{*3}} = 0 $$

This is the reciprocal-form master quadratic. Its roots are $(\alpha, 1/N_c)$ rather than $(1/\alpha, N_c)$.

## 6. The two master quadratics

There are two natural quadratics encoding the same physical content:

**Primary master quadratic** (large-number form):
$$ x^2 - 16 G^{*2} x + 16 G^{*3} = 0 \quad \Rightarrow \quad \{x_+, x_-\} = \{1/\alpha, N_c\} $$

**Reciprocal master quadratic** (small-number form):
$$ y^2 - \frac{1}{G^*} y + \frac{1}{16 G^{*3}} = 0 \quad \Rightarrow \quad \{y_+, y_-\} = \{1/N_c, \alpha\} $$

These two polynomials are obtained from each other by the substitution $y = 1/x$ (modulo an overall scale). The "primary" is scaled by $16 G^{*3}$; the "reciprocal" is scaled by $1$.

The fact that the reciprocal form has coefficients $1/G^*$ and $1/(16 G^{*3})$ — simple inverses of the primary's coefficients — reflects that the polynomial is **invariant under $x \leftrightarrow G^{*3}/x$ reflection** up to scale.

Specifically, if $x$ is a root, then $16 G^{*3}/x$ is also a root (by Vieta product). Check: $16 G^{*3}/x_+ = 16 \cdot 25.9/137.04 = 3.024 = x_-$ ✓.

**The two roots are reciprocals of each other (modulo $16G^{*3}$).**

## 7. What the master quadratic does NOT say

The polynomial contains no physics. Specifically, it does not determine:

- **Which root is α and which is $N_c$**: that's SP4 (based on numerical proximity 137.036 ≈ 1/137.036).
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
| **SP4 reading** | $\mathbf{\alpha + 1/N_c = 1/G^*}$ | The entire physical content on one line |

## 9. Epistemic tags

| Piece | Tag |
|---|---|
| All Vieta identities V1, V2, V3 | **[THEOREM]** (exact by definition) |
| Three-means geometric progression | **[THEOREM]** (derivable from Vieta) |
| Normalized form $w^2 - w + \epsilon = 0$ | **[THEOREM]** (algebraic rearrangement) |
| Small-$\epsilon$ expansion with Catalan coefficients | **[THEOREM]** (generating function of $w_\pm$) |
| Reflection symmetry $x \leftrightarrow 16 G^{*3}/x$ | **[THEOREM]** (immediate from Vieta product) |
| SP4 identification $x_+ = 1/\alpha$, $x_- = N_c$ | [SELECTION] (the one physics assumption) |
| $\alpha + 1/N_c = 1/G^*$ as a physical statement | [THEOREM given SP4] |

**All of §1–6 is exact algebra.** No physics is invoked. The physics enters only at §5 (SP4 reading).

## 10. What this decomposition accomplishes

**Before:** the master quadratic was often presented as a specific polynomial with numerical roots 137.036 and 3.024.

**After:** the polynomial reveals itself as the algebraic encoding of a single relation

$$ \alpha + \frac{1}{N_c} = \frac{1}{G^*} $$

which says EM coupling + inverse color count = inverse lemniscatic ratio. Everything else — the factor 16, the two roots, the geometric-progression means, the normalized form — is consequence of this single identity combined with the product relation $\alpha \cdot N_c^{-1} = 1/(16 G^{*3})$.

**This is the master quadratic, unpacked.** The algebra is simple; the physical identification (SP4) is the only selection. The content is one line.

---

*Filed 2026-04-24. Breaks the master quadratic into its bare algebraic parts, identifies $\alpha + 1/N_c = 1/G^*$ as the single deepest statement, verifies all identities to machine precision via `test_master_quadratic_identities`, and preserves the distinction between pure algebra (exact) and physical identification (SP4).*
