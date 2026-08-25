#!/usr/bin/env python3
"""Exact autonomous reversible C4 Born renewal detector.

For a prepared residual C4 record bank, the existing coprime address rings
visit every ordered bank pair once.  This certificate replaces the prepared
T-cell detector tape by one balanced-ternary detector state q in {-1, 0, +1}.

At a dark address pair, the pointers advance immediately.  At a bright pair,
the detector follows the three-state local path

    ready (0) -> manifested (+1) -> recovery (-1) -> next address ready.

The rule extends to a permutation of the full pointer x detector product.
Misprepared nonready states at dark pointers form isolated two-cycles and
cannot enter the operational orbit.  Every bright pair therefore produces
exactly one exclusive manifested Gauss event and the same detector/source
resource resets autonomously.  Event counts remain exactly |Z_o|^2.  The
result is conditional on the prepared residual bank and on the existing
shared-bank/two-ring physical selection; it does not derive source
preparation, externally heralded one-click trials, or multipartite
no-signalling.
"""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
from fractions import Fraction
from itertools import product

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_c4_coprime_ring_born_pushforward import coherent_norm_squared
from proof_cotangent_stabilizer_packet_gauss_source import (
    boundary,
    packet,
    packet_field,
    scale_integer,
)
from proof_reversible_c4_cancellation_click_circuit import (
    Record,
    canonical_cancel,
    click_compatible,
    records_from_counts,
)


READY = 0
MANIFESTED = 1
RECOVERY = -1
ORIGIN = (0, 0, 0)


@dataclass(frozen=True)
class RenewalState:
    pointer: int
    detector: int


def addressed_pair(
    bank: tuple[Record | None, ...], pointer: int
) -> tuple[Record | None, Record | None]:
    """Read one state of the consecutive coprime address rings."""
    length = len(bank)
    left_address = pointer % length
    right_address = pointer % (length + 1)
    left = bank[left_address]
    right = bank[right_address] if right_address < length else None
    return left, right


def bright_port(bank: tuple[Record | None, ...], pointer: int) -> int | None:
    left, right = addressed_pair(bank, pointer)
    if not click_compatible(left, right):
        return None
    assert left is not None and right is not None
    assert left.outcome == right.outcome
    return left.outcome


def in_total_state_space(
    bank: tuple[Record | None, ...], state: RenewalState
) -> bool:
    period = len(bank) * (len(bank) + 1)
    return 0 <= state.pointer < period and state.detector in (
        RECOVERY,
        READY,
        MANIFESTED,
    )


def renewal_step(
    bank: tuple[Record | None, ...], state: RenewalState
) -> RenewalState:
    """Forward local renewal permutation on the full finite product."""
    assert in_total_state_space(bank, state)
    period = len(bank) * (len(bank) + 1)
    port = bright_port(bank, state.pointer)
    if state.detector == READY:
        if port is None:
            return RenewalState((state.pointer + 1) % period, READY)
        return RenewalState(state.pointer, MANIFESTED)
    if state.detector == MANIFESTED:
        return RenewalState(state.pointer, RECOVERY)
    assert state.detector == RECOVERY
    if port is not None:
        return RenewalState((state.pointer + 1) % period, READY)
    # A misprepared recovery state at a dark pointer is quarantined in a
    # two-cycle with the corresponding manifested label.
    return RenewalState(state.pointer, MANIFESTED)


def renewal_inverse(
    bank: tuple[Record | None, ...], state: RenewalState
) -> RenewalState:
    """Unique inverse; the predecessor predicate chooses ready vs recovery."""
    assert in_total_state_space(bank, state)
    period = len(bank) * (len(bank) + 1)
    if state.detector == MANIFESTED:
        if bright_port(bank, state.pointer) is None:
            return RenewalState(state.pointer, RECOVERY)
        return RenewalState(state.pointer, READY)
    if state.detector == RECOVERY:
        return RenewalState(state.pointer, MANIFESTED)
    previous = (state.pointer - 1) % period
    if bright_port(bank, previous) is None:
        return RenewalState(previous, READY)
    return RenewalState(previous, RECOVERY)


def total_state_space(
    bank: tuple[Record | None, ...],
) -> tuple[RenewalState, ...]:
    period = len(bank) * (len(bank) + 1)
    return tuple(
        RenewalState(pointer, detector)
        for pointer in range(period)
        for detector in (RECOVERY, READY, MANIFESTED)
    )


def operational_state_space(
    bank: tuple[Record | None, ...],
) -> tuple[RenewalState, ...]:
    """Invariant component reached from a correctly prepared ready state."""
    period = len(bank) * (len(bank) + 1)
    states: list[RenewalState] = []
    for pointer in range(period):
        states.append(RenewalState(pointer, READY))
        if bright_port(bank, pointer) is not None:
            states.append(RenewalState(pointer, MANIFESTED))
            states.append(RenewalState(pointer, RECOVERY))
    return tuple(states)


