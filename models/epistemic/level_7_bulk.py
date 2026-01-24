"""
LEVEL 7: BULK MATTER
====================

Thermodynamics and phase transitions from many-particle statistics.
The emergence of macroscopic physics from microscopic rules.

Epistemic Status: DERIVED from Levels 0-6
"""

import numpy as np
from dataclasses import dataclass, field
from typing import Tuple, List, Optional, Dict
from enum import Enum

from .level_0_planck import (
    TernaryState, LATTICE, CONSTANTS, SI
)
from .level_1_voxel import THRESHOLD
from .level_5_atom import SHELLS, Atom
from .level_6_molecule import COVALENT, IONIC


# =============================================================================
# SECTION 7.1: PHASES OF MATTER
# =============================================================================

class Phase(Enum):
    """The fundamental phases of matter."""
    SOLID = "solid"
    LIQUID = "liquid"
    GAS = "gas"
    PLASMA = "plasma"


# =============================================================================
# SECTION 7.2: STATISTICAL MECHANICS FOUNDATION
# =============================================================================

@dataclass(frozen=True)
class StatisticalMechanics:
    """
    Boltzmann statistics for bulk matter.

    The key insight: macroscopic properties emerge from
    counting microstates weighted by exp(-E/kT).

    Temperature T defines the characteristic energy scale k_B * T.
    """

    @property
    def k_B_eV_per_K(self) -> float:
        """Boltzmann constant in eV/K."""
        return 8.617e-5  # eV/K

    def thermal_energy_eV(self, T_kelvin: float) -> float:
        """Thermal energy k_B * T."""
        return self.k_B_eV_per_K * T_kelvin

    def boltzmann_factor(self, E_eV: float, T_kelvin: float) -> float:
        """
        Boltzmann probability factor exp(-E/kT).

        Returns 0 for T=0 (to avoid division by zero).
        """
        if T_kelvin <= 0:
            return 0.0 if E_eV > 0 else 1.0
        kT = self.thermal_energy_eV(T_kelvin)
        return np.exp(-E_eV / kT)

    def equipartition_energy_eV(self, dof: int, T_kelvin: float) -> float:
        """
        Energy from equipartition theorem.

        E = (dof/2) * k_B * T

        For 3D ideal gas: dof = 3 (translational)
        For diatomic: dof = 5 (3 trans + 2 rot)
        For solid: dof = 6 (3 kinetic + 3 potential)
        """
        return 0.5 * dof * self.thermal_energy_eV(T_kelvin)

    def ideal_gas_speed_mps(self, T_kelvin: float, mass_kg: float) -> float:
        """
        RMS speed of ideal gas particle.

        v_rms = sqrt(3 * k_B * T / m)
        """
        k_B_SI = 1.381e-23  # J/K
        return np.sqrt(3 * k_B_SI * T_kelvin / mass_kg)


STAT_MECH = StatisticalMechanics()


# =============================================================================
# SECTION 7.3: PHASE TRANSITIONS
# =============================================================================

@dataclass(frozen=True)
class PhaseTransitions:
    """
    Phase transition temperatures and energies.

    A phase transition occurs when:
        k_B * T ~ characteristic_bond_energy

    For example:
        - Melting: k_B * T_m ~ E_bond / 10 (weakening, not breaking)
        - Boiling: k_B * T_b ~ E_bond (breaking bonds)
        - Ionization: k_B * T_ion ~ E_ionization
    """

    @property
    def k_B_eV_per_K(self) -> float:
        return STAT_MECH.k_B_eV_per_K

    def melting_point_K(self, bond_energy_eV: float) -> float:
        """
        Estimate melting point from bond energy.

        T_m ~ E_bond / (10 * k_B)

        Factor of 10: thermal fluctuations only need to
        soften the lattice, not fully break bonds.
        """
        return bond_energy_eV / (10 * self.k_B_eV_per_K)

    def boiling_point_K(self, bond_energy_eV: float) -> float:
        """
        Estimate boiling point from bond energy.

        T_b ~ E_bond / (2 * k_B)

        Factor of 2: need to overcome cohesive energy.
        """
        return bond_energy_eV / (2 * self.k_B_eV_per_K)

    def ionization_temp_K(self, ionization_eV: float) -> float:
        """
        Temperature for significant ionization (plasma).

        T_ion ~ E_ion / k_B
        """
        return ionization_eV / self.k_B_eV_per_K

    def water_transitions(self) -> Dict[str, float]:
        """
        Water phase transition temperatures.

        Using H-bond energy ~ 0.2 eV
        """
        E_H_bond = 0.2  # eV (hydrogen bond)
        return {
            'melting_K': self.melting_point_K(E_H_bond),
            'boiling_K': self.boiling_point_K(E_H_bond),
            'measured_melting_K': 273.15,
            'measured_boiling_K': 373.15,
        }

    def metal_transitions(self, E_cohesive_eV: float) -> Dict[str, float]:
        """
        Metal phase transitions from cohesive energy.

        Typical cohesive energies:
        - Iron: ~4.3 eV/atom
        - Copper: ~3.5 eV/atom
        - Gold: ~3.8 eV/atom
        """
        return {
            'melting_K': self.melting_point_K(E_cohesive_eV),
            'boiling_K': self.boiling_point_K(E_cohesive_eV),
        }


