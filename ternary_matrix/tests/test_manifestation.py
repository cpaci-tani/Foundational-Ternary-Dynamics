"""
Scenario A: Genesis and Evaporation
Tests Phase 3 of the Master Equation.
Improved: Isolates center candidate to avoid neighbor interference.
"""
import sys
import os
import numpy as np

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation
from ternary_matrix.config import CONSTANTS
from ternary_matrix.physics import forces

def run_test():
    print("Initializing Universe...")
    u = Universe(size=32)
    center = 16
    
    # 1. TEST GENESIS
    print("\n--- Test 1: Genesis ---")
    
    # ISOLATE CANDIDATE:
    # Center has High Density (5.0)
    u.flux[center, center, center, 0] = 5.0
    
    # Neighbors have Low Density (0.1), but provide Divergence
    # Div = (J(x+1) - J(x-1)) / 2
    # We want Div > 0.
    # Set J(x+1) = 0.2
    # Set J(x-1) = -0.2
    # Div = (0.2 - -0.2)/2 = 0.2
    u.flux[center+1, center, center, 0] = 0.2
    u.flux[center-1, center, center, 0] = -0.2
    
    # Update Density manually before tick
    forces.calculate_density(u)
    
    # Debug checks
    den_c = u.density[center,center,center]
    den_r = u.density[center+1,center,center]
    div_c = forces.calculate_divergence(u)[center, center, center]
    
    print(f"DEBUG: Density C={den_c} (Target > {CONSTANTS.KB})")
    print(f"DEBUG: Density R={den_r} (Target < {CONSTANTS.KB})")
    print(f"DEBUG: Divergence C={div_c} (Target > 0)")
    
    # Set seed
    np.random.seed(42)
    
    master_equation.tick(u)
    
    state = u.states[center,center,center]
    print(f"State after: {state}")
    
    if state == 1:
        print("SUCCESS: Genesis occurred (+1 created).")
    elif state == -1:
        print("PARTIAL: Genesis occurred (-1 created).")
    else:
        print("FAILED: No genesis.")
        return False
        
    # 2. TEST EVAPORATION
    print("\n--- Test 2: Evaporation ---")
    # Force density to 0
    u.flux.fill(0)
    forces.calculate_density(u) # Important: density must match flux
    
    master_equation.tick(u)
    state = u.states[center,center,center]
    print(f"State after: {state}")
    
    if state == 0:
        print("SUCCESS: Evaporation occurred.")
        return True
    else:
        print(f"FAILED: State persisted ({state}).")
        return False

if __name__ == "__main__":
    run_test()
