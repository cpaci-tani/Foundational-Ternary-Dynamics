"""Verify locked FTD-0429 result records and the infrared decision."""

from __future__ import annotations

import csv
from hashlib import sha256
import json
import math
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine/results/ftd_0429"
MANIFEST = RESULTS / "manifest.json"


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return [row for row in csv.DictReader(handle) if row.get("arm")]


def row_key(row: dict[str, str]) -> tuple[object, ...]:
    return (
        row["arm"],
        int(row["dx"]), int(row["dy"]), int(row["dz"]),
        int(row["base"]), int(row["duty"]), int(row["orientation"]),
        int(row["n"]),
    )


def expected_full_keys() -> set[tuple[object, ...]]:
    keys: set[tuple[object, ...]] = set()
    for name, direction in (
        ("primary_100", (1, 0, 0)),
        ("primary_110", (1, 1, 0)),
        ("primary_111", (1, 1, 1)),
    ):
        for base, harmonics in ((1, (1, 3)), (2, (2,))):
            for orientation in (1, -1):
                for n in harmonics:
                    keys.add((name, *direction, base, 2, orientation, n))
    for base, harmonics in ((1, (1, 3)), (2, (2,))):
        for duty in (1, 4):
            for n in harmonics:
                keys.add(("amplitude_100", 1, 0, 0, base, duty, 1, n))
    return keys


def expected_infrared_keys() -> set[tuple[object, ...]]:
    return {
        key for key in expected_full_keys()
        if str(key[0]).startswith("primary_") and key[6] == 1
    }


def relative_close(a: float, b: float, tolerance: float) -> bool:
    return abs(a - b) <= tolerance * max(1.0, abs(a), abs(b))


def solve_normal_equations(
    design: list[list[float]], values: list[float]
) -> list[float]:
    columns = len(design[0])
    augmented = [
        [
            sum(row[i] * row[j] for row in design)
            for j in range(columns)
        ]
        + [sum(row[i] * value for row, value in zip(design, values))]
        for i in range(columns)
    ]
    for pivot in range(columns):
        best = max(range(pivot, columns), key=lambda row: abs(augmented[row][pivot]))
        augmented[pivot], augmented[best] = augmented[best], augmented[pivot]
        scale = augmented[pivot][pivot]
        if abs(scale) < 1e-18:
            raise RuntimeError("singular locked regression")
        for column in range(pivot, columns + 1):
            augmented[pivot][column] /= scale
        for row in range(columns):
            if row == pivot:
                continue
            factor = augmented[row][pivot]
            for column in range(pivot, columns + 1):
                augmented[row][column] -= factor * augmented[pivot][column]
    return [augmented[row][columns] for row in range(columns)]


def features(row: dict[str, str], include_constant: bool) -> list[float]:
    size = int(row["L"])
    n = int(row["n"])
    direction = [int(row[name]) for name in ("dx", "dy", "dz")]
    k = [2.0 * math.pi * n * component / size for component in direction]
    q2 = sum(component * component for component in k)
    h4 = sum(component**4 for component in k) / q2
    terms = [q2, h4, q2 * q2]
    return [1.0, *terms] if include_constant else terms


