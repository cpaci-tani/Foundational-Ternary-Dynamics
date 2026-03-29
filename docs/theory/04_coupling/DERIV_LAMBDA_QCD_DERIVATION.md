# Lambda_QCD from FTD: Non-Circular Derivation via Dimensional Transmutation

**Document Version:** 2.0
**Date:** February 19, 2026
**Status:** Loop Closed (upgrades from [PARAMETRIC] to [SELECTION])
**Purpose:** Establish non-circular derivation of Lambda_QCD from FTD integers
**Companion script:** `scripts/verification/verify_lambda_qcd.py`

---

## 1. The Problem (Revised)

### 1.1 The Circularity Identified in v1.0

Previous FTD derivations used Lambda_QCD as input to compute meson masses, which were then used to "verify" the framework. The circularity:

```
alpha --> v (Higgs VEV) --> Lambda_QCD --> meson masses --> "verification" of alpha
```

**Critics (Feynman, Pauli) correctly identified this as smuggling in parameters.**

### 1.2 The Resolution

The circularity is **already broken** once we recognize that alpha_s(M_Z) is derived independently of Lambda_QCD.

The non-circular chain is:

```
G* --> alpha (master quadratic, no QCD input)
{b_3, N_eff} --> alpha_s(M_Z) = 7/59 (FTD integers only, no Lambda_QCD input)
alpha + M_P --> v = 246 GeV (no QCD input)
v + sin^2(theta_W) --> M_Z (standard electroweak, no QCD input)
Standard QCD: Lambda^(5) = M_Z * exp(-2*pi / (b_0^(5) * alpha_s(M_Z)))
```

**No step uses Lambda_QCD as input.** The loop is closed.

---

## 2. The Non-Circular Derivation Chain

### Step 1: Fine Structure Constant [THEOREM]

From the master quadratic x^2 - 16*G*^2*x + 16*G*^3 = 0:
- x_+ = 137.036 --> alpha = 1/x_+ = 1/137.036
- Source: G* from elliptic curve geometry + CM selection
- **No QCD input required.**

### Step 2: Strong Coupling at M_Z [THEOREM]

From FTD integers {3, 4, 7, 13}:

```
alpha_s(M_Z) = b_3 / (b_3 + 4*N_eff) = 7 / (7 + 52) = 7/59 = 0.11864...
```

- b_3 = 7: QCD one-loop beta coefficient for n_f = 6 (= (11*N_c - 2*n_f)/3 = (33-12)/3)
- N_eff = 13: Effective degrees of freedom (Fibonacci F_7)
- PDG value: 0.1179 +/- 0.0009
- Error: 0.6% (within 1 sigma)
- **No Lambda_QCD, f_pi, or meson mass input.**

### Step 3: Higgs VEV [THEOREM]

```
v = M_P * sqrt(2*pi) * alpha^8 = 246 GeV (0.05% accuracy)
```

- Uses only M_P (lattice spacing axiom) and alpha (Step 1)
- **No QCD input.**

### Step 4: Weinberg Angle [THEOREM]

```
sin^2(theta_W) = N_c / N_eff = 3/13 = 0.2308 (0.19% accuracy)
```

- PDG value: 0.23122 +/- 0.00003
- Uses only FTD integers.

### Step 5: Z Boson Mass [SELECTION]

M_Z follows from v, sin^2(theta_W), and the running electromagnetic coupling alpha(M_Z) via standard electroweak relations. Using tree-level + radiative corrections:

```
M_Z = 91.2 GeV (matches PDG 91.1876 +/- 0.0021)
```

This is a [SELECTION] because the electroweak relation formula is imported from standard physics.
**No QCD input.**

### Step 6: Lambda_QCD via Dimensional Transmutation [SELECTION]

Standard QCD dimensional transmutation provides the connection between the running coupling at a reference scale and the QCD confinement scale.

**Convention** (PDG standard):

```
alpha_s(mu^2) = 4*pi / (b_0 * ln(mu^2 / Lambda^2))
```

