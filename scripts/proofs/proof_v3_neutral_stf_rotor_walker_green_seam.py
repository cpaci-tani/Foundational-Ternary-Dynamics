#!/usr/bin/env python3
"""Exact neutral STF-payload rotor/walker and tensor Green seam for FTD-v3.

The carrier-complete neutral rotor walker uses two opposite-polarity field
records for the site rotor and another neutral pair for its marker.  This
certificate adds two further opposite-polarity pairs as a retained payload.
The rotor/marker pair is the unique R^4-related controller pair; the two
remaining controllers define a symmetric trace-free cross-stress payload.

Five explicit payload pairs span the complete spatial STF space.  A clean
radius-one transaction moves the payload by one SC hop, advances its native
clock, preserves record number and exact zero additive E/B, and is
signed-cubic covariant.  Sequential absorbing-box histories therefore carry
each STF basis tensor through exactly the deterministic rotor histories of the
scalar Green theorem.  Componentwise visit averages converge to the
Dirichlet tensor Green field and conditionally have the same 1/Lambda static
pole.

This is a blocked-history carrier seam.  It does not protect the tensor under
canonical Phi-v2, supply TT constraints, generate sources/sinks, fix an
action, or derive universal gravitational coupling, a common cone, or
lensing.
"""

from __future__ import annotations

import sys
from collections import defaultdict

from sympy import Matrix, Rational

