# Threshold-Crossing -> Born Rule Test -- Results

**Date:** 2026-05-23
**Pre-registration:** docs/theory/06_consciousness/PREREG_THRESHOLD_CROSSING_BORN_v1.md
**Git tag:** preregister-threshold-crossing-born-v1

---

## Outcome

**Outcome C.** RICE / UPCROSSING SCALING. H_Rice R^2 = 0.9923, H_power R^2 = 0.7137. [NUMERICAL FACT - Gaussian-process upcrossing] + [CLOSED NEGATIVE for Born]; SPEC_SIX_ALGORITHMS.md:65 + AUDIT_EPISTEMIC_AUDIT.md:393 need retag.

---

## Summary statistics

- L = 24, K_B = 0.5, A = 2.0, trials = 100, ticks_per_trial = 80, total samples = 8000
- Voxels with at least one manifestation event: **13824 / 13824**
- Total events: **5325583**
- mu_sq range: [0.0472, 0.3201], mean = 0.1148
- freq range: [0.000750, 0.106125], mean = 0.048155
- Voxels in analysis mask: **7973 / 13824**
- Non-empty bins: **14**

---

## Bin table

| bin range (mu^2) | mean mu^2 | mean sigma^2 | mean freq | n sites |
|---|---|---|---|---|
| 0.070 - 0.080 | 0.0770 | 0.0040 | 0.020320 | 570 |
| 0.080 - 0.085 | 0.0824 | 0.0046 | 0.026840 | 569 |
| 0.085 - 0.089 | 0.0869 | 0.0051 | 0.031736 | 570 |
| 0.089 - 0.093 | 0.0910 | 0.0057 | 0.036881 | 569 |
| 0.093 - 0.096 | 0.0945 | 0.0062 | 0.040955 | 570 |
| 0.096 - 0.100 | 0.0980 | 0.0068 | 0.044645 | 569 |
| 0.100 - 0.105 | 0.1026 | 0.0077 | 0.049384 | 569 |
| 0.105 - 0.111 | 0.1082 | 0.0092 | 0.052436 | 570 |
| 0.111 - 0.118 | 0.1140 | 0.0106 | 0.056232 | 569 |
| 0.118 - 0.126 | 0.1214 | 0.0129 | 0.059751 | 570 |
| 0.126 - 0.136 | 0.1297 | 0.0157 | 0.060976 | 569 |
| 0.136 - 0.153 | 0.1432 | 0.0203 | 0.066306 | 570 |
| 0.153 - 0.177 | 0.1639 | 0.0313 | 0.069066 | 569 |
| 0.177 - 0.320 | 0.2177 | 0.0879 | 0.065739 | 570 |

---

## Fits

**H_power: `freq ~ |J|^n`**  
- n = **2.1858**
- 95% CI: [1.2565, 3.9874]
- R^2 = 0.7137
- Born predicts n = 2; classical linear predicts n = 1.

**H_Rice: `log freq = log B - k * (K_B - mu)^2 / sigma^2`**  
- k = 0.0971
- log B = -2.6476
- R^2 = 0.9923

---

## Full data

See `threshold_crossing_born_2026-05-23.csv` (13824 rows).
