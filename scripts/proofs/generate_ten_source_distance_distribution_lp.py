#!/usr/bin/env python3
"""Generate the preregistered FTD-0596 Delsarte-LP certificates.

The generator performs no source-configuration or history search.  It builds
the complete cubic translation association scheme, solves the registered
Fourier-positive distance-distribution relaxation, and writes sparse padded
dual certificates for independent C++ and Python verification.
"""

from __future__ import annotations

from collections import defaultdict
import argparse
import csv
import hashlib
import itertools
import json
import math
from pathlib import Path
from typing import Any

import numpy as np
from scipy.optimize import linprog


ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "PREREG_TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_v1.md"
)
PREREG_SHA = "D69E9AFE8FCB2ECA487D285AC0B4A85D57FF1182B68FE613E32B0CADE7D3F2FA"
OUTPUT = ROOT / "engine/results/ftd_0596/solver_raw.json"
VOLUMES = (9, 17, 33, 65)
G_C = 0.0854245431028543695
C2 = 1.0 / 3.0
K_GENESIS = 1.5163860591519780
VIOLATED_CUT_TOL = -1.0e-12
GLOBAL_FOURIER_TOL = -1.0e-10
SOLVER_TOL = 1.0e-10
COEFFICIENT_TOL = 5.0e-12
DUAL_PAD_FLOOR = 1.0e-12
MAX_ITERATIONS = 64
EDGE_CAPS = (0, 0, 1, 2, 4, 5, 7, 9, 12, 13)


def trim(poly: list[int]) -> list[int]:
    while poly and poly[-1] == 0:
        poly.pop()
    return poly


def divide_exact_monic(numerator: list[int],
                       denominator: list[int]) -> list[int]:
    remainder = trim(numerator.copy())
    quotient = [0] * (len(remainder) - len(denominator) + 1)
    while len(remainder) >= len(denominator):
        shift = len(remainder) - len(denominator)
        coefficient = remainder[-1]
        quotient[shift] = coefficient
        for index, value in enumerate(denominator):
            remainder[shift + index] -= coefficient * value
        trim(remainder)
    if remainder:
        raise ValueError("non-exact cyclotomic division")
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
        if coefficient:
            shift = index - degree
            for phi_index, phi_coefficient in enumerate(phi):
                value[shift + phi_index] -= coefficient * phi_coefficient
    return tuple(value[:degree])


def unique_permutations(value: tuple[int, int, int]
                        ) -> list[tuple[int, int, int]]:
    return sorted(set(itertools.permutations(value)))


def orbit_members(lattice_size: int, value: tuple[int, int, int]
                  ) -> list[tuple[int, int, int]]:
    members: set[tuple[int, int, int]] = set()
    for permutation in unique_permutations(value):
        sign_sets = [(-1, 1) if component else (1,)
                     for component in permutation]
        for signs in itertools.product(*sign_sets):
            members.add(tuple(
                (sign * component) % lattice_size
                for sign, component in zip(signs, permutation)))
    return sorted(members)


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


