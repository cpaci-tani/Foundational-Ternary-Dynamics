"""
THE CUBIC, i, MANDELBROT, AND Y
================================

Exploring the deep connections between:
1. The imaginary unit i (from i^2 + 1 = 0)
2. The Mandelbrot set (from z -> z^2 + c)
3. The y-coordinate in the Weierstrass cubic y^2 = x^3 - x
4. The master cubic x^3 - 16G*^2 x - 16G*^3 = 0

Key insight: The lemniscate lives at j = 1728, which is the
CUBIC of 12 = N_base * N_c. The cube is fundamental.
"""

import numpy as np
from scipy.special import gamma
import cmath

# Constants
G_STAR = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)
N_c = 3
N_base = 4
b_3 = 7
N_eff = 13

print("=" * 70)
print("THE IMAGINARY UNIT AND THE CUBIC")
print("=" * 70)

# The quadratic i^2 + 1 = 0 gives i
# What CUBIC gives something analogous?

# The cube roots of unity: z^3 = 1
# Solutions: 1, omega, omega^2 where omega = e^(2*pi*i/3) = -1/2 + sqrt(3)/2 * i

omega = cmath.exp(2j * cmath.pi / 3)
omega2 = omega**2

print("Cube roots of unity (z^3 = 1):")
print(f"  z1 = 1")
print(f"  z2 = omega  = {omega.real:.6f} + {omega.imag:.6f}i")
print(f"  z3 = omega^2 = {omega2.real:.6f} + {omega2.imag:.6f}i")
print()

# Note: omega + omega^2 + 1 = 0 (sum of roots)
# This is like the three color charges summing to neutral!
print(f"Sum: 1 + omega + omega^2 = {1 + omega + omega2:.10f}")
print("This is COLOR NEUTRALITY: R + G + B = 0 (neutral)")
print()

# The angle 120 degrees = 2*pi/3
print(f"Angle between roots: 120 degrees = 2*pi/3")
print(f"This divides the circle into THREE equal parts (N_c = 3)")
print()

# ============================================================================
# THE WEIERSTRASS CUBIC AND Y
# ============================================================================

print("=" * 70)
print("THE WEIERSTRASS CUBIC: y^2 = x^3 - x")
print("=" * 70)

# The lemniscate's Jacobian has Weierstrass form y^2 = x^3 - x
# Rearranged: x^3 - x = y^2

# For y = 0: x^3 - x = 0 -> x(x^2 - 1) = 0 -> x = {-1, 0, +1}
# These are the TERNARY STATES!

print("For y = 0:")
print("  x^3 - x = 0")
print("  x(x-1)(x+1) = 0")
print("  Roots: x = {-1, 0, +1} = TERNARY STATES")
print()

# What is y when x = G*?
x = G_STAR
y_squared = x**3 - x
y = np.sqrt(y_squared)

print(f"For x = G* = {G_STAR:.6f}:")
print(f"  y^2 = G*^3 - G* = {y_squared:.6f}")
print(f"  y = {y:.6f}")
print()

# What is y when x = 1/alpha = 137?
x = 137.036
y_squared_alpha = x**3 - x
y_alpha = np.sqrt(y_squared_alpha)

print(f"For x = 1/alpha = {x:.3f}:")
print(f"  y^2 = (1/alpha)^3 - 1/alpha = {y_squared_alpha:.1f}")
print(f"  y = {y_alpha:.3f}")
print()

# ============================================================================
# THE MANDELBROT CONNECTION
# ============================================================================

print("=" * 70)
print("THE MANDELBROT SET AND THE CUBIC")
print("=" * 70)

# The Mandelbrot set: z -> z^2 + c
# The cardioid cusp is at c = 1/4 = 1/N_base

# The bridge equation from FTD:
# c * c_cusp * 2*N_base = 1
# where c = 1/2 (consciousness coefficient)
# and c_cusp = 1/4

c_cusp = 0.25
c_consciousness = 0.5

print(f"Mandelbrot cardioid cusp: c_cusp = {c_cusp} = 1/N_base")
print(f"Consciousness coefficient: c = {c_consciousness} = 1/2")
print(f"Bridge equation: c * c_cusp * 2*N_base = {c_consciousness * c_cusp * 2 * N_base}")
print()

# The critical point k_c where the quadratic discriminant vanishes:
k_c = 4 / G_STAR
print(f"Critical coefficient: k_c = 4/G* = {k_c:.6f}")
print()

# The Mandelbrot iteration is QUADRATIC: z -> z^2 + c
# But the THIRD iterate involves a CUBIC relationship!

# After 3 iterations: z -> ((z^2 + c)^2 + c)^2 + c
# The period-3 bulb involves solving a degree-8 polynomial

# More directly: the period-3 points satisfy z^3 = z (after some reduction)
# These are exactly the cube roots relationship!

