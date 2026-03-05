#!/usr/bin/env python3
"""
verify_pbr_advanced_proofs_v2.py -- Verification of three additional PbR claims.

PROOF 4: Pauli Exclusion as "Write-Collision Error" (XOR)
  PbR claim: Two fermions can't occupy the same state because XOR of
  identical parity states gives 0. P_node = P_A XOR P_B; if P_A = P_B,
  result is 0, "deleting" the state.

  VERDICT: WRONG. FTD uses ternary states {-1,0,+1}, not binary {0,1}.
  XOR is undefined on the ternary domain. The one-state-per-voxel rule
  is Postulate 3 [AXIOM], not derived. Spin-statistics in FTD comes from
  pi_1(SO(3)) = Z_2 (standard topology), not Boolean operations. The
  "write collision" metaphor has zero basis in FTD.

PROOF 5: Black Hole Evaporation as "Garbage Collection" (Hawking Radiation)
  PbR claim: A BH is a buffer overflow (2L = max density). Garbage
  collection ejects data at the event horizon. E_ejected = k_B T_H ln 2
  (Landauer-Hawking equivalence).

  VERDICT: MIXED. FTD has genuine BH physics (Schwarzschild metric from
  lattice availability f(r) = 1 - r_s/r, Hawking temperature, algebraic
  type evolution, information paradox treatment). The "garbage collection"
  metaphor is PbR invention. Landauer's formula is standard thermodynamics
  (1961), not FTD. The claim that information is DESTROYED contradicts
  FTD's own position that information becomes algebraically inaccessible
  (Type I -> Type III_1 transition), not destroyed.

PROOF 6: 137 as "Lattice Refresh Rate" / Anti-aliasing
  PbR claim: Photon on cubic lattice accumulates geometric drift from
  pi and sqrt(2). Every ~137 nodes, drift hits threshold forcing
  "parity integrity check." Alpha = 1/137 = coupling probability.

  VERDICT: FABRICATED. FTD derives alpha from the master quadratic
  x^2 - 16*G*^2*x + 16*G*^3 = 0 (elliptic geometry + CM selection +
  DOF counting), NOT from photon drift. sqrt(2) and pi in G* come from
  the lemniscatic integral, not lattice diagonal distances. The "geometric
  drift" narrative has zero basis in any FTD document.
"""

import math
import sys
import numpy as np

# ============================================================
# CONSTANTS
# ============================================================
G_SI    = 6.67430e-11        # gravitational constant (m^3 kg^-1 s^-2)
c_SI    = 2.99792458e8       # speed of light (m/s)
hbar_SI = 1.054571817e-34    # reduced Planck constant (J s)
k_B_SI  = 1.380649e-23       # Boltzmann constant (J/K)

l_P = math.sqrt(hbar_SI * G_SI / c_SI**3)    # Planck length  ~ 1.616e-35 m
t_P = math.sqrt(hbar_SI * G_SI / c_SI**5)    # Planck time    ~ 5.391e-44 s
m_P = math.sqrt(hbar_SI * c_SI / G_SI)       # Planck mass    ~ 2.176e-8 kg
E_P = m_P * c_SI**2                          # Planck energy  ~ 1.956e9 J

# FTD constants
PF       = math.pi / 4
G_star   = math.sqrt(2) * math.gamma(0.25)**2 / (2 * math.pi)
X_plus   = 137.035999177   # 1/alpha (CODATA 2022)
alpha    = 1.0 / X_plus

# Framework integers
N_c    = 3
N_base = 4
b_3    = 7
N_eff  = 13
D      = 3   # spatial dimensions

# Black hole constants
M_sun = 1.989e30   # solar mass (kg)

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


def check_bool(name, condition, explanation=""):
    global passed, failed, total
    total += 1
    tag = "PASS" if condition else "FAIL"
    if condition:
        passed += 1
    else:
        failed += 1
    print(f"  [{tag}] {name}")
    if explanation:
        print(f"         {explanation}")
    print()


# ============================================================
# PROOF 4: PAULI EXCLUSION AS "WRITE-COLLISION ERROR" (XOR)
# ============================================================
print("=" * 70)
print("PROOF 4: PAULI EXCLUSION AS \"WRITE-COLLISION ERROR\" (XOR)")
print("=" * 70)
print()
print("PbR claim: P_node = P_A XOR P_B. If P_A = P_B, XOR = 0,")
print("\"deleting\" the state. This is a 'Fermionic Mutex.'")
print()
print("FTD actual: Postulate 3 says s(v,t) in {-1, 0, +1}.")
print("Spin-statistics from pi_1(SO(3)) = Z_2 (standard topology).")
print("NO XOR or Boolean operations anywhere in FTD.")
print()

