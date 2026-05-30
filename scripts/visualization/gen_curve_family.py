"""
FTD Curve Family Figures — Publication-Quality Visualizations
=============================================================
Generates 5 figures from the Curve Family Mathematical Analysis:

  1) fig_nlobe_hierarchy.png      — Full 8-level N-lobe hierarchy (circle to 137-lobe)
  2) fig_fourier_buildup.png      — Progressive harmonic buildup of Lemniscate-Alpha
  3) fig_quadratic_phase.png      — Master quadratic domain partition (real vs complex roots)
  4) fig_origin_avoidance.png     — Lemniscate-Alpha with min-distance annotation
  5) fig_feigenbaum_bridge.png    — Bifurcation diagram with FTD integers marked

Reference: docs/theory/EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md
"""

from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, Arc
from matplotlib.lines import Line2D
from scipy.special import gamma

# Output directory
_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'
_FIGDIR.mkdir(parents=True, exist_ok=True)

# =============================================================================
# Shared constants
# =============================================================================
G_STAR = np.sqrt(2) * (gamma(0.25)**2) / (2 * np.pi)
ALPHA = 1.0 / 137.035999177
DELTA_F = 4.669201609102990  # Feigenbaum constant
VARPI = 2.622057554292119810   # Lemniscate constant

# FTD integers
N_C = 3
N_BASE = 4
B_3 = 7
N_EFF = 13

# Publication style
STYLE = {
    'font.family': 'serif',
    'mathtext.fontset': 'cm',
    'font.size': 10,
    'axes.titlesize': 12,
    'axes.labelsize': 11,
    'legend.fontsize': 9,
    'axes.linewidth': 0.8,
    'xtick.direction': 'in',
    'ytick.direction': 'in',
    'xtick.major.size': 4,
    'ytick.major.size': 4,
}

# Color palette (consistent with gen_master_figures.py)
NAVY = '#003366'
RED = '#CC0000'
CYAN = '#0099CC'
TEAL = '#006699'
GOLD = '#CC9900'
PURPLE = '#7B2D8E'
GREEN = '#228B22'

# FTD integer colors (from ftd_colors.py)
INT_COLORS = {3: '#E74C3C', 4: '#F39C12', 7: '#9B59B6', 13: '#3498DB'}

# Lemniscate mode colors
MODE_COLORS = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#96CEB4', '#FFEAA7']


# =============================================================================
# Lemniscate-Alpha parametric curve
# =============================================================================
def lemniscate_alpha(t):
    """Return (x, y) for the Lemniscate-Alpha curve."""
    x = (np.cos(t) + 0.5 * np.cos(2*t) + 0.5 * np.cos(4*t)
         + 0.4 * np.cos(8*t) + 0.0625 * np.cos(16*t))
    y = (np.sin(t) - 0.5 * np.sin(2*t) + 0.5 * np.sin(4*t)
         - 0.35 * np.sin(8*t) + 0.0625 * np.sin(16*t))
    return x, y


