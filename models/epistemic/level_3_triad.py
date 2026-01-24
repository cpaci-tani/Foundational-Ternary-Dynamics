"""
LEVEL 3: THE TRIAD
==================

The minimal stable multi-particle structure: three voxels.
Here we derive why 3 is the minimum for stability and compute binding energy.

Epistemic Status: DERIVED from Levels 0-2
"""

import numpy as np
from dataclasses import dataclass, field
from typing import Tuple, List, Optional
from enum import Enum
from itertools import combinations

from .level_0_planck import (
    TernaryState, LATTICE, CONSTANTS, SI
)
from .level_1_voxel import THRESHOLD, SingleVoxel
from .level_2_pair import (
    AdjacencyType, classify_adjacency, distance,
    VoxelPair, FLUX
)


# =============================================================================
# SECTION 3.1: WHY THREE IS SPECIAL
# =============================================================================

@dataclass(frozen=True)
class TriadStabilityTheorem:
    """
    Why 3 particles is the minimum for stable binding.

    From CLAUDE.md Chapter 8.1:
        "A particle is locked if it has >= 2 neighbors of the same sign
         within its 26-connected Moore neighborhood."

    Analysis:
        - 1 particle: 0 same-sign neighbors -> NOT locked -> decays
        - 2 particles: Each has 1 same-sign neighbor -> NOT locked -> decay
        - 3 particles (triangle): Each has 2 same-sign neighbors -> LOCKED -> stable!

    The number 3 emerges from the binding rule, not as an input.
    """

    min_neighbors_for_lock: int = 2  # From binding rule

    @property
    def minimum_stable_size(self) -> int:
        """
        The minimum number of particles for a stable structure.

        In a complete graph K_n, each vertex has (n-1) neighbors.
        For stability: n-1 >= min_neighbors_for_lock
        Therefore: n >= min_neighbors_for_lock + 1 = 3
        """
        return self.min_neighbors_for_lock + 1

    @property
    def explanation(self) -> str:
        return f"""
TRIAD STABILITY THEOREM

Given: Binding rule requires >= {self.min_neighbors_for_lock} same-sign neighbors

For n particles in mutual adjacency:
  - Each particle has (n-1) same-sign neighbors
  - Stability requires: n-1 >= {self.min_neighbors_for_lock}
  - Therefore: n >= {self.minimum_stable_size}

The TRIAD (n=3) is the minimum stable configuration.
QED
"""


STABILITY_THEOREM = TriadStabilityTheorem()


# =============================================================================
# SECTION 3.2: TRIAD GEOMETRY
# =============================================================================

class TriadGeometry(Enum):
    """
    Possible geometric arrangements of three adjacent voxels.

    On a cubic lattice, three mutually adjacent voxels can form:
    """
    # All three share a common edge (linear arrangement impossible for mutual adjacency)
    # Actually: for 3 voxels to be mutually adjacent on cubic lattice:

    RIGHT_ANGLE = ("right-angle", [(0,0,0), (1,0,0), (0,1,0)])
    # Forms an L-shape in a plane

    DIAGONAL_PLANE = ("diagonal-plane", [(0,0,0), (1,0,0), (1,1,0)])
    # Another L-shape variant

    THREE_D_CORNER = ("3d-corner", [(0,0,0), (1,0,0), (0,0,1)])
    # L-shape spanning two planes

    @property
    def name(self) -> str:
        return self.value[0]

    @property
    def positions(self) -> List[Tuple[int, int, int]]:
        return self.value[1]


def compute_triad_distances(positions: List[Tuple[int, int, int]]) -> List[float]:
    """Compute all pairwise distances in a triad."""
    distances = []
    for i, j in combinations(range(3), 2):
        d = distance(positions[i], positions[j])
        distances.append(d)
    return sorted(distances)


def is_valid_triad(positions: List[Tuple[int, int, int]]) -> bool:
    """
    Check if three positions form a valid triad (mutually adjacent).

    All three pairs must be within Moore neighborhood (d <= sqrt(3)).
    """
    if len(positions) != 3:
        return False

    for i, j in combinations(range(3), 2):
        adj = classify_adjacency(positions[i], positions[j])
        if adj == AdjacencyType.DISTANT:
            return False

    return True


# =============================================================================
# SECTION 3.3: BINDING ENERGY
# =============================================================================

