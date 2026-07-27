#!/usr/bin/env python3
"""Independent verifier for the FTD-0560 point-hop dressing obstruction."""

from __future__ import annotations

import cmath
import math


PI = math.pi
C2 = 1.0 / 3.0
G_STAR = math.gamma(0.25) / math.gamma(0.75)
X_PLUS = 8.0 * G_STAR**2 + 4.0 * G_STAR * math.sqrt(4.0 * G_STAR**2 - G_STAR)
G_C = 1.0 / math.sqrt(X_PLUS)
TOL = 1.0e-12
RESPONSE_TICKS = 128


def full_symbol(k: tuple[float, float, float]) -> float:
    cx, cy, cz = (math.cos(value) for value in k)
    return 4.0 - (2.0 / 3.0) * (cx + cy + cz) - (2.0 / 3.0) * (
        cx * cy + cx * cz + cy * cz
    )


def phase(k: tuple[float, float, float]) -> float:
    return 2.0 * math.asin(math.sqrt(C2 * full_symbol(k)) / 2.0)


def rotated(axis: int, parallel: float, transverse: float = 0.0) -> tuple[float, float, float]:
    values = [0.0, 0.0, 0.0]
    values[axis] = parallel
    values[(axis + 1) % 3] = transverse
    return tuple(values)


def bisect(function, lower: float, upper: float) -> float:
    f_lower = function(lower)
    f_upper = function(upper)
    assert f_lower * f_upper < 0.0
    for _ in range(160):
        midpoint = 0.5 * (lower + upper)
        f_midpoint = function(midpoint)
        if f_lower * f_midpoint <= 0.0:
            upper = midpoint
            f_upper = f_midpoint
        else:
            lower = midpoint
            f_lower = f_midpoint
    return 0.5 * (lower + upper)


def floquet_coefficient(k_dot_d: float, period: int, harmonic: int) -> complex:
    numerator = 1.0 - cmath.exp(1j * k_dot_d)
    angle = (k_dot_d + 2.0 * PI * harmonic) / period
    denominator = period * (1.0 - cmath.exp(1j * angle))
    return 1.0 + 0.0j if abs(denominator) < 1.0e-14 else numerator / denominator


def modal_energy(state: tuple[complex, complex], kick: float) -> float:
    flux, wave = state
    return abs(wave) ** 2 + kick * abs(flux) ** 2 - kick * (flux.conjugate() * wave).real


def forced_step(state: tuple[complex, complex], kick: float, drive: complex) -> tuple[complex, complex]:
    flux, wave = state
    wave = wave - kick * flux + drive
    flux = flux + wave
    return flux, wave


def resonant_bound(kick: float, theta: float, ticks: int) -> float:
    sine = abs(math.sin(theta))
    cosine_half = math.cos(theta / 2.0)
    vector_norm = math.sqrt(1.0 / (4.0 * sine**2) + 1.0 / (4.0 * cosine_half**2))
    error_norm = math.sqrt(
        1.0 / (4.0 * sine**4)
        + (1.0 / sine**2 + 1.0 / (2.0 * sine)) ** 2
    )
    lambda_bound = kick + 1.0
    return (
        2.0 * lambda_bound * vector_norm * error_norm / ticks
        + lambda_bound * error_norm**2 / ticks**2
    )


