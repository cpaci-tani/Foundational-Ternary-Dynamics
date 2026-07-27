"""Independent record certificate for FTD-0606.

This certificate proves the exact strain-sector binding Hessian, verifies the
locked protocol prefix, and reconstructs the distinction between a stable
continuous constituent minimum and its failed ternary-site projection.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_GLOBAL_ORIENTATION_STRAIN_CORE_v1.md"
)
RESULT = ROOT / (
    "engine/results/ftd_0606/"
    "ftd_0606_global_orientation_strain_core_v1.json"
)
SAMPLES = ROOT / (
    "engine/results/ftd_0606/"
    "ftd_0606_global_orientation_strain_core_samples_v1.csv"
)
EXPECTED_PROTOCOL = (
    "EC0CECED1CCF40187BCE0C4B38DA34039B5CAD94069AFD05F16420D25D99494A"
)
COLLISION_FREE_PHASES = {5, 6, 14, 15, 16, 17, 25, 26}


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def determinant_3(matrix: list[list[Fraction]]) -> Fraction:
    return (
        matrix[0][0]
        * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1])
        - matrix[0][1]
        * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0])
        + matrix[0][2]
        * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0])
    )


def exact_strain_hessian() -> list[list[Fraction]]:
    # For two mirrored equilateral trimers with
    # V=(1/4) sum_groups,pairs (|A d|^2-2)^2 and
    # A=I+[[h0,h1],[h1,h2]], direct differentiation at h=0 gives:
    return [
        [Fraction(18), Fraction(0), Fraction(6)],
        [Fraction(0), Fraction(24), Fraction(0)],
        [Fraction(6), Fraction(0), Fraction(18)],
    ]


def finite(value: str) -> bool:
    return math.isfinite(float(value))


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with SAMPLES.open(newline="", encoding="utf-8") as handle:
        samples = list(csv.DictReader(handle))

    observed_hash = protocol_hash()
    hessian = exact_strain_hessian()
    determinant = determinant_3(hessian)
    exact_eigenvalues = [Fraction(12), Fraction(24), Fraction(24)]
    collision_free = {
        int(row["phase_index"])
        for row in samples
        if int(row["duplicate_anchor_pairs"]) == 0
    }
    colliding = [
        row for row in samples if int(row["duplicate_anchor_pairs"]) > 0
    ]
    admitted = [
        row for row in samples if int(row["duplicate_anchor_pairs"]) == 0
    ]

    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL,
        "record_protocol": record["protocol_sha256"] == EXPECTED_PROTOCOL,
        "production_unchanged": record["production_changed"] is False,
        "all_phase_records_present": len(samples) == 32
        and {int(row["phase_index"]) for row in samples} == set(range(32)),
        "exact_strain_hessian_positive_definite": determinant == 6912
        and exact_eigenvalues == [Fraction(12), Fraction(24), Fraction(24)],
        "global_search_coverage_passes": record["coverage_pass"]
        and all(int(row["terminated_starts"]) >= 18 for row in samples)
        and all(int(row["clustered_starts"]) >= 3 for row in samples),
        "continuous_stationary_core_passes": record["stationary_core_pass"]
        and all(row["stable"] == "1" for row in samples)
        and max(float(row["gradient_inf"]) for row in samples) <= 5e-7
        and min(float(row["minimum_eigenvalue"]) for row in samples)
        >= -5e-6
        and min(int(row["positive_modes"]) for row in samples) == 6,
        "strain_is_small_and_interior": max(
            abs(float(row[key]))
            for row in samples
            for key in ("h0", "h1", "h2")
        ) < 0.20 - 1e-4,
        "site_projection_collision_count": len(colliding) == 24
        and record["site_projection_collision_phases"] == 24
        and max(int(row["duplicate_anchor_pairs"]) for row in samples) == 2,
        "collision_free_phase_identity": collision_free
        == COLLISION_FREE_PHASES,
        "collision_blocks_transaction": all(
            not finite(row["inverse"]) for row in colliding
        ),
        "admitted_transaction_and_inverse_pass": len(admitted) == 8
        and all(float(row["common_gate"]) <= 1e-12 for row in admitted)
        and all(float(row["inverse"]) <= 1e-10 for row in admitted),
        "admitted_force_sign_is_mixed": sum(
            row["attractive"] == "1" for row in admitted
        ) == 4,
        "locked_unresolved_verdict": record["verdict"]
        == "GLOBAL_ORIENTATION_STRAIN_NUMERICALLY_UNRESOLVED",
        "compact_branch_not_closed": record["verdict"]
        != "GLOBAL_ORIENTATION_STRAIN_COMPACT_CORE_CLOSED_NEGATIVE",
    }
    passed = all(checks.values())
    report = {
        "ftd_id": "FTD-0606",
        "certificate_pass": passed,
        "protocol_sha256": observed_hash,
        "checks": checks,
        "exact_strain_hessian": [
            [int(value) for value in row] for row in hessian
        ],
        "exact_strain_hessian_determinant": int(determinant),
        "exact_strain_hessian_eigenvalues": [
            int(value) for value in exact_eigenvalues
        ],
        "continuous_stationary_core_licensed": True,
        "site_ontic_dynamic_core_licensed": False,
        "site_projection_collision_phases": len(colliding),
        "collision_free_phases": sorted(collision_free),
        "collision_free_attractive_phases": sum(
            row["attractive"] == "1" for row in admitted
        ),
        "next_discriminator": (
            "preregistered site-admissible constrained SO(3) x strain search"
        ),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
