#!/usr/bin/env python3
"""
Transfer Matrix Visualizations
================================
Visual outputs of the L=2 and L=3 transfer matrix analyses.
"""

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec
from itertools import product
import matplotlib.colors as mcolors

# ---- Shared setup ----
plt.rcParams.update({
    'figure.facecolor': '#0a0a1a',
    'axes.facecolor': '#0a0a1a',
    'text.color': '#e0e0e0',
    'axes.labelcolor': '#e0e0e0',
    'xtick.color': '#a0a0a0',
    'ytick.color': '#a0a0a0',
    'axes.edgecolor': '#404060',
    'grid.color': '#252540',
    'grid.alpha': 0.5,
    'font.family': 'sans-serif',
    'font.size': 10,
})

GOLD = '#FFD700'
CYAN = '#00CED1'
MAGENTA = '#FF00FF'
LIME = '#7FFF00'
CORAL = '#FF6B6B'
SKY = '#87CEEB'
WHITE = '#FFFFFF'

def build_laplacian(L):
    N = L**3
    def idx(x, y, z):
        return (x % L) * L * L + (y % L) * L + (z % L)
    Lap = np.zeros((N, N))
    for i in range(N):
        x, y, z = i // (L*L), (i // L) % L, i % L
        Lap[i, i] += -4.0
        for dx, dy, dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
            Lap[i, idx(x+dx, y+dy, z+dz)] += 1.0/3.0
        for dx, dy, dz in [(1,1,0),(1,-1,0),(-1,1,0),(-1,-1,0),
                            (1,0,1),(1,0,-1),(-1,0,1),(-1,0,-1),
                            (0,1,1),(0,1,-1),(0,-1,1),(0,-1,-1)]:
            Lap[i, idx(x+dx, y+dy, z+dz)] += 1.0/6.0
    return Lap

def transfer_matrix_L2():
    L = 2
    n_slice = L * L
    Lap = build_laplacian(L)
    G = np.linalg.pinv(-Lap, rcond=1e-10)
    G_aa = G[:n_slice, :n_slice]
    G_ab = G[:n_slice, n_slice:]
    slice_cfgs = list(product([-1, 0, 1], repeat=n_slice))
    S = np.array(slice_cfgs, dtype=np.float64)
    q_aa = np.einsum('ai,ij,aj->a', S, G_aa, S)
    coupling = S @ G_ab @ S.T
    T = np.exp(coupling + 0.25 * (q_aa[:, None] + q_aa[None, :]))
    eigs = np.sort(np.real(np.linalg.eigvals(T)))[::-1]
    return eigs, T, G

# ---- Compute ----
print("Computing L=2 transfer matrix...")
eigs_2, T_2, G_2 = transfer_matrix_L2()

# L=3 eigenvalues (precomputed from the scaling run)
eigs_3_top = np.array([41037.581, 1067.053, 1067.053, 1067.053, 1067.053,
                        220.068, 27.748, 24.903, 24.903, 24.903,
                        22.82, 22.82, 22.82, 22.82, 16.88,
                        3.82, 3.82, 3.82, 3.82, 0.81])

# ===========================================================================
# FIGURE 1: Eigenvalue Spectrum Comparison
# ===========================================================================
fig = plt.figure(figsize=(16, 10))
gs = GridSpec(2, 3, figure=fig, hspace=0.35, wspace=0.35)

# Panel 1: L=2 eigenvalue spectrum (log scale)
ax1 = fig.add_subplot(gs[0, 0])
pos_eigs = eigs_2[eigs_2 > 1e-10]
ax1.bar(range(len(pos_eigs[:25])), np.log10(pos_eigs[:25]), color=CYAN, alpha=0.8, width=0.8)
ax1.axhline(y=np.log10(eigs_2[0]/47), color=GOLD, linestyle='--', alpha=0.7, label='$\\lambda_1/47$')
ax1.set_xlabel('Eigenvalue index')
ax1.set_ylabel('log$_{10}(\\lambda)$')
ax1.set_title('L=2 Eigenvalue Spectrum', color=GOLD, fontsize=12, fontweight='bold')
ax1.legend(fontsize=8)
ax1.grid(True, alpha=0.3)

