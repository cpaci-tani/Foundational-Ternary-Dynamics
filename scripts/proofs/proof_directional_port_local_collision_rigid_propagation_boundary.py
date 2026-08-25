#!/usr/bin/env python3
"""Exact local-collision boundary for rigid directional-port propagation.

Take one representative outgoing sixteen-record handed cotangent port at
stage zero.  A one-tick rigid propagation through a local collision followed
by the certified record stream would have to land on some translated port
state at stage one.

This certificate exhausts every target port frame, pseudoscalar, C4 phase,
charge orientation, field mode, and every translation aligning the four local
collision sites.  It first applies inverse streaming to each target and asks
whether a local collision preserving record number and the six E/B totals can
map the source to that preimage.

Exactly sixteen parameter/translation matches survive, all equivalent
in-plane re-anchoring presentations of the same outgoing plaquette.  Every
allowed translation has zero projection on the Poynting direction.  No local
field-preserving one-tick map translates the port outward.  The complete
two-record field-preserving relation can realize all sixteen in-plane
presentations, but this is refocusing/reparameterization rather than outward
transport; no specific global equivariant matching is selected here.

Full O_h, C4, charge-conjugation, and translation covariance make the chosen
source representative complete for the declared port alphabet.  The result
does not exclude multi-tick dispersive Maxwell propagation, a larger carrier,
or collisions that preserve energy without preserving E/B pairwise.
"""

from __future__ import annotations

from collections import Counter, defaultdict

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_cotangent_handed_directional_radiation_port import (
    DirectionalPortState,
    inverse_stream_records,
    port_records,
    propagation_direction,
)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import layer_value
from proof_oriented_bond_plaquette_hodge_maxwell_target import dot


Vector = tuple[int, int, int]


def records_by_position(records):
    output = defaultdict(list)
    for position, record in records:
        output[position].append(record)
    return {position: tuple(sorted(items)) for position, items in output.items()}


def field_total(records) -> tuple[int, ...]:
    output = [0] * 6
    for record in records:
        value = layer_value(record, 0)
        for component, entry in enumerate(value):
            output[component] += entry
    return tuple(output)


def pair_partitions(records):
    assert len(records) == 4
    first, second, third, fourth = records
    return (
        ((first, second), (third, fourth)),
        ((first, third), (second, fourth)),
        ((first, fourth), (second, third)),
    )


def pair_partition_signatures(records):
    return {
        tuple(sorted(field_total(pair) for pair in partition))
        for partition in pair_partitions(records)
    }


def translate_position(position: Vector, shift: Vector) -> Vector:
    return tuple(position[index] + shift[index] for index in range(3))  # type: ignore[return-value]


def subtract(left: Vector, right: Vector) -> Vector:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def main() -> None:
    checks = 0
    frames = tuple(
        (direction, second)
        for direction in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(direction, second) == 0
    )
    source = DirectionalPortState(
        ((1, 0, 0), (0, 1, 0)), 1, 0, 0, 1, True, 0
    )
    source_direction = propagation_direction(source.frame, source.chirality)
    assert source_direction == (0, 0, 1)
    source_sites = records_by_position(port_records(source))
    assert Counter(map(len, source_sites.values())) == Counter({4: 4})
    source_totals = {
        position: field_total(records) for position, records in source_sites.items()
    }
    source_pair_signatures = {
        position: pair_partition_signatures(records)
        for position, records in source_sites.items()
    }
    checks += 3

    position_alignments = set()
    field_matches = set()
    pair_matches = set()
    target_count = 0
    for frame in frames:
        for chirality in (-1, 1):
            for phase in range(4):
                for orientation in (-1, 1):
                    for outgoing in (False, True):
                        target = DirectionalPortState(
                            frame,
                            chirality,
                            phase,
                            1,
                            orientation,
                            outgoing,
                            1 - int(outgoing),
                        )
                        target_count += 1
                        preimage_sites = records_by_position(
                            inverse_stream_records(port_records(target))
                        )
                        if Counter(map(len, preimage_sites.values())) != Counter({4: 4}):
                            continue

                        for source_position in source_sites:
                            for target_position in preimage_sites:
                                shift = subtract(source_position, target_position)
                                translated = {
                                    translate_position(position, shift): records
                                    for position, records in preimage_sites.items()
                                }
                                if set(translated) != set(source_sites):
                                    continue
                                key = (target, shift)
                                position_alignments.add(key)
                                local_totals_match = all(
                                    source_totals[position]
                                    == field_total(translated[position])
                                    for position in source_sites
                                )
                                if not local_totals_match:
                                    continue
                                field_matches.add(key)
                                local_pair_match = all(
                                    source_pair_signatures[position]
                                    & pair_partition_signatures(translated[position])
                                    for position in source_sites
                                )
                                if local_pair_match:
                                    pair_matches.add(key)
                                checks += 3

    assert target_count == 768
    assert len(position_alignments) == 128
    assert len(field_matches) == 16
    assert len(pair_matches) == 16
    checks += 4

    for target, shift in field_matches:
        assert target.outgoing
        assert propagation_direction(target.frame, target.chirality) == source_direction
        assert dot(shift, source_direction) == 0
        checks += 3

    assert {
        shift for _target, shift in field_matches
    } == {(0, 0, 0), (1, 0, 0), (0, 1, 0), (1, 1, 0)}
    checks += 1

    print("source outgoing port: four sites x four records, Poynting direction +z")
    print(f"stage-one target port states exhausted={target_count}")
    print(f"position-compatible target/shift pairs={len(position_alignments)}")
    print(f"local number+E/B compatible pairs={len(field_matches)}")
    print("all compatible shifts are in-plane: shift dot Poynting=0")
    print("two-record E/B-preserving relation matches all 16 in-plane presentations")
    print(
        f"PASS: directional-port rigid local-collision boundary ({checks} exact checks)"
    )
    print(
        "Boundary: one-tick rigid outward propagation is closed for this port and "
        "local number+E/B-preserving collision class; multi-tick dispersive or "
        "larger collective carriers remain open"
    )


if __name__ == "__main__":
    main()
