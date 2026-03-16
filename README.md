# Foundational Ternary Dynamics

[![CI](https://github.com/williamcpaci-tani/Foundational-Ternary-Dynamics/workflows/CI/badge.svg)](https://github.com/williamcpaci-tani/Foundational-Ternary-Dynamics/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-13.0+-76B900.svg)](https://developer.nvidia.com/cuda-toolkit)

A discrete computational physics framework deriving physical constants from pure mathematics.

---

## Overview

Foundational Ternary Dynamics (FTD) consists of two pillars:

**A mathematical derivation chain** — starting from four constrained integers {3, 4, 7, 13} and the lemniscatic constant G\* ≈ 2.9587, the framework derives ~20 physical constants including the fine structure constant α to 1.26 ppm, particle mass ratios, and neutrino mixing angles.

**A C++ simulation engine** — a 3D cubic lattice where each site holds one of three states {−1, 0, +1}, coupled to a continuous vector flux field J ∈ ℝ³. Six update rules derived from an action principle S[s,J] govern the dynamics. The engine operates at three physics scales (voxel, particle, atom), supports CUDA GPU acceleration , and compiles to WebAssembly for an interactive browser dashboard with 23+ scenarios.

The central result is a quadratic equation — x² − 16G\*²x + 16G\*³ = 0 — whose roots yield 1/α = 137.036 (1.26 ppm from CODATA) and N_c = 3.024 (the number of color charges).

**Author:** William J cpaci-tani III &nbsp;|&nbsp; **Version:** 5.27-neutrino &nbsp;|&nbsp; **Date:** March 2026

---

## Epistemic Notice

FTD maintains honest accounting of what is genuinely derived versus what uses external physics:

| Category | Count | Description |
|----------|-------|-------------|
| **Genuine Derivations** | ~30 | From G\* and integers alone (α, sin²θ_W, mass ratios, mixing angles, neutrino masses) |
| **Parametric Insertions** | ~50 | FTD-derived values inserted into standard physics formulas |
| **External Physics** | ~50+ | Standard Model mechanisms adopted without derivation |

**External inputs required:** M_Planck, G_F, Λ_QCD, decay constants, phase space factors.

See [`docs/theory/AUDIT_EPISTEMIC_AUDIT.md`](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) for the full breakdown.

---

## Quick Start

### Verify the Math

```bash
git clone https://github.com/williamcpaci-tani/Foundational-Ternary-Dynamics.git
cd Foundational-Ternary-Dynamics

pip install numpy scipy sympy mpmath pytest
pytest tests/ -v
```

### Build the Engine

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release
cd engine/build && ctest --output-on-failure -C Release
# 114 tests: unit tests, physics campaigns, Maxwell equations, energy conservation
```

### Launch the Web Dashboard

```bash
python -m http.server 8080 -d engine/web
# Open http://localhost:8080
```

Three.js visualization with 23+ scenarios across three physics scales, real-time energy diagnostics, Lagrangian inspector, and particle catalog. Five standalone force simulations are also available in [`dissemination/interactive/`](dissemination/interactive/).

### Napkin Calculation

Verify the core result with 6 lines of Python:

```python
from scipy.special import gamma
import numpy as np

G_star = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)   # ≈ 2.9587
a, b, c = 1, -16 * G_star**2, 16 * G_star**3
x_plus = (-b + np.sqrt(b**2 - 4*a*c)) / 2

print(f"1/α derived:      {x_plus:.10f}")   # 137.0361714582
print(f"1/α experimental: 137.035999177")    # 1.26 ppm error
```

---

## The Simulation Engine

### Architecture

The engine implements a two-layer ontology on a 3D cubic lattice:

- **Discrete layer** — each voxel holds a ternary state s ∈ {−1, 0, +1} representing void, positive, or negative manifestation
- **Continuous layer** — a vector flux field J ∈ ℝ³ encoding energy density and wave propagation

All dynamics derive from six rules obtained via the action principle S[s,J]: flux wave equation, state-flux coupling, Gauss projection, manifestation/evaporation thresholds, field-mediated forces, and movement with collision handling.

Every physical constant used in the engine is derived from two inputs — the spatial dimension D = 3 and the lemniscatic constant ϖ — through a 9+ layer ontic chain (`ontic.h`). Nothing is fitted or tuned.

Written in C++17 with CMake. See [`engine/SPEC_ENGINE.md`](engine/SPEC_ENGINE.md) for the full specification.

### Three Scales

| Scale | Engine | Description |
|-------|--------|-------------|
| 0 — Voxel | `RenderBridge` | Discrete lattice dynamics, flux propagation, Gauss constraint, Poisson-based Coulomb |
| 1 — Particle | `ParticleEngine` | Continuous positions, Velocity Verlet integration, analytical EM + gravity |
| 2 — Atom | `AtomEngine` | Composite atoms, ionic / Van der Waals / covalent bonding forces |

Scale bridges provide lossless coarsening and refinement between levels.

### Tick Cycle (Scale 0)

```
phase_read    →  Laplacian wave equation + coupling source
phase_write   →  Leapfrog integration, damping, genesis/evaporation
gauss_project →  SOR Poisson solver enforcing ∇·J = s
phase_forces  →  Coulomb (∇φ) + Lorentz (v×B) + gravity (∇ρ)
phase_movement → Velocity integration, remainder accumulation, collisions
tick++
```

### Pedagogy System

19 runtime `TermToggles` (10 core ON by default + 9 extensions OFF) let users enable or disable individual physics terms — wave propagation, coupling, damping, genesis, Gauss projection, forces, gravity, Poisson solver, movement, Lorentz force, plus extensions for selective damping, Larmor radiation, dual-substrate chirality, color forces, weak transmutation, strong force, triad binding, pair production, and exchange forces.

### CUDA GPU Acceleration

A drop-in `GpuEngine` replacement provides GPU-accelerated simulation with cuFFT spectral Poisson solver (exact Gauss constraint, zero violation) and cuRAND stochastic genesis. 26 GPU physics campaigns validate parity with the CPU engine. Performance: **363× speedup** at 64³ lattice on RTX 5090. Build with `cmake -DFTD_ENABLE_CUDA=ON`.

### WebAssembly Dashboard

The engine compiles to WASM via Emscripten and runs entirely in the browser. The dashboard includes Three.js 3D rendering, tabbed panels (Controls, Diagnostics, Charts, Lagrangian, Inspector), a particle catalog covering the Standard Model, and 25 JavaScript modules powering features from orbital visualization to spectroscopy. No build step required for the frontend — serve static files and open in any modern browser.

### Codebase Size

| Component | Lines |
|-----------|-------|
| Headers (`include/ftd/`) | ~3,560 (17 files) |
| Source (`src/`) | ~3,240 (7 files) |
| CUDA (`cuda/`) | ~3,210 (5 files + CMakeLists) |
| WASM bindings | ~1,390 |
| Web frontend | ~20,000 (25 JS modules + HTML) |
| Tests | 115 files → 114 CTests |

---

## Theoretical Results

### The Master Quadratic

```
x² − 16G*²x + 16G*³ = 0

G* = √2 × Γ(1/4)² / (2π) ≈ 2.9587    (the lemniscatic constant)
16 = |Aut(E)|²                           (automorphisms of E: y² = x³ − x)

x₊ = 137.036...  →  1/α                 (1.26 ppm from experiment)
x₋ = 3.024...    →  N_c = 3             (number of color charges)
```

The four framework integers satisfy interlocking constraints:

| Integer | Value | Role | Constraint |
|---------|-------|------|------------|
| N_c | 3 | Color charges | First Fermat-forbidden exponent |
| N_base | 4 | Base dimension | dim(ℍ) = quaternion dimension |
| b₃ | 7 | QCD beta coefficient | N_c + N_base |
| N_eff | 13 | Effective degrees of freedom | Fibonacci F₇ = b₃ + 2N_c |

### Coupling Constants

| Parameter | Derived | Experimental | Accuracy |
|-----------|---------|--------------|----------|
| 1/α (fine structure) | 137.0362 | 137.0360 | **1.26 ppm** |
| sin²θ_W (Weinberg angle) | 0.2308 | 0.2312 | **0.19%** |
| α_s (strong coupling) | 0.1186 | 0.1179 | **0.6%** |
| α_G (gravitational) | 5.91×10⁻³⁹ | 5.91×10⁻³⁹ | **0.01%** |

### Particle Masses

| Particle | Derived | Experimental | Error |
|----------|---------|--------------|-------|
| Electron | 0.5096 MeV | 0.5110 MeV | **0.27%** |
| Tau | 1776.7 MeV | 1776.9 MeV | **0.007%** |
| Proton | 938.3 MeV | 938.3 MeV | **0.017%** |
| W boson | 80.37 GeV | 80.37 GeV | **0.003%** |

### Neutrino Mixing (PMNS)

| Angle | Derived | Experimental | Error |
|-------|---------|--------------|-------|
| θ₁₂ (solar) | 33.5° | 33.4° | **0.2%** |
| θ₂₃ (atmospheric) | 49.6° | 49.2° | **0.9%** |
| θ₁₃ (reactor) | 8.8° | 8.6° | **2.8%** |
| δ (CP phase) | 66.8° | 68° | **1.8%** |

### Falsification Criteria

1. No fourth generation of fermions with standard gauge couplings
2. Normal neutrino mass hierarchy (not inverted)
3. Proton decay with τ_p ~ 10³⁵ years
4. Tensor-to-scalar ratio r ≈ 0.022
5. No WIMPs, no supersymmetry, no extra dimensions

All predictions are currently compatible with experimental bounds.

---

## Repository Structure

```
Foundational-Ternary-Dynamics/
├── engine/                          # C++ simulation engine
│   ├── include/ftd/                 #   17 headers (ontic.h, constants.h, voxel.h, ...)
│   ├── src/                         #   7 source files (render_bridge.cpp, main.cpp, ...)
│   ├── cuda/                        #   CUDA GPU kernels (5 files)
│   ├── wasm/                        #   Emscripten WASM bindings
│   ├── web/                         #   Browser dashboard (25 JS modules + Three.js)
│   ├── tests/                       #   115 files → 114 CTests
│   └── SPEC_ENGINE.md               #   Living reference document
├── docs/
│   ├── theory/                      #   93 core theory documents (10 categories)
│   ├── reference/                   #   Epistemic labels, symbol glossary, scope
│   ├── internal/                    #   Simulation manual (SPEC_CLAUDE.md)
│   └── papers/                      #   Published/submitted papers
├── dissemination/
│   ├── manuscript/                  #   Quarto book (92 chapters)
│   ├── notebooks/                   #   14 Jupyter tutorials
│   └── interactive/                 #   5 standalone HTML force simulations
├── simulations/                     # Mathematical verification suite
├── tests/                           # Python integration tests (14 files)
├── evaluation/                      # Multi-domain assessment (~90 files)
├── archive/                         # Legacy components (Python engine, Qt GUI, web frontends)
├── META_DOCUMENTATION_MAP.md        # Master catalog — start here
└── CHANGELOG.md                     # Version history
```

---

## Documentation

| If you want to... | Start here |
|-------------------|------------|
| Navigate all project documents | [`META_DOCUMENTATION_MAP.md`](META_DOCUMENTATION_MAP.md) |
| Understand FTD from scratch | [`docs/internal/SPEC_CLAUDE.md`](docs/internal/SPEC_CLAUDE.md) |
| See the core mathematics | [`docs/theory/SPEC_THE_MASTER_QUADRATIC_UNIFIED.md`](docs/theory/SPEC_THE_MASTER_QUADRATIC_UNIFIED.md) |
| Assess what is genuinely derived | [`docs/theory/AUDIT_EPISTEMIC_AUDIT.md`](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) |
| Browse all 93 theory documents | [`docs/theory/META_INDEX.md`](docs/theory/META_INDEX.md) |
| Engine architecture and constants | [`engine/SPEC_ENGINE.md`](engine/SPEC_ENGINE.md) |

The project also includes a [Quarto manuscript](dissemination/manuscript/) (92 chapters across 15 books), [14 Jupyter notebooks](dissemination/notebooks/), and [interactive HTML simulations](dissemination/interactive/).

---

## Testing

**Python** — 14 test files covering coupling constants, mass derivations, mixing matrices, cosmology, and epistemic classification:
```bash
pytest tests/ -v
```

**C++ Engine** — 114 CTests across 115 test files, including unit tests, multi-tick physics campaigns, Maxwell equation verification, energy conservation, Poisson solver validation, and GPU parity checks:
```bash
cd engine/build && ctest --output-on-failure -C Release
```

**CI Pipeline** — five jobs run on every push: pytest (Python 3.10-3.12), ruff + black linting, Quarto manuscript build, physics constant validation, and C++ engine build + test.

---

## Requirements

**Python** >= 3.10:
```
numpy scipy matplotlib sympy mpmath
```

**C++ Engine:** CMake >= 3.20, C++17 compiler
**CUDA** (optional): CUDA 13.0+, compute capability >= 8.9
**WASM** (optional): Emscripten SDK >= 5.0
**Manuscript** (optional): Quarto >= 1.4, TeX Live 2024+

---

## Citation

```bibtex
@book{cpaci-tani2026ftd,
  title     = {Foundational Ternary Dynamics: A Discrete Ontology
               from the Ontic to the Cosmic},
  author    = {cpaci-tani III, William J},
  year      = {2026},
  version   = {5.27},
  note      = {Discrete computational physics framework; ~30 genuine
               derivations from 4 constrained integers and the
               lemniscatic constant}
}
```

---

## License

[MIT License](LICENSE)

---

<p align="center">
<b>FTD v5.27-neutrino</b> &nbsp;|&nbsp; March 2026
</p>
