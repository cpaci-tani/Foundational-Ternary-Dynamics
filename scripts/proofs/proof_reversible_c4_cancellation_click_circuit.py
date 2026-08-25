#!/usr/bin/env python3
"""Exact finite-state checks for reversible C4 cancellation and click gates.

Opposite phases are moved, not erased, into an invertible dark-pair record.
The residual record bank is then read by the coprime address orbit, and a
non-destructive reversible comparator toggles a local click bit exactly for
same-outcome/same-phase pairs.  After canonical cancellation, this count is
the C4 coherent norm squared.

The certificate does not derive the gates from the full FTD action, supply
their physical work reservoir, or establish the general-amplitude limit.
"""

from __future__ import annotations

from collections import Counter, defaultdict
from dataclasses import dataclass
from fractions import Fraction
from itertools import product

from proof_c4_coprime_ring_born_pushforward import ring_orbit


@dataclass(frozen=True, order=True)
class Record:
    outcome: int
    phase: int
    identity: int

    def __post_init__(self) -> None:
        if self.phase not in range(4):
            raise ValueError("phase must be a C4 exponent")


@dataclass(frozen=True)
class BoundPair:
    channel: str
    left: Record
    right: Record


def rail(phase: int) -> str:
    return "R" if phase % 2 == 0 else "I"


def phase_relation(left: Record, right: Record) -> str | None:
    if left.outcome != right.outcome or rail(left.phase) != rail(right.phase):
        return None
    delta = (right.phase - left.phase) % 4
    if delta == 0:
        return "bright"
    if delta == 2:
        return "dark"
    raise AssertionError("same rail permits only equal or opposite C4 phases")


def reversible_fusion_gate(
    left: Record | None,
    right: Record | None,
    bound: BoundPair | None,
) -> tuple[Record | None, Record | None, BoundPair | None]:
    """Exchange an active related pair with a payload-complete bound record."""

    if left is not None and right is not None and bound is None:
        channel = phase_relation(left, right)
        if channel is not None:
            return (None, None, BoundPair(channel, left, right))
    if left is None and right is None and bound is not None:
        assert phase_relation(bound.left, bound.right) == bound.channel
        return (bound.left, bound.right, None)
    return (left, right, bound)


def click_compatible(left: Record | None, right: Record | None) -> bool:
    return (
        left is not None
        and right is not None
        and phase_relation(left, right) == "bright"
    )


def reversible_click_gate(
    left: Record | None,
    right: Record | None,
    click_bit: int,
) -> tuple[Record | None, Record | None, int]:
    """A nondestructive controlled-NOT readout; self-address pairs are valid."""

    assert click_bit in (0, 1)
    return (left, right, click_bit ^ int(click_compatible(left, right)))


def inventory(
    left: Record | None,
    right: Record | None,
    bound: BoundPair | None,
) -> Counter[Record]:
    records = [record for record in (left, right) if record is not None]
    if bound is not None:
        records.extend((bound.left, bound.right))
    return Counter(records)


def amplitude_components(records: list[Record]) -> dict[int, tuple[int, int]]:
    counts: dict[int, list[int]] = defaultdict(lambda: [0, 0])
    for record in records:
        if record.phase == 0:
            counts[record.outcome][0] += 1
        elif record.phase == 1:
            counts[record.outcome][1] += 1
        elif record.phase == 2:
            counts[record.outcome][0] -= 1
        else:
            counts[record.outcome][1] -= 1
    return {outcome: (value[0], value[1]) for outcome, value in counts.items()}


def canonical_cancel(records: list[Record]) -> tuple[list[Record], list[BoundPair]]:
    """Deterministic queue pairing; a local router implementation is still open."""

    buckets: dict[tuple[int, int], list[Record]] = defaultdict(list)
    for record in sorted(records):
        buckets[(record.outcome, record.phase)].append(record)

    residual: list[Record] = []
    dark: list[BoundPair] = []
    outcomes = sorted({record.outcome for record in records})
    for outcome in outcomes:
        for positive_phase, negative_phase in ((0, 2), (1, 3)):
            positive = buckets[(outcome, positive_phase)]
            negative = buckets[(outcome, negative_phase)]
            pair_count = min(len(positive), len(negative))
            for index in range(pair_count):
                bound = reversible_fusion_gate(
                    positive[index], negative[index], None
                )[2]
                assert bound is not None and bound.channel == "dark"
                dark.append(bound)
            residual.extend(positive[pair_count:])
            residual.extend(negative[pair_count:])

    return (sorted(residual), dark)


def records_from_counts(outcome_counts: tuple[tuple[int, int, int, int], ...]) -> list[Record]:
    records: list[Record] = []
    identity = 0
    for outcome, phase_counts in enumerate(outcome_counts):
        for phase, count in enumerate(phase_counts):
            for _ in range(count):
                records.append(Record(outcome, phase, identity))
                identity += 1
    return records


