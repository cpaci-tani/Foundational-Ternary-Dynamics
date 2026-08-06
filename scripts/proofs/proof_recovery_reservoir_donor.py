#!/usr/bin/env python3
"""Independent certificate for the locked FTD-0674 invalid verdict."""

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_RECOVERY_RESERVOIR_DONOR_v1.md"
RUNNER = ROOT / "engine/tests/test_recovery_reservoir_donor.cpp"
RESULT = ROOT / "engine/results/ftd_0674/ftd_0674_recovery_reservoir_donor_v1.json"
TICKS = ROOT / "engine/results/ftd_0674/ftd_0674_recovery_reservoir_donor_ticks_v1.csv"

EXPECTED = {
    PROTOCOL: "EC89065A9996C233978E164533D878200275B203646921222500928062C60383",
    RUNNER: "6593028B7137CD35EA7E6EECC301C1E1EFF3ADF3110527B9837B463B35CE0638",
    RESULT: "1848283E5AF91B076E7DD69CB24B4677159FED8594F2C78A5D8D858F441044CB",
    TICKS: "DEA0582DD2E135071524CBAB6F532A74FCCEE49D2F88E163966E6CC6DE4364E9",
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> None:
    for path, expected in EXPECTED.items():
        require(digest(path) == expected, f"hash drift: {path}")

    result = json.loads(RESULT.read_text(encoding="utf-8"))
    require(result["ftd_id"] == "FTD-0674", "wrong id")
    require(result["protocol_sha256"] == EXPECTED[PROTOCOL], "protocol")
    require(result["verdict"] == "RECOVERY_RESERVOIR_DONOR_EXECUTION_INVALID",
            "verdict")
    require(result["production_changed"] is False, "production")
    require(result["schema_complete"] is True, "schema")
    require(result["negative_executed"] is False, "negative invalid")
    require(result["positive_executed"] is False, "positive invalid")

    rows = list(csv.DictReader(TICKS.open(newline="", encoding="utf-8")))
    require(len(rows) == 162, "row count")
    indexed = {(int(row["sign"]), int(row["tick"])): row for row in rows}
    require(len(indexed) == 162, "unique sign/tick")
    for sign in (-1, 1):
        for tick in range(81):
            row = indexed[(sign, tick)]
            require(row["ftd_id"] == "FTD-0674", "row id")
            require(row["protocol_sha256"] == EXPECTED[PROTOCOL], "row protocol")
            require(int(row["observer_valid"]) == 1, "observer validity")

        start = indexed[(sign, 72)]
        end = indexed[(sign, 78)]
        fields = ["target", "other", "nonlinear", "dynamic_field",
                  "field_interference"]
        delta = {field: float(end[field]) - float(start[field])
                 for field in fields}
        require(delta["target"] < 0.0, "no locked recovery")
        require(delta["target"] < 0.05, "recovery gate fails")
        closure = abs(sum(delta.values()))
        require(closure > 1e-8, "interval closure gate also fails")
        require(max(float(indexed[(sign, tick)]["observer_residual"])
                    for tick in range(81)) <= 1e-8,
                "per-tick observer")

    mismatch = abs(
        (float(indexed[(-1, 78)]["target"])
         - float(indexed[(-1, 72)]["target"]))
        - (float(indexed[(1, 78)]["target"])
           - float(indexed[(1, 72)]["target"]))
    )
    require(mismatch <= 1e-8, "polarity decline mismatch")
    print(
        "FTD-0674 recovery-reservoir certificate: PASS "
        f"rows={len(rows)} target_change_mismatch={mismatch:.3e} "
        "verdict=EXECUTION_INVALID_NO_LOCKED_RECOVERY"
    )


if __name__ == "__main__":
    main()
