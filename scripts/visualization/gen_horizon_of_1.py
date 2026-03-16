from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.lines import Line2D

_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'

# Configure dark-mode plot style for high contrast
plt.style.use('dark_background')
fig = plt.figure(figsize=(16, 12))
fig.suptitle("The Horizon of 1: Inversion, Bifurcation, and Convergence",
             fontsize=20, fontweight='bold', y=0.96)

# ==============================================================================
# PANEL 1: Geometric Inversion (Hyperbola -> Unit Circle -> Lemniscate)
# ==============================================================================
ax1 = fig.add_subplot(221)
t = np.linspace(-np.pi/4 + 0.04, np.pi/4 - 0.04, 1000)

# Hyperbola (r > 1 regime, expansive, divergent)
r_hyp = np.sqrt(1 / np.cos(2*t))
x_hyp, y_hyp = r_hyp * np.cos(t), r_hyp * np.sin(t)

# Lemniscate (r < 1 regime, inverted, bounded)
r_lem = np.sqrt(np.cos(2*t))
x_lem, y_lem = r_lem * np.cos(t), r_lem * np.sin(t)

# Unit Circle (The invariant boundary r = 1)
theta = np.linspace(0, 2*np.pi, 500)
x_circ, y_circ = np.cos(theta), np.sin(theta)

ax1.plot(x_hyp, y_hyp, color='#00FFFF', lw=2, label=r'Hyperbola ($r > 1$, Divergent)')
ax1.plot(-x_hyp, -y_hyp, color='#00FFFF', lw=2)
ax1.plot(x_circ, y_circ, color='white', linestyle='--', lw=2, label=r'Unit Circle ($r = 1$, Boundary)')
ax1.plot(x_lem, y_lem, color='#FF3366', lw=3, label=r'Lemniscate ($r < 1$, Convergent)')
ax1.plot(-x_lem, -y_lem, color='#FF3366', lw=3)

ax1.set_xlim(-2.5, 2.5)
ax1.set_ylim(-2.5, 2.5)
ax1.set_aspect('equal')
ax1.set_title('1. Conformal Inversion ($r \\to 1/r$)', fontsize=14, pad=10)
ax1.legend(loc='upper right', facecolor='black', edgecolor='white')
ax1.grid(color='gray', linestyle=':', alpha=0.5)

# ==============================================================================
# PANEL 2: Topological Bifurcation (Cassini Ovals)
# ==============================================================================
ax2 = fig.add_subplot(222)
x = np.linspace(-2, 2, 400)
y = np.linspace(-1.5, 1.5, 400)
X, Y = np.meshgrid(x, y)

# Equation: |z - 1| * |z + 1| = R^2
Z = np.sqrt(((X - 1)**2 + Y**2) * ((X + 1)**2 + Y**2))

# R > 1 (Expansive / Unified Circle)
ax2.contour(X, Y, Z, levels=[1.2, 1.5], colors='#00FFFF', linewidths=2)
# R = 1 (The Exact Switch / Lemniscate Pinch)
ax2.contour(X, Y, Z, levels=[1.0], colors='#FFFFFF', linewidths=3)
# R < 1 (Reductive / Shattered Loops)
ax2.contour(X, Y, Z, levels=[0.4, 0.7], colors='#FF3366', linewidths=2)

# Foci (Complex Conjugate States)
ax2.plot([-1, 1], [0, 0], 'wo', markersize=6)

ax2.set_xlim(-2, 2)
ax2.set_ylim(-1.5, 1.5)
ax2.set_aspect('equal')
ax2.set_title('2. Cassini Ovals: The Topological Switch', fontsize=14, pad=10)

custom_lines = [Line2D([0], [0], color='#00FFFF', lw=2),
                Line2D([0], [0], color='white', lw=3),
                Line2D([0], [0], color='#FF3366', lw=2)]
ax2.legend(custom_lines, ['$R > 1$ (Unified Macro-State)', '$R = 1$ (The Lemniscate Pinch)', '$R < 1$ (Shattered Micro-States)'],
           loc='upper right', facecolor='black', edgecolor='white', fontsize=10)
ax2.grid(color='gray', linestyle=':', alpha=0.5)

# ==============================================================================
# PANEL 3: Iterative Dynamics (The 0.n Reductive Horizon)
# ==============================================================================
ax3 = fig.add_subplot(223)
steps = np.arange(20)

val_exp = 1.0 * (1.15)**steps  # 1.n multiplier
val_iso = 1.0 * (1.0)**steps   # 1.0 multiplier
val_red = 1.0 * (0.85)**steps  # 0.n multiplier

ax3.plot(steps, val_exp, marker='o', color='#00FFFF', lw=2, label=r'Expansive ($1.15^n$): Exploding Gradient')
ax3.plot(steps, val_iso, marker='s', color='white', lw=3, label=r'Isometry ($1.0^n$): Perfect Stability (ReLU)')
ax3.plot(steps, val_red, marker='^', color='#FF3366', lw=2, label=r'Reductive ($0.85^n$): Vanishing Gradient')

ax3.set_title("3. Iterative Multipliers (Algorithmic Gradients)", fontsize=14, pad=10)
ax3.set_xlabel("Iteration Step (Time / Network Depth)", fontsize=12)
ax3.set_ylabel("Signal Amplitude", fontsize=12)
ax3.legend(loc='upper left', facecolor='black', edgecolor='white')
ax3.grid(color='gray', linestyle=':', alpha=0.5)

# ==============================================================================
# PANEL 4: 3D Saddle Singularity (Phase Space Landscape)
# ==============================================================================
ax4 = fig.add_subplot(224, projection='3d')
X3, Y3 = np.meshgrid(np.linspace(-2, 2, 100), np.linspace(-2, 2, 100))
Z3 = np.sqrt(((X3 - 1)**2 + Y3**2) * ((X3 + 1)**2 + Y3**2))
Z3 = np.clip(Z3, 0, 3) # Cap height for clear visualization

# Plot the 3D surface
surf = ax4.plot_surface(X3, Y3, Z3, cmap='magma', alpha=0.7, edgecolor='none')
# Highlight the exact R=1 switch plane
ax4.contour(X3, Y3, Z3, levels=[1.0], colors='cyan', linewidths=3, offset=1.0)

ax4.set_title('4. The $R=1$ Saddle-Node Singularity', fontsize=14, pad=10)
ax4.set_zlim(0, 3)
ax4.view_init(elev=35, azim=45)

plt.tight_layout(rect=[0, 0.03, 1, 0.95])
plt.savefig(_FIGDIR / 'horizon_of_1.png', dpi=150, bbox_inches='tight', facecolor='black')
plt.show()
print(f"Saved to {_FIGDIR / 'horizon_of_1.png'}")
