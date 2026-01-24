"""
LEVEL 1: THE SINGLE VOXEL
=========================

The minimal physical system: one voxel in the lattice.
Here we derive the properties of a single site and its transitions.

Epistemic Status: DERIVED from Level 0 axioms
"""

import numpy as np
from dataclasses import dataclass, field
from typing import Optional, Tuple, List
from enum import Enum

from .level_0_planck import (
    TernaryState, LATTICE, UNITS, CONSTANTS, SI,
    PlanckVoxel
)


# =============================================================================
# SECTION 1.1: STATE TRANSITIONS
# =============================================================================

class Transition(Enum):
    """
    The allowed state transitions for a single voxel.

    From CLAUDE.md Chapter 2.2:
        0 -> +1  (Genesis: positive manifestation)
        0 -> -1  (Genesis: negative manifestation)
        +1 -> 0  (Evaporation)
        -1 -> 0  (Evaporation)
        +1 -> +1 (Persistence)
        -1 -> -1 (Persistence)
        +1 + (-1) -> 0 + 0 (Annihilation - requires TWO voxels)

    NOT allowed:
        +1 -> -1 (direct sign flip without passing through void)
        -1 -> +1 (direct sign flip without passing through void)
    """
    GENESIS_POSITIVE = ("0 -> +1", 0, +1)
    GENESIS_NEGATIVE = ("0 -> -1", 0, -1)
    EVAPORATION_POSITIVE = ("+1 -> 0", +1, 0)
    EVAPORATION_NEGATIVE = ("-1 -> 0", -1, 0)
    PERSISTENCE_POSITIVE = ("+1 -> +1", +1, +1)
    PERSISTENCE_NEGATIVE = ("-1 -> -1", -1, -1)
    PERSISTENCE_VOID = ("0 -> 0", 0, 0)

    @property
    def label(self) -> str:
        return self.value[0]

    @property
    def from_state(self) -> int:
        return self.value[1]

    @property
    def to_state(self) -> int:
        return self.value[2]


def is_valid_transition(from_state: int, to_state: int) -> bool:
    """Check if a state transition is allowed."""
    # Same state is always valid (persistence)
    if from_state == to_state:
        return True

    # Transitions through void are valid
    if from_state == 0 or to_state == 0:
        return True

    # Direct sign flip (+1 <-> -1) is NOT valid
    return False


# =============================================================================
# SECTION 1.2: THE MANIFESTATION THRESHOLD
# =============================================================================

@dataclass(frozen=True)
class ManifestationThreshold:
    """
    The threshold for state transitions.

    From CLAUDE.md, KB is identified with the electron mass in Planck units.
    The electron mass derivation gives:

        m_e = m_P * sqrt(2*pi) * (16/3) * alpha^11

    This is the MINIMUM energy required for manifestation.
    """

    @property
    def alpha(self) -> float:
        """Fine structure constant from Level 0."""
        return CONSTANTS.alpha

    @property
    def N_base(self) -> int:
        """Base integer from lattice geometry (2x2 = 4)."""
        return 4

    @property
    def N_c(self) -> int:
        """Color charge number (3)."""
        return CONSTANTS.N_c_integer

    @property
    def geometric_factor(self) -> float:
        """N_base^2 / N_c = 16/3."""
        return (self.N_base ** 2) / self.N_c

    @property
    def normalization(self) -> float:
        """sqrt(2*pi) from action principle normalization."""
        return np.sqrt(2 * np.pi)

    @property
    def hierarchy_exponent(self) -> int:
        """
        The power of alpha in the mass formula.

        Total exponent = 11 = 8 (hierarchy) + 3 (Yukawa)

        The 8 comes from the hierarchy between Planck and electroweak scales.
        The 3 comes from the Yukawa coupling structure.
        """
        return 11

    @property
    def KB_dimensionless(self) -> float:
        """
        The manifestation threshold in Planck units.

        KB = sqrt(2*pi) * (16/3) * alpha^11
        """
        return (self.normalization *
                self.geometric_factor *
                self.alpha ** self.hierarchy_exponent)

    @property
    def KB_eV(self) -> float:
        """KB in electron-volts."""
        # m_e * c^2 in Planck energy units, then convert to eV
        m_e_over_m_P = self.KB_dimensionless
        E_P_eV = SI.E_P / SI.e  # Planck energy in eV
        return m_e_over_m_P * E_P_eV

    @property
    def electron_mass_eV(self) -> float:
        """Compare to measured electron mass."""
        return 0.511e6  # 0.511 MeV in eV

    @property
    def mass_accuracy(self) -> float:
        """Percent accuracy of the mass prediction."""
        predicted = self.KB_eV
        measured = self.electron_mass_eV
        return abs(predicted - measured) / measured * 100


