# Ancient History Simulations

**Mathematical models for FTD-based ancient technology hypotheses**

---

## Overview

This directory contains Python simulations that explore how ancient structures might have manipulated flux fields according to the FTD framework. All simulations are **speculative explorations**, not verified physics.

---

## Simulation Files

### 1. `flux_field_dynamics.py`
**Core 3D flux field simulation**

- Wave propagation on cubic lattice
- Flux exclusion zones (approaching 8 THz threshold)
- Standing wave formation
- Gravitational flux gradients

**Key classes:**
- `FluxField`: 3D flux field with J, velocity, and state arrays
- Functions for wave propagation, exclusion, and gravity

**Usage:**
```bash
python flux_field_dynamics.py
```

---

### 2. `pyramid_resonance_sim.py`
**Great Pyramid acoustic and resonance analysis**

- Rectangular cavity mode calculations
- King's Chamber acoustic frequencies
- Pyramid standing wave harmonics
- Harmonic cascade from 8 Hz to 8 THz

**Key functions:**
- `rectangular_cavity_modes()`: EM cavity resonances
- `acoustic_room_modes()`: Chamber acoustics
- `pyramid_standing_waves()`: Geometric resonances
- `simulate_harmonic_cascade()`: Frequency stepping model

**Usage:**
```bash
python pyramid_resonance_sim.py
```

---

### 3. `vacuum_energy_sim.py`
**Zero-point energy and Casimir effect models**

- Casimir force calculations
- Dynamic Casimir effect (photon from vacuum)
- Flux gradient energy extraction
- Vacuum fluctuation statistics

**Key classes:**
- `CasimirSimulator`: Mode exclusion and force
- `DynamicCasimirSimulator`: Photon production
- `FluxGradientExtractor`: Asymmetric extraction

**Usage:**
```bash
python vacuum_energy_sim.py
```

---

### 4. `harmonic_cascade_sim.py`
**Frequency multiplication from Hz to THz**

- Resonator chain modeling
- Nonlinear harmonic generation
- Power transfer through stages
- Pyramid-specific cascade design

**Key classes:**
- `Resonator`: Single resonant stage
- `ResonatorChain`: Coupled cascade
- `CascadeSimulation`: Power flow analysis

**Usage:**
```bash
python harmonic_cascade_sim.py
```

---

### 5. `magnetic_flux_sim.py`
**Magnetic field and vacuum flux coupling**

- Dipole field calculations
- Rotating magnet systems
- Asymmetric configurations
- Searl effect analysis (speculative)

**Key classes:**
- `MagneticFluxSystem`: B-field to J-field mapping
- `RotatingMagnetSystem`: Time-varying fields
- `AsymmetricExtractor`: One-way flux flow
- `SearlEffectSim`: Rotating magnet resonance

**Usage:**
```bash
python magnetic_flux_sim.py
```

---

### 6. `integrated_pyramid_system.py`
**Complete pyramid flux manipulation model**

Brings together all components:
- Pyramid geometry with resonance analysis
- 3D flux field on pyramid-shaped grid
- Operator consciousness interface (sLoop)
- Harmonic analysis and standing waves
- Comparative parameter studies

**Key classes:**
- `PyramidGeometry`: Great Pyramid dimensions
- `PyramidFluxField`: 3D flux inside pyramid
- `OperatorInterface`: Consciousness coupling
- `IntegratedPyramidSimulation`: Full system

**Usage:**
```bash
python integrated_pyramid_system.py
```

---

## FTD Constants Used

All simulations use these FTD framework constants:

| Constant | Value | Meaning |
|----------|-------|---------|
| N_C | 3 | Color charges |
| N_BASE | 4 | Spacetime dimensions |
| B_3 | 7 | QCD beta coefficient |
| N_EFF | 13 | Effective degrees of freedom |
| KB | 0.511 | Manifestation threshold (MeV) |
| F_EXCLUSION | 8 THz | Flux exclusion frequency |
| ALPHA | 1/137.036 | Fine structure constant |
| PHI | 1.618... | Golden ratio |

---

## Output

All simulations save results to `../output/` as JSON files:

- `flux_field_results.json`
- `pyramid_resonance_results.json`
- `vacuum_energy_results.json`
- `harmonic_cascade_results.json`
- `magnetic_flux_results.json`
- `integrated_pyramid_results.json`

---

## Running All Simulations

To run all simulations:

```bash
cd simulations
python flux_field_dynamics.py
python pyramid_resonance_sim.py
python vacuum_energy_sim.py
python harmonic_cascade_sim.py
python magnetic_flux_sim.py
python integrated_pyramid_system.py
```

Or create a batch runner:

```python
# run_all.py
import subprocess
import sys

simulations = [
    "flux_field_dynamics.py",
    "pyramid_resonance_sim.py",
    "vacuum_energy_sim.py",
    "harmonic_cascade_sim.py",
    "magnetic_flux_sim.py",
    "integrated_pyramid_system.py"
]

for sim in simulations:
    print(f"\n{'='*70}\nRunning {sim}\n{'='*70}\n")
    subprocess.run([sys.executable, sim])
```

---

## Key Findings

### 1. The 8 Hz → 8 THz Bridge
Requires 10^12 amplification (12 orders of magnitude).
Using 12-15 resonant stages with ~10× gain each is theoretically possible.

### 2. Pyramid Geometry Resonance
The Great Pyramid encodes π, φ, and 2π in its proportions.
These create natural resonance conditions for specific frequencies.

### 3. Casimir as Proof of Concept
The Casimir effect demonstrates vacuum energy is real and extractable.
Dynamic Casimir creates photons from vacuum (proven 2011).

### 4. Consciousness Coupling
Operator coherence affects flux concentration in simulations.
Higher coherence → stronger resonance → approaching threshold.

### 5. Asymmetric Extraction
Asymmetric boundary conditions create flux gradients.
Gradients could drive flux flow through a "load."

---

## Disclaimer

**These simulations are SPECULATIVE EXPLORATION.**

They demonstrate that FTD-consistent mechanisms for:
- Vacuum energy extraction
- Harmonic frequency multiplication
- Flux concentration via geometry
- Consciousness-flux coupling

are **mathematically coherent** within the framework.

They do NOT prove:
- Ancient structures actually worked this way
- These mechanisms can be engineered today
- Any over-unity claims are valid

Use for hypothesis generation and theoretical exploration only.

---

## Dependencies

- Python 3.8+
- NumPy
- (Optional) Matplotlib for visualization
- (Optional) SciPy for advanced analysis

---

*Part of the FTD Ancient History Special Project*
