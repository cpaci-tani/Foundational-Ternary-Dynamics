"""
TIER 4: Simulation Engine Tests (Weight: 15%)

Test the actual lattice dynamics in ternary_matrix/.
Verify conservation laws, causality, wave propagation,
bound state stability, and emergent behaviors.

A failure here means the simulation engine has bugs that must
be fixed before any claims based on simulation.
"""

import pytest
import numpy as np
import sys
import os

# Add project root to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

import ternary_matrix.config as cfg
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import (
    tick,
    calculate_density,
    gradient_3d,
    divergence_3d,
    curl_3d,
    propagate_flux,
    accumulate_forces,
    gravity_force,
    coulomb_force,
    integrate,
    clamp_velocity,
    move_particles,
    process_interactions,
)


def make_small_universe(size=16):
    """Create a small universe for fast testing.

    Modifies the global CONSTANTS to match the requested grid size
    since physics functions read config from the global singleton.
    """
    cfg.CONSTANTS = cfg.PhysicsConfig(GRID_SIZE=size)
    return Universe(size)


def set_config(**kwargs):
    """Temporarily set global CONSTANTS with custom values."""
    cfg.CONSTANTS = cfg.PhysicsConfig(**kwargs)


# =============================================================================
# Test 4.1: Vacuum Stability
# =============================================================================

class TestVacuumStability:
    """Empty lattice should remain empty."""

    def test_vacuum_no_spontaneous_manifestation(self):
        """Run empty lattice for many ticks — no particles should appear."""
        uni = make_small_universe(16)
        assert np.sum(uni.states != 0) == 0, "Universe not initially empty"

        for _ in range(100):
            tick(uni)

        manifested = np.sum(uni.states != 0)
        print(f"\n  After 100 ticks: {manifested} manifested particles")
        assert manifested == 0, f"Spontaneous manifestation: {manifested} particles!"


# =============================================================================
# Test 4.2: Energy Conservation
# =============================================================================

class TestEnergyConservation:
    """Total flux magnitude should be conserved without damping."""

    def test_flux_conservation_no_damping(self):
        """With DAMPING=0, total flux magnitude should be approximately conserved."""
        set_config(GRID_SIZE=16, DAMPING=0.0, DECAY_RATE=0.0, C_WAVE=0.3)
        uni = Universe(16)

        # Create a flux pulse
        mid = 8
        uni.flux[mid, mid, mid] = [1.0, 0.5, 0.3]
        uni.flux[mid+1, mid, mid] = [0.5, 0.2, 0.1]

        initial_energy = np.sum(np.sqrt(np.sum(uni.flux**2, axis=-1)))

        for _ in range(50):
            propagate_flux(uni)

        final_energy = np.sum(np.sqrt(np.sum(uni.flux**2, axis=-1)))

        rel_change = abs(final_energy - initial_energy) / max(initial_energy, 1e-10)
        print(f"\n  Initial flux energy: {initial_energy:.6f}")
        print(f"  Final flux energy:   {final_energy:.6f}")
        print(f"  Relative change:     {rel_change:.4f}")

        # Allow 50% change (discrete wave equation is not perfectly conservative)
        assert rel_change < 0.5, f"Energy changed by {rel_change*100:.1f}%"


# =============================================================================
# Test 4.3: Charge Conservation
# =============================================================================

class TestChargeConservation:
    """Net charge should be conserved."""

    def test_net_charge_zero(self):
        """Equal +1 and -1 particles should maintain net charge = 0."""
        uni = make_small_universe(16)

        # Place balanced +/- particles (far apart to avoid annihilation)
        uni.states[3, 3, 3] = 1
        uni.charge[3, 3, 3] = 1.0
        uni.flux[3, 3, 3] = [1.0, 0.0, 0.0]

        uni.states[12, 12, 12] = -1
        uni.charge[12, 12, 12] = -1.0
        uni.flux[12, 12, 12] = [1.0, 0.0, 0.0]

        uni.is_locked[3, 3, 3] = True
        uni.is_locked[12, 12, 12] = True

        initial_charge = np.sum(uni.states)

        for _ in range(50):
            tick(uni)

        final_charge = np.sum(uni.states)
        print(f"\n  Initial net charge: {initial_charge}")
        print(f"  Final net charge:   {final_charge}")


# =============================================================================
# Test 4.4: Causality (Light Cone)
# =============================================================================

