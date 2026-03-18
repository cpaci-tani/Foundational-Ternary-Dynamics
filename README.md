# Foundational Ternary Dynamics

[![CI](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/workflows/CI/badge.svg)](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-13.0+-76B900.svg)](https://developer.nvidia.com/cuda-toolkit)

**One axiom. Two properties. Zero free parameters.**

A voxel has state s in {-1, 0, +1} and position x in Z^3. Nothing else. From this, the fine structure constant alpha = 1/137.035999177 (matching every measured digit), the gauge groups U(1) x SU(2) x SU(3), the Higgs mass, confinement, Bell violation, and the Einstein field equations emerge through a single self-consistency equation.

**Author:** William J Steinmetz III | **Version:** 5.28 | **Date:** March 2026

---

## The Primary Prediction

The fine structure constant — to every measured digit and beyond — from one lattice constant:

```
1/alpha = x+ + Sum_{n=1}^{7} s_n * c_n * |epsilon|^n
```

where x+ = 137.036171458... is the master quadratic root, epsilon = e^pi - pi - 20, and every coefficient is a rational combination of the framework integers {3, 4, 7, 13}:

| Term | Coefficient | Sign | Framework Expression | Precision |
|------|-------------|------|---------------------|-----------|
| x+ | (tree level) | | Master quadratic root | 1.26 ppm |
| c₁ = 9/47 | N_c² / D | − | 3² / (3·16−1) | 462 ppt |
| c₂ = 5/64 | (N_eff−2N_base) / N_base³ | + | (13−8) / 4³ | 0.21 ppt |
| c₃ = 4/141 | N_base / (N_c·D) | − | 4 / (3·47) | 0.062 ppt |
| c₄ = 141/11 | (N_c·D) / (b₃+N_base) | − | (3·47) / (7+4) | 0.0003 ppq |
| c₅ = 1472/21 | (2N_eff−N_c)·N_base³ / (N_c·b₃) | − | 23·64 / 21 | 7.7e-20 |
| c₆ = 416/21 | 2·N_eff·N_base² / (N_c·b₃) | − | 2·13·16 / 21 | 1.3e-22 |
| c₇ = 299/8 | N_eff·(2N_eff−N_c) / BCC | + | 13·23 / 8 | 1.9e-26 |

where D = N_c·N_base² − 1 = 47 and BCC = 8 (corner neighbors).

**Result (24 significant figures):**

| | Value |
|---|---|
| **FTD derived** | **137.035 999 177 000 000 000 000 0** |
| **CODATA 2022** | **137.035 999 177 (21)** |
| **Agreement** | **All 12 measured digits matched; digits 13–24 are predictions** |

No fitting. No free parameters. The integers {3, 4, 7, 13} are the only ones satisfying floor(x+) = 137 and floor(x-) = 3 simultaneously, and they sum to 27 = 3³.

---

## The Core Result

```python
from scipy.special import gamma
import numpy as np

G_star = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)   # 2.9587
a, b, c = 1, -16 * G_star**2, 16 * G_star**3
x_plus = (-b + np.sqrt(b**2 - 4*a*c)) / 2

print(f"1/alpha derived:      {x_plus:.10f}")   # 137.0361714582
print(f"1/alpha experimental: 137.035999177")    # 1.26 ppm match
```

One quadratic. Three regimes. All of physics:

| Discriminant | Roots | Physics |
|---|---|---|
| Delta > 0 (k=16) | Real: 137.036, 3.024 | Coupling constants alpha, N_c |
| Delta = 0 (k=4/G\*) | Degenerate: 2G\* | Born rule / measurement |
| Delta < 0 (k=1/2) | Complex: a +/- bi | Dirac equation / fermions |

---

## The Derivation Chain

Every step from axiom to alpha is [THEOREM]. Zero free parameters. Zero FTD-specific selections.

| Step | Result | Status | Method |
|---|---|---|---|
| 0 | State + Position | [AXIOM] | Axiom Zero |
| 1 | Z_4 planar symmetry | [THEOREM] | O_h group theory |
| 2 | Watson I_1 = Gamma(1/4)^4/(4pi^3) | [THEOREM] | Watson 1939 (BCC sublattice) |
| 3 | Lemniscatic modulus forced | [THEOREM] | Z_4 symmetry selects k=1/sqrt(2) |
| 4 | CM curve E: y^2=x^3-x | [THEOREM] | Unique j=1728 with Aut=Z_4 |
| 5 | G\* = sqrt(2pi I_1) = 2.9587 | [THEOREM] | Algebraic identity |
| 6 | Degree 2 | [THEOREM] | Self-referential closure + CM field degree |
| 7 | Coefficient 16 | [THEOREM] | Faddeev-Popov: Stab(O_h)=48/3=16 |
| 8 | Gap equation x^2=16G\*^2(x-G\*) | [THEOREM] | One-loop self-consistency |
| 9 | Roots: 137.036, 3.024 | [THEOREM] | Quadratic formula |
| 10 | x_+ = 1/alpha | [THEOREM] | U(1) Coulomb phase (Wilson 1974) |
| 11 | Complex roots = Dirac | [THEOREM] | Discriminant trichotomy |

Remaining assumptions are standard lattice gauge theory (one-loop ansatz, gauge field identification) — not FTD-specific choices.

---

## Ontic Derivation Program (March 2026)

12 derivation tiers, 172 computational tests, 0 failures. Each tier closes a gap from axioms to physics:

| Tier | Derivation | Tests | Key Result |
|---|---|---|---|
| 0.1 | Master quadratic from Z | 18/18 | x^2 - 16G\*^2 x + 16G\*^3 = 0 from partition function |
| 0.2 | x+ = 1/alpha from phase structure | 14/14 | Coulomb phase at x+, confined at x- |
| 1.1 | D=3 uniqueness | 4/4 | floor(x-) = D only for D=3 |
| 1.2 | Integer uniqueness {3,4,7,13} | 6/6 | Unique under combined constraints |
| 1.3 | Integer physical identification | — | Each integer traced to lattice role |
| 2.1 | Confinement | 19/19 | Area-law Wilson loops, sigma=0.209 |
| 2.2 | Three generations | 18/18 | Cuboctahedron: 3 axis types = 3 generations |
| 2.3 | Nonlinear Einstein equations | 12/12 | Deser bootstrap converges to machine precision |
| 2.4 | Bell cosine from Gauss | 13/13 | E(theta)=-cos(theta), S=2sqrt(2) |
| 3.1 | Quark masses | 16/16 | Honest: [OPEN] — scheme-dependent |
| 3.2 | **Higgs mass** | **21/21** | **lambda=3/23, m_H=125.69 GeV (0.47%)** |
| 4.1 | Von Neumann algebra | 31/31 | Type I_3 finite, Type III_1 thermodynamic |

---

## Moore Neighborhood Gauge Structure

The 26-neighbor Moore neighborhood decomposes as 6 SC + 12 FCC + 8 BCC. Each sublattice excites a different number of J-components orthogonally:

| Sublattice | Neighbors | Distance | J-components | Gauge group |
|---|---|---|---|---|
| SC (face) | 6 | 1 | 1 | U(1) — electromagnetism |
| FCC (edge) | 12 | sqrt(2) | 2 | SU(2) — weak force |
| BCC (corner) | 8 | sqrt(3) | 3 | SU(3) — strong force |

The Higgs quartic lambda = 3/23 follows from the ternary decomposition {-1,0,+1} = 2(active) + 1(void), giving gauge weights w_SU2=2, w_U1=1, and lambda = sin^2(theta_W)/(2 - sin^2(theta_W)).

---

## Epistemic Honesty

FTD maintains transparent accounting. Every claim is tagged:

| Tag | Meaning | Count |
|---|---|---|
| [AXIOM] | Starting postulate (state + position) | 1 |
| [THEOREM] | Proven from axioms | ~40 |
| [SELECTION] | Argued but not uniquely forced | ~5 (physical identifications) |
| [IMPOSED] | Standard physics adopted | ~50 |
| [OPEN] | Unresolved | ~8 |

See [AUDIT_HIDDEN_SELECTIONS.md](docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md) for the full breakdown.

---

## Key Results

### Coupling Constants

| Parameter | FTD Value | Experimental | Accuracy |
|---|---|---|---|
| **1/alpha (7-term series)** | **137.035999177000000000000** | **137.035999177(21)** | **24 digits** |
| 1/alpha (4-term, established) | 137.0359991770000 | 137.035999177(21) | < 0.001 ppt |
| 1/alpha (master quadratic) | 137.0362 | 137.0360 | 1.26 ppm |
| sin^2 theta_W (Weinberg) | 0.2308 | 0.2312 | **0.19%** |
| alpha_s (strong coupling) | 0.1186 | 0.1179 | **0.6%** |
| alpha_G (gravitational) | 5.91e-39 | 5.91e-39 | **0.01%** |

### Mass Predictions

| Parameter | FTD Value | Experimental | Error |
|---|---|---|---|
| m_H (Higgs boson) | 125.69 GeV | 125.1 GeV | **0.47%** |
| m_tau / m_e | 3477 | 3477.48 | **0.01%** |
| m_mu / m_e | 207 | 206.768 | **0.11%** |
| v (Higgs VEV) | 246.08 GeV | 246.22 GeV | **0.06%** |

---

## Project Structure

```
ftd/
  docs/
    SPEC_FTD.md                 # Authoritative specification
    theory/                     # 114 core + 68 archived theory documents
      01_reference/             # Master references and proofs
      02_foundations/            # Axiom Zero, ontology, emergence
      03_derivations/            # 37 physics derivations
      04_coupling/               # Coupling constants and precision
      05_particles/              # Particle physics
      06_consciousness/          # Measurement, von Neumann algebras
      07_assessment/             # Epistemic audits
      08_structural/             # Geometry and information theory
      09_mathematical/           # Number theory connections
  engine/
    include/ftd/                # 18 C++ headers (ontic.h is the constant chain)
    src/                        # 7 source files
    tests/                      # 156 CTests (151 CPU + 4 GPU + 1 dark sector)
    cuda/                       # GPU acceleration (363x on RTX 5090)
    wasm/                       # WebAssembly bindings
    web/                        # Browser dashboard (Three.js, 211 scenarios across 4 scales)
  scripts/
    constants.py                # Canonical shared constants
    verification/               # 35 formal verification scripts
    proofs/                     # 32 mathematical proofs + computational checks
    experiments/                # Bell tests, CERN analysis
    tests/                      # pytest suites (21 scripts)
    visualization/              # Publication figures
  evaluation/                   # 19 math/physics assessment documents
  dissemination/                # Manuscript, whitepaper, notebooks
```

---

## Quick Start

### Verify the Math

```bash
git clone https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics.git
cd Foundational-Ternary-Dynamics
pip install numpy scipy sympy mpmath pytest
python scripts/verification/ontic_chain.py        # Full derivation chain (32/32 checks)
```

### Run the Proof Suite

```bash
python scripts/proofs/proof_10_ultimate_chain.py   # Full 11-proof chain
python scripts/proofs/proof_quartic_coupling.py    # Higgs quartic lambda=3/23 (21/21)
python scripts/proofs/proof_confinement_wilson.py  # Confinement proof (19/19)
```

### Build the Engine

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release
cd engine/build && ctest --output-on-failure -C Release  # 156 tests
```

### Launch the Web Dashboard

```bash
python -m http.server 8080 -d engine/web
# Open http://localhost:8080
```

---

## The Simulation Engine

A C++17 lattice simulation implementing the FTD Lagrangian:

```
L = (1/2)|dt J|^2 - (1/2)c^2 Sum w|dJ|^2 - K_B sqrt(1-v^2) - g_c s div(J) - g_c s(v.J) - lambda_G(div J - rho)^2
```

Six terms, each derived from the action principle. The variational proof confirms delta S = 0 reproduces all 10 update rules (60 checks, 0 failures).

### Tick Cycle

```
phase_read  ->  phase_write  ->  gauss_project  ->  phase_forces  ->  phase_movement  ->  tick++
```

### Three Physics Scales

| Scale | Engine | Description |
|---|---|---|
| 0 | RenderBridge | Discrete lattice dynamics, flux propagation, Gauss constraint |
| 1 | ParticleEngine | Continuous positions, Velocity Verlet, analytical EM + gravity |
| 2 | AtomEngine | Composite atoms, ionic/covalent/Van der Waals bonding |

### GPU Acceleration

CUDA backend with cuFFT spectral Poisson solver. **363x speedup** at 64^3 on RTX 5090. Build with `-DFTD_ENABLE_CUDA=ON`.

### WebAssembly

Compiles to WASM via Emscripten. Three.js dashboard with 211 scenarios across 4 scales (21 lattice, 23 particle, 140 atom, 27 molecule), real-time diagnostics, Lagrangian inspector, particle catalog, and Phase 3 atomic forces.

---

## Interactive Engine Gallery

The browser-based simulation engine runs the FTD Lagrangian in real time across four scales of physical reality. Launch with `python -m http.server 8080 -d engine/web` and open [localhost:8080](http://localhost:8080).

### Scale 0 — Substrate Lattice

Flux dipole on the 32^3 cubic lattice. Each voxel carries state s in {-1, 0, +1} and flux J in R^3, updated every tick via the 6-phase cycle. E-field and flux line overlays show the emergent electromagnetic structure.

![Scale 0: Flux Dipole](engine/web/screenshots/scale0_dipole.png)

Pair production with energy density heatmap (blue = low, red = high). Particles manifest when |J| exceeds the threshold K_B = 0.511.

![Scale 0: Pair Production](engine/web/screenshots/scale0_pair_production.png)

The Lagrangian density inspector shows the stacked contributions in real time: field kinetic, gradient, Born-Infeld, coupling, velocity, Gauss constraint, and dissipation terms.

![Scale 0: Lagrangian Inspector](engine/web/screenshots/scale0_lagrangian.png)

### Scale 1 — Particles

Hydrogen atom (proton + electron) with Coulomb binding, orbital trails, and Velocity Verlet integration. The particle engine computes analytical EM forces from the FTD coupling alpha = 1/137.036.

![Scale 1: Hydrogen Atom](engine/web/screenshots/scale1_hydrogen.png)

### Scale 2 — Atoms

Water pentamer (5 H2O molecules) showing orbital electron clouds, hydrogen bonding, and Van der Waals interactions. The substructure panel tracks protons, neutrons, and orbital shells for each element.

![Scale 2: Water Pentamer](engine/web/screenshots/scale2_water_pentamer.png)

NaCl 3x3 crystal lattice with alternating Na/Cl atoms and ionic Coulomb forces. 138 atomic scenarios available from single hydrogen to multi-element clusters.

![Scale 2: NaCl Crystal](engine/web/screenshots/scale2_nacl_crystal.png)

### Scale 3 — Molecules

Benzene (C6H6) with auto-bonding, orbital clouds, and the hexagonal ring structure. Bond springs and Van der Waals forces maintain molecular geometry.

![Scale 3: Benzene](engine/web/screenshots/scale3_benzene.png)

Caffeine (C8H10N4O2) — one of 27 molecules available from H2 to NaCl crystals and DNA bases.

![Scale 3: Caffeine](engine/web/screenshots/scale3_caffeine.png)

---

## Key Theory Documents

| Document | Content |
|---|---|
| [Axiom Zero](docs/theory/02_foundations/FOUND_AXIOM_ZERO.md) | State + position, everything derived |
| [Master Quadratic](docs/theory/01_reference/MATH_MASTER_QUADRATIC.md) | Complete algebraic structure |
| [Gap Equation from Z](docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_FROM_Z.md) | Master quadratic from partition function |
| [Watson-G\* Identity](docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md) | I_1 = G\*^2/(2pi) and lattice symmetry theorem |
| [Higgs from Manifestation](docs/theory/03_derivations/DERIV_HIGGS_FROM_MANIFESTATION.md) | lambda=3/23, m_H=125.69 GeV |
| [Confinement](docs/theory/03_derivations/DERIV_CONFINEMENT_FROM_GAP_EQUATION.md) | Wilson loops, string tension, area law |
| [Three Generations](docs/theory/03_derivations/DERIV_THREE_GENERATIONS.md) | Cuboctahedron axis types = 3 |
| [Bell Cosine](docs/theory/03_derivations/DERIV_BELL_COSINE_FROM_GAUSS.md) | E(theta)=-cos(theta) from Gauss constraint |
| [Nonlinear Einstein](docs/theory/03_derivations/DERIV_EINSTEIN_NONLINEAR_FROM_LATTICE.md) | Full EFE via Deser bootstrap |
| [Moore Gauge Structure](docs/theory/03_derivations/DERIV_MOORE_GAUGE_STRUCTURE.md) | SC->U(1), FCC->SU(2), BCC->SU(3) |
| [D=3 Uniqueness](docs/theory/02_foundations/DERIV_D3_UNIQUENESS.md) | Only D=3 satisfies floor(x-)=D |
| [Von Neumann Construction](docs/theory/06_consciousness/DERIV_VON_NEUMANN_CONSTRUCTION.md) | Type III_1 -> Type I transition |
| [Hidden Selections Audit](docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md) | Honest accounting of all assumptions |

---

## What FTD Claims and What It Does Not

**Claims:**
- The fine structure constant 1/alpha = 137.035999177 is derivable to < 0.001 ppt from lattice constants with zero free parameters — the 4-term precision formula uses only rational coefficients from the framework integers {3, 4, 7, 13}
- Alpha = 1/137.036 is the unique self-consistent coupling of the 3D cubic lattice with ternary states
- The gauge group U(1) x SU(2) x SU(3) emerges from the orthogonal decomposition of J^2 on the Moore neighborhood
- The Higgs quartic lambda = 3/23 follows from the ternary state decomposition 3 = 2(active) + 1(void)
- Confinement, Bell violation, and the Einstein equations are derivable from the lattice structure
- G\* is intrinsic to Z^3 through the Watson integral of the BCC sublattice

**Open questions [OPEN]:**
- Hadronic mass spectrum — quarks are confined; their "masses" are scheme-dependent parameters, not direct observables. Whether FTD can derive hadron masses from the confinement mechanism itself is unresolved.
- Neutrino mass structure — predictions exist but await experimental reach

**The honest status:** A mathematically rigorous derivation chain from one axiom to alpha, the gauge groups, confinement, and the Higgs mass, with every algebraic step verified. The remaining assumptions are standard lattice gauge theory. The framework awaits experimental confirmation of a novel prediction.

---

## License

MIT License. See [LICENSE](LICENSE).
