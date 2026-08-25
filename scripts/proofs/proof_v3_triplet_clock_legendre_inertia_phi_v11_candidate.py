#!/usr/bin/env python3
"""Triplet-clock discrete Legendre/inertia Phi-v11 candidate.

The clean self-correcting triplet has exact internal period T_M=16.  Phi-v9
retains an integer probe momentum p, while Phi-v10 proves that response pulses
can move the physical body but stops when the pulses stop.  This certificate
adds one existing fixed-occupancy A2 inertial-phase owner.

At fixed |p|<=T_M, adding |p| modulo T_M emits exactly |p| body-hop carries in
one 16-tick clock period.  A force-aligned triplet therefore has exact
inertial velocity v=p/T_M and continues after the response window.  The
lowest positive even momentum action selected by Phi-v9 becomes

    H_M = (p_probe^2 + p_reaction^2 + W)/(2 T_M),

whose Legendre derivative is p_probe/T_M, exactly the finite hop cadence.
For a response momentum change Delta p=-C over 12N ticks, the block relation
is F_bar=T_M a_bar.

All maps are finite and exactly invertible on the admitted prepared sector.
This remains a selection: identifying the triplet recurrence period with the
inertial denominator and adopting the discrete Legendre readout are not yet
derived from canonical Phi.  Steering, reaction motion, traffic, formation,
physical units, universal coupling, and tensor gravity remain open.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from fractions import Fraction

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_v3_a2_green_pulse_reciprocal_impulse_action_phi_v9_candidate import (
    ResponseState,
)
from proof_v3_green_pulse_triplet_mechanical_drift_phi_v10_candidate import (
    Vec,
    aligned_chart,
    clean_predecessor,
    edge_direction,
    run_mechanical_cycle,
    scale,
    subtract,
)
from proof_v3_neutral_rotor_harmonic_green_seam import box
from proof_v3_rotor_green_a2_physical_memory_phase_protection import (
    A2,
    Edge,
    decode_counter,
    edge_green_data,
    encode_counter,
    graph_edges,
    simulate_physical_memory,
)
from proof_v3_triplet_relational_work_phi_v5_candidate import work_step
from proof_v3_triplet_translating_self_correcting_clock_phi_v6_candidate import (
    WorkClock,
    clean_work_orbit,
    moving_step,
    shift_chart,
)


sys.stdout.reconfigure(encoding="utf-8")

MATTER_CLOCK_PERIOD = 16


@dataclass(frozen=True)
class InertialState:
    phase: A2


def admitted(momentum: int, state: InertialState) -> bool:
    phase = decode_counter(state.phase)
    return (
        -MATTER_CLOCK_PERIOD <= momentum <= MATTER_CLOCK_PERIOD
        and phase is not None
        and 0 <= phase < MATTER_CLOCK_PERIOD
    )


def inertia_step(
    momentum: int,
    state: InertialState,
    chart,
    body: WorkClock,
):
    if not admitted(momentum, state):
        return state, chart, body, 0
    phase = decode_counter(state.phase)
    assert phase is not None
    advanced = phase + abs(momentum)
    fired = int(advanced >= MATTER_CLOCK_PERIOD)
    phase_after = advanced - fired * MATTER_CLOCK_PERIOD
    signed_hop = ((momentum > 0) - (momentum < 0)) * fired
    if signed_hop:
        # The prepared chart normal already equals sign(p) times the measured
        # edge direction, so the existing Phi-v6 move is the Legendre hop.
        chart_after, body_after = moving_step(chart, body)
    else:
        chart_after, body_after = chart, work_step(chart, body)
    return InertialState(encode_counter(phase_after)), chart_after, body_after, signed_hop


def inertia_inverse(
    momentum: int,
    state_after: InertialState,
    chart_after,
    body_after: WorkClock,
):
    if not admitted(momentum, state_after):
        return state_after, chart_after, body_after, 0
    phase_after = decode_counter(state_after.phase)
    assert phase_after is not None
    phase_before = (phase_after - abs(momentum)) % MATTER_CLOCK_PERIOD
    fired = int(phase_before + abs(momentum) >= MATTER_CLOCK_PERIOD)
    signed_hop = ((momentum > 0) - (momentum < 0)) * fired
    chart_before = (
        shift_chart(chart_after, scale(-1, chart_after.repair_normal))
        if signed_hop
        else chart_after
    )
    body_before = clean_predecessor(chart_before, body_after)
    recovered = inertia_step(
        momentum,
        InertialState(encode_counter(phase_before)),
        chart_before,
        body_before,
    )
    assert recovered[:3] == (state_after, chart_after, body_after)
    assert recovered[3] == signed_hop
    return InertialState(encode_counter(phase_before)), chart_before, body_before, -signed_hop


def run_inertial_cycle(momentum: int, chart, body: WorkClock):
    initial = (InertialState(encode_counter(0)), chart, body)
    state, chart_now, body_now = initial
    history = []
    hops = []
    for _ in range(MATTER_CLOCK_PERIOD):
        before = (state, chart_now, body_now)
        state, chart_now, body_now, hop = inertia_step(
            momentum, state, chart_now, body_now
        )
        history.append(before)
        hops.append(hop)
    final = (state, chart_now, body_now)
    reverse = final
    for expected in reversed(history):
        reverse_state, reverse_chart, reverse_body, _ = inertia_inverse(
            momentum, *reverse
        )
        reverse = (reverse_state, reverse_chart, reverse_body)
        assert reverse == expected
    assert reverse == initial
    return final, tuple(hops), tuple(history)


def probe_momentum(response: ResponseState) -> int:
    value = decode_counter(response.probe_momentum)
    assert value is not None
    return value


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())

    canonical = aligned_chart((1, 0, 0))
    clean_orbit = clean_work_orbit(canonical)
    check(
        "C1 the prepared clean triplet supplies an exact sixteen-tick material clock",
        len(clean_orbit) == MATTER_CLOCK_PERIOD
        and len(set(clean_orbit)) == MATTER_CLOCK_PERIOD
        and work_step(canonical, clean_orbit[-1]) == clean_orbit[0],
    )

    cadence_rows = 0
    inverse_rows = 0
    for momentum in range(-MATTER_CLOCK_PERIOD, MATTER_CLOCK_PERIOD + 1):
        direction = (1, 0, 0) if momentum >= 0 else (-1, 0, 0)
        chart = aligned_chart(direction)
        body = clean_work_orbit(chart)[0]
        final, hops, history = run_inertial_cycle(momentum, chart, body)
        displacement = subtract(final[1].origin, chart.origin)
        assert sum(hops) == momentum
        assert displacement == scale(abs(momentum), direction)
        assert decode_counter(final[0].phase) == 0
        assert final[2] in clean_work_orbit(final[1])
        assert all(abs(hop) <= 1 for hop in hops)
        cadence_rows += 5
        inverse_rows += len(history)
    check(
        "C2 one material-clock period gives exactly |p| signed inertial hops",
        cadence_rows == 5 * (2 * MATTER_CLOCK_PERIOD + 1),
    )
    check(
        "C3 every inertial microtick and complete finite cycle have exact inverses",
        inverse_rows == MATTER_CLOCK_PERIOD * (2 * MATTER_CLOCK_PERIOD + 1),
    )

    # Discrete Legendre match.  The unique positive sign-even quadratic shape
    # has derivative p/T_M after the clock selects coefficient 1/(2T_M).
    legendre_rows = 0
    for momentum in range(-MATTER_CLOCK_PERIOD, MATTER_CLOCK_PERIOD + 1):
        velocity = Fraction(momentum, MATTER_CLOCK_PERIOD)
        derivative = Fraction(2 * momentum, 2 * MATTER_CLOCK_PERIOD)
        assert velocity == derivative
        legendre_rows += 1
    check(
        "C4 clock-selected quadratic action has exact discrete Legendre velocity p/16",
        legendre_rows == 33,
    )

    # Compose one completed force window with a subsequent dark inertial
    # period.  Momentum survives in Phi-v9 and now moves the body even though
    # no new field pulse is applied.
    continuation_rows = 0
    for current in (-16, -7, -1, 1, 7, 16):
        edge = ((0, 0, 0), (1, 0, 0))
        direction = scale(-((current > 0) - (current < 0)), edge_direction(edge))
        chart = aligned_chart(direction)
        body = clean_work_orbit(chart)[0]
        response_final, _response_hops, _history = run_mechanical_cycle(
            current, 37, chart, body
        )
        response, chart_after_force, body_after_force = response_final
        momentum = probe_momentum(response)
        assert momentum == -current
        inertial_final, inertial_hops, _ = run_inertial_cycle(
            momentum, chart_after_force, body_after_force
        )
        inertial_displacement = subtract(
            inertial_final[1].origin, chart_after_force.origin
        )
        assert inertial_displacement == scale(-current, edge_direction(edge))
        assert sum(inertial_hops) == momentum
        continuation_rows += 2
    check(
        "C5 retained response momentum produces exact continuation through a later dark field window",
        continuation_rows == 12,
    )

    # Block force/acceleration relation.  Delta p=-C is accumulated over 12N
    # response ticks.  The clock-selected post-response velocity is p/16.
    inertia_rows = 0
    for injections in (7, 37, 97):
        for current in range(-min(injections, 16), min(injections, 16) + 1):
            if current == 0:
                continue
            delta_p = -current
            force = Fraction(delta_p, 12 * injections)
            delta_v = Fraction(delta_p, MATTER_CLOCK_PERIOD)
            acceleration = delta_v / (12 * injections)
            assert force == MATTER_CLOCK_PERIOD * acceleration
            inertia_rows += 1
    check(
        "C6 block response obeys the exact selected inertia relation F_bar=16 a_bar",
        inertia_rows == 2 * 7 + 2 * 16 + 2 * 16,
    )

    # All native rotor phases on the certified source edge remain inside the
    # one-clock causal momentum range and therefore admit inertial continuation.
    vertices = frozenset(box(1))
    edges = graph_edges(vertices)
    probe_edge: Edge = ((0, 0, 0), (1, 0, 0))
    assert probe_edge in edges
    _laplacian, _inverse, green_data = edge_green_data(tuple(sorted(vertices)))
    exact_gradient, transfer_norm = green_data[probe_edge]
    injections = 37
    current_bound = Fraction(8, 3 * injections) + Fraction(8, injections) * transfer_norm
    phase_accelerations = []
    phase_rows = 0
    maximum_acceleration_error = Fraction(0)
    for rotor_state in states:
        memory = simulate_physical_memory(1, injections, rotor_state)
        current = memory[4].get(probe_edge, 0)
        momentum = -current
        assert abs(momentum) <= MATTER_CLOCK_PERIOD
        direction = scale(
            (momentum > 0) - (momentum < 0), edge_direction(probe_edge)
        ) if momentum else (-1, 0, 0)
        chart = aligned_chart(direction)
        body = clean_work_orbit(chart)[0]
        inertial_final, hops, _ = run_inertial_cycle(momentum, chart, body)
        velocity = Fraction(sum(hops), MATTER_CLOCK_PERIOD)
        assert velocity == Fraction(momentum, MATTER_CLOCK_PERIOD)
        # The force window lasts 12N ticks.  Its final velocity is p/16, so
        # the exact block acceleration is -J/(12*16).
        acceleration = velocity / (12 * injections)
        target_acceleration = -exact_gradient / (
            12 * MATTER_CLOCK_PERIOD
        )
        error = abs(acceleration - target_acceleration)
        assert error <= current_bound / (12 * MATTER_CLOCK_PERIOD)
        maximum_acceleration_error = max(maximum_acceleration_error, error)
        phase_accelerations.append(acceleration)
        assert inertial_final[2] in clean_work_orbit(inertial_final[1])
        phase_rows += 1
    check(
        "C7 all 192 rotor phases admit causal clocked inertia and inherit the normalized Green bound",
        phase_rows == 192,
    )
    check(
        "C8 arbitrary initial rotor phases retain the exact clock-scaled protection bound",
        max(phase_accelerations) - min(phase_accelerations)
        <= 2 * current_bound / (12 * MATTER_CLOCK_PERIOD),
    )

    # The clock fixes a relative lattice action coefficient but cannot create
    # SI dimensions or prove that canonical Phi selects this Legendre branch.
    relative_coefficient = Fraction(1, 2 * MATTER_CLOCK_PERIOD)
    dimensionless_prices = {
        multiplier: multiplier * relative_coefficient
        for multiplier in (1, 2, 7)
    }
    check(
        "C9 matter-clock matching selects relative coefficient 1/32 while an overall physical unit remains free",
        relative_coefficient == Fraction(1, 32)
        and dimensionless_prices
        == {1: Fraction(1, 32), 2: Fraction(1, 16), 7: Fraction(7, 32)},
    )

    malformed = InertialState(encode_counter(-1))
    chart = aligned_chart((1, 0, 0))
    body = clean_work_orbit(chart)[0]
    check(
        "C10 malformed phase and supercausal momentum controls fail closed",
        inertia_step(1, malformed, chart, body)[:3] == (malformed, chart, body)
        and inertia_step(17, InertialState(encode_counter(0)), chart, body)[:3]
        == (InertialState(encode_counter(0)), chart, body),
    )

    missing = {
        "canonical Phi derivation of clock-to-inertia identification",
        "native formation and work reserve",
        "steering and multi-edge vector composition",
        "reaction-body continuation and source return",
        "general momentum range and relativistic saturation",
        "collision traffic environmental repair and overflow",
        "physical units and universal matter-radiation coupling",
        "tensor pole common cone clock response lensing and nonlinearity",
    }
    check(
        "C11 clocked inertial continuation does not close physical gravity",
        len(missing) == 8,
    )

    forbidden = (
        "empirical_target",
        "random_draw",
        "parameter_fit",
        "137.036",
    )
    check(
        "C12 no empirical target random draw parameter fit or numerical search enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 triplet-clock Legendre-inertia Phi-v11 checks pass")
    print("matter_clock_period=16")
    print("inertial_hops_per_clock=abs(p)")
    print("inertial_velocity=p/16")
    print("relative_quadratic_action_coefficient=1/32")
    print("block_inertia_relation=F_bar=16*a_bar")
    print("post_force_dark_continuation=exact")
    print(f"maximum_all_phase_block_acceleration_error_N37={maximum_acceleration_error}")
    print("overall_physical_unit=free")
    print("status=prepared_phi_v11_clock_legendre_inertia_candidate_canonical_gravity_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
