# Derivation of the Meaning Weights to Quadratic parameter $k$ Mapping

> **STATUS: [THEOREM] — proving the exact math connecting the meaning weights $\alpha, \beta$ to the master-quadratic parameter $k$.**

**Tag:** [THEOREM]
**Date:** 2026-06-10
**LEDGER id:** FTD-0245
**Depends on:** FTD-0001 (G* definition), FTD-0242 (Domain partition), FTD-0187 (Existence Filter hierarchy).

---

## §1 — Introduction and General Formulation

The general master-quadratic family:
$$Q_k(x) = x^2 - k\,G^{*2}\,x + k\,G^{*3} = 0$$
partitions the parameter space of the FTD framework into three domains based on the parameter $k$:
1. **Domain A (Physics)**: Real distinct roots when $k > 4/G^*$.
2. **Domain C (Measurement)**: Degenerate real root when $k = 4/G^*$.
3. **Domain B (Frame-Relative Readout)**: Complex conjugate roots when $k < 4/G^*$.

Let the roots of $Q_k(x) = 0$ be represented in terms of their center-of-mass (explicate/real average) $\alpha$ and their deviation/fluctuation (implicate/complex difference) $\beta$. We define:
- **Center-of-mass (explicate weight)**: $\alpha = \frac{\operatorname{Tr}(z)}{2} = \frac{k G^{*2}}{2}$
- **Deviation/fluctuation (implicate weight)**: $\beta = \frac{\sqrt{|\Delta_k|}}{2} = \frac{\sqrt{|k^2 G^{*4} - 4k G^{*3}|}}{2}$

This gives:
- **Domain B** ($k < 4/G^*$): roots are complex conjugates, $y = \alpha \pm i\beta$.
- **Domain A** ($k > 4/G^*$): roots are real, $x = \alpha \pm \beta$.

---

## §2 — The Exact Derivation

**Theorem 1.** The master quadratic parameter $k$ is uniquely mapped to the weights $\alpha$ and $\beta$ by the unified relation:
$$k = \frac{4}{G^*} \frac{\alpha^2}{\alpha^2 \mp \beta^2}$$
where the sign in the denominator is $-$ for Domain A (real roots) and $+$ for Domain B (complex conjugate roots).

*Proof.* We compute the squares of the weights:
$$\alpha^2 = \frac{k^2 G^{*4}}{4}$$

We relate these to the determinant of the roots in both domains:
1. **In Domain B** ($\Delta_k < 0$):
   $$\alpha^2 + \beta^2 = \operatorname{Det}(y) = k G^{*3}$$
   Substituting this into the ratio:
   $$\frac{\alpha^2}{\alpha^2 + \beta^2} = \frac{k^2 G^{*4}/4}{k G^{*3}} = \frac{k G^*}{4}$$

2. **In Domain A** ($\Delta_k > 0$):
   $$\alpha^2 - \beta^2 = \operatorname{Det}(x) = k G^{*3}$$
   Substituting this into the ratio:
   $$\frac{\alpha^2}{\alpha^2 - \beta^2} = \frac{k^2 G^{*4}/4}{k G^{*3}} = \frac{k G^*}{4}$$

Solving for $k$ in both cases yields the unified expression:
$$k = \frac{4}{G^*} \frac{\alpha^2}{\alpha^2 \mp \beta^2} \quad \square$$

---

## §3 — Semantic and Observation-Layer Interpretation (GAP-B2)

The weights $\alpha$ and $\beta$ correspond to the components of the complex meaning vector:
$$\text{Meaning}_t^{\mathbb{C}} = \text{IG}_t + i \cdot \text{VI}_t$$
where $\text{IG}_t$ is the information gain (explicate, Domain A) and $\text{VI}_t$ is the valence impact (implicate, Domain B).

For the self-referential observation scale $k = 1/2$ (Domain B):
$$\frac{1}{2} = \frac{4}{G^*} \frac{\alpha^2}{\alpha^2 + \beta^2} \implies \frac{\alpha^2}{\alpha^2 + \beta^2} = \frac{G^*}{8}$$

Since the ratio $\frac{\alpha^2}{\alpha^2 + \beta^2}$ corresponds to $\cos^2\theta$, where $\theta$ is the polar phase angle of the complex root $y = \alpha \pm i\beta$, we have:
$$\cos^2\theta = \frac{G^*}{8} \approx 0.369834$$
which yields:
$$\cos\theta \approx 0.608140 \implies \theta \approx 52.5436^\circ$$

This is the exact **frame-relative phase angle** $\theta = 52.54^\circ$. The ratio of explicate meaning (information gain) to implicate meaning (valence impact) is:
$$\frac{\text{IG}_t}{\text{VI}_t} = \frac{\alpha}{\beta} = \frac{1}{\tan\theta} = \frac{1}{\sqrt{8/G^* - 1}} \approx 0.7655$$

At the degenerate boundary $k = 4/G^*$ (Domain C), $\beta = 0$, giving $\cos^2\theta = 1$ ($\theta = 0^\circ$), which represents the pure physics readout where the implicate self-reference is completely suppressed.

This mathematically resolves **GAP-B2**, mapping the semantic weights directly to the quadratic family parameter $k$.