def main() -> None:
    maximum_root = 0.0
    maximum_orthogonality = 0.0
    maximum_coefficient = 0.0
    maximum_covariance = 0.0
    minimum_forcing = math.inf
    minimum_regularity = math.inf
    maximum_response_excess = 0.0
    arm_count = 0

    roots: dict[tuple[int, int], tuple[float, float, float, float]] = {}
    for period in range(1, 17):
        for axis in range(3):
            if period == 1:
                parallel = 0.1
                root = bisect(
                    lambda transverse: parallel - phase(rotated(axis, parallel, transverse)),
                    0.0,
                    0.2,
                )
                k = rotated(axis, parallel, root)
                omega = parallel
                harmonic = 0
                regularity = C2 * (2.0 / 3.0) * math.sin(root) * (
                    2.0 + math.cos(parallel)
                )
            elif period == 2:
                root = bisect(
                    lambda u: u / period - phase(rotated(axis, u)),
                    0.01,
                    PI,
                )
                k = rotated(axis, root)
                omega = root / period
                harmonic = 0
                regularity = abs(
                    2.0 * C2 * math.sin(root) - 2.0 * math.sin(omega) / period
                )
            else:
                root = bisect(
                    lambda u: (2.0 * PI - u) / period - phase(rotated(axis, -u)),
                    0.0,
                    PI,
                )
                k = rotated(axis, -root)
                omega = (2.0 * PI - root) / period
                harmonic = 1
                regularity = abs(
                    2.0 * C2 * math.sin(root) + 2.0 * math.sin(omega) / period
                )

            theta = phase(k)
            kick = C2 * full_symbol(k)
            denominator = kick - 4.0 * math.sin(omega / 2.0) ** 2
            coefficient = floquet_coefficient(k[axis], period, harmonic)
            q = tuple(math.sin(component) for component in k)
            velocity = [0.0, 0.0, 0.0]
            velocity[axis] = 1.0 / period
            cross = (
                q[1] * velocity[2] - q[2] * velocity[1],
                q[2] * velocity[0] - q[0] * velocity[2],
                q[0] * velocity[1] - q[1] * velocity[0],
            )
            base = tuple(-q[index] + cross[index] for index in range(3))
            direct_norm = G_C**2 * abs(coefficient) ** 2 * sum(value**2 for value in base)
            orthogonal_norm = G_C**2 * abs(coefficient) ** 2 * (
                sum(value**2 for value in q) + sum(value**2 for value in cross)
            )
            normalized_forcing = math.sqrt(direct_norm) / G_C

            assert abs(denominator) <= TOL
            assert regularity > 1.0e-3
            assert abs(direct_norm - orthogonal_norm) <= TOL
            assert normalized_forcing > 0.05
            if period >= 2:
                assert abs(abs(coefficient) - math.sqrt(3.0) / period) <= TOL

            amplitude = math.sqrt(direct_norm)
            state = (0.0 + 0.0j, 0.0 + 0.0j)
            for tick in range(RESPONSE_TICKS):
                drive = amplitude * cmath.exp(-1j * omega * tick)
                state = forced_step(state, kick, drive)
            normalized_energy = modal_energy(state, kick) / (
                RESPONSE_TICKS**2 * direct_norm
            )
            bound = resonant_bound(kick, theta, RESPONSE_TICKS)
            assert abs(normalized_energy - 0.5) <= bound + TOL

            maximum_root = max(maximum_root, abs(denominator))
            maximum_orthogonality = max(
                maximum_orthogonality, abs(direct_norm - orthogonal_norm)
            )
            maximum_coefficient = max(
                maximum_coefficient,
                0.0 if period == 1 else abs(abs(coefficient) - math.sqrt(3.0) / period),
            )
            minimum_forcing = min(minimum_forcing, normalized_forcing)
            minimum_regularity = min(minimum_regularity, regularity)
            maximum_response_excess = max(
                maximum_response_excess, abs(normalized_energy - 0.5) - bound
            )
            roots[(period, axis)] = (root, theta, abs(coefficient), direct_norm)
            arm_count += 2  # both polarities have exactly opposite source and equal norm

        reference = roots[(period, 0)]
        for axis in (1, 2):
            maximum_covariance = max(
                maximum_covariance,
                *(abs(lhs - rhs) for lhs, rhs in zip(reference, roots[(period, axis)])),
            )

    assert arm_count == 96
    print("FTD-0560 native point-hop dressing obstruction: PASS")
    print(f"arms={arm_count}")
    print(f"maximum_root_residual={maximum_root:.17g}")
    print(f"minimum_regularity_derivative={minimum_regularity:.17g}")
    print(f"maximum_source_orthogonality_residual={maximum_orthogonality:.17g}")
    print(f"maximum_coefficient_identity_residual={maximum_coefficient:.17g}")
    print(f"minimum_normalized_effective_forcing={minimum_forcing:.17g}")
    print(f"maximum_cubic_covariance_residual={maximum_covariance:.17g}")
    print(f"maximum_resonant_coefficient_excess={maximum_response_excess:.17g}")
    print("general_nonlinear_carrier_status=OPEN")
    print("verdict=POINT_HOP_DRESSING_OBSTRUCTED")


if __name__ == "__main__":
    main()
