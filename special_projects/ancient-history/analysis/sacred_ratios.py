#!/usr/bin/env python3
"""
Sacred Ratios Analysis

Examines mathematical constants encoded in ancient architecture,
art, and texts, comparing them to FTD-derived values.

This is exploratory research code - results are suggestive, not conclusive.
"""

import math
from dataclasses import dataclass
from typing import Optional


# =============================================================================
# FTD Framework Constants
# =============================================================================

# Fundamental integers
N_C = 3          # Color charges
N_BASE = 4       # Spacetime dimensions
B_3 = 7          # QCD beta coefficient
N_EFF = 13       # Effective degrees of freedom

# Derived constants
PHI = (1 + math.sqrt(5)) / 2  # Golden ratio
ALPHA_INV = 137.036           # Fine structure constant inverse
G_STAR = 2.9587               # Lemniscatic constant

# Mathematical constants
PI = math.pi
E = math.e
SQRT_2 = math.sqrt(2)
SQRT_3 = math.sqrt(3)
SQRT_5 = math.sqrt(5)


# =============================================================================
# Sacred Measurements
# =============================================================================

@dataclass
class SacredMeasurement:
    """A measurement from an ancient source."""
    name: str
    value: float
    source: str
    unit: str
    notes: Optional[str] = None


# Great Pyramid of Giza ratios
PYRAMID_MEASUREMENTS = [
    SacredMeasurement(
        name="Height/Base ratio",
        value=146.5 / 230.4,  # meters
        source="Great Pyramid",
        unit="ratio",
        notes="Often cited as encoding phi"
    ),
    SacredMeasurement(
        name="Apothem/Half-base",
        value=186.4 / 115.2,  # meters (approximate)
        source="Great Pyramid",
        unit="ratio",
        notes="Close to phi = 1.618..."
    ),
    SacredMeasurement(
        name="Perimeter/Height",
        value=(4 * 230.4) / 146.5,
        source="Great Pyramid",
        unit="ratio",
        notes="Close to 2*pi = 6.283..."
    ),
]

# Parthenon golden ratios
PARTHENON_MEASUREMENTS = [
    SacredMeasurement(
        name="Width/Height",
        value=30.88 / 19.0,  # meters (approximate facade)
        source="Parthenon",
        unit="ratio",
        notes="Golden rectangle proportion"
    ),
]

# Chartres Cathedral
CHARTRES_MEASUREMENTS = [
    SacredMeasurement(
        name="Nave length/width",
        value=130.2 / 16.4,  # meters (approximate)
        source="Chartres Cathedral",
        unit="ratio",
        notes="Multiple of 7 and 8"
    ),
]


# =============================================================================
# Gematria Analysis
# =============================================================================

# Hebrew letter values
HEBREW_GEMATRIA = {
    'aleph': 1, 'bet': 2, 'gimel': 3, 'dalet': 4, 'he': 5,
    'vav': 6, 'zayin': 7, 'chet': 8, 'tet': 9, 'yod': 10,
    'kaf': 20, 'lamed': 30, 'mem': 40, 'nun': 50, 'samech': 60,
    'ayin': 70, 'pe': 80, 'tzadi': 90, 'qof': 100, 'resh': 200,
    'shin': 300, 'tav': 400
}

# Key Hebrew words and their values
HEBREW_WORDS = {
    'YHWH': 10 + 5 + 6 + 5,           # = 26
    'Qabalah': 100 + 2 + 30 + 5,      # = 137
    'El': 1 + 30,                      # = 31
    'Echad': 1 + 8 + 4,               # = 13 (One)
    'Ahavah': 1 + 5 + 2 + 5,          # = 13 (Love)
}

# Greek letter values (isopsephy)
GREEK_ISOPSEPHY = {
    'alpha': 1, 'beta': 2, 'gamma': 3, 'delta': 4, 'epsilon': 5,
    'zeta': 7, 'eta': 8, 'theta': 9, 'iota': 10, 'kappa': 20,
    'lambda': 30, 'mu': 40, 'nu': 50, 'xi': 60, 'omicron': 70,
    'pi': 80, 'rho': 100, 'sigma': 200, 'tau': 300, 'upsilon': 400,
    'phi': 500, 'chi': 600, 'psi': 700, 'omega': 800
}


# =============================================================================
# Analysis Functions
# =============================================================================

