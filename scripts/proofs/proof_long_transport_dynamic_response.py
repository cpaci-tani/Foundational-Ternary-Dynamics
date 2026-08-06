#!/usr/bin/env python3
"""Independent scalar certificate for the locked FTD-0768 CUDA artifact.

This verifier performs no fit or numerical search.  It reconstructs the
registered gates and outcome from the serialized run-of-record scalars.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
from typing import Any


PROTOCOL_SHA256 = (
    "5E4D0E9A81BD8C7E901A765792284E1BEF64129791CC874357D22F9630A2F48F"
)
EXPECTED_TIMES = list(range(0, 769, 64))
COMMON_GATE = 1e-12
REGIONAL_GATE = 1e-10
REVERSE_GATE = 1e-10


class Certificate:
    def __init__(self) -> None:
        self.checks = 0
        self.failures: list[str] = []

    def check(self, condition: bool, label: str) -> None:
        self.checks += 1
        if not condition:
            self.failures.append(label)


def finite(value: Any) -> bool:
    return isinstance(value, (int, float)) and math.isfinite(float(value))


def close(lhs: float, rhs: float, tolerance: float = 1e-12) -> bool:
    return abs(lhs - rhs) <= tolerance * max(1.0, abs(lhs), abs(rhs))


def vec_distance(lhs: list[float], rhs: list[float]) -> float:
    return math.sqrt(sum((float(a) - float(b)) ** 2 for a, b in zip(lhs, rhs)))


def vec_finite(value: Any) -> bool:
    return (
        isinstance(value, list)
        and len(value) == 3
        and all(finite(component) for component in value)
    )


def vec_add(lhs: list[float], rhs: list[float]) -> list[float]:
    return [float(a) + float(b) for a, b in zip(lhs, rhs)]


def check_channel(certificate: Certificate, channel: dict[str, Any], label: str) -> None:
    required = [
        "moving_energy",
        "rest_energy",
        "energy_difference",
        "difference_field_energy",
        "cross_energy",
        "energy_identity_residual",
        "energy_difference_first_moment",
        "difference_field_first_moment",
        "cross_first_moment",
    ]
    certificate.check(all(finite(channel.get(key)) for key in required), f"{label}: finite")
    if not all(finite(channel.get(key)) for key in required):
        return
    moving = float(channel["moving_energy"])
    rest = float(channel["rest_energy"])
    difference = float(channel["energy_difference"])
    norm = float(channel["difference_field_energy"])
    cross = float(channel["cross_energy"])
    residual = float(channel["energy_identity_residual"])
    certificate.check(close(difference, moving - rest), f"{label}: signed difference")
    certificate.check(norm >= -1e-15, f"{label}: nonnegative difference norm")
    certificate.check(close(residual, difference - cross - norm), f"{label}: residual serialization")
    certificate.check(abs(residual) <= COMMON_GATE * max(1.0, abs(moving), abs(rest)),
                      f"{label}: quadratic identity gate")


def check_region_cumulative(
    certificate: Certificate, region: dict[str, Any], label: str
) -> None:
    keys = [
        "boundary_transport",
        "boundary_transport_complement",
        "source_exchange",
        "energy_change",
        "mask_sweep",
        "mask_sweep_complement",
        "initial_region_energy",
        "endpoint_region_energy",
        "transported_energy_change",
        "accumulated_residual",
        "accumulated_transport_identity_residual",
        "accumulated_transport_ledger_residual",
        "accumulated_boundary_quadrature_residual",
        "accumulated_mask_sweep_quadrature_residual",
        "maximum_tick_residual",
        "maximum_global_source_free_residual",
        "maximum_boundary_quadrature_residual",
        "maximum_mask_sweep_quadrature_residual",
        "maximum_transport_identity_residual",
        "maximum_endpoint_chain_residual",
    ]
    certificate.check(all(finite(region.get(key)) for key in keys), f"{label}: finite")
    if not all(finite(region.get(key)) for key in keys):
        return
    residual = (
        float(region["energy_change"])
        - float(region["boundary_transport"])
        - float(region["source_exchange"])
    )
    certificate.check(close(float(region["accumulated_residual"]), residual),
                      f"{label}: accumulated identity")
    scale = max(
        1.0,
        abs(float(region["energy_change"])),
        abs(float(region["boundary_transport"])),
        abs(float(region["source_exchange"])),
    )
    certificate.check(abs(residual) <= REGIONAL_GATE * scale,
                      f"{label}: accumulated regional gate")
    boundary_residual = (
        float(region["boundary_transport"])
        + float(region["boundary_transport_complement"])
    )
    certificate.check(
        close(
            float(region["accumulated_boundary_quadrature_residual"]),
            boundary_residual,
        ),
        f"{label}: complementary quadrature serialization",
    )
    certificate.check(abs(boundary_residual) <= REGIONAL_GATE * scale,
                      f"{label}: complementary boundary orientation")
    initialized = region.get("initialized") is True
    if initialized:
        transported = (
            float(region["endpoint_region_energy"])
            - float(region["initial_region_energy"])
        )
        certificate.check(
            close(float(region["transported_energy_change"]), transported),
            f"{label}: transported endpoint identity",
        )
        transport_identity = (
            transported
            - float(region["energy_change"])
            - float(region["mask_sweep"])
        )
        certificate.check(
            close(
                float(region["accumulated_transport_identity_residual"]),
                transport_identity,
            ),
            f"{label}: transport identity serialization",
        )
        transport_ledger = (
            transported
            - float(region["boundary_transport"])
            - float(region["source_exchange"])
            - float(region["mask_sweep"])
        )
        certificate.check(
            close(
                float(region["accumulated_transport_ledger_residual"]),
                transport_ledger,
            ),
            f"{label}: transport ledger serialization",
        )
        transport_scale = max(
            scale,
            abs(float(region["initial_region_energy"])),
            abs(float(region["endpoint_region_energy"])),
            abs(transported),
            abs(float(region["mask_sweep"])),
        )
        certificate.check(
            abs(transport_identity) <= REGIONAL_GATE * transport_scale,
            f"{label}: cumulative transport identity gate",
        )
        certificate.check(
            abs(transport_ledger) <= REGIONAL_GATE * transport_scale,
            f"{label}: cumulative moving-control-volume ledger",
        )
    else:
        certificate.check(
            all(abs(float(region[key])) <= 1e-15 for key in (
                "initial_region_energy",
                "endpoint_region_energy",
                "transported_energy_change",
                "mask_sweep",
                "mask_sweep_complement",
            )),
            f"{label}: uninitialized zero record",
        )
    sweep_residual = (
        float(region["mask_sweep"])
        + float(region["mask_sweep_complement"])
    )
    certificate.check(
        close(
            float(region["accumulated_mask_sweep_quadrature_residual"]),
            sweep_residual,
        ),
        f"{label}: sweep quadrature serialization",
    )
    certificate.check(
        abs(sweep_residual) <= REGIONAL_GATE * scale,
        f"{label}: complementary sweep orientation",
    )
    certificate.check(float(region["maximum_tick_residual"]) <= COMMON_GATE,
                      f"{label}: per-tick regional gate")
    certificate.check(
        float(region["maximum_global_source_free_residual"]) <= COMMON_GATE,
        f"{label}: global source-free gate",
    )
    certificate.check(
        float(region["maximum_boundary_quadrature_residual"]) <= COMMON_GATE,
        f"{label}: boundary quadrature gate",
    )
    certificate.check(
        float(region["maximum_mask_sweep_quadrature_residual"]) <= REGIONAL_GATE * scale,
        f"{label}: per-step sweep quadrature gate",
    )
    certificate.check(
        float(region["maximum_transport_identity_residual"]) <= REGIONAL_GATE * scale,
        f"{label}: per-step transport identity gate",
    )
    certificate.check(
        float(region["maximum_endpoint_chain_residual"]) <= REGIONAL_GATE * scale,
        f"{label}: endpoint chain gate",
    )
    if "laboratory" in label:
        certificate.check(
            abs(float(region["mask_sweep"])) <= COMMON_GATE * scale,
            f"{label}: fixed laboratory has zero mask sweep",
        )


def check_cumulative(
    certificate: Certificate, cumulative: dict[str, Any], tau: int, label: str
) -> None:
    certificate.check(cumulative.get("valid") is True, f"{label}: valid")
    check_region_cumulative(certificate, cumulative["laboratory"], f"{label}: laboratory")
    check_region_cumulative(certificate, cumulative["moving_near"], f"{label}: moving near")
    scalars = [
        "matter_work",
        "field_work",
        "current_work",
        "maximum_common_residual",
        "maximum_energy_residual",
        "maximum_speed_excess",
        "minimum_graph_margin",
        "minimum_energy_margin",
        "maximum_inverse_residual",
    ]
    certificate.check(all(finite(cumulative.get(key)) for key in scalars), f"{label}: scalar finite")
    if not all(finite(cumulative.get(key)) for key in scalars):
        return
    certificate.check(float(cumulative["maximum_common_residual"]) <= COMMON_GATE,
                      f"{label}: common action")
    certificate.check(float(cumulative["maximum_energy_residual"]) <= COMMON_GATE,
                      f"{label}: complete energy")
    certificate.check(float(cumulative["maximum_speed_excess"]) <= COMMON_GATE,
                      f"{label}: causal speed")
    certificate.check(float(cumulative["minimum_graph_margin"]) >= 1e-6,
                      f"{label}: graph margin")
    certificate.check(float(cumulative["minimum_energy_margin"]) >= 1e-6,
                      f"{label}: energy margin")
    if tau > 0:
        certificate.check(finite(cumulative.get("minimum_sigma")), f"{label}: sigma finite")
        certificate.check(finite(cumulative.get("maximum_condition")), f"{label}: condition finite")
        if finite(cumulative.get("minimum_sigma")):
            certificate.check(float(cumulative["minimum_sigma"]) >= 1e-3,
                              f"{label}: sigma gate")
        if finite(cumulative.get("maximum_condition")):
            certificate.check(float(cumulative["maximum_condition"]) <= 1e4,
                              f"{label}: condition gate")
        certificate.check(float(cumulative["maximum_inverse_residual"]) <= COMMON_GATE,
                          f"{label}: one-step inverse")


def check_interval_region(
    certificate: Certificate,
    before: dict[str, Any],
    after: dict[str, Any],
    label: str,
) -> None:
    energy = float(after["energy_change"]) - float(before["energy_change"])
    boundary = (
        float(after["boundary_transport"])
        - float(before["boundary_transport"])
    )
    complement = (
        float(after["boundary_transport_complement"])
        - float(before["boundary_transport_complement"])
    )
    source = float(after["source_exchange"]) - float(before["source_exchange"])
    residual = energy - boundary - source
    scale = max(1.0, abs(energy), abs(boundary), abs(source))
    certificate.check(
        abs(residual) <= REGIONAL_GATE * scale,
        f"{label}: interval regional balance",
    )
    certificate.check(
        abs(boundary + complement) <= REGIONAL_GATE * scale,
        f"{label}: interval complementary boundary",
    )
    transported = (
        float(after["transported_energy_change"])
        - float(before["transported_energy_change"])
    )
    sweep = float(after["mask_sweep"]) - float(before["mask_sweep"])
    sweep_complement = (
        float(after["mask_sweep_complement"])
        - float(before["mask_sweep_complement"])
    )
    transport_residual = transported - boundary - source - sweep
    transport_scale = max(scale, abs(transported), abs(sweep))
    certificate.check(
        abs(transport_residual) <= REGIONAL_GATE * transport_scale,
        f"{label}: interval moving-control-volume balance",
    )
    certificate.check(
        abs(sweep + sweep_complement) <= REGIONAL_GATE * transport_scale,
        f"{label}: interval complementary sweep",
    )


def check_momentum_checkpoint(
    certificate: Certificate,
    checkpoint: dict[str, Any],
    initial_totals: dict[str, list[float]],
    label: str,
) -> None:
    for arm in ("rest", "moving"):
        matter_key = f"{arm}_matter_momentum"
        for candidate in ("local", "spline"):
            field_key = f"{arm}_{candidate}_momentum"
            defect_key = f"{arm}_{candidate}_momentum_defect"
            matter = checkpoint.get(matter_key)
            field = checkpoint.get(field_key)
            defect = checkpoint.get(defect_key)
            certificate.check(
                vec_finite(matter) and vec_finite(field) and finite(defect),
                f"{label}: {arm} {candidate} momentum finite",
            )
            if not (vec_finite(matter) and vec_finite(field) and finite(defect)):
                continue
            total = vec_add(matter, field)
            reconstructed = vec_distance(
                total, initial_totals[f"{arm}_{candidate}"]
            )
            certificate.check(
                close(float(defect), reconstructed),
                f"{label}: {arm} {candidate} defect reconstruction",
            )


def monotone_nonincreasing(values: list[float]) -> bool:
    if len(values) < 2:
        return False
    return all(
        values[index] <= values[index - 1] + 1e-15 * max(1.0, values[index - 1])
        for index in range(1, len(values))
    )


def reconstruct_outcome(data: dict[str, Any]) -> tuple[str, bool, int]:
    checkpoints = data.get("checkpoints", [])
    execution = (
        data.get("parent_valid") is True
        and data.get("aging_valid") is True
        and data.get("rest_initialized") is True
        and data.get("moving_initialized") is True
        and data.get("forward_valid") is True
        and finite(data.get("maximum_rest_displacement"))
        and float(data["maximum_rest_displacement"]) <= COMMON_GATE
        and data.get("boundary_clear") is True
        and data.get("reverse_valid") is True
        and data.get("reverse_discrete_exact") is True
        and finite(data.get("reverse_recovery"))
        and float(data["reverse_recovery"]) <= REVERSE_GATE
        and data.get("reverse_steps") == 768
        and len(checkpoints) == 13
        and all(checkpoint.get("valid") is True for checkpoint in checkpoints)
    )
    if not execution:
        return "LONG_TRANSPORT_EXECUTION_INVALID", False, -1
    clearing = [checkpoint for checkpoint in checkpoints if checkpoint.get("clearing") is True]
    if not clearing:
        return "CORE_CLEARING_NOT_REACHED", False, -1
    first_tau = int(clearing[0]["tau"])
    post = [checkpoint for checkpoint in checkpoints if int(checkpoint["tau"]) >= first_tau]
    if len(post) < 2:
        return "CORE_CLEARING_REACHED_RESPONSE_UNRESOLVED", True, first_tau
    delta_u = [abs(float(checkpoint["regions"][0]["actual"]["energy_difference"]))
               for checkpoint in post]
    norm = [float(checkpoint["regions"][0]["actual"]["difference_field_energy"])
            for checkpoint in post]
    floor = 1e-6 * float(data["initial_pair_energy_scale"])
    decays = monotone_nonincreasing(delta_u) and monotone_nonincreasing(norm)
    persists = all(value >= floor for value in delta_u) or all(value >= floor for value in norm)
    if decays:
        return "CLEARED_LOCAL_RESPONSE_DECAYS", True, first_tau
    if persists:
        return "CLEARED_LOCAL_RESPONSE_PERSISTS", True, first_tau
    return "CLEARED_LOCAL_RESPONSE_MIXED", True, first_tau


def certify(path: Path) -> dict[str, Any]:
    certificate = Certificate()
    raw = path.read_bytes()
    data = json.loads(raw)
    certificate.check(data.get("ftd_id") == "FTD-0768", "identity")
    certificate.check(data.get("protocol_sha256") == PROTOCOL_SHA256, "protocol hash")
    certificate.check(
        data.get("run_record_schema")
        == "ftd_0768_long_transport_dynamic_response_v1",
        "run-record schema",
    )
    certificate.check(
        data.get("field_representation")
        == "matched_oriented_face_electric_edge_magnetic_half",
        "field representation",
    )
    certificate.check(
        data.get("observer_mode")
        == "paired_actual_selected_residual_complementary_boundary_moving_control_volume_sweep",
        "observer mode",
    )
    certificate.check(data.get("volume") == 321, "volume")
    certificate.check(data.get("formation_ticks") == 160, "formation")
    certificate.check(data.get("preparation_age") == 128, "age")
    certificate.check(data.get("discovery_ticks") == 768, "ticks")
    certificate.check(data.get("checkpoint_stride") == 64, "stride")
    certificate.check(data.get("clearing_distance") == 9, "clearing distance")
    certificate.check(close(float(data.get("boost", math.nan)), 0.030, 1e-15), "boost")
    certificate.check(data.get("direction") == [0, 0, 1], "direction")
    tolerances = data.get("tolerances", {})
    certificate.check(close(float(tolerances.get("common", math.nan)), COMMON_GATE),
                      "common tolerance")
    certificate.check(close(float(tolerances.get("regional", math.nan)), REGIONAL_GATE),
                      "regional tolerance")
    certificate.check(close(float(tolerances.get("reverse", math.nan)), REVERSE_GATE),
                      "reverse tolerance")
    certificate.check(close(float(tolerances.get("minimum_root_singular_value", math.nan)),
                            1e-3), "root singular-value tolerance")
    certificate.check(close(float(tolerances.get("maximum_condition_number", math.nan)),
                            1e4), "condition-number tolerance")
    certificate.check(close(float(tolerances.get("minimum_core_margin", math.nan)),
                            1e-6), "core-margin tolerance")
    expected_margin = 160.5 - 4.0 - 1056.0 / (4.0 * math.sqrt(3.0))
    certificate.check(close(float(data.get("boundary_margin", math.nan)), expected_margin),
                      "causal boundary margin")
    certificate.check(data.get("production_changed") is False, "production unchanged")
    certificate.check(data.get("dynamics_changed") is False, "dynamics unchanged")
    certificate.check(data.get("new_primitive_added") is False, "primitive unchanged")
    certificate.check(data.get("wake_label_available") is False, "no wake label")
    certificate.check(
        finite(data.get("maximum_rest_displacement"))
        and float(data["maximum_rest_displacement"]) <= COMMON_GATE,
        "every-tick rest displacement",
    )

    checkpoints = data.get("checkpoints", [])
    if data.get("forward_valid") is True:
        certificate.check([item.get("tau") for item in checkpoints] == EXPECTED_TIMES,
                          "checkpoint matrix")
    laboratory_center = data.get("laboratory_center", [math.nan] * 3)
    initial_totals: dict[str, list[float]] = {}
    if checkpoints:
        initial = checkpoints[0]
        for arm in ("rest", "moving"):
            for candidate in ("local", "spline"):
                matter = initial.get(f"{arm}_matter_momentum")
                field = initial.get(f"{arm}_{candidate}_momentum")
                certificate.check(
                    vec_finite(matter) and vec_finite(field),
                    f"tau=0: {arm} {candidate} initial momentum",
                )
                if vec_finite(matter) and vec_finite(field):
                    initial_totals[f"{arm}_{candidate}"] = vec_add(matter, field)
    for checkpoint in checkpoints:
        tau = int(checkpoint["tau"])
        label = f"tau={tau}"
        certificate.check(checkpoint.get("clearing")
                          == (float(checkpoint["displacement"]) >= 9.0),
                          f"{label}: clearing predicate")
        certificate.check(bool(checkpoint.get("rest_state_hash")), f"{label}: rest hash")
        certificate.check(bool(checkpoint.get("moving_state_hash")), f"{label}: moving hash")
        certificate.check(vec_distance(checkpoint["rest_center"], laboratory_center) <= 1e-12,
                          f"{label}: rest static")
        if len(initial_totals) == 4:
            check_momentum_checkpoint(certificate, checkpoint, initial_totals, label)
        telemetry = checkpoint["paired_telemetry"]
        certificate.check(telemetry.get("valid") is True, f"{label}: paired telemetry")
        certificate.check(telemetry.get("complete_field_downloads") == 0,
                          f"{label}: scalar-only paired observer")
        certificate.check(int(telemetry.get("device_to_host_bytes", 0)) > 0,
                          f"{label}: scalar reduction returned")
        maximum_identity = 0.0
        regions = checkpoint.get("regions", [])
        certificate.check(len(regions) == 4, f"{label}: four regions")
        for region_index, region in enumerate(regions):
            for channel_name in ("actual", "residual"):
                channel = region[channel_name]
                check_channel(certificate, channel,
                              f"{label}: region={region_index}: {channel_name}")
                if finite(channel.get("energy_identity_residual")):
                    maximum_identity = max(
                        maximum_identity, abs(float(channel["energy_identity_residual"]))
                    )
        certificate.check(close(
            float(checkpoint["maximum_energy_identity_residual"]), maximum_identity
        ), f"{label}: maximum identity")
        check_cumulative(certificate, checkpoint["rest_cumulative"], tau, f"{label}: rest")
        check_cumulative(certificate, checkpoint["moving_cumulative"], tau, f"{label}: moving")

    for index in range(1, len(checkpoints)):
        before = checkpoints[index - 1]
        after = checkpoints[index]
        interval_label = f"tau={before['tau']}..{after['tau']}"
        for arm in ("rest", "moving"):
            for region in ("laboratory", "moving_near"):
                check_interval_region(
                    certificate,
                    before[f"{arm}_cumulative"][region],
                    after[f"{arm}_cumulative"][region],
                    f"{interval_label}: {arm}: {region}",
                )

    outcome, clearing, first_tau = reconstruct_outcome(data)
    certificate.check(data.get("outcome") == outcome, "outcome reconstruction")
    certificate.check(data.get("clearing_reached") is clearing, "clearing summary")
    certificate.check(data.get("first_clearing_tau") == first_tau, "first clearing summary")
    if data.get("forward_valid") is True:
        certificate.check(data.get("reverse_steps") == 768, "full reverse history")
        certificate.check(data.get("reverse_discrete_exact") is True, "reverse discrete exact")
        certificate.check(float(data.get("reverse_recovery", math.inf)) <= REVERSE_GATE,
                          "reverse continuous recovery")
        certificate.check(float(data.get("reverse_maximum_common", math.inf)) <= COMMON_GATE,
                          "reverse common-action gate")
    certificate.check(bool(data.get("moving_initial_hash")), "initial state hash")
    certificate.check(bool(data.get("moving_forward_final_hash")), "forward state hash")
    if data.get("reverse_steps", 0) > 0:
        certificate.check(bool(data.get("moving_reversed_hash")), "reverse state hash")
    return {
        "certificate_passed": not certificate.failures,
        "checks": certificate.checks,
        "failures": certificate.failures,
        "artifact_sha256": hashlib.sha256(raw).hexdigest().upper(),
        "outcome": data.get("outcome"),
    }


def self_test() -> dict[str, Any]:
    base = {
        "parent_valid": True,
        "aging_valid": True,
        "rest_initialized": True,
        "moving_initialized": True,
        "forward_valid": True,
        "maximum_rest_displacement": 0.0,
        "boundary_clear": True,
        "reverse_valid": True,
        "reverse_discrete_exact": True,
        "reverse_recovery": 0.0,
        "reverse_steps": 768,
        "initial_pair_energy_scale": 1.0,
        "checkpoints": [],
    }
    for tau in EXPECTED_TIMES:
        base["checkpoints"].append({
            "tau": tau,
            "valid": True,
            "clearing": tau >= 640,
            "regions": [{"actual": {
                "energy_difference": 1.0 / (1 + tau),
                "difference_field_energy": 1.0 / (1 + tau),
            }}],
        })
    outcome, clearing, first = reconstruct_outcome(base)
    certificate = Certificate()
    region0 = {
        "energy_change": 0.0,
        "transported_energy_change": 0.0,
        "mask_sweep": 0.0,
        "mask_sweep_complement": 0.0,
        "boundary_transport": 0.0,
        "boundary_transport_complement": 0.0,
        "source_exchange": 0.0,
    }
    region1 = {
        "energy_change": 3.0,
        "transported_energy_change": 3.5,
        "mask_sweep": 0.5,
        "mask_sweep_complement": -0.5,
        "boundary_transport": 1.0,
        "boundary_transport_complement": -1.0,
        "source_exchange": 2.0,
    }
    check_interval_region(certificate, region0, region1, "self-test interval")
    cumulative_region = {
        "initialized": True,
        "boundary_transport": 1.0,
        "boundary_transport_complement": -1.0,
        "source_exchange": 2.0,
        "energy_change": 3.0,
        "mask_sweep": 0.5,
        "mask_sweep_complement": -0.5,
        "initial_region_energy": 10.0,
        "endpoint_region_energy": 13.5,
        "transported_energy_change": 3.5,
        "accumulated_residual": 0.0,
        "accumulated_transport_identity_residual": 0.0,
        "accumulated_transport_ledger_residual": 0.0,
        "accumulated_boundary_quadrature_residual": 0.0,
        "accumulated_mask_sweep_quadrature_residual": 0.0,
        "maximum_tick_residual": 0.0,
        "maximum_global_source_free_residual": 0.0,
        "maximum_boundary_quadrature_residual": 0.0,
        "maximum_mask_sweep_quadrature_residual": 0.0,
        "maximum_transport_identity_residual": 0.0,
        "maximum_endpoint_chain_residual": 0.0,
    }
    check_region_cumulative(certificate, cumulative_region, "self-test moving near")
    initial_totals = {
        "rest_local": [1.0, 0.0, 0.0],
        "rest_spline": [0.0, 1.0, 0.0],
        "moving_local": [0.0, 0.0, 1.0],
        "moving_spline": [1.0, 1.0, 0.0],
    }
    momentum_checkpoint = {
        "rest_matter_momentum": [0.25, 0.0, 0.0],
        "rest_local_momentum": [0.75, 0.0, 0.0],
        "rest_spline_momentum": [-0.25, 1.0, 0.0],
        "moving_matter_momentum": [0.0, 0.0, 0.25],
        "moving_local_momentum": [0.0, 0.0, 0.75],
        "moving_spline_momentum": [1.0, 1.0, -0.25],
        "rest_local_momentum_defect": 0.0,
        "rest_spline_momentum_defect": 0.0,
        "moving_local_momentum_defect": 0.0,
        "moving_spline_momentum_defect": 0.0,
    }
    check_momentum_checkpoint(
        certificate, momentum_checkpoint, initial_totals, "self-test momentum"
    )
    return {
        "passed": outcome == "CLEARED_LOCAL_RESPONSE_DECAYS"
        and clearing and first == 640 and not certificate.failures,
        "outcome": outcome,
        "first_clearing_tau": first,
        "observer_checks": certificate.checks,
        "observer_failures": certificate.failures,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--artifact",
        type=Path,
        default=Path("engine/results/ftd_0768/ftd_0768_long_transport_dynamic_response_v1.json"),
    )
    parser.add_argument("--self-test", action="store_true")
    arguments = parser.parse_args()
    if arguments.self_test:
        result = self_test()
    else:
        result = certify(arguments.artifact)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result.get("passed", result.get("certificate_passed", False)) else 1


if __name__ == "__main__":
    raise SystemExit(main())
