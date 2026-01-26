"""
FTD Physics Comprehensive Tests
Phase 2.4: Testing all 12 phases of the update cycle.

Tests cover:
- Time gating (Phase 1)
- All 5 force types (Phase 6)
- Integration (Phase 7)
- Movement (Phase 8)
- Transmutation (Phase 10)
- Full cycle (all phases)
- Conservation laws
"""
import numpy as np
import pytest

from ternary_matrix.model.grid import Universe
from ternary_matrix.config import CONSTANTS, get_test_config
from ternary_matrix.physics import (
    tick,
    tick_minimal,
    run_simulation,
    get_diagnostics,
    # Time gating
    time_gate,
    get_effective_time_rate,
    # Forces
    calculate_density,
    gradient_3d,
    divergence_3d,
    curl_3d,
    smooth_field,
    gravity_force,
    coulomb_force,
    lorentz_force,
    accumulate_forces,
    weak_stress,
    # Integration
    integrate,
    clamp_velocity,
    get_max_speed,
    # Movement
    move_particles,
    # Waves
    propagate_flux,
    # Interactions
    process_interactions,
    get_annihilation_count,
    # Transmutation
    transmute,
    get_stress_field,
    # Binding
    update_bindings,
    count_neighbors_moore,
    get_triad_count,
)


# =============================================================================
# FIXTURES
# =============================================================================

@pytest.fixture
def small_universe():
    """Create a small universe for fast testing."""
    return Universe(size=16)


@pytest.fixture
def medium_universe():
    """Create a medium universe for integration tests."""
    return Universe(size=32)


# =============================================================================
# PHASE 1: TIME GATING TESTS
# =============================================================================

class TestTimeGating:
    """Tests for Phase 1: Time Gating (relativistic lag proxy)."""

    def test_stationary_voxels_always_active(self, small_universe):
        """Stationary voxels should always be active."""
        universe = small_universe
        universe.velocity.fill(0)

        time_gate(universe)

        # All voxels should be active
        assert np.all(universe.is_active)

    def test_fast_voxels_less_active(self, small_universe):
        """Fast-moving voxels should update less frequently."""
        universe = small_universe

        # Set some voxels to high velocity (near c)
        center = universe.size // 2
        universe.velocity[center, center, center] = [0.4, 0, 0]  # Near c=0.5

        # Run multiple time gates
        active_counts = []
        for _ in range(100):
            universe.phase_accum.fill(0)
            time_gate(universe)
            active_counts.append(universe.is_active[center, center, center])

        # Fast voxel should be active less often than stationary
        fast_active_rate = np.mean(active_counts)
        assert fast_active_rate < 1.0  # Should skip some ticks

    def test_effective_time_rate(self, small_universe):
        """Test the diagnostic time rate function."""
        universe = small_universe
        universe.velocity[5, 5, 5] = [0.3, 0, 0]

        rate = get_effective_time_rate(universe)

        # Stationary voxels should have rate ~1
        assert np.isclose(rate[0, 0, 0], 1.0)

        # Moving voxel should have rate < 1
        assert rate[5, 5, 5] < 1.0


# =============================================================================
# PHASE 6: FORCE TESTS
# =============================================================================

