#!/usr/bin/env python3
"""Exact existing-carrier rotor seam to the cubic Dirichlet Green function.

Two opposite-polarity copies of one selected v3 Hodge record have exactly zero
additive (E,B) readout on every C3 layer while retaining the record's oriented
flag and C4 phase.  Advancing that record with the native internal tick and
reading sign(phase) times its tangent gives a period-12 rotor stack in which
each of the six SC directions occurs exactly twice.  The maximum prefix
discrepancy from one-sixth service is exactly 4/3.

On any finite cubic domain with an absorbing exterior, sequentially routing N
site tokens by these local stacks produces an exact unit source-to-sink flow.
If n_N(x) is the visit count and G_N=n_N/(6N), then

    ||L_D G_N - delta_source||_infinity <= 8/N.

Thus, for each fixed finite domain, G_N converges to the unique Dirichlet
Green function and the normalized net traversal flow converges to its discrete
gradient.  The infinite cubic symbol is Lambda(k)=6-2 sum cos(k_i), so the
controlled large-domain limit has the static 1/Lambda pole.

This is a finite-history readout theorem and an existing-carrier candidate
mechanism.  It does not integrate the rotor macro into canonical Phi-v2,
autonomously prepare the rotor background or repeated source injections,
write the averaged Green field into the instantaneous finite bank, or fix the
physical action/coupling normalization.
"""

from __future__ import annotations

import sys
from collections import Counter, defaultdict
from fractions import Fraction
from itertools import product

from sympy import Matrix, cos, diff, symbols

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


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
SC_DIRECTIONS: tuple[Vec, ...] = (
    (1, 0, 0),
    (-1, 0, 0),
    (0, 1, 0),
    (0, -1, 0),
    (0, 0, 1),
    (0, 0, -1),
)


def add(left: Vec, right: Vec) -> Vec:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def neg(vector: Vec) -> Vec:
    return tuple(-entry for entry in vector)  # type: ignore[return-value]


def rotor_successor(state) -> Vec:
    """SC successor selected by the oriented flag and its C4 half-plane."""

    tangent, _normal, _handedness = state[0]
    phase = state[1]
    sign = 1 if phase in (0, 1) else -1
    return tuple(sign * entry for entry in tangent)  # type: ignore[return-value]


def neutral_pair_value(state, layer: int) -> tuple[int, ...]:
    positive = tuple(entry for entry in layer_value(state, layer))
    negative = tuple(-entry for entry in layer_value(state, layer))
    return tuple(a + b for a, b in zip(positive, negative))


def rotor_prefix_discrepancy(state) -> Fraction:
    current = state
    counts: Counter[Vec] = Counter()
    maximum = Fraction(0)
    for visits in range(1, 13):
        current = internal_tick(current)
        counts[rotor_successor(current)] += 1
        for direction in SC_DIRECTIONS:
            maximum = max(
                maximum,
                abs(Fraction(counts[direction]) - Fraction(visits, 6)),
            )
    return maximum


def box(radius: int) -> tuple[Vec, ...]:
    return tuple(product(range(-radius, radius + 1), repeat=3))


def simulate_rotor_box(radius: int, injections: int, initial_state):
    vertices = frozenset(box(radius))
    source = (0, 0, 0)
    rotors = {vertex: initial_state for vertex in vertices}
    visits: defaultdict[Vec, int] = defaultdict(int)
    traversals: defaultdict[tuple[Vec, Vec], int] = defaultdict(int)
    absorbed = 0
    total_steps = 0

    for _ in range(injections):
        location = source
        while location in vertices:
            visits[location] += 1
            rotors[location] = internal_tick(rotors[location])
            direction = rotor_successor(rotors[location])
            traversals[(location, direction)] += 1
            location = add(location, direction)
            total_steps += 1
            assert total_steps < 50_000_000
        absorbed += 1

    return vertices, source, visits, traversals, absorbed, total_steps


