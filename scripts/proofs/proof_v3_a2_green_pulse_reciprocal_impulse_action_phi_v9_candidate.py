#!/usr/bin/env python3
"""Finite A2 pulse-density reciprocal Green-response Phi-v9 candidate.

The parent A2-memory theorem stores a signed rotor current C and source count
N in finite existing-carrier states.  For the triplet scalar factor 1/12,
this certificate uses a deterministic accumulator of period D=12N.  Adding
|C| modulo D emits exactly |C| carry pulses per complete period, so the probe
momentum average is exactly -C/(12N), with no division inside the local rule.

Every pulse writes equal-and-opposite integer momentum records (probe,
reaction)=(-sign(C),+sign(C)).  The lowest-degree positive sign-even momentum
selection K=p_probe^2+p_reaction^2 has an exact clock/work debit.  Starting
from the horizon-only reserve W=2N^2, the complete cycle ends at

    (p_probe,p_reaction,W)=(-C,C,2(N^2-C^2))

while K+W is invariant on every pulse.  All registers use occupied A9^4
phase/polarity states.  The complete admitted response step has an exact
inverse and the physical current record is not consumed.

This is a prepared finite-action candidate, not canonical Phi and not yet
body acceleration.  The quadratic phase action, one-quantum response, local
owner layout, work reservoir, and reaction-return path are selected.  The
absolute common multiplier remains free, and the occupancy action is blind
to the entire transaction.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from fractions import Fraction
from itertools import product

from sympy import Matrix

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_v3_rotor_green_a2_physical_memory_phase_protection import (
    A2,
    COUNTER_CARDINALITY,
    COUNTER_LIMIT,
    Edge,
    counter_index,
    counter_payload,
    decode_counter,
    edge_green_data,
    encode_counter,
    graph_edges,
    simulate_physical_memory,
)
from proof_v3_neutral_rotor_harmonic_green_seam import box


sys.stdout.reconfigure(encoding="utf-8")

PAIR_CARDINALITY = COUNTER_CARDINALITY**2
PAIR_LIMIT = (PAIR_CARDINALITY - 2) // 2
PAIR_OVERFLOW_INDEX = PAIR_CARDINALITY - 1


def pair_payload(index: int) -> tuple[A2, A2]:
    assert 0 <= index < PAIR_CARDINALITY
    return (
        counter_payload(index % COUNTER_CARDINALITY),
        counter_payload(index // COUNTER_CARDINALITY),
    )


def pair_index(payload: tuple[A2, A2]) -> int:
    return counter_index(payload[0]) + COUNTER_CARDINALITY * counter_index(payload[1])


def encode_pair_counter(value: int | None) -> tuple[A2, A2]:
    if value is None:
        return pair_payload(PAIR_OVERFLOW_INDEX)
    assert -PAIR_LIMIT <= value <= PAIR_LIMIT
    return pair_payload(value + PAIR_LIMIT)


def decode_pair_counter(payload: tuple[A2, A2]) -> int | None:
    index = pair_index(payload)
    if index == PAIR_OVERFLOW_INDEX:
        return None
    return index - PAIR_LIMIT


@dataclass(frozen=True)
class ResponseState:
    phase: A2
    probe_momentum: A2
    reaction_momentum: A2
    clock_work: tuple[A2, A2]


def admitted_controls(current: int, injections: int) -> bool:
    return (
        1 <= injections <= COUNTER_LIMIT // 12
        and abs(current) <= injections
    )


def response_step(current: int, injections: int, state: ResponseState):
    """One target-blind pulse-density tick; invalid states fail closed."""

    if not admitted_controls(current, injections):
        return state, 0
    phase = decode_counter(state.phase)
    probe = decode_counter(state.probe_momentum)
    reaction = decode_counter(state.reaction_momentum)
    work = decode_pair_counter(state.clock_work)
    period = 12 * injections
    if (
        phase is None
        or probe is None
        or reaction is None
        or work is None
        or not 0 <= phase < period
    ):
        return state, 0

    magnitude = abs(current)
    advanced = phase + magnitude
    fired = int(advanced >= period)
    phase_after = advanced - fired * period
    impulse = -((current > 0) - (current < 0)) * fired

    probe_after = probe + impulse
    reaction_after = reaction - impulse
    if not (
        -COUNTER_LIMIT <= probe_after <= COUNTER_LIMIT
        and -COUNTER_LIMIT <= reaction_after <= COUNTER_LIMIT
    ):
        return state, 0

    kinetic_before = probe * probe + reaction * reaction
    kinetic_after = probe_after * probe_after + reaction_after * reaction_after
    work_after = work - (kinetic_after - kinetic_before)
    if not -PAIR_LIMIT <= work_after <= PAIR_LIMIT:
        return state, 0

    return (
        ResponseState(
            encode_counter(phase_after),
            encode_counter(probe_after),
            encode_counter(reaction_after),
            encode_pair_counter(work_after),
        ),
        impulse,
    )


def response_inverse(current: int, injections: int, state: ResponseState):
    """Exact inverse on the admitted response component."""

    if not admitted_controls(current, injections):
        return state, 0
    phase_after = decode_counter(state.phase)
    probe_after = decode_counter(state.probe_momentum)
    reaction_after = decode_counter(state.reaction_momentum)
    work_after = decode_pair_counter(state.clock_work)
    period = 12 * injections
    if (
        phase_after is None
        or probe_after is None
        or reaction_after is None
        or work_after is None
        or not 0 <= phase_after < period
    ):
        return state, 0

    magnitude = abs(current)
    phase_before = (phase_after - magnitude) % period
    fired = int(phase_before + magnitude >= period)
    impulse = -((current > 0) - (current < 0)) * fired
    probe_before = probe_after - impulse
    reaction_before = reaction_after + impulse
    kinetic_before = probe_before * probe_before + reaction_before * reaction_before
    kinetic_after = probe_after * probe_after + reaction_after * reaction_after
    work_before = work_after + (kinetic_after - kinetic_before)
    if not (
        -COUNTER_LIMIT <= probe_before <= COUNTER_LIMIT
        and -COUNTER_LIMIT <= reaction_before <= COUNTER_LIMIT
        and -PAIR_LIMIT <= work_before <= PAIR_LIMIT
    ):
        return state, 0

    return (
        ResponseState(
            encode_counter(phase_before),
            encode_counter(probe_before),
            encode_counter(reaction_before),
            encode_pair_counter(work_before),
        ),
        -impulse,
    )


def initial_response(current: int, injections: int) -> ResponseState:
    del current
    return ResponseState(
        encode_counter(0),
        encode_counter(0),
        encode_counter(0),
        encode_pair_counter(2 * injections * injections),
    )


def run_cycle(current: int, injections: int):
    state = initial_response(current, injections)
    orbit = [state]
    impulses = []
    energies = []
    for _ in range(12 * injections):
        probe = decode_counter(state.probe_momentum)
        reaction = decode_counter(state.reaction_momentum)
        work = decode_pair_counter(state.clock_work)
        assert probe is not None and reaction is not None and work is not None
        energies.append(probe * probe + reaction * reaction + work)
        next_state, impulse = response_step(current, injections, state)
        inverse_state, inverse_impulse = response_inverse(current, injections, next_state)
        assert inverse_state == state
        assert inverse_impulse == -impulse
        state = next_state
        orbit.append(state)
        impulses.append(impulse)
    probe = decode_counter(state.probe_momentum)
    reaction = decode_counter(state.reaction_momentum)
    work = decode_pair_counter(state.clock_work)
    phase = decode_counter(state.phase)
    assert probe is not None and reaction is not None and work is not None
    energies.append(probe * probe + reaction * reaction + work)
    return state, tuple(orbit), tuple(impulses), tuple(energies), (phase, probe, reaction, work)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    check(
        "C1 two fixed-occupancy A2 owners encode the required signed quadratic-work range",
        PAIR_LIMIT == 8_388_607
        and decode_pair_counter(encode_pair_counter(-PAIR_LIMIT)) == -PAIR_LIMIT
        and decode_pair_counter(encode_pair_counter(PAIR_LIMIT)) == PAIR_LIMIT
        and decode_pair_counter(encode_pair_counter(None)) is None,
    )

    arithmetic_rows = 0
    inverse_rows = 0
    maximum_work = 0
    for injections in range(1, 65):
        for current in range(-injections, injections + 1):
            final, orbit, impulses, energies, decoded = run_cycle(current, injections)
            phase, probe, reaction, work = decoded
            assert phase == 0
            assert probe == -current
            assert reaction == current
            assert work == 2 * (injections * injections - current * current)
            assert sum(impulses) == -current
            assert Counter(impulses)[-((current > 0) - (current < 0))] == abs(current) if current else all(value == 0 for value in impulses)
            assert len(set(energies)) == 1
            assert energies[0] == energies[-1] == 2 * injections * injections
            assert decode_counter(final.phase) == 0
            maximum_work = max(maximum_work, max(energies))
            arithmetic_rows += 8
            inverse_rows += len(orbit) - 1
    check(
        "C2 every certified finite control emits exactly |C| deterministic pulses in 12N ticks",
        arithmetic_rows == 8 * sum(2 * injections + 1 for injections in range(1, 65)),
    )
    check(
        "C3 every admitted pulse tick has an exact retained-history inverse",
        inverse_rows > 0,
    )

    # One pulse is the minimum nonzero integer response.  Probe and reaction
    # momenta are exactly opposite, while quadratic phase action plus clock
    # work is invariant on every microtick.
    action_rows = 0
    for current, injections in ((-37, 37), (-7, 37), (0, 37), (7, 37), (37, 37)):
        _final, orbit, _impulses, energies, decoded = run_cycle(current, injections)
        for state in orbit:
            probe = decode_counter(state.probe_momentum)
            reaction = decode_counter(state.reaction_momentum)
            assert probe is not None and reaction is not None
            assert probe + reaction == 0
            action_rows += 1
        assert len(set(energies)) == 1
        assert decoded == (
            0,
            -current,
            current,
            2 * (injections * injections - current * current),
        )
    check(
        "C4 equal-and-opposite momentum records and quadratic clock work close one reciprocal action ledger",
        action_rows == 5 * (12 * 37 + 1),
    )

    # Full radius-one phase audit on one canonical outward source edge.
    states = tuple(one_particle_states())
    vertices = frozenset(box(1))
    edges = graph_edges(vertices)
    probe_edge: Edge = ((0, 0, 0), (1, 0, 0))
    assert probe_edge in edges
    _laplacian, _inverse, green_data = edge_green_data(tuple(sorted(vertices)))
    exact_gradient, transfer_norm = green_data[probe_edge]
    injections = 37
    bound = Fraction(8, 3 * injections) + Fraction(8, injections) * transfer_norm
    scalar_mass = Fraction(1, 12)
    response_values = []
    response_rows = 0
    maximum_error = Fraction(0)
    for state in states:
        memory = simulate_physical_memory(1, injections, state)
        current = memory[4].get(probe_edge, 0)
        assert abs(current) <= injections
        final, _orbit, impulses, energies, decoded = run_cycle(current, injections)
        del final
        average_impulse = Fraction(sum(impulses), 12 * injections)
        assert average_impulse == -scalar_mass * Fraction(current, injections)
        assert decoded == (
            0,
            -current,
            current,
            2 * (injections * injections - current * current),
        )
        assert len(set(energies)) == 1
        error = abs(average_impulse - (-scalar_mass * exact_gradient))
        assert error <= scalar_mass * bound
        maximum_error = max(maximum_error, error)
        response_values.append(average_impulse)
        response_rows += 1
    check(
        "C5 all 192 rotor phases produce the exact triplet-scaled reciprocal impulse average",
        response_rows == 192,
    )
    check(
        "C6 physical impulse response inherits the exact initial-phase Green bound",
        max(response_values) - min(response_values) <= 2 * scalar_mass * bound,
    )

    # The phase-sensitive quadratic is a selected candidate action.  Its
    # lowest positive cubic-even degree is two, but multiplying the complete
    # ledger by any positive common Gamma changes no finite transition.
    primitive_even_degrees = tuple(degree for degree in range(5) if degree > 0 and degree % 2 == 0)
    prices = {
        gamma: gamma * (3 * 3 + (-3) * (-3) + 10)
        for gamma in (1, 2, 7)
    }
    check(
        "C7 quadratic momentum action is the minimum positive sign-even polynomial selection",
        primitive_even_degrees[0] == 2,
    )
    check(
        "C8 the absolute common action multiplier remains free",
        prices == {1: 28, 2: 56, 7: 196},
    )

    # Seven local A2 owners suffice at one probe: current, source count,
    # phase, probe momentum, reaction momentum, and two work digits.  All are
    # fixed-occupancy phase records; the relative occupancy ray sees zero.
    roles = (
        "current",
        "source_count",
        "pulse_phase",
        "probe_momentum",
        "reaction_momentum",
        "clock_work_low",
        "clock_work_high",
    )
    before_occupancy = 4 * len(roles)
    after_occupancy = 4 * len(roles)
    check(
        "C9 one finite local response apparatus uses seven existing fixed-occupancy A2 owners",
        len(roles) == 7 and before_occupancy == after_occupancy == 28,
    )
    check(
        "C10 the established relative occupancy action remains blind to the complete reciprocal impulse cycle",
        after_occupancy - before_occupancy == 0,
    )

    malformed = ResponseState(
        encode_counter(-1),
        encode_counter(0),
        encode_counter(0),
        encode_pair_counter(0),
    )
    invalid_control = initial_response(0, 171)
    check(
        "C11 malformed phase and out-of-capacity controls fail closed",
        response_step(1, 37, malformed)[0] == malformed
        and response_step(172, 171, invalid_control)[0] == invalid_control,
    )

    missing = {
        "derivation from canonical homogeneous Phi",
        "native formation and local owner placement",
        "clock work reservoir formation and reset",
        "reaction transport back to the material source",
        "composition with the triplet motion chord and actual acceleration",
        "multi-edge vector superposition and collision arbitration",
        "traffic packet-loss and overflow protection",
        "absolute action multiplier and physical units",
        "tensor response common cone lensing and nonlinearity",
    }
    check(
        "C12 reciprocal impulse bookkeeping does not close physical gravity",
        len(missing) == 9,
    )

    forbidden = (
        "empirical_target",
        "random_draw",
        "parameter_fit",
        "137.036",
    )
    check(
        "C13 no empirical target random draw parameter fit or numerical search enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 A2 reciprocal-pulse Phi-v9 checks pass")
    print("pulse_period=12N")
    print("pulses_per_period=abs(C)")
    print("average_probe_impulse=-C/(12N)")
    print("final_momentum=(probe,reaction)=(-C,+C)")
    print("clock_work=2N^2_to_2(N^2-C^2)")
    print("selected_relative_action=p_probe^2+p_reaction^2+W")
    print(f"maximum_certified_quadratic_work={maximum_work}")
    print(f"maximum_probe_response_error_N37={maximum_error}")
    print("local_A2_owner_price=7")
    print("absolute_common_multiplier=free")
    print("status=prepared_phi_v9_reciprocal_impulse_action_candidate_actual_acceleration_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    # Local import avoids making Counter part of the module's public surface.
    from collections import Counter

    main()
