#!/usr/bin/env python3
"""
verify_pbr_cosmology.py — Independent verification of PbR cosmological calculations.

PbR (Parsimony by Reflexion) relabels standard physics in Planck natural units
with computational vocabulary. This script verifies that:
  1. The unit conversions are correct
  2. The PbR equations produce the right numbers
  3. The claimed "flawless" matches actually hold

Each test:
  - Converts SI inputs to native PbR units (Nodes, Ticks, Latency)
  - Runs the PbR equation in native units
  - Converts the result back to SI
  - Compares against the accepted experimental/observational value
"""

import math
import sys

# ============================================================
# PLANCK UNIT CONVERSION FACTORS (CODATA 2018)
# ============================================================
# These are the "SI Translation Dictionary" from the PbR paper.

# Fundamental constants in SI
G_SI    = 6.67430e-11      # m^3 kg^-1 s^-2  (gravitational constant)
c_SI    = 2.99792458e8     # m/s              (speed of light)
hbar_SI = 1.054571817e-34  # J s              (reduced Planck constant)

# Planck units (derived)
l_P  = math.sqrt(hbar_SI * G_SI / c_SI**3)   # Planck length  ~ 1.616e-35 m
t_P  = math.sqrt(hbar_SI * G_SI / c_SI**5)   # Planck time    ~ 5.391e-44 s
m_P  = math.sqrt(hbar_SI * c_SI / G_SI)      # Planck mass    ~ 2.176e-8 kg

# Derived conversion factors
a_P  = l_P / t_P**2   # Planck acceleration (m/s^2 per native unit)

print("=" * 72)
print("PbR COSMOLOGICAL VERIFICATION SUITE")
print("=" * 72)
print()
print("Planck unit conversion factors:")
print(f"  1 Node  (N)    = {l_P:.4e} m   (Planck length)")
print(f"  1 Tick  (T_G)  = {t_P:.4e} s   (Planck time)")
print(f"  1 Latency (L)  = {m_P:.4e} kg  (Planck mass)")
print(f"  1 accel unit   = {a_P:.4e} m/s^2  (Planck acceleration)")
print(f"  c (native)     = {l_P / t_P:.6f} Nodes/Tick  (should be 1.0)")
print()

passed = 0
failed = 0
total  = 0


def check(name, computed_si, expected_si, tolerance_pct=1.0):
    """Compare computed vs expected, report pass/fail."""
    global passed, failed, total
    total += 1
    if expected_si == 0:
        pct_dev = abs(computed_si)
    else:
        pct_dev = abs(computed_si - expected_si) / abs(expected_si) * 100
    ok = pct_dev <= tolerance_pct
    status = "PASS" if ok else "FAIL"
    if ok:
        passed += 1
    else:
        failed += 1
    print(f"  [{status}] {name}")
    print(f"         Computed:  {computed_si:.4e}")
    print(f"         Expected:  {expected_si:.4e}")
    print(f"         Deviation: {pct_dev:.4f}%")
    print()


# ============================================================
# TEST 0: Verify Planck unit self-consistency
# ============================================================
print("-" * 72)
print("TEST 0: Planck Unit Self-Consistency")
print("-" * 72)

# In Planck units, G = c = hbar = 1. Let's verify our conversion factors
# satisfy G * m_P / (l_P * c^2) = 1 (i.e., G*m_P*t_P^2/l_P^3 = 1)
G_check = G_SI * m_P * t_P**2 / l_P**3
check("G in Planck units = 1", G_check, 1.0, tolerance_pct=0.01)

c_check = l_P / t_P  # should be c_SI... no, should be 1 Node/Tick
# Actually l_P / t_P = sqrt(c^5 * ...) / sqrt(c^5 * ...) ... let's compute
c_native = l_P / t_P
check("c in Planck units (l_P/t_P) = c_SI", c_native, c_SI, tolerance_pct=0.01)

