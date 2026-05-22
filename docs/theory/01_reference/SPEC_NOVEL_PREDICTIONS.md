# FTD Novel Predictions Catalog

**Document Classification:** Prediction Specification
**Version:** 1.0
**Date:** February 25, 2026
**Status:** Complete catalog post-Waves 1-4
**Depends on:** SPEC_FTD_LAGRANGIAN.md, AUDIT_NOVEL_PREDICTIONS.md, AUDIT_EPISTEMIC_AUDIT.md, all Wave 1-4 derivation documents

---

## Abstract

This document catalogs every falsifiable prediction that Foundational Ternary Dynamics (FTD) makes which the Standard Model (SM) does not. The SM has 20 free parameters (19 in the electroweak/Yukawa sector plus $G_N$); it predicts none of their values. FTD claims to derive all 20 from a single axiom ($D = 3$ cubic lattice with ternary states and local causality) plus the mathematical constant $\varpi$ (the lemniscate constant). This catalog organizes those predictions by sharpness and testability, from sub-ppm numerical values to structural constraints to far-future Planck-scale signatures. For each prediction we state the FTD formula, the experimental comparison, the accuracy, and---critically---whether it is a genuine **pre-diction** (made before data) or a **post-diction** (matching already-known values). Most are post-dictions. Intellectual honesty demands this distinction.

---

## Section 1: Precision Numerical Predictions

These are the sharpest claims FTD makes---specific numerical values derived from the master quadratic and the framework integers $\{3, 4, 7, 13\}$. Each prediction is compared against the Particle Data Group (PDG) or CODATA value.

### 1.1 Fine Structure Constant

| Property | Value |
|----------|-------|
| **FTD formula** | $x^2 - 16G^{*2}x + 16G^{*3} = 0$, where $G^* = \varpi/\sqrt{\pi/4}$; $x_+ = 1/\alpha$ **[STRONGLY MOTIVATED CONJECTURE]** |
| **FTD value** | $1/\alpha = 137.0361714582\ldots$ |
| **CODATA 2022** | $1/\alpha = 137.035999177(21)$ |
| **Discrepancy** | 1.26 ppm |
| **Epistemic status** | **[STRONGLY MOTIVATED CONJECTURE]** (FTD-0013). The polynomial itself (master quadratic) is [THEOREM] (FTD-0001); the physical identification $x_+ = 1/\alpha$ rests on the algebraic match (1.26 ppm) plus dual-prediction structure ($x_- \to N_c$) plus CM-curve uniqueness — not on a dynamical derivation. Per LEDGER 2026-04-19, the prior "L → ∞ gap-equation" derivation route is RETRACTED (FTD-0032). |
| **Pre-diction or post-diction?** | **POST-DICTION.** $\alpha$ was measured in 1947. The derivation was constructed after the value was known. |

The tree-level value $x_+ = 137.036\ldots$ emerges from the master quadratic with coefficient 16 (lattice degrees of freedom) and $G^{*2}, G^{*3}$ (lemniscatic constant squared and cubed). The extended 7-term precision formula using $\varepsilon = e^\pi - \pi - 20$ and rational coefficients from $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$ agrees with the CODATA 2022 recommended value to 24 digits **as an algebraic identity** (confirmed 2026-04-17; see [CONJ_SEVEN_TERM_PRECISION_SERIES.md](../09_mathematical/CONJ_SEVEN_TERM_PRECISION_SERIES.md)). Status: **[CONJECTURE]**. The rigidity audit (2026-04-17) shows 6/7 coefficients uniquely forced in the base-integer set at cascade precision, but experimentally the coefficients are underdetermined — CODATA 2022 measures only ~11 digits.

### 1.2 Strong Coupling Constant

| Property | Value |
|----------|-------|
| **FTD formula** | $\alpha_s(M_Z) = b_3 / (b_3 + 4N_{\text{eff}}) = 7/59$ |
| **FTD value** | $0.11864$ |
| **PDG 2024** | $0.1179 \pm 0.0009$ |
| **Discrepancy** | 0.6% |
| **Epistemic status** | **[PARAMETRIC]** (ratio from framework integers; RG running formula is imported from QCD) |
| **Pre-diction or post-diction?** | **POST-DICTION.** $\alpha_s(M_Z)$ was measured in the 1990s. |

The numerator $b_3 = (11N_c - 2N_f)/3 = 7$ is the one-loop QCD beta function coefficient, a framework integer. The denominator structure $b_3 + 4N_{\text{eff}} = 7 + 52 = 59$ combines framework integers. The functional form of the running coupling is imported from standard QCD.

### 1.3 Weak Mixing Angle

| Property | Value |
|----------|-------|
| **FTD formula** | $\sin^2\theta_W = N_c / N_{\text{eff}} = 3/13$ |
| **FTD value** | $0.23077$ |
| **PDG 2024** | $0.23122 \pm 0.00003$ |
| **Discrepancy** | 0.2% |
| **Epistemic status** | **[PARAMETRIC]** (ratio from framework integers; definition of $\theta_W$ is imported from electroweak theory) |
| **Pre-diction or post-diction?** | **POST-DICTION.** $\sin^2\theta_W$ was measured in 1983. |

### 1.4 Electron Mass (Absolute Scale)

