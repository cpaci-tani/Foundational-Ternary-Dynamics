#!/usr/bin/env python3
"""Exact C6 Petrie matter/directional-port recoil-current vertex.

The prepared nonplanar matter route supplies the directional port's plane,
pseudoscalar, and polar propagation direction r.  This certificate adds only
the spatial origin of the manifested neutral dipole and asks whether the local
standing/outgoing port toggle can be made reciprocal.

At fixed material route phase:

    standing field, matter origin x
      <->
    outgoing field +r, matter origin x-r.

The field port remains anchored at the same interaction vertex because the
outgoing state's anchor is defined as matter_origin+r.  Both manifested
endpoints move one SC hop, exact charge continuity holds, the canonical
capacity/field energy remains two, and

    Delta x_recoil + Delta p_field = 0.

The map is an involution, fully signed-cubic covariant, and charge-conjugation
equivariant.  Delta x is an exact material displacement current relative to
the no-emission branch.  It is not yet physical momentum: no mass/dispersion
or translational kinetic-energy law has been derived, and the emitted bank is
not yet handed off to an energy-preserving Maxwell collision/streaming step.
"""

from __future__ import annotations

from dataclasses import dataclass

from proof_c6_petrie_material_clock_endogenous_directional_port import (
    PetrieMatterState,
    charge_distribution as relative_charge_distribution,
    conjugate as conjugate_matter,
    directional_port,
    ordered_triads,
    third_direction,
    transform_state as transform_matter,
)
from proof_cotangent_handed_directional_radiation_port import (
    field_momentum,
    field_norm,
    port_records,
    transform_located_records,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_shared_edge_hodge_flag_bcc_propagation import add, scale


Vector = tuple[int, int, int]


def subtract(left: Vector, right: Vector) -> Vector:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


@dataclass(frozen=True)
class RecoilVertexState:
    matter: PetrieMatterState
    origin: Vector
    outgoing: bool
    capacity: int


def matched(state: RecoilVertexState) -> bool:
    return state.capacity in (0, 1) and state.capacity + int(state.outgoing) == 1


def port_state(state: RecoilVertexState):
    port = directional_port(state.matter, state.outgoing)
    assert port.capacity == state.capacity
    return port


def port_anchor(state: RecoilVertexState) -> Vector:
    return (
        add(state.origin, third_direction(state.matter))
        if state.outgoing
        else state.origin
    )


def located_port_records(state: RecoilVertexState):
    anchor = port_anchor(state)
    return tuple(
        sorted((add(anchor, position), record) for position, record in port_records(port_state(state)))
    )


def absolute_charge_distribution(state: RecoilVertexState) -> dict[Vector, int]:
    return {
        add(state.origin, position): amount
        for position, amount in relative_charge_distribution(state.matter).items()
    }


def recoil_collision(state: RecoilVertexState) -> RecoilVertexState:
    assert matched(state)
    propagation = third_direction(state.matter)
    displacement = propagation if state.outgoing else scale(-1, propagation)
    output = RecoilVertexState(
        state.matter,
        add(state.origin, displacement),
        not state.outgoing,
        1 - state.capacity,
    )
    assert matched(output)
    assert port_anchor(output) == port_anchor(state)
    return output


def material_displacement(state: RecoilVertexState) -> Vector:
    return subtract(recoil_collision(state).origin, state.origin)


def field_momentum_change(state: RecoilVertexState):
    output = recoil_collision(state)
    before = field_momentum(port_state(state))
    after = field_momentum(port_state(output))
    return tuple(after[index] - before[index] for index in range(3))


def total_dimensionless_energy(state: RecoilVertexState):
    return state.capacity + field_norm(port_state(state))


def endpoint_transport_current(state: RecoilVertexState):
    displacement = material_displacement(state)
    return tuple(
        (position, displacement, amount)
        for position, amount in absolute_charge_distribution(state).items()
    )


def chain_boundary(current) -> dict[Vector, int]:
    result: dict[Vector, int] = {}
    for tail, direction, amount in current:
        head = add(tail, direction)
        result[tail] = result.get(tail, 0) - amount
        result[head] = result.get(head, 0) + amount
    return {site: amount for site, amount in result.items() if amount}


def subtract_distributions(after, before) -> dict[Vector, int]:
    sites = set(after) | set(before)
    return {
        site: after.get(site, 0) - before.get(site, 0)
        for site in sites
        if after.get(site, 0) != before.get(site, 0)
    }


def conjugate(state: RecoilVertexState) -> RecoilVertexState:
    return RecoilVertexState(
        conjugate_matter(state.matter), state.origin, state.outgoing, state.capacity
    )


def transform_state(matrix, state: RecoilVertexState) -> RecoilVertexState:
    return RecoilVertexState(
        transform_matter(matrix, state.matter),
        tuple(matrix_vector(matrix, state.origin)),
        state.outgoing,
        state.capacity,
    )


def main() -> None:
    checks = 0
    triads = ordered_triads()
    group = tuple(signed_permutation_matrices())
    origins = ((0, 0, 0), (2, -1, 3), (-4, 5, 1))

    states = tuple(
        RecoilVertexState(
            PetrieMatterState(triad, route_phase, stage, orientation),
            origin,
            outgoing,
            1 - int(outgoing),
        )
        for triad in triads
        for route_phase in range(6)
        for stage in range(12)
        for orientation in (-1, 1)
        for origin in origins
        for outgoing in (False, True)
    )
    assert len(states) == 41_472
    checks += 1

    images = []
    for state in states:
        assert matched(state)
        output = recoil_collision(state)
        images.append(output)
        assert recoil_collision(output) == state
        assert port_anchor(output) == port_anchor(state)
        assert len(located_port_records(state)) == len(located_port_records(output)) == 16
        assert total_dimensionless_energy(state) == total_dimensionless_energy(output) == 2

        delta_material = material_displacement(state)
        delta_field = field_momentum_change(state)
        assert add(delta_material, delta_field) == (0, 0, 0)
        assert delta_material == (
            third_direction(state.matter)
            if state.outgoing
            else scale(-1, third_direction(state.matter))
        )
        assert subtract_distributions(
            absolute_charge_distribution(output),
            absolute_charge_distribution(state),
        ) == chain_boundary(endpoint_transport_current(state))
        assert sum(absolute_charge_distribution(state).values()) == 0
        assert sum(absolute_charge_distribution(output).values()) == 0
        checks += 10

        conjugated = conjugate(state)
        assert recoil_collision(conjugated) == conjugate(output)
        assert absolute_charge_distribution(conjugated) == {
            site: -amount
            for site, amount in absolute_charge_distribution(state).items()
        }
        assert material_displacement(conjugated) == delta_material
        assert field_momentum_change(conjugated) == delta_field
        checks += 4

    assert len(set(images)) == len(states)
    checks += 1

    # Spatial covariance at one origin and cotangent stage; translation and
    # the parent port theorem cover the spectator labels.
    for triad in triads:
        for route_phase in range(6):
            for orientation in (-1, 1):
                for outgoing in (False, True):
                    state = RecoilVertexState(
                        PetrieMatterState(triad, route_phase, 0, orientation),
                        (0, 0, 0),
                        outgoing,
                        1 - int(outgoing),
                    )
                    for matrix in group:
                        transformed = transform_state(matrix, state)
                        assert recoil_collision(transformed) == transform_state(
                            matrix, recoil_collision(state)
                        )
                        assert material_displacement(transformed) == tuple(
                            matrix_vector(matrix, material_displacement(state))
                        )
                        assert field_momentum_change(transformed) == tuple(
                            matrix_vector(matrix, field_momentum_change(state))
                        )
                        assert transform_located_records(
                            matrix, located_port_records(state)
                        ) == located_port_records(transformed)
                        checks += 4

    print("local vertex: standing matter@x <-> outgoing(+r) matter@(x-r)")
    print("field-port anchor and sixteen-record count are invariant")
    print("both manifested endpoints move one SC hop with exact charge continuity")
    print("canonical capacity+field energy remains two")
    print("recoil displacement current + field Poynting change = 0")
    print("collision is involutive, O_h covariant, and charge-conjugation equivariant")
    print(
        f"PASS: C6 Petrie directional-port recoil-current vertex ({checks} exact checks)"
    )
    print(
        "Boundary: displacement current is not yet a mass-normalized physical "
        "momentum; derive translation dispersion/kinetic energy and emitted-bank "
        "handoff before claiming Lorentz force"
    )


if __name__ == "__main__":
    main()
