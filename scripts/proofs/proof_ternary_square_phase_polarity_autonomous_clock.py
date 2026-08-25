#!/usr/bin/env python3
"""Exact ternary-square carrier and autonomous crossing-clock certificate.

Two unrestricted ternary slots contain exactly the nine states required for
one blank plus an oriented C4 token: 1 + 2*4 = 9.  This certificate constructs
polynomial capacity/polarity readouts, a C4 action, a commuting charge-
conjugation involution, the reversible reserve/link actualization map, and a
target-free phase-crossing recurrence.

This is a finite local transfer-map theorem.  It does not derive the rule from
a variational principle, form a stable translating particle, produce field
propagators, prove lensing/Born statistics, or measure alpha.
"""

from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from itertools import product

from proof_c18_uniform_token_blocking import C18_LINE_MOMENTS


T = (-1, 0, 1)
TERNARY_SQUARE = tuple(product(T, repeat=2))
BLANK = (0, 0)
TOKENS = tuple(state for state in TERNARY_SQUARE if state != BLANK)


def occupation(state: tuple[int, int]) -> int:
    """One for every nonblank ternary-square state, zero for the blank."""

    u, v = state
    return u * u + v * v - u * u * v * v


def capacity(state: tuple[int, int]) -> int:
    return 1 - occupation(state)


def polarity(state: tuple[int, int]) -> int:
    """Axis shell is +, diagonal shell is -, and the blank is unsigned."""

    u, v = state
    return u * u + v * v - 3 * u * u * v * v


def phase_coordinates(state: tuple[int, int]) -> tuple[int, int]:
    """Return the shared one-hot C4 phase, independent of polarity shell."""

    u, v = state
    diagonal = u * u * v * v
    axis = u * u + v * v - 2 * diagonal
    phase_u = Fraction(axis * u, 1) + Fraction(diagonal * (u + v), 2)
    phase_v = Fraction(axis * v, 1) + Fraction(diagonal * (v - u), 2)
    assert phase_u.denominator == phase_v.denominator == 1
    return int(phase_u), int(phase_v)


PHASE_POINTS = ((1, 0), (0, 1), (-1, 0), (0, -1))
PHASE_INDEX = {point: index for index, point in enumerate(PHASE_POINTS)}


def phase_index(state: tuple[int, int]) -> int:
    assert occupation(state) == 1
    return PHASE_INDEX[phase_coordinates(state)]


def rotate(state: tuple[int, int], turns: int = 1) -> tuple[int, int]:
    """Counterclockwise C4 action; the opposite clock orientation is inverse."""

    u, v = state
    for _ in range(turns % 4):
        u, v = -v, u
    return u, v


def conjugate_token(state: tuple[int, int]) -> tuple[int, int]:
    """Exchange polarity shells while preserving the C4 phase label."""

    u, v = state
    diagonal = u * u * v * v
    axis = u * u + v * v - 2 * diagonal
    out_u = Fraction(axis * (u - v), 1) + Fraction(diagonal * (u + v), 2)
    out_v = Fraction(axis * (u + v), 1) + Fraction(diagonal * (v - u), 2)
    assert out_u.denominator == out_v.denominator == 1
    output = int(out_u), int(out_v)
    assert output in TERNARY_SQUARE
    return output


@dataclass(frozen=True)
class LocalState:
    left: int
    right: int
    link: tuple[int, int]
    reserve: tuple[int, int]

    def __post_init__(self) -> None:
        assert self.left in T and self.right in T
        assert self.link in TERNARY_SQUARE
        assert self.reserve in TERNARY_SQUARE


def token_count(state: LocalState) -> int:
    return occupation(state.link) + occupation(state.reserve)


def charge(state: LocalState) -> int:
    return state.left + state.right


def actualize(state: LocalState) -> LocalState:
    """Endogenous reserve/link ownership exchange with fail-closed domain."""

    if (
        state.left == 0
        and state.right == 0
        and state.link == BLANK
        and occupation(state.reserve) == 1
    ):
        epsilon = polarity(state.reserve)
        return LocalState(epsilon, -epsilon, state.reserve, BLANK)

    if (
        occupation(state.link) == 1
        and state.reserve == BLANK
        and state.left == polarity(state.link)
        and state.right == -polarity(state.link)
    ):
        return LocalState(0, 0, BLANK, state.link)

    return state


def rotate_state(state: LocalState, turns: int = 1) -> LocalState:
    return LocalState(
        state.left,
        state.right,
        rotate(state.link, turns),
        rotate(state.reserve, turns),
    )


def conjugate_state(state: LocalState) -> LocalState:
    return LocalState(
        -state.left,
        -state.right,
        conjugate_token(state.link),
        conjugate_token(state.reserve),
    )


def phase_gated_actualize(state: LocalState, crossing: int = 0) -> LocalState:
    token = state.link if occupation(state.link) else state.reserve
    if occupation(token) == 1 and phase_index(token) == crossing % 4:
        return actualize(state)
    return state


def tick(state: LocalState, crossing: int = 0) -> LocalState:
    """One selected crossing transaction followed by one C4 phase advance."""

    return rotate_state(phase_gated_actualize(state, crossing), 1)


