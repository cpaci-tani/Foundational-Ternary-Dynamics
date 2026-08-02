"""Independent certificate for corrected FTD-0656 mobile dressing data."""

from __future__ import annotations

import cmath
import csv
import json
import math
from collections import defaultdict
from pathlib import Path

from proof_mobile_dressing_structure_factor import close, family_data, fit, unwrap


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0656"
PROTOCOL = "898AF1958713038FC945D09DD4DEA434A213BC6F79DE44006F64D35A208C99E3"


def main() -> None:
    summary = json.loads((RESULT / "ftd_0656_mobile_dressing_structure_factor_v2.json").read_text())
    assert summary["protocol_sha256"] == PROTOCOL
    assert summary["verdict"] == "MOBILE_DRESSED_STRUCTURE_FACTOR_V2_CONSTRUCTIVE"
    assert summary["arm_count"] == 18
    for gate in ("coverage_pass", "execution_pass", "exact_pass",
                 "coherence_pass", "matter_pass", "field_pass",
                 "mirror_pass", "cubic_pass", "width_trend_pass"):
        assert summary[gate] == 1

    with (RESULT / "ftd_0656_mobile_dressing_structure_factor_arms_v2.csv").open(newline="") as handle:
        arms = {row["label"]: row for row in csv.DictReader(handle)}
    series: dict[str, list[dict[str, str]]] = defaultdict(list)
    with (RESULT / "ftd_0656_mobile_dressing_structure_factor_series_v2.csv").open(newline="") as handle:
        for row in csv.DictReader(handle):
            series[row["label"]].append(row)
    assert len(arms) == len(series) == 18 and set(arms) == set(series)

    by_width = {width: {"mismatch": 0.0, "relative": 0.0, "field_cv": 0.0}
                for width in (2, 3, 4)}
    metrics: dict[str, dict[str, float]] = {}
    for label, arm in arms.items():
        width = int(arm["width"])
        rows = sorted(series[label], key=lambda row: int(row["tick"]))
        assert len(rows) == 32 * width + 1
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

    for metric, stem in (("mismatch", "velocity_mismatch"),
                         ("relative", "relative_phase_rms"),
                         ("field_cv", "field_cv")):
        assert by_width[4][metric] < by_width[3][metric] < by_width[2][metric]
        for width in (2, 3, 4):
            close(by_width[width][metric], summary[f"width{width}_{stem}"])

    print("FTD-0656 independent certificate: PASS")


if __name__ == "__main__":
    main()
