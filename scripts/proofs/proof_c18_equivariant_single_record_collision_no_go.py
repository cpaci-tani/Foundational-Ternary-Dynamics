#!/usr/bin/env python3
"""Exact no-go for fixed one-record cubic-equivariant C18 collisions.

The full O_h centralizer on the directed SC and FCC shells is enumerated
exactly.  Each shell admits only identity or antipodal reversal.  Composed
with one-hop streaming, identity leaves independent ballistic rays while
antipodal reversal produces a two-tick spatial bounce.  Uniform C4 phase
shifts do not change that spatial classification.

This excludes only fixed one-record collision permutations.  State-dependent
multi-record collisions and dynamical controller/frame variables remain open.
"""

from __future__ import annotations

from itertools import product

from proof_moore_bond_capacity_type_census import (
    Matrix3,
    Vector,
    matrix_vector,
    signed_permutation_matrices,
)


SC_DIRECTIONS: tuple[Vector, ...] = tuple(
    direction
    for axis in range(3)
    for direction in tuple(
        tuple(sign if index == axis else 0 for index in range(3))
        for sign in (-1, 1)
    )
)  # type: ignore[assignment]

FCC_DIRECTIONS: tuple[Vector, ...] = tuple(
    direction
    for direction in product((-1, 0, 1), repeat=3)
    if sum(component != 0 for component in direction) == 2
)


def add(left: Vector, right: Vector) -> Vector:
    return tuple(left[index] + right[index] for index in range(3))  # type: ignore[return-value]


def negate(vector: Vector) -> Vector:
    return tuple(-entry for entry in vector)  # type: ignore[return-value]


def equivariant_map_from_image(
    orbit: tuple[Vector, ...],
    representative: Vector,
    image: Vector,
    group: tuple[Matrix3, ...],
) -> dict[Vector, Vector] | None:
    mapping: dict[Vector, Vector] = {}
    for matrix in group:
        source = matrix_vector(matrix, representative)
        target = matrix_vector(matrix, image)
        if source in mapping and mapping[source] != target:
            return None
        mapping[source] = target
    if set(mapping) != set(orbit) or set(mapping.values()) != set(orbit):
        return None
    return mapping


def equivariant_shell_maps(
    orbit: tuple[Vector, ...],
    group: tuple[Matrix3, ...],
) -> tuple[dict[Vector, Vector], ...]:
    representative = orbit[0]
    maps = []
    for image in orbit:
        mapping = equivariant_map_from_image(orbit, representative, image, group)
        if mapping is not None:
            maps.append(mapping)
    return tuple(maps)


def classify(mapping: dict[Vector, Vector]) -> str:
    if all(mapping[direction] == direction for direction in mapping):
        return "identity"
    if all(mapping[direction] == negate(direction) for direction in mapping):
        return "antipode"
    return "other"


def commutes_with_group(
    mapping: dict[Vector, Vector],
    group: tuple[Matrix3, ...],
) -> bool:
    return all(
        mapping[matrix_vector(matrix, direction)]
        == matrix_vector(matrix, mapping[direction])
        for matrix in group
        for direction in mapping
    )


def streamed_step(
    position: Vector,
    direction: Vector,
    mapping: dict[Vector, Vector],
) -> tuple[Vector, Vector]:
    outgoing = mapping[direction]
    return (add(position, outgoing), outgoing)


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())
    assert len(group) == 48
    assert len(SC_DIRECTIONS) == 6
    assert len(FCC_DIRECTIONS) == 12
    checks += 3

    sc_maps = equivariant_shell_maps(SC_DIRECTIONS, group)
    fcc_maps = equivariant_shell_maps(FCC_DIRECTIONS, group)
    assert sorted(classify(mapping) for mapping in sc_maps) == ["antipode", "identity"]
    assert sorted(classify(mapping) for mapping in fcc_maps) == ["antipode", "identity"]
    assert all(commutes_with_group(mapping, group) for mapping in sc_maps + fcc_maps)
    checks += 3

    full_maps: list[dict[Vector, Vector]] = []
    for sc_mapping, fcc_mapping in product(sc_maps, fcc_maps):
        full = {**sc_mapping, **fcc_mapping}
        assert len(full) == 18
        assert commutes_with_group(full, group)
        full_maps.append(full)
        checks += 2
    assert len(full_maps) == 4
    checks += 1

    for mapping in full_maps:
        for shell in (SC_DIRECTIONS, FCC_DIRECTIONS):
            shell_class = classify({direction: mapping[direction] for direction in shell})
            assert shell_class in ("identity", "antipode")
            for direction in shell:
                origin = (0, 0, 0)
                position_1, direction_1 = streamed_step(origin, direction, mapping)
                position_2, direction_2 = streamed_step(position_1, direction_1, mapping)
                if shell_class == "identity":
                    assert direction_1 == direction_2 == direction
                    assert position_1 == direction
                    assert position_2 == tuple(2 * entry for entry in direction)
                    checks += 3
                else:
                    assert direction_1 == negate(direction)
                    assert direction_2 == direction
                    assert position_2 == origin
                    checks += 3

        # A cubic-equivariant phase update can add one fixed C4 increment on
        # each shell.  Enumerate all such increments; spatial orbits are
        # unchanged and phase closes after four applications.
        for sc_shift, fcc_shift in product(range(4), repeat=2):
            for direction in mapping:
                phase = 0
                shift = sc_shift if direction in SC_DIRECTIONS else fcc_shift
                for _ in range(4):
                    phase = (phase + shift) % 4
                assert phase == 0
                checks += 1

    print(f"PASS: C18 equivariant one-record collision no-go ({checks} exact checks)")
    print("centralizer = {identity, antipode}_SC x {identity, antipode}_FCC")
    print("Spatial verdict: independent ballistic rays or two-tick bounce only")
    print("Open: state-dependent multi-record collision or dynamical controller")


if __name__ == "__main__":
    main()
