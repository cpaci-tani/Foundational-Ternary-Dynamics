#!/usr/bin/env python3
"""
Pyramid Resonance Analysis

Calculates resonant frequencies of the Great Pyramid and compares
them to FTD-significant frequencies (harmonics of 8 THz).

This is speculative exploration, not verified physics.
"""

import math
from dataclasses import dataclass
from typing import List, Tuple


# =============================================================================
# Physical Constants
# =============================================================================

C = 299_792_458  # Speed of light (m/s)
SCHUMANN_FUNDAMENTAL = 7.83  # Hz (Earth's resonance)

# FTD Key Frequency
F_EXCLUSION = 8e12  # 8 THz (flux exclusion frequency from antigravity project)

# FTD Integers
N_C = 3
N_BASE = 4
B_3 = 7
N_EFF = 13


# =============================================================================
# Great Pyramid Dimensions
# =============================================================================

@dataclass
class PyramidDimensions:
    """Dimensions of the Great Pyramid of Giza."""
    base_length: float = 230.4  # meters
    height: float = 146.5  # meters
    apothem: float = 186.4  # meters (face slant height)

    # Internal chambers
    kings_chamber_length: float = 10.47  # meters
    kings_chamber_width: float = 5.23  # meters
    kings_chamber_height: float = 5.81  # meters

    queens_chamber_length: float = 5.76  # meters
    queens_chamber_width: float = 5.23  # meters
    queens_chamber_height: float = 6.26  # meters (to apex)


# =============================================================================
# Resonance Calculations
# =============================================================================

def wavelength_to_frequency(wavelength: float) -> float:
    """Convert wavelength (meters) to frequency (Hz)."""
    return C / wavelength


def frequency_to_wavelength(frequency: float) -> float:
    """Convert frequency (Hz) to wavelength (meters)."""
    return C / frequency


def cavity_resonant_frequencies(length: float, width: float, height: float,
                                max_modes: int = 5) -> List[Tuple[int, int, int, float]]:
    """
    Calculate resonant frequencies of a rectangular cavity.

    Returns list of (m, n, p, frequency) tuples for modes.
    Based on: f = (c/2) * sqrt((m/L)² + (n/W)² + (p/H)²)
    """
    modes = []
    for m in range(max_modes + 1):
        for n in range(max_modes + 1):
            for p in range(max_modes + 1):
                if m == 0 and n == 0 and p == 0:
                    continue  # Skip null mode

                freq = (C / 2) * math.sqrt(
                    (m / length)**2 + (n / width)**2 + (p / height)**2
                )
                modes.append((m, n, p, freq))

    return sorted(modes, key=lambda x: x[3])


def acoustic_cavity_frequencies(length: float, width: float, height: float,
                                speed_of_sound: float = 343.0,
                                max_modes: int = 5) -> List[Tuple[int, int, int, float]]:
    """
    Calculate acoustic resonant frequencies of a room.
    """
    modes = []
    for m in range(max_modes + 1):
        for n in range(max_modes + 1):
            for p in range(max_modes + 1):
                if m == 0 and n == 0 and p == 0:
                    continue

                freq = (speed_of_sound / 2) * math.sqrt(
                    (m / length)**2 + (n / width)**2 + (p / height)**2
                )
                modes.append((m, n, p, freq))

    return sorted(modes, key=lambda x: x[3])


def pyramid_standing_waves(base: float, height: float,
                           max_harmonic: int = 30) -> List[Tuple[int, float, float]]:
    """
    Calculate standing wave frequencies for pyramid structure.

    Models pyramid as creating standing waves where:
    - Base length = n * λ/2 (horizontal)
    - Height = m * λ/2 (vertical)

    Returns (harmonic_n, wavelength, frequency) tuples.
    """
    results = []

    # Horizontal modes (base)
    for n in range(1, max_harmonic + 1):
        wavelength = 2 * base / n
        freq = wavelength_to_frequency(wavelength)
        results.append((n, wavelength, freq))

    return results