def regress(
    rows: list[dict[str, str]], include_constant: bool
) -> tuple[list[float], float, float, float]:
    design = [features(row, include_constant) for row in rows]
    values = [float(row["z_real"]) for row in rows]
    coefficients = solve_normal_equations(design, values)
    predicted = [
        sum(value * coefficient for value, coefficient in zip(row, coefficients))
        for row in design
    ]
    rss = sum((value - prediction) ** 2
              for value, prediction in zip(values, predicted))
    rms = math.sqrt(rss / len(values))
    bic = (
        len(values) * math.log(max(rss, 1e-12) / len(values))
        + len(coefficients) * math.log(len(values))
    )
    return coefficients, rss, rms, bic


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    records: dict[str, list[dict[str, str]]] = {}

    for name, metadata in manifest["valid_records"].items():
        path = RESULTS / name
        checks.append((f"HASH {name}", sha256(path.read_bytes()).hexdigest()
                       == metadata["sha256"]))
        rows = read_rows(path)
        records[name] = rows
        checks.append((f"ROWS {name}", len(rows) == metadata["rows"]))
        expected = (expected_full_keys() if metadata["profile"] == "full"
                    else expected_infrared_keys())
        checks.append((f"KEYS {name}", len({row_key(row) for row in rows}) == len(rows)
                       and {row_key(row) for row in rows} == expected))
        expected_actual = "cpu" if "cpu" in metadata["backend"] else "gpu"
        checks.append((f"BACKEND {name}", all(
            row["actual_backend"] == expected_actual
            and int(row["L"]) == metadata["L"] for row in rows)))
        checks.append((f"VALIDITY {name}", all(
            row["valid"] == "1"
            and row["state_unchanged"] == "1"
            and row["forbidden_toggles_off"] == "1"
            and int(row["initial_charge"]) == 0
            and int(row["final_charge"]) == 0
            and int(row["samples"]) == 17
            and float(row["source_abs"]) >= 1e-3
            for row in rows)))
        checks.append((f"FIT GATES {name}", all(
            float(row["fit_residual"]) <= 1e-8
            and float(row["relative_error"]) <= 1e-7
            and abs(float(row["z_imag"]))
                <= 1e-8 * max(1.0, abs(float(row["z_real"])))
            for row in rows)))

    invalid_name, invalid_metadata = next(iter(
        manifest["invalid_run_provenance"].items()))
    invalid_path = RESULTS / invalid_name
    checks.append(("INVALID provenance preserved",
                   sha256(invalid_path.read_bytes()).hexdigest()
                   == invalid_metadata["sha256"]
                   and invalid_name not in manifest["valid_records"]))

    for dataset in ("windows_msvc_L32.csv", "wsl2_cuda_L32.csv"):
        by_key = {row_key(row): row for row in records[dataset]}
        mirror_ok = True
        for key, positive in by_key.items():
            if not str(key[0]).startswith("primary_") or key[6] != 1:
                continue
            negative_key = (*key[:6], -1, key[7])
            negative = by_key[negative_key]
            mirror_ok &= relative_close(float(positive["z_real"]),
                                        float(negative["z_real"]), 1e-7)
            mirror_ok &= relative_close(float(positive["z_imag"]),
                                        float(negative["z_imag"]), 1e-7)
        checks.append((f"MIRROR {dataset}", mirror_ok))

        amplitude_ok = True
        for base, harmonics in ((1, (1, 3)), (2, (2,))):
            for n in harmonics:
                reference = by_key[("primary_100", 1, 0, 0, base, 2, 1, n)]
                for duty in (1, 4):
                    control = by_key[("amplitude_100", 1, 0, 0,
                                      base, duty, 1, n)]
                    amplitude_ok &= relative_close(float(reference["z_real"]),
                                                   float(control["z_real"]), 1e-7)
                    amplitude_ok &= relative_close(float(reference["z_imag"]),
                                                   float(control["z_imag"]), 1e-7)
        checks.append((f"AMPLITUDE {dataset}", amplitude_ok))

    windows = {row_key(row): row for row in records["windows_msvc_L32.csv"]}
    wsl = {row_key(row): row for row in records["wsl2_cuda_L32.csv"]}
    compiler_ok = windows.keys() == wsl.keys()
    for key in windows.keys() & wsl.keys():
        for field in ("source_real", "source_imag", "omega", "z_real",
                      "z_imag", "z_exact", "fit_residual"):
            compiler_ok &= relative_close(float(windows[key][field]),
                                          float(wsl[key][field]), 1e-6)
    checks.append(("MSVC/CUDA-GCC scalar agreement L=32", compiler_ok))

    fit_rows = [
        row
        for name in ("wsl2_cuda_L32.csv", "wsl2_cuda_L64.csv")
        for row in records[name]
        if row["arm"].startswith("primary_")
        and int(row["orientation"]) == 1 and int(row["duty"]) == 2
    ]
    constant = regress(fit_rows, True)
    zero = regress(fit_rows, False)
    delta_bic = zero[3] - constant[3]

    gauge_text = (ROOT / "engine/include/ftd/ontic/gauge_couplings.h").read_text(
        encoding="utf-8"
    )
    match = re.search(r"inline constexpr double G_C = ([0-9.eE+-]+);", gauge_text)
    if not match:
        raise RuntimeError("could not read canonical G_C")
    three_g_c = 3.0 * float(match.group(1))
    intercept_error = abs(constant[0][0] - three_g_c) / three_g_c
    checks.extend(
        [
            ("IR fit row contract", len(fit_rows) == 18),
            ("constant model RMS", constant[2] <= 1e-4),
            ("zero-intercept model rejected", delta_bic >= 10.0),
            ("finite positive infrared intercept",
             constant[0][0] > 0.0 and intercept_error <= 0.01),
            ("manifest constant fit reproduced", all(
                relative_close(actual, expected, 1e-11)
                for actual, expected in zip(
                    constant[0], manifest["infrared_fit"]["constant_model"]["coefficients"]
                ))),
            ("manifest zero fit reproduced", all(
                relative_close(actual, expected, 1e-11)
                for actual, expected in zip(
                    zero[0], manifest["infrared_fit"]["zero_model"]["coefficients"]
                ))),
            ("locked outcome A", manifest["outcome"] == "A"),
        ]
    )

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(
        f"\nZ0={constant[0][0]:.15g}; 3G_C={three_g_c:.15g}; "
        f"relative={intercept_error:.6g}"
    )
    print(
        f"constant RMS={constant[2]:.6g}; delta_BIC={delta_bic:.6f}; "
        f"fit rows={len(fit_rows)}"
    )
    print(
        f"Native dynamic polarity-response result checks: "
        f"{len(checks) - failed}/{len(checks)} passed"
    )
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
