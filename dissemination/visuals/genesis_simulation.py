
import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.colors import LinearSegmentedColormap

# Add repository root to path (Robust relative path)
current_dir = os.path.dirname(os.path.abspath(__file__))
repo_root = os.path.abspath(os.path.join(current_dir, "../../"))
sys.path.append(repo_root)

from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation
from ternary_matrix.config import CONSTANTS

# === CONFIGURATION ===
CONSTANTS.C = 0.5           # Stable Speed
CONSTANTS.ALPHA = 0.00729   # Fine Structure
CONSTANTS.KB = 0.1          # LOW Threshold to encourage lots of creation for the visual
CONSTANTS.GRID_SIZE = 40    # Decent resolution
CONSTANTS.DECAY_RATE = 0.05 # Fast decay to show "cooling"
DAMPING = 0.02

# === INITIALIZATION ===
universe = Universe(size=CONSTANTS.GRID_SIZE)
center = universe.size // 2

# THE BIG BANG: Massive Flux Pulse
# We inject flux in multiple directions to create a "starburst"
universe.flux[center, center, center, 0] = 10.0
universe.flux[center, center, center, 1] = 10.0
universe.flux[center, center, center, 2] = 10.0

# === VISUALIZATION SETUP ===
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 6))
fig.suptitle("FTD GENESIS: Flux Expansion & Matter Manifestation", fontsize=16)

# Flux Map (Left)
# Custom "Void to Fire" colormap
colors_flux = [(0, 0, 0), (0.1, 0, 0.3), (0.8, 0.2, 0), (1, 1, 0.5), (1, 1, 1)]
cmap_flux = LinearSegmentedColormap.from_list("void_fire", colors_flux)
im_flux = ax1.imshow(np.zeros((CONSTANTS.GRID_SIZE, CONSTANTS.GRID_SIZE)), cmap=cmap_flux, vmin=0, vmax=2)
ax1.set_title("Flux Density (Energy)")
ax1.axis('off')

# Matter Map (Right)
# Custom "Ternary" colormap: Blue(-1) - Black(0) - Red(+1)
colors_matter = [(0, 0, 1), (0, 0, 0), (1, 0, 0)] # Neg, Null, Pos
cmap_matter = LinearSegmentedColormap.from_list("ternary", colors_matter, N=3)
im_matter = ax2.imshow(np.zeros((CONSTANTS.GRID_SIZE, CONSTANTS.GRID_SIZE)), cmap=cmap_matter, vmin=-1, vmax=1)
ax2.set_title("Manifested States (+1/-1)")
ax2.axis('off')

FRAMES = 60

def update(frame):
    # Run Physics
    master_equation.tick(universe)
    
    # Extract Slice (Middle Z)
    mid_z = universe.size // 2
    
    # Flux Magnitude
    flux_slice = np.linalg.norm(universe.flux[:, :, mid_z, :], axis=-1)
    im_flux.set_data(flux_slice)
    
    # States
    state_slice = universe.states[:, :, mid_z]
    im_matter.set_data(state_slice)
    
    # Dynamic Title
    fig.suptitle(f"FTD GENESIS: T={frame}", fontsize=16)
    
    return im_flux, im_matter

print("Simulating and Rendering Genesis...")
ani = animation.FuncAnimation(fig, update, frames=FRAMES, interval=100, blit=False)

# Save
output_path = "genesis_event.gif"
ani.save(output_path, writer='pillow', fps=10)
print(f"Animation saved to {output_path}")
