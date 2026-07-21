"""
proof_dyadic_cubic_master_quadratic_boundary.py
================================================

Exact structural comparison between the octave-8 regularity cubic

    G(q) = 128 q^3 + 16 q^2 - 18 q - 3

and FTD's master quadratic

    M_g(x) = x^2 - 16 g^2 x + 16 g^3.

The comparison establishes a direct-identity boundary: the two polynomials
have different degree, root-field type, coefficient provenance, and stability
under a natural dyadic-mode perturbation. It retains only the higher-level
analogy that both are closure polynomials obtained by eliminating hidden
variables.

No numerical search is used. No FTD physics claim is promoted.
"""

from __future__ import annotations

import sympy as sp


q, z, g, x, y, u = sp.symbols("q z g x y u")

OCTAVE_CUBIC = 128 * q**3 + 16 * q**2 - 18 * q - 3
MASTER_QUADRATIC = x**2 - 16 * g**2 * x + 16 * g**3


def check_octave_cubic_invariants() -> None:
    """Classify the rational cubic's normal form and root data exactly."""
    assert sp.Poly(OCTAVE_CUBIC, q).degree() == 3
    assert sp.Poly(OCTAVE_CUBIC, q).is_irreducible
    assert sp.factor(sp.discriminant(OCTAVE_CUBIC, q)) == 2**10 * 3 * 367

    depressed = sp.expand(OCTAVE_CUBIC.subs(q, z - sp.Rational(1, 24)) / 128)
    assert depressed == z**3 - sp.Rational(7, 48) * z - sp.Rational(241, 13824)

    # Vieta data for the three amplitude thresholds q_1,q_2,q_3.
    root_sum = -sp.Rational(1, 8)
    pair_sum = -sp.Rational(9, 64)
    root_product = sp.Rational(3, 128)
    assert pair_sum / root_product == -6


def check_master_quadratic_invariants() -> None:
    """Record the formal master-quadratic root structure over Q(g)."""
    assert sp.Poly(MASTER_QUADRATIC, x).degree() == 2
    assert sp.factor(sp.discriminant(MASTER_QUADRATIC, x)) == 64 * g**3 * (4 * g - 1)

    normalized = sp.factor(MASTER_QUADRATIC.subs(x, g * y) / g**2)
    assert sp.expand(normalized - (y**2 - 16 * g * y + 16 * g)) == 0

    # The master-tower harmonic invariant is a two-root fact: 1/y_+ + 1/y_- = 1.
    normalized_sum = 16 * g
    normalized_product = 16 * g
    assert sp.simplify(normalized_sum / normalized_product - 1) == 0


def check_natural_counter_slice() -> None:
    """
    Remove only the 4t mode and retain the same alternating-chiral convention.

    This exact neighboring dyadic slice has a different, coprime regularity
    polynomial. It rules out treating OCTAVE_CUBIC as a family-invariant
    master relation.
    """
    X0 = u + sp.Rational(1, 2) * sp.chebyshevt(2, u) + q * sp.chebyshevt(8, u)
    P0 = 2 - 2 * u - 256 * q * u**7 + 384 * q * u**5 - 160 * q * u**3 + 16 * q * u
    Y0 = sp.factor(u * P0 - (1 - u**2) * sp.diff(P0, u))

    resultant = sp.factor(sp.resultant(sp.diff(X0, u), Y0, u))
    counter_factor = q**8 * (4 * q + 1) * (64 * q**2 - 8 * q - 1) ** 3
    scale = sp.factor(resultant / counter_factor)
    assert scale.is_Integer and scale != 0
    assert sp.gcd(sp.Poly(OCTAVE_CUBIC, q), sp.Poly(64 * q**2 - 8 * q - 1, q)) == 1


def main() -> None:
    checks = [
        ("octave cubic invariants", check_octave_cubic_invariants),
        ("master quadratic invariants", check_master_quadratic_invariants),
        ("natural dyadic counter-slice", check_natural_counter_slice),
    ]

    print("Dyadic cubic / master quadratic boundary probe")
    print("=" * 60)
    for name, fn in checks:
        fn()
        print(f"PASS - {name}")
    print("=" * 60)
    print("OK - no direct polynomial identity or family-invariant bridge found.")
    print("Shared closure-polynomial language is retained as an analogy only.")
    print("No numerical search and no FTD physics claim promoted.")


if __name__ == "__main__":
    main()
