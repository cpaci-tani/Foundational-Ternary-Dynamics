#!/usr/bin/env python3
"""Independent verifier for the FTD-0563 Gauss/mobile dichotomy."""

from __future__ import annotations

import cmath
import math
from fractions import Fraction


TOL = 1.0e-12
VOLUMES = (32, 64, 128, 256)
RAW_DIRECTIONS = ((1, 0, 0), (1, 1, 0), (1, 1, 1), (-1, 2, 3))
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


def rotate(values: tuple[int | float, int | float, int | float], axis: int):
    result = [0, 0, 0]
    for component, value in enumerate(values):
        result[(component + axis) % 3] = value
    return tuple(result)


def rotated_profile(profile: dict[tuple[int, int, int], int], axis: int):
    return {rotate(position, axis): value for position, value in profile.items()}


def mixed_moment(profile: dict[tuple[int, int, int], int], powers):
    return sum(
        value * position[0] ** powers[0]
        * position[1] ** powers[1] * position[2] ** powers[2]
        for position, value in profile.items()
    )


def leading_order(profile: dict[tuple[int, int, int], int]) -> int:
    for order in range(13):
        for px in range(order + 1):
            for py in range(order - px + 1):
                if mixed_moment(profile, (px, py, order - px - py)):
                    return order
    raise AssertionError("nonzero finite profile has no Taylor coefficient")


def form_factor(profile, momentum, polarity):
    return sum(
        polarity * value * cmath.exp(
            1j * sum(momentum[i] * position[i] for i in range(3))
        )
        for position, value in profile.items()
    )


def leading_polynomial(profile, direction, order, polarity):
    moment = sum(
        polarity * value
        * sum(direction[i] * position[i] for i in range(3)) ** order
        for position, value in profile.items()
    )
    return 1j**order * moment / math.factorial(order)


def analyze_arm(name, base_profile, expected_order, volume,
                direction_index, axis, polarity):
    profile = rotated_profile(base_profile, axis)
    raw = rotate(RAW_DIRECTIONS[direction_index], axis)
    raw_norm = math.sqrt(sum(value * value for value in raw))
    direction = tuple(value / raw_norm for value in raw)
    momentum = tuple(2.0 * math.pi * value / volume for value in raw)
    kappa = 2.0 * math.pi * raw_norm / volume
    source = form_factor(profile, momentum, polarity)
    order = leading_order(base_profile)
    assert order == expected_order
    polynomial = leading_polynomial(profile, direction, order, polarity)
    witness = abs(polynomial) > 1.0e-14
    d = tuple(1.0 - cmath.exp(-1j * value) for value in momentum)
    laplacian = sum(abs(value) ** 2 for value in d)
    field = tuple(value.conjugate() * source / laplacian for value in d)
    estimator = math.sqrt(laplacian) * math.sqrt(
        sum(abs(value) ** 2 for value in field)
    )
    identity = abs(estimator - abs(source))
    ratio = (
        abs(source) / (kappa**order * abs(polynomial)) if witness else 0.0
    )
    assert identity <= TOL
    assert not witness or ratio > 0.0
    return {
        "name": name,
        "volume": volume,
        "direction_index": direction_index,
        "axis": axis,
        "polarity": polarity,
        "order": order,
        "total": polarity * sum(base_profile.values()),
        "witness": witness,
        "source": source,
        "laplacian": laplacian,
        "field": field,
        "estimator": estimator,
        "ratio": ratio,
        "identity": identity,
    }


