#!/usr/bin/env python3
"""Independent run-of-record certificate for FTD-0648."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "engine/results/ftd_0648"
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_CELL_MEASURE_FIXED_MASS_REFINEMENT_v1.md"
JSON_PATH = BASE / "ftd_0648_cell_measure_fixed_mass_refinement_v1.json"
ARMS_PATH = BASE / "ftd_0648_cell_measure_fixed_mass_refinement_arms_v1.csv"
SLOPES_PATH = BASE / "ftd_0648_cell_measure_fixed_mass_refinement_slopes_v1.csv"

HASHES = {
    PREREG: "9CB970060317A99B6D544C4DA05D81A6EC3F82CDD5399A149D5BD55B89A7F5BF",
    JSON_PATH: "7BA9BF4B579A7902F6CD331A4C48F8C7F45AAE097C144DAE37DC9569AD398B8D",
    ARMS_PATH: "D72B2B9B7F8AB3A90FE4D30667FDF4A526675B278822C080C5EED1FFE8A9EFAA",
    SLOPES_PATH: "850FD6E35149B9EB20CD92080CFFD15D119D2238F61E6A60C1B254870600AE65",
}
VERDICT = "CELL_MEASURE_FIXED_MASS_STATIC_DEPINNING_CONSTRUCTIVE"
WIDTHS = (2, 3, 4, 5, 6, 8)
E_REST = 0.511 / 3.0
M_INERTIAL = 0.511


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def close(a: float, b: float, tolerance: float = 1e-12) -> bool:
    return abs(a-b) <= tolerance * max(1.0, abs(a), abs(b))


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS {label}")


def main() -> None:
    for path, expected in HASHES.items():
        require(digest(path) == expected, f"hash {path.name}")
    record = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with ARMS_PATH.open(newline="", encoding="utf-8") as handle:
        arms = list(csv.DictReader(handle))
    with SLOPES_PATH.open(newline="", encoding="utf-8") as handle:
        slopes = list(csv.DictReader(handle))

    require(record["verdict"] == VERDICT, "locked verdict")
    require(record["production_changed"] is False, "production unchanged")
    require(len(arms) == 54 and len(slopes) == 9, "locked coverage")
    for gate in ("normalization_pass", "coverage_pass", "exact_pass",
                 "positivity_pass", "monotonic_pass", "energy_scaling_pass",
                 "barrier_scaling_pass", "endpoint_pass", "cubic_pass"):
        require(record[gate] is True, gate)

    expected_orbit = {(w, o, p) for w in WIDTHS for o in range(3) for p in range(3)}
    actual_orbit = {(int(r["width"]), int(r["orientation"]), int(r["phase_axis"]))
                    for r in arms}
    require(actual_orbit == expected_orbit, "complete cubic width orbit")

    grouped: dict[tuple[int, int], list[tuple[int, float]]] = {}
    for row in arms:
        w = int(row["width"])
        a = 2.0 / w
        require(close(float(row["mass_scale"]), a**3), f"mass measure w={w}")
        require(close(float(row["polarity_scale"]), a**3), f"polarity measure w={w}")
        require(close(float(row["binding_scale"]), a**3), f"binding measure w={w}")
        require(close(float(row["beta_scale"]), 1.0/a), f"field measure w={w}")
        require(close(float(row["integrated_positive"]), 8.0), f"Q+ fixed w={w}")
        require(close(float(row["integrated_negative"]), 8.0), f"Q- fixed w={w}")
        require(close(float(row["rest_energy"]), 16.0*E_REST), f"rest fixed w={w}")
        require(close(float(row["inertial_mass"]), 16.0*M_INERTIAL), f"mass fixed w={w}")
        barrier = float(row["scaled_barrier"])
        require(barrier > 0.0, f"positive barrier w={w}")
        grouped.setdefault((int(row["orientation"]), int(row["phase_axis"])), []).append((w, barrier))
    for orbit, values in grouped.items():
        ordered = [value for _, value in sorted(values)]
        require(all(b < a for a, b in zip(ordered, ordered[1:])),
                f"strict depinning orbit={orbit}")

    for row in slopes:
        es = float(row["energy_slope"])
        bs = float(row["barrier_slope"])
        ratio = float(row["width8_width4_energy_ratio"])
        require(abs(es) <= 0.25, "finite field-energy slope")
        require(-3.5 <= bs <= -2.5, "absolute barrier slope")
        require(0.8 <= ratio <= 1.2, "finite energy endpoint")

    # Dimensional prediction: unit E~w^5 and unit barrier~w^2, while
    # beta*q^2~w*w^-6=w^-5.  Thus physical E~w^0 and B~w^-3.
    require(5 + 1 - 6 == 0, "field-energy exponent prediction")
    require(2 + 1 - 6 == -3, "Peierls exponent prediction")
    print("FTD-0648 certificate complete")


if __name__ == "__main__":
    main()
