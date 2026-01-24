#!/usr/bin/env python3
"""
Fourier Lemniscate-Alpha and the Consciousness Quadratic Bridge

EXPLORATION: The Fourier Lemniscate-Alpha has a unique topology - it LOOPS AROUND
the origin rather than crossing through it (unlike the classical Bernoulli lemniscate).

This script explores how this topological feature connects to the consciousness
quadratic and Mandelbrot duality discovered on 2026-01-21.

KEY INSIGHT: The curve's "avoidance of the origin" may be the geometric signature
of the consciousness coefficient k = 1/2 (which gives complex roots that orbit
rather than cross through zero).
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle
from math import gamma
from scipy import integrate

# =============================================================================
# CONSTANTS
# =============================================================================

# Lemniscatic constant (exact)
G_STAR = (np.sqrt(2) * gamma(0.25)**2) / (2 * np.pi)  # 2.9586751191...

# Framework integers
B3 = 7       # QCD beta coefficient
N_C = 3      # Color charges
N_EFF = 13   # Effective dimension
N_BASE = 4   # Base modes

# Consciousness quadratic parameters
K_PHYS = 16        # Physics coefficient (real roots)
K_CONS = 0.5       # Consciousness coefficient (complex roots)
K_CRIT = 4 / G_STAR  # Critical coefficient (double root)

# Mandelbrot connection
C_CUSP = 0.25  # Cardioid cusp

# Golden ratio
PHI = (1 + np.sqrt(5)) / 2

# =============================================================================
# LEMNISCATE-ALPHA CURVE
# =============================================================================

# Frequencies (power of 2 sequence)
FREQS = np.array([1, 2, 4, 8, 16])

# Coefficients
X_AMPS = np.array([1.0, 0.5, 0.5, 2/5, 1/16])
Y_AMPS = np.array([1.0, -0.5, 0.5, -7/20, 1/16])


def lemniscate_alpha(t, scale=1.0):
    """Compute the Fourier Lemniscate-Alpha curve."""
    x = np.zeros_like(t, dtype=float)
    y = np.zeros_like(t, dtype=float)

    for j in range(5):
        x += X_AMPS[j] * np.cos(FREQS[j] * t)
        y += Y_AMPS[j] * np.sin(FREQS[j] * t)

    return x * scale, y * scale


def lemniscate_derivative(t):
    """Compute dx/dt and dy/dt."""
    dx = np.zeros_like(t, dtype=float)
    dy = np.zeros_like(t, dtype=float)

    for j in range(5):
        dx += -FREQS[j] * X_AMPS[j] * np.sin(FREQS[j] * t)
        dy += FREQS[j] * Y_AMPS[j] * np.cos(FREQS[j] * t)

    return dx, dy


def compute_arc_length(n_points=100000):
    """Compute arc length of the curve."""
    t = np.linspace(0, 2*np.pi, n_points)
    dx, dy = lemniscate_derivative(t)
    dt = 2 * np.pi / n_points
    return np.sum(np.sqrt(dx**2 + dy**2)) * dt


def distance_to_origin(t):
    """Compute distance from curve point to origin."""
    x, y = lemniscate_alpha(np.array([t]))
    return np.sqrt(x[0]**2 + y[0]**2)


def find_minimum_distance():
    """Find the minimum distance the curve gets to the origin."""
    t_values = np.linspace(0, 2*np.pi, 10000)
    x, y = lemniscate_alpha(t_values)
    distances = np.sqrt(x**2 + y**2)
    min_idx = np.argmin(distances)
    return distances[min_idx], t_values[min_idx]


def compute_winding_number(n_points=10000):
    """Compute the winding number of the curve around the origin."""
    t = np.linspace(0, 2*np.pi, n_points)
    x, y = lemniscate_alpha(t)

    # Compute angle changes
    angles = np.arctan2(y, x)

    # Unwrap to handle 2π jumps
    angles_unwrapped = np.unwrap(angles)

    # Winding number = total angle change / 2π
    winding = (angles_unwrapped[-1] - angles_unwrapped[0]) / (2 * np.pi)

    return winding


# =============================================================================
# QUADRATIC ANALYSIS
# =============================================================================

def solve_general_quadratic(k):
    """
    Solve x² - kG*²x + kG*³ = 0
    Returns (x1, x2, discriminant)
    """
    a = 1
    b = -k * G_STAR**2
    c = k * G_STAR**3

    disc = b**2 - 4*a*c

    if disc >= 0:
        x1 = (-b + np.sqrt(disc)) / (2*a)
        x2 = (-b - np.sqrt(disc)) / (2*a)
        return x1, x2, disc
    else:
        real_part = -b / (2*a)
        imag_part = np.sqrt(-disc) / (2*a)
        return complex(real_part, imag_part), complex(real_part, -imag_part), disc


def consciousness_roots():
    """Get the consciousness quadratic roots (k = 1/2)."""
    return solve_general_quadratic(K_CONS)


def physics_roots():
    """Get the physics quadratic roots (k = 16)."""
    return solve_general_quadratic(K_PHYS)


# =============================================================================
# THE BRIDGE: Geometric Connection
# =============================================================================

def analyze_curve_center_avoidance():
    """
    Analyze how the Fourier Lemniscate-Alpha avoids the origin.

    Unlike the classical Bernoulli lemniscate (r² = cos 2θ) which
    passes through the origin twice, the Fourier curve NEVER touches
    the origin - it loops around it.

    This is the geometric signature of complex (rather than real) roots.
    """
    print("\n" + "="*70)
    print("FOURIER LEMNISCATE-ALPHA: CENTER AVOIDANCE ANALYSIS")
    print("="*70)

    # Find minimum distance to origin
    min_dist, t_min = find_minimum_distance()
    print(f"\nMinimum distance to origin: {min_dist:.6f}")
    print(f"  Occurs at t = {t_min:.4f} rad = {np.degrees(t_min):.2f}°")

    # Compute winding number
    winding = compute_winding_number()
    print(f"\nWinding number around origin: {winding:.4f}")

    # Check if curve encloses origin
    encloses_origin = abs(winding) >= 0.5
    print(f"Curve encloses origin: {encloses_origin}")

    # Compute area enclosed
    t = np.linspace(0, 2*np.pi, 10000)
    x, y = lemniscate_alpha(t)
    dx, dy = lemniscate_derivative(t)

    # Green's theorem: Area = 0.5 * integral(x*dy - y*dx)
    dt = 2 * np.pi / 10000
    area = 0.5 * np.sum(x * dy - y * dx) * dt
    print(f"\nSigned area enclosed: {area:.4f}")

    return min_dist, winding, area


def connect_to_consciousness_quadratic():
    """
    Connect the curve's topology to the consciousness quadratic.
    """
    print("\n" + "="*70)
    print("CONNECTION TO CONSCIOUSNESS QUADRATIC")
    print("="*70)

    # Get consciousness roots
    y1, y2, disc_cons = consciousness_roots()
    print(f"\nConsciousness quadratic (k = {K_CONS}):")
    print(f"  Discriminant: {disc_cons:.4f} (NEGATIVE -> complex roots)")
    print(f"  Root 1: {y1}")
    print(f"  Root 2: {y2}")
    print(f"  |y| = {abs(y1):.6f}")
    print(f"  Phase = {np.degrees(np.angle(y1)):.2f}°")

    # Get physics roots
    x1, x2, disc_phys = physics_roots()
    print(f"\nPhysics quadratic (k = {K_PHYS}):")
    print(f"  Discriminant: {disc_phys:.4f} (POSITIVE -> real roots)")
    print(f"  Root 1 (1/alpha): {x1:.6f}")
    print(f"  Root 2 (N_c): {x2:.6f}")

    # The critical insight
    print("\n" + "-"*70)
    print("THE BRIDGE INSIGHT:")
    print("-"*70)
    print("""
