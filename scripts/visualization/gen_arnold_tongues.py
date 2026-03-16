"""
Arnold Tongues and the FTD Integer Connection
==============================================
Visualizes:
  1) The Arnold tongue diagram (mode-locking regions in parameter space)
  2) The devil's staircase (rotation number as a function of Omega)
  3) The Farey tree / Stern-Brocot structure
  4) How {3, 4, 7, 13} appear in the tongue hierarchy
  5) The golden ratio as the "most irrational" — last to lock

Generates: fig_arnold_tongues.png
"""

from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.collections import PatchCollection
from scipy.special import gamma
from fractions import Fraction
import colorsys

# Output directory
_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'
_FIGDIR.mkdir(parents=True, exist_ok=True)

# Constants
G_STAR = np.sqrt(2) * (gamma(0.25)**2) / (2 * np.pi)
DELTA_F = 4.669201609102990
PHI = (1 + np.sqrt(5)) / 2
INV_PHI = 1.0 / PHI   # ≈ 0.6180339887

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
INT_COLORS = {3: '#E74C3C', 4: '#F39C12', 7: '#9B59B6', 13: '#3498DB'}


def rotation_number(omega, K, n_iter=300, n_skip=200):
    """Compute the rotation number for the standard circle map."""
    theta = 0.0
    # Skip transients
    for _ in range(n_skip):
        theta = theta + omega - (K / (2 * np.pi)) * np.sin(2 * np.pi * theta)
    # Measure winding
    theta_start = theta
    for _ in range(n_iter):
        theta = theta + omega - (K / (2 * np.pi)) * np.sin(2 * np.pi * theta)
    return (theta - theta_start) / n_iter


def compute_tongue_boundary(p, q, K_max=1.2, n_K=200):
    """Compute the boundary of the p/q Arnold tongue numerically.

    For each K, find the range of Omega values where rho = p/q.
    """
    omega_target = p / q
    K_vals = np.linspace(0.001, K_max, n_K)
    omega_left = np.zeros(n_K)
    omega_right = np.zeros(n_K)

    for i, K in enumerate(K_vals):
        # Search left boundary (binary search)
        lo, hi = max(0, omega_target - 0.3), omega_target
        for _ in range(40):
            mid = (lo + hi) / 2
            rho = rotation_number(mid, K, n_iter=500, n_skip=300)
            rho_frac = round(rho * q) / q
            if abs(rho_frac - omega_target) < 1e-6:
                hi = mid
            else:
                lo = mid
        omega_left[i] = hi

        # Search right boundary
        lo, hi = omega_target, min(1, omega_target + 0.3)
        for _ in range(40):
            mid = (lo + hi) / 2
            rho = rotation_number(mid, K, n_iter=500, n_skip=300)
            rho_frac = round(rho * q) / q
            if abs(rho_frac - omega_target) < 1e-6:
                lo = mid
            else:
                hi = mid
        omega_right[i] = lo

    return K_vals, omega_left, omega_right


def compute_devils_staircase(K, n_omega=2000, n_iter=500, n_skip=300):
    """Compute the devil's staircase: rho(Omega) at fixed K."""
    omega_vals = np.linspace(0, 1, n_omega)
    rho_vals = np.zeros(n_omega)
    for i, omega in enumerate(omega_vals):
        rho_vals[i] = rotation_number(omega, K, n_iter=n_iter, n_skip=n_skip)
    return omega_vals, rho_vals


def tongue_color(q, alpha=0.6):
    """Color based on denominator q, with FTD integers highlighted."""
    if q in INT_COLORS:
        return INT_COLORS[q]
    # Hue based on q
    hue = (q * 0.137) % 1.0  # golden angle distribution
    r, g, b = colorsys.hls_to_rgb(hue, 0.5, 0.6)
    return (r, g, b, alpha)


