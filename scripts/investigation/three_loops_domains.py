#!/usr/bin/env python3
"""
three_loops_domains.py — The Three Loops Through Six Lenses
============================================================

Six domain-specific figures, each showing the trifecta
(circle/pi, lemniscate/varpi, lemniscate-alpha/G*)
through a different scientific domain.

Outputs:
  domain_1_mathematics.png   — The Pure Geometry
  domain_2_physics.png       — Relativity as Shadow
  domain_3_astronomy.png     — Orbital Harmonics
  domain_4_number_theory.png — The Master Quadratic
  domain_5_chemistry.png     — The Coupling Constant
  domain_6_philosophy.png    — The Ontological Hierarchy
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Circle as MplCircle
from math import gamma, sqrt, pi
import os

# =============================================================================
# SHARED CONSTANTS
# =============================================================================

G4 = gamma(0.25)
G4sq = G4**2
sqrt2 = sqrt(2)

varpi = G4sq / (2 * sqrt(2 * pi))
star = 2 / sqrt(pi)
G_star = sqrt2 * G4sq / (2 * pi)

disc = 256 * G_star**4 - 64 * G_star**3
x_plus = (16 * G_star**2 + sqrt(disc)) / 2
x_minus = (16 * G_star**2 - sqrt(disc)) / 2

alpha_val = 1.0 / x_plus

OUT_DIR = os.path.dirname(os.path.abspath(__file__))

# =============================================================================
# SHARED STYLE
# =============================================================================

BG_DARK = '#0d1117'
BG_PANEL = '#161b22'
BLUE = '#58a6ff'
GOLD = '#f0c040'
RED = '#ff6b6b'
WHITE = '#e6edf3'
GREY = '#8b949e'
GREEN = '#3fb950'
PURPLE = '#bc8cff'
CYAN = '#56d4dd'

def setup_rcparams():
    plt.rcParams.update({
        'figure.facecolor': BG_DARK,
        'axes.facecolor': BG_PANEL,
        'axes.edgecolor': GREY,
        'axes.labelcolor': WHITE,
        'text.color': WHITE,
        'xtick.color': GREY,
        'ytick.color': GREY,
        'font.family': 'sans-serif',
        'font.size': 10,
        'mathtext.fontset': 'cm',
    })

# =============================================================================
# SHARED CURVE FUNCTIONS
# =============================================================================

def circle_xy(t):
    return np.cos(t), np.sin(t)

def lemniscate_xy(theta):
    c2 = np.cos(2 * theta)
    mask = c2 > 0
    r = np.where(mask, np.sqrt(np.maximum(c2, 0)), np.nan)
    return r * np.cos(theta), r * np.sin(theta)

def lemniscate_alpha_xy(t):
    x = (np.cos(t) + 0.5*np.cos(2*t) + 0.5*np.cos(4*t) +
         0.4*np.cos(8*t) + 0.0625*np.cos(16*t))
    y = (np.sin(t) - 0.5*np.sin(2*t) + 0.5*np.sin(4*t) -
         0.35*np.sin(8*t) + 0.0625*np.sin(16*t))
    return x, y

def save_fig(fig, name):
    path = os.path.join(OUT_DIR, name)
    fig.savefig(path, dpi=300, bbox_inches='tight',
                facecolor=fig.get_facecolor(), edgecolor='none')
    print(f"  Saved: {name}")
    plt.close(fig)


# =============================================================================
# FIGURE 1: MATHEMATICS — The Pure Geometry
# =============================================================================

def fig1_mathematics():
    setup_rcparams()
    fig, ax = plt.subplots(figsize=(10, 10))
    ax.set_facecolor(BG_DARK)
    ax.set_aspect('equal')

    t = np.linspace(0, 2*np.pi, 3000)

    # Circle
    cx, cy = circle_xy(t)
    ax.plot(cx, cy, color=BLUE, linewidth=2.0, alpha=0.75, zorder=2)

    # Lemniscate — both lobes
    th1 = np.linspace(-np.pi/4 + 0.001, np.pi/4 - 0.001, 1000)
    th2 = np.linspace(3*np.pi/4 + 0.001, 5*np.pi/4 - 0.001, 1000)
    lx1, ly1 = lemniscate_xy(th1)
    lx2, ly2 = lemniscate_xy(th2)
    ax.plot(lx1, ly1, color=GOLD, linewidth=2.8, alpha=0.9, zorder=3)
    ax.plot(lx2, ly2, color=GOLD, linewidth=2.8, alpha=0.9, zorder=3)

    # Lemniscate-Alpha — scaled to nest inside lemniscate
    la_x, la_y = lemniscate_alpha_xy(t)
    sc = 0.42 / max(np.max(np.abs(la_x)), np.max(np.abs(la_y)))
    ax.plot(la_x * sc, la_y * sc, color=RED, linewidth=1.6, alpha=0.9, zorder=4)

    # Origin glow
    for s, a in [(12, 0.08), (7, 0.15), (4, 0.4)]:
        ax.plot(0, 0, 'o', color=WHITE, markersize=s, alpha=a, zorder=5)
    ax.plot(0, 0, 'o', color=WHITE, markersize=2.5, alpha=1.0, zorder=6)

    # Constant labels
    ax.text(0.0, 1.13, r'$\pi = 3.14159\ldots$', fontsize=13, color=BLUE,
            ha='center', va='bottom', fontweight='bold')
    ax.text(0.95, 0.08, r'$\varpi = 2.62206\ldots$', fontsize=13, color=GOLD,
            ha='left', va='bottom', fontweight='bold')
    ax.text(0.0, -0.55, r'$G^{\!*} = 2.95868\ldots$', fontsize=13, color=RED,
            ha='center', va='top', fontweight='bold')

    # Lobe annotations
    ax.text(0.72, 0.72, '1 lobe', fontsize=9, color=BLUE, alpha=0.6, ha='center')
    ax.text(0.6, -0.28, '2 lobes', fontsize=9, color=GOLD, alpha=0.6, ha='center')
    ax.text(-0.35, 0.35, '3 lobes', fontsize=9, color=RED, alpha=0.6, ha='center')

    # Defining integrals at bottom
    ax.text(0.0, -1.28, (r'$\frac{\pi}{2} = \int_0^1 \frac{dt}{\sqrt{1-t^2}}$'
                          r'$\qquad\qquad$'
                          r'$\frac{\varpi}{2} = \int_0^1 \frac{dt}{\sqrt{1-t^4}}$'),
            fontsize=11, color=GREY, ha='center', va='top')

    ax.set_xlim(-1.45, 1.45)
    ax.set_ylim(-1.45, 1.45)
    ax.set_xticks([])
    ax.set_yticks([])
    for spine in ax.spines.values():
        spine.set_linewidth(0.3)

    # Title
    fig.text(0.5, 0.96, 'THE PURE GEOMETRY', ha='center', va='top',
             fontsize=22, fontweight='bold', color=WHITE)
    fig.text(0.5, 0.93, 'Circle  .  Lemniscate  .  Lemniscate-Alpha',
             ha='center', va='top', fontsize=12, color=GREY, style='italic')

    save_fig(fig, 'domain_1_mathematics.png')


# =============================================================================
# FIGURE 2: PHYSICS — The Kernels: Relativity as Shadow
# =============================================================================

def fig2_physics():
    setup_rcparams()
    fig, ax = plt.subplots(figsize=(12, 8))

    t = np.linspace(0, 0.96, 1000)
    K2 = 1.0 / np.sqrt(1 - t**2)
    K4 = 1.0 / np.sqrt(1 - t**4)

    # Shaded gap
    ax.fill_between(t, K4, K2, alpha=0.12, color=GREEN, zorder=1)

    # Kernel curves
    ax.plot(t, K2, color=BLUE, linewidth=2.8, label=r'$K_2(\beta) = 1/\sqrt{1-\beta^2}$',
            zorder=3)
    ax.plot(t, K4, color=GOLD, linewidth=2.8, label=r'$K_4(\beta) = 1/\sqrt{1-\beta^4}$',
            zorder=3)

    # Reference velocities
    for v, lbl, col in [(0.1, 'v = 0.1c', GREY),
                         (1/sqrt2, r'v = c/$\sqrt{2}$', WHITE),
                         (0.9, 'v = 0.9c', GREY)]:
        ax.axvline(x=v, color=col, linewidth=0.8, linestyle=':', alpha=0.4)
        dev = (1 - 1/sqrt(1 + v**2)) * 100
        ax.text(v + 0.012, 0.95, f'{lbl}\n({dev:.1f}% gap)',
                fontsize=7.5, color=col, alpha=0.7, va='bottom', rotation=90)

    # Annotations
    ax.annotate(r'$\gamma_{\mathrm{Lorentz}} = K_2(v/c)$',
                xy=(0.78, 1/sqrt(1-0.78**2)),
                xytext=(0.42, 3.3),
                fontsize=11, color=BLUE, fontweight='bold',
                arrowprops=dict(arrowstyle='->', color=BLUE, lw=1.5),
                ha='center')

    ax.annotate('self-reference\ncorrection',
                xy=(0.65, (1/sqrt(1-0.65**2) + 1/sqrt(1-0.65**4))/2),
                xytext=(0.25, 2.0),
                fontsize=10, color=GREEN,
                arrowprops=dict(arrowstyle='->', color=GREEN, lw=1.2, alpha=0.7),
                ha='center')

    # Equation box
    eq_text = (r'$K_4 = K_2 \,/\, \sqrt{1 + \beta^2}$' + '\n\n'
               r'$\mathrm{Extra\ factor} = 1/\sqrt{1+\beta^2}$')
    ax.text(0.12, 3.5, eq_text, fontsize=10, color=WHITE,
            va='top', ha='left',
            bbox=dict(boxstyle='round,pad=0.4', facecolor=BG_DARK,
                      edgecolor=GREY, alpha=0.9))

    ax.set_xlabel(r'$\beta = v/c$', fontsize=13)
    ax.set_ylabel('Kernel value', fontsize=13)
    ax.set_xlim(0, 0.98)
    ax.set_ylim(0.85, 4.0)
    ax.legend(loc='upper left', fontsize=10, facecolor=BG_DARK,
              edgecolor=GREY, labelcolor=WHITE, framealpha=0.9)

    # Title
    fig.text(0.5, 0.97, 'THE KERNELS: RELATIVITY AS SHADOW', ha='center',
             va='top', fontsize=20, fontweight='bold', color=WHITE)
    fig.text(0.5, 0.935, r'Einstein found $K_2$. The full picture is $K_4$.',
             ha='center', va='top', fontsize=12, color=GREY, style='italic')

    # Bottom caption
    fig.text(0.5, 0.01,
             r'$K_2$ describes how an observer sees the world.  '
             r'$K_4$ describes an observer seeing itself seeing the world.',
             ha='center', va='bottom', fontsize=9, color=GREY)

    save_fig(fig, 'domain_2_physics.png')


# =============================================================================
# FIGURE 3: ASTRONOMY — Orbital Harmonics
# =============================================================================

def fig3_astronomy():
    setup_rcparams()
    fig, ax = plt.subplots(figsize=(11, 10))
    ax.set_facecolor(BG_DARK)
    ax.set_aspect('equal')

    freqs = [1, 2, 4, 8, 16]
    x_amps = [1.0, 0.5, 0.5, 0.4, 0.0625]
    colors_orbit = [BLUE, '#7db8f0', GOLD, '#e8a040', RED]
    labels_f = ['f = 1', 'f = 2', 'f = 4', 'f = 8', 'f = 16']

    # Kepler-scaled radii: r = freq^(2/3), normalized
    radii_raw = [f**(2/3) for f in freqs]
    r_max = max(radii_raw)
    radii = [r / r_max for r in radii_raw]

    t_orbit = np.linspace(0, 2*np.pi, 500)

    # Central star glow
    for s, a in [(35, 0.03), (22, 0.06), (14, 0.12), (8, 0.25), (4, 0.6)]:
        ax.plot(0, 0, 'o', color=GOLD, markersize=s, alpha=a, zorder=10)
    ax.plot(0, 0, 'o', color=WHITE, markersize=3, alpha=1.0, zorder=11)

    # Orbits + planets
    planet_angles = [0.3, 1.2, 2.5, 4.0, 5.3]  # arbitrary phase angles
    for i, (r, col, lbl, amp, pa) in enumerate(
            zip(radii, colors_orbit, labels_f, x_amps, planet_angles)):
        # Orbit ring
        ox, oy = r * np.cos(t_orbit), r * np.sin(t_orbit)
        ax.plot(ox, oy, color=col, linewidth=1.2, alpha=0.5, zorder=2)

        # Planet dot
        px, py = r * np.cos(pa), r * np.sin(pa)
        ax.plot(px, py, 'o', color=col, markersize=7, zorder=8)
        ax.plot(px, py, 'o', color=WHITE, markersize=3, zorder=9)

        # Frequency label
        label_angle = pa + 0.4
        lx, ly = (r + 0.04) * np.cos(label_angle), (r + 0.04) * np.sin(label_angle)
        ax.text(lx, ly, lbl, fontsize=8, color=col, ha='center', va='center',
                alpha=0.8)

        # Amplitude bar (radial bar from orbit, length proportional to amplitude)
        bar_angle = np.pi/2 + i * 0.15
        bar_start_x = r * np.cos(bar_angle)
        bar_start_y = r * np.sin(bar_angle)
        bar_len = amp * 0.12
        bar_end_x = (r + bar_len) * np.cos(bar_angle)
        bar_end_y = (r + bar_len) * np.sin(bar_angle)
        ax.plot([bar_start_x, bar_end_x], [bar_start_y, bar_end_y],
                color=col, linewidth=3, alpha=0.6, zorder=5, solid_capstyle='round')

    # Resonance annotations between adjacent orbits
    for i in range(len(freqs) - 1):
        mid_r = (radii[i] + radii[i+1]) / 2
        angle = -np.pi/4 + i * 0.3
        mx, my = mid_r * np.cos(angle), mid_r * np.sin(angle)
        ax.text(mx, my, '2 : 1', fontsize=7, color=GREY, alpha=0.5,
                ha='center', va='center',
                bbox=dict(boxstyle='round,pad=0.15', facecolor=BG_DARK,
                          edgecolor='none', alpha=0.7))

    # Fourier series text
    ax.text(0.98, -0.98,
            'Lemniscate-Alpha harmonics:\n'
            r'$x(t) = \cos t + \frac{1}{2}\cos 2t + \frac{1}{2}\cos 4t'
            r' + \frac{2}{5}\cos 8t + \frac{1}{16}\cos 16t$' + '\n\n'
            'Frequencies: {1, 2, 4, 8, 16}\n'
            'Period-doubling cascade',
            fontsize=8, color=GREY, ha='right', va='bottom',
            bbox=dict(boxstyle='round,pad=0.4', facecolor=BG_DARK,
                      edgecolor=GREY, alpha=0.85, linewidth=0.5))

    ax.set_xlim(-1.25, 1.25)
    ax.set_ylim(-1.25, 1.25)
    ax.set_xticks([])
    ax.set_yticks([])
    for spine in ax.spines.values():
        spine.set_linewidth(0.3)

    # Title
    fig.text(0.5, 0.97, 'ORBITAL HARMONICS', ha='center', va='top',
             fontsize=22, fontweight='bold', color=WHITE)
    fig.text(0.5, 0.94,
             'The period-doubling cascade: orbital resonance meets the third loop',
             ha='center', va='top', fontsize=11, color=GREY, style='italic')

    fig.text(0.5, 0.01,
             'Frequencies {1, 2, 4, 8, 16}: the same doubling that builds '
             'the lemniscate-alpha also governs planetary resonance.',
             ha='center', va='bottom', fontsize=9, color=GREY)

    save_fig(fig, 'domain_3_astronomy.png')


# =============================================================================
# FIGURE 4: NUMBER THEORY — The Master Quadratic
# =============================================================================

def fig4_number_theory():
    setup_rcparams()
    fig, ax = plt.subplots(figsize=(13, 8))

    # Parabola
    x = np.linspace(-8, 152, 2000)
    f = x**2 - 16*G_star**2*x + 16*G_star**3
    x_v = 8 * G_star**2
    f_v = x_v**2 - 16*G_star**2*x_v + 16*G_star**3
    f_norm = f / abs(f_v)

    ax.plot(x, f_norm, color=WHITE, linewidth=2.2, zorder=3)
    ax.axhline(y=0, color=GREY, linewidth=0.8, linestyle='--', alpha=0.4)

    # Root markers
    ax.axvline(x=x_minus, color=RED, linewidth=1.2, linestyle=':', alpha=0.6)
    ax.plot(x_minus, 0, 'o', color=RED, markersize=10, zorder=5)
    ax.text(x_minus + 2, 0.12, r'$x_- = 3.024$', fontsize=12, color=RED,
            fontweight='bold', ha='left')
    ax.text(x_minus + 2, -0.05, r'$N_c = 3$ (color charges)', fontsize=9,
            color=RED, alpha=0.8, ha='left')

    ax.axvline(x=x_plus, color=BLUE, linewidth=1.2, linestyle=':', alpha=0.6)
    ax.plot(x_plus, 0, 'o', color=BLUE, markersize=10, zorder=5)
    ax.text(x_plus - 2, 0.12, r'$x_+ = 137.036$', fontsize=12, color=BLUE,
            fontweight='bold', ha='right')
    ax.text(x_plus - 2, -0.05, r'$1/\alpha$ (fine structure)', fontsize=9,
            color=BLUE, alpha=0.8, ha='right')

    # Main equation box
    ax.text(70, 0.82,
            r'$x^2 - 16\,G^{*2}\,x + 16\,G^{*3} = 0$',
            fontsize=15, color=WHITE, ha='center', va='center', fontweight='bold',
            bbox=dict(boxstyle='round,pad=0.4', facecolor=BG_DARK,
                      edgecolor=GREY, alpha=0.95))
    ax.text(70, 0.60,
            r'$G^{\!*} = \sqrt{2}\,\Gamma(1\!/\!4)^2\,/\,(2\pi) = 2.9587\ldots$',
            fontsize=10, color=GREY, ha='center')

    # CM Theory block (left, in the dip)
    ax.text(22, -0.55,
            'CM Theory\n'
            r'$y^2 = x^3 - x$' + '\n'
            r'$j = 1728 = 12^3$' + '\n'
            r'End. ring: $\mathbb{Z}[i]$',
            fontsize=9, color=GOLD, ha='center', va='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor=BG_DARK,
                      edgecolor=GOLD, alpha=0.8, linewidth=0.8))

    # Bridge block (center)
    ax.text(58, -0.82,
            'The Bridge\n'
            r'$\bigstar = 2/\!\sqrt{\pi} = 1.1284\ldots$' + '\n'
            r'$\bigstar^{-2} = \pi/4 = L(\chi_{-4}, 1)$' + '\n'
            r'Detects $\mathbb{Z}[i]$',
            fontsize=9, color=GREEN, ha='center', va='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor=BG_DARK,
                      edgecolor=GREEN, alpha=0.8, linewidth=0.8))

    # Vieta block (right)
    product_val = 16 * G_star**3
    sum_val = 16 * G_star**2
    ax.text(100, -0.55,
            "Vieta's formulas\n"
            r'$x_+  x_- = 16\,G^{*3}$' + f' = {product_val:.1f}\n'
            r'$x_+ + x_- = 16\,G^{*2}$' + f' = {sum_val:.1f}\n'
            r'Product / Sum $= G^{\!*}$',
            fontsize=9, color=WHITE, ha='center', va='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor=BG_DARK,
                      edgecolor=GREY, alpha=0.8, linewidth=0.8))

    ax.set_xlabel(r'$x$', fontsize=13)
    ax.set_ylabel(r'$f(x)\,/\,|f_{\min}|$', fontsize=12)
    ax.set_xlim(-12, 155)
    ax.set_ylim(-1.15, 1.05)

    fig.text(0.5, 0.97, 'THE MASTER QUADRATIC', ha='center', va='top',
             fontsize=22, fontweight='bold', color=WHITE)
    fig.text(0.5, 0.935, 'From elliptic curve theory to the fine structure constant',
             ha='center', va='top', fontsize=12, color=GREY, style='italic')

    fig.text(0.5, 0.01,
             'One equation, two roots: the coupling constant of electromagnetism '
             'and the number of quark colors. Both from G*.',
             ha='center', va='bottom', fontsize=9, color=GREY)

    save_fig(fig, 'domain_4_number_theory.png')


# =============================================================================
# FIGURE 5: CHEMISTRY — The Coupling Constant
# =============================================================================

def fig5_chemistry():
    setup_rcparams()
    fig = plt.figure(figsize=(13, 8))

    # --- Left panel: Energy levels ---
    ax_e = fig.add_axes([0.06, 0.10, 0.48, 0.78])
    ax_e.set_facecolor(BG_PANEL)

    levels = [1, 2, 3, 4, 5]
    E = {n: -13.6 / n**2 for n in levels}
    level_colors = {1: BLUE, 2: '#7db8f0', 3: GOLD, 4: '#e8a040', 5: RED}

    for n in levels:
        ax_e.plot([0.1, 0.85], [E[n], E[n]], color=level_colors[n],
                  linewidth=2.5, zorder=3)
        ax_e.text(0.88, E[n], f'n = {n}\n{E[n]:.2f} eV',
                  fontsize=8, color=level_colors[n], va='center')

    # Ionization threshold
    ax_e.plot([0.1, 0.85], [0, 0], color=GREY, linewidth=1.0,
              linestyle='--', alpha=0.5)
    ax_e.text(0.88, 0.1, r'$n = \infty$ (ionized)', fontsize=8,
              color=GREY, va='center')

    # Fine structure splitting at n=2
    E2_split = alpha_val**2 * 13.6 / 4  # ~0.000362 eV
    # Exaggerate for visibility
    split_visual = 0.35
    ax_e.plot([0.35, 0.65], [E[2] + split_visual/2, E[2] + split_visual/2],
              color='#7db8f0', linewidth=1.0, linestyle='--', alpha=0.7)
    ax_e.plot([0.35, 0.65], [E[2] - split_visual/2, E[2] - split_visual/2],
              color='#7db8f0', linewidth=1.0, linestyle='--', alpha=0.7)
    # Bracket
    ax_e.annotate('', xy=(0.33, E[2] + split_visual/2),
                  xytext=(0.33, E[2] - split_visual/2),
                  arrowprops=dict(arrowstyle='<->', color=GREEN, lw=1.5))
    ax_e.text(0.22, E[2], r'$\alpha^2$' + '\nfine\nstructure',
              fontsize=7.5, color=GREEN, ha='center', va='center')

    # Transition arrows
    transitions = [
        (2, 1, 'Lyman-' + r'$\alpha$' + '\n121.6 nm', PURPLE),
        (3, 2, 'Balmer-' + r'$\alpha$' + '\n656.3 nm', RED),
        (4, 3, 'Paschen\n1875 nm', '#cc4444'),
    ]
    x_arrow = [0.18, 0.50, 0.75]
    for (n_up, n_lo, lbl, col), xa in zip(transitions, x_arrow):
        ax_e.annotate('', xy=(xa, E[n_lo] + 0.2),
                      xytext=(xa, E[n_up] - 0.1),
                      arrowprops=dict(arrowstyle='->', color=col, lw=1.8,
                                     connectionstyle='arc3,rad=0.0'))
        mid_E = (E[n_up] + E[n_lo]) / 2
        ax_e.text(xa + 0.03, mid_E, lbl, fontsize=7, color=col, va='center')

    # Key formula
    ax_e.text(0.5, -14.5,
              r'$E_n = -\,m_e c^2 \,\alpha^2\, /\, (2n^2)$',
              fontsize=12, color=WHITE, ha='center', va='top',
              bbox=dict(boxstyle='round,pad=0.3', facecolor=BG_DARK,
                        edgecolor=GREY, alpha=0.9))

    ax_e.set_xlim(0, 1.1)
    ax_e.set_ylim(-15.5, 1.5)
    ax_e.set_xticks([])
    ax_e.set_ylabel('Energy (eV)', fontsize=11)
    ax_e.set_title('Hydrogen Energy Levels', fontsize=12, color=WHITE, pad=8)

    # --- Right panel: Bohr orbits ---
    ax_o = fig.add_axes([0.58, 0.10, 0.38, 0.78])
    ax_o.set_facecolor(BG_DARK)
    ax_o.set_aspect('equal')

    t_circ = np.linspace(0, 2*np.pi, 300)
    orbit_colors = {1: BLUE, 2: '#7db8f0', 3: GOLD}

    # Nucleus glow
    for s, a in [(20, 0.05), (12, 0.12), (6, 0.3)]:
        ax_o.plot(0, 0, 'o', color=RED, markersize=s, alpha=a, zorder=10)
    ax_o.plot(0, 0, 'o', color=WHITE, markersize=3, zorder=11)
    ax_o.text(0.0, -0.06, 'p', fontsize=8, color=WHITE, ha='center', va='top')

    for n in [1, 2, 3]:
        r = n**2 / 9.0 * 0.85  # scale to fit
        ax_o.plot(r * np.cos(t_circ), r * np.sin(t_circ),
                  color=orbit_colors[n], linewidth=1.2, alpha=0.5)
        # Electron dot
        e_angle = np.pi/3 + n * 1.2
        ex, ey = r * np.cos(e_angle), r * np.sin(e_angle)
        ax_o.plot(ex, ey, 'o', color=orbit_colors[n], markersize=6, zorder=8)
        ax_o.text(r + 0.06, 0, f'n={n}', fontsize=8, color=orbit_colors[n],
                  va='center')

    # Bohr radius annotation
    ax_o.text(0.0, -0.92,
              r'$a_0 = \hbar\,/\,(m_e c \alpha)$' + '\n= 0.529 ' + r'$\AA$',
              fontsize=9, color=WHITE, ha='center', va='top',
              bbox=dict(boxstyle='round,pad=0.3', facecolor=BG_DARK,
                        edgecolor=GREY, alpha=0.9))

    ax_o.set_xlim(-1.05, 1.05)
    ax_o.set_ylim(-1.05, 1.05)
    ax_o.set_xticks([])
    ax_o.set_yticks([])
    for spine in ax_o.spines.values():
        spine.set_linewidth(0.3)
    ax_o.set_title('Bohr Orbits', fontsize=12, color=WHITE, pad=8)

    # Alpha source box (top right area)
    fig.text(0.77, 0.92,
             r'$\alpha = 1/137.036$' + '\n'
             r'from $x^2 - 16G^{*2}x + 16G^{*3} = 0$' + '\n'
             r'$G^{\!*} = \varpi \cdot \bigstar$',
             fontsize=9, color=GOLD, ha='center', va='top',
             bbox=dict(boxstyle='round,pad=0.4', facecolor=BG_DARK,
                       edgecolor=GOLD, alpha=0.9, linewidth=0.8))

    # Title
    fig.text(0.35, 0.97, 'THE COUPLING CONSTANT', ha='center', va='top',
             fontsize=20, fontweight='bold', color=WHITE)
    fig.text(0.35, 0.935,
             r'$\alpha = 1/137.036$ determines every atomic property',
             ha='center', va='top', fontsize=11, color=GREY, style='italic')

    fig.text(0.5, 0.01,
             'From the lemniscate to the Bohr atom: '
             r'$\alpha$ sets energy levels, orbital radii, and fine structure.',
             ha='center', va='bottom', fontsize=9, color=GREY)

    save_fig(fig, 'domain_5_chemistry.png')


# =============================================================================
# FIGURE 6: PHILOSOPHY — The Ontological Hierarchy
# =============================================================================

def fig6_philosophy():
    setup_rcparams()
    fig, ax = plt.subplots(figsize=(10, 13))
    ax.set_facecolor(BG_DARK)
    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-0.5, 10.5)
    ax.set_xticks([])
    ax.set_yticks([])
    for spine in ax.spines.values():
        spine.set_visible(False)

    # Node definitions: (y_pos, label, sublabel, border_color)
    nodes = [
        (9.5, 'Void', 'Pure potentiality', GREY),
        (8.3, '{0, 1}', 'First distinction', WHITE),
        (7.1, 'Self-Reference', r'$f(f(t)) = t^4$', GOLD),
        (5.9, r'$\varpi = 2.622\ldots$', 'The self-contained', GOLD),
        (4.7, r'$\pi = 3.142\ldots$', 'The relational', BLUE),
        (3.5, r'$\bigstar = 2/\!\sqrt{\pi}$', 'The bridge', GREEN),
        (2.3, r'$G^{\!*} = 2.959\ldots$', r'$\varpi \times \bigstar$', RED),
        (1.1, r'$\alpha = 1/137.036$', 'Master quadratic root', RED),
        (0.0, 'PHYSICS', 'All forces, all particles', WHITE),
    ]

    # Zone backgrounds
    zone_specs = [
        (5.4, 10.0, GOLD, 'BEING', r'$(\varpi)$'),
        (3.0, 5.4, BLUE, 'KNOWING', r'$(\pi)$'),
        (-.3, 3.0, RED, 'UNDERSTANDING', r'$(G^{\!*})$'),
    ]
    for y_lo, y_hi, col, label, sym in zone_specs:
        rect = mpatches.FancyBboxPatch(
            (-1.3, y_lo), 2.6, y_hi - y_lo,
            boxstyle='round,pad=0.1', facecolor=col, alpha=0.04,
            edgecolor=col, linewidth=0.5, linestyle='--',
            zorder=0)
        ax.add_patch(rect)
        ax.text(-1.2, (y_lo + y_hi)/2, f'{label}\n{sym}',
                fontsize=8, color=col, alpha=0.5, va='center', ha='left',
                rotation=90)

    # Draw nodes
    node_h = 0.35
    node_w = 0.65
    for y, label, sub, col in nodes:
        rect = mpatches.FancyBboxPatch(
            (-node_w, y - node_h), 2*node_w, 2*node_h,
            boxstyle='round,pad=0.08', facecolor=BG_DARK,
            edgecolor=col, linewidth=1.8, zorder=3)
        ax.add_patch(rect)
        ax.text(0, y + 0.05, label, fontsize=12, color=col, ha='center',
                va='center', fontweight='bold', zorder=4)
        ax.text(0, y - 0.18, sub, fontsize=8, color=GREY, ha='center',
                va='center', zorder=4)

    # Arrows between nodes
    arrow_labels = [
        'distinction', 'self-application', r'$n=4$ selected',
        r'$\Gamma(1\!/\!4)^4/(8\varpi^2)$', r'$2/\!\sqrt{\pi}$',
        r'$\varpi \cdot \bigstar$', 'quadratic', 'coupling',
    ]
    for i in range(len(nodes) - 1):
        y_from = nodes[i][0] - node_h - 0.05
        y_to = nodes[i+1][0] + node_h + 0.05
        ax.annotate('', xy=(0, y_to), xytext=(0, y_from),
                    arrowprops=dict(arrowstyle='->', color=GREY,
                                   linewidth=1.2, mutation_scale=12),
                    zorder=2)
        y_mid = (y_from + y_to) / 2
        ax.text(0.75, y_mid, arrow_labels[i], fontsize=7.5, color=GREY,
                ha='left', va='center', alpha=0.7)

    # Right sidebar: tiny curve silhouettes
    t_mini = np.linspace(0, 2*np.pi, 300)
    sidebar_x = 1.15
    curve_scale = 0.2

    # Lemniscate silhouette (aligned with BEING zone)
    th_m = np.linspace(-np.pi/4+0.01, np.pi/4-0.01, 150)
    lmx, lmy = lemniscate_xy(th_m)
    th_m2 = np.linspace(3*np.pi/4+0.01, 5*np.pi/4-0.01, 150)
    lmx2, lmy2 = lemniscate_xy(th_m2)
    lem_cy = 7.7
    ax.plot(sidebar_x + lmx*curve_scale, lem_cy + lmy*curve_scale,
            color=GOLD, linewidth=1.2, alpha=0.6)
    ax.plot(sidebar_x + lmx2*curve_scale, lem_cy + lmy2*curve_scale,
            color=GOLD, linewidth=1.2, alpha=0.6)

    # Circle silhouette (aligned with KNOWING zone)
    circ_cy = 4.2
    cmx, cmy = circle_xy(t_mini)
    ax.plot(sidebar_x + cmx*curve_scale*0.8, circ_cy + cmy*curve_scale*0.8,
            color=BLUE, linewidth=1.2, alpha=0.6)

    # Lemniscate-alpha silhouette (aligned with UNDERSTANDING zone)
    la_cy = 1.4
    lax, lay = lemniscate_alpha_xy(t_mini)
    la_sc = curve_scale * 0.35 / max(np.max(np.abs(lax)), np.max(np.abs(lay)))
    ax.plot(sidebar_x + lax*la_sc, la_cy + lay*la_sc,
            color=RED, linewidth=1.0, alpha=0.6)

    # Title
    fig.text(0.5, 0.98, 'THE ONTOLOGICAL HIERARCHY', ha='center', va='top',
             fontsize=22, fontweight='bold', color=WHITE)
    fig.text(0.5, 0.955,
             'From void to physics through three levels of self-reference',
             ha='center', va='top', fontsize=11, color=GREY, style='italic')

    # Bottom text
    fig.text(0.5, 0.01,
             'Being produces Knowing.  Knowing produces the Bridge.  '
             'The Bridge returns to Being, producing Understanding.',
             ha='center', va='bottom', fontsize=9, color=GREY)

    save_fig(fig, 'domain_6_philosophy.png')


# =============================================================================
# MAIN
# =============================================================================

if __name__ == '__main__':
    print("Generating 6 domain figures...")
    fig1_mathematics()
    fig2_physics()
    fig3_astronomy()
    fig4_number_theory()
    fig5_chemistry()
    fig6_philosophy()
    print("\nDone. 6 figures saved.")