# Panel 2: L=3 eigenvalue spectrum
ax2 = fig.add_subplot(gs[0, 1])
ax2.bar(range(len(eigs_3_top)), np.log10(np.maximum(eigs_3_top, 0.01)), color=MAGENTA, alpha=0.8, width=0.8)
ax2.axhline(y=np.log10(eigs_3_top[0]/47), color=GOLD, linestyle='--', alpha=0.7, label='$\\lambda_1/47$')
ax2.set_xlabel('Eigenvalue index')
ax2.set_ylabel('log$_{10}(\\lambda)$')
ax2.set_title('L=3 Top 20 Eigenvalues', color=MAGENTA, fontsize=12, fontweight='bold')
ax2.legend(fontsize=8)
ax2.grid(True, alpha=0.3)

# Panel 3: Spectral gap scaling
ax3 = fig.add_subplot(gs[0, 2])
Ls = [2, 3]
gaps = [np.log(45.871034), np.log(38.458818)]
inv_L = [1/l for l in Ls]

ax3.scatter(inv_L, gaps, color=CYAN, s=120, zorder=5, edgecolors=WHITE, linewidths=1.5)
ax3.plot(inv_L, gaps, color=CYAN, alpha=0.5, linewidth=2)

# Extrapolation lines
inv_L_ext = np.linspace(0, 0.6, 100)
# Linear: gap = gap_inf + c/L
gap_inf_lin = 3*gaps[1] - 2*gaps[0]
c_lin = 6*(gaps[0] - gaps[1])
ax3.plot(inv_L_ext, gap_inf_lin + c_lin * inv_L_ext, '--', color=LIME, alpha=0.6, label=f'Linear: gap$_\\infty$ = {gap_inf_lin:.2f}')

# Quadratic
gap_inf_q = (9*gaps[1] - 4*gaps[0]) / 5
c_q = (gaps[0] - gap_inf_q) * 4
ax3.plot(inv_L_ext, gap_inf_q + c_q * inv_L_ext**2, '--', color=CORAL, alpha=0.6, label=f'Quadratic: gap$_\\infty$ = {gap_inf_q:.2f}')

# Reference lines
ax3.axhline(y=np.log(47), color=GOLD, linestyle=':', alpha=0.8, linewidth=2, label=f'ln(47) = {np.log(47):.3f}')
ax3.axhline(y=np.log(27), color=SKY, linestyle=':', alpha=0.6, linewidth=1.5, label=f'ln(27) = {np.log(27):.3f}')

ax3.set_xlabel('1/L')
ax3.set_ylabel('Spectral gap = ln($\\lambda_1/\\lambda_2$)')
ax3.set_title('Spectral Gap Scaling', color=GOLD, fontsize=12, fontweight='bold')
ax3.legend(fontsize=7, loc='upper left')
ax3.set_xlim(-0.05, 0.6)
ax3.grid(True, alpha=0.3)

# Panel 4: Degeneracy histogram L=2
ax4 = fig.add_subplot(gs[1, 0])
rounded_2 = np.round(eigs_2, 3)
unique_2, counts_2 = np.unique(rounded_2[rounded_2 > 0.001], return_counts=True)
sort_idx = np.argsort(-unique_2)
unique_2, counts_2 = unique_2[sort_idx][:15], counts_2[sort_idx][:15]

colors = [GOLD if c == 1 else CYAN if c == 2 else MAGENTA if c == 3 else LIME for c in counts_2]
bars = ax4.barh(range(len(counts_2)), counts_2, color=colors, alpha=0.8)
ax4.set_yticks(range(len(counts_2)))
ax4.set_yticklabels([f'{e:.3f}' for e in unique_2], fontsize=7)
ax4.set_xlabel('Degeneracy')
ax4.set_ylabel('Eigenvalue')
ax4.set_title('L=2 Degeneracy Structure', color=CYAN, fontsize=12, fontweight='bold')
ax4.grid(True, axis='x', alpha=0.3)

# Panel 5: Degeneracy histogram L=3
ax5 = fig.add_subplot(gs[1, 1])
rounded_3 = np.round(eigs_3_top, 2)
unique_3, counts_3 = np.unique(rounded_3[rounded_3 > 0.01], return_counts=True)
sort_idx = np.argsort(-unique_3)
unique_3, counts_3 = unique_3[sort_idx], counts_3[sort_idx]

