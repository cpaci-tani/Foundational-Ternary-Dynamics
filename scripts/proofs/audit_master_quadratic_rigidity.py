#!/usr/bin/env python3
"""
audit_master_quadratic_rigidity.py

Phase I audit: how unique is the master quadratic x^2 - 16 G*^2 x + 16 G*^3 = 0?

Three independent rigidity tests:

  (1) Coefficient scan. Try x^2 - a*G*^p x + b*G*^q for integer a, b in [1, 32]
      and p, q in [1, 4]. How many such quadratics have a positive root within
      1 ppm, 10 ppm, 100 ppm, or 1000 ppm of 137.036?

  (2) G* neighborhood test. The project's G* = 2.958675... is the lemniscatic
      constant (canonical for curve y^2 = x^3 - x). If we perturb G* by small
      amounts, does x+(G*) stay near 137 or drift?

  (3) Alternative-G search. Are there OTHER "nice" constants G' (combinations
      of pi, e, sqrt(n), gamma(p/q)) such that the master-quadratic-shaped
      polynomial x^2 - 16 G'^2 x + 16 G'^3 = 0 also gives a root near 137.036?

Output: a clean report tallying how "locked" the master quadratic actually
is to the fine-structure constant.
"""
from __future__ import annotations
import math
from mpmath import mp, mpf, gamma, sqrt, pi, exp, log

mp.dps = 30  # 30 digits

ALPHA_INV = mpf("137.035999177")  # CODATA 2022
G_STAR = gamma(mpf(1)/4) / gamma(mpf(3)/4)
# Equivalently 2 * Omega / sqrt(pi), as the project uses
assert abs(G_STAR - mpf("2.9586751191886388923108213577277195664748981")) < mpf("1e-20")


def positive_root(a: mpf, b: mpf) -> mpf | None:
    """x^2 - a x + b = 0; return larger root if real, else None."""
    disc = a*a - 4*b
    if disc < 0:
        return None
    return (a + sqrt(disc)) / 2


def rel_err(x: mpf, target: mpf) -> float:
    return float(abs(x - target) / target)


def test_1_coefficient_scan() -> None:
    """Scan (a, b, p, q) for integer coefficients and powers. How many
    near-137 hits exist?"""
    print("=" * 78)
    print("  TEST 1 — coefficient scan")
    print("  Polynomial form: x^2 - a * G*^p * x + b * G*^q = 0")
    print("  a, b in [1, 64], p, q in [1, 4]")
    print("  Tally hits by precision of the larger root vs 1/alpha = 137.0360.")
    print("=" * 78)

    tallies = {1e-6: [], 1e-5: [], 1e-4: [], 1e-3: [], 1e-2: []}
    total = 0
    for a in range(1, 65):
        for b in range(1, 65):
            for p in range(1, 5):
                for q in range(1, 5):
                    A = a * G_STAR**p
                    B = b * G_STAR**q
                    x = positive_root(A, B)
                    if x is None or x <= 0:
                        continue
                    total += 1
                    err = rel_err(x, ALPHA_INV)
                    for cutoff in tallies:
                        if err < cutoff:
                            tallies[cutoff].append((a, b, p, q, x, err))

    print(f"\n  Total valid polynomials scanned: {total}")
    for cutoff in sorted(tallies):
        hits = tallies[cutoff]
        label = {1e-6: "1 ppm", 1e-5: "10 ppm",
                 1e-4: "100 ppm", 1e-3: "1000 ppm", 1e-2: "1%"}[cutoff]
        print(f"\n  Within {label}: {len(hits)} hits")
        for (a, b, p, q, x, err) in hits[:10]:
            marker = "  <-- MASTER" if (a, b, p, q) == (16, 16, 2, 3) else ""
            print(f"    a={a:3d} b={b:3d} p={p} q={q}  x+ = {float(x):.5f}  "
                  f"rel_err = {err:.2e}{marker}")
        if len(hits) > 10:
            print(f"    ... ({len(hits) - 10} more)")


def test_2_gstar_sensitivity() -> None:
    """Perturb G* and see how x+ shifts. If x+ is 'locked' at 137 this
    should drift quickly; if it's a shallow fit, it stays near 137."""
    print("\n" + "=" * 78)
    print("  TEST 2 — G* sensitivity")
    print("  Replace G* with G* * (1 + delta) for small delta, recompute x+.")
    print("  Shows whether 137 is a FIXED POINT at G* or merely nearby.")
    print("=" * 78)
    print(f"\n  {'delta':>12} {'G_perturb':>14} {'x+':>14} {'rel_err(alpha)':>16}")
    for delta in [-1e-3, -1e-4, -1e-5, -1e-6, 0.0, 1e-6, 1e-5, 1e-4, 1e-3]:
        Gp = G_STAR * (1 + mpf(delta))
        A = 16 * Gp**2
        B = 16 * Gp**3
        x = positive_root(A, B)
        err = rel_err(x, ALPHA_INV)
        print(f"  {delta:>12.1e}  {float(Gp):>14.10f}  {float(x):>14.6f}  "
              f"{err:>16.3e}")
    # Derivative dx+/dG* (at delta=0)
    h = mpf("1e-20")
    A0, B0 = 16*G_STAR**2, 16*G_STAR**3
    Gp = G_STAR + h
    Ap, Bp = 16*Gp**2, 16*Gp**3
    x0 = positive_root(A0, B0)
    xp = positive_root(Ap, Bp)
    dxdG = (xp - x0) / h
    print(f"\n  dx+/dG*   (numerical) = {float(dxdG):.4f}")
    print(f"  x+/G*                = {float(x0 / G_STAR):.4f}")
    print(f"  Log derivative        = {float(dxdG * G_STAR / x0):.4f}  "
          f"(ratio of fractional shifts)")


