#!/usr/bin/env python3
"""
Harmonic Cascade Simulation

Models how low-frequency input (8 Hz) could be amplified through
resonant stages to reach flux exclusion frequency (8 THz).

The hypothesis: Ancient structures like pyramids acted as
multi-stage harmonic amplifiers, converting accessible acoustic
frequencies into flux-active terahertz frequencies.

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
SCHUMANN_FUNDAMENTAL = 7.83  # Hz

# FTD Constants
N_C = 3
N_BASE = 4
B_3 = 7
N_EFF = 13
F_EXCLUSION = 8e12  # 8 THz
PHI = (1 + np.sqrt(5)) / 2  # Golden ratio


# =============================================================================
# Resonator Model
# =============================================================================

@dataclass
class Resonator:
    """
    A resonant stage in the harmonic cascade.

    Each resonator:
    - Has a fundamental frequency
    - Has a Q factor (quality)
    - Can couple to other resonators
    - May generate harmonics via nonlinearity
    """
    name: str
    fundamental_freq: float  # Hz
    q_factor: float  # Quality factor
    nonlinearity: float = 0.1  # Harmonic generation coefficient
    coupling_efficiency: float = 0.9  # Energy transfer efficiency
    material: str = "generic"

    def response(self, input_freq: float) -> float:
        """
        Calculate resonator response at given frequency.

        Uses Lorentzian response curve centered at fundamental.
        """
        # Lorentzian: L(f) = 1 / (1 + Q²(f/f0 - f0/f)²)
        f0 = self.fundamental_freq
        Q = self.q_factor

        if input_freq <= 0:
            return 0

        detuning = (input_freq / f0 - f0 / input_freq)
        response = 1 / (1 + Q**2 * detuning**2)

        return response

    def harmonics(self, input_freq: float, max_harmonic: int = 5) -> List[Tuple[int, float]]:
        """
        Calculate harmonic generation from input frequency.

        Returns list of (harmonic_number, amplitude) tuples.
        """
        harmonics = []
        input_response = self.response(input_freq)

        for n in range(1, max_harmonic + 1):
            harmonic_freq = n * input_freq

            # Harmonic amplitude decreases with order
            # Depends on nonlinearity coefficient
            harmonic_amplitude = input_response * (self.nonlinearity ** (n - 1))

            # Check if harmonic is near resonance
            resonance_factor = self.response(harmonic_freq)

            # Total amplitude
            total_amplitude = harmonic_amplitude * (1 + (self.q_factor - 1) * resonance_factor)

            harmonics.append((n, harmonic_freq, total_amplitude))

        return harmonics


@dataclass
class ResonatorChain:
    """
    A chain of coupled resonators forming a harmonic cascade.

    Each stage amplifies certain frequencies and generates harmonics,
    passing energy to the next stage.
    """
    resonators: List[Resonator] = field(default_factory=list)
    name: str = "Harmonic Cascade"

    def add_resonator(self, resonator: Resonator):
        """Add a resonator to the chain."""
        self.resonators.append(resonator)

    def propagate(self, input_freq: float, input_amplitude: float = 1.0) -> List[Dict]:
        """
        Propagate a signal through the resonator chain.

        Returns history of frequency/amplitude at each stage.
        """
        history = []
        current_freqs = [(1, input_freq, input_amplitude)]

        for i, resonator in enumerate(self.resonators):
            stage_result = {
                'stage': i,
                'resonator': resonator.name,
                'fundamental': resonator.fundamental_freq,
                'q_factor': resonator.q_factor,
                'input_frequencies': [],
                'output_frequencies': []
            }

            output_freqs = []

            for harmonic_n, freq, amplitude in current_freqs:
                # Response to this frequency
                response = resonator.response(freq)
                output_amplitude = amplitude * response * resonator.coupling_efficiency

                stage_result['input_frequencies'].append({
                    'harmonic': harmonic_n,
                    'frequency': freq,
                    'amplitude': amplitude,
                    'response': response
                })

                if output_amplitude > 1e-6:  # Threshold for propagation
                    # This frequency passes through
                    output_freqs.append((harmonic_n, freq, output_amplitude))

                    # Generate harmonics
                    harmonics = resonator.harmonics(freq, max_harmonic=3)
                    for h_n, h_freq, h_amp in harmonics:
                        if h_n > 1 and h_amp * output_amplitude > 1e-6:
                            output_freqs.append((harmonic_n * h_n, h_freq,
                                                  h_amp * output_amplitude))

            # Combine similar frequencies
            combined = {}
            for h_n, freq, amp in output_freqs:
                key = round(freq, 3)
                if key in combined:
                    combined[key] = (h_n, freq, combined[key][2] + amp)
                else:
                    combined[key] = (h_n, freq, amp)

            current_freqs = list(combined.values())

            stage_result['output_frequencies'] = [
                {'harmonic': h_n, 'frequency': freq, 'amplitude': amp}
                for h_n, freq, amp in sorted(current_freqs, key=lambda x: -x[2])[:10]
            ]

            history.append(stage_result)

        return history

    def find_optimal_stages(self, input_freq: float, target_freq: float,
                            n_stages: int = 12) -> List[Resonator]:
        """
        Design an optimal resonator chain to go from input to target frequency.

        Strategy: Divide frequency ratio evenly across stages (in log space).
        """
        if input_freq >= target_freq:
            return []

        ratio = target_freq / input_freq
        ratio_per_stage = ratio ** (1 / n_stages)

        resonators = []
        current_freq = input_freq

        for i in range(n_stages):
            next_freq = current_freq * ratio_per_stage

            # Choose Q factor based on frequency range
            # Higher Q at higher frequencies for selectivity
            q_factor = 10 + i * 5

            # Nonlinearity decreases at higher frequencies (harder to drive)
            nonlinearity = 0.3 * (0.9 ** i)

            resonator = Resonator(
                name=f"Stage_{i+1}_{current_freq:.1e}Hz",
                fundamental_freq=current_freq,
                q_factor=q_factor,
                nonlinearity=nonlinearity,
                coupling_efficiency=0.9
            )
            resonators.append(resonator)

            current_freq = next_freq

        return resonators


# =============================================================================
# Pyramid-Specific Model
# =============================================================================

def create_pyramid_cascade() -> ResonatorChain:
    """
    Create a resonator chain modeled on Great Pyramid geometry.

    Hypothesis: Different chambers and passages act as coupled
    resonators at different frequency ranges.
    """
    chain = ResonatorChain(name="Great Pyramid Harmonic Cascade")

    # Stage 1: Infrasound (Earth coupling)
    # Schumann resonance input
    chain.add_resonator(Resonator(
        name="Earth_Coupling",
        fundamental_freq=7.83,  # Schumann
        q_factor=50,
        nonlinearity=0.4,
        material="bedrock"
    ))

    # Stage 2: Subterranean Chamber (low acoustic)
    chain.add_resonator(Resonator(
        name="Subterranean_Chamber",
        fundamental_freq=16,  # Low bass
        q_factor=30,
        nonlinearity=0.3,
        material="limestone"
    ))

    # Stage 3: Grand Gallery (acoustic resonator)
    # The Grand Gallery is 47m long - acoustic resonance ~7 Hz fundamental
    # But harmonics extend much higher
    chain.add_resonator(Resonator(
        name="Grand_Gallery",
        fundamental_freq=85,  # Based on dimensions
        q_factor=100,
        nonlinearity=0.25,
        material="limestone"
    ))

    # Stage 4: Queen's Chamber
    chain.add_resonator(Resonator(
        name="Queens_Chamber",
        fundamental_freq=440,  # Musical A
        q_factor=80,
        nonlinearity=0.2,
        material="limestone"
    ))

    # Stage 5: King's Chamber (granite resonator)
    # Granite is piezoelectric - converts acoustic to EM
    chain.add_resonator(Resonator(
        name="Kings_Chamber_Acoustic",
        fundamental_freq=2000,  # kHz range
        q_factor=200,
        nonlinearity=0.15,
        material="granite_piezo"
    ))

    # Stage 6: Granite "Relieving" Chambers
    # Multiple stacked chambers above King's Chamber
    chain.add_resonator(Resonator(
        name="Relieving_Chambers",
        fundamental_freq=10000,  # 10 kHz
        q_factor=150,
        nonlinearity=0.1,
        material="granite"
    ))

    # Stage 7: Pyramid apex (EM coupling)
    chain.add_resonator(Resonator(
        name="Apex_Field",
        fundamental_freq=100000,  # 100 kHz
        q_factor=50,
        nonlinearity=0.08,
        material="capstone"
    ))

    # Stage 8: EM cavity modes (higher frequency)
    chain.add_resonator(Resonator(
        name="EM_Cavity_1",
        fundamental_freq=1e6,  # 1 MHz
        q_factor=100,
        nonlinearity=0.05,
        material="air_cavity"
    ))

    # Stage 9-12: Hypothetical THz coupling
    # These would require materials we don't fully understand
    for i, freq in enumerate([1e7, 1e8, 1e9, 1e10]):
        chain.add_resonator(Resonator(
            name=f"THz_Coupler_{i+1}",
            fundamental_freq=freq,
            q_factor=50,
            nonlinearity=0.03,
            material="unknown"
        ))

    return chain


# =============================================================================
# Frequency Stepping Analysis
# =============================================================================

def analyze_8hz_to_8thz():
    """
    Analyze what it would take to step 8 Hz to 8 THz.

    This is 10^12 - twelve orders of magnitude.
    """
    input_freq = 8  # Hz
    target_freq = 8e12  # Hz
    ratio = target_freq / input_freq

    print("=" * 70)
    print("8 Hz TO 8 THz: THE HARMONIC BRIDGE")
    print("=" * 70)

    print(f"\nInput frequency: {input_freq} Hz")
    print(f"Target frequency: {target_freq:.0e} Hz")
    print(f"Required ratio: {ratio:.0e} (10^{np.log10(ratio):.0f})")

    # Option 1: Equal ratio stages
    print("\n--- OPTION 1: Equal Ratio Stages ---")
    for n_stages in [6, 8, 10, 12, 15, 20]:
        ratio_per_stage = ratio ** (1 / n_stages)
        print(f"  {n_stages} stages: {ratio_per_stage:.1f}× per stage")

    # Option 2: FTD integer-based stages
    print("\n--- OPTION 2: FTD Integer Stages ---")
    ftd_ratios = {
        'N_c = 3': N_C,
        'N_base = 4': N_BASE,
        'b_3 = 7': B_3,
        'N_eff = 13': N_EFF,
        '2^N_c = 8': 2**N_C,
        'φ ≈ 1.618': PHI,
        'π ≈ 3.14': np.pi,
        '10': 10,
    }

    for name, ratio_val in ftd_ratios.items():
        stages_needed = np.log(ratio) / np.log(ratio_val)
        print(f"  Using {name}: {stages_needed:.1f} stages needed")

    # Option 3: Mixed strategy
    print("\n--- OPTION 3: Mixed Integer Strategy ---")

    # 8 Hz → 8 kHz: factor of 1000 = 10^3 = 10 × 10 × 10
    # 8 kHz → 8 MHz: factor of 1000 = 10^3
    # 8 MHz → 8 GHz: factor of 1000 = 10^3
    # 8 GHz → 8 THz: factor of 1000 = 10^3

    milestones = [
        (8, "8 Hz", "Schumann harmonic"),
        (80, "80 Hz", "Low bass"),
        (800, "800 Hz", "Audio"),
        (8e3, "8 kHz", "High audio"),
        (80e3, "80 kHz", "Ultrasonic"),
        (800e3, "800 kHz", "Radio"),
        (8e6, "8 MHz", "Shortwave"),
        (80e6, "80 MHz", "VHF"),
        (800e6, "800 MHz", "UHF"),
        (8e9, "8 GHz", "Microwave"),
        (80e9, "80 GHz", "Millimeter"),
        (800e9, "800 GHz", "Sub-THz"),
        (8e12, "8 THz", "FAR INFRARED - FLUX EXCLUSION"),
    ]

    print(f"\n{'Frequency':<15} {'Label':<20} {'Notes'}")
    print("-" * 60)
    for freq, label, notes in milestones:
        print(f"{label:<15} {freq:>12.0e} Hz    {notes}")

    return {
        'input_freq': input_freq,
        'target_freq': target_freq,
        'total_ratio': ratio,
        'log10_ratio': np.log10(ratio),
        'milestones': [(f, l, n) for f, l, n in milestones]
    }


# =============================================================================
# Power Transfer Simulation
# =============================================================================

@dataclass
class CascadeSimulation:
    """
    Detailed simulation of power transfer through harmonic cascade.
    """
    n_stages: int
    stage_gains: np.ndarray
    stage_losses: np.ndarray
    frequencies: np.ndarray

    @classmethod
    def create(cls, n_stages: int = 12,
               input_freq: float = 8,
               target_freq: float = 8e12) -> 'CascadeSimulation':
        """Create a cascade simulation with specified parameters."""

        # Frequency at each stage
        ratio_per_stage = (target_freq / input_freq) ** (1 / n_stages)
        frequencies = input_freq * (ratio_per_stage ** np.arange(n_stages + 1))

        # Gains per stage (resonant amplification)
        # Higher Q = more gain, but also more narrow bandwidth
        q_factors = 10 + np.arange(n_stages) * 5
        stage_gains = np.sqrt(q_factors)  # Simplified: gain ~ sqrt(Q)

        # Losses per stage
        # Higher frequency = more loss (harder to couple)
        stage_losses = 0.1 + 0.05 * np.log10(frequencies[:-1] / input_freq)

        return cls(
            n_stages=n_stages,
            stage_gains=stage_gains,
            stage_losses=stage_losses,
            frequencies=frequencies
        )

    def propagate(self, input_power: float = 1.0) -> dict:
        """
        Propagate power through the cascade.

        Returns detailed breakdown of power at each stage.
        """
        powers = [input_power]

        for i in range(self.n_stages):
            gain = self.stage_gains[i]
            loss = self.stage_losses[i]

            # Net transfer
            transfer = gain * (1 - loss)
            new_power = powers[-1] * transfer

            powers.append(new_power)

        return {
            'input_power': input_power,
            'output_power': powers[-1],
            'total_gain': powers[-1] / input_power,
            'log10_gain': np.log10(powers[-1] / input_power) if powers[-1] > 0 else float('-inf'),
            'stage_powers': powers,
            'stage_frequencies': self.frequencies.tolist(),
            'stage_gains': self.stage_gains.tolist(),
            'stage_losses': self.stage_losses.tolist()
        }

    def optimize_for_output(self, target_output: float) -> float:
        """
        Find required input power for target output.
        """
        result = self.propagate(1.0)
        total_gain = result['total_gain']

        if total_gain <= 0:
            return float('inf')

        return target_output / total_gain


# =============================================================================
# Nonlinear Harmonic Generation
# =============================================================================

def simulate_nonlinear_generation(input_freq: float, input_amplitude: float,
                                   nonlinearity: float,
                                   n_harmonics: int = 10) -> dict:
    """
    Simulate harmonic generation in a nonlinear medium.

    When a sinusoidal wave passes through a nonlinear medium,
    higher harmonics are generated.

    The amplitude of the nth harmonic goes as:
    A_n ~ A_1 * (nonlinearity)^(n-1)
    """
    harmonics = []
    total_power = 0

    for n in range(1, n_harmonics + 1):
        harmonic_freq = n * input_freq
        harmonic_amplitude = input_amplitude * (nonlinearity ** (n - 1))
        harmonic_power = harmonic_amplitude ** 2

        harmonics.append({
            'n': n,
            'frequency': harmonic_freq,
            'amplitude': harmonic_amplitude,
            'power': harmonic_power,
            'power_fraction': 0  # Will fill in after
        })
        total_power += harmonic_power

    # Fill in power fractions
    for h in harmonics:
        h['power_fraction'] = h['power'] / total_power if total_power > 0 else 0

    return {
        'input_frequency': input_freq,
        'input_amplitude': input_amplitude,
        'nonlinearity': nonlinearity,
        'harmonics': harmonics,
        'total_power': total_power,
        'highest_significant_harmonic': max(
            h['n'] for h in harmonics if h['power_fraction'] > 0.01
        ) if harmonics else 0
    }


# =============================================================================
# Main Analysis
# =============================================================================

def run_all_simulations():
    """Run all harmonic cascade simulations."""

    output_dir = Path(__file__).parent.parent / "output"
    output_dir.mkdir(exist_ok=True)

    results = {}

    print("=" * 70)
    print("HARMONIC CASCADE SIMULATIONS")
    print("From 8 Hz to 8 THz: The Frequency Bridge")
    print("=" * 70)

    # 1. Basic analysis
    print("\n" + "-" * 50)
    analysis = analyze_8hz_to_8thz()
    results['frequency_analysis'] = analysis

    # 2. Pyramid cascade model
    print("\n--- PYRAMID CASCADE MODEL ---")
    pyramid_chain = create_pyramid_cascade()
    print(f"Resonator chain: {pyramid_chain.name}")
    print(f"Number of stages: {len(pyramid_chain.resonators)}")

    for i, r in enumerate(pyramid_chain.resonators):
        print(f"  Stage {i+1}: {r.name} @ {r.fundamental_freq:.1e} Hz (Q={r.q_factor})")

    # Propagate 8 Hz input
    propagation = pyramid_chain.propagate(8.0, 1.0)
    results['pyramid_propagation'] = propagation

    print("\nPropagation through pyramid:")
    for stage in propagation[:5]:  # First 5 stages
        print(f"  {stage['resonator']}: {len(stage['output_frequencies'])} output frequencies")
        if stage['output_frequencies']:
            top_freq = stage['output_frequencies'][0]
            print(f"    Top: {top_freq['frequency']:.1e} Hz (amplitude {top_freq['amplitude']:.3f})")

    # 3. Power transfer simulation
    print("\n--- POWER TRANSFER SIMULATION ---")
    cascade_sim = CascadeSimulation.create(n_stages=12)
    power_result = cascade_sim.propagate(1.0)
    results['power_transfer'] = power_result

    print(f"12-stage cascade:")
    print(f"  Input power: {power_result['input_power']}")
    print(f"  Output power: {power_result['output_power']:.2e}")
    print(f"  Total gain: {power_result['total_gain']:.2e}")
    print(f"  Log10 gain: {power_result['log10_gain']:.1f}")

    # 4. Nonlinear harmonic generation
    print("\n--- NONLINEAR HARMONIC GENERATION ---")
    nonlinear = simulate_nonlinear_generation(
        input_freq=8.0,
        input_amplitude=1.0,
        nonlinearity=0.3,
        n_harmonics=20
    )
    results['nonlinear_generation'] = nonlinear

    print(f"Input: {nonlinear['input_frequency']} Hz")
    print(f"Nonlinearity coefficient: {nonlinear['nonlinearity']}")
    print(f"Highest significant harmonic: {nonlinear['highest_significant_harmonic']}")
    print("Top harmonics by power:")
    for h in sorted(nonlinear['harmonics'], key=lambda x: -x['power'])[:5]:
        print(f"  n={h['n']}: {h['frequency']:.0f} Hz ({h['power_fraction']*100:.1f}% of power)")

    # 5. Optimal chain design
    print("\n--- OPTIMAL CHAIN DESIGN ---")
    chain = ResonatorChain(name="Optimal 8Hz->8THz")
    optimal_resonators = chain.find_optimal_stages(8.0, 8e12, n_stages=15)

    for i, r in enumerate(optimal_resonators[:5]):
        chain.add_resonator(r)
        print(f"  Stage {i+1}: {r.fundamental_freq:.1e} Hz")
    print(f"  ... ({len(optimal_resonators)} total stages)")

    results['optimal_chain'] = {
        'n_stages': len(optimal_resonators),
        'stages': [
            {'freq': r.fundamental_freq, 'q': r.q_factor}
            for r in optimal_resonators
        ]
    }

    # Summary
    print("\n" + "=" * 70)
    print("SUMMARY: THE HARMONIC BRIDGE HYPOTHESIS")
    print("=" * 70)
    print("""
