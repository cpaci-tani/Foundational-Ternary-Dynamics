#!/usr/bin/env python3
"""Exact algebra checks for the phase-complete bond-action scope.

This certificate checks only the finite alphabet, gate positivity, C18 moment
rank, and the conditional transverse-traceless dimension count.  It does not
test formation, propagation, lensing, Born statistics, or a native coupling.
"""

from __future__ import annotations

from fractions import Fraction
from itertools import product

from proof_moore_bond_capacity_type_census import (
    FCC_LINES,
    SC_LINES,
    rational_rank,
    second_moment_rank,
)


C4_ZERO = ((0, 0), (1, 0), (0, 1), (-1, 0), (0, -1))


def rotate(state: tuple[int, int]) -> tuple[int, int]:
    u, v = state
    return (v, -u)


def capacity(state: tuple[int, int]) -> int:
    u, v = state
    occupation = u * u + v * v
    assert occupation in (0, 1)
    return 1 - occupation


def token_count(state: tuple[int, int]) -> int:
    return 1 - capacity(state)


def manifest_reference(
    epsilon: int,
    link: tuple[int, int],
    reserve: tuple[int, int],
) -> tuple[int, int, tuple[int, int], tuple[int, int]]:
    """Reference witness only; not an autonomous selection law."""

    assert epsilon in (-1, 1)
    assert link == (0, 0)
    assert reserve != (0, 0)
    return (epsilon, -epsilon, reserve, (0, 0))


def unmanifest_reference(
    state_left: int,
    state_right: int,
    link: tuple[int, int],
    reserve: tuple[int, int],
) -> tuple[int, int, tuple[int, int], tuple[int, int]]:
    assert state_left in (-1, 1)
    assert state_right == -state_left
    assert link != (0, 0)
    assert reserve == (0, 0)
    return (0, 0, (0, 0), link)


def occupancy_gate(left_state: int, right_state: int) -> int:
    left = left_state * left_state
    right = right_state * right_state
    return 1 - (left - right) ** 2


def tt_constraint_rank(kx: int, ky: int, kz: int) -> int:
    # Symmetric tensor coordinates: xx, yy, zz, xy, xz, yz.
    rows = [
        [1, 1, 1, 0, 0, 0],
        [kx, 0, 0, ky, kz, 0],
        [0, ky, 0, kx, 0, kz],
        [0, 0, kz, 0, kx, ky],
    ]
    return rational_rank(rows)


def main() -> None:
    checks = 0

    image = {rotate(state) for state in C4_ZERO}
    assert image == set(C4_ZERO)
    checks += 1

    for state in C4_ZERO:
        rotated = state
        for _ in range(4):
            rotated = rotate(rotated)
        assert rotated == state
        assert capacity(rotate(state)) == capacity(state)
        checks += 2
    assert rotate((0, 0)) == (0, 0)
    checks += 1

    for epsilon in (-1, 1):
        for phase_token in C4_ZERO[1:]:
            before = (0, 0, (0, 0), phase_token)
            after = manifest_reference(epsilon, before[2], before[3])
            assert after[:2] == (epsilon, -epsilon)
            assert after[0] + after[1] == before[0] + before[1] == 0
            assert token_count(before[2]) + token_count(before[3]) == 1
            assert token_count(after[2]) + token_count(after[3]) == 1
            assert capacity(before[2]) - capacity(after[2]) == 1
            assert unmanifest_reference(*after) == before
            checks += 6

    for left_state, right_state in product((-1, 0, 1), repeat=2):
        gate = occupancy_gate(left_state, right_state)
        assert gate in (0, 1)
        assert gate == int((left_state == 0) == (right_state == 0))
        checks += 2

    # In the L/R chart, the per-bond block has common eigenvalue c*g_m and
    # relative eigenvalue c.  Verify the exact eigenvector identities.
    for c, gate in product((0, 1), repeat=2):
        matrix = (
            (Fraction(c * (gate + 1), 2), Fraction(c * (gate - 1), 2)),
            (Fraction(c * (gate - 1), 2), Fraction(c * (gate + 1), 2)),
        )
        common = tuple(sum(matrix[row][column] for column in range(2)) for row in range(2))
        relative = tuple(matrix[row][0] - matrix[row][1] for row in range(2))
        assert common == (c * gate, c * gate)
        assert relative == (c, -c)
        assert c * gate >= 0 and c >= 0
        checks += 3

    c18_lines = SC_LINES + FCC_LINES
    assert len(c18_lines) == 9
    assert second_moment_rank(c18_lines) == 6
    checks += 2

    # The document supplies the analytic nonzero-k proof.  This finite exact
    # guard checks every nonzero integer direction in a symmetric local box.
    for kx, ky, kz in product(range(-2, 3), repeat=3):
        if (kx, ky, kz) == (0, 0, 0):
            continue
        assert tt_constraint_rank(kx, ky, kz) == 4
        checks += 1

    assert 6 - tt_constraint_rank(0, 0, 1) == 2
    checks += 1

    print(f"PASS: phase-complete bond action skeleton ({checks} exact checks)")
    print("Scope only: no formation, lensing, spin-2 pole, Born, or alpha claim")


if __name__ == "__main__":
    main()
