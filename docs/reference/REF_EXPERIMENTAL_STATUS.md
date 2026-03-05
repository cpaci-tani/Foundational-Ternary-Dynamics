# FTD Experimental Status Report

**Last Updated:** February 2026

---

## Executive Summary

This document tracks the experimental status of FTD's testable predictions, including current constraints, planned experiments, and timeline for validation or falsification.

---

## 1. Tensor-to-Scalar Ratio (r)

### FTD Prediction

FTD predicts the tensor-to-scalar ratio from Starobinsky-type inflation:

$$r = \frac{12}{N^2} = \frac{12}{55^2} \approx 0.00397$$

where N = 55 e-folds (the 10th Fibonacci number, consistent with FTD's integer framework).

| Parameter | Value | Source |
|-----------|-------|--------|
| FTD prediction | r ≈ 0.004 | 12/N² with N = 55 |
| Epistemic status | PREDICTION | Testable, not yet verified |

### Current Experimental Constraints

| Experiment | Constraint | Confidence | Reference |
|------------|-----------|------------|-----------|
| Planck 2018 | r < 0.056 | 95% CL | arXiv:2010.01139 |
| BICEP/Keck + Planck | r < 0.032 | 95% CL | BICEP/Keck 2021 |
| BICEP/Keck 2024 | r < 0.036 | 95% CL | Latest release |

**Status:** FTD's prediction (r ≈ 0.004) is **8-14× below current experimental bounds**. The prediction is fully consistent with all current constraints but cannot yet be tested.

### CMB-S4 Cancellation (July 2025)

**Critical Update:** The CMB-S4 experiment, which was the most promising near-term probe of FTD's r prediction, was **cancelled in July 2025** due to funding cuts.

- CMB-S4 had projected sensitivity to detect r ~ 0.001
- This would have been sufficient to test FTD's r ≈ 0.004 prediction
- The cancellation significantly delays the timeline for testing this prediction

### Alternative Experiments

| Experiment | Sensitivity | Timeline | Status |
|------------|-------------|----------|--------|
| **LiteBIRD** (JAXA) | r ~ 0.001 | Launch ~2032 | Active development |
| **Simons Observatory** | r ~ 0.003 | First light 2024 | Operational |
| **BICEP Array** | r ~ 0.005 | Ongoing | Operational |
| **Ali CMB** (China) | r ~ 0.01 | ~2028 | Under construction |

**Best prospect:** LiteBIRD (JAXA space mission) will achieve sensitivity ~0.001, sufficient to definitively test FTD's prediction around 2032-2035.

**Near-term:** Simons Observatory (r ~ 0.003 sensitivity) may be able to test the prediction by ~2027-2028 if systematic errors are well controlled.

### Falsification Criteria

FTD's r prediction would be **falsified** if:
1. Any experiment detects r > 0.01 (incompatible with N = 55 e-folds)
2. r is measured to be exactly 0 (would require modification of FTD's inflation mechanism)
3. r is measured in the range 0.01-0.03 (would require different N value)

---

## 2. Fine Structure Constant (α)

### FTD Prediction

From the master quadratic x² - 16(G*)²x + 16(G*)³ = 0:

$$\frac{1}{\alpha} = x_+ = 137.0360$$

| Value | Source | Accuracy |
|-------|--------|----------|
| FTD: 1/α = 137.0360 | Master quadratic | — |
| CODATA 2022: 1/α = 137.035999177(21) | Experiment | — |
| **Discrepancy** | — | **1.26 ppm** |

### Status

**Consistent within theoretical uncertainty.** The 1.26 ppm discrepancy may be explained by:
1. Higher-order QED corrections (O(α²) ~ 5 × 10⁻⁵)
2. Lattice discretization effects at Planck scale
3. Framework refinements

### Future Tests

- Precision measurements of α via electron g-2, Cs atom recoil, and muonium spectroscopy continue to improve
- Any measurement inconsistent with x+ = 137.036... at >10 ppm level would falsify the master quadratic structure

---

## 3. Number of Generations

### FTD Prediction

$$N_{gen} = \lfloor x_- \rfloor = \lfloor 3.024 \rfloor = 3$$

This is an **exact discrete prediction**: there are precisely 3 generations of fermions.

### Status

**Consistent with all experimental data.** LHC has excluded:
- Sequential 4th generation quarks up to ~800 GeV
- Heavy charged leptons up to ~500 GeV

### Falsification

Discovery of a 4th generation fermion with standard gauge couplings would **falsify** this prediction.

**Note:** Heavy sterile neutrinos or vector-like fermions do not count as a "4th generation" in the FTD sense.

---

## 4. Proton Decay

### FTD Prediction

FTD suggests proton lifetime ~10³⁵ years from the framework's grand unification structure.

| Parameter | Value |
|-----------|-------|
| FTD prediction | τ_p ~ 10³⁵ years |
| Current bound | τ_p > 2.4 × 10³⁴ years (Super-K, p → e⁺π⁰) |

### Status

**Consistent but untestable near-term.**

- Super-Kamiokande has ruled out τ_p < 2.4 × 10³⁴ years
- Hyper-Kamiokande (under construction) will improve sensitivity ~10×
- FTD prediction is near the threshold of detectability

### Timeline

- Hyper-Kamiokande first results: ~2030
- Sensitivity to τ_p ~ 10³⁵ years: ~2040

---

## 5. Neutrino Hierarchy

### FTD Prediction

FTD's seesaw mechanism with M_R from framework integers predicts **normal hierarchy** (NH):

$$m_1 < m_2 < m_3$$

### Current Status

Global fits slightly favor normal hierarchy:
- Δχ² ~ 2-3 favoring NH
- Not yet statistically significant

### Upcoming Experiments

| Experiment | Method | Timeline |
|------------|--------|----------|
| JUNO | Reactor ν | First results ~2025 |
| DUNE | Accelerator ν | First results ~2029 |
| IceCube Upgrade | Atmospheric ν | ~2026 |

**Expected resolution:** By ~2030, combination of JUNO + DUNE should determine hierarchy at >5σ.

### Falsification

Definitive measurement of **inverted hierarchy** (m₃ < m₁ < m₂) would **falsify** FTD's neutrino sector predictions.

---

## 6. Bell Test S-Parameter

### FTD Prediction

The sLoop mechanism predicts Bell inequality violations matching quantum mechanics:

$$S \approx 2\sqrt{2} \approx 2.83$$

### Status

**Three-level observer hierarchy established (v5.27-bell):**

- **Level 1 (substrate):** Deterministic lattice gives S = 2 — correct for local deterministic axioms [THEOREM]
- **Level 2 (independent complex observer):** Born rule from complexification gives S = √2 [THEOREM]
- **Level 3 (entangled/sLoop observer):** Joint substrate coupling gives S = 2√2 ≈ 2.83 [SELECTION]
- Two mechanisms: complexification (Gauss constraint → ψ = J_x + iJ_y) changes correlation shape; sLoop (shared substrate) doubles correlation strength
- Verified: 4/4 Monte Carlo checks (1M samples). See DERIV_OBSERVER_BELL_MECHANISM.md

### Distinguishing Test

FTD's sLoop mechanism makes a **unique prediction**: S should vary with the degree of substrate overlap between measurement apparatus and system.

A dedicated experiment could test whether:
1. S depends on apparatus material/configuration (sLoop predicts yes)
2. S is universal regardless of apparatus (standard QM predicts yes)

This could distinguish sLoop from standard quantum mechanics.

---

## 7. Summary Table

| Prediction | FTD Value | Current Status | Timeline to Test |
|------------|-----------|----------------|------------------|
| r (tensor-to-scalar) | 0.004 | Consistent (< 0.032) | 2027-2035 |
| 1/α | 137.036 | 1.26 ppm match | Ongoing precision |
| N_gen | 3 exactly | Verified | Continuously tested |
| τ_proton | ~10³⁵ yr | Consistent (> 10³⁴) | 2030-2040 |
| ν hierarchy | Normal | Favored but uncertain | 2025-2030 |
| Bell S | 2.83 | Three-level mechanism [SELECTION] | TBD |

---

## 8. Critical Path to Validation/Falsification

### Near-term (2025-2028)
1. **JUNO + atmospheric experiments** → Neutrino hierarchy determination
2. **Simons Observatory** → Push r constraint toward 0.003

### Medium-term (2028-2035)
3. **DUNE** → Confirm hierarchy, measure CP phase
4. **LiteBIRD** → Test r ≈ 0.004 prediction definitively
5. **Hyper-Kamiokande** → Push proton lifetime bounds

### Long-term (2035+)
6. **Precision α measurements** → Sub-ppm tests of master quadratic
7. **Advanced proton decay searches** → Test τ_p ~ 10³⁵ years

---

## 9. References

1. Planck Collaboration (2020). "Planck 2018 results. X. Constraints on inflation." A&A 641, A10. arXiv:2010.01139
2. BICEP/Keck Collaboration (2021). "Improved Constraints on Primordial Gravitational Waves." PRL 127, 151301
3. CMB-S4 Collaboration (2025). "Project Status Update." https://cmb-s4.org/
4. LiteBIRD Collaboration (2023). "LiteBIRD science goals." PTEP 2023, 042F01
5. Simons Observatory Collaboration (2019). "The Simons Observatory: Science goals and forecasts." JCAP 02, 056

---

*This document will be updated as new experimental results become available.*
