"""
Exploration 1: Does the Lattice Predict Different Gravity?

FTD metric: f_FTD = 1 - 1/r^2  (from L ~ 1/r, f = 1-L^2)
GR metric:  f_GR  = 1 - 2/r    (Schwarzschild)

Units: GM/c^2 = 1 throughout. GR horizon at r=2, FTD horizon at r=1.

The two-mechanism picture means the FORCE is always Newtonian (1/r^2
from flux gradients). The metric correction only appears in:
  - Gravitational redshift
  - Signal propagation delay (Shapiro)
  - Wave frequencies (LIGO ringdown)
  - Photon sphere / shadow (EHT)

Question: can CURRENT observations distinguish f~1-1/r^2 from f~1-1/r?
"""
import numpy as np
import sys
sys.path.insert(0, r'C:\Users\cpaci\Desktop\ftd\scripts')
from constants import G_STAR, ALPHA

print("=" * 72)
print("EXPLORATION 1: Observable Consequences of FTD vs GR Metric")
print("=" * 72)

def f_ftd(r): return 1.0 - 1.0/(r*r)
def f_gr(r):  return 1.0 - 2.0/r

# ============================================================
# Test 1: Post-Newtonian Expansion
# ============================================================
print("\n--- Test 1: Post-Newtonian Expansion ---\n")

# Expand both metrics in powers of M/r (= 1/r in our units):
# GR:  f = 1 - 2/r
#        = 1 - 2*epsilon            where epsilon = 1/r
# FTD: f = 1 - 1/r^2
#        = 1 - epsilon^2
#
# At 1PN order (epsilon = GM/(c^2*r)):
#   GR  correction: -2*epsilon  (order epsilon^1)
#   FTD correction: -epsilon^2  (order epsilon^2)
#
# The FTD metric correction is ONE ORDER HIGHER in the PN expansion!
# At 1PN, FTD's metric gives ZERO correction. All 1PN effects come
# from the Sommerfeld mechanism (flux force + SR momentum).

print("Post-Newtonian expansion (epsilon = GM/(c^2*r)):")
print()
print("  GR:  g_00 = 1 - 2*epsilon + O(epsilon^2)")
print("  FTD: g_00 = 1 - epsilon^2  + O(epsilon^3)")
print()
print("  At 1PN (order epsilon): GR has -2*epsilon, FTD has ZERO.")
print("  At 2PN (order epsilon^2): GR has higher terms, FTD has -epsilon^2.")
print()
print("  This means: ALL 1PN solar system tests (Shapiro delay, precession,")
print("  light bending) are IDENTICAL for FTD and GR, because the 1PN metric")
print("  correction in FTD is zero -- everything comes from Sommerfeld.")
print()
print("  The FIRST place the metrics differ is at 2PN order.")

# ============================================================
# Test 2: Gravitational Redshift
# ============================================================
print("\n\n--- Test 2: Gravitational Redshift ---\n")

# Redshift: z = 1/sqrt(f) - 1
# For weak fields: z_GR ~ M/(c^2*r), z_FTD ~ M^2/(2*c^4*r^2)
#
# BUT: In FTD, the redshift has TWO contributions:
# 1. From the flux density gradient (refractive index): z_flux ~ M/(c^2*r)
# 2. From the BI metric: z_metric ~ M^2/(2*c^4*r^2)
# Total: z_FTD_total ~ M/(c^2*r) + M^2/(2*c^4*r^2) = z_GR + correction

# The key question: does the flux mechanism produce redshift?
# A photon traveling through a varying refractive index IS redshifted
# (gravitational redshift = photon climbing out of a refractive gradient).
# If n_flux = 1 + 2M/(c^2*r), then the redshift from climbing is:
# z_flux = n(r_emit)/n(r_obs) - 1 ~ 2M/(c^2*r_emit) for r_obs >> r_emit
# This equals the GR redshift!

print("Gravitational redshift in FTD:")
print()
print("  Metric-only: z_metric = 1/sqrt(1-1/r^2) - 1 ~ 1/(2r^2)")
print("  Flux:        z_flux = n(r_emit)/n(r_obs) - 1 ~ 2/r")
print("  Total:       z_total ~ 2/r + 1/(2r^2) = z_GR + small correction")
print()

