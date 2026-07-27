#!/usr/bin/env python3
"""Independent verifier for preregistered FTD-0592.

The verifier reconstructs every cubic mode/displacement orbit and evaluates
all N=9 removal partitions.  It performs no source-geometry, polarity,
schedule, observation-time, or observation-site search.
"""

from __future__ import annotations

import hashlib
import itertools
import json
import math
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "PREREG_NINE_SOURCE_ORBIT_COHERENCE_v1.md"
)
PREREG_SHA = "DDAA7FC084C3F8F146E722F15E1089FDDA83D095EB5C55D2B31823A20BD41DE8"
RESULT = ROOT / "engine/results/ftd_0592/windows_msvc_cpu.json"
ARTIFACTS = {
    "header_sha256": ROOT / "engine/include/ftd/eft/"
    "nine_source_orbit_coherence.h",
    "source_sha256": ROOT / "engine/src/eft/"
    "nine_source_orbit_coherence.cpp",
    "test_sha256": ROOT / "engine/tests/"
    "test_nine_source_orbit_coherence.cpp",
    "proof_sha256": Path(__file__).resolve(),
}
VOLUMES = (9, 17, 33, 65)
G_C = 0.0854245431028543695
C2 = 1.0 / 3.0
K_GENESIS = 1.5163860591519780
ORBIT_TOL = 5.0e-14
CHARACTER_TOL = 5.0e-13
CROSS_TOL = 5.0e-12


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: str) -> None:
        self.rows.append((bool(condition), name, note))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0592 nine-source removal-time orbit coherence")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        print("-" * 79)
        print(f"checks={len(self.rows)} passed={passed} "
              f"failed={len(self.rows)-passed}")
        return passed == len(self.rows)


def permutations(value: tuple[int, int, int]) -> list[tuple[int, int, int]]:
    return sorted(set(itertools.permutations(value)))


def members(lattice_size: int,
            value: tuple[int, int, int]) -> set[tuple[int, int, int]]:
    out: set[tuple[int, int, int]] = set()
    for perm in permutations(value):
        sign_sets = [(-1, 1) if component else (1,) for component in perm]
        for signs in itertools.product(*sign_sets):
            out.add(tuple((sign * component) % lattice_size
                          for sign, component in zip(signs, perm)))
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
    half = lattice_size // 2
    representatives = [
        (a, b, c)
        for a in range(half + 1)
        for b in range(a, half + 1)
        for c in range(b, half + 1)
        if (a, b, c) != (0, 0, 0)
    ]

    orbit_members: list[set[tuple[int, int, int]]] = []
    all_members: set[tuple[int, int, int]] = set()
    exact_disjoint = True
    weights: list[float] = []
    pulse_terms: list[float] = []
    weight_terms: list[float] = []
    step_terms: list[float] = []
    max_invariance = 0.0
    permutation_lists: list[list[tuple[int, int, int]]] = []

    for representative in representatives:
        orbit = members(lattice_size, representative)
        exact_disjoint = exact_disjoint and all_members.isdisjoint(orbit)
        all_members.update(orbit)
        orbit_members.append(orbit)
        permutation_lists.append(permutations(representative))
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
    maximum_numerator = -math.inf
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
                numerator = math.fsum(
                    float(value)
                    for value in weights_array * np.abs(character)
                )
                if numerator > maximum_numerator:
                    maximum_numerator = numerator
                    maximizing = (dx, dy, dz)

    direct_terms: list[float] = []
    max_character_residual = 0.0
    for orbit, values, weight in zip(
            orbit_members, permutation_lists, weights):
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
        direct_terms.append(weight * abs(direct_real))

    pulse_sum = math.fsum(pulse_terms)
    weight_sum = math.fsum(weight_terms)
    common_step_sum = math.fsum(step_terms)
    direct_numerator = math.fsum(direct_terms)
    max_character_residual = max(
        max_character_residual,
        abs(maximum_numerator - direct_numerator) / weight_sum,
    )
    coherence = maximum_numerator / weight_sum
    volume = lattice_size**3
    pulse_operator = G_C / (C2 * volume) * math.sqrt(
        pulse_sum * weight_sum)
    common_step = G_C / C2 * math.sqrt(common_step_sum / volume)
    partition_bounds = [
        common_step * math.sqrt(9 - removed)
        + pulse_operator * math.sqrt(
            removed + coherence * removed * (removed - 1)
        )
        for removed in range(10)
    ]
    maximizing_removed = max(range(10), key=partition_bounds.__getitem__)
    maximum_bound = partition_bounds[maximizing_removed]
    return {
        "lattice_size": lattice_size,
        "nonzero_modes": len(all_members),
        "mode_orbits": orbit_count,
        "displacement_orbits": displacement_count,
        "maximizing_displacement": list(maximizing),
        "pulse_cauchy_sum": pulse_sum,
        "gradient_weight_sum": weight_sum,
        "pulse_operator_coefficient": pulse_operator,
        "maximum_orbit_coherence": coherence,
        "common_step_coefficient": common_step,
        "partition_bounds": partition_bounds,
        "nine_source_bound": maximum_bound,
        "maximizing_removed_count": maximizing_removed,
        "margin": K_GENESIS - maximum_bound,
        "maximum_orbit_invariance_residual": max_invariance,
        "maximum_character_residual": max_character_residual,
        "exact_coverage": exact_coverage,
    }


