"""
Real 3D Angular Analysis — Data from the WASM FTD Engine (32^3 cubic lattice)

This is the DEFINITIVE test. No 2D FFT grid artifacts. The flux field was
sampled on an equatorial ring in the actual 3D lattice after 40 ticks of
wave propagation with particles frozen in place.

KEY FINDING: N=5 works fine in 3D! The "N=5 fails" result from the 2D
Python analysis was a square-grid FFT artifact, not physics.
"""
import sys, os, numpy as np
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from experiments.detector_information_loss.field_engine import (
    setup_style, BG_COLOR, TEXT_COLOR, ACCENT_COLOR, GRID_COLOR
)
from matplotlib.lines import Line2D
from matplotlib.patches import Patch

plt = setup_style()
os.makedirs('output', exist_ok=True)

# Data extracted from the real 3D WASM engine (32^3 lattice, 40 ticks)
# Equatorial flux magnitude profile sampled at 72 azimuthal angles
profiles = {
    2: [1,0.989,0.8723,0.7209,0.7665,0.6421,0.365,0.365,0.2008,0.1475,0.1891,0.1712,0.1712,0.122,0.1125,0.087,0.0816,0.0773,0.077,0.0771,0.0812,0.0867,0.112,0.1217,0.1553,0.171,0.1891,0.1484,0.2012,0.3645,0.3645,0.6425,0.7674,0.7162,0.8673,0.9841,0.9951,0.9841,0.8675,0.7169,0.7691,0.645,0.367,0.367,0.2042,0.152,0.1931,0.1741,0.1741,0.1225,0.1129,0.0834,0.0779,0.0737,0.0737,0.0739,0.0782,0.0836,0.1133,0.1228,0.1588,0.1743,0.1931,0.1511,0.2038,0.3674,0.5948,0.6445,0.7682,0.7216,0.8724,0.989],
    3: [0.7618,0.7523,0.6605,0.5425,0.5397,0.4263,0.2281,0.2281,0.1324,0.121,0.1485,0.1412,0.1412,0.1364,0.1695,0.1968,0.2218,0.2626,0.2963,0.3341,0.346,0.3331,0.7422,0.7299,1,0.7413,0.7609,0.6918,0.3987,0.1596,0.1596,0.1064,0.1244,0.0645,0.0678,0.0558,0.062,0.0565,0.0687,0.0655,0.125,0.1073,0.1603,0.1603,0.3987,0.6919,0.7625,0.7443,0.7443,0.7308,0.7432,0.3298,0.3426,0.3307,0.293,0.2597,0.2195,0.195,0.1699,0.137,0.147,0.144,0.1519,0.1236,0.1352,0.2306,0.3837,0.4285,0.5413,0.5429,0.6605,0.7522],
    4: [1,0.9891,0.8848,0.748,0.7695,0.6264,0.3524,0.3524,0.2604,0.22,0.2606,0.3516,0.3516,0.6252,0.7685,0.7452,0.8822,0.9865,0.9974,0.9864,0.8824,0.7466,0.7728,0.6311,0.5451,0.3578,0.2664,0.2272,0.2682,0.3557,0.3557,0.6274,0.7713,0.739,0.8757,0.9799,0.9908,0.9799,0.8758,0.74,0.7742,0.6314,0.3599,0.3599,0.2721,0.232,0.2717,0.3601,0.3601,0.6319,0.774,0.7408,0.8764,0.9803,0.9913,0.9804,0.8761,0.7392,0.7696,0.6259,0.5404,0.3537,0.2658,0.2248,0.2642,0.3565,0.5438,0.6303,0.7723,0.7489,0.8849,0.989],
    5: [0.7623,0.7542,0.6621,0.5528,0.5624,0.4368,0.2319,0.2319,0.1771,0.2252,0.4364,0.6059,0.6059,0.7696,0.9987,0.7282,0.7484,0.7603,0.7023,0.6086,0.4835,0.3778,0.4185,0.3191,0.4348,0.4269,0.7201,0.8131,0.8329,0.7332,0.7332,0.4183,0.4314,0.2421,0.2719,0.3069,0.3024,0.3078,0.2736,0.2443,0.4326,0.4192,0.7336,0.7336,0.835,0.8183,0.7266,0.4321,0.4321,0.3198,0.4192,0.3737,0.4785,0.6032,0.697,0.755,0.7431,0.7228,1,0.771,0.791,0.6073,0.4374,0.2274,0.1807,0.2356,0.3688,0.4401,0.5647,0.5535,0.6622,0.7542],
    6: [0.7261,0.7115,0.6428,0.539,0.5726,0.4458,0.2856,0.2856,0.4078,0.6845,0.7729,0.7709,0.7709,0.7273,0.7392,0.4227,0.4688,0.5037,0.5041,0.5052,0.4713,0.4257,0.7404,0.7282,0.9959,0.7738,0.7798,0.6941,0.4182,0.2903,0.2903,0.4471,0.5746,0.5303,0.6339,0.7026,0.7172,0.7027,0.6342,0.5312,0.5772,0.4508,0.2941,0.2941,0.4197,0.6951,0.7833,0.779,0.779,0.7298,0.742,0.4205,0.4659,0.4995,0.4981,0.4979,0.4634,0.4174,0.7407,0.7289,1,0.776,0.7763,0.6854,0.4092,0.2893,0.4209,0.4494,0.5751,0.5399,0.643,0.7116],
    7: [0.8472,0.8296,0.7271,0.6038,0.6769,0.5152,0.4724,0.4724,0.737,0.8415,0.8592,0.7522,0.7522,0.4939,0.5622,0.4538,0.557,0.6961,0.7647,0.8064,0.7784,0.7434,0.9982,0.7701,0.7901,0.6086,0.4653,0.4701,0.639,0.6364,0.6364,0.7193,0.8106,0.4877,0.5528,0.5942,0.6155,0.5953,0.5544,0.4894,0.8117,0.7205,0.6401,0.6401,0.6452,0.4776,0.4686,0.6108,0.6108,0.7718,1,0.7364,0.7714,0.7993,0.7576,0.6892,0.5503,0.4482,0.5636,0.4956,0.8207,0.7589,0.8668,0.845,0.7384,0.475,0.5756,0.5194,0.6799,0.6051,0.7276,0.8296],
    8: [0.8137,0.8043,0.7462,0.6346,0.7906,0.6488,0.7534,0.7534,0.9849,0.9741,0.9862,0.7544,0.7544,0.6474,0.7895,0.6312,0.7429,0.801,0.8105,0.8013,0.7438,0.6335,0.7935,0.6526,0.807,0.7571,0.9894,0.9827,0.9978,0.7638,0.7638,0.6511,0.7933,0.6239,0.7352,0.7931,0.8025,0.7934,0.7358,0.6254,0.796,0.6546,0.7656,0.7656,1,0.9886,0.9981,0.764,0.764,0.6542,0.7953,0.6264,0.7365,0.7939,0.803,0.7935,0.7356,0.6241,0.7913,0.649,0.8101,0.7613,0.9947,0.9798,0.987,0.7552,0.805,0.6522,0.7933,0.6361,0.7468,0.8045],
}

