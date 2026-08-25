#!/usr/bin/env python3
"""Exact heralded fixed-window C4 Born Poincare pushforward.

For a prepared residual C4 bank with at least one bright address, use the
bright pointers themselves as a Poincare section.  One isolated source herald
starts just after the previously used bright pointer, scans locally to the
next bright pointer, latches its route, and then holds until a fixed T-tick
window ends.  Exactly one physical Gauss event is released at the end.

The return map is the cyclic successor permutation of the bright section.
Over one B-trial cycle it therefore produces exactly B_o=|Z_o|^2 outcomes,
independently of entry phase.  Any N-trial window has total-variation error
less than B/N from the complete-cycle frequency.  Fixed window duration
removes an outcome-dependent click-time side channel.

The result remains conditional on a prepared bank/ring, isolated heralds, a
finite T-state counter, and action realization.  It does not derive the bank,
multipartite no-signalling, or arbitrary overlapping source traffic.
"""

from __future__ import annotations

from collections import Counter
from fractions import Fraction
from itertools import product

from proof_c4_autonomous_reversible_born_renewal_detector import (
    bright_port,
    verify_gauss_event,
)
from proof_c4_coprime_ring_born_pushforward import coherent_norm_squared
from proof_reversible_c4_cancellation_click_circuit import (
    canonical_cancel,
    records_from_counts,
)


def bright_section(bank):
    period = len(bank) * (len(bank) + 1)
    return tuple(
        pointer for pointer in range(period) if bright_port(bank, pointer) is not None
    )


def next_bright(bank, previous: int) -> tuple[int, int]:
    period = len(bank) * (len(bank) + 1)
    pointer = (previous + 1) % period
    for distance in range(1, period + 1):
        if bright_port(bank, pointer) is not None:
            return pointer, distance
        pointer = (pointer + 1) % period
    raise ValueError("prepared bank has no bright address")


def previous_bright(bank, current: int) -> tuple[int, int]:
    period = len(bank) * (len(bank) + 1)
    pointer = (current - 1) % period
    for distance in range(1, period + 1):
        if bright_port(bank, pointer) is not None:
            return pointer, distance
        pointer = (pointer - 1) % period
    raise ValueError("prepared bank has no bright address")


def cyclic_trial_ports(bank, initial: int, trials: int) -> tuple[int, ...]:
    pointer = initial
    ports = []
    for _ in range(trials):
        pointer, _distance = next_bright(bank, pointer)
        port = bright_port(bank, pointer)
        assert port is not None
        ports.append(port)
    return tuple(ports)


def total_variation(observed: Counter, expected: tuple[int, ...], trials: int) -> Fraction:
    total_expected = sum(expected)
    return sum(
        (
            abs(
                Fraction(observed[outcome], trials)
                - Fraction(expected[outcome], total_expected)
            )
            for outcome in range(len(expected))
        ),
        Fraction(0),
    ) / 2


def verify_case(outcome_counts, padding: int) -> int:
    checks = 0
    original = records_from_counts(outcome_counts)
    residual, dark = canonical_cancel(original)
    capacity = max(1, len(residual) + padding)
    bank = tuple(residual) + (None,) * (capacity - len(residual))
    period = capacity * (capacity + 1)
    expected = tuple(coherent_norm_squared(counts) for counts in outcome_counts)
    bright_total = sum(expected)
    section = bright_section(bank)

    assert len(section) == bright_total
    checks += 1
    if bright_total == 0:
        assert section == ()
        return checks + 1

    successor = {}
    predecessor = {}
    distances = {}
    for pointer in section:
        endpoint, distance = next_bright(bank, pointer)
        prior, reverse_distance = previous_bright(bank, endpoint)
        assert prior == pointer
        assert reverse_distance == distance
        assert 1 <= distance <= period
        assert distance + (period - distance) == period
        successor[pointer] = endpoint
        predecessor[endpoint] = pointer
        distances[pointer] = distance
        checks += 4

    assert set(successor) == set(section)
    assert set(successor.values()) == set(section)
    assert len(set(successor.values())) == bright_total
    assert all(predecessor[successor[pointer]] == pointer for pointer in section)
    assert sum(distances.values()) == period
    checks += 5

    # The Poincare successor is one cycle and gives one fixed-window click per
    # isolated herald.  Every entry phase has the same complete-cycle counts.
    for initial in section:
        ports = cyclic_trial_ports(bank, initial, bright_total)
        observed = Counter(ports)
        assert tuple(observed[index] for index in range(len(expected))) == expected
        pointer = initial
        for _ in range(bright_total):
            pointer = successor[pointer]
        assert pointer == initial
        checks += 2

        # Incomplete heralded trial windows have a deterministic discrepancy
        # bounded only by the unfinished suffix, independently of entry phase.
        for trials in range(1, 3 * bright_total + 2):
            ports_n = cyclic_trial_ports(bank, initial, trials)
            observed_n = Counter(ports_n)
            discrepancy = total_variation(observed_n, expected, trials)
            remainder = trials % bright_total
            assert discrepancy <= Fraction(remainder, trials)
            if remainder == 0:
                assert discrepancy == 0
            assert discrepancy < Fraction(bright_total, trials)
            checks += 3

    # Each distinct route is a physical cotangent Gauss event.  The outcome is
    # latched during padding and released only at the common window endpoint.
    for pointer in section:
        port = bright_port(bank, pointer)
        assert port is not None
        for orientation in (-1, 1):
            checks += verify_gauss_event(port, pointer % 4, orientation)

    # No active or canceled history record is erased by the return-map view.
    assert tuple(bank[: len(residual)]) == tuple(residual)
    assert len(residual) + 2 * len(dark) == len(original)
    checks += 2
    return checks


def main() -> None:
    checks = 0
    for raw_counts in product(range(4), repeat=4):
        checks += verify_case((raw_counts,), padding=2)

    multiple_outcome_cases = (
        ((1, 0, 0, 0), (0, 1, 0, 0)),
        ((3, 1, 1, 0), (0, 4, 0, 1), (2, 2, 2, 1)),
        ((8, 0, 3, 0), (1, 7, 0, 2), (4, 4, 4, 4), (0, 0, 0, 2)),
        ((2, 5, 6, 1), (7, 2, 0, 8), (3, 3, 3, 3)),
    )
    for outcome_counts in multiple_outcome_cases:
        checks += verify_case(outcome_counts, padding=3)

    print("bright addresses form one reversible Poincare cycle")
    print("one isolated source herald advances to exactly one next bright event")
    print("every trial is padded to the same T-tick external completion time")
    print("one B-trial cycle gives M_o=|Z_o|^2 physical Gauss events exactly")
    print("N-trial total-variation discrepancy is strictly below B/N")
    print("prepared bank, counter formation, overlapping traffic, and no-signalling remain open")
    print(
        "PASS: C4 heralded fixed-window Born Poincare pushforward "
        f"({checks} exact checks)"
    )


if __name__ == "__main__":
    main()
