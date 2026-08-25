#!/usr/bin/env python3
"""Exact common material-clock, stress-feedback, and Gauss-packet transaction.

This certificate composes three already priced finite resources on one SC
bond: one A9 material carrier, one A9 stress/capacity response carrier, and
one stabilizer-complete eight-record cotangent packet.  Stress capacity admits
the material tick.  A material ownership event toggles the Gauss packet
between reserve and active ownership.  Persistent manifestation toggles the
even stress response, while the complete cotangent packet advances through
its native twelve-stage internal clock.

The resulting matched composite map is exactly invertible.  When matter is
manifested, its charge-odd phase-neutral current equals the canonically
normalized electric packet, its charge-even tensor source drives the A9
capacity response, and the packet boundary equals the endpoint charge.  Thus
manifestation, a material clock, an electromagnetic Gauss source, and
stress/capacity backpressure coexist in one finite local transaction.

The packet is still a source dressing, not a propagating Maxwell response or
Lorentz-force law.  No variational selection, cross-sector work invariant,
static pole, spin-2 dynamics, lensing, Born preparation, or alpha measurement
is claimed.
"""

from __future__ import annotations

from dataclasses import dataclass
from functools import lru_cache

from sympy import Matrix, Rational

from proof_c18_actualization_moment_source_vertex import (
    LINE_DIRECTIONS,
    LINE_DYADS,
)
from proof_c4_stress_capacity_reciprocal_feedback import (
    frobenius_squared,
    owned_token,
    persistent_sources,
    response_capacity_chart,
)
from proof_cotangent_stabilizer_packet_gauss_source import (
    PacketState,
    advance_packet,
    boundary,
    packet,
    packet_field,
)
from proof_ternary_square_phase_polarity_autonomous_clock import (
    LocalState,
    actualize,
    capacity,
    charge,
    conjugate_state,
    iterate,
    occupation,
    polarity,
    rotate_state,
    tick,
    token_count,
    valid_owned_states,
)


Vector = tuple[int, int, int]
ORIGIN: Vector = (0, 0, 0)
POSITIVE_SC_LINES: tuple[Vector, ...] = tuple(
    tuple(int(component) for component in LINE_DIRECTIONS[index])
    for index in range(3)
)  # type: ignore[assignment]


def scale(integer: int, vector: Vector) -> Vector:
    return tuple(integer * component for component in vector)  # type: ignore[return-value]


@lru_cache(maxsize=None)
def packet_orbit(direction: Vector) -> tuple[PacketState, ...]:
    records = packet(direction, 0)
    orbit = []
    for _stage in range(12):
        orbit.append(records)
        records = advance_packet(records)
    assert records == orbit[0]
    assert len(set(orbit)) == 12
    return tuple(orbit)


@dataclass(frozen=True)
class PacketCell:
    direction: Vector
    stage: int
    active: bool
    reserve: bool

    def __post_init__(self) -> None:
        assert self.direction in tuple(
            scale(sign, direction)
            for direction in POSITIVE_SC_LINES
            for sign in (-1, 1)
        )
        assert self.stage in range(12)
        assert self.active != self.reserve


def packet_records(cell: PacketCell) -> PacketState:
    return packet_orbit(cell.direction)[cell.stage]


def advance_packet_cell(cell: PacketCell, steps: int = 1) -> PacketCell:
    return PacketCell(
        cell.direction,
        (cell.stage + steps) % 12,
        cell.active,
        cell.reserve,
    )


def toggle_packet_ownership(cell: PacketCell) -> PacketCell:
    return PacketCell(cell.direction, cell.stage, cell.reserve, cell.active)


def packet_readout(cell: PacketCell) -> tuple[int, ...]:
    """Read the co-rotating layer on which the packet is pure electric."""

    return packet_field(packet_records(cell), (-cell.stage) % 3)


def active_packet_readout(cell: PacketCell) -> tuple[int, ...]:
    value = packet_readout(cell)
    return tuple(int(cell.active) * component for component in value)


def packet_token_count(_cell: PacketCell) -> int:
    return 8


@dataclass(frozen=True)
class CommonTransactionState:
    matter: LocalState
    stress: LocalState
    gauss_packet: PacketCell


def packet_for_material(
    matter: LocalState, base_direction: Vector, stage: int
) -> PacketCell:
    epsilon = polarity(owned_token(matter))
    manifested = bool(occupation(matter.link))
    return PacketCell(
        scale(epsilon, base_direction),
        stage,
        manifested,
        not manifested,
    )