def compare_to_constants(value: float, label: str = "") -> dict:
    """Compare a value to known mathematical/physical constants."""

    constants = {
        'phi': PHI,
        '2*pi': 2 * PI,
        'pi': PI,
        'e': E,
        'sqrt(2)': SQRT_2,
        'sqrt(3)': SQRT_3,
        'sqrt(5)': SQRT_5,
        '1/alpha': ALPHA_INV,
        'G*': G_STAR,
        'N_c': N_C,
        'N_base': N_BASE,
        'b_3': B_3,
        'N_eff': N_EFF,
    }

    results = {}
    for name, const in constants.items():
        if const != 0:
            ratio = value / const
            diff_pct = abs(ratio - 1) * 100
            results[name] = {
                'ratio': ratio,
                'diff_pct': diff_pct,
                'match': diff_pct < 1.0  # Within 1%
            }

    return results


def analyze_measurement(m: SacredMeasurement) -> None:
    """Analyze a single sacred measurement."""
    print(f"\n{'='*60}")
    print(f"Measurement: {m.name}")
    print(f"Source: {m.source}")
    print(f"Value: {m.value:.6f} {m.unit}")
    if m.notes:
        print(f"Notes: {m.notes}")

    results = compare_to_constants(m.value, m.name)

    # Find best matches
    matches = [(k, v) for k, v in results.items() if v['match']]

    if matches:
        print("\nClose matches (< 1% difference):")
        for name, data in sorted(matches, key=lambda x: x[1]['diff_pct']):
            print(f"  {name}: ratio = {data['ratio']:.6f}, diff = {data['diff_pct']:.3f}%")
    else:
        # Show closest match
        closest = min(results.items(), key=lambda x: x[1]['diff_pct'])
        print(f"\nClosest match: {closest[0]} (diff = {closest[1]['diff_pct']:.2f}%)")


def analyze_gematria() -> None:
    """Analyze Hebrew gematria for FTD connections."""
    print("\n" + "="*60)
    print("GEMATRIA ANALYSIS")
    print("="*60)

    for word, value in HEBREW_WORDS.items():
        print(f"\n{word} = {value}")

        # Check against FTD integers
        if value == N_C:
            print(f"  -> Matches N_c (color charges)")
        elif value == N_BASE:
            print(f"  -> Matches N_base (dimensions)")
        elif value == B_3:
            print(f"  -> Matches b_3 (QCD beta)")
        elif value == N_EFF:
            print(f"  -> Matches N_eff (degrees of freedom)")
        elif value == 2 * N_EFF:
            print(f"  -> Matches 2*N_eff = 26 (Moore neighborhood)")
        elif value == int(ALPHA_INV):
            print(f"  -> Matches 1/alpha = 137 (fine structure)")

        # Check for interesting factorizations
        for i in range(2, value):
            if value % i == 0:
                factor = value // i
                if factor in [N_C, N_BASE, B_3, N_EFF]:
                    print(f"  -> {value} = {i} × {factor}")


def fibonacci_analysis() -> None:
    """Analyze Fibonacci numbers and FTD integers."""
    print("\n" + "="*60)
    print("FIBONACCI SEQUENCE AND FTD")
    print("="*60)

    fib = [1, 1]
    for i in range(2, 15):
        fib.append(fib[-1] + fib[-2])

    ftd_integers = {N_C, N_BASE, B_3, N_EFF}

    for i, f in enumerate(fib, 1):
        mark = "***" if f in ftd_integers else ""
        print(f"F_{i} = {f:4d} {mark}")


def run_analysis() -> None:
    """Run complete sacred ratios analysis."""
    print("="*60)
    print("SACRED RATIOS ANALYSIS")
    print("FTD Framework Comparison")
    print("="*60)

    # Analyze architectural measurements
    print("\n--- ARCHITECTURAL MEASUREMENTS ---")

    all_measurements = (
        PYRAMID_MEASUREMENTS +
        PARTHENON_MEASUREMENTS +
        CHARTRES_MEASUREMENTS
    )

    for m in all_measurements:
        analyze_measurement(m)

    # Gematria analysis
    analyze_gematria()

    # Fibonacci analysis
    fibonacci_analysis()

    # Summary
    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    print("""
Key findings:
1. Great Pyramid encodes phi and 2*pi ratios
2. Hebrew YHWH = 26 = 2 × N_eff (Moore neighborhood)
3. Hebrew Qabalah = 137 = 1/alpha (fine structure constant)
4. Hebrew Echad (One) = Ahavah (Love) = 13 = N_eff
5. Fibonacci F_4 = 3 = N_c, F_7 = 13 = N_eff

These correspondences are suggestive but not conclusive.
They invite further investigation into whether ancient
traditions intuited fundamental physical constants.
""")


if __name__ == "__main__":
    run_analysis()
