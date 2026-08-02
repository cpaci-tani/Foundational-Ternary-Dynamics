"""Independent FTD-0631 cap-two chart obstruction certificate."""
from __future__ import annotations

import csv
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0631"


def main() -> None:
    summary = json.loads((RESULT / "ftd_0631_connected_block_full_half_static_refinement_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0631_connected_block_full_half_arms_v1.csv").open()))
    assert summary["protocol_sha256"] == "ADB2F73EDF9092C8DA0E0446BD26450432698CCA88A2B2CECF204C105FF00EE8"
    assert summary["verdict"] == "CONNECTED_BLOCK_FULL_HALF_STATIC_REFINEMENT_CLOSED_NEGATIVE"
    assert len(arms) == 2 and {r["orientation"] for r in arms} == {"0", "1"}
    for row in arms:
        assert row["init"] == "1" and row["optimization"] == "0"
        assert float(row["starting_energy"]) < 0.0367648643204065
        assert math.isinf(float(row["refined_energy"]))
        assert int(row["evaluations"]) == 41
    assert summary["coverage_pass"] == 1 and summary["initialization_pass"] == 1
    assert summary["reduced_basin_pass"] == 0
    print("FTD-0631 certificate: cap-two Hessian neighbourhood closes negative exactly as recorded")


if __name__ == "__main__":
    main()