def find_ftd_harmonics(frequency: float) -> dict:
    """
    Check if a frequency is a harmonic/subharmonic of 8 THz.
    """
    ratio = F_EXCLUSION / frequency
    log_ratio = math.log10(ratio)

    # Check for power-of-10 relationships
    nearest_power = round(log_ratio)
    deviation = abs(log_ratio - nearest_power)

    # Check for FTD integer relationships
    ftd_ratios = {
        'N_c': N_C,
        'N_base': N_BASE,
        'b_3': B_3,
        'N_eff': N_EFF,
        '2^N_c': 2**N_C,
    }

    results = {
        'frequency': frequency,
        'ratio_to_8THz': ratio,
        'log10_ratio': log_ratio,
        'nearest_power': nearest_power,
        'deviation_from_power': deviation,
        'is_clean_harmonic': deviation < 0.05,
        'ftd_matches': {}
    }

    for name, value in ftd_ratios.items():
        if abs(ratio / value - round(ratio / value)) < 0.1:
            results['ftd_matches'][name] = ratio / value

    return results


# =============================================================================
# Analysis Functions
# =============================================================================

def analyze_pyramid():
    """Comprehensive pyramid resonance analysis."""

    pyramid = PyramidDimensions()

    print("=" * 70)
    print("GREAT PYRAMID RESONANCE ANALYSIS")
    print("=" * 70)

    # Basic geometry
    print("\n--- GEOMETRY ---")
    print(f"Base: {pyramid.base_length} m")
    print(f"Height: {pyramid.height} m")
    print(f"Height/Base ratio: {pyramid.height/pyramid.base_length:.4f}")
    print(f"  (Compare to 2/π = {2/math.pi:.4f})")
    print(f"Apothem/Half-base: {pyramid.apothem/(pyramid.base_length/2):.4f}")
    print(f"  (Compare to φ = {(1+math.sqrt(5))/2:.4f})")
    print(f"Perimeter/Height: {4*pyramid.base_length/pyramid.height:.4f}")
    print(f"  (Compare to 2π = {2*math.pi:.4f})")

    # Standing waves
    print("\n--- STANDING WAVES (Base Length) ---")
    print(f"{'Harmonic':<10} {'Wavelength (m)':<18} {'Frequency':<15} {'8 THz Ratio'}")
    print("-" * 60)

    waves = pyramid_standing_waves(pyramid.base_length, pyramid.height)
    interesting_harmonics = [1, N_C, N_BASE, B_3, N_EFF, 2*N_EFF]

    for n, wavelength, freq in waves:
        if n in interesting_harmonics or n <= 5:
            ratio = F_EXCLUSION / freq
            log_ratio = math.log10(ratio)
            mark = " ***" if n in [N_EFF, 2*N_EFF] else ""
            print(f"n={n:<8} {wavelength:>14.2f} m   {freq:>12.2e} Hz  10^{log_ratio:.1f}{mark}")

    # King's Chamber acoustics
    print("\n--- KING'S CHAMBER ACOUSTIC MODES ---")
    print(f"Dimensions: {pyramid.kings_chamber_length} x {pyramid.kings_chamber_width} x {pyramid.kings_chamber_height} m")

    acoustic_modes = acoustic_cavity_frequencies(
        pyramid.kings_chamber_length,
        pyramid.kings_chamber_width,
        pyramid.kings_chamber_height,
        max_modes=3
    )

    print(f"\n{'Mode (m,n,p)':<15} {'Frequency (Hz)':<15} {'Note'}")
    print("-" * 50)

    for m, n, p, freq in acoustic_modes[:15]:
        note = ""
        if 7.5 < freq < 8.5:
            note = "≈ Schumann!"
        elif 15.5 < freq < 16.5:
            note = "≈ 2 × Schumann"
        elif 110 < freq < 112:
            note = "≈ A2 (110 Hz)"
        print(f"({m},{n},{p})        {freq:>10.2f}       {note}")

    # Schumann relationship
    print("\n--- SCHUMANN RESONANCE CONNECTION ---")
    print(f"Earth fundamental: {SCHUMANN_FUNDAMENTAL} Hz")
    print(f"FTD exclusion frequency: {F_EXCLUSION:.0e} Hz = 8 THz")
    print(f"Ratio: {F_EXCLUSION/SCHUMANN_FUNDAMENTAL:.2e}")
    print(f"Log10 ratio: {math.log10(F_EXCLUSION/SCHUMANN_FUNDAMENTAL):.2f}")
    print(f"  (Close to 12, suggesting Schumann is 10^-12 × 8 THz)")

    # 8 Hz analysis
    print("\n--- 8 Hz ANALYSIS (Schumann-FTD Bridge) ---")
    f_8 = 8.0  # Hz
    ratio = F_EXCLUSION / f_8
    print(f"8 Hz to 8 THz ratio: {ratio:.0e}")
    print(f"  = 10^12 exactly")
    print(f"  = 1 trillion (T in THz)")
    print("\nThis suggests 8 Hz acoustic → 8 THz flux via 12 orders of magnitude.")
    print("Mechanism: harmonic amplification through resonant structure?")

    # FTD integer analysis
    print("\n--- FTD INTEGER APPEARANCES ---")
    print(f"Harmonic n={N_C} (N_c): color charges")
    print(f"Harmonic n={N_BASE} (N_base): spacetime dimensions")
    print(f"Harmonic n={B_3} (b_3): QCD beta coefficient")
    print(f"Harmonic n={N_EFF} (N_eff): effective degrees of freedom")
    print(f"Harmonic n={2*N_EFF} (2×N_eff): Moore neighborhood = 26")

    # The key finding
    print("\n" + "=" * 70)
    print("KEY FINDING")
    print("=" * 70)
    print("""
The Great Pyramid's n=13 harmonic (N_eff) produces:
  - Wavelength: ~35.4 m
  - Frequency: ~8.47 MHz

This is in the shortwave radio band.

To bridge to 8 THz requires ~10^6 multiplication.

Hypothesis: The pyramid's geometry creates a cascade of harmonic
amplification, stepping frequencies up through successive resonances
until they approach the flux exclusion threshold.

The piezoelectric granite in the King's Chamber may convert these
acoustic/EM oscillations into the appropriate flux modulation.

The operator's consciousness provides the coherent input signal,
amplified by group practice and geometric focusing.
    """)


