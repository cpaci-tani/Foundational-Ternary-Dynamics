"""
Deep Exploration of the Feigenbaum Bifurcation Diagram
======================================================
Zooms into the self-similar structures, periodic windows,
and the "consistency in chaos" that the diagram reveals.

Generates: fig_bifurcation_deep.png
"""

from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Rectangle
from scipy.special import gamma

# Output directory
_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'
_FIGDIR.mkdir(parents=True, exist_ok=True)

# Constants
G_STAR = np.sqrt(2) * (gamma(0.25)**2) / (2 * np.pi)
DELTA_F = 4.669201609102990
ALPHA_F = 2.502907875095892   # Feigenbaum's second constant (vertical scaling)

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
}

# Colors
NAVY = '#003366'
RED = '#CC0000'
TEAL = '#006699'
PURPLE = '#7B2D8E'
GOLD = '#CC9900'
GREEN = '#228B22'


def compute_bifurcation(r_min, r_max, n_r=4000, n_skip=500, n_plot=300):
    """Compute the bifurcation diagram for the logistic map."""
    r_vals = np.linspace(r_min, r_max, n_r)
    r_all = []
    x_all = []

    for r in r_vals:
        x = 0.1 + 0.3 * np.random.random()  # slightly randomized IC
        for _ in range(n_skip):
            x = r * x * (1 - x)
        for _ in range(n_plot):
            x = r * x * (1 - x)
            r_all.append(r)
            x_all.append(x)

    return np.array(r_all), np.array(x_all)


def compute_lyapunov(r_min, r_max, n_r=2000, n_iter=1000, n_skip=500):
    """Compute the Lyapunov exponent across r values."""
    r_vals = np.linspace(r_min, r_max, n_r)
    lyap = np.zeros(n_r)

    for i, r in enumerate(r_vals):
        x = 0.5
        for _ in range(n_skip):
            x = r * x * (1 - x)
        log_sum = 0.0
        for _ in range(n_iter):
            deriv = abs(r * (1 - 2 * x))
            if deriv > 0:
                log_sum += np.log(deriv)
            x = r * x * (1 - x)
        lyap[i] = log_sum / n_iter

    return r_vals, lyap


