"""
FTD Heptad Collider
Simulates the collision of two Size-7 Heptads to search for Fusion (Size 13/14).
"""
import os
import numpy as np
import time
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation, forces
from ternary_matrix.analysis.exporter import Exporter
from ternary_matrix.analysis.structure_metrics import analyze_clusters
from ternary_matrix.config import CONSTANTS

def create_heptad(u, center):
    cx, cy, cz = center
    # Center
    u.states[cx, cy, cz] = 1
    # 6 Neighbors (Faces)
    u.states[cx+1, cy, cz] = 1
    u.states[cx-1, cy, cz] = 1
    u.states[cx, cy+1, cz] = 1
    u.states[cx, cy-1, cz] = 1
    u.states[cx, cy, cz+1] = 1
    u.states[cx, cy, cz-1] = 1

def apply_kick(u, center, direction):
    """
    Creates a flux gradient behind the object to push it.
    Direction: vector (1, 0, 0) means push right.
    We place flux at center - direction * distance.
    """
    cx, cy, cz = center
    dx, dy, dz = direction
    
    # Place a "Wave Packet" behind the object
    kick_pos = (cx - dx*5, cy - dy*5, cz - dz*5)
    
    # Gaussian Flux Injection
    J0 = 10.0 # Strong kick
    sigma = 2.0
    
    grid_coords = np.indices(u.shape)
    rx = grid_coords[0] - kick_pos[0]
    ry = grid_coords[1] - kick_pos[1]
    rz = grid_coords[2] - kick_pos[2]
    r2 = rx*rx + ry*ry + rz*rz
    
    u.flux[..., 0] += J0 * np.exp(-r2 / (sigma**2))

def run_collider():
    print("Initializing Heptad Collider (Infinite Space 256^3)...")
    output_dir = os.path.join("visualizer", "public", "data")
    exporter = Exporter(output_dir)
    
    # 256^3 gives effectively infinite space for the event duration
    u = Universe(size=256)
    c_y = 128
    c_z = 128
    
    # Heptad 1 (Left, moving Right) - Start at x=100
    pos1 = (100, c_y, c_z)
    create_heptad(u, pos1)
    apply_kick(u, pos1, (1, 0, 0)) # Kick +X
    
    # Heptad 2 (Right, moving Left) - Start at x=156 (Dist 56)
    pos2 = (156, c_y, c_z) 
    create_heptad(u, pos2)
    apply_kick(u, pos2, (-1, 0, 0)) # Kick -X
    
    # Initial Physics
    forces.calculate_density(u)
    
    max_size_seen = 0
    fusion_detected = False
    
    print("Running Collision Sequence (100 ticks)...")
    for t in range(100):

        # export
        exporter.save_frame(u, t)
        
        # Analyze
        stats = analyze_clusters(u)
        current_max = max(stats.keys()) if stats else 0
        
        # Log interesting events
        if current_max > 7:
            print(f"Tick {t}: DETECTED LARGE CLUSTER: Size {current_max}")
            max_size_seen = max(max_size_seen, current_max)
            if current_max >= 13:
                fusion_detected = True
        
        # Tick
        master_equation.tick(u)
        
    print(f"\nExperiment Complete.")
    print(f"Max Cluster Size: {max_size_seen}")
    if fusion_detected:
        print("SUCCESS: FUSION ACHIEVED (Size 13+).")
        print("The Heptads successfully merged into a complex isomer.")
    else:
        print("RESULT: No stable fusion. Detailed analysis required.")
        if max_size_seen > 7:
            print("Partial fusion or interaction occurred.")
        else:
            print("Objects likely scattered or annihilated.")

if __name__ == "__main__":
    run_collider()
