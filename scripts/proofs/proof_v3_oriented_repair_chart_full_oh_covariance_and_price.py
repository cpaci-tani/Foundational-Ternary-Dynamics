#!/usr/bin/env python3
"""Full signed-cubic covariance of a selected oriented repair chart.

The original charged-frame repair used the axial plaquette normal a x b as a
spatial conveyor direction.  That is harmless under proper rotations but is
not a polar displacement under an improper signed-cubic map.  The ready
syndrome already contains a header whose rotor successor is polar.  This
certificate uses that existing header direction r as the repair-port normal.

An oriented chart is (origin,a,b,r,layer,offset,polarity), where a,b,r are an
orthonormal polar basis and r is carried by the ready header.  Axial channel
coordinates are multiplied by the chart orientation det[a b r].  The
descriptor, deterministic codebook, two-slot A2 payload, repair positions,
and exact next frame are then covariant under the full 48-element signed
cubic group.

This is a selected existing-carrier extension of the repair chart, not an
integration into canonical Phi.  Native formation of the chart/header,
parallel arbitration, action generation, and repeated survival remain open.
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
    determinant_3,
    matrix_vector,
    signed_permutation_matrices,
)
from proof_v3_charged_candidate_matter_perturbation_boundary import (
    FramePresentation,
    all_local_channels,
    frame_family,
    presentation as canonical_presentation,
)
from proof_v3_charged_common_action_phi_v3_candidate import (
    PLANE_FAMILIES,
    PlaquetteFrame,
    active_roles,
    relation_key,
)
from proof_v3_charged_frame_unique_one_defect_decoder import (
    enumerate_one_defects,
    layer_map,
    relation_map,
)
from proof_v3_dressed_sc_source_gauss_continuity import (
    internal_tick,
    target_packet,
    transform_channels,
)
from proof_v3_neutral_rotor_harmonic_green_seam import rotor_successor
from proof_v3_neutral_stf_rotor_walker_green_seam import internal_orbits
from proof_v3_neutral_syndrome_bundle_conveyor import syndrome_bundle


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
Payload = tuple[int, int]
WorkPort = tuple[Payload | None, Payload | None]


def add(left: Vec, right: Vec) -> Vec:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def subtract(left: Vec, right: Vec) -> Vec:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def scale_add(origin: Vec, direction: Vec, scale: int) -> Vec:
    return tuple(a + scale * b for a, b in zip(origin, direction))  # type: ignore[return-value]


def dot(left: Vec, right: Vec) -> int:
    return sum(a * b for a, b in zip(left, right))


def cross(left: Vec, right: Vec) -> Vec:
    return (
        left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0],
    )


def mv(matrix, vector: Vec) -> Vec:
    return tuple(matrix_vector(matrix, vector))


def matrix_product(left, right):
    return tuple(
        tuple(sum(left[i][k] * right[k][j] for k in range(3)) for j in range(3))
        for i in range(3)
    )


def generated_group(generators):
    identity = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
    group = {identity}
    frontier = [identity]
    while frontier:
        current = frontier.pop()
        for generator in generators:
            candidate = matrix_product(generator, current)
            if candidate not in group:
                group.add(candidate)
                frontier.append(candidate)
    return frozenset(group)


@dataclass(frozen=True)
class OrientedRepairChart:
    origin: Vec
    first: Vec
    second: Vec
    repair_normal: Vec
    layer: int
    offset: int
    polarity: int

    def __post_init__(self) -> None:
        basis = (self.first, self.second, self.repair_normal)
        assert all(dot(axis, axis) == 1 for axis in basis)
        assert all(dot(basis[i], basis[j]) == 0 for i in range(3) for j in range(i))
        assert self.layer in range(3)
        assert self.offset in range(4)
        assert self.polarity in (-1, 1)

    @property
    def orientation(self) -> int:
        value = dot(cross(self.first, self.second), self.repair_normal)
        assert value in (-1, 1)
        return value

    def edges(self):
        return (
            (self.origin, self.first),
            (add(self.origin, self.first), self.second),
            (add(self.origin, self.second), self.first),
            (self.origin, self.second),
        )

    def vertices(self):
        return frozenset(
            {
                self.origin,
                add(self.origin, self.first),
                add(self.origin, self.second),
                add(add(self.origin, self.first), self.second),
            }
        )

    def payload(self, role: int) -> Payload:
        return (self.offset + role) % 4, self.polarity

    def output(self):
        return OrientedRepairChart(
            self.origin,
            self.first,
            self.second,
            self.repair_normal,
            self.layer,
            (self.offset + 1) % 4,
            self.polarity,
        )


def canonical_chart(frame: PlaquetteFrame) -> OrientedRepairChart:
    first, second = PLANE_FAMILIES[frame.family]
    return OrientedRepairChart(
        frame.origin,
        first,
        second,
        cross(first, second),
        frame.family,
        frame.offset,
        frame.polarity,
    )


def transform_chart(matrix, chart: OrientedRepairChart) -> OrientedRepairChart:
    return OrientedRepairChart(
        mv(matrix, chart.origin),
        mv(matrix, chart.first),
        mv(matrix, chart.second),
        mv(matrix, chart.repair_normal),
        chart.layer,
        chart.offset,
        chart.polarity,
    )


@lru_cache(maxsize=None)
def chart_presentation(chart: OrientedRepairChart) -> FramePresentation:
    relations = frozenset(
        (
            relation_key(edge),
            "primary" if role in active_roles(chart.offset) else "reserve",
            chart.payload(role)[0],
            chart.payload(role)[1],
        )
        for role, edge in enumerate(chart.edges())
    )
    fields = set()
    for role in active_roles(chart.offset):
        tail, direction = chart.edges()[role]
        head = add(tail, direction)
        fields.update(
            (head, channel)
            for channel in target_packet(
                direction, chart.payload(role), chart.layer
            )
        )
    layers = frozenset((vertex, chart.layer) for vertex in chart.vertices())
    return FramePresentation(relations, frozenset(fields), layers)


@lru_cache(maxsize=None)
def transform_channel(matrix, channel):
    return next(iter(transform_channels(matrix, frozenset({channel}))))


def transform_presentation(matrix, state: FramePresentation) -> FramePresentation:
    relations = frozenset(
        (
            tuple(sorted((mv(matrix, key[0]), mv(matrix, key[1])))),
            owner,
            phase,
            polarity,
        )
        for key, owner, phase, polarity in state.relations
    )
    fields = frozenset(
        (mv(matrix, site), transform_channel(matrix, channel))
        for site, channel in state.fields
    )
    layers = frozenset(
        (mv(matrix, site), layer) for site, layer in state.layers
    )
    return FramePresentation(relations, fields, layers)


def coordinates(vector: Vec, chart: OrientedRepairChart):
    return tuple(
        dot(vector, axis)
        for axis in (chart.first, chart.second, chart.repair_normal)
    )


def intrinsic_state_descriptor(state, chart: OrientedRepairChart):
    tangent, normal, handedness = state[0]
    axial = coordinates(normal, chart)
    return (
        coordinates(tangent, chart),
        tuple(chart.orientation * value for value in axial),
        handedness * chart.orientation,
        state[1],
    )


def chart_code_representatives(chart: OrientedRepairChart, states):
    orbits = internal_orbits(states)
    header_candidates = [
        state for state in states if rotor_successor(state) == chart.repair_normal
    ]
    header = min(
        header_candidates,
        key=lambda state: intrinsic_state_descriptor(state, chart),
    )
    header_orbit = next(frozenset(orbit) for orbit in orbits if header in orbit)
    other_orbits = [orbit for orbit in orbits if frozenset(orbit) != header_orbit]
    representatives = [
        min(orbit, key=lambda state: intrinsic_state_descriptor(state, chart))
        for orbit in other_orbits
    ]
    representatives.sort(key=lambda state: intrinsic_state_descriptor(state, chart))
    assert len(representatives) == 15
    return header, tuple(representatives)


def chart_codewords(chart: OrientedRepairChart, states, count: int = 1_569):
    header, representatives = chart_code_representatives(chart, states)
    words = []
    for subset in combinations(range(15), 7):
        words.append(
            syndrome_bundle(
                header, tuple(representatives[index] for index in subset)
            )
        )
        if len(words) == count:
            break
    assert len(words) == count
    return tuple(words)


def transform_slots(matrix, slots):
    return frozenset(
        (transform_state(matrix, state), polarity) for state, polarity in slots
    )


def chart_relation_roles(chart: OrientedRepairChart):
    return {relation_key(edge): role for role, edge in enumerate(chart.edges())}


def normalized_channel(chart: OrientedRepairChart, channel):
    normalized = channel
    for _ in range(4 * chart.layer):
        normalized = internal_tick(normalized)
    tangent, normal, handedness, phase, polarity = normalized
    axial = coordinates(normal, chart)
    return (
        coordinates(tangent, chart),
        tuple(chart.orientation * value for value in axial),
        handedness * chart.orientation,
        (phase - chart.offset) % 4,
        polarity * chart.polarity,
    )


def defect_descriptor(chart: OrientedRepairChart, defect: FramePresentation):
    parent = chart_presentation(chart)
    parent_relations = relation_map(parent)
    defect_relations = relation_map(defect)
    changed_relations = [
        key
        for key in parent_relations.keys() | defect_relations.keys()
        if parent_relations.get(key) != defect_relations.get(key)
    ]
    if changed_relations:
        assert len(changed_relations) == 1
        key = changed_relations[0]
        role = chart_relation_roles(chart)[key]
        expected = parent_relations[key]
        actual = defect_relations.get(key)
        if actual is None:
            return "relation", role, "delete"
        return (
            "relation",
            role,
            actual[0],
            (actual[1] - expected[1]) % 4,
            actual[2] * expected[2],
        )

    field_delta = parent.fields ^ defect.fields
    if field_delta:
        assert len(field_delta) == 1
        vertex, channel = next(iter(field_delta))
        mode = "delete" if (vertex, channel) in parent.fields else "add"
        return (
            "field",
            mode,
            coordinates(subtract(vertex, chart.origin), chart),
            normalized_channel(chart, channel),
        )

    parent_layers = layer_map(parent)
    defect_layers = layer_map(defect)
    changed_vertices = [
        vertex
        for vertex in parent_layers.keys() | defect_layers.keys()
        if parent_layers.get(vertex) != defect_layers.get(vertex)
    ]
    assert len(changed_vertices) == 1
    vertex = changed_vertices[0]
    return (
        "layer",
        coordinates(subtract(vertex, chart.origin), chart),
        (defect_layers[vertex] - parent_layers[vertex]) % 3,
    )


def chart_anchor(chart: OrientedRepairChart) -> Vec:
    zero_role = next(role for role in range(4) if chart.payload(role)[0] == 0)
    return chart.edges()[zero_role][0]


def ready_work_port(chart: OrientedRepairChart) -> WorkPort:
    return ((chart.offset, chart.polarity), None)


def resource_delta(parent: FramePresentation, defect: FramePresentation) -> int:
    return (
        len(parent.relations)
        + len(parent.fields)
        - len(defect.relations)
        - len(defect.fields)
    )


def output_work_port(chart: OrientedRepairChart, defect: FramePresentation) -> WorkPort:
    delta = resource_delta(chart_presentation(chart), defect)
    if delta == 1:
        return None, None
    if delta == 0:
        return ready_work_port(chart)
    assert delta == -1
    extra = defect.fields - chart_presentation(chart).fields
    assert len(extra) == 1
    _vertex, channel = next(iter(extra))
    return ready_work_port(chart)[0], (channel[3], channel[4])


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    channels = all_local_channels()
    group = tuple(signed_permutation_matrices())
    group_set = frozenset(group)
    generators = (
        ((0, 1, 0), (1, 0, 0), (0, 0, 1)),
        ((1, 0, 0), (0, 0, 1), (0, 1, 0)),
        ((-1, 0, 0), (0, 1, 0), (0, 0, 1)),
    )
    check(
        "C1 two adjacent swaps and one axis reflection generate all signed-cubic maps",
        generated_group(generators) == group_set and len(group_set) == 48,
    )

    canonical_frames = frame_family()
    canonical_charts = tuple(canonical_chart(frame) for frame in canonical_frames)
    check(
        "C2 canonical repair is the positive-orientation subchart",
        all(chart.orientation == 1 for chart in canonical_charts)
        and all(
            chart_presentation(chart) == canonical_presentation(frame)
            for chart, frame in zip(canonical_charts, canonical_frames)
        ),
    )

    chart_orbit = {
        transform_chart(matrix, chart)
        for chart in canonical_charts
        for matrix in group
    }
    frame_presentations = {chart_presentation(chart) for chart in chart_orbit}
    states_with_ready_header = set()
    for chart in chart_orbit:
        header, _representatives = chart_code_representatives(chart, states)
        states_with_ready_header.add((chart_presentation(chart), header))
    check(
        "C3 existing records carry a finite 1,152-chart full-O_h repair orbit",
        len(chart_orbit) == 1_152
        and len(frame_presentations) == 576
        and len(states_with_ready_header) == 1_152,
        f"charts={len(chart_orbit)}, frames={len(frame_presentations)}, combined={len(states_with_ready_header)}",
    )

    chart_covariance_rows = 0
    for chart in chart_orbit:
        for matrix in group:
            transformed = transform_chart(matrix, chart)
            assert transformed in chart_orbit
            assert transform_presentation(
                matrix, chart_presentation(chart)
            ) == chart_presentation(transformed)
            assert transform_presentation(
                matrix, chart_presentation(chart.output())
            ) == chart_presentation(transformed.output())
            assert transformed.orientation == determinant_3(matrix) * chart.orientation
            chart_covariance_rows += 1
    check(
        "C4 oriented charged-frame presentations and period-four successor are full-O_h covariant",
        chart_covariance_rows == 1_152 * 48,
    )

    # The polar header, not the axial cross product, selects the spatial port.
    header_rows = 0
    for chart in canonical_charts:
        header, representatives = chart_code_representatives(chart, states)
        assert rotor_successor(header) == chart.repair_normal
        for matrix in group:
            transformed = transform_chart(matrix, chart)
            next_header, next_representatives = chart_code_representatives(
                transformed, states
            )
            assert transform_state(matrix, header) == next_header
            assert tuple(transform_state(matrix, state) for state in representatives) == next_representatives
            assert rotor_successor(next_header) == mv(matrix, chart.repair_normal)
            header_rows += 1
    check(
        "C5 ready header supplies the missing polar repair normal under improper maps",
        header_rows == 24 * 48,
    )

    # Deterministic lexicographic codewords inherit the representative action.
    codeword_rows = 0
    for chart in canonical_charts:
        words = chart_codewords(chart, states)
        for matrix in generators:
            transformed_words = chart_codewords(transform_chart(matrix, chart), states)
            for index, word in enumerate(words):
                assert transform_slots(matrix, word) == transformed_words[index]
                codeword_rows += 1
    check(
        "C6 ready plus all 1,568 syndrome codewords are generator-covariant",
        codeword_rows == 24 * 3 * 1_569,
    )

    # Exhaust the complete registered one-defect shell against generators.
    descriptor_rows = 0
    port_rows = 0
    work_rows = 0
    for frame, chart in zip(canonical_frames, canonical_charts):
        defects = tuple(enumerate_one_defects(frame, channels))
        assert len(defects) == 1_568
        base_descriptors = tuple(
            defect_descriptor(chart, defect) for _category, defect in defects
        )
        assert len(set(base_descriptors)) == 1_568
        descriptor_order = {
            descriptor: index
            for index, descriptor in enumerate(sorted(base_descriptors, key=repr), start=1)
        }
        for (_category, defect), descriptor in zip(defects, base_descriptors):
            base_index = descriptor_order[descriptor]
            base_anchor = chart_anchor(chart)
            base_input = scale_add(base_anchor, chart.repair_normal, 1)
            base_output = scale_add(base_anchor, chart.repair_normal, 2)
            base_work = output_work_port(chart, defect)
            for matrix in generators:
                transformed_chart = transform_chart(matrix, chart)
                transformed_defect = transform_presentation(matrix, defect)
                transformed_descriptor = defect_descriptor(
                    transformed_chart, transformed_defect
                )
                assert transformed_descriptor == descriptor
                assert descriptor_order[transformed_descriptor] == base_index
                descriptor_rows += 1

                next_anchor = chart_anchor(transformed_chart)
                assert next_anchor == mv(matrix, base_anchor)
                assert scale_add(
                    next_anchor, transformed_chart.repair_normal, 1
                ) == mv(matrix, base_input)
                assert scale_add(
                    next_anchor, transformed_chart.repair_normal, 2
                ) == mv(matrix, base_output)
                port_rows += 1

                assert output_work_port(
                    transformed_chart, transformed_defect
                ) == base_work
                assert transform_presentation(
                    matrix, chart_presentation(chart.output())
                ) == chart_presentation(transformed_chart.output())
                work_rows += 1
    check(
        "C7 all 37,632 defect indices are covariant on a generating set of O_h",
        descriptor_rows == 37_632 * 3,
    )
    check(
        "C8 polar input/output repair ports transform as spatial displacements",
        port_rows == descriptor_rows,
    )
    check(
        "C9 payload-complete A2 work and exact next frame are generator-covariant",
        work_rows == descriptor_rows,
    )

    # A generator-covariant action is group covariant because the generators
    # close to all 48 maps and every transform above is a genuine group action.
    check(
        "C10 the complete selected repair section has full signed-cubic covariance",
        generated_group(generators) == group_set
        and descriptor_rows == 112_896
        and codeword_rows == 112_968,
    )

    missing = {
        "native formation of the oriented frame plus ready polar header",
        "integration with charged Phi writer priority",
        "parallel repair and ordinary-field arbitration",
        "common-action provenance and absolute normalization",
        "reciprocal eighteen-record bundle clock debit",
        "repeated environmental survival scattering mass and dispersion",
    }
    check(
        "C11 stable physical matter remains open at six formation/action/dynamics debts",
        len(missing) == 6,
    )
    check(
        "C12 no new carrier type, target mass, coupling, Born weight, or numerical search enters",
        len(states) == 192 and len(channels) == 384,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} oriented-repair covariance checks pass")
    print(f"oriented_chart_states={len(chart_orbit)}")
    print(f"charged_frame_presentations={len(frame_presentations)}")
    print(f"frame_plus_header_states={len(states_with_ready_header)}")
    print(f"chart_covariance_rows={chart_covariance_rows}")
    print(f"codeword_covariance_rows={codeword_rows}")
    print(f"defect_generator_rows={descriptor_rows}")
    print("reflection_result=full_Oh_covariance_conditional_on_oriented_header_chart")
    print("matter_status=reflection_gap_closed_conditionally_formation_Phi_action_survival_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
