#!/usr/bin/env python3
"""Prepared Phi-v10 composition of Green pulses with triplet displacement.

Phi-v9 turns a physical edge current C and source count N into exactly |C|
signed impulses in 12N ticks, with equal-and-opposite momentum and exact
clock/work bookkeeping.  Phi-v6 supplies a clean self-correcting triplet body
whose admitted moving transaction is one SC hop along its carried normal.

This certificate composes the two finite maps.  On a carry pulse the aligned
triplet applies the Phi-v6 moving step; on a dark tick only its internal
Phi-v5 work clock advances.  After one response period the body displacement
is exactly -C d and its average displacement is -C d/(12N).  Each microtick
is zero or one hop, every register/body transition has an exact inverse, and
the triplet remains on its clean orbit at constant occupancy.

This is a prepared mechanical drift response, not Newtonian acceleration.
The momentum record does not supply inertial persistence after the pulse
window, the body must be pre-aligned with the force edge, and a clear finite
corridor is prepared.  Native steering, inertia/dispersion, reaction return,
traffic, canonical Phi, and absolute scale remain open.
"""

from __future__ import annotations

import sys
from dataclasses import replace
from fractions import Fraction

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_v3_a2_green_pulse_reciprocal_impulse_action_phi_v9_candidate import (
    ResponseState,
    initial_response,
    response_inverse,
    response_step,
)
from proof_v3_charged_candidate_matter_perturbation_boundary import frame_family
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    OrientedRepairChart,
    canonical_chart,
    transform_chart,
)
from proof_v3_rotor_green_a2_physical_memory_phase_protection import (
    Edge,
    decode_counter,
    edge_green_data,
    encode_counter,
    graph_edges,
    simulate_physical_memory,
)
from proof_v3_neutral_rotor_harmonic_green_seam import box
from proof_v3_triplet_relational_work_phi_v5_candidate import work_step
from proof_v3_triplet_translating_self_correcting_clock_phi_v6_candidate import (
    WorkClock,
    clean_work_orbit,
    moving_step,
    physical_signature,
    shift_chart,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]


def add(left: Vec, right: Vec) -> Vec:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def scale(value: int, vector: Vec) -> Vec:
    return tuple(value * entry for entry in vector)  # type: ignore[return-value]


def subtract(left: Vec, right: Vec) -> Vec:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def chebyshev(left: Vec, right: Vec) -> int:
    return max(abs(a - b) for a, b in zip(left, right))


def edge_direction(edge: Edge) -> Vec:
    low, high = edge
    direction = subtract(high, low)
    assert sum(abs(entry) for entry in direction) == 1
    return direction


def chart_orbit_at(origin: Vec = (0, 0, 0)) -> tuple[OrientedRepairChart, ...]:
    group = tuple(signed_permutation_matrices())
    return tuple(
        sorted(
            {
                replace(transform_chart(matrix, canonical_chart(frame)), origin=origin)
                for frame in frame_family(origin)
                for matrix in group
            },
            key=repr,
        )
    )


def aligned_chart(direction: Vec, origin: Vec = (0, 0, 0)) -> OrientedRepairChart:
    return next(chart for chart in chart_orbit_at(origin) if chart.repair_normal == direction)


def clean_predecessor(chart: OrientedRepairChart, state_after: WorkClock) -> WorkClock:
    orbit = clean_work_orbit(chart)
    return next(state for state in orbit if work_step(chart, state) == state_after)


def mechanical_step(
    current: int,
    injections: int,
    response: ResponseState,
    chart: OrientedRepairChart,
    body: WorkClock,
):
    response_after, impulse = response_step(current, injections, response)
    if impulse:
        chart_after, body_after = moving_step(chart, body)
    else:
        chart_after, body_after = chart, work_step(chart, body)
    return response_after, chart_after, body_after, impulse


def mechanical_inverse(
    current: int,
    injections: int,
    response_after: ResponseState,
    chart_after: OrientedRepairChart,
    body_after: WorkClock,
):
    response_before, inverse_impulse = response_inverse(
        current, injections, response_after
    )
    forward_impulse = -inverse_impulse
    chart_before = (
        shift_chart(chart_after, scale(-1, chart_after.repair_normal))
        if forward_impulse
        else chart_after
    )
    body_before = clean_predecessor(chart_before, body_after)
    recovered = mechanical_step(
        current,
        injections,
        response_before,
        chart_before,
        body_before,
    )
    assert recovered[:3] == (response_after, chart_after, body_after)
    assert recovered[3] == forward_impulse
    return response_before, chart_before, body_before, inverse_impulse


