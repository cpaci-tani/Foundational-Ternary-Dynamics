"""
The Triad: How pi, G*, and varpi merge into each other.

Three constants. One identity: pi = 4varpi²/G*².
Any two determine the third. This script verifies all three directions
and visualizes the convergence of the Wallis products that define them.

Requirements: pip install numpy matplotlib scipy
"""

import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib.patches import FancyArrowPatch, Circle
from scipy.special import gamma
from matplotlib import rcParams

# ── Professional styling ─────────────────────────────────────────
rcParams['font.family'] = 'serif'
rcParams['font.size'] = 10
rcParams['axes.linewidth'] = 0.8
rcParams['figure.dpi'] = 150

# ── Exact constants (from Gamma values) ──────────────────────────
g1 = gamma(0.25)    # Γ(1/4)
g2 = gamma(0.50)    # Γ(1/2) = √pi
g34 = gamma(0.75)   # Γ(3/4)

PI = g2**2
GSTAR = g1 / g34                          # = g1² / (√2 · g2²)
VARPI = g1**2 / (2 * np.sqrt(2) * g2)

print("=" * 65)
print("  THE TRIAD: pi, G*, varpi — THREE CONSTANTS, ONE IDENTITY")
print("=" * 65)
print()

# ══════════════════════════════════════════════════════════════════
# PART 1: Algebraic verification — all three merge directions
# ══════════════════════════════════════════════════════════════════

print("─── DIRECTION 1: pi and G* merge into varpi ───")
varpi_from_pi_gstar = GSTAR * np.sqrt(PI) / 2
print(f"  varpi = G* · √pi / 2 = {varpi_from_pi_gstar:.16f}")
print(f"  varpi (exact)        = {VARPI:.16f}")
print(f"  Match: {np.abs(varpi_from_pi_gstar - VARPI) < 1e-14}")
print()

print("─── DIRECTION 2: varpi and G* merge into pi ───")
pi_from_varpi_gstar = 4 * VARPI**2 / GSTAR**2
print(f"  pi = 4varpi²/G*² = {pi_from_varpi_gstar:.16f}")
print(f"  pi (exact)    = {PI:.16f}")
print(f"  Match: {np.abs(pi_from_varpi_gstar - PI) < 1e-14}")
print()

print("─── DIRECTION 3: pi and varpi merge into G* ───")
gstar_from_pi_varpi = 2 * VARPI / np.sqrt(PI)
print(f"  G* = 2varpi/√pi = {gstar_from_pi_varpi:.16f}")
print(f"  G* (exact)  = {GSTAR:.16f}")
print(f"  Match: {np.abs(gstar_from_pi_varpi - GSTAR) < 1e-14}")
print()

print("─── THE SIMPLEST FORMS ───")
print(f"  G* = Γ(1/4) / Γ(3/4)         = {g1/g34:.16f}")
print(f"  √pi = Γ(1/2)                  = {g2:.16f}")
print(f"  varpi  = Γ(1/4)² / (2√2·Γ(1/2)) = {VARPI:.16f}")
print(f"  pi  = Γ(1/2)²                 = {PI:.16f}")
print()

# ══════════════════════════════════════════════════════════════════
# PART 2: Wallis product convergence
# ══════════════════════════════════════════════════════════════════

N_MAX = 500

def race1_partial(n):
    """√pi = lim N^{-1/2} * prod(2k)/(2k-1)"""
    prod = 1.0
    for k in range(1, n + 1):
        prod *= (2*k) / (2*k - 1)
    return prod / np.sqrt(n) if n > 0 else 0

def race2_partial(n):
    """G* = lim (N+1)^{-1/2} * prod(4k+3)/(4k+1)"""
    prod = 1.0
    for k in range(0, n + 1):
        prod *= (4*k + 3) / (4*k + 1)
    return prod / np.sqrt(n + 1)

ns = np.arange(1, N_MAX + 1)
sqrt_pi_vals = np.array([race1_partial(n) for n in ns])
gstar_vals = np.array([race2_partial(n) for n in ns])
varpi_vals = gstar_vals * sqrt_pi_vals / 2

