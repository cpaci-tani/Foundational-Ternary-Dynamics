"""Independent certificate for the locked FTD-0699 symmetry-ray spectrum."""

from __future__ import annotations

import csv
import hashlib
import json
import math
import statistics
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DIRECTORY = ROOT / "engine/results/ftd_0699"
RESULT = DIRECTORY / "ftd_0699_internal_excitation_symmetry_ray_spectrum_v2.json"
TICKS = DIRECTORY / "ftd_0699_internal_excitation_symmetry_ray_spectrum_ticks_v2.csv"
PARENT = DIRECTORY / "ftd_0699_internal_excitation_symmetry_ray_parent_v2.json"
PROTOCOL = "C1609A6060C5148A0D5B4B6334B862E2212C2C55B22579A25BC34858F7610858"
EXPECTED_HASHES = {
    RESULT: "3FC06F519817779FCFE83B3D945D8A349AC31B4D7F17329C3DD0F3D86E34DB71",
    TICKS: "F5640C3567A2F4BDD844DF014835EEC13E9DD73A69EC427B333130518346C020",
    PARENT: "A2713A7B0F00EB2C5E9A1BD25F9EA7A4CB089FF1610E904B278B7D89D63D638C",
}
L = 113
HORIZON = 96
HARMONICS = 56
PHASE = 1.0911648733663635
C2 = 1.0 / 3.0
RAYS = ((1, 0, 0), (1, 1, 0), (1, 1, 1))


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for path, expected in EXPECTED_HASHES.items():
    assert sha256(path) == expected

result = json.loads(RESULT.read_text())
parent = json.loads(PARENT.read_text())
assert result["protocol_sha256"] == PROTOCOL
assert result["verdict"] == "SYMMETRY_RAY_RESONANT_TRANSFER_CONSTRUCTIVE"
assert result["execution_pass"] == 1
assert result["row_count"] == (HORIZON + 1) * 2 * 3 * HARMONICS
assert math.isclose(result["internal_phase"], PHASE, rel_tol=0.0, abs_tol=2e-15)
assert result["maximum_projection_residual"] <= 1e-12
assert result["sign_field_residual"] <= 1e-4
assert result["sign_current_residual"] <= 1e-4
assert parent["parent_pass"] is True
assert parent["negative_exact"] is True and parent["positive_exact"] is True
assert parent["verdict"] == "DISTRIBUTED_FIELD_MIXED + CONTINUING_EXCITATION_TRANSFER"
assert max(parent["negative_recovery"], parent["positive_recovery"]) <= 1e-8

field_weight: dict[tuple[int, int, int], float] = defaultdict(float)
current_weight: dict[tuple[int, int, int], float] = defaultdict(float)
frequencies: dict[tuple[int, int], float] = {}
rows_seen = 0
maximum_power_residual = 0.0
maximum_frequency_residual = 0.0

with TICKS.open(newline="") as stream:
    for row in csv.DictReader(stream):
        tick = int(row["tick"])
        sign = int(row["sign"])
        ray = int(row["ray"])
        harmonic = int(row["harmonic"])
        expected_index = rows_seen
        expected_tick = expected_index // (2 * 3 * HARMONICS)
        within_tick = expected_index % (2 * 3 * HARMONICS)
        expected_sign = -1 if within_tick < 3 * HARMONICS else 1
        sign_offset = 0 if expected_sign < 0 else 3 * HARMONICS
        expected_ray = (within_tick - sign_offset) // HARMONICS
        expected_harmonic = (within_tick - sign_offset) % HARMONICS + 1
        assert (tick, sign, ray, harmonic) == (
            expected_tick,
            expected_sign,
            expected_ray,
            expected_harmonic,
        )

        direction = RAYS[ray]
        s_value = sum(
            math.sin(math.pi * harmonic * component / L) ** 2
            for component in direction
        )
        omega = 2.0 * math.asin(math.sqrt(s_value / 3.0))
        recorded_omega = float(row["omega"])
        maximum_frequency_residual = max(
            maximum_frequency_residual, abs(recorded_omega - omega)
        )
        assert abs(recorded_omega - omega) <= 2e-15
        frequencies[ray, harmonic] = recorded_omega

        e = [
            complex(float(row[f"et{axis}_re"]), float(row[f"et{axis}_im"]))
            for axis in "xyz"
        ]
        b = [
            complex(float(row[f"bt{axis}_re"]), float(row[f"bt{axis}_im"]))
            for axis in "xyz"
        ]
        current = [
            complex(float(row[f"k{axis}_re"]), float(row[f"k{axis}_im"]))
            for axis in "xyz"
        ]
        reconstructed_field = sum(abs(value) ** 2 for value in e) + C2 * sum(
            abs(value) ** 2 for value in b
        )
        reconstructed_current = sum(abs(value) ** 2 for value in current)
        recorded_field = float(row["field_transverse"])
        recorded_current = float(row["current_total"])
        maximum_power_residual = max(
            maximum_power_residual,
            abs(reconstructed_field - recorded_field),
            abs(reconstructed_current - recorded_current),
        )
        assert math.isclose(
            reconstructed_field, recorded_field, rel_tol=3e-14, abs_tol=1e-34
        )
        assert math.isclose(
            reconstructed_current, recorded_current, rel_tol=3e-14, abs_tol=1e-34
        )
        assert float(row["projection_residual"]) <= 1e-12
        if tick > 0:
            field_weight[sign, ray, harmonic] += recorded_field
            current_weight[sign, ray, harmonic] += recorded_current
        rows_seen += 1

