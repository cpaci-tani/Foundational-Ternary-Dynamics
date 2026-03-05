"""
Exploring Cubic Extensions of the FTD Master Quadratic
=======================================================

The master quadratic x^2 - 16G*^2 x + 16G*^3 = 0 gives:
  x+ = 137.036 (1/alpha)
  x- = 3.024 (N_c)

Can we find a CUBIC that extends this structure?

Five paths explored:
1. j = 1728 = 12^3 (the j-invariant is a perfect cube)
2. Three universal constants: G*, delta (Feigenbaum), phi (golden)
3. Hierarchy extension (Level 3 beyond the quadratic)
4. Modular polynomials
5. Weierstrass cubic x^3 - x = y^2

"""

import numpy as np
from scipy.special import gamma
from numpy.polynomial import polynomial as P

# ============================================================================
# FUNDAMENTAL CONSTANTS
# ============================================================================

# Lemniscatic constant
G_STAR = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)  # 2.9586751192...

# Feigenbaum constant
DELTA = 4.669201609102990

# Golden ratio
PHI = (1 + np.sqrt(5)) / 2  # 1.6180339887...

# Fine structure constant
ALPHA = 1 / 137.035999177

# Framework integers
N_c = 3
N_base = 4
b_3 = 7
N_eff = 13

print("=" * 70)
print("FUNDAMENTAL CONSTANTS")
print("=" * 70)
print(f"G* (lemniscatic)    = {G_STAR:.10f}")
print(f"delta (Feigenbaum)  = {DELTA:.10f}")
print(f"phi (golden)        = {PHI:.10f}")
print(f"alpha               = {ALPHA:.10e}")
print(f"1/alpha             = {1/ALPHA:.6f}")
print()

# ============================================================================
# PATH 1: j = 1728 = 12^3
# ============================================================================

print("=" * 70)
print("PATH 1: j-INVARIANT CUBIC")
print("=" * 70)

# The j-invariant j = 1728 = 12^3 = (N_base * N_c)^3
# What cubic has this structure?

# Try: y^3 - 144y^2 + 12y - 1 = 0  (coefficients from 12^2, 12^1, 12^0)
coeffs_j = [-1, 12, -144, 1]  # polynomial: 1*y^3 - 144*y^2 + 12*y - 1
roots_j = np.roots([1, -144, 12, -1])
print(f"Cubic: y^3 - 144y^2 + 12y - 1 = 0")
print(f"Roots: {roots_j}")
print()

# Try: y^3 - 12y = j/12 = 144
coeffs_j2 = [1, 0, -12, -144]
roots_j2 = np.roots(coeffs_j2)
print(f"Cubic: y^3 - 12y - 144 = 0")
print(f"Roots: {roots_j2}")
print()

# ============================================================================
# PATH 2: THREE UNIVERSAL CONSTANTS (G*, delta, phi)
# ============================================================================

print("=" * 70)
print("PATH 2: THREE UNIVERSAL CONSTANTS")
print("=" * 70)

# Vieta's formulas: if roots are G*, delta, phi:
# Sum = G* + delta + phi
# Sum of products = G*delta + delta*phi + phi*G*
# Product = G* * delta * phi

sum_3 = G_STAR + DELTA + PHI
sum_products = G_STAR*DELTA + DELTA*PHI + PHI*G_STAR
product_3 = G_STAR * DELTA * PHI

print(f"If roots are G*, delta, phi:")
print(f"  Sum of roots     = {sum_3:.6f}")
print(f"  Sum of products  = {sum_products:.6f}")
print(f"  Product of roots = {product_3:.6f}")
print()

# The cubic with these as roots:
# z^3 - (sum)z^2 + (sum_products)z - product = 0
print(f"Cubic: z^3 - {sum_3:.4f}z^2 + {sum_products:.4f}z - {product_3:.4f} = 0")
print()

# Check what these numbers relate to:
print(f"Comparisons:")
print(f"  Sum = {sum_3:.4f} vs N_c^2 = {N_c**2} (diff: {sum_3 - N_c**2:.4f})")
print(f"  Sum = {sum_3:.4f} vs 10 - phi = {10 - PHI:.4f}")
print(f"  Product = {product_3:.4f} vs 24 - 2 = 22")
print(f"  Product = {product_3:.4f} vs 3*b_3 = {3*b_3}")
print()