# =============================================================================
# Figure 1: Full N-Lobe Hierarchy
# =============================================================================
def gen_nlobe_hierarchy():
    """8 panels showing the curve hierarchy with correct ontic ordering.

    The ontic ordering is: varpi (lemniscate) is more fundamental than pi (circle).
    G* derives from varpi. The circle is a degenerate limiting case, not a foundation.
    The figure reflects this: the top row shows the ontic descent (G* -> varpi -> pi),
    and the bottom row shows the ascent into physical structure (4,7,13,27,137 lobes).
    """
    plt.rcParams.update(STYLE)
    fig = plt.figure(figsize=(14, 8.5))

    # --- Top row: The ontic core (3 panels + derivation diagram) ---
    # Panel layout: [derivation diagram] [G* 3-lobe] [varpi lemniscate] [pi circle]
    ax_deriv = fig.add_axes([0.02, 0.54, 0.22, 0.40])
    ax_gstar = fig.add_axes([0.26, 0.54, 0.24, 0.40])
    ax_varpi = fig.add_axes([0.52, 0.54, 0.24, 0.40])
    ax_pi    = fig.add_axes([0.78, 0.54, 0.20, 0.40])

    # --- Bottom row: physical structure (5 panels) ---
    bottom_axes = []
    for i in range(5):
        ax = fig.add_axes([0.02 + i * 0.196, 0.05, 0.18, 0.40])
        bottom_axes.append(ax)

    fig.text(0.5, 0.98, 'The N-Lobe Curve Hierarchy (Ontic Ordering)',
             fontsize=14, fontweight='bold', ha='center', va='top')

    theta = np.linspace(0, 2 * np.pi, 20000)

    # --- Derivation diagram (top-left) ---
    ax_deriv.set_xlim(0, 10)
    ax_deriv.set_ylim(0, 10)
    ax_deriv.axis('off')
    ax_deriv.set_title('Ontic Descent', fontsize=11, fontweight='bold')

    # Show derivation chain: G* -> varpi -> pi
    labels = [
        (5, 8.5, r'$G^* = \frac{\sqrt{2}\,\Gamma(\frac{1}{4})^2}{2\pi}$',
         RED, 'ONTIC\nSEED'),
        (5, 5.0, r'$\varpi = \frac{G^*\,\pi}{\sqrt{2}\,M}$',
         TEAL, 'ELLIPTIC\nPRIMITIVE'),
        (5, 1.5, r'$\pi = \frac{\varpi \cdot \sqrt{2} \cdot M}{G^*/(2\pi)}$',
         NAVY, 'DERIVED\n(limiting case)'),
    ]
    for x, y, formula, color, tag in labels:
        ax_deriv.text(x, y, formula, fontsize=9, ha='center', va='center',
                      color=color, fontweight='bold',
                      bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                                edgecolor=color, alpha=0.9))
        ax_deriv.text(9.5, y, tag, fontsize=7, ha='right', va='center',
                      color=color, alpha=0.7, style='italic')

    # Arrows between levels
    for y_start, y_end in [(7.8, 5.8), (4.2, 2.3)]:
        ax_deriv.annotate('', xy=(5, y_end), xytext=(5, y_start),
                          arrowprops=dict(arrowstyle='->', color='gray',
                                          lw=1.5, ls='-'))

    # --- G* panel (3-lobe, the ontic seed) ---
    r = np.abs(np.cos(3 * theta / 2))
    x = r * np.cos(theta)
    y = r * np.sin(theta)
    ax_gstar.plot(x, y, color=RED, lw=1.5)
    ax_gstar.set_title('3-Lobe  (Ontic Seed)', fontsize=10, fontweight='bold',
                        color=RED)
    ax_gstar.set_aspect('equal')
    ax_gstar.set_xlim(-1.35, 1.35)
    ax_gstar.set_ylim(-1.35, 1.35)
    ax_gstar.tick_params(labelsize=7)
    ax_gstar.grid(True, alpha=0.15, linewidth=0.5)
    ax_gstar.plot(0, 0, '+', color='gray', markersize=6, mew=0.8)
    ax_gstar.text(0.05, 0.05, f'$G^*$ = {G_STAR:.4f}',
                  transform=ax_gstar.transAxes, fontsize=9, color=RED,
                  bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                            edgecolor=RED, alpha=0.85))

    # --- Varpi panel (lemniscate, elliptic primitive) ---
    r2 = np.maximum(np.cos(2 * theta), 0)
    r = np.sqrt(r2)
    x = r * np.cos(theta)
    y = r * np.sin(theta)
    ax_varpi.plot(x, y, color=TEAL, lw=1.5)
    ax_varpi.set_title('Lemniscate  (Elliptic Primitive)', fontsize=10,
                        fontweight='bold', color=TEAL)
    ax_varpi.set_aspect('equal')
    ax_varpi.set_xlim(-1.35, 1.35)
    ax_varpi.set_ylim(-1.35, 1.35)
    ax_varpi.tick_params(labelsize=7)
    ax_varpi.grid(True, alpha=0.15, linewidth=0.5)
    ax_varpi.plot(0, 0, '+', color='gray', markersize=6, mew=0.8)
    ax_varpi.text(0.05, 0.05, f'$\\varpi$ = {VARPI:.4f}',
                  transform=ax_varpi.transAxes, fontsize=9, color=TEAL,
                  bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                            edgecolor=TEAL, alpha=0.85))

    # --- Pi panel (circle, derived/limiting case) ---
    x = np.cos(theta)
    y = np.sin(theta)
    ax_pi.plot(x, y, color=NAVY, lw=1.2, ls='--', alpha=0.7)
    ax_pi.set_title('Circle  (Derived)', fontsize=10, color=NAVY)
    ax_pi.set_aspect('equal')
    ax_pi.set_xlim(-1.35, 1.35)
    ax_pi.set_ylim(-1.35, 1.35)
    ax_pi.tick_params(labelsize=7)
    ax_pi.grid(True, alpha=0.15, linewidth=0.5)
    ax_pi.plot(0, 0, '+', color='gray', markersize=6, mew=0.8)
    ax_pi.text(0.05, 0.05, f'$\\pi$ = {np.pi:.4f}',
               transform=ax_pi.transAxes, fontsize=9, color=NAVY,
               bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                         edgecolor=NAVY, alpha=0.85))

    # Arrows between top panels showing derivation direction
    # Use fig.text with unicode arrows since Figure.annotate doesn't exist
    fig.text(0.51, 0.74, r'$\longrightarrow$', fontsize=14, ha='center',
             va='center', color='gray')
    fig.text(0.77, 0.74, r'$\longrightarrow$', fontsize=14, ha='center',
             va='center', color='gray')

    # --- Bottom row: Physical structure (ascending complexity) ---
    fig.text(0.5, 0.47, 'Physical Structure  (ascending complexity from ontic seed)',
             fontsize=11, ha='center', va='bottom', style='italic', color='gray')

    phys_levels = [
        ('4-Lobe\n($N_{base}$)', 4, GOLD),
        ('7-Lobe\n($b_3$)', 7, PURPLE),
        ('13-Lobe\n($N_{eff}$)', 13, GREEN),
        ('27-Lobe\n($3^3$)', 27, '#D35400'),
        ('137-Lobe\n($1/\\alpha$)', 137, '#2C3E50'),
    ]

    for idx, (label, n_lobes, color) in enumerate(phys_levels):
        ax = bottom_axes[idx]
        r = np.abs(np.cos(n_lobes * theta / 2))
        if n_lobes >= 4:
            r *= (1 + 0.15 * np.cos(3 * theta))
        x = r * np.cos(theta)
        y = r * np.sin(theta)

        ax.plot(x, y, color=color, lw=0.6 if n_lobes >= 27 else 1.0)
        ax.set_title(label, fontsize=9, color=color)
        ax.set_aspect('equal')
        ax.set_xlim(-1.35, 1.35)
        ax.set_ylim(-1.35, 1.35)
        ax.tick_params(labelsize=6)
        ax.grid(True, alpha=0.15, linewidth=0.5)
        ax.plot(0, 0, '+', color='gray', markersize=5, mew=0.6)

    out = _FIGDIR / 'fig_nlobe_hierarchy.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Figure 2: Fourier Harmonic Buildup
