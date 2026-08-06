"""Independent run-of-record checks for FTD-0651."""

from __future__ import annotations

import csv
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0651/ftd_0651_repeated_exact_root_acceleration_v1.json"
ARMS = ROOT / "engine/results/ftd_0651/ftd_0651_repeated_exact_root_acceleration_arms_v1.csv"
PROTOCOL_SHA = "06371B4E788FBB3E2840875340557F620617C593D686E8C410ECFE341266298A"


def truth(value: str) -> bool:
    return value.strip().lower() == "true"


def main() -> None:
    result = json.loads(RESULT.read_text(encoding="utf-8"))
    with ARMS.open(newline="", encoding="utf-8") as handle:
        arms = list(csv.DictReader(handle))

    assert result["protocol_sha256"] == PROTOCOL_SHA
    assert result["verdict"] == "REPEATED_EXACT_ROOT_ACCELERATION_CONSTRUCTIVE"
    assert len(arms) == result["arm_count"] == 12
    expected = {
        f"w{width}_{launch}"
        for width in (2, 3, 4)
        for launch in ("v01_100", "v04_100", "v04_110", "v04_111")
    }
    assert {row["label"] for row in arms} == expected

    for row in arms:
        assert all(
            truth(row[key])
            for key in (
                "initialized",
                "matrix_free_valid",
                "cached_valid",
                "action",
                "equivalent",
                "inverse",
                "reuse",
            )
        )
        assert float(row["max_state_difference"]) <= 1e-8
        assert float(row["max_matrix_free_action"]) <= 1e-9
        assert float(row["max_cached_action"]) <= 1e-9
        assert float(row["matrix_free_recovery"]) <= 1e-8
        assert float(row["cached_recovery"]) <= 1e-8
        assert int(row["cached_refreshes"]) >= 1
        assert int(row["cached_reuses"]) >= 1
        assert int(row["ticks"]) == (3 if int(row["width"]) == 2 else 1)

    repeated = [row for row in arms if int(row["width"]) == 2]
    matrix_evaluations = sum(int(row["matrix_free_evaluations"]) for row in repeated)
    cached_evaluations = sum(int(row["cached_evaluations"]) for row in repeated)
    assert matrix_evaluations == result["repeated_matrix_free_evaluations"]
    assert cached_evaluations == result["repeated_cached_evaluations"]
    assert cached_evaluations < matrix_evaluations
    assert max(float(row["max_state_difference"]) for row in arms) == result[
        "worst_state_difference"
    ]
    print("FTD-0651 independent certificate: PASS")


if __name__ == "__main__":
    main()
