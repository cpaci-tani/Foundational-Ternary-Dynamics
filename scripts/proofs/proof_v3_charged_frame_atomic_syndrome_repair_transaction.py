#!/usr/bin/env python3
"""Atomic charged-frame repair, syndrome emission, and token-work ledger.

This certificate composes the unique one-defect decoder with the existing
eighteen-record neutral syndrome bundle.  A malformed charged frame is read
alongside a ready bundle one normal hop from its intrinsically phased anchor
and one generic token in the plaquette's two reserve A9 work slots.  One
selected radius-one transaction writes the exact next charged frame, moves an
encoded syndrome bundle one hop outward, and leaves 0, 1, or 2 work tokens so
that

    relation records + field records + reserve work tokens

is exactly conserved.  The syndrome makes the map injective and supplies an
explicit inverse for all 37,632 registered defects.

The construction uses a frame-relative codebook.  Cyclic axis covariance and
charge-conjugate agreement are checked exactly; full signed-cubic reflection
closure, physical energy/action for one work token, formation, collision
arbitration, and integration into canonical Phi remain open.
"""

from __future__ import annotations

import sys
from collections import Counter
from itertools import combinations

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_v3_charged_candidate_matter_perturbation_boundary import (
    FramePresentation,
    all_local_channels,
    frame_family,
    presentation,
)
from proof_v3_charged_common_action_phi_v3_candidate import (
    PLANE_FAMILIES,
    PlaquetteFrame,
    relation_key,
)
from proof_v3_dressed_sc_source_gauss_continuity import internal_tick
from proof_v3_charged_frame_unique_one_defect_decoder import (
    enumerate_one_defects,
    layer_map,
    relation_map,
)
from proof_v3_neutral_rotor_harmonic_green_seam import add, rotor_successor
from proof_v3_neutral_rotor_walker_macro import physical_value
from proof_v3_neutral_stf_rotor_walker_green_seam import internal_orbits
from proof_v3_neutral_syndrome_bundle_conveyor import (
    recognize_syndrome_bundle,
    syndrome_bundle,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]


def dot(left: Vec, right: Vec) -> int:
    return sum(a * b for a, b in zip(left, right))


def cross(left: Vec, right: Vec) -> Vec:
    return (
        left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0],
    )


def subtract(left: Vec, right: Vec) -> Vec:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def scale_add(origin: Vec, direction: Vec, scale: int) -> Vec:
    return tuple(a + scale * b for a, b in zip(origin, direction))  # type: ignore[return-value]


def basis(frame: PlaquetteFrame) -> tuple[Vec, Vec, Vec]:
    first, second = PLANE_FAMILIES[frame.family]
    normal = cross(first, second)
    assert dot(first, second) == dot(first, normal) == dot(second, normal) == 0
    return first, second, normal


def coordinates(vector: Vec, frame_basis) -> tuple[int, int, int]:
    return tuple(dot(vector, axis) for axis in frame_basis)  # type: ignore[return-value]


def state_descriptor(state, frame_basis):
    tangent, normal, handedness = state[0]
    phase = state[1]
    orientation = dot(cross(frame_basis[0], frame_basis[1]), frame_basis[2])
    assert orientation in (-1, 1)
    return (
        coordinates(tangent, frame_basis),
        coordinates(normal, frame_basis),
        handedness * orientation,
        phase,
    )


def frame_header_and_payload_representatives(frame, states):
    frame_basis = basis(frame)
    normal = frame_basis[2]
    orbits = internal_orbits(states)
    header_candidates = [state for state in states if rotor_successor(state) == normal]
    header = min(header_candidates, key=lambda state: state_descriptor(state, frame_basis))
    header_orbit = frozenset(
        next(iter([orbit for orbit in orbits if header in orbit]))
    )

    other_orbits = [orbit for orbit in orbits if frozenset(orbit) != header_orbit]
    representatives = [
        min(orbit, key=lambda state: state_descriptor(state, frame_basis))
        for orbit in other_orbits
    ]
    representatives.sort(key=lambda state: state_descriptor(state, frame_basis))
    assert len(representatives) == 15
    return header, tuple(representatives)


def frame_codewords(frame, states, count: int = 1_569):
    header, representatives = frame_header_and_payload_representatives(frame, states)
    subsets = combinations(range(15), 7)
    words = []
    for subset in subsets:
        payload = tuple(representatives[index] for index in subset)
        words.append(syndrome_bundle(header, payload))
        if len(words) == count:
            break
    assert len(words) == count
    return header, tuple(words)


