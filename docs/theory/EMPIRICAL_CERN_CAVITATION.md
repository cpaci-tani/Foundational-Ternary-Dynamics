# Empirical Test: Topological Cavitation in CMS MET Data

**Document ID:** EMPIRICAL_CERN_CAVITATION
**Category:** 10. Empirical Validation & Observations
**Epistemic Status:** [DISFAVORED] — Partial correlation genuine (7/10 tests survive) but energy threshold analysis rules out FTD cavitation at LHC by ~10^16; anomaly requires alternative explanation
**Date:** 2026-02-28
**Version:** 1.5 (multi-scale hierarchy analysis confirms no FTD scale matches, 2026-02-28)

---

## 1. Overview

This document reports the first external data test of an FTD prediction using publicly available CMS Open Data. The analysis investigates FTD's **topological cavitation hypothesis** — the prediction that high-energy events should correlate with displaced secondary vertices due to vacuum bubble formation in the discrete substrate.

**Result:** A statistically significant excess correlation is observed that is not reproduced by available Standard Model Monte Carlo. The correlation **survives** partial correlation control for kinematics (SV mass, B-jet, ntracks) and is **not** a selection artifact (persists within narrow dlenSig bands). Forced $\beta = 0.5$ is weakly disfavored ($\Delta$AIC = 4.0) but not definitively excluded. The excess displacement $\delta R = \text{med}(\text{data}) - \text{med}(\text{MC})$ is approximately **constant** (~0.4 cm) across all MET bins, rather than growing as $\sqrt{E}$. A critical observable mismatch was identified: $R_{\text{cav}} = \max(\text{SV\_dxy})$ measures hadron flight distance (kinematic), not vacuum bubble radius (topological). Missing ttbar MC remains a critical limitation.

### Dependencies

| Depends On | Status |
|------------|--------|
| FTD discrete spacetime axioms (POSTULATE 1-5) | [AXIOM] |
| Topological cavitation mechanism | [CONJECTURE] |
| CMS Run2016G MET dataset (Record 30529) | External data |
| WJetsToLNu MC (Record 69747) | External data |
| ZJetsToNuNu MC (Records 74908, 74910) | External data |
| QCD HT700-1500 MC (Records 63081, 63138) | External data |

### What This Test Does NOT Establish

- This is NOT a discovery of new physics
- This is NOT proof that FTD is correct
- This is NOT a full CMS-level analysis (no detector simulation, no pileup correction, incomplete MC)
- Missing ttbar MC is a critical limitation

---

## 2. The FTD Prediction

### 2.1 Topological Cavitation Hypothesis

In FTD's discrete spacetime, sufficiently energetic events can "tear" the lattice, creating expanding spherical vacuum bubbles. The bubble radius should scale with the event energy:

$$R_{\text{cav}} \sim \sqrt{E_{\text{MET}}}$$

This predicts:
1. A positive correlation between MET (missing transverse energy) and secondary vertex displacement
2. The correlation should strengthen when selecting purer long-lived particle candidates
3. The scaling exponent should be $\beta = 0.5$

### 2.1.1 Derivation of $\beta = 0.5$ [SELECTION]

The scaling exponent $\beta = 0.5$ is **derived** (not postulated) from the consciousness coupling $k = 1/2$ via the Domain A-to-B transition mechanism. The derivation chain:

1. **$k = 1/2$** from complementation fixed point $f(k) = 1-k$ [THEOREM]
2. **Cavitation = Domain A-to-B transition** — extreme energy drives local $k_{\text{eff}}$ below $k_{\text{crit}} = 4/G^*$ [CONJECTURE]
3. **Bubble boundary = $S^2$** in $D = 3$ (codimension-1 surface) [THEOREM given Step 2]
4. **Energy flux** $\varepsilon(R) = E/(4\pi R^2)$, threshold condition gives $R \sim \sqrt{E}$ [SELECTION]
5. **$\beta = 1/(D-1) = 1/2$** for $D = 3$ [THEOREM given premises]

The value $1/2$ appearing in both the consciousness coupling and the cavitation exponent reflects boundary self-duality in $D = 3$. See [DERIV_CAVITATION_CONSCIOUSNESS_BRIDGE.md](DERIV_CAVITATION_CONSCIOUSNESS_BRIDGE.md) for the full derivation. Numerical verification: `simulations/cavitation_consciousness_bridge_verification.py` (13/13 checks passed).

**Caveat:** $\beta = 1/2$ is also the dimensional-analysis default for any spherical threshold in 3D.

### 2.2 Observable Mapping

| FTD Concept | CMS Observable |
|-------------|----------------|
| Vacuum bubble radius | max(SV_dxy) — maximum secondary vertex transverse displacement |
| Event energy | MET_pt — missing transverse energy |
| Bubble purity | SV_dlenSig — decay length significance |
| Background contamination | B-jet tag + SV mass window |

---

## 3. Dataset and Methods

### 3.1 Data

- **Dataset:** CMS Run2016G MET (NanoAODv9, Record 30529)
- **Raw events:** 27,148,615
- **Selection:** MET > 200 GeV, nSV > 0
- **Selected events (basic):** 4,570,000 (16.8%)
- **Selected events (enhanced):** 1,500,000 (with SV_mass, btag, dlenSig)

### 3.2 Monte Carlo Samples

| Sample | Record | Events Processed | Selected | Cross-section |
|--------|--------|-----------------|----------|---------------|
| WJetsToLNu | 69747 | 19,800,000 | 4,599 (0.02%) | 61526.7 pb |
| ZJetsToNuNu (200-Inf) | 74910 | 33,000 | 20,445 (61.5%) | 18.01 pb |
| ZJetsToNuNu (100-200) | 74908 | 136,000 | 3,616 (2.7%) | 91.82 pb |
| QCD HT1000-1500 | 63081 | 477,000 | 2,438 (0.5%) | 1005.0 pb |
| QCD HT700-1000 | 63138 | 499,000 | 529 (0.1%) | 6831.0 pb |

**Critical missing sample:** TTTo2L2Nu NanoAODSIM is NOT available on CERN Open Data Portal.

### 3.3 Statistical Methods

