# CODE-QUAL Expert Review: Computational Reproducibility Assessment

**Reviewer**: CODE-QUAL (Tenured PhD in Scientific Computing, Reproducible Research, and Code Quality)

**Document**: Foundational Ternary Dynamics (FTD) Manuscript

**Date**: 2026-01-25

**Review Focus**: Computational reproducibility, code quality, frozen results integrity, and numerical verification

---

## Executive Summary

The FTD manuscript contains executable Python code embedded in Quarto markdown chapters, with pre-computed results stored in JSON freeze files. This review evaluates the computational aspects of the manuscript for reproducibility, clarity, and scientific rigor.

**Overall Assessment**: The computational infrastructure demonstrates **above-average quality** for a theoretical physics manuscript, with clear code, proper dependency documentation, and verifiable numerical results. However, several areas require attention for full reproducibility standards.

---

## 1. CODE CLARITY

### Grade: B+

### Findings

#### Strengths

1. **Well-Structured Code Blocks**: The Python code in `1.10b-master-quadratic-derivation.qmd` is cleanly formatted with clear variable naming:
   ```python
   G_STAR = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)
   ```
   Variable names like `G_STAR`, `x_plus`, `x_minus` directly correspond to mathematical notation in the text.

2. **Inline Comments**: Code blocks include explanatory comments:
   ```python
   # Lemniscatic constant
   G_STAR = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)
   print(f"G* = {G_STAR:.10f}")
   ```

3. **Progressive Computation**: Each code block builds logically on previous calculations, making the derivation chain traceable.

4. **Formatted Output**: Results are printed with appropriate precision and labeled clearly:
   ```python
   print(f"  x_+ = {x_plus:.10f}  (cf. 1/alpha = 137.035999177)")
   ```

#### Weaknesses

1. **Limited Docstrings in Embedded Code**: The Quarto code blocks lack docstrings explaining the physical meaning of each computation step.

2. **No Type Hints**: Modern Python best practices suggest type hints for clarity:
   ```python
   def compute_g_star() -> float:  # Missing in current code
   ```

3. **Magic Numbers**: Some code contains unexplained constants:
   ```python
   M_delta_geo = (17/alpha + 81) * m_e  # Why 17? Why 81?
   ```
   While the text explains these, the code itself should be self-documenting.

### Recommendations
- Add brief docstrings to each code cell explaining the physical derivation step
- Consider extracting common constants to a shared module
- Add inline comments for non-obvious numerical choices

---

## 2. REPRODUCIBILITY

### Grade: B

### Findings

#### Strengths

1. **Quarto Freeze System**: The manuscript uses `execute: freeze: true` in `_quarto.yml`, ensuring results are captured:
   ```yaml
   execute:
     freeze: true
   ```
   This is appropriate for a manuscript where results should not change between builds.

2. **Comprehensive Requirements**: `requirements.txt` includes pinned version ranges:
   ```
   numpy>=1.24.0,<2.0.0
   scipy>=1.11.0,<2.0.0
   matplotlib>=3.7.0,<4.0.0
   sympy>=1.12,<2.0
   mpmath>=1.3.0,<2.0.0
   ```

3. **Test Suite**: The `tests/` directory contains comprehensive verification:
   - `test_master_quadratic.py`: 4 test classes, 9 test methods
   - `test_framework_integers.py`: Framework constant verification
   - `test_particle_masses.py`: Mass derivation checks
   - `test_coupling_constants.py`: Coupling constant verification
   - `run_all_tests.py`: Master test runner

4. **Verification Report Generation**: `run_all_tests.py` generates detailed reports:
   ```python
   def save_report(result, output, filename="tests/verification_report.txt"):
   ```

#### Weaknesses

1. **No Lockfile**: While version ranges are specified, exact versions (via `pip freeze` or `poetry.lock`) are not captured. This could lead to slight numerical differences across environments.

2. **Missing Python Version Specification**: `requirements.txt` does not specify the Python version (3.9? 3.10? 3.11?).

3. **Quarto Version Not Documented**: The manuscript requires Quarto but no version is specified. Quarto's rendering behavior can vary between versions.

