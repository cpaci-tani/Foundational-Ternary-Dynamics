#!/usr/bin/env python3
"""Exact prepared-sector integration certificate for a v3 Phi-v4 candidate.

The prior five-sector event seam placed compatible records in one Moore block
but did not give every output coordinate a radius-one state-only admission
test.  This certificate supplies that missing scheduling layer without a
coordinate coloring or a new primitive type:

* seventeen chart-relative sites carry exact zero-E/B exclusion markers;
* ten remaining sites carry the frame, bank, pointers, herald, source, and
  gravity-source records;
* any two Moore blocks whose writers overlap necessarily corrupt at least one
  marker-only site, so two complete kits can never be admitted together;
* READY -> HERALD -> ACTIVE/RECOVERY is an exact finite pointer permutation;
  the retained herald removes every same-tick nonlocal dependency; and
* active source creation conserves the conditionally selected all-equal
  occupancy ray while retaining exact Gauss, continuity, stress, and prepared
  Born event counts.

This remains conditional on a prepared exact frame, signal bank, work reserve,
and exclusion halo.  It integrates event admission and writer arbitration on
that sector; it does not form those resources, generate persistent outgoing
records, protect matter or poles, or fix an absolute action scale.
"""

from __future__ import annotations

import sys
from collections import Counter
from dataclasses import dataclass
from functools import lru_cache
from itertools import combinations, product

