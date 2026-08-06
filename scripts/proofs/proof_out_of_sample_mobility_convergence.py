"""Independent run-of-record checks for FTD-0654."""

from __future__ import annotations

import csv
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT_DIR = ROOT / "engine/results/ftd_0654"
RESULT = RESULT_DIR / "ftd_0654_out_of_sample_mobility_v1.json"
ARMS = RESULT_DIR / "ftd_0654_out_of_sample_mobility_arms_v1.csv"
CHECKPOINTS = RESULT_DIR / "checkpoints"
PROTOCOL_SHA = "10C77F2DF5DADA77E583145498ED4D33EF1E2F0A3EF31938BA5A883D301CBEA2"


def truth(value: str) -> bool:
    return value.strip().lower() == "true"


def main() -> None:
    result = json.loads(RESULT.read_text(encoding="utf-8"))
    with ARMS.open(newline="", encoding="utf-8") as handle:
        arms = list(csv.DictReader(handle))

    assert result["protocol_sha256"] == PROTOCOL_SHA
    assert result["verdict"] == "OUT_OF_SAMPLE_NORMALIZED_MOBILITY_CONSTRUCTIVE"
    assert result["arm_count"] == len(arms) == 30
    assert all(result[key] for key in (
        "coverage", "execution", "exact", "coherence", "zero", "mirror",
        "cubic", "all_primary_persistent", "normalized_target",
    ))
    assert not result["renormalized_common_candidate"]
    assert result["worst_action"] <= 1e-9
    assert result["worst_recovery"] <= 1e-7
    assert result["worst_strain"] <= 0.10
    assert result["worst_zero"] <= 1e-6
    assert result["mirror_residual"] <= 1e-6
    assert result["cubic_residual"] <= 1e-6

    primary = [row for row in arms if row["kind"] == "primary"]
    assert len(primary) == 18
    assert all(truth(row["persistent"]) for row in primary)
    for row in arms:
        assert all(truth(row[key]) for key in (
            "initialized", "forward", "reverse", "exact", "coherent"
        ))
        width = int(row["width"])
        assert int(row["ticks"]) == 32 * width
        checkpoint = CHECKPOINTS / f"{row['label']}.csv"
        with checkpoint.open(newline="", encoding="utf-8") as handle:
            ticks = list(csv.DictReader(handle))
        assert len(ticks) == 64 * width
        assert all(tick["protocol_sha256"] == PROTOCOL_SHA for tick in ticks)
        assert all(float(tick["action"]) <= 1e-9 for tick in ticks)
        assert all(float(tick["causal"]) <= 1e-12 for tick in ticks)
        assert all(float(tick["relative_edge_strain"]) <= 0.10 for tick in ticks)

    for speed_name, speed in (("02", 0.02), ("03", 0.03)):
        errors: dict[int, float] = {}
        spans: dict[int, float] = {}
        defects: dict[int, float] = {}
        for width in (2, 3, 4):
            group = [
                row for row in primary
                if int(row["width"]) == width
                and abs(float(row["speed"]) - speed) <= 1e-14
            ]
            assert len(group) == 3
            mobility = [float(row["mobility"]) for row in group]
            errors[width] = max(abs(value - 1.0) for value in mobility)
            spans[width] = max(mobility) - min(mobility)
            defects[width] = max(float(row["normalized_spline_defect"]) for row in group)
            recorded = result["metrics"][speed_name][str(width)]
            assert abs(errors[width] - recorded["error"]) <= 1e-12
            assert abs(spans[width] - recorded["span"]) <= 1e-12
            assert abs(defects[width] - recorded["defect"]) <= 1e-12
        assert errors[4] < errors[3] < errors[2]
        assert spans[4] < spans[3] < spans[2]
        assert defects[4] < defects[2]

    print("FTD-0654 independent certificate: PASS")


if __name__ == "__main__":
    main()
