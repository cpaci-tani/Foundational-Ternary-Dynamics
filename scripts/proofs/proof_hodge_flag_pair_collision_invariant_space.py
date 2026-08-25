#!/usr/bin/env python3
"""Exact additive-invariant space of parity-twisted Hodge flag pairs.

One-particle states are the 48 shared-edge flags times four C4 phases.  Even
phases carry a polar edge value E=u_p d; odd phases carry an axial face value
B=v_p n.  Grouping every unordered two-record state by total (E,B) yields a
complete collision relation whose additive invariant space is exactly record
number plus E and B: seven dimensions.

This proves the mode-reduction capacity of the state space, not a selected
equivariant deterministic collision or a finite Maxwell action.
"""

from __future__ import annotations

from collections import Counter, defaultdict
from itertools import combinations

from sympy import Matrix
from sympy.polys.matrices import DomainMatrix

from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_shared_edge_hodge_flag_bcc_propagation import (
    Flag,
    flags,
    transform_flag,
)


PHASES = tuple(range(4))
PHASE_COORDINATES = ((1, 0), (0, 1), (-1, 0), (0, -1))
OneParticleState = tuple[Flag, int]


def one_particle_states() -> tuple[OneParticleState, ...]:
    return tuple((flag, phase) for flag in flags() for phase in PHASES)


def field_value(state: OneParticleState) -> tuple[int, ...]:
    (tangent, normal, _handedness), phase = state
    u, v = PHASE_COORDINATES[phase]
    electric = tuple(u * component for component in tangent)
    magnetic = tuple(v * component for component in normal)
    return electric + magnetic


def transform_state(matrix, state: OneParticleState) -> OneParticleState:
    flag, phase = state
    return transform_flag(matrix, flag), phase


def add(left, right):
    return tuple(a + b for a, b in zip(left, right))


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())
    states = one_particle_states()
    state_index = {state: index for index, state in enumerate(states)}
    assert len(states) == 192
    assert len(state_index) == 192
    checks += 2

    # E is polar and B is axial under the complete signed cubic group.
    for state in states:
        electric = field_value(state)[:3]
        magnetic = field_value(state)[3:]
        for matrix in group:
            transformed = field_value(transform_state(matrix, state))
            determinant = round(Matrix(matrix).det())
            expected_electric = tuple(Matrix(matrix) * Matrix(electric))
            expected_magnetic = tuple(
                determinant * entry
                for entry in (Matrix(matrix) * Matrix(magnetic))
            )
            assert transformed == expected_electric + expected_magnetic
            checks += 1

    pair_sectors: defaultdict[
        tuple[int, ...], list[tuple[OneParticleState, OneParticleState]]
    ] = defaultdict(list)
    for left, right in combinations(states, 2):
        pair_sectors[add(field_value(left), field_value(right))].append(
            (left, right)
        )

    assert sum(len(sector) for sector in pair_sectors.values()) == 18336
    assert len(pair_sectors) == 73
    sector_histogram = Counter(len(sector) for sector in pair_sectors.values())
    assert sector_histogram == Counter({256: 60, 120: 12, 1536: 1})
    assert pair_sectors[(0,) * 6]
    assert all(len(sector) % 2 == 0 for sector in pair_sectors.values())
    checks += 5

    transition_rows = []
    for sector in pair_sectors.values():
        reference = sector[0]
        for candidate in sector[1:]:
            row = [0] * len(states)
            for state in candidate:
                row[state_index[state]] += 1
            for state in reference:
                row[state_index[state]] -= 1
            transition_rows.append(row)

    transition = Matrix(transition_rows)
    transition_rank = DomainMatrix.from_Matrix(transition).rank()
    assert transition.shape == (18263, 192)
    assert transition_rank == 185
    assert len(states) - transition_rank == 7
    checks += 3

    invariant_rows = [Matrix([[1 for _state in states]])]
    for component in range(6):
        invariant_rows.append(
            Matrix([[field_value(state)[component] for state in states]])
        )
    invariant_stack = Matrix.vstack(*invariant_rows)
    assert DomainMatrix.from_Matrix(invariant_stack).rank() == 7
    for row in invariant_rows:
        assert transition * row.T == Matrix.zeros(len(transition_rows), 1)
        checks += 1

    # The seven rows span the complete additive left nullspace by the exact
    # rank count.  Handedness, individual phase counts, flag-cycle identity,
    # and BCC ray label are therefore not unavoidable additive invariants of
    # the complete (E,B)-preserving pair relation.
    print(f"PASS: Hodge-flag pair collision invariant space ({checks} exact checks)")
    print("one_particle_states=192, unordered_pairs=18336, field_sectors=73")
    print(f"sector_histogram={sorted(sector_histogram.items())}")
    print("transition_rank=185, nullity=7")
    print("complete additive invariants: record number + polar E(3) + axial B(3)")
    print("all field sectors are even, so abstract involutive matchings exist")
    print("Open: cubic/C4-equivariant deterministic matching and transport compatibility")


if __name__ == "__main__":
    main()
