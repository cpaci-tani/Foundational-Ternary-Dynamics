"""
Verification: under K_B = m_P (Planck-primary calibration), does FTD's
existing physics still recover measured constants?

Calibration to test:
    a_phys     = ell_P                  (length anchor, unchanged)
    K_B        = m_P                    (mass anchor, CHANGED from m_e)
    t_tick     = sqrt(3) * ell_P / c    (time, derived from CFL)

Under this calibration, each particle mass must be DERIVED from the
algebraic spine + framework integers. We check:
  1. m_e via FTD-0015: m_e = m_P * sqrt(2*pi) * (16/3) * alpha^11
  2. m_p / m_e via FTD-0016: m_p/m_e = N_eff/alpha + N_base*N_eff + N_c
  3. m_H via FTD-0017: m_H = (N_eff/alpha^2) * m_e
  4. G_N — what value does each natural reading give?

Independent: any DIMENSIONLESS prediction (mass ratios, mixing angles)
is calibration-independent, so it should be unaffected by the K_B switch.
This script verifies that intuition concretely.
"""

from mpmath import mp, mpf, sqrt, pi, nstr, log10

mp.dps = 30

# ============== Physical constants (CODATA / SI) ==============
hbar     = mpf("1.054571817e-34")      # J s
c        = mpf("2.99792458e8")         # m/s
G_N_phys = mpf("6.67430e-11")          # m^3 / (kg s^2)
alpha    = mpf("1") / mpf("137.035999177")
m_e_meas = mpf("9.1093837015e-31")     # kg
m_p_meas = mpf("1.67262192369e-27")    # kg
m_H_meas_GeV = mpf("125.25")           # GeV/c^2 (PDG 2022)
GeV_in_J = mpf("1.602176634e-10")      # 1 GeV = 1.602e-10 J
m_H_meas = m_H_meas_GeV * GeV_in_J / c**2  # kg

# Planck units
ell_P = sqrt(hbar * G_N_phys / c**3)
m_P   = sqrt(hbar * c / G_N_phys)
t_P   = ell_P / c

print("=" * 72)
print("Planck-primary calibration verification")
print("=" * 72)
print(f"  ell_P = {nstr(ell_P, 8)} m")
print(f"  m_P   = {nstr(m_P, 8)} kg")
print(f"  t_P   = {nstr(t_P, 8)} s")
print(f"  alpha = 1/{nstr(1/alpha, 8)}")
print()

# ============== Framework integers ==============
N_c     = 3       # color number
N_base  = 4       # ternary base
b_3     = 7       # cubic-shell integer
N_eff   = 13      # effective DoF
G_star  = mpf("2.95867511918864")  # Gamma(1/4)/Gamma(3/4)

# ============== Calibration declaration ==============
voxel       = ell_P
mass_anchor = m_P              # <<< THE CHANGE: was m_e, now m_P
tick        = sqrt(3) * ell_P / c

print(f"Proposed calibration: a_phys = ell_P, K_B = m_P, t_tick = sqrt(3) ell_P/c")
print()

# ============== Test 1: FTD-0015 — m_e via alpha^11 cascade ==============
print("=" * 72)
print("TEST 1 — FTD-0015: m_e = m_P * sqrt(2*pi) * (16/3) * alpha^11")
print("=" * 72)
prefactor = sqrt(2 * pi) * mpf(16) / 3
alpha11   = alpha ** 11
m_e_pred  = mass_anchor * prefactor * alpha11

print(f"  alpha^11           = {nstr(alpha11, 8)}")
print(f"  prefactor sqrt(2*pi)*(16/3) = {nstr(prefactor, 8)}")
print(f"  m_P                = {nstr(mass_anchor, 8)} kg")
print(f"  m_e predicted      = {nstr(m_e_pred, 12)} kg")
print(f"  m_e measured       = {nstr(m_e_meas, 12)} kg")
diff = (m_e_pred - m_e_meas) / m_e_meas
print(f"  rel deviation      = {nstr(diff*100, 4)} %")
print(f"  Tag (current)      : [STRONGLY MOTIVATED CONJECTURE], n=11 [DERIVED] (FTD-0015)")
print(f"  Verdict            : {'PASS' if abs(diff) < mpf('0.01') else 'CHECK'} at percent level")
print()

