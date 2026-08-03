"""Independent frozen-artifact certificate for FTD-0772.

The certificate reads the immutable FTD-0659 corpus directly.  It does not
import the FTD-0772 analyzer, rerun the engine, fit an exponent, or change the
registered signed coordinate.  Only the locked controls m = {2, 4, 6} are
evaluated.
"""

from __future__ import annotations

from bisect import bisect_right
from collections import Counter, defaultdict
import csv
import hashlib
import json
import math
from pathlib import Path
from typing import Any, Sequence

from scipy.special import betainc


ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT / "docs/theory/10_eft_program/preregistrations"
    / "PREREG_NATIVE_TEMPORAL_OCCUPANCY_v1.md"
)
PARENT_DIR = ROOT / "engine/results/ftd_0659"
RESULT_DIR = ROOT / "engine/results/ftd_0772"
PARENT_JSON = PARENT_DIR / "ftd_0659_native_excited_matter_clock_v1.json"
PARENT_ARMS = PARENT_DIR / "ftd_0659_native_excited_matter_clock_arms_v1.csv"
PARENT_TICKS = PARENT_DIR / "ftd_0659_native_excited_matter_clock_ticks_v1.csv"
RESULT_JSON = RESULT_DIR / "ftd_0772_native_temporal_occupancy_v1.json"
RESULT_CSV = RESULT_DIR / "ftd_0772_native_temporal_occupancy_cells_v1.csv"

PROTOCOL_SHA256 = "3E779CDFFDE5D17299921750A06E26B075000572CAB60E9DA8FBF154239CC41C"
PARENT_PROTOCOL_SHA256 = "FF9566F6D6B7BCAEB7970359043C62F643A6A8315AF43C01EE0C5CFD21ECC342"
HASHES = {
    PREREG: PROTOCOL_SHA256,
    PARENT_JSON: "DB6CA66770812E4C8FC94411B109F23E424FFF1CE3173A5D16AB43B5949ACEEE",
    PARENT_ARMS: "4F7D2E38B0FE4D6EF33F137E2AA753E4143B3AD541F6934CF39FD11772844941",
    PARENT_TICKS: "4EF51456F161E6CD836518B72EBAACE4A5007F5EF5525E07CD097B343566634A",
    RESULT_JSON: "FAD820D59EE5A0E7ED8FE22BB187FDD97544F085208219A56990E7ADDE86AD9B",
    RESULT_CSV: "600A00611D81AC329612617F0E60206820C1670791AE4A20862221E34D6173A3",
}

