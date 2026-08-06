"""Independent certificate for the FTD-0714 two-tick gait obstruction."""
from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0713"
SUMMARY = RESULT / "ftd_0713_causal_bound_internal_gait_continuation_v1.json"
STATE = RESULT / "ftd_0713_causal_bound_internal_gait_state_v1.csv"
RUNNER = ROOT / "engine/tests/test_causal_bound_internal_gait_continuation.cpp"
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_CAUSAL_BOUND_INTERNAL_GAIT_CONTINUATION_v1.md"
)
PROTOCOL = "901F2F2FDACEB47D62ED57EE0E4E114B1C4C29C6DF7F8188EA39E86F3DC724BF"
HASHES = {
    SUMMARY: "E32B537808A128B4B080FE2EC6B42C4DF5E494F87E9D1EF09CB86CBB88DFE051",
    STATE: "6C9B7684DBEB2976823B2A0B908407ED201253E4A6ED22D1F83B053712C4ACDF",
    RUNNER: "B4A0CB9824A10EAC2FCCF005A43430F5949D16C2093A742B903C2A703BFF08D1",
    PREREG: PROTOCOL,
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for path, expected in HASHES.items():
    assert sha256(path) == expected, path

summary = json.loads(SUMMARY.read_text(encoding="utf-8"))
assert summary["protocol_sha256"] == PROTOCOL
assert summary["verdict"] == "CAUSAL_INTERNAL_GAIT_CANCELLATION_CONSTRUCTIVE"
assert summary["root_pass"] == 1 and summary["production_changed"] is False

with STATE.open(newline="", encoding="utf-8") as handle:
    rows = list(csv.DictReader(handle))
assert len(rows) == 16
deltas = [tuple(float(row[key]) for key in ("dx", "dy", "dz")) for row in rows]
center = tuple(sum(delta[axis] for delta in deltas) for axis in range(3))
assert max(abs(value) for value in center) <= 1e-14
maximum_delta = max(abs(value) for delta in deltas for value in delta)
assert maximum_delta == summary["maximum_displacement"]
assert maximum_delta > 1e-9

first = [(0.5 + dx, dy, dz) for dx, dy, dz in deltas]
second = [(0.5 - dx, -dy, -dz) for dx, dy, dz in deltas]
increment_difference = max(
    abs(first[a][axis] - second[a][axis])
    for a in range(16)
    for axis in range(3)
)
assert abs(increment_difference - 2.0 * maximum_delta) <= 1e-15

# Any common vector w approximating two vectors a,b obeys
# max(||w-a||_inf,||w-b||_inf) >= ||a-b||_inf/2 by the triangle inequality.
minimum_equal_increment_error = 0.5 * increment_difference
assert abs(minimum_equal_increment_error - maximum_delta) <= 1e-15
assert minimum_equal_increment_error > 1e-9

# Direct numerical sanity check of the exact endpoint symmetry V(p,q)=V(q,p).
c2 = 1.0 / 3.0
rest = 0.511


def energy(p: tuple[float, float, float]) -> float:
    return math.sqrt(rest * rest + c2 * sum(value * value for value in p))


def velocity(
    p: tuple[float, float, float], q: tuple[float, float, float]
) -> tuple[float, float, float]:
    denominator = energy(p) + energy(q)
    return tuple(c2 * (p[i] + q[i]) / denominator for i in range(3))


samples = [
    ((0.1, -0.2, 0.3), (-0.4, 0.5, 0.2)),
    ((1.0, 0.0, 0.0), (0.0, 1.0, 0.0)),
    ((-0.7, 0.25, -0.1), (0.33, -0.4, 0.9)),
]
symmetry_residual = max(
    abs(a - b)
    for p, q in samples
    for a, b in zip(velocity(p, q), velocity(q, p))
)
assert symmetry_residual == 0.0

print("FTD-0714 two-tick reciprocal-gait obstruction certificate: PASS")
print(
    f"max_delta={maximum_delta:.12e} "
    f"equal_increment_error_lower_bound={minimum_equal_increment_error:.12e}"
)
