#!/usr/bin/env python3
"""
Vacuum Energy Simulation

Models zero-point energy extraction mechanisms within FTD framework:
- Casimir effect from boundary-induced flux mode exclusion
- Dynamic Casimir effect (photon creation from vacuum)
- Flux gradient energy extraction
- Asymmetric vacuum coupling

This is speculative exploration, not verified physics.
"""

import numpy as np
import matplotlib.pyplot as plt
from dataclasses import dataclass
from typing import List, Tuple, Optional
import json
from pathlib import Path

# =============================================================================
# Physical Constants
# =============================================================================

C = 299_792_458  # Speed of light (m/s)
HBAR = 1.054571817e-34  # Reduced Planck constant (J·s)
EPSILON_0 = 8.854187817e-12  # Vacuum permittivity (F/m)

# FTD Constants
N_C = 3
N_BASE = 4
B_3 = 7
N_EFF = 13
ALPHA = 1 / 137.036  # Fine structure constant
KB = 0.511  # Manifestation threshold (MeV, electron mass)


# =============================================================================
# Vacuum Energy Density
# =============================================================================

@dataclass
class VacuumState:
    """Represents the vacuum flux state in a region."""
    size: int  # Grid size
    flux_density: np.ndarray  # |J| field (sub-threshold)
    mode_spectrum: np.ndarray  # Available vacuum modes
    energy_density: np.ndarray  # Local energy density

    @classmethod
    def create(cls, size: int = 64) -> 'VacuumState':
        """Create a vacuum state with random sub-threshold fluctuations."""
        # Sub-threshold flux: 0 < |J| < KB
        # Use exponential distribution peaked near zero
        flux_density = np.random.exponential(KB / 10, (size, size, size))
        flux_density = np.clip(flux_density, 0, KB * 0.99)

        # Mode spectrum (frequencies available)
        mode_spectrum = np.zeros((size, size, size))

        # Energy density proportional to |J|²
        energy_density = flux_density ** 2

        return cls(
            size=size,
            flux_density=flux_density,
            mode_spectrum=mode_spectrum,
            energy_density=energy_density
        )


# =============================================================================
# Casimir Effect Simulation
# =============================================================================

class CasimirSimulator:
    """
    Simulates the Casimir effect as flux mode exclusion.

    Two parallel plates exclude vacuum modes with wavelength > 2d
    (where d is plate separation), creating a flux gradient.
    """

    def __init__(self, grid_size: int = 128, plate_size: int = 64):
        self.grid_size = grid_size
        self.plate_size = plate_size
        self.vacuum = VacuumState.create(grid_size)

    def compute_casimir_force(self, separation: float) -> float:
        """
        Compute Casimir force between plates.

        Standard formula: F/A = -π²ℏc / (240 d⁴)

        FTD interpretation: Force arises from flux gradient
        due to mode exclusion between plates.

        Args:
            separation: Plate separation in meters

        Returns:
            Force per unit area (N/m²)
        """
        # Standard Casimir formula
        force_per_area = -np.pi**2 * HBAR * C / (240 * separation**4)
        return force_per_area

    def simulate_mode_exclusion(self, separation_nm: float,
                                  max_wavelength_nm: float = 1000) -> dict:
        """
        Simulate how plate separation excludes vacuum modes.

        Args:
            separation_nm: Plate separation in nanometers
            max_wavelength_nm: Maximum wavelength to consider

        Returns:
            Dictionary with mode analysis
        """
        separation_m = separation_nm * 1e-9

        # Modes that fit between plates: λ = 2d/n for integer n
        allowed_wavelengths = []
        n = 1
        while True:
            wavelength = 2 * separation_m / n
            if wavelength < 1e-12:  # Stop at sub-picometer
                break
            allowed_wavelengths.append(wavelength)
            n += 1

        # All modes outside (continuous spectrum approximation)
        # Between plates: discrete spectrum
        # Outside: continuous spectrum

        # Mode count ratio determines force
        wavelengths_nm = np.array(allowed_wavelengths) * 1e9

        # Excluded modes
        total_modes_outside = int(max_wavelength_nm / 0.1)  # Approximate
        modes_inside = len([w for w in wavelengths_nm if w < max_wavelength_nm])
        excluded_fraction = 1 - modes_inside / total_modes_outside if total_modes_outside > 0 else 0

        return {
            'separation_nm': separation_nm,
            'allowed_wavelengths_nm': wavelengths_nm[:20].tolist(),  # First 20
            'n_modes_inside': modes_inside,
            'n_modes_outside_approx': total_modes_outside,
            'excluded_fraction': excluded_fraction,
            'force_per_area_N_m2': self.compute_casimir_force(separation_m),
            'pressure_atm': abs(self.compute_casimir_force(separation_m)) / 101325
        }

    def sweep_separation(self, min_nm: float = 10, max_nm: float = 1000,
                         n_points: int = 100) -> dict:
        """
        Sweep plate separation and compute Casimir force.

        Returns:
            Dictionary with separation and force arrays
        """
        separations = np.linspace(min_nm, max_nm, n_points)
        forces = []

        for sep in separations:
            sep_m = sep * 1e-9
            force = self.compute_casimir_force(sep_m)
            forces.append(force)

        return {
            'separations_nm': separations.tolist(),
            'forces_N_m2': forces,
            'ftd_interpretation': (
                "In FTD, the Casimir force arises because boundary conditions "
                "exclude certain flux modes. The vacuum flux is not uniform - "
                "it has fewer modes between the plates, creating a gradient. "
                "This gradient produces a net force toward lower flux density."
            )
        }