| Property | Value |
|----------|-------|
| **FTD formula** | $m_e = M_P \sqrt{2\pi} \cdot (N_{\text{base}}^2 / N_c) \cdot \alpha^{11} = M_P \sqrt{2\pi} \cdot (16/3) \cdot \alpha^{11}$ |
| **FTD value** | $0.5096$ MeV |
| **PDG** | $0.51100$ MeV |
| **Discrepancy** | 0.19% |
| **Epistemic status** | **[STRONGLY MOTIVATED CONJECTURE]** (given $\alpha$ from master quadratic and $M_P$ as scale identification) |
| **Pre-diction or post-diction?** | **POST-DICTION.** $m_e$ was measured in 1897. The $M_P$ identification is **[IMPOSED]**. |

The derivation requires identifying 1 voxel = 1 Planck length, which sets $M_P$. This is a scale calibration, not a derivation from pure integers. The power $\alpha^{11}$ encodes 11 layers of continuous-to-discrete projection: $\alpha^8$ (hierarchy suppression from Planck to electroweak) times $\alpha^3$ (Yukawa structure).

### 1.5 Tau Mass

| Property | Value |
|----------|-------|
| **FTD formula** | $m_\tau / m_e = (N_{\text{eff}} + N_{\text{base}}) \times 207 - 2 N_c \times b_3 = 17 \times 207 - 42 = 3477$ |
| **FTD value** | $m_\tau = 3477 \times 0.51100 = 1776.7$ MeV |
| **PDG** | $1776.86 \pm 0.12$ MeV |
| **Discrepancy** | 0.007% |
| **Epistemic status** | **[STRONGLY MOTIVATED CONJECTURE]** (integer arithmetic from framework constants) |
| **Pre-diction or post-diction?** | **POST-DICTION.** $m_\tau$ was measured in 1975. This is FTD's most accurate mass prediction. |

### 1.6 Higgs Mass

| Property | Value |
|----------|-------|
| **FTD formula** | $m_H = (N_{\text{eff}} / \alpha^2) \cdot m_e$ |
| **FTD value** | $124.8$ GeV |
| **PDG** | $125.25 \pm 0.17$ GeV |
| **Discrepancy** | 0.36% |
| **Epistemic status** | **[SELECTION]** (the formula structure is argued, not uniquely derived) |
| **Pre-diction or post-diction?** | **POST-DICTION.** $m_H$ was measured in 2012. |

### 1.7 Higgs VEV

| Property | Value |
|----------|-------|
| **FTD formula** | $v = M_P \sqrt{2\pi} \cdot \alpha^8$ |
| **FTD value** | $246.09$ GeV |
| **PDG** | $246.22$ GeV |
| **Discrepancy** | 0.05% |
| **Epistemic status** | **[STRONGLY MOTIVATED CONJECTURE]** (from $\alpha$-power hierarchy) |
| **Pre-diction or post-diction?** | **POST-DICTION.** The Higgs VEV was established in the 1980s via $G_F$ measurements. |

### 1.8 Gravitational Hierarchy

| Property | Value |
|----------|-------|
| **FTD formula** | $\alpha_G = 2\pi (16/3)^2 (N_{\text{eff}} + 3/b_3)^2 \alpha^{20}$ |
| **FTD value** | $5.909 \times 10^{-39}$ |
| **Experimental** | $5.906 \times 10^{-39}$ |
| **Discrepancy** | 0.06% |
| **Epistemic status** | **[THEOREM]** (integer structure with $\alpha^{20}$ hierarchy) |
| **Pre-diction or post-diction?** | **POST-DICTION.** $G_N$ was measured in 1798 (Cavendish). |

The exponent $20 = N_{\text{eff}} + b_3 = 13 + 7$ comes from cross-domain coupling (spatial to temporal degrees of freedom). The factor $(N_{\text{eff}} + 3/b_3)^2 = (13 + 3/7)^2 = (94/7)^2$ is derived from the framework integers.

### 1.9 Anomalous Magnetic Moment (g-2)

| Property | Value |
|----------|-------|
| **FTD formula** | $a_e = \alpha / (2\pi)$ (Schwinger result derived from one-loop vertex correction on FTD lattice) |
| **FTD value** | $0.001161$ |
| **Experimental** | $0.001159652\ldots$ |
| **Discrepancy** | 0.1% (tree-level; higher loops would improve) |
| **Epistemic status** | **[THEOREM]** (one-loop vertex correction on compact Brillouin zone) |
| **Pre-diction or post-diction?** | **POST-DICTION.** Schwinger computed this in 1948. FTD re-derives it from its own lattice Feynman rules. |

This is not a novel prediction---it is a verification that the FTD lattice Feynman rules reproduce the Schwinger result. The lattice UV regularization automatically renders the integral finite without external renormalization.

### 1.10 Neutral Pion Decay Width

| Property | Value |
|----------|-------|
| **FTD formula** | $\Gamma(\pi^0 \to \gamma\gamma) = (\alpha^2 m_\pi^3) / (64\pi^3 f_\pi^2) \times N_c^2$ (from chiral anomaly on FTD lattice) |
| **FTD value** | $7.73$ eV |
| **PDG** | $7.82 \pm 0.14$ eV |
| **Discrepancy** | 1.2% |
| **Epistemic status** | **[THEOREM]** (triangle diagram on compact BZ with $N_c = 3$) |
| **Pre-diction or post-diction?** | **POST-DICTION.** The $\pi^0$ lifetime was measured in the 1960s. FTD re-derives the anomaly coefficient. |

### 1.11 QCD Beta Function Coefficient

