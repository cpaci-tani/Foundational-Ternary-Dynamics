# Scientific Status of FTD Engine Tests

**Last updated: 2026-03-05**

## What These Tests DO Verify

### Internal Consistency (Grade: A)
- **Charge conservation**: Exact (Q = sum of states) across all campaigns, 50K+ ticks
- **Energy conservation**: <1% drift in steady-state (post-Phase-4 fixes)
- **Gauss constraint**: FFT solver gives exact violation = 0.0 on GPU
- **Maxwell equations**: All 4 verified on lattice (div(B)=0, Faraday, Ampere-Maxwell, E perp B)
- **CPU/GPU parity**: Bit-level agreement on SoA round-trip, <0.001% energy difference

### Emergent Force Laws (Grade: B+)
- **Coulomb exponent**: -2.067 on 128^3 GPU (3.4% from ideal -2.0, R^2=0.9999)
- **Isotropy**: 1.0 at r=5 (perfect on Poisson solver)
- **Gravitational attraction**: RMS shrinkage confirmed with 20 particles
- **Lorentz force**: v dot F = 0 (zero work, as required)
- **Binding**: Opposite charges attract and bind at predicted separation

### Framework Constants (Grade: B)
- **Fine structure constant**: x+ = 137.036 from master quadratic (1.26 ppm from CODATA)
- **Precision formula**: 4-term correction gives <1 ppt match to CODATA 2022
- **Weinberg angle**: sin^2(theta_W) = 3/13 = 0.2308 (0.19% from experiment)
- **Strong coupling**: alpha_s = 7/59 = 0.1186 (0.6% from experiment)
- **Generation count**: floor(x-) = 3 exactly

### Integer Uniqueness (Grade: A)
- **Exhaustive sweep**: Only {3, 4, 7, 13} passes all 5 experimental criteria simultaneously
- **Near-misses**: Very few 4/5 combinations exist, demonstrating tight constraint

## What These Tests DO NOT Verify

### Missing External Cross-Validation (Critical Gap)
- **Zero tests** compare FTD output against independently measured physical data
- No comparison with lattice QCD static quark potential
- No comparison with atomic spectral lines
- No scattering cross-section measurements
- No thermalization or equipartition tests

### Missing Quantum Mechanics Validation (Major Gap)
- **Born rule**: The test verifies that manifestation uses |J|^2, but |J|^2 was *imposed* as the threshold rule. This is circular.
- **Bell inequality**: The substrate correctly gives S <= 2 (local deterministic), but the aggregate S > 2 emergence mechanism is not demonstrated in simulation.
- **Entanglement**: Correlations are from shared origin (pre-established), not from quantum nonlocality.

### Framework Integers Are Inputs
- The integers {3, 4, 7, 13} are hardcoded in `ontic.h`
- The integer sweep test shows they are *uniquely constrained* but does not derive them from D=3 alone
- A reviewer may legitimately ask: "Where do these integers come from?"
- Answer: They are identified with Standard Model quantities (N_c = color charges, b_3 = QCD beta coefficient, etc.) — the identification is a *model choice*, not a derivation

### Disabled/Missing Physics
- 53 phenomenological tests were disabled in the "logic-first" refactor
- No QCD confinement dynamics (only force law)
- No weak interaction dynamics (only transmutation threshold)
- No spin-statistics from dynamics (imposed as exchange force)
- No Higgs mechanism
- No second quantization

### Only External Test Failed
- The CERN cavitation analysis (the only test against real experimental data) reported:
  - Energy scaling: predicted beta=0.5, observed beta=0.12 (FAILED)
  - Energy threshold: ruled out by factor of 10^16 (DECISIVELY FAILED)
  - This is honestly documented in `EMPIRICAL_CERN_CAVITATION.md`

## Test Categories and Their Scientific Meaning

| Category | Count | What It Proves | What It Doesn't Prove |
|----------|-------|----------------|----------------------|
| Unit tests (constants) | ~20 | Code implements formulas correctly | Formulas describe reality |
| Unit tests (dynamics) | ~35 | Engine phases work as designed | Design matches physics |
| Force law campaigns | ~8 | Forces follow intended profiles | Profiles match nature |
| Conservation campaigns | ~5 | Closed system conserves quantities | Quantities map to physical ones |
| Novel predictions | 7 | Constants match CODATA | Constants aren't just fitted |
| Falsifiability tests | 12 | Wrong params give wrong physics | Right params aren't coincidence |
| Integer sweep | 7 | {3,4,7,13} is uniquely constrained | Integers are derivable |
| Hydrogen spectrum | 8 | Bound states form with correct scaling | Energy levels are quantitative |
| Two-slit interference | 7 | Flux superposition creates fringes | Fringes match QM predictions |
| Born ensemble | 4 | Manifestation follows |J|^2 | |J|^2 is the correct measure |
| Bell substrate | 4 | S <= 2 from local determinism | Aggregate QM statistics emerge |

## What Would Convince a Scientific Body

### Minimum Requirements (Not Yet Met)
1. **External benchmarks**: Compare against published lattice QCD data, atomic spectra, or other independent measurements
2. **Statistical rigor**: Chi-squared tests, confidence intervals, systematic error budgets
3. **Blind predictions**: Make a prediction BEFORE the measurement, not after
4. **Independent replication**: Someone outside the project reproduces the results

### Current Assessment
- **Software engineering**: Excellent (A)
- **Internal consistency**: Very good (A-)
- **Physical validation**: Insufficient (C)
- **External cross-validation**: Not attempted (F)
- **Novel predictions**: Post-dictions only (D+)
- **Overall scientific credibility**: C+

### Path Forward
The test suite needs to evolve from "does the code work?" to "does the physics work?" The most impactful additions would be:
1. Comparison with lattice QCD static quark potential data
2. Quantitative hydrogen energy levels with error analysis
3. Genuine pre-observation prediction (before measurement)
4. Independent external validation

## Epistemic Honesty Statement

This document exists because intellectual honesty is more important than appearing impressive. Every framework can be made to look good with selective testing. The purpose of this test suite is to find where FTD fails, not just where it succeeds.

The CERN cavitation failure is documented prominently, not hidden. Disabled tests are listed, not removed. Circular reasoning (Born rule) is flagged. This honesty is itself a form of scientific credibility — it shows the framework is held to genuine standards.
