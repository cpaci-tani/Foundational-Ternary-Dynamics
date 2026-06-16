# FTD Phase-Law Specification v1.0-draft

**Classification:** Canonical Transition Operator Specification
**Status:** `[CANDIDATE UPDATE CYCLE]` 
**Purpose:** To define the explicit, closed-form update law $\Omega_t \to \Omega_{t+1}$ implemented by the FTD Engine, specifying the operational physics layer independent of continuum limits or effective actions.

---

## 1. State Space ($\Omega_t$)

The simulation substrate is defined over a finite 3D cubic lattice $L$ of $N^3$ voxels. Time $t \in \mathbb{N}$ advances in discrete, globally synchronized ticks.
The total state at any tick is an assignment of one voxel-state to every lattice site:
$$ \Omega_t : L \to \Omega_{\text{voxel}} $$

## 2. Allowed Fields Per Voxel

Each voxel $v \in L$ carries a closed, strictly bounded inventory of variables:

**Primary Pair (Orthogonal):**
*   **`flux`** ($J \in \mathbb{R}^3$): The dispositional vector field. *Note on ontology: $J \in \mathbb{R}^3$ is the continuum idealization of a bounded finite-precision engine variable. All empirical predictions must be stable under finite precision refinement.*
*   **`state`** ($s \in \{-1, 0, +1\}$): The discrete actualized manifestation layer.

**Nested Wave Sector (Inside Flux):**
*   **`wave_vel`** ($p = \partial_t J \in \mathbb{R}^3$): The symplectic momentum conjugate to flux, driving staggered wave propagation.

**Kinematic / Mechanical Sector:**
*   **`velocity`** ($u \in \mathbb{R}^3$): The macroscopic sub-lattice movement vector for manifested states.
*   **`remainder`** ($r \in \mathbb{R}^3$): Accumulator for sub-lattice fractional hops.

**Sector Extensions (Toggle-Gated):**
*   **`latency`**: A scalar field tracking gravitational/proper-time dilation.
*   **Dual Substrate**: `flux_L`, `flux_R` (splits $J$ for parity-violating physics).

## 3. Boundary Convention

**Undefined-Boundary Lattice:** The dynamics are strictly local (Moore neighborhood). For computational tractability, the engine implements a finite block with 3D periodic boundary conditions. The physical claim is that the true lattice possesses *no defined boundary* (not a completed infinity $\mathbb{Z}^3$, but arbitrarily large finite scopes). 

## 4. Ordered Update Phases (The Transition Operator $U$)

The update law is the sequential composition of discrete functional phases. 
$$ U = U_{movement} \circ U_{forces} \circ U_{Gauss} \circ U_{write} \circ U_{read} $$

The full canonical phase-ladder per tick $t \to t+1$:

1.  **$U_{read}$ (Field Read):** A parallel, read-only loop over the lattice. Computes $\Delta J$ using the 18-point Moore Laplacian $\nabla^2_{18} J$ and the state-flux coupling $g_c \nabla s + g_c \nabla \times (s \cdot u)$.
    * The exact 18-point Laplacian stencil is:
      $$ \nabla^2_{18}J(v) = \frac{1}{3}\sum_{\text{faces}}J(v+n) + \frac{1}{6}\sum_{\text{edges}}J(v+n) - 4J(v) $$
2.  **$U_{write}$ (Field Write & Manifestation):** A parallel commit loop.
    *   Advances the staggered wave: $p \gets p + \Delta J$, $J \gets J + p$.
    *   Applies damping (entropy).
    *   Executes Genesis ($0 \to \pm 1$) and Evaporation ($\pm 1 \to 0$) based on $|J|$.
3.  **$U_{Gauss}$ (Constraint Projection):** Enforces $\nabla \cdot J = s$ to couple the newly written flux to the newly manifested state. Solves the Poisson equation and corrects flux: $J \gets J - \nabla \phi$.
4.  **$U_{forces}$ (Kinematic Accumulation):** Iterates over $s \neq 0$. Computes gradients of the static/emergent potentials and integrates $F$ to update the sub-lattice velocity $u$, subject to the bandwidth speed limit $c = 1/\sqrt{3}$.
5.  **$U_{movement}$ (Discrete Hops):** Accumulates $r \gets r + u$. If $||r||_{\infty} \ge 1$ (or any component crosses the lattice boundary), a discrete coordinate hop is attempted, triggering collision or annihilation rules.