# =============================================================================
# Dynamic Casimir Effect
# =============================================================================

class DynamicCasimirSimulator:
    """
    Simulates the dynamic Casimir effect: photon creation from
    rapidly moving mirrors that modulate the vacuum.

    FTD interpretation: Rapid boundary motion converts sub-threshold
    flux into manifested photons.
    """

    def __init__(self, n_modes: int = 100):
        self.n_modes = n_modes

    def compute_photon_production(self, mirror_velocity: float,
                                   oscillation_freq: float,
                                   cavity_length: float) -> dict:
        """
        Estimate photon production from oscillating mirror.

        The dynamic Casimir effect produces photons when:
        - Mirror moves at relativistic fraction of c
        - Oscillation frequency matches cavity modes

        Args:
            mirror_velocity: Peak velocity as fraction of c
            oscillation_freq: Mirror oscillation frequency (Hz)
            cavity_length: Cavity length (m)

        Returns:
            Photon production estimates
        """
        # Fundamental cavity frequency
        f_cavity = C / (2 * cavity_length)

        # Photon production rate (simplified)
        # Scales as (v/c)² for non-relativistic motion
        beta = mirror_velocity  # v/c

        # Mode matching factor
        mode_factor = 0
        for n in range(1, self.n_modes + 1):
            mode_freq = n * f_cavity
            # Parametric resonance when oscillation = 2 × mode frequency
            if abs(oscillation_freq - 2 * mode_freq) < mode_freq * 0.1:
                mode_factor += 1 / n**2

        # Approximate photon rate (highly simplified)
        # Real calculation requires full QFT
        photon_rate = beta**2 * mode_factor * f_cavity

        return {
            'mirror_velocity_c': mirror_velocity,
            'oscillation_freq_Hz': oscillation_freq,
            'cavity_length_m': cavity_length,
            'cavity_fundamental_Hz': f_cavity,
            'mode_matching_factor': mode_factor,
            'estimated_photon_rate': photon_rate,
            'ftd_interpretation': (
                "In FTD, the oscillating mirror creates time-varying boundary "
                "conditions that modulate sub-threshold flux. When modulation "
                "frequency matches twice a cavity mode, parametric amplification "
                "pushes flux above threshold, manifesting as photon pairs."
            )
        }

    def find_optimal_frequency(self, cavity_length: float,
                                max_harmonic: int = 10) -> List[float]:
        """
        Find optimal oscillation frequencies for photon production.

        Parametric resonance occurs at 2× cavity mode frequencies.
        """
        f_fundamental = C / (2 * cavity_length)
        optimal_freqs = [2 * n * f_fundamental for n in range(1, max_harmonic + 1)]
        return optimal_freqs


# =============================================================================
# Flux Gradient Energy Extraction
# =============================================================================