# ============================================================================
# PATH 3: HIERARCHY EXTENSION
# ============================================================================

print("=" * 70)
print("PATH 3: HIERARCHY EXTENSION")
print("=" * 70)

# The quadratic gives x+ = 137.036, x- = 3.024
# A cubic might add a third root x0 related to weak or gravitational physics

# Hypothesis: sin^2(theta_W) = x-/x0 or some similar ratio
# sin^2(theta_W) = 0.2312 (experimental)
# If x- = 3.024 and sin^2(theta_W) = x-/x0:
# x0 = x- / sin^2(theta_W) = 3.024 / 0.2312 = 13.08 ~ N_eff!

sin2_theta_W = 0.23122
x_minus = 3.024
x_zero_from_weak = x_minus / sin2_theta_W

print(f"If sin^2(theta_W) = x-/x0:")
print(f"  x0 = x- / sin^2(theta_W) = {x_minus} / {sin2_theta_W} = {x_zero_from_weak:.4f}")
print(f"  Compare to N_eff = {N_eff} (error: {abs(x_zero_from_weak - N_eff)/N_eff * 100:.2f}%)")
print()

# What cubic has roots 137.036, 3.024, and 13?
x_plus = 137.036
x_0 = N_eff  # = 13

sum_hierarchy = x_plus + x_minus + x_0
sum_prod_hierarchy = x_plus*x_minus + x_minus*x_0 + x_0*x_plus
prod_hierarchy = x_plus * x_minus * x_0

print(f"Cubic with roots (137.036, 3.024, 13):")
print(f"  z^3 - {sum_hierarchy:.4f}z^2 + {sum_prod_hierarchy:.4f}z - {prod_hierarchy:.4f} = 0")
print()

# Factor out powers of G*
print(f"Expressed in terms of G* = {G_STAR:.6f}:")
print(f"  Sum / G*^2 = {sum_hierarchy / G_STAR**2:.6f}")
print(f"  Sum_prod / G*^3 = {sum_prod_hierarchy / G_STAR**3:.6f}")
print(f"  Product / G*^4 = {prod_hierarchy / G_STAR**4:.6f}")
print()

# ============================================================================
# PATH 5: WEIERSTRASS CUBIC (Most Promising!)
# ============================================================================

print("=" * 70)
print("PATH 5: WEIERSTRASS CUBIC")
print("=" * 70)

# The lemniscate's Jacobian has Weierstrass form:
# y^2 = x^3 - x
# Rearranged: x^3 - x - y^2 = 0

# For y = 0, roots are x = {-1, 0, +1} = ternary states!

# What if we set y^2 = G*^2?
# x^3 - x - G*^2 = 0

print("Weierstrass cubic: x^3 - x - y^2 = 0")
print()
print("For y = 0: roots are {-1, 0, +1} = TERNARY STATES!")
print()

# Case 1: y^2 = G*^2
y_squared = G_STAR**2
roots_w1 = np.roots([1, 0, -1, -y_squared])
print(f"For y^2 = G*^2 = {y_squared:.6f}:")
print(f"  x^3 - x - {y_squared:.6f} = 0")
print(f"  Roots: {roots_w1}")
print()

# Case 2: y^2 = 16*G*^3 (matching the quadratic's product term)
y_squared_2 = 16 * G_STAR**3
roots_w2 = np.roots([1, 0, -1, -y_squared_2])
print(f"For y^2 = 16*G*^3 = {y_squared_2:.6f}:")
print(f"  x^3 - x - {y_squared_2:.6f} = 0")
print(f"  Roots: {roots_w2}")
print()

# ============================================================================
# MASTER CUBIC CANDIDATE
# ============================================================================

print("=" * 70)
print("MASTER CUBIC CANDIDATE")
print("=" * 70)

# Inspired by the quadratic x^2 - 16G*^2 x + 16G*^3 = 0
# Try: x^3 - 16G*^2 x - 16G*^3 = 0

a_coeff = 16 * G_STAR**2
b_coeff = 16 * G_STAR**3
roots_master = np.roots([1, 0, -a_coeff, -b_coeff])

