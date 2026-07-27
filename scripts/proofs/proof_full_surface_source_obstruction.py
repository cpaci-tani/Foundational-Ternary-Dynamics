#!/usr/bin/env python3
"""Independent verifier for the FTD-0562 finite-source obstruction."""

from __future__ import annotations

import cmath
import math
from dataclasses import dataclass


PI = math.pi
C2 = 1.0 / 3.0
TOL = 1.0e-12
PERIODS = (64, 128, 256, 512)
RAW_DIRECTIONS = (
    (-1, 0, 0),
    (1, 1, 0),
    (1, 0, 1),
    (1, 1, 1),
    (-1, 1, 1),
    (2, -1, 3),
    (-2, 3, 1),
    (3, 2, -1),
)
PROFILES = {
    "point": ({(0, 0, 0): 1}, 0),
    "axial_dipole": ({(0, 0, 0): 1, (1, 0, 0): -1}, 1),
    "planar_quadrupole": (
        {(0, 0, 0): 1, (1, 0, 0): -1,
         (0, 1, 0): -1, (1, 1, 0): 1},
        2,
    ),
    "cubic_octupole": (
        {(0, 0, 0): 1, (1, 0, 0): -1,
         (0, 1, 0): -1, (0, 0, 1): -1,
         (1, 1, 0): 1, (1, 0, 1): 1,
         (0, 1, 1): 1, (1, 1, 1): -1},
        3,
    ),
}


@dataclass(frozen=True)
class Arm:
    profile: str
    period: int
    direction_index: int
    axis: int
    polarity: int
    leading_order: int
    witness: bool
    root: float
    denominator_residual: float
    scaled_derivative: float
    form_factor: complex
    forcing: float
    ratio: float
    radius_error: float


def rotate(values: tuple[float, float, float], axis: int) -> tuple[float, float, float]:
    result = [0.0, 0.0, 0.0]
    for component, value in enumerate(values):
        result[(component + axis) % 3] = value
    return tuple(result)  # type: ignore[return-value]


def unit(values: tuple[int, int, int]) -> tuple[float, float, float]:
    norm = math.sqrt(sum(value * value for value in values))
    return tuple(value / norm for value in values)  # type: ignore[return-value]


def rotated_profile(
    profile: dict[tuple[int, int, int], int], axis: int
) -> dict[tuple[int, int, int], int]:
    return {
        tuple(int(value) for value in rotate(position, axis)): polarity
        for position, polarity in profile.items()
    }


def symbol(k: tuple[float, float, float]) -> float:
    cx, cy, cz = (math.cos(value) for value in k)
    return 4.0 - (2.0 / 3.0) * (cx + cy + cz) - (2.0 / 3.0) * (
        cx * cy + cx * cz + cy * cz
    )


def symbol_gradient(k: tuple[float, float, float]) -> tuple[float, float, float]:
    cx, cy, cz = (math.cos(value) for value in k)
    sx, sy, sz = (math.sin(value) for value in k)
    return (
        (2.0 / 3.0) * sx * (1.0 + cy + cz),
        (2.0 / 3.0) * sy * (1.0 + cx + cz),
        (2.0 / 3.0) * sz * (1.0 + cx + cy),
    )


def denominator(
    radius: float,
    direction: tuple[float, float, float],
    axis: int,
    period: int,
) -> float:
    k = tuple(radius * value / period for value in direction)
    omega = (2.0 * PI + k[axis]) / period
    return C2 * symbol(k) - 4.0 * math.sin(omega / 2.0) ** 2


def radial_root(
    direction: tuple[float, float, float], axis: int, period: int
) -> float:
    r0 = 2.0 * PI / math.sqrt(C2)
    lower, upper = 0.0, 2.0 * r0
    f_lower = denominator(lower, direction, axis, period)
    f_upper = denominator(upper, direction, axis, period)
    assert f_lower < 0.0 < f_upper
    for _ in range(180):
        midpoint = (lower + upper) / 2.0
        f_midpoint = denominator(midpoint, direction, axis, period)
        if f_lower * f_midpoint <= 0.0:
            upper = midpoint
        else:
            lower, f_lower = midpoint, f_midpoint
    return (lower + upper) / 2.0


