#!/usr/bin/env python3
"""
Magnetic Flux Coupling Simulation

Models how magnetic systems might interact with vacuum flux fields
within the FTD framework.

Explores:
- Permanent magnet flux patterns
- Rotating magnetic systems
- Asymmetric flux coupling
- Vacuum energy extraction via magnetic gradients

This is speculative exploration, not verified physics.
"""

import numpy as np
import matplotlib.pyplot as plt
from dataclasses import dataclass, field
from typing import List, Tuple, Dict, Optional
import json
from pathlib import Path

# =============================================================================
# Constants
# =============================================================================

C = 299_792_458  # Speed of light (m/s)
MU_0 = 4 * np.pi * 1e-7  # Vacuum permeability (H/m)
EPSILON_0 = 8.854187817e-12  # Vacuum permittivity (F/m)

# FTD Constants
N_C = 3
N_BASE = 4
B_3 = 7
N_EFF = 13
ALPHA = 1 / 137.036  # Fine structure constant
KB = 0.511  # Manifestation threshold


# =============================================================================
# Magnetic Field Models
# =============================================================================

def dipole_field(position: np.ndarray, moment: np.ndarray,
                 dipole_position: np.ndarray = None) -> np.ndarray:
    """
    Calculate magnetic field of a dipole at a given position.

    B(r) = (μ₀/4π) [3(m·r̂)r̂ - m] / r³

    Args:
        position: Field point (3D vector or array of vectors)
        moment: Magnetic dipole moment vector
        dipole_position: Location of dipole (default: origin)

    Returns:
        Magnetic field vector(s)
    """
    if dipole_position is None:
        dipole_position = np.zeros(3)

    r = position - dipole_position
    r_mag = np.linalg.norm(r, axis=-1, keepdims=True)
    r_mag = np.maximum(r_mag, 1e-10)  # Avoid singularity

    r_hat = r / r_mag

    # Dipole field formula
    m_dot_rhat = np.sum(moment * r_hat, axis=-1, keepdims=True)
    B = (MU_0 / (4 * np.pi)) * (3 * m_dot_rhat * r_hat - moment) / (r_mag ** 3)

    return B


def create_field_grid(size: int = 32, extent: float = 1.0) -> Tuple[np.ndarray, ...]:
    """Create a 3D grid for field calculations."""
    x = np.linspace(-extent, extent, size)
    y = np.linspace(-extent, extent, size)
    z = np.linspace(-extent, extent, size)
    X, Y, Z = np.meshgrid(x, y, z, indexing='ij')
    return X, Y, Z


# =============================================================================
# Flux-Magnetic Coupling Model
# =============================================================================

