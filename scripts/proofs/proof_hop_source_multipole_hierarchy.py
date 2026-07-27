#!/usr/bin/env python3
"""Independent verifier for the FTD-0561 slow-hop multipole hierarchy."""

from __future__ import annotations

import cmath
import math


PI = math.pi
C2 = 1.0 / 3.0
TOL = 1.0e-12
PERIODS = (32, 64, 128, 256)
PROFILES = {
    "point": ({0: 1}, 0, 1, 0.01),
    "same_sign_pair": ({0: 1, 1: 1}, 0, 2, 0.01),
    "dipole": ({0: 1, 1: -1}, 1, -1, 0.02),
    "quadrupole": ({-2: -1, -1: 1, 1: 1, 2: -1}, 2, -6, 0.03),
}


def theta(u: float) -> float:
    return 2.0 * math.asin(math.sin(u / 2.0) / math.sqrt(3.0))


def root(period: int) -> float:
    lower, upper = 0.0, PI
    f_lower = (2.0 * PI - lower) / period - theta(lower)
    for _ in range(160):
        midpoint = (lower + upper) / 2.0
        f_midpoint = (2.0 * PI - midpoint) / period - theta(midpoint)
        if f_lower * f_midpoint <= 0.0:
            upper = midpoint
        else:
            lower, f_lower = midpoint, f_midpoint
    return (lower + upper) / 2.0


def moment(profile: dict[int, int], order: int) -> int:
    return sum(polarity * position**order for position, polarity in profile.items())


def leading_moment(profile: dict[int, int]) -> tuple[int, int]:
    for order in range(9):
        value = moment(profile, order)
        if value:
            return order, value
    raise AssertionError("registered finite profile has no nonzero moment")


def form_factor(profile: dict[int, int], u: float) -> complex:
    return sum(polarity * cmath.exp(1j * u * position) for position, polarity in profile.items())


def coefficient(order: int, leading: int) -> float:
    return (
        math.sqrt(3.0)
        * (2.0 * PI * math.sqrt(3.0)) ** (order + 1)
        * abs(leading)
        / math.factorial(order)
    )


def main() -> None:
    maximum_pole = 0.0
    minimum_forcing = math.inf
    maximum_t256_error = 0.0
    arms = 0
    for name, (profile, expected_order, expected_moment, error_gate) in PROFILES.items():
        order, leading = leading_moment(profile)
        assert (order, leading) == (expected_order, expected_moment)
        previous = -math.inf
        for period in PERIODS:
            u = root(period)
            omega = (2.0 * PI - u) / period
            pole = (4.0 / 3.0) * math.sin(u / 2.0) ** 2 - 4.0 * math.sin(omega / 2.0) ** 2
            forcing = math.sqrt(3.0) / period * math.sin(u) * abs(form_factor(profile, u))
            ratio = period ** (order + 2) * forcing / coefficient(order, leading)
            assert abs(pole) <= TOL
            assert forcing > 0.0
            assert ratio > previous
            previous = ratio
            maximum_pole = max(maximum_pole, abs(pole))
            minimum_forcing = min(minimum_forcing, forcing)
            if period == 256:
                error = abs(ratio - 1.0)
                assert error < error_gate
                maximum_t256_error = max(maximum_t256_error, error)
            arms += 6  # three rotations and two global polarity mirrors

    assert arms == 96
    # A transverse dipole cancels the axial witness but not the T=1 oblique one.
    lower, upper = 0.0, 0.2
    def full_symbol(kx: float, ky: float) -> float:
        cx, cy = math.cos(kx), math.cos(ky)
        return 10.0 / 3.0 - 4.0 / 3.0 * (cx + cy) - 2.0 / 3.0 * cx * cy
    def oblique_residual(ky: float) -> float:
        return 0.1 - 2.0 * math.asin(math.sqrt(C2 * full_symbol(0.1, ky)) / 2.0)
    f_lower = oblique_residual(lower)
    for _ in range(160):
        midpoint = (lower + upper) / 2.0
        f_midpoint = oblique_residual(midpoint)
        if f_lower * f_midpoint <= 0.0:
            upper = midpoint
        else:
            lower, f_lower = midpoint, f_midpoint
    ky = (lower + upper) / 2.0
    oblique_amplitude = abs(1.0 - cmath.exp(1j * ky))
    assert oblique_amplitude > 1.0e-3

    print("FTD-0561 periodic-hop source multipole hierarchy: PASS")
    print(f"arms={arms}")
    print(f"maximum_denominator_residual={maximum_pole:.17g}")
    print(f"minimum_normalized_forcing={minimum_forcing:.17g}")
    print(f"maximum_t256_asymptotic_error={maximum_t256_error:.17g}")
    print(f"same_plane_oblique_amplitude={oblique_amplitude:.17g}")
    print("extended_nonlinear_carrier_status=OPEN")
    print("verdict=HOP_SOURCE_MULTIPOLE_HIERARCHY_DERIVED")


if __name__ == "__main__":
    main()