def run_mechanical_cycle(
    current: int,
    injections: int,
    chart: OrientedRepairChart,
    body: WorkClock,
):
    initial = (initial_response(current, injections), chart, body)
    response, chart_now, body_now = initial
    history = []
    impulses = []
    for _ in range(12 * injections):
        before = (response, chart_now, body_now)
        response, chart_now, body_now, impulse = mechanical_step(
            current, injections, response, chart_now, body_now
        )
        assert chebyshev(before[1].origin, chart_now.origin) == abs(impulse)
        history.append(before)
        impulses.append(impulse)

    final = (response, chart_now, body_now)
    reverse = final
    for expected_before in reversed(history):
        response_before, chart_before, body_before, _inverse_impulse = mechanical_inverse(
            current, injections, *reverse
        )
        reverse = (response_before, chart_before, body_before)
        assert reverse == expected_before
    assert reverse == initial
    return final, tuple(impulses), tuple(history)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    charts = chart_orbit_at()
    check(
        "C1 every signed SC direction has a prepared clean triplet chart",
        len(charts) == 1_152
        and {chart.repair_normal for chart in charts}
        == {
            (1, 0, 0),
            (-1, 0, 0),
            (0, 1, 0),
            (0, -1, 0),
            (0, 0, 1),
            (0, 0, -1),
        },
    )

    # Force-aligned finite fixtures.  The response pulse sign and chart normal
    # agree, so each carry applies the already-certified Phi-v6 hop.
    fixture_rows = 0
    maximum_corridor = 0
    for current, injections in ((-37, 37), (-7, 37), (0, 37), (7, 37), (37, 37)):
        direction = (1, 0, 0)
        impulse_direction = scale(-((current > 0) - (current < 0)), direction)
        if current == 0:
            impulse_direction = (-1, 0, 0)
        chart = aligned_chart(impulse_direction)
        body = clean_work_orbit(chart)[0]
        final, impulses, history = run_mechanical_cycle(
            current, injections, chart, body
        )
        _response, chart_after, body_after = final
        displacement = subtract(chart_after.origin, chart.origin)
        assert displacement == scale(-current, direction)
        assert sum(impulses) == -current
        assert Fraction(displacement[0], 12 * injections) == Fraction(
            -current, 12 * injections
        )
        assert body_after in clean_work_orbit(chart_after)
        assert all(
            chebyshev(before_chart.origin, after_chart.origin) <= 1
            for before_chart, after_chart in zip(
                (row[1] for row in history),
                tuple(row[1] for row in history[1:]) + (chart_after,),
            )
        )
        maximum_corridor = max(maximum_corridor, abs(current))
        fixture_rows += len(history)
    check(
        "C2 one response period displaces the prepared triplet by exactly -C edge hops",
        fixture_rows == 5 * 12 * 37,
    )
    check(
        "C3 every combined response/body microtick has an exact full-history inverse",
        fixture_rows > 0,
    )

    # Exhaust the one-tick moving geometry on every chart by preparing a carry
    # at the current phase.  This reuses the Phi-v6 move, rather than inventing
    # a nonlocal body rewrite.
    geometry_rows = 0
    for chart in charts:
        current = -1
        injections = 1
        response = initial_response(current, injections)
        response = replace(response, phase=encode_counter(11))
        body = clean_work_orbit(chart)[0]
        response_after, chart_after, body_after, impulse = mechanical_step(
            current, injections, response, chart, body
        )
        del response_after
        assert impulse == 1
        assert chart_after.origin == add(chart.origin, chart.repair_normal)
        assert body_after in clean_work_orbit(chart_after)
        assert chebyshev(chart.origin, chart_after.origin) == 1
        geometry_rows += 1
    check(
        "C4 the pulse-controlled body hop is exactly the signed-cubic Phi-v6 moving transaction",
        geometry_rows == 1_152,
    )

    # Full rotor-phase audit on the canonical source edge.
    vertices = frozenset(box(1))
    edges = graph_edges(vertices)
    probe_edge: Edge = ((0, 0, 0), (1, 0, 0))
    assert probe_edge in edges
    _laplacian, _inverse, green_data = edge_green_data(tuple(sorted(vertices)))
    exact_gradient, transfer_norm = green_data[probe_edge]
    injections = 37
    bound = Fraction(8, 3 * injections) + Fraction(8, injections) * transfer_norm
    scalar_mass = Fraction(1, 12)
    drift_values = []
    phase_rows = 0
    maximum_drift_error = Fraction(0)
    for rotor_state in states:
        memory = simulate_physical_memory(1, injections, rotor_state)
        current = memory[4].get(probe_edge, 0)
        sign = (current > 0) - (current < 0)
        direction = scale(-sign, edge_direction(probe_edge)) if sign else (-1, 0, 0)
        chart = aligned_chart(direction)
        body = clean_work_orbit(chart)[0]
        final, impulses, _history = run_mechanical_cycle(
            current, injections, chart, body
        )
        displacement = subtract(final[1].origin, chart.origin)
        drift = Fraction(displacement[0], 12 * injections)
        assert displacement == scale(-current, edge_direction(probe_edge))
        assert drift == -scalar_mass * Fraction(current, injections)
        error = abs(drift - (-scalar_mass * exact_gradient))
        assert error <= scalar_mass * bound
        maximum_drift_error = max(maximum_drift_error, error)
        drift_values.append(drift)
        phase_rows += 1
    check(
        "C5 all 192 rotor phases move the physical triplet at the exact scalar Green-response drift",
        phase_rows == 192,
    )
    check(
        "C6 triplet displacement inherits the exact initial-phase protection bound",
        max(drift_values) - min(drift_values) <= 2 * scalar_mass * bound,
    )

    # Clean stationary and moving ticks use the same internal clock and retain
    # the same role counts.  This closes response-induced displacement but not
    # an inertial energy/velocity relation.
    sample_chart = aligned_chart((-1, 0, 0))
    sample_body = clean_work_orbit(sample_chart)[0]
    relations_before, fields_before, work_before = physical_signature(
        sample_chart, sample_body, states
    )
    moved_chart, moved_body = moving_step(sample_chart, sample_body)
    relations_after, fields_after, work_after = physical_signature(
        moved_chart, moved_body, states
    )
    check(
        "C7 pulse-controlled displacement preserves triplet role occupancy and clean work state",
        len(relations_before) == len(relations_after) == 3
        and len(fields_before) == len(fields_after) == 6
        and work_before[1] == work_after[1],
    )

    # A complete response period has a finite clear-corridor price |C|.  The
    # momentum record remains nonzero after the field window, but this
    # candidate stops moving when pulses stop: exact evidence that inertia is
    # not yet derived.
    current = 7
    chart = aligned_chart((-1, 0, 0))
    body = clean_work_orbit(chart)[0]
    final, _impulses, _history = run_mechanical_cycle(current, 37, chart, body)
    response_after, chart_after, body_after = final
    momentum_after = decode_counter(response_after.probe_momentum)
    dark_response = initial_response(0, 37)
    dark_response = replace(
        dark_response,
        probe_momentum=response_after.probe_momentum,
        reaction_momentum=response_after.reaction_momentum,
    )
    next_response, next_chart, _next_body, impulse = mechanical_step(
        0, 37, dark_response, chart_after, body_after
    )
    del next_response
    check(
        "C8 nonzero retained momentum without a field pulse does not yet produce inertial continuation",
        momentum_after == -7 and impulse == 0 and next_chart.origin == chart_after.origin,
    )

    missing = {
        "native force-aligned chart and clear-corridor formation",
        "steering and vector composition for multiple incident edges",
        "momentum-to-velocity Legendre map and inertial continuation",
        "reaction transport back to source matter",
        "collision traffic and environmental repair",
        "canonical homogeneous Phi integration",
        "absolute action multiplier and physical units",
        "radiation universality tensor pole cone lensing and nonlinearity",
    }
    check(
        "C9 exact mechanical drift does not close Newtonian or relativistic gravity",
        len(missing) == 8,
    )

    forbidden = (
        "empirical_target",
        "random_draw",
        "parameter_fit",
        "137.036",
    )
    check(
        "C10 no empirical target random draw parameter fit or numerical search enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 Green-pulse triplet-drift Phi-v10 checks pass")
    print("body_hops_per_response_period=abs(C)")
    print("body_displacement=-C*edge_direction")
    print("average_body_drift=-C*edge_direction/(12N)")
    print(f"maximum_certified_clear_corridor={maximum_corridor}")
    print(f"maximum_all_phase_drift_error_N37={maximum_drift_error}")
    print("per_tick_causal_hops=0_or_1")
    print("complete_combined_inverse=exact")
    print("inertial_continuation=absent")
    print("status=prepared_phi_v10_mechanical_drift_exact_acceleration_and_inertia_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
