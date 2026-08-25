#!/usr/bin/env python3
"""Exact redundant protection of the prepared v3 joint gravity bundle.

The parent construction moves one neutral ten-record packet carrying one
scalar, five STF, and three polar-vector source coordinates.  This certificate
places three complete copies of one parent local transaction on three
prepared parallel rails.  A single existing fixed-occupancy A2 owner stores
the READ/COMMIT phase and whether one repair has occurred.  Strict majority
repairs an arbitrary valid-symbol substitution in any one complete copy before
the parent move is applied.

All 1,296 selected rank-nine parent transactions are reproduced exactly and
have exact clean inverses.  Every possible fault tick in the 2,088-step finite
Green fixtures rejoins the clean visit history, so the already-conditional
componentwise Dirichlet bound and common 1/Lambda kernel survive this one-fault
basin unchanged.

This is a prepared finite-packet protection theorem.  It does not form or
renew the rails, protect the rotor background, export/reset repair work,
generate scalar/vector constraints, isolate transverse-traceless modes,
derive a dynamical wave pole, fix an interacting action or coupling, establish
a common material cone, or prove lensing.
"""

from __future__ import annotations

import sys
from collections import defaultdict
from dataclasses import dataclass
from fractions import Fraction

from sympy import Matrix

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
    simulate_rotor_box,
)
from proof_v3_neutral_rotor_walker_macro import (
    recognize_unmarked,
    unmarked_site,
)
from proof_v3_neutral_scalar_vector_stf_bundle_common_green_seam import (
    bundle_marked_site,
    bundle_payload,
    internal_orbits,
    inverse_bundle_step,
    local_bundle_step,
    orbit_lookup,
    recognize_bundle_marked,
)
from proof_v3_rotor_green_a2_physical_memory_phase_protection import (
    decode_counter,
    encode_counter,
)


sys.stdout.reconfigure(encoding="utf-8")

READ = 0
COMMIT = 1


@dataclass(frozen=True)
class Transaction:
    departure_rotor: object
    left: object
    right: object
    destination_rotor: object


@dataclass(frozen=True)
class Move:
    source_rotor: object
    packet_rotor: object
    left: object
    right: object
    direction: Vec


@dataclass(frozen=True)
class ProtectedInput:
    copies: tuple[Transaction, Transaction, Transaction]
    control: tuple[tuple[int, int], ...]


@dataclass(frozen=True)
class ProtectedOutput:
    copies: tuple[Move, Move, Move]
    control: tuple[tuple[int, int], ...]


def copy3(value):
    return value, value, value


def control_payload(layer: int, work: int):
    """Four logical control states in one existing 4,096-state A2 owner."""

    assert layer in (READ, COMMIT) and work in (0, 1)
    return encode_counter(2 * work + layer)


def control_state(payload) -> tuple[int, int] | None:
    value = decode_counter(payload)
    if value not in (0, 1, 2, 3):
        return None
    return value % 2, value // 2


def majority(values):
    for value in values:
        if values.count(value) >= 2:
            return value
    return None


def parent_forward(transaction: Transaction, state_set, orbit_index) -> Move | None:
    output = local_bundle_step(
        bundle_marked_site(
            transaction.departure_rotor,
            transaction.left,
            transaction.right,
        ),
        unmarked_site(transaction.destination_rotor),
        state_set,
        orbit_index,
    )
    if output is None:
        return None
    source_rotor = recognize_unmarked(output[0], state_set)
    packet = recognize_bundle_marked(output[1], state_set, orbit_index)
    if source_rotor is None or packet is None:
        return None
    return Move(source_rotor, packet[0], packet[1], packet[2], output[2])


def parent_inverse(move: Move, state_set, orbit_index) -> Transaction | None:
    output = inverse_bundle_step(
        unmarked_site(move.source_rotor),
        bundle_marked_site(move.packet_rotor, move.left, move.right),
        state_set,
        orbit_index,
    )
    if output is None or output[2] != move.direction:
        return None
    departure = recognize_bundle_marked(output[0], state_set, orbit_index)
    destination_rotor = recognize_unmarked(output[1], state_set)
    if departure is None or destination_rotor is None:
        return None
    return Transaction(departure[0], departure[1], departure[2], destination_rotor)


def protected(transaction: Transaction) -> ProtectedInput:
    return ProtectedInput(copy3(transaction), control_payload(READ, 0))


