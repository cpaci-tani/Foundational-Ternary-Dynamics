"""Verify FTD-0432 records and reproduce its locked outcome gates."""

from __future__ import annotations

import csv
from collections import defaultdict
from hashlib import sha256
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine/results/ftd_0432"
MANIFEST = RESULTS / "manifest.json"
MODES = (((1, 0, 0), 1, "100_n1"),
         ((1, 1, 0), 2, "110_n2"),
         ((1, 1, 1), 3, "111_n3"))


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def key(row: dict[str, str]) -> tuple[object, ...]:
    return (row["arm"], int(row["seed"]), int(row["dx"]),
            int(row["dy"]), int(row["dz"]), int(row["n"]),
            int(row["tick"]))


def group_key(row: dict[str, str]) -> tuple[object, ...]:
    return (row["arm"], int(row["dx"]), int(row["dy"]),
            int(row["dz"]), int(row["n"]), int(row["tick"]))


def expected_keys() -> set[tuple[object, ...]]:
    out: set[tuple[object, ...]] = set()
    for direction, n, _ in MODES:
        for tick in range(32):
            for seed in range(8):
                out.add(("isolated", seed, *direction, n, tick))
                out.add(("coupled", seed, *direction, n, tick))
            out.add(("locked_control", 0, *direction, n, tick))
    return out


def complex_field(row: dict[str, str], prefix: str) -> complex:
    return complex(float(row[f"{prefix}_real"]),
                   float(row[f"{prefix}_imag"]))


def close(a: float, b: float, tolerance: float = 1e-10) -> bool:
    return abs(a - b) <= tolerance * max(1.0, abs(a), abs(b))


