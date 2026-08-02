"""Independent artifact certificate for FTD-0755.

``--preflight`` checks the frozen implementation and refuses any existing
registered result directory.  The default mode consumes the complete locked
CSV/JSON record; it never runs the dynamics or repairs an artifact.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0755"
PROTOCOL_HASH = "1E713DB4B997DAED0D55F098A6E7D63FC0F2D773391CE44FFE03AADD92A504BC"
EXPECTED_HASHES = {
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_M3_SUPPORT_INVARIANT_VALIDATION_v1.md": PROTOCOL_HASH,
    "engine/include/ftd/eft/support_invariant_matter_predicate.h":
        "B11E087E2E7E16375C173185233AD001AB8B9F049E9B9B5A3156D8618CB4F104",
    "engine/src/eft/support_invariant_matter_predicate.cpp":
        "752CE7C3B03A9944C1E7016A62CCA584FAC868EF191D8241ACEE7E6C9C550D21",
    "engine/tests/campaign_m3_support_invariant_validation_cuda.cpp":
        "F2CCACB00E0DF697B10838E3E85EC636E38BC94E2B2707A55A86811FFE80DCEA",
}
DIRECTIONS = {
    "face": "0_0_1",
    "edge": "0_1_-1",
    "body": "1_1_1",
}
VARIANTS = ("center", "energy_hostile", "graph_hostile")
REGISTERED_NAMES = {
    ("face", "energy_hostile"): "srp_s1p_s2m_rin_fminus",
    ("face", "graph_hostile"): "srp_s1m_s2m_rin_fminus",
    ("edge", "energy_hostile"): "srp_s1m_s2m_rin_fminus",
    ("edge", "graph_hostile"): "srp_s1m_s2p_rin_fminus",
    ("body", "energy_hostile"): "srp_s1m_s2m_rin_fminus",
    ("body", "graph_hostile"): "srp_s1p_s2m_rin_fminus",
}
VOLUMES = (321, 385)
TICKS = tuple(range(160, 313))
CHECKPOINTS = (160, 200, 240, 280, 312)
COMMON_GATE = 1.0e-10
RECOIL_GATE = 1.0e-9
ENERGY_GATE = 1.0e-8
SPEED_GATE = 1.0e-12
CORE_GATE = 1.0e-6
SIGMA_GATE = 1.0e-3
CONDITION_GATE = 1.0e4
SCALE_GATE = 1.0e-5
VOLUME_GATE = 2.0e-13


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


def rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def finite(row: dict[str, str], fields: tuple[str, ...]) -> bool:
    return all(math.isfinite(float(row[field])) for field in fields)


def exact_hashes() -> None:
    check("protocol locked", PROTOCOL_HASH != "UNLOCKED")
    for relative, expected in EXPECTED_HASHES.items():
        path = ROOT / relative
        check(f"source exists: {relative}", path.is_file())
        check(f"source hash locked: {relative}", expected != "UNLOCKED")
        if path.is_file() and expected != "UNLOCKED":
            check(f"source hash exact: {relative}", sha256(path) == expected)


def certify_candidate(arm: str, variant: str) -> dict[str, bool]:
    outcome = {"infrastructure": False, "classifier": False,
               "persists": False, "robust": False, "complete": False}
    stem = f"ftd_0755_m3_candidate_v1_{DIRECTIONS[arm]}_{variant}"
    csv_path, json_path = RESULTS / f"{stem}.csv", RESULTS / f"{stem}.json"
    check(f"{arm}/{variant} csv", csv_path.is_file())
    check(f"{arm}/{variant} json", json_path.is_file())
    if not csv_path.is_file() or not json_path.is_file():
        return outcome
    metadata = json.loads(json_path.read_text(encoding="utf-8"))
    check(f"{arm}/{variant} id", metadata.get("ftd_id") == "FTD-0755")
    check(f"{arm}/{variant} protocol",
          metadata.get("protocol_sha256") == PROTOCOL_HASH)
    check(f"{arm}/{variant} direction",
          metadata.get("direction") == DIRECTIONS[arm])
    check(f"{arm}/{variant} variant", metadata.get("variant") == variant)
    expected_name = "center" if variant == "center" else REGISTERED_NAMES[(arm, variant)]
    check(f"{arm}/{variant} selector",
          metadata.get("registered_name") == expected_name)
    for field in ("small_initialized", "small_executed", "small_pass",
                  "large_initialized", "large_executed", "large_pass",
                  "volume_comparison_pass"):
        check(f"{arm}/{variant} {field} bit", metadata.get(field) in (0, 1))
    check(f"{arm}/{variant} class mismatch count",
          isinstance(metadata.get("class_mismatches"), int))
    check(f"{arm}/{variant} branch mismatch count",
          isinstance(metadata.get("branch_mismatches"), int))
    for field in ("maximum_core_difference", "maximum_constituent_difference",
                  "maximum_local_field_difference"):
        check(f"{arm}/{variant} {field}",
              math.isfinite(float(metadata.get(field, math.inf)))
              and float(metadata[field]) <= VOLUME_GATE)
    check(f"{arm}/{variant} heldout", metadata.get("held_out_validation") is True)
    check(f"{arm}/{variant} dynamics", metadata.get("dynamics_changed") is False)

    data = rows(csv_path)
    grouped: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in data:
        grouped[int(row["volume"])].append(row)
    initialized = (metadata.get("small_initialized") == 1
                   and metadata.get("large_initialized") == 1)
    executed = (metadata.get("small_executed") == 1
                and metadata.get("large_executed") == 1)
    if executed:
        check(f"{arm}/{variant} row count", len(data) == 2 * len(TICKS))
        check(f"{arm}/{variant} volumes", tuple(sorted(grouped)) == VOLUMES)
    volume_passes: list[bool] = []
    all_member = True
    all_clear = True
    all_action = True
    all_regularity = True
    all_observer = True
    all_ladder = True
    for volume in VOLUMES:
        volume_rows = grouped.get(volume, [])
        tick_shape = tuple(int(row["tick"]) for row in volume_rows) == TICKS
        checkpoint_shape = tuple(int(row["tick"]) for row in volume_rows
                                 if row["checkpoint"] == "1") == CHECKPOINTS
        if executed:
            check(f"{arm}/{variant}/L{volume} ticks", tick_shape)
            check(f"{arm}/{variant}/L{volume} checkpoints", checkpoint_shape)
        member = clear = action = regularity = observer = ladder = tick_shape
        for row in volume_rows:
            tick = int(row["tick"])
            prefix = f"{arm}/{variant}/L{volume}/t{tick}"
            check(prefix + " finite", finite(row, (
                "graph_margin", "energy_margin", "pair_energy", "rx", "ry", "rz",
                "p0x", "p0y", "p0z", "p1x", "p1y", "p1z")))
            member = member and row["member"] == "1"
            clear = clear and float(row["graph_margin"]) >= CORE_GATE
            clear = clear and float(row["energy_margin"]) >= CORE_GATE
            observer = observer and row["observer_valid"] == "1"
            ladder = ladder and row["ladder_valid"] == "1"
            if tick == 160:
                continue
            if row["step_valid"] == "1":
                check(prefix + " transaction finite", finite(row, (
                    "max_residual", "energy_residual", "recoil_defect", "speed_excess",
                    "sigma_min", "condition_number", "scale_difference")))
            action = action and row["step_valid"] == "1" and row["common"] == "1"
            action = action and float(row["max_residual"]) <= COMMON_GATE
            action = action and float(row["energy_residual"]) <= ENERGY_GATE
            action = action and float(row["recoil_defect"]) <= RECOIL_GATE
            action = action and float(row["speed_excess"]) <= SPEED_GATE
            action = action and row["graph_local"] == "1"
            action = action and row["site_projection_valid"] == "1"
            regularity = regularity and row["regularity_measured"] == "1"
            regularity = regularity and float(row["sigma_min"]) >= SIGMA_GATE
            regularity = regularity and float(row["condition_number"]) <= CONDITION_GATE
            regularity = regularity and float(row["scale_difference"]) <= SCALE_GATE
        computed_pass = (tick_shape and checkpoint_shape and member and clear
                         and action and regularity and observer and ladder)
        metadata_pass = metadata.get("small_pass") if volume == 321 else metadata.get("large_pass")
        check(f"{arm}/{variant}/L{volume} summary parity",
              metadata_pass == int(computed_pass))
        volume_passes.append(computed_pass)
        all_member = all_member and member
        all_clear = all_clear and clear
        all_action = all_action and action
        all_regularity = all_regularity and regularity
        all_observer = all_observer and observer
        all_ladder = all_ladder and ladder

    recorded_volume = metadata.get("volume_comparison_pass") == 1
    volume_metrics = (metadata.get("class_mismatches") == 0
                      and metadata.get("branch_mismatches") == 0
                      and all(float(metadata[field]) <= VOLUME_GATE for field in (
                          "maximum_core_difference", "maximum_constituent_difference",
                          "maximum_local_field_difference")))
    check(f"{arm}/{variant} volume summary parity",
          recorded_volume == (initialized and executed and volume_metrics))
    outcome["infrastructure"] = initialized and executed
    outcome["classifier"] = all_observer and all_ladder and recorded_volume
    outcome["persists"] = all_member
    outcome["robust"] = all_clear and all_action and all_regularity
    outcome["complete"] = all(volume_passes) and recorded_volume
    return outcome


def certify_fibre(arm: str) -> dict[str, bool]:
    outcome = {"infrastructure": False, "classifier": False}
    stem = f"ftd_0755_m3_causal_fibre_v1_{DIRECTIONS[arm]}"
    csv_path, json_path = RESULTS / f"{stem}.csv", RESULTS / f"{stem}.json"
    check(f"{arm} fibre csv", csv_path.is_file())
    check(f"{arm} fibre json", json_path.is_file())
    if not csv_path.is_file() or not json_path.is_file():
        return outcome
    metadata = json.loads(json_path.read_text(encoding="utf-8"))
    check(f"{arm} fibre id", metadata.get("ftd_id") == "FTD-0755")
    check(f"{arm} fibre protocol", metadata.get("protocol_sha256") == PROTOCOL_HASH)
    check(f"{arm} fibre direction", metadata.get("direction") == DIRECTIONS[arm])
    check(f"{arm} fibre volumes", metadata.get("volumes") == list(VOLUMES))
    check(f"{arm} fibre pass bit", metadata.get("fibre_pass") in (0, 1))
    check(f"{arm} fibre heldout", metadata.get("held_out_validation") is True)
    check(f"{arm} fibre dynamics", metadata.get("dynamics_changed") is False)
    data = rows(csv_path)
    grouped: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in data:
        grouped[int(row["volume"])].append(row)
    records = metadata.get("records")
    check(f"{arm} fibre records", isinstance(records, list) and len(records) == 2)
    record_by_volume = {int(record["volume"]): record for record in records or []}
    infrastructure = True
    classifier = True
    for volume in VOLUMES:
        record = record_by_volume.get(volume, {})
        for field in ("valid", "baseline_initialized", "baseline_executed",
                      "baseline_pass", "remote_initialized", "remote_executed",
                      "remote_pass"):
            check(f"{arm} fibre L{volume} {field} bit", record.get(field) in (0, 1))
        executed = (record.get("baseline_initialized") == 1
                    and record.get("baseline_executed") == 1
                    and record.get("remote_initialized") == 1
                    and record.get("remote_executed") == 1)
        infrastructure = infrastructure and executed
        volume_rows = grouped.get(volume, [])
        tick_shape = tuple(int(row["tick"]) for row in volume_rows) == tuple(range(160, 225))
        if executed:
            check(f"{arm} fibre L{volume} ticks", tick_shape)
        volume_classifier = tick_shape and record.get("baseline_pass") == 1
        volume_classifier = volume_classifier and record.get("remote_pass") == 1
        for row in volume_rows:
            prefix = f"{arm}/fibre/L{volume}/t{row['tick']}"
            check(prefix + " finite", finite(row, (
                "graph_difference", "energy_difference", "constituent_difference",
                "local_field_difference", "bound_energy_difference",
                "initial_global_energy_difference")))
            volume_classifier = volume_classifier and row["baseline_member"] == "1"
            volume_classifier = volume_classifier and row["remote_member"] == "1"
            for field in ("graph_difference", "energy_difference",
                          "constituent_difference", "local_field_difference",
                          "bound_energy_difference"):
                volume_classifier = volume_classifier and float(row[field]) <= VOLUME_GATE
            volume_classifier = (volume_classifier
                                 and float(row["initial_global_energy_difference"]) > 1.0e-12)
        check(f"{arm} fibre L{volume} summary parity",
              record.get("valid") == int(volume_classifier))
        classifier = classifier and volume_classifier
    check(f"{arm} fibre aggregate parity",
          metadata.get("fibre_pass") == int(classifier))
    outcome["infrastructure"] = infrastructure
    outcome["classifier"] = classifier
    return outcome


parser = argparse.ArgumentParser()
parser.add_argument("--preflight", action="store_true")
args = parser.parse_args()
exact_hashes()
if args.preflight:
    check("registered result directory absent", not RESULTS.exists())
    mode = "preflight"
else:
    check("registered result directory exists", RESULTS.is_dir())
    candidates = [certify_candidate(arm, variant)
                  for arm in DIRECTIONS for variant in VARIANTS]
    fibres = [certify_fibre(arm) for arm in DIRECTIONS]
    if not all(item["infrastructure"] for item in candidates + fibres):
        verdict = "M3_VALIDATION_INFRASTRUCTURE_UNRESOLVED"
    elif (not all(item["classifier"] for item in candidates + fibres)):
        verdict = "M3_STATE_ONLY_CLASSIFIER_INVALID"
    elif not all(item["persists"] for item in candidates):
        verdict = "M3_FINITE_TIME_FAMILY_CLOSED_NEGATIVE"
    elif not all(item["robust"] for item in candidates):
        verdict = "M3_SAMPLED_ROBUSTNESS_ONLY"
    elif all(item["complete"] for item in candidates):
        verdict = "M3_FINITE_TIME_SELECTED_MATTER_FAMILY"
    else:
        verdict = "M3_SAMPLED_ROBUSTNESS_ONLY"
    mode = "artifact"

if failures:
    print(f"FTD-0755 {mode}: {checks - len(failures)}/{checks} checks")
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)
print(f"FTD-0755 {mode}: {checks}/{checks} checks")
if not args.preflight:
    print(f"verdict={verdict}")
