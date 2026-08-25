#!/usr/bin/env python3
"""Exact O_h-symmetrized Moore-local dual-capacity mixing schedule.

A mixed-radix walk on Z_L^3 visits every relative translation exactly once.
Its carry steps are SC, FCC, or BCC Moore-neighbor hops.  Pairing the walk with
its reverse removes drift.  Running the pair in all 48 signed-cubic frames
gives an exactly O_h-invariant step multiset with isotropic second moment.

Because every relative translation is visited equally, arbitrary primal and
dual binary capacity patterns factorize exactly over the complete schedule.
The construction is a finite global-clock reference schedule, not yet a
locally selected collision, cotangent/TT composition, static source solution,
or lensing result.
"""

from __future__ import annotations

from itertools import product

from sympy import Matrix, Rational

from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)


Vector = tuple[int, int, int]


def add_mod(left: Vector, right: Vector, length: int) -> Vector:
    return tuple(
        (left[axis] + right[axis]) % length for axis in range(3)
    )  # type: ignore[return-value]


def negate(vector: Vector) -> Vector:
    return tuple(-component for component in vector)  # type: ignore[return-value]


def base_position(pointer: int, length: int) -> Vector:
    return (
        pointer % length,
        (pointer // length) % length,
        (pointer // (length * length)) % length,
    )


def base_step(pointer: int, length: int) -> Vector:
    x, y, _z = base_position(pointer, length)
    return (
        1,
        int(x == length - 1),
        int(x == length - 1 and y == length - 1),
    )


def transform_position(matrix, position: Vector, length: int) -> Vector:
    return tuple(
        component % length for component in matrix_vector(matrix, position)
    )  # type: ignore[return-value]


def transform_step(matrix, step: Vector) -> Vector:
    return tuple(matrix_vector(matrix, step))  # type: ignore[return-value]


def frame_block(length: int, matrix, reverse: bool):
    size = length**3
    for tick_index in range(size):
        if not reverse:
            pointer = tick_index
            position = base_position(pointer, length)
            step = base_step(pointer, length)
        else:
            pointer = (-tick_index) % size
            position = base_position(pointer, length)
            previous_forward_pointer = (pointer - 1) % size
            step = negate(base_step(previous_forward_pointer, length))
        yield (
            transform_position(matrix, position, length),
            transform_step(matrix, step),
        )


def dot(left: Vector, right: Vector) -> int:
    return sum(a * b for a, b in zip(left, right))


def verify_every_block_is_local_translation_bijection() -> int:
    checks = 0
    group = tuple(signed_permutation_matrices())
    assert len(group) == 48
    checks += 1

    for length in range(2, 9):
        expected_positions = set(product(range(length), repeat=3))
        for matrix in group:
            for reverse in (False, True):
                block = tuple(frame_block(length, matrix, reverse))
                positions = tuple(position for position, _step in block)
                assert len(block) == length**3
                assert set(positions) == expected_positions
                checks += 2

                for index, (position, step) in enumerate(block):
                    next_position = block[(index + 1) % len(block)][0]
                    assert step != (0, 0, 0)
                    assert all(component in (-1, 0, 1) for component in step)
                    assert add_mod(position, step, length) == next_position
                    checks += 3
    return checks


def verify_zero_drift_and_isotropic_step_covariance() -> int:
    checks = 0
    group = tuple(signed_permutation_matrices())
    for length in range(2, 13):
        first_moment = Matrix.zeros(3, 1)
        second_moment = Matrix.zeros(3, 3)
        step_count = 0
        for matrix in group:
            for reverse in (False, True):
                for _position, step in frame_block(length, matrix, reverse):
                    vector = Matrix(step)
                    first_moment += vector
                    second_moment += vector * vector.T
                    step_count += 1

        expected_count = 96 * length**3
        expected_trace = 96 * (length**3 + length**2 + length)
        expected_diagonal = expected_trace // 3
        assert step_count == expected_count
        assert first_moment == Matrix.zeros(3, 1)
        assert second_moment == expected_diagonal * Matrix.eye(3)
        assert second_moment.trace() == expected_trace
        assert Rational(expected_diagonal, expected_count) == (
            Rational(1, 3)
            * (1 + Rational(1, length) + Rational(1, length * length))
        )
        checks += 5
    return checks


def translate_pattern(pattern, displacement: Vector, length: int):
    translated = []
    for position in product(range(length), repeat=3):
        source = tuple(
            (position[axis] - displacement[axis]) % length
            for axis in range(3)
        )
        source_index = source[0] + length * source[1] + length * length * source[2]
        translated.append(pattern[source_index])
    return tuple(translated)


def joint_count(primal, dual, displacement: Vector, length: int) -> int:
    translated = translate_pattern(dual, displacement, length)
    return sum(left * right for left, right in zip(primal, translated))


def verify_arbitrary_pattern_factorization() -> int:
    checks = 0
    length = 2
    size = length**3
    configurations = tuple(product((0, 1), repeat=size))
    displacements = tuple(product(range(length), repeat=3))
    assert len(configurations) == 256
    assert len(displacements) == size
    checks += 2

    # Exhaust every pair of 2x2x2 binary patterns.  A single displacement
    # census suffices because every frame/reversal block contains exactly this
    # same translation set.
    for primal in configurations:
        primal_count = sum(primal)
        for dual in configurations:
            dual_count = sum(dual)
            total_joint = sum(
                joint_count(primal, dual, displacement, length)
                for displacement in displacements
            )
            assert total_joint == primal_count * dual_count
            assert Rational(total_joint, size * size) == (
                Rational(primal_count, size) * Rational(dual_count, size)
            )
            checks += 2

    # Structured larger-volume controls verify the same identity without
    # searching configurations.
    for length in range(3, 9):
        positions = tuple(product(range(length), repeat=3))
        patterns = (
            tuple(0 for _ in positions),
            tuple(1 for _ in positions),
            tuple(int(sum(position) % 2 == 0) for position in positions),
            tuple(int(position[0] == 0) for position in positions),
            tuple(int(position[1] <= 1) for position in positions),
            tuple(int(position[2] == position[0]) for position in positions),
        )
        for primal in patterns:
            for dual in patterns:
                total_joint = sum(
                    joint_count(primal, dual, displacement, length)
                    for displacement in positions
                )
                assert total_joint == sum(primal) * sum(dual)
                checks += 1
    return checks


def verify_full_schedule_multiplicity() -> int:
    checks = 0
    group = tuple(signed_permutation_matrices())
    for length in range(2, 9):
        multiplicity = {
            position: 0 for position in product(range(length), repeat=3)
        }
        for matrix in group:
            for reverse in (False, True):
                for position, _step in frame_block(length, matrix, reverse):
                    multiplicity[position] += 1
        assert set(multiplicity.values()) == {96}
        checks += 1
    return checks


def main() -> None:
    checks = verify_every_block_is_local_translation_bijection()
    checks += verify_zero_drift_and_isotropic_step_covariance()
    checks += verify_arbitrary_pattern_factorization()
    checks += verify_full_schedule_multiplicity()

    print("mixed-radix Z_L^3 walk visits every relative translation once")
    print("each update is one SC/FCC/BCC Moore-neighbor hop with exact inverse")
    print("forward+reverse x 48 signed-cubic frames: zero drift, isotropic covariance")
    print("every displacement occurs 96 times in the full schedule")
    print("arbitrary 3D binary patterns: averaged joint=open_primal*open_dual exactly")
    print(
        "PASS: O_h-symmetrized Moore-local dual-capacity mixing schedule "
        f"({checks} exact checks)"
    )
    print(
        "Open: native frame scheduling, C18-only alternative if required, "
        "source ledger, cotangent/C3 composition, TT lift, static pole, and lensing"
    )


if __name__ == "__main__":
    main()
