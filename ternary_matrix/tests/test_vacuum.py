"""
Verification Probe: Vacuum Stability
Chapter 18.2: "Empty lattice evolution -> No spontaneous manifestation"
"""
import sys
import os
import numpy as np

# Add parent to path to import package
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))


from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation

def run_test():
    print("Initializing Null Universe...")
    u = Universe(size=32)
    
    # Verify initial state
    assert np.all(u.states == 0)
    assert np.all(u.flux == 0)
    
    print("Running 100 ticks of evolution...")
    for t in range(100):
        master_equation.tick(u)
        
        # Check invariants
        total_energy = np.sum(u.density)
        manifested_count = np.count_nonzero(u.states)
        
        if manifested_count > 0:
            print(f"FAILED at tick {t}: Spontaneous manifestation detected!")
            return False
            
        if total_energy > 1e-9: # Floating point tolerance
            print(f"FAILED at tick {t}: Energy created from nothing!")
            return False
            
    print("SUCCESS: Vacuum remained stable.")
    return True

if __name__ == "__main__":
    run_test()