def gen_bifurcation_deep():
    """Multi-panel deep exploration of bifurcation structure."""
    plt.rcParams.update(STYLE)

    fig = plt.figure(figsize=(16, 14))
    fig.suptitle('Self-Similarity in the Feigenbaum Diagram:\nConsistency Within Chaos',
                 fontsize=15, fontweight='bold', y=0.99)

    # Layout:
    # Row 1: Full diagram (wide) | Lyapunov exponent (wide)
    # Row 2: Period-3 window zoom | Accumulation point zoom | Period-5 window
    # Row 3: Deep self-similarity | Attractor density | Sarkovskii ordering diagram

    ax_full = fig.add_axes([0.06, 0.68, 0.58, 0.27])
    ax_lyap = fig.add_axes([0.06, 0.56, 0.58, 0.10], sharex=ax_full)
    ax_p3   = fig.add_axes([0.70, 0.56, 0.27, 0.40])
    ax_acc  = fig.add_axes([0.06, 0.29, 0.29, 0.24])
    ax_deep = fig.add_axes([0.38, 0.29, 0.29, 0.24])
    ax_dens = fig.add_axes([0.70, 0.29, 0.27, 0.24])
    ax_sark = fig.add_axes([0.06, 0.03, 0.29, 0.22])
    ax_univ = fig.add_axes([0.38, 0.03, 0.29, 0.22])
    ax_note = fig.add_axes([0.70, 0.03, 0.27, 0.22])

    # =========================================================================
    # Panel A: Full bifurcation diagram (high resolution)
    # =========================================================================
    print('  Computing full bifurcation diagram...')
    r_all, x_all = compute_bifurcation(2.5, 4.0, n_r=5000, n_skip=600, n_plot=400)
    ax_full.plot(r_all, x_all, ',', color=NAVY, alpha=0.08, markersize=0.15,
                 rasterized=True)
    ax_full.set_xlim(2.5, 4.0)
    ax_full.set_ylim(0, 1)
    ax_full.set_ylabel('$x^*$', fontsize=11)
    ax_full.set_title('(A) Full Bifurcation Diagram with Periodic Windows',
                       fontsize=11, fontweight='bold')
    ax_full.tick_params(labelbottom=False)

    # Label key periodic windows
    windows = [
        (3.0, 'Period 2', 0.67),
        (3.449, 'Period 4', 0.85),
        (3.544, 'P-8', 0.88),
        (3.627, 'P-6', 0.72),
        (3.739, 'P-5', 0.67),
        (3.831, 'Period 3', 0.5),
    ]
    for r_w, label, y_pos in windows:
        ax_full.axvline(r_w, color=RED, ls=':', lw=0.4, alpha=0.4)
        ax_full.text(r_w, y_pos, label, fontsize=7, ha='center', color=RED,
                     rotation=90, alpha=0.7,
                     bbox=dict(boxstyle='round,pad=0.15', facecolor='white',
                               edgecolor='none', alpha=0.85))

    # Mark the zoom regions with colored rectangles
    # Period-3 zoom box
    rect_p3 = Rectangle((3.82, 0.1), 0.06, 0.8, lw=1.5, edgecolor=TEAL,
                         facecolor=TEAL, alpha=0.08)
    ax_full.add_patch(rect_p3)
    ax_full.text(3.85, 0.05, 'B', fontsize=10, color=TEAL, fontweight='bold')

    # Accumulation point zoom box
    rect_acc = Rectangle((3.54, 0.3), 0.035, 0.55, lw=1.5, edgecolor=PURPLE,
                          facecolor=PURPLE, alpha=0.08)
    ax_full.add_patch(rect_acc)
    ax_full.text(3.555, 0.25, 'C', fontsize=10, color=PURPLE, fontweight='bold')

    # Accumulation point
    r_inf = 3.5699456
    ax_full.axvline(r_inf, color=RED, ls='--', lw=0.8, alpha=0.5)
    ax_full.text(r_inf + 0.005, 0.97, r'$r_\infty$', fontsize=9, color=RED,
                 va='top')

    # =========================================================================
    # Panel A': Lyapunov exponent (below full diagram)
    # =========================================================================
    print('  Computing Lyapunov exponent...')
    r_lyap, lyap = compute_lyapunov(2.5, 4.0, n_r=3000)
    ax_lyap.fill_between(r_lyap, 0, lyap, where=(lyap > 0),
                          color=RED, alpha=0.3, label=r'$\lambda > 0$ (chaos)')
    ax_lyap.fill_between(r_lyap, 0, lyap, where=(lyap <= 0),
                          color=NAVY, alpha=0.3, label=r'$\lambda < 0$ (order)')
    ax_lyap.plot(r_lyap, lyap, color='black', lw=0.3, alpha=0.6)
    ax_lyap.axhline(0, color='gray', lw=0.5)
    ax_lyap.set_ylabel(r'$\lambda$', fontsize=10)
    ax_lyap.set_xlabel('$r$', fontsize=11)
    ax_lyap.set_ylim(-2, 1)
    ax_lyap.legend(fontsize=8, loc='lower right', ncol=2, framealpha=0.9)
    ax_lyap.text(0.01, 0.95, r"Lyapunov exponent: $\lambda > 0 \Rightarrow$ chaos",
                 transform=ax_lyap.transAxes, fontsize=8, va='top', alpha=0.7)

    # =========================================================================
    # Panel B: Period-3 window zoom (self-similar copy)
    # =========================================================================
    print('  Computing period-3 window zoom...')
    r_p3, x_p3 = compute_bifurcation(3.82, 3.858, n_r=4000, n_skip=800, n_plot=500)
    ax_p3.plot(r_p3, x_p3, ',', color=TEAL, alpha=0.15, markersize=0.3,
               rasterized=True)
    ax_p3.set_xlim(3.82, 3.858)
    ax_p3.set_ylim(0.12, 0.97)
    ax_p3.set_title('(B) Period-3 Window Zoom\n'
                     r'$\leftarrow$ A miniature copy of the entire diagram',
                     fontsize=10, fontweight='bold', color=TEAL)
    ax_p3.set_xlabel('$r$', fontsize=10)
    ax_p3.set_ylabel('$x^*$', fontsize=10)
    ax_p3.tick_params(labelsize=8)

    # Highlight the three branches
    ax_p3.text(3.834, 0.16, 'Branch 1', fontsize=7, color=TEAL, alpha=0.7)
    ax_p3.text(3.834, 0.50, 'Branch 2', fontsize=7, color=TEAL, alpha=0.7)
    ax_p3.text(3.834, 0.88, 'Branch 3', fontsize=7, color=TEAL, alpha=0.7)

    # Mark where P-3 itself period-doubles
    ax_p3.axvline(3.8415, color=RED, ls=':', lw=0.5, alpha=0.5)
    ax_p3.text(3.8415, 0.13, 'P-3 doubles\nto P-6', fontsize=7, ha='center',
               color=RED, alpha=0.7)

    ax_p3.text(0.05, 0.05,
               'Each periodic window contains\n'
               'its own period-doubling cascade\n'
               r'governed by the same $\delta$!',
               transform=ax_p3.transAxes, fontsize=9, va='bottom',
               bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                         edgecolor=TEAL, alpha=0.9))

    # =========================================================================
    # Panel C: Accumulation point zoom (X-shapes)
    # =========================================================================
    print('  Computing accumulation point zoom...')
    r_acc, x_acc = compute_bifurcation(3.54, 3.575, n_r=5000, n_skip=1000, n_plot=400)
    ax_acc.plot(r_acc, x_acc, ',', color=PURPLE, alpha=0.2, markersize=0.3,
                rasterized=True)
    ax_acc.set_xlim(3.54, 3.575)
    ax_acc.set_ylim(0.34, 0.9)
    ax_acc.set_title('(C) Accumulation Point: The X-Shapes',
                      fontsize=10, fontweight='bold', color=PURPLE)
    ax_acc.set_xlabel('$r$', fontsize=10)
    ax_acc.set_ylabel('$x^*$', fontsize=10)
    ax_acc.tick_params(labelsize=8)

    # Mark bifurcation points to show the ratio converging to delta
    r_bifs = [3.5441, 3.5644, 3.5688, 3.56969]
    for i, rb in enumerate(r_bifs):
        ax_acc.axvline(rb, color=RED, ls=':', lw=0.5, alpha=0.5)

    # Show delta ratio
    if len(r_bifs) >= 3:
        d1 = r_bifs[1] - r_bifs[0]
        d2 = r_bifs[2] - r_bifs[1]
        ratio = d1 / d2 if d2 > 0 else 0
        ax_acc.text(0.05, 0.95,
                    f'Successive ratios\n'
                    r'$\rightarrow \delta = 4.669...$'
                    f'\nMeasured: {ratio:.2f}',
                    transform=ax_acc.transAxes, fontsize=9, va='top',
                    bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                              edgecolor=PURPLE, alpha=0.9))

    # =========================================================================
    # Panel D: Deep self-similarity (zoom into chaos)
    # =========================================================================
    print('  Computing deep self-similarity zoom...')
    # Zoom into the chaotic region near r = 3.593 where there's a tiny P-12 window
    r_deep, x_deep = compute_bifurcation(3.59, 3.64, n_r=5000, n_skip=1000,
                                          n_plot=500)
    ax_deep.plot(r_deep, x_deep, ',', color=NAVY, alpha=0.12, markersize=0.25,
                 rasterized=True)
    ax_deep.set_xlim(3.59, 3.64)
    ax_deep.set_ylim(0.0, 1.0)
    ax_deep.set_title('(D) Deep Zoom: Structure in Chaos',
                       fontsize=10, fontweight='bold')
    ax_deep.set_xlabel('$r$', fontsize=10)
    ax_deep.set_ylabel('$x^*$', fontsize=10)
    ax_deep.tick_params(labelsize=8)

    # Mark a visible periodic window
    ax_deep.axvline(3.6275, color=GREEN, ls=':', lw=0.8, alpha=0.6)
    ax_deep.text(3.628, 0.05, 'P-6\nwindow', fontsize=7, color=GREEN,
                 ha='center')

    ax_deep.text(0.05, 0.95,
                 '"Chaos" is dense with\n'
                 'periodic windows—\n'
                 'order nested in disorder\n'
                 'nested in order...',
                 transform=ax_deep.transAxes, fontsize=9, va='top',
                 style='italic', color=NAVY,
                 bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                           edgecolor=NAVY, alpha=0.9))

    # =========================================================================
    # Panel E: Attractor density histogram
    # =========================================================================
    print('  Computing attractor density at r=4...')
    # At r=4 (fully chaotic), the invariant density is known analytically:
    # rho(x) = 1 / (pi * sqrt(x(1-x)))
    x_hist = []
    x = 0.3
    for _ in range(500):
        x = 4.0 * x * (1 - x)
    for _ in range(100000):
        x = 4.0 * x * (1 - x)
        x_hist.append(x)

    ax_dens.hist(x_hist, bins=200, density=True, color=NAVY, alpha=0.5,
                 edgecolor='none')

    # Overlay analytical density
    x_an = np.linspace(0.001, 0.999, 500)
    rho_an = 1.0 / (np.pi * np.sqrt(x_an * (1 - x_an)))
    ax_dens.plot(x_an, rho_an, color=RED, lw=2.0,
                 label=r'$\rho(x) = \frac{1}{\pi\sqrt{x(1-x)}}$')

    ax_dens.set_title('(E) Attractor Density at $r=4$',
                       fontsize=10, fontweight='bold')
    ax_dens.set_xlabel('$x^*$', fontsize=10)
    ax_dens.set_ylabel('Density', fontsize=10)
    ax_dens.set_xlim(0, 1)
    ax_dens.set_ylim(0, 5)
    ax_dens.tick_params(labelsize=8)
    ax_dens.legend(fontsize=9, loc='upper center')
    ax_dens.text(0.5, 0.6,
                 'Even "total chaos"\nhas exact structure:\n'
                 r'$\rho(x)$ is a known function',
                 transform=ax_dens.transAxes, fontsize=9, ha='center',
                 style='italic', color=NAVY, alpha=0.7)

    # =========================================================================
    # Panel F: Sarkovskii ordering diagram
    # =========================================================================
    ax_sark.set_xlim(0, 10)
    ax_sark.set_ylim(0, 10)
    ax_sark.axis('off')
    ax_sark.set_title("(F) Sarkovskii's Ordering", fontsize=10, fontweight='bold')

    # Sarkovskii ordering (partial)
    ordering = [
        'Powers of 2:',
        r'$1 \triangleleft 2 \triangleleft 4 \triangleleft 8 \triangleleft 16 \triangleleft \cdots$',
        '',
        'Odd × powers of 2:',
        r'$\cdots \triangleleft 12 \triangleleft 10 \triangleleft 6$',
        r'$\cdots \triangleleft 9 \triangleleft 7 \triangleleft 5 \triangleleft 3$',
    ]

    y_pos = 9.0
    for line in ordering:
        fontsize = 9 if line.startswith(('P', 'O')) else 10
        style = 'italic' if line.endswith(':') else 'normal'
        color = NAVY if '$' in line else 'gray'
        ax_sark.text(0.5, y_pos, line, fontsize=fontsize, va='top',
                     ha='left', color=color, style=style)
        y_pos -= 1.2

    ax_sark.text(0.5, 1.0,
                 'Period 3 is last  (strongest)\n'
                 r'$\Rightarrow$ Period-3 $\Rightarrow$ all other periods exist',
                 fontsize=9, va='bottom', ha='left', color=RED,
                 bbox=dict(boxstyle='round,pad=0.3', facecolor='#FFF0F0',
                           edgecolor=RED, alpha=0.9))

    # =========================================================================
    # Panel G: Universality demonstration
    # =========================================================================
    print('  Computing universality comparison...')
    ax_univ.set_title('(G) Universality: Different Maps, Same $\\delta$',
                       fontsize=10, fontweight='bold')

    # Compare logistic map with sine map: x_{n+1} = r * sin(pi * x_n)
    # Both have the same Feigenbaum constant!
    r_min_s, r_max_s = 0.7, 1.0
    r_vals_s = np.linspace(r_min_s, r_max_s, 3000)
    r_s_all, x_s_all = [], []
    for r in r_vals_s:
        x = 0.5
        for _ in range(500):
            x = r * np.sin(np.pi * x)
            x = max(0.001, min(x, 0.999))  # keep bounded
        for _ in range(200):
            x = r * np.sin(np.pi * x)
            x = max(0.001, min(x, 0.999))
            r_s_all.append(r)
            x_s_all.append(x)

    ax_univ.plot(r_s_all, x_s_all, ',', color=GREEN, alpha=0.12, markersize=0.3,
                 rasterized=True)
    ax_univ.set_xlim(r_min_s, r_max_s)
    ax_univ.set_ylim(0, 1)
    ax_univ.set_xlabel('$r$ (sine map)', fontsize=10)
    ax_univ.set_ylabel('$x^*$', fontsize=10)
    ax_univ.tick_params(labelsize=8)

    ax_univ.text(0.05, 0.95,
                 r'$x_{n+1} = r \sin(\pi x_n)$' + '\n'
                 'Completely different map,\n'
                 r'same $\delta = 4.669...$' + '\n'
                 'same structural patterns',
                 transform=ax_univ.transAxes, fontsize=9, va='top',
                 color=GREEN,
                 bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                           edgecolor=GREEN, alpha=0.9))

    # =========================================================================
    # Panel H: Summary / reflection
    # =========================================================================
    ax_note.set_xlim(0, 10)
    ax_note.set_ylim(0, 10)
    ax_note.axis('off')
    ax_note.set_title('(H) What the Patterns Mean', fontsize=10, fontweight='bold')

    insights = [
        (r'$\mathbf{X}$-shapes:', 'Period-doubling forks—each\norbit splits in two, scaled by '
         r'$\delta$'),
        ('Blank bands:', 'Periodic windows—islands of\norder with their own '
         r'$\delta$-cascades'),
        ('Self-similarity:', 'Zoom in anywhere and find\nthe whole diagram repeated'),
        ('Universality:', r'$\delta$ appears in turbulence,'
         '\ndripping faucets, heart rhythms...'),
    ]

    y = 9.0
    for label, desc in insights:
        ax_note.text(0.3, y, label, fontsize=10, va='top', ha='left',
                     fontweight='bold', color=NAVY)
        ax_note.text(3.2, y, desc, fontsize=9, va='top', ha='left',
                     color='#333333')
        y -= 2.3

    ax_note.text(0.3, 0.5,
                 r'"Chaos is not the opposite of order—' + '\n'
                 r'it is order with infinite depth."',
                 fontsize=10, va='bottom', ha='left',
                 style='italic', color=PURPLE)

    # =========================================================================
    # Save
    # =========================================================================
    out = _FIGDIR / 'fig_bifurcation_deep.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')
    return out


if __name__ == '__main__':
    print('Generating deep bifurcation exploration...')
    print()
    gen_bifurcation_deep()
    print('\nDone.')
