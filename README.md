# Foundational Ternary Dynamics (FTD)

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![Engine v2.18.0](https://img.shields.io/badge/engine-v2.18.0-orange.svg)](engine/SPEC_ENGINE.md)

Foundational Ternary Dynamics (FTD) is a discrete, logic-first computational framework that models physical space as a three-dimensional cubic lattice $\Lambda \subset \mathbb{Z}^3$, where each site ("voxel") takes on a ternary state $s \in \{-1, 0, +1\}$. Dynamics are governed by strict local update rules within a 26-connected Moore neighborhood, with information propagating at a maximum velocity of one lattice unit per discrete time step. The framework aims to explore the deep algebraic structures that emerge naturally from a discrete ontology, identify the exact boundary of what discreteness alone can determine, and enforce absolute epistemic discipline.

---

## 1. Epistemic Discipline and Tag System

Every load-bearing claim in this codebase and theory corpus is tagged explicitly to reflect its rigorous standing. There are three core rules of epistemic discipline that govern all research in this repository:
1. **No Numerical Near-Miss Searches:** We do not scan for numerical coincidences. Substitution identities (plugging FTD values into standard physics formulas) are strictly tagged `[PARAMETRIC]`, never `[DERIVED]`.
2. **Hash-Locked Pre-Registrations:** Before executing any computational scan or measurement campaign, the launcher script's SHA-256 hash is committed to a `preregister-*` git tag. Results are evaluated strictly against the pre-registered protocol.
3. **Preservation of Closed-Negative Results:** Hypotheses that are falsified are never silently deleted; they are preserved with a `[CLOSED NEGATIVE]` tag in the canonical ledgers to map out the boundaries of the discrete model.

### Epistemic Taxonomies

| Tag | Rigorous Definition | Reviewer Expectation |
| :--- | :--- | :--- |
| **`[AXIOM]`** | A structural postulate of the framework that is not derivable. | Accepted as model definition. |
| **`[THEOREM]`** | A proposition rigorously proven from the core axioms with no free parameters. | Verify deductive proof. |
| **`[DERIVED]`** | Established by a verified chain of steps, but contingent on specific physical assumptions. | Review steps and preconditions. |
| **`[SELECTION]`** | Selected based on consistency or structural uniqueness, not uniquely proven. | Review consistency arguments. |
| **`[STRONGLY MOTIVATED CONJECTURE]`** | A conjecture backed by strong multi-route convergence or unique mathematical scans. | Demand further derivation. |
| **`[NUMERICAL FACT]`** | Verified by computational execution over a specified finite domain. | Run verification scripts. |
| **`[PARAMETRIC]`** | Standard physics formula filled with FTD constants (a calibration input, not a derivation). | Note as parameter choice. |
| **`[CLOSED NEGATIVE]`** | Preserved historical failure; tested and falsified. | Do not re-attempt. |

---

## 2. The Bedrock: The Algebraic Spine (Theorems T1–T9)

The algebraic spine represents the rock-solid mathematical foundation of FTD, independent of any dynamical stencils or physical conjectures. The canonical reference is [`SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md).

| Theorem | Name | Physical / Algebraic Description | Proof Document | Verification Script |
| :--- | :--- | :--- | :--- | :--- |
| **T1** | Lemniscate Uniqueness | Lemniscatic domain representation via Kronecker character $\chi_{-4}$ and Joint Domain representation. $G^* = \Gamma(1/4)/\Gamma(3/4) \approx 2.95867512$. Note: $G^* \neq \varpi$ (Gauss lemniscate constant $\approx 2.62205755$). | [`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`](docs/theory/01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md) | `proof_harmonic_invariant_tower.py` |
| **T2** | Fine Structure Constant | The master quadratic polynomial $P(x) = x^2 - 16G^{*2}x + 16G^{*3}$ yields a larger root $x_+ \approx 137.036$, matching $\alpha^{-1}$ to 1.26 ppm. | [`MATH_MASTER_QUADRATIC.md`](docs/theory/01_reference/MATH_MASTER_QUADRATIC.md) | `proof_07_master_quadratic.py` |
| **T3** | Loop Coefficients | Tree, one-loop, and multi-loop coefficients ($c_1, c_2, c_3$) derived from lattice Feynman diagram sums. | [`DERIV_ONE_LOOP_LATTICE_ALPHA.md`](docs/theory/04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md) | `proof_03_critical_coupling.py` |
| **T4** | Electron Mass Formula | Prefactor 16 derived from $\|Aut(E)\|^2$ of the elliptic curve $y^2 = x^3 - x$. Expresses $m_e = m_P \sqrt{2\pi} \frac{16}{3} \alpha^{11}$ to 0.19% error. | [`DERIV_ELECTRON_MASS_MOTIVATION.md`](docs/theory/05_particles/DERIV_ELECTRON_MASS_MOTIVATION.md) | `proof_complete_sm.py` |
| **T5** | Higgs Mass & Coupling | Emergent Higgs mass $m_H = (N_{eff}/\alpha^2) \cdot m_e \approx 124.8$ GeV and self-coupling $\lambda_H = m_H^2 / (2v^2)$ via RG stencils. | [`DERIV_COMPLETE_PARTICLE_PHYSICS.md`](docs/theory/05_particles/DERIV_COMPLETE_PARTICLE_PHYSICS.md) | `proof_complete_sm.py` |
| **T6** | Proton Mass Ratio | Proton mass ratio $m_p/m_e = N_{eff}/\alpha + N_{base} \cdot N_{eff} + N_c \approx 1836.47$ (174 ppm). | [`EXPLR_FTD_MASS_CHAIN.md`](docs/theory/05_particles/EXPLR_FTD_MASS_CHAIN.md) | `proof_complete_sm.py` |
| **T7** | Electron $g-2$ | anomalous magnetic moment $a_e$ computed to 5-loop order matching $a_e = 2.55$ ppb. | [`DERIV_COMPLETE_PARTICLE_PHYSICS.md`](docs/theory/05_particles/DERIV_COMPLETE_PARTICLE_PHYSICS.md) | `proof_complete_sm.py` |
| **T8** | Lamb Shift Quantization | The emergent Lamb shift in the hydrogenic spectrum yielding $1055.4$ MHz (0.23% from experiment). | [`THEOREM_HARMONIC_INVARIANT_TOWER.md`](docs/theory/03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md) | `proof_harmonic_invariant_tower.py` |
| **T9** | Color Charge Selection | Topological quantization on the 26-Moore neighborhood selecting gauge groups U(1)×SU(2)×SU(3) and $N_c = 3$. | [`DERIV_NC_FROM_TOPOLOGY.md`](docs/theory/03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md) | `proof_complete_sm.py` |

---

## 3. Core Axioms & Postulates

Operational simulations and the underlying mathematical ontology of FTD are grounded in a rigid set of core postulates and deep axioms.

### Operational Postulates
* **Discrete Space (Postulate 1):** Space is modeled as a 3D cubic lattice $\Lambda$ with no defined boundary (undefined-boundary ontology, avoiding the mathematical commitment to a completed-infinity $\mathbb{Z}^3$).
* **Discrete Time (Postulate 2):** Time advances in uniform discrete steps ("ticks" $t \in \mathbb{N}$), implying absolute simultaneity at the substrate level.
* **Ternary States & Two-Layer Ontology (Postulate 3):** Ontic variables consist of a continuous dispositional flux field $\mathbf{J} \in \mathbb{R}^3$ and a discrete manifestation state $s \in \{-1, 0, +1\}$. Manifestation is determined when the local flux magnitude crosses a threshold ($|\mathbf{J}| \geq K_B$). The ternary values represent real projections of the unit group of Gaussian integers extended by zero: $\{i^2, 0, |i^2|\}$.
* **Local Causality (Postulate 4):** Updates to a voxel depend strictly on its local 26-neighbor Moore neighborhood, establishing the causal propagation speed limit $C = 1$ lattice unit per tick.
* **Determinism (Postulate 5):** The system's evolution is entirely deterministic; apparent quantum-like randomness is epistemic.

### Deep Mathematical Axioms
* **Axiom A1 (First Distinction):** $0 = (+1) + (-1)$, representing identity, inverse, and conservation of distinction.
* **Axiom A2 (Self-Reference Requirement):** There exists a self-observation map $\sigma: \Omega \to \Omega$ such that $\sigma(\Omega) \subseteq \Omega$. When the self-reference map is constrained by rotational symmetry ($\sigma^4 = \text{id}, \sigma^2 \neq \text{id}$), complex numbers $\mathbb{C}$ are mathematically necessitated.

---

## 4. Epistemic Demarcation & The Decoupling Wall

While the algebraic spine (T1–T9) is mathematically robust, FTD maintains a rigorous demarcation between proven mathematical identities and physical conjectures.

### The Physical Conjectures
* **$\alpha^{-1} = x_+$:** The identification of the master quadratic's root $x_+$ with the physical fine structure constant $\alpha^{-1}$ is classified strictly as a **`[STRONGLY MOTIVATED CONJECTURE]`** ([OT-5.1](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md), [LEDGER FTD-0013](docs/theory/07_assessment/core_ledgers/LEDGER.md)). It is *not* `[DERIVED]` because a rigorous forward-dynamical mapping is an open challenge.

### The Decoupling Wall
The primary transition barrier from FTD's static period algebra (such as $G^*$ and the master quadratic) to emergent continuous dynamics is defined by the **Decoupling Wall**:
* **The Commutativity Wall:** The ternary discrete lattice is fundamentally a commutative ring of period algebra, whereas continuous quantum mechanics and relativity require non-commutative operator algebras.
* **W-CRIT-2 Obstruction:** In any dynamical readout attempt, trace and determinant are independent operator invariants of a $2 \times 2$ matrix. Current first-principles dynamics do not uniquely force the pairing of trace $= 16G^{*2}$ and determinant $= 16G^{*3}$ over the set of static scalars $\{16, G^*, G^{*2}\}$. Thus, the readout structure remains *imposed*, not *forced*.

---

## 5. Closed-Negative Catalog (Limits of Discreteness)

Per the mandate that falsified paths are valuable scientific boundaries, the following results are locked as `[CLOSED NEGATIVE]` in [`LEDGER.md`](docs/theory/07_assessment/core_ledgers/LEDGER.md):
* **Retired $\alpha$ Derivation Routes:** Eleven distinct dynamical routes (including transverse stiffness, Langevin equipartition, and monomial scans) have been conclusively closed as insufficient to derive $\alpha$ from discrete stencils alone.
* **Yilmaz Metric Invalidation (FTD-0184):** Any attempt to formulate gravity on a parallel exponential-metric discrete ontology is falsified; the Yilmaz-type metric fails to satisfy diff-invariance.
* **6-Neighbor Born Rule Failure (FTD-0200):** Stencils limited to 6-neighbor stasis are incapable of recovering the continuous Born rule; the 26-neighbor Moore neighborhood is the minimal causally-viable boundary.
* **Scale 0 Clock Dilation No-Go (FTD-0208):** Attempts to derive the quadratic proper-time relation $(d\tau/dt)^2 + v^2 = 1$ natively at Scale 0 are structurally impossible due to the discrete $L^\infty$ metric. Clock dilation is an emergent Scale 5 macroscopic feature.

---

## 6. Upper & Lower Limitations (What is Still Missing)

To map out the limits of the framework and prevent overclaiming, FTD explicitly catalogs the open boundaries of the current ontology:

### Lower Limitations (Substrate Discreteness)
* **Approximate Lorentz Invariance:** Lorentz invariance is fundamentally broken at the substrate level by the preferred frame of the cubic lattice. Rotational and boost symmetries only emerge relationally in the long-wavelength limit ($\lambda \gg a$) for momenta $|p| \ll \pi/a$, carrying an error of $O((a/\lambda)^2)$.
* **Lattice Spacing Gauge Freedom:** The physical lattice spacing $a_{\text{phys}}$ is a gauge degree of freedom and is not derivable from FTD axioms. Setting $a_{\text{phys}} \equiv \ell_P$ is an *interpretive calibration choice* (model calibration), not an emergent prediction of the dynamics.
* **Substrate Bell Bound:** The deterministic, local updates of the lattice substrate strictly satisfy the classical Bell inequality ($S \le 2$). Quantum correlations violating this bound ($S = 2\sqrt{2}$) are statistical ensemble properties of aggregate observer-measurement frameworks, not fundamental substrate dynamics.

### Upper Limitations & Absent Physics
* **Non-Abelian Gauge Fields:** While Abelian $U(1)$ electrodynamics emerge naturally from divergence constraints (Gauss law), non-Abelian $SU(2)$ electroweak and $SU(3)$ color gauge fields are not dynamically forced. Although they are modeled geometrically (e.g., via spatial axis alignments), a rigorous dynamical derivation of non-Abelian gauge fields is missing.
* **Nonlinear Gravity Curvature:** The weak-field limit of flux gradients recovers Newtonian gravity and linearized general relativity. However, full general relativistic spacetime curvature, strong-field gravity (e.g., Kerr/Schwarzschild metrics), and a unified quantum gravity framework remain open theoretical goals.
* **QFT Vacuum & Renormalization:** FTD lacks a dynamical renormalization group framework and a virtual particle vacuum structure in the quantum field theory sense. Particle masses and coupling parameters are mapped and calibrated, but are not dynamically forced from first principles.

---

## 7. Audience Reading Paths

To assist reviewers and collaborators of varying backgrounds, the following paths are recommended:

| Audience | Primary Entry Point | Deep Dive |
| :--- | :--- | :--- |
| **Mathematician** | [`SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) | [`PAPER_GSTAR_INTRODUCTION.pdf`](docs/papers/PAPER_GSTAR_INTRODUCTION.pdf) |
| **Physicist** | [`SPEC_DOCTRINE_LEDGER.md`](docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md) | [`PAPER_FTD_AS_WILSONIAN_EFT.pdf`](dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.pdf) |
| **Skeptic / Reviewer** | [`AUDIT_EPISTEMIC_AUDIT.md`](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) | [`FTD_Whitepaper.pdf`](dissemination/whitepaper/FTD_Whitepaper.pdf) |
| **Programmer** | [`engine/SPEC_ENGINE.md`](engine/SPEC_ENGINE.md) | [`PAPER_GSTAR_FTD_BRIDGE.pdf`](docs/papers/PAPER_GSTAR_FTD_BRIDGE.pdf) |

---

## 8. Golden Verification Guidelines

FTD enforces full reproducibility through continuous verification of both the C++ simulation engine and the Python mathematical proofs.

### CPU Parallel Build & Test
The host C++ simulation engine leverages high parallelization. Maximize your core load (e.g. on AMD 9950X3D) using these flags:
```bash
# Build simulation engine
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release --parallel 24

# Execute parallel unit tests
cd engine/build
ctest -j 24 --output-on-failure -C Release
```

### GPU Execution via WSL2
RTX 5090 acceleration (~30× speedup) is strictly supported via the WSL2 Ubuntu-22.04 environment.
```bash
# Execute the golden-tick regression benchmark on GPU
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
    cmake --build engine/build_wsl --target test_render_bridge_golden -j 8 && \
    engine/build_wsl/test_render_bridge_golden"
```

### Python Mathematical Proof Suite
The entire mathematical spine is checked via a 54-test battery.
```bash
# Install mathematical prerequisites
pip install numpy scipy sympy mpmath pytest

# Run the master proof verification script
python -m pytest scripts/tests/
python scripts/proofs/proof_master_verification.py
```

### Golden Regression Hash
Bit-exact correctness across CPU and GPU architectures is enforced via a regression hash:
* **Golden Hash:** `0xcd957b601d47868a` (evaluated at lattice size $L = 16$).

---

## 7. License and Citation

This repository is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License (**CC BY-NC-SA 4.0**).

To cite this framework in academic publications:
```bibtex
@misc{steinmetz2026ftd,
  author = {William J. Steinmetz III},
  title  = {Foundational Ternary Dynamics},
  year   = {2026},
  note   = {Version 5.40, engine 2.18.0},
  url    = {https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics}
}
```
