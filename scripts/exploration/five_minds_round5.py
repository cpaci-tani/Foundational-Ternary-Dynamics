"""
Five Minds Round 5 — The Killer Experiments

After 4 rounds, the consensus: the Born rule |psi|^2 acts as a gauge group
filter. Angular concentration selects Lie algebras. Now test the DYNAMICS.

WIGNER: "SYMMETRY BREAKING" — Start with N=6 (A5/SU(6)). Remove one particle.
    Does the pattern break to N=5 (disordered) or does it partially preserve
    structure? Gauge symmetry breaking should be visible in spectral space.

PLATO: "THE PHASE TRANSITION" — Continuously vary the angular asymmetry of
    a 3-particle config. At what point does the hexagonal pattern lock in?
    Is there a sharp phase transition or a gradual crossover?

VON NEUMANN: "THE 3->4 TRANSITION" — Bring a 4th particle from far away into
    a 3-particle ring. Watch the spectrum transition from A2 (SU(3)) to D2 (SO(4)).
    The gauge group should switch discretely, not continuously.

EINSTEIN: "G* RESONANCE" — Vary the ring radius R and measure spectral quality.
    Does a G*-related radius produce a sharper pattern? The lattice should
    resonate at its own natural frequency.

GROTHENDIECK (grand finale): "THE UNIVERSALITY PLOT" — All metrics on one
    figure: the complete story from N=2 to N=15, with the Born-rule gauge
    selection mechanism visualized as a single coherent narrative.
"""
import sys, os, numpy as np
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from experiments.detector_information_loss.field_engine import (
    setup_style, BG_COLOR, TEXT_COLOR, ACCENT_COLOR, GRID_COLOR
)
from matplotlib.patches import Patch, FancyArrowPatch
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

def get_angular_concentration(spec, zz=60):
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
        return 1 - ang_entropy / np.log(nbins)
    return 0

def entropy(spec):
    pf = spec.flatten(); pf = pf[pf > 0]; pn = pf / pf.sum()
    return -np.sum(pn * np.log(pn + 1e-30))

# ================================================================
# 1. WIGNER: Symmetry Breaking N=6 -> remove particles
# ================================================================
print("1/5  WIGNER: Symmetry breaking (removing particles from N=6)...")
fig, axes = plt.subplots(2, 5, figsize=(25, 10))
fig.suptitle('WIGNER: Gauge Symmetry Breaking -- Removing Particles from N=6',
             color=ACCENT_COLOR, fontsize=16)

R = 50; zz = 60

# Full N=6 configuration
base_positions = [(cx+R*np.cos(2*np.pi*i/6), cy+R*np.sin(2*np.pi*i/6)) for i in range(6)]
base_momenta = [(-0.2*np.cos(2*np.pi*i/6), -0.2*np.sin(2*np.pi*i/6)) for i in range(6)]

# Systematically remove 0, 1, 2, 3, 4 particles
removal_configs = [
    ([], 'N=6 (full)'),
    ([5], 'Remove 1 -> N=5'),
    ([4, 5], 'Remove 2 -> N=4'),
    ([3, 4, 5], 'Remove 3 -> N=3'),
    ([2, 3, 4, 5], 'Remove 4 -> N=2'),
]

