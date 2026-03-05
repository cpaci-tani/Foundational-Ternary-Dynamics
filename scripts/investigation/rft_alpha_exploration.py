"""
Comprehensive exploration of the RFT formula for the fine structure constant:
    1/alpha ~ 4*pi^3 + pi^2 + pi = 137.03630378...

Investigating algebraic structure, roots, connections to FTD, lemniscate constants,
number theory, and more.
"""

import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

import numpy as np
from scipy.special import gamma
from scipy.optimize import brentq
import cmath

# ============================================================================
# CONSTANTS
# ============================================================================
PI = np.float64(np.pi)
E_CONST = np.float64(np.e)
PHI = (1 + np.sqrt(5)) / 2  # golden ratio
ALPHA_CODATA = 137.035999177  # CODATA 2022 1/α
ALPHA_INV = ALPHA_CODATA

# Lemniscate constant ϖ = 2 * ∫₀¹ dt/√(1-t⁴)
# ϖ = Γ(1/4)² / (2√(2π))
GAMMA_QUARTER = gamma(0.25)
VARPI = GAMMA_QUARTER**2 / (2 * np.sqrt(2 * PI))  # lemniscate constant ≈ 2.6220575

# Gauss's constant G = 1/M(1,√2) = 2ϖ/π  (agm relation)
# G* (lemniscatic constant as used in FTD) = √2 × Γ(1/4)² / (2π)
G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * PI)

# Gauss constant (reciprocal of AGM(1, √2))
G_GAUSS = 2 * VARPI / PI

print("=" * 80)
print("COMPREHENSIVE EXPLORATION OF THE RFT FORMULA: α⁻¹ ≈ 4π³ + π² + π")
print("=" * 80)

print("\n--- FUNDAMENTAL CONSTANTS ---")
print(f"  π                = {PI:.16f}")
print(f"  e                = {E_CONST:.16f}")
print(f"  φ (golden ratio) = {PHI:.16f}")
print(f"  Γ(1/4)           = {GAMMA_QUARTER:.16f}")
print(f"  ϖ (lemniscate)   = {VARPI:.16f}")
print(f"  G* (FTD)         = {G_STAR:.16f}")
print(f"  G (Gauss const)  = {G_GAUSS:.16f}")
print(f"  1/α (CODATA)     = {ALPHA_CODATA:.9f}")

# ============================================================================
# PART 1: The polynomial p(x) = 4x³ + x² + x
# ============================================================================
print("\n" + "=" * 80)
print("PART 1: THE POLYNOMIAL p(x) = 4x³ + x² + x")
print("=" * 80)

def p(x):
    return 4*x**3 + x**2 + x

p_pi = p(PI)
print(f"\n  p(π) = 4π³ + π² + π = {p_pi:.16f}")
print(f"  CODATA 1/α           = {ALPHA_CODATA:.9f}")
print(f"  Difference            = {p_pi - ALPHA_CODATA:.16e}")
print(f"  Relative error        = {abs(p_pi - ALPHA_CODATA)/ALPHA_CODATA:.6e}")
print(f"  Error in ppm          = {abs(p_pi - ALPHA_CODATA)/ALPHA_CODATA * 1e6:.4f} ppm")

print("\n  --- Roots of 4x³ + x² + x = 0 ---")
print("  Factor: x(4x² + x + 1) = 0")
print(f"  Root 1: x = 0")

# Quadratic 4x² + x + 1 = 0
disc = 1 - 4*4*1  # b² - 4ac
print(f"\n  Inner quadratic: 4x² + x + 1 = 0")
print(f"  Discriminant = 1 - 16 = {disc}")
print(f"  Discriminant < 0 → complex roots")

r1 = (-1 + cmath.sqrt(disc)) / 8
r2 = (-1 - cmath.sqrt(disc)) / 8
print(f"\n  Root 2: r₁ = (-1 + i√15)/8 = {r1}")
print(f"           Re(r₁) = {r1.real:.16f}")
print(f"           Im(r₁) = {r1.imag:.16f}")
print(f"  Root 3: r₂ = (-1 - i√15)/8 = {r2}")
print(f"           Re(r₂) = {r2.real:.16f}")
print(f"           Im(r₂) = {r2.imag:.16f}")

mod_r = abs(r1)
arg_r = cmath.phase(r1)
print(f"\n  |r₁| = {mod_r:.16f}")
print(f"  |r₁|² = {mod_r**2:.16f}")
print(f"  Note: |r|² = (1 + 15)/64 = 16/64 = 1/4 ✓" if abs(mod_r**2 - 0.25) < 1e-14 else "  WARNING: |r|² ≠ 1/4")
print(f"  |r₁| = 1/√4 = 1/2 = {0.5:.16f} ✓" if abs(mod_r - 0.5) < 1e-14 else f"  |r₁| = {mod_r:.16f}")

