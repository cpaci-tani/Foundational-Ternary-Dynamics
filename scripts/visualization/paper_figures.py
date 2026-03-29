"""
Publication Figures for the FTD Paper Path

Generates high-quality matplotlib renderings of the nested polyhedra
from multiple canonical viewing angles, suitable for LaTeX papers.
"""

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Line3DCollection
import os

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'docs', 'papers', 'src', 'figures')
os.makedirs(OUTPUT_DIR, exist_ok=True)

# =====================================================
# GEOMETRY DEFINITIONS
# =====================================================

# SC: octahedron vertices (distance 1)
SC = np.array([[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]], dtype=float)

# FCC: cuboctahedron vertices (distance sqrt(2))
FCC = []
for a in [-1, 1]:
    for b in [-1, 1]:
        FCC.append([a, b, 0])
        FCC.append([a, 0, b])
        FCC.append([0, a, b])
FCC = np.array(FCC, dtype=float)

# BCC: cube vertices (distance sqrt(3))
BCC = np.array([[a, b, c] for a in [-1,1] for b in [-1,1] for c in [-1,1]], dtype=float)

def get_edges(verts, target_dist, tol=0.05):
    edges = []
    for i in range(len(verts)):
        for j in range(i+1, len(verts)):
            d = np.linalg.norm(verts[i] - verts[j])
            if abs(d - target_dist) < tol:
                edges.append([verts[i], verts[j]])
    return edges

OCT_EDGES = get_edges(SC, np.sqrt(2))
CUBOCT_EDGES = get_edges(FCC, np.sqrt(2))
CUBE_EDGES = get_edges(BCC, 2.0)

# Rotation axes
AXES_C4 = np.array([[1,0,0],[0,1,0],[0,0,1]], dtype=float)
AXES_C3 = np.array([[1,1,1],[1,1,-1],[1,-1,1],[-1,1,1]], dtype=float)
AXES_C3 = AXES_C3 / np.sqrt(3)
AXES_C2 = np.array([[1,1,0],[1,-1,0],[1,0,1],[1,0,-1],[0,1,1],[0,1,-1]], dtype=float)
AXES_C2 = AXES_C2 / np.sqrt(2)


def draw_nested_polyhedra(ax, elev=25, azim=45, show_axes=True, show_labels=True, title=None):
    """Draw the three nested polyhedra on a 3D axis."""

    ax.set_facecolor('#08080f')
    ax.xaxis.pane.fill = False
    ax.yaxis.pane.fill = False
    ax.zaxis.pane.fill = False
    ax.xaxis.pane.set_edgecolor('#222')
    ax.yaxis.pane.set_edgecolor('#222')
    ax.zaxis.pane.set_edgecolor('#222')
    ax.grid(False)

    # Edges
    for edge in CUBE_EDGES:
        xs, ys, zs = zip(*edge)
        ax.plot(xs, ys, zs, color='#3366cc', alpha=0.3, linewidth=0.8)

    for edge in CUBOCT_EDGES:
        xs, ys, zs = zip(*edge)
        ax.plot(xs, ys, zs, color='#33cc66', alpha=0.4, linewidth=1.0)

    for edge in OCT_EDGES:
        xs, ys, zs = zip(*edge)
        ax.plot(xs, ys, zs, color='#cc3333', alpha=0.5, linewidth=1.2)

    # Rotation axes
    if show_axes:
        L = 1.6
        for a in AXES_C4:
            ax.plot([-a[0]*L, a[0]*L], [-a[1]*L, a[1]*L], [-a[2]*L, a[2]*L],
                    '--', color='#ff9944', alpha=0.3, linewidth=0.8)
        for a in AXES_C3:
            ax.plot([-a[0]*L, a[0]*L], [-a[1]*L, a[1]*L], [-a[2]*L, a[2]*L],
                    '--', color='#44ddaa', alpha=0.25, linewidth=0.7)
        for a in AXES_C2:
            ax.plot([-a[0]*L, a[0]*L], [-a[1]*L, a[1]*L], [-a[2]*L, a[2]*L],
                    '--', color='#aa88ff', alpha=0.2, linewidth=0.6)

    # Points
    ax.scatter(*BCC.T, s=50, c='#4488ff', alpha=0.9, zorder=5, edgecolors='#2255aa', linewidth=0.5)
    ax.scatter(*FCC.T, s=40, c='#44ff88', alpha=0.9, zorder=6, edgecolors='#22aa44', linewidth=0.5)
    ax.scatter(*SC.T, s=60, c='#ff4444', alpha=0.9, zorder=7, edgecolors='#aa2222', linewidth=0.5)

    # Origin
    ax.scatter([0], [0], [0], s=30, c='white', alpha=0.8, zorder=8, edgecolors='#888', linewidth=0.5)

    ax.view_init(elev=elev, azim=azim)
    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.5)
    ax.set_zlim(-1.5, 1.5)
    ax.set_axis_off()

    if title:
        ax.set_title(title, color='white', fontsize=11, pad=-5)