- **Spearman rank correlation** between sqrt(MET) and max(SV_dxy)
- **Bootstrap confidence intervals** (1000 resamples) for data-MC correlation difference
- **Permutation test** (5000 permutations) for correlation significance
- **KS tests** per MET bin for data-MC shape comparison
- **Power-law fitting** for scaling exponent $\beta$
- **SV mass window discrimination** to distinguish FTD from ttbar

---

## 4. Results

### 4.1 Global Correlation: Near Zero

| Category | Spearman $\rho$ | Events |
|----------|-----------------|--------|
| All events | +0.003 | 4,570,000 |
| Inner tracker (R < 2.9 cm) | -0.039 | 82% of events |
| Outer tracker (R $\geq$ 2.9 cm) | +0.052 | 18% of events |

**Interpretation:** CMS pixel barrel layer at 2.9 cm creates a sharp acceptance boundary dominating R_cav distribution. Global correlation is washed out by detector geometry.

### 4.2 Excess Correlation in Long-Lived Candidates [KEY RESULT]

Selecting events by decay-length significance reveals growing excess over SM MC:

| dlenSig Cut | $\rho$(Data) | $\rho$(MC) | **Excess** | **95% Bootstrap CI** |
|-------------|-------------|-----------|------------|---------------------|
| > 0 | +0.003 | +0.049 | -0.046 | [-0.058, -0.035] |
| > 5 | +0.045 | +0.043 | +0.002 | [-0.011, +0.014] |
| > 10 | +0.077 | +0.034 | **+0.042** | [+0.028, +0.058] |
| > 20 | +0.101 | +0.023 | **+0.079** | [+0.059, +0.098] |
| > 30 | +0.109 | +0.020 | **+0.089** | [+0.065, +0.114] |
| > 50 | +0.107 | +0.022 | **+0.085** | [+0.055, +0.113] |
| > 100 | +0.075 | -0.022 | **+0.096** | [+0.048, +0.142] |

All cuts from dlenSig > 10 onward show excess at > 95% confidence level. **[CONJECTURE]**

### 4.3 Statistical Significance

**Permutation test** for dlenSig > 30 signal region (354,019 events):
- Observed $\rho$ = 0.1086
- Null distribution: mean = 0.00002, std = 0.00165
- Z-score = **65.6 $\sigma$**
- p-value < $10^{-6}$

The correlation in data is real beyond statistical doubt. The question is whether the *excess over MC* is physical.

### 4.4 Scaling Exponent: DOES NOT MATCH [FALSIFICATION]

| Quantity | Data $\beta$ | MC $\beta$ | FTD Prediction |
|----------|-------------|-----------|----------------|
| Median R_cav | 0.097 | 0.214 | **0.5** |
| P90 R_cav | 0.026 | 0.030 | **0.5** |
| P(R > 5 cm) | 0.270 | 0.264 | **0.5** |

**[CONJECTURE] $\to$ FALSIFIED:** The specific FTD prediction $R \sim \sqrt{E}$ is not supported. Both data and MC show energy dependence far weaker than predicted.

#### 4.4.1 Scaling Correction Analysis (v1.1 Update)

Five functional forms were tested against median R_cav vs MET data in the dlenSig > 30 signal region:

| Model | Parameters | $\chi^2$ | AIC | Interpretation |
|-------|-----------|---------|-----|----------------|
| **Saturating** | $R_\infty = 4.55$ cm, $E_{1/2} = 116$ GeV | 10.3 | 14.3 | **BEST FIT** |
| Lattice-corrected | $\beta_{\text{bare}} = 1.10$, $R_{\max} = 4.45$ cm | 8.3 | 14.3 | Free $\beta$ near 1.0 |
| Log-corrected | $\beta = 0.74$, $\gamma = -0.40$ | 13.6 | 19.6 | Logarithmic suppression |
| **Forced $\beta = 0.5$** | $R_{\max} = 4.90$ cm | **17.1** | **21.1** | **DISFAVORED ($\Delta$AIC = +6.8)** |
| Pure power law | $\beta = 0.097$ | 27.8 | 31.8 | Worst fit |

**Key findings:**

1. **Forced $\beta = 0.5$ is strongly disfavored** ($\Delta$AIC = +6.8 relative to best model; $\chi^2/\text{dof} = 2.14$ vs 1.29). The FTD naive prediction cannot be rescued by lattice saturation effects.

2. **The data is best described by saturation**, with $R_\infty \approx 4.55$ cm and $E_{1/2} \approx 116$ GeV. The displacement saturates at ~4.5 cm regardless of energy.

3. **The lattice-corrected model (free $\beta$) finds $\beta_{\text{bare}} \approx 1.1$**, not 0.5. If saturation were the explanation, the bare exponent would need to be ~1.0 (linear in E), not the FTD-predicted 0.5.

4. **The effective energy fraction decreases sharply**: if $R \sim (f \cdot E)^{0.5}$, then $f \sim E^{-0.81}$. At 1000 GeV, only 27% of the effective energy fraction contributes compared to 200 GeV.

#### 4.4.2 Detector Geometry as Explanation for Saturation

The saturation at $R_{\max} \approx 4.55$ cm is **suspiciously coincident with the CMS pixel barrel geometry**:

| Feature | R_cav Peak (cm) | CMS Layer | Distance |
|---------|-----------------|-----------|----------|
| Peak 3 | 4.26 | Pixel L1 (4.4 cm) | 0.14 cm |
| Peak 4 | 7.17 | Pixel L2 (7.3 cm) | 0.13 cm |
| Peak 5 | 10.08 | Pixel L3 (10.2 cm) | 0.12 cm |

**Distribution by tracker region:**

| Region | Fraction | Description |
|--------|----------|-------------|
| R < 4.4 cm | 67.1% | Inside pixel L1 |
| 4.4-7.3 cm | 16.6% | Between pixel L1-L2 |
| 7.3-10.2 cm | 9.7% | Between pixel L2-L3 |
| > 10.2 cm | 6.6% | Outside pixel detector |

The R_cav distribution peaks align precisely with CMS pixel barrel layers to within ~0.15 cm. **The saturation at ~4.55 cm almost certainly reflects detector acceptance** (tracking efficiency drops sharply outside the pixel volume), not a physical saturation of the cavitation mechanism. **[EMERGENT]**

#### 4.4.3 High-Energy Reversal

Within-bin correlation $\rho(\sqrt{\text{MET}}, R_{\text{cav}})$ shows unexpected behavior at high energy:

| MET Range | N(data) | $\rho$ | Median R | P(R>5) |
|-----------|---------|--------|----------|--------|
| 200-300 | 34,996 | +0.044 | 3.94 | 0.319 |
| 300-400 | 5,619 | +0.039 | 4.31 | 0.387 |
| 400-500 | 1,407 | +0.031 | 4.39 | 0.409 |
| 500-600 | 454 | **-0.039** | 4.82 | 0.480 |
| 600-700 | 178 | **-0.060** | 4.65 | 0.472 |
| 700-1500 | 166 | +0.12 | 4.26 | 0.375 |

The within-bin correlation **reverses** at MET 500-700 GeV, then becomes noisy at very high MET (N < 200). This is inconsistent with a simple monotonic scaling law. The reversal may reflect:
1. Saturation (median R_cav plateaus at ~4.5 cm, inside pixel L1)
2. Selection effects (very high MET events have different SV composition)
3. B-jet fraction drops from 57% to 37% at high MET, changing the SV population

#### 4.4.4 Data vs MC Scaling by Subpopulation

| Selection | $\beta_{\text{data}}$ | $\beta_{\text{MC}}$ | Difference |
|-----------|---------------------|-------------------|------------|
| All SVs | +0.103 | +0.214 | -0.111 |
| Exotic (m<1.5, no B) | -0.019 | +0.034 | -0.053 |
| B-mass (4-7 GeV) | +0.165 | +0.065 | +0.100 |
| Outer tracker (R>2.9) | +0.304 | +0.205 | +0.099 |
| Inner tracker (R<2.9) | +0.073 | -0.026 | +0.099 |

**The difference $\beta_{\text{data}} - \beta_{\text{MC}} \approx +0.10$ is consistent across B-mass, outer, and inner subpopulations.** The exotic sample shows weaker effect. The MC actually has *higher* overall $\beta$ than data, suggesting the excess is not a different scaling law but a uniform upward shift in large-R probability.

### 4.5 Reinvestigation: 8 Independent Tests (v1.2 Update)

Three parallel explorations identified critical issues with the prior analysis:

1. **Observable mismatch**: $R_{\text{cav}} = \max(\text{SV\_dxy})$ measures how far a B/D meson flew before decaying — a purely kinematic quantity ($d = \gamma\beta c\tau \propto p/m$), NOT a vacuum bubble radius.
2. **Trivial kinematic explanation**: Higher MET $\to$ higher-$p_T$ jets $\to$ more boosted hadrons $\to$ larger flight distance. This creates MET-$R_{\text{cav}}$ correlation without new physics.
3. **Missing tests**: Several critical tests had never been performed.

An 8-test reinvestigation was conducted to determine whether $\beta = 0.5$ can be rescued when properly tested on the **excess** (data $-$ MC) rather than raw data.

#### 4.5.1 Test 1: $R^2$ vs MET (Direct Linear Regression)

If $R \sim \sqrt{E}$, then $R^2 \sim E$ (linear). Event-level regressions:

| Model | $R^2_{\text{fit}}$ (Data) | $R^2_{\text{fit}}$ (MC) |
|-------|--------------------------|------------------------|
| $R^2$ vs MET | 0.0022 | 0.0001 |
| $R$ vs MET | 0.0092 | 0.0008 |
| $R$ vs $\sqrt{\text{MET}}$ | **0.0101** | **0.0008** |

All correlations are extremely weak. Best model is $R$ vs $\sqrt{\text{MET}}$ for both data and MC — but with $R^2 \approx 0.01$, this explains only 1% of variance.

#### 4.5.2 Test 2: Excess Scaling (Data $-$ MC)

The critical missing test: fitting $\sqrt{E}$ scaling to the **excess** displacement. However, this test **failed** due to MC statistics — the excess mean displacement per MET bin was wildly unstable (ranging from $-800$ to $+500$) because MC normalization creates artifacts with only 7,302 signal MC events. **Inconclusive.**

#### 4.5.3 Test 3: Distribution Width vs MET

| Metric | Data scaling | MC scaling | Ratio trend |
|--------|-------------|-----------|-------------|
| IQR $\sim$ MET$^\gamma$ | $\gamma = 0.169$ | $\gamma = 0.293$ | Flat ($p = 0.44$) |

The width ratio (data/MC) is consistently > 1 (data has broader R_cav distributions) but does NOT increase with MET. This is inconsistent with cavitation broadening.

#### 4.5.4 Test 4: Improved Forced $\beta = 0.5$ (3 Models, Multistart)

With properly parameterized saturation models and bootstrapped errors:

| Model | $\chi^2$ | AIC | $\Delta$AIC | Status |
|-------|---------|-----|-------------|--------|
| Saturating (free $\beta$) | 0.023 | -35.9 | 0.0 | **Best** |
| Screened $\sqrt{E}$ ($\beta = 0.5$) | 0.034 | -32.0 | +4.0 | Weakly disfavored |
| Tanh $\sqrt{E}$ ($\beta = 0.5$) | 0.040 | -30.4 | +5.6 | Weakly disfavored |
| Michaelis-Menten $\sqrt{E}$ | 0.079 | -23.6 | +12.4 | Strongly disfavored |
| Power law (free $\beta$) | 0.113 | -20.0 | +15.9 | Worst |

**Key revision:** The $\Delta$AIC for forced $\beta = 0.5$ drops from **+6.8** (v1.1 analysis) to **+4.0** (v1.2 with better parameterization and bootstrapped errors). This moves $\beta = 0.5$ from "strongly disfavored" to **"weakly disfavored"** ($2 < \Delta\text{AIC} < 6$). The screened model $R = A\sqrt{E} \exp(-E/E_{\text{screen}})$ with $E_{\text{screen}} \approx 1063$ GeV provides a physically reasonable forced-$\beta=0.5$ fit.

#### 4.5.5 Test 5: Data $-$ MC Displacement Difference

The excess $\delta R = \text{median}(\text{data}) - \text{median}(\text{MC})$ per MET bin:

| MET range | $\delta R$ (cm) | Error (cm) |
|-----------|----------------|------------|
| 200-225 | +0.19 | 0.13 |
| 225-250 | +0.18 | 0.17 |
| 250-275 | +0.56 | 0.19 |
| 275-300 | +0.51 | 0.16 |
| 300-350 | +0.56 | 0.13 |
| 350-400 | +0.54 | 0.22 |
| 400-500 | +0.41 | 0.19 |
| 500-600 | +0.67 | 0.47 |
| 600-800 | +0.38 | 0.52 |
| 800-1200 | -0.94 | 1.64 |