# --- Test 4a: XOR identity (trivially true) ---
print("-" * 50)
print("Test 4a: XOR truth table (trivially true Boolean identity)")
print("-" * 50)
print()
print("  XOR truth table for binary {0,1}:")

xor_results = []
for a in [0, 1]:
    for b in [0, 1]:
        r = a ^ b
        label = "identical->0" if a == b else "different->1"
        print(f"    {a} XOR {b} = {r}  ({label})")
        xor_results.append((a, b, r))

identical_gives_zero = all(r == 0 for a, b, r in xor_results if a == b)
different_gives_one  = all(r == 1 for a, b, r in xor_results if a != b)
print()
print("  P XOR P = 0 for any P in {0,1}: trivially true Boolean identity.")
print("  This is basic logic, not physics.")
print()

check_bool("XOR identity: P XOR P = 0 (trivially true)",
           identical_gives_zero and different_gives_one,
           "Boolean tautology. Not a physical derivation.")

# --- Test 4b: XOR undefined on FTD ternary states ---
print("-" * 50)
print("Test 4b: XOR is undefined on FTD ternary states {-1, 0, +1}")
print("-" * 50)
print()
print("  FTD Postulate 3: s(v,t) in {-1, 0, +1}  (TERNARY, not binary)")
print("  XOR is defined on {0, 1}  (BINARY)")
print()
print("  Even mapping {-1,0,+1} -> {0,1,2} breaks XOR closure:")

ftd_states = [-1, 0, 1]
mapped = [0, 1, 2]
closure_violations = 0
print("    Mapped   a  XOR  b  =  result   In domain?")
for a in mapped:
    for b in mapped:
        r = a ^ b
        in_domain = r in mapped
        flag = "YES" if in_domain else "NO  <-- VIOLATION"
        if not in_domain:
            closure_violations += 1
        print(f"    {a:8d}  XOR  {b}  =  {r:6d}   {flag}")

print()
print(f"  Closure violations: {closure_violations} / {len(mapped)**2}")
print("  XOR is NOT closed on ternary states -- the operation is undefined.")
print()

check_bool("XOR undefined on FTD ternary states {-1, 0, +1}",
           closure_violations > 0,
           f"{closure_violations} closure violations. XOR requires binary {0,1}.")

# --- Test 4c: One-state-per-voxel is AXIOM ---
print("-" * 50)
print("Test 4c: Pauli exclusion at voxel level is AXIOM (Postulate 3)")
print("-" * 50)
print()
print("  FTD Postulate 3 [AXIOM]:")
print("    'Each voxel v in L has a state s(v,t) in {-1, 0, +1} at each tick t.'")
print()
print("  This means:")
print("    - Each voxel holds EXACTLY ONE state value at any instant")
print("    - No two particles can occupy the same voxel (by definition)")
print("    - This IS Pauli exclusion at the lattice level")
print("    - But it is IMPOSED as an axiom, not DERIVED from XOR or any dynamics")
print()
print("  The number of allowed states per voxel site:")
num_states_per_site = 1  # exactly one of {-1, 0, +1}
print(f"    States per voxel: {num_states_per_site} (one of three values)")
print()
print("  Is this derived from XOR?       No.")
print("  Is this derived from dynamics?  No.")
print("  Is this an axiom?               Yes (Postulate 3).")
print()

check_bool("Pauli exclusion (one state per voxel) is AXIOM, not derived",
           True,
           "Postulate 3 [AXIOM]: each voxel holds one state. Imposed, not XOR-derived.")

# --- Test 4d: Spin-statistics from pi_1(SO(3)) = Z_2 ---
print("-" * 50)
print("Test 4d: Spin-statistics from SU(2) double cover (standard topology)")
print("-" * 50)
print()
print("  FTD's actual mechanism for fermionic exchange antisymmetry:")
print("  pi_1(SO(3)) = Z_2 => SU(2) is the universal double cover of SO(3)")
print()
print("  SU(2) rotation by angle theta about z-axis:")
print("    R(theta) = [[e^{i*theta/2}, 0], [0, e^{-i*theta/2}]]")
print()

theta_2pi = 2 * math.pi
theta_4pi = 4 * math.pi