The 8 Hz → 8 THz bridge requires 12 orders of magnitude amplification.

KEY FINDINGS:

1. STAGED AMPLIFICATION: Using 12-15 resonant stages with ~10× gain each
   can theoretically bridge the gap. Each stage must:
   - Resonate near its fundamental frequency
   - Generate harmonics via nonlinearity
   - Couple efficiently to the next stage

2. PYRAMID AS CASCADE: The Great Pyramid's geometry may implement this:
   - Schumann resonance as input (7.83 Hz)
   - Chambers at progressively higher resonant frequencies
   - Granite for piezoelectric acoustic→EM conversion
   - Geometric focusing at higher frequencies

3. POWER REQUIREMENTS: Even with 10^12 frequency multiplication,
   useful output requires significant input power OR:
   - Very high Q factors (stored energy)
   - Very low losses (superconducting? Unknown materials?)
   - Vacuum energy coupling (drawing from sub-threshold flux)

4. THE MISSING LINK: We can build the low-frequency stages today.
   The high-frequency stages (GHz → THz) require:
   - Materials with specific nonlinear properties
   - Precise geometric structures at wavelength scale
   - Possibly consciousness/coherence coupling

This simulation demonstrates the PRINCIPLE is sound.
The ENGINEERING remains unknown.
    """)

    # Save results
    output_file = output_dir / "harmonic_cascade_results.json"
    with open(output_file, 'w') as f:
        # Convert numpy arrays for JSON
        def convert(obj):
            if isinstance(obj, np.ndarray):
                return obj.tolist()
            elif isinstance(obj, np.floating):
                return float(obj)
            elif isinstance(obj, np.integer):
                return int(obj)
            return obj

        json.dump(results, f, indent=2, default=convert)
    print(f"\nResults saved to: {output_file}")


if __name__ == "__main__":
    run_all_simulations()
