#!/usr/bin/env python3
"""Exact rank-20 collision closure and TT-leakage certificate.

The right-regular one-record collision census shows that a projected tensor
curl is possible only when the selected ten STF moments leak into fast modes.
This certificate retains those modes instead of discarding them.

Across every cotangent layer, the 48 right-regular collisions close the ten
STF rows into dimensions 10, 20, or 26, with sixteen collisions in each class.
A minimum leaky involution closes at dimension twenty.  For a selected
five-hop C18 target-containing schedule, that rank-20 carrier is exact,
reversible at zero momentum, and its co-rotating first moment is self-adjoint
in the induced energy metric.  But a four-dimensional TT seed generates an
8-, 16-, or 18-dimensional Krylov space on axis, body-diagonal, or generic
wavevectors.  The spectrum is direction dependent.

This is a selected fixed-C4-quadrature boundary diagnostic, not a native
spin-2 derivation.  The phase-complete tensor closure has a separate doubled
price.
"""

from __future__ import annotations

from collections import Counter

from sympy import Matrix, symbols

from proof_c18_tensor_doublet_tt_reduction import (
    FROBENIUS_GRAM,
    tt_projector,
)
from proof_cotangent_right_regular_collision_spin2_closure_obstruction import (
    FLAGS,
    FRAME_FOR_FLAG,
    GROUP,
    right_regular_permutation,
)
from proof_cotangent_stf_parity_spin2_curl_target import (
    STF_BASIS,
    restricted_matrix,
    stf_coordinates,
    symmetric_curl_matrix,
)
from proof_moore_bond_capacity_type_census import matrix_vector


STF_COMPONENTS = (0, 1, 3, 4, 5)
SELECTED_COLLISION_INDEX = 8
SELECTED_ROUTE_SCHEDULE = (
    ((1, 1, 0), 2),
    ((1, 0, 1), 1),
    ((-1, 0, 0), 2),
)


def tensor_rows(layer: int) -> Matrix:
    even = Matrix.hstack(
        *(
            Matrix(
                [stf_coordinates((flag, 0), layer)[entry] for entry in STF_COMPONENTS]
            )
            for flag in FLAGS
        )
    )
    odd = Matrix.hstack(
        *(
            Matrix(
                [
                    flag[2] * stf_coordinates((flag, 0), layer)[entry]
                    for entry in STF_COMPONENTS
                ]
            )
            for flag in FLAGS
        )
    )
    return Matrix.vstack(even, odd)


def invariant_closure(rows: Matrix, permutation: Matrix) -> Matrix:
    closure = rows
    current = rows
    while True:
        current = current * permutation
        candidate = Matrix.vstack(closure, current)
        independent = Matrix.vstack(*candidate.rowspace())
        if independent.rows == closure.rows:
            return closure
        closure = independent


def verify_closure_census() -> int:
    checks = 0
    expected = Counter({10: 16, 20: 16, 26: 16})
    for layer in range(3):
        rows = tensor_rows(layer)
        histogram = Counter(
            invariant_closure(rows, right_regular_permutation(frame)).rows
            for frame in GROUP
        )
        assert histogram == expected
        checks += 1
    return checks


def selected_rank20_carrier():
    layer = 0
    rows = tensor_rows(layer)
    permutation = right_regular_permutation(GROUP[SELECTED_COLLISION_INDEX])
    closure = Matrix.vstack(rows, rows * permutation)
    assert closure.shape == (20, 48)
    assert closure.rank() == 20
    assert Matrix.vstack(closure, closure * permutation).rank() == 20
    return rows, closure, permutation


def displacement_sum(axis: int) -> Matrix:
    diagonal = Matrix.zeros(48, 48)
    for seed, multiplicity in SELECTED_ROUTE_SCHEDULE:
        routes = tuple(matrix_vector(FRAME_FOR_FLAG[flag], seed) for flag in FLAGS)
        diagonal += multiplicity * Matrix.diag(*(route[axis] for route in routes))
    return diagonal


def verify_selected_raw_target(rows: Matrix, permutation: Matrix) -> int:
    even = rows[0:5, :]
    odd = rows[5:10, :]
    odd_gram_inverse = (odd * odd.T).inv()
    checks = 0
    for axis in range(3):
        projected = (
            even
            * displacement_sum(axis)
            * permutation
            * odd.T
            * odd_gram_inverse
        )
        wavevector = Matrix([int(component == axis) for component in range(3)])
        target = restricted_matrix(
            symmetric_curl_matrix(wavevector), STF_BASIS, FROBENIUS_GRAM
        )
        assert projected == target
        checks += 1
    return checks


