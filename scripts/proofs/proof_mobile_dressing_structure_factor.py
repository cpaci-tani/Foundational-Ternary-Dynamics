"""Independent raw-data and protocol-nonconformance certificate for FTD-0655."""

from __future__ import annotations

import cmath
import csv
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0655"
PROTOCOL = "09523E64E273E7808FF21A446B26C012531931EF948F3F48F090D4F851C0F2A0"


def unwrap(values: list[float]) -> list[float]:
    result = [values[0]]
    for current, previous in zip(values[1:], values[:-1]):
        delta = current - previous
        while delta > math.pi:
            delta -= 2.0 * math.pi
        while delta < -math.pi:
            delta += 2.0 * math.pi
        result.append(result[-1] + delta)
    return result


def fit(values: list[complex], k: float) -> tuple[float, float, float, float]:
    amplitudes = [abs(value) for value in values]
    mean_amplitude = sum(amplitudes) / len(amplitudes)
    cv = math.sqrt(sum((value - mean_amplitude) ** 2 for value in amplitudes)
                   / len(amplitudes)) / mean_amplitude
    theta = unwrap([cmath.phase(value) for value in values])
    mean_t = 0.5 * (len(theta) - 1)
    mean_theta = sum(theta) / len(theta)
    denominator = sum((index - mean_t) ** 2 for index in range(len(theta)))
    slope = sum((index - mean_t) * (value - mean_theta)
                for index, value in enumerate(theta)) / denominator
    phase_rms = math.sqrt(sum((value - mean_theta
                               - slope * (index - mean_t)) ** 2
                              for index, value in enumerate(theta))
                          / len(theta))
    return mean_amplitude, cv, phase_rms, -slope / k


def family_data(label: str, family: str) -> tuple[tuple[float, float, float], int]:
    if family == "110":
        return ((1 / math.sqrt(2), 1 / math.sqrt(2), 0), 2)
    if family == "111":
        return ((1 / math.sqrt(3),) * 3, 3)
    if label.endswith("_o1"):
        return ((0, 1, 0), 1)
    if label.endswith("_o2"):
        return ((0, 0, 1), 1)
    return ((1, 0, 0), 1)


def close(lhs: float, rhs: float, tolerance: float = 2e-12) -> None:
    assert abs(lhs - rhs) <= tolerance * max(1.0, abs(lhs), abs(rhs))


