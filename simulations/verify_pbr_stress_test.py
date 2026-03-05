#!/usr/bin/env python3
"""
verify_pbr_stress_test.py -- Deep stress test of PbR architecture.

Goes beyond the cosmological spot-checks to answer:
  1. WHERE exactly does PbR = standard physics? (algebraic proof)
  2. WHERE does PbR go BEYOND standard physics? (novel content)
  3. WHERE does PbR FAIL? (regimes where Newtonian gravity is wrong)

If PbR is "just a relabeling," it should fail everywhere Newton fails.
If PbR has genuine novel content, those equations should be identifiable.
"""

import math
import sys

# ============================================================
# CONSTANTS
# ============================================================
G_SI    = 6.67430e-11
c_SI    = 2.99792458e8
hbar_SI = 1.054571817e-34

l_P = math.sqrt(hbar_SI * G_SI / c_SI**3)
t_P = math.sqrt(hbar_SI * G_SI / c_SI**5)
m_P = math.sqrt(hbar_SI * c_SI / G_SI)
a_P = l_P / t_P**2

passed = 0
failed = 0
total  = 0


def check(name, computed, expected, tol_pct=1.0):
    global passed, failed, total
    total += 1
    if expected == 0:
        pct = abs(computed) * 100
    else:
        pct = abs(computed - expected) / abs(expected) * 100
    ok = pct <= tol_pct
    tag = "PASS" if ok else "FAIL"
    if ok:
        passed += 1
    else:
        failed += 1
    print(f"  [{tag}] {name}")
    print(f"         Computed:  {computed:.6e}")
    print(f"         Expected:  {expected:.6e}")
    print(f"         Deviation: {pct:.4f}%")
    print()


def section(title):
    print()
    print("=" * 72)
    print(f"  {title}")
    print("=" * 72)
    print()


# ============================================================
# PART 1: ALGEBRAIC IDENTITY PROOF
# ============================================================
section("PART 1: WHY THE GRAVITY SECTOR IS STANDARD PHYSICS")

print("""
  The claim is algebraic, not dismissive. Here is the proof:

  STANDARD PHYSICS (Planck natural units, G = c = hbar = 1):
  ----------------------------------------------------------
  Orbital velocity:      v = sqrt(M / r)
  Gravitational accel:   a = M / r^2
  Schwarzschild radius:  R_s = 2M
  Newton's 2nd law:      F = M * a
  Newton's gravity:      F = M_1 * M_2 / r^2

  PbR (with variable renaming M->L, r->N, t->T_G):
  ----------------------------------------------------------
  Orbital velocity:      v_N = sqrt(L / N)
  Gravitational accel:   a_N = L / N^2
  Buffer overflow:       N_crash = 2L
  Bandwidth allocation:  F_N = L * a_N
  Bandwidth override:    F_N = L_A * L_B / N^2

  These are CHARACTER-FOR-CHARACTER identical after substitution.
  This is not an approximation -- it is an algebraic identity.

  The SI verification step (multiply by Planck conversion factors)
  is ALSO standard -- it's how Planck units have always worked.

  This does NOT mean PbR is trivial. It means the gravity sector
  of PbR reproduces known physics exactly, which is a necessary
  (but not sufficient) condition for any candidate framework.
""")

# ============================================================
# PART 2: WHERE DOES NEWTONIAN GRAVITY FAIL?
# ============================================================
section("PART 2: STRESS TESTS -- REGIMES WHERE NEWTON FAILS")

print("  If PbR gravity = Newton in Planck units, PbR should fail")
print("  everywhere Newton fails. Let's test three known failures.")
print()

# ----------------------------------------------------------
# Test 2A: Mercury's perihelion precession
# ----------------------------------------------------------
print("-" * 72)
print("TEST 2A: Mercury Perihelion Precession")
print("-" * 72)
print()
print("  Newton predicts ZERO anomalous precession.")
print("  GR predicts 42.98 arcseconds/century.")
print("  Observed: 43.11 +/- 0.45 arcseconds/century.")
print()

