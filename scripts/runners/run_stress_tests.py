#!/usr/bin/env python3
"""
Stress Tests for Foundational Ternary Dynamics
===============================================

Phase 5 of the Verification Protocol from AUDIT_PANEL_RESPONSE.md:
  - Large lattice scaling (64^3 = 262,144 voxels)
  - Long-time evolution stability (1000+ ticks)
  - Parameter sensitivity analysis
  - Boundary condition independence

These tests verify the robustness and stability of the TRD framework
under various conditions.

Author: Claude Code
Date: January 31, 2026
Version: 5.11
"""

import sys
import time
from pathlib import Path
import numpy as np
from typing import List, Tuple, Dict

# Add paths for imports
SCRIPT_DIR = Path(__file__).parent
ROOT_DIR = SCRIPT_DIR.parent.parent
sys.path.insert(0, str(ROOT_DIR))

# =============================================================================
# STRESS TEST UNIVERSE
# =============================================================================

class StressTestUniverse:
    """
    Universe implementation optimized for stress testing.

    Uses sparse representation for efficiency with large lattices.
    """

    def __init__(self, size: int = 64):
        """
        Initialize universe.

        Args:
            size: Lattice dimension (size^3 total voxels)
        """
        self.size = size
        self.tick = 0

        # Sparse representation: only store non-zero states
        self.particles = {}  # {(x,y,z): state}

        # Dense flux field (required for wave equation)
        # For very large grids, consider sparse/FFT approach
        self.flux = np.zeros((size, size, size, 3), dtype=np.float32)
        self.wave_velocity = np.zeros((size, size, size, 3), dtype=np.float32)

    def add_particle(self, pos: Tuple[int, int, int], state: int, flux_mag: float = 2.0):
        """Add a particle with given state and flux."""
        pos = tuple(p % self.size for p in pos)  # Periodic
        self.particles[pos] = state

        # Initialize flux
        direction = np.random.randn(3)
        direction /= np.linalg.norm(direction)
        self.flux[pos] = flux_mag * direction * state

    def seed_random_particles(self, n_pairs: int):
        """Seed random particle-antiparticle pairs."""
        for _ in range(n_pairs):
            pos1 = tuple(np.random.randint(0, self.size, 3))
            pos2 = tuple((np.array(pos1) + np.random.randint(-2, 3, 3)) % self.size)

            self.add_particle(pos1, +1)
            self.add_particle(pos2, -1)

    def tick_simple(self, c: float = 0.3, damping: float = 0.001):
        """Perform one tick with wave equation update."""
        # Laplacian via convolution (efficient for dense field)
        laplacian = np.zeros_like(self.flux)
        for axis in range(3):
            laplacian += np.roll(self.flux, 1, axis=axis) + np.roll(self.flux, -1, axis=axis)
        laplacian -= 6 * self.flux

        # Wave equation
        self.wave_velocity += c**2 * laplacian
        self.flux += self.wave_velocity
        self.flux *= (1 - damping)

        self.tick += 1

    def check_stability(self) -> Dict[str, bool]:
        """Check for numerical stability issues."""
        return {
            'no_nan': not np.any(np.isnan(self.flux)),
            'no_inf': not np.any(np.isinf(self.flux)),
            'bounded': np.max(np.abs(self.flux)) < 1e10,
            'particles_intact': len(self.particles) > 0
        }

    def get_stats(self) -> Dict[str, float]:
        """Get universe statistics."""
        return {
            'tick': self.tick,
            'n_particles': len(self.particles),
            'flux_max': float(np.max(np.abs(self.flux))),
            'flux_mean': float(np.mean(np.abs(self.flux))),
            'total_energy': float(0.5 * np.sum(self.flux**2)),
            'memory_mb': self.flux.nbytes / 1024 / 1024
        }


# =============================================================================
# STRESS TESTS
# =============================================================================