# =============================================================================
def gen_fourier_buildup():
    """Progressive addition of harmonics {1, 2, 4, 8, 16}."""
    plt.rcParams.update(STYLE)
    fig, axes = plt.subplots(2, 3, figsize=(13, 8.5))
    fig.suptitle('Fourier Harmonic Buildup of the Lemniscate-Alpha',
                 fontsize=14, fontweight='bold', y=0.98)

    t = np.linspace(0, 2 * np.pi, 8000)

    # Harmonic definitions: (freq, x_amp, y_amp)
    harmonics = [
        (1,  1.0,    1.0),
        (2,  0.5,   -0.5),
        (4,  0.5,    0.5),
        (8,  0.4,   -0.35),
        (16, 0.0625, 0.0625),
    ]

    # Panels: show cumulative buildup
    panel_labels = [
        r'$f=1$ (fundamental)',
        r'$f=1+2$',
        r'$f=1+2+4$',
        r'$f=1+2+4+8$',
        r'$f=1+2+4+8+16$ (complete)',
        'Individual modes',
    ]

    for panel_idx in range(5):
        ax = axes[panel_idx // 3, panel_idx % 3]

        # Cumulative sum up to this harmonic
        x_cum = np.zeros_like(t)
        y_cum = np.zeros_like(t)
        for h_idx in range(panel_idx + 1):
            freq, xa, ya = harmonics[h_idx]
            x_cum += xa * np.cos(freq * t)
            y_cum += ya * np.sin(freq * t)

        # Plot previous harmonics ghosted
        if panel_idx > 0:
            x_prev = np.zeros_like(t)
            y_prev = np.zeros_like(t)
            for h_idx in range(panel_idx):
                freq, xa, ya = harmonics[h_idx]
                x_prev += xa * np.cos(freq * t)
                y_prev += ya * np.sin(freq * t)
            ax.plot(x_prev, y_prev, color='gray', lw=0.5, alpha=0.3)

        # Plot current cumulative
        color = MODE_COLORS[panel_idx]
        ax.plot(x_cum, y_cum, color=color, lw=1.5)
        ax.set_title(panel_labels[panel_idx], fontsize=10)
        ax.set_aspect('equal')
        ax.set_xlim(-2.8, 2.8)
        ax.set_ylim(-2.2, 2.2)
        ax.grid(True, alpha=0.15, linewidth=0.5)
        ax.tick_params(labelsize=7)
        ax.plot(0, 0, '+', color='gray', markersize=6, mew=0.8)

        # Mark the new frequency being added
        freq, xa, ya = harmonics[panel_idx]
        ax.text(0.95, 0.95, f'+ cos({freq}t)',
                transform=ax.transAxes, fontsize=8, ha='right', va='top',
                color=color, fontweight='bold',
                bbox=dict(boxstyle='round,pad=0.2', facecolor='white',
                          edgecolor=color, alpha=0.8))

    # Panel 6: All individual modes overlaid
    ax = axes[1, 2]
    for h_idx, (freq, xa, ya) in enumerate(harmonics):
        x_mode = xa * np.cos(freq * t)
        y_mode = ya * np.sin(freq * t)
        ax.plot(x_mode, y_mode, color=MODE_COLORS[h_idx], lw=1.0,
                label=f'f={freq}', alpha=0.8)
    ax.set_title(panel_labels[5], fontsize=10)
    ax.set_aspect('equal')
    ax.set_xlim(-1.3, 1.3)
    ax.set_ylim(-1.3, 1.3)
    ax.grid(True, alpha=0.15, linewidth=0.5)
    ax.tick_params(labelsize=7)
    ax.legend(fontsize=8, loc='lower right', framealpha=0.9)
    ax.plot(0, 0, '+', color='gray', markersize=6, mew=0.8)

    # Add annotation: frequencies are powers of 2 (Feigenbaum cascade)
    fig.text(0.5, 0.01,
             r'Frequencies $\{1, 2, 4, 8, 16\} = \{2^0, 2^1, 2^2, 2^3, 2^4\}$'
             r' — the Feigenbaum period-doubling cascade frozen into geometry',
             ha='center', fontsize=10, style='italic', color=TEAL)

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    out = _FIGDIR / 'fig_fourier_buildup.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Figure 3: Master Quadratic Phase Diagram
# =============================================================================
def gen_quadratic_phase():
    """Root trajectories of x^2 - k*G*^2*x + k*G*^3 = 0 as k varies."""
    plt.rcParams.update(STYLE)
    fig, (ax_re, ax_im) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)
    fig.suptitle(r'Master Quadratic $x^2 - kG^{*2}x + kG^{*3} = 0$: Root Trajectories',
                 fontsize=14, fontweight='bold')

    k_crit = 4.0 / G_STAR  # Critical k where discriminant = 0
    k_vals = np.linspace(0.01, 25, 2000)

    re_plus, re_minus = [], []
    im_plus, im_minus = [], []

    for k in k_vals:
        a_coeff = 1.0
        b_coeff = -k * G_STAR**2
        c_coeff = k * G_STAR**3
        disc = b_coeff**2 - 4 * a_coeff * c_coeff

        center = -b_coeff / (2 * a_coeff)

        if disc >= 0:
            half = np.sqrt(disc) / (2 * a_coeff)
            re_plus.append(center + half)
            re_minus.append(center - half)
            im_plus.append(0.0)
            im_minus.append(0.0)
        else:
            half_im = np.sqrt(-disc) / (2 * a_coeff)
            re_plus.append(center)
            re_minus.append(center)
            im_plus.append(half_im)
            im_minus.append(-half_im)

    re_plus = np.array(re_plus)
    re_minus = np.array(re_minus)
    im_plus = np.array(im_plus)
    im_minus = np.array(im_minus)

    # --- Top panel: Real parts ---
    ax_re.plot(k_vals, re_plus, color=NAVY, lw=1.5, label=r'$x_+$ (Re)')
    ax_re.plot(k_vals, re_minus, color=RED, lw=1.5, label=r'$x_-$ (Re)')

    # Mark critical k
    ax_re.axvline(k_crit, color='gray', ls='--', lw=0.8, alpha=0.6)
    ax_re.text(k_crit + 0.2, 100, f'$k_{{crit}} = 4/G^* \\approx {k_crit:.2f}$',
               fontsize=9, color='gray')

    # Mark physics point (k=16)
    k_phys = 16
    xp_phys = re_plus[np.argmin(np.abs(k_vals - k_phys))]
    xm_phys = re_minus[np.argmin(np.abs(k_vals - k_phys))]
    ax_re.plot(k_phys, xp_phys, 'o', color=NAVY, markersize=8, zorder=5)
    ax_re.plot(k_phys, xm_phys, 'o', color=RED, markersize=8, zorder=5)
    ax_re.annotate(f'$x_+ = {xp_phys:.1f}$\n$(1/\\alpha)$',
                   xy=(k_phys, xp_phys), xytext=(k_phys + 2, xp_phys - 10),
                   fontsize=9, color=NAVY, fontweight='bold',
                   arrowprops=dict(arrowstyle='->', color=NAVY, lw=1.0))
    ax_re.annotate(f'$x_- = {xm_phys:.2f}$\n$(N_c)$',
                   xy=(k_phys, xm_phys), xytext=(k_phys + 2, xm_phys + 8),
                   fontsize=9, color=RED, fontweight='bold',
                   arrowprops=dict(arrowstyle='->', color=RED, lw=1.0))

    # Mark reference frame context point (k=0.5)
    k_con = 0.5
    re_con = re_plus[np.argmin(np.abs(k_vals - k_con))]
    ax_re.plot(k_con, re_con, 's', color=PURPLE, markersize=8, zorder=5)
    ax_re.annotate(r'$k=\frac{1}{2}$ (reference frame context)',
                   xy=(k_con, re_con), xytext=(k_con + 1.5, re_con + 25),
                   fontsize=9, color=PURPLE,
                   arrowprops=dict(arrowstyle='->', color=PURPLE, lw=1.0))

    # Shade regions
    ax_re.axvspan(0, k_crit, alpha=0.06, color=PURPLE, label='Complex roots\n(reference frame context)')
    ax_re.axvspan(k_crit, 25, alpha=0.06, color=NAVY, label='Real roots\n(physics)')

    ax_re.set_ylabel('Re(x)', fontsize=11)
    ax_re.legend(fontsize=9, loc='upper left')
    ax_re.set_ylim(-5, 160)
    ax_re.grid(True, alpha=0.15)

    # --- Bottom panel: Imaginary parts ---
    ax_im.plot(k_vals, im_plus, color=PURPLE, lw=1.5, label=r'$x_+$ (Im)')
    ax_im.plot(k_vals, im_minus, color='#B565A7', lw=1.5, label=r'$x_-$ (Im)')
    ax_im.axvline(k_crit, color='gray', ls='--', lw=0.8, alpha=0.6)
    ax_im.axhline(0, color='gray', lw=0.5)

    # Mark reference frame context Im values
    im_con = im_plus[np.argmin(np.abs(k_vals - k_con))]
    ax_im.plot(k_con, im_con, 's', color=PURPLE, markersize=8, zorder=5)
    ax_im.plot(k_con, -im_con, 's', color='#B565A7', markersize=8, zorder=5)
    ax_im.annotate(f'Im = {im_con:.2f}',
                   xy=(k_con, im_con), xytext=(k_con + 2, im_con + 0.3),
                   fontsize=9, color=PURPLE,
                   arrowprops=dict(arrowstyle='->', color=PURPLE, lw=1.0))

    # Shade regions
    ax_im.axvspan(0, k_crit, alpha=0.06, color=PURPLE)
    ax_im.axvspan(k_crit, 25, alpha=0.06, color=NAVY)

    ax_im.set_xlabel('Coefficient $k$', fontsize=11)
    ax_im.set_ylabel('Im(x)', fontsize=11)
    ax_im.legend(fontsize=9, loc='upper right')
    ax_im.grid(True, alpha=0.15)

    # Label regions
    ax_im.text(k_crit / 2, ax_im.get_ylim()[0] * 0.85,
               'COMPLEX\n(reference frame context)', ha='center', fontsize=10,
               color=PURPLE, alpha=0.6, fontweight='bold')
    ax_im.text((k_crit + 25) / 2, ax_im.get_ylim()[0] * 0.85,
               'REAL\n(physics)', ha='center', fontsize=10,
               color=NAVY, alpha=0.6, fontweight='bold')

    plt.tight_layout()
    out = _FIGDIR / 'fig_quadratic_phase.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Figure 4: Origin Avoidance Annotated
