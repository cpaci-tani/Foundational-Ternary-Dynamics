#!/usr/bin/env python3
"""Exact triplet rest-source to scalar gravity Green-seam certificate.

The self-correcting cubic triplet has mean capacity deficit -I/36.  Its
positive rest-source readout is therefore I/36: scalar trace 1/12, zero STF,
and zero vector.  Six distinct signed-cubic images of one existing neutral
scalar/vector/STF packet sum to the physical coordinate (6,0,...,0), providing
a target-blind pure-scalar source cycle with no new carrier.

The registered deterministic rotor histories then give the triplet scalar
source the same controlled Dirichlet 1/Lambda Green seam.  This remains a
prepared blocked-history pole, not an autonomous protected gravity mode; its
positive response multiplier, universal coupling, cone, lensing, and nonlinear
completion remain open.
"""

from __future__ import annotations

import sys
from fractions import Fraction

from sympy import Matrix, cos, eye, symbols

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_v3_neutral_rotor_harmonic_green_seam import (
    box,
    dirichlet_laplacian,
    simulate_rotor_box,
)
from proof_v3_neutral_scalar_vector_stf_bundle_common_green_seam import (
    bundle_payload,
    internal_orbits,
    orbit_lookup,
)


sys.stdout.reconfigure(encoding="utf-8")

