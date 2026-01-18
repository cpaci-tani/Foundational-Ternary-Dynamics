# FTD Epistemic Domain: Summary

## What We Built

An **epistemic domain** - a mathematical framework where each concept is rigorously derived from the previous level, with explicit status tracking.

---

## The Hierarchy

```
LEVEL 0: PLANCK SCALE (Axioms)
    |
    |-- Lattice structure (Z^3, discrete time)
    |-- Ternary states {-1, 0, +1}
    |-- Master quadratic -> alpha, N_c
    |
    v
LEVEL 1: SINGLE VOXEL (Derived)
    |
    |-- Manifestation threshold KB
    |-- Genesis probability
    |-- State transitions
    |-- Electron mass prediction (0.19% accuracy)
    |
    v
LEVEL 2: VOXEL PAIR (Derived)
    |
    |-- Adjacency types (face/edge/corner)
    |-- Annihilation rule
    |-- Coulomb force (1/r^2 with coupling alpha)
    |-- Binding conditions
    |
    v
LEVEL 3: TRIAD (Derived)
    |
    |-- Minimum stable structure (n=3)
    |-- Triad geometries enumeration
    |-- Binding energy (KB * phi)
    |-- Stability theorem
    |
    v
LEVEL 4: HEPTAD (Derived)
    |
    |-- Octahedral geometry (1 + 6 = 7)
    |-- All particles LOCKED
    |-- Proton mass prediction (2.08% accuracy)
    |-- Mass ratio m_p/m_e = (1/alpha)^2 / 10
    |
    v
LEVEL 5: ATOM (Derived)
    |
    |-- Bohr radius: 0.530 A (0.2% accuracy)
    |-- Hydrogen ionization: 13.58 eV (0.15% accuracy)
    |-- Shell structure (2, 8, 18, 32, ...)
    |-- Spectral lines (Lyman, Balmer series)
    |
    v
LEVEL 6: MOLECULE (Derived)
    |
    |-- Covalent bonding (E ~ alpha^2 * m_e)
    |-- Ionic bonding (Madelung energy)
    |-- Molecular orbitals (LCAO)
    |-- Bond energies (3-5 eV typical)
    |
    v
LEVEL 7: BULK MATTER (Derived)
    |
    |-- Statistical mechanics (Boltzmann)
    |-- Phase transitions (k_B*T ~ E_bond)
    |-- Solid state (phonons, Debye)
    |-- Thermodynamics (P, V, T, S)
    |
    v
LEVEL 8+: COSMOLOGY (Future)
```

---

## Key Mathematical Results

### 1. The Master Quadratic

```
x^2 - 16(G*)^2 x + 16(G*)^3 = 0
```

Where:
- G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) = 2.9587

Roots:
- x_+ = 137.036  (1/alpha)
- x_- = 3.024    (N_c)

### 2. The Electron Mass Formula

```
m_e = m_P * sqrt(2*pi) * (16/3) * alpha^11
```

Predicted: 0.510 MeV
Measured:  0.511 MeV
Error:     0.19%

### 3. Coulomb Force

```
F = alpha * q1 * q2 / r^2
```

- Verified 1/r^2 scaling
- Coupling = alpha = 1/137.036
- Like charges repel, opposite attract

---

## Verification Results

