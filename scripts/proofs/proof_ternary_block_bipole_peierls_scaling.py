"""Independent FTD-0621 integer block-bipole spectral certificate."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT / "docs/theory/10_eft_program/preregistrations"
    / "PREREG_TERNARY_BLOCK_BIPOLE_PEIERLS_SCALING_v1.md"
)
RESULT_DIR = ROOT / "engine/results/ftd_0621"
RESULT = RESULT_DIR / "ftd_0621_ternary_block_bipole_peierls_v1.json"
CSV = RESULT_DIR / "ftd_0621_ternary_block_bipole_peierls_v1.csv"
INVALID_1 = RESULT_DIR / "ftd_0621_invalid_execution_1.json"
INVALID_2 = RESULT_DIR / "ftd_0621_invalid_execution_2.json"

PROTOCOL_SHA = "905819BD83E8C4AC6698A75D7C87640B807BC2C621350DCAAE8563945148CB31"
RESULT_SHA = "D6ED6A0BF3C9B351ED59E4B16C0FD82430A4713B4ED06B0092F9BDCBB4026383"
CSV_SHA = "693AB224212F1D57CFC7F293EC88408FFBB7E8990C430290FCC66A100A5928B2"
INVALID_1_SHA = "F0A08F13A2FB31905965411B1E50375CE0B9DE96C4B75246817DEC4DDF313EF9"
INVALID_2_SHA = "73ECA638E0AF44A9D8699D6D938C742A86DD2722D9BC05756DD4BE65EAEA034D"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def close(a: float, b: float, relative: float = 1e-12) -> bool:
    return abs(a - b) <= relative * max(1e-300, abs(a), abs(b))


def log_slope(widths: list[int], values: list[float]) -> float:
    x = [math.log(w) for w in widths]
    y = [math.log(value) for value in values]
    count = len(x)
    sx, sy = sum(x), sum(y)
    sxx = sum(value * value for value in x)
    sxy = sum(a * b for a, b in zip(x, y, strict=True))
    return (count * sxy - sx * sy) / (count * sxx - sx * sx)


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    invalid_1 = json.loads(INVALID_1.read_text(encoding="utf-8"))
    invalid_2 = json.loads(INVALID_2.read_text(encoding="utf-8"))
    with CSV.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    keyed = {
        (int(row["L"]), int(row["width"]), int(row["orientation"]),
         int(row["translation_axis"])): row
        for row in rows
    }
    checks: list[tuple[str, bool]] = []

    def check(name: str, condition: bool) -> None:
        checks.append((name, bool(condition)))

    check("protocol_hash", sha256(PROTOCOL) == PROTOCOL_SHA)
    check("record_protocol", record["protocol_sha256"] == PROTOCOL_SHA)
    check("result_hash", sha256(RESULT) == RESULT_SHA)
    check("csv_hash", sha256(CSV) == CSV_SHA)
    check("invalid_1_hash", sha256(INVALID_1) == INVALID_1_SHA)
    check("invalid_2_hash", sha256(INVALID_2) == INVALID_2_SHA)
    check("invalid_executions_preserved",
          invalid_1["verdict"] == "TERNARY_BLOCK_BIPOLE_OBSERVER_INVALID"
          and invalid_2["verdict"] == "TERNARY_BLOCK_BIPOLE_OBSERVER_INVALID"
          and not bool(invalid_1["algebraic_pass"])
          and not bool(invalid_2["algebraic_pass"]))
    check("production_unchanged", record["production_changed"] is False)
    check("record_gates", all(bool(record[name]) for name in (
        "coverage_pass", "algebraic_pass", "covariance_pass", "volume_pass",
        "monotonic_pass", "scaling_pass", "endpoint_pass")))

    volumes = {193, 257}
    widths = [5, 9, 15, 23, 35]
    expected_keys = {
        (volume, width, orientation, translation)
        for volume in volumes for width in widths
        for orientation in range(3) for translation in range(3)
    }
    check("arm_coverage", len(rows) == 90 and set(keyed) == expected_keys)
    check("primitive_counts", all(
        int(row["positive_sites"]) == int(row["width"]) ** 3
        and int(row["negative_sites"]) == int(row["width"]) ** 3
        for row in rows))
    check("strict_positive", all(
        float(row[name]) > 0.0
        for row in rows
        for name in ("energy", "coefficient", "barrier", "pinning_index")))
    check("coefficient_barrier_identity", all(close(
        float(row["coefficient"]), 16.0 * float(row["barrier"]), 2e-15)
        for row in rows))
    check("pinning_energy_identity", all(close(
        float(row["pinning_index"]),
        float(row["barrier"]) / float(row["energy"]), 2e-15)
        for row in rows))
    check("spectral_average_identity", max(
        abs(float(row["pinning_index"]) - float(row["spectral_average"]))
        for row in rows) <= 1e-12)
    check("structure_gate", max(float(row["structure_residual"])
                                for row in rows) <= 1e-11)

    covariance = 0.0
    for volume in volumes:
        for width in widths:
            parallel = float(keyed[(volume, width, 0, 0)]["pinning_index"])
            transverse = float(keyed[(volume, width, 0, 1)]["pinning_index"])
            energy = float(keyed[(volume, width, 0, 0)]["energy"])
            for orientation in range(3):
                covariance = max(covariance, abs(
                    float(keyed[(volume, width, orientation, 0)]["energy"])
                    - energy) / energy)
                for translation in range(3):
                    expected = parallel if orientation == translation else transverse
                    measured = float(keyed[(volume, width, orientation,
                                            translation)]["pinning_index"])
                    covariance = max(covariance,
                                     abs(measured - expected) / expected)
    check("cubic_covariance", covariance <= 1e-12)
    check("covariance_record", close(
        covariance, float(record["worst_covariance_residual"]), 1e-12))

    volume_difference = 0.0
    for width in widths:
        if width > 23:
            continue
        for orientation in range(3):
            for translation in range(3):
                main = float(keyed[(257, width, orientation,
                                    translation)]["pinning_index"])
                replica = float(keyed[(193, width, orientation,
                                       translation)]["pinning_index"])
                volume_difference = max(
                    volume_difference,
                    abs(main - replica) / max(abs(main), abs(replica)))
    check("volume_gate", volume_difference <= 0.08)
    check("volume_record", close(
        volume_difference,
        float(record["worst_volume_relative_difference"]), 1e-12))

    improvements: list[float] = []
    endpoints: list[float] = []
    monotonic = True
    energy_residual = 0.0
    barrier_residual = 0.0
    pinning_residual = 0.0
    fit_widths = widths[1:]
    for orientation in range(3):
        energy_values = [float(keyed[(257, width, orientation, 0)]["energy"])
                         for width in fit_widths]
        energy_residual = max(
            energy_residual, abs(log_slope(fit_widths, energy_values) - 5.0))
        for translation in range(3):
            values = [float(keyed[(257, width, orientation,
                                   translation)]["pinning_index"])
                      for width in widths]
            monotonic &= all(after < before
                             for before, after in zip(values, values[1:]))
            improvements.append(values[0] / values[-1])
            endpoints.append(values[-1])
            barriers = [float(keyed[(257, width, orientation,
                                     translation)]["barrier"])
                        for width in fit_widths]
            pinning = values[1:]
            barrier_residual = max(
                barrier_residual,
                abs(log_slope(fit_widths, barriers) - 2.0))
            pinning_residual = max(
                pinning_residual,
                abs(log_slope(fit_widths, pinning) + 3.0))
    check("strict_monotonicity", monotonic)
    check("energy_slope_gate", energy_residual <= 0.35)
    check("barrier_slope_gate", barrier_residual <= 0.35)
    check("pinning_slope_gate", pinning_residual <= 0.35)
    check("slope_records",
          abs(energy_residual
              - float(record["worst_energy_slope_residual"])) <= 1e-12
          and abs(barrier_residual
                  - float(record["worst_barrier_slope_residual"])) <= 1e-12
          and abs(pinning_residual
                  - float(record["worst_pinning_slope_residual"])) <= 1e-12)
    check("endpoint_gate", max(endpoints) < 5e-5)
    check("endpoint_record", close(
        max(endpoints), float(record["largest_endpoint_pinning_index"]), 1e-12))
    check("improvement_record", close(
        min(improvements), float(record["smallest_endpoint_improvement"]),
        1e-12))
    check("verdict", record["verdict"]
          == "INTEGER_TERNARY_EXTENSION_SUPPRESSES_PEIERLS")

    passed = sum(ok for _, ok in checks)
    for name, ok in checks:
        print(f"{'PASS' if ok else 'FAIL'} {name}")
    print(f"{passed}/{len(checks)} checks passed")
    print(json.dumps({
        "ftd_id": "FTD-0621",
        "protocol_sha256": PROTOCOL_SHA,
        "result_sha256": RESULT_SHA,
        "minimum_improvement": min(improvements),
        "largest_endpoint_pinning_index": max(endpoints),
        "energy_slope_residual": energy_residual,
        "barrier_slope_residual": barrier_residual,
        "pinning_slope_residual": pinning_residual,
        "passed": passed,
        "total": len(checks),
    }, indent=2))
    return 0 if passed == len(checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())