4. **No Container/Environment File**: A `Dockerfile` or `environment.yml` (conda) would ensure identical computational environments.

5. **Random Seed**: While the code appears deterministic, no explicit random seed is set for any stochastic operations.

### Recommendations
- Add `pyproject.toml` or `requirements-frozen.txt` with exact versions
- Specify Python version in a `.python-version` file
- Document Quarto version requirement
- Consider adding a `Dockerfile` for exact reproducibility

---

## 3. DEPENDENCIES

### Grade: B+

### Findings

#### Strengths

1. **Clear Dependency Structure**:
   - `requirements.txt`: Core scientific computing dependencies
   - `requirements-dev.txt`: Development/testing dependencies (references core)

2. **Appropriate Packages**: Dependencies are standard scientific Python:
   - `numpy`, `scipy`: Core numerical computing
   - `sympy`, `mpmath`: Symbolic mathematics (appropriate for a theoretical physics manuscript)
   - `matplotlib`: Visualization

3. **Version Bounds**: Upper bounds prevent breaking changes:
   ```
   numpy>=1.24.0,<2.0.0
   ```

4. **Optional Dependencies Noted**:
   ```
   # Animation (optional)
   manim>=0.18.0,<1.0.0
   ```

#### Weaknesses

1. **scipy.special Dependency**: Code uses `from scipy.special import gamma, ellipk` without explicit documentation that these are critical for the central derivation.

2. **No Dependency Justification**: No documentation explains why each package is needed.

3. **Quarto Not in Requirements**: Quarto (the document rendering tool) is external but critical:
   ```
   # Documentation
   # quarto (install separately via official installer)
   ```

### Recommendations
- Add a DEPENDENCIES.md explaining the role of each package
- Consider creating a `setup.py` or `pyproject.toml` for proper packaging
- Document the Quarto installation requirement more prominently

---

## 4. OUTPUT VERIFICATION

### Grade: A-

### Findings

#### Strengths

1. **Frozen Results Match Code**: Examining the freeze files, the JSON outputs match expected computations:
   ```json
   {
     "hash": "f188e50434768fb459777cc08b6362de",
     "result": {
       "engine": "jupyter",
       ...
     }
   }
   ```
   The hash ensures integrity detection.

2. **Vieta Verification**: The code explicitly verifies algebraic consistency:
   ```python
   # Vieta verification
   print(f"  x_+ + x_- = {x_plus + x_minus:.10f}")
   print(f"  16(G*)^2 = {16 * G_STAR**2:.10f}")
   ```
   This is excellent practice for mathematical derivations.

3. **Residual Checking**: The code checks that computed roots satisfy the quadratic:
   ```python
   residual_plus = x_plus**2 + b*x_plus + c
   print(f"  f(x_+) = {residual_plus:.2e}")  # Should be ~0
   ```

4. **Comprehensive Test Coverage**: The test suite verifies:
   - Lemniscatic constant (G*) computation
   - Quadratic coefficients
   - Root values
   - Vieta relations
   - Fine structure constant accuracy
   - Calculator-reproducible derivation

5. **Experimental Comparison**: Results are compared to CODATA values:
   ```python
   ALPHA_INV_EXP = 137.035999177  # CODATA 2022
   error_ppm = abs(alpha_pred - alpha_exp) / alpha_exp * 1e6
   print(f"  Error: {ppm_error:.2f} ppm")
   ```

#### Weaknesses

1. **Some Freeze Files Show Stale Content**: The `tex.json` and `epub.json` files exist alongside `html.json`, but their content may not be synchronized if only HTML output is tested.

2. **No Automated CI Verification**: No GitHub Actions or similar CI to verify that frozen results can be regenerated.

3. **Output Format Differences**: The frozen markdown in JSON differs slightly from the source `.qmd` files (expected but worth noting for auditing).

### Recommendations
- Add CI workflow to verify freeze integrity
- Consider checksums for numerical outputs beyond Quarto's hash
- Document the verification process for reviewers

---

## 5. ERROR HANDLING

### Grade: C+

### Findings

#### Strengths

