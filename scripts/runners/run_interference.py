"""
FTD Experiment: The Discrete Double Slit
Can discrete particles surfing a flux field produce interference?
"""
import numpy as np
from ternary_matrix.config import CONSTANTS
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import waves, forces, interactions, binding, master_equation
from ternary_matrix.analysis.structure_metrics import analyze_clusters
from ternary_matrix.analysis.exporter import Exporter
import os
import shutil

# Override Config for this experiment
CONSTANTS.GRID_SIZE = 200
CONSTANTS.J0 = 2.0  # Strong Flux
CONSTANTS.sigma = 4.0 # Broad Wavefront

def build_barrier(universe, x_pos, slit_width=6, slit_sep=16):
    """
    Creates a solid locked wall with two vertical slits.
    """
    center_y = universe.size // 2
    center_z = universe.size // 2
    
    # Wall spans full Y/Z at x_pos
    # We make it 3 voxels thick
    universe.states[x_pos:x_pos+3, :, :] = 1
    universe.is_locked[x_pos:x_pos+3, :, :] = True
    
    # Cut Slit 1
    y1_start = center_y - slit_sep//2 - slit_width
    y1_end = center_y - slit_sep//2
    universe.states[x_pos:x_pos+3, y1_start:y1_end, :] = 0
    universe.is_locked[x_pos:x_pos+3, y1_start:y1_end, :] = False

    # Cut Slit 2
    y2_start = center_y + slit_sep//2
    y2_end = center_y + slit_sep//2 + slit_width
    universe.states[x_pos:x_pos+3, y2_start:y2_end, :] = 0
    universe.is_locked[x_pos:x_pos+3, y2_start:y2_end, :] = False
    
    print(f"Barrier built at X={x_pos} with slits at Y={y1_start}-{y1_end} and Y={y2_start}-{y2_end}")

def create_heptad(universe, center):
    """
    Injects a Heptad (Size 7 Stable Isomer)
    Shape: Center + 6 Orthogonal neighbors
    """
    x, y, z = center
    # Set Matter
    universe.states[x, y, z] = 1
    universe.states[x+1, y, z] = 1
    universe.states[x-1, y, z] = 1
    universe.states[x, y+1, z] = 1
    universe.states[x, y-1, z] = 1
    universe.states[x, y, z+1] = 1
    universe.states[x, y, z-1] = 1
    
    # Force Lock
    universe.is_locked[x, y, z] = True
    # Neighbors usually lock in next tick

def apply_kick(universe, center, velocity_vec):
    """
    Applies Gaussian Flux Impulse to propel the structure.
    """
    x, y, z = center
    vx, vy, vz = velocity_vec
    
    # Add Flux behind the object to push it?
    # Or gradient?
    # We apply a flux blob *at* the object with a gradient
    
    grid_size = universe.size
    xx, yy, zz = np.indices((grid_size, grid_size, grid_size))
    
    # Impulse center slightly behind object
    ix = x - vx * 5
    iy = y - vy * 5
    iz = z - vz * 5
    
    r2 = (xx - ix)**2 + (yy - iy)**2 + (zz - iz)**2
    
    # Add to specific flux component to create directional gradient
    # If we want to move +X, we need High Flux at -X?
    # No, Flux flows High -> Low.
    # To move Matter +X, we need Flux flowing +X?
    # Matter interacts with Flux Gradients.
    # Let's just create a high pressure zone behind it.
    
    universe.flux[..., 0] += CONSTANTS.J0 * vx * np.exp(-r2 / (CONSTANTS.sigma**2))
    universe.flux[..., 1] += CONSTANTS.J0 * vy * np.exp(-r2 / (CONSTANTS.sigma**2))
    universe.flux[..., 2] += CONSTANTS.J0 * vz * np.exp(-r2 / (CONSTANTS.sigma**2))

def run_interference():
    print("Initializing Double Slit Experiment...")
    output_dir = os.path.join("visualizer", "public", "data")
    if os.path.exists(output_dir):
        try:
            shutil.rmtree(output_dir)
        except OSError as e:
            print(f"Warning: Could not clear directory {output_dir}: {e}")
            # Continue anyway, Exporter handles file creation
            
    # Ensure it exists
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        
    exporter = Exporter(output_dir)

    
    u = Universe(size=200)
    
    # 1. Build The Wall
    build_barrier(u, x_pos=100)
    
    # 2. Fire Sequence
    # We will simulate for 200 ticks
    # Fire a particle every 40 ticks
    
    print("Running Interference Stream...")
    
    gun_pos = (40, 100, 100) # Centered in Y/Z, behind wall
    
    for t in range(200):
        # Fire?
        if t % 50 == 10:
            print(f"Tick {t}: Firing Heptad!")
            create_heptad(u, gun_pos)
            apply_kick(u, gun_pos, (2, 0, 0)) # Strong kick +X
        
        # Physics Step
        master_equation.tick(u)

        
        # Export
        exporter.save_frame(u, t)
        
        if t % 10 == 0:
            print(f"Tick {t} complete")

if __name__ == "__main__":
    run_interference()
