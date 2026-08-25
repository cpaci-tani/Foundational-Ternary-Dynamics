#!/usr/bin/env python3
"""Exact obstruction to a one-record cotangent STF streaming lift.

The 48 cotangent flags form a regular O_h orbit.  Consequently every
deterministic O_h-equivariant single-record route into the C18 neighborhood is
fixed by one of eighteen route seeds.  This certificate exhausts those seeds.

For a co-layer even/odd STF readout, every first spatial moment vanishes,
including the more general case in which the route seed depends on the C4
phase.  An adjacent-layer parity stagger produces one nonzero cubic operator
family, but the exact span of both stagger orientations has rank three and
does not contain the isotropic symmetric tensor curl (augmentation rank four).
The canonical tangent witness preserves TT only on the six axial primitive
wavevectors in the registered test set.

This closes only the single-record equivariant C18 streaming route.  It does
not exclude a multi-record collision, a larger on-site phase/parity carrier,
or a longer-range finite lift.
"""

from __future__ import annotations

from itertools import product

from sympy import Matrix
from sympy.polys.matrices import DomainMatrix

from proof_c18_tensor_doublet_tt_reduction import (
    FROBENIUS_GRAM,
    primitive_wavevectors,
    tt_projector,
)
from proof_cotangent_stf_parity_spin2_curl_target import (
    STF_BASIS,
    phase_parity_quartet,
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


STATES = one_particle_states()
FLAGS = tuple(sorted({state[0] for state in STATES}))
GROUP = tuple(signed_permutation_matrices())
REFERENCE_FLAG = FLAGS[0]
FRAME_FOR_FLAG = {
    transform_flag(frame, REFERENCE_FLAG): frame for frame in GROUP
}


def route_from_seed(state, seed):
    return matrix_vector(FRAME_FOR_FLAG[state[0]], seed)


def co_layer_phase_rows(layer: int) -> Matrix:
    """Number plus the twenty independent C4 phase/parity STF coordinates."""

    columns = []
    for state in STATES:
        quartet = phase_parity_quartet(state, layer)
        values = []
        for offset in (0, 6, 12, 18):
            values.extend(quartet[offset + component] for component in STF_COMPONENTS)
        columns.append(Matrix([1, *values]))
    rows = Matrix.hstack(*columns)
    assert rows.shape == (21, 192)
    assert exact_rank(rows) == 21
    return rows


def staggered_rows(even_layer: int, odd_layer: int) -> Matrix:
    """Number plus independent even and odd STF coordinates on two layers."""

    columns = []
    for state in STATES:
        even = stf_coordinates(state, even_layer)
        odd = stf_coordinates(state, odd_layer)
        handedness = state[0][2]
        columns.append(
            Matrix(
                [1]
                + [even[component] for component in STF_COMPONENTS]
                + [handedness * odd[component] for component in STF_COMPONENTS]
            )
        )
    rows = Matrix.hstack(*columns)
    assert rows.shape == (11, 192)
    assert exact_rank(rows) == 11
    return rows


def first_moment(
    rows: Matrix,
    gram_inverse: Matrix,
    routes,
    axis: int,
    phase: int | None = None,
) -> Matrix:
    diagonal = Matrix.diag(
        *(
            route[axis] if phase is None or state[1] == phase else 0
            for state, route in zip(STATES, routes)
        )
    )
    return rows * diagonal * rows.T * gram_inverse


def target_tensor_curl_vector() -> Matrix:
    blocks = []
    for axis in range(3):
        wavevector = Matrix([int(component == axis) for component in range(3)])
        curl = restricted_matrix(
            symmetric_curl_matrix(wavevector), STF_BASIS, FROBENIUS_GRAM
        )
        target = Matrix.zeros(11, 11)
        target[1:6, 6:11] = -curl
        target[6:11, 1:6] = curl
        assert exact_rank(target) == 8
        blocks.append(target.reshape(121, 1))
    return Matrix.vstack(*blocks)


def verify_route_census() -> int:
    checks = 0
    assert len(STATES) == 192
    assert len(FLAGS) == len(GROUP) == len(FRAME_FOR_FLAG) == 48
    assert len(C18_DIRECTIONS) == 18
    checks += 3

    # The flag action is regular.  Choosing one reference seed therefore
    # exhausts every phase-fixed O_h-equivariant deterministic C18 route.
    for seed in C18_DIRECTIONS:
        routes_by_flag = {
            flag: matrix_vector(FRAME_FOR_FLAG[flag], seed) for flag in FLAGS
        }
        assert set(routes_by_flag.values()) <= set(C18_DIRECTIONS)
        for frame in GROUP:
            for flag in FLAGS:
                transformed_flag = transform_flag(frame, flag)
                assert routes_by_flag[transformed_flag] == matrix_vector(
                    frame, routes_by_flag[flag]
                )
                checks += 1
        checks += 1
    return checks


def verify_co_layer_zero() -> int:
    checks = 0
    for layer in range(3):
        rows = co_layer_phase_rows(layer)
        gram_inverse = (rows * rows.T).inv()
        for phase in range(4):
            for seed in C18_DIRECTIONS:
                routes = tuple(route_from_seed(state, seed) for state in STATES)
                for axis in range(3):
                    assert first_moment(
                        rows, gram_inverse, routes, axis, phase
                    ) == Matrix.zeros(21, 21)
                    checks += 1
    return checks


def staggered_operator_columns() -> tuple[Matrix, int]:
    columns = []
    checks = 0
    for shift in (1, 2):
        shift_columns = []
        for even_layer in range(3):
            odd_layer = (even_layer + shift) % 3
            rows = staggered_rows(even_layer, odd_layer)
            gram_inverse = (rows * rows.T).inv()
            for seed in C18_DIRECTIONS:
                routes = tuple(route_from_seed(state, seed) for state in STATES)
                moments = tuple(
                    first_moment(rows, gram_inverse, routes, axis)
                    for axis in range(3)
                )
                shift_columns.append(
                    Matrix.vstack(*(moment.reshape(121, 1) for moment in moments))
                )
                checks += 1
        shift_span = Matrix.hstack(*shift_columns)
        assert shift_span.shape == (363, 54)
        assert exact_rank(shift_span) == 3
        columns.extend(shift_columns)
        checks += 2
    return Matrix.hstack(*columns), checks


def verify_staggered_target_obstruction() -> int:
    span, checks = staggered_operator_columns()
    target = target_tensor_curl_vector()
    assert span.shape == (363, 108)
    assert target.shape == (363, 1)
    assert exact_rank(span) == 3
    assert exact_rank(Matrix.hstack(span, target)) == 4
    checks += 4
    return checks


def verify_canonical_stagger_tt_leakage() -> int:
    """The natural tangent schedule is TT only on cubic symmetry axes."""

    axes = [Matrix.zeros(11, 11) for _ in range(3)]
    for even_layer in range(3):
        odd_layer = (even_layer + 1) % 3
        rows = staggered_rows(even_layer, odd_layer)
        gram_inverse = (rows * rows.T).inv()
        routes = tuple(state[0][0] for state in STATES)
        for axis in range(3):
            axes[axis] += first_moment(rows, gram_inverse, routes, axis) / 3

    gram = STF_BASIS.T * FROBENIUS_GRAM * STF_BASIS
    preserved = 0
    checks = 0
    for wavevector in primitive_wavevectors(2):
        projector = tt_projector(wavevector)
        basis_six = Matrix.hstack(*projector.columnspace())
        basis_five = gram.inv() * STF_BASIS.T * FROBENIUS_GRAM * basis_six
        even_to_odd = sum(
            (
                wavevector[axis] * axes[axis][1:6, 6:11]
                for axis in range(3)
            ),
            Matrix.zeros(5, 5),
        )
        odd_to_even = sum(
            (
                wavevector[axis] * axes[axis][6:11, 1:6]
                for axis in range(3)
            ),
            Matrix.zeros(5, 5),
        )
        even_image = STF_BASIS * even_to_odd * basis_five
        odd_image = STF_BASIS * odd_to_even * basis_five
        if (
            projector * even_image == even_image
            and projector * odd_image == odd_image
        ):
            preserved += 1
        checks += 1

    assert len(primitive_wavevectors(2)) == 98
    assert preserved == 6
    checks += 2
    return checks


def main() -> None:
    checks = verify_route_census()
    checks += verify_co_layer_zero()
    checks += verify_staggered_target_obstruction()
    checks += verify_canonical_stagger_tt_leakage()

    print("regular cotangent flag orbit: 48 flags, 18 equivariant C18 route seeds")
    print("co-layer even/odd STF first moment: zero for every phase-dependent route")
    print("both adjacent-layer staggers: combined operator span rank=3")
    print("symmetric tensor-curl augmentation rank=4: target is outside the span")
    print("canonical tangent stagger preserves TT on 6/98 primitive wavevectors only")
    print(
        "PASS: cotangent single-record STF streaming-lift obstruction "
        f"({checks} exact checks)"
    )
    print(
        "Scoped closed negative: a native spin-2 lift requires a genuine "
        "multi-record collision, larger carrier, or longer-range construction"
    )


if __name__ == "__main__":
    main()
