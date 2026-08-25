#!/usr/bin/env python3
"""Exact C6 cubic-Petrie proto-matter clock and endogenous radiation port.

The planar square clock owns an ordered plane but not the spatial
pseudoscalar needed to choose a polar outgoing normal.  The six-edge cubic
Petrie route

    d, v, w, -d, -v, -w

closes without repetition when (d,v,w) is an ordered orthonormal SC triad.
Every local pair of consecutive route directions has a third retained route
direction.  Its scalar triple product is the required pseudoscalar chi, and

    third = chi * (current x next).

Thus the nonplanar route supplies the handed directional cotangent port
without an independent chirality label.

A neutral ternary dipole advances around the route with exact incidence
continuity.  The route is period six, the common cotangent/C4 stage is period
twelve, and their always-admitted product is period twelve.  Mean material
stress is I/3.  Arbitrary retained admission words give the usual
global-stage/material-clock split.

This is prepared proto-matter, not autonomous formation or stable binding.
It supplies the directional port context but does not yet write the required
opposite recoil into a material translational momentum coordinate.
"""

from __future__ import annotations

from dataclasses import dataclass
from itertools import product

from sympy import Matrix, Rational

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_cotangent_handed_directional_radiation_port import (
    DirectionalPortState,
    field_momentum,
    field_norm,
    port_records,
    propagation_direction,
    transform_state as transform_port,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import cross, dot
from proof_shared_edge_hodge_flag_bcc_propagation import add, scale


Vector = tuple[int, int, int]
Triad = tuple[Vector, Vector, Vector]


def negate(vector: Vector) -> Vector:
    return scale(-1, vector)


def subtract(left: Vector, right: Vector) -> Vector:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def ordered_triads() -> tuple[Triad, ...]:
    return tuple(
        (first, second, third)
        for first in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        for third in SC_DIRECTIONS
        if dot(first, second) == dot(first, third) == dot(second, third) == 0
    )


def route_directions(triad: Triad) -> tuple[Vector, ...]:
    first, second, third = triad
    return first, second, third, negate(first), negate(second), negate(third)


def route_vertices(triad: Triad) -> tuple[Vector, ...]:
    vertices = [(0, 0, 0)]
    for direction in route_directions(triad):
        vertices.append(add(vertices[-1], direction))
    assert vertices[-1] == (0, 0, 0)
    return tuple(vertices)


def scalar_triple(first: Vector, second: Vector, third: Vector) -> int:
    return dot(cross(first, second), third)


@dataclass(frozen=True)
class PetrieMatterState:
    triad: Triad
    route_phase: int
    cotangent_stage: int
    orientation: int


def current_direction(state: PetrieMatterState) -> Vector:
    return route_directions(state.triad)[state.route_phase % 6]


def next_direction(state: PetrieMatterState) -> Vector:
    return route_directions(state.triad)[(state.route_phase + 1) % 6]


def third_direction(state: PetrieMatterState) -> Vector:
    return route_directions(state.triad)[(state.route_phase + 2) % 6]


def route_chirality(state: PetrieMatterState) -> int:
    output = scalar_triple(
        current_direction(state), next_direction(state), third_direction(state)
    )
    assert output in (-1, 1)
    return output


def current_tail(state: PetrieMatterState) -> Vector:
    return route_vertices(state.triad)[state.route_phase % 6]


def charge_distribution(state: PetrieMatterState) -> dict[Vector, int]:
    tail = current_tail(state)
    head = add(tail, current_direction(state))
    return {tail: -state.orientation, head: state.orientation}


def transport_current(state: PetrieMatterState) -> tuple[tuple[Vector, Vector, int], ...]:
    old_tail = current_tail(state)
    old_head = add(old_tail, current_direction(state))
    next_head = add(old_head, next_direction(state))
    return (
        (old_tail, current_direction(state), -state.orientation),
        (old_head, next_direction(state), state.orientation),
    )


def chain_boundary(current) -> dict[Vector, int]:
    result: dict[Vector, int] = {}
    for tail, direction, amount in current:
        head = add(tail, direction)
        result[tail] = result.get(tail, 0) - amount
        result[head] = result.get(head, 0) + amount
    return {site: amount for site, amount in result.items() if amount}


def subtract_distributions(after, before) -> dict[Vector, int]:
    sites = set(after) | set(before)
    return {
        site: after.get(site, 0) - before.get(site, 0)
        for site in sites
        if after.get(site, 0) != before.get(site, 0)
    }


def step(state: PetrieMatterState, admitted: bool = True) -> PetrieMatterState:
    return PetrieMatterState(
        state.triad,
        (state.route_phase + int(admitted)) % 6,
        (state.cotangent_stage + 1) % 12,
        state.orientation,
    )


def inverse_step(state: PetrieMatterState, admitted: bool = True) -> PetrieMatterState:
    return PetrieMatterState(
        state.triad,
        (state.route_phase - int(admitted)) % 6,
        (state.cotangent_stage - 1) % 12,
        state.orientation,
    )


def stress_dyad(state: PetrieMatterState) -> Matrix:
    direction = Matrix(current_direction(state))
    return direction * direction.T


def conjugate(state: PetrieMatterState) -> PetrieMatterState:
    return PetrieMatterState(
        state.triad, state.route_phase, state.cotangent_stage, -state.orientation
    )


def transform_state(matrix, state: PetrieMatterState) -> PetrieMatterState:
    return PetrieMatterState(
        tuple(tuple(matrix_vector(matrix, direction)) for direction in state.triad),
        state.route_phase,
        state.cotangent_stage,
        state.orientation,
    )


def directional_port(state: PetrieMatterState, outgoing: bool) -> DirectionalPortState:
    return DirectionalPortState(
        (current_direction(state), next_direction(state)),
        route_chirality(state),
        state.cotangent_stage % 4,
        state.cotangent_stage,
        state.orientation,
        outgoing,
        1 - int(outgoing),
    )


def main() -> None:
    checks = 0
    triads = ordered_triads()
    group = tuple(signed_permutation_matrices())
    assert len(triads) == 48
    checks += 1

    for triad in triads:
        directions = route_directions(triad)
        vertices = route_vertices(triad)
        assert len(directions) == 6
        assert len(set(vertices[:-1])) == 6
        assert vertices[-1] == vertices[0]
        assert all(direction in SC_DIRECTIONS for direction in directions)
        assert sum((Matrix(direction) for direction in directions), Matrix.zeros(3, 1)) == Matrix.zeros(3, 1)
        mean_stress = sum(
            (Matrix(direction) * Matrix(direction).T for direction in directions),
            Matrix.zeros(3, 3),
        ) / 6
        assert mean_stress == Matrix.eye(3) / 3
        checks += 6

    states = tuple(
        PetrieMatterState(triad, route_phase, stage, orientation)
        for triad in triads
        for route_phase in range(6)
        for stage in range(12)
        for orientation in (-1, 1)
    )
    assert len(states) == 6_912
    checks += 1

    images = []
    for state in states:
        current = current_direction(state)
        following = next_direction(state)
        third = third_direction(state)
        chirality = route_chirality(state)
        assert dot(current, following) == dot(current, third) == dot(following, third) == 0
        assert propagation_direction((current, following), chirality) == third

        output = step(state)
        images.append(output)
        assert inverse_step(output) == state
        assert step(inverse_step(state)) == state
        assert subtract_distributions(
            charge_distribution(output), charge_distribution(state)
        ) == chain_boundary(transport_current(state))
        assert charge_distribution(conjugate(state)) == {
            site: -amount for site, amount in charge_distribution(state).items()
        }
        assert stress_dyad(conjugate(state)) == stress_dyad(state)

        standing = directional_port(state, False)
        outgoing = directional_port(state, True)
        assert len(port_records(standing)) == len(port_records(outgoing)) == 16
        assert field_norm(standing) == 1
        assert field_norm(outgoing) == 2
        assert field_momentum(standing) == (0, 0, 0)
        assert field_momentum(outgoing) == third
        checks += 13

    assert len(set(images)) == len(states)
    checks += 1

    # Always-admitted recurrence closes after lcm(6,12)=12 global ticks.
    for state in states:
        current = state
        for _ in range(12):
            current = step(current)
        assert current == state
        checks += 1

    # Arbitrary retained permissions separate global stage from admitted
    # material-clock phase exactly; exhaustive words through length eight.
    reference = PetrieMatterState(triads[0], 0, 0, 1)
    for length in range(9):
        for word in product((False, True), repeat=length):
            current = reference
            for permission in word:
                next_state = step(current, permission)
                assert inverse_step(next_state, permission) == current
                current = next_state
                checks += 1
            assert current.route_phase == sum(word) % 6
            assert current.cotangent_stage == length % 12
            checks += 2

    # Full signed-cubic covariance, including the route-derived pseudoscalar
    # and the endogenous port.  Phase/stage/charge are spatial spectators, so
    # one representative of those labels suffices.
    for triad in triads:
        for route_phase in range(6):
            state = PetrieMatterState(triad, route_phase, 0, 1)
            for matrix in group:
                transformed = transform_state(matrix, state)
                assert transformed.triad in triads
                assert step(transformed) == transform_state(matrix, step(state))
                assert current_direction(transformed) == tuple(
                    matrix_vector(matrix, current_direction(state))
                )
                assert third_direction(transformed) == tuple(
                    matrix_vector(matrix, third_direction(state))
                )
                assert directional_port(transformed, True) == transform_port(
                    matrix, directional_port(state, True)
                )
                checks += 5

    # The six material phases average to isotropic stress and zero current.
    for triad in triads:
        for orientation in (-1, 1):
            phase_states = tuple(
                PetrieMatterState(triad, phase, 0, orientation) for phase in range(6)
            )
            mean_stress = sum(
                (stress_dyad(state) for state in phase_states), Matrix.zeros(3, 3)
            ) / 6
            mean_current = tuple(
                Rational(
                    sum(
                        amount * direction[axis]
                        for state in phase_states
                        for _tail, direction, amount in transport_current(state)
                    ),
                    6,
                )
                for axis in range(3)
            )
            assert mean_stress == Matrix.eye(3) / 3
            assert mean_current == (0, 0, 0)
            checks += 2

    print("cubic Petrie route: 48 ordered triads, six distinct vertices, exact closure")
    print("neutral dipole clock: period 6 with exact endpoint-current continuity")
    print("common cotangent/material recurrence: period 12; arbitrary admission split exact")
    print("mean material stress=I/3 and mean route current=0")
    print("route-derived chi=det(current,next,third); third=chi*(current cross next)")
    print("endogenous port: standing norm/momentum=(1,0), outgoing=(2,third)")
    print(
        f"PASS: C6 Petrie material clock and endogenous directional port ({checks} exact checks)"
    )
    print(
        "Open: autonomous formation/binding, material translational recoil, "
        "coarse Maxwell-energy-preserving collision, Lorentz force, charged "
        "pole, gravity/lensing, Born, and alpha"
    )


if __name__ == "__main__":
    main()
