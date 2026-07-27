#!/usr/bin/env python3
"""Independent verifier for preregistered FTD-0595.

This program independently reconstructs the exact cyclotomic shared-M shells,
the complete two-class displacement kernel, and the free cubic animals through
size nine.  It performs no source-history, polarity, schedule, observation-time,
or threshold-directed geometry search.
"""

from __future__ import annotations

from collections import defaultdict
import hashlib
import itertools
import json
import math
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "PREREG_TEN_SOURCE_PAIR_DISTANCE_CAPACITY_v1.md"
)
PREREG_SHA = "3652D216C915389CD1838CA453C6B0A42F47D748771A9C5D3A1AF23BEEA5AB96"
RESULT = ROOT / "engine/results/ftd_0595/windows_msvc_cpu.json"
ARTIFACTS = {
    "header_sha256": ROOT / "engine/include/ftd/eft/"
    "ten_source_pair_distance_capacity.h",
    "source_sha256": ROOT / "engine/src/eft/"
    "ten_source_pair_distance_capacity.cpp",
    "test_sha256": ROOT / "engine/tests/"
    "test_ten_source_pair_distance_capacity.cpp",
    "proof_sha256": Path(__file__).resolve(),
}
VOLUMES = (9, 17, 33, 65)
G_C = 0.0854245431028543695
C2 = 1.0 / 3.0
K_GENESIS = 1.5163860591519780
KERNEL_TOL = 5.0e-13
CROSS_TOL = 5.0e-12
AXIAL = (
    (1, 0, 0), (-1, 0, 0),
    (0, 1, 0), (0, -1, 0),
    (0, 0, 1), (0, 0, -1),
)
POSITIVE_AXIAL = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
TRANSFORMS = tuple(
    (permutation, signs)
    for permutation in itertools.permutations(range(3))
    for signs in itertools.product((-1, 1), repeat=3)
)


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: str) -> None:
        self.rows.append((bool(condition), name, note))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0595 ten-source pair-distance capacity")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        print("-" * 79)
        print(f"checks={len(self.rows)} passed={passed} "
              f"failed={len(self.rows)-passed}")
        return passed == len(self.rows)


def trim(poly: list[int]) -> list[int]:
    while poly and poly[-1] == 0:
        poly.pop()
    return poly


def multiply(lhs: list[int], rhs: list[int]) -> list[int]:
    if not lhs or not rhs:
        return []
    out = [0] * (len(lhs) + len(rhs) - 1)
    for i, left in enumerate(lhs):
        for j, right in enumerate(rhs):
            out[i + j] += left * right
    return trim(out)


def divide_exact_monic(numerator: list[int],
                       denominator: list[int]) -> list[int]:
    remainder = trim(numerator.copy())
    if not denominator or denominator[-1] != 1:
        raise ValueError("denominator must be monic")
    quotient = [0] * (len(remainder) - len(denominator) + 1)
    while len(remainder) >= len(denominator):
        shift = len(remainder) - len(denominator)
        coefficient = remainder[-1]
        quotient[shift] = coefficient
        for j, value in enumerate(denominator):
            remainder[shift + j] -= coefficient * value
        trim(remainder)
    if remainder:
        raise ValueError("non-exact polynomial division")
    return trim(quotient)


def divisors(value: int) -> list[int]:
    return [candidate for candidate in range(1, value + 1)
            if value % candidate == 0]


def cyclotomic(value: int, cache: dict[int, list[int]]) -> list[int]:
    if value in cache:
        return cache[value]
    polynomial = [-1] + [0] * (value - 1) + [1]
    for divisor in divisors(value):
        if divisor != value:
            polynomial = divide_exact_monic(
                polynomial, cyclotomic(divisor, cache))
    cache[value] = polynomial
    return polynomial


def exact_key(lattice_size: int, mode: tuple[int, int, int],
              phi: list[int]) -> tuple[int, ...]:
    value = [0] * lattice_size

    def add(exponent: int, coefficient: int) -> None:
        value[exponent % lattice_size] += coefficient

    add(0, 24)
    for component in mode:
        add(component, -2)
        add(-component, -2)
    for left in range(3):
        for right in range(left + 1, 3):
            for left_sign in (-1, 1):
                for right_sign in (-1, 1):
                    add(left_sign * mode[left]
                        + right_sign * mode[right], -1)
    degree = len(phi) - 1
    for index in range(len(value) - 1, degree - 1, -1):
        coefficient = value[index]
        if not coefficient:
            continue
        shift = index - degree
        for phi_index, phi_coefficient in enumerate(phi):
            value[shift + phi_index] -= coefficient * phi_coefficient
    return tuple(value[:degree])