def fig_multi_angle():
    """Figure 1: Four canonical viewing angles."""
    fig = plt.figure(figsize=(14, 14), facecolor='#08080f')

    views = [
        (25, 45, 'General view'),
        (0, 0, 'Along C4 axis (x-axis)\nZ$_4$ symmetry manifest'),
        (35.26, 45, 'Along C3 axis (body diagonal)\nZ$_3$ symmetry manifest'),
        (0, 45, 'Along C2 axis (edge)\nZ$_2$ symmetry manifest'),
    ]

    for i, (elev, azim, title) in enumerate(views):
        ax = fig.add_subplot(2, 2, i+1, projection='3d')
        draw_nested_polyhedra(ax, elev=elev, azim=azim, title=title)

    fig.suptitle('The Three Shells of $\\mathbb{Z}^3$: Octahedron $\\subset$ Cuboctahedron $\\subset$ Cube',
                 color='white', fontsize=14, y=0.98)

    # Legend
    fig.text(0.5, 0.02,
             'Red: Octahedron (SC, 6 pts, U(1))    '
             'Green: Cuboctahedron (FCC, 12 pts, SU(2))    '
             'Blue: Cube (BCC, 8 pts, SU(3))',
             ha='center', color='#aaa', fontsize=10)

    plt.tight_layout(rect=[0, 0.04, 1, 0.96])
    return fig


def fig_c4_face_on():
    """Figure 2: Looking straight down a C4 axis — Z4 symmetry."""
    fig = plt.figure(figsize=(8, 8), facecolor='#08080f')
    ax = fig.add_subplot(111, projection='3d')
    draw_nested_polyhedra(ax, elev=0, azim=0, show_axes=True)
    ax.set_title('View along $C_4$ axis: $\\mathbb{Z}_4$ symmetry\n'
                 'This symmetry forces the lemniscatic modulus $k = 1/\\sqrt{2}$',
                 color='white', fontsize=11, pad=10)
    plt.tight_layout()
    return fig


def fig_c3_body_diagonal():
    """Figure 3: Looking down the body diagonal — C3 axis."""
    fig = plt.figure(figsize=(8, 8), facecolor='#08080f')
    ax = fig.add_subplot(111, projection='3d')
    draw_nested_polyhedra(ax, elev=35.26, azim=45, show_axes=True)
    ax.set_title('View along $C_3$ axis: body diagonal\n'
                 '4 three-fold axes = $N_{\\mathrm{base}} = 4$',
                 color='white', fontsize=11, pad=10)
    plt.tight_layout()
    return fig


def fig_shells_separated():
    """Figure 4: Three shells shown separately side by side."""
    fig = plt.figure(figsize=(16, 5), facecolor='#08080f')

    elev, azim = 25, 45

    # Shell 1: Octahedron
    ax1 = fig.add_subplot(131, projection='3d')
    ax1.set_facecolor('#08080f')
    for edge in OCT_EDGES:
        xs, ys, zs = zip(*edge)
        ax1.plot(xs, ys, zs, color='#cc3333', alpha=0.6, linewidth=1.5)
    ax1.scatter(*SC.T, s=80, c='#ff4444', alpha=0.95, zorder=5, edgecolors='#aa2222', linewidth=0.5)
    ax1.scatter([0], [0], [0], s=30, c='white', alpha=0.5, zorder=8)
    ax1.view_init(elev=elev, azim=azim)
    ax1.set_xlim(-1.5, 1.5); ax1.set_ylim(-1.5, 1.5); ax1.set_zlim(-1.5, 1.5)
    ax1.set_axis_off()
    ax1.set_title('Octahedron (SC)\n6 vertices, $d=1$\nU(1) — Electromagnetism',
                  color='#ff6666', fontsize=10, pad=-5)

    # Shell 2: Cuboctahedron
    ax2 = fig.add_subplot(132, projection='3d')
    ax2.set_facecolor('#08080f')
    for edge in CUBOCT_EDGES:
        xs, ys, zs = zip(*edge)
        ax2.plot(xs, ys, zs, color='#33cc66', alpha=0.5, linewidth=1.2)
    ax2.scatter(*FCC.T, s=60, c='#44ff88', alpha=0.95, zorder=5, edgecolors='#22aa44', linewidth=0.5)
    ax2.scatter([0], [0], [0], s=30, c='white', alpha=0.5, zorder=8)
    ax2.view_init(elev=elev, azim=azim)
    ax2.set_xlim(-1.5, 1.5); ax2.set_ylim(-1.5, 1.5); ax2.set_zlim(-1.5, 1.5)
    ax2.set_axis_off()
    ax2.set_title('Cuboctahedron (FCC)\n12 vertices, $d=\\sqrt{2}$\nSU(2) — Weak / Bridge',
                  color='#66ff99', fontsize=10, pad=-5)

    # Shell 3: Cube
    ax3 = fig.add_subplot(133, projection='3d')
    ax3.set_facecolor('#08080f')
    for edge in CUBE_EDGES:
        xs, ys, zs = zip(*edge)
        ax3.plot(xs, ys, zs, color='#3366cc', alpha=0.5, linewidth=1.2)
    ax3.scatter(*BCC.T, s=70, c='#4488ff', alpha=0.95, zorder=5, edgecolors='#2255aa', linewidth=0.5)
    ax3.scatter([0], [0], [0], s=30, c='white', alpha=0.5, zorder=8)
    ax3.view_init(elev=elev, azim=azim)
    ax3.set_xlim(-1.5, 1.5); ax3.set_ylim(-1.5, 1.5); ax3.set_zlim(-1.5, 1.5)
    ax3.set_axis_off()
    ax3.set_title('Cube (BCC)\n8 vertices, $d=\\sqrt{3}$\nSU(3) — Strong',
                  color='#6699ff', fontsize=10, pad=-5)

    fig.suptitle('The Three Coordination Shells of $\\mathbb{Z}^3$',
                 color='white', fontsize=13, y=0.98)
    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


