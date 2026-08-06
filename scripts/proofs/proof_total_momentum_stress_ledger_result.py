#!/usr/bin/env python3
"""Independent certificate for the FTD-0769 total-momentum stress-ledger RUN.

`proof_total_momentum_stress_ledger.py` (this directory) verifies the frozen
*protocol* -- fixture closures and the `protocol_sha256` lock record -- before
any engine run and reads no engine result artifact. This script is the
companion that certifies the *executed* `--run` campaign after the fact, in
the pattern of `proof_long_transport_dynamic_response.py` for FTD-0768: it
reconstructs every registered scalar from the artifact's own raw fields and
independently re-derives the outcome, rather than trusting the engine's
self-reported `outcome` string.

`engine/results/` is gitignored (local-only campaign output per
`REF_PREREGISTER_MANIFEST.md`), so a fresh clone will not have the artifact;
`--self-test` exercises the classifier against synthetic fixtures without it.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_ARTIFACT = (
    ROOT / "engine" / "results" / "ftd_0769"
    / "ftd_0769_total_momentum_stress_ledger_v1.json"
)
FTD_0768_ARTIFACT = (
    ROOT / "engine" / "results" / "ftd_0768"
    / "ftd_0768_long_transport_dynamic_response_v1.json"
)

PROTOCOL_SHA256 = (
    "215B03A85A76B706E91099CA24E276FAC3B57DE3852353981456F79F411D8A13"
)
REVERSE_GATE = 1e-10


class Certificate:
    def __init__(self) -> None:
        self.checks = 0
        self.failures: list[str] = []

    def check(self, condition: bool, label: str) -> None:
        self.checks += 1
        if not condition:
            self.failures.append(label)


def close(value: float, target: float, tol: float = 1e-12) -> bool:
    return math.isfinite(value) and abs(value - target) <= tol


def classify(data: dict[str, Any]) -> tuple[str, bool]:
    """Re-derive the Sec 7 verdict-map outcome from raw fields only.

    Returns (outcome, execution_valid). Mirrors Sec 7 item 1-2: an
    infrastructure/pre-check miss takes priority over the G2 inherited-gate
    miss, which itself takes priority over every physics bucket (items 3-11),
    none of which this script attempts to reconstruct -- they are gated on
    G0-G7 all passing (Sec 9), which this run never reaches.
    """
    precheck_pass = bool(data.get("exactness_precheck", {}).get("pass"))
    firewall_pass = bool(data.get("firewall", {}).get("pass"))
    if not (precheck_pass and firewall_pass):
        return "MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED", False

    g2_fields_pass = (
        data.get("parent_valid") is True
        and data.get("aging_valid") is True
        and data.get("rest_initialized") is True
        and data.get("moving_initialized") is True
        and data.get("forward_valid") is True
        and data.get("boundary_clear") is True
        and data.get("reverse_discrete_exact") is True
        and data.get("reverse_steps") == 768
    )
    reverse_recovery = float(data.get("reverse_recovery", math.inf))
    reverse_within_gate = reverse_recovery <= REVERSE_GATE
    checkpoints = data.get("checkpoints", [])
    checkpoints_pass = (
        len(checkpoints) == 13
        and all(checkpoint.get("valid") is True for checkpoint in checkpoints)
    )

    g2_pass = g2_fields_pass and reverse_within_gate and checkpoints_pass
    if not g2_pass:
        return "MOMENTUM_LEDGER_BASELINE_INVALID", False

    # Sec 7 items 3-11 are physics buckets this script does not reconstruct;
    # if G2 passes, the artifact's own per-axis `verdicts` carry the result.
    return "PHYSICS_VERDICT_PENDING_MANUAL_REVIEW", True


def certify(path: Path, cross_check: Path | None) -> dict[str, Any]:
    certificate = Certificate()
    raw = path.read_bytes()
    data = json.loads(raw)

    certificate.check(data.get("ftd_id") == "FTD-0769", "identity")
    certificate.check(
        data.get("protocol_sha256") == PROTOCOL_SHA256, "protocol hash"
    )
    certificate.check(
        data.get("run_record_schema") == "ftd_0769_total_momentum_stress_ledger_v1",
        "run-record schema",
    )
    certificate.check(data.get("volume") == 321, "volume")
    certificate.check(data.get("formation_ticks") == 160, "formation ticks")
    certificate.check(data.get("preparation_age") == 128, "preparation age")
    certificate.check(data.get("discovery_ticks") == 768, "discovery ticks")
    certificate.check(data.get("checkpoint_stride") == 64, "checkpoint stride")
    certificate.check(close(float(data.get("boost", math.nan)), 0.03), "boost")
    certificate.check(data.get("direction") == [0, 0, 1], "direction (body ray)")

    tolerances = data.get("tolerances", {})
    certificate.check(
        close(float(tolerances.get("reverse", math.nan)), REVERSE_GATE),
        "reverse tolerance is the frozen 1e-10 (Sec 6.3 G2, not loosened)",
    )

    outcome, execution_valid = classify(data)
    certificate.check(
        outcome == data.get("outcome"),
        f"independently re-derived outcome ({outcome}) matches the artifact's "
        f"self-reported outcome ({data.get('outcome')})",
    )
    certificate.check(not execution_valid, "run is execution-invalid at G2")

    reverse_recovery = float(data.get("reverse_recovery", math.inf))
    certificate.check(
        reverse_recovery > REVERSE_GATE,
        "reverse_recovery exceeds the frozen 1e-10 gate "
        f"({reverse_recovery} > {REVERSE_GATE})",
    )
    certificate.check(
        data.get("reverse_discrete_exact") is True,
        "discrete state nonetheless recovers exactly (isolates the failure to "
        "continuous floating-point drift, not to non-reversible dynamics)",
    )
    certificate.check(data.get("reverse_valid") is False, "reverse_valid is false")

    verdicts = data.get("verdicts", [])
    certificate.check(len(verdicts) == 3, "one verdict entry per axis")
    for component in verdicts:
        certificate.check(
            component.get("verdict") == "MOMENTUM_LEDGER_BASELINE_INVALID",
            f"axis {component.get('component')} verdict is "
            "MOMENTUM_LEDGER_BASELINE_INVALID",
        )
        certificate.check(
            component.get("qualifying_checkpoints") == 0,
            f"axis {component.get('component')} reports zero qualifying "
            "checkpoints (Sec 7 items 3-11 never evaluated after G2 fails)",
        )
        certificate.check(
            component.get("L1_bucket") == "" and component.get("L2_bucket") == "",
            f"axis {component.get('component')} localization buckets are empty "
            "(no physics claim is licensed)",
        )

    checkpoints = data.get("checkpoints", [])
    certificate.check(len(checkpoints) == 13, "13 checkpoints recorded")
    certificate.check(
        all(checkpoint.get("valid") is True for checkpoint in checkpoints),
        "every forward checkpoint is internally valid",
    )

    cross_result: dict[str, Any] | None = None
    if cross_check is not None and cross_check.is_file():
        parent = json.loads(cross_check.read_bytes())
        shared_fields = (
            "moving_initial_hash",
            "moving_forward_final_hash",
            "moving_reversed_hash",
            "reverse_recovery",
            "reverse_discrete_exact",
            "reverse_valid",
            "reverse_steps",
            "reverse_maximum_common",
        )
        mismatches = [
            field
            for field in shared_fields
            if data.get(field) != parent.get(field)
        ]
        certificate.check(
            not mismatches,
            "every shared forward/reverse field is bit-identical to the "
            f"FTD-0768 parent artifact (mismatches: {mismatches or 'none'})",
        )
        cross_result = {
            "parent_artifact": str(cross_check),
            "fields_compared": len(shared_fields),
            "mismatches": mismatches,
        }

    return {
        "passed": not certificate.failures,
        "outcome": outcome,
        "reverse_recovery": reverse_recovery,
        "reverse_gate": REVERSE_GATE,
        "certificate_checks": certificate.checks,
        "certificate_failures": certificate.failures,
        "cross_check_vs_ftd_0768": cross_result,
    }


def self_test() -> dict[str, Any]:
    """Classifier self-test on synthetic fixtures; needs no artifact on disk."""
    valid_checkpoints = [{"tau": t, "valid": True} for t in range(0, 769, 64)]
    baseline_invalid_fixture = {
        "exactness_precheck": {"pass": True},
        "firewall": {"pass": True},
        "parent_valid": True,
        "aging_valid": True,
        "rest_initialized": True,
        "moving_initialized": True,
        "forward_valid": True,
        "boundary_clear": True,
        "reverse_discrete_exact": True,
        "reverse_steps": 768,
        "reverse_recovery": 3.8786822642578045e-09,
        "checkpoints": valid_checkpoints,
    }
    outcome, execution_valid = classify(baseline_invalid_fixture)
    check_a = outcome == "MOMENTUM_LEDGER_BASELINE_INVALID" and not execution_valid

    clean_fixture = dict(baseline_invalid_fixture)
    clean_fixture["reverse_recovery"] = 1e-13
    outcome_b, execution_valid_b = classify(clean_fixture)
    check_b = outcome_b == "PHYSICS_VERDICT_PENDING_MANUAL_REVIEW" and execution_valid_b

    infra_fixture = dict(baseline_invalid_fixture)
    infra_fixture["firewall"] = {"pass": False}
    outcome_c, execution_valid_c = classify(infra_fixture)
    check_c = (
        outcome_c == "MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED"
        and not execution_valid_c
    )

    passed = check_a and check_b and check_c
    return {
        "passed": passed,
        "baseline_invalid_reproduced": check_a,
        "clean_reverse_reaches_physics_gate": check_b,
        "firewall_failure_is_infrastructure_unresolved": check_c,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--artifact", type=Path, default=DEFAULT_ARTIFACT)
    parser.add_argument(
        "--cross-check", type=Path, default=FTD_0768_ARTIFACT,
        help="parent artifact to diff shared forward/reverse fields against",
    )
    parser.add_argument("--self-test", action="store_true")
    arguments = parser.parse_args()
    if arguments.self_test:
        result = self_test()
    else:
        result = certify(arguments.artifact, arguments.cross_check)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result.get("passed") else 1


if __name__ == "__main__":
    raise SystemExit(main())
