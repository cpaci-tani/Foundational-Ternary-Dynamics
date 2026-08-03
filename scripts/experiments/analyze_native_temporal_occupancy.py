"""FTD-0772 locked retrospective native temporal-occupancy analysis.

This script reanalyzes the immutable FTD-0659 tick corpus.  It does not run
the engine, fit a potential power, remap the registered coordinate, or infer
a continuous invariant measure from finite tick samples.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import Counter
from pathlib import Path
from typing import Iterable, Sequence

from scipy.special import betainc


ROOT = Path(__file__).resolve().parents[2]
PARENT_DIR = ROOT / "engine" / "results" / "ftd_0659"
OUTPUT_DIR = ROOT / "engine" / "results" / "ftd_0772"

PARENT_JSON = PARENT_DIR / "ftd_0659_native_excited_matter_clock_v1.json"
PARENT_ARMS = PARENT_DIR / "ftd_0659_native_excited_matter_clock_arms_v1.csv"
PARENT_TICKS = PARENT_DIR / "ftd_0659_native_excited_matter_clock_ticks_v1.csv"
OUTPUT_JSON = OUTPUT_DIR / "ftd_0772_native_temporal_occupancy_v1.json"
OUTPUT_CSV = OUTPUT_DIR / "ftd_0772_native_temporal_occupancy_cells_v1.csv"

PROTOCOL_SHA256 = "3E779CDFFDE5D17299921750A06E26B075000572CAB60E9DA8FBF154239CC41C"
PARENT_PROTOCOL_SHA256 = "FF9566F6D6B7BCAEB7970359043C62F643A6A8315AF43C01EE0C5CFD21ECC342"
EXPECTED_HASHES = {
    PARENT_JSON.name: "DB6CA66770812E4C8FC94411B109F23E424FFF1CE3173A5D16AB43B5949ACEEE",
    PARENT_ARMS.name: "4F7D2E38B0FE4D6EF33F137E2AA753E4143B3AD541F6934CF39FD11772844941",
    PARENT_TICKS.name: "4EF51456F161E6CD836518B72EBAACE4A5007F5EF5525E07CD097B343566634A",
}

POLARIZATIONS = {
    0: (1.0, 0.0),
    1: (0.0, 1.0),
    2: (1.0 / math.sqrt(2.0), 1.0 / math.sqrt(2.0)),
}
WINDOWS = ((0, 85), (86, 171), (172, 256))
MOMENT_KEYS = ("mu1", "mu2", "mu4")

G_STAR = math.gamma(0.25) / math.gamma(0.75)
TARGETS = {
    "mu1": math.sqrt(math.pi) / G_STAR,
    "mu2": 4.0 / (G_STAR * G_STAR),
    "mu4": 1.0 / 3.0,
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def as_bool(value: str) -> bool:
    return value.strip() in {"1", "true", "True"}


def as_float(value: str) -> float:
    return float(value)


def empirical_moments(values: Sequence[float]) -> dict[str, float]:
    count = len(values)
    if count == 0:
        raise ValueError("empty empirical sample")
    mu1 = math.fsum(abs(value) for value in values) / count
    mu2 = math.fsum(value * value for value in values) / count
    mu4 = math.fsum(value**4 for value in values) / count
    return {
        "mean": math.fsum(values) / count,
        "mu1": mu1,
        "mu2": mu2,
        "mu4": mu4,
        "g_rms": 2.0 / math.sqrt(mu2) if mu2 > 0.0 else math.inf,
        "g_abs": math.sqrt(math.pi) / mu1 if mu1 > 0.0 else math.inf,
        "max_abs_x": max(abs(value) for value in values),
    }


def target_cdf(power: int, value: float) -> float:
    if value <= -1.0:
        return 0.0
    if value >= 1.0:
        return 1.0
    if value == 0.0:
        return 0.5
    regularized = float(betainc(1.0 / power, 0.5, abs(value) ** power))
    return 0.5 + math.copysign(0.5 * regularized, value)


def one_sample_ks(values: Sequence[float], power: int) -> float:
    ordered = sorted(values)
    count = len(ordered)
    distance = 0.0
    for index, value in enumerate(ordered, start=1):
        cdf = target_cdf(power, value)
        distance = max(
            distance,
            index / count - cdf,
            cdf - (index - 1) / count,
        )
    return distance


def two_sample_ks(left: Sequence[float], right: Sequence[float]) -> float:
    a = sorted(left)
    b = sorted(right)
    ia = ib = 0
    distance = 0.0
    for value in sorted(set(a + b)):
        while ia < len(a) and a[ia] <= value:
            ia += 1
        while ib < len(b) and b[ib] <= value:
            ib += 1
        distance = max(distance, abs(ia / len(a) - ib / len(b)))
    return distance


def pairwise_max(samples: Sequence[Sequence[float]]) -> float:
    maximum = 0.0
    for left in range(len(samples)):
        for right in range(left + 1, len(samples)):
            maximum = max(maximum, two_sample_ks(samples[left], samples[right]))
    return maximum


def spread(records: Sequence[dict[str, float]], key: str) -> float:
    values = [record[key] for record in records]
    return max(values) - min(values)


def relative_error(value: float, target: float) -> float:
    return abs(value - target) / abs(target)


def relative_difference(left: float, right: float) -> float:
    return abs(left - right) / max(abs(left), abs(right), 1e-300)


def load_rows(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as handle:
        return list(csv.DictReader(handle))


def arm_values(
    label: str,
    arm: dict[str, str],
    ticks: Sequence[dict[str, str]],
) -> tuple[list[float], float, float]:
    polarization = int(arm["polarization"])
    u0, u1 = POLARIZATIONS[polarization]
    amplitude = as_float(arm["modal_amplitude"])
    if not math.isfinite(amplitude) or amplitude <= 0.0:
        raise ValueError(f"{label}: invalid locked modal amplitude")

    values: list[float] = []
    transverse = 0.0
    radial = 0.0
    for row in ticks:
        q0 = as_float(row["q0"])
        q1 = as_float(row["q1"])
        q_parallel = u0 * q0 + u1 * q1
        values.append(q_parallel / amplitude)
        radial_term = q0 * q0 + q1 * q1
        radial += radial_term
        transverse += max(0.0, radial_term - q_parallel * q_parallel)
    epsilon_perp = transverse / radial if radial > 0.0 else math.inf
    return values, epsilon_perp, amplitude


def analyze_arm(
    label: str,
    arm: dict[str, str],
    ticks: Sequence[dict[str, str]],
) -> dict[str, object]:
    values, epsilon_perp, amplitude = arm_values(label, arm, ticks)
    moments = empirical_moments(values)
    finite_phases = [
        as_float(row["unwrapped_phase"])
        for row in ticks
        if math.isfinite(as_float(row["unwrapped_phase"]))
    ]
    cycles = (
        abs(finite_phases[-1] - finite_phases[0]) / (4.0 * math.pi)
        if len(finite_phases) >= 2
        else 0.0
    )
    distances = {f"d{power}": one_sample_ks(values, power) for power in (2, 4, 6)}
    closest = min((2, 4, 6), key=lambda power: (distances[f"d{power}"], power))
    return {
        "label": label,
        "sample_count": len(values),
        "values": values,
        "amplitude": amplitude,
        "epsilon_perp": epsilon_perp,
        "cycles": cycles,
        **moments,
        **distances,
        "closest_control_m": closest,
        "parent_action_drift": as_float(arm["max_action_drift"]),
        "parent_min_support": as_float(arm["min_support"]),
        "parent_phase_error": as_float(arm["phase_error"]),
        "parent_phase_rms": as_float(arm["phase_step_rms"]),
        "parent_leakage": as_float(arm["leakage"]),
    }


def serialize_arm(record: dict[str, object]) -> dict[str, object]:
    return {key: value for key, value in record.items() if key != "values"}


def parent_arm_gate(arm: dict[str, str]) -> bool:
    return (
        as_bool(arm["initialization"])
        and as_bool(arm["forward"])
        and as_bool(arm["reverse"])
        and as_bool(arm["bounded"])
        and as_bool(arm["phase_defined"])
        and as_float(arm["min_support"]) >= 0.90
        and as_float(arm["phase_error"]) <= 0.02
        and as_float(arm["phase_step_rms"]) <= 0.05
        and as_float(arm["leakage"]) <= 0.10
        and as_float(arm["max_energy_drift"]) <= 1e-12
        and as_float(arm["max_common"]) <= 1e-10
        and as_float(arm["recovery"]) <= 1e-10
    )


def main() -> None:
    observed_hashes = {
        path.name: sha256(path) for path in (PARENT_JSON, PARENT_ARMS, PARENT_TICKS)
    }
    provenance_pass = observed_hashes == EXPECTED_HASHES

    with PARENT_JSON.open("r", encoding="utf-8") as handle:
        parent = json.load(handle)
    arm_rows = load_rows(PARENT_ARMS)
    tick_rows = load_rows(PARENT_TICKS)
    arms = {row["label"]: row for row in arm_rows}
    ticks_by_label: dict[str, list[dict[str, str]]] = {}
    for row in tick_rows:
        ticks_by_label.setdefault(row["label"], []).append(row)
    for rows in ticks_by_label.values():
        rows.sort(key=lambda row: int(row["tick"]))

    expected_nonzero = [
        f"o{o}_p{p}_a{a}_q{q}"
        for o in range(2)
        for p in range(3)
        for a in range(3)
        for q in range(4)
    ]
    expected_zero = [f"o{o}_zero" for o in range(2)]
    expected_labels = set(expected_nonzero + expected_zero)
    schema_pass = (
        parent.get("ftd_id") == "FTD-0659"
        and parent.get("protocol_sha256") == PARENT_PROTOCOL_SHA256
        and parent.get("arm_count") == 74
        and set(arms) == expected_labels
        and set(ticks_by_label) == expected_labels
    )
    coverage_pass = schema_pass and all(
        len(ticks_by_label[label]) == 257
        and [int(row["tick"]) for row in ticks_by_label[label]] == list(range(257))
        for label in expected_labels
    )
    zero_pass = coverage_pass and all(
        as_bool(arms[label]["zero"])
        and as_float(arms[label]["initial_action"]) <= 1e-20
        and all(
            abs(as_float(row[field])) <= 1e-20
            for row in ticks_by_label[label]
            for field in ("q0", "q1", "p0", "p1", "action", "z_abs")
        )
        for label in expected_zero
    )
    finite_pass = coverage_pass and all(
        all(
            math.isfinite(as_float(row[field]))
            for field in ("q0", "q1", "p0", "p1", "action", "z_abs", "support")
        )
        for label in expected_nonzero
        for row in ticks_by_label[label]
    )
    parent_global_pass = (
        bool(parent.get("eigenspace_pass"))
        and bool(parent.get("coverage_pass"))
        and bool(parent.get("execution_pass"))
        and bool(parent.get("bounded_pass"))
        and bool(parent.get("amplitude_pass"))
        and bool(parent.get("quadrature_pass"))
        and bool(parent.get("polarization_pass"))
        and bool(parent.get("covariance_pass"))
        and bool(parent.get("zero_control_pass"))
        and float(parent.get("worst_common_residual", math.inf)) <= 1e-10
        and float(parent.get("worst_energy_drift", math.inf)) <= 1e-12
        and float(parent.get("worst_recovery", math.inf)) <= 1e-10
        and float(parent.get("minimum_support", -math.inf)) >= 0.90
        and float(parent.get("worst_phase_error", math.inf)) <= 0.02
        and float(parent.get("worst_phase_rms", math.inf)) <= 0.05
    )
    execution_valid = (
        provenance_pass
        and schema_pass
        and coverage_pass
        and zero_pass
        and finite_pass
        and parent_global_pass
    )

    analyses: dict[str, dict[str, object]] = {}
    if execution_valid:
        for label in expected_nonzero:
            analyses[label] = analyze_arm(label, arms[label], ticks_by_label[label])

    cells: list[dict[str, object]] = []
    if execution_valid:
        for orientation in range(2):
            for polarization in range(3):
                for amplitude in range(3):
                    labels = [
                        f"o{orientation}_p{polarization}_a{amplitude}_q{quadrature}"
                        for quadrature in range(4)
                    ]
                    records = [analyses[label] for label in labels]
                    primary = records[0]
                    primary_values = primary["values"]
                    assert isinstance(primary_values, list)
                    window_values = [
                        primary_values[start : stop + 1] for start, stop in WINDOWS
                    ]
                    window_moments = [empirical_moments(values) for values in window_values]
                    window_ks = pairwise_max(window_values)
                    window_moment_spread = max(
                        spread(window_moments, key) for key in MOMENT_KEYS
                    )
                    window_min_peak = min(
                        max(abs(value) for value in values) for values in window_values
                    )
                    quadrature_ks = max(
                        two_sample_ks(primary_values, record["values"])
                        for record in records[1:]
                    )
                    quadrature_moment_delta = max(
                        abs(float(primary[key]) - float(record[key]))
                        for record in records[1:]
                        for key in MOMENT_KEYS
                    )
                    all_parent_arms_pass = all(parent_arm_gate(arms[label]) for label in labels)
                    applicability = {
                        "cycle_pass": float(primary["cycles"]) >= 8.0,
                        "fixed_ray_pass": max(float(record["epsilon_perp"]) for record in records)
                        <= 0.05,
                        "normalized_bound_pass": max(
                            float(record["max_abs_x"]) for record in records
                        )
                        <= 1.05,
                        "window_turn_pass": window_min_peak >= 0.85,
                        "window_cdf_pass": window_ks <= 0.10,
                        "window_moment_pass": window_moment_spread <= 0.05,
                        "quadrature_cdf_pass": quadrature_ks <= 0.10,
                        "quadrature_moment_pass": quadrature_moment_delta <= 0.05,
                        "parent_arm_pass": all_parent_arms_pass,
                    }
                    quartic = {
                        "cdf_absolute_pass": float(primary["d4"]) <= 0.04,
                        "cdf_control_pass": float(primary["d4"])
                        < min(float(primary["d2"]), float(primary["d6"])),
                        "mu1_pass": abs(float(primary["mu1"]) - TARGETS["mu1"]) <= 0.02,
                        "mu2_pass": abs(float(primary["mu2"]) - TARGETS["mu2"]) <= 0.02,
                        "mu4_pass": abs(float(primary["mu4"]) - TARGETS["mu4"]) <= 0.02,
                        "symmetry_pass": abs(float(primary["mean"])) <= 0.02,
                        "g_rms_pass": relative_error(float(primary["g_rms"]), G_STAR)
                        <= 0.02,
                        "g_abs_pass": relative_error(float(primary["g_abs"]), G_STAR)
                        <= 0.02,
                        "g_consistency_pass": relative_difference(
                            float(primary["g_rms"]), float(primary["g_abs"])
                        )
                        <= 0.02,
                    }
                    cells.append(
                        {
                            "orientation": orientation,
                            "polarization": polarization,
                            "amplitude_index": amplitude,
                            "primary": serialize_arm(primary),
                            "window_max_pairwise_ks": window_ks,
                            "window_max_moment_spread": window_moment_spread,
                            "window_min_peak": window_min_peak,
                            "quadrature_max_ks": quadrature_ks,
                            "quadrature_max_moment_delta": quadrature_moment_delta,
                            "applicability_gates": applicability,
                            "applicability_pass": all(applicability.values()),
                            "quartic_gates": quartic,
                            "quartic_pass": all(quartic.values()),
                        }
                    )

    amplitude_controls: list[dict[str, object]] = []
    covariance_controls: list[dict[str, object]] = []
    if execution_valid:
        for orientation in range(2):
            for polarization in range(3):
                records = [
                    analyses[f"o{orientation}_p{polarization}_a{amplitude}_q0"]
                    for amplitude in range(3)
                ]
                ks = pairwise_max([record["values"] for record in records])
                moment_spread = max(spread(records, key) for key in MOMENT_KEYS)
                amplitude_controls.append(
                    {
                        "orientation": orientation,
                        "polarization": polarization,
                        "max_pairwise_ks": ks,
                        "max_moment_spread": moment_spread,
                        "pass": ks <= 0.075 and moment_spread <= 0.04,
                    }
                )
        for amplitude in range(3):
            records = [
                analyses[f"o{orientation}_p{polarization}_a{amplitude}_q0"]
                for orientation in range(2)
                for polarization in range(3)
            ]
            ks = pairwise_max([record["values"] for record in records])
            moment_spread = max(spread(records, key) for key in MOMENT_KEYS)
            covariance_controls.append(
                {
                    "amplitude_index": amplitude,
                    "max_pairwise_ks": ks,
                    "max_moment_spread": moment_spread,
                    "pass": ks <= 0.075 and moment_spread <= 0.04,
                }
            )

    cell_applicability_pass = execution_valid and all(
        bool(cell["applicability_pass"]) for cell in cells
    )
    amplitude_invariance_pass = execution_valid and all(
        bool(control["pass"]) for control in amplitude_controls
    )
    covariance_pass = execution_valid and all(
        bool(control["pass"]) for control in covariance_controls
    )
    recurrence_qualified = (
        cell_applicability_pass and amplitude_invariance_pass and covariance_pass
    )
    quartic_shape_pass = execution_valid and all(bool(cell["quartic_pass"]) for cell in cells)

    if not execution_valid:
        verdict = "NATIVE_TEMPORAL_OCCUPANCY_EXECUTION_INVALID"
    elif not recurrence_qualified:
        verdict = "NATIVE_TEMPORAL_OCCUPANCY_RECURRENCE_UNQUALIFIED"
    elif not quartic_shape_pass:
        verdict = "NATIVE_QUARTIC_OCCUPANCY_CLOSED_NEGATIVE_FOR_FTD0659"
    else:
        verdict = "NATIVE_QUARTIC_OCCUPANCY_RETROSPECTIVE_CANDIDATE"

    closest_counts = Counter(
        int(cell["primary"]["closest_control_m"]) for cell in cells
    )
    pooled_primary = [
        value
        for orientation in range(2)
        for polarization in range(3)
        for amplitude in range(3)
        for value in (
            analyses.get(f"o{orientation}_p{polarization}_a{amplitude}_q0", {}).get(
                "values", []
            )
        )
    ]
    pooled = None
    if pooled_primary:
        pooled = {
            **empirical_moments(pooled_primary),
            **{f"d{power}": one_sample_ks(pooled_primary, power) for power in (2, 4, 6)},
            "sample_count": len(pooled_primary),
        }

    result = {
        "ftd_id": "FTD-0772",
        "protocol_sha256": PROTOCOL_SHA256,
        "campaign_type": "LOCKED_RETROSPECTIVE_REANALYSIS",
        "parent_ftd_id": "FTD-0659",
        "parent_protocol_sha256": PARENT_PROTOCOL_SHA256,
        "parent_artifact_sha256": observed_hashes,
        "production_changed": False,
        "engine_rerun": False,
        "observer": "Q_u=u^T(q_6,q_7), x=Q_u/parent_modal_amplitude",
        "sample_measure": "finite_atomic_tick_measure",
        "natural_coordinate_closure": "UNTESTED_IN_PARENT_CORPUS",
        "continuous_measure_claim": False,
        "g_star_target": G_STAR,
        "quartic_targets": TARGETS,
        "execution": {
            "provenance_pass": provenance_pass,
            "schema_pass": schema_pass,
            "coverage_pass": coverage_pass,
            "zero_control_pass": zero_pass,
            "finite_pass": finite_pass,
            "parent_global_pass": parent_global_pass,
            "execution_valid": execution_valid,
        },
        "recurrence": {
            "cell_applicability_pass": cell_applicability_pass,
            "amplitude_invariance_pass": amplitude_invariance_pass,
            "covariance_pass": covariance_pass,
            "recurrence_qualified": recurrence_qualified,
            "parent_harmonic_action_drift_diagnostic": parent.get("worst_action_drift"),
        },
        "quartic_shape_pass": quartic_shape_pass,
        "closest_control_counts": {str(key): value for key, value in sorted(closest_counts.items())},
        "pooled_primary_descriptive": pooled,
        "arms": {
            label: serialize_arm(analyses[label])
            for label in sorted(analyses)
        },
        "amplitude_controls": amplitude_controls,
        "covariance_controls": covariance_controls,
        "cells": cells,
        "verdict": verdict,
        "fresh_confirmation_required": verdict
        == "NATIVE_QUARTIC_OCCUPANCY_RETROSPECTIVE_CANDIDATE",
        "scope_note": (
            "No native potential or continuous invariant measure is inferred; "
            "the fixed natural-coordinate closure is unavailable in FTD-0659."
        ),
    }

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    with OUTPUT_JSON.open("w", encoding="utf-8", newline="\n") as handle:
        json.dump(result, handle, indent=2, sort_keys=True, allow_nan=False)
        handle.write("\n")

    fieldnames = [
        "orientation",
        "polarization",
        "amplitude_index",
        "sample_count",
        "amplitude",
        "cycles",
        "epsilon_perp",
        "max_abs_x",
        "mean",
        "mu1",
        "mu2",
        "mu4",
        "g_rms",
        "g_abs",
        "d2",
        "d4",
        "d6",
        "closest_control_m",
        "window_max_pairwise_ks",
        "window_max_moment_spread",
        "window_min_peak",
        "quadrature_max_ks",
        "quadrature_max_moment_delta",
        "applicability_pass",
        "quartic_pass",
    ]
    with OUTPUT_CSV.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for cell in cells:
            primary = cell["primary"]
            writer.writerow(
                {
                    "orientation": cell["orientation"],
                    "polarization": cell["polarization"],
                    "amplitude_index": cell["amplitude_index"],
                    **{key: primary[key] for key in fieldnames if key in primary},
                    "window_max_pairwise_ks": cell["window_max_pairwise_ks"],
                    "window_max_moment_spread": cell["window_max_moment_spread"],
                    "window_min_peak": cell["window_min_peak"],
                    "quadrature_max_ks": cell["quadrature_max_ks"],
                    "quadrature_max_moment_delta": cell["quadrature_max_moment_delta"],
                    "applicability_pass": cell["applicability_pass"],
                    "quartic_pass": cell["quartic_pass"],
                }
            )

    print(f"FTD-0772 protocol_sha256={PROTOCOL_SHA256}")
    print(f"execution_valid={execution_valid}")
    print(f"recurrence_qualified={recurrence_qualified}")
    print(f"quartic_shape_pass={quartic_shape_pass}")
    print(f"closest_control_counts={dict(sorted(closest_counts.items()))}")
    if pooled is not None:
        print(
            "pooled_primary "
            f"mu1={pooled['mu1']:.12g} mu2={pooled['mu2']:.12g} "
            f"mu4={pooled['mu4']:.12g} d2={pooled['d2']:.12g} "
            f"d4={pooled['d4']:.12g} d6={pooled['d6']:.12g}"
        )
    print(f"verdict={verdict}")
    print(f"result_json={OUTPUT_JSON}")
    print(f"result_csv={OUTPUT_CSV}")


if __name__ == "__main__":
    main()