# Errors
sqrt_pi_err = np.abs(sqrt_pi_vals - np.sqrt(PI))
gstar_err = np.abs(gstar_vals - GSTAR)
varpi_err = np.abs(varpi_vals - VARPI)

# Locked digits
def locked_digits(val, exact):
    if val == 0 or exact == 0:
        return 0
    return max(0, -int(np.floor(np.log10(np.abs(val - exact) / np.abs(exact) + 1e-20))))

sqrt_pi_digits = [locked_digits(v, np.sqrt(PI)) for v in sqrt_pi_vals]
gstar_digits = [locked_digits(v, GSTAR) for v in gstar_vals]
varpi_digits = [locked_digits(v, VARPI) for v in varpi_vals]

print("─── CONVERGENCE TABLE ───")
print(f"{'N':>6} {'√pi digits':>10} {'G* digits':>10} {'varpi digits':>10}")
print("-" * 40)
for n in [1, 2, 5, 10, 20, 50, 100, 200, 500]:
    i = n - 1
    print(f"{n:6d} {sqrt_pi_digits[i]:10d} {gstar_digits[i]:10d} {varpi_digits[i]:10d}")
print()

# ══════════════════════════════════════════════════════════════════
# PART 3: Visualization
# ══════════════════════════════════════════════════════════════════

fig = plt.figure(figsize=(16, 14))
gs = gridspec.GridSpec(3, 3, hspace=0.35, wspace=0.3,
                       left=0.07, right=0.95, top=0.93, bottom=0.05)

fig.suptitle("The Triad: pi, G*, and varpi — Three Constants, One Identity",
             fontsize=16, fontweight='bold', y=0.97)

# ── Colors ──
COL_PI = '#d44'
COL_G = '#26c'
COL_V = '#c80'
COL_BG = '#f8f8fa'

# ─────────────────────────────────────────────────────────────────
# Panel 1: The triad diagram (top-left, spans 2 columns)
# ─────────────────────────────────────────────────────────────────
ax_triad = fig.add_subplot(gs[0, 0:2])
ax_triad.set_xlim(-1.5, 1.5)
ax_triad.set_ylim(-1.0, 1.2)
ax_triad.set_aspect('equal')
ax_triad.axis('off')
ax_triad.set_title('The Triad Identity', fontsize=13, fontweight='bold')

# Triangle vertices
verts = {'pi': (0, 1.0), 'G*': (-1.1, -0.5), 'varpi': (1.1, -0.5)}
colors = {'pi': COL_PI, 'G*': COL_G, 'varpi': COL_V}
values = {'pi': f'{PI:.8f}', 'G*': f'{GSTAR:.8f}', 'varpi': f'{VARPI:.8f}'}

for name, (x, y) in verts.items():
    circ = Circle((x, y), 0.25, facecolor=colors[name], alpha=0.15,
                  edgecolor=colors[name], linewidth=2)
    ax_triad.add_patch(circ)
    ax_triad.text(x, y + 0.05, name, ha='center', va='center',
                  fontsize=18, fontweight='bold', color=colors[name])
    ax_triad.text(x, y - 0.15, values[name], ha='center', va='center',
                  fontsize=7, color=colors[name], family='monospace')

# Edges with formulas
edges = [
    ('pi', 'G*', 'varpi = G*·√pi / 2'),
    ('G*', 'varpi', 'pi = 4varpi² / G*²'),
    ('varpi', 'pi', 'G* = 2varpi / √pi'),
]
for (a, b, label) in edges:
    xa, ya = verts[a]
    xb, yb = verts[b]
    mx, my = (xa + xb) / 2, (ya + yb) / 2
    dx, dy = xb - xa, yb - ya
    norm = np.sqrt(dx**2 + dy**2)
    # Shorten arrows
    ax_triad.annotate('', xy=(xb - 0.3*dx/norm, yb - 0.3*dy/norm),
                      xytext=(xa + 0.3*dx/norm, ya + 0.3*dy/norm),
                      arrowprops=dict(arrowstyle='->', color='#888',
                                      lw=1.5, connectionstyle='arc3,rad=0.1'))
    # Label offset perpendicular to edge
    nx, ny = -dy/norm, dx/norm
    ax_triad.text(mx + 0.2*nx, my + 0.2*ny, label, ha='center', va='center',
                  fontsize=8, style='italic', color='#555',
                  bbox=dict(boxstyle='round,pad=0.2', facecolor='white',
                            edgecolor='#ddd', alpha=0.9))

