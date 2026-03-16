# FTD Topological Cavitation Analysis: CMS Open Data Results

**Date:** 2026-02-28
**Dataset:** CMS Run2016G MET (27M raw events, 1.5M selected with enhanced variables)
**MC Samples:** WJetsToLNu (20M), ZJetsToNuNu (170K), QCD HT700-1500 (976K)
**Analysis scripts:** `ftd_cern_*.py`, `ftd_cern_residual_test.py`

## The FTD Prediction

FTD's topological cavitation hypothesis predicts:
- High-energy events tear the discrete vacuum, creating expanding spherical bubbles
- The bubble radius scales as **R_cav ~ sqrt(E_MET)**
- This should manifest as a correlation between MET (missing transverse energy) and SV_dxy (secondary vertex transverse displacement)
- The correlation should be **stronger** than what SM backgrounds predict

## Observable Definition

- **MET** = Missing transverse energy (proxy for undetected energy)
- **R_cav** = max(SV_dxy) per event = maximum secondary vertex transverse displacement
- **dlenSig** = decay length significance = displacement / uncertainty
- **B-veto** = DeepFlavour b-tagging medium WP (removes known B hadron decays)

## Results Summary

### 1. Global Correlation: Near Zero (Expected)

| Category | Spearman rho | Interpretation |
|----------|-------------|----------------|
| All events | +0.003 | Dominated by inner tracker (R < 2.9cm) |
| Inner tracker (R < 2.9cm, 82%) | -0.039 | Detector acceptance dominates |
| Outer tracker (R >= 2.9cm, 18%) | +0.052 | Positive — correct direction |

**Verdict:** Global correlation is washed out by CMS pixel detector geometry. The first pixel barrel layer at 2.9 cm creates a sharp acceptance boundary that dominates the R_cav distribution.

### 2. Key Finding: Excess Correlation in Long-Lived Particle Candidates

When selecting events by decay-length significance (dlenSig), the data shows progressively stronger correlation that **exceeds SM Monte Carlo**:

| dlenSig Cut | rho(Data) | rho(MC) | **Excess** | **95% Bootstrap CI** |
|-------------|-----------|---------|------------|---------------------|
| > 0 (all) | +0.003 | +0.049 | -0.046 | [-0.058, -0.035] |
| > 5 | +0.045 | +0.043 | +0.002 | [-0.011, +0.014] |
| **> 10** | **+0.077** | **+0.034** | **+0.042** | **[+0.028, +0.058]** |
| **> 20** | **+0.101** | **+0.023** | **+0.079** | **[+0.059, +0.098]** |
| **> 30** | **+0.109** | **+0.020** | **+0.089** | **[+0.065, +0.114]** |
| **> 50** | **+0.107** | **+0.022** | **+0.085** | **[+0.055, +0.113]** |
| **> 75** | **+0.091** | **+0.003** | **+0.088** | **[+0.052, +0.123]** |
| **> 100** | **+0.075** | **-0.022** | **+0.096** | **[+0.048, +0.142]** |

All cuts from dlenSig > 10 onward show excess at **> 95% confidence level**.

### 3. Permutation Test

For dlenSig > 30 signal region (354,019 events):
- Observed rho = 0.1086
- Null distribution: mean = 0.00002, std = 0.00165
- **Z-score = 65.6 sigma**
- p-value < 10^-6

The correlation in data is real beyond any statistical doubt. The question is whether the EXCESS over MC is also robust.

### 4. Energy-Binned Displacement (dlenSig > 30)

| MET Range | Data Median R | MC Median R | Data P(R>5cm) | MC P(R>5cm) |
|-----------|--------------|-------------|---------------|-------------|
| 200-250 | 3.84 cm | 3.65 cm | 30.9% | 30.3% |
| 250-300 | 4.15 cm | 3.64 cm | 34.7% | 29.7% |
| 300-400 | 4.31 cm | 3.74 cm | 38.7% | 30.3% |
| 400-600 | 4.50 cm | 4.03 cm | 42.6% | 34.5% |
| 600-1000 | 4.53 cm | 4.27 cm | 43.8% | 41.4% |

Data displacements consistently exceed MC, with the excess growing at higher MET.

### 5. Scaling Exponent Test

FTD predicts R_cav ~ MET^0.5. The measured scaling:

| Quantity | Data beta | MC beta | FTD Prediction |
|----------|-----------|---------|----------------|
| Median R_cav | 0.122 | 0.137 | 0.5 |
| P90 R_cav | 0.026 | 0.030 | 0.5 |
| P(R>5cm) | 0.270 | 0.264 | 0.5 |

**Verdict:** The scaling exponent does NOT match the FTD prediction of 0.5. Both data and MC show much weaker energy dependence (~0.1-0.3) than predicted.

### 6. KS Tests: Data vs MC Shape

| MET Bin | KS Statistic | p-value | Significant? |
|---------|-------------|---------|-------------|
| 200-250 | 0.050 | 4.2e-07 | Yes |
| 250-300 | 0.089 | 2.0e-10 | Yes |
| 300-400 | 0.099 | 7.7e-10 | Yes |
| 400-600 | 0.107 | 8.1e-05 | Yes |
| 600-1000 | 0.124 | 0.15 | No (low stats) |

The R_cav shape difference between data and MC is highly significant in every well-populated MET bin.

### 7. Background-Subtracted Tail Excess (from deep analysis)

| R_cav Range | Observed / Expected (from exponential fit) |
|-------------|-------------------------------------------|
| R > 4 cm | 1.5-3x excess |
| R > 8 cm | 3-15x excess |
| R > 20 cm | 20-200x excess |

The tail of the R_cav distribution has a massive excess over the exponential background model, and this excess is energy-dependent.

## Honest Assessment

### What Supports FTD

1. **Excess correlation exists and is real** (65.6 sigma in permutation test)
2. **Excess is NOT explained by SM MC** (WJets + ZJets + QCD cannot reproduce it)
3. **Excess grows with dlenSig** (selecting purer long-lived candidates strengthens it)
4. **Direction is correct** (positive correlation between sqrt(MET) and R_cav)
5. **Displacement grows with energy** in data, consistently exceeding MC

### What Does NOT Support FTD

1. **Scaling exponent is wrong**: Data shows beta ~ 0.12, not 0.5. Both data and MC have similar weak scaling. FTD predicts a strong sqrt(E) dependence that is simply not observed.
2. **Global correlation is near zero**: Only visible after aggressive selection cuts
3. **ttbar MC is missing**: A major MET background (top quark decays produce real B hadrons + neutrinos) is unavailable on CERN Open Data. ttbar could explain some or all of the excess.
4. **MC statistics are limited**: 32K MC events vs 1.5M data events — the MC correlation estimates have larger uncertainty.
5. **Detector effects**: CMS pixel geometry dominates the R_cav distribution. The excess could reflect unmodeled detector systematics rather than new physics.

### Most Likely SM Explanation

The excess correlation in high-dlenSig events is most likely due to **ttbar events** (not available in our MC):
- Top quarks produce real MET (from W -> l nu)
- Top quarks produce B hadrons with large displacement and high dlenSig
- Higher-pT tops -> higher MET AND harder B hadrons with larger boost -> larger SV_dxy
- This would create exactly the positive correlation we observe
- The dlenSig dependence is also expected: higher dlenSig = more B-like = more top-like

### CRITICAL UPDATE: Discriminant Test Results (TEST 8)

We ran a discriminant test to distinguish FTD cavitation from missing ttbar. The key finding:

**The excess correlation is present in ALL SV mass windows, not just B-meson:**

| SV Mass Window | rho(Data) | rho(MC) | Excess |
|---------------|-----------|---------|--------|
| Low (< 1.5 GeV) | +0.090 | +0.033 | **+0.057** |
| D-meson (1.5-2.5 GeV) | +0.102 | +0.070 | +0.032 |
| Intermediate (2.5-4 GeV) | +0.101 | +0.010 | **+0.091** |
| B-meson (4-7 GeV) | +0.102 | -0.001 | **+0.103** |
| Exotic (> 7 GeV) | +0.069 | -0.044 | **+0.113** |

**Critical test — removing B/D mass events:**
- All signal: rho = +0.109 (excess = +0.089)
- After removing B-mass (4-7 GeV): rho = +0.109 (excess = +0.086) — **UNCHANGED**
- After removing B+D mass: rho = +0.104 (excess = +0.096) — **STILL PRESENT**
- B-mass only: rho = +0.102 (excess = +0.103) — also present

**Clean exotic sample (no B-jet + dlenSig>30 + SV mass < 4 GeV):**
- 106,316 data events with rho = +0.051
- 2,461 MC events with rho = +0.027
- Excess = +0.023 (reduced but still present)

