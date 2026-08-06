"""Independent FTD-0632 multiplicity-law certificate."""
from __future__ import annotations

import csv
import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0632"


def main() -> None:
    summary = json.loads((RESULT / "ftd_0632_connected_block_cubic_eight_fibre_v1.json").read_text())
    rows = list(csv.DictReader((RESULT / "ftd_0632_connected_block_cubic_eight_fibre_stencil_v1.csv").open()))
    assert summary["protocol_sha256"] == "EAB5EA583966BB1FA7C2F8AD7CAFAB66111407F1F6F477F87293F1B7B374C8AD"
    assert summary["verdict"] == "CUBIC_EIGHT_FIBRE_NECESSARY_AND_SUFFICIENT_FOR_LOCKED_CHART"
    assert len(rows) == 66
    counts = {0: Counter(), 1: Counter()}
    failures = {2: 0, 4: 0, 8: 0}
    for row in rows:
        m = int(row["multiplicity"])
        counts[int(row["orientation"])][m] += 1
        for cap in (2, 4, 8):
            passed = row[f"cap{cap}"] == "1"
            assert passed == (m <= cap)
            failures[cap] += not passed
        assert row["positions"] == "1" and row["law"] == "1"
        if row["same_anchor_separation"] != "inf":
            assert float(row["same_anchor_separation"]) >= 0.9
    assert counts[0] == counts[1]
    assert max(int(r["multiplicity"]) for r in rows) == 8
    assert failures == {2: 26, 4: 14, 8: 0}
    assert float(summary["maximum_gauss_residual"]) <= 1e-11
    print("FTD-0632 certificate: 66/66 geometries satisfy the exact cap/multiplicity law")


if __name__ == "__main__":
    main()
