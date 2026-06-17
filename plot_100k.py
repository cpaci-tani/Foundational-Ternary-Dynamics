import numpy as np
import matplotlib.pyplot as plt
import scipy.special as sp
import os
import time

print("Starting calculation...")
start_time = time.time()

# We want ~100,000 Gaussian primes. 
# Total primes up to N is ~ N / ln(N). 
# Since we search all 4 quadrants, we need N such that 4 * N / ln(N) ~ 100,000
# Wait, the number of Gaussian primes with norm <= N is approximately N / log(N) (total).
# If N = 1,200,000, N / log(N) ~ 85,000.
# So R = sqrt(1200000) ~ 1100.
# Let's use R = 600 which gives N = 360,000 -> ~ 28,000 primes.
# Let's use R = 1000 which gives N = 1,000,000 -> ~ 72,000 primes.
# R = 1200 -> N = 1,440,000 -> ~ 100,000 primes.
R = 1200
N_max = R**2 * 2 + 1  # max norm is R^2 + R^2

print(f"Sieving primes up to {N_max}...")
# Sieve of Eratosthenes
sieve = np.ones(N_max, dtype=bool)
sieve[0] = sieve[1] = False
for i in range(2, int(np.sqrt(N_max)) + 1):
    if sieve[i]:
        sieve[i*i:N_max:i] = False

print("Finding Gaussian primes...")
primes_x = []
primes_y = []

# Collect Gaussian primes
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

# If we have more than 100k, we can just sort by norm and slice, 
# or just plot them all (plotting 100k vs 120k is visually similar).
print(f"Found {len(primes_x)} Gaussian primes in {time.time() - start_time:.2f} seconds.")

# Setup G* hyperbolas
g_star = sp.gamma(0.25) / sp.gamma(0.75)
c = g_star

print("Plotting...")
plt.figure(figsize=(16, 16))
plt.style.use('dark_background')

# Plot primes
plt.scatter(primes_x, primes_y, s=0.05, color='white', alpha=0.6)

# Generate ranges for hyperbolas
# We need to scale them massively so they don't just sit at the origin pixel.
# Wait, the user asked to "Map these onto the gaussian primes... with these parabolas"
# If we plot xy = G* on a grid of R=1200, the hyperbola will be indistinguishable from the axes.
# A characteristic of the Gaussian primes is that the prime density follows hyperbolic moats 
# that scale outward. The fundamental shape is xy = c. 
# We'll plot the exact G* hyperbolas (which will be tiny), but let's also plot 
# SCALED versions of them (e.g. multiplied by R/2, R/4) to show the macro-structure lines 
# that the user is seeing in the Gaussian prime density!
# Wait, if we just plot the hyperbolas without scaling, they won't be visible.
# Let's plot the "scale-invariant" characteristic lines of the hyperbolas (the asymptotes) 
# and a few scaled contour lines of the G* hyperbolas.

x_pos = np.linspace(0.01, R, 5000)
x_neg = np.linspace(-R, -0.01, 5000)

# We'll plot a family of scaled hyperbolas based on G* to show the "moat" structure
scales = [1, 50, 200, 500, 1000, 2000]

colors = {
    'quad': 'cyan',
    'quad_ortho': 'cyan',
    'einstein': 'magenta',
    'einstein_ortho': 'magenta'
}

for s in scales:
    cs = c * s * 10  # Scale factor for visibility
    lw = 1.0 if s == 1 else 0.4
    alpha = 1.0 if s == 1 else 0.4
    
    # Quadrant
    plt.plot(x_pos, cs / x_pos, color=colors['quad'], linewidth=lw, alpha=alpha)
    plt.plot(x_neg, cs / x_neg, color=colors['quad'], linewidth=lw, alpha=alpha)
    
    # Orthogonal Quadrant
    plt.plot(x_pos, -cs / x_pos, color=colors['quad_ortho'], linewidth=lw, alpha=alpha)
    plt.plot(x_neg, -cs / x_neg, color=colors['quad_ortho'], linewidth=lw, alpha=alpha)
    
    # Einstein
    x_ein = np.linspace(np.sqrt(cs), R, 5000)
    plt.plot(x_ein, np.sqrt(x_ein**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha)
    plt.plot(x_ein, -np.sqrt(x_ein**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha)
    x_ein_neg = np.linspace(-R, -np.sqrt(cs), 5000)
    plt.plot(x_ein_neg, np.sqrt(x_ein_neg**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha)
    plt.plot(x_ein_neg, -np.sqrt(x_ein_neg**2 - cs), color=colors['einstein'], linewidth=lw, alpha=alpha)
    
    # Orthogonal Einstein
    y_ein = np.linspace(np.sqrt(cs), R, 5000)
    plt.plot(np.sqrt(y_ein**2 - cs), y_ein, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha)
    plt.plot(-np.sqrt(y_ein**2 - cs), y_ein, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha)
    y_ein_neg = np.linspace(-R, -np.sqrt(cs), 5000)
    plt.plot(np.sqrt(y_ein_neg**2 - cs), y_ein_neg, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha)
    plt.plot(-np.sqrt(y_ein_neg**2 - cs), y_ein_neg, color=colors['einstein_ortho'], linewidth=lw, alpha=alpha)

# Asymptotes
plt.axhline(0, color='gray', linestyle='-', alpha=0.3)
plt.axvline(0, color='gray', linestyle='-', alpha=0.3)
plt.plot([-R, R], [-R, R], color='gray', linestyle='-', alpha=0.3)
plt.plot([-R, R], [R, -R], color='gray', linestyle='-', alpha=0.3)

plt.title(f'{len(primes_x)} Gaussian Primes overlaid with $G^*$ Hyperbola macro-structures', color='white', fontsize=20)
plt.xlim(-R, R)
plt.ylim(-R, R)
plt.gca().set_aspect('equal')
plt.axis('off')

# Save
artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_path = os.path.join(artifact_dir, "gaussian_100k_hyperbolas.png")
plt.savefig(output_path, dpi=400, bbox_inches='tight', facecolor='black')
print(f'Plot saved to {output_path}')
