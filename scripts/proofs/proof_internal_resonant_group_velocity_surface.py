"""Exact/numerical certificate for FTD-0695 group-velocity surface."""

from __future__ import annotations

import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MODES = (
    ROOT
    / "engine/results/ftd_0640/"
    "ftd_0640_connected_block_analytic_matter_modes_modes_v1.csv"
)


with MODES.open(newline="") as stream:
    rows = list(csv.DictReader(stream))
row = next(
    item for item in rows if item["orientation"] == "0" and item["mode"] == "6"
)
phi = float(row["phase"])
assert math.isclose(phi, 1.0911648733663635, rel_tol=0.0, abs_tol=2e-15)

s_star = 3.0 * math.sin(phi / 2.0) ** 2
assert 0.0 < s_star < 1.0
assert math.isclose(s_star, 0.8078216321246361, rel_tol=0.0, abs_tol=2e-15)

expected = {
    1: (2.233998332573721, 0.2960835685214112),
    2: (1.377414916810145, 0.5214560095416422),
    3: (1.091164873366363, 1.0 / math.sqrt(3.0)),
}

for dimension, (expected_q, expected_speed) in expected.items():
    q = 2.0 * math.asin(math.sqrt(s_star / dimension))
    omega = 2.0 * math.asin(
        math.sqrt(dimension * math.sin(q / 2.0) ** 2 / 3.0)
    )
    speed = (
        math.sqrt(dimension)
        * math.sin(q)
        / (2.0 * math.sqrt(s_star * (3.0 - s_star)))
    )
    assert math.isclose(q, expected_q, rel_tol=0.0, abs_tol=2e-15)
    assert math.isclose(omega, phi, rel_tol=0.0, abs_tol=2e-15)
    assert math.isclose(speed, expected_speed, rel_tol=0.0, abs_tol=2e-15)

# Along the body diagonal, Omega(q,q,q)=q and |grad Omega|=1/sqrt(3).
for q in (0.1, 0.4, 0.9, phi, 2.0, 3.0):
    omega = 2.0 * math.asin(math.sin(q / 2.0))
    assert math.isclose(omega, q, rel_tol=0.0, abs_tol=2e-15)
    s_value = 3.0 * math.sin(q / 2.0) ** 2
    speed = (
        math.sqrt(3.0)
        * math.sin(q)
        / (2.0 * math.sqrt(s_value * (3.0 - s_value)))
    )
    assert math.isclose(speed, 1.0 / math.sqrt(3.0), rel_tol=0.0, abs_tol=3e-15)

print("FTD-0695 internal-resonant group-velocity certificate: PASS")
print(f"phi_int={phi:.16g} S_star={s_star:.16g}")
for dimension in (1, 2, 3):
    q, speed = expected[dimension]
    print(f"d={dimension} q={q:.16g} q/pi={q / math.pi:.16g} |vg|={speed:.16g}")
