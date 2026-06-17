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

print("Plotting at 16K resolution...")
# figsize=32, dpi=500 -> 16000 x 16000 pixels
plt.figure(figsize=(32, 32))
plt.style.use('dark_background')

# Even smaller scatter size for extreme high resolution
plt.scatter(primes_x, primes_y, s=0.01, color='white', alpha=0.8)

x_pos = np.linspace(0.01, R, 10000)
x_neg = np.linspace(-R, -0.01, 10000)

scales = [1, 50, 200, 500, 1000, 2000]

colors = {
    'quad': 'cyan',
    'quad_ortho': 'cyan',
    'einstein': 'magenta',
    'einstein_ortho': 'magenta'
}

for s in scales:
    cs = c * s * 10 
    lw = 2.0 if s == 1 else 0.8
    alpha = 1.0 if s == 1 else 0.4
    
    plt.plot(x_pos, cs / x_pos, color=colors['quad'], linewidth=lw, alpha=alpha)
    plt.plot(x_neg, cs / x_neg, color=colors['quad'], linewidth=lw, alpha=alpha)
    
    plt.plot(x_pos, -cs / x_pos, color=colors['quad_ortho'], linewidth=lw, alpha=alpha)
    plt.plot(x_neg, -cs / x_neg, color=colors['quad_ortho'], linewidth=lw, alpha=alpha)
    
    x_ein = np.linspace(np.sqrt(cs), R, 10000)
    plt.plot(x_ein, np.sqrt(x_ein**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha)
    plt.plot(x_ein, -np.sqrt(x_ein**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha)
    x_ein_neg = np.linspace(-R, -np.sqrt(cs), 10000)
    plt.plot(x_ein_neg, np.sqrt(x_ein_neg**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha)
    plt.plot(x_ein_neg, -np.sqrt(x_ein_neg**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha)
    
    y_ein = np.linspace(np.sqrt(cs), R, 10000)
    plt.plot(np.sqrt(y_ein**2 - cs), y_ein, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha)
    plt.plot(-np.sqrt(y_ein**2 - cs), y_ein, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha)
    y_ein_neg = np.linspace(-R, -np.sqrt(cs), 10000)
    plt.plot(np.sqrt(y_ein_neg**2 - cs), y_ein_neg, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha)
    plt.plot(-np.sqrt(y_ein_neg**2 - cs), y_ein_neg, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha)

plt.axhline(0, color='gray', linestyle='-', alpha=0.3, linewidth=1)
plt.axvline(0, color='gray', linestyle='-', alpha=0.3, linewidth=1)
plt.plot([-R, R], [-R, R], color='gray', linestyle='-', alpha=0.3, linewidth=1)
plt.plot([-R, R], [R, -R], color='gray', linestyle='-', alpha=0.3, linewidth=1)

plt.title(f'{len(primes_x)} Gaussian Primes overlaid with $G^*$ Hyperbola macro-structures (16K UHD)', color='white', fontsize=60)
plt.xlim(-R, R)
plt.ylim(-R, R)
plt.gca().set_aspect('equal')
plt.axis('off')

# Save
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "gaussian_16k_uhd.png")
print(f"Saving to {output_path}...")
plt.savefig(output_path, dpi=500, bbox_inches='tight', facecolor='black')
print(f'16K Plot saved to {output_path}')
