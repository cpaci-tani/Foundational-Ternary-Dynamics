# SPEC - Connection Extraction Rule & Topological Holonomy

**Tag:** [OPEN PROGRAM]
**LEDGER:** FTD-0209 [SYNTHESIS] - formalizes the extraction of $A_J$ to bypass the longitudinal holonomy obstruction.
**Companion docs:** `SPEC_CLOSED_FLUX_LOOP_READOUT.md`, `SPEC_LATTICE_HODGE_CONNECTION.md`

---

## 0. Purpose

Following the Lattice Hodge decomposition (FTD-0208), the naive closed-loop integral of the relaxed physical flux field ($\oint J_{\infty} \cdot d\ell$) is highly susceptible to the longitudinal holonomy obstruction. If the field is perfectly exact ($d\phi$), the signal identically vanishes.

This document formally defines **Option C**: the extraction of an FTD-native connection $A_J$ from the underlying $\mathbb{Z}[i]$-module quadrature structure. By shifting from continuous flux circulation to a discrete phase holonomy, the observable gains the ability to measure topological winding around the minimal neutral source $\Omega_{\min}$, decisively avoiding the Phase 1 zero-signal trap.

---

## 1. The Quadrature Field Definition

The canonical FTD architecture requires mapping the continuous flux $J$ and discrete state $s \in \{-1, 0, 1\}$ into the intrinsic quarter-conjugate representation space. 

We define the discrete quadrature field evaluated at a lattice vertex $v$:
$$ z(v) = q(v) + i p(v) $$
where $q(v)$ and $p(v)$ are real-valued lattice functions synthesized from the coupled state-flux $(s, J)$ limits under the relaxed stable neutral source limit. The exact composition of $z(v)$ is structurally bounded by the lemniscatic curve $y^2 = x^3 - x$ mapping requirements (as detailed in the Algebraic Spine).

---

## 2. The Native Connection ($A_J$)

The connection is not the raw physical flux. It is mathematically extracted from the quadrature phase difference between adjacent sites.

For a directed lattice edge $e$ from vertex $v$ to $v+e$, the FTD-native phase connection $A_J(e)$ is defined as the argument of the complex product:

$$ A_J(e) = \operatorname{Arg}\left(\overline{z(v)} z(v+e)\right) $$

This geometrically defines parallel transport of the intrinsic state phase across the lattice edge. 

---

## 3. The Topological Loop Observable

The Phase 1 topological loop observable $\mathcal{L}_C$ is defined as the discrete sum of the extracted connection over a closed loop contour $C$:

$$ \mathcal{L}_C = \sum_{e \in C} A_J(e) $$

### The Winding Mechanism
*   **Contractible/Trivial Regions:** In regions where the quadrature field $z(v)$ is smooth and non-singular, $A_J(e)$ behaves as a pure exact phase gradient. By discrete Stokes' theorem, $\sum_C A_J = 0$.
*   **Topological Defects:** If the minimal neutral source $\Omega_{\min}$ induces a vortex, defect, or singularity in the quadrature phase field, the closed sum will yield a non-zero topological winding number (e.g., $2\pi n$).

This topological winding specifically bypasses the longitudinal holonomy obstruction. It allows the observable to carry a non-zero, fundamentally discrete, topology-driven signal into Phase 2 of the readout proof.

---

## 4. The Decisive Falsification Target

This mechanism anchors Phase 1 of the Alpha Readout Program. The mathematical proof obligation narrows to the following test:

> [!IMPORTANT]
> **Does the minimal neutral source $\Omega_{\min}$ induce a non-zero topological winding in the extracted phase connection $A_J$?**

*   **If YES:** The connection $A_J$ is canonically forced. The loop survives Phase 1, and the derivation proceeds to Phase 2 to evaluate the response matrix $W_L^{(\infty)}$ for the master quadratic coefficients ($16G^{*2}$, $16G^{*3}$).
*   **If NO:** All native topological holonomies vanish for the neutral source. The $\alpha$ bridge closes entirely in its final viable sector.

**Status:** Awaiting rigorous mathematical test of the winding number over $\Omega_{\min}$.
