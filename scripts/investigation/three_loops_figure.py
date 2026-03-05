#!/usr/bin/env python3
"""
three_loops_figure.py — The Three Loops (Publication Figure)
=============================================================

A single figure showing:
  Panel A: The three curves (circle, lemniscate, lemniscate-alpha)
  Panel B: The closed triangle (varpi -> pi -> star -> G*)
  Panel C: The kernels K_2 and K_4 (Lorentz factor = circle kernel)
  Panel D: The master quadratic (roots = 1/alpha and N_c)
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyArrowPatch
from math import gamma, sqrt, pi

# =============================================================================
# CONSTANTS
# =============================================================================

G4 = gamma(0.25)                          # Gamma(1/4)
G4sq = G4**2
sqrt2 = sqrt(2)

varpi = G4sq / (2 * sqrt(2 * pi))         # lemniscatic constant
star = 2 / sqrt(pi)                        # bridge operator
G_star = sqrt2 * G4sq / (2 * pi)           # = varpi * star

disc = 256 * G_star**4 - 64 * G_star**3
x_plus = (16 * G_star**2 + sqrt(disc)) / 2   # = 137.036...
x_minus = (16 * G_star**2 - sqrt(disc)) / 2  # = 3.024...

# =============================================================================
# STYLE
# =============================================================================

BG_DARK = '#0d1117'
BG_PANEL = '#161b22'
BLUE = '#58a6ff'
GOLD = '#f0c040'
RED = '#ff6b6b'
WHITE = '#e6edf3'
GREY = '#8b949e'
GREEN = '#3fb950'

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
# CURVE DEFINITIONS
# =============================================================================

def circle_xy(t):
    return np.cos(t), np.sin(t)

def lemniscate_xy(theta):
    """Lemniscate of Bernoulli: r^2 = cos(2*theta), only where cos(2t)>=0."""
    c2 = np.cos(2 * theta)
    mask = c2 > 0
    r = np.where(mask, np.sqrt(np.maximum(c2, 0)), np.nan)
    return r * np.cos(theta), r * np.sin(theta)

def lemniscate_alpha_xy(t):
    """The 5-harmonic Fourier curve with frequencies {1,2,4,8,16}."""
    x = (np.cos(t) + 0.5*np.cos(2*t) + 0.5*np.cos(4*t) +
         0.4*np.cos(8*t) + 0.0625*np.cos(16*t))
    y = (np.sin(t) - 0.5*np.sin(2*t) + 0.5*np.sin(4*t) -
         0.35*np.sin(8*t) + 0.0625*np.sin(16*t))
    return x, y


# =============================================================================
# FIGURE
# =============================================================================

fig = plt.figure(figsize=(14, 11))

# Title
fig.text(0.5, 0.97, 'THE THREE LOOPS', ha='center', va='top',
         fontsize=22, fontweight='bold', color=WHITE,
         fontfamily='sans-serif')
fig.text(0.5, 0.945, 'Three curves. Three constants. Three levels of self-reference.',
         ha='center', va='top', fontsize=11, color=GREY, style='italic')

# Bottom summary bar
summary = (
    r'$\varpi = 2.6221\ldots$      '
    r'$\pi = 3.1416\ldots$      '
    r'$G^{\!*} = 2.9587\ldots$      '
    r'$\bigstar = 2/\!\sqrt{\pi}$      '
    r'$G^{\!*} = \varpi \cdot \bigstar$      '
    r'$\pi = \Gamma(1\!/\!4)^4 / (8\varpi^2)$'
)
fig.text(0.5, 0.015, summary, ha='center', va='bottom',
         fontsize=10, color=GREY)


# =============================================================================
# PANEL A: The Three Curves
# =============================================================================

ax_a = fig.add_axes([0.05, 0.48, 0.42, 0.43])
ax_a.set_facecolor(BG_PANEL)
ax_a.set_aspect('equal')

t = np.linspace(0, 2*np.pi, 2000)

# Circle (unit circle)
cx, cy = circle_xy(t)
ax_a.plot(cx, cy, color=BLUE, linewidth=1.8, alpha=0.85, label='Circle', zorder=2)

# Lemniscate — plot both lobes via polar
theta_lem = np.linspace(-np.pi/4 + 0.001, np.pi/4 - 0.001, 1000)
lx, ly = lemniscate_xy(theta_lem)
ax_a.plot(lx, ly, color=GOLD, linewidth=2.2, alpha=0.9, label='Lemniscate', zorder=3)
# second lobe
theta_lem2 = np.linspace(3*np.pi/4 + 0.001, 5*np.pi/4 - 0.001, 1000)
lx2, ly2 = lemniscate_xy(theta_lem2)
ax_a.plot(lx2, ly2, color=GOLD, linewidth=2.2, alpha=0.9, zorder=3)

# Lemniscate-Alpha — scale to ~0.55 of the circle so it fits inside
la_x, la_y = lemniscate_alpha_xy(t)
la_scale = 0.55 / max(np.max(np.abs(la_x)), np.max(np.abs(la_y)))
la_x_s, la_y_s = la_x * la_scale, la_y * la_scale
ax_a.plot(la_x_s, la_y_s, color=RED, linewidth=1.5, alpha=0.9,
          label='Lemniscate-Alpha', zorder=4)

# Origin dot
ax_a.plot(0, 0, 'o', color=WHITE, markersize=3, zorder=5)

# Labels — position carefully to avoid overlaps
ax_a.text(0.0, 1.15, r'$\pi$', fontsize=15, color=BLUE,
          ha='center', va='bottom', fontweight='bold')
ax_a.text(0.92, 0.12, r'$\varpi$', fontsize=15, color=GOLD,
          ha='left', va='bottom', fontweight='bold')
ax_a.text(0.0, -0.65, r'$G^{\!*}$', fontsize=15, color=RED,
          ha='center', va='top', fontweight='bold')

# Lobe annotations
ax_a.annotate('1 lobe', xy=(0.75, 0.75), fontsize=8, color=BLUE, alpha=0.7,
              ha='center')
ax_a.annotate('2 lobes', xy=(0.55, -0.35), fontsize=8, color=GOLD, alpha=0.7,
              ha='center')
ax_a.annotate('3 lobes', xy=(-0.55, 0.45), fontsize=8, color=RED, alpha=0.7,
              ha='center')

ax_a.set_xlim(-1.35, 1.35)
ax_a.set_ylim(-1.35, 1.35)
ax_a.set_xticks([])
ax_a.set_yticks([])
ax_a.set_title('The Three Curves', fontsize=13, color=WHITE, pad=10,
               fontweight='bold')


# =============================================================================
# PANEL B: The Closed Triangle
# =============================================================================

ax_b = fig.add_axes([0.53, 0.48, 0.42, 0.43])
ax_b.set_facecolor(BG_PANEL)
ax_b.set_aspect('equal')
ax_b.set_xlim(-1.5, 1.5)
ax_b.set_ylim(-1.2, 1.5)
ax_b.set_xticks([])
ax_b.set_yticks([])

# Triangle vertices
vx, vy = 0.0, 1.15       # varpi (top)
px, py = -1.0, -0.55      # pi (bottom-left)
gx, gy = 1.0, -0.55       # G* (bottom-right)

# Node circles
node_r = 0.28
for (nx, ny, label, val, col) in [
    (vx, vy, r'$\varpi$', '2.6221', GOLD),
    (px, py, r'$\pi$', '3.1416', BLUE),
    (gx, gy, r'$G^{\!*}$', '2.9587', RED),
]:
    circle_patch = plt.Circle((nx, ny), node_r, facecolor=BG_DARK,
                               edgecolor=col, linewidth=2.0, zorder=3)
    ax_b.add_patch(circle_patch)
    ax_b.text(nx, ny + 0.05, label, fontsize=14, color=col,
              ha='center', va='center', fontweight='bold', zorder=4)
    ax_b.text(nx, ny - 0.15, val, fontsize=7.5, color=GREY,
              ha='center', va='center', zorder=4)

# Edges with arrows and labels
arrow_kw = dict(arrowstyle='->', color=GREY, linewidth=1.5,
                connectionstyle='arc3,rad=0.08',
                mutation_scale=15)

# varpi -> pi (left edge, going down)
ax_b.annotate('', xy=(px + 0.22, py + 0.18), xytext=(vx - 0.22, vy - 0.18),
              arrowprops=arrow_kw, zorder=2)
ax_b.text(-0.75, 0.45, r'$\frac{\Gamma(1\!/\!4)^4}{8\varpi^2}$',
          fontsize=9, color=WHITE, ha='center', va='center',
          rotation=50,
          bbox=dict(boxstyle='round,pad=0.15', facecolor=BG_DARK,
                    edgecolor='none', alpha=0.8))

# pi -> star (bottom edge, going right)
ax_b.annotate('', xy=(gx - 0.28, gy), xytext=(px + 0.28, py),
              arrowprops=arrow_kw, zorder=2)
ax_b.text(0.0, -0.78, r'$\bigstar = 2/\!\sqrt{\pi}$',
          fontsize=9, color=WHITE, ha='center', va='center',
          bbox=dict(boxstyle='round,pad=0.15', facecolor=BG_DARK,
                    edgecolor='none', alpha=0.8))

# G* -> varpi (right edge, going up) — or rather varpi*star -> G*
ax_b.annotate('', xy=(vx + 0.22, vy - 0.18), xytext=(gx - 0.05, gy + 0.28),
              arrowprops=dict(arrowstyle='->', color=GREY, linewidth=1.5,
                             connectionstyle='arc3,rad=-0.08',
                             mutation_scale=15), zorder=2)
ax_b.text(0.75, 0.45, r'$\varpi \cdot \bigstar$',
          fontsize=9, color=WHITE, ha='center', va='center',
          rotation=-50,
          bbox=dict(boxstyle='round,pad=0.15', facecolor=BG_DARK,
                    edgecolor='none', alpha=0.8))

# Center annotation
ax_b.text(0.0, 0.05, 'The triangle\ncloses', fontsize=8, color=GREY,
          ha='center', va='center', style='italic', alpha=0.6)

ax_b.set_title('The Closed Triangle', fontsize=13, color=WHITE, pad=10,
               fontweight='bold')


# =============================================================================
# PANEL C: The Kernels
# =============================================================================

ax_c = fig.add_axes([0.05, 0.06, 0.42, 0.36])
ax_c.set_facecolor(BG_PANEL)

t_k = np.linspace(0, 0.94, 500)
K2 = 1.0 / np.sqrt(1 - t_k**2)
K4 = 1.0 / np.sqrt(1 - t_k**4)

ax_c.fill_between(t_k, K4, K2, alpha=0.15, color=GREEN,
                   label=r'$1/\sqrt{1+t^2}$  (self-reference)')
ax_c.plot(t_k, K2, color=BLUE, linewidth=2.2, label=r'$K_2(t) = 1/\sqrt{1-t^2}$',
          zorder=3)
ax_c.plot(t_k, K4, color=GOLD, linewidth=2.2, label=r'$K_4(t) = 1/\sqrt{1-t^4}$',
          zorder=3)

# Annotations
ax_c.annotate(r'$\gamma_{\mathrm{Lorentz}} = K_2(v/c)$',
              xy=(0.75, 1/sqrt(1-0.75**2)),
              xytext=(0.35, 2.8),
              fontsize=9, color=BLUE,
              arrowprops=dict(arrowstyle='->', color=BLUE, lw=1.2),
              ha='center')

ax_c.annotate('what self-reference\nadds',
              xy=(0.7, (K2[int(0.7/0.94*499)] + K4[int(0.7/0.94*499)])/2),
              xytext=(0.25, 1.6),
              fontsize=8, color=GREEN, alpha=0.8,
              arrowprops=dict(arrowstyle='->', color=GREEN, lw=1.0, alpha=0.6),
              ha='center')

ax_c.set_xlabel(r'$t = v/c$', fontsize=11)
ax_c.set_ylabel('Kernel value', fontsize=11)
ax_c.set_xlim(0, 0.98)
ax_c.set_ylim(0.9, 3.5)
ax_c.legend(loc='upper left', fontsize=8.5, facecolor=BG_DARK,
            edgecolor=GREY, labelcolor=WHITE, framealpha=0.9)
ax_c.set_title('The Kernels: Circle vs. Lemniscate', fontsize=13,
               color=WHITE, pad=10, fontweight='bold')


# =============================================================================
# PANEL D: The Master Quadratic
# =============================================================================

ax_d = fig.add_axes([0.53, 0.06, 0.42, 0.36])
ax_d.set_facecolor(BG_PANEL)

# f(x) = x^2 - 16*G*^2*x + 16*G*^3
# This has a huge range. Let's plot two insets or use a log-like approach.
# Better: show the function near EACH root separately using a split axis approach.
# Simplest: plot the normalized version near each root.

# Split the panel into two sub-regions
# Left sub: near x_minus = 3.024 (N_c)
# Right sub: near x_plus = 137.036 (1/alpha)

# Actually, let's use a clever transformation. Plot g(x) = f(x) / f_max
# to show both roots on one axis. The parabola dips to negative between roots.

# The vertex is at x_v = 16*G*^2/2 = 8*G*^2
x_v = 8 * G_star**2
f_v = x_v**2 - 16*G_star**2*x_v + 16*G_star**3  # minimum value (negative)

# Plot range: slightly beyond both roots
x_range = np.linspace(-5, 155, 2000)
f_x = x_range**2 - 16*G_star**2*x_range + 16*G_star**3

# Normalize for display
f_max = max(abs(f_v), abs(f_x[0]), abs(f_x[-1]))
f_norm = f_x / abs(f_v)

ax_d.plot(x_range, f_norm, color=WHITE, linewidth=1.8, zorder=3)
ax_d.axhline(y=0, color=GREY, linewidth=0.8, linestyle='--', alpha=0.5)

# Mark roots
for xr, label, col, ha_pos, y_off in [
    (x_minus, r'$N_c = 3.024$', RED, 'left', 0.3),
    (x_plus, r'$1/\alpha = 137.036$', BLUE, 'right', 0.3),
]:
    ax_d.axvline(x=xr, color=col, linewidth=1.2, linestyle=':', alpha=0.7)
    ax_d.plot(xr, 0, 'o', color=col, markersize=8, zorder=5)
    ax_d.text(xr + (3 if ha_pos == 'left' else -3), y_off,
              label, fontsize=10, color=col,
              ha=ha_pos, va='bottom', fontweight='bold')

# Equation
ax_d.text(70, 0.7, r'$x^2 - 16\,G^{*2}\,x + 16\,G^{*3} = 0$',
          fontsize=12, color=WHITE, ha='center', va='center',
          bbox=dict(boxstyle='round,pad=0.3', facecolor=BG_DARK,
                    edgecolor=GREY, alpha=0.9))

# Sub-annotation
ax_d.text(70, 0.45, r'$G^{\!*} = \sqrt{2}\,\Gamma(1\!/\!4)^2 / (2\pi) = 2.9587\ldots$',
          fontsize=8, color=GREY, ha='center', va='center')

ax_d.set_xlabel(r'$x$', fontsize=11)
ax_d.set_ylabel(r'$f(x) / |f_{\min}|$', fontsize=11)
ax_d.set_xlim(-10, 155)
ax_d.set_ylim(-1.2, 1.1)
ax_d.set_title('The Master Quadratic', fontsize=13, color=WHITE, pad=10,
               fontweight='bold')


# =============================================================================
# SAVE
# =============================================================================

out_path = 'scripts/investigation/three_loops_figure.png'
fig.savefig(out_path, dpi=300, bbox_inches='tight',
            facecolor=fig.get_facecolor(), edgecolor='none')
print(f"Saved: {out_path}")
plt.close(fig)
