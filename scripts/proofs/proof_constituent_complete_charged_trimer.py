"""Independent exact checks for the FTD-0600 charged-trimer transaction.

This verifier does not call the C++ solver.  It proves the selected pair
discrete-gradient work and impulse identities over exact rationals, verifies
the locked protocol hash, and audits the versioned C++ run record against the
preregistered verdict gates.
"""

from __future__ import annotations

import hashlib
import json
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_CONSTITUENT_COMPLETE_CHARGED_TRIMER_TRANSACTION_v1.md"
)
RESULT = ROOT / "engine/results/ftd_0600/ftd_0600_charged_trimer_v1.json"
EXPECTED_PROTOCOL = (
    "F24CC0BFBF0741B0F1A07DCE3B719EA6452E3DC81BB0E9F76013F211D25F6328"
)


def dot(lhs: tuple[Fraction, ...], rhs: tuple[Fraction, ...]) -> Fraction:
    return sum((a * b for a, b in zip(lhs, rhs)), Fraction(0))


def sub(lhs: tuple[Fraction, ...], rhs: tuple[Fraction, ...]) -> tuple[Fraction, ...]:
    return tuple(a - b for a, b in zip(lhs, rhs))


def add(lhs: tuple[Fraction, ...], rhs: tuple[Fraction, ...]) -> tuple[Fraction, ...]:
    return tuple(a + b for a, b in zip(lhs, rhs))


def scale(value: Fraction, vector: tuple[Fraction, ...]) -> tuple[Fraction, ...]:
    return tuple(value * entry for entry in vector)


def binding_energy(points: tuple[tuple[Fraction, ...], ...]) -> Fraction:
    energy = Fraction(0)
    for a in range(3):
        for b in range(a + 1, 3):
            displacement = sub(points[a], points[b])
            u = dot(displacement, displacement) - 2
            energy += u * u / 4
    return energy


def binding_impulses(
    start: tuple[tuple[Fraction, ...], ...],
    end: tuple[tuple[Fraction, ...], ...],
) -> tuple[tuple[Fraction, ...], ...]:
    impulses = [(Fraction(0), Fraction(0), Fraction(0)) for _ in range(3)]
    for a in range(3):
        for b in range(a + 1, 3):
            d0 = sub(start[a], start[b])
            d1 = sub(end[a], end[b])
            u0 = dot(d0, d0) - 2
            u1 = dot(d1, d1) - 2
            gradient = scale((u0 + u1) / 4, add(d0, d1))
            impulses[a] = sub(impulses[a], gradient)
            impulses[b] = add(impulses[b], gradient)
    return tuple(impulses)


def exact_identity_campaign() -> tuple[int, Fraction, Fraction]:
    fixtures = 0
    worst_work = Fraction(0)
    worst_impulse = Fraction(0)
    rest = (
        (Fraction(0), Fraction(0), Fraction(0)),
        (Fraction(1), Fraction(1), Fraction(0)),
        (Fraction(1), Fraction(0), Fraction(1)),
    )
    assert binding_energy(rest) == 0
    for seed in range(1, 65):
        displacements = tuple(
            tuple(Fraction(((seed + 3 * a + 5 * axis) % 11) - 5, 200)
                  for axis in range(3))
            for a in range(3)
        )
        end = tuple(add(rest[a], displacements[a]) for a in range(3))
        impulses = binding_impulses(rest, end)
        work = sum(
            (dot(displacements[a], impulses[a]) for a in range(3)),
            Fraction(0),
        )
        work_residual = binding_energy(end) - binding_energy(rest) + work
        impulse_sum = tuple(
            sum((impulses[a][axis] for a in range(3)), Fraction(0))
            for axis in range(3)
        )
        worst_work = max(worst_work, abs(work_residual))
        worst_impulse = max(worst_impulse, *(abs(value) for value in impulse_sum))
        fixtures += 1
    return fixtures, worst_work, worst_impulse


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    marker = b"`protocol_sha256="
    prefix = raw[: raw.index(marker)]
    return hashlib.sha256(prefix).hexdigest().upper()


def main() -> int:
    fixtures, work_residual, impulse_residual = exact_identity_campaign()
    observed_hash = protocol_hash()
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL
        == record["protocol_sha256"],
        "exact_binding_work": work_residual == 0,
        "exact_binding_impulse_sum": impulse_residual == 0,
        "locked_one_step_count": record["one_step_forward_arms"] == 32
        and record["one_step_reverse_arms"] == 32,
        "one_step_gate": record["one_step_pass"]
        and record["worst_one_step_gate"] <= 1e-12,
        "state_only_inverse": record["worst_one_step_inverse"] <= 1e-10,
        "symmetry": max(
            record["worst_translation_residual"],
            record["worst_rotation_residual"],
            record["worst_permutation_residual"],
        ) <= 1e-12,
        "repeated_gate": record["repeated_run"] and record["repeated_pass"]
        and record["repeated_forward_steps"] == 64
        and record["repeated_reverse_steps"] == 64
        and record["site_hops"] > 0
        and record["repeated_state_recovery"] <= 1e-8
        and record["repeated_energy_drift"] <= 1e-9,
        "bound_shape": 1.0 < record["minimum_pair_distance"]
        <= record["maximum_pair_distance"] < 2.0,
        "pseudomomentum_defect_disclosed":
            record["worst_pseudomomentum_defect"] > 1e-12,
        "verdict": record["verdict"]
        == "CHARGED_TRIMER_COMMON_ACTION_CONSTRUCTIVE",
        "production_unchanged": record["production_changed"] is False,
    }
    report = {
        "ftd_id": "FTD-0600",
        "protocol_sha256": observed_hash,
        "exact_rational_binding_fixtures": fixtures,
        "exact_binding_work_residual": str(work_residual),
        "exact_binding_impulse_sum_residual": str(impulse_residual),
        "cxx_verdict": record["verdict"],
        "worst_one_step_gate": record["worst_one_step_gate"],
        "worst_one_step_inverse": record["worst_one_step_inverse"],
        "repeated_state_recovery": record["repeated_state_recovery"],
        "repeated_energy_drift": record["repeated_energy_drift"],
        "worst_pseudomomentum_defect": record["worst_pseudomomentum_defect"],
        "isolated_total_momentum_claim_licensed": False,
        "checks": checks,
        "certificate_pass": all(checks.values()),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["certificate_pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
