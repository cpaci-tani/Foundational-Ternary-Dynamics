#!/usr/bin/env python3
"""Independent FTD-0735 run-of-record certificate."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations" / (
    "PREREG_CAPTURE_ROOT_REGULARITY_NEIGHBORHOOD_v1.md"
)
RUNNER = ROOT / "engine/tests/test_capture_root_regularity_neighborhood.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0735" / (
    "ftd_0735_capture_root_regularity_neighborhood_v1.json"
)
CSV_PATH = ROOT / "engine/results/ftd_0735" / (
    "ftd_0735_capture_root_regularity_neighborhood_v1.csv"
)

EXPECTED_HASHES = {
    PREREG: "C8439AD7BCE95CF1EE530B28F741F9C1A11A7933478FD775FE4316C214C2A668",
    RUNNER: "5E242010C5FA334D92AF4BC7CD703425730228602AB2FCE89CF435D123B6DD48",
    JSON_PATH: "2927040F79C7C781FBCC6D032F70E03AE9611B3471D75757605F7526DA73D178",
    CSV_PATH: "C924AA735916E0D2682FB6E2CBCFF895AB7A44CB4C28786F489C4B7E90C22E9D",
}

SELECTORS = {
    "0_0_1": (
        "srp_s1p_s2m_rin_fminus",
        "srp_s1m_s2m_rin_fminus",
    ),
    "0_1_-1": (
        "srp_s1m_s2m_rin_fminus",
        "srp_s1m_s2p_rin_fminus",
    ),
    "1_1_1": (
        "srp_s1m_s2m_rin_fminus",
        "srp_s1p_s2m_rin_fminus",
    ),
}
POLARITIES = {"plus_minus", "minus_plus"}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def close(a: float, b: float, tolerance: float = 2e-15) -> bool:
    return abs(a - b) <= tolerance * max(1.0, abs(a), abs(b))


def main() -> int:
    checks = 0

    def check(condition: bool, message: str) -> None:
        nonlocal checks
        checks += 1
        if not condition:
            raise AssertionError(message)

    for path, expected in EXPECTED_HASHES.items():
        check(path.is_file(), f"missing artifact: {path}")
        check(sha256(path) == expected, f"hash mismatch: {path}")

    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    check(summary["identifier"] == "FTD-0735", "identifier")
    check(summary["protocol_sha256"] == EXPECTED_HASHES[PREREG], "protocol")
    check(
        summary["verdict"]
        == "CAPTURE_FINITE_TIME_OPEN_NEIGHBORHOOD_NUMERICALLY_SUPPORTED",
        "verdict",
    )
    check(summary["history_count"] == 18, "history count")
    check(summary["root_count"] == 9216, "root count")
    check(summary["survives"] == 18, "survival count")

    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    check(len(rows) == 9216, "CSV row count")

    grouped: dict[tuple[str, str, str], list[dict[str, str]]] = defaultdict(list)
    minimum_sigma = math.inf
    maximum_condition = 0.0
    maximum_scale = 0.0
    minimum_energy_margin = math.inf
    minimum_graph_margin = math.inf

    for row in rows:
        key = (row["direction"], row["polarity"], row["variant"])
        grouped[key].append(row)
        check(row["step_valid"] == "1", f"invalid root {key}")
        check(row["gates_pass"] == "1", f"failed action gate {key}")
        check(row["measured"] == "1", f"missing regularity {key}")
        check(int(row["evaluations"]) == 24, f"evaluation count {key}")
        sigma_min = float(row["sigma_min"])
        sigma_max = float(row["sigma_max"])
        condition = float(row["condition"])
        scale_difference = float(row["scale_difference"])
        common = float(row["common_residual"])
        check(math.isfinite(sigma_min) and sigma_min >= 1e-3, f"sigma {key}")
        check(sigma_max >= sigma_min, f"singular ordering {key}")
        check(math.isfinite(condition) and condition <= 1e4, f"condition {key}")
        check(scale_difference <= 1e-5, f"scale stability {key}")
        check(common <= 1e-10, f"common residual {key}")
        minimum_sigma = min(minimum_sigma, sigma_min)
        maximum_condition = max(maximum_condition, condition)
        maximum_scale = max(maximum_scale, scale_difference)
        if row["phase"] == "forward":
            pair_energy = float(row["pair_energy"])
            graph_margin = float(row["graph_margin"])
            check(pair_energy < -1e-6, f"forward energy {key}")
            check(graph_margin > 0.0, f"forward graph {key}")
            minimum_energy_margin = min(minimum_energy_margin, -pair_energy / 0.01)
            minimum_graph_margin = min(minimum_graph_margin, graph_margin)

    expected_keys = {
        (direction, polarity, variant)
        for direction, selectors in SELECTORS.items()
        for polarity in POLARITIES
        for variant in ("center", *selectors)
    }
    check(set(grouped) == expected_keys, "history matrix")
    for key, history in grouped.items():
        check(len(history) == 512, f"history length {key}")
        forward_ticks = sorted(
            int(row["tick"]) for row in history if row["phase"] == "forward"
        )
        reverse_ticks = sorted(
            int(row["tick"]) for row in history if row["phase"] == "reverse"
        )
        check(forward_ticks == list(range(1, 257)), f"forward ticks {key}")
        check(reverse_ticks == list(range(1, 257)), f"reverse ticks {key}")

    check(close(minimum_sigma, float(summary["minimum_sigma"])), "minimum sigma")
    check(
        close(maximum_condition, float(summary["maximum_condition"])),
        "maximum condition",
    )
    check(
        close(maximum_scale, float(summary["maximum_scale_difference"])),
        "maximum scale difference",
    )
    check(
        close(minimum_energy_margin, float(summary["minimum_energy_margin"])),
        "minimum energy margin",
    )
    check(
        close(minimum_graph_margin, float(summary["minimum_graph_margin"])),
        "minimum graph margin",
    )
    check(float(summary["maximum_inverse"]) <= 1e-8, "inverse gate")

    print(f"FTD-0735 certificate: {checks}/{checks} checks PASS")
    print(f"verdict={summary['verdict']}")
    print(
        "histories=18/18 roots=9216/9216 "
        f"sigma_min={minimum_sigma:.9g} "
        f"condition_max={maximum_condition:.9g} "
        f"scale_difference_max={maximum_scale:.9g}"
    )
    print("scope=finite horizon on admissible selected-dynamics state manifold")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
