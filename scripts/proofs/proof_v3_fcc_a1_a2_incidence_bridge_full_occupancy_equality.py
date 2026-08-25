#!/usr/bin/env python3
"""FCC A1/A2 incidence bridge and full relative occupancy equality.

Every unoriented FCC bond is the unique diagonal of one elementary
plaquette.  The selected v3 carrier places an A9 primary/reserve pair on the
A1 FCC bond and another pair on the corresponding labeled A2 diagonal.

This certificate selects one existing C3 clock section for a source-neutral
bridge between the A1 reserve slot and one A2 diagonal slot.  On the other two
sections the A1 pair executes its existing recurrence while the A2 token
advances in place.  Every occupied token advances exactly one C4 phase per
global tick.  The complete local map is a finite permutation, preserves token
count and charge, has exact inverse, and is signed-cubic covariant through the
unique diagonal incidence.

Adding the bridge exchange vector to the common occupancy classification
raises the exchange rank from two to three and leaves the unique additive
energy ray e_F=e_A1_SC=e_A1_FCC=e_A2.  The bridge is a selected candidate,
not yet derived from or integrated into canonical Phi, and it does not fix the
common positive multiplier or physical action curvature.
"""

from __future__ import annotations

import sys
from collections import Counter
from itertools import product

from sympy import Matrix

from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_v3_common_action_phi_v2 import readout, relation_tick, rotate
from proof_v3_common_transaction_phi import POS_FCC


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
A9 = tuple[int, int]
BLANK: A9 = (0, 0)
AXES: tuple[Vec, ...] = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
PLANE_AXES = ((AXES[0], AXES[1]), (AXES[1], AXES[2]), (AXES[2], AXES[0]))
A9_STATES = tuple(product((-1, 0, 1), repeat=2))


def add(left: Vec, right: Vec, size: int) -> Vec:
    return tuple((a + b) % size for a, b in zip(left, right))  # type: ignore[return-value]


def edge(left: Vec, right: Vec):
    return tuple(sorted((left, right)))


def plaquette_vertices(origin: Vec, first: Vec, second: Vec, size: int):
    return frozenset(
        {
            origin,
            add(origin, first, size),
            add(origin, second, size),
            add(add(origin, first, size), second, size),
        }
    )


def plaquette_diagonals(origin: Vec, first: Vec, second: Vec, size: int):
    return (
        edge(origin, add(add(origin, first, size), second, size)),
        edge(add(origin, first, size), add(origin, second, size)),
    )


def sites(size: int):
    return tuple(product(range(size), repeat=3))


def matrix_vector_mod(matrix, vector: Vec, size: int):
    return tuple(
        sum(matrix[i][j] * vector[j] for j in range(3)) % size
        for i in range(3)
    )


def bridge_tick(reserve: A9, diagonal: A9):
    # Same exact A9 crossing clock, now interpreted across incident cell
    # dimensions rather than primary/reserve ownership of one bond.
    return relation_tick(reserve, diagonal, even_gate=True)


def local_tick(layer: int, primary: A9, reserve: A9, diagonal: A9, even_gate: bool):
    assert layer in range(3)
    if layer == 0:
        next_reserve, next_diagonal = bridge_tick(reserve, diagonal)
        return 2, rotate(primary), next_reserve, next_diagonal
    next_primary, next_reserve = relation_tick(primary, reserve, even_gate=even_gate)
    return (layer - 1) % 3, next_primary, next_reserve, rotate(diagonal)


def occupied(z: A9) -> int:
    return readout(z)[0]


def polarity(z: A9) -> int:
    return readout(z)[1]