# c in Planck units is 1 Node/Tick by definition of l_P and t_P
# But l_P/t_P in SI gives c_SI in m/s. The "1 Node/Tick" is the abstract statement.
print("  Note: c_native = 1.0 Nodes/Tick means 1 Planck length per 1 Planck time")
print(f"        which equals {c_native:.4e} m/s = c (speed of light)")
print()

# ============================================================
# TEST 1: Earth's Orbital Velocity Around the Sun
# ============================================================
print("-" * 72)
print("TEST 1: Earth Orbital Velocity (Stellar Hub)")
print("-" * 72)

# SI inputs
M_sun_si   = 1.989e30     # kg
r_orbit_si = 1.496e11     # m (1 AU)

# Convert to native PbR units
L_sun   = M_sun_si / m_P   # Latency units
N_orbit = r_orbit_si / l_P  # Node units

print(f"  Sun Latency (L_sun):     {L_sun:.4e} L")
print(f"  Orbital distance (N):    {N_orbit:.4e} N")

# PbR equation: v_N = sqrt(L_host / N_orbit)
v_native = math.sqrt(L_sun / N_orbit)  # Nodes/Tick

print(f"  v_native:                {v_native:.4e} Nodes/Tick")

# Convert to SI: v_SI = v_native * (l_P / t_P) = v_native * c_SI
v_si = v_native * c_SI  # m/s
v_si_kms = v_si / 1000  # km/s

print(f"  v_SI:                    {v_si:.4e} m/s = {v_si_kms:.2f} km/s")

# Accepted value
v_earth_expected = 29.78e3  # m/s

check("Earth orbital velocity", v_si, v_earth_expected, tolerance_pct=1.0)

# Cross-check: standard physics v = sqrt(GM/r)
v_standard = math.sqrt(G_SI * M_sun_si / r_orbit_si)
check("Standard physics cross-check", v_standard, v_earth_expected, tolerance_pct=1.0)

# Verify PbR and standard give the same answer
check("PbR vs standard physics match", v_si, v_standard, tolerance_pct=0.01)

# ============================================================
# TEST 2: Neutron Star Surface Gravity
# ============================================================
print("-" * 72)
print("TEST 2: Neutron Star Surface Gravity (Extreme Frame Drag)")
print("-" * 72)

# SI inputs
M_ns_si = 1.4 * M_sun_si  # kg (1.4 solar masses)
R_ns_si = 10e3             # m (10 km radius)

# Convert to native PbR units
L_ns = M_ns_si / m_P   # Latency units
N_ns = R_ns_si / l_P   # Node units

print(f"  NS Latency (L_ns):       {L_ns:.4e} L")
print(f"  NS radius (N_ns):        {N_ns:.4e} N")

# PbR equation: a_N = L_ns / N_ns^2
a_native = L_ns / N_ns**2  # Nodes/Tick^2

print(f"  a_native:                {a_native:.4e} Nodes/Tick^2")

# Convert to SI: a_SI = a_native * (l_P / t_P^2)
a_si = a_native * a_P

print(f"  a_SI:                    {a_si:.4e} m/s^2")

# Accepted value (Newtonian, which is approximate for NS but this is what's being tested)
a_ns_expected = G_SI * M_ns_si / R_ns_si**2
print(f"  Standard GM/r^2:         {a_ns_expected:.4e} m/s^2")

check("Neutron star surface gravity", a_si, a_ns_expected, tolerance_pct=0.1)

# ============================================================
# TEST 3: Sagittarius A* Schwarzschild Radius
# ============================================================
print("-" * 72)
print("TEST 3: Sagittarius A* Event Horizon (Buffer Overflow)")
print("-" * 72)

# SI inputs
M_sgra_si = 4.1e6 * M_sun_si  # kg (4.1 million solar masses)

# Convert to native PbR units
L_sgra = M_sgra_si / m_P  # Latency units

