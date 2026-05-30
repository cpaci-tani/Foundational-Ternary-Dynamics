# PRE-REGISTRATION — Alpha Readout Boundary-Condition Closure (ARC-A1) v2

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-05-30
**Framework:** Foundational Ternary Dynamics v5.33
**LEDGER:** FTD-0238 (Pre-registration of ARC-A1 v2)

---

## 0. Context and Prior Failure (v1)

The v1 attempt (`archive/closed_negative/PREREG_ALPHA_READOUT_BOUNDARY_v1.md`) failed because the chosen boundary condition (a hard Dirichlet wall) admitted an infinite continuous spectrum of resonances, possessing a free parameter that allowed tuning to $1/\alpha$ trivially. It failed Gate 2 (No scheme tuning) of the No-Cheat checklist.

This v2 attempt shifts the paradigm: instead of imposing a hard wall, we treat the lattice boundary topologically as a 2D torus parametrized by the complex modular parameter $\tau$.

## 1. The ARC-A1 Hypothesis

**Hypothesis:** The physical boundary of the discrete 3D FTD lattice acts as a 2D modular torus. The self-consistency of transition amplitudes across this boundary strictly filters the bulk spectrum, uniquely admitting states whose boundary modular parameter $\tau$ lies exactly at the lemniscatic fixed point $\tau = i$, where the modular invariants coincide with the master quadratic periods ($G^*$).

If true, the boundary naturally acts as a "filter" that forces the system's external macroscopic coupling to lock to the roots of the master quadratic ($x_+$).

## 2. The ARC-A1 Tuple Specification

Per the `SPEC_ALPHA_READOUT_CONTRACT.md`, we declare:

*   **Preparation Class ($P$):** A stable bulk voxel cluster enclosed within a periodic 2D boundary surface $S$ (topologically $T^2$).
*   **Observable Algebra ($\mathcal{A}_{\text{obs}}$):** The modular invariant $j(\tau)$ of the boundary transition amplitude on $S$.
*   **Electromagnetic Measurement ($O_{\text{EM}}$):** The macroscopic back-reaction of the bulk cluster emitting flux through the boundary surface.
*   **Readout Map ($R$):** The ratio of the emitted flux variance to the modular period of the boundary state.
*   **Calibration ($C$):** Dimensionless topological winding of the boundary, requiring no bare unit conversion.

## 3. The 5-Step Evaluation Method

To rigorously evaluate this hypothesis without target-insertion, we will execute the following blind mathematical steps:

1.  **Define the discrete boundary transition amplitude $Z_S(\tau)$.** We construct the partition sum over all ternary configurations $s(x) \in \{-1, 0, 1\}$ restricted to the 2D surface $S$.
2.  **Impose transition self-consistency.** Demand that $Z_S(\tau)$ is invariant under the modular group $PSL(2, \mathbb{Z})$ mapping the lattice to itself.
3.  **Identify the fixed points.** Solve the self-consistency condition $\tau \to -1/\tau$ to rigorously isolate the stable fixed point $\tau = i$ (the lemniscatic curve).
4.  **Extract the characteristic equation.** Compute the characteristic variance of the flux field at the fixed point $\tau=i$.
5.  **Check for the master quadratic.** Evaluate whether the characteristic equation exactly yields $x^2 - 16 G^{*2} x + 16 G^{*3} = 0$.

## 4. Banned Moves

*   Do not multiply the result by arbitrary powers of $\pi$ or 2 to force a match.
*   Do not identify the boundary area with $4\pi r^2$ if the lattice is intrinsically cubic.
*   Do not compute the determinant $16 G^{*3}$ by assuming it must follow from the trace.

## 5. Pre-Registered Outcomes

*   **FOUND:** The boundary self-consistency equation explicitly yields the complete master quadratic without additional free parameters. (Upgrades MC-T4.3 to [DERIVED]).
*   **UNDERDETERMINED:** The boundary modular parameter locks to $\tau=i$, yielding the trace ($16 G^{*2}$), but fails to uniquely determine the odd-powered determinant ($16 G^{*3}$).
*   **CLOSED-NEGATIVE:** The boundary transition amplitude is either trivial, unstable, or contains a tunable parameter allowing an infinite family of couplings.
