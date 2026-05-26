# SCOPE — FTD-0152 Alpha Readout Contract: Next Steps and Scoping Analysis

**Tag:** [SCOPING MEMO] / canonical
**Date:** 2026-05-26
**LEDGER row:** FTD-0210 (new scoping memo claim)
**Depends on:** FTD-0152 (Alpha Readout Contract), FTD-0205 (ARC-B1 closed-negative synthesis)

---

## 0 · Purpose and context

Following the 2026-05-23 **CLOSED-NEGATIVE** synthesis of the three primary **ARC-B1** observable-selection routes (plaquette bivectors, boundary-to-boundary transfer, and reflexive projections; recorded in [`AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE_SYNTHESIS.md`](../10_eft_program/AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE_SYNTHESIS.md)), the search space for the central foundational obstruction **MC-T4.3 (Operational alpha-readout mechanism)** must narrow and adapt.

The ARC-B1 closures demonstrated a profound **categorical structural mismatch**:
* **Substrate arithmetic:** FTD-native lattice observables yield eigenvalues and characteristic equations whose coefficients are finite-combinatorial (e.g., Lie algebra structure constants, subalgebra dimensions) or lattice-spectral (e.g., discrete Laplacian exponentials $e^{-E_k \tau}$).
* **Lemniscatic arithmetic:** The master quadratic and its roots ($x_+ \approx 137.04$, $x_- \approx 3.02$) have coefficients ($16 = |\text{Aut}(E)|^2$, $G^* = \Gamma(1/4)/\Gamma(3/4)$) that are number-theoretic invariants of the lemniscatic elliptic curve $E: y^2 = x^3 - x$ over $\mathbb{Z}[i]$.

To "earn the map" from the algebraic spine to a physical QED readout without circularity, future work must bridge this categorical mismatch. This scoping memo outlines the next unattempted technical routes under the `SPEC_ALPHA_READOUT_CONTRACT.md` framework:
1. **Candidate A (Boundary-Condition Readout)** — Self-consistency condition on open boundaries.
2. **Candidate C (Quantization/Readout Rule)** — Non-action discrete normalization.
3. **Candidate B2 (BCC Algebraic Bridge)** — Operationalizing the $\mathbb{Z}[i]$-module structure of $V_{\text{complex}}$ as an observable.

---

## 1 · Technical routes and scoping analysis

### Route 1: Candidate A (Boundary-Condition Readout)

* **Concept:** Define a finite or undefined-boundary self-consistency condition on the FTD lattice whose spectrum naturally selects the master-quadratic root as the unique stable boundary coupling.
* **Algebraic anchor:** The boundary of a 3D cubic lattice naturally breaks the full $O_h$ point-group symmetry to a 2D boundary subgroup (typically $C_{4v}$). If the boundary fields are constrained by a self-consistency cycle (e.g., transition amplitude matching), the eigenvalue problem on the boundary states may induce lemniscatic periods through modular parametrization of the 2D boundary torus.
* **Tractability:** High-risk but mathematically elegant. Requires solving the lattice Poisson equation with open boundary conditions and identifying a self-consistency map (e.g., Möbius boundary reductions).
* **Next action:** Author `PREREG_ALPHA_READOUT_BOUNDARY_v1.md` detailing the transition-amplitude boundary spectrum.

### Route 2: Candidate C (Quantization / Readout Rule)

* **Concept:** Derive a discrete measurement rule that maps the FQCR/master-quadratic dominant eigenvalue directly to the QED coupling without passing through a continuous-field action.
* **Algebraic anchor:** The lemniscatic ratio $G^* = \Gamma(1/4)/\Gamma(3/4)$ represents the regularized asymmetry between inert and split primes in the arithmetic of $\mathbb{Z}[i]$ ( Lerch's zeta period). A quantization rule that measures charge as a winding number or topological index on the $\mathbb{Z}[i]$-complex manifold in the BCC lattice could naturally map the physical coupling to the reciprocal of $x_+$.
* **Tractability:** Medium-effort desk track. Building on the FQCR determinant-one symmetric recurrence and the motivically-derived master quadratic.
* **Next action:** Author a methodological audit (`AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md`) evaluating the exact QED-vs-native normalization boundary.

### Route 3: Candidate B2 (BCC Algebraic Bridge)

* **Concept:** A variant within the observable-selection class (ARC-B). Rather than generic lattice bilinears, this route specifically operationalizes the $\mathbb{Z}[i]$-module structure of the $V_{\text{complex}}$ representation in the BCC decomposition (proven in `DERIV_BCC_COMPLEX_STRUCTURE.md` / `OT-1.5` [THEOREM]).
* **Algebraic anchor:** The 8 BCC corners decompose under $O_h$ regular representation as $\mathbb{Z}[\text{BCC}] \otimes \mathbb{Q} \cong V_{\text{triv}}^2 \oplus V_{\text{sign}}^2 \oplus V_{\text{complex}}^2$, where $V_{\text{complex}}$ carries a natural $\mathbb{Z}[i]$-module structure isomorphic to $\mathbb{Z}[i]^2$. Because $V_{\text{complex}}$ is arithmetically isomorphic to the Gaussian integers, it carries exactly the unit group $\mu_4 = \mathbb{Z}[i]^\times$ of order 4 (giving the coefficient $16 = |\mu_4|^2$).
* **Readout mechanism:** An observable $O_{\text{BCC}}$ defined specifically on the $V_{\text{complex}}$ subspace could bridge the discrete lattice to $\mathbb{Z}[i]$ arithmetic naturally, since the subspace itself is structurally isomorphic to $\mathbb{Z}[i]^2$.
* **Tractability:** Excellent mathematical backing (the structural theorem `OT-1.5` is already T1 bedrock). The open gap is designing an *operational* measurement protocol that isolates the $V_{\text{complex}}$ component without violating the banned-moves circularity rule (F-j).

---

## 2 · Scoping schedule

| Work item | Route class | Effort | Prerequisites | Prior success probability |
|---|---|---|---|---|
| **ARC-C1 (Charge normalization)** | Candidate C | ~3-5 days | `SPEC_ALPHA_READOUT_CONTRACT.md` | Medium |
| **ARC-B2 (BCC algebraic readout)** | Candidate B variant | ~1 week | `DERIV_BCC_COMPLEX_STRUCTURE.md` | High |
| **ARC-A1 (Boundary self-consistency)** | Candidate A | ~2 weeks | `AUDIT_*_CLOSED_NEGATIVE_SYNTHESIS.md` | Low-Medium |

**Recommended order:** ARC-C1 first (cleans the methodological baseline); then ARC-B2 (exposes the T1 bedrock $V_{\text{complex}}$ structure); then ARC-A1.
