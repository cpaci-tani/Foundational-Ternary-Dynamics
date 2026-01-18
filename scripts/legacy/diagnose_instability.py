"""
Diagnose the numerical instability in FTD simulation.
"""
import numpy as np
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import waves, forces
from ternary_matrix.config import CONSTANTS

def diagnose_wave_instability():
    """Check if the wave equation is numerically stable."""
    print("="*60)
    print("WAVE EQUATION STABILITY DIAGNOSIS")
    print("="*60)

    print(f"\nConfig parameters:")
    print(f"  C (speed): {CONSTANTS.C}")
    print(f"  DAMPING: {CONSTANTS.DAMPING}")
    print(f"  DECAY_RATE: {CONSTANTS.DECAY_RATE}")
    print(f"  KB (threshold): {CONSTANTS.KB}")

    # CFL condition for wave equation stability: C * dt / dx <= 1
    # In our case: dt = 1, dx = 1, so need C <= 1
    # But the wave equation is: d^2J/dt^2 = C^2 * Laplacian(J)
    # For explicit integration, stability requires C^2 * dt^2 / dx^2 <= some_factor

    print(f"\nCFL-like stability check:")
    print(f"  C^2 = {CONSTANTS.C**2}")
    print(f"  For 3D Laplacian with 6-stencil, max eigenvalue is 12")
    print(f"  Stability typically requires C^2 * 12 < 4, i.e., C^2 < 0.33")
    print(f"  Current C^2 = {CONSTANTS.C**2} {'VIOLATES' if CONSTANTS.C**2 > 0.33 else 'OK'} stability bound")

    # Test with a simple pulse
    u = Universe(size=16)
    u.flux[8, 8, 8, 0] = 1.0

    print(f"\nEvolution of max |flux| over time:")
    for t in range(10):
        max_flux = np.max(np.abs(u.flux))
        total_energy = np.sum(u.flux**2)
        print(f"  Tick {t}: max|J|={max_flux:.4e}, total_energy={total_energy:.4e}")

        waves.propagate_flux(u)

    if np.max(np.abs(u.flux)) > 1e10:
        print("\nDIAGNOSIS: Wave equation is UNSTABLE (exponential growth)")
        print("ROOT CAUSE: C=1.0 violates CFL stability for explicit Verlet integration")
    else:
        print("\nWave equation appears stable")

def diagnose_heptad_decay():
    """Check why Heptads decay."""
    print("\n" + "="*60)
    print("HEPTAD DECAY DIAGNOSIS")
    print("="*60)

    from ternary_matrix.physics import binding, master_equation

    u = Universe(size=16)
    c = 8

    # Create Heptad
    u.states[c, c, c] = 1
    u.states[c+1, c, c] = 1
    u.states[c-1, c, c] = 1
    u.states[c, c+1, c] = 1
    u.states[c, c-1, c] = 1
    u.states[c, c, c+1] = 1
    u.states[c, c, c-1] = 1

    # Initialize flux to support the structure
    # Particles need density > KB to persist
    # But we have NO FLUX initially!

    forces.calculate_density(u)
    binding.update_bindings(u)

    print(f"\nInitial state:")
    print(f"  Particles: {np.count_nonzero(u.states)}")
    print(f"  Locked: {np.count_nonzero(u.is_locked)}")
    print(f"  Density at center: {u.density[c, c, c]:.4f}")
    print(f"  KB threshold: {CONSTANTS.KB}")

    # Check evaporation condition
    print(f"\nEvaporation condition: density < KB")
    print(f"  Center density {u.density[c, c, c]:.4f} < {CONSTANTS.KB}? {u.density[c, c, c] < CONSTANTS.KB}")

    # The issue: particles exist but have no flux, so they evaporate!
    print("\nDIAGNOSIS: Particles created without supporting flux will EVAPORATE")
    print("The model requires flux >= KB for particles to persist")
    print("Creating 'bare' particles without flux is not physical in this model")

def suggest_fixes():
    """Suggest how to fix the simulation."""
    print("\n" + "="*60)
    print("SUGGESTED FIXES")
    print("="*60)

    print("""
1. WAVE EQUATION STABILITY:
   - Reduce C to ~0.5 (satisfies CFL condition)
   - Or use implicit integration (unconditionally stable)
   - Or increase DAMPING significantly

2. PARTICLE PERSISTENCE:
   - Particles need flux density >= KB to exist
   - When creating particles, also inject supporting flux
   - Or modify the model so locked particles don't evaporate

3. PROPER INITIALIZATION:
   - Particles should be created WITH their flux fields
   - A Heptad needs a flux "halo" around it
   - The flux should satisfy |J| >= KB at particle locations

4. CURRENT DAMPING (0.05) vs DECAY_RATE (0.001):
   - DAMPING = 0.05 removes 5% of flux per tick
   - DECAY_RATE = 0.001 for unlocked particles only
   - These compete with the unstable growth
""")

def main():
    diagnose_wave_instability()
    diagnose_heptad_decay()
    suggest_fixes()

if __name__ == "__main__":
    main()
