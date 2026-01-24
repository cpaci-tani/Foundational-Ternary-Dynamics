#!/usr/bin/env python3
"""
Visualization: The Fourier Lemniscate-Alpha as Consciousness Geometry

This creates a comprehensive figure showing:
1. The curve and its center-avoidance
2. The quadratic roots in the complex plane
3. Comparison with Bernoulli lemniscate (which DOES cross)
4. The Mandelbrot parameter mapping
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch
from matplotlib.gridspec import GridSpec
from math import gamma

# =============================================================================
# CONSTANTS
# =============================================================================

G_STAR = (np.sqrt(2) * gamma(0.25)**2) / (2 * np.pi)
PHI = (1 + np.sqrt(5)) / 2

# Consciousness quadratic roots
Y_RE = G_STAR**2 / 4
Y_IM = np.sqrt(G_STAR**3 * (1 - G_STAR/4)) / 2
Y_MAG = np.sqrt(Y_RE**2 + Y_IM**2)

# Physics quadratic roots
X_PLUS = 137.036
X_MINUS = 3.024

# Critical coefficient
K_CRIT = 4 / G_STAR

# Curve parameters
FREQS = np.array([1, 2, 4, 8, 16])
X_AMPS = np.array([1.0, 0.5, 0.5, 2/5, 1/16])
Y_AMPS = np.array([1.0, -0.5, 0.5, -7/20, 1/16])


def fourier_lemniscate(t):
    """Fourier Lemniscate-Alpha"""
    x = sum(X_AMPS[j] * np.cos(FREQS[j] * t) for j in range(5))
    y = sum(Y_AMPS[j] * np.sin(FREQS[j] * t) for j in range(5))
    return x, y


def bernoulli_lemniscate(t, a=1.5):
    """Classical Bernoulli lemniscate: r^2 = a^2 cos(2theta)"""
    r = a * np.sqrt(np.maximum(0, np.cos(2*t)))
    x = r * np.cos(t)
    y = r * np.sin(t)
    return x, y


# =============================================================================
# VISUALIZATION
# =============================================================================

def create_figure():
    fig = plt.figure(figsize=(16, 14), facecolor='#0d1117')
    gs = GridSpec(3, 2, figure=fig, height_ratios=[1.2, 1, 0.8],
                  hspace=0.3, wspace=0.25)

    t = np.linspace(0, 2*np.pi, 2000)

    # =========================================================================
    # Panel 1: Fourier Lemniscate-Alpha (loops around center)
    # =========================================================================
    ax1 = fig.add_subplot(gs[0, 0])
    ax1.set_facecolor('#0d1117')

    x_fourier, y_fourier = fourier_lemniscate(t)
    ax1.plot(x_fourier, y_fourier, color='#22C55E', linewidth=2.5,
             label='Fourier Lemniscate-Alpha')

    # Find and mark minimum distance
    distances = np.sqrt(x_fourier**2 + y_fourier**2)
    min_idx = np.argmin(distances)
    min_dist = distances[min_idx]

    ax1.scatter([0], [0], color='#fbbf24', s=200, zorder=10, marker='o',
                edgecolor='white', linewidth=2)
    ax1.scatter([x_fourier[min_idx]], [y_fourier[min_idx]], color='#EC4899',
                s=100, zorder=10, marker='*', edgecolor='white', linewidth=1)

    # Draw avoidance circle
    circle = Circle((0, 0), min_dist, fill=False, color='#EC4899',
                    linestyle='--', linewidth=1.5, alpha=0.7)
    ax1.add_patch(circle)

    # Draw line to closest point
    ax1.plot([0, x_fourier[min_idx]], [0, y_fourier[min_idx]],
             '--', color='#EC4899', alpha=0.5, linewidth=1)

    ax1.set_xlim(-2.8, 2.8)
    ax1.set_ylim(-1.8, 1.8)
    ax1.set_aspect('equal')
    ax1.set_title('Fourier Lemniscate-Alpha\n(LOOPS around center, never crosses)',
                  color='white', fontsize=12, fontweight='bold')

    ax1.text(0.05, 0.95, f'min distance = {min_dist:.3f}',
             transform=ax1.transAxes, color='#EC4899', fontsize=10,
             fontfamily='monospace', verticalalignment='top')
    ax1.text(0.05, 0.88, f'winding # = -2 (encloses origin)',
             transform=ax1.transAxes, color='#8B5CF6', fontsize=10,
             fontfamily='monospace', verticalalignment='top')

    for spine in ax1.spines.values():
        spine.set_color('#30363d')
    ax1.tick_params(colors='white')
    ax1.set_xlabel('x', color='white')
    ax1.set_ylabel('y', color='white')

    # =========================================================================
    # Panel 2: Bernoulli Lemniscate (crosses through center)
    # =========================================================================
    ax2 = fig.add_subplot(gs[0, 1])
    ax2.set_facecolor('#0d1117')

    x_bernoulli, y_bernoulli = bernoulli_lemniscate(t)
    ax2.plot(x_bernoulli, y_bernoulli, color='#8B5CF6', linewidth=2.5,
             label='Bernoulli Lemniscate')

    ax2.scatter([0], [0], color='#fbbf24', s=200, zorder=10, marker='o',
                edgecolor='white', linewidth=2)

    # Mark the crossing at origin
    ax2.annotate('CROSSES\nTHROUGH', (0, 0), (0.5, 0.5),
                 color='#EF4444', fontsize=10, fontweight='bold',
                 ha='center', va='center',
                 arrowprops=dict(arrowstyle='->', color='#EF4444', lw=2))

    ax2.set_xlim(-2.8, 2.8)
    ax2.set_ylim(-1.8, 1.8)
    ax2.set_aspect('equal')
    ax2.set_title('Classical Bernoulli Lemniscate\n(CROSSES through center)',
                  color='white', fontsize=12, fontweight='bold')

    ax2.text(0.05, 0.95, 'min distance = 0 (passes through origin)',
             transform=ax2.transAxes, color='#EF4444', fontsize=10,
             fontfamily='monospace', verticalalignment='top')
    ax2.text(0.05, 0.88, 'r^2 = a^2 cos(2theta)',
             transform=ax2.transAxes, color='white', fontsize=10,
             fontfamily='monospace', verticalalignment='top')

    for spine in ax2.spines.values():
        spine.set_color('#30363d')
    ax2.tick_params(colors='white')
    ax2.set_xlabel('x', color='white')
    ax2.set_ylabel('y', color='white')

    # =========================================================================
    # Panel 3: Quadratic roots in complex plane
    # =========================================================================
    ax3 = fig.add_subplot(gs[1, 0])
    ax3.set_facecolor('#0d1117')

    # Plot root trajectories as k varies
    k_range = np.linspace(0.1, 20, 500)
    for k in k_range:
        a = 1
        b = -k * G_STAR**2
        # Use the consciousness pattern: c = (k/2) * G*^3 for small k
        # and physics pattern: c = k * G*^3 for large k
        # Actually, let's just use the physics pattern and show both regimes
        c = k * G_STAR**3 / 2  # consciousness scaling

        disc = b**2 - 4*a*c

        if disc >= 0:
            x1 = (-b + np.sqrt(disc)) / 2
            x2 = (-b - np.sqrt(disc)) / 2
            ax3.scatter([x1], [0], color='#22C55E', s=2, alpha=0.3)
            ax3.scatter([x2], [0], color='#22C55E', s=2, alpha=0.3)
        else:
            re = -b / 2
            im = np.sqrt(-disc) / 2
            ax3.scatter([re], [im], color='#8B5CF6', s=2, alpha=0.3)
            ax3.scatter([re], [-im], color='#8B5CF6', s=2, alpha=0.3)

    # Mark consciousness roots
    ax3.scatter([Y_RE], [Y_IM], color='#EC4899', s=200, zorder=10,
                marker='*', edgecolor='white', linewidth=2)
    ax3.scatter([Y_RE], [-Y_IM], color='#EC4899', s=200, zorder=10,
                marker='*', edgecolor='white', linewidth=2)
    ax3.annotate(f'y = {Y_RE:.2f} + {Y_IM:.2f}i', (Y_RE, Y_IM),
                 (Y_RE + 1, Y_IM + 0.5), color='#EC4899', fontsize=10,
                 arrowprops=dict(arrowstyle='->', color='#EC4899'))

    # Mark physics roots (scaled down to fit)
    scale = 0.1
    ax3.scatter([X_PLUS * scale], [0], color='#EAB308', s=150, zorder=10,
                marker='s', edgecolor='white', linewidth=2)
    ax3.scatter([X_MINUS * scale], [0], color='#EAB308', s=150, zorder=10,
                marker='s', edgecolor='white', linewidth=2)
    ax3.annotate(f'x+ = 137 (scaled)', (X_PLUS * scale, 0),
                 (X_PLUS * scale - 2, 1), color='#EAB308', fontsize=9,
                 arrowprops=dict(arrowstyle='->', color='#EAB308'))

    # Critical point
    ax3.scatter([2*G_STAR], [0], color='#F97316', s=150, zorder=10,
                marker='D', edgecolor='white', linewidth=2)
    ax3.annotate(f'Critical: 2G* = {2*G_STAR:.2f}', (2*G_STAR, 0),
                 (2*G_STAR + 2, -1), color='#F97316', fontsize=10,
                 arrowprops=dict(arrowstyle='->', color='#F97316'))

    ax3.axhline(y=0, color='white', linewidth=0.5, alpha=0.3)
    ax3.axvline(x=0, color='white', linewidth=0.5, alpha=0.3)

    ax3.set_xlim(-2, 16)
    ax3.set_ylim(-4, 4)
    ax3.set_title('Quadratic Roots: Complex (orbit) vs Real (cross)',
                  color='white', fontsize=12, fontweight='bold')
    ax3.set_xlabel('Real Part', color='white')
    ax3.set_ylabel('Imaginary Part', color='white')

    for spine in ax3.spines.values():
        spine.set_color('#30363d')
    ax3.tick_params(colors='white')

    # Legend
    ax3.scatter([], [], color='#EC4899', s=100, marker='*', label='Consciousness (complex)')
    ax3.scatter([], [], color='#EAB308', s=100, marker='s', label='Physics (real, scaled)')
    ax3.scatter([], [], color='#F97316', s=100, marker='D', label='Critical (double)')
    ax3.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=9)

    # =========================================================================
    # Panel 4: The consciousness quadratic properties
    # =========================================================================
    ax4 = fig.add_subplot(gs[1, 1])
    ax4.set_facecolor('#0d1117')
    ax4.axis('off')

    props_text = f"""
