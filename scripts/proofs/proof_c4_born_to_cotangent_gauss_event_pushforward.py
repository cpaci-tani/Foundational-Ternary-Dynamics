#!/usr/bin/env python3
"""Exact prepared C4 Born pushforward to physical cotangent Gauss events.

The existing prepared finite Born tape moves one detector C4 token whenever a
phase-compatible ordered history pair is visited.  This certificate attaches
one eight-record cotangent source reserve to every detector cell and transfers
that packet in the same reversible gate.

Each click now consists of:
  * opposite manifested ternary endpoint states,
  * one canonically normalized electric edge packet,
  * exact local Delta div(E)=Delta rho incidence, and
  * retained signal, dark, detector, and source payload/capacity.

The physical Gauss-event counts remain exactly |Z_o|^2.  The result is
conditional on prepared finite integer C4 records, a fresh nine-token detector
cell, and a port-to-SC-route registry.  It does not derive general amplitudes,
single-trial competition, multipartite no-signalling, or biological memory.
"""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
from fractions import Fraction
from itertools import product

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_c4_coprime_ring_born_pushforward import coherent_norm_squared, ring_orbit
from proof_c4_physical_born_actualization_tape import (
    DetectorCell,
    detector_gate,
    fresh_cell,
    rotate_cell,
)
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


@dataclass(frozen=True)
class GaussDetectorCell:
    detector: DetectorCell
    packet_direction: tuple[int, int, int] | None
    packet_active: bool
    packet_reserve: bool


def fresh_gauss_cell(tick: int, orientation: int = 1) -> GaussDetectorCell:
    return GaussDetectorCell(fresh_cell(tick, orientation), None, False, True)


def physical_token_count(cell: GaussDetectorCell) -> int:
    detector_count = int(cell.detector.actualization.link is not None) + int(
        cell.detector.actualization.reserve is not None
    )
    packet_count = 8 * (int(cell.packet_active) + int(cell.packet_reserve))
    return detector_count + packet_count


def gauss_detector_gate(
    left: Record | None,
    right: Record | None,
    cell: GaussDetectorCell,
) -> GaussDetectorCell:
    detector_output = detector_gate(left, right, cell.detector)
    if detector_output == cell.detector:
        return cell

    if (
        cell.detector.port is None
        and detector_output.port is not None
        and not cell.packet_active
        and cell.packet_reserve
    ):
        outcome = detector_output.port
        assert outcome < len(SC_DIRECTIONS)
        orientation = detector_output.actualization.state_left
        direction = scale_integer(orientation, SC_DIRECTIONS[outcome])
        return GaussDetectorCell(detector_output, direction, True, False)

    if (
        cell.detector.port is not None
        and detector_output.port is None
        and cell.packet_active
        and not cell.packet_reserve
    ):
        return GaussDetectorCell(detector_output, None, False, True)

    return cell


def rotate_record(record: Record | None, turns: int) -> Record | None:
    if record is None:
        return None
    return Record(record.outcome, (record.phase + turns) % 4, record.identity)


def rotate_gauss_cell(cell: GaussDetectorCell, turns: int) -> GaussDetectorCell:
    return GaussDetectorCell(
        rotate_cell(cell.detector, turns),
        cell.packet_direction,
        cell.packet_active,
        cell.packet_reserve,
    )


def run_tape(bank, tape):
    orbit = ring_orbit(len(bank))
    assert len(tape) == len(orbit)
    output = []
    for (left_address, right_address), cell in zip(orbit, tape):
        left = bank[left_address]
        right = bank[right_address] if right_address < len(bank) else None
        output.append(gauss_detector_gate(left, right, cell))
    return tuple(output)


def tape_counts(tape) -> Counter[int]:
    return Counter(
        cell.detector.port
        for cell in tape
        if cell.detector.port is not None
    )


def verify_gate_algebra() -> int:
    checks = 0
    records = tuple(
        Record(outcome, phase, identity)
        for identity, (outcome, phase) in enumerate(product(range(4), range(4)))
    )
    for left in records:
        for right in records:
            for detector_phase, orientation in product(range(4), (-1, 1)):
                cell = fresh_gauss_cell(detector_phase, orientation)
                output = gauss_detector_gate(left, right, cell)
                assert gauss_detector_gate(left, right, output) == cell
                assert physical_token_count(output) == physical_token_count(cell) == 9
                assert output.packet_active == click_compatible(left, right)
                assert output.packet_reserve != output.packet_active
                checks += 4

                if output.packet_active:
                    assert output.packet_direction is not None
                    phase = output.detector.actualization.link.phase
                    records_packet = packet(output.packet_direction, phase)
                    value = packet_field(records_packet, 0)
                    assert value[:3] == scale_integer(8, output.packet_direction)
                    assert value[3:] == (0, 0, 0)

                    # Select electric charge as minus the detector ternary
                    # state.  This conventional sign makes the oriented packet
                    # boundary agree with the manifested endpoint pair.
                    left_charge = -output.detector.actualization.state_left
                    right_charge = -output.detector.actualization.state_right
                    route = SC_DIRECTIONS[output.detector.port]
                    if output.detector.actualization.state_left == 1:
                        delta_charge = {(0, 0, 0): left_charge, route: right_charge}
                        delta_divergence = boundary((0, 0, 0), route, 1)
                    else:
                        delta_charge = {(0, 0, 0): left_charge, route: right_charge}
                        delta_divergence = boundary(route, scale_integer(-1, route), 1)
                    assert delta_divergence == delta_charge
                    checks += 3

                for turns in range(4):
                    assert gauss_detector_gate(
                        rotate_record(left, turns),
                        rotate_record(right, turns),
                        rotate_gauss_cell(cell, turns),
                    ) == rotate_gauss_cell(output, turns)
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
    initial_tape = tuple(fresh_gauss_cell(tick) for tick in range(period))

    manifested = run_tape(bank, initial_tape)
    restored = run_tape(bank, manifested)
    assert restored == initial_tape
    checks += 1

    expected = tuple(coherent_norm_squared(counts) for counts in outcome_counts)
    observed = tape_counts(manifested)
    assert tuple(observed[outcome] for outcome in range(len(expected))) == expected
    assert sum(observed.values()) == sum(expected)
    checks += 2
    if sum(expected) > 0:
        for outcome, count in enumerate(expected):
            assert Fraction(observed[outcome], sum(observed.values())) == Fraction(
                count, sum(expected)
            )
            checks += 1

    assert all(physical_token_count(cell) == 9 for cell in manifested)
    assert all(
        cell.packet_active == (cell.detector.port is not None) for cell in manifested
    )
    assert tuple(bank[: len(residual)]) == tuple(residual)
    assert len(residual) + 2 * len(dark) == len(original)
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

    print("prepared detector cell resources=1 C4 token + 8 cotangent tokens")
    print("each bright pair manifests endpoints and one canonical Gauss edge event")
    print("all signal, dark, detector, and source resources are retained")
    print("physical Gauss-event counts M_o=|Z_o|^2 exactly")
    print(
        f"PASS: C4 Born to cotangent Gauss-event pushforward ({checks} exact checks)"
    )
    print(
        "Open: native preparation, general amplitudes, single-trial competition, "
        "multipartite no-signalling, and detector formation"
    )


if __name__ == "__main__":
    main()