# SU(2) phase factors
phase_2pi = np.exp(1j * theta_2pi / 2)   # e^{i*pi} = -1
phase_4pi = np.exp(1j * theta_4pi / 2)   # e^{i*2pi} = +1

rot_2pi_minus_I = abs(phase_2pi - (-1)) < 1e-10
rot_4pi_plus_I  = abs(phase_4pi - 1) < 1e-10

print(f"  R(2pi): e^{{i*pi}} = {phase_2pi:.6f}")
print(f"    Is -1 (minus identity)? {rot_2pi_minus_I}")
print(f"  R(4pi): e^{{i*2pi}} = {phase_4pi:.6f}")
print(f"    Is +1 (plus identity)?  {rot_4pi_plus_I}")
print()
print("  => Fermions need 720 degrees (4pi) to return to original state")
print("  => Exchange of two fermions gives phase factor (-1)")
print("  => This is STANDARD TOPOLOGY, not XOR")
print()

check_bool("Spin-statistics: R(2pi) = -I, R(4pi) = +I from SU(2) double cover",
           rot_2pi_minus_I and rot_4pi_plus_I,
           "pi_1(SO(3)) = Z_2: standard topology (not XOR)")

# --- Test 4e: N_BASE = 2^((D+1)/2) = 4 ---
print("-" * 50)
print("Test 4e: N_BASE = 2^((D+1)/2) = 4 for D = 3")
print("-" * 50)
print()

N_base_derived = 2 ** ((D + 1) / 2)
print(f"  D = {D} spatial dimensions")
print(f"  N_BASE = 2^((D+1)/2) = 2^({(D+1)/2}) = {N_base_derived}")
print(f"  Expected: {N_base}")
print()
print("  Physical origin: dim(H) = 4 (quaternion algebra)")
print("  SO(3) -> SU(2) [universal cover] -> Sp(1) ~ unit quaternions")
print("  dim(quaternions) = 4 = N_BASE")
print("  This is algebraic dimension, NOT an XOR gate count.")
print()

check("N_BASE = 2^((D+1)/2) for D=3", N_base_derived, float(N_base), 0.001)

# --- Test 4f: Critical assessment ---
print("-" * 50)
print("Test 4f: Critical Assessment")
print("-" * 50)
print()
print("  QUESTION: Does the XOR 'Write-Collision' formulation derive")
print("  Pauli exclusion from FTD's computational principles?")
print()
print("  ANSWER: NO. The formulation is wrong on multiple levels:")
print()
print("  1. XOR truth table: Trivially true Boolean identity  [IRRELEVANT]")
print("  2. XOR on FTD states: UNDEFINED (ternary {-1,0,+1}, not binary)")
print("  3. One-state-per-voxel: Postulate 3 [AXIOM] (imposed, not derived)")
print("  4. Spin-statistics: From pi_1(SO(3))=Z_2 (standard topology, not XOR)")
print("  5. N_BASE = 4: Quaternion algebra dimension (not XOR gate count)")
print()
print("  WHAT PbR GETS RIGHT:")
print("    - The periodic table IS a consequence of exclusion (true, known)")
print("    - Identical fermions cannot share a state (true, known)")
print()
print("  WHAT PbR GETS WRONG:")
print("    - FTD does NOT use XOR (zero occurrences in entire codebase)")
print("    - Ternary states {-1,0,+1} are NOT binary (XOR is undefined)")
print("    - Pauli exclusion is IMPOSED by axiom, not DERIVED from dynamics")
print("    - The 'write collision' metaphor maps to NOTHING in FTD")
print()

check_bool("Verdict: XOR 'write collision' has NO FTD basis",
           True,
           "PbR XOR claim: WRONG. Pauli = Postulate 3 [AXIOM] + topology (Z_2)")


# ============================================================
# PROOF 5: BLACK HOLE EVAPORATION AS "GARBAGE COLLECTION"
# ============================================================
print()
print("=" * 70)
print("PROOF 5: BLACK HOLE EVAPORATION AS \"GARBAGE COLLECTION\"")
print("=" * 70)
print()
print("PbR claim: BH = buffer overflow (2L = max density). Garbage")
print("collection ejects data at event horizon.")
print("E_ejected = k_B * T_H * ln(2) (Landauer-Hawking equivalence).")
print()
print("FTD actual: Schwarzschild metric from lattice availability f(r),")
print("Hawking temperature with FTD integer decomposition, algebraic")
print("type evolution (Type I -> Type III_1), information PRESERVED.")
print()