def mixed_moment(
    profile: dict[tuple[int, int, int], int],
    powers: tuple[int, int, int],
) -> int:
    return sum(
        polarity
        * position[0] ** powers[0]
        * position[1] ** powers[1]
        * position[2] ** powers[2]
        for position, polarity in profile.items()
    )


def leading_order(profile: dict[tuple[int, int, int], int]) -> int:
    for order in range(13):
        for px in range(order + 1):
            for py in range(order - px + 1):
                pz = order - px - py
                if mixed_moment(profile, (px, py, pz)):
                    return order
    raise AssertionError("nonzero finite profile has no Taylor coefficient")


def form_factor(
    profile: dict[tuple[int, int, int], int],
    k: tuple[float, float, float],
    polarity: int,
) -> complex:
    return sum(
        polarity
        * site_polarity
        * cmath.exp(1j * sum(k[i] * position[i] for i in range(3)))
        for position, site_polarity in profile.items()
    )


def leading_polynomial(
    profile: dict[tuple[int, int, int], int],
    direction: tuple[float, float, float],
    order: int,
    polarity: int,
) -> complex:
    moment = sum(
        polarity
        * site_polarity
        * sum(direction[i] * position[i] for i in range(3)) ** order
        for position, site_polarity in profile.items()
    )
    return 1j ** order * moment / math.factorial(order)


def cross(
    lhs: tuple[float, float, float], rhs: tuple[float, float, float]
) -> tuple[float, float, float]:
    return (
        lhs[1] * rhs[2] - lhs[2] * rhs[1],
        lhs[2] * rhs[0] - lhs[0] * rhs[2],
        lhs[0] * rhs[1] - lhs[1] * rhs[0],
    )


def analyze_arm(
    name: str,
    base_profile: dict[tuple[int, int, int], int],
    expected_order: int,
    period: int,
    direction_index: int,
    axis: int,
    polarity: int,
) -> Arm:
    profile = rotated_profile(base_profile, axis)
    direction = rotate(unit(RAW_DIRECTIONS[direction_index]), axis)
    order = leading_order(base_profile)
    assert order == expected_order
    root = radial_root(direction, axis, period)
    k = tuple(root * value / period for value in direction)
    omega = (2.0 * PI + k[axis]) / period
    residual = abs(C2 * symbol(k) - 4.0 * math.sin(omega / 2.0) ** 2)
    gradient = symbol_gradient(k)
    derivative = C2 * sum(
        gradient[i] * direction[i] / period for i in range(3)
    ) - 2.0 * math.sin(omega) * direction[axis] / period**2
    scaled_derivative = period**2 * abs(derivative)
    source = form_factor(profile, k, polarity)
    polynomial = leading_polynomial(profile, direction, order, polarity)
    witness = abs(direction[axis] * polynomial) > 1.0e-14
    coefficient = (1.0 - cmath.exp(1j * k[axis])) / (
        period * (1.0 - cmath.exp(1j * omega))
    )
    q = tuple(math.sin(value) for value in k)
    velocity = tuple(1.0 / period if i == axis else 0.0 for i in range(3))
    transverse = cross(q, velocity)
    forcing = abs(coefficient) * abs(source) * math.sqrt(
        sum(value * value for value in q)
        + sum(value * value for value in transverse)
    )
    r0 = 2.0 * PI / math.sqrt(C2)
    asymptotic = math.sqrt(3.0) * r0 ** (order + 1) * abs(
        direction[axis] * polynomial
    )
    ratio = period ** (order + 2) * forcing / asymptotic if witness else 0.0
    radius_error = abs(period * (root - r0) - 6.0 * PI * direction[axis])
    assert residual <= TOL
    assert scaled_derivative > 1.0
    assert not witness or (forcing > 0.0 and math.isfinite(ratio))
    return Arm(
        name, period, direction_index, axis, polarity, order, witness,
        root, residual, scaled_derivative, source, forcing, ratio, radius_error,
    )


