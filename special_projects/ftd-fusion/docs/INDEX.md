# FTD Fusion Documentation Index

## Overview

This directory contains complete documentation for the FTD Fusion package, which derives nuclear fusion energy from first principles using the Foundational Ternary Dynamics framework.

---

## Documents

| Document | Purpose |
|----------|---------|
| [DERIVATION_CHAIN.md](DERIVATION_CHAIN.md) | Complete mathematical derivation from integers to fusion |
| [VERIFICATION_REPORT.md](VERIFICATION_REPORT.md) | Test results and accuracy metrics |
| [SCIENTIFIC_SIGNIFICANCE.md](SCIENTIFIC_SIGNIFICANCE.md) | Why this matters for physics |
| [USAGE_GUIDE.md](USAGE_GUIDE.md) | How to use the package with examples |

---

## Quick Links

### For Scientists
- [DERIVATION_CHAIN.md](DERIVATION_CHAIN.md) - See the mathematics
- [VERIFICATION_REPORT.md](VERIFICATION_REPORT.md) - See the accuracy

### For Developers
- [USAGE_GUIDE.md](USAGE_GUIDE.md) - API reference and examples

### For Understanding Significance
- [SCIENTIFIC_SIGNIFICANCE.md](SCIENTIFIC_SIGNIFICANCE.md) - Why this matters

---

## Key Results Summary

| Quantity | FTD Value | Experimental | Error |
|----------|-----------|--------------|-------|
| D-T Q-value | 17.59 MeV | 17.59 MeV | 0.0% |
| Fe-56 binding | 492.22 MeV | 492.25 MeV | 0.01% |
| Iron peak | A = 52 | A = 56 | 7% |
| Overall RMS | - | - | 3.10% |

---

## Source Code

| File | Description |
|------|-------------|
| `../derivations/binding_energy.py` | SEMF from FTD integers |
| `../derivations/mass_defect.py` | Q-value calculations |
| `../derivations/fusion_fission.py` | Iron boundary analysis |

---

## Framework Integers

```
N_c = 3       (number of colors)
N_base = 4    (base dimensions)
b_3 = 7       (QCD beta coefficient)
N_eff = 13    (effective degrees of freedom)
```

These same integers derive:
- Fine structure constant: 1/137.036 (1.26 ppm)
- Electron mass: 0.511 MeV (0.27%)
- All nuclear binding energies
- Fusion Q-values

---

## Contact

- **Repository:** https://github.com/ftd/ftd-fusion
- **Issues:** https://github.com/ftd/ftd-fusion/issues
- **License:** MIT
