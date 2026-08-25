#!/usr/bin/env python3
"""Exact recurrent C4 actualization clock and capacity-gated local tick.

A persistent bright record pair drives one detector token through the same
reversible actualization gate on every admitted tick, while global C4 advance
rotates all phase payloads.  Because actualization has order two and phase
advance has order four and they commute, the joint state has exact period four.

Over one cycle the vector and common phase-tensor moments cancel, while the
bond retains a nonzero average capacity deficit and neutral recurrent ternary
manifestation.  A finite permission word demonstrates global ticks with a
locally admitted proper-tick count.  The physical origin of that permission
from gravity/backpressure remains open.
"""

from __future__ import annotations

from dataclasses import dataclass
from itertools import product

from sympy import Matrix, Rational

from proof_c18_actualization_moment_source_vertex import (
    LINE_DYADS,
    MomentChart,
    zero_chart,
)
from proof_c4_controlled_actualization_transaction import charge, token_count
from proof_c4_physical_born_actualization_tape import (
    DetectorCell,
    detector_gate,
    fresh_cell,
    rotate_cell,
    rotate_record,
)
from proof_reversible_c4_cancellation_click_circuit import Record, click_compatible


@dataclass(frozen=True)
class ClockState:
    controller_left: Record
    controller_right: Record
    detector: DetectorCell


def clock_step(state: ClockState, capacity_open: bool = True) -> ClockState:
    if not capacity_open:
        return state
    transacted = detector_gate(
        state.controller_left,
        state.controller_right,
        state.detector,
    )
    rotated_left = rotate_record(state.controller_left, 1)
    rotated_right = rotate_record(state.controller_right, 1)
    assert rotated_left is not None and rotated_right is not None
    return ClockState(
        rotated_left,
        rotated_right,
        rotate_cell(transacted, 1),
    )


def initial_clock(outcome: int, phase: int, orientation: int) -> ClockState:
    left = Record(outcome, phase, 2 * outcome)
    right = Record(outcome, phase, 2 * outcome + 1)
    detector = fresh_cell(phase, orientation)
    # fresh_cell uses tick modulo four, so this sets the detector phase to the
    # same declared initial phase without reading a target frequency.
    assert detector.actualization.reserve is not None
    assert detector.actualization.reserve.phase == phase
    return ClockState(left, right, detector)


def add_charts(charts: tuple[MomentChart, ...]) -> MomentChart:
    return MomentChart(
        sum((chart.relative_u for chart in charts), start=Matrix.zeros(3, 1)),
        sum((chart.relative_v for chart in charts), start=Matrix.zeros(3, 1)),
        sum((chart.tensor_q for chart in charts), start=Matrix.zeros(3, 3)),
        sum((chart.tensor_p for chart in charts), start=Matrix.zeros(3, 3)),
        sum((chart.capacity for chart in charts), start=Matrix.zeros(3, 3)),
        sum(chart.state_left for chart in charts),
        sum(chart.state_right for chart in charts),
    )


def advance(state: ClockState, admitted_ticks: int) -> ClockState:
    output = state
    for _ in range(admitted_ticks):
        output = clock_step(output, True)
    return output


def main() -> None:
    checks = 0

    for line_index in range(9):
        dyad = LINE_DYADS[line_index]
        for outcome, phase, orientation in product(range(3), range(4), (-1, 1)):
            initial = initial_clock(outcome, phase, orientation)
            assert click_compatible(initial.controller_left, initial.controller_right)
            states = []
            state = initial
            for tick in range(1, 5):
                state = clock_step(state, True)
                states.append(state)
                assert click_compatible(state.controller_left, state.controller_right)
                assert token_count(state.detector.actualization) == 1
                assert charge(state.detector.actualization) == 0
                expected_phase = (phase + tick) % 4
                owned_token = (
                    state.detector.actualization.link
                    if state.detector.actualization.link is not None
                    else state.detector.actualization.reserve
                )
                assert owned_token is not None and owned_token.phase == expected_phase
                assert (state.detector.port is not None) == (tick % 2 == 1)
                checks += 5

            assert states[-1] == initial
            assert all(candidate != initial for candidate in states[:-1])
            checks += 2

            charts = tuple(
                zero_chart(candidate.detector.actualization, line_index)
                for candidate in states
            )
            total = add_charts(charts)
            assert total.relative_u == Matrix.zeros(3, 1)
            assert total.relative_v == Matrix.zeros(3, 1)
            assert total.tensor_q == Matrix.zeros(3, 3)
            assert total.tensor_p == Matrix.zeros(3, 3)
            assert total.capacity / 4 == dyad / 12
            assert total.state_left + total.state_right == 0
            assert sum(
                chart.state_left**2 + chart.state_right**2 for chart in charts
            ) == 4
            checks += 7

    # Capacity/backpressure gating: global ticks always advance, while the
    # local recurrent state advances only on admitted transactions.  The final
    # state depends on the count of open gates, not their placement.
    reference = initial_clock(0, 0, 1)
    for length in range(9):
        for permission_word in product((False, True), repeat=length):
            state = reference
            for permission in permission_word:
                state = clock_step(state, permission)
            admitted = sum(permission_word)
            assert state == advance(reference, admitted)
            assert state == advance(reference, admitted % 4)
            checks += 2

    print(f"PASS: recurrent C4 actualization material clock ({checks} exact checks)")
    print("exact local recurrence period=4 admitted ticks")
    print("cycle sums: relative vectors=0, phase tensors=0, mean capacity=M/12")
    print("neutral recurrent ternary activity: mean (s_left^2+s_right^2)=1")
    print("global ticks vs local admitted ticks verified for every permission word through length 8")
    print(
        "Open here: autonomous controller/body formation and physical permission law; "
        "a finite A9 feedback successor exists"
    )


if __name__ == "__main__":
    main()
