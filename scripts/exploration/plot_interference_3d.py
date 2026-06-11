#!/usr/bin/env python3
"""
plot_interference_3d.py -- Generate a 3D visualization of the 27-block
genesis pattern at A=14, highlighting the missing FCC edges.
"""

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from radial_genesis_cascade import StochasticLatticeField

def plot_3d_cluster(A=14, L=16, ticks=15):
    field = StochasticLatticeField(L, A, seed=42)
    
    for _ in range(ticks):
        field.tick()
        
    c = L // 2
    
    # Collect coordinates for different categories
    center = []
    sc_faces = []
    bcc_corners = []
    fcc_edges_inactive = []
    
    for i in range(field.N):
        z = i % L
        y = (i // L) % L
        x = i // (L * L)
        
        dx = x - c
        dy = y - c
        dz = z - c
        
        # Only look at the 27-block
        if abs(dx) <= 1 and abs(dy) <= 1 and abs(dz) <= 1:
            r = np.sqrt(dx*dx + dy*dy + dz*dz)
            state = field.state[i]
            
            if r < 0.01:
                center.append((dx, dy, dz))
            elif abs(r - 1.0) < 0.01 and state != 0:
                sc_faces.append((dx, dy, dz))
            elif abs(r - 1.732) < 0.01 and state != 0:
                bcc_corners.append((dx, dy, dz))
            elif abs(r - 1.414) < 0.01 and state == 0:
                # The missing edges
                fcc_edges_inactive.append((dx, dy, dz))

    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection='3d')
    
    # Plot center
    if center:
        xs, ys, zs = zip(*center)
        ax.scatter(xs, ys, zs, c='black', marker='*', s=300, label='Center (Manifested)')
        
    # Plot SC faces
    if sc_faces:
        xs, ys, zs = zip(*sc_faces)
        ax.scatter(xs, ys, zs, c='blue', marker='o', s=150, alpha=0.8, label='SC Faces (Manifested)')
        
    # Plot BCC corners
    if bcc_corners:
        xs, ys, zs = zip(*bcc_corners)
        ax.scatter(xs, ys, zs, c='green', marker='s', s=150, alpha=0.8, label='BCC Corners (Manifested)')
        
    # Plot inactive FCC edges
    if fcc_edges_inactive:
        xs, ys, zs = zip(*fcc_edges_inactive)
        ax.scatter(xs, ys, zs, c='red', marker='x', s=100, alpha=0.6, label='FCC Edges (INACTIVE)')

    # Draw grid lines for the 27-block
    for i in [-1, 0, 1]:
        for j in [-1, 0, 1]:
            ax.plot([-1, 1], [i, i], [j, j], color='gray', alpha=0.2)
            ax.plot([i, i], [-1, 1], [j, j], color='gray', alpha=0.2)
            ax.plot([i, i], [j, j], [-1, 1], color='gray', alpha=0.2)

    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.5)
    ax.set_zlim(-1.5, 1.5)
    
    # Hide axes ticks for cleaner look
    ax.set_xticks([-1, 0, 1])
    ax.set_yticks([-1, 0, 1])
    ax.set_zticks([-1, 0, 1])
    
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    
    plt.title(f'3D Genesis Pattern at A={A}\nShowing missing FCC edges due to interference')
    plt.legend(loc='center left', bbox_to_anchor=(1.05, 0.5))
    
    # Set view angle to see the corners and edges clearly
    ax.view_init(elev=20, azim=45)
    
    artifact_path = r"C:\Users\cpaci\.gemini\antigravity\brain\284848ef-2c38-4b75-ace8-33b9a87081e1\interference_3d.png"
    plt.savefig(artifact_path, dpi=300, bbox_inches='tight')
    print(f"3D Plot saved to {artifact_path}")

if __name__ == "__main__":
    plot_3d_cluster()
