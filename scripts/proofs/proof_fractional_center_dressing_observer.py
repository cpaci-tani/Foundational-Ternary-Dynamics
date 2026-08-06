"""Independent frozen-artifact certificate for FTD-0763."""

from __future__ import annotations

import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0763"
PROTOCOL = "FB78C2688A90E18D01071DA390BFE230FFD76CF340FD2CB56AD6D545CDD8C63A"
EXPECTED_HASHES = {
    "ftd_0763_fractional_center_dressing_observer_v1.json":
        "58F9C85E7DAAFA4CFC738CB22CD1E64EA5509403A8D7FC9229928DE65B112BEA",
    "ftd_0763_fractional_center_dressing_observer_v1_face.json":
        "E66D8C01C6AAA73AB89EBFB2D6741F773A7DA7FC47D19345CE7C0588C316D9A8",
    "ftd_0763_fractional_center_dressing_observer_v1_edge.json":
        "83D38D419A9AB81BBE1200C7172DAE99DD9C137529B867C72BC07BEA08D4F44B",
    "ftd_0763_fractional_center_dressing_observer_v1_body.json":
        "3D446549387D98506FB1592A343E95E69F106671E60449A6DFFA8E04C19F79A5",
}
EXPECTED_FRACTIONAL_NORMS = {
    "face": 0.40424658062249819,
    "edge": 0.40005326208963671,
    "body": 0.41567332180486855,
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


def vec_norm(values: list[float]) -> float:
    return math.sqrt(sum(float(value) ** 2 for value in values))


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


for name, expected in EXPECTED_HASHES.items():
    path = RESULTS / name
    check(f"artifact exists: {name}", path.is_file())
    if path.is_file():
        check(f"artifact hash: {name}", sha256(path) == expected)

aggregate_path = RESULTS / "ftd_0763_fractional_center_dressing_observer_v1.json"
aggregate = json.loads(aggregate_path.read_text(encoding="utf-8"))
check("aggregate id", aggregate.get("ftd_id") == "FTD-0763")
check("aggregate protocol", aggregate.get("protocol_sha256") == PROTOCOL)
check("aggregate verdict",
      aggregate.get("verdict") == "FRACTIONAL_CENTER_OBSERVER_CONSTRUCTED")
check("aggregate complete", aggregate.get("all_artifacts_present") is True)
check("aggregate rays", aggregate.get("all_rays_pass") is True)
check("aggregate production frozen", aggregate.get("production_changed") is False)
check("aggregate dynamics frozen", aggregate.get("dynamics_changed") is False)
check("aggregate no co-motion claim",
      aggregate.get("co_moving_dressing_claimed") is False)

for slug in ("face", "edge", "body"):
    path = RESULTS / f"ftd_0763_fractional_center_dressing_observer_v1_{slug}.json"
    value = json.loads(path.read_text(encoding="utf-8"))
    check(f"{slug} finite tree", finite_tree(value))
    check(f"{slug} id", value.get("ftd_id") == "FTD-0763")
    check(f"{slug} protocol", value.get("protocol_sha256") == PROTOCOL)
    check(f"{slug} registered volume", value.get("volume") == 321)
    check(f"{slug} formation tick", value.get("formation_tick") == 160)
    check(f"{slug} transport ticks", value.get("transport_ticks") == 64)
    check(f"{slug} boost", float(value.get("boost", math.inf)) == 0.015)
    check(f"{slug} parent", value.get("parent_valid") is True)
    check(f"{slug} replay", value.get("replay_executed") is True)
    check(f"{slug} common action", value.get("common_action") is True)
    check(f"{slug} pass", value.get("pass") is True)
    check(f"{slug} production frozen", value.get("production_changed") is False)
    check(f"{slug} dynamics frozen", value.get("dynamics_changed") is False)
    check(f"{slug} no co-motion claim",
          value.get("co_moving_dressing_claimed") is False)
    check(f"{slug} nonzero recorded momentum defect",
          vec_norm(value["matter_momentum_defect"]) > 1e-6)
    check(f"{slug} nonzero displacement",
          vec_norm(value["center_displacement"]) > 0.39)

    for tick in ("tick_160", "tick_224"):
        checkpoint = value[tick]
        check(f"{slug} {tick} valid", checkpoint.get("valid") is True)
        check(f"{slug} {tick} observer", checkpoint.get("observer_valid") is True)
        check(f"{slug} {tick} boundary ledger",
              checkpoint.get("boundary_ledger_valid") is True)
        check(f"{slug} {tick} ladder", checkpoint.get("ladder_valid") is True)
        check(f"{slug} {tick} scalar only",
              checkpoint.get("cuda_scalar_only") is True)
        check(f"{slug} {tick} observer error",
              checkpoint.get("observer_error") == "")
        check(f"{slug} {tick} ladder error",
              checkpoint.get("ladder_error") == "")
        center = [float(x) for x in checkpoint["center"]]
        support = [float(x) for x in checkpoint["support_center"]]
        offset = [float(x) for x in checkpoint["fractional_center_offset"]]
        check(f"{slug} {tick} chart identity",
              vec_norm([c - s - o for c, s, o in zip(center, support, offset)])
              <= 1e-12)
        check(f"{slug} {tick} Gauss gate",
              abs(float(checkpoint["actual_gauss_residual"])) <= 1e-12)
        check(f"{slug} {tick} partition gate",
              abs(float(checkpoint["energy_partition_residual"])) <= 1e-12)
        check(f"{slug} {tick} boundary identity gate",
              abs(float(checkpoint["boundary_identity_residual"])) <= 1e-12)
        check(f"{slug} {tick} readout reconstruction gate",
              abs(float(checkpoint["readout_reconstruction_residual"])) <= 1e-12)
        check(f"{slug} {tick} characteristic gate",
              abs(float(checkpoint["characteristic_flux_residual"])) <= 1e-12)
        check(f"{slug} {tick} ladder energy gate",
              abs(float(checkpoint["ladder_energy_residual"])) <= 1e-12)
        check(f"{slug} {tick} ladder projection gate",
              abs(float(checkpoint["ladder_projection_residual"])) <= 1e-12)
        check(f"{slug} {tick} shell radii",
              [shell["radius"] for shell in checkpoint["shells"]]
              == [8, 12, 16, 24, 32, 48])
        check(f"{slug} {tick} no full-field reduction download",
              int(checkpoint["device_to_host_bytes"]) < 4 * 1024 * 1024)

    check(f"{slug} integer formation center",
          float(value["tick_160"]["fractional_center_norm"]) <= 1e-12)
    observed = float(value["tick_224"]["fractional_center_norm"])
    check(f"{slug} fractional replay center", observed > 0.39)
    check(f"{slug} FTD-0762 offset reproduction",
          abs(observed - EXPECTED_FRACTIONAL_NORMS[slug]) <= 1e-12)

print(f"FTD-0763 artifact certificate: {checks - len(failures)}/{checks} checks")
if failures:
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)
print("verdict=FRACTIONAL_CENTER_OBSERVER_CONSTRUCTED")
print("co_moving_dressing_claimed=false")