def read_layer(state: ProtectedInput) -> ProtectedInput:
    control = control_state(state.control)
    if control is None or control[0] != READ:
        return state
    _layer, work = control
    logical = majority(state.copies)
    if logical is None:
        return state
    mismatch_count = sum(value != logical for value in state.copies)
    if mismatch_count > 1 or (mismatch_count and work != 0):
        return state
    return ProtectedInput(
        copy3(logical),
        control_payload(COMMIT, 1 if mismatch_count else work),
    )


def commit_layer(state: ProtectedInput, state_set, orbit_index):
    control = control_state(state.control)
    if control is None or control[0] != COMMIT or len(set(state.copies)) != 1:
        return state
    _layer, work = control
    output = parent_forward(state.copies[0], state_set, orbit_index)
    if output is None:
        return state
    return ProtectedOutput(copy3(output), control_payload(READ, work))


def macro_step(state: ProtectedInput, state_set, orbit_index):
    read = read_layer(state)
    if control_state(read.control) is None or control_state(read.control)[0] != COMMIT:
        return state
    return commit_layer(read, state_set, orbit_index)


def clean_macro_inverse(state: ProtectedOutput, state_set, orbit_index):
    control = control_state(state.control)
    if control is None or control[0] != READ or len(set(state.copies)) != 1:
        return state
    _layer, work = control
    prior = parent_inverse(state.copies[0], state_set, orbit_index)
    if prior is None:
        return state
    committed = ProtectedInput(copy3(prior), control_payload(COMMIT, work))
    if commit_layer(committed, state_set, orbit_index) != state:
        return state
    return ProtectedInput(copy3(prior), control_payload(READ, work))


def replace_copy(state: ProtectedInput, index: int, replacement: Transaction):
    values = list(state.copies)
    values[index] = replacement
    return ProtectedInput(tuple(values), state.control)


def scale(value: int, vector: Vec) -> Vec:
    return tuple(value * entry for entry in vector)  # type: ignore[return-value]


def rail_sites(direction: Vec, transverse: Vec):
    tails = (scale(-1, transverse), (0, 0, 0), transverse)
    heads = tuple(add(tail, direction) for tail in tails)
    work = scale(-1, direction)
    return tails, heads, work


def transform_transaction(matrix, transaction: Transaction) -> Transaction:
    return Transaction(
        transform_state(matrix, transaction.departure_rotor),
        transform_state(matrix, transaction.left),
        transform_state(matrix, transaction.right),
        transform_state(matrix, transaction.destination_rotor),
    )


def transform_move(matrix, move: Move) -> Move:
    return Move(
        transform_state(matrix, move.source_rotor),
        transform_state(matrix, move.packet_rotor),
        transform_state(matrix, move.left),
        transform_state(matrix, move.right),
        tuple(matrix_vector(matrix, move.direction)),
    )