class AssociationScheme:
    def __init__(self, lattice_size: int) -> None:
        self.lattice_size = lattice_size
        self.half = lattice_size // 2
        self.representatives = [
            (a, b, c)
            for a in range(self.half + 1)
            for b in range(a, self.half + 1)
            for c in range(b, self.half + 1)
            if (a, b, c) != (0, 0, 0)
        ]
        self.index_by_representative = {
            value: index for index, value in enumerate(self.representatives)
        }
        self.members = [orbit_members(lattice_size, value)
                        for value in self.representatives]
        self.multiplicities = np.asarray(
            [len(value) for value in self.members], dtype=np.float64)
        self.permutations = [unique_permutations(value)
                             for value in self.representatives]
        maximum_permutations = max(len(value) for value in self.permutations)
        orbit_count = len(self.representatives)
        self.permutation_array = np.zeros(
            (maximum_permutations, orbit_count, 3), dtype=np.int16)
        self.permutation_valid = np.zeros(
            (maximum_permutations, orbit_count), dtype=np.float64)
        for orbit_index, values in enumerate(self.permutations):
            for slot, value in enumerate(values):
                self.permutation_array[slot, orbit_index] = value
                self.permutation_valid[slot, orbit_index] = 1.0
        self.factor = np.empty(
            (self.half + 1, self.half + 1), dtype=np.float64)
        for magnitude in range(self.half + 1):
            for displacement in range(self.half + 1):
                self.factor[magnitude, displacement] = (
                    1.0 if magnitude == 0 else
                    2.0 * math.cos(
                        2.0 * math.pi * magnitude * displacement
                        / lattice_size))
        self._character_cache: dict[int, np.ndarray] = {}
        self._build_shell_data()
        self._build_lattice_orbit_index()
        self.kappa = self._build_kernel()

    def _build_shell_data(self) -> None:
        cache: dict[int, list[int]] = {}
        phi = cyclotomic(self.lattice_size, cache)
        keys = [exact_key(self.lattice_size, value, phi)
                for value in self.representatives]
        unique_keys = sorted(set(keys))
        shell_lookup = {key: index for index, key in enumerate(unique_keys)}
        self.shell_indices = np.asarray(
            [shell_lookup[key] for key in keys], dtype=np.int32)
        self.shell_count = len(unique_keys)
        weights: list[float] = []
        pulse_terms: list[float] = []
        step_terms: list[float] = []
        for representative, multiplicity in zip(
                self.representatives, self.multiplicities):
            symbol, gradient2 = symbol_and_gradient(
                self.lattice_size, representative)
            weight = gradient2 / symbol
            denominator = math.sqrt(1.0 - C2 * symbol / 4.0)
            weights.append(weight)
            pulse_terms.append(
                multiplicity * (2.0 / denominator) ** 2 / symbol)
            step_terms.append(
                multiplicity * (1.0 + 1.0 / denominator) ** 2 / symbol)
        self.weights = np.asarray(weights, dtype=np.float64)
        self.weight_sum = math.fsum(
            float(multiplicity * weight)
            for multiplicity, weight in zip(
                self.multiplicities, self.weights))
        volume = self.lattice_size ** 3
        self.pulse_operator = G_C / (C2 * volume) * math.sqrt(
            math.fsum(pulse_terms) * self.weight_sum)
        self.common_step = G_C / C2 * math.sqrt(
            math.fsum(step_terms) / volume)

    def _build_lattice_orbit_index(self) -> None:
        lattice_size = self.lattice_size
        self.lattice_orbit_index = np.full(
            (lattice_size, lattice_size, lattice_size), -1, dtype=np.int32)
        for orbit_index, members in enumerate(self.members):
            for x, y, z in members:
                if self.lattice_orbit_index[x, y, z] != -1:
                    raise RuntimeError("displacement orbit overlap")
                self.lattice_orbit_index[x, y, z] = orbit_index
        if np.count_nonzero(self.lattice_orbit_index < 0) != 1:
            raise RuntimeError("incomplete displacement orbit coverage")

    def character_vector(self, orbit_index: int) -> np.ndarray:
        if orbit_index in self._character_cache:
            return self._character_cache[orbit_index]
        displacement = self.representatives[orbit_index]
        character = np.zeros(len(self.representatives), dtype=np.float64)
        for slot in range(self.permutation_array.shape[0]):
            permutation = self.permutation_array[slot]
            character += (
                self.permutation_valid[slot]
                * self.factor[permutation[:, 0], displacement[0]]
                * self.factor[permutation[:, 1], displacement[1]]
                * self.factor[permutation[:, 2], displacement[2]])
        self._character_cache[orbit_index] = character
        return character

    def character_row(self, momentum_orbit_index: int) -> np.ndarray:
        return (self.character_vector(momentum_orbit_index)
                / self.multiplicities)

    def _build_kernel(self) -> np.ndarray:
        values = np.empty(len(self.representatives), dtype=np.float64)
        for displacement_index in range(len(self.representatives)):
            contributions = (
                self.weights * self.character_vector(displacement_index))
            shells = np.bincount(
                self.shell_indices, weights=contributions,
                minlength=self.shell_count)
            values[displacement_index] = math.fsum(
                abs(float(value)) for value in shells) / self.weight_sum
        return values

    def fourier_values(self, distribution: np.ndarray) -> np.ndarray:
        lattice_size = self.lattice_size
        autocorrelation = np.zeros(
            (lattice_size, lattice_size, lattice_size), dtype=np.float64)
        autocorrelation[0, 0, 0] = 1.0
        for orbit_index, members in enumerate(self.members):
            per_displacement = (
                distribution[orbit_index] / len(members))
            for x, y, z in members:
                autocorrelation[x, y, z] = per_displacement
        transform = np.fft.fftn(autocorrelation).real
        return np.asarray(
            [transform[representative]
             for representative in self.representatives],
            dtype=np.float64)