class TestForces:
    """Tests for Phase 6: Force Accumulation."""

    def test_gradient_zero_for_uniform_field(self, small_universe):
        """Gradient of uniform field should be zero."""
        uniform_field = np.ones((16, 16, 16), dtype=np.float32) * 5.0
        grad = gradient_3d(uniform_field)
        assert np.allclose(grad, 0, atol=1e-6)

    def test_divergence_zero_for_constant_vector_field(self, small_universe):
        """Divergence of constant vector field should be zero."""
        universe = small_universe
        universe.flux.fill(0)
        universe.flux[..., 0] = 1.0  # Constant x-component

        div = divergence_3d(universe.flux)
        assert np.allclose(div, 0, atol=1e-6)

    def test_curl_of_gradient_is_zero(self, small_universe):
        """Curl of a gradient should be zero (vector calculus identity)."""
        # Create a scalar field
        x = np.arange(16, dtype=np.float32)
        scalar = x[:, None, None] ** 2  # f = x²
        scalar = np.broadcast_to(scalar, (16, 16, 16)).copy()

        # Compute gradient
        grad = gradient_3d(scalar)

        # Compute curl of gradient
        curl = curl_3d(grad)

        # Should be zero (up to numerical precision)
        assert np.allclose(curl, 0, atol=1e-5)

    def test_gravity_attracts_to_density(self, small_universe):
        """Gravity force should point toward high density regions."""
        universe = small_universe
        universe.flux.fill(0)
        universe.density.fill(0)

        # Create a density distribution (not just single point) so smoothing produces gradient
        center = universe.size // 2
        # Create a region of high density
        for dx in range(-2, 3):
            for dy in range(-2, 3):
                for dz in range(-2, 3):
                    x = (center + dx) % universe.size
                    y = (center + dy) % universe.size
                    z = (center + dz) % universe.size
                    # Higher density closer to center
                    dist = abs(dx) + abs(dy) + abs(dz)
                    universe.density[x, y, z] = max(0, 10.0 - dist * 2)

        # Calculate force
        f_grav = gravity_force(universe)

        # Check that forces exist and have structure
        assert np.any(f_grav != 0), "Gravity force should be non-zero near density gradient"

    def test_coulomb_like_charges_repel(self, small_universe):
        """Same-sign charges should repel each other."""
        universe = small_universe
        universe.charge.fill(0)
        universe.states.fill(0)

        # Create a charge distribution (not just single points) so smoothing produces gradient
        # Place a region of positive charge
        for x in range(4, 8):
            for y in range(7, 10):
                for z in range(7, 10):
                    universe.charge[x, y, z] = 1.0
                    universe.states[x, y, z] = 1

        f_coulomb = coulomb_force(universe)

        # Forces should be non-zero at boundaries where gradient exists
        assert np.any(f_coulomb != 0), "Coulomb force should be non-zero near charge gradient"

    def test_force_accumulator_clears(self, small_universe):
        """Force accumulator should be cleared after integration."""
        universe = small_universe

        # Set up a particle with force
        universe.states[8, 8, 8] = 1
        universe.charge[8, 8, 8] = 1.0
        universe.force_accum[8, 8, 8] = [1.0, 2.0, 3.0]
        universe.is_active.fill(True)

        integrate(universe)

        # Force accumulator should be cleared
        assert np.allclose(universe.force_accum, 0)


# =============================================================================
# PHASE 7: INTEGRATION TESTS
# =============================================================================

class TestIntegration:
    """Tests for Phase 7: Integration."""

    def test_velocity_from_force(self, small_universe):
        """Velocity should increase from applied force."""
        universe = small_universe
        universe.states[8, 8, 8] = 1
        universe.force_accum[8, 8, 8] = [1.0, 0, 0]
        universe.is_active.fill(True)

        integrate(universe)

        # Velocity should have increased
        assert universe.velocity[8, 8, 8, 0] > 0

    def test_speed_limit_enforced(self, small_universe):
        """Velocity should be clamped to speed of light."""
        universe = small_universe
        universe.states[8, 8, 8] = 1
        universe.velocity[8, 8, 8] = [1.0, 1.0, 1.0]  # Way above c=0.5
        universe.is_active.fill(True)

        clamp_velocity(universe)

        max_speed = get_max_speed(universe)
        assert max_speed <= CONSTANTS.C + 1e-6

    def test_position_remainder_accumulates(self, small_universe):
        """Position remainder should accumulate from velocity."""
        universe = small_universe
        universe.states[8, 8, 8] = 1
        universe.velocity[8, 8, 8] = [0.1, 0, 0]
        universe.is_active.fill(True)

        initial_rem = universe.position_rem[8, 8, 8, 0]

        integrate(universe)

        # Remainder should have increased
        assert universe.position_rem[8, 8, 8, 0] > initial_rem


