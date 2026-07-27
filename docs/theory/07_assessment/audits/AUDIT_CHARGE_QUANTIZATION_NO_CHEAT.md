# AUDIT · Charge Quantization and QED Normalization Integrity (ARC-C1)

**Tag:** [AUDIT] / canonical
**LEDGER row:** FTD-0231 (new methodological audit claim)
**Depends on:** FTD-0152 (Alpha Readout Contract), FTD-0153 (Math-First Ontology)
**Status:** [AUDIT] complete; establishes the mathematical and methodological boundary separating QED imports from native FTD observables.

> **FTD-0584 correction (2026-07-26):** the earlier claim that ternary
> discreteness or the module `V_complex ~= Z[i]^2` by itself establishes
> topological electric-charge quantization is false. A discrete site label is
> not a loop, bundle class, or invariant of the production transition graph.
> The `C4` representation decomposition survives; the charge/winding
> interpretation below is corrected to a selected candidate requiring an
> explicit history-to-loop map and dynamical conservation.

---

## 0 · Executive summary

To prevent circularity or post-hoc parameter tuning in a proposed fine-structure constant ($\alpha$) readout, this audit establishes a "no-cheat" boundary between imported QED quantities and FTD-native variables. It distinguishes the primitive ternary polarity label from a conserved electric charge and supplies a non-circular checklist for any future physical readout proposal.

Furthermore, we explore the arithmetic origin of the lemniscatic ratio $G^* = \Gamma(1/4)/\Gamma(3/4)$ and a **candidate** winding construction on the Body-Centered Cubic (BCC) representation. No native map from a production history to that winding is derived here.

---

## 1 · The native charge-quantization boundary

In standard Quantum Electrodynamics (QED), the electric charge $e$ is a continuous parameter that runs with energy scale under the Renormalization Group (RG). Its low-energy value $e_{\text{phys}} = \sqrt{4\pi\alpha_{\text{QED}}}$ is an empirical input.

In FTD, the microscopic ontology contains a discrete polarity alphabet. That fact does not by itself induce a conserved or topological electric charge:

### 1.1 Microscopic state alphabet
Voxel states are ternary: $s(x, t) \in \{-1, 0, +1\}$ ([AXIOM] 3). The signed source density is defined as:
$$\rho_{\text{lattice}}(x, t) = s(x, t)$$
which represents positive, void, or negative **polarity** in the primitive ontology. Thus `s` has unit-spaced labels and no continuous value. Calling this label electric charge requires an additional conserved-current/readout derivation. Genesis and weak transmutation change the registered additive signed-state feature, and FTD-0421 finds nullity zero in the registered four-feature additive basis.

### 1.2 The projected U(1) gauge field
In the selected matched-face sidecar, one may impose the discrete Gauss constraint:
$$\nabla \cdot J_L = \rho_{\text{lattice}}$$
while transverse modes satisfy $\nabla \cdot J_T = 0$. This is a field decomposition/selected coupling, not a proof that `s` is a conserved U(1) charge. The auxiliary projected gauge field $A$ is represented via the transverse projector:
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

### 3.2 Candidate winding number on BCC
On the BCC Unit-Cube corners, the permutation group action induces a $\mathbb{Z}[i]$-module structure $V_{\text{complex}} \cong \mathbb{Z}[i]^2$. Let $y$ represent a state in $V_{\text{complex}}$. A discrete topological index can be defined via the cyclic rotation $J$:
$$\text{Ind}(y) = \frac{1}{4} \sum_{k=0}^{3} \text{Im} \left( \frac{\langle y, J^k y \rangle}{\|y\|^2} \right)$$

That expression is a scalar of one projected state, not the winding number of a closed loop. The module isomorphism alone supplies no canonical map

$$\gamma:S^1\longrightarrow V_{\text{complex}}\setminus\{0\}$$

from an engine history, and it supplies no proof that such a loop is preserved by production transitions. Only after an explicit nonvanishing closed loop is constructed does the standard integral $(2\pi)^{-1}\oint_\gamma d\theta$ define an integer winding.

Therefore Candidate C is an underdetermined selected readout proposal. It does not yet bind primitive polarity, a conserved field current, or the arithmetic period into one native mechanism.
