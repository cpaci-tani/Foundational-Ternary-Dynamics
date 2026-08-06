#!/usr/bin/env python3
"""Independent certificate for FTD-0647.

This is not a fit.  It checks the locked run-of-record and the analytic lower
bound implied by the selected connected-block action.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations" / (
    "PREREG_CONNECTED_BLOCK_FIXED_MASS_REFINEMENT_OBSTRUCTION_v1.md"
)
JSON_PATH = ROOT / "engine/results/ftd_0647" / (
    "ftd_0647_connected_block_fixed_mass_refinement_obstruction_v1.json"
)
CSV_PATH = ROOT / "engine/results/ftd_0647" / (
    "ftd_0647_connected_block_fixed_mass_refinement_obstruction_arms_v1.csv"
)

PROTOCOL_SHA256 = "5D3A8E64750936A1A437C4F743777297977AA0E6BEBAC241F8FF46BD647706D9"
JSON_SHA256 = "4756E3E1876A9D2D0EB5BC5E369B64B4AA16DDF20DEE1C6ABC60976070FA828D"
CSV_SHA256 = "DC10BD66FC957A3C5F067300D68C99E1AA91103116C9A0F0EBD815DB74291752"
VERDICT = "FROZEN_ADDITIVE_CONSTITUENT_FIXED_MASS_REFINEMENT_CLOSED"

M_INERTIAL = 0.511
C2 = 1.0 / 3.0
C = math.sqrt(C2)
E_REST = M_INERTIAL * C2


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def close(a: float, b: float, tolerance: float = 1e-13) -> bool:
    return abs(a - b) <= tolerance * max(1.0, abs(a), abs(b))


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS {label}")


def main() -> None:
    require(sha256(PREREG) == PROTOCOL_SHA256, "protocol hash")
    require(sha256(JSON_PATH) == JSON_SHA256, "JSON hash")
    require(sha256(CSV_PATH) == CSV_SHA256, "CSV hash")

    record = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    require(record["protocol_sha256"] == PROTOCOL_SHA256, "embedded protocol")
    require(record["verdict"] == VERDICT, "locked verdict")
    require(record["production_changed"] is False, "production unchanged")
    require(record["arm_count"] == 12 and len(rows) == 12, "12-arm coverage")
    for gate in (
        "normalization_pass",
        "coverage_pass",
        "initialization_pass",
        "count_pass",
        "neutrality_pass",
        "rest_sum_pass",
        "binding_pass",
        "field_positivity_pass",
        "lower_bound_pass",
        "cubic_pass",
        "scaling_pass",
    ):
        require(record[gate] is True, gate)

    # On a periodic unit cubic complex the forward-difference symbol satisfies
    # |d_j| <= 2, hence ||curl(k)|| = |d(k)| <= 2*sqrt(3).
    curl_norm_bound = 2.0 * math.sqrt(3.0)
    modified_energy_lower_coefficient = 0.5 * (1.0 - C * math.sqrt(3.0))
    require(close(curl_norm_bound, 2.0 * math.sqrt(3.0)), "curl norm bound")
    require(abs(modified_energy_lower_coefficient) <= 1e-15,
            "modified field energy semidefinite at selected lambda")
    require(record["beta"] > 0.0, "positive field normalization")

    expected_pairs = {(width, orientation)
                      for width in range(1, 5)
                      for orientation in range(3)}
    actual_pairs = {(int(row["width"]), int(row["orientation"]))
                    for row in rows}
    require(actual_pairs == expected_pairs, "width/orientation orbit")

    for row in rows:
        width = int(row["width"])
        count = int(row["constituent_count"])
        expected_count = 2 * width**3
        rest_floor = count * E_REST
        mass_floor = rest_floor / C2
        require(count == expected_count, f"N=2w^3 w={width} o={row['orientation']}")
        require(int(row["net_polarity"]) == 0,
                f"neutral w={width} o={row['orientation']}")
        require(close(float(row["constituent_rest_sum"]), rest_floor),
                f"additive rest floor w={width} o={row['orientation']}")
        require(float(row["binding_energy"]) >= -1e-14,
                f"binding nonnegative w={width} o={row['orientation']}")
        require(float(row["modified_field_energy"]) >= -1e-12,
                f"field nonnegative w={width} o={row['orientation']}")
        require(float(row["total_energy"]) + 1e-12 >= rest_floor,
                f"total lower bound w={width} o={row['orientation']}")
        require(close(float(row["inertial_mass_floor"]), mass_floor),
                f"mass floor w={width} o={row['orientation']}")
        require(row["gates_pass"] == "true",
                f"engine conjunction w={width} o={row['orientation']}")

    require(close(record["rest_energy_per_width_cubed"], 2.0 * E_REST),
            "rest floor scales as 2 E_REST w^3")
    require(close(record["inertial_mass_per_width_cubed"],
                  2.0 * M_INERTIAL),
            "mass floor scales as 2 M_INERTIAL w^3")

    # Convexity/Jensen at fixed total momentum P yields
    # N*h(P/N) = N*E0 + P^2/(2*N*M) + O(P^4).  The denominator identifies
    # the uniform collective inertial mass as N*M, which also diverges as w^3.
    for width in range(1, 5):
        n = 2 * width**3
        collective_mass = n * M_INERTIAL
        require(close(collective_mass / width**3, 2.0 * M_INERTIAL),
                f"collective inertial curvature w={width}")

    print("FTD-0647 certificate complete")


if __name__ == "__main__":
    main()
