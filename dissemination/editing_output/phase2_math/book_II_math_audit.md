# Book II Mathematical Audit Report

**Chapters Audited**: 2.1 through 2.7
**Audit Date**: 2026-01-10
**Auditor**: Mathematical Consistency Review

---

## Executive Summary

Book II covers particle physics at the Planck scale, voxel anatomy, the particle zoo, quantum phenomena, the Higgs mechanism, flavor physics, and the weak force. The mathematical content ranges from basic lattice definitions to complex derivations of Standard Model parameters from FTD framework integers.

**Overall Assessment**: Generally consistent notation and dimensionally sound equations, with several issues requiring attention.

---

## Chapter-by-Chapter Audit

### Chapter 2.1: The Planck Scale

**Equation Numbering**: No formal equation numbers used.

**Variable Definitions**:
- `l_P` (Planck length): Defined as "1 voxel"
- `t_P` (Planck time): Defined as "1 tick"
- `E_P` (Planck energy): Defined as "K_B"
- `K_B`: Referenced but not explicitly defined in this chapter (cross-reference to @sec-constants)

**Issues Found**:
1. **ISSUE-2.1.1**: Line 20 defines Planck energy as K_B, but K_B is the manifestation threshold (~0.511 MeV in physical units), not the Planck energy (~1.2 x 10^19 GeV). This appears to be a conceptual error or requires clarification.
   - **Recommendation**: Clarify that K_B represents the *minimum* manifestation energy, not the Planck energy scale.

2. **ISSUE-2.1.2**: Line 38 uses `[CONJECTURE]` tag appropriately.

**Notation Consistency**: Uses standard notation for position vectors (x, y, z).

**Units**: Consistent with natural units framework.

---

### Chapter 2.2: Voxel Anatomy

**Equation Numbering**: Uses display equations without numbers (lines 97, 125, 159, 164, 169, 174).

**Variable Definitions**:
- All voxel structure fields defined explicitly
- `J`, `J_x`, `J_y`, `J_z`: Flux vector components
- `rho = |J|`: Density definition (line 97)
- `L(x)`: Lag factor, referenced but not defined

