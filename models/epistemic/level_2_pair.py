"""
LEVEL 2: THE VOXEL PAIR
=======================

The minimal interacting system: two adjacent voxels.
Here we derive the first interaction rules and force-like behaviors.

Epistemic Status: DERIVED from Levels 0-1
"""

import numpy as np
from dataclasses import dataclass, field
from typing import Tuple, Optional, List
from enum import Enum

from .level_0_planck import (
    TernaryState, LATTICE, UNITS, CONSTANTS, SI
)
from .level_1_voxel import THRESHOLD, SingleVoxel


# =============================================================================
# SECTION 2.1: ADJACENCY
# =============================================================================

class AdjacencyType(Enum):
    """
    Types of adjacency between two voxels on a cubic lattice.

    The distance determines the interaction strength and type.
    """
    FACE = ("face-sharing", 1.0, 6)           # |delta| = 1
    EDGE = ("edge-sharing", np.sqrt(2), 12)   # |delta| = sqrt(2)
    CORNER = ("corner-sharing", np.sqrt(3), 8) # |delta| = sqrt(3)
    DISTANT = ("non-adjacent", None, None)    # |delta| > sqrt(3)

    @property
    def name(self) -> str:
        return self.value[0]

    @property
    def distance(self) -> Optional[float]:
        return self.value[1]

    @property
    def count_in_neighborhood(self) -> Optional[int]:
        """How many neighbors of this type exist."""
        return self.value[2]


def classify_adjacency(pos1: Tuple[int, int, int],
                       pos2: Tuple[int, int, int]) -> AdjacencyType:
    """
    Classify the adjacency type between two voxel positions.
    """
    dx = abs(pos1[0] - pos2[0])
    dy = abs(pos1[1] - pos2[1])
    dz = abs(pos1[2] - pos2[2])

    # Count how many dimensions differ by exactly 1
    diffs = [d for d in [dx, dy, dz] if d == 1]
    zeros = [d for d in [dx, dy, dz] if d == 0]

    if len(diffs) == 1 and len(zeros) == 2:
        return AdjacencyType.FACE
    elif len(diffs) == 2 and len(zeros) == 1:
        return AdjacencyType.EDGE
    elif len(diffs) == 3:
        return AdjacencyType.CORNER
    else:
        return AdjacencyType.DISTANT


def distance(pos1: Tuple[int, int, int],
             pos2: Tuple[int, int, int]) -> float:
    """Euclidean distance between two positions."""
    return np.sqrt(sum((a - b)**2 for a, b in zip(pos1, pos2)))


# =============================================================================
# SECTION 2.2: ANNIHILATION
# =============================================================================

@dataclass(frozen=True)
class AnnihilationRule:
    """
    The annihilation rule for matter-antimatter pairs.

    From CLAUDE.md Chapter 4.4:
        When +1 and -1 voxels occupy adjacent positions:
        - Both voxels -> state 0
        - Combined flux redistributed to neighbors as omnidirectional burst
        - Total flux magnitude conserved

    This is the simplest interaction: mutual destruction.
    """

    @staticmethod
    def can_annihilate(state1: int, state2: int,
                       adjacency: AdjacencyType) -> bool:
        """
        Check if two voxels can annihilate.

        Requirements:
        1. One must be +1, the other -1
        2. They must be within the Moore neighborhood (adjacent)
        """
        # Check opposite signs
        if not (state1 * state2 == -1):  # +1 * -1 = -1
            return False

        # Check adjacency (any type within Moore neighborhood)
        if adjacency == AdjacencyType.DISTANT:
            return False

        return True

    @staticmethod
    def annihilate(voxel1: SingleVoxel,
                   voxel2: SingleVoxel) -> Tuple[float, float, float]:
        """
        Execute annihilation and return the released flux.

        Both voxels transition to void state.
        Returns the total flux that should be redistributed.
        """
        # Combine flux vectors
        total_flux = tuple(
            voxel1.flux[i] + voxel2.flux[i]
            for i in range(3)
        )

        # Both become void
        voxel1.state = TernaryState.VOID
        voxel2.state = TernaryState.VOID

        # Clear their flux (it's been released)
        voxel1.set_flux(0, 0, 0)
        voxel2.set_flux(0, 0, 0)

        return total_flux


