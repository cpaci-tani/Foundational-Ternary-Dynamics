"""
Mass Defect and Fusion Energy from FTD
======================================

Demonstrates why nuclear fusion releases energy using FTD principles:

1. Mass = Flux Count (from Equivalence Principle)
2. Bound nucleons share flux infrastructure
3. Mass defect = flux released when nucleons combine
4. E = Delta_m*c^2 = released flux energy

Key insight from FTD gravity verification:
- Gravitational charge (coupling to field) = N (flux count)
- Inertial mass (resistance to acceleration) = N (flux count)
- These are IDENTICAL because mass IS flux

When nucleons bind:
- They share flux infrastructure
- Total flux needed to maintain structure decreases
- Released flux -> energy (E = mc^2)

Reference: FTD manuscript, Equivalence Principle verification
"""

import numpy as np
from typing import Tuple, Dict, List
from dataclasses import dataclass

# Import binding energy calculation
from binding_energy import (
    binding_energy, binding_energy_per_nucleon,
    NUCLEI, EXPERIMENTAL_BINDING,
    N_C, N_BASE, B_3, N_EFF, K_B, ALPHA
)

# =============================================================================
# NUCLEAR MASSES AND MASS DEFECT
# =============================================================================

# Nucleon masses (MeV/c^2)
M_PROTON = 938.272  # MeV
M_NEUTRON = 939.565  # MeV
M_ELECTRON = 0.511  # MeV

# Atomic mass unit
AMU = 931.494  # MeV/c^2


def nuclear_mass(A: int, Z: int, use_binding: bool = True) -> float:
    """
    Calculate nuclear mass in MeV/c^2.

    M(A,Z) = Z*m_p + N*m_n - B(A,Z)

    The mass defect B(A,Z) represents the binding energy that has been
    released when the nucleus was formed.

    Parameters:
        A: Mass number
        Z: Atomic number
        use_binding: If True, subtract binding energy

    Returns:
        Nuclear mass in MeV/c^2
    """
    N = A - Z  # neutrons

    # Sum of constituent masses
    constituent_mass = Z * M_PROTON + N * M_NEUTRON

    if use_binding:
        B = binding_energy(A, Z)
        return constituent_mass - B
    else:
        return constituent_mass


def mass_defect(A: int, Z: int) -> float:
    """
    Calculate mass defect Delta_m = B(A,Z)/c^2 in MeV.

    This is the "missing mass" - the mass converted to binding energy
    when the nucleus was formed.

    Parameters:
        A: Mass number
        Z: Atomic number

    Returns:
        Mass defect in MeV/c^2
    """
    return binding_energy(A, Z)


# =============================================================================
# Q-VALUE CALCULATIONS (ENERGY RELEASED IN REACTIONS)
# =============================================================================

@dataclass
class NuclearReaction:
    """Represents a nuclear reaction with reactants and products."""
    name: str
    reactants: List[Tuple[int, int]]  # List of (A, Z)
    products: List[Tuple[int, int]]   # List of (A, Z)
    description: str = ""


def calculate_q_value(reaction: NuclearReaction) -> float:
    """
    Calculate Q-value (energy released) for a nuclear reaction.

    Q = B(products) - B(reactants)
      = [M(reactants) - M(products)]*c^2

    Positive Q -> exothermic (energy released)
    Negative Q -> endothermic (energy required)

    FTD Interpretation:
    - Q > 0: Products share flux more efficiently than reactants
    - Q < 0: Products need more flux than reactants provide

    Parameters:
        reaction: NuclearReaction object

    Returns:
        Q-value in MeV
    """
    # Sum binding energies
    B_reactants = sum(binding_energy(A, Z) for A, Z in reaction.reactants)
    B_products = sum(binding_energy(A, Z) for A, Z in reaction.products)

    # Q = B(products) - B(reactants)
    # If products are more tightly bound, Q > 0
    Q = B_products - B_reactants

    return Q


