import matplotlib.pyplot as plt
import numpy as np
import os

out_dir = r"C:\Users\cpaci\Desktop\ftd\docs\theory\media"
os.makedirs(out_dir, exist_ok=True)

plt.style.use('dark_background')

# 1. Vacuum Energy Cutoff Toymodel
fig, ax = plt.subplots(figsize=(8, 5))
k = np.linspace(0, 5, 500)
# Mocking the phase space integral E ~ k^3
continuous_energy = 0.5 * k**3 
ax.plot(k, continuous_energy, color='#FF3333', linestyle='--', linewidth=2, label='Continuous QFT (UV Divergence to $\infty$)')

# FTD Discrete Cutoff (Brillouin Zone)
k_cutoff = np.pi
k_discrete = np.linspace(0, k_cutoff, 200)
# FTD dispersion relation E ~ sin^2(k/2) * k^2 (phase space)
discrete_energy = 2.0 * np.sin(k_discrete/2)**2 * k_discrete**2
ax.fill_between(k_discrete, 0, discrete_energy, color='#33FF57', alpha=0.5, label='FTD Discrete Cutoff (Finite Area)')
ax.axvline(x=k_cutoff, color='white', linestyle=':', linewidth=2, label='Brillouin Zone Boundary ($\pi$)')

ax.set_title("Vacuum Energy: Infinite Divergence vs. Discrete Cutoff", fontsize=14)
ax.set_xlabel("Momentum mode ($k$)", fontsize=12)
ax.set_ylabel("Energy Density Contribution", fontsize=12)
ax.set_ylim(0, 20)
ax.legend()
plt.savefig(os.path.join(out_dir, "fig_vacuum.png"), dpi=300, bbox_inches='tight')
plt.close()

# 2. Mass Gap / Wilson Loop Toymodel
fig, ax = plt.subplots(figsize=(6, 6))
# draw grid
for i in range(10):
    ax.axhline(i, color='#333333', linewidth=1)
    ax.axvline(i, color='#333333', linewidth=1)

R = 6
x0, y0 = 2, 2
ax.add_patch(plt.Rectangle((x0, y0), R, R, fill=True, color='#3357FF', alpha=0.4))
ax.add_patch(plt.Rectangle((x0, y0), R, R, fill=False, edgecolor='#FF3333', linewidth=3))

# Arrows representing gauge links
ax.arrow(x0, y0+R, R, 0, head_width=0.3, color='#FF3333', length_includes_head=True)
ax.arrow(x0+R, y0+R, 0, -R, head_width=0.3, color='#FF3333', length_includes_head=True)
ax.arrow(x0+R, y0, -R, 0, head_width=0.3, color='#FF3333', length_includes_head=True)
ax.arrow(x0, y0, 0, R, head_width=0.3, color='#FF3333', length_includes_head=True)

ax.text(x0+R/2, y0+R/2, "Area-Law Flux\n$W(C) \propto \exp(-\sigma \cdot A)$", 
        color='white', ha='center', va='center', fontsize=14, fontweight='bold')

ax.set_title("Topological Wilson Loop: Quark Confinement", fontsize=14)
ax.axis('off')
plt.savefig(os.path.join(out_dir, "fig_mass_gap.png"), dpi=300, bbox_inches='tight')
plt.close()

# 3. 3D Monotile Toymodel
fig = plt.figure(figsize=(8, 8))
ax = fig.add_subplot(111, projection='3d')
ax.set_facecolor('black')
voxels = np.zeros((6,6,6), dtype=bool)
coords = [(0,0,0), (1,0,0), (0,1,0), (0,0,1), (1,1,0), (2,1,0), (1,0,1), (-1,1,1)]
for dx,dy,dz in coords: voxels[dx+2, dy+2, dz+2] = True

colors = np.empty(voxels.shape, dtype=object)
colors[voxels] = '#FF8333'

ax.voxels(voxels, facecolors=colors, edgecolor='white', linewidth=1, alpha=0.9)
ax.set_axis_off()
ax.view_init(elev=25, azim=55)
ax.set_title("3D Aperiodic Monotile (Chiral Polycube Topology)", color='white', fontsize=14)
plt.savefig(os.path.join(out_dir, "fig_monotile.png"), dpi=300, bbox_inches='tight', facecolor='black')
plt.close()

print("Toymodels generated successfully.")