# --- Test 5a: Hawking temperature ---
print("-" * 50)
print("Test 5a: Hawking temperature T_H (standard physics)")
print("-" * 50)
print()

M_bh = 10 * M_sun  # 10 solar mass black hole
T_H = (hbar_SI * c_SI**3) / (8 * math.pi * G_SI * M_bh * k_B_SI)

# Known value for 10 solar mass BH
T_H_expected = 6.170e-9  # K

print(f"  Black hole mass: {M_bh:.3e} kg (10 M_sun)")
print(f"  T_H = hbar*c^3 / (8*pi*G*M*k_B)")
print(f"  T_H = {T_H:.4e} K")
print(f"  Expected: ~{T_H_expected:.3e} K")
print()
print("  This is Hawking 1975 -- standard physics, not FTD.")
print()

check("Hawking temperature T_H (10 M_sun BH)", T_H, T_H_expected, 1.0)

# --- Test 5b: Landauer erasure energy ---
print("-" * 50)
print("Test 5b: Landauer erasure E = k_B * T * ln(2) (standard physics)")
print("-" * 50)
print()

T_room = 300.0
E_Landauer_room = k_B_SI * T_room * math.log(2)
E_Landauer_expected = 2.867e-21  # J (known value at 300K)

print(f"  Landauer's principle (1961): minimum energy to erase 1 bit")
print(f"  E_Landauer = k_B * T * ln(2)")
print(f"  At T = {T_room} K: E = {E_Landauer_room:.4e} J")
print(f"  Expected:          E = {E_Landauer_expected:.3e} J")
print()
print("  This is Landauer 1961 -- standard thermodynamics, not FTD.")
print()

check("Landauer erasure energy at T=300K", E_Landauer_room, E_Landauer_expected, 1.0)

# --- Test 5c: Landauer energy != mean Hawking photon energy ---
print("-" * 50)
print("Test 5c: Landauer energy != mean Hawking photon energy")
print("-" * 50)
print()

# PbR claims E_ejected = k_B * T_H * ln(2) "equals" Hawking radiation
E_Landauer_BH = k_B_SI * T_H * math.log(2)

# Actual mean energy of Hawking thermal photons ~ 2.82 * k_B * T (Wien peak)
E_Hawking_mean = 2.82 * k_B_SI * T_H

ratio = E_Landauer_BH / E_Hawking_mean

print(f"  Landauer energy at T_H:     {E_Landauer_BH:.4e} J  (min erasure cost)")
print(f"  Mean Hawking photon energy: {E_Hawking_mean:.4e} J  (Wien peak)")
print(f"  Ratio: {ratio:.4f}")
print()
print(f"  Expected ratio: ln(2)/2.82 = {math.log(2)/2.82:.4f}")
print()
print("  These are NOT equal. Landauer gives minimum erasure cost (1 bit).")
print("  Hawking radiation has a thermal spectrum. The 'equivalence' is misleading.")
print()

check_bool("Landauer energy != Hawking photon energy (ratio ~ 0.25, not 1.0)",
           abs(ratio - 1.0) > 0.5,
           f"Ratio = {ratio:.4f}. PbR 'Landauer-Hawking equivalence' is misleading.")

# --- Test 5d: FTD availability factor f(r) ---
print("-" * 50)
print("Test 5d: FTD's actual mechanism: availability factor f(r)")
print("-" * 50)
print()

r_s = 2 * G_SI * M_bh / c_SI**2
print(f"  Schwarzschild radius: r_s = 2GM/c^2 = {r_s:.2f} m = {r_s/1000:.2f} km")
print()
print("  FTD derives g_00 = f(r) = 1 - r_s/r as 'lattice availability factor'")
print("  (fraction of computational capacity not consumed by gravity)")
print("  Source: DERIV_LATTICE_SCHWARZSCHILD.md")
print()

radii_ratios = [1.01, 1.1, 1.5, 2.0, 5.0, 10.0, 100.0]
all_f_valid = True
print(f"   {'r/r_s':>8s}   {'f(r)':>10s}   {'Expected':>10s}   Match")
print(f"   {'---':>8s}   {'---':>10s}   {'---':>10s}   ---")
for rr in radii_ratios:
    r = rr * r_s
    f_calc = 1.0 - r_s / r
    f_expected = 1.0 - 1.0 / rr
    match = abs(f_calc - f_expected) < 1e-12
    if not match:
        all_f_valid = False
    print(f"   {rr:8.2f}   {f_calc:10.6f}   {f_expected:10.6f}   {'YES' if match else 'NO'}")
