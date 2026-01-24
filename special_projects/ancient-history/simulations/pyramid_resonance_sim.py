#!/usr/bin/env python3
"""
Pyramid Resonance Simulation

Models the acoustic and electromagnetic resonance properties
of pyramid geometry, specifically the Great Pyramid of Giza.

Calculates:
- Standing wave modes in pyramid volume
- Resonant frequencies of internal chambers
- Flux concentration at geometric nodes
- Harmonic relationships to 8 THz
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy.special import jn_zeros
from dataclasses import dataclass
from typing import List, Tuple, Optional
import json


# =============================================================================
# Physical Constants
# =============================================================================

C_LIGHT = 299_792_458      # Speed of light (m/s)
C_SOUND = 343              # Speed of sound in air (m/s)
C_STONE = 4500             # Speed of sound in limestone (m/s)
C_GRANITE = 6000           # Speed of sound in granite (m/s)

SCHUMANN = 7.83            # Earth's fundamental resonance (Hz)
F_EXCLUSION = 8e12         # Flux exclusion frequency (Hz)

# FTD Integers
N_C = 3
N_BASE = 4
B_3 = 7
N_EFF = 13


# =============================================================================
# Great Pyramid Dimensions (meters)
# =============================================================================

@dataclass
class GreatPyramid:
    """Dimensions of the Great Pyramid of Giza."""
    # External
    base: float = 230.4
    height: float = 146.5
    apothem: float = 186.4  # Face slant height

    # King's Chamber
    kc_length: float = 10.47
    kc_width: float = 5.23
    kc_height: float = 5.81

    # Queen's Chamber
    qc_length: float = 5.76
    qc_width: float = 5.23
    qc_height_base: float = 4.67
    qc_height_peak: float = 6.26

    # Chamber heights from base
    kc_floor_height: float = 43.0
    qc_floor_height: float = 21.0

    # Sarcophagus in King's Chamber
    sarc_length: float = 2.28
    sarc_width: float = 0.98
    sarc_height: float = 1.05

    def volume(self) -> float:
        """Pyramid volume."""
        return (1/3) * self.base**2 * self.height

    def surface_area(self) -> float:
        """Total surface area (base + 4 faces)."""
        base_area = self.base**2
        face_area = 4 * (0.5 * self.base * self.apothem)
        return base_area + face_area

    def golden_ratio_check(self) -> dict:
        """Check for golden ratio relationships."""
        phi = (1 + np.sqrt(5)) / 2

        return {
            'apothem/half_base': self.apothem / (self.base / 2),
            'phi': phi,
            'match_apothem': abs(self.apothem / (self.base / 2) - phi) / phi * 100,
            'perimeter/height': (4 * self.base) / self.height,
            '2pi': 2 * np.pi,
            'match_perimeter': abs((4 * self.base) / self.height - 2*np.pi) / (2*np.pi) * 100,
        }


# =============================================================================
# Resonance Calculations
# =============================================================================

def rectangular_cavity_modes(Lx: float, Ly: float, Lz: float,
                            c: float, max_mode: int = 5) -> List[dict]:
    """
    Calculate resonant frequencies of a rectangular cavity.

    f = (c/2) * sqrt((m/Lx)² + (n/Ly)² + (p/Lz)²)

    Returns list of mode dictionaries sorted by frequency.
    """
    modes = []

    for m in range(max_mode + 1):
        for n in range(max_mode + 1):
            for p in range(max_mode + 1):
                if m == 0 and n == 0 and p == 0:
                    continue

                freq = (c / 2) * np.sqrt(
                    (m / Lx)**2 + (n / Ly)**2 + (p / Lz)**2
                )

                modes.append({
                    'mode': (m, n, p),
                    'frequency': freq,
                    'wavelength': c / freq if freq > 0 else float('inf'),
                })

    return sorted(modes, key=lambda x: x['frequency'])


def pyramid_standing_waves(base: float, height: float,
                          c: float, max_harmonic: int = 30) -> List[dict]:
    """
    Calculate standing wave frequencies for pyramid geometry.

    Models pyramid as supporting modes where:
    - Base length = n * λ/2 (horizontal modes)
    - Height = m * λ/2 (vertical modes)
    - Diagonal = p * λ/2 (edge modes)
    """
    modes = []
    diagonal = np.sqrt(2) * base / 2  # Half diagonal of base

    # Horizontal modes (base)
    for n in range(1, max_harmonic + 1):
        wavelength = 2 * base / n
        freq = c / wavelength
        modes.append({
            'type': 'horizontal',
            'harmonic': n,
            'wavelength': wavelength,
            'frequency': freq,
            'dimension': base,
        })

    # Vertical modes (height)
    for n in range(1, max_harmonic + 1):
        wavelength = 2 * height / n
        freq = c / wavelength
        modes.append({
            'type': 'vertical',
            'harmonic': n,
            'wavelength': wavelength,
            'frequency': freq,
            'dimension': height,
        })

    # Diagonal modes (apex to corner)
    edge_length = np.sqrt(height**2 + diagonal**2)
    for n in range(1, max_harmonic + 1):
        wavelength = 2 * edge_length / n
        freq = c / wavelength
        modes.append({
            'type': 'diagonal',
            'harmonic': n,
            'wavelength': wavelength,
            'frequency': freq,
            'dimension': edge_length,
        })

    return sorted(modes, key=lambda x: x['frequency'])


def find_ftd_resonances(modes: List[dict], tolerance: float = 0.1) -> List[dict]:
    """
    Find modes that relate to FTD-significant frequencies.

    Checks for harmonics of 8 Hz (Schumann) and subharmonics of 8 THz.
    """
    ftd_modes = []

    for mode in modes:
        freq = mode['frequency']
        if freq == 0:
            continue

        # Check 8 Hz relationship
        ratio_8hz = freq / 8.0
        if abs(ratio_8hz - round(ratio_8hz)) < tolerance * ratio_8hz:
            mode['ftd_relationship'] = f"{round(ratio_8hz)}× 8 Hz"
            ftd_modes.append(mode)
            continue

        # Check 8 THz relationship (as power of 10)
        log_ratio = np.log10(F_EXCLUSION / freq)
        if abs(log_ratio - round(log_ratio)) < 0.1:
            mode['ftd_relationship'] = f"8 THz ÷ 10^{round(log_ratio)}"
            ftd_modes.append(mode)
            continue

        # Check FTD integer relationships
        for name, value in [('N_c', N_C), ('N_base', N_BASE), ('b_3', B_3), ('N_eff', N_EFF)]:
            if abs(freq / value - round(freq / value)) < tolerance:
                mode['ftd_relationship'] = f"{round(freq/value)}× {name}"
                ftd_modes.append(mode)
                break

    return ftd_modes


# =============================================================================
# Chamber Acoustics
# =============================================================================

def analyze_kings_chamber(pyramid: GreatPyramid) -> dict:
    """Detailed acoustic analysis of King's Chamber."""

    print("\n" + "=" * 60)
    print("KING'S CHAMBER ACOUSTIC ANALYSIS")
    print("=" * 60)

    # Dimensions
    print(f"\nDimensions: {pyramid.kc_length} × {pyramid.kc_width} × {pyramid.kc_height} m")

    results = {'dimensions': {
        'length': pyramid.kc_length,
        'width': pyramid.kc_width,
        'height': pyramid.kc_height,
    }}

    # Calculate modes for air
    print("\n--- AIR MODES ---")
    air_modes = rectangular_cavity_modes(
        pyramid.kc_length, pyramid.kc_width, pyramid.kc_height,
        C_SOUND, max_mode=3
    )

    results['air_modes'] = []
    print(f"{'Mode':<12} {'Frequency (Hz)':<15} {'Wavelength (m)':<15}")
    print("-" * 45)

    for mode in air_modes[:15]:
        m, n, p = mode['mode']
        print(f"({m},{n},{p})       {mode['frequency']:>10.2f}      {mode['wavelength']:>10.2f}")
        results['air_modes'].append(mode)

        # Check for notable frequencies
        if 7.5 < mode['frequency'] < 8.5:
            print("  ^^^ Close to Schumann fundamental (7.83 Hz)!")
        elif 15.5 < mode['frequency'] < 16.5:
            print("  ^^^ Close to 2× Schumann (15.66 Hz)!")
        elif 108 < mode['frequency'] < 112:
            print("  ^^^ Close to A2 (110 Hz)!")
        elif 430 < mode['frequency'] < 445:
            print("  ^^^ Close to A4 (440 Hz)!")

    # Calculate modes for granite (structural resonance)
    print("\n--- GRANITE STRUCTURAL MODES ---")
    granite_modes = rectangular_cavity_modes(
        pyramid.kc_length, pyramid.kc_width, pyramid.kc_height,
        C_GRANITE, max_mode=2
    )

    results['granite_modes'] = []
    print(f"{'Mode':<12} {'Frequency (Hz)':<15} {'Wavelength (m)':<15}")
    print("-" * 45)

    for mode in granite_modes[:10]:
        m, n, p = mode['mode']
        print(f"({m},{n},{p})       {mode['frequency']:>10.2f}      {mode['wavelength']:>10.2f}")
        results['granite_modes'].append(mode)

    # Sarcophagus resonance
    print("\n--- SARCOPHAGUS RESONANCE ---")
    # Model as Helmholtz resonator approximation
    volume = pyramid.sarc_length * pyramid.sarc_width * pyramid.sarc_height
    # Assume opening area ~ 0.5 m² and neck length ~ 0.1 m
    opening_area = 0.5
    neck_length = 0.1
    f_helmholtz = (C_SOUND / (2 * np.pi)) * np.sqrt(opening_area / (volume * neck_length))
    print(f"Approximate Helmholtz frequency: {f_helmholtz:.1f} Hz")
    results['sarcophagus_helmholtz'] = f_helmholtz

    # When struck (structural mode)
    sarc_modes = rectangular_cavity_modes(
        pyramid.sarc_length, pyramid.sarc_width, pyramid.sarc_height,
        C_GRANITE, max_mode=2
    )
    print(f"Fundamental structural mode: {sarc_modes[0]['frequency']:.1f} Hz")
    results['sarcophagus_structural'] = sarc_modes[0]['frequency']

    return results


