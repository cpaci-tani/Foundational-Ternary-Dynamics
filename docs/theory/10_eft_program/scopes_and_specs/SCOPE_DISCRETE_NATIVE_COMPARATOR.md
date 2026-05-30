# SCOPE — Discrete-Native Comparator Inventory (ARC-D1)

**Tag:** [SCOPING MEMO]
**Date:** 2026-05-30
**Framework:** Foundational Ternary Dynamics v5.33
**Reference:** `SPEC_ALPHA_READOUT_CONTRACT.md` (Candidate D)
**LEDGER:** FTD-0237 (ARC-D1 Scoping)

---

## 0. Executive Summary

The **ARC-D1** route attempts to bypass the continuous QFT-reconstruction step entirely. If the discrete ternary structure intrinsically regulates the interaction strength, then macroscopic engine-native observables (such as defect scattering, relaxation times, or cluster branching ratios) should natively exhibit the fine-structure constant ratio ($1/\alpha \approx 137.036$) without requiring a continuum action or Wilson-loop intermediary.

This scoping memo inventories the available dimensionless observables in the FTD C++ engine that have direct physical (QED) experimental comparators.

---

## 1. The ARC-D1 Contract

To close MC-T4.3 via ARC-D, we must specify:
1. An engine-native observable $O_{\text{eng}}$ that is strictly dimensionless.
2. A rigorous experimental comparator $O_{\text{exp}}$ (e.g., a cross-section ratio or anomalous magnetic moment).
3. Proof that $O_{\text{eng}}$ is calibration-invariant and $L$-stable (independent of lattice size $L$ as $L \to \infty$).
4. The direct relation $O_{\text{eng}} = O_{\text{exp}}(\alpha)$, yielding $x_+$.

---

## 2. Engine-Native Observable Inventory

Based on recent engine updates (`cluster_observables.h`, `cluster_genealogy.h`) and measurement campaigns, we have the following discrete-native candidate observables:

### 2.1 Cluster Fission-Fusion Branching Ratios
*   **Engine Hook:** `campaign_cluster_fission_fusion.cpp`
*   **Observable:** The ratio of non-radiative elastic scattering to radiative (fission) splitting of a stable cluster.
    $$ R_{\text{fission}} = \frac{\Gamma(\text{Cluster} \to A + B)}{\Gamma(\text{Cluster} \to \text{Cluster}^*)} $$
*   **QED Comparator:** In QED, the probability of emitting a soft bremsstrahlung photon during scattering is proportional to $\alpha$.
*   **Assessment:** Highly promising. Requires isolating a strictly stable discrete "soliton" cluster and bombarding it with minimal-flux perturbations. The branching ratio would natively extract $\alpha$ from purely combinatorial transition rates.

### 2.2 Topological Defect Relaxation Time (Thermalization)
*   **Engine Hook:** `campaign_cluster_relaxation.cpp`
*   **Observable:** The dimensionless relaxation time $\tau_{\text{rel}} \cdot \omega_{\text{char}}$ for a perturbed defect to return to its ground-state phase equilibrium.
*   **QED Comparator:** The decay width of an excited bound state (e.g., Positronium), which scales with $\alpha^5 m_e$ for ortho-positronium.
*   **Assessment:** Difficult to decouple from the mass scale $m_e$ and the arbitrary lattice tick-rate. Fails the "strict dimensionless" gate unless a natural frequency ratio is constructed.

### 2.3 Soliton Scattering Angles
*   **Engine Hook:** `test_soliton_sweeps.cpp`
*   **Observable:** The discrete angular deflection $\Delta \theta$ of two grazing ternary clusters.
*   **QED Comparator:** Rutherford / Møller scattering differential cross-sections.
*   **Assessment:** Requires an extremely large lattice to resolve sub-degree deflection angles cleanly. At achievable engine sizes ($L=128, 256$), the discrete voxel grid dominates the angular resolution, masking the fine-structure correction.

---

## 3. Next Actions (ARC-D1)

The most viable path is **2.1: Cluster Fission-Fusion Branching Ratios**. 

**Action Item:**
1. Define a strictly stable "electron-analog" cluster at $L=128$.
2. Run a massive Monte Carlo sweep (`campaign_cluster_fission_fusion.cpp`) injecting minimal flux perturbations.
3. Count the exact integer ratio of elastic bounces vs. inelastic fissions.
4. If this discrete integer ratio converges to a function of $\sim 137$, we have an operational ARC-D readout.

**Prior on Success:** Medium-Low. The engine's deterministic Moore-neighborhood rules may produce highly chaotic, non-perturbative branching ratios that do not cleanly isolate the QED tree-level vertex. However, it rigorously obeys all "No-Cheat" exclusion rules.
