"""
FTD Experiment: Final Double Slit (2D Slice)
Simulates a 3D wave but only exports the Z=100 plane to creating a clean "Ripple Tank" visualization.
"""
import numpy as np
from ternary_matrix.config import CONSTANTS
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation
import json
import os
import shutil

# Config
CONSTANTS.GRID_SIZE = 200
CONSTANTS.J0 = 1.0
CONSTANTS.KB = 10.0 # Suppress Matter. Show pure Flux.
CONSTANTS.DECAY_RATE = 0.0 # Lossless medium for ripple tank


def build_barrier(universe, x_pos, slit_width=8, slit_sep=24):
    center_y = universe.size // 2
    # Wall
    universe.states[x_pos:x_pos+4, :, :] = 1
    universe.is_locked[x_pos:x_pos+4, :, :] = True
    
    # Slits
    y1_start = center_y - slit_sep//2 - slit_width
    y1_end = center_y - slit_sep//2
    universe.states[x_pos:x_pos+4, y1_start:y1_end, :] = 0
    universe.is_locked[x_pos:x_pos+4, y1_start:y1_end, :] = False

    y2_start = center_y + slit_sep//2
    y2_end = center_y + slit_sep//2 + slit_width
    universe.states[x_pos:x_pos+4, y2_start:y2_end, :] = 0
    universe.is_locked[x_pos:x_pos+4, y2_start:y2_end, :] = False

def inject_pulse(universe, x_pos, t):
    # Pulse train: Active for 10 ticks, off for 40
    cycle = t % 50
    if cycle < 10:
        val = 10.0 * np.sin( cycle * 0.3 )
        if val > 0:
            universe.flux[x_pos, :, :, 0] = val


def save_slice(universe, tick, output_dir):
    """
    Manually save only Z=100 slice
    """
    z_slice = 100
    frame_data = {"tick": tick, "voxels": []}
    
    # Static Barrier (Matter)
    # We want to see the barrier even if we filter Z
    # Barrier exists at all Z. So take Z=100.
    
    # Get Matter at Z=100
    matter = universe.states[:, :, z_slice]
    mx, my = np.where(matter != 0)
    for i in range(len(mx)):
        frame_data["voxels"].append({
            "x": int(mx[i]), "y": int(my[i]), "z": 100,
            "type": "matter", "val": 1
        })
        
    # Get Flux at Z=100
    # Threshold > 0.1 to see faint waves
    rho = universe.density[:, :, z_slice]
    fx, fy = np.where(rho > 0.1)
    for i in range(len(fx)):
        # Don't overlap matter
        if matter[fx[i], fy[i]] == 0:
            frame_data["voxels"].append({
                "x": int(fx[i]), "y": int(fy[i]), "z": 100,
                "type": "flux", "val": 0, "rho": float(rho[fx[i], fy[i]])
            })

            
    filename = os.path.join(output_dir, f"frame_{tick:04d}.json")
    with open(filename, 'w') as f:
        json.dump(frame_data, f)
    
    return len(frame_data["voxels"])

def run_slit_final():
    print("Initializing 2D Ripple Tank...")
    output_dir = os.path.join("visualizer", "public", "data")
    if os.path.exists(output_dir):
        try:
            shutil.rmtree(output_dir)
        except:
            pass
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        
    u = Universe(size=200)
    build_barrier(u, 100)
    
    print("Running...")
    for t in range(250):
        inject_pulse(u, 20, t)
        master_equation.tick(u)
        
        # Save SLICE
        count = save_slice(u, t, output_dir)
        
        if t % 10 == 0:
            print(f"Tick {t}: {count} voxels (Slice)")

if __name__ == "__main__":
    run_slit_final()