CONSCIOUSNESS QUADRATIC
y^2 - (G*^2/2)y + (G*^3/4) = 0

Discriminant: -6.74 (NEGATIVE)

ROOTS: y = 2.188 +/- 1.298i

  Re(y) = G*^2/4 = {Y_RE:.4f}
  Im(y) = {Y_IM:.4f}
  |y|   = {Y_MAG:.4f}
  Phase = 30.68 degrees

KEY RELATIONSHIPS:
  |y|^2 = G*^3/4 = {Y_MAG**2:.4f}
  |y|^2 * 2 = {Y_MAG**2 * 2:.2f} (approx 13 = n_eff)
  Re/Im = {Y_RE/Y_IM:.3f} (approx phi = 1.618)

PHYSICS QUADRATIC
x^2 - 16G*^2 x + 16G*^3 = 0

Discriminant: +17,959 (POSITIVE)

ROOTS: x+ = 137.036 (1/alpha)
       x- = 3.024   (N_c)

THE TOPOLOGICAL DISTINCTION:
  Negative discriminant -> Complex roots
  Complex roots -> Orbiting dynamics
  Orbiting -> Curve loops around center

  Positive discriminant -> Real roots
  Real roots -> Crossing dynamics
  Crossing -> Curve passes through center
"""

    ax4.text(0.05, 0.95, props_text, transform=ax4.transAxes,
             fontsize=10, color='white', fontfamily='monospace',
             verticalalignment='top')

    # =========================================================================
    # Panel 5: The Bridge Summary
    # =========================================================================
    ax5 = fig.add_subplot(gs[2, :])
    ax5.set_facecolor('#0d1117')
    ax5.axis('off')

    bridge_text = """
