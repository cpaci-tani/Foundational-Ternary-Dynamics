"""
Publication-quality cuboctahedron figure.
Uses manual orthographic projection with painter's algorithm
for correct depth sorting — no mplot3d garbage.
"""
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.collections import PatchCollection
from itertools import combinations, product

# ---- Cuboctahedron geometry ----
verts_3d = []
for dx, dy, dz in product([-1, 0, 1], repeat=3):
    if dx**2 + dy**2 + dz**2 == 2:
        verts_3d.append(np.array([dx, dy, dz], dtype=float))
verts_3d = np.array(verts_3d)

# Edges
edges = []
for i in range(len(verts_3d)):
    for j in range(i+1, len(verts_3d)):
        if abs(np.linalg.norm(verts_3d[i] - verts_3d[j]) - np.sqrt(2)) < 0.01:
            edges.append((i, j))

# Triangular faces
tri_faces = []
for i, j, k in combinations(range(len(verts_3d)), 3):
    dists = [np.linalg.norm(verts_3d[a] - verts_3d[b]) for a, b in [(i,j),(i,k),(j,k)]]
    if all(abs(d - np.sqrt(2)) < 0.01 for d in dists):
        tri_faces.append([i, j, k])

# Square faces
sq_faces = []
for axis in range(3):
    for sign in [+1, -1]:
        idxs = [i for i, v in enumerate(verts_3d) if v[axis] == sign]
        if len(idxs) == 4:
            # Sort by angle for proper polygon
            center = np.mean(verts_3d[idxs], axis=0)
            others = [a for a in range(3) if a != axis]
            angles = [np.arctan2(verts_3d[i][others[1]] - center[others[1]], 
                                  verts_3d[i][others[0]] - center[others[0]]) for i in idxs]
            sq_faces.append([idxs[k] for k in np.argsort(angles)])

# ---- Orthographic projection ----
# Rotation: tilt to show structure clearly
elev = 25 * np.pi / 180
azim = 35 * np.pi / 180

def project(pts):
    """Isometric-ish projection"""
    # Rotate around z-axis (azimuth)
    cos_a, sin_a = np.cos(azim), np.sin(azim)
    Rz = np.array([[cos_a, -sin_a, 0], [sin_a, cos_a, 0], [0, 0, 1]])
    # Rotate around x-axis (elevation)
    cos_e, sin_e = np.cos(elev), np.sin(elev)
    Rx = np.array([[1, 0, 0], [0, cos_e, -sin_e], [0, sin_e, cos_e]])
    R = Rx @ Rz
    rotated = (R @ pts.T).T
    return rotated[:, 0], rotated[:, 1], rotated[:, 2]

x2d, y2d, z_depth = project(verts_3d)

# ---- Render with proper painter's algorithm ----
fig, ax = plt.subplots(figsize=(7, 7), facecolor='white')
ax.set_aspect('equal')

# Colors
TRI_COLOR = '#5B9BD5'   # Calm blue
SQ_COLOR  = '#E88E8E'   # Soft coral
EDGE_COL  = '#333333'
VERT_COL  = '#1a1a1a'
BG_TRI    = '#5B9BD5'
BG_SQ     = '#E88E8E'

# Compute face depths for sorting
all_faces = []
for face in tri_faces:
    centroid_z = np.mean([z_depth[i] for i in face])
    all_faces.append(('tri', face, centroid_z))
for face in sq_faces:
    centroid_z = np.mean([z_depth[i] for i in face])
    all_faces.append(('sq', face, centroid_z))

# Sort back-to-front (painter's algorithm)
all_faces.sort(key=lambda x: x[2])

# Draw faces
for ftype, face, _ in all_faces:
    poly_pts = np.array([(x2d[i], y2d[i]) for i in face])
    color = TRI_COLOR if ftype == 'tri' else SQ_COLOR
    polygon = Polygon(poly_pts, closed=True, 
                      facecolor=color, edgecolor=EDGE_COL, 
                      alpha=0.45, linewidth=1.3, zorder=2)
    ax.add_patch(polygon)

# Draw edges (all of them, sorted by depth)
edge_depths = [(i, j, (z_depth[i] + z_depth[j])/2) for i, j in edges]
edge_depths.sort(key=lambda x: x[2])
for i, j, d in edge_depths:
    ax.plot([x2d[i], x2d[j]], [y2d[i], y2d[j]], 
            color=EDGE_COL, linewidth=1.4, zorder=3, solid_capstyle='round')

# Draw vertices (sorted by depth, front on top)
vert_order = np.argsort(z_depth)
for idx in vert_order:
    ax.plot(x2d[idx], y2d[idx], 'o', color=VERT_COL, markersize=7, 
            markeredgecolor='white', markeredgewidth=0.8, zorder=4)

# Coordinate axes (subtle, behind everything)
axis_len = 1.55
axis_labels = ['x', 'y', 'z']
axis_col = '#AAAAAA'
for a in range(3):
    for sign in [1, -1]:
        tip = np.zeros(3)
        tip[a] = sign * axis_len
        tx, ty, _ = project(tip.reshape(1, 3))
        style = '-' if sign > 0 else '--'
        ax.plot([0, tx[0]], [0, ty[0]], style, color=axis_col, 
                linewidth=0.7, zorder=1)
        if sign > 0:
            lx, ly, _ = project((tip * 1.15).reshape(1, 3))
            ax.text(lx[0], ly[0], axis_labels[a], fontsize=12, 
                    color='#888888', ha='center', va='center',
                    fontstyle='italic', fontfamily='serif')

# Title and subtitle
ax.text(0.5, 0.97, 'Cuboctahedron', transform=ax.transAxes,
        fontsize=18, ha='center', va='top', fontfamily='serif',
        fontweight='normal')
ax.text(0.5, 0.935, 'V = 12,  E = 24,  F = 14  (8 triangular + 6 square)',
        transform=ax.transAxes, fontsize=11, ha='center', va='top',
        fontfamily='serif', color='#555555')

# Legend
from matplotlib.patches import Patch
legend_elements = [
    Patch(facecolor=TRI_COLOR, alpha=0.55, edgecolor=EDGE_COL, linewidth=0.8,
          label='Triangular faces (8)'),
    Patch(facecolor=SQ_COLOR, alpha=0.55, edgecolor=EDGE_COL, linewidth=0.8,
          label='Square faces (6)'),
]
ax.legend(handles=legend_elements, loc='lower left', fontsize=10.5,
          framealpha=0.95, edgecolor='#CCCCCC', fancybox=False)

# Clean axes
ax.set_xlim(-2, 2)
ax.set_ylim(-2, 2)
ax.axis('off')

# Save
out = r'c:\Users\cpaci\Desktop\pbr_pedagogy\dissemination\Foundational-Ternary-Dynamics\docs\papers\fig1_cuboctahedron.png'
plt.savefig(out, dpi=300, bbox_inches='tight', facecolor='white', pad_inches=0.15)
print(f'Saved: {out}')
print(f'V={len(verts_3d)} E={len(edges)} F_tri={len(tri_faces)} F_sq={len(sq_faces)}')
print(f'Euler: {len(verts_3d)} - {len(edges)} + {len(tri_faces)+len(sq_faces)} = {len(verts_3d)-len(edges)+len(tri_faces)+len(sq_faces)}')
