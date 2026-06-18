import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp
import os

R = 35

# Sieve for primes
N_max = R**2 * 2 + 1
sieve = np.ones(N_max, dtype=bool)
sieve[0] = sieve[1] = False
for i in range(2, int(np.sqrt(N_max)) + 1):
    if sieve[i]:
        sieve[i*i:N_max:i] = False

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

g_star = sp.gamma(0.25) / sp.gamma(0.75)

plt.figure(figsize=(12, 12))
plt.style.use('dark_background')

# Plot primes strictly as solid dots OVER the lines (zorder=5)
plt.scatter(primes_x, primes_y, s=12, color='white', marker='o', edgecolors='none', zorder=5)

x_pos = np.linspace(0.01, R, 2000)
x_neg = np.linspace(-R, -0.01, 2000)

colors = {
    'quad': 'cyan',
    'quad_ortho': 'cyan',
    'einstein': 'magenta',
    'einstein_ortho': 'magenta'
}

# The user was viewing the s=1 case from the 100k plot, which used cs = G* * 10
# Let's plot the true G* (cs=G*) and the cs=10*G* that framed the inner moat.
scales = [1, 10]

for s in scales:
    cs = g_star * s
    lw = 1.0  # Thin lines
    alpha = 0.8

    # Quadrant
    plt.plot(x_pos, cs / x_pos, color=colors['quad'], linewidth=lw, alpha=alpha, zorder=2)
    plt.plot(x_neg, cs / x_neg, color=colors['quad'], linewidth=lw, alpha=alpha, zorder=2)

    # Orthogonal Quadrant
    plt.plot(x_pos, -cs / x_pos, color=colors['quad_ortho'], linewidth=lw, alpha=alpha, zorder=2)
    plt.plot(x_neg, -cs / x_neg, color=colors['quad_ortho'], linewidth=lw, alpha=alpha, zorder=2)

    # Einstein
    x_ein = np.linspace(np.sqrt(cs), R, 2000)
    plt.plot(x_ein, np.sqrt(x_ein**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha, zorder=2)
    plt.plot(x_ein, -np.sqrt(x_ein**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha, zorder=2)
    x_ein_neg = np.linspace(-R, -np.sqrt(cs), 2000)
    plt.plot(x_ein_neg, np.sqrt(x_ein_neg**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha, zorder=2)
    plt.plot(x_ein_neg, -np.sqrt(x_ein_neg**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha, zorder=2)

    # Orthogonal Einstein
    y_ein = np.linspace(np.sqrt(cs), R, 2000)
    plt.plot(np.sqrt(y_ein**2 - cs), y_ein, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha, zorder=2)
    plt.plot(-np.sqrt(y_ein**2 - cs), y_ein, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha, zorder=2)
    y_ein_neg = np.linspace(-R, -np.sqrt(cs), 2000)
    plt.plot(np.sqrt(y_ein_neg**2 - cs), y_ein_neg, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha, zorder=2)
    plt.plot(-np.sqrt(y_ein_neg**2 - cs), y_ein_neg, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha, zorder=2)

# Asymptotes
plt.axhline(0, color='gray', linestyle='-', alpha=0.3, zorder=1)
plt.axvline(0, color='gray', linestyle='-', alpha=0.3, zorder=1)
plt.plot([-R, R], [-R, R], color='gray', linestyle='-', alpha=0.3, zorder=1)
plt.plot([-R, R], [R, -R], color='gray', linestyle='-', alpha=0.3, zorder=1)

plt.title(f'Exact Center: Gaussian Primes & Thin $G^*$ Hyperbolas', color='white', fontsize=18)
plt.xlim(-R, R)
plt.ylim(-R, R)
plt.gca().set_aspect('equal')
plt.axis('off')

artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "gaussian_center_thin.png")
plt.savefig(output_path, dpi=300, bbox_inches='tight', facecolor='black')
print(f'Plot saved to {output_path}')