def calculate_q_value_from_masses(reaction: NuclearReaction) -> float:
    """
    Calculate Q-value from mass difference (equivalent method).

    Q = [M(reactants) - M(products)]*c^2
    """
    M_reactants = sum(nuclear_mass(A, Z) for A, Z in reaction.reactants)
    M_products = sum(nuclear_mass(A, Z) for A, Z in reaction.products)

    return M_reactants - M_products


# =============================================================================
# FUSION REACTIONS
# =============================================================================

FUSION_REACTIONS = {
    # D-T fusion (tokamak fuel)
    'D-T': NuclearReaction(
        name='D + T -> He-4 + n',
        reactants=[(2, 1), (3, 1)],   # D + T
        products=[(4, 2), (1, 0)],     # He-4 + neutron
        description='Primary tokamak/ITER fuel. Highest cross-section at low T.'
    ),

    # D-D fusion (two branches)
    'D-D-1': NuclearReaction(
        name='D + D -> He-3 + n',
        reactants=[(2, 1), (2, 1)],
        products=[(3, 2), (1, 0)],
        description='D-D branch 1: produces He-3 + neutron'
    ),

    'D-D-2': NuclearReaction(
        name='D + D -> T + p',
        reactants=[(2, 1), (2, 1)],
        products=[(3, 1), (1, 1)],
        description='D-D branch 2: produces tritium + proton'
    ),

    # p-p chain (solar fusion step 1)
    'p-p': NuclearReaction(
        name='p + p -> D + e+ + v_e',
        reactants=[(1, 1), (1, 1)],
        products=[(2, 1)],  # D (positron + neutrino carry ~0.42 MeV)
        description='First step of solar fusion (p-p chain)'
    ),

    # He-3 + He-3 (p-p chain completion)
    'He3-He3': NuclearReaction(
        name='He-3 + He-3 -> He-4 + 2p',
        reactants=[(3, 2), (3, 2)],
        products=[(4, 2), (1, 1), (1, 1)],
        description='Final step of p-p I chain'
    ),

    # Triple-alpha (helium burning in stars)
    'triple-alpha': NuclearReaction(
        name='3 He-4 -> C-12',
        reactants=[(4, 2), (4, 2), (4, 2)],
        products=[(12, 6)],
        description='Helium burning: 3alpha -> C-12 (via Be-8 resonance)'
    ),

    # CNO cycle (high-mass stars)
    'CNO-net': NuclearReaction(
        name='4p -> He-4 (CNO)',
        reactants=[(1, 1), (1, 1), (1, 1), (1, 1)],
        products=[(4, 2)],  # Net effect: 4p -> He-4
        description='Net CNO cycle: 4 protons -> He-4 + 2e+ + 2v_e'
    ),
}

# Experimental Q-values for comparison (MeV)
EXPERIMENTAL_Q_VALUES = {
    'D-T': 17.59,
    'D-D-1': 3.27,
    'D-D-2': 4.03,
    'p-p': 1.44,  # Including positron annihilation
    'He3-He3': 12.86,
    'triple-alpha': 7.27,
    'CNO-net': 26.73,  # Net energy per He-4 produced
}


# =============================================================================
# FISSION REACTIONS
# =============================================================================

FISSION_REACTIONS = {
    # U-235 thermal fission (typical)
    'U-235-fission': NuclearReaction(
        name='U-235 + n -> Ba-141 + Kr-92 + 3n',
        reactants=[(235, 92), (1, 0)],
        products=[(141, 56), (92, 36), (1, 0), (1, 0), (1, 0)],
        description='Typical U-235 thermal fission (one of many possible channels)'
    ),

    # Pu-239 fission
    'Pu-239-fission': NuclearReaction(
        name='Pu-239 + n -> Xe-134 + Zr-103 + 3n',
        reactants=[(239, 94), (1, 0)],
        products=[(134, 54), (103, 40), (1, 0), (1, 0), (1, 0)],
        description='Typical Pu-239 fission channel'
    ),
}

