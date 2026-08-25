#!/usr/bin/env python3
"""Exact certificate for the C4 coprime-ring pair pushforward.

Two cyclic address heads of lengths L and L+1 advance one slot per tick over a
shared residual-record bank.  Their joint pointer is one orbit of length
L(L+1), so every ordered pair of occupied addresses occurs exactly once.  The
same-outcome/same-rail click count is then the coherent norm squared.

This proves the finite deterministic enumerator.  It does not prove that the
FTD action prepares duplicate residual registers, performs reversible local
cancellation, pays detector work, or recovers general complex amplitudes.
"""

from __future__ import annotations

from collections import Counter
from fractions import Fraction
from itertools import product
from math import gcd


PhaseCounts = tuple[int, int, int, int]
Record = tuple[int, str, int]


def coherent_norm_squared(counts: PhaseCounts) -> int:
    n_0, n_1, n_2, n_3 = counts
    return (n_0 - n_2) ** 2 + (n_1 - n_3) ** 2


def residual_records(outcome: int, counts: PhaseCounts) -> list[Record]:
    """Return distinguishable active records after canonical rail cancellation."""

    n_0, n_1, n_2, n_3 = counts
    records: list[Record] = []

    real_sign = 0 if n_0 >= n_2 else 2
    for serial in range(abs(n_0 - n_2)):
        records.append((outcome, "R", 10_000 * outcome + 100 * real_sign + serial))

    imag_sign = 1 if n_1 >= n_3 else 3
    for serial in range(abs(n_1 - n_3)):
        records.append((outcome, "I", 10_000 * outcome + 100 * imag_sign + serial))

    return records


def ring_orbit(length: int) -> list[tuple[int, int]]:
    assert length >= 1
    return [
        (tick % length, tick % (length + 1))
        for tick in range(length * (length + 1))
    ]


def enumerate_clicks(
    records: list[Record],
    capacity: int,
) -> tuple[Counter[int], int, int]:
    """Run one exact joint orbit and count physical compatibility clicks."""

    assert len(records) <= capacity
    blank = None
    record_bank: list[Record | None] = records + [blank] * (capacity - len(records))

    clicks: Counter[int] = Counter()
    occupied_pairs = 0
    for index_a, index_b in ring_orbit(capacity):
        left = record_bank[index_a]
        # The second address head has one extra physical delay/blank cell.
        right = record_bank[index_b] if index_b < capacity else blank
        if left is None or right is None:
            continue
        occupied_pairs += 1
        left_outcome, left_rail, _left_serial = left
        right_outcome, right_rail, _right_serial = right
        if left_outcome == right_outcome and left_rail == right_rail:
            clicks[left_outcome] += 1

    return clicks, occupied_pairs, capacity * (capacity + 1)


def verify_mixer_geometry(max_length: int) -> int:
    checks = 0
    for length in range(1, max_length + 1):
        assert gcd(length, length + 1) == 1
        orbit = ring_orbit(length)
        assert len(orbit) == length * (length + 1)
        assert len(set(orbit)) == len(orbit)
        assert set(orbit) == set(product(range(length), range(length + 1)))
        checks += 4

        nonblank_pairs = [(a, b) for a, b in orbit if b < length]
        assert len(nonblank_pairs) == length * length
        assert set(nonblank_pairs) == set(product(range(length), repeat=2))
        checks += 2
    return checks


def verify_single_outcome_counts() -> int:
    checks = 0
    for raw_counts in product(range(6), repeat=4):
        counts: PhaseCounts = raw_counts
        records = residual_records(0, counts)
        capacity = max(1, len(records) + 2)
        clicks, occupied_pairs, period = enumerate_clicks(records, capacity)
        expected = coherent_norm_squared(counts)

        assert occupied_pairs == len(records) ** 2
        assert clicks[0] == expected
        assert sum(clicks.values()) == expected
        assert period == capacity * (capacity + 1)
        checks += 4
    return checks


def verify_multiple_outcomes() -> int:
    cases: tuple[tuple[PhaseCounts, ...], ...] = (
        ((1, 0, 0, 0), (0, 1, 0, 0)),
        ((3, 1, 1, 0), (0, 4, 0, 1), (2, 2, 2, 1)),
        ((8, 0, 3, 0), (1, 7, 0, 2), (4, 4, 4, 4), (0, 0, 0, 2)),
        ((2, 5, 6, 1), (7, 2, 0, 8), (3, 3, 3, 3)),
    )
    checks = 0

    for outcome_counts in cases:
        records = [
            record
            for outcome, counts in enumerate(outcome_counts)
            for record in residual_records(outcome, counts)
        ]
        capacity = max(1, len(records) + 3)
        clicks, occupied_pairs, period = enumerate_clicks(records, capacity)
        expected = tuple(coherent_norm_squared(counts) for counts in outcome_counts)

        assert occupied_pairs == len(records) ** 2
        assert tuple(clicks[index] for index in range(len(expected))) == expected
        assert sum(clicks.values()) == sum(expected)
        checks += 3

        if sum(expected) > 0:
            for outcome, norm in enumerate(expected):
                empirical = Fraction(clicks[outcome], sum(clicks.values()))
                born = Fraction(norm, sum(expected))
                assert empirical == born
                checks += 1

        # The unconditional time rate is also exact; conditioning on a click
        # removes incompatible and blank pointer states.
        assert Fraction(sum(clicks.values()), period) == Fraction(sum(expected), period)
        checks += 1

    return checks


def main() -> None:
    checks = 0
    checks += verify_mixer_geometry(max_length=32)
    checks += verify_single_outcome_counts()
    checks += verify_multiple_outcomes()

    print(f"PASS: C4 coprime-ring Born pushforward ({checks} exact checks)")
    print("Finite equal-weight enumerator only: cancellation/action/general amplitudes remain open")


if __name__ == "__main__":
    main()
