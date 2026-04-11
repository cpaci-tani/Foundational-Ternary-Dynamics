"""
Five Minds Round 4 — Cross-Mind Synthesis

The minds have seen each other's Round 3 results. Now they COMBINE.

WIGNER x VON NEUMANN: "Symmetry Quality Index" = concentration / effective_rank.
    Gauge groups should maximize this. Plot SQI vs N.

PLATO x WIGNER: Autocorrelation peak positions overlaid on a universal angle chart.
    Do the peaks trace out the Dynkin diagram structure?

VON NEUMANN x EINSTEIN: Cross-correlate the N=4xN=4 product with ideal BCC/FCC
    templates. Quantify which lattice is the best match for each product.

EINSTEIN x GROTHENDIECK: Is fractal dim of products additive or multiplicative?
    dim(NaxNb) vs dim(Na) + dim(Nb)?

GROTHENDIECK (grand synthesis): The "Periodic Table of Spectral N" — unified
    heatmap of ALL four measures across N=2..15.
"""
import sys, os, numpy as np
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from experiments.detector_information_loss.field_engine import (
    setup_style, BG_COLOR, TEXT_COLOR, ACCENT_COLOR, GRID_COLOR
)
from matplotlib.patches import Patch
from matplotlib.lines import Line2D
import matplotlib.gridspec as gridspec

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

crystallographic = {2, 3, 4, 6}
lie_algebra_N = {2, 3, 4, 6, 8}

# Precompute
print("Precomputing spectra for N=2..15...")
Ns = list(range(2, 16))
spectra = {}
for N in Ns:
    spectra[N] = get_spec(N)
zz = 60

# Compute all metrics for each N
print("Computing all metrics...")
metrics = {}
for N in Ns:
    spec = spectra[N]
    region = spec[256-zz:256+zz, 256-zz:256+zz]

    # Angular concentration
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
        concentration = 1 - ang_entropy / np.log(nbins)
    else:
        concentration = 0

    # Effective rank
    region_norm = region / region.max()
    U, s, Vt = np.linalg.svd(region_norm, full_matrices=False)
    s_norm = s / s.sum()
    s_pos = s_norm[s_norm > 1e-10]
    eff_rank = np.exp(-np.sum(s_pos * np.log(s_pos)))

    # Entropy gap (relative to neighbors)
    ent = entropy(spec)
    if N > 2 and N < 15:
        ent_prev = entropy(get_spec(N-1))
        ent_next = entropy(get_spec(N+1))
        gap = ent - (ent_prev + ent_next) / 2
    else:
        gap = 0

    # Peak count
    peak_count = int(np.sum(spec > 0.01 * spec.max()))

    metrics[N] = {
        'concentration': concentration,
        'eff_rank': eff_rank,
        'entropy': ent,
        'gap': gap,
        'peak_count': peak_count,
    }

# ================================================================
# 1. WIGNER x VON NEUMANN: Symmetry Quality Index
# ================================================================
print("\n1/5  WIGNER x VON NEUMANN: Symmetry Quality Index...")
fig, axes = plt.subplots(1, 3, figsize=(24, 7))
fig.suptitle('WIGNER x VON NEUMANN: Symmetry Quality Index = Concentration / Rank',
             color=ACCENT_COLOR, fontsize=15)

# SQI = concentration / log(eff_rank)
sqis = []
for N in Ns:
    m = metrics[N]
    sqi = m['concentration'] / (np.log(m['eff_rank']) + 0.1)
    sqis.append(sqi)

# Panel 1: SQI vs N
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
    ax.bar(N, sqis[i], color=clr, edgecolor='none', width=0.7, alpha=0.85)
ax.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('SQI = concentration / log(rank)', color=TEXT_COLOR, fontsize=12)
ax.set_title('Symmetry Quality Index', color=TEXT_COLOR, fontsize=13)
ax.set_xticks(Ns)
ax.grid(True, alpha=0.2, axis='y')

