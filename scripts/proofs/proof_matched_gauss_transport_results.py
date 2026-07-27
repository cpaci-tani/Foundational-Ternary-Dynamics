"""Apply the frozen FTD-0427 gates to all run-of-record CSV files."""

from __future__ import annotations

import csv
from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "engine/results/ftd_0427/manifest.json"
TOL = 1e-12
DIRECTIONS = ("+x", "-x", "+y", "-y", "+z", "-z")


def load_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def unique_steps(rows: list[dict[str, str]]) -> dict[tuple[int, str, int], dict[str, str]]:
    out: dict[tuple[int, str, int], dict[str, str]] = {}
    for row in rows:
        key = (int(row["orientation"]), row["direction"], int(row["tick"]))
        out.setdefault(key, row)
    return out


def maximum(rows: list[dict[str, str]], column: str, absolute: bool = True) -> float:
    values = [float(row[column]) for row in rows]
    if absolute:
        values = [abs(value) for value in values]
    return max(values)


def dataset_checks(run: dict[str, object], rows: list[dict[str, str]]) -> list[tuple[str, bool]]:
    label = str(run["backend"])
    steps = unique_steps(rows)
    moving = [row for (_, _, tick), row in steps.items() if tick <= 12]
    stationary = [row for (_, _, tick), row in steps.items() if tick > 12]
    arms = {(int(row["orientation"]), row["direction"]) for row in rows}
    movement_counts = {
        arm: sum(
            int(row["transported_events"])
            for (q, direction, _), row in steps.items()
            if (q, direction) == arm
        )
        for arm in arms
    }
    expected_arms = {(q, direction) for q in (+1, -1) for direction in DIRECTIONS}
    return [
        (f"{label} row contract", len(rows) == 720 and len(steps) == 240),
        (f"{label} metadata contract",
         {row["backend"] for row in rows} == {label} and
         {int(row["L"]) for row in rows} == {int(run["L"])}),
        (f"{label} complete arm set", arms == expected_arms),
        (f"{label} all rows valid", all(row["valid"] == "1" for row in rows)),
        (f"{label} no reactions",
         all(int(row["annihilation_pairs"]) == 0 and
             int(row["reaction_sites"]) == 0 and
             int(row["reaction_l1"]) == 0 for row in rows)),
        (f"{label} movement count gate",
         all(count >= 5 for count in movement_counts.values())),
        (f"{label} exact transport", maximum(rows, "transport_residual") <= TOL),
        (f"{label} exact div curl", maximum(rows, "curl_divergence") <= TOL),
        (f"{label} projection-free Gauss", maximum(rows, "gauss_residual") <= TOL),
        (f"{label} surface telescope", maximum(rows, "telescope_residual") <= TOL),
        (f"{label} surface charge",
         max(abs(float(row["boundary_flux"]) - int(row["orientation"]))
             for row in rows) <= TOL),
        (f"{label} radius plateau", maximum(rows, "plateau") <= TOL),
        (f"{label} global neutrality",
         all(int(row["total_state"]) == 0 for row in rows)),
        (f"{label} transverse challenge nonzero",
         min(float(row["curl_l1"]) for row in rows) > 0.0),
        (f"{label} stationary current zero",
         len(stationary) == 96 and all(float(row["current_l1"]) == 0.0
                                      for row in stationary)),
        (f"{label} moving interval present", len(moving) == 144),
    ]


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    datasets: dict[tuple[str, int], list[dict[str, str]]] = {}

    for run in manifest["runs"]:
        path = ROOT / run["output"]
        checks.append((f"HASH {run['backend']} L={run['L']}",
                       sha256(path.read_bytes()).hexdigest() == run["sha256"]))
        rows = load_rows(path)
        datasets[(run["backend"], int(run["L"]))] = rows
        checks.extend(dataset_checks(run, rows))

    numeric_columns = (
        "transported_events", "annihilation_pairs", "reaction_sites",
        "reaction_l1", "current_l1", "transport_residual", "curl_l1",
        "curl_divergence", "gauss_residual", "total_state", "radius",
        "boundary_flux", "divergence_sum", "telescope_residual", "plateau",
    )
    for L in (32, 64):
        windows = datasets[("windows_msvc_cpu", L)]
        wsl = datasets[("wsl2_gcc_cpu", L)]
        same_keys = all(
            (a["orientation"], a["direction"], a["stage"], a["tick"], a["radius"])
            == (b["orientation"], b["direction"], b["stage"], b["tick"], b["radius"])
            for a, b in zip(windows, wsl)
        )
        max_delta = max(
            abs(float(a[column]) - float(b[column]))
            for a, b in zip(windows, wsl)
            for column in numeric_columns
        )
        checks.append((f"MSVC/GCC row alignment L={L}", same_keys))
        checks.append((f"MSVC/GCC numeric agreement L={L}", max_delta <= TOL))

    checks.append(("locked verdict is outcome A",
                   manifest["verdict"] == "A_SELECTED_LOCAL_PROJECTION_FREE_TRANSPORT"))

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed

    for (backend, L), rows in datasets.items():
        print(
            f"{backend} L={L}: gauss={maximum(rows, 'gauss_residual'):.9g}, "
            f"divcurl={maximum(rows, 'curl_divergence'):.9g}, "
            f"surface={max(abs(float(row['boundary_flux']) - int(row['orientation'])) for row in rows):.9g}, "
            f"telescope={maximum(rows, 'telescope_residual'):.9g}"
        )

    print(f"\nMatched Gauss transport result checks: {len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
