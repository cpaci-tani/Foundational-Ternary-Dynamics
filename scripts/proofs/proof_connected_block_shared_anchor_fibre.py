"""Independent record certificate for FTD-0626.

This script reads only the versioned JSON/CSV run records.  It does not call
the C++ implementation or recompute its trajectories.
"""

from __future__ import annotations

import csv
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0626"
PROTOCOL = "67806EA9B3D8ED02B2BF04A839B21E1053FDE1199DE46FB2E064D6E061544C52"


checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


with (RESULTS / "ftd_0626_connected_block_shared_anchor_fibre_v1.json").open(
    encoding="utf-8"
) as handle:
    summary = json.load(handle)

with (RESULTS / "ftd_0626_connected_block_shared_anchor_fibre_arms_v1.csv").open(
    newline="", encoding="utf-8"
) as handle:
    arms = list(csv.DictReader(handle))

with (RESULTS / "ftd_0626_connected_block_shared_anchor_fibre_ticks_v1.csv").open(
    newline="", encoding="utf-8"
) as handle:
    ticks = list(csv.DictReader(handle))

with (
    RESULTS / "ftd_0626_connected_block_shared_anchor_fibre_regressions_v1.csv"
).open(newline="", encoding="utf-8") as handle:
    regressions = list(csv.DictReader(handle))


by_label = {row["label"]: row for row in arms}
ticks_by_label: dict[str, list[dict[str, str]]] = {}
for row in ticks:
    ticks_by_label.setdefault(row["label"], []).append(row)

expected = {
    "exact_half_rest_x",
    "exact_half_rest_y",
    "near_half_zero",
    "circulation_positive_1B",
    "circulation_negative_1B",
    "circulation_positive_4B",
    "circulation_negative_4B",
    "cyclic_positive_1B",
    "cyclic_positive_4B",
}

check("record id", summary["ftd_id"] == "FTD-0626")
check("protocol lock", summary["protocol_sha256"] == PROTOCOL)
check("production unchanged", summary["production_changed"] is False)
check("option default false", summary["shared_anchor_option_default"] is False)
check("coverage", summary["coverage_pass"] == 1 and set(by_label) == expected)
check("execution", summary["execution_pass"] == 1)
check("strict regression", summary["default_false_regression_pass"] == 1)
check("fibre exercised", summary["fibre_exercised"] == 1)
check("registered verdict", summary["verdict"] == "CONNECTED_BLOCK_FIBRE_CLOSED_NEGATIVE")
check("fixed rest fails", summary["rest_pass"] == 0)
check("motion continuation passes", summary["motion_pass"] == 1)
check("symmetry", float(summary["symmetry_residual"]) <= 1e-8)
check("covariance", float(summary["covariance_residual"]) <= 1e-8)
check("common action", float(summary["worst_common_residual"]) <= 1e-10)
check("energy drift", float(summary["worst_energy_drift"]) <= 1e-9)
check("state recovery", float(summary["worst_recovery"]) <= 1e-8)

check("nine arms", len(arms) == 9)
check("sixteen ticks each", all(len(ticks_by_label[name]) == 16 for name in expected))
check("all forward", all(row["forward"] == "1" for row in arms))
check("all reverse", all(row["reverse"] == "1" for row in arms))
check("all exact", all(row["exact"] == "1" for row in arms))
check("all metadata", all(row["metadata"] == "1" for row in arms))
check("all fibre regular", all(row["fibre"] == "1" for row in arms))
check("multiplicity bound", max(int(row["max_multiplicity"]) for row in arms) == 2)

rest = [by_label["exact_half_rest_x"], by_label["exact_half_rest_y"]]
check("rest centres stationary", max(float(row["max_displacement"]) for row in rest) < 1e-12)
check("rest centre momentum", max(float(row["max_momentum"]) for row in rest) < 1e-12)
check("rest fibre every tick", all(int(row["shared_states"]) == 16 for row in rest))
check("rest pairs well separated", min(float(row["min_separation"]) for row in rest) > 0.99)
check("rest has internal motion", min(float(row["max_shape"]) for row in rest) > 1e-3)
check("rest has bond response", min(float(row["max_strain"]) for row in rest) > 3e-3)
check("rest stationarity gate fails", all(row["stationary"] == "0" for row in rest))
check("rest still reverses", all(row["reverse"] == "1" for row in rest))

near = by_label["near_half_zero"]
check("near-half no alias needed", near["shared_states"] == "0" and near["qualified"] == "1")

circulation = [row for row in arms if "circulation_" in row["label"] or "cyclic_" in row["label"]]
check("six circulation arms", len(circulation) == 6)
check("circulation all qualified", all(row["qualified"] == "1" for row in circulation))
check("circulation fibre exercised", all(int(row["shared_states"]) > 0 for row in circulation))
check("circulation remains separated", min(float(row["min_separation"]) for row in circulation) > 0.99)

check("two strict regressions", len(regressions) == 2)
check("strict expected ticks", {int(row["failure_tick"]) for row in regressions} == {1, 2})
check("strict roots converged", all(row["converged"] == "1" for row in regressions))
check("strict site-only rejection", all(row["site_rejected"] == "1" for row in regressions))
check("strict graphs intact", all(row["graph"] == "1" for row in regressions))
check("strict opposite only", all(row["same_pairs"] == "0" and int(row["opposite_pairs"]) > 0 for row in regressions))
check("strict residuals", all(float(row["residual"]) <= 1e-10 for row in regressions))

failed = [name for name, passed in checks if not passed]
for name, passed in checks:
    print(f"{'PASS' if passed else 'FAIL'}: {name}")
print(f"\n{sum(passed for _, passed in checks)}/{len(checks)} checks pass")
if failed:
    raise SystemExit("failed checks: " + ", ".join(failed))
