#!/usr/bin/env python3
"""Exact-rational certificate for the FTD-0675 modal mass metric."""

from fractions import Fraction as F


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def dot(left: list[F], right: list[F]) -> F:
    return sum((a * b for a, b in zip(left, right)), F(0))


def main() -> None:
    mass = F(1, 4)
    modes = [[F(2), F(0)], [F(0), F(2)]]
    for i, left in enumerate(modes):
        for j, right in enumerate(modes):
            require(
                mass * dot(left, right) == (F(1) if i == j else F(0)),
                "mass orthonormality",
            )

    q = [F(3, 7), F(-2, 9)]
    canonical_p = [F(5, 11), F(4, 13)]
    omega = [F(2, 3), F(5, 6)]
    x = [sum((modes[j][i] * q[j] for j in range(2)), F(0))
         for i in range(2)]
    p = [mass * sum((modes[j][i] * canonical_p[j]
                    for j in range(2)), F(0))
         for i in range(2)]
    recovered_q = [mass * dot(mode, x) for mode in modes]
    recovered_p = [dot(mode, p) for mode in modes]
    require(recovered_q == q, "q=V^T M x")
    require(recovered_p == canonical_p, "P=V^T p")

    cartesian = dot(p, p) / (2 * mass)
    modal = dot(canonical_p, canonical_p) / 2
    require(cartesian == modal, "kinetic canonical transform")

    legacy_q = [dot(mode, x) for mode in modes]
    require(legacy_q == [value / mass for value in q],
            "legacy coordinate misses M")
    true_potential = sum(
        (omega[i] * omega[i] * q[i] * q[i] / 2 for i in range(2)), F(0)
    )
    legacy_potential = sum(
        (omega[i] * omega[i] * legacy_q[i] * legacy_q[i] / 2
         for i in range(2)), F(0)
    )
    require(legacy_potential == true_potential / (mass * mass),
            "legacy potential overweight")
    print(
        "FTD-0675 mass-metric modal certificate: PASS "
        f"mass={mass} legacy_potential_factor={1/(mass*mass)} "
        "arithmetic=rational"
    )


if __name__ == "__main__":
    main()
