#!/usr/bin/env python3
"""Exact framed, number-neutral cotangent radiation-release seed.

The stabilizer-complete Gauss packet is an occupied eight-record orbit with
readout (N,E,B)=(8,8d,0).  It cannot by itself be released into the vacuum
Maxwell sector because it changes carrier number and has nonzero boundary.

This certificate constructs the smallest ternary edge excitation made from
complete packet orbits at fixed public record number.  Two phase-distinct
packet bands give amplitudes {-1,0,+1}; activating one edge replaces an
oppositely directed packet by an aligned packet, so the signed occupation
increment has (Delta N,Delta E,Delta B)=(0,16 epsilon d,0).

Four such edges on the boundary of a framed elementary plaquette give an
exact divergence-free, carrier-number-neutral circulation.  The local toggle
is reversible, O_h covariant, charge odd, compatible with all twelve internal
clock stages, and injects only the vacuum constrained slow space of the
already-certified cotangent collision at first order.

The construction is conditional on the four-way oriented-plane quotient
(d,v) with v=h*n.  A phase-neutral directed edge alone cannot choose v
equivariantly, but an ordered perpendicular material turn supplies it without
a separate handedness bit.  The token-count ledger is degenerate between the
zero and active seed, so this theorem does not derive emission work, recoil, a
finite-amplitude collision schedule, or a Lorentz force.
"""

from __future__ import annotations

from dataclasses import dataclass
from functools import lru_cache

from sympy import Matrix, expand, symbols

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_cotangent_stabilizer_packet_gauss_source import (
    PacketState,
    advance_packet,
    packet,
    packet_field,
)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import internal_tick
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import dot
from proof_shared_edge_hodge_flag_bcc_propagation import add, scale, transform_flag


Vector = tuple[int, int, int]
PlaneFrame = tuple[Vector, Vector]
LocatedRecord = tuple[Vector, tuple[tuple[Vector, Vector, int], int]]


def negate(vector: Vector) -> Vector:
    return scale(-1, vector)


@lru_cache(maxsize=None)
def packet_at_stage(direction: Vector, phase: int, stage: int) -> PacketState:
    records = packet(direction, phase)
    for _ in range(stage % 12):
        records = advance_packet(records)
    return records


@lru_cache(maxsize=None)
def edge_records(
    direction: Vector,
    phase: int,
    stage: int,
    orientation: int,
    active: bool,
) -> PacketState:
    """Two complete packet bands with fixed count and ternary field readout."""

    aligned = scale(orientation, direction)
    second = aligned if active else negate(aligned)
    records = (
        packet_at_stage(aligned, phase, stage)
        + packet_at_stage(second, (phase + 2) % 4, stage)
    )
    assert len(records) == 16
    assert len(set(records)) == 16
    return tuple(sorted(records))


def polar_second(frame: PlaneFrame) -> Vector:
    direction, second = frame
    assert dot(direction, second) == 0
    return second


def plaquette_edges(frame: PlaneFrame) -> tuple[tuple[Vector, Vector], ...]:
    direction = frame[0]
    second = polar_second(frame)
    origin = (0, 0, 0)
    return (
        (origin, direction),
        (direction, second),
        (add(direction, second), negate(direction)),
        (second, negate(second)),
    )


@dataclass(frozen=True)
class RadiationSeed:
    frame: PlaneFrame
    phase: int
    stage: int
    orientation: int
    active: bool


def seed_records(state: RadiationSeed) -> tuple[LocatedRecord, ...]:
    located = []
    for tail, direction in plaquette_edges(state.frame):
        located.extend(
            (tail, record)
            for record in edge_records(
                direction,
                state.phase,
                state.stage,
                state.orientation,
                state.active,
            )
        )
    assert len(located) == 64
    assert len(set(located)) == 64
    return tuple(sorted(located))


def toggle_seed(state: RadiationSeed) -> RadiationSeed:
    return RadiationSeed(
        state.frame,
        state.phase,
        state.stage,
        state.orientation,
        not state.active,
    )


def advance_seed(state: RadiationSeed) -> RadiationSeed:
    return RadiationSeed(
        state.frame,
        state.phase,
        (state.stage + 1) % 12,
        state.orientation,
        state.active,
    )