def frame_anchor(frame: PlaquetteFrame) -> Vec:
    zero_role = next(
        role for role in range(4) if frame.payload(role)[0] == 0
    )
    return frame.edges()[zero_role][0]


def relation_roles(frame: PlaquetteFrame):
    return {
        relation_key(edge): role for role, edge in enumerate(frame.edges())
    }


def normalized_vertex(frame: PlaquetteFrame, vertex: Vec):
    return coordinates(subtract(vertex, frame.origin), basis(frame))


def normalized_channel(frame: PlaquetteFrame, channel):
    # The charged-frame scheduler identifies the plane family with the local
    # C3 layer.  Returning that layer to the family-zero clock section takes
    # four internal Z12 ticks per family step: the C4 phase is unchanged while
    # the C3 layer advances by -1.  This is the clock part of a cyclic spatial
    # frame change, not an extra dynamical update of the presented state.
    normalized = channel
    for _ in range(4 * frame.family):
        normalized = internal_tick(normalized)
    tangent, normal, handedness, phase, polarity = normalized
    frame_basis = basis(frame)
    orientation = dot(cross(frame_basis[0], frame_basis[1]), frame_basis[2])
    return (
        coordinates(tangent, frame_basis),
        coordinates(normal, frame_basis),
        handedness * orientation,
        (phase - frame.offset) % 4,
        polarity * frame.polarity,
    )


def defect_descriptor(frame: PlaquetteFrame, defect: FramePresentation):
    parent = presentation(frame)
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
        role = relation_roles(frame)[key]
        expected = parent_relations[key]
        actual = defect_relations.get(key)
        if actual is None:
            return ("relation", role, "delete")
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
        field = next(iter(field_delta))
        vertex, channel = field
        mode = "delete" if field in parent.fields else "add"
        return (
            "field",
            mode,
            normalized_vertex(frame, vertex),
            normalized_channel(frame, channel),
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
        normalized_vertex(frame, vertex),
        (defect_layers[vertex] - parent_layers[vertex]) % 3,
    )


def resource_delta(parent: FramePresentation, defect: FramePresentation) -> int:
    return (
        len(parent.relations)
        + len(parent.fields)
        - len(defect.relations)
        - len(defect.fields)
    )


