"""Independent FTD-0645 coherent-boost certificate."""
from __future__ import annotations

import csv
import hashlib
import json
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_ANALYTIC_CENTER_COLLECTIVE_BOOST_LADDER_v3.md"
RESULT = ROOT / "engine/results/ftd_0645"
SHA = "7E8C98F2ECABC80DE4D27ECA21CFA151922C16CADF0DFC05FD30DD0902B14A1D"


def cycle(v: tuple[float, float, float], count: int) -> tuple[float, float, float]:
    for _ in range(count): v = (v[2], v[0], v[1])
    return v


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    summary = json.loads((RESULT / "ftd_0645_analytic_center_collective_boost_ladder_v3.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0645_analytic_center_collective_boost_ladder_arms_v3.csv").open()))
    ticks = list(csv.DictReader((RESULT / "ftd_0645_analytic_center_collective_boost_ladder_ticks_v3.csv").open()))
    assert summary["protocol_sha256"] == SHA
    assert summary["verdict"] == "ANALYTIC_CENTER_V3_COHERENT_NO_THRESHOLD_AT_LADDER_RESOLUTION"
    assert summary["arm_count"] == len(arms) == 32 and len(ticks) == 32*16
    assert all(summary[k] == 1 for k in ("rotation_preflight_pass", "coverage_pass",
        "execution_pass", "coherence_pass", "rest_pass", "high_boost_pass",
        "monotonic_pass", "mirror_pass", "cubic_pass"))
    assert summary["rotation_energy_residual"] <= 1e-9
    assert summary["rotation_spectrum_residual"] <= 1e-9
    assert all(a[k] == "1" for a in arms for k in ("initialized", "forward", "reverse", "coherent"))
    assert all(a["onset"] == "mobile" for a in arms if a["kind"] == "ladder")
    assert min(float(a["mobility"]) for a in arms if a["kind"] == "ladder") >= 0.5
    assert max(float(a["max_shape"]) for a in arms) <= 0.05
    assert max(float(a["max_strain"]) for a in arms) <= 0.05
    assert max(float(a["max_drift"]) for a in arms) <= 1e-10
    assert max(float(a["max_common"]) for a in arms) <= 1e-10
    assert max(float(a["recovery"]) for a in arms) <= 1e-9
    high = [a for a in arms if a["kind"] == "ladder" and abs(float(a["p"])-0.12) < 1e-15]
    assert len(high) == 3
    for a in high:
        assert a["high"] == "1" and float(a["projected"]) >= 0.75
        assert float(a["mobility"]) >= 0.75 and float(a["transverse"]) <= 0.10
        assert int(a["total_hops"]) >= 16 and float(a["final_velocity"]) > 0
        assert float(a["soft_fraction"]) >= 0.95 and float(a["max_dressing"]) <= 0.50

    by: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in ticks: by[row["label"]].append(row)
    pairs = [("100_p0.120000", "100_y", 1), ("100_p0.120000", "100_z", 2),
             ("110_p0.120000", "110_yz", 1), ("110_p0.120000", "110_xz", 2)]
    cubic = 0.0
    for canonical, rotated, maps in pairs:
        for a, b in zip(by[canonical], by[rotated]):
            da = cycle(tuple(float(a[f"disp_{q}"]) for q in "xyz"), maps)
            db = tuple(float(b[f"disp_{q}"]) for q in "xyz")
            pa = cycle(tuple(float(a[f"momentum_{q}"]) for q in "xyz"), maps)
            pb = tuple(float(b[f"momentum_{q}"]) for q in "xyz")
            cubic = max(cubic, *(abs(x-y) for x,y in zip(da,db)),
                        *(abs(x-y) for x,y in zip(pa,pb)),
                        abs(float(a["shape"])-float(b["shape"])),
                        abs(float(a["field_energy"])-float(b["field_energy"])),
                        abs(float(a["soft_fraction"])-float(b["soft_fraction"])),
                        abs(float(a["dressing"])-float(b["dressing"])))
    assert cubic <= 1e-7
    assert abs(cubic-summary["cubic_residual"]) <= 1e-15
    print("FTD-0645 certificate: 32 coherent reversible boost arms pass; no threshold resolved on locked ladder")


if __name__ == "__main__":
    main()
