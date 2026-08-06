"""Independent run-record certificate for FTD-0720."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations" / (
    "PREREG_INTERACTING_COMMON_ACTION_ROOT_MULTISEED_v1.md"
)
RESULT = ROOT / "engine/results/ftd_0720" / (
    "ftd_0720_interacting_common_action_root_multiseed_v1.json"
)
ROWS = ROOT / "engine/results/ftd_0720" / (
    "ftd_0720_interacting_common_action_root_multiseed_v1.csv"
)

PROTOCOL = "DB516877F7762BECF9E61AB54861A616F84DCA7E6701907A8CE1F3D4BA21668C"
RESULT_HASH = "AB54FD7025BF2D6F4DC583287B9D85E22A3A06535BADAEF430CE3B9EB0248DA6"
ROWS_HASH = "12D3274A6831004E9E4EBFB28A4052A44C59C189D9AE68156943227E750F2165"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, label: str, checks: list[str]) -> None:
    assert condition, label
    checks.append(label)


def main() -> None:
    checks: list[str] = []
    check(digest(PREREG) == PROTOCOL, "locked protocol hash", checks)
    check(digest(RESULT) == RESULT_HASH, "JSON run-record hash", checks)
    check(digest(ROWS) == ROWS_HASH, "CSV run-record hash", checks)

    summary = json.loads(RESULT.read_text(encoding="utf-8"))
    check(summary["ftd_id"] == "FTD-0720", "identifier", checks)
    check(summary["protocol_sha256"] == PROTOCOL, "protocol embedded", checks)
    check(summary["production_changed"] is False, "production unchanged", checks)
    check(summary["arms"] == 3, "three interacting arms", checks)
    check(summary["registered_seeds"] == 39, "39 seeds registered", checks)
    check(summary["accepted_seeds"] == 39, "39 seeds accepted", checks)
    check(summary["inverse_pass_seeds"] == 39, "39 inverses accepted", checks)
    check(summary["all_seed_arms"] == 3, "all seed arms complete", checks)
    check(summary["relabeling_pass_arms"] == 3, "three relabeling arms", checks)
    check(summary["multiple_root_arms"] == 0, "no distinct root witnessed", checks)
    check(
        summary["maximum_state_difference"] <= 1e-9,
        "complete-state root agreement",
        checks,
    )
    check(
        summary["maximum_current_difference"] <= 1e-9,
        "deposited-current agreement",
        checks,
    )
    check(
        summary["maximum_inverse_recovery"] <= 1e-8,
        "state-only inverse recovery",
        checks,
    )
    check(
        summary["verdict"] == "INTERACTING_COMMON_ACTION_ONE_BASIN_WITNESSED",
        "locked one-basin verdict",
        checks,
    )

    with ROWS.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    arms = {"rest_axial", "fractional_parallel", "fractional_transverse"}
    seeds = {"incoming"}
    for family in ("common", "odd"):
        for axis in "xyz":
            for sign in ("minus", "plus"):
                seeds.add(f"{family}_{axis}_{sign}")
    check(len(rows) == 39, "39 CSV observations", checks)
    check({row["arm"] for row in rows} == arms, "arm set exact", checks)
    for arm in arms:
        selected = [row for row in rows if row["arm"] == arm]
        check(len(selected) == 13, f"{arm}: 13 seeds", checks)
        check({row["seed"] for row in selected} == seeds,
              f"{arm}: seed set exact", checks)
    check(all(row["accepted"] == "1" for row in rows),
          "every forward root accepted", checks)
    check(all(row["inverse"] == "1" for row in rows),
          "every inverse accepted", checks)
    check(max(float(row["state_difference"]) for row in rows) <= 1e-9,
          "rowwise state agreement", checks)
    check(max(float(row["current_difference"]) for row in rows) <= 1e-9,
          "rowwise current agreement", checks)
    check(max(float(row["recovery"]) for row in rows) <= 1e-8,
          "rowwise inverse agreement", checks)

    print(f"FTD-0720 certificate: {len(checks)}/{len(checks)} checks PASS")
    print("verdict=INTERACTING_COMMON_ACTION_ONE_BASIN_WITNESSED")


if __name__ == "__main__":
    main()