colors_3 = [GOLD if c == 1 else MAGENTA if c == 4 else LIME for c in counts_3]
bars3 = ax5.barh(range(len(counts_3)), counts_3, color=colors_3, alpha=0.8)
ax5.set_yticks(range(len(counts_3)))
ax5.set_yticklabels([f'{e:.1f}' for e in unique_3], fontsize=8)
ax5.set_xlabel('Degeneracy')
ax5.set_ylabel('Eigenvalue')
ax5.set_title('L=3 Degeneracy Structure', color=MAGENTA, fontsize=12, fontweight='bold')
# Annotate the 4-fold degeneracies
for i, (u, c) in enumerate(zip(unique_3, counts_3)):
    if c == 4:
        ax5.annotate(f'4 = N_base', xy=(c + 0.1, i), fontsize=7, color=GOLD, va='center')
ax5.grid(True, axis='x', alpha=0.3)

# Panel 6: Integer Green's function heatmap
ax6 = fig.add_subplot(gs[1, 2])
G128 = np.round(128 * G_2).astype(int)
im = ax6.imshow(G128, cmap='RdBu_r', aspect='equal', vmin=-10, vmax=30)
ax6.set_title('128 * G  (L=2 torus)', color=LIME, fontsize=12, fontweight='bold')
ax6.set_xlabel('Site j')
ax6.set_ylabel('Site i')
# Annotate each cell
for i in range(8):
    for j in range(8):
        val = G128[i, j]
        color = '#000000' if abs(val) < 15 else WHITE
        ax6.text(j, i, str(val), ha='center', va='center', fontsize=9,
                color=color, fontweight='bold')
cbar = plt.colorbar(im, ax=ax6, shrink=0.8)
cbar.ax.yaxis.set_tick_params(color='#a0a0a0')
plt.setp(plt.getp(cbar.ax.axes, 'yticklabels'), color='#a0a0a0')

fig.suptitle('Transfer Matrix on Ternary Torus: L=2 vs L=3',
             color=WHITE, fontsize=16, fontweight='bold', y=0.98)

plt.savefig('output/transfer_matrix_spectrum.png', dpi=150, bbox_inches='tight',
            facecolor='#0a0a1a', edgecolor='none')
print("Saved: output/transfer_matrix_spectrum.png")

# ===========================================================================
# FIGURE 2: Framework Polynomial and Mod-p Structure
# ===========================================================================
fig2 = plt.figure(figsize=(16, 10))
gs2 = GridSpec(2, 3, figure=fig2, hspace=0.35, wspace=0.35)

# Panel 1: P(x) = (x-3)(x-4)(x-7)(x-13)
ax = fig2.add_subplot(gs2[0, 0])
x = np.linspace(-2, 20, 500)
Px = (x - 3) * (x - 4) * (x - 7) * (x - 13)
ax.plot(x, Px, color=GOLD, linewidth=2, label='P(x) = (x-3)(x-4)(x-7)(x-13)')
ax.axhline(y=0, color='#404060', linewidth=0.5)
for root, name, col in [(3, '$N_c$', CYAN), (4, '$N_{base}$', MAGENTA),
                          (7, '$b_3$', LIME), (13, '$N_{eff}$', CORAL)]:
    ax.axvline(x=root, color=col, linestyle='--', alpha=0.5)
    ax.scatter([root], [0], color=col, s=100, zorder=5, edgecolors=WHITE, linewidths=1.5)
    ax.annotate(f'{name}={root}', xy=(root, -50), fontsize=9, color=col,
               ha='center', va='top', fontweight='bold')
ax.set_xlabel('x')
ax.set_ylabel('P(x)')
ax.set_title('Framework Polynomial', color=GOLD, fontsize=12, fontweight='bold')
ax.set_ylim(-200, 400)
ax.grid(True, alpha=0.3)
ax.legend(fontsize=7, loc='upper left')

# Panel 2: P(x) mod 27
ax2 = fig2.add_subplot(gs2[0, 1])
x_int = np.arange(0, 27)
Px_mod27 = ((x_int - 3) * (x_int - 4) * (x_int - 7) * (x_int - 13)) % 27
Qx_mod27 = ((x_int - 1) * (x_int - 3) * (x_int**2 + 4*x_int + 13)) % 27