def ensemble_metrics(rows: list[dict[str, str]]) -> tuple[list[float], list[float]]:
    groups: dict[tuple[object, ...], list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        if row["arm"] != "locked_control":
            groups[group_key(row)].append(row)
    source_z: list[float] = []
    occupancy_z: list[float] = []
    volume = 32 ** 3
    for samples in groups.values():
        if len(samples) != 8:
            raise RuntimeError("incomplete seed ensemble")
        variance_sum = sum(float(row["removal_variance"])
                           for row in samples)
        sigma_source = math.sqrt(variance_sum) / (8.0 * volume)
        sigma_occupancy = math.sqrt(variance_sum) / 8.0
        predicted = sum(complex_field(row, "predicted_next")
                        for row in samples) / 8.0
        actual = sum(complex_field(row, "source_after")
                     for row in samples) / 8.0
        expected_removed = sum(float(row["expected_removals"])
                               for row in samples) / 8.0
        actual_removed = sum(float(row["actual_removed"])
                             for row in samples) / 8.0
        source_z.append(abs(actual - predicted) / max(1e-15, sigma_source))
        occupancy_z.append(abs(actual_removed - expected_removed)
                           / max(1e-15, sigma_occupancy))
    return source_z, occupancy_z


def coupled_feedback(
    rows: list[dict[str, str]], direction: tuple[int, int, int], n: int
) -> tuple[float, float, float, float, float]:
    selected = [row for row in rows if row["arm"] == "coupled"
                and tuple(int(row[name]) for name in ("dx", "dy", "dz"))
                == direction and int(row["n"]) == n]
    by_tick: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in selected:
        by_tick[int(row["tick"])].append(row)
    hazards: list[float] = []
    mean_probabilities: list[float] = []
    mean_energies: list[float] = []
    for tick in range(32):
        samples = by_tick[tick]
        source = sum(complex_field(row, "source_before")
                     for row in samples) / 8.0
        loss = sum(complex_field(row, "expected_loss")
                   for row in samples) / 8.0
        hazards.append((loss * source.conjugate()).real / abs(source) ** 2)
        mean_probabilities.append(sum(float(row["mean_site_probability"])
                                      for row in samples) / 8.0)
        mean_energies.append(sum(float(row["mean_local_energy"])
                                 for row in samples) / 8.0)
    return (min(hazards), max(hazards), max(hazards) - min(hazards),
            min(mean_probabilities), max(mean_energies))


def rms(values: list[float]) -> float:
    return math.sqrt(sum(value * value for value in values) / len(values))


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    records: dict[str, list[dict[str, str]]] = {}

    for name, metadata in manifest["valid_records"].items():
        path = RESULTS / name
        rows = read_rows(path)
        records[name] = rows
        checks.append((f"HASH {name}", sha256(path.read_bytes()).hexdigest()
                       == metadata["sha256"]))
        checks.append((f"ROWS/KEYS {name}", len(rows) == metadata["rows"]
                       and len({key(row) for row in rows}) == len(rows)
                       and {key(row) for row in rows} == expected_keys()))
        checks.append((f"BACKEND {name}", all(
            row["backend_label"] == metadata["backend"]
            and row["actual_backend"] == metadata["actual_backend"]
            and int(row["L"]) == 32 for row in rows)))
        checks.append((f"EXECUTION {name}", all(
            row["toggles_valid"] == "1"
            and row["structural_valid"] == "1"
            and row["execution_valid"] == "1" for row in rows)))
        checks.append((f"PROBABILITY BOUNDS {name}", all(
            0.0 <= float(row["min_site_probability"])
            <= float(row["max_site_probability"]) <= 0.1 + 1e-15
            for row in rows)))

        isolated = [row for row in rows if row["arm"] == "isolated"]
        checks.append((f"ISOLATED EXACT HAZARD {name}", all(
            abs(float(row["mean_site_probability"]) - 0.1) <= 1e-12
            and abs(float(row["source_hazard"]) - 0.1) <= 1e-12
            and float(row["max_local_energy"]) <= 1e-14
            for row in isolated)))
        locked = [row for row in rows if row["arm"] == "locked_control"]
        checks.append((f"LOCKED ZERO HAZARD {name}", all(
            int(row["eligible_sites"]) == 0
            and float(row["expected_removals"]) == 0.0
            and int(row["actual_removed"]) == 0
            and abs(complex_field(row, "source_after")
                    - complex_field(row, "source_before")) <= 1e-14
            for row in locked)))
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

        source_z, occupancy_z = ensemble_metrics(rows)
        checks.append((f"SOURCE EXPECTATION GATES {name}",
                       max(source_z) <= 6.0 and rms(source_z) <= 2.5))
        checks.append((f"OCCUPANCY EXPECTATION GATES {name}",
                       max(occupancy_z) <= 6.0
                       and rms(occupancy_z) <= 2.5))

    reference = records["windows_msvc_cpu_L32.csv"]
    comparison = records["wsl2_cuda_L32.csv"]
    checks.append(("CPU/CUDA ROW ORDER", [key(row) for row in reference]
                   == [key(row) for row in comparison]))
    backend_fields = ("source_before", "expected_loss", "predicted_next",
                      "source_after")
    backend_ok = True
    for left, right in zip(reference, comparison):
        backend_ok &= all(abs(complex_field(left, field)
                              - complex_field(right, field)) <= 1e-10
                          for field in backend_fields)
    checks.append(("CPU/CUDA SOURCE AGREEMENT", backend_ok))

    source_z, occupancy_z = ensemble_metrics(comparison)
    summary = manifest["conditional_expectation"]
    checks.append(("MANIFEST STANDARDIZED RESIDUALS",
                   close(max(source_z), summary["source_z_max"])
                   and close(rms(source_z), summary["source_z_rms"])
                   and close(max(occupancy_z), summary["occupancy_z_max"])
                   and close(rms(occupancy_z), summary["occupancy_z_rms"])))

    for direction, n, label in MODES:
        feedback = coupled_feedback(comparison, direction, n)
        recorded = manifest["coupled_feedback"][label]
        checks.append((f"FEEDBACK GATES {label}", feedback[2] >= 0.02
                       and feedback[0] <= 0.05 and feedback[3] <= 0.05))
        checks.append((f"MANIFEST FEEDBACK {label}",
                       close(feedback[0], recorded["hazard_min"])
                       and close(feedback[1], recorded["hazard_max"])
                       and close(feedback[2], recorded["hazard_range"])
                       and close(feedback[3],
                                 recorded["mean_site_probability_min"])
                       and close(feedback[4],
                                 recorded["mean_local_energy_max"])))

    checks.append(("OUTCOME A RECORDED", manifest["outcome"] == "A"
                   and manifest["status"]
                   == "DRESSED_HAZARD_EXPLAINS_NONEXPONENTIALITY"
                   and manifest["scope"] == "MECHANISM_VALIDATION_ONLY"
                   and manifest["conservation_inference"] == "NONE"
                   and manifest["infrared_inference"] == "NONE"))

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nFTD-0432 result checks: {len(checks) - failed}/{len(checks)} "
          f"passed; zS_max={max(source_z):.6g}; "
          f"zN_max={max(occupancy_z):.6g}; outcome={manifest['outcome']}")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
