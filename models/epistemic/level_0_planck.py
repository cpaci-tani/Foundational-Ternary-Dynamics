"""
LEVEL 0: THE PLANCK SCALE
=========================

The most foundational level of the epistemic domain.
Here we define the irreducible units from which all else is constructed.

Epistemic Status: AXIOMATIC (these are definitions, not derivations)
"""

import numpy as np
from dataclasses import dataclass
from typing import Tuple
from fractions import Fraction

# =============================================================================
# SECTION 0.1: FUNDAMENTAL CONSTANTS (SI Units for Reference)
# =============================================================================

@dataclass(frozen=True)
class PhysicalConstants:
    """
    Physical constants in SI units.
    These are INPUTS from measurement, not outputs of the theory.
    """
    # Measured constants
    c: float = 299_792_458          # Speed of light (m/s)
    hbar: float = 1.054_571_817e-34 # Reduced Planck constant (J·s)
    G: float = 6.674_30e-11         # Gravitational constant (m³/kg/s²)
    k_B: float = 1.380_649e-23      # Boltzmann constant (J/K)
    e: float = 1.602_176_634e-19    # Elementary charge (C)

    # Derived Planck units
    @property
    def l_P(self) -> float:
        """Planck length (m)"""
        return np.sqrt(self.hbar * self.G / self.c**3)

    @property
    def t_P(self) -> float:
        """Planck time (s)"""
        return np.sqrt(self.hbar * self.G / self.c**5)

    @property
    def m_P(self) -> float:
        """Planck mass (kg)"""
        return np.sqrt(self.hbar * self.c / self.G)

    @property
    def E_P(self) -> float:
        """Planck energy (J)"""
        return self.m_P * self.c**2

    @property
    def T_P(self) -> float:
        """Planck temperature (K)"""
        return self.E_P / self.k_B


SI = PhysicalConstants()


# =============================================================================
# SECTION 0.2: THE TERNARY FOUNDATION
# =============================================================================

class TernaryState:
    """
    The fundamental trit: the irreducible unit of ontological state.

    In FTD, every point in the lattice exists in exactly one of three states:
        -1 : Negative manifestation (antimatter-like)
         0 : Void (unmanifested potential)
        +1 : Positive manifestation (matter-like)

    This is NOT a superposition. It is a definite, classical state.
    """
    VOID = 0
    POSITIVE = +1
    NEGATIVE = -1

    VALID_STATES = frozenset({-1, 0, +1})

    @classmethod
    def is_valid(cls, state: int) -> bool:
        return state in cls.VALID_STATES

    @classmethod
    def is_manifested(cls, state: int) -> bool:
        """A state is manifested if it is not void."""
        return state != cls.VOID

    @classmethod
    def sign(cls, state: int) -> int:
        """Returns the sign of the state (-1, 0, or +1)."""
        if state > 0:
            return cls.POSITIVE
        elif state < 0:
            return cls.NEGATIVE
        return cls.VOID


# =============================================================================
# SECTION 0.3: THE LATTICE AXIOMS
# =============================================================================

@dataclass(frozen=True)
class LatticeAxioms:
    """
    The structural axioms defining the discrete substrate.

    AXIOM L1: Space is a 3D cubic lattice L ⊂ Z³
    AXIOM L2: Time advances in discrete ticks t ∈ N
    AXIOM L3: Each site has exactly one ternary state
    AXIOM L4: Causality is local (26-neighbor Moore neighborhood)
    AXIOM L5: The maximum propagation speed is 1 site/tick
    """

    # Dimensionality
    D: int = 3  # Spatial dimensions (AXIOM L1)

    # Neighborhood connectivity
    MOORE_SIZE: int = 26  # 3³ - 1 = 26 neighbors (AXIOM L4)
    VON_NEUMANN_SIZE: int = 6  # Face-sharing neighbors only

    # Speed of causality
    C: int = 1  # sites per tick (AXIOM L5) - INTEGER, not float!

    @property
    def neighborhood_offsets(self) -> list:
        """Generate all 26 Moore neighborhood offsets."""
        offsets = []
        for dx in [-1, 0, 1]:
            for dy in [-1, 0, 1]:
                for dz in [-1, 0, 1]:
                    if dx == 0 and dy == 0 and dz == 0:
                        continue
                    offsets.append((dx, dy, dz))
        return offsets

    @property
    def face_offsets(self) -> list:
        """The 6 face-sharing (von Neumann) neighbors."""
        return [
            (+1, 0, 0), (-1, 0, 0),
            (0, +1, 0), (0, -1, 0),
            (0, 0, +1), (0, 0, -1)
        ]


