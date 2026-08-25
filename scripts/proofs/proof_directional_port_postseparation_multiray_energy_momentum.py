#!/usr/bin/env python3
"""Exact post-separation directional multi-ray energy/momentum carrier.

The outgoing handed port contains two phase-distinct copies of one
eight-record ray bank.  Collisionless cotangent streaming leaves four
internal-handed ray pairs co-located for ticks 0 and 1.  At tick 2 the eight
spatial rays separate.

This certificate solves every pairwise affine BCC trajectory intersection
exactly and proves that no distinct rays meet again for any future tick.
Because the two C4 phase bands on each ray remain co-located and have identical
phase-blind clock-matched E/B readout, the post-separation coarse field norm and Poynting
momentum are exactly constant:

    h_free = 1,
    p_free = r/2.

The eight-ray centroid advances by r every three ticks, every ray has positive
three-tick projection on r, record count is fixed, and streaming has an exact
inverse.  The port-to-free transition changes the coarse quadratic norm from
2 to 1 during separation; that missing unit is not accounted by the raw
stream.  The carrier is also eight ballistic rays, not two Maxwell modes.
"""

from __future__ import annotations

from collections import Counter, defaultdict
from fractions import Fraction

from sympy import Rational

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_cotangent_handed_directional_radiation_port import (
    DirectionalPortState,
    inverse_stream_records,
    port_records,
    propagation_direction,
    ray_bank_records,
    stream_records,
)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import layer_value
from proof_oriented_bond_plaquette_hodge_maxwell_target import cross, dot
from proof_shared_edge_hodge_flag_bcc_propagation import (
    add,
    scale,
    three_step_displacement,
    update_flag,
)


Vector = tuple[int, int, int]


def vector_sum(vectors) -> Vector:
    output = (0, 0, 0)
    for vector in vectors:
        output = add(output, vector)
    return output


def prefix_displacement(flag, residue: int) -> Vector:
    output = (0, 0, 0)
    current = flag
    for _ in range(residue):
        output = add(output, current[0])
        current = update_flag(current)
    return output


def ray_position(position: Vector, flag, tick: int) -> Vector:
    cycles, residue = divmod(tick, 3)
    return add(
        add(position, scale(cycles, three_step_displacement(flag))),
        prefix_displacement(flag, residue),
    )


def affine_collision_cycle(position_a, flag_a, position_b, flag_b, residue: int):
    offset_a = add(position_a, prefix_displacement(flag_a, residue))
    offset_b = add(position_b, prefix_displacement(flag_b, residue))
    constant = tuple(a - b for a, b in zip(offset_a, offset_b))
    slope = tuple(
        a - b
        for a, b in zip(
            three_step_displacement(flag_a), three_step_displacement(flag_b)
        )
    )
    candidate = None
    for constant_component, slope_component in zip(constant, slope):
        if slope_component == 0:
            if constant_component != 0:
                return None
            continue
        value = Fraction(-constant_component, slope_component)
        if candidate is None:
            candidate = value
        elif candidate != value:
            return None
    if candidate is None:
        return "all"
    if candidate.denominator != 1 or candidate < 0:
        return None
    return int(candidate)


def local_moments(records, tick: int):
    layer = (-tick) % 3
    moments = defaultdict(lambda: [0] * 6)
    for position, record in records:
        value = layer_value(record, layer)
        for component, entry in enumerate(value):
            moments[position][component] += entry
    return {position: tuple(value) for position, value in moments.items()}


def coarse_norm_and_momentum(records, tick: int):
    squared = 0
    momentum = (0, 0, 0)
    for value in local_moments(records, tick).values():
        electric = value[:3]
        magnetic = value[3:]
        squared += dot(electric, electric) + dot(magnetic, magnetic)
        momentum = add(momentum, cross(electric, magnetic))
    return Rational(squared, 64), tuple(
        Rational(component, 64) for component in momentum
    )


