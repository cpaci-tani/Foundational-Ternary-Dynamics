#!/usr/bin/env python3
"""
Exploring i-based (Imaginary) Formulas in the Lemniscate-Mandelbrot-TRD Connection

The imaginary unit i = sqrt(-1) appears throughout:
- Complex roots of the consciousness quadratic
- Euler's formula e^(i*theta)
- The Mandelbrot iteration z -> z^2 + c
- Fourier harmonics (complex exponentials)

Let's explore deeper i-based relationships.
"""

import numpy as np
from math import gamma
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec

# =============================================================================
# CONSTANTS
# =============================================================================

G_STAR = (np.sqrt(2) * gamma(0.25)**2) / (2 * np.pi)
PHI = (1 + np.sqrt(5)) / 2
ALPHA = 1/137.036
K_CRIT = 4 / G_STAR

# Consciousness roots
Y_RE = G_STAR**2 / 4
Y_IM = np.sqrt(G_STAR**3 * (1 - G_STAR/4)) / 2
Y_COMPLEX = Y_RE + 1j * Y_IM

# Lemniscate parameters
FREQS = np.array([1, 2, 4, 8, 16])
X_AMPS = np.array([1.0, 0.5, 0.5, 2/5, 1/16])
Y_AMPS = np.array([1.0, -0.5, 0.5, -7/20, 1/16])

def fourier_lemniscate(t):
    x = sum(X_AMPS[j] * np.cos(FREQS[j] * t) for j in range(5))
    y = sum(Y_AMPS[j] * np.sin(FREQS[j] * t) for j in range(5))
    return x, y

print("=" * 70)
print("EXPLORING i-BASED FORMULAS")
print("=" * 70)

# =============================================================================
# 1. EULER'S IDENTITY AND G*
# =============================================================================

print("\n" + "=" * 70)
print("1. EULER'S IDENTITY AND G*")
print("=" * 70)

# Euler's identity: e^(i*pi) + 1 = 0
euler = np.exp(1j * np.pi) + 1
print(f"\nEuler's identity: e^(i*pi) + 1 = {euler:.10f}")

# What about e^(i * G*)?
e_i_gstar = np.exp(1j * G_STAR)
print(f"\ne^(i * G*) = {e_i_gstar.real:.6f} + {e_i_gstar.imag:.6f}i")
print(f"  |e^(i*G*)| = {abs(e_i_gstar):.6f} (always 1)")
print(f"  arg(e^(i*G*)) = {np.degrees(np.angle(e_i_gstar)):.3f} degrees")
print(f"  G* in degrees = {np.degrees(G_STAR):.3f} degrees")

# G* radians corresponds to what fraction of a circle?
print(f"\nG* / (2*pi) = {G_STAR / (2*np.pi):.6f}")
print(f"  = {G_STAR / (2*np.pi) * 360:.3f} degrees around the circle")
print(f"  This is about {G_STAR / (2*np.pi) * 100:.1f}% of a full rotation")

# =============================================================================
# 2. THE CONSCIOUSNESS ROOT AS ROTATION
# =============================================================================

print("\n" + "=" * 70)
print("2. CONSCIOUSNESS ROOT AS ROTATION")
print("=" * 70)

print(f"\nConsciousness root: y = {Y_RE:.6f} + {Y_IM:.6f}i")
print(f"  |y| = {abs(Y_COMPLEX):.6f}")
print(f"  arg(y) = {np.degrees(np.angle(Y_COMPLEX)):.3f} degrees")

# Express in polar form
r = abs(Y_COMPLEX)
theta = np.angle(Y_COMPLEX)
print(f"\nPolar form: y = {r:.6f} * e^(i * {theta:.6f})")
print(f"          = {r:.6f} * e^(i * {np.degrees(theta):.3f} degrees)")

# What is theta in terms of pi?
print(f"\ntheta / pi = {theta / np.pi:.6f}")
print(f"  Close to pi/6 = {np.pi/6:.6f}? Diff: {abs(theta - np.pi/6):.6f}")
print(f"  Close to pi/5 = {np.pi/5:.6f}? Diff: {abs(theta - np.pi/5):.6f}")