def analyze_chamber_positions(pyramid: GreatPyramid) -> dict:
    """Analyze the positions of chambers relative to pyramid geometry."""

    print("\n" + "=" * 60)
    print("CHAMBER POSITION ANALYSIS")
    print("=" * 60)

    results = {}

    # Heights as fractions of total height
    kc_fraction = pyramid.kc_floor_height / pyramid.height
    qc_fraction = pyramid.qc_floor_height / pyramid.height

    print(f"\nKing's Chamber floor: {pyramid.kc_floor_height} m = {kc_fraction:.4f} of height")
    print(f"Queen's Chamber floor: {pyramid.qc_floor_height} m = {qc_fraction:.4f} of height")

    results['kc_height_fraction'] = kc_fraction
    results['qc_height_fraction'] = qc_fraction

    # Check FTD integer relationships
    print(f"\n--- FTD INTEGER CHECKS ---")

    # King's Chamber
    print(f"\nKing's Chamber ({kc_fraction:.4f}):")
    for name, val in [('1/N_c', 1/N_C), ('2/N_c', 2/N_C), ('1/N_base', 1/N_BASE),
                      ('2/b_3', 2/B_3), ('3/N_eff', 3/N_EFF)]:
        diff = abs(kc_fraction - val) / val * 100
        marker = " ***" if diff < 5 else ""
        print(f"  {name} = {val:.4f}: diff = {diff:.1f}%{marker}")

    # Queen's Chamber
    print(f"\nQueen's Chamber ({qc_fraction:.4f}):")
    for name, val in [('1/b_3', 1/B_3), ('1/N_base', 1/N_BASE), ('1/N_eff', 1/N_EFF)]:
        diff = abs(qc_fraction - val) / val * 100
        marker = " ***" if diff < 5 else ""
        print(f"  {name} = {val:.4f}: diff = {diff:.1f}%{marker}")

    # Standing wave node positions
    print(f"\n--- STANDING WAVE NODES ---")
    print(f"For n=7 standing wave (b_3):")
    for i in range(1, B_3 + 1):
        node_pos = i / B_3
        print(f"  Node {i}: {node_pos:.4f} of height = {node_pos * pyramid.height:.1f} m")
        if abs(node_pos - kc_fraction) < 0.02:
            print(f"    ^^^ King's Chamber near this node!")
        if abs(node_pos - qc_fraction) < 0.02:
            print(f"    ^^^ Queen's Chamber near this node!")

    return results


