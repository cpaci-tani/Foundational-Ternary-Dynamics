# Derivation: The Cavitation Hierarchy of Discrete Spacetime

## Multi-Scale Phase Transitions from Substrate to Cosmos

**Document ID:** DERIV_CAVITATION_HIERARCHY
**Category:** 10. Empirical Validation & Observations
**Epistemic Status:** [SELECTION] -- hierarchy derived from FTD + standard physics; QCD level [EXTERNAL/CONFIRMED]; EW level [SELECTION]; substrate level [THEOREM given premises]; CERN link [DISFAVORED]
**Date:** 2026-03-01
**Framework:** Foundational Ternary Dynamics v5.27-bell

---

## 1. Abstract

Water cavitation does not break water molecules. It breaks the **hydrogen bonds between them** -- a binding energy of ~0.2 eV, not the ~4.8 eV O-H covalent bond or the ~938 MeV proton mass. The cavitation threshold is set by the weakest link in the structure, not the deepest.

This insight transforms FTD's cavitation analysis. The previous threshold derivation (DERIV_CAVITATION_THRESHOLD.md) showed that **substrate** cavitation requires Planck-scale energy (~5 x 10^20 GeV) -- impossible at LHC by a factor of 10^16. But FTD's lattice supports multiple stacked "media," each with its own binding energy. Like Russian dolls, you can break an outer shell without disturbing the inner ones.

This document maps **eight distinct cavitation scales** in FTD, from the lattice substrate (Planck energy) to molecular bonds (0.2 eV) -- spanning 28 orders of magnitude in binding energy and 40+ orders of magnitude in bubble size. The key finding: **QCD vacuum cavitation (deconfinement) is real, experimentally confirmed at RHIC/LHC, and the closest analog to FTD's theoretical prediction.** But even QCD cavitation produces only fm-scale bubbles -- 13 orders of magnitude below the cm-scale displacements in the CERN data.

### Dependencies

| Depends On | Status | Document |
|------------|--------|----------|
| Substrate threshold analysis | [THEOREM given premises] | DERIV_CAVITATION_THRESHOLD.md |
| beta = 1/2 scaling | [SELECTION] | DERIV_CAVITATION_CONSCIOUSNESS_BRIDGE.md |
| EW phase transition | [THEOREM + SELECTION] | DERIV_HIGGS_FROM_MANIFESTATION.md |
| SU(3) confinement | [THEOREM + SELECTION] | DERIV_LATTICE_SU3_GAUGE.md |
| Lambda_QCD derivation | [SELECTION] | DERIV_LAMBDA_QCD_DERIVATION.md |
| Ontic derivation chain | [THEOREM] | engine/include/ftd/ontic.h |
| CERN Open Data results | [DISFAVORED] | EMPIRICAL_CERN_CAVITATION.md v1.4 |

---

## 2. The Water Analogy: What Actually Breaks?

### 2.1 The Principle

Every material is a hierarchy of bonds. Cavitation breaks the **weakest** one -- the one with the lowest binding energy per unit area. The deeper bonds survive intact.

### 2.2 Physical Cavitation Across Scales

| Medium | What Breaks | Binding Energy | How It Happens |
|--------|-------------|----------------|----------------|
| Water (liquid) | Hydrogen bonds | ~0.2 eV | Propeller, ultrasound, snapping shrimp |
| Crystal (solid) | Lattice bonds | ~1-5 eV | Fracture, shock wave |
| Atom | Electron binding | ~13.6 eV (H) | Ionization, lightning, plasma |
| Nucleus | Nuclear binding | ~8 MeV/nucleon | Fission, spallation, cosmic rays |
| QCD vacuum | Quark-gluon condensate | ~170 MeV | Heavy-ion collisions (RHIC/LHC) |
| Higgs condensate | Electroweak symmetry | ~246 GeV | Early universe (T ~ 10^15 K) |
| Spacetime (FTD) | Lattice substrate | ~10^19 GeV | Big Bang only |

### 2.3 The Nesting Principle

Each medium sits ON TOP of deeper media. Water cavitation does not ionize the water molecules. Nuclear fission does not deconfine the quarks. QCD deconfinement does not break the lattice substrate. **Each level can cavitate independently.**

The threshold for each is set by its own binding energy and characteristic length scale:

