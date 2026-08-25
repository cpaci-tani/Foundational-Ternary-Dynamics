#!/usr/bin/env python3
"""Exact C4 phase-parity half-admitted two-polarization field carrier.

The outgoing sixteen-record directional carrier already has only two internal
field polarizations: every ray satisfies B=r x E and the eight-ray readout has
rank two.  Its ungated centroid advances by r per three ticks, twice the
certified cotangent Maxwell speed 1/6.

Use the existing transported C4 phase parity as an endogenous movement clock:
records move one tangent hop on even phase and hold position on odd phase,
while their internal flag/phase clock advances every tick.  The two retained
phase bands differ by two and therefore share the same permission, remaining
co-located.  Over six ticks each ray uses each of its three tangent legs once,
so its displacement is the prior three-step BCC displacement.  The reduced
two-polarization centroid therefore advances by r per six ticks.

The selected C4-trivial/resolved-handedness field metric stays exactly
H=1, P=r/2 at every tick.  The six-tick polarization symbol is

    z_r * (z_d + z_d^-1)(z_v + z_v^-1)/4 * I_2,

so its first-order longitudinal cone is twofold degenerate with speed 1/6;
transverse leakage begins at second order.  The map is local, reversible,
O_h covariant, and its even/odd schedules are related by one C4 phase shift.

This is a finite kinematic/action target.  It does not prove that the common
action selects phase-parity admission, protect the two modes at nonlinear
collision, or supply a charged pole, Lorentz force, inertia, or alpha.
"""

from __future__ import annotations

from collections import Counter, defaultdict

from sympy import Matrix, Rational, diag, simplify, symbols

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_cotangent_handed_directional_radiation_port import (
    DirectionalPortState,
    advance_record,
    port_records,
    propagation_direction,
    ray_bank_records,
    transform_located_records,
)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick,
    layer_value,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import cross, dot
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_shared_edge_hodge_flag_bcc_propagation import (
    add,
    scale,
    three_step_displacement,
)


Vector = tuple[int, int, int]


def subtract(left: Vector, right: Vector) -> Vector:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def permission(phase: int, parity: int = 0) -> int:
    """Move on the selected C4 parity; parity choices are time translates."""

    assert parity in (0, 1)
    return int(phase % 2 == parity)


def gated_stream(records, parity: int = 0):
    output = []
    for position, record in records:
        move = permission(record[1], parity)
        output.append(
            (
                add(position, scale(move, record[0][0])),
                internal_tick(record),
            )
        )
    return tuple(sorted(output))


def inverse_gated_stream(records, parity: int = 0):
    output = []
    for position, record in records:
        previous = advance_record(record, -1)
        move = permission(previous[1], parity)
        output.append(
            (
                subtract(position, scale(move, previous[0][0])),
                previous,
            )
        )
    return tuple(sorted(output))


def selected_local_moments(records, layer: int):
    """Merge phase bands for fixed handed flag, but not distinct flags."""

    groups = defaultdict(lambda: [0] * 6)
    for position, record in records:
        key = (position, record[0])
        value = layer_value(record, layer)
        for component, entry in enumerate(value):
            groups[key][component] += entry
    return tuple(tuple(value) for value in groups.values())


def selected_energy_momentum(records, layer: int):
    energy = 0
    momentum = (0, 0, 0)
    for value in selected_local_moments(records, layer):
        electric = value[:3]
        magnetic = value[3:]
        energy += dot(electric, electric) + dot(magnetic, magnetic)
        momentum = add(momentum, cross(electric, magnetic))
    return Rational(energy, 64), tuple(
        Rational(component, 64) for component in momentum
    )


def six_tick_displacement(position, record, parity: int = 0):
    located = ((position, record),)
    for _ in range(6):
        located = gated_stream(located, parity)
    final_position, final_record = located[0]
    return subtract(final_position, position), final_record


def polarization_coordinates(bank, frame, layer: int):
    direction, second = frame
    columns = []
    readouts = []
    for _position, record in bank:
        value = layer_value(record, layer)
        electric = value[:3]
        magnetic = value[3:]
        columns.append((dot(electric, direction), dot(electric, second)))
        readouts.append(value)
    return Matrix(columns).T, Matrix.hstack(*(Matrix(value) for value in readouts))