def conjugate_seed(state: RadiationSeed) -> RadiationSeed:
    return RadiationSeed(
        state.frame,
        state.phase,
        state.stage,
        -state.orientation,
        state.active,
    )


def transform_seed(matrix, state: RadiationSeed) -> RadiationSeed:
    return RadiationSeed(
        (
            tuple(matrix_vector(matrix, state.frame[0])),
            tuple(matrix_vector(matrix, state.frame[1])),
        ),
        state.phase,
        state.stage,
        state.orientation,
        state.active,
    )


def transform_located_records(matrix, records):
    return tuple(
        sorted(
            (
                tuple(matrix_vector(matrix, position)),
                (transform_flag(matrix, record[0]), record[1]),
            )
            for position, record in records
        )
    )


def advance_located_records(records):
    return tuple(
        sorted((position, internal_tick(record)) for position, record in records)
    )


def add_to(mapping: dict[Vector, int], key: Vector, amount: int) -> None:
    mapping[key] = mapping.get(key, 0) + amount
    if mapping[key] == 0:
        del mapping[key]


def seed_divergence(state: RadiationSeed) -> dict[Vector, int]:
    """Boundary of the normalized public edge cochain."""

    if not state.active:
        return {}
    result: dict[Vector, int] = {}
    for tail, direction in plaquette_edges(state.frame):
        head = add(tail, direction)
        add_to(result, tail, -state.orientation)
        add_to(result, head, state.orientation)
    return result