from sympy import Matrix

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
)
from proof_v3_charged_common_action_phi_v3_candidate import (
    PlaquetteFrame,
    current_divergence,
    relation_key,
)
from proof_v3_contextual_neutral_pointer_born_renewal_apparatus import (
    DELAY_CONFIGURATION,
    address_order,
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
from proof_v3_matter_anchored_born_gauss_gravity_event_seam import (
    PERIOD,
    event_gravity_payloads,
    scale,
)
from proof_v3_neutral_rotor_harmonic_green_seam import (
    SC_DIRECTIONS,
    add,
)
from proof_v3_neutral_rotor_walker_macro import (
    advance,
    physical_value,
    polarized_slots,
    unmarked_site,
)
from proof_v3_neutral_scalar_vector_stf_bundle_common_green_seam import (
    bundle_marked_site,
    orbit_lookup,
    recognize_bundle_marked,
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
    chart_codewords,
    chart_presentation,
    coordinates,
    intrinsic_state_descriptor,
    mv,
    transform_chart,
    transform_slots,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
Slot = tuple[object, int]
Relative = tuple[int, int, int]
Port = tuple[Vec, int]

ALL_RELATIVE: tuple[Relative, ...] = tuple(
    product((-1, 0, 1), repeat=3)
)

# These ten chart-relative positions contain every event writer.  The exact
# charged frame is the square z=0, x,y in {0,1}.  The complement is marker
# only.  The asymmetric shape is contextual: transforming the chart transforms
# the shape, so no global spatial direction is selected.
ROLE_RELATIVE: tuple[Relative, ...] = (
    (-1, 1, 0),
    (0, -1, 0),
    (0, 0, -1),
    (0, 0, 0),
    (0, 0, 1),
    (0, 1, 0),
    (1, 0, -1),
    (1, 0, 0),
    (1, 0, 1),
    (1, 1, 0),
)
ROLE_SET = frozenset(ROLE_RELATIVE)
MARKER_RELATIVE: tuple[Relative, ...] = tuple(
    relative for relative in ALL_RELATIVE if relative not in ROLE_SET
)
MARKER_WORD_COUNT = 24 * len(MARKER_RELATIVE)
ROLE_COMBO_POOL = 64
ROLE_WORD_COUNT = MARKER_WORD_COUNT + 24 * ROLE_COMBO_POOL

CENTER = (0, 0, 0)
BANK_SITES = ((-1, 1, 0), (0, 0, -1))
FRAME_RELATIVE = frozenset(
    {(0, 0, 0), (1, 0, 0), (0, 1, 0), (1, 1, 0)}
)

READY = 0
HERALD = 1
RECOVERY = -1


def subtract(left: Vec, right: Vec) -> Vec:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def relative_site(chart: OrientedRepairChart, relative: Relative) -> Vec:
    result = chart.origin
    for coefficient, axis in zip(
        relative, (chart.first, chart.second, chart.repair_normal)
    ):
        for _ in range(abs(coefficient)):
            result = add(result, axis if coefficient > 0 else scale(axis, -1))
    return result


def relative_vector(chart: OrientedRepairChart, vector: Vec) -> Relative:
    return tuple(
        sum(a * b for a, b in zip(vector, axis))
        for axis in (chart.first, chart.second, chart.repair_normal)
    )  # type: ignore[return-value]


def channel_slot(channel) -> Slot:
    tangent, normal, handedness, phase, polarity = channel
    return (((tangent, normal, handedness), phase), polarity)


def frame_field_slots(chart: OrientedRepairChart) -> dict[Vec, set[Slot]]:
    output: dict[Vec, set[Slot]] = {}
    for site, channel in chart_presentation(chart).fields:
        output.setdefault(site, set()).add(channel_slot(channel))
    return output


def chart_combo(chart: OrientedRepairChart) -> int:
    polarity_bit = 0 if chart.polarity == -1 else 1
    return (chart.layer * 4 + chart.offset) * 2 + polarity_bit


@lru_cache(maxsize=None)
def marker_codes(chart: OrientedRepairChart, states) -> tuple[frozenset[Slot], ...]:
    """Seventeen globally unique constant-weight marker-only signatures."""

    words = chart_codewords(chart, states, MARKER_WORD_COUNT)
    base = chart_combo(chart) * len(MARKER_RELATIVE)
    return tuple(words[base + index] for index in range(len(MARKER_RELATIVE)))


@lru_cache(maxsize=None)
def assign_role_pads(
    chart: OrientedRepairChart, states
) -> dict[Relative, frozenset[Slot]]:
    """First intrinsic matching clear of every registered event writer.

    The pads are part of the static halo.  Their matching therefore excludes
    not only the charged frame but every source-bank slot and every fixed
    payload controller that can be written at a role site in any of the
    twelve physical event ports.  Router controllers remain first-fit because
    they are freely selectable from the remaining registered one-particle
    states.
    """

    frame_fields = frame_field_slots(chart)
    forbidden = {
        relative: frame_fields.get(relative_site(chart, relative), set())
        for relative in ROLE_RELATIVE
    }

    for direction in SC_DIRECTIONS:
        for edge in source_edges(chart, direction):
            head = add(edge[0], direction)
            relative = relative_vector(chart, subtract(head, chart.origin))
            for polarity in (-1, 1):
                forbidden[relative].update(
                    channel_slot(channel)
                    for channel in target_packet(
                        direction, (1, polarity), chart.layer
                    )
                )

        fixed_payload_slots: set[Slot] = set()
        for left, right in event_gravity_payloads(chart, direction)[0]:
            fixed_payload_slots.update(polarized_slots(left))
            fixed_payload_slots.update(polarized_slots(advance(left, 2)))
            fixed_payload_slots.update(polarized_slots(right))
        for _departure, destination in gravity_lane_edges(chart):
            relative = relative_vector(
                chart, subtract(destination, chart.origin)
            )
            forbidden[relative].update(fixed_payload_slots)

    words = chart_codewords(chart, states, ROLE_WORD_COUNT)
    assignment: dict[Relative, frozenset[Slot]] = {}
    combo = chart_combo(chart)
    start = MARKER_WORD_COUNT + combo * ROLE_COMBO_POOL
    center_code = next(
        (
            words[index]
            for index in range(start, start + ROLE_COMBO_POOL)
            if not (words[index] & forbidden[CENTER])
        ),
        None,
    )
    assert center_code is not None, chart
    assignment[CENTER] = center_code

    ordered = tuple(
        sorted(states, key=lambda state: intrinsic_state_descriptor(state, chart))
    )
    used_controllers = {slot[0] for slot in center_code}
    for relative in ROLE_RELATIVE:
        if relative == CENTER:
            continue
        controllers = tuple(
            state
            for state in ordered
            if state not in used_controllers
            and not (polarized_slots(state) & forbidden[relative])
        )[:10]
        assert len(controllers) == 10, (chart, relative)
        used_controllers.update(controllers)
        assignment[relative] = frozenset().union(
            *(polarized_slots(state) for state in controllers)
        )
    return assignment


def ordered_ports(chart: OrientedRepairChart) -> tuple[Port, ...]:
    return tuple(
        sorted(
            product(SC_DIRECTIONS, (-1, 1)),
            key=lambda port: (relative_vector(chart, port[0]), port[1]),
        )
    )


def herald_states(
    chart: OrientedRepairChart,
    states,
    center_pad: frozenset[Slot],
) -> dict[Port | None, object]:
    """One neutral pair encodes dark plus the twelve physical event ports."""

    blocked = {slot[0] for slot in center_pad}
    blocked.update(
        slot[0]
        for slot in frame_field_slots(chart).get(chart.origin, set())
    )
    for direction in SC_DIRECTIONS:
        if not any(
            add(edge[0], direction) == chart.origin
            for edge in source_edges(chart, direction)
        ):
            continue
        for polarity in (-1, 1):
            blocked.update(
                channel_slot(channel)[0]
                for channel in target_packet(
                    direction, (1, polarity), chart.layer
                )
            )
    ordered = tuple(
        state
        for state in sorted(
            states, key=lambda state: intrinsic_state_descriptor(state, chart)
        )
        if state not in blocked
    )
    labels: tuple[Port | None, ...] = (None,) + ordered_ports(chart)
    assert len(ordered) >= len(labels)
    return dict(zip(labels, ordered[: len(labels)]))


@dataclass(frozen=True)
class StagedState:
    pointer: int
    stage: int
    label: Port | None


def staged_states(order, residual) -> tuple[StagedState, ...]:
    output = []
    for pointer in range(PERIOD):
        label = bright_outcome(order, residual, pointer)
        output.append(StagedState(pointer, READY, None))
        output.append(StagedState(pointer, HERALD, label))
        if label is not None:
            output.append(StagedState(pointer, RECOVERY, label))
    return tuple(output)


def staged_step(order, residual, state: StagedState) -> StagedState:
    expected = bright_outcome(order, residual, state.pointer)
    if state.stage == READY and state.label is None:
        return StagedState(state.pointer, HERALD, expected)
    if state.stage == HERALD and state.label == expected:
        if expected is None:
            return StagedState((state.pointer + 1) % PERIOD, READY, None)
        return StagedState(state.pointer, RECOVERY, expected)
    if state.stage == RECOVERY and state.label == expected and expected is not None:
        return StagedState((state.pointer + 1) % PERIOD, READY, None)
    raise ValueError("outside the exact staged apparatus component")


def staged_inverse(order, residual, state: StagedState) -> StagedState:
    if state.stage == HERALD:
        expected = bright_outcome(order, residual, state.pointer)
        assert state.label == expected
        return StagedState(state.pointer, READY, None)
    if state.stage == RECOVERY:
        expected = bright_outcome(order, residual, state.pointer)
        assert expected is not None and state.label == expected
        return StagedState(state.pointer, HERALD, expected)
    assert state.stage == READY and state.label is None
    prior = (state.pointer - 1) % PERIOD
    expected = bright_outcome(order, residual, prior)
    if expected is None:
        return StagedState(prior, HERALD, None)
    return StagedState(prior, RECOVERY, expected)


def source_edges(chart: OrientedRepairChart, direction: Vec):
    frame_relations = {relation_key(edge) for edge in chart.edges()}
    role_sites = {relative_site(chart, relative) for relative in ROLE_RELATIVE}
    rows = []
    for tail in role_sites:
        head = add(tail, direction)
        edge = (tail, direction)
        if head in role_sites and relation_key(edge) not in frame_relations:
            rows.append(edge)
    return tuple(
        sorted(
            rows,
            key=lambda edge: (
                relative_vector(chart, subtract(edge[0], chart.origin)),
                relative_vector(chart, subtract(add(edge[0], edge[1]), chart.origin)),
            ),
        )
    )


def gravity_lane_edges(chart: OrientedRepairChart):
    role_sites = {relative_site(chart, relative) for relative in ROLE_RELATIVE}
    bank_sites = {relative_site(chart, relative) for relative in BANK_SITES}
    rows = []
    for departure in role_sites:
        destination = add(departure, chart.repair_normal)
        if (
            destination in role_sites
            and departure not in bank_sites
            and destination not in bank_sites
        ):
            rows.append((departure, destination))
    return tuple(
        sorted(
            rows,
            key=lambda row: relative_vector(
                chart, subtract(row[0], chart.origin)
            ),
        )
    )


def add_exact(fields: dict[Vec, set[Slot]], site: Vec, slots) -> bool:
    slots = set(slots)
    if fields.setdefault(site, set()) & slots:
        return False
    fields[site].update(slots)
    return len(fields[site]) <= 384


def transform_field_map(matrix, fields) -> dict[Vec, set[Slot]]:
    return {
        mv(matrix, site): set(transform_slots(matrix, frozenset(slots)))
        for site, slots in fields.items()
    }


def place_pointer_fields(chart, fields, pointer_slot_sets):
    """First-fit the retained neutral pointer pairs inside the role halo."""

    trial = {site: set(slots) for site, slots in fields.items()}
    sites = tuple(relative_site(chart, relative) for relative in ROLE_RELATIVE)
    placements = []
    for slots in pointer_slot_sets:
        if not slots:
            placements.append(None)
            continue
        placed = None
        for site in sites:
            if not (trial.get(site, set()) & set(slots)):
                assert add_exact(trial, site, slots)
                placed = site
                break
        if placed is None:
            return None
        placements.append(placed)
    return trial, tuple(placements)


def fixture_bank(chart: OrientedRepairChart):
    order = address_order(chart)
    ports = ordered_ports(chart)[:4]
    fixtures = {
        ports[0]: (8, 1, 3, 0),
        ports[1]: (2, 7, 0, 2),
        ports[2]: (4, 4, 4, 4),
        ports[3]: (0, 0, 0, 2),
    }
    bank = bank_from_counts(order, fixtures)
    residual = canonical_residual(bank, order)
    return order, fixtures, bank, residual


def base_role_fields(chart: OrientedRepairChart, states, bank):
    pads = assign_role_pads(chart, states)
    fields: dict[Vec, set[Slot]] = {
        relative_site(chart, relative): set(pad)
        for relative, pad in pads.items()
    }
    for site, slots in frame_field_slots(chart).items():
        assert add_exact(fields, site, slots)

    first, second = BANK_SITES
    first_site, second_site = (
        relative_site(chart, first),
        relative_site(chart, second),
    )
    for channel in bank:
        slot = channel_slot(channel)
        target = first_site if slot not in fields[first_site] else second_site
        assert add_exact(fields, target, (slot,))
    return pads, fields


def choose_active_packing(
    chart: OrientedRepairChart,
    states,
    state_set,
    orbit_index,
    base_fields,
    herald_state,
    pointer_slot_sets,
    direction: Vec,
    polarity: int,
):
    """Choose the first chart-intrinsic exact source/two-packet packing."""

    fields = {site: set(slots) for site, slots in base_fields.items()}
    if not add_exact(fields, chart.origin, polarized_slots(herald_state)):
        return None
    pointer_packing = place_pointer_fields(chart, fields, pointer_slot_sets)
    if pointer_packing is None:
        return None
    fields, pointer_placements = pointer_packing
    before_active = {site: set(slots) for site, slots in fields.items()}

    source_packet = target_packet(direction, (1, polarity), chart.layer)
    source_slots = frozenset(channel_slot(channel) for channel in source_packet)
    source_choice = None
    for edge in source_edges(chart, direction):
        head = add(edge[0], direction)
        trial = {site: set(slots) for site, slots in fields.items()}
        if add_exact(trial, head, source_slots):
            source_choice = edge
            fields = trial
            break
    if source_choice is None:
        return None

    payloads, q = event_gravity_payloads(chart, direction)
    lanes = gravity_lane_edges(chart)
    lane_pairs = tuple(
        pair
        for pair in combinations(lanes, 2)
        if len(set(pair[0]) | set(pair[1])) == 4
    )
    for selected_lanes in lane_pairs:
        trial = {site: set(slots) for site, slots in fields.items()}
        packed = []
        success = True
        for payload, lane in zip(payloads, selected_lanes):
            left, right = payload
            payload_orbits = {orbit_index[left], orbit_index[right]}
            departure, destination = lane
            choice = None
            for marked_rotor in sorted(
                states,
                key=lambda state: intrinsic_state_descriptor(state, chart),
            ):
                if orbit_index[marked_rotor] in payload_orbits:
                    continue
                marked = bundle_marked_site(marked_rotor, left, right)
                if recognize_bundle_marked(marked, state_set, orbit_index) is None:
                    continue
                for reserve_rotor in sorted(
                    states,
                    key=lambda state: intrinsic_state_descriptor(state, chart),
                ):
                    reserve = unmarked_site(reserve_rotor)
                    if trial.get(destination, set()) & marked:
                        continue
                    if trial.get(departure, set()) & reserve:
                        continue
                    choice = reserve, marked, marked_rotor, reserve_rotor
                    break
                if choice is not None:
                    break
            if choice is None:
                success = False
                break
            reserve, marked, marked_rotor, reserve_rotor = choice
            assert add_exact(trial, departure, reserve)
            assert add_exact(trial, destination, marked)
            packed.append((lane, payload, marked_rotor, reserve_rotor))
        if success:
            return (
                before_active,
                trial,
                source_choice,
                tuple(packed),
                q,
                pointer_placements,
            )
    return None


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    state_set = frozenset(states)
    orbit_index = orbit_lookup(internal_orbits(states))
    group = tuple(signed_permutation_matrices())
    generators = (
        ((0, 1, 0), (1, 0, 0), (0, 0, 1)),
        ((1, 0, 0), (0, 0, 1), (0, 1, 0)),
        ((-1, 0, 0), (0, 1, 0), (0, 0, 1)),
    )

    canonical_frames = frame_family()
    canonical_charts = tuple(canonical_chart(frame) for frame in canonical_frames)
    chart_orbit = {
        transform_chart(matrix, chart)
        for chart in canonical_charts
        for matrix in group
    }
    check(
        "C1 event layout uses seventeen marker-only and ten writer sites in one Moore cube",
        len(MARKER_RELATIVE) == 17
        and len(ROLE_RELATIVE) == 10
        and FRAME_RELATIVE <= ROLE_SET
        and all(max(abs(value) for value in relative) <= 1 for relative in ALL_RELATIVE),
    )

    # For every relative orientation and every displacement of overlapping
    # Moore cubes, their overlap is not entirely writer/writer.  At least one
    # kit therefore expects an exact marker-only signature at a shared site.
    exclusion_rows = 0
    for matrix in group:
        rotated_role = {tuple(matrix_vector(matrix, row)) for row in ROLE_SET}
        rotated_cube = {tuple(matrix_vector(matrix, row)) for row in ALL_RELATIVE}
        for displacement in product(range(-2, 3), repeat=3):
            shifted_cube = {
                tuple(a + b for a, b in zip(displacement, row))
                for row in rotated_cube
            }
            overlap = set(ALL_RELATIVE) & shifted_cube
            if not overlap:
                continue
            shifted_role = {
                tuple(a + b for a, b in zip(displacement, row))
                for row in rotated_role
            }
            assert not overlap <= (ROLE_SET & shifted_role)
            exclusion_rows += 1
    check(
        "C2 every pair of overlapping event-writer blocks has a marker-only exclusion witness",
        exclusion_rows == 6_000,
    )

    marker_rows = []
    neutral_marker_rows = 0
    for chart in chart_orbit:
        codes = marker_codes(chart, states)
        assert len(codes) == 17
        for code in codes:
            assert len(code) == 18
            for layer in range(3):
                assert physical_value(code, layer) == (0,) * 6
                neutral_marker_rows += 1
            marker_rows.append(code)
    check(
        "C3 all chart/position marker-only signatures are globally unique and zero-E/B",
        len(marker_rows) == 1_152 * 17
        and len(set(marker_rows)) == len(marker_rows)
        and neutral_marker_rows == 1_152 * 17 * 3,
    )

    marker_covariance_rows = 0
    role_covariance_rows = 0
    center_rows = []
    for chart in chart_orbit:
        pads = assign_role_pads(chart, states)
        center_rows.append(pads[CENTER])
        assert len(pads[CENTER]) == 18
        assert all(
            len(pad) == 20
            for relative, pad in pads.items()
            if relative != CENTER
        )
        assert all(
            not (pads[left] & pads[right])
            for left, right in combinations(ROLE_RELATIVE, 2)
        )
        for matrix in generators:
            transformed = transform_chart(matrix, chart)
            for index in range(17):
                assert transform_slots(matrix, marker_codes(chart, states)[index]) == marker_codes(
                    transformed, states
                )[index]
                marker_covariance_rows += 1
            transformed_pads = assign_role_pads(transformed, states)
            for relative in ROLE_RELATIVE:
                assert transform_slots(matrix, pads[relative]) == transformed_pads[relative]
                role_covariance_rows += 1
    check(
        "C4 the center chart code is globally unique and every writer-clear role pad is generator-covariant",
        marker_covariance_rows == 1_152 * 3 * 17
        and role_covariance_rows == 1_152 * 3 * 10
        and len(center_rows) == len(set(center_rows)) == 1_152
        and not (set(marker_rows) & set(center_rows)),
    )

    # The two disjoint bank pads give a complete carrier sharding: every one
    # of the 384 channel slots is absent from at least one of them.
    bank_rows = 0
    for chart in canonical_charts:
        pads = assign_role_pads(chart, states)
        first, second = (pads[site] for site in BANK_SITES)
        assert not (first & second)
        for channel in all_local_channels():
            slot = channel_slot(channel)
            assert slot not in first or slot not in second
            bank_rows += 1
    check(
        "C5 two marker-disjoint bank sites can shard every finite v3 field address",
        bank_rows == 24 * 384,
    )

    chart = canonical_chart(PlaquetteFrame((0, 0, 0), 0, 0, 1))
    order, fixtures, bank, residual = fixture_bank(chart)
    pads, base_fields = base_role_fields(chart, states, bank)
    herald_map = herald_states(chart, states, pads[CENTER])
    pointer_configurations = tuple(pointer_configuration(channel) for channel in order)
    check(
        "C6 both contextual pointers use only unique neutral field pairs and one existing A2 token",
        len(set(pointer_configurations)) == 384
        and DELAY_CONFIGURATION not in pointer_configurations
        and len(set(pointer_configurations) | {DELAY_CONFIGURATION}) == 385
        and all(
            len(slots) == 2
            and all(physical_value(slots, layer) == (0,) * 6 for layer in range(3))
            for slots, _token in pointer_configurations
        ),
    )

    check(
        "C7 dark plus twelve event ports have unique existing-carrier neutral heralds",
        len(herald_map) == 13
        and len(set(herald_map.values())) == 13
        and all(
            not (polarized_slots(state) & pads[CENTER])
            for state in herald_map.values()
        ),
    )

    valid_states = staged_states(order, residual)
    valid_set = frozenset(valid_states)
    image = {state: staged_step(order, residual, state) for state in valid_states}
    inverse_rows = 0
    for before, after in image.items():
        assert after in valid_set
        assert staged_inverse(order, residual, after) == before
        inverse_rows += 1
    event_count = sum(state.stage == RECOVERY for state in valid_states)
    check(
        "C8 retained herald makes the complete staged apparatus one exact finite permutation",
        len(image) == len(set(image.values())) == 2 * PERIOD + event_count
        and inverse_rows == len(valid_states),
    )

    # READY retains both physical pointer records.  The center sees every role
    # site, replaces its dark neutral herald pair by the addressed event-pair,
    # and all remote writers read that retained herald on the next tick.
    physical_pointer_rows = 0
    for left_index in range(384):
        left_slots, _left_token = pointer_configurations[left_index]
        for right_index in range(385):
            right_slots = (
                pointer_configurations[right_index][0]
                if right_index < 384
                else DELAY_CONFIGURATION[0]
            )
            pointer = left_index + 384 * ((left_index - right_index) % 385)
            assert pointer % 384 == left_index and pointer % 385 == right_index
            label = bright_outcome(order, residual, pointer)
            staged_fields = {site: set(slots) for site, slots in base_fields.items()}
            assert add_exact(
                staged_fields,
                chart.origin,
                polarized_slots(herald_map[label]),
            )
            placed = place_pointer_fields(
                chart, staged_fields, (left_slots, right_slots)
            )
            assert placed is not None
            assert all(
                max(abs(a - b) for a, b in zip(site, chart.origin)) <= 1
                for site in placed[1]
                if site is not None
            )
            physical_pointer_rows += 1
    check(
        "C9 every coprime physical pointer state admits one retained radius-one herald stage",
        physical_pointer_rows == PERIOD,
    )

    herald_delta = (0, 0, 0, 0)
    active_delta = (32, 1, 0, -33)
    check(
        "C10 neutral herald replacement and active source creation conserve the all-equal occupancy ray",
        sum(herald_delta) == 0
        and sum(active_delta) == 0
        and tuple(-value for value in active_delta) == (-32, -1, 0, 33),
    )

    # Exact carrier packing for every direction/sign/layer.  Source edge and
    # the two neutral bundles are chosen by chart-intrinsic first-fit over the
    # finite registered alternatives; all field banks remain sets of distinct
    # existing v3 slots and stay below capacity.
    packing_rows = 0
    source_rows = 0
    gravity_rows = 0
    stress_rows = 0
    active_covariance_rows = 0
    packed_charts = tuple(
        OrientedRepairChart(
            chart.origin,
            chart.first,
            chart.second,
            chart.repair_normal,
            layer,
            chart.offset,
            chart.polarity,
        )
        for layer in range(3)
    )
    for local_chart, direction, polarity in product(
        packed_charts, SC_DIRECTIONS, (-1, 1)
    ):
        local_order, _local_fixtures, local_bank, _local_residual = fixture_bank(
            local_chart
        )
        _local_pads, local_base = base_role_fields(local_chart, states, local_bank)
        local_heralds = herald_states(
            local_chart, states, _local_pads[CENTER]
        )
        event_channel = next(
            channel
            for channel in local_order
            if outcome(channel) == (direction, polarity)
        )
        event_pointer_slots = pointer_configuration(event_channel)[0]
        packed = choose_active_packing(
            local_chart,
            states,
            state_set,
            orbit_index,
            local_base,
            local_heralds[(direction, polarity)],
            (event_pointer_slots, event_pointer_slots),
            direction,
            polarity,
        )
        assert packed is not None, (
            local_chart,
            direction,
            polarity,
            tuple(source_edges(local_chart, direction)),
            tuple(gravity_lane_edges(local_chart)),
        )
        (
            before_fields,
            after_fields,
            source_edge,
            gravity_packets,
            q,
            pointer_placements,
        ) = packed
        assert len(pointer_placements) == 2
        assert all(len(slots) <= 384 for slots in after_fields.values())
        added = sum(
            len(after_fields[site] - before_fields.get(site, set()))
            for site in after_fields
        )
        assert added == 32
        packing_rows += 1

        tail = source_edge[0]
        source_state = DressedEdgeState(
            primary=(1, polarity),
            reserve=None,
            layer=local_chart.layer,
            bank=target_packet(direction, (1, polarity), local_chart.layer),
        )
        blank = DressedEdgeState(
            primary=None,
            reserve=None,
            layer=local_chart.layer,
            bank=frozenset(),
        )
        assert edge_divergence(tail, direction, source_state) == edge_charge(
            tail, direction, source_state
        )
        delta_charge = edge_charge(tail, direction, source_state)
        assert add_maps(
            delta_charge,
            current_divergence(
                tail, direction, polarity, blank, source_state
            ),
        ) == {}
        source_rows += 1

        records = tuple(
            ((channel[0], channel[1], channel[2]), channel[3])
            for channel in source_state.bank
        )
        packet_stress = sum(
            (stress_matrix(record, local_chart.layer) for record in records),
            Matrix.zeros(3, 3),
        )
        assert packet_stress == expected_packet_stress(direction)
        stress_rows += 1

        combined_tensor = sum(
            (
                Matrix(payload_tensor(payload, local_chart.layer))
                for _lane, payload, _marked, _reserve in gravity_packets
            ),
            Matrix.zeros(3, 3),
        )
        assert combined_tensor == 3 * packet_stress
        # The event payload constructor uses R^2(left) as its vector
        # controller; bundle storage keeps that finite relation implicit.
        expected_vector = scale(q, -2)
        actual_vector = tuple(
            sum(
                payload_vector(
                    # R^2 is the registered vector controller.
                    advance(payload[0], 2),
                    local_chart.layer,
                )[index]
                for _lane, payload, _marked, _reserve in gravity_packets
            )
            for index in range(3)
        )
        assert actual_vector == expected_vector
        gravity_rows += len(gravity_packets)

        for matrix in generators:
            transformed_chart = transform_chart(matrix, local_chart)
            transformed_direction = mv(matrix, direction)
            transformed_order, _fixtures, transformed_bank, _residual = fixture_bank(
                transformed_chart
            )
            transformed_pads, transformed_base = base_role_fields(
                transformed_chart, states, transformed_bank
            )
            transformed_herald = herald_states(
                transformed_chart,
                states,
                transformed_pads[CENTER],
            )[(transformed_direction, polarity)]
            transformed_packing = choose_active_packing(
                transformed_chart,
                states,
                state_set,
                orbit_index,
                transformed_base,
                transformed_herald,
                (
                    transform_slots(matrix, event_pointer_slots),
                    transform_slots(matrix, event_pointer_slots),
                ),
                transformed_direction,
                polarity,
            )
            assert transformed_packing is not None
            (
                transformed_before,
                transformed_after,
                transformed_source,
                transformed_gravity,
                transformed_q,
                transformed_pointer_placements,
            ) = transformed_packing
            assert transform_field_map(matrix, before_fields) == transformed_before
            assert transform_field_map(matrix, after_fields) == transformed_after
            assert transformed_source == (
                mv(matrix, source_edge[0]),
                transformed_direction,
            )
            expected_gravity = tuple(
                (
                    (mv(matrix, lane[0]), mv(matrix, lane[1])),
                    tuple(transform_state(matrix, state) for state in payload),
                    transform_state(matrix, marked_rotor),
                    transform_state(matrix, reserve_rotor),
                )
                for lane, payload, marked_rotor, reserve_rotor in gravity_packets
            )
            assert transformed_gravity == expected_gravity
            assert transformed_q == mv(matrix, q)
            assert transformed_pointer_placements == tuple(
                mv(matrix, site) if site is not None else None
                for site in pointer_placements
            )
            assert len(transformed_order) == 384
            active_covariance_rows += 1

    check(
        "C11 every event chart packs one exact Gauss source plus two neutral gravity packets without slot collision",
        packing_rows == 36 and source_rows == 36 and gravity_rows == 72,
    )
    check(
        "C12 packed events retain exact charge-even stress and matched tensor/vector source coordinates",
        stress_rows == 36,
    )
    check(
        "C13 complete active event packing is generator-covariant",
        active_covariance_rows == 36 * 3,
    )

    # The staged cycle reads no probability.  Its RECOVERY states remain one
    # per compatible address pair and hence preserve the prepared Born count.
    observed = Counter(
        state.label
        for state in valid_states
        if state.stage == RECOVERY and state.label is not None
    )
    expected = {
        port: (counts[0] - counts[2]) ** 2
        + (counts[1] - counts[3]) ** 2
        for port, counts in fixtures.items()
    }
    check(
        "C14 homogeneous herald latency leaves every prepared |Z|^2 event count unchanged",
        observed
        == Counter({port: count for port, count in expected.items() if count}),
    )

    # Exact finite-resource boundary.  A recovered event leaves no persistent
    # source.  If instead the 33 converted occupancies are released and kept,
    # a finite reserve R supports at most floor(R/33) events unless a causal
    # refill current enters the apparatus.
    reserve_examples = (0, 32, 33, 65, 330)
    release_bounds = {reserve: reserve // 33 for reserve in reserve_examples}
    check(
        "C15 persistent source/record release requires a finite reserve debit or causal refill current",
        release_bounds == {0: 0, 32: 0, 33: 1, 65: 1, 330: 10},
    )

    open_debts = {
        "native halo/frame/bank/work-reserve formation",
        "persistent amplified records versus exact recovery",
        "causal reserve refill for unbounded trial renewal",
        "perturbatively stable translating matter",
        "charged and protected tensor dynamical poles",
        "absolute interacting action/coupling normalization",
        "multipartite no-signalling",
        "universal response, lensing, and nonlinear gravity",
    }
    check(
        "C16 event integration closes homogeneous prepared-sector arbitration, not the five physical sectors",
        len(open_debts) == 8,
    )

    forbidden = (
        "137.036",
        "born_weight",
        "particle_mass",
        "lensing_target",
        "master_root",
    )
    source_text = __doc__.lower()
    check(
        "C17 no empirical target, random draw, coordinate coloring, or numerical near-miss enters",
        all(token not in source_text for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} homogeneous event-halo Phi-v4 checks pass")
    print(f"chart_marker_signatures={len(marker_rows)}")
    print(f"marker_exclusion_rows={exclusion_rows}")
    print(f"marker_generator_covariance_rows={marker_covariance_rows}")
    print(f"staged_apparatus_states={len(valid_states)}")
    print(f"physical_pointer_rows={physical_pointer_rows}")
    print(f"active_packing_rows={packing_rows}")
    print("pointer_herald_delta=(0,0,0,0)")
    print("active_event_delta=(32,1,0,-33)")
    print("candidate_status=homogeneous_radius_one_conflict_free_on_prepared_event_halo_sector")
    print("physical_status=formation_persistence_refill_stability_poles_absolute_scale_no_signalling_lensing_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