| Property | Value |
|----------|-------|
| **FTD formula** | $\beta_0 = (11N_c - 2N_f)/3 = (33 - 12)/3 = 7$ |
| **FTD value** | $7$ |
| **Standard QCD** | $7$ (exact match) |
| **Epistemic status** | **[THEOREM]** (this is a check, not a prediction---$\beta_0 = 7$ follows from $N_c = 3$, $N_f = 6$, which are framework outputs) |
| **Pre-diction or post-diction?** | **CHECK.** The formula is standard QCD. FTD provides $N_c$ and $N_f$ as inputs. |

### Prediction vs Post-diction Summary (Section 1)

| ID | Observable | FTD Value | PDG Value | Error | Pre/Post |
|----|-----------|-----------|-----------|-------|----------|
| P-1 | $1/\alpha$ | 137.036 | 137.036 | 1.26 ppm | Post |
| P-2 | $\alpha_s(M_Z)$ | 0.1186 | 0.1179 | 0.6% | Post |
| P-3 | $\sin^2\theta_W$ | 0.2308 | 0.2312 | 0.2% | Post |
| P-4 | $m_e$ | 0.5096 MeV | 0.5110 MeV | 0.19% | Post |
| P-5 | $m_\tau$ | 1776.7 MeV | 1776.9 MeV | 0.007% | Post |
| P-6 | $m_H$ | 124.8 GeV | 125.25 GeV | 0.36% | Post |
| P-7 | $v$ (Higgs VEV) | 246.09 GeV | 246.22 GeV | 0.05% | Post |
| P-8 | $\alpha_G$ | $5.909 \times 10^{-39}$ | $5.906 \times 10^{-39}$ | 0.06% | Post |
| P-9 | $a_e$ (g-2) | $\alpha/(2\pi)$ | $\alpha/(2\pi) + \ldots$ | 0.1% | Post |
| P-10 | $\pi^0 \to \gamma\gamma$ | 7.73 eV | 7.82 eV | 1.2% | Post |
| P-11 | $\beta_0$ (QCD) | 7 | 7 | exact | Check |

**Honest assessment:** Every numerical prediction in this section is a post-diction. All experimental values were known before FTD was formulated. This does not invalidate the framework---matching known data from a small set of inputs is necessary and impressive---but these are consistency checks, not predictions in the scientific sense.

---

## Section 2: Structural Predictions

These are discrete, qualitative claims about the structure of particle physics. Unlike numerical predictions, they are binary: either true or false.

### 2.1 Exactly Three Generations

**FTD claim:** $N_{\text{gen}} = \lfloor x_- \rfloor = \lfloor 3.024 \rfloor = 3$ exactly. No fourth-generation fermions with standard gauge couplings exist at any energy.

**Formula:** The smaller root of the master quadratic gives $x_- = 3.024$, and the floor function yields the integer 3. This is a structural output of the quadratic, not adjustable.

**Current status:** LHC excludes 4th-generation quarks with $m > 800$ GeV. Precision electroweak fits disfavor $N_{\text{gen}} = 4$.

**Falsification:** Discovery of $t'$, $b'$, $\tau'$, or $\nu_4$ with standard $\text{SU}(2) \times \text{U}(1)$ chiral couplings. Heavy sterile particles or vector-like fermions would NOT falsify this prediction.

**Pre-diction or post-diction?** **POST-DICTION.** LEP measured $N_\nu = 2.984 \pm 0.008$ from $Z$-width in 1989. Three generations were known before FTD.

**Epistemic status:** **[THEOREM]** (follows from quadratic root structure).

### 2.2 Exactly Three Colors

**FTD claim:** $N_c = \lfloor x_- \rfloor = 3$ exactly. QCD has SU(3) gauge symmetry with 3 color charges, no more.

**Current status:** All hadronic physics is consistent with $N_c = 3$.

**Falsification:** Discovery of hadrons requiring $N_c > 3$ color charges, or detection of non-SU(3) color structure.

**Pre-diction or post-diction?** **POST-DICTION.** $N_c = 3$ was established in the 1970s (R-ratio, $\pi^0$ decay).

**Epistemic status:** **[THEOREM]**

### 2.3 Gauge Group is $\text{U}(1) \times \text{SU}(2) \times \text{SU}(3)$

**FTD claim:** The gauge group is exactly $\text{U}(1) \times \text{SU}(2) \times \text{SU}(3)$. There is no larger unifying group (no SU(5), SO(10), $E_6$, etc.) that embeds this product at accessible energies.

**Derivation:** U(1) from Gauss constraint, SU(2) from ternary state doublet, SU(3) from three spatial flux components. No room for additional gauge bosons---the lattice structure is exhausted.

**Falsification:** Discovery of additional gauge bosons ($X$, $Y$ leptoquarks from GUT, $Z'$, $W'$) with fundamental gauge couplings.

**Pre-diction or post-diction?** **POST-DICTION.** The SM gauge group was established by the 1970s. The prediction that no unification group exists at accessible energies is structurally more interesting.

**Epistemic status:** U(1) **[THEOREM]**, SU(2) **[SELECTION]**, SU(3) **[SELECTION]**.

### 2.4 No Magnetic Monopoles

**FTD claim:** Magnetic monopoles do not exist. The lattice topology does not support the requisite non-trivial $\pi_2(\text{U}(1)) = 0$ bundle structure needed for Dirac monopoles.

**Current status:** No monopoles detected. MoEDAL at LHC has null results.

