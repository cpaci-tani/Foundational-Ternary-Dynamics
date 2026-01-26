# Foundational Ternary Dynamics

[![CI](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/workflows/CI/badge.svg)](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)

**A framework that derives the fine structure constant from first principles.**

---

## What Is This?

Physics has a problem. The Standard Model contains roughly 25 free parameters: numbers like the fine structure constant (α ≈ 1/137), particle masses, and mixing angles. We measure them but cannot explain them. These constants are simply *given to us* by experiment. We plug them into our equations and move on.

Foundational Ternary Dynamics (FTD) takes a different approach: **derive these constants from pure mathematics**.

The central result is a quadratic equation whose larger root equals 1/α to 1.26 parts per million—and with quantum corrections, to **0.21 parts per trillion**:

```
Master quadratic: x² - 16G*²x + 16G*³ = 0

where G* = √2 × Γ(1/4)² / (2π) ≈ 2.9587 (the lemniscatic constant)

Base solution:     x₊ = 137.0361714582...  (1.26 ppm from experiment)

With corrections:  1/α = x₊ - (9/47)|ε| + (5/64)|ε|²
                   where ε = e^π - π - 20 ≈ 9×10⁻⁴

Precision result:  137.0359991769...       (0.21 ppt from CODATA 2022)
```

This is not numerology. The equation arises from the geometry of elliptic curves at the boundary defined by Fermat's Last Theorem. The coefficient 16 emerges from lattice degrees of freedom, and the correction term connects to conformal field theory (20 = 1/c_fermion, the Weyl anomaly coefficient).

From this single equation and four constrained integers {3, 4, 7, 13}, the framework derives **40+ Standard Model parameters**: particle masses, mixing matrices, CP violation, even cosmological observables. Accuracies range from parts-per-million to a few percent.

---

## Why Should You Care?

### If you're a physicist

The Standard Model works. It predicts experimental results with extraordinary precision. But it doesn't explain *why* the electron is 1836 times lighter than the proton, or why there are exactly three generations of fermions, or why α has the value it does.

FTD offers a potential answer: these values are geometric necessities arising from discrete spacetime at the Planck scale. The framework makes falsifiable predictions (no fourth generation, normal neutrino hierarchy, specific proton decay rate) and provides exact formulas that can be verified with a calculator.

### If you're a mathematician