M_sun = 1.989e30          # kg
a_merc = 5.791e10          # m (semi-major axis)
e_merc = 0.2056            # eccentricity
T_merc = 7.600e6           # s (orbital period)

# GR precession formula: delta_phi = 6*pi*G*M / (a*c^2*(1-e^2))
precession_gr = 6 * math.pi * G_SI * M_sun / (a_merc * c_SI**2 * (1 - e_merc**2))
# per orbit, in radians. Convert to arcsec/century:
orbits_per_century = 100 * 365.25 * 24 * 3600 / T_merc
precession_arcsec = precession_gr * orbits_per_century * (180/math.pi) * 3600

print(f"  GR prediction:     {precession_arcsec:.2f} arcsec/century")
print(f"  Observed:          43.11 arcsec/century")
print()

# PbR equation: a_N = L / N^2  (pure Newton, no GR correction)
# PbR predicts ZERO anomalous precession
pbr_precession = 0.0

print(f"  PbR prediction:    {pbr_precession:.2f} arcsec/century (= Newton = ZERO)")
print()
print("  RESULT: PbR gravity equation (a = L/N^2) gives Newtonian orbits.")
print("  It CANNOT produce the 43 arcsec/century precession because that")
print("  requires the r^{-3} correction from GR's Schwarzschild metric.")
print("  This is the EXPECTED failure mode if PbR gravity = Newton.")
print()

check("GR precession matches observation",
      precession_arcsec, 43.11, tol_pct=2.0)

# ----------------------------------------------------------
# Test 2B: Gravitational time dilation
# ----------------------------------------------------------
print("-" * 72)
print("TEST 2B: GPS Gravitational Time Dilation")
print("-" * 72)
print()

R_earth = 6.371e6     # m
h_gps   = 20.2e6      # m (GPS orbit altitude)
r_gps   = R_earth + h_gps
M_earth = 5.972e24    # kg

# GR: fractional time dilation = GM/(r*c^2)
dilation_surface = G_SI * M_earth / (R_earth * c_SI**2)
dilation_gps     = G_SI * M_earth / (r_gps * c_SI**2)
delta_dilation   = dilation_surface - dilation_gps  # surface runs slower

# This causes GPS clocks to gain ~45.85 microseconds/day relative to ground
usec_per_day_grav = delta_dilation * 86400 * 1e6

print(f"  GR gravitational time dilation: {usec_per_day_grav:.2f} usec/day")
print(f"  Known value:                    ~45.85 usec/day")
print()
print("  PbR has no time dilation equation in the current formulation.")
print("  The native Tick (T_G) is a universal clock -- it does NOT run")
print("  at different rates at different gravitational potentials.")
print("  This is another expected failure mode of pure Newtonian gravity.")
print()

check("GR time dilation matches known value",
      usec_per_day_grav, 45.85, tol_pct=2.0)

# ----------------------------------------------------------
# Test 2C: Gravitational lensing angle
# ----------------------------------------------------------
print("-" * 72)
print("TEST 2C: Gravitational Lensing (Light Deflection by Sun)")
print("-" * 72)
print()

# Newton predicts half the correct value
# Newton: theta = 2GM/(rc^2)
# GR:     theta = 4GM/(rc^2)
R_sun = 6.957e8  # m (solar radius)

theta_newton = 2 * G_SI * M_sun / (R_sun * c_SI**2)  # radians
theta_gr     = 4 * G_SI * M_sun / (R_sun * c_SI**2)  # radians

theta_newton_arcsec = theta_newton * (180/math.pi) * 3600
theta_gr_arcsec     = theta_gr * (180/math.pi) * 3600

print(f"  Newton prediction: {theta_newton_arcsec:.3f} arcsec")
print(f"  GR prediction:     {theta_gr_arcsec:.3f} arcsec")
print(f"  Observed (1919):   1.75 arcsec")
print()
print("  PbR gravity (a = L/N^2) is Newtonian, so it predicts the")
print("  Newtonian value of ~0.875 arcsec -- HALF the observed value.")
print("  GR's factor-of-2 enhancement comes from spacetime curvature")
print("  (the spatial part of the metric), which PbR does not have.")
print()