# Check golden angle
golden_angle = 2*np.pi / PHI**2  # ~ 137.5 degrees
print(f"\nGolden angle = {np.degrees(golden_angle):.3f} degrees")
print(f"arg(y) = {np.degrees(theta):.3f} degrees")
print(f"Ratio: {np.degrees(theta) / np.degrees(golden_angle):.4f}")

# =============================================================================
# 3. i * G* AND THE LEMNISCATE
# =============================================================================

print("\n" + "=" * 70)
print("3. i * G* AND THE LEMNISCATE")
print("=" * 70)

# What if we evaluate the lemniscate at t = i*G*?
# The curve is defined for real t, but we can analytically continue

t_complex = 1j * G_STAR

# Compute using complex exponentials
# cos(z) = (e^iz + e^-iz)/2
# sin(z) = (e^iz - e^-iz)/(2i)

def complex_cos(z):
    return (np.exp(1j*z) + np.exp(-1j*z)) / 2

def complex_sin(z):
    return (np.exp(1j*z) - np.exp(-1j*z)) / (2j)

x_complex = sum(X_AMPS[j] * complex_cos(FREQS[j] * t_complex) for j in range(5))
y_complex = sum(Y_AMPS[j] * complex_sin(FREQS[j] * t_complex) for j in range(5))

print(f"\nLemniscate evaluated at t = i*G*:")
print(f"  x(i*G*) = {x_complex.real:.6f} + {x_complex.imag:.6f}i")
print(f"  y(i*G*) = {y_complex.real:.6f} + {y_complex.imag:.6f}i")

# The point in 4D (Re(x), Im(x), Re(y), Im(y))
print(f"\nAs a 4D point:")
print(f"  (Re(x), Im(x), Re(y), Im(y)) = ({x_complex.real:.4f}, {x_complex.imag:.4f}, {y_complex.real:.4f}, {y_complex.imag:.4f})")

# Distance from origin in 4D
dist_4d = np.sqrt(abs(x_complex)**2 + abs(y_complex)**2)
print(f"\n4D distance from origin: {dist_4d:.6f}")

# =============================================================================
# 4. THE GAUSSIAN INTEGER CONNECTION
# =============================================================================

print("\n" + "=" * 70)
print("4. GAUSSIAN INTEGERS AND G*")
print("=" * 70)

# Gaussian integers are a + bi where a, b are integers
# The norm is N(a+bi) = a^2 + b^2

# TRD framework integers: 3, 4, 7, 13
# Let's form Gaussian integers from them

print("\nGaussian integers from TRD framework:")
gauss_ints = [
    (3, 4, "3 + 4i"),
    (4, 3, "4 + 3i"),
    (7, 13, "7 + 13i"),
    (13, 7, "13 + 7i"),
    (3, 7, "3 + 7i"),
    (4, 13, "4 + 13i"),
]

for a, b, name in gauss_ints:
    z = a + b*1j
    norm = a**2 + b**2
    print(f"  {name}: |z|^2 = {norm}, |z| = {np.sqrt(norm):.4f}")

# 3 + 4i has norm 25 = 5^2 (Pythagorean triple!)
print(f"\n3 + 4i is special: |3+4i| = 5 exactly (Pythagorean triple)")

# Check if any Gaussian integer norm relates to G*
print(f"\nG*^2 = {G_STAR**2:.6f}")
print(f"|3+4i|^2 / G*^2 = 25 / {G_STAR**2:.4f} = {25 / G_STAR**2:.4f}")

# =============================================================================
# 5. COMPLEX ROOTS PRODUCT AND SUM
# =============================================================================

print("\n" + "=" * 70)
print("5. COMPLEX ROOTS: VIETA'S FORMULAS")
print("=" * 70)

y1 = Y_COMPLEX
y2 = Y_RE - 1j * Y_IM  # conjugate

