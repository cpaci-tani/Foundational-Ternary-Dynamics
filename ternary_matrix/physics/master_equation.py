"""
FTD Master Equation
Implements the complete 12-Phase Update Cycle defined in CLAUDE.md Chapter 5.

Phase 2.4 Update: All 12 phases now implemented.
"""
import numpy as np
from ..config import CONSTANTS
from . import waves, forces, interactions, binding
from . import time_gating, integration, movement, transmutation


def apply_decay(universe):
    """
    Phase 2: Entropy
    Apply decay to unlocked manifested voxels.

    From CLAUDE.md §4.3:
    if not is_locked(v): flux(v) *= (1 - γ)
    """
    decay_factor = 1.0 - CONSTANTS.DECAY_RATE

    # Only apply decay to UNLOCKED voxels
    unlocked_mask = ~universe.is_locked

    # Create a factor array where unlocked = decay, locked = 1.0
    factor = np.ones_like(universe.density)  # (N,N,N)
    factor[unlocked_mask] = decay_factor

    # Reshape to (N,N,N,1) for broadcasting against (N,N,N,3)
    factor = factor[..., np.newaxis]

    universe.flux *= factor


def update_manifestation(universe):
    """
    Phase 3: Existence Transitions
    Handles Genesis (0 -> +/-1) and Evaporation (+/-1 -> 0).

    From CLAUDE.md §4.1 and §4.2.
    """
    # 1. Evaporation: State -> 0 if Density < KB
    evaporation_mask = (universe.states != 0) & (universe.density < CONSTANTS.KB)
    universe.states[evaporation_mask] = 0
    # Also clear velocity and other properties for evaporated voxels
    universe.velocity[evaporation_mask] = 0
    universe.charge[evaporation_mask] = 0

    # 2. Genesis: 0 -> State if Density > KB
    candidate_mask = (universe.states == 0) & (universe.density > CONSTANTS.KB)

    if np.any(candidate_mask):
        divergence = forces.calculate_divergence(universe)
        pos_mask = candidate_mask & (divergence > 0)
        neg_mask = candidate_mask & (divergence < 0)

        # Genesis Probability (Ch 4.1)
        # p_manifest = clamp(1 - exp(-(density - KB) / KB), 0, 1)
        rho = universe.density
        p_manifest = 1.0 - np.exp(-(rho - CONSTANTS.KB) / CONSTANTS.KB)
        p_manifest = np.clip(p_manifest, 0, 1)

        roll = np.random.random(universe.states.shape)
        success_mask = roll < p_manifest

        # Apply genesis
        universe.states[pos_mask & success_mask] = 1
        universe.states[neg_mask & success_mask] = -1

        # Initialize charge based on state
        universe.charge[pos_mask & success_mask] = 1.0
        universe.charge[neg_mask & success_mask] = -1.0


def tick(universe):
    """
    Advance the universe by one discrete time step (dt=1).

    Implements the complete 12-phase update cycle from CLAUDE.md §5.1:

    PHASE 1:  Time Gating - relativistic lag proxy
    PHASE 2:  Entropy - decay for unlocked voxels
    PHASE 3:  Existence Transitions - genesis and evaporation
    PHASE 4:  Wave Propagation - flux wave equation
    PHASE 5:  Field Computation - density calculation
    PHASE 6:  Force Accumulation - all 5 force types
    PHASE 7:  Integration - velocity and position updates
    PHASE 8:  Movement - discrete lattice moves
    PHASE 9:  Collisions - annihilation and elastic collisions
    PHASE 10: Transmutation - weak-force polarity flips
    PHASE 11: Binding - triad detection and locking
    PHASE 12: Increment - advance tick counter
    """

    # PHASE 1: Time Gating
    # Mark which voxels are active this tick (relativistic time dilation)
    time_gating.time_gate(universe)

    # PHASE 2: Entropy
    # Apply decay to unlocked manifested voxels
    apply_decay(universe)

    # PHASE 3: Existence Transitions
    # Genesis (0 -> ±1) and Evaporation (±1 -> 0)
    update_manifestation(universe)

    # PHASE 4: Wave Propagation
    # Update flux field according to wave equation
    waves.propagate_flux(universe)

    # PHASE 5: Field Computation
    # Calculate derived fields (density, etc.)
    forces.calculate_density(universe)

    # PHASE 6: Force Accumulation
    # Compute all forces: gravity, Coulomb, Lorentz, strong, weak stress
    forces.accumulate_forces(universe)

    # PHASE 7: Integration
    # Update velocities from forces, accumulate position remainders
    integration.integrate(universe)

    # PHASE 8: Movement
    # Execute discrete lattice moves when remainder >= 1
    movement.move_particles(universe)

    # PHASE 9: Collisions
    # Handle annihilation (+1 meets -1) and elastic collisions
    interactions.process_interactions(universe)

    # PHASE 10: Transmutation
    # Weak-force polarity flips under high stress
    transmutation.transmute(universe)

    # PHASE 11: Binding
    # Detect stable structures (triads) and set lock flags
    binding.update_bindings(universe)

    # PHASE 12: Increment
    # Advance global tick counter
    universe.tick += 1

    return universe.tick


def tick_minimal(universe):
    """
    Minimal tick function for performance testing.
    Only runs essential phases (2, 3, 4, 5, 9, 11).
    """
    apply_decay(universe)
    update_manifestation(universe)
    waves.propagate_flux(universe)
    forces.calculate_density(universe)
    interactions.process_interactions(universe)
    binding.update_bindings(universe)
    universe.tick += 1
    return universe.tick


def run_simulation(universe, num_ticks, callback=None, callback_interval=100):
    """
    Run the simulation for a specified number of ticks.

    Args:
        universe: The Universe instance to evolve
        num_ticks: Number of ticks to run
        callback: Optional function called periodically with (universe, tick)
        callback_interval: How often to call the callback

    Returns:
        Final tick count
    """
    for i in range(num_ticks):
        tick(universe)

        if callback and (universe.tick % callback_interval == 0):
            callback(universe, universe.tick)

    return universe.tick


def get_diagnostics(universe):
    """
    Return a dictionary of diagnostic values for the current state.
    """
    return {
        'tick': universe.tick,
        'manifested_count': universe.get_manifested_count(),
        'positive_count': universe.get_positive_count(),
        'negative_count': universe.get_negative_count(),
        'total_flux': universe.get_total_flux_magnitude(),
        'total_charge': universe.get_total_charge(),
        'avg_speed': integration.get_average_speed(universe),
        'max_speed': integration.get_max_speed(universe),
        'kinetic_energy': integration.get_kinetic_energy(universe),
    }