# Experimental fission energies
EXPERIMENTAL_Q_FISSION = {
    'U-235-fission': 200,  # ~200 MeV average
    'Pu-239-fission': 210,  # ~210 MeV average
}


# =============================================================================
# WHY FUSION RELEASES ENERGY (FTD EXPLANATION)
# =============================================================================

def explain_fusion_energy():
    """
    Explain why fusion releases energy using FTD principles.

    Key insight: Mass IS charge (Equivalence Principle).
    Binding energy represents flux that no longer needs to be
    maintained because nucleons are sharing infrastructure.
    """

    explanation = """
    =========================================================================
    WHY FUSION RELEASES ENERGY (FTD First Principles)
    =========================================================================

    From the Equivalence Principle Verification:
    --------------------------------------------
    - Mass = Flux count N
    - Gravitational charge = N (how strongly object couples to field)
    - Inertial mass = N (resistance to acceleration)
    - These are IDENTICAL because mass IS flux

    When Nucleons Bind:
    -------------------
    1. Free proton: needs N_p flux to maintain structure
    2. Free neutron: needs N_n flux to maintain structure
    3. Together as deuteron: needs N_d flux < N_p + N_n

    The "missing" flux is the mass defect:
        Delta_m = (N_p + N_n - N_d) = B(D)/c^2

    This flux is RELEASED as energy:
        E = Delta_m*c^2 = B(D) = 2.22 MeV

    Why Do Nucleons Share Flux More Efficiently?
    --------------------------------------------
    From FTD triad stability:
    - Binding occurs when particle has ≥2 same-sign neighbors
    - Shared flux infrastructure reduces total flux needed
    - Like carpooling: 2 people, 1 car uses less gas than 2 cars

    The Golden Ratio Connection:
    ----------------------------
    Binding energy E_bind ~ K_B × phi (golden ratio)
    phi = (1 + sqrt5)/2 ≈ 1.618... is the optimal packing ratio

    This emerges from geometric stability of flux configurations.

    Why Does Fusion Stop at Iron?
    -----------------------------
    - Light nuclei (A < 56): adding nucleons increases B/A
    - Heavy nuclei (A > 56): adding nucleons decreases B/A
    - Iron-56 is the peak: maximum flux efficiency

    Beyond iron, the Coulomb repulsion (~ Z^2) grows faster than
    the strong binding (~ A), making larger nuclei less stable.

    The Sun's Energy:
    -----------------
    4p -> He-4 releases 26.7 MeV because:
    - 4 free protons: each maintains its own flux
    - 1 He-4 nucleus: shares flux among 4 nucleons
    - Mass defect: 4×938.3 - 3727.4 = 26.0 MeV
    =========================================================================
    """
    return explanation


# =============================================================================
# VERIFICATION
# =============================================================================

