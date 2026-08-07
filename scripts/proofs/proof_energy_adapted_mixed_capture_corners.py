"""Independent run-record certificate for FTD-0734."""

from __future__ import annotations

import csv
import hashlib
import json
import math
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_ENERGY_ADAPTED_MIXED_CAPTURE_CORNERS_v1.md"
TEST = ROOT / "engine/tests/test_energy_adapted_mixed_capture_corners.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0734/ftd_0734_energy_adapted_mixed_capture_corners_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0734/ftd_0734_energy_adapted_mixed_capture_corners_v1.csv"

PREREG_SHA256 = "E2F4F92894526CBDE66B919D13AA22B739268739E3DF783977F08F8D7D2251C3"
TEST_SHA256 = "3F29678D76B70304599068EFB29F2643CF37CE3D4E2E6DFAF7679E8EBFF936F4"
JSON_SHA256 = "41E0FB2E5D3F95518DF086C941C8F6533F3FEEEC5250FB56B5D94E9AB6998889"
CSV_SHA256 = "FCB930BEE96D13773799EC74CED5E598FBBA91934CCAC7C7E11C2F28D8BEF947"
VERDICT = "CAPTURE_ENERGY_ADAPTED_MIXED_CORNERS_SURVIVE"
DEPTH = 0.01
CUTOFF2 = 1.5
CORNER = re.compile(
    r"^sr([mp])_s1([mp])_s2([mp])_r(in|out)_f(minus|plus)$"
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, message: str, checks: list[str]) -> None:
    if not condition:
        raise AssertionError(message)
    checks.append(message)


def close(actual: float, expected: float, tolerance: float = 1e-12) -> bool:
    return abs(actual - expected) <= tolerance * max(1.0, abs(expected))


def values(row: dict[str, str], name: str) -> list[float]:
    return [float(value) for value in row[name].split(";")]


def ticks(row: dict[str, str]) -> list[int]:
    return [] if not row["transition_ticks"] else [
        int(value) for value in row["transition_ticks"].split(";")
    ]


def key(row: dict[str, str]) -> tuple[str, str, str]:
    return row["direction"], row["polarity"], row["variant"]


def inside(separation: float) -> bool:
    return separation * separation < CUTOFF2


def potential(d: float) -> float:
    if d >= CUTOFF2:
        return 0.0
    return -16.0 * DEPTH * (d - 1.5) ** 2 * (d - 0.75)


def recompute_transitions(separation: list[float]) -> list[int]:
    membership = [inside(value) for value in separation]
    return [
        tick for tick in range(1, len(membership))
        if membership[tick] != membership[tick - 1]
    ]


