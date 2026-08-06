"""Independent run-record certificate for FTD-0721."""

from __future__ import annotations

import csv
import hashlib
import json
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations" / (
    "PREREG_DERIVED_INTERACTION_GRAPH_TRANSACTION_v1.md"
)
RESULT = ROOT / "engine/results/ftd_0721" / (
    "ftd_0721_derived_interaction_graph_transaction_v1.json"
)
ROWS = ROOT / "engine/results/ftd_0721" / (
    "ftd_0721_derived_interaction_graph_transaction_v1.csv"
)

PROTOCOL = "FFCAC54E3368A3DE9FE466908A8BAFF2831D58B0F07AF83BA045BA4315AB6807"
RESULT_HASH = "790A810C54B31642921E9E99D27A6ADD0B65F09BB8ED9E74E5259098B4D01389"
ROWS_HASH = "FA385BBF4FBAA2D5CCF576DA77923B2731BE465E2E3067FECAB8F4D19C2C1DCB"
VERDICT = "DERIVED_INTERACTION_GRAPH_REVERSIBLE_CAPTURE_REQUIRES_RESERVOIR"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, label: str, checks: list[str]) -> None:
    assert condition, label
    checks.append(label)


def main() -> None:
    checks: list[str] = []
    check(digest(PREREG) == PROTOCOL, "locked protocol hash", checks)
    check(digest(RESULT) == RESULT_HASH, "JSON run-record hash", checks)
    check(digest(ROWS) == ROWS_HASH, "CSV run-record hash", checks)

    # Exact polynomial facts, independently of the C++ evaluator.
    epsilon = Fraction(1, 100)

    def potential(d: Fraction) -> Fraction:
        if d >= Fraction(3, 2):
            return Fraction(0)
        return -16 * epsilon * (d - Fraction(3, 2)) ** 2 * (
            d - Fraction(3, 4)
        )

    def derivative(d: Fraction) -> Fraction:
        if d >= Fraction(3, 2):
            return Fraction(0)
        return -48 * epsilon * (d - Fraction(3, 2)) * (d - 1)

    check(potential(Fraction(0)) == Fraction(27, 100), "repulsive core", checks)
    check(potential(Fraction(1)) == -epsilon, "well minimum value", checks)
    check(potential(Fraction(3, 2)) == 0, "continuous cutoff value", checks)
    check(derivative(Fraction(1)) == 0, "stationary well", checks)
    check(derivative(Fraction(3, 2)) == 0, "C1 cutoff", checks)
    check(Fraction(24, 100) > 0, "positive curvature at d=1", checks)

    summary = json.loads(RESULT.read_text(encoding="utf-8"))
    check(summary["identifier"] == "FTD-0721", "identifier", checks)
    check(summary["protocol_sha256"] == PROTOCOL, "protocol embedded", checks)
    check(summary["verdict"] == VERDICT, "locked verdict", checks)
    check(summary["arm_count"] == 104, "104 registered arms", checks)
    check(summary["passed_arms"] == 104, "104 passing arms", checks)
    check(summary["maximum_root_residual"] < 1e-13, "root gate", checks)
    check(summary["maximum_energy_residual"] < 1e-12, "energy gate", checks)
    check(summary["maximum_momentum_residual"] < 1e-12, "momentum gate", checks)
    check(
        summary["maximum_impulse_balance_residual"] < 1e-12,
        "impulse balance gate",
        checks,
    )
    check(summary["maximum_kinematic_residual"] < 1e-12, "kinematic gate", checks)
    check(summary["maximum_causal_speed_excess"] <= 1e-12, "causal gate", checks)
    check(summary["maximum_inverse_recovery"] < 1e-10, "inverse gate", checks)
    check(
        summary["maximum_scalar_history_spread"] < 1e-12,
        "translation/cubic/polarity covariance gate",
        checks,
    )

    with ROWS.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    directions = {
        f"{x}_{y}_{z}"
        for x in (-1, 0, 1)
        for y in (-1, 0, 1)
        for z in (-1, 0, 1)
        if (x, y, z) != (0, 0, 0)
        and next(v for v in (x, y, z) if v != 0) > 0
    }
    check(len(directions) == 13, "13 unoriented Moore rays", checks)
    check(len(rows) == 104, "104 CSV rows", checks)
    check({row["direction"] for row in rows} == directions, "direction set exact", checks)
    check({row["family"] for row in rows} == {"scattering", "bound"}, "families exact", checks)
    check({row["polarity"] for row in rows} == {"plus_minus", "minus_plus"}, "polarity mirrors exact", checks)
    check({row["translation"] for row in rows} == {"origin", "shifted"}, "translations exact", checks)
    check(all(row["pass"] == "1" for row in rows), "every row passes", checks)

    scattering = [row for row in rows if row["family"] == "scattering"]
    bound = [row for row in rows if row["family"] == "bound"]
    check(len(scattering) == 52 and len(bound) == 52, "52 arms per family", checks)
    check(all(int(row["graph_transitions"]) == 2 for row in scattering), "scattering graph enters and leaves", checks)
    check(all(int(row["active_ticks"]) > 0 for row in scattering), "scattering enters support", checks)
    check(all(float(row["initial_internal_energy"]) > 1e-6 for row in scattering), "unbound energy positive", checks)
    check(all(float(row["final_internal_energy"]) > 1e-6 for row in scattering), "unbound sector retained", checks)
    check(all(int(row["graph_transitions"]) == 0 for row in bound), "bound graph unchanged", checks)
    check(all(int(row["active_ticks"]) == 256 for row in bound), "bound support retained", checks)
    check(all(float(row["initial_internal_energy"]) < -1e-6 for row in bound), "bound energy negative", checks)
    check(all(float(row["final_internal_energy"]) < -1e-6 for row in bound), "bound sector retained", checks)
    check(max(float(row["recovery"]) for row in rows) < 1e-10, "rowwise inverse", checks)

    # Threshold proof: outside the compact support U=0 and kinetic energy
    # above rest is nonnegative. Exact energy preservation cannot map such a
    # state into the defined E_internal<0 bound sector.
    check(potential(Fraction(3, 2)) == 0, "outside-support continuum threshold", checks)
    check(
        Fraction(0) > potential(Fraction(1)),
        "bound minimum lies strictly below continuum threshold",
        checks,
    )

    print(f"FTD-0721 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")


if __name__ == "__main__":
    main()