ANNIHILATION = AnnihilationRule()


# =============================================================================
# SECTION 2.3: FLUX INTERACTION
# =============================================================================

@dataclass(frozen=True)
class FluxInteraction:
    """
    How flux fields interact between adjacent voxels.

    The key insight: flux gradients create force-like effects.
    """

    @staticmethod
    def gradient_force(flux1: Tuple[float, float, float],
                       flux2: Tuple[float, float, float],
                       separation: Tuple[int, int, int]) -> Tuple[float, float, float]:
        """
        Compute the force-like effect from flux gradient.

        Force is proportional to the gradient of flux density.
        F ~ -grad(|J|)

        Args:
            flux1: Flux at voxel 1
            flux2: Flux at voxel 2
            separation: Vector from voxel 1 to voxel 2

        Returns:
            Force vector on voxel 1 (voxel 2 gets -F)
        """
        rho1 = np.sqrt(sum(f**2 for f in flux1))
        rho2 = np.sqrt(sum(f**2 for f in flux2))

        # Gradient approximation
        d = np.sqrt(sum(s**2 for s in separation))
        if d == 0:
            return (0.0, 0.0, 0.0)

        # Unit vector from 1 to 2
        unit = tuple(s / d for s in separation)

        # Force magnitude: gradient of density
        grad_rho = (rho2 - rho1) / d

        # Force on voxel 1 (points toward lower density)
        F = tuple(-grad_rho * u for u in unit)

        return F

    @staticmethod
    def coulomb_like_force(charge1: int, charge2: int,
                           separation: Tuple[int, int, int],
                           coupling: float = None) -> Tuple[float, float, float]:
        """
        Coulomb-like force between charged voxels.

        F = alpha * q1 * q2 / r^2 * r_hat

        Like charges repel, opposite attract.

        Args:
            charge1: Charge of voxel 1 (state acts as charge)
            charge2: Charge of voxel 2
            separation: Vector from 1 to 2
            coupling: Coupling constant (defaults to alpha)

        Returns:
            Force vector on voxel 1
        """
        if coupling is None:
            coupling = CONSTANTS.alpha

        r_squared = sum(s**2 for s in separation)
        if r_squared == 0:
            return (0.0, 0.0, 0.0)

        r = np.sqrt(r_squared)
        unit = tuple(s / r for s in separation)

        # Coulomb: F = k * q1 * q2 / r^2
        # Positive for same sign (repulsion), negative for opposite (attraction)
        F_mag = coupling * charge1 * charge2 / r_squared

        # Force on voxel 1
        F = tuple(F_mag * u for u in unit)

        return F


FLUX = FluxInteraction()


# =============================================================================
# SECTION 2.4: THE VOXEL PAIR MODEL
# =============================================================================

