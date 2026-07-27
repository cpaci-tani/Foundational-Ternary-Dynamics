"""Independent record certificate for FTD-0605.

The script does not run the C++ optimizer or field solver. It proves the exact
rank-three binding Hessian over the registered six-coordinate shape chart,
reconstructs the locked campaign disposition from CSV/JSON, and enforces the
scope boundary between a failed compact local basin and extended matter.
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
    "PREREG_FULL_MIRRORED_INTERNAL_SHAPE_CORE_v1.md"
)
RESULT = ROOT / (
    "engine/results/ftd_0605/ftd_0605_full_mirrored_shape_core_v1.json"
)
SAMPLES = ROOT / (
    "engine/results/ftd_0605/"
    "ftd_0605_full_mirrored_shape_core_samples_v1.csv"
)
EXPECTED_PROTOCOL = (
    "388926B3947F0C0A378FC3B52BD99E3C94D8F9BBB0A4D325E26CE1252B79C70F"
)
NPARAM = 6


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def matrix_rank(matrix: list[list[Fraction]]) -> int:
    work = [row[:] for row in matrix]
    rows = len(work)
    columns = len(work[0])
    rank = 0
    for column in range(columns):
        pivot = next(
            (row for row in range(rank, rows) if work[row][column]), None
        )
        if pivot is None:
            continue
        work[rank], work[pivot] = work[pivot], work[rank]
        scale = work[rank][column]
        work[rank] = [value / scale for value in work[rank]]
        for row in range(rows):
            if row == rank or not work[row][column]:
                continue
            factor = work[row][column]
            work[row] = [
                value - factor * pivot_value
                for value, pivot_value in zip(work[row], work[rank])
            ]
        rank += 1
    return rank


def exact_binding_hessian() -> tuple[list[list[Fraction]], int]:
    reference = (
        (Fraction(-2, 3), Fraction(-1, 3), Fraction(-1, 3)),
        (Fraction(1, 3), Fraction(2, 3), Fraction(-1, 3)),
        (Fraction(1, 3), Fraction(-1, 3), Fraction(2, 3)),
    )
    # Linear maps r_a = r_a^0 + M_a q, with r_2=-r_0-r_1.
    maps = []
    for particle in range(3):
        matrix = [[Fraction(0) for _ in range(NPARAM)] for _ in range(3)]
        for axis in range(3):
            if particle == 0:
                matrix[axis][axis] = Fraction(1)
            elif particle == 1:
                matrix[axis][axis + 3] = Fraction(1)
            else:
                matrix[axis][axis] = Fraction(-1)
                matrix[axis][axis + 3] = Fraction(-1)
        maps.append(matrix)

    gradients: list[list[Fraction]] = []
    for a, b in ((0, 1), (0, 2), (1, 2)):
        edge = [reference[a][axis] - reference[b][axis] for axis in range(3)]
        assert sum(value * value for value in edge) == 2
        gradient = []
        for coordinate in range(NPARAM):
            derivative = sum(
                edge[axis]
                * (maps[a][axis][coordinate] - maps[b][axis][coordinate])
                for axis in range(3)
            )
            gradient.append(2 * derivative)
        gradients.append(gradient)

    # The two mirrored trimers give V=(1/2) sum_e u_e^2. At u_e=0,
    # H=sum_e grad(u_e) outer grad(u_e): exact PSD with rank at most three.
    hessian = [[Fraction(0) for _ in range(NPARAM)] for _ in range(NPARAM)]
    for gradient in gradients:
        for row in range(NPARAM):
            for column in range(NPARAM):
                hessian[row][column] += gradient[row] * gradient[column]
    return hessian, matrix_rank(hessian)


def finite(value: str) -> bool:
    return math.isfinite(float(value))


def main() -> int:
    observed_hash = protocol_hash()
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with SAMPLES.open(newline="", encoding="utf-8") as handle:
        samples = list(csv.DictReader(handle))

    hessian, rank = exact_binding_hessian()
    expected_hessian = [
        [20, 4, 16, 4, -4, 8],
        [4, 8, -4, -4, 4, -8],
        [16, -4, 20, 8, -8, 16],
        [4, -4, 8, 8, 4, 4],
        [-4, 4, -8, 4, 20, -16],
        [8, -8, 16, 4, -16, 20],
    ]
    hessian_matches = hessian == [
        [Fraction(value) for value in row] for row in expected_hessian
    ]

    phase_indices = {int(row["phase_index"]) for row in samples}
    completed = [row for row in samples if finite(row["gradient_inf"])]
    exhausted = [row for row in samples if int(row["evaluations"]) == 900]
    boundary = [
        row for row in completed
        if max(abs(float(row[f"u{i}"])) for i in range(NPARAM))
        >= 0.20 - 1e-9
    ]
    completed_gradients = [float(row["gradient_inf"]) for row in completed]
    completed_field = [float(row["field_gate"]) for row in completed]
    completed_common = [float(row["common_gate"]) for row in completed]
    completed_inverse = [float(row["inverse"]) for row in completed]
    completed_inward = [float(row["inward_impulse"]) for row in completed]
    completed_separation = [
        float(row["separation_decrease"]) for row in completed
    ]

    expected_verdict = (
        "FULL_MIRRORED_SHAPE_STATIC_BRANCH_CLOSED_NEGATIVE"
        if not record["optimizer_pass"]
        or not record["interior_pass"]
        or not record["stability_pass"]
        or not record["field_pass"]
        or not record["common_pass"]
        or not record["inverse_pass"]
        or not record["periodicity_pass"]
        else "FULL_MIRRORED_SHAPE_PHASE_ROBUST_CONSTRUCTIVE"
        if record["attraction_robust"]
        else "FULL_MIRRORED_SHAPE_RELAXES_BUT_FORCE_SIGN_FAILS"
    )

    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL
        == record["protocol_sha256"],
        "all_phase_records_present": len(samples) == 32
        and phase_indices == set(range(32)),
        "exact_binding_hessian": hessian_matches,
        "exact_binding_rank_three": rank == 3,
        "exact_binding_three_soft_modes": NPARAM - rank == 3,
        "green_kernel_pass": record["green_pass"]
        and record["green_residual"] <= 1e-15,
        "optimizer_coverage_fails": not record["optimizer_pass"]
        and len(exhausted) == 29
        and len(completed) == 3,
        "completed_minima_hit_boundary": not record["interior_pass"]
        and len(boundary) == len(completed) == 3,
        "completed_gradients_not_stationary": min(completed_gradients) > 5e-7,
        "direct_field_crosscheck": max(completed_field) <= 1e-11
        and record["worst_direct_energy_residual"] <= 1e-11,
        "executed_common_action": max(completed_common) <= 1e-12,
        "executed_state_inverse": max(completed_inverse) <= 1e-10,
        "executed_boundary_arms_nonattractive": max(completed_inward) < 0.0
        and max(completed_separation) < 0.0,
        "incomplete_barrier_not_reported": record["reference_barrier"] is None
        and record["relaxed_barrier"] is None
        and record["barrier_ratio"] is None,
        "locked_verdict": record["verdict"] == expected_verdict
        == "FULL_MIRRORED_SHAPE_STATIC_BRANCH_CLOSED_NEGATIVE",
        "production_unchanged": record["production_changed"] is False,
    }
    report = {
        "ftd_id": "FTD-0605",
        "protocol_sha256": observed_hash,
        "sample_count": len(samples),
        "exact_binding_hessian": expected_hessian,
        "exact_binding_rank": rank,
        "exact_soft_modes": NPARAM - rank,
        "optimizer_exhausted_phases": len(exhausted),
        "returned_boundary_phases": [
            int(row["phase_index"]) for row in boundary
        ],
        "minimum_returned_gradient": min(completed_gradients),
        "compact_local_shape_core_licensed": False,
        "extended_matter_closed": False,
        "checks": checks,
        "certificate_pass": all(checks.values()),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["certificate_pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