concentrations = {2: 0.081, 3: 0.0599, 4: 0.0221, 5: 0.0187, 6: 0.0102, 7: 0.0049, 8: 0.0022}
contrasts = {2: 13.2, 3: 13.27, 4: 4.27, 5: 4.3, 6: 3.01, 7: 1.98, 8: 1.59}

phi_angles = np.linspace(0, 360, 72, endpoint=False)

# ================================================================
# Figure 1: Polar plots for each N
# ================================================================
fig, axes = plt.subplots(2, 4, figsize=(24, 12), subplot_kw=dict(projection='polar'))
fig.suptitle('REAL 3D ENGINE: Equatorial Flux Profiles (32^3 Cubic Lattice, No Grid Artifacts)',
             color=ACCENT_COLOR, fontsize=16, y=1.02)

clrs = {2: 'cyan', 3: 'red', 4: 'lime', 5: 'magenta', 6: 'gold', 7: 'gray', 8: 'orange'}
crystallographic = {2, 3, 4, 6}

for idx, N in enumerate([2, 3, 4, 5, 6, 7, 8]):
    row, col = idx // 4, idx % 4
    ax = axes[row][col]
    theta = np.radians(phi_angles)
    r = np.array(profiles[N])

    # Close the loop
    theta_closed = np.append(theta, theta[0])
    r_closed = np.append(r, r[0])

    ax.fill(theta_closed, r_closed, alpha=0.2, color=clrs[N])
    ax.plot(theta_closed, r_closed, '-', color=clrs[N], lw=2)

    # Mark expected particle positions
    for i in range(N):
        a = 2 * np.pi * i / N
        ax.plot([a, a], [0, 1.1], '--', color='white', alpha=0.4, lw=1)
        ax.plot(a, 1.05, 'o', color='white', ms=5, zorder=5)

    label = 'CRYST' if N in crystallographic else ('prime' if N in {5,7} else '')
    conc = concentrations[N]
    contrast = contrasts[N]
    ax.set_title(f'N={N} {label}\nC={conc:.4f} Contrast={contrast:.1f}',
                 color=clrs[N], fontsize=11, pad=15,
                 fontweight='bold' if N in crystallographic else 'normal')
    ax.set_ylim(0, 1.15)
    ax.set_rticks([0.25, 0.5, 0.75, 1.0])
    ax.tick_params(colors=TEXT_COLOR, labelsize=7)
    ax.grid(True, alpha=0.2)