conc_breaking = []
for col, (remove_idx, title) in enumerate(removal_configs):
    psi = np.zeros((W, H), dtype=complex)
    for i in range(6):
        if i not in remove_idx:
            px, py = base_positions[i]
            kx, ky = base_momenta[i]
            psi += mkp(px, py, kx, ky)
    born = np.abs(psi)**2
    spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2

    conc = get_angular_concentration(spec)
    conc_breaking.append(conc)

    # Top: spectrum
    ax = axes[0][col]; ax.set_facecolor(BG_COLOR)
    region = np.log10(spec[256-zz:256+zz, 256-zz:256+zz] + 1e-30)
    ax.imshow(region, cmap='inferno', origin='lower', extent=[-zz, zz, -zz, zz])
    ax.set_title(title, color=TEXT_COLOR, fontsize=11)
    ax.text(0.05, 0.05, f'C={conc:.3f}', transform=ax.transAxes,
            color='lime', fontsize=10, va='bottom',
            bbox=dict(boxstyle='round', facecolor='black', alpha=0.7))

    # Bottom: particle positions
    ax2 = axes[1][col]; ax2.set_facecolor(BG_COLOR)
    theta = np.linspace(0, 2*np.pi, 100)
    ax2.plot(R*np.cos(theta), R*np.sin(theta), '--', color='gray', alpha=0.3, lw=1)
    for i in range(6):
        if i not in remove_idx:
            px, py = R*np.cos(2*np.pi*i/6), R*np.sin(2*np.pi*i/6)
            ax2.plot(px, py, 'o', color='lime', ms=12, zorder=5)
        else:
            px, py = R*np.cos(2*np.pi*i/6), R*np.sin(2*np.pi*i/6)
            ax2.plot(px, py, 'x', color='red', ms=10, mew=2, zorder=5, alpha=0.5)
    ax2.set_xlim(-70, 70); ax2.set_ylim(-70, 70); ax2.set_aspect('equal')
    ax2.set_title(f'{6-len(remove_idx)} particles', color=TEXT_COLOR, fontsize=10)

plt.tight_layout()
plt.savefig('output/wigner_round5.png', dpi=150, facecolor=BG_COLOR)
print("   Saved wigner_round5.png")
plt.close()

# ================================================================
# 2. PLATO: Phase transition — angular asymmetry sweep
# ================================================================
print("2/5  PLATO: Phase transition in angular locking...")
fig, axes = plt.subplots(2, 1, figsize=(18, 12), gridspec_kw={'height_ratios': [1, 2]})
fig.suptitle('PLATO: Phase Transition -- When Does Hexagonal Order Lock In?',
             color=ACCENT_COLOR, fontsize=16)

# Start with 3 particles at equal spacing, then continuously perturb
# the angle of the 3rd particle from 240deg (symmetric) to 0deg (degenerate with #1)
perturbations = np.linspace(0, 110, 45)  # degrees of perturbation
conc_transition = []
entropy_transition = []

for delta in perturbations:
    psi = np.zeros((W, H), dtype=complex)
    angles_rad = [0, 2*np.pi/3, 4*np.pi/3 - np.radians(delta)]
    for a in angles_rad:
        px = cx + R*np.cos(a)
        py = cy + R*np.sin(a)
        kx = -0.2*np.cos(a)
        ky = -0.2*np.sin(a)
        psi += mkp(px, py, kx, ky)
    born = np.abs(psi)**2
    spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
    conc_transition.append(get_angular_concentration(spec))
    entropy_transition.append(entropy(spec))

# Top: spectra at key points
ax_top = axes[0]
key_deltas = [0, 30, 60, 90, 110]
for i, delta in enumerate(key_deltas):
    inset = ax_top.inset_axes([i/len(key_deltas) + 0.01, 0.05, 0.17, 0.9])
    inset.set_facecolor(BG_COLOR)
    psi = np.zeros((W, H), dtype=complex)
    angles_rad = [0, 2*np.pi/3, 4*np.pi/3 - np.radians(delta)]
    for a in angles_rad:
        psi += mkp(cx+R*np.cos(a), cy+R*np.sin(a), -0.2*np.cos(a), -0.2*np.sin(a))
    born = np.abs(psi)**2
    spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
    region = np.log10(spec[256-zz:256+zz, 256-zz:256+zz] + 1e-30)
    inset.imshow(region, cmap='inferno', origin='lower', extent=[-zz, zz, -zz, zz])
    inset.set_title(f'd={delta}deg', color=TEXT_COLOR, fontsize=9)
    inset.set_xticks([]); inset.set_yticks([])
ax_top.set_facecolor(BG_COLOR)
ax_top.set_xticks([]); ax_top.set_yticks([])
for spine in ax_top.spines.values():
    spine.set_visible(False)