## 5. Conservation Laws and Admissions

*   **Charge ($s$):** Charge conservation requires that the net lattice sum $\sum s(v)$ remains invariant. Genesis and evaporation rules must either execute strictly as paired dipole events ($0,0 \to +1,-1$) or explicitly admit non-conservation, which forces the Gauss solver to strip the zero-mode. 
*   **Energy ($|J|^2$):** **Not strictly conserved.** The $U_{Gauss}$ projection is mathematically a non-variational operator that introduces a known energy leakage floor. Damping phases are also explicitly permitted. FTD does not demand perfect continuous-time Noether energy conservation on the discrete grid.

## 6. Genesis & Evaporation Thresholds

The coupling bridge connecting the orthogonal fields (Flux $\to$ State).
*   **Genesis:** At a void voxel ($s=0$), if $|J| > K_{GENESIS}$, manifestation triggers. The sign of $s$ is selected by the local flux divergence geometry: $s = \operatorname{sign}(\nabla \cdot J)$. To ensure strict determinism, if $\nabla \cdot J = 0$, polarity is tie-broken by local curl handedness. 
*   **Evaporation:** At a manifested voxel ($s \neq 0$), if the 7-site neighborhood (center + 6 face neighbors) energy falls below $K_B^2 \times 10^{-6}$, the state reverts to void ($s=0$).

## 7. Collision & Annihilation Rules

During $U_{movement}$, simultaneous attempts by multiple manifested voxels to hop into the same target coordinates must be resolved deterministically without depending on iteration order. 

The canonical resolution sequence is:
1. Collect all attempted moves into target buckets.
2. Resolve annihilating pairs first ($+1$ and $-1$ targeting the same voxel mutually annihilate; $s \gets 0$ and self-field bursts as flux).
3. Resolve same-sign conflicts by momentum-weighted bounce ($||u||_2$ dominates), using a lexicographic neighbor-index invariant as the absolute tie-breaker.
4. Commit all surviving moves to their targets. Unsuccessful movers undergo elastic bounce (remainder $r$ inverted).

## 8. Gauss Projection Rule

To bridge State $\to$ Flux, the lattice demands the Gauss constraint. On a periodic lattice, the Poisson equation $\nabla^2 \phi = \rho$ is solvable only if the zero mode vanishes ($\sum \rho = 0$). During $U_{Gauss}$:
1.  Compute the discrepancy source: $\rho = \nabla \cdot J - \kappa \cdot s$.
2.  **Zero-Mode Renormalization:** Explicitly remove the mean to enforce solvability: $\rho \gets \rho - \bar{\rho}$, where $\bar{\rho} = \frac{1}{N^3} \sum_{v \in L} \rho(v)$.
3.  Solve the discrete Poisson problem $\nabla^2_{18} \phi = \rho$ using a Successive Over-Relaxation (SOR) or spectral method.
4.  Project the flux: $J_{t+1} = J_t - \nabla \phi$.

## 9. Numerical Stability Rule

The lattice obeys the Courant-Friedrichs-Lewy (CFL) stability condition for the 3D wave equation. 
*   The fundamental speed limit of causality is exactly $c = 1/\sqrt{3}$ voxels per tick.
*   Wave integration utilizes a staggered symplectic leapfrog scheme.
*   Particle velocities are strictly Euclidean-clamped: $||u||_2 \le 1/\sqrt{3}$.

## 10. Readout Hooks ($R_{phys}$)

The observable endpoints where an internal frame functionally interrogates the invariant structure. The framework exposes specific functionals:
*   **Total Field Energy:** $\frac{1}{2} \sum |J|^2$ (Exposed to check lattice scaling and calibration geometry).
*   **Geometric Potential:** $\alpha_r = r G_L(r)$ (The Phase G Coulomb readout).
*   **Detection Statistics:** Local threshold-upcrossing event counters (Produces Rice statistics, confronting Born predictions).
*   **Correlations:** $S$-value accumulators reading $s_{A} \cdot s_{B}$ over localized test functions (The CHSH readout mechanism).
*   **Cluster Invariants:** $k = E_{mean} / E_{peak}$ coefficient extractors on stable manifested geometries.

> [!WARNING]
> FTD fails or succeeds precisely here. Until an operational $R_{phys}$ map explicitly uniquely dictates *why* the readout returns electromagnetism's $\alpha$ from the $U$ operator output—without importing $1/137.036$ by hand—the physics remains a conjecture.