Model comparison for $\delta R$ profile:

| Model | AIC | Interpretation |
|-------|-----|----------------|
| **Constant** ($\delta R = c$) | **-13.8** | **Best: excess is flat** |
| Linear in $\sqrt{\text{MET}}$ | -6.6 | $\Delta$AIC = +7.2 |
| Linear in MET | -6.3 | $\Delta$AIC = +7.5 |

**The excess is approximately constant ($\delta R \approx 0.41$ cm)**, not growing with $\sqrt{E}$. This is a **normalization anomaly** (data has uniformly more large-$R$ events), not a scaling anomaly.

#### 4.5.6 Test 6: Selection Bias

- $\rho(\text{dlenSig}, \text{MET}) = +0.034$ — negligible direct bias
- **Conditional test** in narrow dlenSig bands shows correlation **persists**:

| dlenSig band | N | $\rho(\sqrt{\text{MET}}, R_{\text{cav}})$ |
|-------------|---|----------------------------------------|
| [25, 35) | 48,152 | +0.095 |
| [35, 45) | 69,302 | +0.101 |
| [45, 55) | 48,200 | +0.121 |
| [55, 75) | 61,583 | +0.125 |
| [75, 100) | 42,007 | +0.114 |

The correlation is **NOT a selection artifact** — it persists at consistent magnitude within narrow dlenSig bands. The dlenSig cut selects a cleaner population but does not create the correlation.

#### 4.5.7 Test 7: Partial Correlation (Controlling for Kinematics)

| Correlation type | Data | MC | Excess |
|-----------------|------|-----|--------|
| Raw $\rho(\sqrt{\text{MET}}, R_{\text{cav}})$ | +0.109 | +0.020 | +0.089 |
| Partial (OLS, controlling sv_mass + bjet + ntracks) | +0.075 | +0.040 | +0.035 |
| Partial (rank, controlling sv_mass only) | **+0.103** | — | — |

The correlation **SURVIVES** kinematic control:
- Rank-based partial correlation (removing sv_mass effect): only 5% reduction (+0.109 $\to$ +0.103)
- OLS-based partial (removing sv_mass + bjet + ntracks): 31% reduction (+0.109 $\to$ +0.075)
- Excess partial correlation: +0.035 (down from +0.089)

**The MET-$R_{\text{cav}}$ correlation is not purely kinematic.** After removing the variance explained by SV mass (which determines hadron lifetime/species), a significant residual correlation persists.

#### 4.5.8 Test 8: Kinematic Model

B meson kinematics predict $d \sim (p_T/m) \cdot c\tau \propto \text{MET}$ (linear, $\beta = 1.0$). The observed $\beta \approx 0.10$ is far below this, indicating that the SV **population changes** with MET — at higher MET, the mix of hadron species shifts (B-jet fraction drops from 57% to 37%), and detector acceptance saturates the observable.

#### 4.5.9 Reinvestigation Summary

| Test | Result | Verdict |
|------|--------|---------|
| 1. $R^2$ vs MET | All $R^2_{\text{fit}} < 0.01$ | WEAK (inconclusive) |
| 2. Excess scaling | Failed (MC statistics) | INCONCLUSIVE |
| 3. Width scaling | $\gamma = 0.17 \neq 0.5$ | AGAINST 0.5 |
| 4. AIC forced $\beta=0.5$ | $\Delta$AIC = 4.0 | WEAKLY DISFAVORED |
| 5. $\delta R$ profile | Constant best model | FLAT (no $\sqrt{E}$ growth) |
| 6. Selection bias | Persists in narrow bands | NOT an artifact |
| 7. Partial correlation | **Survives** kinematic control | NON-KINEMATIC |
| 8. Kinematic model | $\beta_{\text{kin}} = 1.0 \gg 0.10$ | POPULATION SHIFT |

**Key revisions to prior conclusions:**

1. **$\beta = 0.5$ is weakly disfavored, not definitively falsified.** The $\Delta$AIC drops from 6.8 (v1.1) to 4.0 (v1.2) with improved parameterization. The screened-$\sqrt{E}$ model provides a reasonable forced-$\beta=0.5$ fit.

2. **The correlation is real and non-kinematic.** It survives partial correlation control for SV mass, persists within narrow dlenSig bands, and is not a selection artifact.

3. **But the excess is constant, not growing.** The $\delta R$ profile is flat at ~0.4 cm across all MET bins — this is a normalization anomaly, not a scaling anomaly. FTD predicts the excess should grow as $\sqrt{E}$.

4. **Observable mismatch is the fundamental issue.** $R_{\text{cav}} = \max(\text{SV\_dxy})$ measures kinematic flight distance, not topological bubble radius. A proper test of FTD cavitation requires an observable that probes the vacuum structure, not hadron kinematics.

### 4.6 Mass Window Discrimination [DISTINGUISHES FTD FROM TTBAR]

If the excess were from missing ttbar MC, it should concentrate in the B-meson SV mass window (4-7 GeV). Instead:

| SV Mass Window | $\rho$(Data) | $\rho$(MC) | Excess |
|---------------|-------------|-----------|--------|
| Low (< 1.5 GeV) | +0.090 | +0.033 | **+0.057** |
| D-meson (1.5-2.5 GeV) | +0.102 | +0.070 | +0.032 |
| Intermediate (2.5-4 GeV) | +0.101 | +0.010 | **+0.091** |
| B-meson (4-7 GeV) | +0.102 | -0.001 | **+0.103** |
| Exotic (> 7 GeV) | +0.069 | -0.044 | **+0.113** |

**Removing B/D mass events does not diminish the signal:**
- All signal (dlenSig > 30): $\rho$ = +0.109
- After removing B-mass (4-7 GeV): $\rho$ = +0.109 **[UNCHANGED]**
- After removing B+D mass: $\rho$ = +0.104 **[STILL PRESENT]**

**Clean exotic sample** (no B-jet + dlenSig > 30 + SV mass < 4 GeV):
- 106,316 data events, $\rho$ = +0.051
- 2,461 MC events, $\rho$ = +0.027
- Excess = +0.023 (reduced but persists)

