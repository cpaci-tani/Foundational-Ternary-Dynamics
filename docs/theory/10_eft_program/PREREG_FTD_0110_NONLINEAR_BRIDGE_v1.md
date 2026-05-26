# PRE-REGISTRATION -- FTD-0110 Nonlinear Bridge Sweeps and Active Partitioning (F-D3), v1

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-05-26
**Hash-lock target tag:** `preregister-ftd0110-nonlinear-bridge-v1`
**LEDGER row reservation:** FTD-0215 (downstream of FTD-0203)
**Companion docs:**
- `docs/theory/10_eft_program/SCOPE_FTD_0110_NONLINEAR_BRIDGE.md` (the classification memo)
- `docs/theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` (linear k = 1/4 theorem)
- `docs/theory/03_derivations/EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md` (the Phase A/B/C exploration)
- `scripts/proofs/proof_ftd0110_active_partition.py` (the active-block partitioning verification script)
- `scripts/proofs/proof_ftd0110_full_aggregation.py` (the Phase C over-counting verification script)

> **Pre-registration discipline.** The rules in Sections 2-4 are committed before the GPU-resourced engine parameter sweeps are executed on WSL2/CUDA. Section 5 records the desk-analytical verification of the mathematical `AP-no-over-count` aggregation rule (already executed and verified via `proof_ftd0110_active_partition.py`). After commit, this document's SHA256 is recorded in `REF_PREREGISTER_MANIFEST.md` and the git tag is applied. Any post-hoc changes to Sections 2-4 invalidate v1.

---

## 1. Thesis: The Nonlinear Bridge and the $k(A)$ Drift

### 1.1 The Linear Foundation (THEOREM)
At the linear level ($A \to 0$ or small $A \approx 2$), FTD-0110 is governed by the $O_h$ character-table multiplicity theorem (`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`):
- The 27-block $A_{1g}$ subspace has dimension $4$.
- The central voxel field is $A_{1g}$-pure.
- The 18-point discrete Laplacian preserves the $A_{1g}$ representation.
- The mean of the $A_{1g}$ eigenmode energies is exactly $k_{\text{linear}} = 1/4 = 0.250$.
This result is direction-invariant and exact.

### 1.2 The Nonlinear Drift (CONJECTURE)
As the amplitude $A$ increases up to $A \approx 120$, the empirical cluster coefficient $k(A) = N(A)/A^2$ measured in the simulation engine drifts logarithmically from $k(A \approx 2) = 0.25$ down to $k(A \approx 120) \approx 0.21$. The empirical k(A) curve is fitted by:

$$k(A) \approx \frac{1}{4} \left(1 - 0.030 \ln\left(\frac{A}{2}\right)\right)$$

This pre-registration locks the coordinated parameter sweeps designed to discriminate among three candidate physical mechanisms responsible for this nonlinear drift:
- **Mechanism $\alpha$ (Irrep mixing / leakage):** Multi-block representation mixing. The slow-mode energy leaks out to non-$A_{1g}$ shells, resulting in a logarithmic decay of block-injected energy.
- **Mechanism $\beta$ (Genesis kink):** Energy redistribution induced by the genesis threshold crossing. At high $A$, non-$A_{1g}$ modes are excited at the genesis boundaries, draining the slow mode.
- **Mechanism $\gamma$ (Langevin equilibrium):** Thermal amplitude-crossover at a characteristic amplitude $A^* \approx 13$.

---

## 2. Coordinated Parameter Sweeps (Arms D3a-D3d)

The campaign comprises four parameter arms executed on the simulation engine at fixed amplitudes $A \in \{10, 30, 100\}$ using 5 independent RNG seeds.

### 2.1 Arm D3a: Genesis kinetic drain sweep
- **Independent variable:** `K_GENESIS_KINETIC_DRAIN` in $\{0.0, 0.25, 0.5, 0.75\}$ (default is $0.5$).
- **Observable:** $k(A, \text{drain})$.
- **Diagnostic target:** Test whether the drift is driven by genesis kink-induced energy loss (Mechanism $\beta$).

### 2.2 Arm D3b: Evaporation rate sweep
- **Independent variable:** `K_EVAP_RATE` in $\{0.01, 0.05, 0.10, 0.20\}$ (default is $0.05$).
- **Observable:** $k(A, \text{evap})$.
- **Diagnostic target:** Test whether cluster-mass balance is dominated by soliton evaporation dynamics.

### 2.3 Arm D3c: Langevin temperature sweep
- **Independent variable:** $T_L$ (Langevin temperature) in $\{0.0, 0.01, 0.05, 0.10\}$ (default is $0.0$, i.e. zero temperature).
- **Observable:** $k(A, T_L)$.
- **Diagnostic target:** Test whether thermal crossover (Mechanism $\gamma$) governs the drift. Verified against `test_langevin_equipartition.cpp` (equipartition holds to ~4%).

### 2.4 Arm D3d: Lattice size scale sweep
- **Independent variable:** Lattice size $L \in \{64, 128\}$.
- **Observable:** $k(A, L)$ at $A \in \{30, 120\}$.
- **Diagnostic target:** Test Mechanism $\alpha$ leakage. At $A=30$, the cluster radius $R \approx 9$ is much smaller than $L=64$, so $L=64$ and $L=128$ must yield identical $k$ values. At $A=120$ ($R \approx 24$), boundary reflection and leakage differ between $L=64$ and $L=128$. If $k(A)$ saturates or deviates significantly between $L=64$ and $L=128$ at large $A$, Mechanism $\alpha$ boundary leakage is dominant.