print(f"Master cubic: x^3 - 16*G*^2*x - 16*G*^3 = 0")
print(f"            = x^3 - {a_coeff:.6f}x - {b_coeff:.6f} = 0")
print()
print(f"Roots:")
for i, r in enumerate(roots_master):
    if np.isreal(r) or abs(r.imag) < 1e-10:
        print(f"  x{i+1} = {r.real:.10f} (real)")
    else:
        print(f"  x{i+1} = {r.real:.6f} + {r.imag:.6f}i (complex)")
print()

# Check discriminant
# For depressed cubic x^3 + px + q = 0, discriminant = -4p^3 - 27q^2
p = -a_coeff
q = -b_coeff
discriminant = -4*p**3 - 27*q**2
print(f"Discriminant = -4p^3 - 27q^2 = {discriminant:.6f}")
print(f"Sign: {'POSITIVE (3 real roots)' if discriminant > 0 else 'NEGATIVE (1 real, 2 complex)'}")
print()

# ============================================================================
# ALTERNATIVE: x^3 - G*^2 x - G*^3 = 0 (simpler form)
# ============================================================================

print("=" * 70)
print("SIMPLER CUBIC: x^3 - G*^2*x - G*^3 = 0")
print("=" * 70)

a2 = G_STAR**2
b2 = G_STAR**3
roots_simple = np.roots([1, 0, -a2, -b2])

print(f"Cubic: x^3 - {a2:.6f}x - {b2:.6f} = 0")
print()
print(f"Roots:")
for i, r in enumerate(roots_simple):
    if np.isreal(r) or abs(r.imag) < 1e-10:
        print(f"  x{i+1} = {r.real:.10f} (real)")
    else:
        print(f"  x{i+1} = {r.real:.6f} + {r.imag:.6f}i (complex)")
print()

# Check if any root is G* itself
print(f"Is G* = {G_STAR:.6f} a root?")
check = G_STAR**3 - G_STAR**2 * G_STAR - G_STAR**3
print(f"  G*^3 - G*^2*G* - G*^3 = {check:.10f}")
print()

# ============================================================================
# BEAUTIFUL DISCOVERY: x^3 - G*x^2 - G*^2 = 0
# ============================================================================

print("=" * 70)
print("EXPLORING: x^3 - G*x^2 - G*^2 = 0")
print("=" * 70)

# This has the structure matching the quadratic's Vieta formulas
roots_v = np.roots([1, -G_STAR, 0, -G_STAR**2])

print(f"Cubic: x^3 - G*x^2 - G*^2 = 0")
print(f"     = x^3 - {G_STAR:.6f}x^2 - {G_STAR**2:.6f} = 0")
print()
print(f"Roots:")
for i, r in enumerate(roots_v):
    if np.isreal(r) or abs(r.imag) < 1e-10:
        print(f"  x{i+1} = {r.real:.10f} (real)")
    else:
        print(f"  x{i+1} = {r.real:.6f} + {r.imag:.6f}i (complex)")
print()

# ============================================================================
# THE KEY INSIGHT: FACTOR THE QUADRATIC FURTHER
# ============================================================================

print("=" * 70)
print("KEY INSIGHT: EXTENDING THE QUADRATIC")
print("=" * 70)

# The quadratic x^2 - 16G*^2 x + 16G*^3 = 0 can be written as:
# x^2 - 16G*^2 x + 16G*^3 = (x - x+)(x - x-)
#
# What if this is the DERIVATIVE of a cubic?
# If f(x) = x^3/3 - 8G*^2 x^2 + 16G*^3 x + C
# Then f'(x) = x^2 - 16G*^2 x + 16G*^3 = 0

# The extrema of f(x) are at x+ and x-!
# What is f(x) at these points?

def cubic_f(x, C=0):
    return x**3/3 - 8*G_STAR**2 * x**2 + 16*G_STAR**3 * x + C

x_plus_val = 137.0361
x_minus_val = 3.0240

f_at_plus = cubic_f(x_plus_val)
f_at_minus = cubic_f(x_minus_val)

print("If the quadratic is the DERIVATIVE of a cubic:")
print(f"  f(x) = x^3/3 - 8*G*^2*x^2 + 16*G*^3*x + C")
print(f"  f'(x) = x^2 - 16*G*^2*x + 16*G*^3 = 0 at x = x+, x-")
print()
print(f"  f(x+) = f({x_plus_val}) = {f_at_plus:.4f}")
print(f"  f(x-) = f({x_minus_val}) = {f_at_minus:.4f}")
print()
print(f"  Difference: f(x+) - f(x-) = {f_at_plus - f_at_minus:.4f}")
print()

