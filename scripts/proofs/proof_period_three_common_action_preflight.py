"""Independent certificate for the FTD-0717 common-action preflight."""
from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0717"
SUMMARY = RESULT / "ftd_0717_period_three_common_action_preflight_v1.json"
TICKS = RESULT / "ftd_0717_period_three_common_action_ticks_v1.csv"
RUNNER = ROOT / "engine/tests/test_period_three_common_action_preflight.cpp"
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_PERIOD_THREE_COMMON_ACTION_PREFLIGHT_v1.md"
)
PROTOCOL = "BCAE18C3786A02266910F80875DD13FD0E3337A91635A01F83252170B5BD294B"
HASHES = {
    SUMMARY: "292F869D1CA6AF953725C73D25740CACDB7B44FEF4BB44996AA2E76D4F8F7B51",
    TICKS: "20BB5020E3AE41AB5A5E89B289CFFDCD004B761D9419C525C2CE045B030CB1E0",
    RUNNER: "559C84A63F455F0885B3BF75DE48BA4A20BF792B76BA01538EACDCF6077BD923",
    PREREG: PROTOCOL,
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def vector(row: dict[str, str], prefix: str) -> tuple[float, float, float]:
    return tuple(float(row[f"{prefix}_{axis}"]) for axis in "xyz")  # type: ignore[return-value]


def norm(value: tuple[float, float, float]) -> float:
    return math.sqrt(sum(entry * entry for entry in value))


for path, expected in HASHES.items():
    assert sha256(path) == expected, path

summary = json.loads(SUMMARY.read_text(encoding="utf-8"))
assert summary["protocol_sha256"] == PROTOCOL
assert summary["verdict"] == "PERIOD_THREE_MINIMUM_NORM_FIELD_REQUIRES_COUPLED_SELECTION"
assert summary["production_changed"] is False
assert summary["translated_return_pass"] == 1
assert summary["gauss_pass"] == 1
assert summary["energy_pass"] == 0
assert summary["local_momentum_pass"] == 0
assert summary["spline_momentum_pass"] == 0

with TICKS.open(newline="", encoding="utf-8") as handle:
    rows = list(csv.DictReader(handle))
assert len(rows) == 3 and [int(row["tick"]) for row in rows] == [0, 1, 2]

energy = [float(row["total_energy_residual"]) for row in rows]
local = [norm(vector(row, "local_defect")) for row in rows]
spline = [norm(vector(row, "spline_defect")) for row in rows]
gauss = [
    max(float(row["gauss_before"]), float(row["gauss_after"])) for row in rows
]
assert abs(max(energy) - summary["maximum_total_energy_residual"]) <= 1e-15
assert abs(max(local) - summary["maximum_local_momentum_defect"]) <= 1e-15
assert abs(max(spline) - summary["maximum_spline_momentum_defect"]) <= 1e-15
assert abs(max(gauss) - summary["maximum_gauss_residual"]) <= 1e-18
assert max(energy) > 1e-10 and max(local) > 1e-10 and max(spline) > 1e-10
assert max(gauss) <= 1e-10
assert summary["complete_return_residual"] <= 1e-10

matter_changes = [float(row["matter_energy_change"]) for row in rows]
field_changes = [float(row["field_energy_change"]) for row in rows]
assert abs(sum(matter_changes)) <= 1e-12
assert abs(sum(field_changes)) <= 1e-12
assert abs(sum(matter_changes) + sum(field_changes)) <= 1e-12

# Cycle closure is not a per-tick common action: the first and last energy
# exchanges miss by O(1e-1) even though their three-tick sums telescope.
assert abs(matter_changes[0]) / abs(field_changes[0]) > 50.0
assert abs(matter_changes[2]) / abs(field_changes[2]) > 30.0

print("FTD-0717 period-three common-action preflight certificate: PASS")
print(
    f"gauss={max(gauss):.12e} return={summary['complete_return_residual']:.12e} "
    f"energy={max(energy):.12e} local={max(local):.12e} "
    f"spline={max(spline):.12e}"
)
