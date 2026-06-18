import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp

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

            # Case 1: a=0, |b| is a prime p = 3 (mod 4)
            if a == 0:
                if is_prime(abs(b)) and abs(b) % 4 == 3:
                    primes_x.append(a)
                    primes_y.append(b)
            # Case 2: b=0, |a| is a prime p = 3 (mod 4)
            elif b == 0:
                if is_prime(abs(a)) and abs(a) % 4 == 3:
                    primes_x.append(a)
                    primes_y.append(b)
            # Case 3: norm is a prime number (this covers p=2 and p = 1 (mod 4))
            else:
                if is_prime(norm):
                    primes_x.append(a)
                    primes_y.append(b)

    return primes_x, primes_y

limit = 100
px, py = get_gaussian_primes(limit)

plt.figure(figsize=(10, 10))

# Plot Gaussian primes
plt.scatter(px, py, s=1.5, color='black', alpha=0.8, label='Gaussian Primes')

# We can overlay the hyperbolas, but we need to scale them up to see them in this grid.
# Let's just plot the prime field first, or scale G* by an arbitrary factor to show the structural alignment.
# The user says the distribution *itself* creates the hyperbola shape.
# We'll plot the asymptotes to highlight the D4 symmetry.

plt.axhline(0, color='gray', linestyle=':', alpha=0.5)
plt.axvline(0, color='gray', linestyle=':', alpha=0.5)
plt.plot([-limit, limit], [-limit, limit], color='gray', linestyle=':', alpha=0.5)
plt.plot([-limit, limit], [limit, -limit], color='gray', linestyle=':', alpha=0.5)

plt.title(f'Gaussian Primes $\\mathbb{{Z}}[i]$ up to Re, Im $\\leq {limit}$', fontsize=16)
plt.xlim(-limit, limit)
plt.ylim(-limit, limit)
plt.gca().set_aspect('equal')
plt.axis('off') # Remove axis for cleaner structural view

# Save to artifacts dir
import os
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "gaussian_primes_hyperbola_shape.png")
plt.savefig(output_path, dpi=300, bbox_inches='tight', facecolor='white')
print(f'Plot saved to {output_path}')