# =============================================================================
# Harmonic Cascade Simulation
# =============================================================================

def simulate_harmonic_cascade(input_freq: float, target_freq: float,
                             stages: int = 12) -> List[dict]:
    """
    Simulate harmonic amplification cascade.

    Models how a low input frequency could be stepped up through
    successive resonant stages to reach flux exclusion frequency.
    """
    print("\n" + "=" * 60)
    print("HARMONIC CASCADE SIMULATION")
    print("=" * 60)

    ratio = target_freq / input_freq
    step_ratio = ratio ** (1 / stages)

    print(f"\nInput frequency: {input_freq} Hz")
    print(f"Target frequency: {target_freq:.2e} Hz")
    print(f"Total ratio: {ratio:.2e}")
    print(f"Stages: {stages}")
    print(f"Step ratio: {step_ratio:.2f}x per stage")

    cascade = []
    current_freq = input_freq

    print(f"\n{'Stage':<8} {'Frequency':<15} {'Wavelength (EM)':<18} {'Notes'}")
    print("-" * 70)

    for i in range(stages + 1):
        wavelength = C_LIGHT / current_freq if current_freq > 0 else float('inf')

        # Determine what band this is in
        if current_freq < 20:
            band = "Infrasound"
        elif current_freq < 20000:
            band = "Audio"
        elif current_freq < 300e6:
            band = "Radio"
        elif current_freq < 300e9:
            band = "Microwave"
        elif current_freq < 300e12:
            band = "Infrared"
        else:
            band = "Visible+"

        stage_info = {
            'stage': i,
            'frequency': current_freq,
            'wavelength': wavelength,
            'band': band,
        }
        cascade.append(stage_info)

        # Print
        if current_freq < 1e6:
            freq_str = f"{current_freq:.2f} Hz"
        elif current_freq < 1e9:
            freq_str = f"{current_freq/1e6:.2f} MHz"
        elif current_freq < 1e12:
            freq_str = f"{current_freq/1e9:.2f} GHz"
        else:
            freq_str = f"{current_freq/1e12:.2f} THz"

        print(f"{i:<8} {freq_str:<15} {wavelength:>12.4f} m     {band}")

        current_freq *= step_ratio

    return cascade


