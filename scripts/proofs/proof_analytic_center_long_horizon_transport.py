"""Independent FTD-0646 long-horizon transport certificate."""
from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_ANALYTIC_CENTER_LONG_HORIZON_TRANSPORT_v1.md"
PARENT = ROOT / "engine/results/ftd_0645/ftd_0645_analytic_center_collective_boost_ladder_v3.json"
RESULT = ROOT / "engine/results/ftd_0646"
SHA = "AFE7AD10F6A3935D882EE327F08FDA721ABF19A223029201EACB24FC98C989AF"
PARENT_SHA = "694D46A2EBA1D5ABC96A6525B253737359BCD43F442277F2231150DBEBE8CFD4"


def fit(rows: list[dict[str, str]]) -> tuple[float, float]:
    points = [(int(r["tick"]), float(r["projected"])) for r in rows if int(r["tick"]) >= 65]
    mt = sum(t for t,_ in points)/len(points)
    my = sum(y for _,y in points)/len(points)
    covariance = sum((t-mt)*(y-my) for t,y in points)
    variance = sum((t-mt)**2 for t,_ in points)
    slope = covariance/variance
    intercept = my-slope*mt
    total = sum((y-my)**2 for _,y in points)
    residual = sum((y-(intercept+slope*t))**2 for t,y in points)
    return slope, 1-residual/total


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    assert hashlib.sha256(PARENT.read_bytes()).hexdigest().upper() == PARENT_SHA
    summary = json.loads((RESULT / "ftd_0646_analytic_center_long_horizon_transport_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0646_analytic_center_long_horizon_transport_arms_v1.csv").open()))
    ticks = list(csv.DictReader((RESULT / "ftd_0646_analytic_center_long_horizon_transport_ticks_v1.csv").open()))
    assert summary["protocol_sha256"] == SHA and summary["parent_result_sha256"] == PARENT_SHA
    assert summary["verdict"] == "ANALYTIC_CENTER_LONG_HORIZON_MIXED"
    assert summary["arm_count"] == len(arms) == 23 and len(ticks) == 23*256
    assert all(summary[k] == 1 for k in ("coverage_pass", "execution_pass", "coherence_pass",
                                         "rest_pass", "mirror_pass", "cubic_pass"))
    assert summary["persistent_count"] == 2 and summary["bounded_reversal_count"] == 4
    assert all(a[k] == "1" for a in arms for k in ("initialized", "forward", "reverse", "coherent"))
    assert max(float(a["max_drift"]) for a in arms) <= 1e-9
    assert max(float(a["max_common"]) for a in arms) <= 1e-10
    assert max(float(a["recovery"]) for a in arms) <= 1e-8
    assert max(float(a["max_shape"]) for a in arms) <= 0.05
    assert max(float(a["max_strain"]) for a in arms) <= 0.05

    by: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in ticks: by[row["label"]].append(row)
    persistent, reversal = [], []
    for arm in arms:
        rows = sorted(by[arm["label"]], key=lambda r: int(r["tick"]))
        assert [int(r["tick"]) for r in rows] == list(range(1,257))
        if arm["kind"] != "rest":
            checkpoints = [float(rows[i-1]["projected"]) for i in (64,128,192,256)]
            recorded = [float(arm[f"checkpoint{i}"]) for i in (64,128,192,256)]
            assert max(abs(a-b) for a,b in zip(checkpoints,recorded)) <= 1e-15
            slope,r2 = fit(rows)
            assert abs(slope-float(arm["fit_velocity"])) <= 1e-15
            assert abs(r2-float(arm["fit_r2"])) <= 2e-14
        if arm["kind"] == "ladder":
            if arm["persistent"] == "1": persistent.append(arm["label"])
            if arm["bounded_reversal"] == "1": reversal.append(arm["label"])
    assert persistent == ["100_p0.015000", "110_p0.015000"]
    assert reversal == ["100_p0.003750", "110_p0.001875", "111_p0.001875", "111_p0.007500"]
    assert summary["mirror_residual"] <= 1e-6 and summary["cubic_residual"] <= 1e-6
    print("FTD-0646 certificate: coherent long-horizon dynamics are mixed; 2/12 canonical arms persistent")


if __name__ == "__main__":
    main()