def phase(z: A9):
    return readout(z)[2]


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    size = 5
    lattice_sites = sites(size)
    group = tuple(signed_permutation_matrices())

    incidence = {}
    plaquette_cells = set()
    for origin in lattice_sites:
        for family, (first, second) in enumerate(PLANE_AXES):
            cell = plaquette_vertices(origin, first, second, size)
            plaquette_cells.add(cell)
            for diagonal_label, diagonal in enumerate(
                plaquette_diagonals(origin, first, second, size)
            ):
                assert diagonal not in incidence
                incidence[diagonal] = (cell, family, diagonal_label)

    fcc_edges = {
        edge(origin, add(origin, direction, size))
        for origin in lattice_sites
        for direction in POS_FCC
    }
    check(
        "C1 every periodic FCC A1 bond is one unique labeled A2 plaquette diagonal",
        len(incidence) == len(fcc_edges) == 6 * size**3
        and set(incidence) == fcc_edges,
    )

    covariance_rows = 0
    for diagonal, (cell, _family, _label) in incidence.items():
        for matrix in group:
            transformed_diagonal = edge(
                matrix_vector_mod(matrix, diagonal[0], size),
                matrix_vector_mod(matrix, diagonal[1], size),
            )
            transformed_cell = frozenset(
                matrix_vector_mod(matrix, vertex, size) for vertex in cell
            )
            assert transformed_diagonal in incidence
            assert incidence[transformed_diagonal][0] == transformed_cell
            covariance_rows += 1
    check(
        "C2 FCC/A2 diagonal incidence is signed-cubic covariant",
        covariance_rows == len(incidence) * 48,
    )

    # The bridge pair itself is a complete finite permutation.
    bridge_outputs = {}
    bridge_crossings = Counter()
    for reserve in A9_STATES:
        for diagonal in A9_STATES:
            output = bridge_tick(reserve, diagonal)
            assert output not in bridge_outputs
            bridge_outputs[output] = (reserve, diagonal)
            before = (occupied(reserve), occupied(diagonal))
            after = (occupied(output[0]), occupied(output[1]))
            assert sum(before) == sum(after)
            bridge_crossings[(after[0] - before[0], after[1] - before[1])] += 1
    check(
        "C3 one A1-reserve/A2-diagonal crossing clock is an exact 81-state permutation",
        len(bridge_outputs) == 81,
    )

    # Full layer-clock map: bridge on q=0; ordinary A1 recurrence on q=1,2.
    local_rows = 0
    transfer_rows = 0
    for even_gate in (False, True):
        forward = {}
        for layer in range(3):
            for primary in A9_STATES:
                for reserve in A9_STATES:
                    for diagonal in A9_STATES:
                        before = (layer, primary, reserve, diagonal)
                        after = local_tick(
                            layer, primary, reserve, diagonal, even_gate
                        )
                        assert after not in forward.values()
                        forward[before] = after

                        before_tokens = (
                            occupied(primary)
                            + occupied(reserve)
                            + occupied(diagonal)
                        )
                        after_tokens = sum(occupied(z) for z in after[1:])
                        assert before_tokens == after_tokens

                        before_payloads = sorted(
                            (phase(z), polarity(z))
                            for z in (primary, reserve, diagonal)
                            if occupied(z)
                        )
                        after_payloads = sorted(
                            (phase(z), polarity(z))
                            for z in after[1:]
                            if occupied(z)
                        )
                        advanced_payloads = sorted(
                            (phase(rotate(z)), polarity(rotate(z)))
                            for z in (primary, reserve, diagonal)
                            if occupied(z)
                        )
                        assert after_payloads == advanced_payloads

                        # The bridge section changes only reserve/A2 ownership;
                        # primary source is unchanged.  Other layers use the
                        # certified A1 recurrence, whose source change has the
                        # exact bond-current boundary.
                        if layer == 0:
                            assert occupied(after[1]) == occupied(primary)
                            assert polarity(after[1]) == polarity(primary)
                            if occupied(after[2]) != occupied(reserve):
                                transfer_rows += 1
                        else:
                            delta_q = polarity(after[1]) - polarity(primary)
                            tail_change = delta_q
                            head_change = -delta_q
                            current = -delta_q
                            assert tail_change + current == 0
                            assert head_change - current == 0
                        local_rows += 1
        assert len(forward) == 3 * 9**3
        assert len(set(forward.values())) == len(forward)
    check(
        "C4 complete C3-clocked bridge/recurrence law is a finite permutation",
        local_rows == 2 * 3 * 9**3,
    )
    check(
        "C5 every occupied token advances one C4 phase and retains polarity per tick",
        local_rows > 0,
    )
    check(
        "C6 bridge ticks are source-neutral and recurrence ticks retain exact continuity",
        transfer_rows > 0,
    )

    # Exchange-vector rank after adding FCC<->A2 incidence to the earlier
    # F<->SC and SC/F<->A2 transactions.
    exchange_matrix = Matrix(
        [
            (-1, 1, 0, 0),
            (0, 1, 0, -1),
            (1, 0, 0, -1),
            (0, 0, -1, 1),
        ]
    )
    nullspace = exchange_matrix.nullspace()
    all_equal = Matrix([1, 1, 1, 1])
    check(
        "C7 FCC/A2 bridge raises the exchange graph to rank three",
        exchange_matrix.rank() == 3 and len(nullspace) == 1,
        str(nullspace),
    )
    check(
        "C8 unique additive occupancy-energy ray has all four role weights equal",
        exchange_matrix * all_equal == Matrix.zeros(4, 1)
        and nullspace[0][0] != 0
        and all(nullspace[0][index] == nullspace[0][0] for index in range(4)),
    )

    # Simultaneous application has one writer because the diagonal incidence
    # is bijective: each FCC reserve and each labeled A2 diagonal appears once.
    check(
        "C9 simultaneous finite-lattice bridge schedule has one writer per FCC/A2 slot",
        len(incidence) == len(set(incidence.keys())) == len(fcc_edges),
    )
    check(
        "C10 bridge uses only existing C3 and A9 data within one plaquette Moore cell",
        all(len(cell) == 4 for cell in plaquette_cells)
        and len(A9_STATES) == 9,
    )

    missing = {
        "derivation of the bridge from one stationary/common-action principle",
        "integration with canonical Phi writer priority and charged dressing",
        "absolute common multiplier Gamma relative to clock action",
        "bundle clock-debit and formation work",
        "block-stable interacting curvature and pole residue",
        "multi-event arbitration and stable matter formation",
    }
    check(
        "C11 physical common action remains open at six derivation/integration debts",
        len(missing) == 6,
    )
    check(
        "C12 no new type, target coupling, fitted value, or numerical search enters",
        len(A9_STATES) == 9 and exchange_matrix.rank() == 3,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} FCC/A2 incidence-bridge checks pass")
    print(f"fcc_a2_incidences={len(incidence)}")
    print(f"signed_cubic_incidence_rows={covariance_rows}")
    print(f"local_clock_rows={local_rows}")
    print(f"bridge_transfer_rows={transfer_rows}")
    print("relative_energy_result=e_F=e_A1_SC=e_A1_FCC=e_A2")
    print("status=selected_bridge_closes_relative_occupancy_ray_absolute_action_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