@dataclass
class MagneticFluxSystem:
    """
    Models the interaction between magnetic fields and vacuum flux.

    FTD Hypothesis: Magnetism arises from the curl of the flux field.
    F_mag = β · (∇×J) × Ĵ

    A permanent magnet has a "frozen" flux curl pattern.
    """
    grid_size: int = 32
    extent: float = 1.0

    # Grids
    X: np.ndarray = field(init=False)
    Y: np.ndarray = field(init=False)
    Z: np.ndarray = field(init=False)
    positions: np.ndarray = field(init=False)

    # Fields
    B_field: np.ndarray = field(init=False)  # Magnetic field
    J_field: np.ndarray = field(init=False)  # Flux field
    curl_J: np.ndarray = field(init=False)   # Curl of flux

    # Energy
    energy_density: np.ndarray = field(init=False)

    def __post_init__(self):
        """Initialize grids and fields."""
        self.X, self.Y, self.Z = create_field_grid(self.grid_size, self.extent)

        # Stack into position array
        self.positions = np.stack([self.X, self.Y, self.Z], axis=-1)

        # Initialize fields to zero
        self.B_field = np.zeros((self.grid_size, self.grid_size, self.grid_size, 3))
        self.J_field = np.zeros_like(self.B_field)
        self.curl_J = np.zeros_like(self.B_field)
        self.energy_density = np.zeros((self.grid_size, self.grid_size, self.grid_size))

    def add_dipole(self, moment: np.ndarray, position: np.ndarray = None):
        """
        Add a magnetic dipole to the system.

        In FTD, this creates a curl pattern in the flux field.
        """
        if position is None:
            position = np.zeros(3)

        # Calculate dipole field
        flat_positions = self.positions.reshape(-1, 3)
        B_dipole = dipole_field(flat_positions, moment, position)
        B_dipole = B_dipole.reshape(self.grid_size, self.grid_size, self.grid_size, 3)

        self.B_field += B_dipole

        # FTD interpretation: B ∝ ∇×J
        # So J is the "vector potential" of the flux field
        # We approximate J from B (inverse curl is not unique, but we use it symbolically)
        self._update_flux_from_B()

    def _update_flux_from_B(self):
        """
        Update flux field to be consistent with magnetic field.

        Since B ~ ∇×J, we can estimate J such that its curl gives B.
        This is a simplified model using central differences.
        """
        # For a uniform B in z-direction, J circulates in xy-plane
        # We approximate: J_x ~ -∫B_y dz, J_y ~ ∫B_x dz
        # This is a rough approximation

        # Use cumulative sum as integration proxy
        dx = 2 * self.extent / (self.grid_size - 1)

        # J_x from B_z (via Stokes)
        self.J_field[..., 0] = np.cumsum(self.B_field[..., 2], axis=1) * dx
        # J_y from B_z
        self.J_field[..., 1] = -np.cumsum(self.B_field[..., 2], axis=0) * dx
        # J_z from B_x, B_y
        self.J_field[..., 2] = (
            np.cumsum(self.B_field[..., 1], axis=0) -
            np.cumsum(self.B_field[..., 0], axis=1)
        ) * dx

        # Compute curl of J (should approximately recover B)
        self._compute_curl()

        # Update energy density
        self.energy_density = np.sum(self.J_field ** 2, axis=-1)

    def _compute_curl(self):
        """Compute curl of flux field using finite differences."""
        dx = 2 * self.extent / (self.grid_size - 1)

        # ∇×J = (∂Jz/∂y - ∂Jy/∂z, ∂Jx/∂z - ∂Jz/∂x, ∂Jy/∂x - ∂Jx/∂y)
        self.curl_J[..., 0] = (
            np.gradient(self.J_field[..., 2], dx, axis=1) -
            np.gradient(self.J_field[..., 1], dx, axis=2)
        )
        self.curl_J[..., 1] = (
            np.gradient(self.J_field[..., 0], dx, axis=2) -
            np.gradient(self.J_field[..., 2], dx, axis=0)
        )
        self.curl_J[..., 2] = (
            np.gradient(self.J_field[..., 1], dx, axis=0) -
            np.gradient(self.J_field[..., 0], dx, axis=1)
        )

    def compute_total_energy(self) -> float:
        """Compute total energy in the flux field."""
        dx = 2 * self.extent / (self.grid_size - 1)
        dV = dx ** 3
        return float(np.sum(self.energy_density) * dV)

    def compute_flux_gradient(self) -> np.ndarray:
        """Compute gradient of flux magnitude."""
        flux_mag = np.sqrt(np.sum(self.J_field ** 2, axis=-1))
        dx = 2 * self.extent / (self.grid_size - 1)

        grad = np.stack([
            np.gradient(flux_mag, dx, axis=0),
            np.gradient(flux_mag, dx, axis=1),
            np.gradient(flux_mag, dx, axis=2)
        ], axis=-1)

        return grad


# =============================================================================
# Rotating Magnet System
# =============================================================================

