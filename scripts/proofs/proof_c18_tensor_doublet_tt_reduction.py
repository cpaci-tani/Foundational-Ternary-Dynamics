#!/usr/bin/env python3
"""Exact constraint and TT reduction for the C18 tensor doublet.

For each nonzero primitive integer wavevector in a bounded exhaustive domain,
the certificate constructs:
  * the three spatial gauge directions of Q,
  * one scalar Q constraint,
  * three divergence constraints on P,
  * one scalar gauge direction of P, and
  * the symmetric transverse-traceless projector.

It verifies the 12-dimensional (Q,P) phase space reduces to two canonical
configuration polarizations plus their two partners.  This is a conditional
kinematic target: the finite C18 action has not derived these constraints.
"""

from __future__ import annotations

from math import gcd

from sympy import Matrix, Rational, diag


FROBENIUS_GRAM = diag(1, 1, 1, 2, 2, 2)


def symmetric_matrix(vector: Matrix) -> Matrix:
    xx, yy, zz, xy, xz, yz = vector
    return Matrix([[xx, xy, xz], [xy, yy, yz], [xz, yz, zz]])


def symmetric_coordinates(matrix: Matrix) -> Matrix:
    return Matrix(
        [
            matrix[0, 0],
            matrix[1, 1],
            matrix[2, 2],
            matrix[0, 1],
            matrix[0, 2],
            matrix[1, 2],
        ]
    )


def spatial_gauge_matrix(k: Matrix) -> Matrix:
    columns = []
    for axis in range(3):
        xi = Matrix([int(index == axis) for index in range(3)])
        columns.append(symmetric_coordinates(k * xi.T + xi * k.T))
    return Matrix.hstack(*columns)


def scalar_q_constraint(k: Matrix) -> Matrix:
    kx, ky, kz = k
    k2 = (k.T * k)[0]
    return Matrix(
        [[
            kx * kx - k2,
            ky * ky - k2,
            kz * kz - k2,
            2 * kx * ky,
            2 * kx * kz,
            2 * ky * kz,
        ]]
    )


def momentum_constraint_matrix(k: Matrix) -> Matrix:
    kx, ky, kz = k
    return Matrix(
        [
            [kx, 0, 0, ky, kz, 0],
            [0, ky, 0, kx, 0, kz],
            [0, 0, kz, 0, kx, ky],
        ]
    )


def scalar_p_gauge(k: Matrix) -> Matrix:
    k2 = (k.T * k)[0]
    return symmetric_coordinates(k * k.T - k2 * Matrix.eye(3))


def tt_projector(k: Matrix) -> Matrix:
    k2 = (k.T * k)[0]
    transverse = Matrix.eye(3) - k * k.T / k2
    columns = []
    for coordinate in range(6):
        basis = Matrix([int(index == coordinate) for index in range(6)])
        tensor = symmetric_matrix(basis)
        transverse_tensor = transverse * tensor * transverse
        trace_transverse = (transverse * tensor).trace()
        projected = transverse_tensor - Rational(1, 2) * transverse * trace_transverse
        columns.append(symmetric_coordinates(projected))
    return Matrix.hstack(*columns)


def primitive_wavevectors(radius: int) -> tuple[Matrix, ...]:
    vectors = []
    for kx in range(-radius, radius + 1):
        for ky in range(-radius, radius + 1):
            for kz in range(-radius, radius + 1):
                if (kx, ky, kz) == (0, 0, 0):
                    continue
                divisor = gcd(gcd(abs(kx), abs(ky)), abs(kz))
                if divisor == 1:
                    vectors.append(Matrix([kx, ky, kz]))
    return tuple(vectors)


def main() -> None:
    checks = 0
    vectors = primitive_wavevectors(2)
    assert len(vectors) == 98
    checks += 1

    complex_structure = Matrix.vstack(
        Matrix.hstack(Matrix.zeros(6), -Matrix.eye(6)),
        Matrix.hstack(Matrix.eye(6), Matrix.zeros(6)),
    )

    for k in vectors:
        q_gauge = spatial_gauge_matrix(k)
        q_constraint = scalar_q_constraint(k)
        p_constraint = momentum_constraint_matrix(k)
        p_gauge = scalar_p_gauge(k)
        projector = tt_projector(k)

        assert q_gauge.rank() == 3
        assert q_constraint.rank() == 1
        assert q_constraint * q_gauge == Matrix.zeros(1, 3)
        assert p_constraint.rank() == 3
        assert p_constraint * p_gauge == Matrix.zeros(3, 1)
        checks += 5

        assert projector.rank() == 2
        assert projector * projector == projector
        assert projector * q_gauge == Matrix.zeros(6, 3)
        assert projector * p_gauge == Matrix.zeros(6, 1)
        assert q_constraint * projector == Matrix.zeros(1, 6)
        assert p_constraint * projector == Matrix.zeros(3, 6)
        assert projector.T * FROBENIUS_GRAM == FROBENIUS_GRAM * projector
        checks += 7

        joint_projector = diag(projector, projector)
        assert joint_projector.rank() == 4
        assert joint_projector * complex_structure == complex_structure * joint_projector
        checks += 2

        # Dimension ledger: one Q constraint plus three Q gauge directions,
        # and three P constraints plus one P gauge direction.
        assert 6 - q_constraint.rank() - q_gauge.rank() == 2
        assert 6 - p_constraint.rank() - p_gauge.rank() == 2
        checks += 2

    # Axis representative exposes the usual plus/cross coordinates exactly.
    axis = Matrix([0, 0, 1])
    projector = tt_projector(axis)
    plus = Matrix([1, -1, 0, 0, 0, 0])
    cross = Matrix([0, 0, 0, 1, 0, 0])
    assert projector * plus == plus
    assert projector * cross == cross
    assert Matrix.hstack(plus, cross).rank() == 2
    checks += 3

    print(f"PASS: C18 tensor-doublet TT reduction ({checks} exact checks)")
    print(f"primitive nonzero wavevectors checked={len(vectors)}")
    print("Q: 6 - one scalar constraint - three spatial gauge directions = 2")
    print("P: 6 - three momentum constraints - one scalar gauge direction = 2")
    print("joint TT projector rank=4 and commutes with native C4 complex structure")
    print("Open: action-derived constraints/algebra, kinetic pole, universal source, lensing")


if __name__ == "__main__":
    main()