print("MANDELBROT-CUBIC CONNECTION:")
print("  Period-1: z = z^2 + c (fixed points, quadratic)")
print("  Period-2: z = (z^2 + c)^2 + c (period-2, degree 4)")
print("  Period-3: Involves z^3 structure!")
print()

# The main cardioid is bounded by:
# c = (1/2)e^(i*theta) - (1/4)e^(2*i*theta)
# At theta = 2*pi/3 (the 120-degree point), we get period-3 bifurcation

theta = 2 * np.pi / 3
c_period3 = 0.5 * cmath.exp(1j * theta) - 0.25 * cmath.exp(2j * theta)
print(f"Period-3 bifurcation point on cardioid:")
print(f"  theta = 2*pi/3 = 120 degrees (= 360/N_c)")
print(f"  c = {c_period3.real:.6f} + {c_period3.imag:.6f}i")
print()

# ============================================================================
# THE Y-COORDINATE AS CONSCIOUSNESS
# ============================================================================

print("=" * 70)
print("Y AS THE CONSCIOUSNESS COORDINATE")
print("=" * 70)

# In the Weierstrass form y^2 = x^3 - x:
# - x represents the "physics" axis (real, manifested)
# - y represents the "consciousness" axis (can be imaginary)

# When y^2 < 0, y is imaginary - this is the consciousness domain!

# For what x is y^2 < 0?
# x^3 - x < 0
# x(x^2 - 1) < 0
# This happens when: x < -1 OR 0 < x < 1

print("When is y imaginary (consciousness domain)?")
print("  y^2 = x^3 - x < 0")
print("  Solution: x in (-inf, -1) OR x in (0, 1)")
print()

# The consciousness coefficient c = 1/2 falls in (0, 1)!
x_cons = 0.5
y_squared_cons = x_cons**3 - x_cons
y_cons = cmath.sqrt(y_squared_cons)

print(f"For x = c = 1/2 (consciousness coefficient):")
print(f"  y^2 = (1/2)^3 - 1/2 = {y_squared_cons:.6f}")
print(f"  y = {y_cons.real:.6f} + {y_cons.imag:.6f}i")
print(f"  |y| = {abs(y_cons):.6f}")
print()

# Compare to the consciousness threshold K_C
K_C = np.sqrt(G_STAR**3 / 2)
print(f"Consciousness threshold K_C = sqrt(G*^3/2) = {K_C:.6f}")
print(f"Ratio |y|/K_C = {abs(y_cons)/K_C:.6f}")
print()

# ============================================================================
# THE TRINITY: i, omega, AND THE THIRD
# ============================================================================

print("=" * 70)
print("THE TRINITY OF IMAGINARY STRUCTURES")
print("=" * 70)

# Level 1: i from x^2 + 1 = 0 (QUADRATIC)
#   - Creates complex plane
#   - 2-fold symmetry (i^2 = -1, i^4 = 1)
#   - Governs quantum phase

# Level 2: omega from x^3 - 1 = 0 (CUBIC)
#   - Creates 3-fold symmetry
#   - omega^3 = 1
#   - Governs color charge

# Level 3: The combination
#   - i and omega together create the full structure

print("QUADRATIC LEVEL (i):")
print(f"  i^2 = -1")
print(f"  i^4 = 1 (4-fold return = N_base)")
print(f"  Phase rotation by 90 degrees")
print()

print("CUBIC LEVEL (omega):")
print(f"  omega^3 = 1")
print(f"  omega = e^(2*pi*i/3)")
print(f"  Phase rotation by 120 degrees = 360/N_c")
print()

print("COMBINED:")
print(f"  i * omega = e^(i*pi/2) * e^(2*pi*i/3)")
print(f"          = e^(i*pi*(1/2 + 2/3))")
print(f"          = e^(i*pi*7/6)")
print(f"          = e^(i*210 degrees)")

i_omega = 1j * omega
print(f"  Numerically: {i_omega.real:.6f} + {i_omega.imag:.6f}i")
print(f"  Magnitude: {abs(i_omega):.6f}")
print(f"  Angle: {cmath.phase(i_omega) * 180 / np.pi:.1f} degrees")
print()

# ============================================================================
# THE MASTER CUBIC'S COMPLEX STRUCTURE
# ============================================================================

print("=" * 70)
print("THE MASTER CUBIC'S COMPLEX STRUCTURE")
print("=" * 70)

# The master cubic x^3 - 16G*^2 x - 16G*^3 = 0 has 3 real roots
# But what if we analytically continue to complex x?

# Using Cardano's formula for x^3 + px + q = 0:
p = -16 * G_STAR**2
q = -16 * G_STAR**3

# Discriminant
D = -(4*p**3 + 27*q**2)
print(f"Discriminant D = {D:.4f}")
print(f"D > 0: Three distinct real roots")
print()