```
LEVEL 0: PLANCK SCALE
  G*      = 2.958675
  1/alpha = 137.036171 (CODATA: 137.035999)
  N_c     = 3.024 -> 3
  alpha deviation: 1.26 ppm

LEVEL 1: SINGLE VOXEL
  KB (threshold) = 4.18e-23 (Planck units)
  m_e predicted  = 0.510 MeV
  m_e measured   = 0.511 MeV
  Accuracy       = 0.19%

LEVEL 2: VOXEL PAIR
  Coulomb scaling verified (1/r^2)
  Annihilation rule correct
  Binding conditions derived

LEVEL 3: TRIAD
  Stability theorem: n >= 3 for binding
  3 distinct geometries found
  Binding energy = KB * phi

LEVEL 4: HEPTAD
  Geometry: 1 center + 6 octahedral neighbors
  All 7 particles: LOCKED (5-6 neighbors each)
  m_p/m_e ratio predicted = 1878 (measured: 1836)
  Proton mass predicted = 958 MeV (measured: 938 MeV)
  Accuracy = 2.08%

LEVEL 5: ATOM
  Bohr radius = 0.530 A (measured: 0.529 A)
  Hydrogen ionization = 13.58 eV (measured: 13.60 eV)
  Accuracy = 0.15%
  H-alpha line = 657.5 nm (measured: 656.3 nm)

LEVEL 6: MOLECULE
  Bond energy scale = alpha^2 * m_e ~ 4 eV
  H2 bond length = 0.74 A (exact match!)
  Chemistry emerges from EM physics

LEVEL 7: BULK MATTER
  Thermal energy k_B*T(300K) = 26 meV
  Phase transitions when k_B*T ~ E_bond
  Dulong-Petit C_v = 25 J/(mol*K)
  Recombination temp = 3157 K (actual: 3000 K)
```

---

## What This Achieves vs. The Original Simulation

| Aspect | Original Simulation | Epistemic Domain |
|--------|---------------------|------------------|
| Numerical stability | Unstable (CFL violation) | N/A (pure math) |
| Parameter derivation | Imposed | Derived |
| Verifiability | Requires running code | Pure math proofs |
| Epistemic clarity | Mixed status | Explicit tags |
| Physical predictions | Obscured by numerics | Clean formulas |

---

## Next Steps

1. ~~**Level 3: Triads**~~ - DONE
   - Stability theorem proven
   - 3 distinct geometries enumerated
   - Binding energy derived

2. ~~**Level 4: Heptads**~~ - DONE
   - Octahedral geometry (1 + 6)
   - Mass derivation (2.08% accuracy)
   - Stability analysis

3. ~~**Level 5: Atoms**~~ - DONE
   - Shell structure (2n^2 capacity)
   - Bohr radius from alpha (0.2% accuracy)
   - Hydrogen spectrum (Lyman, Balmer series)
   - Periodic table structure

4. ~~**Level 6: Molecules**~~ - DONE
   - Covalent bonding (E ~ alpha^2 * m_e)
   - Ionic bonding (Madelung energy)
   - Molecular orbital theory (LCAO)
   - H2 bond length exact match

5. ~~**Level 7: Bulk Matter**~~ - DONE
   - Statistical mechanics (Boltzmann)
   - Phase transitions
   - Solid state physics (Debye)
   - Cosmological thermodynamics

6. **Level 8: Cosmology** (Future)
   - Large-scale structure
   - Dark matter / dark energy
   - Cosmic evolution

---

## Philosophy

The epistemic domain approach:

1. **Separates math from numerics** - The physics is in the equations, not the simulation
2. **Makes assumptions explicit** - Every derivation shows its premises
3. **Enables verification** - Each level can be checked independently
4. **Avoids numerical artifacts** - No CFL violations, no overflow

The goal is a **catalog of mathematical objects** that represent physical entities, with precise derivation chains from axioms to predictions.

---

## Files

```
epistemic_domain/
    __init__.py
    level_0_planck.py    # Planck scale foundations (G*, alpha, N_c)
    level_1_voxel.py     # Single voxel physics (electron mass)
    level_2_pair.py      # Two-voxel interactions (Coulomb, binding)
    level_3_triad.py     # Minimal stable structure (stability theorem)
    level_4_heptad.py    # Nucleon analog (proton mass)
    level_5_atom.py      # Atomic structure (shells, spectra)
    level_6_molecule.py  # Molecular bonding (covalent, ionic)
    level_7_bulk.py      # Bulk matter (thermodynamics, phases)
    README.md            # Documentation
    SUMMARY.md           # This file
```

Run verification:
```bash
python -m epistemic_domain.level_0_planck
python -m epistemic_domain.level_1_voxel
python -m epistemic_domain.level_2_pair
python -m epistemic_domain.level_3_triad
python -m epistemic_domain.level_4_heptad
python -m epistemic_domain.level_5_atom
python -m epistemic_domain.level_6_molecule
python -m epistemic_domain.level_7_bulk
```
