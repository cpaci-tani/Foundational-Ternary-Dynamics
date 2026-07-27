"""Apply every preregistered FTD-0428 gate to the run-of-record CSVs."""

from __future__ import annotations

import csv
from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "engine/results/ftd_0428/manifest.json"


def load_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def maximum(rows: list[dict[str, str]], column: str) -> float:
    return max(abs(float(row[column])) for row in rows)


def dataset_checks(run: dict[str, object], rows: list[dict[str, str]]) -> list[tuple[str, bool]]:
    label = f"{run['backend']} L={run['L']}"
    L = int(run["L"])
    static = [row for row in rows if row["arm"] == "static_dressing"]
    movement = [row for row in rows if row["arm"] == "movement"]
    wave = [row for row in rows if row["arm"] == "transverse_wave"]
    return [
        (f"{label} row contract", len(rows) == 11 and len(static) == 4 and
         len(movement) == 6 and len(wave) == 1),
        (f"{label} metadata", {row["backend"] for row in rows} == {run["backend"]} and
         {int(row["L"]) for row in rows} == {L}),
        (f"{label} all rows valid", all(row["valid"] == "1" for row in rows)),
        (f"{label} solver convergence", all(
            int(row["init_iterations"]) <= 12 * L and
            float(row["solver_residual"]) <= 1e-10 for row in rows)),
        (f"{label} initialized longitudinal identity",
         maximum(rows, "curl_residual") <= 1e-10),
        (f"{label} projection-free Gauss", maximum(rows, "gauss_max") <= 1e-9),
        (f"{label} voxel mirror", maximum(rows, "voxel_sync_max") <= 1e-9),
        (f"{label} static radii", {int(row["radius"]) for row in static} == {2, 3, 4, 5}),
        (f"{label} static surface", maximum(static, "surface_error") <= 1e-9),
        (f"{label} minimum energy beats string", all(
            float(row["minimum_energy"]) < float(row["string_energy"])
            for row in static)),
        (f"{label} static energy", all(
            float(row["energy_drift_max"]) <=
            1e-9 * abs(float(row["modified_energy_initial"]))
            for row in static)),
        (f"{label} movement arms",
         {(int(row["polarity"]), row["direction"]) for row in movement} ==
         {(q, direction) for q in (+1, -1) for direction in ("+x", "+y", "+z")}),
        (f"{label} movement count", all(float(row["current_l1"]) >= 5.0
                                         for row in movement)),
        (f"{label} movement reactions absent", all(int(row["reaction_l1"]) == 0
                                                    for row in movement)),
        (f"{label} stationary current zero", all(
            float(row["stationary_current_max"]) == 0.0 for row in movement)),
        (f"{label} movement surface", maximum(movement, "surface_error") <= 1e-9),
        (f"{label} wave Gauss", maximum(wave, "gauss_max") <= 1e-10),
        (f"{label} wave modified energy", all(
            float(row["energy_drift_max"]) <=
            1e-8 * abs(float(row["modified_energy_initial"]))
            for row in wave)),
        (f"{label} wave causal support", all(
            int(row["support_excess"]) <= 0 for row in wave)),
        (f"{label} wave propagates outward", all(
            int(row["support_tick12"]) >= int(row["support_initial"]) + 3
            for row in wave)),
    ]


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    datasets: dict[tuple[str, int], list[dict[str, str]]] = {}
    for run in manifest["runs"]:
        path = ROOT / str(run["output"])
        checks.append((f"HASH {run['backend']} L={run['L']}",
                       sha256(path.read_bytes()).hexdigest() == run["sha256"]))
        rows = load_rows(path)
        datasets[(str(run["backend"]), int(run["L"]))] = rows
        checks.extend(dataset_checks(run, rows))

    numeric = (
        "solver_residual", "gauss_max", "curl_residual", "surface_flux",
        "surface_error", "minimum_energy", "string_energy",
        "modified_energy_initial", "energy_drift_max", "current_l1",
        "stationary_current_max", "voxel_sync_max",
    )
    discrete = (
        "arm", "polarity", "direction", "radius", "ticks", "init_valid",
        "init_iterations", "reaction_l1", "support_initial", "support_tick12",
        "support_final", "support_excess", "valid",
    )
    for L in (32, 64):
        windows = datasets[("windows-msvc-14.44", L)]
        wsl = datasets[("wsl2-gcc", L)]
        checks.append((f"MSVC/GCC row alignment L={L}", all(
            all(a[column] == b[column] for column in discrete)
            for a, b in zip(windows, wsl))))
        delta = max(abs(float(a[column]) - float(b[column]))
                    for a, b in zip(windows, wsl) for column in numeric)
        checks.append((f"MSVC/GCC scalar agreement L={L}", delta <= 1e-9))

    checks.append(("locked outcome A",
                   manifest["verdict"] ==
                   "A_SELECTED_INTEGRATED_PROJECTION_FREE_MAXWELL"))
    checks.append(("status remains selected, not native",
                   manifest["epistemic_status"] ==
                   "selected_engine_extension_measured_compatible"))

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    for (backend, L), rows in datasets.items():
        wave = next(row for row in rows if row["arm"] == "transverse_wave")
        print(
            f"{backend} L={L}: gauss={maximum(rows, 'gauss_max'):.9g}, "
            f"surface={maximum(rows, 'surface_error'):.9g}, "
            f"drift={maximum(rows, 'energy_drift_max'):.9g}, "
            f"CG={max(int(row['init_iterations']) for row in rows)}, "
            f"support={wave['support_initial']}->{wave['support_tick12']}"
            f"->{wave['support_final']}"
        )
    print(f"\nMatched Maxwell integration result checks: "
          f"{len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