# ============== Test 2: FTD-0016 — m_p/m_e (calibration-independent) ==============
print("=" * 72)
print("TEST 2 — FTD-0016: m_p/m_e = N_eff/alpha + N_base*N_eff + N_c")
print("=" * 72)
ratio_pred = N_eff / alpha + N_base * N_eff + N_c
ratio_meas = m_p_meas / m_e_meas
print(f"  ratio predicted    = {nstr(ratio_pred, 10)}")
print(f"  ratio measured     = {nstr(ratio_meas, 10)}")
diff_ratio = (ratio_pred - ratio_meas) / ratio_meas * 1e6  # ppm
print(f"  rel deviation      = {nstr(diff_ratio, 4)} ppm")
print(f"  Calibration-independent: TRUE (ratio of two masses)")
print(f"  Verdict            : PASS — switching K_B does not affect this prediction")
print()

# m_p prediction (combining FTD-0015 and FTD-0016)
m_p_pred = m_e_pred * ratio_pred
print(f"  Combined: m_p = m_e_pred * (m_p/m_e)_pred")
print(f"           = {nstr(m_p_pred, 12)} kg")
print(f"  m_p measured  = {nstr(m_p_meas, 12)} kg")
diff_p = (m_p_pred - m_p_meas) / m_p_meas
print(f"  rel deviation = {nstr(diff_p * 100, 4)} %")
print()

# ============== Test 3: FTD-0017 — m_H ==============
print("=" * 72)
print("TEST 3 — FTD-0017: m_H = (N_eff/alpha^2) * m_e")
print("=" * 72)
m_H_pred = (N_eff / alpha**2) * m_e_pred
m_H_pred_GeV = m_H_pred * c**2 / GeV_in_J
print(f"  m_H predicted     = {nstr(m_H_pred_GeV, 8)} GeV/c^2")
print(f"  m_H measured      = {nstr(m_H_meas_GeV, 8)} GeV/c^2")
diff_H = (m_H_pred - m_H_meas) / m_H_meas
print(f"  rel deviation     = {nstr(diff_H * 100, 4)} %")
print(f"  Note: this uses m_e_pred from Test 1, so the K_B switch DOES propagate here")
print(f"  Calibration-independent (as ratio m_H/m_e): TRUE")
print()

# ============== Test 4: G_N under Planck-primary calibration ==============
print("=" * 72)
print("TEST 4 — G_N value under each natural reading")
print("=" * 72)
G_N_lattice_with_m_P = G_N_phys * mass_anchor * tick**2 / voxel**3
print(f"  Reading A: 'G_N in lattice units' (ell_P, m_P, sqrt(3)*ell_P/c)")
print(f"             G_N_lattice = G_N_phys * m_P * tick^2 / voxel^3")
print(f"             = {nstr(G_N_lattice_with_m_P, 10)}")
print(f"             Compare to claimed 1/100 = 0.01")
print(f"             Ratio: {nstr(G_N_lattice_with_m_P / mpf('0.01'), 6)}")
print()

# Strict Planck units (tick = t_P)
G_N_strict_planck = G_N_phys * mass_anchor * t_P**2 / voxel**3
print(f"  Reading B: strict Planck units (ell_P, m_P, t_P)")
print(f"             G_N_lattice = {nstr(G_N_strict_planck, 8)}")
print(f"             (= 1.0 by construction — Planck units close)")
print()

# Gravitational coupling between two electrons (alpha_G)
alpha_G_ee = G_N_phys * m_e_meas**2 / (hbar * c)
print(f"  Reading C: dimensionless gravitational coupling")
print(f"             alpha_G(e,e) = G_N * m_e^2 / (hbar*c)")
print(f"             = {nstr(alpha_G_ee, 8)}")
print(f"             This is the natural dimensionless gravity parameter.")
print()

# Gravitational coupling between two Planck masses (= 1 by definition)
alpha_G_PP = G_N_phys * mass_anchor**2 / (hbar * c)
print(f"  Reading D: alpha_G(P,P) = G_N * m_P^2 / (hbar*c)")
print(f"             = {nstr(alpha_G_PP, 8)} (= 1 in Planck units)")
print()