def complete_orbit(
    bank: tuple[Record | None, ...],
) -> tuple[RenewalState, ...]:
    initial = RenewalState(0, READY)
    orbit: list[RenewalState] = []
    seen: set[RenewalState] = set()
    state = initial
    while state not in seen:
        seen.add(state)
        orbit.append(state)
        state = renewal_step(bank, state)
    assert state == initial
    return tuple(orbit)


def verify_gauss_event(port: int, phase: int, orientation: int) -> int:
    """One manifested renewal state carries one canonical Gauss edge event."""
    checks = 0
    route = SC_DIRECTIONS[port]
    packet_direction = scale_integer(orientation, route)
    records = packet(packet_direction, phase)
    electric_magnetic = packet_field(records, 0)
    assert electric_magnetic[:3] == scale_integer(8, packet_direction)
    assert electric_magnetic[3:] == ORIGIN
    checks += 2

    if orientation == 1:
        endpoint_state = {ORIGIN: 1, route: -1}
        divergence = boundary(ORIGIN, route, 1)
    else:
        endpoint_state = {ORIGIN: -1, route: 1}
        divergence = boundary(route, scale_integer(-1, route), 1)
    charge = {site: -value for site, value in endpoint_state.items()}
    assert divergence == charge
    assert sum(endpoint_state.values()) == 0
    assert sum(charge.values()) == 0
    checks += 3

    # One detector token plus one eight-record cotangent packet is retained in
    # reserve, active, or recovery ownership at every detector phase.
    assert 1 + len(records) == 9
    checks += 1
    return checks


def verify_case(outcome_counts, padding: int) -> int:
    checks = 0
    assert len(outcome_counts) <= len(SC_DIRECTIONS)
    original = records_from_counts(outcome_counts)
    residual, dark = canonical_cancel(original)
    capacity = max(1, len(residual) + padding)
    bank = tuple(residual) + (None,) * (capacity - len(residual))
    period = capacity * (capacity + 1)

    states = total_state_space(bank)
    image = tuple(renewal_step(bank, state) for state in states)
    assert len(set(image)) == len(states)
    assert set(image) == set(states)
    checks += 2

    for state in states:
        assert renewal_inverse(bank, renewal_step(bank, state)) == state
        assert renewal_step(bank, renewal_inverse(bank, state)) == state
        checks += 2

    expected = tuple(coherent_norm_squared(counts) for counts in outcome_counts)
    bright_total = sum(expected)
    orbit = complete_orbit(bank)
    operational_states = operational_state_space(bank)
    assert len(orbit) == period + 2 * bright_total
    assert len(orbit) == len(operational_states)
    assert set(orbit) == set(operational_states)
    checks += 3

    observed = Counter()
    for state in orbit:
        if state.detector != MANIFESTED:
            continue
        port = bright_port(bank, state.pointer)
        assert port is not None
        observed[port] += 1
        for orientation in (-1, 1):
            checks += verify_gauss_event(port, state.pointer % 4, orientation)

    assert tuple(observed[outcome] for outcome in range(len(expected))) == expected
    assert sum(observed.values()) == bright_total
    checks += 2
    if bright_total:
        for outcome, count in enumerate(expected):
            assert Fraction(observed[outcome], bright_total) == Fraction(
                count, bright_total
            )
            checks += 1

    # Signal and canceled payload records remain present and distinguishable.
    assert tuple(bank[: len(residual)]) == tuple(residual)
    assert len(residual) + 2 * len(dark) == len(original)
    checks += 2
    return checks


def main() -> None:
    checks = 0
    for raw_counts in product(range(5), repeat=4):
        checks += verify_case((raw_counts,), padding=2)

    multiple_outcome_cases = (
        ((1, 0, 0, 0), (0, 1, 0, 0)),
        ((3, 1, 1, 0), (0, 4, 0, 1), (2, 2, 2, 1)),
        ((8, 0, 3, 0), (1, 7, 0, 2), (4, 4, 4, 4), (0, 0, 0, 2)),
        ((2, 5, 6, 1), (7, 2, 0, 8), (3, 3, 3, 3)),
    )
    for outcome_counts in multiple_outcome_cases:
        checks += verify_case(outcome_counts, padding=3)

    print("one balanced-ternary detector: ready -> manifest -> recover")
    print("renewal map permutes the full pointer x detector product")
    print("one exclusive canonical Gauss event occurs at every bright pair")
    print("event counts M_o=|Z_o|^2 exactly; no prepared detector tape")
    print(
        "PASS: C4 autonomous reversible Born renewal detector "
        f"({checks} exact checks)"
    )
    print(
        "Open: native record-bank preparation, externally heralded trials, "
        "general-amplitude bank generation, and multipartite no-signalling"
    )


if __name__ == "__main__":
    main()
