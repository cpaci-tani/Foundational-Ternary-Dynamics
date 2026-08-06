"""Independent FTD-0633 static-basin certificate."""
from __future__ import annotations

import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0633"


def main() -> None:
    summary = json.loads((RESULT / "ftd_0633_connected_block_eight_fibre_static_refinement_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0633_connected_block_eight_fibre_arms_v1.csv").open()))
    translations = list(csv.DictReader((RESULT / "ftd_0633_connected_block_eight_fibre_translation_v1.csv").open()))
    ticks = list(csv.DictReader((RESULT / "ftd_0633_connected_block_eight_fibre_ticks_v1.csv").open()))
    assert summary["protocol_sha256"] == "E8B52368BF33D6ED175014DA359626967C8E41570B2A38530E1E13A5C92DB44E"
    assert summary["verdict"] == "CONNECTED_BLOCK_EIGHT_FIBRE_STATIC_BASIN_CONSTRUCTIVE"
    assert summary["fibre_limit"] == 8 and len(arms) == 2 and len(translations) == 6 and len(ticks) == 128
    for row in arms:
        assert all(row[k] == "1" for k in ("optimization", "positive_hessian", "translation_basin", "full_stationarity", "forward", "reverse", "repeated"))
        assert float(row["refined_energy"]) < float(row["starting_energy"]) < 0.0367648643204065
        assert max(abs(float(row[f"g{i}"])) for i in range(4)) <= 1e-9
        assert min(float(row[f"e{i}"]) for i in range(4)) > 1e-6
        assert float(row["max_impulse"]) <= 1e-9
        assert float(row["max_state_distance"]) <= 1e-8
        assert float(row["max_energy_drift"]) <= 1e-12
        assert float(row["recovery"]) <= 1e-10
    assert all(abs(float(r["gradient"])) <= 1e-9 and float(r["curvature"]) > 1e-4 for r in translations)
    assert float(summary["covariance_residual"]) <= 1e-9
    print("FTD-0633 certificate: 46/46 static, translation, action, and inverse gates pass")


if __name__ == "__main__":
    main()
