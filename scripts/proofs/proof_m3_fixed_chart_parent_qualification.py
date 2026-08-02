"""Independent artifact certificate for FTD-0757 fixed-chart qualification."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0757"
BASELINE = ROOT / "engine" / "results" / "ftd_0753"
PROTOCOL_HASH = "E867A86868E00673EDAA716F1D7CB021A2E9BFB6F798BDC8C552385C4EE6DB50"
EXPECTED_HASHES = {
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_M3_FIXED_CHART_PARENT_QUALIFICATION_v1.md": PROTOCOL_HASH,
    "engine/tests/campaign_m3_fixed_chart_parent_qualification_cuda.cpp":
        "09D850CD775D7746623E16DAA1FBECBFED068E9119FB344DDDE46A01B30F88E0",
}
ARMS = {
    "face": ("0_0_1", 57),
    "edge": ("0_1_-1", 30),
    "body": ("1_1_1", 122),
}
VOLUMES = (321, 385)
COMPARE = {
    "separation": "separation",
    "pair_energy": "pair_energy",
    "max_residual": "max_residual",
    "energy_residual": "total_energy_residual",
    "recoil_defect": "recoil_defect",
    "speed_excess": "speed_excess",
}

checks = 0
failures: list[str] = []


def check(label: str, condition: bool) -> None:
    global checks
    checks += 1
    if not condition:
        failures.append(label)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def hashes() -> None:
    check("protocol locked", PROTOCOL_HASH != "UNLOCKED")
    for relative, expected in EXPECTED_HASHES.items():
        path = ROOT / relative
        check(f"exists: {relative}", path.is_file())
        check(f"hash locked: {relative}", expected != "UNLOCKED")
        if path.is_file() and expected != "UNLOCKED":
            check(f"hash exact: {relative}", sha256(path) == expected)


parser = argparse.ArgumentParser()
parser.add_argument("--preflight", action="store_true")
args = parser.parse_args()
hashes()

if args.preflight:
    check("result directory absent", not RESULTS.exists())
    mode = "preflight"
    verdict = "NOT_RUN"
else:
    check("result directory exists", RESULTS.is_dir())
    preparation_failure = False
    observer_failure = False
    dynamics_failure = False
    replay_divergence = False
    diagnosis_incomplete = False
    volume_pass = {321: True, 385: True}
    total_rows = 0
    exact_scalar_comparisons = 0
    maximum_numeric_difference = 0.0

    for arm, (direction, expected_fractional) in ARMS.items():
        baseline_path = BASELINE / (
            f"ftd_0753_explicit_rounding_causal_horizon_m2_v1_{arm}.csv")
        check(f"{arm} baseline exists", baseline_path.is_file())
        baseline = {int(row["tick"]): row for row in read_csv(baseline_path)}
        for volume in VOLUMES:
            stem = f"ftd_0757_m3_fixed_chart_parent_v1_{arm}_L{volume}"
            csv_path = RESULTS / f"{stem}.csv"
            json_path = RESULTS / f"{stem}.json"
            check(f"{arm}/L{volume} csv", csv_path.is_file())
            check(f"{arm}/L{volume} json", json_path.is_file())
            if not csv_path.is_file() or not json_path.is_file():
                volume_pass[volume] = False
                continue
            metadata = json.loads(json_path.read_text(encoding="utf-8"))
            data = read_csv(csv_path)
            total_rows += len(data)
            center = str(volume // 2)
            check(f"{arm}/L{volume} id", metadata.get("ftd_id") == "FTD-0757")
            check(f"{arm}/L{volume} protocol",
                  metadata.get("protocol_sha256") == PROTOCOL_HASH)
            check(f"{arm}/L{volume} volume", metadata.get("volume") == volume)
            check(f"{arm}/L{volume} arm", metadata.get("arm") == arm)
            check(f"{arm}/L{volume} direction", metadata.get("direction") == direction)
            check(f"{arm}/L{volume} fixed center",
                  metadata.get("fixed_center") == [volume // 2] * 3)
            check(f"{arm}/L{volume} row count", metadata.get("row_count") == len(data))
            check(f"{arm}/L{volume} tick sequence",
                  tuple(int(row["tick"]) for row in data) == tuple(range(len(data))))
            check(f"{arm}/L{volume} dynamics unchanged",
                  metadata.get("dynamics_changed") is False)
            for row in data:
                tick = int(row["tick"])
                check(f"{arm}/L{volume}/t{tick} labels",
                      int(row["volume"]) == volume and row["arm"] == arm
                      and row["direction"] == direction)
                check(f"{arm}/L{volume}/t{tick} chart",
                      row["fixed_center_x"] == center
                      and row["fixed_center_y"] == center
                      and row["fixed_center_z"] == center)
                midpoint = tuple(float(row[field]) for field in (
                    "midpoint_x", "midpoint_y", "midpoint_z"))
                residual = max(abs(value - round(value)) for value in midpoint)
                recorded = float(row["midpoint_integer_residual"])
                check(f"{arm}/L{volume}/t{tick} midpoint finite",
                      all(math.isfinite(value) for value in midpoint))
                check(f"{arm}/L{volume}/t{tick} midpoint residual",
                      residual == recorded)
                check(f"{arm}/L{volume}/t{tick} API bit",
                      (row["moving_center_api_admissible"] == "1")
                      == (recorded == 0.0))
                check(f"{arm}/L{volume}/t{tick} finite core",
                      all(math.isfinite(float(row[field])) for field in (
                          "separation_squared", "separation", "graph_margin",
                          "energy_margin", "pair_energy")))

            prep = all(metadata.get(field) == 1 for field in (
                "preparation_valid", "density_contained", "compact_support",
                "zero_boundary_crossing"))
            preparation_failure = preparation_failure or not prep
            first_failure_stage = int(metadata.get("first_failure_stage", 0))
            observer_failure = observer_failure or first_failure_stage == 4
            dynamics_failure = dynamics_failure or (
                int(metadata.get("first_failure_tick", -1)) >= 0
                and first_failure_stage != 4)
            diagnosis_ok = (
                metadata.get("first_fractional_midpoint_tick") == expected_fractional
                and metadata.get("expected_fractional_midpoint_tick")
                == expected_fractional)
            diagnosis_incomplete = diagnosis_incomplete or not diagnosis_ok
            parent_pass = (
                metadata.get("reached_tick_160") == 1
                and len(data) == 161
                and metadata.get("first_failure_tick") == -1
                and metadata.get("final_sector_valid") == 1
                and metadata.get("final_member") == 1
                and float(metadata.get("final_graph_margin", -math.inf)) >= 1e-6
                and float(metadata.get("final_energy_margin", -math.inf)) >= 1e-6)
            volume_pass[volume] = volume_pass[volume] and parent_pass
            if parent_pass:
                for row in data[1:]:
                    tick = int(row["tick"])
                    check(f"{arm}/L{volume}/t{tick} step pass",
                          row["step_valid"] == "1" and row["common"] == "1"
                          and row["observer_valid"] == "1"
                          and row["failure_stage"] == "0")

            if volume != 321:
                continue
            for row in data:
                tick = int(row["tick"])
                if tick not in baseline:
                    replay_divergence = True
                    continue
                source = baseline[tick]
                fields = ("separation", "pair_energy") if tick == 0 else tuple(COMPARE)
                if tick > 0:
                    for field in ("valid", "common"):
                        exact = row["step_valid" if field == "valid" else field] == source[field]
                        replay_divergence = replay_divergence or not exact
                        exact_scalar_comparisons += 1
                for field in fields:
                    baseline_field = COMPARE.get(field, field)
                    exact = row[field] == source[baseline_field]
                    replay_divergence = replay_divergence or not exact
                    exact_scalar_comparisons += 1
                    maximum_numeric_difference = max(
                        maximum_numeric_difference,
                        abs(float(row[field]) - float(source[baseline_field])))

    if preparation_failure:
        verdict = "M3_FIXED_CHART_PREPARATION_FAILURE"
    elif observer_failure:
        verdict = "M3_FIXED_CHART_OBSERVER_FAILURE"
    elif dynamics_failure:
        verdict = "M3_FIXED_CHART_PARENT_DYNAMICS_FAILURE"
    elif replay_divergence:
        verdict = "M3_FIXED_CHART_PARENT_REPLAY_DIVERGENCE"
    elif diagnosis_incomplete:
        verdict = "M3_MOVING_CENTER_DIAGNOSIS_INCOMPLETE"
    elif volume_pass[321] and not volume_pass[385]:
        verdict = "M3_FIXED_CHART_LARGE_VOLUME_FAILURE"
    elif volume_pass[321] and volume_pass[385]:
        verdict = "M3_FIXED_CHART_PARENT_QUALIFIED"
    else:
        verdict = "M3_FIXED_CHART_PARENT_DYNAMICS_FAILURE"
    mode = "artifact"

if failures:
    print(f"FTD-0757 {mode}: {checks-len(failures)}/{checks} checks")
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)
print(f"FTD-0757 {mode}: {checks}/{checks} checks")
print(f"verdict={verdict}")
if not args.preflight:
    print(f"rows={total_rows}")
    print(f"exact_scalar_comparisons={exact_scalar_comparisons}")
    print(f"maximum_numeric_difference={maximum_numeric_difference:.17g}")
