"""Independent certificate for the FTD-0715 period-three momentum lift."""
from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0715"
SUMMARY = RESULT / "ftd_0715_period_three_internal_momentum_lift_v1.json"
SEGMENTS = RESULT / "ftd_0715_period_three_internal_momentum_lift_segments_v1.csv"
IMPULSES = RESULT / "ftd_0715_period_three_internal_momentum_lift_impulses_v1.csv"
RUNNER = ROOT / "engine/tests/test_period_three_internal_momentum_lift.cpp"
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_PERIOD_THREE_INTERNAL_MOMENTUM_LIFT_v1.md"
)
PROTOCOL = "668C2D55EBB59572CE6C1E01928E4AE9A94E0913C964F5E45A69CCC8B5C2B4F9"
HASHES = {
    SUMMARY: "210E0D6D1DCC8DE331B48E99C73E44BF935757E16C6AD005B725ADB15A8A36A9",
    SEGMENTS: "BAB1F8139A06F8FF4E1FA1853CD1F7227E2D534AD605037C33C3E97BA7819A87",
    IMPULSES: "1BEFF09718BE5FC186C6B0582B570BF715FBC9A024ADA2E121DF8C54E627FC88",
    RUNNER: "BD582D463D8B52C4B72E4FB520536A5968F916F70237D1D381D090E68EBC0F95",
    PREREG: PROTOCOL,
}
C = 1.0 / math.sqrt(3.0)
REST = 0.511 * C * C


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def vec(row: dict[str, str], prefix: str) -> tuple[float, float, float]:
    return tuple(
        float(row[f"{prefix}_{axis}"] if f"{prefix}_{axis}" in row else row[f"{prefix}{axis}"])
        for axis in "xyz"
    )  # type: ignore[return-value]


def add(*values: tuple[float, float, float]) -> tuple[float, float, float]:
    return tuple(sum(value[i] for value in values) for i in range(3))  # type: ignore[return-value]


def sub(
    a: tuple[float, float, float], b: tuple[float, float, float]
) -> tuple[float, float, float]:
    return tuple(a[i] - b[i] for i in range(3))  # type: ignore[return-value]


def scale(
    factor: float, value: tuple[float, float, float]
) -> tuple[float, float, float]:
    return tuple(factor * entry for entry in value)  # type: ignore[return-value]


def dot(a: tuple[float, float, float], b: tuple[float, float, float]) -> float:
    return sum(a[i] * b[i] for i in range(3))


def norm(value: tuple[float, float, float]) -> float:
    return math.sqrt(dot(value, value))


def maxdiff(a: tuple[float, float, float], b: tuple[float, float, float]) -> float:
    return max(abs(a[i] - b[i]) for i in range(3))


def energy(momentum: tuple[float, float, float]) -> float:
    return math.sqrt(REST * REST + C * C * dot(momentum, momentum))


def velocity(
    before: tuple[float, float, float], after: tuple[float, float, float]
) -> tuple[float, float, float]:
    factor = C * C / (energy(before) + energy(after))
    return scale(factor, add(before, after))


for path, expected in HASHES.items():
    assert sha256(path) == expected, path

summary = json.loads(SUMMARY.read_text(encoding="utf-8"))
assert summary["protocol_sha256"] == PROTOCOL
assert summary["verdict"] == "PERIOD_THREE_MOMENTUM_LIFT_CONSTRUCTIVE"
assert summary["production_changed"] is False

with SEGMENTS.open(newline="", encoding="utf-8") as handle:
    rows = list(csv.DictReader(handle))
assert len(rows) == 48

by_particle: dict[int, list[dict[str, str]]] = {}
by_tick: dict[int, list[dict[str, str]]] = {0: [], 1: [], 2: []}
maximum_velocity_residual = 0.0
maximum_work_residual = 0.0
maximum_speed = 0.0