# Panel 2: concentration vs rank scatter
ax2 = axes[1]
for i, N in enumerate(Ns):
    m = metrics[N]
    if N in crystallographic:
        clr, ms, marker = 'red', 16, 's'
    elif N in lie_algebra_N:
        clr, ms, marker = 'gold', 14, 'D'
    elif is_prime(N):
        clr, ms, marker = 'magenta', 12, '^'
    else:
        clr, ms, marker = 'cyan', 12, 'o'
    ax2.plot(m['eff_rank'], m['concentration'], marker, color=clr, ms=ms, zorder=5,
             markeredgecolor='white', markeredgewidth=0.5)
    ax2.annotate(f'{N}', xy=(m['eff_rank'], m['concentration']),
                 xytext=(6, 3), textcoords='offset points', fontsize=10, color=clr)
ax2.set_xlabel('Effective rank (lower = simpler)', color=TEXT_COLOR, fontsize=12)
ax2.set_ylabel('Angular concentration (higher = more structured)', color=TEXT_COLOR, fontsize=12)
ax2.set_title('The Gauge Group Quadrant', color=TEXT_COLOR, fontsize=13)
# Draw quadrant lines at medians
med_rank = np.median([metrics[N]['eff_rank'] for N in Ns])
med_conc = np.median([metrics[N]['concentration'] for N in Ns])
ax2.axvline(med_rank, color='white', ls=':', alpha=0.3)
ax2.axhline(med_conc, color='white', ls=':', alpha=0.3)
ax2.text(0.02, 0.98, 'GAUGE\nGROUPS', transform=ax2.transAxes, color='lime', fontsize=14,
         fontweight='bold', va='top', alpha=0.5)
ax2.text(0.98, 0.02, 'DISORDERED', transform=ax2.transAxes, color='red', fontsize=14,
         fontweight='bold', va='bottom', ha='right', alpha=0.5)
ax2.grid(True, alpha=0.2)

# Panel 3: SQI ranked
ax3 = axes[2]
sorted_pairs = sorted(zip(Ns, sqis), key=lambda x: -x[1])
for rank_i, (N, sqi) in enumerate(sorted_pairs):
    if N in crystallographic:
        clr = 'red'
    elif N in lie_algebra_N:
        clr = 'gold'
    elif is_prime(N):
        clr = 'magenta'
    else:
        clr = 'cyan'
    ax3.barh(rank_i, sqi, color=clr, edgecolor='none', height=0.7, alpha=0.85)
    ax3.text(sqi + 0.001, rank_i, f'N={N}', color=clr, fontsize=10, va='center')
ax3.set_yticks([])
ax3.set_xlabel('SQI (higher = better gauge group candidate)', color=TEXT_COLOR, fontsize=12)
ax3.set_title('Ranked by Symmetry Quality', color=TEXT_COLOR, fontsize=13)
ax3.invert_yaxis()
ax3.grid(True, alpha=0.2, axis='x')

plt.tight_layout()
plt.savefig('output/wigner_vonneumann_round4.png', dpi=150, facecolor=BG_COLOR)
print("   Saved wigner_vonneumann_round4.png")
plt.close()

# ================================================================
# 2. PLATO x WIGNER: Angular peak positions vs Dynkin angles
# ================================================================
print("2/5  PLATO x WIGNER: Angular peak chart...")
fig, ax = plt.subplots(figsize=(16, 10))
fig.suptitle('PLATO x WIGNER: Angular Peak Positions -- Do They Match Root System Angles?',
             color=ACCENT_COLOR, fontsize=15)

test_Ns = [2, 3, 4, 5, 6, 7, 8]
clr_map = {2: 'cyan', 3: 'red', 4: 'lime', 5: 'magenta', 6: 'gold', 7: 'gray', 8: 'orange'}

# Expected root angles for each Lie algebra
expected_roots = {
    2: [0, 180],          # A1
    3: [0, 60, 120, 180, 240, 300],  # A2
    4: [0, 90, 180, 270],  # D2
    6: [0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330],  # A5 or G2
    8: [0, 45, 90, 135, 180, 225, 270, 315],  # D4
}