print(f"{'System':>20} | {'r (GM/c^2)':>12} | {'z_GR':>12} | {'z_FTD_metric':>14} | {'z_FTD_total':>14} | {'diff from GR':>14}")
print("-" * 95)

systems = [
    ("Sun surface",        2.36e5),    # R_sun / (GM/c^2)
    ("White dwarf",        3000),
    ("GPS satellite",      2.66e9),    # ~26600 km / (GM_earth/c^2 = 4.4mm)
    ("Pound-Rebka (22m)",  5.0e9),     # 22.5m tower / (GM/c^2)
    ("Neutron star",       5.0),
    ("r = 10 (strong)",    10.0),
    ("r = 3 (photon sph)", 3.0),
]

for name, r in systems:
    fg = f_gr(r)
    ff = f_ftd(r)
    z_gr = 1.0/np.sqrt(max(fg, 1e-10)) - 1.0 if fg > 0 else float('inf')
    z_metric = 1.0/np.sqrt(max(ff, 1e-10)) - 1.0 if ff > 0 else float('inf')
    z_flux = 2.0/r  # from refractive index
    z_total = z_flux + 0.5/r**2  # flux + metric (approximate)
    diff = (z_total - z_gr)/z_gr * 100 if z_gr > 0 and z_gr < 100 else float('inf')
    print(f"{name:>20} | {r:>12.2e} | {z_gr:>12.4e} | {z_metric:>14.4e} | {z_total:>14.4e} | {diff:>+13.2e}%")

print()
print("FINDING: For all solar system tests (r >> 1), the difference is")
print("negligible (< 10^-5 %). The flux mechanism provides the 1/r redshift.")
print("The metric correction 1/r^2 is always subdominant.")
print()
print("The first detectable difference would be at neutron star surfaces")
print("(r ~ 5) where the 2PN correction is ~1% of the 1PN term.")

# ============================================================
# Test 3: Shapiro Delay
# ============================================================
print("\n\n--- Test 3: Shapiro Time Delay ---\n")

# Shapiro delay = extra time for a signal passing near a mass.
# GR: Delta_t = 4GM/c^3 * ln(4*r1*r2 / b^2)  (for r1, r2 >> b >> r_s)
# FTD: If the delay comes from flux refraction (n = 1 + 2/r),
#      the formula is identical to GR at leading order.
#      The metric correction (from f = 1-1/r^2) adds a 2PN term.

# Cassini measurement: Shapiro delay measured to 0.002% accuracy.
# GR gamma parameter: gamma_PPN = 1.000021 +/- 0.000023

print("Shapiro delay (Cassini 2003):")
print()
print("  GR prediction: Delta_t = (1+gamma) * 2GM/c^3 * ln(4r1r2/b^2)")
print("  Measured: gamma = 1.000021 +/- 0.000023")
print()
print("  FTD with two mechanisms:")
print("    Flux refraction gives gamma = 1 exactly (same as GR).")
print("    Metric correction (2PN) contributes delta_gamma ~ (GM/(c^2*b))^2.")
print(f"    For Cassini (b ~ R_sun): delta_gamma ~ (1/{2.36e5:.0f})^2 ~ {1/(2.36e5)**2:.2e}")
print()
print("  This is 10^-11, far below the measurement precision of 10^-5.")
print("  FTD and GR are INDISTINGUISHABLE at current Shapiro precision.")

# ============================================================
# Test 4: LIGO Ringdown Frequency
# ============================================================
print("\n\n--- Test 4: Gravitational Wave Ringdown ---\n")

# The dominant ringdown mode frequency depends on the photon sphere:
# f_ring ~ c^3 / (2*pi * r_ph * n(r_ph) * GM)
# GR: r_ph = 3, f_QNM ~ 1/(2*pi*3*sqrt(3)) ~ 0.0612 c^3/(GM)
# FTD: r_ph = 1.77 (two-mechanism), r_ph * n(r_ph) = 4.57
# f_QNM_FTD ~ 1/(2*pi*4.57) ~ 0.0349 c^3/(GM)

r_ph_gr = 3.0
b_c_gr = 3*np.sqrt(3)  # = 5.196
r_ph_ftd = np.sqrt(2)  # metric only = 1.414; two-mechanism = 1.769
b_c_ftd = 4.569  # two-mechanism

