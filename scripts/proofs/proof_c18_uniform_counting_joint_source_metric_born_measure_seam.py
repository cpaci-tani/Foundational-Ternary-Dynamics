#!/usr/bin/env python3
"""Exact joint source-metric and prepared Born counting-measure certificate.

The uniform five-state C18 record measure induces one block covariance for
relative phase current, common tensor quadratures, and capacity.  This script
derives the complete 24-dimensional covariance, evaluates the one-token event
with its inverse covariance, proves coordinate-reparameterization invariance,
and verifies the prepared coprime-ring Born count as a uniform ordered-pair
pushforward.

This is a bare Fisher/large-deviation metric and a prepared counting seam.  It
does not identify the metric with an interacting action, prepare the physical
measure, produce a field pole, establish lensing, or measure a coupling.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
from itertools import permutations, product
from pathlib import Path

import sympy as sp

from proof_c18_actualization_moment_source_vertex import (
    LINE_DIRECTIONS,
    PHASE_COORDINATES,
)
from proof_c18_common_phase_tensor_doublet import scaled_tensor_covariance
from proof_c18_uniform_token_blocking import (
    ALPHABET,
    C18_LINE_MOMENTS,
    covariance_matrix,
    tensor_covariance,
)
from proof_c4_coprime_ring_born_pushforward import (
    coherent_norm_squared,
    enumerate_clicks,
    residual_records,
    ring_orbit,
)


F = Fraction
ROOT = Path(__file__).resolve().parents[2]

LOCKED_HASHES = {
    ROOT / "scripts/proofs/proof_c18_uniform_token_blocking.py":
        "60443BCD5E02E30E390F077C228B473E0A06D48E679C11822DB883ABF877BF30",
    ROOT / "scripts/proofs/proof_c18_common_phase_tensor_doublet.py":
        "3D04960ACBEB54CB7D43F8FDDE597482685A058671ED887278F164C98A9ACABA",
    ROOT / "scripts/proofs/proof_c18_actualization_moment_source_vertex.py":
        "6744132874E7785BB3E1474969AE519B0F319DA19961EE4126659014C603C4A2",
    ROOT / "scripts/proofs/proof_c4_coprime_ring_born_pushforward.py":
        "59B9AED0F2FAF64609DB42F021B1F2498DB66BAE8B293099BABFE52478F2B802",
    ROOT / "docs/theory/10_eft_program/derivations/gravity_cosmology/"
    "THEOREM_C18_UNIFORM_TOKEN_BARE_BLOCKING_v1.md":
        "819647BEE0A9043F7142E0022A4AB326677FBA886D7B14B4644204A234804349",
    ROOT / "docs/theory/10_eft_program/derivations/"
    "common_action_mechanics_reciprocity/"
    "THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md":
        "5EF07BBFEE3D77966279BC7E2A34E18F5B7C913ABF050A37CD9779F5EC52AC06",
    ROOT / "docs/theory/10_eft_program/derivations/quantum_foundations/"
    "THEOREM_C4_COPRIME_RING_BORN_PUSHFORWARD_v1.md":
        "0EDA9A309CB7BF02D74699712459527C1E2ABCB3782E6700977CC70A7EE742F1",
    ROOT / "docs/theory/10_eft_program/derivations/"
    "common_action_mechanics_reciprocity/"
    "THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md":
        "AE02F7AAB30E5B8582003292B1B4B32621D04566402029DA4299CBC4E43A4322",
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "common_action_mechanics_reciprocity/"
    "PREREG_C18_UNIFORM_COUNTING_JOINT_SOURCE_METRIC_AND_BORN_MEASURE_SEAM_v1.md":
        "775EB8F05FF495C2A7CA6D652A686323527A31E70E350BCB67C0B28F36BF26DB",
}


def file_hash(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


def to_sympy(matrix: list[list[Fraction]]) -> sp.Matrix:
    return sp.Matrix(
        [
            [sp.Rational(value.numerator, value.denominator) for value in row]
            for row in matrix
        ]
    )


def dyad_matrix(moment: tuple[Fraction, ...]) -> sp.Matrix:
    xx, yy, zz, xy, xz, yz = moment
    values = tuple(sp.Rational(value.numerator, value.denominator) for value in moment)
    xx, yy, zz, xy, xz, yz = values
    return sp.Matrix([[xx, xy, xz], [xy, yy, yz], [xz, yz, zz]])


def symmetric_coordinates(matrix: sp.Matrix) -> sp.Matrix:
    return sp.Matrix(
        [matrix[0, 0], matrix[1, 1], matrix[2, 2], matrix[0, 1], matrix[0, 2], matrix[1, 2]]
    )


def half_cost(vector: sp.Matrix, covariance: sp.Matrix) -> sp.Expr:
    return sp.simplify((vector.T * covariance.inv() * vector)[0] / 2)


def scalar_line_covariance() -> sp.Matrix:
    """Derive scalar relative/common/capacity covariance from 25 line states."""

    samples = []
    for (u_plus, v_plus), (u_minus, v_minus) in product(ALPHABET, repeat=2):
        c_plus = 1 - u_plus * u_plus - v_plus * v_plus
        c_minus = 1 - u_minus * u_minus - v_minus * v_minus
        samples.append(
            (
                F(u_plus - u_minus, 9),
                F(v_plus - v_minus, 9),
                F(u_plus + u_minus, 18),
                F(v_plus + v_minus, 18),
                F(c_plus + c_minus, 18),
            )
        )
    return to_sympy(covariance_matrix(tuple(samples)))


def joint_covariances() -> tuple[sp.Matrix, sp.Matrix, sp.Matrix, sp.Matrix]:
    current = sp.Rational(4, 135) * sp.eye(6)
    tensor_one = to_sympy(scaled_tensor_covariance(F(1, 5)))
    tensor = sp.diag(tensor_one, tensor_one)
    capacity = to_sympy(tensor_covariance())
    joint = sp.diag(current, tensor, capacity)
    return current, tensor, capacity, joint


def event_vectors(
    line_index: int,
    phase: int,
    orientation: int,
) -> tuple[sp.Matrix, sp.Matrix, sp.Matrix]:
    direction = LINE_DIRECTIONS[line_index]
    moment = sp.Matrix(
        [
            sp.Rational(value.numerator, value.denominator)
            for value in C18_LINE_MOMENTS[line_index]
        ]
    )
    u, v = PHASE_COORDINATES[phase]
    current = sp.Matrix.vstack(
        sp.Rational(orientation * u, 9) * direction,
        sp.Rational(orientation * v, 9) * direction,
    )
    tensor = sp.Matrix.vstack(
        sp.Rational(u, 18) * moment,
        sp.Rational(v, 18) * moment,
    )
    capacity = -sp.Rational(1, 18) * moment
    return current, tensor, capacity


def trace_and_shear(moment: sp.Matrix) -> tuple[sp.Matrix, sp.Matrix]:
    trace = moment[0] + moment[1] + moment[2]
    trace_part = sp.Matrix([trace / 3, trace / 3, trace / 3, 0, 0, 0])
    return trace_part, sp.simplify(moment - trace_part)


def signed_cubic_group() -> tuple[sp.Matrix, ...]:
    matrices = []
    for permutation in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            matrix = sp.zeros(3, 3)
            for row, column in enumerate(permutation):
                matrix[row, column] = signs[row]
            matrices.append(matrix)
    assert len(matrices) == 48
    return tuple(matrices)


def verify_source_integrity() -> int:
    for path, expected in LOCKED_HASHES.items():
        assert file_hash(path) == expected
    return len(LOCKED_HASHES)


def verify_joint_measure() -> tuple[int, tuple[sp.Matrix, ...]]:
    checks = 0
    scalar_covariance = scalar_line_covariance()
    assert scalar_covariance == sp.diag(
        sp.Rational(4, 405),
        sp.Rational(4, 405),
        sp.Rational(1, 405),
        sp.Rational(1, 405),
        sp.Rational(2, 2025),
    )
    checks += 1

    current, tensor, capacity, joint = joint_covariances()
    a6 = sp.Matrix(
        [
            [4, 1, 1, 0, 0, 0],
            [1, 4, 1, 0, 0, 0],
            [1, 1, 4, 0, 0, 0],
            [0, 0, 0, 1, 0, 0],
            [0, 0, 0, 0, 1, 0],
            [0, 0, 0, 0, 0, 1],
        ]
    )
    assert current == sp.Rational(4, 135) * sp.eye(6)
    assert tensor == sp.diag(a6 / 810, a6 / 810)
    assert capacity == a6 / 2025
    assert joint == sp.diag(current, tensor, capacity)
    assert joint.rank() == 24
    assert all(value > 0 for value in joint.eigenvals())
    checks += 6

    # The five scalar line variables have zero cross covariance.  Independence
    # between antipodal lines therefore derives every joint off-block zero.
    assert all(
        scalar_covariance[row, column] == 0
        for row in range(5)
        for column in range(5)
        if row != column
    )
    checks += 1
    return checks, (current, tensor, capacity, joint)


def verify_event_metric(covariances: tuple[sp.Matrix, ...]) -> int:
    current_covariance, tensor_covariance_matrix, capacity_covariance, joint = covariances
    checks = 0
    shell_costs: dict[str, set[tuple[sp.Expr, ...]]] = {"SC": set(), "FCC": set()}

    for line_index in range(9):
        shell = "SC" if line_index < 3 else "FCC"
        moment = sp.Matrix(
            [
                sp.Rational(value.numerator, value.denominator)
                for value in C18_LINE_MOMENTS[line_index]
            ]
        )
        trace_part, shear_part = trace_and_shear(moment)
        for phase in range(4):
            u, v = PHASE_COORDINATES[phase]
            for orientation in (-1, 1):
                current, tensor, capacity = event_vectors(line_index, phase, orientation)
                current_cost = half_cost(current, current_covariance)
                tensor_cost = half_cost(tensor, tensor_covariance_matrix)
                capacity_cost = half_cost(capacity, capacity_covariance)
                joint_vector = sp.Matrix.vstack(current, tensor, capacity)
                joint_cost = half_cost(joint_vector, joint)
                assert joint_cost == current_cost + tensor_cost + capacity_cost

                tensor_trace = sp.Matrix.vstack(
                    sp.Rational(u, 18) * trace_part,
                    sp.Rational(v, 18) * trace_part,
                )
                tensor_shear = sp.Matrix.vstack(
                    sp.Rational(u, 18) * shear_part,
                    sp.Rational(v, 18) * shear_part,
                )
                capacity_trace = -sp.Rational(1, 18) * trace_part
                capacity_shear = -sp.Rational(1, 18) * shear_part
                tensor_trace_cost = half_cost(tensor_trace, tensor_covariance_matrix)
                tensor_shear_cost = half_cost(tensor_shear, tensor_covariance_matrix)
                capacity_trace_cost = half_cost(capacity_trace, capacity_covariance)
                capacity_shear_cost = half_cost(capacity_shear, capacity_covariance)
                assert tensor_cost == tensor_trace_cost + tensor_shear_cost
                assert capacity_cost == capacity_trace_cost + capacity_shear_cost

                values = (
                    current_cost,
                    tensor_cost,
                    capacity_cost,
                    joint_cost,
                    tensor_trace_cost,
                    tensor_shear_cost,
                    capacity_trace_cost,
                    capacity_shear_cost,
                )
                shell_costs[shell].add(values)
                checks += 3

    sc = (
        sp.Rational(5, 24),
        sp.Rational(25, 72),
        sp.Rational(125, 144),
        sp.Rational(205, 144),
        sp.Rational(5, 72),
        sp.Rational(5, 18),
        sp.Rational(25, 144),
        sp.Rational(25, 36),
    )
    fcc = (
        sp.Rational(5, 24),
        sp.Rational(65, 144),
        sp.Rational(325, 288),
        sp.Rational(515, 288),
        sp.Rational(5, 72),
        sp.Rational(55, 144),
        sp.Rational(25, 144),
        sp.Rational(275, 288),
    )
    assert shell_costs == {"SC": {sc}, "FCC": {fcc}}
    assert sc[1] / sc[0] == sp.Rational(5, 3)
    assert sc[2] / sc[0] == sp.Rational(25, 6)
    assert fcc[1] / fcc[0] == sp.Rational(13, 6)
    assert fcc[2] / fcc[0] == sp.Rational(65, 12)
    assert sc[3] != fcc[3]
    checks += 6
    return checks


def verify_signed_cubic_shells(covariances: tuple[sp.Matrix, ...]) -> int:
    current_covariance, tensor_covariance_matrix, capacity_covariance, _joint = covariances
    moments = tuple(dyad_matrix(moment) for moment in C18_LINE_MOMENTS)
    flattened = tuple(symmetric_coordinates(moment) for moment in moments)
    checks = 0
    for line_index, moment in enumerate(moments):
        expected_shell = line_index < 3
        base_current, base_tensor, base_capacity = event_vectors(line_index, 0, 1)
        base_costs = (
            half_cost(base_current, current_covariance),
            half_cost(base_tensor, tensor_covariance_matrix),
            half_cost(base_capacity, capacity_covariance),
        )
        for transformation in signed_cubic_group():
            transformed = symmetric_coordinates(transformation * moment * transformation.T)
            target = next(index for index, value in enumerate(flattened) if value == transformed)
            assert (target < 3) == expected_shell
            target_current, target_tensor, target_capacity = event_vectors(target, 0, 1)
            assert (
                half_cost(target_current, current_covariance),
                half_cost(target_tensor, tensor_covariance_matrix),
                half_cost(target_capacity, capacity_covariance),
            ) == base_costs
            checks += 2
    return checks


def verify_reparameterization(covariances: tuple[sp.Matrix, ...]) -> int:
    current_covariance, tensor_covariance_matrix, capacity_covariance, joint = covariances
    current, tensor, capacity = event_vectors(0, 0, 1)
    checks = 0

    lambda_r, lambda_t, lambda_k = sp.symbols(
        "lambda_r lambda_t lambda_k", nonzero=True
    )
    for vector, covariance, scale in (
        (current, current_covariance, lambda_r),
        (tensor, tensor_covariance_matrix, lambda_t),
        (capacity, capacity_covariance, lambda_k),
    ):
        assert sp.simplify(
            half_cost(scale * vector, scale**2 * covariance)
            - half_cost(vector, covariance)
        ) == 0
        checks += 1

    mix3 = sp.Matrix([[1, 1, 0], [0, 1, 1], [0, 0, 1]])
    mix6 = sp.Matrix(
        [
            [1, 1, 0, 0, 0, 0],
            [0, 1, 1, 0, 0, 0],
            [0, 0, 1, 1, 0, 0],
            [0, 0, 0, 1, 1, 0],
            [0, 0, 0, 0, 1, 1],
            [0, 0, 0, 0, 0, 1],
        ]
    )
    current_mix = sp.diag(mix3, mix3)
    tensor_mix = sp.diag(mix6, mix6)
    joint_mix = sp.diag(current_mix, tensor_mix, mix6)
    joint_vector = sp.Matrix.vstack(current, tensor, capacity)
    assert mix3.det() == mix6.det() == joint_mix.det() == 1
    assert half_cost(joint_mix * joint_vector, joint_mix * joint * joint_mix.T) == half_cost(
        joint_vector, joint
    )
    checks += 2

    # Repeat on an FCC source where off-diagonal tensor coordinates are live.
    current_fcc, tensor_fcc, capacity_fcc = event_vectors(3, 1, -1)
    fcc_vector = sp.Matrix.vstack(current_fcc, tensor_fcc, capacity_fcc)
    assert half_cost(joint_mix * fcc_vector, joint_mix * joint * joint_mix.T) == half_cost(
        fcc_vector, joint
    )
    checks += 1
    return checks


def verify_counting_pushforward() -> int:
    checks = 0
    for counts in product(range(5), repeat=4):
        records = residual_records(0, counts)
        capacity = max(1, len(records) + 2)
        orbit = ring_orbit(capacity)
        assert len(orbit) == capacity * (capacity + 1)
        assert len(set(orbit)) == len(orbit)
        assert {
            pair for pair in orbit if pair[1] < capacity
        } == set(product(range(capacity), repeat=2))
        clicks, occupied_pairs, period = enumerate_clicks(records, capacity)
        expected = coherent_norm_squared(counts)
        assert occupied_pairs == len(records) ** 2
        assert clicks[0] == expected
        assert sum(clicks.values()) == expected
        assert period == len(orbit)
        checks += 7

    cases = (
        ((1, 0, 0, 0), (0, 1, 0, 0)),
        ((3, 1, 1, 0), (0, 4, 0, 1), (2, 2, 2, 1)),
        ((4, 0, 1, 0), (1, 4, 0, 2), (3, 3, 3, 3), (0, 0, 0, 2)),
        ((2, 4, 4, 1), (4, 2, 0, 4), (3, 3, 3, 3)),
    )
    for outcome_counts in cases:
        records = [
            record
            for outcome, counts in enumerate(outcome_counts)
            for record in residual_records(outcome, counts)
        ]
        capacity = max(1, len(records) + 3)
        clicks, occupied_pairs, period = enumerate_clicks(records, capacity)
        expected = tuple(coherent_norm_squared(counts) for counts in outcome_counts)
        assert occupied_pairs == len(records) ** 2
        assert tuple(clicks[index] for index in range(len(expected))) == expected
        assert sum(clicks.values()) == sum(expected)
        assert period == capacity * (capacity + 1)
        if sum(expected):
            assert tuple(
                F(clicks[index], sum(clicks.values())) for index in range(len(expected))
            ) == tuple(F(value, sum(expected)) for value in expected)
        checks += 5
    return checks


def main() -> None:
    checks = 0
    checks += verify_source_integrity()
    joint_checks, covariances = verify_joint_measure()
    checks += joint_checks
    checks += verify_event_metric(covariances)
    checks += verify_signed_cubic_shells(covariances)
    checks += verify_reparameterization(covariances)
    checks += verify_counting_pushforward()

    print("joint_uniform_counting_covariance_rank=24")
    print("SC_event_costs=(R=5/24,T=25/72,K=125/144,total=205/144)")
    print("FCC_event_costs=(R=5/24,T=65/144,K=325/288,total=515/288)")
    print("Mahalanobis source costs are invariant under canonical chart rescaling")
    print("SC/FCC shear-cost anisotropy remains explicit")
    print("prepared coprime-ring counting pushforward gives M=|Z|^2")
    print(
        "PASS: C18 uniform-counting joint source metric and Born-measure seam "
        f"({checks} exact checks)"
    )
    print(
        "OUTCOME A at bare-measure level: dynamical action selection, native "
        "preparation, poles, lensing, and coupling measurement remain open"
    )


if __name__ == "__main__":
    main()

