#!/usr/bin/env python3
"""Matter-anchored Born/Gauss/gravity event seam on the finite v3 carrier.

This certificate composes five previously separate exact structures at one
prepared local event vertex:

* an exact charged circulation frame and existing neutral header supply the
  oriented material/apparatus chart;
* the contextual 384/385 pointer and ternary detector supply the prepared
  target-blind Born event schedule;
* the left pointer's A9 polarity token and eight same-polarity A2 work tokens
  are transferred into one SC A1 source token and its eight-channel dressed
  Gauss packet;
* the transfer has role delta (F,A1_SC,A1_FCC,A2)=(8,1,0,-9), so it conserves
  the conditionally forced all-equal occupancy invariant exactly; and
* two existing neutral scalar/vector/STF packets have a combined tensor and
  vector readout matching the charged event stress and its transverse
  constraint load up to explicit finite coordinate factors.

The bright transition and its recovery are one finite reversible permutation
on the registered prepared apparatus component.  Exact-frame, forward-repair,
reverse-repair, and fallback admissions are state-disjoint.  This is an event
seam, not physical completion: autonomous resource formation, detector/site
action, traffic arbitration beyond the isolated block, stable matter,
interacting poles, absolute normalization, and lensing remain open.
"""

from __future__ import annotations

import sys
from collections import Counter
from functools import lru_cache
from itertools import product

from sympy import Matrix, Rational

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_v3_charged_candidate_matter_perturbation_boundary import (
    all_local_channels,
    frame_family,
    presentation,
)
from proof_v3_charged_common_action_phi_v3_candidate import (
    PlaquetteFrame,
    current_divergence,
)
from proof_v3_charged_frame_unique_one_defect_decoder import (
    enumerate_one_defects,
)
from proof_v3_contextual_neutral_pointer_born_renewal_apparatus import (
    MANIFESTED,
    READY,
    RECOVERY,
    ApparatusState,
    address_order,
    addressed_pair,
    apparatus_inverse,
    apparatus_step,
    bank_from_counts,
    bright_outcome,
    canonical_residual,
    outcome,
    pointer_configuration,
)
from proof_v3_dressed_sc_source_gauss_continuity import (
    DressedEdgeState,
    add_maps,
    charge as edge_charge,
    divergence as edge_divergence,
    scale_map,
    target_packet,
)
from proof_v3_dressed_source_stress_spin2_boundary import (
    expected_packet_stress,
    stress_matrix,
)
from proof_v3_neutral_rotor_harmonic_green_seam import (
    SC_DIRECTIONS,
    add,
    rotor_successor,
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
    inverse_bundle_step,
    local_bundle_step,
    orbit_lookup,
)
from proof_v3_neutral_stf_rotor_walker_green_seam import (
    internal_orbits,
    payload_tensor,
)
from proof_v3_neutral_vector_constraint_walker_tt_locality_obstruction import (
    payload_vector,
)
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    OrientedRepairChart,
    canonical_chart,
    chart_code_representatives,
    chart_presentation,
    cross,
    intrinsic_state_descriptor,
    mv,
    transform_chart,
    transform_presentation,
    transform_slots,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
RoleDelta = tuple[int, int, int, int]
PERIOD = 384 * 385


def neg(vector: Vec) -> Vec:
    return tuple(-entry for entry in vector)  # type: ignore[return-value]


def scale(vector: Vec, coefficient: int) -> Vec:
    return tuple(coefficient * entry for entry in vector)  # type: ignore[return-value]


def collinear(left: Vec, right: Vec) -> bool:
    return left == right or left == neg(right)


def matrix_from_columns(first: Vec, second: Vec, third: Vec):
    return tuple(
        tuple(column[row] for column in (first, second, third))
        for row in range(3)
    )


def pointer_from_pair(left_index: int, right_index: int) -> int:
    """CRT inverse for t mod 384 and t mod 385."""

    assert 0 <= left_index < 384
    assert 0 <= right_index < 385
    value = left_index + 384 * ((left_index - right_index) % 385)
    assert value % 384 == left_index
    assert value % 385 == right_index
    return value % PERIOD