@dataclass
class RotatingMagnetSystem:
    """
    Models a rotating magnet and its flux dynamics.

    The hypothesis: Rotating magnets create time-varying flux patterns
    that may couple to vacuum energy.
    """
    n_magnets: int = 4
    radius: float = 0.5
    moment_strength: float = 1.0
    angular_velocity: float = 1.0  # rad/s

    def get_magnet_positions(self, time: float) -> np.ndarray:
        """Get magnet positions at given time."""
        angles = np.linspace(0, 2*np.pi, self.n_magnets, endpoint=False)
        angles += self.angular_velocity * time

        positions = np.zeros((self.n_magnets, 3))
        positions[:, 0] = self.radius * np.cos(angles)
        positions[:, 1] = self.radius * np.sin(angles)

        return positions

    def get_magnet_moments(self, time: float) -> np.ndarray:
        """
        Get magnetic moments at given time.

        Moments point radially outward (or inward for alternating).
        """
        angles = np.linspace(0, 2*np.pi, self.n_magnets, endpoint=False)
        angles += self.angular_velocity * time

        moments = np.zeros((self.n_magnets, 3))

        for i in range(self.n_magnets):
            # Radial direction
            sign = 1 if i % 2 == 0 else -1  # Alternating polarity
            moments[i, 0] = sign * self.moment_strength * np.cos(angles[i])
            moments[i, 1] = sign * self.moment_strength * np.sin(angles[i])

        return moments

    def compute_field_at_time(self, system: MagneticFluxSystem,
                               time: float) -> MagneticFluxSystem:
        """
        Compute the total field at a given time.
        """
        # Reset fields
        system.B_field[:] = 0
        system.J_field[:] = 0

        positions = self.get_magnet_positions(time)
        moments = self.get_magnet_moments(time)

        for i in range(self.n_magnets):
            system.add_dipole(moments[i], positions[i])

        return system

    def simulate_rotation(self, n_steps: int = 100,
                          total_time: float = 2*np.pi) -> List[Dict]:
        """
        Simulate magnet rotation over time.

        Returns history of energy and other quantities.
        """
        system = MagneticFluxSystem(grid_size=24, extent=1.5)
        times = np.linspace(0, total_time, n_steps)
        history = []

        for t in times:
            self.compute_field_at_time(system, t)

            record = {
                'time': float(t),
                'total_energy': system.compute_total_energy(),
                'max_B': float(np.max(np.linalg.norm(system.B_field, axis=-1))),
                'max_J': float(np.max(np.linalg.norm(system.J_field, axis=-1))),
                'center_B': system.B_field[12, 12, 12].tolist(),
            }
            history.append(record)

        return history


# =============================================================================
# Asymmetric Flux Extraction
# =============================================================================

@dataclass
class AsymmetricExtractor:
    """
    Models an asymmetric magnetic configuration for flux extraction.

    The hypothesis: Asymmetric magnet arrangements can create
    one-way flux flow, enabling energy extraction from vacuum.
    """

    def create_asymmetric_config(self) -> Tuple[List[np.ndarray], List[np.ndarray]]:
        """
        Create an asymmetric magnet configuration.

        Uses different field strengths on entry vs exit.
        """
        # Strong magnets on input side
        input_positions = [
            np.array([-0.5, 0, 0]),
            np.array([-0.5, 0.2, 0]),
            np.array([-0.5, -0.2, 0]),
        ]
        input_moments = [
            np.array([1.0, 0, 0]),
            np.array([0.8, 0, 0]),
            np.array([0.8, 0, 0]),
        ]

        # Weak magnets on output side
        output_positions = [
            np.array([0.5, 0, 0]),
        ]
        output_moments = [
            np.array([-0.5, 0, 0]),  # Opposite polarity, weaker
        ]

        all_positions = input_positions + output_positions
        all_moments = input_moments + output_moments

        return all_positions, all_moments

    def analyze_asymmetry(self) -> Dict:
        """
        Analyze flux flow in asymmetric configuration.
        """
        system = MagneticFluxSystem(grid_size=32, extent=1.0)
        positions, moments = self.create_asymmetric_config()

        for pos, mom in zip(positions, moments):
            system.add_dipole(mom, pos)

        # Compute flux gradient
        grad = system.compute_flux_gradient()
        grad_mag = np.linalg.norm(grad, axis=-1)

        # Check for net flux flow direction
        # Average flux direction in center region
        center_slice = slice(12, 20)
        center_flux = system.J_field[center_slice, center_slice, center_slice]
        avg_flux_direction = np.mean(center_flux, axis=(0, 1, 2))
        avg_flux_direction = avg_flux_direction / (np.linalg.norm(avg_flux_direction) + 1e-10)

        # Left vs right energy density
        left_energy = np.mean(system.energy_density[:16, :, :])
        right_energy = np.mean(system.energy_density[16:, :, :])
        asymmetry_ratio = left_energy / (right_energy + 1e-10)

        return {
            'n_magnets': len(positions),
            'total_energy': system.compute_total_energy(),
            'avg_flux_direction': avg_flux_direction.tolist(),
            'left_energy_density': float(left_energy),
            'right_energy_density': float(right_energy),
            'asymmetry_ratio': float(asymmetry_ratio),
            'max_gradient': float(np.max(grad_mag)),
            'interpretation': (
                "Asymmetry ratio > 1 indicates higher flux density on input side. "
                f"Ratio = {asymmetry_ratio:.2f}. "
                "If vacuum flux flows from high to low density, this creates "
                "net flux flow through the device."
            )
        }


# =============================================================================
# Searl Effect Simulation (Speculative)
# =============================================================================