# ─────────────────────────────────────────────────────────────────
# Panel 2: Wallis product terms (top-right)
# ─────────────────────────────────────────────────────────────────
ax_terms = fig.add_subplot(gs[0, 2])
ax_terms.set_title('Individual Wallis Factors', fontsize=11, fontweight='bold')

ks = np.arange(1, 30)
race1_factors = (2*ks) / (2*ks - 1)
race2_factors = (4*ks + 3) / (4*ks + 1)  # shifted: k starts at 0 for race2

ax_terms.plot(ks, race1_factors, 'o-', color=COL_PI, markersize=4,
              label='Race 1: (2k)/(2k−1)', linewidth=1.2)
ax_terms.plot(ks, race2_factors, 's-', color=COL_G, markersize=4,
              label='Race 2: (4k+3)/(4k+1)', linewidth=1.2)
ax_terms.axhline(y=1, color='#ccc', linewidth=0.8, linestyle='--')
ax_terms.set_xlabel('k')
ax_terms.set_ylabel('Factor value')
ax_terms.legend(fontsize=7, loc='upper right')
ax_terms.set_ylim(0.98, 2.1)

# ─────────────────────────────────────────────────────────────────
# Panel 3: Convergence of partial products (middle-left)
# ─────────────────────────────────────────────────────────────────
ax_conv = fig.add_subplot(gs[1, 0:2])
ax_conv.set_title('Convergence of the Two Races + Composite varpi', fontsize=11, fontweight='bold')

ax_conv.plot(ns, sqrt_pi_vals, color=COL_PI, linewidth=1.5, alpha=0.8,
             label=f'√pi → {np.sqrt(PI):.6f}')
ax_conv.plot(ns, gstar_vals, color=COL_G, linewidth=1.5, alpha=0.8,
             label=f'G* → {GSTAR:.6f}')
ax_conv.plot(ns, varpi_vals, color=COL_V, linewidth=1.5, alpha=0.8,
             label=f'varpi = G*·√pi/2 → {VARPI:.6f}')

ax_conv.axhline(y=np.sqrt(PI), color=COL_PI, linewidth=0.8, linestyle='--', alpha=0.5)
ax_conv.axhline(y=GSTAR, color=COL_G, linewidth=0.8, linestyle='--', alpha=0.5)
ax_conv.axhline(y=VARPI, color=COL_V, linewidth=0.8, linestyle='--', alpha=0.5)

ax_conv.set_xlabel('N (number of terms)')
ax_conv.set_ylabel('Partial product S_N')
ax_conv.legend(fontsize=8, loc='right')
ax_conv.set_xlim(1, N_MAX)

# ─────────────────────────────────────────────────────────────────
# Panel 4: Error decay (middle-right)
# ─────────────────────────────────────────────────────────────────
ax_err = fig.add_subplot(gs[1, 2])
ax_err.set_title('Error Decay (log scale)', fontsize=11, fontweight='bold')

ax_err.semilogy(ns[1:], sqrt_pi_err[1:], color=COL_PI, linewidth=1.2,
                alpha=0.8, label='|S_N − √pi|')
ax_err.semilogy(ns[1:], gstar_err[1:], color=COL_G, linewidth=1.2,
                alpha=0.8, label='|S_N − G*|')
ax_err.semilogy(ns[1:], varpi_err[1:], color=COL_V, linewidth=1.2,
                alpha=0.8, label='|S_N − varpi|')

# Reference line: O(1/N)
ref_n = ns[1:]
ax_err.semilogy(ref_n, 0.5/ref_n, 'k--', linewidth=0.6, alpha=0.4, label='O(1/N)')

ax_err.set_xlabel('N')
ax_err.set_ylabel('Absolute error')
ax_err.legend(fontsize=7)
ax_err.set_xlim(2, N_MAX)

