#!/usr/bin/env python3
"""Exact flat-band boundary for a closed oriented plaquette C4 orbit.

A token that advances around the four directed edges of one elementary
plaquette is a local reversible C4 clock.  Its four-step net displacement is
zero, so the exact Bloch transfer matrix satisfies P(k)^4=I and has
characteristic polynomial lambda^4-1 independent of k.  It cannot by itself
produce a Maxwell light cone.
"""

from __future__ import annotations

from sympy import Matrix, expand, symbols

from proof_oriented_bond_plaquette_hodge_maxwell_target import (
    SC_DIRECTIONS,
    axial_normal,
    canonical_oriented_plane,
    dot,
    negate,
)


def oriented_states():
    return {
        canonical_oriented_plane(first, second, orientation)
        for first in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(first, second) == 0
        for orientation in (-1, 1)
    }


def ordered_spanning_vectors(state):
    first, second, orientation = state
    return (first, second) if orientation == 1 else (second, first)


def boundary_steps(state):
    first, second = ordered_spanning_vectors(state)
    return (first, second, negate(first), negate(second))


def monomial(step, variables):
    return variables[0] ** step[0] * variables[1] ** step[1] * variables[2] ** step[2]


def bloch_cycle(state, variables):
    transfer = Matrix.zeros(4, 4)
    for index, step in enumerate(boundary_steps(state)):
        transfer[(index + 1) % 4, index] = monomial(step, variables)
    return transfer


def main() -> None:
    checks = 0
    x, y, z = symbols("x y z", nonzero=True)
    eigenvalue = symbols("eigenvalue")
    variables = (x, y, z)
    states = oriented_states()
    assert len(states) == 24
    checks += 1

    for state in states:
        first, second = ordered_spanning_vectors(state)
        assert axial_normal(state) == (
            first[1] * second[2] - first[2] * second[1],
            first[2] * second[0] - first[0] * second[2],
            first[0] * second[1] - first[1] * second[0],
        )
        steps = boundary_steps(state)
        assert tuple(sum(step[component] for step in steps) for component in range(3)) == (
            0,
            0,
            0,
        )
        transfer = bloch_cycle(state, variables)
        assert transfer**4 == Matrix.eye(4)
        assert expand(
            transfer.charpoly(eigenvalue).as_expr() - (eigenvalue**4 - 1)
        ) == 0
        checks += 4

        reversed_state = canonical_oriented_plane(state[0], state[1], -state[2])
        reversed_transfer = bloch_cycle(reversed_state, variables)
        assert reversed_transfer**4 == Matrix.eye(4)
        assert expand(
            reversed_transfer.charpoly(eigenvalue).as_expr() - (eigenvalue**4 - 1)
        ) == 0
        checks += 2

    print(f"PASS: oriented plaquette C4 flat-band boundary ({checks} exact checks)")
    print("24 oriented plaquette carriers; every boundary orbit has zero net displacement")
    print("P(k)^4=I and characteristic=lambda^4-1 for both orientations")
    print("local four-cycle is a clock/circulation carrier, not a propagating light cone")
    print("Open: reversible shared-edge exchange between neighboring plaquettes")


if __name__ == "__main__":
    main()