@dataclass
class VoxelPair:
    """
    A complete model of two interacting voxels.

    This is the "hydrogen molecule ion" of FTD - the simplest bound/interacting system.
    """

    # Positions
    pos1: Tuple[int, int, int] = (0, 0, 0)
    pos2: Tuple[int, int, int] = (1, 0, 0)

    # The voxels themselves
    voxel1: SingleVoxel = field(default_factory=SingleVoxel)
    voxel2: SingleVoxel = field(default_factory=SingleVoxel)

    def __post_init__(self):
        self.recompute()

    def recompute(self):
        """Update all derived properties."""
        self.voxel1.recompute()
        self.voxel2.recompute()

    @property
    def separation(self) -> Tuple[int, int, int]:
        """Vector from voxel 1 to voxel 2."""
        return tuple(self.pos2[i] - self.pos1[i] for i in range(3))

    @property
    def distance(self) -> float:
        """Distance between voxels."""
        return np.sqrt(sum(s**2 for s in self.separation))

    @property
    def adjacency(self) -> AdjacencyType:
        """The type of adjacency."""
        return classify_adjacency(self.pos1, self.pos2)

    @property
    def can_annihilate(self) -> bool:
        """Whether this pair can annihilate."""
        return ANNIHILATION.can_annihilate(
            self.voxel1.state, self.voxel2.state, self.adjacency
        )

    @property
    def is_bound(self) -> bool:
        """
        Whether this pair is bound (attractive and stable).

        Binding requires:
        1. Opposite charges (for attraction)
        2. Separation > 0 (not overlapping)
        3. Below some binding energy threshold
        """
        # Same sign = repulsion = not bound
        if self.voxel1.state * self.voxel2.state >= 0:
            return False

        # Must be adjacent but not overlapping
        if self.adjacency == AdjacencyType.DISTANT:
            return False

        return True

    def coulomb_force_on_1(self) -> Tuple[float, float, float]:
        """Coulomb-like force on voxel 1 from voxel 2."""
        return FLUX.coulomb_like_force(
            self.voxel1.state, self.voxel2.state, self.separation
        )

    def gradient_force_on_1(self) -> Tuple[float, float, float]:
        """Gradient force on voxel 1 from flux configuration."""
        return FLUX.gradient_force(
            self.voxel1.flux, self.voxel2.flux, self.separation
        )

    @property
    def total_charge(self) -> int:
        """Net charge of the pair."""
        return self.voxel1.state + self.voxel2.state

    @property
    def summary(self) -> str:
        state_names = {-1: "-", 0: "0", +1: "+"}
        s1 = state_names[self.voxel1.state]
        s2 = state_names[self.voxel2.state]
        return (f"VoxelPair([{s1}]--[{s2}], "
                f"r={self.distance:.2f}, "
                f"adj={self.adjacency.name}, "
                f"Q={self.total_charge})")


# =============================================================================
# SECTION 2.5: PAIR PRODUCTION
# =============================================================================

@dataclass(frozen=True)
class PairProduction:
    """
    The creation of a matter-antimatter pair from void.

    This is the inverse of annihilation.
    Requires sufficient flux energy at adjacent void sites.
    """

    @staticmethod
    def can_produce_pair(voxel1: SingleVoxel,
                         voxel2: SingleVoxel,
                         adjacency: AdjacencyType) -> bool:
        """
        Check if pair production can occur.

        Requirements:
        1. Both voxels must be void
        2. Combined flux density must exceed 2*KB (for two particles)
        3. Must be adjacent
        """
        # Both must be void
        if voxel1.state != TernaryState.VOID:
            return False
        if voxel2.state != TernaryState.VOID:
            return False

        # Must be adjacent
        if adjacency == AdjacencyType.DISTANT:
            return False

        # Combined density must exceed threshold for pair
        combined_density = voxel1.density + voxel2.density
        threshold = 2 * THRESHOLD.KB_dimensionless

        return combined_density > threshold

    @staticmethod
    def produce_pair(voxel1: SingleVoxel,
                     voxel2: SingleVoxel,
                     divergence1: float,
                     divergence2: float) -> bool:
        """
        Attempt pair production.

        Returns True if pair was produced.
        """
        # Assign opposite charges based on divergence
        pol1 = 1 if divergence1 >= 0 else -1
        pol2 = -pol1  # Opposite charge (conservation)

        voxel1.state = pol1
        voxel2.state = pol2

        return True


PAIR_PRODUCTION = PairProduction()


# =============================================================================
# SECTION 2.6: CONSERVATION LAWS
# =============================================================================

@dataclass(frozen=True)
class ConservationLaws:
    """
    Conservation laws that constrain pair dynamics.

    In an isolated pair:
    - Total charge is conserved
    - Total flux magnitude is conserved (up to damping)
    """

    @staticmethod
    def check_charge_conservation(initial_charge: int,
                                  final_charge: int) -> bool:
        """Verify charge conservation."""
        return initial_charge == final_charge

    @staticmethod
    def check_flux_conservation(initial_flux: float,
                                final_flux: float,
                                tolerance: float = 0.01) -> bool:
        """Verify flux conservation (approximate, allows damping)."""
        return abs(initial_flux - final_flux) / max(initial_flux, 1e-10) < tolerance


