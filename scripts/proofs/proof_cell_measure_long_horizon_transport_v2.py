"""Independent run-of-record checks for FTD-0652."""

from __future__ import annotations

import csv
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT_DIR = ROOT / "engine/results/ftd_0652"
RESULT = RESULT_DIR / "ftd_0652_cell_measure_long_horizon_v2.json"
ARMS = RESULT_DIR / "ftd_0652_cell_measure_long_horizon_arms_v2.csv"
CHECKPOINTS = RESULT_DIR / "checkpoints"
PROTOCOL_SHA = "1F6AB75BC11FD05D93E450029D020CDCA94B76CA7E1186A8197CC110AFFC829D"


def truth(value: str) -> bool:
    return value.strip().lower() == "true"


def close(left: float, right: float, tolerance: float = 1e-12) -> bool:
    return math.isclose(left, right, rel_tol=tolerance, abs_tol=tolerance)


def main() -> None:
    result = json.loads(RESULT.read_text(encoding="utf-8"))
    with ARMS.open(newline="", encoding="utf-8") as handle:
        arms = list(csv.DictReader(handle))

    assert result["protocol_sha256"] == PROTOCOL_SHA
    assert result["verdict"] == "CELL_MEASURE_LONG_HORIZON_MIXED"
    assert result["arm_count"] == len(arms) == 30
    assert all(result[key] for key in (
        "coverage_pass", "execution_pass", "exact_pass", "coherence_pass",
        "zero_pass", "mirror_pass", "cubic_pass", "transport_pass",
        "anisotropy_trend_pass", "defect_trend_pass",
    ))
    assert not result["mobility_trend_pass"]
    assert not result["resolution_pass"]

    labels = {row["label"] for row in arms}
    assert len(labels) == 30
    for row in arms:
        assert all(truth(row[key]) for key in (
            "initialized", "forward", "reverse", "exact", "coherent"
        ))
        width = int(row["width"])
        assert int(row["ticks"]) == 16 * width
        checkpoint = json.loads(
            (CHECKPOINTS / f"{row['label']}.json").read_text(encoding="utf-8")
        )
        assert checkpoint["protocol_sha256"] == PROTOCOL_SHA
        assert checkpoint["label"] == row["label"]
        assert checkpoint["record_count"] == 32 * width
        assert checkpoint["initialized"] and checkpoint["forward"]
        assert checkpoint["reverse"] and checkpoint["exact"]
        assert checkpoint["coherent"]
        with (CHECKPOINTS / f"{row['label']}.csv").open(
            newline="", encoding="utf-8"
        ) as handle:
            tick_rows = list(csv.DictReader(handle))
        assert len(tick_rows) == 32 * width
        assert all(float(tick["action"]) <= 1e-9 for tick in tick_rows)
        assert all(float(tick["causal"]) <= 1e-12 for tick in tick_rows)
        assert all(float(tick["relative_edge_strain"]) <= 0.10 for tick in tick_rows)

    high = [
        row for row in arms
        if row["kind"] == "primary" and close(float(row["speed"]), 0.04)
    ]
    low = [
        row for row in arms
        if row["kind"] == "primary" and close(float(row["speed"]), 0.01)
    ]
    assert len(high) == len(low) == 9
    assert sum(truth(row["persistent"]) for row in high) == 9
    assert sum(truth(row["persistent"]) for row in low) == 6

    minima: dict[str, float] = {}
    spans: dict[str, float] = {}
    defects: dict[str, float] = {}
    for width in (2, 3, 4):
        group = [row for row in high if int(row["width"]) == width]
        mobility = [float(row["mobility"]) for row in group]
        defect = [float(row["normalized_spline_defect"]) for row in group]
        minima[str(width)] = min(mobility)
        spans[str(width)] = max(mobility) - min(mobility)
        defects[str(width)] = max(defect)
        assert close(minima[str(width)], result["minimum_high_mobility"][str(width)])
        assert close(spans[str(width)], result["high_mobility_span"][str(width)])
        assert close(defects[str(width)], result["maximum_high_defect"][str(width)])

    assert not (minima["2"] <= minima["3"] + 1e-4
                and minima["3"] <= minima["4"] + 1e-4)
    assert spans["4"] < spans["2"]
    assert defects["4"] < defects["2"]
    assert result["worst_action_residual"] <= 1e-9
    assert result["worst_recovery"] <= 1e-7
    assert result["worst_relative_edge_strain"] <= 0.10
    assert result["worst_zero_displacement"] <= 1e-6
    assert result["mirror_residual"] <= 1e-6
    assert result["cubic_residual"] <= 1e-6
    print("FTD-0652 independent certificate: PASS (locked MIXED verdict)")


if __name__ == "__main__":
    main()
