"""
Resonance, Universality, and the Emergence of Discrete Structure
================================================================
Five publication-quality figures exploring how Arnold tongues,
Feigenbaum cascades, and golden-ratio number theory converge
on the FTD integers {3, 4, 7, 13}.

Generates:
  1) fig_winding_number_portrait.png  — Fractal heatmap of rotation number
  2) fig_tongue_bifurcation.png       — Internal cascade within 1/3 tongue
  3) fig_grid_emergence.png           — Mode-locking → lattice structure
  4) fig_golden_orbit.png             — Golden mean orbit vs convergents
  5) fig_universality_triptych.png    — Grand synthesis: δ + G* + φ → {3,4,7,13}

Reference: conversation on Arnold tongues, Feigenbaum bridge,
           and "the grids come naturally" insight.
"""

from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch
from matplotlib.lines import Line2D
from mpl_toolkits.mplot3d import Axes3D
from scipy.special import gamma
from fractions import Fraction
import colorsys

# Output directory
_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'
_FIGDIR.mkdir(parents=True, exist_ok=True)

# =============================================================================
# Shared constants
# =============================================================================
G_STAR = np.sqrt(2) * (gamma(0.25)**2) / (2 * np.pi)
DELTA_F = 4.669201609102990
VARPI = 2.622057554292119810
ALPHA = 1.0 / 137.035999177
PHI = (1 + np.sqrt(5)) / 2
INV_PHI = 1.0 / PHI

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

# Color palette
NAVY = '#003366'
RED = '#CC0000'
CYAN = '#0099CC'
TEAL = '#006699'
GOLD = '#CC9900'
PURPLE = '#7B2D8E'
GREEN = '#228B22'
INT_COLORS = {3: '#E74C3C', 4: '#F39C12', 7: '#9B59B6', 13: '#3498DB'}


# =============================================================================
# Circle map helpers
# =============================================================================
def circle_map(theta, omega, K):
    """Single iteration of the standard circle map."""
    return theta + omega - (K / (2 * np.pi)) * np.sin(2 * np.pi * theta)


def rotation_number(omega, K, n_iter=400, n_skip=300):
    """Compute the rotation number for the standard circle map."""
    theta = 0.0
    for _ in range(n_skip):
        theta = circle_map(theta, omega, K)
    theta_start = theta
    for _ in range(n_iter):
        theta = circle_map(theta, omega, K)
    return (theta - theta_start) / n_iter


def circle_lyapunov(omega, K, n_iter=1000, n_skip=500):
    """Compute Lyapunov exponent for the circle map."""
    theta = 0.5
    for _ in range(n_skip):
        theta = circle_map(theta, omega, K)
    log_sum = 0.0
    for _ in range(n_iter):
        deriv = abs(1 - K * np.cos(2 * np.pi * theta))
        if deriv > 1e-15:
            log_sum += np.log(deriv)
        theta = circle_map(theta, omega, K)
    return log_sum / n_iter


