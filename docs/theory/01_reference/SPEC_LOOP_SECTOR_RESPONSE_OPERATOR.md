# SPEC - Loop Sector Response Operator

**Tag:** [SUPERSEDED]
**Date:** 2026-06-15
**LEDGER:** FTD-0211 [SYNTHESIS] - replaces the dead derivative-of-winding math with the fluctuation response operator formulation.

> **[SUPERSEDED]:** The spatial fluctuation response operator over a static loop sector was explicitly falsified by an engine-driven numerical check. The `[OPEN PROGRAM]` for the Alpha Readout target operator has been migrated to the dynamical return-map formulation defined in `SPEC_ALPHA_READOUT_OSCILLATORY_CLOUD.md`.
**Companion docs:** `SPEC_CLOSED_FLUX_LOOP_READOUT.md`, `SPEC_CONNECTION_EXTRACTION_RULE.md`

---

## 0. Purpose

The Connection Extraction Rule (FTD-0209) established that the Phase 1 topological holonomy $\mathcal{L}_C = \sum A_J$ takes the form of an integer winding number ($2\pi n$). 

While passing Phase 1, this mathematical fact creates a fatal obstruction for the original Phase 2 formulation. Because integer winding is locally constant under small perturbations, its continuous derivative is identically zero ($D\mathcal{L}_C \equiv 0$). Therefore, linearizing the winding number itself yields a zero matrix, which can never produce the structural master quadratic coefficients ($16G^{*2}, 16G^{*3}$).

This document formally corrects Phase 2: The topological winding is **not** the $\alpha$-readout observable. It is the **Sector Selector**. The actual observable is the **fluctuation response operator** around that topological defect.

---

## 1. Phase 1 as the Sector Selector

The integer topological winding $n_C \neq 0$ serves a singular mathematical purpose in the alpha readout program: it proves that the minimal neutral source $\Omega_{\min}$ opens a non-trivial topological sector that escapes the longitudinal holonomy obstruction ($d\phi = 0$).

Once $n_C \neq 0$ is proven, the raw value of the winding number is discarded. The derivation must shift focus entirely to the continuous geometry of the field *inside* that non-zero topological sector.

---

## 2. The Fluctuation Response Kernel ($\mathcal{K}_{A_J}$)

The alpha-relevant mathematical object is the second-order continuous response spectrum of fluctuations around the topological defect. 

We define the **Loop-Sector Fluctuation Kernel** $\mathcal{K}_{A_J}^{(n_C \neq 0)}$. This kernel measures the transfer response of the connection field to continuous perturbations induced by the operational sequence $U_{\text{Gauss}} \circ U_{\text{wave}}$ operating inside the non-zero holonomy background.

> [!WARNING]
> **Resolved Formulation:** The mathematical formulation of $\mathcal{K}_{A_J}$ has been explicitly identified as the covariant inverse Moore Laplacian $\Delta_{A_J}^{-1}$. See `DERIV_FLUCTUATION_KERNEL_GREEN.md` for the proof linking its trace to the geometric Watson integral $W_3$.

---

## 3. The Alpha Readout Operator ($W_U$)

The target 2-by-2 response operator $W_U$ defined by the Alpha Readout Contract is extracted by projecting the fluctuation kernel into the forced quarter-conjugate two-channel perturbation directions ($\eta_1, \eta_2$).

$$ W_U = \Pi_{\mathbb{Z}[i]} \, \mathcal{K}_{A_J}^{(n_C \neq 0)} \, \Pi_{\mathbb{Z}[i]} $$

where:
*   $\Pi_{\mathbb{Z}[i]}$ represents the projection onto the intrinsic $\mathbb{Z}[i]$-module structural basis.
*   The matrix elements are strictly constructed as $\left\langle \eta_a, \mathcal{K}_{A_J}, \eta_b \right\rangle$.

---

## 4. The Revised Phase 2 Decisive Target

The trace and determinant tests are applied exclusively to this projected fluctuation operator, not the topological winding.

$$ T_L = \operatorname{Tr} W_U \to 16G^{*2} $$
$$ D_L = \det W_U \to 16G^{*3} $$

> [!CAUTION]
> **Status: [OPEN PROGRAM]**
> The architecture of Phase 2 is now mathematically coherent, but the quantitative proof is completely open. The Alpha Readout cannot be claimed as a theorem until the following two exact calculation bottlenecks are resolved.

### 4.1 Proof Obligation 1: The Kernel Normalization
The trace of the discrete inverse Moore Laplacian $\operatorname{Tr}(\Delta_{A_J}^{-1})$ evaluates natively to the Watson integral $W_3 \approx 1.3932$. However, the target trace is $16G^{*2} \approx 140.06$. 

The relationship is approximately $16G^{*2} = 32\pi W_3$. This scaling factor cannot be inserted by hand. A successful Phase 2 proof must structurally derive this exact factor from the lattice measure, the loop orientation sum, the $\mathbb{Z}[i]$ projection, or the fundamental phase-law normalization.

### 4.2 Proof Obligation 2: Spectral Non-Triviality (The Gauge Trap)
If the holonomy is exactly $2\pi n$, a compact $U(1)$ theory can gauge-transform it away everywhere except exactly at the defect core. If $\Delta_{A_J}$ is globally gauge-equivalent to the trivial Laplacian $\Delta$, its spectrum does not change.

A successful Phase 2 proof must demonstrate that the defect remains *physically active* (e.g., via a branch cut, an excluded origin site, or a forced boundary condition) such that the spectrum of $\Delta_{A_J}^{-1}$ breaks the trivial symmetry and produces the required odd quarter-conjugacy $G^*$ determinant bridge. If the spectrum is identical to the vacuum, the alpha readout fails.
