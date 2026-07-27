#!/usr/bin/env python3
"""FTD-0558 corrected moving-source pole verifier.

This script deliberately does not reproduce the retracted FTD-0115 formula.
It verifies the production discrete-time 18-point response, canonical crystal
momentum wrapping, and the positive phase-speed bound.  It is an independent
Python cross-check of the C++ observer, not a physical radiation calculation.
"""

from __future__ import annotations

import cmath
import math


C_WAVE = 1.0 / math.sqrt(3.0)
C2 = C_WAVE * C_WAVE
TOL = 1.0e-12


def full_symbol(kx: float, ky: float, kz: float) -> float:
    cx, cy, cz = math.cos(kx), math.cos(ky), math.cos(kz)
    return 4.0 - (2.0 / 3.0) * (cx + cy + cz) - (2.0 / 3.0) * (
        cx * cy + cx * cz + cy * cz
    )


def phase(symbol: float) -> float:
    return 2.0 * math.asin(0.5 * C_WAVE * math.sqrt(symbol))


def canonical_mode(n: int, size: int) -> int:
    reduced = n % size
    if 2 * reduced >= size:
        reduced -= size
    return reduced


def driven_denominator(symbol: float, omega: float) -> float:
    return C2 * symbol - 4.0 * math.sin(0.5 * omega) ** 2


def verify_driven_response() -> float:
    registered = (
        (17, 1, -2, 3),
        (19, -3, 2, 1),
        (23, 4, 1, -2),
        (29, -2, -3, 1),
    )
    worst = 0.0
    arms = 0
    for size, nx, ny, nz in registered:
        scale = 2.0 * math.pi / size
        symbol = full_symbol(scale * nx, scale * ny, scale * nz)
        a = C2 * symbol
        for omega in (0.17, 0.41, 0.73):
            z = cmath.exp(-1j * omega)
            determinant = z * z - (2.0 - a) * z + 1.0
            direct = z / determinant
            closed = 1.0 / driven_denominator(symbol, omega)
            residual = abs(direct - closed)
            static_residual = abs(1.0 / a - 1.0 / driven_denominator(symbol, 0.0))
            worst = max(worst, residual, static_residual)
            arms += 1
    assert arms == 12
    assert worst <= TOL
    return worst


def verify_alias() -> tuple[float, float, float]:
    size = 16
    old_n = 15
    wrapped_n = canonical_mode(old_n, size)
    assert wrapped_n == -1
    scale = 2.0 * math.pi / size
    old_symbol = full_symbol(scale * old_n, 0.0, 0.0)
    wrapped_symbol = full_symbol(scale * wrapped_n, 0.0, 0.0)
    old_phase = phase(old_symbol)
    wrapped_phase = phase(wrapped_symbol)
    symbol_residual = abs(old_symbol - wrapped_symbol)
    phase_residual = abs(old_phase - wrapped_phase)
    ratio = (wrapped_phase / abs(scale * wrapped_n)) / (
        old_phase / abs(scale * old_n)
    )
    assert symbol_residual <= TOL
    assert phase_residual <= TOL
    assert ratio > 10.0
    return symbol_residual, phase_residual, ratio


def enumerate_threshold(size: int, direction: tuple[int, int, int]) -> float:
    norm = math.sqrt(sum(component * component for component in direction))
    scale = 2.0 * math.pi / size
    minimum = math.inf
    for nx in range(-size // 2, size // 2):
        for ny in range(-size // 2, size // 2):
            for nz in range(-size // 2, size // 2):
                if nx == ny == nz == 0:
                    continue
                kx, ky, kz = scale * nx, scale * ny, scale * nz
                projected = abs(kx * direction[0] + ky * direction[1] + kz * direction[2]) / norm
                if projected <= 1.0e-15:
                    continue
                minimum = min(minimum, phase(full_symbol(kx, ky, kz)) / projected)
    return minimum


def main() -> None:
    response_residual = verify_driven_response()
    symbol_residual, phase_residual, alias_ratio = verify_alias()
    universal_floor = 2.0 * C_WAVE / (math.pi * math.sqrt(3.0))
    seven_point_floor = 2.0 / math.pi
    thresholds = [
        enumerate_threshold(size, direction)
        for size in (16, 32, 64)
        for direction in ((1, 0, 0), (1, 1, 0), (1, 1, 1))
    ]
    assert len(thresholds) == 9
    assert min(thresholds) >= universal_floor - TOL
    print("FTD-0558 corrected production moving-source pole: PASS")
    print(f"driven_response_residual={response_residual:.17g}")
    print(f"seven_point_ratio_floor={seven_point_floor:.17g}")
    print(f"universal_production_speed_floor={universal_floor:.17g}")
    print(f"minimum_enumerated_phase_speed={min(thresholds):.17g}")
    print(f"alias_symbol_residual={symbol_residual:.17g}")
    print(f"alias_phase_residual={phase_residual:.17g}")
    print(f"wrapped_to_old_alias_ratio={alias_ratio:.17g}")
    print("verdict=NATIVE_MOVING_SOURCE_POLE_CORRECTED")


if __name__ == "__main__":
    main()