def main() -> None:
    checks = 0
    frames = tuple(
        (direction, second)
        for direction in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(direction, second) == 0
    )

    # Solve all future spatial intersections on one representative bank. C4
    # phase does not affect trajectories; O_h and charge conjugation carry the
    # result to all other port states.
    frame = ((1, 0, 0), (0, 1, 0))
    chirality = 1
    base_bank = ray_bank_records(frame, chirality, 0, 0, 1)
    rays = tuple((position, record[0]) for position, record in base_bank)
    assert len(rays) == 8
    collision_times = Counter()
    for left_index in range(len(rays)):
        for right_index in range(left_index + 1, len(rays)):
            left_position, left_flag = rays[left_index]
            right_position, right_flag = rays[right_index]
            for residue in range(3):
                solution = affine_collision_cycle(
                    left_position,
                    left_flag,
                    right_position,
                    right_flag,
                    residue,
                )
                assert solution != "all"
                if solution is not None:
                    tick = 3 * solution + residue
                    assert ray_position(left_position, left_flag, tick) == ray_position(
                        right_position, right_flag, tick
                    )
                    collision_times[tick] += 1
                checks += 2

    assert collision_times == Counter({0: 4, 1: 4})
    checks += 1

    # Directly verify the transient and stable post-separation readout for all
    # spatial frames, propagation branches, and charge orientations.
    for current_frame in frames:
        for current_chirality in (-1, 1):
            propagation = propagation_direction(current_frame, current_chirality)
            for orientation in (-1, 1):
                state = DirectionalPortState(
                    current_frame,
                    current_chirality,
                    0,
                    0,
                    orientation,
                    True,
                    0,
                )
                records = port_records(state)
                assert len(records) == 16
                for tick in range(14):
                    norm, momentum = coarse_norm_and_momentum(records, tick)
                    expected_norm = 2 if tick < 2 else 1
                    expected_momentum = tuple(
                        Rational(component, 1 if tick < 2 else 2)
                        for component in propagation
                    )
                    assert norm == expected_norm
                    assert momentum == expected_momentum
                    assert inverse_stream_records(stream_records(records)) == records
                    records = stream_records(records)
                    checks += 4

    # The two phase bands follow identical spatial trajectories. After tick 2
    # there are exactly eight sites, two records per site, forever; the exact
    # affine-intersection classifier above proves the statement beyond this
    # finite display window.
    representative = DirectionalPortState(frame, chirality, 0, 0, 1, True, 0)
    records = port_records(representative)
    for tick in range(14):
        site_histogram = Counter(
            Counter(position for position, _record in records).values()
        )
        assert site_histogram == (
            Counter({4: 4}) if tick < 2 else Counter({2: 8})
        )
        records = stream_records(records)
        checks += 1

    # Every spatial ray advances positively and the eight-ray centroid moves
    # by exactly r per three ticks.
    for current_frame in frames:
        for current_chirality in (-1, 1):
            propagation = propagation_direction(current_frame, current_chirality)
            bank = ray_bank_records(current_frame, current_chirality, 0, 0, 1)
            displacements = tuple(
                three_step_displacement(record[0]) for _position, record in bank
            )
            assert all(dot(displacement, propagation) == 1 for displacement in displacements)
            assert vector_sum(displacements) == scale(8, propagation)
            checks += 2

    print("exact pairwise ray intersections: four at tick 0, four at tick 1, none later")
    print("ticks 0-1: coarse norm=2 and Poynting=r")
    print("all ticks >=2: eight separated rays, coarse norm=1, Poynting=r/2")
    print("two C4 phase bands remain co-located and phase-blind under the current readout")
    print("every ray has positive r projection; centroid advances r per three ticks")
    print("record streaming is a strict local permutation with exact inverse")
    print(
        f"PASS: post-separation directional multi-ray carrier ({checks} exact checks)"
    )
    print(
        "Boundary: port-to-free norm loss is unaccounted and eight ballistic rays "
        "are not the two-mode Maxwell sector; derive an energy-preserving handoff "
        "and hydrodynamic mode reduction"
    )


if __name__ == "__main__":
    main()
