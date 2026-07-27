"""Verify FTD-0431 records and reproduce its locked outcome adjudication."""

from __future__ import annotations

import csv
from collections import defaultdict
from hashlib import sha256
import json
import math
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine/results/ftd_0431"
MANIFEST = RESULTS / "manifest.json"
DIRECTIONS = ((1, 0, 0), (1, 1, 0), (1, 1, 1))


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def group_key(row: dict[str, str]) -> tuple[object, ...]:
    return (
        row["arm"], int(row["seed"]), int(row["dx"]),
        int(row["dy"]), int(row["dz"]), int(row["n"]),
    )


def expected_groups(profile: str) -> set[tuple[object, ...]]:
    groups: set[tuple[object, ...]] = set()
    for direction in DIRECTIONS:
        for n in (1, 2, 3):
            if profile == "full":
                for seed in range(8):
                    groups.add(("isolated", seed, *direction, n))
                    groups.add(("coupled", seed, *direction, n))
                groups.add(("locked_control", 0, *direction, n))
            else:
                for seed in range(8):
                    groups.add(("coupled", seed, *direction, n))
    return groups


def grouped(rows: list[dict[str, str]]) -> dict[tuple[object, ...], list[dict[str, str]]]:
    groups: dict[tuple[object, ...], list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        groups[group_key(row)].append(row)
    for samples in groups.values():
        samples.sort(key=lambda row: int(row["tick"]))
    return groups


def ensemble_series(
    groups: dict[tuple[object, ...], list[dict[str, str]]],
    arm: str,
    direction: tuple[int, int, int],
    n: int,
    omitted_seed: int | None = None,
) -> list[complex]:
    seeds = [seed for seed in range(8) if seed != omitted_seed]
    out: list[complex] = []
    for tick in range(17):
        values = []
        for seed in seeds:
            row = groups[(arm, seed, *direction, n)][tick]
            values.append(complex(float(row["source_real"]),
                                  float(row["source_imag"])))
        out.append(sum(values) / len(values))
    return out


def decay_fit(series: list[complex]) -> tuple[float, float, list[float]]:
    initial = series[0]
    norm = abs(initial) ** 2
    amplitudes = [
        (value * initial.conjugate()).real / norm for value in series[:7]
    ]
    if any(value <= 0.0 or not math.isfinite(value) for value in amplitudes):
        return math.nan, math.inf, amplitudes
    x_values = list(range(7))
    y_values = [math.log(value) for value in amplitudes]
    x_mean = sum(x_values) / len(x_values)
    y_mean = sum(y_values) / len(y_values)
    denominator = sum((value - x_mean) ** 2 for value in x_values)
    slope = sum(
        (x_value - x_mean) * (y_value - y_mean)
        for x_value, y_value in zip(x_values, y_values)
    ) / denominator
    intercept = y_mean - slope * x_mean
    residual_sq = sum(
        (value - (intercept + slope * x_value)) ** 2
        for x_value, value in zip(x_values, y_values)
    )
    data_sq = sum(value * value for value in y_values)
    return -slope, math.sqrt(residual_sq / max(1e-30, data_sq)), amplitudes


def read_constant(name: str) -> float:
    text = (ROOT / "engine/include/ftd/ontic/gauge_couplings.h").read_text(
        encoding="utf-8")
    match = re.search(
        rf"inline constexpr double {name} = ([0-9.eE+-]+);", text)
    if not match:
        raise RuntimeError(f"could not read canonical {name}")
    return float(match.group(1))


def field_residual(
    samples: list[dict[str, str]], c_wave: float, g_c: float
) -> float:
    size = int(samples[0]["L"])
    n = int(samples[0]["n"])
    k = [
        2.0 * math.pi * n * int(samples[0][name]) / size
        for name in ("dx", "dy", "dz")
    ]
    cx, cy, cz = [math.cos(value) for value in k]
    symbol = 4.0 - (2.0 / 3.0) * (cx + cy + cz) \
        - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz)
    gradient = sum(math.sin(value) ** 2 for value in k)

    def source(row: dict[str, str]) -> complex:
        return complex(float(row["source_real"]), float(row["source_imag"]))

    def divergence(row: dict[str, str]) -> complex:
        return complex(float(row["div_real"]), float(row["div_imag"]))

    maximum = 0.0
    for index in range(1, len(samples) - 1):
        previous = divergence(samples[index - 1])
        current = divergence(samples[index])
        following = divergence(samples[index + 1])
        forcing = g_c * gradient * source(samples[index])
        predicted = (2.0 - c_wave * c_wave * symbol) * current \
            - previous + forcing
        scale = max(1.0, abs(following), abs(current), abs(previous),
                    abs(forcing))
        maximum = max(maximum, abs(following - predicted) / scale)
    return maximum


