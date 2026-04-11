"""
Five Minds Round 3 — The Systematic Sweep

Each mind's proposal after Round 2:

WIGNER:  "N=5 failed. Now sweep N=2..15 systematically. Plot angular
          concentration vs N. If Lie-algebra N's cluster HIGH and non-Lie N's
          cluster LOW, the Born rule is literally a gauge-group filter."

PLATO:   "Compute the angular autocorrelation C(theta) for N=3,4,5,6.
          Lie-algebra N's should show sharp delta-like peaks at root angles.
          N=5 should be smooth/featureless."

VON NEUMANN: "Treat each spectral pattern as a matrix. Compute singular values.
              Crystallographic N should have LOW effective rank (few dominant modes).
              Non-crystallographic N should be high-rank (no structure)."

EINSTEIN: "Compute ALL pairwise spectral products for N in {2,3,4,6}.
           Build a product table. Which products match which lattices?"

GROTHENDIECK: "Compute the fractal (box-counting) dimension of each spectral
               pattern. Primes should have higher dimension (more complex, irreducible)."
"""
import sys, os, numpy as np
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from experiments.detector_information_loss.field_engine import (
    setup_style, BG_COLOR, TEXT_COLOR, ACCENT_COLOR, GRID_COLOR
)
from matplotlib.patches import Patch
from matplotlib.lines import Line2D

plt = setup_style()
os.makedirs('output', exist_ok=True)

W, H = 512, 512
cx, cy = W//2, H//2
X, Y = np.meshgrid(np.arange(W), np.arange(H))

def mkp(px, py, kx, ky, s=12):
    r2 = (X-px)**2 + (Y-py)**2
    return np.exp(-r2/(2*s**2)) * np.exp(1j*(kx*(X-px) + ky*(Y-py)))

def get_spec(N, R=50):
    psi = sum(mkp(cx+R*np.cos(2*np.pi*i/N), cy+R*np.sin(2*np.pi*i/N),
                  -0.2*np.cos(2*np.pi*i/N), -0.2*np.sin(2*np.pi*i/N))
              for i in range(N))
    born = np.abs(psi)**2
    return np.abs(np.fft.fftshift(np.fft.fft2(born)))**2

def entropy(spec):
    pf = spec.flatten(); pf = pf[pf > 0]; pn = pf / pf.sum()
    return -np.sum(pn * np.log(pn + 1e-30))

def is_prime(n):
    if n < 2: return False
    if n < 4: return True
    if n % 2 == 0 or n % 3 == 0: return False
    i = 5
    while i*i <= n:
        if n % i == 0 or n % (i+2) == 0: return False
        i += 6
    return True

# Lie algebra dimensions that have simple Lie algebras:
# N=2: A1/SU(2), N=3: A2/SU(3), N=4: D2/SO(4)~SU(2)xSU(2), B2/SO(5), C2/Sp(4)
# N=6: A5/SU(6), D3/SO(6), N=8: D4/SO(8), A7/SU(8)
# Crystallographic: 1,2,3,4,6
lie_algebra_N = {2, 3, 4, 6, 8}
crystallographic = {2, 3, 4, 6}

# ================================================================
# Precompute all spectra for N=2..15
# ================================================================
print("Precomputing spectra for N=2..15...")
Ns = list(range(2, 16))
spectra = {}
for N in Ns:
    spectra[N] = get_spec(N)
    print(f"  N={N} done")

# ================================================================
# 1. WIGNER: Angular concentration sweep N=2..15
# ================================================================
print("\n1/5  WIGNER: Angular concentration sweep...")
fig, axes = plt.subplots(2, 1, figsize=(16, 12), gridspec_kw={'height_ratios': [2, 1]})
fig.suptitle('WIGNER: Angular Concentration vs N  --  Born Rule as Gauge Group Filter',
             color=ACCENT_COLOR, fontsize=15)