@dataclass(frozen=True)
class TriadBindingEnergy:
    """
    The binding energy of a triad configuration.

    From CLAUDE.md Chapter 8.1:
        "binding_energy ~ KB * PHI"

    Where PHI = golden ratio = (1 + sqrt(5)) / 2

    The binding energy is the energy required to separate the triad
    into three isolated (and therefore decaying) particles.
    """

    @property
    def phi(self) -> float:
        """The golden ratio."""
        return (1 + np.sqrt(5)) / 2

    @property
    def KB(self) -> float:
        """Manifestation threshold from Level 1."""
        return THRESHOLD.KB_dimensionless

    @property
    def binding_energy_per_triad(self) -> float:
        """
        Total binding energy of one triad.

        E_bind = KB * phi
        """
        return self.KB * self.phi

    @property
    def binding_energy_per_particle(self) -> float:
        """Binding energy per particle in the triad."""
        return self.binding_energy_per_triad / 3

    @property
    def binding_energy_eV(self) -> float:
        """Binding energy in electron-volts."""
        # Convert from Planck units
        E_P_eV = SI.E_P / SI.e
        return self.binding_energy_per_triad * E_P_eV

    def coulomb_potential_energy(self,
                                 positions: List[Tuple[int, int, int]],
                                 charges: List[int]) -> float:
        """
        Compute Coulomb potential energy of the configuration.

        U = sum over pairs of: alpha * q_i * q_j / r_ij
        """
        alpha = CONSTANTS.alpha
        U_total = 0.0

        for (i, j) in combinations(range(3), 2):
            r = distance(positions[i], positions[j])
            if r > 0:
                U_total += alpha * charges[i] * charges[j] / r

        return U_total


BINDING = TriadBindingEnergy()


# =============================================================================
# SECTION 3.4: THE TRIAD MODEL
# =============================================================================

@dataclass
class Triad:
    """
    A complete model of a three-particle bound state.

    This is the "proton/neutron core" of FTD - the minimal stable structure.
    """

    # Positions of the three voxels
    positions: List[Tuple[int, int, int]] = field(
        default_factory=lambda: [(0,0,0), (1,0,0), (0,1,0)]
    )

    # The three voxels (all same sign for binding)
    voxels: List[SingleVoxel] = field(default_factory=list)

    # Common charge sign
    charge_sign: int = TernaryState.POSITIVE

    def __post_init__(self):
        if not self.voxels:
            self.voxels = [
                SingleVoxel(state=self.charge_sign)
                for _ in range(3)
            ]
        self.validate()

    def validate(self):
        """Verify this is a valid triad configuration."""
        if len(self.positions) != 3:
            raise ValueError("Triad must have exactly 3 positions")
        if len(self.voxels) != 3:
            raise ValueError("Triad must have exactly 3 voxels")
        if not is_valid_triad(self.positions):
            raise ValueError("Positions do not form a valid triad (not mutually adjacent)")

        # All must have same sign
        signs = [v.state for v in self.voxels]
        if not all(s == signs[0] for s in signs):
            raise ValueError("All voxels in a triad must have the same sign")
        if signs[0] == TernaryState.VOID:
            raise ValueError("Triad voxels cannot be void")

    @property
    def is_locked(self) -> bool:
        """
        Check if all particles are locked (stable).

        Each particle has exactly 2 same-sign neighbors,
        which meets the binding threshold.
        """
        # In a valid triad, each particle has exactly 2 neighbors
        # Binding rule: locked if >= 2 same-sign neighbors
        return True  # By construction of valid triad

    @property
    def total_charge(self) -> int:
        """Net charge of the triad."""
        return sum(v.state for v in self.voxels)

    @property
    def center_of_mass(self) -> Tuple[float, float, float]:
        """Geometric center of the triad."""
        return tuple(
            sum(p[i] for p in self.positions) / 3
            for i in range(3)
        )

    @property
    def pairwise_distances(self) -> List[float]:
        """All three pairwise distances."""
        return compute_triad_distances(self.positions)

    @property
    def mean_distance(self) -> float:
        """Average pairwise distance."""
        return np.mean(self.pairwise_distances)

    @property
    def coulomb_energy(self) -> float:
        """Coulomb potential energy (repulsive for same-sign)."""
        charges = [v.state for v in self.voxels]
        return BINDING.coulomb_potential_energy(self.positions, charges)

    @property
    def binding_energy(self) -> float:
        """Total binding energy."""
        return BINDING.binding_energy_per_triad

    @property
    def net_energy(self) -> float:
        """
        Net energy = Coulomb repulsion - binding attraction.

        If negative, the triad is bound.
        """
        return self.coulomb_energy - self.binding_energy

    @property
    def is_energetically_bound(self) -> bool:
        """
        Whether binding energy exceeds Coulomb repulsion.

        NOTE: In FTD, stability comes from LOCKING (geometric constraint),
        not energetic binding. Triads are stable because each particle
        has >= 2 same-sign neighbors, preventing decay.

        The energetic analysis here is for comparison with QCD-style
        binding, but the primary stability mechanism is topological.
        """
        return self.net_energy < 0

    @property
    def summary(self) -> str:
        sign = "+" if self.charge_sign > 0 else "-"
        return (f"Triad({sign}{sign}{sign}, "
                f"Q={self.total_charge}, "
                f"r_mean={self.mean_distance:.3f}, "
                f"E_coulomb={self.coulomb_energy:.6e}, "
                f"E_bind={self.binding_energy:.6e}, "
                f"bound={self.is_energetically_bound})")