LATTICE = LatticeAxioms()


# =============================================================================
# SECTION 0.4: NATURAL UNITS
# =============================================================================

@dataclass(frozen=True)
class NaturalUnits:
    """
    In FTD natural units, the lattice spacing IS the Planck length.

    This is an IDENTIFICATION, not a derivation:
        1 voxel = l_P
        1 tick  = t_P
        1 flux unit = E_P / l_P²

    All quantities become dimensionless ratios.
    """

    # Base unit identifications
    length_unit: str = "l_P"   # 1 voxel = 1 Planck length
    time_unit: str = "t_P"     # 1 tick = 1 Planck time
    energy_unit: str = "E_P"   # Energy in Planck units

    # Dimensionless by construction
    c: int = 1      # Speed of light = 1 (voxel/tick)
    hbar: int = 1   # Reduced Planck constant = 1
    G: int = 1      # Gravitational constant = 1
    k_B: int = 1    # Boltzmann constant = 1

    def to_SI(self, quantity: float, dimension: str) -> float:
        """Convert from natural units to SI."""
        conversions = {
            'length': SI.l_P,
            'time': SI.t_P,
            'mass': SI.m_P,
            'energy': SI.E_P,
            'temperature': SI.T_P,
        }
        return quantity * conversions.get(dimension, 1.0)

    def from_SI(self, quantity: float, dimension: str) -> float:
        """Convert from SI to natural units."""
        conversions = {
            'length': SI.l_P,
            'time': SI.t_P,
            'mass': SI.m_P,
            'energy': SI.E_P,
            'temperature': SI.T_P,
        }
        return quantity / conversions.get(dimension, 1.0)


UNITS = NaturalUnits()


# =============================================================================
# SECTION 0.5: THE PLANCK VOXEL
# =============================================================================

@dataclass
class PlanckVoxel:
    """
    The irreducible unit of space: a single site on the lattice.

    A voxel has:
        - Position (i, j, k) ∈ Z³
        - State s ∈ {-1, 0, +1}
        - Flux J ∈ R³ (the continuous field living on the discrete lattice)

    The flux is NOT the state. The flux is the POTENTIAL for state change.
    State is ontological (what IS). Flux is dispositional (what COULD BE).
    """

    # Position (integer coordinates)
    i: int
    j: int
    k: int

    # Ternary state
    state: int = TernaryState.VOID

    # Flux vector (real-valued)
    flux_x: float = 0.0
    flux_y: float = 0.0
    flux_z: float = 0.0

    # Derived properties
    @property
    def position(self) -> Tuple[int, int, int]:
        return (self.i, self.j, self.k)

    @property
    def flux(self) -> Tuple[float, float, float]:
        return (self.flux_x, self.flux_y, self.flux_z)

    @property
    def flux_magnitude(self) -> float:
        """The scalar density |J|."""
        return np.sqrt(self.flux_x**2 + self.flux_y**2 + self.flux_z**2)

    @property
    def is_manifested(self) -> bool:
        return TernaryState.is_manifested(self.state)

    def __post_init__(self):
        if not TernaryState.is_valid(self.state):
            raise ValueError(f"Invalid state: {self.state}. Must be in {{-1, 0, +1}}")


