#!/usr/bin/env python3
"""Exact certificate for finite Cartesian linearization and composition.

The general theorem is the orthonormal-basis proof in the companion document.
This script regression-checks representative nondegenerate instances, the
direct-sum countermodel, one exact entanglement witness, and Pauli saturation
of the Tsirelson bound.  It performs no numerical search or target matching.
"""

from __future__ import annotations

import sys
from itertools import product

from sympy import Matrix, eye, kronecker_product, simplify, sqrt, symbols


sys.stdout.reconfigure(encoding="utf-8")

checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, bool(condition), detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def unit(index: int, dimension: int) -> tuple[int, ...]:
    return tuple(1 if slot == index else 0 for slot in range(dimension))


def tensor_weight(i: int, j: int, m: int, n: int) -> tuple[int, ...]:
    """Character exponent θ_i+φ_j in the independent phase torus."""

    return unit(i, m) + unit(j, n)


def direct_sum_weights(m: int, n: int) -> tuple[tuple[int, ...], ...]:
    """Character exponents θ_i or φ_j for H_A direct-sum H_B."""

    left = tuple(unit(i, m) + (0,) * n for i in range(m))
    right = tuple((0,) * m + unit(j, n) for j in range(n))
    return left + right


def main() -> None:
    instances = ((2, 2), (2, 3), (3, 2), (3, 4))
    bijections = 0
    for m, n in instances:
        pairs = tuple(product(range(m), range(n)))
        flattened = tuple(i * n + j for i, j in pairs)
        assert sorted(flattened) == list(range(m * n))
        bijections += 1
    check(
        "C1 Cartesian pair bases biject with tensor bases",
        bijections == len(instances),
    )

    # The canonical basis map is a permutation of an orthonormal basis.  With
    # lexicographic ordering it is literally the identity matrix.
    gram_rows = 0
    for m, n in instances:
        canonical_map = eye(m * n)
        assert canonical_map.T.conjugate() * canonical_map == eye(m * n)
        gram_rows += m * n
    check(
        "C2 the canonical linear extension is exactly unitary",
        gram_rows == sum(m * n for m, n in instances),
    )

    # Write x_i=|ψ_i|^2 and y_j=|φ_j|^2.  This is exact polynomial
    # factorization of the complex squared norm.
    x = symbols("x0:3", nonnegative=True)
    y = symbols("y0:4", nonnegative=True)
    joint_norm = sum(xi * yj for xi in x for yj in y)
    product_norm = sum(x) * sum(y)
    check(
        "C3 product-vector squared norm factors exactly",
        simplify(joint_norm - product_norm) == 0,
    )

    phase_rows = 0
    for m, n in instances:
        weights = tuple(
            tensor_weight(i, j, m, n) for i, j in product(range(m), range(n))
        )
        assert len(weights) == m * n
        assert all(sum(weight) == 2 for weight in weights)
        assert len(set(weights)) == m * n
        phase_rows += len(weights)
    check(
        "C4 independent local phases add on every joint pair character",
        phase_rows == sum(m * n for m, n in instances),
    )

    # Both parenthesizations use the same lexicographic triple label.
    associative_rows = 0
    m, n, p = 2, 3, 4
    for i, j, k in product(range(m), range(n), range(p)):
        left = (i * n + j) * p + k
        right = i * (n * p) + (j * p + k)
        assert left == right
        associative_rows += 1
    check(
        "C5 finite tensor composition is canonically associative",
        associative_rows == m * n * p,
    )

    swap_rows = 0
    m, n = 2, 3
    for i, j in product(range(m), range(n)):
        swapped = j * m + i
        restored_i, restored_j = swapped % m, swapped // m
        assert (restored_i, restored_j) == (i, j)
        swap_rows += 1
    check(
        "C6 the canonical factor swap is an involutive basis bijection",
        swap_rows == m * n,
    )

    # Locality and independent phase actions alone permit a direct sum.  Its
    # local block actions commute exactly.
    ua = Matrix([[0, 1], [1, 0]])
    vb = Matrix([[1, 0, 0], [0, 0, 1], [0, 1, 0]])
    rho_a = ua.diag(eye(3))
    rho_b = eye(2).diag(vb)
    check(
        "C7 direct-sum local actions commute exactly",
        rho_a * rho_b == rho_b * rho_a,
    )

    tensor_weights = {
        tensor_weight(i, j, 2, 2) for i, j in product(range(2), repeat=2)
    }
    sum_weights = set(direct_sum_weights(2, 2))
    check(
        "C8 even at 2+2=2*2 the direct-sum and pair-character types differ",
        len(tensor_weights) == len(sum_weights) == 4
        and tensor_weights.isdisjoint(sum_weights),
    )

    # |00>+|11> has coefficient matrix rank two.  Every simple tensor is an
    # outer product and hence has rank at most one.
    entangled_coefficients = eye(2)
    a0, a1, b0, b1 = symbols("a0 a1 b0 b1")
    product_coefficients = Matrix([[a0 * b0, a0 * b1], [a1 * b0, a1 * b1]])
    check(
        "C9 a rank-two coefficient matrix is not a product vector",
        entangled_coefficients.det() == 1
        and simplify(product_coefficients.det()) == 0,
    )

    # Standard exact saturation fixture for the conditional Hilbert-space
    # Tsirelson corollary.
    pauli_x = Matrix([[0, 1], [1, 0]])
    pauli_z = Matrix([[1, 0], [0, -1]])
    b0_op = (pauli_z + pauli_x) / sqrt(2)
    b1_op = (pauli_z - pauli_x) / sqrt(2)
    chsh = (
        kronecker_product(pauli_z, b0_op + b1_op)
        + kronecker_product(pauli_x, b0_op - b1_op)
    )
    eigenvalues = chsh.eigenvals()
    check(
        "C10 Pauli CHSH operator attains the exact 2*sqrt(2) ceiling",
        simplify(chsh * chsh * (chsh * chsh - 8 * eye(4)))
        == Matrix.zeros(4)
        and eigenvalues.get(2 * sqrt(2), 0) == 1
        and eigenvalues.get(-2 * sqrt(2), 0) == 1,
    )

    passed = sum(condition for _, condition, _ in checks)
    total = len(checks)
    print(f"\nRESULT: {passed}/{total} checks passed")
    if passed != total:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
