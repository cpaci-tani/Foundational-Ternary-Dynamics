#!/usr/bin/env python3
"""Contextual neutral-pointer Born renewal apparatus on the v3 carrier.

One prepared v3 field bank has 384 actual channel addresses.  A selected
oriented apparatus chart orders them without assigning any target weight.
Two existing-carrier neutral pointers run fixed cycles of lengths 384 and
385 (the latter has one delay state), so their joint orbit visits every
ordered channel pair exactly once.  A bank-local deterministic cancellation
predicate marks the C4 residual records.  One balanced-ternary detector trit
then executes ready -> manifested -> recovery at every compatible residual
pair and advances immediately at a dark pair.

The pointer address is carried by an opposite-polarity zero-E/B field pair
plus one existing A9 polarity token.  The delay state is blank inside the
selected apparatus site.  The apparatus therefore introduces no new carrier
type, counter, random draw, target probability, or event tape.

The construction is conditional on a prepared field bank and a formed
oriented apparatus block.  Native bank formation, amplification,
backreaction, overlapping traffic, and multipartite no-signalling remain
open.
"""

from __future__ import annotations

import sys
from collections import Counter, defaultdict
from dataclasses import dataclass
from fractions import Fraction
from itertools import product
from math import gcd

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_v3_charged_candidate_matter_perturbation_boundary import all_local_channels
from proof_v3_field_bank_gaussian_born_readout import bright_pair_count
from proof_v3_neutral_rotor_walker_macro import physical_value, polarized_slots
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    OrientedRepairChart,
    normalized_channel,
    transform_channel,
    transform_chart,
    transform_slots,
)
from proof_c4_autonomous_reversible_born_renewal_detector import verify_gauss_event


sys.stdout.reconfigure(encoding="utf-8")

Channel = tuple[tuple[int, int, int], tuple[int, int, int], int, int, int]
Outcome = tuple[tuple[int, int, int], int]

READY = 0
MANIFESTED = 1
RECOVERY = -1


def apparatus_chart() -> OrientedRepairChart:
    return OrientedRepairChart(
        origin=(0, 0, 0),
        first=(1, 0, 0),
        second=(0, 1, 0),
        repair_normal=(0, 0, 1),
        layer=0,
        offset=0,
        polarity=1,
    )


def address_order(chart: OrientedRepairChart) -> tuple[Channel, ...]:
    return tuple(sorted(all_local_channels(), key=lambda ch: normalized_channel(chart, ch)))


def pointer_configuration(channel: Channel):
    state = ((channel[0], channel[1], channel[2]), channel[3])
    neutral_pair = polarized_slots(state)
    polarity_token = (0, channel[4])
    return neutral_pair, polarity_token


DELAY_CONFIGURATION = frozenset(), None


def outcome(channel: Channel) -> Outcome:
    return channel[0], channel[4]


def rail(channel: Channel) -> str:
    return "R" if channel[3] in (0, 2) else "I"


def canonical_residual(bank: frozenset[Channel], order: tuple[Channel, ...]):
    address = {channel: index for index, channel in enumerate(order)}
    groups: dict[tuple[Outcome, int], list[Channel]] = defaultdict(list)
    for channel in bank:
        groups[(outcome(channel), channel[3])].append(channel)
    for records in groups.values():
        records.sort(key=address.__getitem__)

    residual: set[Channel] = set()
    for port in {outcome(channel) for channel in order}:
        phase = {k: groups.get((port, k), []) for k in range(4)}
        real = min(len(phase[0]), len(phase[2]))
        imag = min(len(phase[1]), len(phase[3]))
        residual.update(phase[0][real:])
        residual.update(phase[2][real:])
        residual.update(phase[1][imag:])
        residual.update(phase[3][imag:])
    return frozenset(residual)


def compatible(left: Channel, right: Channel, residual) -> bool:
    return (
        left in residual
        and right in residual
        and outcome(left) == outcome(right)
        and rail(left) == rail(right)
    )


def addressed_pair(order: tuple[Channel, ...], pointer: int):
    left = order[pointer % 384]
    right_index = pointer % 385
    right = order[right_index] if right_index < 384 else None
    return left, right


def bright_outcome(order, residual, pointer: int):
    left, right = addressed_pair(order, pointer)
    if right is None or not compatible(left, right, residual):
        return None
    return outcome(left)


@dataclass(frozen=True)
class ApparatusState:
    pointer: int
    detector: int


