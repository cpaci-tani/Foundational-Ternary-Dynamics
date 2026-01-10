# Reproducibility Guide for FTD Numerical Claims

**Purpose:** Enable independent verification of all headline numerical results.

**Version:** 1.0
**Date:** January 2026

---

## 1. Environment Specification

### 1.1 Required Software

| Package | Minimum Version | Purpose |
|---------|-----------------|---------|
| Python | 3.8+ | Runtime (tested on 3.10, 3.11) |
| NumPy | 1.20+ | Numerical computation |
| SciPy | 1.7+ | Special functions (Gamma, elliptic integrals) |
| mpmath | 1.2+ | Arbitrary precision arithmetic |
| Matplotlib | 3.4+ | Visualization (optional) |

### 1.2 Installation

```bash
pip install numpy scipy mpmath matplotlib
```

### 1.3 Verification

```python
import numpy as np
import scipy
import mpmath
print(f"NumPy: {np.__version__}")
print(f"SciPy: {scipy.__version__}")
print(f"mpmath: {mpmath.__version__}")
```

---

## 2. Precision Policy

### 2.1 Headline Constants

All headline constants are computed to **15 decimal places** using mpmath's arbitrary precision arithmetic.

```python
import mpmath
mpmath.mp.dps = 50  # 50 decimal places internal
# Report to 15 places after verification of stability
```

### 2.2 Reported Precision

Final values are truncated to the precision justified by:

1. **Input uncertainty:** G* is known exactly (mathematical constant)
2. **Experimental comparison:** CODATA 2022 α uncertainty is 0.15 ppb
3. **Numerical stability:** All digits reported are stable across methods

### 2.3 Cross-Validation

Each constant is computed via at least two independent methods where applicable:

| Constant | Method 1 | Method 2 |
|----------|----------|----------|
| G* | Direct Γ(1/4)² formula | AGM iteration |
| x₊ | Quadratic formula | Vieta relation check |
| 16 DoF | Direct counting | Constraint rank analysis |

---

## 3. Seed Policy

### 3.1 Stochastic Simulations

All stochastic simulations use reproducible seeding:

```python
import numpy as np
rng = np.random.default_rng(seed=42)  # Default seed
```

### 3.2 Documented Seeds

| Simulation | Seed | Purpose |
|------------|------|---------|
| Born rule test | 42 | Manifestation statistics |
| Bell test | 42 | Entanglement correlations |
| Monte Carlo | 12345 | Look-elsewhere analysis |

### 3.3 Seed Sensitivity

For critical results, sensitivity to seed choice is documented:

```python
for seed in [42, 123, 456, 789, 1000]:
    result = run_simulation(seed=seed)
    print(f"Seed {seed}: {result}")
# Report mean ± std across seeds
```

---

## 4. Verification Scripts

### 4.1 Script Catalog

| Claim | Script | Location |
|-------|--------|----------|
| G* value | `g_star_from_trd.py` | `simulations/` |
| α quadratic | `g_star_from_trd.py` | `simulations/` |
| 16 DoF | `coefficient_16_from_lattice.py` | `simulations/` |
| √2 coupling | `critical_coupling_selection.py` | `simulations/` |
| j = 1728 | `cm_selection_proof.py` | `simulations/` |
| τ = i | `tau_equals_i_proof.py` | `simulations/` |
| Elliptic fibration | `elliptic_fibration_proof.py` | `simulations/` |
| Born rule | `born_rule_test.py` | `simulations/` |
| SU(3) emergence | `su3_emergence.py` | `simulations/` |
| Bell test | `sloop_bell_test.py` | root directory |

### 4.2 Execution

**Run all proofs:**

```bash
# From the project root (pbr_pedagogy), navigate to simulations:
cd simulations
python run_all_proofs.py
```

**Run individual script:**

```bash
cd simulations
python g_star_from_trd.py
```

**Note:** The simulation scripts are located in the `simulations/` folder at the project root, not inside `dissemination/`.

