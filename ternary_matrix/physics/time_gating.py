"""
FTD Time Gating
Phase 1: Relativistic lag proxy via phase accumulation.

From CLAUDE.md §5.1:
- Check phase accumulators (relativistic lag proxy)
- Mark active voxels
"""
import numpy as np
from ..config import CONSTANTS


def time_gate(universe):
    """
    Phase 1: Time Gating

    Voxels with higher velocity accumulate phase slower,
    simulating time dilation (special relativistic effect).

    phase_rate = 1 / gamma = sqrt(1 - v²/c²)

    Voxels are marked "active" when their phase accumulator
    crosses an integer threshold. This means fast-moving
    particles update less frequently than stationary ones.
    """
    # Calculate v² for each voxel
    v_squared = np.sum(universe.velocity ** 2, axis=-1)
    c_squared = CONSTANTS.C ** 2

    # Compute 1 - v²/c², clamped to avoid negative values near c
    # This gives us gamma_inv_squared = 1/gamma²
    gamma_inv_squared = np.maximum(1.0 - v_squared / c_squared, CONSTANTS.MIN_GAMMA ** 2)

    # Lorentz factor inverse: 1/gamma = sqrt(1 - v²/c²)
    # This is the "proper time" rate relative to coordinate time
    gamma_inv = np.sqrt(gamma_inv_squared)

    # Accumulate phase (slower for faster particles)
    # Stationary particles: gamma_inv = 1, accumulate 1.0 per tick
    # Fast particles: gamma_inv < 1, accumulate less per tick
    universe.phase_accum += gamma_inv

    # Mark active: phase crossed integer threshold (>= 1.0)
    universe.is_active = universe.phase_accum >= 1.0

    # Consume one unit of phase for active voxels
    # This keeps the accumulator bounded while allowing fractional accumulation
    universe.phase_accum[universe.is_active] -= 1.0

    # Return count of active voxels for diagnostics
    return np.count_nonzero(universe.is_active)


def get_effective_time_rate(universe):
    """
    Diagnostic: Return the effective time rate (1/gamma) field.
    Useful for visualization of time dilation effects.
    """
    v_squared = np.sum(universe.velocity ** 2, axis=-1)
    c_squared = CONSTANTS.C ** 2
    gamma_inv_squared = np.maximum(1.0 - v_squared / c_squared, CONSTANTS.MIN_GAMMA ** 2)
    return np.sqrt(gamma_inv_squared)
