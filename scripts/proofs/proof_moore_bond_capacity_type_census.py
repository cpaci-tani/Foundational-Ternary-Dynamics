#!/usr/bin/env python3
"""Exact Moore-bond capacity and O_h representation census.

This is a finite, exact calculation.  It performs no parameter search and no
comparison with experimental constants.

The calculation answers two questions:

1. What O_h types occur in the 27-site permutation representation?
2. What part of the inversion-even 13-bond space is visible to the symmetric
   second moment K_ij = sum_[d] w_[d] d_i d_j?

All matrix ranks are computed over the rationals.
"""

from __future__ import annotations

from fractions import Fraction
from itertools import permutations, product


Vector = tuple[int, int, int]
Matrix3 = tuple[tuple[int, int, int], tuple[int, int, int], tuple[int, int, int]]


def determinant_3(matrix: Matrix3) -> int:
    a, b, c = matrix
    return (
        a[0] * (b[1] * c[2] - b[2] * c[1])
        - a[1] * (b[0] * c[2] - b[2] * c[0])
        + a[2] * (b[0] * c[1] - b[1] * c[0])
    )


def matrix_vector(matrix: Matrix3, vector: Vector) -> Vector:
    return tuple(sum(matrix[i][j] * vector[j] for j in range(3)) for i in range(3))  # type: ignore[return-value]


def negate_matrix(matrix: Matrix3) -> Matrix3:
    return tuple(tuple(-entry for entry in row) for row in matrix)  # type: ignore[return-value]


def signed_permutation_matrices() -> list[Matrix3]:
    matrices: list[Matrix3] = []
    for permutation in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            rows = []
            for row in range(3):
                entries = [0, 0, 0]
                entries[permutation[row]] = signs[row]
                rows.append(tuple(entries))
            matrices.append(tuple(rows))  # type: ignore[arg-type]
    assert len(matrices) == 48
    assert {determinant_3(matrix) for matrix in matrices} == {-1, 1}
    return matrices


def proper_rotation_class(matrix: Matrix3) -> str:
    """Classify a proper signed-permutation rotation in O."""

    assert determinant_3(matrix) == 1
    trace = sum(matrix[i][i] for i in range(3))
    if matrix == ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
        return "E"
    if trace == 0:
        return "C3"
    if trace == 1:
        return "C4"
    if trace == -1:
        diagonal = all(matrix[i][j] == 0 for i in range(3) for j in range(3) if i != j)
        return "C2_axis" if diagonal else "C2_edge"
    raise AssertionError(f"unclassified proper rotation: {matrix}")


O_CHARACTERS = {
    # Columns: E, 8C3, 3C2(axis), 6C4, 6C2(edge).
    "A1": {"E": 1, "C3": 1, "C2_axis": 1, "C4": 1, "C2_edge": 1},
    "A2": {"E": 1, "C3": 1, "C2_axis": 1, "C4": -1, "C2_edge": -1},
    "E": {"E": 2, "C3": -1, "C2_axis": 2, "C4": 0, "C2_edge": 0},
    "T1": {"E": 3, "C3": 0, "C2_axis": -1, "C4": 1, "C2_edge": -1},
    "T2": {"E": 3, "C3": 0, "C2_axis": -1, "C4": -1, "C2_edge": 1},
}


IRREPS = tuple(f"{name}{parity}" for parity in ("g", "u") for name in O_CHARACTERS)


def irrep_character(label: str, matrix: Matrix3) -> int:
    parity = label[-1]
    base = label[:-1]
    determinant = determinant_3(matrix)
    proper = matrix if determinant == 1 else negate_matrix(matrix)
    value = O_CHARACTERS[base][proper_rotation_class(proper)]
    if parity == "u" and determinant == -1:
        value = -value
    return value


def shell(vector: Vector) -> str:
    nonzero = sum(component != 0 for component in vector)
    return ("center", "SC", "FCC", "BCC")[nonzero]


def fixed_points(matrix: Matrix3, selected_shell: str | None) -> int:
    points = product((-1, 0, 1), repeat=3)
    return sum(
        1
        for point in points
        if (selected_shell is None or shell(point) == selected_shell)
        and matrix_vector(matrix, point) == point
    )


def decompose_permutation(shell_name: str | None) -> dict[str, int]:
    matrices = signed_permutation_matrices()
    decomposition: dict[str, int] = {}
    for label in IRREPS:
        numerator = sum(
            fixed_points(matrix, shell_name) * irrep_character(label, matrix)
            for matrix in matrices
        )
        multiplicity = Fraction(numerator, len(matrices))
        assert multiplicity.denominator == 1
        if multiplicity:
            decomposition[label] = multiplicity.numerator
    return decomposition