**Falsification:** Detection of any isolated magnetic charge.

**Pre-diction or post-diction?** **WEAK PRE-DICTION.** No monopole has been found, but most GUTs predict them. FTD's prediction that they are structurally impossible is more definitive than the SM (which is agnostic on monopoles).

**Epistemic status:** **[THEOREM]** (lattice topology argument).

### 2.5 No Proton Decay

**FTD claim:** The proton is absolutely stable ($\tau_p = \infty$). Baryon number conservation is exact on the discrete lattice---charge conservation in a finite discrete system forbids baryon number violation.

**Current status:** Super-K bound: $\tau_p > 2.4 \times 10^{34}$ years ($p \to e^+\pi^0$).

**Falsification:** Any proton decay observation at any rate.

**Distinguishing from GUTs:** GUTs predict $\tau_p \sim 10^{34} - 10^{36}$ years. Continued null results beyond $10^{36}$ years (Hyper-K era) would favor FTD over GUTs.

**Pre-diction or post-diction?** **WEAK PRE-DICTION.** No decay has been observed. FTD makes the stronger claim that decay is impossible, not merely rare.

**Epistemic status:** **[THEOREM]** (discrete charge conservation).

### 2.6 Normal Neutrino Mass Hierarchy

**FTD claim:** Neutrino masses follow the normal hierarchy ($m_1 < m_2 < m_3$), producing $\Delta m^2_{31} > 0$.

**Current status:** JUNO expected to determine hierarchy by 2027. Current data slightly favors normal ($\Delta\chi^2 \sim 3$).

**Falsification:** Definitive measurement of inverted hierarchy ($m_3 < m_1 < m_2$).

**Pre-diction or post-diction?** **GENUINE PRE-DICTION** (if confirmed before alternative). The hierarchy is currently undetermined.

**Epistemic status:** **[SELECTION]** (follows from FTD seesaw structure, but seesaw is partially imported).

---

## Section 3: Lattice-Specific Predictions

These predictions are unique to discrete spacetime models. The SM, formulated on continuous spacetime, makes none of these claims. Most are currently untestable.

### 3.1 Planck-Scale Photon Dispersion

**FTD formula:**

$$v(E) = c\left[1 - \frac{E^2}{24 E_P^2}\right]$$

where $E_P = 1.22 \times 10^{19}$ GeV is the Planck energy.

**Coefficient derivation:** The factor 1/24 comes from the second-order Taylor expansion of the lattice dispersion relation $\omega(k) = 2\sin(k/2)$ around $k = 0$. Specifically, $\sin^2(k/2) \approx k^2/4 - k^4/48$, yielding the $E^2/(24E_P^2)$ correction.

**Expected magnitude:** At $E = 100$ GeV: $\Delta v / c \sim 10^{-36}$. At $E = 10$ TeV: $\Delta v / c \sim 10^{-32}$. Both are many orders of magnitude below detectability.

**Current bounds:** Fermi-LAT GRB observations constrain $E_{\text{QG}} > 10^{18}$ GeV (linear dispersion). FTD predicts quadratic dispersion, which is less constrained.

**Falsification:** Detection of photon dispersion with the wrong sign (superluminal at high energy) or wrong power law (linear instead of quadratic).

**Epistemic status:** **[THEOREM]** (follows from any cubic lattice discretization). **Not unique to FTD**---any discrete spacetime model predicts similar dispersion.

### 3.2 Cubic Lattice Anisotropy

**FTD prediction:** Space has preferred directions at the Planck scale due to the cubic lattice structure.

**Expected magnitude:** $\varepsilon \sim (E/E_P)^4 \sim 10^{-80}$ at TeV scales.

**Status:** Undetectable by many orders of magnitude with any foreseeable technology.

**Falsification:** Detection of anisotropy with wrong symmetry (non-cubic) or at much larger magnitude than predicted.

**Epistemic status:** **[THEOREM]** (follows from cubic lattice axiom). Not unique to FTD.

### 3.3 No Landau Pole in QED

**FTD claim:** The QED Landau pole (where $\alpha_{\text{EM}}$ would formally diverge) does not exist. The compact Brillouin zone of the lattice imposes a natural UV cutoff at $k = \pi$ (in lattice units), preventing the running coupling from diverging.

**SM status:** Perturbative QED predicts a Landau pole at $\Lambda_{\text{Landau}} \sim M_P \cdot e^{3\pi/(2\alpha)} \sim 10^{286}$ eV, far beyond physical relevance. The SM is agnostic about what happens there.

**FTD prediction:** The running coupling saturates at a finite value determined by the lattice cutoff. No divergence occurs at any energy.

**Falsification:** Observation of QED coupling divergence or non-perturbative QED effects at a scale inconsistent with lattice predictions.

**Epistemic status:** **[THEOREM]** (compact BZ implies bounded integrals). Not unique to FTD---any lattice QFT has this property.

### 3.4 UV Finiteness of All Loop Integrals

**FTD claim:** All loop integrals in the FTD framework are automatically UV-finite. The compact Brillouin zone $[-\pi, \pi]^3$ replaces $(-\infty, \infty)^3$ in momentum integrals, rendering every diagram convergent without external regularization or renormalization.

**SM status:** The SM requires renormalization to handle UV divergences. The renormalized predictions are extraordinarily accurate, but the bare theory is divergent.

**Consequence:** FTD predicts that the "hierarchy problem" (why $m_H \ll M_P$) is an artifact of continuous spacetime. On the lattice, no quadratic divergence exists to destabilize the Higgs mass.

