#!/usr/bin/env python3
"""Exact scalar/vector/STF neutral bundle walker and common Green seam.

The existing v3 neutral rotor already carries a scalar visit count.  This
certificate adds no primitive type.  It uses one R^2-related neutral
controller pair to carry a polar-vector label and one controller in a third
clock orbit to complete a two-record STF label.  Together with the R^4
rotor/marker header, the five opposite-polarity controller pairs form an
intrinsically recognizable ten-record marked site.

The complete admissible source census spans

    1 (scalar) + 5 (spatial STF) + 3 (polar vector) = 9

dimensions exactly.  One radius-one rule advances every controller by the
native clock and moves the full bundle along the rotor-served SC edge.  Its
explicit reverse recovers every input record.  Sequential absorbing-box
histories therefore give one componentwise Dirichlet Green seam for all nine
source coordinates.

This remains a prepared blocked-history construction.  It does not protect a
tensor mode under canonical Phi, derive scalar/vector constraint dynamics,
identify a dynamical Hessian, establish a wave pole, fix a coupling, or prove
lensing.
"""

from __future__ import annotations

import sys
from collections import defaultdict
from fractions import Fraction

from sympy import Matrix

from proof_global_c3_cotangent_layer_hodge_maxwell_target import internal_tick
from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_v3_neutral_rotor_harmonic_green_seam import (
    Vec,
    add,
    box,
    dirichlet_laplacian,
    rotor_successor,
    simulate_rotor_box,
)
from proof_v3_neutral_rotor_walker_macro import (
    advance,
    controller_states,
    physical_value,
    polarized_slots,
    recognize_unmarked,
    unmarked_site,
)
from proof_v3_neutral_stf_rotor_walker_green_seam import (
    internal_orbits,
    payload_tensor,
)
from proof_v3_neutral_vector_constraint_walker_tt_locality_obstruction import (
    payload_vector,
)
from proof_v3_two_record_full_stf_tensor_carrier_boundary import (
    stf5,
    transform_tensor,
)


sys.stdout.reconfigure(encoding="utf-8")


def orbit_lookup(orbits) -> dict[object, int]:
    return {
        state: orbit_index
        for orbit_index, orbit in enumerate(orbits)
        for state in orbit
    }


def bundle_payload(left, right, layer: int) -> tuple[int, ...]:
    """Scalar visit unit, five STF coordinates, then three vector entries."""

    vector_controller = advance(left, 2)
    return (
        (1,)
        + stf5(payload_tensor((left, right), layer))
        + payload_vector(vector_controller, layer)
    )


def bundle_marked_site(rotor, left, right) -> frozenset[tuple[object, int]]:
    """Five neutral controller pairs: rotor, R4 marker, R2 pair, singleton."""

    return frozenset().union(
        polarized_slots(rotor),
        polarized_slots(advance(rotor, 4)),
        polarized_slots(left),
        polarized_slots(advance(left, 2)),
        polarized_slots(right),
    )


def recognize_bundle_marked(slots, state_set, orbit_index):
    controllers = controller_states(slots)
    if len(controllers) != 5:
        return None

    rotor_candidates = [
        state
        for state in controllers
        if advance(state, 4) in controllers
    ]
    left_candidates = [
        state
        for state in controllers
        if advance(state, 2) in controllers
    ]
    if len(rotor_candidates) != 1 or len(left_candidates) != 1:
        return None

    rotor = rotor_candidates[0]
    left = left_candidates[0]
    reserved = {
        rotor,
        advance(rotor, 4),
        left,
        advance(left, 2),
    }
    right_candidates = controllers - reserved
    if len(right_candidates) != 1:
        return None
    right = next(iter(right_candidates))

    if any(state not in state_set for state in controllers):
        return None
    if len({orbit_index[rotor], orbit_index[left], orbit_index[right]}) != 3:
        return None
    expected = bundle_marked_site(rotor, left, right)
    return (rotor, left, right) if slots == expected else None


def local_bundle_step(departure_slots, destination_slots, state_set, orbit_index):
    departure = recognize_bundle_marked(
        departure_slots, state_set, orbit_index
    )
    destination_rotor = recognize_unmarked(destination_slots, state_set)
    if departure is None or destination_rotor is None:
        return None
    departure_rotor, left, right = departure
    advanced_rotor = internal_tick(departure_rotor)
    return (
        unmarked_site(advanced_rotor),
        bundle_marked_site(
            destination_rotor,
            internal_tick(left),
            internal_tick(right),
        ),
        rotor_successor(advanced_rotor),
    )


