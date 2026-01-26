"""
FTD Interactions
Phase 9: Collisions and Annihilation.

From CLAUDE.md §4.4 and §5.1:
- Annihilation: +1 and -1 adjacent → both become 0
- Combined flux redistributed to neighbors as omnidirectional burst
- Total flux magnitude conserved

Topology Note:
- Annihilation uses VON NEUMANN (6 neighbors) because physical contact
  requires direct face-sharing adjacency, not diagonal proximity
- This differs from binding (Phase 11) which uses MOORE (26 neighbors)
"""
import numpy as np
from ..config import VON_NEUMANN_SHIFTS


def process_interactions(universe):
    """
    Phase 9: Collisions and Annihilation

    Handle particle-particle interactions:
    1. Annihilation: +1 adjacent to -1 → both become 0
    2. Release energy as flux burst to neighbors

    Uses 6-connected Von Neumann neighborhood for adjacency detection.
    """
    states = universe.states

    # Create masks for +1 and -1
    pos_matter = (states == 1)
    neg_matter = (states == -1)

    # Detect adjacency
    has_neg_neighbor = np.zeros_like(states, dtype=bool)
    has_pos_neighbor = np.zeros_like(states, dtype=bool)

    for sx, sy, sz in VON_NEUMANN_SHIFTS:
        # Check if neighbor is -1
        shifted_neg = np.roll(neg_matter, (sx, sy, sz), axis=(0, 1, 2))
        has_neg_neighbor |= shifted_neg

        # Check if neighbor is +1
        shifted_pos = np.roll(pos_matter, (sx, sy, sz), axis=(0, 1, 2))
        has_pos_neighbor |= shifted_pos

    # Annihilation conditions
    kill_pos = pos_matter & has_neg_neighbor
    kill_neg = neg_matter & has_pos_neighbor

    # Calculate energy to release (sum of flux magnitudes)
    annihilating = kill_pos | kill_neg
    if np.any(annihilating):
        # Release flux as burst to neighbors
        release_annihilation_energy(universe, annihilating)

    # Apply state transitions
    universe.states[kill_pos] = 0
    universe.states[kill_neg] = 0

    # Clear other properties for annihilated voxels
    universe.velocity[annihilating] = 0
    universe.charge[annihilating] = 0
    universe.position_rem[annihilating] = 0
    universe.phase_accum[annihilating] = 0

    return np.count_nonzero(annihilating)


def release_annihilation_energy(universe, annihilating_mask):
    """
    Release flux from annihilating particles to their neighbors.

    From CLAUDE.md §4.4:
    - Combined flux redistributed to neighbors as omnidirectional burst
    - Total flux magnitude conserved
    """
    # Get total flux to release
    flux_to_release = universe.flux[annihilating_mask].copy()
    total_energy = np.sqrt(np.sum(flux_to_release ** 2, axis=-1))

    # Clear flux at annihilation sites
    universe.flux[annihilating_mask] = 0

    # Find annihilation sites
    annihilating_indices = np.argwhere(annihilating_mask)

    for i, idx in enumerate(annihilating_indices):
        idx = tuple(idx)

        # Distribute energy to 6 neighbors equally
        # Each neighbor gets 1/6 of the released flux
        energy = total_energy[i]
        burst_magnitude = energy / 6.0

        for sx, sy, sz in VON_NEUMANN_SHIFTS:
            neighbor = (
                (idx[0] + sx) % universe.size,
                (idx[1] + sy) % universe.size,
                (idx[2] + sz) % universe.size
            )

            # Add radial flux pointing away from annihilation site
            direction = np.array([sx, sy, sz], dtype=np.float32)
            direction_norm = np.sqrt(np.sum(direction ** 2))
            if direction_norm > 0:
                direction /= direction_norm

            universe.flux[neighbor] += burst_magnitude * direction


def get_annihilation_count(universe):
    """
    Diagnostic: Count how many particles are about to annihilate.
    Does not modify state.
    """
    states = universe.states
    pos_matter = (states == 1)
    neg_matter = (states == -1)

    has_neg_neighbor = np.zeros_like(states, dtype=bool)
    has_pos_neighbor = np.zeros_like(states, dtype=bool)

    for sx, sy, sz in VON_NEUMANN_SHIFTS:
        shifted_neg = np.roll(neg_matter, (sx, sy, sz), axis=(0, 1, 2))
        has_neg_neighbor |= shifted_neg

        shifted_pos = np.roll(pos_matter, (sx, sy, sz), axis=(0, 1, 2))
        has_pos_neighbor |= shifted_pos

    kill_pos = pos_matter & has_neg_neighbor
    kill_neg = neg_matter & has_pos_neighbor

    return np.count_nonzero(kill_pos | kill_neg)


def get_collision_pairs(universe):
    """
    Diagnostic: Return list of (pos_idx, neg_idx) pairs that will annihilate.
    """
    pairs = []

    states = universe.states
    pos_indices = np.argwhere(states == 1)
    neg_indices = np.argwhere(states == -1)

    # For each positive particle, check if any negative is adjacent
    for pos_idx in pos_indices:
        pos_idx = tuple(pos_idx)
        for sx, sy, sz in VON_NEUMANN_SHIFTS:
            neighbor = (
                (pos_idx[0] + sx) % universe.size,
                (pos_idx[1] + sy) % universe.size,
                (pos_idx[2] + sz) % universe.size
            )
            if states[neighbor] == -1:
                pairs.append((pos_idx, neighbor))
                break  # Only record one pair per positive

    return pairs
