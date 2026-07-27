#!/usr/bin/env python3
"""Independent verifier for preregistered FTD-0594.

The verifier constructs cyclotomic polynomials over the integers, reduces the
exact key R_L=6M in Z[x]/(Phi_L), compares every C++ shell key/multiplicity,
and independently recomputes the shared-M displacement norm and all N=10
partition bounds.  It performs no source-history search.
"""

from __future__ import annotations

from collections import defaultdict
import hashlib
import itertools
import json
import math
from pathlib import Path
import subprocess

import numpy as np


ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "PREREG_TEN_SOURCE_SHARED_M_COHERENCE_v1.md"
)
PREREG_SHA = "F7E04AA0E1B417CC856C58C2B60A4AEABF8D81CA0B766DF5756AC4CEF8A83E25"
RESULT = ROOT / "engine/results/ftd_0594/windows_msvc_cpu.json"
EXECUTABLE = (
    ROOT / "engine/build/Release/test_ten_source_shared_m_coherence.exe"
)
ARTIFACTS = {
    "header_sha256": ROOT / "engine/include/ftd/eft/"
    "ten_source_shared_m_coherence.h",
    "source_sha256": ROOT / "engine/src/eft/"
    "ten_source_shared_m_coherence.cpp",
    "test_sha256": ROOT / "engine/tests/"
    "test_ten_source_shared_m_coherence.cpp",
    "proof_sha256": Path(__file__).resolve(),
}
VOLUMES = (9, 17, 33, 65)
G_C = 0.0854245431028543695
C2 = 1.0 / 3.0
K_GENESIS = 1.5163860591519780
ORBIT_TOL = 5.0e-14
CHARACTER_TOL = 5.0e-13
SHELL_TOL = 5.0e-13
CROSS_TOL = 5.0e-12


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: str) -> None:
        self.rows.append((bool(condition), name, note))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0594 ten-source exact shared-M coherence")
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


def divisors(n: int) -> list[int]:
    return [value for value in range(1, n + 1) if n % value == 0]


def cyclotomic(n: int, cache: dict[int, list[int]]) -> list[int]:
    if n in cache:
        return cache[n]
    value = [-1] + [0] * (n - 1) + [1]
    for divisor in divisors(n):
        if divisor != n:
            value = divide_exact_monic(value, cyclotomic(divisor, cache))
    cache[n] = value
    return value


def cyclotomic_identity(lattice_size: int,
                        cache: dict[int, list[int]]) -> bool:
    product = [1]
    for divisor in divisors(lattice_size):
        product = multiply(product, cyclotomic(divisor, cache))
    expected = [-1] + [0] * (lattice_size - 1) + [1]
    return product == expected


def exact_key(lattice_size: int, mode: tuple[int, int, int],
              phi: list[int]) -> tuple[int, ...]:
    value = [0] * lattice_size

    def add(exponent: int, coefficient: int) -> None:
        value[exponent % lattice_size] += coefficient

    add(0, 24)
    for component in mode:
        add(component, -2)
        add(-component, -2)
    for i in range(3):
        for j in range(i + 1, 3):
            for si in (-1, 1):
                for sj in (-1, 1):
                    add(si * mode[i] + sj * mode[j], -1)
    degree = len(phi) - 1
    for index in range(len(value) - 1, degree - 1, -1):
        coefficient = value[index]
        if not coefficient:
            continue
        shift = index - degree
        for j, phi_coefficient in enumerate(phi):
            value[shift + j] -= coefficient * phi_coefficient
    return tuple(value[:degree])


def serialize_key(key: tuple[int, ...]) -> str:
    return ":".join(str(value) for value in key)


def permutations(value: tuple[int, int, int]) -> list[tuple[int, int, int]]:
    return sorted(set(itertools.permutations(value)))


