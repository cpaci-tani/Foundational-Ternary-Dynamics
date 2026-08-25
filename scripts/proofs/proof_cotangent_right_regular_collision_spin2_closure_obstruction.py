#!/usr/bin/env python3
"""Exact slow-closure obstruction for right-regular cotangent collisions.

The 48 cotangent flags form the regular O_h representation, so its local
O_h-equivariant permutation centralizer is the 48-element right-regular
action.  This certificate exhausts those collisions and all eighteen C18
streaming seeds on each cotangent layer.

If slow-space closure is ignored, their projected first spatial moments span
the symmetric tensor curl.  Exactly sixteen collisions preserve the selected
ten-dimensional even/odd STF slow space at zero momentum on each layer.  For
that physically closed subset the entire first-derivative span is zero.

Thus the apparent curl in the unrestricted projection is a fast-mode leakage
artifact, not a finite spin-2 lift.  This theorem is scoped to one-record
right-regular collisions; multi-record nonlinear collisions remain open.
"""

from __future__ import annotations

from itertools import product

from sympy import Matrix
from sympy.polys.matrices import DomainMatrix

from proof_c18_tensor_doublet_tt_reduction import FROBENIUS_GRAM
from proof_cotangent_stf_parity_spin2_curl_target import (
    STF_BASIS,
    restricted_matrix,
    stf_coordinates,
    symmetric_curl_matrix,
)
from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_shared_edge_hodge_flag_bcc_propagation import transform_flag


STF_COMPONENTS = (0, 1, 3, 4, 5)
C18_DIRECTIONS = tuple(
    vector
    for vector in product((-1, 0, 1), repeat=3)
    if sum(component * component for component in vector) in (1, 2)
)


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def matrix_product(left, right):
    return tuple(
        tuple(
            sum(left[row][middle] * right[middle][column] for middle in range(3))
            for column in range(3)
        )
        for row in range(3)
    )


STATES = one_particle_states()
FLAGS = tuple(sorted({state[0] for state in STATES}))
GROUP = tuple(signed_permutation_matrices())
REFERENCE_FLAG = FLAGS[0]
FRAME_FOR_FLAG = {
    transform_flag(frame, REFERENCE_FLAG): frame for frame in GROUP
}
FLAG_INDEX = {flag: index for index, flag in enumerate(FLAGS)}


def right_regular_permutation(right_frame) -> Matrix:
    permutation = Matrix.zeros(48, 48)
    for source, flag in enumerate(FLAGS):
        target_flag = transform_flag(
            matrix_product(FRAME_FOR_FLAG[flag], right_frame), REFERENCE_FLAG
        )
        permutation[FLAG_INDEX[target_flag], source] = 1
    return permutation


RIGHT_PERMUTATIONS = tuple(right_regular_permutation(frame) for frame in GROUP)
ROUTES = {
    seed: tuple(matrix_vector(FRAME_FOR_FLAG[flag], seed) for flag in FLAGS)
    for seed in C18_DIRECTIONS
}


def slow_rows(layer: int) -> Matrix:
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
    rows = Matrix.vstack(even, odd)
    assert rows.shape == (10, 48)
    assert exact_rank(rows) == 10
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


def verify_right_regular_group() -> int:
    checks = 0
    assert len(FLAGS) == len(GROUP) == len(RIGHT_PERMUTATIONS) == 48
    assert len(set(tuple(permutation) for permutation in RIGHT_PERMUTATIONS)) == 48
    assert len(C18_DIRECTIONS) == 18
    checks += 3
    identity = Matrix.eye(48)
    for permutation in RIGHT_PERMUTATIONS:
        assert permutation.T * permutation == identity
        assert all(sum(permutation[row, column] for row in range(48)) == 1 for column in range(48))
        assert all(sum(permutation[row, column] for column in range(48)) == 1 for row in range(48))
        checks += 3
    return checks


def verify_layer(layer: int, target: Matrix) -> int:
    rows = slow_rows(layer)
    gram_inverse = (rows * rows.T).inv()
    all_columns = []
    preserving_columns = []
    preserving_count = 0
    checks = 0

    for permutation in RIGHT_PERMUTATIONS:
        carrier = rows * permutation * rows.T * gram_inverse
        residual = rows * permutation - carrier * rows
        preserves_slow_space = residual == Matrix.zeros(10, 48)
        if preserves_slow_space:
            preserving_count += 1

        for seed in C18_DIRECTIONS:
            routes = ROUTES[seed]
            moments = []
            for axis in range(3):
                displacement = Matrix.diag(*(route[axis] for route in routes))
                moment = (
                    rows
                    * displacement
                    * permutation
                    * rows.T
                    * gram_inverse
                )
                moments.append(moment)
                checks += 1
            column = Matrix.vstack(
                *(moment.reshape(100, 1) for moment in moments)
            )
            all_columns.append(column)
            if preserves_slow_space:
                preserving_columns.append(column)
                assert column == Matrix.zeros(300, 1)
                checks += 1

    full_span = Matrix.hstack(*all_columns)
    preserving_span = Matrix.hstack(*preserving_columns)
    assert full_span.shape == (300, 864)
    assert preserving_span.shape == (300, 288)
    assert preserving_count == 16
    assert exact_rank(full_span) == 6
    assert exact_rank(Matrix.hstack(full_span, target)) == 6
    assert exact_rank(preserving_span) == 0
    assert exact_rank(Matrix.hstack(preserving_span, target)) == 1
    checks += 7
    return checks


def main() -> None:
    checks = verify_right_regular_group()
    target = tensor_curl_target()
    assert target.shape == (300, 1)
    checks += 1

    for layer in range(3):
        checks += verify_layer(layer, target)

    print("right-regular O_h collision centralizer: 48 exact permutations")
    print("each cotangent layer: 16/48 collisions preserve the even/odd STF slow space")
    print("unrestricted projected derivative span rank=6; tensor curl is contained")
    print("slow-preserving projected derivative span rank=0; target augmentation rank=1")
    print(
        "PASS: cotangent right-regular collision spin-2 closure obstruction "
        f"({checks} exact checks)"
    )
    print(
        "Scoped closed negative: target-producing one-record collisions leak "
        "the tensor observables into fast modes at zero momentum"
    )
    print("Open: genuine multi-record parity collision, larger carrier, static gravity, and lensing")


if __name__ == "__main__":
    main()
