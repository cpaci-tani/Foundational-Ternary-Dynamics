"""Independent frozen-artifact certificate for FTD-0764."""

from __future__ import annotations

import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0764"
PROTOCOL = "4F68CCD8A037363438CF94B728C56059066BFA9B2B3D8C0F82A6A5DDF3D7BDF8"
EXPECTED_HASHES = {
    "ftd_0764_transported_chart_morphology_v1.json":
        "9769BE1330CF422FDC10CE77CF89057153E634FDC3E054522C58F2CC0145AA56",
    "ftd_0764_transported_chart_morphology_v1_face.json":
        "0DDB53E3A138AF564EB3FF09F6D052D0120E9A3D74D5B132850D86990FB1017C",
    "ftd_0764_transported_chart_morphology_v1_edge.json":
        "762E5C50315C2564070CDBA0009635673EA4519F5810692418996F46A38C7AA4",
    "ftd_0764_transported_chart_morphology_v1_body.json":
        "43E2182CCE16BDC356A6FC2BB9A275343F9633571DA45AEC2C7CEA76E4412DF5",
}
EXPECTED_FINAL = {
    "face": {
        "near_distance": 0.18008270414662531,
        "near_ratio": 1.1015000323742306,
        "local_defect": 0.00727570019754039,
        "spline_defect": 0.007198704309182829,
    },
    "edge": {
        "near_distance": 0.18056403398749435,
        "near_ratio": 1.2965055626515312,
        "local_defect": 0.008901253179568126,
        "spline_defect": 0.008827065334194001,
    },
    "body": {
        "near_distance": 0.23602040648854983,
        "near_ratio": 1.5736770744117081,
        "local_defect": 0.0069239361249579245,
        "spline_defect": 0.006966175112285912,
    },
}

checks = 0
failures: list[str] = []


def check(label: str, condition: bool) -> None:
    global checks
    checks += 1
    if not condition:
        failures.append(label)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def finite_tree(value: object) -> bool:
    if isinstance(value, bool) or value is None or isinstance(value, str):
        return True
    if isinstance(value, (int, float)):
        return math.isfinite(float(value))
    if isinstance(value, list):
        return all(finite_tree(item) for item in value)
    if isinstance(value, dict):
        return all(finite_tree(item) for item in value.values())
    return False


def norm(values: list[float]) -> float:
    return math.sqrt(sum(float(value) ** 2 for value in values))


def close(left: float, right: float, tolerance: float = 1e-14) -> bool:
    return abs(float(left) - float(right)) <= tolerance


for name, expected in EXPECTED_HASHES.items():
    path = RESULTS / name
    check(f"artifact exists: {name}", path.is_file())
    if path.is_file():
        check(f"artifact hash: {name}", sha256(path) == expected)

aggregate_path = RESULTS / "ftd_0764_transported_chart_morphology_v1.json"
aggregate = json.loads(aggregate_path.read_text(encoding="utf-8"))
check("aggregate id", aggregate.get("ftd_id") == "FTD-0764")
check("aggregate protocol", aggregate.get("protocol_sha256") == PROTOCOL)
check("aggregate complete", aggregate.get("all_artifacts_present") is True)
check("aggregate execution", aggregate.get("all_execution_valid") is True)
check("aggregate bound controls fail", aggregate.get("all_bound_controls") is False)
check("aggregate near coherence fails", aggregate.get("all_near_coherent") is False)
check("aggregate detached outgoing", aggregate.get("all_detached_outgoing") is True)
check("aggregate trailing wake", aggregate.get("all_trailing_wake") is True)
check("aggregate local momentum open", aggregate.get("all_local_momentum_close") is False)
check("aggregate spline momentum open", aggregate.get("all_spline_momentum_close") is False)
check("aggregate verdicts", aggregate.get("ray_verdicts") == [
    "NO_TRANSPORTED_FIELD_COHERENCE",
    "NO_TRANSPORTED_FIELD_COHERENCE",
    "NO_TRANSPORTED_FIELD_COHERENCE",
])
check("aggregate production frozen", aggregate.get("production_changed") is False)
check("aggregate dynamics frozen", aggregate.get("dynamics_changed") is False)