# =============================================================================
def gen_origin_avoidance():
    """Lemniscate-Alpha with minimum distance ring and winding arrows."""
    plt.rcParams.update(STYLE)
    fig, ax = plt.subplots(1, 1, figsize=(8, 7))
    ax.set_title('Lemniscate-Alpha: Origin Avoidance and Winding Structure',
                 fontsize=13, fontweight='bold')

    t = np.linspace(0, 2 * np.pi, 8000)
    x, y = lemniscate_alpha(t)

    # Compute distance from origin at each point
    dist = np.sqrt(x**2 + y**2)
    min_idx = np.argmin(dist)
    min_dist = dist[min_idx]
    theoretical_min = G_STAR**2 / 32

    # Plot the curve
    ax.plot(x, y, color=NAVY, lw=1.8, zorder=3)

    # Exclusion zone circle
    circle = plt.Circle((0, 0), theoretical_min, color=RED, alpha=0.12, zorder=1)
    ax.add_artist(circle)
    circle_edge = plt.Circle((0, 0), theoretical_min, fill=False,
                              edgecolor=RED, lw=1.2, ls='--', alpha=0.7, zorder=2)
    ax.add_artist(circle_edge)

    # Mark origin
    ax.plot(0, 0, 'x', color='black', markersize=10, mew=2, zorder=5)

    # Mark closest approach point
    ax.plot(x[min_idx], y[min_idx], 'o', color=RED, markersize=8, zorder=5)

    # Draw distance line from origin to closest point
    ax.plot([0, x[min_idx]], [0, y[min_idx]], color=RED, lw=1.0, ls='-', zorder=4)

    # Annotate min distance
    mid_x = x[min_idx] / 2
    mid_y = y[min_idx] / 2
    ax.annotate(
        f'$d_{{min}} = {min_dist:.4f}$\n'
        f'$G^{{*2}}/32 = {theoretical_min:.4f}$\n'
        f'Error: {abs(min_dist - theoretical_min)/theoretical_min*100:.2f}%',
        xy=(x[min_idx], y[min_idx]),
        xytext=(x[min_idx] + 0.5, y[min_idx] - 0.6),
        fontsize=9, color=RED,
        bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                  edgecolor=RED, alpha=0.9),
        arrowprops=dict(arrowstyle='->', color=RED, lw=1.2))

    # Draw winding arrows (show clockwise double winding w = -2)
    arrow_params = dict(arrowstyle='->', color=TEAL, lw=1.5,
                        mutation_scale=15)
    # Place arrows at roughly evenly spaced parameter values
    for t_arrow in [0.3, 1.5, 3.5, 5.0]:
        idx = int(t_arrow / (2 * np.pi) * len(t))
        idx2 = min(idx + 200, len(t) - 1)
        ax.annotate('', xy=(x[idx2], y[idx2]), xytext=(x[idx], y[idx]),
                    arrowprops=arrow_params)

    # Annotation for winding number
    ax.text(0.05, 0.95, r'Winding number $w = -2$' + '\n(double clockwise loop)',
            transform=ax.transAxes, fontsize=10, va='top', color=TEAL,
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                      edgecolor=TEAL, alpha=0.9))

    # Annotation explaining the exclusion zone
    ax.text(0.05, 0.05,
            r'Exclusion zone: $r_{min} = G^{*2}/(2 \cdot N_{base}^2) = G^{*2}/32$'
            '\nThe curve never crosses the origin — the\n'
            'reference frame context roots remain complex, not real.',
            transform=ax.transAxes, fontsize=9, va='bottom',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='#FFF8F0',
                      edgecolor=GOLD, alpha=0.9))

    ax.set_aspect('equal')
    ax.set_xlim(-2.8, 2.8)
    ax.set_ylim(-2.2, 2.2)
    ax.grid(True, alpha=0.15, linewidth=0.5)
    ax.set_xlabel('x')
    ax.set_ylabel('y')

    plt.tight_layout()
    out = _FIGDIR / 'fig_origin_avoidance.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Figure 5: Feigenbaum Bifurcation Bridge