def click_counts(records: list[Record], capacity: int) -> tuple[Counter[int], int]:
    assert len(records) <= capacity
    bank: list[Record | None] = records + [None] * (capacity - len(records))
    clicks: Counter[int] = Counter()
    for left_address, right_address in ring_orbit(capacity):
        left = bank[left_address]
        right = bank[right_address] if right_address < capacity else None
        if click_compatible(left, right):
            assert left is not None
            clicks[left.outcome] += 1
    return clicks, capacity * (capacity + 1)


def verify_gate_algebra() -> int:
    checks = 0
    records = tuple(
        Record(outcome, phase, identity)
        for identity, (outcome, phase) in enumerate(product(range(2), range(4)))
    )

    for left in records:
        for right in records:
            relation = phase_relation(left, right)
            state = (left, right, None)
            transformed = reversible_fusion_gate(*state)
            assert reversible_fusion_gate(*transformed) == state
            assert inventory(*transformed) == inventory(*state)
            assert (transformed[2] is not None) == (relation is not None)
            checks += 3

            if relation is not None:
                bound = transformed[2]
                assert bound is not None and bound.channel == relation
                assert 2 == len((bound.left, bound.right))
                checks += 2

            for bit in (0, 1):
                clicked = reversible_click_gate(left, right, bit)
                assert reversible_click_gate(*clicked) == (left, right, bit)
                assert clicked[:2] == (left, right)
                assert clicked[2] == (bit ^ int(relation == "bright"))
                checks += 3

            # A common C4 rotation preserves the relative-phase channel.
            rotated_left = Record(left.outcome, (left.phase + 1) % 4, left.identity)
            rotated_right = Record(right.outcome, (right.phase + 1) % 4, right.identity)
            assert phase_relation(rotated_left, rotated_right) == relation
            checks += 1

    return checks


def verify_cancellation_and_clicks() -> int:
    checks = 0
    for raw_counts in product(range(5), repeat=4):
        outcome_counts = (raw_counts,)
        original = records_from_counts(outcome_counts)
        residual, dark = canonical_cancel(original)

        reconstructed = Counter(residual)
        for pair in dark:
            reconstructed.update((pair.left, pair.right))
            assert pair.channel == "dark"
            checks += 1
        assert reconstructed == Counter(original)
        assert len(residual) + 2 * len(dark) == len(original)
        residual_amplitudes = amplitude_components(residual)
        original_amplitudes = amplitude_components(original)
        for outcome in range(len(outcome_counts)):
            assert residual_amplitudes.get(outcome, (0, 0)) == original_amplitudes.get(
                outcome, (0, 0)
            )
            checks += 1
        checks += 2

        for outcome in range(len(outcome_counts)):
            for rail_name, phases in (("R", (0, 2)), ("I", (1, 3))):
                surviving_phases = {
                    record.phase
                    for record in residual
                    if record.outcome == outcome and rail(record.phase) == rail_name
                }
                assert len(surviving_phases) <= 1
                assert surviving_phases.issubset(phases)
                checks += 2

        capacity = max(1, len(residual) + 2)
        clicks, period = click_counts(residual, capacity)
        real, imag = original_amplitudes.get(0, (0, 0))
        expected = real * real + imag * imag
        assert clicks[0] == expected
        assert Fraction(clicks[0], period) == Fraction(expected, period)
        checks += 2

    multi_cases = (
        ((3, 1, 1, 0), (0, 4, 0, 1), (2, 2, 2, 1)),
        ((8, 0, 3, 0), (1, 7, 0, 2), (4, 4, 4, 4), (0, 0, 0, 2)),
    )
    for outcome_counts in multi_cases:
        original = records_from_counts(outcome_counts)
        residual, dark = canonical_cancel(original)
        capacity = max(1, len(residual) + 3)
        clicks, _period = click_counts(residual, capacity)
        amplitudes = amplitude_components(original)
        expected = tuple(
            sum(component * component for component in amplitudes.get(outcome, (0, 0)))
            for outcome in range(len(outcome_counts))
        )
        assert tuple(clicks[outcome] for outcome in range(len(expected))) == expected
        assert len(residual) + 2 * len(dark) == len(original)
        checks += 2
        if sum(expected) > 0:
            for outcome, value in enumerate(expected):
                assert Fraction(clicks[outcome], sum(clicks.values())) == Fraction(
                    value, sum(expected)
                )
                checks += 1

    return checks


def main() -> None:
    checks = verify_gate_algebra() + verify_cancellation_and_clicks()
    print(f"PASS: reversible C4 cancellation/click circuit ({checks} exact checks)")
    print("Finite gate candidate only: common-action work and general Born recovery remain open")


if __name__ == "__main__":
    main()