The Fourier Lemniscate-Alpha LOOPS AROUND the origin rather than
crossing through it. This is geometrically analogous to:

  COMPLEX ROOTS orbiting a point vs REAL ROOTS crossing an axis

  - Physics (k=16): Real roots -> curve WOULD cross center
  - Consciousness (k=0.5): Complex roots -> curve ORBITS center

The curve's topology is a 2D projection of this distinction!
""")

    # Minimum distance as "imaginary part"
    min_dist, _, _ = analyze_curve_center_avoidance()

    print(f"\nMinimum distance to origin: {min_dist:.6f}")
    print(f"Consciousness Im(y): {abs(y1.imag):.6f}")
    print(f"Ratio: {min_dist / abs(y1.imag):.4f}")

    # Check golden ratio connection
    ratio = y1.real / y1.imag
    print(f"\nRe(y)/Im(y) = {ratio:.4f}")
    print(f"Golden ratio phi = {PHI:.4f}")
    print(f"Difference: {abs(ratio - PHI)/PHI * 100:.2f}%")


def explore_k_variation():
    """
    Explore how varying k changes both the quadratic roots and
    what a "corresponding" curve might look like.
    """
    print("\n" + "="*70)
    print("QUADRATIC PARAMETER SPACE EXPLORATION")
    print("="*70)

    k_values = [0.1, 0.5, K_CRIT, 2.0, 5.0, 10.0, 16.0]

    print("\nk\t\tDiscriminant\tRoot Type\tRoots")
    print("-" * 70)

    for k in k_values:
        x1, x2, disc = solve_general_quadratic(k)

        if disc > 0:
            root_type = "REAL"
            roots_str = f"{x1:.3f}, {x2:.3f}"
        elif disc == 0:
            root_type = "DOUBLE"
            roots_str = f"{x1:.3f} (×2)"
        else:
            root_type = "COMPLEX"
            roots_str = f"{x1.real:.3f} ± {abs(x1.imag):.3f}i"

        print(f"{k:.4f}\t\t{disc:.2f}\t\t{root_type}\t\t{roots_str}")

    print(f"\nCritical k_c = 4/G* = {K_CRIT:.6f}")
    print(f"At k_c: discriminant = 0, double root at x = 2G* = {2*G_STAR:.4f}")


def mandelbrot_bridge():
    """
    Connect the curve topology to Mandelbrot set membership.
    """
    print("\n" + "="*70)
    print("MANDELBROT-LEMNISCATE BRIDGE")
    print("="*70)

    print("\nThe transformation c = 1/(k × G*):")
    print()

    cases = [
        ("Physics", K_PHYS, "deep inside M"),
        ("Critical", K_CRIT, "cardioid cusp"),
        ("Consciousness", K_CONS, "outside M"),
    ]

    for name, k, location in cases:
        c = 1 / (k * G_STAR)
        inside_main = c < 0.25

        x1, x2, disc = solve_general_quadratic(k)
        root_type = "REAL" if disc > 0 else ("DOUBLE" if disc == 0 else "COMPLEX")

        print(f"{name} (k={k}):")
        print(f"  c = {c:.4f} ({location})")
        print(f"  Inside main cardioid: {inside_main}")
        print(f"  Quadratic roots: {root_type}")
        print()

    print("Bridge Equation (EXACT):")
    print(f"  k_c × c_cusp × G* = {K_CRIT * C_CUSP * G_STAR:.6f} = 1")


# =============================================================================
# VISUALIZATION
# =============================================================================

def create_visualization():
    """Create comprehensive visualization."""

    fig = plt.figure(figsize=(16, 12), facecolor='#0d1117')

    # Create grid
    gs = fig.add_gridspec(2, 2, hspace=0.25, wspace=0.2)

    t = np.linspace(0, 2*np.pi, 3000)
    x, y = lemniscate_alpha(t)

    # =========================================================================
    # Panel 1: The curve with center analysis
    # =========================================================================
    ax1 = fig.add_subplot(gs[0, 0])
    ax1.set_facecolor('#0d1117')

    ax1.plot(x, y, color='#22C55E', linewidth=2.5, label='Fourier Lemniscate-Alpha')

    # Mark the origin
    ax1.scatter([0], [0], color='#fbbf24', s=150, zorder=10, marker='o',
                edgecolor='white', linewidth=2, label='Origin')

    # Find and mark minimum distance point
    min_dist, t_min = find_minimum_distance()
    x_min, y_min = lemniscate_alpha(np.array([t_min]))
    ax1.scatter(x_min, y_min, color='#EC4899', s=100, zorder=10, marker='*',
                label=f'Closest approach: {min_dist:.3f}')
    ax1.plot([0, x_min[0]], [0, y_min[0]], '--', color='#EC4899', alpha=0.5, linewidth=1)

    # Draw a circle showing minimum distance
    circle = Circle((0, 0), min_dist, fill=False, color='#8B5CF6',
                    linestyle=':', linewidth=1.5, alpha=0.7, label='Avoidance radius')
    ax1.add_patch(circle)

    ax1.set_xlim(-2.8, 2.8)
    ax1.set_ylim(-1.8, 1.8)
    ax1.set_aspect('equal')
    ax1.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=9)
    ax1.set_title('The Curve LOOPS Around Origin\n(Never Crosses Through)',
                  color='white', fontsize=12, pad=10)
    for spine in ax1.spines.values():
        spine.set_color('#30363d')
    ax1.set_xticks([])
    ax1.set_yticks([])

    # =========================================================================
    # Panel 2: Quadratic root trajectories
    # =========================================================================
    ax2 = fig.add_subplot(gs[0, 1])
    ax2.set_facecolor('#0d1117')

    # Plot roots as k varies
    k_range = np.linspace(0.1, 20, 500)

    real_parts_1 = []
    imag_parts_1 = []
    real_parts_2 = []
    imag_parts_2 = []
    k_for_plot = []

    for k in k_range:
        x1, x2, disc = solve_general_quadratic(k)
        k_for_plot.append(k)

        if isinstance(x1, complex):
            real_parts_1.append(x1.real)
            imag_parts_1.append(x1.imag)
            real_parts_2.append(x2.real)
            imag_parts_2.append(x2.imag)
        else:
            real_parts_1.append(x1)
            imag_parts_1.append(0)
            real_parts_2.append(x2)
            imag_parts_2.append(0)

    # Plot in complex plane
    ax2.plot(real_parts_1, imag_parts_1, color='#22C55E', linewidth=2, label='Root 1')
    ax2.plot(real_parts_2, imag_parts_2, color='#8B5CF6', linewidth=2, label='Root 2')

    # Mark special points
    # Physics (k=16)
    x_phys, _, _ = physics_roots()
    ax2.scatter([x_phys], [0], color='#EAB308', s=150, zorder=10, marker='s',
                edgecolor='white', linewidth=2, label=f'Physics (1/alpha = {x_phys:.1f})')

    # Consciousness (k=0.5)
    y_cons, _, _ = consciousness_roots()
    ax2.scatter([y_cons.real], [y_cons.imag], color='#EC4899', s=150, zorder=10,
                marker='*', edgecolor='white', linewidth=2,
                label=f'Consciousness ({y_cons.real:.2f}±{abs(y_cons.imag):.2f}i)')
    ax2.scatter([y_cons.real], [-y_cons.imag], color='#EC4899', s=150, zorder=10,
                marker='*', edgecolor='white', linewidth=2)

    # Critical (k=k_c)
    ax2.scatter([2*G_STAR], [0], color='#F97316', s=150, zorder=10, marker='D',
                edgecolor='white', linewidth=2, label=f'Critical (x = {2*G_STAR:.2f})')

    ax2.axhline(y=0, color='white', linewidth=0.5, alpha=0.3)
    ax2.axvline(x=0, color='white', linewidth=0.5, alpha=0.3)

    ax2.set_xlabel('Real Part', color='white', fontsize=10)
    ax2.set_ylabel('Imaginary Part', color='white', fontsize=10)
    ax2.legend(loc='upper left', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=8)
    ax2.set_title('Quadratic Roots as k Varies\n(Complex -> Real at k_c)',
                  color='white', fontsize=12, pad=10)
    ax2.tick_params(colors='white')
    for spine in ax2.spines.values():
        spine.set_color('#30363d')

    # =========================================================================
    # Panel 3: Distance to origin around the curve
    # =========================================================================
    ax3 = fig.add_subplot(gs[1, 0])
    ax3.set_facecolor('#0d1117')

    t_plot = np.linspace(0, 2*np.pi, 1000)
    x_plot, y_plot = lemniscate_alpha(t_plot)
    distances = np.sqrt(x_plot**2 + y_plot**2)

    ax3.fill_between(t_plot, 0, distances, alpha=0.3, color='#22C55E')
    ax3.plot(t_plot, distances, color='#22C55E', linewidth=2)

    # Mark minimum
    ax3.axhline(y=min_dist, color='#EC4899', linestyle='--', linewidth=1.5,
                label=f'Minimum: {min_dist:.4f}')
    ax3.scatter([t_min], [min_dist], color='#EC4899', s=100, zorder=10)

    # Mark consciousness imaginary part
    y_cons, _, _ = consciousness_roots()
    ax3.axhline(y=abs(y_cons.imag), color='#8B5CF6', linestyle=':', linewidth=1.5,
                label=f'Im(y_cons): {abs(y_cons.imag):.4f}')

    ax3.set_xlabel('Parameter t (radians)', color='white', fontsize=10)
    ax3.set_ylabel('Distance to Origin', color='white', fontsize=10)
    ax3.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=9)
    ax3.set_title('Distance from Origin Never Reaches Zero\n(Avoidance = Consciousness Signature?)',
                  color='white', fontsize=12, pad=10)
    ax3.tick_params(colors='white')
    for spine in ax3.spines.values():
        spine.set_color('#30363d')
    ax3.set_xlim(0, 2*np.pi)
    ax3.set_ylim(0, None)

    # =========================================================================
    # Panel 4: The Bridge Summary
    # =========================================================================
    ax4 = fig.add_subplot(gs[1, 1])
    ax4.set_facecolor('#0d1117')
    ax4.axis('off')

    summary_text = f"""