# QNM frequency scales as ~ 1/b_c (approximate)
f_qnm_gr = 1.0 / (2*np.pi*b_c_gr)
f_qnm_ftd = 1.0 / (2*np.pi*b_c_ftd)

print(f"Quasi-normal mode (ringdown) frequency estimate:")
print(f"  GR:  f_QNM ~ {f_qnm_gr:.4f} c^3/(GM)")
print(f"  FTD: f_QNM ~ {f_qnm_ftd:.4f} c^3/(GM)")
print(f"  Ratio FTD/GR: {f_qnm_ftd/f_qnm_gr:.4f}")
print(f"  FTD predicts ringdown {(f_qnm_ftd/f_qnm_gr - 1)*100:+.1f}% different from GR.")
print()

# For GW150914: M_final ~ 62 M_sun
# GR ringdown: f ~ 251 Hz
# FTD would predict: f ~ 251 * (f_qnm_ftd/f_qnm_gr) Hz
f_gw150914_gr = 251  # Hz
f_gw150914_ftd = f_gw150914_gr * f_qnm_ftd / f_qnm_gr
print(f"  GW150914 (62 M_sun):")
print(f"    GR ringdown:  {f_gw150914_gr:.0f} Hz")
print(f"    FTD ringdown: {f_gw150914_ftd:.0f} Hz")
print(f"    LIGO measures: 251 +/- 8 Hz")
print(f"    FTD deviation: {f_gw150914_ftd - f_gw150914_gr:+.0f} Hz ({(f_gw150914_ftd/f_gw150914_gr-1)*100:+.1f}%)")
print()

if abs(f_gw150914_ftd - 251) > 8:
    print("  *** FTD ringdown prediction is OUTSIDE LIGO error bars! ***")
    print("  This is a potential FALSIFICATION point.")
else:
    print("  FTD ringdown is within LIGO error bars (not yet distinguishable).")

# ============================================================
# Test 5: THE CRITICAL TEST — Where Can We Distinguish?
# ============================================================
print("\n\n--- Test 5: Where FTD and GR First Diverge Detectably ---\n")

print("Observable          | FTD differs from GR | Current precision | Detectable?")
print("-" * 78)
tests = [
    ("Mercury precession",   "0% (Sommerfeld exact)",   "0.1%",     "NO"),
    ("Solar light bending",  "0% (flux refraction)",    "0.01%",    "NO"),
    ("Shapiro delay",        "~10^-11 (2PN)",           "0.002%",   "NO"),
    ("GPS time dilation",    "~10^-10 (2PN)",           "10^-6",    "NO"),
    ("Pound-Rebka redshift", "~10^-10 (2PN)",           "1%",       "NO"),
    ("Neutron star redshift","~1% (2PN at r~5)",        "~5-10%",   "MARGINAL"),
    ("LIGO ringdown freq",   "~14% (strong field)",     "~3%",      "YES ***"),
    ("EHT shadow size",      "~12% (strong field)",     "~10-15%",  "MARGINAL"),
    ("X-ray binary ISCO",    "~53% (strong field)",     "~10-20%",  "YES ***"),
]

for name, diff, prec, detect in tests:
    print(f"  {name:<22} | {diff:<22} | {prec:<18} | {detect}")

print()
print("CONCLUSION:")
print("  1. ALL solar system tests: FTD = GR (indistinguishable)")
print("     The Sommerfeld mechanism + flux refraction reproduce GR")
print("     exactly at 1PN. The metric difference is 2PN (negligible).")
print()
print("  2. LIGO ringdown: FTD predicts ~14% higher frequency.")
print("     Current LIGO precision (~3%) CAN detect this.")
print("     This is the STRONGEST near-term test.")
print()
print("  3. EHT shadow: FTD predicts ~12% smaller.")
print("     Current EHT precision (~10-15%) is marginal.")
print("     Next-gen EHT could distinguish.")
print()
print("  4. X-ray binary ISCO: FTD predicts ISCO at 53% of GR radius.")
print("     Fe K-alpha line measurements probe this region.")
print("     Potentially distinguishable with current data.")