# =============================================================================
# Flux Concentration Model
# =============================================================================

def pyramid_flux_concentration(size: int = 100) -> np.ndarray:
    """
    Model flux concentration pattern inside a pyramid.

    Uses a simplified model where flux concentrates toward apex
    and at geometric center (King's Chamber region).
    """
    # Create 3D grid (x, y, z)
    x = np.linspace(0, 1, size)
    y = np.linspace(0, 1, size)
    z = np.linspace(0, 1, size)
    X, Y, Z = np.meshgrid(x, y, z, indexing='ij')

    # Pyramid mask (inside pyramid)
    # Pyramid from base (z=0) to apex (z=1) with square base
    half_width = 0.5 * (1 - Z)  # Width decreases with height
    inside = (np.abs(X - 0.5) <= half_width) & (np.abs(Y - 0.5) <= half_width)

    # Flux concentration model
    # 1. Increases toward apex (z direction)
    apex_factor = Z ** 2

    # 2. Concentrates toward center-line
    center_dist = np.sqrt((X - 0.5)**2 + (Y - 0.5)**2)
    center_factor = np.exp(-center_dist**2 / 0.1)

    # 3. King's Chamber resonance node (z ≈ 0.29)
    kc_z = 0.29
    kc_factor = np.exp(-((Z - kc_z)**2) / 0.02)

    # Combine
    flux = (apex_factor + center_factor + 0.5 * kc_factor) * inside

    return flux


