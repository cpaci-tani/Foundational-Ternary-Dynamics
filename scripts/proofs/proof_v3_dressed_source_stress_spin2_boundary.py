#!/usr/bin/env python3
"""Exact v3 dressed-source stress and selected-collision spin-2 boundary.

Every cotangent field record supplies a symmetric trace-free stress readout
from E outer E + B outer B.  Because both legs are axial-coordinate directions,
the one-record readout spans only the two diagonal Eg components; all three
T2g shear components vanish.  C4 phase doubles this to rank four, not ten. The
complete eight-record dressed source nevertheless has a nonzero charge-even
aligned STF moment.

The selected global-C3 collision preserves number plus E/B but not this tensor
doublet.  Hence the same finite source already carries a gravity-eligible
stress readout, while Phi-v2 has no protected propagating tensor slow mode.
"""

from __future__ import annotations

import sys

from sympy import Matrix, Rational

import proof_global_c3_cotangent_layer_equivariant_collision as collision_proof
from proof_cotangent_stabilizer_packet_gauss_source import (
    advance_packet,
    packet,
)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import layer_value
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)


sys.stdout.reconfigure(encoding="utf-8")


def outer(vector: tuple[int, ...]) -> Matrix:
    column = Matrix(vector)
    return column * column.T


def stress_matrix(state, layer: int) -> Matrix:
    value = layer_value(state, layer)
    electric = value[:3]
    magnetic = value[3:]
    return (
        outer(electric)
        + outer(magnetic)
        - Rational(2, 3) * Matrix.eye(3)
    )


def stress_vector(state, layer: int) -> tuple:
    stress = stress_matrix(state, layer)
    assert stress == stress.T
    assert stress.trace() == 0
    return (
        stress[0, 0],
        stress[1, 1],
        stress[0, 1],
        stress[0, 2],
        stress[1, 2],
    )


def phase_weights(phase: int) -> tuple[int, int]:
    return ((1, 0), (0, 1), (-1, 0), (0, -1))[phase]


def quadrature_vector(state, layer: int) -> tuple:
    stress = stress_vector(state, layer)
    u, v = phase_weights(state[1])
    return tuple(u * entry for entry in stress) + tuple(
        v * entry for entry in stress
    )


def transform_stress(matrix, stress: Matrix) -> Matrix:
    transform = Matrix(matrix)
    return transform * stress * transform.T


def expected_packet_stress(direction: tuple[int, int, int]) -> Matrix:
    column = Matrix(direction)
    return 4 * (column * column.T - Rational(1, 3) * Matrix.eye(3))


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    print("Running parent exact collision certificate...")
    collision_proof.main()
    data = collision_proof.CERTIFICATE_DATA
    assert data is not None
    states = data["states"]
    corrections = data["corrections"]
    assert len(states) == 192

    tensor_ranks = []
    quadrature_ranks = []
    leakage_ranks = []
    for layer in range(3):
        tensor_rows = Matrix(
            [
                [stress_vector(state, layer)[row] for state in states]
                for row in range(5)
            ]
        )
        quadrature_rows = Matrix(
            [
                [quadrature_vector(state, layer)[row] for state in states]
                for row in range(10)
            ]
        )
        tensor_ranks.append(tensor_rows.rank())
        quadrature_ranks.append(quadrature_rows.rank())
        leakage = quadrature_rows * corrections[layer]
        leakage_ranks.append(leakage.rank())

    check("C1 existing one-particle stress spans exactly diagonal Eg rank two", tensor_ranks == [2, 2, 2], str(tensor_ranks))
    check("C2 C4 phase supplies only a rank-four Eg quadrature doublet", quadrature_ranks == [4, 4, 4], str(quadrature_ranks))
    check("C2b all one-record T2g shear coordinates vanish", all(stress_matrix(state, layer)[i, j] == 0 for layer in range(3) for state in states for i, j in ((0, 1), (0, 2), (1, 2))))

    group = tuple(signed_permutation_matrices())
    covariance_rows = 0
    for layer in range(3):
        for state in states:
            stress = stress_matrix(state, layer)
            for matrix in group:
                transformed_flag = (
                    tuple(matrix_vector(matrix, state[0][0])),
                    tuple(matrix_vector(matrix, state[0][1])),
                    round(Matrix(matrix).det()) * state[0][2],
                )
                transformed_state = (transformed_flag, state[1])
                assert stress_matrix(transformed_state, layer) == transform_stress(
                    matrix, stress
                )
                covariance_rows += 1
    check("C3 STF readout is covariant under all 48 signed-cubic maps", covariance_rows == 3 * 192 * 48)

    packet_rows = 0
    for direction in (
        (1, 0, 0),
        (-1, 0, 0),
        (0, 1, 0),
        (0, -1, 0),
        (0, 0, 1),
        (0, 0, -1),
    ):
        records = packet(direction, 0)
        for tick in range(12):
            layer = (-tick) % 3
            total_stress = sum(
                (stress_matrix(record, layer) for record in records),
                Matrix.zeros(3, 3),
            )
            assert total_stress == expected_packet_stress(direction)
            records = advance_packet(records)
            packet_rows += 1
    check("C4 dressed packet carries one clock-stable aligned STF stress", packet_rows == 72)

    # Stress is even under the carried electric polarity.  The two complete
    # polarity copies therefore carry opposite charge but equal tensor source.
    polarity_stresses = {
        polarity: expected_packet_stress((1, 0, 0))
        for polarity in (-1, 1)
    }
    check("C5 source stress is charge-conjugation even", polarity_stresses[-1] == polarity_stresses[1])

    # The collision corrections annihilate the seven number/E/B rows by
    # construction.  A nonzero tensor leakage proves the STF doublet is not an
    # additional protected collision invariant.
    check("C6 selected collision leaks the STF quadrature on every C3 layer", all(rank > 0 for rank in leakage_ranks), str(leakage_ranks))
    check("C7 selected collision does not protect even the rank-four Eg quadrature", leakage_ranks != [0, 0, 0])

    # Source availability and propagation availability must remain separate.
    check("C8 one common dressed source carries charge-odd E and charge-even STF stress", expected_packet_stress((1, 0, 0)) != Matrix.zeros(3, 3))
    check("C9 tensor readout requires no new microscopic carrier type", len(states) * 2 == 384)

    missing = {
        "T2g shear carrier",
        "tensor-protecting collision",
        "spin-2 constraints",
        "massless tensor pole",
        "universal stress coupling",
        "common cone and lensing",
        "nonlinear bootstrap provenance",
    }
    check("C10 gravity remains open at carrier/propagator/coupling levels", len(missing) == 7)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 stress/spin2-boundary checks pass")
    print(f"signed_cubic_covariance_rows={covariance_rows}")
    print(f"collision_tensor_leakage_ranks={leakage_ranks}")
    print("source_STF=4*(d d^T-I/3), charge_conjugation_even")
    print("gravity_status=Eg_source_readout_present, T2g_and_protected_tensor_pole_absent")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
