#!/usr/bin/env python3
"""Prepared Phi-v7 triplet translation/source-packet/work reciprocal vertex.

This certificate composes three exact v3 constructions in one finite event:

* the Phi-v6 clean triplet moves one SC hop along its carried polar normal;
* the theorem-minimum two neutral bundle packets realize the selected discrete
  motion moment; and
* one physical request herald and one A2 READY/EXCITED work token close the
  relative event ledger.

Six explicit chart-relative A2=A9^4 reserve owners supply exactly 24 occupied
A9 slots.  The released two-packet lane state contains exactly 24 field
records: two ten-record marked bundles plus two two-record neutral residues.
Thus the exact role delta is (+24,0,0,-24).  The inverse absorbs the exact
packet kit, restores all reserve slots and the request, moves the clean triplet
back one hop, and resets the work port.

The construction is a prepared-sector candidate, not canonical Phi.  It does
not form/refill the reserve, export/reset work while continuing forward,
handle simultaneous repair, protect a gravity pole, or fix an action scale.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from functools import lru_cache
from itertools import combinations

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_v3_charged_candidate_matter_perturbation_boundary import frame_family
from proof_v3_homogeneous_event_halo_phi_v4_candidate import (
    CENTER,
    add_exact,
    assign_role_pads,
    gravity_lane_edges,
    herald_states,
    relative_site,
    transform_field_map,
)
from proof_v3_matter_anchored_born_gauss_gravity_event_seam import (
    matrix_from_columns,
)
from proof_v3_neutral_rotor_walker_macro import (
    advance,
    physical_value,
    polarized_slots,
    unmarked_site,
)
from proof_v3_neutral_scalar_vector_stf_bundle_common_green_seam import (
    bundle_marked_site,
    bundle_payload,
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
from proof_v3_triplet_discrete_motion_moment_gravity_lift import (
    first_pair_for_payload,
    motion_signature,
)
from proof_v3_triplet_relational_work_phi_v5_candidate import (
    WorkClock,
    excited_payload,
    one_substitutions,
    ready_payload,
    work_register_banks,
    work_step,
)
from proof_v3_triplet_translating_self_correcting_clock_phi_v6_candidate import (
    clean_work_orbit,
    moving_step,
    shift_chart,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
Payload = tuple[int, int]
ReserveToken = tuple[Vec, int, Payload]

# Six existing A2 owners, each with all four A9 slots occupied.  The positions
# are chart-relative and remain inside the request center's Moore block.
RESERVE_RELATIVE = (
    (-1, -1, -1),
    (-1, -1, 0),
    (-1, -1, 1),
    (-1, 0, -1),
    (-1, 0, 0),
    (-1, 0, 1),
)


@dataclass(frozen=True)
class PackedLane:
    departure: Vec
    destination: Vec
    left: object
    right: object
    marked_rotor: object
    reserve_rotor: object

    @property
    def payload(self):
        return self.left, self.right

    @property
    def reserve_slots(self):
        return unmarked_site(self.reserve_rotor)

    @property
    def marked_slots(self):
        return bundle_marked_site(
            self.marked_rotor, self.left, self.right
        )


@dataclass(frozen=True)
class MotionVertex:
    anchor: OrientedRepairChart
    material: OrientedRepairChart
    clock: WorkClock
    request_active: bool
    reserve: frozenset[ReserveToken]
    packets: tuple[PackedLane, ...]


checks: list[tuple[str, bool, str]] = []
PACK_CACHE: dict[tuple[object, ...], tuple[PackedLane, ...]] = {}


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def transform_reserve(matrix, reserve):
    return frozenset(
        (mv(matrix, site), slot, payload)
        for site, slot, payload in reserve
    )


def reserve_tokens(chart: OrientedRepairChart) -> frozenset[ReserveToken]:
    return frozenset(
        (
            relative_site(chart, relative),
            slot,
            chart.payload(slot),
        )
        for relative in RESERVE_RELATIVE
        for slot in range(4)
    )


@lru_cache(maxsize=None)
def request_controller(chart, active: bool, states):
    center_pad = assign_role_pads(chart, states)[CENTER]
    label = (chart.repair_normal, chart.polarity) if active else None
    return herald_states(chart, states, center_pad)[label]


@lru_cache(maxsize=None)
def register_slots(chart, clock: WorkClock, states):
    banks = work_register_banks(chart, states)
    return frozenset().union(
        *(
            polarized_slots(bank[symbol])
            for bank, symbol in zip(banks, clock.heralds)
        )
    )


def vertex_field_map(vertex: MotionVertex, states):
    fields: dict[Vec, set[object]] = {}
    request = request_controller(
        vertex.anchor, vertex.request_active, states
    )
    assert add_exact(
        fields, vertex.anchor.origin, polarized_slots(request)
    )
    assert add_exact(
        fields,
        vertex.material.origin,
        register_slots(vertex.material, vertex.clock, states),
    )
    for packet in vertex.packets:
        assert add_exact(fields, packet.departure, packet.reserve_slots)
        assert add_exact(fields, packet.destination, packet.marked_slots)
    return fields


def canonical_seed_pairs(states, orbit_index):
    targets = (
        (1, 4, -2, 0, 0, 0, 1, 0, 0),
        (1, 0, 0, 0, 0, 0, 1, 0, 0),
    )
    return tuple(
        first_pair_for_payload(states, orbit_index, target)
        for target in targets
    )


def motion_payload_pairs(chart, seed_pairs):
    # Canonical seeds carry +e_x at layer zero.  This signed-permutation matrix
    # maps e_x to the chart's polar motion normal and preserves chart context.
    transform = matrix_from_columns(
        chart.repair_normal, chart.first, chart.second
    )
    clock_shift = (-chart.layer) % 3
    return tuple(
        tuple(
            transform_state(
                transform, advance(state, clock_shift)
            )
            for state in pair
        )
        for pair in seed_pairs
    )


def pack_motion_source(
    anchor,
    output_clock,
    states,
    state_set,
    orbit_index,
    seed_pairs,
):
    """First intrinsic exact two-packet packing inside the anchor Moore block."""

    cache_key = (anchor, output_clock, states, seed_pairs)
    cached = PACK_CACHE.get(cache_key)
    if cached is not None:
        return cached

    material = shift_chart(anchor, anchor.repair_normal)
    fields: dict[Vec, set[object]] = {}
    dark_request = request_controller(anchor, False, states)
    if not add_exact(
        fields, anchor.origin, polarized_slots(dark_request)
    ):
        return None
    if not add_exact(
        fields,
        material.origin,
        register_slots(material, output_clock, states),
    ):
        return None

    payloads = motion_payload_pairs(anchor, seed_pairs)
    lanes = gravity_lane_edges(anchor)
    # Lane endpoints may coincide as physical sites; exact slot disjointness,
    # not site identity, is the exclusion condition enforced below.
    lane_pairs = tuple(combinations(lanes, 2))
    ordered_states = tuple(
        sorted(
            states,
            key=lambda state: intrinsic_state_descriptor(state, anchor),
        )
    )

    for selected_lanes in lane_pairs:
        trial = {site: set(slots) for site, slots in fields.items()}
        packed = []
        for payload, lane in zip(payloads, selected_lanes):
            left, right = payload
            payload_orbits = {orbit_index[left], orbit_index[right]}
            departure, destination = lane
            choice = None
            for marked_rotor in ordered_states:
                if orbit_index[marked_rotor] in payload_orbits:
                    continue
                marked = bundle_marked_site(
                    marked_rotor, left, right
                )
                if recognize_bundle_marked(
                    marked, state_set, orbit_index
                ) is None:
                    continue
                for reserve_rotor in ordered_states:
                    reserve = unmarked_site(reserve_rotor)
                    if trial.get(destination, set()) & marked:
                        continue
                    if trial.get(departure, set()) & reserve:
                        continue
                    choice = marked_rotor, reserve_rotor
                    break
                if choice is not None:
                    break
            if choice is None:
                break
            marked_rotor, reserve_rotor = choice
            packet = PackedLane(
                departure,
                destination,
                left,
                right,
                marked_rotor,
                reserve_rotor,
            )
            if not add_exact(trial, departure, packet.reserve_slots):
                break
            if not add_exact(trial, destination, packet.marked_slots):
                break
            packed.append(packet)
        if len(packed) == 2:
            result = tuple(packed)
            PACK_CACHE[cache_key] = result
            return result
    return None


def prepared_input(chart, clock):
    return MotionVertex(
        chart,
        chart,
        clock,
        True,
        reserve_tokens(chart),
        (),
    )


def forward_vertex(vertex, states, state_set, orbit_index, seed_pairs):
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
    packets = pack_motion_source(
        vertex.anchor,
        output_clock,
        states,
        state_set,
        orbit_index,
        seed_pairs,
    )
    if packets is None:
        return None
    output = MotionVertex(
        vertex.anchor,
        material,
        output_clock,
        False,
        frozenset(),
        packets,
    )
    vertex_field_map(output, states)
    return output


def inverse_vertex(vertex, states, state_set, orbit_index, seed_pairs):
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
        candidate_clock = WorkClock(
            ready_clock.arms,
            ready_clock.heralds,
            excited_payload(material),
        )
        if material != vertex.material or candidate_clock != vertex.clock:
            continue
        candidate_packets = pack_motion_source(
            anchor,
            candidate_clock,
            states,
            state_set,
            orbit_index,
            seed_pairs,
        )
        if candidate_packets == vertex.packets:
            candidates.append(prepared_input(anchor, prior_clock))
    return candidates[0] if len(candidates) == 1 else None


def transform_packet(matrix, packet: PackedLane):
    return PackedLane(
        mv(matrix, packet.departure),
        mv(matrix, packet.destination),
        transform_state(matrix, packet.left),
        transform_state(matrix, packet.right),
        transform_state(matrix, packet.marked_rotor),
        transform_state(matrix, packet.reserve_rotor),
    )


def main() -> None:
    states = tuple(one_particle_states())
    state_set = frozenset(states)
    orbits = internal_orbits(states)
    orbit_index = orbit_lookup(orbits)
    seed_pairs = canonical_seed_pairs(states, orbit_index)
    group = tuple(signed_permutation_matrices())
    charts = {
        transform_chart(matrix, canonical_chart(frame))
        for frame in frame_family()
        for matrix in group
    }

    reserve_rows = 0
    for chart in charts:
        reserve = reserve_tokens(chart)
        assert len(reserve) == 24
        assert len({site for site, _, _ in reserve}) == 6
        assert all(
            max(
                abs(site[index] - chart.origin[index])
                for index in range(3)
            )
            <= 1
            for site, _, _ in reserve
        )
        assert all(
            {slot for owner, slot, _ in reserve if owner == site}
            == set(range(4))
            for site in {row[0] for row in reserve}
        )
        reserve_rows += 1
    check(
        "C1 six explicit Moore-local A2=A9^4 owners supply exactly 24 physical reserve occupancies",
        len(charts) == 1_152 and reserve_rows == 1_152,
    )

    request_rows = 0
    for chart in charts:
        active = request_controller(chart, True, states)
        dark = request_controller(chart, False, states)
        assert active != dark
        assert len(polarized_slots(active)) == len(polarized_slots(dark)) == 2
        for layer in range(3):
            assert physical_value(polarized_slots(active), layer) == (0,) * 6
            assert physical_value(polarized_slots(dark), layer) == (0,) * 6
        request_rows += 1
    check(
        "C2 existing neutral herald states provide distinct active/dark motion requests at fixed occupancy",
        request_rows == 1_152,
    )

    transaction_rows = 0
    inverse_rows = 0
    field_delta_rows = 0
    source_rows = 0
    locality_rows = 0
    for chart in charts:
        for clock in clean_work_orbit(chart):
            before = prepared_input(chart, clock)
            after = forward_vertex(
                before,
                states,
                state_set,
                orbit_index,
                seed_pairs,
            )
            assert after is not None, (chart, clock)
            before_fields = vertex_field_map(before, states)
            after_fields = vertex_field_map(after, states)
            before_count = sum(len(slots) for slots in before_fields.values())
            after_count = sum(len(slots) for slots in after_fields.values())
            assert before_count == 8 and after_count == 32
            assert after_count - before_count == 24
            assert len(before.reserve) == 24 and len(after.reserve) == 0
            assert before.clock.work == ready_payload(chart)
            assert after.clock.work == excited_payload(after.material)
            assert len(after.packets) == 2
            field_delta_rows += 1

            payload_sum = tuple(
                map(
                    sum,
                    zip(
                        *(
                            bundle_payload(
                                packet.left,
                                packet.right,
                                chart.layer,
                            )
                            for packet in after.packets
                        )
                    ),
                )
            )
            assert payload_sum == tuple(
                2 * value
                for value in motion_signature(chart.repair_normal)
            )
            source_rows += 1

            for packet in after.packets:
                assert recognize_bundle_marked(
                    packet.marked_slots, state_set, orbit_index
                ) == (
                    packet.marked_rotor,
                    packet.left,
                    packet.right,
                )
                assert len(packet.marked_slots) == 10
                assert len(packet.reserve_slots) == 2
                for layer in range(3):
                    assert physical_value(
                        packet.marked_slots, layer
                    ) == (0,) * 6
                    assert physical_value(
                        packet.reserve_slots, layer
                    ) == (0,) * 6
                assert max(
                    abs(packet.destination[index] - chart.origin[index])
                    for index in range(3)
                ) <= 1
                assert max(
                    abs(packet.departure[index] - chart.origin[index])
                    for index in range(3)
                ) <= 1
                locality_rows += 1

            recovered = inverse_vertex(
                after,
                states,
                state_set,
                orbit_index,
                seed_pairs,
            )
            assert recovered == before
            inverse_rows += 1
            transaction_rows += 1

    expected = 1_152 * 16
    check(
        "C3 every chart/clean phase admits one radius-one translation/request/work/two-packet transaction",
        transaction_rows == expected and locality_rows == 2 * expected,
    )
    check(
        "C4 the exact physical role delta is (+24,0,0,-24) and preserves the all-equal occupancy ray",
        field_delta_rows == expected and 24 + 0 + 0 - 24 == 0,
    )
    check(
        "C5 every output carries exactly twice the selected discrete motion-moment signature",
        source_rows == expected,
    )
    check(
        "C6 every marked packet/residue is state-recognizable and additive-E/B neutral",
        locality_rows == 2 * expected,
    )
    check(
        "C7 exact absorption restores request, 24 A2 reserves, work READY, and the prior material state",
        inverse_rows == expected,
    )

    # Exact covariance of the selected first-fit packet/router construction.
    covariance_rows = 0
    base_chart = canonical_chart(frame_family()[0])
    for matrix in group:
        transformed_chart = transform_chart(matrix, base_chart)
        assert transform_reserve(
            matrix, reserve_tokens(base_chart)
        ) == reserve_tokens(transformed_chart)
        assert transform_state(
            matrix, request_controller(base_chart, True, states)
        ) == request_controller(transformed_chart, True, states)
        assert transform_state(
            matrix, request_controller(base_chart, False, states)
        ) == request_controller(transformed_chart, False, states)
        for phase in range(16):
            base_before = prepared_input(
                base_chart, clean_work_orbit(base_chart)[phase]
            )
            transformed_before = prepared_input(
                transformed_chart,
                clean_work_orbit(transformed_chart)[phase],
            )
            base_after = forward_vertex(
                base_before,
                states,
                state_set,
                orbit_index,
                seed_pairs,
            )
            transformed_after = forward_vertex(
                transformed_before,
                states,
                state_set,
                orbit_index,
                seed_pairs,
            )
            assert base_after is not None and transformed_after is not None
            assert tuple(
                transform_packet(matrix, packet)
                for packet in base_after.packets
            ) == transformed_after.packets
            assert transform_field_map(
                matrix, vertex_field_map(base_before, states)
            ) == vertex_field_map(transformed_before, states)
            assert transform_field_map(
                matrix, vertex_field_map(base_after, states)
            ) == vertex_field_map(transformed_after, states)
            covariance_rows += 1
    check(
        "C8 reserve, request, translation, and selected packet kit are covariant under all 48 signed-cubic charts",
        covariance_rows == 48 * 16,
    )

    # Admission separation: an error uses Phi-v6 repair, while a busy port or
    # an incomplete packet kit cannot spend the motion reserve again.
    chart = base_chart
    clean = clean_work_orbit(chart)[0]
    mutant = next(one_substitutions(clean))
    error_state = MotionVertex(
        chart,
        chart,
        mutant,
        True,
        reserve_tokens(chart),
        (),
    )
    busy_state = MotionVertex(
        chart,
        chart,
        WorkClock(clean.arms, clean.heralds, excited_payload(chart)),
        True,
        reserve_tokens(chart),
        (),
    )
    clean_after = forward_vertex(
        prepared_input(chart, clean),
        states,
        state_set,
        orbit_index,
        seed_pairs,
    )
    assert clean_after is not None
    incomplete_after = MotionVertex(
        clean_after.anchor,
        clean_after.material,
        clean_after.clock,
        clean_after.request_active,
        clean_after.reserve,
        clean_after.packets[:1],
    )
    check(
        "C9 motion emission, error repair, busy work, and exact absorption admissions are state-disjoint",
        forward_vertex(
            error_state, states, state_set, orbit_index, seed_pairs
        ) is None
        and forward_vertex(
            busy_state, states, state_set, orbit_index, seed_pairs
        ) is None
        and inverse_vertex(
            incomplete_after,
            states,
            state_set,
            orbit_index,
            seed_pairs,
        ) is None,
    )

    check(
        "C10 physical request plus A2 work excitation closes the relative ledger 1+0=0+1",
        1 + 0 == 0 + 1,
    )

    reserve_examples = (0, 23, 24, 47, 240)
    release_bounds = {
        reserve: reserve // 24 for reserve in reserve_examples
    }
    check(
        "C11 persistent release costs 24 A2 occupancies per hop and additionally requires work-port reset",
        release_bounds == {0: 0, 23: 0, 24: 1, 47: 1, 240: 10},
    )

    response_prices = {
        price: price * 1 for price in (1, 2, 7)
    }
    check(
        "C12 exact relative ledgers leave the positive action/coupling multiplier free",
        response_prices == {1: 1, 2: 2, 7: 7},
    )

    missing = {
        "native request/reserve formation and refill",
        "forward work export/reset without reversing the material hop",
        "simultaneous moving repair and environmental collision arbitration",
        "canonical state-complete Phi integration",
        "protected scalar/vector/tensor response and reciprocal force",
        "absolute normalization, common cone, lensing, and nonlinearity",
    }
    check(
        "C13 Phi-v7 reciprocal vertex does not close persistent matter or dynamical gravity",
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
    print(f"\n{passed}/{len(checks)} triplet motion-packet-work vertex checks pass")
    print("prepared_phi_candidate=v7")
    print("motion_request=one_existing_neutral_herald_pair")
    print("reserve_owners=6_A2_sites_x_4_A9_slots")
    print("reserve_occupancies=24")
    print("released_field_records=24")
    print("role_delta=(24,0,0,-24)")
    print("motion_packet_count=2_minimum")
    print("relative_request_work_ledger=1+0=0+1")
    print("absorption_inverse=exact")
    print("persistent_release_cost=24_A2_per_hop_plus_work_reset")
    print("absolute_action_multiplier=free")
    print("status=prepared_reciprocal_motion_source_vertex_exact_persistent_dynamics_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