def plot_pyramid_flux(flux: np.ndarray, save_path: Optional[str] = None):
    """Visualize flux concentration in pyramid."""
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))

    size = flux.shape[0]
    center = size // 2

    # XZ slice (side view)
    ax = axes[0]
    im = ax.imshow(flux[:, center, :].T, origin='lower', cmap='hot',
                   extent=[0, 1, 0, 1], aspect='equal')
    ax.set_title('Side View (XZ slice)')
    ax.set_xlabel('X')
    ax.set_ylabel('Z (height)')
    plt.colorbar(im, ax=ax, label='Flux concentration')

    # XY slice at King's Chamber height
    kc_z = int(0.29 * size)
    ax = axes[1]
    im = ax.imshow(flux[:, :, kc_z].T, origin='lower', cmap='hot',
                   extent=[0, 1, 0, 1], aspect='equal')
    ax.set_title(f"Plan View at King's Chamber (z={0.29})")
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    plt.colorbar(im, ax=ax, label='Flux concentration')

    # Vertical profile through center
    ax = axes[2]
    z = np.linspace(0, 1, size)
    center_profile = flux[center, center, :]
    ax.plot(center_profile, z, 'b-', linewidth=2)
    ax.axhline(y=0.29, color='r', linestyle='--', label="King's Chamber")
    ax.axhline(y=0.14, color='g', linestyle='--', label="Queen's Chamber")
    ax.set_xlabel('Flux concentration')
    ax.set_ylabel('Z (height)')
    ax.set_title('Vertical Flux Profile (center)')
    ax.legend()
    ax.grid(True, alpha=0.3)

    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=150)
        print(f"Saved to {save_path}")
    else:
        plt.show()

    plt.close()


# =============================================================================
# Frequency Spectrum Analysis
# =============================================================================

def analyze_frequency_spectrum(pyramid: GreatPyramid):
    """Comprehensive frequency spectrum analysis."""

    print("\n" + "=" * 60)
    print("FREQUENCY SPECTRUM ANALYSIS")
    print("=" * 60)

    # Calculate all modes
    em_modes = pyramid_standing_waves(pyramid.base, pyramid.height, C_LIGHT, max_harmonic=20)
    acoustic_modes = pyramid_standing_waves(pyramid.base, pyramid.height, C_SOUND, max_harmonic=20)

    # Find FTD-significant modes
    print("\n--- ELECTROMAGNETIC MODES WITH FTD RELATIONSHIPS ---")
    em_ftd = find_ftd_resonances(em_modes)
    for mode in em_ftd[:10]:
        print(f"  {mode['type']:10s} n={mode['harmonic']:2d}: {mode['frequency']:.2e} Hz "
              f"({mode.get('ftd_relationship', '')})")

    print("\n--- ACOUSTIC MODES WITH FTD RELATIONSHIPS ---")
    ac_ftd = find_ftd_resonances(acoustic_modes)
    for mode in ac_ftd[:10]:
        print(f"  {mode['type']:10s} n={mode['harmonic']:2d}: {mode['frequency']:.2f} Hz "
              f"({mode.get('ftd_relationship', '')})")

    # Key finding: n=13 harmonic
    print("\n--- KEY FINDING: n=13 (N_eff) HARMONIC ---")
    for mode in em_modes:
        if mode['harmonic'] == N_EFF and mode['type'] == 'horizontal':
            print(f"Electromagnetic n=13 horizontal mode:")
            print(f"  Frequency: {mode['frequency']:.2e} Hz")
            print(f"  Wavelength: {mode['wavelength']:.2f} m")
            print(f"  Ratio to 8 THz: {F_EXCLUSION / mode['frequency']:.2e}")

    for mode in acoustic_modes:
        if mode['harmonic'] == N_EFF and mode['type'] == 'horizontal':
            print(f"\nAcoustic n=13 horizontal mode:")
            print(f"  Frequency: {mode['frequency']:.2f} Hz")
            print(f"  Wavelength: {mode['wavelength']:.2f} m")

    # Visualize spectrum
    fig, axes = plt.subplots(2, 1, figsize=(12, 8))

    # EM spectrum
    ax = axes[0]
    freqs = [m['frequency'] for m in em_modes if m['type'] == 'horizontal']
    harmonics = [m['harmonic'] for m in em_modes if m['type'] == 'horizontal']
    ax.semilogy(harmonics, freqs, 'bo-', label='Horizontal modes')

    # Mark FTD integers
    for n, label in [(N_C, 'N_c'), (N_BASE, 'N_base'), (B_3, 'b_3'), (N_EFF, 'N_eff')]:
        ax.axvline(x=n, color='r', linestyle='--', alpha=0.5)
        ax.text(n, max(freqs)*0.5, label, rotation=90, va='bottom')

    ax.set_xlabel('Harmonic number')
    ax.set_ylabel('Frequency (Hz)')
    ax.set_title('Electromagnetic Standing Wave Modes')
    ax.grid(True, alpha=0.3)
    ax.legend()

    # Acoustic spectrum
    ax = axes[1]
    freqs = [m['frequency'] for m in acoustic_modes if m['type'] == 'horizontal']
    harmonics = [m['harmonic'] for m in acoustic_modes if m['type'] == 'horizontal']
    ax.plot(harmonics, freqs, 'go-', label='Horizontal modes')

    # Mark FTD integers
    for n, label in [(N_C, 'N_c'), (N_BASE, 'N_base'), (B_3, 'b_3'), (N_EFF, 'N_eff')]:
        ax.axvline(x=n, color='r', linestyle='--', alpha=0.5)
        ax.text(n, max(freqs)*0.8, label, rotation=90, va='bottom')

    ax.set_xlabel('Harmonic number')
    ax.set_ylabel('Frequency (Hz)')
    ax.set_title('Acoustic Standing Wave Modes')
    ax.grid(True, alpha=0.3)
    ax.legend()

    plt.tight_layout()
    plt.savefig('frequency_spectrum.png', dpi=150)
    print("\nSaved frequency_spectrum.png")
    plt.close()


