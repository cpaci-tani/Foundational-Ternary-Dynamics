#!/usr/bin/env python3
"""Exact handed cotangent radiation port and momentum boundary.

An ordered polar SC plane (d,v) determines only the axial normal d x v.  The
reflection fixing d and v reverses every polar normal, so the plane alone
cannot select an outgoing Poynting direction under the complete signed cubic
group.  One pseudoscalar chi closes the parity type:

    r = chi (d x v)

is polar and exactly O_h covariant.

Using r, every oriented plaquette edge e has the axial magnetic direction
b=r x e.  Two internal-handed cotangent records on each of four edges form an
eight-record ray bank with E=2 eps e, B=2 eps b and three-tick centroid
displacement r.  Two phase-distinct banks give fixed public record number:

* standing: one +r bank and one -r bank;
* outgoing: two +r banks.

Both modes contain sixteen records.  In the canonical cotangent slow-space
metric, the standing mode has field norm one, the outgoing mode norm two, and
their normalized Poynting momenta are zero and r.  A complementary capacity
bit therefore gives an exact dimensionless work ledger without the previous
ad hoc 1/16 divisor.  The relative physical action scale is still not fixed.

The microscopic streaming map is a strict local permutation and every
outgoing record has positive r projection over three ticks.  The packet is
not a rigid finite-amplitude Maxwell soliton: its coarse quadratic field norm
is not proved invariant after dispersion/collision.  The existing planar
matter clock also does not own chi or a translational recoil coordinate.
"""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
from functools import lru_cache

