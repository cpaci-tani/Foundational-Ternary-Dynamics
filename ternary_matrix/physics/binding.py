"""
FTD Binding Logic
Phase 11: Structure Detection (Triads).

From CLAUDE.md §8.1:
- Triads are 3 same-sign particles forming a stable configuration
- Particles are locked when they have >= 2 neighbors of same sign
- Uses 26-connected Moore Neighborhood for neighbor detection

Topology Note:
- Binding uses MOORE (26 neighbors) because triads can form in any
  geometric configuration within the 3x3x3 cube
- This differs from annihilation (Phase 9) which uses VON NEUMANN (6)
  because contact requires direct face-sharing adjacency
"""
import numpy as np
from ..config import MOORE_SHIFTS


def update_bindings(universe):
    """
    Phase 11: Binding

    Detect stable structures (triads) and set lock flags.
    A particle is locked if it has >= 2 neighbors of the same sign
    within its 26-connected Moore neighborhood.

    Locked particles:
    - Do not decay (Phase 2 skips them)
    - Form stable bound structures
    - Analog of nucleon binding
    """
    states = universe.states

    # Reset locks
    universe.is_locked.fill(False)

    # Create masks for positive and negative matter
    pos_matter = (states == 1)
    neg_matter = (states == -1)

    # Count same-sign neighbors for each voxel
    n_pos = count_neighbors_moore(pos_matter)
    n_neg = count_neighbors_moore(neg_matter)

    # Binding rule: >= 2 neighbors of same type
    # This creates triads (3 mutually adjacent particles)
    pos_lock = pos_matter & (n_pos >= 2)
    neg_lock = neg_matter & (n_neg >= 2)

    universe.is_locked[pos_lock] = True
    universe.is_locked[neg_lock] = True


def count_neighbors_moore(mask):
    """
    Count neighbors in 26-connected Moore neighborhood.

    Args:
        mask: Boolean array (Nx, Ny, Nz) indicating presence

    Returns:
        Count array (Nx, Ny, Nz) with neighbor count 0-26
    """
    count = np.zeros(mask.shape, dtype=np.int8)

    for sx, sy, sz in MOORE_SHIFTS:
        # Roll and add
        count += np.roll(mask, (sx, sy, sz), axis=(0, 1, 2)).astype(np.int8)

    return count


def get_triad_count(universe):
    """
    Diagnostic: Count the number of triads (locked triplets).

    A proper triad is 3 same-sign particles where all 3 are locked.
    We count this by finding connected components of locked particles.
    """
    # Simple approximation: count locked particles / 3
    locked_count = np.count_nonzero(universe.is_locked)
    return locked_count // 3


def get_binding_energy(universe):
    """
    Diagnostic: Estimate total binding energy.

    From CLAUDE.md §8.1: binding_energy ≈ KB × PHI per triad
    """
    from ..config import CONSTANTS
    PHI = 1.618033988749895  # Golden ratio

    triad_count = get_triad_count(universe)
    return triad_count * CONSTANTS.KB * PHI


def detect_triads(universe):
    """
    Detect and return positions of triads.

    Returns:
        List of (centroid_position, sign, member_positions) tuples
    """
    triads = []

    # Find locked voxels
    locked = universe.is_locked
    states = universe.states

    # Use flood-fill to find connected components of locked voxels
    visited = np.zeros_like(locked)

    locked_indices = np.argwhere(locked)

    for idx in locked_indices:
        idx = tuple(idx)
        if visited[idx]:
            continue

        # Flood fill from this point
        sign = states[idx]
        members = []
        stack = [idx]

        while stack:
            current = stack.pop()
            if visited[current]:
                continue
            if not locked[current]:
                continue
            if states[current] != sign:
                continue

            visited[current] = True
            members.append(current)

            # Add Moore neighbors to stack
            for sx, sy, sz in MOORE_SHIFTS:
                neighbor = (
                    (current[0] + sx) % universe.size,
                    (current[1] + sy) % universe.size,
                    (current[2] + sz) % universe.size
                )
                if not visited[neighbor]:
                    stack.append(neighbor)

        # If we found a component, record it
        if len(members) >= 3:
            centroid = np.mean(members, axis=0)
            triads.append((centroid, sign, members))

    return triads