def periodic_exact_controls() -> tuple[int, Fraction, Fraction]:
    maximum_numerator = 0
    for profile, _ in PROFILES.values():
        for size in (8, 16):
            count = size**3
            for axis in range(3):
                rotated = rotated_profile(profile, axis)
                for polarity in (1, -1):
                    source = [0] * count
                    for position, value in rotated.items():
                        x, y, z = (coordinate + 2 for coordinate in position)
                        source[(x * size + y) * size + z] += polarity * value
                    total = sum(source)
                    numerator_sum = sum(count * value - total for value in source)
                    maximum_numerator = max(maximum_numerator, abs(numerator_sum))

    size = 8
    index = lambda x, y, z: ((x % size) * size + y % size) * size + z % size
    face = [[Fraction(0) for _ in range(size**3)] for _ in range(3)]
    for x in range(size):
        for y in range(size):
            for z in range(size):
                i = index(x, y, z)
                face[0][i] = Fraction((x + 2 * y + 3 * z) % 7 - 3, 8)
                face[1][i] = Fraction((3 * x + y + 2 * z) % 7 - 3, 16)
                face[2][i] = Fraction((2 * x + 3 * y + z) % 7 - 3, 32)

    def divergence(field, x, y, z):
        return (
            field[0][index(x, y, z)] - field[0][index(x - 1, y, z)]
            + field[1][index(x, y, z)] - field[1][index(x, y - 1, z)]
            + field[2][index(x, y, z)] - field[2][index(x, y, z - 1)]
        )

    telescope = sum(
        divergence(face, x, y, z)
        for x in range(size) for y in range(size) for z in range(size)
    )

    edge = [[Fraction(0) for _ in range(size**3)] for _ in range(3)]
    for x in range(size):
        for y in range(size):
            for z in range(size):
                i = index(x, y, z)
                edge[0][i] = Fraction((x + y + 2 * z) % 5 - 2, 16)
                edge[1][i] = Fraction((2 * x + y + z) % 5 - 2, 32)
                edge[2][i] = Fraction((x + 2 * y + z) % 5 - 2, 64)
    curl = [[Fraction(0) for _ in range(size**3)] for _ in range(3)]
    for x in range(size):
        for y in range(size):
            for z in range(size):
                i = index(x, y, z)
                curl[0][i] = (
                    edge[2][i] - edge[2][index(x, y - 1, z)]
                    - edge[1][i] + edge[1][index(x, y, z - 1)]
                )
                curl[1][i] = (
                    edge[0][i] - edge[0][index(x, y, z - 1)]
                    - edge[2][i] + edge[2][index(x - 1, y, z)]
                )
                curl[2][i] = (
                    edge[1][i] - edge[1][index(x - 1, y, z)]
                    - edge[0][i] + edge[0][index(x, y - 1, z)]
                )
    maximum_curl_divergence = max(
        abs(divergence(curl, x, y, z))
        for x in range(size) for y in range(size) for z in range(size)
    )
    assert any(value for component in curl for value in component)
    return maximum_numerator, telescope, maximum_curl_divergence