def iterate(state: LocalState, count: int, crossing: int = 0) -> LocalState:
    for _ in range(count):
        state = tick(state, crossing)
    return state


def valid_owned_states() -> tuple[LocalState, ...]:
    states: list[LocalState] = []
    for token in TOKENS:
        epsilon = polarity(token)
        states.append(LocalState(0, 0, BLANK, token))
        states.append(LocalState(epsilon, -epsilon, token, BLANK))
    return tuple(states)


def main() -> None:
    checks = 0

    # Cardinality is exact and minimal among products of ternary registers.
    assert len(TERNARY_SQUARE) == 9 == 1 + 4 * 2
    assert 3 < 9 <= 3**2
    checks += 2

    for state in TERNARY_SQUARE:
        n = occupation(state)
        c = capacity(state)
        epsilon = polarity(state)
        assert n in (0, 1) and c in (0, 1) and n + c == 1
        assert (epsilon == 0) == (state == BLANK)
        if n:
            assert epsilon in (-1, 1)
            assert phase_coordinates(state) in PHASE_POINTS
        else:
            assert phase_coordinates(state) == BLANK
        checks += 4

        rotated = state
        for _ in range(4):
            rotated = rotate(rotated)
        assert rotated == state
        assert occupation(rotate(state)) == n
        assert polarity(rotate(state)) == epsilon
        checks += 3

        conjugated = conjugate_token(state)
        assert conjugate_token(conjugated) == state
        assert occupation(conjugated) == n
        assert phase_coordinates(conjugated) == phase_coordinates(state)
        assert polarity(conjugated) == -epsilon
        assert conjugate_token(rotate(state)) == rotate(conjugated)
        checks += 5

    assert len({(phase_index(token), polarity(token)) for token in TOKENS}) == 8
    checks += 1

    # The ownership transfer is a charge-conjugation- and C4-equivariant
    # involution on its whole finite local domain, with invalid states fixed.
    all_local_states = (
        LocalState(left, right, link, reserve)
        for left, right, link, reserve in product(T, T, TERNARY_SQUARE, TERNARY_SQUARE)
    )
    for state in all_local_states:
        output = actualize(state)
        assert actualize(output) == state
        assert token_count(output) == token_count(state)
        assert charge(output) == charge(state)
        assert actualize(rotate_state(state)) == rotate_state(output)
        assert actualize(conjugate_state(state)) == conjugate_state(output)
        checks += 5

    # Selecting one phase crossing gives an autonomous eight-tick local
    # recurrence.  Four ticks return the phase but exchange ownership; eight
    # return the complete state.  The four possible crossing sections form a
    # C4-covariant family.
    for state in valid_owned_states():
        assert iterate(state, 8) == state
        assert iterate(state, 4) == actualize(state)
        assert all(iterate(state, power) != state for power in range(1, 8))
        assert tick(conjugate_state(state)) == conjugate_state(tick(state))
        checks += 4

        for crossing in range(4):
            left = rotate_state(phase_gated_actualize(rotate_state(state, -1), crossing))
            right = phase_gated_actualize(state, (crossing + 1) % 4)
            assert left == right
            checks += 1

        orbit = [iterate(state, step) for step in range(8)]
        manifested = [item for item in orbit if occupation(item.link)]
        assert len(manifested) == 4
        assert sum(item.left * item.left + item.right * item.right for item in orbit) == 8
        assert sum(phase_coordinates(item.link)[0] for item in manifested) == 0
        assert sum(phase_coordinates(item.link)[1] for item in manifested) == 0
        assert sum(polarity(item.link) * phase_coordinates(item.link)[0] for item in manifested) == 0
        assert sum(polarity(item.link) * phase_coordinates(item.link)[1] for item in manifested) == 0
        checks += 6

        # Every C18 capacity dyad has the same exact cycle mean: half blank
        # (M/9) and half occupied (M/18), hence M/12 and deficit -M/36.
        for moment in C18_LINE_MOMENTS:
            for component in moment:
                mean_capacity = (
                    Fraction(4, 8) * Fraction(component, 9)
                    + Fraction(4, 8) * Fraction(component, 18)
                )
                assert mean_capacity == Fraction(component, 12)
                assert mean_capacity - Fraction(component, 9) == -Fraction(component, 36)
                checks += 2

    # Capacity/backpressure stalls global ticks without changing the local
    # orbit: only the number of admitted ticks matters.
    for state in valid_owned_states():
        for word in product((0, 1), repeat=8):
            output = state
            for admitted in word:
                if admitted:
                    output = tick(output)
            assert output == iterate(state, sum(word))
            checks += 1

    print(f"PASS: ternary-square phase/polarity carrier and autonomous clock ({checks} exact checks)")
    print("two ternary slots encode blank plus C4 x Z2 exactly; no separate orientation register")
    print("phase-crossing transaction has exact period 8 and mean capacity deficit -M/36")
    print("Open: rule derivation, routing/formation, blocking, stable matter, poles, lensing, Born, alpha")


if __name__ == "__main__":
    main()