bars = ax2.bar(x_int - 0.2, Px_mod27, width=0.4, color=GOLD, alpha=0.8, label='P(x) mod 27')
bars2 = ax2.bar(x_int + 0.2, Qx_mod27, width=0.4, color=CYAN, alpha=0.8,
                label='$(x-1)(x-N_c)(x^2+N_{base}x+N_{eff})$ mod 27')
ax2.set_xlabel('x (mod 27)')
ax2.set_ylabel('Value (mod 27)')
ax2.set_title('P(x) mod 27 = Q(x) mod 27', color=CYAN, fontsize=12, fontweight='bold')
ax2.legend(fontsize=7)
ax2.grid(True, alpha=0.3)

# Panel 3: Transfer matrix heatmap (L=2, log scale)
ax3 = fig2.add_subplot(gs2[0, 2])
T_log = np.log10(np.maximum(T_2, 1e-20))
im3 = ax3.imshow(T_log, cmap='inferno', aspect='equal')
ax3.set_title('log$_{10}$(T)  (L=2, 81x81)', color=CORAL, fontsize=12, fontweight='bold')
ax3.set_xlabel('Config b')
ax3.set_ylabel('Config a')
cbar3 = plt.colorbar(im3, ax=ax3, shrink=0.8)
cbar3.ax.yaxis.set_tick_params(color='#a0a0a0')
plt.setp(plt.getp(cbar3.ax.axes, 'yticklabels'), color='#a0a0a0')

# Panel 4: Eigenvalue ratios with framework integers
ax4 = fig2.add_subplot(gs2[1, 0])
ratios_2 = []
labels_2 = []
for i in range(1, min(15, len(eigs_2))):
    if eigs_2[i] > 0.01:
        ratios_2.append(eigs_2[0] / eigs_2[i])
        labels_2.append(f'$\\lambda_1/\\lambda_{{{i+1}}}$')

colors_r = []
for r in ratios_2:
    if abs(r - 3) < 0.5: colors_r.append(CYAN)
    elif abs(r - 13) < 2: colors_r.append(CORAL)
    elif abs(r - 47) < 5: colors_r.append(GOLD)
    else: colors_r.append('#606080')

ax4.barh(range(len(ratios_2)), ratios_2, color=colors_r, alpha=0.8)
ax4.set_yticks(range(len(ratios_2)))
ax4.set_yticklabels(labels_2, fontsize=8)
ax4.set_xlabel('Ratio')
# Reference lines
for val, name, col in [(3, '$N_c$', CYAN), (np.sqrt(13), '$\\sqrt{N_{eff}}$', MAGENTA),
                        (13, '$N_{eff}$', CORAL), (47, '$D$', GOLD)]:
    ax4.axvline(x=val, color=col, linestyle='--', alpha=0.6, linewidth=1.5)
    ax4.text(val, len(ratios_2)-0.5, name, color=col, fontsize=8, ha='center', va='bottom')
ax4.set_title('L=2 Eigenvalue Ratios', color=GOLD, fontsize=12, fontweight='bold')
ax4.set_xlim(0, max(ratios_2) * 1.1)
ax4.grid(True, axis='x', alpha=0.3)

# Panel 5: G(0,0) convergence to Watson integral
ax5 = fig2.add_subplot(gs2[1, 1])
Ls_plot = [2, 3, 4, 5, 6, 8, 10, 15, 20]
G00_exact = []
for L in Ls_plot:
    Lap = build_laplacian(L)
    G = np.linalg.pinv(-Lap, rcond=1e-10)
    G00_exact.append(G[0, 0])
    print(f"  L={L}: G(0,0) = {G[0,0]:.8f}")

watson = 2.95868**2 / (2 * np.pi)
ax5.plot(Ls_plot, G00_exact, 'o-', color=CYAN, markersize=8, linewidth=2,
         markeredgecolor=WHITE, markeredgewidth=1.5, label='G(0,0) computed')
ax5.axhline(y=watson, color=GOLD, linestyle='--', linewidth=2, alpha=0.8,
            label=f'$G*^2/(2\\pi)$ = {watson:.4f}')
ax5.set_xlabel('Lattice size L')
ax5.set_ylabel('G(0,0)')
ax5.set_title('G(0,0) $\\rightarrow$ Watson BCC Integral', color=CYAN, fontsize=12, fontweight='bold')
ax5.legend(fontsize=8)
ax5.grid(True, alpha=0.3)

