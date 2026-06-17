import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp

# Compute G*
g_star = sp.gamma(0.25) / sp.gamma(0.75)

# Generate ranges
x_pos = np.linspace(0.1, 8, 2000)
x_neg = np.linspace(-8, -0.1, 2000)

plt.figure(figsize=(10, 10))

# 1. Quadrant Hyperbola (0 deg): xy = G*
plt.plot(x_pos, g_star / x_pos, 'b-', linewidth=2, label=f'$xy = G^*$')
plt.plot(x_neg, g_star / x_neg, 'b-', linewidth=2)

# 2. Quadrant Hyperbola Orthogonal (90 deg): xy = -G*
plt.plot(x_pos, -g_star / x_pos, 'c-', linewidth=2, label=f'$xy = -G^*$')
plt.plot(x_neg, -g_star / x_neg, 'c-', linewidth=2)

# 3. Einstein Hyperbola (45 deg): x^2 - y^2 = G*
x_einstein = np.linspace(np.sqrt(g_star), 8, 2000)
plt.plot(x_einstein, np.sqrt(x_einstein**2 - g_star), 'r-', linewidth=2, label=f'$x^2 - y^2 = G^*$')
plt.plot(x_einstein, -np.sqrt(x_einstein**2 - g_star), 'r-', linewidth=2)
x_einstein_neg = np.linspace(-8, -np.sqrt(g_star), 2000)
plt.plot(x_einstein_neg, np.sqrt(x_einstein_neg**2 - g_star), 'r-', linewidth=2)
plt.plot(x_einstein_neg, -np.sqrt(x_einstein_neg**2 - g_star), 'r-', linewidth=2)

# 4. Einstein Hyperbola Orthogonal (135 deg): y^2 - x^2 = G*  => x^2 - y^2 = -G*
y_einstein2 = np.linspace(np.sqrt(g_star), 8, 2000)
plt.plot(np.sqrt(y_einstein2**2 - g_star), y_einstein2, 'm-', linewidth=2, label=f'$y^2 - x^2 = G^*$')
plt.plot(-np.sqrt(y_einstein2**2 - g_star), y_einstein2, 'm-', linewidth=2)
y_einstein2_neg = np.linspace(-8, -np.sqrt(g_star), 2000)
plt.plot(np.sqrt(y_einstein2_neg**2 - g_star), y_einstein2_neg, 'm-', linewidth=2)
plt.plot(-np.sqrt(y_einstein2_neg**2 - g_star), y_einstein2_neg, 'm-', linewidth=2)

# Plot asymptotes for all
plt.axhline(0, color='black', linestyle=':', alpha=0.5)
plt.axvline(0, color='black', linestyle=':', alpha=0.5)
plt.plot([-8, 8], [-8, 8], color='black', linestyle=':', alpha=0.5)
plt.plot([-8, 8], [8, -8], color='black', linestyle=':', alpha=0.5)

# Formatting
plt.title(f'All Orthogonal Rotations of G* Hyperbolas ($D_4$ Symmetry)', fontsize=16)
plt.xlim(-8, 8)
plt.ylim(-8, 8)
plt.legend(loc='upper right')
plt.grid(True, alpha=0.3)
plt.gca().set_aspect('equal')

# Save to artifacts dir
import os
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "orthogonal_hyperbolas.png")
plt.savefig(output_path, dpi=300, bbox_inches='tight')
print(f'Plot saved to {output_path}')
