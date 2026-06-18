import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp
import scipy.integrate as spi

# Define the integrands for Gamma(1/4) and Gamma(3/4)
def integrand_1_4(t):
    return t**(-0.75) * np.exp(-t)

def integrand_3_4(t):
    return t**(-0.25) * np.exp(-t)

# Compute the integrals
gamma_1_4, _ = spi.quad(integrand_1_4, 0, np.inf)
gamma_3_4, _ = spi.quad(integrand_3_4, 0, np.inf)
g_star = gamma_1_4 / gamma_3_4

# Create the plot
t = np.linspace(0.01, 5, 1000)
y1 = integrand_1_4(t)
y2 = integrand_3_4(t)

plt.figure(figsize=(10, 6))

plt.plot(t, y1, 'b-', label=r'$\Gamma(1/4)$ Integrand: $t^{-3/4}e^{-t}$', linewidth=2)
plt.fill_between(t, y1, 0, color='blue', alpha=0.1)

plt.plot(t, y2, 'r-', label=r'$\Gamma(3/4)$ Integrand: $t^{-1/4}e^{-t}$', linewidth=2)
plt.fill_between(t, y2, 0, color='red', alpha=0.1)

plt.title(f'Shape of G* Derived from Gamma Function Integrals\n'
          f'Area 1: $\\Gamma(1/4) \\approx {gamma_1_4:.4f}$\n'
          f'Area 2: $\\Gamma(3/4) \\approx {gamma_3_4:.4f}$\n'
          f'$G^* = \\frac{{\\text{{Area 1}}}}{{\\text{{Area 2}}}} = {g_star:.5f}$')

plt.xlabel('t')
plt.ylabel('Integrand Value')
plt.ylim(0, 10)
plt.legend()
plt.grid(True, alpha=0.3)

# Save to artifacts dir
import os
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "gamma_shape_g_star.png")
plt.savefig(output_path, dpi=300, bbox_inches='tight')
print(f'Plot saved to {output_path}')