def unique_permutations(value: tuple[int, int, int]
                        ) -> list[tuple[int, int, int]]:
    return sorted(set(itertools.permutations(value)))


def orbit_members(lattice_size: int, value: tuple[int, int, int]
                  ) -> set[tuple[int, int, int]]:
    members: set[tuple[int, int, int]] = set()
    for permutation in unique_permutations(value):
        sign_sets = [(-1, 1) if component else (1,)
                     for component in permutation]
        for signs in itertools.product(*sign_sets):
            members.add(tuple(
                (sign * component) % lattice_size
                for sign, component in zip(signs, permutation)))
    return members


def symbol_and_gradient(lattice_size: int,
                        mode: tuple[int, int, int]) -> tuple[float, float]:
    angles = [2.0 * math.pi * component / lattice_size
              for component in mode]
    cx, cy, cz = (math.cos(angle) for angle in angles)
    symbol = (
        4.0 - (2.0 / 3.0) * (cx + cy + cz)
        - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz)
    )
    gradient2 = math.fsum(math.sin(angle) ** 2 for angle in angles)
    return symbol, gradient2


def canonical_free_animal(points: tuple[tuple[int, int, int], ...]
                          ) -> tuple[tuple[int, int, int], ...]:
    best: tuple[tuple[int, int, int], ...] | None = None
    for permutation, signs in TRANSFORMS:
        transformed = [
            tuple(signs[axis] * point[permutation[axis]]
                  for axis in range(3))
            for point in points
        ]
        minima = [min(point[axis] for point in transformed)
                  for axis in range(3)]
        candidate = tuple(sorted(
            tuple(point[axis] - minima[axis] for axis in range(3))
            for point in transformed))
        if best is None or candidate < best:
            best = candidate
    if best is None:
        raise RuntimeError("empty animal")
    return best


def canonical_periodic_animal(
        lattice_size: int, points: tuple[tuple[int, int, int], ...]
) -> tuple[tuple[int, int, int], ...]:
    best: tuple[tuple[int, int, int], ...] | None = None
    for permutation, signs in TRANSFORMS:
        transformed = [
            tuple((signs[axis] * point[permutation[axis]]) % lattice_size
                  for axis in range(3))
            for point in points
        ]
        for origin in transformed:
            candidate = tuple(sorted(
                tuple((point[axis] - origin[axis]) % lattice_size
                      for axis in range(3))
                for point in transformed))
            if best is None or candidate < best:
                best = candidate
    if best is None:
        raise RuntimeError("empty periodic animal")
    return best


def free_edge_count(points: tuple[tuple[int, int, int], ...]) -> int:
    occupied = set(points)
    return sum(
        tuple(point[axis] + step[axis] for axis in range(3)) in occupied
        for point in points for step in POSITIVE_AXIAL)


def periodic_edge_count(lattice_size: int,
                        points: tuple[tuple[int, int, int], ...]) -> int:
    occupied = set(points)
    return sum(
        tuple((point[axis] + step[axis]) % lattice_size
              for axis in range(3)) in occupied
        for point in points for step in POSITIVE_AXIAL)


def enumerate_animals() -> dict[str, list[int]]:
    current = {((0, 0, 0),)}
    free_counts = [0] * 10
    free_edges = [0] * 10
    l9_counts = [0] * 10
    l9_edges = [0] * 10
    for size in range(1, 10):
        free_counts[size] = len(current)
        free_edges[size] = max(free_edge_count(animal)
                               for animal in current)
        periodic = {
            canonical_periodic_animal(9, animal)
            for animal in current
        }
        l9_counts[size] = len(periodic)
        l9_edges[size] = max(periodic_edge_count(9, animal)
                             for animal in periodic)
        if size == 9:
            break
        next_animals: set[tuple[tuple[int, int, int], ...]] = set()
        for animal in current:
            occupied = set(animal)
            boundary = {
                tuple(point[axis] + step[axis] for axis in range(3))
                for point in animal for step in AXIAL
            } - occupied
            for point in boundary:
                next_animals.add(canonical_free_animal(animal + (point,)))
        current = next_animals
    return {
        "l9_counts": l9_counts,
        "l9_edges": l9_edges,
        "l17_counts": free_counts,
        "l17_edges": free_edges,
    }