# =============================================================================
# SECTION 3.5: TRIAD CONFIGURATIONS CATALOG
# =============================================================================

def enumerate_triad_geometries() -> List[dict]:
    """
    Enumerate all distinct triad geometries on the cubic lattice.

    Returns list of configurations with their properties.
    """
    # Generate all possible triads with one vertex at origin
    origin = (0, 0, 0)
    neighbors = LATTICE.neighborhood_offsets

    triads = []
    seen = set()

    for n1 in neighbors:
        pos1 = tuple(origin[i] + n1[i] for i in range(3))
        for n2 in neighbors:
            if n2 <= n1:  # Avoid duplicates
                continue
            pos2 = tuple(origin[i] + n2[i] for i in range(3))

            # Check if all three are mutually adjacent
            positions = [origin, pos1, pos2]
            if not is_valid_triad(positions):
                continue

            # Compute signature (sorted distances) for uniqueness
            distances = tuple(compute_triad_distances(positions))
            if distances in seen:
                continue
            seen.add(distances)

            triads.append({
                'positions': positions,
                'distances': distances,
                'mean_r': np.mean(distances),
            })

    return triads


# =============================================================================
# SECTION 3.6: VERIFICATION
# =============================================================================

def verify_level_3():
    """Verify Level 3 derivations."""
    print("=" * 60)
    print("LEVEL 3: TRIAD VERIFICATION")
    print("=" * 60)

    print("\n--- Stability Theorem ---")
    print(STABILITY_THEOREM.explanation)

    print("--- Golden Ratio in Binding ---")
    print(f"  phi = {BINDING.phi:.6f}")
    print(f"  KB  = {BINDING.KB:.6e}")
    print(f"  E_bind = KB * phi = {BINDING.binding_energy_per_triad:.6e}")
    print(f"  E_bind (eV) = {BINDING.binding_energy_eV:.3e} eV")

    print("\n--- Triad Geometries ---")
    geometries = enumerate_triad_geometries()
    print(f"  Found {len(geometries)} distinct triad geometries:")
    for i, g in enumerate(geometries[:5]):  # Show first 5
        print(f"    {i+1}. distances={[f'{d:.3f}' for d in g['distances']]}, mean_r={g['mean_r']:.3f}")

    print("\n--- Example Triad Analysis ---")
    # Standard right-angle triad
    triad = Triad(
        positions=[(0,0,0), (1,0,0), (0,1,0)],
        charge_sign=TernaryState.POSITIVE
    )
    print(f"  {triad.summary}")
    print(f"  Pairwise distances: {triad.pairwise_distances}")
    print(f"  Center of mass: {triad.center_of_mass}")
    print(f"  Is locked: {triad.is_locked}")

    print("\n--- Energy Balance ---")
    print(f"  Coulomb repulsion: {triad.coulomb_energy:.6e}")
    print(f"  Binding energy:    {triad.binding_energy:.6e}")
    print(f"  Net energy:        {triad.net_energy:.6e}")
    print(f"  Bound state:       {triad.is_energetically_bound}")

    # Compare different geometries
    print("\n--- Geometry Comparison ---")
    for g in geometries[:3]:
        t = Triad(positions=g['positions'], charge_sign=TernaryState.POSITIVE)
        print(f"  r={g['mean_r']:.3f}: E_coulomb={t.coulomb_energy:.6e}, net={t.net_energy:.6e}, bound={t.is_energetically_bound}")

    return True


if __name__ == "__main__":
    verify_level_3()
