"""
FTD 3D Metamaterial CAD Renderings
Generates a 4-panel 3D figure for publication and fabrication reference.

Panels:
  A) Topological Phonon Waveguide (Sonoluminescence Lens) — twisted lemniscate extrusion
  B) Fractal Acoustic Funnel (Nested cuboctahedral step-down transformer)
  C) Casimir Ratchet Wafer (Spin-2 ZPE diode surface with sawtooth parity breaking)
  D) 3D Manifestation Landscape (Rotationally symmetric Softplus transduction funnel)
"""
from pathlib import Path

_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.colors import LightSource


def get_cuboc_vertices_edges(scale=1.0):
    """Generates the vertices and edges of a cuboctahedron."""
    v = []
    # Permutations of (+/-1, +/-1, 0)
    for i in [-1, 1]:
        for j in [-1, 1]:
            v.append([i, j, 0])
            v.append([i, 0, j])
            v.append([0, i, j])
    v = np.unique(v, axis=0)

    edges = []
    # Edges connect vertices at distance sqrt(2) in the ideal unscaled lattice
    for i in range(len(v)):
        for j in range(i + 1, len(v)):
            dist = np.linalg.norm(v[i] - v[j])
            if np.isclose(dist, np.sqrt(2)):
                edges.append((i, j))

    v = v.astype(float) * scale
    return v, edges