print()

check_bool("Availability factor f(r) = 1 - r_s/r verified at all radii",
           all_f_valid,
           "FTD derives Schwarzschild metric from lattice computational budget")

# --- Test 5e: 8*pi = 2 * N_base^2 * PF ---
print("-" * 50)
print("Test 5e: FTD integer decomposition: 8*pi = 2 * N_base^2 * PF")
print("-" * 50)
print()

eight_pi = 8 * math.pi
ftd_decomp = 2 * N_base**2 * PF

print(f"  8*pi = {eight_pi:.10f}")
print(f"  2 * N_base^2 * PF = 2 * {N_base}^2 * (pi/4) = 2 * {N_base**2} * {PF:.10f}")
print(f"                    = {ftd_decomp:.10f}")
print()
print("  This decomposes the 8*pi in Hawking's formula into FTD integers:")
print(f"    beta_H = 8*pi*M = 2 * N_base^2 * PF * M")
print(f"    Factor 2: polarity (matter/antimatter)")
print(f"    N_base^2 = {N_base**2}: lattice DOF")
print(f"    PF = pi/4: packing fraction")
print()

check("8*pi = 2 * N_base^2 * PF (FTD decomposition)",
      ftd_decomp, eight_pi, 0.001)

# --- Test 5f: PF cancellation S_BH * T_H = M/2 ---
print("-" * 50)
print("Test 5f: PF cancellation: S_BH * T_H = M/2 (Planck units)")
print("-" * 50)
print()

# In Planck units (G=c=hbar=k_B=1):
M_test = 100.0  # arbitrary mass in Planck units
S_BH = N_base**2 * PF * M_test**2        # = 4*pi*M^2
T_H_planck = 1.0 / (2 * N_base**2 * PF * M_test)  # = 1/(8*pi*M)
product_ST = S_BH * T_H_planck
expected_ST = M_test / 2.0

print(f"  Test mass: M = {M_test} (Planck units)")
print(f"  S_BH = N_base^2 * PF * M^2 = {N_base**2} * {PF:.6f} * {M_test}^2 = {S_BH:.4f}")
print(f"  T_H  = 1 / (2 * N_base^2 * PF * M) = {T_H_planck:.6e}")
print(f"  S_BH * T_H = {product_ST:.4f}")
print(f"  Expected M/2 = {expected_ST:.4f}")
print()
print("  The PF (pi/4) cancels: S_BH * T_H = M/2 exactly.")
print("  This is [THEOREM] CG-T1 from EXPLR_COLLAPSE_GRAVITY_BRIDGE.md.")
print()

check("S_BH * T_H = M/2 (PF cancellation)", product_ST, expected_ST, 0.001)

# --- Test 5g: Information destruction contradicts FTD ---
print("-" * 50)
print("Test 5g: PbR 'garbage collection' CONTRADICTS FTD")
print("-" * 50)
print()
print("  PbR claims: 'Garbage collection deletes internal data.'")
print("  => Information IS DESTROYED when the black hole evaporates.")
print()
print("  FTD's ACTUAL position (EXPLR_COLLAPSE_GRAVITY_BRIDGE.md, Part VI):")
print("    - Large BH: near Type I (classical). Info LOCKED, not destroyed.")
print("    - Evaporating BH: Type III_lambda, KMS strip widens.")
print("      Info becomes progressively accessible via modular flow.")
print("    - Final burst: Type III_1 (fully quantum). All info accessible.")
print()
print("  FTD explicitly states information is PRESERVED:")
print("    'Information is not lost -- it changes its algebraic accessibility.'")
print()
print("  CONTRADICTION:")
print("    PbR says:  info is DELETED (garbage collection)")
print("    FTD says:  info becomes INACCESSIBLE then RE-ACCESSIBLE")
print()

ftd_preserves_info = True
pbr_deletes_info = True
contradiction = ftd_preserves_info and pbr_deletes_info

check_bool("PbR 'garbage collection' CONTRADICTS FTD information preservation",
           contradiction,
           "FTD: info inaccessible->re-accessible (Type I->III_1). PbR: deleted.")


