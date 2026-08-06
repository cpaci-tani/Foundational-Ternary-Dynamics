"""Independent certificate for FTD-0756 parent-replay forensics."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0756"
BASELINE = ROOT / "engine" / "results" / "ftd_0753"
PROTOCOL_HASH = "773BDB791B06A0250C980945A1B52EF9F2A6F119EF8905E9AC57DC83A6FB5CFC"
EXPECTED_HASHES = {
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_M3_PARENT_REPLAY_FORENSICS_v1.md": PROTOCOL_HASH,
    "engine/tests/campaign_m3_parent_replay_forensics_cuda.cpp":
        "66FAFE008B4008BA20674A7EA0D562E5D4B7E07B4B2A3C6469E92861DEAF90CE",
}
ARMS = {"face": "0_0_1", "edge": "0_1_-1", "body": "1_1_1"}
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
    replay_divergence = False
    wrapper_failure = False
    predicate_failure = False
    l321_pass = True
    l385_pass = True
    total_rows = 0
    exact_scalar_comparisons = 0
    maximum_numeric_difference = 0.0

    for arm, direction in ARMS.items():
        baseline_path = BASELINE / (
            f"ftd_0753_explicit_rounding_causal_horizon_m2_v1_{arm}.csv")
        check(f"{arm} baseline exists", baseline_path.is_file())
        baseline = {int(row["tick"]): row for row in read_csv(baseline_path)}
        for volume in VOLUMES:
            stem = f"ftd_0756_m3_parent_forensics_v1_{arm}_L{volume}"
            csv_path, json_path = RESULTS / f"{stem}.csv", RESULTS / f"{stem}.json"
            check(f"{arm}/L{volume} csv", csv_path.is_file())
            check(f"{arm}/L{volume} json", json_path.is_file())
            if not csv_path.is_file() or not json_path.is_file():
                continue
            metadata = json.loads(json_path.read_text(encoding="utf-8"))
            data = read_csv(csv_path)
            total_rows += len(data)
            check(f"{arm}/L{volume} id", metadata.get("ftd_id") == "FTD-0756")
            check(f"{arm}/L{volume} protocol",
                  metadata.get("protocol_sha256") == PROTOCOL_HASH)
            check(f"{arm}/L{volume} volume", metadata.get("volume") == volume)
            check(f"{arm}/L{volume} arm", metadata.get("arm") == arm)
            check(f"{arm}/L{volume} direction", metadata.get("direction") == direction)
            check(f"{arm}/L{volume} rows", metadata.get("row_count") == len(data))
            check(f"{arm}/L{volume} dynamics", metadata.get("dynamics_changed") is False)
            check(f"{arm}/L{volume} tick sequence",
                  tuple(int(row["tick"]) for row in data) == tuple(range(len(data))))
            for row in data:
                check(f"{arm}/L{volume}/t{row['tick']} labels",
                      int(row["volume"]) == volume and row["arm"] == arm
                      and row["direction"] == direction)
                check(f"{arm}/L{volume}/t{row['tick']} finite core",
                      all(math.isfinite(float(row[field])) for field in (
                          "separation_squared", "separation", "graph_margin",
                          "energy_margin", "pair_energy")))

            prep = all(metadata.get(field) == 1 for field in (
                "preparation_valid", "density_contained", "compact_support",
                "zero_boundary_crossing"))
            preparation_failure = preparation_failure or not prep
            parent_pass = (metadata.get("reached_tick_160") == 1
                           and metadata.get("final_sector_valid") == 1
                           and metadata.get("final_member") == 1
                           and float(metadata.get("final_graph_margin", -math.inf)) >= 1e-6
                           and float(metadata.get("final_energy_margin", -math.inf)) >= 1e-6)
            if volume == 321:
                l321_pass = l321_pass and parent_pass
            else:
                l385_pass = l385_pass and parent_pass

            first_failure = int(metadata.get("first_failure_tick", -1))
            if prep and first_failure >= 0:
                wrapper_failure = wrapper_failure or volume == 321
            if (metadata.get("reached_tick_160") == 1
                    and metadata.get("final_member") != 1):
                predicate_failure = predicate_failure or volume == 321

            if volume != 321:
                continue
            for row in data:
                tick = int(row["tick"])
                if tick not in baseline:
                    replay_divergence = True
                    continue
                source = baseline[tick]
                fields = ("separation", "pair_energy") if tick == 0 else tuple(COMPARE)
                if tick > 0 and (row["step_valid"] != "1" or row["common"] != "1"):
                    continue
                if tick > 0:
                    check(f"{arm}/t{tick} valid string", source["valid"] == "1")
                    check(f"{arm}/t{tick} common string", source["common"] == "1")
                    exact_scalar_comparisons += 2
                for field in fields:
                    baseline_field = COMPARE.get(field, field)
                    exact = row[field] == source[baseline_field]
                    replay_divergence = replay_divergence or not exact
                    exact_scalar_comparisons += 1
                    maximum_numeric_difference = max(maximum_numeric_difference,
                        abs(float(row[field]) - float(source[baseline_field])))

    if preparation_failure:
        verdict = "M3_PARENT_FINITE_SUPPORT_PREPARATION_FAILURE"
    elif replay_divergence:
        verdict = "M3_PARENT_REPLAY_DIVERGENCE"
    elif wrapper_failure:
        verdict = "M3_PARENT_WRAPPER_TRANSACTION_FAILURE"
    elif predicate_failure:
        verdict = "M3_PARENT_PREDICATE_RECONCILIATION_FAILURE"
    elif l321_pass and not l385_pass:
        verdict = "M3_PARENT_LARGE_VOLUME_FAILURE"
    elif l321_pass and l385_pass:
        verdict = "M3_PARENT_FORENSICS_PASS_FTD0755_DISPOSITION_INCONSISTENT"
    else:
        verdict = "M3_PARENT_WRAPPER_TRANSACTION_FAILURE"
    mode = "artifact"

if failures:
    print(f"FTD-0756 {mode}: {checks-len(failures)}/{checks} checks")
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)
print(f"FTD-0756 {mode}: {checks}/{checks} checks")
print(f"verdict={verdict}")
if not args.preflight:
    print(f"rows={total_rows}")
    print(f"exact_scalar_comparisons={exact_scalar_comparisons}")
    print(f"maximum_numeric_difference={maximum_numeric_difference:.17g}")