**Epistemic status:** **[THEOREM]** (mathematical property of compact integration domains).

### 3.5 Lattice Corrections to Propagators

**FTD claim:** At momenta approaching the lattice cutoff ($k \sim \pi$ in lattice units, corresponding to $E \sim E_P$), propagators deviate from their continuum forms. The lattice propagator:

$$G(k) = \frac{1}{\hat{k}^2 + m^2}, \quad \hat{k}_\mu = 2\sin(k_\mu/2)$$

differs from the continuum $1/(k^2 + m^2)$ by terms of order $(k/\pi)^2$.

**Testability:** Requires probing physics at $E \sim 10^{19}$ GeV. Currently inaccessible.

**Epistemic status:** **[THEOREM]** (standard lattice QFT result).

### 3.6 Modified Dispersion at Trans-Planckian Energies

**FTD claim:** At energies above $E_P$, the dispersion relation wraps around the Brillouin zone. Energy is periodic in momentum: $\omega(k + 2\pi) = \omega(k)$. There is a maximum momentum $p_{\max} = \pi \hbar / \ell_P$, beyond which particles cannot be accelerated---they simply acquire lower-frequency lattice modes.

**Consequence:** No trans-Planckian catastrophe. Black hole formation above $E_P$ is naturally regulated.

**Epistemic status:** **[THEOREM]** (follows from lattice periodicity). Untestable with foreseeable technology.

---

## Section 4: Cosmological Predictions

### 4.1 Inflationary Spectral Index

| Property | Value |
|----------|-------|
| **FTD formula** | $n_s = 1 - 2/N_e$, with $N_e = \alpha^{-1}/\sqrt{2\pi} \approx 56.3$ e-folds |
| **FTD value** | $n_s = 0.9645$ |
| **Planck 2018** | $n_s = 0.9649 \pm 0.0042$ |
| **Discrepancy** | $0.1\sigma$ |
| **Epistemic status** | **[SELECTION]** (the formula $n_s = 1 - 2/N_e$ is standard slow-roll; $N_e$ from FTD is **[SELECTION]**) |
| **Pre-diction or post-diction?** | **POST-DICTION.** Planck data (2013-2018) predates FTD v5.0. However, the formula was fixed by framework before comparison. |

### 4.2 Tensor-to-Scalar Ratio

| Property | Value |
|----------|-------|
| **FTD formula** | $r = 4\alpha \cdot (3/4) \approx 0.022$ |
| **Alternative** | $r = 8/N_e \approx 0.007$ (from $N_e \approx 56.3$) |
| **Current bound** | $r < 0.036$ (95% CL, Planck + BICEP) |
| **Epistemic status** | **[SELECTION]** |
| **Pre-diction or post-diction?** | **GENUINE PRE-DICTION.** $r$ has not been measured---only upper bounds exist. LiteBIRD (2030s) aims for $\sigma(r) \sim 0.001$, which would test both FTD values. |

**Falsification:** Measurement of $r > 0.03$ falsifies the higher estimate. Measurement of $r < 0.005$ falsifies both.

### 4.3 Baryon Asymmetry

| Property | Value |
|----------|-------|
| **FTD formula** | $\eta \sim \alpha^3 \cdot J_{\text{CP}} \sim 10^{-10}$ (from Sakharov conditions + CP violation on lattice) |
| **Observed** | $\eta = (6.12 \pm 0.04) \times 10^{-10}$ |
| **Discrepancy** | Correct order of magnitude |
| **Epistemic status** | **[SELECTION]** (order-of-magnitude, not precise) |
| **Pre-diction or post-diction?** | **POST-DICTION.** The baryon asymmetry was measured from BBN and CMB decades before FTD. |

### 4.4 Dark Matter as Sub-Threshold Flux

**FTD claim:** Dark matter is not particulate. It consists of sub-threshold flux regions where $0 < |\mathbf{J}| < K_B$---flux that gravitates (contributes to $\mathcal{L}$-latency via Poisson equation) but never manifests as $s = \pm 1$ (never becomes visible matter).

**Consequences:**
- All WIMP direct detection experiments will yield null results indefinitely
- Dark matter "halos" are smooth flux distributions, not collections of particles
- No annihilation signals (dark matter is not particle-antiparticle)

**Current status:** LZ, XENONnT, PandaX-4T: all null. Consistent with FTD.

**Falsification:** Statistically significant ($>5\sigma$) WIMP direct detection signal.

**Epistemic status:** **[CONJECTURE]**. The identification is qualitative. No connection has been established between $\mathcal{L} = 0.75$ and $\Omega_{\text{DM}} \approx 0.27$. No galaxy rotation curves have been computed from this ansatz.

### 4.5 Dark Energy from Vacuum Computational Budget

**FTD claim:** The cosmological constant $\Lambda$ reflects the residual computational budget of the vacuum---the fraction of lattice processing capacity not consumed by matter or radiation.

**Epistemic status:** **[CONJECTURE]**. This is a qualitative framing with no quantitative derivation of $\Lambda$ from lattice parameters.

---

## Section 5: Negative Predictions

FTD predicts that certain entities and phenomena will never be observed. These are falsifiable: a single confirmed discovery would refute the relevant prediction.

### 5.1 No WIMPs

**Prediction:** All WIMP direct detection experiments will yield null results at all sensitivities.