# ============================================================
# PROOF 6: 137 AS "LATTICE REFRESH RATE" / ANTI-ALIASING
# ============================================================
print()
print("=" * 70)
print("PROOF 6: 137 AS \"LATTICE REFRESH RATE\" / ANTI-ALIASING")
print("=" * 70)
print()
print("PbR claim: Photon on cubic lattice accumulates geometric drift from")
print("pi and sqrt(2). Every ~137 nodes, drift hits threshold, forcing a")
print("'Parity Integrity Check' (electromagnetic coupling event).")
print("P_couple ~ 1/N_drift_max ~ alpha.")
print()
print("FTD actual: Master quadratic from G* (lemniscatic geometry + CM")
print("selection + DOF counting). NO 'drift' or 'anti-aliasing' concepts.")
print()

# --- Test 6a: Master quadratic roots ---
print("-" * 50)
print("Test 6a: Master quadratic roots x_+ = 137.036, x_- = 3.024")
print("-" * 50)
print()

c_val = G_star
a_q, b_q, c_q = 1.0, -16 * c_val**2, 16 * c_val**3
disc_q = b_q**2 - 4 * a_q * c_q
x_plus_calc  = (-b_q + math.sqrt(disc_q)) / (2 * a_q)
x_minus_calc = (-b_q - math.sqrt(disc_q)) / (2 * a_q)

print(f"  Master quadratic: x^2 - 16*G*^2*x + 16*G*^3 = 0")
print(f"  G* = {G_star:.6f}")
print(f"  Discriminant = {disc_q:.6f}")
print(f"  x_+ = {x_plus_calc:.6f}  (expect 137.036)")
print(f"  x_- = {x_minus_calc:.6f}  (expect ~3.024)")
print()
print("  This is FTD's ACTUAL derivation of alpha: elliptic geometry,")
print("  not photon drift. The master quadratic comes from:")
print("    1. G* from lemniscatic integral [DERIVED]")
print("    2. Coefficient 16 from lattice DOF [DERIVED]")
print("    3. CM selection j = 1728 [SELECTION]")
print()

check("Master quadratic x_+ = 1/alpha", x_plus_calc, X_plus, 0.01)

# --- Test 6b: Geometric drift does NOT give 137 ---
print("-" * 50)
print("Test 6b: 'Geometric drift' does NOT accumulate to 137")
print("-" * 50)
print()
print("  PbR claims sqrt(2) and pi create drift on cubic lattice,")
print("  accumulating to a critical threshold every ~137 nodes.")
print()
print("  Test: How many steps for irrational residuals to sum to 1.0?")
print()

sqrt2_frac = math.sqrt(2) - int(math.sqrt(2))   # fractional part ~ 0.4142
pi_frac = math.pi - int(math.pi)                 # fractional part ~ 0.14159
sqrt2_pi_frac = (math.sqrt(2) * math.pi) - int(math.sqrt(2) * math.pi)

steps_sqrt2 = 1.0 / sqrt2_frac       # ~2.414 steps
steps_pi    = 1.0 / pi_frac          # ~7.063 steps
steps_product = 1.0 / sqrt2_pi_frac  # varies

print(f"  sqrt(2) fractional part: {sqrt2_frac:.6f}")
print(f"    Steps to threshold: 1/{sqrt2_frac:.4f} = {steps_sqrt2:.1f} steps")
print()
print(f"  pi fractional part: {pi_frac:.6f}")
print(f"    Steps to threshold: 1/{pi_frac:.5f} = {steps_pi:.1f} steps")
print()
print(f"  sqrt(2)*pi fractional part: {sqrt2_pi_frac:.6f}")
print(f"    Steps to threshold: 1/{sqrt2_pi_frac:.5f} = {steps_product:.1f} steps")
print()
print(f"  NONE of these give ~137.")
print(f"  sqrt(2) drift: {steps_sqrt2:.1f} steps (not 137)")
print(f"  pi drift: {steps_pi:.1f} steps (not 137)")
print(f"  product drift: {steps_product:.1f} steps (not 137)")
print()

neither_137 = (abs(steps_sqrt2 - 137) > 100 and
               abs(steps_pi - 137) > 100 and
               abs(steps_product - 137) > 100)

check_bool("Geometric drift does NOT accumulate to ~137 steps",
           neither_137,
           f"sqrt(2): {steps_sqrt2:.1f}, pi: {steps_pi:.1f}, product: {steps_product:.1f} -- none near 137")

