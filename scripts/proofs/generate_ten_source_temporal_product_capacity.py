#!/usr/bin/env python3
"""Generate preregistered FTD-0597 temporal-product LP certificates.

The generator reuses the frozen FTD-0596 association scheme and LP machinery,
but replaces the absolute exact-shell kernel by the signed temporal product
kernel forced by -1/4 <= u_i u_j <= 1.  It performs no source, polarity,
schedule, observation-time, or history search.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
from typing import Any

import numpy as np

import generate_ten_source_distance_distribution_lp as base


ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "PREREG_TEN_SOURCE_TEMPORAL_PRODUCT_CAPACITY_v1.md"
)
PREREG_SHA = "7FF1D85959CE80932C3F60FBC0E39BEBC09E7567EF39724B166879F41843801D"
OUTPUT = ROOT / "engine/results/ftd_0597/solver_raw.json"
PARENT_RESULT = ROOT / "engine/results/ftd_0596/solver_raw.json"
VOLUMES = (9, 17, 33, 65)


class TemporalAssociationScheme(base.AssociationScheme):
    """FTD-0596 scheme carrying signed exact-shell masses and tau."""

    def _build_kernel(self) -> np.ndarray:
        count = len(self.representatives)
        temporal = np.empty(count, dtype=np.float64)
        parent = np.empty(count, dtype=np.float64)
        positive = np.empty(count, dtype=np.float64)
        negative = np.empty(count, dtype=np.float64)
        alternate = np.empty(count, dtype=np.float64)
        for displacement_index in range(count):
            contributions = (
                self.weights * self.character_vector(displacement_index))
            shells = np.bincount(
                self.shell_indices, weights=contributions,
                minlength=self.shell_count)
            p_value = math.fsum(
                max(float(value), 0.0) for value in shells)
            n_value = math.fsum(
                max(-float(value), 0.0) for value in shells)
            absolute = (p_value + n_value) / self.weight_sum
            signed = abs(p_value - n_value) / self.weight_sum
            tau = max(
                p_value + 0.25 * n_value,
                n_value + 0.25 * p_value) / self.weight_sum
            parent[displacement_index] = absolute
            positive[displacement_index] = p_value / self.weight_sum
            negative[displacement_index] = n_value / self.weight_sum
            temporal[displacement_index] = tau
            alternate[displacement_index] = (
                0.625 * absolute + 0.375 * signed)
        self.parent_kappa = parent
        self.positive_mass = positive
        self.negative_mass = negative
        self.alternate_tau = alternate
        return temporal


def parent_by_volume() -> dict[int, dict[str, Any]]:
    record = json.loads(PARENT_RESULT.read_text(encoding="utf-8"))
    return {int(row["lattice_size"]): row for row in record["volumes"]}


def analyze_volume(lattice_size: int,
                   parent_volume: dict[str, Any]) -> dict[str, Any]:
    print(f"building L={lattice_size} temporal association scheme",
          flush=True)
    scheme = TemporalAssociationScheme(lattice_size)
    partitions: list[dict[str, Any]] = [{
        "removed_count": 0,
        "gram_factor": 0.0,
        "partition_bound": scheme.common_step * math.sqrt(10.0),
    }, {
        "removed_count": 1,
        "gram_factor": 1.0,
        "partition_bound": scheme.common_step * 3.0
        + scheme.pulse_operator,
    }]
    for removed in range(2, 10):
        print(f"solving L={lattice_size} r={removed}", flush=True)
        partitions.append(base.solve_partition(scheme, removed))

    # The all-removed partition is deliberately inherited unchanged from
    # FTD-0596, as locked in the FTD-0597 protocol.
    parent_r10 = parent_volume["partitions"][10]
    partitions.append({
        "removed_count": 10,
        "gram_factor": float(parent_r10["gram_factor"]),
        "partition_bound": float(parent_r10["partition_bound"]),
    })
    maximizing = max(
        range(11), key=lambda index: partitions[index]["partition_bound"])
    maximum_bound = float(partitions[maximizing]["partition_bound"])
    maximum_index = int(np.argmax(scheme.kappa))
    maximum_alternate_residual = float(np.max(
        np.abs(scheme.kappa - scheme.alternate_tau)))
    maximum_parent_excess = float(np.max(
        scheme.kappa - scheme.parent_kappa))
    valid = (
        maximum_alternate_residual <= 5.0e-12
        and maximum_parent_excess <= 5.0e-12
        and maximum_bound
        <= float(parent_volume["distance_distribution_bound"]) + 1.0e-8
        and all(
            partition.get("minimum_fourier_value", 0.0)
            >= base.GLOBAL_FOURIER_TOL
            and partition.get("minimum_dual_slack", 0.0) >= -1.0e-12
            and partition.get("primal_dual_gap", 0.0) <= 1.0e-8
            for partition in partitions))
    return {
        "lattice_size": lattice_size,
        "cyclotomic_degree": len(base.cyclotomic(lattice_size, {})) - 1,
        "orbit_count": len(scheme.representatives),
        "shell_count": scheme.shell_count,
        "pulse_operator_coefficient": scheme.pulse_operator,
        "common_step_coefficient": scheme.common_step,
        "maximum_temporal_kernel": float(scheme.kappa[maximum_index]),
        "maximum_temporal_kernel_displacement": list(
            scheme.representatives[maximum_index]),
        "maximum_parent_kernel": float(np.max(scheme.parent_kappa)),
        "maximum_alternate_formula_residual": maximum_alternate_residual,
        "maximum_parent_excess": maximum_parent_excess,
        "temporal_kernel_values": [float(value) for value in scheme.kappa],
        "parent_kernel_values": [
            float(value) for value in scheme.parent_kappa],
        "positive_shell_mass_values": [
            float(value) for value in scheme.positive_mass],
        "negative_shell_mass_values": [
            float(value) for value in scheme.negative_mass],
        "partitions": partitions,
        "maximizing_removed_count": maximizing,
        "temporal_product_bound": maximum_bound,
        "parent_distance_distribution_bound": float(
            parent_volume["distance_distribution_bound"]),
        "margin": base.K_GENESIS - maximum_bound,
        "valid": valid,
    }


def write_certificate(path: Path, volumes: list[dict[str, Any]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow([
            "kind", "L", "r", "index", "value", "aux1", "aux2",
            "aux3", "aux4", "aux5", "aux6", "aux7", "aux8",
        ])
        for volume in volumes:
            writer.writerow([
                "volume", volume["lattice_size"], -1,
                volume["orbit_count"],
                format(volume["maximum_temporal_kernel"], ".17g"),
                volume["shell_count"],
                format(volume["pulse_operator_coefficient"], ".17g"),
                format(volume["common_step_coefficient"], ".17g"),
                volume["maximizing_removed_count"],
                format(volume["temporal_product_bound"], ".17g"),
                format(volume["margin"], ".17g"),
                ":".join(map(
                    str, volume["maximum_temporal_kernel_displacement"])),
                int(volume["valid"]),
            ])
            for index, tau in enumerate(volume["temporal_kernel_values"]):
                writer.writerow([
                    "tau", volume["lattice_size"], -1, index,
                    format(tau, ".17g"),
                    format(volume["parent_kernel_values"][index], ".17g"),
                    format(
                        volume["positive_shell_mass_values"][index], ".17g"),
                    format(
                        volume["negative_shell_mass_values"][index], ".17g"),
                    "", "", "", "", "",
                ])
            for partition in volume["partitions"]:
                removed = partition["removed_count"]
                writer.writerow([
                    "partition", volume["lattice_size"], removed, -1,
                    format(partition["partition_bound"], ".17g"),
                    format(partition["gram_factor"], ".17g"),
                    format(partition.get("primal_objective", 0.0), ".17g"),
                    format(
                        partition.get("certified_objective", 0.0), ".17g"),
                    format(partition.get("lambda", 0.0), ".17g"),
                    format(partition.get("epsilon", 0.0), ".17g"),
                    format(partition.get("delta", 0.0), ".17g"),
                    format(
                        partition.get("minimum_fourier_value", 0.0), ".17g"),
                    format(partition.get("minimum_dual_slack", 0.0), ".17g"),
                ])
                for kind, key in (("y", "active_cut_duals"),
                                  ("z", "upper_bound_duals"),
                                  ("a", "primal_distribution")):
                    for index, value in partition.get(key, []):
                        writer.writerow([
                            kind, volume["lattice_size"], removed, index,
                            format(value, ".17g"),
                            "", "", "", "", "", "", "", "",
                        ])


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=OUTPUT)
    parser.add_argument("--volumes", type=int, nargs="*", default=VOLUMES)
    args = parser.parse_args()
    actual_prereg = hashlib.sha256(PREREG.read_bytes()).hexdigest().upper()
    if actual_prereg != PREREG_SHA:
        raise RuntimeError(f"preregistration hash changed: {actual_prereg}")
    parents = parent_by_volume()
    rows = [analyze_volume(lattice_size, parents[lattice_size])
            for lattice_size in args.volumes]
    all_valid = all(row["valid"] for row in rows)
    all_closed = all(
        row["temporal_product_bound"] < base.K_GENESIS for row in rows)
    verdict = (
        "ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_TEMPORAL_PRODUCT_CAPACITY"
        if all_valid and all_closed else
        "TEN_SOURCE_TEMPORAL_PRODUCT_BOUND_INCONCLUSIVE"
        if all_valid else "PROTOCOL_INVALID")
    record = {
        "identifier": "FTD-0597",
        "date": "2026-07-26",
        "preregistration_sha256": actual_prereg,
        "generator_sha256": hashlib.sha256(
            Path(__file__).read_bytes()).hexdigest().upper(),
        "registered_source_count": 10,
        "threshold": base.K_GENESIS,
        "violated_cut_tolerance": base.VIOLATED_CUT_TOL,
        "global_fourier_tolerance": base.GLOBAL_FOURIER_TOL,
        "solver_tolerance": base.SOLVER_TOL,
        "coefficient_tolerance": base.COEFFICIENT_TOL,
        "dual_pad_floor": base.DUAL_PAD_FLOOR,
        "volumes": rows,
        "configuration_search_performed": False,
        "polarity_search_performed": False,
        "history_search_performed": False,
        "time_scan_performed": False,
        "extra_cut_added": False,
        "production_changed": False,
        "valid": all_valid,
        "verdict": verdict,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(record, indent=2) + "\n",
                           encoding="utf-8")
    write_certificate(args.output.with_name("solver_certificate.csv"), rows)
    print(json.dumps({
        "valid": all_valid,
        "verdict": verdict,
        "bounds": [row["temporal_product_bound"] for row in rows],
        "margins": [row["margin"] for row in rows],
        "maximum_temporal_kernels": [
            row["maximum_temporal_kernel"] for row in rows],
    }, indent=2))
    return 0 if all_valid else 1


if __name__ == "__main__":
    raise SystemExit(main())
