"""
proof_dyadic_lift_desingularizer.py
====================================

Exact symbolic probe for a one-coordinate lift of finite dyadic Fourier curves.

For

    x(t) = a_0 cos(t) + sum_{k=1}^m a_k cos(2^k t),   a_0 != 0,

the carrier (x(t), sin(t)) is a regular embedded circle.  Consequently,

    Gamma(t) = (x(t), y(t), sin(t))

is an embedded, unknotted space curve for every smooth second coordinate y(t).

This is an exact finite trigonometric/topological statement.  It does not run
a numerical search and does not assert an FTD physics claim.
"""

from __future__ import annotations

import sympy as sp


t = sp.symbols("t", real=True)
u = sp.symbols("u", real=True)


def check_half_turn_reflection_identity(max_k: int = 8) -> None:
    """Only the fundamental cosine changes under t -> pi-t."""
    a0 = sp.symbols("a0", nonzero=True, real=True)
    coeffs = sp.symbols(f"a1:{max_k + 1}", real=True)
    x = a0 * sp.cos(t) + sum(coeffs[k - 1] * sp.cos(2**k * t) for k in range(1, max_k + 1))
    reflected_difference = sp.trigsimp(x.subs(t, sp.pi - t) - x)
    assert sp.simplify(reflected_difference + 2 * a0 * sp.cos(t)) == 0


def check_seed_carrier_parity() -> None:
    """Recover the seed's exact odd part X(u)-X(-u)=2u."""
    coeffs = [sp.Rational(1), sp.Rational(1, 2), sp.Rational(1, 2), sp.Rational(3, 8)]
    X = sum(coeffs[k] * sp.chebyshevt(2**k, u) for k in range(len(coeffs)))
    expected = 2 * u
    assert sp.expand(X - X.subs(u, -u) - expected) == 0


def check_quadrature_regularness(max_k: int = 8) -> None:
    """At sin(t)=+/-1, the x derivative is nonzero when a0 is nonzero."""
    a0 = sp.symbols("a0", nonzero=True, real=True)
    coeffs = sp.symbols(f"a1:{max_k + 1}", real=True)
    x = a0 * sp.cos(t) + sum(coeffs[k - 1] * sp.cos(2**k * t) for k in range(1, max_k + 1))
    dx = sp.diff(x, t)
    assert sp.simplify(dx.subs(t, sp.pi / 2) + a0) == 0
    assert sp.simplify(dx.subs(t, 3 * sp.pi / 2) - a0) == 0


def check_seed_lift_specialization() -> None:
    """Check the C3 carrier/lift formula directly from its Fourier seed."""
    x = (
        sp.cos(t)
        + sp.Rational(1, 2) * sp.cos(2 * t)
        + sp.Rational(1, 2) * sp.cos(4 * t)
        + sp.Rational(3, 8) * sp.cos(8 * t)
    )
    z = sp.sin(t)

    assert sp.trigsimp(x.subs(t, sp.pi - t) - x + 2 * sp.cos(t)) == 0
    assert sp.trigsimp(z.subs(t, sp.pi - t) - z) == 0
    assert sp.simplify(sp.diff(x, t).subs(t, sp.pi / 2) + 1) == 0
    assert sp.simplify(sp.diff(x, t).subs(t, 3 * sp.pi / 2) - 1) == 0


def main() -> None:
    checks = [
        ("dyadic half-turn reflection identity", check_half_turn_reflection_identity),
        ("C3 carrier odd-part identity", check_seed_carrier_parity),
        ("quadrature regularness", check_quadrature_regularness),
        ("C3 lift specialization", check_seed_lift_specialization),
    ]

    print("Dyadic lift desingularizer probe")
    print("=" * 60)
    for name, fn in checks:
        fn()
        print(f"PASS - {name}")
    print("=" * 60)
    print("OK - z=sin(t) restores a regular embedded carrier when a0 is nonzero.")
    print("No FTD physics claim promoted.")


if __name__ == "__main__":
    main()
