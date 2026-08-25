#!/usr/bin/env python3
"""Neutral vector-constraint walker and exact local-TT obstruction for v3.

Three opposite-polarity payload pairs encode a full polar-vector basis while
remaining exactly zero in additive E/B.  A state-only rotor/marker header
moves each payload one SC hop with the native clock, fixed record number, and
signed-cubic covariance.  Sequential absorbing-box histories reproduce the
scalar rotor visits componentwise, giving a conditional vector 1/Lambda Green
seam using only the selected field bank.

Separately, the exact cubic lattice TT projector contains D_i D_j/Lambda.
The Laplacian symbol vanishes at an explicit algebraic torus point where the
cross numerator does not, so the ratio is not Laurent polynomial.  Hence an
exact translation-invariant finite-range TT projector cannot be an on-site or
finite-radius collision.  Local gravity must propagate auxiliary constraints
or pay a nonlocal rule; the walker supplies carrier kinematics, not the common
constraint action.
"""

from __future__ import annotations

import sys
from collections import defaultdict

from sympy import Matrix, Rational, simplify, sqrt

from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick,
    layer_value,
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
from proof_v3_neutral_stf_rotor_walker_green_seam import internal_orbits


sys.stdout.reconfigure(encoding="utf-8")


def vector_marked_site(rotor, payload):
    return (
        polarized_slots(rotor)
        | polarized_slots(advance(rotor, 4))
        | polarized_slots(payload)
    )


def recognize_vector_marked(slots, state_set):
    controllers = controller_states(slots)
    if len(controllers) != 3:
        return None
    rotor_candidates = [
        state
        for state in controllers
        if state in state_set and advance(state, 4) in controllers
    ]
    if len(rotor_candidates) != 1:
        return None
    rotor = rotor_candidates[0]
    payloads = controllers - {rotor, advance(rotor, 4)}
    if len(payloads) != 1:
        return None
    payload = next(iter(payloads))
    return (rotor, payload) if vector_marked_site(rotor, payload) == slots else None


def payload_vector(payload, layer: int):
    return layer_value(payload, layer)[:3]