def test_3_alternative_constants() -> None:
    """Try other 'nice' constants in place of G*. Do any yield root near 137?"""
    print("\n" + "=" * 78)
    print("  TEST 3 — alternative constants in the master-quadratic shape")
    print("  Polynomial x^2 - 16 C^2 x + 16 C^3 = 0 with different C's.")
    print("  If only C = G* hits 137, identification is strong.")
    print("=" * 78)

    candidates = [
        ("G* = Gamma(1/4)/Gamma(3/4)", G_STAR),
        ("pi", pi),
        ("e",  exp(mpf(1))),
        ("sqrt(2)+sqrt(3)", sqrt(2)+sqrt(3)),
        ("Gamma(1/3)", gamma(mpf(1)/3)),
        ("sqrt(pi)", sqrt(pi)),
        ("Euler gamma + 2.5", mpf("0.5772156649") + mpf("2.5")),
        ("phi (golden)", (1+sqrt(5))/2),
        ("ln(100)", log(100)),
        ("2*sqrt(2.2)", 2*sqrt(mpf("2.2"))),
        ("137^(1/3)/2.5", (mpf(137))**mpf("1/3") / mpf("2.5")),
    ]

    print(f"\n  {'constant':>30} {'value':>14} {'x+':>14} {'rel_err':>14}")
    for name, C in candidates:
        A = 16 * C**2
        B = 16 * C**3
        x = positive_root(A, B)
        if x is None:
            print(f"  {name:>30} {float(C):>14.6f} {'(complex)':>14} {'-':>14}")
            continue
        err = rel_err(x, ALPHA_INV)
        marker = "  <-- CANONICAL" if name.startswith("G* ") else ""
        print(f"  {name:>30} {float(C):>14.6f} {float(x):>14.4f} "
              f"{err:>14.3e}{marker}")


def test_4_closest_simple_target() -> None:
    """What's the simplest integer (a,b) quadratic that beats the master quadratic?
    This inverts the question: how hard would it be to 'fit' 137 with a degree-2
    polynomial?"""
    print("\n" + "=" * 78)
    print("  TEST 4 — integer (a, b) with no G* involvement")
    print("  Simplest alternative: can x^2 - a x + b = 0 hit 137 for small integers?")
    print("=" * 78)

    # Target: roots 137 and 3 gives (x - 137)(x - 3) = x^2 - 140 x + 411.
    # 16 * G*^2 ~= 140.060, 16 * G*^3 ~= 414.392. So the master quadratic's
    # coefficients are close to but not exactly (140, 411).
    print(f"\n  Target (roots 1/alpha, 3): x^2 - 140.036 x + 411.108 = 0")
    print(f"  Master quadratic:           x^2 - {float(16*G_STAR**2):.3f} x + {float(16*G_STAR**3):.3f} = 0")
    print(f"  Naive integer:              x^2 - 140 x + 411 = 0")
    print()
    hits = []
    for a in range(100, 200):
        for b in range(300, 500):
            x = positive_root(mpf(a), mpf(b))
            if x is None:
                continue
            err = rel_err(x, ALPHA_INV)
            if err < 1e-3:
                hits.append((a, b, float(x), err))
    hits.sort(key=lambda t: t[3])
    print(f"  Integer (a, b) with both coeffs small and root within 1000 ppm:")
    print(f"  {'a':>6} {'b':>6} {'x+':>14} {'rel_err':>14}")
    for a, b, x, err in hits[:15]:
        print(f"  {a:>6} {b:>6} {x:>14.6f} {err:>14.3e}")


def main() -> None:
    print()
    print(f"  alpha_inv (CODATA 2022) = {float(ALPHA_INV):.9f}")
    print(f"  G*                      = {float(G_STAR):.9f}")
    print(f"  master quadratic: x^2 - 16 G*^2 x + 16 G*^3 = 0")
    print(f"  x+                      = {float(positive_root(16*G_STAR**2, 16*G_STAR**3)):.9f}")
    print(f"  rel_err vs 1/alpha      = {rel_err(positive_root(16*G_STAR**2, 16*G_STAR**3), ALPHA_INV):.3e}")

    test_1_coefficient_scan()
    test_2_gstar_sensitivity()
    test_3_alternative_constants()
    test_4_closest_simple_target()


if __name__ == "__main__":
    main()