ARM_FIELDS = (
    "ftd_id", "label", "orientation", "polarization", "amplitude",
    "quadrature", "zero", "initialization", "forward", "reverse",
    "bounded", "phase_defined", "modal_amplitude", "initial_action",
    "max_action_drift", "min_support", "mean_phase_step",
    "phase_step_rms", "phase_error", "leakage", "max_center",
    "max_energy_drift", "max_common", "recovery",
)
TICK_FIELDS = (
    "ftd_id", "label", "tick", "q0", "q1", "p0", "p1", "action",
    "z_abs", "support", "raw_phase", "unwrapped_phase", "phase_step",
    "energy_drift", "common",
)
RESULT_FIELDS = (
    "orientation", "polarization", "amplitude_index", "sample_count",
    "amplitude", "cycles", "epsilon_perp", "max_abs_x", "mean", "mu1",
    "mu2", "mu4", "g_rms", "g_abs", "d2", "d4", "d6",
    "closest_control_m", "window_max_pairwise_ks",
    "window_max_moment_spread", "window_min_peak", "quadrature_max_ks",
    "quadrature_max_moment_delta", "applicability_pass", "quartic_pass",
)
FIXED_POWERS = (2, 4, 6)
WINDOWS = ((0, 85), (86, 171), (172, 256))
MOMENTS = ("mu1", "mu2", "mu4")
POLARIZATIONS = {
    0: (1.0, 0.0),
    1: (0.0, 1.0),
    2: (1.0 / math.sqrt(2.0), 1.0 / math.sqrt(2.0)),
}
G_STAR = math.gamma(0.25) / math.gamma(0.75)
TARGETS = {
    "mu1": math.sqrt(math.pi) / G_STAR,
    "mu2": 4.0 / G_STAR**2,
    "mu4": 1.0 / 3.0,
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def read_csv(path: Path) -> tuple[tuple[str, ...], list[dict[str, str]]]:
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        rows = list(reader)
        return tuple(reader.fieldnames or ()), rows


def truth(value: str) -> bool:
    return value.strip().lower() in {"1", "true"}


def moments(values: Sequence[float]) -> dict[str, float]:
    count = len(values)
    assert count > 0
    mu1 = math.fsum(abs(value) for value in values) / count
    mu2 = math.fsum(value * value for value in values) / count
    mu4 = math.fsum(value**4 for value in values) / count
    return {
        "mean": math.fsum(values) / count,
        "mu1": mu1,
        "mu2": mu2,
        "mu4": mu4,
        "g_rms": 2.0 / math.sqrt(mu2),
        "g_abs": math.sqrt(math.pi) / mu1,
        "max_abs_x": max(abs(value) for value in values),
    }


def target_cdf(power: int, value: float) -> float:
    assert power in FIXED_POWERS
    if value <= -1.0:
        return 0.0
    if value >= 1.0:
        return 1.0
    if value == 0.0:
        return 0.5
    regularized = float(betainc(1.0 / power, 0.5, abs(value) ** power))
    return 0.5 + math.copysign(0.5 * regularized, value)


def one_sample_distance(values: Sequence[float], power: int) -> float:
    ordered = sorted(values)
    count = len(ordered)
    distance = 0.0
    for index, value in enumerate(ordered, 1):
        cdf = target_cdf(power, value)
        distance = max(distance, index / count - cdf,
                       cdf - (index - 1) / count)
    return distance


def two_sample_distance(left: Sequence[float], right: Sequence[float]) -> float:
    # An independent empirical-CDF implementation using binary searches.
    a, b = sorted(left), sorted(right)
    return max(
        abs(bisect_right(a, value) / len(a)
            - bisect_right(b, value) / len(b))
        for value in sorted(set(a) | set(b))
    )


def pairwise_max(samples: Sequence[Sequence[float]]) -> float:
    return max(
        two_sample_distance(samples[i], samples[j])
        for i in range(len(samples)) for j in range(i + 1, len(samples))
    )


def range_of(records: Sequence[dict[str, float]], key: str) -> float:
    values = [record[key] for record in records]
    return max(values) - min(values)


def relative_error(value: float, target: float) -> float:
    return abs(value - target) / abs(target)


def relative_difference(left: float, right: float) -> float:
    return abs(left - right) / max(abs(left), abs(right), 1e-300)


def close_tree(actual: Any, expected: Any) -> bool:
    if isinstance(expected, bool):
        return actual is expected
    if isinstance(expected, dict):
        return (isinstance(actual, dict) and actual.keys() == expected.keys()
                and all(close_tree(actual[key], value)
                        for key, value in expected.items()))
    if isinstance(expected, list):
        return (isinstance(actual, list) and len(actual) == len(expected)
                and all(close_tree(a, e) for a, e in zip(actual, expected)))
    if isinstance(expected, float):
        return (isinstance(actual, (int, float))
                and math.isclose(float(actual), expected,
                                 rel_tol=2e-11, abs_tol=2e-14))
    return actual == expected


def arm_gate(arm: dict[str, str]) -> bool:
    return (
        all(truth(arm[key]) for key in
            ("initialization", "forward", "reverse", "bounded", "phase_defined"))
        and float(arm["min_support"]) >= 0.90
        and float(arm["phase_error"]) <= 0.02
        and float(arm["phase_step_rms"]) <= 0.05
        and float(arm["leakage"]) <= 0.10
        and float(arm["max_energy_drift"]) <= 1e-12
        and float(arm["max_common"]) <= 1e-10
        and float(arm["recovery"]) <= 1e-10
    )


def analyze_arm(
    label: str,
    arm: dict[str, str],
    tick_rows: Sequence[dict[str, str]],
) -> dict[str, Any]:
    u0, u1 = POLARIZATIONS[int(arm["polarization"])]
    amplitude = float(arm["modal_amplitude"])
    assert math.isfinite(amplitude) and amplitude > 0.0
    values: list[float] = []
    radial = transverse = 0.0
    for row in tick_rows:
        q0, q1 = float(row["q0"]), float(row["q1"])
        projected = u0 * q0 + u1 * q1
        values.append(projected / amplitude)
        norm_squared = q0 * q0 + q1 * q1
        radial += norm_squared
        transverse += max(0.0, norm_squared - projected * projected)
    phases = [float(row["unwrapped_phase"]) for row in tick_rows
              if math.isfinite(float(row["unwrapped_phase"]))]
    distances = {f"d{power}": one_sample_distance(values, power)
                 for power in FIXED_POWERS}
    closest = min(FIXED_POWERS,
                  key=lambda power: (distances[f"d{power}"], power))
    return {
        "label": label,
        "sample_count": len(values),
        "values": values,
        "amplitude": amplitude,
        "epsilon_perp": transverse / radial,
        "cycles": abs(phases[-1] - phases[0]) / (4.0 * math.pi),
        **moments(values),
        **distances,
        "closest_control_m": closest,
        "parent_action_drift": float(arm["max_action_drift"]),
        "parent_min_support": float(arm["min_support"]),
        "parent_phase_error": float(arm["phase_error"]),
        "parent_phase_rms": float(arm["phase_step_rms"]),
        "parent_leakage": float(arm["leakage"]),
    }


def public_arm(record: dict[str, Any]) -> dict[str, Any]:
    return {key: value for key, value in record.items() if key != "values"}


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in HASHES.items():
        check(f"frozen hash {path.name}", path.is_file() and sha256(path) == expected)

    parent = json.loads(PARENT_JSON.read_text(encoding="utf-8"))
    result = json.loads(RESULT_JSON.read_text(encoding="utf-8"))
    arm_fields, arm_rows = read_csv(PARENT_ARMS)
    tick_fields, tick_rows = read_csv(PARENT_TICKS)
    result_fields, result_rows = read_csv(RESULT_CSV)
    check("parent arm schema", arm_fields == ARM_FIELDS)
    check("parent tick schema", tick_fields == TICK_FIELDS)
    check("result CSV schema", result_fields == RESULT_FIELDS)

    nonzero_labels = [
        f"o{o}_p{p}_a{a}_q{q}"
        for o in range(2) for p in range(3) for a in range(3) for q in range(4)
    ]
    zero_labels = [f"o{o}_zero" for o in range(2)]
    expected_labels = set(nonzero_labels + zero_labels)
    arms = {row["label"]: row for row in arm_rows}
    by_label: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in tick_rows:
        by_label[row["label"]].append(row)
    for rows in by_label.values():
        rows.sort(key=lambda row: int(row["tick"]))

    schema_pass = (
        parent.get("ftd_id") == "FTD-0659"
        and parent.get("protocol_sha256") == PARENT_PROTOCOL_SHA256
        and parent.get("arm_count") == 74
        and len(arm_rows) == len(arms) == 74
        and set(arms) == expected_labels
        and set(by_label) == expected_labels
        and all(row["ftd_id"] == "FTD-0659" for row in arm_rows + tick_rows)
    )
    coverage_pass = schema_pass and len(tick_rows) == 74 * 257 and all(
        len(by_label[label]) == 257
        and [int(row["tick"]) for row in by_label[label]] == list(range(257))
        for label in expected_labels
    )
    zero_pass = coverage_pass and all(
        truth(arms[label]["zero"])
        and float(arms[label]["initial_action"]) <= 1e-20
        and all(abs(float(row[field])) <= 1e-20
                for row in by_label[label]
                for field in ("q0", "q1", "p0", "p1", "action", "z_abs"))
        for label in zero_labels
    )
    finite_pass = coverage_pass and all(
        all(math.isfinite(float(row[field]))
            for field in ("q0", "q1", "p0", "p1", "action", "z_abs", "support"))
        for label in nonzero_labels for row in by_label[label]
    )
    parent_global_pass = (
        all(bool(parent.get(key)) for key in (
            "eigenspace_pass", "coverage_pass", "execution_pass", "bounded_pass",
            "amplitude_pass", "quadrature_pass", "polarization_pass",
            "covariance_pass", "zero_control_pass"))
        and float(parent.get("worst_common_residual", math.inf)) <= 1e-10
        and float(parent.get("worst_energy_drift", math.inf)) <= 1e-12
        and float(parent.get("worst_recovery", math.inf)) <= 1e-10
        and float(parent.get("minimum_support", -math.inf)) >= 0.90
        and float(parent.get("worst_phase_error", math.inf)) <= 0.02
        and float(parent.get("worst_phase_rms", math.inf)) <= 0.05
    )
    execution_valid = all((schema_pass, coverage_pass, zero_pass,
                           finite_pass, parent_global_pass))
    check("parent schema and 74-arm identity", schema_pass)
    check("complete unique tick coverage 0..256", coverage_pass)
    check("two exact rest controls excluded", zero_pass)
    check("all 72 occupancy histories finite", finite_pass)
    check("parent global execution gates", parent_global_pass)
    check("execution valid reconstructed", execution_valid)

    analyses = {
        label: analyze_arm(label, arms[label], by_label[label])
        for label in nonzero_labels
    }
    check("signed observer retained", all(
        min(record["values"]) < 0.0 < max(record["values"])
        for record in analyses.values()))
    check("all arm reports independently reproduced", close_tree(
        result.get("arms"),
        {label: public_arm(analyses[label]) for label in sorted(analyses)},
    ))

    cells: list[dict[str, Any]] = []
    for orientation in range(2):
        for polarization in range(3):
            for amplitude in range(3):
                labels = [
                    f"o{orientation}_p{polarization}_a{amplitude}_q{quadrature}"
                    for quadrature in range(4)
                ]
                records = [analyses[label] for label in labels]
                primary = records[0]
                windows = [primary["values"][start:stop + 1]
                           for start, stop in WINDOWS]
                window_stats = [moments(values) for values in windows]
                window_ks = pairwise_max(windows)
                window_spread = max(range_of(window_stats, key) for key in MOMENTS)
                window_peak = min(max(abs(value) for value in values)
                                  for values in windows)
                quadrature_ks = max(two_sample_distance(primary["values"], record["values"])
                                    for record in records[1:])
                quadrature_delta = max(
                    abs(primary[key] - record[key])
                    for record in records[1:] for key in MOMENTS
                )
                applicability = {
                    "cycle_pass": primary["cycles"] >= 8.0,
                    "fixed_ray_pass": max(record["epsilon_perp"] for record in records) <= 0.05,
                    "normalized_bound_pass": max(record["max_abs_x"] for record in records) <= 1.05,
                    "window_turn_pass": window_peak >= 0.85,
                    "window_cdf_pass": window_ks <= 0.10,
                    "window_moment_pass": window_spread <= 0.05,
                    "quadrature_cdf_pass": quadrature_ks <= 0.10,
                    "quadrature_moment_pass": quadrature_delta <= 0.05,
                    "parent_arm_pass": all(arm_gate(arms[label]) for label in labels),
                }
                quartic = {
                    "cdf_absolute_pass": primary["d4"] <= 0.04,
                    "cdf_control_pass": primary["d4"] < min(primary["d2"], primary["d6"]),
                    "mu1_pass": abs(primary["mu1"] - TARGETS["mu1"]) <= 0.02,
                    "mu2_pass": abs(primary["mu2"] - TARGETS["mu2"]) <= 0.02,
                    "mu4_pass": abs(primary["mu4"] - TARGETS["mu4"]) <= 0.02,
                    "symmetry_pass": abs(primary["mean"]) <= 0.02,
                    "g_rms_pass": relative_error(primary["g_rms"], G_STAR) <= 0.02,
                    "g_abs_pass": relative_error(primary["g_abs"], G_STAR) <= 0.02,
                    "g_consistency_pass": relative_difference(
                        primary["g_rms"], primary["g_abs"]) <= 0.02,
                }
                cells.append({
                    "orientation": orientation,
                    "polarization": polarization,
                    "amplitude_index": amplitude,
                    "primary": public_arm(primary),
                    "window_max_pairwise_ks": window_ks,
                    "window_max_moment_spread": window_spread,
                    "window_min_peak": window_peak,
                    "quadrature_max_ks": quadrature_ks,
                    "quadrature_max_moment_delta": quadrature_delta,
                    "applicability_gates": applicability,
                    "applicability_pass": all(applicability.values()),
                    "quartic_gates": quartic,
                    "quartic_pass": all(quartic.values()),
                })

    check("18 registered primary cells", len(cells) == 18)
    for computed, reported in zip(cells, result.get("cells", [])):
        key = (computed["orientation"], computed["polarization"],
               computed["amplitude_index"])
        check(f"cell {key} moments/CDF/windows/gates", close_tree(reported, computed))

    amplitude_controls: list[dict[str, Any]] = []
    for orientation in range(2):
        for polarization in range(3):
            records = [analyses[f"o{orientation}_p{polarization}_a{a}_q0"]
                       for a in range(3)]
            ks = pairwise_max([record["values"] for record in records])
            spread = max(range_of(records, key) for key in MOMENTS)
            amplitude_controls.append({
                "orientation": orientation, "polarization": polarization,
                "max_pairwise_ks": ks, "max_moment_spread": spread,
                "pass": ks <= 0.075 and spread <= 0.04,
            })
    covariance_controls: list[dict[str, Any]] = []
    for amplitude in range(3):
        records = [analyses[f"o{o}_p{p}_a{amplitude}_q0"]
                   for o in range(2) for p in range(3)]
        ks = pairwise_max([record["values"] for record in records])
        spread = max(range_of(records, key) for key in MOMENTS)
        covariance_controls.append({
            "amplitude_index": amplitude, "max_pairwise_ks": ks,
            "max_moment_spread": spread,
            "pass": ks <= 0.075 and spread <= 0.04,
        })
    check("amplitude controls reproduced",
          close_tree(result.get("amplitude_controls"), amplitude_controls))
    check("covariance controls reproduced",
          close_tree(result.get("covariance_controls"), covariance_controls))

    cell_pass = all(cell["applicability_pass"] for cell in cells)
    amplitude_pass = all(control["pass"] for control in amplitude_controls)
    covariance_pass = all(control["pass"] for control in covariance_controls)
    recurrence_qualified = execution_valid and cell_pass and amplitude_pass and covariance_pass
    quartic_shape_pass = execution_valid and all(cell["quartic_pass"] for cell in cells)
    if not execution_valid:
        verdict = "NATIVE_TEMPORAL_OCCUPANCY_EXECUTION_INVALID"
    elif not recurrence_qualified:
        verdict = "NATIVE_TEMPORAL_OCCUPANCY_RECURRENCE_UNQUALIFIED"
    elif not quartic_shape_pass:
        verdict = "NATIVE_QUARTIC_OCCUPANCY_CLOSED_NEGATIVE_FOR_FTD0659"
    else:
        verdict = "NATIVE_QUARTIC_OCCUPANCY_RETROSPECTIVE_CANDIDATE"

    check("reported execution map", result.get("execution") == {
        "provenance_pass": True, "schema_pass": schema_pass,
        "coverage_pass": coverage_pass, "zero_control_pass": zero_pass,
        "finite_pass": finite_pass, "parent_global_pass": parent_global_pass,
        "execution_valid": execution_valid,
    })
    check("amplitude invariance passes", amplitude_pass)
    check("covariance passes", covariance_pass)
    check("cell applicability fails", not cell_pass)
    check("recurrence is unqualified", not recurrence_qualified)
    check("quartic shape also fails descriptively", not quartic_shape_pass)
    check("locked control winner is sextic in every primary", Counter(
        cell["primary"]["closest_control_m"] for cell in cells) == {6: 18})
    check("verdict independently reconstructed", result.get("verdict") == verdict
          == "NATIVE_TEMPORAL_OCCUPANCY_RECURRENCE_UNQUALIFIED")
    check("scope firewall preserved",
          result.get("natural_coordinate_closure") == "UNTESTED_IN_PARENT_CORPUS"
          and result.get("continuous_measure_claim") is False
          and result.get("production_changed") is False
          and result.get("engine_rerun") is False
          and result.get("fresh_confirmation_required") is False)

    csv_by_key = {
        (int(row["orientation"]), int(row["polarization"]),
         int(row["amplitude_index"])): row for row in result_rows
    }
    check("result CSV has 18 unique cells", len(result_rows) == len(csv_by_key) == 18)
    csv_match = True
    for cell in cells:
        key = (cell["orientation"], cell["polarization"], cell["amplitude_index"])
        row = csv_by_key.get(key, {})
        expected: dict[str, Any] = {
            **cell["primary"],
            "orientation": key[0], "polarization": key[1], "amplitude_index": key[2],
            "window_max_pairwise_ks": cell["window_max_pairwise_ks"],
            "window_max_moment_spread": cell["window_max_moment_spread"],
            "window_min_peak": cell["window_min_peak"],
            "quadrature_max_ks": cell["quadrature_max_ks"],
            "quadrature_max_moment_delta": cell["quadrature_max_moment_delta"],
            "applicability_pass": cell["applicability_pass"],
            "quartic_pass": cell["quartic_pass"],
        }
        for field in RESULT_FIELDS:
            if field in ("applicability_pass", "quartic_pass"):
                csv_match &= truth(row.get(field, "")) is expected[field]
            elif field in ("orientation", "polarization", "amplitude_index",
                           "sample_count", "closest_control_m"):
                csv_match &= int(row.get(field, -1)) == int(expected[field])
            else:
                csv_match &= math.isclose(float(row.get(field, "nan")),
                                          float(expected[field]),
                                          rel_tol=2e-11, abs_tol=2e-14)
    check("result CSV values independently reproduced", csv_match)

    failures = [label for label, passed in checks if not passed]
    print(f"FTD-0772 native temporal occupancy certificate: "
          f"{len(checks) - len(failures)}/{len(checks)} checks PASS")
    print(f"execution_valid={execution_valid}")
    print(f"recurrence_qualified={recurrence_qualified}")
    print(f"quartic_shape_pass={quartic_shape_pass}")
    print(f"closest_control_counts={dict(Counter(cell['primary']['closest_control_m'] for cell in cells))}")
    print(f"verdict={verdict}")
    for label in failures:
        print(f"FAIL {label}")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
