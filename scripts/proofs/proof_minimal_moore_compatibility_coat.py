"""Independent exact proof checks for FTD-0577.

This script performs symbolic/rational verification only.  It does not search
for filters, constants, near-misses, or physical matches.
"""

from fractions import Fraction

import sympy as sp


def require(label: str, condition: bool) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS {label}")


def main() -> None:
    z, zx, zy, zz, k = sp.symbols("z zx zy zz k", nonzero=True)
    a, b = sp.symbols("a b")

    solution = sp.solve((sp.Eq(2 * a + b, 1), sp.Eq(-2 * a + b, 0)),
                        (a, b), dict=True)
    require("unique symmetric radius-one solution",
            solution == [{a: sp.Rational(1, 4), b: sp.Rational(1, 2)}])
    B = sp.Rational(1, 4) / z + sp.Rational(1, 2) + sp.Rational(1, 4) * z
    require("checkerboard factorization",
            sp.simplify(B - (z + 1) ** 2 / (4 * z)) == 0)

    dc = (z - 1 / z) / 2
    df = 1 - 1 / z
    A = (1 + 1 / z) / 2
    require("local face-to-central factorization",
            sp.simplify(dc * A - B * df) == 0)

    def b_axis(symbol: sp.Symbol) -> sp.Expr:
        return (symbol + 1) ** 2 / (4 * symbol)

    BM = b_axis(zx) * b_axis(zy) * b_axis(zz)
    for axis, symbol in enumerate((zx, zy, zz)):
        central = (symbol - 1 / symbol) / 2
        face = 1 - 1 / symbol
        bridge = (1 + 1 / symbol) / 2
        transverse = sp.prod(b_axis(other)
                             for other in (zx, zy, zz) if other != symbol)
        require(f"3D component factorization axis {axis}",
                sp.simplify(central * bridge * transverse
                            - BM * face) == 0)

    weights: dict[tuple[int, int, int], Fraction] = {}
    one_d = {-1: Fraction(1, 4), 0: Fraction(1, 2), 1: Fraction(1, 4)}
    for dx in (-1, 0, 1):
        for dy in (-1, 0, 1):
            for dz in (-1, 0, 1):
                weights[(dx, dy, dz)] = one_d[dx] * one_d[dy] * one_d[dz]
    require("27 positive Moore weights", len(weights) == 27
            and all(value > 0 for value in weights.values()))
    require("exact partition", sum(weights.values()) == 1)
    require("exact zero first moment",
            all(sum(Fraction(site[axis]) * value
                    for site, value in weights.items()) == 0
                for axis in range(3)))
    shell = {0: Fraction(0), 1: Fraction(0), 2: Fraction(0), 3: Fraction(0)}
    for site, value in weights.items():
        shell[sum(component != 0 for component in site)] += value
    require("Moore shell totals",
            shell == {0: Fraction(1, 8), 1: Fraction(3, 8),
                      2: Fraction(3, 8), 3: Fraction(1, 8)})

    # Exact periodic 1D replay with an arbitrary rational face current.
    L = 17
    face_current = [Fraction((7 * i * i + 3 * i + 2) % 19 - 9, 11)
                    for i in range(L)]
    delta_rho = [-(face_current[i] - face_current[(i - 1) % L])
                 for i in range(L)]

    def smooth(values: list[Fraction]) -> list[Fraction]:
        return [Fraction(1, 4) * values[(i - 1) % L]
                + Fraction(1, 2) * values[i]
                + Fraction(1, 4) * values[(i + 1) % L]
                for i in range(L)]

    coated_delta = smooth(delta_rho)
    central_current = [Fraction(1, 2)
                       * (face_current[i] + face_current[(i - 1) % L])
                       for i in range(L)]
    central_divergence = [Fraction(1, 2)
                          * (central_current[(i + 1) % L]
                             - central_current[(i - 1) % L])
                          for i in range(L)]
    require("exact rational periodic continuity",
            all(coated_delta[i] + central_divergence[i] == 0
                for i in range(L)))

    trig_form = sp.cos(k / 2) ** 2
    require("Laurent and trigonometric symbols agree",
            sp.simplify(B.subs(z, sp.exp(sp.I * k)) - trig_form) == 0)
    series = sp.series(sp.prod(sp.cos(symbol / 2) ** 2
                               for symbol in (zx, zy, zz)),
                       zx, 0, 3).removeO()
    series = sp.series(series, zy, 0, 3).removeO()
    series = sp.series(series, zz, 0, 3).removeO()
    quadratic = 1 - (zx ** 2 + zy ** 2 + zz ** 2) / 4
    require("infrared form factor begins at one with isotropic quadratic term",
            sp.Poly(sp.expand(series - quadratic), zx, zy, zz).terms()
            and all(sum(monomial) >= 4
                    for monomial, _ in sp.Poly(
                        sp.expand(series - quadratic), zx, zy, zz).terms()))

    # Along a principal axis the FTD-0575 static kernel stays finite.  Coating
    # source and probe multiplies it by B^2 and cannot manufacture 1/k^2.
    native_static = 3 * sp.sin(k) ** 2 / (2 * (1 - sp.cos(k)))
    coated_static = sp.simplify(native_static * sp.cos(k / 2) ** 4)
    require("coated static response still has no Coulomb pole",
            sp.limit(coated_static, k, 0) == 3)
    require("coat is non-cardinal at integer center",
            weights[(0, 0, 0)] == Fraction(1, 8))

    print("verdict="
          "MINIMAL_MOORE_COAT_RESTORES_LOCAL_CENTRAL_CONTINUITY_"
          "NONCARDINAL_SELECTED")


if __name__ == "__main__":
    main()