# =============================================================================
# Figure 1: Winding Number Portrait
# =============================================================================
def gen_winding_number_portrait():
    """Fractal heatmap of rotation number across (Omega, K) plane."""
    plt.rcParams.update(STYLE)

    fig = plt.figure(figsize=(14, 10))
    ax_main = fig.add_axes([0.06, 0.08, 0.68, 0.85])
    ax_stair = fig.add_axes([0.77, 0.08, 0.20, 0.85])

    fig.suptitle('Winding Number Portrait of the Standard Circle Map',
                 fontsize=14, fontweight='bold', y=0.99)

    # --- Main heatmap ---
    print('    Computing winding number grid (this takes ~2-3 min)...')
    n_omega = 800
    n_K = 500
    omega_vals = np.linspace(0, 1, n_omega)
    K_vals = np.linspace(0, 2.0, n_K)

    rho_grid = np.zeros((n_K, n_omega))

    for j, K in enumerate(K_vals):
        if j % 50 == 0:
            print(f'      K = {K:.2f} ({j}/{n_K})')
        for i, omega in enumerate(omega_vals):
            rho_grid[j, i] = rotation_number(omega, K, n_iter=300, n_skip=200)

    # Use HSV colormap (cyclic: rho=0 and rho=1 same color)
    ax_main.imshow(rho_grid, extent=[0, 1, 0, 2.0], origin='lower',
                   aspect='auto', cmap='hsv', vmin=0, vmax=1,
                   interpolation='bilinear')

    # Mark K=1 critical line
    ax_main.axhline(1.0, color='white', ls=':', lw=0.8, alpha=0.6)
    ax_main.text(0.02, 1.03, '$K = 1$', fontsize=8, color='white', alpha=0.8)

    # Mark golden mean
    ax_main.axvline(INV_PHI, color=GOLD, ls='--', lw=1.5, alpha=0.7)
    ax_main.text(INV_PHI + 0.01, 1.9, r'$1/\phi$', fontsize=10, color=GOLD,
                 fontweight='bold')

    # Label key tongues at K=0
    tongue_labels = [
        (0, '0'), (1/4, '1/4'), (1/3, '1/3'), (2/5, '2/5'),
        (1/2, '1/2'), (3/5, '3/5'), (2/3, '2/3'), (3/4, '3/4'), (1.0, '1'),
    ]
    for omega_t, label in tongue_labels:
        ax_main.plot(omega_t, 0, '|', color='white', markersize=6, mew=1.0)

    # FTD-denominator tongue labels
    ftd_tongues = [
        (1/3, '1/3', INT_COLORS[3]),
        (1/4, '1/4', INT_COLORS[4]),
        (3/7, '3/7', INT_COLORS[7]),
        (8/13, '8/13', INT_COLORS[13]),
    ]
    for omega_t, label, color in ftd_tongues:
        ax_main.annotate(label, xy=(omega_t, 0.6), xytext=(omega_t, 0.3),
                         fontsize=9, color=color, fontweight='bold',
                         ha='center',
                         arrowprops=dict(arrowstyle='->', color=color, lw=1.2))

    ax_main.set_xlabel(r'$\Omega$ (bare frequency)', fontsize=11)
    ax_main.set_ylabel('$K$ (coupling strength)', fontsize=11)
    ax_main.set_xlim(0, 1)
    ax_main.set_ylim(0, 2.0)

    # Colorbar
    sm = plt.cm.ScalarMappable(cmap='hsv', norm=plt.Normalize(0, 1))
    sm.set_array([])
    cbar = fig.colorbar(sm, ax=ax_main, fraction=0.02, pad=0.01)
    cbar.set_label(r'Rotation number $\rho$', fontsize=10)

    # --- Right strip: Devil's staircase at K=1 ---
    print('    Computing devil\'s staircase...')
    omega_s = np.linspace(0, 1, 2000)
    rho_s = np.array([rotation_number(o, 1.0, n_iter=500, n_skip=300)
                       for o in omega_s])

    # Color each point by its rho value using hsv
    colors_s = plt.cm.hsv(rho_s)
    ax_stair.scatter(rho_s, omega_s, c=colors_s, s=0.3, marker=',',
                     rasterized=True)
    ax_stair.plot(rho_s, omega_s, color='black', lw=0.3, alpha=0.4)
    ax_stair.set_xlabel(r'$\rho$', fontsize=10)
    ax_stair.set_ylabel(r'$\Omega$', fontsize=10)
    ax_stair.set_title("Devil's Staircase\n($K = 1$)", fontsize=10)
    ax_stair.set_xlim(0, 1)
    ax_stair.set_ylim(0, 1)
    ax_stair.tick_params(labelsize=8)

    # Mark golden mean on staircase
    ax_stair.axhline(INV_PHI, color=GOLD, ls='--', lw=1.0, alpha=0.5)
    ax_stair.axvline(INV_PHI, color=GOLD, ls='--', lw=1.0, alpha=0.5)

    out = _FIGDIR / 'fig_winding_number_portrait.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Figure 2: Internal Tongue Bifurcation