**Reason:** FTD dark matter is sub-threshold flux ($s = 0$ always), not massive particles. Nothing scatters off nuclei.

**Falsification:** Any $>5\sigma$ direct detection signal.

### 5.2 No Proton Decay

**Prediction:** $\tau_p = \infty$. Discrete charge conservation on the lattice is exact.

**Falsification:** Any proton decay event at any rate.

### 5.3 No Magnetic Monopoles

**Prediction:** Isolated magnetic charges cannot exist in the lattice topology.

**Falsification:** Confirmed magnetic monopole detection.

### 5.4 No Extra Spatial Dimensions

**Prediction:** $D = 3$ is uniquely selected. No Kaluza-Klein modes, no large extra dimensions, no deviations from $1/r^2$ gravity at any scale.

**Falsification:** Detection of KK excitations, or gravitational deviation from inverse-square law.

### 5.5 No Supersymmetric Partners

**Prediction:** SUSY is incompatible with the discrete lattice. No superpartner particles exist.

**Reason:** The SUSY algebra requires the continuous Lorentz group. The cubic lattice fundamentally breaks the continuous symmetry needed for SUSY generators.

**Falsification:** Discovery of any superpartner with standard SUSY quantum numbers.

### 5.6 No Landau Pole

**Prediction:** QED does not develop a Landau pole. The compact Brillouin zone prevents coupling divergence.

**Falsification:** Observation of QED non-perturbative effects consistent with a Landau pole.

### 5.7 No CPT Violation

**Prediction:** CPT symmetry is exact. Although $T$ alone is not a fundamental symmetry (time is ontologically irreversible, $t \in \mathbb{N}$), the combined CPT operation---which acts on spatial dynamics, not on time itself---is preserved by the lattice structure.

**Falsification:** Any confirmed CPT violation ($m_{\text{particle}} \neq m_{\text{antiparticle}}$, or lifetime asymmetry beyond experimental uncertainty).

### Negative Predictions Summary

| Prediction | SM Status | FTD Status | Current Evidence |
|-----------|-----------|------------|-----------------|
| No WIMPs | SM is agnostic | **Predicted null** | Null (consistent) |
| No proton decay | SM allows it; GUTs predict it | **Absolute stability** | Null (consistent) |
| No monopoles | SM is agnostic; GUTs predict them | **Structurally impossible** | Null (consistent) |
| No extra dimensions | SM is agnostic | **$D = 3$ unique** | Null (consistent) |
| No SUSY | SM is agnostic | **Incompatible with lattice** | Null (consistent) |
| No Landau pole | SM predicts one (irrelevantly far) | **Compact BZ prevents it** | Untestable |
| No CPT violation | SM preserves CPT | **Exact** | Preserved (consistent) |

---

## Section 6: The Sub-ppm Alpha Challenge

### The Single Most Impactful Potential Prediction

If FTD could predict $\alpha$ to sub-ppm precision---better than current CODATA measurements---it would constitute a genuine, falsifiable pre-diction of unprecedented sharpness. This is the only realistic near-term path to a novel prediction that distinguishes FTD from the SM.

### Current Status

The tree-level master quadratic gives $1/\alpha = 137.0361714582\ldots$, which is 1.26 ppm above the CODATA 2022 value $137.035999177(21)$. The gap is $\Delta = 0.000172$, approximately $1.72 \times 10^{-4}$.

### The Two-Loop Opportunity

The one-loop vacuum polarization on the FTD lattice shifts the coupling by order $\alpha/(3\pi) \approx 7.7 \times 10^{-4}$. The two-loop correction is order $\alpha^2$ smaller, giving $\sim 5 \times 10^{-6}$---the right order of magnitude to close the 1.26 ppm gap. See DERIV_TWO_LOOP_ALPHA.md.

### What Is Needed

1. **Exact two-loop calculation on the compact Brillouin zone** (finite, no regularization needed)
2. **Demonstration that the correction shifts $x_+$ toward CODATA**, not away
3. **Residual discrepancy smaller than CODATA uncertainty** ($\pm 0.15$ ppm)

### Current Assessment

The two-loop correction has the right magnitude. Whether it has the right sign and precise value to close the gap is **[OPEN]**. The 4-term precision formula (see DERIV_ALPHA_PRECISION_FORMULA.md) achieves agreement at digit-counts beyond CODATA's experimental precision (~11 digits), but the formula's coefficient identifications are **[CONJECTURE]** (FTD-0022, post-hoc fit), not uniquely derived.

**Epistemic status:** **[CONJECTURE]** that sub-ppm $\alpha$ prediction is achievable. The framework has the structural ingredients; the explicit calculation remains incomplete.

---

## Section 7: Testability Timeline

| Timeline | Predictions | Test Method |
|----------|------------|-------------|
| **Now testable** | $1/\alpha = 137.036$ (1.26 ppm); $\sin^2\theta_W = 3/13$; $m_\tau/m_e = 3477$; $N_{\text{gen}} = 3$; $N_c = 3$; $m_H \approx 124.8$ GeV; no SUSY; no extra dimensions | Precision QED measurements; collider searches; particle catalogs |
| **Near-term (5-10 years)** | Sub-ppm $\alpha$ (if 2-loop calculation completed); neutrino hierarchy (JUNO, 2027); Higgs precision (HL-LHC); continued WIMP null results; $r \approx 0.022$ (LiteBIRD, 2030s) | JUNO, HL-LHC, LiteBIRD, next-gen dark matter detectors |
| **Medium-term (10-30 years)** | Proton stability beyond $10^{36}$ years (Hyper-K); baryon masses ($\Omega_b^*$, $B_c(2S)$) | Hyper-Kamiokande, LHCb upgrades |
| **Far-term (>30 years)** | Planck-scale photon dispersion; cubic lattice anisotropy; trans-Planckian dispersion | Next-generation gamma-ray telescopes; gravitational wave astronomy; unknown future technology |
| **Possibly never** | Lattice corrections to propagators ($k \sim \pi$); BZ periodicity; Landau pole absence | Would require probing $E_P \sim 10^{19}$ GeV directly |

