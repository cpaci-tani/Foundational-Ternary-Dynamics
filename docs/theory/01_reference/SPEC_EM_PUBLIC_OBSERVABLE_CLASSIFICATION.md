# SPEC - Electromagnetic Public Observable Classification

**Tag:** [REFERENCE] / [OPEN PROGRAM]
**LEDGER:** FTD-0206 [SYNTHESIS] - formalizes the taxonomy of public electromagnetic measurement channels under the FTD phase-law.
**Companion docs:** `SPEC_ALPHA_READOUT_CONTRACT.md`, `SPEC_ALPHA_READOUT_PROGRAM.md`

---

## 0. Purpose

The Alpha Readout Contract explicitly limits admissible electromagnetic readouts ($R_{\text{EM}}$) to non-circular, FTD-native public observables. This document classifies the six major structural types of observables against the phase-law. It isolates which channels are mathematically falsified (via categorical structural mismatch) and which remain viable.

---

## 1. Classification

### 1. Site-Local Readouts
*   **Examples:** $s(v)$, $J(v)$
*   **Status:** `[CLOSED NEGATIVE]`
*   **Obstruction:** FTD-0073 Mode-Erasure No-Go. 
*   **Reason:** The Gauss projection entirely erases the longitudinal $G_C$ contribution at the site-local level every tick. It cannot produce the global continuous transcendental values required by the algebraic spine.

### 2. Link-Local / Bilinear Observables
*   **Examples:** $J_i(x)J_j(y)$ on adjacent sites.
*   **Status:** `[CLOSED NEGATIVE]` (as standalone readouts)
*   **Obstruction:** Finite-combinatorial ceiling.
*   **Reason:** Link-local bilinears form the necessary construction primitives for higher-order observables (Catalog Item 3), but on their own, their characteristic polynomials are bounded by lattice spacing combinatorics and cannot reach the number-theoretic arithmetic geometry of $G^*$.

### 3. Plaquette / Bivector Readouts
*   **Examples:** The canonical lattice 2-form $P_{ij}(x)$.
*   **Status:** `[CLOSED NEGATIVE]`
*   **Obstruction:** FTD-0204 Audit (Categorical Structural Mismatch).
*   **Reason:** Forward derivations of transfer operators on the bivector algebra produce $\mathfrak{su}(2)$-type characteristic polynomials with $\kappa \varepsilon_{abc}$ structure-constant coefficients. These are finite-combinatorial quantities, categorically mismatched from the master quadratic's lemniscatic-curve invariants.

### 4. Boundary-to-Boundary Transfer Observables
*   **Examples:** Propagator/transition-amplitude functionals from a source face to a sink face.
*   **Status:** `[CLOSED NEGATIVE]`
*   **Obstruction:** FTD-0205 Audit (§1.B).
*   **Reason:** The lattice-Laplacian transfer matrix spectrum is defined by $\{e^{-E_k \tau}\}$ indexed by lattice momenta. Any 2D projection yields traces and determinants bounded by $O(1)$, which can never scale to match the $16(G^*)^2 \approx 140.05$ or $16(G^*)^3 \approx 414.36$ target values.

### 5. Reference Frame Projections (Public Channel)
*   **Examples:** Lindblad-style macroscopic coarse-graining (e.g., zero-momentum mode).
*   **Status:** `[CLOSED NEGATIVE]`
*   **Obstruction:** FTD-0205 Audit (§1.C).
*   **Reason:** Projecting onto the public macroscopic channel produces characteristic equations with coefficients bounded by Lindblad dissipation-decay rates ($\gamma_L$) and subalgebra conservation dimensions (1, 3, 4). These are dynamical and counting integers, not arithmetic geometry invariants.

### 6. Closed Flux-Loops / Wilson Loops
*   **Examples:** Dynamically coupled macroscopic flux loops, holonomies.
*   **Status:** `[UNDERDETERMINED / STILL VIABLE]`
*   **Reason:** While fixed-field Wilson-Dirac paths closed negative (measuring lattice artifacts, not QED loops), the broader class of ARC-B1 Catalog Item 5 variants (dynamically coupled flux-loops crossing non-local blocks) has not been structurally excluded. Loops represent the only remaining pure-topological observable class capable of bridging the FTD discrete limit to the continuum without decaying into bounded finite-combinatorial or dissipation-rate scalars.

---

## 2. Conclusion

The rigorous application of the Phase-Law and the 11-step closure method has effectively falsified Site-Local, Link-Local, Plaquette, Boundary-Transfer, and Frame-Projection channels. **Closed Flux-Loops** remain the only currently identified non-site-local public observable class not structurally excluded by the present audits, retaining the topological capacity to house the $W_U$ response operator required by the Alpha Readout Contract. Any future $\alpha$-derivation attempts must be concentrated in this topological sector.