# ─────────────────────────────────────────────────────────────────
# Panel 5: Digits locked (bottom-left)
# ─────────────────────────────────────────────────────────────────
ax_dig = fig.add_subplot(gs[2, 0])
ax_dig.set_title('Correct Digits vs. Terms', fontsize=11, fontweight='bold')

ax_dig.plot(ns, sqrt_pi_digits, color=COL_PI, linewidth=1.5, label='√pi')
ax_dig.plot(ns, gstar_digits, color=COL_G, linewidth=1.5, label='G*')
ax_dig.plot(ns, varpi_digits, color=COL_V, linewidth=1.5, label='varpi')

ax_dig.set_xlabel('N (terms)')
ax_dig.set_ylabel('Locked digits')
ax_dig.legend(fontsize=8)
ax_dig.set_xlim(1, N_MAX)

# ─────────────────────────────────────────────────────────────────
# Panel 6: The three merge directions (bottom-center)
# ─────────────────────────────────────────────────────────────────
ax_merge = fig.add_subplot(gs[2, 1])
ax_merge.set_title('Three Merge Directions', fontsize=11, fontweight='bold')
ax_merge.axis('off')

merges = [
    (f'pi + G* → varpi', f'varpi = G*·√pi/2 = {VARPI:.10f}', COL_V),
    (f'varpi + G* → pi', f'pi = 4varpi²/G*² = {PI:.10f}', COL_PI),
    (f'pi + varpi → G*', f'G* = 2varpi/√pi = {GSTAR:.10f}', COL_G),
]
for i, (title, formula, col) in enumerate(merges):
    y = 0.85 - i * 0.33
    ax_merge.text(0.05, y, title, fontsize=11, fontweight='bold',
                  color=col, transform=ax_merge.transAxes)
    ax_merge.text(0.05, y - 0.12, formula, fontsize=9, family='monospace',
                  color='#444', transform=ax_merge.transAxes)
    ax_merge.plot([0.05, 0.95], [y - 0.2, y - 0.2], color='#ddd',
                 linewidth=0.5, transform=ax_merge.transAxes)

# ─────────────────────────────────────────────────────────────────
# Panel 7: Race structure (bottom-right)
# ─────────────────────────────────────────────────────────────────
ax_race = fig.add_subplot(gs[2, 2])
ax_race.set_title('The Race Structure', fontsize=11, fontweight='bold')
ax_race.axis('off')

race_text = [
    ('Race 1 (mod 2)', '∏ (2k)/(2k−1)', '→ √pi', COL_PI),
    ('Race 2 (mod 4)', '∏ (4k+3)/(4k+1)', '→ G*', COL_G),
    ('Both races / 2', 'Race 1 × Race 2 / 2', '→ varpi', COL_V),
    ('', '', '', '#888'),
    ('Vocabulary', 'evens vs odds', '= √pi', COL_PI),
    ('Grammar', 'inert vs split', '= G*', COL_G),
    ('Sentence', 'vocabulary × grammar', '= varpi', COL_V),
]
for i, (a, b, c, col) in enumerate(race_text):
    y = 0.92 - i * 0.13
    if a:
        ax_race.text(0.02, y, a, fontsize=9, fontweight='bold',
                     color=col, transform=ax_race.transAxes)
        ax_race.text(0.02, y - 0.06, f'  {b} {c}', fontsize=7.5,
                     family='monospace', color='#555', transform=ax_race.transAxes)

# ── Save ──
output_path = 'C:/Users/cpaci/Desktop/ftd/docs/papers/triad_visualization.png'
plt.savefig(output_path, dpi=200, bbox_inches='tight', facecolor='white')
print(f"Saved: {output_path}")

output_pdf = 'C:/Users/cpaci/Desktop/ftd/docs/papers/triad_visualization.pdf'
plt.savefig(output_pdf, bbox_inches='tight', facecolor='white')
print(f"Saved: {output_pdf}")

plt.show()

print()
print("=" * 65)
print("  VERIFICATION COMPLETE")
print("  pi = 4varpi²/G*² ✓   varpi = G*√pi/2 ✓   G* = 2varpi/√pi ✓")
print("  The Wallis product for √pi IS √pi.")
print("  The Wallis product for G* IS G*.")
print("  Both go all the way down.")
print("=" * 65)
