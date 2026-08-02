"""Independent post-hoc entrainment certificate for FTD-0765.

This performs exact observer algebra on the locked FTD-0764 artifacts.  It is
not a parameter search and does not rerun or modify the engine.
"""

from __future__ import annotations

import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0764"
ARTIFACTS = {
    "face": (
        "ftd_0764_transported_chart_morphology_v1_face.json",
        "0DDB53E3A138AF564EB3FF09F6D052D0120E9A3D74D5B132850D86990FB1017C",
        (0.0, 0.0, 1.0),
    ),
    "edge": (
        "ftd_0764_transported_chart_morphology_v1_edge.json",
        "762E5C50315C2564070CDBA0009635673EA4519F5810692418996F46A38C7AA4",
        (0.0, 1.0 / math.sqrt(2.0), -1.0 / math.sqrt(2.0)),
    ),
    "body": (
        "ftd_0764_transported_chart_morphology_v1_body.json",
        "43E2182CCE16BDC356A6FC2BB9A275343F9633571DA45AEC2C7CEA76E4412DF5",
        (1.0 / math.sqrt(3.0),) * 3,
    ),
}
EXPECTED = {
    "face": (0.4042465806222424, -0.004309856472843876,
             -0.010661454368296368, 0.40855643709508627),
    "edge": (0.40005326208963665, 0.06263385192047236,
             0.15656378251563538, 0.3374194101691643),
    "body": (0.41567332180485217, 0.07024336925869527,
             0.1689869557990847, 0.3454299525461569),
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


def dot(left: list[float] | tuple[float, ...],
        right: list[float] | tuple[float, ...]) -> float:
    return sum(float(a) * float(b) for a, b in zip(left, right))


def add(left: list[float], right: list[float]) -> list[float]:
    return [float(a) + float(b) for a, b in zip(left, right)]


def subtract(left: list[float], right: list[float]) -> list[float]:
    return [float(a) - float(b) for a, b in zip(left, right)]


def residual_centroid(checkpoint: dict[str, object]) -> list[float]:
    morphology = checkpoint["morphology"]
    near = float(morphology["near_residual_energy"])
    outer = float(morphology["outer_residual_energy"])
    total = near + outer
    relative = [
        (near * float(morphology["near_first_moment"][axis])
         + outer * float(morphology["outer_first_moment"][axis])) / total
        for axis in range(3)
    ]
    return add([float(value) for value in morphology["center"]], relative)


rows: dict[str, dict[str, float]] = {}
for slug, (name, expected_hash, direction) in ARTIFACTS.items():
    path = RESULTS / name
    check(f"{slug} artifact exists", path.is_file())
    check(f"{slug} artifact hash", sha256(path) == expected_hash)
    value = json.loads(path.read_text(encoding="utf-8"))
    check(f"{slug} FTD-0764 source", value.get("ftd_id") == "FTD-0764")
    check(f"{slug} execution", value.get("execution_valid") is True)

    rest = value["rest"]["checkpoints"]
    moved = value["plus"]["checkpoints"]
    check(f"{slug} tick grid", [item["tick"] for item in moved]
          == [160, 176, 192, 208, 224])
    check(f"{slug} rest tick grid", [item["tick"] for item in rest]
          == [160, 176, 192, 208, 224])

    moved_residual0 = residual_centroid(moved[0])
    rest_residual0 = residual_centroid(rest[0])
    core0 = [float(item) for item in moved[0]["morphology"]["center"]]
    last_row: dict[str, float] = {}

    for index in range(1, len(moved)):
        tick = int(moved[index]["tick"])
        moved_residual = residual_centroid(moved[index])
        rest_residual = residual_centroid(rest[index])
        core = [float(item) for item in moved[index]["morphology"]["center"]]
        core_displacement = dot(subtract(core, core0), direction)
        residual_displacement = dot(subtract(
            subtract(moved_residual, moved_residual0),
            subtract(rest_residual, rest_residual0)), direction)
        entrainment = residual_displacement / core_displacement
        lag = core_displacement - residual_displacement

        moved_moment = float(moved[index]["longitudinal_combined_moment"])
        moved_moment0 = float(moved[0]["longitudinal_combined_moment"])
        rest_moment = float(rest[index]["longitudinal_combined_moment"])
        rest_moment0 = float(rest[0]["longitudinal_combined_moment"])
        moment_lag = -(moved_moment - moved_moment0) + (rest_moment - rest_moment0)

        check(f"{slug} tick {tick} positive core displacement",
              core_displacement > 0.0)
        check(f"{slug} tick {tick} lag identity",
              abs(lag - moment_lag) <= 5e-13)
        check(f"{slug} tick {tick} finite entrainment",
              math.isfinite(entrainment))
        check(f"{slug} tick {tick} residual under-entrained",
              entrainment < 0.20)
        last_row = {
            "core_displacement": core_displacement,
            "residual_displacement": residual_displacement,
            "entrainment": entrainment,
            "lag": lag,
        }

    expected = EXPECTED[slug]
    for key, target in zip(("core_displacement", "residual_displacement",
                            "entrainment", "lag"), expected):
        check(f"{slug} final {key}", abs(last_row[key] - target) <= 1e-14)
    rows[slug] = last_row

check("finite-scale orientation dependence",
      max(row["entrainment"] for row in rows.values())
      - min(row["entrainment"] for row in rows.values()) > 0.15)
check("no ray reaches majority entrainment",
      all(row["entrainment"] < 0.5 for row in rows.values()))

print(f"FTD-0765 entrainment certificate: {checks - len(failures)}/{checks} checks")
for slug, row in rows.items():
    print(
        f"{slug}: core={row['core_displacement']:.17g} "
        f"residual={row['residual_displacement']:.17g} "
        f"entrainment={row['entrainment']:.17g} lag={row['lag']:.17g}"
    )
if failures:
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)
print("rigid_residual_entrainment=false")
print("independent_wake_creation_established=false")
print("finite_scale_orientation_dependence=true")