def analyze_harmonics_table():
    """Print table of FTD-relevant harmonics."""

    print("\n" + "=" * 70)
    print("8 THz HARMONIC LADDER")
    print("=" * 70)
    print(f"\n{'Division':<15} {'Frequency':<15} {'Band':<20} {'Notes'}")
    print("-" * 70)

    harmonics = [
        (1, "8 THz", "Far Infrared", "Flux exclusion threshold"),
        (10**3, "8 GHz", "Microwave", "Radar, satellite comm"),
        (10**6, "8 MHz", "Shortwave radio", "Pyramid n=13 harmonic"),
        (10**9, "8 kHz", "Audio (high)", "Upper human hearing"),
        (10**12, "8 Hz", "Infrasound", "≈ Schumann resonance"),
    ]

    for div, freq, band, notes in harmonics:
        print(f"÷ {div:<12} {freq:<15} {band:<20} {notes}")

    print("\nThe 8 Hz → 8 THz ladder spans 12 orders of magnitude.")
    print("Ancient acoustic practices (chanting at ~8 Hz harmonics)")
    print("may have been the INPUT to a harmonic amplification system.")


if __name__ == "__main__":
    analyze_pyramid()
    analyze_harmonics_table()

    print("\n" + "=" * 70)
    print("DISCLAIMER")
    print("=" * 70)
    print("""
This analysis is SPECULATIVE. It explores whether:
1. Pyramid geometry creates significant resonances
2. Those resonances relate to FTD frequencies
3. A harmonic bridge could exist between acoustic and flux domains

No claims are made about historical fact.
This is hypothesis generation for future investigation.
    """)