def analyze_volume(lattice_size: int, edge_caps: list[int]
                   ) -> dict[str, object]:
    cache: dict[int, list[int]] = {}
    phi = cyclotomic(lattice_size, cache)
    half = lattice_size // 2
    representatives = [
        (a, b, c)
        for a in range(half + 1)
        for b in range(a, half + 1)
        for c in range(b, half + 1)
        if (a, b, c) != (0, 0, 0)
    ]
    members: list[set[tuple[int, int, int]]] = []
    permutations: list[list[tuple[int, int, int]]] = []
    keys: list[tuple[int, ...]] = []
    weights: list[float] = []
    pulse_terms: list[float] = []
    step_terms: list[float] = []
    covered: set[tuple[int, int, int]] = set()
    exact_invariant = True
    disjoint = True
    for representative in representatives:
        orbit = orbit_members(lattice_size, representative)
        key = exact_key(lattice_size, representative, phi)
        exact_invariant = exact_invariant and all(
            exact_key(lattice_size, member, phi) == key
            for member in orbit)
        disjoint = disjoint and covered.isdisjoint(orbit)
        covered.update(orbit)
        symbol, gradient2 = symbol_and_gradient(
            lattice_size, representative)
        weight = gradient2 / symbol
        denominator = math.sqrt(1.0 - C2 * symbol / 4.0)
        multiplicity = len(orbit)
        members.append(orbit)
        permutations.append(unique_permutations(representative))
        keys.append(key)
        weights.append(weight)
        pulse_terms.append(multiplicity * (2.0 / denominator) ** 2 / symbol)
        step_terms.append(
            multiplicity * (1.0 + 1.0 / denominator) ** 2 / symbol)

    unique_keys = sorted(set(keys))
    shell_lookup = {key: index for index, key in enumerate(unique_keys)}
    shell_indices = np.asarray([shell_lookup[key] for key in keys],
                               dtype=np.int32)
    weights_array = np.asarray(weights, dtype=np.float64)
    weight_sum = math.fsum(
        len(orbit) * weight for orbit, weight in zip(members, weights))
    maximum_permutations = max(len(value) for value in permutations)
    permutation_array = np.zeros(
        (maximum_permutations, len(representatives), 3), dtype=np.int16)
    permutation_valid = np.zeros(
        (maximum_permutations, len(representatives)), dtype=np.float64)
    for orbit_index, values in enumerate(permutations):
        for slot, value in enumerate(values):
            permutation_array[slot, orbit_index] = value
            permutation_valid[slot, orbit_index] = 1.0
    factor = np.empty((half + 1, half + 1), dtype=np.float64)
    for magnitude in range(half + 1):
        for displacement in range(half + 1):
            factor[magnitude, displacement] = (
                1.0 if magnitude == 0 else
                2.0 * math.cos(2.0 * math.pi * magnitude * displacement
                               / lattice_size))

    def kernel(displacement: tuple[int, int, int]) -> float:
        character = np.zeros(len(representatives), dtype=np.float64)
        for slot in range(maximum_permutations):
            permutation = permutation_array[slot]
            character += (
                permutation_valid[slot]
                * factor[permutation[:, 0], displacement[0]]
                * factor[permutation[:, 1], displacement[1]]
                * factor[permutation[:, 2], displacement[2]])
        shell_values = np.bincount(
            shell_indices, weights=weights_array * character,
            minlength=len(unique_keys))
        return math.fsum(abs(float(value)) for value in shell_values) / weight_sum

    axial = kernel((0, 0, 1))
    second = -math.inf
    second_displacement = (0, 0, 0)
    shared_maximum = -math.inf
    displacement_count = 0
    for dx in range(half + 1):
        for dy in range(dx, half + 1):
            for dz in range(dy, half + 1):
                displacement = (dx, dy, dz)
                if displacement == (0, 0, 0):
                    continue
                displacement_count += 1
                value = kernel(displacement)
                shared_maximum = max(shared_maximum, value)
                if displacement != (0, 0, 1) and value > second:
                    second = value
                    second_displacement = displacement

    def direct_kernel(displacement: tuple[int, int, int]) -> tuple[float, float]:
        shell_values = [0.0] * len(unique_keys)
        maximum_imaginary = 0.0
        for orbit_index, orbit in enumerate(members):
            real = math.fsum(
                math.cos(2.0 * math.pi * (
                    sum(mode[axis] * displacement[axis]
                        for axis in range(3)) % lattice_size)
                         / lattice_size)
                for mode in orbit)
            imaginary = math.fsum(
                -math.sin(2.0 * math.pi * (
                    sum(mode[axis] * displacement[axis]
                        for axis in range(3)) % lattice_size)
                          / lattice_size)
                for mode in orbit)
            maximum_imaginary = max(maximum_imaginary, abs(imaginary))
            shell_values[shell_indices[orbit_index]] += (
                weights[orbit_index] * real)
        return (
            math.fsum(abs(value) for value in shell_values) / weight_sum,
            maximum_imaginary / weight_sum,
        )

    covariance = 0.0
    for displacement in AXIAL:
        direct, imaginary = direct_kernel(displacement)
        covariance = max(covariance, abs(direct - axial), imaginary)
    direct_second, second_imaginary = direct_kernel(second_displacement)
    direct_residual = max(abs(direct_second - second), second_imaginary)

    volume = lattice_size ** 3
    pulse_operator = G_C / (C2 * volume) * math.sqrt(
        math.fsum(pulse_terms) * weight_sum)
    common_step = G_C / C2 * math.sqrt(math.fsum(step_terms) / volume)
    gram = [0.0] * 10
    pair_bounds = [0.0] * 11
    for removed in range(10):
        pairs = removed * (removed - 1) / 2.0
        gram[removed] = removed + 2.0 * (
            edge_caps[removed] * axial
            + (pairs - edge_caps[removed]) * second)
        pair_bounds[removed] = (
            common_step * math.sqrt(10 - removed)
            + pulse_operator * math.sqrt(max(0.0, gram[removed])))
    pair_bounds[10] = pulse_operator * math.sqrt(
        10.0 + shared_maximum * 10.0 * 9.0)
    maximizing_removed = max(range(11), key=pair_bounds.__getitem__)
    maximum_bound = pair_bounds[maximizing_removed]
    return {
        "lattice_size": lattice_size,
        "cyclotomic_degree": len(phi) - 1,
        "mode_orbits": len(representatives),
        "eigenvalue_shell_count": len(unique_keys),
        "exact_key_invariance": exact_invariant,
        "exact_mode_coverage": (
            disjoint and len(covered) == lattice_size ** 3 - 1),
        "displacement_orbits": displacement_count,
        "axial_kernel": axial,
        "second_kernel": second,
        "second_kernel_displacement": list(second_displacement),
        "shared_maximum": shared_maximum,
        "axial_covariance_residual": covariance,
        "direct_kernel_residual": direct_residual,
        "pulse_operator_coefficient": pulse_operator,
        "common_step_coefficient": common_step,
        "axial_edge_caps": edge_caps,
        "pair_gram_factors": gram,
        "pair_partition_bounds": pair_bounds,
        "pair_distance_bound": maximum_bound,
        "maximizing_removed_count": maximizing_removed,
        "margin": K_GENESIS - maximum_bound,
    }