def main() -> None:
    checks = 0

    # The two cadence choices are related by one global C4 phase shift.
    for phase in range(4):
        assert permission((phase + 1) % 4, 0) == permission(phase, 1)
        assert permission((phase + 1) % 4, 1) == permission(phase, 0)
        checks += 2

    frames = tuple(
        (direction, second)
        for direction in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(direction, second) == 0
    )
    assert len(frames) == 24
    checks += 1

    # The movement/hold choice is a phase scalar, so the gated local map
    # commutes exactly with every signed-cubic spatial transformation.
    representative_state = DirectionalPortState(
        ((1, 0, 0), (0, 1, 0)), 1, 0, 0, 1, True, 0
    )
    representative_records = port_records(representative_state)
    for matrix in signed_permutation_matrices():
        transformed = transform_located_records(matrix, representative_records)
        for parity in (0, 1):
            assert transform_located_records(
                matrix, gated_stream(representative_records, parity)
            ) == gated_stream(transformed, parity)
            checks += 1

    z_d, z_v, z_r = symbols("z_d z_v z_r", nonzero=True)
    expected_symbol_scalar = simplify(
        z_r * (z_d + 1 / z_d) * (z_v + 1 / z_v) / 4
    )

    for frame in frames:
        direction, second = frame
        for chirality in (-1, 1):
            propagation = propagation_direction(frame, chirality)
            assert cross(direction, second) == scale(chirality, propagation)

            # One phase bank already spans exactly the two outgoing Maxwell
            # polarizations B=r x E, independent of its eight ray velocities.
            bank = ray_bank_records(frame, chirality, 0, 0, 1)
            coordinates, readout = polarization_coordinates(bank, frame, 0)
            assert readout.rank() == coordinates.rank() == 2
            assert coordinates * coordinates.T == 4 * Matrix.eye(2)
            for column in range(readout.cols):
                electric = tuple(readout[row, column] for row in range(3))
                magnetic = tuple(readout[row + 3, column] for row in range(3))
                assert dot(electric, propagation) == 0
                assert dot(magnetic, propagation) == 0
                assert magnetic == cross(propagation, electric)
                checks += 3
            checks += 3

            # Six ticks use every tangent leg exactly once and shift C4 phase
            # by two. Twelve ticks restore the complete internal record.
            displacements = []
            for position, record in bank:
                displacement, six_record = six_tick_displacement(position, record)
                assert displacement == three_step_displacement(record[0])
                assert six_record[0] == record[0]
                assert six_record[1] == (record[1] + 2) % 4

                located = ((position, record),)
                for _ in range(12):
                    located = gated_stream(located)
                twelve_position, twelve_record = located[0]
                assert subtract(twelve_position, position) == scale(2, displacement)
                assert twelve_record == record
                displacements.append(displacement)
                checks += 5

            # Exact two-polarization six-tick streaming moments.
            gram = coordinates * coordinates.T
            inverse_gram = gram.inv()
            displacement_axes = tuple(
                diag(*(displacement[axis] for displacement in displacements))
                for axis in range(3)
            )
            for axis in range(3):
                first = simplify(
                    coordinates * displacement_axes[axis] * coordinates.T * inverse_gram
                )
                assert first == propagation[axis] * Matrix.eye(2)
                checks += 1
                for other in range(3):
                    second_moment = simplify(
                        coordinates
                        * displacement_axes[axis]
                        * displacement_axes[other]
                        * coordinates.T
                        * inverse_gram
                    )
                    assert second_moment == (
                        Matrix.eye(2) if axis == other else Matrix.zeros(2, 2)
                    )
                    checks += 1

            # The complete six-tick Laurent symbol is scalar on the two
            # polarizations. Coordinates are measured in the ordered frame.
            monomials = []
            for displacement in displacements:
                exponent_d = dot(displacement, direction)
                exponent_v = dot(displacement, second)
                exponent_r = dot(displacement, propagation)
                assert (abs(exponent_d), abs(exponent_v), exponent_r) == (1, 1, 1)
                monomials.append(
                    z_d**exponent_d * z_v**exponent_v * z_r**exponent_r
                )
                checks += 1
            symbol = simplify(
                coordinates * diag(*monomials) * coordinates.T * inverse_gram
            )
            assert simplify(symbol - expected_symbol_scalar * Matrix.eye(2)) == Matrix.zeros(
                2, 2
            )
            checks += 1

            # Full sixteen-record carrier: phase bands remain paired, the
            # selected energy/Poynting ledger is exact at every tick, and the
            # local map has an exact inverse. Spatial covariance was exhausted
            # above by the frame/symbol census, so the longer finite-state trace
            # uses one orbit representative while exhausting all four phase
            # origins, both charges, all C12 stages, and both time shifts.
            if frame != ((1, 0, 0), (0, 1, 0)) or chirality != 1:
                continue
            for phase in range(4):
                for orientation in (-1, 1):
                    for stage in range(12):
                        for parity in (0, 1):
                            state = DirectionalPortState(
                                frame,
                                chirality,
                                phase,
                                stage,
                                orientation,
                                True,
                                0,
                            )
                            records = port_records(state)
                            assert len(records) == 16
                            for tick in range(12):
                                layer = (-stage - tick) % 3
                                energy, momentum = selected_energy_momentum(records, layer)
                                assert energy == 1
                                assert momentum == scale(Rational(1, 2), propagation)
                                pair_histogram = Counter(
                                    Counter(
                                        (position, record[0])
                                        for position, record in records
                                    ).values()
                                )
                                assert pair_histogram == Counter({2: 8})
                                advanced = gated_stream(records, parity)
                                assert inverse_gated_stream(advanced, parity) == records
                                # Every update is a hold or one local SC hop.
                                old_by_payload = {record: position for position, record in records}
                                for new_position, new_record in advanced:
                                    previous = advance_record(new_record, -1)
                                    displacement = subtract(
                                        new_position, old_by_payload[previous]
                                    )
                                    assert displacement in ((0, 0, 0), previous[0][0])
                                    checks += 1
                                records = advanced
                                checks += 5

    print("outgoing eight-ray readout rank=2 with B=r x E")
    print("C4 phase parity admits movement on exactly one half of ticks")
    print("every ray advances its prior BCC displacement once per six ticks")
    print("selected field ledger is exactly H=1, P=r/2 at every tick")
    print("six-tick two-polarization symbol=z_r*cos(k_d)*cos(k_v)*I_2")
    print("first-order longitudinal cone: two degenerate polarizations, speed=1/6")
    print("even/odd admission schedules are one-C4-tick time translates")
    print(
        "PASS: C4 phase-parity half-admitted two-polarization carrier "
        f"({checks} exact checks)"
    )
    print(
        "Boundary: action selection/protection, nonlinear collisions, canonical momentum, "
        "charged pole, Lorentz force, inertia, and alpha remain open"
    )


if __name__ == "__main__":
    main()
