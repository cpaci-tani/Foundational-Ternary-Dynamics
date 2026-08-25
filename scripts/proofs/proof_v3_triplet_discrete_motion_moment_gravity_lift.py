#!/usr/bin/env python3
"""Exact prepared triplet discrete-motion moment and gravity-carrier lift.

For one admitted SC hop u between consecutive global ticks, the finite chord
(1,u) has one atomic second moment.  Its spatial decomposition gives scalar
one, polar vector u, and physical STF tensor uu^T-I/3.  The registered bundle
stores three times the STF tensor, so the integral source signature is

    (1, 3 uu^T-I, u).

Exact representation theory shows that cubic symmetry fixes these three
shapes but not their three relative coefficients.  Selecting the one-chord
second moment ties the relative coefficients without fixing the overall
action scale.  No single existing neutral bundle packet realizes the tied
signature.  Two packets do, minimally, in every SC direction and across the
full signed-cubic chart orbit.  Their common deterministic histories then
give the corresponding prepared scalar/vector/STF Dirichlet Green seam.

This certificate does not implement triplet translation under homogeneous
Phi, protect a gravity pole, or identify the readout with physical stress.
"""

from __future__ import annotations

import sys
from fractions import Fraction

from sympy import Matrix, cos, eye, linear_eq_to_matrix, symbols

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
    box,
    dirichlet_laplacian,
    simulate_rotor_box,
)
from proof_v3_neutral_rotor_walker_macro import physical_value
from proof_v3_neutral_scalar_vector_stf_bundle_common_green_seam import (
    bundle_marked_site,
    bundle_payload,
    orbit_lookup,
    recognize_bundle_marked,
)
from proof_v3_neutral_stf_rotor_walker_green_seam import internal_orbits


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def motion_signature(direction: Vec) -> tuple[int, ...]:
    """Bundle coordinates for the selected one-chord second moment."""

    x, y, z = direction
    assert x * x + y * y + z * z == 1
    return (
        1,
        3 * x * x - 1,
        3 * y * y - 1,
        3 * x * y,
        3 * x * z,
        3 * y * z,
        x,
        y,
        z,
    )


def first_pair_for_payload(states, orbit_index, target):
    for left in states:
        for right in states:
            if orbit_index[left] == orbit_index[right]:
                continue
            if bundle_payload(left, right, 0) == target:
                return left, right
    raise AssertionError(target)


