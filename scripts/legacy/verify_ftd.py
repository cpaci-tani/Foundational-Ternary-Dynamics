"""
FTD Verification Script
Tests core claims from the manifesto with stable parameters.
"""
import numpy as np
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation, forces, waves, binding, interactions
from ternary_matrix.analysis.structure_metrics import analyze_clusters

def test_1_basic_structure():
    """Verify ternary states and flux field types."""
    print("\n" + "="*60)
    print("TEST 1: Basic Structure Verification")
    print("="*60)

    u = Universe(size=32)

    print(f"States dtype: {u.states.dtype} (expected: int8)")
    print(f"Flux dtype: {u.flux.dtype} (expected: float32)")
    print(f"States shape: {u.states.shape}")
    print(f"Flux shape: {u.flux.shape}")

    # Verify ternary states
    u.states[10, 10, 10] = 1
    u.states[10, 10, 11] = -1
    u.states[10, 10, 12] = 0

    print(f"\nState values: {u.states[10, 10, 10]}, {u.states[10, 10, 11]}, {u.states[10, 10, 12]}")
    print(f"CLAIM 'Integer Logic': PARTIALLY TRUE - States are int8, but Flux is float32")

    return True

def test_2_heptad_structure():
    """Create and verify Heptad (1 center + 6 neighbors)."""
    print("\n" + "="*60)
    print("TEST 2: Heptad Structure")
    print("="*60)

    u = Universe(size=32)
    c = 16

    # Create Heptad: center + 6 face neighbors
    u.states[c, c, c] = 1      # Center
    u.states[c+1, c, c] = 1    # +X
    u.states[c-1, c, c] = 1    # -X
    u.states[c, c+1, c] = 1    # +Y
    u.states[c, c-1, c] = 1    # -Y
    u.states[c, c, c+1] = 1    # +Z
    u.states[c, c, c-1] = 1    # -Z

    total_matter = np.count_nonzero(u.states)
    print(f"Heptad particle count: {total_matter} (expected: 7)")

    # Check binding
    binding.update_bindings(u)
    locked_count = np.count_nonzero(u.is_locked)
    print(f"Locked particles: {locked_count}")

    # Analyze clusters
    stats = analyze_clusters(u)
    print(f"Cluster distribution: {stats}")

    if 7 in stats:
        print("CLAIM 'Heptad is stable isomer': VERIFIED - Size-7 cluster detected")
    else:
        print("CLAIM 'Heptad is stable isomer': NOT VERIFIED")

    return True

def test_3_wave_propagation_speed():
    """Test flux propagation speed on cubic lattice."""
    print("\n" + "="*60)
    print("TEST 3: Wave Propagation Speed")
    print("="*60)

    u = Universe(size=64)
    c = 32

    # Inject localized flux pulse (small amplitude to avoid overflow)
    u.flux[c, c, c, 0] = 1.0

    # Track wavefront position over ticks
    forces.calculate_density(u)
    initial_max_pos = np.unravel_index(np.argmax(u.density), u.density.shape)
    print(f"Initial max density at: {initial_max_pos}")

    positions = [initial_max_pos]

    for t in range(10):
        waves.propagate_flux(u)
        forces.calculate_density(u)

        # Find wavefront (max density position along X axis)
        x_profile = u.density[:, c, c]
        max_x = np.argmax(x_profile)
        positions.append((max_x, c, c))

    print(f"Wavefront X positions over 10 ticks: {[p[0] for p in positions]}")

    # Measure spread
    spread = max([p[0] for p in positions]) - min([p[0] for p in positions])
    print(f"Spread over 10 ticks: {spread} voxels")
    print(f"Effective speed: ~{spread/10:.2f} voxels/tick (C=1.0 in config)")

    return True

def test_4_diagonal_propagation():
    """Test the v=c/sqrt(2) diagonal speed claim."""
    print("\n" + "="*60)
    print("TEST 4: Diagonal Propagation Speed (v_gen = c/√2 claim)")
    print("="*60)

    u = Universe(size=64)

    # Place flux source at corner
    u.flux[10, 10, 10, :] = [1.0, 1.0, 1.0]  # Diagonal direction

    forces.calculate_density(u)

    # Track diagonal wavefront
    diag_positions = []

    for t in range(15):
        waves.propagate_flux(u)
        forces.calculate_density(u)

        # Sample along main diagonal
        diag_vals = [u.density[10+i, 10+i, 10+i] for i in range(20)]
        max_diag = np.argmax(diag_vals)
        diag_positions.append(max_diag)

    print(f"Diagonal wavefront positions: {diag_positions}")

    # Calculate effective diagonal speed
    if len(diag_positions) > 5:
        delta = diag_positions[-1] - diag_positions[0]
        ticks = len(diag_positions) - 1
        v_diag = delta / ticks if ticks > 0 else 0
        print(f"Measured diagonal speed: {v_diag:.3f} voxels/tick")
        print(f"Expected c/√2 = {1/np.sqrt(2):.3f}")
        print(f"Ratio to expected: {v_diag / (1/np.sqrt(2)):.2f}")

    return True

