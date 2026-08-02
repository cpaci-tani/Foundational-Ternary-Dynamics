"""Independent certificate for FTD-0705 moving transverse-field campaign."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0705"
ARMS = RESULT / "ftd_0705_moving_dressed_matter_transverse_field_growth_arms_v1.csv"
FITS = RESULT / "ftd_0705_moving_dressed_matter_transverse_field_growth_fits_v1.csv"
TICKS = RESULT / "ftd_0705_moving_dressed_matter_transverse_field_growth_ticks_v1.csv"
SUMMARY = RESULT / "ftd_0705_moving_dressed_matter_transverse_field_growth_v1.json"
RUNNER = ROOT / "engine/tests/test_moving_dressed_matter_transverse_field_growth.cpp"
PREREG = (ROOT / "docs/theory/10_eft_program/preregistrations/"
          "PREREG_MOVING_DRESSED_MATTER_TRANSVERSE_FIELD_GROWTH_v1.md")

PROTOCOL_SHA = "A60CF2A5E5EE0DFA6903B185D07CACEBDCD8F1D1E57AAC619D1AD6E49B6F18DE"
HASHES = {
    ARMS: "7553E2DF2E6DBC6EA883C4936B1A86E690C35ACBECEBF880C5030B221281CBEC",
    FITS: "8EB2578A4DE182E7AF28B72EC3725CD7B446764C1A52A8E2AA7A2BACE52B199C",
    TICKS: "AE7543645A1F763F8ED593B3D81722F6206F7BEBE9B853D87A087B5883884920",
    SUMMARY: "7FA8699D4C6BCCE24D81D3C76BE0B759E25BA0C7AE15F0E6A6E3237D06890AD4",
    RUNNER: "83504AC80F7C4F29530A9D71683CD9104CCD73C4F83B09ECD24C8E56286D4E67",
    PREREG: PROTOCOL_SHA,
}


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for path, expected in HASHES.items():
    assert sha(path) == expected, path

summary = json.loads(SUMMARY.read_text())
assert summary["protocol_sha256"] == PROTOCOL_SHA
assert summary["verdict"] == (
    "MOVING_DRESSED_MATTER_DYNAMIC_TRANSVERSE_NO_THRESHOLD_SEPARATION")
assert summary["production_changed"] is False
assert summary["volume"] == 65 and summary["ticks_each_direction"] == 24
for gate in ("execution_pass", "coherence_pass", "source_quality_pass",
             "observer_pass", "coupling_pass", "collinear_pass"):
    assert summary[gate] == 1
assert summary["growth45_pass"] == 0 and summary["growth50_pass"] == 0

with ARMS.open(newline="") as stream:
    arms = list(csv.DictReader(stream))
with FITS.open(newline="") as stream:
    fits = list(csv.DictReader(stream))
with TICKS.open(newline="") as stream:
    ticks = list(csv.DictReader(stream))
assert len(arms) == 3 and len(fits) == 12 and len(ticks) == 288
assert {round(float(row["speed"]), 12) for row in arms} == {0.35, 0.45, 0.50}

for arm in arms:
    assert all(int(arm[key]) == 1 for key in
               ("initialized", "forward", "reverse", "coherent",
                "source_quality", "observer"))
    assert int(arm["max_multiplicity"]) <= 8
    separation = float(arm["min_separation"])
    assert not math.isfinite(separation) or separation >= 0.9
    assert float(arm["max_shape"]) <= 0.05
    assert float(arm["max_strain"]) <= 0.05
    assert float(arm["max_transverse"]) <= 1e-8
    assert float(arm["max_energy_drift"]) <= 1e-10
    assert float(arm["max_common"]) <= 1e-10
    assert float(arm["recovery"]) <= 1e-9
    assert abs(float(arm["mean_speed"])-float(arm["speed"])) <= 0.05
    assert float(arm["increment_cv"]) <= 0.15

modes = {"R45": (31, 9, 0), "R50": (24, 5, 0),
         "C45": (26, 0, 0), "C50": (22, 0, 0)}
by_key: dict[tuple[float, str], list[dict[str, str]]] = {}
worst_dispersion = 0.0
worst_projection = 0.0
radial_rows = 0
for row in ticks:
    speed = round(float(row["speed"]), 12)
    label = row["label"]
    assert (int(row["nx"]), int(row["ny"]), int(row["nz"])) == modes[label]
    nx, ny, nz = modes[label]
    k = [2.0*math.pi*value/65.0 for value in (nx, ny, nz)]
    omega = 2.0*math.asin(math.sqrt(sum(math.sin(value/2.0)**2
                                         for value in k)/3.0))
    detuning = abs(omega-speed*k[0])
    worst_dispersion = max(worst_dispersion,
                           abs(omega-float(row["omega"])),
                           abs(detuning-float(row["detuning"])))
    worst_projection = max(worst_projection,
                           float(row["field_projection_residual"]),
                           float(row["current_projection_residual"]))
    assert float(row["increment"]) > 0.0
    far = float(row["magnetic_far_fraction_r6"])
    if int(row["tick"]) in (8, 16, 24):
        assert 0.0 <= far <= 1.0
        radial_rows += 1
    else:
        assert math.isnan(far)
    by_key.setdefault((speed, label), []).append(row)
assert worst_dispersion <= 5e-16
assert worst_projection <= 1e-12
assert radial_rows == 3*3*4


def recompute_fit(rows: list[dict[str, str]]) -> tuple[float, float, float, float, float]:
    late = sorted((row for row in rows if int(row["tick"]) >= 9),
                  key=lambda row: int(row["tick"]))
    assert len(late) == 16
    times = [float(row["tick"]) for row in late]
    values = [complex(float(row["electric_real"]),
                      float(row["electric_imag"])) for row in late]
    currents = [abs(complex(float(row["current_real"]),
                            float(row["current_imag"]))) for row in late]
    mean_t = sum(times)/len(times)
    mean_z = sum(values)/len(values)
    denominator = sum((time-mean_t)**2 for time in times)
    slope_complex = sum((time-mean_t)*(value-mean_z)
                        for time, value in zip(times, values))/denominator
    intercept = mean_z-slope_complex*mean_t
    residual = sum(abs(value-intercept-slope_complex*time)**2
                   for time, value in zip(times, values))
    total = sum(abs(value-mean_z)**2 for value in values)
    slope = abs(slope_complex)
    r_squared = 1.0-residual/total if total > 0.0 else 0.0
    amplitude = abs(values[-1])/max(1e-300, abs(values[0]))
    mean_current = sum(currents)/len(currents)
    response = slope/mean_current if mean_current > 0.0 else 0.0
    return slope, r_squared, amplitude, mean_current, response


fit_map = {(round(float(row["speed"]), 12), row["label"]): row for row in fits}
for key, rows in by_key.items():
    observed = tuple(float(fit_map[key][name]) for name in
                     ("slope", "r_squared", "amplitude_ratio",
                      "mean_current", "response"))
    expected = recompute_fit(rows)
    for actual, recomputed in zip(observed, expected):
        assert math.isclose(actual, recomputed, rel_tol=2e-14, abs_tol=2e-15), (
            key, observed, expected)

r45 = tuple(float(fit_map[(0.45, "R45")][key]) for key in
            ("slope", "r_squared", "amplitude_ratio", "mean_current", "response"))
r50 = tuple(float(fit_map[(0.50, "R50")][key]) for key in
            ("slope", "r_squared", "amplitude_ratio", "mean_current", "response"))
b45 = float(fit_map[(0.35, "R45")]["response"])
b50 = float(fit_map[(0.35, "R50")]["response"])
c45_slope = float(fit_map[(0.45, "C45")]["slope"])
c50_slope = float(fit_map[(0.50, "C50")]["slope"])
k45 = c45_slope/r45[3]
k50 = c50_slope/r50[3]
assert math.isclose(summary["Q45"], r45[4], rel_tol=0.0, abs_tol=2e-16)
assert math.isclose(summary["Q50"], r50[4], rel_tol=0.0, abs_tol=2e-16)
assert math.isclose(summary["B45"], b45, rel_tol=0.0, abs_tol=2e-16)
assert math.isclose(summary["B50"], b50, rel_tol=0.0, abs_tol=2e-16)
assert math.isclose(summary["K45"], k45, rel_tol=0.0, abs_tol=2e-24)
assert math.isclose(summary["K50"], k50, rel_tol=0.0, abs_tol=2e-24)
assert not (r45[1] >= 0.80 and r45[2] >= 2.0
            and r45[4] >= 5.0*max(b45, k45))
assert not (r50[1] >= 0.80 and r50[2] >= 2.0
            and r50[4] >= 5.0*max(b50, k50))

collinear45 = float(by_key[(0.45, "C45")][-1]["current_transverse_fraction"])
collinear50 = float(by_key[(0.50, "C50")][-1]["current_transverse_fraction"])
assert collinear45 <= 1e-20 and collinear50 <= 1e-20

print("FTD-0705 moving transverse-field certificate: PASS")
print(f"rows={len(ticks)} dispersion={worst_dispersion:.3e} projection={worst_projection:.3e}")
print(f"Q45/B45={r45[4]/b45:.6f} R2={r45[1]:.6f} amp={r45[2]:.6f}")
print(f"Q50/B50={r50[4]/b50:.6f} R2={r50[1]:.6f} amp={r50[2]:.6f}")