check("GR lensing matches observation",
      theta_gr_arcsec, 1.75, tol_pct=2.0)

# ============================================================
# PART 3: WHAT IS GENUINELY NOVEL IN PbR?
# ============================================================
section("PART 3: NOVEL PbR CONTENT (NOT STANDARD PHYSICS)")

print("""
  The gravity sector (Tests 1-3 from verify_pbr_cosmology.py) is
  standard Newtonian physics in Planck units. But PbR contains
  equations that do NOT map to any standard physics formula:

  NOVEL EQUATION 1: Auto-Commit Threshold (Decoherence)
  -----------------------------------------------------
    L_crit = N_buffer / (PF * sqrt(G*))

    - PF = pi/4 (Grid Packing Fraction) -- not in standard physics
    - G* = lemniscatic constant -- not in standard physics
    - This proposes a SPECIFIC decoherence threshold based on
      lattice geometry. Standard QM has no such formula.
    - STATUS: Genuinely novel. Requires experimental test.

  NOVEL EQUATION 2: Master Lagrangian with C_macro
  -------------------------------------------------
    delta[ (1/C_macro) * sum_T_G( |Psi_raw - N_discrete|_pm ) ] = 0

    - C_macro = 4*pi*(X+)^3 where X+ = 137.036
    - This connects the fine-structure constant to a variational
      principle over quantization errors -- not in standard physics.
    - The |Psi_raw - N_discrete| term (continuous-to-discrete
      rounding error) is a genuinely new physical quantity.
    - STATUS: Genuinely novel. No standard physics analog.

  NOVEL EQUATION 3: Packing Fraction constraint
  -----------------------------------------------
    PF = pi/4

    - Claims that spherical data writing to cubic nodes has a
      fundamental volumetric efficiency limit.
    - This would affect any process involving continuous-to-discrete
      conversion in the lattice.
    - STATUS: Geometrically motivated but needs physical consequence.

  NOVEL CONCEPT: Mass as Topological Latency
  -------------------------------------------
    Standard physics: mass is a Lorentz scalar, source of gravity.
    PbR: mass is "rounding error drag" from discretization.
    This is an INTERPRETIVE claim, not a mathematical one.
    The equations work the same either way (M -> L relabeling).
    But the interpretation could lead to novel predictions about
    mass generation mechanisms.
""")

# ============================================================
# PART 4: NUMERICAL VERIFICATION OF NOVEL CONSTANTS
# ============================================================
section("PART 4: NOVEL CONSTANT VERIFICATION")

print("-" * 72)
print("TEST 4A: C_macro = 4*pi*(X+)^3")
print("-" * 72)

X_plus = 137.035999177  # CODATA 2022 value of 1/alpha
C_macro = 4 * math.pi * X_plus**3
print(f"  X+ (1/alpha):    {X_plus:.6f}")
print(f"  C_macro:         {C_macro:.4e}")
print(f"  Gemini claimed:  ~3.23e7")
check("C_macro = 4*pi*(137.036)^3", C_macro, 3.23e7, tol_pct=1.0)

print("-" * 72)
print("TEST 4B: Grid Packing Fraction PF = pi/4")
print("-" * 72)

PF = math.pi / 4
print(f"  PF = pi/4 = {PF:.6f}")
print()
print("  Geometric interpretation: A circle inscribed in a unit square")
print("  has area pi/4. In 2D, this is the packing efficiency of a")
print("  circular object in a square cell.")
print()
print("  In 3D, sphere-in-cube packing fraction is pi/6 = 0.5236.")
print("  PbR uses pi/4 (the 2D value), not pi/6.")
print()

PF_3d = math.pi / 6
print(f"  2D circle-in-square: pi/4 = {PF:.4f}")
print(f"  3D sphere-in-cube:   pi/6 = {PF_3d:.4f}")
print(f"  PbR uses:            pi/4 = {PF:.4f} (the 2D value)")
print()

