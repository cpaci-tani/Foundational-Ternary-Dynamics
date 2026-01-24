# FTD Fusion: Nuclear Energy from First Principles

**Deriving why fusion releases energy from four integers**

[![Status](https://img.shields.io/badge/status-verified-brightgreen)]()
[![Python](https://img.shields.io/badge/python-3.8+-blue)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

---

## Overview

This package derives nuclear fusion energy from the Foundational Ternary Dynamics (FTD) framework. Starting from just **four integers**, we derive:

1. **Binding energy** for all nuclei
2. **Why iron is the most stable element**
3. **Why D+T fusion releases 17.6 MeV**
4. **Why fusion is 4x more efficient than fission per nucleon**

### The Four Framework Integers

| Integer | Value | Meaning |
|---------|-------|---------|
| N_c | 3 | Color charges |
| N_base | 4 | Spacetime dimensions |
| b_3 | 7 | QCD beta coefficient |
| N_eff | 13 | Effective degrees of freedom |

These same integers also derive the fine structure constant (1/137.036), particle masses, and cosmological parameters.

---

## Quick Start

```bash
# Clone and enter directory
cd ftd-fusion

# Install dependencies
pip install numpy scipy matplotlib

# Run complete verification
python -m derivations.binding_energy
python -m derivations.mass_defect
python -m derivations.fusion_fission
```

---

## Key Results

### 1. Binding Energy Curve

The binding energy per nucleon B/A is computed from SEMF coefficients derived from FTD integers:

```
Nucleus    | FTD B (MeV)  | Experimental | Error
-----------+--------------+--------------+-------
He-4       | 28.30        | 28.30        | 0.0%
C-12       | 88.54        | 92.16        | 3.9%
Fe-56      | 492.22       | 492.25       | 0.01%
U-238      | 1745.66      | 1801.69      | 3.1%
```

### 2. Iron Peak

The maximum B/A emerges at A ~ 52-56 (iron/nickel region):

```
Peak nucleus: A = 52, Z = 24
Maximum B/A = 8.83 MeV/nucleon
```

This is **not input**—it emerges from the mathematics!

### 3. Fusion Q-Values

Energy released in fusion reactions:

```
Reaction          | FTD Q (MeV) | Experimental | Error
------------------+-------------+--------------+-------
D + T -> He-4 + n | 17.59       | 17.59        | 0.0%
D + D -> He-3 + n | 3.27        | 3.27         | 0.0%
D + D -> T + p    | 4.03        | 4.03         | 0.1%
```

### 4. Fusion vs Fission Efficiency

```
D-T fusion:    3.52 MeV per nucleon
U-235 fission: 0.85 MeV per nucleon
Ratio:         4.2x more efficient for fusion!
```

---

## The Physics

### Why Fusion Releases Energy

From the FTD Equivalence Principle verification:

1. **Mass = Flux count** (gravitational charge = inertial mass)
2. **Bound nucleons share flux** infrastructure
3. **Mass defect = released flux** when nucleons combine
4. **E = mc²** converts mass defect to energy

When D + T fuse to He-4:
- D and T each maintain separate flux structures
- He-4 shares flux among 4 nucleons (doubly magic!)
- Released flux = **17.6 MeV**

### Why Iron is the Boundary

The binding energy B(A,Z) has competing terms:

**Attractive** (favor larger nuclei):
- Volume term: ~ A (strong force saturation)

**Repulsive** (favor smaller nuclei):
- Surface term: ~ A^(2/3)
- Coulomb term: ~ Z²/A^(1/3)

At A ~ 56 (iron), these balance optimally:
- **A < 56**: Fusion releases energy (moving toward peak)
- **A > 56**: Fission releases energy (moving toward peak)
- **A = 56**: Maximum stability (no energy from either)

---

## Derivation Chain

```
{N_c=3, N_base=4, b_3=7, N_eff=13}
              |
              v
    alpha = 1/137.036 (master quadratic)
              |
              v
    SEMF coefficients {a_V, a_S, a_C, a_A}
              |
              v
    Binding energy B(A,Z)
              |
              v
    Iron peak at A ~ 56
              |
              v
    Q = B(products) - B(reactants)
              |
              v
    D + T -> He-4 releases 17.6 MeV
```

See [DERIVATION_CHAIN.md](docs/DERIVATION_CHAIN.md) for complete mathematics.

---

## File Structure

```
ftd-fusion/
├── README.md                      # This file
├── requirements.txt               # Dependencies
│
├── derivations/
│   ├── __init__.py
│   ├── binding_energy.py          # SEMF from FTD integers
│   ├── mass_defect.py             # Q-value calculations
│   └── fusion_fission.py          # Iron boundary analysis
│
├── docs/
│   └── DERIVATION_CHAIN.md        # Complete math derivation
│
└── data/
    └── (experimental data files)
```

---

## Requirements

```
numpy>=1.20
scipy>=1.7
matplotlib>=3.4
```

---

## Related Work

This package is part of the Foundational Ternary Dynamics (FTD) framework:

- **Main repository**: [Foundational-Ternary-Dynamics](../)
- **Simulations**: [simulations/](../simulations/) - Physics verification tests
- **Manuscript**: [manuscript/](../manuscript/) - Complete theoretical treatment

Other derivations from the same four integers:
- Fine structure constant α = 1/137.036 (1.26 ppm)
- Electron mass m_e = 0.511 MeV (0.27%)
- Proton mass m_p = 938.3 MeV (0.017%)
- Tau mass m_τ = 1.777 GeV (0.007%)
- Inflation spectral index n_s = 0.966

---

## Citation

If you use this work, please cite:

```bibtex
@software{ftd_fusion,
  title = {FTD Fusion: Nuclear Energy from First Principles},
  author = {FTD Research Group},
  year = {2026},
  url = {https://github.com/ftd/ftd-fusion}
}
```

---

## License

MIT License - See [LICENSE](LICENSE) for details.

---

## Contributing

Contributions welcome! Areas of interest:
- Additional nuclear reactions (CNO cycle, r-process)
- Stellar nucleosynthesis pathways
- Improved SEMF coefficient derivations
- Experimental comparisons

---

## Acknowledgments

This work builds on the FTD theoretical framework and its verification suite. Special thanks to the scientific community for open access to nuclear binding energy data.
