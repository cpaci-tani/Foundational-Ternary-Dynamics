#!/usr/bin/env python3
"""Exact bare-blocking certificate for directed C18 C4-plus-blank tokens.

The microscopic reference measure is uniform on the five-state alphabet for
each directed channel.  Streaming is a one-hop permutation.  This script
derives the exact alphabet covariance, antipodal common/relative covariance,
isotropic current covariance, and the positive rank-six capacity-tensor
covariance/Hessian using rational arithmetic.

It is a bare-vacuum blocking result.  It does not derive interacting matter,
the full production action, lensing, a tensor pole, or a physical coupling.
"""

from __future__ import annotations

from fractions import Fraction
from itertools import product

from proof_moore_bond_capacity_type_census import rational_rank


F = Fraction
ALPHABET = ((0, 0), (1, 0), (0, 1), (-1, 0), (0, -1))

# Symmetric normalized outer products in coordinates xx, yy, zz, xy, xz, yz.
C18_LINE_MOMENTS = (
    (F(1), F(0), F(0), F(0), F(0), F(0)),
    (F(0), F(1), F(0), F(0), F(0), F(0)),
    (F(0), F(0), F(1), F(0), F(0), F(0)),
    (F(1, 2), F(1, 2), F(0), F(1, 2), F(0), F(0)),
    (F(1, 2), F(1, 2), F(0), F(-1, 2), F(0), F(0)),
    (F(1, 2), F(0), F(1, 2), F(0), F(1, 2), F(0)),
    (F(1, 2), F(0), F(1, 2), F(0), F(-1, 2), F(0)),
    (F(0), F(1, 2), F(1, 2), F(0), F(0), F(1, 2)),
    (F(0), F(1, 2), F(1, 2), F(0), F(0), F(-1, 2)),
)


def mean(values: tuple[Fraction, ...]) -> Fraction:
    return sum(values, start=F(0)) / len(values)


def covariance_matrix(samples: tuple[tuple[Fraction, ...], ...]) -> list[list[Fraction]]:
    width = len(samples[0])
    means = [mean(tuple(sample[index] for sample in samples)) for index in range(width)]
    return [
        [
            mean(
                tuple(
                    (sample[row] - means[row]) * (sample[column] - means[column])
                    for sample in samples
                )
            )
            for column in range(width)
        ]
        for row in range(width)
    ]


def invert_matrix(matrix: list[list[Fraction]]) -> list[list[Fraction]]:
    size = len(matrix)
    augmented = [
        list(row) + [F(int(i == j)) for j in range(size)]
        for i, row in enumerate(matrix)
    ]
    for column in range(size):
        pivot = next(row for row in range(column, size) if augmented[row][column] != 0)
        augmented[column], augmented[pivot] = augmented[pivot], augmented[column]
        scale = augmented[column][column]
        augmented[column] = [value / scale for value in augmented[column]]
        for row in range(size):
            if row == column:
                continue
            factor = augmented[row][column]
            if factor != 0:
                augmented[row] = [
                    value - factor * pivot_value
                    for value, pivot_value in zip(augmented[row], augmented[column])
                ]
    return [row[size:] for row in augmented]


def matrix_multiply(
    left: list[list[Fraction]],
    right: list[list[Fraction]],
) -> list[list[Fraction]]:
    return [
        [
            sum(
                (left[row][inner] * right[inner][column] for inner in range(len(right))),
                start=F(0),
            )
            for column in range(len(right[0]))
        ]
        for row in range(len(left))
    ]


def tensor_covariance() -> list[list[Fraction]]:
    # A line capacity is the average of two independent directed capacities.
    # Single-channel c has variance 4/25, hence the antipodal average has 2/25.
    capacity_variance = F(2, 25)
    normalization = F(1, len(C18_LINE_MOMENTS) ** 2)
    return [
        [
            capacity_variance
            * normalization
            * sum(
                (moment[row] * moment[column] for moment in C18_LINE_MOMENTS),
                start=F(0),
            )
            for column in range(6)
        ]
        for row in range(6)
    ]


def quadratic_form(matrix: list[list[Fraction]], vector: tuple[Fraction, ...]) -> Fraction:
    return sum(
        (
            vector[row]
            * matrix[row][column]
            * vector[column]
            for row in range(len(vector))
            for column in range(len(vector))
        ),
        start=F(0),
    )


