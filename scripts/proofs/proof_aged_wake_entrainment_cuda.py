"""Independent FTD-0766 artifact certificate.

This reconstructs the locked labels and descriptive metrics from the completed
CUDA artifact.  It performs no parameter search and does not rerun the engine.
"""

from __future__ import annotations

import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ARTIFACT = (
    ROOT / "engine" / "results" / "ftd_0766"
    / "ftd_0766_aged_wake_entrainment_v1.json"
)
ARTIFACT_SHA256 = (
    "116F47B0CD0092F1F814B151084C436E719DA2F278E957CD5083AF44AF38C090"
)
PROTOCOL_SHA256 = (
    "B8FF05668DF306D05B6D3F7F4715C38B6C3A78C9205E9C747146C2F3A95AFA7F"
)
AGES = [0, 64, 128]
BOOSTS = [0.0, -0.030, -0.015, -0.0075, 0.0075, 0.015, 0.030]
TIMES = [0, 16, 32, 48, 64]
MAGNITUDES = [0.0075, 0.015, 0.030]

checks = 0
failures: list[str] = []


def check(label: str, condition: bool) -> None:
    global checks
    checks += 1
    if not condition:
        failures.append(label)


def close(left: float, right: float, tolerance: float = 5e-13) -> bool:
    return abs(left - right) <= tolerance * max(1.0, abs(left), abs(right))


def vec_sub(left: list[float], right: list[float]) -> list[float]:
    return [float(a) - float(b) for a, b in zip(left, right)]


def vec_add(left: list[float], right: list[float]) -> list[float]:
    return [float(a) + float(b) for a, b in zip(left, right)]


def dot(left: list[float], right: list[float]) -> float:
    return sum(float(a) * float(b) for a, b in zip(left, right))


def norm(value: list[float]) -> float:
    return math.sqrt(dot(value, value))


def partition(checkpoint: dict[str, object]) -> dict[str, float]:
    return {key: float(checkpoint["union"][key])
            for key in ("trailing", "neutral", "leading")}


def asymmetry(value: dict[str, float]) -> float:
    directed = value["trailing"] + value["leading"]
    return (value["trailing"] - value["leading"]) / directed


def normalized_difference(left: float, right: float) -> float:
    return abs(left - right) / max(1e-300, abs(left), abs(right))


check("artifact exists", ARTIFACT.is_file())
check("artifact hash", hashlib.sha256(ARTIFACT.read_bytes()).hexdigest().upper()
      == ARTIFACT_SHA256)
value = json.loads(ARTIFACT.read_text(encoding="utf-8"))
check("identifier", value["ftd_id"] == "FTD-0766")
check("protocol hash", value["protocol_sha256"] == PROTOCOL_SHA256)
check("volume", value["volume"] == 321)
check("age grid", [age["age"] for age in value["ages"]] == AGES)

