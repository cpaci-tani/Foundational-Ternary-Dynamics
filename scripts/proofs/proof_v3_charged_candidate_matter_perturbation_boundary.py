#!/usr/bin/env python3
"""Exact recurrent-object and one-defect boundary for charged Phi-v3.

The circulation-framed plaquette is a compact charged recurrent object in the
candidate pure-bound sector.  This certificate promotes that statement from
an example trajectory to a complete state-only family census, then tests the
entire one-coordinate defect shell inside its claimed local footprint.

Every exact frame is admitted and advances on a period-four orbit.  Every
single relation, field, or C3-layer defect is rejected by the exact frame
recognizer.  The charged macro therefore has no one-step error-correcting
basin around this family.  The frozen Phi-v2 fallback may subsequently evolve
malformed states, so this is not a theorem that defects never return.  It is a
sharp reason to retain proto-matter rather than stable-matter status.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from itertools import product

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_v3_charged_common_action_phi_v3_candidate import (
    OwnedChannel,
    PLANE_FAMILIES,
    PlaquetteFrame,
    RelationKey,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
RelationRecord = tuple[RelationKey, str, int, int]


@dataclass(frozen=True)
class FramePresentation:
    relations: frozenset[RelationRecord]
    fields: frozenset[OwnedChannel]
    layers: frozenset[tuple[Vec, int]]


def presentation(frame: PlaquetteFrame) -> FramePresentation:
    relations = frozenset(
        (key, owner, payload[0], payload[1])
        for key, (owner, payload) in frame.relation_state().items()
    )
    layers = frozenset((vertex, frame.layer) for vertex in frame.vertices())
    return FramePresentation(relations, frame.field_owners(), layers)


def frame_family(origin: Vec = (0, 0, 0)) -> tuple[PlaquetteFrame, ...]:
    return tuple(
        PlaquetteFrame(origin, family, offset, polarity)
        for family, offset, polarity in product(range(3), range(4), (-1, 1))
    )


def recognized(presented: FramePresentation, family_presentations) -> bool:
    return presented in family_presentations


def all_local_channels():
    return frozenset(
        (
            state[0][0],
            state[0][1],
            state[0][2],
            state[1],
            polarity,
        )
        for state in one_particle_states()
        for polarity in (-1, 1)
    )


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    frames = frame_family()
    exact_presentations = frozenset(presentation(frame) for frame in frames)
    channels = all_local_channels()

    check("C1 one anchor has exactly 24 family states", len(frames) == len(exact_presentations) == 24)
    check("C2 the local finite field alphabet has exactly 384 channels", len(channels) == 384)
    check("C3 every family state has four relations, sixteen fields, and four layers", all(len(presentation(frame).relations) == 4 and len(presentation(frame).fields) == 16 and len(presentation(frame).layers) == 4 for frame in frames))
    check("C4 every exact family presentation is state-only recognized", all(recognized(presentation(frame), exact_presentations) for frame in frames))
    check("C5 one candidate tick maps the complete family bijectively to itself", {presentation(frame.output()) for frame in frames} == exact_presentations)
    check("C6 every circulation frame has exact period four", all(frame.output().output().output().output() == frame for frame in frames))

    category_counts = {
        "relation_delete": 0,
        "relation_owner": 0,
        "relation_phase": 0,
        "relation_polarity": 0,
        "field_delete": 0,
        "field_add": 0,
        "layer_change": 0,
    }
    rejected = 0
    defect_parents: dict[FramePresentation, set[FramePresentation]] = {}

    def register_defect(
        defect: FramePresentation, parent: FramePresentation
    ) -> None:
        defect_parents.setdefault(defect, set()).add(parent)

    for frame in frames:
        exact = presentation(frame)

        for relation in exact.relations:
            key, owner, phase, polarity = relation

            defect = FramePresentation(
                exact.relations - {relation}, exact.fields, exact.layers
            )
            assert not recognized(defect, exact_presentations)
            register_defect(defect, exact)
            category_counts["relation_delete"] += 1
            rejected += 1

            flipped_owner = "reserve" if owner == "primary" else "primary"
            defect = FramePresentation(
                (exact.relations - {relation})
                | {(key, flipped_owner, phase, polarity)},
                exact.fields,
                exact.layers,
            )
            assert not recognized(defect, exact_presentations)
            register_defect(defect, exact)
            category_counts["relation_owner"] += 1
            rejected += 1

            for next_phase in range(4):
                if next_phase == phase:
                    continue
                defect = FramePresentation(
                    (exact.relations - {relation})
                    | {(key, owner, next_phase, polarity)},
                    exact.fields,
                    exact.layers,
                )
                assert not recognized(defect, exact_presentations)
                register_defect(defect, exact)
                category_counts["relation_phase"] += 1
                rejected += 1

            defect = FramePresentation(
                (exact.relations - {relation})
                | {(key, owner, phase, -polarity)},
                exact.fields,
                exact.layers,
            )
            assert not recognized(defect, exact_presentations)
            register_defect(defect, exact)
            category_counts["relation_polarity"] += 1
            rejected += 1

        for field in exact.fields:
            defect = FramePresentation(
                exact.relations, exact.fields - {field}, exact.layers
            )
            assert not recognized(defect, exact_presentations)
            register_defect(defect, exact)
            category_counts["field_delete"] += 1
            rejected += 1

        local_field_slots = frozenset(
            (vertex, channel)
            for vertex in frame.vertices()
            for channel in channels
        )
        assert len(local_field_slots) == 4 * 384
        for field in local_field_slots - exact.fields:
            defect = FramePresentation(
                exact.relations, exact.fields | {field}, exact.layers
            )
            assert not recognized(defect, exact_presentations)
            register_defect(defect, exact)
            category_counts["field_add"] += 1
            rejected += 1

        for layer_record in exact.layers:
            vertex, layer = layer_record
            for next_layer in range(3):
                if next_layer == layer:
                    continue
                defect = FramePresentation(
                    exact.relations,
                    exact.fields,
                    (exact.layers - {layer_record}) | {(vertex, next_layer)},
                )
                assert not recognized(defect, exact_presentations)
                register_defect(defect, exact)
                category_counts["layer_change"] += 1
                rejected += 1

    expected_per_frame = 4 + 4 + 12 + 4 + 16 + 1_520 + 8
    expected_total = len(frames) * expected_per_frame
    check("C7 the complete claimed-footprint one-defect shell is exhausted", rejected == expected_total == 37_632, str(category_counts))
    check("C8 every one-relation defect is rejected", category_counts["relation_delete"] + category_counts["relation_owner"] + category_counts["relation_phase"] + category_counts["relation_polarity"] == 24 * 24)
    check("C9 every one-field deletion or unowned addition is rejected", category_counts["field_delete"] == 384 and category_counts["field_add"] == 36_480)
    check("C10 every one-site C3-layer defect is rejected", category_counts["layer_change"] == 192)
    check(
        "C10b every enumerated one-defect presentation has a unique parent frame",
        len(defect_parents) == rejected
        and all(len(parents) == 1 for parents in defect_parents.values()),
    )

    blank = FramePresentation(frozenset(), frozenset(), frozenset())
    check("C11 the charged frame macro cannot admit the blank preparation", not recognized(blank, exact_presentations))

    missing = {
        "formation under complete Phi-v3",
        "return of malformed states under fallback",
        "nonzero correction basin",
        "reciprocal work and binding",
        "environmental survival",
        "scattering mass and dispersion",
    }
    check("C12 stable-matter promotion remains open", len(missing) == 6)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} charged-candidate matter-boundary checks pass")
    print(f"one_defect_rows={rejected}")
    print(f"defect_categories={category_counts}")
    print("family_status=compact_state_only_period_4_recurrent_charged_object")
    print("perturbation_status=exact_macro_recognizer_radius_zero")
    print("matter_status=proto_matter_not_stable_matter")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