def members(lattice_size: int,
            value: tuple[int, int, int]) -> set[tuple[int, int, int]]:
    out: set[tuple[int, int, int]] = set()
    for permutation in permutations(value):
        sign_sets = [(-1, 1) if component else (1,)
                     for component in permutation]
        for signs in itertools.product(*sign_sets):
            out.add(tuple((sign * component) % lattice_size
                          for sign, component in zip(signs, permutation)))
    return out


def symbol_and_gradient(
        lattice_size: int,
        mode: tuple[int, int, int]) -> tuple[float, float]:
    angles = [2.0 * math.pi * component / lattice_size for component in mode]
    cx, cy, cz = (math.cos(angle) for angle in angles)
    symbol = (
        4.0 - (2.0 / 3.0) * (cx + cy + cz)
        - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz)
    )
    gradient2 = math.fsum(math.sin(angle) ** 2 for angle in angles)
    return symbol, gradient2


def analyze(lattice_size: int) -> dict[str, object]:
    cache: dict[int, list[int]] = {}
    phi = cyclotomic(lattice_size, cache)
    identity_exact = cyclotomic_identity(lattice_size, cache)
    half = lattice_size // 2
    representatives = [
        (a, b, c)
        for a in range(half + 1)
        for b in range(a, half + 1)
        for c in range(b, half + 1)
        if (a, b, c) != (0, 0, 0)
    ]

    orbit_members: list[set[tuple[int, int, int]]] = []
    permutation_lists: list[list[tuple[int, int, int]]] = []
    keys: list[tuple[int, ...]] = []
    all_members: set[tuple[int, int, int]] = set()
    exact_disjoint = True
    key_invariance = True
    weights: list[float] = []
    pulse_terms: list[float] = []
    weight_terms: list[float] = []
    step_terms: list[float] = []
    max_invariance = 0.0

    for representative in representatives:
        orbit = members(lattice_size, representative)
        key = exact_key(lattice_size, representative, phi)
        key_invariance = key_invariance and all(
            exact_key(lattice_size, member, phi) == key
            for member in orbit
        )
        exact_disjoint = exact_disjoint and all_members.isdisjoint(orbit)
        all_members.update(orbit)
        orbit_members.append(orbit)
        permutation_lists.append(permutations(representative))
        keys.append(key)
        symbol, gradient2 = symbol_and_gradient(lattice_size, representative)
        weight = gradient2 / symbol
        denominator = math.sqrt(1.0 - C2 * symbol / 4.0)
        pulse = 2.0 / denominator
        step = 1.0 + 1.0 / denominator
        multiplicity = len(orbit)
        weights.append(weight)
        pulse_terms.append(multiplicity * pulse * pulse / symbol)
        weight_terms.append(multiplicity * weight)
        step_terms.append(multiplicity * step * step / symbol)
        for mode in orbit:
            member_symbol, member_gradient2 = symbol_and_gradient(
                lattice_size, mode)
            max_invariance = max(
                max_invariance,
                abs(member_symbol - symbol),
                abs(member_gradient2 - gradient2),
                abs(member_gradient2 / member_symbol - weight),
            )

    exact_coverage = (
        exact_disjoint
        and len(all_members) == lattice_size**3 - 1
        and (0, 0, 0) not in all_members
    )
    unique_keys = sorted(set(keys))
    shell_by_key = {key: index for index, key in enumerate(unique_keys)}
    shell_indices = np.asarray(
        [shell_by_key[key] for key in keys], dtype=np.int32)
    grouped: dict[tuple[int, ...], list[int]] = defaultdict(list)
    for orbit_index, key in enumerate(keys):
        grouped[key].append(orbit_index)
    shell_records = {
        serialize_key(key): (
            len(indices),
            sum(len(orbit_members[index]) for index in indices),
        )
        for key, indices in grouped.items()
    }
    shell_mode_count = sum(value[1] for value in shell_records.values())
    multi_shells = sum(value[0] > 1 for value in shell_records.values())
    max_orbits = max(value[0] for value in shell_records.values())

    orbit_count = len(representatives)
    permutation_count = max(len(value) for value in permutation_lists)
    permutation_array = np.zeros(
        (permutation_count, orbit_count, 3), dtype=np.int16)
    permutation_valid = np.zeros(
        (permutation_count, orbit_count), dtype=np.float64)
    for orbit_index, values in enumerate(permutation_lists):
        for slot, value in enumerate(values):
            permutation_array[slot, orbit_index, :] = value
            permutation_valid[slot, orbit_index] = 1.0

    factor = np.empty((half + 1, half + 1), dtype=np.float64)
    for magnitude in range(half + 1):
        for displacement in range(half + 1):
            factor[magnitude, displacement] = (
                1.0 if magnitude == 0 else
                2.0 * math.cos(2.0 * math.pi * magnitude * displacement
                               / lattice_size)
            )

    weights_array = np.asarray(weights, dtype=np.float64)
    maximum_shared = -math.inf
    maximum_orbit = -math.inf
    maximizing = (0, 0, 0)
    displacement_count = 0
    for dx in range(half + 1):
        for dy in range(dx, half + 1):
            for dz in range(dy, half + 1):
                if (dx, dy, dz) == (0, 0, 0):
                    continue
                displacement_count += 1
                character = np.zeros(orbit_count, dtype=np.float64)
                for slot in range(permutation_count):
                    p = permutation_array[slot]
                    character += (
                        permutation_valid[slot]
                        * factor[p[:, 0], dx]
                        * factor[p[:, 1], dy]
                        * factor[p[:, 2], dz]
                    )
                contributions = weights_array * character
                orbit_numerator = math.fsum(
                    abs(float(value)) for value in contributions)
                shell_values = np.bincount(
                    shell_indices, weights=contributions,
                    minlength=len(unique_keys))
                shared_numerator = math.fsum(
                    abs(float(value)) for value in shell_values)
                maximum_orbit = max(maximum_orbit, orbit_numerator)
                if shared_numerator > maximum_shared:
                    maximum_shared = shared_numerator
                    maximizing = (dx, dy, dz)

    direct_characters: list[float] = []
    max_character_residual = 0.0
    for orbit, values in zip(orbit_members, permutation_lists):
        formula = math.fsum(
            math.prod(
                1.0 if magnitude == 0 else
                2.0 * math.cos(2.0 * math.pi * magnitude * displacement
                               / lattice_size)
                for magnitude, displacement in zip(value, maximizing)
            )
            for value in values
        )
        direct_real = math.fsum(
            math.cos(2.0 * math.pi * (
                sum(mode[i] * maximizing[i] for i in range(3))
                % lattice_size) / lattice_size)
            for mode in orbit
        )
        direct_imag = math.fsum(
            -math.sin(2.0 * math.pi * (
                sum(mode[i] * maximizing[i] for i in range(3))
                % lattice_size) / lattice_size)
            for mode in orbit
        )
        max_character_residual = max(
            max_character_residual,
            abs(formula - direct_real),
            abs(direct_imag),
        )
        direct_characters.append(direct_real)

    pulse_sum = math.fsum(pulse_terms)
    weight_sum = math.fsum(weight_terms)
    common_step_sum = math.fsum(step_terms)
    direct_contributions = weights_array * np.asarray(direct_characters)
    direct_shell_values = np.bincount(
        shell_indices, weights=direct_contributions,
        minlength=len(unique_keys))
    direct_shared = math.fsum(abs(float(value))
                              for value in direct_shell_values)
    shell_residual = abs(maximum_shared - direct_shared) / weight_sum
    shared_coherence = maximum_shared / weight_sum
    orbit_coherence = maximum_orbit / weight_sum
    volume = lattice_size**3
    pulse_operator = G_C / (C2 * volume) * math.sqrt(
        pulse_sum * weight_sum)
    common_step = G_C / C2 * math.sqrt(common_step_sum / volume)
    partition_bounds = [
        common_step * math.sqrt(10 - removed)
        + pulse_operator * math.sqrt(
            removed + shared_coherence * removed * (removed - 1)
        )
        for removed in range(11)
    ]
    maximizing_removed = max(range(11), key=partition_bounds.__getitem__)
    maximum_bound = partition_bounds[maximizing_removed]
    return {
        "lattice_size": lattice_size,
        "cyclotomic_degree": len(phi) - 1,
        "cyclotomic_identity_exact": identity_exact,
        "exact_key_invariance": key_invariance,
        "exact_coverage": exact_coverage,
        "nonzero_modes": len(all_members),
        "mode_orbits": orbit_count,
        "displacement_orbits": displacement_count,
        "eigenvalue_shell_count": len(unique_keys),
        "multi_orbit_shell_count": multi_shells,
        "maximum_orbits_per_shell": max_orbits,
        "shell_mode_count": shell_mode_count,
        "shell_records": shell_records,
        "maximizing_displacement": list(maximizing),
        "maximum_shared_m_coherence": shared_coherence,
        "orbit_coherence_recomputed": orbit_coherence,
        "coherence_improvement": orbit_coherence - shared_coherence,
        "pulse_operator_coefficient": pulse_operator,
        "common_step_coefficient": common_step,
        "partition_bounds": partition_bounds,
        "ten_source_shared_m_bound": maximum_bound,
        "maximizing_removed_count": maximizing_removed,
        "margin": K_GENESIS - maximum_bound,
        "maximum_orbit_invariance_residual": max_invariance,
        "maximum_character_residual": max_character_residual,
        "shell_regrouping_residual": shell_residual,
    }


