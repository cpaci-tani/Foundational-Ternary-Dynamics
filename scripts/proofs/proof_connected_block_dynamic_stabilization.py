"""Independent record audit for FTD-0625 (no search, no engine mutation)."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0625"
JSON_PATH = RESULTS / "ftd_0625_connected_block_dynamic_stabilization_v1.json"
ARMS_PATH = RESULTS / "ftd_0625_connected_block_dynamic_stabilization_arms_v1.csv"
TICKS_PATH = RESULTS / "ftd_0625_connected_block_dynamic_stabilization_ticks_v1.csv"
PROTOCOL = "E95F2EB5A91C599AEFF790F55A34E548628D95FE247E95C843446A6940E751CA"
HASHES = {
    JSON_PATH.name: "99F6337B6B210DFBDF08C175DA62F6777FCD20E376D46D57B1A25A94229A4C02",
    ARMS_PATH.name: "3BC30ADA092A0317EE8CA865949034150E0862E83208D909C19B08990F7C5DF4",
    TICKS_PATH.name: "DD77EC9CC2C9CD30A34C5C4A94739AF530B6B2C095908D23EBC9693579C981DB",
}

checks = 0
failures = 0


def check(label: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if condition:
        print(f"PASS {checks:02d}: {label}")
    else:
        failures += 1
        print(f"FAIL {checks:02d}: {label}")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def f(row: dict[str, str], key: str) -> float:
    return float(row[key])


def i(row: dict[str, str], key: str) -> int:
    return int(row[key])


def main() -> int:
    for path in (JSON_PATH, ARMS_PATH, TICKS_PATH):
        check(f"frozen hash {path.name}", digest(path) == HASHES[path.name])
    record = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with ARMS_PATH.open(newline="", encoding="utf-8") as stream:
        arms = list(csv.DictReader(stream))
    with TICKS_PATH.open(newline="", encoding="utf-8") as stream:
        ticks = list(csv.DictReader(stream))
    by_label = {row["label"]: row for row in arms}

    check("record identifies FTD-0625", record["ftd_id"] == "FTD-0625")
    check("protocol is locked", record["protocol_sha256"] == PROTOCOL)
    check("production remains unchanged", record["production_changed"] is False)
    check("all seven arms are present", len(arms) == 7)
    check("campaign execution is valid", record["coverage_pass"] == 1
          and record["execution_pass"] == 1)
    check("registered rigid-circulation verdict is negative", record["verdict"]
          == "RIGID_CIRCULATION_DYNAMIC_STABILIZATION_CLOSED_NEGATIVE")
    check("neither energy family qualifies", record["family_one_pass"] == 0
          and record["family_four_pass"] == 0)
    check("sign and cyclic comparisons pass", record["symmetry_pass"] == 1
          and record["covariance_pass"] == 1)

    control = by_label["near_half_zero"]
    check("zero-circulation control completes forward and reverse",
          i(control, "forward_complete") == i(control, "reverse") == 1)
    check("zero control remains conflict free", i(control, "max_conflicts") == 0
          and i(control, "failure_tick") == 0)
    check("zero control is exact and coherent", i(control, "coherence") == 1
          and i(control, "exact_steps") == 1)
    check("zero control recovers below gate", f(control, "recovery") <= 1e-8)
    check("baseline chart margin is recorded", abs(
        f(control, "min_chart_margin")-record["baseline_margin"]) <= 1e-15)

    nonzero = [row for row in arms if i(row, "energy_multiple") > 0]
    one = [row for row in nonzero if i(row, "energy_multiple") == 1]
    four = [row for row in nonzero if i(row, "energy_multiple") == 4]
    check("all circulation amplitudes are model-internally normalized", all(
        i(row, "amplitude_pass") == 1
        and f(row, "amplitude_residual") <= 1e-13 for row in nonzero))
    check("all launches have zero total momentum", all(
        f(row, "initial_momentum") <= 1e-14 for row in nonzero))
    check("one-barrier amplitudes are identical", max(
        f(row, "amplitude") for row in one)-min(
        f(row, "amplitude") for row in one) <= 1e-15)
    check("four-barrier amplitudes are identical", max(
        f(row, "amplitude") for row in four)-min(
        f(row, "amplitude") for row in four) <= 1e-15)
    check("higher energy has larger amplitude", min(
        f(row, "amplitude") for row in four) > max(
        f(row, "amplitude") for row in one))

    check("every circulating solve converges before failure", all(
        i(row, "failure_solve_converged") == 1 for row in nonzero))
    check("every circulating failure is the site-projection gate", all(
        i(row, "failure_site_projection") == 1
        and i(row, "failure_graph") == 0 for row in nonzero))
    check("every failed root is numerically exact", max(
        f(row, "failure_root_residual") for row in nonzero) <= 1e-10)
    check("every failure has exactly two conflicts", all(
        i(row, "failure_conflicts") == 2 for row in nonzero))
    check("every conflict is opposite polarity", all(
        i(row, "failure_same_polarity_pairs") == 0
        and i(row, "failure_opposite_polarity_pairs") == 2
        for row in nonzero))
    check("one-barrier circulation fails at tick two", all(
        i(row, "failure_tick") == 2 for row in one))
    check("four-barrier circulation fails at tick one", all(
        i(row, "failure_tick") == 1 for row in four))
    check("higher circulation energy does not delay collision", min(
        i(row, "failure_tick") for row in four) < min(
        i(row, "failure_tick") for row in one))

    positive_one = by_label["circulation_positive_1B"]
    negative_one = by_label["circulation_negative_1B"]
    positive_four = by_label["circulation_positive_4B"]
    negative_four = by_label["circulation_negative_4B"]
    check("circulation sign reverses initial angular momentum", 
          abs(f(positive_one, "initial_angular_momentum")
              +f(negative_one, "initial_angular_momentum")) <= 1e-15
          and abs(f(positive_four, "initial_angular_momentum")
                  +f(negative_four, "initial_angular_momentum")) <= 1e-15)
    check("registered symmetry residual closes", record["symmetry_residual"] <= 1e-8)
    check("registered covariance residual closes", record["covariance_residual"] <= 1e-8)

    check("tick record has the expected valid-prefix rows", len(ticks) == 19)
    check("every recorded prefix tick is conflict free", all(
        i(row, "conflicts") == 0 for row in ticks))
    check("every recorded prefix tick passes the action gate", max(
        f(row, "common_residual") for row in ticks) <= 1e-10)
    check("recorded energy drift remains exact", max(
        f(row, "energy_drift") for row in ticks) <= 1e-9)
    check("non-finite recoveries occur only on closed circulation arms", all(
        math.isinf(f(row, "recovery")) for row in nonzero))

    print(f"\nFTD-0625 proof checks: {checks - failures}/{checks} passed")
    print("verdict=RIGID_CIRCULATION_ACCELERATES_OPPOSITE_POLARITY_COLLISION")
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