print(f"\n  arg(r₁) = {arg_r:.16f} radians")
print(f"          = {np.degrees(arg_r):.10f} degrees")
print(f"          = {arg_r/PI:.16f} × π")
print(f"  arg(r₁) = arctan(√15) + π (since in Q2)")
print(f"          = π - arctan(√15) = {PI - np.arctan(np.sqrt(15)):.16f}")

# ============================================================================
# PART 2: The "inverse" polynomial
# ============================================================================
print("\n" + "=" * 80)
print("PART 2: THE 'INVERSE' POLYNOMIAL — solving p(x) = 1/α exactly")
print("=" * 80)

# Solve 4x³ + x² + x = 137.035999177
# Rearrange: 4x³ + x² + x - 137.035999177 = 0
coeffs = [4, 1, 1, -ALPHA_CODATA]
roots_inv = np.roots(coeffs)
print(f"\n  Solving 4x³ + x² + x = {ALPHA_CODATA}")
for i, root in enumerate(roots_inv):
    if np.isreal(root):
        x_real = root.real
        print(f"  Real root: x = {x_real:.16f}")
        print(f"    π        = {PI:.16f}")
        print(f"    x - π    = {x_real - PI:.16e}")
        print(f"    Relative = {abs(x_real - PI)/PI:.6e}")
        print(f"    In ppm   = {abs(x_real - PI)/PI * 1e6:.4f} ppm")
    else:
        print(f"  Complex root: {root}")

# More precise solve using brentq
def f_inv(x):
    return 4*x**3 + x**2 + x - ALPHA_CODATA

x_exact = brentq(f_inv, 3.0, 3.2)
print(f"\n  High-precision real root (brentq):")
print(f"    x_exact = {x_exact:.16f}")
print(f"    π       = {PI:.16f}")
print(f"    diff    = {x_exact - PI:.16e}")
print(f"    |diff|/π = {abs(x_exact - PI)/PI:.6e}")
print(f"    ppm     = {abs(x_exact - PI)/PI * 1e6:.6f} ppm")
print(f"    p(x_exact) = {p(x_exact):.16f}")

# ============================================================================
# PART 3: Related polynomials
# ============================================================================
print("\n" + "=" * 80)
print("PART 3: RELATED POLYNOMIALS")
print("=" * 80)

# What constant c makes p(π) + c = 1/α exactly?
c_needed = ALPHA_CODATA - p_pi
print(f"\n  --- Additive correction ---")
print(f"  p(π)    = {p_pi:.16f}")
print(f"  1/α     = {ALPHA_CODATA:.9f}")
print(f"  c needed = 1/α - p(π) = {c_needed:.16e}")
print(f"  So: 4π³ + π² + π + ({c_needed:.10e}) = 1/α exactly")

print(f"\n  --- Full cubic (adding constant term 1) ---")
print(f"  4π³ + π² + π + 1 = {p_pi + 1:.16f}")
print(f"  4π³ + π² + π - 1 = {p_pi - 1:.16f}")

print(f"\n  --- Nearby integer-coefficient cubics ax³ + bx² + cx at x = π ---")
best_err = float('inf')
best_abc = None
results_nearby = []
for a in range(3, 6):
    for b in range(-2, 4):
        for c in range(-2, 4):
            val = a * PI**3 + b * PI**2 + c * PI
            err = abs(val - ALPHA_CODATA)
            results_nearby.append(((a, b, c), val, err))
            if err < best_err:
                best_err = err
                best_abc = (a, b, c)

results_nearby.sort(key=lambda x: x[2])
print(f"  Top 10 closest (sorted by |error|):")
for (a, b, c), val, err in results_nearby[:10]:
    print(f"    {a}x³ + {b:+d}x² + {c:+d}x = {val:.10f}  (err = {err:.6e}, {err/ALPHA_CODATA*1e6:.2f} ppm)")

# ============================================================================
# PART 4: Connection to FTD's master quadratic
# ============================================================================
print("\n" + "=" * 80)
print("PART 4: CONNECTION TO FTD's MASTER QUADRATIC")
print("=" * 80)

print(f"\n  FTD master quadratic: x² - 16G*²x + 16G*³ = 0")
print(f"  G* = √2 × Γ(1/4)² / (2π) = {G_STAR:.16f}")
print(f"  G*² = {G_STAR**2:.16f}")
print(f"  G*³ = {G_STAR**3:.16f}")
print(f"  16G*² = {16*G_STAR**2:.16f}")
print(f"  16G*³ = {16*G_STAR**3:.16f}")

# Roots: x = 8G*² ± 8G*²√(1 - 1/G*)
# Using quadratic formula: x = [16G*² ± √(256G*⁴ - 64G*³)] / 2
disc_ftd = (16*G_STAR**2)**2 - 4*16*G_STAR**3
x_plus = (16*G_STAR**2 + np.sqrt(disc_ftd)) / 2
x_minus = (16*G_STAR**2 - np.sqrt(disc_ftd)) / 2

