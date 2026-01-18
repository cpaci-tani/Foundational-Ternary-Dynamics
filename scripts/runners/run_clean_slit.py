"""
FTD Experiment: Clean Double Slit (Plane Wave)
Tests diffraction of a coherent flux wavefront.
"""
import numpy as np
from ternary_matrix.config import CONSTANTS
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import waves, forces, interactions, binding, master_equation
from ternary_matrix.analysis.exporter import Exporter
import os
import shutil

# Override Config
CONSTANTS.GRID_SIZE = 200 # Fixed size
CONSTANTS.J0 = 1.0        # Moderate Flux
CONSTANTS.KB = 4.5        # High Genesis Threshold - only manifest peaks to show wavefronts


def build_barrier(universe, x_pos, slit_width=8, slit_sep=24):
    """
    Creates a solid locked wall with two vertical slits.
    """
    center_y = universe.size // 2
    
    # Wall
    universe.states[x_pos:x_pos+4, :, :] = 1
    universe.is_locked[x_pos:x_pos+4, :, :] = True
    
    # Slit 1
    y1_start = center_y - slit_sep//2 - slit_width
    y1_end = center_y - slit_sep//2
    universe.states[x_pos:x_pos+4, y1_start:y1_end, :] = 0
    universe.is_locked[x_pos:x_pos+4, y1_start:y1_end, :] = False

    # Slit 2
    y2_start = center_y + slit_sep//2
    y2_end = center_y + slit_sep//2 + slit_width
    universe.states[x_pos:x_pos+4, y2_start:y2_end, :] = 0
    universe.is_locked[x_pos:x_pos+4, y2_start:y2_end, :] = False
    
    print(f"Barrier at X={x_pos}. Slits: {slit_width} width, {slit_sep} sep.")

def inject_plane_wave(universe, x_pos, t):
    """
    Injects a plane wave of Flux along the X-axis.
    Sinusoidal driver.
    """
    # Amplitude
    A = 5.0 
    
    # Frequency
    w = 0.2
    
    val = A * np.sin(w * t)
    
    if val > 0:
        # Inject +X flux
        universe.flux[x_pos, :, :, 0] = val

def run_clean_slit():
    print("Initializing Clean Plane Wave Split...")
    output_dir = os.path.join("visualizer", "public", "data")
    
    if os.path.exists(output_dir):
        try:
            shutil.rmtree(output_dir)
        except OSError:
            pass
            
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        
    exporter = Exporter(output_dir)
    
    u = Universe(size=200)
    
    # 1. Build The Barrier
    build_barrier(u, x_pos=100)
    
    print("Starting Simulation...")
    
    for t in range(250):
        
        # Drive the wave at X=20
        inject_plane_wave(u, x_pos=20, t=t)
        
        # Physics
        master_equation.tick(u)
        
        # Export
        exporter.save_frame(u, t)
        
        if t % 10 == 0:
            count = np.count_nonzero(u.states)
            print(f"Tick {t}: {count} particles.")

if __name__ == "__main__":
    run_clean_slit()