def local_vector_step(departure_slots, destination_slots, state_set):
    departure = recognize_vector_marked(departure_slots, state_set)
    destination_rotor = recognize_unmarked(destination_slots, state_set)
    if departure is None or destination_rotor is None:
        return None
    departure_rotor, payload = departure
    advanced_rotor = internal_tick(departure_rotor)
    advanced_payload = internal_tick(payload)
    destination_after = vector_marked_site(destination_rotor, advanced_payload)
    if recognize_vector_marked(destination_after, state_set) is None:
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
    router_orbit = orbits[0]

    check(
        "C1 existing field controller has sixteen native period-twelve orbits",
        len(orbits) == 16 and all(len(orbit) == 12 for orbit in orbits),
    )

    # Canonical nonrouter orbit representatives: -x, -y, -z at layer zero.
    payload_basis = (orbits[1][0], orbits[8][0], orbits[12][0])
    vector_basis = tuple(payload_vector(payload, 0) for payload in payload_basis)
    check(
        "C2 three explicit neutral payloads span the complete polar-vector rank three",
        Matrix(vector_basis).rank() == 3,
        str(vector_basis),
    )

    role_rows = 0
    neutral_rows = 0
    clock_rows = 0
    for payload_seed in payload_basis:
        for offset in range(12):
            rotor = router_orbit[offset]
            payload = advance(payload_seed, offset)
            marked = vector_marked_site(rotor, payload)
            assert recognize_vector_marked(marked, state_set) == (rotor, payload)
            assert len(marked) == 6
            for layer in range(3):
                assert physical_value(marked, layer) == (0,) * 6
                assert payload_vector(internal_tick(payload), (layer - 1) % 3) == payload_vector(payload, layer)
                neutral_rows += 1
                clock_rows += 1
            role_rows += 1
    check(
        "C3 rotor/marker and vector-payload roles are state-only unique",
        role_rows == 36,
    )
    check(
        "C4 every vector marker has six records and exact zero additive E/B",
        neutral_rows == 108,
    )
    check(
        "C5 vector payload is covariantly constant under the native C4/C3 clock",
        clock_rows == 108,
    )

    transaction_rows = 0
    for payload in payload_basis:
        for departure_rotor in router_orbit:
            for destination_rotor in router_orbit:
                before_departure = vector_marked_site(departure_rotor, payload)
                before_destination = unmarked_site(destination_rotor)
                output = local_vector_step(
                    before_departure,
                    before_destination,
                    state_set,
                )
                assert output is not None
                after_departure, after_destination, direction = output
                recognized = recognize_vector_marked(after_destination, state_set)
                assert recognized is not None
                assert recognize_unmarked(after_departure, state_set) == internal_tick(departure_rotor)
                assert recognized == (destination_rotor, internal_tick(payload))
                assert direction == rotor_successor(internal_tick(departure_rotor))
                assert len(before_departure) + len(before_destination) == 8
                assert len(after_departure) + len(after_destination) == 8
                transaction_rows += 1
    check(
        "C6 radius-one transaction conserves eight records and moves one vector payload",
        transaction_rows == 3 * 12 * 12,
    )

    covariance_rows = 0
    for payload in payload_basis:
        departure_rotor = router_orbit[0]
        destination_rotor = router_orbit[3]
        base = local_vector_step(
            vector_marked_site(departure_rotor, payload),
            unmarked_site(destination_rotor),
            state_set,
        )
        assert base is not None
        base_payload = recognize_vector_marked(base[1], state_set)[1]
        base_vector = payload_vector(base_payload, 2)
        for matrix in group:
            transformed = local_vector_step(
                vector_marked_site(
                    transform_state(matrix, departure_rotor),
                    transform_state(matrix, payload),
                ),
                unmarked_site(transform_state(matrix, destination_rotor)),
                state_set,
            )
            assert transformed is not None
            transformed_payload = recognize_vector_marked(transformed[1], state_set)[1]
            assert transformed[2] == tuple(matrix_vector(matrix, base[2]))
            assert payload_vector(transformed_payload, 2) == tuple(
                matrix_vector(matrix, base_vector)
            )
            covariance_rows += 1
    check(
        "C7 complete vector-walker basis is signed-cubic covariant",
        covariance_rows == 3 * 48,
    )

    reproduction_rows = 0
    poisson_rows = 0
    total_steps = 0
    for payload_seed in payload_basis:
        for radius, injections in ((1, 7), (1, 37), (2, 7)):
            vertices = tuple(box(radius))
            vertex_set = frozenset(vertices)
            source = (0, 0, 0)
            rotors = {vertex: router_orbit[0] for vertex in vertices}
            visits: defaultdict[Vec, int] = defaultdict(int)
            location = source
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
                    output = local_vector_step(
                        vector_marked_site(departure_rotor, payload),
                        unmarked_site(destination_rotor),
                        state_set,
                    )
                    assert output is not None
                    rotors[location] = recognize_unmarked(output[0], state_set)
                    payload = recognize_vector_marked(output[1], state_set)[1]
                    location = destination
                    total_steps += 1

            parent = simulate_rotor_box(radius, injections, router_orbit[0])
            assert visits == parent[2]
            reproduction_rows += len(vertices)

            laplacian = dirichlet_laplacian(vertices)
            source_index = vertices.index(source)
            green = Matrix(
                [Rational(visits[vertex], 6 * injections) for vertex in vertices]
            )
            residual = laplacian * green
            residual[source_index] -= 1
            assert max(abs(value) for value in residual) <= Rational(8, injections)
            vector = payload_vector(payload_seed, 0)
            for component in vector:
                component_residual = [component * value for value in residual]
                assert max(abs(value) for value in component_residual) <= abs(component) * Rational(8, injections)
                poisson_rows += len(component_residual)

    check(
        "C8 vector walkers reproduce the exact certified scalar rotor histories",
        reproduction_rows == 3 * (27 + 27 + 125) and total_steps > 0,
    )
    check(
        "C9 vector visit fields obey the exact componentwise Dirichlet-Poisson bound",
        poisson_rows == 3 * 3 * (27 + 27 + 125),
    )

    # Laurent-polynomial obstruction for an instantaneous finite-range TT
    # projector.  Lambda vanishes at an exact algebraic point while the cross
    # derivative numerator does not; hence Lambda cannot divide it.
    x = Rational(-1)
    y = Rational(-1)
    z = 5 + 2 * sqrt(6)
    lattice_lambda = 6 - (x + 1 / x + y + 1 / y + z + 1 / z)
    cross_numerator = (x - 1) * (y - 1)
    check(
        "C10 exact TT cross-projector symbol has a nonremovable 1/Lambda denominator",
        simplify(lattice_lambda) == 0 and simplify(cross_numerator) == 4,
        f"Lambda={simplify(lattice_lambda)}, numerator={simplify(cross_numerator)}",
    )
    check(
        "C11 exact translation-invariant finite-range TT projection is obstructed",
        simplify(lattice_lambda) == 0 and simplify(cross_numerator) != 0,
    )

    missing = {
        "integration into canonical Phi and native constraint sources/sinks",
        "common scalar/vector/tensor constraint action",
        "composition of auxiliary Green solves into exact lattice TT dynamics",
        "protected autonomous tensor collision and common cone",
        "universal conserved-stress coupling and normalization",
        "lensing and nonlinear gravity",
    }
    check(
        "C12 gravity remains open at six action/integration debts",
        len(missing) == 6,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} vector-constraint/TT-locality checks pass")
    print(f"vector_basis={vector_basis}")
    print(f"local_transaction_rows={transaction_rows}")
    print(f"signed_cubic_covariance_rows={covariance_rows}")
    print(f"finite_history_steps={total_steps}")
    print("vector_pole_status=conditional_rank_three_blocked_history_1_over_Lambda")
    print("TT_status=exact_finite_range_instantaneous_projector_obstructed")
    print("gravity_status=local_auxiliary_constraint_dynamics_and_common_action_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