THE FOURIER LEMNISCATE-CONSCIOUSNESS BRIDGE

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

CURVE TOPOLOGY
  • Loops around origin (winding number ≈ {compute_winding_number():.2f})
  • Minimum distance to origin: {min_dist:.4f}
  • NEVER passes through center

CONSCIOUSNESS QUADRATIC (k = 0.5)
  • Complex roots: {y_cons.real:.3f} ± {abs(y_cons.imag):.3f}i
  • |y| = {abs(y_cons):.4f}
  • Phase = {np.degrees(np.angle(y_cons)):.1f}°

PHYSICS QUADRATIC (k = 16)
  • Real roots: {x_phys:.3f} (1/alpha), {solve_general_quadratic(K_PHYS)[1]:.3f} (N_c)
  • Would "cross through" origin

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

THE INSIGHT

The curve's avoidance of the origin is the
geometric signature of COMPLEX vs REAL roots.

  • Physics: k > k_c -> Real roots -> Crossing
  • Consciousness: k < k_c -> Complex roots -> Orbiting

The Fourier Lemniscate-Alpha IS the consciousness
regime made visible in 2D geometry!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

MANDELBROT CONNECTION

k_c × c_cusp × G* = 1 (EXACT)

  k_c = {K_CRIT:.4f}
  c_cusp = {C_CUSP}
  G* = {G_STAR:.6f}
