#!/usr/bin/env python3
"""Independent certificate for FTD-0736 causal-buffer formation."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CAUSAL_BUFFER_RELATIONAL_FORMATION_v1.md"
RUNNER = ROOT / "engine/tests/test_causal_buffer_relational_formation.cpp"
RESULT_JSON = ROOT / "engine/results/ftd_0736/ftd_0736_causal_buffer_relational_formation_v1.json"
RESULT_CSV = ROOT / "engine/results/ftd_0736/ftd_0736_causal_buffer_relational_formation_v1.csv"

EXPECTED_HASHES = {
    PROTOCOL: "955FC3331A64B6DB7C495AE6ACFFE82DBE9ADE42DE730B68A7E2610F885EFFAB",
    RUNNER: "B01CFCB2309B28404D93949C9072B43E423F2E4DE50E6AF091B75D451FE8931F",
    RESULT_JSON: "E6C8ECBCFAFC2755D4C5D07D846CA18FB63388B92438165AC8C9DE50AD41B53F",
    RESULT_CSV: "9B0C8296CD0DB4D841C42BB82B32D6AD8245A2DD8489A6D6AA14A11994BA1BDE",
}

EXPECTED_ARMS = {
    ("bound", "0_0_1", "plus_minus"),
    ("unbound", "0_0_1", "plus_minus"),
    ("unbound", "0_1_-1", "plus_minus"),
    ("unbound", "1_1_1", "minus_plus"),
    ("unbound", "1_1_1", "plus_minus"),
}
EXPECTED_TRANSITIONS = {
    "0_0_1": [7, 26, 63],
    "0_1_-1": [7, 26, 79],
    "1_1_1": [7, 26, 96],
}


class Certificate:
    def __init__(self) -> None:
        self.checks = 0

    def require(self, condition: bool, message: str) -> None:
        self.checks += 1
        if not condition:
            raise AssertionError(message)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def flag(value: str) -> bool:
    return value == "1"


def f(value: str) -> float:
    result = float(value)
    if not math.isfinite(result):
        raise AssertionError(f"non-finite value: {value}")
    return result


def transitions(rows: list[dict[str, str]]) -> list[int]:
    forward = sorted(
        (row for row in rows if row["phase"] == "forward"),
        key=lambda row: int(row["tick"]),
    )
    result: list[int] = []
    previous = flag(forward[0]["graph_inside"])
    for row in forward[1:]:
        current = flag(row["graph_inside"])
        if current != previous:
            result.append(int(row["tick"]))
        previous = current
    return result


def durable_negative_onset(
    rows: list[dict[str, str]], start: int
) -> int | None:
    forward = sorted(
        (row for row in rows if row["phase"] == "forward"),
        key=lambda row: int(row["tick"]),
    )
    for row in forward:
        tick = int(row["tick"])
        if tick < start or f(row["pair_energy"]) >= -1e-6:
            continue
        tail = [item for item in forward if int(item["tick"]) >= tick]
        if all(flag(item["graph_inside"])
               and f(item["pair_energy"]) < -1e-6 for item in tail):
            return tick
    return None


def main() -> None:
    cert = Certificate()
    for path, expected in EXPECTED_HASHES.items():
        cert.require(path.is_file(), f"missing {path}")
        cert.require(sha256(path) == expected, f"hash mismatch {path}")

    data = json.loads(RESULT_JSON.read_text(encoding="utf-8"))
    with RESULT_CSV.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    cert.require(data["ftd_id"] == "FTD-0736", "wrong id")
    cert.require(data["protocol_sha256"] == EXPECTED_HASHES[PROTOCOL],
                 "protocol link mismatch")
    cert.require(data["verdict"] ==
                 "PRECONTACT_REENTRY_WITHOUT_PERSISTENT_CORE",
                 "wrong verdict")
    cert.require(data["volume"] == 129, "wrong volume")
    cert.require(data["horizon"] == 112, "wrong horizon")
    cert.require(data["source_radius_cap"] == 8, "wrong support cap")
    cert.require(data["locked_contact_tick"] == 113,
                 "wrong locked contact tick")
    cert.require(data["history_count"] == 5, "wrong history count")
    cert.require(data["step_row_count"] == 1125, "wrong JSON row count")
    cert.require(len(rows) == 1125, "wrong CSV row count")

    grouped: dict[tuple[str, str, str], list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        grouped[(row["family"], row["direction"], row["polarity"])].append(row)
    cert.require(set(grouped) == EXPECTED_ARMS, "wrong arm matrix")

    formed = persistent = receivers = controls = 0
    maximum_source = 0
    minimum_contact = 129
    posthoc_onsets: dict[str, int] = {}
    for key, arm_rows in grouped.items():
        family, direction, polarity = key
        forward = sorted(
            (row for row in arm_rows if row["phase"] == "forward"),
            key=lambda row: int(row["tick"]),
        )
        reverse = sorted(
            (row for row in arm_rows if row["phase"] == "reverse"),
            key=lambda row: int(row["tick"]),
        )
        cert.require([int(row["tick"]) for row in forward]
                     == list(range(113)), f"forward ticks {key}")
        cert.require([int(row["tick"]) for row in reverse]
                     == list(range(1, 113)), f"reverse ticks {key}")
        for row in arm_rows:
            cert.require(flag(row["valid"]), f"invalid step {key}")
            cert.require(flag(row["common"]), f"common gate {key}")
            cert.require(f(row["max_residual"]) <= 1e-10,
                         f"action residual {key}")
            cert.require(f(row["total_energy_residual"]) <= 1e-10,
                         f"energy residual {key}")
            cert.require(f(row["recoil_defect"]) <= 1e-9,
                         f"recoil {key}")
            cert.require(f(row["causal_speed_excess"]) <= 1e-12,
                         f"speed {key}")
            source = int(row["source_radius"])
            cert.require(source <= 8, f"source support {key}")
            maximum_source = max(maximum_source, source)
        minimum_contact = min(minimum_contact, 129 - 2*max(
            int(row["source_radius"]) for row in arm_rows))

        arm_transitions = transitions(arm_rows)
        if family == "bound":
            cert.require(arm_transitions == [], "bound transition")
            cert.require(all(flag(row["graph_inside"])
                             and f(row["pair_energy"]) < -1e-6
                             for row in forward), "bound control released")
            controls += 1
            continue

        cert.require(not flag(forward[0]["graph_inside"]),
                     f"unbound graph initial {key}")
        cert.require(f(forward[0]["pair_energy"]) > 1e-6,
                     f"unbound energy initial {key}")
        cert.require(arm_transitions == EXPECTED_TRANSITIONS[direction],
                     f"transition sequence {key}")
        formed += 1

        third = arm_transitions[2]
        preregistered_tail = [
            row for row in forward if int(row["tick"]) >= third
        ]
        preregistered_persistent = all(
            flag(row["graph_inside"])
            and f(row["pair_energy"]) < -1e-6
            for row in preregistered_tail
        )
        cert.require(not preregistered_persistent,
                     f"negative verdict not reproduced {key}")
        persistent += int(preregistered_persistent)

        morphology = [row for row in forward
                      if flag(row["morphology_measured"])]
        cert.require([int(row["tick"]) for row in morphology]
                     == [48, 96, 112], f"morphology ticks {key}")
        cert.require(all(flag(row["morphology_valid"])
                         for row in morphology), f"morphology validity {key}")
        receiver = (f(forward[-1]["field_energy"])
                    - f(forward[0]["field_energy"]) > 1e-6
                    and any(f(row["dynamic_norm"]) > 1e-8
                            and f(row["magnetic_energy"]) > 1e-10
                            and int(row["doubled_median_radius"]) >= 5
                            for row in morphology))
        cert.require(receiver, f"receiver gate {key}")
        receivers += 1

        onset = durable_negative_onset(arm_rows, third)
        cert.require(onset is not None, f"missing post-hoc onset {key}")
        posthoc_onsets[f"{direction}:{polarity}"] = int(onset)

    cert.require(formed == data["unbound_formed"] == 4, "formed aggregate")
    cert.require(persistent == data["unbound_persistent"] == 0,
                 "persistent aggregate")
    cert.require(receivers == data["unbound_receiver"] == 4,
                 "receiver aggregate")
    cert.require(controls == data["bound_controls"] == 1,
                 "control aggregate")
    cert.require(maximum_source == data["maximum_source_radius"] == 3,
                 "source aggregate")
    cert.require(minimum_contact == data["minimum_measured_contact_tick"]
                 == 123, "contact aggregate")
    cert.require(data["polarity_scalar_difference"] == 0,
                 "polarity mismatch")
    cert.require(data["maximum_common_residual"] <= 1e-10,
                 "JSON common residual")
    cert.require(data["maximum_total_energy_residual"] <= 1e-10,
                 "JSON energy residual")
    cert.require(data["maximum_recoil_defect"] <= 1e-9,
                 "JSON recoil")
    cert.require(data["maximum_causal_speed_excess"] <= 1e-12,
                 "JSON speed")
    cert.require(data["maximum_inverse_recovery"] <= 1e-8,
                 "JSON inverse")
    cert.require(data["maximum_pair_field_balance"] <= 1e-8,
                 "JSON balance")

    delays = {
        key: posthoc_onsets[key] - EXPECTED_TRANSITIONS[key.split(":")[0]][2]
        for key in posthoc_onsets
    }
    print(f"FTD-0736 certificate: {cert.checks}/{cert.checks} checks PASS")
    print(f"verdict={data['verdict']}")
    print(f"formed={formed}/4 persistent_from_reentry={persistent}/4 "
          f"receiver={receivers}/4 source_radius={maximum_source} "
          f"contact_tick={minimum_contact}")
    print("posthoc_durable_negative_onsets=" + json.dumps(
        posthoc_onsets, sort_keys=True))
    print("posthoc_reentry_to_negative_delays=" + json.dumps(
        delays, sort_keys=True))


if __name__ == "__main__":
    main()