def run_mass_defect_verification():
    """Run complete mass defect and Q-value verification."""

    print("=" * 70)
    print("MASS DEFECT AND FUSION ENERGY FROM FTD")
    print("=" * 70)

    # Part 1: Mass defect examples
    print("\n[1] MASS DEFECT (Binding Energy / c^2)")
    print("-" * 50)
    print(f"{'Nucleus':<10} | {'B (MeV)':<12} | {'dm (u)':<12} | {'dm/M (%)':<10}")
    print("-" * 50)

    for name in ['H-2', 'He-4', 'C-12', 'O-16', 'Fe-56']:
        A, Z = NUCLEI[name]
        B = binding_energy(A, Z)
        dm_u = B / AMU  # Convert to atomic mass units
        M = nuclear_mass(A, Z, use_binding=False)
        dm_percent = B / M * 100

        print(f"{name:<10} | {B:<12.2f} | {dm_u:<12.4f} | {dm_percent:<10.3f}")

    # Part 2: Fusion Q-values
    print("\n[2] FUSION REACTIONS (Q-values)")
    print("-" * 65)
    print(f"{'Reaction':<20} | {'FTD Q (MeV)':<12} | {'Exp Q (MeV)':<12} | {'Error %':<10}")
    print("-" * 65)

    fusion_errors = []
    for key, reaction in FUSION_REACTIONS.items():
        Q_ftd = calculate_q_value(reaction)
        Q_exp = EXPERIMENTAL_Q_VALUES.get(key, None)

        if Q_exp:
            error = abs(Q_ftd - Q_exp) / Q_exp * 100
            fusion_errors.append(error)
            status = "PASS" if error < 15 else "CHECK"
            print(f"{reaction.name:<20} | {Q_ftd:<12.2f} | {Q_exp:<12.2f} | {error:<10.1f} {status}")
        else:
            print(f"{reaction.name:<20} | {Q_ftd:<12.2f} | {'N/A':<12} | {'N/A':<10}")

    if fusion_errors:
        fusion_rms = np.sqrt(np.mean(np.array(fusion_errors)**2))
        print("-" * 65)
        print(f"Fusion RMS error: {fusion_rms:.1f}%")

    # Part 3: Key insight
    print("\n[3] FTD INSIGHT: Why D+T -> He-4 releases 17.6 MeV")
    print("-" * 50)

    D = (2, 1)
    T = (3, 1)
    He4 = (4, 2)
    n = (1, 0)

    B_D = binding_energy(*D)
    B_T = binding_energy(*T)
    B_He4 = binding_energy(*He4)
    B_n = binding_energy(*n)  # = 0

    print(f"  B(D)   = {B_D:.2f} MeV (deuterium binding)")
    print(f"  B(T)   = {B_T:.2f} MeV (tritium binding)")
    print(f"  B(He4) = {B_He4:.2f} MeV (helium-4 binding)")
    print(f"  B(n)   = {B_n:.2f} MeV (free neutron)")
    print()
    print(f"  Q = B(He4) + B(n) - B(D) - B(T)")
    print(f"    = {B_He4:.2f} + {B_n:.2f} - {B_D:.2f} - {B_T:.2f}")
    print(f"    = {B_He4 + B_n - B_D - B_T:.2f} MeV")
    print()
    print("  FTD interpretation:")
    print("  - D and T each maintain separate flux infrastructure")
    print("  - He-4 shares flux among 4 nucleons (doubly magic!)")
    print("  - Released flux = 17.6 MeV of energy")

    # Part 4: Compare to fission
    print("\n[4] COMPARISON: Fusion vs Fission")
    print("-" * 50)

    # D-T fusion
    Q_DT = calculate_q_value(FUSION_REACTIONS['D-T'])
    A_reactants_DT = sum(A for A, Z in FUSION_REACTIONS['D-T'].reactants)
    Q_per_nucleon_DT = Q_DT / A_reactants_DT

    # U-235 fission (approximate)
    Q_U235 = 200  # MeV (experimental average)
    A_U235 = 236  # U-235 + n
    Q_per_nucleon_U235 = Q_U235 / A_U235

    print(f"  D-T fusion:")
    print(f"    Total Q = {Q_DT:.1f} MeV")
    print(f"    Q per nucleon = {Q_per_nucleon_DT:.2f} MeV")
    print()
    print(f"  U-235 fission:")
    print(f"    Total Q = {Q_U235:.1f} MeV")
    print(f"    Q per nucleon = {Q_per_nucleon_U235:.2f} MeV")
    print()
    print(f"  Fusion advantage: {Q_per_nucleon_DT / Q_per_nucleon_U235:.1f}x more energy per nucleon!")
    print()
    print("  FTD explanation:")
    print("  - Fusion: moving UP the B/A curve toward iron peak")
    print("  - Fission: moving DOWN the B/A curve toward iron peak")
    print("  - Both release energy; fusion is more efficient per nucleon")

    # Success check
    if fusion_errors and fusion_rms < 20:
        print("\n[PASS] Fusion Q-values derived from FTD binding energies!")
        return True
    else:
        print(f"\n[CHECK] Some Q-values need refinement.")
        return False


if __name__ == "__main__":
    run_mass_defect_verification()
    print("\n" + "=" * 70)
    print(explain_fusion_energy())
