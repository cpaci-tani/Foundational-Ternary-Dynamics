#!/usr/bin/env python3
"""Exact clock-compatible Hodge-triad field readout.

The parity-twisted fixed-frame readout E=Re(i^p)d, B=Im(i^p)n has the right
O_h types but is not invariant under the shared-edge C3 flag update plus C4
phase advance.  Its twelve clock images span 36 field rows, so a commuting
collision would have to protect 30 surplus modes.

This certificate constructs the minimal invariant alternative from the full
oriented triad carried by one flag.  The polar triad sum

    e = d + h n + d x n

is fixed by the C3 update, and b=h e is its axial partner.  Record number plus
the three components of e and b form exactly seven independent rows, remain
fixed under the entire C3xC4 tick, and are the complete additive invariants of
the corresponding unrestricted two-record field relation.

This is a carrier/readout theorem, not yet a selected collision or Maxwell
pole.  No physical coefficient or measured target is used.
"""

from __future__ import annotations

from collections import Counter, defaultdict
from itertools import combinations

from sympy import Matrix
from sympy.polys.matrices import DomainMatrix

from proof_hodge_flag_pair_collision_invariant_space import (
    add,
    field_value as fixed_frame_value,
    one_particle_states,
)
from proof_moore_bond_capacity_type_census import (
    determinant_3,
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import cross
from proof_shared_edge_hodge_flag_bcc_propagation import (
    scale,
    transform_flag,
    update_flag,
)


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def triad_readout(state) -> tuple[int, ...]:
    (tangent, normal, handedness), _phase = state
    third = cross(tangent, normal)
    electric = add(add(tangent, scale(handedness, normal)), third)
    magnetic = scale(handedness, electric)
    return electric + magnetic


def internal_tick(state):
    flag, phase = state
    return update_flag(flag), (phase + 1) % 4


def main() -> None:
    checks = 0
    states = one_particle_states()
    state_index = {state: index for index, state in enumerate(states)}
    group = tuple(signed_permutation_matrices())
    assert len(states) == 192
    assert len(group) == 48
    checks += 2

    # The three polar triad legs cycle under the flag update.  The axial
    # partner is h times the same body-diagonal label.
    readout_types = Counter(triad_readout(state) for state in states)
    assert len(readout_types) == 16
    assert set(readout_types.values()) == {12}
    for value in readout_types:
        electric = value[:3]
        magnetic = value[3:]
        assert set(map(abs, electric)) == {1}
        handedness = 1 if magnetic == electric else -1
        assert magnetic == scale(handedness, electric)
        checks += 2

    for state in states:
        value = triad_readout(state)
        assert triad_readout(internal_tick(state)) == value
        for matrix in group:
            transformed_state = (transform_flag(matrix, state[0]), state[1])
            transformed_value = triad_readout(transformed_state)
            determinant = determinant_3(matrix)
            expected_electric = tuple(matrix_vector(matrix, value[:3]))
            expected_magnetic = tuple(
                determinant * entry
                for entry in matrix_vector(matrix, value[3:])
            )
            assert transformed_value == expected_electric + expected_magnetic
            checks += 1
        checks += 1

    internal_image = tuple(state_index[internal_tick(state)] for state in states)
    internal = Matrix.zeros(192, 192)
    for source, target in enumerate(internal_image):
        internal[target, source] = 1
    assert internal**12 == Matrix.eye(192)
    checks += 1

    triad_rows = Matrix.vstack(
        Matrix([[1] * 192]),
        *(
            Matrix([[triad_readout(state)[component] for state in states]])
            for component in range(6)
        ),
    )
    fixed_rows = Matrix.vstack(
        Matrix([[1] * 192]),
        *(
            Matrix([[fixed_frame_value(state)[component] for state in states]])
            for component in range(6)
        ),
    )
    assert exact_rank(triad_rows) == 7
    assert triad_rows * internal == triad_rows
    checks += 2

    # The fixed-frame field requires a 37-dimensional protected clock orbit;
    # the triad readout closes on its original seven rows.
    fixed_orbit = []
    triad_orbit = []
    power = Matrix.eye(192)
    for _tick in range(12):
        fixed_orbit.append(fixed_rows * power)
        triad_orbit.append(triad_rows * power)
        power = internal * power
    fixed_orbit_rank = exact_rank(Matrix.vstack(*fixed_orbit))
    triad_orbit_rank = exact_rank(Matrix.vstack(*triad_orbit))
    assert fixed_orbit_rank == 37
    assert triad_orbit_rank == 7
    checks += 2

    # In the linear flag ansatz there are three cyclic polar legs and four
    # phase indicators.  Their combined (r,p)->(r+1,p+1) action is a single
    # twelve-cycle, hence its fixed coefficient space is exactly one
    # dimensional for each O_h vector copy.  The same statement holds for the
    # axial triad.  This is the minimality claim within the registered ansatz.
    coefficient_cycle = Matrix.zeros(12, 12)
    labels = tuple((leg, phase) for leg in range(3) for phase in range(4))
    label_index = {label: index for index, label in enumerate(labels)}
    for source, (leg, phase) in enumerate(labels):
        target = label_index[((leg + 1) % 3, (phase + 1) % 4)]
        coefficient_cycle[target, source] = 1
    assert coefficient_cycle**12 == Matrix.eye(12)
    assert exact_rank(coefficient_cycle - Matrix.eye(12)) == 11
    checks += 2

    # Complete unrestricted two-record relation for the new field totals.
    sectors: defaultdict[tuple[int, ...], list[tuple[int, int]]] = defaultdict(list)
    for left, right in combinations(range(192), 2):
        sectors[add(triad_readout(states[left]), triad_readout(states[right]))].append(
            (left, right)
        )
    assert sum(map(len, sectors.values())) == 18336
    assert all(len(sector) >= 2 for sector in sectors.values())
    sector_histogram = Counter(len(sector) for sector in sectors.values())
    checks += 2

    transition_rows = []
    for sector in sectors.values():
        reference = sector[0]
        for candidate in sector[1:]:
            row = [0] * 192
            for index in candidate:
                row[index] += 1
            for index in reference:
                row[index] -= 1
            transition_rows.append(row)
    transition = Matrix(transition_rows)
    transition_rank = exact_rank(transition)
    transition_nullity = 192 - transition_rank
    assert transition * triad_rows.T == Matrix.zeros(len(transition_rows), 7)
    checks += 1

    print("one_particle_states=192, clock_invariant_field_types=16")
    print(f"field_sector_count={len(sectors)}")
    print(f"field_sector_histogram={sorted(sector_histogram.items())}")
    print(
        f"complete_relation_rank={transition_rank}, "
        f"nullity={transition_nullity}"
    )
    print(f"fixed_frame_clock_orbit_rank={fixed_orbit_rank}")
    print(f"triad_clock_orbit_rank={triad_orbit_rank}")
    print(
        f"PASS: clock-compatible Hodge-triad readout ({checks} exact checks)"
    )
    print(
        "Open: select one O_h x C12-equivariant seven-invariant collision and "
        "test its finite-k edge-face generator"
    )


if __name__ == "__main__":
    main()
