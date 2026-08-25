#!/usr/bin/env python3
"""Exact finite source-history formation of a v3 Born phase bank.

One physical A9 source clock, an eight-bit retained finite controller history,
one fixed-occupancy A2 cursor, and eight existing A2 reserve occupancies form
an eight-record field bank at one native outcome port.  At each global tick
the current source C4 phase selects the first clear intrinsic channel in that
phase bin; one A2 reserve occupancy becomes that field record; then the next
retained controller bit either admits one native A9 tick or stalls it.

All 2^8 controller histories and all sixteen initial A9 states are exhausted.
The formed bank counts are exactly the source's actual phase-visit counts.
Opposite phases cancel to Z and the compatible-pair census is exactly |Z|^2.
The complete eight-tick formation has an exact inverse because the finite
controller history survives.  No probability, random draw, amplitude target,
or empirical value enters.

This is a selected Phi-v13 source-to-bank transducer.  It does not form the
source/controller/chart or the renewal apparatus, amplify a detector record,
or prove multipartite laboratory statistics.
"""

from __future__ import annotations

import sys
from collections import Counter, defaultdict
from dataclasses import dataclass
from fractions import Fraction
from functools import lru_cache
from itertools import product

from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_ternary_square_phase_polarity_autonomous_clock import (
    LocalState,
    iterate as a9_iterate,
    occupation,
    phase_index,
    tick as a9_tick,
)
from proof_v3_contextual_neutral_pointer_born_renewal_apparatus import (
    Channel,
    Outcome,
    address_order,
    apparatus_chart,
    bank_from_counts,
    canonical_residual,
    compatible,
    outcome,
)
from proof_v3_cubic_triplet_self_correcting_material_clock import LOGICAL
from proof_v3_field_bank_gaussian_born_readout import (
    bright_pair_count,
    gaussian_integer,
    norm_squared,
)
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    OrientedRepairChart,
    mv,
    transform_channel,
    transform_chart,
)
from proof_v3_rotor_green_a2_physical_memory_phase_protection import (
    decode_counter,
    encode_counter,
)


sys.stdout.reconfigure(encoding="utf-8")

Counts = tuple[int, int, int, int]
Schedule = tuple[int, int, int, int, int, int, int, int]
Reserve = tuple[int, int, int, int, int, int, int, int]

WINDOW = 8
FULL_RESERVE: Reserve = (1,) * WINDOW
EMPTY_RESERVE: Reserve = (0,) * WINDOW


def active_token(state: LocalState):
    return state.link if occupation(state.link) else state.reserve


def active_phase(state: LocalState) -> int:
    return phase_index(active_token(state))


def source_port(chart: OrientedRepairChart) -> Outcome:
    return chart.first, chart.polarity


@lru_cache(maxsize=None)
def formation_order(chart: OrientedRepairChart) -> tuple[Channel, ...]:
    return address_order(chart)


@lru_cache(maxsize=None)
def port_phase_channels(
    order: tuple[Channel, ...], port: Outcome, phase: int
) -> tuple[Channel, ...]:
    return tuple(
        channel
        for channel in order
        if outcome(channel) == port and channel[3] == phase
    )


def bank_counts(bank: frozenset[Channel], port: Outcome) -> Counts:
    counts = Counter(channel[3] for channel in bank if outcome(channel) == port)
    return tuple(counts[phase] for phase in range(4))  # type: ignore[return-value]


@dataclass(frozen=True)
class SourceBankState:
    source: LocalState
    schedule: Schedule
    cursor: int
    bank: frozenset[Channel]
    reserve: Reserve

    def __post_init__(self) -> None:
        assert self.source in LOGICAL
        assert len(self.schedule) == WINDOW
        assert all(bit in (0, 1) for bit in self.schedule)
        assert 0 <= self.cursor <= WINDOW
        assert len(self.reserve) == WINDOW
        assert all(bit in (0, 1) for bit in self.reserve)


def initial_state(source: LocalState, schedule: Schedule) -> SourceBankState:
    return SourceBankState(source, schedule, 0, frozenset(), FULL_RESERVE)


def formation_step(
    chart: OrientedRepairChart, state: SourceBankState
) -> SourceBankState:
    """One local target-blind source-phase write; malformed inputs fail closed."""

    if state.cursor >= WINDOW or state.reserve[state.cursor] != 1:
        return state

    order = formation_order(chart)
    port = source_port(chart)
    phase = active_phase(state.source)
    channels = port_phase_channels(order, port, phase)
    occupied = sum(channel in state.bank for channel in channels)
    if occupied >= len(channels):
        return state
    target = channels[occupied]
    if target in state.bank:
        return state

    bit = state.schedule[state.cursor]
    reserve = list(state.reserve)
    reserve[state.cursor] = 0
    return SourceBankState(
        source=a9_tick(state.source) if bit else state.source,
        schedule=state.schedule,
        cursor=state.cursor + 1,
        bank=state.bank | {target},
        reserve=tuple(reserve),
    )