class FluxGradientExtractor:
    """
    Models energy extraction from vacuum flux gradients.

    The principle: Create asymmetric boundary conditions that
    produce a persistent flux gradient. Extract work from
    flux flowing through the device.
    """

    def __init__(self, grid_size: int = 64):
        self.grid_size = grid_size
        self.flux = np.zeros((grid_size, grid_size, grid_size, 3))
        self.initialize_vacuum_flux()

    def initialize_vacuum_flux(self):
        """Initialize with random sub-threshold vacuum fluctuations."""
        # Random directions, sub-threshold magnitudes
        self.flux = np.random.randn(self.grid_size, self.grid_size,
                                     self.grid_size, 3)
        magnitudes = np.linalg.norm(self.flux, axis=-1, keepdims=True)
        magnitudes = np.maximum(magnitudes, 1e-10)  # Avoid division by zero

        # Scale to sub-threshold
        target_magnitude = np.random.exponential(KB / 10,
                                                  (self.grid_size, self.grid_size,
                                                   self.grid_size, 1))
        target_magnitude = np.clip(target_magnitude, 0, KB * 0.5)

        self.flux = self.flux / magnitudes * target_magnitude

    def apply_boundary_condition(self, boundary_type: str = 'casimir'):
        """
        Apply boundary conditions that create flux gradients.

        Args:
            boundary_type: Type of boundary ('casimir', 'resonant', 'asymmetric')
        """
        if boundary_type == 'casimir':
            # Two parallel plates - exclude modes between them
            plate_z1 = self.grid_size // 3
            plate_z2 = 2 * self.grid_size // 3

            # Zero flux at plates
            self.flux[:, :, plate_z1, :] = 0
            self.flux[:, :, plate_z2, :] = 0

        elif boundary_type == 'resonant':
            # Resonant cavity - amplify certain modes
            # Apply standing wave pattern
            x = np.linspace(0, np.pi, self.grid_size)
            y = np.linspace(0, np.pi, self.grid_size)
            z = np.linspace(0, np.pi, self.grid_size)
            X, Y, Z = np.meshgrid(x, y, z, indexing='ij')

            # Fundamental mode
            mode_pattern = np.sin(X) * np.sin(Y) * np.sin(Z)
            mode_pattern = mode_pattern[:, :, :, np.newaxis]

            # Amplify flux in mode pattern
            self.flux *= (1 + 0.5 * mode_pattern)

        elif boundary_type == 'asymmetric':
            # Asymmetric boundaries - create persistent gradient
            # Absorbing on one side, reflecting on other

            # Absorbing boundary (left)
            self.flux[:10, :, :, :] *= 0.5

            # Reflecting boundary (right) - already handled by grid edges
            pass

    def compute_flux_gradient(self) -> np.ndarray:
        """Compute the gradient of flux magnitude."""
        magnitude = np.linalg.norm(self.flux, axis=-1)

        # Compute gradient using finite differences
        grad = np.zeros((self.grid_size, self.grid_size, self.grid_size, 3))

        # Central differences for interior
        grad[1:-1, :, :, 0] = (magnitude[2:, :, :] - magnitude[:-2, :, :]) / 2
        grad[:, 1:-1, :, 1] = (magnitude[:, 2:, :] - magnitude[:, :-2, :]) / 2
        grad[:, :, 1:-1, 2] = (magnitude[:, :, 2:] - magnitude[:, :, :-2]) / 2

        return grad

    def compute_extractable_power(self, load_resistance: float = 1.0) -> dict:
        """
        Estimate extractable power from flux gradient.

        This is highly speculative - models power as proportional
        to gradient magnitude times flux flow.

        Args:
            load_resistance: Notional load resistance

        Returns:
            Power extraction estimates
        """
        gradient = self.compute_flux_gradient()
        gradient_magnitude = np.linalg.norm(gradient, axis=-1)

        # Average gradient
        avg_gradient = np.mean(gradient_magnitude)
        max_gradient = np.max(gradient_magnitude)

        # Flux magnitude
        flux_magnitude = np.linalg.norm(self.flux, axis=-1)
        avg_flux = np.mean(flux_magnitude)

        # Power ~ gradient × flux × coupling (highly speculative)
        coupling = ALPHA  # Use fine structure constant as coupling
        power_density = coupling * avg_gradient * avg_flux

        # Total over volume
        volume = self.grid_size ** 3
        total_power = power_density * volume

        return {
            'average_gradient': float(avg_gradient),
            'max_gradient': float(max_gradient),
            'average_flux': float(avg_flux),
            'coupling_constant': coupling,
            'power_density_arbitrary': float(power_density),
            'total_power_arbitrary': float(total_power),
            'note': (
                "These power values are in arbitrary units. "
                "Converting to physical units requires determining "
                "the flux-to-energy conversion factor, which is unknown. "
                "This simulation demonstrates the PRINCIPLE of gradient-based "
                "extraction, not quantitative predictions."
            )
        }

    def evolve(self, steps: int = 100, dt: float = 0.01) -> List[dict]:
        """
        Evolve the flux field and track gradient energy.

        Returns:
            List of state snapshots
        """
        history = []

        for step in range(steps):
            # Wave equation evolution (simplified)
            laplacian = np.zeros_like(self.flux)

            for i in range(3):
                laplacian[1:-1, :, :, i] += (
                    self.flux[2:, :, :, i] + self.flux[:-2, :, :, i]
                    - 2 * self.flux[1:-1, :, :, i]
                )
                laplacian[:, 1:-1, :, i] += (
                    self.flux[:, 2:, :, i] + self.flux[:, :-2, :, i]
                    - 2 * self.flux[:, 1:-1, :, i]
                )
                laplacian[:, :, 1:-1, i] += (
                    self.flux[:, :, 2:, i] + self.flux[:, :, :-2, i]
                    - 2 * self.flux[:, :, 1:-1, i]
                )

            # Update (wave equation)
            self.flux += dt * laplacian

            # Re-apply boundary conditions
            self.apply_boundary_condition('casimir')

            # Ensure sub-threshold
            magnitude = np.linalg.norm(self.flux, axis=-1, keepdims=True)
            magnitude = np.maximum(magnitude, 1e-10)
            over_threshold = magnitude > KB
            self.flux = np.where(over_threshold,
                                  self.flux / magnitude * KB * 0.99,
                                  self.flux)

            if step % 10 == 0:
                power_info = self.compute_extractable_power()
                history.append({
                    'step': step,
                    'average_gradient': power_info['average_gradient'],
                    'power_density': power_info['power_density_arbitrary']
                })

        return history