P = Proof()
actual_prereg = hashlib.sha256(PREREG.read_bytes()).hexdigest().upper()
P.check("frozen preregistration hash", actual_prereg == PREREG_SHA,
        actual_prereg)
record = json.loads(RESULT.read_text(encoding="utf-8"))
P.check("run identifier", record["identifier"] == "FTD-0595",
        record["identifier"])
P.check("recorded preregistration hash",
        record["preregistration_sha256"] == actual_prereg,
        record["preregistration_sha256"])
for field, path in ARTIFACTS.items():
    actual_hash = hashlib.sha256(path.read_bytes()).hexdigest().upper()
    P.check(f"artifact hash {field}", record[field] == actual_hash,
            actual_hash)
P.check("registered source count", record["registered_source_count"] == 10,
        str(record["registered_source_count"]))
P.check("registered threshold", record["threshold"] == K_GENESIS,
        str(record["threshold"]))
P.check("registered kernel tolerance",
        record["kernel_tolerance"] == KERNEL_TOL,
        str(record["kernel_tolerance"]))
P.check("registered cross-language tolerance",
        record["cross_language_tolerance"] == CROSS_TOL,
        str(record["cross_language_tolerance"]))
P.check("run record valid", record["valid"] is True, str(record["valid"]))
P.check("no threshold-directed shape",
        record["threshold_dependent_shape_selected"] is False,
        str(record["threshold_dependent_shape_selected"]))