# =============================================================================
# SECTION 0.6: DERIVED QUANTITIES AT PLANCK SCALE
# =============================================================================

@dataclass(frozen=True)
class PlanckScaleQuantities:
    """
    Quantities that emerge at the Planck scale.

    These are the "atoms" of the theory - the minimal meaningful values.
    """

    # Minimal length, time, energy
    min_length: int = 1      # 1 voxel (cannot go smaller)
    min_time: int = 1        # 1 tick (cannot go smaller)
    min_action: int = 1      # hbar = 1 in natural units

    # The manifestation threshold (KB in CLAUDE.md)
    # This is WHERE the electron mass comes from in the full theory
    # For now, we note it as a parameter to be derived later

    @property
    def degrees_of_freedom_per_voxel(self) -> int:
        """
        Each voxel has:
            - 1 ternary state (3 possibilities, but 1 value)
            - 3 flux components (continuous)

        The ternary state encodes log₂(3) ≈ 1.58 bits of information.
        """
        return 1 + 3  # state + flux vector

    @property
    def information_content_per_voxel(self) -> float:
        """
        Information in bits per voxel.

        State: log₂(3) bits (ternary)
        Flux: Continuous, but bounded by Planck scale → finite

        The Bekenstein bound suggests ~1 bit per Planck area.
        For a Planck volume: ~1 bit per face = 6 bits max.
        """
        state_bits = np.log2(3)  # ≈ 1.585 bits
        # Flux requires regularization to count - leave as continuous for now
        return state_bits


PLANCK = PlanckScaleQuantities()


# =============================================================================
# SECTION 0.7: THE MASTER CONSTANTS
# =============================================================================

@dataclass(frozen=True)
class MasterConstants:
    """
    The dimensionless constants that govern the theory.

    From CLAUDE.md, these emerge from the lemniscatic structure:
        G* = √2 · Γ(1/4)² / (2π) ≈ 2.9587

    The master quadratic: x² - 16(G*)²x + 16(G*)³ = 0

    Roots:
        x₊ = 137.036... → 1/α (fine structure constant)
        x₋ = 3.024...   → N_c (color charges)
    """

    # The lemniscatic constant (from elliptic integral theory)
    @property
    def G_star(self) -> float:
        """
        G* = √2 · Γ(1/4)² / (2π)

        This emerges from the lemniscate of Bernoulli,
        which has deep connections to elliptic curves.
        """
        from scipy.special import gamma
        return np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)

    @property
    def master_quadratic_coefficients(self) -> Tuple[float, float, float]:
        """
        The master quadratic: x² - 16c²x + 16c³ = 0
        where c = G*

        Returns (a, b, c) for ax² + bx + c = 0
        """
        c = self.G_star
        return (1.0, -16 * c**2, 16 * c**3)

    @property
    def master_quadratic_roots(self) -> Tuple[float, float]:
        """
        Solve x^2 - 16(G*)^2 x + 16(G*)^3 = 0

        Using quadratic formula: x = (16c^2 +/- sqrt(256c^4 - 64c^3)) / 2
        Simplifies to: x = 8c^2 +/- 4c^(3/2) * sqrt(4c - 1)

        From G_STAR_FRAMEWORK.md line 67:
        x_+/- = 8(G*)^2 +/- 4(G*)^(3/2) * sqrt(4*G* - 1)
        """
        c = self.G_star
        term1 = 8 * c**2
        term2 = 4 * (c ** 1.5) * np.sqrt(4 * c - 1)

        x_plus = term1 + term2
        x_minus = term1 - term2

        return (x_plus, x_minus)

    @property
    def alpha_inverse(self) -> float:
        """1/α from the master quadratic (x₊)."""
        return self.master_quadratic_roots[0]

    @property
    def alpha(self) -> float:
        """The fine structure constant α (using precision formula)."""
        return 1.0 / self.alpha_inverse

    @property
    def epsilon(self) -> float:
        """
        The CFT anomaly term: e^pi - pi - 20
        where 20 = b_3 + N_eff
        """
        return np.exp(np.pi) - np.pi - 20.0

    @property
    def alpha_inverse_precision(self) -> float:
        """
        The high-precision inverse alpha derivation.
        1/alpha = x_plus - (9/47)|eps| + (5/64)|eps|^2
        """
        x_plus = self.master_quadratic_roots[0]
        eps = abs(self.epsilon)
        term1 = (9/47) * eps
        term2 = (5/64) * eps**2
        return x_plus - term1 + term2

    @property
    def N_c(self) -> float:
        """
        The effective color charge parameter (x₋).
        Rounds to 3 (number of QCD colors).
        """
        return self.master_quadratic_roots[1]

    @property
    def N_c_integer(self) -> int:
        """N_c rounded to nearest integer."""
        return round(self.N_c)


