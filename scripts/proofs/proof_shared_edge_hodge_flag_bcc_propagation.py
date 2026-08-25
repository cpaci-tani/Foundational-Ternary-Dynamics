#!/usr/bin/env python3
"""Exact shared-edge Hodge flag transport and BCC coarse propagation.

Flags carry a polar SC edge tangent d, a perpendicular axial SC face normal n,
and a pseudoscalar handedness h.  The local parity-twisted update

    (d,n,h) -> (h n, h d x n, h)

is an order-three cubic-covariant permutation.  Streaming one step along the
current tangent before each update produces a BCC body-diagonal displacement
in three ticks and exact speed squared 1/3.

This is a finite transport primitive, not a Maxwell completion: it has sixteen
ballistic flag cycles and no collision/constraint reduction to two modes.
"""

from __future__ import annotations

from collections import Counter
from itertools import permutations, product

from sympy import Matrix, Rational, expand, symbols

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_moore_bond_capacity_type_census import (
    determinant_3,
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import cross, dot


Vector = tuple[int, int, int]
Flag = tuple[Vector, Vector, int]


def scale(sign: int, vector: Vector) -> Vector:
    return tuple(sign * component for component in vector)  # type: ignore[return-value]


def add(left: Vector, right: Vector) -> Vector:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def flags() -> tuple[Flag, ...]:
    return tuple(
        (tangent, normal, handedness)
        for tangent in SC_DIRECTIONS
        for normal in SC_DIRECTIONS
        if dot(tangent, normal) == 0
        for handedness in (-1, 1)
    )


def update_flag(flag: Flag) -> Flag:
    tangent, normal, handedness = flag
    return (
        scale(handedness, normal),
        scale(handedness, cross(tangent, normal)),
        handedness,
    )


def transform_flag(matrix, flag: Flag) -> Flag:
    tangent, normal, handedness = flag
    determinant = determinant_3(matrix)
    return (
        matrix_vector(matrix, tangent),
        scale(determinant, matrix_vector(matrix, normal)),
        determinant * handedness,
    )


def three_step_displacement(flag: Flag) -> Vector:
    displacement = (0, 0, 0)
    current = flag
    for _ in range(3):
        displacement = add(displacement, current[0])
        current = update_flag(current)
    assert current == flag
    return displacement


def monomial(displacement: Vector, variables) -> object:
    return (
        variables[0] ** displacement[0]
        * variables[1] ** displacement[1]
        * variables[2] ** displacement[2]
    )


def cycle_transfer(cycle: tuple[Flag, Flag, Flag], variables) -> Matrix:
    transfer = Matrix.zeros(3, 3)
    for index, flag in enumerate(cycle):
        transfer[(index + 1) % 3, index] = monomial(flag[0], variables)
    return transfer


def wrapped_add(position: Vector, step: Vector, length: int) -> Vector:
    return tuple((position[index] + step[index]) % length for index in range(3))  # type: ignore[return-value]


def full_tick(state: tuple[Vector, Flag, int], length: int):
    position, flag, phase = state
    return (
        wrapped_add(position, flag[0], length),
        update_flag(flag),
        (phase + 1) % 4,
    )


def inverse_tick(state: tuple[Vector, Flag, int], length: int):
    position, flag, phase = state
    previous_flag = update_flag(update_flag(flag))
    previous_position = wrapped_add(position, scale(-1, previous_flag[0]), length)
    return previous_position, previous_flag, (phase - 1) % 4


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())
    state_flags = flags()
    assert len(state_flags) == 48
    assert len(set(state_flags)) == 48
    checks += 2

    # At a fixed directed edge, the D4 stabilizer acting on the four incident
    # axial normals has centralizer {identity, antipode}.  A context-free
    # equivariant rule therefore cannot choose either quarter turn.  The
    # pseudoscalar handedness is the exact missing routing datum.
    for tangent in SC_DIRECTIONS:
        incident_normals = tuple(
            normal for normal in SC_DIRECTIONS if dot(tangent, normal) == 0
        )
        normal_index = {
            normal: index for index, normal in enumerate(incident_normals)
        }
        stabilizer = tuple(
            matrix
            for matrix in group
            if matrix_vector(matrix, tangent) == tangent
        )
        assert len(stabilizer) == 8
        actions = tuple(
            tuple(
                normal_index[
                    scale(
                        determinant_3(matrix),
                        matrix_vector(matrix, normal),
                    )
                ]
                for normal in incident_normals
            )
            for matrix in stabilizer
        )

        def compose(left, right):
            return tuple(left[right[index]] for index in range(4))

        centralizer = tuple(
            permutation
            for permutation in permutations(range(4))
            if all(
                compose(permutation, action) == compose(action, permutation)
                for action in actions
            )
        )
        identity = tuple(range(4))
        antipode = tuple(
            normal_index[scale(-1, normal)] for normal in incident_normals
        )
        assert set(centralizer) == {identity, antipode}
        checks += 2

    image = {flag: update_flag(flag) for flag in state_flags}
    assert set(image.values()) == set(state_flags)
    assert all(update_flag(update_flag(update_flag(flag))) == flag for flag in state_flags)
    checks += 2

    for flag in state_flags:
        tangent, normal, handedness = flag
        updated_tangent, updated_normal, updated_handedness = image[flag]
        assert dot(updated_tangent, updated_normal) == 0
        assert updated_tangent in SC_DIRECTIONS
        assert updated_normal in SC_DIRECTIONS
        assert updated_handedness == handedness
        for matrix in group:
            assert update_flag(transform_flag(matrix, flag)) == transform_flag(
                matrix, image[flag]
            )
            checks += 1
        checks += 4

    bcc_directions = set(product((-1, 1), repeat=3))
    displacement_histogram = Counter(
        three_step_displacement(flag) for flag in state_flags
    )
    assert set(displacement_histogram) == bcc_directions
    assert set(displacement_histogram.values()) == {6}
    for displacement in displacement_histogram:
        assert sum(component * component for component in displacement) == 3
        assert Rational(3, 3 * 3) == Rational(1, 3)
        checks += 2
    checks += 2

    unseen = set(state_flags)
    cycles = []
    while unseen:
        start = min(unseen)
        cycle = (start, update_flag(start), update_flag(update_flag(start)))
        assert update_flag(cycle[-1]) == start
        assert len(set(cycle)) == 3
        for flag in cycle:
            unseen.remove(flag)
        cycles.append(cycle)
    assert len(cycles) == 16
    cycle_displacements = Counter(three_step_displacement(cycle[0]) for cycle in cycles)
    assert set(cycle_displacements) == bcc_directions
    assert set(cycle_displacements.values()) == {2}
    checks += 4

    x, y, z = symbols("x y z", nonzero=True)
    eigenvalue = symbols("eigenvalue")
    variables = (x, y, z)
    for cycle in cycles:
        transfer = cycle_transfer(cycle, variables)
        displacement = three_step_displacement(cycle[0])
        net_phase = monomial(displacement, variables)
        assert transfer**3 == net_phase * Matrix.eye(3)
        assert expand(
            transfer.charpoly(eigenvalue).as_expr()
            - (eigenvalue**3 - net_phase)
        ) == 0
        checks += 2

    # The complete finite update, including an independent common C4 phase
    # advance, is a bijection on finite periodic boxes and has an exact inverse.
    for length in (2, 3, 4):
        positions = tuple(product(range(length), repeat=3))
        states = tuple(
            (position, flag, phase)
            for position in positions
            for flag in state_flags
            for phase in range(4)
        )
        outputs = tuple(full_tick(state, length) for state in states)
        assert len(set(outputs)) == len(states)
        assert all(inverse_tick(full_tick(state, length), length) == state for state in states)
        for state in states[: min(256, len(states))]:
            advanced = state
            for _ in range(12):
                advanced = full_tick(advanced, length)
            expected_position = wrapped_add(
                state[0], scale(4, three_step_displacement(state[1])), length
            )
            assert advanced == (expected_position, state[1], state[2])
            checks += 1
        checks += 2

    print(f"PASS: shared-edge Hodge flag BCC propagation ({checks} exact checks)")
    print("48 flags -> 16 internal three-cycles -> 8 BCC directions (two cycles each)")
    print("three SC hops give BCC displacement Delta with |Delta|^2=3")
    print("coarse speed squared=1/3, speed=1/sqrt(3) lattice units per tick")
    print("cycle Bloch polynomial=lambda^3-exp(i k.Delta): exact ballistic BCC rays")
    print("with common C4 phase, internal return is 12 ticks and displacement 4 Delta")
    print("Open: flag collision/constraint reduction from 16 rays to two Maxwell modes")


if __name__ == "__main__":
    main()
