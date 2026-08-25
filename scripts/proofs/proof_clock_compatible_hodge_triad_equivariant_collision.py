#!/usr/bin/env python3
"""Exact O_h x C12-equivariant collision for the Hodge-triad readout.

The clock-compatible triad field has exactly seven possible additive
invariants under its complete pair relation.  This certificate asks the
stronger constructive question: does one fixed-point-free deterministic pair
involution preserve that field and commute with both all signed cubic actions
and the complete internal C3-flag x C4-phase tick?

The construction classifies pair-state group orbits and chooses exact
stabilizer-compatible orbit involutions by a deterministic finite-field
rank-greedy rule.  No physical coefficient, dispersion target, or measured
number is used.
"""

from __future__ import annotations

from collections import Counter, defaultdict
from itertools import combinations

import networkx as nx
from sympy import Matrix
from sympy.polys.domains import GF
from sympy.polys.matrices import DomainMatrix

from proof_clock_compatible_hodge_triad_readout import (
    internal_tick,
    triad_readout,
)
from proof_hodge_flag_pair_collision_invariant_space import add, one_particle_states
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_shared_edge_hodge_flag_bcc_propagation import transform_flag


Pair = tuple[int, int]
CERTIFICATE_DATA = None


def canonical_pair(left: int, right: int) -> Pair:
    assert left != right
    return (left, right) if left < right else (right, left)


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def main() -> None:
    global CERTIFICATE_DATA
    checks = 0
    states = one_particle_states()
    state_index = {state: index for index, state in enumerate(states)}
    spatial_group = tuple(signed_permutation_matrices())
    size = len(states)
    assert size == 192
    checks += 1

    spatial_actions = tuple(
        tuple(
            state_index[(transform_flag(matrix, state[0]), state[1])]
            for state in states
        )
        for matrix in spatial_group
    )
    internal_action = tuple(state_index[internal_tick(state)] for state in states)
    internal_powers = [tuple(range(size))]
    for _ in range(11):
        previous = internal_powers[-1]
        internal_powers.append(tuple(internal_action[index] for index in previous))
    assert tuple(internal_action[index] for index in internal_powers[-1]) == tuple(
        range(size)
    )

    actions = tuple(
        tuple(internal_power[spatial[index]] for index in range(size))
        for spatial in spatial_actions
        for internal_power in internal_powers
    )
    assert len(actions) == 576
    assert len(set(actions)) == 576
    assert all(len(set(action)) == size for action in actions)
    checks += 4

    pairs = tuple(combinations(range(size), 2))
    pair_set = set(pairs)

    def transform_pair(action, pair: Pair) -> Pair:
        return canonical_pair(action[pair[0]], action[pair[1]])

    field_keys = tuple(triad_readout(state) for state in states)

    def pair_field(pair: Pair):
        return add(field_keys[pair[0]], field_keys[pair[1]])

    sectors: defaultdict[tuple[int, ...], list[Pair]] = defaultdict(list)
    for pair in pairs:
        sectors[pair_field(pair)].append(pair)
    assert len(sectors) == 117
    checks += 1

    orbit_id: dict[Pair, int] = {}
    representatives: list[Pair] = []
    orbits: list[tuple[Pair, ...]] = []
    unseen = set(pairs)
    while unseen:
        seed = min(unseen)
        orbit = tuple(sorted({transform_pair(action, seed) for action in actions}))
        identifier = len(orbits)
        for pair in orbit:
            assert pair in unseen
            orbit_id[pair] = identifier
            unseen.remove(pair)
        representatives.append(orbit[0])
        orbits.append(orbit)
    orbit_histogram = Counter(map(len, orbits))
    assert sum(map(len, orbits)) == len(pairs)
    checks += 1

    stabilizers = []
    for identifier, representative in enumerate(representatives):
        stabilizer = tuple(
            action_index
            for action_index, action in enumerate(actions)
            if transform_pair(action, representative) == representative
        )
        assert len(stabilizer) * len(orbits[identifier]) == len(actions)
        stabilizers.append(stabilizer)
        checks += 1

    self_options: defaultdict[int, list[Pair]] = defaultdict(list)
    edge_witness: dict[tuple[int, int], tuple[int, Pair]] = {}
    for identifier, representative in enumerate(representatives):
        stabilizer = stabilizers[identifier]
        for candidate in sectors[pair_field(representative)]:
            if candidate == representative:
                continue
            if not all(
                transform_pair(actions[action_index], candidate) == candidate
                for action_index in stabilizer
            ):
                continue
            candidate_stabilizer_size = sum(
                transform_pair(action, candidate) == candidate for action in actions
            )
            if candidate_stabilizer_size != len(stabilizer):
                continue
            target_identifier = orbit_id[candidate]
            if target_identifier == identifier:
                if any(
                    transform_pair(action, representative) == candidate
                    and transform_pair(action, candidate) == representative
                    for action in actions
                ):
                    if candidate not in self_options[identifier]:
                        self_options[identifier].append(candidate)
            else:
                key = tuple(sorted((identifier, target_identifier)))
                edge_witness.setdefault(key, (identifier, candidate))

    graph = nx.Graph()
    graph.add_nodes_from(range(len(orbits)))
    graph.add_edges_from(edge_witness)
    unmatched_nodes = set(range(len(orbits))) - set(self_options)
    matching = nx.algorithms.matching.max_weight_matching(
        graph.subgraph(unmatched_nodes), maxcardinality=True
    )
    matched_nodes = {node for edge in matching for node in edge}
    unmatched_after = unmatched_nodes - matched_nodes

    print(f"pair_orbits={len(orbits)}, orbit_histogram={sorted(orbit_histogram.items())}")
    print(f"self_matchable_orbits={len(self_options)}")
    print(f"self_option_histogram={sorted(Counter(map(len, self_options.values())).items())}")
    print(f"distinct_compatible_edges={len(edge_witness)}")
    print(
        f"required_distinct_nodes={len(unmatched_nodes)}, "
        f"matching_edges={len(matching)}, unmatched_after={len(unmatched_after)}"
    )
    assert not unmatched_after
    checks += 1

    def option_transitions(identifier: int, candidate: Pair):
        source_rep = representatives[identifier]
        mapping = {}
        for action in actions:
            source = transform_pair(action, source_rep)
            output = transform_pair(action, candidate)
            if source in mapping:
                assert mapping[source] == output
            mapping[source] = output
        assert set(mapping) == set(orbits[identifier])
        return tuple(sorted(mapping.items()))

    def option_rows(identifier: int, candidate: Pair):
        rows = []
        for source, output in option_transitions(identifier, candidate):
            row = [0] * size
            for index in output:
                row[index] += 1
            for index in source:
                row[index] -= 1
            rows.append(row)
        return tuple(rows)

    prime = 1_000_003

    def reduced_add(basis: dict[int, tuple[int, ...]], row) -> bool:
        vector = [int(entry) % prime for entry in row]
        for pivot in sorted(basis):
            if vector[pivot] == 0:
                continue
            factor = vector[pivot]
            vector = [
                (entry - factor * basis_entry) % prime
                for entry, basis_entry in zip(vector, basis[pivot])
            ]
        pivot = next((index for index, entry in enumerate(vector) if entry), None)
        if pivot is None:
            return False
        inverse = pow(vector[pivot], prime - 2, prime)
        basis[pivot] = tuple(entry * inverse % prime for entry in vector)
        return True

    option_cache = {
        (identifier, candidate): option_rows(identifier, candidate)
        for identifier, candidates in self_options.items()
        for candidate in candidates
    }

    # Install mandatory distinct matches first, then greedily maximize the
    # transition rank using one exact self-involution on every remaining orbit.
    collision: dict[Pair, Pair] = {}
    modular_basis: dict[int, tuple[int, ...]] = {}
    occupied = set(matched_nodes)

    def install_orbit_map(source_rep: Pair, target: Pair) -> None:
        for action in actions:
            source = transform_pair(action, source_rep)
            output = transform_pair(action, target)
            if source in collision:
                assert collision[source] == output
            collision[source] = output

    def compatible_target(source_identifier: int, target_identifier: int) -> Pair:
        key = tuple(sorted((source_identifier, target_identifier)))
        witness_source, candidate = edge_witness[key]
        if witness_source == source_identifier and orbit_id[candidate] == target_identifier:
            return candidate
        source_rep = representatives[source_identifier]
        return next(
            option
            for option in sectors[pair_field(source_rep)]
            if orbit_id[option] == target_identifier
            and all(
                transform_pair(actions[action_index], option) == option
                for action_index in stabilizers[source_identifier]
            )
        )

    for left_identifier, right_identifier in sorted(tuple(sorted(edge)) for edge in matching):
        target = compatible_target(left_identifier, right_identifier)
        for row in option_rows(left_identifier, target):
            reduced_add(modular_basis, row)
        install_orbit_map(representatives[left_identifier], target)
        install_orbit_map(target, representatives[left_identifier])

    chosen_self: dict[int, Pair] = {}
    processing_order = sorted(
        set(range(len(orbits))) - occupied,
        key=lambda identifier: (len(self_options[identifier]), identifier),
    )
    gain_histogram = Counter()
    for identifier in processing_order:
        best_candidate = None
        best_basis = None
        best_gain = -1
        for candidate in sorted(self_options[identifier]):
            trial_basis = dict(modular_basis)
            old_rank = len(trial_basis)
            for row in option_cache[(identifier, candidate)]:
                reduced_add(trial_basis, row)
            gain = len(trial_basis) - old_rank
            if gain > best_gain:
                best_candidate = candidate
                best_basis = trial_basis
                best_gain = gain
        assert best_candidate is not None and best_basis is not None
        chosen_self[identifier] = best_candidate
        modular_basis = best_basis
        gain_histogram[best_gain] += 1
        install_orbit_map(representatives[identifier], best_candidate)

    print(f"rank_greedy_modular_rank={len(modular_basis)}")
    print(f"rank_greedy_gain_histogram={sorted(gain_histogram.items())}")

    assert set(collision) == pair_set
    assert set(collision.values()) == pair_set
    assert all(collision[collision[pair]] == pair for pair in pairs)
    assert all(collision[pair] != pair for pair in pairs)
    checks += 4

    # The four generators below generate O_h x C12.  Full equivariance also
    # follows from orbit installation using all 576 actions; generator checks
    # provide an independent end-to-end verification at lower output cost.
    generator_matrices = (
        ((0, 1, 0), (1, 0, 0), (0, 0, 1)),
        ((0, 1, 0), (0, 0, 1), (1, 0, 0)),
        ((-1, 0, 0), (0, 1, 0), (0, 0, 1)),
    )
    generator_actions = [
        tuple(
            state_index[(transform_flag(matrix, state[0]), state[1])]
            for state in states
        )
        for matrix in generator_matrices
    ] + [internal_action]

    for pair in pairs:
        outgoing = collision[pair]
        assert pair_field(outgoing) == pair_field(pair)
        for action in generator_actions:
            assert collision[transform_pair(action, pair)] == transform_pair(
                action, outgoing
            )
            checks += 1

    selected_rows = []
    for pair in pairs:
        outgoing = collision[pair]
        row = [0] * size
        for index in outgoing:
            row[index] += 1
        for index in pair:
            row[index] -= 1
        selected_rows.append(row)
    selected_transition = Matrix(selected_rows)
    selected_rank = exact_rank(selected_transition)
    selected_nullity = size - selected_rank
    print(f"selected_transition_rank={selected_rank}, nullity={selected_nullity}")

    CERTIFICATE_DATA = {
        "states": states,
        "collision": collision,
        "actions": actions,
        "orbits": orbits,
    }

    print(
        "PASS: clock-compatible Hodge-triad equivariant collision "
        f"({checks} exact checks)"
    )
    print(
        "Open: exact product-reference kernel and finite-k Maxwell/Hodge test"
    )


if __name__ == "__main__":
    main()