---

## Section 8: Comparison with the Standard Model

### What the SM Predicts

The Standard Model has **20 free parameters** (19 in the electroweak/Yukawa sector plus $G_N$). It predicts:

- **Zero** parameter values from first principles
- Ratios between processes once parameters are fixed (cross sections, branching ratios, etc.)
- The existence of the Higgs boson (confirmed 2012)
- Asymptotic freedom of QCD
- CP violation in the weak sector
- The structure of radiative corrections (anomalous magnetic moments, running couplings)

The SM does not predict $\alpha$, $\sin^2\theta_W$, any fermion mass, the number of generations, the CKM/PMNS parameters, or $G_N$.

### What FTD Predicts That the SM Cannot

| Category | Count | Examples |
|----------|-------|---------|
| **Coupling constant values** | 3 | $\alpha$, $\alpha_s(M_Z)$, $\sin^2\theta_W$ |
| **Mass values** | 5+ | $m_e$, $m_\tau$, $m_H$, $v$, $m_p/m_e$ |
| **Mass ratios** | 3+ | $m_\mu/m_e$, $m_\tau/m_e$, $m_p/m_e$ |
| **Mixing parameters** | 4+ | CKM $\theta_{12}$, CKM $\delta$, PMNS angles |
| **Gravitational hierarchy** | 1 | $\alpha_G = \alpha^{20} \times \ldots$ |
| **Structural constraints** | 6 | $N_{\text{gen}} = 3$, $N_c = 3$, no SUSY, no monopoles, no proton decay, no extra $D$ |
| **Cosmological parameters** | 2 | $n_s$, $r$ |
| **Negative predictions** | 7 | No WIMPs, no SUSY, no monopoles, etc. |

**Total: ~30 predictions the SM cannot make.**

### The Honest Caveat

Almost all of these are **post-dictions**. The values were known before FTD was formulated. A framework that matches 30 known numbers from a small input set is scientifically interesting, but it is not the same as predicting 30 numbers before measurement. The critical test is whether FTD can produce a genuinely novel pre-diction---see Section 6 (sub-ppm $\alpha$) and Section 4.2 (tensor-to-scalar ratio $r$).

---

## Section 9: Claims Table

