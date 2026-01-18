"""
Scenario B: Matter-Antimatter Annihilation
Tests Phase 9 of the Master Equation.
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
    center = 16
    
    # Setup: Adjacent Pair
    # [+1][-1]
    u.states[center, center, center] = 1
    u.states[center+1, center, center] = -1
    
    # We must ensure they don't evaporate before annihilating.
    # Evaporation happens in Phase 3. Interactions in Phase 9.
    # So we need Density > KB for both.
    u.flux[center, center, center, 0] = 5.0
    u.flux[center+1, center, center, 0] = 5.0
    from ternary_matrix.physics import forces
    forces.calculate_density(u)
    
    print(f"State A before: {u.states[center, center, center]}")
    print(f"State B before: {u.states[center+1, center, center]}")
    
    master_equation.tick(u)
    
    state_a = u.states[center, center, center]
    state_b = u.states[center+1, center, center]
    
    print(f"State A after: {state_a}")
    print(f"State B after: {state_b}")
    
    if state_a == 0 and state_b == 0:
        print("SUCCESS: Annihilation occurred.")
        return True
    else:
        print("FAILED: Pair persisted.")
        return False

if __name__ == "__main__":
    run_test()