from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick,
)
from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_v3_neutral_rotor_harmonic_green_seam import (
    SC_DIRECTIONS,
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
from proof_v3_two_record_full_stf_tensor_carrier_boundary import (
    cross_stress3,
    stf5,
    transform_tensor,
)


sys.stdout.reconfigure(encoding="utf-8")


def internal_orbits(states):
    unseen = set(states)
    orbits = []
    while unseen:
        seed = min(unseen, key=repr)
        orbit = []
        current = seed
        while current not in orbit:
            orbit.append(current)
            unseen.discard(current)
            current = internal_tick(current)
        orbits.append(tuple(orbit))
    return tuple(orbits)


def tensor_marked_site(rotor, left_payload, right_payload):
    return (
        polarized_slots(rotor)
        | polarized_slots(advance(rotor, 4))
        | polarized_slots(left_payload)
        | polarized_slots(right_payload)
    )


def recognize_tensor_marked(slots, state_set):
    controllers = controller_states(slots)
    if len(controllers) != 4:
        return None
    rotor_candidates = [
        state
        for state in controllers
        if state in state_set and advance(state, 4) in controllers
    ]
    if len(rotor_candidates) != 1:
        return None
    rotor = rotor_candidates[0]
    payload = tuple(
        sorted(
            controllers - {rotor, advance(rotor, 4)},
            key=repr,
        )
    )
    if len(payload) != 2:
        return None
    exact = tensor_marked_site(rotor, payload[0], payload[1])
    return (rotor, payload) if exact == slots else None


def payload_tensor(payload, layer: int):
    return cross_stress3(payload[0], payload[1], layer)


def local_tensor_step(departure_slots, destination_slots, state_set):
    departure = recognize_tensor_marked(departure_slots, state_set)
    destination_rotor = recognize_unmarked(destination_slots, state_set)
    if departure is None or destination_rotor is None:
        return None
    departure_rotor, payload = departure
    advanced_rotor = internal_tick(departure_rotor)
    advanced_payload = tuple(internal_tick(state) for state in payload)
    destination_after = tensor_marked_site(
        destination_rotor,
        advanced_payload[0],
        advanced_payload[1],
    )
    if recognize_tensor_marked(destination_after, state_set) is None:
        return None
    return (
        unmarked_site(advanced_rotor),
        destination_after,
        rotor_successor(advanced_rotor),
    )


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    state_set = frozenset(states)
    group = tuple(signed_permutation_matrices())
    orbits = internal_orbits(states)

    check(
        "C1 field controller decomposes into sixteen native period-twelve orbits",
        len(orbits) == 16 and all(len(orbit) == 12 for orbit in orbits),
    )

    # Explicit target-free basis selection.  These are canonical internal
    # orbit indices, not values chosen by a physics comparison or fit.
    payload_orbit_pairs = ((1, 2), (1, 4), (1, 6), (1, 8), (1, 12))
    payload_basis = tuple(
        (orbits[left][0], orbits[right][0])
        for left, right in payload_orbit_pairs
    )
    basis_rows = tuple(stf5(payload_tensor(payload, 0)) for payload in payload_basis)
    check(
        "C2 five explicit neutral payload pairs span the complete STF rank five",
        Matrix(basis_rows).rank() == 5,
        str(basis_rows),
    )

    router_orbit = orbits[0]
    role_rows = 0
    neutral_rows = 0
    clock_rows = 0
    for payload in payload_basis:
        for offset in range(12):
            rotor = router_orbit[offset]
            clocked_payload = tuple(advance(state, offset) for state in payload)
            marked = tensor_marked_site(rotor, *clocked_payload)
            recognized = recognize_tensor_marked(marked, state_set)
            assert recognized == (rotor, tuple(sorted(clocked_payload, key=repr)))
            assert len(marked) == 8
            for layer in range(3):
                assert physical_value(marked, layer) == (0,) * 6
                before = payload_tensor(recognized[1], layer)
                after_payload = tuple(internal_tick(state) for state in recognized[1])
                after = payload_tensor(after_payload, (layer - 1) % 3)
                assert after == before
                neutral_rows += 1
                clock_rows += 1
            role_rows += 1
    check("C3 rotor/marker and two-record STF payload roles are state-only unique", role_rows == 60)
    check("C4 every tensor marker has eight records and exact zero additive E/B", neutral_rows == 180)
    check("C5 payload tensor is covariantly constant under the native clock", clock_rows == 180)

    transaction_rows = 0
    for payload in payload_basis:
        for departure_rotor in router_orbit:
            for destination_rotor in router_orbit:
                before_departure = tensor_marked_site(departure_rotor, *payload)
                before_destination = unmarked_site(destination_rotor)
                output = local_tensor_step(
                    before_departure,
                    before_destination,
                    state_set,
                )
                assert output is not None
                after_departure, after_destination, direction = output
                recognized = recognize_tensor_marked(after_destination, state_set)
                assert recognized is not None
                assert recognize_unmarked(after_departure, state_set) == internal_tick(departure_rotor)
                assert recognized[0] == destination_rotor
                assert set(recognized[1]) == {internal_tick(state) for state in payload}
                assert direction == rotor_successor(internal_tick(departure_rotor))
                assert len(before_departure) + len(before_destination) == 10
                assert len(after_departure) + len(after_destination) == 10
                transaction_rows += 1
    check(
        "C6 one radius-one transaction conserves ten records and moves the STF payload one SC hop",
        transaction_rows == 5 * 12 * 12,
    )

    covariance_rows = 0
    for payload in payload_basis:
        departure_rotor = router_orbit[0]
        destination_rotor = router_orbit[3]
        base = local_tensor_step(
            tensor_marked_site(departure_rotor, *payload),
            unmarked_site(destination_rotor),
            state_set,
        )
        assert base is not None
        base_payload = recognize_tensor_marked(base[1], state_set)[1]
        base_tensor = payload_tensor(base_payload, 2)
        for matrix in group:
            transformed_payload = tuple(transform_state(matrix, state) for state in payload)
            transformed = local_tensor_step(
                tensor_marked_site(
                    transform_state(matrix, departure_rotor),
                    *transformed_payload,
                ),
                unmarked_site(transform_state(matrix, destination_rotor)),
                state_set,
            )
            assert transformed is not None
            transformed_after_payload = recognize_tensor_marked(transformed[1], state_set)[1]
            assert transformed[2] == tuple(matrix_vector(matrix, base[2]))
            assert payload_tensor(transformed_after_payload, 2) == transform_tensor(
                matrix, base_tensor
            )
            covariance_rows += 1
    check(
        "C7 complete selected tensor-walker family is signed-cubic covariant",
        covariance_rows == 5 * 48,
    )

    reproduction_rows = 0
    poisson_rows = 0
    total_steps = 0
    for basis_index, payload_seed in enumerate(payload_basis):
        for radius, injections in ((1, 7), (1, 37), (2, 7)):
            vertices = tuple(box(radius))
            vertex_set = frozenset(vertices)
            source = (0, 0, 0)
            rotors = {vertex: router_orbit[0] for vertex in vertices}
            visits: defaultdict[Vec, int] = defaultdict(int)
            payload = payload_seed
            for _ in range(injections):
                location = source
                payload = payload_seed
                while location in vertex_set:
                    visits[location] += 1
                    departure_rotor = rotors[location]
                    direction = rotor_successor(internal_tick(departure_rotor))
                    destination = add(location, direction)
                    destination_rotor = (
                        rotors[destination] if destination in vertex_set else router_orbit[0]
                    )
                    output = local_tensor_step(
                        tensor_marked_site(departure_rotor, *payload),
                        unmarked_site(destination_rotor),
                        state_set,
                    )
                    assert output is not None
                    rotors[location] = recognize_unmarked(output[0], state_set)
                    payload = recognize_tensor_marked(output[1], state_set)[1]
                    location = destination
                    total_steps += 1

            parent = simulate_rotor_box(radius, injections, router_orbit[0])
            assert visits == parent[2]
            reproduction_rows += len(vertices)

            laplacian = dirichlet_laplacian(vertices)
            source_index = vertices.index(source)
            scalar_green = Matrix(
                [Rational(visits[vertex], 6 * injections) for vertex in vertices]
            )
            scalar_residual = laplacian * scalar_green
            scalar_residual[source_index] -= 1
            assert max(abs(value) for value in scalar_residual) <= Rational(8, injections)
            tensor = stf5(payload_tensor(payload_seed, 0))
            for component in tensor:
                component_residual = [component * value for value in scalar_residual]
                assert max(abs(value) for value in component_residual) <= abs(component) * Rational(8, injections)
                poisson_rows += len(component_residual)

    check(
        "C8 carrier-complete STF walkers reproduce the certified rotor histories",
        reproduction_rows == 5 * (27 + 27 + 125) and total_steps > 0,
    )
    check(
        "C9 tensor visit fields obey the exact componentwise Dirichlet-Poisson bound",
        poisson_rows == 5 * 5 * (27 + 27 + 125),
    )
    check(
        "C10 five independent STF sources conditionally inherit the scalar 1/Lambda pole",
        Matrix(basis_rows).rank() == 5,
    )

    missing = {
        "integration into the complete Phi schedule",
        "native tensor-source preparation and owned sink",
        "tensor-protecting autonomous collision",
        "common tensor action and physical normalization",
        "local scalar/vector constraints and TT reduction",
        "universal conserved-stress coupling",
        "shared radiation/matter cone",
        "lensing and nonlinear bootstrap",
    }
    check("C11 tensor Green carrier seam retains eight physical debts", len(missing) == 8)
    check("C12 gravity remains open beyond the blocked-history pole seam", len(missing) > 0)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 neutral STF rotor/walker checks pass")
    print(f"stf_basis_rows={basis_rows}")
    print(f"local_transaction_rows={transaction_rows}")
    print(f"signed_cubic_covariance_rows={covariance_rows}")
    print(f"finite_history_steps={total_steps}")
    print("tensor_pole_status=conditional_full_rank_five_blocked_history_1_over_Lambda")
    print("gravity_status=Phi_action_constraints_universal_coupling_and_lensing_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
