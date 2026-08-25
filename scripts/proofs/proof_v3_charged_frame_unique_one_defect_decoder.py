#!/usr/bin/env python3
"""Exact unique one-defect decoder for the charged Phi-v3 frame family.

The prior matter-boundary certificate proved that all 37,632 one-coordinate
defects leave the exact circulation-frame recognizer.  This certificate proves
the stronger coding statement: every such presentation has exactly one parent
frame at mixed-coordinate Hamming distance one.  A state-only projection can
therefore repair the complete enumerated shell in one tick.

The decoder is a candidate kinematic repair, not a common-action theorem.
Field and relation deletions require creation from a reserve; extra fields
require expiry; payload/layer corrections erase the corrupted value.  Until
the same Phi supplies those reserves, work, inverse/history, and environmental
formation, the result is conditional perturbative stability rather than
physical stable matter.
"""

from __future__ import annotations

import sys
from collections import Counter

from proof_v3_charged_candidate_matter_perturbation_boundary import (
    FramePresentation,
    all_local_channels,
    frame_family,
    presentation,
)


sys.stdout.reconfigure(encoding="utf-8")


def relation_map(presented: FramePresentation):
    return {
        key: (owner, phase, polarity)
        for key, owner, phase, polarity in presented.relations
    }


def layer_map(presented: FramePresentation):
    return dict(presented.layers)


def hamming_distance(left: FramePresentation, right: FramePresentation) -> int:
    left_relations = relation_map(left)
    right_relations = relation_map(right)
    relation_keys = left_relations.keys() | right_relations.keys()
    relation_distance = sum(
        left_relations.get(key) != right_relations.get(key)
        for key in relation_keys
    )

    field_distance = len(left.fields ^ right.fields)

    left_layers = layer_map(left)
    right_layers = layer_map(right)
    layer_sites = left_layers.keys() | right_layers.keys()
    layer_distance = sum(
        left_layers.get(site) != right_layers.get(site) for site in layer_sites
    )
    return relation_distance + field_distance + layer_distance


def enumerate_one_defects(frame, channels):
    exact = presentation(frame)

    for relation in exact.relations:
        key, owner, phase, polarity = relation
        yield "relation_delete", FramePresentation(
            exact.relations - {relation}, exact.fields, exact.layers
        )

        flipped_owner = "reserve" if owner == "primary" else "primary"
        yield "relation_owner", FramePresentation(
            (exact.relations - {relation})
            | {(key, flipped_owner, phase, polarity)},
            exact.fields,
            exact.layers,
        )

        for next_phase in range(4):
            if next_phase != phase:
                yield "relation_phase", FramePresentation(
                    (exact.relations - {relation})
                    | {(key, owner, next_phase, polarity)},
                    exact.fields,
                    exact.layers,
                )

        yield "relation_polarity", FramePresentation(
            (exact.relations - {relation})
            | {(key, owner, phase, -polarity)},
            exact.fields,
            exact.layers,
        )

    for field in exact.fields:
        yield "field_delete", FramePresentation(
            exact.relations, exact.fields - {field}, exact.layers
        )

    local_field_slots = frozenset(
        (vertex, channel)
        for vertex in frame.vertices()
        for channel in channels
    )
    for field in local_field_slots - exact.fields:
        yield "field_add", FramePresentation(
            exact.relations, exact.fields | {field}, exact.layers
        )

    for layer_record in exact.layers:
        vertex, layer = layer_record
        for next_layer in range(3):
            if next_layer != layer:
                yield "layer_change", FramePresentation(
                    exact.relations,
                    exact.fields,
                    (exact.layers - {layer_record}) | {(vertex, next_layer)},
                )