P = Proof()
actual_prereg = hashlib.sha256(PREREG.read_bytes()).hexdigest().upper()
P.check("frozen preregistration hash", actual_prereg == PREREG_SHA,
        actual_prereg)
record = json.loads(RESULT.read_text(encoding="utf-8"))
P.check("run identifier", record["identifier"] == "FTD-0592",
        record["identifier"])
P.check("recorded preregistration hash",
        record["preregistration_sha256"] == actual_prereg,
        record["preregistration_sha256"])
for field, path in ARTIFACTS.items():
    actual_hash = hashlib.sha256(path.read_bytes()).hexdigest().upper()
    P.check(f"artifact hash {field}", record[field] == actual_hash,
            actual_hash)
P.check("registered source count", record["registered_source_count"] == 9,
        str(record["registered_source_count"]))
P.check("registered threshold", record["threshold"] == K_GENESIS,
        str(record["threshold"]))
P.check("registered orbit tolerance",
        record["orbit_invariance_tolerance"] == ORBIT_TOL,
        str(record["orbit_invariance_tolerance"]))
P.check("registered character tolerance",
        record["character_tolerance"] == CHARACTER_TOL,
        str(record["character_tolerance"]))
P.check("registered cross-language tolerance",
        record["cross_language_tolerance"] == CROSS_TOL,
        str(record["cross_language_tolerance"]))
P.check("run record valid", record["valid"] is True, str(record["valid"]))
P.check("no geometry search", record["geometry_search_performed"] is False,
        str(record["geometry_search_performed"]))
P.check("no schedule search",
        record["removal_schedule_search_performed"] is False,
        str(record["removal_schedule_search_performed"]))
P.check("production unchanged", record["production_changed"] is False,
        str(record["production_changed"]))

derived_rows: list[dict[str, object]] = []
for lattice_size, expected in zip(VOLUMES, record["volumes"]):
    derived = analyze(lattice_size)
    derived_rows.append(derived)
    P.check(f"L={lattice_size} exact orbit coverage",
            bool(derived["exact_coverage"]),
            f"modes={derived['nonzero_modes']}")
    P.check(f"L={lattice_size} exact mode count",
            derived["nonzero_modes"] == lattice_size**3 - 1,
            str(derived["nonzero_modes"]))
    expected_orbits = math.comb(lattice_size // 2 + 3, 3) - 1
    P.check(f"L={lattice_size} complete canonical orbit count",
            derived["mode_orbits"] == expected_orbits
            and derived["displacement_orbits"] == expected_orbits,
            str(expected_orbits))
    P.check(f"L={lattice_size} orbit invariance",
            float(derived["maximum_orbit_invariance_residual"]) <= ORBIT_TOL,
            f"{derived['maximum_orbit_invariance_residual']:.3e}")
    P.check(f"L={lattice_size} direct character",
            float(derived["maximum_character_residual"]) <= CHARACTER_TOL,
            f"{derived['maximum_character_residual']:.3e}")
    for key in (
        "pulse_cauchy_sum",
        "gradient_weight_sum",
        "pulse_operator_coefficient",
        "maximum_orbit_coherence",
        "common_step_coefficient",
        "nine_source_bound",
        "margin",
        "maximum_orbit_invariance_residual",
        "maximum_character_residual",
    ):
        residual = abs(float(derived[key]) - float(expected[key]))
        P.check(f"L={lattice_size} C++/Python {key}",
                residual <= CROSS_TOL, f"residual={residual:.3e}")
    for removed, (derived_bound, expected_bound) in enumerate(zip(
            derived["partition_bounds"], expected["partition_bounds"])):
        residual = abs(float(derived_bound) - float(expected_bound))
        P.check(f"L={lattice_size} r={removed} partition bound",
                residual <= CROSS_TOL, f"residual={residual:.3e}")
    P.check(f"L={lattice_size} maximizing displacement",
            derived["maximizing_displacement"]
            == expected["maximizing_displacement"],
            str(derived["maximizing_displacement"]))
    P.check(f"L={lattice_size} maximizing removal partition",
            derived["maximizing_removed_count"]
            == expected["maximizing_removed_count"],
            str(derived["maximizing_removed_count"]))
    P.check(f"L={lattice_size} strict N=9 closure",
            float(derived["nine_source_bound"]) < K_GENESIS,
            f"margin={derived['margin']:.17g}")

all_closed = all(
    float(row["nine_source_bound"]) < K_GENESIS for row in derived_rows)
expected_verdict = (
    "ARBITRARY_REMOVAL_N_LE_9_CLOSED_BY_ORBIT_COHERENCE"
    if all_closed else "NINE_SOURCE_ORBIT_BOUND_INCONCLUSIVE"
)
P.check("registered verdict", record["verdict"] == expected_verdict,
        record["verdict"])
P.check("uniform nine-source first-event bound", all_closed,
        "all four registered quotients are strict")

raise SystemExit(0 if P.report() else 1)


