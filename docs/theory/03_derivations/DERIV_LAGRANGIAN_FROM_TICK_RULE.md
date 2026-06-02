# DERIV - Lagrangian from Tick Rule (Time-Discretization)

**Tag:** [DERIVATION] / [OPEN]
**Date:** 2026-05-30
**LEDGER:** FTD-0246 [Mechanism B: Bare Action Construction]

## 1. The Physics Objective

To execute **Mechanism B (Lattice-to-Continuum Matching)**, we must compute the 1-loop $\beta$-function coefficient (screening self-energy) from the FTD dynamics and compare it to QED. 
Because the FTD engine is a classical update rule, it has no intrinsic quantum $\beta$-function. We must first promote the engine to a quantum partition function:
$$ Z = \int \mathcal{D}J \, \mathcal{D}s \, e^{-S_E[J, s]} $$

The critical first step—which this document addresses—is extracting the Euclidean action $S_E$ directly from the engine's time-discretized tick rules (the wave equation, Gauss projection, and state update).

## 2. The Engine's Tick Rule

From `engine/src/lagrangian.cpp` and `ontic.h`, the explicit discrete time-evolution for the flux field $J$ at site $i$ and discrete time $t$ is:
$$ \Delta J_{i, t} = J_{i, t+1} - J_{i, t} = c^2 \nabla^2 J_{i, t} + g_c \nabla s_{i, t} + g_c \nabla \times (s_{i, t} v_{i, t}) $$
where:
- $c = C_{\text{SPEED}} = 1/\sqrt{3}$ is the lattice wave speed.
- $g_c$ is the bare Gauss coupling (currently empirically set to $\sqrt{2\pi \alpha}$).
- $\nabla$, $\nabla^2$, $\nabla \times$ are discrete spatial difference operators.

## 3. Discretized Equations of Motion to Action

The update rule $\Delta J = F(J, s)$ is a first-order (in time) difference equation for the flux. However, standard lattice wave equations are second-order in time. In FTD, the wave velocity $v_J$ is an independent variable updated alongside $J$, which effectively makes the system second-order:
$$ \partial_t^2 J \approx \nabla^2 J + \dots $$

To find the action whose Euler-Lagrange equations yield this tick rule, we integrate the equations of motion.
The total Lagrangian density $L = L_{\text{kin}} + L_{\text{grad}} + L_{\text{int}}$ evaluated in `lagrangian.cpp` is:
$$ \mathcal{L} = \frac{1}{2} (\partial_t J)^2 - \frac{c^2}{2} (\nabla J)^2 - g_c s (\nabla \cdot J) - g_c s (v \cdot J) $$

### 4. Euclidean Wick Rotation

To construct the partition function, we Wick-rotate to imaginary time $\tau = i t$. The Euclidean action is $S_E = \int d\tau d^3x \, \mathcal{L}_E$:
$$ \mathcal{L}_E = \frac{1}{2} (\partial_\tau J)^2 + \frac{c^2}{2} (\nabla J)^2 + g_c s (\nabla \cdot J) + g_c s (v \cdot J) $$

On the discrete lattice, the integral becomes a sum over the spacetime lattice $\Lambda$:
$$ S_E = \sum_{x \in \Lambda} \left[ \frac{1}{2} (\Delta_\tau J_x)^2 + \frac{c^2}{2} (\nabla J_x)^2 + g_c s_x (\nabla \cdot J_x) + \dots \right] $$

## 5. The $g_c$ Free Parameter Problem

The explicit derivation of $S_E$ reveals a critical structural fact: **the time-discretization does not mathematically constrain $g_c$.** 
The lattice spacing $a$ and time step $\Delta t$ can be absorbed into the definition of the fields, but the relative coefficient between the kinetic term $\frac{c^2}{2} (\nabla J)^2$ and the interaction term $g_c s (\nabla \cdot J)$ remains a free parameter of the discrete Lagrangian.

If we perform the path integral matching (Mechanism B) using this action, the resulting continuum coupling $e_{\text{eff}}$ will be a function of $g_c$. 
$$ e_{\text{eff}}^2 = f(g_c) $$

**Conclusion on First-Principles $g_c$:** The classical tick rules alone do not force $g_c = \sqrt{2\pi \alpha}$. Mechanism B requires computing the exact loop integral on the $J$-twisted spectrum (as outlined in `DERIV_LATTICE_PATH_INTEGRAL_JTWIST.md`) to see if the topological properties of the $V_{\text{complex}}$ vector space force a specific quantized eigenvalue for $g_c$ to ensure the path integral is well-defined.

This formally scopes the multi-month Mechanism B physics task: computing the 1-loop $\beta$-function from this precise $S_E$ action over the quarter-twisted spectrum.