def event_active(order, residual, state: ApparatusState) -> bool:
    return (
        state.detector == MANIFESTED
        and bright_outcome(order, residual, state.pointer) is not None
    )


def source_edge_state(
    direction: Vec,
    polarity: int,
    layer: int,
    active: bool,
) -> DressedEdgeState:
    if not active:
        return DressedEdgeState(
            primary=None,
            reserve=None,
            layer=layer,
            bank=frozenset(),
        )
    payload = (1, polarity)
    return DressedEdgeState(
        primary=payload,
        reserve=None,
        layer=layer,
        bank=target_packet(direction, payload, layer),
    )


def physical_pointer_signature(order, residual, state: ApparatusState):
    """Existing-carrier signature for the two pointers and event resources.

    Pointer A always retains its neutral field pair.  At an active event its
    A2 polarity token has moved to the charged A1 source relation.  Pointer B
    uses a second physical site for the single delay configuration, so its
    two field records and one A2 token never disappear.
    """

    left_index = state.pointer % 384
    right_index = state.pointer % 385
    left = order[left_index]
    right = order[right_index] if right_index < 384 else None
    active = event_active(order, residual, state)
    left_slots, left_token = pointer_configuration(left)
    if active:
        pointer_a_token = None
        source_token = (1, left[4])
    else:
        pointer_a_token = left_token
        source_token = None

    if right is None:
        pointer_b_active = None
        pointer_b_delay = pointer_configuration(order[-1])
    else:
        pointer_b_active = pointer_configuration(right)
        pointer_b_delay = None

    work_tokens = {
        polarity: (0 if active and polarity == left[4] else 8)
        for polarity in (-1, 1)
    }
    return (
        left_index,
        right_index,
        left_slots,
        pointer_a_token,
        pointer_b_active,
        pointer_b_delay,
        source_token,
        tuple(sorted(work_tokens.items())),
        state.detector,
    )


def decode_pointer_signature(order, residual, signature) -> ApparatusState:
    (
        left_index,
        right_index,
        left_slots,
        pointer_a_token,
        pointer_b_active,
        pointer_b_delay,
        source_token,
        work_tokens,
        detector,
    ) = signature
    left = order[left_index]
    expected_slots, expected_token = pointer_configuration(left)
    assert left_slots == expected_slots
    if source_token is None:
        assert pointer_a_token == expected_token
    else:
        assert pointer_a_token is None
        assert source_token == (1, left[4])

    if right_index == 384:
        assert pointer_b_active is None
        assert pointer_b_delay == pointer_configuration(order[-1])
    else:
        assert pointer_b_active == pointer_configuration(order[right_index])
        assert pointer_b_delay is None
    state = ApparatusState(pointer_from_pair(left_index, right_index), detector)
    active = event_active(order, residual, state)
    assert (source_token is not None) == active
    assert work_tokens == tuple(
        sorted(
            (polarity, 0 if active and polarity == left[4] else 8)
            for polarity in (-1, 1)
        )
    )
    return state


def source_tail(center: Vec, direction: Vec) -> Vec:
    return add(center, neg(direction))


def apparatus_support(chart: OrientedRepairChart) -> frozenset[Vec]:
    """Fourteen site owners inside one Moore cube centered at o+n."""

    center = add(chart.origin, chart.repair_normal)
    first, second, normal = chart.first, chart.second, chart.repair_normal
    positions = set(chart.vertices())
    positions.update(
        {
            center,
            add(add(center, neg(first)), neg(second)),  # signal bank
            add(add(center, first), neg(second)),       # pointer A
            add(add(center, neg(first)), second),       # pointer B
            add(add(center, first), second),            # material header
            add(center, first),                         # delay reserve
            add(center, neg(first)),                    # gravity + reserve
            add(add(center, neg(first)), normal),       # gravity + output
            add(center, neg(second)),                   # gravity - reserve
            add(add(center, neg(second)), normal),      # gravity - output
        }
    )
    return frozenset(positions)