print(f"  Sgr A* Latency (L):      {L_sgra:.4e} L")

# PbR equation: N_crash = 2 * L
N_crash = 2 * L_sgra  # Node units

print(f"  N_crash (native):        {N_crash:.4e} N")

# Convert to SI
R_crash_si = N_crash * l_P  # meters

print(f"  R_crash (SI):            {R_crash_si:.4e} m")
print(f"  R_crash:                 {R_crash_si / 1e3:.2e} km")

# Accepted Schwarzschild radius: R_s = 2GM/c^2
R_s_expected = 2 * G_SI * M_sgra_si / c_SI**2

print(f"  Standard R_s:            {R_s_expected:.4e} m")

check("Sgr A* Schwarzschild radius", R_crash_si, R_s_expected, tolerance_pct=0.1)

# ============================================================
# TEST 4: Verify Gemini's intermediate values
# ============================================================
print("-" * 72)
print("TEST 4: Verify Gemini's Claimed Intermediate Values")
print("-" * 72)

# Gemini claimed specific native unit values. Let's check them.

# Test 1 intermediates
check("Gemini L_sun = 9.140e37",  L_sun,   9.140e37, tolerance_pct=1.0)
check("Gemini N_orbit = 9.257e45", N_orbit, 9.257e45, tolerance_pct=1.0)
check("Gemini v_N = 9.936e-5",    v_native, 9.936e-5, tolerance_pct=1.0)

# Test 2 intermediates
check("Gemini L_ns = 1.277e38",   L_ns,    1.277e38, tolerance_pct=1.0)
check("Gemini N_ns = 6.188e38",   N_ns,    6.188e38, tolerance_pct=1.0)
check("Gemini a_N = 3.334e-40",   a_native, 3.334e-40, tolerance_pct=5.0)

# Test 3 intermediates
check("Gemini L_sgra = 3.745e44", L_sgra,  3.745e44, tolerance_pct=1.0)
check("Gemini N_crash = 7.490e44", N_crash, 7.490e44, tolerance_pct=1.0)

# ============================================================
# TEST 5: Structural Isomorphism Check
# ============================================================
print("-" * 72)
print("TEST 5: Structural Isomorphism (PbR vs Standard Physics)")
print("-" * 72)
print()
print("  PbR equation          Standard physics (Planck units, G=c=hbar=1)")
print("  -------------------   ---------------------------------------------")
print("  v = sqrt(L/N)     =   v = sqrt(M/r)      [orbital velocity]")
print("  a = L/N^2         =   a = M/r^2           [gravitational accel]")
print("  N_crash = 2L      =   R_s = 2M            [Schwarzschild radius]")
print("  F = L*a           =   F = M*a             [Newton's 2nd law]")
print("  F = L_A*L_B/N^2   =   F = M_A*M_B/r^2    [Newton's gravity]")
print()
print("  Conclusion: Every PbR equation is standard physics in Planck units")
print("  with variable renaming: M -> L, r -> N, t -> T_G.")
print("  This is a notational isomorphism, not a new prediction.")
print()

# ============================================================
# SUMMARY
# ============================================================
print("=" * 72)
print(f"RESULTS: {passed}/{total} passed, {failed}/{total} failed")
print("=" * 72)

if failed > 0:
    print("SOME TESTS FAILED — see details above.")
    sys.exit(1)
else:
    print("ALL TESTS PASSED.")
    print()
    print("VERDICT: The PbR equations are numerically correct. They are")
    print("structurally isomorphic to standard Newtonian gravity in Planck")
    print("natural units (G=c=hbar=1) with relabeled variables:")
    print("  Mass -> Latency (L)")
    print("  Distance -> Nodes (N)")
    print("  Time -> Ticks (T_G)")
    print()
    print("The cosmological tests confirm the unit conversions are done")
    print("correctly. The numerical matches are expected because the")
    print("equations ARE standard physics — expressed in Planck units.")
    sys.exit(0)