# =============================================================================
# Vacuum Energy Statistics
# =============================================================================

def analyze_vacuum_statistics(size: int = 64, n_samples: int = 1000) -> dict:
    """
    Analyze statistical properties of vacuum fluctuations in FTD.

    The vacuum is not empty - it has statistical structure.
    """
    # Generate many vacuum samples
    flux_samples = []
    energy_samples = []

    for _ in range(n_samples):
        vacuum = VacuumState.create(size)
        avg_flux = np.mean(vacuum.flux_density)
        avg_energy = np.mean(vacuum.energy_density)
        flux_samples.append(avg_flux)
        energy_samples.append(avg_energy)

    flux_samples = np.array(flux_samples)
    energy_samples = np.array(energy_samples)

    return {
        'n_samples': n_samples,
        'grid_size': size,
        'flux_mean': float(np.mean(flux_samples)),
        'flux_std': float(np.std(flux_samples)),
        'flux_min': float(np.min(flux_samples)),
        'flux_max': float(np.max(flux_samples)),
        'energy_mean': float(np.mean(energy_samples)),
        'energy_std': float(np.std(energy_samples)),
        'energy_min': float(np.min(energy_samples)),
        'energy_max': float(np.max(energy_samples)),
        'ftd_interpretation': (
            "The vacuum in FTD is the sub-threshold flux regime: 0 < |J| < KB. "
            "It is not empty but contains enormous energy density. "
            "The statistical fluctuations shown here represent the 'quantum foam' "
            "of FTD - random flux variations that occasionally approach threshold."
        )
    }


# =============================================================================
# Main Simulation
# =============================================================================