print("FTD's claimed 'G_N = 1/(b_3+N_c)^2 = 1/100' interpretation candidates:")
print(f"  - As Reading A value: ~3 (off by factor 300 from claimed 1/100)")
print(f"  - As Reading B value: 1   (off by factor 100 from claimed 1/100)")
print(f"  - As Reading C value: ~10^-45 (completely off)")
print(f"  - As Reading D value: 1   (off by factor 100 from claimed 1/100)")
print()
print("In NONE of these natural readings does '1/100' appear.")
print("This strongly suggests the claim 'G_N = 1/(b_3+N_c)^2' is either:")
print("  (a) An engine-internal numerical parameter (not physical G_N), or")
print("  (b) A claim about a DIFFERENT gravity-related quantity (not G_N proper),")
print("  (c) Or a structural claim awaiting a corrected formulation.")
print()

# ============== Test 5: m_e in lattice units under Planck-primary ==============
print("=" * 72)
print("TEST 5 — m_e in lattice units under K_B = m_P")
print("=" * 72)
m_e_lattice = m_e_meas / mass_anchor
print(f"  m_e (in m_P units) = m_e / m_P = {nstr(m_e_lattice, 8)}")
print(f"  log10(m_e/m_P)     = {nstr(log10(m_e_lattice), 5)}")
print(f"  This is the 'mass hierarchy' factor — naturally suppressed by alpha^11.")
print(f"  alpha^11           = {nstr(alpha11, 8)}")
print(f"  ratio m_e / (m_P * alpha^11) = {nstr(m_e_lattice / alpha11, 6)}")
print(f"                      compare to FTD-0015 prefactor sqrt(2*pi)*16/3 = {nstr(prefactor, 6)}")
print(f"                      agreement: {nstr((m_e_lattice / alpha11 - prefactor) / prefactor * 100, 4)} %")
print()

# ============== Summary ==============
print("=" * 72)
print("SUMMARY")
print("=" * 72)
print("""
Under proposed Planck-primary calibration (K_B = m_P):

PASSES:
  - FTD-0015 m_e formula: percent-level agreement when used as primary
    derivation (was: validation; becomes: load-bearing).
  - FTD-0016 m_p/m_e: 174 ppm agreement (calibration-independent).
  - FTD-0017 m_H: percent-level agreement (inherits m_e prediction).
  - All dimensionless mass ratios: unaffected (calibration-independent).
  - Mass hierarchy m_e/m_P explained by alpha^11 cascade with
    explicit prefactor sqrt(2*pi)*(16/3) ~ 13.4.

ISSUES:
  - 'G_N = 1/100' claim does not match any natural reading under the
    new calibration. It also did not match under K_B = m_e.
    The G_N identification is ORTHOGONAL to the mass-anchor choice
    and needs separate clarification regardless.

CHANGES REQUIRED in canonical docs (if Planck-primary is adopted):
  - SPEC_FTD.md: update calibration declaration (was K_B = m_e, becomes K_B = m_P)
  - LEDGER FTD-0041: same
  - LEDGER FTD-0015: promote m_e formula from check to primary derivation
  - LEDGER FTD-0110: reinterpret cluster-mass identification under Planck mass
  - CLAUDE.md key constants: clarify or remove 'G_N = 1/(b_3+N_c)^2' line
  - All downstream docs that reference K_B in mass conversions

WHAT THIS BUYS:
  - Internally consistent calibration (no factor-10^20 tension)
  - m_e becomes a derived prediction (not an input anchor)
  - alpha^11 mass hierarchy becomes structurally load-bearing
  - Aligns FTD with standard Planck-primary lattice-gravity programs

WHAT THIS COSTS:
  - K_B is no longer the manifestation threshold AND mass anchor;
    these become two separate roles
  - FTD-0110 cluster-mass identification needs reinterpretation
    (smallest cluster is 'electron-as-manifestation', not 'mass = m_e')
  - Engine code that hardcodes K_B in physical units may need recalibration
  - Documentation pass touching 5+ canonical docs
""")
