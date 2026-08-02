"""Independent FTD-0630 finite-difference and verdict certificate."""
from __future__ import annotations

import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0630"


def main() -> None:
    summary = json.loads((RESULT / "ftd_0630_connected_block_translation_curvature_v1.json").read_text())
    rows = list(csv.DictReader((RESULT / "ftd_0630_connected_block_translation_curvatures_v1.csv").open()))
    assert summary["protocol_sha256"] == "4BF3F43F841ABC653611E40FA74B6BF0AB7FEEE14C5F226062D456B34DB76586"
    assert summary["verdict"] == "CONNECTED_BLOCK_TRANSLATION_CURVATURE_EXECUTION_INVALID"
    assert len(rows) == 12
    h = 2e-4
    for row in rows:
        em, e0, ep = (float(row[k]) for k in ("minus", "center", "plus"))
        g = (ep - em) / (2 * h)
        k = (ep - 2 * e0 + em) / h**2
        assert abs(g - float(row["gradient"])) < 1e-15
        assert abs(k - float(row["curvature"])) < 1e-12
    body = [float(r["curvature"]) for r in rows if r["state"] == "body_half"]
    full = [float(r["curvature"]) for r in rows if r["state"] == "full_half"]
    assert sum(x < -1e-6 for x in body) == 4
    assert all(x > 1e-6 for x in full)
    assert summary["body_saddle_pass"] == 1 and summary["full_half_basin_pass"] == 1
    assert summary["covariance_pass"] == 0
    assert float(summary["covariance_residual"]) > 1e-8
    print("FTD-0630 certificate: 24/24 checks pass; locked covariance gate fails as recorded")


if __name__ == "__main__":
    main()
