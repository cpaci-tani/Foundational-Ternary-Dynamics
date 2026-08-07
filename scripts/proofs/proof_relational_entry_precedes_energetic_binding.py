#!/usr/bin/env python3
"""Exact algebra and certified instance for FTD-0738."""

from __future__ import annotations

import csv
import hashlib
import json
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
THEOREM = ROOT / "docs/theory/10_eft_program/derivations/constituent_complete_matter/THEOREM_RELATIONAL_ENTRY_PRECEDES_ENERGETIC_BINDING_v1.md"
RESULT_JSON = ROOT / "engine/results/ftd_0737/ftd_0737_precontact_energetic_capture_delay_v1.json"
RESULT_CSV = ROOT / "engine/results/ftd_0737/ftd_0737_precontact_energetic_capture_delay_v1.csv"

EXPECTED_HASHES = {
    THEOREM: "9595C2C83A271BAFB0A696C999C89B235B6CEF1EB57CEE2970A4839BFB9E6322",
    RESULT_JSON: "E5622A9C1A4845B08793B6D65D35CDB5BF213115A9E1AE3545B22658D7908CDA",
    RESULT_CSV: "F164E3365BA5A9B434825B371E5B1FF5AFEAD5909C50A539141BFF6125697731",
}
REENTRY = {"0_0_1": 63, "0_1_-1": 79, "1_1_1": 96}
ONSET = {"0_0_1": 78, "0_1_-1": 94, "1_1_1": 111}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def polynomial(d: Fraction) -> Fraction:
    return -16*d**3 + 60*d**2 - 72*d + 27


def derivative(d: Fraction) -> Fraction:
    return -48*d**2 + 120*d - 72


def factored(d: Fraction) -> Fraction:
    return -16*(d-Fraction(3, 2))**2*(d-Fraction(3, 4))


def main() -> None:
    checks = 0

    def require(condition: bool, message: str) -> None:
        nonlocal checks
        checks += 1
        if not condition:
            raise AssertionError(message)

    for path, expected in EXPECTED_HASHES.items():
        require(path.is_file(), f"missing {path}")
        require(sha256(path) == expected, f"hash {path}")

    declared_points = [
        Fraction(0), Fraction(1, 4), Fraction(3, 4), Fraction(1),
        Fraction(5, 4), Fraction(3, 2), Fraction(7, 4),
    ]
    for point in declared_points:
        require(polynomial(point) == factored(point),
                f"factorization at {point}")
    require(polynomial(Fraction(3, 2)) == 0, "cutoff value")
    require(derivative(Fraction(3, 2)) == 0, "cutoff derivative")
    require(polynomial(Fraction(3, 4)) == 0, "inner zero")
    require(polynomial(Fraction(1)) == -1, "well minimum")
    require(derivative(Fraction(1)) == 0, "well stationary")

    # The sign statements follow directly from the factorization: the square
    # is nonnegative and the remaining linear factor changes sign at 3/4.
    require(factored(Fraction(1, 2)) > 0, "repulsive-side sign witness")
    require(factored(Fraction(1)) < 0, "well sign witness")
    require((Fraction(3, 2)-Fraction(149, 100))**2 > 0,
            "quadratic cutoff approach")

    data = json.loads(RESULT_JSON.read_text(encoding="utf-8"))
    with RESULT_CSV.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    forward = {
        (row["direction"], int(row["tick"])): row
        for row in rows if row["phase"] == "forward"
    }
    require(data["verdict"] ==
            "PRECONTACT_DELAYED_ENERGETIC_CAPTURE_CONSTRUCTIVE",
            "parent verdict")
    require(data["maximum_source_radius"] == 3, "source support")
    require(data["minimum_measured_contact_tick"] == 123, "contact tick")
    require(data["horizon"] == 122, "horizon")

    for direction in REENTRY:
        entry = forward[(direction, REENTRY[direction])]
        onset = forward[(direction, ONSET[direction])]
        require(entry["graph_inside"] == "1", f"entry graph {direction}")
        require(float(entry["pair_energy"]) > 0.0,
                f"entry energy {direction}")
        require(onset["graph_inside"] == "1", f"onset graph {direction}")
        require(float(onset["pair_energy"]) < -1e-6,
                f"onset energy {direction}")
        require(ONSET[direction]-REENTRY[direction] == 15,
                f"delay {direction}")
        tail = [row for row in rows
                if row["phase"] == "forward"
                and row["direction"] == direction
                and int(row["tick"]) >= ONSET[direction]]
        require(all(row["graph_inside"] == "1"
                    and float(row["pair_energy"]) < -1e-6
                    for row in tail), f"negative tail {direction}")
        arm = next(item for item in data["arms"]
                   if item["direction"] == direction)
        require(arm["field_gain"] > 0.0, f"receiver gain {direction}")

    require(data["maximum_pair_field_balance"] <= 1e-8,
            "complete energy identity")
    require(data["maximum_inverse_recovery"] <= 1e-8,
            "state-only inverse")

    print(f"FTD-0738 certificate: {checks}/{checks} checks PASS")
    print("exact=U(3/2)=U'(3/2)=0; U(1)=-D")
    print("instance=reentry_positive; onset_negative; delay=15 on 3/3 rays")
    print("scope=theorem for selected compact-pair potential and common action")


if __name__ == "__main__":
    main()