zz = 60
concentrations = []
for N in Ns:
    spec = spectra[N]
    region = spec[256-zz:256+zz, 256-zz:256+zz]
    thresh = 0.05 * region.max()
    py, px = np.where(region > thresh); px -= zz; py -= zz
    r = np.sqrt(px**2 + py**2); mask = (r > 3) & (r < 45)
    if mask.sum() > 10:
        angles = np.arctan2(py[mask], px[mask])
        weights = region[py[mask]+zz, px[mask]+zz]
        nbins = 36
        hist, _ = np.histogram(angles, bins=nbins, range=(-np.pi, np.pi), weights=weights)
        hist_norm = hist / hist.sum()
        ang_entropy = -np.sum(hist_norm[hist_norm > 0] * np.log(hist_norm[hist_norm > 0]))
        max_ent = np.log(nbins)
        concentrations.append(1 - ang_entropy / max_ent)
    else:
        concentrations.append(0)

ax = axes[0]
for i, N in enumerate(Ns):
    if N in crystallographic:
        clr, ms, marker = 'red', 14, 's'
    elif N in lie_algebra_N:
        clr, ms, marker = 'gold', 12, 'D'
    elif is_prime(N):
        clr, ms, marker = 'magenta', 10, '^'
    else:
        clr, ms, marker = 'cyan', 10, 'o'
    ax.plot(N, concentrations[i], marker, color=clr, ms=ms, zorder=5,
            markeredgecolor='white', markeredgewidth=0.5)
    ax.annotate(f'{concentrations[i]:.3f}', xy=(N, concentrations[i]),
                xytext=(0, 10), textcoords='offset points', fontsize=8,
                color=clr, ha='center')

ax.plot(Ns, concentrations, '-', color='gray', alpha=0.3, lw=1)
ax.axhline(np.mean([concentrations[Ns.index(n)] for n in [2,3,4,6]]),
           color='red', ls='--', alpha=0.5, label='Crystallographic mean')
ax.axhline(np.mean([concentrations[Ns.index(n)] for n in [5,7,9,11,13] if n in Ns]),
           color='magenta', ls='--', alpha=0.5, label='Non-cryst. prime mean')
