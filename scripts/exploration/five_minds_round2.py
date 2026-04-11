"""Five Minds Round 2 — Follow-up computations requested by each mind."""
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

def get_spec(N, R=50, res=512):
    if res != 512:
        xg = np.arange(res); yg = np.arange(res)
        Xg, Yg = np.meshgrid(xg, yg)
        cxg, cyg = res//2, res//2
        Rg = res//10
        psi = np.zeros((res, res), dtype=complex)
        for i in range(N):
            a = 2*np.pi*i/N
            r2 = (Xg-(cxg+Rg*np.cos(a)))**2 + (Yg-(cyg+Rg*np.sin(a)))**2
            psi += np.exp(-r2/(2*8**2))*np.exp(1j*(-0.2*np.cos(a)*(Xg-cxg-Rg*np.cos(a))))
        born = np.abs(psi)**2
        return np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
    psi = sum(mkp(cx+R*np.cos(2*np.pi*i/N), cy+R*np.sin(2*np.pi*i/N),
                  -0.2*np.cos(2*np.pi*i/N), -0.2*np.sin(2*np.pi*i/N))
              for i in range(N))
    born = np.abs(psi)**2
    return np.abs(np.fft.fftshift(np.fft.fft2(born)))**2

def entropy(spec):
    pf = spec.flatten(); pf = pf[pf > 0]; pn = pf / pf.sum()
    return -np.sum(pn * np.log(pn + 1e-30))

# ================================================================
# 1. PLATO: Extended entropy gap — include N=24, 60
# ================================================================
print("1/5  PLATO: Extended entropy gap (N=2..65)...")
fig, axes = plt.subplots(2, 1, figsize=(16, 10), gridspec_kw={'height_ratios': [2, 1]})
fig.suptitle('PLATO: Spectral Entropy Gap — Extended to Exceptional Symmetries',
             color=ACCENT_COLOR, fontsize=15)

Ns = list(range(2, 66))
ents = [entropy(get_spec(N)) for N in Ns]
gaps = [ents[i] - (ents[i-1] + ents[i+1])/2 for i in range(1, len(ents)-1)]
gap_Ns = list(range(3, 65))

platonic = [4, 6, 8, 12, 20]
exceptional = [24, 60]

ax = axes[0]
colors = []
for n in gap_Ns:
    if n in exceptional:
        colors.append('lime')
    elif n in platonic:
        colors.append('gold')
    else:
        colors.append('cyan')
ax.bar(gap_Ns, gaps, color=colors, edgecolor='none', width=0.7, alpha=0.85)
ax.axhline(0, color='white', ls='--', alpha=0.4)
for n in platonic:
    if n in gap_Ns:
        idx = gap_Ns.index(n)
        ax.annotate(f'N={n}', xy=(n, gaps[idx]), xytext=(0, -18 if gaps[idx] < 0 else 12),
                    textcoords='offset points', color='gold', fontsize=9, fontweight='bold', ha='center')
for n in exceptional:
    if n in gap_Ns:
        idx = gap_Ns.index(n)
        ax.annotate(f'N={n}', xy=(n, gaps[idx]), xytext=(0, -18 if gaps[idx] < 0 else 12),
                    textcoords='offset points', color='lime', fontsize=11, fontweight='bold', ha='center')