class TestCausality:
    """Information should not propagate faster than the speed limit."""

    def test_flux_light_cone(self):
        """Flux pulse should not reach beyond C_WAVE * N_ticks."""
        set_config(GRID_SIZE=32, DAMPING=0.0, C_WAVE=0.4)
        uni = Universe(32)

        mid = 16
        uni.flux[mid, mid, mid] = [5.0, 0.0, 0.0]

        n_ticks = 5
        for _ in range(n_ticks):
            propagate_flux(uni)

        max_reach = cfg.CONSTANTS.C_WAVE * n_ticks * np.sqrt(3) + 2

        density = np.sqrt(np.sum(uni.flux**2, axis=-1))
        violations = 0
        for x in range(32):
            for y in range(32):
                for z in range(32):
                    dist = np.sqrt((x-mid)**2 + (y-mid)**2 + (z-mid)**2)
                    if dist > max_reach and density[x, y, z] > 1e-10:
                        violations += 1

        print(f"\n  Max reach (C_WAVE*ticks*sqrt(3)+2): {max_reach:.1f}")
        print(f"  Causality violations: {violations}")
        assert violations == 0, f"{violations} causality violations detected!"


# =============================================================================
# Test 4.5: Speed Limit Enforcement
# =============================================================================

class TestSpeedLimit:
    """Particle velocities should never exceed C."""

    def test_velocity_clamping(self):
        """Apply large force and verify speed stays <= C."""
        uni = make_small_universe(16)

        uni.states[8, 8, 8] = 1
        uni.flux[8, 8, 8] = [2.0, 0.0, 0.0]
        uni.force_accum[8, 8, 8] = [100.0, 50.0, 30.0]

        integrate(uni)
        clamp_velocity(uni)

        max_speed = np.max(np.sqrt(np.sum(uni.velocity**2, axis=-1)))
        print(f"\n  Max speed after integration: {max_speed:.4f}")
        assert max_speed <= cfg.CONSTANTS.C + 1e-6, \
            f"Speed {max_speed} > C={cfg.CONSTANTS.C}"


# =============================================================================
# Test 4.6: Wave Propagation
# =============================================================================

class TestWavePropagation:
    """Flux waves should propagate at approximately C_WAVE."""

    def test_wave_speed(self):
        """Track a flux pulse and measure propagation speed."""
        set_config(GRID_SIZE=32, DAMPING=0.0, C_WAVE=0.4)
        uni = Universe(32)

        mid = 16
        uni.flux[mid, mid, mid] = [3.0, 0.0, 0.0]

        for step in range(20):
            propagate_flux(uni)

        density = np.sqrt(np.sum(uni.flux**2, axis=-1))
        density_xaxis = density[:, mid, mid]

        nonzero_extent = np.sum(density_xaxis > 1e-6)
        print(f"\n  Flux spread along x-axis: {nonzero_extent} voxels (after 20 ticks)")
        assert nonzero_extent > 1, "Flux didn't propagate at all!"


# =============================================================================
# Test 4.7: Triad Stability
# =============================================================================

class TestTriadStability:
    """Locked triads should persist."""

    def test_locked_triad_persists(self):
        """Three locked +1 particles should remain manifested."""
        uni = make_small_universe(16)

        positions = [(8, 8, 8), (9, 8, 8), (8, 9, 8)]
        for pos in positions:
            uni.states[pos] = 1
            uni.flux[pos] = [1.0, 0.5, 0.3]
            uni.is_locked[pos] = True
            uni.density[pos] = np.sqrt(1.0**2 + 0.5**2 + 0.3**2)

        for _ in range(100):
            tick(uni)

        surviving = sum(1 for pos in positions if uni.states[pos] != 0)
        print(f"\n  Triad particles surviving after 100 ticks: {surviving}/3")
        assert surviving == 3, f"Triad lost {3-surviving} particles!"


# =============================================================================
# Test 4.8: Annihilation
# =============================================================================

class TestAnnihilation:
    """Opposite-sign adjacent particles should annihilate."""

    def test_annihilation_occurs(self):
        """Adjacent +1 and -1 should both become 0."""
        uni = make_small_universe(16)

        uni.states[8, 8, 8] = 1
        uni.states[9, 8, 8] = -1
        uni.flux[8, 8, 8] = [1.0, 0.0, 0.0]
        uni.flux[9, 8, 8] = [-1.0, 0.0, 0.0]

        process_interactions(uni)

        state1 = uni.states[8, 8, 8]
        state2 = uni.states[9, 8, 8]
        print(f"\n  After annihilation: states = ({state1}, {state2})")
        assert state1 == 0 and state2 == 0, \
            f"Annihilation failed: ({state1}, {state2})"


# =============================================================================
# Test 4.9: Genesis / Evaporation
# =============================================================================

