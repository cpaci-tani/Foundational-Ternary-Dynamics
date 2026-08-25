#!/usr/bin/env python3
"""Exact orientation price and discrete Hodge-Maxwell target.

The script proves that an unordered perpendicular bond pair cannot choose an
axial plaquette normal equivariantly under O_h, while one orientation bit makes
the normal well defined.  It then certifies the exact centered-incidence curl
generator: divergence constraints, anti-Hermiticity, and two transverse
polarizations with linear |q| dispersion.

This is a kinematic target and representation theorem.  It does not construct
the finite local transaction/permutation that realizes the generator.
"""

from __future__ import annotations

from collections import Counter
from itertools import combinations

from sympy import I, Matrix, eye, symbols

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_moore_bond_capacity_type_census import (
    determinant_3,
    matrix_vector,
    signed_permutation_matrices,
)


Vector = tuple[int, int, int]
OrientedPlane = tuple[Vector, Vector, int]


def dot(left: Vector, right: Vector) -> int:
    return sum(a * b for a, b in zip(left, right))


def cross(left: Vector, right: Vector) -> Vector:
    return (
        left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0],
    )


def negate(vector: Vector) -> Vector:
    return tuple(-entry for entry in vector)  # type: ignore[return-value]


def line(direction: Vector) -> Vector:
    opposite = negate(direction)
    return min(direction, opposite)


def canonical_oriented_plane(
    first: Vector,
    second: Vector,
    orientation: int,
) -> OrientedPlane:
    assert orientation in (-1, 1)
    assert dot(first, second) == 0
    presentations = (
        (first, second, orientation),
        (second, first, -orientation),
    )
    return min(presentations)


def axial_normal(state: OrientedPlane) -> Vector:
    first, second, orientation = state
    return tuple(
        orientation * component for component in cross(first, second)
    )  # type: ignore[return-value]


def transform_oriented_plane(matrix, state: OrientedPlane) -> OrientedPlane:
    first, second, orientation = state
    return canonical_oriented_plane(
        matrix_vector(matrix, first),
        matrix_vector(matrix, second),
        orientation,
    )


def axial_transform(matrix, normal: Vector) -> Vector:
    determinant = determinant_3(matrix)
    return tuple(
        determinant * component for component in matrix_vector(matrix, normal)
    )  # type: ignore[return-value]


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())
    assert len(group) == 48
    checks += 1

    # Unoriented perpendicular SC line pairs have no canonical signed normal.
    # For each plane, an O_h stabilizer fixes the line pair but reverses its
    # candidate axial normal.
    positive_axes = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
    for first, second in combinations(positive_axes, 2):
        plane = frozenset((line(first), line(second)))
        normal = cross(first, second)
        witnesses = []
        for matrix in group:
            transformed_plane = frozenset(
                (
                    line(matrix_vector(matrix, first)),
                    line(matrix_vector(matrix, second)),
                )
            )
            if transformed_plane != plane:
                continue
            if axial_transform(matrix, normal) == negate(normal):
                witnesses.append(matrix)
        assert witnesses
        checks += 1

    # One sign bit attached to pair presentation removes the ambiguity.
    oriented_states = {
        canonical_oriented_plane(first, second, orientation)
        for first in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(first, second) == 0
        for orientation in (-1, 1)
    }
    assert len(oriented_states) == 24
    normal_histogram = Counter(axial_normal(state) for state in oriented_states)
    assert set(normal_histogram) == set(SC_DIRECTIONS)
    assert set(normal_histogram.values()) == {4}
    checks += 3

    for state in oriented_states:
        first, second, orientation = state
        assert canonical_oriented_plane(second, first, -orientation) == state
        assert axial_normal(
            canonical_oriented_plane(second, first, -orientation)
        ) == axial_normal(state)
        for matrix in group:
            transformed = transform_oriented_plane(matrix, state)
            assert axial_normal(transformed) == axial_transform(
                matrix, axial_normal(state)
            )
            checks += 1
        checks += 2

    # Exact centered-incidence Fourier symbol.  For the cubic cell complex,
    # q_a=2 sin(k_a/2); leave q real and symbolic so no continuum expansion is
    # used in the algebra below.
    qx, qy, qz = symbols("qx qy qz", real=True)
    eigenvalue = symbols("eigenvalue")
    q = Matrix([qx, qy, qz])
    cross_matrix = Matrix(
        [
            [0, -qz, qy],
            [qz, 0, -qx],
            [-qy, qx, 0],
        ]
    )
    curl = I * cross_matrix
    zero = Matrix.zeros(3, 3)
    generator = zero.row_join(curl).col_join((-curl).row_join(zero))

    assert cross_matrix.T == -cross_matrix
    assert curl.H == curl
    assert generator.H == -generator
    assert (q.T * curl) == Matrix.zeros(1, 3)
    assert curl * q == Matrix.zeros(3, 1)
    checks += 5

    # The polar-edge / axial-face parity assignment makes the curl generator
    # covariant under the complete signed cubic group, including reflections.
    def curl_symbol(vector: Matrix) -> Matrix:
        x_component, y_component, z_component = vector
        return I * Matrix(
            [
                [0, -z_component, y_component],
                [z_component, 0, -x_component],
                [-y_component, x_component, 0],
            ]
        )

    for matrix_tuple in group:
        rotation = Matrix(matrix_tuple)
        determinant = determinant_3(matrix_tuple)
        transformed_curl = curl_symbol(rotation * q)
        transformed_generator = zero.row_join(transformed_curl).col_join(
            (-transformed_curl).row_join(zero)
        )
        representation = Matrix.diag(rotation, determinant * rotation)
        assert transformed_generator * representation == representation * generator
        checks += 1

    q_squared = qx**2 + qy**2 + qz**2
    assert generator.charpoly(eigenvalue).as_expr().factor() == (
        eigenvalue**2 * (eigenvalue**2 + q_squared) ** 2
    )
    checks += 1

    # Curl squared is the transverse projector times |q|^2.  Hence the full
    # generator squares to -|q|^2 on both transverse field sectors.
    assert (curl * curl).applyfunc(lambda entry: entry.simplify()) == (
        q_squared * eye(3) - q * q.T
    )
    checks += 1

    for wavevector in (
        (1, 0, 0),
        (0, 1, 0),
        (0, 0, 1),
        (1, 1, 0),
        (1, 1, 1),
        (1, 2, 3),
    ):
        substitutions = dict(zip((qx, qy, qz), wavevector))
        concrete_generator = generator.subs(substitutions)
        constraints = Matrix(
            [
                [*wavevector, 0, 0, 0],
                [0, 0, 0, *wavevector],
            ]
        )
        assert constraints.rank() == 2
        assert concrete_generator.rank() == 4
        assert constraints * concrete_generator == Matrix.zeros(2, 6)
        checks += 3

    print(f"PASS: oriented bond-plaquette Hodge-Maxwell target ({checks} exact checks)")
    print("unoriented perpendicular bond pair: axial sign obstructed by O_h stabilizer")
    print("one orientation bit: 24 oriented plane states -> 6 axial normals, multiplicity 4")
    print("curl generator characteristic=lambda^2(lambda^2+|q|^2)^2")
    print("two divergence constraints leave rank 4: two polarizations plus conjugates")
    print("Open: finite local permutation/lift, work, source coupling, matter, gravity, alpha")


if __name__ == "__main__":
    main()
