# Foundational Ternary Dynamics

[![CI](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/workflows/CI/badge.svg)](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-13.0+-76B900.svg)](https://developer.nvidia.com/cuda-toolkit)

**One axiom. Two properties. Zero free parameters.**

A voxel has state s in {-1, 0, +1} and position x in Z^3. Nothing else. From this, the fine structure constant alpha = 1/137.036, the Dirac equation, the Born rule, and the coupling constants of the Standard Model emerge through a single self-consistency equation.

**Author:** William J Steinmetz III | **Version:** 5.28 | **Date:** March 2026

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

## Epistemic Honesty

FTD maintains transparent accounting. Every claim is tagged:

| Tag | Meaning | Count |
|---|---|---|
| [AXIOM] | Starting postulate (state + position) | 1 |
| [THEOREM] | Proven from axioms | ~30 |
| [SELECTION] | Argued but not uniquely forced | ~5 (physical identifications) |
| [IMPOSED] | Standard physics adopted | ~50 |
| [OPEN] | Unresolved | ~10 |

See [AUDIT_HIDDEN_SELECTIONS.md](docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md) for the full breakdown.

---

## Key Results

### Coupling Constants

| Parameter | FTD Value | Experimental | Accuracy |
|---|---|---|---|
| 1/alpha (fine structure) | 137.0362 | 137.0360 | **1.26 ppm** |
| sin^2 theta_W (Weinberg) | 0.2308 | 0.2312 | **0.19%** |
| alpha_s (strong coupling) | 0.1186 | 0.1179 | **0.6%** |
| alpha_G (gravitational) | 5.91e-39 | 5.91e-39 | **0.01%** |

### Mass Ratios

| Ratio | FTD | Experimental | Error |
|---|---|---|---|
| m_tau / m_e | 3477 | 3477.48 | **0.01%** |
| m_mu / m_e | 207 | 206.768 | **0.11%** |

### The Precision Formula

```
1/alpha = x+ - (9/47)|epsilon| + (5/64)|epsilon|^2 - (4/141)|epsilon|^3 - (141/11)|epsilon|^4
```

where epsilon = e^pi - pi - 20 and all coefficients are rational combinations of {3, 4, 7, 13}. Matches CODATA 2022 central value to < 0.001 ppt (every measured digit).

---

## Project Structure

```
ftd/
  docs/
    SPEC_FTD.md                 # Authoritative specification
    theory/                     # 97 core theory documents
      01_reference/             # Master references and proofs
      02_foundations/            # Axiom Zero, ontology, emergence
      03_derivations/            # 35 physics derivations
      04_coupling/               # Coupling constants and precision
      05_particles/              # Particle physics
      07_assessment/             # Epistemic audits
      08_structural/             # Geometry and information theory
      09_mathematical/           # Number theory connections
  engine/
    include/ftd/                # 18 C++ headers (ontic.h is the constant chain)
    src/                        # 7 source files
    tests/                      # 155 CTests (151 CPU + 4 GPU)
    cuda/                       # GPU acceleration (363x on RTX 5090)
    wasm/                       # WebAssembly bindings
    web/                        # Browser dashboard (Three.js)
  scripts/
    constants.py                # Canonical shared constants
    verification/               # 35 formal verification scripts
    proofs/                     # 25 mathematical proofs + computational checks
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

### Build the Engine

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release
cd engine/build && ctest --output-on-failure -C Release  # 155 tests
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

Compiles to WASM via Emscripten. Three.js dashboard with 23+ scenarios, real-time diagnostics, Lagrangian inspector, and particle catalog. No build step required for the frontend.

---

## Key Theory Documents

| Document | Content |
|---|---|
| [Axiom Zero](docs/theory/02_foundations/FOUND_AXIOM_ZERO.md) | State + position, everything derived |
| [Master Quadratic](docs/theory/01_reference/MATH_MASTER_QUADRATIC.md) | Complete algebraic structure |
| [Gap Equation](docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) | Self-consistency from the lattice |
| [Watson-G\* Identity](docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md) | I_1 = G\*^2/(2pi) and lattice symmetry theorem |
| [Dirac from Quadratic](docs/theory/03_derivations/DERIV_DIRAC_FROM_MASTER_QUADRATIC.md) | Fermions from complex roots |
| [Born Rule Null Cone](docs/theory/02_foundations/FOUND_BORN_RULE_NULL_CONE.md) | i^2+a^2+b^2=0 encodes everything |
| [Coulomb Scattering](docs/theory/03_derivations/DERIV_COULOMB_SCATTERING_AMPLITUDE.md) | First scattering amplitude from FTD |
| [Three Resolutions](docs/theory/03_derivations/DERIV_THREE_RESOLUTIONS.md) | Compact U(1), bare=physical, one loop exact |
| [L-Function Connection](docs/theory/09_mathematical/DERIV_LFUNCTION_GSTAR_CONNECTION.md) | G\* = 8L(E,1)/sqrt(pi) via BSD |
| [Hidden Selections Audit](docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md) | Honest accounting of all assumptions |

---

## What FTD Claims and What It Does Not

**Claims:**
- Alpha = 1/137.036 is the unique self-consistent coupling of the 3D cubic lattice with ternary states
- The Dirac equation, Born rule, and coupling constants emerge from one quadratic's three discriminant regimes
- G\* is intrinsic to Z^3 through the Watson integral of the BCC sublattice

**Does not claim:**
- To replace the Standard Model's computational machinery
- To derive the full particle spectrum from first principles
- To have confirmed novel predictions (best candidates: neutrino hierarchy ~2027, sum m_nu ~2030)

**The honest status:** A mathematically rigorous derivation chain from one axiom to alpha, with every algebraic step verified. The remaining assumptions are standard lattice gauge theory. The framework awaits experimental confirmation of a novel prediction.

---

## License

MIT License. See [LICENSE](LICENSE).
