"""Independent record audit for FTD-0624.

This script validates the preregistered classification and the exact-half
site-projection obstruction from the frozen CSV/JSON records.  It performs no
parameter search and does not rerun the engine.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0624"
JSON_PATH = RESULTS / "ftd_0624_connected_block_translation_stability_v1.json"
ARMS_PATH = RESULTS / "ftd_0624_connected_block_translation_stability_arms_v1.csv"
TICKS_PATH = RESULTS / "ftd_0624_connected_block_translation_stability_ticks_v1.csv"
PROTOCOL = "CB8AA8843B92F2D8ACB791C5DB01081C6BB2F6AD70E86EC074BBE0EA3E5720A2"
HASHES = {
    JSON_PATH.name: "55D34381B4968653740DF57A0F2330A3D175CC2CFD52012A2C4657D601825653",
    ARMS_PATH.name: "761B5F7CC461AC82A51B86D634560C0E44D4CD789B590E59445BE8237854E2BD",
    TICKS_PATH.name: "8B4383B394CC18423F9AF6184059E01469A49736217703511A9854D984D573F8",
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


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def number(row: dict[str, str], key: str) -> float:
    return float(row[key])


def integer(row: dict[str, str], key: str) -> int:
    return int(row[key])


def vec(row: dict[str, str], prefix: str) -> tuple[float, float, float]:
    return tuple(number(row, f"{prefix}{axis}") for axis in ("x", "y", "z"))


def max_abs(values: tuple[float, ...]) -> float:
    return max(abs(value) for value in values)


def add(a: tuple[float, ...], b: tuple[float, ...]) -> tuple[float, ...]:
    return tuple(x + y for x, y in zip(a, b))


def cycle(a: tuple[float, float, float]) -> tuple[float, float, float]:
    return (a[2], a[0], a[1])


def main() -> int:
    for path in (JSON_PATH, ARMS_PATH, TICKS_PATH):
        check(f"frozen hash {path.name}", sha256(path) == HASHES[path.name])

    record = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with ARMS_PATH.open(newline="", encoding="utf-8") as stream:
        arms = list(csv.DictReader(stream))
    with TICKS_PATH.open(newline="", encoding="utf-8") as stream:
        ticks = list(csv.DictReader(stream))
    by_label = {row["label"]: row for row in arms}
    tick_by_label: dict[str, list[dict[str, str]]] = {}
    for row in ticks:
        tick_by_label.setdefault(row["label"], []).append(row)

    check("record identifies FTD-0624", record["ftd_id"] == "FTD-0624")
    check("protocol fingerprint is locked", record["protocol_sha256"] == PROTOCOL)
    check("production remains unchanged", record["production_changed"] is False)
    check("all fourteen arms are present", len(arms) == 14)
    check("static spectral law passes", record["static_law_pass"] == 1)
    check("registered verdict is execution-invalid", record["verdict"]
          == "CONNECTED_TRANSLATION_STABILITY_EXECUTION_INVALID")
    check("invalidity is confined to action coverage", record["action_pass"] == 0
          and record["runaway_pass"] == 1 and record["restoring_pass"] == 1)
    check("undefined covariance is legal JSON null", record["covariance_residual"] is None)

    static_ok = True
    for axis in record["static_axes"]:
        coefficient = float(axis["coefficient"])
        barrier = float(axis["barrier"])
        zero = float(axis["energy_zero"])
        positive = float(axis["energy_positive_half"])
        negative = float(axis["energy_negative_half"])
        static_ok &= (axis["pass"] == 1 and coefficient > 0.0
                      and abs(barrier - coefficient / 16.0) <= 1e-15
                      and zero > positive and zero > negative
                      and abs(positive - negative) <= 1e-15
                      and float(axis["maximum_residual"]) <= 1e-10)
    check("both axes obey U(0)-U(half)=C/16", static_ok)

    exact_maxima = [by_label["x_exact_max"], by_label["y_exact_max"]]
    check("integer extrema are stationary and reversible", all(
        integer(row, "forward") == integer(row, "reverse") == 1
        and max_abs(vec(row, "final_d")) <= 1e-8
        and max_abs(vec(row, "final_p")) <= 1e-8
        for row in exact_maxima))

    x_half = by_label["x_exact_min"]
    check("x half-cell solve converges before the ontic gate", 
          integer(x_half, "failed_solve_converged") == 1
          and number(x_half, "maximum_common") <= 1e-10)
    check("x half-cell endpoint violates unique-site projection at tick one",
          integer(x_half, "forward") == 0
          and integer(x_half, "failure_tick") == 1
          and integer(x_half, "failed_site_projection") == 1)

    y_half = by_label["y_exact_min"]
    check("y half-cell advances eight ticks", integer(y_half, "forward") == 1
          and integer(y_half, "total_hops") == 8)
    check("y half-cell state-only inverse is undefined", integer(y_half, "reverse") == 0
          and math.isinf(number(y_half, "recovery")))

    accepted = [row for row in arms
                if row["label"] not in {"x_exact_min", "y_exact_min"}]
    check("the other twelve arms are coherent and state-only reversible", all(
        all(integer(row, key) == 1
            for key in ("init", "static_law", "forward", "reverse", "coherence"))
        for row in accepted))
    check("accepted arms retain exact action residuals", max(
        number(row, "maximum_common") for row in accepted) <= 1e-10)
    check("accepted arms conserve total energy", max(
        number(row, "maximum_energy_drift") for row in accepted) <= 1e-9)
    check("accepted arms remain coherent", max(
        number(row, "maximum_shape") for row in accepted) <= 0.05
        and max(number(row, "maximum_strain") for row in accepted) <= 0.10)
    check("accepted arms invert below the registered gate", max(
        number(row, "recovery") for row in accepted) <= 1e-8)

    maximum_rows = [row for row in arms
                    if row["kind"] == "maximum_perturbation"]
    minimum_rows = [row for row in arms
                    if row["kind"] == "minimum_perturbation"]
    check("every maximum perturbation runs away", len(maximum_rows) == 5
          and all(integer(row, "runaway") == 1 for row in maximum_rows))
    check("every minimum perturbation restores", len(minimum_rows) == 5
          and all(integer(row, "restoring") == 1 for row in minimum_rows))

    mirror_ok = True
    for positive, negative in (
        ("x_max_positive", "x_max_negative"),
        ("x_min_positive", "x_min_negative"),
        ("y_max_positive", "y_max_negative"),
        ("y_min_positive", "y_min_negative"),
    ):
        lhs, rhs = by_label[positive], by_label[negative]
        mirror_ok &= max_abs(add(vec(lhs, "final_d"), vec(rhs, "final_d"))) <= 1e-8
        mirror_ok &= max_abs(add(vec(lhs, "first_p"), vec(rhs, "first_p"))) <= 1e-8
    check("all perturbation partners mirror", mirror_ok)

    covariance_ok = True
    for base, rotated in (("x_max_positive", "cyclic_max_positive"),
                          ("x_min_positive", "cyclic_min_positive")):
        lhs, rhs = by_label[base], by_label[rotated]
        covariance_ok &= max_abs(add(vec(rhs, "final_d"),
                                     tuple(-v for v in cycle(vec(lhs, "final_d"))))) <= 1e-8
        covariance_ok &= max_abs(add(vec(rhs, "first_p"),
                                     tuple(-v for v in cycle(vec(lhs, "first_p"))))) <= 1e-8
    check("accepted cyclic controls are covariant", covariance_ok)

    expected_tick_rows = 12 * 8 + 8
    check("tick record contains all accepted forward steps", len(ticks) == expected_tick_rows)
    check("every recorded endpoint preserves topology", all(
        integer(row, "topology_pass") == 1 for row in ticks))
    check("every recorded tick satisfies the common-action gate", max(
        number(row, "common_residual") for row in ticks) <= 1e-10)

    print(f"\nFTD-0624 proof checks: {checks - failures}/{checks} passed")
    print("verdict=EXACT_HALF_CELL_SITE_PROJECTION_OBSTRUCTION_CONFIRMED")
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
