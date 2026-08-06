"""Independent certificate for the FTD-0754C nested-support replay.

This script consumes only the already-seen FTD-0754B/0754C artifacts.  It
does not run dynamics, generate perturbations, inspect FTD-0755 data, or
search for physical-constant coincidences.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0754_support_ladder"
BOUNDARY = ROOT / "engine" / "results" / "ftd_0754_boundary_accounting"
PROTOCOL_HASH = "F1E8A18631D923040607128D34CCC6C2FF17D6D9D0BA594CBF57C7A9157BD48A"
EXPECTED_HASHES = {
    "docs/theory/10_eft_program/preregistrations/PREREG_STATE_ONLY_SUPPORT_LADDER_v1.md":
        PROTOCOL_HASH,
    "engine/include/ftd/eft/state_only_matter_field_observer.h":
        "F180DAE14DF62244E9F091F68670EA1EEA192881D87BAE86D43BE633C09CC696",
    "engine/src/eft/state_only_matter_field_observer.cpp":
        "10BF768DC480C5A0699A18B097E44AC685A27D13BF2C90C95758EC1FF3D3FB2F",
    "engine/tests/campaign_state_only_observer_discovery_cuda.cpp":
        "AB2B2F4EB238413B59D1D48240509D72EE43C195080EE814668A67CE01F63107",
}
ARMS = {
    "face": "0_0_1",
    "edge": "0_1_-1",
    "body": "1_1_1",
}
TICKS = (0, 80, 96, 115, 160, 240, 297, 312)
SUPPORTS = (4, 6, 8)
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


def close(value: float, expected: float, gate: float = GATE) -> bool:
    scale = max(1.0, abs(value), abs(expected))
    return math.isfinite(value) and abs(value - expected) <= gate * scale


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


for relative, expected in EXPECTED_HASHES.items():
    path = ROOT / relative
    check(f"hash exists: {relative}", path.is_file())
    if path.is_file():
        check(f"hash exact: {relative}", sha256(path) == expected)

maximum_reconstruction = 0.0
maximum_projection = 0.0
maximum_pythagorean = 0.0
maximum_prior_primitive_mismatch = 0.0
minimum_drop = math.inf
flow_fractions: list[float] = []
all_rows = 0
all_transitions = 0

for arm, direction in ARMS.items():
    stem = f"ftd_0754c_state_only_support_ladder_v1_{arm}"
    csv_path = RESULTS / f"{stem}.csv"
    json_path = RESULTS / f"{stem}.json"
    check(f"{arm} csv exists", csv_path.is_file())
    check(f"{arm} json exists", json_path.is_file())
    if not csv_path.is_file() or not json_path.is_file():
        continue

    metadata = json.loads(json_path.read_text(encoding="utf-8"))
    check(f"{arm} source id", metadata.get("source_ftd_id") == "FTD-0754")
    check(f"{arm} addendum", metadata.get("analytic_addendum") == "FTD-0754C")
    check(f"{arm} protocol", metadata.get("protocol_sha256") == PROTOCOL_HASH)
    check(f"{arm} scope",
          metadata.get("scope") == "posthoc_existing_discovery_corpus_no_validation")
    check(f"{arm} supports", metadata.get("support_half_widths") == list(SUPPORTS))
    check(f"{arm} observer rows", metadata.get("observer_rows") == len(TICKS))
    check(f"{arm} replay exact", metadata.get("scalar_replay_exact") == 1)
    check(f"{arm} dynamics unchanged", metadata.get("dynamics_changed") is False)
    check(f"{arm} heldout absent",
          metadata.get("held_out_validation_consumed") is False)

    rows = read_csv(csv_path)
    all_rows += len(rows)
    check(f"{arm} scale row count", len(rows) == len(TICKS) * len(SUPPORTS))
    grouped: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        check(f"{arm} row arm", row["arm"] == arm)
        check(f"{arm} row direction", row["direction"] == direction)
        grouped[int(row["tick"])].append(row)
    check(f"{arm} tick set", tuple(sorted(grouped)) == TICKS)

    boundary_path = BOUNDARY / f"ftd_0754b_boundary_accounting_v1_{arm}.csv"
    check(f"{arm} boundary source exists", boundary_path.is_file())
    boundary_by_tick = {
        int(row["tick"]): row for row in read_csv(boundary_path)
    } if boundary_path.is_file() else {}

    for tick in TICKS:
        tick_rows = sorted(
            grouped.get(tick, []), key=lambda row: int(row["support_half_width"]))
        check(f"{arm}/{tick} support set",
              tuple(int(row["support_half_width"]) for row in tick_rows) == SUPPORTS)
        if len(tick_rows) != len(SUPPORTS):
            continue
        actual_values = [float(row["actual_face_energy"]) for row in tick_rows]
        check(f"{arm}/{tick} actual scale invariant",
              all(value == actual_values[0] for value in actual_values[1:]))

        bound_values: list[float] = []
        for index, row in enumerate(tick_rows):
            prefix = f"{arm}/{tick}/R{SUPPORTS[index]}"
            check(prefix + " valid", row["valid"] == "1")
            check(prefix + " replay", row["scalar_replay_exact"] == "1")
            check(prefix + " ladder", row["ladder_valid"] == "1")
            actual = float(row["actual_face_energy"])
            bound = float(row["bound_face_energy"])
            residual = float(row["residual_face_energy"])
            cross = float(row["primitive_interference"])
            reconstruction = float(row["energy_reconstruction_residual"])
            bound_values.append(bound)
            check(prefix + " finite",
                  all(math.isfinite(value)
                      for value in (actual, bound, residual, cross, reconstruction)))
            check(prefix + " nonnegative energies",
                  actual >= 0.0 and bound >= 0.0 and residual >= 0.0)
            check(prefix + " direct energy identity",
                  close(actual, bound + residual + cross))
            check(prefix + " recorded reconstruction",
                  close(reconstruction, actual - bound - residual - cross))
            check(prefix + " reconstruction gate", abs(reconstruction) <= GATE)
            maximum_reconstruction = max(maximum_reconstruction,
                                         abs(reconstruction))

            if index == 0 and tick in boundary_by_tick:
                prior_primitive = float(
                    boundary_by_tick[tick]["primitive_face_interference"])
                primitive_mismatch = abs(cross - prior_primitive)
                maximum_prior_primitive_mismatch = max(
                    maximum_prior_primitive_mismatch, primitive_mismatch)
                check(prefix + " prior primitive bounded",
                      close(cross, prior_primitive))

            if index == 0:
                check(prefix + " no transition",
                      int(row["inner_half_width"]) == 0
                      and int(row["outer_half_width"]) == 0)
                continue

            all_transitions += 1
            inner = int(row["inner_half_width"])
            outer = int(row["outer_half_width"])
            relaxation = float(row["relaxation_energy"])
            projection = float(row["outer_difference_inner_product"])
            pythagorean = float(row["pythagorean_residual"])
            margin = float(row["monotonicity_margin"])
            expected_margin = bound_values[index - 1] - bound
            check(prefix + " transition labels",
                  (inner, outer) == (SUPPORTS[index - 1], SUPPORTS[index]))
            check(prefix + " monotone", margin >= -GATE)
            check(prefix + " margin direct", close(margin, expected_margin))
            check(prefix + " relaxation nonnegative", relaxation >= 0.0)
            check(prefix + " pythagorean direct",
                  close(pythagorean, margin - relaxation))
            check(prefix + " projection gate", abs(projection) <= GATE)
            check(prefix + " pythagorean gate", abs(pythagorean) <= GATE)
            maximum_projection = max(maximum_projection, abs(projection))
            maximum_pythagorean = max(maximum_pythagorean, abs(pythagorean))
            minimum_drop = min(minimum_drop, margin)

        if bound_values and bound_values[0] > 0.0:
            flow_fractions.append((bound_values[0] - bound_values[-1])
                                  / bound_values[0])

check("total scale rows", all_rows == 72)
check("total transition rows", all_transitions == 48)
check("flow fraction count", len(flow_fractions) == 24)

if failures:
    print(f"FTD-0754C support ladder: {checks - len(failures)}/{checks} checks")
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)

print(f"FTD-0754C support ladder: {checks}/{checks} checks")
print(f"scale_rows={all_rows} transition_rows={all_transitions}")
print(f"max_energy_reconstruction_residual={maximum_reconstruction:.17g}")
print(f"max_projection_inner_product={maximum_projection:.17g}")
print(f"max_pythagorean_residual={maximum_pythagorean:.17g}")
print(f"max_prior_primitive_mismatch={maximum_prior_primitive_mismatch:.17g}")
print(f"minimum_bound_energy_drop={minimum_drop:.17g}")
print(f"minimum_R4_to_R8_flow_fraction={min(flow_fractions):.17g}")
print(f"maximum_R4_to_R8_flow_fraction={max(flow_fractions):.17g}")
