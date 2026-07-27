"""Independent exact certificate for FTD-0601.

This script does not call the C++ solver.  It checks the two independent
trimer binding identities over exact rationals, verifies the immutable
protocol hash, and audits the run-of-record against the preregistered
common-action, attraction, momentum, symmetry, and repeated-state gates.
"""

from __future__ import annotations

import hashlib
import json
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_CLOSED_NEUTRAL_TRIMER_PAIR_DYNAMICS_v1.md"
)
RESULT = ROOT / "engine/results/ftd_0601/ftd_0601_closed_neutral_pair_v1.json"
EXPECTED_PROTOCOL = (
    "89979BF190B8A5FD36DF6642356E455F13ED01C9A2C42E20777B150996C1C1F3"
)


Vector = tuple[Fraction, Fraction, Fraction]


def dot(lhs: Vector, rhs: Vector) -> Fraction:
    return sum((a * b for a, b in zip(lhs, rhs)), Fraction(0))


def add(lhs: Vector, rhs: Vector) -> Vector:
    return tuple(a + b for a, b in zip(lhs, rhs))  # type: ignore[return-value]


def sub(lhs: Vector, rhs: Vector) -> Vector:
    return tuple(a - b for a, b in zip(lhs, rhs))  # type: ignore[return-value]


def scale(value: Fraction, vector: Vector) -> Vector:
    return tuple(value * entry for entry in vector)  # type: ignore[return-value]


def group_energy(points: tuple[Vector, Vector, Vector]) -> Fraction:
    result = Fraction(0)
    for a in range(3):
        for b in range(a + 1, 3):
            displacement = sub(points[a], points[b])
            u = dot(displacement, displacement) - 2
            result += u * u / 4
    return result


def group_impulses(
    start: tuple[Vector, Vector, Vector],
    end: tuple[Vector, Vector, Vector],
) -> tuple[Vector, Vector, Vector]:
    impulses: list[Vector] = [
        (Fraction(0), Fraction(0), Fraction(0)) for _ in range(3)
    ]
    for a in range(3):
        for b in range(a + 1, 3):
            d0 = sub(start[a], start[b])
            d1 = sub(end[a], end[b])
            u0 = dot(d0, d0) - 2
            u1 = dot(d1, d1) - 2
            gradient = scale((u0 + u1) / 4, add(d0, d1))
            impulses[a] = sub(impulses[a], gradient)
            impulses[b] = add(impulses[b], gradient)
    return tuple(impulses)  # type: ignore[return-value]


def exact_identity_campaign() -> tuple[int, Fraction, Fraction]:
    rest_a: tuple[Vector, Vector, Vector] = (
        (Fraction(0), Fraction(0), Fraction(0)),
        (Fraction(1), Fraction(1), Fraction(0)),
        (Fraction(1), Fraction(0), Fraction(1)),
    )
    rest_b: tuple[Vector, Vector, Vector] = (
        (Fraction(0), Fraction(0), Fraction(0)),
        (Fraction(-1), Fraction(-1), Fraction(0)),
        (Fraction(-1), Fraction(0), Fraction(-1)),
    )
    assert group_energy(rest_a) == group_energy(rest_b) == 0
    fixtures = 0
    worst_work = Fraction(0)
    worst_impulse = Fraction(0)
    for seed in range(1, 65):
        for group_index, rest in enumerate((rest_a, rest_b)):
            displacements: tuple[Vector, Vector, Vector] = tuple(
                tuple(
                    Fraction(
                        ((seed + 7 * group_index + 3 * a + 5 * axis) % 13) - 6,
                        240,
                    )
                    for axis in range(3)
                )
                for a in range(3)
            )  # type: ignore[assignment]
            end = tuple(
                add(rest[a], displacements[a]) for a in range(3)
            )  # type: ignore[assignment]
            impulses = group_impulses(rest, end)
            work = sum(
                (dot(displacements[a], impulses[a]) for a in range(3)),
                Fraction(0),
            )
            work_residual = group_energy(end) - group_energy(rest) + work
            impulse_sum = tuple(
                sum((impulses[a][axis] for a in range(3)), Fraction(0))
                for axis in range(3)
            )
            worst_work = max(worst_work, abs(work_residual))
            worst_impulse = max(
                worst_impulse, *(abs(value) for value in impulse_sum)
            )
            fixtures += 1
    return fixtures, worst_work, worst_impulse


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def main() -> int:
    fixtures, work_residual, impulse_residual = exact_identity_campaign()
    observed_hash = protocol_hash()
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    symmetry = max(
        record["worst_translation_residual"],
        record["worst_rotation_residual"],
        record["worst_permutation_residual"],
        record["worst_charge_conjugation_residual"],
    )
    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL
        == record["protocol_sha256"],
        "exact_binding_work": work_residual == 0,
        "exact_binding_impulse_sum": impulse_residual == 0,
        "locked_one_step_count": record["one_step_forward_arms"] == 20
        and record["one_step_reverse_arms"] == 20,
        "common_action_gate": record["common_one_step_pass"]
        and record["worst_one_step_gate"] <= 1e-12,
        "state_only_inverse": record["worst_one_step_inverse"] <= 1e-10,
        "symmetry": symmetry <= 1e-12,
        "repeated_gate": record["repeated_run"] and record["repeated_pass"]
        and record["repeated_forward_steps"] == 48
        and record["repeated_reverse_steps"] == 48
        and record["site_hops"] > 0
        and record["repeated_state_recovery"] <= 1e-8
        and record["repeated_energy_drift"] <= 1e-9,
        "bound_shape": record["minimum_internal_pair_distance"] >= 1.35
        and record["maximum_internal_pair_distance"] <= 1.48,
        "no_stationary_compensator":
            record["stationary_compensator_present"] is False,
        "isolated_momentum_gate_fails": record["momentum_pass"] is False
        and record["worst_pseudomomentum_defect"] > 1e-12,
        "locked_inward_gate_fails": record["inward_response_pass"] is False
        and record["rest_inward_impulse"] < -1e-10
        and record["rest_separation_after_one_step"]
        > record["rest_separation_before"],
        "later_return_is_disclosed": record["rest_separation_after_repeated"]
        < record["rest_separation_before"],
        "verdict": record["verdict"]
        == "NEUTRAL_TRIMER_PAIR_NONATTRACTIVE_SELECTED_DYNAMICS",
        "production_unchanged": record["production_changed"] is False,
    }
    report = {
        "ftd_id": "FTD-0601",
        "protocol_sha256": observed_hash,
        "exact_rational_group_fixtures": fixtures,
        "exact_binding_work_residual": str(work_residual),
        "exact_binding_impulse_sum_residual": str(impulse_residual),
        "cxx_verdict": record["verdict"],
        "worst_one_step_gate": record["worst_one_step_gate"],
        "worst_one_step_inverse": record["worst_one_step_inverse"],
        "repeated_state_recovery": record["repeated_state_recovery"],
        "repeated_energy_drift": record["repeated_energy_drift"],
        "rest_inward_impulse": record["rest_inward_impulse"],
        "worst_pseudomomentum_defect": record["worst_pseudomomentum_defect"],
        "isolated_total_momentum_claim_licensed": False,
        "electrostatic_attraction_claim_licensed": False,
        "checks": checks,
        "certificate_pass": all(checks.values()),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["certificate_pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

