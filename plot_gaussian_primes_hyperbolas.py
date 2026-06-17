import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp
import os

def is_prime(n):
    if n <= 1: return False
    if n <= 3: return True
    if n % 2 == 0 or n % 3 == 0: return False
    i = 5
    while i * i <= n:
        if n % i == 0 or n % (i + 2) == 0:
            return False
        i += 6
    return True

def get_gaussian_primes(limit):
    primes_x = []
    primes_y = []
    
    for a in range(-limit, limit + 1):
        for b in range(-limit, limit + 1):
            if a == 0 and b == 0:
                continue
                
            norm = a**2 + b**2
            
            if a == 0:
                if is_prime(abs(b)) and abs(b) % 4 == 3:
                    primes_x.append(a)
                    primes_y.append(b)
            elif b == 0:
                if is_prime(abs(a)) and abs(a) % 4 == 3:
                    primes_x.append(a)
                    primes_y.append(b)
            else:
                if is_prime(norm):
                    primes_x.append(a)
                    primes_y.append(b)
                    
    return primes_x, primes_y

# Set limits and compute G*
limit = 30
g_star = sp.gamma(0.25) / sp.gamma(0.75)
# Use a scale factor so it's visible if needed, or just raw G*. The user explicitly asked to plot xy=G*.
# The raw G* is ~2.95, which is small. We'll plot the raw equations but zoom in to limit=30.
scale = 1.0 # raw G*
c = g_star * scale

px, py = get_gaussian_primes(limit)

plt.figure(figsize=(10, 10))

# Plot Gaussian primes
plt.scatter(px, py, s=15, color='black', alpha=0.8, label='Gaussian Primes', zorder=2)

# Generate ranges for hyperbolas
x_pos = np.linspace(0.01, limit, 2000)
x_neg = np.linspace(-limit, -0.01, 2000)

# 1. Quadrant Hyperbola: xy = G*
plt.plot(x_pos, c / x_pos, 'b-', linewidth=2.5, label=f'$xy = G^*$ (Blue)', zorder=3)
plt.plot(x_neg, c / x_neg, 'b-', linewidth=2.5, zorder=3)

# 2. Orthogonal Quadrant Hyperbola: xy = -G*
plt.plot(x_pos, -c / x_pos, 'c-', linewidth=2.5, label=f'$xy = -G^*$ (Cyan)', zorder=3)
plt.plot(x_neg, -c / x_neg, 'c-', linewidth=2.5, zorder=3)

# 3. Einstein Hyperbola: x^2 - y^2 = G*
x_ein = np.linspace(np.sqrt(c), limit, 2000)
plt.plot(x_ein, np.sqrt(x_ein**2 - c), 'r-', linewidth=2.5, label=f'$x^2 - y^2 = G^*$ (Red)', zorder=3)
plt.plot(x_ein, -np.sqrt(x_ein**2 - c), 'r-', linewidth=2.5, zorder=3)
x_ein_neg = np.linspace(-limit, -np.sqrt(c), 2000)
plt.plot(x_ein_neg, np.sqrt(x_ein_neg**2 - c), 'r-', linewidth=2.5, zorder=3)
plt.plot(x_ein_neg, -np.sqrt(x_ein_neg**2 - c), 'r-', linewidth=2.5, zorder=3)

# 4. Orthogonal Einstein Hyperbola: y^2 - x^2 = G*
y_ein = np.linspace(np.sqrt(c), limit, 2000)
plt.plot(np.sqrt(y_ein**2 - c), y_ein, 'm-', linewidth=2.5, label=f'$y^2 - x^2 = G^*$ (Magenta)', zorder=3)
plt.plot(-np.sqrt(y_ein**2 - c), y_ein, 'm-', linewidth=2.5, zorder=3)
y_ein_neg = np.linspace(-limit, -np.sqrt(c), 2000)
plt.plot(np.sqrt(y_ein_neg**2 - c), y_ein_neg, 'm-', linewidth=2.5, zorder=3)
plt.plot(-np.sqrt(y_ein_neg**2 - c), y_ein_neg, 'm-', linewidth=2.5, zorder=3)

# Asymptotes
plt.axhline(0, color='gray', linestyle=':', alpha=0.5, zorder=1)
plt.axvline(0, color='gray', linestyle=':', alpha=0.5, zorder=1)
plt.plot([-limit, limit], [-limit, limit], color='gray', linestyle=':', alpha=0.5, zorder=1)
plt.plot([-limit, limit], [limit, -limit], color='gray', linestyle=':', alpha=0.5, zorder=1)

# Formatting
plt.title(f'Gaussian Primes $\mathbb{{Z}}[i]$ overlaid with $G^*$ Hyperbolas', fontsize=16)
plt.xlim(-limit, limit)
plt.ylim(-limit, limit)
plt.legend(loc='upper right', framealpha=0.9)
plt.gca().set_aspect('equal')
plt.grid(True, alpha=0.2)

# Save
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "gaussian_primes_hyperbolas_overlaid.png")
plt.savefig(output_path, dpi=300, bbox_inches='tight', facecolor='white')
print(f'Plot saved to {output_path}')