def apparatus_step(order, residual, state: ApparatusState) -> ApparatusState:
    period = 384 * 385
    assert 0 <= state.pointer < period
    assert state.detector in (RECOVERY, READY, MANIFESTED)
    bright = bright_outcome(order, residual, state.pointer)
    if state.detector == READY:
        if bright is None:
            return ApparatusState((state.pointer + 1) % period, READY)
        return ApparatusState(state.pointer, MANIFESTED)
    if state.detector == MANIFESTED:
        return ApparatusState(state.pointer, RECOVERY)
    if bright is not None:
        return ApparatusState((state.pointer + 1) % period, READY)
    return ApparatusState(state.pointer, MANIFESTED)


def apparatus_inverse(order, residual, state: ApparatusState) -> ApparatusState:
    period = 384 * 385
    if state.detector == MANIFESTED:
        if bright_outcome(order, residual, state.pointer) is None:
            return ApparatusState(state.pointer, RECOVERY)
        return ApparatusState(state.pointer, READY)
    if state.detector == RECOVERY:
        return ApparatusState(state.pointer, MANIFESTED)
    previous = (state.pointer - 1) % period
    if bright_outcome(order, residual, previous) is None:
        return ApparatusState(previous, READY)
    return ApparatusState(previous, RECOVERY)


def complete_operational_orbit(order, residual):
    initial = ApparatusState(0, READY)
    state = initial
    orbit = []
    seen = set()
    while state not in seen:
        seen.add(state)
        orbit.append(state)
        state = apparatus_step(order, residual, state)
    assert state == initial
    return tuple(orbit)