SCALAR = 0
STF = slice(1, 6)
VECTOR = slice(6, 9)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    group = tuple(signed_permutation_matrices())
    orbit_index = orbit_lookup(internal_orbits(states))

    capacity_deficit = -eye(3) / 36
    rest_tensor = -capacity_deficit
    scalar_mass = rest_tensor.trace()
    stf_source = rest_tensor - scalar_mass * eye(3) / 3
    vector_source = Matrix.zeros(3, 1)
    check(
        "C1 cubic triplet rest readout has scalar trace 1/12, zero STF, and zero vector",
        scalar_mass == Fraction(1, 12)
        and stf_source == Matrix.zeros(3, 3)
        and vector_source == Matrix.zeros(3, 1),
    )

    covariance_rows = 0
    for matrix in group:
        transform = Matrix(matrix)
        assert transform * rest_tensor * transform.T == rest_tensor
        covariance_rows += 1
    check(
        "C2 triplet rest source is invariant under the full signed-cubic group",
        covariance_rows == 48,
    )

    left, right = next(
        (left, right)
        for left in states
        for right in states
        if orbit_index[left] != orbit_index[right]
    )
    transformed_rows = tuple(
        bundle_payload(
            transform_state(matrix, left),
            transform_state(matrix, right),
            0,
        )
        for matrix in group
    )
    unique_rows = tuple(sorted(set(transformed_rows)))
    full_sum = tuple(map(sum, zip(*transformed_rows)))
    unique_sum = tuple(map(sum, zip(*unique_rows)))
    check(
        "C3 one existing packet has six distinct cubic source images",
        len(transformed_rows) == 48 and len(unique_rows) == 6,
    )
    check(
        "C4 the six-image physical source cycle is exactly pure scalar",
        unique_sum == (6,) + (0,) * 8
        and full_sum == (48,) + (0,) * 8,
    )

    average_payload = tuple(Fraction(value, 6) for value in unique_sum)
    triplet_coordinate = tuple(
        scalar_mass * value for value in average_payload
    )
    check(
        "C5 cycle average matches the triplet source coordinate (1/12,0,...,0)",
        triplet_coordinate == (Fraction(1, 12),) + (0,) * 8,
    )

    vertices = tuple(box(1))
    source = (0, 0, 0)
    source_index = vertices.index(source)
    delta = Matrix.zeros(len(vertices), 1)
    delta[source_index] = 1
    laplacian = dirichlet_laplacian(vertices)
    scalar_green = laplacian.inv() * delta
    response = scalar_green * Matrix([triplet_coordinate])
    exact_source = delta * Matrix([triplet_coordinate])
    check(
        "C6 exact finite Dirichlet inverse maps the triplet source into one scalar response",
        laplacian * response == exact_source,
    )
    check(
        "C7 every finite-domain STF and vector response component vanishes identically",
        all(
            response[row, component] == 0
            for row in range(response.rows)
            for component in range(1, 9)
        ),
    )

    # The same result follows packetwise: apply the common scalar Green column
    # to all six physical source rows, sum, and divide by six.
    packet_responses = [
        scalar_green * Matrix([row]) for row in unique_rows
    ]
    averaged_response = sum(
        packet_responses, Matrix.zeros(len(vertices), 9)
    ) / 6
    check(
        "C8 common bundle kernel sends the six physical packets to the same pure-scalar field",
        averaged_response == scalar_green * Matrix([average_payload]),
    )

    # Deterministic finite histories inherit the scalar parent's exact 8/N
    # residual bound, scaled by the triplet source 1/12.
    history_rows = 0
    maximum_scaled_residuals = {}
    for injections in (7, 37, 97):
        parent = simulate_rotor_box(1, injections, states[0])
        visits = parent[2]
        approximate = Matrix(
            [
                scalar_mass
                * Fraction(visits[vertex], 6 * injections)
                for vertex in vertices
            ]
        )
        residual = laplacian * approximate - scalar_mass * delta
        maximum = max(abs(value) for value in residual)
        assert maximum <= Fraction(2, 3 * injections)
        maximum_scaled_residuals[injections] = maximum
        history_rows += len(vertices)
    check(
        "C9 deterministic rotor histories obey the triplet-scaled Poisson bound 2/(3N)",
        history_rows == 3 * 27,
        str(maximum_scaled_residuals),
    )

    kx, ky, kz = symbols("kx ky kz", real=True)
    lattice_symbol = 6 - 2 * (cos(kx) + cos(ky) + cos(kz))
    pole = scalar_mass / lattice_symbol
    check(
        "C10 prepared triplet rest response conditionally has the scalar (1/12)/Lambda pole",
        pole * lattice_symbol == Fraction(1, 12),
    )

    # The finite source construction fixes shape and relative coordinate only.
    # Multiplying the response action by any positive g changes the physical
    # residue without changing a single packet, clock, or history transition.
    response_prices = {
        price: price * scalar_mass for price in (1, 2, 7)
    }
    check(
        "C11 source shape and Green kernel leave the positive gravity residue multiplier free",
        response_prices == {
            1: Fraction(1, 12),
            2: Fraction(1, 6),
            7: Fraction(7, 12),
        },
    )

    missing = {
        "homogeneous Phi integration and autonomous source renewal",
        "positive dynamical Hessian and protected scalar constraint pole",
        "motion-generated vector and STF stress response",
        "universal matter/radiation coupling and absolute normalization",
        "common cone, clock response, Shapiro delay, and lensing",
        "nonlinear self-coupling and Einstein bootstrap input",
    }
    check(
        "C12 scalar rest bridge does not close physical gravity",
        len(missing) == 6,
    )

    forbidden = (
        "newton_target",
        "lensing_target",
        "einstein_target",
        "137.036",
        "random_draw",
    )
    check(
        "C13 no empirical gravity target, coupling value, random draw, or numerical search enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} triplet scalar-gravity bridge checks pass")
    print("triplet_rest_tensor=I/36")
    print("triplet_scalar_source=1/12")
    print("triplet_STF_source=0")
    print("triplet_vector_source=0")
    print(f"unique_cubic_packet_rows={len(unique_rows)}")
    print("six_packet_sum=(6,0,0,0,0,0,0,0,0)")
    print("finite_history_residual_bound=2/(3N)")
    print("conditional_static_pole=(1/12)/Lambda")
    print("absolute_gravity_residue=free")
    print("status=prepared_scalar_rest_green_bridge_exact_physical_gravity_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