# =============================================================================
def gen_tongue_bifurcation():
    """Period-doubling cascade inside the 1/3 Arnold tongue."""
    plt.rcParams.update(STYLE)
    fig = plt.figure(figsize=(16, 10))

    # Layout: large left panel + 5 smaller panels
    ax_bif = fig.add_axes([0.06, 0.08, 0.42, 0.85])
    ax_p3 = fig.add_axes([0.54, 0.60, 0.20, 0.32])
    ax_p6 = fig.add_axes([0.77, 0.60, 0.20, 0.32])
    ax_chaos = fig.add_axes([0.54, 0.08, 0.20, 0.32])
    ax_lyap = fig.add_axes([0.77, 0.08, 0.20, 0.32])

    fig.suptitle('Period-Doubling Inside the 1/3 Arnold Tongue',
                 fontsize=14, fontweight='bold', y=0.99)

    # --- Panel A: Bifurcation diagram inside the 1/3 tongue ---
    print('    Computing internal bifurcation at Omega=1/3...')
    omega_fixed = 1.0 / 3.0
    K_min, K_max = 0.01, 4.0 * np.pi
    n_K = 3000
    n_skip = 500
    n_plot = 200

    K_vals = np.linspace(K_min, K_max, n_K)
    K_all, theta_all = [], []

    for K in K_vals:
        theta = 0.1
        for _ in range(n_skip):
            theta = circle_map(theta, omega_fixed, K)
        for _ in range(n_plot):
            theta = circle_map(theta, omega_fixed, K)
            K_all.append(K)
            theta_all.append(theta % 1)

    ax_bif.plot(K_all, theta_all, ',', color=NAVY, alpha=0.1, markersize=0.2,
                rasterized=True)
    ax_bif.set_xlabel('$K$ (coupling strength)', fontsize=11)
    ax_bif.set_ylabel(r'$\theta^*$ (mod 1)', fontsize=11)
    ax_bif.set_title(r'(A) Bifurcation at $\Omega = 1/3$', fontsize=11,
                      fontweight='bold')
    ax_bif.set_xlim(K_min, K_max)
    ax_bif.set_ylim(0, 1)
    ax_bif.tick_params(labelsize=9)

    # Mark K=1 (critical)
    ax_bif.axvline(1.0, color=RED, ls=':', lw=0.8, alpha=0.5)
    ax_bif.text(1.05, 0.95, '$K=1$', fontsize=8, color=RED, va='top')

    # --- Phase portraits at specific K values ---
    def plot_orbit_on_circle(ax, omega, K, title, n_show=150):
        """Plot the orbit as points on the unit circle."""
        theta = 0.1
        for _ in range(1000):
            theta = circle_map(theta, omega, K)
        thetas = []
        for _ in range(n_show):
            theta = circle_map(theta, omega, K)
            thetas.append(theta % 1)
        thetas = np.array(thetas)

        # Draw unit circle
        t_circ = np.linspace(0, 2 * np.pi, 200)
        ax.plot(np.cos(t_circ), np.sin(t_circ), color='gray', lw=0.5)
        ax.plot(0, 0, '+', color='gray', markersize=4, mew=0.5)

        # Plot iterates
        angles = 2 * np.pi * thetas
        x_pts = np.cos(angles)
        y_pts = np.sin(angles)

        colors_iter = plt.cm.viridis(np.linspace(0, 1, len(thetas)))
        ax.scatter(x_pts, y_pts, c=colors_iter, s=8, zorder=3, alpha=0.8)

        # Connect consecutive points
        for i in range(len(thetas) - 1):
            ax.plot([x_pts[i], x_pts[i+1]], [y_pts[i], y_pts[i+1]],
                    color='gray', lw=0.2, alpha=0.3)

        ax.set_aspect('equal')
        ax.set_xlim(-1.3, 1.3)
        ax.set_ylim(-1.3, 1.3)
        ax.set_title(title, fontsize=9, fontweight='bold')
        ax.tick_params(labelsize=6)

    # Period-3 orbit (K just above 0, well inside tongue)
    plot_orbit_on_circle(ax_p3, omega_fixed, 0.8,
                         '(B) $K=0.8$: Period-3')

    # Period-6 (K past first doubling)
    plot_orbit_on_circle(ax_p6, omega_fixed, 3.5,
                         '(C) $K=3.5$: Period-6')

    # Chaotic (K large)
    plot_orbit_on_circle(ax_chaos, omega_fixed, 10.0,
                         '(D) $K=10$: Chaos', n_show=500)

    # --- Panel E: Lyapunov exponent ---
    print('    Computing Lyapunov exponent...')
    n_lyap = 500
    K_lyap = np.linspace(0.01, K_max, n_lyap)
    lyap = np.array([circle_lyapunov(omega_fixed, K) for K in K_lyap])

    ax_lyap.fill_between(K_lyap, 0, lyap, where=(lyap > 0),
                          color=RED, alpha=0.3)
    ax_lyap.fill_between(K_lyap, 0, lyap, where=(lyap <= 0),
                          color=NAVY, alpha=0.3)
    ax_lyap.plot(K_lyap, lyap, color='black', lw=0.4)
    ax_lyap.axhline(0, color='gray', lw=0.5)
    ax_lyap.set_xlabel('$K$', fontsize=10)
    ax_lyap.set_ylabel(r'$\lambda$', fontsize=10)
    ax_lyap.set_title(r'(E) Lyapunov exponent', fontsize=9, fontweight='bold')
    ax_lyap.tick_params(labelsize=7)
    ax_lyap.set_xlim(K_min, K_max)

    out = _FIGDIR / 'fig_tongue_bifurcation.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Figure 3: Grid Emergence from Mode-Locking