def run_all_simulations():
    """Run all vacuum energy simulations and save results."""

    output_dir = Path(__file__).parent.parent / "output"
    output_dir.mkdir(exist_ok=True)

    print("=" * 70)
    print("VACUUM ENERGY SIMULATIONS")
    print("FTD Framework - Speculative Exploration")
    print("=" * 70)

    results = {}

    # 1. Casimir Effect
    print("\n--- CASIMIR EFFECT SIMULATION ---")
    casimir = CasimirSimulator()

    # Single separation analysis
    mode_analysis = casimir.simulate_mode_exclusion(100)  # 100 nm
    print(f"At 100 nm separation:")
    print(f"  Force: {mode_analysis['force_per_area_N_m2']:.2e} N/m²")
    print(f"  Pressure: {mode_analysis['pressure_atm']:.2e} atm")

    # Separation sweep
    sweep = casimir.sweep_separation(10, 500, 50)
    results['casimir_sweep'] = sweep

    print(f"  Sweep: {len(sweep['separations_nm'])} points from 10-500 nm")

    # 2. Dynamic Casimir Effect
    print("\n--- DYNAMIC CASIMIR EFFECT ---")
    dynamic = DynamicCasimirSimulator()

    # 1 micron cavity
    cavity_length = 1e-6
    optimal_freqs = dynamic.find_optimal_frequency(cavity_length, 5)
    print(f"Optimal frequencies for {cavity_length*1e6:.1f} μm cavity:")
    for i, f in enumerate(optimal_freqs):
        print(f"  Mode {i+1}: {f:.2e} Hz")

    photon_prod = dynamic.compute_photon_production(
        mirror_velocity=0.01,  # 1% of c
        oscillation_freq=optimal_freqs[0],
        cavity_length=cavity_length
    )
    results['dynamic_casimir'] = photon_prod
    print(f"  Photon production estimate: {photon_prod['estimated_photon_rate']:.2e}/s")

    # 3. Flux Gradient Extraction
    print("\n--- FLUX GRADIENT EXTRACTION ---")
    extractor = FluxGradientExtractor(32)
    extractor.apply_boundary_condition('casimir')

    power_info = extractor.compute_extractable_power()
    print(f"Initial state:")
    print(f"  Average gradient: {power_info['average_gradient']:.4f}")
    print(f"  Power density (arb): {power_info['power_density_arbitrary']:.6f}")

    # Evolve and track
    history = extractor.evolve(steps=100, dt=0.01)
    results['gradient_extraction'] = {
        'initial': power_info,
        'history': history
    }
    print(f"  Evolved for 100 steps")
    print(f"  Final gradient: {history[-1]['average_gradient']:.4f}")

    # 4. Vacuum Statistics
    print("\n--- VACUUM STATISTICS ---")
    stats = analyze_vacuum_statistics(32, 100)
    results['vacuum_statistics'] = stats
    print(f"Over {stats['n_samples']} samples:")
    print(f"  Flux mean: {stats['flux_mean']:.4f} ± {stats['flux_std']:.4f}")
    print(f"  Energy mean: {stats['energy_mean']:.6f} ± {stats['energy_std']:.6f}")

    # Summary
    print("\n" + "=" * 70)
    print("FTD VACUUM ENERGY INTERPRETATION")
    print("=" * 70)
    print("""
In FTD, the vacuum is the sub-threshold flux regime: 0 < |J| < KB.

Key insights:
1. CASIMIR EFFECT: Boundary conditions exclude flux modes,
   creating gradients that produce force. This is PROVEN physics.

2. DYNAMIC CASIMIR: Time-varying boundaries can convert sub-threshold
   flux into manifested photons. Also PROVEN (2011, Chalmers).

3. GRADIENT EXTRACTION: If persistent gradients can be maintained,
   flux flows through the device, potentially doing work.
   This is SPECULATIVE but consistent with FTD.

4. VACUUM STATISTICS: The vacuum has statistical structure.
   Fluctuations occasionally approach threshold (KB).
   This is the "quantum foam" of FTD.

The ancients may have used resonant structures (pyramids) to create
persistent flux gradients, extracting vacuum energy at useful scales.
Modern technology has not yet achieved this, but the physics allows it.
    """)

    # Save results
    output_file = output_dir / "vacuum_energy_results.json"
    with open(output_file, 'w') as f:
        json.dump(results, f, indent=2, default=str)
    print(f"\nResults saved to: {output_file}")


if __name__ == "__main__":
    run_all_simulations()