def main() -> None:
    summary = json.loads((RESULT / "ftd_0655_mobile_dressing_structure_factor_v1.json").read_text())
    assert summary["protocol_sha256"] == PROTOCOL
    assert summary["verdict"] == "MOBILE_DRESSED_STRUCTURE_FACTOR_CONSTRUCTIVE"
    assert summary["arm_count"] == 18
    for gate in ("coverage_pass", "execution_pass", "exact_pass",
                 "coherence_pass", "matter_pass", "field_pass",
                 "mirror_pass", "cubic_pass", "width_trend_pass"):
        assert summary[gate] == 1

    with (RESULT / "ftd_0655_mobile_dressing_structure_factor_arms_v1.csv").open(newline="") as handle:
        arms = {row["label"]: row for row in csv.DictReader(handle)}
    assert len(arms) == 18

    series: dict[str, list[dict[str, str]]] = defaultdict(list)
    with (RESULT / "ftd_0655_mobile_dressing_structure_factor_series_v1.csv").open(newline="") as handle:
        for row in csv.DictReader(handle):
            series[row["label"]].append(row)
    assert set(series) == set(arms)

    by_width: dict[int, dict[str, float]] = {
        width: {"mismatch": 0.0, "relative": 0.0, "field_cv": 0.0}
        for width in (2, 3, 4)
    }
    metrics: dict[str, dict[str, float]] = {}
    for label, arm in arms.items():
        width = int(arm["width"])
        rows = sorted(series[label], key=lambda row: int(row["tick"]))
        # The executable followed T_phys/a = 32w, but the locked arm section
        # literally required 64w.  This proves the execution invalid even
        # though the raw numerical gates below are reproducible.
        assert len(rows) == 32 * width + 1
        assert len(rows) != 64 * width + 1
        assert [int(row["tick"]) for row in rows] == list(range(32 * width + 1))
        direction, squared_norm = family_data(label, arm["family"])
        k = 2.0 * math.pi * math.sqrt(squared_norm) / (8 * width + 1)
        matter_values = [complex(float(row["matter_real"]), float(row["matter_imag"]))
                         for row in rows]
        field_values = [complex(float(row["field_real"]), float(row["field_imag"]))
                        for row in rows]
        matter_mean, matter_cv, matter_rms, matter_velocity = fit(matter_values, k)
        field_mean, field_cv, field_rms, field_velocity = fit(field_values, k)
        relative = unwrap([cmath.phase(field / matter)
                           for field, matter in zip(field_values, matter_values)])
        relative_mean = sum(relative) / len(relative)
        relative_rms = math.sqrt(sum((value - relative_mean) ** 2 for value in relative)
                                 / len(relative))
        first, last = rows[0], rows[-1]
        displacement = tuple(float(last[f"center_{axis}"])
                             - float(first[f"center_{axis}"])
                             for axis in "xyz")
        center_velocity = (2.0 / width) * sum(a * b for a, b in zip(displacement, direction)) / 64.0
        mismatch_center = abs(matter_velocity - center_velocity) / 0.03
        mismatch_field = abs(field_velocity - matter_velocity) / 0.03

        for name, value in (("matter_mean", matter_mean), ("field_mean", field_mean),
                            ("matter_cv", matter_cv), ("field_cv", field_cv),
                            ("matter_phase_rms", matter_rms),
                            ("field_phase_rms", field_rms),
                            ("matter_velocity", matter_velocity),
                            ("field_velocity", field_velocity),
                            ("center_velocity", center_velocity),
                            ("relative_phase_rms", relative_rms),
                            ("matter_center_mismatch", mismatch_center),
                            ("field_matter_mismatch", mismatch_field)):
            close(value, float(arm[name]))

        assert matter_mean > 1e-8 and field_mean > 1e-12
        assert matter_rms < 0.10 and field_rms < 0.20
        assert matter_cv < 0.10 and field_cv < 0.20
        assert relative_rms < 0.20
        assert mismatch_center < 0.10 and mismatch_field < 0.10
        assert arm["initialized"] == arm["forward"] == arm["reverse"] == "1"
        assert arm["exact"] == arm["coherent"] == "1"
        assert float(arm["max_action"]) <= 1e-9
        assert float(arm["max_strain"]) <= 0.10
        assert float(arm["recovery"]) <= 1e-7
        by_width[width]["mismatch"] = max(by_width[width]["mismatch"], mismatch_field)
        by_width[width]["relative"] = max(by_width[width]["relative"], relative_rms)
        by_width[width]["field_cv"] = max(by_width[width]["field_cv"], field_cv)
        metrics[label] = {"matter_velocity": matter_velocity,
                          "field_velocity": field_velocity,
                          "center_velocity": center_velocity,
                          "matter_rms": matter_rms, "field_rms": field_rms,
                          "matter_cv": matter_cv, "field_cv": field_cv,
                          "relative": relative_rms}

    mirror_residual = cubic_residual = 0.0
    for width in (2, 3, 4):
        primary = metrics[f"p_w{width}_v03_100"]
        mirror = metrics[f"m_w{width}_v03_100"]
        mirror_residual = max(mirror_residual,
                              *(abs(primary[name] + mirror[name])
                                for name in ("matter_velocity", "field_velocity",
                                             "center_velocity")))
        for suffix in ("o1", "o2"):
            cubic = metrics[f"c_w{width}_{suffix}"]
            cubic_residual = max(cubic_residual,
                                 *(abs(primary[name] - cubic[name])
                                   for name in primary))
    assert mirror_residual < 1e-8 and cubic_residual < 1e-8
    close(mirror_residual, summary["mirror_residual"])
    close(cubic_residual, summary["cubic_residual"])

    for metric, json_stem in (("mismatch", "velocity_mismatch"),
                              ("relative", "relative_phase_rms"),
                              ("field_cv", "field_cv")):
        assert by_width[4][metric] < by_width[3][metric] < by_width[2][metric]
        for width in (2, 3, 4):
            close(by_width[width][metric], summary[f"width{width}_{json_stem}"])

    print("FTD-0655 raw certificate: PASS; protocol conformance: FAIL (32w != 64w)")


if __name__ == "__main__":
    main()