print(f"\n  Discriminant = 256G*⁴ - 64G*³ = {disc_ftd:.16f}")
print(f"  √(discriminant) = {np.sqrt(disc_ftd):.16f}")
print(f"\n  x₊ = {x_plus:.16f}")
print(f"  x₋ = {x_minus:.16f}")
print(f"\n  Product x₊ × x₋ = 16G*³ = {x_plus * x_minus:.16f} (should be {16*G_STAR**3:.16f})")
print(f"  Sum x₊ + x₋ = 16G*² = {x_plus + x_minus:.16f} (should be {16*G_STAR**2:.16f})")

# Compare with RFT
diff_rft_ftd = p_pi - x_plus
print(f"\n  --- Comparison: RFT vs FTD ---")
print(f"  RFT:  4π³ + π² + π = {p_pi:.16f}")
print(f"  FTD:  x₊             = {x_plus:.16f}")
print(f"  CODATA 1/α           = {ALPHA_CODATA:.9f}")
print(f"\n  RFT - FTD = {diff_rft_ftd:.16e}")
print(f"  RFT - CODATA = {p_pi - ALPHA_CODATA:.16e}")
print(f"  FTD - CODATA = {x_plus - ALPHA_CODATA:.16e}")

# Try to express the differences in terms of known constants
diff_rf = p_pi - ALPHA_CODATA
diff_ftd_c = x_plus - ALPHA_CODATA

print(f"\n  --- Trying to identify RFT - CODATA = {diff_rf:.10e} ---")
candidates = {
    'π²/137²': PI**2 / 137**2,
    'π/137²': PI / 137**2,
    'α²': (1/137.036)**2,
    'α²×π': (1/137.036)**2 * PI,
    'π³/137³': PI**3 / 137**3,
    'α×π': (1/137.036) * PI,
    'G*/137²': G_STAR / 137**2,
    'ϖ/137²': VARPI / 137**2,
    'π²/(2×137²)': PI**2 / (2*137**2),
    '1/(4π²)': 1/(4*PI**2),
    'α/π': (1/137.036) / PI,
    'π/α': PI * 137.036,
    '(α/π)²': ((1/137.036) / PI)**2,
    'α²/2': (1/137.036)**2 / 2,
}

print(f"  Candidate expressions for difference {diff_rf:.10e}:")
for name, val in sorted(candidates.items(), key=lambda kv: abs(kv[1] - diff_rf)):
    ratio = diff_rf / val if val != 0 else float('inf')
    print(f"    {name:20s} = {val:.10e}  (ratio diff/expr = {ratio:.6f})")

# Try ratios with small integers
print(f"\n  --- Difference as N × known_constant ---")
for name, val in [('π', PI), ('π²', PI**2), ('α', 1/137.036), ('G*', G_STAR), ('ϖ', VARPI), ('1', 1.0)]:
    ratio = diff_rf / val
    print(f"    (RFT-CODATA)/{name:5s} = {ratio:.10e}")

print(f"\n  --- Difference FTD - CODATA = {diff_ftd_c:.10e} ---")
for name, val in [('π', PI), ('π²', PI**2), ('α', 1/137.036), ('G*', G_STAR), ('ϖ', VARPI), ('1', 1.0)]:
    ratio = diff_ftd_c / val
    print(f"    (FTD-CODATA)/{name:5s} = {ratio:.10e}")

# ============================================================================
# PART 5: Complex roots deep dive
# ============================================================================
print("\n" + "=" * 80)
print("PART 5: COMPLEX ROOTS DEEP DIVE")
print("=" * 80)

print(f"\n  Roots of 4x² + x + 1 = 0:")
print(f"  r₁ = (-1 + i√15)/8 = {r1.real:.16f} + {r1.imag:.16f}i")
print(f"  r₂ = (-1 - i√15)/8 = {r2.real:.16f} + {r2.imag:.16f}i")
print(f"\n  |r₁| = {abs(r1):.16f}")
print(f"  |r₁|² = {abs(r1)**2:.16f} = 1/4 {'✓' if abs(abs(r1)**2 - 0.25) < 1e-14 else '✗'}")
print(f"  √(1/4) = 1/2 = 0.5 {'✓' if abs(abs(r1) - 0.5) < 1e-14 else '✗'}")

arg1 = cmath.phase(r1)
arg2 = cmath.phase(r2)
print(f"\n  arg(r₁) = {arg1:.16f} rad = {np.degrees(arg1):.10f}°")
print(f"  arg(r₂) = {arg2:.16f} rad = {np.degrees(arg2):.10f}°")
print(f"  arg(r₁)/π = {arg1/PI:.16f}")
print(f"  arg(r₂)/π = {arg2/PI:.16f}")