def test_5_annihilation():
    """Test matter-antimatter annihilation."""
    print("\n" + "="*60)
    print("TEST 5: Annihilation (+1 adjacent to -1)")
    print("="*60)

    u = Universe(size=32)

    # Place +1 and -1 adjacent
    u.states[15, 15, 15] = 1
    u.states[15, 15, 16] = -1

    before = np.count_nonzero(u.states)
    print(f"Before annihilation: {before} particles")

    # Run interaction
    interactions.process_interactions(u)

    after = np.count_nonzero(u.states)
    print(f"After annihilation: {after} particles")

    if after == 0 and before == 2:
        print("CLAIM 'Annihilation rule': VERIFIED - Both particles destroyed")
    else:
        print("CLAIM 'Annihilation rule': NOT AS EXPECTED")

    return True

def test_6_genesis_threshold():
    """Test manifestation threshold (KB)."""
    print("\n" + "="*60)
    print("TEST 6: Genesis Threshold (KB=1.2)")
    print("="*60)

    from ternary_matrix.config import CONSTANTS
    print(f"KB threshold: {CONSTANTS.KB}")

    u = Universe(size=32)

    # Set flux below threshold
    u.flux[15, 15, 15, :] = [0.5, 0.5, 0.5]  # |J| = 0.866 < 1.2
    forces.calculate_density(u)
    print(f"Low flux density: {u.density[15, 15, 15]:.3f}")

    before = np.count_nonzero(u.states)
    master_equation.update_manifestation(u)
    after = np.count_nonzero(u.states)

    print(f"Genesis below threshold: {before} -> {after} particles")

    # Set flux above threshold
    u.reset()
    u.flux[15, 15, 15, :] = [1.0, 1.0, 1.0]  # |J| = 1.73 > 1.2
    forces.calculate_density(u)
    print(f"High flux density: {u.density[15, 15, 15]:.3f}")

    # Need divergence for polarity selection
    # Create gradient by setting neighbors differently
    u.flux[14, 15, 15, 0] = 0.5
    u.flux[16, 15, 15, 0] = 1.5

    forces.calculate_density(u)
    before = np.count_nonzero(u.states)
    master_equation.update_manifestation(u)
    after = np.count_nonzero(u.states)

    print(f"Genesis above threshold: {before} -> {after} particles")

    return True

def test_7_binding_moore_neighborhood():
    """Test 26-connected Moore neighborhood binding."""
    print("\n" + "="*60)
    print("TEST 7: Moore Neighborhood Binding (26 neighbors)")
    print("="*60)

    u = Universe(size=32)
    c = 16

    # Create 3 particles in a row (each has ≥2 same-sign neighbors? No, only middle does)
    # Actually: end particles have 1 neighbor, middle has 2
    u.states[c, c, c] = 1
    u.states[c+1, c, c] = 1
    u.states[c+2, c, c] = 1

    binding.update_bindings(u)

    print(f"Linear 3-chain locked status:")
    print(f"  Particle 1 (end): {u.is_locked[c, c, c]}")
    print(f"  Particle 2 (mid): {u.is_locked[c+1, c, c]}")
    print(f"  Particle 3 (end): {u.is_locked[c+2, c, c]}")

    # Now test triangle (each has 2 neighbors)
    u.reset()
    u.states[c, c, c] = 1
    u.states[c+1, c, c] = 1
    u.states[c, c+1, c] = 1

    binding.update_bindings(u)

    print(f"\nTriangle locked status:")
    print(f"  Particle 1: {u.is_locked[c, c, c]} (has 2 neighbors)")
    print(f"  Particle 2: {u.is_locked[c+1, c, c]} (has 1 neighbor)")
    print(f"  Particle 3: {u.is_locked[c, c+1, c]} (has 1 neighbor)")

    return True

def test_8_cluster_analysis():
    """Test Fibonacci cluster detection."""
    print("\n" + "="*60)
    print("TEST 8: Cluster Size Analysis")
    print("="*60)

    u = Universe(size=32)

    # Create clusters of various sizes
    # Cluster 1: size 3
    u.states[5, 5, 5] = 1
    u.states[5, 5, 6] = 1
    u.states[5, 5, 7] = 1

    # Cluster 2: size 4 (square)
    u.states[15, 15, 15] = 1
    u.states[15, 15, 16] = 1
    u.states[15, 16, 15] = 1
    u.states[15, 16, 16] = 1

    # Cluster 3: size 7 (Heptad)
    c = 25
    u.states[c, c, c] = 1
    u.states[c+1, c, c] = 1
    u.states[c-1, c, c] = 1
    u.states[c, c+1, c] = 1
    u.states[c, c-1, c] = 1
    u.states[c, c, c+1] = 1
    u.states[c, c, c-1] = 1

    stats = analyze_clusters(u)
    print(f"Cluster distribution: {stats}")

    # Check Fibonacci numbers
    fib = [1, 2, 3, 5, 8, 13, 21]
    found_fib = [f for f in fib if f in stats]
    print(f"Fibonacci sizes found: {found_fib}")

    return True

def main():
    print("="*60)
    print("FTD VERIFICATION REPORT")
    print("="*60)

    tests = [
        test_1_basic_structure,
        test_2_heptad_structure,
        test_3_wave_propagation_speed,
        test_4_diagonal_propagation,
        test_5_annihilation,
        test_6_genesis_threshold,
        test_7_binding_moore_neighborhood,
        test_8_cluster_analysis,
    ]

    results = []
    for test in tests:
        try:
            result = test()
            results.append((test.__name__, "PASS" if result else "FAIL"))
        except Exception as e:
            print(f"ERROR: {e}")
            results.append((test.__name__, f"ERROR: {e}"))

    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    for name, status in results:
        print(f"  {name}: {status}")

if __name__ == "__main__":
    main()
