#!/usr/bin/env python3
"""Exact square proto-matter clock with an endogenous radiation plane.

An ordered perpendicular SC pair (d,v) defines a four-edge square.  A neutral
ternary dipole (-epsilon,+epsilon) occupies one edge and advances to the next
edge each admitted tick.  The resulting map is a finite period-four
permutation.  Its two endpoint charges obey exact discrete continuity through
two simultaneous signed edge currents, and every tick retains the ordered
right-angle turn needed by the number-neutral cotangent radiation seed.

This constructs a spatially recurrent proto-matter clock and removes an
external frame router for its radiation vertex.  It does not derive formation,
binding energy, radiation work/recoil, finite-amplitude Maxwell propagation,
or a Lorentz force.
"""

from __future__ import annotations

from dataclasses import dataclass
from itertools import product

from sympy import Matrix, Rational

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_cotangent_framed_plaquette_radiation_release import (
    PlaneFrame,
    RadiationSeed,
    plaquette_edges,
    seed_divergence,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import dot
from proof_shared_edge_hodge_flag_bcc_propagation import add, scale


Vector = tuple[int, int, int]


def negate(vector: Vector) -> Vector:
    return scale(-1, vector)


def add_to(mapping: dict[Vector, int], key: Vector, amount: int) -> None:
    mapping[key] = mapping.get(key, 0) + amount
    if mapping[key] == 0:
        del mapping[key]


@dataclass(frozen=True)
class SquareMatterState:
    frame: PlaneFrame
    phase: int
    orientation: int


def step(state: SquareMatterState) -> SquareMatterState:
    return SquareMatterState(
        state.frame,
        (state.phase + 1) % 4,
        state.orientation,
    )


def inverse_step(state: SquareMatterState) -> SquareMatterState:
    return SquareMatterState(
        state.frame,
        (state.phase - 1) % 4,
        state.orientation,
    )


def gated_step(state: SquareMatterState, admitted: bool) -> SquareMatterState:
    return step(state) if admitted else state


def advance(state: SquareMatterState, ticks: int) -> SquareMatterState:
    output = state
    for _ in range(ticks):
        output = step(output)
    return output


def current_edge(state: SquareMatterState) -> tuple[Vector, Vector]:
    return plaquette_edges(state.frame)[state.phase]


def next_edge(state: SquareMatterState) -> tuple[Vector, Vector]:
    return plaquette_edges(state.frame)[(state.phase + 1) % 4]


def charge_distribution(state: SquareMatterState) -> dict[Vector, int]:
    tail, direction = current_edge(state)
    return {
        tail: -state.orientation,
        add(tail, direction): state.orientation,
    }


def chain_boundary(chain: tuple[tuple[Vector, Vector, int], ...]):
    result: dict[Vector, int] = {}
    for tail, direction, amount in chain:
        add_to(result, tail, -amount)
        add_to(result, add(tail, direction), amount)
    return result


def transport_current(state: SquareMatterState):
    """Move the negative endpoint on the old edge and positive on the next."""

    old_tail, old_direction = current_edge(state)
    next_tail, next_direction = next_edge(state)
    assert add(old_tail, old_direction) == next_tail
    return (
        (old_tail, old_direction, -state.orientation),
        (next_tail, next_direction, state.orientation),
    )


def current_vector(state: SquareMatterState) -> Matrix:
    return sum(
        (amount * Matrix(direction) for _tail, direction, amount in transport_current(state)),
        start=Matrix.zeros(3, 1),
    )


def stress_dyad(state: SquareMatterState) -> Matrix:
    direction = Matrix(current_edge(state)[1])
    return direction * direction.T


def turn_frame(state: SquareMatterState) -> PlaneFrame:
    incoming = current_edge(state)[1]
    outgoing = next_edge(state)[1]
    assert dot(incoming, outgoing) == 0
    return incoming, outgoing


def conjugate(state: SquareMatterState) -> SquareMatterState:
    return SquareMatterState(state.frame, state.phase, -state.orientation)


def transform_state(matrix, state: SquareMatterState) -> SquareMatterState:
    return SquareMatterState(
        (
            tuple(matrix_vector(matrix, state.frame[0])),
            tuple(matrix_vector(matrix, state.frame[1])),
        ),
        state.phase,
        state.orientation,
    )


def transform_distribution(matrix, distribution):
    return {
        tuple(matrix_vector(matrix, site)): amount
        for site, amount in distribution.items()
    }


def transform_current(matrix, chain):
    return tuple(
        (
            tuple(matrix_vector(matrix, tail)),
            tuple(matrix_vector(matrix, direction)),
            amount,
        )
        for tail, direction, amount in chain
    )


def subtract_distributions(after, before):
    result = dict(after)
    for site, amount in before.items():
        add_to(result, site, -amount)
    return result


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())
    frames = tuple(
        (direction, second)
        for direction in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(direction, second) == 0
    )
    assert len(frames) == 24
    checks += 1

    for frame in frames:
        edges = plaquette_edges(frame)
        assert len(edges) == 4
        assert len({tail for tail, _direction in edges}) == 4
        for index, (tail, direction) in enumerate(edges):
            next_tail = edges[(index + 1) % 4][0]
            assert add(tail, direction) == next_tail
            assert dot(direction, edges[(index + 1) % 4][1]) == 0
            checks += 2

        for orientation in (-1, 1):
            initial = SquareMatterState(frame, 0, orientation)
            orbit = [initial]
            state = initial
            for _ in range(4):
                output = step(state)
                assert inverse_step(output) == state
                assert sum(charge_distribution(state).values()) == 0

                delta_charge = subtract_distributions(
                    charge_distribution(output),
                    charge_distribution(state),
                )
                current = transport_current(state)
                assert delta_charge == chain_boundary(current)
                assert sum(delta_charge.values()) == 0

                # The material turn supplies the exact four-way radiation
                # frame; no independent normal/handed selector is required.
                radiation = RadiationSeed(
                    turn_frame(state),
                    state.phase,
                    state.phase,
                    state.orientation,
                    True,
                )
                assert seed_divergence(radiation) == {}

                conjugated = conjugate(state)
                assert charge_distribution(conjugated) == {
                    site: -amount
                    for site, amount in charge_distribution(state).items()
                }
                assert current_vector(conjugated) == -current_vector(state)
                assert stress_dyad(conjugated) == stress_dyad(state)
                checks += 9

                for matrix in group:
                    transformed = transform_state(matrix, state)
                    assert step(transformed) == transform_state(matrix, output)
                    assert charge_distribution(transformed) == transform_distribution(
                        matrix, charge_distribution(state)
                    )
                    assert transport_current(transformed) == transform_current(
                        matrix, current
                    )
                    assert turn_frame(transformed) == (
                        tuple(matrix_vector(matrix, turn_frame(state)[0])),
                        tuple(matrix_vector(matrix, turn_frame(state)[1])),
                    )
                    checks += 4

                state = output
                orbit.append(state)

            assert state == initial
            assert len(set(orbit[:-1])) == 4
            assert sum(
                (current_vector(item) for item in orbit[:-1]),
                start=Matrix.zeros(3, 1),
            ) == Matrix.zeros(3, 1)
            plane_projector = (
                Matrix(frame[0]) * Matrix(frame[0]).T
                + Matrix(frame[1]) * Matrix(frame[1]).T
            )
            mean_stress = sum(
                (stress_dyad(item) for item in orbit[:-1]),
                start=Matrix.zeros(3, 3),
            ) / 4
            assert mean_stress == Rational(1, 2) * plane_projector
            assert mean_stress.trace() == 1
            checks += 5

            # Global ticks versus locally admitted material turns.
            for length in range(9):
                for permission_word in product((False, True), repeat=length):
                    gated = initial
                    for permission in permission_word:
                        gated = gated_step(gated, permission)
                    admitted = sum(permission_word)
                    assert gated == advance(initial, admitted)
                    assert gated == advance(initial, admitted % 4)
                    checks += 2

    print("square matter alphabet: 24 oriented SC planes x C4 phase x charge polarity")
    print("one admitted tick: neutral dipole advances one edge with exact continuity")
    print("material recurrence: period 4, zero cycle current, positive mean plane stress")
    print("global/admitted clock split: every permission word through length 8")
    print("every corner supplies ordered (incoming,outgoing) radiation frame equivariantly")
    print(
        f"PASS: C4 square material turn clock and radiation frame ({checks} exact checks)"
    )
    print(
        "Open: formation/binding, common stress feedback, radiation work/recoil, "
        "finite Maxwell lift, Lorentz force, gravity/lensing, Born, and alpha"
    )


if __name__ == "__main__":
    main()
