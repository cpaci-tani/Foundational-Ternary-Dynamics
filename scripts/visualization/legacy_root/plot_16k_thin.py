import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp
import os
import time

print("Starting calculation...")
start_time = time.time()

R = 1200
N_max = R**2 * 2 + 1

print(f"Sieving primes up to {N_max}...")
sieve = np.ones(N_max, dtype=bool)
sieve[0] = sieve[1] = False
for i in range(2, int(np.sqrt(N_max)) + 1):
    if sieve[i]:
        sieve[i*i:N_max:i] = False

print("Finding Gaussian primes...")
primes_x = []
primes_y = []

for a in range(-R, R + 1):
    for b in range(-R, R + 1):
        if a == 0 and b == 0:
            continue

        norm = a*a + b*b

        if a == 0:
            if sieve[abs(b)] and abs(b) % 4 == 3:
                primes_x.append(a)
                primes_y.append(b)
        elif b == 0:
            if sieve[abs(a)] and abs(a) % 4 == 3:
                primes_x.append(a)
                primes_y.append(b)
        else:
            if sieve[norm]:
                primes_x.append(a)
                primes_y.append(b)

print(f"Found {len(primes_x)} Gaussian primes in {time.time() - start_time:.2f} seconds.")

g_star = sp.gamma(0.25) / sp.gamma(0.75)
c = g_star

print("Plotting at 16K resolution (1px lines, 1px dots)...")
# figsize=32, dpi=500 -> 16000 x 16000 pixels
plt.figure(figsize=(32, 32))
plt.style.use('dark_background')

# 1 pixel at 500 DPI is 72/500 = 0.144 points
# Linewidth in points:
lw = 0.15
# Scatter size (s) in points^2:
# Area of 1 pixel circle ~ 0.016 points^2
s_size = 0.02

# Plot primes strictly as solid dots OVER the lines (zorder=5)
plt.scatter(primes_x, primes_y, s=s_size, color='white', marker='o', edgecolors='none', zorder=5)

x_pos = np.linspace(0.01, R, 15000)
x_neg = np.linspace(-R, -0.01, 15000)

scales = [1, 50, 200, 500, 1000, 2000]

colors = {
    'quad': 'cyan',
    'quad_ortho': 'cyan',
    'einstein': 'magenta',
    'einstein_ortho': 'magenta'
}

for s in scales:
    cs = c * s * 10
    # Use the 1px line width for all scales
    current_lw = lw
    alpha = 0.8

    # Quadrant
    plt.plot(x_pos, cs / x_pos, color=colors['quad'], linewidth=current_lw, alpha=alpha, zorder=2)
    plt.plot(x_neg, cs / x_neg, color=colors['quad'], linewidth=current_lw, alpha=alpha, zorder=2)

    # Orthogonal Quadrant
    plt.plot(x_pos, -cs / x_pos, color=colors['quad_ortho'], linewidth=current_lw, alpha=alpha, zorder=2)
    plt.plot(x_neg, -cs / x_neg, color=colors['quad_ortho'], linewidth=current_lw, alpha=alpha, zorder=2)

    # Einstein
    x_ein = np.linspace(np.sqrt(cs), R, 15000)
    plt.plot(x_ein, np.sqrt(x_ein**2 - cs), color=colors['einstein'], linewidth=current_lw, alpha=alpha, zorder=2)
    plt.plot(x_ein, -np.sqrt(x_ein**2 - cs), color=colors['einstein'], linewidth=current_lw, alpha=alpha, zorder=2)
    x_ein_neg = np.linspace(-R, -np.sqrt(cs), 15000)
    plt.plot(x_ein_neg, np.sqrt(x_ein_neg**2 - cs), color=colors['einstein'], linewidth=current_lw, alpha=alpha, zorder=2)
    plt.plot(x_ein_neg, -np.sqrt(x_ein_neg**2 - cs), color=colors['einstein'], linewidth=current_lw, alpha=alpha, zorder=2)

    # Orthogonal Einstein
    y_ein = np.linspace(np.sqrt(cs), R, 15000)
    plt.plot(np.sqrt(y_ein**2 - cs), y_ein, color=colors['einstein_ortho'], linewidth=current_lw, alpha=alpha, zorder=2)
    plt.plot(-np.sqrt(y_ein**2 - cs), y_ein, color=colors['einstein_ortho'], linewidth=current_lw, alpha=alpha, zorder=2)
    y_ein_neg = np.linspace(-R, -np.sqrt(cs), 15000)
    plt.plot(np.sqrt(y_ein_neg**2 - cs), y_ein_neg, color=colors['einstein_ortho'], linewidth=current_lw, alpha=alpha, zorder=2)
    plt.plot(-np.sqrt(y_ein_neg**2 - cs), y_ein_neg, color=colors['einstein_ortho'], linewidth=current_lw, alpha=alpha, zorder=2)

plt.axhline(0, color='gray', linestyle='-', alpha=0.3, linewidth=lw, zorder=1)
plt.axvline(0, color='gray', linestyle='-', alpha=0.3, linewidth=lw, zorder=1)
plt.plot([-R, R], [-R, R], color='gray', linestyle='-', alpha=0.3, linewidth=lw, zorder=1)
plt.plot([-R, R], [R, -R], color='gray', linestyle='-', alpha=0.3, linewidth=lw, zorder=1)

# Keep the title text size appropriate for the 32x32 inch figure
plt.title(f'{len(primes_x)} Gaussian Primes over 1px $G^*$ Hyperbolas (16K UHD)', color='white', fontsize=60)
plt.xlim(-R, R)
plt.ylim(-R, R)
plt.gca().set_aspect('equal')
plt.axis('off')

# Save
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "gaussian_16k_uhd_thin.png")
print(f"Saving to {output_path}...")
plt.savefig(output_path, dpi=500, bbox_inches='tight', facecolor='black')
print(f'16K Thin Plot saved to {output_path}')
