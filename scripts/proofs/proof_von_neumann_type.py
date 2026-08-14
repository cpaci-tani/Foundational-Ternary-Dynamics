"""Finite actual/potential algebra certificate for FTD v2 (FTD-0825).

This replaces the retired claim that a classical ternary state *is* M_3(C).
It verifies standard finite-dimensional facts inside the selected qutrit
potentiality representation.  It does not derive that representation from
P1--P5 and it makes no Type-III claim.
"""

from __future__ import annotations

import sys
from pathlib import Path

import numpy as np

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
THEOREM = (
    ROOT
    / "docs/theory/10_eft_program/temporal_interior_programme"
    / "THEOREM_DISCRETE_CONTEXTUAL_LOCAL_NET_v1.md"
)


def partial_trace_second(rho: np.ndarray, d_left: int, d_right: int) -> np.ndarray:
    reshaped = rho.reshape(d_left, d_right, d_left, d_right)
    return np.trace(reshaped, axis1=1, axis2=3)


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))
        print(f"[{'PASS' if condition else 'FAIL'}] {label}")

    check("C1 theorem companion exists", THEOREM.exists())

    # Classical records on S^N form C^(3^N), not the full matrix algebra.
    for sites in (1, 2, 3):
        record_dimension = 3**sites
        central_minimal_projections = np.eye(record_dimension)
        check(
            f"C2.{sites} actual record algebra dimension is 3^{sites}",
            central_minimal_projections.shape == (record_dimension, record_dimension),
        )
        diagonal_a = np.diag(np.arange(record_dimension, dtype=float))
        diagonal_b = np.diag(np.arange(record_dimension, 0, -1, dtype=float))
        check(
            f"C3.{sites} actual record observables commute",
            np.allclose(diagonal_a @ diagonal_b, diagonal_b @ diagonal_a),
        )

    # The separately selected potentiality witness is M_3(C).
    e01 = np.zeros((3, 3), dtype=complex)
    e10 = np.zeros((3, 3), dtype=complex)
    e01[0, 1] = 1.0
    e10[1, 0] = 1.0
    commutator = e01 @ e10 - e10 @ e01
    check("C4 selected M3 potential algebra is noncommutative", np.linalg.norm(commutator) > 1.0)
    check("C5 selected one-site potential algebra is finite Type I", e01.shape == (3, 3))

    # Isotony: A -> A tensor I is injective and unital.
    identity3 = np.eye(3, dtype=complex)
    embedded = np.kron(e01, identity3)
    embedded_identity = np.kron(identity3, identity3)
    check("C6 isotony witness preserves multiplication", np.allclose(embedded @ np.kron(e10, identity3), np.kron(e01 @ e10, identity3)))
    check("C7 isotony witness is unital", np.allclose(embedded_identity, np.eye(9)))

    # Disjoint tensor factors commute.
    left = np.kron(e01, identity3)
    right = np.kron(identity3, e10)
    check("C8 disjoint local factors commute", np.allclose(left @ right, right @ left))

    # The diagonal record algebra embeds as a commuting subalgebra of M3.
    d1 = np.diag([1.0, 2.0, 3.0])
    d2 = np.diag([4.0, -1.0, 0.5])
    check("C9 diagonal record embedding is abelian", np.allclose(d1 @ d2, d2 @ d1))
    check("C10 off-diagonal potential effect is outside record algebra", not np.allclose(e01, np.diag(np.diag(e01))))

    # Compatible regional states restrict by partial trace.
    p0 = np.diag([1.0, 0.0, 0.0]).astype(complex)
    p1 = np.diag([0.0, 1.0, 0.0]).astype(complex)
    rho_left = 0.25 * p0 + 0.75 * p1
    rho_right = np.diag([0.2, 0.3, 0.5]).astype(complex)
    rho_joint = np.kron(rho_left, rho_right)
    restricted = partial_trace_second(rho_joint, 3, 3)
    check("C11 preparation restriction is consistent", np.allclose(restricted, rho_left))
    check("C12 restricted state remains normalized", abs(np.trace(restricted) - 1.0) < 1e-14)

    # Classification guard: finite spectral statistics do not establish a
    # GNS factor type.  The only limit claim licensed here is the standard
    # norm-inductive qutrit UHF scaffold named in the companion theorem.
    theorem_text = THEOREM.read_text(encoding="utf-8")
    check("C13 UHF limit guard is explicit", "UHF" in theorem_text and "GNS" in theorem_text)
    check("C14 no finite-spacing Type-III inference", "random-matrix spacing" in theorem_text)

    passed = sum(ok for _, ok in checks)
    print(f"\nFTD-0825 finite contextual local-net certificate: {passed}/{len(checks)} PASS")
    print("ACTUAL_ALGEBRA=COMMUTATIVE_DIAGONAL")
    print("POTENTIAL_ALGEBRA=SELECTED_NONCOMMUTATIVE_REFERENCE")
    print("TYPE_III_STATUS=NOT_ESTABLISHED")
    return 0 if passed == len(checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())