# Is arg related to known angles?
known_angles_deg = {
    '60°': 60, '90°': 90, '120°': 120, '150°': 150,
    '100°': 100, '105°': 105, '108° (pentagon)': 108,
    '109.47° (tetrahedral)': 109.4712,
}
arg1_deg = np.degrees(arg1)
print(f"\n  arg(r₁) = {arg1_deg:.6f}° — nearby known angles:")
for name, angle in sorted(known_angles_deg.items(), key=lambda kv: abs(kv[1] - arg1_deg)):
    print(f"    {name:25s}: diff = {arg1_deg - angle:.6f}°")

# Powers of r₁
print(f"\n  --- Powers of r₁ = {r1} ---")
print(f"  Since |r₁| = 1/2, |r₁ⁿ| = (1/2)ⁿ → shrinks rapidly")
for n in [2, 3, 4, 5, 6, 7, 8, 12, 13, 15, 30, 137]:
    rn = r1**n
    print(f"    r₁^{n:3d} = {rn.real:+.10e} {'+' if rn.imag >= 0 else '-'} {abs(rn.imag):.10e}i   |r₁^{n}| = {abs(rn):.6e}  arg/π = {cmath.phase(rn)/PI:.8f}")

# Special: what power n makes arg(r₁ⁿ) closest to 0 or π?
print(f"\n  --- Powers closest to real axis (arg ≈ 0 or ±π) ---")
real_closeness = []
for n in range(1, 200):
    rn = r1**n
    arg_n = cmath.phase(rn)
    closeness_to_real = min(abs(arg_n), abs(arg_n - PI), abs(arg_n + PI))
    real_closeness.append((n, closeness_to_real, rn))
real_closeness.sort(key=lambda x: x[1])
for n, cl, rn in real_closeness[:10]:
    print(f"    n={n:3d}: arg/π = {cmath.phase(rn)/PI:.8f}  closeness = {cl:.6f} rad  r₁ⁿ ≈ {rn.real:.6e}")

# ============================================================================
# PART 6: Polynomial evaluated at other constants
# ============================================================================
print("\n" + "=" * 80)
print("PART 6: p(x) = 4x³ + x² + x EVALUATED AT OTHER CONSTANTS")
print("=" * 80)

test_vals = {
    'π': PI,
    'e': E_CONST,
    'φ (golden ratio)': PHI,
    '√2': np.sqrt(2),
    'ϖ (lemniscate)': VARPI,
    'G* (FTD)': G_STAR,
    '1': 1.0,
    '2': 2.0,
    '3': 3.0,
    '4': 4.0,
    '5': 5.0,
    'π/2': PI/2,
    '2π': 2*PI,
    'Γ(1/4)': GAMMA_QUARTER,
}

for name, val in test_vals.items():
    result = p(val)
    print(f"  p({name:18s}) = p({val:.10f}) = {result:.10f}")

# Factor form
print(f"\n  Note: p(x) = x(4x² + x + 1)")
print(f"  So p(n) for integer n:")
for n in range(1, 11):
    inner = 4*n**2 + n + 1
    print(f"    p({n}) = {n} × {inner} = {n*inner}")

# ============================================================================
# PART 7: Factoring over different fields
# ============================================================================
print("\n" + "=" * 80)
print("PART 7: ALGEBRAIC STRUCTURE AND GALOIS THEORY")
print("=" * 80)

print(f"\n  4x³ + x² + x = x(4x² + x + 1)")
print(f"  The quadratic 4x² + x + 1 is irreducible over ℚ (and ℝ)")
print(f"  Discriminant Δ = 1 - 16 = -15")
print(f"  -15 = -1 × 3 × 5")
print(f"\n  The splitting field is ℚ(√(-15)) = ℚ(i√15)")
print(f"  This is a degree-2 extension of ℚ")
print(f"  Galois group: Gal(ℚ(i√15)/ℚ) ≅ ℤ/2ℤ")
print(f"\n  Over ℚ(i√15), the factorization is:")
print(f"  4x³ + x² + x = x · 4 · (x - r₁)(x - r₂)")
print(f"  where r₁ = (-1 + i√15)/8, r₂ = (-1 - i√15)/8")

print(f"\n  --- Algebraic properties of the discriminant -15 ---")
print(f"  -15 is a Heegner number discriminant? Heegner numbers: -1,-2,-3,-7,-11,-19,-43,-67,-163")
print(f"  -15 is NOT a Heegner discriminant")
print(f"  But the class number of ℚ(√(-15)):")
print(f"  h(-15) = 2 (the class group has order 2)")

print(f"\n  --- Connection to quadratic forms ---")
print(f"  The principal form: x² + 15y² represents primes p ≡ 1 (mod 15) or p | 15")
print(f"  Primes 3 and 5 divide 15")
print(f"  -15 ≡ 1 (mod 4)? -15 mod 4 = 1. Yes.")
print(f"  So the ring of integers of ℚ(√(-15)) is ℤ[(1+i√15)/2]")

