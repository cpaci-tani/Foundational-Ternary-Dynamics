#!/usr/bin/env python3
"""Independent certificate for the locked FTD-0676 decay verdict."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CANONICAL_PRECONTACT_MODE_DECAY_v1.md"
PARENT_JSON = ROOT / "engine/results/ftd_0674/ftd_0674_recovery_reservoir_donor_v1.json"
PARENT_CSV = ROOT / "engine/results/ftd_0674/ftd_0674_recovery_reservoir_donor_ticks_v1.csv"
RESULT_JSON = ROOT / "engine/results/ftd_0676/ftd_0676_canonical_precontact_mode_decay_v1.json"
RESULT_CSV = ROOT / "engine/results/ftd_0676/ftd_0676_canonical_precontact_mode_decay_ticks_v1.csv"

PROTOCOL = "1DCD7CEB1FCF429FDF63CE7251D713C76D9E5B9F80DBD75B4D061E715564A6B6"
PARENT_JSON_SHA = "1848283E5AF91B076E7DD69CB24B4677159FED8594F2C78A5D8D858F441044CB"
PARENT_CSV_SHA = "DEA0582DD2E135071524CBAB6F532A74FCCEE49D2F88E163966E6CC6DE4364E9"
PARENT_RATE = 0.006537123419844565


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def close(actual: float, expected: float, tolerance: float, label: str) -> None:
    require(abs(actual - expected) <= tolerance, f"{label}: {actual} != {expected}")


def fit(rows: list[dict[str, str]]) -> dict[str, float]:
    samples = [
        (float(row["tick"]), math.log(float(row["target"])))
        for row in rows
        if 8 <= int(row["tick"]) <= 64
    ]
    require(len(samples) == 57, "fit sample count")
    require(all(math.isfinite(y) for _, y in samples), "finite log target")
    n = float(len(samples))
    sx = sum(x for x, _ in samples)
    sy = sum(y for _, y in samples)
    sxx = sum(x * x for x, _ in samples)
    sxy = sum(x * y for x, y in samples)
    denominator = n * sxx - sx * sx
    require(denominator > 0.0, "fit denominator")
    slope = (n * sxy - sx * sy) / denominator
    intercept = (sy - slope * sx) / n
    mean = sy / n
    rss0 = sum((y - mean) ** 2 for _, y in samples)
    rss1 = sum((y - intercept - slope * x) ** 2 for x, y in samples)
    require(rss0 > 0.0 and rss1 > 0.0, "positive RSS")
    bic0 = n * math.log(rss0 / n) + math.log(n)
    bic1 = n * math.log(rss1 / n) + 2.0 * math.log(n)
    return {
        "gamma": -slope,
        "delta_bic": bic0 - bic1,
        "r_squared": 1.0 - rss1 / rss0,
    }


def main() -> None:
    require(sha256(PREREG) == PROTOCOL, "preregistration hash")
    require(sha256(PARENT_JSON) == PARENT_JSON_SHA, "parent JSON hash")
    require(sha256(PARENT_CSV) == PARENT_CSV_SHA, "parent CSV hash")

    result = json.loads(RESULT_JSON.read_text(encoding="utf-8"))
    require(result["ftd_id"] == "FTD-0676", "result id")
    require(result["protocol_sha256"] == PROTOCOL, "result protocol")
    require(result["parent_json_sha256"] == PARENT_JSON_SHA, "recorded parent JSON")
    require(result["parent_csv_sha256"] == PARENT_CSV_SHA, "recorded parent CSV")
    require(result["verdict"] == "CANONICAL_PRECONTACT_EXPONENTIAL_TRANSFER_CONSTRUCTIVE", "verdict")
    require(result["production_changed"] is False, "production unchanged")
    require(result["parent_pass"] is True, "parent pass")
    require(result["initial_fields_bitwise_equal"] is True, "initial fields")
    require(result["exact_execution_pass"] is True, "exact execution")
    require(result["volume"] == 97 and result["horizon"] == 80, "volume/horizon")
    require(result["fit_start_tick"] == 8 and result["fit_end_tick"] == 64, "fit window")
    close(float(result["maximum_constituent_momentum_amplitude"]), 5e-7, 1e-15, "amplitude")
    close(float(result["parent_energy_decay_rate"]), PARENT_RATE, 1e-18, "parent rate")

    with RESULT_CSV.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    require(len(rows) == 162, "CSV row count")
    by_sign: dict[int, list[dict[str, str]]] = {-1: [], 1: []}
    for row in rows:
        require(row["ftd_id"] == "FTD-0676", "row id")
        require(row["protocol_sha256"] == PROTOCOL, "row protocol")
        sign = int(row["sign"])
        require(sign in by_sign, "row sign")
        require(row["observer_valid"] == "1", "observer row valid")
        require(float(row["target"]) > 0.0, "positive target")
        by_sign[sign].append(row)
    for sign in (-1, 1):
        by_sign[sign].sort(key=lambda row: int(row["tick"]))
        require([int(row["tick"]) for row in by_sign[sign]] == list(range(81)), f"ticks {sign}")

    fits = {sign: fit(by_sign[sign]) for sign in (-1, 1)}
    for sign, prefix in ((-1, "negative"), (1, "positive")):
        measured = fits[sign]
        close(measured["gamma"], float(result[f"{prefix}_gamma_energy"]), 5e-15, f"gamma {sign}")
        close(measured["delta_bic"], float(result[f"{prefix}_delta_bic"]), 5e-10, f"BIC {sign}")
        close(measured["r_squared"], float(result[f"{prefix}_r_squared"]), 5e-14, f"R2 {sign}")
        start = float(by_sign[sign][8]["target"])
        end = float(by_sign[sign][64]["target"])
        decline = 1.0 - end / start
        close(decline, float(result[f"{prefix}_decline_tick8_tick64"]), 5e-14, f"decline {sign}")
        parent_difference = abs(measured["gamma"] - PARENT_RATE) / PARENT_RATE
        close(parent_difference, float(result[f"{prefix}_parent_rate_relative_difference"]), 5e-13, f"parent difference {sign}")
        require(result[f"{prefix}_exact"] is True, f"exact {sign}")
        require(measured["gamma"] > 0.0, f"positive gamma {sign}")
        require(measured["delta_bic"] >= 10.0, f"BIC gate {sign}")
        require(measured["r_squared"] >= 0.995, f"R2 gate {sign}")
        require(decline >= 0.20, f"decline gate {sign}")
        require(parent_difference <= 0.05, f"parent stability {sign}")
        require(float(result[f"{prefix}_max_observer_residual"]) <= 1e-8, f"observer {sign}")
        require(float(result[f"{prefix}_max_energy_drift"]) <= 1e-10, f"energy {sign}")
        require(float(result[f"{prefix}_max_common_residual"]) <= 1e-10, f"action {sign}")
        require(float(result[f"{prefix}_recovery"]) <= 1e-8, f"inverse {sign}")

    rate_scale = max(abs(fits[-1]["gamma"]), abs(fits[1]["gamma"]), 1e-300)
    rate_difference = abs(fits[-1]["gamma"] - fits[1]["gamma"]) / rate_scale
    rms = math.sqrt(sum(
        (float(left["target"]) - float(right["target"])) ** 2
        for left, right in zip(by_sign[-1], by_sign[1], strict=True)
    ) / 81.0)
    close(rate_difference, float(result["polarity_rate_relative_difference"]), 5e-13, "rate polarity")
    close(rms, float(result["polarity_target_history_rms"]), 5e-14, "history RMS")
    require(rate_difference <= 1e-4, "rate polarity gate")
    require(rms <= 1e-5, "history polarity gate")

    print(
        "FTD-0676 canonical pre-contact mode-decay certificate: PASS "
        f"rows={len(rows)} gamma_minus={fits[-1]['gamma']:.15g} "
        f"gamma_plus={fits[1]['gamma']:.15g} verdict={result['verdict']}"
    )


if __name__ == "__main__":
    main()