for slug in ("face", "edge", "body"):
    path = RESULTS / f"ftd_0764_transported_chart_morphology_v1_{slug}.json"
    value = json.loads(path.read_text(encoding="utf-8"))
    check(f"{slug} finite tree", finite_tree(value))
    check(f"{slug} id", value.get("ftd_id") == "FTD-0764")
    check(f"{slug} protocol", value.get("protocol_sha256") == PROTOCOL)
    check(f"{slug} label", value.get("slug") == slug)
    check(f"{slug} volume", value.get("volume") == 321)
    check(f"{slug} parent", value.get("parent_valid") is True)
    check(f"{slug} execution", value.get("execution_valid") is True)
    check(f"{slug} verdict",
          value.get("morphology_verdict") == "NO_TRANSPORTED_FIELD_COHERENCE")
    check(f"{slug} bound control fails", value.get("bound_control") is False)
    check(f"{slug} near coherence fails", value.get("near_coherent") is False)
    check(f"{slug} detached outgoing", value.get("detached_outgoing") is True)
    check(f"{slug} trailing wake", value.get("trailing_wake") is True)
    check(f"{slug} local momentum open", value.get("local_momentum_closes") is False)
    check(f"{slug} spline momentum open", value.get("spline_momentum_closes") is False)
    check(f"{slug} production frozen", value.get("production_changed") is False)
    check(f"{slug} dynamics frozen", value.get("dynamics_changed") is False)
    check(f"{slug} no invented momentum",
          value.get("substrate_momentum_invented") is False)

    for arm_name in ("rest", "plus"):
        arm = value[arm_name]
        check(f"{slug} {arm_name} name", arm.get("name") == arm_name)
        check(f"{slug} {arm_name} initialized", arm.get("initialized") is True)
        check(f"{slug} {arm_name} executed", arm.get("executed") is True)
        check(f"{slug} {arm_name} valid", arm.get("valid") is True)
        checkpoints = arm["checkpoints"]
        check(f"{slug} {arm_name} ticks",
              [item["tick"] for item in checkpoints] == [160, 176, 192, 208, 224])
        for checkpoint in checkpoints:
            tick = checkpoint["tick"]
            prefix = f"{slug} {arm_name} tick {tick}"
            check(f"{prefix} valid", checkpoint.get("valid") is True)
            check(f"{prefix} observer", checkpoint.get("fractional_observer_valid") is True)
            check(f"{prefix} boundary ledger", checkpoint.get("boundary_ledger_valid") is True)
            check(f"{prefix} ladder", checkpoint.get("ladder_valid") is True)
            check(f"{prefix} causal", float(checkpoint["speed_excess"]) <= 1e-12)
            check(f"{prefix} common", abs(float(checkpoint["common_residual"])) <= 1e-12)
            check(f"{prefix} energy", abs(float(checkpoint["energy_residual"])) <= 1e-12)
            check(f"{prefix} inverse valid", checkpoint.get("inverse_valid") is True)
            check(f"{prefix} inverse residual",
                  abs(float(checkpoint["inverse_residual"])) <= 1e-12)

            evidence = checkpoint["field_evidence"]
            check(f"{prefix} evidence valid", evidence.get("valid") is True)
            check(f"{prefix} evidence observer", evidence.get("observer_valid") is True)
            check(f"{prefix} evidence boundary",
                  evidence.get("boundary_ledger_valid") is True)
            check(f"{prefix} evidence ladder", evidence.get("ladder_valid") is True)
            check(f"{prefix} evidence scalar only",
                  evidence.get("cuda_scalar_only") is True)
            for key in (
                "actual_gauss_residual",
                "energy_partition_residual",
                "boundary_identity_residual",
                "readout_reconstruction_residual",
                "characteristic_flux_residual",
                "ladder_energy_residual",
                "ladder_projection_residual",
            ):
                check(f"{prefix} {key}", abs(float(evidence[key])) <= 1e-12)
            check(f"{prefix} scalar transfer",
                  int(evidence["device_to_host_bytes"]) < 4 * 1024 * 1024)
            check(f"{prefix} shell radii",
                  [shell["radius"] for shell in evidence["shells"]]
                  == [8, 12, 16, 24, 32, 48])

            morphology = checkpoint["morphology"]
            check(f"{prefix} morphology", morphology.get("valid") is True)
            check(f"{prefix} energy reconstruction",
                  abs(float(morphology["energy_reconstruction_residual"])) <= 1e-12)
            check(f"{prefix} mode reconstruction maximum",
                  abs(float(morphology["maximum_mode_reconstruction_residual"])) <= 1e-12)
            check(f"{prefix} mode count", len(morphology["modes"]) == 18)
            for mode_index, mode in enumerate(morphology["modes"]):
                reconstructed = [
                    float(mode["bound"][part])
                    + float(mode["residual"][part])
                    + float(mode["interference"][part])
                    for part in (0, 1)
                ]
                check(f"{prefix} mode {mode_index} identity",
                      norm([float(mode["actual"][part]) - reconstructed[part]
                            for part in (0, 1)]) <= 1e-12)

        first_center = checkpoints[0]["morphology"]["center"]
        last_center = checkpoints[-1]["morphology"]["center"]
        displacement = norm([
            float(last) - float(first)
            for first, last in zip(first_center, last_center)
        ])
        if arm_name == "rest":
            check(f"{slug} rest center fixed", displacement <= 1e-12)
            check(f"{slug} rest matter momentum",
                  max(norm(item["matter_momentum"]) for item in checkpoints) <= 1e-12)
        else:
            check(f"{slug} plus center moves", displacement > 0.39)

    rested = value["rest"]["checkpoints"]
    moved = value["plus"]["checkpoints"]
    check(f"{slug} comparisons registered",
          all(item["comparison"]["valid"] is True for item in moved[1:]))
    check(f"{slug} bound gate actually fails",
          any(float(item["comparison"]["bound_distance"]) > 0.02
              for item in moved[1:]))
    check(f"{slug} near gate actually fails",
          any(float(item["comparison"]["near_residual_distance"]) > 0.10
              or not 0.8 <= float(item["comparison"]["near_residual_energy_ratio"]) <= 1.2
              for item in moved[1:]))
    outer = [float(item["morphology"]["outer_residual_energy"])
             for item in moved]
    rest_outer = [float(item["morphology"]["outer_residual_energy"])
                  for item in rested]
    check(f"{slug} outer energy grows consecutively",
          outer[1] < outer[2] < outer[3] < outer[4])
    check(f"{slug} rest outer energy grows consecutively",
          rest_outer[1] < rest_outer[2] < rest_outer[3] < rest_outer[4])
    for index in (2, 3, 4):
        moving_shell48 = next(
            shell for shell in moved[index]["field_evidence"]["shells"]
            if shell["radius"] == 48
        )
        rest_shell48 = next(
            shell for shell in rested[index]["field_evidence"]["shells"]
            if shell["radius"] == 48
        )
        check(f"{slug} tick {moved[index]['tick']} shell48 outgoing",
              float(moving_shell48["signed_radial_poynting"]) > 0.0)
        check(f"{slug} tick {moved[index]['tick']} rest shell48 outgoing",
              float(rest_shell48["signed_radial_poynting"]) > 0.0)
    final_moving_shell48 = next(
        shell for shell in moved[-1]["field_evidence"]["shells"]
        if shell["radius"] == 48
    )
    final_rest_shell48 = next(
        shell for shell in rested[-1]["field_evidence"]["shells"]
        if shell["radius"] == 48
    )
    moving_flux = float(final_moving_shell48["signed_radial_poynting"])
    rest_flux = float(final_rest_shell48["signed_radial_poynting"])
    check(f"{slug} outgoing signal rest matched",
          abs(moving_flux - rest_flux) / max(abs(moving_flux), abs(rest_flux))
          <= 0.01)
    check(f"{slug} outer energy rest matched",
          abs(float(moved[-1]["morphology"]["outer_residual_energy"])
              - float(rested[-1]["morphology"]["outer_residual_energy"]))
          / max(float(moved[-1]["morphology"]["outer_residual_energy"]),
                float(rested[-1]["morphology"]["outer_residual_energy"]))
          <= 0.01)
    moments = [float(item["longitudinal_combined_moment"]) for item in moved]
    check(f"{slug} moved moments trail", all(item < 0.0 for item in moments[1:]))
    check(f"{slug} wake magnitude grows",
          sum(abs(moments[index]) > abs(moments[index - 1])
              for index in range(2, 5)) >= 3)
    check(f"{slug} local ledger truly open",
          any(float(item["local_defect"]) > 1e-9 for item in moved[1:]))
    check(f"{slug} spline ledger truly open",
          any(float(item["spline_defect"]) > 1e-9 for item in moved[1:]))

    expected = EXPECTED_FINAL[slug]
    final = moved[-1]
    check(f"{slug} final near distance locked",
          close(final["comparison"]["near_residual_distance"],
                expected["near_distance"]))
    check(f"{slug} final near ratio locked",
          close(final["comparison"]["near_residual_energy_ratio"],
                expected["near_ratio"]))
    check(f"{slug} final local defect locked",
          close(final["local_defect"], expected["local_defect"]))
    check(f"{slug} final spline defect locked",
          close(final["spline_defect"], expected["spline_defect"]))

print(f"FTD-0764 artifact certificate: {checks - len(failures)}/{checks} checks")
if failures:
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)
print("morphology_verdict=NO_TRANSPORTED_FIELD_COHERENCE")
print("registered_detached_outgoing_component=true")
print("motion_induced_radiation_established=false")
print("trailing_wake_candidate=true")
print("momentum_candidate_closes=false")