**Issues Found**:
1. **ISSUE-2.2.1**: Line 105 uses notation `E = Hf` but should be `E = hf` (Planck's constant) or clarify that H is defined differently in FTD context.
   - **Recommendation**: Use consistent notation - either h (Planck constant) or explicitly define H.

2. **ISSUE-2.2.2**: Line 125 - Phase accumulation equation:
   ```
   tau <- tau + 1/L(x)
   ```
   L(x) is not defined in this chapter. Cross-reference needed.

**Dimensional Analysis**:
- Line 97: `rho = |J| = sqrt(J_x^2 + J_y^2 + J_z^2)` - Dimensionally correct

**Notation Consistency**:
- Vectors: Uses `vec{J}` notation consistently
- Partial derivatives: Standard notation in lines 159-175

---

### Chapter 2.3: The Particle Zoo

**Equation Numbering**: Several unnumbered display equations.

**Variable Definitions**:
- Mass values given in "units" (line 22-24, 34-36, etc.) - units not specified
- CKM matrix elements defined (line 245)
- PMNS matrix elements defined (line 280)

**Issues Found**:
1. **ISSUE-2.3.1**: Lines 22-24, 44-72 - Mass values given without units. The mass column shows values like "0.6", "0.511", "2.1" but doesn't specify if these are MeV, GeV, or FTD units.
   - **Recommendation**: Add unit specification to mass tables.

2. **ISSUE-2.3.2**: Line 160 - Equation `pi_1(SO(3)) = Z_2` should use proper LaTeX: `\pi_1(SO(3)) = \mathbb{Z}_2`.

3. **ISSUE-2.3.3**: Line 185-186 - Spinor transformation equation:
   ```
   SU(2) --2:1--> SO(3)
   ```
   Uses arrow notation that may not render correctly. Consider using `\xrightarrow{2:1}`.

4. **ISSUE-2.3.4**: Line 253 - Jarlskog invariant value stated as "3 x 10^-5" but later in Chapter 2.6 it's "3.1 x 10^-5" experimentally. Internal consistency needed.

5. **ISSUE-2.3.5**: Lines 265-270 - Wolfenstein parameters table shows "Error" column with percentages, but FTD values are given without uncertainties. Should clarify these are point estimates.

6. **ISSUE-2.3.6**: Lines 286-290 - PMNS mixing angles table: Formatting consistent with prior tables.

7. **ISSUE-2.3.7**: Lines 304-307 - Neutrino mass differences:
   - Dm^2_31 stated as "2.5 x 10^-3 eV^2" with "exact" match - this is an extraordinary claim requiring verification
   - Dm^2_21 shows 20% error which is more realistic

**Notation Consistency**: Generally consistent use of subscripts for matrix elements.

---

### Chapter 2.4: Quantum Phenomena

**Equation Numbering**: Several important equations without numbers.

**Variable Definitions**:
- `psi = J_x + iJ_y`: Wave function from complexified flux (line 24)
- `H_FTD`: Hilbert space (line 38)
- Born rule probability (line 64)

**Issues Found**:
1. **ISSUE-2.4.1**: Line 24 defines `psi = J_x + iJ_y` but this only uses 2 of 3 flux components. Line 28 addresses this by noting J_z is constrained by Gauss law.

2. **ISSUE-2.4.2**: Line 38 - Hilbert space definition:
   ```
   H_FTD = L^2(Lattice, C) = { psi: L -> C | sum_v |psi(v)|^2 < infinity }
   ```
   This is mathematically correct.

3. **ISSUE-2.4.3**: Lines 107-108 - Tunneling probability:
   ```
   psi(x) ~ exp(-kappa*x) where kappa = sqrt(2m(V-E)/hbar^2)
   ```
   Uses hbar but earlier chapters use natural units (hbar = 1). Consistency check needed.

4. **ISSUE-2.4.4**: Lines 113-114 - Full tunneling formula uses hbar^2 explicitly. If natural units, hbar = 1 and this should be stated.

5. **ISSUE-2.4.5**: Line 165 - Bell-CHSH inequality:
   ```
   |S| <= 2 (Bell-CHSH inequality)
   ```
   Correct statement.

6. **ISSUE-2.4.6**: Lines 173-178 - Quantum prediction for Bell parameter shows expectation value with cosine terms. The notation `<B>` should perhaps be `<S>` for consistency with line 165.

7. **ISSUE-2.4.7**: Line 203 - Uncertainty principle uses hbar:
   ```
   Delta x * Delta p >= hbar/2
   ```
   Again, natural units would have hbar = 1.

**Dimensional Analysis**:
- Tunneling equations are dimensionally correct if hbar is retained
- Uncertainty principle dimensionally correct

---

### Chapter 2.5: The Higgs Mechanism

**Equation Numbering**: No formal numbering.

**Variable Definitions**:
- `v`: Vacuum expectation value (line 64)
- `H`: Higgs field (line 57)
- `h(x)`: Higgs boson (line 60)
- Yukawa coupling `y_f` (line 100)

**Issues Found**:
1. **ISSUE-2.5.1**: Line 46 - Mexican hat potential:
   ```
   V(|J|) = -mu^2 |J|^2 + lambda |J|^4
   ```
   Uses mu^2 with negative sign, which is non-standard. Typically written as `V = -mu^2 |phi|^2 + lambda |phi|^4` where mu^2 > 0. Clarify sign convention.

2. **ISSUE-2.5.2**: Line 68 - VEV derivation:
   ```
   v = K_B / alpha * sqrt(2) = 246 GeV
   ```
   Let's check: K_B = 0.511 MeV, alpha = 1/137
   v = 0.511 MeV * 137 * sqrt(2) = 0.511 * 137 * 1.414 = 99 MeV
   This does NOT equal 246 GeV.
   **CRITICAL ERROR**: This derivation appears numerically incorrect.
   - **Recommendation**: Verify and correct the VEV derivation formula.

3. **ISSUE-2.5.3**: Line 92 - W boson mass calculation:
   ```
   m_W = gv/2 = g * 246/2 ~ 80 GeV
   ```
   This requires g ~ 0.65, which is reasonable (weak coupling).

4. **ISSUE-2.5.4**: Lines 124-128 - Higgs mass derivation:
   ```
   m_H/m_e = n_eff/alpha^2 = 13/(1/137)^2 ~ 244,000
   m_H ~ 0.511 * 244,000 keV ~ 125 GeV
   ```
   Check: 0.511 MeV * 244,000 = 124,684 MeV = 124.7 GeV
   This is approximately correct (matches 125.1 GeV to ~0.3%).

5. **ISSUE-2.5.5**: Line 151 - Electron Yukawa:
   ```
   y_e = sqrt(2) * m_e / v = sqrt(2) * 0.511 MeV / 246 GeV ~ 3 x 10^-6
   ```
   Check: 1.414 * 0.511 / 246000 = 2.94 x 10^-6 - Correct.

6. **ISSUE-2.5.6**: Line 159 - Top quark Yukawa:
   ```
   y_t = sqrt(2) * 173 GeV / 246 GeV ~ 1
   ```
   Check: 1.414 * 173 / 246 = 0.995 - Correct.

**Units**: Mixed MeV/GeV usage - generally handled correctly with conversions.

---

### Chapter 2.6: Flavor Physics

**Equation Numbering**: No formal numbering.

**Variable Definitions**:
- Four integers: N_c = 3, N_base = 4, b_3 = 7, n_eff = 13
- All mixing angles defined

**Issues Found**:
1. **ISSUE-2.6.1**: Line 35 - Weinberg angle:
   ```
   sin^2(theta_W) = N_c/n_eff = 3/13 = 0.2308
   ```
   Check: 3/13 = 0.2308 - Correct.
   Experimental: 0.2312 - Agreement 0.19% as stated.

2. **ISSUE-2.6.2**: Line 45-46 - Strong coupling:
   ```
   alpha_s = b_3/(b_3 + 4*n_eff) = 7/(7 + 52) = 7/59 = 0.1186
   ```
   Check: 7 + 4*13 = 7 + 52 = 59. 7/59 = 0.1186 - Correct.

3. **ISSUE-2.6.3**: Lines 62-64 - Cabibbo angle:
   ```
   lambda = sqrt(2 * sin^2(theta_W) * alpha_s)
          = sqrt(2 * 0.2308 * 0.1186) = 0.234
   ```
   Check: sqrt(2 * 0.2308 * 0.1186) = sqrt(0.0547) = 0.234 - Correct.

4. **ISSUE-2.6.4**: Lines 78-79 - A parameter:
   ```
   A = (|y_8|/|y_4|) / sqrt(n_eff/16) = (0.35/0.5) / sqrt(13/16) = 0.7/0.90 = 0.78
   ```
   Check: sqrt(13/16) = 0.901. 0.7/0.901 = 0.777 ~ 0.78 - Correct.
   Note: y_8 = 0.35 and y_4 = 0.5 are stated without derivation.

5. **ISSUE-2.6.5**: Lines 107-109 - Reactor angle:
   ```
   sin(theta_13) = sqrt(alpha * N_c) = sqrt((1/137) * 3) = 0.148
   theta_13 = arcsin(0.148) = 8.5 deg
   ```
   Check: sqrt(3/137) = sqrt(0.0219) = 0.148 - Correct.
   arcsin(0.148) = 8.51 degrees - Correct.

6. **ISSUE-2.6.6**: Lines 122-126 - Solar angle:
   ```
   sin^2(theta_12) = sqrt((sin^2(theta_W) * (1 - sin^2(theta_W)))/2)
                   = sqrt((3/13 * 10/13)/2) = sqrt(0.0888) = 0.298
   ```
   Check: (3/13) * (10/13) = 30/169 = 0.1775
   0.1775/2 = 0.0888
   sqrt(0.0888) = 0.298 - Correct.
   theta_12 = arcsin(sqrt(0.298)) = 33.1 degrees - Correct.

7. **ISSUE-2.6.7**: Lines 136-138 - Atmospheric angle:
   ```
   theta_23 = arctan(sqrt(a_4/a_8)) * (1 - alpha_s/2)
            = arctan(sqrt(0.5/0.375)) * 0.941 = 49.1 * 0.941 = 46.2 deg
   ```
   Check: sqrt(0.5/0.375) = sqrt(1.333) = 1.155
   arctan(1.155) = 49.1 degrees
   49.1 * 0.941 = 46.2 degrees - Correct.
   Note: a_4 = 0.5 and a_8 = 0.375 are stated without derivation.

8. **ISSUE-2.6.8**: Lines 162-166 - Jarlskog invariant:
   ```
   J = lambda^4 * (alpha/(2*pi)) * sin(2*pi/N_c) * n_eff
     = 0.234^4 * (1/137)/(2*pi) * sin(120 deg) * 13
     = 0.00300 * 0.00116 * 0.866 * 13 = 3.9 x 10^-5
   ```
   Check:
   0.234^4 = 0.00300 - Correct
   (1/137)/(2*pi) = 0.00729/6.28 = 0.00116 - Correct
   sin(120 deg) = 0.866 - Correct
   0.00300 * 0.00116 * 0.866 * 13 = 3.92 x 10^-5 - Correct.

9. **ISSUE-2.6.9**: Lines 174-175 - CKM phase:
   ```
   delta_CKM = 2*pi * b_3 / (n_eff + N_base) = 2*pi * 7 / 17 = 148 deg
   ```
   Check: 2*pi * 7/17 = 6.28 * 0.412 = 2.59 rad = 148 degrees - Correct.
   Note: Experimental CKM phase is ~68-70 degrees, not 148 degrees. This is a significant discrepancy.
   **CRITICAL**: The text claims delta_CKM = 148 degrees, but experimental value is ~68 degrees.

10. **ISSUE-2.6.10**: Lines 182-183 - Mass ratio:
    ```
    Dm^2_21/Dm^2_31 = (alpha * n_eff)/N_base = (1/137 * 13)/4 = 0.024
    ```
    Check: (13/137)/4 = 0.0949/4 = 0.024 - Correct.

**Numerical Values**: All intermediate calculations verified as correct. One critical issue with CKM phase.

---

### Chapter 2.7: The Weak Force

**Equation Numbering**: No formal numbering.

**Variable Definitions**:
- M_W, M_Z: W and Z boson masses
- G_F: Fermi coupling constant
- CKM matrix elements

**Issues Found**:
1. **ISSUE-2.7.1**: Lines 37-38 - W/Z mass ratio:
   ```
   M_W/M_Z = cos(theta_W) = sqrt(1 - sin^2(theta_W)) = sqrt(1 - 3/13) = sqrt(10/13) = 0.877
   ```
   Check: sqrt(10/13) = sqrt(0.769) = 0.877 - Correct.
   Measured: 80.4/91.2 = 0.882 - Agreement ~0.6% as stated.

2. **ISSUE-2.7.2**: Lines 49-53 - W mass formula:
   ```
   M_W = (v/2) * g = (67/(8*alpha^2)) * m_e
   M_W = (67/(8 * (1/137)^2)) * 0.511 MeV = 80.3 GeV
   ```
   Check: 67/(8 * (1/18769)) = 67 * 18769/8 = 67 * 2346 = 157,182
   157,182 * 0.511 MeV = 80.3 GeV - Correct.
   Note: The coefficient 67 appears without derivation.

3. **ISSUE-2.7.3**: Line 87 - Neutron lifetime formula:
   ```
   tau_n = 192*pi^3 / (G_F^2 * m_n^5 * |V_ud|^2 * f)
   ```
   This is the standard formula but should note that m_n^5 has unusual dimensions. The formula is typically written with (m_n*c^2)^5 or uses natural units with c=1.

4. **ISSUE-2.7.4**: Lines 206-211 - Fermi coupling derivation:
   ```
   G_F = sqrt(2) * pi * alpha / (2 * sin^2(theta_W) * M_W^2)
   ```
   Standard formula with sin^2(theta_W) and M_W. Numerically consistent.

5. **ISSUE-2.7.5**: Lines 230-233 - Z coupling to electrons:
   ```
   g_L = -1/2 - (-1)(3/13) = -1/2 + 3/13 = -7/26
   g_R = -(-1)(3/13) = 3/13
   ```
   Check g_L: -1/2 + 3/13 = -13/26 + 6/26 = -7/26 - Correct.
   Check g_R: 3/13 - Correct.

6. **ISSUE-2.7.6**: Lines 277-279 - W boson range:
   ```
   Delta x ~ hbar*c / M_W*c^2 ~ 200 MeV*fm / 80,000 MeV ~ 0.0025 fm
   ```
   Check: 200/80000 = 0.0025 fm - Correct.
   Note: Text says "10^-18 m" but 0.0025 fm = 2.5 x 10^-18 m, which is consistent.

**Units**: Consistent use of GeV for masses, proper handling of natural units.

---

## Summary of Issues

### Critical Issues (Require Immediate Correction)
| ID | Chapter | Description |
|----|---------|-------------|
| 2.1.1 | 2.1 | Planck energy incorrectly equated to K_B |
| 2.5.2 | 2.5 | VEV derivation formula numerically incorrect |
| 2.6.9 | 2.6 | CKM phase 148 degrees vs experimental 68 degrees |

### Major Issues (Should Be Addressed)
| ID | Chapter | Description |
|----|---------|-------------|
| 2.2.1 | 2.2 | H vs h notation for Planck constant |
| 2.3.1 | 2.3 | Mass units not specified in tables |
| 2.3.7 | 2.3 | "Exact" match claim for Dm^2_31 |
| 2.4.3 | 2.4 | Natural units vs explicit hbar usage |

### Minor Issues (Recommended Improvements)
| ID | Chapter | Description |
|----|---------|-------------|
| 2.2.2 | 2.2 | L(x) cross-reference needed |
| 2.3.2 | 2.3 | LaTeX formatting for pi_1 |
| 2.3.4 | 2.3 | Jarlskog value internal consistency |
| 2.4.6 | 2.4 | Notation <B> vs <S> for Bell parameter |

---

## Recommendations

1. **Establish consistent unit conventions**: Either use natural units throughout (hbar = c = 1) or explicitly include dimensional constants. Currently mixed.

2. **Add equation numbers**: For equations referenced elsewhere or derived results, formal numbering aids cross-referencing.

3. **Verify VEV derivation**: The formula v = K_B/alpha * sqrt(2) does not yield 246 GeV. Either the formula or the interpretation needs correction.

4. **Clarify CKM phase**: The derived value of 148 degrees conflicts with experimental ~68 degrees. This may be a phase convention issue (148 ~ 180 - 68 + correction) but needs explicit explanation.

5. **Add derivation sources**: Coefficients like 67, y_8 = 0.35, a_4 = 0.5 appear without justification. Reference companion materials or add derivations.
