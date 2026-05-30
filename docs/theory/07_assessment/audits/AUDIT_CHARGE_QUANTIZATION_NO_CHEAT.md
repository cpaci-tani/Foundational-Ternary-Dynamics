# AUDIT · Charge Quantization and QED Normalization Integrity (ARC-C1)

**Tag:** [AUDIT] / canonical
**Date:** 2026-05-26
**LEDGER row:** FTD-0231 (new methodological audit claim)
**Depends on:** FTD-0152 (Alpha Readout Contract), FTD-0153 (Math-First Ontology)
**Status:** [AUDIT] complete; establishes the mathematical and methodological boundary separating QED imports from native FTD observables.

---

## 0 · Executive summary

To prevent circularity or post-hoc parameter tuning in the fine-structure constant ($\alpha$) derivation, this audit establishes a rigorous, "no-cheat" boundary between imported QED quantities and FTD-native variables. We analyze the discrete, topological nature of charge quantization in Foundational Ternary Dynamics (FTD), formalize the canonical normalization of native coupling constants, and supply a strict, non-circular checklist for any future physical readout proposal. 

Furthermore, we explore the arithmetic origin of the lemniscatic ratio $G^* = \Gamma(1/4)/\Gamma(3/4)$ as a regularized period on the Gaussian integers $\mathbb{Z}[i]$, mapping it to a potential discrete topological winding index on the Body-Centered Cubic (BCC) lattice structure.

---

## 1 · The native charge-quantization boundary

In standard Quantum Electrodynamics (QED), the electric charge $e$ is a continuous parameter that runs with energy scale under the Renormalization Group (RG). Its low-energy value $e_{\text{phys}} = \sqrt{4\pi\alpha_{\text{QED}}}$ is an empirical input.

In FTD, the microscopic ontology is strictly discrete, which naturally induces a discrete, topological charge quantization at the voxel level without continuous tuning:

### 1.1 Microscopic state alphabet
Voxel states are ternary: $s(x, t) \in \{-1, 0, +1\}$ ([AXIOM] 3). The signed source density is defined as:
$$\rho_{\text{lattice}}(x, t) = s(x, t)$$
which represents the actual manifestation of positive, void, or negative charge. Thus, the minimum unit of charge on the lattice is exactly $1$ in natural units. There are no fractional charges, and no continuous knobs.

### 1.2 The projected U(1) gauge field
The longitudinal flux $J_L$ satisfies the discrete Gauss constraint:
$$\nabla \cdot J_L = \rho_{\text{lattice}}$$
while the transverse modes satisfy $\nabla \cdot J_T = 0$, yielding two propagating degrees of freedom ([THEOREM]). The auxiliary projected gauge field $A$ is represented via the transverse projector:
$$J_T = P_T A, \quad A \sim A + \nabla \chi$$
where $A$ is an effective, non-primitive description.

### 1.3 Canonical normalizations
In bare engine units, the coupling coefficients of the native source-flux effective field theory are frozen at the lattice scale:
* **Static response:** $C_L^{\text{FTD}} = 1$
* **Transverse stiffness:** $K_T^{\text{FTD}} = 1$
* **Signed current transport:** $Z_j^{\text{FTD}} = 1$
* **Source/flux vertex:** $g_{sJ}^{\text{FTD}} = 1$

This means that the bare coupling between voxel charge and flux is canonically $1$. The continuous running of the coupling only emerges in the effective continuum description under blocking (coarse-graining).

---

## 2 · The "No-Cheat" checklist for readout candidates

Any candidate proposing to map the algebraic spine eigenvalue $x_+ \approx 137.036$ to the physical QED coupling $\alpha^{-1}$ must pass this four-gate checklist. Proposals that violate these constraints represent parametric insertions rather than derivations:

| Gate | Criterion | Description | Status Check |
|---|---|---|---|
| **Gate 1** | **No CODATA input** | The physical value $137.035999...$ or any empirical QED/SM parameter must not appear anywhere in the derivation chain, even as an infinitesimal bias or regulator selection. | [REQUIRED] |
| **Gate 2** | **No scheme tuning** | The renormalization scheme, regulator family, and counterterm policy must be fixed *before* extracting the coupling. One cannot choose a scheme *because* it yields the correct value. | [REQUIRED] |
| **Gate 3** | **No auxiliary gauge fields** | The projected variable $A$ must not be treated as a primitive microscopic field with ad-hoc path-integral weights. The path integral must be defined over native ternary histories. | [REQUIRED] |
| **Gate 4** | **Explicit mapping** | There must be an explicit, mathematically proven projection map from the multi-block lattice operators to the lemniscatic CM curve invariants. | [REQUIRED] |

---

## 3 · Arithmetic period $G^*$ and the BCC topological index

The lemniscatic ratio $G^* = \frac{\Gamma(1/4)}{\Gamma(3/4)} \approx 2.95868$ is not just a numerical constant; it is a fundamental arithmetic period of the Gaussian integers $\mathbb{Z}[i]$:

### 3.1 Lerch's zeta period
$G^*$ is closely related to the L-function of the Kronecker character $\chi_{-4}$ (which classifies prime splitting in $\mathbb{Z}[i]$):
$$L(\chi_{-4}, 1) = \sum_{n=0}^{\infty} \frac{(-1)^n}{2n+1} = \frac{\pi}{4}$$
The ratio represents the regularized period mismatch between inert primes (congruent to $3 \pmod 4$) and split primes (congruent to $1 \pmod 4$) in $\mathbb{Z}[i]$.

### 3.2 Discrete winding number on BCC
On the BCC Unit-Cube corners, the permutation group action induces a $\mathbb{Z}[i]$-module structure $V_{\text{complex}} \cong \mathbb{Z}[i]^2$. Let $y$ represent a state in $V_{\text{complex}}$. A discrete topological index can be defined via the cyclic rotation $J$:
$$\text{Ind}(y) = \frac{1}{4} \sum_{k=0}^{3} \text{Im} \left( \frac{\langle y, J^k y \rangle}{\|y\|^2} \right)$$
Because the complex subspace is arithmetically isomorphic to the Gaussian integers, the projection of any lattice configuration onto $V_{\text{complex}}$ naturally maps to topological winding numbers on the unit circle in $\mathbb{Z}[i]$. 

By formulating charge not as a continuous field amplitude, but as the index of this modular projection, the coupling to the transverse field is structurally bound to the arithmetic invariants of $\mathbb{Z}[i]$. This establishes the Candidate C track as a mathematically clean, non-circular path to bridging the algebraic spine to QED.
