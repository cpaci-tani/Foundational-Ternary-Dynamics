#!/usr/bin/env python3
"""Independent verifier for the preregistered FTD-0590 orbit bound.

The script exhausts cubic mode and displacement orbits.  It does not optimize
source geometry, polarity, removal time, observation time, or field amplitude.
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
    "PREREG_REMOVAL_TIME_ORBIT_COHERENCE_v1.md"
)
PREREG_SHA = "E7C766CB3AD7062452F6AC1DDD9B3DC854F0DF6BCC6B2D32B1DC402281BD7721"
RESULT = ROOT / "engine/results/ftd_0590/windows_msvc_cpu.json"
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
        print("FTD-0590 removal-time cubic-orbit coherence")
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


def members(lattice_size: int, value: tuple[int, int, int]) -> set[tuple[int, int, int]]:
    out: set[tuple[int, int, int]] = set()
    for perm in permutations(value):
        sign_sets = [(-1, 1) if component else (1,) for component in perm]
        for signs in itertools.product(*sign_sets):
            out.add(tuple((sign * component) % lattice_size
                          for sign, component in zip(signs, perm)))
    return out


def symbol_and_gradient(lattice_size: int, mode: tuple[int, int, int]) -> tuple[float, float]:
    angles = [2.0 * math.pi * component / lattice_size for component in mode]
    cosines = [math.cos(angle) for angle in angles]
    sines = [math.sin(angle) for angle in angles]
    cx, cy, cz = cosines
    symbol = (
        4.0 - (2.0 / 3.0) * (cx + cy + cz)
        - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz)
    )
    gradient2 = sum(value * value for value in sines)
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
    pulse_sum_terms: list[float] = []
    weight_sum_terms: list[float] = []
    step_sum_terms: list[float] = []
    max_invariance = 0.0
    perm_lists: list[list[tuple[int, int, int]]] = []

    for representative in representatives:
        orbit = members(lattice_size, representative)
        exact_disjoint = exact_disjoint and all_members.isdisjoint(orbit)
        all_members.update(orbit)
        orbit_members.append(orbit)
        perm_lists.append(permutations(representative))
        symbol, gradient2 = symbol_and_gradient(lattice_size, representative)
        weight = gradient2 / symbol
        denominator = math.sqrt(1.0 - C2 * symbol / 4.0)
        pulse = 2.0 / denominator
        step = 1.0 + 1.0 / denominator
        multiplicity = len(orbit)
        weights.append(weight)
        pulse_sum_terms.append(multiplicity * pulse * pulse / symbol)
        weight_sum_terms.append(multiplicity * weight)
        step_sum_terms.append(multiplicity * step * step / symbol)
        for mode in orbit:
            member_symbol, member_gradient2 = symbol_and_gradient(lattice_size, mode)
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
    weights_array = np.asarray(weights, dtype=np.float64)
    orbit_count = len(representatives)
    perm_count = max(len(value) for value in perm_lists)
    perm_array = np.zeros((perm_count, orbit_count, 3), dtype=np.int16)
    perm_valid = np.zeros((perm_count, orbit_count), dtype=np.float64)
    for orbit_index, values in enumerate(perm_lists):
        for slot, value in enumerate(values):
            perm_array[slot, orbit_index, :] = value
            perm_valid[slot, orbit_index] = 1.0

    factor = np.empty((half + 1, half + 1), dtype=np.float64)
    for magnitude in range(half + 1):
        for displacement in range(half + 1):
            factor[magnitude, displacement] = (
                1.0 if magnitude == 0 else
                2.0 * math.cos(2.0 * math.pi * magnitude * displacement
                               / lattice_size)
            )

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
                for slot in range(perm_count):
                    p = perm_array[slot]
                    character += (
                        perm_valid[slot]
                        * factor[p[:, 0], dx]
                        * factor[p[:, 1], dy]
                        * factor[p[:, 2], dz]
                    )
                numerator = math.fsum(
                    float(value) for value in weights_array * np.abs(character)
                )
                if numerator > maximum_numerator:
                    maximum_numerator = numerator
                    maximizing = (dx, dy, dz)

    direct_terms: list[float] = []
    max_character_residual = 0.0
    for representative, orbit, values, weight in zip(
            representatives, orbit_members, perm_lists, weights):
        del representative
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

    pulse_sum = math.fsum(pulse_sum_terms)
    weight_sum = math.fsum(weight_sum_terms)
    common_step_sum = math.fsum(step_sum_terms)
    direct_numerator = math.fsum(direct_terms)
    max_character_residual = max(
        max_character_residual,
        abs(maximum_numerator - direct_numerator) / weight_sum,
    )
    coherence = maximum_numerator / weight_sum
    volume = lattice_size**3
    pulse_operator = G_C / (C2 * volume) * math.sqrt(pulse_sum * weight_sum)
    common_step = G_C / C2 * math.sqrt(common_step_sum / volume)
    bounds = [
        common_step * math.sqrt(7 - removed)
        + pulse_operator * math.sqrt(
            removed + coherence * removed * (removed - 1)
        )
        for removed in range(8)
    ]
    maximizing_removed = max(range(8), key=bounds.__getitem__)
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
        "seven_source_bound": bounds[maximizing_removed],
        "maximizing_removed_count": maximizing_removed,
        "margin": K_GENESIS - bounds[maximizing_removed],
        "maximum_orbit_invariance_residual": max_invariance,
        "maximum_character_residual": max_character_residual,
        "exact_coverage": exact_coverage,
    }


P = Proof()
actual_prereg = hashlib.sha256(PREREG.read_bytes()).hexdigest().upper()
P.check("frozen preregistration hash", actual_prereg == PREREG_SHA,
        actual_prereg)
record = json.loads(RESULT.read_text(encoding="utf-8"))
P.check("run identifier", record["identifier"] == "FTD-0590",
        record["identifier"])
P.check("run record valid", record["valid"] is True, str(record["valid"]))
P.check("no geometry search", record["geometry_search_performed"] is False,
        str(record["geometry_search_performed"]))
P.check("no schedule search", record["removal_schedule_search_performed"] is False,
        str(record["removal_schedule_search_performed"]))
P.check("production unchanged", record["production_changed"] is False,
        str(record["production_changed"]))

derived_rows = []
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
        "seven_source_bound",
        "margin",
    ):
        residual = abs(float(derived[key]) - float(expected[key]))
        P.check(f"L={lattice_size} C++/Python {key}",
                residual <= CROSS_TOL, f"residual={residual:.3e}")
    P.check(f"L={lattice_size} maximizing displacement",
            derived["maximizing_displacement"]
            == expected["maximizing_displacement"],
            str(derived["maximizing_displacement"]))
    P.check(f"L={lattice_size} maximizing removal partition",
            derived["maximizing_removed_count"] == 6
            == expected["maximizing_removed_count"],
            str(derived["maximizing_removed_count"]))
    P.check(f"L={lattice_size} coherence unit interval",
            0.0 <= float(derived["maximum_orbit_coherence"]) <= 1.0,
            f"{derived['maximum_orbit_coherence']:.17g}")
    P.check(f"L={lattice_size} strict N=7 closure",
            float(derived["seven_source_bound"]) < K_GENESIS,
            f"margin={derived['margin']:.17g}")

P.check(
    "registered verdict",
    record["verdict"]
    == "ARBITRARY_REMOVAL_N_LE_7_CLOSED_BY_ORBIT_COHERENCE",
    record["verdict"],
)
P.check(
    "uniform seven-source first-event bound",
    all(float(row["seven_source_bound"]) < K_GENESIS for row in derived_rows),
    "all four registered quotients are strict",
)

raise SystemExit(0 if P.report() else 1)