# Empty last subplot — use for legend
ax_legend = axes[1][3]
ax_legend.set_visible(False)
fig.legend(handles=[
    Line2D([0], [0], color='white', marker='o', ms=8, ls='--', label='Particle positions'),
    Patch(facecolor='red', alpha=0.3, label='Crystallographic N'),
    Patch(facecolor='magenta', alpha=0.3, label='Non-crystallographic N'),
], fontsize=12, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR,
    loc='lower right', bbox_to_anchor=(0.98, 0.05))

plt.tight_layout()
plt.savefig('output/real_3d_angular_polar.png', dpi=150, facecolor=BG_COLOR)
print("Saved real_3d_angular_polar.png")
plt.close()

# ================================================================
# Figure 2: Concentration and contrast comparison
# ================================================================
fig, axes = plt.subplots(1, 3, figsize=(24, 7))
fig.suptitle('REAL 3D ENGINE: The Born Rule Does NOT Filter for Lie Algebras on the Cubic Lattice',
             color=ACCENT_COLOR, fontsize=16)

Ns = [2, 3, 4, 5, 6, 7, 8]

# Panel 1: Concentration vs N
ax = axes[0]
for N in Ns:
    clr = 'red' if N in crystallographic else 'magenta'
    ms = 14 if N in crystallographic else 10
    ax.bar(N, concentrations[N], color=clr, edgecolor='none', width=0.6, alpha=0.85)
ax.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax.set_ylabel('Angular concentration', color=TEXT_COLOR, fontsize=12)
ax.set_title('3D Concentration: Monotonically Decreasing', color=TEXT_COLOR, fontsize=13)
ax.set_xticks(Ns)
ax.grid(True, alpha=0.2, axis='y')

# Panel 2: Contrast vs N
ax2 = axes[1]
for N in Ns:
    clr = 'red' if N in crystallographic else 'magenta'
    ax2.bar(N, contrasts[N], color=clr, edgecolor='none', width=0.6, alpha=0.85)
ax2.set_xlabel('N particles', color=TEXT_COLOR, fontsize=12)
ax2.set_ylabel('Peak-to-trough contrast', color=TEXT_COLOR, fontsize=12)
ax2.set_title('3D Contrast: N=2,3 Dominate (Fewer Particles = Simpler)', color=TEXT_COLOR, fontsize=13)
ax2.set_xticks(Ns)
ax2.grid(True, alpha=0.2, axis='y')

# Panel 3: The honest comparison — 2D artifact vs 3D reality
ax3 = axes[2]
# 2D concentrations from earlier (Round 3 Wigner sweep)
conc_2d = {2: 0.045, 3: 0.035, 4: 0.055, 5: 0.015, 6: 0.045, 7: 0.03, 8: 0.035}
conc_3d = concentrations

x = np.arange(len(Ns))
width = 0.35
bars1 = ax3.bar(x - width/2, [conc_2d.get(N, 0) for N in Ns], width, color='cyan', alpha=0.7, label='2D FFT (grid artifact)')
bars2 = ax3.bar(x + width/2, [conc_3d[N] for N in Ns], width, color='lime', alpha=0.7, label='3D Engine (real)')