# Panel 6: Laplacian eigenvalue spectra comparison
ax6 = fig2.add_subplot(gs2[1, 2])
for L, col, label in [(2, CYAN, 'L=2'), (3, MAGENTA, 'L=3'), (4, LIME, 'L=4')]:
    Lap = build_laplacian(L)
    eig_lap = np.sort(np.linalg.eigvalsh(Lap))
    # Normalize to [-1, 0]
    eig_norm = eig_lap / np.min(eig_lap) if np.min(eig_lap) != 0 else eig_lap
    ax6.plot(np.linspace(0, 1, len(eig_norm)), np.sort(eig_norm),
             'o-', color=col, markersize=3, linewidth=1.5, alpha=0.8, label=label)

ax6.set_xlabel('Normalized index')
ax6.set_ylabel('Normalized eigenvalue')
ax6.set_title('Laplacian Spectra (normalized)', color=LIME, fontsize=12, fontweight='bold')
ax6.legend(fontsize=8)
ax6.grid(True, alpha=0.3)

fig2.suptitle('Framework Polynomial, Mod-27 Arithmetic, and Convergence',
              color=WHITE, fontsize=16, fontweight='bold', y=0.98)

plt.savefig('output/transfer_matrix_arithmetic.png', dpi=150, bbox_inches='tight',
            facecolor='#0a0a1a', edgecolor='none')
print("Saved: output/transfer_matrix_arithmetic.png")

# ===========================================================================
# FIGURE 3: Golden Ratio Skeleton and Lucas/Fibonacci
# ===========================================================================
fig3, axes = plt.subplots(1, 3, figsize=(16, 6), facecolor='#0a0a1a')

# Panel 1: Lucas and Fibonacci sequences with framework integers
ax = axes[0]
n_vals = range(0, 12)
lucas = [2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123, 199]
fib = [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89]

ax.semilogy(n_vals, [max(l, 0.5) for l in lucas], 'o-', color=GOLD, markersize=8,
            linewidth=2, label='Lucas $L_n$', markeredgecolor=WHITE, markeredgewidth=1)
ax.semilogy(n_vals, [max(f, 0.5) for f in fib], 's-', color=CYAN, markersize=7,
            linewidth=2, label='Fibonacci $F_n$', markeredgecolor=WHITE, markeredgewidth=1)

# Highlight framework integers
framework = {3: ('$N_c$=3', 'L_2'), 4: ('$N_{base}$=4', 'L_3'),
             7: ('$b_3$=7', 'L_4'), 47: ('D=47', 'L_8'),
             13: ('$N_{eff}$=13', 'F_7')}
for val, (name, seq) in framework.items():
    if val in lucas:
        idx = lucas.index(val)
        ax.annotate(f'{name}\n{seq}', xy=(idx, val), xytext=(idx+0.5, val*2),
                   fontsize=8, color=GOLD, fontweight='bold',
                   arrowprops=dict(arrowstyle='->', color=GOLD, alpha=0.5))
    if val in fib:
        idx = fib.index(val)
        ax.annotate(f'{name}\n{seq}', xy=(idx, val), xytext=(idx+0.5, val*0.3),
                   fontsize=8, color=CYAN, fontweight='bold',
                   arrowprops=dict(arrowstyle='->', color=CYAN, alpha=0.5))

ax.set_xlabel('n')
ax.set_ylabel('Value')
ax.set_title('Golden Ratio Skeleton', color=GOLD, fontsize=12, fontweight='bold')
ax.legend(fontsize=8)
ax.grid(True, alpha=0.3)

# Panel 2: Supersingular vs Ordinary classification
ax2 = axes[1]
primes_fw = [3, 7, 13, 47]
types = ['Supersingular\n(p mod 4 = 3)', 'Supersingular\n(p mod 4 = 3)',
         'Ordinary\n(p mod 4 = 1)', 'Supersingular\n(p mod 4 = 3)']
colors_ss = [MAGENTA, MAGENTA, LIME, MAGENTA]
names = ['$N_c$', '$b_3$', '$N_{eff}$', 'D']