@dataclass
class SearlEffectSim:
    """
    Simulates the claimed Searl Effect Generator.

    The claim: Rotating magnets at specific speeds create
    levitation and anomalous energy output.

    FTD interpretation: At certain rotation frequencies,
    the time-varying flux may approach exclusion conditions.
    """
    n_rollers: int = 12
    inner_radius: float = 0.3
    outer_radius: float = 0.6
    moment_strength: float = 1.0

    def compute_critical_frequency(self) -> Dict:
        """
        Compute what frequency might cause flux exclusion.

        FTD says exclusion occurs at 8 THz. What mechanical
        frequency would couple to this?
        """
        f_exclusion = 8e12  # Hz

        # For a magnet at radius r rotating at frequency f,
        # the magnetic field at the center varies at frequency n*f
        # where n is related to magnet geometry

        # To couple to 8 THz, we need astronomical rotation rates
        # But harmonics might build up...

        # If we have 12 rollers, and each creates 2 field variations per rotation
        multiplier = self.n_rollers * 2  # 24

        # Direct coupling would need
        direct_freq = f_exclusion / multiplier  # Still ~3e11 Hz

        # But if there's harmonic cascade...
        # Start at 1000 Hz, need 10^9 amplification
        practical_freq = 1000  # Hz
        harmonics_needed = f_exclusion / (practical_freq * multiplier)

        return {
            'f_exclusion_Hz': f_exclusion,
            'n_rollers': self.n_rollers,
            'field_multiplier': multiplier,
            'direct_coupling_freq_Hz': direct_freq,
            'practical_base_freq_Hz': practical_freq,
            'harmonics_needed': float(harmonics_needed),
            'log10_harmonics': float(np.log10(harmonics_needed)),
            'interpretation': (
                "Direct mechanical coupling to 8 THz is impossible. "
                "However, if the Searl effect is real, it may work via: "
                "1) Harmonic generation in the magnetic material "
                "2) Resonance with vacuum flux modes "
                "3) Some unknown mechanism we don't understand "
                f"We need ~10^{np.log10(harmonics_needed):.0f} harmonic amplification."
            )
        }

    def simulate_energy_vs_frequency(self,
                                      freq_range: Tuple[float, float] = (1, 1000),
                                      n_points: int = 50) -> Dict:
        """
        Simulate how energy changes with rotation frequency.

        Looking for resonances or anomalies.
        """
        freqs = np.logspace(np.log10(freq_range[0]),
                            np.log10(freq_range[1]),
                            n_points)
        energies = []
        anomalies = []

        rotating_system = RotatingMagnetSystem(
            n_magnets=self.n_rollers,
            radius=self.outer_radius,
            moment_strength=self.moment_strength
        )

        for freq in freqs:
            rotating_system.angular_velocity = 2 * np.pi * freq

            # Simulate for short time
            history = rotating_system.simulate_rotation(n_steps=20,
                                                         total_time=1/freq)

            avg_energy = np.mean([h['total_energy'] for h in history])
            energy_variance = np.var([h['total_energy'] for h in history])

            energies.append(avg_energy)

            # Check for anomalies (high variance could indicate resonance)
            if energy_variance > 0.01 * avg_energy ** 2:
                anomalies.append({
                    'frequency': float(freq),
                    'energy': float(avg_energy),
                    'variance': float(energy_variance)
                })

        return {
            'frequencies_Hz': freqs.tolist(),
            'average_energies': energies,
            'anomalies': anomalies,
            'n_anomalies': len(anomalies)
        }


# =============================================================================
# Main Analysis
# =============================================================================