print(f"\n  --- The quadratic 4x² + x + 1 modulo small primes ---")
for prime in [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37]:
    roots_mod_p = []
    for x in range(prime):
        if (4*x**2 + x + 1) % prime == 0:
            roots_mod_p.append(x)
    status = f"roots: {roots_mod_p}" if roots_mod_p else "irreducible"
    print(f"    mod {prime:2d}: {status}")

# ============================================================================
# PART 8: Number-theoretic observations
# ============================================================================
print("\n" + "=" * 80)
print("PART 8: NUMBER-THEORETIC OBSERVATIONS")
print("=" * 80)

inner_val = 4*PI**2 + PI + 1
print(f"\n  4π² + π + 1 = {inner_val:.16f}")
print(f"  So α⁻¹ ≈ π × (4π² + π + 1) = {PI * inner_val:.16f}")
print(f"\n  4π² = {4*PI**2:.16f}")
print(f"  4π² - 39 = {4*PI**2 - 39:.16f}")
print(f"  4π² - 40 = {4*PI**2 - 40:.16f}")
print(f"  40 - 4π² = {40 - 4*PI**2:.16f}")
print(f"  Note: 40 - 4π² ≈ 0.522 ≈ m_e(MeV)/c² hmm...")
print(f"  Actually m_e = 0.511 MeV, 40 - 4π² = {40 - 4*PI**2:.6f}")

# Continued fraction of 4π² + π + 1
from fractions import Fraction

def continued_fraction_coefficients(x, n_terms=15):
    """Get continued fraction coefficients."""
    coeffs = []
    for _ in range(n_terms):
        a = int(np.floor(x))
        coeffs.append(a)
        frac = x - a
        if abs(frac) < 1e-12:
            break
        x = 1.0 / frac
    return coeffs

print(f"\n  Continued fraction of 4π² + π + 1:")
cf = continued_fraction_coefficients(inner_val)
print(f"    [{cf[0]}; {', '.join(map(str, cf[1:]))}]")

print(f"\n  Continued fraction of 4π³ + π² + π:")
cf2 = continued_fraction_coefficients(p_pi)
print(f"    [{cf2[0]}; {', '.join(map(str, cf2[1:]))}]")

print(f"\n  Continued fraction of 1/α (CODATA):")
cf3 = continued_fraction_coefficients(ALPHA_CODATA)
print(f"    [{cf3[0]}; {', '.join(map(str, cf3[1:]))}]")

# Nearby rationals
print(f"\n  --- Best rational approximations to 4π² + π + 1 ---")
best_rats = []
for d in range(1, 200):
    n = round(inner_val * d)
    err = abs(inner_val - n/d)
    best_rats.append((n, d, err))
best_rats.sort(key=lambda x: x[2])
for n, d, err in best_rats[:8]:
    print(f"    {n}/{d} = {n/d:.10f}  (err = {err:.8e})")

# Is 4π² + π + 1 close to 131/3?
print(f"\n  131/3 = {131/3:.10f}")
print(f"  4π² + π + 1 = {inner_val:.10f}")
print(f"  Difference = {inner_val - 131/3:.10e}")

# Egyptian-fraction-like decomposition
print(f"\n  --- Decomposition of p(π) ---")
print(f"  p(π) = 137 + {p_pi - 137:.16f}")
print(f"  p(π) = 137 + 1/{1/(p_pi - 137):.6f}")
rem = p_pi - 137
print(f"  Fractional part = {rem:.16f}")
print(f"  1/fractional_part = {1/rem:.10f}")
print(f"  Nearest integer to 1/frac: {round(1/rem)}")
print(f"  p(π) ≈ 137 + 1/{round(1/rem)} = {137 + 1/round(1/rem):.10f}")

# ============================================================================
# PART 9: Connection to the lemniscate
# ============================================================================
print("\n" + "=" * 80)
print("PART 9: CONNECTION TO THE LEMNISCATE")
print("=" * 80)

# π = ϖ/G where G is Gauss's constant (related to agm)
# Actually, the relationship is: ϖ = π × G_GAUSS / 2... let me be precise
# The lemniscate constant: ϖ = 2∫₀¹ dt/√(1-t⁴) = Γ(1/4)²/(2√(2π))
# Gauss's constant: G = 2/π × ϖ = Γ(1/4)²/(π√(2π))
# So π = 2ϖ/G... no. Let's just compute directly.

# G* = √2 × Γ(1/4)²/(2π) = √2 × 2√(2π) × ϖ / (2π) = √2 × √(2π) × ϖ / π
# Actually let me just verify: ϖ = Γ(1/4)²/(2√(2π))
# G* = √2 × Γ(1/4)²/(2π) = √2 × 2√(2π) × ϖ / (2π) = 2√2 × √(2π) × ϖ / (2π)
# = 2 × 2 × √π × ϖ / (2π) = 2ϖ/√π ... let me just compute numerically