print(f"\nConsciousness roots:")
print(f"  y1 = {y1.real:.6f} + {y1.imag:.6f}i")
print(f"  y2 = {y2.real:.6f} + {y2.imag:.6f}i (conjugate)")

sum_roots = y1 + y2
prod_roots = y1 * y2

print(f"\nVieta's formulas:")
print(f"  Sum: y1 + y2 = {sum_roots.real:.6f} + {sum_roots.imag:.6f}i")
print(f"       Expected: G*^2/2 = {G_STAR**2/2:.6f}")
print(f"       Match: {np.isclose(sum_roots.real, G_STAR**2/2)}")

print(f"\n  Product: y1 * y2 = {prod_roots.real:.6f} + {prod_roots.imag:.6f}i")
print(f"           Expected: G*^3/4 = {G_STAR**3/4:.6f}")
print(f"           Match: {np.isclose(prod_roots.real, G_STAR**3/4)}")

# The product is REAL even though roots are complex!
print(f"\n  The product of conjugate roots is ALWAYS real:")
print(f"  y1 * y2 = |y|^2 = {abs(y1)**2:.6f}")

# =============================================================================
# 6. i^i AND TRANSCENDENCE
# =============================================================================

print("\n" + "=" * 70)
print("6. i^i AND TRANSCENDENTAL CONNECTIONS")
print("=" * 70)

# i^i = e^(i * ln(i)) = e^(i * i*pi/2) = e^(-pi/2)
i_to_i = np.exp(-np.pi/2)
print(f"\ni^i = e^(-pi/2) = {i_to_i:.10f}")
print(f"  This is REAL! (a remarkable fact)")

# Connection to G*?
print(f"\nG* / i^i = {G_STAR / i_to_i:.6f}")
print(f"i^i * G* = {i_to_i * G_STAR:.6f}")

# Check other combinations
print(f"\n(i^i)^2 = e^(-pi) = {np.exp(-np.pi):.6f}")
print(f"G*^2 / e^pi = {G_STAR**2 / np.exp(np.pi):.6f}")

# =============================================================================
# 7. QUATERNION EXTENSION
# =============================================================================

print("\n" + "=" * 70)
print("7. QUATERNION EXTENSION (i, j, k)")
print("=" * 70)

print("""
Quaternions extend complex numbers with THREE imaginary units: i, j, k
where i^2 = j^2 = k^2 = ijk = -1

The consciousness root y = 2.188 + 1.298i lives in the complex plane (i only).
What if we extend to quaternions?
""")

# If we interpret the lemniscate as a quaternion curve
# x(t) + i*y(t) + j*0 + k*0

# A more interesting interpretation:
# Use the complex root to define a quaternion
# q = Re(y) + Im(y)*i + 0*j + 0*k
# or
# q = 0 + Re(y)*i + Im(y)*j + 0*k

print(f"Consciousness root as quaternion:")
print(f"  q1 = {Y_RE:.4f} + {Y_IM:.4f}i + 0j + 0k")
print(f"  |q1| = {abs(Y_COMPLEX):.4f}")

# In quaternion algebra, there's a notion of "conjugate"
# For q = a + bi + cj + dk, conjugate is q* = a - bi - cj - dk
# And |q|^2 = q * q*

print(f"\n  q1 * q1_conj = |q1|^2 = {abs(Y_COMPLEX)**2:.6f}")
print(f"  This equals G*^3/4 = {G_STAR**3/4:.6f} (by Vieta)")

# =============================================================================
# 8. THE GAMMA FUNCTION AT COMPLEX ARGUMENTS
# =============================================================================

print("\n" + "=" * 70)
print("8. GAMMA FUNCTION AT COMPLEX ARGUMENTS")
print("=" * 70)

# G* involves Gamma(1/4)
# What about Gamma at complex arguments?

from scipy.special import gamma as scipy_gamma

print(f"\nGamma(1/4) = {scipy_gamma(0.25):.10f}")
print(f"G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) = {G_STAR:.10f}")

