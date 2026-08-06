"""Independent run-record certificate for FTD-0731."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_MULTIPASS_FORMATION_PERSISTENCE_v1.md"
TEST = ROOT / "engine/tests/test_multipass_formation_persistence.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0731/ftd_0731_multipass_formation_persistence_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0731/ftd_0731_multipass_formation_persistence_v1.csv"

PREREG_SHA256 = "F319B4CA5C0A8F9A777578507828FC0881E996023FD09AA83033D797B47C01EE"
TEST_SHA256 = "CE40EFAC3ED27A3101B205104331F0407A5D90A12694F3D33E9A63E00B575266"
JSON_SHA256 = "0D4F8519F44F15BF941A410D055947EE4E079A115AF41C476866DF413D45F03D"
CSV_SHA256 = "BC060706C00E5A15A0C8FF34960EB521301BA73DDC9F33015E1930D65DE5F163"
VERDICT = "MULTIPASS_RADIATIVE_CAPTURE_VOLUME_STABLE"
CUTOFF2 = 1.5
MORPHOLOGY_TICKS = (48, 96, 128, 160, 192)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, message: str, checks: list[str]) -> None:
    if not condition:
        raise AssertionError(message)
    checks.append(message)


def close(actual: float, expected: float, tolerance: float = 1e-13) -> bool:
    return abs(actual - expected) <= tolerance * max(1.0, abs(expected))


def values(row: dict[str, str], name: str) -> list[float]:
    return [float(value) for value in row[name].split(";")]


def ticks(row: dict[str, str]) -> list[int]:
    return [] if not row["transition_ticks"] else [
        int(value) for value in row["transition_ticks"].split(";")
    ]


def key(row: dict[str, str]) -> tuple[str, str, str, str]:
    return row["family"], row["momentum"], row["direction"], row["polarity"]


def inside(separation: float) -> bool:
    return separation * separation < CUTOFF2


def negative_inside(
    separation: list[float], internal: list[float], first: int, last: int
) -> bool:
    return all(
        internal[tick] < -1e-6 and inside(separation[tick])
        for tick in range(first, last + 1)
    )


def recompute_transitions(separation: list[float]) -> list[int]:
    membership = [inside(value) for value in separation]
    return [
        tick for tick in range(1, len(membership))
        if membership[tick] != membership[tick - 1]
    ]


def main() -> None:
    checks: list[str] = []
    check(sha256(PREREG) == PREREG_SHA256, "protocol hash locked", checks)
    check(sha256(TEST) == TEST_SHA256, "runner hash locked", checks)
    check(sha256(JSON_PATH) == JSON_SHA256, "JSON hash locked", checks)
    check(sha256(CSV_PATH) == CSV_SHA256, "CSV hash locked", checks)
    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    check(summary["identifier"] == "FTD-0731", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256,
          "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked verdict", checks)
    check(summary["arm_count"] == 48 and len(rows) == 48,
          "48 locked histories", checks)
    check({int(row["volume"]) for row in rows} == {33, 65},
          "two volumes", checks)
    check({row["direction"] for row in rows}
          == {"0_0_1", "0_1_-1", "1_1_1"},
          "three cubic direction classes", checks)
    check({row["polarity"] for row in rows}
          == {"plus_minus", "minus_plus"}, "two polarities", checks)
    check(all(row["initialized"] == "1" for row in rows),
          "all initial dresses valid", checks)
    check(all(row["executed"] == "1" for row in rows),
          "all histories execute", checks)
    check(all(row["identity_pass"] == "1" for row in rows),
          "all action identities pass", checks)
    check(all(row["inverse_pass"] == "1" for row in rows),
          "all state-only inverses pass", checks)
    check(all(row["recoil_pass"] == "1" for row in rows),
          "all recoil gates pass", checks)

    for index, row in enumerate(rows):
        separation = values(row, "separation_history")
        internal = values(row, "internal_history")
        field = values(row, "field_history")
        label = f"row {index} L={row['volume']} {row['momentum']} {row['direction']} {row['polarity']}"
        check(len(separation) == len(internal) == len(field) == 193,
              f"193 persisted states {label}", checks)
        observed_ticks = recompute_transitions(separation)
        check(observed_ticks == ticks(row),
              f"transition ticks recomputed {label}", checks)
        check(int(row["graph_transitions"]) == len(observed_ticks),
              f"transition count recomputed {label}", checks)
        balance = abs((field[-1] - field[0]) + (internal[-1] - internal[0]))
        check(close(balance, float(row["pair_field_balance"])),
              f"energy balance recomputed {label}", checks)
        check(balance <= 1e-8, f"energy balance gate {label}", checks)
        check(float(row["inverse_recovery"]) <= 1e-8,
              f"inverse gate {label}", checks)
        check(float(row["max_common_residual"]) <= 1e-10,
              f"common-action residual {label}", checks)
        check(float(row["max_recoil_defect"]) <= 1e-9,
              f"recoil residual {label}", checks)

        momentum = float(row["momentum"])
        if row["family"] == "unbound" and close(momentum, 0.012):
            initial_positive_outside = internal[0] > 1e-6 and not inside(separation[0])
            final_tail = negative_inside(separation, internal, 129, 192)
            final_transition_entry = (
                bool(observed_ticks) and len(observed_ticks) % 2 == 1
                and inside(separation[-1])
            )
            field_gain = field[-1] > field[0]
            morphology = any(
                float(row[f"dynamic_norm_{tick}"]) > 1e-8
                and float(row[f"magnetic_energy_{tick}"]) > 1e-10
                and int(row[f"median_radius2_{tick}"]) >= 5
                for tick in MORPHOLOGY_TICKS
            )
            durable = (
                initial_positive_outside and len(observed_ticks) >= 3
                and final_transition_entry and final_tail and field_gain
                and morphology
            )
            check(durable, f"durable capture recomputed {label}", checks)
            check(row["durable_multipass_capture"] == "1",
                  f"emitted durable class agrees {label}", checks)
            check(row["final_class"] == "durable_multipass_capture",
                  f"emitted final class agrees {label}", checks)
        elif row["family"] == "bound":
            persistent = internal[0] < -1e-6 and negative_inside(
                separation, internal, 97, 192
            )
            check(persistent and row["bound_control_pass"] == "1",
                  f"bound control tail recomputed {label}", checks)
        else:
            persistent = negative_inside(separation, internal, 97, 192)
            check(persistent and row["extended_persistent"] == "1",
                  f"parent tail recomputed {label}", checks)

    by_volume = {
        volume: [row for row in rows if int(row["volume"]) == volume]
        for volume in (33, 65)
    }
    expected_third = {"0_0_1": 63, "0_1_-1": 79, "1_1_1": 96}
    for volume, group in by_volume.items():
        check(len(group) == 24, f"24 arms L={volume}", checks)
        p012 = [row for row in group if close(float(row["momentum"]), 0.012)]
        parents = [
            row for row in group
            if row["family"] == "unbound" and float(row["momentum"]) < 0.012
        ]
        bound = [row for row in group if row["family"] == "bound"]
        check(len(p012) == 6 and len(parents) == 12 and len(bound) == 6,
              f"matrix partition L={volume}", checks)
        for row in p012:
            expected = [7, 26, expected_third[row["direction"]]]
            check(ticks(row) == expected,
                  f"locked transition sequence L={volume} {row['direction']} {row['polarity']}", checks)

    rows33 = {key(row): row for row in by_volume[33]}
    rows65 = {key(row): row for row in by_volume[65]}
    check(rows33.keys() == rows65.keys(), "matched volume keys", checks)
    for arm_key in rows33:
        smaller, larger = rows33[arm_key], rows65[arm_key]
        smaller_ticks, larger_ticks = ticks(smaller), ticks(larger)
        check(len(smaller_ticks) == len(larger_ticks),
              f"volume transition-count match {arm_key}", checks)
        check(all(abs(a - b) <= 2 for a, b in zip(smaller_ticks, larger_ticks)),
              f"volume transition-time gate {arm_key}", checks)
        check(smaller["final_class"] == larger["final_class"],
              f"volume final-class match {arm_key}", checks)

    check(summary["matched_arms"] == 24, "summary matched arms", checks)
    check(summary["matched_p012"] == 6, "summary matched p012", checks)
    check(summary["transition_count_mismatches"] == 0,
          "summary transition-count match", checks)
    check(summary["transition_timing_mismatches"] == 0,
          "summary transition-timing match", checks)
    check(summary["final_class_mismatches"] == 0,
          "summary final-class match", checks)
    check(summary["maximum_transition_tick_difference"] == 0,
          "zero observed volume transition shift", checks)
    for item in summary["volumes"]:
        volume = item["volume"]
        group = by_volume[volume]
        check(item["executed"] == item["identities"] == 24,
              f"summary execution L={volume}", checks)
        check(item["inverses"] == item["recoils"] == 24,
              f"summary inverse/recoil L={volume}", checks)
        check(item["parent_persistent"] == 12,
              f"summary parent persistence L={volume}", checks)
        check(item["bound_controls"] == 6,
              f"summary bound controls L={volume}", checks)
        check(item["durable_capture"] == 6,
              f"summary durable capture L={volume}", checks)
        check(item["recurrent_scattering"] == item["later_release"]
              == item["p012_other"] == 0,
              f"summary alternative classes absent L={volume}", checks)
        check(close(item["maximum_inverse"], max(
            float(row["inverse_recovery"]) for row in group)),
            f"summary inverse maximum L={volume}", checks)
        check(close(item["maximum_balance"], max(
            float(row["pair_field_balance"]) for row in group)),
            f"summary balance maximum L={volume}", checks)

    print(f"FTD-0731 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print("p012 transitions: face 7/26/63, edge 7/26/79, body 7/26/96")
    print("all p012 tails negative/inside ticks 129-192; volume transition shift=0")


if __name__ == "__main__":
    main()