def co_rotating_axes(closure: Matrix, permutation: Matrix):
    gram = closure * closure.T
    metric = gram.inv()
    carrier = closure * permutation * closure.T * metric
    assert closure * permutation == carrier * closure
    assert carrier * carrier == Matrix.eye(20)

    axes = []
    for axis in range(3):
        raw_moment = (
            closure
            * displacement_sum(axis)
            * permutation
            * closure.T
            * metric
        )
        co_rotating = carrier.inv() * raw_moment
        assert co_rotating.T * metric == metric * co_rotating
        assert co_rotating.rank() == 16
        axes.append(co_rotating)
    return tuple(axes)


def tt_seed(wavevector: Matrix) -> Matrix:
    projector = tt_projector(wavevector)
    basis_six = Matrix.hstack(*projector.columnspace())
    gram_five = STF_BASIS.T * FROBENIUS_GRAM * STF_BASIS
    basis_five = gram_five.inv() * STF_BASIS.T * FROBENIUS_GRAM * basis_six
    seed = Matrix.zeros(20, 4)
    seed[0:5, 0:2] = basis_five
    seed[5:10, 2:4] = basis_five
    assert seed.rank() == 4
    return seed


def krylov_dimension(operator: Matrix, seed: Matrix) -> int:
    closure = seed
    current = seed
    while True:
        current = operator * current
        candidate = Matrix.hstack(closure, current)
        independent = Matrix.hstack(*candidate.columnspace())
        if independent.cols == closure.cols:
            return closure.cols
        closure = independent


def verify_spectra_and_tt_leakage(axes) -> int:
    eigenvalue = symbols("lambda")
    cases = (
        (
            (1, 0, 0),
            8,
            eigenvalue**4
            * (2 * eigenvalue - 1) ** 4
            * (2 * eigenvalue + 1) ** 4
            * (3 * eigenvalue**4 - 10 * eigenvalue**2 + 4) ** 2
            / 2304,
        ),
        (
            (1, 1, 1),
            16,
            eigenvalue**4
            * (4 * eigenvalue**4 - 23 * eigenvalue**2 + 12) ** 4
            / 256,
        ),
        (
            (1, 2, 3),
            18,
            eigenvalue**4
            * (
                12 * eigenvalue**8
                - 644 * eigenvalue**6
                + 9457 * eigenvalue**4
                - 43465 * eigenvalue**2
                + 55223
            )
            ** 2
            / 144,
        ),
    )
    checks = 0
    for components, expected_dimension, expected_characteristic in cases:
        wavevector = Matrix(components)
        operator = sum(
            (components[axis] * axes[axis] for axis in range(3)),
            Matrix.zeros(20, 20),
        )
        assert operator.rank() == 16
        assert krylov_dimension(operator, tt_seed(wavevector)) == expected_dimension
        assert (
            operator.charpoly(eigenvalue).as_expr() - expected_characteristic
        ).expand() == 0
        checks += 3
    return checks


def main() -> None:
    checks = verify_closure_census()
    rows, closure, permutation = selected_rank20_carrier()
    assert GROUP[SELECTED_COLLISION_INDEX] == (
        (-1, 0, 0),
        (0, 0, -1),
        (0, -1, 0),
    )
    assert permutation * permutation == Matrix.eye(48)
    checks += 2
    checks += verify_selected_raw_target(rows, permutation)
    axes = co_rotating_axes(closure, permutation)
    checks += 5
    checks += verify_spectra_and_tt_leakage(axes)

    print("right-collision closure census per layer: 16x rank10, 16x rank20, 16x rank26")
    print("minimum target-producing involution closes the tensor carrier at rank20")
    print("selected five-hop raw projection contains the exact symmetric curl")
    print("rank20 co-rotating first moment is energy-self-adjoint but cubically anisotropic")
    print("TT Krylov dimensions: axis=8, body diagonal=16, generic=18")
    print(
        "PASS: cotangent rank20 collision closure and TT leakage "
        f"({checks} exact checks)"
    )
    print(
        "Open: native multi-record collision/constraint, isolated helicity-two pole, "
        "static response, and lensing"
    )


if __name__ == "__main__":
    main()