def packet_matches_material(
    cell: PacketCell, matter: LocalState, base_direction: Vector
) -> bool:
    epsilon = polarity(owned_token(matter))
    return (
        cell.direction == scale(epsilon, base_direction)
        and cell.active == bool(occupation(matter.link))
        and cell.reserve != cell.active
    )


def common_step(state: CommonTransactionState) -> CommonTransactionState:
    admitted = capacity(state.stress.link)
    matter_after = tick(state.matter) if admitted else state.matter
    ownership_event = occupation(matter_after.link) != occupation(
        state.matter.link
    )
    packet_after_event = (
        toggle_packet_ownership(state.gauss_packet)
        if ownership_event
        else state.gauss_packet
    )
    source_present = occupation(matter_after.link)
    stress_kicked = actualize(state.stress) if source_present else state.stress
    return CommonTransactionState(
        matter_after,
        rotate_state(stress_kicked, 1),
        advance_packet_cell(packet_after_event, 1),
    )


def common_inverse(state: CommonTransactionState) -> CommonTransactionState:
    packet_after_event = advance_packet_cell(state.gauss_packet, -1)
    stress_kicked = rotate_state(state.stress, -1)
    source_present = occupation(state.matter.link)
    stress_before = actualize(stress_kicked) if source_present else stress_kicked
    admitted = capacity(stress_before.link)
    matter_before = iterate(state.matter, 7) if admitted else state.matter
    ownership_event = occupation(state.matter.link) != occupation(
        matter_before.link
    )
    packet_before = (
        toggle_packet_ownership(packet_after_event)
        if ownership_event
        else packet_after_event
    )
    return CommonTransactionState(matter_before, stress_before, packet_before)


def conjugate_packet(cell: PacketCell) -> PacketCell:
    return PacketCell(
        scale(-1, cell.direction),
        cell.stage,
        cell.active,
        cell.reserve,
    )


def conjugate_common(state: CommonTransactionState) -> CommonTransactionState:
    return CommonTransactionState(
        conjugate_state(state.matter),
        state.stress,
        conjugate_packet(state.gauss_packet),
    )


def endpoint_charge(state: LocalState, base_direction: Vector) -> dict[Vector, int]:
    return {
        ORIGIN: -state.left,
        base_direction: -state.right,
    }