def gen_arnold_tongues():
    """Multi-panel Arnold tongue exploration with FTD connections."""
    plt.rcParams.update(STYLE)

    fig = plt.figure(figsize=(16, 14))
    fig.suptitle('Arnold Tongues, Mode-Locking, and the FTD Integers {3, 4, 7, 13}',
                 fontsize=14, fontweight='bold', y=0.995)

    # Layout:
    # Row 1: Arnold tongue diagram (big) | Devil's staircase stack
    # Row 2: Farey tree | Golden ratio detail | FTD connection diagram
    ax_tongue = fig.add_axes([0.06, 0.52, 0.52, 0.43])
    ax_stair1 = fig.add_axes([0.64, 0.74, 0.33, 0.20])
    ax_stair2 = fig.add_axes([0.64, 0.52, 0.33, 0.20])
    ax_farey  = fig.add_axes([0.06, 0.04, 0.30, 0.42])
    ax_golden = fig.add_axes([0.39, 0.04, 0.28, 0.42])
    ax_conn   = fig.add_axes([0.70, 0.04, 0.27, 0.42])

    # =========================================================================
    # Panel A: Arnold Tongue Diagram
    # =========================================================================
    print('  Computing Arnold tongue boundaries...')

    # Compute tongues for key rationals
    # Focus on the ones relevant to FTD: denominators 1,2,3,4,5,7,8,13
    tongues_to_plot = []

    # Major tongues (small denominators)
    fractions_list = [
        (0, 1), (1, 1),  # 0/1, 1/1
        (1, 2),           # 1/2
        (1, 3), (2, 3),   # 1/3, 2/3
        (1, 4), (3, 4),   # 1/4, 3/4
        (1, 5), (2, 5), (3, 5), (4, 5),  # fifths
        (1, 7), (2, 7), (3, 7), (4, 7), (5, 7), (6, 7),  # sevenths (b_3!)
        (1, 8), (3, 8), (5, 8), (7, 8),  # eighths
        (5, 13), (8, 13),  # thirteenths (n_eff!)
    ]

    K_MAX = 1.15

    for p, q in fractions_list:
        if p >= q or p < 0:
            continue
        print(f'    Tongue {p}/{q}...')
        K_vals, ol, or_ = compute_tongue_boundary(p, q, K_max=K_MAX, n_K=80)

        # Build polygon
        poly_omega = np.concatenate([ol, or_[::-1], [ol[0]]])
        poly_K = np.concatenate([K_vals, K_vals[::-1], [K_vals[0]]])
        tongues_to_plot.append((p, q, poly_omega, poly_K))

    # Plot tongues
    for p, q, poly_o, poly_k in tongues_to_plot:
        color = tongue_color(q)
        lw = 1.5 if q in [3, 4, 7, 13] else 0.5
        alpha_fill = 0.5 if q in [3, 4, 7, 13] else 0.25
        ax_tongue.fill(poly_o, poly_k, color=color, alpha=alpha_fill)
        ax_tongue.plot(poly_o, poly_k, color=color, lw=lw, alpha=0.8)

        # Label the larger tongues
        if q <= 5 or q in [7, 13]:
            label_y = min(K_MAX * 0.85, 0.15 + 0.06 * q)
            ax_tongue.text(p/q, label_y, f'{p}/{q}', fontsize=7 if q > 5 else 8,
                          ha='center', va='bottom', color=color, fontweight='bold')

    # Mark the golden mean (most irrational)
    ax_tongue.axvline(INV_PHI, color=GOLD, ls='--', lw=1.5, alpha=0.7, zorder=5)
    ax_tongue.text(INV_PHI + 0.01, K_MAX * 0.95,
                   r'$1/\phi$ = ' + f'{INV_PHI:.4f}\n(last to lock)',
                   fontsize=9, color=GOLD, va='top', fontweight='bold')

    # Mark K=1 critical line
    ax_tongue.axhline(1.0, color='gray', ls=':', lw=1.0, alpha=0.5)
    ax_tongue.text(0.02, 1.02, '$K = 1$ (critical)', fontsize=8, color='gray')

    ax_tongue.set_xlabel(r'$\Omega$ (bare frequency ratio)', fontsize=11)
    ax_tongue.set_ylabel('$K$ (coupling strength)', fontsize=11)
    ax_tongue.set_title('(A) Arnold Tongues: Mode-Locking Regions',
                         fontsize=11, fontweight='bold')
    ax_tongue.set_xlim(0, 1)
    ax_tongue.set_ylim(0, K_MAX)
    ax_tongue.tick_params(labelsize=9)

    # Legend for FTD integers
    for q, color in INT_COLORS.items():
        ax_tongue.plot([], [], 's', color=color, markersize=8,
                       label=f'q = {q}')
    ax_tongue.legend(fontsize=8, loc='upper right', title='FTD denominators',
                     title_fontsize=8, framealpha=0.9)

    # =========================================================================
    # Panel B: Devil's Staircase at K=0.5 (subcritical)
    # =========================================================================
    print('  Computing devil\'s staircase K=0.5...')
    omega_s, rho_s = compute_devils_staircase(0.5, n_omega=3000)
    ax_stair1.plot(omega_s, rho_s, color=NAVY, lw=0.5)
    ax_stair1.plot([0, 1], [0, 1], '--', color='gray', lw=0.5, alpha=0.5)

    # Highlight FTD-denominator steps
    for p, q in [(1,3), (1,4), (2,7), (3,7), (5,13), (8,13)]:
        target = p / q
        mask = np.abs(rho_s - target) < 0.003
        if np.any(mask):
            o_range = omega_s[mask]
            ax_stair1.fill_between(o_range, target - 0.005, target + 0.005,
                                    color=INT_COLORS.get(q, 'gray'), alpha=0.5)

    ax_stair1.axhline(INV_PHI, color=GOLD, ls=':', lw=0.8, alpha=0.5)
    ax_stair1.text(0.02, INV_PHI + 0.02, r'$1/\phi$', fontsize=8, color=GOLD)

    ax_stair1.set_ylabel(r'$\rho$ (rotation number)', fontsize=10)
    ax_stair1.set_title("(B) Devil's Staircase ($K = 0.5$)",
                         fontsize=10, fontweight='bold')
    ax_stair1.set_xlim(0, 1)
    ax_stair1.set_ylim(0, 1)
    ax_stair1.tick_params(labelsize=8)
    ax_stair1.tick_params(labelbottom=False)

    # =========================================================================
    # Panel C: Devil's Staircase at K=1.0 (critical — complete)
    # =========================================================================
    print('  Computing devil\'s staircase K=1.0...')
    omega_c, rho_c = compute_devils_staircase(1.0, n_omega=3000)
    ax_stair2.plot(omega_c, rho_c, color=RED, lw=0.5)
    ax_stair2.plot([0, 1], [0, 1], '--', color='gray', lw=0.5, alpha=0.5)

    for p, q in [(1,3), (1,4), (2,7), (3,7), (5,13), (8,13)]:
        target = p / q
        mask = np.abs(rho_c - target) < 0.005
        if np.any(mask):
            o_range = omega_c[mask]
            ax_stair2.fill_between(o_range, target - 0.005, target + 0.005,
                                    color=INT_COLORS.get(q, 'gray'), alpha=0.5)

    ax_stair2.axhline(INV_PHI, color=GOLD, ls=':', lw=0.8, alpha=0.5)
    ax_stair2.text(0.02, INV_PHI + 0.02, r'$1/\phi$', fontsize=8, color=GOLD)

    ax_stair2.set_xlabel(r'$\Omega$', fontsize=10)
    ax_stair2.set_ylabel(r'$\rho$', fontsize=10)
    ax_stair2.set_title("(C) Complete Devil's Staircase ($K = 1$)",
                         fontsize=10, fontweight='bold')
    ax_stair2.set_xlim(0, 1)
    ax_stair2.set_ylim(0, 1)
    ax_stair2.tick_params(labelsize=8)

    ax_stair2.text(0.55, 0.15,
                   'Steps cover full\nmeasure — all\nfrequencies locked',
                   fontsize=8, style='italic', color=RED,
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                             edgecolor=RED, alpha=0.9),
                   transform=ax_stair2.transAxes)

    # =========================================================================
    # Panel D: Farey Tree / Stern-Brocot with FTD integers highlighted
    # =========================================================================
    ax_farey.set_xlim(-0.05, 1.05)
    ax_farey.set_ylim(-0.5, 6.5)
    ax_farey.set_title('(D) Stern-Brocot Tree: Farey Mediant Hierarchy',
                        fontsize=10, fontweight='bold')
    ax_farey.axis('off')

    def draw_farey_tree(ax, max_depth=5):
        """Draw the Stern-Brocot tree between 0/1 and 1/1."""
        nodes = {}  # (p, q) -> (x_pos, depth)

        def add_node(p, q, depth, x_left, x_right):
            if depth > max_depth or q > 21:
                return
            x = (x_left + x_right) / 2
            nodes[(p, q)] = (x, depth)

            # Left child: mediant with left parent
            # Right child: mediant with right parent
            # For Stern-Brocot: mediant of (p_L/q_L, p/q) and (p/q, p_R/q_R)
            # Simplified: just recurse with bisection
            add_node(p, q, depth + 1, x_left, x)  # placeholder
            add_node(p, q, depth + 1, x, x_right)

        # Build the tree manually for clarity
        tree = [
            # depth 0
            [(0, 1, 0.0), (1, 1, 1.0)],
            # depth 1
            [(1, 2, 0.5)],
            # depth 2
            [(1, 3, 0.25), (2, 3, 0.75)],
            # depth 3
            [(1, 4, 0.125), (2, 5, 0.375), (3, 5, 0.625), (3, 4, 0.875)],
            # depth 4
            [(1, 5, 0.0625), (2, 7, 0.1875), (3, 8, 0.3125), (3, 7, 0.4375),
             (4, 7, 0.5625), (5, 8, 0.6875), (5, 7, 0.8125), (4, 5, 0.9375)],
            # depth 5 (partial — focus on FTD-relevant)
            [(1, 6, 0.03), (2, 9, 0.10), (3, 11, 0.16), (3, 10, 0.22),
             (5, 13, 0.34), (5, 12, 0.40), (4, 9, 0.47),
             (5, 11, 0.53), (7, 12, 0.60), (8, 13, 0.66),
             (7, 10, 0.72), (7, 9, 0.78), (6, 7, 0.86), (5, 6, 0.97)],
        ]

        y_scale = 6.0
        for depth, level in enumerate(tree):
            y = y_scale - depth * 1.1
            for p, q, x in level:
                # Determine color
                if q in INT_COLORS:
                    color = INT_COLORS[q]
                    size = 10
                    fontweight = 'bold'
                    bbox_props = dict(boxstyle='round,pad=0.2', facecolor='white',
                                      edgecolor=color, alpha=0.95, lw=1.5)
                elif (p, q) in [(5, 13), (8, 13)]:
                    color = INT_COLORS[13]
                    size = 9
                    fontweight = 'bold'
                    bbox_props = dict(boxstyle='round,pad=0.2', facecolor='white',
                                      edgecolor=color, alpha=0.95, lw=1.5)
                else:
                    color = NAVY
                    size = 7
                    fontweight = 'normal'
                    bbox_props = dict(boxstyle='round,pad=0.15', facecolor='white',
                                      edgecolor='gray', alpha=0.7, lw=0.5)

                ax.text(x, y, f'{p}/{q}', fontsize=size, ha='center', va='center',
                        color=color, fontweight=fontweight, bbox=bbox_props)

        # Draw connecting lines (parent to children mediants)
        connections = [
            # depth 0->1
            ((0.0, y_scale), (0.5, y_scale - 1.1)),
            ((1.0, y_scale), (0.5, y_scale - 1.1)),
            # depth 1->2
            ((0.0, y_scale), (0.25, y_scale - 2.2)),
            ((0.5, y_scale - 1.1), (0.25, y_scale - 2.2)),
            ((0.5, y_scale - 1.1), (0.75, y_scale - 2.2)),
            ((1.0, y_scale), (0.75, y_scale - 2.2)),
            # depth 2->3
            ((0.25, y_scale - 2.2), (0.125, y_scale - 3.3)),
            ((0.25, y_scale - 2.2), (0.375, y_scale - 3.3)),
            ((0.75, y_scale - 2.2), (0.625, y_scale - 3.3)),
            ((0.75, y_scale - 2.2), (0.875, y_scale - 3.3)),
        ]

        for (x1, y1), (x2, y2) in connections:
            ax.plot([x1, x2], [y1, y2], '-', color='gray', lw=0.5, alpha=0.4)

    draw_farey_tree(ax_farey)

    # Label key relationship
    ax_farey.text(0.5, -0.3,
                  r'$\frac{a}{b} \oplus \frac{c}{d} = \frac{a+c}{b+d}$'
                  '   (Farey mediant)',
                  fontsize=10, ha='center', color=NAVY)

    # =========================================================================
    # Panel E: Golden Mean and Fibonacci Convergents
    # =========================================================================
    ax_golden.set_title(r'(E) $1/\phi$: The Last Frequency to Lock',
                         fontsize=10, fontweight='bold', color=GOLD)

    # Plot tongue widths vs denominator q
    # Width ~ K^q at small K
    K_test = 0.7
    q_vals = np.arange(1, 22)
    widths = K_test ** q_vals  # approximate scaling

    ax_golden.bar(q_vals, widths, color=[INT_COLORS.get(q, '#AAAAAA') for q in q_vals],
                  edgecolor='gray', lw=0.3, alpha=0.7)
    ax_golden.set_xlabel('Denominator $q$', fontsize=10)
    ax_golden.set_ylabel(f'Tongue width $\\propto K^q$ ($K={K_test}$)', fontsize=10)
    ax_golden.set_yscale('log')
    ax_golden.tick_params(labelsize=8)

    # Mark Fibonacci denominators (golden mean convergents)
    fib_dens = [1, 2, 3, 5, 8, 13, 21]
    for fd in fib_dens:
        if fd <= 21:
            ax_golden.plot(fd, K_test**fd, 'v', color=GOLD, markersize=8,
                          zorder=5, markeredgecolor='black', markeredgewidth=0.5)

    # Mark FTD integers
    for q in [3, 4, 7, 13]:
        ax_golden.plot(q, K_test**q, '*', color=INT_COLORS[q], markersize=14,
                      zorder=6, markeredgecolor='black', markeredgewidth=0.5)

    # Convergent table
    convergents = [
        (r'$1/1$', 1.0, 'above'),
        (r'$1/2$', 0.5, 'below'),
        (r'$2/3$', 2/3, 'above'),
        (r'$3/5$', 3/5, 'below'),
        (r'$5/8$', 5/8, 'above'),
        (r'$\mathbf{8/13}$', 8/13, 'below'),
        (r'$13/21$', 13/21, 'above'),
    ]

    # Inset showing convergents bracketing 1/phi
    ax_ins = ax_golden.inset_axes([0.35, 0.45, 0.62, 0.50])
    ax_ins.set_xlim(-0.5, 6.5)
    ax_ins.set_ylim(0.48, 0.68)
    ax_ins.axhline(INV_PHI, color=GOLD, lw=2.0, alpha=0.5)
    ax_ins.text(6.3, INV_PHI, r'$1/\phi$', fontsize=9, color=GOLD,
                va='center', fontweight='bold')

    for i, (label, val, side) in enumerate(convergents):
        color = INT_COLORS.get([1,2,3,5,8,13,21][i], NAVY)
        marker = 'v' if side == 'above' else '^'
        ax_ins.plot(i, val, marker, color=color, markersize=6)
        ax_ins.text(i, val + (0.015 if side == 'above' else -0.02),
                    label, fontsize=7, ha='center',
                    va='bottom' if side == 'above' else 'top',
                    color=color)

    ax_ins.set_xlabel('Convergent index', fontsize=7)
    ax_ins.set_ylabel(r'$F_{n-1}/F_n$', fontsize=7)
    ax_ins.tick_params(labelsize=6)
    ax_ins.set_title('Fibonacci convergents bracket $1/\\phi$',
                      fontsize=8)

    # Legend
    from matplotlib.lines import Line2D
    legend_elements = [
        Line2D([0], [0], marker='v', color='w', markerfacecolor=GOLD,
               markersize=8, label='Fibonacci $F_n$'),
        Line2D([0], [0], marker='*', color='w', markerfacecolor=RED,
               markersize=12, label='FTD integer'),
    ]
    ax_golden.legend(handles=legend_elements, fontsize=8, loc='upper right')

    # =========================================================================
    # Panel F: FTD Connection Diagram
    # =========================================================================
    ax_conn.set_xlim(0, 10)
    ax_conn.set_ylim(0, 14)
    ax_conn.axis('off')
    ax_conn.set_title('(F) Why These Integers?', fontsize=10, fontweight='bold')

    y = 13.5

    # Title block
    ax_conn.text(5, y, 'Three Universal Structures\nConverge on {3, 4, 7, 13}',
                 fontsize=10, ha='center', va='top', fontweight='bold',
                 bbox=dict(boxstyle='round,pad=0.4', facecolor='#F0F0FF',
                           edgecolor=NAVY, lw=1.5))

    y -= 2.2

    # Structure 1: Lucas
    ax_conn.text(0.3, y, 'Lucas numbers:', fontsize=9, va='top',
                 fontweight='bold', color=PURPLE)
    ax_conn.text(0.3, y - 0.8,
                 r'$L_2=\mathbf{3},\ L_3=\mathbf{4},\ L_4=\mathbf{7}$'
                 '\n' + r'$L_n = L_{n-1} + L_{n-2}$'
                 '\n' + r'$L_n / L_{n-1} \to \phi$',
                 fontsize=9, va='top', color=PURPLE)

    y -= 3.2

    # Structure 2: Fibonacci
    ax_conn.text(0.3, y, 'Fibonacci numbers:', fontsize=9, va='top',
                 fontweight='bold', color=GOLD)
    ax_conn.text(0.3, y - 0.8,
                 r'$F_4 = \mathbf{3},\quad F_7 = \mathbf{13}$'
                 '\n' + r'$8/\mathbf{13} \to 1/\phi$  (convergent)'
                 '\n' + r'$\mathbf{4}/\mathbf{13} = [0;\, \mathbf{3},\, \mathbf{4}]$  (CF!)',
                 fontsize=9, va='top', color=GOLD)

    y -= 3.2

    # Structure 3: Feigenbaum × G*
    ax_conn.text(0.3, y, r'$\delta \times G^*$ arithmetic:', fontsize=9, va='top',
                 fontweight='bold', color=RED)
    ax_conn.text(0.3, y - 0.8,
                 r'$\lfloor\delta\rfloor = \mathbf{4}$'
                 '\n' + r'$\lfloor\delta + G^*\rfloor = \mathbf{7}$'
                 '\n' + r'$\lfloor\delta \times G^*\rfloor = \mathbf{13}$'
                 '\n' + r'$\mathrm{round}(\delta - G^* + 1) = \mathbf{3}$',
                 fontsize=9, va='top', color=RED)

    y -= 3.5

    # Synthesis
    ax_conn.text(5, y,
                 'Arnold tongues with denominators\n'
                 r'{$\mathbf{3, 4, 7, 13}$} guard the golden mean —'
                 '\n'
                 'the most irrational frequency,\n'
                 'the last orbit to mode-lock.',
                 fontsize=9, ha='center', va='top', style='italic',
                 bbox=dict(boxstyle='round,pad=0.4', facecolor='#FFFFF0',
                           edgecolor=GOLD, lw=1.5))

    # =========================================================================
    # Save
    # =========================================================================
    out = _FIGDIR / 'fig_arnold_tongues.png'
    fig.savefig(out, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f'  Saved: {out}')
    return out


if __name__ == '__main__':
    print('Generating Arnold tongue exploration...')
    print()
    gen_arnold_tongues()
    print('\nDone.')
