"""Verify FTD-0433 records and reproduce its locked outcome."""

from __future__ import annotations

import csv
from collections import defaultdict
from hashlib import sha256
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine/results/ftd_0433"
MANIFEST = RESULTS / "manifest.json"
VOLUMES = (12, 16, 20, 24, 32, 40, 48)
C_WAVE = 0.57735026918962576451


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def complex_field(row: dict[str, str], prefix: str) -> complex:
    return complex(float(row[f"{prefix}_real"]),
                   float(row[f"{prefix}_imag"]))


def close(left: float, right: float, tolerance: float = 1e-12) -> bool:
    return abs(left - right) <= tolerance * max(1.0, abs(left), abs(right))


def pole_phase(volume: int) -> tuple[float, int, float]:
    k = 2.0 * math.pi / volume
    cx = math.cos(k)
    symbol = 4.0 - (2.0 / 3.0) * (cx + 2.0) \
        - (2.0 / 3.0) * (2.0 * cx + 1.0)
    omega = math.acos(1.0 - 0.5 * C_WAVE * C_WAVE * symbol)
    target = round(math.pi / omega) - 1
    return omega, target, abs((target + 1) * omega - math.pi)


def expected_keys(volume: int) -> set[tuple[int, int]]:
    target = pole_phase(volume)[1]
    return {(seed, tick) for seed in range(8)
            for tick in range(target + 1)}


def standardized_residuals(
    rows: list[dict[str, str]], volume: int
) -> tuple[list[float], list[float]]:
    groups: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        groups[int(row["tick"])].append(row)
    source: list[float] = []
    occupancy: list[float] = []
    for samples in groups.values():
        if len(samples) != 8:
            raise RuntimeError("incomplete seed ensemble")
        variance = sum(float(row["removal_variance"]) for row in samples)
        predicted = sum((complex_field(row, "predicted_next")
                         for row in samples), 0j) / 8.0
        actual = sum((complex_field(row, "source_after")
                      for row in samples), 0j) / 8.0
        source_sigma = math.sqrt(variance) / (8.0 * volume ** 3)
        source.append(abs(actual - predicted) / max(1e-15, source_sigma))
        expected_removed = sum(float(row["expected_removals"])
                               for row in samples) / 8.0
        actual_removed = sum(float(row["actual_removed"])
                             for row in samples) / 8.0
        occupancy_sigma = math.sqrt(variance) / 8.0
        occupancy.append(abs(actual_removed - expected_removed)
                         / max(1e-15, occupancy_sigma))
    return source, occupancy


def rms(values: list[float]) -> float:
    return math.sqrt(sum(value * value for value in values) / len(values))


def hazard(samples: list[dict[str, str]]) -> float:
    source = sum((complex_field(row, "source_before")
                  for row in samples), 0j) / len(samples)
    loss = sum((complex_field(row, "expected_loss")
                for row in samples), 0j) / len(samples)
    return (loss * source.conjugate()).real / abs(source) ** 2


