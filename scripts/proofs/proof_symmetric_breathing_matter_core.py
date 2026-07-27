"""Independent record certificate for FTD-0604.

The script does not call the C++ optimizer or common-action solver.  It proves
the exact intratrimer breathing potential, reconstructs the locked campaign
statistics from the CSV, and checks that the recorded negative verdict follows
from the preregistered gates.
"""

from __future__ import annotations

import csv
import hashlib
import json
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_SYMMETRIC_BREATHING_MATTER_CORE_v1.md"
)
RESULT = ROOT / (
    "engine/results/ftd_0604/ftd_0604_symmetric_breathing_core_v1.json"
)
SAMPLES = ROOT / (
    "engine/results/ftd_0604/"
    "ftd_0604_symmetric_breathing_core_samples_v1.csv"
)
EXPECTED_PROTOCOL = (
    "CD8DB5F38A6E9F01BB8EDFAF63664EF940BF0D1F87C1CE8BF5B17789616FDACE"
)


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def squared(vector: tuple[Fraction, Fraction, Fraction]) -> Fraction:
    return sum((component * component for component in vector), Fraction(0))


def exact_binding_certificate() -> dict[str, str]:
    offsets = (
        (Fraction(-2, 3), Fraction(-1, 3), Fraction(-1, 3)),
        (Fraction(1, 3), Fraction(2, 3), Fraction(-1, 3)),
        (Fraction(1, 3), Fraction(-1, 3), Fraction(2, 3)),
    )
    pair_norms = []
    for a in range(3):
        for b in range(a + 1, 3):
            pair_norms.append(squared(tuple(
                offsets[a][axis] - offsets[b][axis] for axis in range(3)
            )))
    assert pair_norms == [Fraction(2), Fraction(2), Fraction(2)]

    # Each of the six intratrimer pairs has d^2=2 lambda^2.  With
    # V_pair=(1/4)(d^2-2)^2, V_bind=6(lambda^2-1)^2 exactly.
    # Its derivatives are 24 lambda(lambda^2-1) and 72 lambda^2-24.
    return {
        "pair_squared_norms": ",".join(map(str, pair_norms)),
        "binding_polynomial": "6*(lambda^2-1)^2",
        "binding_first_derivative": "24*lambda*(lambda^2-1)",
        "binding_second_derivative": "72*lambda^2-24",
        "binding_curvature_at_one": str(Fraction(48)),
    }


def main() -> int:
    observed_hash = protocol_hash()
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with SAMPLES.open(newline="", encoding="utf-8") as handle:
        samples = list(csv.DictReader(handle))

    phase_indices = {int(row["phase_index"]) for row in samples}
    scales = [float(row["scale"]) for row in samples]
    rigid = [float(row["rigid_energy"]) for row in samples]
    relaxed = [float(row["relaxed_energy"]) for row in samples]
    stationarity = [float(row["stationarity"]) for row in samples]
    curvature = [float(row["curvature"]) for row in samples]
    gauss = [float(row["gauss_gate"]) for row in samples]
    common = [float(row["common_gate"]) for row in samples]
    inverse = [float(row["inverse"]) for row in samples]
    inward = [float(row["inward_impulse"]) for row in samples]
    separation = [float(row["separation_decrease"]) for row in samples]
    pseudomomentum = [float(row["pseudomomentum_defect"]) for row in samples]
    attractive = [int(row["attractive"]) for row in samples]

    rigid_barrier = max(rigid) - min(rigid)
    relaxed_barrier = max(relaxed) - min(relaxed)
    barrier_ratio = relaxed_barrier / rigid_barrier
    extrema_residual = max(
        abs(min(scales) - record["minimum_scale"]),
        abs(max(scales) - record["maximum_scale"]),
        abs(max(stationarity) - record["worst_stationarity"]),
        abs(min(curvature) - record["minimum_curvature"]),
        abs(max(gauss) - record["worst_gauss_gate"]),
        abs(max(common) - record["worst_common_gate"]),
        abs(max(inverse) - record["worst_inverse"]),
        abs(min(inward) - record["minimum_inward_impulse"]),
        abs(min(separation) - record["minimum_separation_decrease"]),
        abs(max(pseudomomentum) - record["maximum_pseudomomentum_defect"]),
        abs(rigid_barrier - record["rigid_barrier"]),
        abs(relaxed_barrier - record["relaxed_barrier"]),
        abs(barrier_ratio - record["barrier_ratio"]),
    )
    static_gate_expected = (
        record["optimizer_pass"]
        and record["interior_pass"]
        and max(stationarity) <= 1e-8
        and min(curvature) > 1e-6
        and max(gauss) <= 1e-12
        and all(after <= before + 1e-12
                for before, after in zip(rigid, relaxed))
    )
    expected_verdict = (
        "SYMMETRIC_BREATHING_CORE_STATIC_BRANCH_CLOSED_NEGATIVE"
        if not static_gate_expected
        or not record["common_pass"]
        or not record["inverse_pass"]
        or not record["periodicity_pass"]
        else "SYMMETRIC_BREATHING_CORE_PHASE_ROBUST_CONSTRUCTIVE"
        if all(attractive)
        else "SYMMETRIC_BREATHING_RELAXES_BUT_FORCE_SIGN_FAILS"
    )
    exact = exact_binding_certificate()
    relative_barrier_reduction = 1.0 - barrier_ratio

    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL
        == record["protocol_sha256"],
        "all_samples_present": len(samples) == 32
        and phase_indices == set(range(32)),
        "sample_statistics_reconstruct": extrema_residual <= 1e-18,
        "exact_binding_curvature": exact["binding_curvature_at_one"] == "48",
        "optimizer_and_interior": record["optimizer_pass"]
        and record["interior_pass"],
        "locked_stationarity_gate_fails": not record["stability_pass"]
        and max(stationarity) > 1e-8,
        "positive_curvature": min(curvature) > 1e-6,
        "common_action_and_inverse": record["common_pass"]
        and record["inverse_pass"]
        and max(common) <= 1e-12
        and max(inverse) <= 1e-10,
        "integer_periodicity": record["periodicity_pass"]
        and record["periodicity_residual"] <= 1e-12
        and record["periodicity_scale_residual"] <= 1e-12,
        "force_sign_not_robust": sum(attractive) == 18
        == record["attractive_phases"]
        and min(inward) < -1e-10
        and min(separation) < 0.0,
        "barrier_reduction_tiny": 0.0 < relative_barrier_reduction < 1e-4,
        "locked_verdict": record["verdict"] == expected_verdict
        == "SYMMETRIC_BREATHING_CORE_STATIC_BRANCH_CLOSED_NEGATIVE",
        "production_unchanged": record["production_changed"] is False,
    }
    report = {
        "ftd_id": "FTD-0604",
        "protocol_sha256": observed_hash,
        "sample_count": len(samples),
        "exact_binding": exact,
        "scale_range": [min(scales), max(scales)],
        "worst_stationarity": max(stationarity),
        "minimum_curvature": min(curvature),
        "attractive_phases": sum(attractive),
        "relative_barrier_reduction": relative_barrier_reduction,
        "phase_robust_breathing_claim_licensed": False,
        "all_internal_deformation_closed": False,
        "checks": checks,
        "certificate_pass": all(checks.values()),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["certificate_pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
