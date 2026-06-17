import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp

# We use the scale factor a = 1 / sqrt(pi) so the lemniscate perimeter is G*
a = 1.0 / np.sqrt(np.pi)

# 1. Lemniscate of Bernoulli
# r^2 = a^2 * cos(2*theta)
theta_lem = np.linspace(-np.pi/4 + 0.01, np.pi/4 - 0.01, 1000)
r_lem = a * np.sqrt(np.cos(2*theta_lem))

x_lem1 = r_lem * np.cos(theta_lem)
y_lem1 = r_lem * np.sin(theta_lem)

x_lem2 = -r_lem * np.cos(theta_lem)
y_lem2 = -r_lem * np.sin(theta_lem)

# 2. Geometric Inverse (Hyperbola: x^2 - y^2 = a^2)
# Inverse radius r_hyp = a^2 / r_lem
r_hyp = a**2 / r_lem

x_hyp1 = r_hyp * np.cos(theta_lem)
y_hyp1 = r_hyp * np.sin(theta_lem)

x_hyp2 = -r_hyp * np.cos(theta_lem)
y_hyp2 = -r_hyp * np.sin(theta_lem)

plt.figure(figsize=(8, 8))

# Plot Lemniscate
plt.plot(x_lem1, y_lem1, 'b-', linewidth=2, label='Lemniscate (r² = a² cos 2θ)')
plt.plot(x_lem2, y_lem2, 'b-', linewidth=2)
plt.fill_between(x_lem1, y_lem1, -y_lem1, color='blue', alpha=0.1)
plt.fill_between(x_lem2, y_lem2, -y_lem2, color='blue', alpha=0.1)

# Plot Hyperbolas
plt.plot(x_hyp1, y_hyp1, 'r-', linewidth=2, label='Inverse Hyperbola (x² - y² = a²)')
plt.plot(x_hyp1, -y_hyp1, 'r-', linewidth=2)
plt.plot(x_hyp2, y_hyp2, 'r-', linewidth=2)
plt.plot(x_hyp2, -y_hyp2, 'r-', linewidth=2)

# Plot Inversion Circle
circle = plt.Circle((0, 0), a, color='green', fill=False, linestyle='--', label='Inversion Circle (r = a)')
plt.gca().add_patch(circle)

# Formatting
plt.title("Geometric Inversion: Lemniscate to Einstein's Hyperbolas", fontsize=14)
plt.xlim(-2.5*a, 2.5*a)
plt.ylim(-2.5*a, 2.5*a)
plt.axhline(0, color='black', linewidth=0.5, alpha=0.5)
plt.axvline(0, color='black', linewidth=0.5, alpha=0.5)
plt.legend()
plt.grid(True, alpha=0.3)
plt.gca().set_aspect('equal')

# Save to artifacts dir
import os
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "lemniscate_hyperbola.png")
plt.savefig(output_path, dpi=300, bbox_inches='tight')
print(f'Plot saved to {output_path}')