def target_metrics(rows: list[dict[str, str]]) -> tuple[int, float, float, float]:
    target = int(rows[0]["target_transition"])
    samples = [row for row in rows if int(row["tick"]) == target]
    estimate = hazard(samples)
    leave_one_out = [
        hazard([row for row in samples if int(row["seed"]) != seed])
        for seed in range(8)
    ]
    jackknife_mean = sum(leave_one_out) / 8.0
    jackknife_se = math.sqrt(
        7.0 / 8.0
        * sum((value - jackknife_mean) ** 2 for value in leave_one_out))
    initial = sum((complex_field(row, "initial_source")
                   for row in samples), 0j) / 8.0
    after = sum((complex_field(row, "source_after")
                 for row in samples), 0j) / 8.0
    survival = (after * initial.conjugate()).real / abs(initial) ** 2
    return target, estimate, jackknife_se, survival


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    records: dict[str, list[dict[str, str]]] = {}

    for name, metadata in manifest["valid_records"].items():
        path = RESULTS / name
        rows = read_rows(path)
        records[name] = rows
        volume = int(metadata["L"])
        keys = {(int(row["seed"]), int(row["tick"])) for row in rows}
        checks.append((f"HASH {name}", sha256(path.read_bytes()).hexdigest()
                       == metadata["sha256"]))
        checks.append((f"ROWS/KEYS {name}", len(rows) == metadata["rows"]
                       and len(keys) == len(rows)
                       and keys == expected_keys(volume)))
        checks.append((f"BACKEND/SOURCE {name}", all(
            row["backend_label"] == metadata["backend"]
            and row["actual_backend"] == metadata["actual_backend"]
            and int(row["L"]) == volume
            and (int(row["dx"]), int(row["dy"]), int(row["dz"]),
                 int(row["n"])) == (1, 0, 0, 1)
            for row in rows)))
        omega, target, phase_error = pole_phase(volume)
        checks.append((f"POLE PHASE {name}", all(
            close(float(row["omega"]), omega, 2e-15)
            and int(row["target_transition"]) == target
            and close(float(row["phase_error"]), phase_error, 2e-15)
            and float(row["phase_error"])
                <= 0.5 * float(row["omega"]) + 1e-14
            for row in rows)))
        checks.append((f"STRUCTURE {name}", all(
            row["toggles_valid"] == "1"
            and row["structural_valid"] == "1"
            and row["execution_valid"] == "1"
            and int(row["initial_occupancy"]) == volume ** 3
            and int(row["initial_signed_state"]) == 0
            and abs(complex_field(row, "initial_source")) >= 0.3
            and int(row["occupancy_after"])
                <= int(row["occupancy_before"])
            and (complex_field(row, "source_before")
                 * complex_field(row, "initial_source").conjugate()).real > 0
            and (complex_field(row, "source_after")
                 * complex_field(row, "initial_source").conjugate()).real > 0
            for row in rows)))
        checks.append((f"HAZARD BOUNDS {name}", all(
            0.0 <= float(row["min_site_probability"])
            <= float(row["max_site_probability"]) <= 0.1 + 1e-15
            and math.isfinite(float(row["expected_removals"]))
            and float(row["expected_removals"]) >= 0.0
            and math.isfinite(float(row["removal_variance"]))
            and float(row["removal_variance"]) >= 0.0
            for row in rows)))
        if metadata["actual_backend"] == "cpu":
            checks.append((f"CPU HISTORY {name}", all(
                row["history_required"] == "1"
                and row["history_enabled"] == "1"
                and int(row["history_evaporation"])
                    == int(row["actual_removed"])
                and int(row["history_other"]) == 0 for row in rows)))
        else:
            checks.append((f"GPU HISTORY CONTRACT {name}", all(
                row["history_required"] == "0"
                and row["history_enabled"] == "0"
                and int(row["history_evaporation"]) == 0
                and int(row["history_other"]) == 0 for row in rows)))

    primary = {volume: records[f"wsl2_cuda_L{volume}.csv"]
               for volume in VOLUMES}
    all_source_z: list[float] = []
    all_occupancy_z: list[float] = []
    for volume in VOLUMES:
        source_z, occupancy_z = standardized_residuals(
            primary[volume], volume)
        all_source_z.extend(source_z)
        all_occupancy_z.extend(occupancy_z)
    summary = manifest["conditional_expectation"]
    checks.append(("SOURCE EXPECTATION GATES",
                   max(all_source_z) <= 6.0 and rms(all_source_z) <= 2.5))
    checks.append(("OCCUPANCY EXPECTATION GATES",
                   max(all_occupancy_z) <= 6.0
                   and rms(all_occupancy_z) <= 2.5))
    checks.append(("MANIFEST STANDARDIZED RESIDUALS",
                   close(max(all_source_z), summary["source_z_max"])
                   and close(rms(all_source_z), summary["source_z_rms"])
                   and close(max(all_occupancy_z),
                             summary["occupancy_z_max"])
                   and close(rms(all_occupancy_z),
                             summary["occupancy_z_rms"])))

    cpu = records["windows_msvc_cpu_L32.csv"]
    cuda = primary[32]
    row_key = lambda row: (int(row["seed"]), int(row["tick"]))
    checks.append(("CPU/CUDA ROW ORDER",
                   [row_key(row) for row in cpu]
                   == [row_key(row) for row in cuda]))
    complex_names = ("source_before", "expected_loss", "predicted_next",
                     "source_after")
    scalar_names = ("source_hazard", "expected_removals",
                    "removal_variance", "actual_removed",
                    "mean_site_probability", "min_site_probability",
                    "max_site_probability", "mean_local_energy",
                    "max_local_energy")
    complex_max = max(abs(complex_field(left, field)
                          - complex_field(right, field))
                      for left, right in zip(cpu, cuda)
                      for field in complex_names)
    scalar_max = max(abs(float(left[field]) - float(right[field]))
                     for left, right in zip(cpu, cuda)
                     for field in scalar_names)
    checks.append(("CPU/CUDA REGISTERED FIELD AGREEMENT",
                   complex_max <= 1e-10 and scalar_max <= 1e-10))
    agreement = manifest["backend_agreement"]
    checks.append(("MANIFEST BACKEND AGREEMENT",
                   close(complex_max, agreement["complex_field_max_abs"])
                   and close(scalar_max,
                             agreement["scalar_field_max_abs"])))

    metrics: dict[int, tuple[int, float, float, float]] = {
        volume: target_metrics(primary[volume]) for volume in VOLUMES
    }
    for volume, values in metrics.items():
        recorded = manifest["target_metrics"][str(volume)]
        checks.append((f"MANIFEST TARGET L={volume}",
                       values[0] == recorded["target_transition"]
                       and close(values[1], recorded["hazard"])
                       and close(values[2], recorded["jackknife_se"])
                       and close(values[3], recorded["survival"])))

    exponents: dict[str, float] = {}
    for left, right in zip(VOLUMES, VOLUMES[1:]):
        value = math.log(metrics[right][1] / metrics[left][1]) \
            / math.log((2.0 * math.pi / right) / (2.0 * math.pi / left))
        exponents[f"{left}_{right}"] = value
    checks.append(("MANIFEST EFFECTIVE EXPONENTS", all(
        close(value, manifest["effective_exponents"][name])
        for name, value in exponents.items())))

    hazards = [metrics[volume][1] for volume in VOLUMES]
    ratio = metrics[48][1] / metrics[12][1]
    lower = metrics[48][1] - 1.96 * metrics[48][2]
    upper = metrics[48][1] + 1.96 * metrics[48][2]
    largest = manifest["largest_volume"]
    checks.append(("MANIFEST LARGEST VOLUME",
                   close(ratio, largest["hazard_ratio_48_12"])
                   and close(lower, largest["hazard_95_lower"])
                   and close(upper, largest["hazard_95_upper"])))

    strictly_decreasing = all(right < left
                              for left, right in zip(hazards, hazards[1:]))
    outcome_a = (all(value > 0.0 for value in hazards)
                 and strictly_decreasing and ratio <= 0.25
                 and upper < 0.01
                 and exponents["32_40"] > 0.25
                 and exponents["40_48"] > 0.25
                 and metrics[48][3] > 0.1)
    outcome_b = (lower > 0.01
                 or (ratio >= 0.75
                     and abs(exponents["32_40"]) < 0.25
                     and abs(exponents["40_48"]) < 0.25))
    checks.append(("OUTCOME A FAILS LOCKED GATES", not outcome_a
                   and not strictly_decreasing and ratio > 0.25))
    checks.append(("OUTCOME B FAILS LOCKED GATES", not outcome_b
                   and lower < 0.01
                   and (abs(exponents["32_40"]) >= 0.25
                        or abs(exponents["40_48"]) >= 0.25)))
    checks.append(("OUTCOME C RECORDED", manifest["outcome"] == "C"
                   and manifest["status"] == "UNRESOLVED_SCALING"
                   and manifest["infrared_inference"] == "NONE"
                   and manifest["conservation_inference"] == "NONE"))

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nFTD-0433 result checks: {len(checks) - failed}/"
          f"{len(checks)} passed; h48/h12={ratio:.6g}; "
          f"p32,40={exponents['32_40']:.6g}; "
          f"p40,48={exponents['40_48']:.6g}; outcome=C")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