ax.set_ylabel('Angular concentration (1 = perfect, 0 = isotropic)', color=TEXT_COLOR, fontsize=12)
ax.set_title('Does Each N Select a Gauge Group?', color=TEXT_COLOR, fontsize=13)
ax.legend(handles=[
    Line2D([0], [0], marker='s', color='red', ms=12, ls='', label='Crystallographic (2,3,4,6)'),
    Line2D([0], [0], marker='D', color='gold', ms=10, ls='', label='Lie algebra (8)'),
    Line2D([0], [0], marker='^', color='magenta', ms=10, ls='', label='Prime (non-Lie)'),
    Line2D([0], [0], marker='o', color='cyan', ms=10, ls='', label='Composite (non-cryst.)'),
], fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.grid(True, alpha=0.2)
ax.set_xticks(Ns)

# Bottom: bar chart sorted by concentration
ax2 = axes[1]
sorted_pairs = sorted(zip(Ns, concentrations), key=lambda x: -x[1])
sorted_Ns = [p[0] for p in sorted_pairs]
sorted_conc = [p[1] for p in sorted_pairs]
bar_colors = []
for n in sorted_Ns:
    if n in crystallographic: bar_colors.append('red')
    elif n in lie_algebra_N: bar_colors.append('gold')
    elif is_prime(n): bar_colors.append('magenta')
    else: bar_colors.append('cyan')
ax2.bar(range(len(sorted_Ns)), sorted_conc, color=bar_colors, edgecolor='none', width=0.7)
ax2.set_xticks(range(len(sorted_Ns)))
ax2.set_xticklabels([f'N={n}' for n in sorted_Ns], rotation=45, fontsize=9)
ax2.set_ylabel('Concentration (ranked)', color=TEXT_COLOR, fontsize=11)
ax2.set_title('Ranked: Which N Values Have the Strongest Angular Structure?', color=TEXT_COLOR, fontsize=12)
ax2.grid(True, alpha=0.2, axis='y')

plt.tight_layout()
plt.savefig('output/wigner_round3.png', dpi=150, facecolor=BG_COLOR)
print("   Saved wigner_round3.png")
plt.close()

# ================================================================
# 2. PLATO: Angular autocorrelation C(theta) for N=3,4,5,6
# ================================================================
print("2/5  PLATO: Angular autocorrelation...")
fig, axes = plt.subplots(2, 2, figsize=(16, 12))
fig.suptitle('PLATO: Angular Autocorrelation C(theta) -- Root Angles as Delta Peaks',
             color=ACCENT_COLOR, fontsize=15)

test_Ns = [3, 4, 5, 6]
root_angles = {
    3: [60, 120, 180, 240, 300],
    4: [90, 180, 270],
    5: [72, 144, 216, 288],
    6: [60, 120, 180, 240, 300],
}
clrs = {3: 'red', 4: 'lime', 5: 'magenta', 6: 'gold'}

for idx, N in enumerate(test_Ns):
    ax = axes[idx//2][idx%2]; ax.set_facecolor(BG_COLOR)
    spec = spectra[N]
    region = spec[256-zz:256+zz, 256-zz:256+zz]
    thresh = 0.05 * region.max()
    py, px = np.where(region > thresh); px -= zz; py -= zz
    r = np.sqrt(px**2 + py**2); mask = (r > 3) & (r < 45)

    if mask.sum() > 10:
        angles = np.arctan2(py[mask], px[mask])
        weights = region[py[mask]+zz, px[mask]+zz]
        # Build angular profile
        nbins = 180
        hist, bin_edges = np.histogram(angles, bins=nbins, range=(-np.pi, np.pi), weights=weights)
        hist = hist / hist.max()
        bin_centers = 0.5*(bin_edges[:-1] + bin_edges[1:]) * 180/np.pi

        # Autocorrelation
        autocorr = np.correlate(hist, hist, mode='full')
        autocorr = autocorr[len(autocorr)//2:]
        autocorr = autocorr / autocorr[0]
        lags = np.arange(len(autocorr)) * 360 / nbins

        ax.plot(lags, autocorr, '-', color=clrs[N], lw=2)
        ax.fill_between(lags, autocorr, alpha=0.15, color=clrs[N])

        # Mark expected root angles
        for ra in root_angles[N]:
            idx_ra = int(ra * nbins / 360)
            if idx_ra < len(autocorr):
                ax.axvline(ra, color='white', ls=':', alpha=0.5, lw=1)
                ax.plot(ra, autocorr[min(idx_ra, len(autocorr)-1)], 'v', color='white', ms=8, zorder=5)

        # Compute sharpness: peak-to-valley ratio
        peaks = autocorr[autocorr > 0.3]
        valleys = autocorr[(autocorr < 0.3) & (autocorr > 0)]
        if len(valleys) > 0:
            sharpness = np.mean(peaks) / np.mean(valleys) if np.mean(valleys) > 0 else 0
        else:
            sharpness = 0
        ax.text(0.95, 0.95, f'Sharpness: {sharpness:.1f}', transform=ax.transAxes,
                color=clrs[N], fontsize=11, va='top', ha='right',
                bbox=dict(boxstyle='round', facecolor=BG_COLOR, edgecolor=clrs[N], alpha=0.8))

    label = 'Lie algebra' if N != 5 else 'NO Lie algebra'
    ax.set_title(f'N={N} ({label})', color=clrs[N], fontsize=13,
                 fontweight='bold' if N == 5 else 'normal')
    ax.set_xlabel('Angular lag (degrees)', color=TEXT_COLOR, fontsize=10)
    ax.set_ylabel('Autocorrelation', color=TEXT_COLOR, fontsize=10)
    ax.set_xlim(0, 360)
    ax.grid(True, alpha=0.2)

plt.tight_layout()
plt.savefig('output/plato_round3.png', dpi=150, facecolor=BG_COLOR)
print("   Saved plato_round3.png")
plt.close()

# ================================================================
# 3. VON NEUMANN: Singular value spectrum / effective rank
# ================================================================
print("3/5  VON NEUMANN: Singular value analysis...")
fig, axes = plt.subplots(1, 2, figsize=(20, 8))
fig.suptitle('VON NEUMANN: Spectral Rank -- How Many Modes Define Each Pattern?',
             color=ACCENT_COLOR, fontsize=15)

ax = axes[0]
eff_ranks = []
for N in Ns:
    spec = spectra[N]
    region = spec[256-zz:256+zz, 256-zz:256+zz]
    region_norm = region / region.max()
    # SVD of the central spectral region
    U, s, Vt = np.linalg.svd(region_norm, full_matrices=False)
    s_norm = s / s.sum()
    # Effective rank (Shannon entropy of singular values)
    s_pos = s_norm[s_norm > 1e-10]
    eff_rank = np.exp(-np.sum(s_pos * np.log(s_pos)))
    eff_ranks.append(eff_rank)

    # Plot top 20 singular values for selected N
    if N in [3, 4, 5, 6, 7]:
        clr = 'red' if N in crystallographic else ('magenta' if is_prime(N) else 'cyan')
        ls = '-' if N in crystallographic else '--'
        ax.semilogy(range(1, 21), s[:20]/s[0], ls, color=clr, lw=2,
                    label=f'N={N} (rank={eff_rank:.1f})', alpha=0.8)

ax.set_xlabel('Singular value index', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Normalized singular value', color=TEXT_COLOR, fontsize=12)
ax.set_title('Top 20 Singular Values of Spectral Region', color=TEXT_COLOR, fontsize=13)
ax.legend(fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.grid(True, alpha=0.2)

# Right panel: effective rank vs N
ax2 = axes[1]
for i, N in enumerate(Ns):
    if N in crystallographic:
        clr, ms, marker = 'red', 14, 's'
    elif N in lie_algebra_N:
        clr, ms, marker = 'gold', 12, 'D'
    elif is_prime(N):
        clr, ms, marker = 'magenta', 10, '^'
    else:
        clr, ms, marker = 'cyan', 10, 'o'
    ax2.plot(N, eff_ranks[i], marker, color=clr, ms=ms, zorder=5,
             markeredgecolor='white', markeredgewidth=0.5)
ax2.plot(Ns, eff_ranks, '-', color='gray', alpha=0.3, lw=1)
ax2.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax2.set_ylabel('Effective rank (lower = more structured)', color=TEXT_COLOR, fontsize=12)
ax2.set_title('Spectral Complexity: Effective Rank', color=TEXT_COLOR, fontsize=13)
ax2.legend(handles=[
    Line2D([0], [0], marker='s', color='red', ms=12, ls='', label='Crystallographic'),
    Line2D([0], [0], marker='D', color='gold', ms=10, ls='', label='Lie algebra'),
    Line2D([0], [0], marker='^', color='magenta', ms=10, ls='', label='Prime (non-Lie)'),
    Line2D([0], [0], marker='o', color='cyan', ms=10, ls='', label='Composite'),
], fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax2.set_xticks(Ns)
ax2.grid(True, alpha=0.2)

plt.tight_layout()
plt.savefig('output/vonneumann_round3.png', dpi=150, facecolor=BG_COLOR)
print("   Saved vonneumann_round3.png")
plt.close()

# ================================================================
# 4. EINSTEIN: Full product table {2,3,4,6} x {2,3,4,6}
# ================================================================
print("4/5  EINSTEIN: Product table...")
product_Ns = [2, 3, 4, 6]
fig, axes = plt.subplots(4, 4, figsize=(20, 20))
fig.suptitle('EINSTEIN: Spectral Product Table -- Which Lattices Emerge?',
             color=ACCENT_COLOR, fontsize=16)

for row, Na in enumerate(product_Ns):
    for col, Nb in enumerate(product_Ns):
        ax = axes[row][col]; ax.set_facecolor(BG_COLOR)
        if Na == Nb:
            # Diagonal: just show the spectrum squared
            data = spectra[Na] ** 2
            label = f'N={Na} x N={Na}'
        else:
            data = spectra[Na] * spectra[Nb]
            label = f'N={Na} x N={Nb}'
        region = np.log10(data[256-zz:256+zz, 256-zz:256+zz] + 1e-30)
        ax.imshow(region, cmap='inferno', origin='lower', extent=[-zz, zz, -zz, zz])
        ax.set_title(label, color=TEXT_COLOR, fontsize=11)
        ax.set_xticks([]); ax.set_yticks([])

        # Add lattice type annotation based on symmetry
        gcd_N = np.gcd(Na, Nb) if Na != Nb else Na
        lcm_N = (Na * Nb) // gcd_N if Na != Nb else Na
        ax.text(0.05, 0.05, f'gcd={gcd_N}\nlcm={lcm_N}',
                transform=ax.transAxes, color='white', fontsize=8, va='bottom',
                bbox=dict(boxstyle='round', facecolor='black', alpha=0.6))

plt.tight_layout()
plt.savefig('output/einstein_round3.png', dpi=150, facecolor=BG_COLOR)
print("   Saved einstein_round3.png")
plt.close()

# ================================================================
# 5. GROTHENDIECK: Fractal dimension of spectral patterns
# ================================================================
print("5/5  GROTHENDIECK: Fractal dimension analysis...")
fig, axes = plt.subplots(1, 2, figsize=(20, 8))
fig.suptitle('GROTHENDIECK: Fractal Dimension of Spectral Patterns -- Arithmetic Complexity',
             color=ACCENT_COLOR, fontsize=15)

def box_count_dim(image, thresholds=[0.01, 0.02, 0.05]):
    """Estimate fractal dimension via box counting on thresholded image."""
    dims = []
    for thresh_frac in thresholds:
        binary = (image > thresh_frac * image.max()).astype(float)
        sizes = [4, 8, 16, 32, 64]
        counts = []
        for size in sizes:
            h, w = binary.shape
            count = 0
            for y in range(0, h, size):
                for x in range(0, w, size):
                    block = binary[y:y+size, x:x+size]
                    if block.sum() > 0:
                        count += 1
            counts.append(count)
        # Fit log(count) vs log(1/size)
        log_inv_size = np.log(1.0 / np.array(sizes))
        log_counts = np.log(np.array(counts) + 1)
        if len(log_inv_size) > 1:
            z = np.polyfit(log_inv_size, log_counts, 1)
            dims.append(z[0])
    return np.mean(dims)

frac_dims = []
for N in Ns:
    spec = spectra[N]
    region = spec[256-zz:256+zz, 256-zz:256+zz]
    fd = box_count_dim(region)
    frac_dims.append(fd)

ax = axes[0]
for i, N in enumerate(Ns):
    if N in crystallographic:
        clr, ms, marker = 'red', 14, 's'
    elif is_prime(N):
        clr, ms, marker = 'magenta', 12, 'D'
    else:
        clr, ms, marker = 'cyan', 10, 'o'
    ax.plot(N, frac_dims[i], marker, color=clr, ms=ms, zorder=5,
            markeredgecolor='white', markeredgewidth=0.5)
    ax.annotate(f'{frac_dims[i]:.2f}', xy=(N, frac_dims[i]),
                xytext=(0, 10), textcoords='offset points', fontsize=8,
                color=clr, ha='center')
ax.plot(Ns, frac_dims, '-', color='gray', alpha=0.3, lw=1)
ax.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Box-counting dimension', color=TEXT_COLOR, fontsize=12)
ax.set_title('Fractal Dimension of Spectral Pattern', color=TEXT_COLOR, fontsize=13)
ax.legend(handles=[
    Line2D([0], [0], marker='s', color='red', ms=12, ls='', label='Crystallographic'),
    Line2D([0], [0], marker='D', color='magenta', ms=10, ls='', label='Prime'),
    Line2D([0], [0], marker='o', color='cyan', ms=10, ls='', label='Composite'),
], fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.set_xticks(Ns)
ax.grid(True, alpha=0.2)

# Right: fractal dim vs angular concentration scatter
ax2 = axes[1]
for i, N in enumerate(Ns):
    if N in crystallographic:
        clr, ms, marker = 'red', 14, 's'
    elif is_prime(N):
        clr, ms, marker = 'magenta', 12, 'D'
    else:
        clr, ms, marker = 'cyan', 10, 'o'
    ax2.plot(concentrations[i], frac_dims[i], marker, color=clr, ms=ms, zorder=5,
             markeredgecolor='white', markeredgewidth=0.5)
    ax2.annotate(f'N={N}', xy=(concentrations[i], frac_dims[i]),
                 xytext=(5, 5), textcoords='offset points', fontsize=9, color=clr)

ax2.set_xlabel('Angular concentration (Wigner)', color=TEXT_COLOR, fontsize=12)
ax2.set_ylabel('Fractal dimension (Grothendieck)', color=TEXT_COLOR, fontsize=12)
ax2.set_title('Cross-Mind: Concentration vs Fractal Dimension', color=TEXT_COLOR, fontsize=13)
ax2.grid(True, alpha=0.2)

plt.tight_layout()
plt.savefig('output/grothendieck_round3.png', dpi=150, facecolor=BG_COLOR)
print("   Saved grothendieck_round3.png")
plt.close()

print("\nRound 3 complete -- 5 figures saved to output/")
