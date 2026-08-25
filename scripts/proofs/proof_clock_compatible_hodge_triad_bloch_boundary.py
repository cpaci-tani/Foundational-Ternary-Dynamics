#!/usr/bin/env python3
"""Exact first-order Bloch boundary for the clock-compatible Hodge triad.

This certificate composes the exact O_h x C12-equivariant seven-invariant pair
collision with shared-edge streaming.  At k=0 the collision and internal tick
commute and preserve record number plus the polar/axial triad fields.  The
first-order reduced Bloch generator is computed exactly.

The result is a single scalar--longitudinal electric acoustic pair with speed
1/3.  Both transverse electric components and all magnetic components have
zero k-linear generator.  The route therefore does not produce the Hodge curl
or a Maxwell cone, even though it solves the clock/collision compatibility
problem.

No fitted parameter, physical target search, or numerical eigensolver is used.
"""

from __future__ import annotations

from itertools import combinations

from sympy import I, Matrix, Rational, expand, symbols
from sympy.polys.matrices import DomainMatrix

import proof_clock_compatible_hodge_triad_equivariant_collision as collision_proof
from proof_clock_compatible_hodge_triad_readout import internal_tick, triad_readout


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def occupation_delta(before, after, size: int) -> Matrix:
    delta = [0] * size
    for index in after:
        delta[index] += 1
    for index in before:
        delta[index] -= 1
    return Matrix(delta)


def main() -> None:
    checks = 0
    collision_proof.main()
    data = collision_proof.CERTIFICATE_DATA
    assert data is not None
    states = data["states"]
    collision = data["collision"]
    state_index = {state: index for index, state in enumerate(states)}
    size = len(states)
    assert size == 192
    checks += 2

    correction = Matrix.zeros(size, size)
    transition_count = 0
    for before in combinations(range(size), 2):
        after = collision[before]
        if before >= after:
            continue
        delta = occupation_delta(before, after, size)
        correction -= 2 * delta * delta.T
        transition_count += 1
    assert transition_count == 9168
    assert correction == correction.T
    assert exact_rank(correction) == 185
    checks += 3

    left = Matrix.vstack(
        Matrix([[1] * size]),
        *(
            Matrix([[triad_readout(state)[component] for state in states]])
            for component in range(6)
        ),
    )
    right = left.T
    gram = left * right
    assert gram == 192 * Matrix.eye(7)
    assert left * correction == Matrix.zeros(7, size)
    assert correction * right == Matrix.zeros(size, 7)
    checks += 3

    internal_image = tuple(state_index[internal_tick(state)] for state in states)
    internal = Matrix.zeros(size, size)
    for source, target in enumerate(internal_image):
        internal[target, source] = 1
    assert internal**12 == Matrix.eye(size)
    assert internal * correction == correction * internal
    assert left * internal == left
    assert internal * right == right
    checks += 4

    reduced_axes = []
    for component in range(3):
        displacement = Matrix.diag(
            *[state[0][0][component] for state in states]
        )
        reduced_axes.append(left * displacement * right * gram.inv())

    expected_axes = []
    for component in range(3):
        expected = Matrix.zeros(7, 7)
        expected[0, 1 + component] = Rational(1, 3)
        expected[1 + component, 0] = Rational(1, 3)
        expected_axes.append(expected)
    assert reduced_axes == expected_axes
    checks += 1

    kx, ky, kz, eigenvalue = symbols("kx ky kz eigenvalue")
    reduced = kx * reduced_axes[0] + ky * reduced_axes[1] + kz * reduced_axes[2]
    wave_number_squared = kx**2 + ky**2 + kz**2
    assert expand(
        reduced.charpoly(eigenvalue).as_expr()
        - eigenvalue**5 * (9 * eigenvalue**2 - wave_number_squared) / 9
    ) == 0
    generator = -I * reduced
    assert expand(
        generator.charpoly(eigenvalue).as_expr()
        - eigenvalue**5 * (9 * eigenvalue**2 + wave_number_squared) / 9
    ) == 0
    checks += 2

    # Along z, a Maxwell reduction would retain (E_x,E_y,B_x,B_y) with two
    # nonzero curl pairs.  This exact transverse block vanishes instead.
    transverse_indices = (1, 2, 4, 5)
    transverse = reduced_axes[2].extract(transverse_indices, transverse_indices)
    scalar_longitudinal = reduced_axes[2].extract((0, 3), (0, 3))
    assert transverse == Matrix.zeros(4, 4)
    assert scalar_longitudinal == Matrix(
        [[0, Rational(1, 3)], [Rational(1, 3), 0]]
    )
    checks += 2

    print("collision_rank=185, collision_nullity=7")
    print("collision_commutes_with_internal_C12=true")
    print("first_order_characteristic=lambda^5*(lambda^2+|k|^2/9)")
    print("one scalar-longitudinal electric pair has speed=1/3")
    print("transverse_EB_first_order_block=0_4x4")
    print(
        "PASS: clock-compatible Hodge-triad Bloch boundary "
        f"({checks} exact checks plus parent certificate)"
    )
    print(
        "Scoped closed negative: clock-averaged triad transport is acoustic, "
        "not Maxwell/Hodge"
    )
    print(
        "Required repair: retain an edge-face stagger or cotangent orientation "
        "whose clock-compatible invariant flux contains an antisymmetric curl"
    )


if __name__ == "__main__":
    main()