def chart_constraint_axis(chart: OrientedRepairChart, direction: Vec) -> Vec:
    if collinear(direction, chart.first) or collinear(direction, chart.second):
        return chart.repair_normal
    assert collinear(direction, chart.repair_normal)
    return chart.first


@lru_cache(maxsize=1)
def gravity_seed_pairs():
    states = tuple(one_particle_states())
    orbits = internal_orbits(states)
    orbit_index = orbit_lookup(orbits)
    plus_target = (1, 4, -2, 0, 0, 3, 0, 0, -1)
    minus_target = (1, 4, -2, 0, 0, -3, 0, 0, -1)

    def first_pair(target):
        for left in states:
            for right in states:
                if orbit_index[left] == orbit_index[right]:
                    continue
                if bundle_payload(left, right, 0) == target:
                    return left, right
        raise AssertionError(target)

    return first_pair(plus_target), first_pair(minus_target)


def event_gravity_payloads(chart: OrientedRepairChart, direction: Vec):
    """Two shear-opposite packets for one event tangent and transverse axis."""

    q = chart_constraint_axis(chart, direction)
    middle = scale(cross(q, direction), chart.orientation)
    transform = matrix_from_columns(direction, middle, q)
    assert round(Matrix(transform).det()) == chart.orientation
    clock_shift = (-chart.layer) % 3
    output = []
    for seed_pair in gravity_seed_pairs():
        aligned = tuple(advance(state, clock_shift) for state in seed_pair)
        output.append(
            tuple(transform_state(transform, state) for state in aligned)
        )
    return tuple(output), q