def weighted_dual_certificate(
        kappa: np.ndarray, character_matrix: np.ndarray,
        cut_duals: np.ndarray, upper_bounds: np.ndarray, h: float
) -> dict[str, Any]:
    y = np.maximum(cut_duals, 0.0)
    pressure = character_matrix.T @ y if y.size else np.zeros_like(kappa)
    thresholds = kappa + pressure
    order = np.argsort(-thresholds, kind="stable")
    cumulative = 0.0
    lambda_value = float(thresholds[order[-1]])
    for index in order:
        cumulative += float(upper_bounds[index])
        lambda_value = float(thresholds[index])
        if cumulative + 1.0e-15 >= h:
            break
    z = np.maximum(thresholds - lambda_value, 0.0)
    lhs = lambda_value + z - pressure
    epsilon = max(0.0, float(np.max(kappa - lhs)))
    delta = COEFFICIENT_TOL * (1.0 + math.fsum(float(v) for v in y)) \
        + DUAL_PAD_FLOOR
    padded_lambda = lambda_value + epsilon + delta
    objective = (
        h * padded_lambda
        + math.fsum(float(value) for value in y)
        + float(np.dot(upper_bounds, z)))
    return {
        "y": y,
        "z": z,
        "lambda": lambda_value,
        "epsilon": epsilon,
        "delta": delta,
        "padded_lambda": padded_lambda,
        "objective": objective,
        "minimum_dual_slack": float(np.min(
            padded_lambda + z - pressure - kappa)),
    }


def solve_partition(scheme: AssociationScheme, removed: int
                    ) -> dict[str, Any]:
    orbit_count = len(scheme.representatives)
    h = float(removed - 1)
    upper_bounds = np.minimum(scheme.multiplicities, h)
    axial_index = scheme.index_by_representative[(0, 0, 1)]
    upper_bounds[axial_index] = min(
        upper_bounds[axial_index],
        2.0 * EDGE_CAPS[removed] / removed)
    active: list[int] = []
    active_set: set[int] = set()
    character_rows: list[np.ndarray] = []
    result = None
    fourier = None
    for iteration in range(1, MAX_ITERATIONS + 1):
        matrix = (np.vstack(character_rows)
                  if character_rows else np.empty((0, orbit_count)))
        result = linprog(
            c=-scheme.kappa,
            A_ub=-matrix if matrix.size else None,
            b_ub=np.ones(len(active), dtype=np.float64)
            if active else None,
            A_eq=np.ones((1, orbit_count), dtype=np.float64),
            b_eq=np.asarray([h]),
            bounds=list(zip(np.zeros(orbit_count), upper_bounds)),
            method="highs-ds",
            options={
                "dual_feasibility_tolerance": SOLVER_TOL,
                "primal_feasibility_tolerance": SOLVER_TOL,
                "presolve": True,
            },
        )
        if not result.success:
            raise RuntimeError(
                f"L={scheme.lattice_size} r={removed} LP failed: "
                f"{result.message}")
        fourier = scheme.fourier_values(result.x)
        violated = [
            index for index, value in enumerate(fourier)
            if value < VIOLATED_CUT_TOL and index not in active_set
        ]
        if not violated and float(np.min(fourier)) < 0.0:
            minimum = float(np.min(fourier))
            for index, value in enumerate(fourier):
                if (index not in active_set
                        and abs(float(value) - minimum) <= 1.0e-14):
                    violated = [index]
                    break
        if not violated:
            break
        for index in violated:
            active.append(index)
            active_set.add(index)
            character_rows.append(scheme.character_row(index))
    else:
        raise RuntimeError("cutting-plane iteration limit exceeded")
    if result is None or fourier is None:
        raise RuntimeError("LP did not execute")
    matrix = (np.vstack(character_rows)
              if character_rows else np.empty((0, orbit_count)))
    cut_duals = (-np.asarray(result.ineqlin.marginals)
                 if active else np.empty(0))
    certificate = weighted_dual_certificate(
        scheme.kappa, matrix, cut_duals, upper_bounds, h)
    primal_objective = -float(result.fun)
    certified_objective = float(certificate["objective"])
    sparse_distribution = [
        [index, float(value)]
        for index, value in enumerate(result.x) if abs(value) > 1.0e-18
    ]
    sparse_y = [
        [active[index], float(value)]
        for index, value in enumerate(certificate["y"])
        if abs(value) > 1.0e-18
    ]
    sparse_z = [
        [index, float(value)]
        for index, value in enumerate(certificate["z"])
        if abs(value) > 1.0e-18
    ]
    gram = removed * (1.0 + certified_objective)
    bound = scheme.common_step * math.sqrt(10 - removed) \
        + scheme.pulse_operator * math.sqrt(max(0.0, gram))
    return {
        "removed_count": removed,
        "iterations": iteration,
        "active_cut_count": len(active),
        "minimum_fourier_value": float(np.min(fourier)),
        "primal_objective": primal_objective,
        "certified_objective": certified_objective,
        "primal_dual_gap": certified_objective - primal_objective,
        "lambda": certificate["lambda"],
        "epsilon": certificate["epsilon"],
        "delta": certificate["delta"],
        "padded_lambda": certificate["padded_lambda"],
        "minimum_dual_slack": certificate["minimum_dual_slack"],
        "gram_factor": gram,
        "partition_bound": bound,
        "upper_bound_axial": float(upper_bounds[axial_index]),
        "active_cut_duals": sparse_y,
        "upper_bound_duals": sparse_z,
        "primal_distribution": sparse_distribution,
    }