def run_all_simulations():
    """Run all magnetic flux simulations."""

    output_dir = Path(__file__).parent.parent / "output"
    output_dir.mkdir(exist_ok=True)

    results = {}

    print("=" * 70)
    print("MAGNETIC FLUX COUPLING SIMULATIONS")
    print("FTD Framework - Speculative Exploration")
    print("=" * 70)

    # 1. Basic magnetic flux system
    print("\n--- BASIC MAGNETIC FLUX SYSTEM ---")
    system = MagneticFluxSystem(grid_size=32, extent=1.0)

    # Add a dipole
    moment = np.array([0, 0, 1.0])  # Z-directed
    system.add_dipole(moment)

    total_energy = system.compute_total_energy()
    max_B = np.max(np.linalg.norm(system.B_field, axis=-1))
    max_J = np.max(np.linalg.norm(system.J_field, axis=-1))

    print(f"Single dipole (moment = {moment}):")
    print(f"  Total flux energy: {total_energy:.4f}")
    print(f"  Max |B|: {max_B:.4f}")
    print(f"  Max |J|: {max_J:.4f}")

    results['single_dipole'] = {
        'moment': moment.tolist(),
        'total_energy': total_energy,
        'max_B': float(max_B),
        'max_J': float(max_J)
    }

    # 2. Rotating magnet system
    print("\n--- ROTATING MAGNET SYSTEM ---")
    rotating = RotatingMagnetSystem(n_magnets=4, radius=0.5,
                                     angular_velocity=2*np.pi)

    history = rotating.simulate_rotation(n_steps=50, total_time=2*np.pi)
    results['rotating_magnets'] = {
        'n_magnets': rotating.n_magnets,
        'radius': rotating.radius,
        'history': history
    }

    print(f"4 rotating magnets:")
    print(f"  Initial energy: {history[0]['total_energy']:.4f}")
    print(f"  Final energy: {history[-1]['total_energy']:.4f}")
    energy_variation = np.std([h['total_energy'] for h in history])
    print(f"  Energy variation: {energy_variation:.4f}")

    # 3. Asymmetric extractor
    print("\n--- ASYMMETRIC FLUX EXTRACTOR ---")
    asymmetric = AsymmetricExtractor()
    asym_analysis = asymmetric.analyze_asymmetry()
    results['asymmetric'] = asym_analysis

    print(f"Asymmetric configuration:")
    print(f"  Total energy: {asym_analysis['total_energy']:.4f}")
    print(f"  Asymmetry ratio: {asym_analysis['asymmetry_ratio']:.2f}")
    print(f"  Avg flux direction: {asym_analysis['avg_flux_direction']}")

    # 4. Searl effect analysis
    print("\n--- SEARL EFFECT ANALYSIS ---")
    searl = SearlEffectSim(n_rollers=12)
    critical_freq = searl.compute_critical_frequency()
    results['searl_critical'] = critical_freq

    print(f"Searl-type configuration (12 rollers):")
    print(f"  Field multiplier: {critical_freq['field_multiplier']}×")
    print(f"  Harmonics needed for 8 THz: 10^{critical_freq['log10_harmonics']:.1f}")

    # Frequency sweep (limited range)
    freq_sweep = searl.simulate_energy_vs_frequency((1, 100), n_points=20)
    results['searl_sweep'] = freq_sweep
    print(f"  Frequency sweep: {len(freq_sweep['frequencies_Hz'])} points")
    print(f"  Anomalies found: {freq_sweep['n_anomalies']}")

    # Summary
    print("\n" + "=" * 70)
    print("FTD MAGNETIC-FLUX INTERPRETATION")
    print("=" * 70)
    print("""
In FTD, magnetism arises from the curl of the flux field:
  F_mag = β · (∇×J) × Ĵ

KEY FINDINGS:

1. FLUX-MAGNETIC CORRESPONDENCE
   A permanent magnet represents a "frozen" curl pattern in the flux.
   The magnetic field B is proportional to ∇×J.

2. ROTATING MAGNETS
   Time-varying magnetic fields create time-varying flux patterns.
   Energy in the flux field oscillates with rotation.
   Resonances may occur at specific frequencies.

3. ASYMMETRIC CONFIGURATIONS
   Asymmetric magnet arrangements create flux gradients.
   These gradients could, in principle, drive flux flow.
   The asymmetry ratio quantifies the gradient strength.

4. SEARL EFFECT HYPOTHESIS
   If real, would require ~10^8 harmonic amplification to reach 8 THz.
   This is unlikely from simple rotation.
   May require resonant materials or unknown coupling mechanisms.

5. ENERGY EXTRACTION POTENTIAL
   Magnetic systems create flux gradients.
   Gradients enable flux flow.
   Flow through a "load" could extract work.
   BUT: Maintaining gradients requires energy input.
   NET EXTRACTION requires coupling to vacuum flux.

The permanent magnet "puzzle" (where does the energy come from?)
may be resolved in FTD: magnets are flux pumps, continuously
cycling vacuum energy through their structure.
    """)

    # Save results
    output_file = output_dir / "magnetic_flux_results.json"

    def convert(obj):
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        elif isinstance(obj, np.floating):
            return float(obj)
        elif isinstance(obj, np.integer):
            return int(obj)
        return obj

    with open(output_file, 'w') as f:
        json.dump(results, f, indent=2, default=convert)
    print(f"\nResults saved to: {output_file}")


if __name__ == "__main__":
    run_all_simulations()