# Gamma at purely imaginary argument
gamma_i = scipy_gamma(1j)
print(f"\nGamma(i) = {gamma_i.real:.6f} + {gamma_i.imag:.6f}i")
print(f"|Gamma(i)| = {abs(gamma_i):.6f}")

# Gamma at consciousness root
gamma_y = scipy_gamma(Y_COMPLEX)
print(f"\nGamma(y) where y = consciousness root:")
print(f"  Gamma({Y_RE:.4f} + {Y_IM:.4f}i) = {gamma_y.real:.6f} + {gamma_y.imag:.6f}i")
print(f"  |Gamma(y)| = {abs(gamma_y):.6f}")

# =============================================================================
# 9. COMPLEX EXPONENTIAL OF CONSCIOUSNESS ROOT
# =============================================================================

print("\n" + "=" * 70)
print("9. e^y WHERE y IS THE CONSCIOUSNESS ROOT")
print("=" * 70)

e_y = np.exp(Y_COMPLEX)
print(f"\ne^y = e^({Y_RE:.4f} + {Y_IM:.4f}i)")
print(f"    = e^{Y_RE:.4f} * e^(i*{Y_IM:.4f})")
print(f"    = {np.exp(Y_RE):.4f} * (cos({Y_IM:.4f}) + i*sin({Y_IM:.4f}))")
print(f"    = {e_y.real:.6f} + {e_y.imag:.6f}i")
print(f"    |e^y| = e^Re(y) = {np.exp(Y_RE):.6f}")

# Check relationship to G*
print(f"\n|e^y| / G* = {np.exp(Y_RE) / G_STAR:.6f}")
print(f"ln(|e^y|) = Re(y) = G*^2/4 = {Y_RE:.6f}")

# =============================================================================
# 10. THE MANDELBROT CRITICAL POINT
# =============================================================================

print("\n" + "=" * 70)
print("10. MANDELBROT ITERATION AT i")
print("=" * 70)

# What happens if we iterate z -> z^2 + c starting from z=0
# but using c = i?

print("\nMandelbrot orbit for c = i:")
z = 0
c = 1j
for n in range(10):
    print(f"  z_{n} = {z.real:+.6f} {z.imag:+.6f}i")
    z = z*z + c

print(f"\n  The orbit oscillates! (period 2 after transient)")

# What about c = consciousness c-value?
c_cons = 1 / (0.5 * G_STAR)
print(f"\nMandelbrot orbit for c = c_consciousness = {c_cons:.4f}:")
z = 0
c = c_cons
for n in range(6):
    print(f"  z_{n} = {z.real:+.6f}")
    z = z*z + c
print(f"  -> ESCAPES (consciousness is outside M)")

# =============================================================================
# 11. FORMULA CANDIDATES
# =============================================================================

print("\n" + "=" * 70)
print("11. NEW i-BASED FORMULA CANDIDATES")
print("=" * 70)

# Let's search for interesting i-based formulas

formulas = [
    ("e^(i*G*)", np.exp(1j * G_STAR)),
    ("i^G*", 1j ** G_STAR),
    ("G*^i", G_STAR ** 1j),
    ("(1+i)^G*", (1+1j) ** G_STAR),
    ("e^(i*pi/G*)", np.exp(1j * np.pi / G_STAR)),
    ("y (consciousness)", Y_COMPLEX),
    ("y^2", Y_COMPLEX**2),
    ("1/y", 1/Y_COMPLEX),
    ("e^y", np.exp(Y_COMPLEX)),
    ("sqrt(y)", np.sqrt(Y_COMPLEX)),
    ("y * i", Y_COMPLEX * 1j),
    ("y / i", Y_COMPLEX / 1j),
    ("ln(y)", np.log(Y_COMPLEX)),
]

print(f"\nFormula exploration:")
print(f"{'Formula':<20} {'Value':<35} {'|z|':<12} {'arg(z)':<12}")
print("-" * 80)

