"""FTD-0767 post-hoc certificate for dynamic response and wake clearing.

The script reads only the hash-locked FTD-0766 artifact and the frozen
observer source.  It performs no parameter search and does not rerun the
engine.  Its purpose is to distinguish a local, velocity-aligned field
deformation from a spatially detached wake.
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
MORPHOLOGY_SOURCE = (
    ROOT / "engine" / "tests"
    / "campaign_transported_chart_matter_morphology_cuda.cpp"
)
WAKE_SOURCE = (
    ROOT / "engine" / "tests" / "campaign_aged_wake_entrainment_cuda.cpp"
)

SUPPORT_HALF_WIDTH = 4.0
NEAR_RADIUS = 8.0
SUPPORT_DISJOINT_DISTANCE = 2.0 * SUPPORT_HALF_WIDTH
NEAR_DISJOINT_DISTANCE = 2.0 * NEAR_RADIUS
MAGNITUDES = (0.0075, 0.015, 0.030)

checks = 0
failures: list[str] = []


def check(label: str, condition: bool) -> None:
    global checks
    checks += 1
    if not condition:
        failures.append(label)


def close(left: float, right: float, tolerance: float = 5e-12) -> bool:
    return abs(left - right) <= tolerance * max(1.0, abs(left), abs(right))


def dot(left: list[float], right: list[float]) -> float:
    return sum(float(a) * float(b) for a, b in zip(left, right))


def displacement(arm: dict[str, object]) -> float:
    first = arm["checkpoints"][0]["core_center"]
    last = arm["checkpoints"][-1]["core_center"]
    delta = [float(b) - float(a) for a, b in zip(first, last)]
    return dot(delta, arm["aligned_direction"])


def union(checkpoint: dict[str, object]) -> dict[str, float]:
    return {
        key: float(checkpoint["union"][key])
        for key in ("trailing", "neutral", "leading")
    }


def directed_energy(value: dict[str, float]) -> float:
    return value["trailing"] - value["leading"]


check("artifact exists", ARTIFACT.is_file())
check(
    "artifact hash",
    hashlib.sha256(ARTIFACT.read_bytes()).hexdigest().upper()
    == ARTIFACT_SHA256,
)
source = MORPHOLOGY_SOURCE.read_text(encoding="utf-8")
wake_source = WAKE_SOURCE.read_text(encoding="utf-8")
check("support half-width frozen", "options.support_half_width=4;" in source)
check("near radius frozen", "options.near_radius=8;" in source)
check("64-tick horizon frozen", "kFtd0766Ticks=64;" in wake_source)

value = json.loads(ARTIFACT.read_text(encoding="utf-8"))
check("source identifier", value["ftd_id"] == "FTD-0766")

valid_displacements: list[float] = []
rows: list[tuple[int, float, float, float, float]] = []

for age in value["ages"]:
    age_number = int(age["age"])
    arms = {float(arm["boost"]): arm for arm in age["arms"]}
    rest = arms[0.0]
    check(f"age {age_number} rest valid", bool(rest["valid"]))

    for boost, arm in arms.items():
        if boost == 0.0 or not arm["valid"]:
            continue
        travel = displacement(arm)
        valid_displacements.append(travel)
        check(f"age {age_number} q={boost} moves forward", travel > 0.0)
        check(f"age {age_number} q={boost} less than one site", travel < 1.0)
        check(
            f"age {age_number} q={boost} bound supports overlap",
            travel < SUPPORT_DISJOINT_DISTANCE,
        )
        check(
            f"age {age_number} q={boost} near windows overlap",
            travel < NEAR_DISJOINT_DISTANCE,
        )

    pair_records = {float(pair["magnitude"]): pair for pair in age["pairs"]}
    for magnitude in MAGNITUDES:
        plus = arms[magnitude]
        minus = arms[-magnitude]
        if not plus["valid"] or not minus["valid"]:
            check(
                f"age {age_number} a={magnitude} unavailable is age-zero high boost",
                age_number == 0 and magnitude == 0.030,
            )
            continue

        # The rest arm is stored in +d alignment.  In the -d chart its
        # trailing and leading halves exchange.  Subtract each arm's matched
        # rest contribution before averaging the signed response.
        rest_first = union(rest["checkpoints"][0])
        rest_last = union(rest["checkpoints"][-1])
        plus_first = union(plus["checkpoints"][0])
        plus_last = union(plus["checkpoints"][-1])
        minus_first = union(minus["checkpoints"][0])
        minus_last = union(minus["checkpoints"][-1])

        plus_initial = directed_energy(plus_first) - directed_energy(rest_first)
        minus_initial = directed_energy(minus_first) + directed_energy(rest_first)
        check(
            f"age {age_number} a={magnitude} plus initial response zero",
            close(plus_initial, 0.0),
        )
        check(
            f"age {age_number} a={magnitude} minus initial response zero",
            close(minus_initial, 0.0),
        )

        plus_dynamic = directed_energy(plus_last) - directed_energy(rest_last)
        minus_dynamic = directed_energy(minus_last) + directed_energy(rest_last)
        pair_dynamic = 0.5 * (plus_dynamic + minus_dynamic)
        pair_directed_scale = 0.5 * (
            plus_last["trailing"] + plus_last["leading"]
            + minus_last["trailing"] + minus_last["leading"]
        )
        local_dynamic_asymmetry = pair_dynamic / pair_directed_scale
        pair_travel = 0.5 * (displacement(plus) + displacement(minus))
        recorded = float(pair_records[magnitude]["final_pair_asymmetry"])

        check(
            f"age {age_number} a={magnitude} local response positive",
            pair_dynamic > 0.0,
        )
        check(
            f"age {age_number} a={magnitude} corrected asymmetry reproduces record",
            close(local_dynamic_asymmetry, recorded),
        )
        rows.append(
            (
                age_number,
                magnitude,
                pair_travel,
                pair_dynamic,
                local_dynamic_asymmetry,
            )
        )

check("sixteen valid moving arms", len(valid_displacements) == 16)
maximum_travel = max(valid_displacements)
minimum_travel = min(valid_displacements)
check("minimum travel locked", close(minimum_travel, 0.19082199688148194))
check("maximum travel locked", close(maximum_travel, 0.875652302091055))
check("no bound-support clearing", maximum_travel < SUPPORT_DISJOINT_DISTANCE)
check("no near-field clearing", maximum_travel < NEAR_DISJOINT_DISTANCE)

for age_number in (64, 128):
    age_rows = [row for row in rows if row[0] == age_number]
    responses = [row[3] for row in age_rows]
    check(
        f"age {age_number} dynamic response reverses amplitude ordering",
        responses[0] > responses[1] > responses[2] > 0.0,
    )

print(
    f"FTD-0767 dynamic-response/clearing certificate: "
    f"{checks - len(failures)}/{checks} checks"
)
for age_number, magnitude, travel, response, asymmetry in rows:
    print(
        f"age={age_number} a={magnitude:.4g} travel={travel:.17g} "
        f"deltaW={response:.17g} D_dynamic={asymmetry:.17g}"
    )
print(f"minimum_travel={minimum_travel:.17g}")
print(f"maximum_travel={maximum_travel:.17g}")
print(
    "support_clearing_fraction="
    f"{maximum_travel / SUPPORT_DISJOINT_DISTANCE:.17g}"
)
print(
    "near_clearing_fraction="
    f"{maximum_travel / NEAR_DISJOINT_DISTANCE:.17g}"
)
if failures:
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)
print("local_velocity_aligned_deformation=OBSERVED_DESCRIPTIVELY")
print("spatially_detached_wake=NOT_TESTED")
