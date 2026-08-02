#!/usr/bin/env python3
"""Independent certificate for the locked FTD-0672 regional-flow verdict."""

from __future__ import annotations

import csv
import hashlib
import json
from collections import defaultdict
from decimal import Decimal, getcontext
from pathlib import Path

getcontext().prec = 50
D = Decimal

ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CAUSAL_REGIONAL_FIELD_FLOW_v1.md"
RUNNER = ROOT / "engine/tests/test_causal_regional_field_flow.cpp"
RESULT = ROOT / "engine/results/ftd_0672/ftd_0672_causal_regional_field_flow_v1.json"
TICKS = ROOT / "engine/results/ftd_0672/ftd_0672_causal_regional_field_flow_ticks_v1.csv"
PARENT_JSON = ROOT / "engine/results/ftd_0670/ftd_0670_causally_isolated_envelope_turning_v1.json"
PARENT_CSV = ROOT / "engine/results/ftd_0670/ftd_0670_causally_isolated_envelope_turning_ticks_v1.csv"

EXPECTED = {
    PREREG: "F0A2F895C07ADD99FC0BF4E39B95CD2FCEEE4BEBC10A4EE16CE4E47324B9C971",
    RUNNER: "77C4DBAB010A3ED599AAB0109E9D6AB4EAB590D63560A90900A8BC4446944843",
    RESULT: "E3EFB78EC36F32FEFE7627A3EE368E2A5A700BCE0890FBEF1E27D2D8E9B414D3",
    TICKS: "C4339D5985F4EB36DFE2F0DDF28A4151C805D4EC5913ED08EAF1A601F84F5C8E",
    PARENT_JSON: "631BFCD005E5B223641260F8D1A59442EAFDFCF88565B8EEDBEAC8E4F228DC10",
    PARENT_CSV: "8C3CBCDAC9137114B2A17202FA04FF77362465D13BE94636C19493A1A31F347A",
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def close(left: D, right: D, tolerance: D = D("1e-12")) -> bool:
    return abs(left - right) <= tolerance


def classify_turn(ticks: dict[int, dict[str, str]]) -> tuple[bool, int, D, D]:
    troughs = [
        tick for tick in range(60, 80)
        if D(ticks[tick]["doublet_ratio"])
        < D(ticks[tick - 1]["doublet_ratio"])
        and D(ticks[tick]["doublet_ratio"])
        < D(ticks[tick + 1]["doublet_ratio"])
    ]
    primary = min(troughs, key=lambda tick: (D(ticks[tick]["doublet_ratio"]), tick))
    before = [tick for tick in troughs if tick < primary]
    after = [tick for tick in troughs if tick > primary]
    require(len(before) >= 3 and len(after) >= 2, "trough count")
    before = before[-3:]
    after = after[:2]
    primary_ratio = D(ticks[primary]["doublet_ratio"])
    descending = (
        D(ticks[before[0]]["doublet_ratio"])
        > D(ticks[before[1]]["doublet_ratio"])
        > D(ticks[before[2]]["doublet_ratio"])
        > primary_ratio
    )
    ascending = (
        primary_ratio
        < D(ticks[after[0]]["doublet_ratio"])
        < D(ticks[after[1]]["doublet_ratio"])
    )
    recovery = D(ticks[after[1]]["doublet_ratio"]) - primary_ratio
    return 71 <= primary <= 73 and descending and ascending and recovery >= D("0.05"), primary, primary_ratio, recovery


def main() -> None:
    for path, expected in EXPECTED.items():
        require(path.is_file(), f"missing {path}")
        require(sha256(path) == expected, f"hash {path}")

    result = json.loads(RESULT.read_text(encoding="utf-8"))
    require(result["ftd_id"] == "FTD-0672", "id")
    require(result["protocol_sha256"] == EXPECTED[PREREG], "protocol")
    require(result["volume"] == 97 and result["horizon"] == 80, "geometry")
    require(result["causal_contact_tick"] == 81, "contact tick")
    require(result["initial_fields_bitwise_equal"], "initial fields")
    require(result["polarity_consistent"], "polarity")
    require(result["schema_complete"], "schema")

    with TICKS.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    require(len(rows) == 486, "row count")
    by_sign_radius: dict[tuple[int, int], list[dict[str, str]]] = defaultdict(list)
    by_sign_tick: dict[tuple[int, int], dict[str, str]] = {}
    for row in rows:
        require(row["ftd_id"] == "FTD-0672", "row id")
        require(row["protocol_sha256"] == EXPECTED[PREREG], "row protocol")
        sign = int(row["sign"])
        tick = int(row["tick"])
        radius = int(D(row["radius"]))
        require(sign in (-1, 1) and 0 <= tick <= 80, "row coordinates")
        require(radius in (8, 16, 24), "row radius")
        require(row["observer_valid"] == "1", "observer validity")
        require(D(row["update_residual"]) <= D("1e-10"), "update residual")
        require(D(row["partition_residual"]) <= D("1e-10"), "partition residual")
        require(D(row["ledger_residual"]) <= D("1e-10"), "ledger residual")
        require(D(row["energy_drift"]) <= D("1e-10"), "energy drift")
        require(D(row["common_residual"]) <= D("1e-10"), "action residual")
        require(int(row["source_support_radius"]) <= 8, "source support")
        ledger = (
            D(row["regional_energy_after"])
            - D(row["regional_energy_before"])
            - D(row["boundary_transport_into"])
            - D(row["source_exchange_into_field"])
        )
        require(abs(ledger) <= D("2e-12"), "normalized regional ledger")
        by_sign_radius[(sign, radius)].append(row)
        by_sign_tick.setdefault((sign, tick), row)

    summaries = {}
    for sign, prefix in ((-1, "negative"), (1, "positive")):
        ticks = {tick: by_sign_tick[(sign, tick)] for tick in range(81)}
        turning, primary, primary_ratio, recovery = classify_turn(ticks)
        require(turning and result[f"{prefix}_turning"], "turning")
        require(primary == result[f"{prefix}_primary_tick"], "primary tick")
        require(close(primary_ratio, D(str(result[f"{prefix}_primary_ratio"]))), "primary ratio")
        require(close(recovery, D(str(result[f"{prefix}_recovery_increment"]))), "recovery")

        outward = []
        inward = []
        net = []
        exchange = []
        for radius in (8, 16, 24):
            arm_rows = sorted(by_sign_radius[(sign, radius)], key=lambda row: int(row["tick"]))
            require([int(row["tick"]) for row in arm_rows] == list(range(81)), "tick coverage")
            outward.append(sum(
                max(-D(row["boundary_transport_into"]), D(0))
                for row in arm_rows if 1 <= int(row["tick"]) <= 67
            ))
            inward.append(sum(
                max(D(row["boundary_transport_into"]), D(0))
                for row in arm_rows if 68 <= int(row["tick"]) <= 80
            ))
            net.append(sum(
                -D(row["boundary_transport_into"])
                for row in arm_rows if 1 <= int(row["tick"]) <= 80
            ))
            exchange.append(sum(
                D(row["source_exchange_into_field"])
                for row in arm_rows if 68 <= int(row["tick"]) <= 80
            ))
            require(close(outward[-1], D(str(result[f"{prefix}_outward_before_68_r{radius}"]))), "outward")
            require(close(inward[-1], D(str(result[f"{prefix}_inward_68_80_r{radius}"]))), "inward")
            require(close(net[-1], D(str(result[f"{prefix}_net_outward_r{radius}"]))), "net")
            require(close(exchange[-1], D(str(result[f"{prefix}_exchange_68_80_r{radius}"]))), "exchange")

        require(max(exchange) - min(exchange) <= D("1e-10"), "source exchange locality")
        require(all(value == 0 for value in inward), "recovery-window inward flow")
        require(outward[2] < D("0.05"), "locked substantial pre-68 gate")
        final_near = D(ticks[80]["near_fraction"])
        require(final_near < D("0.50"), "near-bound gate")
        require(result[f"{prefix}_transport_class"] == "REGIONAL_FLOW_MIXED", "transport class")
        require(exchange[0] >= D("0.01"), "positive current-to-field exchange")
        require(result[f"{prefix}_exchange_class"] == "CURRENT_TO_DYNAMIC_FIELD", "exchange class")
        late_outward_24 = net[2] - outward[2]
        require(late_outward_24 > D(0), "late outward transport")
        require(late_outward_24 / exchange[2] > D("0.97"), "late outgoing fraction")
        summaries[sign] = (outward, inward, net, exchange, late_outward_24)

    for metric in range(5):
        if metric < 4:
            for radius in range(3):
                require(abs(summaries[-1][metric][radius] - summaries[1][metric][radius]) <= D("1e-4"), "polarity metric")
        else:
            require(abs(summaries[-1][metric] - summaries[1][metric]) <= D("1e-4"), "polarity late flow")

    require(result["verdict"] == "CAUSAL_REGIONAL_FIELD_FLOW_MIXED", "verdict")
    print(
        "FTD-0672 causal regional-flow certificate: PASS "
        "rows=486 inward_68_80=0 verdict=CAUSAL_REGIONAL_FIELD_FLOW_MIXED "
        f"late_outward_fraction={summaries[1][4] / summaries[1][3][2]:.12f}"
    )


if __name__ == "__main__":
    main()
