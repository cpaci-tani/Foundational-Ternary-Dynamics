"""Independent FTD-0642 coupled transverse-response certificate."""
from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0642"
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_CONNECTED_BLOCK_COUPLED_TRANSVERSE_RESPONSE_v1.md"
MATTER_PARENT = ROOT / "engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_v1.json"
FIELD_PARENT = ROOT / "engine/results/ftd_0641/ftd_0641_connected_block_independent_field_modes_v1.json"
SHA = "74CA689FEF47322CA54AC9C2A9C412B895CA0CC94BFAB5AEE63404355E031A78"
MATTER_SHA = "AB43D342CFE48BEF452955E56B1EDC34F9EE51911F7D899932E7E542877E6B9A"
FIELD_SHA = "EA24EF12476533DB8395C0E64C1E381A6605662EAA9ED35C1E38D66D560189E6"


def relative(a: float, b: float) -> float:
    return abs(a-b)/max(1e-300, abs(a), abs(b))


def phase(rows: list[dict[str, str]], column: str) -> float:
    q = [float(row[column]) for row in rows]
    numerator = sum(q[i]*(q[i+1]+q[i-1]) for i in range(1, 255))
    denominator = sum(2*q[i]*q[i] for i in range(1, 255))
    return math.acos(max(-1.0, min(1.0, numerator/denominator)))


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    assert hashlib.sha256(MATTER_PARENT.read_bytes()).hexdigest().upper() == MATTER_SHA
    assert hashlib.sha256(FIELD_PARENT.read_bytes()).hexdigest().upper() == FIELD_SHA

    summary = json.loads((RESULT / "ftd_0642_connected_block_coupled_transverse_response_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0642_connected_block_coupled_transverse_response_arms_v1.csv").open()))
    ticks = list(csv.DictReader((RESULT / "ftd_0642_connected_block_coupled_transverse_response_ticks_v1.csv").open()))

    assert summary["protocol_sha256"] == SHA
    assert summary["matter_parent_sha256"] == MATTER_SHA
    assert summary["field_parent_sha256"] == FIELD_SHA
    assert summary["verdict"] == "CONNECTED_BLOCK_COUPLED_TRANSVERSE_WEAK_HYBRID_CONSTRUCTIVE"
    assert summary["production_changed"] is False
    assert summary["arm_count"] == 18 and summary["ticks_each_direction"] == 256
    assert all(summary[key] == 1 for key in (
        "coverage_pass", "execution_pass", "bounded_pass", "coupling_pass",
        "linearity_pass", "weak_hybrid_pass"))
    assert summary["decoupled_pass"] == 0
    assert len(arms) == 18 and len(ticks) == 18*256

    by_label: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in ticks:
        by_label[row["label"]].append(row)
    for rows in by_label.values():
        rows.sort(key=lambda row: int(row["tick"]))
        assert [int(row["tick"]) for row in rows] == list(range(1, 257))

    amap = {(a["family"], int(a["polarization"]), a["kind"]): a for a in arms}
    primary = [a for a in arms if a["kind"] == "primary"]
    assert len(primary) == 6
    for arm in arms:
        assert all(arm[key] == "1" for key in (
            "initialized", "forward", "reverse", "bounded", "sector", "no_hops"))
        assert int(arm["max_multiplicity"]) <= 8
        assert float(arm["min_separation"]) >= 0.9
        assert float(arm["max_center"]) <= 1e-3
        assert float(arm["max_state"]) <= 1e-3
        assert float(arm["max_drift"]) <= 1e-12
        assert float(arm["max_common"]) <= 1e-10
        assert float(arm["recovery"]) <= 1e-10
        rows = by_label[arm["label"]]
        coupled_phase, bare_phase = phase(rows, "field_q"), phase(rows, "bare_q")
        assert abs(coupled_phase-float(arm["phase"])) <= 2e-13
        assert abs(bare_phase-float(arm["bare_phase"])) <= 2e-13
        assert abs(relative(coupled_phase, bare_phase)-float(arm["phase_shift"])) <= 2e-12
        matter_columns = [f"matter_q{i}" for i in range(48)]
        matter_rms = math.sqrt(sum(float(row[c])**2 for row in rows for c in matter_columns)/256)
        assert relative(matter_rms, float(arm["matter_rms"])) <= 2e-13

    for arm in primary:
        assert float(arm["matter_rms"]) > 1e-9
        assert float(arm["phase_shift"]) <= 0.05
        assert float(arm["field_distortion"]) <= 0.25
        assert float(arm["field_leakage"]) <= 0.25

    worst_amplitude = worst_sign = 0.0
    for family in ("100", "110", "111"):
        for polarization in (0, 1):
            full = amap[(family, polarization, "primary")]
            half = amap[(family, polarization, "half")]
            negative = amap[(family, polarization, "negative")]
            ratio = float(full["matter_rms"])/float(half["matter_rms"])
            phase_amplitude = relative(float(full["phase"]), float(half["phase"]))
            phase_sign = relative(float(full["phase"]), float(negative["phase"]))
            assert 1.8 <= ratio <= 2.2
            assert phase_amplitude <= 0.01 and phase_sign <= 0.01
            frows, nrows = by_label[full["label"]], by_label[negative["label"]]
            field_sign = max(abs(float(f["field_q"])+float(n["field_q"])) for f,n in zip(frows,nrows))/1e-7
            numerator = sum((float(f[f"matter_q{i}"])+float(n[f"matter_q{i}"]))**2
                            for f,n in zip(frows,nrows) for i in range(48))
            denominator = sum(float(f[f"matter_q{i}"])**2 for f in frows for i in range(48))
            matter_sign = math.sqrt(numerator/denominator)
            assert field_sign <= 0.1 and matter_sign <= 0.1
            worst_amplitude = max(worst_amplitude, abs(ratio-2.0), phase_amplitude)
            worst_sign = max(worst_sign, phase_sign, field_sign, matter_sign)

    assert abs(worst_amplitude-summary["amplitude_residual"]) <= 2e-12
    assert abs(worst_sign-summary["sign_residual"]) <= 2e-12
    assert abs(max(float(a["phase_shift"]) for a in primary)-summary["worst_phase_shift"]) <= 2e-15
    assert abs(max(float(a["field_distortion"]) for a in primary)-summary["worst_field_distortion"]) <= 2e-15
    assert abs(max(float(a["field_leakage"]) for a in primary)-summary["worst_field_leakage"]) <= 2e-15
    print("FTD-0642 certificate: 18 coupled arms pass; reversible weak-hybrid transverse response confirmed")


if __name__ == "__main__":
    main()
