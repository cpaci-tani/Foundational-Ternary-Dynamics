"""Verify the FTD-0430 v2 result records and locked outcome gates."""

from __future__ import annotations

import csv
from hashlib import sha256
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine/results/ftd_0430"
MANIFEST = RESULTS / "manifest.json"
FTD_0429_MANIFEST = ROOT / "engine/results/ftd_0429/manifest.json"


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def row_key(row: dict[str, str]) -> tuple[int, int, int, int, int]:
    return tuple(int(row[name]) for name in ("orientation", "dx", "dy", "dz", "n"))


def expected_keys(profile: str) -> set[tuple[int, int, int, int, int]]:
    orientations = (1, -1) if profile == "full" else (1,)
    directions = ((1, 0, 0), (1, 1, 0), (1, 1, 1))
    return {
        (orientation, *direction, n)
        for orientation in orientations
        for direction in directions
        for n in (1, 2, 3)
    }


def relative_close(a: float, b: float, tolerance: float) -> bool:
    return abs(a - b) <= tolerance * max(1.0, abs(a), abs(b))


def solve_normal_equations(
    design: list[list[float]], values: list[float]
) -> list[float]:
    columns = len(design[0])
    augmented = [
        [sum(row[i] * row[j] for row in design) for j in range(columns)]
        + [sum(row[i] * value for row, value in zip(design, values))]
        for i in range(columns)
    ]
    for pivot in range(columns):
        best = max(range(pivot, columns),
                   key=lambda row: abs(augmented[row][pivot]))
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


def features(
    row: dict[str, str], include_constant: bool, normalized_h4: bool
) -> list[float]:
    size = int(row["L"])
    n = int(row["n"])
    k = [
        2.0 * math.pi * n * int(row[name]) / size
        for name in ("dx", "dy", "dz")
    ]
    q2 = sum(component * component for component in k)
    raw_h4 = sum(component**4 for component in k)
    h4 = raw_h4 / q2 if normalized_h4 else raw_h4
    terms = [q2, h4, q2 * q2]
    return [1.0, *terms] if include_constant else terms


def regress(
    rows: list[dict[str, str]], include_constant: bool, normalized_h4: bool
) -> tuple[list[float], float, float, float]:
    design = [features(row, include_constant, normalized_h4) for row in rows]
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