for row in rows:
    particle = int(row["particle"])
    tick = int(row["tick"])
    by_particle.setdefault(particle, []).append(row)
    by_tick[tick].append(row)
    before = vec(row, "p_before")
    after = vec(row, "p_after")
    target = vec(row, "target_v")
    computed = velocity(before, after)
    maximum_velocity_residual = max(
        maximum_velocity_residual, maxdiff(computed, target)
    )
    work = dot(computed, sub(after, before))
    maximum_work_residual = max(
        maximum_work_residual, abs((energy(after) - energy(before)) - work)
    )
    maximum_speed = max(maximum_speed, norm(target))

assert sorted(by_particle) == list(range(16))
assert maximum_velocity_residual <= 1e-12
assert maximum_work_residual <= 1e-12
assert maximum_speed <= C + 1e-12

base = (1.0 / 3.0, 0.0, 0.0)
maximum_return = 0.0
maximum_phase = 0.0
maximum_impulse_telescope = 0.0
for particle_rows in by_particle.values():
    particle_rows.sort(key=lambda row: int(row["tick"]))
    targets = [vec(row, "target_v") for row in particle_rows]
    delta = tuple(float(particle_rows[0][f"d{axis}"]) for axis in "xyz")
    assert maxdiff(sub(targets[0], base), delta) <= 1e-15
    assert maxdiff(sub(add(targets[0], targets[1]), scale(2.0, base)), scale(-1.0, delta)) <= 1e-15
    maximum_return = max(
        maximum_return, maxdiff(add(*targets), (1.0, 0.0, 0.0))
    )
    maximum_phase = max(
        maximum_phase, *(norm(sub(target, base)) for target in targets)
    )
    momenta_before = [vec(row, "p_before") for row in particle_rows]
    momenta_after = [vec(row, "p_after") for row in particle_rows]
    assert maxdiff(momenta_after[0], momenta_before[1]) <= 1e-15
    assert maxdiff(momenta_after[1], momenta_before[2]) <= 1e-15
    assert maxdiff(momenta_after[2], momenta_before[0]) <= 1e-15
    impulses = [sub(momenta_after[i], momenta_before[i]) for i in range(3)]
    maximum_impulse_telescope = max(
        maximum_impulse_telescope, norm(add(*impulses))
    )

assert maximum_return <= 1e-14
assert maximum_phase >= 1e-3
assert maximum_impulse_telescope <= 1e-12

for tick, tick_rows in by_tick.items():
    center = scale(1.0 / len(tick_rows), add(*(vec(row, "target_v") for row in tick_rows)))
    assert maxdiff(center, base) <= 1e-13, tick

with IMPULSES.open(newline="", encoding="utf-8") as handle:
    impulse_rows = list(csv.DictReader(handle))
assert len(impulse_rows) == 3
total_impulses = [vec(row, "total_impulse") for row in impulse_rows]
assert norm(add(*total_impulses)) <= 1e-12
assert max(norm(value) for value in total_impulses) > 1e-3

# The local linear reason period three escapes FTD-0714: the cyclic endpoint
# sum map has matrix [[1,1,0],[0,1,1],[1,0,1]], whose determinant is 2.  The
# corresponding two-tick matrix [[1,1],[1,1]] has determinant zero.
det_period_three = 2
det_period_two = 0
assert det_period_three != 0 and det_period_two == 0

assert abs(summary["maximum_velocity_residual"] - maximum_velocity_residual) <= 1e-18
assert abs(summary["maximum_work_residual"] - maximum_work_residual) <= 1e-18
assert abs(summary["maximum_speed"] - maximum_speed) <= 1e-15
assert summary["cubic_covariance_residual"] <= 1e-10
assert summary["mirror_residual"] <= 1e-10

print("FTD-0715 period-three internal-momentum lift certificate: PASS")
print(
    f"velocity={maximum_velocity_residual:.12e} "
    f"work={maximum_work_residual:.12e} "
    f"speed={maximum_speed:.12e} "
    f"tick_impulse={max(norm(value) for value in total_impulses):.12e}"
)