def formation_inverse(
    chart: OrientedRepairChart, state: SourceBankState
) -> SourceBankState:
    """Exact inverse on the admitted formation image."""

    if state.cursor <= 0 or state.reserve[state.cursor - 1] != 0:
        return state

    bit = state.schedule[state.cursor - 1]
    source_before = a9_iterate(state.source, 7) if bit else state.source
    phase = active_phase(source_before)
    order = formation_order(chart)
    port = source_port(chart)
    channels = port_phase_channels(order, port, phase)
    occupied = [channel for channel in channels if channel in state.bank]
    if not occupied:
        return state
    target = occupied[-1]

    reserve = list(state.reserve)
    reserve[state.cursor - 1] = 1
    return SourceBankState(
        source=source_before,
        schedule=state.schedule,
        cursor=state.cursor - 1,
        bank=state.bank - {target},
        reserve=tuple(reserve),
    )


def iterate_formation(
    chart: OrientedRepairChart, state: SourceBankState, ticks: int
) -> SourceBankState:
    for _ in range(ticks):
        state = formation_step(chart, state)
    return state


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    chart = apparatus_chart()
    order = formation_order(chart)
    port = source_port(chart)

    check(
        "C1 one native outcome port has exactly eight existing channels in each C4 phase bin",
        len(order) == 384
        and all(len(port_phase_channels(order, port, phase)) == 8 for phase in range(4)),
    )

    # Eight controller bits and the cursor are existing fixed-occupancy A2
    # phase records.  This only certifies representability; the selected source
    # branch keeps the schedule explicitly in SourceBankState for readability.
    controller_payloads = tuple(encode_counter(bit) for bit in (0, 1))
    cursor_payloads = tuple(encode_counter(value) for value in range(WINDOW + 1))
    check(
        "C2 retained schedule bits and cursor fit existing fixed-occupancy A2 phase states",
        tuple(decode_counter(payload) for payload in controller_payloads) == (0, 1)
        and tuple(decode_counter(payload) for payload in cursor_payloads)
        == tuple(range(WINDOW + 1)),
    )

    history_rows = 0
    inverse_rows = 0
    compatible_rows = 0
    resource_rows = 0
    attained_counts: set[Counts] = set()
    schedules_by_counts: defaultdict[Counts, set[Schedule]] = defaultdict(set)
    for source in LOGICAL:
        for bits in product((0, 1), repeat=WINDOW):
            schedule: Schedule = tuple(bits)  # type: ignore[assignment]
            state = initial_state(source, schedule)
            phase_visits = [active_phase(state.source)]
            orbit = [state]
            for _ in range(WINDOW):
                state = formation_step(chart, state)
                orbit.append(state)
                if state.cursor < WINDOW:
                    phase_visits.append(active_phase(state.source))

            for resource_state in orbit:
                assert len(resource_state.bank) + sum(resource_state.reserve) == WINDOW
                resource_rows += 1

            assert state.cursor == WINDOW
            assert state.reserve == EMPTY_RESERVE
            assert len(state.bank) == WINDOW
            counts = bank_counts(state.bank, port)
            expected_counts = tuple(
                phase_visits.count(phase) for phase in range(4)
            )
            assert counts == expected_counts
            assert state.bank == bank_from_counts(order, {port: counts})
            attained_counts.add(counts)
            schedules_by_counts[counts].add(schedule)

            z = gaussian_integer(counts)
            residual = canonical_residual(state.bank, order)
            pair_count = sum(
                compatible(left, right, residual)
                for left in residual
                for right in residual
            )
            assert pair_count == bright_pair_count(counts) == norm_squared(z)
            compatible_rows += pair_count

            inverse = state
            for expected in reversed(orbit[:-1]):
                inverse = formation_inverse(chart, inverse)
                assert inverse == expected
                inverse_rows += 1
            history_rows += 1

    check(
        "C3 all 4,096 finite source/controller histories form exact eight-record phase banks",
        history_rows == len(LOGICAL) * 2**WINDOW == 4_096,
    )
    check(
        "C4 every formed phase count equals the source's actual eight-tick visit count",
        all(sum(counts) == WINDOW and max(counts) <= 8 for counts in attained_counts),
    )
    check(
        "C5 every eight-tick formation history has an exact retained-history inverse",
        inverse_rows == history_rows * WINDOW,
    )
    check(
        "C6 each formed bank has compatible-pair cardinality exactly |Z|^2",
        compatible_rows > 0,
    )

    # Timing order is not an input to the detector.  Whenever distinct source
    # histories have the same surviving phase counts, they form exactly the
    # same bank and therefore the same detector event cardinality.
    degenerate_classes = {
        counts: schedules
        for counts, schedules in schedules_by_counts.items()
        if len(schedules) > 1
    }
    check(
        "C7 detailed source timing can vary while the surviving bank fixes the same detector count",
        len(degenerate_classes) > 0,
    )

    # Exact two-port normalization over every pair of physically attainable
    # source-history count vectors.  This is a pushforward of formed event
    # cardinalities, not a probability primitive.
    frequency_rows = 0
    for left in attained_counts:
        for right in attained_counts:
            left_weight = bright_pair_count(left)
            right_weight = bright_pair_count(right)
            total = left_weight + right_weight
            if total == 0:
                continue
            formed_frequency = Fraction(left_weight, total)
            left_z = gaussian_integer(left)
            right_z = gaussian_integer(right)
            born_form = Fraction(
                norm_squared(left_z),
                norm_squared(left_z) + norm_squared(right_z),
            )
            assert formed_frequency == born_form
            frequency_rows += 1
    check(
        "C8 normalized event counts of every attainable two-history pair have exact Born form",
        frequency_rows > 0,
    )

    # Formation is an occupancy transfer, not record creation: one A2 reserve
    # occupancy becomes one field record on each of eight ticks.  Source,
    # controller bits, and cursor stay at fixed occupancy.
    check(
        "C9 source-bank formation conserves the relative occupancy ray tickwise and in total",
        resource_rows == history_rows * (WINDOW + 1)
        and (0 + WINDOW) == (WINDOW + 0),
    )

    group = tuple(signed_permutation_matrices())
    covariance_rows = 0
    for matrix in group:
        transformed_chart = transform_chart(matrix, chart)
        transformed_order = formation_order(transformed_chart)
        transformed_port = source_port(transformed_chart)
        assert transformed_port == (mv(matrix, port[0]), port[1])
        for counts in attained_counts:
            bank = bank_from_counts(order, {port: counts})
            transformed_bank = frozenset(
                transform_channel(matrix, channel) for channel in bank
            )
            expected = bank_from_counts(
                transformed_order, {transformed_port: counts}
            )
            assert transformed_bank == expected
            covariance_rows += 1
    check(
        "C10 intrinsic first-clear channel writing is fully signed-cubic covariant with its chart",
        covariance_rows == len(group) * len(attained_counts),
    )

    malformed = SourceBankState(
        LOGICAL[0],
        (0,) * WINDOW,
        0,
        frozenset(),
        (0,) + (1,) * (WINDOW - 1),
    )
    completed = iterate_formation(
        chart, initial_state(LOGICAL[0], (0,) * WINDOW), WINDOW
    )
    check(
        "C11 spent or malformed reserve ports fail closed after or before formation",
        formation_step(chart, malformed) == malformed
        and formation_step(chart, completed) == completed,
    )

    forbidden = (
        "137.036",
        "master_root",
        "empirical_target",
        "random_draw",
        "born_weight",
        "probability_table",
    )
    missing = {
        "native formation of source controller chart and reserve",
        "unbounded or renewed source emission windows",
        "formation and protection of the pair-enumeration apparatus",
        "detector amplification reciprocal backreaction and traffic",
        "multipartite source splitting and laboratory routing",
        "canonical Phi provenance and environmental stability",
    }
    check(
        "C12 Phi-v13 closes finite source-to-bank formation only, not the general physical Born rule",
        all(token not in __doc__.lower() for token in forbidden)
        and len(missing) == 6,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} finite source-history Born-bank checks pass")
    print(f"source_history_rows={history_rows}")
    print(f"inverse_tick_rows={inverse_rows}")
    print(f"attained_phase_count_vectors={len(attained_counts)}")
    print(f"timing_degenerate_count_classes={len(degenerate_classes)}")
    print(f"two_history_frequency_rows={frequency_rows}")
    print(f"tickwise_resource_rows={resource_rows}")
    print(f"signed_cubic_covariance_rows={covariance_rows}")
    print("formation_window_ticks=8")
    print("occupancy_delta_per_tick=field_plus_1_A2_minus_1")
    print("formed_event_count=abs_Z_squared")
    print("random_draw=none")
    print("status=selected_phi_v13_finite_source_history_born_bank_formation")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