def analyze_volume(lattice_size: int) -> dict[str, Any]:
    print(f"building L={lattice_size} association scheme", flush=True)
    scheme = AssociationScheme(lattice_size)
    partitions: list[dict[str, Any]] = []
    partitions.append({
        "removed_count": 0,
        "gram_factor": 0.0,
        "partition_bound": scheme.common_step * math.sqrt(10.0),
    })
    partitions.append({
        "removed_count": 1,
        "gram_factor": 1.0,
        "partition_bound": scheme.common_step * 3.0
        + scheme.pulse_operator,
    })
    for removed in range(2, 10):
        print(f"solving L={lattice_size} r={removed}", flush=True)
        partitions.append(solve_partition(scheme, removed))
    shared_maximum = float(np.max(scheme.kappa))
    r10_gram = 10.0 * (1.0 + 9.0 * shared_maximum)
    partitions.append({
        "removed_count": 10,
        "gram_factor": r10_gram,
        "partition_bound": scheme.pulse_operator * math.sqrt(r10_gram),
    })
    maximizing = max(
        range(11), key=lambda index: partitions[index]["partition_bound"])
    maximum_bound = float(partitions[maximizing]["partition_bound"])
    return {
        "lattice_size": lattice_size,
        "cyclotomic_degree": len(cyclotomic(lattice_size, {})) - 1,
        "orbit_count": len(scheme.representatives),
        "shell_count": scheme.shell_count,
        "pulse_operator_coefficient": scheme.pulse_operator,
        "common_step_coefficient": scheme.common_step,
        "maximum_kernel": shared_maximum,
        "maximum_kernel_displacement": list(
            scheme.representatives[int(np.argmax(scheme.kappa))]),
        "kernel_values": [float(value) for value in scheme.kappa],
        "partitions": partitions,
        "maximizing_removed_count": maximizing,
        "distance_distribution_bound": maximum_bound,
        "margin": K_GENESIS - maximum_bound,
        "valid": all(
            partition.get("minimum_fourier_value", 0.0)
            >= GLOBAL_FOURIER_TOL
            and partition.get("minimum_dual_slack", 0.0) >= -1.0e-12
            and partition.get("primal_dual_gap", 0.0) <= 1.0e-8
            for partition in partitions),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=OUTPUT)
    parser.add_argument("--volumes", type=int, nargs="*", default=VOLUMES)
    args = parser.parse_args()
    actual_prereg = hashlib.sha256(PREREG.read_bytes()).hexdigest().upper()
    if actual_prereg != PREREG_SHA:
        raise RuntimeError(
            f"preregistration hash changed: {actual_prereg}")
    rows = [analyze_volume(lattice_size) for lattice_size in args.volumes]
    all_valid = all(row["valid"] for row in rows)
    all_closed = all(
        row["distance_distribution_bound"] < K_GENESIS for row in rows)
    verdict = (
        "ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_DISTANCE_DISTRIBUTION_LP"
        if all_valid and all_closed else
        "TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_INCONCLUSIVE"
        if all_valid else "PROTOCOL_INVALID")
    record = {
        "identifier": "FTD-0596",
        "date": "2026-07-26",
        "preregistration_sha256": actual_prereg,
        "generator_sha256": hashlib.sha256(
            Path(__file__).read_bytes()).hexdigest().upper(),
        "registered_source_count": 10,
        "threshold": K_GENESIS,
        "violated_cut_tolerance": VIOLATED_CUT_TOL,
        "global_fourier_tolerance": GLOBAL_FOURIER_TOL,
        "solver_tolerance": SOLVER_TOL,
        "coefficient_tolerance": COEFFICIENT_TOL,
        "dual_pad_floor": DUAL_PAD_FLOOR,
        "volumes": rows,
        "configuration_search_performed": False,
        "history_search_performed": False,
        "extra_cut_added": False,
        "production_changed": False,
        "valid": all_valid,
        "verdict": verdict,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(record, indent=2) + "\n",
                           encoding="utf-8")
    certificate_path = args.output.with_name("solver_certificate.csv")
    with certificate_path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow([
            "kind", "L", "r", "index", "value", "aux1", "aux2",
            "aux3", "aux4", "aux5", "aux6", "aux7", "aux8",
        ])
        for volume in rows:
            writer.writerow([
                "volume", volume["lattice_size"], -1,
                volume["orbit_count"], format(volume["maximum_kernel"], ".17g"),
                volume["shell_count"],
                format(volume["pulse_operator_coefficient"], ".17g"),
                format(volume["common_step_coefficient"], ".17g"),
                volume["maximizing_removed_count"],
                format(volume["distance_distribution_bound"], ".17g"),
                format(volume["margin"], ".17g"),
                ":".join(map(str, volume["maximum_kernel_displacement"])),
                int(volume["valid"]),
            ])
            for orbit_index, value in enumerate(volume["kernel_values"]):
                writer.writerow([
                    "kappa", volume["lattice_size"], -1, orbit_index,
                    format(value, ".17g"), "", "", "", "", "", "", "", "",
                ])
            for partition in volume["partitions"]:
                removed = partition["removed_count"]
                writer.writerow([
                    "partition", volume["lattice_size"], removed, -1,
                    format(partition["partition_bound"], ".17g"),
                    format(partition["gram_factor"], ".17g"),
                    format(partition.get("primal_objective", 0.0), ".17g"),
                    format(partition.get("certified_objective", 0.0), ".17g"),
                    format(partition.get("lambda", 0.0), ".17g"),
                    format(partition.get("epsilon", 0.0), ".17g"),
                    format(partition.get("delta", 0.0), ".17g"),
                    format(partition.get("minimum_fourier_value", 0.0), ".17g"),
                    format(partition.get("minimum_dual_slack", 0.0), ".17g"),
                ])
                for index, value in partition.get("active_cut_duals", []):
                    writer.writerow([
                        "y", volume["lattice_size"], removed, index,
                        format(value, ".17g"), "", "", "", "", "", "", "", "",
                    ])
                for index, value in partition.get("upper_bound_duals", []):
                    writer.writerow([
                        "z", volume["lattice_size"], removed, index,
                        format(value, ".17g"), "", "", "", "", "", "", "", "",
                    ])
                for index, value in partition.get("primal_distribution", []):
                    writer.writerow([
                        "a", volume["lattice_size"], removed, index,
                        format(value, ".17g"), "", "", "", "", "", "", "", "",
                    ])
    print(json.dumps({
        "valid": all_valid,
        "verdict": verdict,
        "bounds": [row["distance_distribution_bound"] for row in rows],
        "margins": [row["margin"] for row in rows],
    }, indent=2))
    return 0 if all_valid else 1


if __name__ == "__main__":
    raise SystemExit(main())
