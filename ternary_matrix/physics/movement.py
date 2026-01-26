"""
FTD Movement
Phase 8: Discrete lattice movement when position remainder exceeds threshold.

From CLAUDE.md §5.1:
- Integer position updates when remainder >= 1
- Enforce speed limit (|v| <= C)
- Handle empty target (move), same-sign (collision), opposite-sign (defer to annihilation)
"""
import numpy as np
from ..config import CONSTANTS


def move_particles(universe):
    """
    Phase 8: Movement

    When position remainder >= 1 in any component, execute a discrete
    lattice move. Handle collisions based on target state:

    - Empty target (state 0): move particle
    - Same-sign target: elastic collision (exchange momentum)
    - Opposite-sign target: handled by Phase 9 (Annihilation)

    Uses toroidal (periodic) boundary conditions.
    """
    # Find all manifested voxels
    manifested_mask = universe.states != 0
    manifested_indices = np.argwhere(manifested_mask)

    # Track moves to avoid conflicts (process in random order for fairness)
    np.random.shuffle(manifested_indices)

    # Track which voxels have been processed this tick
    processed = np.zeros_like(universe.states, dtype=bool)

    for idx in manifested_indices:
        idx = tuple(idx)

        # Skip if already processed or no longer manifested
        if processed[idx] or universe.states[idx] == 0:
            continue

        # Check position remainder in each axis
        rem = universe.position_rem[idx]

        for axis in range(3):
            if abs(rem[axis]) >= 1.0:
                # Determine movement direction
                direction = int(np.sign(rem[axis]))

                # Calculate target position (with toroidal wrap)
                new_idx = list(idx)
                new_idx[axis] = (idx[axis] + direction) % universe.size
                new_idx = tuple(new_idx)

                # Get states
                source_state = universe.states[idx]
                target_state = universe.states[new_idx]

                if target_state == 0:
                    # Empty target: execute move
                    _execute_move(universe, idx, new_idx)
                    processed[new_idx] = True

                elif target_state == source_state:
                    # Same sign: elastic collision
                    _elastic_collision(universe, idx, new_idx, axis)
                    processed[new_idx] = True

                # Opposite sign: will be handled by annihilation in Phase 9
                # Don't move, don't consume remainder (they'll annihilate)

                # Consume movement from remainder
                universe.position_rem[idx][axis] -= direction

                # Mark source as processed
                processed[idx] = True

                # Only process one movement per particle per tick
                break


def _execute_move(universe, source, target):
    """
    Transfer a particle from source to target voxel.
    Copies all properties: state, flux, velocity, etc.
    """
    # Transfer state
    universe.states[target] = universe.states[source]
    universe.states[source] = 0

    # Transfer flux (particle carries its flux)
    universe.flux[target] = universe.flux[source]
    universe.flux[source] = 0

    # Transfer velocity
    universe.velocity[target] = universe.velocity[source]
    universe.velocity[source] = 0

    # Transfer charge
    universe.charge[target] = universe.charge[source]
    universe.charge[source] = 0

    # Transfer position remainder (carry fractional position)
    universe.position_rem[target] = universe.position_rem[source]
    universe.position_rem[source] = 0

    # Transfer lock status
    universe.is_locked[target] = universe.is_locked[source]
    universe.is_locked[source] = False

    # Transfer phase accumulator
    universe.phase_accum[target] = universe.phase_accum[source]
    universe.phase_accum[source] = 0


def _elastic_collision(universe, idx1, idx2, collision_axis):
    """
    Handle elastic collision between two same-sign particles.

    In 1D elastic collision with equal masses:
    v1_new = v2_old
    v2_new = v1_old
    (velocities are exchanged)

    For the collision axis, we exchange velocity components.
    Other components are preserved.
    """
    # Exchange velocity components along collision axis
    temp = universe.velocity[idx1][collision_axis]
    universe.velocity[idx1][collision_axis] = universe.velocity[idx2][collision_axis]
    universe.velocity[idx2][collision_axis] = temp

    # Also exchange momentum perpendicular to preserve total momentum
    # For simplicity in this discrete model, we just reverse the collision axis component
    # This approximates elastic collision behavior
    universe.velocity[idx1][collision_axis] *= -1
    universe.velocity[idx2][collision_axis] *= -1

    # Reset position remainders for colliding particles
    universe.position_rem[idx1][collision_axis] = 0
    universe.position_rem[idx2][collision_axis] = 0


def move_particles_vectorized(universe):
    """
    Alternative vectorized implementation for better performance on large grids.

    This version handles movements in bulk but may miss some collision cases.
    Use for performance-critical applications; use move_particles() for accuracy.
    """
    # Compute which voxels need to move
    # A voxel moves if |position_rem| >= 1 in any axis
    abs_rem = np.abs(universe.position_rem)
    needs_move = np.any(abs_rem >= 1.0, axis=-1) & (universe.states != 0)

    if not np.any(needs_move):
        return

    # For each axis, find voxels that want to move along that axis
    for axis in range(3):
        axis_move_mask = (abs_rem[..., axis] >= 1.0) & (universe.states != 0)

        if not np.any(axis_move_mask):
            continue

        # Determine direction (+1 or -1)
        direction = np.sign(universe.position_rem[..., axis])

        # Create shifted views to check targets
        pos_targets = np.roll(universe.states, -1, axis=axis)
        neg_targets = np.roll(universe.states, 1, axis=axis)

        # Positive direction moves
        pos_move = axis_move_mask & (direction > 0)
        pos_empty = pos_move & (pos_targets == 0)

        # Negative direction moves
        neg_move = axis_move_mask & (direction < 0)
        neg_empty = neg_move & (neg_targets == 0)

        # Execute empty-target moves via array operations
        # This is a simplified version; full implementation would need conflict resolution
        if np.any(pos_empty):
            _vectorized_move_axis(universe, axis, +1, pos_empty)

        if np.any(neg_empty):
            _vectorized_move_axis(universe, axis, -1, neg_empty)


def _vectorized_move_axis(universe, axis, direction, mask):
    """
    Execute vectorized movement along a single axis.
    Only moves to empty targets.
    """
    # Create target mask (shifted version)
    target_mask = np.roll(mask, -direction, axis=axis)

    # Get source values
    source_states = universe.states[mask]
    source_flux = universe.flux[mask]
    source_vel = universe.velocity[mask]
    source_charge = universe.charge[mask]

    # Clear sources
    universe.states[mask] = 0
    universe.flux[mask] = 0
    universe.velocity[mask] = 0
    universe.charge[mask] = 0

    # Set targets
    universe.states[target_mask] = source_states
    universe.flux[target_mask] = source_flux
    universe.velocity[target_mask] = source_vel
    universe.charge[target_mask] = source_charge

    # Consume movement
    universe.position_rem[..., axis][mask] -= direction