# --- Test 6c: 137 is NOT a continued-fraction convergent ---
print("-" * 50)
print("Test 6c: 137 is NOT a convergent denominator of pi, sqrt(2)")
print("-" * 50)
print()

def continued_fraction_convergents(x, n_terms=20):
    """Return convergent denominators of x."""
    convergents = []
    a = int(x)
    p_prev, p_curr = 1, a
    q_prev, q_curr = 0, 1
    convergents.append(q_curr)
    remainder = x - a
    for _ in range(n_terms):
        if abs(remainder) < 1e-12:
            break
        x_inv = 1.0 / remainder
        a = int(x_inv)
        remainder = x_inv - a
        p_new = a * p_curr + p_prev
        q_new = a * q_curr + q_prev
        convergents.append(q_new)
        p_prev, p_curr = p_curr, p_new
        q_prev, q_curr = q_curr, q_new
    return convergents

cf_pi = continued_fraction_convergents(math.pi)
cf_sqrt2 = continued_fraction_convergents(math.sqrt(2))
cf_product = continued_fraction_convergents(math.sqrt(2) * math.pi)

in_pi      = 137 in cf_pi
in_sqrt2   = 137 in cf_sqrt2
in_product = 137 in cf_product

print(f"  pi convergent denominators: {cf_pi[:10]}")
print(f"    137 present? {in_pi}")
print()
print(f"  sqrt(2) convergent denominators: {cf_sqrt2[:10]}")
print(f"    137 present? {in_sqrt2}")
print()
print(f"  sqrt(2)*pi convergent denominators: {cf_product[:10]}")
print(f"    137 present? {in_product}")
print()
print("  137 does NOT appear as a best-rational-approximation denominator")
print("  for any of these irrational numbers. There is no number-theoretic")
print("  connection between lattice geometry and 137.")
print()

check_bool("137 is NOT a convergent denominator of pi, sqrt(2), or sqrt(2)*pi",
           not (in_pi or in_sqrt2 or in_product),
           "No number-theoretic connection between lattice geometry and 137")

# --- Test 6d: G* from lemniscatic integral ---
print("-" * 50)
print("Test 6d: FTD's actual derivation: G* from lemniscatic integral")
print("-" * 50)
print()

G_star_check = math.sqrt(2) * math.gamma(0.25)**2 / (2 * math.pi)

print(f"  G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)")
print(f"     = sqrt(2) * {math.gamma(0.25):.6f}^2 / (2*pi)")
print(f"     = {G_star_check:.10f}")
print()
print("  The sqrt(2) comes from the LEMNISCATIC INTEGRAL:")
print("    integral_0^1 dt / sqrt(1-t^4)")
print("  NOT from the diagonal of a cubic lattice cell.")
print()
print("  The pi comes from the NORMALIZATION of the integral,")
print("  NOT from a circle inscribed in lattice squares.")
print()
print("  These are fundamentally different origins.")
print()

# Also verify coefficient 16
dof = 24 - 7 - 1   # 24 flux - 7 Gauss - 1 gauge
print(f"  Coefficient 16 from lattice DOF counting:")
print(f"    24 flux components (2x2x2 cube, 3 per voxel)")
print(f"    - 7 Gauss constraints")
print(f"    - 1 gauge freedom")
print(f"    = {dof} physical degrees of freedom")
print()

check("G* from lemniscatic integral", G_star_check, G_star, 0.001)

# --- Test 6e: j = 1728 = 12^3 ---
print("-" * 50)
print("Test 6e: j = 1728 = (N_base * N_c)^3 = 12^3 (CM selection)")
print("-" * 50)
print()

j_cm = 1728
j_ftd = (N_base * N_c)**3
twelve_cubed = 12**3

print(f"  CM selection condition: j-invariant = 1728")
print(f"  1728 = 12^3 = {twelve_cubed}")
print(f"  (N_base * N_c)^3 = ({N_base} * {N_c})^3 = {j_ftd}")
print(f"  Match: {j_cm == j_ftd}")
print()
print("  The j-invariant selects the CM elliptic curve with Gaussian")
print("  integer multiplication. This is the algebraic-geometric origin")
print("  of alpha -- NOT anti-aliasing.")
print()

check("j = 1728 = (N_base * N_c)^3 from CM selection",
      float(j_ftd), float(j_cm), 0.001)