print(f"\n  ϖ = {VARPI:.16f}")
print(f"  G* = {G_STAR:.16f}")
print(f"  π = {PI:.16f}")

# Ratio G*/ϖ
print(f"\n  G*/ϖ = {G_STAR/VARPI:.16f}")
print(f"  √2 = {np.sqrt(2):.16f}")
print(f"  G*/ϖ / √2 = {G_STAR/VARPI/np.sqrt(2):.16f}")

# Since ϖ = Γ(1/4)²/(2√(2π)) and G* = √2 Γ(1/4)²/(2π):
# G*/ϖ = [√2 Γ(1/4)²/(2π)] / [Γ(1/4)²/(2√(2π))] = √2 × 2√(2π) / (2π) = √2 × √(2π)/π = 2/√π
print(f"  G*/ϖ = 2/√π = {2/np.sqrt(PI):.16f}")
print(f"  Verification: G*/ϖ = {G_STAR/VARPI:.16f} vs 2/√π = {2/np.sqrt(PI):.16f}")

# Express p(π) in terms of ϖ
# p(π) = π(4π² + π + 1)
# Need π in terms of ϖ and G*
# From G* = 2ϖ/√π × something... Let's use the relation differently
# ϖ × G* = Γ(1/4)²/(2√(2π)) × √2 Γ(1/4)²/(2π)
varpi_gstar = VARPI * G_STAR
print(f"\n  ϖ × G* = {varpi_gstar:.16f}")
print(f"  Γ(1/4)⁴/(4π√(2π)) = {GAMMA_QUARTER**4 / (4*PI*np.sqrt(2*PI)):.16f}")
print(f"  (should match)")

# ϖ/π ratio
print(f"\n  ϖ/π = {VARPI/PI:.16f}")
print(f"  G*/π = {G_STAR/PI:.16f}")

# The quadratic 4ϖ² + ϖG + G²  (from Part 9 of prompt)
# Actually from the factored form: p(π) = (ϖ/G)(4ϖ²/G² + ϖ/G + 1) where G is chosen so π = ϖ/G
# But what's the right G?
# Gauss's constant M = agm(1, √2), G_gauss = 1/M
# π and ϖ relationship: ϖ = π × M(1,√2)/2? Let me just check numerically
print(f"\n  --- ϖ and π relationship ---")
print(f"  ϖ/π = {VARPI/PI:.16f}")
print(f"  π/ϖ = {PI/VARPI:.16f}")

# Let's define G_ratio = ϖ/π so π = ϖ/G_ratio
G_ratio = VARPI / PI
print(f"  G_ratio = ϖ/π = {G_ratio:.16f}")
print(f"  So π = ϖ/G_ratio")

inner_lemniscate = 4*VARPI**2/G_ratio**2 + VARPI/G_ratio + 1
print(f"\n  4ϖ²/G² + ϖ/G + 1 = 4π² + π + 1 = {inner_lemniscate:.16f}")

# Direct: 4ϖ² + ϖ×G_ratio + G_ratio²
quad_lem = 4*VARPI**2 + VARPI*G_ratio + G_ratio**2
print(f"\n  4ϖ² + ϖ(ϖ/π) + (ϖ/π)² = {quad_lem:.16f}")
print(f"  = 4ϖ² + ϖ²/π + ϖ²/π² = ϖ²(4 + 1/π + 1/π²)")
val2 = VARPI**2 * (4 + 1/PI + 1/PI**2)
print(f"  = {val2:.16f}")

# Now the full expression
print(f"\n  p(π) = π(4π² + π + 1)")
print(f"       = (ϖ/G_ratio)(4π² + π + 1)")
print(f"       = ϖ(4π² + π + 1)/G_ratio")
print(f"       = ϖ × π × (4π² + π + 1) / ϖ  ... (trivial)")
print(f"\n  More interesting: express entirely in ϖ:")
print(f"  Using π = ϖ × (π/ϖ) = ϖ × {PI/VARPI:.10f}")

# ============================================================================
# PART 10: Wild card explorations
# ============================================================================
print("\n" + "=" * 80)
print("PART 10: WILD CARD EXPLORATIONS")
print("=" * 80)

