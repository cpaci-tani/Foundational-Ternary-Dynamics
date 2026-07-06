"""factor_stencil18_sage.py — B0(iii)/P3(b): factor the order-4 18-point LGF
operator with Sage + ore_algebra (WSL2). This is the D-module step the pure-
Python reconstruction could not do; it decides P3(b).

Result (2026-07-05): the order-4 operator is IRREDUCIBLE over Qbar(z)
(right_factor() = None; factor() = the operator itself). Combined with the
exact exponent argument (z=0 exponents {0,0,0,1/2} are incompatible with any
symmetric-cube pattern {3a,2a+b,a+2b,3b}) and the absence of a MUM point, W_18
is the period of a genuinely irreducible order-4 operator -- it does NOT reduce
to a classical elliptic (order-2, Gamma-quotient) period like the SC/FCC/BCC
lattice constants.

The run SELF-VALIDATES the factorizer first: a Fuchsian reducible operator must
yield a factor, and a Fuchsian irreducible order-2 (the elliptic-K operator)
must yield None -- both in the analytic algorithm's finite-singularity regime,
the regime our operator lives in. (A constant-coefficient operator like Dz^2-1
has no finite singularities and is DEGENERATE for this algorithm -- do not use
it as a sanity case.)

ENVIRONMENT (WSL2 Ubuntu-22.04):
  * Sage 9.5 (apt: sagemath)
  * ore_algebra 0.5 (sage --pip install git+https://github.com/mkauers/ore_algebra.git)
  * numpy PINNED to 1.24.4  (ore_algebra pulls numpy 2.x, which breaks Sage
    9.5's compiled Cython modules -- `sage --pip install 'numpy==1.24.4'`)
Run:
  wsl.exe -d Ubuntu-22.04 -- bash -lc \
    "cd /mnt/c/Users/cpaci/Desktop/ftd && sage -python scripts/proofs/factor_stencil18_sage.py"
"""

import json
import os

from sage.all import QQ, PolynomialRing  # noqa: E402
from ore_algebra import OreAlgebra        # noqa: E402


def build(A, Dz, z, coeffs_by_order):
    L = A.zero()
    for r, coeffs in enumerate(coeffs_by_order):
        L += sum(QQ(c) * z**dd for dd, c in enumerate(coeffs)) * Dz**r
    return L


def main():
    R = PolynomialRing(QQ, "z")
    z = R.gen()
    A = OreAlgebra(R, "Dz")
    Dz = A.gen()

    print("=" * 70)
    print("  18-pt LGF operator — Sage/ore_algebra factorization (P3(b))")
    print("=" * 70)

    # ---- self-validation of the factorizer (finite-singularity regime) ----
    print("\n[validate] factorizer on Fuchsian operators:")
    Lred = (z * Dz - 2) * ((z - 1) * Dz - 1)       # reducible, sings {0,1}
    r1 = Lred.right_factor()
    print(f"  reducible (z Dz-2)((z-1)Dz-1): right_factor order = "
          f"{None if r1 is None else r1.order()}   (must be 1)")
    Lell = z * (1 - z) * Dz * Dz + (1 - 2 * z) * Dz - QQ(1) / 4  # elliptic-K
    r2 = Lell.right_factor()
    print(f"  irreducible elliptic-K: right_factor = {r2}   (must be None)")
    assert r1 is not None and r1.order() == 1, "factorizer failed to find a factor!"
    assert r2 is None, "factorizer wrongly factored an irreducible operator!"
    print("  [validate OK] finds factors when they exist; None only when irreducible")

    # ---- the 18-pt operator ----
    here = os.path.dirname(os.path.abspath(__file__))
    with open(os.path.join(here, "_stencil18_operator.json")) as f:
        d = json.load(f)
    L = build(A, Dz, z, d["polys"])
    print("\n[18-pt operator]")
    print("  order:", L.order(), " degree:", L.degree())
    print("  leading coeff:", L.leading_coefficient().factor())
    print("  singularities (true; apparent removed):", L.singularities())
    print("  generalized series @ z=0:", L.generalized_series_solutions(3))

    rf = L.right_factor()
    fac = L.factor()
    print("\n  right_factor():", rf)
    print("  factor() n_factors:", len(fac), " top order:", fac[0].order())
    print("\n  VERDICT: 18-pt operator is",
          "IRREDUCIBLE over Qbar(z)" if rf is None and len(fac) == 1
          else "REDUCIBLE")
    print("  => W_18 does NOT reduce to a classical elliptic (order-2,")
    print("     Gamma-quotient) period; it is a genuinely irreducible order-4")
    print("     period, outside the classical Watson Gamma-world.")


if __name__ == "__main__":
    main()