P.check("no geometry/history search",
        record["geometry_history_search_performed"] is False,
        str(record["geometry_history_search_performed"]))
P.check("no schedule search",
        record["removal_schedule_search_performed"] is False,
        str(record["removal_schedule_search_performed"]))
P.check("production unchanged", record["production_changed"] is False,
        str(record["production_changed"]))

animals = enumerate_animals()
for name, key in (("L=9", "l9"), ("L=17", "l17")):
    expected = record["animals"][key]
    counts = animals[f"{key}_counts"]
    edges = animals[f"{key}_edges"]
    P.check(f"{name} exact animal counts",
            counts == expected["canonical_animal_counts"], str(counts[1:]))
    P.check(f"{name} exact edge maxima",
            edges == expected["maximum_axial_edges"], str(edges[1:]))
    for size in range(1, 10):
        P.check(f"{name} size={size} animal count",
                counts[size] == expected["canonical_animal_counts"][size],
                str(counts[size]))
        P.check(f"{name} size={size} edge cap",
                edges[size] == expected["maximum_axial_edges"][size],
                str(edges[size]))

derived_rows: list[dict[str, object]] = []
for lattice_size, expected in zip(VOLUMES, record["volumes"]):
    edge_key = "l9_edges" if lattice_size == 9 else "l17_edges"
    derived = analyze_volume(lattice_size, animals[edge_key])
    derived_rows.append(derived)
    P.check(f"L={lattice_size} exact key invariance",
            bool(derived["exact_key_invariance"]), "all orbit members")
    P.check(f"L={lattice_size} exact mode coverage",
            bool(derived["exact_mode_coverage"]),
            f"orbits={derived['mode_orbits']}")
    P.check(f"L={lattice_size} displacement coverage",
            derived["displacement_orbits"] == derived["mode_orbits"],
            str(derived["displacement_orbits"]))
    for key in (
            "cyclotomic_degree", "mode_orbits", "eigenvalue_shell_count",
            "maximizing_removed_count"):
        P.check(f"L={lattice_size} C++/Python {key}",
                derived[key] == expected[key], str(derived[key]))
    for key in (
            "axial_kernel", "second_kernel", "axial_covariance_residual",
            "direct_kernel_residual", "pulse_operator_coefficient",
            "common_step_coefficient", "pair_distance_bound", "margin"):
        residual = abs(float(derived[key]) - float(expected[key]))
        P.check(f"L={lattice_size} C++/Python {key}",
                residual <= CROSS_TOL, f"residual={residual:.3e}")
    P.check(f"L={lattice_size} second displacement",
            derived["second_kernel_displacement"]
            == expected["second_kernel_displacement"],
            str(derived["second_kernel_displacement"]))
    P.check(f"L={lattice_size} cubic covariance",
            float(derived["axial_covariance_residual"]) <= KERNEL_TOL,
            f"{derived['axial_covariance_residual']:.3e}")
    P.check(f"L={lattice_size} direct kernel",
            float(derived["direct_kernel_residual"]) <= KERNEL_TOL,
            f"{derived['direct_kernel_residual']:.3e}")
    P.check(f"L={lattice_size} axial class maximal",
            float(derived["axial_kernel"])
            >= float(derived["second_kernel"]) - KERNEL_TOL,
            f"delta={float(derived['axial_kernel'])-float(derived['second_kernel']):.3e}")
    for key in ("axial_edge_caps", "pair_gram_factors",
                "pair_partition_bounds"):
        for index, (actual, target) in enumerate(zip(
                derived[key], expected[key])):
            residual = abs(float(actual) - float(target))
            P.check(f"L={lattice_size} {key}[{index}]",
                    residual <= CROSS_TOL, f"residual={residual:.3e}")

all_closed = all(float(row["pair_distance_bound"]) < K_GENESIS
                 for row in derived_rows)
expected_verdict = (
    "ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_PAIR_DISTANCE_CAPACITY"
    if all_closed else "TEN_SOURCE_PAIR_DISTANCE_BOUND_INCONCLUSIVE")
P.check("registered verdict", record["verdict"] == expected_verdict,
        record["verdict"])
P.check("registered pair-distance result is inconclusive", not all_closed,
        "at least one registered maximum exceeds threshold")

raise SystemExit(0 if P.report() else 1)