# Is 4π³ + π² + π close to any expression involving ϖ directly?
print(f"\n  --- Expressions involving ϖ near 1/α ---")
expressions = {
    'ϖ⁵': VARPI**5,
    'ϖ⁶': VARPI**6,
    'ϖ⁴×π': VARPI**4 * PI,
    'ϖ³×π²': VARPI**3 * PI**2,
    'ϖ²×π³': VARPI**2 * PI**3,
    '2ϖ⁵': 2*VARPI**5,
    'ϖ⁵/2': VARPI**5/2,
    '4ϖ³+ϖ²+ϖ': 4*VARPI**3 + VARPI**2 + VARPI,
    '5ϖ³': 5*VARPI**3,
    '6ϖ³': 6*VARPI**3,
    '7ϖ³': 7*VARPI**3,
    '8ϖ³': 8*VARPI**3,
    'ϖ×G*×π': VARPI * G_STAR * PI,
    'ϖ²×G*': VARPI**2 * G_STAR,
    'ϖ×G*²': VARPI * G_STAR**2,
    'G*×π²': G_STAR * PI**2,
    'G*²×π': G_STAR**2 * PI,
    'ϖ×π×G*': VARPI * PI * G_STAR,
    '16×G*²': 16 * G_STAR**2,
    'π⁵/2': PI**5/2,
    'π⁵': PI**5,
    'Γ(1/4)⁴/π': GAMMA_QUARTER**4/PI,
    'Γ(1/4)⁴/(2π)': GAMMA_QUARTER**4/(2*PI),
    'Γ(1/4)⁴/(4π)': GAMMA_QUARTER**4/(4*PI),
}

sorted_expr = sorted(expressions.items(), key=lambda kv: abs(kv[1] - ALPHA_CODATA))
print(f"  Target: 1/α = {ALPHA_CODATA:.10f}")
for name, val in sorted_expr[:15]:
    err = val - ALPHA_CODATA
    print(f"    {name:25s} = {val:.10f}  (diff = {err:+.6e}, {abs(err)/ALPHA_CODATA*1e6:.1f} ppm)")

# What polynomial in ϖ gives closest to 1/α?
print(f"\n  --- Best aϖ³ + bϖ² + cϖ for small integers ---")
best_varpi = []
for a in range(1, 12):
    for b in range(-5, 6):
        for c in range(-5, 6):
            val = a*VARPI**3 + b*VARPI**2 + c*VARPI
            err = abs(val - ALPHA_CODATA)
            best_varpi.append(((a, b, c), val, err))
best_varpi.sort(key=lambda x: x[2])
print(f"  Top 10:")
for (a, b, c), val, err in best_varpi[:10]:
    print(f"    {a}ϖ³ + {b:+d}ϖ² + {c:+d}ϖ = {val:.10f}  (err = {err:.6e}, {err/ALPHA_CODATA*1e6:.2f} ppm)")

# Same for G*
print(f"\n  --- Best aG*³ + bG*² + cG* for small integers ---")
best_gstar = []
for a in range(1, 12):
    for b in range(-10, 11):
        for c in range(-10, 11):
            val = a*G_STAR**3 + b*G_STAR**2 + c*G_STAR
            err = abs(val - ALPHA_CODATA)
            best_gstar.append(((a, b, c), val, err))
best_gstar.sort(key=lambda x: x[2])
print(f"  Top 10:")
for (a, b, c), val, err in best_gstar[:10]:
    print(f"    {a}G*³ + {b:+d}G*² + {c:+d}G* = {val:.10f}  (err = {err:.6e}, {err/ALPHA_CODATA*1e6:.2f} ppm)")

# Relationship between RFT formula and FTD master quadratic root
print(f"\n  --- x₊ = 4π³ + π² + π - f(α) ? ---")
print(f"  x₊ = {x_plus:.16f}")
print(f"  p(π) = {p_pi:.16f}")
diff = p_pi - x_plus
print(f"  p(π) - x₊ = {diff:.16e}")
alpha_val = 1/137.036
print(f"  diff/α = {diff/alpha_val:.10f}")
print(f"  diff/α² = {diff/alpha_val**2:.10f}")
print(f"  diff/π = {diff/PI:.10e}")
print(f"  diff × 137 = {diff * 137:.10f}")
print(f"  diff × 137² = {diff * 137**2:.10f}")

# Bonus: Are there polynomial identities?
print(f"\n  --- Polynomial identity search ---")
print(f"  x₊ satisfies: x₊² - 16G*²x₊ + 16G*³ = 0")
print(f"  So: x₊² = 16G*²x₊ - 16G*³")
print(f"  And: x₊ = 8G*² - 8G*²√(1-1/G*)")
# Verify
print(f"  Verify: 8G*²(1 - √(1-1/G*)) = {8*G_STAR**2*(1-np.sqrt(1-1/G_STAR)):.16f}")
print(f"  x₋ = 8G*²(1 + √(1-1/G*))? No, the other way:")
print(f"  x₊ = 8G*² + 4G*√(16G*²-4G*) ... let me just use the formula")
print(f"  x₊ = [16G*² + √(256G*⁴ - 64G*³)] / 2")
print(f"     = 8G*² + √(64G*³(4G* - 1)) / 2")
print(f"     = 8G*² + 4G*^(3/2)√(4G* - 1)")
val_check = 8*G_STAR**2 + 4*G_STAR**(1.5)*np.sqrt(4*G_STAR - 1)
print(f"     = {val_check:.16f} (should be {x_plus:.16f})")