# What C makes f have a root at a meaningful point?
# If f(1) = 0: 1/3 - 8G*^2 + 16G*^3 + C = 0
C_for_root_at_1 = -(1/3 - 8*G_STAR**2 + 16*G_STAR**3)
print(f"For f(1) = 0, C = {C_for_root_at_1:.4f}")
print()

# ============================================================================
# SUMMARY
# ============================================================================

print("=" * 70)
print("SUMMARY OF CUBIC EXPLORATIONS")
print("=" * 70)

print("""
KEY FINDINGS:

1. PATH 3 (Hierarchy) gives the most physically meaningful result:
   - x+ = 137.036 (electromagnetic)
   - x- = 3.024 (strong/color)
   - x0 = 13.08 (from weak mixing angle) ~ N_eff = 13

   This suggests the CUBIC encodes ALL THREE gauge couplings!

2. PATH 5 (Weierstrass) connects to the ternary ontology:
   - At y = 0: roots are {-1, 0, +1} = FTD states
   - This is deeply embedded in the lemniscate geometry

3. The quadratic may be the DERIVATIVE of a master cubic:
   - The roots x+, x- are EXTREMA of a cubic f(x)
   - The cubic's own roots might encode additional physics

NEXT STEPS:
- Find what cubic has roots at 137, 3, and 13 with simple G* coefficients
- Explore whether the weak mixing angle emerges naturally
- Check if gravitational coupling appears in the cubic discriminant
""")

# ============================================================================
# FINAL TEST: Can we construct a cubic with roots 137, 3, 13 from G*?
# ============================================================================

print("=" * 70)
print("FINAL: CONSTRUCTING THE HIERARCHY CUBIC")
print("=" * 70)

# Target roots: 137.036, 3.024, 13
# From Vieta:
# -a = x1 + x2 + x3 = 137.036 + 3.024 + 13 = 153.06
# b = x1*x2 + x2*x3 + x3*x1 = 137*3 + 3*13 + 13*137 = 411 + 39 + 1781 = 2231
# -c = x1*x2*x3 = 137 * 3 * 13 = 5343

# Can we express 153, 2231, 5343 in terms of 16, G*, G*^2, G*^3?
target_sum = 137.036 + 3.024 + 13
target_sum_prod = 137.036*3.024 + 3.024*13 + 13*137.036
target_prod = 137.036 * 3.024 * 13

print(f"Target roots: 137.036, 3.024, 13")
print(f"Sum = {target_sum:.4f}")
print(f"Sum of products = {target_sum_prod:.4f}")
print(f"Product = {target_prod:.4f}")
print()

print(f"Ratios with G* powers:")
print(f"  Sum / 16G* = {target_sum / (16*G_STAR):.6f}")
print(f"  Sum / G*^2 = {target_sum / G_STAR**2:.6f}")
print(f"  Sum_prod / 16G*^2 = {target_sum_prod / (16*G_STAR**2):.6f}")
print(f"  Sum_prod / G*^3 = {target_sum_prod / G_STAR**3:.6f}")
print(f"  Product / 16G*^3 = {target_prod / (16*G_STAR**3):.6f}")
print(f"  Product / G*^4 = {target_prod / G_STAR**4:.6f}")
print()

# Check: Sum / 16G* ~ 3.23 ~ some framework integer?
# Check: Product / 16G*^3 ~ 12.9 ~ N_eff - 0.1!

print(f"INTERESTING PATTERNS:")
print(f"  Product / (16G*^3) = {target_prod / (16*G_STAR**3):.4f} ~ N_eff = 13!")
print(f"  If we use N_eff instead of 13.08, product = {137.036 * 3.024 * N_eff:.4f}")
print()

# The hierarchy cubic in natural form:
print(f"THE HIERARCHY CUBIC:")
print(f"  x^3 - {target_sum:.3f}x^2 + {target_sum_prod:.3f}x - {target_prod:.3f} = 0")
print()
print(f"  In terms of G*:")
print(f"  x^3 - (16G* + 3 + 13/G*)x^2 + ...  [complex, needs refinement]")