# =============================================================================
def gen_grid_emergence():
    """1D mode-locking → 2D grid → 3D lattice."""
    plt.rcParams.update(STYLE)
    fig = plt.figure(figsize=(16, 11))

    ax_1d = fig.add_axes([0.04, 0.55, 0.30, 0.38])
    ax_2d = fig.add_axes([0.37, 0.55, 0.30, 0.38])
    ax_3d = fig.add_axes([0.70, 0.52, 0.28, 0.44], projection='3d')
    ax_zoom = fig.add_axes([0.04, 0.06, 0.30, 0.40])
    ax_ftd = fig.add_axes([0.37, 0.06, 0.30, 0.40], projection='3d')
    ax_text = fig.add_axes([0.70, 0.06, 0.28, 0.40])

    fig.suptitle('The Grids Come Naturally: Mode-Locking Produces Lattice Structure',
                 fontsize=14, fontweight='bold', y=0.99)

    # Generate rational fractions up to denominator q_max
    def rationals(q_max):
        fracs = set()
        for q in range(1, q_max + 1):
            for p in range(0, q + 1):
                if Fraction(p, q) == Fraction(p, q):  # reduced
                    fracs.add(Fraction(p, q))
        return sorted(fracs)

    rats_13 = rationals(13)
    rats_21 = rationals(21)

    # --- Panel A: 1D Devil's Staircase as bars ---
    ax_1d.set_title("(A) 1D: Devil's Staircase → Discrete Steps",
                     fontsize=10, fontweight='bold')

    for frac in rats_13:
        q = frac.denominator
        if q > 13:
            continue
        val = float(frac)
        width = max(0.005, 0.08 / q)
        color = INT_COLORS.get(q, '#888888')
        alpha = 0.8 if q in INT_COLORS else 0.3
        ax_1d.barh(val, width, height=0.004, left=val - width/2,
                   color=color, alpha=alpha)
        ax_1d.plot(val, val, 's', color=color, markersize=max(2, 8 - q*0.4),
                   alpha=alpha)

    ax_1d.plot([0, 1], [0, 1], '--', color='gray', lw=0.5, alpha=0.5)
    ax_1d.set_xlabel(r'$\Omega$ (input frequency)', fontsize=10)
    ax_1d.set_ylabel(r'$\rho$ (locked output)', fontsize=10)
    ax_1d.set_xlim(0, 1)
    ax_1d.set_ylim(0, 1)
    ax_1d.tick_params(labelsize=8)

    ax_1d.text(0.05, 0.92, 'Continuous input\n→ discrete output',
               transform=ax_1d.transAxes, fontsize=9, va='top',
               style='italic',
               bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                         edgecolor='gray', alpha=0.9))

    # --- Panel B: 2D Rational Lattice ---
    ax_2d.set_title('(B) 2D: Two Oscillators → Grid', fontsize=10,
                     fontweight='bold')

    for f1 in rats_13:
        for f2 in rats_13:
            q_max_pair = max(f1.denominator, f2.denominator)
            if q_max_pair > 13:
                continue
            x, y = float(f1), float(f2)
            size = max(1, 30 / q_max_pair)
            color = INT_COLORS.get(q_max_pair, '#AAAAAA')
            alpha = 0.7 if q_max_pair in INT_COLORS else 0.15
            ax_2d.plot(x, y, 's', color=color, markersize=size**(0.5),
                      alpha=alpha)

    ax_2d.set_xlabel(r'$\rho_x$', fontsize=10)
    ax_2d.set_ylabel(r'$\rho_y$', fontsize=10)
    ax_2d.set_xlim(0, 1)
    ax_2d.set_ylim(0, 1)
    ax_2d.set_aspect('equal')
    ax_2d.tick_params(labelsize=8)
    ax_2d.grid(True, alpha=0.1)

    ax_2d.text(0.05, 0.92, 'Locked states form\na natural 2D grid',
               transform=ax_2d.transAxes, fontsize=9, va='top',
               style='italic',
               bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                         edgecolor='gray', alpha=0.9))

    # --- Panel C: 3D Rational Lattice ---
    ax_3d.set_title('(C) 3D: Three Oscillators → Cubic Lattice',
                     fontsize=9, fontweight='bold')

    rats_7 = rationals(7)
    for f1 in rats_7:
        for f2 in rats_7:
            for f3 in rats_7:
                q_max_trip = max(f1.denominator, f2.denominator, f3.denominator)
                if q_max_trip > 7:
                    continue
                x, y, z = float(f1), float(f2), float(f3)
                size = max(2, 20 / q_max_trip)
                color = INT_COLORS.get(q_max_trip, '#AAAAAA')
                alpha = 0.6 if q_max_trip in INT_COLORS else 0.1
                ax_3d.scatter(x, y, z, c=color, s=size, alpha=alpha)

    ax_3d.set_xlabel(r'$\rho_x$', fontsize=8)
    ax_3d.set_ylabel(r'$\rho_y$', fontsize=8)
    ax_3d.set_zlabel(r'$\rho_z$', fontsize=8)
    ax_3d.tick_params(labelsize=6)
    ax_3d.view_init(elev=20, azim=45)

    # --- Panel D: 2D Zoom showing refinement ---
    ax_zoom.set_title('(D) Zoom: Finer Denominators → Denser Grid',
                       fontsize=10, fontweight='bold')

    for f1 in rats_21:
        for f2 in rats_21:
            q_max_pair = max(f1.denominator, f2.denominator)
            if q_max_pair > 21:
                continue
            x, y = float(f1), float(f2)
            if not (0.2 < x < 0.8 and 0.2 < y < 0.8):
                continue
            size = max(0.5, 15 / q_max_pair)
            if q_max_pair in INT_COLORS:
                color = INT_COLORS[q_max_pair]
                alpha = 0.7
            elif q_max_pair <= 7:
                color = NAVY
                alpha = 0.4
            else:
                color = '#CCCCCC'
                alpha = 0.2
            ax_zoom.plot(x, y, 'o', color=color, markersize=size**(0.5),
                        alpha=alpha)

    ax_zoom.set_xlabel(r'$\rho_x$', fontsize=10)
    ax_zoom.set_ylabel(r'$\rho_y$', fontsize=10)
    ax_zoom.set_xlim(0.2, 0.8)
    ax_zoom.set_ylim(0.2, 0.8)
    ax_zoom.set_aspect('equal')
    ax_zoom.tick_params(labelsize=8)
    ax_zoom.grid(True, alpha=0.1)

    ax_zoom.text(0.05, 0.95, 'Increasing $q_{max}$\nrefines the lattice',
                 transform=ax_zoom.transAxes, fontsize=9, va='top',
                 style='italic',
                 bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                           edgecolor='gray', alpha=0.9))

    # --- Panel E: FTD Cubic Lattice for comparison ---
    ax_ftd.set_title('(E) FTD Postulate: Cubic Lattice',
                      fontsize=9, fontweight='bold')

    L = 5
    for x in range(L):
        for y in range(L):
            for z in range(L):
                # Color by state (random ternary for visual effect)
                r = np.random.random()
                if r < 0.1:
                    color = '#DD4444'  # matter
                    alpha, s = 0.8, 30
                elif r < 0.2:
                    color = '#4488DD'  # antimatter
                    alpha, s = 0.8, 30
                else:
                    color = '#888888'  # void
                    alpha, s = 0.08, 5
                ax_ftd.scatter(x, y, z, c=color, s=s, alpha=alpha)

    ax_ftd.set_xlabel('x', fontsize=8)
    ax_ftd.set_ylabel('y', fontsize=8)
    ax_ftd.set_zlabel('z', fontsize=8)
    ax_ftd.tick_params(labelsize=6)
    ax_ftd.view_init(elev=20, azim=45)

    # --- Panel F: Conceptual text ---
    ax_text.set_xlim(0, 10)
    ax_text.set_ylim(0, 10)
    ax_text.axis('off')
    ax_text.set_title('(F) The Argument', fontsize=10, fontweight='bold')

    steps = [
        (8.5, 'Nonlinear coupling', NAVY),
        (7.0, r'$\downarrow$', 'gray'),
        (6.2, 'Mode-locking at\nrational frequencies', TEAL),
        (4.8, r'$\downarrow$', 'gray'),
        (4.0, '2D/3D coupling\n→ rational grid', PURPLE),
        (2.6, r'$\downarrow$', 'gray'),
        (1.6, 'Cubic lattice\n(FTD Postulate 1)', RED),
    ]

    for y, text, color in steps:
        fontsize = 10 if r'$\downarrow$' not in text else 14
        style = 'normal' if r'$\downarrow$' not in text else 'normal'
        ax_text.text(5, y, text, fontsize=fontsize, ha='center', va='center',
                     color=color, fontweight='bold' if fontsize == 10 else 'normal',
                     bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                               edgecolor=color, alpha=0.9) if fontsize == 10 else None)

    ax_text.text(5, 0.3,
                 '"The grids come naturally"',
                 fontsize=11, ha='center', va='center',
                 style='italic', color=GOLD, fontweight='bold')

    out = _FIGDIR / 'fig_grid_emergence.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Figure 4: Golden Orbit
