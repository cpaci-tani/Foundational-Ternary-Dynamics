#!/usr/bin/env python3
"""Exact-rational certificate for the FTD-0671 regional energy split."""

from fractions import Fraction as F
from itertools import product


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def dot(left: list[F], right: list[F]) -> F:
    return sum((a * b for a, b in zip(left, right)), F(0))


def matvec(matrix: list[list[F]], vector: list[F]) -> list[F]:
    return [dot(row, vector) for row in matrix]


def transpose_matvec(matrix: list[list[F]], vector: list[F]) -> list[F]:
    return [
        sum((matrix[i][j] * vector[i] for i in range(len(matrix))), F(0))
        for j in range(len(matrix[0]))
    ]


def add(left: list[F], right: list[F], scale: F = F(1)) -> list[F]:
    return [a + scale * b for a, b in zip(left, right)]


def masked(vector: list[F], mask: tuple[int, ...]) -> list[F]:
    return [value if keep else F(0) for value, keep in zip(vector, mask)]


def global_energy(electric: list[F], magnetic: list[F],
                  curl: list[list[F]], lam: F) -> F:
    return (
        dot(electric, electric) / 2
        + dot(magnetic, magnetic) / 2
        - lam * dot(magnetic, transpose_matvec(curl, electric)) / 2
    )


def regional_energy(electric: list[F], magnetic: list[F],
                    curl: list[list[F]], lam: F,
                    electric_mask: tuple[int, ...],
                    magnetic_mask: tuple[int, ...]) -> F:
    selected_electric = masked(electric, electric_mask)
    selected_magnetic = masked(magnetic, magnetic_mask)
    return (
        dot(selected_electric, selected_electric) / 2
        + dot(selected_magnetic, selected_magnetic) / 2
        - lam * (
            dot(selected_magnetic, transpose_matvec(curl, electric))
            + dot(magnetic, transpose_matvec(curl, selected_electric))
        ) / 4
    )


def complement(mask: tuple[int, ...]) -> tuple[int, ...]:
    return tuple(1 - value for value in mask)


def certify_case(curl: list[list[F]], electric0: list[F],
                 magnetic0: list[F], current: list[F], lam: F) -> int:
    magnetic1 = add(
        magnetic0, transpose_matvec(curl, electric0), -lam
    )
    electric_star = add(electric0, matvec(curl, magnetic1), lam)
    electric1 = add(electric_star, current, F(-1))

    before = global_energy(electric0, magnetic0, curl, lam)
    source_free = global_energy(electric_star, magnetic1, curl, lam)
    require(source_free == before, "source-free modified energy")

    masks_checked = 0
    for electric_mask in product((0, 1), repeat=len(electric0)):
        for magnetic_mask in product((0, 1), repeat=len(magnetic0)):
            energies = []
            for electric, magnetic in (
                (electric0, magnetic0),
                (electric_star, magnetic1),
                (electric1, magnetic1),
            ):
                inside = regional_energy(
                    electric, magnetic, curl, lam,
                    electric_mask, magnetic_mask
                )
                outside = regional_energy(
                    electric, magnetic, curl, lam,
                    complement(electric_mask), complement(magnetic_mask)
                )
                require(
                    inside + outside
                    == global_energy(electric, magnetic, curl, lam),
                    "inside/outside partition",
                )
                energies.append((inside, outside))

            transport_in = energies[1][0] - energies[0][0]
            transport_out = energies[1][1] - energies[0][1]
            source_in = energies[2][0] - energies[1][0]
            total_in = energies[2][0] - energies[0][0]
            require(transport_in + transport_out == 0,
                    "source-free boundary transfer")
            require(total_in == transport_in + source_in,
                    "regional transport/source ledger")
            masks_checked += 1

    all_electric = tuple(1 for _ in electric0)
    all_magnetic = tuple(1 for _ in magnetic0)
    full_transport = (
        regional_energy(electric_star, magnetic1, curl, lam,
                        all_electric, all_magnetic)
        - regional_energy(electric0, magnetic0, curl, lam,
                          all_electric, all_magnetic)
    )
    require(full_transport == 0, "full-region transport")
    return masks_checked


def main() -> None:
    cases = [
        (
            [
                [F(1), F(-2), F(0)],
                [F(0), F(3), F(1)],
                [F(-1), F(0), F(2)],
                [F(2), F(1), F(-1)],
            ],
            [F(2, 3), F(-1, 4), F(5, 7), F(3, 5)],
            [F(-2, 5), F(4, 9), F(1, 6)],
            [F(1, 11), F(-2, 13), F(3, 17), F(-1, 19)],
            F(1, 3),
        ),
        (
            [
                [F(0), F(1), F(-1)],
                [F(2), F(0), F(1)],
                [F(-3), F(2), F(0)],
                [F(1), F(-1), F(2)],
            ],
            [F(-3, 8), F(5, 12), F(7, 10), F(-1, 9)],
            [F(2, 7), F(-4, 11), F(3, 13)],
            [F(-1, 5), F(2, 9), F(-3, 14), F(4, 15)],
            F(2, 5),
        ),
    ]
    total_masks = sum(certify_case(*case) for case in cases)
    print(
        "FTD-0671 exact regional-energy certificate: PASS "
        f"cases={len(cases)} masks={total_masks} arithmetic=rational"
    )


if __name__ == "__main__":
    main()
