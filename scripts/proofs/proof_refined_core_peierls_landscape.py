#!/usr/bin/env python3
"""Independent record certificate for FTD-0614."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_REFINED_CORE_PEIERLS_LANDSCAPE_v1.md"
RESULT = ROOT / "engine/results/ftd_0614/ftd_0614_refined_core_peierls_landscape_v1.json"
SAMPLES = ROOT / "engine/results/ftd_0614/ftd_0614_refined_core_peierls_samples_v1.csv"
BOOST = ROOT / "engine/results/ftd_0613/ftd_0613_refined_directional_boost_v1.json"
EXPECTED = "D409501414737F70D884A553CA05E86200EA42876854FCFD834BE04581493D82"
E_REST = 0.511
C_SPEED = 1.0 / math.sqrt(3.0)


def protocol_hash() -> str:
    raw = PROTOCOL.read_bytes()
    return hashlib.sha256(raw[: raw.index(b"`protocol_sha256=")]).hexdigest().upper()


def kinetic_budget(speed: float) -> float:
    mass = E_REST / (C_SPEED * C_SPEED)
    gamma = 1.0 / math.sqrt(1.0 - speed * speed / (C_SPEED * C_SPEED))
    momentum = gamma * mass * speed
    energy = math.sqrt(E_REST * E_REST + C_SPEED * C_SPEED * momentum * momentum)
    return 3.0 * (energy - E_REST)


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    boost = json.loads(BOOST.read_text(encoding="utf-8"))
    with SAMPLES.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    paths = record["paths"]
    arms = record["covariance_arms"]
    grouped: dict[tuple[int, int, int, int], list[dict[str, str]]] = {}
    for row in rows:
        key = tuple(int(row[name]) for name in ("family", "axis", "sign", "rotation"))
        grouped.setdefault(key, []).append(row)

    reconstructed_barrier = 0.0
    reconstructed_hysteresis = 0.0
    for path in paths:
        key = (path["family"], path["axis"], path["sign"], path["rotation"])
        samples = grouped[key]
        relaxed = [float(row["relaxed_forward_energy"]) for row in samples]
        backward = [float(row["relaxed_backward_energy"]) for row in samples]
        barrier = max(relaxed) - relaxed[0]
        hysteresis = max(abs(a - b) for a, b in zip(relaxed, backward))
        reconstructed_barrier = max(reconstructed_barrier, abs(barrier - path["relaxed_barrier"]))
        reconstructed_hysteresis = max(reconstructed_hysteresis, abs(hysteresis - path["hysteresis"]))

    main_paths = paths[:6]
    no_mobile_below_path = True
    pinned_above_path = 0
    maximum_pinned_budget_ratio = 0.0
    for arm in boost["arms"]:
        axis = arm["direction_index"] // 2
        sign = 1 if arm["direction_index"] % 2 == 0 else -1
        path = next(p for p in main_paths if p["axis"] == axis and p["sign"] == sign)
        budget = kinetic_budget(arm["speed"])
        if arm["mobility_pass"] and budget + 1e-14 < path["relaxed_barrier"]:
            no_mobile_below_path = False
        if not arm["mobility_pass"] and budget >= path["relaxed_barrier"]:
            pinned_above_path += 1
            maximum_pinned_budget_ratio = max(
                maximum_pinned_budget_ratio, budget / path["relaxed_barrier"]
            )

    transverse = [p for p in main_paths if p["axis"] in (0, 1)]
    axial = [p for p in main_paths if p["axis"] == 2]
    checks = {
        "protocol_hash": protocol_hash() == EXPECTED,
        "record_protocol": record["protocol_sha256"] == EXPECTED,
        "identity": record["ftd_id"] == "FTD-0614",
        "production_unchanged": record["production_changed"] is False,
        "rest": record["rest_fingerprint_pass"] and record["rest_gate_pass"],
        "cardinality": len(paths) == 10 and len(rows) == 10 * 65 and len(arms) == 12,
        "sample_groups": len(grouped) == 10 and all(len(group) == 65 for group in grouped.values()),
        "barrier_reconstruction": reconstructed_barrier <= 1e-16,
        "hysteresis_reconstruction": reconstructed_hysteresis <= 1e-16,
        "positive_barriers": all(p["relaxed_barrier"] > 1e-12 for p in paths),
        "periodic_endpoints": all(
            p["rigid_endpoint_residual"] <= 1e-12
            and p["relaxed_endpoint_residual"] <= 1e-12
            for p in paths
        ),
        "threshold_identity": all(p["threshold_energy_residual"] <= 1e-12 for p in paths),
        "transverse_bistability": all(p["hysteresis"] > 1e-4 for p in transverse),
        "axial_single_branch": all(p["hysteresis"] <= 1e-12 for p in axial),
        "landscape_rotates": record["maximum_landscape_covariance_residual"] <= 1e-12,
        "dynamic_covariance": record["dynamic_covariance_pass"]
        and record["maximum_dynamic_covariance_residual"] <= 1e-10
        and all(a["complete"] for a in arms),
        "no_mobile_energy_contradiction": no_mobile_below_path,
        "static_budget_not_sufficient": pinned_above_path >= 8
        and maximum_pinned_budget_ratio > 4.0,
        "locked_unresolved_verdict": (not record["path_coverage"])
        and (not record["landscape_covariance_pass"])
        and record["verdict"] == "REFINED_CORE_PEIERLS_LANDSCAPE_NUMERICALLY_UNRESOLVED",
    }
    failed = [name for name, passed in checks.items() if not passed]
    print(f"FTD-0614 independent certificate: {len(checks)-len(failed)}/{len(checks)} checks pass")
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    print(f"selected_path_barrier_range={record['minimum_relaxed_barrier']:.17g},{record['maximum_relaxed_barrier']:.17g}")
    print(f"maximum_transverse_hysteresis={max(p['hysteresis'] for p in transverse):.17g}")
    print(f"maximum_pinned_budget_ratio={maximum_pinned_budget_ratio:.17g}")
    if failed:
        print("failed=" + ",".join(failed))
        return 1
    print("verdict=REFINED_CORE_PEIERLS_LANDSCAPE_NUMERICALLY_UNRESOLVED")
    print("measured_pattern=positive selected-path barriers; transverse internal branch hysteresis; exact proper covariance")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