def generate_ftd_3d_metamaterials():
    """
    Generates 3D solid-mechanics CAD renderings of the FTD Metamaterials.
    """
    plt.style.use('dark_background')
    fig = plt.figure(figsize=(16, 16))
    fig.suptitle('FTD Metamaterial Fabrication CAD: 3D Topological Hardware',
                 fontsize=22, fontweight='bold', y=0.96, color='white')

    # =========================================================================
    # PANEL A: The Sonoluminescence Harvester (Topological Waveguide)
    # =========================================================================
    ax1 = fig.add_subplot(221, projection='3d')

    t = np.linspace(0, 2 * np.pi, 500)
    z_extrude = np.linspace(0, 4, 150)
    T, Z_grid = np.meshgrid(t, z_extrude)

    # The 5-Harmonic Fourier Series (Feigenbaum Cascade)
    x_2d = (np.cos(T) + 0.5 * np.cos(2 * T) + 0.5 * np.cos(4 * T)
            + 0.4 * np.cos(8 * T) + 0.0625 * np.cos(16 * T))
    y_2d = (np.sin(T) - 0.5 * np.sin(2 * T) + 0.5 * np.sin(4 * T)
            - 0.35 * np.sin(8 * T) + 0.0625 * np.sin(16 * T))

    # Twist the 2D curve as it extrudes along Z (Topological Winding w=-2)
    twist_rate = 0.5
    X = x_2d * np.cos(twist_rate * Z_grid) - y_2d * np.sin(twist_rate * Z_grid)
    Y = x_2d * np.sin(twist_rate * Z_grid) + y_2d * np.cos(twist_rate * Z_grid)
    Z = Z_grid

    ls1 = LightSource(azdeg=270, altdeg=45)
    rgb1 = ls1.shade(Z, cmap=plt.cm.cividis, vert_exag=0.1, blend_mode='soft')

    ax1.plot_surface(X, Y, Z, facecolors=rgb1, rstride=2, cstride=2,
                     antialiased=True, alpha=0.9)

    ax1.plot([0, 0], [0, 0], [0, 4], color='red', lw=2, linestyle='--',
             label='Euclidean Void Core')

    ax1.set_title("A) Topological Phonon Waveguide\n(The Sonoluminescence Lens)",
                  color='cyan', pad=15, fontsize=15)
    ax1.set_axis_off()
    ax1.view_init(elev=35, azim=45)

    # =========================================================================
    # PANEL B: The Fractal Phononic Funnel (Nested Feigenbaum Lattice)
    # =========================================================================
    ax2 = fig.add_subplot(222, projection='3d')

    scales = [1.0, 0.5, 0.25, 0.125]
    colors = ['#444444', '#005599', '#0088CC', '#00FFFF']
    alphas = [0.2, 0.4, 0.7, 1.0]

    for s, c, a in zip(scales, colors, alphas):
        v, edges = get_cuboc_vertices_edges(scale=s)
        for (i, j) in edges:
            ax2.plot([v[i, 0], v[j, 0]], [v[i, 1], v[j, 1]], [v[i, 2], v[j, 2]],
                     color=c, lw=2, alpha=a)
        ax2.scatter(v[:, 0], v[:, 1], v[:, 2], color=c, s=20 * s, alpha=a)

    ax2.set_xlim([-1.2, 1.2])
    ax2.set_ylim([-1.2, 1.2])
    ax2.set_zlim([-1.2, 1.2])
    ax2.set_title("B) Fractal Acoustic Funnel\n(Geometric Step-Down Transformer)",
                  color='cyan', pad=15, fontsize=15)
    ax2.set_axis_off()

    # =========================================================================
    # PANEL C: The Casimir Ratchet Wafer (The ZPE Geometric Diode)
    # =========================================================================
    ax3 = fig.add_subplot(223, projection='3d')

    r_wafer = np.linspace(0.1, 1.0, 150)
    theta_wafer = np.linspace(0, 2 * np.pi, 400)
    R, THETA = np.meshgrid(r_wafer, theta_wafer)

    X2 = R * np.cos(THETA)
    Y2 = R * np.sin(THETA)

    # 1. Spatial Parity Breaking (sawtooth ratchet along the radius)
    ratchet_teeth = (R * 10) % 1.0

    # 2. Spin-2 Quadrupolar Moire Envelope (from 137 mod 4 = 1)
    quadrupole = 1 + 0.3 * np.cos(2 * THETA)

    # 3. High-Frequency Lobe Structure (N=27 for CAD rendering stability)
    lobes = np.abs(np.cos(27 * THETA / 2))

    # Combine into Z-axis surface relief
    Z2 = 0.2 * ratchet_teeth * quadrupole * (0.5 + 0.5 * lobes)

    ls2 = LightSource(azdeg=120, altdeg=45)
    rgb2 = ls2.shade(Z2, cmap=plt.cm.copper, vert_exag=0.2, blend_mode='soft')

    ax3.plot_surface(X2, Y2, Z2, facecolors=rgb2, rstride=1, cstride=1,
                     antialiased=True, alpha=1.0)

    ax3.set_title("C) The Casimir Ratchet Wafer\n(Spin-2 ZPE Diode Surface)",
                  color='gold', pad=15, fontsize=15)
    ax3.set_axis_off()
    ax3.view_init(elev=50, azim=-45)

    # =========================================================================
    # PANEL D: The 3D Softplus Thermodynamic Phase Landscape
    # =========================================================================
    ax4 = fig.add_subplot(224, projection='3d')

    X_sp = np.linspace(-2.5, 2.5, 100)
    Y_sp = np.linspace(-2.5, 2.5, 100)
    X_sp, Y_sp = np.meshgrid(X_sp, Y_sp)
    R_sp = np.sqrt(X_sp**2 + Y_sp**2)

    K_B = 1.0
    beta = 8.0

    Z_sp = (1 / beta) * np.log1p(np.exp(np.clip(beta * (R_sp - K_B), -500, 500)))
    Z_sp[Z_sp < 0.02] = 0

    ls3 = LightSource(azdeg=225, altdeg=45)
    rgb3 = ls3.shade(Z_sp, cmap=plt.cm.magma, vert_exag=0.1, blend_mode='soft')

    ax4.plot_surface(X_sp, Y_sp, Z_sp, facecolors=rgb3, linewidth=0,
                     antialiased=True, alpha=0.9)
    # Base plane (The Void)
    ax4.plot_surface(X_sp, Y_sp, np.zeros_like(Z_sp), color='black', alpha=0.5)

    ax4.set_title("D) 3D Manifestation Landscape\n(Topological Transduction Funnel)",
                  color='gold', pad=15, fontsize=15)
    ax4.set_axis_off()
    ax4.view_init(elev=25, azim=45)

    plt.tight_layout()
    fig.subplots_adjust(top=0.92)
    plt.savefig(_FIGDIR / 'FTD_3D_Metamaterials.pdf', format='pdf', dpi=600,
                bbox_inches='tight', facecolor=fig.get_facecolor())
    plt.savefig(_FIGDIR / 'FTD_3D_Metamaterials.png', format='png', dpi=300,
                bbox_inches='tight', facecolor=fig.get_facecolor())
    print(f"Saved to {_FIGDIR}")
    plt.close()


if __name__ == "__main__":
    generate_ftd_3d_metamaterials()