y_offset = 0
for N in test_Ns:
    spec = spectra[N]
    region = spec[256-zz:256+zz, 256-zz:256+zz]
    thresh = 0.05 * region.max()
    py, px = np.where(region > thresh); px -= zz; py -= zz
    r = np.sqrt(px**2 + py**2); mask = (r > 3) & (r < 45)

    if mask.sum() > 10:
        angles_rad = np.arctan2(py[mask], px[mask])
        weights = region[py[mask]+zz, px[mask]+zz]

        # Build angular profile
        nbins = 72
        hist, bin_edges = np.histogram(angles_rad, bins=nbins, range=(-np.pi, np.pi), weights=weights)
        bin_centers = 0.5*(bin_edges[:-1] + bin_edges[1:]) * 180/np.pi
        hist_norm = hist / hist.max()

        # Find peaks (local maxima)
        from scipy.signal import find_peaks
        peaks_idx, props = find_peaks(hist_norm, height=0.3, distance=3)
        peak_angles = bin_centers[peaks_idx]

        # Plot the angular profile as a thin line
        ax.fill_between(bin_centers, y_offset, y_offset + hist_norm * 0.8,
                         alpha=0.3, color=clr_map[N])
        ax.plot(bin_centers, y_offset + hist_norm * 0.8, '-', color=clr_map[N], lw=1)

        # Mark detected peaks
        for pa in peak_angles:
            ax.plot(pa, y_offset + hist_norm[np.argmin(np.abs(bin_centers - pa))] * 0.8,
                    'v', color=clr_map[N], ms=6, zorder=5)

        # Mark expected root angles
        if N in expected_roots:
            for era in expected_roots[N]:
                # Map to [-180, 180]
                era_mapped = era if era <= 180 else era - 360
                ax.axvline(era_mapped, ymin=(y_offset)/(len(test_Ns)+0.5),
                           ymax=(y_offset+1)/(len(test_Ns)+0.5),
                           color=clr_map[N], ls=':', alpha=0.4, lw=1)

    label = f'N={N}'
    if N in expected_roots:
        label += f' ({len(expected_roots[N])} roots)'
    else:
        label += ' (no Lie algebra)'
    ax.text(-185, y_offset + 0.4, label, color=clr_map[N], fontsize=11, va='center',
            fontweight='bold' if N in crystallographic else 'normal')

    y_offset += 1.0

ax.set_xlabel('Angle (degrees)', color=TEXT_COLOR, fontsize=12)
ax.set_xlim(-190, 190)
ax.set_ylim(-0.1, y_offset + 0.2)
ax.set_yticks([])
ax.grid(True, alpha=0.15, axis='x')

plt.tight_layout()
plt.savefig('output/plato_wigner_round4.png', dpi=150, facecolor=BG_COLOR)
print("   Saved plato_wigner_round4.png")
plt.close()

# ================================================================
# 3. VON NEUMANN x EINSTEIN: Template matching for products
# ================================================================
print("3/5  VON NEUMANN x EINSTEIN: Product template matching...")
fig, axes = plt.subplots(2, 3, figsize=(21, 14))
fig.suptitle('VON NEUMANN x EINSTEIN: How Well Do Products Match Ideal Lattices?',
             color=ACCENT_COLOR, fontsize=15)

# Generate ideal lattice templates
def make_lattice_template(lattice_type, size=120, spacing=18):
    template = np.zeros((size, size))
    half = size // 2
    if lattice_type == 'SC':  # Simple cubic
        for i in range(-10, 11):
            for j in range(-10, 11):
                x, y = half + i*spacing, half + j*spacing
                if 0 <= x < size and 0 <= y < size:
                    template[y, x] = 1.0
    elif lattice_type == 'BCC':
        for i in range(-10, 11):
            for j in range(-10, 11):
                x, y = half + i*spacing, half + j*spacing
                if 0 <= x < size and 0 <= y < size:
                    template[y, x] = 1.0
                x2, y2 = half + i*spacing + spacing//2, half + j*spacing + spacing//2
                if 0 <= x2 < size and 0 <= y2 < size:
                    template[y2, x2] = 0.8
    elif lattice_type == 'FCC':
        for i in range(-10, 11):
            for j in range(-10, 11):
                x, y = half + i*spacing, half + j*spacing
                if 0 <= x < size and 0 <= y < size:
                    template[y, x] = 1.0
                # Face centers
                x2, y2 = half + i*spacing + spacing//2, half + j*spacing
                if 0 <= x2 < size and 0 <= y2 < size:
                    template[y2, x2] = 0.8
                x3, y3 = half + i*spacing, half + j*spacing + spacing//2
                if 0 <= x3 < size and 0 <= y3 < size:
                    template[y3, x3] = 0.8
    elif lattice_type == 'HEX':
        for i in range(-10, 11):
            for j in range(-10, 11):
                x = half + i*spacing + (j % 2) * spacing//2
                y = half + int(j * spacing * np.sqrt(3)/2)
                if 0 <= x < size and 0 <= y < size:
                    template[y, x] = 1.0
    # Gaussian blur
    from scipy.ndimage import gaussian_filter
    template = gaussian_filter(template, sigma=2)
    return template / (template.max() + 1e-10)