def verify_total_map_and_sources() -> int:
    checks = 0
    local_states = valid_owned_states()

    for line_index, base_direction in enumerate(POSITIVE_SC_LINES):
        dyad = LINE_DYADS[line_index]
        states = tuple(
            CommonTransactionState(
                matter,
                stress,
                packet_for_material(matter, base_direction, stage),
            )
            for matter in local_states
            for stress in local_states
            for stage in range(12)
        )
        assert len(states) == 3072
        checks += 1

        images = []
        for state in states:
            output = common_step(state)
            images.append(output)
            assert common_inverse(output) == state
            assert common_step(common_inverse(state)) == state
            assert packet_matches_material(
                output.gauss_packet, output.matter, base_direction
            )
            assert token_count(output.matter) == token_count(state.matter) == 1
            assert token_count(output.stress) == token_count(state.stress) == 1
            assert packet_token_count(output.gauss_packet) == 8
            assert charge(output.matter) == charge(state.matter) == 0
            assert charge(output.stress) == charge(state.stress) == 0
            assert output.gauss_packet.stage == (
                state.gauss_packet.stage + 1
            ) % 12
            assert packet_records(output.gauss_packet) == advance_packet(
                packet_records(state.gauss_packet)
            )
            checks += 9

            admitted = capacity(state.stress.link)
            expected_matter = tick(state.matter) if admitted else state.matter
            ownership_event = occupation(expected_matter.link) != occupation(
                state.matter.link
            )
            assert output.matter == expected_matter
            assert (
                output.gauss_packet.active != state.gauss_packet.active
            ) == ownership_event
            assert output.gauss_packet.active == bool(
                occupation(output.matter.link)
            )
            source_present = occupation(output.matter.link)
            assert (
                capacity(output.stress.link) != capacity(state.stress.link)
            ) == bool(source_present)
            checks += 4

            current, tensor, vector_cross, tensor_cross = persistent_sources(
                output.matter, line_index
            )
            epsilon = polarity(owned_token(output.matter))
            assert current == Rational(source_present * epsilon, 9) * Matrix(
                base_direction
            )
            assert tensor == Rational(source_present, 18) * dyad
            assert vector_cross == Matrix.zeros(3, 1)
            assert tensor_cross == Matrix.zeros(3, 3)
            checks += 4

            electric_magnetic = active_packet_readout(output.gauss_packet)
            electric = Matrix(electric_magnetic[:3])
            magnetic = Matrix(electric_magnetic[3:])
            assert magnetic == Matrix.zeros(3, 1)
            assert electric == 8 * source_present * Matrix(
                output.gauss_packet.direction
            )
            assert electric / 8 == 9 * current
            checks += 3

            stress_delta = (
                response_capacity_chart(output.stress, line_index)
                - response_capacity_chart(state.stress, line_index)
            )
            if source_present:
                assert stress_delta in (dyad / 18, -dyad / 18)
                assert frobenius_squared(stress_delta) == frobenius_squared(
                    tensor
                )
                checks += 2
            else:
                assert stress_delta == Matrix.zeros(3, 3)
                checks += 1

            if source_present:
                tail = ORIGIN if epsilon == 1 else base_direction
                divergence = boundary(
                    tail, output.gauss_packet.direction, 1
                )
                assert divergence == endpoint_charge(
                    output.matter, base_direction
                )
                assert sum(divergence.values()) == 0
                checks += 2
            else:
                assert endpoint_charge(output.matter, base_direction) == {
                    ORIGIN: 0,
                    base_direction: 0,
                }
                checks += 1

            conjugated = conjugate_common(state)
            conjugated_output = common_step(conjugated)
            assert conjugated_output == conjugate_common(output)
            conjugate_current, conjugate_tensor, _vc, _tc = persistent_sources(
                conjugated_output.matter, line_index
            )
            conjugate_field = Matrix(
                active_packet_readout(conjugated_output.gauss_packet)[:3]
            )
            assert conjugate_current == -current
            assert conjugate_tensor == tensor
            assert conjugate_field == -electric
            assert conjugated_output.stress == output.stress
            checks += 5

        assert len(set(images)) == len(states)
        checks += 1
    return checks


def verify_complete_orbit_census() -> int:
    checks = 0
    local_states = valid_owned_states()
    for base_direction in POSITIVE_SC_LINES:
        unseen = {
            CommonTransactionState(
                matter,
                stress,
                packet_for_material(matter, base_direction, stage),
            )
            for matter in local_states
            for stress in local_states
            for stage in range(12)
        }
        sourced_orbits = 0
        closed_orbits = 0
        while unseen:
            start = min(unseen, key=repr)
            orbit = []
            state = start
            while state not in orbit:
                orbit.append(state)
                state = common_step(state)
            assert state == start
            assert len(orbit) == 12
            unseen -= set(orbit)
            checks += 2

            admissions = sum(capacity(item.stress.link) for item in orbit)
            sources = sum(
                occupation(common_step(item).matter.link) for item in orbit
            )
            ownership_events = sum(
                occupation(common_step(item).matter.link)
                != occupation(item.matter.link)
                for item in orbit
            )
            active_packets = sum(item.gauss_packet.active for item in orbit)
            if sources:
                assert admissions == sources == active_packets == 8
                assert ownership_events == 2
                sourced_orbits += 1
                checks += 3
            else:
                assert admissions == active_packets == ownership_events == 0
                closed_orbits += 1
                checks += 1

        assert sourced_orbits == 192
        assert closed_orbits == 64
        checks += 2
    return checks


def main() -> None:
    checks = verify_total_map_and_sources()
    checks += verify_complete_orbit_census()

    print("resources per SC cell: material A9 + stress A9 + 8-record Gauss packet")
    print("one map: stress admission -> material tick -> packet event -> stress kick -> clocks")
    print("active packet: E/8=9*j=epsilon*d, B=0, div(E)=endpoint charge")
    print("stress response: |DeltaK|_F^2=|t|_F^2=1/324 when manifested")
    print("charge conjugation: j,E reverse; t and stress response are fixed")
    print("per SC line: 192 sourced + 64 closed period-12 composite orbits")
    print(
        "PASS: C4 common material/stress/Gauss transaction "
        f"({checks} exact checks)"
    )
    print(
        "Open: variational selection, cross-sector work, propagating Maxwell "
        "response/Lorentz force, tensor pole, lensing, Born preparation, alpha"
    )


if __name__ == "__main__":
    main()
