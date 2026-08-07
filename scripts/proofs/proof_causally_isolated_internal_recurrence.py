#!/usr/bin/env python3
"""Independent certificate for FTD-0668.

This certifies the locked mixed verdict and run integrity.  The late turning
point and outward-field summaries are descriptive only because they were not
the registered verdict predicates.
"""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_CAUSALLY_ISOLATED_INTERNAL_RECURRENCE_v1.md"
RUNNER = ROOT / "engine/tests/test_causally_isolated_internal_recurrence.cpp"
RESULT = ROOT / "engine/results/ftd_0668/ftd_0668_causally_isolated_internal_recurrence_v1.json"
TICKS = ROOT / "engine/results/ftd_0668/ftd_0668_causally_isolated_internal_recurrence_ticks_v1.csv"

EXPECTED = {
    PROTOCOL: "FD959EADB5B50D237D78929295A45BC507DE37843DECA151705856F2359FA70C",
    RUNNER: "92314D0F21BE50365E5BC5198912D1470EC334393651FB889F7CCA6EBF595870",
    RESULT: "D1EF53978C9B04F9EEC2FF34954D7D04CA9163AAE6FAD6833D7CCF352CEAE0D2",
    TICKS: "E34AC8AAE7FC703B037D9F1B730A2A97213419A9A5D01996D5C9716999256FDB",
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
    require(result["ftd_id"] == "FTD-0668", "wrong ledger id")
    require(result["protocol_sha256"] == EXPECTED[PROTOCOL], "wrong protocol")
    require(result["verdict"] == "CAUSALLY_ISOLATED_INTERNAL_RECURRENCE_MIXED", "wrong verdict")
    require(result["production_changed"] is False, "production changed")
    require(result["volume"] == 97 and result["horizon"] == 80, "wrong grid")
    require(result["causal_contact_tick"] == 81, "wrong contact time")
    require(result["horizon"] < result["causal_contact_tick"], "not causal")
    require(result["initial_fields_bitwise_equal"] is True, "initial field mismatch")
    require(result["dense_sparse_equivalent"] is True, "storage inequivalence")
    require(result["dense_sparse_state_difference"] <= 1e-10, "storage difference")
    require(result["negative_executed"] and result["positive_executed"], "execution")
    require(result["negative_return_tick"] == -1, "negative returned")
    require(result["positive_return_tick"] == -1, "positive returned")
    require(result["maximum_source_radius"] <= result["source_radius_limit"], "source escaped")
    require(result["negative_recovery"] <= 1e-8, "negative recovery")
    require(result["positive_recovery"] <= 1e-8, "positive recovery")

    with TICKS.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    require(len(rows) == 162, "wrong row count")
    required = {
        "ftd_id", "sign", "tick", "doublet_ratio", "field_energy_ratio",
        "positive_field_norm_ratio", "near_fraction", "radius_second_moment",
        "dynamic_support_radius", "source_support_radius", "energy_drift",
        "common_residual",
    }
    require(set(rows[0]) == required, "wrong schema")

    by_sign: dict[int, list[dict[str, str]]] = {-1: [], 1: []}
    for row in rows:
        require(row["ftd_id"] == "FTD-0668", "row id")
        sign = int(row["sign"])
        require(sign in by_sign, "row sign")
        by_sign[sign].append(row)
        require(float(row["energy_drift"]) <= 1e-10, "energy drift")
        require(float(row["common_residual"]) <= 1e-10, "action residual")
        require(int(row["source_support_radius"]) <= 8, "source support")

    for sign, arm in by_sign.items():
        require([int(row["tick"]) for row in arm] == list(range(81)), f"ticks {sign}")
        require(abs(float(arm[0]["doublet_ratio"]) - 1.0) <= 1e-12, "normalization")
        require(abs(float(arm[0]["field_energy_ratio"])) <= 1e-15, "initial field energy")
        minimum = min((float(row["doublet_ratio"]), int(row["tick"])) for row in arm)
        maximum = max((float(row["doublet_ratio"]), int(row["tick"])) for row in arm)
        require(minimum[1] == 72 and minimum[0] > 0.60, f"minimum {sign}")
        require(maximum[1] == 10 and maximum[0] > 3.60, f"maximum {sign}")
        require(float(arm[-1]["field_energy_ratio"]) > 0.39, "late field energy")
        require(float(arm[-1]["positive_field_norm_ratio"]) > 0.55, "late norm")
        require(float(arm[-1]["near_fraction"]) < 0.24, "late near fraction")
        require(float(arm[-1]["radius_second_moment"]) > 490.0, "late spread")

    for negative, positive in zip(by_sign[-1], by_sign[1]):
        for key in (
            "doublet_ratio", "field_energy_ratio", "positive_field_norm_ratio",
            "near_fraction", "radius_second_moment",
        ):
            require(abs(float(negative[key]) - float(positive[key])) <= 1e-5,
                    f"polarity mismatch: {key}")

    print(
        "FTD-0668 causal-buffer certificate: PASS "
        "(executable MIXED; no locked threshold return; descriptive minimum tick 72)"
    )


if __name__ == "__main__":
    main()