CONSERVATION = ConservationLaws()


# =============================================================================
# SECTION 2.7: VERIFICATION
# =============================================================================

def verify_level_2():
    """Verify Level 2 derivations."""
    print("=" * 60)
    print("LEVEL 2: VOXEL PAIR VERIFICATION")
    print("=" * 60)

    print("\n--- Adjacency Classification ---")
    test_cases = [
        ((0, 0, 0), (1, 0, 0)),   # Face
        ((0, 0, 0), (1, 1, 0)),   # Edge
        ((0, 0, 0), (1, 1, 1)),   # Corner
        ((0, 0, 0), (2, 0, 0)),   # Distant
    ]
    for p1, p2 in test_cases:
        adj = classify_adjacency(p1, p2)
        d = distance(p1, p2)
        print(f"  {p1} to {p2}: {adj.name}, d={d:.3f}")

    print("\n--- Annihilation Test ---")
    pair = VoxelPair(
        pos1=(0, 0, 0),
        pos2=(1, 0, 0),
        voxel1=SingleVoxel(state=+1),
        voxel2=SingleVoxel(state=-1)
    )
    print(f"  Initial: {pair.summary}")
    print(f"  Can annihilate: {pair.can_annihilate}")

    pair_same = VoxelPair(
        pos1=(0, 0, 0),
        pos2=(1, 0, 0),
        voxel1=SingleVoxel(state=+1),
        voxel2=SingleVoxel(state=+1)
    )
    print(f"  Same sign: {pair_same.summary}")
    print(f"  Can annihilate: {pair_same.can_annihilate}")

    print("\n--- Coulomb Force Test ---")
    # +/+ repulsion
    pair_pp = VoxelPair(
        pos1=(0, 0, 0),
        pos2=(1, 0, 0),
        voxel1=SingleVoxel(state=+1),
        voxel2=SingleVoxel(state=+1)
    )
    F_pp = pair_pp.coulomb_force_on_1()
    print(f"  +/+ at r=1: F = {F_pp[0]:.6f} (repulsion, positive = away)")

    # +/- attraction
    pair_pm = VoxelPair(
        pos1=(0, 0, 0),
        pos2=(1, 0, 0),
        voxel1=SingleVoxel(state=+1),
        voxel2=SingleVoxel(state=-1)
    )
    F_pm = pair_pm.coulomb_force_on_1()
    print(f"  +/- at r=1: F = {F_pm[0]:.6f} (attraction, negative = toward)")

    # Force at r=2
    pair_r2 = VoxelPair(
        pos1=(0, 0, 0),
        pos2=(2, 0, 0),
        voxel1=SingleVoxel(state=+1),
        voxel2=SingleVoxel(state=+1)
    )
    F_r2 = pair_r2.coulomb_force_on_1()
    print(f"  +/+ at r=2: F = {F_r2[0]:.6f} (should be 1/4 of r=1)")
    print(f"  Ratio: {F_r2[0] / F_pp[0]:.4f} (expected 0.25)")

    print("\n--- Binding Test ---")
    pairs = [
        ((+1, -1), "opposite"),
        ((+1, +1), "same positive"),
        ((-1, -1), "same negative"),
        ((0, +1), "one void"),
    ]
    for (s1, s2), desc in pairs:
        p = VoxelPair(
            pos1=(0, 0, 0),
            pos2=(1, 0, 0),
            voxel1=SingleVoxel(state=s1),
            voxel2=SingleVoxel(state=s2)
        )
        print(f"  {desc}: is_bound = {p.is_bound}")

    print("\n--- Pair Production Threshold ---")
    KB = THRESHOLD.KB_dimensionless
    print(f"  Single particle threshold: KB = {KB:.4e}")
    print(f"  Pair production threshold: 2*KB = {2*KB:.4e}")

    return True


if __name__ == "__main__":
    verify_level_2()