$$\varepsilon_{\text{crit}} = \frac{E_{\text{binding}}}{L_{\text{char}}^2}$$

where L_char is the characteristic size of the structures being disrupted.

---

## 3. FTD's Layered Media

### 3.1 Mapping Ontic Layers to Cavitation Media

The ontic derivation chain (engine/include/ftd/ontic.h) defines a hierarchy of energy scales. Each scale represents a different "medium" that could undergo its own cavitation:

| FTD Layer | Medium | Binding Energy | Length Scale | What Cavitation Means |
|-----------|--------|----------------|-------------|----------------------|
| Substrate | Lattice itself | E_P ~ 10^19 GeV | l_P ~ 10^-35 m | Destroy spacetime |
| Layer 8 (K_C) | Consciousness domain | ~3.60 E_P | ~l_P | Domain A-to-B transition |
| Layer 6b (v) | Higgs condensate | 246 GeV | ~8 x 10^-19 m | Restore EW symmetry |
| Layer 5b (Lambda) | QCD vacuum | 170 MeV | ~1 fm | Deconfinement (QGP) |
| Layer 6 (K_B) | Manifestation field | 0.511 MeV | ~386 fm | Evaporation zone |
| (Standard) | Nuclear matter | 8 MeV | ~1 fm | Fission/spallation |
| (Standard) | Atomic electrons | 13.6 eV | 0.53 A | Ionization |
| (Standard) | Molecular bonds | 0.2 eV | 2.8 A | Dissociation |

### 3.2 FTD-Specific vs Standard Physics

The top five layers (Substrate through K_B) involve FTD-specific scales derived from the ontic chain. The bottom three (Nuclear, Atomic, Molecular) are standard physics that FTD does not claim to modify. The genuine FTD contribution is connecting all eight levels through a single derivation chain rooted in D = 3 and the lemniscatic constant G*.

---

## 4. Bubble Sizes at LHC Energy

### 4.1 The Universal Formula

For any spherical-threshold cavitation in D = 3:

$$R_{\text{cav}} = \sqrt{\frac{E}{4\pi\,\varepsilon_{\text{crit}}}}$$

where E is the deposited energy and epsilon_crit = E_binding / L_char^2.

### 4.2 Results for All Eight Scales

At E = 500 GeV (typical high-MET CMS event):

| Layer | epsilon_crit (GeV/m^2) | R_cav | Physical Scale | CMS Visible? |
|-------|------------------------|-------|---------------|--------------|
| Substrate | 4.7 x 10^88 | 2.9 x 10^-44 m | Sub-Planck | No |
| K_C | 1.7 x 10^89 | 1.5 x 10^-44 m | Sub-Planck | No |
| EW (Higgs) | 3.8 x 10^38 | 3.2 x 10^-19 m | Sub-attometer | No |
| QCD | 1.3 x 10^29 | ~18 fm | Nuclear | No |
| K_B | 3.4 x 10^21 | ~0.1 nm | Sub-micrometer | No |
| Nuclear | 8.0 x 10^27 | ~0.07 fm | Sub-nuclear | No |
| Atomic | 4.9 x 10^12 | ~3 um | Micrometer | No |
| Molecular | 2.6 x 10^9 | ~125 um | Hair-width | Marginal |

### 4.3 Key Observation

**No FTD-specific scale produces bubbles visible at CMS** (resolution ~100 um). The molecular scale (~125 um) is marginally above CMS resolution but represents standard chemistry, not a novel FTD prediction. The most physically interesting scale (QCD deconfinement) gives fm-scale bubbles -- real, but 12 orders of magnitude below cm-scale displaced vertices.

---

## 5. QCD Vacuum Cavitation IS Real

### 5.1 Experimental Confirmation

QCD vacuum cavitation is not speculative -- it is one of the most studied phenomena in modern nuclear physics:

- **RHIC (2000-present):** Au+Au collisions at sqrt(s_NN) = 200 GeV produce quark-gluon plasma (QGP). The 2005 "perfect liquid" discovery confirmed that the QCD vacuum deconfines under sufficient energy density.
- **LHC (2010-present):** Pb+Pb collisions at sqrt(s_NN) = 5.02 TeV produce QGP at higher temperatures and larger volumes.
- **pp collisions:** Recent ALICE and CMS results show signatures of collective behavior even in high-multiplicity proton-proton collisions, suggesting micro-QGP droplet formation.