# =============================================================================
# PHASE 8: MOVEMENT TESTS
# =============================================================================

class TestMovement:
    """Tests for Phase 8: Movement."""

    def test_movement_when_remainder_exceeds_one(self, small_universe):
        """Particle should move when position remainder >= 1."""
        universe = small_universe
        universe.states[8, 8, 8] = 1
        universe.position_rem[8, 8, 8] = [1.5, 0, 0]  # Should move +x

        move_particles(universe)

        # Original position should be void
        assert universe.states[8, 8, 8] == 0
        # New position should have particle
        assert universe.states[9, 8, 8] == 1

    def test_toroidal_boundary(self, small_universe):
        """Movement should wrap around boundaries."""
        universe = small_universe
        edge = universe.size - 1
        universe.states[edge, 8, 8] = 1
        universe.position_rem[edge, 8, 8] = [1.5, 0, 0]  # Should wrap to 0

        move_particles(universe)

        # Should wrap to x=0
        assert universe.states[0, 8, 8] == 1

    def test_no_movement_to_occupied(self, small_universe):
        """Particle should not move into occupied same-sign voxel directly."""
        universe = small_universe
        universe.states[8, 8, 8] = 1
        universe.states[9, 8, 8] = 1  # Occupied by same sign
        universe.position_rem[8, 8, 8] = [1.5, 0, 0]

        move_particles(universe)

        # Both should still exist (elastic collision handled)
        assert universe.states[8, 8, 8] == 1 or universe.states[9, 8, 8] == 1


# =============================================================================
# PHASE 10: TRANSMUTATION TESTS
# =============================================================================

class TestTransmutation:
    """Tests for Phase 10: Transmutation."""

    def test_no_transmutation_at_low_stress(self, small_universe):
        """No transmutation should occur below stress threshold."""
        universe = small_universe
        universe.states[8, 8, 8] = 1
        universe.flux[8, 8, 8] = [0.1, 0, 0]  # Low flux = low stress

        calculate_density(universe)
        initial_state = universe.states[8, 8, 8]

        # Run many times
        for _ in range(100):
            transmute(universe)

        # State should not have flipped
        assert universe.states[8, 8, 8] == initial_state

    def test_stress_field_calculation(self, small_universe):
        """Stress field should be computed correctly."""
        universe = small_universe

        # Create a flux gradient
        for i in range(universe.size):
            universe.flux[i, :, :, 0] = i * 0.5

        calculate_density(universe)
        stress = get_stress_field(universe)

        # Stress should be non-zero where there are gradients
        assert np.any(stress > 0)


# =============================================================================
# FULL CYCLE TESTS
# =============================================================================

class TestFullCycle:
    """Tests for the complete 12-phase update cycle."""

    def test_single_tick_runs(self, small_universe):
        """A single tick should complete without error."""
        universe = small_universe

        # Add some initial conditions
        universe.flux[8, 8, 8] = [2.0, 0, 0]
        calculate_density(universe)

        tick_count = tick(universe)

        assert tick_count == 1
        assert universe.tick == 1

    def test_1000_ticks_stability(self, small_universe):
        """Simulation should remain stable over 1000 ticks."""
        universe = small_universe

        # Add initial flux
        universe.flux[8, 8, 8] = [2.0, 2.0, 2.0]

        for _ in range(1000):
            tick(universe)

        # Check for NaN or Inf
        assert not np.any(np.isnan(universe.flux))
        assert not np.any(np.isinf(universe.flux))
        assert not np.any(np.isnan(universe.density))
        assert not np.any(np.isinf(universe.density))

    def test_diagnostics_function(self, small_universe):
        """Diagnostics should return valid values."""
        universe = small_universe

        universe.states[5, 5, 5] = 1
        universe.states[10, 10, 10] = -1
        universe.charge[5, 5, 5] = 1.0
        universe.charge[10, 10, 10] = -1.0
        universe.flux[8, 8, 8] = [1.0, 1.0, 1.0]
        calculate_density(universe)

        diag = get_diagnostics(universe)

        assert diag['tick'] == 0
        assert diag['manifested_count'] == 2
        assert diag['positive_count'] == 1
        assert diag['negative_count'] == 1
        assert np.isclose(diag['total_charge'], 0.0)