reconstructed_pairs: dict[int, dict[float, tuple[float, float]]] = {}
for age in value["ages"]:
    age_number = int(age["age"])
    check(f"age {age_number} aging valid", age["aging_valid"] is True)
    arms = {float(arm["boost"]): arm for arm in age["arms"]}
    check(f"age {age_number} boost grid", list(arms) == BOOSTS)

    for boost, arm in arms.items():
        expected_valid = not (age_number == 0 and abs(boost) == 0.030)
        check(f"age {age_number} q={boost} validity",
              arm["valid"] is expected_valid)
        if not expected_valid:
            check(f"age {age_number} q={boost} initialization rejection",
                  arm["initialized"] is False
                  and arm["executed"] is False
                  and arm["checkpoints"] == [])
            continue
        check(f"age {age_number} q={boost} initialized/executed",
              arm["initialized"] is True and arm["executed"] is True)
        check(f"age {age_number} q={boost} tick grid",
              [item["tau"] for item in arm["checkpoints"]] == TIMES)
        check(f"age {age_number} q={boost} checkpoint validity",
              all(item["valid"] for item in arm["checkpoints"]))
        check(f"age {age_number} q={boost} common gate",
              float(arm["maximum_common_residual"]) <= 1e-12)
        check(f"age {age_number} q={boost} energy gate",
              float(arm["maximum_energy_residual"]) <= 1e-12)
        check(f"age {age_number} q={boost} causal gate",
              float(arm["maximum_speed_excess"]) <= 1e-12)
        for checkpoint in arm["checkpoints"]:
            observed = asymmetry(partition(checkpoint))
            check(f"age {age_number} q={boost} tau={checkpoint['tau']} D",
                  close(observed, float(checkpoint["union"]["asymmetry"])))
            check(f"age {age_number} q={boost} tau={checkpoint['tau']} observer",
                  checkpoint["fractional_observer_valid"] is True
                  and checkpoint["boundary_ledger_valid"] is True
                  and checkpoint["ladder_valid"] is True
                  and float(checkpoint["morphology_reconstruction_residual"])
                      <= 1e-12
                  and float(checkpoint["partition_residual"]) <= 1e-12)

    pair_rows = {float(pair["magnitude"]): pair for pair in age["pairs"]}
    check(f"age {age_number} magnitude grid",
          list(pair_rows) == MAGNITUDES)
    reconstructed_pairs[age_number] = {}
    rest = arms[0.0]
    for magnitude, pair in pair_rows.items():
        plus = arms[magnitude]
        minus = arms[-magnitude]
        if not plus["valid"] or not minus["valid"]:
            check(f"age {age_number} a={magnitude} unavailable pair",
                  pair["valid"] is False
                  and pair["final_pair_asymmetry"] is None
                  and pair["final_pair_entrainment"] is None)
            continue

        core_mirror = 0.0
        field_mirror = 0.0
        common_center = plus["checkpoints"][0]["core_center"]
        for plus_cp, minus_cp in zip(plus["checkpoints"],
                                     minus["checkpoints"]):
            plus_displacement = vec_sub(plus_cp["core_center"], common_center)
            minus_displacement = vec_sub(minus_cp["core_center"], common_center)
            core_mirror = max(core_mirror,
                              norm(vec_add(plus_displacement,
                                           minus_displacement)))
            plus_partition = partition(plus_cp)
            minus_partition = partition(minus_cp)
            plus_energy = sum(plus_partition.values())
            minus_energy = sum(minus_partition.values())
            field_mirror = max(
                field_mirror,
                normalized_difference(plus_partition["trailing"],
                                      minus_partition["trailing"]),
                normalized_difference(plus_partition["leading"],
                                      minus_partition["leading"]),
                normalized_difference(plus_energy, minus_energy),
            )

        plus0, plus1 = plus["checkpoints"][0], plus["checkpoints"][-1]
        minus0, minus1 = minus["checkpoints"][0], minus["checkpoints"][-1]
        rest0, rest1 = rest["checkpoints"][0], rest["checkpoints"][-1]
        plus_direction = [float(item) for item in plus["aligned_direction"]]
        minus_direction = [float(item) for item in minus["aligned_direction"]]
        plus_core = dot(vec_sub(plus1["core_center"], plus0["core_center"]),
                        plus_direction)
        minus_core = dot(vec_sub(minus1["core_center"], minus0["core_center"]),
                         minus_direction)
        rest_residual_motion = vec_sub(rest1["residual_centroid"],
                                       rest0["residual_centroid"])
        plus_residual = dot(vec_sub(
            vec_sub(plus1["residual_centroid"], plus0["residual_centroid"]),
            rest_residual_motion), plus_direction)
        minus_residual = dot(vec_sub(
            vec_sub(minus1["residual_centroid"], minus0["residual_centroid"]),
            rest_residual_motion), minus_direction)
        pair_asymmetry = 0.5 * (
            asymmetry(partition(plus1)) + asymmetry(partition(minus1)))
        pair_entrainment = 0.5 * (plus_residual / plus_core
                                  + minus_residual / minus_core)

        check(f"age {age_number} a={magnitude} core mirror",
              close(core_mirror, float(pair["maximum_core_mirror_residual"])))
        check(f"age {age_number} a={magnitude} field mirror",
              close(field_mirror, float(pair["maximum_field_mirror_residual"])))
        check(f"age {age_number} a={magnitude} asymmetry",
              close(pair_asymmetry, float(pair["final_pair_asymmetry"])))
        check(f"age {age_number} a={magnitude} entrainment",
              close(pair_entrainment, float(pair["final_pair_entrainment"])))
        check(f"age {age_number} a={magnitude} core symmetry passes",
              core_mirror <= 1e-10)
        check(f"age {age_number} a={magnitude} field symmetry fails",
              field_mirror > 1e-10 and pair["valid"] is False)
        reconstructed_pairs[age_number][magnitude] = (
            pair_asymmetry, pair_entrainment)

check("registered execution invalid", value["execution_valid"] is False)
check("official wake invalid", value["wake_verdict"]
      == "AGED_WAKE_EXECUTION_INVALID")
check("official entrainment invalid", value["entrainment_verdict"]
      == "ENTRAINMENT_EXECUTION_INVALID")
check("no component labels promoted",
      value["aligned_trailing_excess"] is False
      and value["amplitude_ordered"] is False
      and value["age_stable"] is False)

# Descriptive post-hoc facts; these do not repair the invalid execution.
for age_number in (64, 128):
    row = reconstructed_pairs[age_number]
    check(f"age {age_number} positive final asymmetry",
          all(row[magnitude][0] > 1e-5 for magnitude in MAGNITUDES))
    check(f"age {age_number} reverse amplitude ordering",
          row[0.0075][0] > row[0.015][0] > row[0.030][0] > 0.0)
    check(f"age {age_number} under-entrainment",
          all(abs(row[magnitude][1]) < 0.05 for magnitude in MAGNITUDES))

for magnitude in MAGNITUDES:
    age64 = reconstructed_pairs[64][magnitude][0]
    age128 = reconstructed_pairs[128][magnitude][0]
    relative_change = abs(age128 - age64) / max(abs(age128), abs(age64))
    check(f"a={magnitude} age-stability failure", relative_change > 0.25)

print(f"FTD-0766 aged-wake certificate: {checks-len(failures)}/{checks} checks")
for age_number in AGES:
    for magnitude, (wake, entrainment) in reconstructed_pairs[age_number].items():
        print(f"age={age_number} a={magnitude:.4g} D={wake:.17g} "
              f"entrainment={entrainment:.17g}")
if failures:
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)
print("registered_execution=AGED_WAKE_EXECUTION_INVALID")
print("wake_creation=NOT_ESTABLISHED")
print("aged_mobility_basin=OBSERVED_DESCRIPTIVELY")
print("field_pair_symmetry=FAILED")
print("amplitude_ordering=REVERSED")
print("residual_entrainment=UNDER_5_PERCENT")