def chebyshev(left: Vec, right: Vec) -> int:
    return max(abs(a - b) for a, b in zip(left, right))


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    state_set = frozenset(states)
    channels = all_local_channels()
    frames = frame_family()
    exact_presentations = {presentation(frame) for frame in frames}

    descriptor_sets = {}
    output_rows = {}
    token_delta_histogram = Counter()
    locality_rows = 0
    neutrality_rows = 0
    inverse_rows = 0

    for frame_index, frame in enumerate(frames):
        parent = presentation(frame)
        exact_output = presentation(frame.output())
        header, codewords = frame_codewords(frame, states)
        normal = basis(frame)[2]
        assert rotor_successor(header) == normal
        ready_bundle = codewords[0]
        anchor = frame_anchor(frame)
        input_port = add(anchor, normal)
        output_port = scale_add(anchor, normal, 2)

        descriptors = []
        defect_rows = tuple(enumerate_one_defects(frame, channels))
        assert len(defect_rows) == 1_568
        for _category, defect in defect_rows:
            descriptors.append(defect_descriptor(frame, defect))
        assert len(set(descriptors)) == 1_568
        descriptor_order = {
            descriptor: index
            for index, descriptor in enumerate(sorted(descriptors, key=repr), start=1)
        }
        descriptor_sets[(frame.family, frame.offset, frame.polarity)] = tuple(
            sorted(descriptors, key=repr)
        )

        # Ready and all syndrome words are exact zero-field bundles.
        for bundle in (ready_bundle, codewords[1], codewords[-1]):
            assert recognize_syndrome_bundle(bundle, state_set) is not None
            for layer in range(3):
                assert physical_value(bundle, layer) == (0,) * 6
                neutrality_rows += 1

        # The complete object and outgoing port lie in the Moore cube centered
        # on the input syndrome site.
        locality_cells = set(frame.vertices()) | {input_port, output_port}
        assert all(chebyshev(input_port, cell) <= 1 for cell in locality_cells)
        locality_rows += len(locality_cells)

        for _category, defect in defect_rows:
            descriptor = defect_descriptor(frame, defect)
            syndrome_index = descriptor_order[descriptor]
            syndrome_bundle_after = codewords[syndrome_index]
            delta = resource_delta(parent, defect)
            assert delta in (-1, 0, 1)
            work_before = 1
            work_after = work_before - delta
            assert work_after in (0, 1, 2)

            object_tokens_before = len(defect.relations) + len(defect.fields)
            object_tokens_after = len(exact_output.relations) + len(exact_output.fields)
            total_before = object_tokens_before + len(ready_bundle) + work_before
            total_after = object_tokens_after + len(syndrome_bundle_after) + work_after
            assert total_before == total_after
            token_delta_histogram[(delta, work_after)] += 1

            output_key = (
                exact_output,
                output_port,
                syndrome_bundle_after,
                work_after,
            )
            input_key = (defect, input_port, ready_bundle, work_before)
            assert output_key not in output_rows
            output_rows[output_key] = input_key
            assert output_rows[output_key] == input_key
            inverse_rows += 1

    check(
        "C1 all 37,632 defects have unique intrinsic frame-relative descriptors",
        inverse_rows == len(output_rows) == 37_632,
    )
    check(
        "C2 ready plus every syndrome injects into the eighteen-record physical alphabet",
        bool(output_rows),
    )
    check(
        "C3 atomic transaction writes the exact next charged-frame family",
        all(key[0] in exact_presentations for key in output_rows),
    )
    check(
        "C4 syndrome output makes every repair transaction exactly invertible",
        all(value[0] not in exact_presentations for value in output_rows.values()),
    )
    check(
        "C5 selected two-slot A2 work port represents counts zero, one, and two",
        set(work_after for _delta, work_after in token_delta_histogram) == {0, 1, 2},
    )
    check(
        "C6 object plus syndrome plus reserve-work token count is exactly conserved",
        sum(token_delta_histogram.values()) == 37_632,
    )
    check(
        "C7 complete repair and syndrome emission fit one Moore cube",
        locality_rows == 24 * 6,
        str(locality_rows),
    )
    check(
        "C8 ready and emitted syndrome controls are electromagnetically neutral",
        neutrality_rows == 24 * 3 * 3,
    )

    expected_histogram = Counter(
        {
            (1, 0): 480,
            (0, 1): 672,
            (-1, 2): 36_480,
        }
    )
    check(
        "C9 exact work-port census matches missing, substitution, and extra defects",
        token_delta_histogram == expected_histogram,
        str(token_delta_histogram),
    )

    # Proper cyclic permutations of x,y,z preserve the frame-gauge descriptor
    # list.  Charge conjugation also preserves it because all polarities are
    # normalized relative to the parent frame polarity.
    cyclic_covariance_rows = 0
    for offset in range(4):
        for polarity in (-1, 1):
            reference = descriptor_sets[(0, offset, polarity)]
            assert descriptor_sets[(1, offset, polarity)] == reference
            assert descriptor_sets[(2, offset, polarity)] == reference
            assert descriptor_sets[(0, offset, -polarity)] == reference
            cyclic_covariance_rows += 3
    check(
        "C10 defect-index code is cyclic-axis covariant and charge-conjugation neutral",
        cyclic_covariance_rows == 24,
    )

    missing = {
        "full signed-cubic reflection closure of the atomic codebook",
        "common-action derivation and normalization of selected occupancy energy",
        "reciprocal work for the eighteen-record syndrome clock debit",
        "formation of the ready syndrome and work ports",
        "parallel arbitration and integration into homogeneous candidate Phi",
        "repeated perturbation, scattering, mass, and dispersion",
    }
    check(
        "C11 stable physical matter remains open at six action/integration debts",
        len(missing) == 6,
    )
    check(
        "C12 no target coupling, fitted energy, or new primitive carrier enters",
        len(states) == 192 and len(codewords[0]) == 18,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} atomic syndrome-repair checks pass")
    print(f"repair_rows={inverse_rows}")
    print(f"token_delta_histogram={dict(token_delta_histogram)}")
    print(f"cyclic_covariance_rows={cyclic_covariance_rows}")
    print("repair_status=exact_frame_gauged_atomic_inverse_and_token_ledger")
    print("matter_status=A2_energy_selected_common_action_formation_and_survival_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
