import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp

# Compute G*
g_star = sp.gamma(0.25) / sp.gamma(0.75)

# Generate ranges
x_pos = np.linspace(0.1, 6, 1000)
x_neg = np.linspace(-6, -0.1, 1000)

# 1. Quadrant Hyperbola: xy = G* -> y = G* / x
y_quad_pos = g_star / x_pos
y_quad_neg = g_star / x_neg

# 2. Einstein's Hyperbola (Spacetime interval): x^2 - y^2 = G*
# y = +/- sqrt(x^2 - G*)
# Real values exist only when x^2 >= G*, i.e., |x| >= sqrt(G*)
x_einstein = np.linspace(np.sqrt(g_star), 6, 1000)
y_ein_pos = np.sqrt(x_einstein**2 - g_star)
y_ein_neg = -np.sqrt(x_einstein**2 - g_star)

x_einstein_neg = np.linspace(-6, -np.sqrt(g_star), 1000)
y_ein_neg_pos = np.sqrt(x_einstein_neg**2 - g_star)
y_ein_neg_neg = -np.sqrt(x_einstein_neg**2 - g_star)

plt.figure(figsize=(8, 8))

# Plot Quadrant Hyperbola
plt.plot(x_pos, y_quad_pos, 'b-', linewidth=2, label=f'Quadrant Hyperbola ($xy = G^*$)')
plt.plot(x_neg, y_quad_neg, 'b-', linewidth=2)

# Plot Einstein's Hyperbola
plt.plot(x_einstein, y_ein_pos, 'r-', linewidth=2, label=f"Einstein's Hyperbola ($x^2 - y^2 = G^*$)")
plt.plot(x_einstein, y_ein_neg, 'r-', linewidth=2)
plt.plot(x_einstein_neg, y_ein_neg_pos, 'r-', linewidth=2)
plt.plot(x_einstein_neg, y_ein_neg_neg, 'r-', linewidth=2)

# Plot asymptotes for quadrant hyperbola (the axes)
plt.axhline(0, color='blue', linestyle=':', alpha=0.5)
plt.axvline(0, color='blue', linestyle=':', alpha=0.5)

# Plot asymptotes for Einstein's hyperbola (y = +/- x)
plt.plot([-6, 6], [-6, 6], color='red', linestyle=':', alpha=0.5)
plt.plot([-6, 6], [6, -6], color='red', linestyle=':', alpha=0.5)

# Formatting
plt.title(f'Comparison: G* Quadrant Hyperbola vs Einstein Hyperbola', fontsize=14)
plt.xlim(-6, 6)
plt.ylim(-6, 6)
plt.legend()
plt.grid(True, alpha=0.3)
plt.gca().set_aspect('equal')

# Save to artifacts dir
import os
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "hyperbola_comparison.png")
plt.savefig(output_path, dpi=300, bbox_inches='tight')
print(f'Plot saved to {output_path}')
