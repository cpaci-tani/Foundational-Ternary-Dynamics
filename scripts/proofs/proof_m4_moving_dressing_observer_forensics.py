"""Independent frozen-artifact certificate for FTD-0762.

Consumes the three CUDA JSON records and reconstructs the registered
observer-chart verdict.  It does not run or modify the dynamics.
"""

from __future__ import annotations

import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0762"
PROTOCOL_HASH = "880293A2DC1F129637D1D1C28D8C0D9AE5FA3AC29D76042348CFE09ABB9E5B46"
RUNNER_HASH = "77FEDD160A6FBC12DD72EBA2DD025B1837146937D7A5CD45CEF250DC84778004"
EXECUTABLE_HASH = "3BBED134F84664F81880D8D99E28F8FEB541B6FD7D4A07301912415891C228D2"
ARTIFACT_HASHES = {
    "ftd_0762_moving_dressing_observer_forensics_v1_face.json":
        "B803F7152E7FD76C612B739640CF8593E09A9C9BE66BBF6BE025C22ED494B5F4",
    "ftd_0762_moving_dressing_observer_forensics_v1_edge.json":
        "A376C7991A8DFB40075FC029831B26018E7D401860A84BA5885EB7CA4EA6F295",
    "ftd_0762_moving_dressing_observer_forensics_v1_body.json":
        "02FD4577A70F000B075E41DB77B0A6FF9C0FB6535601B40F8EFD96B7DCAE2A34",
    "ftd_0762_moving_dressing_observer_forensics_v1.json":
        "279A1567C7973E10B58864EDEA8529FB20DFF83A03018CA41E8961788BFBFF83",
}
DIRECTIONS = {"face": "0_0_1", "edge": "0_1_-1", "body": "1_1_1"}
GATE = 1.0e-12

checks = 0
failures: list[str] = []


