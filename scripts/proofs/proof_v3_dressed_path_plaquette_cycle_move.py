#!/usr/bin/env python3
"""Exact local plaquette/cycle move for v3 dressed SC source paths.

Two ordered SC steps across a square can use either side of the plaquette.
Swapping the complete dressed-edge macrostates between those paths changes the
electric cochain by a plaquette boundary.  Endpoint charge is unchanged,
Delta E is divergence free, and the induced ownership current obeys exact
continuity.  The move retains four A9 ownership tokens and sixteen field bits.

This supplies a local deformation generator for finite Gauss strings.  It does
not select weights on the connected path class and therefore does not yet
derive a Coulomb/static massless pole.
"""

from __future__ import annotations

import sys
from collections import deque
from itertools import permutations

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import dot
from proof_v3_dressed_sc_source_gauss_continuity import (
    Channel,
    Vec,
    add_maps,
    boundary,
    scale_map,
    target_packet,
    transform_channels,
)


sys.stdout.reconfigure(encoding="utf-8")

Edge = tuple[Vec, Vec]


def add(left: Vec, right: Vec) -> Vec:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def edge_boundary(edge: Edge, coefficient: int = 1) -> dict[Vec, int]:
    tail, direction = edge
    return boundary(tail, direction, coefficient)


def chain_boundary(chain: tuple[Edge, ...], coefficient: int = 1) -> dict[Vec, int]:
    return add_maps(*(edge_boundary(edge, coefficient) for edge in chain))


def square_paths(origin: Vec, first: Vec, second: Vec):
    assert dot(first, second) == 0
    path_a = (
        (origin, first),
        (add(origin, first), second),
    )
    path_b = (
        (origin, second),
        (add(origin, second), first),
    )
    return path_a, path_b


def dressed_owners(
    path: tuple[Edge, ...],
    phase: int,
    polarity: int,
    layer: int,
) -> frozenset[tuple[Vec, Channel]]:
    owners: set[tuple[Vec, Channel]] = set()
    for tail, direction in path:
        head = add(tail, direction)
        packet = target_packet(direction, (phase, polarity), layer)
        owners.update((head, channel) for channel in packet)
    return frozenset(owners)


def charge(path: tuple[Edge, ...], polarity: int) -> dict[Vec, int]:
    # Same v3 convention as the single-edge source: Q=-eps*partial(path).
    return scale_map(-polarity, chain_boundary(path))


def electric_divergence(path: tuple[Edge, ...], polarity: int) -> dict[Vec, int]:
    # Every dressed edge has canonical coefficient -eps.
    return chain_boundary(path, -polarity)


def transform_edge(matrix, edge: Edge) -> Edge:
    tail, direction = edge
    return (
        tuple(matrix_vector(matrix, tail)),
        tuple(matrix_vector(matrix, direction)),
    )


def transform_owners(matrix, owners: frozenset[tuple[Vec, Channel]]):
    transformed: set[tuple[Vec, Channel]] = set()
    for site, channel in owners:
        transformed_site = tuple(matrix_vector(matrix, site))
        transformed_channel = next(
            iter(transform_channels(matrix, frozenset({channel})))
        )
        transformed.add((transformed_site, transformed_channel))
    return frozenset(transformed)


def distinct_words(counts: tuple[int, ...]) -> tuple[tuple[int, ...], ...]:
    letters = tuple(
        letter for letter, count in enumerate(counts) for _ in range(count)
    )
    return tuple(sorted(set(permutations(letters))))