**Implication:** The excess is uniformly distributed across all SV mass scales. This is *inconsistent* with a pure ttbar explanation but *consistent* with a scale-free mechanism (like cavitation). **[CONJECTURE]**

### 4.7 KS Tests: Shape Differences

| MET Bin (GeV) | KS Statistic | p-value | Significant? |
|---------------|-------------|---------|-------------|
| 200-250 | 0.050 | 4.2e-07 | Yes |
| 250-300 | 0.089 | 2.0e-10 | Yes |
| 300-400 | 0.099 | 7.7e-10 | Yes |
| 400-600 | 0.107 | 8.1e-05 | Yes |
| 600-1000 | 0.124 | 0.15 | No (low statistics) |

The R_cav shape difference is highly significant in every well-populated MET bin. **[EMERGENT]**

### 4.8 Deep Partial Correlation Investigation (v1.3 Update)

The partial correlation $\rho_{\text{partial}} = +0.103$ (only 5% reduction from raw) was subjected to 10 deeper tests. Script: `simulations/ftd_cern_partial_correlation_deep.py`.

#### 4.8.1 Iterative Control Waterfall

Adding kinematic controls one at a time (rank-based):

| Controls | $\rho_{\text{partial}}$ | Reduction |
|----------|-------------------------|-----------|
| Raw | +0.109 | 0% |
| +sv_mass | +0.103 | 5.5% |
| +bjet | +0.087 | 20.3% |
| +ntracks | +0.085 | 21.3% |
| +dlsig | +0.081 | 25.5% |
| +sv_mass$^2$ | +0.081 | 25.6% |

B-jet tag is the most impactful single control (15% additional reduction). All controls combined reduce correlation by only 26%.

#### 4.8.2 Nonlinear Residualization

| Method | $\rho$(sqrt(MET), residuals) |
|--------|------------------------------|
| Polynomial (deg 2) | +0.075 |
| Polynomial (deg 3) | +0.074 |

Even cubic polynomial regression of R_cav on (sv_mass, bjet, ntracks, dlsig) leaves 68% of the correlation intact.

#### 4.8.3 B-Fraction Reweighting

B-fraction drops from 58% (MET~200) to 46% (MET~700). After reweighting to hold B-fraction constant at 68% across all MET bins: $\rho_{\text{reweighted}} = +0.101 \pm 0.002$ (only 7.3% reduction). **The correlation is NOT a composition artifact.**

#### 4.8.4 SV Mass Stratification

| Mass band | $\rho$ | Events |
|-----------|--------|--------|
| [0, 1) GeV (K/light) | +0.059 | 94,684 |
| [1, 2.5) GeV (D meson) | +0.107 | 156,841 |
| [2.5, 4) GeV (transition) | +0.101 | 68,332 |
| [4, 7) GeV (B meson) | +0.102 | 24,133 |
| [7+) GeV (exotic) | +0.069 | 10,017 |

Correlation is present in ALL mass bands ($\rho = +0.06$ to $+0.11$). Strongest in D-meson and transition regions, NOT concentrated in B-meson window.

#### 4.8.5 Track Multiplicity Stratification

| ntracks | $\rho$ | Events |
|---------|--------|--------|
| 2 | +0.093 | 115,627 |
| 3 | +0.116 | 87,107 |
| 4 | +0.107 | 64,207 |
| $\geq$5 | +0.094 | 87,078 |

Universal across all track multiplicities. Not topology-dependent.

#### 4.8.6 MET-Dependent Partial Correlation

| MET bin | Raw $\rho$ | Partial $\rho$ |
|---------|------------|----------------|
| [200, 250) | +0.015 | +0.016 |
| [250, 325) | +0.027 | +0.028 |
| [325, 450) | +0.025 | +0.024 |
| [450, 1200) | +0.007 | +0.006 |

**Within individual MET bins, the correlation is much weaker** ($\rho \sim 0.02$). This reveals that the global $\rho = +0.109$ is predominantly a **between-bin** effect: median R_cav increases across MET bins, creating the overall correlation. This is an important structural insight.

#### 4.8.7 Permutation Null Distribution

| Statistic | Value |
|-----------|-------|
| Observed $\rho_{\text{partial}}$ (50K subsample) | +0.111 |
| Null mean | +0.000 |
| Null std | 0.004 |
| Z-score | 24.9 |
| p-value | < 10$^{-6}$ |

The partial correlation is **astronomically significant** under permutation. Not a statistical fluctuation.

#### 4.8.8 MC Comparison

| Metric | Data | MC | Excess |
|--------|------|----|--------|
| Raw $\rho$ | +0.109 | +0.020 | +0.089 |
| Partial (sv_mass) | +0.103 | +0.043 | +0.060 |
| Partial (sv_mass+bjet) | +0.087 | +0.046 | +0.041 |

MC shows **some** residual partial correlation (+0.043), but data exceeds MC by a factor of 2.4x. The excess $\Delta\rho = +0.060$ is genuine — not reproduced by available SM processes. Missing ttbar remains the primary caveat.

#### 4.8.9 R_cav Range Dependence

| Range | $\rho$ | Events |
|-------|--------|--------|
| R < 1 cm | -0.001 | 29,540 |
| [1, 4.4) cm | +0.063 | 207,919 |
| [4.4, 10.2) cm | +0.043 | 93,043 |
| R > 10.2 cm | -0.031 | 23,497 |

Correlation concentrated in **R = 1-10 cm range** (between CMS pixel layers). Zero correlation at short distances (R<1) and slightly negative at long distances (R>10). This is consistent with detector geometry effects (pixel layer boundaries at 4.4, 7.3, 10.2 cm).

#### 4.8.10 Partial Distance Correlation

| Metric | Value |
|--------|-------|
| dcor(sqrt(MET), R_cav) | 0.096 |
| dcor partial (linear) | 0.097 |
| dcor partial (poly-3) | 0.087 |
| Permutation p-value | < 0.002 |

Distance correlation captures ALL nonlinear dependencies. The association survives arbitrary nonlinear control for SV mass.

#### 4.8.11 Deep Investigation Summary

