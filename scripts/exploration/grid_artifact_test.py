"""
CRITICAL TEST: Are the spectral peaks physics or square-grid artifacts?

If peaks ROTATE with particle configuration -> physics (Born rule selects symmetries)
If peaks STAY at 45 deg diagonals regardless -> grid artifact (FFT Brillouin zone M-points)

Test: Take N=3, rotate the entire configuration by 0, 15, 30, 45, 60, 75, 90 degrees.
Track where the spectral peaks appear. If they're locked to the grid, we've been
fooling ourselves.

Also test: use different grid sizes (non-power-of-2) to break FFT symmetries.
Also test: compare radial vs angular structure more carefully.
"""
import sys, os, numpy as np
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from experiments.detector_information_loss.field_engine import (
    setup_style, BG_COLOR, TEXT_COLOR, ACCENT_COLOR, GRID_COLOR
)
from matplotlib.lines import Line2D

plt = setup_style()
os.makedirs('output', exist_ok=True)

W, H = 512, 512
cx, cy = W//2, H//2
X, Y = np.meshgrid(np.arange(W), np.arange(H))

def mkp(px, py, kx, ky, s=12):
    r2 = (X-px)**2 + (Y-py)**2
    return np.exp(-r2/(2*s**2)) * np.exp(1j*(kx*(X-px) + ky*(Y-py)))

# ================================================================
# TEST 1: Rotate N=3 configuration — do peaks follow?
# ================================================================
print("TEST 1: Rotating N=3 configuration...")
rotations = [0, 15, 30, 45, 60, 75, 90]
fig, axes = plt.subplots(3, len(rotations), figsize=(4*len(rotations), 12))
fig.suptitle('GRID ARTIFACT TEST: Do Spectral Peaks Rotate With Particles or Lock to Grid?',
             color=ACCENT_COLOR, fontsize=16)

R = 50; zz = 60

peak_angles_by_rot = []

for col, rot_deg in enumerate(rotations):
    rot_rad = np.radians(rot_deg)

    # Build wavefunction with rotated configuration
    psi = np.zeros((W, H), dtype=complex)
    for i in range(3):
        a = 2*np.pi*i/3 + rot_rad
        px = cx + R*np.cos(a)
        py = cy + R*np.sin(a)
        kx = -0.2*np.cos(a)
        ky = -0.2*np.sin(a)
        psi += mkp(px, py, kx, ky)

    born = np.abs(psi)**2
    spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2
    region = spec[256-zz:256+zz, 256-zz:256+zz]

    # Row 1: The spectrum
    ax = axes[0][col]; ax.set_facecolor(BG_COLOR)
    ax.imshow(np.log10(region + 1e-30), cmap='inferno', origin='lower',
              extent=[-zz, zz, -zz, zz])
    ax.set_title(f'Rot = {rot_deg} deg', color=TEXT_COLOR, fontsize=12)
    # Draw expected peak directions (should rotate with particles)
    for i in range(6):  # A2 has 6 roots
        a = 2*np.pi*i/6 + rot_rad
        ax.plot([0, 40*np.cos(a)], [0, 40*np.sin(a)], '--', color='lime', lw=1, alpha=0.7)
    # Draw grid diagonals (fixed)
    for a in [np.pi/4, 3*np.pi/4, 5*np.pi/4, 7*np.pi/4]:
        ax.plot([0, 40*np.cos(a)], [0, 40*np.sin(a)], ':', color='red', lw=1.5, alpha=0.7)

    # Row 2: Particle positions
    ax2 = axes[1][col]; ax2.set_facecolor(BG_COLOR)
    theta = np.linspace(0, 2*np.pi, 100)
    ax2.plot(R*np.cos(theta), R*np.sin(theta), '--', color='gray', alpha=0.3)
    for i in range(3):
        a = 2*np.pi*i/3 + rot_rad
        ax2.plot(R*np.cos(a), R*np.sin(a), 'o', color='lime', ms=12, zorder=5)
        ax2.arrow(R*np.cos(a), R*np.sin(a),
                  -10*np.cos(a), -10*np.sin(a),
                  head_width=3, head_length=2, fc='lime', ec='lime', alpha=0.5)
    ax2.set_xlim(-70, 70); ax2.set_ylim(-70, 70); ax2.set_aspect('equal')
    ax2.set_title('Particles', color=TEXT_COLOR, fontsize=10)

    # Row 3: Angular profile of spectral peaks
    ax3 = axes[2][col]; ax3.set_facecolor(BG_COLOR)
    thresh = 0.05 * region.max()
    py_p, px_p = np.where(region > thresh); px_p -= zz; py_p -= zz
    r_p = np.sqrt(px_p**2 + py_p**2)
    mask = (r_p > 5) & (r_p < 45)
    if mask.sum() > 10:
        angles = np.arctan2(py_p[mask], px_p[mask])
        weights = region[py_p[mask]+zz, px_p[mask]+zz]
        nbins = 72
        hist, bin_edges = np.histogram(angles, bins=nbins, range=(-np.pi, np.pi), weights=weights)
        bin_centers = 0.5*(bin_edges[:-1] + bin_edges[1:]) * 180/np.pi
        hist_norm = hist / hist.max()
        ax3.fill_between(bin_centers, hist_norm, alpha=0.3, color='cyan')
        ax3.plot(bin_centers, hist_norm, '-', color='cyan', lw=1)

        # Mark expected peak angles (green: particle-following)
        for i in range(6):
            expected = (i * 60 + rot_deg) % 360
            if expected > 180: expected -= 360
            ax3.axvline(expected, color='lime', ls='--', alpha=0.6, lw=1)

        # Mark grid diagonal angles (red: fixed)
        for ga in [45, 135, -45, -135]:
            ax3.axvline(ga, color='red', ls=':', alpha=0.6, lw=1.5)

        # Find actual peak angles
        from scipy.signal import find_peaks
        peaks_idx, _ = find_peaks(hist_norm, height=0.3, distance=3)
        actual_peaks = bin_centers[peaks_idx]
        peak_angles_by_rot.append((rot_deg, actual_peaks))
        for pa in actual_peaks:
            ax3.plot(pa, hist_norm[peaks_idx[list(actual_peaks).index(pa)]], 'v',
                     color='white', ms=6, zorder=5)

    ax3.set_xlim(-180, 180)
    ax3.set_xlabel('Angle (deg)', color=TEXT_COLOR, fontsize=9)
    if col == 0:
        ax3.set_ylabel('Spectral weight', color=TEXT_COLOR, fontsize=10)