def check(label: str, condition: bool) -> None:
    global checks
    checks += 1
    if not condition:
        failures.append(label)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def load(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8"))


def finite(value: object) -> bool:
    return isinstance(value, (int, float)) and math.isfinite(float(value))


def certify_direction(slug: str) -> bool:
    name = f"ftd_0762_moving_dressing_observer_forensics_v1_{slug}.json"
    path = RESULTS / name
    check(f"{slug} artifact exists", path.is_file())
    if not path.is_file():
        return False
    check(f"{slug} artifact hash", sha256(path) == ARTIFACT_HASHES[name])
    try:
        value = load(path)
    except (OSError, json.JSONDecodeError):
        check(f"{slug} JSON parses", False)
        return False

    check(f"{slug} id", value.get("ftd_id") == "FTD-0762")
    check(f"{slug} protocol", value.get("protocol_sha256") == PROTOCOL_HASH)
    check(f"{slug} direction", value.get("direction") == DIRECTIONS[slug])
    check(f"{slug} slug", value.get("slug") == slug)
    check(f"{slug} volume", value.get("volume") == 321)
    check(f"{slug} formation", value.get("formation_tick") == 160)
    check(f"{slug} replay horizon", value.get("forensic_ticks") == 64)
    check(f"{slug} boost", value.get("boost") == 0.015)
    check(f"{slug} parent", value.get("parent_valid") is True)
    check(f"{slug} replay executed", value.get("replay_executed") is True)
    check(f"{slug} replay common", value.get("replay_common") is True)

    center = value.get("center")
    center_valid = (isinstance(center, list) and len(center) == 3
                    and all(finite(item) for item in center))
    check(f"{slug} center finite", center_valid)
    if center_valid:
        reconstructed = math.sqrt(sum(
            (float(item) - round(float(item))) ** 2 for item in center))
        reported = float(value.get("fractional_center_norm", math.inf))
        check(f"{slug} fractional norm reconstructed",
              math.isclose(reconstructed, reported,
                           rel_tol=5.0e-14, abs_tol=5.0e-14))
        check(f"{slug} center outside integer chart", reported > GATE)

    for key in ("cpu_observer_valid", "cpu_boundary_ledger_valid",
                "cpu_ladder_valid", "cuda_observer_valid",
                "cuda_boundary_ledger_valid", "cuda_ladder_valid",
                "same_geometry_preparation_valid"):
        check(f"{slug} {key} false", value.get(key) is False)
    check(f"{slug} CUDA observer failure localized",
          value.get("cuda_observer_error") ==
          "compact observer preparation failed")
    check(f"{slug} CUDA ladder failure localized",
          value.get("cuda_ladder_error") ==
          "compact bound preparation failed")

    for key in ("recentered_preparation_valid",
                "recentered_cuda_observer_valid",
                "recentered_cuda_boundary_ledger_valid",
                "recentered_cuda_ladder_valid"):
        check(f"{slug} {key} true", value.get(key) is True)
    for key in ("recentered_fractional_center_norm",
                "relative_geometry_residual",
                "momentum_preservation_residual",
                "recentered_maximum_reconstruction_residual",
                "recentered_actual_gauss_residual",
                "recentered_energy_partition_residual",
                "recentered_characteristic_flux_residual",
                "recentered_ladder_energy_residual",
                "recentered_ladder_projection_residual"):
        scalar = value.get(key)
        check(f"{slug} {key} finite", finite(scalar))
        if finite(scalar):
            check(f"{slug} {key} gated", abs(float(scalar)) <= GATE)
    check(f"{slug} CUDA transfer occurred",
          int(value.get("cuda_host_to_device_bytes", 0)) > 0
          and int(value.get("cuda_device_to_host_bytes", 0)) > 0)
    check(f"{slug} CUDA kernels executed",
          float(value.get("cuda_kernel_ms", 0.0)) > 0.0)
    check(f"{slug} chart verdict", value.get("chart_obstruction") is True)
    check(f"{slug} physical mismatch absent",
          value.get("physical_dressing_mismatch") is False)
    check(f"{slug} infrastructure resolved",
          value.get("infrastructure_unresolved") is False)
    check(f"{slug} production frozen",
          value.get("production_changed") is False)
    check(f"{slug} dynamics frozen", value.get("dynamics_changed") is False)
    return (value.get("chart_obstruction") is True
            and value.get("physical_dressing_mismatch") is False)


def main() -> int:
    protocol = (ROOT / "docs" / "theory" / "10_eft_program"
                / "preregistrations"
                / "PREREG_M4_MOVING_DRESSING_OBSERVER_FORENSICS_v1.md")
    runner = (ROOT / "engine" / "tests"
              / "campaign_m4_moving_dressing_observer_forensics_cuda.cpp")
    executable = (ROOT / "engine" / "build_wsl"
                  / "campaign_m4_moving_dressing_observer_forensics_cuda")
    for label, path, expected in (
        ("protocol", protocol, PROTOCOL_HASH),
        ("runner", runner, RUNNER_HASH),
        ("executable", executable, EXECUTABLE_HASH),
    ):
        check(f"{label} exists", path.is_file())
        if path.is_file():
            check(f"{label} hash", sha256(path) == expected)

    direction_verdicts = [certify_direction(slug) for slug in DIRECTIONS]
    aggregate_path = (RESULTS
                      / "ftd_0762_moving_dressing_observer_forensics_v1.json")
    check("aggregate exists", aggregate_path.is_file())
    if aggregate_path.is_file():
        check("aggregate hash", sha256(aggregate_path) ==
              ARTIFACT_HASHES[aggregate_path.name])
        aggregate = load(aggregate_path)
        check("aggregate id", aggregate.get("ftd_id") == "FTD-0762")
        check("aggregate protocol",
              aggregate.get("protocol_sha256") == PROTOCOL_HASH)
        check("aggregate verdict", aggregate.get("verdict") ==
              "OBSERVER_INTEGER_CENTER_CHART_OBSTRUCTION")
        check("aggregate artifacts", aggregate.get("all_artifacts_present")
              is True)
        check("aggregate chart", aggregate.get("all_rays_chart_obstruction")
              is True)
        check("aggregate physical mismatch absent",
              aggregate.get("all_rays_physical_dressing_mismatch") is False)
        check("aggregate production frozen",
              aggregate.get("production_changed") is False)
        check("aggregate dynamics frozen",
              aggregate.get("dynamics_changed") is False)
    check("independent verdict reconstruction", all(direction_verdicts))

    print(f"FTD-0762 artifact certificate: {checks - len(failures)}/{checks} checks")
    print("verdict=OBSERVER_INTEGER_CENTER_CHART_OBSTRUCTION")
    for failure in failures:
        print(f"FAIL: {failure}")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
