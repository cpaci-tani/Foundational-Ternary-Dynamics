"""
Visual Simulation Runner
Runs the 'Atom' scenario and exports JSON frames.
"""
import sys
import os
import numpy as np

# Adjust path for package import
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation, forces
from ternary_matrix.analysis.exporter import Exporter
from ternary_matrix.analysis.structure_metrics import analyze_clusters

def run_sim():
    print("Setting up simulation (Extreme Scale 200^3)...")
    output_dir = os.path.join("visualizer", "public", "data")
    exporter = Exporter(output_dir)
    
    u = Universe(size=200)
    c = 100


    
    # SETUP: "Big Bang" Atom (Square Nucleus + Smooth Flux)
    # Using the exact setup from test_atom.py that caused the explosion
    
    # Nucleus (4 particles)
    n1 = (c, c, c)
    n2 = (c+1, c, c)
    n3 = (c, c+1, c)
    n4 = (c+1, c+1, c)
    u.states[n1] = 1
    u.states[n2] = 1
    u.states[n3] = 1
    u.states[n4] = 1
    
    # Electron (Orbiting closer this time?)
    e_pos = (c, c, c+3) # Dist 3
    u.states[e_pos] = -1
    
    # Smooth Flux
    J0 = 2.5  # Lower amplitude (was 10.0) -> Less violent gradient
    sigma = 2.0 # Wider spread -> Smoother gradient
    grid_coords = np.indices((200, 200, 200))



    
    # Nucleus Blob
    rx = grid_coords[0] - (c + 0.5)
    ry = grid_coords[1] - (c + 0.5)
    rz = grid_coords[2] - c
    r2 = rx*rx + ry*ry + rz*rz
    u.flux[..., 0] += J0 * np.exp(-r2 / (sigma**2))
    
    # Electron Blob
    ex, ey, ez = e_pos
    rx_e = grid_coords[0] - ex
    ry_e = grid_coords[1] - ey
    rz_e = grid_coords[2] - ez
    r2_e = rx_e**2 + ry_e**2 + rz_e**2
    u.flux[..., 0] += J0 * np.exp(-r2_e / (sigma**2))
    
    # Initial Calculation
    forces.calculate_density(u)
    
    print("Starting Main Loop (50 ticks)...")
    for t in range(50):
        # 1. Export current state
        count = exporter.save_frame(u, t)
        print(f"Tick {t}: Saved {count} voxels.")
        
        # 2. Advance
        master_equation.tick(u)
        
    print("Simulation Complete.")
    print("Running Fibonacci Cluster Analysis...")
    stats = analyze_clusters(u)
    
    print("\n=== CLUSTER SIZE DISTRIBUTION ===")
    # Sort by size
    sorted_sizes = sorted(stats.keys())
    for size in sorted_sizes:
        if size < 20: # Only print small integers relevant to theory
            print(f"Size {size}: {stats[size]} clusters")
        
    # Check specifically for 3, 4, 7, 13
    print("\n--- Fibonacci Check ---")
    for target in [3, 4, 7, 13]:
        count = stats.get(target, 0)
        print(f"Target {target}: {count} found")


if __name__ == "__main__":
    run_sim()