bars = ax2.bar(range(4), primes_fw, color=colors_ss, alpha=0.8, edgecolor=WHITE, linewidth=1.5)
ax2.set_xticks(range(4))
ax2.set_xticklabels([f'{names[i]}\n= {primes_fw[i]}\n{types[i]}' for i in range(4)], fontsize=7)
ax2.set_ylabel('Value')
ax2.set_title('Supersingular vs Ordinary\n(CM curve y$^2$ = x$^3$ - x)', color=MAGENTA, fontsize=12, fontweight='bold')
ax2.grid(True, axis='y', alpha=0.3)

# Annotate: 3 supersingular, 1 ordinary
ax2.text(0.5, 0.95, '3 supersingular + 1 ordinary = 4 = $N_{base}$',
        transform=ax2.transAxes, fontsize=9, color=GOLD, ha='center', va='top',
        bbox=dict(boxstyle='round,pad=0.3', facecolor='#1a1a2e', edgecolor=GOLD, alpha=0.8))

# Panel 3: Partition function structure
ax3 = axes[2]
# P(x) mod 27 roots visualization as a complex plot
theta = np.linspace(0, 2*np.pi, 100)

# The roots of x^2 + 4x + 13 = 0 are -2 +/- 3i
roots_real = [1, 3, -2, -2]
roots_imag = [0, 0, 3, -3]
root_names = ['1', '$N_c$=3', '-2+3i', '-2-3i']
root_colors = [SKY, CYAN, MAGENTA, MAGENTA]

# Draw unit circle and |z|=sqrt(13) circle
ax3.plot(np.cos(theta), np.sin(theta), '--', color='#404060', alpha=0.5, linewidth=1)
ax3.plot(np.sqrt(13)*np.cos(theta), np.sqrt(13)*np.sin(theta), '--', color=CORAL, alpha=0.5,
         linewidth=1.5, label=f'|z| = $\\sqrt{{N_{{eff}}}}$ = {np.sqrt(13):.2f}')

for r, im, name, col in zip(roots_real, roots_imag, root_names, root_colors):
    ax3.scatter([r], [im], color=col, s=150, zorder=5, edgecolors=WHITE, linewidths=2)
    offset = (0.3, 0.3) if im >= 0 else (0.3, -0.5)
    ax3.annotate(name, xy=(r, im), xytext=(r+offset[0], im+offset[1]),
                fontsize=10, color=col, fontweight='bold')

ax3.axhline(y=0, color='#404060', linewidth=0.5)
ax3.axvline(x=0, color='#404060', linewidth=0.5)
ax3.set_xlabel('Re(z)')
ax3.set_ylabel('Im(z)')
ax3.set_title('Roots of P(x) mod 27\nin Complex Plane', color=CORAL, fontsize=12, fontweight='bold')
ax3.set_aspect('equal')
ax3.legend(fontsize=7, loc='upper left')
ax3.grid(True, alpha=0.3)
ax3.set_xlim(-5, 5)
ax3.set_ylim(-5, 5)

fig3.suptitle('Golden Ratio Skeleton: Lucas Numbers, CM Curve, and Root Structure',
              color=WHITE, fontsize=14, fontweight='bold', y=1.02)

plt.savefig('output/golden_ratio_skeleton.png', dpi=150, bbox_inches='tight',
            facecolor='#0a0a1a', edgecolor='none')
print("Saved: output/golden_ratio_skeleton.png")

# ===========================================================================
# FIGURE 4: Summary Dashboard
# ===========================================================================
fig4 = plt.figure(figsize=(16, 9))
gs4 = GridSpec(2, 4, figure=fig4, hspace=0.4, wspace=0.4)

# Big number panels
def big_number(ax, value, label, sublabel, color):
    ax.text(0.5, 0.6, value, transform=ax.transAxes, fontsize=28, fontweight='bold',
            color=color, ha='center', va='center', family='monospace')
    ax.text(0.5, 0.25, label, transform=ax.transAxes, fontsize=10,
            color='#c0c0c0', ha='center', va='center')
    ax.text(0.5, 0.08, sublabel, transform=ax.transAxes, fontsize=7,
            color='#808080', ha='center', va='center')
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis('off')

# Row 1: Key numbers
ax_n1 = fig4.add_subplot(gs4[0, 0])
big_number(ax_n1, '45.87', '$\\lambda_1/\\lambda_2$ (L=2)', 'Target: D=47 | 0.6% match', GOLD)