| Test | Result | Verdict |
|------|--------|---------|
| T1: Iterative controls | 26% reduction max | SURVIVES |
| T2: Nonlinear residualization | 32% reduction | AMBIGUOUS |
| T3: B-fraction reweighting | 7% reduction | SURVIVES |
| T4: SV mass strata | Uniform across bands | SURVIVES |
| T5: ntracks strata | Universal | SURVIVES |
| T6: MET-dependent partial | Within-bin $\rho \sim 0.02$ | **REVEALS STRUCTURE** |
| T7: Permutation | Z = 24.9 | SURVIVES |
| T8: MC comparison | Data 2.4x MC | GENUINE EXCESS |
| T9: R_cav range | 1-10 cm only | DETECTOR GEOMETRY |
| T10: Distance correlation | Survives poly-3 | SURVIVES |

**Overall: 7 SURVIVES, 2 STRUCTURAL INSIGHT, 1 AMBIGUOUS.**

**Key findings:**
1. The partial correlation is **not a composition artifact** (B-fraction reweighting, mass strata)
2. It is **not a simple nonlinear kinematic effect** (polynomial deg-3 residualization)
3. It **survives all controls** combined (26% reduction, 74% intact)
4. It is concentrated in the **1-10 cm displacement range** (detector geometry region)
5. Within individual MET bins, the correlation is **weak** ($\rho \sim 0.02$) — the global correlation is predominantly a between-bin level shift
6. MC shows partial correlation (+0.043) but data exceeds MC by 2.4x

### 4.9 Energy Threshold Analysis (v1.4 Update)

The cavitation-consciousness bridge (DERIV_CAVITATION_CONSCIOUSNESS_BRIDGE.md) derives the scaling exponent $\beta = 1/2$ but leaves the critical energy flux $\varepsilon_{\text{crit}}$ unspecified. A separate analysis (DERIV_CAVITATION_THRESHOLD.md) derives $\varepsilon_{\text{crit}}$ from FTD's algebraic structure.

**Key results:**

| Candidate for $\varepsilon_{\text{crit}}$ | Value (GeV/m$^2$) | $R_{\text{cav}}$ at 500 GeV | Gap to 1 cm |
|------------------------------------------|-------------------|----------------------------|-------------|
| Planck density | $4.7 \times 10^{88}$ | $3 \times 10^{-44}$ m | $10^{44}$ |
| $K_C$ in Planck units | $1.7 \times 10^{89}$ | $1.5 \times 10^{-44}$ m | $10^{44}$ |
| $K_C$ scaled (nuclear area) | $9 \times 10^{25}$ | 660 fm | $10^{10}$ |
| QCD deconfinement | $10^{30}$ | 6.3 fm | $10^{12}$ |
| Electroweak | $9.4 \times 10^{40}$ | $2 \times 10^{-20}$ m | $10^{17}$ |
| **Required for 1 cm** | **$4 \times 10^{5}$** | **1 cm** | **1** |

**The minimum energy for a single-voxel cavitation bubble** (the smallest possible on the FTD lattice) is $E_{\min} = 4\pi K_C \approx 45$ Planck energies $\approx 5.5 \times 10^{20}$ GeV. The LHC delivers $\sim 10^4$ GeV -- a factor of $4 \times 10^{16}$ too low.

**Conclusion:** FTD topological cavitation **cannot occur at LHC energies**. The partial correlation $\rho = +0.103$ is genuine but requires a different explanation.

See DERIV_CAVITATION_THRESHOLD.md for the full derivation and `simulations/cavitation_threshold_verification.py` (10/10 checks passed) for numerical verification.

**Multi-scale hierarchy (v1.5 update):** A comprehensive analysis (DERIV_CAVITATION_HIERARCHY.md) maps **eight** distinct cavitation scales in FTD, from substrate (Planck energy) to molecular bonds (0.2 eV). The key finding: QCD vacuum cavitation (deconfinement) is real and experimentally confirmed at RHIC/LHC, but produces only fm-scale bubbles -- 12 orders of magnitude below cm-scale displacements. No FTD-specific scale produces CMS-visible bubbles. See `simulations/cavitation_hierarchy_verification.py` (10/10 checks passed).

---

## 5. Honest Assessment (Updated v1.4)

### 5.1 What Supports FTD

| Finding | Strength | Status |
|---------|----------|--------|
| Excess correlation is real (65.6$\sigma$) | Strong | **[EMERGENT]** |
| Not reproduced by available SM MC | Moderate | **[CONJECTURE]** (missing ttbar) |
| Grows with dlenSig purity cuts | Strong | **[EMERGENT]** |
| Direction correct (positive $\rho$) | Weak | Generic prediction |
| Uniform across SV mass scales | Moderate | Against pure ttbar |
| Survives all anti-B-jet cuts | Moderate | Against pure ttbar |
| **Survives partial correlation control** | **Strong** | Only 5% reduction after removing SV mass effect |
| **Not a selection artifact** | **Strong** | Persists within narrow dlenSig bands |
| Tail fraction P(R>3cm) grows 2.3x faster in data than MC | Moderate | **[EMERGENT]** |
| **Survives 10-test deep investigation (v1.3)** | **Strong** | 7/10 tests confirm genuine non-kinematic signal |
| **Not a composition artifact** | **Strong** | B-fraction reweighting: only 7% reduction |
| **Universal across SV topologies** | **Strong** | Present in all ntracks bins and mass bands |
| **Survives nonlinear control (dcor)** | **Strong** | Distance correlation survives poly-3 residualization |
| **Data exceeds MC by 2.4x** | **Strong** | Partial $\rho$: data +0.103, MC +0.043 |

### 5.2 What Does NOT Support FTD

