"""Independent certificate for FTD-0703 deposited-current form factor."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT_DIR = ROOT / "engine/results/ftd_0703"
CSV_PATH = RESULT_DIR / "ftd_0703_connected_bipole_deposited_current_form_factor_v1.csv"
JSON_PATH = RESULT_DIR / "ftd_0703_connected_bipole_deposited_current_form_factor_v1.json"
STATE_PATH = (ROOT / "engine/results/ftd_0638/"
              "ftd_0638_connected_block_analytic_static_refinement_states_v1.csv")
PROTOCOL_SHA = "D68433E89A6DC20FF8649E72782F00D6FF6A96EC1992CAD5807FC10B2E4B196D"
STATE_SHA = "8A717BC9DFE3A43FB21A6B46EF723BD2649D5F1F5BC2174BBA6027D25550214F"
CSV_SHA = "B9F8C7E149DF2BF4758FC397026980E3419EF514DA5CB24817521F586E2272FD"
JSON_SHA = "4A405F9FB87D8C839B3980B36B2145AC54DBB3511CCB0E3E6D29B3DA21A8784B"


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


assert sha(STATE_PATH) == STATE_SHA
assert sha(CSV_PATH) == CSV_SHA
assert sha(JSON_PATH) == JSON_SHA

summary = json.loads(JSON_PATH.read_text())
assert summary["protocol_sha256"] == PROTOCOL_SHA
assert summary["state_sha256"] == STATE_SHA
assert summary["verdict"] == "DEPOSITED_CURRENT_EDGE_SCREENING_PARTIAL"
assert summary["production_changed"] is False
assert summary["row_count"] == 96

with CSV_PATH.open(newline="") as stream:
    rows = list(csv.DictReader(stream))
assert len(rows) == 96

fractions = (2.0/3.0, 0.70, 0.75, 0.80, 0.85, 0.90, 0.95, 1.00)
expected_keys = {
    (axis, sign, scale, round(fraction, 12))
    for axis in range(3) for sign in (-1, 1) for scale in range(2)
    for fraction in fractions
}
observed_keys = {
    (int(row["axis"]), int(row["sign"]), int(row["scale"]),
     round(float(row["k_fraction"]), 12)) for row in rows
}
assert observed_keys == expected_keys

worst_phase = 0.0
worst_power = 0.0
worst_partition = 0.0
for row in rows:
    axis = int(row["axis"])
    kp = float(row["k_parallel"])
    kt = float(row["k_perp"])
    k = [kp, kt, 0.0]
    for _ in range(axis):
        k = [k[2], k[0], k[1]]
    s_value = sum(math.sin(component/2.0)**2 for component in k)
    omega = 2.0*math.asin(math.sqrt(s_value/3.0))
    worst_phase = max(worst_phase, abs(omega-0.5*kp),
                      abs(float(row["phase_residual"])-abs(omega-0.5*kp)))

    coefficient = [
        complex(float(row[f"j{axis_name}_re"]), float(row[f"j{axis_name}_im"]))
        for axis_name in ("x", "y", "z")
    ]
    khat = [2.0*math.sin(component/2.0) for component in k]
    norm2 = sum(component*component for component in khat)
    dot = sum(khat[index]*coefficient[index] for index in range(3))
    longitudinal = [khat[index]*dot/norm2 for index in range(3)]
    transverse = [coefficient[index]-longitudinal[index] for index in range(3)]
    total = sum(abs(value)**2 for value in coefficient)
    trans = sum(abs(value)**2 for value in transverse)
    long = sum(abs(value)**2 for value in longitudinal)
    worst_power = max(worst_power,
        abs(total-float(row["total_power"])),
        abs(trans-float(row["transverse_power"])),
        abs(long-float(row["longitudinal_power"])))
    worst_partition = max(worst_partition, abs(total-trans-long))

assert worst_phase <= 5e-16
assert worst_power <= 2e-16
assert worst_partition <= 2e-16


def selected(axis: int, sign: int, scale: int, fraction: float) -> dict[str, str]:
    return next(row for row in rows
                if int(row["axis"]) == axis and int(row["sign"]) == sign
                and int(row["scale"]) == scale
                and abs(float(row["k_fraction"])-fraction) < 1e-12)


primaries = [selected(axis, 1, 1, fraction)
             for axis in range(3) for fraction in fractions]
collinear = max(float(row["transverse_fraction"]) for row in primaries
                if abs(float(row["k_fraction"])-2.0/3.0) < 1e-12)
edge_power = max(float(row["total_power"]) for row in primaries
                 if abs(float(row["k_fraction"])-1.0) < 1e-12)
edge_fraction = max(abs(float(row["transverse_fraction"])-1.0/3.0)
                    for row in primaries
                    if abs(float(row["k_fraction"])-1.0) < 1e-12)
offedge = min(float(row["transverse_power"]) for row in primaries
              if abs(float(row["k_fraction"])-0.9) < 1e-12)
interior = max(float(row["transverse_power"]) for row in primaries
               if abs(float(row["k_fraction"])-1.0) >= 1e-12)
contrast = interior/edge_power

assert collinear <= 1e-24
assert edge_power <= 1e-7
assert edge_fraction <= 1e-12
assert offedge >= 1e-5
assert contrast >= 100.0
assert math.isclose(edge_power, summary["edge_total_power"],
                    rel_tol=0.0, abs_tol=2e-22)
assert math.isclose(offedge, summary["offedge_transverse_power"],
                    rel_tol=0.0, abs_tol=2e-18)
assert math.isclose(interior, summary["maximum_interior_transverse_power"],
                    rel_tol=0.0, abs_tol=2e-18)
assert math.isclose(contrast, summary["interior_edge_contrast"],
                    rel_tol=0.0, abs_tol=2e-9)

print("FTD-0703 connected-bipole deposited-current certificate: PASS")
print(f"rows={len(rows)} phase={worst_phase:.3e} power={worst_power:.3e}")
print(f"collinear={collinear:.16g} edge={edge_power:.16g}")
print(f"offedge={offedge:.16g} interior={interior:.16g} contrast={contrast:.16g}")