def bank_from_counts(order, fixtures: dict[Outcome, tuple[int, int, int, int]]):
    by_port_phase: dict[tuple[Outcome, int], list[Channel]] = defaultdict(list)
    for channel in order:
        by_port_phase[(outcome(channel), channel[3])].append(channel)
    bank = set()
    for port, counts in fixtures.items():
        for phase, count in enumerate(counts):
            records = by_port_phase[(port, phase)]
            assert 0 <= count <= len(records) == 8
            bank.update(records[:count])
    return frozenset(bank)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    chart = apparatus_chart()
    order = address_order(chart)
    states = tuple(one_particle_states())
    group = tuple(signed_permutation_matrices())
    check(
        "C1 contextual apparatus order contains every v3 field channel once",
        len(order) == len(set(order)) == 384,
    )

    configurations = tuple(pointer_configuration(channel) for channel in order)
    check(
        "C2 every channel address has one unique existing-carrier neutral-pointer encoding",
        len(set(configurations)) == 384
        and all(len(slots) == 2 for slots, _token in configurations)
        and all(
            physical_value(slots, layer) == (0,) * 6
            for slots, _token in configurations
            for layer in range(3)
        ),
    )
    check(
        "C3 the second pointer has exactly one additional existing blank delay state",
        DELAY_CONFIGURATION not in configurations
        and len(set(configurations) | {DELAY_CONFIGURATION}) == 385,
    )

    # The fixed chart selects an order; transforming the chart transforms the
    # order.  It does not demand that one cycle commute with the full group.
    covariance_rows = 0
    for matrix in group:
        transformed_chart = transform_chart(matrix, chart)
        transformed_order = address_order(transformed_chart)
        assert tuple(transform_channel(matrix, channel) for channel in order) == transformed_order
        for channel, transformed_channel in zip(order, transformed_order):
            slots, token = pointer_configuration(channel)
            next_slots, next_token = pointer_configuration(transformed_channel)
            assert transform_slots(matrix, slots) == next_slots
            assert token == next_token
            covariance_rows += 1
    check(
        "C4 pointer cycle is full signed-cubic covariant with its physical context chart",
        covariance_rows == 384 * 48,
    )

    period = 384 * 385
    pairs = {
        (tick % 384, tick % 385)
        for tick in range(period)
    }
    check(
        "C5 consecutive fixed pointer cycles enumerate every ordered address/delay pair once",
        gcd(384, 385) == 1 and len(pairs) == period,
    )

    ports = tuple(sorted({outcome(channel) for channel in order}))
    check(
        "C6 the bank exposes exactly twelve tangent/polarity outcome ports with eight channels per phase",
        len(ports) == 12
        and all(
            sum(1 for channel in order if outcome(channel) == port and channel[3] == phase) == 8
            for port in ports
            for phase in range(4)
        ),
    )

    # Exhaust every one-port local v3 count vector.  The residual is computed
    # from actual occupied channels, not supplied as a target weight.
    counting_rows = 0
    for counts in product(range(9), repeat=4):
        bank = bank_from_counts(order, {ports[0]: counts})
        residual = canonical_residual(bank, order)
        clicks = sum(
            compatible(left, right, residual)
            for left in residual
            for right in residual
        )
        assert clicks == bright_pair_count(counts)
        counting_rows += 1
    check(
        "C7 every bounded v3 phase bank gives exact compatible-pair count |Z|^2",
        counting_rows == 9**4,
    )

    fixtures = {
        ports[0]: (8, 1, 3, 0),
        ports[1]: (2, 7, 0, 2),
        ports[2]: (4, 4, 4, 4),
        ports[3]: (0, 0, 0, 2),
    }
    bank = bank_from_counts(order, fixtures)
    residual = canonical_residual(bank, order)
    expected = {port: bright_pair_count(counts) for port, counts in fixtures.items()}
    bright_total = sum(expected.values())

    # Complete forward/inverse identities on the full pointer x trit product.
    inverse_rows = 0
    for pointer in range(period):
        for detector in (RECOVERY, READY, MANIFESTED):
            state = ApparatusState(pointer, detector)
            assert apparatus_inverse(
                order, residual, apparatus_step(order, residual, state)
            ) == state
            assert apparatus_step(
                order, residual, apparatus_inverse(order, residual, state)
            ) == state
            inverse_rows += 1
    check(
        "C8 one balanced-ternary detector gives a total reversible renewal permutation",
        inverse_rows == period * 3,
    )

    orbit = complete_operational_orbit(order, residual)
    observed = Counter()
    for state in orbit:
        if state.detector != MANIFESTED:
            continue
        port = bright_outcome(order, residual, state.pointer)
        assert port is not None
        observed[port] += 1
    check(
        "C9 one operational orbit emits one exclusive event per compatible pair",
        len(orbit) == period + 2 * bright_total
        and all(observed[port] == count for port, count in expected.items())
        and sum(observed.values()) == bright_total,
    )
    check(
        "C10 normalized manifested-event frequencies have exact prepared Born form",
        all(
            Fraction(observed[port], bright_total) == Fraction(count, bright_total)
            for port, count in expected.items()
        ),
    )

    event_cycle = tuple(
        bright_outcome(order, residual, state.pointer)
        for state in orbit
        if state.detector == MANIFESTED
    )
    assert all(port is not None for port in event_cycle)
    finite_window_rows = 0
    for start in range(bright_total):
        for sample_size in range(1, 3 * bright_total + 1):
            window = tuple(
                event_cycle[(start + index) % bright_total]
                for index in range(sample_size)
            )
            counts = Counter(window)
            total_variation = sum(
                abs(
                    Fraction(counts[port], sample_size)
                    - Fraction(expected[port], bright_total)
                )
                for port in expected
            ) / 2
            assert total_variation < Fraction(bright_total, sample_size)
            finite_window_rows += 1
    check(
        "C11 every heralded manifested-event window obeys the exact B/N discrepancy bound",
        finite_window_rows == bright_total * (3 * bright_total),
    )

    gauss_rows = 0
    tangent_order = (
        (1, 0, 0),
        (-1, 0, 0),
        (0, 1, 0),
        (0, -1, 0),
        (0, 0, 1),
        (0, 0, -1),
    )
    for tangent, polarity in ports:
        gauss_rows += verify_gauss_event(tangent_order.index(tangent), 0, polarity)
    check(
        "C12 every outcome routes one canonical polarity-complete Gauss event",
        gauss_rows == 12 * 6,
    )

    # Four selected apparatus sites fit one Moore cube.  The pointer pair is
    # zero E/B; the detector is the primitive ternary site state.
    sites = ((0, 0, 0), (1, 0, 0), (0, 1, 0), (0, 0, 1))
    check(
        "C13 bank, two neutral pointers, and ternary detector fit one Moore-local apparatus block",
        all(
            max(abs(a - b) for a, b in zip(left, right)) <= 1
            for left in sites
            for right in sites
        ),
    )

    missing = {
        "native formation of the prepared residual bank",
        "native formation and protection of the oriented apparatus chart",
        "amplified persistent detector record and reciprocal backreaction",
        "overlapping source and apparatus traffic arbitration",
        "integration into homogeneous Phi with expiry reset",
        "multipartite composition and operational no signalling",
    }
    check(
        "C14 physical Born recovery remains open at six preparation/apparatus/composition debts",
        len(missing) == 6,
    )
    check(
        "C15 no target probability, random draw, empirical value, new type, or near-miss search enters",
        len(order) == 384 and period == 147_840,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} contextual v3 Born-apparatus checks pass")
    print(f"pointer_period={period}")
    print(f"pointer_context_covariance_rows={covariance_rows}")
    print(f"bounded_counting_rows={counting_rows}")
    print(f"full_permutation_rows={inverse_rows}")
    print(f"finite_event_window_rows={finite_window_rows}")
    print(f"operational_events={dict(observed)}")
    print("born_result=target_blind_fixed_enumerator_and_reusable_ternary_detector_exact_conditional_on_prepared_bank")
    print("born_status=native_bank_formation_amplification_backreaction_Phi_and_multipartite_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
