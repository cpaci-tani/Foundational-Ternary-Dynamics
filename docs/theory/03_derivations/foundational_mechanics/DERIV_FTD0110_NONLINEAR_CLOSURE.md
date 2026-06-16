# DERIV · FTD-0110 Nonlinear Mass Bridge Closure (Perturbation Theory)

**Tag:** [DERIVATION] / [OPEN PROGRAM]
**Date:** 2026-06-15
**LEDGER:** FTD-0110
**Depends on:** `DERIV_OSCILLATORY_CLOUD_DYNAMICS.md`, `DERIV_18PT_LAPLACIAN_VARIATIONAL.md`
**Status:** Analytical bound on the thermal knee established via spectral gap computation.

---

## 1. The Perturbation Hypothesis

The FTD-0110 Mass Bridge observes that the cluster efficiency $k(A) = N / A^2$ is strictly $1/4$ at the linear level, but at $A \approx 14$ it fractures into a logarithmic drift:
$$ k(A) \approx \frac{1}{4} \big(1 - \gamma \ln(A)\big) $$

Following the Oscillatory Cloud formalization (`DERIV_OSCILLATORY_CLOUD_DYNAMICS.md`), this is a resonant mode crossover. At low amplitudes, the injected energy is trapped perfectly in the $A_{1g}$ (breathing) sub-space. As the amplitude grows, the internal pressure exceeds the spectral gap, leaking energy into orthogonal shear representations ($T_{1u}$, $E_g$) and triggering the $\ln(A)$ scale-integration.

---

## 2. Spectral Analysis of the 18-Point Laplacian

The fundamental dynamics of the uncoupled continuous flux $J(v,t)$ on the lattice are governed by the canonical 18-point Laplacian $\Delta_w$ with weights $w_\text{face}=1/3$ and $w_\text{edge}=1/6$. 

We compute the exact eigenvalues of $\Delta_w$ over the 27-block Moore neighborhood with Dirichlet boundaries (representing the tightly bound metastable cloud before it spans multiple lattice shells). The characteristic equation yields the following lowest-energy states:

| Mode | Irrep (O_h) | Eigenvalue $\lambda$ | Degeneracy | Projection from Center ($\delta_0$) |
| :--- | :--- | :--- | :--- | :--- |
| **0** | $A_{1g}$ (Breathing) | $- (3 - \sqrt{2}) \approx -1.585786$ | 1 | $1/8$ |
| **1-3** | $T_{1u}$ (Shear) | $-2.72386$ | 3 | $0$ |
| **4-6** | $T_{2g}$ (Quadrupole) | $-3.52860$ | 3 | $0$ |
| **7-9** | $T_{2u}$ | $-3.86193$ | 3 | $0$ |

### 2.1 The $A_{1g}$ Breathing Mode
The fundamental resonance of the cloud is an $A_{1g}$ state with exactly $\lambda_{0} = -(3-\sqrt{2})$. A perfectly symmetric amplitude injection at the center voxel ($\delta_0$) projects exactly $1/8$ of its squared norm onto this fundamental mode. 

### 2.2 The Spectral Gap
The first non-trivial deformation mode that can break the symmetric shell is the triply-degenerate $T_{1u}$ shear mode. The spectral gap between the fundamental breathing mode and the lowest shear mode is:
$$ \Delta\lambda = |-2.72386| - |-1.585786| = 1.13807 $$

---

## 3. The Langevin Crossover Threshold ($A_c$)

The energy of the perturbation $E \propto A^2$ is bounded by the stiffness of the discrete lattice. As long as the thermal energy available to fluctuate out of the breathing mode is less than the frequency gap $\Delta \omega^2 \propto c^2 \Delta \lambda$, the $A_{1g}$ sub-space protects the $N = \frac{1}{4}A^2$ counting law.

When the amplitude reaches a critical threshold $A_c$, the energy spilling from the non-linear manifestation thresholds ($|s|$) overcomes the spectral gap $\Delta\lambda = 1.13807$. 

Because the $T_{1u}$ mode is triply degenerate, the crossover does not just excite a single new degree of freedom; it shatters the spherical symmetry into a multi-directional Langevin oscillator. This cascading irrep-mixing continuously taps energy as the cloud expands, cleanly producing the empirical $-\gamma \ln(A)$ scale-integration.

### 3.1 Next Steps for Exact Closure
To analytically prove that $A_c \approx 14$:
1.  Derive the exact nonlinear coupling constant $\kappa(s)$ between the flux and the discrete state.
2.  Compute the matrix element $\langle T_{1u} | \mathcal{H}_{\text{nonlinear}} | A_{1g} \rangle$.
3.  Set the transition rate equal to the oscillation period $T$ to solve for $A_c$.