1. **Assertions in Tests**: Test code uses `unittest` assertions appropriately:
   ```python
   self.assertGreater(discriminant, 0, "Discriminant must be positive")
   self.assertAlmostEqual(x_plus, 137.036, places=2)
   ```

2. **Edge Case Documentation**: The manuscript notes edge cases in the theory section:
   > **Edge Case (KB = 0)**: If KB = 0, the expression becomes undefined...

#### Weaknesses

1. **No Input Validation in Embedded Code**: The Quarto code blocks do not validate inputs:
   ```python
   # No check that gamma(0.25) returns expected value
   G_STAR = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)
   ```

2. **Silent Failures Possible**: If a computation fails, there's no graceful error message in the manuscript code.

3. **No Numerical Stability Checks**: The discriminant calculation `b**2 - 4*a*c` can suffer from floating-point issues for large coefficients (not critical here, but good practice to check).

4. **Missing Division-by-Zero Guards**: Some formulas like `1/alpha` could fail if alpha were computed as zero.

### Recommendations
- Add sanity checks for intermediate values
- Include assertions in manuscript code (not just tests)
- Document expected value ranges

---

## 6. NUMERICAL PRECISION

### Grade: A-

### Findings

#### Strengths

1. **Appropriate Precision Display**: Results show precision appropriate to their significance:
   ```python
   print(f"G* = {G_STAR:.10f}")  # 10 decimal places for constant
   print(f"Error: {ppm_error:.2f} ppm")  # 2 decimals for error
   ```

2. **Double Precision Used**: All calculations use IEEE 754 double precision (numpy default), appropriate for these computations.

3. **ppm Error Quantification**: Errors are reported in scientifically meaningful units:
   ```
   Accuracy:
     1/alpha predicted: 137.0361714582
     1/alpha experimental: 137.035999177
     Error: 1.26 ppm
   ```

4. **Gamma Function Precision**: The `scipy.special.gamma` function provides sufficient precision for Gamma(1/4).

5. **Verification Against Tables**: The test suite includes verification against known mathematical constants:
   ```python
   expected = 2.9586751192  # From mathematical tables
   self.assertAlmostEqual(g_star, expected, places=8)
   ```

#### Weaknesses

1. **No Uncertainty Propagation**: While experimental uncertainties are noted (CODATA 2022), no formal uncertainty propagation is performed.

2. **Single Precision Check**: The code does not verify it's running in double precision.

3. **No Extended Precision for Critical Constants**: For a manuscript claiming ppm-level precision, extended precision (mpmath) could be used for verification:
   ```python
   from mpmath import mp
   mp.dps = 50  # 50 decimal places
   ```
   mpmath is in requirements but not used in the manuscript code.

4. **Intermediate Rounding**: Some calculations may accumulate rounding errors:
   ```python
   # Multiple chained operations
   x_plus = (-b + np.sqrt(D)) / (2*a)
   ```

### Recommendations
- Add extended precision verification using mpmath
- Include formal uncertainty analysis
- Document numerical precision limitations explicitly

---

## 7. FREEZE INTEGRITY

### Grade: B+

### Findings

#### Strengths

1. **Quarto Freeze System**: The `_freeze` directory properly captures execution results:
   ```
   manuscript/_freeze/chapters/1.10b-master-quadratic-derivation/execute-results/
   ├── html.json
   ├── tex.json
   └── epub.json
   ```

2. **Content Hash**: Each freeze file contains a hash for integrity:
   ```json
   "hash": "f188e50434768fb459777cc08b6362de"
   ```

3. **Full Markdown Capture**: The frozen results include the complete rendered markdown, allowing verification that outputs match source.

4. **Multi-Format Support**: Results frozen for HTML, TeX, and EPUB formats.

5. **Supporting Files Reference**: The freeze structure properly references supporting files:
   ```json
   "supporting": ["1.10b-master-quadratic-derivation_files"]
   ```

#### Weaknesses

1. **No Freeze Verification Script**: There's no automated way to verify that frozen results match re-executed code.

2. **Freeze Files in Git History**: Examining git status shows freeze files are modified, suggesting they may drift from source:
   ```
   M manuscript/_freeze/chapters/1.10b-master-quadratic-derivation/execute-results/html.json
   ```

