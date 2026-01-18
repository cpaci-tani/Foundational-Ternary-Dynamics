"""
Mini Collision Test - Verify Heptad collision dynamics.
Uses smaller scale to avoid numerical overflow.
"""
import numpy as np
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation, forces, binding
from ternary_matrix.analysis.structure_metrics import analyze_clusters

def create_heptad(u, center, sign=1):
    """Create a Heptad (1 center + 6 face neighbors)."""
    cx, cy, cz = center
    u.states[cx, cy, cz] = sign
    u.states[cx+1, cy, cz] = sign
    u.states[cx-1, cy, cz] = sign
    u.states[cx, cy+1, cz] = sign
    u.states[cx, cy-1, cz] = sign
    u.states[cx, cy, cz+1] = sign
    u.states[cx, cy, cz-1] = sign

def run_collision_test():
    print("="*60)
    print("HEPTAD COLLISION TEST (Small Scale)")
    print("="*60)

    # Use small grid to avoid overflow
    u = Universe(size=48)
    c = 24

    # Create two Heptads facing each other
    pos1 = (c - 8, c, c)  # Left Heptad
    pos2 = (c + 8, c, c)  # Right Heptad (separation = 16)

    create_heptad(u, pos1, sign=1)
    create_heptad(u, pos2, sign=1)

    # Calculate initial state
    forces.calculate_density(u)
    binding.update_bindings(u)

    initial_stats = analyze_clusters(u)
    print(f"Initial clusters: {initial_stats}")
    print(f"Total particles: {np.count_nonzero(u.states)}")
    print(f"Locked particles: {np.count_nonzero(u.is_locked)}")

    # Apply gentle kicks toward each other (small flux to avoid instability)
    # Heptad 1 gets pushed right (+X)
    u.flux[pos1[0]-2, pos1[1], pos1[2], 0] = 0.5

    # Heptad 2 gets pushed left (-X)
    u.flux[pos2[0]+2, pos2[1], pos2[2], 0] = -0.5

    print("\n--- Evolution ---")

    history = []
    for t in range(30):
        # Analyze before tick
        stats = analyze_clusters(u)
        n_particles = np.count_nonzero(u.states)
        n_locked = np.count_nonzero(u.is_locked)
        max_cluster = max(stats.keys()) if stats else 0

        history.append({
            'tick': t,
            'particles': n_particles,
            'locked': n_locked,
            'max_cluster': max_cluster,
            'clusters': dict(stats)
        })

        if t % 5 == 0 or max_cluster > 7:
            print(f"Tick {t:2d}: {n_particles} particles, {n_locked} locked, max_cluster={max_cluster}")
            if max_cluster > 7:
                print(f"  >>> FUSION CANDIDATE: clusters={stats}")

        # Run one tick
        master_equation.tick(u)

    # Final analysis
    print("\n--- Final State ---")
    final_stats = analyze_clusters(u)
    print(f"Final clusters: {final_stats}")
    print(f"Final particles: {np.count_nonzero(u.states)}")

    # Check for fusion (cluster > 7)
    fusion_detected = any(size > 7 for size in final_stats.keys())
    if fusion_detected:
        print("\nRESULT: FUSION DETECTED")
    else:
        print("\nRESULT: No fusion (clusters remained separate or annihilated)")

    return history

def analyze_history(history):
    """Analyze collision timeline."""
    print("\n" + "="*60)
    print("COLLISION TIMELINE ANALYSIS")
    print("="*60)

    # Find when particles started disappearing (contact)
    initial_particles = history[0]['particles']

    contact_tick = None
    for h in history:
        if h['particles'] < initial_particles:
            contact_tick = h['tick']
            break

    if contact_tick:
        print(f"First contact (particle loss): Tick {contact_tick}")

        # Calculate implied speed
        initial_separation = 16  # voxels between Heptad centers
        # Each Heptad has radius ~1, so gap is ~14 voxels
        gap = 14
        if contact_tick > 0:
            approach_speed = gap / contact_tick / 2  # /2 because both moving
            print(f"Implied approach speed: {approach_speed:.3f} voxels/tick per Heptad")
    else:
        print("No contact detected (particles preserved)")

    # Find max cluster achieved
    max_ever = max(h['max_cluster'] for h in history)
    print(f"Maximum cluster size achieved: {max_ever}")

def run_static_stability_test():
    """Test if Heptads are truly stable without flux kicks."""
    print("\n" + "="*60)
    print("HEPTAD STABILITY TEST (No Flux)")
    print("="*60)

    u = Universe(size=32)
    c = 16

    create_heptad(u, (c, c, c), sign=1)

    forces.calculate_density(u)
    binding.update_bindings(u)

    print(f"Initial: {np.count_nonzero(u.states)} particles, {np.count_nonzero(u.is_locked)} locked")

    # Run without flux injection
    for t in range(20):
        master_equation.tick(u)

    final_count = np.count_nonzero(u.states)
    final_locked = np.count_nonzero(u.is_locked)
    print(f"After 20 ticks: {final_count} particles, {final_locked} locked")

    if final_count == 7:
        print("RESULT: Heptad is STABLE (all 7 particles preserved)")
    elif final_count > 0:
        print(f"RESULT: Heptad partially decayed ({7-final_count} particles lost)")
    else:
        print("RESULT: Heptad completely evaporated")

def main():
    run_static_stability_test()
    history = run_collision_test()
    analyze_history(history)

if __name__ == "__main__":
    main()