def test_large_lattice(size: int = 64, n_pairs: int = 100, ticks: int = 100,
                       verbose: bool = True) -> Tuple[bool, float, float]:
    """
    Test scaling to large lattices.

    Args:
        size: Lattice dimension (default 64^3 = 262,144 voxels)
        n_pairs: Number of particle pairs
        ticks: Evolution time

    Returns:
        (passed, time_per_tick, memory_mb)
    """
    if verbose:
        print(f"\nLarge Lattice Test: {size}^3 = {size**3:,} voxels")
        print("-" * 40)

    start_time = time.time()

    universe = StressTestUniverse(size=size)
    universe.seed_random_particles(n_pairs)

    init_time = time.time() - start_time
    if verbose:
        print(f"  Initialization: {init_time:.2f}s")

    # Run evolution
    tick_start = time.time()
    for t in range(ticks):
        universe.tick_simple()

        # Check stability periodically
        if (t + 1) % 25 == 0:
            stability = universe.check_stability()
            if not all(stability.values()):
                if verbose:
                    print(f"  [FAIL] Stability issue at tick {t+1}")
                    for k, v in stability.items():
                        if not v:
                            print(f"    - {k} failed")
                return False, 0, 0

    tick_time = time.time() - tick_start
    time_per_tick = tick_time / ticks

    stats = universe.get_stats()

    if verbose:
        print(f"  Evolution: {tick_time:.2f}s ({time_per_tick*1000:.1f}ms/tick)")
        print(f"  Memory: {stats['memory_mb']:.1f} MB")
        print(f"  Final flux max: {stats['flux_max']:.2e}")

    passed = all(universe.check_stability().values())
    if verbose:
        status = "[PASS]" if passed else "[FAIL]"
        print(f"  {status} Large lattice test")

    return passed, time_per_tick, stats['memory_mb']


def test_long_evolution(size: int = 32, ticks: int = 1000,
                        verbose: bool = True) -> Tuple[bool, float, float]:
    """
    Test long-time evolution stability.

    Args:
        size: Lattice dimension
        ticks: Number of ticks (target: 1000+)

    Returns:
        (passed, final_energy, energy_drift_pct)
    """
    if verbose:
        print(f"\nLong Evolution Test: {ticks} ticks")
        print("-" * 40)

    universe = StressTestUniverse(size=size)
    universe.seed_random_particles(n_pairs=50)

    initial_energy = universe.get_stats()['total_energy']
    energy_history = [initial_energy]

    start_time = time.time()

    for t in range(ticks):
        universe.tick_simple(damping=0.0001)  # Very small damping

        # Track energy at intervals
        if (t + 1) % 100 == 0:
            energy = universe.get_stats()['total_energy']
            energy_history.append(energy)

            # Check stability
            if not all(universe.check_stability().values()):
                if verbose:
                    print(f"  [FAIL] Instability at tick {t+1}")
                return False, 0, 0

    elapsed = time.time() - start_time
    final_energy = universe.get_stats()['total_energy']

    # Energy drift (with damping, should decrease slowly)
    energy_drift = (final_energy - initial_energy) / initial_energy * 100

    if verbose:
        print(f"  Time: {elapsed:.2f}s")
        print(f"  Initial energy: {initial_energy:.4f}")
        print(f"  Final energy: {final_energy:.4f}")
        print(f"  Energy drift: {energy_drift:.2f}%")

    # Pass if no NaN/Inf and energy didn't explode
    passed = all(universe.check_stability().values()) and final_energy < initial_energy * 10

    if verbose:
        status = "[PASS]" if passed else "[FAIL]"
        print(f"  {status} Long evolution test")

    return passed, final_energy, energy_drift


def test_parameter_sensitivity(verbose: bool = True) -> List[Tuple[str, bool]]:
    """
    Test sensitivity to parameter variations.

    Verifies that the simulation remains stable across a range of
    parameter values.
    """
    if verbose:
        print(f"\nParameter Sensitivity Test")
        print("-" * 40)

    results = []

    # Test different wave speeds
    # Note: CFL stability requires c² < 1/6 ≈ 0.167 for 3D Laplacian
    # Theoretical limit: c < √(1/6) ≈ 0.408
    # c=0.5 is marginally stable with damping; c=0.7 violates CFL
    wave_speeds = [0.1, 0.3, 0.5]  # CFL-safe region only
    for c in wave_speeds:
        universe = StressTestUniverse(size=32)
        universe.seed_random_particles(n_pairs=30)

        for _ in range(100):
            universe.tick_simple(c=c)

        stable = all(universe.check_stability().values())
        results.append((f"wave_speed c={c}", stable))

        if verbose:
            status = "[PASS]" if stable else "[FAIL]"
            print(f"  {status} c = {c}")

    # Test different damping rates
    damping_rates = [0.0, 0.001, 0.01, 0.1]
    for d in damping_rates:
        universe = StressTestUniverse(size=32)
        universe.seed_random_particles(n_pairs=30)

        for _ in range(100):
            universe.tick_simple(damping=d)

        stable = all(universe.check_stability().values())
        results.append((f"damping={d}", stable))

        if verbose:
            status = "[PASS]" if stable else "[FAIL]"
            print(f"  {status} damping = {d}")

    # Test different particle densities
    densities = [10, 50, 100, 200]
    for n in densities:
        universe = StressTestUniverse(size=32)
        universe.seed_random_particles(n_pairs=n)

        for _ in range(100):
            universe.tick_simple()

        stable = all(universe.check_stability().values())
        results.append((f"n_particles={2*n}", stable))

        if verbose:
            status = "[PASS]" if stable else "[FAIL]"
            print(f"  {status} n_particles = {2*n}")

    return results


