"""Exact/numerical certificate for FTD-0663 field-band embedding."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MODES = ROOT / "engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_modes_v1.csv"
TRANSFER = ROOT / "engine/results/ftd_0662/ftd_0662_internal_mode_action_transfer_v3.json"
TRANSFER_SHA = "8F791E773C35AB2E85D09EC8BDA67C26D1DED51798578542435DA3AAA8111FE0"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


with MODES.open(newline="") as stream:
    rows = list(csv.DictReader(stream))
row = next(item for item in rows
           if item["orientation"] == "0" and item["mode"] == "6")
phi = float(row["phase"])
assert math.isclose(phi, 1.0911648733663635, rel_tol=0.0, abs_tol=2e-15)

# Omega(k)=2 asin(sqrt(sum sin^2(k_a/2)/3)); full corner gives pi.
band_min = 0.0
band_max = 2.0 * math.asin(1.0)
axis_max = 2.0 * math.asin(1.0 / math.sqrt(3.0))
assert band_min < phi < band_max
assert phi < axis_max
k_star = 2.0 * math.asin(math.sqrt(3.0) * math.sin(phi / 2.0))
assert 0.0 < k_star < math.pi
omega_star = 2.0 * math.asin(math.sin(k_star / 2.0) / math.sqrt(3.0))
assert math.isclose(omega_star, phi, rel_tol=0.0, abs_tol=2e-15)

assert sha256(TRANSFER) == TRANSFER_SHA
transfer = json.loads(TRANSFER.read_text())
assert transfer["verdict"] == "INTERNAL_MODE_DYNAMIC_FIELD_TRANSFER_CONSTRUCTIVE"
assert transfer["transfer_pass"] == 1
assert transfer["dynamic_morphology_pass"] == 1
assert transfer["minimum_dynamic_ratio"] >= 0.05
assert transfer["minimum_far_fraction"] >= 0.10

print("FTD-0663 internal-mode field-band embedding certificate: PASS")