| Finding | Severity | Status |
|---------|----------|--------|
| Scaling exponent $\beta \approx 0.10 \neq 0.5$ | Moderate | Raw scaling wrong, but excess may differ |
| Forced $\beta = 0.5$ weakly disfavored ($\Delta$AIC = 4.0) | Moderate | Not definitively excluded ($2 < \Delta\text{AIC} < 6$) |
| Excess $\delta R$ is **constant** (~0.4 cm), not $\sqrt{E}$ | **Critical** | Flat excess profile contradicts $\sqrt{E}$ growth |
| Width scaling $\gamma = 0.17 \neq 0.5$ | Moderate | Distribution doesn't broaden as predicted |
| $R_{\max} \approx 4.55$ cm coincides with pixel L1 (4.4 cm) | **Critical** | Saturation likely detector artifact |
| Observable mismatch: SV_dxy is kinematic, not topological | **Critical** | Testing the wrong quantity |
| Missing ttbar MC | **Critical** | Major systematic uncertainty |
| **Within-bin $\rho \sim 0.02$ (v1.3)** | **Structural** | Global $\rho$ is mainly between-bin level shift, not within-bin scaling |
| **Correlation zero at R < 1 cm and R > 10 cm (v1.3)** | **Moderate** | Concentrated in detector geometry region (1-10 cm) |
| **MC shows partial $\rho = +0.043$ (v1.3)** | **Moderate** | SM kinematics explain ~42% of partial correlation |
| MC statistics limited (32K vs 1.5M) | **Critical** | Excess scaling test (Test 2) failed entirely |
| Global correlation near zero | Moderate | Only visible after cuts |
| **Energy threshold gap $\sim 10^{16}$ (v1.4)** | **FATAL** | FTD cavitation requires GUT/Planck energy; LHC is $4 \times 10^{16}$x too weak |
| **$R_{\text{cav}}$ at LHC is sub-Planck (v1.4)** | **FATAL** | Cavitation bubble would be smaller than one lattice site -- logical impossibility |
| **No loophole mechanism identified (v1.4)** | **Critical** | No known RG running, tunneling, or collective effect bridges the gap |
| **Multi-scale hierarchy confirms: no FTD scale matches (v1.5)** | **FATAL** | 8 cavitation scales mapped; QCD deconfinement gives fm-scale bubbles (gap: $10^{12}$); molecular scale is standard chemistry |

### 5.3 Balance of Evidence (Updated v1.5)

The 8-test reinvestigation significantly revises the assessment from v1.1:

**Revisions from v1.1:**
1. **$\beta = 0.5$ is weakly disfavored, not definitively falsified.** The $\Delta$AIC drops from 6.8 (v1.1, one parameterization) to 4.0 (v1.2, three parameterizations with multistart). The screened model $R = A\sqrt{E}\exp(-E/E_{\text{screen}})$ with $E_{\text{screen}} = 1063$ GeV is a physically reasonable $\beta=0.5$ fit.
2. **The correlation is genuinely non-kinematic.** Partial correlation controlling for SV mass reduces the correlation by only 5% (rank-based). This was never tested before and is a significant finding.
3. **The correlation is not a selection artifact.** It persists at consistent magnitude ($\rho \approx 0.10$–$0.13$) within narrow dlenSig bands, ruling out artificial correlation from the dlenSig cut.

**Persistent problems:**
1. **The excess is flat, not growing.** The displacement difference $\delta R \approx 0.41$ cm is approximately constant across all MET bins. FTD predicts the excess should grow as $\sqrt{E}$. This is the strongest evidence against the specific $\beta = 0.5$ prediction.
2. **Observable mismatch.** $R_{\text{cav}} = \max(\text{SV\_dxy})$ measures hadron flight distance — a kinematic quantity dominated by the boost factor $\gamma\beta$ — not the topological bubble radius that FTD predicts. Even if cavitation exists, it would not manifest as a flight distance change unless it altered the production or kinematics of long-lived hadrons.
3. **Detector saturation.** The $R_{\max} \approx 4.55$ cm coincides with CMS pixel L1 (4.4 cm), and the R_cav distribution peaks align with all three pixel layers.

**Against pure ttbar explanation:**
1. The excess survives all anti-B-jet and anti-B-mass cuts (excess reduces from +0.089 to +0.054).
2. The excess is uniform across SV mass windows — ttbar would concentrate at 4-7 GeV.
3. The B-jet fraction *decreases* at higher MET (57% → 37%), yet the data/MC excess ratio *increases*.

**Most likely explanation:** A combination of:
1. **Missing ttbar MC** (~40-60% of excess, explaining the B-jet correlated portion)
2. **Detector geometry effects** (tracking efficiency changes near pixel layers)
3. **Other missing MC** (single-top, diboson, ~10-20%)
4. **Pileup effects** (not corrected)
5. **A small residual anomaly** that cannot be excluded with current data — the partial correlation surviving kinematic control is noteworthy

**The excess is a normalization anomaly** (uniformly more large-$R$ events in data) rather than a scaling anomaly ($\beta$ is similar in data and MC). The genuine excess correlation that survives kinematic control ($\rho_{\text{excess,partial}} \approx +0.035$) may represent missing SM backgrounds, detector effects, or an interesting anomaly requiring further investigation.

---

## 6. Falsification Assessment (Updated v1.2)

| FTD Prediction | Observed | Status |
|----------------|----------|--------|
| Positive MET-R_cav correlation | Yes (+0.109 at dlenSig > 30) | **Consistent** |
| Correlation survives kinematic control | Yes ($\rho_{\text{partial}} = +0.103$) | **Consistent** |
| Not a selection artifact | Confirmed (persists within dlenSig bands) | **Consistent** |
| $R \sim \sqrt{E}$ scaling ($\beta = 0.5$) | Weakly disfavored ($\Delta$AIC = 4.0) | **Weakly disfavored** |
| Excess grows with $\sqrt{E}$ | No ($\delta R$ is constant ~0.4 cm) | **Inconsistent** |
| Excess over SM backgrounds | Yes (in available MC) | **Inconclusive** (missing ttbar) |
| Scale-free (all mass windows) | Yes (uniform distribution) | **Consistent** |
| Observable = bubble radius | **No** (SV_dxy is kinematic flight distance) | **Observable mismatch** |

**The scaling exponent is weakly disfavored but not definitively falsified.** With improved parameterization (three saturation models, multistart optimization), forced $\beta = 0.5$ achieves $\Delta$AIC = 4.0 (down from 6.8 in v1.1). This places it in the "weakly disfavored" range ($2 < \Delta\text{AIC} < 6$), not "strongly excluded."

**However, the excess displacement profile is flat** — $\delta R \approx 0.4$ cm constant across all MET bins, with the constant model strongly preferred over $\sqrt{\text{MET}}$-linear ($\Delta$AIC = 7.2). This is the strongest evidence against the specific FTD prediction.

**The most important finding:** The observable ($\max(\text{SV\_dxy})$) does not test what FTD predicts. A vacuum bubble radius would not manifest as a change in B-meson flight distance unless the bubble altered hadron production or kinematics — a connection that has not been established theoretically.

---

## 7. What Would Be Needed for Definitive Test