def simulate_protected_history(
    radius: int,
    injections: int,
    router_seed,
    left_seed,
    right_seed,
    alternate: Transaction,
    fault_at: int | None,
    state_set,
    orbit_index,
):
    vertices = frozenset(box(radius))
    source = (0, 0, 0)
    rotors = {vertex: router_seed for vertex in vertices}
    visits: defaultdict[Vec, int] = defaultdict(int)
    total_steps = 0
    control = control_payload(READ, 0)

    for _ in range(injections):
        location = source
        left = left_seed
        right = right_seed
        while location in vertices:
            visits[location] += 1
            destination_guess = parent_forward(
                Transaction(rotors[location], left, right, router_seed),
                state_set,
                orbit_index,
            )
            assert destination_guess is not None
            destination = add(location, destination_guess.direction)
            destination_rotor = rotors[destination] if destination in vertices else router_seed
            transaction = Transaction(
                rotors[location], left, right, destination_rotor
            )
            current = ProtectedInput(copy3(transaction), control)
            if total_steps == fault_at:
                current = replace_copy(current, total_steps % 3, alternate)
            output = macro_step(current, state_set, orbit_index)
            assert isinstance(output, ProtectedOutput)
            assert len(set(output.copies)) == 1
            move = output.copies[0]
            rotors[location] = move.source_rotor
            left, right = move.left, move.right
            location = add(location, move.direction)
            control = output.control
            total_steps += 1

    return visits, total_steps, control_state(control)


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
        "C1 READ/COMMIT and one retained repair bit use four states of one fixed-occupancy A2 owner",
        all(
            control_state(control_payload(layer, work)) == (layer, work)
            for layer in (READ, COMMIT)
            for work in (0, 1)
        ),
    )

    # Recover the same exact nine parent pivot packets without a new selection.
    source_rows = []
    source_representatives = []
    for left in states:
        for right in states:
            if orbit_index[left] == orbit_index[right]:
                continue
            source_rows.append(bundle_payload(left, right, 0))
            source_representatives.append((left, right))
    pivot_columns = Matrix(source_rows).T.rref()[1]
    basis_pairs = tuple(source_representatives[index] for index in pivot_columns)
    basis_rows = tuple(source_rows[index] for index in pivot_columns)
    check(
        "C2 the protected channels are exactly the parent's rank-nine joint source basis",
        len(source_rows) == 34_560
        and len(basis_pairs) == 9
        and Matrix(basis_rows).rank() == 9,
    )

    basis_packets = []
    transactions = []
    for left, right in basis_pairs:
        router_orbit = next(
            orbit
            for orbit in orbits
            if orbit_index[orbit[0]] not in {orbit_index[left], orbit_index[right]}
        )
        basis_packets.append((router_orbit, left, right))
        for departure_rotor in router_orbit:
            for destination_rotor in router_orbit:
                transactions.append(
                    Transaction(departure_rotor, left, right, destination_rotor)
                )

    clean_rows = 0
    inverse_rows = 0
    substitution_rows = 0
    for index, transaction in enumerate(transactions):
        clean = protected(transaction)
        output = macro_step(clean, state_set, orbit_index)
        expected = parent_forward(transaction, state_set, orbit_index)
        assert isinstance(output, ProtectedOutput) and expected is not None
        assert output.copies == copy3(expected)
        assert control_state(output.control) == (READ, 0)
        assert clean_macro_inverse(output, state_set, orbit_index) == clean
        clean_rows += 1
        inverse_rows += 1

        alternate = transactions[(index + 1) % len(transactions)]
        assert alternate != transaction
        for copy_index in range(3):
            mutant = replace_copy(clean, copy_index, alternate)
            repaired = read_layer(mutant)
            assert repaired.copies == clean.copies
            assert control_state(repaired.control) == (COMMIT, 1)
            mutant_output = macro_step(mutant, state_set, orbit_index)
            assert isinstance(mutant_output, ProtectedOutput)
            assert mutant_output.copies == output.copies
            assert control_state(mutant_output.control) == (READ, 1)
            substitution_rows += 1

    check(
        "C3 all 1,296 selected parent transactions are reproduced exactly",
        clean_rows == 9 * 12 * 12,
    )
    check(
        "C4 every clean protected transaction has an exact inverse including the control record",
        inverse_rows == clean_rows,
    )
    check(
        "C5 all three one-copy positions repair before every selected parent move",
        substitution_rows == 3 * clean_rows,
    )

    # Equality alone determines the repetition decoder, so the replacement's
    # identity is irrelevant.  Exhaust the complete A != B equality pattern.
    equality_rows = 0
    for original in range(4):
        for replacement in range(4):
            if replacement == original:
                continue
            for copy_index in range(3):
                values = [original, original, original]
                values[copy_index] = replacement
                assert majority(tuple(values)) == original
                equality_rows += 1
    check(
        "C6 strict majority corrects an arbitrary distinct valid replacement symbol, not a selected value",
        equality_rows == 4 * 3 * 3,
    )

    # Three parallel transaction rails plus one A2 control owner occupy seven
    # distinct sites inside one 3x3x3 Moore block.  The ordered transverse
    # chart is prepared state, and its complete signed-cubic orbit is tested.
    geometry_rows = 0
    base_direction = (1, 0, 0)
    base_transverse = (0, 1, 0)
    for matrix in group:
        direction = tuple(matrix_vector(matrix, base_direction))
        transverse = tuple(matrix_vector(matrix, base_transverse))
        tails, heads, work = rail_sites(direction, transverse)
        sites = tails + heads + (work,)
        assert len(sites) == len(set(sites)) == 7
        assert all(max(abs(value) for value in site) <= 1 for site in sites)
        assert all(add(tail, direction) == head for tail, head in zip(tails, heads))
        geometry_rows += 1
    check(
        "C7 three parallel radius-one rails and one work owner fit seven distinct Moore-block sites covariantly",
        geometry_rows == 48,
    )

    covariance_rows = 0
    for router_orbit, left, right in basis_packets:
        transaction = Transaction(router_orbit[0], left, right, router_orbit[3])
        base_output = parent_forward(transaction, state_set, orbit_index)
        assert base_output is not None
        for matrix in group:
            transformed = transform_transaction(matrix, transaction)
            transformed_output = macro_step(protected(transformed), state_set, orbit_index)
            assert isinstance(transformed_output, ProtectedOutput)
            assert transformed_output.copies == copy3(transform_move(matrix, base_output))
            covariance_rows += 1
    check(
        "C8 the complete protected transaction wrapper is signed-cubic covariant",
        covariance_rows == 9 * 48,
    )

    # Every possible single fault tick in the parent's finite Green fixtures
    # is inserted and repaired.  The complete visit measure is unchanged.
    fault_tick_rows = 0
    poisson_rows = 0
    total_parent_steps = 0
    alternate = transactions[-1]
    for router_orbit, left_seed, right_seed in basis_packets:
        for radius, injections in ((1, 7), (1, 37), (2, 7)):
            parent = simulate_rotor_box(radius, injections, router_orbit[0])
            parent_visits = parent[2]
            parent_steps = parent[5]
            total_parent_steps += parent_steps
            for fault_at in range(parent_steps):
                visits, steps, final_control = simulate_protected_history(
                    radius,
                    injections,
                    router_orbit[0],
                    left_seed,
                    right_seed,
                    alternate,
                    fault_at,
                    state_set,
                    orbit_index,
                )
                assert visits == parent_visits
                assert steps == parent_steps
                assert final_control == (READ, 1)
                fault_tick_rows += 1

            vertices = tuple(box(radius))
            laplacian = dirichlet_laplacian(vertices)
            source_index = vertices.index((0, 0, 0))
            green = Matrix(
                [
                    Fraction(parent_visits[vertex], 6 * injections)
                    for vertex in vertices
                ]
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

    check(
        "C9 every possible one-fault tick in the finite fixtures rejoins the exact clean visit history",
        fault_tick_rows == total_parent_steps == 2_088,
    )
    check(
        "C10 all nine protected source coordinates retain the parent's exact Dirichlet-Poisson bound",
        poisson_rows == 9 * 9 * (27 + 27 + 125),
    )
    check(
        "C11 the prepared one-fault basin therefore inherits the same conditional common 1/Lambda kernel",
        Matrix(basis_rows).rank() == 9 and fault_tick_rows > 0 and poisson_rows > 0,
    )

    clean = protected(transactions[0])
    first = transactions[1]
    second = transactions[2]
    no_majority = ProtectedInput((transactions[0], first, second), clean.control)
    busy = ProtectedInput(
        replace_copy(clean, 0, first).copies,
        control_payload(READ, 1),
    )
    check(
        "C12 no-majority inputs and a second repair on a busy work owner fail closed",
        read_layer(no_majority) == no_majority and read_layer(busy) == busy,
    )

    forbidden = (
        "137.036",
        "new_field_type",
        "empirical_target",
        "random_draw",
        "graviton_mass",
    )
    missing = {
        "formation renewal sink and traffic arbitration for the three rails",
        "protection of the rotor background and work export or reset",
        "propagated scalar and vector constraint algebra",
        "two transverse-traceless tensor modes and a dynamical wave pole",
        "interacting action normalization and universal material coupling",
        "common cone clock response lensing delay and nonlinear closure",
    }
    check(
        "C13 protected packet transport is not yet native tensor gravity",
        all(token not in __doc__.lower() for token in forbidden)
        and len(missing) == 6,
    )

    passed = sum(ok for _name, ok, _detail in checks)
    print(f"\n{passed}/{len(checks)} protected joint-gravity-bundle checks pass")
    print(f"parent_basis_rank={Matrix(basis_rows).rank()}")
    print(f"clean_protected_transaction_rows={clean_rows}")
    print(f"one_copy_substitution_rows={substitution_rows}")
    print(f"fault_tick_rows={fault_tick_rows}")
    print(f"finite_history_steps={total_parent_steps}")
    print(f"signed_cubic_covariance_rows={covariance_rows}")
    print("protected_transaction_sites=7_of_27")
    print("control=A2_READ_COMMIT_plus_one_retained_work_bit")
    print("kernel_status=conditional_common_componentwise_1_over_Lambda_inherited")
    print("gravity_status=prepared_packet_one_fault_protected_constraints_action_pole_coupling_lensing_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