# Legend
axes[0][0].legend(handles=[
    Line2D([0], [0], color='lime', ls='--', lw=1.5, label='Expected (rotates with particles)'),
    Line2D([0], [0], color='red', ls=':', lw=1.5, label='Grid diagonals (fixed at 45 deg)'),
], fontsize=9, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR, loc='lower left')

plt.tight_layout()
plt.savefig('output/grid_artifact_test1.png', dpi=150, facecolor=BG_COLOR)
print("   Saved grid_artifact_test1.png")
plt.close()

# ================================================================
# TEST 2: Peak angle tracking plot
# ================================================================
print("\nTEST 2: Peak angle tracking...")
fig, ax = plt.subplots(figsize=(14, 8))
fig.suptitle('DEFINITIVE: Do Peak Angles Track Particles (green) or Grid (red)?',
             color=ACCENT_COLOR, fontsize=15)

for rot_deg, actual_peaks in peak_angles_by_rot:
    for pa in actual_peaks:
        ax.plot(rot_deg, pa, 'o', color='white', ms=6, alpha=0.7, zorder=5)

# Expected: peaks should follow y = x + offset (for each root angle)
for root_offset in [0, 60, 120, -60, -120, 180]:
    x_line = np.linspace(0, 90, 100)
    y_line = x_line + root_offset
    # Wrap to [-180, 180]
    y_line = ((y_line + 180) % 360) - 180
    ax.plot(x_line, y_line, '--', color='lime', lw=1.5, alpha=0.5)

# Grid-locked: horizontal lines at 45, 135, -45, -135
for ga in [45, 135, -45, -135, 0, 90, -90, 180]:
    ax.axhline(ga, color='red', ls=':', alpha=0.4, lw=1)