def main() -> None:
    arms: list[Arm] = []
    for name, (profile, expected_order) in PROFILES.items():
        for period in PERIODS:
            for direction_index in range(len(RAW_DIRECTIONS)):
                for axis in range(3):
                    for polarity in (1, -1):
                        arms.append(analyze_arm(
                            name, profile, expected_order, period,
                            direction_index, axis, polarity,
                        ))
    assert len(arms) == 768

    groups = 0
    for name in PROFILES:
        for period in PERIODS:
            for axis in range(3):
                for polarity in (1, -1):
                    group = [arm for arm in arms if arm.profile == name
                             and arm.period == period and arm.axis == axis
                             and arm.polarity == polarity]
                    assert any(arm.witness and arm.forcing > 0.0 for arm in group)
                    groups += 1
    assert groups == 96

    maximum_mirror = 0.0
    maximum_covariance = 0.0
    for name in PROFILES:
        for period in PERIODS:
            for direction_index in range(len(RAW_DIRECTIONS)):
                for axis in range(3):
                    plus = next(arm for arm in arms if arm.profile == name
                                and arm.period == period
                                and arm.direction_index == direction_index
                                and arm.axis == axis and arm.polarity == 1)
                    minus = next(arm for arm in arms if arm.profile == name
                                 and arm.period == period
                                 and arm.direction_index == direction_index
                                 and arm.axis == axis and arm.polarity == -1)
                    maximum_mirror = max(
                        maximum_mirror,
                        abs(plus.form_factor + minus.form_factor),
                        abs(plus.forcing - minus.forcing),
                    )
                for polarity in (1, -1):
                    rotated = [next(arm for arm in arms if arm.profile == name
                                    and arm.period == period
                                    and arm.direction_index == direction_index
                                    and arm.axis == axis
                                    and arm.polarity == polarity)
                               for axis in range(3)]
                    for candidate in rotated[1:]:
                        maximum_covariance = max(
                            maximum_covariance,
                            abs(rotated[0].root - candidate.root),
                            abs(rotated[0].form_factor - candidate.form_factor),
                            abs(rotated[0].forcing - candidate.forcing),
                            abs(rotated[0].scaled_derivative
                                - candidate.scaled_derivative),
                        )

    t512 = [arm for arm in arms if arm.period == 512]
    maximum_radius_error = max(arm.radius_error for arm in t512)
    maximum_forcing_error = max(
        abs(arm.ratio - 1.0) for arm in t512 if arm.witness
    )
    maximum_denominator = max(arm.denominator_residual for arm in arms)
    minimum_regularity = min(arm.scaled_derivative for arm in arms)
    minimum_scaled_forcing = min(
        arm.period ** (arm.leading_order + 2) * arm.forcing
        for arm in arms if arm.witness
    )

    assert maximum_mirror <= TOL
    assert maximum_covariance <= TOL
    assert maximum_radius_error < 0.25
    assert maximum_forcing_error < 0.20
    assert minimum_scaled_forcing > 0.0

    print("FTD-0562 finite-source full-surface obstruction: PASS")
    print(f"arms={len(arms)}")
    print(f"witness_groups={groups}")
    print(f"maximum_denominator_residual={maximum_denominator:.17g}")
    print(f"minimum_scaled_radial_derivative={minimum_regularity:.17g}")
    print(f"maximum_polarity_mirror_residual={maximum_mirror:.17g}")
    print(f"maximum_cyclic_covariance_residual={maximum_covariance:.17g}")
    print(f"maximum_t512_radius_correction_residual={maximum_radius_error:.17g}")
    print(f"maximum_t512_asymptotic_error={maximum_forcing_error:.17g}")
    print(f"minimum_witness_scaled_forcing={minimum_scaled_forcing:.17g}")
    print("nonlinear_deforming_carrier_status=OPEN")
    print("verdict=FINITE_RIGID_FULL_SURFACE_CANCELLATION_OBSTRUCTED")


if __name__ == "__main__":
    main()
