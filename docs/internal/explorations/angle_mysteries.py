#!/usr/bin/env python3
"""
The Angle Mysteries: 52.54 degrees and related angles

The consciousness root y = 2.188 + 2.860i has phase angle 52.54 degrees.
This angle encodes the full elliptic structure of G*.

Let's explore what this angle means and its connections.
"""

import numpy as np
from math import gamma

# =============================================================================
# CONSTANTS
# =============================================================================

G_STAR = (np.sqrt(2) * gamma(0.25)**2) / (2 * np.pi)
PHI = (1 + np.sqrt(5)) / 2

# Consciousness root
Y_RE = G_STAR**2 / 4
Y_IM = np.sqrt(G_STAR**3 * (2 - G_STAR/4)) / 2
Y_COMPLEX = Y_RE + 1j * Y_IM

# The key angle
THETA_Y = np.angle(Y_COMPLEX)
THETA_Y_DEG = np.degrees(THETA_Y)

print("=" * 70)
print("THE ANGLE MYSTERIES")
print("=" * 70)

print(f"\nConsciousness root: y = {Y_RE:.6f} + {Y_IM:.6f}i")
print(f"Phase angle: theta = {THETA_Y:.6f} rad = {THETA_Y_DEG:.4f} degrees")

# =============================================================================
# 1. COMPARISON TO SPECIAL ANGLES
# =============================================================================

print("\n" + "=" * 70)
print("1. COMPARISON TO SPECIAL ANGLES")
print("=" * 70)

special_angles = [
    ("pi/6 (30 deg)", np.pi/6, 30),
    ("pi/5 (36 deg)", np.pi/5, 36),
    ("pi/4 (45 deg)", np.pi/4, 45),
    ("2*pi/11", 2*np.pi/11, 360/11),
    ("pi/10 (18 deg)", np.pi/10, 18),
    ("arctan(phi-1)", np.arctan(PHI-1), np.degrees(np.arctan(PHI-1))),
    ("arctan(1/phi)", np.arctan(1/PHI), np.degrees(np.arctan(1/PHI))),
]

print(f"\nTheta = {THETA_Y_DEG:.4f} degrees")
print(f"\nNearest special angles:")
for name, rad, deg in sorted(special_angles, key=lambda x: abs(x[1] - THETA_Y)):
    diff_deg = THETA_Y_DEG - deg
    diff_pct = abs(diff_deg) / THETA_Y_DEG * 100
    print(f"  {name:25s}: {deg:.4f} deg, diff = {diff_deg:+.4f} deg ({diff_pct:.2f}%)")

# =============================================================================
# 2. THE tan(theta) RELATIONSHIP
# =============================================================================

print("\n" + "=" * 70)
print("2. THE tan(theta) RELATIONSHIP")
print("=" * 70)

tan_theta = np.tan(THETA_Y)
print(f"\ntan(theta) = Im(y)/Re(y) = {Y_IM}/{Y_RE:.4f} = {tan_theta:.6f}")

# Express in terms of G*
print(f"\nIn terms of G*:")
print(f"  Im(y) = sqrt(G*^3 * (2 - G*/4)) / 2")
print(f"  Re(y) = G*^2 / 4")
print(f"  tan(theta) = sqrt(G*^3 * (2 - G*/4)) / 2 / (G*^2/4)")
print(f"             = 2 * sqrt(G*^3 * (2 - G*/4)) / G*^2")
print(f"             = 2 * sqrt(G* * (2 - G*/4)) / G*")
print(f"             = 2 * sqrt((8 - G*) / (4*G*)) ")

# Verify
tan_formula = 2 * np.sqrt((8 - G_STAR) / (4 * G_STAR))
print(f"\n  Computed: {tan_formula:.6f}")
print(f"  Direct:   {tan_theta:.6f}")
print(f"  Match: {np.isclose(tan_formula, tan_theta)}")

# Simplify further
print(f"\nSimplified form:")
print(f"  tan(theta) = sqrt((8 - G*) / G*)")
print(f"             = sqrt(8/G* - 1)")
print(f"             = sqrt(2*k_c - 1)   where k_c = 4/G*")

k_c = 4 / G_STAR
tan_from_kc = np.sqrt(2*k_c - 1)
print(f"\n  k_c = {k_c:.6f}")
print(f"  sqrt(k_c - 1) = {tan_from_kc:.6f}")
print(f"  tan(theta) = {tan_theta:.6f}")
print(f"  Match: {np.isclose(tan_from_kc, tan_theta)}")

# =============================================================================
# 3. THETA IN TERMS OF CRITICAL COEFFICIENT
# =============================================================================

