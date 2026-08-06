"""Independent record certificate for FTD-0609."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_SHARED_ANCHOR_CONSTITUENT_FIBRE_TRANSPORT_v1.md"
)
RESULT = ROOT / (
    "engine/results/ftd_0609/ftd_0609_shared_anchor_fibre_v1.json"
)
TICKS = ROOT / (
    "engine/results/ftd_0609/ftd_0609_shared_anchor_fibre_ticks_v1.csv"
)
EXPECTED_PROTOCOL = (
    "8CA3984F9E3FF2B8BE53BBBEA20028618EACFFC54C1B361994D10AD8B95D4D95"
)


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with TICKS.open(newline="", encoding="utf-8") as handle:
        ticks = list(csv.DictReader(handle))
    arms = record["motion_arms"]
    forward = [row for row in ticks if row["direction"] == "1"]
    observed_hash = protocol_hash()

    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL,
        "record_protocol": record["protocol_sha256"] == EXPECTED_PROTOCOL,
        "production_unchanged": record["production_changed"] is False,
        "option_default_is_strict": record["shared_anchor_option_default"]
        is False,
        "strict_baseline_reproduces": record["strict_regression_pass"]
        and record["strict_failure_ticks"] == [4, 2],
        "static_seed_reproduces": record["static_seed_pass"]
        and record["admissible_starts"] == 24
        and record["terminated_starts"] >= 18
        and record["clustered_starts"] >= 2,
        "both_histories_complete": record["motion_execution_complete"]
        and arms[0]["forward_ticks"] == 128
        and arms[0]["reverse_ticks"] == 128
        and arms[1]["forward_ticks"] == 64
        and arms[1]["reverse_ticks"] == 64
        and len(ticks) == 384,
        "common_action_gates_pass_every_tick": all(
            float(row["common_gate"]) <= 1e-12 for row in ticks
        ),
        "forward_energy_is_exact": len(forward) == 192
        and all(math.isfinite(float(row["energy_drift"])) for row in forward)
        and max(float(row["energy_drift"]) for row in forward) <= 1e-10,
        "two_record_fibre_is_exercised": all(
            arm["shared_anchor_states"] > 0
            and arm["maximum_anchor_multiplicity"] == 2
            for arm in arms
        ),
        "sharing_is_internal_not_neutralizer_overlap": all(
            arm["shared_within_trimer_states"] == arm["shared_anchor_states"]
            and arm["shared_cross_trimer_states"] == 0
            and arm["first_shared_pair"] == [3, 5]
            for arm in arms
        ),
        "constituents_remain_distinct": all(
            arm["minimum_constituent_distance"] >= 1e-3 for arm in arms
        ),
        "internal_trimer_geometry_remains_bound": all(
            arm["minimum_internal_distance"] >= 0.5
            and arm["maximum_internal_distance"] <= 2.0
            for arm in arms
        ),
        "state_only_inverse_passes": all(
            arm["reverse_recovery"] <= 1e-9 for arm in arms
        ),
        "integer_translation_covariance": record["integer_covariance_pass"]
        and record["integer_covariance_residual"] <= 1e-12,
        "fast_arm_is_constructive": arms[1]["valid"]
        and arms[1]["site_hops"] >= 6
        and arms[1]["longitudinal_displacement"] >= 1.5
        and arms[1]["maximum_separation_change"] <= 0.25,
        "slow_arm_fails_only_registered_transport_geometry":
            not arms[0]["valid"]
            and arms[0]["longitudinal_displacement"] < 1.5
            and arms[0]["maximum_separation_change"] > 0.25
            and arms[0]["worst_common_gate"] <= 1e-12
            and arms[0]["maximum_energy_drift"] <= 1e-10
            and arms[0]["reverse_recovery"] <= 1e-9,
        "locked_closed_negative_verdict": record["verdict"]
        == "SHARED_ANCHOR_FIBRE_TRANSPORT_CLOSED_NEGATIVE",
    }
    passed = all(checks.values())
    report = {
        "ftd_id": "FTD-0609",
        "certificate_pass": passed,
        "protocol_sha256": observed_hash,
        "checks": checks,
        "arm_summaries": [
            {
                "velocity": arm["velocity"],
                "site_hops": arm["site_hops"],
                "shared_anchor_states": arm["shared_anchor_states"],
                "longitudinal_displacement": arm["longitudinal_displacement"],
                "separation_change": arm["maximum_separation_change"],
                "energy_drift": arm["maximum_energy_drift"],
                "reverse_recovery": arm["reverse_recovery"],
                "valid": arm["valid"],
            }
            for arm in arms
        ],
        "licensed_constructive_statement": (
            "a two-record internal site-chart fibre removes the FTD-0608 "
            "solver obstruction while preserving common action and inversion; "
            "the v=1/32 transport arm passes"
        ),
        "licensed_negative_statement": (
            "the conjunction of both velocities fails because the v=1/64 "
            "neutral-pair arm loses co-transport and pair separation"
        ),
        "next_discriminator": (
            "separate single-core mobility from finite-volume neutralizer "
            "interaction before adding any new binding primitive"
        ),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