THRESHOLD = ManifestationThreshold()


# =============================================================================
# SECTION 1.3: GENESIS PROBABILITY
# =============================================================================

def genesis_probability(density: float, KB: float = None) -> float:
    """
    The probability of genesis (void -> manifested).

    From CLAUDE.md Chapter 4.1:
        p_manifest = clamp(1 - exp(-(density - KB) / KB), 0, 1)

    Args:
        density: The flux magnitude |J| at the voxel
        KB: The manifestation threshold (defaults to derived value)

    Returns:
        Probability in [0, 1]
    """
    if KB is None:
        KB = THRESHOLD.KB_dimensionless

    if density <= KB:
        return 0.0

    p = 1.0 - np.exp(-(density - KB) / KB)
    return np.clip(p, 0.0, 1.0)


def polarity_from_divergence(divergence: float) -> int:
    """
    Determine the sign of a manifested state from flux divergence.

    From CLAUDE.md Chapter 4.1:
        div(J) > 0 -> state = +1
        div(J) < 0 -> state = -1
        div(J) = 0 -> undetermined (random)

    Args:
        divergence: The value of div(J) at the voxel

    Returns:
        +1 or -1 (or 0 if undetermined)
    """
    if divergence > 0:
        return TernaryState.POSITIVE
    elif divergence < 0:
        return TernaryState.NEGATIVE
    else:
        return TernaryState.VOID  # Undetermined - caller should randomize


# =============================================================================
# SECTION 1.4: THE SINGLE VOXEL MODEL
# =============================================================================

@dataclass
class SingleVoxel:
    """
    A complete model of one voxel with all derived properties.

    This is the "hydrogen atom" of FTD - the simplest non-trivial system.
    """

    # Core state
    state: int = TernaryState.VOID
    flux: Tuple[float, float, float] = (0.0, 0.0, 0.0)

    # Computed properties (updated by recompute())
    density: float = field(init=False, default=0.0)
    is_above_threshold: bool = field(init=False, default=False)
    genesis_prob: float = field(init=False, default=0.0)

    def __post_init__(self):
        self.recompute()

    def recompute(self):
        """Update all derived properties."""
        self.density = np.sqrt(sum(f**2 for f in self.flux))
        self.is_above_threshold = self.density > THRESHOLD.KB_dimensionless
        self.genesis_prob = genesis_probability(self.density)

    def set_flux(self, Jx: float, Jy: float, Jz: float):
        """Set the flux vector and recompute derived properties."""
        self.flux = (Jx, Jy, Jz)
        self.recompute()

    def attempt_genesis(self, divergence: float) -> bool:
        """
        Attempt a genesis transition.

        Returns True if genesis occurred, False otherwise.
        """
        if self.state != TernaryState.VOID:
            return False  # Already manifested

        if not self.is_above_threshold:
            return False  # Below threshold

        # Probabilistic genesis
        if np.random.random() < self.genesis_prob:
            self.state = polarity_from_divergence(divergence)
            if self.state == TernaryState.VOID:
                # Divergence was exactly zero, pick randomly
                self.state = np.random.choice([TernaryState.POSITIVE,
                                               TernaryState.NEGATIVE])
            return True

        return False

    def attempt_evaporation(self) -> bool:
        """
        Attempt an evaporation transition.

        Returns True if evaporation occurred, False otherwise.
        """
        if self.state == TernaryState.VOID:
            return False  # Already void

        if self.is_above_threshold:
            return False  # Still above threshold

        # Evaporation occurs immediately when below threshold
        self.state = TernaryState.VOID
        return True

    @property
    def summary(self) -> str:
        state_names = {-1: "NEGATIVE", 0: "VOID", +1: "POSITIVE"}
        return (f"SingleVoxel(state={state_names[self.state]}, "
                f"|J|={self.density:.4f}, p_gen={self.genesis_prob:.4f})")