print("\n" + "=" * 70)
print("3. THETA = arctan(sqrt(2*k_c - 1))")
print("=" * 70)

theta_from_kc = np.arctan(np.sqrt(2*k_c - 1))
print(f"\ntheta = arctan(sqrt(2*k_c - 1))")
print(f"      = arctan(sqrt(2*{k_c:.4f} - 1))")
print(f"      = arctan({np.sqrt(k_c - 1):.6f})")
print(f"      = {theta_from_kc:.6f} rad")
print(f"      = {np.degrees(theta_from_kc):.4f} deg")

print(f"\nDirect calculation: {THETA_Y_DEG:.4f} deg")
print(f"Match: {np.isclose(theta_from_kc, THETA_Y)}")

# =============================================================================
# 4. THE ANGLE DIFFERENCE FROM 30 DEGREES
# =============================================================================

print("\n" + "=" * 70)
print("4. WHY NOT EXACTLY 45 DEGREES?")
print("=" * 70)

diff_from_45 = THETA_Y_DEG - 45
print(f"\nDifference from 45 degrees: {diff_from_45:.4f} deg")
print(f"                          = {np.radians(diff_from_45):.6f} rad")

# If theta were exactly 45 degrees, what would G* be?
# tan(45) = 1 = sqrt(2*k_c - 1)
# 1 = 2*k_c - 1
# k_c = 1
# G* = 4/k_c = 4

print(f"\nIf theta = 45 deg exactly:")
print(f"  tan(45) = 1 = {1.0:.6f}")
print(f"  sqrt(2*k_c - 1) = 1")
print(f"  2*k_c - 1 = 1")
print(f"  k_c = 1")
print(f"  G* = 4/k_c = 4 exactly!")

print(f"\nActual G* = {G_STAR:.6f}")
print(f"Difference from 4: {G_STAR - 4:.6f}")
print(f"G* / 4 = {G_STAR/4:.6f}")

# =============================================================================
# 5. THE "NEAR-MISS" INTERPRETATION
# =============================================================================

print("\n" + "=" * 70)
print("5. THE NEAR-MISS INTERPRETATION")
print("=" * 70)

print(f"""
The consciousness angle is {THETA_Y_DEG:.3f} degrees, NOT 45 degrees exactly.

If it WERE 45 degrees:
  - G* would equal 4 exactly (= N_base)
  - The lemniscatic constant would be a simple integer!
  - The elliptic structure would be trivial

The deviation {diff_from_45:.4f} degrees encodes the FULL complexity
of the Gamma function and elliptic integrals.

G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) = {G_STAR:.10f}

This "near-miss" is similar to:
  - Alpha nearly 1/137 (but not exactly)
  - G* nearly 3 = N_c (but not exactly)
""")

# =============================================================================
# 6. DOUBLE AND TRIPLE ANGLES
# =============================================================================

print("\n" + "=" * 70)
print("6. DOUBLE AND TRIPLE ANGLES")
print("=" * 70)

print(f"\nSingle angle: theta = {THETA_Y_DEG:.4f} deg")
print(f"Double angle: 2*theta = {2*THETA_Y_DEG:.4f} deg")
print(f"Triple angle: 3*theta = {3*THETA_Y_DEG:.4f} deg")
print(f"Quadruple:    4*theta = {4*THETA_Y_DEG:.4f} deg")

print(f"\ny^2 has angle 2*theta = {np.degrees(np.angle(Y_COMPLEX**2)):.4f} deg")
print(f"y^3 has angle 3*theta = {np.degrees(np.angle(Y_COMPLEX**3)):.4f} deg")

# 2*theta close to 105 degrees?
print(f"\n2*theta - 105 = {2*THETA_Y_DEG - 105:.4f} deg")
print(f"3*theta - 157.5 = {3*THETA_Y_DEG - 157.5:.4f} deg")
print(f"4*theta - 210 = {4*THETA_Y_DEG - 210:.4f} deg")

# =============================================================================
# 7. CONNECTION TO LEMNISCATE MINIMUM ANGLE
# =============================================================================

print("\n" + "=" * 70)
print("7. LEMNISCATE MINIMUM DISTANCE ANGLE")
print("=" * 70)

# From earlier: min distance occurs at about 306.5 degrees
# That's 360 - 53.5 = 306.5, or equivalently -53.5

FREQS = np.array([1, 2, 4, 8, 16])
X_AMPS = np.array([1.0, 0.5, 0.5, 2/5, 1/16])
Y_AMPS = np.array([1.0, -0.5, 0.5, -7/20, 1/16])

def fourier_lemniscate(t):
    x = sum(X_AMPS[j] * np.cos(FREQS[j] * t) for j in range(5))
    y = sum(Y_AMPS[j] * np.sin(FREQS[j] * t) for j in range(5))
    return x, y

