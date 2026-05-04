"""
Dimensional sanity check on FTD's claimed gravitational coupling.

CLAUDE.md key constants table says:
    G_N = 1/(b_3 + N_c)^2 = 1/100 in "lattice units"

Calibration declarations (FTD-0041 / SPEC_FTD.md):
    a_phys = ℓ_P  (one voxel = one Planck length)
    t_phys = √3 · ℓ_P / c  (one tick, from CFL)
    K_B    = m_e (mass unit)

Convert the framework's G_N value through this calibration to physical units
and compare to the measured Newton constant.
"""

from mpmath import mp, mpf, sqrt, nstr

mp.dps = 30

# ─────────────────────────── Physical constants (SI) ───────────────────────────
hbar     = mpf("1.054571817e-34")   # J·s
c_light  = mpf("2.99792458e8")      # m/s
G_N_phys = mpf("6.67430e-11")       # m^3/(kg·s^2) — measured Newton constant
m_e      = mpf("9.1093837015e-31")  # kg

# Derived
ell_P    = sqrt(hbar * G_N_phys / c_light**3)   # Planck length, m
m_P      = sqrt(hbar * c_light / G_N_phys)      # Planck mass, kg
t_P      = ell_P / c_light                      # Planck time, s
print("Planck-scale references:")
print(f"  ℓ_P      = {nstr(ell_P, 8)} m")
print(f"  m_P      = {nstr(m_P, 8)} kg")
print(f"  t_P      = {nstr(t_P, 8)} s")
print()

# ─────────────────────────── FTD calibration ───────────────────────────
# Per FTD-0041:
voxel_length = ell_P                 # 1 voxel = 1 ℓ_P
tick_time    = sqrt(3) * ell_P / c_light   # 1 tick = √3 ℓ_P/c
mass_unit    = m_e                   # K_B = m_e

print("FTD calibration ladder:")
print(f"  voxel_length = ℓ_P                 = {nstr(voxel_length, 8)} m")
print(f"  tick_time    = √3 · ℓ_P / c        = {nstr(tick_time, 8)} s")
print(f"  mass_unit    = K_B = m_e           = {nstr(mass_unit, 8)} kg")
print()

# ─────────────────────────── FTD's claimed G_N ───────────────────────────
b_3 = 7
N_c = 3
G_N_claimed_lattice = mpf(1) / (b_3 + N_c)**2   # = 1/100

print(f"FTD claim: G_N (lattice units) = 1/(b_3 + N_c)^2 = 1/{(b_3+N_c)**2}")
print(f"                                                 = {nstr(G_N_claimed_lattice, 8)}")
print()

# ─────────────────────────── Convert FTD G_N to physical units ───────────────────────────
# G_N has dimensions [length^3 / (mass · time^2)].
# G_N_physical = G_N_lattice · (voxel_length)^3 / (mass_unit · tick_time^2)

G_N_FTD_physical = G_N_claimed_lattice * voxel_length**3 / (mass_unit * tick_time**2)

print("Converting FTD's claimed G_N from lattice units to SI:")
print(f"  G_N_phys_predicted = G_N_lattice · (voxel)^3 / (mass · tick^2)")
print(f"                     = {nstr(G_N_FTD_physical, 12)} m^3/(kg·s^2)")
print()
print(f"Measured G_N (SI)    = {nstr(G_N_phys, 12)} m^3/(kg·s^2)")
print()

ratio = G_N_FTD_physical / G_N_phys
log10_ratio = mp.log10(ratio)
print(f"Ratio (predicted / measured) = {nstr(ratio, 8)}")
print(f"log10(ratio)                 = {nstr(log10_ratio, 6)}")
print()

# ─────────────────────────── Diagnosis ───────────────────────────
print("=" * 72)
print("DIAGNOSIS")
print("=" * 72)
print(f"""
FTD's claim "G_N = 1/100 in lattice units" with the calibration
(a_phys = ℓ_P, K_B = m_e, t_tick = √3 ℓ_P/c) gives a physical G_N that
is off from the measured value by a factor of {nstr(ratio, 4)}
(about 10^{int(round(float(log10_ratio)))}).

This is NOT a small disagreement — it is roughly 20 orders of magnitude.
Either:
  (a) The "G_N = 1/100 in lattice units" statement in CLAUDE.md key constants
      is using a DIFFERENT unit system than the (a_phys = ℓ_P, K_B = m_e)
      calibration declared in SPEC_FTD.md / FTD-0041;
  (b) The framework's "gravitational coupling" labeled G_N is actually
      something other than physical Newton's constant — for example, an
      engine-internal numerical parameter that doesn't directly map to G_N;
  (c) The framework integer identification G_N = 1/(b_3 + N_c)^2 is wrong
      under this calibration.

The most likely interpretation is (a) or (b): the engine's gravity sector
uses an internal coupling parameter for simulation purposes that is NOT
the physical G_N under FTD's stated mass-and-length calibration.

Cross-check: if we instead use mass_unit = m_P (Planck mass) instead of m_e,
we recover G_N = 1 in Planck units (the standard result). The factor 1/100
would then be a STRUCTURAL PREFACTOR on top of natural Planck-unit gravity.
""")

mass_unit_planck = m_P
G_N_FTD_planck_units = G_N_claimed_lattice * voxel_length**3 / (mass_unit_planck * tick_time**2)
print(f"Cross-check with mass_unit = m_P (Planck mass):")
print(f"  G_N_phys_predicted = {nstr(G_N_FTD_planck_units, 8)}")
print(f"  Measured G_N       = {nstr(G_N_phys, 8)}")
print(f"  Ratio              = {nstr(G_N_FTD_planck_units / G_N_phys, 6)}")
print()
print("With Planck-mass calibration the ratio is ~1/100 — i.e. the FTD")
print("'1/100' would be saying gravity is 100× weaker than naive Planck-unit")
print("dimensional analysis predicts. That's a STRUCTURAL claim worth examining,")
print("but NOT the same thing as 'G_N = 1/100 with K_B = m_e calibration'.")
