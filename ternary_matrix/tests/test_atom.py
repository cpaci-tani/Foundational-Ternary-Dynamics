"""
Scenario D: Stable Atom (Nucleus + Electron)
Tests complex structure stability.
"""
import sys
import os
import numpy as np

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation

def run_test():
    print("Initializing Universe...")
    u = Universe(size=32)
    c = 16
    
    # 1. Build Nucleus (Stable Triad of +1)
    # Uses the L-shape which we know locks in Moore neighborhood
    n1 = (c, c, c)
    n2 = (c+1, c, c)
    n3 = (c, c+1, c)
    n4 = (c+1, c+1, c) # Complete the square
    
    u.states[n1] = 1
    u.states[n2] = 1
    u.states[n3] = 1
    u.states[n4] = 1

    
    # Give them high flux, but SMOOTH to prevent divergence spikes
    # Gaussian blob centered at c+0.5, c+0.5, c
    J0 = 10.0
    sigma = 1.5
    
    grid_coords = np.indices((32, 32, 32))
    # Coordinates relative to center of nucleus
    rx = grid_coords[0] - (c + 0.5)
    ry = grid_coords[1] - (c + 0.5)
    rz = grid_coords[2] - c
    
    r2 = rx*rx + ry*ry + rz*rz
    u.flux[..., 0] += J0 * np.exp(-r2 / (sigma**2))
    
    # 2. Build Electron (-1)
    e_pos = (c, c, c+2)
    u.states[e_pos] = -1
    
    # Also add smooth flux for Electron
    ex, ey, ez = e_pos
    rx_e = grid_coords[0] - ex
    ry_e = grid_coords[1] - ey
    rz_e = grid_coords[2] - ez
    r2_e = rx_e**2 + ry_e**2 + rz_e**2
    u.flux[..., 0] += J0 * np.exp(-r2_e / (sigma**2))


    
    # 3. Initial "Kick" (simulating angular momentum?)
    # Add flux flow in Y direction for electron?
    u.wave_velocity[e_pos][1] = 1.0 
    
    # Update density
    from ternary_matrix.physics import forces
    forces.calculate_density(u)
    
    print("Running Atom Simulation (20 ticks)...")
    
    # DEBUG: Initial State
    print(f"DEBUG: Initial Nucleons: {np.sum(u.states == 1)}")
    print(f"DEBUG: Initial Electrons: {np.sum(u.states == -1)}")
    print(f"DEBUG: Density at Nucleus P1: {u.density[n1]}")
    
    survived = True
    for t in range(20):

        master_equation.tick(u)
        
        # Check Nucleus
        n_count = np.sum(u.states == 1)
        e_count = np.sum(u.states == -1)
        
        # Check Locks
        n_locked = np.sum(u.is_locked & (u.states == 1))
        
        print(f"T{t}: Nucleons={n_count} (Locked={n_locked}), Electrons={e_count}")
        
        if n_count < 3:
            print("FAILURE: Nucleus decayed.")
            survived = False
            break
            
        if e_count < 1:
            print("FAILURE: Electron lost (Ann? Evap?).")
            # This is expected if orbit isn't stable.
            # But let's see how long it lasts.
            survived = False
            break
            
    if survived:
        print("SUCCESS: Atom Structure Persisted for 20 ticks.")
        return True
    else:
        print("PARTIAL: Structure unstable.")
        return False

if __name__ == "__main__":
    run_test()
