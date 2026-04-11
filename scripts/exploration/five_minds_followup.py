"""Five Minds Follow-up Visualizations — individual figures for each mind."""
import sys, os, numpy as np
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from experiments.detector_information_loss.field_engine import (
    setup_style, BG_COLOR, TEXT_COLOR, ACCENT_COLOR, GRID_COLOR
)
from matplotlib.patches import Patch

plt = setup_style()
os.makedirs('output', exist_ok=True)

W, H = 512, 512
cx, cy = W//2, H//2
x = np.arange(W); y = np.arange(H)
X, Y = np.meshgrid(x, y)

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

# ================================================================
# PLATO: Entropy gap
# ================================================================
print("Computing Plato...")
fig, ax = plt.subplots(figsize=(14, 6))
Ns = list(range(2, 25))
ents = [entropy(get_spec(N)) for N in Ns]
gaps = [ents[i] - (ents[i-1] + ents[i+1])/2 for i in range(1, len(ents)-1)]
gap_Ns = list(range(3, 24))
platonic = [4, 6, 8, 12, 20]
colors = ['gold' if n in platonic else 'cyan' for n in gap_Ns]
ax.bar(gap_Ns, gaps, color=colors, edgecolor='none', width=0.7)
ax.axhline(0, color='white', ls='--', alpha=0.5)
ax.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Entropy gap (negative = more ordered than neighbors)', color=TEXT_COLOR, fontsize=12)
ax.set_title('PLATO: Spectral Entropy Gap', color=ACCENT_COLOR, fontsize=14)
ax.legend(handles=[Patch(facecolor='gold', label='Platonic vertices'),
                   Patch(facecolor='cyan', label='Other N')],
          fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.grid(True, alpha=0.2, axis='y')
plt.tight_layout()
plt.savefig('output/plato_followup.png', dpi=150, facecolor=BG_COLOR)
print("  Saved plato_followup.png")
plt.close()

# ================================================================
# WIGNER: N=2,3,4 root systems
# ================================================================
print("Computing Wigner...")
fig, axes = plt.subplots(1, 3, figsize=(21, 7))
fig.suptitle('WIGNER: Born Rule Peaks vs Lie Algebra Root Systems',
             color=ACCENT_COLOR, fontsize=16)
zz = 60
roots = {
    2: ('A1 / SU(2)', [0, 180], 'cyan'),
    3: ('A2 / SU(3)', [0, 60, 120, 180, 240, 300], 'red'),
    4: ('D2 / SO(4)', [0, 90, 180, 270], 'lime'),
}
for col, N in enumerate([2, 3, 4]):
    ax = axes[col]; ax.set_facecolor(BG_COLOR)
    spec = get_spec(N)
    region = spec[256-zz:256+zz, 256-zz:256+zz]
    thresh = 0.05 * region.max()
    py, px = np.where(region > thresh); px -= zz; py -= zz
    r = np.sqrt(px**2 + py**2); mask = (r > 3) & (r < 45)
    if mask.sum() > 0:
        vals = np.log10(region[py[mask]+zz, px[mask]+zz] + 1e-30)
        ax.scatter(px[mask], py[mask], c=vals, cmap='hot', s=8, alpha=0.6, zorder=3)
    name, angs, clr = roots[N]
    for a in angs:
        rad = np.radians(a)
        ax.plot([0, 35*np.cos(rad)], [0, 35*np.sin(rad)], '--', color=clr, lw=2, alpha=0.8)
        ax.plot(35*np.cos(rad), 35*np.sin(rad), 'o', color=clr, ms=8, zorder=5)
    ax.set_xlim(-50, 50); ax.set_ylim(-50, 50); ax.set_aspect('equal')
    ax.set_title(f'N={N}: {name}', color=TEXT_COLOR, fontsize=13)
    ax.axhline(0, color='gray', lw=0.5, alpha=0.3)
    ax.axvline(0, color='gray', lw=0.5, alpha=0.3)
plt.tight_layout()
plt.savefig('output/wigner_followup.png', dpi=150, facecolor=BG_COLOR)
print("  Saved wigner_followup.png")
plt.close()

# ================================================================
# VON NEUMANN: Crystallographic stability
# ================================================================
print("Computing von Neumann...")
fig, ax = plt.subplots(figsize=(12, 7))
Ns_vn = list(range(2, 13))
conds = []
for N in Ns_vn:
    M = []
    for R in [30, 35, 40, 45, 50, 55, 60]:
        sp = get_spec(N, R=R)
        flat = sp.flatten()
        top = flat[np.argsort(flat)[-20:]]
        M.append(top)
    conds.append(np.linalg.cond(np.array(M)))
cryst = [2, 3, 4, 6]
for i, N in enumerate(Ns_vn):
    clr = 'red' if N in cryst else 'gray'
    ms = 14 if N in cryst else 8
    fc = clr if N in cryst else 'none'
    ax.semilogy(N, conds[i], 'o', color=clr, ms=ms, markerfacecolor=fc,
                markeredgecolor=clr, zorder=5 if N in cryst else 3)
    if N in cryst:
        ax.annotate(f'N={N}', xy=(N, conds[i]), xytext=(8, -5),
                    textcoords='offset points', color='red', fontsize=11, fontweight='bold')
ax.axhline(conds[1], color='red', ls='--', alpha=0.4, label=f'N=3 level')
ax.annotate('FTD ground state', xy=(3, conds[1]), xytext=(40, 40),
            textcoords='offset points', color='lime', fontsize=13, fontweight='bold',
            arrowprops=dict(arrowstyle='->', color='lime'))
ax.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Condition number (log)', color=TEXT_COLOR, fontsize=12)
ax.set_title('VON NEUMANN: Spectral Stability Selects Crystallographic Dimensions',
             color=ACCENT_COLOR, fontsize=14)
ax.legend(fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.grid(True, alpha=0.2)
plt.tight_layout()
plt.savefig('output/vonneumann_followup.png', dpi=150, facecolor=BG_COLOR)
print("  Saved vonneumann_followup.png")
plt.close()

# ================================================================
# EINSTEIN: FCC overlay
# ================================================================
print("Computing Einstein...")
fig, axes = plt.subplots(1, 3, figsize=(21, 7))
fig.suptitle('EINSTEIN: Spectral Product vs Ideal FCC Reciprocal Lattice',
             color=ACCENT_COLOR, fontsize=16)
s3 = get_spec(3); s4 = get_spec(4); prod = s3 * s4
zz = 60
for col, (data, title) in enumerate([(s3, 'N=3 Honeycomb'), (s4, 'N=4 Square'),
                                      (prod, 'Product (FCC?)')]):
    ax = axes[col]; ax.set_facecolor(BG_COLOR)
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
plt.tight_layout()
plt.savefig('output/einstein_followup.png', dpi=150, facecolor=BG_COLOR)
print("  Saved einstein_followup.png")
plt.close()

# ================================================================
# GROTHENDIECK: Multi-resolution scaling
# ================================================================
print("Computing Grothendieck...")
fig, ax = plt.subplots(figsize=(12, 7))
test_Ns = [7, 12, 23]
resolutions = [128, 256, 512]
clrs = {7: 'cyan', 12: 'gold', 23: 'magenta'}
for N in test_Ns:
    counts = []
    for res in resolutions:
        xg = np.arange(res); yg = np.arange(res)
        Xg, Yg = np.meshgrid(xg, yg)
        cxg, cyg = res//2, res//2
        R = res//10
        psi = np.zeros((res, res), dtype=complex)
        for i in range(N):
            a = 2*np.pi*i/N
            r2 = (Xg-(cxg+R*np.cos(a)))**2 + (Yg-(cyg+R*np.sin(a)))**2
            psi += np.exp(-r2/(2*8**2))*np.exp(1j*(-0.2*np.cos(a)*(Xg-cxg-R*np.cos(a))))
        born = np.abs(psi)**2
        sp = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
        counts.append(int(np.sum(sp > 0.01*sp.max())))
    lbl = 'prime' if N in [7, 23] else 'composite'
    ax.loglog(resolutions, counts, 'o-', color=clrs[N], lw=2, ms=8,
              label=f'N={N} ({lbl})')
ax.loglog(resolutions, [r**2/500 for r in resolutions], '--', color='gray',
          alpha=0.5, label='~res^2')
ax.set_xlabel('Grid resolution', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Peak count', color=TEXT_COLOR, fontsize=12)
ax.set_title('GROTHENDIECK: Peak Count vs Resolution', color=ACCENT_COLOR, fontsize=14)
ax.legend(fontsize=10, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.grid(True, alpha=0.2)
plt.tight_layout()
plt.savefig('output/grothendieck_followup.png', dpi=150, facecolor=BG_COLOR)
print("  Saved grothendieck_followup.png")
plt.close()

print("\nAll 5 follow-up figures saved to output/")
