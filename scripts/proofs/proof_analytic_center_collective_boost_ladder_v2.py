"""Independent FTD-0644 directional-closure certificate."""
from __future__ import annotations

import csv
import hashlib
import json
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_ANALYTIC_CENTER_COLLECTIVE_BOOST_LADDER_v2.md"
RESULT = ROOT / "engine/results/ftd_0644"
SHA = "24464297DBB82B8D14E143CADB593D49EA07DFDC42648D57350CEF66971B1DD3"


def cycle(v: tuple[float, float, float], count: int) -> tuple[float, float, float]:
    for _ in range(count):
        v = (v[2], v[0], v[1])
    return v


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    summary = json.loads((RESULT / "ftd_0644_analytic_center_collective_boost_ladder_v2.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0644_analytic_center_collective_boost_ladder_arms_v2.csv").open()))
    ticks = list(csv.DictReader((RESULT / "ftd_0644_analytic_center_collective_boost_ladder_ticks_v2.csv").open()))
    assert summary["protocol_sha256"] == SHA
    assert summary["verdict"] == "ANALYTIC_CENTER_V2_DIRECTIONAL_TRANSPORT_CLOSED"
    assert summary["arm_count"] == len(arms) == 32 and len(ticks) == 32*16
    assert all(summary[k] == 1 for k in ("rotation_preflight_pass", "coverage_pass",
        "execution_pass", "coherence_pass", "rest_pass", "high_boost_pass",
        "monotonic_pass", "mirror_pass"))
    assert summary["cubic_pass"] == 0
    assert summary["rotation_energy_residual"] <= 1e-9
    assert summary["rotation_spectrum_residual"] <= 1e-9
    assert all(a[k] == "1" for a in arms for k in ("initialized", "forward", "reverse", "coherent"))
    assert all(a["onset"] == "mobile" for a in arms if a["kind"] == "ladder")
    assert min(float(a["mobility"]) for a in arms if a["kind"] == "ladder") >= 0.5

    by: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in ticks: by[row["label"]].append(row)
    pairs = [("100_p0.120000", "100_y", 1), ("100_p0.120000", "100_z", 2),
             ("110_p0.120000", "110_yz", 1), ("110_p0.120000", "110_xz", 2)]
    maxima = {k: 0.0 for k in ("center", "momentum", "shape", "field", "soft", "dressing")}
    for canonical, rotated, maps in pairs:
        for a, b in zip(by[canonical], by[rotated]):
            da = cycle(tuple(float(a[f"disp_{q}"]) for q in "xyz"), maps)
            db = tuple(float(b[f"disp_{q}"]) for q in "xyz")
            pa = cycle(tuple(float(a[f"momentum_{q}"]) for q in "xyz"), maps)
            pb = tuple(float(b[f"momentum_{q}"]) for q in "xyz")
            maxima["center"] = max(maxima["center"], *(abs(x-y) for x,y in zip(da,db)))
            maxima["momentum"] = max(maxima["momentum"], *(abs(x-y) for x,y in zip(pa,pb)))
            maxima["shape"] = max(maxima["shape"], abs(float(a["shape"])-float(b["shape"])))
            maxima["field"] = max(maxima["field"], abs(float(a["field_energy"])-float(b["field_energy"])))
            maxima["soft"] = max(maxima["soft"], abs(float(a["soft_fraction"])-float(b["soft_fraction"])))
            maxima["dressing"] = max(maxima["dressing"], abs(float(a["dressing"])-float(b["dressing"])))
    assert max(maxima[k] for k in maxima if k != "soft") <= 1e-7
    assert maxima["soft"] > 1e-7
    assert abs(maxima["soft"]-summary["cubic_residual"]) <= 1e-15
    print("FTD-0644 certificate: dynamics covariant; unrotated soft-basis observer alone closes conjunction")


if __name__ == "__main__":
    main()