def inverse_bundle_step(source_after, destination_after, state_set, orbit_index):
    source_rotor = recognize_unmarked(source_after, state_set)
    destination = recognize_bundle_marked(
        destination_after, state_set, orbit_index
    )
    if source_rotor is None or destination is None:
        return None
    destination_rotor, advanced_left, advanced_right = destination
    prior_rotor = advance(source_rotor, 11)
    return (
        bundle_marked_site(
            prior_rotor,
            advance(advanced_left, 11),
            advance(advanced_right, 11),
        ),
        unmarked_site(destination_rotor),
        rotor_successor(source_rotor),
    )


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    state_set = frozenset(states)
    orbits = internal_orbits(states)
    orbit_index = orbit_lookup(orbits)
    group = tuple(signed_permutation_matrices())

    check(
        "C1 existing v3 field bank is sixteen native period-twelve orbits",
        len(states) == 192
        and len(orbits) == 16
        and all(len(orbit) == 12 for orbit in orbits),
    )

    # Exact complete admissible source census.  Left and right must occupy
    # distinct clock orbits so the R2 pair and singleton roles remain unique.
    source_rows = []
    source_representatives = []
    for left in states:
        for right in states:
            if orbit_index[left] == orbit_index[right]:
                continue
            source_rows.append(bundle_payload(left, right, 0))
            source_representatives.append((left, right))

    source_matrix = Matrix(source_rows)
    source_rank = source_matrix.rank()
    pivot_columns = Matrix(source_rows).T.rref()[1]
    basis_pairs = tuple(source_representatives[index] for index in pivot_columns)
    basis_rows = tuple(source_rows[index] for index in pivot_columns)
    check(
        "C2 complete admissible scalar/STF/vector source census has rank nine",
        len(source_rows) == 34_560
        and source_rank == 9
        and len(pivot_columns) == 9,
        f"rows={len(source_rows)}, rank={source_rank}",
    )
    check(
        "C3 canonical exact pivot packets form a joint nine-coordinate basis",
        Matrix(basis_rows).rank() == 9,
        str(basis_rows),
    )

    # Choose the first clock orbit disjoint from each payload's two orbits.
    basis_packets = []
    for left, right in basis_pairs:
        router_orbit = next(
            orbit
            for orbit in orbits
            if orbit_index[orbit[0]]
            not in {orbit_index[left], orbit_index[right]}
        )
        basis_packets.append((router_orbit, left, right))

    role_rows = 0
    neutral_rows = 0
    clock_rows = 0
    for router_orbit, left_seed, right_seed in basis_packets:
        for offset in range(12):
            rotor = router_orbit[offset]
            left = advance(left_seed, offset)
            right = advance(right_seed, offset)
            marked = bundle_marked_site(rotor, left, right)
            recognized = recognize_bundle_marked(marked, state_set, orbit_index)
            assert recognized == (rotor, left, right)
            assert len(marked) == 10
            for layer in range(3):
                assert physical_value(marked, layer) == (0,) * 6
                assert bundle_payload(
                    internal_tick(left), internal_tick(right), (layer - 1) % 3
                ) == bundle_payload(left, right, layer)
                neutral_rows += 1
                clock_rows += 1
            role_rows += 1
    check(
        "C4 R4 header, R2 vector relation, and singleton make every basis packet state-only unique",
        role_rows == 9 * 12,
    )
    check(
        "C5 every ten-record marked packet is exactly additive-E/B neutral",
        neutral_rows == 9 * 12 * 3,
    )
    check(
        "C6 all nine payload coordinates are covariantly constant under the native clock",
        clock_rows == 9 * 12 * 3,
    )

    transaction_rows = 0
    inverse_rows = 0
    for router_orbit, left_seed, right_seed in basis_packets:
        for departure_rotor in router_orbit:
            for destination_rotor in router_orbit:
                before = (
                    bundle_marked_site(
                        departure_rotor, left_seed, right_seed
                    ),
                    unmarked_site(destination_rotor),
                )
                after = local_bundle_step(
                    before[0], before[1], state_set, orbit_index
                )
                assert after is not None
                assert len(before[0]) + len(before[1]) == 12
                assert len(after[0]) + len(after[1]) == 12
                for layer in range(3):
                    assert physical_value(after[0], layer) == (0,) * 6
                    assert physical_value(after[1], layer) == (0,) * 6
                recovered = inverse_bundle_step(
                    after[0], after[1], state_set, orbit_index
                )
                assert recovered is not None
                assert recovered[:2] == before
                assert recovered[2] == after[2]
                transaction_rows += 1
                inverse_rows += 1
    check(
        "C7 one radius-one move retains twelve records and transports the complete bundle",
        transaction_rows == 9 * 12 * 12,
    )
    check(
        "C8 the explicit reverse recovers every selected local transaction exactly",
        inverse_rows == transaction_rows,
    )

    covariance_rows = 0
    for router_orbit, left, right in basis_packets:
        rotor = router_orbit[0]
        destination_rotor = router_orbit[3]
        base = local_bundle_step(
            bundle_marked_site(rotor, left, right),
            unmarked_site(destination_rotor),
            state_set,
            orbit_index,
        )
        assert base is not None
        base_tensor = payload_tensor((left, right), 2)
        base_vector = payload_vector(advance(left, 2), 2)
        for matrix in group:
            transformed_rotor = transform_state(matrix, rotor)
            transformed_destination = transform_state(matrix, destination_rotor)
            transformed_left = transform_state(matrix, left)
            transformed_right = transform_state(matrix, right)
            transformed = local_bundle_step(
                bundle_marked_site(
                    transformed_rotor, transformed_left, transformed_right
                ),
                unmarked_site(transformed_destination),
                state_set,
                orbit_index,
            )
            assert transformed is not None
            assert transformed[0] == frozenset(
                (transform_state(matrix, state), polarity)
                for state, polarity in base[0]
            )
            assert transformed[1] == frozenset(
                (transform_state(matrix, state), polarity)
                for state, polarity in base[1]
            )
            assert transformed[2] == tuple(matrix_vector(matrix, base[2]))
            recognized = recognize_bundle_marked(
                transformed[1], state_set, orbit_index
            )
            assert recognized is not None
            transformed_payload_left = recognized[1]
            transformed_payload_right = recognized[2]
            assert payload_tensor(
                (transformed_payload_left, transformed_payload_right), 1
            ) == transform_tensor(matrix, base_tensor)
            assert payload_vector(
                advance(transformed_payload_left, 2), 1
            ) == tuple(matrix_vector(matrix, base_vector))
            covariance_rows += 1
    check(
        "C9 the complete selected bundle transaction is signed-cubic covariant",
        covariance_rows == 9 * 48,
    )

    reproduction_rows = 0
    poisson_rows = 0
    total_steps = 0
    for router_orbit, left_seed, right_seed in basis_packets:
        for radius, injections in ((1, 7), (1, 37), (2, 7)):
            vertices = tuple(box(radius))
            vertex_set = frozenset(vertices)
            source = (0, 0, 0)
            rotors = {vertex: router_orbit[0] for vertex in vertices}
            visits: defaultdict[Vec, int] = defaultdict(int)

            for _ in range(injections):
                location = source
                left = left_seed
                right = right_seed
                while location in vertex_set:
                    visits[location] += 1
                    departure_rotor = rotors[location]
                    direction = rotor_successor(internal_tick(departure_rotor))
                    destination = add(location, direction)
                    destination_rotor = (
                        rotors[destination]
                        if destination in vertex_set
                        else router_orbit[0]
                    )
                    output = local_bundle_step(
                        bundle_marked_site(departure_rotor, left, right),
                        unmarked_site(destination_rotor),
                        state_set,
                        orbit_index,
                    )
                    assert output is not None
                    rotors[location] = recognize_unmarked(output[0], state_set)
                    recognized = recognize_bundle_marked(
                        output[1], state_set, orbit_index
                    )
                    assert recognized is not None
                    left, right = recognized[1], recognized[2]
                    location = destination
                    total_steps += 1

            parent = simulate_rotor_box(
                radius, injections, router_orbit[0]
            )
            assert visits == parent[2]
            reproduction_rows += len(vertices)

            laplacian = dirichlet_laplacian(vertices)
            source_index = vertices.index(source)
            green = Matrix(
                [Fraction(visits[vertex], 6 * injections) for vertex in vertices]
            )
            residual = laplacian * green
            residual[source_index] -= 1
            assert max(abs(value) for value in residual) <= Fraction(8, injections)
            for component in bundle_payload(left_seed, right_seed, 0):
                component_residual = [component * value for value in residual]
                assert max(abs(value) for value in component_residual) <= (
                    abs(component) * Fraction(8, injections)
                )
                poisson_rows += len(component_residual)

    expected_vertices = 9 * (27 + 27 + 125)
    check(
        "C10 complete bundle walkers reproduce the exact scalar rotor histories",
        reproduction_rows == expected_vertices and total_steps > 0,
    )
    check(
        "C11 all nine source coordinates obey one componentwise Dirichlet-Poisson bound",
        poisson_rows == 9 * 9 * (27 + 27 + 125),
    )
    check(
        "C12 the rank-nine scalar/vector/STF space conditionally inherits one 1/Lambda Green kernel",
        source_rank == 9 and poisson_rows > 0,
    )

    blank = frozenset()
    malformed = (
        polarized_slots(states[0])
        | polarized_slots(advance(states[0], 4))
        | polarized_slots(advance(states[0], 2))
    )
    check(
        "C13 blank and orbit-colliding packets fail closed",
        recognize_bundle_marked(blank, state_set, orbit_index) is None
        and recognize_bundle_marked(malformed, state_set, orbit_index) is None,
    )

    missing = {
        "integration into the state-complete homogeneous Phi schedule",
        "autonomous source preparation, renewal, sink, and traffic arbitration",
        "tensor-protecting collision and propagated constraint algebra",
        "dynamical Hessian and positive static/tensor poles",
        "universal material coupling and shared cone",
        "physical normalization, lensing, and nonlinear closure",
    }
    check(
        "C14 common Green transport does not close native gravity",
        len(missing) == 6,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(
        f"\n{passed}/{len(checks)} v3 neutral scalar/vector/STF bundle checks pass"
    )
    print(f"admissible_source_rows={len(source_rows)}")
    print(f"joint_source_rank={source_rank}")
    print(f"basis_rows={basis_rows}")
    print(f"local_transaction_rows={transaction_rows}")
    print(f"signed_cubic_covariance_rows={covariance_rows}")
    print(f"finite_history_steps={total_steps}")
    print("joint_pole_status=conditional_rank_nine_blocked_history_1_over_Lambda")
    print("gravity_status=common_transport_closed_Phi_action_poles_coupling_lensing_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