from sympy import Rational

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_cotangent_framed_plaquette_radiation_release import plaquette_edges
from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick,
    layer_value,
)
from proof_moore_bond_capacity_type_census import (
    determinant_3,
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import cross, dot
from proof_shared_edge_hodge_flag_bcc_propagation import (
    add,
    scale,
    three_step_displacement,
    transform_flag,
)


Vector = tuple[int, int, int]
PlaneFrame = tuple[Vector, Vector]
Record = tuple[tuple[Vector, Vector, int], int]
LocatedRecord = tuple[Vector, Record]


def subtract(left: Vector, right: Vector) -> Vector:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def vector_sum(vectors) -> Vector:
    total = (0, 0, 0)
    for vector in vectors:
        total = add(total, vector)
    return total


def propagation_direction(frame: PlaneFrame, chirality: int) -> Vector:
    assert chirality in (-1, 1)
    direction, second = frame
    assert dot(direction, second) == 0
    return scale(chirality, cross(direction, second))


def magnetic_direction(propagation: Vector, electric: Vector) -> Vector:
    output = cross(propagation, electric)
    assert output in SC_DIRECTIONS
    return output


def advance_record(record: Record, ticks: int) -> Record:
    output = record
    for _ in range(ticks % 12):
        output = internal_tick(output)
    return output


@lru_cache(maxsize=None)
def ray_bank_records(
    frame: PlaneFrame,
    chirality: int,
    phase: int,
    stage: int,
    orientation: int,
) -> tuple[LocatedRecord, ...]:
    """Eight records in four edge pairs with one coarse propagation sign."""

    assert orientation in (-1, 1)
    propagation = propagation_direction(frame, chirality)
    records = []
    for tail, electric in plaquette_edges(frame):
        magnetic = magnetic_direction(propagation, electric)
        for internal_handedness in (-1, 1):
            base: Record = (
                (
                    scale(orientation, electric),
                    scale(orientation, magnetic),
                    internal_handedness,
                ),
                phase % 4,
            )
            records.append((tail, advance_record(base, stage)))
    assert len(records) == 8
    assert len(set(records)) == 8
    return tuple(sorted(records))


@dataclass(frozen=True)
class DirectionalPortState:
    frame: PlaneFrame
    chirality: int
    phase: int
    stage: int
    orientation: int
    outgoing: bool
    capacity: int


def matched(state: DirectionalPortState) -> bool:
    return state.capacity in (0, 1) and state.capacity + int(state.outgoing) == 1


def port_records(state: DirectionalPortState) -> tuple[LocatedRecord, ...]:
    """Sixteen records: counterpropagating standing or co-propagating outgoing."""

    first = ray_bank_records(
        state.frame,
        state.chirality,
        state.phase,
        state.stage,
        state.orientation,
    )
    second_chirality = state.chirality if state.outgoing else -state.chirality
    second = ray_bank_records(
        state.frame,
        second_chirality,
        (state.phase + 2) % 4,
        state.stage,
        state.orientation,
    )
    output = tuple(sorted(first + second))
    assert len(output) == 16
    assert len(set(output)) == 16
    return output


def port_step(state: DirectionalPortState) -> DirectionalPortState:
    assert matched(state)
    output = DirectionalPortState(
        state.frame,
        state.chirality,
        state.phase,
        (state.stage + 1) % 12,
        state.orientation,
        not state.outgoing,
        1 - state.capacity,
    )
    assert matched(output)
    return output


def port_inverse(state: DirectionalPortState) -> DirectionalPortState:
    assert matched(state)
    output = DirectionalPortState(
        state.frame,
        state.chirality,
        state.phase,
        (state.stage - 1) % 12,
        state.orientation,
        not state.outgoing,
        1 - state.capacity,
    )
    assert matched(output)
    return output


def local_fields(state: DirectionalPortState) -> dict[Vector, tuple[Vector, Vector]]:
    layer = (-state.stage) % 3
    accumulators: dict[Vector, list[int]] = {}
    for position, record in port_records(state):
        values = accumulators.setdefault(position, [0] * 6)
        field = layer_value(record, layer)
        for component, value in enumerate(field):
            values[component] += value
    return {
        position: (tuple(values[:3]), tuple(values[3:]))
        for position, values in accumulators.items()
    }


def add_boundary(
    result: dict[Vector, int], tail: Vector, direction: Vector, amount: int
) -> None:
    head = add(tail, direction)
    result[tail] = result.get(tail, 0) - amount
    result[head] = result.get(head, 0) + amount
    if result[tail] == 0:
        del result[tail]
    if result.get(head) == 0:
        del result[head]


def cochain_divergence(state: DirectionalPortState, component: int) -> dict[Vector, int]:
    fields = local_fields(state)
    propagation = propagation_direction(state.frame, state.chirality)
    result: dict[Vector, int] = {}
    edges = plaquette_edges(state.frame)
    for edge_index, (tail, electric_edge) in enumerate(edges):
        pair = fields[tail]
        vector = pair[component]
        cochain_direction = (
            electric_edge
            if component == 0
            else magnetic_direction(propagation, electric_edge)
        )
        # B is an axial face/dual-edge cochain, not a second primal edge at
        # the E tail.  The two propagation signs select the two staggered
        # presentations of the same dual plaquette.  For negative chirality
        # its oriented dual edge begins at the opposite primal vertex.
        cochain_tail = (
            tail if component == 0 or state.chirality == 1
            else edges[(edge_index + 2) % 4][0]
        )
        assert all(
            vector[index] == dot(vector, cochain_direction) * cochain_direction[index]
            for index in range(3)
        )
        add_boundary(
            result,
            cochain_tail,
            cochain_direction,
            dot(vector, cochain_direction),
        )
    return result


def field_norm(state: DirectionalPortState):
    squared = 0
    for electric, magnetic in local_fields(state).values():
        squared += dot(electric, electric) + dot(magnetic, magnetic)
    return Rational(squared, 64)


def field_momentum(state: DirectionalPortState):
    momentum = vector_sum(
        cross(electric, magnetic)
        for electric, magnetic in local_fields(state).values()
    )
    return tuple(Rational(component, 64) for component in momentum)


def total_dimensionless_energy(state: DirectionalPortState):
    return state.capacity + field_norm(state)


def stream_records(records: tuple[LocatedRecord, ...]) -> tuple[LocatedRecord, ...]:
    return tuple(
        sorted(
            (add(position, record[0][0]), internal_tick(record))
            for position, record in records
        )
    )


def inverse_stream_records(records: tuple[LocatedRecord, ...]) -> tuple[LocatedRecord, ...]:
    output = []
    for position, record in records:
        previous = advance_record(record, -1)
        output.append((subtract(position, previous[0][0]), previous))
    return tuple(sorted(output))


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


def transform_state(matrix, state: DirectionalPortState) -> DirectionalPortState:
    determinant = determinant_3(matrix)
    return DirectionalPortState(
        (
            tuple(matrix_vector(matrix, state.frame[0])),
            tuple(matrix_vector(matrix, state.frame[1])),
        ),
        determinant * state.chirality,
        state.phase,
        state.stage,
        state.orientation,
        state.outgoing,
        state.capacity,
    )


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

    # The plane reflection fixes the two polar frame legs but reverses every
    # polar normal.  A transverse polar emission direction therefore needs a
    # pseudoscalar.  The signed-cubic group realizes the obstruction exactly.
    for frame in frames:
        direction, second = frame
        stabilizer = tuple(
            matrix
            for matrix in group
            if tuple(matrix_vector(matrix, direction)) == direction
            and tuple(matrix_vector(matrix, second)) == second
        )
        assert len(stabilizer) == 2
        polar_normals = tuple(
            candidate
            for candidate in SC_DIRECTIONS
            if dot(candidate, direction) == dot(candidate, second) == 0
        )
        assert len(polar_normals) == 2
        assert not any(
            all(tuple(matrix_vector(matrix, candidate)) == candidate for matrix in stabilizer)
            for candidate in polar_normals
        )
        checks += 3

        for chirality in (-1, 1):
            propagation = propagation_direction(frame, chirality)
            assert propagation in SC_DIRECTIONS
            assert dot(propagation, direction) == dot(propagation, second) == 0
            for matrix in group:
                transformed = transform_state(
                    matrix,
                    DirectionalPortState(frame, chirality, 0, 0, 1, False, 1),
                )
                assert propagation_direction(
                    transformed.frame, transformed.chirality
                ) == tuple(matrix_vector(matrix, propagation))
                checks += 1
            checks += 2

    states = tuple(
        DirectionalPortState(
            frame,
            chirality,
            phase,
            stage,
            orientation,
            outgoing,
            1 - int(outgoing),
        )
        for frame in frames
        for chirality in (-1, 1)
        for phase in range(4)
        for stage in range(12)
        for orientation in (-1, 1)
        for outgoing in (False, True)
    )
    assert len(states) == 9_216
    checks += 1

    images = []
    for state in states:
        assert matched(state)
        records = port_records(state)
        fields = local_fields(state)
        propagation = propagation_direction(state.frame, state.chirality)
        assert len(records) == 16
        assert len(fields) == 4
        assert cochain_divergence(state, 0) == {}
        assert cochain_divergence(state, 1) == {}
        assert field_norm(state) == (2 if state.outgoing else 1)
        assert field_momentum(state) == (
            propagation if state.outgoing else (0, 0, 0)
        )
        assert total_dimensionless_energy(state) == 2
        output = port_step(state)
        images.append(output)
        assert port_inverse(output) == state
        assert port_step(port_inverse(state)) == state
        assert total_dimensionless_energy(output) == total_dimensionless_energy(state)
        assert field_norm(output) - field_norm(state) == -(output.capacity - state.capacity)
        expected_impulse = tuple(
            -(after - before)
            for before, after in zip(field_momentum(state), field_momentum(output))
        )
        assert add(
            tuple(field_momentum(output)[i] - field_momentum(state)[i] for i in range(3)),
            expected_impulse,
        ) == (0, 0, 0)
        assert inverse_stream_records(stream_records(records)) == records
        checks += 15

        # Charge conjugation reverses E and B while leaving energy and
        # Poynting direction invariant.
        conjugate = DirectionalPortState(
            state.frame,
            state.chirality,
            state.phase,
            state.stage,
            -state.orientation,
            state.outgoing,
            state.capacity,
        )
        conjugate_fields = local_fields(conjugate)
        assert conjugate_fields == {
            position: (scale(-1, electric), scale(-1, magnetic))
            for position, (electric, magnetic) in fields.items()
        }
        assert field_norm(conjugate) == field_norm(state)
        assert field_momentum(conjugate) == field_momentum(state)
        checks += 3

    assert len(set(images)) == len(states)
    checks += 1

    # Spatial covariance is independent of clock stage, phase band, and
    # charge sign; check both field modes and both pseudoscalar branches.
    for frame in frames:
        for chirality in (-1, 1):
            for outgoing in (False, True):
                state = DirectionalPortState(
                    frame, chirality, 0, 0, 1, outgoing, 1 - int(outgoing)
                )
                for matrix in group:
                    transformed = transform_state(matrix, state)
                    assert transform_located_records(
                        matrix, port_records(state)
                    ) == port_records(transformed)
                    expected_momentum = tuple(
                        matrix_vector(matrix, field_momentum(state))
                    )
                    assert field_momentum(transformed) == expected_momentum
                    checks += 2

    # Every ray record advances one polar-normal unit in three ticks on
    # average.  The standing port combines opposite centroids; the outgoing
    # port doubles the selected centroid.  These are microscopic streaming
    # facts, not a finite-amplitude Maxwell-energy theorem.
    for frame in frames:
        for chirality in (-1, 1):
            propagation = propagation_direction(frame, chirality)
            for orientation in (-1, 1):
                bank = ray_bank_records(frame, chirality, 0, 0, orientation)
                displacements = tuple(
                    three_step_displacement(record[0]) for _position, record in bank
                )
                assert all(dot(displacement, propagation) == 1 for displacement in displacements)
                assert vector_sum(displacements) == scale(8, propagation)
                reverse = ray_bank_records(frame, -chirality, 0, 0, orientation)
                reverse_displacements = tuple(
                    three_step_displacement(record[0]) for _position, record in reverse
                )
                assert vector_sum(reverse_displacements) == scale(-8, propagation)
                checks += 3

    # The mode swap changes eight records, retains eight, and preserves public
    # record number.  It is an emission port, not creation from a blank.
    for frame in frames:
        standing = DirectionalPortState(frame, 1, 0, 0, 1, False, 1)
        outgoing = DirectionalPortState(frame, 1, 0, 0, 1, True, 0)
        standing_records = set(port_records(standing))
        outgoing_records = set(port_records(outgoing))
        assert len(standing_records & outgoing_records) == 8
        assert len(standing_records - outgoing_records) == 8
        assert len(outgoing_records - standing_records) == 8
        checks += 3

    print("ordered-plane stabilizer: no nonzero equivariant polar normal")
    print("one pseudoscalar chi gives polar propagation r=chi*(d cross v)")
    print("standing/outgoing port: 16 records each; eight retained and eight swapped")
    print("canonical field norms: standing=1, outgoing=2; capacity+field=2")
    print("normalized Poynting: standing=0, outgoing=r")
    print("outgoing microscopic bank: exact reversible streaming, three-tick centroid=r")
    print(
        f"PASS: cotangent handed directional radiation port ({checks} exact checks)"
    )
    print(
        "Open: derive the pseudoscalar and recoil coordinate from formed matter, "
        "preserve coarse Maxwell energy under finite collision/streaming, close "
        "Lorentz response, charged pole, gravity/lensing, Born, and alpha"
    )


if __name__ == "__main__":
    main()