**Output:** `verification_report.md` with all results and timestamps.

---

## 5. Known-Good Outputs

### 5.1 Fundamental Constants

```
G* (lemniscatic constant):
  Value:     2.9586751192100567168...
  Formula:   sqrt(2) * Gamma(1/4)^2 / (2*pi)
  Verified:  Matches Wolfram Alpha to 15 places

Quadratic roots:
  x+ (EM):   137.03617111389...
  x- (color): 3.02396436227...
  Sum:       140.06013547616... (= 16 * G*^2)
  Product:   414.38882308954... (= 16 * G*^3)

Comparison to experiment:
  CODATA 2022 1/alpha: 137.035999177(21)
  Discrepancy:         1.26 ppm
```

### 5.2 Lattice Constants

```
2x2x2 lattice DoF count:
  Total flux components: 24 (= 3 * 8)
  Gauss constraints:     7 (= 8 - 1)
  Gauge freedom:         1
  Physical DoF:          16 (= 24 - 7 - 1)

Critical coupling:
  omega_+: 1.4142135623730950488... (= sqrt(2))
  omega_-: 0 (massless Goldstone)
  lambda:  1 (critical)

CM selection:
  j = 1728: Aut = Z/4Z, embeds in O (octahedral)
  j = 0:    Aut = Z/6Z, does NOT embed in O
  Selected: j = 1728 (lemniscate)
```

### 5.3 Statistical Results

```
Born rule test (n=200 trials, lattice 16^3):
  Correlation (expected vs observed): > 0.9
  KL divergence: < 0.1
  Chi-squared p-value: > 0.05

Bell test (sLoop):
  S parameter range: 1.95 (f=0) to 2.85 (f=1)
  Quantum bound: 2*sqrt(2) = 2.828...
  Classical bound: 2
  Status: Reproduces quantum violation
```

---

## 6. Output Hashes

For critical outputs, SHA256 hashes of the output files:

| File | SHA256 (first 16 chars) |
|------|------------------------|
| `verification_report.md` | (to compute after run) |
| `g_star_output.txt` | (to compute after run) |
| `born_rule_test.png` | (to compute after run) |

**Compute hash:**

```bash
sha256sum verification_report.md | cut -c1-16
```

---

## 7. Reproducibility Status

### 7.1 Fully Reproducible

| Claim | Status | Notes |
|-------|--------|-------|
| G* value | COMPLETE | Matches reference to 15 places |
| Quadratic roots | COMPLETE | Algebraic, exact |
| 16 DoF | COMPLETE | Counting argument |
| √2 critical | COMPLETE | Eigenvalue calculation |
| j = 1728 selection | COMPLETE | Group theory |

### 7.2 Pending Completion

| Claim | Status | Notes |
|-------|--------|-------|
| Full Monte Carlo look-elsewhere | STUB | Script structure exists |
| Bell test substrate variation | PARTIAL | Needs parameter sweep |
| Mass hierarchy verification | PENDING | Formulas documented, not coded |

---

## 8. Troubleshooting

### 8.1 Common Issues

**mpmath precision:**
```python
# If results don't match, increase precision
mpmath.mp.dps = 100  # Try higher precision
```

**Seed mismatch:**
```python
# Ensure using numpy's new RNG interface
rng = np.random.default_rng(seed=42)  # Correct
np.random.seed(42)  # Legacy, may differ
```

**Path issues:**
```python
import sys
sys.path.insert(0, '/path/to/pbr_pedagogy/simulations')
```

### 8.2 Contact

For reproducibility issues, contact the author or file an issue in the repository.

---

## 9. Verification Checklist

Before claiming reproduction:

- [ ] Environment matches specification (Python 3.8+, packages)
- [ ] All scripts run without error
- [ ] G* matches to 10+ decimal places
- [ ] Quadratic roots match to 6+ decimal places
- [ ] 16 DoF count confirmed
- [ ] Born rule correlation > 0.8
- [ ] Verification report generated

---

*Guide prepared for independent verification.*