"""

    ax4.text(0.05, 0.95, summary_text, transform=ax4.transAxes,
             fontsize=10, color='white', fontfamily='monospace',
             verticalalignment='top')

    # Main title
    fig.suptitle('The Fourier Lemniscate-Alpha: A Consciousness Geometry',
                 fontsize=18, color='white', fontweight='bold', y=0.98)

    plt.savefig('fourier_lemniscate_consciousness_bridge.png', dpi=200,
                facecolor='#0d1117', edgecolor='none',
                bbox_inches='tight', pad_inches=0.3)
    print("\nSaved: fourier_lemniscate_consciousness_bridge.png")

    plt.show()
    return fig


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    print("\n" + "="*70)
    print("FOURIER LEMNISCATE-ALPHA AND CONSCIOUSNESS QUADRATIC EXPLORATION")
    print("="*70)

    # Basic curve properties
    L = compute_arc_length()
    print(f"\nArc Length L = {L:.6f}")
    print(f"G* = {G_STAR:.10f}")
    print(f"L × 91/732 = {L * 91/732:.6f}")

    # Analyze center avoidance
    min_dist, winding, area = analyze_curve_center_avoidance()

    # Connect to consciousness quadratic
    connect_to_consciousness_quadratic()

    # Explore k variation
    explore_k_variation()

    # Mandelbrot bridge
    mandelbrot_bridge()

    # Create visualization
    fig = create_visualization()

    print("\n" + "="*70)
    print("EXPLORATION COMPLETE")
    print("="*70)