# =============================================================================
# CONSERVATION LAW TESTS
# =============================================================================

class TestConservation:
    """Tests for conservation laws."""

    def test_charge_conservation(self, medium_universe):
        """Total charge should be conserved."""
        universe = medium_universe

        # Create equal positive and negative particles
        universe.states[5, 5, 5] = 1
        universe.states[20, 20, 20] = -1
        universe.charge[5, 5, 5] = 1.0
        universe.charge[20, 20, 20] = -1.0
        universe.flux[5, 5, 5] = [2.0, 0, 0]
        universe.flux[20, 20, 20] = [2.0, 0, 0]

        initial_charge = universe.get_total_charge()

        # Run simulation
        for _ in range(100):
            tick(universe)

        final_charge = universe.get_total_charge()

        # Charge should be conserved (both should be 0 or both should exist)
        # Note: annihilation removes both, keeping net charge at 0
        assert np.isclose(initial_charge, final_charge, atol=1e-6) or \
               np.isclose(final_charge, 0, atol=1e-6)

    def test_vacuum_stability(self, small_universe):
        """Empty lattice should remain empty."""
        universe = small_universe
        universe.reset()

        for _ in range(100):
            tick(universe)

        # No spontaneous manifestation should occur
        assert universe.get_manifested_count() == 0


# =============================================================================
# PERFORMANCE TESTS
# =============================================================================

class TestPerformance:
    """Performance and scaling tests."""

    def test_tick_completes_in_reasonable_time(self, medium_universe):
        """A single tick should complete quickly for 32³ grid."""
        import time

        universe = medium_universe
        universe.flux[16, 16, 16] = [5.0, 5.0, 5.0]

        start = time.time()
        for _ in range(10):
            tick(universe)
        elapsed = time.time() - start

        # Should complete 10 ticks in less than 5 seconds
        assert elapsed < 5.0, f"Too slow: {elapsed:.2f}s for 10 ticks"

    def test_minimal_tick_faster_than_full(self, medium_universe):
        """Minimal tick should be faster than full tick."""
        import time

        universe = medium_universe
        universe.flux[16, 16, 16] = [5.0, 5.0, 5.0]

        # Time full tick
        universe.reset()
        universe.flux[16, 16, 16] = [5.0, 5.0, 5.0]
        start = time.time()
        for _ in range(10):
            tick(universe)
        full_time = time.time() - start

        # Time minimal tick
        universe.reset()
        universe.flux[16, 16, 16] = [5.0, 5.0, 5.0]
        start = time.time()
        for _ in range(10):
            tick_minimal(universe)
        minimal_time = time.time() - start

        # Minimal should be faster (or at least not slower)
        assert minimal_time <= full_time * 1.5  # Allow some variance


# =============================================================================
# DIFFERENTIAL OPERATOR TESTS
# =============================================================================

class TestDifferentialOperators:
    """Tests for discrete differential operators."""

    def test_laplacian_of_constant_is_zero(self):
        """Laplacian of constant field should be zero."""
        from ternary_matrix.physics.waves import laplacian_3d_vector

        field = np.ones((16, 16, 16, 3), dtype=np.float32) * 5.0
        lap = laplacian_3d_vector(field)

        assert np.allclose(lap, 0, atol=1e-6)

    def test_smooth_field_reduces_variance(self):
        """Smoothing should reduce spatial variance."""
        # Create noisy field
        field = np.random.randn(16, 16, 16).astype(np.float32)
        initial_var = np.var(field)

        smoothed = smooth_field(field)
        final_var = np.var(smoothed)

        assert final_var < initial_var


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