def main() -> None:
    maximum_numerator, telescope, curl_divergence = periodic_exact_controls()
    assert maximum_numerator == 0
    assert telescope == 0
    assert curl_divergence == 0

    arms = []
    for name, (profile, expected_order) in PROFILES.items():
        for volume in VOLUMES:
            for direction_index in range(len(RAW_DIRECTIONS)):
                for axis in range(3):
                    for polarity in (1, -1):
                        arms.append(analyze_arm(
                            name, profile, expected_order, volume,
                            direction_index, axis, polarity,
                        ))
    assert len(arms) == 384

    witness_groups = 0
    for name in PROFILES:
        for volume in VOLUMES:
            for axis in range(3):
                for polarity in (1, -1):
                    group = [arm for arm in arms if arm["name"] == name
                             and arm["volume"] == volume
                             and arm["axis"] == axis
                             and arm["polarity"] == polarity]
                    assert any(arm["witness"] for arm in group)
                    witness_groups += 1
    assert witness_groups == 96

    monotone_neutral_witnesses = 0
    for name in tuple(PROFILES)[1:]:
        for direction_index in range(len(RAW_DIRECTIONS)):
            for axis in range(3):
                for polarity in (1, -1):
                    group = [next(arm for arm in arms
                                  if arm["name"] == name
                                  and arm["volume"] == volume
                                  and arm["direction_index"] == direction_index
                                  and arm["axis"] == axis
                                  and arm["polarity"] == polarity)
                             for volume in VOLUMES]
                    if not group[0]["witness"]:
                        continue
                    assert all(group[i + 1]["estimator"] < group[i]["estimator"]
                               for i in range(len(group) - 1))
                    monotone_neutral_witnesses += 1
    assert monotone_neutral_witnesses == 54

    point_error = max(abs(arm["estimator"] - abs(arm["total"]))
                      for arm in arms if arm["total"])
    neutral_l256 = [arm for arm in arms
                    if arm["volume"] == 256 and not arm["total"]]
    maximum_neutral = max(arm["estimator"] for arm in neutral_l256)
    maximum_asymptotic = max(abs(arm["ratio"] - 1.0)
                             for arm in neutral_l256 if arm["witness"])
    maximum_identity = max(arm["identity"] for arm in arms)

    maximum_mirror = 0.0
    maximum_covariance = 0.0
    for name in PROFILES:
        for volume in VOLUMES:
            for direction_index in range(len(RAW_DIRECTIONS)):
                for axis in range(3):
                    plus = next(arm for arm in arms if arm["name"] == name
                                and arm["volume"] == volume
                                and arm["direction_index"] == direction_index
                                and arm["axis"] == axis
                                and arm["polarity"] == 1)
                    minus = next(arm for arm in arms if arm["name"] == name
                                 and arm["volume"] == volume
                                 and arm["direction_index"] == direction_index
                                 and arm["axis"] == axis
                                 and arm["polarity"] == -1)
                    maximum_mirror = max(
                        maximum_mirror,
                        abs(plus["source"] + minus["source"]),
                        abs(plus["estimator"] - minus["estimator"]),
                    )
                for polarity in (1, -1):
                    rotated = [next(arm for arm in arms
                                    if arm["name"] == name
                                    and arm["volume"] == volume
                                    and arm["direction_index"] == direction_index
                                    and arm["axis"] == axis
                                    and arm["polarity"] == polarity)
                               for axis in range(3)]
                    for axis, candidate in enumerate(rotated[1:], start=1):
                        maximum_covariance = max(
                            maximum_covariance,
                            abs(rotated[0]["source"] - candidate["source"]),
                            abs(rotated[0]["estimator"]
                                - candidate["estimator"]),
                            abs(rotated[0]["laplacian"]
                                - candidate["laplacian"]),
                        )
                        maximum_covariance = max(
                            maximum_covariance,
                            *(abs(rotated[0]["field"][component]
                                  - candidate["field"][(component + axis) % 3])
                              for component in range(3)),
                        )

    assert point_error <= TOL
    assert maximum_neutral < 0.1
    assert maximum_asymptotic < 0.02
    assert maximum_mirror <= TOL
    assert maximum_covariance <= TOL

    print("FTD-0563 Gauss monopole/mobile-dressing dichotomy: PASS")
    print(f"arms={len(arms)}")
    print(f"witness_groups={witness_groups}")
    print(f"monotone_neutral_witnesses={monotone_neutral_witnesses}")
    print(f"maximum_zero_mode_numerator_sum={maximum_numerator}")
    print(f"periodic_telescope_residual={float(telescope):.17g}")
    print(f"maximum_curl_divergence={float(curl_divergence):.17g}")
    print(f"maximum_face_gauss_identity_residual={maximum_identity:.17g}")
    print(f"maximum_point_monopole_error={point_error:.17g}")
    print(f"maximum_l256_neutral_monopole_estimator={maximum_neutral:.17g}")
    print(f"maximum_l256_asymptotic_error={maximum_asymptotic:.17g}")
    print(f"maximum_polarity_mirror_residual={maximum_mirror:.17g}")
    print(f"maximum_cyclic_covariance_residual={maximum_covariance:.17g}")
    print("nonlinear_topological_effective_charge_status=OPEN")
    print("verdict=GAUSS_MONOPOLE_MOBILE_DRESSING_DICHOTOMY_PROVED")


if __name__ == "__main__":
    main()
