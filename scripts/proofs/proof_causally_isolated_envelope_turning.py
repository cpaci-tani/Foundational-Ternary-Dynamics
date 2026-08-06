#!/usr/bin/env python3
"""Independent certificate for the locked FTD-0670 verdict."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CAUSALLY_ISOLATED_ENVELOPE_TURNING_v1.md"
RUNNER = ROOT / "engine/tests/test_causally_isolated_envelope_turning.cpp"
RESULT = ROOT / "engine/results/ftd_0670/ftd_0670_causally_isolated_envelope_turning_v1.json"
TICKS = ROOT / "engine/results/ftd_0670/ftd_0670_causally_isolated_envelope_turning_ticks_v1.csv"
PARENT_JSON = ROOT / "engine/results/ftd_0668/ftd_0668_causally_isolated_internal_recurrence_v1.json"
PARENT_TICKS = ROOT / "engine/results/ftd_0668/ftd_0668_causally_isolated_internal_recurrence_ticks_v1.csv"

EXPECTED = {
    PROTOCOL: "92B98E746C02BAA980A43AF8C9E84B8CF6B5DC8161968511DBF14365D8237412",
    RUNNER: "982ABC83170B77660D8002B34F8037BF84BECAA730166F95D10623EED15DBCD0",
    RESULT: "631BFCD005E5B223641260F8D1A59442EAFDFCF88565B8EEDBEAC8E4F228DC10",
    TICKS: "8C3CBCDAC9137114B2A17202FA04FF77362465D13BE94636C19493A1A31F347A",
    PARENT_JSON: "D1EF53978C9B04F9EEC2FF34954D7D04CA9163AAE6FAD6833D7CCF352CEAE0D2",
    PARENT_TICKS: "E34AC8AAE7FC703B037D9F1B730A2A97213419A9A5D01996D5C9716999256FDB",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> None:
    for path, expected in EXPECTED.items():
        require(path.is_file(), f"missing artifact: {path}")
        require(sha256(path) == expected, f"hash mismatch: {path}")

    result = json.loads(RESULT.read_text(encoding="utf-8"))
    require(result["ftd_id"] == "FTD-0670", "wrong id")
    require(result["protocol_sha256"] == EXPECTED[PROTOCOL], "wrong protocol")
    require(result["parent_json_sha256"] == EXPECTED[PARENT_JSON], "wrong parent JSON")
    require(result["parent_csv_sha256"] == EXPECTED[PARENT_TICKS], "wrong parent CSV")
    require(result["verdict"] == "CAUSALLY_ISOLATED_ENVELOPE_TURNING_CONSTRUCTIVE", "wrong verdict")
    require(result["production_changed"] is False, "production changed")
    require(result["volume"] == 97 and result["horizon"] == 80, "wrong grid")
    require(result["maximum_constituent_momentum_amplitude"] == 4e-6, "wrong amplitude")
    require(result["causal_contact_tick"] == 81, "wrong contact tick")
    require(result["horizon"] < result["causal_contact_tick"], "not pre-contact")
    require(result["initial_fields_bitwise_equal"] is True, "initial fields differ")
    require(result["negative_executed"] and result["positive_executed"], "execution failed")
    require(result["negative_recovery"] <= 1e-8, "negative inverse")
    require(result["positive_recovery"] <= 1e-8, "positive inverse")

    with TICKS.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    require(len(rows) == 162, "wrong row count")
    by_sign: dict[int, list[dict[str, str]]] = {-1: [], 1: []}
    for row in rows:
        require(row["ftd_id"] == "FTD-0670", "row id")
        sign = int(row["sign"])
        require(sign in by_sign, "row sign")
        by_sign[sign].append(row)
        require(float(row["energy_drift"]) <= 1e-10, "energy drift")
        require(float(row["common_residual"]) <= 1e-10, "action residual")
        require(int(row["source_support_radius"]) <= 8, "source escaped")

    summaries: dict[int, tuple[int, float, float]] = {}
    for sign, arm in by_sign.items():
        arm.sort(key=lambda row: int(row["tick"]))
        require([int(row["tick"]) for row in arm] == list(range(81)), "tick schema")
        troughs: list[tuple[int, float]] = []
        for i in range(1, len(arm) - 1):
            tick = int(arm[i]["tick"])
            value = float(arm[i]["doublet_ratio"])
            if 60 <= tick <= 79 and value < float(arm[i - 1]["doublet_ratio"]) and value < float(arm[i + 1]["doublet_ratio"]):
                troughs.append((tick, value))
        require(len(troughs) >= 6, "insufficient troughs")
        primary_tick, primary = min(troughs, key=lambda pair: (pair[1], pair[0]))
        require(71 <= primary_tick <= 73, "primary outside window")
        before = [pair for pair in troughs if pair[0] < primary_tick]
        after = [pair for pair in troughs if pair[0] > primary_tick]
        require(len(before) >= 3 and len(after) >= 2, "insufficient sides")
        descending = [pair[1] for pair in before[-3:]] + [primary]
        require(all(a > b for a, b in zip(descending, descending[1:])), "not descending")
        ascending = [primary, after[0][1], after[1][1]]
        require(all(a < b for a, b in zip(ascending, ascending[1:])), "not ascending")
        increment = after[1][1] - primary
        require(increment >= 0.05, "insufficient recovery")
        final = arm[-1]
        require(float(final["positive_field_norm_ratio"]) > 0.0, "zero field")
        require(float(final["near_fraction"]) < 0.40, "field too near")
        require(float(final["radius_second_moment"]) > 300.0, "field not spread")
        summaries[sign] = (primary_tick, primary, increment)

    negative, positive = summaries[-1], summaries[1]
    require(abs(negative[0] - positive[0]) <= 1, "tick polarity mismatch")
    require(abs(negative[1] - positive[1]) <= 1e-4, "trough polarity mismatch")
    require(abs(negative[2] - positive[2]) <= 1e-4, "recovery polarity mismatch")
    require(result["negative_primary_tick"] == negative[0], "negative JSON tick")
    require(result["positive_primary_tick"] == positive[0], "positive JSON tick")
    require(abs(result["negative_primary_ratio"] - negative[1]) <= 1e-15, "negative JSON ratio")
    require(abs(result["positive_primary_ratio"] - positive[1]) <= 1e-15, "positive JSON ratio")
    require(abs(result["negative_recovery_increment"] - negative[2]) <= 1e-15, "negative JSON increment")
    require(abs(result["positive_recovery_increment"] - positive[2]) <= 1e-15, "positive JSON increment")

    print(
        "FTD-0670 causal-envelope certificate: PASS "
        "(held-out half-amplitude turning CONSTRUCTIVE at tick 72)"
    )


if __name__ == "__main__":
    main()
