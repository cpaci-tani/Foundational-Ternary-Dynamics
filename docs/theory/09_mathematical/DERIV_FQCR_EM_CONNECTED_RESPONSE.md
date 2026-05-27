# Derivation of the FQCR-EM Connected Response at Tree Level

**Tag:** [THEOREM] (for the mathematical limits), [SELECTION] (for the choice of FQCR-EM connected response variable structure)  
**Date:** 2026-05-27  
**Framework:** Foundational Ternary Dynamics v5.33  
**Authoritative Reference:** [`docs/theory/01_reference/SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md)  
**Companion Documents:** [`docs/theory/04_coupling/DERIV_ALPHA_PRECISION_FORMULA.md`](../04_coupling/DERIV_ALPHA_PRECISION_FORMULA.md), [`docs/theory/09_mathematical/EXPLR_FQCR_OBSERVER_TESTS_SUITE.md`](EXPLR_FQCR_OBSERVER_TESTS_SUITE.md)

---

## 0. Executive Summary

This document provides the formal mathematical derivation of the **tree-level limit of the FQCR-EM connected response variable $R_{\text{conn}}$**. 

We prove that in the long-distance / unperturbed infrared limit (modeled by the scale parameter $t \to \infty$), the connected response converges exactly to the identity:

$$ \lim_{t \to \infty} R_{\text{conn}}(t) = 1 $$

Substituting this tree-level limit back into the FQCR-EM dominant branch equation recovers the **unperturbed, bedrock master quadratic root** $x_+ \approx 137.036171458$:

$$ \alpha^{-1}_{\text{tree}} = 8 G^{*2} + 4 G^{*3/2}\sqrt{4G^* - 1} $$

We also derive the exact series expansions of the leading-order modular corrections, illustrating how finite-scale modular deviations act as loop corrections to shift the inverse fine structure constant toward the CODATA recommended value at the physical coupling point $t = 1$.

---

## 1. The FQCR-EM Dominant Branch and Connected Response

### Definition 1: The Master Branch Equation
Under the Finite Quarter-Conjugacy Recurrence (FQCR) transfer-matrix model, the dominant physical coupling branch is given by:

$$ \alpha^{-1}_{\text{FQCR}}(t) = 8 G^{*2} + 4 G^{*3/2}\sqrt{4G^* - R_{\text{conn}}(t)} $$

where $G^* = \Gamma(1/4)/\Gamma(3/4) \approx 2.9586751$ is the scaled lemniscatic reflection ratio.

### Definition 2: The Connected Response Variable
The connected response $R_{\text{conn}}(t)$ is defined as an additive combination of three structural terms:

$$ R_{\text{conn}}(t) = 1 + \lambda(4 i t) + A(t) $$

where:
1. **$1$** represents the canonical tree-level background identity.
2. **$\lambda(4 i t)$** is the elliptic modular lambda function (shape term), representing state-field modular shape deviations.
3. **$A(t)$** is the Eisenstein-anomaly flow term (projection anomaly), representing continuous projection anomaly flow.

---

## 2. Mathematical Asymptotics of the Components

We now analyze the behavior of the modular corrections $\lambda(4 i t)$ and $A(t)$ in the limit $t \to \infty$, where the modular nome $Q = e^{-2\pi t} \to 0$.

### 2.1 The Elliptic Modular Lambda shape term $\lambda(4 i t)$

The modular lambda function is expressed as:

$$ \lambda(\tau) = \left( \frac{\theta_2(0, q)}{\theta_3(0, q)} \right)^4 $$

where $q = e^{i \pi \tau}$ is the modular nome associated with $\tau$. 

For $\tau = 4 i t$, the nome is:

$$ q = e^{i \pi (4 i t)} = e^{-4 \pi t} = Q^2 $$

The Jacobi theta functions have the following standard expansions for $q \to 0$:

$$ \theta_2(0, q) = 2 q^{1/4} \left( 1 + q^2 + q^6 + q^{12} + \dots \right) $$
$$ \theta_3(0, q) = 1 + 2 \sum_{n=1}^{\infty} q^{n^2} = 1 + 2 q + 2 q^4 + 2 q^9 + \dots $$

Substituting $q = Q^2$:

$$ \theta_2(0, Q^2) = 2 Q^{1/2} \left( 1 + Q^4 + Q^{12} + \dots \right) $$
$$ \theta_3(0, Q^2) = 1 + 2 Q^2 + 2 Q^8 + \dots $$

Therefore, the ratio of the theta functions behaves asymptotically as:

$$ \frac{\theta_2(0, Q^2)}{\theta_3(0, Q^2)} = \frac{2 Q^{1/2} (1 + Q^4 + O(Q^{12}))}{1 + 2 Q^2 + O(Q^8)} = 2 Q^{1/2} \left( 1 - 2 Q^2 + Q^4 + O(Q^6) \right) $$

Raising this ratio to the fourth power yields:

$$ \lambda(4 i t) = 16 Q^2 \left( 1 - 2 Q^2 + Q^4 + O(Q^6) \right)^4 = 16 Q^2 \left( 1 - 8 Q^2 + O(Q^4) \right) $$

Expressing this in terms of $t$:

$$ \lambda(4 i t) = 16 e^{-4\pi t} - 128 e^{-8\pi t} + O(e^{-12\pi t}) $$

> **[THEOREM]** As $t \to \infty$, $e^{-4\pi t} \to 0$, forcing:
> 
> $$ \lim_{t \to \infty} \lambda(4 i t) = 0 $$

---

### 2.2 The Eisenstein-Anomaly Flow Term $A(t)$

The anomaly flow term is given by the logarithmic derivative of the projection anomaly partition function $\Psi(t)$:

$$ A(t) = \frac{1}{3} \partial_t \log \Psi(t) $$

where the $(4,6; 3,2)$ projected partition function is defined as:

$$ \Psi(t) = \prod_{n=1}^{\infty} \frac{(1 - Q^{4n})^6}{(1 - Q^{3n})^2}, \qquad Q = e^{-2\pi t} $$

Taking the logarithm of $\Psi(t)$:

$$ \log \Psi(t) = 6 \sum_{n=1}^{\infty} \log(1 - Q^{4n}) - 2 \sum_{n=1}^{\infty} \log(1 - Q^{3n}) $$

Differentiating with respect to $t$ (using $\partial_t Q = -2\pi Q$):

$$ \partial_t \log(1 - Q^k) = \frac{-k Q^{k-1}}{1 - Q^k} (-2\pi Q) = 2\pi k \frac{Q^k}{1 - Q^k} $$

Thus, the derivative of the logarithm is:

$$ \partial_t \log \Psi(t) = 12 \pi \sum_{n=1}^{\infty} \frac{4n Q^{4n}}{1 - Q^{4n}} - 4\pi \sum_{n=1}^{\infty} \frac{3n Q^{3n}}{1 - Q^{3n}} = 48\pi \sum_{n=1}^{\infty} \frac{n Q^{4n}}{1 - Q^{4n}} - 12\pi \sum_{n=1}^{\infty} \frac{n Q^{3n}}{1 - Q^{3n}} $$

Dividing by 3 yields the explicit expression for the anomaly flow:

$$ A(t) = 16\pi \sum_{n=1}^{\infty} \frac{n Q^{4n}}{1 - Q^{4n}} - 4\pi \sum_{n=1}^{\infty} \frac{n Q^{3n}}{1 - Q^{3n}} $$

Expanding this for $Q \to 0$:

$$ \frac{Q^{3n}}{1 - Q^{3n}} = Q^{3n} + O(Q^{6n}), \qquad \frac{Q^{4n}}{1 - Q^{4n}} = Q^{4n} + O(Q^{8n}) $$

The leading terms of the series are:

$$ A(t) = 16\pi \left( Q^4 + 2 Q^8 + O(Q^{12}) \right) - 4\pi \left( Q^3 + 2 Q^6 + 3 Q^9 + O(Q^{12}) \right) $$
$$ A(t) = -4\pi Q^3 + 16\pi Q^4 - 8\pi Q^6 + 32\pi Q^8 + O(Q^9) $$

Expressing this in terms of $t$:

$$ A(t) = -4\pi e^{-6\pi t} + 16\pi e^{-8\pi t} - 8\pi e^{-12\pi t} + O(e^{-16\pi t}) $$

> **[THEOREM]** As $t \to \infty$, $e^{-6\pi t} \to 0$ and $e^{-8\pi t} \to 0$, forcing:
> 
> $$ \lim_{t \to \infty} A(t) = 0 $$

---

## 3. The Tree-Level Limit and Master Quadratic Recovery

### Theorem 1: Convergence of the Connected Response
Combining the asymptotic limits of the shape and flow terms:

$$ \lim_{t \to \infty} R_{\text{conn}}(t) = 1 + \lim_{t \to \infty} \lambda(4 i t) + \lim_{t \to \infty} A(t) = 1 + 0 + 0 = 1 $$

This mathematically demonstrates that at tree level ($t \to \infty$), the connected response reduces exactly to the trivial background identity:

$$ R_{\text{conn}}^{\text{tree}} = 1 $$

### Corollary 1: Recovery of the Bedrock Master Quadratic Root
Substituting $R_{\text{conn}}^{\text{tree}} = 1$ back into the dominant branch equation:

$$ \alpha^{-1}_{\text{tree}} = 8 G^{*2} + 4 G^{*3/2}\sqrt{4G^* - 1} $$

This is exactly the larger root $x_+$ of the unperturbed bedrock master quadratic equation:

$$ x^2 - 16 G^{*2} x + 16 G^{*3} = 0 $$

Evaluating this numerically using $G^* \approx 2.958675119$ yields:

$$ \alpha^{-1}_{\text{tree}} = x_+ \approx 137.036171458 $$

which lies within $1.26\text{ ppm}$ of the experimental value $1/\alpha \approx 137.035999177$.

---

## 4. Leading-Order Modular Corrections and QED Loops

For a large but finite scale parameter $t$, we can expand the connected response around its tree-level limit:

$$ R_{\text{conn}}(t) = 1 + 16 e^{-4\pi t} - 4\pi e^{-6\pi t} + 16\pi e^{-8\pi t} - 128 e^{-8\pi t} + O(e^{-12\pi t}) $$
$$ R_{\text{conn}}(t) = 1 + 16 e^{-4\pi t} - 4\pi e^{-6\pi t} + 16(\pi - 8) e^{-8\pi t} + O(e^{-12\pi t}) $$

This asymptotic expansion illustrates how finite-scale modular deviations modify the tree-level background:

1. **Lepton / Gauge Shape Anomaly ($e^{-4\pi t}$):** The leading-order correction is positive, driven by the modular shape term $\lambda(4 i t) \approx 16 e^{-4\pi t}$.
2. **Vacuum Polarization Anomaly ($-e^{-6\pi t}$):** The secondary correction is negative, driven by the leading term of the Eisenstein anomaly flow $A(t) \approx -4\pi e^{-6\pi t}$.

These terms act as the FQCR equivalent of loop corrections (one-loop and higher), introducing scale-dependence (running) into the coupling constant $\alpha^{-1}(t)$ and shifting it from the tree-level $137.036171458$ to the physical value at $t = 1$.

---

## 5. Epistemic Status and Verification

- **[THEOREM]:** The limits $\lim_{t \to \infty} \lambda(4 i t) = 0$, $\lim_{t \to \infty} A(t) = 0$, and the resulting recovery of $x_+$ are rigorous mathematical theorems. They are verified numerically to high precision and are analytically exact.
- **[SELECTION]:** The choice of the additive response law $R_{\text{conn}} = 1 + \lambda + A$ is a selection-level parameterization. While it successfully reproduces the CODATA value at $t=1$, it remains one of several possible combinations of these primitives (see [`explore_fqcr_response_laws.py`](../../../scripts/exploration/explore_fqcr_response_laws.py)).
- **[SMC] (Strongly Motivated Conjecture):** The physical reading of the scale parameter $t$ as representing energy scale (RG running) and the identification of $\alpha^{-1}_{\text{FQCR}}(1)$ with the physical coupling remain conjectural.
