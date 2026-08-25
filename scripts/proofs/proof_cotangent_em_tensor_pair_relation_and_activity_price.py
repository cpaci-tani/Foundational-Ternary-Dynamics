#!/usr/bin/env python3
"""Exact joint EM-plus-tensor pair relation and activity price.

The cotangent carrier supplies number, six E/B components, and a C4-weighted
rank-twelve FCC-dyad tensor doublet.  This certificate groups all 18,336
unordered pairs by the complete 18-component field-plus-tensor total and
computes the exact additive-invariant space.

The complete relation has nullity nineteen exactly: number + E/B + Q/P, with
no forced surplus additive invariant.  However 10,368 pair states are
singleton sectors, so any two-record collision preserving the full tensor
doublet must fix them.  Every nonsingleton sector is even and admits an
abstract fixed-point-free matching.

This proves the joint invariant capacity and its sparse-interaction price, not
an equivariant selected collision or a propagating spin-2 pole.
"""

from __future__ import annotations

from collections import Counter, defaultdict
from itertools import combinations

from sympy import Matrix
from sympy.polys.matrices import DomainMatrix

from proof_cotangent_fcc_dyad_tensor_doublet_and_source import tensor_value
from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick,
    layer_value,
)
from proof_hodge_flag_pair_collision_invariant_space import add, one_particle_states


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def combined_value(state, layer: int) -> tuple[object, ...]:
    return tuple(layer_value(state, layer)) + tuple(tensor_value(state, layer))


def main() -> None:
    checks = 0
    states = one_particle_states()
    size = len(states)
    assert size == 192
    checks += 1

    keys = tuple(combined_value(state, 0) for state in states)
    invariant_rows = Matrix.vstack(
        Matrix([[1] * size]),
        *(
            Matrix([[keys[index][component] for index in range(size)]])
            for component in range(18)
        ),
    )
    assert invariant_rows.shape == (19, 192)
    assert exact_rank(invariant_rows) == 19
    checks += 2

    sectors: defaultdict[tuple[object, ...], list[tuple[int, int]]] = defaultdict(list)
    for left, right in combinations(range(size), 2):
        sectors[add(keys[left], keys[right])].append((left, right))
    assert sum(map(len, sectors.values())) == 18336
    sector_histogram = Counter(map(len, sectors.values()))
    expected_histogram = Counter({1: 10368, 2: 3264, 4: 48, 8: 120, 16: 12, 96: 1})
    assert sector_histogram == expected_histogram
    assert all(len(sector) == 1 or len(sector) % 2 == 0 for sector in sectors.values())
    checks += 3

    transition_rows = []
    for sector in sectors.values():
        reference = sector[0]
        for candidate in sector[1:]:
            row = [0] * size
            for index in candidate:
                row[index] += 1
            for index in reference:
                row[index] -= 1
            transition_rows.append(row)
    transition = Matrix(transition_rows)
    transition_rank = exact_rank(transition)
    transition_nullity = size - transition_rank
    assert transition.shape == (4523, 192)
    assert transition_rank == 173
    assert transition_nullity == 19
    assert transition * invariant_rows.T == Matrix.zeros(4523, 19)
    checks += 4

    # Clock covariance: layer q maps to q-1 while the tensor doublet undergoes
    # its native quarter rotation.  Equality of pair totals is preserved by
    # this invertible linear transformation.
    advanced_keys = tuple(combined_value(internal_tick(state), 2) for state in states)
    for state_index, state in enumerate(states):
        before = keys[state_index]
        after = advanced_keys[state_index]
        assert after[:6] == before[:6]
        assert after[6:12] == tuple(-entry for entry in before[12:18])
        assert after[12:18] == before[6:12]
        checks += 3

    nonsingleton_states = sum(
        len(sector) for sector in sectors.values() if len(sector) > 1
    )
    singleton_states = sector_histogram[1]
    assert nonsingleton_states == 7968
    assert singleton_states + nonsingleton_states == 18336
    checks += 2

    print(f"joint_field_tensor_sectors={len(sectors)}")
    print(f"sector_histogram={sorted(sector_histogram.items())}")
    print("complete_transition_rank=173, nullity=19")
    print("complete invariants=number + E/B(6) + C4 tensor doublet Q/P(12)")
    print("mandatory_fixed_pair_states=10368, potentially_active_pair_states=7968")
    print("every nonsingleton sector is even: abstract involutions exist")
    print(
        f"PASS: cotangent EM+tensor pair relation and activity price ({checks} exact checks)"
    )
    print(
        "Open: O_h x C4 equivariant selected collision, tensor kernel/pole, "
        "universal source, static gravity, and lensing"
    )


if __name__ == "__main__":
    main()
