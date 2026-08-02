"""Independent FTD-0641 matched field-mode certificate."""
from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0641"
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_v1.md"
SHA = "6EB4C1035C29187F22D2FC8BD7A152EF47F148165204B7614504A70979EDB9C8"


def relative(a: float, b: float) -> float:
    return abs(a-b)/max(1e-300, abs(a), abs(b))


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    summary = json.loads((RESULT / "ftd_0641_connected_block_independent_field_modes_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0641_connected_block_independent_field_modes_arms_v1.csv").open()))
    ticks = list(csv.DictReader((RESULT / "ftd_0641_connected_block_independent_field_modes_ticks_v1.csv").open()))
    assert summary["protocol_sha256"] == SHA
    assert summary["verdict"] == "CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_CONSTRUCTIVE"
    assert summary["production_changed"] is False and summary["arm_count"] == 54
    assert all(summary[key] == 1 for key in ("coverage_pass", "execution_pass", "bounded_pass",
        "frequency_pass", "recurrence_pass", "amplitude_pass", "sign_pass",
        "polarization_pass", "cubic_pass", "monotonic_pass"))
    assert len(arms) == 54 and len(ticks) == 54*256
    by_label: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in ticks: by_label[row["label"]].append(row)
    amap = {(a["family"], max(int(a[x]) for x in ("nx","ny","nz")),
             int(a["permutation"]), int(a["polarization"]), a["kind"]): a for a in arms}
    worst_phase = 0.0
    for arm in arms:
        assert all(arm[k] == "1" for k in ("initialized","complete","reversed","bounded"))
        assert abs(float(arm["initial_max"])-float(arm["target"])) <= 1e-14
        assert float(arm["max_divergence"]) <= 1e-12
        assert float(arm["max_full_drift"]) <= 1e-12
        assert float(arm["max_background_drift"]) <= 1e-12
        assert float(arm["max_recurrence"]) <= 1e-8
        assert float(arm["recovery"]) <= 1e-11
        nx,ny,nz=(int(arm[k]) for k in ("nx","ny","nz"))
        sigma=2*math.sqrt(sum(math.sin(math.pi*n/17)**2 for n in (nx,ny,nz)))
        predicted=2*math.asin((1/math.sqrt(3))*sigma/2)
        assert abs(sigma-float(arm["sigma"])) <= 1e-15
        assert abs(predicted-float(arm["predicted_phase"])) <= 1e-15
        rows=sorted(by_label[arm["label"]],key=lambda r:int(r["tick"]))
        q=[float(r["q"]) for r in rows]
        num=sum(q[i]*(q[i+1]+q[i-1]) for i in range(1,255))
        den=sum(2*q[i]*q[i] for i in range(1,255))
        phase=math.acos(max(-1,min(1,num/den)))
        assert abs(phase-float(arm["phase"])) <= 2e-13
        if arm["kind"] == "primary":
            assert relative(phase,predicted) <= 1e-8
            worst_phase=max(worst_phase,relative(phase,predicted))
    for family in ("100","110","111"):
        canonical=2 if family=="110" else 0
        for pol in (0,1):
            p=amap[(family,1,canonical,pol,"primary")]
            h=amap[(family,1,canonical,pol,"half")]
            n=amap[(family,1,canonical,pol,"negative")]
            assert relative(float(p["phase"]),float(h["phase"])) <= 1e-8
            assert relative(float(p["phase"]),float(n["phase"])) <= 1e-8
            assert max(abs(float(a["q"])+float(b["q"])) for a,b in zip(by_label[p["label"]],by_label[n["label"]]))/1e-7 <= 1e-8
        prior=0.0
        for n in (1,2,3):
            phase=float(amap[(family,n,canonical,0,"primary")]["phase"])
            assert phase>prior
            prior=phase
    print(f"FTD-0641 certificate: 54 independent field arms pass; worst primary phase error {worst_phase:.3e}")


if __name__ == "__main__": main()