# Is there a continued fraction relationship?
print(f"\n  --- Continued fraction of x₊ ---")
cf_xp = continued_fraction_coefficients(x_plus)
print(f"  x₊ = [{cf_xp[0]}; {', '.join(map(str, cf_xp[1:]))}]")

print(f"\n  --- Continued fraction of G* ---")
cf_gs = continued_fraction_coefficients(G_STAR)
print(f"  G* = [{cf_gs[0]}; {', '.join(map(str, cf_gs[1:]))}]")

print(f"\n  --- Continued fraction of ϖ ---")
cf_varpi = continued_fraction_coefficients(VARPI)
print(f"  ϖ  = [{cf_varpi[0]}; {', '.join(map(str, cf_varpi[1:]))}]")

# ============================================================================
# BONUS: The complete picture
# ============================================================================
print("\n" + "=" * 80)
print("BONUS: SUMMARY COMPARISON TABLE")
print("=" * 80)

print(f"\n  {'Formula':<45s} {'Value':>20s} {'Error (ppm)':>15s}")
print(f"  {'-'*80}")
print(f"  {'CODATA 2022 (1/α)':<45s} {ALPHA_CODATA:>20.10f} {'(reference)':>15s}")
print(f"  {'4π³ + π² + π  (RFT)':<45s} {p_pi:>20.10f} {abs(p_pi-ALPHA_CODATA)/ALPHA_CODATA*1e6:>15.4f}")
print(f"  {'x₊ from 16G*² quadratic (FTD)':<45s} {x_plus:>20.10f} {abs(x_plus-ALPHA_CODATA)/ALPHA_CODATA*1e6:>15.4f}")

# A few more formulas people have proposed
val_feynman = 1/0.007297352  # approximately 137.036
val_eddington = 137.0  # Eddington's guess
val_wyler = 9/(16*PI**3) * (PI/5)**(1/4) * (245/(2*PI))**0.5  # rough Wyler
# Wyler's formula: α = (9/16π³)(π/5)^(1/4)(245/2π)^(1/2) ... it's actually α not 1/α
# Let me compute it correctly
# Wyler: α = (9/(8π⁴)) × (π⁵/2⁴·5)^(1/4) ... there are various versions
# Skip Wyler, use known approximations

val_simple = 137.0
val_137_036 = 137.036

print(f"  {'137 (Eddington integer)':<45s} {137.0:>20.10f} {abs(137-ALPHA_CODATA)/ALPHA_CODATA*1e6:>15.1f}")

# Also compute: is there a "dual" formula in terms of 1/α?
print(f"\n  --- Can we write 1/α = 4π³ + π² + π + δ? ---")
delta = ALPHA_CODATA - p_pi
print(f"  δ = {delta:.16e}")
print(f"  δ/α = {delta * (1/ALPHA_CODATA):.16e}")
print(f"  δ × α⁻¹ = {delta * ALPHA_CODATA:.16e}")
print(f"  δ/π = {delta/PI:.16e}")
print(f"  δ²  = {delta**2:.16e}")

print(f"\n  --- Is δ related to higher order terms? ---")
# Maybe 1/α = 4π³ + π² + π + f(α)
# f(α) = α²×k for some k?
print(f"  δ/α² = {delta/alpha_val**2:.10f}")
print(f"  δ/(α²π) = {delta/(alpha_val**2*PI):.10f}")
print(f"  δ/(α²/π) = {delta*PI/alpha_val**2:.10f}")
print(f"  δ/(α/π) = {delta*PI/alpha_val:.10f}")
print(f"  δ/(α³) = {delta/alpha_val**3:.10f}")

# Maybe radiative correction style: δ = α/π × something
print(f"  δ/(α/(2π)) = {delta/(alpha_val/(2*PI)):.10f}")
print(f"  δ/(α²/(2π)) = {delta/(alpha_val**2/(2*PI)):.10f}")
print(f"  δ/(α²/(π²)) = {delta/(alpha_val**2/PI**2):.10f}")

print(f"\n  --- Final check: the polynomial coefficients {4,1,1} ---")
print(f"  4 = 2²")
print(f"  1 = 1")
print(f"  1 = 1")
print(f"  Product: 4×1×1 = 4")
print(f"  Sum: 4+1+1 = 6 = 3!")
print(f"  In FTD: coefficient 16 = 4² for master quadratic")
print(f"  Note: 4 = N_base² where N_base = 2 (binary)")
print(f"  Note: p(x) = x(4x²+x+1) — the '4' gives the cubic its power")
print(f"  Derivative: p'(x) = 12x² + 2x + 1")
print(f"  p'(π) = {12*PI**2 + 2*PI + 1:.10f}")
print(f"  This is dp/dπ — the sensitivity of 1/α to changes in π")
print(f"  A 1 ppm shift in π would shift 1/α by {(12*PI**2 + 2*PI + 1)*PI*1e-6:.6e}")

print("\n" + "=" * 80)
print("END OF EXPLORATION")
print("=" * 80)