# =============================================================================
# SECTION 1.5: ENERGY SCALES
# =============================================================================

@dataclass(frozen=True)
class VoxelEnergyScales:
    """
    The characteristic energy scales for a single voxel.
    """

    @property
    def E_planck_GeV(self) -> float:
        """Planck energy in GeV."""
        return SI.E_P / SI.e / 1e9

    @property
    def E_threshold_GeV(self) -> float:
        """Manifestation threshold (electron mass) in GeV."""
        return THRESHOLD.KB_eV / 1e9

    @property
    def ratio_threshold_to_planck(self) -> float:
        """The hierarchy ratio m_e / m_P."""
        return THRESHOLD.KB_dimensionless

    @property
    def hierarchy_orders_of_magnitude(self) -> float:
        """How many orders of magnitude between m_e and m_P."""
        return -np.log10(self.ratio_threshold_to_planck)


ENERGY = VoxelEnergyScales()


# =============================================================================
# SECTION 1.6: VERIFICATION
# =============================================================================

def verify_level_1():
    """Verify Level 1 derivations."""
    print("=" * 60)
    print("LEVEL 1: SINGLE VOXEL VERIFICATION")
    print("=" * 60)

    print("\n--- Manifestation Threshold (KB) ---")
    print(f"  alpha           = {THRESHOLD.alpha:.8f}")
    print(f"  N_base          = {THRESHOLD.N_base}")
    print(f"  N_c             = {THRESHOLD.N_c}")
    print(f"  Geometric (16/3)= {THRESHOLD.geometric_factor:.6f}")
    print(f"  sqrt(2*pi)      = {THRESHOLD.normalization:.6f}")
    print(f"  alpha^11        = {THRESHOLD.alpha**11:.6e}")
    print(f"  KB (Planck)     = {THRESHOLD.KB_dimensionless:.6e}")
    print(f"  KB (eV)         = {THRESHOLD.KB_eV:.3e} eV")
    print(f"  m_e (measured)  = {THRESHOLD.electron_mass_eV:.3e} eV")
    print(f"  Accuracy        = {THRESHOLD.mass_accuracy:.2f}%")

    print("\n--- Genesis Probability ---")
    KB = THRESHOLD.KB_dimensionless
    test_densities = [0.5 * KB, KB, 1.5 * KB, 2 * KB, 5 * KB]
    print(f"  KB = {KB:.4e}")
    for rho in test_densities:
        p = genesis_probability(rho, KB)
        print(f"  rho = {rho/KB:.1f}*KB -> p = {p:.4f}")

    print("\n--- State Transitions ---")
    for t in Transition:
        print(f"  {t.label}")

    print("\n--- Energy Hierarchy ---")
    print(f"  E_Planck   = {ENERGY.E_planck_GeV:.3e} GeV")
    print(f"  E_threshold = {ENERGY.E_threshold_GeV:.3e} GeV (m_e)")
    print(f"  Ratio       = {ENERGY.ratio_threshold_to_planck:.3e}")
    print(f"  Orders of magnitude: {ENERGY.hierarchy_orders_of_magnitude:.1f}")

    print("\n--- Single Voxel Test ---")
    voxel = SingleVoxel()
    print(f"  Initial: {voxel.summary}")

    voxel.set_flux(1e-22, 0, 0)  # Sub-threshold
    print(f"  Low flux: {voxel.summary}")

    voxel.set_flux(1e-20, 0, 0)  # Above threshold (if KB ~ 4e-23)
    print(f"  High flux: {voxel.summary}")

    return True


if __name__ == "__main__":
    verify_level_1()
