"""Independent certificate for FTD-0704 high-speed dressed-matter preflight."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0704"
ARMS = RESULT / "ftd_0704_connected_dressed_matter_high_speed_preflight_arms_v1.csv"
TICKS = RESULT / "ftd_0704_connected_dressed_matter_high_speed_preflight_ticks_v1.csv"
SUMMARY = RESULT / "ftd_0704_connected_dressed_matter_high_speed_preflight_v1.json"
RUNNER = ROOT / "engine/tests/test_connected_dressed_matter_high_speed_preflight.cpp"
PREREG = (ROOT / "docs/theory/10_eft_program/preregistrations/"
          "PREREG_CONNECTED_DRESSED_MATTER_HIGH_SPEED_PREFLIGHT_v1.md")

PROTOCOL_SHA = "E70EC4DA01504CA929A710482ACE6CCAEAB09075951505EF7F0ECD1D6B374E5E"
HASHES = {
    ARMS: "2ED532A2A362740BB2C808122F83E86B2D6AC054E5AAA10505CB11EAA163E466",
    TICKS: "3DFF61320C61E625DA88601DAA7E079A02E2A9B190D849E919C81C21E1BDAD3D",
    SUMMARY: "E28B979ADF346B2CDB8159F01ADC94D2A61C71214D9A06DD85BD0D872FC8EC4F",
    RUNNER: "6D27AD118D0224B03BE9B5966B6FECD44BFD62765C9F56DBD70493E43C03B257",
    PREREG: PROTOCOL_SHA,
}


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for path, expected in HASHES.items():
    assert sha(path) == expected, path

summary = json.loads(SUMMARY.read_text())
assert summary["protocol_sha256"] == PROTOCOL_SHA
assert summary["verdict"] == "DRESSED_MATTER_HIGH_SPEED_PREFLIGHT_CONSTRUCTIVE"
assert summary["production_changed"] is False
assert summary["volume"] == 33 and summary["ticks_each_direction"] == 8
for gate in ("coverage_pass", "execution_pass", "coherence_pass",
             "source_quality_pass", "mirror_pass"):
    assert summary[gate] == 1

with ARMS.open(newline="") as stream:
    arms = list(csv.DictReader(stream))
with TICKS.open(newline="") as stream:
    ticks = list(csv.DictReader(stream))
assert len(arms) == 6 and len(ticks) == 48

expected = {(speed, sign) for speed in (0.35, 0.45, 0.50)
            for sign in (-1, 1)}
observed = {(round(float(row["target_speed"]), 12), int(row["sign"]))
            for row in arms}
assert observed == expected

for arm in arms:
    assert all(int(arm[key]) == 1 for key in
               ("initialized", "forward", "reverse", "coherent",
                "source_quality"))
    assert int(arm["total_hops"]) >= 16
    assert int(arm["max_multiplicity"]) <= 8
    separation = float(arm["min_separation"])
    assert not math.isfinite(separation) or separation >= 0.9
    assert float(arm["max_shape"]) <= 0.05
    assert float(arm["max_strain"]) <= 0.05
    assert float(arm["max_transverse"]) <= 1e-8
    assert float(arm["max_energy_drift"]) <= 1e-10
    assert float(arm["max_common"]) <= 1e-10
    assert float(arm["recovery"]) <= 1e-9
    assert abs(float(arm["mean_speed"])-float(arm["target_speed"])) <= 0.05
    assert float(arm["increment_cv"]) <= 0.15

by_label: dict[str, list[dict[str, str]]] = {}
for row in ticks:
    by_label.setdefault(row["label"], []).append(row)
for rows in by_label.values():
    rows.sort(key=lambda row: int(row["tick"]))
    assert [int(row["tick"]) for row in rows] == list(range(1, 9))
    assert all(float(row["axial_increment"]) > 0.0 for row in rows)

mirror = 0.0
for speed in (0.35, 0.45, 0.50):
    positive = by_label[f"p{speed:.6f}"]
    negative = by_label[f"n{speed:.6f}"]
    for p_row, n_row in zip(positive, negative):
        for key in ("axial_displacement", "axial_increment",
                    "mean_axial_velocity", "axial_momentum", "shape",
                    "strain", "field_energy"):
            mirror = max(mirror, abs(float(p_row[key])-float(n_row[key])))
assert mirror <= 1e-6
assert math.isclose(mirror, summary["mirror_residual"],
                    rel_tol=0.0, abs_tol=2e-15)

print("FTD-0704 dressed-matter high-speed preflight certificate: PASS")
print(f"arms={len(arms)} ticks={len(ticks)} mirror={mirror:.3e}")
print("speeds=" + ",".join(
    f"{float(row['mean_speed']):.12f}" for row in arms if int(row["sign"]) > 0))