# =============================================================================
def gen_golden_orbit():
    """Golden mean orbit vs Fibonacci convergent locked orbits."""
    plt.rcParams.update(STYLE)
    fig, axes = plt.subplots(2, 3, figsize=(16, 10))
    fig.suptitle(r'The Golden Mean Orbit: $1/\phi$ — Last Frequency to Lock',
                 fontsize=14, fontweight='bold', y=0.99)

    K_sub = 0.8  # subcritical

    # Three orbits: 5/8 (locked), 8/13 (locked), 1/phi (quasiperiodic)
    orbits = [
        (5/8, '5/8', INT_COLORS[13], 'Period-8 (locked)', 100),
        (8/13, '8/13', INT_COLORS[13], 'Period-13 (locked)', 150),
        (INV_PHI, r'1/\phi', GOLD, 'Quasiperiodic (never locks)', 200),
    ]

    for col, (omega_target, label, color, desc, n_show) in enumerate(orbits):
        # Find the omega that gives this rotation number at K=K_sub
        # For rational ones, use center of tongue; for golden, use exact value
        if omega_target == INV_PHI:
            omega = INV_PHI
        else:
            # Search for omega that gives the target rho
            omega = omega_target  # start at target
            # Fine-tune
            for _ in range(20):
                rho = rotation_number(omega, K_sub, n_iter=500, n_skip=300)
                omega += (omega_target - rho) * 0.5

        # Generate orbit
        theta = 0.1
        for _ in range(1000):
            theta = circle_map(theta, omega, K_sub)
        thetas = []
        for _ in range(n_show):
            theta = circle_map(theta, omega, K_sub)
            thetas.append(theta % 1)
        thetas = np.array(thetas)

        # --- Top row: Circle plots ---
        ax = axes[0, col]
        t_circ = np.linspace(0, 2 * np.pi, 200)
        ax.plot(np.cos(t_circ), np.sin(t_circ), color='#DDDDDD', lw=1.0)

        angles = 2 * np.pi * thetas
        x_pts = np.cos(angles)
        y_pts = np.sin(angles)

        # Color by iteration order
        colors_iter = plt.cm.viridis(np.linspace(0, 1, len(thetas)))
        ax.scatter(x_pts, y_pts, c=colors_iter, s=15, zorder=3, alpha=0.8,
                   edgecolors='none')

        # Connect with thin lines
        for i in range(len(thetas) - 1):
            ax.plot([x_pts[i], x_pts[i+1]], [y_pts[i], y_pts[i+1]],
                    color=color, lw=0.3, alpha=0.2)

        ax.set_aspect('equal')
        ax.set_xlim(-1.4, 1.4)
        ax.set_ylim(-1.4, 1.4)
        ax.set_title(f'({"ABC"[col]}) $\\rho = {label}$\n{desc}',
                     fontsize=10, fontweight='bold', color=color)
        ax.tick_params(labelsize=7)
        ax.plot(0, 0, '+', color='gray', markersize=4, mew=0.5)

        # --- Bottom row: Time series ---
        ax_ts = axes[1, col]
        ax_ts.plot(range(n_show), thetas, color=color, lw=0.5, alpha=0.7)
        ax_ts.scatter(range(n_show), thetas, c=colors_iter, s=3, zorder=3)
        ax_ts.set_xlabel('Iteration $n$', fontsize=10)
        ax_ts.set_ylabel(r'$\theta_n$ (mod 1)', fontsize=10)
        ax_ts.set_xlim(0, n_show)
        ax_ts.set_ylim(0, 1)
        ax_ts.tick_params(labelsize=7)

        if col == 0:
            ax_ts.set_title('(D) Time series: repeats every 8', fontsize=9)
        elif col == 1:
            ax_ts.set_title('(E) Repeats every 13', fontsize=9)
        else:
            ax_ts.set_title('(F) Never repeats', fontsize=9,
                             color=GOLD, fontweight='bold')

    # Convergent annotation on the golden orbit panel
    axes[0, 2].text(0.05, 0.05,
                    r'$\frac{F_{n-1}}{F_n} \to \frac{1}{\phi}$:'
                    '\n' + r'$\frac{1}{2}, \frac{2}{3}, \frac{3}{5},'
                    r' \frac{5}{8}, \mathbf{\frac{8}{13}}, \frac{13}{21}...$',
                    transform=axes[0, 2].transAxes, fontsize=9, va='bottom',
                    bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                              edgecolor=GOLD, alpha=0.9))

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    out = _FIGDIR / 'fig_golden_orbit.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Figure 5: Universality Triptych
# =============================================================================
def gen_universality_triptych():
    """Grand synthesis: delta (chaos) + G* (geometry) + phi (number theory)."""
    plt.rcParams.update(STYLE)
    fig = plt.figure(figsize=(16, 14))

    # Three columns + synthesis row
    ax_chaos = fig.add_axes([0.03, 0.38, 0.31, 0.56])
    ax_geom = fig.add_axes([0.36, 0.38, 0.31, 0.56])
    ax_numb = fig.add_axes([0.69, 0.38, 0.29, 0.56])
    ax_synth = fig.add_axes([0.06, 0.03, 0.88, 0.30])

    fig.suptitle('Three Universal Structures Converge on {3, 4, 7, 13}',
                 fontsize=15, fontweight='bold', y=0.99)

    # --- Column A: Feigenbaum δ (Chaos) ---
    ax_chaos.set_title(r'(A) $\delta = 4.669...$ — Chaos Theory',
                        fontsize=11, fontweight='bold', color=RED)

    # Compact bifurcation diagram
    r_vals = np.linspace(2.8, 4.0, 2000)
    r_all, x_all = [], []
    for r in r_vals:
        x = 0.5
        for _ in range(300):
            x = r * x * (1 - x)
        for _ in range(100):
            x = r * x * (1 - x)
            r_all.append(r)
            x_all.append(x)

    ax_chaos.plot(r_all, x_all, ',', color=RED, alpha=0.1, markersize=0.15,
                  rasterized=True)
    ax_chaos.set_xlabel('$r$', fontsize=10)
    ax_chaos.set_ylabel('$x^*$', fontsize=10)
    ax_chaos.set_xlim(2.8, 4.0)
    ax_chaos.set_ylim(0, 1)
    ax_chaos.tick_params(labelsize=8)

    # Integer extraction box
    ax_chaos.text(0.05, 0.05,
                  r'$\lfloor\delta\rfloor = \mathbf{4}$' + '\n'
                  r'$\lfloor\delta + G^*\rfloor = \mathbf{7}$' + '\n'
                  r'$\lfloor\delta \times G^*\rfloor = \mathbf{13}$' + '\n'
                  r'$\mathrm{round}(\delta - G^* + 1) = \mathbf{3}$',
                  transform=ax_chaos.transAxes, fontsize=10, va='bottom',
                  bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                            edgecolor=RED, alpha=0.95))

    # --- Column B: Lemniscatic G* (Geometry) ---
    ax_geom.set_title(r'(B) $G^* = 2.959...$ — Elliptic Geometry',
                       fontsize=11, fontweight='bold', color=TEAL)

    # Draw lemniscate
    theta = np.linspace(0, 2 * np.pi, 5000)
    r2 = np.maximum(np.cos(2 * theta), 0)
    r = np.sqrt(r2)
    x = r * np.cos(theta)
    y = r * np.sin(theta)
    ax_geom.plot(x, y, color=TEAL, lw=2.0)
    ax_geom.plot(0, 0, '+', color='gray', markersize=8, mew=1)
    ax_geom.set_aspect('equal')
    ax_geom.set_xlim(-1.3, 1.3)
    ax_geom.set_ylim(-0.8, 0.8)
    ax_geom.tick_params(labelsize=8)

    # Master quadratic and roots
    ax_geom.text(0.5, 0.02,
                 r'$x^2 - 16G^{*2}x + 16G^{*3} = 0$' + '\n\n'
                 r'$x_+ = 137.036 = 1/\alpha$' + '\n'
                 r'$x_- = 3.024 \approx N_c = \mathbf{3}$' + '\n\n'
                 r'Coefficient $16 = \mathbf{4}^2$',
                 transform=ax_geom.transAxes, fontsize=10, ha='center',
                 va='bottom',
                 bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                           edgecolor=TEAL, alpha=0.95))

    ax_geom.text(0.5, 0.85, r'$\varpi = 2.622...$' + '\n(lemniscate constant)',
                 transform=ax_geom.transAxes, fontsize=9, ha='center',
                 va='top', color=TEAL, style='italic')

    # --- Column C: Golden Ratio φ (Number Theory) ---
    ax_numb.set_title(r'(C) $\phi = 1.618...$ — Number Theory',
                       fontsize=11, fontweight='bold', color=GOLD)

    # Draw Fibonacci spiral
    ax_numb.set_xlim(-1, 14)
    ax_numb.set_ylim(-1, 9)
    ax_numb.set_aspect('equal')
    ax_numb.tick_params(labelsize=8)

    # Draw Fibonacci squares
    fibs = [1, 1, 2, 3, 5, 8, 13]
    # Manual placement of Fibonacci spiral squares
    squares = [
        (0, 0, 1), (1, 0, 1), (0, 1, 2), (2, 0, 3),
        (0, -2, 5), (5, -2, 8), (0, 3, 13),
    ]

    fib_sq_colors = ['#FFCCCC', '#CCCCFF', '#CCFFCC', INT_COLORS[3],
                     '#FFF0CC', '#E0E0E0', INT_COLORS[13]]

    for idx, (sx, sy, size) in enumerate(squares):
        color = fib_sq_colors[idx] if idx < len(fib_sq_colors) else '#EEEEEE'
        alpha = 0.5 if size not in [3, 13] else 0.4
        edgecolor = INT_COLORS.get(size, 'gray')
        lw = 2.0 if size in [3, 4, 7, 13] else 0.5
        rect = plt.Rectangle((sx, sy), size, size, facecolor=color,
                              edgecolor=edgecolor, lw=lw, alpha=alpha)
        ax_numb.add_patch(rect)
        if size >= 2:
            ax_numb.text(sx + size/2, sy + size/2, f'$F = {size}$',
                         fontsize=8 if size < 8 else 10,
                         ha='center', va='center',
                         fontweight='bold' if size in [3, 13] else 'normal',
                         color=edgecolor)

    # Number theory annotations
    ax_numb.text(0.02, 0.02,
                 r'Lucas: $L_2=\mathbf{3},\, L_3=\mathbf{4},\, L_4=\mathbf{7}$'
                 '\n' + r'Fibonacci: $F_4=\mathbf{3},\, F_7=\mathbf{13}$'
                 '\n' + r'CF: $\mathbf{4}/\mathbf{13} = [0;\, \mathbf{3},\, \mathbf{4}]$',
                 transform=ax_numb.transAxes, fontsize=10, va='bottom',
                 bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                           edgecolor=GOLD, alpha=0.95))

    # --- Bottom: Synthesis ---
    ax_synth.set_xlim(0, 20)
    ax_synth.set_ylim(0, 6)
    ax_synth.axis('off')
    ax_synth.set_title('(D) Synthesis: Three Independent Routes → Same Four Integers',
                        fontsize=12, fontweight='bold')

    # Three source boxes
    sources = [
        (3, 4.5, r'$\delta$' + '\nChaos\nTheory', RED),
        (10, 4.5, r'$G^*$' + '\nElliptic\nGeometry', TEAL),
        (17, 4.5, r'$\phi$' + '\nNumber\nTheory', GOLD),
    ]
    for x, y, text, color in sources:
        ax_synth.text(x, y, text, fontsize=10, ha='center', va='center',
                      color=color, fontweight='bold',
                      bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                                edgecolor=color, lw=2, alpha=0.95))

    # Central integers
    center_x, center_y = 10, 1.2
    for i, (val, color) in enumerate([(3, INT_COLORS[3]), (4, INT_COLORS[4]),
                                       (7, INT_COLORS[7]), (13, INT_COLORS[13])]):
        x = center_x - 3 + i * 2
        ax_synth.text(x, center_y, str(val), fontsize=18, ha='center',
                      va='center', color=color, fontweight='bold',
                      bbox=dict(boxstyle='circle,pad=0.4', facecolor='white',
                                edgecolor=color, lw=2.5))

    # Arrows from sources to integers
    for sx in [3, 10, 17]:
        for ix in [center_x - 3, center_x - 1, center_x + 1, center_x + 3]:
            ax_synth.annotate('', xy=(ix, center_y + 0.7),
                              xytext=(sx, 3.5),
                              arrowprops=dict(arrowstyle='->', color='gray',
                                              lw=0.5, alpha=0.3))

    # Cross-connections
    connections = [
        (5.5, 3.0, r'$\delta / G^* \approx \phi$' + '\n(2.5% gap)', '#666666'),
        (14, 3.0, r'$8/\mathbf{13} \to 1/\phi$', '#666666'),
        (10, 2.5, r'$\delta^2 - G^{*2} \approx \mathbf{13}$' + '\n(0.4%)', '#666666'),
    ]
    for x, y, text, color in connections:
        ax_synth.text(x, y, text, fontsize=8, ha='center', va='center',
                      color=color, style='italic')

    out = _FIGDIR / 'fig_universality_triptych.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')


# =============================================================================
# Main
# =============================================================================
if __name__ == '__main__':
    print('='*60)
    print('Generating Universality Figure Set (5 figures)')
    print('='*60)
    print()

    print('[1/5] Winding Number Portrait...')
    gen_winding_number_portrait()
    print()

    print('[2/5] Tongue Bifurcation...')
    gen_tongue_bifurcation()
    print()

    print('[3/5] Grid Emergence...')
    gen_grid_emergence()
    print()

    print('[4/5] Golden Orbit...')
    gen_golden_orbit()
    print()

    print('[5/5] Universality Triptych...')
    gen_universality_triptych()
    print()

    print('='*60)
    print('All 5 figures generated.')
    print('='*60)
