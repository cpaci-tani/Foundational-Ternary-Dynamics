"""
FTD Integration
Phase 7: Velocity and position integration from accumulated forces.

From CLAUDE.md §5.1:
- Update velocities from forces
- Accumulate position remainders
"""
import numpy as np
from ..config import CONSTANTS


def integrate(universe, dt=1.0):
    """
    Phase 7: Integration

    Update velocities from accumulated forces, then accumulate
    position remainders. Only process active voxels.

    Velocity update: v += F × dt (assuming unit mass)
    Position remainder: r += v × dt

    The position remainder represents sub-lattice position.
    When it exceeds 1.0 in any component, Phase 8 (Movement)
    will execute a discrete lattice move.
    """
    # Only update active, manifested voxels
    active_manifested = universe.is_active & (universe.states != 0)
    mask_3d = active_manifested[..., np.newaxis]

    # Update velocity: v += F × dt (assuming unit mass)
    # Only for active manifested voxels
    universe.velocity = np.where(
        mask_3d,
        universe.velocity + universe.force_accum * dt,
        universe.velocity
    )

    # Enforce speed limit: |v| <= c
    clamp_velocity(universe)

    # Accumulate position remainder: r += v × dt
    universe.position_rem = np.where(
        mask_3d,
        universe.position_rem + universe.velocity * dt,
        universe.position_rem
    )

    # Clear force accumulator for next tick
    universe.force_accum.fill(0)


def clamp_velocity(universe, eps=1e-10):
    """
    Enforce the speed of causality limit.
    No particle can exceed speed c.

    |v| > c  =>  v = c × v̂
    """
    speed = np.sqrt(np.sum(universe.velocity ** 2, axis=-1, keepdims=True))
    c = CONSTANTS.C

    # Find voxels exceeding speed limit
    too_fast = speed > c

    # Scale down velocities that exceed c
    # v_new = c × (v / |v|) = v × (c / |v|)
    scale = np.where(too_fast, c / np.maximum(speed, eps), 1.0)
    universe.velocity *= scale


def get_kinetic_energy(universe):
    """
    Diagnostic: Compute total kinetic energy of manifested particles.
    KE = 0.5 × m × v² (assuming unit mass)
    """
    manifested = universe.states != 0
    v_squared = np.sum(universe.velocity ** 2, axis=-1)
    return 0.5 * np.sum(v_squared[manifested])


def get_average_speed(universe):
    """
    Diagnostic: Compute average speed of manifested particles.
    """
    manifested = universe.states != 0
    if not np.any(manifested):
        return 0.0

    speed = np.sqrt(np.sum(universe.velocity ** 2, axis=-1))
    return np.mean(speed[manifested])


def get_max_speed(universe):
    """
    Diagnostic: Compute maximum speed in the system.
    Should never exceed c after clamping.
    """
    speed = np.sqrt(np.sum(universe.velocity ** 2, axis=-1))
    return np.max(speed)