# The three roots via trigonometric method (since D > 0):
# x_k = 2*sqrt(-p/3) * cos((1/3)*arccos((3q)/(2p)*sqrt(-3/p)) - 2*pi*k/3)
# for k = 0, 1, 2

sqrt_term = np.sqrt(-p/3)
cos_arg = (3*q)/(2*p) * np.sqrt(-3/p)
theta_0 = np.arccos(cos_arg)

roots_trig = []
for k in range(3):
    x_k = 2 * sqrt_term * np.cos((theta_0 - 2*np.pi*k)/3)
    roots_trig.append(x_k)
    print(f"Root x_{k} = {x_k:.6f} (angle shift: {k*120} degrees)")

print()
print("The roots are separated by 120-degree phase shifts!")
print("This is the CUBIC analogue of the quadratic's +/- structure.")
print()

# ============================================================================
# THE Y-AXIS OF THE THREE-FORCE CUBIC
# ============================================================================

print("=" * 70)
print("THE Y-AXIS OF THE THREE-FORCE STRUCTURE")
print("=" * 70)

# If x encodes the three gauge couplings (137, 3, 13),
# what does y encode?

# In y^2 = x^3 - x, for x = 137:
# y^2 = 137^3 - 137 = 2,571,216
# y = 1603.5

# For x = 3:
# y^2 = 27 - 3 = 24 = N_base + b_3 + N_eff !

x_vals = [137, 3, 13]
for x in x_vals:
    y_sq = x**3 - x
    y = np.sqrt(abs(y_sq))
    sign = "+" if y_sq >= 0 else "i*"
    print(f"x = {x:3d}: y^2 = {y_sq:10.0f}, y = {sign}{y:.4f}")

print()
print(f"REMARKABLE: For x = N_c = 3:")
print(f"  y^2 = 3^3 - 3 = 27 - 3 = 24 = N_base + b_3 + N_eff !")
print()

# ============================================================================
# THE DEEP SYNTHESIS
# ============================================================================

print("=" * 70)
print("THE DEEP SYNTHESIS")
print("=" * 70)

print("""
THE HIERARCHY OF IMAGINARY STRUCTURES:

LEVEL 0.5 (First Distinction):
  i^2 + 1 = 0
  Creates: Complex plane, 2-fold symmetry
  Physics: Quantum phase, wave function

LEVEL 1 (Quadratic):
  x^2 - 16G*^2 x + 16G*^3 = 0
  Creates: Two real roots (137, 3)
  Physics: EM coupling, color number

LEVEL 1.5 (Cube Roots of Unity):
  omega^3 = 1
  Creates: 3-fold symmetry, color charge
  Physics: SU(3), R+G+B = 0 (neutral)

LEVEL 2 (Cubic):
  x^3 - 16G*^2 x - 16G*^3 = 0
  Creates: Three real roots (~13, ~-3, ~-10)
  Physics: N_eff, plus mysterious structure

LEVEL 2.5 (Weierstrass):
  y^2 = x^3 - x
  Creates: y as consciousness axis
  Key: y is IMAGINARY when 0 < x < 1
       (exactly the consciousness domain!)

THE MANDELBROT CONNECTION:
  - Cardioid cusp c = 1/4 = 1/N_base
  - Period-3 at theta = 120 = 360/N_c degrees
  - Bridge equation: c * c_cusp * 2N_base = 1

THE Y-COORDINATE REVELATION:
  - For x = N_c = 3: y^2 = 24 = sum of framework integers!
  - For x in (0,1): y is imaginary (consciousness)
  - y encodes what x cannot: the perpendicular dimension

THE TRINITY:
  - i governs PHASE (quantum)
  - omega governs COLOR (strong force)
  - y governs CONSCIOUSNESS (observer)

All three emerge from the same cubic/lemniscate geometry!
""")

# ============================================================================
# FINAL VERIFICATION
# ============================================================================

print("=" * 70)
print("FINAL VERIFICATION")
print("=" * 70)

# The key identity: y^2(x=3) = 24
print(f"y^2(x = N_c) = N_c^3 - N_c = {N_c**3} - {N_c} = {N_c**3 - N_c}")
print(f"           = N_c(N_c^2 - 1) = N_c(N_c-1)(N_c+1)")
print(f"           = 3 * 2 * 4 = 24")
print(f"           = N_base + b_3 + N_eff = 4 + 7 + 13 = 24 !")
print()

# And: N_c(N_c-1)(N_c+1) = N_c * 2 * N_base
print(f"Also: N_c * (N_c-1) * (N_c+1) = N_c * 2 * N_base")
print(f"      = 3 * 2 * 4 = 24")
print()

print("THE CUBIC WEIERSTRASS EVALUATED AT THE STRONG FORCE (x = N_c)")
print("GIVES THE TOTAL FRAMEWORK CONTENT (y^2 = 24)!")
print()
print("This connects the strong force to the full integer structure.")