def cpp_shell_records() -> dict[int, dict[str, tuple[int, int]]]:
    completed = subprocess.run(
        [str(EXECUTABLE), "--dump-shells"], check=True,
        capture_output=True, text=True)
    records: dict[int, dict[str, tuple[int, int]]] = defaultdict(dict)
    for line in completed.stdout.splitlines():
        if not line.startswith("shell,"):
            continue
        fields = line.split(",")
        lattice_size = int(fields[2])
        records[lattice_size][fields[4]] = (int(fields[6]), int(fields[8]))
    return records


P = Proof()
actual_prereg = hashlib.sha256(PREREG.read_bytes()).hexdigest().upper()
P.check("frozen preregistration hash", actual_prereg == PREREG_SHA,
        actual_prereg)
record = json.loads(RESULT.read_text(encoding="utf-8"))
P.check("run identifier", record["identifier"] == "FTD-0594",
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
P.check("registered orbit tolerance",
        record["orbit_invariance_tolerance"] == ORBIT_TOL,
        str(record["orbit_invariance_tolerance"]))
P.check("registered character tolerance",
        record["character_tolerance"] == CHARACTER_TOL,
        str(record["character_tolerance"]))
P.check("registered shell tolerance",
        record["shell_regrouping_tolerance"] == SHELL_TOL,
        str(record["shell_regrouping_tolerance"]))
P.check("registered cross-language tolerance",
        record["cross_language_tolerance"] == CROSS_TOL,
        str(record["cross_language_tolerance"]))
P.check("run record valid", record["valid"] is True, str(record["valid"]))
P.check("no approximate clustering",
        record["approximate_eigenvalue_clustering_used"] is False,
        str(record["approximate_eigenvalue_clustering_used"]))
P.check("no geometry search", record["geometry_search_performed"] is False,
        str(record["geometry_search_performed"]))
P.check("no schedule search",
        record["removal_schedule_search_performed"] is False,
        str(record["removal_schedule_search_performed"]))
P.check("production unchanged", record["production_changed"] is False,
        str(record["production_changed"]))

cpp_shells = cpp_shell_records()
derived_rows: list[dict[str, object]] = []
for lattice_size, expected in zip(VOLUMES, record["volumes"]):
    derived = analyze(lattice_size)
    derived_rows.append(derived)
    P.check(f"L={lattice_size} exact cyclotomic identity",
            bool(derived["cyclotomic_identity_exact"]),
            f"degree={derived['cyclotomic_degree']}")
    P.check(f"L={lattice_size} exact key invariance",
            bool(derived["exact_key_invariance"]), "all orbit members")
    P.check(f"L={lattice_size} exact orbit coverage",
            bool(derived["exact_coverage"]),
            f"modes={derived['nonzero_modes']}")
    P.check(f"L={lattice_size} exact mode count",
            derived["nonzero_modes"] == lattice_size**3 - 1,
            str(derived["nonzero_modes"]))
    P.check(f"L={lattice_size} exact shell coverage",
            derived["shell_mode_count"] == lattice_size**3 - 1,
            str(derived["shell_mode_count"]))
    P.check(f"L={lattice_size} full C++ shell partition",
            derived["shell_records"] == cpp_shells[lattice_size],
            f"shells={len(cpp_shells[lattice_size])}")
    for key in (
        "cyclotomic_degree",
        "eigenvalue_shell_count",
        "multi_orbit_shell_count",
        "maximum_orbits_per_shell",
        "shell_mode_count",
        "maximizing_removed_count",
    ):
        P.check(f"L={lattice_size} C++/Python {key}",
                derived[key] == expected[key], str(derived[key]))
    for key in (
        "maximum_shared_m_coherence",
        "orbit_coherence_recomputed",
        "coherence_improvement",
        "pulse_operator_coefficient",
        "common_step_coefficient",
        "ten_source_shared_m_bound",
        "margin",
        "maximum_orbit_invariance_residual",
        "maximum_character_residual",
        "shell_regrouping_residual",
    ):
        residual = abs(float(derived[key]) - float(expected[key]))
        P.check(f"L={lattice_size} C++/Python {key}",
                residual <= CROSS_TOL, f"residual={residual:.3e}")
    P.check(f"L={lattice_size} maximizing displacement",
            derived["maximizing_displacement"]
            == expected["maximizing_displacement"],
            str(derived["maximizing_displacement"]))
    P.check(f"L={lattice_size} orbit invariance",
            float(derived["maximum_orbit_invariance_residual"]) <= ORBIT_TOL,
            f"{derived['maximum_orbit_invariance_residual']:.3e}")
    P.check(f"L={lattice_size} direct character",
            float(derived["maximum_character_residual"]) <= CHARACTER_TOL,
            f"{derived['maximum_character_residual']:.3e}")
    P.check(f"L={lattice_size} shell regrouping",
            float(derived["shell_regrouping_residual"]) <= SHELL_TOL,
            f"{derived['shell_regrouping_residual']:.3e}")
    P.check(f"L={lattice_size} shared-M no weaker",
            float(derived["maximum_shared_m_coherence"])
            <= float(derived["orbit_coherence_recomputed"]) + SHELL_TOL,
            f"delta={derived['coherence_improvement']:.3e}")
    for removed, (derived_bound, expected_bound) in enumerate(zip(
            derived["partition_bounds"], expected["partition_bounds"])):
        residual = abs(float(derived_bound) - float(expected_bound))
        P.check(f"L={lattice_size} r={removed} partition bound",
                residual <= CROSS_TOL, f"residual={residual:.3e}")

all_closed = all(
    float(row["ten_source_shared_m_bound"]) < K_GENESIS
    for row in derived_rows)
expected_verdict = (
    "ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_SHARED_M_COHERENCE"
    if all_closed else "TEN_SOURCE_SHARED_M_BOUND_INCONCLUSIVE"
)
P.check("registered verdict", record["verdict"] == expected_verdict,
        record["verdict"])
P.check("registered shared-M result is inconclusive", not all_closed,
        "at least one exact shared-M maximum exceeds threshold")

raise SystemExit(0 if P.report() else 1)