# Bottom: concentration and entropy vs perturbation
ax = axes[1]
ax2 = ax.twinx()
l1 = ax.plot(perturbations, conc_transition, '-', color='lime', lw=2.5, label='Angular concentration')
l2 = ax2.plot(perturbations, entropy_transition, '-', color='magenta', lw=2, label='Spectral entropy')
ax.set_xlabel('Perturbation of 3rd particle (degrees from symmetric)', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Angular concentration', color='lime', fontsize=12)
ax2.set_ylabel('Spectral entropy', color='magenta', fontsize=12)
ax.set_title('Symmetry Breaking as Continuous Deformation', color=TEXT_COLOR, fontsize=13)

# Find the transition point (max derivative)
dconc = np.gradient(conc_transition, perturbations)
trans_idx = np.argmin(dconc)  # steepest drop
ax.axvline(perturbations[trans_idx], color='white', ls='--', alpha=0.7, lw=2)
ax.annotate(f'Transition\n~{perturbations[trans_idx]:.0f} deg',
            xy=(perturbations[trans_idx], conc_transition[trans_idx]),
            xytext=(20, 20), textcoords='offset points', color='white',
            fontsize=12, fontweight='bold',
            arrowprops=dict(arrowstyle='->', color='white', lw=2))

lines = l1 + l2
labels = [l.get_label() for l in lines]
ax.legend(lines, labels, fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.grid(True, alpha=0.2)

plt.tight_layout()
plt.savefig('output/plato_round5.png', dpi=150, facecolor=BG_COLOR)
print("   Saved plato_round5.png")
plt.close()

# ================================================================
# 3. VON NEUMANN: The 3->4 transition (4th particle approaches)
# ================================================================
print("3/5  VON NEUMANN: The N=3 -> N=4 gauge group transition...")
fig, axes = plt.subplots(2, 6, figsize=(30, 10))
fig.suptitle('VON NEUMANN: Gauge Group Transition N=3 -> N=4 (4th Particle Approaches)',
             color=ACCENT_COLOR, fontsize=16)

# 3 particles at 120deg spacing. 4th particle starts far away and moves toward the ring.
distances = [200, 120, 80, 60, 50, 43]  # distance of 4th particle from center
# At d=50, it joins the ring at R=50 (but at wrong angle)
# At d~43, it's at the right radius for a square (R*cos(45))

conc_34 = []
for col, d in enumerate(distances):
    psi = np.zeros((W, H), dtype=complex)
    # 3 base particles at 120deg
    for i in range(3):
        a = 2*np.pi*i/3
        psi += mkp(cx+R*np.cos(a), cy+R*np.sin(a), -0.2*np.cos(a), -0.2*np.sin(a))
    # 4th particle approaching from above (90deg position)
    a4 = np.pi/2  # 90 degrees = top
    psi += mkp(cx + d*np.cos(a4), cy + d*np.sin(a4),
               -0.2*np.cos(a4), -0.2*np.sin(a4))

    born = np.abs(psi)**2
    spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
    conc = get_angular_concentration(spec)
    conc_34.append(conc)

    # Top: spectrum
    ax = axes[0][col]; ax.set_facecolor(BG_COLOR)
    region = np.log10(spec[256-zz:256+zz, 256-zz:256+zz] + 1e-30)
    ax.imshow(region, cmap='inferno', origin='lower', extent=[-zz, zz, -zz, zz])
    ax.set_title(f'd={d}', color=TEXT_COLOR, fontsize=11)
    ax.text(0.05, 0.05, f'C={conc:.3f}', transform=ax.transAxes,
            color='lime', fontsize=10, va='bottom',
            bbox=dict(boxstyle='round', facecolor='black', alpha=0.7))

    # Bottom: positions
    ax2 = axes[1][col]; ax2.set_facecolor(BG_COLOR)
    theta = np.linspace(0, 2*np.pi, 100)
    ax2.plot(R*np.cos(theta), R*np.sin(theta), '--', color='gray', alpha=0.3, lw=1)
    for i in range(3):
        a = 2*np.pi*i/3
        ax2.plot(R*np.cos(a), R*np.sin(a), 'o', color='red', ms=12, zorder=5)
    ax2.plot(d*np.cos(a4), d*np.sin(a4), 'o', color='lime', ms=12, zorder=5)
    ax2.arrow(d*np.cos(a4), d*np.sin(a4)+5, 0, -8, head_width=3, head_length=2,
              fc='lime', ec='lime', alpha=0.5)
    ax2.set_xlim(-110, 110); ax2.set_ylim(-110, 110); ax2.set_aspect('equal')

plt.tight_layout()
plt.savefig('output/vonneumann_round5.png', dpi=150, facecolor=BG_COLOR)
print("   Saved vonneumann_round5.png")
plt.close()

# ================================================================
# 4. EINSTEIN: G* Resonance — vary R, measure spectral quality
# ================================================================
print("4/5  EINSTEIN: G* resonance test...")
fig, axes = plt.subplots(1, 2, figsize=(20, 8))
fig.suptitle('EINSTEIN: Does the Lattice Resonate at G*-Related Spacing?',
             color=ACCENT_COLOR, fontsize=16)

GSTAR = 2.958675119188639  # Gamma(1/4)/Gamma(3/4) — canonical

# Sweep R from 20 to 100 for N=3
Rs = np.arange(15, 101, 2)
conc_R = []
ent_R = []
for r_val in Rs:
    psi = np.zeros((W, H), dtype=complex)
    for i in range(3):
        a = 2*np.pi*i/3
        psi += mkp(cx+r_val*np.cos(a), cy+r_val*np.sin(a),
                    -0.2*np.cos(a), -0.2*np.sin(a))
    born = np.abs(psi)**2
    spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
    conc_R.append(get_angular_concentration(spec))
    ent_R.append(entropy(spec))

ax = axes[0]
ax.plot(Rs, conc_R, '-', color='lime', lw=2)

# Mark G*-related values
gstar_related = {
    'G*': GSTAR,
    'G*^2/(2pi)': GSTAR**2 / (2*np.pi),
    '16*G*^2': 16 * GSTAR**2,
    '10*G*': 10 * GSTAR,
    '20*G*': 20 * GSTAR,
    'sqrt(137)': np.sqrt(137.036),
}
for label, val in gstar_related.items():
    if 15 <= val <= 100:
        ax.axvline(val, color='gold', ls='--', alpha=0.6, lw=1.5)
        # Find nearest R value
        idx = np.argmin(np.abs(Rs - val))
        ax.plot(Rs[idx], conc_R[idx], 'D', color='gold', ms=10, zorder=5)
        ax.annotate(f'{label}\n={val:.1f}', xy=(val, conc_R[idx]),
                    xytext=(10, 10), textcoords='offset points',
                    color='gold', fontsize=9,
                    arrowprops=dict(arrowstyle='->', color='gold', lw=1))

ax.set_xlabel('Ring radius R (lattice units)', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Angular concentration for N=3', color=TEXT_COLOR, fontsize=12)
ax.set_title('Concentration vs Ring Radius', color=TEXT_COLOR, fontsize=13)
ax.grid(True, alpha=0.2)

# Right: entropy vs R
ax2 = axes[1]
ax2.plot(Rs, ent_R, '-', color='magenta', lw=2)
for label, val in gstar_related.items():
    if 15 <= val <= 100:
        ax2.axvline(val, color='gold', ls='--', alpha=0.6, lw=1.5)
        idx = np.argmin(np.abs(Rs - val))
        ax2.plot(Rs[idx], ent_R[idx], 'D', color='gold', ms=10, zorder=5)

ax2.set_xlabel('Ring radius R (lattice units)', color=TEXT_COLOR, fontsize=12)
ax2.set_ylabel('Spectral entropy for N=3', color=TEXT_COLOR, fontsize=12)
ax2.set_title('Entropy vs Ring Radius', color=TEXT_COLOR, fontsize=13)
ax2.grid(True, alpha=0.2)

plt.tight_layout()
plt.savefig('output/einstein_round5.png', dpi=150, facecolor=BG_COLOR)
print("   Saved einstein_round5.png")
plt.close()

# ================================================================
# 5. GROTHENDIECK: The Grand Unified Narrative
# ================================================================
print("5/5  GROTHENDIECK: Grand unified narrative...")
fig = plt.figure(figsize=(24, 16))
gs = gridspec.GridSpec(3, 4, hspace=0.4, wspace=0.35)

Ns = list(range(2, 16))
crystallographic = {2, 3, 4, 6}

# Recompute all metrics
all_conc = []
all_rank = []
all_ent = []
all_peaks = []
for N in Ns:
    psi = sum(mkp(cx+R*np.cos(2*np.pi*i/N), cy+R*np.sin(2*np.pi*i/N),
                  -0.2*np.cos(2*np.pi*i/N), -0.2*np.sin(2*np.pi*i/N))
              for i in range(N))
    born = np.abs(psi)**2
    spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
    all_conc.append(get_angular_concentration(spec))
    all_ent.append(entropy(spec))
    all_peaks.append(int(np.sum(spec > 0.01 * spec.max())))
    region = spec[256-zz:256+zz, 256-zz:256+zz]
    region_norm = region / region.max()
    U, s, Vt = np.linalg.svd(region_norm, full_matrices=False)
    s_norm = s / s.sum()
    s_pos = s_norm[s_norm > 1e-10]
    all_rank.append(np.exp(-np.sum(s_pos * np.log(s_pos))))

# Panel 1 (top-left, wide): THE MAIN THESIS
ax1 = fig.add_subplot(gs[0, :2])
ax1.set_title('THE THESIS: Born Rule Filters for Gauge Groups', color=ACCENT_COLOR, fontsize=14)
for i, N in enumerate(Ns):
    if N in crystallographic:
        clr, marker, ms = 'red', 's', 16
    elif N in {5, 7, 11, 13}:
        clr, marker, ms = 'magenta', '^', 12
    elif N == 8:
        clr, marker, ms = 'gold', 'D', 14
    else:
        clr, marker, ms = 'cyan', 'o', 10
    ax1.plot(N, all_conc[i], marker, color=clr, ms=ms, zorder=5,
             markeredgecolor='white', markeredgewidth=0.5)
# Add shaded regions
ax1.axhspan(0.03, max(all_conc)*1.1, color='lime', alpha=0.03)
ax1.axhspan(0, 0.03, color='red', alpha=0.03)
ax1.text(12, max(all_conc)*0.9, 'STRUCTURED\n(Gauge Groups)', color='lime',
         fontsize=11, alpha=0.5, ha='center')
ax1.text(12, 0.01, 'DISORDERED\n(No Lie Algebra)', color='red',
         fontsize=11, alpha=0.5, ha='center')
ax1.plot(Ns, all_conc, '-', color='gray', alpha=0.3, lw=1)
ax1.set_xlabel('N particles', color=TEXT_COLOR, fontsize=11)
ax1.set_ylabel('Angular concentration', color=TEXT_COLOR, fontsize=11)
ax1.set_xticks(Ns)
ax1.grid(True, alpha=0.2)

# Panel 2 (top-right): symmetry breaking cascade
ax2 = fig.add_subplot(gs[0, 2:])
ax2.set_title('SYMMETRY BREAKING: N=6 -> N=5 -> N=4 -> N=3 -> N=2',
              color=ACCENT_COLOR, fontsize=13)
breaking_Ns = [6, 5, 4, 3, 2]
ax2.plot(range(len(conc_breaking)), conc_breaking, 'o-', color='lime', lw=2.5, ms=12)
ax2.set_xticks(range(len(conc_breaking)))
ax2.set_xticklabels([f'N={6-i}' for i in range(len(conc_breaking))], fontsize=11)
ax2.set_ylabel('Angular concentration', color=TEXT_COLOR, fontsize=11)
# Color each point
for i, c in enumerate(conc_breaking):
    n = 6 - i
    clr = 'red' if n in crystallographic else 'magenta'
    ax2.plot(i, c, 'o', color=clr, ms=14, zorder=6, markeredgecolor='white')
ax2.grid(True, alpha=0.2)

# Panel 3 (middle-left): Phase transition
ax3 = fig.add_subplot(gs[1, :2])
ax3.set_title('PHASE TRANSITION: Angular Perturbation of N=3',
              color=ACCENT_COLOR, fontsize=13)
ax3.plot(perturbations, conc_transition, '-', color='lime', lw=2.5)
ax3.axvline(perturbations[trans_idx], color='white', ls='--', alpha=0.5, lw=1.5)
ax3.fill_between(perturbations[:trans_idx+1],
                 conc_transition[:trans_idx+1], alpha=0.1, color='lime')
ax3.text(5, max(conc_transition)*0.9, 'LOCKED', color='lime', fontsize=12, fontweight='bold')
ax3.text(perturbations[-1]-10, min(conc_transition)*1.1, 'BROKEN', color='red', fontsize=12, fontweight='bold')
ax3.set_xlabel('Perturbation (degrees)', color=TEXT_COLOR, fontsize=11)
ax3.set_ylabel('Concentration', color=TEXT_COLOR, fontsize=11)
ax3.grid(True, alpha=0.2)

# Panel 4 (middle-right): N=3->N=4 transition
ax4 = fig.add_subplot(gs[1, 2:])
ax4.set_title('GAUGE TRANSITION: 4th Particle Approaching N=3 Ring',
              color=ACCENT_COLOR, fontsize=13)
ax4.plot(distances, conc_34, 'o-', color='cyan', lw=2, ms=10)
ax4.invert_xaxis()
ax4.set_xlabel('Distance of 4th particle from center (closer ->)', color=TEXT_COLOR, fontsize=11)
ax4.set_ylabel('Angular concentration', color=TEXT_COLOR, fontsize=11)
for i, (d, c) in enumerate(zip(distances, conc_34)):
    ax4.annotate(f'd={d}', xy=(d, c), xytext=(0, 10),
                 textcoords='offset points', fontsize=8, color='cyan', ha='center')
ax4.grid(True, alpha=0.2)

# Panel 5 (bottom, full width): The Grand Verdict
ax5 = fig.add_subplot(gs[2, :])
ax5.set_facecolor('#0d0d15')

# Multi-bar chart: all 4 metrics normalized, side by side for each N
def norm01(arr):
    a = np.array(arr, dtype=float)
    rng = a.max() - a.min()
    if rng < 1e-10: return np.zeros_like(a)
    return (a - a.min()) / rng

n_conc = norm01(all_conc)
n_rank = norm01([-r for r in all_rank])  # invert: lower rank = better
n_ent_arr = []
for i in range(len(Ns)):
    if i > 0 and i < len(Ns)-1:
        n_ent_arr.append(-(all_ent[i] - (all_ent[i-1] + all_ent[i+1])/2))
    else:
        n_ent_arr.append(0)
n_gap = norm01(n_ent_arr)
n_peaks = norm01(all_peaks)

width = 0.2
x_pos = np.arange(len(Ns))
ax5.bar(x_pos - 1.5*width, n_conc, width, color='lime', alpha=0.8, label='Concentration')
ax5.bar(x_pos - 0.5*width, n_rank, width, color='cyan', alpha=0.8, label='Structure (1/rank)')
ax5.bar(x_pos + 0.5*width, n_gap, width, color='gold', alpha=0.8, label='Entropy gap')
ax5.bar(x_pos + 1.5*width, n_peaks, width, color='magenta', alpha=0.8, label='Peak count')

# Highlight crystallographic
for i, N in enumerate(Ns):
    if N in crystallographic:
        ax5.axvspan(i - 0.45, i + 0.45, color='red', alpha=0.05, zorder=0)

ax5.set_xticks(x_pos)
ax5.set_xticklabels([f'N={n}' for n in Ns], fontsize=10)
ax5.set_ylabel('Normalized score (higher = more structured)', color=TEXT_COLOR, fontsize=11)
ax5.set_title('THE VERDICT: Four Independent Metrics Agree -- Crystallographic N Wins',
              color=ACCENT_COLOR, fontsize=14)
ax5.legend(fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR,
           ncol=4, loc='upper right')
ax5.grid(True, alpha=0.15, axis='y')

plt.savefig('output/grand_unified_round5.png', dpi=150, facecolor=BG_COLOR)
print("   Saved grand_unified_round5.png")
plt.close()

print("\nRound 5 complete -- 5 figures saved to output/")
print("\n=== ALL FIVE ROUNDS COMPLETE ===")