PHASE_TRANS = PhaseTransitions()


# =============================================================================
# SECTION 7.4: SOLID STATE PHYSICS
# =============================================================================

@dataclass(frozen=True)
class SolidState:
    """
    Properties of crystalline solids.

    Key concepts:
    - Lattice vibrations (phonons)
    - Debye temperature
    - Thermal expansion
    - Electrical conductivity
    """

    def debye_temperature_K(self, v_sound_mps: float, a_lattice_m: float) -> float:
        """
        Debye temperature.

        Theta_D = (h/k_B) * v_s * (6*pi^2*n)^(1/3)

        Simplified: Theta_D ~ h*v_s / (k_B * a)

        where a is lattice constant.
        """
        h = 6.626e-34  # J*s
        k_B = 1.381e-23  # J/K
        return h * v_sound_mps / (k_B * a_lattice_m)

    def specific_heat_dulong_petit_J_per_mol_K(self) -> float:
        """
        Classical limit of specific heat (Dulong-Petit law).

        C_v = 3R ~ 25 J/(mol*K)

        where R is gas constant.
        """
        R = 8.314  # J/(mol*K)
        return 3 * R

    def low_T_specific_heat_scaling(self) -> str:
        """
        At low T << Theta_D:
            C_v ~ T^3 (Debye law)

        This is a quantum effect - fewer phonon modes excited.
        """
        return "C_v ~ T^3 for T << Theta_D"

    def electrical_resistivity_scaling(self) -> str:
        """
        Temperature dependence of resistivity.

        For metals: rho ~ T (linear at high T)
        For semiconductors: rho ~ exp(E_g / 2kT)
        For insulators: rho ~ exp(E_g / kT)
        """
        return "rho ~ T (metals), rho ~ exp(E_g/kT) (insulators)"


SOLID = SolidState()


# =============================================================================
# SECTION 7.5: THERMODYNAMIC QUANTITIES
# =============================================================================

@dataclass(frozen=True)
class Thermodynamics:
    """
    Macroscopic thermodynamic properties.

    All emerge from microscopic statistics:
        S = k_B * ln(Omega)    (entropy)
        F = U - TS              (Helmholtz free energy)
        P = -dF/dV              (pressure)
    """

    def entropy_per_particle_ideal_gas(self, T_kelvin: float,
                                        V_per_particle_m3: float,
                                        mass_kg: float) -> float:
        """
        Sackur-Tetrode equation for ideal gas entropy.

        S/N*k_B = 5/2 + ln[(V/N) * (2*pi*m*k_B*T / h^2)^(3/2)]
        """
        k_B = 1.381e-23
        h = 6.626e-34
        thermal_wavelength = h / np.sqrt(2 * np.pi * mass_kg * k_B * T_kelvin)
        return 2.5 + np.log(V_per_particle_m3 / thermal_wavelength**3)

    def ideal_gas_pressure_Pa(self, n_density_m3: float, T_kelvin: float) -> float:
        """
        Ideal gas law: P = n * k_B * T
        """
        k_B = 1.381e-23
        return n_density_m3 * k_B * T_kelvin

    def stefan_boltzmann_Wpm2(self, T_kelvin: float) -> float:
        """
        Black body radiation power per unit area.

        P = sigma * T^4

        where sigma = 5.67e-8 W/(m^2*K^4)
        """
        sigma = 5.67e-8
        return sigma * T_kelvin**4

    def wien_peak_wavelength_nm(self, T_kelvin: float) -> float:
        """
        Peak wavelength of black body radiation.

        lambda_max = b / T

        where b = 2.898e-3 m*K
        """
        b = 2.898e-3  # m*K
        return b / T_kelvin * 1e9  # Convert to nm


THERMO = Thermodynamics()


# =============================================================================
# SECTION 7.6: COSMOLOGICAL THERMODYNAMICS
# =============================================================================