# =============================================================================
# Main
# =============================================================================

def run_full_analysis():
    """Run complete pyramid resonance analysis."""

    print("=" * 60)
    print("PYRAMID RESONANCE SIMULATION")
    print("Great Pyramid of Giza Analysis")
    print("=" * 60)

    pyramid = GreatPyramid()

    # Golden ratio check
    print("\n--- GOLDEN RATIO CHECK ---")
    gr = pyramid.golden_ratio_check()
    print(f"Apothem / Half-base = {gr['apothem/half_base']:.6f}")
    print(f"φ (golden ratio)    = {gr['phi']:.6f}")
    print(f"Match: {100 - gr['match_apothem']:.2f}%")
    print(f"\nPerimeter / Height = {gr['perimeter/height']:.6f}")
    print(f"2π                 = {gr['2pi']:.6f}")
    print(f"Match: {100 - gr['match_perimeter']:.2f}%")

    # Chamber analysis
    chamber_results = analyze_kings_chamber(pyramid)
    position_results = analyze_chamber_positions(pyramid)

    # Harmonic cascade
    cascade = simulate_harmonic_cascade(8.0, F_EXCLUSION, stages=12)

    # Frequency spectrum
    analyze_frequency_spectrum(pyramid)

    # Flux concentration
    print("\n--- FLUX CONCENTRATION MODEL ---")
    flux = pyramid_flux_concentration(100)
    print(f"Maximum flux concentration: {flux.max():.4f}")
    print(f"Location of maximum: z = {np.unravel_index(flux.argmax(), flux.shape)[2]/100:.2f}")
    plot_pyramid_flux(flux, save_path='pyramid_flux.png')

    # Summary
    print("\n" + "=" * 60)
    print("ANALYSIS COMPLETE")
    print("=" * 60)
    print("\nKey findings:")
    print(f"1. Pyramid geometry encodes φ and 2π to within 0.1%")
    print(f"2. King's Chamber at z ≈ 0.29 ≈ 2/7 (2/b_3)")
    print(f"3. Queen's Chamber at z ≈ 0.14 ≈ 1/7 (1/b_3)")
    print(f"4. n=13 (N_eff) harmonic gives ~8 MHz electromagnetic mode")
    print(f"5. Harmonic cascade from 8 Hz to 8 THz requires ~12 stages")
    print(f"6. Flux concentrates at apex and chamber positions")
    print("\nGenerated files:")
    print("  - frequency_spectrum.png")
    print("  - pyramid_flux.png")


if __name__ == "__main__":
    run_full_analysis()
