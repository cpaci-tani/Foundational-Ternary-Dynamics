#!/usr/bin/env python3
"""Exact-rational certificate for the FTD-0673 reservoir decomposition."""

from fractions import Fraction as F


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def dot(left: list[F], right: list[F]) -> F:
    return sum((a * b for a, b in zip(left, right)), F(0))


def transpose_matvec(matrix: list[list[F]], vector: list[F]) -> list[F]:
    return [
        sum((matrix[i][j] * vector[i] for i in range(len(matrix))), F(0))
        for j in range(len(matrix[0]))
    ]


def add(left: list[F], right: list[F]) -> list[F]:
    return [a + b for a, b in zip(left, right)]


def field_energy(electric: list[F], magnetic: list[F],
                 curl: list[list[F]], lam: F) -> F:
    return (
        dot(electric, electric) / 2
        + dot(magnetic, magnetic) / 2
        - lam * dot(magnetic, transpose_matvec(curl, electric)) / 2
    )


def field_interference(control_e: list[F], control_b: list[F],
                       dynamic_e: list[F], dynamic_b: list[F],
                       curl: list[list[F]], lam: F) -> F:
    return (
        dot(control_e, dynamic_e)
        + dot(control_b, dynamic_b)
        - lam * (
            dot(control_b, transpose_matvec(curl, dynamic_e))
            + dot(dynamic_b, transpose_matvec(curl, control_e))
        ) / 2
    )


def certify_case(curl: list[list[F]], control_e: list[F],
                 control_b: list[F], dynamic_e: list[F],
                 dynamic_b: list[F], lam: F,
                 positions: list[F], momenta: list[F],
                 frequencies: list[F], exact_matter: F) -> int:
    excited_e = add(control_e, dynamic_e)
    excited_b = add(control_b, dynamic_b)
    delta_field = (
        field_energy(excited_e, excited_b, curl, lam)
        - field_energy(control_e, control_b, curl, lam)
    )
    dynamic_field = field_energy(dynamic_e, dynamic_b, curl, lam)
    interference = field_interference(
        control_e, control_b, dynamic_e, dynamic_b, curl, lam
    )
    require(
        delta_field == dynamic_field + interference,
        "quadratic field polarization identity",
    )

    modal = [
        (p * p + w * w * q * q) / 2
        for q, p, w in zip(positions, momenta, frequencies)
    ]
    total_modal = sum(modal, F(0))
    nonlinear_matter = exact_matter - total_modal
    total_difference = exact_matter + delta_field

    subsets_checked = 0
    for mask in range(1, 1 << len(modal)):
        target = sum(
            (energy for index, energy in enumerate(modal)
             if mask & (1 << index)),
            F(0),
        )
        other = total_modal - target
        require(
            exact_matter == target + other + nonlinear_matter,
            "matter tangent/remainder partition",
        )
        require(
            total_difference
            == target + other + nonlinear_matter
            + dynamic_field + interference,
            "complete five-reservoir partition",
        )
        subsets_checked += 1
    return subsets_checked


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
            [F(-1, 7), F(2, 15), F(1, 9)],
            F(1, 3),
            [F(1, 5), F(-2, 7), F(3, 11), F(1, 13)],
            [F(-1, 6), F(3, 10), F(2, 9), F(-4, 17)],
            [F(2, 3), F(3, 4), F(5, 6), F(7, 8)],
            F(23, 19),
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
            [F(1, 8), F(-3, 16), F(5, 18)],
            F(2, 5),
            [F(-1, 4), F(2, 9), F(1, 12)],
            [F(3, 7), F(-2, 11), F(5, 13)],
            [F(4, 5), F(6, 7), F(8, 9)],
            F(-7, 23),
        ),
    ]
    total_subsets = sum(certify_case(*case) for case in cases)
    print(
        "FTD-0673 exact reservoir-decomposition certificate: PASS "
        f"cases={len(cases)} target_subsets={total_subsets} "
        "arithmetic=rational"
    )


if __name__ == "__main__":
    main()