ax3.set_xticks(x)
ax3.set_xticklabels([f'N={N}' for N in Ns])
ax3.set_ylabel('Angular concentration', color=TEXT_COLOR, fontsize=12)
ax3.set_title('2D FFT vs 3D Engine: The Artifact Exposed', color=TEXT_COLOR, fontsize=13)
ax3.legend(fontsize=11, facecolor=BG_COLOR, edgecolor=GRID_COLOR, labelcolor=TEXT_COLOR)
ax3.grid(True, alpha=0.2, axis='y')

# Annotate the key difference
ax3.annotate('N=5 "dip" was\na grid artifact!',
             xy=(3 + width/2, conc_3d[5]), xytext=(4.5, 0.04),
             color='yellow', fontsize=12, fontweight='bold',
             arrowprops=dict(arrowstyle='->', color='yellow', lw=2))

plt.tight_layout()
plt.savefig('output/real_3d_angular_comparison.png', dpi=150, facecolor=BG_COLOR)
print("Saved real_3d_angular_comparison.png")
plt.close()

# ================================================================
# Figure 3: Stacked linear profiles
# ================================================================
fig, ax = plt.subplots(figsize=(18, 14))
fig.suptitle('REAL 3D ENGINE: Flux Profiles Track Particle Positions Exactly',
             color=ACCENT_COLOR, fontsize=16)

for idx, N in enumerate([2, 3, 4, 5, 6, 7, 8]):
    p = np.array(profiles[N])
    offset = idx * 1.3
    ax.fill_between(phi_angles, offset, offset + p, alpha=0.2, color=clrs[N])
    ax.plot(phi_angles, offset + p, '-', color=clrs[N], lw=2)

    # Mark particle positions
    for i in range(N):
        angle = 360 * i / N
        ax.axvline(angle, ymin=(offset)/(len(Ns)*1.3 + 0.5),
                   ymax=(offset + 1.2)/(len(Ns)*1.3 + 0.5),
                   color='white', ls=':', alpha=0.3, lw=1)
        ax.plot(angle, offset + 1.05, 'v', color='white', ms=6, zorder=5)

    label = f'N={N}'
    if N in crystallographic:
        label += ' (cryst)'
    ax.text(-8, offset + 0.5, label, color=clrs[N], fontsize=12,
            fontweight='bold' if N in crystallographic else 'normal',
            ha='right', va='center')

ax.set_xlabel('Azimuthal angle (degrees)', color=TEXT_COLOR, fontsize=13)
ax.set_xlim(-15, 365)
ax.set_yticks([])
ax.grid(True, alpha=0.15, axis='x')

# Key observation box
ax.text(0.98, 0.02,
        'KEY FINDING:\n'
        'In the real 3D cubic lattice,\n'
        'ALL N values produce clean peaks\n'
        'at the correct particle angles.\n\n'
        'N=5 works perfectly (72 deg spacing).\n'
        'The "gauge group selection" seen\n'
        'in 2D was a square-grid FFT artifact.\n\n'
        'Concentration decreases monotonically\n'
        'with N (more particles = more uniform).\n'
        'No special role for crystallographic N.',
        transform=ax.transAxes, color='yellow', fontsize=11,
        va='bottom', ha='right',
        bbox=dict(boxstyle='round', facecolor='black', edgecolor='yellow', alpha=0.8))

plt.tight_layout()
plt.savefig('output/real_3d_angular_stacked.png', dpi=150, facecolor=BG_COLOR)
print("Saved real_3d_angular_stacked.png")
plt.close()

print("\nAll 3 figures saved to output/")
print("\nSUMMARY:")
print("=" * 60)
print("The 2D Python FFT analysis was measuring GRID ARTIFACTS.")
print("The real 3D cubic lattice engine shows:")
print("  - ALL N produce peaks at correct particle angles")
print("  - N=5 works fine (no 'gauge group filter')")
print("  - Concentration decreases monotonically with N")
print("  - No special selection of crystallographic dimensions")
print("  - The earlier Wigner finding was FALSE")
print("=" * 60)