def main() -> None:
    checks = 0

    samples = tuple(
        (F(u), F(v), F(1 - u * u - v * v))
        for u, v in ALPHABET
    )
    alphabet_means = tuple(mean(tuple(sample[index] for sample in samples)) for index in range(3))
    alphabet_covariance = covariance_matrix(samples)
    assert alphabet_means == (F(0), F(0), F(1, 5))
    assert alphabet_covariance == [
        [F(2, 5), F(0), F(0)],
        [F(0), F(2, 5), F(0)],
        [F(0), F(0), F(4, 25)],
    ]
    checks += 2

    # For two independent antipodal directed channels, the normalized common
    # and relative phase coordinates are exactly uncorrelated with equal
    # variance.  We avoid irrational sqrt(2) by checking sums/differences,
    # whose variances are both 4/5; division by sqrt(2) returns 2/5.
    directed_pairs = tuple(product(samples, repeat=2))
    pair_samples = tuple(
        (
            left[0] + right[0],
            left[0] - right[0],
            left[1] + right[1],
            left[1] - right[1],
            (left[2] + right[2]) / 2,
        )
        for left, right in directed_pairs
    )
    pair_covariance = covariance_matrix(pair_samples)
    assert [pair_covariance[index][index] for index in range(5)] == [
        F(4, 5),
        F(4, 5),
        F(4, 5),
        F(4, 5),
        F(2, 25),
    ]
    assert all(
        pair_covariance[row][column] == 0
        for row in range(5)
        for column in range(5)
        if row != column
    )
    checks += 2

    # Exact one-hop streaming is a permutation for every periodic ring size.
    for length in range(2, 33):
        mapping = tuple((index + 1) % length for index in range(length))
        assert set(mapping) == set(range(length))
        position = 0
        for _ in range(length):
            position = mapping[position]
        assert position == 0
        checks += 2

    # The nine normalized C18 line moments sum to 3 I, so a signed relative
    # phase current averaged over lines has covariance 4/135 times I.
    second_moment_sum = [
        sum((moment[index] for moment in C18_LINE_MOMENTS), start=F(0))
        for index in range(6)
    ]
    assert second_moment_sum == [F(3), F(3), F(3), F(0), F(0), F(0)]
    current_covariance = F(4, 5) * F(3, 9 * 9)
    assert current_covariance == F(4, 135)
    checks += 2

    covariance = tensor_covariance()
    assert rational_rank(covariance) == 6
    inverse = invert_matrix(covariance)
    identity = matrix_multiply(covariance, inverse)
    assert identity == [
        [F(int(row == column)) for column in range(6)]
        for row in range(6)
    ]
    checks += 2

    # Cubic irreducible test directions: trace, two diagonal shear directions,
    # and three off-diagonal shear directions.  Every quadratic coefficient is
    # positive.  Cubic symmetry makes each T2g coefficient equal; the Eg basis
    # is non-orthonormal, so report its two exact directional values.
    irrep_directions = {
        "trace": (F(1), F(1), F(1), F(0), F(0), F(0)),
        "eg_xy": (F(1), F(-1), F(0), F(0), F(0), F(0)),
        "eg_z": (F(1), F(1), F(-2), F(0), F(0), F(0)),
        "t2g_xy": (F(0), F(0), F(0), F(1), F(0), F(0)),
        "t2g_xz": (F(0), F(0), F(0), F(0), F(1), F(0)),
        "t2g_yz": (F(0), F(0), F(0), F(0), F(0), F(1)),
    }
    coefficients = {
        name: F(1, 2) * quadratic_form(inverse, direction)
        for name, direction in irrep_directions.items()
    }
    assert all(value > 0 for value in coefficients.values())
    assert coefficients["t2g_xy"] == coefficients["t2g_xz"] == coefficients["t2g_yz"]
    checks += 2

    print(f"PASS: C18 uniform-token bare blocking ({checks} exact checks)")
    print(f"alphabet mean (u,v,c) = {alphabet_means}")
    print(f"relative-current covariance per cell = {current_covariance} I_3")
    print("capacity tensor covariance =")
    for row in covariance:
        print("  ", row)
    print("capacity quadratic half-Hessian coefficients =")
    for name, value in coefficients.items():
        print(f"  {name}: {value}")
    print("Bare invariant measure only: interactions, poles, lensing, and coupling remain open")


if __name__ == "__main__":
    main()