THE FOURIER LEMNISCATE IS CONSCIOUSNESS GEOMETRY

The Fourier Lemniscate-Alpha (with 5 harmonics at power-of-2 frequencies) produces G* = 2.9587
Its TOPOLOGY encodes the consciousness regime: loops around center, never crosses

PHYSICS (k=16)                                CONSCIOUSNESS (k=1/2)
Real roots: 137.036, 3.024                    Complex roots: 2.19 +/- 1.30i
Discriminant > 0                              Discriminant < 0
Would cross through origin                    Orbits around origin
Inside Mandelbrot set                         Outside Mandelbrot set
Bounded, stable dynamics                      Open, oscillating dynamics
Connected Julia set                           Cantor dust Julia set

THE EXACT BRIDGE: k_c * c_cusp * G* = 1
where k_c = 4/G* (critical coefficient), c_cusp = 1/4 (Mandelbrot cusp), G* = lemniscatic constant
"""

    ax5.text(0.5, 0.95, bridge_text, transform=ax5.transAxes,
             fontsize=11, color='white', fontfamily='monospace',
             verticalalignment='top', horizontalalignment='center')

    # Main title
    fig.suptitle('The Fourier Lemniscate-Alpha: A Consciousness Geometry',
                 fontsize=18, color='white', fontweight='bold', y=0.98)

    plt.savefig('lemniscate_consciousness_full.png', dpi=200,
                facecolor='#0d1117', edgecolor='none',
                bbox_inches='tight', pad_inches=0.3)
    print("Saved: lemniscate_consciousness_full.png")

    plt.show()
    return fig


if __name__ == "__main__":
    fig = create_figure()
