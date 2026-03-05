#!/usr/bin/env python3
"""
Deep Exploration: Lemniscate-Alpha and Mandelbrot Set Overlays

This script explores multiple ways to visualize the connection between
the Fourier Lemniscate-Alpha and the Mandelbrot set:

1. Direct overlay (scaled)
2. Mirrored versions
3. The transformation c = 1/(k*G*) traced on both
4. Julia sets at key parameter values
5. Root trajectories mapped onto Mandelbrot
6. The "inverse" - what does the lemniscate look like in Mandelbrot coordinates?
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap
from matplotlib.gridspec import GridSpec
from math import gamma
from numba import jit
import warnings
warnings.filterwarnings('ignore')

# =============================================================================
# CONSTANTS
# =============================================================================

G_STAR = (np.sqrt(2) * gamma(0.25)**2) / (2 * np.pi)  # 2.9586751192
K_CRIT = 4 / G_STAR  # 1.3520
C_CUSP = 0.25
PHI = (1 + np.sqrt(5)) / 2

# Consciousness quadratic roots (correct formula)
Y_RE = G_STAR**2 / 4  # 2.1884
Y_IM = np.sqrt(G_STAR**3 * (2 - G_STAR/4)) / 2  # 2.8567

# Physics quadratic roots
X_PLUS = 137.036
X_MINUS = 3.024

# Curve parameters
FREQS = np.array([1, 2, 4, 8, 16])
X_AMPS = np.array([1.0, 0.5, 0.5, 2/5, 1/16])
Y_AMPS = np.array([1.0, -0.5, 0.5, -7/20, 1/16])

# =============================================================================
# CURVE FUNCTIONS
# =============================================================================

def fourier_lemniscate(t, scale=1.0, offset=(0, 0)):
    """Fourier Lemniscate-Alpha"""
    x = sum(X_AMPS[j] * np.cos(FREQS[j] * t) for j in range(5))
    y = sum(Y_AMPS[j] * np.sin(FREQS[j] * t) for j in range(5))
    return x * scale + offset[0], y * scale + offset[1]


def lemniscate_to_mandelbrot_c(t, k=0.5):
    """
    Map lemniscate parameter t to Mandelbrot c-plane
    using c = 1/(k * G*) as base, modulated by curve shape
    """
    x, y = fourier_lemniscate(t)
    # Normalize curve to unit scale
    r = np.sqrt(x**2 + y**2)
    theta = np.arctan2(y, x)

    # Map to c-plane: use curve shape to modulate around c = 1/(k*G*)
    c_base = 1 / (k * G_STAR)
    c_real = c_base * (1 + 0.3 * x / 2.5)  # modulate by x
    c_imag = 0.3 * y / 1.5  # y becomes imaginary part

    return c_real, c_imag


# =============================================================================
# MANDELBROT COMPUTATION
# =============================================================================

@jit(nopython=True)
def mandelbrot_escape(c_real, c_imag, max_iter=256):
    """Compute escape time for a point in the Mandelbrot set"""
    z_real, z_imag = 0.0, 0.0
    for i in range(max_iter):
        z_real_new = z_real*z_real - z_imag*z_imag + c_real
        z_imag = 2*z_real*z_imag + c_imag
        z_real = z_real_new
        if z_real*z_real + z_imag*z_imag > 4:
            return i
    return max_iter


@jit(nopython=True)
def compute_mandelbrot(x_min, x_max, y_min, y_max, width, height, max_iter=256):
    """Compute full Mandelbrot set"""
    result = np.zeros((height, width))
    for j in range(height):
        for i in range(width):
            c_real = x_min + (x_max - x_min) * i / width
            c_imag = y_min + (y_max - y_min) * j / height
            result[j, i] = mandelbrot_escape(c_real, c_imag, max_iter)
    return result


@jit(nopython=True)
def julia_escape(z_real, z_imag, c_real, c_imag, max_iter=256):
    """Compute escape time for Julia set"""
    for i in range(max_iter):
        z_real_new = z_real*z_real - z_imag*z_imag + c_real
        z_imag = 2*z_real*z_imag + c_imag
        z_real = z_real_new
        if z_real*z_real + z_imag*z_imag > 4:
            return i
    return max_iter


@jit(nopython=True)
def compute_julia(c_real, c_imag, x_min, x_max, y_min, y_max, width, height, max_iter=256):
    """Compute Julia set for given c"""
    result = np.zeros((height, width))
    for j in range(height):
        for i in range(width):
            z_real = x_min + (x_max - x_min) * i / width
            z_imag = y_min + (y_max - y_min) * j / height
            result[j, i] = julia_escape(z_real, z_imag, c_real, c_imag, max_iter)
    return result


# =============================================================================
# VISUALIZATION 1: Direct Overlay
# =============================================================================

def create_overlay_visualization():
    """Overlay lemniscate on Mandelbrot set with various transformations"""

    fig = plt.figure(figsize=(20, 16), facecolor='#0d1117')
    gs = GridSpec(2, 3, figure=fig, hspace=0.25, wspace=0.2)

    t = np.linspace(0, 2*np.pi, 2000)

    # Custom colormap for Mandelbrot
    colors = ['#0d1117', '#1a1a2e', '#16213e', '#1f4068', '#e94560', '#ffd460']
    cmap = LinearSegmentedColormap.from_list('custom', colors, N=256)

    # =========================================================================
    # Panel 1: Full Mandelbrot with key c-values marked
    # =========================================================================
    ax1 = fig.add_subplot(gs[0, 0])
    ax1.set_facecolor('#0d1117')

    # Compute Mandelbrot
    mandel = compute_mandelbrot(-2.5, 1.0, -1.5, 1.5, 800, 600, 256)
    ax1.imshow(mandel, extent=[-2.5, 1.0, -1.5, 1.5], cmap=cmap,
               origin='lower', aspect='equal')

    # Mark key c-values from TRD
    c_physics = 1 / (16 * G_STAR)  # 0.0211
    c_critical = C_CUSP  # 0.25
    c_consciousness = 1 / (0.5 * G_STAR)  # 0.676

    ax1.scatter([c_physics], [0], color='#22C55E', s=150, zorder=10,
                marker='o', edgecolor='white', linewidth=2, label=f'Physics c={c_physics:.3f}')
    ax1.scatter([c_critical], [0], color='#F97316', s=150, zorder=10,
                marker='D', edgecolor='white', linewidth=2, label=f'Critical c={c_critical:.3f}')
    ax1.scatter([c_consciousness], [0], color='#EC4899', s=150, zorder=10,
                marker='*', edgecolor='white', linewidth=2, label=f'Consciousness c={c_consciousness:.3f}')

    ax1.set_title('Mandelbrot Set with TRD c-values', color='white', fontsize=12, fontweight='bold')
    ax1.legend(loc='upper left', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=8)
    ax1.set_xlabel('Re(c)', color='white')
    ax1.set_ylabel('Im(c)', color='white')
    ax1.tick_params(colors='white')

    # =========================================================================
    # Panel 2: Lemniscate scaled and centered on Mandelbrot
    # =========================================================================
    ax2 = fig.add_subplot(gs[0, 1])
    ax2.set_facecolor('#0d1117')

    ax2.imshow(mandel, extent=[-2.5, 1.0, -1.5, 1.5], cmap=cmap,
               origin='lower', aspect='equal', alpha=0.7)

    # Scale lemniscate to fit over main cardioid
    # Main cardioid is roughly centered at (-0.25, 0) with radius ~0.5
    x_lem, y_lem = fourier_lemniscate(t, scale=0.25, offset=(-0.5, 0))
    ax2.plot(x_lem, y_lem, color='#22C55E', linewidth=2.5, alpha=0.9,
             label='Lemniscate-Alpha (scaled)')

    ax2.set_title('Lemniscate Overlaid on Mandelbrot\n(scaled to cardioid)',
                  color='white', fontsize=12, fontweight='bold')
    ax2.legend(loc='upper left', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=9)
    ax2.set_xlabel('Re(c)', color='white')
    ax2.set_ylabel('Im(c)', color='white')
    ax2.tick_params(colors='white')

    # =========================================================================
    # Panel 3: Lemniscate MIRRORED overlaid
    # =========================================================================
    ax3 = fig.add_subplot(gs[0, 2])
    ax3.set_facecolor('#0d1117')

    ax3.imshow(mandel, extent=[-2.5, 1.0, -1.5, 1.5], cmap=cmap,
               origin='lower', aspect='equal', alpha=0.7)

    # Mirror the lemniscate (flip x-axis)
    x_lem_mirror = -x_lem - 1.0  # Flip and shift to align with bulb
    ax3.plot(x_lem_mirror, y_lem, color='#EC4899', linewidth=2.5, alpha=0.9,
             label='Lemniscate MIRRORED')

    # Also plot original for comparison
    ax3.plot(x_lem, y_lem, color='#22C55E', linewidth=1.5, alpha=0.5,
             label='Original')

    ax3.set_title('Lemniscate Mirrored\n(flipped horizontally)',
                  color='white', fontsize=12, fontweight='bold')
    ax3.legend(loc='upper left', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=9)
    ax3.set_xlabel('Re(c)', color='white')
    ax3.set_ylabel('Im(c)', color='white')
    ax3.tick_params(colors='white')

    # =========================================================================
    # Panel 4: Zoomed cardioid with lemniscate
    # =========================================================================
    ax4 = fig.add_subplot(gs[1, 0])
    ax4.set_facecolor('#0d1117')

    # Compute zoomed Mandelbrot (cardioid region)
    mandel_zoom = compute_mandelbrot(-0.8, 0.4, -0.6, 0.6, 600, 600, 512)
    ax4.imshow(mandel_zoom, extent=[-0.8, 0.4, -0.6, 0.6], cmap=cmap,
               origin='lower', aspect='equal', alpha=0.8)

    # Cardioid boundary parametric equation: c = (e^(it) - e^(2it)/2)/2
    t_card = np.linspace(0, 2*np.pi, 1000)
    cardioid_x = 0.5 * np.cos(t_card) - 0.25 * np.cos(2*t_card)
    cardioid_y = 0.5 * np.sin(t_card) - 0.25 * np.sin(2*t_card)
    cardioid_x = cardioid_x * 0.5 - 0.25  # Scale and shift
    cardioid_y = cardioid_y * 0.5

    ax4.plot(cardioid_x, cardioid_y, '--', color='white', linewidth=1, alpha=0.5,
             label='Cardioid boundary')

    # Lemniscate scaled to match
    x_lem2, y_lem2 = fourier_lemniscate(t, scale=0.15, offset=(-0.2, 0))
    ax4.plot(x_lem2, y_lem2, color='#22C55E', linewidth=2, alpha=0.9,
             label='Lemniscate-Alpha')

    ax4.set_title('Zoomed: Cardioid + Lemniscate',
                  color='white', fontsize=12, fontweight='bold')
    ax4.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=9)
    ax4.set_xlabel('Re(c)', color='white')
    ax4.set_ylabel('Im(c)', color='white')
    ax4.tick_params(colors='white')

    # =========================================================================
    # Panel 5: Both curves mirrored and overlaid
    # =========================================================================
    ax5 = fig.add_subplot(gs[1, 1])
    ax5.set_facecolor('#0d1117')

    # Flip Mandelbrot horizontally
    ax5.imshow(np.fliplr(mandel), extent=[-1.0, 2.5, -1.5, 1.5], cmap=cmap,
               origin='lower', aspect='equal', alpha=0.7)

    # Original lemniscate
    x_lem3, y_lem3 = fourier_lemniscate(t, scale=0.4, offset=(0.5, 0))
    ax5.plot(x_lem3, y_lem3, color='#22C55E', linewidth=2.5, alpha=0.9,
             label='Lemniscate')

    ax5.set_title('BOTH Mirrored\n(Mandelbrot flipped)',
                  color='white', fontsize=12, fontweight='bold')
    ax5.legend(loc='upper left', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=9)
    ax5.set_xlabel('Re(c)', color='white')
    ax5.set_ylabel('Im(c)', color='white')
    ax5.tick_params(colors='white')

    # =========================================================================
    # Panel 6: The transformation path c = 1/(k*G*)
    # =========================================================================
    ax6 = fig.add_subplot(gs[1, 2])
    ax6.set_facecolor('#0d1117')

    ax6.imshow(mandel, extent=[-2.5, 1.0, -1.5, 1.5], cmap=cmap,
               origin='lower', aspect='equal', alpha=0.7)

    # Trace the path c = 1/(k*G*) for k from 0.3 to 20
    k_range = np.linspace(0.3, 20, 200)
    c_path = 1 / (k_range * G_STAR)

    # Color by k value
    ax6.scatter(c_path, np.zeros_like(c_path), c=k_range, cmap='plasma',
                s=20, alpha=0.8, zorder=5)

    # Mark special points
    ax6.scatter([c_physics], [0], color='#22C55E', s=200, zorder=10,
                marker='o', edgecolor='white', linewidth=2)
    ax6.scatter([c_critical], [0], color='#F97316', s=200, zorder=10,
                marker='D', edgecolor='white', linewidth=2)
    ax6.scatter([c_consciousness], [0], color='#EC4899', s=200, zorder=10,
                marker='*', edgecolor='white', linewidth=2)

    ax6.annotate('k=16\n(Physics)', (c_physics, 0), (c_physics-0.3, 0.5),
                 color='#22C55E', fontsize=9,
                 arrowprops=dict(arrowstyle='->', color='#22C55E'))
    ax6.annotate('k=k_c\n(Critical)', (c_critical, 0), (c_critical+0.2, 0.5),
                 color='#F97316', fontsize=9,
                 arrowprops=dict(arrowstyle='->', color='#F97316'))
    ax6.annotate('k=0.5\n(Consciousness)', (c_consciousness, 0), (c_consciousness+0.1, -0.5),
                 color='#EC4899', fontsize=9,
                 arrowprops=dict(arrowstyle='->', color='#EC4899'))

    ax6.set_title('Path c = 1/(k*G*) on Mandelbrot\n(colored by k)',
                  color='white', fontsize=12, fontweight='bold')
    ax6.set_xlabel('Re(c)', color='white')
    ax6.set_ylabel('Im(c)', color='white')
    ax6.tick_params(colors='white')

    # Main title
    fig.suptitle('Lemniscate-Alpha and Mandelbrot: Geometric Duality',
                 fontsize=18, color='white', fontweight='bold', y=0.98)

    plt.savefig('lemniscate_mandelbrot_overlay.png', dpi=150,
                facecolor='#0d1117', edgecolor='none',
                bbox_inches='tight', pad_inches=0.3)
    print("Saved: lemniscate_mandelbrot_overlay.png")

    return fig


# =============================================================================
# VISUALIZATION 2: Julia Sets Comparison
# =============================================================================

def create_julia_comparison():
    """Compare Julia sets at physics, critical, and consciousness c-values"""

    fig = plt.figure(figsize=(18, 12), facecolor='#0d1117')
    gs = GridSpec(2, 3, figure=fig, hspace=0.25, wspace=0.15)

    # Custom colormap
    colors = ['#0d1117', '#1a1a2e', '#16213e', '#1f4068', '#e94560', '#ffd460', '#ffffff']
    cmap = LinearSegmentedColormap.from_list('custom', colors, N=256)

    t = np.linspace(0, 2*np.pi, 1000)

    # c-values to compare
    c_values = [
        (1/(16*G_STAR), 0, 'Physics (k=16)', '#22C55E'),
        (C_CUSP, 0, 'Critical (k=k_c)', '#F97316'),
        (1/(0.5*G_STAR), 0, 'Consciousness (k=0.5)', '#EC4899'),
        (-0.75, 0, 'Basilica (c=-0.75)', '#8B5CF6'),
        (-0.123, 0.745, 'Douady Rabbit', '#EAB308'),
        (0.285, 0.01, 'Near cusp', '#06B6D4'),
    ]

    for idx, (c_re, c_im, title, color) in enumerate(c_values):
        ax = fig.add_subplot(gs[idx // 3, idx % 3])
        ax.set_facecolor('#0d1117')

        # Compute Julia set
        julia = compute_julia(c_re, c_im, -1.8, 1.8, -1.5, 1.5, 600, 500, 256)
        ax.imshow(julia, extent=[-1.8, 1.8, -1.5, 1.5], cmap=cmap,
                  origin='lower', aspect='equal')

        # Overlay scaled lemniscate
        x_lem, y_lem = fourier_lemniscate(t, scale=0.5, offset=(0, 0))
        ax.plot(x_lem, y_lem, color=color, linewidth=1.5, alpha=0.8)

        ax.set_title(f'{title}\nc = {c_re:.4f} + {c_im:.4f}i',
                     color='white', fontsize=11, fontweight='bold')
        ax.tick_params(colors='white')

        # Show if connected or Cantor dust
        # (Inside M -> connected, Outside M -> Cantor dust)
        is_inside = mandelbrot_escape(c_re, c_im, 1000) == 1000
        status = "Connected" if is_inside else "Cantor dust"
        ax.text(0.02, 0.98, status, transform=ax.transAxes, color=color,
                fontsize=10, fontweight='bold', verticalalignment='top',
                bbox=dict(boxstyle='round', facecolor='#161b22', alpha=0.8))

    fig.suptitle('Julia Sets at Key c-Values (with Lemniscate Overlay)',
                 fontsize=16, color='white', fontweight='bold', y=0.98)

    plt.savefig('julia_sets_comparison.png', dpi=150,
                facecolor='#0d1117', edgecolor='none',
                bbox_inches='tight', pad_inches=0.3)
    print("Saved: julia_sets_comparison.png")

    return fig


# =============================================================================
# VISUALIZATION 3: Lemniscate in "Mandelbrot Coordinates"
# =============================================================================

def create_lemniscate_transformed():
    """Transform lemniscate points through various mappings"""

    fig = plt.figure(figsize=(18, 14), facecolor='#0d1117')
    gs = GridSpec(2, 2, figure=fig, hspace=0.25, wspace=0.2)

    t = np.linspace(0, 2*np.pi, 2000)
    x_lem, y_lem = fourier_lemniscate(t)

    # =========================================================================
    # Panel 1: Original lemniscate
    # =========================================================================
    ax1 = fig.add_subplot(gs[0, 0])
    ax1.set_facecolor('#0d1117')
    ax1.plot(x_lem, y_lem, color='#22C55E', linewidth=2)
    ax1.scatter([0], [0], color='#fbbf24', s=100, zorder=10)
    ax1.set_title('Original Lemniscate-Alpha', color='white', fontsize=12, fontweight='bold')
    ax1.set_aspect('equal')
    ax1.tick_params(colors='white')
    for spine in ax1.spines.values():
        spine.set_color('#30363d')

    # =========================================================================
    # Panel 2: Inversion z -> 1/z
    # =========================================================================
    ax2 = fig.add_subplot(gs[0, 1])
    ax2.set_facecolor('#0d1117')

    # Complex inversion
    z = x_lem + 1j * y_lem
    # Avoid division by zero near origin
    z_safe = np.where(np.abs(z) < 0.1, 0.1 * np.exp(1j * np.angle(z)), z)
    z_inv = 1 / z_safe

    ax2.plot(z_inv.real, z_inv.imag, color='#EC4899', linewidth=2)
    ax2.scatter([0], [0], color='#fbbf24', s=100, zorder=10)
    ax2.set_title('Inverted: z -> 1/z', color='white', fontsize=12, fontweight='bold')
    ax2.set_aspect('equal')
    ax2.tick_params(colors='white')
    for spine in ax2.spines.values():
        spine.set_color('#30363d')
    ax2.set_xlim(-5, 5)
    ax2.set_ylim(-5, 5)

    # =========================================================================
    # Panel 3: Squared z -> z^2
    # =========================================================================
    ax3 = fig.add_subplot(gs[1, 0])
    ax3.set_facecolor('#0d1117')

    z_sq = z ** 2
    ax3.plot(z_sq.real, z_sq.imag, color='#8B5CF6', linewidth=2)
    ax3.scatter([0], [0], color='#fbbf24', s=100, zorder=10)
    ax3.set_title('Squared: z -> z^2', color='white', fontsize=12, fontweight='bold')
    ax3.set_aspect('equal')
    ax3.tick_params(colors='white')
    for spine in ax3.spines.values():
        spine.set_color('#30363d')

    # =========================================================================
    # Panel 4: Mandelbrot iteration: z -> z^2 + c (one step)
    # =========================================================================
    ax4 = fig.add_subplot(gs[1, 1])
    ax4.set_facecolor('#0d1117')

    # Use consciousness c-value
    c_cons = 1 / (0.5 * G_STAR)
    z_mandel = z ** 2 + c_cons

    ax4.plot(z_mandel.real, z_mandel.imag, color='#F97316', linewidth=2,
             label='z^2 + c_consciousness')

    # Also show with physics c
    c_phys = 1 / (16 * G_STAR)
    z_mandel_phys = z ** 2 + c_phys
    ax4.plot(z_mandel_phys.real, z_mandel_phys.imag, color='#22C55E', linewidth=1.5,
             alpha=0.7, label='z^2 + c_physics')

    ax4.scatter([0], [0], color='#fbbf24', s=100, zorder=10)
    ax4.set_title('Mandelbrot Step: z -> z^2 + c', color='white', fontsize=12, fontweight='bold')
    ax4.set_aspect('equal')
    ax4.tick_params(colors='white')
    ax4.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d',
               labelcolor='white', fontsize=9)
    for spine in ax4.spines.values():
        spine.set_color('#30363d')

    fig.suptitle('Lemniscate Under Complex Transformations',
                 fontsize=16, color='white', fontweight='bold', y=0.98)

    plt.savefig('lemniscate_transformations.png', dpi=150,
                facecolor='#0d1117', edgecolor='none',
                bbox_inches='tight', pad_inches=0.3)
    print("Saved: lemniscate_transformations.png")

    return fig


# =============================================================================
# VISUALIZATION 4: The "Dual" View
# =============================================================================

def create_dual_view():
    """Side-by-side: Lemniscate and Mandelbrot with matching features highlighted"""

    fig = plt.figure(figsize=(20, 10), facecolor='#0d1117')
    gs = GridSpec(1, 2, figure=fig, wspace=0.15)

    t = np.linspace(0, 2*np.pi, 2000)

    # Custom colormap
    colors = ['#0d1117', '#1a1a2e', '#16213e', '#1f4068', '#e94560', '#ffd460']
    cmap = LinearSegmentedColormap.from_list('custom', colors, N=256)

    # =========================================================================
    # Left: Lemniscate with annotations
    # =========================================================================
    ax1 = fig.add_subplot(gs[0, 0])
    ax1.set_facecolor('#0d1117')

    x_lem, y_lem = fourier_lemniscate(t)
    ax1.plot(x_lem, y_lem, color='#22C55E', linewidth=3)

    # Find and mark key points
    distances = np.sqrt(x_lem**2 + y_lem**2)
    min_idx = np.argmin(distances)
    max_idx = np.argmax(distances)

    # Minimum (closest to origin)
    ax1.scatter([x_lem[min_idx]], [y_lem[min_idx]], color='#EC4899', s=150, zorder=10,
                marker='*', edgecolor='white', linewidth=2)
    ax1.annotate('Closest\n(consciousness)', (x_lem[min_idx], y_lem[min_idx]),
                 (x_lem[min_idx]+0.5, y_lem[min_idx]+0.5), color='#EC4899',
                 fontsize=10, arrowprops=dict(arrowstyle='->', color='#EC4899'))

    # Maximum (farthest from origin)
    ax1.scatter([x_lem[max_idx]], [y_lem[max_idx]], color='#22C55E', s=150, zorder=10,
                marker='s', edgecolor='white', linewidth=2)
    ax1.annotate('Farthest\n(physics)', (x_lem[max_idx], y_lem[max_idx]),
                 (x_lem[max_idx]-0.8, y_lem[max_idx]+0.3), color='#22C55E',
                 fontsize=10, arrowprops=dict(arrowstyle='->', color='#22C55E'))

    # Origin
    ax1.scatter([0], [0], color='#fbbf24', s=200, zorder=10, marker='o',
                edgecolor='white', linewidth=2)
    ax1.annotate('Origin\n(void)', (0, 0), (-0.8, -0.8), color='#fbbf24',
                 fontsize=10, arrowprops=dict(arrowstyle='->', color='#fbbf24'))

    ax1.set_xlim(-3, 3)
    ax1.set_ylim(-2, 2)
    ax1.set_aspect('equal')
    ax1.set_title('Fourier Lemniscate-Alpha\n(Configuration Space)',
                  color='white', fontsize=14, fontweight='bold')
    ax1.tick_params(colors='white')
    for spine in ax1.spines.values():
        spine.set_color('#30363d')

    # =========================================================================
    # Right: Mandelbrot with annotations
    # =========================================================================
    ax2 = fig.add_subplot(gs[0, 1])
    ax2.set_facecolor('#0d1117')

    mandel = compute_mandelbrot(-2.5, 1.0, -1.5, 1.5, 800, 600, 256)
    ax2.imshow(mandel, extent=[-2.5, 1.0, -1.5, 1.5], cmap=cmap,
               origin='lower', aspect='equal')

    # Mark corresponding points
    c_phys = 1 / (16 * G_STAR)
    c_crit = C_CUSP
    c_cons = 1 / (0.5 * G_STAR)

    ax2.scatter([c_phys], [0], color='#22C55E', s=150, zorder=10,
                marker='s', edgecolor='white', linewidth=2)
    ax2.annotate('Physics\n(stable)', (c_phys, 0), (c_phys-0.5, 0.6), color='#22C55E',
                 fontsize=10, arrowprops=dict(arrowstyle='->', color='#22C55E'))

    ax2.scatter([c_crit], [0], color='#F97316', s=150, zorder=10,
                marker='D', edgecolor='white', linewidth=2)
    ax2.annotate('Critical\n(cusp)', (c_crit, 0), (c_crit+0.3, 0.6), color='#F97316',
                 fontsize=10, arrowprops=dict(arrowstyle='->', color='#F97316'))

    ax2.scatter([c_cons], [0], color='#EC4899', s=150, zorder=10,
                marker='*', edgecolor='white', linewidth=2)
    ax2.annotate('Consciousness\n(escaping)', (c_cons, 0), (c_cons-0.3, -0.6), color='#EC4899',
                 fontsize=10, arrowprops=dict(arrowstyle='->', color='#EC4899'))

    ax2.scatter([0], [0], color='#fbbf24', s=200, zorder=10, marker='o',
                edgecolor='white', linewidth=2)
    ax2.annotate('Origin', (0, 0), (-0.5, -0.8), color='#fbbf24',
                 fontsize=10, arrowprops=dict(arrowstyle='->', color='#fbbf24'))

    ax2.set_title('Mandelbrot Set\n(Parameter Space)',
                  color='white', fontsize=14, fontweight='bold')
    ax2.tick_params(colors='white')

    # Add bridge equation
    fig.text(0.5, 0.02, 'BRIDGE: k_c * c_cusp * G* = 1  |  Transformation: c = 1/(k * G*)',
             ha='center', color='white', fontsize=12, fontfamily='monospace',
             bbox=dict(boxstyle='round', facecolor='#161b22', edgecolor='#30363d'))

    fig.suptitle('The Duality: Lemniscate <-> Mandelbrot',
                 fontsize=18, color='white', fontweight='bold', y=0.98)

    plt.savefig('lemniscate_mandelbrot_dual.png', dpi=150,
                facecolor='#0d1117', edgecolor='none',
                bbox_inches='tight', pad_inches=0.3)
    print("Saved: lemniscate_mandelbrot_dual.png")

    return fig


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    print("="*70)
    print("LEMNISCATE-MANDELBROT EXPLORATION")
    print("="*70)

    print(f"\nKey constants:")
    print(f"  G* = {G_STAR:.6f}")
    print(f"  k_c = 4/G* = {K_CRIT:.6f}")
    print(f"  c_cusp = {C_CUSP}")
    print(f"  Bridge: k_c * c_cusp * G* = {K_CRIT * C_CUSP * G_STAR:.6f}")

    print(f"\nTRD c-values:")
    print(f"  Physics (k=16): c = {1/(16*G_STAR):.6f}")
    print(f"  Critical (k=k_c): c = {C_CUSP:.6f}")
    print(f"  Consciousness (k=0.5): c = {1/(0.5*G_STAR):.6f}")

    print("\nGenerating visualizations...")

    # Create all visualizations
    fig1 = create_overlay_visualization()
    fig2 = create_julia_comparison()
    fig3 = create_lemniscate_transformed()
    fig4 = create_dual_view()

    print("\nAll visualizations complete!")
    print("="*70)

    plt.show()