def fig_axis_structure():
    """Figure 5: The 13 rotation axes of the cuboctahedron."""
    fig = plt.figure(figsize=(8, 8), facecolor='#08080f')
    ax = fig.add_subplot(111, projection='3d')
    ax.set_facecolor('#08080f')

    # Draw cuboctahedron lightly
    for edge in CUBOCT_EDGES:
        xs, ys, zs = zip(*edge)
        ax.plot(xs, ys, zs, color='#33cc66', alpha=0.15, linewidth=0.8)
    ax.scatter(*FCC.T, s=25, c='#44ff88', alpha=0.4, zorder=5)

    # Draw axes prominently
    L = 1.8
    for i, a in enumerate(AXES_C4):
        ax.plot([-a[0]*L, a[0]*L], [-a[1]*L, a[1]*L], [-a[2]*L, a[2]*L],
                '-', color='#ff9944', alpha=0.8, linewidth=2.5)
        ax.scatter([a[0]*L], [a[1]*L], [a[2]*L], s=40, c='#ff9944', zorder=10)
        if i == 0:
            ax.text(a[0]*L*1.15, a[1]*L*1.15, a[2]*L*1.15, '$C_4$', color='#ff9944', fontsize=10)

    for i, a in enumerate(AXES_C3):
        ax.plot([-a[0]*L, a[0]*L], [-a[1]*L, a[1]*L], [-a[2]*L, a[2]*L],
                '-', color='#44ddaa', alpha=0.7, linewidth=2.0)
        ax.scatter([a[0]*L], [a[1]*L], [a[2]*L], s=35, c='#44ddaa', zorder=10)
        if i == 0:
            ax.text(a[0]*L*1.15, a[1]*L*1.15, a[2]*L*1.15, '$C_3$', color='#44ddaa', fontsize=10)

    for i, a in enumerate(AXES_C2):
        ax.plot([-a[0]*L, a[0]*L], [-a[1]*L, a[1]*L], [-a[2]*L, a[2]*L],
                '-', color='#aa88ff', alpha=0.6, linewidth=1.5)
        ax.scatter([a[0]*L], [a[1]*L], [a[2]*L], s=30, c='#aa88ff', zorder=10)
        if i == 0:
            ax.text(a[0]*L*1.15, a[1]*L*1.15, a[2]*L*1.15, '$C_2$', color='#aa88ff', fontsize=10)

    ax.view_init(elev=20, azim=35)
    ax.set_xlim(-2, 2); ax.set_ylim(-2, 2); ax.set_zlim(-2, 2)
    ax.set_axis_off()
    ax.set_title('13 Rotation Axes of the Cuboctahedron = $N_{\\mathrm{eff}}$\n'
                 '3($C_4$) + 4($C_3$) + 6($C_2$) = 13\n'
                 '3 axis types = $N_{\\mathrm{gen}}$ (fermion generations)',
                 color='white', fontsize=11, pad=10)
    plt.tight_layout()
    return fig


# =====================================================
# GENERATE ALL FIGURES
# =====================================================

if __name__ == '__main__':
    figures = [
        ('fig_nested_4views.png', fig_multi_angle, 'Four canonical views'),
        ('fig_c4_axis.png', fig_c4_face_on, 'C4 axis view (Z4 symmetry)'),
        ('fig_c3_axis.png', fig_c3_body_diagonal, 'C3 body diagonal view'),
        ('fig_shells_separated.png', fig_shells_separated, 'Three shells separated'),
        ('fig_rotation_axes.png', fig_axis_structure, '13 rotation axes'),
    ]

    for filename, func, desc in figures:
        print(f"  Generating {desc}...")
        fig = func()
        path = os.path.join(OUTPUT_DIR, filename)
        fig.savefig(path, dpi=200, bbox_inches='tight', facecolor=fig.get_facecolor())
        plt.close(fig)
        print(f"    Saved: {path}")

    print(f"\nAll {len(figures)} figures saved to {OUTPUT_DIR}")