### 5.2 QGP Properties

| Property | Value | Source |
|----------|-------|--------|
| Critical temperature | T_c ~ 155-170 MeV | Lattice QCD |
| Energy density threshold | ~1 GeV/fm^3 | Lattice QCD |
| Bubble size (Pb+Pb) | ~5-10 fm | Experiment |
| Lifetime | ~5-10 fm/c (~10^-23 s) | Experiment |
| Temperature achieved | 300-600 MeV | Experiment |

### 5.3 FTD Interpretation [SELECTION]

In FTD terms, QGP formation corresponds to thermal randomization of the flux-axis color alignment (DERIV_LATTICE_SU3_GAUGE.md). The Wilson loop area law (confinement) transitions to a perimeter law (deconfinement) -- the color flux tubes between quarks "melt." This IS cavitation of the QCD vacuum condensate, in the precise sense that the binding medium has been disrupted and a new phase emerges.

FTD's contribution here is **postdictive, not predictive** -- QGP was discovered before FTD existed. However, FTD connects this phenomenon to the same ontic chain that produces alpha, N_c, and Lambda_QCD.

---

## 6. The fm-to-cm Gap

### 6.1 The Problem

The CERN anomaly shows cm-scale displaced vertices (rho = +0.103 partial correlation). Even the most favorable FTD cavitation scale (QCD deconfinement) produces only fm-scale bubbles at LHC energies. The gap: 10^13.

### 6.2 Three Candidate Bridging Mechanisms

#### 6.2.1 Direct Bubble Size

Can any cavitation scale produce cm-scale bubbles at LHC energy?

**Answer: No.** Section 4 shows that all FTD-specific scales give R_cav << 1 cm. Even molecular-scale cavitation gives only ~20 um. To get R = 1 cm requires epsilon_crit ~ 4 x 10^5 GeV/m^2, which is 10^12 times below the QCD scale and corresponds to no known physical medium.

#### 6.2.2 Cavitation -> Hadronization -> Flight Distance

The bubble itself need not be cm-scale. If QGP formation modifies the hadronization process, the RESULTING hadrons might have altered lifetimes or kinematics, producing different flight distances (which ARE cm-scale for B mesons with tau ~ 1.5 ps).

**Assessment:**
- (a) This chain is standard heavy-ion physics, not an FTD prediction
- (b) QGP effects on heavy-flavor hadronization are actively studied (D and B meson R_AA, v_2)
- (c) Effects are at the ~10-30% level in Pb+Pb, much smaller in pp
- (d) The CERN data is from pp collisions, where QGP formation is marginal at best
- (e) A 10-30% modification to B-meson kinematics would produce rho ~ 0.01-0.03, not rho ~ 0.10

**Verdict: Insufficient.** Even if pp micro-QGP exists, the effect on displaced vertices is too small to explain rho = +0.103.

#### 6.2.3 pp "Mini-QGP"

Recent results suggest collective behavior in high-multiplicity pp events at LHC. Could energy density in jet cores exceed T_c?

**Assessment:**
- Jet core energy density: E_jet / V_jet ~ (100 GeV) / (1 fm)^3 ~ 100 GeV/fm^3 >> T_c
- This suggests jets DO locally deconfine the QCD vacuum
- BUT: this happens in EVERY high-pT collision -- it is not special to the anomalous events
- The correlation with MET would require that HIGHER MET events have MORE QGP, which is plausible but unquantified
- This is an open question in standard QCD, not an FTD prediction

**Verdict: Possible in principle, but (a) not FTD-specific and (b) effect size likely too small.**

### 6.3 Gap Verdict

No known mechanism bridges the fm-to-cm gap within FTD. The CERN anomaly (partial correlation rho = +0.103) cannot be explained as cavitation at any FTD scale. The most likely explanations remain: missing ttbar MC, detector geometry effects, and residual kinematic correlations.

---

## 7. What FTD's Hierarchy Actually Predicts

### 7.1 The Genuine Contribution

FTD's distinctive claim is not that any individual scale is novel -- QCD deconfinement was known before FTD. The claim is that **all scales are connected through one derivation chain**:

$$D = 3 \xrightarrow{\text{ontic}} G^* \xrightarrow{\text{quadratic}} \alpha, N_c \xrightarrow{\text{integers}} b_3, N_{\text{eff}} \xrightarrow{\text{RG}} \Lambda_{\text{QCD}} \xrightarrow{\text{seesaw}} v_{\text{Higgs}} \xrightarrow{\text{hierarchy}} m_e \xrightarrow{\text{lattice}} E_P$$

The ratios between cavitation thresholds are determined by framework integers and powers of alpha:

| Ratio | Value | FTD Expression |
|-------|-------|----------------|
| E_P / v_Higgs | ~5 x 10^16 | 1/(sqrt(2*pi) * alpha^8) |
| v_Higgs / Lambda_QCD | ~1140 | Complex (RG running) |
| Lambda_QCD / m_e | ~420 | Complex (RG + seesaw) |
| E_P / m_e | ~2.4 x 10^22 | 1/(sqrt(2*pi) * (16/3) * alpha^11) |

### 7.2 Predictions by Scale

| Scale | FTD Prediction | Testable? |
|-------|---------------|-----------|
| Substrate (Planck) | Cavitation at E > 5 x 10^20 GeV | No (early universe only) |
| EW (Higgs) | First-order EW phase transition | Possibly (gravitational waves) |
| QCD | Deconfinement at T_c ~ 170 MeV | Yes -- CONFIRMED |
| K_B (manifestation) | Evaporation zone at ~0.5 MeV | FTD-specific (untested) |

### 7.3 Cosmological Implications

The hierarchy suggests that the early universe underwent MULTIPLE cavitation events as it cooled:

1. **Planck epoch** (t ~ 10^-43 s): Substrate cavitation -- spacetime itself crystallized
2. **GUT epoch** (t ~ 10^-36 s): Gauge symmetry breaking (if applicable)
3. **EW epoch** (t ~ 10^-12 s): Higgs condensation -- EW symmetry broke
4. **QCD epoch** (t ~ 10^-6 s): Confinement -- quarks bound into hadrons
5. **Nuclear epoch** (t ~ 1-300 s): Nucleosynthesis -- nuclei formed
6. **Recombination** (t ~ 380,000 yr): Atoms formed -- photons decoupled

Each transition is a phase change -- literally cavitation of one medium while deeper media remain intact.

---

## 8. Honest Epistemic Assessment

| Claim | Status | Reasoning |
|-------|--------|-----------|
| Multiple cavitation scales exist in FTD | [THEOREM] | Follows from energy scale hierarchy |
| QCD deconfinement is vacuum cavitation | [EXTERNAL/CONFIRMED] | Experimentally established (RHIC/LHC) |
| EW restoration is vacuum cavitation | [SELECTION] | Standard SM interpretation |
| Substrate cavitation at ~10^20 GeV | [THEOREM given premises] | From K_C and lattice structure |
| All scales connected via ontic chain | [SELECTION] | FTD's unique contribution |
| beta = 1/2 at any individual scale | [SELECTION] | Geometric, not FTD-specific |
| CERN anomaly = FTD cavitation | [DISFAVORED] | No scale matches cm-displacement |
| fm-to-cm bridging mechanism | [OPEN] | No known mechanism; standard QCD insufficient |
| Hierarchy ratios from framework integers | [SELECTION] | Approximate, not exact |

---

## 9. References

- **DERIV_CAVITATION_THRESHOLD.md** -- Substrate-level threshold analysis (predecessor document)
- **DERIV_CAVITATION_CONSCIOUSNESS_BRIDGE.md** -- beta = 1/2 from k = 1/2
- **EMPIRICAL_CERN_CAVITATION.md v1.4** -- CERN Open Data results
- **DERIV_HIGGS_FROM_MANIFESTATION.md** -- EW phase transition as manifestation transition
- **DERIV_LATTICE_SU3_GAUGE.md** -- SU(3) confinement mechanism in FTD
- **DERIV_LAMBDA_QCD_DERIVATION.md** -- Non-circular Lambda_QCD derivation chain
- **engine/include/ftd/ontic.h** -- Complete ontic derivation chain

---

*Document created: 2026-03-01*
*Epistemic status: [SELECTION] -- hierarchy derived; QCD confirmed; CERN link disfavored*
*Verification: simulations/cavitation_hierarchy_verification.py*
