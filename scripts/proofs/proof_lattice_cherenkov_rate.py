#!/usr/bin/env python3
"""FTD-0558 negative audit of the former lattice-Cherenkov power claim.

The archived script counted modes near an incorrectly constructed pole and
used 1/|chi| as a proxy; it did not compute its stated surface Jacobian or an
energy-normalized power.  This replacement verifies only the exact Floquet
spectrum of a prescribed periodic integer-hop source.  Radiation power remains
open.
"""

from __future__ import annotations

import cmath
import math


TOL = 1.0e-12


def floquet_coefficients(k_dot_d: float, period: int) -> list[complex]:
    numerator = 1.0 - cmath.exp(1j * k_dot_d)
    coefficients: list[complex] = []
    for harmonic in range(period):
        phase = (k_dot_d + 2.0 * math.pi * harmonic) / period
        denominator = period * (1.0 - cmath.exp(1j * phase))
        coefficients.append(numerator / denominator)
    return coefficients


def verify_schedule(k_dot_d: float, period: int) -> tuple[float, float, float]:
    coefficients = floquet_coefficients(k_dot_d, period)
    reconstruction = 0.0
    for remainder in range(period):
        actual = sum(
            coefficient * cmath.exp(-2j * math.pi * harmonic * remainder / period)
            for harmonic, coefficient in enumerate(coefficients)
        )
        expected = cmath.exp(1j * k_dot_d * remainder / period)
        reconstruction = max(reconstruction, abs(actual - expected))
    parseval = abs(sum(abs(coefficient) ** 2 for coefficient in coefficients) - 1.0)
    nonfundamental = max(abs(value) for value in coefficients[1:])
    return reconstruction, parseval, nonfundamental


def main() -> None:
    worst_reconstruction = 0.0
    worst_parseval = 0.0
    maximum_nonfundamental = 0.0
    arms = 0
    for period in (4, 8, 16, 32):
        for numerator in (2, 4, 6):
            k_dot_d = numerator * math.pi / 17.0
            reconstruction, parseval, nonfundamental = verify_schedule(k_dot_d, period)
            assert reconstruction <= TOL
            assert parseval <= TOL
            assert nonfundamental > 1.0e-6
            worst_reconstruction = max(worst_reconstruction, reconstruction)
            worst_parseval = max(worst_parseval, parseval)
            maximum_nonfundamental = max(maximum_nonfundamental, nonfundamental)
            arms += 1
    assert arms == 12
    print("FTD-0558 integer-hop Floquet correction: PASS")
    print(f"floquet_schedule_arms={arms}")
    print(f"maximum_reconstruction_residual={worst_reconstruction:.17g}")
    print(f"maximum_parseval_residual={worst_parseval:.17g}")
    print(f"maximum_nonfundamental_amplitude={maximum_nonfundamental:.17g}")
    print("radiation_power_status=OPEN_NOT_COMPUTED")
    print("verdict=FORMER_CHERENKOV_POWER_CLAIM_RETRACTED")


if __name__ == "__main__":
    main()
