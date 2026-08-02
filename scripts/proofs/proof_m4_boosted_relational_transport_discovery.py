"""Independent frozen-artifact certificate for FTD-0761.

``--preflight`` verifies the locked CUDA implementation and refuses an
existing FTD-0761 result directory.  The default mode consumes the complete
CSV/JSON record and independently reconstructs every registered verdict.
It never runs, repairs, or completes the dynamics.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0761"
PROTOCOL_HASH = "AD6368C6793374771703A1506FA60C06E1D11C0649227F315DD1A79A0F3BDA5C"
EXPECTED_HASHES = {
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_M4_BOOSTED_RELATIONAL_TRANSPORT_DISCOVERY_v1.md": PROTOCOL_HASH,
    "engine/tests/campaign_m4_boosted_relational_transport_discovery_cuda.cpp":
        "E13CDB23EBBE8E1127719D8E174C4DDCD6D9CAC1139EB889A5F939172CE09459",
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_M3_RELATIONAL_CHART_HELD_OUT_VALIDATION_v1.md":
        "681FA36CCE4479D268D37651E4CD58AA6C1D5A4809F989EA4FF2AA24B7B40722",
    "engine/tests/campaign_m3_support_invariant_validation_cuda.cpp":
        "F2CCACB00E0DF697B10838E3E85EC636E38BC94E2B2707A55A86811FFE80DCEA",
    "engine/include/ftd/eft/support_invariant_matter_predicate.h":
        "B11E087E2E7E16375C173185233AD001AB8B9F049E9B9B5A3156D8618CB4F104",
    "engine/src/eft/support_invariant_matter_predicate.cpp":
        "752CE7C3B03A9944C1E7016A62CCA584FAC868EF191D8241ACEE7E6C9C550D21",
    "engine/build_wsl/campaign_m4_boosted_relational_transport_discovery_cuda":
        "F682EBB62E2A0D8728EDAD48345181A902B80A30DD6787D6D0D1C5A9882A52D1",
}
DIRECTIONS = {
    "face": ((0.0, 0.0, 1.0), "0_0_1"),
    "edge": ((0.0, 1.0 / math.sqrt(2.0), -1.0 / math.sqrt(2.0)),
             "0_1_-1"),
    "body": ((1.0 / math.sqrt(3.0),) * 3, "1_1_1"),
}
ARMS = ("rest", "plus", "minus")
SIGNS = {"rest": 0, "plus": 1, "minus": -1}
TICKS = tuple(range(160, 417))
CHECKPOINTS = (160, 224, 288, 352, 416)
POST_CHECKPOINTS = CHECKPOINTS[1:]
CORE_GATE = 1.0e-6
COMMON_GATE = 1.0e-10
ENERGY_GATE = 1.0e-8
SPEED_GATE = 1.0e-12
SIGMA_GATE = 1.0e-3
CONDITION_GATE = 1.0e4
REVERSE_GATE = 1.0e-10
REST_GATE = 1.0e-9
FINAL_GATE = 1.0
TRANSVERSE_GATE = 0.10
MIRROR_GATE = 1.0e-7
MOMENTUM_STEP_GATE = 1.0e-9
MOMENTUM_CUMULATIVE_GATE = 1.0e-8


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


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def bit(row: dict[str, str], key: str) -> bool:
    return row[key] == "1"


def scalar(row: dict[str, str], key: str) -> float:
    return float(row[key])


def vector(row: dict[str, str], prefix: str) -> tuple[float, float, float]:
    return tuple(float(row[prefix + axis]) for axis in "xyz")  # type: ignore[return-value]


def sub(a: tuple[float, float, float],
        b: tuple[float, float, float]) -> tuple[float, float, float]:
    return tuple(x - y for x, y in zip(a, b))  # type: ignore[return-value]


def add(a: tuple[float, float, float],
        b: tuple[float, float, float]) -> tuple[float, float, float]:
    return tuple(x + y for x, y in zip(a, b))  # type: ignore[return-value]


def mul(a: tuple[float, float, float], value: float) -> tuple[float, float, float]:
    return tuple(value * x for x in a)  # type: ignore[return-value]


def dot(a: tuple[float, float, float], b: tuple[float, float, float]) -> float:
    return sum(x * y for x, y in zip(a, b))


def norm(a: tuple[float, float, float]) -> float:
    return math.sqrt(dot(a, a))


def max_component(a: tuple[float, float, float]) -> float:
    return max(abs(value) for value in a)


def close(a: float, b: float, tolerance: float = 5.0e-13) -> bool:
    return math.isfinite(a) and math.isfinite(b) and math.isclose(
        a, b, rel_tol=tolerance, abs_tol=tolerance)


def exact_hashes() -> None:
    check("protocol locked", PROTOCOL_HASH != "UNLOCKED")
    for relative, expected in EXPECTED_HASHES.items():
        path = ROOT / relative
        check(f"locked file exists: {relative}", path.is_file())
        check(f"locked hash present: {relative}", expected != "UNLOCKED")
        if path.is_file() and expected != "UNLOCKED":
            check(f"locked hash exact: {relative}", sha256(path) == expected)


def preflight() -> int:
    exact_hashes()
    check("registered result directory absent", not RESULTS.exists())
    source = (ROOT / "engine/tests/"
              "campaign_m4_boosted_relational_transport_discovery_cuda.cpp")
    text = source.read_text(encoding="utf-8") if source.is_file() else ""
    for token in (
        "CudaMatchedFieldPipeline", "--qualify", "--run",
        "solve_connected_moore_block_reverse", "kTransportTicks = 256",
        "kBoost = 0.015", "checkpoint_tick", "object_center",
        "momentum_cumulative_defect", r"dynamics_changed\": false",
    ):
        check(f"runner token: {token}", token in text)
    return report("FTD-0761 preflight")


def finite_fields(row: dict[str, str], fields: tuple[str, ...]) -> bool:
    try:
        return all(math.isfinite(float(row[field])) for field in fields)
    except (KeyError, ValueError):
        return False


def reconstruct_direction(slug: str) -> dict[str, object]:
    unit, direction = DIRECTIONS[slug]
    stem = f"ftd_0761_m4_boosted_transport_v1_{slug}"
    csv_path = RESULTS / f"{stem}.csv"
    json_path = RESULTS / f"{stem}.json"
    check(f"{slug} csv exists", csv_path.is_file())
    check(f"{slug} json exists", json_path.is_file())
    if not csv_path.is_file() or not json_path.is_file():
        return {"infrastructure": False, "baseline": False,
                "coherence": False, "transport": False,
                "field_balanced": False}
    try:
        metadata = json.loads(json_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        check(f"{slug} json parses", False)
        return {"infrastructure": False, "baseline": False,
                "coherence": False, "transport": False,
                "field_balanced": False}
    check(f"{slug} id", metadata.get("ftd_id") == "FTD-0761")
    check(f"{slug} protocol", metadata.get("protocol_sha256") == PROTOCOL_HASH)
    check(f"{slug} direction", metadata.get("direction") == direction)
    check(f"{slug} slug", metadata.get("slug") == slug)
    check(f"{slug} volume", metadata.get("volume") == 321)
    check(f"{slug} formation", metadata.get("formation_tick") == 160)
    check(f"{slug} horizon", metadata.get("transport_ticks") == 256)
    check(f"{slug} boost", metadata.get("boost") == 0.015)
    check(f"{slug} production frozen", metadata.get("production_changed") is False)
    check(f"{slug} dynamics frozen", metadata.get("dynamics_changed") is False)

    data = read_rows(csv_path)
    grouped = {arm: [row for row in data if row.get("arm") == arm]
               for arm in ARMS}
    check(f"{slug} total row count", len(data) == 3 * len(TICKS))
    exact_by_arm: dict[str, bool] = {}
    balanced_by_arm: dict[str, bool] = {}
    hops_by_arm: dict[str, int] = {}
    summaries = {item.get("name"): item for item in metadata.get("arms", [])
                 if isinstance(item, dict)}
    check(f"{slug} arm summaries", tuple(summaries) == ARMS)
    numeric_fields = (
        "graph_margin", "energy_margin", "pair_energy", "cx", "cy", "cz",
        "rx", "ry", "rz", "r0x", "r0y", "r0z", "p0x", "p0y", "p0z",
        "r1x", "r1y", "r1z", "p1x", "p1y", "p1z", "matter_px",
        "matter_py", "matter_pz", "field_px", "field_py", "field_pz",
        "total_px", "total_py", "total_pz", "common_residual",
        "energy_residual", "energy_drift", "speed_excess", "sigma_min",
        "condition_number", "reverse_recovery", "momentum_step_defect",
        "momentum_cumulative_defect",
    )
    for arm in ARMS:
        arm_rows = grouped[arm]
        check(f"{slug}/{arm} row count", len(arm_rows) == len(TICKS))
        check(f"{slug}/{arm} tick shape",
              tuple(int(row["tick"]) for row in arm_rows) == TICKS)
        check(f"{slug}/{arm} signs",
              all(int(row["sign"]) == SIGNS[arm] for row in arm_rows))
        check(f"{slug}/{arm} ids",
              all(row["ftd_id"] == "FTD-0761" and
                  row["protocol_sha256"] == PROTOCOL_HASH and
                  row["direction"] == direction for row in arm_rows))
        check(f"{slug}/{arm} finite rows",
              all(finite_fields(row, numeric_fields) for row in arm_rows))
        checkpoint_shape = tuple(int(row["tick"]) for row in arm_rows
                                 if bit(row, "checkpoint"))
        check(f"{slug}/{arm} checkpoint shape", checkpoint_shape == CHECKPOINTS)
        core = all(bit(row, "member") and
                   scalar(row, "graph_margin") >= CORE_GATE and
                   scalar(row, "energy_margin") >= CORE_GATE
                   for row in arm_rows)
        checkpoints = all(
            bit(row, "observer_valid") and bit(row, "ladder_valid") and
            bit(row, "regularity_measured") and
            scalar(row, "sigma_min") >= SIGMA_GATE and
            scalar(row, "condition_number") <= CONDITION_GATE
            for row in arm_rows if int(row["tick"]) in CHECKPOINTS)
        transactions = all(
            bit(row, "step_valid") and bit(row, "common") and
            scalar(row, "common_residual") <= COMMON_GATE and
            scalar(row, "energy_residual") <= ENERGY_GATE and
            scalar(row, "speed_excess") <= SPEED_GATE
            for row in arm_rows if int(row["tick"]) > 160)
        reverse = all(
            bit(row, "reverse_valid") and
            scalar(row, "reverse_recovery") <= REVERSE_GATE
            for row in arm_rows if int(row["tick"]) in POST_CHECKPOINTS)
        exact = (len(arm_rows) == len(TICKS) and core and checkpoints and
                 transactions and reverse)
        exact_by_arm[arm] = exact
        step_defect = max(scalar(row, "momentum_step_defect")
                          for row in arm_rows[1:])
        cumulative_defect = max(scalar(row, "momentum_cumulative_defect")
                                for row in arm_rows)
        balanced_by_arm[arm] = (exact and step_defect <= MOMENTUM_STEP_GATE and
                                cumulative_defect <= MOMENTUM_CUMULATIVE_GATE)
        hops_by_arm[arm] = sum(int(row["site_hops"]) for row in arm_rows[1:])
        summary = summaries.get(arm, {})
        check(f"{slug}/{arm} initialized bit", summary.get("initialized") is True)
        check(f"{slug}/{arm} executed bit", summary.get("executed") is True)
        check(f"{slug}/{arm} exact parity", summary.get("exact") is exact)
        check(f"{slug}/{arm} sign parity", summary.get("sign") == SIGNS[arm])
        check(f"{slug}/{arm} hop parity", summary.get("total_hops") == hops_by_arm[arm])
        check(f"{slug}/{arm} balance parity",
              summary.get("field_balanced") is balanced_by_arm[arm])
        check(f"{slug}/{arm} graph summary", close(
            float(summary.get("minimum_graph_margin", math.nan)),
            min(scalar(row, "graph_margin") for row in arm_rows[1:])))
        check(f"{slug}/{arm} energy summary", close(
            float(summary.get("minimum_energy_margin", math.nan)),
            min(scalar(row, "energy_margin") for row in arm_rows[1:])))
        check(f"{slug}/{arm} common summary", close(
            float(summary.get("maximum_common", math.nan)),
            max(scalar(row, "common_residual") for row in arm_rows[1:])))
        check(f"{slug}/{arm} reverse summary", close(
            float(summary.get("maximum_reverse_recovery", math.nan)),
            max(scalar(row, "reverse_recovery") for row in arm_rows
                if int(row["tick"]) in POST_CHECKPOINTS)))

    infrastructure = (metadata.get("parent_pass") is True and
                      all(len(grouped[arm]) == len(TICKS) for arm in ARMS))
    rest_displacement = norm(sub(vector(grouped["rest"][-1], "c"),
                                 vector(grouped["rest"][0], "c")))
    baseline = infrastructure and exact_by_arm["rest"] and rest_displacement <= REST_GATE
    coherence = baseline and exact_by_arm["plus"] and exact_by_arm["minus"]
    plus_values: list[float] = []
    minus_values: list[float] = []
    maximum_transverse = 0.0
    mirror_residual = 0.0
    if coherence:
        for index in range(len(TICKS)):
            rest = grouped["rest"][index]
            plus = grouped["plus"][index]
            minus = grouped["minus"][index]
            dp = sub(vector(plus, "c"), vector(rest, "c"))
            dm = sub(vector(minus, "c"), vector(rest, "c"))
            sp = dot(dp, unit)
            sm = -dot(dm, unit)
            plus_values.append(sp)
            minus_values.append(sm)
            maximum_transverse = max(
                maximum_transverse, norm(sub(dp, mul(unit, sp))),
                norm(add(dm, mul(unit, sm))))
            mirror_residual = max(
                mirror_residual, max_component(add(dp, dm)),
                abs(scalar(plus, "graph_margin") -
                    scalar(minus, "graph_margin")),
                abs(scalar(plus, "energy_margin") -
                    scalar(minus, "energy_margin")))
    block_indices = (0, 64, 128, 192, 256)
    plus_blocks = [plus_values[index] for index in block_indices] if coherence else [0.0] * 5
    minus_blocks = [minus_values[index] for index in block_indices] if coherence else [0.0] * 5
    plus_increments = [b - a for a, b in zip(plus_blocks, plus_blocks[1:])]
    minus_increments = [b - a for a, b in zip(minus_blocks, minus_blocks[1:])]
    transport = (coherence and plus_blocks[-1] >= FINAL_GATE and
                 minus_blocks[-1] >= FINAL_GATE and
                 all(value > 0.0 for value in plus_increments + minus_increments) and
                 maximum_transverse <= TRANSVERSE_GATE and
                 hops_by_arm["plus"] >= 2 and hops_by_arm["minus"] >= 2 and
                 mirror_residual <= MIRROR_GATE)
    field_balanced = (transport and balanced_by_arm["plus"] and
                      balanced_by_arm["minus"])
    check(f"{slug} infrastructure parity",
          metadata.get("infrastructure_pass") is infrastructure)
    check(f"{slug} baseline parity", metadata.get("baseline_pass") is baseline)
    check(f"{slug} coherence parity", metadata.get("coherence_pass") is coherence)
    check(f"{slug} transport parity", metadata.get("transport_pass") is transport)
    check(f"{slug} field balance parity", metadata.get("field_balanced") is field_balanced)
    check(f"{slug} plus final parity", close(
        float(metadata.get("plus_final", math.nan)), plus_blocks[-1]))
    check(f"{slug} minus final parity", close(
        float(metadata.get("minus_final", math.nan)), minus_blocks[-1]))
    check(f"{slug} transverse parity", close(
        float(metadata.get("maximum_transverse", math.nan)), maximum_transverse))
    check(f"{slug} mirror parity", close(
        float(metadata.get("mirror_residual", math.nan)), mirror_residual))
    return {"infrastructure": infrastructure, "baseline": baseline,
            "coherence": coherence, "transport": transport,
            "field_balanced": field_balanced}


def certify() -> int:
    exact_hashes()
    check("result directory exists", RESULTS.is_dir())
    outcomes = {slug: reconstruct_direction(slug) for slug in DIRECTIONS}
    aggregate_path = RESULTS / "ftd_0761_m4_boosted_transport_v1.json"
    check("aggregate exists", aggregate_path.is_file())
    if not aggregate_path.is_file():
        return report("FTD-0761 artifact certificate")
    try:
        aggregate = json.loads(aggregate_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        check("aggregate parses", False)
        return report("FTD-0761 artifact certificate")
    infrastructure = all(bool(value["infrastructure"])
                         for value in outcomes.values())
    baseline = all(bool(value["baseline"]) for value in outcomes.values())
    coherence = all(bool(value["coherence"]) for value in outcomes.values())
    passing = [value for value in outcomes.values() if value["transport"]]
    count = len(passing)
    all_balanced = all(bool(value["field_balanced"]) for value in passing)
    if not infrastructure:
        verdict = "M4_BOOST_DISCOVERY_INFRASTRUCTURE_UNRESOLVED"
    elif not baseline:
        verdict = "M4_BOOST_DISCOVERY_BASELINE_INVALID"
    elif not coherence:
        verdict = "M4_BOOSTED_RELATIONAL_COHERENCE_CLOSED_AT_REGISTERED_SCALE"
    elif count == 0:
        verdict = "M4_BOOSTED_RELATIONAL_TRANSPORT_CLOSED_AT_REGISTERED_SCALE"
    elif count < 3:
        verdict = "M4_BOOSTED_RELATIONAL_TRANSPORT_ANISOTROPIC_WITNESS"
    else:
        verdict = "M4_BOOSTED_RELATIONAL_TRANSPORT_WITNESS"
    if count > 0:
        verdict += ("_FIELD_BALANCED" if all_balanced else
                    "_SUBSTRATE_REACTION_UNRESOLVED")
    check("aggregate id", aggregate.get("ftd_id") == "FTD-0761")
    check("aggregate protocol", aggregate.get("protocol_sha256") == PROTOCOL_HASH)
    check("aggregate infrastructure parity",
          aggregate.get("infrastructure_pass") is infrastructure)
    check("aggregate baseline parity", aggregate.get("baseline_pass") is baseline)
    check("aggregate coherence parity", aggregate.get("coherence_pass") is coherence)
    check("aggregate direction count", aggregate.get("transport_direction_count") == count)
    check("aggregate balance parity",
          aggregate.get("passing_directions_field_balanced") is all_balanced)
    check("aggregate verdict", aggregate.get("verdict") == verdict)
    check("aggregate production frozen", aggregate.get("production_changed") is False)
    check("aggregate dynamics frozen", aggregate.get("dynamics_changed") is False)
    return report("FTD-0761 artifact certificate", verdict)


def report(label: str, verdict: str | None = None) -> int:
    if failures:
        print(f"{label}: {checks - len(failures)}/{checks} checks")
        for failure in failures:
            print(f"FAIL: {failure}")
        return 1
    suffix = f"; verdict={verdict}" if verdict else ""
    print(f"{label}: {checks}/{checks} checks{suffix}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--preflight", action="store_true")
    arguments = parser.parse_args()
    return preflight() if arguments.preflight else certify()


if __name__ == "__main__":
    raise SystemExit(main())