assert rows_seen == result["row_count"]

recomputed = []
maximum_sign_field = 0.0
maximum_sign_current = 0.0
for ray in range(3):
    maximum_current = {
        sign: max(current_weight[sign, ray, n] for n in range(1, HARMONICS + 1))
        for sign in (-1, 1)
    }
    for sign in (-1, 1):
        eligible = [
            n
            for n in range(1, HARMONICS + 1)
            if current_weight[sign, ray, n] >= 1e-6 * maximum_current[sign]
        ]
        response = {
            n: field_weight[sign, ray, n] / current_weight[sign, ray, n]
            for n in eligible
        }
        peak = max(eligible, key=response.get)
        closest = min(
            range(1, HARMONICS + 1),
            key=lambda n: abs(frequencies[ray, n] - PHASE),
        )
        spacings = []
        if closest > 1:
            spacings.append(
                abs(frequencies[ray, closest] - frequencies[ray, closest - 1])
            )
        if closest < HARMONICS:
            spacings.append(
                abs(frequencies[ray, closest + 1] - frequencies[ray, closest])
            )
        allowed = max(spacings)
        detuning = abs(frequencies[ray, peak] - PHASE)
        contrast = response[peak] / statistics.median(response.values())
        assert len(eligible) >= 8
        assert detuning <= allowed
        assert contrast >= 5.0
        recomputed.append((sign, ray, closest, peak, detuning, allowed, contrast))

    for harmonic in range(1, HARMONICS + 1):
        if (
            current_weight[-1, ray, harmonic] < 1e-6 * maximum_current[-1]
            or current_weight[1, ray, harmonic] < 1e-6 * maximum_current[1]
        ):
            continue
        negative_field = field_weight[-1, ray, harmonic]
        positive_field = field_weight[1, ray, harmonic]
        negative_current = current_weight[-1, ray, harmonic]
        positive_current = current_weight[1, ray, harmonic]
        maximum_sign_field = max(
            maximum_sign_field,
            abs(negative_field - positive_field)
            / max(abs(negative_field), abs(positive_field), 1e-300),
        )
        maximum_sign_current = max(
            maximum_sign_current,
            abs(negative_current - positive_current)
            / max(abs(negative_current), abs(positive_current), 1e-300),
        )

assert math.isclose(
    maximum_sign_field, result["sign_field_residual"], rel_tol=2e-14, abs_tol=1e-18
)
assert math.isclose(
    maximum_sign_current,
    result["sign_current_residual"],
    rel_tol=2e-14,
    abs_tol=1e-18,
)

for negative, positive in zip(recomputed[0::2], recomputed[1::2]):
    assert negative[1] == positive[1]
    assert abs(negative[3] - positive[3]) <= 1

print("FTD-0699 internal-excitation symmetry-ray spectrum certificate: PASS")
print(f"rows={rows_seen} max_frequency_residual={maximum_frequency_residual:.3e}")
print(f"max_power_residual={maximum_power_residual:.3e}")
print(f"sign_field={maximum_sign_field:.16g} sign_current={maximum_sign_current:.16g}")
for sign, ray, closest, peak, detuning, allowed, contrast in recomputed:
    print(
        f"sign={sign:+d} ray={ray} closest={closest} peak={peak} "
        f"detuning={detuning:.16g} allowed={allowed:.16g} contrast={contrast:.16g}"
    )
