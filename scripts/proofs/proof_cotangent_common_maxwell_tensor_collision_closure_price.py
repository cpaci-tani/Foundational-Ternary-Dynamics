#!/usr/bin/env python3
"""Exact common Maxwell/tensor collision-closure price.

The fixed-C4-quadrature rank-20 tensor closure cannot be assessed
independently of the cotangent Maxwell sector.  This certificate exhausts the
same 48 right-regular local collisions and all eighteen C18 route seeds while
retaining both:

  * the seven number/E/B Maxwell rows; and
  * the ten even/odd STF tensor rows.

On every cotangent layer the full invariant common closures have dimensions
17, 23, 30, or 45 with multiplicities 8, 8, 16, and 16.  The two smaller
classes have zero tensor-derivative span.  The symmetric tensor curl belongs
to the rank-six derivative span of the dimension-30 and dimension-45 classes.
Therefore the smallest target-containing one-record common carrier has
fixed-quadrature dimension thirty, not twenty, and necessarily enlarges the
seven-dimensional Maxwell slow space to dimension ten.  The phase-complete
successor doubles the tensor closure and raises this target price to fifty.

This is an exact type/closure price, not a native spin-2, Maxwell, gravity, or
lensing derivation.
"""

from __future__ import annotations

from collections import Counter, defaultdict
from itertools import product

from sympy import Matrix
from sympy.polys.matrices import DomainMatrix

from proof_c18_tensor_doublet_tt_reduction import FROBENIUS_GRAM
from proof_cotangent_rank20_collision_closure_tt_leakage import (
    invariant_closure,
    tensor_rows,
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
    symmetric_curl_matrix,
)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import layer_value
from proof_moore_bond_capacity_type_census import matrix_vector


C18_DIRECTIONS = tuple(
    vector
    for vector in product((-1, 0, 1), repeat=3)
    if sum(component * component for component in vector) in (1, 2)
)
EXPECTED_CLOSURES = Counter(
    {
        (10, 7, 17): 8,
        (10, 13, 23): 8,
        (20, 10, 30): 16,
        (26, 19, 45): 16,
    }
)


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def maxwell_rows(layer: int) -> Matrix:
    rows = Matrix.vstack(
        Matrix([[1] * 48]),
        *(
            Matrix([[layer_value((flag, 0), layer)[component] for flag in FLAGS]])
            for component in range(6)
        ),
    )
    assert rows.shape == (7, 48)
    assert exact_rank(rows) == 7
    return rows


def tensor_curl_target() -> Matrix:
    blocks = []
    for axis in range(3):
        wavevector = Matrix([int(component == axis) for component in range(3)])
        curl = restricted_matrix(
            symmetric_curl_matrix(wavevector), STF_BASIS, FROBENIUS_GRAM
        )
        target = Matrix.zeros(10, 10)
        target[0:5, 5:10] = -curl
        target[5:10, 0:5] = curl
        assert exact_rank(target) == 8
        blocks.append(target.reshape(100, 1))
    return Matrix.vstack(*blocks)


def derivative_column(rows: Matrix, permutation: Matrix, seed) -> Matrix:
    gram_inverse = (rows * rows.T).inv()
    routes = tuple(matrix_vector(FRAME_FOR_FLAG[flag], seed) for flag in FLAGS)
    moments = []
    for axis in range(3):
        displacement = Matrix.diag(*(route[axis] for route in routes))
        moments.append(
            rows
            * displacement
            * permutation
            * rows.T
            * gram_inverse
        )
    return Matrix.vstack(*(moment.reshape(100, 1) for moment in moments))


def verify_layer(layer: int, target: Matrix) -> int:
    tensor = tensor_rows(layer)
    maxwell = maxwell_rows(layer)
    common = Matrix.vstack(maxwell, tensor)
    assert exact_rank(tensor) == 10
    assert exact_rank(common) == 17

    census = Counter()
    derivative_buckets = defaultdict(list)
    checks = 2

    for frame in GROUP:
        collision = right_regular_permutation(frame)
        tensor_closure = invariant_closure(tensor, collision)
        maxwell_closure = invariant_closure(maxwell, collision)
        common_closure = invariant_closure(common, collision)
        signature = (
            tensor_closure.rows,
            maxwell_closure.rows,
            common_closure.rows,
        )
        census[signature] += 1
        assert common_closure.rows == tensor_closure.rows + maxwell_closure.rows
        checks += 1

        for seed in C18_DIRECTIONS:
            derivative_buckets[common_closure.rows].append(
                derivative_column(tensor, collision, seed)
            )
            checks += 1

    assert census == EXPECTED_CLOSURES
    assert set(derivative_buckets) == {17, 23, 30, 45}
    checks += 2

    expected = {
        17: (0, 1, 8),
        23: (0, 1, 8),
        30: (6, 6, 16),
        45: (6, 6, 16),
    }
    for closure_dimension, columns in derivative_buckets.items():
        span = Matrix.hstack(*columns)
        span_rank = exact_rank(span)
        augmentation_rank = exact_rank(Matrix.hstack(span, target))
        expected_span, expected_augmentation, expected_collisions = expected[
            closure_dimension
        ]
        assert len(columns) == 18 * expected_collisions
        assert span_rank == expected_span
        assert augmentation_rank == expected_augmentation
        checks += 3

    return checks


def main() -> None:
    target = tensor_curl_target()
    assert target.shape == (300, 1)
    checks = 1
    for layer in range(3):
        checks += verify_layer(layer, target)

    print("common closure census per layer:")
    print("  8 x (tensor10, Maxwell7, common17)")
    print("  8 x (tensor10, Maxwell13, common23)")
    print(" 16 x (tensor20, Maxwell10, common30)")
    print(" 16 x (tensor26, Maxwell19, common45)")
    print("common17/common23 tensor-derivative span rank=0")
    print("common30/common45 derivative span rank=6 and contains symmetric curl")
    print("minimum fixed-C4-quadrature target-containing common carrier dimension=30")
    print(
        "PASS: cotangent common Maxwell/tensor collision-closure price "
        f"({checks} exact checks)"
    )
    print(
        "Open: phase-complete carrier/reality choice, native constraint-generating "
        "action, recovered Maxwell/Gauss and spin-2 poles, static response, and lensing"
    )


if __name__ == "__main__":
    main()