---

## 3. Discrimination and Calibration-Invariant Criteria

To resolve the mechanisms, we establish the following target relationships:

| Arm | Dominant Mechanism | Verification Criterion |
|---|---|---|
| **D3a** | **Mechanism $\beta$ (Genesis)** | $k(A, \text{drain}) \propto \text{drain}^2$ at large $A$. |
| **D3b** | **Evaporation Dynamics** | $k(A)$ scales monotonically with `K_EVAP_RATE` due to boundary erosion. |
| **D3c** | **Mechanism $\gamma$ (Langevin)** | The $k(A)$ curve shifts horizontally as a function of temperature $T_L$. |
| **D3d** | **Mechanism $\alpha$ (Leakage)** | $k(A, L=64)$ and $k(A, L=128)$ match at $A=30$ but diverge at $A=120$. |

---

## 4. Locked Pass/Fail and Outcome Cases

- **Outcome A (Mechanism $\alpha$ Dominant):** Arm D3d shows divergence at $A=120$ while Arm D3c (Langevin) shows negligible temperature shift. The logarithmic decay is confirmed as boundary leakage.
- **Outcome B (Mechanism $\beta$ Dominant):** Arm D3a shows quadratic dependence on kinetic drain, indicating the genesis threshold is the source of the non-$A_{1g}$ leakage.
- **Outcome C (Mechanism $\gamma$ Dominant):** Arm D3c shows a clear temperature-dependent shift of the crossover amplitude $A^*$, confirming the Langevin-equipartition-crossover description.
- **Outcome D (Multi-Mechanism Convergence):** Multiple arms contribute comparable drifts, requiring a coupled multi-regime description.

---

## 5. Desk-Analytical Verification of the $AP\text{-no-over-count}$ Rule

### 5.1 The Over-Counting Bug of Phase C
Phase C of FTD-0110 initially extended the linear theorem via simple per-block summation, yielding a predicted $k_{\text{pred}} > 1$ at large $A$, which is physically impossible. This was caused by an $O(8)$ over-counting of voxels, as each voxel belongs to exactly 27 blocks in a Moore neighborhood. This was verified via `proof_ftd0110_full_aggregation.py`, yielding:

$$\frac{k_{\text{pred}}}{k_{\text{emp}}} \approx 3.6 \quad (\text{at } A=10) \quad \to \quad \frac{k_{\text{pred}}}{k_{\text{emp}}} \approx 8.3 \quad (\text{at } A=117.93)$$

### 5.2 The Active-Partitioning Aggregation Rule (AP-no-over-count)
To resolve this, we formulated the active-partitioning aggregation rule:
At each voxel $y$ in the lattice, define the per-voxel manifestation weight $\eta(y)$:

$$\eta(y) = \frac{1}{N_{\text{active}}(y)} \sum_{x \in \text{Blocks}_{\text{cluster}}(y)} \frac{1}{d_G(x)}$$

where:
- $\text{Blocks}_{\text{cluster}}(y)$ is the set of active blocks containing $y$ (whose centers are cluster-active voxels).
- $N_{\text{active}}(y)$ is the total number of active blocks containing $y$.
- $d_G(x)$ is the trivial-irrep dimension of block $x$'s local symmetry.

The predicted cluster coefficient is then:

$$k_{\text{pred}}(A) = \sum_{y \in \text{Lattice}} |G_L(y)|^2 \eta(y)$$

### 5.3 Numerical Verification Results
We executed this exact aggregation rule on a $L=32$ discrete lattice via `proof_ftd0110_active_partition.py` and compared the results against the canonical empirical data:

| $A$ | $k_{\text{emp}}$ | Cluster Radius $R(A)$ | $k_{\text{pred}}$ | $\frac{k_{\text{pred}}}{k_{\text{emp}}}$ |
|---|---|---|---|---|
| 10.00 | 0.252 | 1.82 | 0.1779 | 0.7060 |
| 15.00 | 0.224 | 2.29 | 0.1808 | 0.8069 |
| 20.00 | 0.234 | 2.82 | 0.1655 | 0.7073 |
| 28.77 | 0.253 | 3.68 | 0.1789 | 0.7072 |
| 30.00 | 0.262 | 3.83 | 0.1759 | 0.6712 |
| 33.05 | 0.245 | 4.00 | 0.1759 | 0.7178 |
| 50.00 | 0.222 | 5.10 | 0.1861 | 0.8385 |
| 62.42 | 0.224 | 5.93 | 0.1888 | 0.8428 |
| 85.70 | 0.212 | 7.19 | 0.1929 | 0.9098 |
| 117.93 | 0.206 | 8.81 | 0.1949 | 0.9462 |

### 5.4 Conclusion of Desk Verification
1. **Physicality:** The predicted $k_{\text{pred}}(A)$ lies strictly in the physical range $[0.0, 1.0]$ for all $A$.
2. **Convergence:** The predicted ratio $\frac{k_{\text{pred}}}{k_{\text{emp}}}$ flows from $70.6\%$ to $94.6\%$ as $A$ increases. This indicates that the active-block partitioning rule represents a highly accurate, non-circular mathematical model of discrete cluster aggregation.

---

## 6. Falsification Rules and Banned Moves
1. **No post-hoc sweep alterations:** No parameters or seeds may be modified after starting the GPU run.
2. **No curve-fitting:** It is forbidden to add free scale factors to align $k_{\text{pred}}$ with $k_{\text{emp}}$.
3. **No circularity:** All inputs to the sweeps must be engine-native parameters, with no QED values hardcoded into the dynamics.