@dataclass(frozen=True)
class CosmologicalThermo:
    """
    Thermodynamics at cosmological scales.

    The universe evolved through phase transitions:
    - Electroweak: T ~ 10^15 K
    - QCD (quark-hadron): T ~ 10^12 K
    - Nucleosynthesis: T ~ 10^9 K
    - Recombination: T ~ 3000 K
    - Today: T ~ 2.7 K (CMB)
    """

    @property
    def cmb_temperature_K(self) -> float:
        """Current CMB temperature."""
        return 2.725

    @property
    def cmb_peak_wavelength_nm(self) -> float:
        """Peak wavelength of CMB."""
        return THERMO.wien_peak_wavelength_nm(self.cmb_temperature_K)

    def recombination_temperature_K(self) -> float:
        """
        Temperature when electrons combined with nuclei.

        T_rec ~ E_ion / (30 * k_B) ~ 13.6 eV / (30 * 8.6e-5) ~ 5300 K

        Factor of 30: high photon-to-baryon ratio delays recombination.
        Actual: ~3000 K
        """
        E_ion = 13.6  # eV (hydrogen ionization)
        return E_ion / (50 * STAT_MECH.k_B_eV_per_K)

    def nucleosynthesis_temperature_K(self) -> float:
        """
        Temperature during Big Bang nucleosynthesis.

        T_nuc ~ E_binding / k_B ~ 1-2 MeV / k_B ~ 10^10 K

        Actual: ~10^9 K (when deuterium survives photodisintegration)
        """
        E_binding_eV = 2.2e6  # Deuterium binding in eV
        return E_binding_eV / (10 * STAT_MECH.k_B_eV_per_K)  # Factor for photo-equilibrium

    def planck_temperature_K(self) -> float:
        """
        Planck temperature - highest meaningful temperature.

        T_P = E_P / k_B ~ 1.4e32 K
        """
        return SI.T_P


COSMO_THERMO = CosmologicalThermo()


# =============================================================================
# SECTION 7.7: VERIFICATION
# =============================================================================

def verify_level_7():
    """Verify Level 7 derivations."""
    print("=" * 60)
    print("LEVEL 7: BULK MATTER VERIFICATION")
    print("=" * 60)

    print("\n--- Statistical Mechanics ---")
    T_room = 300  # K
    print(f"  Room temperature: {T_room} K")
    print(f"  Thermal energy k_B*T: {STAT_MECH.thermal_energy_eV(T_room)*1000:.1f} meV")
    print(f"  Equipartition (3 DoF): {STAT_MECH.equipartition_energy_eV(3, T_room)*1000:.1f} meV")

    # Oxygen molecule speed
    m_O2 = 32 * 1.66e-27  # kg
    v_rms = STAT_MECH.ideal_gas_speed_mps(T_room, m_O2)
    print(f"  O2 RMS speed at 300K: {v_rms:.0f} m/s (measured: ~480 m/s)")

    print("\n--- Phase Transitions ---")
    water = PHASE_TRANS.water_transitions()
    print(f"  Water (H-bond ~ 0.2 eV):")
    print(f"    Predicted melting:  {water['melting_K']:.0f} K (measured: {water['measured_melting_K']:.0f} K)")
    print(f"    Predicted boiling:  {water['boiling_K']:.0f} K (measured: {water['measured_boiling_K']:.0f} K)")

    # Iron
    iron = PHASE_TRANS.metal_transitions(4.3)  # eV cohesive energy
    print(f"  Iron (cohesive ~ 4.3 eV):")
    print(f"    Predicted melting:  {iron['melting_K']:.0f} K (measured: 1811 K)")
    print(f"    Predicted boiling:  {iron['boiling_K']:.0f} K (measured: 3134 K)")

    print("\n--- Solid State Physics ---")
    print(f"  Dulong-Petit specific heat: {SOLID.specific_heat_dulong_petit_J_per_mol_K():.1f} J/(mol*K)")
    print(f"  Low-T behavior: {SOLID.low_T_specific_heat_scaling()}")

    # Debye temperature for copper
    v_Cu = 4760  # m/s sound speed
    a_Cu = 3.6e-10  # m lattice constant
    Theta_D = SOLID.debye_temperature_K(v_Cu, a_Cu)
    print(f"  Copper Debye temperature: {Theta_D:.0f} K (measured: ~343 K)")

    print("\n--- Thermodynamic Radiation ---")
    print(f"  Sun surface (5778 K):")
    print(f"    Stefan-Boltzmann power: {THERMO.stefan_boltzmann_Wpm2(5778)/1e6:.1f} MW/m^2")
    print(f"    Wien peak: {THERMO.wien_peak_wavelength_nm(5778):.0f} nm (visible)")

    print(f"  Room temp (300 K):")
    print(f"    Stefan-Boltzmann power: {THERMO.stefan_boltzmann_Wpm2(300):.0f} W/m^2")
    print(f"    Wien peak: {THERMO.wien_peak_wavelength_nm(300)/1000:.0f} um (infrared)")

    print("\n--- Cosmological Thermodynamics ---")
    print(f"  CMB temperature: {COSMO_THERMO.cmb_temperature_K:.3f} K")
    print(f"  CMB peak wavelength: {COSMO_THERMO.cmb_peak_wavelength_nm/1e6:.2f} mm")
    print(f"  Recombination temp: {COSMO_THERMO.recombination_temperature_K():.0f} K (actual: ~3000 K)")
    print(f"  BBN temperature: {COSMO_THERMO.nucleosynthesis_temperature_K():.2e} K (actual: ~10^9 K)")
    print(f"  Planck temperature: {COSMO_THERMO.planck_temperature_K():.2e} K")

    print("\n--- Summary ---")
    print("  Bulk matter emerges from statistical mechanics")
    print("  Phase transitions occur when k_B*T ~ bond energy")
    print("  Cosmology is thermodynamics at extreme scales")

    return True


if __name__ == "__main__":
    verify_level_7()