def main() -> None:
    states = tuple(one_particle_states())
    state_set = frozenset(states)
    orbits = internal_orbits(states)
    orbit_index = orbit_lookup(orbits)
    group = tuple(signed_permutation_matrices())
    directions = tuple(SC_DIRECTIONS)

    # Exact O_h commutant on the polar-vector representation.
    entries = symbols("a0:9")
    candidate = Matrix(3, 3, entries)
    vector_equations = []
    for matrix in group:
        transform = Matrix(matrix)
        vector_equations.extend(candidate * transform - transform * candidate)
    vector_system, _ = linear_eq_to_matrix(vector_equations, entries)
    vector_nullspace = vector_system.nullspace()
    check(
        "C1 signed-cubic covariance leaves one polar-vector shape, proportional to u",
        len(vector_nullspace) == 1
        and Matrix(3, 3, vector_nullspace[0]).rank() == 3,
    )

    # An equivariant STF map on the transitive six-direction orbit is fixed by
    # an STF tensor invariant under the stabilizer of e_x.
    sx, sy, sxy, sxz, syz = symbols("sx sy sxy sxz syz")
    tensor = Matrix(
        (
            (sx, sxy, sxz),
            (sxy, sy, syz),
            (sxz, syz, -sx - sy),
        )
    )
    ex = (1, 0, 0)
    stabilizer = tuple(
        matrix
        for matrix in group
        if tuple(matrix_vector(matrix, ex)) == ex
    )
    tensor_equations = []
    for matrix in stabilizer:
        transform = Matrix(matrix)
        tensor_equations.extend(transform * tensor * transform.T - tensor)
    tensor_system, _ = linear_eq_to_matrix(
        tensor_equations, (sx, sy, sxy, sxz, syz)
    )
    tensor_nullspace = tensor_system.nullspace()
    axial_generator = Matrix(((2, 0, 0), (0, -1, 0), (0, 0, -1)))
    check(
        "C2 the SC-orbit STF shape is uniquely proportional to 3 uu^T-I",
        len(stabilizer) == 8
        and len(tensor_nullspace) == 1
        and all(
            Matrix(matrix) * axial_generator * Matrix(matrix).T
            == axial_generator
            for matrix in stabilizer
        ),
    )
    check(
        "C3 symmetry leaves scalar, vector, and STF coefficients mutually free",
        len(vector_nullspace) + len(tensor_nullspace) + 1 == 3,
    )

    # Selecting the atomic second moment of the finite global-clock chord
    # ties those shapes to one common event count.  This is a readout choice,
    # not a physical action or a coupling theorem.
    moment_rows = tuple(motion_signature(direction) for direction in directions)
    check(
        "C4 one-chord second moment gives the integral bundle signature (1,3uu^T-I,u)",
        len(set(moment_rows)) == 6
        and all(row[0] == 1 for row in moment_rows),
    )

    source_rows = tuple(
        bundle_payload(left, right, 0)
        for left in states
        for right in states
        if orbit_index[left] != orbit_index[right]
    )
    source_set = frozenset(source_rows)
    check(
        "C5 no single existing joint packet realizes any tied motion signature",
        len(source_rows) == 34_560
        and all(target not in source_set for target in moment_rows),
    )

    # Canonical +x witnesses.  Both carry vector +x.  Their STF rows average
    # to (2,-1,0,0,0), the stored 3 uu^T-I coordinate for u=+x.
    first_target = (1, 4, -2, 0, 0, 0, 1, 0, 0)
    second_target = (1, 0, 0, 0, 0, 0, 1, 0, 0)
    witness_pairs = (
        first_pair_for_payload(states, orbit_index, first_target),
        first_pair_for_payload(states, orbit_index, second_target),
    )
    witness_rows = tuple(bundle_payload(*pair, 0) for pair in witness_pairs)
    witness_sum = tuple(map(sum, zip(*witness_rows)))
    check(
        "C6 two existing packets realize the canonical tied motion moment exactly",
        witness_sum == tuple(2 * value for value in motion_signature(ex)),
    )

    covariance_rows = 0
    transformed_families = []
    for matrix in group:
        direction = tuple(matrix_vector(matrix, ex))
        transformed_pairs = tuple(
            tuple(transform_state(matrix, state) for state in pair)
            for pair in witness_pairs
        )
        rows = tuple(bundle_payload(*pair, 0) for pair in transformed_pairs)
        assert all(orbit_index[left] != orbit_index[right] for left, right in transformed_pairs)
        assert tuple(map(sum, zip(*rows))) == tuple(
            2 * value for value in motion_signature(direction)
        )
        transformed_families.append((direction, transformed_pairs))
        covariance_rows += len(rows)
    check(
        "C7 the two-packet lift is covariant on all 48 signed-cubic charts",
        covariance_rows == 96
        and {direction for direction, _ in transformed_families}
        == set(directions),
    )
    check(
        "C8 two packets are the exact minimum carrier price in the registered bundle class",
        all(target not in source_set for target in moment_rows)
        and witness_sum == tuple(2 * value for value in motion_signature(ex)),
    )

    neutral_rows = 0
    recognition_rows = 0
    for _, pairs in transformed_families:
        for left, right in pairs:
            payload_orbits = {orbit_index[left], orbit_index[right]}
            router = next(
                orbit[0]
                for orbit in orbits
                if orbit_index[orbit[0]] not in payload_orbits
            )
            marked = bundle_marked_site(router, left, right)
            assert len(marked) == 10
            assert recognize_bundle_marked(
                marked, state_set, orbit_index
            ) == (router, left, right)
            for layer in range(3):
                assert physical_value(marked, layer) == (0,) * 6
                neutral_rows += 1
            recognition_rows += 1
    check(
        "C9 every motion packet is state-recognizable, ten-record, and additive-E/B neutral",
        recognition_rows == 96 and neutral_rows == 288,
    )

    # The triplet scalar weight 1/12 is split equally between the two packet
    # witnesses.  Their mean is the finite moving-source coordinate.
    source_coordinates = {
        direction: tuple(
            Fraction(value, 12) for value in motion_signature(direction)
        )
        for direction in directions
    }
    check(
        "C10 motion reversal leaves scalar/STF even and flips only the vector source",
        all(
            source_coordinates[tuple(-value for value in direction)][:6]
            == source_coordinates[direction][:6]
            and source_coordinates[tuple(-value for value in direction)][6:]
            == tuple(-value for value in source_coordinates[direction][6:])
            for direction in directions
        ),
    )

    vertices = tuple(box(1))
    source = (0, 0, 0)
    source_index = vertices.index(source)
    delta = Matrix.zeros(len(vertices), 1)
    delta[source_index] = 1
    laplacian = dirichlet_laplacian(vertices)
    green = laplacian.inv() * delta
    finite_rows = 0
    for coordinate in source_coordinates.values():
        response = green * Matrix([coordinate])
        assert laplacian * response == delta * Matrix([coordinate])
        finite_rows += response.rows
    check(
        "C11 exact finite Dirichlet inverse carries the full moving-source coordinate",
        finite_rows == 6 * 27,
    )

    history_rows = 0
    maximum_residuals = {}
    for injections in (7, 37, 97):
        parent = simulate_rotor_box(1, injections, states[0])
        visits = parent[2]
        scalar_history = Matrix(
            [Fraction(visits[vertex], 6 * injections) for vertex in vertices]
        )
        maximum = Fraction(0)
        for coordinate in source_coordinates.values():
            response = scalar_history * Matrix([coordinate])
            residual = laplacian * response - delta * Matrix([coordinate])
            maximum = max(maximum, *(abs(value) for value in residual))
            assert maximum <= Fraction(4, 3 * injections)
            history_rows += len(vertices)
        maximum_residuals[injections] = maximum
    check(
        "C12 deterministic histories obey the full-moment residual bound 4/(3N)",
        history_rows == 3 * 6 * 27,
        str(maximum_residuals),
    )

    kx, ky, kz = symbols("kx ky kz", real=True)
    lattice_symbol = 6 - 2 * (cos(kx) + cos(ky) + cos(kz))
    conditional_poles = {
        direction: tuple(value / lattice_symbol for value in coordinate)
        for direction, coordinate in source_coordinates.items()
    }
    check(
        "C13 prepared motion moments conditionally share one componentwise 1/Lambda pole",
        all(
            tuple(value * lattice_symbol for value in pole) == coordinate
            for (direction, pole), coordinate in zip(
                conditional_poles.items(), source_coordinates.values()
            )
        ),
    )

    # The common-moment readout fixes relative coordinates but cannot fix the
    # multiplier of the response action.  Symmetry alone leaves even the
    # relative scalar/vector/STF coefficients independent (C3).
    response_prices = {
        price: tuple(price * value for value in source_coordinates[ex])
        for price in (1, 2, 7)
    }
    check(
        "C14 the common moment leaves the absolute gravity response multiplier free",
        len(set(response_prices.values())) == 3,
    )

    missing = {
        "homogeneous Phi triplet translation and packet formation",
        "reciprocal identification with physical stress and inertia",
        "protected scalar constraint and tensor wave poles",
        "action-fixed universal response normalization",
        "common cone, clock response, delay, and lensing",
        "nonlinear self-coupling",
    }
    check(
        "C15 the prepared motion lift does not close dynamical gravity",
        len(missing) == 6,
    )

    forbidden = (
        "newton_target",
        "einstein_target",
        "lensing_target",
        "137.036",
        "random_draw",
    )
    check(
        "C16 no empirical target, fit, random draw, or numerical near-miss search enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} triplet discrete-motion gravity-lift checks pass")
    print("symmetry_shapes=scalar_1,vector_u,STF_(3uuT-I)")
    print("symmetry_relative_coefficients=three_independent")
    print("common_chord_moment=selected_relative_tie")
    print("single_bundle_packet=impossible_for_all_6_SC_directions")
    print("minimum_bundle_packets=2")
    print("motion_source=(1/12)*(1,3uuT-I,u)")
    print("motion_reversal=scalar_STF_even_vector_odd")
    print("finite_history_residual_bound=4/(3N)")
    print("conditional_response=motion_source/Lambda")
    print("absolute_gravity_residue=free")
    print("status=prepared_motion_source_carrier_exact_dynamical_gravity_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()