3. **No Timestamp**: Freeze files don't include execution timestamp.

4. **Jupyter Engine Only**: All execution uses Jupyter kernel; no verification with alternative Python execution.

### Recommendations
- Add a verification script that re-executes and compares to frozen results
- Include execution timestamps in freeze metadata
- Commit freeze files only when explicitly verified

---

## Detailed File Analysis

### Chapters with Executable Code

| Chapter | Code Cells | Verification | Issues |
|---------|-----------|--------------|--------|
| 1.10b-master-quadratic-derivation.qmd | 2 cells | Full numerical verification, Vieta check | None significant |
| 1.12-gravity-from-integers.qmd | 0 cells (theory only) | N/A | Equations not computationally verified |
| 1.13-grand-unification.qmd | 0 cells | N/A | Complex derivation chain not coded |
| 1.14-proton-decay.qmd | 0 cells | N/A | Lifetime formula not verified |
| 2.6-flavor-physics.qmd | 0 cells | N/A | Multiple formulas unverified |

### Test Suite Analysis

| Test File | Coverage | Quality |
|-----------|----------|---------|
| test_master_quadratic.py | G* derivation, quadratic roots, Vieta, alpha accuracy | Excellent |
| test_framework_integers.py | Integer consistency | Good |
| test_particle_masses.py | Mass derivations | Good |
| test_coupling_constants.py | Coupling values | Good |
| test_mixing_matrices.py | CKM/PMNS matrices | Good |
| test_cosmology.py | Cosmological predictions | Adequate |
| verify_pedagogy.py | Student reproducibility | Excellent pedagogical focus |

---

## Summary Grades

| Criterion | Grade | Weight | Weighted Score |
|-----------|-------|--------|----------------|
| Code Clarity | B+ | 15% | 3.3 |
| Reproducibility | B | 20% | 3.0 |
| Dependencies | B+ | 10% | 3.3 |
| Output Verification | A- | 20% | 3.7 |
| Error Handling | C+ | 10% | 2.3 |
| Numerical Precision | A- | 15% | 3.7 |
| Freeze Integrity | B+ | 10% | 3.3 |

**Overall Weighted Grade: B+ (3.26/4.0)**

---

## Final Assessment

### Strengths of Computational Infrastructure

1. **Strong Verification Philosophy**: The manuscript demonstrates commitment to verifiability with explicit numerical checks, Vieta validation, and comparison to experimental values.

2. **Pedagogical Focus**: Code is designed for readability and student reproducibility, including "napkin derivation" tests.

3. **Comprehensive Test Suite**: The `tests/` directory provides extensive automated verification independent of the manuscript.

4. **Appropriate Technology Stack**: Quarto + Python + scipy is an excellent choice for reproducible scientific manuscripts.

### Critical Gaps

1. **Incomplete Code Coverage**: Only 1 of 5 chapters with claimed derivations has executable verification code. The gravity, unification, proton decay, and flavor physics chapters lack computational verification.

2. **Environment Reproducibility**: Without exact dependency versions and containerization, results may vary slightly across environments.

3. **No CI/CD**: Automated verification of results is missing.

### Recommendations for Publication

1. **Mandatory**: Add computational verification cells to chapters 1.12, 1.13, 1.14, and 2.6.

2. **Strongly Recommended**: Create frozen dependency lockfile and document Python version.

3. **Recommended**: Add GitHub Actions workflow to verify freeze integrity on each commit.

4. **Optional**: Add mpmath extended precision verification for claimed ppm-level results.

### Conclusion

The FTD manuscript demonstrates **above-average computational practices** for a theoretical physics work. The central derivation (master quadratic) is well-verified with multiple independent checks. However, the broader claims (gravitational hierarchy, grand unification, proton decay, flavor physics) lack the same level of computational verification. For a manuscript claiming "derivation" of physical constants, extending the computational verification to all derived quantities would significantly strengthen the work.

The existing test infrastructure is solid and could readily be extended. The use of Quarto's freeze system is appropriate but would benefit from automated verification. Overall, the computational aspects support reproducibility but require additional work to fully verify all claimed derivations.

---

*Review conducted according to standards for computational reproducibility in scientific publishing.*