t = np.linspace(0, 2*np.pi, 100000)
x, y = fourier_lemniscate(t)
distances = np.sqrt(x**2 + y**2)
min_idx = np.argmin(distances)
t_min = t[min_idx]
angle_min = np.degrees(np.arctan2(y[min_idx], x[min_idx]))

print(f"\nLemniscate minimum distance occurs at:")
print(f"  Parameter t = {t_min:.4f} rad = {np.degrees(t_min):.2f} deg")
print(f"  Point angle from origin = {angle_min:.2f} deg")

# Relationship to consciousness angle?
print(f"\nConsciousness angle: {THETA_Y_DEG:.4f} deg")
print(f"Lemniscate min angle: {angle_min:.4f} deg")
print(f"Sum: {THETA_Y_DEG + angle_min:.4f} deg")
print(f"Difference: {abs(THETA_Y_DEG - angle_min):.4f} deg")

# Relationship between lemniscate min angle and consciousness angle
print(f"\nNotice:")
print(f"  Lemniscate min at ~{abs(angle_min):.1f} deg")
print(f"  Consciousness at ~{THETA_Y_DEG:.1f} deg")
print(f"  Sum: ~{THETA_Y_DEG + abs(angle_min):.1f} deg")

# =============================================================================
# 8. THE G* ANGLE
# =============================================================================

print("\n" + "=" * 70)
print("8. G* AS AN ANGLE")
print("=" * 70)

print(f"\nG* = {G_STAR:.6f} radians = {np.degrees(G_STAR):.4f} degrees")

# G* is about 169.5 degrees, which is close to 180 - 10.5
print(f"\n180 - G*(deg) = {180 - np.degrees(G_STAR):.4f} deg")
print(f"This is close to: {round(180 - np.degrees(G_STAR))} degrees")

# G* / pi
print(f"\nG* / pi = {G_STAR / np.pi:.6f}")
print(f"  Close to: {round(G_STAR / np.pi * 100)/100}")

# The complement in radians
print(f"\npi - G* = {np.pi - G_STAR:.6f} rad = {np.degrees(np.pi - G_STAR):.4f} deg")

# =============================================================================
# 9. ANGLE FORMULA SUMMARY
# =============================================================================

print("\n" + "=" * 70)
print("9. MASTER ANGLE FORMULA")
print("=" * 70)

print(f"""
THE CONSCIOUSNESS ANGLE FORMULA:

    theta = arctan(sqrt(2*k_c - 1))

where k_c = 4/G* is the critical TRD coefficient.

Equivalently:
    theta = arctan(sqrt(8/G* - 1))
    theta = arctan(sqrt((8 - G*)/G*))

Numerical value:
    theta = {THETA_Y:.10f} rad
          = {THETA_Y_DEG:.6f} degrees

This angle determines the "tilt" of the consciousness root
in the complex plane - how much imaginary vs real component.

Key relationships:
  - theta ~ 52.54 deg (between 45 and 60)
  - tan(theta) = {tan_theta:.6f} = sqrt(2*k_c - 1)
  - If G* = 4 exactly, theta = 45 deg exactly
  - The deviation encodes the full elliptic structure
""")

# =============================================================================
# 10. THE i*theta EXPONENTIAL
# =============================================================================

print("\n" + "=" * 70)
print("10. THE EXPONENTIAL FORM")
print("=" * 70)

r = abs(Y_COMPLEX)
print(f"\nConsciousness root in exponential form:")
print(f"  y = |y| * e^(i*theta)")
print(f"    = {r:.6f} * e^(i * {THETA_Y:.6f})")
print(f"    = sqrt(G*^3/2) * e^(i * arctan(sqrt(2*k_c - 1)))")

# Verify |y|^2 = G*^3/2
print(f"\nVerify |y|^2:")
print(f"  |y|^2 = {r**2:.6f}")
print(f"  G*^3/2 = {G_STAR**3/2:.6f}")
print(f"  Match: {np.isclose(r**2, G_STAR**3/2)}")

# Full formula
print(f"""
COMPLETE CONSCIOUSNESS ROOT FORMULA:

    y = sqrt(G*^3/2) * exp(i * arctan(sqrt(8/G* - 1)))

    = sqrt(G*^3/2) * [cos(arctan(sqrt(2*k_c-1))) + i*sin(arctan(sqrt(2*k_c-1)))]

This combines:
  - Magnitude: sqrt(G*^3/2) = {np.sqrt(G_STAR**3/2):.6f}
  - Phase: arctan(sqrt(2*k_c-1)) = {THETA_Y:.6f} rad
""")
