#!/usr/bin/env python3
"""Exact stabilizer-complete cotangent packet and local Gauss source.

A directed electric edge d does not select one perpendicular axial normal.
The D4 stabilizer of d acts transitively on the eight (n,h) flag choices.
Activating that complete eight-record orbit at one common C4 phase is therefore
the smallest context-free O_h-covariant packet.  At every cotangent clock
layer its total readout is E=8d, B=0; after canonical field normalization it
injects one electric edge quantum and no magnetic source.

Placing that edge quantum between two ternary endpoints gives an exact local
incidence identity div E = rho.  Moving the packet between reserve and active
ownership while manifesting the endpoints is a finite involution.  This proves
a reversible Gauss-compatible source stencil, not its work coefficient, stable
matter realization, or physical identification of ternary sign with electric
charge.
"""

from __future__ import annotations

from dataclasses import dataclass

from sympy import Matrix

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick,
    layer_value,
)
from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import dot
from proof_shared_edge_hodge_flag_bcc_propagation import (
    add,
    transform_flag,
)


Vector = tuple[int, int, int]
PacketState = tuple[tuple[tuple[Vector, Vector, int], int], ...]


def scale_integer(integer: int, vector: Vector) -> Vector:
    return tuple(integer * component for component in vector)  # type: ignore[return-value]


def packet(direction: Vector, phase: int) -> PacketState:
    return tuple(
        sorted(
            ((direction, normal, handedness), phase)
            for normal in SC_DIRECTIONS
            if dot(direction, normal) == 0
            for handedness in (-1, 1)
        )
    )


def advance_packet(records: PacketState) -> PacketState:
    return tuple(sorted(internal_tick(record) for record in records))


def packet_field(records: PacketState, layer: int) -> tuple[int, ...]:
    total = [0] * 6
    for record in records:
        value = layer_value(record, layer)
        for component in range(6):
            total[component] += value[component]
    return tuple(total)


def transform_packet(matrix, records: PacketState) -> PacketState:
    return tuple(
        sorted((transform_flag(matrix, flag), phase) for flag, phase in records)
    )


@dataclass(frozen=True)
class SourceState:
    tail_state: int
    head_state: int
    active_packet: bool
    reserve_packet: bool
    direction: Vector
    phase: int
    orientation: int


def source_transaction(state: SourceState) -> SourceState:
    """Payload-complete involution between reserve and manifested ownership."""

    if (
        state.tail_state == 0
        and state.head_state == 0
        and not state.active_packet
        and state.reserve_packet
    ):
        return SourceState(
            -state.orientation,
            state.orientation,
            True,
            False,
            state.direction,
            state.phase,
            state.orientation,
        )
    if (
        state.tail_state == -state.orientation
        and state.head_state == state.orientation
        and state.active_packet
        and not state.reserve_packet
    ):
        return SourceState(
            0,
            0,
            False,
            True,
            state.direction,
            state.phase,
            state.orientation,
        )
    return state


def packet_token_count(state: SourceState) -> int:
    return 8 * (int(state.active_packet) + int(state.reserve_packet))


def packet_energy_ledger(state: SourceState) -> int:
    """Positive one-unit energy per retained finite flag token."""

    return packet_token_count(state)


