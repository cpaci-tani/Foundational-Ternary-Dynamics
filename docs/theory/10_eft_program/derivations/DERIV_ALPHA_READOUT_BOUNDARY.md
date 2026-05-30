# Derivation — Alpha Readout Boundary-Condition Closure (ARC-A1 v2)

**Tag:** [UNDERDETERMINED] (Trace forced by modular invariant; Determinant mathematically unforced)
**Date:** 2026-05-30
**Framework:** Foundational Ternary Dynamics v5.33
**Reference:** `PREREG_ALPHA_READOUT_BOUNDARY_v2.md`
**LEDGER:** FTD-0239 (ARC-A1 v2 Execution)

---

## 0. Executive Summary

This document executes the mathematical derivation for the **ARC-A1 (Boundary-Condition Readout)** hypothesis, modeling the physical boundary of the FTD lattice as a 2D modular torus $T^2$. 

By imposing $PSL(2, \mathbb{Z})$ modular invariance on the discrete ternary partition amplitude $Z_S(\tau)$, we successfully lock the boundary parameter to the stable lemniscatic fixed point $\tau = i$. At this fixed point, the characteristic variance of the boundary flux field is strictly proportional to $G^{*2}$, cleanly deriving the trace of the master quadratic ($16 G^{*2}$) without inserting CODATA values.

However, the master quadratic's determinant ($16 G^{*3}$) requires an *odd* power of the fundamental period $G^*$. Our mathematical analysis (verified via `scripts/proofs/proof_alpha_readout_boundary.py`) proves that modular forms and theta functions on the torus (e.g., Eisenstein series $E_4, E_6$) exclusively generate *even* powers of $G^*$ (i.e., $G^{*2}, G^{*4}$). Therefore, no 2D boundary transition amplitude can structurally force the odd-powered determinant without inserting an external dimensional scale.

**Verdict:** The ARC-A1 path is **[UNDERDETERMINED]**. It elegantly maps the trace to the modular geometry, but fails to recover the full master quadratic.

---

## 1. The Modular Boundary Amplitude

We define the discrete boundary transition amplitude $Z_S(\tau)$ as the partition sum over all ternary configurations restricted to the 2D surface $S$ parametrized by $\tau$:
$$ Z_S(\tau) = \sum_{\{s \in -1,0,1\}} \exp(-S_{\text{boundary}}) $$

To ensure self-consistency across the macroscopic boundary, $Z_S(\tau)$ must be invariant under the modular group $PSL(2, \mathbb{Z})$ transformations $\tau \to \tau+1$ and $\tau \to -1/\tau$.

The only strictly stable fixed point under the S-transformation ($\tau \to -1/\tau$) in the fundamental domain is the lemniscatic fixed point:
$$ \tau = i $$

This provides a mathematically rigid, non-empirical selection mechanism for the boundary parameter, fulfilling Gate 2 of the "No-Cheat" checklist.

---

## 2. Extraction of the Characteristic Variance

At the fixed point $\tau = i$, the geometry of the torus is square, isomorphic to the $\mathbb{Z}[i]$ lattice. The characteristic invariants of this geometry are the lemniscatic periods, specifically $G^* = \Gamma(1/4)/\Gamma(3/4)$.

The variance (or 2-point Green's function) of the flux field on this boundary evaluates exactly to the lattice Watson integral, scaling as $G^{*2}$. 
Normalizing by the automorphism group size of the complex structure ($|\mu_4|^2 = 16$), the trace of the associated transfer operator is strictly forced:
$$ \text{Tr}(T) = 16 G^{*2} $$

This successfully derives the $x$ coefficient of the master quadratic purely from boundary modular stability.

---

## 3. The Odd-Power Obstruction (Determinant Failure)

To fully recover the master quadratic $x^2 - 16 G^{*2} x + 16 G^{*3} = 0$, the determinant must evaluate to $16 G^{*3}$.

In the theory of modular forms on the torus at $\tau=i$:
*   The Eisenstein series $E_4(i)$ is proportional to $G^{*4}$ (an even power).
*   The Eisenstein series $E_6(i) = 0$.
*   Any algebraically generated invariant from the partition amplitude will scale as an *even* power of the period ($G^{*2n}$).

There is no native modular object or boundary invariant that scales as $G^{*3}$. To obtain $G^{*3}$, one must explicitly multiply the trace ($16 G^{*2}$) by the asymmetric regularized period $G^*$. But in the context of the boundary transition amplitude, there is no geometric mechanism that couples the variance to a single asymmetric period without an external phenomenological insertion.

---

## 4. Evaluation Against the "No-Cheat" Checklist

| Gate | Criterion | Status | Notes |
|---|---|---|---|
| **Gate 1** | No CODATA input | **PASS** | $137.036$ never appears. Construction relies on modular fixed points. |
| **Gate 2** | No scheme tuning | **PASS** | $\tau=i$ is the unique stable S-transformation fixed point. |
| **Gate 3** | No auxiliary gauge fields | **PASS** | The sum is defined over the native ternary boundary configurations. |
| **Gate 4** | Explicit mapping | **FAIL** | Derives the trace ($16 G^{*2}$), but the determinant ($16 G^{*3}$) is unforced because modular invariants produce only even powers of $G^*$. |

---

## 5. Conclusion

The **ARC-A1 (Boundary-Condition)** route suffers from the exact same mathematical obstruction as the ARC-B2 and ARC-C1 routes: the native geometry of the FTD lattice rigorously generates *even* powers of the lemniscatic period $G^*$, but the target master quadratic requires an *odd* power for its determinant. 

The Alpha Readout bottleneck (MC-T4.3) cannot be solved via purely geometric or modular mathematical derivations without violating the non-circularity constraint. The master quadratic remains a **[FOUNDATIONAL OBSTRUCTION]**.