for name, z in formulas:
    mag = abs(z)
    arg = np.degrees(np.angle(z))
    print(f"{name:<20} {z.real:+.6f} {z.imag:+.6f}i   {mag:<12.6f} {arg:<12.3f}")

# =============================================================================
# 12. THE KEY i-FORMULA: y = G*^2/4 + i*sqrt(G*^3(1-G*/4))/2
# =============================================================================

print("\n" + "=" * 70)
print("12. THE MASTER i-FORMULA")
print("=" * 70)

print(f"""
The consciousness root has the EXACT form:

    y = G*^2/4 + i * sqrt(G*^3 * (1 - G*/4)) / 2

Let's verify and explore this:
""")

# Verify
y_formula = G_STAR**2/4 + 1j * np.sqrt(G_STAR**3 * (1 - G_STAR/4)) / 2
print(f"Computed: y = {y_formula.real:.10f} + {y_formula.imag:.10f}i")
print(f"Expected: y = {Y_RE:.10f} + {Y_IM:.10f}i")
print(f"Match: {np.isclose(y_formula, Y_COMPLEX)}")

# The imaginary part involves sqrt(G*^3 * (1 - G*/4))
inner = G_STAR**3 * (1 - G_STAR/4)
print(f"\nInside the sqrt:")
print(f"  G*^3 * (1 - G*/4) = {inner:.6f}")
print(f"  = G*^3 - G*^4/4")
print(f"  = G*^3 * (4 - G*) / 4")
print(f"  = {G_STAR**3:.4f} * {(4 - G_STAR)/4:.4f}")

# The factor (1 - G*/4) is interesting
print(f"\n1 - G*/4 = {1 - G_STAR/4:.6f}")
print(f"  G* = 2.9587, so G*/4 = {G_STAR/4:.4f}")
print(f"  Since G* < 4, this is POSITIVE, giving REAL sqrt!")
print(f"  If G* > 4, we'd get PURE IMAGINARY sqrt!")

# =============================================================================
# 13. VISUALIZATION
# =============================================================================