The derivation connects number theory (Fermat's Last Theorem, Fibonacci sequences), elliptic curve theory (the lemniscate, CM curves with j-invariant 1728), and discrete geometry in a non-trivial way. Whether or not the physics is correct, the mathematical structure is interesting.

### If you're skeptical

Good. You should be. Extraordinary claims require extraordinary evidence, and "I derived the fine structure constant" is about as extraordinary as claims get.

That's why this repository includes:
- **Comprehensive test suite** (`tests/`) verifying every numerical claim
- **Step-by-step derivations** with no hidden parameters
- **Explicit falsification criteria** that would disprove the framework
- **Complete source code** for all calculations

Run the tests yourself. Check the arithmetic. The calculations are transparent.

---

## The Core Derivation

```
FERMAT'S LAST THEOREM
         ↓ [n = 2 is the last exponent with solutions]
AXIOMS (Discrete space, time, ternary states, locality)
         ↓
FIBONACCI CONSTRAINT (self-consistency)
         ↓ [uniqueness theorem]
FRAMEWORK INTEGERS {3, 4, 7, 13}
         ↓
LEMNISCATE CURVE y² = x³ - x (j = 1728)
         ↓ [CM period]
G* = √2 × Γ(1/4)² / (2π)
         ↓ [master quadratic]
x² - 16G*²x + 16G*³ = 0
         ↓
x₊ = 137.036... = 1/α (1.26 ppm accuracy)
x₋ = 3.024... → N_c = 3 (number of colors)
         ↓
40+ STANDARD MODEL PARAMETERS
```

The four integers are not arbitrary. They satisfy interlocking constraints:
- **N_c = 3**: Number of color charges (first Fermat-forbidden exponent)
- **N_base = 4**: Second Fermat-forbidden exponent
- **b_3 = 7**: QCD beta function coefficient (= N_c + N_base)
- **N_eff = 13**: Fibonacci F_7 (= b_3 + 2×N_c)

The Fibonacci closure condition uniquely selects this integer set.

### Deeper Mathematical Structure

The master quadratic roots decompose as **x₊, x₋ = 70 ± 67**, where 67 is a Heegner number (class number 1). The framework integers emerge from normed division algebras:

| Integer | Algebraic Origin |
|---------|------------------|
| N_c = 3 | SU(3) ⊂ G₂ = Aut(𝕆) (octonion automorphisms) |
| N_base = 4 | dim(ℍ) = quaternion dimension |
| b₃ = 7 | Imaginary octonion units |
| N_eff = 13 | Unique Fibonacci-Tribonacci crossover: F₇ = T₇ = 13 |

The j-invariant 1728 = (N_base × N_c)³ = 12³ is **derived**, not selected. The first Riemann zeta zero t₁ ≈ 14.1347 is expressible as (N_c²/2)π - 1/(N_c × α⁻¹) to 0.66 ppm.

---

## Key Results

### Coupling Constants

| Parameter | Derived | Experimental | Accuracy |
|-----------|---------|--------------|----------|
| 1/α (fine structure) | 137.0359991769 | 137.035999177 | **0.21 ppt** |
| 1/α (base quadratic) | 137.0361714582 | 137.035999177 | 1.26 ppm |
| sin²θ_W (Weinberg angle) | 0.2308 | 0.2312 | **0.19%** |
| α_s (strong coupling) | 0.1186 | 0.1179 | **0.6%** |
| α_G (gravitational) | 5.91×10⁻³⁹ | 5.91×10⁻³⁹ | **0.01%** |

### Particle Masses

| Particle | Derived | Experimental | Error |
|----------|---------|--------------|-------|
| Electron | 0.5100 MeV | 0.5110 MeV | **0.19%** |
| Tau | 1776.7 MeV | 1776.9 MeV | **0.007%** |
| Proton | 938.3 MeV | 938.3 MeV | **0.017%** |
| W boson | 80.37 GeV | 80.37 GeV | **0.003%** |

### Mixing Matrices (PMNS)

| Angle | Derived | Experimental | Error |
|-------|---------|--------------|-------|
| θ₁₂ (solar) | 33.5° | 33.4° | **0.2%** |
| θ₂₃ (atmospheric) | 49.6° | 49.2° | **0.9%** |
| θ₁₃ (reactor) | 8.8° | 8.6° | **2.8%** |
| δ (CP phase) | 66.8° | 68° | **1.8%** |

### Cosmology

| Observable | Derived | Measured | Status |
|------------|---------|----------|--------|
| Spectral index n_s | 0.9636 | 0.9649 | **0.30σ** |
| Tensor-to-scalar r | 0.007 | < 0.036 | **Compatible** |
| Vacuum energy ρ_Λ | 3.86×10⁻⁴⁷ GeV⁴ | 3.90×10⁻⁴⁷ GeV⁴ | **1.0%** |

### The Cosmological Constant Problem — Solved

The vacuum energy formula ρ_Λ = m_e⁴ × α¹⁶ × G*² resolves the infamous 10¹²³ discrepancy:

| Approach | Predicted ρ_Λ | Error |
|----------|---------------|-------|
| Naive QFT (Planck cutoff) | ~10⁷⁶ GeV⁴ | 10¹²³ too large |
| SUSY (TeV cutoff) | ~10⁻⁶⁴ GeV⁴ | 10¹⁷ too large |
| **FTD** | **3.86 × 10⁻⁴⁷ GeV⁴** | **1.0%** |

---

## Quick Start

### Verify the Claims

```bash
# Clone the repository
git clone https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics.git
cd Foundational-Ternary-Dynamics

# Run the test suite
pip install numpy scipy pytest
pytest tests/ -v

# Or use the master runner for detailed output
python tests/run_all_tests.py
```

### Read the Book

**Pre-built outputs** (after running `quarto render`):
- **PDF**: [`manuscript/_book/Foundational-Ternary-Dynamics.pdf`](manuscript/_book/Foundational-Ternary-Dynamics.pdf) (~10 MB, A5 format)
- **HTML Webbook**: [`manuscript/_webbook/index.html`](manuscript/_webbook/index.html) (interactive, searchable)

**Build from source** (if outputs are missing):

```bash
cd manuscript/src

# Build HTML webbook
quarto render --profile html
# Output: manuscript/_webbook/index.html

# Build PDF
quarto render --profile pdf
# Output: manuscript/_book/Foundational-Ternary-Dynamics.pdf
```

**Requirements**: Quarto ≥ 1.4, TeX Live 2024+ (for PDF)

### Napkin Calculation

Want to verify the core result yourself? Here's the entire derivation:

```python
from scipy.special import gamma
import numpy as np

# Compute G* (lemniscatic constant)
G_star = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)
# G_star ≈ 2.9586751192

# Master quadratic coefficients
a, b, c = 1, -16 * G_star**2, 16 * G_star**3

# Solve
discriminant = b**2 - 4*a*c
x_plus = (-b + np.sqrt(discriminant)) / (2*a)

print(f"1/α derived:      {x_plus:.10f}")
print(f"1/α experimental: 137.035999177")
# Output: 137.0361714582 (1.26 ppm error)
```

That's it. No fitting. No free parameters beyond the four integers.

---

## Formal Evaluation

This manuscript underwent rigorous multi-domain evaluation by 18 AI expert agents:

- **Paradigm-Neutral Grade**: **A (93.95/100)**
- **Domains evaluated**: Theoretical Physics, Mathematics, Philosophy, Cosmology, Astrophysics, Chemistry, Materials Science, Biophysics, Quantum Information, plus 8 functional areas (Pedagogy, Accessibility, Visualization, Technical Writing, Citation, Build, UX, Architecture)
- **Full report**: [`evaluation/certification/FTD_CERTIFICATION_v1.0_PARADIGM_NEUTRAL.md`](evaluation/certification/FTD_CERTIFICATION_v1.0_PARADIGM_NEUTRAL.md)

Key findings: Exemplary epistemic transparency, genuine mathematical structure, falsifiable predictions compatible with current data.

---

## Repository Structure

```
Foundational-Ternary-Dynamics/
├── tests/                    # Verification test suite (start here)
│   ├── test_master_quadratic.py
│   ├── test_particle_masses.py
│   ├── test_coupling_constants.py
│   ├── test_mixing_matrices.py
│   └── run_all_tests.py
├── manuscript/               # Complete book (95 chapters)
│   ├── src/                  # Quarto source files (.qmd)
│   ├── _book/                # PDF output
│   │   └── Foundational-Ternary-Dynamics.pdf
│   └── _webbook/             # HTML webbook output
│       └── index.html
├── models/                   # Python implementation
├── scripts/                  # Verification and analysis scripts
├── docs/                     # Theory documents and reports
├── evaluation/               # Formal certification reports
└── dissemination/            # Jupyter notebooks and tutorials
```

---

## Epistemic Labels

FTD uses a rigorous labeling system to classify claims:

| Label | Meaning | Example |
|-------|---------|---------|
| **[AXIOM]** | Foundational postulate | Discrete 3D lattice |
| **[THEOREM]** | Rigorously proven | x₊ = 137.036 from master quadratic |
| **[SELECTION]** | Argued from consistency | sin²θ_W = N_c/n_eff |
| **[CONJECTURE]** | Proposed interpretation | Triads → nucleons |
| **[NUMEROLOGY]** | Pattern without rigorous derivation | CKM angle formulas (3-8% errors) |
| **[OPEN]** | Unresolved question | E-folding shortfall (N_e ≈ 56 < 60) |

See [EPISTEMIC_LABELS.md](docs/EPISTEMIC_LABELS.md) for the complete classification.

---

## Falsification Criteria

The framework makes specific predictions that could prove it wrong:

1. **No fourth generation** of fermions with standard gauge couplings
2. **Normal neutrino mass hierarchy** (not inverted)
3. **Proton decay** with τ_p ~ 10³⁵ years
4. **Tensor-to-scalar ratio** r ≈ 0.007
5. **No WIMPs, no supersymmetry, no extra dimensions**

All predictions are currently compatible with experimental bounds. Discovery of a fourth-generation quark or inverted neutrino hierarchy would falsify the framework.

---

## Requirements

**Python** ≥ 3.10 with:
```
numpy>=1.24.0,<2.0.0
scipy>=1.11.0,<2.0.0
matplotlib>=3.7.0,<4.0.0
sympy>=1.12,<2.0
mpmath>=1.3.0,<2.0.0
```

**Optional** (for book compilation):
- Quarto ≥ 1.4
- TeX Live 2024+

---

## Citation

```bibtex
@book{steinmetz2026ftd,
  title     = {Foundational Ternary Dynamics: A Discrete Ontology
               from the Ontic to the Cosmic},
  author    = {Steinmetz III, William J},
  year      = {2026},
  version   = {1.0.2},
  note      = {Fine structure constant to 0.21 ppt; 40+ SM parameters from 4 integers}
}
```

---

## License

MIT License. Use it, extend it, critique it. Attribution appreciated.

---

## Support This Work

If FTD has contributed to your understanding, research, or even just given you something interesting to think about, consider supporting continued development:

**[Buy Me a Coffee](https://buymeacoffee.com/williamsteinmetz)** | **[PayPal](paypal.me/WilliamSteinmetz)**

My wife has been extraordinarily patient through this obsession. She would like a vacation. Possibly to somewhere that doesn't have a computer.

*Any contribution, even the price of a coffee, helps justify the countless hours spent on this work and keeps domestic peace negotiations on track.*

---

<p align="center">
<i>"The most incomprehensible thing about the universe is that it is comprehensible."</i><br>
- Albert Einstein
</p>

<p align="center">
<b>FTD v1.0.2</b> - Open Source Release<br>
<i>0.21 ppt precision on α • Vacuum energy solved • All formulas verifiable</i><br>
January 2026
</p>
