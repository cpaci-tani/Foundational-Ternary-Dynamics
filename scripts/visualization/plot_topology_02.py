import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D

def map_color_to_flux(color_id):
    jx = 1 if (color_id & 1) else -1
    jy = 1 if (color_id & 2) else -1
    jz = 1 if (color_id & 4) else -1
    if (color_id & 8): 
        jx, jy, jz = 0, jy, jz
    return np.array([jx, jy, jz])

# Topology #02 signature
topology_02 = [3, 7, 2, 8, 6, 12]

# Face centers for +X, -X, +Y, -Y, +Z, -Z
face_centers = [
    np.array([1, 0, 0]), np.array([-1, 0, 0]),
    np.array([0, 1, 0]), np.array([0, -1, 0]),
    np.array([0, 0, 1]), np.array([0, 0, -1])
]

# Face names and colors for the plot
face_names = ['+X Face', '-X Face', '+Y Face', '-Y Face', '+Z Face', '-Z Face']
arrow_colors = ['#FF3333', '#FF8333', '#33FF57', '#33FFB3', '#3357FF', '#F333FF']

fig = plt.figure(figsize=(10, 8), facecolor='#0d0d12')
ax = fig.add_subplot(111, projection='3d')
ax.set_facecolor('#0d0d12')

# Draw a translucent central cube
r = [-1, 1]
X, Y = np.meshgrid(r, r)
# Top and bottom faces
ax.plot_surface(X, Y, np.ones_like(X), alpha=0.1, color='#4da6ff', edgecolor='#ffffff', lw=0.5)
ax.plot_surface(X, Y, -np.ones_like(X), alpha=0.1, color='#4da6ff', edgecolor='#ffffff', lw=0.5)
# Left and right
ax.plot_surface(X, -np.ones_like(X), Y, alpha=0.1, color='#4da6ff', edgecolor='#ffffff', lw=0.5)
ax.plot_surface(X, np.ones_like(X), Y, alpha=0.1, color='#4da6ff', edgecolor='#ffffff', lw=0.5)
# Front and back
ax.plot_surface(-np.ones_like(X), X, Y, alpha=0.1, color='#4da6ff', edgecolor='#ffffff', lw=0.5)
ax.plot_surface(np.ones_like(X), X, Y, alpha=0.1, color='#4da6ff', edgecolor='#ffffff', lw=0.5)

# Plot the flux vectors
for i, color_id in enumerate(topology_02):
    flux = map_color_to_flux(color_id)
    start = face_centers[i]
    
    # Scale flux for visibility
    vector_len = np.linalg.norm(flux)
    if vector_len > 0:
        dir_norm = flux / vector_len
        ax.quiver(start[0], start[1], start[2], 
                  dir_norm[0]*1.5, dir_norm[1]*1.5, dir_norm[2]*1.5,
                  color=arrow_colors[i], linewidth=3, arrow_length_ratio=0.2)
        
        # Add a text label
        label_pos = start + dir_norm * 1.8
        ax.text(label_pos[0], label_pos[1], label_pos[2], 
                f"{face_names[i]}\\nJ = {flux}", 
                color=arrow_colors[i], fontsize=9, fontweight='bold',
                bbox=dict(facecolor='#000000', alpha=0.5, edgecolor='none'))
    else:
        # Zero flux (vacuum mode)
        ax.scatter(start[0], start[1], start[2], color=arrow_colors[i], s=100, marker='o')
        ax.text(start[0], start[1], start[2]+0.5, 
                f"{face_names[i]}\\nJ = [0,0,0]", 
                color=arrow_colors[i], fontsize=9, fontweight='bold',
                bbox=dict(facecolor='#000000', alpha=0.5, edgecolor='none'))

# Remove axes for clean look
ax.set_axis_off()
ax.set_xlim([-3, 3])
ax.set_ylim([-3, 3])
ax.set_zlim([-3, 3])
plt.title("FTD Ground State: Topology #02 Flux Vectors", color='#4da6ff', fontsize=14, pad=20)

output_path = 'C:/Users/cpaci/.gemini/antigravity/brain/8a1dbcc6-029f-4d4d-a292-26fe4e3890b6/topology_02_flux.png'
plt.savefig(output_path, dpi=200, bbox_inches='tight', facecolor='#0d0d12')
print(f"Saved visualization to {output_path}")
