"""Independent record certificate for FTD-0607.

This certificate verifies the locked protocol and reconstructs the exact
scope of the result: the registered constrained search found five qualified
site-interior static cores, but phase zero and the campaign-wide coverage gate
failed, so no autonomous-motion arm was licensed or executed.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_SITE_ADMISSIBLE_COMPACT_MATTER_MOTION_v1.md"
)
RESULT = ROOT / (
    "engine/results/ftd_0607/"
    "ftd_0607_site_admissible_motion_v1.json"
)
STATIC = ROOT / (
    "engine/results/ftd_0607/"
    "ftd_0607_site_admissible_static_samples_v1.csv"
)
MOTION = ROOT / (
    "engine/results/ftd_0607/ftd_0607_motion_ticks_v1.csv"
)
EXPECTED_PROTOCOL = (
    "CA37FB9700A2416FE293B26A903A9DCA5233091C215E0AEB83D92BA802D871E9"
)
QUALIFIED_PHASES = {14, 15, 16, 17, 26}


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with STATIC.open(newline="", encoding="utf-8") as handle:
        samples = list(csv.DictReader(handle))
    with MOTION.open(newline="", encoding="utf-8") as handle:
        motion = list(csv.DictReader(handle))

    observed_hash = protocol_hash()
    qualified = {
        int(row["phase_index"])
        for row in samples
        if row["qualified"] == "1"
    }
    covered = {
        int(row["phase_index"])
        for row in samples
        if row["coverage"] == "1"
    }
    phase_zero = next(row for row in samples if row["phase_index"] == "0")
    qualified_rows = [
        row for row in samples if int(row["phase_index"]) in qualified
    ]
    uncovered_rows = [row for row in samples if row["coverage"] == "0"]

    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL,
        "record_protocol": record["protocol_sha256"] == EXPECTED_PROTOCOL,
        "production_unchanged": record["production_changed"] is False,
        "all_phase_records_present": len(samples) == 32
        and {int(row["phase_index"]) for row in samples} == set(range(32)),
        "all_registered_starts_admissible": all(
            int(row["admissible_starts"]) == 24 for row in samples
        ),
        "termination_gate_is_not_the_blocker": all(
            int(row["terminated_starts"])
            >= math.ceil(0.75 * int(row["admissible_starts"]))
            for row in samples
        ),
        "repeatability_coverage_fails": bool(uncovered_rows)
        and all(int(row["clustered_starts"]) < 2 for row in uncovered_rows)
        and record["static_coverage_pass"] is False,
        "qualified_phase_identity": qualified == QUALIFIED_PHASES
        and record["qualified_phases"] == len(QUALIFIED_PHASES),
        "qualified_static_gates_pass": all(
            float(row["chart_margin"]) >= 5e-3
            and float(row["gradient_inf"]) <= 5e-7
            and float(row["minimum_eigenvalue"]) > 1e-6
            and int(row["positive_modes"]) == 6
            and float(row["field_gate"]) <= 1e-11
            for row in qualified_rows
        ),
        "phase_zero_is_boundary_limited": phase_zero["qualified"] == "0"
        and float(phase_zero["chart_margin"]) < 5e-3
        and record["phase_zero_selected"] is False,
        "motion_was_correctly_withheld": motion == []
        and all(arm["ticks_requested"] == 0 for arm in record["motion_arms"])
        and all(arm["valid"] is False for arm in record["motion_arms"]),
        "covariance_was_not_claimed": record["integer_covariance_pass"]
        is False
        and record["integer_covariance_residual"] is None,
        "locked_unresolved_verdict": record["verdict"]
        == "SITE_ADMISSIBLE_COMPACT_MATTER_NUMERICALLY_UNRESOLVED",
        "no_negative_dynamics_verdict": record["verdict"]
        != "SITE_ADMISSIBLE_STATIC_CORE_DYNAMICS_CLOSED_NEGATIVE",
    }
    passed = all(checks.values())
    report = {
        "ftd_id": "FTD-0607",
        "certificate_pass": passed,
        "protocol_sha256": observed_hash,
        "checks": checks,
        "covered_phases": sorted(covered),
        "qualified_phases": sorted(qualified),
        "minimum_qualified_chart_margin": min(
            float(row["chart_margin"]) for row in qualified_rows
        ),
        "minimum_qualified_tangent_eigenvalue": min(
            float(row["minimum_eigenvalue"]) for row in qualified_rows
        ),
        "maximum_qualified_gradient": max(
            float(row["gradient_inf"]) for row in qualified_rows
        ),
        "maximum_qualified_field_gate": max(
            float(row["field_gate"]) for row in qualified_rows
        ),
        "licensed_static_statement": (
            "five registered phase slices contain repeatable, stable, "
            "field-clean site-interior compact cores"
        ),
        "licensed_dynamic_statement": "none; motion arms were not run",
        "next_discriminator": (
            "new preregistration selecting one qualified interior phase "
            "before executing the unchanged autonomous-motion arms"
        ),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
