"""Independent record certificate for FTD-0608.

The certificate verifies the locked launch state, all completed common-action
ticks, and the diagnostic that both registered velocities first lose solver
admissibility when the free predictor creates one duplicate site anchor.
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
    "PREREG_QUALIFIED_INTERIOR_COMPACT_MATTER_TRANSPORT_v1.md"
)
RESULT = ROOT / (
    "engine/results/ftd_0608/"
    "ftd_0608_qualified_interior_transport_v1.json"
)
TICKS = ROOT / (
    "engine/results/ftd_0608/"
    "ftd_0608_qualified_interior_transport_ticks_v1.csv"
)
EXPECTED_PROTOCOL = (
    "B64BB90EF082EC8E47BE83BA1F9951D7B30C3C5904AE8E4C639B33543020C5E0"
)
PRIOR_ENERGY = 0.0031781023845096961


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with TICKS.open(newline="", encoding="utf-8") as handle:
        ticks = list(csv.DictReader(handle))
    valid_ticks = [row for row in ticks if math.isfinite(float(row["energy_drift"]))]
    failures = record["failure_diagnostics"]
    arms = record["motion_arms"]
    observed_hash = protocol_hash()

    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL,
        "record_protocol": record["protocol_sha256"] == EXPECTED_PROTOCOL,
        "production_unchanged": record["production_changed"] is False,
        "phase_fixed_to_prior_lowest_qualified_core":
            record["selected_phase_index"] == 15
            and record["selected_phase"] == 15 / 32,
        "static_search_reproduces": record["static_coverage_pass"]
        and record["static_seed_pass"]
        and record["admissible_starts"] == 24
        and record["terminated_starts"] >= 18
        and record["clustered_starts"] >= 2,
        "static_fingerprint_is_exact": record["energy"] == PRIOR_ENERGY
        and record["energy_fingerprint_residual"] == 0,
        "static_differential_and_field_gates":
            record["chart_margin"] >= 5e-3
            and record["gradient_inf"] <= 5e-7
            and record["minimum_eigenvalue"] > 1e-6
            and record["positive_modes"] == 6
            and record["field_gate"] <= 1e-11,
        "integer_translation_covariance": record["integer_covariance_pass"]
        and record["integer_covariance_residual"] <= 1e-12,
        "completed_ticks_are_common_action_clean": len(valid_ticks) == 6
        and all(float(row["common_gate"]) <= 1e-12 for row in valid_ticks)
        and all(float(row["energy_drift"]) <= 1e-10 for row in valid_ticks)
        and all(int(row["duplicate_anchors"]) == 0 for row in valid_ticks)
        and all(float(row["minimum_distance"]) >= 0.5 for row in valid_ticks)
        and all(float(row["maximum_distance"]) <= 2.0 for row in valid_ticks),
        "registered_failure_ticks": arms[0]["forward_ticks"] == 4
        and arms[1]["forward_ticks"] == 2
        and arms[0]["reverse_ticks"] == 0
        and arms[1]["reverse_ticks"] == 0
        and arms[0]["site_hops"] == 0
        and arms[1]["site_hops"] == 0,
        "failure_occurs_at_common_nominal_boundary_time":
            failures[0]["tick"] * arms[0]["velocity"] == 1 / 16
            and failures[1]["tick"] * arms[1]["velocity"] == 1 / 16,
        "failure_is_initial_candidate_inadmissibility": all(
            item["observed"]
            and item["solve_attempted"]
            and not item["solve_converged"]
            and item["iterations"] == 0
            and item["residual"] is None
            for item in failures
        ),
        "free_predictor_creates_one_anchor_alias": all(
            item["input_duplicate_anchors"] == 0
            and item["free_predictor_duplicate_anchors"] == 1
            and not item["returned_site_projection_valid"]
            for item in failures
        ),
        "locked_unresolved_verdict": record["verdict"]
        == "QUALIFIED_INTERIOR_COMPACT_MATTER_NUMERICALLY_UNRESOLVED",
        "no_negative_dynamics_claim": record["verdict"]
        != "QUALIFIED_INTERIOR_COMPACT_TRANSPORT_CLOSED_NEGATIVE",
    }
    passed = all(checks.values())
    report = {
        "ftd_id": "FTD-0608",
        "certificate_pass": passed,
        "protocol_sha256": observed_hash,
        "checks": checks,
        "clean_forward_ticks": [arms[0]["forward_ticks"], arms[1]["forward_ticks"]],
        "failure_ticks": [failures[0]["tick"], failures[1]["tick"]],
        "failure_nominal_displacements": [
            failures[0]["tick"] * arms[0]["velocity"],
            failures[1]["tick"] * arms[1]["velocity"],
        ],
        "free_predictor_duplicate_anchor_pairs": [
            failures[0]["free_predictor_duplicate_anchors"],
            failures[1]["free_predictor_duplicate_anchors"],
        ],
        "licensed_statement": (
            "the fixed interior core evolves cleanly until the first predicted "
            "anchor alias; the strict site chart then prevents solver startup"
        ),
        "unlicensed_statement": (
            "compact dynamics fails physically; the registered solver never "
            "evaluates an allowed shared-anchor transition"
        ),
        "next_discriminator": (
            "preregister a shared-anchor chart/fibre extension while keeping "
            "the same current, field, action, and inverse gates"
        ),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
