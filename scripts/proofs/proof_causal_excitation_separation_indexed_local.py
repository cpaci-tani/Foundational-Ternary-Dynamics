"""Independent certificate for the locked FTD-0694 run of record."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0694"
JSON_PATH = RESULT / "ftd_0694_causal_excitation_separation_indexed_local_v1.json"
CSV_PATH = RESULT / "ftd_0694_causal_excitation_separation_indexed_local_v1_ticks.csv"
PROTOCOL = "29DE3DCA11FEC2C77F5A765F89CCF4FDD06379CD23FE9A9EE73B044025B5025A"
EXPECTED_JSON_SHA256 = "9D80E03709684A4847DCEC718EE1AB5662DB75F652955439697C4EAF50DD0B96"
EXPECTED_CSV_SHA256 = "4667648AC9B5552C71B790203A20D3554741A19DB02B22EB2E6ECA4989CDF197"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def linear_fit(samples: list[tuple[float, float]]) -> tuple[float, float, float]:
    n = float(len(samples))
    sx = sum(x for x, _ in samples)
    sy = sum(y for _, y in samples)
    sxx = sum(x * x for x, _ in samples)
    sxy = sum(x * y for x, y in samples)
    denominator = n * sxx - sx * sx
    assert denominator > 0.0
    slope = (n * sxy - sx * sy) / denominator
    intercept = (sy - slope * sx) / n
    mean = sy / n
    residual = sum((y - intercept - slope * x) ** 2 for x, y in samples)
    total = sum((y - mean) ** 2 for _, y in samples)
    return slope, intercept, 1.0 - residual / total


def close(lhs: float, rhs: float, tolerance: float = 1e-12) -> bool:
    return abs(lhs - rhs) <= tolerance * max(1.0, abs(lhs), abs(rhs))


def main() -> None:
    assert sha256(JSON_PATH) == EXPECTED_JSON_SHA256
    assert sha256(CSV_PATH) == EXPECTED_CSV_SHA256
    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    assert summary["ftd_id"] == "FTD-0694"
    assert summary["protocol_sha256"] == PROTOCOL
    assert summary["verdict"] == (
        "DISTRIBUTED_FIELD_MIXED + CONTINUING_EXCITATION_TRANSFER"
    )
    assert summary["production_changed"] is False
    assert summary["negative_exact"] and summary["positive_exact"]

    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    assert len(rows) == 2 * 25 * 6
    assert all(row["ftd_id"] == "FTD-0694" for row in rows)
    assert all(row["protocol_sha256"] == PROTOCOL for row in rows)
    assert all(row["valid"] == "1" for row in rows)

    records: dict[tuple[int, int, int], dict[str, str]] = {}
    for row in rows:
        key = (int(row["sign"]), int(row["tick"]), int(row["radius"]))
        assert key not in records
        records[key] = row

    radii = [8, 16, 24, 32, 40, 48]
    arrivals_by_sign: dict[int, list[int]] = {}
    for sign in (-1, 1):
        arrivals: list[int] = []
        for radius in radii:
            arrival = -1
            for tick in range(0, 97, 4):
                if float(records[(sign, tick, radius)]["cumulative_outward"]) >= 0.001:
                    arrival = tick
                    break
            arrivals.append(arrival)
        assert arrivals == [20, 40, 60, 80, 96, -1]
        arrivals_by_sign[sign] = arrivals
        samples = [
            (float(radius), float(arrival))
            for radius, arrival in zip(radii[1:], arrivals[1:])
            if arrival >= 0
        ]
        slope, _, r_squared = linear_fit(samples)
        speed = 1.0 / slope
        prefix = "negative" if sign < 0 else "positive"
        assert close(speed, summary[f"{prefix}_shell_speed"])
        assert close(r_squared, summary[f"{prefix}_shell_fit_r_squared"])

    assert summary["negative_max_profile_residual"] < 1e-12
    assert summary["positive_max_profile_residual"] < 1e-12
    assert summary["negative_max_regional_residual"] < 1e-10
    assert summary["positive_max_regional_residual"] < 1e-10
    assert summary["negative_max_energy_drift"] < 1e-10
    assert summary["positive_max_energy_drift"] < 1e-10
    assert summary["negative_max_common_residual"] < 1e-10
    assert summary["positive_max_common_residual"] < 1e-10
    assert summary["negative_recovery"] < 1e-8
    assert summary["positive_recovery"] < 1e-8
    assert summary["core_history_rms"] < 1e-4
    assert summary["final_fraction_difference"] < 1e-4

    negative = [
        records[(-1, tick, 8)] for tick in range(20, 97, 4)
    ]
    spreading: dict[str, dict[str, float]] = {}
    for field in ("field_mean_radius", "field_rms_radius", "r50", "r90", "r99"):
        samples = [(float(row["tick"]), float(row[field])) for row in negative]
        slope, intercept, r_squared = linear_fit(samples)
        assert slope > 0.0 and r_squared > 0.99
        spreading[field] = {
            "slope": slope,
            "intercept": intercept,
            "r_squared": r_squared,
        }

    initial_target = float(records[(-1, 0, 8)]["target_energy"])
    final = records[(-1, 96, 8)]
    positive_profile_ratio = float(final["field_total"]) / initial_target
    near_profile_ratio = float(final["cumulative_field"]) / initial_target
    source_transfer = sum(
        float(records[(-1, tick, 8)]["source_exchange_into_field"])
        for tick in range(0, 97, 4)
    )
    assert max(int(row["source_support_radius"]) for row in negative) <= 3
    assert math.isfinite(source_transfer) and source_transfer > 0.0

    print("FTD-0694 independent certificate: PASS")
    print(f"rows={len(rows)} arrivals={arrivals_by_sign[-1]}")
    print(
        f"shell_speed={summary['negative_shell_speed']:.15g} "
        f"shell_r2={summary['negative_shell_fit_r_squared']:.15g}"
    )
    for field, fit in spreading.items():
        print(
            f"{field}: slope={fit['slope']:.15g} "
            f"r2={fit['r_squared']:.15g}"
        )
    print(
        f"tick96_target_ratio={float(final['target_ratio']):.15g} "
        f"positive_profile_ratio={positive_profile_ratio:.15g} "
        f"near_profile_ratio={near_profile_ratio:.15g} "
        f"modified_energy_source_transfer={source_transfer:.15g}"
    )
    print(
        "scope: positive profile norms and exact modified-energy transport "
        "are distinct observables and are not added as one energy budget"
    )


if __name__ == "__main__":
    main()

