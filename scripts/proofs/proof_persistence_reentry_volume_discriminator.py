"""Independent run-record certificate for FTD-0730."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_PERSISTENCE_REENTRY_VOLUME_DISCRIMINATOR_v1.md"
TEST = ROOT / "engine/tests/test_persistence_reentry_volume_discriminator.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0730/ftd_0730_persistence_reentry_volume_discriminator_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0730/ftd_0730_persistence_reentry_volume_discriminator_v1.csv"

PREREG_SHA256 = "50582DF6FAE3DBBC27AF4E9B271F4E141597BE04E1EF55FE0DF6C137C9ABEB83"
TEST_SHA256 = "6EBAC4DA86F26C6F5B5D73FAD59777CF4B507512A423818F350EB09584BED210"
JSON_SHA256 = "ADA8931C266E860FD7D38C2D9FC14435FDCD615DCBFF5A0BB9257CE98E706DB4"
CSV_SHA256 = "2C40C6B38A9CF48CB44779E5334D8DEBC88A661602765CA0A20F0F6F1A9EFCCA"
VERDICT = "P012_REENTRY_LOCAL_DYNAMICS_VOLUME_STABLE"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, message: str, checks: list[str]) -> None:
    if not condition:
        raise AssertionError(message)
    checks.append(message)


def close(actual: float, expected: float, tolerance: float = 1e-13) -> bool:
    return abs(actual - expected) <= tolerance * max(1.0, abs(expected))


def key(row: dict[str, str]) -> tuple[str, str, str, str]:
    return row["family"], row["momentum"], row["direction"], row["polarity"]


def main() -> None:
    checks: list[str] = []
    check(sha256(PREREG) == PREREG_SHA256, "protocol hash locked", checks)
    check(sha256(TEST) == TEST_SHA256, "runner hash locked", checks)
    check(sha256(JSON_PATH) == JSON_SHA256, "JSON hash locked", checks)
    check(sha256(CSV_PATH) == CSV_SHA256, "CSV hash locked", checks)
    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    check(summary["identifier"] == "FTD-0730", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256, "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked verdict", checks)
    check(summary["arm_count"] == 88 and len(rows) == 88, "88 locked histories", checks)
    check({int(row["volume"]) for row in rows} == {33, 65}, "two volumes", checks)
    check({row["polarity"] for row in rows} == {"plus_minus", "minus_plus"}, "two polarities", checks)
    check(all(row["initialized"] == "1" for row in rows), "all initial dresses valid", checks)
    check(all(row["executed"] == "1" for row in rows), "all histories execute", checks)
    check(all(row["identity_pass"] == "1" for row in rows), "all identities pass", checks)
    check(all(row["inverse_pass"] == "1" for row in rows), "all inverses pass", checks)
    check(all(row["recoil_pass"] == "1" for row in rows), "all recoils pass", checks)
    check(max(float(row["pair_field_balance"]) for row in rows) <= 1e-8, "energy balance gate", checks)
    check(max(float(row["inverse_recovery"]) for row in rows) <= 1e-8, "inverse recovery gate", checks)

    by_volume = {L: [row for row in rows if int(row["volume"]) == L] for L in (33, 65)}
    for L, group in by_volume.items():
        check(len(group) == 44, f"44 arms L={L}", checks)
        bound = [row for row in group if row["family"] == "bound"]
        parent = [row for row in group if row["family"] == "unbound" and float(row["momentum"]) < 0.012]
        p012 = [row for row in group if row["family"] == "unbound" and float(row["momentum"]) == 0.012]
        check(len(bound) == 6 and len(parent) == 12 and len(p012) == 26, f"matrix partition L={L}", checks)
        check(all(row["bound_control_pass"] == "1" for row in bound), f"bound controls L={L}", checks)
        check(all(int(row["graph_transitions"]) == 0 for row in bound), f"bound topology L={L}", checks)
        check(all(row["tail_persistent"] == "1" for row in parent), f"parent persistence L={L}", checks)
        check(all(int(row["graph_transitions"]) == 1 for row in parent), f"parent entry topology L={L}", checks)
        check(all(int(row["dynamic_median_radius2_48"]) == 3 for row in parent), f"parent radius tick48 L={L}", checks)
        check(all(int(row["dynamic_median_radius2_96"]) in {5, 6} for row in parent), f"parent radius tick96 L={L}", checks)
        check(all(int(row["graph_transitions"]) == 3 for row in p012), f"p012 three transitions L={L}", checks)
        check(all(int(row["transition_tick_1"]) == 7 for row in p012), f"p012 entry tick L={L}", checks)
        check(all(int(row["transition_tick_2"]) == 26 for row in p012), f"p012 exit tick L={L}", checks)
        check(sum(row["negative_sector"] == "1" for row in p012) == 6, f"p012 final negative count L={L}", checks)

    rows33 = {key(row): row for row in by_volume[33]}
    rows65 = {key(row): row for row in by_volume[65]}
    check(rows33.keys() == rows65.keys(), "matched volume keys", checks)
    for k in rows33:
        a, b = rows33[k], rows65[k]
        check(a["graph_transitions"] == b["graph_transitions"], f"transition count match {k}", checks)
        check(a["transition_tick_1"] == b["transition_tick_1"], f"transition one match {k}", checks)
        check(a["transition_tick_2"] == b["transition_tick_2"], f"transition two match {k}", checks)
        check(a["transition_tick_3"] == b["transition_tick_3"], f"transition three match {k}", checks)
        check(a["negative_sector"] == b["negative_sector"], f"sign class match {k}", checks)
        check(a["dynamic_median_radius2_96"] == b["dynamic_median_radius2_96"], f"radius match {k}", checks)

    face = {"0_0_1", "0_1_0", "1_0_0"}
    edge = {"0_1_-1", "0_1_1", "1_-1_0", "1_0_-1", "1_0_1", "1_1_0"}
    body = {"1_-1_-1", "1_-1_1", "1_1_-1", "1_1_1"}
    p012_33 = [row for row in by_volume[33] if float(row["momentum"]) == 0.012]
    for row in p012_33:
        direction = row["direction"]
        if direction in face:
            check(int(row["transition_tick_3"]) == 63, f"face re-entry tick {row['polarity']} {direction}", checks)
            check(row["negative_sector"] == "1", f"face final negative {row['polarity']} {direction}", checks)
            check(int(row["dynamic_median_radius2_96"]) == 3, f"face radius {row['polarity']} {direction}", checks)
        elif direction in edge:
            check(int(row["transition_tick_3"]) == 79, f"edge re-entry tick {row['polarity']} {direction}", checks)
            check(int(row["dynamic_median_radius2_96"]) == 6, f"edge radius {row['polarity']} {direction}", checks)
        else:
            check(direction in body, f"body direction classified {direction}", checks)
            check(int(row["transition_tick_3"]) == 96, f"body re-entry tick {row['polarity']} {direction}", checks)
            check(int(row["dynamic_median_radius2_96"]) == 12, f"body radius {row['polarity']} {direction}", checks)

    check(summary["matched_p012"] == 26, "summary matched p012", checks)
    check(summary["both_reentered"] == 26, "summary both reentered", checks)
    check(summary["l65_reentered"] == 26, "summary L65 reentered", checks)
    check(summary["matched_parents"] == 12, "summary matched parents", checks)
    check(summary["persistent_both"] == 12, "summary persistent both", checks)
    check(summary["maximum_third_transition_tick_difference"] == 0, "zero transition-time shift", checks)
    check(summary["maximum_parent_radius_difference"] == 0, "zero parent-radius shift", checks)
    for item in summary["volumes"]:
        L = item["volume"]
        group = by_volume[L]
        check(item["executed"] == 44 and item["identities"] == 44, f"summary execution L={L}", checks)
        check(item["inverses"] == 44 and item["recoils"] == 44, f"summary inverse/recoil L={L}", checks)
        check(close(item["maximum_inverse"], max(float(row["inverse_recovery"]) for row in group)), f"summary inverse max L={L}", checks)
        check(close(item["maximum_balance"], max(float(row["pair_field_balance"]) for row in group)), f"summary balance max L={L}", checks)

    print(f"FTD-0730 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print("p012 transitions: face 7/26/63, edge 7/26/79, body 7/26/96")
    print("volume shifts: third_transition=0; parent_radius=0")


if __name__ == "__main__":
    main()
