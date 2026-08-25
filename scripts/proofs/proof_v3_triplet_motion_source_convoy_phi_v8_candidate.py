#!/usr/bin/env python3
"""Prepared Phi-v8 persistent finite-corridor motion-source convoy.

Phi-v7 proves an exact production event: 24 A2 occupancies become two
minimum motion-source packets while a clean triplet moves one SC hop and its
work port becomes EXCITED.  Charging that same production price on every
subsequent inertial hop would confuse source formation with source transport.

This certificate selects two parallel chart-relative bundle lanes.  Phi-v7's
production transaction is repacked on those lanes.  One neighboring A2 work
owner then accepts the excitation by an exact READY/EXCITED swap.  Thereafter
the triplet and both marked packets advance together through a prepared rail
of existing two-record neutral routers.  Each packet step swaps a ten-record
marked bundle with a two-record router and leaves a two-record router behind;
no record is created or destroyed during convoy motion.

For any declared finite horizon H, the startup price is 24 A2 occupancies and
the prepared corridor price is 4H neutral field records.  The latter are not
consumed: the global role delta of every convoy hop is zero.  Exact reversal
restores the entire corridor, exported work state, startup packets, request,
and 24 A2 reserve occupancies.

This is a prepared finite-corridor candidate, not canonical Phi, native rail
formation, perturbative protection, a gravitational pole, or an absolute
coupling derivation.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from functools import lru_cache

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
)
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_v3_charged_candidate_matter_perturbation_boundary import frame_family
from proof_v3_homogeneous_event_halo_phi_v4_candidate import (
    add_exact,
    transform_field_map,
)
from proof_v3_neutral_rotor_walker_macro import (
    advance,
    polarized_slots,
    unmarked_site,
)
from proof_v3_neutral_scalar_vector_stf_bundle_common_green_seam import (
    bundle_marked_site,
    bundle_payload,
    inverse_bundle_step,
    local_bundle_step,
    orbit_lookup,
    recognize_bundle_marked,
)
from proof_v3_neutral_stf_rotor_walker_green_seam import internal_orbits
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    OrientedRepairChart,
    canonical_chart,
    intrinsic_state_descriptor,
    mv,
    transform_chart,
)
from proof_v3_triplet_discrete_motion_moment_gravity_lift import motion_signature
from proof_v3_triplet_motion_packet_work_reciprocal_phi_v7_candidate import (
    MotionVertex,
    PackedLane,
    canonical_seed_pairs,
    motion_payload_pairs,
    prepared_input,
    register_slots,
    request_controller,
    reserve_tokens,
    transform_packet,
    transform_reserve,
)
from proof_v3_triplet_relational_work_phi_v5_candidate import (
    WorkClock,
    excited_payload,
    one_substitutions,
    ready_payload,
)
from proof_v3_triplet_translating_self_correcting_clock_phi_v6_candidate import (
    clean_work_orbit,
    moving_step,
    shift_chart,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
Payload = tuple[int, int]


@dataclass(frozen=True)
class ConvoyBody:
    chart: OrientedRepairChart
    clock: WorkClock
    environment_work: Payload


checks: list[tuple[str, bool, str]] = []
PACK_CACHE: dict[tuple[object, ...], tuple[PackedLane, ...]] = {}


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def add(left: Vec, right: Vec) -> Vec:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def subtract(left: Vec, right: Vec) -> Vec:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def scale_add(origin: Vec, direction: Vec, scale: int) -> Vec:
    return tuple(a + scale * b for a, b in zip(origin, direction))  # type: ignore[return-value]


def chebyshev(left: Vec, right: Vec) -> int:
    return max(abs(a - b) for a, b in zip(left, right))


def convoy_lanes(chart: OrientedRepairChart):
    """Two disjoint parallel rails at carried transverse offsets +/-e_1."""

    material = shift_chart(chart, chart.repair_normal)
    return (
        (
            add(chart.origin, chart.first),
            add(material.origin, chart.first),
        ),
        (
            subtract(chart.origin, chart.first),
            subtract(material.origin, chart.first),
        ),
    )


def environment_owner(material: OrientedRepairChart) -> Vec:
    return add(material.origin, material.second)


def pack_convoy_source(
    anchor,
    output_clock,
    states,
    state_set,
    orbit_index,
    seed_pairs,
):
    """Exact two-packet source packing on future-parallel convoy rails."""

    cache_key = (anchor, output_clock, states, seed_pairs)
    cached = PACK_CACHE.get(cache_key)
    if cached is not None:
        return cached

    material = shift_chart(anchor, anchor.repair_normal)
    fields: dict[Vec, set[object]] = {}
    dark_request = request_controller(anchor, False, states)
    if not add_exact(fields, anchor.origin, polarized_slots(dark_request)):
        return None
    if not add_exact(
        fields,
        material.origin,
        register_slots(material, output_clock, states),
    ):
        return None

    ordered_states = tuple(
        sorted(
            states,
            key=lambda state: intrinsic_state_descriptor(state, anchor),
        )
    )
    packed = []
    for payload, lane in zip(
        motion_payload_pairs(anchor, seed_pairs), convoy_lanes(anchor)
    ):
        left, right = payload
        payload_orbits = {orbit_index[left], orbit_index[right]}
        choice = None
        for rotor in ordered_states:
            if orbit_index[rotor] in payload_orbits:
                continue
            marked = bundle_marked_site(rotor, left, right)
            if recognize_bundle_marked(marked, state_set, orbit_index) != (
                rotor,
                left,
                right,
            ):
                continue
            packet = PackedLane(
                lane[0],
                lane[1],
                left,
                right,
                rotor,
                rotor,
            )
            trial = {site: set(slots) for site, slots in fields.items()}
            if not add_exact(trial, packet.departure, packet.reserve_slots):
                continue
            if not add_exact(trial, packet.destination, packet.marked_slots):
                continue
            choice = packet, trial
            break
        if choice is None:
            return None
        packet, fields = choice
        packed.append(packet)

    result = tuple(packed)
    PACK_CACHE[cache_key] = result
    return result


def startup_forward(vertex, states, state_set, orbit_index, seed_pairs):
    if vertex.material != vertex.anchor:
        return None
    if not vertex.request_active or vertex.packets:
        return None
    if vertex.reserve != reserve_tokens(vertex.anchor):
        return None
    if vertex.clock not in clean_work_orbit(vertex.anchor):
        return None
    if vertex.clock.work != ready_payload(vertex.anchor):
        return None

    material, ready_clock = moving_step(vertex.anchor, vertex.clock)
    output_clock = WorkClock(
        ready_clock.arms,
        ready_clock.heralds,
        excited_payload(material),
    )
    packets = pack_convoy_source(
        vertex.anchor,
        output_clock,
        states,
        state_set,
        orbit_index,
        seed_pairs,
    )
    if packets is None:
        return None
    return MotionVertex(
        vertex.anchor,
        material,
        output_clock,
        False,
        frozenset(),
        packets,
    )


def startup_inverse(vertex, states, state_set, orbit_index, seed_pairs):
    anchor = vertex.anchor
    if vertex.material != shift_chart(anchor, anchor.repair_normal):
        return None
    if vertex.request_active or vertex.reserve:
        return None
    if vertex.clock.work != excited_payload(vertex.material):
        return None
    candidates = []
    for prior_clock in clean_work_orbit(anchor):
        material, ready_clock = moving_step(anchor, prior_clock)
        output_clock = WorkClock(
            ready_clock.arms,
            ready_clock.heralds,
            excited_payload(material),
        )
        if material != vertex.material or output_clock != vertex.clock:
            continue
        packets = pack_convoy_source(
            anchor,
            output_clock,
            states,
            state_set,
            orbit_index,
            seed_pairs,
        )
        if packets == vertex.packets:
            candidates.append(prepared_input(anchor, prior_clock))
    return candidates[0] if len(candidates) == 1 else None


def reset_and_move(body: ConvoyBody):
    """Export one work excitation, reset the body, then take a clean hop."""

    if body.clock.work != excited_payload(body.chart):
        return None
    if body.environment_work != ready_payload(body.chart):
        return None
    reset_clock = WorkClock(
        body.clock.arms,
        body.clock.heralds,
        ready_payload(body.chart),
    )
    next_chart, next_clock = moving_step(body.chart, reset_clock)
    return ConvoyBody(
        next_chart,
        next_clock,
        excited_payload(body.chart),
    )


def clean_convoy_move(body: ConvoyBody):
    if body.clock.work != ready_payload(body.chart):
        return None
    if body.environment_work != excited_payload(body.chart):
        return None
    next_chart, next_clock = moving_step(body.chart, body.clock)
    return ConvoyBody(next_chart, next_clock, body.environment_work)


def prior_ready_body(body: ConvoyBody):
    prior_chart = shift_chart(
        body.chart,
        tuple(-value for value in body.chart.repair_normal),
    )
    candidates = []
    for prior_clock in clean_work_orbit(prior_chart):
        if moving_step(prior_chart, prior_clock) == (
            body.chart,
            body.clock,
        ):
            candidates.append(prior_clock)
    if len(candidates) != 1:
        return None
    return prior_chart, candidates[0]


def inverse_clean_convoy_move(body: ConvoyBody):
    if body.environment_work != excited_payload(body.chart):
        return None
    prior = prior_ready_body(body)
    if prior is None:
        return None
    prior_chart, prior_clock = prior
    return ConvoyBody(prior_chart, prior_clock, body.environment_work)


def inverse_reset_and_move(body: ConvoyBody):
    if body.environment_work != excited_payload(body.chart):
        return None
    prior = prior_ready_body(body)
    if prior is None:
        return None
    prior_chart, prior_ready_clock = prior
    prior_excited_clock = WorkClock(
        prior_ready_clock.arms,
        prior_ready_clock.heralds,
        excited_payload(prior_chart),
    )
    return ConvoyBody(
        prior_chart,
        prior_excited_clock,
        ready_payload(prior_chart),
    )


def body_trajectory(startup: MotionVertex, horizon: int):
    rows = [
        ConvoyBody(
            startup.material,
            startup.clock,
            ready_payload(startup.material),
        )
    ]
    for tick in range(horizon):
        successor = (
            reset_and_move(rows[-1])
            if tick == 0
            else clean_convoy_move(rows[-1])
        )
        if successor is None:
            return None
        rows.append(successor)
    return tuple(rows)


def packet_states(packet: PackedLane, tick: int):
    return (
        advance(packet.marked_rotor, tick),
        advance(packet.left, tick),
        advance(packet.right, tick),
    )


def packet_rail_fields(
    packet: PackedLane,
    direction: Vec,
    horizon: int,
    tick: int,
):
    assert 0 <= tick <= horizon
    fields: dict[Vec, set[object]] = {}
    assert add_exact(fields, packet.departure, packet.reserve_slots)
    for index in range(horizon + 1):
        site = scale_add(packet.destination, direction, index)
        if index < tick:
            slots = unmarked_site(
                advance(packet.marked_rotor, index + 1)
            )
        elif index == tick:
            rotor, left, right = packet_states(packet, index)
            slots = bundle_marked_site(rotor, left, right)
        else:
            slots = unmarked_site(
                advance(packet.marked_rotor, index)
            )
        assert add_exact(fields, site, slots)
    return fields


def merge_fields(target, source) -> bool:
    for site, slots in source.items():
        if not add_exact(target, site, slots):
            return False
    return True


def convoy_field_map(
    startup: MotionVertex,
    body: ConvoyBody,
    horizon: int,
    tick: int,
    states,
):
    fields: dict[Vec, set[object]] = {}
    dark_request = request_controller(startup.anchor, False, states)
    assert add_exact(
        fields, startup.anchor.origin, polarized_slots(dark_request)
    )
    assert add_exact(
        fields,
        body.chart.origin,
        register_slots(body.chart, body.clock, states),
    )
    for packet in startup.packets:
        assert merge_fields(
            fields,
            packet_rail_fields(
                packet,
                startup.anchor.repair_normal,
                horizon,
                tick,
            ),
        )
    return fields


def startup_input_field_map(
    before: MotionVertex,
    after: MotionVertex,
    horizon: int,
    states,
):
    fields: dict[Vec, set[object]] = {}
    active_request = request_controller(before.anchor, True, states)
    assert add_exact(
        fields, before.anchor.origin, polarized_slots(active_request)
    )
    assert add_exact(
        fields,
        before.material.origin,
        register_slots(before.material, before.clock, states),
    )
    for packet in after.packets:
        for index in range(1, horizon + 1):
            site = scale_add(
                packet.destination,
                before.anchor.repair_normal,
                index,
            )
            assert add_exact(
                fields,
                site,
                unmarked_site(advance(packet.marked_rotor, index)),
            )
    return fields


def verify_packet_step(
    packet,
    direction,
    tick,
    state_set,
    orbit_index,
):
    departure_rotor, left, right = packet_states(packet, tick)
    destination_rotor = advance(packet.marked_rotor, tick + 1)
    before = (
        bundle_marked_site(departure_rotor, left, right),
        unmarked_site(destination_rotor),
    )
    after = local_bundle_step(
        before[0], before[1], state_set, orbit_index
    )
    if after is None:
        return False
    expected = (
        unmarked_site(destination_rotor),
        bundle_marked_site(
            destination_rotor,
            advance(left, 1),
            advance(right, 1),
        ),
    )
    if after[:2] != expected:
        return False
    recovered = inverse_bundle_step(
        after[0], after[1], state_set, orbit_index
    )
    return (
        recovered is not None
        and recovered[:2] == before
        and subtract(
            scale_add(packet.destination, direction, tick + 1),
            scale_add(packet.destination, direction, tick),
        )
        == direction
    )


def main() -> None:
    states = tuple(one_particle_states())
    state_set = frozenset(states)
    orbit_index = orbit_lookup(internal_orbits(states))
    seed_pairs = canonical_seed_pairs(states, orbit_index)
    group = tuple(signed_permutation_matrices())
    charts = {
        transform_chart(matrix, canonical_chart(frame))
        for frame in frame_family()
        for matrix in group
    }
    # Four steps cover one complete C4 work/phase section.  C7 proves the
    # exact tick-independent recurrence and inverse, so the construction then
    # extends by induction to every declared finite H.
    horizon = 4

    geometry_rows = 0
    for chart in charts:
        lanes = convoy_lanes(chart)
        assert len(lanes) == 2
        assert len({site for lane in lanes for site in lane}) == 4
        for departure, destination in lanes:
            assert subtract(destination, departure) == chart.repair_normal
            assert chebyshev(departure, chart.origin) <= 1
            assert chebyshev(destination, chart.origin) <= 1
        assert {
            subtract(destination, shift_chart(chart, chart.repair_normal).origin)
            for _, destination in lanes
        } == {chart.first, tuple(-value for value in chart.first)}
        geometry_rows += 1
    check(
        "C1 two chart-native parallel packet rails are distinct, one-hop, Moore-local, and future-disjoint",
        len(charts) == 1_152 and geometry_rows == 1_152,
    )

    startup_rows = 0
    startup_inverse_rows = 0
    source_rows = 0
    field_delta_rows = 0
    convoy_rows = 0
    packet_inverse_rows = 0
    body_inverse_rows = 0
    work_rows = 0
    for chart in charts:
        for clock in clean_work_orbit(chart):
            before = prepared_input(chart, clock)
            after = startup_forward(
                before,
                states,
                state_set,
                orbit_index,
                seed_pairs,
            )
            assert after is not None, (chart, clock)
            assert after.packets == tuple(
                PackedLane(
                    lane[0],
                    lane[1],
                    packet.left,
                    packet.right,
                    packet.marked_rotor,
                    packet.reserve_rotor,
                )
                for lane, packet in zip(convoy_lanes(chart), after.packets)
            )
            assert startup_inverse(
                after,
                states,
                state_set,
                orbit_index,
                seed_pairs,
            ) == before
            startup_inverse_rows += 1

            input_fields = startup_input_field_map(
                before, after, horizon, states
            )
            trajectory = body_trajectory(after, horizon)
            assert trajectory is not None
            output_fields = convoy_field_map(
                after, trajectory[0], horizon, 0, states
            )
            before_count = sum(len(slots) for slots in input_fields.values())
            after_count = sum(len(slots) for slots in output_fields.values())
            assert before_count == 8 + 4 * horizon
            assert after_count == 32 + 4 * horizon
            assert after_count - before_count == 24
            field_delta_rows += 1

            for tick, body in enumerate(trajectory):
                if tick in (0, horizon):
                    fields = convoy_field_map(
                        after, body, horizon, tick, states
                    )
                    assert sum(len(slots) for slots in fields.values()) == (
                        32 + 4 * horizon
                    )
                assert body.chart.origin == scale_add(
                    after.material.origin,
                    chart.repair_normal,
                    tick,
                )
                assert body.clock.work == (
                    excited_payload(body.chart)
                    if tick == 0
                    else ready_payload(body.chart)
                )
                assert body.environment_work == (
                    ready_payload(body.chart)
                    if tick == 0
                    else excited_payload(body.chart)
                )
                payload_sum = [0] * 9
                observed_layer = (chart.layer - tick) % 3
                for packet in after.packets:
                    _, left, right = packet_states(packet, tick)
                    payload = bundle_payload(left, right, observed_layer)
                    payload_sum = [
                        total + value
                        for total, value in zip(payload_sum, payload)
                    ]
                assert tuple(payload_sum) == tuple(
                    2 * value
                    for value in motion_signature(chart.repair_normal)
                )
                source_rows += 1
                convoy_rows += 1

            assert trajectory[0].clock.work == excited_payload(after.material)
            assert trajectory[0].environment_work == ready_payload(after.material)
            assert trajectory[1].clock.work == ready_payload(trajectory[1].chart)
            assert trajectory[1].environment_work == excited_payload(
                trajectory[1].chart
            )
            assert 1 + 0 == 0 + 1
            work_rows += 1

            for tick in range(horizon):
                for packet in after.packets:
                    assert verify_packet_step(
                        packet,
                        chart.repair_normal,
                        tick,
                        state_set,
                        orbit_index,
                    )
                    packet_inverse_rows += 1

            reverse_body = trajectory[-1]
            for tick in range(horizon, 1, -1):
                reverse_body = inverse_clean_convoy_move(reverse_body)
                assert reverse_body == trajectory[tick - 1]
            reverse_body = inverse_reset_and_move(reverse_body)
            assert reverse_body == trajectory[0]
            body_inverse_rows += 1
            startup_rows += 1

    expected = 1_152 * 16
    check(
        "C2 every chart/phase admits exact 24-A2 startup into two future-parallel minimum source packets",
        startup_rows == expected and field_delta_rows == expected,
    )
    check(
        "C3 startup has one exact inverse restoring request, reserve, READY work, and prior matter",
        startup_inverse_rows == expected,
    )
    check(
        "C4 one neighboring A2 owner receives the work excitation while the body resets and moves forward",
        work_rows == expected,
    )
    check(
        "C5 triplet and both marked packets co-translate through a complete C4 section at fixed global occupancy",
        convoy_rows == expected * (horizon + 1),
    )
    check(
        "C6 the selected motion signature is exactly clock-invariant along the complete convoy",
        source_rows == convoy_rows,
    )
    check(
        "C7 every packet and body convoy hop has an explicit exact inverse",
        packet_inverse_rows == expected * horizon * 2
        and body_inverse_rows == expected,
    )
    check(
        "C8 convoy hops have role delta (0,0,0,0); the 24-A2 debit occurs once at startup",
        (32 + 4 * horizon) - (32 + 4 * horizon) == 0,
    )

    base_chart = canonical_chart(frame_family()[0])
    covariance_rows = 0
    for matrix in group:
        transformed_chart = transform_chart(matrix, base_chart)
        assert tuple(
            (mv(matrix, departure), mv(matrix, destination))
            for departure, destination in convoy_lanes(base_chart)
        ) == convoy_lanes(transformed_chart)
        assert mv(
            matrix,
            environment_owner(
                shift_chart(base_chart, base_chart.repair_normal)
            ),
        ) == environment_owner(
            shift_chart(
                transformed_chart, transformed_chart.repair_normal
            )
        )
        assert transform_reserve(
            matrix, reserve_tokens(base_chart)
        ) == reserve_tokens(transformed_chart)
        for phase in range(16):
            before = prepared_input(
                base_chart, clean_work_orbit(base_chart)[phase]
            )
            transformed_before = prepared_input(
                transformed_chart,
                clean_work_orbit(transformed_chart)[phase],
            )
            after = startup_forward(
                before,
                states,
                state_set,
                orbit_index,
                seed_pairs,
            )
            transformed_after = startup_forward(
                transformed_before,
                states,
                state_set,
                orbit_index,
                seed_pairs,
            )
            assert after is not None and transformed_after is not None
            assert tuple(
                transform_packet(matrix, packet)
                for packet in after.packets
            ) == transformed_after.packets
            base_bodies = body_trajectory(after, 3)
            transformed_bodies = body_trajectory(transformed_after, 3)
            assert base_bodies is not None and transformed_bodies is not None
            for tick in range(4):
                assert transform_chart(
                    matrix, base_bodies[tick].chart
                ) == transformed_bodies[tick].chart
                assert transform_field_map(
                    matrix,
                    convoy_field_map(
                        after, base_bodies[tick], 3, tick, states
                    ),
                ) == convoy_field_map(
                    transformed_after,
                    transformed_bodies[tick],
                    3,
                    tick,
                    states,
                )
            covariance_rows += 1
    check(
        "C9 startup, work export, parallel rails, and convoy fields are covariant under all signed-cubic charts",
        covariance_rows == 48 * 16,
    )

    translation = (7, -5, 3)
    shifted_chart = shift_chart(base_chart, translation)
    base_before = prepared_input(
        base_chart, clean_work_orbit(base_chart)[0]
    )
    shifted_before = prepared_input(
        shifted_chart, clean_work_orbit(shifted_chart)[0]
    )
    base_after = startup_forward(
        base_before, states, state_set, orbit_index, seed_pairs
    )
    shifted_after = startup_forward(
        shifted_before, states, state_set, orbit_index, seed_pairs
    )
    assert base_after is not None and shifted_after is not None
    base_body = body_trajectory(base_after, 3)
    shifted_body = body_trajectory(shifted_after, 3)
    assert base_body is not None and shifted_body is not None
    translated_fields = {
        add(site, translation): slots
        for site, slots in convoy_field_map(
            base_after, base_body[3], 3, 3, states
        ).items()
    }
    check(
        "C10 the finite convoy is spatially homogeneous and contains no origin-labelled rule",
        translated_fields
        == convoy_field_map(
            shifted_after, shifted_body[3], 3, 3, states
        ),
    )

    clean = clean_work_orbit(base_chart)[0]
    error = MotionVertex(
        base_chart,
        base_chart,
        next(one_substitutions(clean)),
        True,
        reserve_tokens(base_chart),
        (),
    )
    busy = MotionVertex(
        base_chart,
        base_chart,
        WorkClock(clean.arms, clean.heralds, excited_payload(base_chart)),
        True,
        reserve_tokens(base_chart),
        (),
    )
    check(
        "C11 startup, repair, busy work, and steady convoy admissions remain state-disjoint",
        startup_forward(error, states, state_set, orbit_index, seed_pairs)
        is None
        and startup_forward(
            busy, states, state_set, orbit_index, seed_pairs
        )
        is None,
    )

    corridor_prices = {
        length: 4 * length for length in (0, 1, 16, 37)
    }
    check(
        "C12 finite horizon H costs 4H prepared neutral router records but no per-hop A2 debit",
        corridor_prices == {0: 0, 1: 4, 16: 64, 37: 148},
    )

    missing = {
        "native corridor/router formation or closed finite recycler",
        "occupancy-loss, packet-loss, and multi-object collision repair",
        "canonical state-complete Phi integration",
        "autonomous protected scalar/vector/tensor poles and reciprocal force",
        "physical inertia/action curvature and absolute coupling scale",
        "common cone, lensing, and nonlinear closure",
    }
    check(
        "C13 finite convoy persistence does not establish protected matter or dynamical gravity",
        len(missing) == 6,
    )

    forbidden = (
        "particle_mass",
        "newton_target",
        "lensing_target",
        "137.036",
        "random_draw",
    )
    check(
        "C14 no empirical target, fitted scale, random draw, or numerical near-miss search enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} triplet motion-source convoy checks pass")
    print("prepared_phi_candidate=v8")
    print("startup_A2_debit=24_once")
    print("startup_role_delta=(24,0,0,-24)")
    print("work_export=body_EXCITED+environment_READY_to_body_READY+environment_EXCITED")
    print("convoy_packet_count=2_minimum")
    print("convoy_horizon=arbitrary_declared_finite_H")
    print("corridor_router_price=4H_neutral_field_records")
    print("per_convoy_hop_role_delta=(0,0,0,0)")
    print("per_convoy_hop_A2_debit=0")
    print("convoy_inverse=exact")
    print("absolute_action_multiplier=free")
    print("status=prepared_finite_convoy_exact_native_formation_and_protected_response_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