products = [
    (2, 2, 'N=2 x N=2'), (2, 3, 'N=2 x N=3'), (2, 4, 'N=2 x N=4'),
    (3, 3, 'N=3 x N=3'), (3, 4, 'N=3 x N=4'), (4, 4, 'N=4 x N=4'),
]
lattice_types = ['SC', 'BCC', 'FCC', 'HEX']

# Compute cross-correlation scores
scores = {}
for Na, Nb, label in products:
    prod = spectra[Na] * spectra[Nb]
    region = prod[256-zz:256+zz, 256-zz:256+zz]
    region_norm = region / region.max()
    scores[label] = {}
    for lt in lattice_types:
        template = make_lattice_template(lt, size=2*zz, spacing=18)
        # Normalized cross-correlation
        corr = np.sum(region_norm * template) / (np.sqrt(np.sum(region_norm**2) * np.sum(template**2)) + 1e-10)
        scores[label][lt] = corr

# Plot as grouped bar chart
for idx, (Na, Nb, label) in enumerate(products):
    ax = axes[idx//3][idx%3]; ax.set_facecolor(BG_COLOR)
    vals = [scores[label][lt] for lt in lattice_types]
    colors = ['cyan', 'lime', 'gold', 'magenta']
    best = lattice_types[np.argmax(vals)]
    bars = ax.bar(lattice_types, vals, color=colors, edgecolor='none', width=0.6, alpha=0.85)
    # Highlight best
    best_idx = np.argmax(vals)
    bars[best_idx].set_edgecolor('white')
    bars[best_idx].set_linewidth(2)
    ax.set_title(f'{label} -> best: {best}', color=TEXT_COLOR, fontsize=12)
    ax.set_ylabel('Cross-correlation', color=TEXT_COLOR, fontsize=10)
    ax.grid(True, alpha=0.2, axis='y')
    ax.text(0.95, 0.95, f'{best}: {vals[best_idx]:.4f}',
            transform=ax.transAxes, color='white', fontsize=11, va='top', ha='right',
            fontweight='bold', bbox=dict(boxstyle='round', facecolor=colors[best_idx], alpha=0.3))

plt.tight_layout()
plt.savefig('output/vonneumann_einstein_round4.png', dpi=150, facecolor=BG_COLOR)
print("   Saved vonneumann_einstein_round4.png")
plt.close()

# ================================================================
# 4. EINSTEIN x GROTHENDIECK: Product fractal dimension
# ================================================================
print("4/5  EINSTEIN x GROTHENDIECK: Product fractal dimensions...")
fig, axes = plt.subplots(1, 2, figsize=(18, 8))
fig.suptitle('EINSTEIN x GROTHENDIECK: Is Spectral Complexity Additive Under Products?',
             color=ACCENT_COLOR, fontsize=15)

def box_count_dim(image, thresholds=[0.01, 0.02, 0.05]):
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
                    if binary[y:y+size, x:x+size].sum() > 0:
                        count += 1
            counts.append(count)
        log_inv = np.log(1.0 / np.array(sizes))
        log_c = np.log(np.array(counts) + 1)
        if len(log_inv) > 1:
            z = np.polyfit(log_inv, log_c, 1)
            dims.append(z[0])
    return np.mean(dims)

# Individual dims
individual_dims = {}
for N in [2, 3, 4, 6]:
    region = spectra[N][256-zz:256+zz, 256-zz:256+zz]
    individual_dims[N] = box_count_dim(region)

# Product dims
prod_results = []
product_pairs = [(2,2), (2,3), (2,4), (2,6), (3,3), (3,4), (3,6), (4,4), (4,6), (6,6)]
for Na, Nb in product_pairs:
    prod = spectra[Na] * spectra[Nb]
    region = prod[256-zz:256+zz, 256-zz:256+zz]
    prod_dim = box_count_dim(region)
    sum_dim = individual_dims[Na] + individual_dims[Nb]
    mean_dim = (individual_dims[Na] + individual_dims[Nb]) / 2
    prod_results.append((Na, Nb, prod_dim, sum_dim, mean_dim,
                         individual_dims[Na], individual_dims[Nb]))

ax = axes[0]
# Plot prod_dim vs mean_dim
for Na, Nb, pd, sd, md, da, db in prod_results:
    ax.plot(md, pd, 'o', color='cyan', ms=12, zorder=5, markeredgecolor='white')
    ax.annotate(f'{Na}x{Nb}', xy=(md, pd), xytext=(5, 5),
                textcoords='offset points', fontsize=9, color='white')
# Perfect correlation line
xlim = [min(r[4] for r in prod_results) - 0.05, max(r[4] for r in prod_results) + 0.05]
ax.plot(xlim, xlim, '--', color='lime', lw=2, alpha=0.5, label='Product dim = Mean(individual dims)')
ax.set_xlabel('Mean of individual dims: (d_A + d_B)/2', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Fractal dim of product', color=TEXT_COLOR, fontsize=12)
ax.set_title('Product Complexity vs Mean Input Complexity', color=TEXT_COLOR, fontsize=13)
ax.legend(fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.grid(True, alpha=0.2)

# Right: individual dims as reference
ax2 = axes[1]
for N in [2, 3, 4, 6]:
    ax2.bar(N, individual_dims[N], color='gold', edgecolor='none', width=0.6, alpha=0.85)
    ax2.text(N, individual_dims[N] + 0.01, f'{individual_dims[N]:.3f}',
             ha='center', color='gold', fontsize=11)
ax2.set_xlabel('N', color=TEXT_COLOR, fontsize=12)
ax2.set_ylabel('Fractal dimension', color=TEXT_COLOR, fontsize=12)
ax2.set_title('Individual Spectral Fractal Dimensions', color=TEXT_COLOR, fontsize=13)
ax2.set_xticks([2, 3, 4, 6])
ax2.grid(True, alpha=0.2, axis='y')

plt.tight_layout()
plt.savefig('output/einstein_grothendieck_round4.png', dpi=150, facecolor=BG_COLOR)
print("   Saved einstein_grothendieck_round4.png")
plt.close()

# ================================================================
# 5. GROTHENDIECK: The Periodic Table of Spectral N
# ================================================================
print("5/5  GROTHENDIECK: Periodic Table of Spectral N...")
fig = plt.figure(figsize=(20, 12))
gs = gridspec.GridSpec(2, 2, hspace=0.35, wspace=0.3)

# Collect all metrics into arrays
conc_arr = [metrics[N]['concentration'] for N in Ns]
rank_arr = [metrics[N]['eff_rank'] for N in Ns]
gap_arr = [metrics[N]['gap'] for N in Ns]
peak_arr = [metrics[N]['peak_count'] for N in Ns]

# Normalize each to [0,1]
def norm01(arr):
    a = np.array(arr)
    return (a - a.min()) / (a.max() - a.min() + 1e-10)

# Build the master matrix: rows = metrics, cols = N values
metric_names = ['Concentration\n(Wigner)', 'Low Rank\n(von Neumann)',
                'Entropy Gap\n(Plato)', 'Peak Count\n(Grothendieck)']
matrix = np.zeros((4, len(Ns)))
matrix[0] = norm01(conc_arr)                     # Higher = better
matrix[1] = norm01([-r for r in rank_arr])        # Lower rank = better (invert)
matrix[2] = norm01([-g for g in gap_arr])          # More negative gap = better (invert)
matrix[3] = norm01(peak_arr)                      # Higher = more complex

# Composite score (equal weight)
composite = matrix.mean(axis=0)

# Panel 1: The heatmap
ax1 = fig.add_subplot(gs[0, :])
im = ax1.imshow(matrix, aspect='auto', cmap='magma', interpolation='nearest')
ax1.set_xticks(range(len(Ns)))
ax1.set_xticklabels([f'N={n}' for n in Ns], fontsize=10)
ax1.set_yticks(range(4))
ax1.set_yticklabels(metric_names, fontsize=10)
ax1.set_title('The Periodic Table of Spectral N  (brighter = stronger)',
              color=ACCENT_COLOR, fontsize=14)

# Annotate cells
for i in range(4):
    for j in range(len(Ns)):
        val = matrix[i, j]
        clr = 'black' if val > 0.6 else 'white'
        ax1.text(j, i, f'{val:.2f}', ha='center', va='center', fontsize=8, color=clr)

# Color-code x-axis labels
for j, N in enumerate(Ns):
    if N in crystallographic:
        ax1.get_xticklabels()[j].set_color('red')
        ax1.get_xticklabels()[j].set_fontweight('bold')
    elif is_prime(N):
        ax1.get_xticklabels()[j].set_color('magenta')

plt.colorbar(im, ax=ax1, label='Normalized score', shrink=0.6)

# Panel 2: Composite score bar chart
ax2 = fig.add_subplot(gs[1, 0])
bar_colors = []
for n in Ns:
    if n in crystallographic: bar_colors.append('red')
    elif n in lie_algebra_N: bar_colors.append('gold')
    elif is_prime(n): bar_colors.append('magenta')
    else: bar_colors.append('cyan')
ax2.bar(Ns, composite, color=bar_colors, edgecolor='none', width=0.7, alpha=0.85)
ax2.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax2.set_ylabel('Composite Symmetry Score', color=TEXT_COLOR, fontsize=12)
ax2.set_title('Grand Unified Score (mean of 4 metrics)', color=TEXT_COLOR, fontsize=13)
ax2.set_xticks(Ns)
ax2.grid(True, alpha=0.2, axis='y')

# Annotate top 3
sorted_comp = sorted(zip(Ns, composite), key=lambda x: -x[1])
for rank_i, (N, score) in enumerate(sorted_comp[:3]):
    ax2.annotate(f'#{rank_i+1}', xy=(N, score), xytext=(0, 8),
                 textcoords='offset points', fontsize=12, color='lime',
                 fontweight='bold', ha='center')

# Panel 3: Radar chart for top 5
ax3 = fig.add_subplot(gs[1, 1], polar=True)
top5 = sorted_comp[:5]
angles_radar = np.linspace(0, 2*np.pi, 4, endpoint=False).tolist()
angles_radar += angles_radar[:1]  # close the polygon
radar_labels = ['Concentration', 'Low Rank', 'Entropy Gap', 'Peak Count']

for N, _ in top5:
    idx = Ns.index(N)
    vals = [matrix[m, idx] for m in range(4)]
    vals += vals[:1]
    if N in crystallographic:
        clr = 'red'
    elif is_prime(N):
        clr = 'magenta'
    else:
        clr = 'cyan'
    ax3.plot(angles_radar, vals, '-', color=clr, lw=2, label=f'N={N}', alpha=0.7)
    ax3.fill(angles_radar, vals, color=clr, alpha=0.05)

ax3.set_xticks(angles_radar[:-1])
ax3.set_xticklabels(radar_labels, fontsize=9, color=TEXT_COLOR)
ax3.set_title('Top 5 N: Radar Profile', color=TEXT_COLOR, fontsize=12, pad=20)
ax3.legend(fontsize=9, loc='lower right', facecolor=BG_COLOR,
           edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)

plt.savefig('output/periodic_table_round4.png', dpi=150, facecolor=BG_COLOR)
print("   Saved periodic_table_round4.png")
plt.close()

print("\nRound 4 complete -- 5 figures saved to output/")