ax_n2 = fig4.add_subplot(gs4[0, 1])
big_number(ax_n2, '38.46', '$\\lambda_1/\\lambda_2$ (L=3)', 'Decreasing with L', MAGENTA)

ax_n3 = fig4.add_subplot(gs4[0, 2])
big_number(ax_n3, '~27', 'Extrapolated\n$\\lambda_1/\\lambda_2$', '$= N_c^3 = 3^3$', CYAN)

ax_n4 = fig4.add_subplot(gs4[0, 3])
big_number(ax_n4, '{25,-3,-7}', '128*G entries\n(L=2)', '$-3 = -N_c$, $-7 = -b_3$', LIME)

# Row 2: Convergence plot (wider)
ax_conv = fig4.add_subplot(gs4[1, :2])
ax_conv.plot([2, 3], [45.871, 38.459], 'o-', color=CYAN, markersize=12,
             linewidth=3, markeredgecolor=WHITE, markeredgewidth=2)
# Extrapolation
L_ext = np.array([2, 3, 4, 5, 6, 8, 10, 20, 50, 100])
# Linear in 1/L
ratio_ext = np.exp(gap_inf_lin + c_lin / L_ext) if 'gap_inf_lin' in dir() else np.exp(3.297 + 1.057 / L_ext)
gap_inf_lin_val = 3*np.log(38.458818) - 2*np.log(45.871034)
c_lin_val = 6*(np.log(45.871034) - np.log(38.458818))
ratio_ext = np.exp(gap_inf_lin_val + c_lin_val / L_ext)

ax_conv.plot(L_ext[2:], ratio_ext[2:], '--', color=LIME, alpha=0.6, linewidth=2, label='Linear extrapolation')
ax_conv.axhline(y=47, color=GOLD, linestyle=':', linewidth=2, alpha=0.8, label='D = 47')
ax_conv.axhline(y=27, color=SKY, linestyle=':', linewidth=1.5, alpha=0.6, label='$N_c^3$ = 27')
ax_conv.set_xlabel('Lattice size L', fontsize=11)
ax_conv.set_ylabel('$\\lambda_1 / \\lambda_2$', fontsize=12)
ax_conv.set_title('Spectral Ratio: Convergence to $N_c^3$, not D', color=CYAN, fontsize=13, fontweight='bold')
ax_conv.legend(fontsize=9)
ax_conv.grid(True, alpha=0.3)
ax_conv.set_xlim(1.5, 25)
ax_conv.set_ylim(20, 55)

# Verdict panel
ax_verdict = fig4.add_subplot(gs4[1, 2:])
verdict_text = (
    "CONFIRMED:\n"
    "  G(0,0) -> Watson BCC integral [THEOREM]\n"
    "  128*G has entries {-N_c, -b_3} [THEOREM]\n"
    "  P(x) mod 27 factorizes with\n"
    "    framework integers [THEOREM]\n"
    "  Degeneracy 4 = N_base [OBSERVATION]\n\n"
    "NOT CONFIRMED:\n"
    "  Spectral gap != ln(47) at L->inf\n"
    "  Near-47 at L=2 is finite-size effect\n"
    "  Extrapolation -> 27 = N_c^3\n\n"
    "STATUS: 3 theorems, 1 observation,\n"
    "  1 finite-size coincidence"
)
ax_verdict.text(0.05, 0.95, verdict_text, transform=ax_verdict.transAxes,
               fontsize=9, color='#c0c0c0', va='top', family='monospace',
               bbox=dict(boxstyle='round,pad=0.5', facecolor='#1a1a2e', edgecolor=GOLD, alpha=0.9))
ax_verdict.set_xlim(0, 1)
ax_verdict.set_ylim(0, 1)
ax_verdict.axis('off')
ax_verdict.set_title('Honest Verdict', color=GOLD, fontsize=13, fontweight='bold')

fig4.suptitle('Transfer Matrix on Ternary Torus: Complete Assessment',
              color=WHITE, fontsize=16, fontweight='bold', y=0.98)

plt.savefig('output/transfer_matrix_dashboard.png', dpi=150, bbox_inches='tight',
            facecolor='#0a0a1a', edgecolor='none')
print("Saved: output/transfer_matrix_dashboard.png")

print("\nAll 4 figures generated successfully.")