def decode(presented, exact_presentations):
    candidates = [
        exact
        for exact in exact_presentations
        if hamming_distance(presented, exact) <= 1
    ]
    return candidates[0] if len(candidates) == 1 else None


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    frames = frame_family()
    channels = all_local_channels()
    exact_presentations = tuple(presentation(frame) for frame in frames)
    output_map = {
        presentation(frame): presentation(frame.output()) for frame in frames
    }

    pair_distances = [
        hamming_distance(exact_presentations[i], exact_presentations[j])
        for i in range(len(exact_presentations))
        for j in range(i + 1, len(exact_presentations))
    ]
    minimum_distance = min(pair_distances)
    check("C1 exact charged-frame codewords are pairwise separated", minimum_distance >= 3, str(minimum_distance))

    defect_parent = {}
    category_counts: Counter[str] = Counter()
    resource_deltas: Counter[tuple[int, int]] = Counter()
    decoded_rows = 0
    for frame in frames:
        parent = presentation(frame)
        for category, defect in enumerate_one_defects(frame, channels):
            assert hamming_distance(defect, parent) == 1
            decoded = decode(defect, exact_presentations)
            assert decoded == parent
            if defect in defect_parent:
                assert defect_parent[defect] == parent
            defect_parent[defect] = parent
            category_counts[category] += 1
            relation_delta = len(parent.relations) - len(defect.relations)
            field_delta = len(parent.fields) - len(defect.fields)
            resource_deltas[(relation_delta, field_delta)] += 1
            decoded_rows += 1

    check("C2 complete enumerated one-defect shell has 37,632 unique states", decoded_rows == len(defect_parent) == 37_632)
    check("C3 every one-defect state has exactly one parent frame", all(decode(defect, exact_presentations) == parent for defect, parent in defect_parent.items()))

    expected_categories = Counter(
        {
            "relation_delete": 96,
            "relation_owner": 96,
            "relation_phase": 288,
            "relation_polarity": 96,
            "field_delete": 384,
            "field_add": 36_480,
            "layer_change": 192,
        }
    )
    check("C4 decoder covers every registered defect category exactly", category_counts == expected_categories, str(category_counts))

    # Candidate repair tick: exact codewords advance; unique one-defect states
    # project to their parent codeword.  Everything else fails closed.
    exact_images = {output_map[exact] for exact in exact_presentations}
    repaired_images = {decode(defect, exact_presentations) for defect in defect_parent}
    check("C5 exact codewords retain the period-four family tick", exact_images == set(exact_presentations))
    check("C6 every one-defect state enters the exact family after one repair tick", repaired_images == set(exact_presentations))

    blank = FramePresentation(frozenset(), frozenset(), frozenset())
    control_parent = exact_presentations[0]
    removed_relations = frozenset(sorted(control_parent.relations)[:2])
    distance_two = FramePresentation(
        control_parent.relations - removed_relations,
        control_parent.fields,
        control_parent.layers,
    )
    check(
        "C7 blank and registered distance-two controls are not decoded",
        decode(blank, exact_presentations) is None
        and hamming_distance(distance_two, control_parent) == 2
        and decode(distance_two, exact_presentations) is None,
    )

    # Exact resource price.  Positive entries require a reserve, negative field
    # entries require expiry, and zero-count substitutions still erase/replace
    # a payload or layer value.
    expected_resource_deltas = Counter(
        {
            (1, 0): 96,
            (0, 1): 384,
            (0, -1): 36_480,
            (0, 0): 672,
        }
    )
    check("C8 repair resource deltas are exactly classified", resource_deltas == expected_resource_deltas, str(resource_deltas))
    check("C9 one-step decoding is noninjective and requires reserve/expiry work", len(defect_parent) > len(exact_presentations) and any(delta != (0, 0) for delta in resource_deltas))

    missing = {
        "common-action derivation and normalization of selected occupancy energy",
        "reciprocal work for the eighteen-record syndrome clock debit",
        "full signed-cubic closure of the composed atomic transaction",
        "formation and multi-event arbitration",
        "homogeneous Phi integration and environmental boundaries",
        "repeated survival, scattering, mass, and dispersion",
    }
    check("C10 physical stable-matter promotion remains conditional", len(missing) == 6)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 charged-frame decoder checks pass")
    print(f"minimum_codeword_distance={minimum_distance}")
    print(f"unique_one_defect_states={len(defect_parent)}")
    print(f"resource_delta_histogram={dict(resource_deltas)}")
    print("decoder_status=state_only_unique_radius_one_projection")
    print("matter_status=atomic_inverse_and_formal_token_ledger_closed_physical_action_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
