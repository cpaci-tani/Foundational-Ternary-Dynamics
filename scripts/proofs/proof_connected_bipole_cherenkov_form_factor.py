"""Certificate for FTD-0701 connected-bipole Cherenkov form factor."""

from __future__ import annotations

import cmath
import csv
import hashlib
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
STATE_CSV = (
    ROOT / "engine/results/ftd_0638/"
    "ftd_0638_connected_block_analytic_static_refinement_states_v1.csv"
)
STATE_SHA256 = "8A717BC9DFE3A43FB21A6B46EF723BD2649D5F1F5BC2174BBA6027D25550214F"

assert hashlib.sha256(STATE_CSV.read_bytes()).hexdigest().upper() == STATE_SHA256


def ideal_points() -> list[tuple[float, float, float, int]]:
    result = []
    for x_value, charge in ((-1.5, 1), (-0.5, 1), (0.5, -1), (1.5, -1)):
        for y_value in (-0.5, 0.5):
            for z_value in (-0.5, 0.5):
                result.append((x_value, y_value, z_value, charge))
    return result


def factor(points: list[tuple[float, float, float, int]],
           wavevector: tuple[float, float, float]) -> complex:
    kx, ky, kz = wavevector
    return sum(charge * cmath.exp(-1j * (kx*x_value + ky*y_value + kz*z_value))
               for x_value, y_value, z_value, charge in points)


def closed_form(wavevector: tuple[float, float, float]) -> complex:
    kx, ky, kz = wavevector
    return (16j * math.sin(kx) * math.cos(kx / 2.0)
            * math.cos(ky / 2.0) * math.cos(kz / 2.0))


ideal = ideal_points()
for wavevector in ((0.3, 0.2, 0.1), (1.1, 0.7, 0.4),
                   (2.0, 0.5, 0.0), (math.pi, math.pi/2.0, 0.0)):
    assert abs(factor(ideal, wavevector) - closed_form(wavevector)) < 8e-15

witness = (math.pi, math.pi / 2.0, 0.0)
assert abs(factor(ideal, witness)) < 1e-14

# The edge zero is quadratic in epsilon at fixed transverse wavevector.
for epsilon in (1e-2, 5e-3, 2.5e-3):
    value = abs(closed_form((math.pi-epsilon, math.pi/2.0, 0.0)))
    scaled = value / (epsilon*epsilon)
    assert abs(scaled - 4.0*math.sqrt(2.0)) < 0.05

with STATE_CSV.open(newline="") as stream:
    rows = [row for row in csv.DictReader(stream) if row["orientation"] == "0"]
assert len(rows) == 16
refined = [(float(row["x1"]), float(row["y1"]), float(row["z1"]),
            int(row["charge"])) for row in rows]

edge_amplitude = abs(factor(refined, witness)) / 16.0
edge_power = edge_amplitude * edge_amplitude
assert math.isclose(edge_amplitude, 5.117229666019852e-5,
                    rel_tol=0.0, abs_tol=2e-16)
assert math.isclose(edge_power, 2.618603945479364e-9,
                    rel_tol=0.0, abs_tol=2e-20)

# A fixed off-edge point on the exact v=1/2 resonant curve.
kx = 0.9 * math.pi
sy = 3.0 * math.sin(kx / 4.0) ** 2 - math.sin(kx / 2.0) ** 2
assert 0.0 < sy < 1.0
ky = 2.0 * math.asin(math.sqrt(sy))
s_value = math.sin(kx / 2.0) ** 2 + math.sin(ky / 2.0) ** 2
omega = 2.0 * math.asin(math.sqrt(s_value / 3.0))
assert math.isclose(omega, 0.5*kx, rel_tol=0.0, abs_tol=2e-15)

offedge_amplitude = abs(factor(refined, (kx, ky, 0.0))) / 16.0
offedge_power = offedge_amplitude * offedge_amplitude
assert math.isclose(ky / math.pi, 0.36190382064542864,
                    rel_tol=0.0, abs_tol=2e-16)
assert math.isclose(offedge_amplitude, 0.041001359857794066,
                    rel_tol=0.0, abs_tol=2e-16)
assert math.isclose(offedge_power, 0.0016811115101883266,
                    rel_tol=0.0, abs_tol=2e-18)

print("FTD-0701 connected-bipole Cherenkov form-factor certificate: PASS")
print(f"edge amplitude={edge_amplitude:.16g} power={edge_power:.16g}")
print(f"offedge ky/pi={ky/math.pi:.16g} amplitude={offedge_amplitude:.16g} "
      f"power={offedge_power:.16g}")
