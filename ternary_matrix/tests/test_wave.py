"""
Verification Probe: Flux Propagation
Chapter 18.2: "Flux Propagation -> Propagation at speed C"
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
    
    # Inject Pulse at Center
    center = 16
    u.flux[center, center, center, 0] = 100.0 # Jx pulse
    
    print("Initial State: Pulse at [16,16,16]")
    print(f"Flux at center: {u.flux[center, center, center, 0]}")
    print(f"Flux at neighbor: {u.flux[center+1, center, center, 0]}")
    
    # Tick 1
    master_equation.tick(u)
    print("\nAfter Tick 1:")
    # At t=1, velocity updates from Laplacian.
    # Lap center = -600.
    # Vel center = -600.
    # Flux center = 100 - 600 = -500. 
    # Valid? Discrete equations can be unstable if C is too high relative to dt=1.
    # Let's check what happened.
    
    c_flux = u.flux[center, center, center, 0]
    n_flux = u.flux[center+1, center, center, 0]
    
    print(f"Flux at center: {c_flux}")
    print(f"Flux at neighbor: {n_flux}")
    
    # Neighbor should have received some +flux from the negative Laplacian at neighbor?
    # Lap neighbor = +100 (from center).
    # Vel neighbor += 100.
    # Flux neighbor += 100.
    
    if n_flux > 0:
        print("SUCCESS: Wave propagated to neighbor.")
        return True
    else:
        print("FAILED: Wave did not propagate.")
        return False

if __name__ == "__main__":
    run_test()