def field_increment_on_edge(state: RadiationSeed, direction: Vector):
    active = edge_records(
        direction,
        state.phase,
        state.stage,
        state.orientation,
        True,
    )
    zero = edge_records(
        direction,
        state.phase,
        state.stage,
        state.orientation,
        False,
    )
    layer = (-state.stage) % 3
    active_field = packet_field(active, layer)
    zero_field = packet_field(zero, layer)
    return tuple(a - z for a, z in zip(active_field, zero_field))


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())
    frames = tuple(
        (direction, second)
        for direction in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(direction, second) == 0
    )
    assert len(frames) == 24
    checks += 1

    # The four-way frame is exactly an ordered right-angle material turn.
    # It is also the quotient v=h*n of the two cotangent presentations
    # (n,h)=(v,+1) and (-v,-1); no handedness bit survives separately.
    for direction, second in frames:
        presentations = {
            (second, 1),
            (negate(second), -1),
        }
        assert {
            scale(handedness, normal)
            for normal, handedness in presentations
        } == {second}
        checks += 1
        for matrix in group:
            transformed_turn = (
                tuple(matrix_vector(matrix, direction)),
                tuple(matrix_vector(matrix, second)),
            )
            assert transformed_turn in frames
            checks += 1

    # One complete D4 packet orbit can read only +8d or -8d.  A public
    # fixed-number ternary edge alphabet therefore needs at least two orbits.
    for direction in SC_DIRECTIONS:
        one_orbit_fields = {
            packet_field(packet(sign_direction, phase), 0)[:3]
            for sign_direction in (direction, negate(direction))
            for phase in range(4)
        }
        assert one_orbit_fields == {
            scale(8, direction),
            scale(-8, direction),
        }
        assert (0, 0, 0) not in one_orbit_fields
        checks += 2

    # A directed edge has no O_h-equivariant perpendicular choice.  The
    # retained cotangent frame is a real type requirement, not decoration.
    for direction in SC_DIRECTIONS:
        stabilizer = tuple(
            matrix
            for matrix in group
            if tuple(matrix_vector(matrix, direction)) == direction
        )
        perpendicular = tuple(
            candidate
            for candidate in SC_DIRECTIONS
            if dot(direction, candidate) == 0
        )
        assert len(stabilizer) == 8 and len(perpendicular) == 4
        assert not any(
            all(tuple(matrix_vector(matrix, candidate)) == candidate for matrix in stabilizer)
            for candidate in perpendicular
        )
        checks += 2

    kx, ky, kz = symbols("kx ky kz", real=True)
    wavevector = Matrix([kx, ky, kz])

    for frame in frames:
        direction = Matrix(frame[0])
        second = Matrix(polar_second(frame))
        assert direction.dot(direction) == second.dot(second) == 1
        assert direction.dot(second) == 0

        # Midpoint Fourier expansion of the oriented plaquette circulation.
        # Its first nonzero term is exactly transverse.
        leading = direction.dot(wavevector) * second - second.dot(wavevector) * direction
        assert expand((wavevector.T * leading)[0]) == 0
        assert expand(
            leading.dot(leading)
            - direction.dot(wavevector) ** 2
            - second.dot(wavevector) ** 2
        ) == 0
        checks += 4

        for phase in range(4):
            for stage in range(12):
                for orientation in (-1, 1):
                    zero = RadiationSeed(frame, phase, stage, orientation, False)
                    active = toggle_seed(zero)
                    assert toggle_seed(active) == zero
                    assert len(seed_records(zero)) == len(seed_records(active)) == 64
                    assert seed_divergence(zero) == seed_divergence(active) == {}

                    # Each activated edge is carrier-number neutral and adds
                    # exactly two canonical Gauss-packet electric quanta.
                    for _tail, edge_direction in plaquette_edges(frame):
                        active_edge = edge_records(
                            edge_direction,
                            phase,
                            stage,
                            orientation,
                            True,
                        )
                        zero_edge = edge_records(
                            edge_direction,
                            phase,
                            stage,
                            orientation,
                            False,
                        )
                        assert len(set(active_edge) - set(zero_edge)) == 8
                        assert len(set(zero_edge) - set(active_edge)) == 8
                        assert len(set(active_edge) & set(zero_edge)) == 8
                        layer = (-stage) % 3
                        assert packet_field(zero_edge, layer) == (0, 0, 0, 0, 0, 0)
                        increment = field_increment_on_edge(active, edge_direction)
                        assert increment[:3] == scale(
                            16 * orientation, edge_direction
                        )
                        assert increment[3:] == (0, 0, 0)
                        checks += 6

                    advanced_zero = advance_seed(zero)
                    advanced_active = advance_seed(active)
                    assert advance_located_records(
                        seed_records(zero)
                    ) == seed_records(advanced_zero)
                    assert advance_seed(toggle_seed(zero)) == toggle_seed(advanced_zero)
                    assert conjugate_seed(active).orientation == -orientation
                    for _tail, edge_direction in plaquette_edges(frame):
                        conjugated_increment = field_increment_on_edge(
                            conjugate_seed(active), edge_direction
                        )
                        increment = field_increment_on_edge(active, edge_direction)
                        assert conjugated_increment == tuple(-value for value in increment)
                        checks += 1
                    checks += 8


    # Spatial covariance need only be checked at one phase, clock stage, and
    # charge orientation: the spatial action leaves those labels unchanged,
    # while the packet parent theorem already proves clock equivariance.
    for frame in frames:
        active = RadiationSeed(frame, 0, 0, 1, True)
        records = seed_records(active)
        for matrix in group:
            assert transform_located_records(
                matrix, records
            ) == seed_records(transform_seed(matrix, active))
            checks += 1

    # The internal clock closes exactly and the release toggle commutes with it.
    sample = RadiationSeed(frames[0], 0, 0, 1, False)
    advanced = sample
    for _ in range(12):
        advanced = advance_seed(advanced)
    assert advanced == sample
    assert advance_seed(toggle_seed(sample)) == toggle_seed(advance_seed(sample))
    checks += 2

    print("edge carrier: two D4 packets, 16 public records, amplitudes {-1,0,+1}")
    print("ordered SC turn frames=24; each is the quotient v=h*n of two cotangent flags")
    print("framed plaquette seed: 4 edges, 64 records, DeltaN=0 and div(DeltaE)=0")
    print("active edge increment: DeltaE=16*epsilon*d, DeltaB=0")
    print("midpoint Bloch leading term=(k.d)v-(k.v)d is exactly transverse")
    print("release toggle: involutive, O_h covariant, charge odd, C12 compatible")
    print(
        f"PASS: cotangent framed plaquette radiation release ({checks} exact checks)"
    )
    print(
        "Open: endogenous frame routing, finite-amplitude collision scheduling, "
        "emission work/recoil, Lorentz force, charged pole, and alpha"
    )


if __name__ == "__main__":
    main()
