"""Independent FTD-0620 phase-return certificate."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations"
    / "PREREG_BALANCED_GAIT_PHASE_RETURN_v1.md"
)
RESULT = (
    ROOT
    / "engine/results/ftd_0620"
    / "ftd_0620_balanced_gait_phase_return_v1.json"
)
TICKS = (
    ROOT
    / "engine/results/ftd_0620"
    / "ftd_0620_balanced_gait_phase_ticks_v1.csv"
)
RETURNS = (
    ROOT
    / "engine/results/ftd_0620"
    / "ftd_0620_balanced_gait_phase_returns_v1.csv"
)

PROTOCOL_SHA = "A5B97A9251C46736065A1DD4A0ECA0CCDC28ED0CB7B9EF8FE74ACC494CB8B78C"
PARENT_0618_SHA = "5F04E64DFD7CBFD10CE3AC779361C4124654C817320DFC81E6D5A482889F54D3"
PARENT_0619_SHA = "0FEE2158E3DCB5EED2F837D74E89127F4B01160335057115F095FDF3C724669D"
RESULT_SHA = "0D66A13CB212BC32DEC7FC3D3C8DC90949954913F6CE5ACCBCB7910C6F7608DF"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def close(a: float, b: float, tol: float = 1e-12) -> bool:
    return abs(a - b) <= tol


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with TICKS.open(newline="", encoding="utf-8") as stream:
        tick_rows = list(csv.DictReader(stream))
    with RETURNS.open(newline="", encoding="utf-8") as stream:
        return_rows = list(csv.DictReader(stream))
    by_sign: dict[int, list[dict[str, str]]] = {}
    for row in tick_rows:
        by_sign.setdefault(int(row["sign"]), []).append(row)
    arms = {int(arm["sign"]): arm for arm in record["arms"]}

    checks: list[tuple[str, bool]] = []

    def check(name: str, condition: bool) -> None:
        checks.append((name, bool(condition)))

    check("protocol_hash", sha256(PROTOCOL) == PROTOCOL_SHA)
    check("record_protocol", record["protocol_sha256"] == PROTOCOL_SHA)
    check("parent_0618", record["parent_0618_sha256"] == PARENT_0618_SHA)
    check("parent_0619", record["parent_0619_sha256"] == PARENT_0619_SHA)
    check("result_hash", sha256(RESULT) == RESULT_SHA)
    check("production_unchanged", record["production_changed"] is False)
    check("arm_coverage", set(arms) == {-1, 0, 1})
    check("tick_coverage", {s: len(rows) for s, rows in by_sign.items()}
          == {-1: 513, 0: 129, 1: 513})
    check("return_file_empty", len(return_rows) == 0)
    check("algebraic", bool(record["algebraic_pass"])
          and all(bool(arm["algebraic_pass"]) for arm in arms.values()))
    check("forward_reverse_counts",
          arms[0]["forward_ticks"] == arms[0]["reverse_ticks"] == 128
          and all(arms[s]["forward_ticks"] == arms[s]["reverse_ticks"] == 512
                  for s in (-1, 1)))
    check("common_gate", max(float(arm["worst_common_gate"])
                             for arm in arms.values()) <= 1e-12)
    check("energy_gate", max(float(arm["maximum_energy_drift"])
                             for arm in arms.values()) <= 1e-10)
    check("inverse_gate", max(float(arm["reverse_recovery"])
                              for arm in arms.values()) <= 1e-8)
    check("geometry_gate", all(float(arm["minimum_internal_distance"]) >= 0.5
                               and float(arm["maximum_internal_distance"]) <= 2.0
                               and int(arm["maximum_anchor_multiplicity"]) <= 2
                               for arm in arms.values()))
    check("rest_gate", bool(record["rest_pass"])
          and math.sqrt(sum(float(by_sign[0][-1][k]) ** 2
                            for k in ("dx", "dy", "dz"))) <= 1e-8)

    mirror = 0.0
    for plus, minus in zip(by_sign[1], by_sign[-1], strict=True):
        displacement = math.sqrt(sum(
            (float(plus[k]) + float(minus[k])) ** 2
            for k in ("dx", "dy", "dz")
        ))
        mirror = max(mirror, displacement,
                     abs(float(plus["position_return"])
                         - float(minus["position_return"])),
                     abs(float(plus["momentum_return"])
                         - float(minus["momentum_return"])))
    check("sign_mirror_identity",
          close(mirror, float(record["maximum_sign_mirror_residual"]), 1e-15))
    check("sign_mirror_gate", mirror <= 1e-8)

    for sign in (-1, 1):
        rows = by_sign[sign]
        amplitude = float(arms[sign]["amplitude"])
        threshold = amplitude / 20.0
        candidates = []
        minimum = (math.inf, -1)
        last_tick = -1000
        for i in range(1, len(rows) - 1):
            tick = int(rows[i]["tick"])
            if tick < 32:
                continue
            distance = float(rows[i]["phase_distance"])
            if distance < minimum[0]:
                minimum = (distance, tick)
            local = (distance <= float(rows[i - 1]["phase_distance"])
                     and distance < float(rows[i + 1]["phase_distance"]))
            if (local and float(rows[i]["position_return"]) <= threshold
                    and float(rows[i]["momentum_return"]) <= threshold
                    and tick - last_tick >= 4):
                candidates.append(tick)
                last_tick = tick
        check(f"sign_{sign}_return_reconstruction", len(candidates) == 0
              and int(arms[sign]["return_count"]) == 0
              and close(minimum[0],
                        float(arms[sign]["minimum_phase_distance_after_32"]),
                        1e-12)
              and minimum[1] == int(arms[sign]["minimum_phase_tick"]))

        windows = [abs(float(rows[end]["dz"])
                       - float(rows[end - 128]["dz"]))
                   for end in (128, 256, 384, 512)]
        check(f"sign_{sign}_window_reconstruction",
              close(min(windows),
                    float(arms[sign]["minimum_window_axial_displacement"]),
                    1e-12)
              and close(windows[-1],
                        float(arms[sign]["final_window_axial_displacement"]),
                        1e-12)
              and min(windows) < 0.5 and windows[-1] >= 0.5)

        ratio = (float(arms[sign]["final_internal_momentum_norm"])
                 / float(arms[sign]["initial_internal_momentum_norm"]))
        check(f"sign_{sign}_not_relaxed", ratio > 0.1
              and not bool(arms[sign]["one_time_relaxation"]))

        angles = [float(row["phase_angle"]) for row in rows]
        check(f"sign_{sign}_phase_unwrapped",
              max(abs(b - a) for a, b in zip(angles, angles[1:]))
              <= math.pi + 1e-12)

    check("no_recurrence", all(not bool(arms[s]["recurrent"])
                               for s in (-1, 1)))
    check("intermittent_not_persistent", all(not bool(arms[s]["persistent"])
                                             for s in (-1, 1)))
    check("verdict", record["verdict"] == "BALANCED_GAIT_PHASE_BEHAVIOR_MIXED")

    passed = sum(ok for _, ok in checks)
    for name, ok in checks:
        print(f"{'PASS' if ok else 'FAIL'} {name}")
    print(f"{passed}/{len(checks)} checks passed")
    print(json.dumps({
        "ftd_id": "FTD-0620",
        "protocol_sha256": PROTOCOL_SHA,
        "result_sha256": RESULT_SHA,
        "maximum_sign_mirror_residual": mirror,
        "plus_final_displacement": float(by_sign[1][-1]["dz"]),
        "plus_minimum_phase_distance": arms[1]["minimum_phase_distance_after_32"],
        "plus_internal_momentum_fraction": (
            float(arms[1]["final_internal_momentum_norm"])
            / float(arms[1]["initial_internal_momentum_norm"])
        ),
        "passed": passed,
        "total": len(checks),
    }, indent=2))
    return 0 if passed == len(checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())

