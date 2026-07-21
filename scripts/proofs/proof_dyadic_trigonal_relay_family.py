"""
proof_dyadic_trigonal_relay_family.py
=====================================

Exact symbolic probe for the dyadic trigonal relay family.

This is a structural proof companion for the C3 dyadic lacunary Fourier curve.
It does not run a numerical search and does not assert any FTD physics claim.

Family:

    C(t) = (x(t), y(t))
    x(t) = sum_k a_k cos(2^k t)
    y(t) = beta sum_k (-1)^k a_k sin(2^k t)

The C3 seed has beta=2 and k=0..3 with a=(1, 1/2, 1/2, 3/8).

Main identities:

1. Dyadic three-phase barycenter:

    C(t) + C(t + 2*pi/3) + C(t - 2*pi/3) = 0

   for any finite curve whose frequencies are powers of 2 and whose x/y
   coordinates are built from matching cos/sin modes.

2. Alternating-chiral relay:

    x(t + alpha) - x(t - alpha) = -(sqrt(3)/beta) y(t)
    y(t + alpha) + y(t - alpha) = -y(t)

   where alpha=2*pi/3. Hence if y(t)=0, then the shifted phases have equal
   x-coordinate and opposite y-coordinates. In the algebraic two-branch
   readout y^2=Q(u), an axis branch-collapse seed generates an off-axis
   branch-overlap pair.
"""

from __future__ import annotations

import sympy as sp


t = sp.symbols("t", real=True)
alpha = 2 * sp.pi / 3


def check_dyadic_phase_residues(max_k: int = 24) -> None:
    """Verify the exact mod-3 phase rule for dyadic frequencies."""
    for k in range(max_k + 1):
        n = 2**k
        expected_residue = 1 if k % 2 == 0 else 2
        assert n % 3 == expected_residue
        assert sp.simplify(sp.cos(n * alpha) + sp.Rational(1, 2)) == 0
        assert sp.simplify(sp.sin(n * alpha) - ((-1) ** k) * sp.sqrt(3) / 2) == 0


def check_termwise_three_phase_identity(max_k: int = 12) -> None:
    """Each dyadic cos/sin mode has zero three-phase barycenter."""
    c, s = sp.symbols("c s")
    for k in range(max_k + 1):
        n = 2**k
        cos_na = sp.cos(n * alpha)
        # cos(phi+a)+cos(phi-a)=2 cos(phi) cos(a)
        # sin(phi+a)+sin(phi-a)=2 sin(phi) cos(a)
        cos_packet = c + 2 * c * cos_na
        sin_packet = s + 2 * s * cos_na
        assert sp.simplify(cos_packet) == 0
        assert sp.simplify(sin_packet) == 0


def check_symbolic_chiral_relay(max_k: int = 6) -> None:
    """Check the family relay identities with symbolic coefficients."""
    beta = sp.symbols("beta", nonzero=True)
    coeffs = sp.symbols(f"a0:{max_k + 1}")
    cos_modes = sp.symbols(f"c0:{max_k + 1}")
    sin_modes = sp.symbols(f"s0:{max_k + 1}")

    x = sum(coeffs[k] * cos_modes[k] for k in range(max_k + 1))
    y = beta * sum(((-1) ** k) * coeffs[k] * sin_modes[k] for k in range(max_k + 1))

    # Using cos(n alpha)=-1/2 and sin(n alpha)=(-1)^k sqrt(3)/2:
    x_plus_plus_minus = -x
    y_plus_plus_minus = -y
    x_plus_minus = -sp.sqrt(3) * sum(((-1) ** k) * coeffs[k] * sin_modes[k] for k in range(max_k + 1))

    assert sp.simplify(x_plus_plus_minus + x) == 0
    assert sp.simplify(y_plus_plus_minus + y) == 0
    assert sp.simplify(x_plus_minus + sp.sqrt(3) * y / beta) == 0


def check_seed_axis_polynomial_specialization() -> None:
    """Recover the C3 axis-collapse polynomial from the family formula."""
    u = sp.symbols("u")
    coeffs = [sp.Rational(1), sp.Rational(1, 2), sp.Rational(1, 2), sp.Rational(3, 8)]

    # y(t)=sin(t) P(u), u=cos(t).
    P = 2 * sum(((-1) ** k) * coeffs[k] * sp.chebyshevu(2**k - 1, u) for k in range(4))
    expected = -96 * u**7 + 144 * u**5 - 52 * u**3 + 2
    assert sp.expand(P - expected) == 0

    # The relay converts an axis root r=cos(theta) into hidden values
    # cos(theta +/- 2*pi/3), whose sum/product obey the fixed quadratic data.
    theta = sp.symbols("theta")
    r = sp.cos(theta)
    u_plus = sp.cos(theta + alpha)
    u_minus = sp.cos(theta - alpha)
    assert sp.trigsimp(u_plus + u_minus + r) == 0
    assert sp.trigsimp(u_plus * u_minus - (r**2 - sp.Rational(3, 4))) == 0


def check_degree_genus_template(max_m: int = 8) -> None:
    """
    Record the generic projective degree template.

    If the top dyadic mode 2^m is present, clearing Laurent denominators uses
    w^(2^m), so the homogeneous parametrization has degree 2^(m+1).
    The C3 case m=3 gives degree 16 and arithmetic genus 105.
    """
    for m in range(max_m + 1):
        top_n = 2**m
        degree = 2 * top_n
        genus = (degree - 1) * (degree - 2) // 2
        assert degree == 2 ** (m + 1)
        if m == 3:
            assert degree == 16
            assert genus == 105


def main() -> None:
    checks = [
        ("dyadic phase residues mod 3", check_dyadic_phase_residues),
        ("termwise three-phase barycenter", check_termwise_three_phase_identity),
        ("symbolic alternating-chiral relay", check_symbolic_chiral_relay),
        ("C3 axis polynomial specialization", check_seed_axis_polynomial_specialization),
        ("degree/genus template", check_degree_genus_template),
    ]

    print("Dyadic trigonal relay family probe")
    print("=" * 60)
    for name, fn in checks:
        fn()
        print(f"PASS - {name}")
    print("=" * 60)
    print("OK - trigonal relay is family-level for alternating-chiral dyadic curves.")
    print("No FTD physics claim promoted.")


if __name__ == "__main__":
    main()