**VERDICT: Missing ttbar CANNOT fully explain the excess.** The excess correlation is distributed uniformly across all SV mass scales — it is NOT concentrated in the B-meson window as the ttbar hypothesis predicts. Removing all B/D-mass events does not diminish the signal. However, the excess IS weaker in the cleanest exotic sample, suggesting ttbar contributes but does not account for everything.

### What Would Be Needed for a Definitive Test

1. **TTbar MC**: Still the most important missing piece. TTTo2L2Nu NanoAODSIM is NOT on CERN Open Data.
2. **Full CMS MC stack**: Including DY+jets, single-top, diboson
3. **Detector simulation**: Proper CMS detector response modeling
4. **Pileup corrections**: Not applied in our analysis
5. **An observable that DISTINGUISHES FTD from SM**: The mass-window test partially addresses this — the uniform distribution of excess across all SV masses is unexpected for ttbar but could be explained by other detector effects or pileup.

## Files Produced

| File | Description |
|------|-------------|
| `ftd_cern_exploration.py` | Initial 12-panel exploration (energy bins, fits, KS tests) |
| `ftd_cern_deep_analysis.py` | Background-subtracted tail analysis (9 panels) |
| `ftd_cern_bveto_analysis.py` | B-veto + SV discrimination (Docker/local) |
| `ftd_cern_mc_comparison.py` | Full MC comparison (WJets + ZJets + QCD, 12 panels) |
| `ftd_cern_residual_test.py` | Bootstrap CI + permutation + scaling tests (12 panels) |
| `ftd_cern_discriminant_test.py` | FTD vs TTbar mass-window discrimination (12 panels) |
| `ftd_cavitation_EXPLORATION.png` | Exploration plot |
| `ftd_cavitation_DEEP_ANALYSIS.png` | Deep analysis plot |
| `ftd_cavitation_BVETO.png` | B-veto local analysis |
| `ftd_cavitation_BVETO_FULL.png` | B-veto Docker (1.5M events) |
| `ftd_cavitation_MC_COMPARISON.png` | MC comparison plot |
| `ftd_cavitation_RESIDUAL_TEST.png` | Residual test plot |
| `ftd_cavitation_DISCRIMINANT.png` | FTD vs TTbar discriminant plot |
| `ftd_mc_cache.npz` | Cached MC data (32K events) |
| `ftd_full_enhanced.npz` | Enhanced data cache (1.5M events) |
| `ftd_full_extracted.npz` | Basic data cache (4.57M events) |
| `ftd_mc_comparison_results.txt` | MC comparison results |
| `ftd_residual_test_results.txt` | Residual test results |
| `ftd_cavitation_results.txt` | Overall results summary |
| `ftd_cern_scaling_corrections.py` | 5 functional form fits + excess characterization |
| `ftd_cern_scaling_followup.py` | Detector geometry + high-energy reversal + AIC/BIC |
| `ftd_cern_reinvestigation.py` | 8-test reinvestigation (R^2, excess, width, AIC, deltaR, bias, partial corr, kinematic) |
| `ftd_cavitation_SCALING_CORRECTIONS.png` | Scaling corrections figure |
| `ftd_cavitation_FOLLOWUP.png` | Follow-up analysis figure |
| `ftd_cavitation_REINVESTIGATION.png` | 16-panel reinvestigation figure |
| `ftd_reinvestigation_results.txt` | Reinvestigation results |
| `ftd_cern_partial_correlation_deep.py` | Deep partial correlation investigation (10 tests) |
| `ftd_partial_correlation_DEEP.png` | 12-panel deep investigation figure |
| `ftd_partial_correlation_deep_results.txt` | Deep investigation results |
| `cavitation_consciousness_bridge_verification.py` | Beta=1/2 derivation verification (13 checks) |
| `cavitation_threshold_verification.py` | Energy threshold analysis (10 checks) -- rules out FTD cavitation at LHC |
| `cavitation_hierarchy_verification.py` | Multi-scale hierarchy analysis (10 checks) -- 8 scales mapped, QCD confirmed, no FTD scale matches |

## Conclusion (Updated after Hierarchy Analysis, v1.5)

The CMS MET data shows a real, statistically significant excess correlation between missing energy and secondary vertex displacement that is not reproduced by available SM Monte Carlo (WJets, ZJets, QCD). Key findings:

1. **Excess is real**: 65.6 sigma in permutation test, >95% CL in bootstrap CI for dlenSig > 10
2. **Excess is NOT solely from B hadrons**: Present uniformly across all SV mass windows; removing B/D-mass events does not diminish it
3. **Excess grows with purity**: Stronger at higher dlenSig cuts, reaching rho_diff ~ +0.10
4. **Scaling exponent weakly disfavored**: beta ~ 0.10, forced beta=0.5 at DAIC=4.0 (weakly disfavored, not excluded)
5. **Correlation survives kinematic control**: Partial correlation rho=+0.103 after removing SV mass effect (only 5% reduction)
6. **Not a selection artifact**: Correlation persists at rho~0.10-0.13 within narrow dlenSig bands
7. **Excess is FLAT**: deltaR ~ 0.4 cm constant across all MET bins (normalization anomaly, not scaling)
8. **Observable mismatch**: max(SV_dxy) measures kinematic flight distance, NOT vacuum bubble radius
9. **Deep investigation (v1.3): 7/10 tests confirm genuine signal**: Survives iterative controls (26% max reduction), B-fraction reweighting (7% reduction), mass/ntracks stratification (universal), nonlinear distance correlation, and permutation testing (Z=24.9)
10. **Within-bin correlation is weak**: Global rho=+0.109 is mainly a between-bin level shift; within individual MET bins, rho ~ 0.02
11. **Concentrated in 1-10 cm range**: Zero correlation at R<1 cm and R>10 cm; consistent with detector geometry (pixel layers at 4.4, 7.3, 10.2 cm)
12. **MC explains ~42% of partial correlation**: MC partial rho=+0.043 vs data +0.103; excess partial rho=+0.060
13. **ENERGY THRESHOLD RULES OUT FTD CAVITATION (v1.4)**: epsilon_crit derived from K_C = sqrt(G*^3/2) ~ 3.60 Planck energies; minimum cavitation energy ~ 5 x 10^20 GeV; LHC is 4 x 10^16 times too weak; R_cav at LHC would be sub-Planck (logical impossibility in discrete lattice)
14. **No loophole mechanism identified (v1.4)**: RG running, quantum tunneling, collective effects all insufficient to bridge the 10^16 gap
15. **Multi-scale hierarchy mapped (v1.5)**: 8 cavitation scales from substrate (10^19 GeV) to molecular (0.2 eV); QCD deconfinement is real (RHIC/LHC) but gives only fm-scale bubbles; fm-to-cm gap is 10^12
16. **No FTD-specific scale produces CMS-visible bubbles (v1.5)**: Molecular scale (125 um) is marginally visible but is standard chemistry, not FTD

**Balance of evidence:**
- The mass-window uniformity and universal ntracks/mass stratification argue AGAINST a pure ttbar explanation
- The surviving partial correlation (7/10 deep tests) argues FOR a non-kinematic effect in data
- The flat excess profile argues AGAINST the specific FTD R ~ sqrt(E) prediction
- The within-bin weakness (rho~0.02) suggests the correlation is a LEVEL SHIFT (median R increases with MET bins) rather than a within-event scaling
- The 1-10 cm concentration suggests detector geometry may play a role
- MC explains about 42% of the partial correlation; the remaining 58% excess is unaccounted for
- **DECISIVE: Energy threshold analysis (v1.4) rules out FTD cavitation at LHC by 10^16** — the anomaly CANNOT be FTD cavitation regardless of other considerations
- **CONFIRMED: Multi-scale hierarchy (v1.5) independently confirms** — all 8 cavitation scales examined; QCD deconfinement (the most interesting) gives fm-scale bubbles; no FTD-specific scale reaches CMS resolution
- The most likely explanation remains: missing ttbar MC + detector effects + other missing MC + pileup

**Epistemic status: [DISFAVORED] — A genuine non-kinematic anomaly is present in the data (confirmed by 7/10 deep tests, Z=24.9 permutation significance, universal across mass bands and track multiplicities). However, energy threshold analysis (v1.4) CONCLUSIVELY rules out FTD topological cavitation at LHC energies: the minimum cavitation energy is ~5 x 10^20 GeV, while the LHC delivers ~10^4 GeV. The 10^16 hierarchy gap cannot be bridged by any known mechanism. The CERN anomaly is real but requires a different explanation (missing ttbar MC, detector effects, or other SM process).**