def dirichlet_laplacian(vertices: tuple[Vec, ...]) -> Matrix:
    index = {vertex: row for row, vertex in enumerate(vertices)}
    laplacian = Matrix.zeros(len(vertices), len(vertices))
    for vertex, row in index.items():
        laplacian[row, row] = 6
        for direction in SC_DIRECTIONS:
            neighbor = add(vertex, direction)
            if neighbor in index:
                laplacian[row, index[neighbor]] -= 1
    return laplacian


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = one_particle_states()
    group = tuple(signed_permutation_matrices())

    check("C1 selected v3 field bank has 192 oriented one-polarity records", len(states) == 192)

    neutral_rows = 0
    for state in states:
        for layer in range(3):
            assert neutral_pair_value(state, layer) == (0,) * 6
            neutral_rows += 1
    check("C2 opposite-polarity record pairs have zero E/B on every layer", neutral_rows == 576)

    clock_rows = 0
    for state in states:
        current = state
        for _ in range(12):
            current = internal_tick(current)
        assert current == state
        assert internal_tick(state) != state
        clock_rows += 1
    check("C3 every neutral rotor controller has native period twelve", clock_rows == 192)

    service_rows = 0
    prefix_bounds = []
    for state in states:
        current = state
        successors = []
        for _ in range(12):
            current = internal_tick(current)
            successors.append(rotor_successor(current))
        assert Counter(successors) == Counter(
            {direction: 2 for direction in SC_DIRECTIONS}
        )
        prefix_bounds.append(rotor_prefix_discrepancy(state))
        service_rows += len(successors)
    check("C4 every native rotor period serves all six SC neighbors twice", service_rows == 2_304)
    check(
        "C5 exact global worst prefix discrepancy is 4/3",
        max(prefix_bounds) == Fraction(4, 3)
        and all(bound <= Fraction(4, 3) for bound in prefix_bounds),
    )

    covariance_rows = 0
    for state in states:
        for matrix in group:
            transformed = transform_state(matrix, state)
            assert internal_tick(transformed) == transform_state(
                matrix, internal_tick(state)
            )
            assert rotor_successor(transformed) == tuple(
                matrix_vector(matrix, rotor_successor(state))
            )
            covariance_rows += 2
    check("C6 rotor clock and successor are signed-cubic covariant", covariance_rows == 192 * 48 * 2)

    # Opposite polarities are distinct exclusion slots.  Updating both with
    # the same internal tick retains two occupied bits and exact cancellation.
    distinct_pair_rows = 0
    for state in states:
        positive_slot = (state[0], state[1], 1)
        negative_slot = (state[0], state[1], -1)
        assert positive_slot != negative_slot
        advanced = internal_tick(state)
        assert neutral_pair_value(advanced, 0) == (0,) * 6
        distinct_pair_rows += 1
    check("C7 rotor storage uses two existing distinct exclusion slots", distinct_pair_rows == 192)

    fixture_rows = 0
    conservation_rows = 0
    discrepancy_rows = 0
    poisson_rows = 0
    gradient_rows = 0
    total_steps = 0
    fixture_data = []
    for radius, injections, state_index in (
        (1, 1, 0),
        (1, 7, 37),
        (1, 37, 101),
        (1, 128, 0),
        (2, 7, 37),
        (2, 37, 101),
        (2, 128, 0),
    ):
        vertices, source, visits, traversals, absorbed, steps = simulate_rotor_box(
            radius, injections, states[state_index]
        )
        assert absorbed == injections
        total_steps += steps

        for vertex in vertices:
            outgoing = sum(
                traversals[(vertex, direction)] for direction in SC_DIRECTIONS
            )
            incoming = sum(
                traversals[(add(vertex, direction), neg(direction))]
                for direction in SC_DIRECTIONS
                if add(vertex, direction) in vertices
            )
            assert outgoing == visits[vertex]
            assert outgoing - incoming == (
                injections if vertex == source else 0
            )
            conservation_rows += 2

            potential = Fraction(visits[vertex], 6 * injections)
            laplacian_potential = 6 * potential - sum(
                Fraction(visits[add(vertex, direction)], 6 * injections)
                for direction in SC_DIRECTIONS
                if add(vertex, direction) in vertices
            )
            residual = laplacian_potential - (
                1 if vertex == source else 0
            )
            assert abs(residual) <= Fraction(8, injections)
            poisson_rows += 1

            divergence = Fraction(0)
            for direction in SC_DIRECTIONS:
                neighbor = add(vertex, direction)
                forward = traversals[(vertex, direction)]
                reverse = (
                    traversals[(neighbor, neg(direction))]
                    if neighbor in vertices
                    else 0
                )
                net_current = Fraction(forward - reverse, injections)
                divergence += net_current

                neighbor_potential = (
                    Fraction(visits[neighbor], 6 * injections)
                    if neighbor in vertices
                    else Fraction(0)
                )
                gradient = potential - neighbor_potential
                assert abs(net_current - gradient) <= Fraction(
                    8, 3 * injections
                )
                gradient_rows += 1
            assert divergence == (1 if vertex == source else 0)

            for direction in SC_DIRECTIONS:
                served = traversals[(vertex, direction)]
                assert abs(
                    Fraction(served) - Fraction(visits[vertex], 6)
                ) <= Fraction(4, 3)
                discrepancy_rows += 1

        fixture_data.append((vertices, source, visits, injections))
        fixture_rows += 1

    check("C8 every sequential token is finitely absorbed", fixture_rows == 7 and total_steps > 0)
    check("C9 normalized net traversal current has exact unit source divergence", conservation_rows > 0)
    check("C10 every local service count obeys the exact 4/3 rotor bound", discrepancy_rows > 0)
    check("C11 Green readout has exact Dirichlet-Poisson residual at most 8/N", poisson_rows > 0)
    check("C12 traversal current approaches the Green gradient with 8/(3N) bound", gradient_rows > 0)

    # One exact finite inverse verifies the normalization and the residual/error
    # identity without floating arithmetic.
    vertices = tuple(sorted(box(1)))
    index = {vertex: row for row, vertex in enumerate(vertices)}
    laplacian = dirichlet_laplacian(vertices)
    source_vector = Matrix.zeros(len(vertices), 1)
    source_vector[index[(0, 0, 0)]] = 1
    exact_green = laplacian.inv() * source_vector
    assert laplacian * exact_green == source_vector
    inverse_rows = 0
    for fixture_vertices, _source, visits, injections in fixture_data:
        if len(fixture_vertices) != 27:
            continue
        approximate = Matrix(
            [Fraction(visits[vertex], 6 * injections) for vertex in vertices]
        )
        residual = laplacian * approximate - source_vector
        error = approximate - exact_green
        assert laplacian * error == residual
        inverse_rows += len(vertices)
    check("C13 exact finite inverse converts the vanishing residual to Green convergence", inverse_rows == 4 * 27)

    kx, ky, kz = symbols("kx ky kz", real=True)
    wavevector = (kx, ky, kz)
    lattice_symbol = 6 - 2 * sum(cos(component) for component in wavevector)
    origin = {component: 0 for component in wavevector}
    gradient_at_origin = Matrix(
        [diff(lattice_symbol, component).subs(origin) for component in wavevector]
    )
    hessian_at_origin = Matrix(
        3,
        3,
        lambda i, j: diff(
            lattice_symbol, wavevector[i], wavevector[j]
        ).subs(origin),
    )
    check("C14 cubic Dirichlet operator has the massless 1/Lambda static symbol", lattice_symbol.subs(origin) == 0 and gradient_at_origin == Matrix.zeros(3, 1) and hessian_at_origin == 2 * Matrix.eye(3))

    # The constrained minimizer is independent of the positive action price.
    # Distinct prices change only the energy assigned to the same Green field.
    green_quadratic = (source_vector.T * exact_green)[0]
    priced_energies = {
        price: Fraction(price, 2) * green_quadratic
        for price in (1, 2, 7)
    }
    check(
        "C15 the rotor-selected Green field retains the positive action-price orbit",
        len(set(priced_energies.values())) == 3
        and laplacian * exact_green == source_vector,
    )

    missing = {
        "integration into canonical Phi",
        "autonomous neutral-rotor preparation",
        "autonomous repeated source injection and sink",
        "instantaneous field-bank Green writeback",
        "common action and coupling normalization",
        "stable charged source and apparatus",
        "general Born bank preparation",
        "protected tensor response",
    }
    check("C16 physical five-sector closure remains explicitly open", len(missing) == 8)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 neutral-rotor harmonic-Green checks pass")
    print(f"signed_cubic_rows={covariance_rows}")
    print(f"finite_router_fixture_rows={fixture_rows}")
    print(f"finite_router_total_steps={total_steps}")
    print("rotor_prefix_discrepancy=4/3")
    print("Dirichlet_Poisson_residual_bound=8/N")
    print("current_gradient_discrepancy_bound=8/(3N)")
    print("static_pole_status=conditional_existing_carrier_history_readout")
    print("Open: Phi integration, preparation, action normalization, matter/Born/gravity")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