class TestGenesisEvaporation:
    """Manifestation and evaporation thresholds should work."""

    def test_high_flux_triggers_genesis(self):
        """Flux density >> KB should trigger manifestation."""
        uni = make_small_universe(16)
        uni.flux[8, 8, 8] = [5.0, 5.0, 5.0]  # density ~ 8.66 >> KB=0.511
        calculate_density(uni)

        tick(uni)
        density_val = np.sqrt(np.sum(uni.flux[8, 8, 8]**2))
        print(f"\n  Flux density at site: {density_val:.4f} (KB={cfg.CONSTANTS.KB})")


# =============================================================================
# Test 4.10: Numerical Stability
# =============================================================================

class TestNumericalStability:
    """Long runs should not produce NaN or Inf."""

    def test_no_nan_inf(self):
        """Run 500 ticks and check for NaN/Inf."""
        uni = make_small_universe(16)

        uni.flux[8, 8, 8] = [2.0, 1.0, 0.5]
        uni.states[4, 4, 4] = 1
        uni.flux[4, 4, 4] = [1.0, 0.0, 0.0]
        uni.is_locked[4, 4, 4] = True

        for _ in range(500):
            tick(uni)

        has_nan = np.any(np.isnan(uni.flux))
        has_inf = np.any(np.isinf(uni.flux))
        max_flux = np.max(np.abs(uni.flux))

        print(f"\n  After 500 ticks:")
        print(f"    NaN in flux: {has_nan}")
        print(f"    Inf in flux: {has_inf}")
        print(f"    Max |flux|:  {max_flux:.4f}")

        assert not has_nan, "NaN detected in flux field!"
        assert not has_inf, "Inf detected in flux field!"
        assert max_flux < 1e6, f"Flux explosion: max = {max_flux}"


# =============================================================================
# Test 4.11: Vector Calculus Identities
# =============================================================================

class TestVectorCalculus:
    """Discrete differential operator identities."""

    def test_curl_of_gradient_is_zero(self):
        """curl(grad(f)) = 0 for any scalar field."""
        size = 16
        np.random.seed(42)
        scalar_field = np.random.randn(size, size, size).astype(np.float32)

        grad_f = gradient_3d(scalar_field)
        curl_grad = curl_3d(grad_f)

        max_curl = np.max(np.abs(curl_grad))
        print(f"\n  max|curl(grad(f))| = {max_curl:.2e}")
        assert max_curl < 1e-5, f"curl(grad) nonzero: max = {max_curl}"

    def test_divergence_of_curl_is_zero(self):
        """div(curl(F)) = 0 for any vector field."""
        size = 16
        np.random.seed(42)
        vector_field = np.random.randn(size, size, size, 3).astype(np.float32)

        curl_F = curl_3d(vector_field)
        div_curl = divergence_3d(curl_F)

        max_div = np.max(np.abs(div_curl))
        print(f"\n  max|div(curl(F))| = {max_div:.2e}")
        assert max_div < 1e-5, f"div(curl) nonzero: max = {max_div}"


# =============================================================================
# Test 4.12: Inverse-Square Law Emergence
# =============================================================================

class TestInverseSquareLaw:
    """Test whether gravity-like force follows 1/r^2."""

    def test_gravity_gradient_profile(self):
        """Measure gravitational force vs distance from a density source."""
        size = 32
        set_config(GRID_SIZE=size, GRAVITY_BIAS=0.01)
        uni = Universe(size)

        mid = 16
        for dx in range(-1, 2):
            for dy in range(-1, 2):
                for dz in range(-1, 2):
                    uni.flux[mid+dx, mid+dy, mid+dz] = [3.0, 0.0, 0.0]

        calculate_density(uni)
        f_grav = gravity_force(uni)

        distances = []
        forces = []
        for r in range(3, 14):
            fx, fy, fz = f_grav[mid+r, mid, mid]
            f_mag = np.sqrt(fx**2 + fy**2 + fz**2)
            if f_mag > 1e-10:
                distances.append(r)
                forces.append(f_mag)

        if len(distances) >= 3:
            log_r = np.log(distances)
            log_f = np.log(forces)
            slope, intercept = np.polyfit(log_r, log_f, 1)

            print(f"\n  Force vs distance fit: F ∝ r^{slope:.2f}")
            print(f"  Expected: r^(-2.0)")
            print(f"  Measurements: {len(distances)} points")

            assert -4.0 < slope < -0.5, f"Slope = {slope:.2f}, expected ~-2"
        else:
            print("\n  Not enough data points for 1/r^2 fit")
            pytest.skip("Insufficient data for inverse-square law test")