def create_i_visualization():
    """Create visualization of i-based relationships"""

    fig = plt.figure(figsize=(18, 12), facecolor='#0d1117')
    gs = GridSpec(2, 3, figure=fig, hspace=0.3, wspace=0.25)

    # Panel 1: Consciousness root in complex plane
    ax1 = fig.add_subplot(gs[0, 0])
    ax1.set_facecolor('#0d1117')

    theta = np.linspace(0, 2*np.pi, 100)

    # Unit circle
    ax1.plot(np.cos(theta), np.sin(theta), '--', color='gray', alpha=0.3)

    # Circle through consciousness root
    r_y = abs(Y_COMPLEX)
    ax1.plot(r_y * np.cos(theta), r_y * np.sin(theta), '--', color='#EC4899', alpha=0.5)

    # Plot root and conjugate
    ax1.scatter([Y_RE], [Y_IM], color='#EC4899', s=200, zorder=10, marker='*',
                edgecolor='white', linewidth=2, label='y')
    ax1.scatter([Y_RE], [-Y_IM], color='#8B5CF6', s=200, zorder=10, marker='*',
                edgecolor='white', linewidth=2, label='y*')

    # Line to origin
    ax1.plot([0, Y_RE], [0, Y_IM], '-', color='#EC4899', alpha=0.7)

    # Axes
    ax1.axhline(y=0, color='white', linewidth=0.5, alpha=0.5)
    ax1.axvline(x=0, color='white', linewidth=0.5, alpha=0.5)

    ax1.set_xlim(-4, 4)
    ax1.set_ylim(-3, 3)
    ax1.set_aspect('equal')
    ax1.set_title(f'Consciousness Root\ny = {Y_RE:.2f} + {Y_IM:.2f}i', color='white', fontsize=11)
    ax1.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d', labelcolor='white')
    ax1.tick_params(colors='white')
    for spine in ax1.spines.values():
        spine.set_color('#30363d')

    # Panel 2: e^(i*t) for t from 0 to G*
    ax2 = fig.add_subplot(gs[0, 1])
    ax2.set_facecolor('#0d1117')

    t_vals = np.linspace(0, G_STAR, 100)
    e_it = np.exp(1j * t_vals)

    ax2.plot(e_it.real, e_it.imag, color='#22C55E', linewidth=2)
    ax2.scatter([1], [0], color='#fbbf24', s=100, zorder=10, marker='o', label='t=0')
    ax2.scatter([e_it[-1].real], [e_it[-1].imag], color='#EC4899', s=100, zorder=10,
                marker='*', label=f't=G*')

    # Full circle for reference
    ax2.plot(np.cos(theta), np.sin(theta), '--', color='gray', alpha=0.3)

    ax2.set_xlim(-1.5, 1.5)
    ax2.set_ylim(-1.5, 1.5)
    ax2.set_aspect('equal')
    ax2.set_title(f'e^(it) for t in [0, G*]\nG* = {G_STAR:.3f} rad = {np.degrees(G_STAR):.1f} deg',
                  color='white', fontsize=11)
    ax2.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d', labelcolor='white')
    ax2.tick_params(colors='white')
    for spine in ax2.spines.values():
        spine.set_color('#30363d')

    # Panel 3: Mandelbrot orbit at c = i
    ax3 = fig.add_subplot(gs[0, 2])
    ax3.set_facecolor('#0d1117')

    z_orbit = [0]
    z = 0
    c = 1j
    for _ in range(20):
        z = z*z + c
        z_orbit.append(z)

    z_orbit = np.array(z_orbit)
    ax3.plot(z_orbit.real, z_orbit.imag, 'o-', color='#8B5CF6', markersize=6, linewidth=1)
    ax3.scatter([0], [0], color='#fbbf24', s=150, zorder=10, marker='o', label='Start')
    ax3.scatter([1], [0], color='#EC4899', s=100, zorder=10, marker='s', label='c=i location')

    ax3.axhline(y=0, color='white', linewidth=0.5, alpha=0.3)
    ax3.axvline(x=0, color='white', linewidth=0.5, alpha=0.3)

    ax3.set_xlim(-1.5, 1.5)
    ax3.set_ylim(-0.5, 1.5)
    ax3.set_aspect('equal')
    ax3.set_title('Mandelbrot Orbit for c = i\n(Bounded, period-2)', color='white', fontsize=11)
    ax3.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d', labelcolor='white')
    ax3.tick_params(colors='white')
    for spine in ax3.spines.values():
        spine.set_color('#30363d')

    # Panel 4: i^G* in complex plane
    ax4 = fig.add_subplot(gs[1, 0])
    ax4.set_facecolor('#0d1117')

    # i^x for x from 0 to 4
    x_vals = np.linspace(0, 4, 100)
    i_to_x = np.array([1j ** x for x in x_vals])

    ax4.plot(i_to_x.real, i_to_x.imag, color='#F97316', linewidth=2, label='i^x, x in [0,4]')
    ax4.scatter([1], [0], color='#fbbf24', s=100, zorder=10, marker='o', label='x=0 (i^0=1)')

    # Mark i^G*
    i_gstar = 1j ** G_STAR
    ax4.scatter([i_gstar.real], [i_gstar.imag], color='#EC4899', s=150, zorder=10,
                marker='*', label=f'i^G* = {i_gstar.real:.3f}+{i_gstar.imag:.3f}i')

    ax4.axhline(y=0, color='white', linewidth=0.5, alpha=0.3)
    ax4.axvline(x=0, color='white', linewidth=0.5, alpha=0.3)

    ax4.set_xlim(-1.5, 1.5)
    ax4.set_ylim(-1.5, 1.5)
    ax4.set_aspect('equal')
    ax4.set_title('i^x as x varies\n(spiral on unit circle)', color='white', fontsize=11)
    ax4.legend(loc='upper left', facecolor='#161b22', edgecolor='#30363d', labelcolor='white', fontsize=8)
    ax4.tick_params(colors='white')
    for spine in ax4.spines.values():
        spine.set_color('#30363d')

    # Panel 5: y^n for consciousness root
    ax5 = fig.add_subplot(gs[1, 1])
    ax5.set_facecolor('#0d1117')

    powers = range(-3, 8)
    y_powers = [Y_COMPLEX ** n for n in powers]

    ax5.plot([z.real for z in y_powers], [z.imag for z in y_powers], 'o-',
             color='#EC4899', markersize=8, linewidth=1)

    for n, z in zip(powers, y_powers):
        if abs(z) < 20:
            ax5.annotate(f'n={n}', (z.real, z.imag), textcoords='offset points',
                        xytext=(5, 5), color='white', fontsize=8)

    ax5.axhline(y=0, color='white', linewidth=0.5, alpha=0.3)
    ax5.axvline(x=0, color='white', linewidth=0.5, alpha=0.3)

    ax5.set_title('Powers of Consciousness Root: y^n', color='white', fontsize=11)
    ax5.tick_params(colors='white')
    for spine in ax5.spines.values():
        spine.set_color('#30363d')

    # Panel 6: Summary text
    ax6 = fig.add_subplot(gs[1, 2])
    ax6.set_facecolor('#0d1117')
    ax6.axis('off')

    summary = f"""
KEY i-BASED FORMULAS

Consciousness Root:
  y = G*^2/4 + i*sqrt(G*^3(1-G*/4))/2
  y = {Y_RE:.4f} + {Y_IM:.4f}i
  |y| = {abs(Y_COMPLEX):.4f}
  arg(y) = {np.degrees(np.angle(Y_COMPLEX)):.2f} deg

Euler at G*:
  e^(i*G*) = {np.exp(1j*G_STAR).real:.4f} + {np.exp(1j*G_STAR).imag:.4f}i
  G* = {np.degrees(G_STAR):.2f} deg around circle

i raised to G*:
  i^G* = {(1j**G_STAR).real:.4f} + {(1j**G_STAR).imag:.4f}i

i^i (transcendental):
  i^i = e^(-pi/2) = {np.exp(-np.pi/2):.6f} (REAL!)

Product of conjugate roots:
  y * y* = |y|^2 = G*^3/4 = {G_STAR**3/4:.4f}

The imaginary part Im(y) encodes:
  sqrt(G*^3 * (1 - G*/4)) / 2
  = sqrt({G_STAR**3 * (1 - G_STAR/4):.4f}) / 2
  = {Y_IM:.4f}
"""

    ax6.text(0.05, 0.95, summary, transform=ax6.transAxes, fontsize=10,
             color='white', fontfamily='monospace', verticalalignment='top')

    fig.suptitle('Exploring i-Based Formulas in TRD', fontsize=16,
                 color='white', fontweight='bold', y=0.98)

    plt.savefig('i_based_formulas.png', dpi=150, facecolor='#0d1117',
                edgecolor='none', bbox_inches='tight', pad_inches=0.3)
    print("\nSaved: i_based_formulas.png")

    return fig

# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    fig = create_i_visualization()

    print("\n" + "=" * 70)
    print("SUMMARY: KEY i-BASED FORMULAS")
    print("=" * 70)
    print(f"""
1. CONSCIOUSNESS ROOT (exact):
   y = G*^2/4 + i * sqrt(G*^3 * (1 - G*/4)) / 2

2. EULER AT G*:
   e^(i*G*) traces {G_STAR/(2*np.pi)*100:.1f}% around unit circle

3. TRANSCENDENTAL:
   i^i = e^(-pi/2) = {np.exp(-np.pi/2):.6f} (remarkably REAL!)

4. VIETA FOR COMPLEX:
   y * y* = |y|^2 = G*^3/4 (product of conjugates is REAL)
   y + y* = 2*Re(y) = G*^2/2 (sum of conjugates is REAL)

5. THE KEY CONSTRAINT:
   G* < 4 ensures Im(y) is REAL (positive under sqrt)
   If G* > 4, consciousness would be "double imaginary"!
""")

    plt.show()