def main() -> None:
    checks: list[str] = []
    check(sha256(PREREG) == PREREG_SHA256, "protocol hash locked", checks)
    check(sha256(TEST) == TEST_SHA256, "runner hash locked", checks)
    check(sha256(JSON_PATH) == JSON_SHA256, "JSON hash locked", checks)
    check(sha256(CSV_PATH) == CSV_SHA256, "CSV hash locked", checks)

    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    check(summary["identifier"] == "FTD-0734", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256,
          "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked positive verdict", checks)
    check(summary["arm_count"] == len(rows) == 216,
          "216 locked histories", checks)
    stage_a = [row for row in rows if row["volume"] == "33"]
    stage_b = [row for row in rows if row["volume"] == "65"]
    check(len(stage_a) == 198 and len(stage_b) == 18,
          "198/18 stage partition", checks)
    check(all(row["parent_valid"] == "1" for row in rows),
          "all parent centers valid", checks)
    check(all(row["parent_reproduction_pass"] == "1" for row in rows),
          "all parent anchors reproduce", checks)

    registered = [row for row in rows if row["registered_corner"] == "1"]
    centers = [row for row in rows if row["variant"] == "center"]
    check(len(registered) == 204 and len(centers) == 12,
          "204 mixed corners and 12 centers", checks)
    for direction in {row["direction"] for row in stage_a}:
        for polarity in {row["polarity"] for row in stage_a}:
            group = [row for row in stage_a
                     if row["direction"] == direction
                     and row["polarity"] == polarity]
            names = {row["variant"] for row in group if row["variant"] != "center"}
            check(len(group) == 33 and len(names) == 32,
                  f"complete Stage-A corner cube {direction} {polarity}", checks)

    for index, row in enumerate(rows):
        label = f"row {index} L={row['volume']} {key(row)}"
        separation = values(row, "separation_history")
        internal = values(row, "internal_history")
        field = values(row, "field_history")
        check(len(separation) == len(internal) == len(field) == 257,
              f"257 persisted states {label}", checks)
        check(recompute_transitions(separation) == ticks(row) == [],
              f"no graph transition recomputed {label}", checks)
        check(all(inside(value) for value in separation),
              f"all states graph-inside {label}", checks)
        check(all(value < -1e-6 for value in internal),
              f"all states negative-energy {label}", checks)
        check(all(value >= -1e-12 for value in field),
              f"all field energies nonnegative {label}", checks)
        balance = abs((field[-1] - field[0]) + (internal[-1] - internal[0]))
        check(close(balance, float(row["pair_field_balance"]), 2e-14),
              f"energy balance recomputed {label}", checks)
        check(balance <= 1e-8, f"energy balance gate {label}", checks)
        check(row["initialized"] == row["executed"] == row["identity_pass"]
              == row["recoil_pass"] == row["inverse_pass"]
              == row["positive_field_energy"] == row["survives"] == "1",
              f"all emitted gates pass {label}", checks)
        check(row["final_class"] == "survives",
              f"survival class emitted {label}", checks)
        check(float(row["initial_gauss_residual"]) <= 1e-12,
              f"initial Gauss gate {label}", checks)
        check(float(row["initial_momentum_preservation"]) <= 1e-15,
              f"initial momentum gate {label}", checks)
        check(float(row["initial_maximum_speed"]) <= 1 / math.sqrt(3) + 1e-12,
              f"initial causal gate {label}", checks)
        check(float(row["max_common_residual"]) <= 1e-10,
              f"common-action gate {label}", checks)
        check(float(row["max_recoil_defect"]) <= 1e-9,
              f"recoil gate {label}", checks)
        check(float(row["inverse_recovery"]) <= 1e-8,
              f"inverse gate {label}", checks)
        energy_margin = min(-value / DEPTH for value in internal)
        graph_margin = min(math.sqrt(CUTOFF2) - value for value in separation)
        check(close(energy_margin, float(row["minimum_energy_margin"])),
              f"energy margin recomputed {label}", checks)
        check(close(graph_margin, float(row["minimum_graph_margin"])),
              f"graph margin recomputed {label}", checks)

        if row["registered_corner"] != "1":
            continue
        match = CORNER.fullmatch(row["variant"])
        check(match is not None, f"corner name grammar {label}", checks)
        assert match is not None
        signs = tuple(-1 if item == "m" else 1 for item in match.group(1, 2, 3))
        radial_side = -1 if match.group(4) == "in" else 1
        field_scale = 0.95 if match.group(5) == "minus" else 1.05
        check(signs == (int(row["sigma_r"]), int(row["sigma_1"]),
                        int(row["sigma_2"])),
              f"corner signs decode {label}", checks)
        check(radial_side == int(row["radial_side"]),
              f"radial side decodes {label}", checks)
        check(float(row["field_scale"]) == field_scale,
              f"field scale decodes {label}", checks)
        kinetic = float(row["kinetic"])
        inner = float(row["inner_d"])
        outer = float(row["outer_d"])
        margin = float(row["nearest_margin"])
        target = float(row["target_d"])
        parent_d = target - radial_side * 0.5 * margin
        check(0.0 < kinetic < DEPTH,
              f"kinetic lies below well depth {label}", checks)
        check(0.75 < inner < 1.0 < outer < CUTOFF2,
              f"two exact shell roots bracket minimum {label}", checks)
        root_residual = max(abs(kinetic + potential(inner)),
                            abs(kinetic + potential(outer)))
        check(close(root_residual, float(row["root_residual"]), 1e-14)
              and root_residual <= 1e-12,
              f"energy roots recomputed {label}", checks)
        check(inner < parent_d < outer,
              f"parent lies inside its kinetic shell {label}", checks)
        check(close(margin, min(parent_d - inner, outer - parent_d), 1e-13),
              f"nearest exact shell margin recomputed {label}", checks)
        check(close(target, parent_d + radial_side * 0.5 * margin, 1e-14),
              f"half-margin radial point recomputed {label}", checks)
        check(inner < target < outer,
              f"target remains strictly shell-interior {label}", checks)
        check(close(internal[0], kinetic + potential(target), 2e-13),
              f"initial pair energy recomputed {label}", checks)

    check(sum(row["survives"] == "1" for row in stage_a) == 198,
          "198/198 Stage-A histories survive", checks)
    check(sum(row["survives"] == "1" for row in stage_b) == 18,
          "18/18 Stage-B histories survive", checks)
    check(len(centers) == 12 and all(row["survives"] == "1" for row in centers),
          "12/12 centers survive through parent tick 384", checks)

    selectors = summary["selectors"]
    check(len(selectors) == 6, "six held-out selector groups", checks)
    by_a = {key(row): row for row in stage_a}
    for selector in selectors:
        direction = selector["direction"]
        polarity = selector["polarity"]
        candidates = [row for row in stage_a
                      if row["direction"] == direction
                      and row["polarity"] == polarity
                      and row["variant"] != "center"]
        energy_variant = min(
            candidates,
            key=lambda row: (float(row["minimum_energy_margin"]),
                             row["variant"]))["variant"]
        graph_variant = next(
            row["variant"] for row in sorted(
                candidates,
                key=lambda row: (float(row["minimum_graph_margin"]),
                                 row["variant"]))
            if row["variant"] != energy_variant
        )
        check(selector["energy_variant"] == energy_variant,
              f"energy selector reproduced {direction} {polarity}", checks)
        check(selector["graph_variant"] == graph_variant,
              f"graph selector reproduced {direction} {polarity}", checks)
        for variant in ("center", energy_variant, graph_variant):
            small = by_a[(direction, polarity, variant)]
            large = next(row for row in stage_b
                         if row["direction"] == direction
                         and row["polarity"] == polarity
                         and row["variant"] == variant)
            check(small["survives"] == large["survives"] == "1",
                  f"held-out volume class match {direction} {polarity} {variant}",
                  checks)
            check(ticks(small) == ticks(large) == [],
                  f"held-out transition match {direction} {polarity} {variant}",
                  checks)

    for direction in {row["direction"] for row in stage_a}:
        variants = {row["variant"] for row in stage_a
                    if row["direction"] == direction}
        for variant in variants:
            plus = by_a[(direction, "plus_minus", variant)]
            minus = by_a[(direction, "minus_plus", variant)]
            check(plus["survives"] == minus["survives"] == "1",
                  f"polarity class match {direction} {variant}", checks)

    check(summary["stage_a_survives"] == 198, "summary Stage A", checks)
    check(summary["stage_b_survives"] == 18, "summary Stage B", checks)
    check(summary["center_survives"] == 12, "summary centers", checks)
    check(summary["polarity_mismatches"] == 0,
          "summary polarity match", checks)
    check(summary["volume_mismatches"] == 0,
          "summary volume match", checks)
    check(close(summary["maximum_common"], max(
        float(row["max_common_residual"]) for row in rows)),
        "summary common maximum", checks)
    check(close(summary["maximum_recoil"], max(
        float(row["max_recoil_defect"]) for row in rows)),
        "summary recoil maximum", checks)
    check(close(summary["maximum_inverse"], max(
        float(row["inverse_recovery"]) for row in rows)),
        "summary inverse maximum", checks)
    check(close(summary["maximum_balance"], max(
        float(row["pair_field_balance"]) for row in rows)),
        "summary balance maximum", checks)
    check(close(summary["minimum_shell_margin"], min(
        float(row["nearest_margin"]) for row in registered)),
        "summary shell-margin minimum", checks)

    print(f"FTD-0734 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print("histories: Stage A 198/198; Stage B 18/18; centers 12/12")
    print("scope: finite directions/amplitudes/volumes/horizon; no open basin")


if __name__ == "__main__":
    main()
