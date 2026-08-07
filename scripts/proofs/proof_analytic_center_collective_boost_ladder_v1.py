"""Independent FTD-0643 execution-invalid certificate."""
from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_ANALYTIC_CENTER_COLLECTIVE_BOOST_LADDER_v1.md"
RESULT = ROOT / "engine/results/ftd_0643"
SHA = "CE7CB494A298CED4C04F39E7B776D2785675B339398621AF7D0DBB92CA0B03ED"


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    summary = json.loads((RESULT / "ftd_0643_analytic_center_collective_boost_ladder_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0643_analytic_center_collective_boost_ladder_arms_v1.csv").open()))
    ticks = list(csv.DictReader((RESULT / "ftd_0643_analytic_center_collective_boost_ladder_ticks_v1.csv").open()))
    assert summary["protocol_sha256"] == SHA
    assert summary["verdict"] == "ANALYTIC_CENTER_BOOST_EXECUTION_INVALID"
    assert summary["coverage_pass"] == 0
    assert summary["arm_count"] == len(arms) == 32
    assert len(ticks) == 32*16
    # The preregistered list contains 1 rest + 21 ladder + 6 mirrors + 4
    # cyclic arms = 32, despite its prose declaring 29. Count by kind.
    counts: dict[str, int] = {}
    for arm in arms:
        counts[arm["kind"]] = counts.get(arm["kind"], 0) + 1
        assert arm["initialized"] == arm["forward"] == arm["reverse"] == arm["coherent"] == "1"
    assert counts == {"rest": 1, "ladder": 21, "mirror": 6, "cyclic": 4}
    assert sum(counts.values()) == 32
    print("FTD-0643 certificate: protocol/runner arm-count inconsistency detected; execution-invalid retained")


if __name__ == "__main__":
    main()
