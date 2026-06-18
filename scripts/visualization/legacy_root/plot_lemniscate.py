import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp

# Define G*
g_star = sp.gamma(0.25) / sp.gamma(0.75)

# Define varpi (lemniscate constant)
varpi = sp.gamma(0.25)**2 / (2 * np.sqrt(2 * np.pi))

# Perimeter of r^2 = a^2 cos(2*theta) is 2 * varpi * a
# We want perimeter = G*
a = 1.0 / np.sqrt(np.pi)

theta = np.linspace(-np.pi/4, np.pi/4, 1000)
r = a * np.sqrt(np.cos(2*theta))

x1 = r * np.cos(theta)
y1 = r * np.sin(theta)

x2 = -r * np.cos(theta)
y2 = -r * np.sin(theta)

plt.figure(figsize=(8, 4))
plt.plot(x1, y1, 'b-', linewidth=2)
plt.plot(x2, y2, 'b-', linewidth=2)
plt.fill_between(x1, y1, 0, color='blue', alpha=0.1)
plt.fill_between(x1, y1, y2, color='blue', alpha=0.1)
plt.fill_between(x2, y2, 0, color='blue', alpha=0.1)
plt.fill_between(x2, y2, y1, color='blue', alpha=0.1)
plt.title(f'Shape of G*: Lemniscate of Bernoulli scaled by a = 1/sqrt(pi)\nPerimeter = G* = {g_star:.5f}')
plt.axis('equal')
plt.grid(True, alpha=0.3)

# Save to artifacts dir
import os
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "lemniscate_g_star.png")
plt.savefig(output_path, dpi=300, bbox_inches='tight')
print(f'Plot saved to {output_path}')