ax.set_xlabel('Configuration rotation (degrees)', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Detected peak angle (degrees)', color=TEXT_COLOR, fontsize=12)
ax.set_title('White dots = detected peaks. Green diagonals = particle-tracking.\nRed horizontals = grid-locked.',
             color=TEXT_COLOR, fontsize=12)
ax.legend(handles=[
    Line2D([0], [0], color='lime', ls='--', lw=2, label='Physics: peaks rotate with particles'),
    Line2D([0], [0], color='red', ls=':', lw=2, label='Artifact: peaks locked to grid'),
    Line2D([0], [0], marker='o', color='white', ls='', ms=8, label='Detected peaks'),
], fontsize=11, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax.set_xlim(-5, 95)
ax.set_ylim(-180, 180)
ax.grid(True, alpha=0.15)

plt.tight_layout()
plt.savefig('output/grid_artifact_test2.png', dpi=150, facecolor=BG_COLOR)
print("   Saved grid_artifact_test2.png")
plt.close()

# ================================================================
# TEST 3: Different grid sizes (break power-of-2 FFT symmetry)
# ================================================================
print("\nTEST 3: Non-power-of-2 grid sizes...")
fig, axes = plt.subplots(2, 4, figsize=(24, 12))
fig.suptitle('GRID SIZE TEST: Does Changing Grid Size Change the Pattern?',
             color=ACCENT_COLOR, fontsize=16)

grid_sizes = [256, 384, 500, 512, 600, 700, 768, 1024]

for col, gs in enumerate(grid_sizes):
    row = col // 4
    c = col % 4
    ax = axes[row][c]; ax.set_facecolor(BG_COLOR)

    xg = np.arange(gs); yg = np.arange(gs)
    Xg, Yg = np.meshgrid(xg, yg)
    cxg, cyg = gs//2, gs//2
    Rg = gs//10

    psi = np.zeros((gs, gs), dtype=complex)
    for i in range(3):
        a = 2*np.pi*i/3
        px = cxg + Rg*np.cos(a)
        py = cyg + Rg*np.sin(a)
        r2 = (Xg - px)**2 + (Yg - py)**2
        psi += np.exp(-r2/(2*(gs/42)**2)) * np.exp(1j*(-0.2*np.cos(a)*(Xg - px)))

    born = np.abs(psi)**2
    spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2

    zz_g = gs//8
    half_g = gs//2
    region = np.log10(spec[half_g-zz_g:half_g+zz_g, half_g-zz_g:half_g+zz_g] + 1e-30)
    ax.imshow(region, cmap='inferno', origin='lower',
              extent=[-zz_g, zz_g, -zz_g, zz_g])
    ax.set_title(f'{gs}x{gs}', color=TEXT_COLOR, fontsize=12)

    # Draw grid diagonals
    for a_d in [np.pi/4, 3*np.pi/4, 5*np.pi/4, 7*np.pi/4]:
        ax.plot([0, (zz_g-5)*np.cos(a_d)], [0, (zz_g-5)*np.sin(a_d)],
                ':', color='red', lw=1, alpha=0.5)
    # Draw A2 root directions
    for i in range(6):
        a_r = 2*np.pi*i/6
        ax.plot([0, (zz_g-5)*np.cos(a_r)], [0, (zz_g-5)*np.sin(a_r)],
                '--', color='lime', lw=1, alpha=0.5)

plt.tight_layout()
plt.savefig('output/grid_artifact_test3.png', dpi=150, facecolor=BG_COLOR)
print("   Saved grid_artifact_test3.png")
plt.close()

# ================================================================
# TEST 4: Pure angular analysis — radial average
# ================================================================
print("\nTEST 4: Radial-averaged angular profile...")
fig, axes = plt.subplots(2, 3, figsize=(21, 12))
fig.suptitle('RADIAL-AVERAGED Angular Profiles: Eliminating Grid Bias',
             color=ACCENT_COLOR, fontsize=16)

for col, N in enumerate([3, 4, 5]):
    psi = np.zeros((W, H), dtype=complex)
    for i in range(N):
        a = 2*np.pi*i/N
        psi += mkp(cx+R*np.cos(a), cy+R*np.sin(a), -0.2*np.cos(a), -0.2*np.sin(a))
    born = np.abs(psi)**2
    spec = np.abs(np.fft.fftshift(np.fft.fft2(born)))**2

    # Compute angular profile averaged over RADIAL shells
    # This removes radial structure and isolates angular dependence
    region = spec[256-zz:256+zz, 256-zz:256+zz]
    yy, xx = np.mgrid[-zz:zz, -zz:zz]
    rr = np.sqrt(xx**2 + yy**2)
    aa = np.arctan2(yy, xx)

    # Average over radial bands
    nbins_a = 72
    ang_bins = np.linspace(-np.pi, np.pi, nbins_a + 1)
    ang_centers = 0.5*(ang_bins[:-1] + ang_bins[1:]) * 180/np.pi

    # Multiple radial shells
    radial_bands = [(5, 15), (15, 25), (25, 35), (35, 45)]
    clrs_r = ['cyan', 'lime', 'gold', 'magenta']

    ax = axes[0][col]; ax.set_facecolor(BG_COLOR)
    for band_i, (r_lo, r_hi) in enumerate(radial_bands):
        r_mask = (rr >= r_lo) & (rr < r_hi)
        profile = np.zeros(nbins_a)
        for b in range(nbins_a):
            a_mask = (aa >= ang_bins[b]) & (aa < ang_bins[b+1])
            combined = r_mask & a_mask
            if combined.sum() > 0:
                profile[b] = np.mean(region[combined])
        profile /= (profile.max() + 1e-30)
        ax.plot(ang_centers, profile + band_i*0.3, '-', color=clrs_r[band_i], lw=1.5,
                label=f'r=[{r_lo},{r_hi}]' if col == 0 else '', alpha=0.8)

    ax.set_xlabel('Angle (degrees)', color=TEXT_COLOR, fontsize=10)
    ax.set_title(f'N={N}: Angular Profile by Radial Shell', color=TEXT_COLOR, fontsize=12)
    ax.set_xlim(-180, 180)
    if col == 0:
        ax.legend(fontsize=9, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
    ax.grid(True, alpha=0.15, axis='x')

    # Mark grid diagonals and expected roots
    for ga in [45, 135, -45, -135]:
        ax.axvline(ga, color='red', ls=':', alpha=0.3, lw=1)
    root_angles_expected = {3: [0, 60, 120, -60, -120, 180],
                            4: [0, 90, -90, 180],
                            5: [0, 72, 144, -72, -144]}
    if N in root_angles_expected:
        for ra in root_angles_expected[N]:
            ax.axvline(ra, color='lime', ls='--', alpha=0.3, lw=1)

    # Bottom row: full angular average (all radii combined)
    ax2 = axes[1][col]; ax2.set_facecolor(BG_COLOR)
    r_mask_all = (rr >= 5) & (rr < 45)
    profile_all = np.zeros(nbins_a)
    for b in range(nbins_a):
        a_mask = (aa >= ang_bins[b]) & (aa < ang_bins[b+1])
        combined = r_mask_all & a_mask
        if combined.sum() > 0:
            profile_all[b] = np.mean(region[combined])
    profile_all /= (profile_all.max() + 1e-30)

    ax2.fill_between(ang_centers, profile_all, alpha=0.3, color='cyan')
    ax2.plot(ang_centers, profile_all, '-', color='cyan', lw=2)

    # Mark grid vs physics
    for ga in [45, 135, -45, -135]:
        ax2.axvline(ga, color='red', ls=':', alpha=0.5, lw=1.5, label='Grid diagonal' if ga == 45 and col == 0 else '')
    for ra in root_angles_expected.get(N, []):
        ax2.axvline(ra, color='lime', ls='--', alpha=0.5, lw=1.5, label='Expected root' if ra == 0 and col == 0 else '')

    ax2.set_xlabel('Angle (degrees)', color=TEXT_COLOR, fontsize=10)
    ax2.set_ylabel('Radially-averaged intensity', color=TEXT_COLOR, fontsize=10)
    ax2.set_title(f'N={N}: Full Radial Average', color=TEXT_COLOR, fontsize=12)
    ax2.set_xlim(-180, 180)
    if col == 0:
        ax2.legend(fontsize=9, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
    ax2.grid(True, alpha=0.15, axis='x')

plt.tight_layout()
plt.savefig('output/grid_artifact_test4.png', dpi=150, facecolor=BG_COLOR)
print("   Saved grid_artifact_test4.png")
plt.close()

print("\nAll 4 grid artifact tests complete.")
print("VERDICT: Check if white dots follow green diagonals (physics)")
print("         or cluster on red horizontals (grid artifact).")
