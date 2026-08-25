#!/usr/bin/env python3
"""Exact cotangent FCC-dyad tensor doublet and shared source moment.

At cotangent layer q, E=P_q and B=A_q are perpendicular polar/axial SC legs.
Because hB is polar, r=E+hB is an FCC direction.  Its normalized dyad
D=rr^T/2 spans all six symmetric tensor components across the existing flag
alphabet.  Multiplication by the native C4 quadratures supplies a rank-twelve
tensor coordinate/momentum doublet that rotates (Q,P)->(-P,Q) under one tick.

The same eight-record electric stabilizer packet has a nonzero aligned
trace-free dyad source.  This is a native kinematic spin-2-capacity carrier and
shared source type, not yet a protected propagating tensor pole, universal
gravity, or lensing theorem.
"""

from __future__ import annotations

from sympy import Matrix, Rational
from sympy.polys.matrices import DomainMatrix

from proof_cotangent_stabilizer_packet_gauss_source import packet
from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick,
    layer_value,
)
from proof_hodge_flag_pair_collision_invariant_space import (
    PHASE_COORDINATES,
    one_particle_states,
)
from proof_moore_bond_capacity_type_census import (
    determinant_3,
    matrix_vector,
    signed_permutation_matrices,
)
from proof_shared_edge_hodge_flag_bcc_propagation import transform_flag


SYMMETRIC_COMPONENTS = ((0, 0), (1, 1), (2, 2), (0, 1), (0, 2), (1, 2))


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def dyad(state, layer: int) -> Matrix:
    flag, _phase = state
    handedness = flag[2]
    value = layer_value(state, layer)
    electric = Matrix(value[:3])
    magnetic = Matrix(value[3:])
    direction = electric + handedness * magnetic
    assert (direction.T * direction)[0] == 2
    return direction * direction.T / 2


def flatten_symmetric(matrix: Matrix) -> tuple[object, ...]:
    return tuple(matrix[left, right] for left, right in SYMMETRIC_COMPONENTS)


def tensor_value(state, layer: int) -> tuple[object, ...]:
    _flag, phase = state
    u, v = PHASE_COORDINATES[phase]
    flattened = flatten_symmetric(dyad(state, layer))
    return tuple(u * entry for entry in flattened) + tuple(
        v * entry for entry in flattened
    )


def main() -> None:
    checks = 0
    states = one_particle_states()
    group = tuple(signed_permutation_matrices())
    assert len(states) == 192
    checks += 1

    for layer in range(3):
        dyad_rows = Matrix(
            [list(flatten_symmetric(dyad(state, layer))) for state in states]
        ).T
        tensor_rows = Matrix(
            [list(tensor_value(state, layer)) for state in states]
        ).T
        field_rows = Matrix.vstack(
            Matrix([[1] * 192]),
            *(
                Matrix([[layer_value(state, layer)[component] for state in states]])
                for component in range(6)
            ),
        )
        assert exact_rank(dyad_rows) == 6
        assert exact_rank(tensor_rows) == 12
        assert exact_rank(Matrix.vstack(field_rows, tensor_rows)) == 19
        checks += 3

        unique_dyads = {tuple(dyad(state, layer)) for state in states}
        assert len(unique_dyads) == 6
        checks += 1

        for state in states:
            current = tensor_value(state, layer)
            advanced = tensor_value(internal_tick(state), (layer - 1) % 3)
            assert advanced[:6] == tuple(-entry for entry in current[6:])
            assert advanced[6:] == current[:6]
            checks += 1

            source_matrix = dyad(state, layer)
            for matrix in group:
                transformed_state = (transform_flag(matrix, state[0]), state[1])
                transformed = dyad(transformed_state, layer)
                rotation = Matrix(matrix)
                assert transformed == rotation * source_matrix * rotation.T
                assert determinant_3(matrix) in (-1, 1)
                checks += 2

    # The symmetric tensor has four independent transverse-traceless
    # constraints for every SC propagation axis, leaving two tensor
    # polarizations.  The C4 doublet therefore leaves four phase-space modes.
    for axis in range(3):
        constraints = []
        trace_row = [1, 1, 1, 0, 0, 0]
        constraints.append(trace_row)
        for column in range(3):
            row = [0] * 6
            pair = tuple(sorted((axis, column)))
            if pair == (0, 0):
                row[0] = 1
            elif pair == (1, 1):
                row[1] = 1
            elif pair == (2, 2):
                row[2] = 1
            elif pair == (0, 1):
                row[3] = 1
            elif pair == (0, 2):
                row[4] = 1
            elif pair == (1, 2):
                row[5] = 1
            constraints.append(row)
        constraint_matrix = Matrix(constraints)
        assert exact_rank(constraint_matrix) == 4
        assert len(constraint_matrix.nullspace()) == 2
        checks += 2

    # Shared eight-record source at q=0.  Sum its FCC dyads and isolate the
    # trace-free part.  It is aligned with the electric source edge.
    for direction in (
        (1, 0, 0),
        (-1, 0, 0),
        (0, 1, 0),
        (0, -1, 0),
        (0, 0, 1),
        (0, 0, -1),
    ):
        direction_column = Matrix(direction)
        expected_sum = 2 * Matrix.eye(3) + 2 * direction_column * direction_column.T
        expected_stf = 2 * (
            direction_column * direction_column.T - Matrix.eye(3) / 3
        )
        for phase in range(4):
            records = packet(direction, phase)
            total_dyad = sum(
                (dyad(state, 0) for state in records),
                start=Matrix.zeros(3, 3),
            )
            stf = total_dyad - total_dyad.trace() * Matrix.eye(3) / 3
            assert total_dyad == expected_sum
            assert stf == expected_stf
            assert stf.trace() == 0
            u, v = PHASE_COORDINATES[phase]
            total_tensor = sum(
                (Matrix(tensor_value(state, 0)) for state in records),
                start=Matrix.zeros(12, 1),
            )
            expected_flat = Matrix(flatten_symmetric(expected_sum))
            assert total_tensor[:6, 0] == u * expected_flat
            assert total_tensor[6:, 0] == v * expected_flat
            checks += 5

    print("cotangent_r=E+hB is an FCC direction with normalized dyad rank=6")
    print("C4-weighted tensor doublet rank=12; combined number+E+B+Q+P rank=19")
    print("TT constraints leave two tensor polarizations and four phase-space modes")
    print("eight-record Gauss packet STF source=2*(d d^T-I/3)")
    print(
        f"PASS: cotangent FCC-dyad tensor doublet and source ({checks} exact checks)"
    )
    print(
        "Open: collision protection, tensor Bloch pole, universal capacity "
        "coupling, static gravity, and lensing"
    )


if __name__ == "__main__":
    main()