# =============================================================================
def gen_feigenbaum_bridge():
    """Logistic map bifurcation diagram with FTD integers marked."""
    plt.rcParams.update(STYLE)
    fig, (ax_bif, ax_int) = plt.subplots(1, 2, figsize=(14, 6),
                                          gridspec_kw={'width_ratios': [2, 1]})
    fig.suptitle(r'The Feigenbaum-Lemniscate Bridge: $\delta$ and $G^*$ Produce the FTD Integers',
                 fontsize=13, fontweight='bold', y=0.98)

    # --- Left panel: Bifurcation diagram ---
    # Logistic map: x_{n+1} = r * x_n * (1 - x_n)
    r_min, r_max = 2.5, 4.0
    n_r = 2000
    n_skip = 300   # transients to skip
    n_plot = 200   # points to plot per r

    r_vals = np.linspace(r_min, r_max, n_r)
    r_all = []
    x_all = []

    for r in r_vals:
        x = 0.5
        # Skip transients
        for _ in range(n_skip):
            x = r * x * (1 - x)
        # Collect steady state
        for _ in range(n_plot):
            x = r * x * (1 - x)
            r_all.append(r)
            x_all.append(x)

    ax_bif.plot(r_all, x_all, ',', color=NAVY, alpha=0.15, markersize=0.2)
    ax_bif.set_xlabel('$r$ (growth rate)', fontsize=11)
    ax_bif.set_ylabel('$x^*$ (steady state)', fontsize=11)
    ax_bif.set_title('Logistic Map Bifurcation Diagram', fontsize=11)
    ax_bif.set_xlim(r_min, r_max)
    ax_bif.set_ylim(0, 1)
    ax_bif.grid(True, alpha=0.15)

    # Mark the period-doubling bifurcation points
    # First few bifurcation points of logistic map
    r_bifs = [3.0, 3.44949, 3.54409, 3.5644, 3.5688]
    for i, r_b in enumerate(r_bifs[:4]):
        ax_bif.axvline(r_b, color=RED, ls=':', lw=0.6, alpha=0.5)
        if i < 3:
            ax_bif.text(r_b, 0.02, f'$r_{i+1}$', fontsize=8, color=RED,
                        ha='center')

    # Mark accumulation point (onset of chaos)
    r_inf = 3.5699456  # Feigenbaum accumulation point
    ax_bif.axvline(r_inf, color=RED, ls='--', lw=1.0, alpha=0.7)
    ax_bif.text(r_inf + 0.01, 0.95, r'$r_\infty$' + f'\n({r_inf:.4f})',
                fontsize=9, color=RED, va='top')

    # Annotate delta
    ax_bif.text(0.02, 0.02,
                r'$\delta = \lim \frac{r_{n}-r_{n-1}}{r_{n+1}-r_n} = 4.66920...$'
                '\n(Feigenbaum universal constant)',
                transform=ax_bif.transAxes, fontsize=9,
                bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                          edgecolor=RED, alpha=0.9))

    # --- Right panel: Integer derivation diagram ---
    ax_int.set_xlim(0, 10)
    ax_int.set_ylim(-0.5, 5.5)
    ax_int.set_title(r'$\delta$ + $G^*$ $\rightarrow$ FTD Integers', fontsize=11)
    ax_int.axis('off')

    # Show the four operations
    operations = [
        (r'$\lfloor \delta \rfloor$',
         f'{DELTA_F:.3f}', f'= {int(np.floor(DELTA_F))}',
         f'$N_{{base}}$ = {N_BASE}', INT_COLORS[4]),

        (r'$\lfloor \delta + G^* \rfloor$',
         f'{DELTA_F + G_STAR:.3f}', f'= {int(np.floor(DELTA_F + G_STAR))}',
         f'$b_3$ = {B_3}', INT_COLORS[7]),

        (r'$\lfloor \delta \times G^* \rfloor$',
         f'{DELTA_F * G_STAR:.3f}', f'= {int(np.floor(DELTA_F * G_STAR))}',
         f'$N_{{eff}}$ = {N_EFF}', INT_COLORS[13]),

        (r'round$(\delta - G^* + 1)$',
         f'{DELTA_F - G_STAR + 1:.3f}', f'= {round(DELTA_F - G_STAR + 1)}',
         f'$N_c$ = {N_C}', INT_COLORS[3]),
    ]

    y_start = 4.8
    for i, (op, val, eq, name, color) in enumerate(operations):
        y = y_start - i * 1.3

        # Operation name
        ax_int.text(0.5, y, op, fontsize=13, va='center', ha='left',
                    fontweight='bold')

        # Numerical value
        ax_int.text(4.5, y, val, fontsize=11, va='center', ha='center',
                    family='monospace')

        # Result
        ax_int.text(6.2, y, eq, fontsize=13, va='center', ha='center',
                    fontweight='bold', color=color)

        # FTD integer name
        ax_int.text(8.0, y, name, fontsize=12, va='center', ha='center',
                    color=color, fontweight='bold',
                    bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                              edgecolor=color, alpha=0.9))

    # Constants at top
    ax_int.text(5, 5.3,
                rf'$\delta = {DELTA_F:.6f}$    $G^* = {G_STAR:.6f}$',
                fontsize=10, ha='center', va='bottom',
                bbox=dict(boxstyle='round,pad=0.4', facecolor='#F0F0F0',
                          edgecolor='gray'))

    # Bottom note
    ax_int.text(5, -0.2,
                'Both constants are universal:\n'
                r'$\delta$ from chaos theory,  $G^*$ from elliptic integrals',
                fontsize=9, ha='center', va='top', style='italic', color='gray')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    out = _FIGDIR / 'fig_feigenbaum_bridge.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Main
# =============================================================================
if __name__ == '__main__':
    print('Generating FTD Curve Family Figures...')
    print()
    gen_nlobe_hierarchy()
    gen_fourier_buildup()
    gen_quadratic_phase()
    gen_origin_avoidance()
    gen_feigenbaum_bridge()
    print()
    print('All 5 figures generated.')
