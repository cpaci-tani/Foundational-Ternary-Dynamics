#!/usr/bin/env python3
"""Independent certificate for FTD-0737 precontact energetic capture."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_PRECONTACT_ENERGETIC_CAPTURE_DELAY_v1.md"
RUNNER = ROOT / "engine/tests/test_precontact_energetic_capture_delay.cpp"
RESULT_JSON = ROOT / "engine/results/ftd_0737/ftd_0737_precontact_energetic_capture_delay_v1.json"
RESULT_CSV = ROOT / "engine/results/ftd_0737/ftd_0737_precontact_energetic_capture_delay_v1.csv"
PARENT_CSV = ROOT / "engine/results/ftd_0736/ftd_0736_causal_buffer_relational_formation_v1.csv"

EXPECTED_HASHES = {
    PROTOCOL: "677B054C1C52470F85B272FBD575880274431EB2FF4CEDAB2A4A59C7EAC816C7",
    RUNNER: "BA7141F620044EE2065C4AB2C1B05CCB5A9D57FCBCF0C0C8D7FF090CDADC5D1C",
    RESULT_JSON: "E5622A9C1A4845B08793B6D65D35CDB5BF213115A9E1AE3545B22658D7908CDA",
    RESULT_CSV: "F164E3365BA5A9B434825B371E5B1FF5AFEAD5909C50A539141BFF6125697731",
    PARENT_CSV: "9B0C8296CD0DB4D841C42BB82B32D6AD8245A2DD8489A6D6AA14A11994BA1BDE",
}
EXPECTED_TRANSITIONS = {
    "0_0_1": [7, 26, 63],
    "0_1_-1": [7, 26, 79],
    "1_1_1": [7, 26, 96],
}
EXPECTED_ONSETS = {"0_0_1": 78, "0_1_-1": 94, "1_1_1": 111}


class Certificate:
    def __init__(self) -> None:
        self.checks = 0

    def require(self, condition: bool, message: str) -> None:
        self.checks += 1
        if not condition:
            raise AssertionError(message)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def finite(value: str) -> float:
    result = float(value)
    if not math.isfinite(result):
        raise AssertionError(f"non-finite {value}")
    return result


def flag(value: str) -> bool:
    return value == "1"


def graph_transitions(rows: list[dict[str, str]]) -> list[int]:
    result: list[int] = []
    previous = flag(rows[0]["graph_inside"])
    for row in rows[1:]:
        current = flag(row["graph_inside"])
        if current != previous:
            result.append(int(row["tick"]))
        previous = current
    return result


def onset(rows: list[dict[str, str]], start: int) -> int | None:
    for candidate in range(start, 123):
        tail = [row for row in rows if int(row["tick"]) >= candidate]
        if all(flag(row["graph_inside"])
               and finite(row["pair_energy"]) < -1e-6 for row in tail):
            return candidate
    return None


def main() -> None:
    cert = Certificate()
    for path, expected in EXPECTED_HASHES.items():
        cert.require(path.is_file(), f"missing {path}")
        cert.require(sha256(path) == expected, f"hash {path}")

    data = json.loads(RESULT_JSON.read_text(encoding="utf-8"))
    with RESULT_CSV.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    with PARENT_CSV.open(newline="", encoding="utf-8") as handle:
        parent_rows = list(csv.DictReader(handle))

    cert.require(data["ftd_id"] == "FTD-0737", "id")
    cert.require(data["protocol_sha256"] == EXPECTED_HASHES[PROTOCOL],
                 "protocol link")
    cert.require(data["verdict"] ==
                 "PRECONTACT_DELAYED_ENERGETIC_CAPTURE_CONSTRUCTIVE",
                 "verdict")
    cert.require(data["volume"] == 129, "volume")
    cert.require(data["horizon"] == 122, "horizon")
    cert.require(data["source_radius_cap"] == 3, "source cap")
    cert.require(data["contact_tick"] == 123, "contact")
    cert.require(data["history_count"] == 3, "histories")
    cert.require(data["row_count"] == len(rows) == 735, "row count")

    grouped: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        grouped[row["direction"]].append(row)
    cert.require(set(grouped) == set(EXPECTED_TRANSITIONS), "directions")

    parent = {
        (row["direction"], row["phase"], int(row["tick"])): row
        for row in parent_rows
        if row["family"] == "unbound" and row["polarity"] == "plus_minus"
    }
    maximum_source = 0
    minimum_contact = 129
    for direction, arm_rows in grouped.items():
        forward = sorted(
            (row for row in arm_rows if row["phase"] == "forward"),
            key=lambda row: int(row["tick"]),
        )
        reverse = sorted(
            (row for row in arm_rows if row["phase"] == "reverse"),
            key=lambda row: int(row["tick"]),
        )
        cert.require([int(row["tick"]) for row in forward]
                     == list(range(123)), f"forward ticks {direction}")
        cert.require([int(row["tick"]) for row in reverse]
                     == list(range(1, 123)), f"reverse ticks {direction}")
        for row in arm_rows:
            cert.require(flag(row["valid"]), f"valid {direction}")
            cert.require(flag(row["common"]), f"common {direction}")
            cert.require(finite(row["max_residual"]) <= 1e-10,
                         f"residual {direction}")
            cert.require(finite(row["total_energy_residual"]) <= 1e-10,
                         f"energy {direction}")
            cert.require(finite(row["recoil_defect"]) <= 1e-9,
                         f"recoil {direction}")
            cert.require(finite(row["causal_speed_excess"]) <= 1e-12,
                         f"speed {direction}")
            source = int(row["source_radius"])
            cert.require(source <= 3, f"source {direction}")
            maximum_source = max(maximum_source, source)
        arm_source = max(int(row["source_radius"]) for row in arm_rows)
        minimum_contact = min(minimum_contact, 129-2*arm_source)

        cert.require(not flag(forward[0]["graph_inside"]),
                     f"initial graph {direction}")
        cert.require(finite(forward[0]["pair_energy"]) > 1e-6,
                     f"initial energy {direction}")
        transition_ticks = graph_transitions(forward)
        cert.require(transition_ticks == EXPECTED_TRANSITIONS[direction],
                     f"transitions {direction}")
        found_onset = onset(forward, transition_ticks[2])
        cert.require(found_onset == EXPECTED_ONSETS[direction],
                     f"onset {direction}")
        cert.require(found_onset-transition_ticks[2] == 15,
                     f"delay {direction}")
        cert.require(all(flag(row["graph_inside"])
                         and finite(row["pair_energy"]) < -1e-6
                         for row in forward
                         if int(row["tick"]) >= found_onset),
                     f"tail {direction}")
        cert.require(finite(forward[-1]["field_energy"])
                     - finite(forward[0]["field_energy"]) > 1e-6,
                     f"field receiver {direction}")

        for row in forward:
            tick = int(row["tick"])
            if tick > 112:
                continue
            reference = parent[(direction, "forward", tick)]
            for column in (
                "valid", "common", "max_residual", "total_energy_residual",
                "recoil_defect", "causal_speed_excess", "source_radius",
                "source_entries", "separation", "pair_energy", "field_energy",
                "graph_inside",
            ):
                cert.require(row[column] == reference[column],
                             f"parent prefix {direction} {tick} {column}")

    cert.require(data["delay_passes"] == 3, "delay aggregate")
    cert.require(data["tail_passes"] == 3, "tail aggregate")
    cert.require(data["receiver_passes"] == 3, "receiver aggregate")
    cert.require(data["maximum_source_radius"] == maximum_source == 3,
                 "source aggregate")
    cert.require(data["minimum_measured_contact_tick"]
                 == minimum_contact == 123, "contact aggregate")
    cert.require(data["maximum_common_residual"] <= 1e-10,
                 "common aggregate")
    cert.require(data["maximum_energy_residual"] <= 1e-10,
                 "energy aggregate")
    cert.require(data["maximum_recoil_defect"] <= 1e-9,
                 "recoil aggregate")
    cert.require(data["maximum_speed_excess"] <= 1e-12,
                 "speed aggregate")
    cert.require(data["maximum_inverse_recovery"] <= 1e-8,
                 "inverse aggregate")
    cert.require(data["maximum_pair_field_balance"] <= 1e-8,
                 "balance aggregate")

    print(f"FTD-0737 certificate: {cert.checks}/{cert.checks} checks PASS")
    print(f"verdict={data['verdict']}")
    print("onsets=" + json.dumps(EXPECTED_ONSETS, sort_keys=True)
          + " delay=15 contact=123 horizon=122")
    print(f"source_radius={maximum_source} "
          f"inverse={data['maximum_inverse_recovery']:.9g}")


if __name__ == "__main__":
    main()