| ID | Statement | Tag | Testability | Pre/Post |
|----|-----------|-----|-------------|----------|
| NP-1 | $1/\alpha = 137.0361714582\ldots$ from master quadratic | [STRONGLY MOTIVATED CONJECTURE] | Now (1.26 ppm off CODATA) | Post |
| NP-2 | $\alpha_s(M_Z) = 7/59 = 0.11864$ | [PARAMETRIC] | Now (0.6% off PDG) | Post |
| NP-3 | $\sin^2\theta_W = 3/13 = 0.23077$ | [PARAMETRIC] | Now (0.2% off PDG) | Post |
| NP-4 | $m_e = M_P\sqrt{2\pi}(16/3)\alpha^{11}$ | [STRONGLY MOTIVATED CONJECTURE] | Now (0.19% off PDG) | Post |
| NP-5 | $m_\tau/m_e = 3477$ | [STRONGLY MOTIVATED CONJECTURE] | Now (0.007% off PDG) | Post |
| NP-6 | $m_H = (N_{\text{eff}}/\alpha^2) m_e \approx 124.8$ GeV | [STRUCTURALLY MOTIVATED PARAMETRIC] | Now (0.36% off PDG) | Post |
| NP-7 | $v = M_P\sqrt{2\pi}\alpha^8 = 246.09$ GeV | [STRUCTURALLY MOTIVATED PARAMETRIC] | Now (0.05% off PDG) | Post |
| NP-8 | $\alpha_G = 2\pi(16/3)^2(N_{\text{eff}}+3/b_3)^2\alpha^{20}$ | [THEOREM] | Now (0.06% off experimental) | Post |
| NP-9 | $a_e = \alpha/(2\pi)$ from lattice vertex correction | [THEOREM] | Now (Schwinger re-derivation) | Post |
| NP-10 | $\Gamma(\pi^0 \to \gamma\gamma) = 7.73$ eV from lattice anomaly | [THEOREM] | Now (1.2% off PDG) | Post |
| NP-11 | $\beta_0 = 7$ (QCD one-loop) | [THEOREM] | Now (exact match) | Check |
| NP-12 | $N_{\text{gen}} = 3$ exactly | [THEOREM] | Now (consistent) | Post |
| NP-13 | $N_c = 3$ exactly | [STRONGLY MOTIVATED CONJECTURE] | Now (consistent) | Post |
| NP-14 | Gauge group = $\text{U}(1) \times \text{SU}(2) \times \text{SU}(3)$ | [THEOREM] + [SELECTION] | Now (consistent) | Post |
| NP-15 | No magnetic monopoles | [THEOREM] | Now (null searches consistent) | Weak pre |
| NP-16 | No proton decay ($\tau_p = \infty$) | [THEOREM] | Now/near-term (null consistent) | Weak pre |
| NP-17 | No SUSY particles | [THEOREM] | Now (null LHC consistent) | Weak pre |
| NP-18 | No extra spatial dimensions | [THEOREM] | Now (null searches consistent) | Weak pre |
| NP-19 | No WIMPs (dark matter non-particulate) | [CONJECTURE] | Now (null detection consistent) | Weak pre |
| NP-20 | Normal neutrino mass hierarchy | [SELECTION] | Near-term (JUNO ~2027) | **Pre** |
| NP-21 | $r \approx 0.022$ (tensor-to-scalar ratio) | [SELECTION] | Near-term (LiteBIRD ~2032) | **Pre** |
| NP-22 | $n_s = 0.9645$ (spectral index) | [SELECTION] | Now ($0.1\sigma$ from Planck) | Post |
| NP-23 | $\eta \sim 10^{-10}$ (baryon asymmetry) | [SELECTION] | Now (order of magnitude) | Post |
| NP-24 | Dark matter = sub-threshold flux | [CONJECTURE] | Near-term (continued WIMP nulls) | Weak pre |
| NP-25 | Photon dispersion $v(E) = c[1 - E^2/(24E_P^2)]$ | [THEOREM] | Far-term (Planck-scale effect) | Pre (generic) |
| NP-26 | Cubic lattice anisotropy $\sim (E/E_P)^4$ | [THEOREM] | Far-term ($\sim 10^{-80}$) | Pre (generic) |
| NP-27 | No Landau pole in QED | [THEOREM] | Possibly never | Pre (generic) |
| NP-28 | UV finiteness of all loops | [THEOREM] | Possibly never (formal result) | Pre (generic) |
| NP-29 | No CPT violation | [THEOREM] | Now (consistent) | Post |
| NP-30 | Sub-ppm $\alpha$ from 2-loop calculation | [CONJECTURE] | Near-term (if calculation completed) | **Pre** |
| NP-31 | $\Omega_b^*(6350)$ baryon mass | [SELECTION] | Near-term (LHCb) | **Pre** |
| NP-32 | $B_c(2S) = 6871 \pm 5$ MeV | [SELECTION] | Near-term (LHCb) | **Pre** |
| NP-33 | No forces between EM and gravity coupling scales | [SELECTION] | Now (no fifth force detected) | Weak pre |
| NP-34 | Proton stability beyond $10^{36}$ years | [THEOREM] | Medium-term (Hyper-K) | **Pre** |

---

## Summary of Genuinely Novel Pre-dictions

Out of 34 cataloged predictions, only a handful are genuinely pre-dictive (made before experimental confirmation):

| Priority | Prediction | Timeline | Impact if Confirmed |
|----------|-----------|----------|---------------------|
| **Highest** | NP-30: Sub-ppm $\alpha$ | Near-term | Would be the first parameter value predicted more precisely than measured |
| **High** | NP-21: $r \approx 0.022$ | LiteBIRD ~2032 | Would distinguish FTD from many inflation models |
| **High** | NP-20: Normal hierarchy | JUNO ~2027 | Would be confirmed alongside other frameworks; not unique to FTD |
| **Medium** | NP-31, NP-32: Baryon masses | LHCb | Would test framework mass formulas on unobserved states |
| **Medium** | NP-34: Proton stable beyond $10^{36}$ yr | Hyper-K | Would rule out minimal GUTs in favor of FTD |
| **Low (generic)** | NP-25, NP-26: Planck-scale effects | Far future | Not unique to FTD; any lattice model predicts similar |

The framework's most impressive results---matching $\alpha$, $\sin^2\theta_W$, $m_\tau/m_e$, and $\alpha_G$ from a small set of integers---are post-dictions. They are necessary but not sufficient for scientific validation. The path to genuine confirmation runs through Section 6 (sub-ppm $\alpha$) and Section 4.2 ($r$), where FTD makes claims before the data exists.

---

## References

1. AUDIT_NOVEL_PREDICTIONS.md -- Timestamped pre-registered predictions (v1.1, Feb 2026)
2. AUDIT_EPISTEMIC_AUDIT.md -- Honest derivation accounting (v2.0)
3. SPEC_FTD_LAGRANGIAN.md -- Born-Infeld render-bridge action with all constants
4. SPEC_THE_MASTER_QUADRATIC_UNIFIED.md -- Master quadratic derivation and roots
5. DERIV_ALPHA_PRECISION_FORMULA.md -- 4-term precision formula
6. DERIV_TWO_LOOP_ALPHA.md -- Two-loop correction and road to sub-ppm
7. DERIV_LATTICE_CHIRAL_ANOMALY.md -- Chiral anomaly and pion decay
8. DERIV_LATTICE_VERTEX_CORRECTION.md -- One-loop g-2 from lattice
9. DERIV_LATTICE_SU3_GAUGE.md -- SU(3) from flux geometry
10. DERIV_LATTICE_SU2_WEAK.md -- SU(2) weak sector
11. DERIV_HIGGS_FROM_MANIFESTATION.md -- Higgs mechanism from manifestation
12. FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md -- Space-time separation predictions

---

*Document version 1.0 -- Complete Novel Predictions Catalog*
*February 25, 2026*
*Framework: Foundational Ternary Dynamics v5.26*
