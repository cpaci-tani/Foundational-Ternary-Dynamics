"""Independent certificate for FTD-0602.

This script does not call the C++ matter solver.  It proves the periodic
minimum-energy decomposition on an exact rational lattice, verifies the
immutable protocol hash, and audits the run-of-record against the locked
initializer, common-action, force-sign, inverse, and momentum gates.
"""

from __future__ import annotations

import hashlib
import json
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_MINIMUM_ENERGY_NEUTRAL_PAIR_FORCE_SIGN_v1.md"
)
RESULT = ROOT / (
    "engine/results/ftd_0602/ftd_0602_minimum_energy_force_sign_v1.json"
)
EXPECTED_PROTOCOL = (
    "1ECB8957CCBA4AE5770FDB310E883357F745418DD36AD30CD5C7E7D35366F341"
)

L = 3
Scalar = list[Fraction]
Vector = tuple[Scalar, Scalar, Scalar]


def idx(x: int, y: int, z: int) -> int:
    return ((x % L) * L + (y % L)) * L + (z % L)


def gradient(phi: Scalar) -> Vector:
    result = ([Fraction(0)] * (L**3), [Fraction(0)] * (L**3),
              [Fraction(0)] * (L**3))
    for x in range(L):
        for y in range(L):
            for z in range(L):
                i = idx(x, y, z)
                result[0][i] = phi[i] - phi[idx(x + 1, y, z)]
                result[1][i] = phi[i] - phi[idx(x, y + 1, z)]
                result[2][i] = phi[i] - phi[idx(x, y, z + 1)]
    return result


def divergence(face: Vector) -> Scalar:
    result = [Fraction(0)] * (L**3)
    for x in range(L):
        for y in range(L):
            for z in range(L):
                i = idx(x, y, z)
                result[i] = (
                    face[0][i] - face[0][idx(x - 1, y, z)]
                    + face[1][i] - face[1][idx(x, y - 1, z)]
                    + face[2][i] - face[2][idx(x, y, z - 1)]
                )
    return result


def curl(edge: Vector) -> Vector:
    result = ([Fraction(0)] * (L**3), [Fraction(0)] * (L**3),
              [Fraction(0)] * (L**3))
    for x in range(L):
        for y in range(L):
            for z in range(L):
                i = idx(x, y, z)
                xm, ym, zm = (
                    idx(x - 1, y, z), idx(x, y - 1, z), idx(x, y, z - 1)
                )
                result[0][i] = edge[2][i] - edge[2][ym] - edge[1][i] + edge[1][zm]
                result[1][i] = edge[0][i] - edge[0][zm] - edge[2][i] + edge[2][xm]
                result[2][i] = edge[1][i] - edge[1][xm] - edge[0][i] + edge[0][ym]
    return result


def inner(lhs: Vector, rhs: Vector) -> Fraction:
    return sum(
        (lhs[axis][i] * rhs[axis][i]
         for axis in range(3) for i in range(L**3)),
        Fraction(0),
    )


def add(lhs: Vector, rhs: Vector) -> Vector:
    return tuple(
        [lhs[axis][i] + rhs[axis][i] for i in range(L**3)]
        for axis in range(3)
    )  # type: ignore[return-value]


def exact_minimum_energy_certificate() -> dict[str, object]:
    phi = [Fraction(((11 * i + 7) % 19) - 9, 13) for i in range(L**3)]
    edge: Vector = tuple(
        [Fraction(((17 * i + 5 * axis + 3) % 23) - 11, 29)
         for i in range(L**3)]
        for axis in range(3)
    )  # type: ignore[assignment]
    longitudinal = gradient(phi)
    transverse = curl(edge)
    combined = add(longitudinal, transverse)
    div_transverse = divergence(transverse)
    orthogonality = inner(longitudinal, transverse)
    energy_gap = (inner(combined, combined) - inner(longitudinal, longitudinal)) / 2
    transverse_energy = inner(transverse, transverse) / 2
    return {
        "div_curl_residual": max(abs(value) for value in div_transverse),
        "longitudinal_transverse_inner_product": orthogonality,
        "energy_decomposition_residual": energy_gap - transverse_energy,
        "transverse_energy": transverse_energy,
    }


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def main() -> int:
    exact = exact_minimum_energy_certificate()
    observed_hash = protocol_hash()
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL
        == record["protocol_sha256"],
        "exact_div_curl": exact["div_curl_residual"] == 0,
        "exact_orthogonality":
            exact["longitudinal_transverse_inner_product"] == 0,
        "exact_energy_decomposition":
            exact["energy_decomposition_residual"] == 0
            and exact["transverse_energy"] > 0,
        "locked_arm_count": record["forward_arms"] == 12
            and record["reverse_arms"] == 12,
        "minimum_energy_initializer": record["initializer_pass"]
            and record["initializer_solver_residual"] <= 1e-13
            and record["initializer_gauss_residual"] <= 1e-12
            and record["initializer_curl_residual"] <= 1e-12,
        "transverse_challenge": record["minimum_control_pass"]
            and record["transverse_gauss_residual"] <= 1e-12
            and record["transverse_energy_gap"] > 0,
        "common_action_and_symmetry": record["common_pass"]
            and record["worst_common_gate"] <= 1e-12
            and record["worst_inverse"] <= 1e-10
            and record["worst_symmetry"] <= 1e-12,
        "attraction_restored": record["sign_pass"]
            and record["inward_impulse"] > 1e-10
            and record["separation_after_one_step"] < record["separation_before"]
            and record["separation_after_repeated"] < record["separation_before"],
        "repeated_reversibility": record["repeated_pass"]
            and record["repeated_forward_steps"] == 16
            and record["repeated_reverse_steps"] == 16
            and record["state_recovery"] <= 1e-8
            and record["energy_drift"] <= 1e-9,
        "momentum_channel_missing": not record["momentum_pass"]
            and record["worst_pseudomomentum_defect"] > 1e-12,
        "verdict": record["verdict"]
            == "MINIMUM_ENERGY_ATTRACTION_RESTORED_MOMENTUM_CHANNEL_MISSING",
        "production_unchanged": record["production_changed"] is False,
    }
    report = {
        "ftd_id": "FTD-0602",
        "protocol_sha256": observed_hash,
        "exact_periodic_lattice_size": L,
        "exact_div_curl_residual": str(exact["div_curl_residual"]),
        "exact_longitudinal_transverse_inner_product": str(
            exact["longitudinal_transverse_inner_product"]
        ),
        "exact_energy_decomposition_residual": str(
            exact["energy_decomposition_residual"]
        ),
        "cxx_verdict": record["verdict"],
        "inward_impulse": record["inward_impulse"],
        "separation_after_repeated": record["separation_after_repeated"],
        "worst_pseudomomentum_defect": record["worst_pseudomomentum_defect"],
        "electrostatic_force_sign_licensed_for_locked_pair": True,
        "isolated_total_momentum_claim_licensed": False,
        "checks": checks,
        "certificate_pass": all(checks.values()),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["certificate_pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