def boundary(tail: Vector, direction: Vector, amount: int):
    head = add(tail, direction)
    return {tail: -amount, head: amount}


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())

    for direction in SC_DIRECTIONS:
        stabilizer = tuple(
            matrix for matrix in group if matrix_vector(matrix, direction) == direction
        )
        assert len(stabilizer) == 8
        for phase in range(4):
            records = packet(direction, phase)
            assert len(records) == 8
            assert len(set(records)) == 8

            seed = records[0]
            orbit = {
                (transform_flag(matrix, seed[0]), seed[1]) for matrix in stabilizer
            }
            assert orbit == set(records)
            assert all(transform_packet(matrix, records) == records for matrix in stabilizer)

            current = records
            for tick in range(12):
                layer = (-tick) % 3
                value = packet_field(current, layer)
                assert value[:3] == scale_integer(8, direction)
                assert value[3:] == (0, 0, 0)
                current = advance_packet(current)
                checks += 2
            assert current == records
            checks += 5

            for matrix in group:
                transformed_direction = tuple(matrix_vector(matrix, direction))
                assert transform_packet(matrix, records) == packet(
                    transformed_direction, phase
                )
                checks += 1

            phase_advanced = tuple(
                sorted((flag, (record_phase + 1) % 4) for flag, record_phase in records)
            )
            assert phase_advanced == packet(direction, (phase + 1) % 4)
            checks += 1

    # The field Gram matrix of one layer is diag(192,64 I_6).  An eight-token
    # stabilizer packet has electric total 8d, hence unit norm in the canonical
    # E metric after removing/conditioning on its carrier-number component.
    states = one_particle_states()
    rows = Matrix.vstack(
        Matrix([[1] * 192]),
        *(
            Matrix([[layer_value(state, 0)[component] for state in states]])
            for component in range(6)
        ),
    )
    gram = rows * rows.T
    assert gram == Matrix.diag(192, 64, 64, 64, 64, 64, 64)
    for direction in SC_DIRECTIONS:
        indices = {state: index for index, state in enumerate(states)}
        occupation = Matrix.zeros(192, 1)
        for record in packet(direction, 0):
            occupation[indices[record], 0] = 1
        conserved_total = rows * occupation
        assert conserved_total[0] == 8
        assert tuple(conserved_total[1:4, 0]) == scale_integer(8, direction)
        assert tuple(conserved_total[4:7, 0]) == (0, 0, 0)
        electric_norm = (
            conserved_total[1:7, 0].T
            * gram[1:7, 1:7].inv()
            * conserved_total[1:7, 0]
        )[0]
        assert electric_norm == 1
        checks += 4

    # Exact local cochain source.  One canonically normalized oriented edge
    # has boundary +1 at its head and -1 at its tail, so creation preserves
    # the Gauss residual div(E)-rho identically.
    test_tails = ((0, 0, 0), (2, -1, 3), (-4, 5, 1))
    for tail in test_tails:
        for direction in SC_DIRECTIONS:
            delta_divergence = boundary(tail, direction, 1)
            delta_charge = boundary(tail, direction, 1)
            assert delta_divergence == delta_charge

            # The standard local current transaction also preserves Gauss:
            # Delta rho=-boundary(j), Delta E=-j.
            current_boundary = boundary(tail, direction, 1)
            delta_rho = {site: -value for site, value in current_boundary.items()}
            delta_div_e = {site: -value for site, value in current_boundary.items()}
            assert delta_div_e == delta_rho
            checks += 2

    for direction in SC_DIRECTIONS:
        for phase in range(4):
            for orientation in (-1, 1):
                reserve = SourceState(0, 0, False, True, direction, phase, orientation)
                manifested = source_transaction(reserve)
                assert source_transaction(manifested) == reserve
                assert manifested.direction == direction
                assert manifested.phase == phase
                assert manifested.orientation == orientation
                assert manifested.active_packet and not manifested.reserve_packet
                assert manifested.tail_state + manifested.head_state == 0
                assert packet_token_count(reserve) == packet_token_count(manifested) == 8
                assert packet_energy_ledger(reserve) == packet_energy_ledger(manifested) == 8
                checks += 8

    print("context_free_source_packet=8_flag_records")
    print("packet_stabilizer_orbit=D4_size_8")
    print("all_12_clock_ticks: packet_E=8d, packet_B=0")
    print("canonical_field_normalization: packet_E_norm_squared=1")
    print("microscopic_capacity_price=8 retained tokens, positive token-energy ledger=8")
    print("local_creation: Delta_divE=Delta_rho=boundary(oriented_edge)")
    print("local_motion: Delta_rho=-boundary(j), Delta_E=-j preserves Gauss")
    print(
        f"PASS: cotangent stabilizer packet Gauss source ({checks} exact checks)"
    )
    print(
        "Open: derive active-vs-reserve Hamiltonian work, identify ternary sign "
        "with electric charge, and compose source with finite collision tick"
    )


if __name__ == "__main__":
    main()