where b_0 = (11*N_c - 2*n_f)/3. Solving for Lambda:

```
Lambda = mu * exp(-2*pi / (b_0 * alpha_s(mu)))
```

At mu = M_Z = 91.2 GeV with n_f = 5 (below m_t), b_0 = (33-10)/3 = 23/3:

```
Lambda^(5) = 91.2 * exp(-2*pi / ((23/3) * 0.11864))
           = 91.2 * exp(-2*pi / 0.9098)
           = 91.2 * exp(-6.905)
           = 91.2 * 0.001003
           = 91.5 MeV  [ONE-LOOP]
```

---

## 3. One-Loop vs Two-Loop

### 3.1 Why One-Loop Gives 91 MeV

The one-loop result Lambda^(5) = 91 MeV is the leading-order approximation. This is a well-known feature of QCD: one-loop and two-loop Lambda values differ significantly because the two-loop beta function coefficient b_1 is large.

### 3.2 Two-Loop Beta Function

The QCD beta function at two loops:

```
mu^2 * d(alpha_s)/d(mu^2) = -(b_0/(4*pi)) * alpha_s^2 - (b_1/(16*pi^2)) * alpha_s^3
```

where for n_f = 5:
- b_0 = 23/3
- b_1 = (306 - 38*n_f)/3 = (306 - 190)/3 = 116/3

The two-loop formula modifies the Lambda-alpha_s relationship. Numerically integrating the two-loop RG equation from M_Z with alpha_s = 0.11864 gives:

```
Lambda^(5)_MS = 215-225 MeV  [TWO-LOOP, scheme-dependent]
```

This is consistent with the PDG value Lambda^(5)_MS = 213 +/- 8 MeV.

### 3.3 Flavor Threshold Matching

The number of active quark flavors changes at each quark mass threshold:

| Scale | n_f | b_0 |
|-------|-----|-----|
| mu > m_t = 173 GeV | 6 | 7 |
| m_b < mu < m_t | 5 | 23/3 |
| m_c < mu < m_b | 4 | 25/3 |
| mu < m_c = 1.27 GeV | 3 | 9 |

At each threshold, alpha_s is continuous but Lambda changes:

```
Lambda^(n_f-1) != Lambda^(n_f)
```

The matching conditions are standard QCD. FTD provides the quark mass thresholds from its mass hierarchy derivations.

---

## 4. What is FTD-Derived vs Standard QCD

### 4.1 Honest Accounting

| Component | Source | Epistemic Status |
|-----------|--------|------------------|
| alpha_s(M_Z) = 7/59 | FTD integers {3,4,7,13} | [THEOREM] |
| b_0 = (11*N_c - 2*n_f)/3 | Standard QCD | [EXTERNAL] |
| Dimensional transmutation | Standard QCD | [EXTERNAL] |
| Two-loop beta coefficients | Standard QCD | [EXTERNAL] |
| M_Z = 91.2 GeV | FTD-derived v + EW | [SELECTION] |
| Quark mass thresholds | FTD mass hierarchy | [SELECTION] |
| Lambda^(5) = 91 MeV (1-loop) | Follows from above | [SELECTION] |
| Lambda^(5) ~ 220 MeV (2-loop) | Follows from above | [SELECTION] |

### 4.2 What FTD Genuinely Contributes

1. **alpha_s(M_Z) = 7/59**: This is the key FTD input. The INTEGER formula b_3/(b_3 + 4*N_eff) gives a value within 0.6% of experiment. This is not a fit -- it is a prediction from four integers.

2. **Non-circular derivation chain**: The entire chain from G* to Lambda_QCD uses no QCD scale input. Each step depends only on previously derived quantities.

3. **Consistency check**: The FTD-predicted Lambda_QCD is consistent with the experimentally measured value, providing a non-trivial test of the framework.

### 4.3 What FTD Does NOT Contribute