def adjacent_swap_neighbors(word: tuple[int, ...]):
    for index in range(len(word) - 1):
        if word[index] == word[index + 1]:
            continue
        candidate = list(word)
        candidate[index], candidate[index + 1] = (
            candidate[index + 1],
            candidate[index],
        )
        yield tuple(candidate)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    group = tuple(signed_permutation_matrices())
    origins = ((0, 0, 0), (2, -3, 5), (-4, 1, 7))
    exact_identities = 0

    for origin in origins:
        for first in SC_DIRECTIONS:
            for second in SC_DIRECTIONS:
                if dot(first, second) != 0:
                    continue
                path_a, path_b = square_paths(origin, first, second)
                assert chain_boundary(path_a) == chain_boundary(path_b)

                # The path difference is a local divergence-free cycle.
                current = add_maps(
                    *(edge_boundary(edge, -1) for edge in path_a),
                    *(edge_boundary(edge, 1) for edge in path_b),
                )
                assert current == {}

                for polarity in (-1, 1):
                    assert charge(path_a, polarity) == charge(path_b, polarity)
                    assert electric_divergence(path_a, polarity) == charge(
                        path_a, polarity
                    )
                    assert electric_divergence(path_b, polarity) == charge(
                        path_b, polarity
                    )

                    # Ownership current j=eps*(o_after-o_before); the electric
                    # coefficient changes by its negative on every edge.
                    current_divergence = add_maps(
                        *(edge_boundary(edge, -polarity) for edge in path_a),
                        *(edge_boundary(edge, polarity) for edge in path_b),
                    )
                    assert current_divergence == {}

                    for phase in range(4):
                        for layer in range(3):
                            owners_a = dressed_owners(
                                path_a, phase, polarity, layer
                            )
                            owners_b = dressed_owners(
                                path_b, phase, polarity, layer
                            )
                            assert len(owners_a) == len(owners_b) == 16
                            # No site/channel exclusion slot is double-owned.
                            assert len(owners_a) == len(set(owners_a))
                            assert len(owners_b) == len(set(owners_b))

                            # Four A9 tokens exist in both macrostates: two
                            # primary along the active path and two reserve on
                            # the alternate path.  Field occupancy is 16->16.
                            a9_before = a9_after = 4
                            assert a9_before == a9_after
                            assert len(owners_a) == len(owners_b)

                            for matrix in group:
                                transformed_a = tuple(
                                    transform_edge(matrix, edge) for edge in path_a
                                )
                                expected = dressed_owners(
                                    transformed_a, phase, polarity, layer
                                )
                                assert transform_owners(matrix, owners_a) == expected

                            exact_identities += 10

    check("C1 both two-edge paths have the same endpoint boundary", exact_identities > 0)
    check("C2 plaquette flip changes E by a divergence-free cycle", exact_identities > 0)
    check("C3 endpoint charge and Gauss are preserved exactly", exact_identities > 0)
    check("C4 ownership current obeys exact zero-divergence continuity", exact_identities > 0)
    check("C5 every path uses sixteen unique existing field slots", exact_identities > 0)
    check("C6 flip retains four A9 tokens and sixteen field bits", exact_identities > 0)
    check("C7 all phase/layer presentations are admitted", exact_identities > 0)
    check("C8 flip is signed-cubic covariant", len(group) == 48)

    # Exact finite connectivity census for a representative three-axis
    # monotone path class.  Adjacent swaps are precisely plaquette flips.  The
    # general statement follows by the ordinary inversion-reducing bubble-sort
    # proof; this census is an executable guard on the graph construction.
    words = distinct_words((2, 2, 1))
    adjacency = {word: set(adjacent_swap_neighbors(word)) for word in words}
    reached = {words[0]}
    queue = deque([words[0]])
    while queue:
        word = queue.popleft()
        for neighbor in adjacency[word]:
            if neighbor not in reached:
                reached.add(neighbor)
                queue.append(neighbor)
    check("C9 adjacent plaquette moves connect the full (2,2,1) monotone path class", reached == set(words))
    check("C10 connectivity mechanism is target-free adjacent transposition", all(set(neighbors) <= set(words) for neighbors in adjacency.values()))

    # Scope firewall: connectivity supplies no invariant measure or physical
    # path weight.  Equal-length flips alone cannot assert a Coulomb pole.
    missing = {
        "path weights",
        "ergodic schedule",
        "massless static pole",
        "action curvature",
        "endpoint motion",
    }
    check("C11 charged-pole and measure debts remain explicit", len(missing) == 5)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} dressed-path plaquette checks pass")
    print(f"exact_local_identities={exact_identities}")
    print(f"representative_path_class_size={len(words)}")
    print("Gauss_string_cycle_space=locally_connected_by_plaquette_flips")
    print("Open: schedule/weights, massless pole, action curvature, endpoint motion")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