def close_sequence(actual: list[float], expected: list[float]) -> bool:
    return len(actual) == len(expected) and all(
        relative_close(a, b, 1e-10) for a, b in zip(actual, expected)
    )


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    prior = json.loads(FTD_0429_MANIFEST.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    records: dict[str, list[dict[str, str]]] = {}

    for name, metadata in manifest["valid_records"].items():
        path = RESULTS / name
        checks.append((f"HASH {name}", sha256(path.read_bytes()).hexdigest()
                       == metadata["sha256"]))
        rows = read_rows(path)
        records[name] = rows
        checks.append((f"ROWS {name}", len(rows) == metadata["rows"]))
        keys = {row_key(row) for row in rows}
        checks.append((f"KEYS {name}", len(keys) == len(rows)
                       and keys == expected_keys(metadata["profile"])))
        expected_backend = "cpu" if "cpu" in metadata["backend"] else "gpu"
        checks.append((f"BACKEND/PROFILE {name}", all(
            row["actual_backend"] == expected_backend
            and int(row["L"]) == metadata["L"]
            and row["profile"] == metadata["profile"] for row in rows)))
        checks.append((f"EXECUTION {name}", all(
            row["exact_hop"] == "1"
            and row["states_frozen"] == "1"
            and row["toggles_valid"] == "1"
            and row["backend_valid"] == "1"
            and row["execution_valid"] == "1"
            and int(row["reaction_events"]) == 0
            and int(row["samples"]) == 18
            and float(row["delta_source_abs"]) >= 1e-8
            for row in rows)))
        expected_movements = 2 if expected_backend == "cpu" else 0
        checks.append((f"MOVEMENT JOURNAL {name}", all(
            int(row["movement_events"]) == expected_movements for row in rows)))
        checks.append((f"CAUSAL CONE {name}", all(
            row["causal_pass"] == "1"
            and float(row["tau0_max_abs"]) <= 1e-11
            and float(row["tau1_max_abs"]) > 1e-10
            and float(row["max_outside_cone"]) <= 1e-11
            and int(row["final_support_radius"]) > 2
            for row in rows)))
        checks.append((f"POLE/RESIDUE {name}", all(
            row["mode_pass"] == "1" and row["advance"] == "1"
            and float(row["fit_residual"]) <= 1e-7
            and float(row["z_relative_error"]) <= 1e-6
            and float(row["residue_relative_error"]) <= 1e-5
            and abs(float(row["z_imag"]))
                <= 1e-7 * max(1.0, abs(float(row["z_real"])))
            for row in rows)))

    for name, metadata in manifest["invalid_v1_analysis_provenance"].items():
        path = RESULTS / name
        checks.append((f"V1 provenance {name}",
                       sha256(path.read_bytes()).hexdigest() == metadata["sha256"]
                       and name not in manifest["valid_records"]))

    windows = {row_key(row): row for row in records["windows_msvc_L48.csv"]}
    wsl = {row_key(row): row for row in records["wsl2_cuda_L48.csv"]}
    backend_ok = windows.keys() == wsl.keys()
    for key in windows.keys() & wsl.keys():
        for field in ("delta_source_real", "delta_source_imag", "omega",
                      "z_real", "z_imag", "z_exact", "fit_residual",
                      "residue_ratio"):
            backend_ok &= relative_close(
                float(windows[key][field]), float(wsl[key][field]), 1e-5)
    checks.append(("MSVC/CUDA-GCC agreement L=48", backend_ok))

    mirror_ok = True
    for dataset in ("windows_msvc_L48.csv", "wsl2_cuda_L48.csv"):
        by_key = {row_key(row): row for row in records[dataset]}
        for direction in ((1, 0, 0), (1, 1, 0), (1, 1, 1)):
            for n in (1, 2, 3):
                positive = by_key[(1, *direction, n)]
                negative = by_key[(-1, *direction, n)]
                mirror_ok &= relative_close(float(positive["z_real"]),
                                            float(negative["z_real"]), 1e-5)
                mirror_ok &= relative_close(float(positive["z_imag"]),
                                            float(negative["z_imag"]), 1e-5)
    checks.append(("GLOBAL POLARITY MIRROR", mirror_ok))

    v1_rows = [
        row
        for name in ("wsl2_cuda_L32.csv", "wsl2_cuda_L64.csv")
        for row in read_rows(RESULTS / name)
        if int(row["orientation"]) == 1
    ]
    v1_literal = regress(v1_rows, True, normalized_h4=False)
    v1_manifest = manifest["v1_literal_invalid_fit"]
    checks.append(("V1 literal fit reproduced",
                   close_sequence(v1_literal[0],
                                  v1_manifest["constant_model"]["coefficients"])
                   and relative_close(v1_literal[2],
                                      v1_manifest["constant_model"]["rms"], 1e-10)))
    checks.append(("V1 is invalid rather than reinterpreted",
                   v1_literal[2] > 1e-4
                   and v1_manifest["verdict"]
                   == "D_INVALID_ANALYSIS_SPECIFICATION"))

    fit_rows = [
        row
        for name in ("wsl2_cuda_L48.csv", "wsl2_cuda_L96.csv")
        for row in records[name]
        if int(row["orientation"]) == 1
    ]
    constant = regress(fit_rows, True, normalized_h4=True)
    zero = regress(fit_rows, False, normalized_h4=True)
    delta_bic = zero[3] - constant[3]
    fit_manifest = manifest["infrared_fit"]
    checks.append(("V2 constant regression reproduced",
                   close_sequence(constant[0],
                                  fit_manifest["constant_model"]["coefficients"])
                   and relative_close(constant[1],
                                      fit_manifest["constant_model"]["rss"], 1e-10)
                   and relative_close(constant[2],
                                      fit_manifest["constant_model"]["rms"], 1e-10)
                   and relative_close(constant[3],
                                      fit_manifest["constant_model"]["bic"], 1e-10)))
    checks.append(("V2 zero regression reproduced",
                   close_sequence(zero[0],
                                  fit_manifest["zero_model"]["coefficients"])
                   and relative_close(zero[1],
                                      fit_manifest["zero_model"]["rss"], 1e-10)
                   and relative_close(zero[3],
                                      fit_manifest["zero_model"]["bic"], 1e-10)))

    z0 = constant[0][0]
    three_g_c = fit_manifest["three_g_c"]
    prior_z0 = prior["infrared_fit"]["z0"]
    checks.append(("NONZERO INTERCEPT BIC gate", delta_bic >= 10.0))
    checks.append(("INFRARED RMS gate", constant[2] <= 1e-4))
    checks.append(("3 G_C gate", abs(z0 - three_g_c) / three_g_c <= 0.01))
    checks.append(("FTD-0429 equality gate",
                   abs(z0 - prior_z0) / prior_z0 <= 0.002))
    checks.append(("MANIFEST outcome A",
                   manifest["outcome"] == "A"
                   and manifest["status"]
                   == "OUTCOME_A_RETARDED_NATIVE_COARSE_POLARITY_RESPONSE"))

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(
        f"\nFTD-0430 result checks: {len(checks) - failed}/{len(checks)} passed; "
        f"Z0={z0:.15g}; RMS={constant[2]:.6g}; delta_BIC={delta_bic:.6f}; "
        f"outcome={manifest['outcome']}"
    )
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