def close(a: float, b: float, tolerance: float = 1e-10) -> bool:
    return abs(a - b) <= tolerance * max(1.0, abs(a), abs(b))


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    records: dict[str, list[dict[str, str]]] = {}
    groups_by_record: dict[str, dict[tuple[object, ...], list[dict[str, str]]]] = {}

    c_wave = read_constant("C_WAVE")
    g_c = read_constant("G_C")
    exact_gamma = -math.log(0.9)

    for name, metadata in manifest["valid_records"].items():
        path = RESULTS / name
        checks.append((f"HASH {name}", sha256(path.read_bytes()).hexdigest()
                       == metadata["sha256"]))
        rows = read_rows(path)
        records[name] = rows
        groups = grouped(rows)
        groups_by_record[name] = groups
        checks.append((f"ROWS {name}", len(rows) == metadata["rows"]))
        checks.append((f"GROUPS/TICKS {name}",
                       set(groups) == expected_groups(metadata["profile"])
                       and all([int(row["tick"]) for row in samples]
                               == list(range(17))
                               for samples in groups.values())))
        expected_backend = "cpu" if metadata["backend"] == "windows_msvc_cpu" \
            else "gpu"
        checks.append((f"BACKEND/PROFILE {name}", all(
            row["backend_label"] == metadata["backend"]
            and row["actual_backend"] == expected_backend
            and int(row["L"]) == metadata["L"]
            and row["profile"] == metadata["profile"] for row in rows)))
        checks.append((f"EXECUTION FLAGS {name}", all(
            row["toggles_valid"] == "1" and row["event_valid"] == "1"
            and row["source_valid"] == "1" and row["field_valid"] == "1"
            and row["execution_valid"] == "1" for row in rows)))

        structure_ok = True
        history_ok = True
        source_ok = True
        field_ok = True
        recurrence_max = 0.0
        for key, samples in groups.items():
            arm = str(key[0])
            occupancies = [int(row["occupancy"]) for row in samples]
            removed = [int(row["removed_since_last"]) for row in samples]
            cumulative = [int(row["cumulative_removed"]) for row in samples]
            structure_ok &= int(samples[0]["global_charge"]) == 0
            structure_ok &= occupancies[0] == metadata["L"] ** 3
            structure_ok &= float(samples[0]["source_abs"]) >= 0.3
            structure_ok &= all(a >= b for a, b in zip(occupancies,
                                                        occupancies[1:]))
            structure_ok &= all(
                cumulative[index] == occupancies[0] - occupancies[index]
                for index in range(17))
            structure_ok &= all(
                removed[index] == occupancies[index - 1] - occupancies[index]
                for index in range(1, 17))
            if arm == "locked_control":
                structure_ok &= occupancies[-1] == occupancies[0]
                source0 = complex(float(samples[0]["source_real"]),
                                  float(samples[0]["source_imag"]))
                sourcef = complex(float(samples[-1]["source_real"]),
                                  float(samples[-1]["source_imag"]))
                source_ok &= abs(sourcef - source0) <= 1e-14
            else:
                structure_ok &= removed[1] > 0 and cumulative[-1] > 0

            if expected_backend == "cpu":
                history_ok &= all(row["history_required"] == "1"
                                  and row["history_enabled"] == "1"
                                  and int(row["history_other"]) == 0
                                  and int(row["history_evaporation"])
                                  == int(row["removed_since_last"])
                                  for row in samples)
            else:
                history_ok &= all(row["history_required"] == "0"
                                  and row["history_enabled"] == "0"
                                  and int(row["history_other"]) == 0
                                  and int(row["history_evaporation"]) == 0
                                  for row in samples)

            divergence = [float(row["div_abs"]) for row in samples]
            if arm == "isolated":
                field_ok &= max(divergence) <= 1e-14
            else:
                field_ok &= max(divergence[:3]) > 1e-8
                recurrence_max = max(
                    recurrence_max, field_residual(samples, c_wave, g_c))

        checks.append((f"REACTION STRUCTURE {name}", structure_ok))
        checks.append((f"HISTORY CONTRACT {name}", history_ok))
        checks.append((f"SOURCE CONTROLS {name}", source_ok))
        checks.append((f"FIELD ACTIVATION {name}", field_ok))
        tolerance = 1e-10 if expected_backend == "cpu" else 1e-8
        checks.append((f"FIELD RECURRENCE {name}", recurrence_max <= tolerance
                       and close(recurrence_max,
                                 metadata["max_field_recurrence_residual"],
                                 1e-8)))

    for name, metadata in manifest["invalid_run_provenance"].items():
        path = RESULTS / name
        rows = read_rows(path)
        checks.append((f"INVALID PROVENANCE {name}",
                       sha256(path.read_bytes()).hexdigest()
                       == metadata["sha256"]
                       and len(rows) == metadata["rows"]
                       and name not in manifest["valid_records"]))

    fit_summary: dict[str, dict[tuple[int, int, int, int], tuple[float, float]]] = {}
    for name, groups in groups_by_record.items():
        fits: dict[tuple[int, int, int, int], tuple[float, float]] = {}
        for direction in DIRECTIONS:
            for n in (1, 2, 3):
                for arm in ("isolated", "coupled"):
                    if (arm, 0, *direction, n) not in groups:
                        continue
                    gamma, rms, _ = decay_fit(
                        ensemble_series(groups, arm, direction, n))
                    fits[(0 if arm == "isolated" else 1, *direction, n)] = (
                        gamma, rms)
        fit_summary[name] = fits

    for name in ("windows_msvc_cpu_L32_full.csv", "wsl2_cuda_L32_full.csv"):
        fits = fit_summary[name]
        isolated = [value for key, value in fits.items() if key[0] == 0]
        checks.append((f"ISOLATED DECAY CALIBRATION {name}", all(
            abs(gamma - exact_gamma) / exact_gamma <= 0.02 and rms <= 0.02
            for gamma, rms in isolated)))

    windows = fit_summary["windows_msvc_cpu_L32_full.csv"]
    cuda = fit_summary["wsl2_cuda_L32_full.csv"]
    checks.append(("CPU/CUDA L32 DECAY AGREEMENT", windows.keys() == cuda.keys()
                   and all(abs(windows[key][0] - cuda[key][0]) <= 0.01
                           for key in windows)))

    coupled_rms = [
        rms for fits in fit_summary.values()
        for key, (_, rms) in fits.items() if key[0] == 1
    ]
    coupled_gate_pass = bool(coupled_rms) and max(coupled_rms) <= 0.02
    checks.append(("LOCKED COUPLED EXPONENTIAL GATE FAILS",
                   not coupled_gate_pass
                   and close(max(coupled_rms),
                             manifest["analysis_validity"]["max_coupled_normalized_rms"],
                             1e-10)))

    lowest_groups = groups_by_record["wsl2_cuda_L32_full.csv"]
    lowest = ensemble_series(lowest_groups, "coupled", (1, 0, 0), 1)
    initial = lowest[0]
    a16 = (lowest[16] * initial.conjugate()).real / abs(initial) ** 2
    checks.append(("DESCRIPTIVE A16 REPRODUCED",
                   close(a16, manifest["descriptive_only"]["A16_L32_100_n1"],
                         1e-10)))
    checks.append(("OUTCOME D RECORDED",
                   manifest["outcome"] == "D"
                   and manifest["status"] == "INVALID_ANALYSIS_MODEL"
                   and manifest["physics_inference"] == "NONE"
                   and not manifest["infrared_fit_admissible"]
                   and not manifest["analysis_validity"]["L64_execution_complete"]))

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(
        f"\nFTD-0431 result checks: {len(checks) - failed}/{len(checks)} passed; "
        f"max coupled normalized RMS={max(coupled_rms):.12g}; "
        f"A16={a16:.12g}; outcome={manifest['outcome']}"
    )
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