# ============================================================
# PART 5: SCALE-SPANNING CONSISTENCY
# ============================================================
section("PART 5: SCALE-SPANNING CONSISTENCY (100+ ORDERS OF MAGNITUDE)")

print("  Testing that the PbR<->SI conversion is self-consistent")
print("  across vastly different scales.")
print()

test_cases = [
    ("Proton radius",     8.414e-16,    "m",  "distance"),
    ("Hydrogen atom",     5.29e-11,     "m",  "distance"),
    ("Human height",      1.8,          "m",  "distance"),
    ("Earth radius",      6.371e6,      "m",  "distance"),
    ("1 AU",              1.496e11,     "m",  "distance"),
    ("Light-year",        9.461e15,     "m",  "distance"),
    ("Observable univ",   4.4e26,       "m",  "distance"),
    ("Electron mass",     9.109e-31,    "kg", "mass"),
    ("Proton mass",       1.673e-27,    "kg", "mass"),
    ("1 kilogram",        1.0,          "kg", "mass"),
    ("Earth mass",        5.972e24,     "kg", "mass"),
    ("Solar mass",        1.989e30,     "kg", "mass"),
    ("Milky Way mass",    1.5e42,       "kg", "mass"),
]

print(f"  {'Object':<20s} {'SI Value':>12s}  {'Native (Nodes/L)':>16s}  {'Round-trip SI':>14s}  {'Match':>6s}")
print(f"  {'-'*20} {'-'*12}  {'-'*16}  {'-'*14}  {'-'*6}")

all_roundtrip_ok = True
for name, si_val, unit, dtype in test_cases:
    if dtype == "distance":
        native = si_val / l_P
        roundtrip = native * l_P
    else:  # mass
        native = si_val / m_P
        roundtrip = native * m_P

    rel_err = abs(roundtrip - si_val) / si_val
    ok = rel_err < 1e-10
    if not ok:
        all_roundtrip_ok = False

    print(f"  {name:<20s} {si_val:>12.3e}  {native:>16.3e}  {roundtrip:>14.3e}  {'OK' if ok else 'ERR':>6s}")

print()
total += 1
if all_roundtrip_ok:
    passed += 1
    print("  [PASS] All round-trip conversions exact (within floating point)")
else:
    failed += 1
    print("  [FAIL] Some round-trip conversions have errors")
print()

# ============================================================
# PART 6: THE HONEST VERDICT
# ============================================================
section("FINAL VERDICT")

print(f"  Automated checks: {passed}/{total} passed, {failed}/{total} failed")
print()
print("""
  WHAT THE MATH SHOWS:
  ====================

  1. GRAVITY SECTOR: The five PbR gravity equations are algebraically
     identical to Newtonian gravity in Planck natural units (G=c=hbar=1).
     This is provable by direct substitution: M->L, r->N, t->T_G.
     The numerical matches in Gemini's tests are GUARANTEED by this
     identity -- they are not independent confirmations.

  2. FAILURE MODES: PbR gravity fails exactly where Newton fails:
     - No perihelion precession (needs GR's r^-3 term)
     - No gravitational time dilation (needs metric theory)
     - Half the correct light bending (needs spatial curvature)
     These failures CONFIRM the structural isomorphism with Newton.

  3. NOVEL CONTENT: PbR contains equations that are NOT in standard
     physics: the Auto-Commit Threshold, the Master Lagrangian with
     C_macro = 4*pi*(1/alpha)^3, and the Grid Packing Fraction.
     These are genuinely new proposals requiring independent validation.

  4. INTERPRETIVE LAYER: The computational vocabulary (Latency, Nodes,
     Buffer Overflow, Bandwidth) is an interpretive framework laid
     OVER the same mathematics. The interpretation that mass IS
     rounding error is philosophically distinct from standard physics,
     even though the equations are identical in the gravity sector.

  BOTTOM LINE: PbR's gravity sector is verified but not novel.
  PbR's genuinely novel content lies in the Lagrangian structure,
  the decoherence threshold, and the ontological interpretation --
  none of which were tested in the cosmological spot-checks.
""")

sys.exit(1 if failed > 0 else 0)
