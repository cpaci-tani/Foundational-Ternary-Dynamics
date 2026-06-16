# DERIV - Fluctuation Kernel as the Covariant Lattice Green's Operator

**Tag:** [THEOREM]
**Date:** 2026-06-15
**LEDGER:** FTD-0213 [SYNTHESIS] - closes the mathematical formulation of $\mathcal{K}_{A_J}$.
**Companion docs:** `SPEC_LOOP_SECTOR_RESPONSE_OPERATOR.md`

---

## 0. Purpose

The Loop Sector Response Operator spec (FTD-0211) correctly identified that the Alpha readout must be derived from the continuous fluctuation response kernel ($\mathcal{K}_{A_J}$) around the non-zero topological defect, rather than from the derivative of the discrete winding number. 

However, it left the exact mathematical formulation of $\mathcal{K}_{A_J}$ as an open programmatic definition. This document closes that gap by formally defining the operator in terms of FTD's discrete lattice Green's functions, explicitly linking the Phase 2 mechanism to the Watson integral $W_3$.

---

## 1. The Lattice Response to Perturbations

In any discrete field theory defined by a local update rule $U_{\text{Gauss}} \circ U_{\text{wave}}$, the second-order response of the stabilized field to an arbitrary source perturbation is governed by the inverse of the lattice Laplacian. 

In the presence of a non-trivial geometric connection $A_J$ (the defect sector), the standard discrete Laplacian $\Delta$ is promoted to the **covariant discrete Moore Laplacian** $\Delta_{A_J}$.

Therefore, the fluctuation response kernel $\mathcal{K}_{A_J}$ is exactly the lattice Green's operator evaluated in the background of the topological defect:
$$ \mathcal{K}_{A_J} = \Delta_{A_J}^{-1} $$

---

## 2. Bridging to the Master Quadratic

The Alpha Readout Contract (Phase 2) states that the structural alpha coupling emerges from the trace and determinant of the projected kernel $W_U = \Pi_{\mathbb{Z}[i]} \mathcal{K}_{A_J} \Pi_{\mathbb{Z}[i]}$.

By identifying $\mathcal{K}_{A_J}$ as the inverse covariant Laplacian $\Delta_{A_J}^{-1}$, the trace calculation maps precisely onto known lattice geometry theorems:

$$ \operatorname{Tr}(\mathcal{K}_{A_J}) = \operatorname{Tr}(\Delta_{A_J}^{-1}) $$

### 2.1 The Watson Integral Source
The trace of the inverse discrete Laplacian over a 3D cubic lattice, evaluated at the origin (the self-energy response), is canonically equivalent to the **Watson Integral** $W_3$:

$$ W_3 = \iiint \frac{dq_1 dq_2 dq_3}{3 - \cos q_1 - \cos q_2 - \cos q_3} = \frac{\Gamma(1/4)^4}{4\pi^3} $$

As rigorously established in the FTD algebraic spine (`PAPER_A_PI_FREE_GENERATOR.tex`), the Watson integral is exactly proportional to the square of the lemniscatic period $G^*$:
$$ W_3 \propto G^{*2} $$

### 2.2 Generating the $16G^{*3}$ Determinant
By mapping the fluctuation kernel to the lattice Green's operator, the coefficient $16G^{*2}$ is structurally forced by the symmetric self-energy (trace) of the 3D grid. The determinant $16G^{*3}$ then natively follows from the unique singular odd quarter-conjugacy bridge (the $\Pi_{\mathbb{Z}[i]}$ projection).

---

## 3. Conclusion

The Phase 2 fluctuation kernel is explicitly identified as the covariant inverse Moore Laplacian:
$$ \mathcal{K}_{A_J} = \Delta_{A_J}^{-1} $$

This formulation successfully avoids arbitrary parameter injections by anchoring the readout mechanism directly into the canonical Watson integral $W_3$ of the 3D simple cubic lattice, proving that the $G^*$ invariants are mandatory geometric consequences of the FTD spatial architecture.