ax.set_ylabel('Entropy gap', color=TEXT_COLOR, fontsize=12)
ax.legend(handles=[Patch(facecolor='gold', label='Platonic (4,6,8,12,20)'),
                   Patch(facecolor='lime', label='Exceptional (24,60)'),
                   Patch(facecolor='cyan', label='Other N')],
          fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.grid(True, alpha=0.2, axis='y')

# Bottom panel: running entropy
ax2 = axes[1]
ax2.plot(Ns, ents, '-', color='cyan', lw=1.5)
for n in platonic:
    idx = Ns.index(n)
    ax2.plot(n, ents[idx], 'o', color='gold', ms=8, zorder=5)
for n in exceptional:
    idx = Ns.index(n)
    ax2.plot(n, ents[idx], 's', color='lime', ms=10, zorder=5)
ax2.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax2.set_ylabel('Spectral entropy', color=TEXT_COLOR, fontsize=12)
ax2.grid(True, alpha=0.2)
plt.tight_layout()
plt.savefig('output/plato_round2.png', dpi=150, facecolor=BG_COLOR)
print("   Saved plato_round2.png")
plt.close()

# ================================================================
# 2. WIGNER: N=5 test — no simple Lie algebra has 5-fold symmetry
# ================================================================
print("2/5  WIGNER: N=5 falsification test...")
fig, axes = plt.subplots(1, 4, figsize=(28, 7))
fig.suptitle('WIGNER: Does N=5 Break the Pattern? (No Simple Lie Algebra with 5-fold Symmetry)',
             color=ACCENT_COLOR, fontsize=16)

zz = 60
root_data = {
    2: ('A1 / SU(2)', [0, 180], 'cyan'),
    3: ('A2 / SU(3)', [0, 60, 120, 180, 240, 300], 'red'),
    4: ('D2 / SO(4)', [0, 90, 180, 270], 'lime'),
    5: ('??? / NO LIE ALGEBRA', [0, 72, 144, 216, 288], 'magenta'),
}

for col, N in enumerate([2, 3, 4, 5]):
    ax = axes[col]; ax.set_facecolor(BG_COLOR)
    spec = get_spec(N)
    region = spec[256-zz:256+zz, 256-zz:256+zz]
    thresh = 0.05 * region.max()
    py, px = np.where(region > thresh); px -= zz; py -= zz
    r = np.sqrt(px**2 + py**2); mask = (r > 3) & (r < 45)
    if mask.sum() > 0:
        vals = np.log10(region[py[mask]+zz, px[mask]+zz] + 1e-30)
        ax.scatter(px[mask], py[mask], c=vals, cmap='hot', s=10, alpha=0.6, zorder=3)

    name, angs, clr = root_data[N]
    for a in angs:
        rad = np.radians(a)
        ax.plot([0, 35*np.cos(rad)], [0, 35*np.sin(rad)], '--', color=clr, lw=2, alpha=0.8)
        ax.plot(35*np.cos(rad), 35*np.sin(rad), 'o', color=clr, ms=8, zorder=5)
    ax.set_xlim(-50, 50); ax.set_ylim(-50, 50); ax.set_aspect('equal')
    border_color = 'red' if N == 5 else TEXT_COLOR
    border_width = 3 if N == 5 else 1
    for spine in ax.spines.values():
        spine.set_edgecolor(border_color)
        spine.set_linewidth(border_width)
    title_str = f'N={N}: {name}'
    ax.set_title(title_str, color='magenta' if N == 5 else TEXT_COLOR, fontsize=13,
                 fontweight='bold' if N == 5 else 'normal')
    ax.axhline(0, color='gray', lw=0.5, alpha=0.3)
    ax.axvline(0, color='gray', lw=0.5, alpha=0.3)

    # Compute angular concentration metric
    if mask.sum() > 10:
        angles = np.arctan2(py[mask], px[mask])
        weights = region[py[mask]+zz, px[mask]+zz]
        # Compute angular histogram
        nbins = 36
        hist, bin_edges = np.histogram(angles, bins=nbins, range=(-np.pi, np.pi), weights=weights)
        hist_norm = hist / hist.sum()
        ang_entropy = -np.sum(hist_norm[hist_norm > 0] * np.log(hist_norm[hist_norm > 0]))
        max_ent = np.log(nbins)
        concentration = 1 - ang_entropy / max_ent
        ax.text(0.05, 0.05, f'Angular\nconcentration:\n{concentration:.3f}',
                transform=ax.transAxes, color=clr, fontsize=9, va='bottom',
                bbox=dict(boxstyle='round', facecolor=BG_COLOR, edgecolor=clr, alpha=0.8))

plt.tight_layout()
plt.savefig('output/wigner_round2.png', dpi=150, facecolor=BG_COLOR)
print("   Saved wigner_round2.png")
plt.close()

# ================================================================
# 3. VON NEUMANN: Noise robustness test
# ================================================================
print("3/5  VON NEUMANN: Noise robustness...")
fig, axes = plt.subplots(1, 2, figsize=(20, 8))
fig.suptitle('VON NEUMANN: Spectral Robustness Under Geometric Perturbation',
             color=ACCENT_COLOR, fontsize=16)

noise_levels = [0, 1, 2, 4, 8, 12, 16, 20]
test_Ns = [2, 3, 4, 5, 6, 7, 8]
clr_map = {2: 'cyan', 3: 'red', 4: 'lime', 5: 'magenta', 6: 'gold', 7: 'gray', 8: 'orange'}
cryst = [2, 3, 4, 6]

np.random.seed(42)

ax = axes[0]
for N in test_Ns:
    deviations = []
    ref_spec = get_spec(N, R=50)
    ref_flat = ref_spec.flatten()
    ref_norm = ref_flat / ref_flat.sum()

    for noise in noise_levels:
        if noise == 0:
            deviations.append(0)
            continue
        # Average over 5 random trials
        devs = []
        for trial in range(5):
            R_base = 50
            psi = np.zeros((W, H), dtype=complex)
            for i in range(N):
                a = 2*np.pi*i/N
                dx = np.random.normal(0, noise)
                dy = np.random.normal(0, noise)
                px = cx + R_base*np.cos(a) + dx
                py = cy + R_base*np.sin(a) + dy
                psi += mkp(px, py, -0.2*np.cos(a), -0.2*np.sin(a))
            born = np.abs(psi)**2
            noisy_spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
            noisy_flat = noisy_spec.flatten()
            noisy_norm = noisy_flat / noisy_flat.sum()
            # KL divergence (symmetrized)
            m = 0.5 * (ref_norm + noisy_norm)
            kl = 0.5 * np.sum(ref_norm * np.log((ref_norm + 1e-30) / (m + 1e-30))) + \
                 0.5 * np.sum(noisy_norm * np.log((noisy_norm + 1e-30) / (m + 1e-30)))
            devs.append(kl)
        deviations.append(np.mean(devs))

    ls = '-' if N in cryst else '--'
    lw = 2.5 if N in cryst else 1.2
    ms = 8 if N in cryst else 5
    lbl = f'N={N}' + (' (cryst)' if N in cryst else '')
    ax.plot(noise_levels, deviations, 'o-', color=clr_map[N], lw=lw, ms=ms,
            ls=ls, label=lbl, alpha=0.9)

ax.set_xlabel('Position noise σ (lattice units)', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Jensen-Shannon divergence from ideal', color=TEXT_COLOR, fontsize=12)
ax.set_title('Spectral Sensitivity to Geometric Noise', color=TEXT_COLOR, fontsize=13)
ax.legend(fontsize=9, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR, ncol=2)
ax.grid(True, alpha=0.2)

# Right panel: robustness ranking at noise=8
ax2 = axes[1]
noise_idx = noise_levels.index(8)
rankings = []
for N in test_Ns:
    ref_spec = get_spec(N, R=50)
    ref_flat = ref_spec.flatten()
    ref_norm = ref_flat / ref_flat.sum()
    devs = []
    for trial in range(10):
        psi = np.zeros((W, H), dtype=complex)
        for i in range(N):
            a = 2*np.pi*i/N
            dx = np.random.normal(0, 8)
            dy = np.random.normal(0, 8)
            px = cx + 50*np.cos(a) + dx
            py = cy + 50*np.sin(a) + dy
            psi += mkp(px, py, -0.2*np.cos(a), -0.2*np.sin(a))
        born = np.abs(psi)**2
        noisy_spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
        noisy_flat = noisy_spec.flatten()
        noisy_norm = noisy_flat / noisy_flat.sum()
        m = 0.5 * (ref_norm + noisy_norm)
        kl = 0.5 * np.sum(ref_norm * np.log((ref_norm + 1e-30) / (m + 1e-30))) + \
             0.5 * np.sum(noisy_norm * np.log((noisy_norm + 1e-30) / (m + 1e-30)))
        devs.append(kl)
    rankings.append((N, np.mean(devs), np.std(devs)))

rankings.sort(key=lambda x: x[1])
ns = [r[0] for r in rankings]
means = [r[1] for r in rankings]
stds = [r[2] for r in rankings]
bar_colors = ['red' if n == 3 else ('gold' if n in cryst else 'gray') for n in ns]
bars = ax2.barh(range(len(ns)), means, xerr=stds, color=bar_colors, edgecolor='none',
                height=0.6, alpha=0.85)
ax2.set_yticks(range(len(ns)))
ax2.set_yticklabels([f'N={n}' for n in ns])
ax2.set_xlabel('JS divergence at σ=8 (lower = more robust)', color=TEXT_COLOR, fontsize=12)
ax2.set_title('Robustness Ranking (10 trials)', color=TEXT_COLOR, fontsize=13)
ax2.invert_yaxis()

# Annotate winner
ax2.annotate('MOST ROBUST', xy=(means[0], 0), xytext=(means[0]*3, 0),
             color='lime', fontsize=12, fontweight='bold', va='center',
             arrowprops=dict(arrowstyle='->', color='lime', lw=2))
ax2.grid(True, alpha=0.2, axis='x')

plt.tight_layout()
plt.savefig('output/vonneumann_round2.png', dpi=150, facecolor=BG_COLOR)
print("   Saved vonneumann_round2.png")
plt.close()

# ================================================================
# 4. EINSTEIN: N=3 x N=6 -> BCC reciprocal lattice?
# ================================================================
print("4/5  EINSTEIN: N=3xN=6 product -> BCC?...")
fig, axes = plt.subplots(2, 3, figsize=(21, 14))
fig.suptitle('EINSTEIN: Spectral Products — Do They Reconstruct Lattice Structures?',
             color=ACCENT_COLOR, fontsize=16)

# Reuse specs from earlier computations
s3 = get_spec(3); s4 = get_spec(4); s6 = get_spec(6)
prod_34 = s3 * s4
prod_36 = s3 * s6

zz = 60

# Top row: N=3, N=4, N=3xN=4 (FCC)
for col, (data, title) in enumerate([(s3, 'N=3 Honeycomb'), (s4, 'N=4 Square'),
                                      (prod_34, 'N=3 x N=4 -> FCC?')]):
    ax = axes[0][col]; ax.set_facecolor(BG_COLOR)
    region = np.log10(data[256-zz:256+zz, 256-zz:256+zz] + 1e-30)
    ax.imshow(region, cmap='inferno', origin='lower', extent=[-zz, zz, -zz, zz])
    if col == 2:
        a_fcc = 18
        for i in range(-4, 5):
            for j in range(-4, 5):
                fx, fy = i*a_fcc, j*a_fcc
                if abs(fx) < 55 and abs(fy) < 55:
                    ax.plot(fx, fy, 'o', color='white', ms=7,
                            markerfacecolor='none', markeredgewidth=1.5, zorder=5)
                fx2, fy2 = i*a_fcc + a_fcc/2, j*a_fcc + a_fcc/2
                if abs(fx2) < 55 and abs(fy2) < 55:
                    ax.plot(fx2, fy2, 'o', color='cyan', ms=5,
                            markerfacecolor='none', markeredgewidth=1, zorder=5)
    ax.set_title(title, color=TEXT_COLOR, fontsize=13)

# Bottom row: N=3, N=6, N=3xN=6 (BCC?)
for col, (data, title) in enumerate([(s3, 'N=3 Honeycomb'), (s6, 'N=6 Hexagonal'),
                                      (prod_36, 'N=3 x N=6 -> BCC?')]):
    ax = axes[1][col]; ax.set_facecolor(BG_COLOR)
    region = np.log10(data[256-zz:256+zz, 256-zz:256+zz] + 1e-30)
    ax.imshow(region, cmap='inferno', origin='lower', extent=[-zz, zz, -zz, zz])
    if col == 2:
        # BCC reciprocal lattice = FCC -> overlay FCC points
        # BCC real -> FCC reciprocal with spacing a_bcc_recip
        a_bcc = 16
        for i in range(-4, 5):
            for j in range(-4, 5):
                # FCC reciprocal of BCC
                bx, by = i*a_bcc, j*a_bcc
                if abs(bx) < 55 and abs(by) < 55:
                    ax.plot(bx, by, 's', color='white', ms=7,
                            markerfacecolor='none', markeredgewidth=1.5, zorder=5)
                # Body centers
                bx2, by2 = i*a_bcc + a_bcc/2, j*a_bcc + a_bcc/2
                if abs(bx2) < 55 and abs(by2) < 55:
                    ax.plot(bx2, by2, 's', color='lime', ms=5,
                            markerfacecolor='none', markeredgewidth=1, zorder=5)
    ax.set_title(title, color=TEXT_COLOR, fontsize=13)

plt.tight_layout()
plt.savefig('output/einstein_round2.png', dpi=150, facecolor=BG_COLOR)
print("   Saved einstein_round2.png")
plt.close()

# ================================================================
# 5. GROTHENDIECK: Prime/composite ratio envelope
# ================================================================
print("5/5  GROTHENDIECK: Prime/composite spectral complexity ratio...")
fig, axes = plt.subplots(1, 2, figsize=(20, 8))
fig.suptitle('GROTHENDIECK: Arithmetic Signature in Born Rule Spectral Complexity',
             color=ACCENT_COLOR, fontsize=16)

def is_prime(n):
    if n < 2: return False
    if n < 4: return True
    if n % 2 == 0 or n % 3 == 0: return False
    i = 5
    while i*i <= n:
        if n % i == 0 or n % (i+2) == 0: return False
        i += 6
    return True

# Compute peak counts and entropies for N=2..30
test_range = list(range(2, 31))
peak_counts = []
spec_entropies = []
for N in test_range:
    sp = get_spec(N)
    pc = int(np.sum(sp > 0.01 * sp.max()))
    peak_counts.append(pc)
    spec_entropies.append(entropy(sp))

# Left panel: peak count with prime/composite coloring
ax = axes[0]
prime_Ns = [n for n in test_range if is_prime(n)]
comp_Ns = [n for n in test_range if not is_prime(n)]
prime_pcs = [peak_counts[test_range.index(n)] for n in prime_Ns]
comp_pcs = [peak_counts[test_range.index(n)] for n in comp_Ns]

ax.scatter(prime_Ns, prime_pcs, c='magenta', s=80, zorder=5, label='Prime N', marker='D')
ax.scatter(comp_Ns, comp_pcs, c='cyan', s=50, zorder=4, label='Composite N', marker='o')

# Connect with line
ax.plot(test_range, peak_counts, '-', color='gray', alpha=0.3, lw=1)

# Upper envelope (primes)
if len(prime_Ns) > 2:
    z = np.polyfit(prime_Ns, prime_pcs, 2)
    xfit = np.linspace(2, 30, 100)
    ax.plot(xfit, np.polyval(z, xfit), '--', color='magenta', alpha=0.5, lw=2, label='Prime envelope')

ax.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Peak count (>1% of max)', color=TEXT_COLOR, fontsize=12)
ax.set_title('Spectral Peak Count: Primes vs Composites', color=TEXT_COLOR, fontsize=13)
ax.legend(fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.grid(True, alpha=0.2)

# Right panel: ratio to nearest prime
ax2 = axes[1]
# For each composite, find ratio to the nearest prime's peak count
# Interpolate prime peak counts
from scipy.interpolate import interp1d
if len(prime_Ns) > 2:
    prime_interp = interp1d(prime_Ns, prime_pcs, kind='linear', fill_value='extrapolate')

    ratios = []
    ratio_Ns = []
    for n in test_range:
        if not is_prime(n):
            expected = prime_interp(n)
            if expected > 0:
                ratios.append(peak_counts[test_range.index(n)] / expected)
                ratio_Ns.append(n)

    # Color by number of prime factors
    def num_prime_factors(n):
        count = 0
        d = 2
        temp = n
        while d * d <= temp:
            while temp % d == 0:
                count += 1
                temp //= d
            d += 1
        if temp > 1: count += 1
        return count

    n_factors = [num_prime_factors(n) for n in ratio_Ns]
    factor_colors = {2: 'cyan', 3: 'lime', 4: 'gold', 5: 'orange'}

    for i, (n, ratio) in enumerate(zip(ratio_Ns, ratios)):
        nf = n_factors[i]
        clr = factor_colors.get(nf, 'red')
        ax2.bar(n, ratio, color=clr, edgecolor='none', width=0.7, alpha=0.8)

    ax2.axhline(1.0, color='magenta', ls='--', lw=2, alpha=0.7, label='Prime level (1.0)')
    ax2.set_xlabel('N (composites only)', color=TEXT_COLOR, fontsize=12)
    ax2.set_ylabel('Peak count / interpolated prime count', color=TEXT_COLOR, fontsize=12)
    ax2.set_title('Composite Deficit Relative to Prime Envelope', color=TEXT_COLOR, fontsize=13)
    ax2.legend(handles=[
        Patch(facecolor='cyan', label='2 prime factors'),
        Patch(facecolor='lime', label='3 prime factors'),
        Patch(facecolor='gold', label='4 prime factors'),
        Line2D([0], [0], color='magenta', ls='--', lw=2, label='Prime level (1.0)')
    ], fontsize=9, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
    ax2.grid(True, alpha=0.2, axis='y')

plt.tight_layout()
plt.savefig('output/grothendieck_round2.png', dpi=150, facecolor=BG_COLOR)
print("   Saved grothendieck_round2.png")
plt.close()

print("\nAll 5 Round 2 figures saved to output/")
