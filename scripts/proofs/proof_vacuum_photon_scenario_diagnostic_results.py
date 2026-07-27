"""Verify the FTD-0434 exact vacuum-photon diagnostic run of record."""

from __future__ import annotations

import csv
from hashlib import sha256
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "engine/results/ftd_0434/manifest.json"


def close(actual: float, expected: float, tolerance: float = 1e-12) -> bool:
    return math.isclose(actual, expected, rel_tol=tolerance, abs_tol=tolerance)


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    record = ROOT / manifest["record"]["path"]
    rows = list(csv.DictReader(record.open(encoding="utf-8", newline="")))
    arms = {
        arm: [row for row in rows if row["arm"] == arm]
        for arm in ("dashboard", "wave_only")
    }
    by_tick = {
        arm: {int(row["tick"]): row for row in arm_rows}
        for arm, arm_rows in arms.items()
    }

    initial_dashboard = by_tick["dashboard"][0]
    initial_wave = by_tick["wave_only"][0]
    tick1_dashboard = by_tick["dashboard"][1]
    tick20_dashboard = by_tick["dashboard"][20]
    tick20_wave = by_tick["wave_only"][20]
    initial_width = float(initial_wave["width_x"])
    dashboard_width_ratio = float(tick20_dashboard["width_x"]) / initial_width
    wave_width_ratio = float(tick20_wave["width_x"]) / initial_width
    dashboard_div_ratio = (
        float(tick1_dashboard["divergence_normalized"])
        / float(initial_dashboard["divergence_normalized"])
    )

    checks: list[tuple[str, bool]] = [
        ("MANIFEST valid revision-2 execution",
         manifest["identifier"] == "FTD-0434"
         and manifest["revision"] == 2
         and manifest["execution_status"] == "VALID"),
        ("RECORD hash and byte count",
         sha256(record.read_bytes()).hexdigest() == manifest["record"]["sha256"]
         and record.stat().st_size == manifest["record"]["bytes"]),
        ("RECORD contains exactly two 25-tick arms",
         len(rows) == 50
         and all(len(arm_rows) == 25 for arm_rows in arms.values())
         and all(set(by_tick[arm]) == set(range(25)) for arm in arms)),
        ("EXECUTION gates pass for every row",
         all(row["actual_backend"] == "cpu"
             and row["dispatched"] == "1"
             and row["toggles_valid"] == "1"
             and row["execution_valid"] == "1" for row in rows)),
        ("STATE field remains empty",
         all(row["occupancy"] == "0" and row["signed_state"] == "0"
             for row in rows)),
        ("INITIAL seed is Jz plus orthogonal Wx",
         close(float(initial_wave["flux_x2"]), 0.0)
         and float(initial_wave["flux_z2"]) > 0.0
         and float(initial_wave["wave_x2"]) > 0.0
         and close(float(initial_wave["wave_z2"]), 0.0)),
        ("INITIAL right-moving Jz relation fails maximally",
         close(float(initial_wave["right_moving_residual"]), 1.0)
         and close(float(initial_wave["wave_fraction_x"]), 1.0)
         and close(float(initial_wave["wave_fraction_y"]), 0.0)
         and close(float(initial_wave["wave_fraction_z"]), 0.0)),
        ("WAVE-ONLY centroid does not translate",
         abs(float(tick20_wave["displacement_x"])) < 1e-12
         and int(tick20_wave["best_shift"]) == 0),
        ("WAVE-ONLY seed spreads and decorrelates",
         wave_width_ratio >= 1.25
         and float(tick20_wave["best_shift_overlap"]) < 0.8
         and tick20_wave["nontranslating_clause"] == "1"),
        ("DASHBOARD fails translating-packet displacement gate",
         float(tick20_dashboard["displacement_x"]) < 8.0
         and tick20_dashboard["translating_clause"] == "0"),
        ("DASHBOARD broadens and loses translated-profile overlap",
         dashboard_width_ratio >= 1.25
         and float(tick20_dashboard["best_shift_overlap"]) < 0.8),
        ("DASHBOARD does not meet locked projection-dominated threshold",
         dashboard_div_ratio > 0.01
         and tick20_dashboard["projection_dominated_clause"] == "0"),
        ("BOTH arms reject demonstrated propagating photon",
         all(by_tick[arm][20]["translating_clause"] == "0" for arm in arms)
         and manifest["outcome"]
         == "CLOSED_NEGATIVE_FOR_PROPAGATING_PHOTON_SCENARIO_CLAIM"),
    ]

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(
        f"\nVacuum-photon diagnostic result checks: "
        f"{len(checks) - failed}/{len(checks)} passed"
    )
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