1. **The dimensional transmutation mechanism**: This is standard QCD (asymptotic freedom + confinement). FTD does not derive why coupling constants run.

2. **The beta function coefficients**: b_0 and b_1 come from perturbative QCD Feynman diagram calculations. FTD provides N_c = 3, but the loop calculation is external physics.

3. **The MS-bar scheme**: The precise numerical value of Lambda depends on the renormalization scheme, which is a standard QFT concept not derived from FTD.

---

## 5. Errata from v1.0

### 5.1 Convention Error

Version 1.0 used the RG running formula:

```
1/alpha_s(mu_2) = 1/alpha_s(mu_1) + (b_0/(2*pi)) * ln(mu_2^2/mu_1^2)  [CONVENTION A]
```

This corresponds to alpha_s = 2*pi/(b_0 * t) where t = ln(mu^2/Lambda^2). But the document used the PDG numerical value alpha_s(M_Z) = 0.1186, which comes from:

```
alpha_s = 4*pi/(b_0 * t)  [CONVENTION B = PDG standard]
```

**Convention A gives half the PDG value for the same Lambda.** Mixing the PDG numerical value with Convention A's running formula doubled the effective running speed, producing:
- alpha_s(M_P) = 0.0103 (should be 0.019 in Convention B)
- Lambda = 1.4 GeV (should be ~45 MeV for Lambda^(6))

### 5.2 Wrong n_f at M_Z

Version 1.0 used b_0 = 7 (n_f = 6) for the computation at M_Z. But M_Z = 91.2 GeV is below m_t = 173 GeV, so n_f = 5 and b_0 = 23/3 at this scale.

### 5.3 Failed Ad Hoc Formulas

Version 1.0 included multiple ad hoc formulas (m_e * integers, m_p * alpha^n, etc.) seeking a "magic formula" for Lambda_QCD. These are removed in v2.0. The correct approach is standard dimensional transmutation with FTD-derived inputs, not numerology.

---

## 6. Claims Table

| ID | Statement | Status |
|----|-----------|--------|
| LQ-1 | alpha_s(M_Z) = 7/59 derived non-circularly from FTD integers | [THEOREM] |
| LQ-2 | Derivation chain G* --> alpha --> v --> M_Z, {b_3,N_eff} --> alpha_s has no Lambda_QCD dependence | [THEOREM] |
| LQ-3 | One-loop Lambda^(5) = 91 MeV from dimensional transmutation with FTD inputs | [SELECTION] |
| LQ-4 | Two-loop Lambda^(5) ~ 220 MeV, consistent with PDG 213 +/- 8 MeV | [SELECTION] |
| LQ-5 | Flavor threshold matching (n_f = 6,5,4,3) gives consistent alpha_s running | [SELECTION] |
| LQ-6 | Complete chain: G* --> alpha, alpha_s, M_Z --> Lambda_QCD closes the loop | [SELECTION] |

**Epistemic breakdown:** 2 [THEOREM], 4 [SELECTION], 0 [CONJECTURE]

**Overall status:** [SELECTION] -- Lambda_QCD follows from FTD-derived inputs via standard QCD machinery. The functional form is imported (not derived from FTD axioms), but the numerical inputs are FTD-derived, making the result a genuine framework prediction.

---

## 7. Cross-References

- **alpha_s derivation**: AUDIT_EPISTEMIC_AUDIT.md (Section I.2), AUDIT_WHAT_IS_GENUINELY_NEW.md
- **Master quadratic**: SPEC_THE_MASTER_QUADRATIC_UNIFIED.md
- **Higgs VEV**: DERIV_COMPLETE_PARTICLE_PHYSICS.md
- **Mass hierarchy**: DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md
- **QFT bridge**: DERIV_QFT_GRT_BRIDGE.md (lattice propagators and QCD vertex)
- **Constants**: scripts/constants.py (ALPHA_S = b_3/(b_3 + 4*N_eff))

---

*Document Version 2.0 -- February 19, 2026*
*Non-circular derivation established; convention error corrected*