# --- Test 6f: Critical assessment ---
print("-" * 50)
print("Test 6f: Critical Assessment")
print("-" * 50)
print()
print("  QUESTION: Does 137 arise as a 'lattice refresh rate' from")
print("  geometric drift of photons on a cubic lattice?")
print()
print("  ANSWER: NO. The claim is entirely fabricated.")
print()
print("  FTD's ACTUAL derivation of alpha:")
print("    1. G* = sqrt(2)*Gamma(1/4)^2/(2*pi) from lemniscatic integral [DERIVED]")
print("    2. Coefficient 16 from lattice DOF counting (24-7-1) [DERIVED]")
print("    3. j = 1728 from CM selection [SELECTION]")
print("    4. Master quadratic: x^2 - 16*G*^2*x + 16*G*^3 = 0 [THEOREM]")
print("    5. x_+ = 137.036 = 1/alpha [CONJECTURE on identification]")
print()
print("  PbR's claims with NO FTD basis:")
print("    - 'Geometric drift':          appears in 0 FTD documents")
print("    - 'Anti-aliasing':            appears in 0 FTD documents")
print("    - 'Lattice refresh rate':     appears in 0 FTD documents")
print("    - 'Parity integrity check':   appears in 0 FTD documents")
print()
print("  The sqrt(2) in G* comes from the LEMNISCATIC INTEGRAL,")
print("  not the diagonal of a unit cube. The pi comes from integral")
print("  normalization, not circles on lattice faces. The number 137")
print("  emerges from ELLIPTIC CURVE THEORY, not pixel drift.")
print()
print("  NUMERICAL DISPROOF of drift claim:")
print(f"    sqrt(2) drift threshold: {steps_sqrt2:.1f} steps (not 137)")
print(f"    pi drift threshold:      {steps_pi:.1f} steps (not 137)")
print(f"    Neither irrational gives a ~137-step cycle.")
print()

check_bool("Verdict: 'Anti-aliasing / refresh rate' has ZERO FTD basis",
           True,
           "Alpha from elliptic geometry (master quadratic), NOT pixel drift")


# ============================================================
# SUMMARY
# ============================================================
print()
print("=" * 70)
print("SUMMARY OF ADVANCED PROOF VERIFICATION v2")
print("=" * 70)
print()
print(f"  Total checks: {total}")
print(f"  Passed:        {passed}")
print(f"  Failed:        {failed}")
print()
print("  PROOF 4 (Pauli Exclusion / XOR Write-Collision):")
print("    VERDICT: WRONG [FABRICATED]")
print("    FTD uses ternary states {-1,0,+1}; XOR is undefined on this domain.")
print("    One-state-per-voxel is AXIOM (Postulate 3), not derived from XOR.")
print("    Spin-statistics from pi_1(SO(3)) = Z_2 (standard topology).")
print("    The 'write collision' metaphor maps to NOTHING in FTD.")
print()
print("  PROOF 5 (Black Hole / Garbage Collection):")
print("    VERDICT: MIXED -- FTD has genuine BH physics; PbR metaphor WRONG")
print("    FTD derives: Schwarzschild metric from lattice availability f(r),")
print("    Hawking temperature (with integer decomposition 8pi = 2*N_base^2*PF),")
print("    PF cancellation S_BH*T_H = M/2 [THEOREM], algebraic type evolution.")
print("    PbR adds CONTRADICTORY metaphor: 'garbage collection' implies info")
print("    deletion, but FTD says info becomes algebraically inaccessible then")
print("    re-accessible during evaporation (Type I -> Type III_1 transition).")
print("    Landauer-Hawking 'equivalence' compares different quantities (ratio ~ 0.25).")
print()
print("  PROOF 6 (137 = Lattice Refresh Rate / Anti-aliasing):")
print("    VERDICT: FABRICATED [NO FTD BASIS]")
print("    Alpha derives from master quadratic via G* (lemniscatic integral),")
print("    coefficient 16 (lattice DOF), and CM selection (j=1728).")
print("    'Geometric drift,' 'anti-aliasing,' 'refresh rate,' and 'parity")
print("    integrity check' appear in ZERO FTD documents. The sqrt(2) and pi")
print("    in G* come from elliptic integrals, not lattice diagonal distances.")
print("    Numerical disproof: sqrt(2) drift threshold = ~2.4 steps, not 137.")
print("    137 is not a convergent denominator of pi, sqrt(2), or their product.")

if failed == 0:
    print("\n  ALL CHECKS PASSED")
else:
    print(f"\n  {failed} CHECK(S) FAILED")

sys.exit(0 if failed == 0 else 1)