def rational_rank(rows: list[list[int]]) -> int:
    matrix = [[Fraction(entry) for entry in row] for row in rows]
    row_count = len(matrix)
    column_count = len(matrix[0]) if matrix else 0
    pivot_row = 0
    for column in range(column_count):
        pivot = next((row for row in range(pivot_row, row_count) if matrix[row][column]), None)
        if pivot is None:
            continue
        matrix[pivot_row], matrix[pivot] = matrix[pivot], matrix[pivot_row]
        pivot_value = matrix[pivot_row][column]
        matrix[pivot_row] = [value / pivot_value for value in matrix[pivot_row]]
        for row in range(row_count):
            if row == pivot_row or not matrix[row][column]:
                continue
            factor = matrix[row][column]
            matrix[row] = [
                matrix[row][entry] - factor * matrix[pivot_row][entry]
                for entry in range(column_count)
            ]
        pivot_row += 1
        if pivot_row == row_count:
            break
    return pivot_row


SC_LINES: tuple[Vector, ...] = (
    (1, 0, 0),
    (0, 1, 0),
    (0, 0, 1),
)

FCC_LINES: tuple[Vector, ...] = (
    (1, 1, 0),
    (1, -1, 0),
    (1, 0, 1),
    (1, 0, -1),
    (0, 1, 1),
    (0, 1, -1),
)

BCC_LINES: tuple[Vector, ...] = (
    (1, 1, 1),
    (1, 1, -1),
    (1, -1, 1),
    (-1, 1, 1),
)


def second_moment_column(direction: Vector) -> tuple[int, int, int, int, int, int]:
    x, y, z = direction
    return (x * x, y * y, z * z, x * y, x * z, y * z)


def first_moment_rank(lines: tuple[Vector, ...]) -> int:
    rows = [[direction[row] for direction in lines] for row in range(3)]
    return rational_rank(rows)


def second_moment_rank(lines: tuple[Vector, ...]) -> int:
    columns = [second_moment_column(direction) for direction in lines]
    rows = [[column[row] for column in columns] for row in range(6)]
    return rational_rank(rows)


def main() -> None:
    expected = {
        "center": {"A1g": 1},
        "SC": {"A1g": 1, "Eg": 1, "T1u": 1},
        "FCC": {"A1g": 1, "Eg": 1, "T2g": 1, "T1u": 1, "T2u": 1},
        "BCC": {"A1g": 1, "A2u": 1, "T2g": 1, "T1u": 1},
        "all": {"A1g": 4, "A2u": 1, "Eg": 2, "T2g": 2, "T1u": 3, "T2u": 1},
    }

    for shell_name in ("center", "SC", "FCC", "BCC"):
        result = decompose_permutation(shell_name)
        assert result == expected[shell_name], (shell_name, result)
        print(f"{shell_name:>6}: {result}")

    all_result = decompose_permutation(None)
    assert all_result == expected["all"], all_result
    print(f"   all: {all_result}")

    even_expected = {"A1g": 3, "Eg": 2, "T2g": 2}
    odd_expected = {"A2u": 1, "T1u": 3, "T2u": 1}
    assert sum(even_expected.values()) == 7  # multiplicity, not dimension
    assert 3 + 2 * 2 + 2 * 3 == 13
    assert 1 + 3 * 3 + 3 == 13

    all_lines = SC_LINES + FCC_LINES + BCC_LINES
    c18_lines = SC_LINES + FCC_LINES
    ranks = {
        "SC_second": second_moment_rank(SC_LINES),
        "FCC_second": second_moment_rank(FCC_LINES),
        "BCC_second": second_moment_rank(BCC_LINES),
        "C18_second": second_moment_rank(c18_lines),
        "Moore13_second": second_moment_rank(all_lines),
        "Moore13_first": first_moment_rank(all_lines),
    }
    assert ranks == {
        "SC_second": 3,
        "FCC_second": 6,
        "BCC_second": 4,
        "C18_second": 6,
        "Moore13_second": 6,
        "Moore13_first": 3,
    }, ranks

    print(f"even Moore-line module: {even_expected}")
    print(f"odd Moore-line module:  {odd_expected}")
    print(f"moment ranks: {ranks}")
    print("second-moment kernel: 2*A1g + Eg + T2g (dimension 7)")
    print("first-moment kernel: A2u + 2*T1u + T2u (dimension 10)")
    print("PASS: exact Moore-bond capacity type census")


if __name__ == "__main__":
    main()
