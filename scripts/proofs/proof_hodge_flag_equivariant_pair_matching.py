#!/usr/bin/env python3
"""Exact O_h-equivariant matching census for Hodge-flag field sectors.

This script attempts to lift the complete (E,B)-preserving pair relation to a
single fixed-point-free O_h-equivariant involution.  Pair-state orbits are
classified by exact stabilizers.  Compatible distinct orbits are paired by an
exact graph matching; self-orbit involutions are admitted only when an explicit
group element squares into the representative stabilizer.

No physical target or numerical parameter search is involved.
"""

from __future__ import annotations

from collections import Counter, defaultdict, deque
from itertools import combinations

import networkx as nx
from sympy import Matrix
from sympy.polys.domains import GF
from sympy.polys.matrices import DomainMatrix

from proof_hodge_flag_pair_collision_invariant_space import (
    add,
    field_value,
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import signed_permutation_matrices


Pair = tuple[int, int]
CERTIFICATE_DATA = None


def canonical_pair(left: int, right: int) -> Pair:
    assert left != right
    return (left, right) if left < right else (right, left)


def main() -> None:
    global CERTIFICATE_DATA
    checks = 0
    states = one_particle_states()
    state_index = {state: index for index, state in enumerate(states)}
    group = tuple(signed_permutation_matrices())
    actions = tuple(
        tuple(state_index[transform_state(matrix, state)] for state in states)
        for matrix in group
    )
    assert len(states) == 192
    assert len(group) == 48
    assert all(len(set(action)) == 192 for action in actions)
    checks += 3

    pairs = tuple(combinations(range(len(states)), 2))
    pair_set = set(pairs)
    assert len(pairs) == 18336
    checks += 1

    def transform_pair(action, pair: Pair) -> Pair:
        return canonical_pair(action[pair[0]], action[pair[1]])

    field_keys = tuple(field_value(state) for state in states)

    def pair_field(pair: Pair):
        return add(field_keys[pair[0]], field_keys[pair[1]])

    sectors: defaultdict[tuple[int, ...], list[Pair]] = defaultdict(list)
    for pair in pairs:
        sectors[pair_field(pair)].append(pair)

    orbit_id: dict[Pair, int] = {}
    representatives: list[Pair] = []
    orbits: list[tuple[Pair, ...]] = []
    unseen = set(pairs)
    while unseen:
        seed = min(unseen)
        orbit = tuple(sorted({transform_pair(action, seed) for action in actions}))
        identifier = len(orbits)
        for pair in orbit:
            orbit_id[pair] = identifier
            unseen.remove(pair)
        representatives.append(orbit[0])
        orbits.append(orbit)

    orbit_histogram = Counter(len(orbit) for orbit in orbits)
    assert sum(map(len, orbits)) == len(pairs)
    checks += 1

    stabilizers = []
    for representative in representatives:
        stabilizer = tuple(
            index
            for index, action in enumerate(actions)
            if transform_pair(action, representative) == representative
        )
        assert len(stabilizer) * len(orbits[orbit_id[representative]]) == len(group)
        stabilizers.append(stabilizer)
        checks += 1

    # Candidate orbit maps.  A representative may map only to a state in the
    # same exact field sector fixed by its stabilizer and with no larger
    # stabilizer.  This is necessary and sufficient for a well-defined
    # equivariant bijection between the two transitive O_h sets.
    edge_witness: dict[tuple[int, int], tuple[int, Pair]] = {}
    self_witness: dict[int, Pair] = {}
    self_candidate_options: defaultdict[int, list[Pair]] = defaultdict(list)
    for identifier, representative in enumerate(representatives):
        stabilizer = stabilizers[identifier]
        sector = sectors[pair_field(representative)]
        for candidate in sector:
            if candidate == representative:
                continue
            if not all(
                transform_pair(actions[group_index], candidate) == candidate
                for group_index in stabilizer
            ):
                continue
            candidate_stabilizer_size = sum(
                transform_pair(action, candidate) == candidate for action in actions
            )
            if candidate_stabilizer_size != len(stabilizer):
                continue
            target_identifier = orbit_id[candidate]
            if target_identifier == identifier:
                # Find an explicit transporter whose square fixes the source.
                valid = False
                for action in actions:
                    if transform_pair(action, representative) != candidate:
                        continue
                    if transform_pair(action, candidate) == representative:
                        valid = True
                        break
                if valid:
                    self_witness.setdefault(identifier, candidate)
                    if candidate not in self_candidate_options[identifier]:
                        self_candidate_options[identifier].append(candidate)
                continue
            key = tuple(sorted((identifier, target_identifier)))
            edge_witness.setdefault(key, (identifier, candidate))

    graph = nx.Graph()
    graph.add_nodes_from(range(len(orbits)))
    graph.add_edges_from(edge_witness)

    # Self-matchable orbits require no partner.  First retain all of them, then
    # ask for a perfect matching on the genuinely unmatched remainder.
    unmatched_nodes = set(range(len(orbits))) - set(self_witness)
    induced = graph.subgraph(unmatched_nodes).copy()
    matching = nx.algorithms.matching.max_weight_matching(induced, maxcardinality=True)
    matched_nodes = {node for edge in matching for node in edge}
    unmatched_after = unmatched_nodes - matched_nodes

    print(f"pair_orbits={len(orbits)}, orbit_histogram={sorted(orbit_histogram.items())}")
    print(f"self_matchable_orbits={len(self_witness)}")
    print(
        "self_candidate_histogram="
        f"{sorted(Counter(map(len, self_candidate_options.values())).items())}"
    )
    print(f"distinct_compatible_edges={len(edge_witness)}")
    print(f"nodes_requiring_distinct_partner={len(unmatched_nodes)}")
    print(f"distinct_matching_edges={len(matching)}, unmatched_after={len(unmatched_after)}")

    assert not unmatched_after
    checks += 1

    # Every orbit is self-matchable in this census.  Choose among its exact
    # self-involutions by a deterministic finite-field rank-greedy rule.  The
    # rule knows only transition independence, not any physical coefficient or
    # target spectrum.  Final rank and every invariant are rechecked over Q.
    assert len(self_candidate_options) == len(orbits)
    prime = 1_000_003

    def reduced_add(basis: dict[int, tuple[int, ...]], row: list[int]) -> bool:
        vector = [entry % prime for entry in row]
        for pivot in sorted(basis):
            if vector[pivot] == 0:
                continue
            factor = vector[pivot]
            basis_row = basis[pivot]
            vector = [
                (entry - factor * basis_entry) % prime
                for entry, basis_entry in zip(vector, basis_row)
            ]
        pivot = next((index for index, entry in enumerate(vector) if entry), None)
        if pivot is None:
            return False
        inverse = pow(vector[pivot], prime - 2, prime)
        normalized = tuple((entry * inverse) % prime for entry in vector)
        basis[pivot] = normalized
        return True

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

    def option_rows(identifier: int, candidate: Pair) -> tuple[list[int], ...]:
        rows = []
        for source, output in option_transitions(identifier, candidate):
            row = [0] * len(states)
            for index in output:
                row[index] += 1
            for index in source:
                row[index] -= 1
            rows.append(row)
        return tuple(rows)

    option_row_cache = {
        (identifier, candidate): option_rows(identifier, candidate)
        for identifier, candidates in self_candidate_options.items()
        for candidate in candidates
    }
    union_basis: dict[int, tuple[int, ...]] = {}
    for rows in option_row_cache.values():
        for row in rows:
            reduced_add(union_basis, row)
    print(f"all_self_options_union_rank={len(union_basis)}")

    union_matrix = Matrix([list(union_basis[pivot]) for pivot in sorted(union_basis)])
    union_domain = DomainMatrix.from_Matrix(union_matrix).convert_to(GF(prime))
    quotient_nullspace = union_domain.nullspace().to_Matrix()
    assert quotient_nullspace.shape == (10, 192)

    def small_modular_rank(rows: list[list[int]]) -> int:
        basis: dict[int, tuple[int, ...]] = {}
        for row in rows:
            reduced_add(basis, row)
        return len(basis)

    distinct_gain_histogram = Counter()
    distinct_projected_rows: dict[tuple[int, int], tuple[list[int], ...]] = {}
    best_distinct_edges = []
    best_distinct_gain = -1
    for edge in sorted(edge_witness):
        source_identifier, candidate = edge_witness[edge]
        projected_rows = []
        for source, output in option_transitions(source_identifier, candidate):
            projected_rows.append(
                [
                    (
                        quotient_nullspace[null_index, output[0]]
                        + quotient_nullspace[null_index, output[1]]
                        - quotient_nullspace[null_index, source[0]]
                        - quotient_nullspace[null_index, source[1]]
                    )
                    % prime
                    for null_index in range(quotient_nullspace.rows)
                ]
            )
        gain = small_modular_rank(projected_rows)
        distinct_projected_rows[edge] = tuple(projected_rows)
        distinct_gain_histogram[gain] += 1
        if gain > best_distinct_gain:
            best_distinct_gain = gain
            best_distinct_edges = [edge]
        elif gain == best_distinct_gain:
            best_distinct_edges.append(edge)
    print(f"distinct_edge_union_gain_histogram={sorted(distinct_gain_histogram.items())}")
    print(
        f"best_distinct_gain={best_distinct_gain}, "
        f"best_distinct_edges={best_distinct_edges[:12]}"
    )

    # A single distinct-orbit exchange contributes at most one of the three
    # missing quotient directions.  Select a deterministic disjoint set whose
    # projected rows span all three.  Disjointness is required because a pair
    # orbit can participate in only one involutive exchange.
    quotient_basis: dict[int, tuple[int, ...]] = {}
    selected_cross_edges: list[tuple[int, int]] = []
    occupied_cross_orbits: set[int] = set()
    while len(quotient_basis) < 3:
        best_edge = None
        best_basis = None
        best_gain = -1
        for edge in sorted(distinct_projected_rows):
            if any(identifier in occupied_cross_orbits for identifier in edge):
                continue
            trial_basis = dict(quotient_basis)
            old_rank = len(trial_basis)
            for row in distinct_projected_rows[edge]:
                reduced_add(trial_basis, row)
            gain = len(trial_basis) - old_rank
            if gain > best_gain:
                best_edge = edge
                best_basis = trial_basis
                best_gain = gain
        assert best_edge is not None and best_basis is not None and best_gain > 0
        selected_cross_edges.append(best_edge)
        occupied_cross_orbits.update(best_edge)
        quotient_basis = best_basis
    print(
        f"selected_cross_edges={selected_cross_edges}, "
        f"quotient_rank={len(quotient_basis)}"
    )

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
                transform_pair(actions[group_index], option) == option
                for group_index in stabilizers[source_identifier]
            )
        )

    # Seed the actual collision rank with the three selected cross-orbit maps,
    # then choose one self-involution for every remaining orbit.
    modular_basis: dict[int, tuple[int, ...]] = {}
    selected_cross_targets: dict[tuple[int, int], Pair] = {}
    for left_identifier, right_identifier in selected_cross_edges:
        target = compatible_target(left_identifier, right_identifier)
        selected_cross_targets[(left_identifier, right_identifier)] = target
        for row in option_rows(left_identifier, target):
            reduced_add(modular_basis, row)
    chosen_self_witness: dict[int, Pair] = {}
    processing_order = sorted(
        range(len(orbits)),
        key=lambda identifier: (
            len(self_candidate_options[identifier]) != 1,
            len(self_candidate_options[identifier]),
            identifier,
        ),
    )
    greedy_gain_histogram = Counter()
    for identifier in processing_order:
        if identifier in occupied_cross_orbits:
            continue
        best_candidate = None
        best_basis = None
        best_gain = -1
        for candidate in sorted(self_candidate_options[identifier]):
            trial_basis = dict(modular_basis)
            old_rank = len(trial_basis)
            for row in option_row_cache[(identifier, candidate)]:
                reduced_add(trial_basis, row)
            gain = len(trial_basis) - old_rank
            if gain > best_gain:
                best_candidate = candidate
                best_basis = trial_basis
                best_gain = gain
        assert best_candidate is not None and best_basis is not None
        chosen_self_witness[identifier] = best_candidate
        modular_basis = best_basis
        greedy_gain_histogram[best_gain] += 1

    print(f"rank_greedy_modular_rank={len(modular_basis)}")
    print(f"rank_greedy_gain_histogram={sorted(greedy_gain_histogram.items())}")

    # Build the global involution from self-orbit and paired-orbit witnesses.
    collision: dict[Pair, Pair] = {}

    def install_orbit_map(source_rep: Pair, target: Pair) -> None:
        for action in actions:
            source = transform_pair(action, source_rep)
            output = transform_pair(action, target)
            if source in collision:
                assert collision[source] == output
            else:
                collision[source] = output

    for identifier, candidate in chosen_self_witness.items():
        install_orbit_map(representatives[identifier], candidate)

    for left_identifier, right_identifier in selected_cross_edges:
        left_rep = representatives[left_identifier]
        candidate_in_other = selected_cross_targets[(left_identifier, right_identifier)]
        install_orbit_map(left_rep, candidate_in_other)
        # Install the exact inverse map using the matched target as source.
        install_orbit_map(candidate_in_other, left_rep)

    assert set(collision) == pair_set
    assert set(collision.values()) == pair_set
    assert all(collision[collision[pair]] == pair for pair in pairs)
    assert all(collision[pair] != pair for pair in pairs)
    checks += 4

    for pair in pairs:
        outgoing = collision[pair]
        assert pair_field(outgoing) == pair_field(pair)
        for action in actions:
            assert collision[transform_pair(action, pair)] == transform_pair(
                action, outgoing
            )
            checks += 1

    selected_rows = []
    for pair in pairs:
        outgoing = collision[pair]
        row = [0] * len(states)
        for index in outgoing:
            row[index] += 1
        for index in pair:
            row[index] -= 1
        selected_rows.append(row)
    selected_transition = Matrix(selected_rows)
    selected_rank = DomainMatrix.from_Matrix(selected_transition).rank()
    selected_nullity = len(states) - selected_rank
    print(f"selected_transition_rank={selected_rank}, selected_nullity={selected_nullity}")
    assert selected_rank == 185
    assert selected_nullity == 7
    checks += 2

    # Expose the exact selected map to downstream full-tick certificates.  It
    # is populated only after all construction and rank assertions pass.
    CERTIFICATE_DATA = {
        "states": states,
        "collision": collision,
        "orbits": orbits,
        "selected_cross_edges": tuple(selected_cross_edges),
    }

    print(f"PASS: Hodge-flag O_h-equivariant pair matching ({checks} exact checks)")
    print("global collision is fixed-point-free, involutive, field-preserving, and O_h-equivariant")
    print("Open: parity-twisted C4/time-reversal covariance and full streaming kernel")


if __name__ == "__main__":
    main()