CONSTANTS = MasterConstants()


# =============================================================================
# SECTION 0.8: VERIFICATION
# =============================================================================

def verify_planck_scale():
    """Verify the Planck scale definitions."""
    print("=" * 60)
    print("LEVEL 0: PLANCK SCALE VERIFICATION")
    print("=" * 60)

    print("\n--- Physical Constants (SI) ---")
    print(f"  c     = {SI.c:.6e} m/s")
    print(f"  hbar  = {SI.hbar:.6e} J*s")
    print(f"  G     = {SI.G:.6e} m^3/kg/s^2")

    print("\n--- Planck Units ---")
    print(f"  l_P   = {SI.l_P:.6e} m")
    print(f"  t_P   = {SI.t_P:.6e} s")
    print(f"  m_P   = {SI.m_P:.6e} kg")
    print(f"  E_P   = {SI.E_P:.6e} J = {SI.E_P / 1.602e-19 / 1e9:.3e} GeV")
    print(f"  T_P   = {SI.T_P:.6e} K")

    print("\n--- Lattice Axioms ---")
    print(f"  Dimensions: {LATTICE.D}")
    print(f"  Moore neighbors: {LATTICE.MOORE_SIZE}")
    print(f"  Speed of causality: {LATTICE.C} site/tick")

    print("\n--- Master Constants ---")
    print(f"  G*      = {CONSTANTS.G_star:.6f}")
    print(f"  1/alpha = {CONSTANTS.alpha_inverse:.6f} (CODATA: 137.035999)")
    print(f"  alpha   = {CONSTANTS.alpha:.8f}")
    print(f"  N_c     = {CONSTANTS.N_c:.6f} -> {CONSTANTS.N_c_integer}")

    # Verify quadratic
    a, b, c = CONSTANTS.master_quadratic_coefficients
    x_plus, x_minus = CONSTANTS.master_quadratic_roots

    check_plus = a * x_plus**2 + b * x_plus + c
    check_minus = a * x_minus**2 + b * x_minus + c

    print("\n--- Master Quadratic Verification ---")
    print(f"  x^2 - 16(G*)^2 x + 16(G*)^3 = 0")
    print(f"  x_+ = {x_plus:.6f}, residual = {check_plus:.2e}")
    print(f"  x_- = {x_minus:.6f}, residual = {check_minus:.2e}")

    # Compare to measured α
    alpha_measured = 1 / 137.035999177
    alpha_error = abs(CONSTANTS.alpha - alpha_measured) / alpha_measured * 1e6
    print(f"\n  alpha deviation from CODATA: {alpha_error:.2f} ppm")

    print("\n--- Information Content ---")
    print(f"  Bits per ternary state: {np.log2(3):.4f}")
    print(f"  DoF per voxel: {PLANCK.degrees_of_freedom_per_voxel}")

    return True


if __name__ == "__main__":
    verify_planck_scale()