1. **TTbar MC**: TTTo2L2Nu NanoAODSIM (not available on CERN Open Data)
2. **Full CMS MC stack**: DY+jets, single-top, diboson
3. **Detector simulation**: Proper CMS response modeling (CMSSW) — critical for understanding pixel geometry effects
4. **Pileup corrections**: Not applied in our analysis
5. **A proper FTD observable**: The current observable ($\max(\text{SV\_dxy})$) is kinematic, not topological. What observable would actually test for vacuum bubbles? Possibilities:
   - Event-level multiplicity changes (bubble would produce extra particles)
   - Angular correlations (spherical bubble $\to$ isotropic emission)
   - Energy spectrum of soft particles near the bubble boundary
   - None of these are available in NanoAOD format
6. **Larger MC statistics**: The excess scaling test (Test 2) failed entirely due to only 7,302 MC signal events. Need $O(100\text{K})$ MC in signal region

---

## 8. Analysis Scripts

All analysis code is in `simulations/`:

| Script | Purpose |
|--------|---------|
| `ftd_cern_exploration.py` | Initial 12-panel exploration |
| `ftd_cern_deep_analysis.py` | Background-subtracted tail analysis |
| `ftd_cern_bveto_analysis.py` | B-veto + SV discrimination |
| `ftd_cern_mc_comparison.py` | Full MC comparison (WJets + ZJets + QCD) |
| `ftd_cern_residual_test.py` | Bootstrap CI + permutation + scaling tests |
| `ftd_cern_discriminant_test.py` | FTD vs ttbar mass-window discrimination |
| `ftd_cern_scaling_corrections.py` | Scaling correction models + AIC comparison + excess characterization |
| `ftd_cern_scaling_followup.py` | Detector geometry investigation + high-energy reversal + ttbar tests |
| `ftd_cern_reinvestigation.py` | 8-test reinvestigation: R^2, excess scaling, width, improved AIC, deltaR, selection bias, partial correlation, kinematic model |
| `ftd_cern_partial_correlation_deep.py` | 10-test deep investigation: iterative control, nonlinear residualization, B-fraction reweighting, mass/ntracks/MET strata, permutation null, MC comparison, R_cav range, distance correlation |

See `simulations/ANALYSIS_CERN_CAVITATION_SUMMARY.md` for detailed results tables.

---

## 9. Implications for FTD

### 9.1 For the Prediction Catalog (SPEC_NOVEL_PREDICTIONS.md)

The cavitation test should be added to the prediction catalog with status:
- **Direction:** Confirmed (positive non-kinematic correlation in long-lived candidates)
- **Magnitude:** Weakly disfavored ($\Delta$AIC = 4.0); excess is constant, not $\sqrt{E}$
- **Overall:** [CONJECTURE] — anomaly present, not fully explained by available MC, observable mismatch complicates interpretation

### 9.2 For the Epistemic Audit (AUDIT_EPISTEMIC_AUDIT.md)

This analysis represents the first attempt at **external empirical contact**. It should be classified as:
- **NOT a genuine derivation** (no prediction from first principles)
- **NOT a parametric insertion** (not using standard formulas)
- **An empirical observation** — a new epistemic category for FTD
- **An inconclusive test** — the observable does not directly probe the predicted quantity

### 9.3 Updated Assessment (v1.2)

The 8-test reinvestigation provides a nuanced picture that is less definitively negative than v1.1 concluded:

**What was revised:**
1. $\beta = 0.5$ is **weakly disfavored** ($\Delta$AIC = 4.0), not "definitively falsified" ($\Delta$AIC = 6.8 in v1.1). The improvement comes from better parameterization with three saturation models and multistart optimization.
2. The correlation **survives kinematic control** — only 5% reduction after removing the effect of SV mass via rank-based partial correlation. This is a genuinely unexpected finding.
3. The correlation is **not a selection artifact** — it persists at $\rho \approx 0.10$–$0.13$ within narrow dlenSig bands.
4. The observable ($\max(\text{SV\_dxy})$) is **kinematic** (hadron flight distance), not topological (bubble radius). This means the entire analysis tests a proxy, not the predicted quantity.

**What the reinvestigation confirmed:**
1. The excess displacement $\delta R$ is **constant** (~0.4 cm) across all MET bins — a normalization anomaly, not scaling. This contradicts $\sqrt{E}$ growth.
2. The width scaling $\gamma = 0.17$ does not match 0.5.
3. The SV population shifts with MET (B-jet fraction drops from 57% to 37%), dominating the observed weak scaling.
4. Kinematic models predict $\beta = 1.0$ (linear), while observed $\beta \approx 0.10$ — the discrepancy is due to detector acceptance saturation and population shifts, not new physics.

**What remains genuinely interesting:**
1. The **partial correlation surviving kinematic control** ($\rho_{\text{partial}} = +0.103$, $\rho_{\text{excess,partial}} = +0.035$) means something beyond SV mass (species/lifetime) drives the MET-$R_{\text{cav}}$ connection in data but not MC.
2. The excess is **uniform across all SV mass windows** — not concentrated in B-meson range.
3. The excess **persists** even after removing all B/D-mass events ($\rho = +0.104$).

### 9.4 For Future Work

1. **Identify a proper FTD observable** — this is the most important theoretical task. What CMS-level observable would probe vacuum bubble formation? The current observable (SV_dxy) does not test the prediction.
2. **Obtain ttbar MC** — resolve the dominant systematic uncertainty
3. **Explain the partial correlation result** — why does a non-kinematic MET-$R_{\text{cav}}$ correlation exist in data but not MC? Possible explanations:
   - Missing ttbar creates a non-trivial MET-displacement correlation beyond simple SV mass dependence
   - Pileup-induced tracking effects correlate with MET
   - A genuine anomaly in displaced-vertex production at high MET
4. **Run full CMS detector simulation** — essential for understanding pixel geometry effects
5. **Test with upgraded detector** — CMS Phase-1 pixel (4 layers, innermost at 2.9 cm) would shift detector-induced saturation

---

*Document created: 2026-02-28*
*Updated: 2026-02-27 (v1.2 — 8-test reinvestigation)*
*Framework: Foundational Ternary Dynamics v5.27-bell*
*Epistemic status: [CONJECTURE] — observable mismatch identified, weakly disfavored but not falsified*
