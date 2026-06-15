# SPEC - Closed Flux-Loop Readout Formalism

**Tag:** [SUPERSEDED]
**Date:** 2026-06-15
**LEDGER:** FTD-0207 [SYNTHESIS] - defines the focused topological candidate for alpha readout and the two-phase proof obligation.

> **[SUPERSEDED]:** This static geometric/topological approach was falsified by direct engine evaluation. The Phase 2 operator failed the "Gauge Trap" test. This entire formalism is replaced by the dynamical return-map architecture in `SPEC_ALPHA_READOUT_OSCILLATORY_CLOUD.md`.
**Companion docs:** `SPEC_ALPHA_READOUT_CONTRACT.md`, `SPEC_EM_PUBLIC_OBSERVABLE_CLASSIFICATION.md`

---

## 0. Purpose

The Alpha Readout Contract (v2.0 Operational Formalization) bounds the electromagnetic readout $W_U$ to four deterministic objects: $\Omega_{\min}$, $W_L$, $\mathcal{B}_{\mathbb{Z}[i]}$, and $R_{\text{EM}}$. The Electromagnetic Public Observable Classification falsifies five geometric readout channels, leaving dynamically coupled flux-loops as the only currently identified viable mechanism.

This document formally defines the candidate observable $\mathcal{L}_C[J,s]$ and structures the required proof into two rigorous phases, acknowledging a critical risk: the longitudinal holonomy obstruction.

---

## 1. Phase 1: The Longitudinal Holonomy Obstruction

To survive the FTD-0204 plaquette-bivector no-go, the flux-loop observable must be genuinely nonlocal and dynamically coupled. A naive formulation defines it as the closed contour circulation of the dynamically relaxed flux state $J_{\infty}$:
$$ \mathcal{L}_C[J] = \oint_C J_{\infty} \cdot d\ell $$

**The Obstruction:** If the relaxed Gauss solver drives the field $J_{\infty}$ to a pure curl-free longitudinal gradient ($J_{\infty} = -\nabla \phi$), then by definition $\oint_C \nabla \phi \cdot d\ell = 0$. This would kill the observable before it can even be evaluated.

### 1.1 The Preliminary Proof Obligation
The immediate hurdle for this program is to answer:
> **Does the actual FTD Gauss solver force $\oint_C J_{\infty} \cdot d\ell \neq 0$ for a minimal neutral source?**

If the answer is **zero**, the naive loop route closes negative, and the derivation must pivot to one of three structural contingencies.

### 1.2 Structural Contingencies
If the naive longitudinal holonomy vanishes, the topological readout must be reformulated into one of the following surviving sectors:

*   **Option C: True Connection Holonomy (Primary Candidate):** 
    Treating the physical field $J$ not as the connection, but extracting an FTD-native connection $A_J$ directly from the $\mathbb{Z}[i]$-module / quadrature structure. The observable becomes a genuine topological holonomy: $\mathcal{L}_C = \oint_C A_J \cdot d\ell$. This is mathematically the strongest path to bridging the lemniscatic curve invariants.
*   **Option A: Transverse-Loop Projection:**
    Defining the loop strictly on the transverse/curl sector ($J_{\infty} = J_L + J_T$). The observable becomes $\mathcal{L}_C = \oint_C J_T \cdot d\ell$. This requires proving that the minimal neutral excitation forces a non-zero relaxed transverse circulation.
*   **Option B: Surface Dipole Flux:**
    Shifting from line circulation to surface flux: $\Phi_{\Sigma}[J] = \int_{\Sigma} J_{\infty} \cdot dS$. For a neutral source ($\sum s = 0$), the net flux vanishes unless the observable explicitly separates internal dipole structure.

---

## 2. Phase 2: The Decisive Readout (The $G^*$ Bridge)

Only if a non-zero topological formulation survives Phase 1 (whether via a non-trivial naive integral, an extracted connection $A_J$, or a transverse circulation), does the proof proceed to the decisive target.

> [!WARNING]
> **Topological Derivative Nullification:** If Phase 1 survives via an integer topological winding number ($2\pi n$), the continuous field derivative of the observable vanishes identically ($D\mathcal{L}_C = 0$).

Therefore, the topological holonomy acts strictly as the **Sector Selector**. It proves the sector is open. The actual alpha readout must be extracted from the **Loop Sector Response Operator** ($\mathcal{K}_{A_J}$), representing the continuous spectrum of fluctuations *around* the defect.

(See `SPEC_LOOP_SECTOR_RESPONSE_OPERATOR.md` for the explicit construction of $W_U = \Pi_{\mathbb{Z}[i]} \mathcal{K}_{A_J} \Pi_{\mathbb{Z}[i]}$).

### 2.1 The Master Quadratic Target
To identify the dominant structural eigenvalue $\lambda_+$ with the physical electromagnetic fine-structure constant $\alpha^{-1}$, the extracted 2-by-2 response matrix $W_U$ evaluated in the defect sector must organically generate the exact coefficients of the FTD master quadratic:
$$ \chi_W(\lambda) = \lambda^2 - 16G^{*2}\lambda + 16G^{*3} $$

> [!IMPORTANT]
> **Does the projected fluctuation kernel $\mathcal{K}_{A_J}$ force exactly one odd $G^*$ quarter-conjugacy bridge?**

*   **Trace ($16G^{*2}$):** Does the symmetric self-energy of the fluctuation kernel yield the Watson-integral reflection value?
*   **Determinant ($16G^{*3}$):** Does the structure uniquely produce the product of an even self-energy channel and a singular odd quarter-conjugacy bridge?

### Status
This target remains an **[OPEN PROGRAM]**. 
- If Phase 1 falsifies all loop formulations, the Alpha Readout Program hits a definitive dead end in the current topological geometry.
- If Phase 1 isolates a non-zero sector, and Phase 2's fluctuation operator confirms the master quadratic coefficients, the identification $x_+ = \alpha^{-1}$ transitions from strongly motivated conjecture to physical theorem.
