#!/usr/bin/env python3
"""Exact finite physical Born pushforward onto a reversible detector tape.

Prepared residual C4 records remain in one shared bank.  Coprime address heads
visit every ordered pair.  Each pointer state owns one fresh detector token;
a local bright-pair predicate reversibly moves that token from reserve to a
manifested, outcome-routed bond.  The resulting physical detector tape has
exact |Z_o|^2 event counts while retaining all signal, dark, and detector
payloads, including self-address contributions.

This is conditional on prepared finite integer C4 records and a fresh finite
detector tape.  It does not derive preparation, general amplitudes,
single-trial competition, or multipartite causality.
"""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
from fractions import Fraction
from itertools import product

from proof_c4_controlled_actualization_transaction import (
    ActualizationState,
    Token,
    actualization_macro,
    charge,
    payload,
    rotate_state,
    token_count,
)
from proof_c4_coprime_ring_born_pushforward import (
    coherent_norm_squared,
    ring_orbit,
)
from proof_reversible_c4_cancellation_click_circuit import (
    Record,
    canonical_cancel,
    click_compatible,
    records_from_counts,
)


@dataclass(frozen=True)
class DetectorCell:
    """One complete detector token and its physical outcome-route ownership."""

    port: int | None
    actualization: ActualizationState


def fresh_cell(tick: int, orientation: int = 1) -> DetectorCell:
    token = Token(tick % 4, orientation)
    return DetectorCell(
        None,
        ActualizationState(0, 0, 0, None, token),
    )


def detector_gate(
    left: Record | None,
    right: Record | None,
    cell: DetectorCell,
) -> DetectorCell:
    """Bright pair controls one reversible token ownership transfer."""

    if not click_compatible(left, right):
        return cell
    assert left is not None and right is not None
    outcome = left.outcome

    if cell.port is None and cell.actualization.link is None and cell.actualization.reserve is not None:
        return DetectorCell(
            outcome,
            actualization_macro(cell.actualization, True),
        )

    if (
        cell.port == outcome
        and cell.actualization.link is not None
        and cell.actualization.reserve is None
    ):
        return DetectorCell(
            None,
            actualization_macro(cell.actualization, True),
        )

    return cell


def rotate_record(record: Record | None, turns: int) -> Record | None:
    if record is None:
        return None
    return Record(record.outcome, (record.phase + turns) % 4, record.identity)


def rotate_cell(cell: DetectorCell, turns: int) -> DetectorCell:
    return DetectorCell(cell.port, rotate_state(cell.actualization, turns))


def run_tape(
    bank: tuple[Record | None, ...],
    tape: tuple[DetectorCell, ...],
) -> tuple[DetectorCell, ...]:
    capacity = len(bank)
    orbit = ring_orbit(capacity)
    assert len(tape) == len(orbit)
    output = []
    for (left_address, right_address), cell in zip(orbit, tape):
        left = bank[left_address]
        right = bank[right_address] if right_address < capacity else None
        output.append(detector_gate(left, right, cell))
    return tuple(output)


def tape_counts(tape: tuple[DetectorCell, ...]) -> Counter[int]:
    return Counter(cell.port for cell in tape if cell.port is not None)


def verify_gate_algebra() -> int:
    checks = 0
    records = tuple(
        Record(outcome, phase, identity)
        for identity, (outcome, phase) in enumerate(product(range(2), range(4)))
    )

    for left in records:
        for right in records:
            for detector_phase, orientation in product(range(4), (-1, 1)):
                cell = DetectorCell(
                    None,
                    ActualizationState(
                        0,
                        0,
                        0,
                        None,
                        Token(detector_phase, orientation),
                    ),
                )
                output = detector_gate(left, right, cell)
                assert detector_gate(left, right, output) == cell
                assert token_count(output.actualization) == token_count(cell.actualization) == 1
                assert payload(output.actualization) == payload(cell.actualization)
                assert charge(output.actualization) == charge(cell.actualization) == 0
                assert (output.port is not None) == click_compatible(left, right)
                checks += 5

                for turns in range(4):
                    assert detector_gate(
                        rotate_record(left, turns),
                        rotate_record(right, turns),
                        rotate_cell(cell, turns),
                    ) == rotate_cell(output, turns)
                    checks += 1

    return checks


def verify_case(outcome_counts: tuple[tuple[int, int, int, int], ...], padding: int) -> int:
    checks = 0
    original = records_from_counts(outcome_counts)
    residual, dark = canonical_cancel(original)
    capacity = max(1, len(residual) + padding)
    bank: tuple[Record | None, ...] = tuple(residual) + (None,) * (capacity - len(residual))
    period = capacity * (capacity + 1)
    initial_tape = tuple(fresh_cell(tick) for tick in range(period))

    manifested_tape = run_tape(bank, initial_tape)
    restored_tape = run_tape(bank, manifested_tape)
    assert restored_tape == initial_tape
    assert period % capacity == 0 and period % (capacity + 1) == 0
    checks += 2

    expected = tuple(coherent_norm_squared(counts) for counts in outcome_counts)
    observed = tape_counts(manifested_tape)
    assert tuple(observed[outcome] for outcome in range(len(expected))) == expected
    assert sum(observed.values()) == sum(expected)
    checks += 2

    if sum(expected) > 0:
        for outcome, expected_count in enumerate(expected):
            assert Fraction(observed[outcome], sum(observed.values())) == Fraction(
                expected_count, sum(expected)
            )
            checks += 1

    # Every signal record contributes its diagonal self-address event without
    # being copied or consumed; the separate detector cell is what manifests.
    orbit = ring_orbit(capacity)
    self_clicks = 0
    for tick, (left_address, right_address) in enumerate(orbit):
        if right_address >= capacity or left_address != right_address:
            continue
        record = bank[left_address]
        if record is not None:
            assert manifested_tape[tick].port == record.outcome
            self_clicks += 1
            checks += 1
    assert self_clicks == len(residual)
    checks += 1

    # No record is erased: active residuals and payload-complete dark pairs are
    # unchanged, and every detector cell still owns exactly one token.
    assert tuple(bank[: len(residual)]) == tuple(residual)
    assert len(residual) + 2 * len(dark) == len(original)
    assert all(token_count(cell.actualization) == 1 for cell in manifested_tape)
    assert all(charge(cell.actualization) == 0 for cell in manifested_tape)
    checks += 4

    return checks


def main() -> None:
    checks = verify_gate_algebra()

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

    print(f"PASS: physical C4 Born actualization tape ({checks} exact checks)")
    print("prepared residual bank is unchanged; dark records and detector payloads are retained")
    print("one manifested detector token per compatible ordered pair, including self-address terms")
    print("finite normalized detector frequencies = |Z_o|^2 / sum_r |Z_r|^2")
    print("Open: native preparation, finite-tape formation, general amplitudes, trial competition, no-signalling")


if __name__ == "__main__":
    main()