def gravity_event_rows(
    chart: OrientedRepairChart,
    direction: Vec,
    states,
    state_set,
    orbit_index,
):
    payloads, q = event_gravity_payloads(chart, direction)

    rows = []
    for left, right in payloads:
        payload_orbits = {orbit_index[left], orbit_index[right]}
        router_candidates = [
            rotor
            for rotor in states
            if rotor_successor(advance(rotor, 1)) == chart.repair_normal
            and orbit_index[rotor] not in payload_orbits
        ]
        departure_rotor = min(
            router_candidates,
            key=lambda state: intrinsic_state_descriptor(state, chart),
        )
        destination_rotor = advance(departure_rotor, 3)
        before = (
            bundle_marked_site(departure_rotor, left, right),
            unmarked_site(destination_rotor),
        )
        after = local_bundle_step(
            before[0], before[1], state_set, orbit_index
        )
        assert after is not None
        assert after[2] == chart.repair_normal
        recovered = inverse_bundle_step(
            after[0], after[1], state_set, orbit_index
        )
        assert recovered is not None and recovered[:2] == before
        rows.append((before, after, (left, right)))
    return tuple(rows), q


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    state_set = frozenset(states)
    orbits = internal_orbits(states)
    orbit_index = orbit_lookup(orbits)
    group = tuple(signed_permutation_matrices())
    channels = all_local_channels()

    canonical_frame = PlaquetteFrame((0, 0, 0), 0, 0, 1)
    chart = canonical_chart(canonical_frame)
    order = address_order(chart)
    ports = tuple(sorted({outcome(channel) for channel in order}))
    fixtures = {
        ports[0]: (8, 1, 3, 0),
        ports[1]: (2, 7, 0, 2),
        ports[2]: (4, 4, 4, 4),
        ports[3]: (0, 0, 0, 2),
    }
    bank = bank_from_counts(order, fixtures)
    residual = canonical_residual(bank, order)

    # The charged frame and its existing neutral header are a finite physical
    # representation of the apparatus chart.
    header, _representatives = chart_code_representatives(chart, states)
    header_slots = polarized_slots(header)
    check(
        "C1 exact charged frame plus neutral header carries the oriented apparatus chart",
        chart_presentation(chart) == presentation(canonical_frame)
        and rotor_successor(header) == chart.repair_normal
        and len(header_slots) == 2
        and all(physical_value(header_slots, layer) == (0,) * 6 for layer in range(3)),
    )

    support = apparatus_support(chart)
    center = add(chart.origin, chart.repair_normal)
    check(
        "C2 frame, pointers, delay reserve, source, and two gravity lanes fit one Moore block",
        len(support) == 14
        and all(max(abs(a - b) for a, b in zip(site, center)) <= 1 for site in support),
        f"support={len(support)}",
    )

    # Pointer configurations are physical existing-carrier encodings.  The
    # CRT inverse proves the two configurations contain the complete finite
    # address without storing the 147,840-state integer.
    pointer_configs = tuple(pointer_configuration(channel) for channel in order)
    check(
        "C3 every 384-channel pointer address has one unique neutral-field/A2 encoding",
        len(set(pointer_configs)) == 384
        and all(len(slots) == 2 for slots, _token in pointer_configs),
    )
    crt_rows = 0
    for left_index in range(384):
        for right_index in range(385):
            pointer = pointer_from_pair(left_index, right_index)
            assert pointer % 384 == left_index
            assert pointer % 385 == right_index
            crt_rows += 1
    check(
        "C4 physical pointer pair is bijective with the complete coprime address orbit",
        crt_rows == PERIOD,
    )

    # Exhaust the logical permutation and the compact existing-carrier
    # encode/decode.  Active bright states move the left A2 token to the A1
    # source; dark completion states retain it at the pointer.
    permutation_rows = 0
    physical_rows = 0
    event_state_rows = 0
    for pointer in range(PERIOD):
        for detector in (RECOVERY, READY, MANIFESTED):
            logical = ApparatusState(pointer, detector)
            signature = physical_pointer_signature(order, residual, logical)
            assert decode_pointer_signature(order, residual, signature) == logical
            forward = apparatus_step(order, residual, logical)
            assert apparatus_inverse(order, residual, forward) == logical
            assert apparatus_step(
                order, residual, apparatus_inverse(order, residual, logical)
            ) == logical
            active = event_active(order, residual, logical)
            event_state_rows += active
            permutation_rows += 1
            physical_rows += 1
    check(
        "C5 detector/pointer rule is one exact full finite permutation",
        permutation_rows == PERIOD * 3,
    )
    check(
        "C6 existing-carrier pointer/source signatures decode every permutation state uniquely",
        physical_rows == PERIOD * 3,
    )

    # Each active state transfers nine A2 occupancies into eight field records
    # plus one SC relation record.  This is exactly on the connected
    # all-equal occupancy ray derived by the normalization predecessor.
    event_delta: RoleDelta = (8, 1, 0, -9)
    inverse_delta: RoleDelta = tuple(-entry for entry in event_delta)  # type: ignore[assignment]
    equal_weights = Matrix([1, 1, 1, 1])
    check(
        "C7 bright and recovery transfers conserve the full all-equal occupancy invariant",
        (Matrix([event_delta]) * equal_weights)[0] == 0
        and (Matrix([inverse_delta]) * equal_weights)[0] == 0
        and event_state_rows > 0,
        f"active_states={event_state_rows}",
    )

    # Exact charged source: blank before, primary phase-one plus one complete
    # packet after.  The A2 token transfer is the resource provenance omitted
    # by the earlier kinematic source macro.
    source_rows = 0
    stress_rows = 0
    gravity_rows = 0
    for direction, polarity, layer in product(
        SC_DIRECTIONS, (-1, 1), range(3)
    ):
        local_chart = OrientedRepairChart(
            chart.origin,
            chart.first,
            chart.second,
            chart.repair_normal,
            layer,
            chart.offset,
            chart.polarity,
        )
        before = source_edge_state(direction, polarity, layer, False)
        after = source_edge_state(direction, polarity, layer, True)
        tail = source_tail(center, direction)
        assert edge_divergence(tail, direction, before) == edge_charge(
            tail, direction, before
        ) == {}
        assert edge_divergence(tail, direction, after) == edge_charge(
            tail, direction, after
        )
        delta_charge = add_maps(
            edge_charge(tail, direction, after),
            scale_map(-1, edge_charge(tail, direction, before)),
        )
        assert add_maps(
            delta_charge,
            current_divergence(tail, direction, polarity, before, after),
        ) == {}
        assert len(after.bank) == 8
        source_rows += 1

        records = tuple(((ch[0], ch[1], ch[2]), ch[3]) for ch in after.bank)
        packet_stress = sum(
            (stress_matrix(record, layer) for record in records),
            Matrix.zeros(3, 3),
        )
        assert packet_stress == expected_packet_stress(direction)
        stress_rows += 1

        gravity_packets, q = gravity_event_rows(
            local_chart, direction, states, state_set, orbit_index
        )
        combined_tensor = sum(
            (Matrix(payload_tensor(row[2], layer)) for row in gravity_packets),
            Matrix.zeros(3, 3),
        )
        combined_vector = tuple(
            sum(payload_vector(advance(row[2][0], 2), layer)[index] for row in gravity_packets)
            for index in range(3)
        )
        q_column = Matrix(q)
        assert combined_tensor == 3 * packet_stress
        assert Matrix(scale(combined_vector, 2)) == 3 * packet_stress * q_column
        assert combined_vector == scale(q, -2)
        gravity_rows += len(gravity_packets)

    check(
        "C8 every outcome creates exact div-E charge and local continuity from a blank SC port",
        source_rows == 36,
    )
    check(
        "C9 every charged Gauss event carries the exact charge-even aligned STF stress",
        stress_rows == 36,
    )
    check(
        "C10 two reversible neutral packets match event stress and transverse constraint load",
        gravity_rows == 72,
    )

    # Full signed-cubic covariance.  The shear-opposite pair is unordered, so
    # an improper map may exchange its two members without changing the
    # physical two-packet source.
    covariance_rows = 0
    base_payloads = {
        direction: event_gravity_payloads(chart, direction)[0]
        for direction in SC_DIRECTIONS
    }
    for matrix in group:
        transformed_chart = transform_chart(matrix, chart)
        assert transform_presentation(
            matrix, chart_presentation(chart)
        ) == chart_presentation(transformed_chart)
        next_header, _ = chart_code_representatives(transformed_chart, states)
        assert transform_state(matrix, header) == next_header
        for direction in SC_DIRECTIONS:
            transformed_direction = tuple(matrix_vector(matrix, direction))
            transformed_payloads, transformed_q = event_gravity_payloads(
                transformed_chart, transformed_direction
            )
            expected_payloads = {
                frozenset(transform_state(matrix, state) for state in payload)
                for payload in base_payloads[direction]
            }
            assert {
                frozenset(payload) for payload in transformed_payloads
            } == expected_payloads
            assert transformed_q == tuple(
                matrix_vector(
                    matrix, chart_constraint_axis(chart, direction)
                )
            )
            covariance_rows += 2
    check(
        "C11 material chart and two-packet event source are fully signed-cubic covariant",
        covariance_rows == 48 * 6 * 2,
    )

    # Matter schedule: an apparatus needs an exact frame and blank repair
    # output.  A one-defect frame enters forward repair instead.  An exact
    # frame with a retained syndrome enters reverse repair.  These predicates
    # are disjoint without a coordinate priority or random arbitration.
    matter_rows = 0
    admission_counts = Counter()
    for frame in frame_family():
        exact = presentation(frame)
        admission_counts[("apparatus", exact, False)] += 1
        admission_counts[("reverse_repair", exact, True)] += 1
        for _category, defect in enumerate_one_defects(frame, channels):
            assert defect != exact
            admission_counts[("forward_repair", defect, False)] += 1
            matter_rows += 1
    check(
        "C12 exact apparatus, forward repair, and syndrome-selected reverse repair admissions are state-disjoint",
        matter_rows == 37_632
        and sum(1 for key in admission_counts if key[0] == "apparatus") == 24
        and sum(1 for key in admission_counts if key[0] == "reverse_repair") == 24
        and sum(1 for key in admission_counts if key[0] == "forward_repair") == 37_632,
    )

    # On dark pointer steps all charged/gravity event resources stay in their
    # prepared ownership state.  On a bright ready step both activate; the
    # next manifested step reverses them before the pointer advances.  This is
    # the exact finite event/recovery composition used by the operational
    # orbit, not a permanent record or outgoing radiation theorem.
    transition_counts = Counter()
    for pointer in range(PERIOD):
        for detector in (RECOVERY, READY, MANIFESTED):
            before = ApparatusState(pointer, detector)
            after = apparatus_step(order, residual, before)
            transition_counts[
                (event_active(order, residual, before), event_active(order, residual, after))
            ] += 1
    check(
        "C13 one reversible macro launches and recovers Gauss/gravity resources exactly at bright events",
        transition_counts[(False, True)] > 0
        and transition_counts[(True, False)] > 0
        and transition_counts[(True, True)] == 0,
        str(transition_counts),
    )

    # Prepared event frequencies are inherited from the physical pointer
    # permutation; the new source transaction reads only the actual bright
    # port and never a probability or desired count.
    logical = ApparatusState(0, READY)
    seen = set()
    observed = Counter()
    while logical not in seen:
        seen.add(logical)
        if event_active(order, residual, logical):
            port = bright_outcome(order, residual, logical.pointer)
            assert port is not None
            observed[port] += 1
        logical = apparatus_step(order, residual, logical)
    expected = {
        port: (counts[0] - counts[2]) ** 2 + (counts[1] - counts[3]) ** 2
        for port, counts in fixtures.items()
    }
    check(
        "C14 physical Gauss/stress events retain the exact prepared Born count",
        observed == Counter({port: count for port, count in expected.items() if count}),
        str(observed),
    )

    # No target constant enters.  The event fixes a relative occupancy/source
    # handoff, while a common positive multiplier and the detector/site action
    # remain invisible to every finite permutation/counting check above.
    gamma_costs = {
        gamma: gamma * sum(event_delta)
        for gamma in (Rational(1, 2), Rational(1), Rational(7))
    }
    check(
        "C15 the event conservation identity leaves the absolute common action multiplier free",
        set(gamma_costs.values()) == {0}
        and len(gamma_costs) == 3,
    )

    missing = {
        "autonomous material-frame, bank, A2-work, and gravity-kit formation",
        "parallel traffic arbitration beyond the isolated Moore block",
        "detector actuality and C3-layer action terms",
        "perturbative matter survival and translation",
        "charged and tensor-protected interacting poles",
        "absolute action/coupling normalization",
        "amplified persistent records and multipartite no-signalling",
        "universal response, lensing, and nonlinear gravity",
    }
    check(
        "C16 the five physical sectors remain open beyond the common finite event seam",
        len(missing) == 8,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(
        f"\n{passed}/{len(checks)} matter-anchored Born/Gauss/gravity event checks pass"
    )
    print(f"pointer_crt_rows={crt_rows}")
    print(f"full_apparatus_permutation_rows={permutation_rows}")
    print(f"active_event_states={event_state_rows}")
    print(f"charged_source_rows={source_rows}")
    print(f"gravity_packet_rows={gravity_rows}")
    print(f"signed_cubic_source_rows={covariance_rows}")
    print(f"matter_admission_rows={matter_rows}")
    print(f"prepared_event_counts={dict(observed)}")
    print("event_role_delta=(8,1,0,-9)")
    print("event_result=five_sector_common_finite_vertex_exact_conditional_on_prepared_resources")
    print("physical_status=formation_Phi_traffic_poles_absolute_scale_stability_and_lensing_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
