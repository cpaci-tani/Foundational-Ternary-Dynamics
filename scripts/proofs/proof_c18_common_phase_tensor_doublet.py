#!/usr/bin/env python3
"""Exact C18 common-phase symmetric-tensor doublet certificate.

The two common C4 quadratures on each antipodal C18 line are blocked against
the nine normalized line dyads.  Their exact covariance gives two independent
rank-six symmetric tensors.  Global C4 phase advance acts as the canonical
quarter-turn (Q, P) -> (-P, Q).

This is a carrier/type result only.  It does not derive a Poisson bracket,
constraints, a tensor kinetic pole, lensing, or gravity.
"""

from __future__ import annotations

from fractions import Fraction
from itertools import product

from proof_c18_uniform_token_blocking import (
    ALPHABET,
    C18_LINE_MOMENTS,
    covariance_matrix,
    invert_matrix,
    matrix_multiply,
    mean,
)
from proof_moore_bond_capacity_type_census import rational_rank


F = Fraction


def scaled_tensor_covariance(line_variance: Fraction) -> list[list[Fraction]]:
    normalization = F(1, len(C18_LINE_MOMENTS) ** 2)
    return [
        [
            line_variance
            * normalization
            * sum(
                (moment[row] * moment[column] for moment in C18_LINE_MOMENTS),
                start=F(0),
            )
            for column in range(6)
        ]
        for row in range(6)
    ]


def block_diagonal(left, right):
    size = len(left)
    zero = F(0)
    return [
        [
            left[row][column] if row < size and column < size
            else right[row - size][column - size] if row >= size and column >= size
            else zero
            for column in range(2 * size)
        ]
        for row in range(2 * size)
    ]


def main() -> None:
    checks = 0

    # Antipodal common quadratures use arithmetic means, avoiding irrational
    # normalization while retaining an exact finite-alphabet chart.
    pairs = tuple(product(ALPHABET, repeat=2))
    common_samples = tuple(
        (
            F(left[0] + right[0], 2),
            F(left[1] + right[1], 2),
            F(
                (1 - left[0] ** 2 - left[1] ** 2)
                + (1 - right[0] ** 2 - right[1] ** 2),
                2,
            ),
        )
        for left, right in pairs
    )
    common_mean = tuple(
        mean(tuple(sample[index] for sample in common_samples)) for index in range(3)
    )
    common_covariance = covariance_matrix(common_samples)
    assert common_mean == (F(0), F(0), F(1, 5))
    assert common_covariance == [
        [F(1, 5), F(0), F(0)],
        [F(0), F(1, 5), F(0)],
        [F(0), F(0), F(2, 25)],
    ]
    checks += 2

    q_covariance = scaled_tensor_covariance(F(1, 5))
    p_covariance = scaled_tensor_covariance(F(1, 5))
    expected = [
        [F(4, 810), F(1, 810), F(1, 810), F(0), F(0), F(0)],
        [F(1, 810), F(4, 810), F(1, 810), F(0), F(0), F(0)],
        [F(1, 810), F(1, 810), F(4, 810), F(0), F(0), F(0)],
        [F(0), F(0), F(0), F(1, 810), F(0), F(0)],
        [F(0), F(0), F(0), F(0), F(1, 810), F(0)],
        [F(0), F(0), F(0), F(0), F(0), F(1, 810)],
    ]
    assert q_covariance == p_covariance == expected
    assert rational_rank(q_covariance) == 6
    joint_covariance = block_diagonal(q_covariance, p_covariance)
    assert rational_rank(joint_covariance) == 12
    checks += 3

    inverse = invert_matrix(q_covariance)
    assert matrix_multiply(q_covariance, inverse) == [
        [F(int(row == column)) for column in range(6)] for row in range(6)
    ]
    checks += 1

    # Global multiplication by i sends (u, v) -> (-v, u), hence the blocked
    # tensor doublet obeys (Q, P) -> (-P, Q).  Verify the induced 12x12
    # complex structure and its preservation of the standard symplectic form.
    identity = [[F(int(row == column)) for column in range(6)] for row in range(6)]
    zero = [[F(0) for _column in range(6)] for _row in range(6)]
    complex_structure = [
        zero[row] + [-entry for entry in identity[row]] for row in range(6)
    ] + [
        identity[row] + zero[row] for row in range(6)
    ]
    symplectic = [
        zero[row] + identity[row] for row in range(6)
    ] + [
        [-entry for entry in identity[row]] + zero[row] for row in range(6)
    ]
    minus_identity_12 = [
        [F(-int(row == column)) for column in range(12)] for row in range(12)
    ]
    assert matrix_multiply(complex_structure, complex_structure) == minus_identity_12
    transpose_complex = [list(column) for column in zip(*complex_structure)]
    assert matrix_multiply(
        matrix_multiply(transpose_complex, symplectic), complex_structure
    ) == symplectic
    checks += 2

    # The covariance is invariant under the same quarter-turn.
    transpose = transpose_complex
    rotated_covariance = matrix_multiply(
        matrix_multiply(complex_structure, joint_covariance), transpose
    )
    assert rotated_covariance == joint_covariance
    checks += 1

    print(f"PASS: C18 common-phase tensor doublet ({checks} exact checks)")
    print("common quadrature covariance=diag(1/5, 1/5)")
    print("Cov(Q)=Cov(P)=1/810 * [[4,1,1],[1,4,1],[1,1,4]] plus shear I_3")
    print("joint tensor covariance rank=12")
    print("C4 action: (Q,P)->(-P,Q), J^2=-I, J^T Omega J=Omega")
    print("Open: native bracket, four first-class constraints, kinetic pole, lensing")


if __name__ == "__main__":
    main()