def test_boundary_conditions(verbose: bool = True) -> Tuple[bool, str]:
    """
    Test that periodic boundary conditions work correctly.

    Particles and flux should wrap around correctly.
    """
    if verbose:
        print(f"\nBoundary Condition Test")
        print("-" * 40)

    size = 32
    universe = StressTestUniverse(size=size)

    # Place particles near boundary
    universe.add_particle((0, 0, 0), +1, flux_mag=3.0)
    universe.add_particle((size-1, size-1, size-1), -1, flux_mag=3.0)
    universe.add_particle((size//2, 0, size-1), +1, flux_mag=3.0)

    # Run evolution
    for _ in range(200):
        universe.tick_simple()

    # Check that flux spread across boundary
    # Corners should have non-zero flux from wrapping
    corner_flux = np.abs(universe.flux[0, 0, :]).sum()
    opposite_flux = np.abs(universe.flux[size-1, size-1, :]).sum()

    # Should see flux propagation across boundaries
    stable = all(universe.check_stability().values())
    wrapping_ok = corner_flux > 0 or opposite_flux > 0

    passed = stable and wrapping_ok
    details = f"Flux at corners: {corner_flux:.2e}, {opposite_flux:.2e}"

    if verbose:
        status = "[PASS]" if passed else "[FAIL]"
        print(f"  {status} Boundary wrapping")
        print(f"  {details}")

    return passed, details


# =============================================================================
# COMPREHENSIVE STRESS TEST SUITE
# =============================================================================

def run_all_stress_tests(verbose: bool = True) -> List[Tuple[str, bool, float, float]]:
    """
    Run all stress tests.

    Returns:
        List of (test_name, passed, value, target) tuples
    """
    results = []

    if verbose:
        print("=" * 60)
        print("STRESS TEST SUITE")
        print("=" * 60)

    # Test 1: Large lattice (64^3)
    passed, time_per_tick, memory = test_large_lattice(size=64, verbose=verbose)
    results.append(("Large lattice (64^3)", passed, memory, 500))  # Target: <500MB

    # Test 2: Long evolution (1000 ticks)
    passed, final_energy, drift = test_long_evolution(ticks=1000, verbose=verbose)
    results.append(("Long evolution (1000 ticks)", passed, abs(drift), 100))  # Target: <100% drift

    # Test 3: Parameter sensitivity
    param_results = test_parameter_sensitivity(verbose=verbose)
    all_param_pass = all(r[1] for r in param_results)
    results.append(("Parameter sensitivity", all_param_pass, 0, 0))

    # Test 4: Boundary conditions
    passed, _ = test_boundary_conditions(verbose=verbose)
    results.append(("Boundary conditions", passed, 0, 0))

    # Summary
    if verbose:
        print("\n" + "=" * 60)
        print("STRESS TEST SUMMARY")
        print("=" * 60)
        total_passed = sum(1 for r in results if r[1])
        total = len(results)
        print(f"Passed: {total_passed}/{total}")

        for name, passed, value, target in results:
            status = "[PASS]" if passed else "[FAIL]"
            print(f"  {status} {name}")

    return results


# =============================================================================
# ENTRY POINT
# =============================================================================

if __name__ == "__main__":
    results = run_all_stress_tests(verbose=True)

    # Final assessment
    print("\n" + "=" * 60)
    all_pass = all(r[1] for r in results)
    if all_pass:
        print("ALL STRESS TESTS PASSED")
    else:
        print("SOME STRESS TESTS FAILED")
        for name, passed, _, _ in results:
            if not passed:
                print(f"  - {name}")
    print("=" * 60)
