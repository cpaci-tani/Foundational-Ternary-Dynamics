"""
FTD Transmutation
Phase 10: Weak-force polarity flips under high field stress.

From CLAUDE.md §6.5:
- stress(v) = |∇·J| + |∇×J| + |∇ρ|
- if stress(v) > WEAK_THRESHOLD: polarity may flip (+1 ↔ -1)
"""
import numpy as np
from ..config import CONSTANTS
from . import forces


def transmute(universe):
    """
    Phase 10: Transmutation

    High field stress enables polarity flip (+1 ↔ -1).
    This is the weak-force analog in FTD.

    The probability of transmutation scales with excess stress
    above the threshold, capped at 50% per tick to prevent
    rapid oscillation.
    """
    # Calculate stress field
    stress = forces.weak_stress(universe)

    # Find manifested voxels exceeding threshold
    manifested = universe.states != 0
    above_threshold = stress > CONSTANTS.WEAK_THRESHOLD
    candidates = manifested & above_threshold

    if not np.any(candidates):
        return 0  # No transmutations

    # Calculate flip probability based on excess stress
    # p = (stress - threshold) / threshold, clamped to [0, 0.5]
    excess = stress[candidates] - CONSTANTS.WEAK_THRESHOLD
    flip_prob = np.clip(excess / CONSTANTS.WEAK_THRESHOLD, 0, 0.5)

    # Stochastic flip decision
    random_values = np.random.random(np.sum(candidates))
    do_flip = random_values < flip_prob

    # Apply flips
    # Get indices of candidates
    candidate_indices = np.argwhere(candidates)

    flip_count = 0
    for i, idx in enumerate(candidate_indices):
        if do_flip[i]:
            idx = tuple(idx)
            universe.states[idx] *= -1  # +1 → -1 or -1 → +1
            universe.charge[idx] *= -1  # Flip charge too
            flip_count += 1

    return flip_count


def transmute_vectorized(universe):
    """
    Vectorized version of transmutation for better performance.
    Uses boolean indexing to avoid explicit loops.
    """
    # Calculate stress field
    stress = forces.weak_stress(universe)

    # Find manifested voxels exceeding threshold
    manifested = universe.states != 0
    above_threshold = stress > CONSTANTS.WEAK_THRESHOLD
    candidates = manifested & above_threshold

    if not np.any(candidates):
        return 0

    # Calculate flip probability
    flip_prob = np.zeros_like(stress)
    flip_prob[candidates] = np.clip(
        (stress[candidates] - CONSTANTS.WEAK_THRESHOLD) / CONSTANTS.WEAK_THRESHOLD,
        0, 0.5
    )

    # Generate random field and determine flips
    random_field = np.random.random(stress.shape)
    do_flip = candidates & (random_field < flip_prob)

    # Count flips before applying
    flip_count = np.count_nonzero(do_flip)

    # Apply flips
    universe.states[do_flip] *= -1
    universe.charge[do_flip] *= -1

    return flip_count


def get_stress_field(universe):
    """
    Diagnostic: Return the full stress field for visualization.
    """
    return forces.weak_stress(universe)


def get_transmutation_candidates(universe):
    """
    Diagnostic: Return mask of voxels that could potentially transmute.
    """
    stress = forces.weak_stress(universe)
    manifested = universe.states != 0
    above_threshold = stress > CONSTANTS.WEAK_THRESHOLD
    return manifested & above_threshold


def get_transmutation_probability(universe):
    """
    Diagnostic: Return the probability field for transmutation.
    Non-candidate voxels have probability 0.
    """
    stress = forces.weak_stress(universe)
    manifested = universe.states != 0
    above_threshold = stress > CONSTANTS.WEAK_THRESHOLD
    candidates = manifested & above_threshold

    prob = np.zeros_like(stress)
    if np.any(candidates):
        prob[candidates] = np.clip(
            (stress[candidates] - CONSTANTS.WEAK_THRESHOLD) / CONSTANTS.WEAK_THRESHOLD,
            0, 0.5
        )
    return prob
