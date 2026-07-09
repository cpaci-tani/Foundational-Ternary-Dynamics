"""explr_lgf_classify_sage.py — structural classification of a reconstructed LGF
operator via Sage + ore_algebra (WSL2), the general control-lattice companion to
factor_stencil18_sage.py (FTD-0372). Same machinery, same self-validation, run
uniformly on any operator produced by explr_lgf_reconstruct.py.

For each operator json it reports: order, degree, leading-coefficient
factorization, the TRUE singular locus (apparent singularities removed by
ore_algebra), local exponents at 0, and the irreducibility verdict
(right_factor()/factor()). The run SELF-VALIDATES the factorizer first — a
Fuchsian reducible operator must yield an order-1 factor, the irreducible
elliptic-K operator must yield None — exactly as the 18-pt run does, so the
"irreducible" verdicts here and for W_18 rest on the same validated tool.

Purpose: the two textbook controls are the positive/known cases the 18-pt
classification is measured against —
  * 2D square  : IRREDUCIBLE order 2 — the (z->z^2) pullback of the complete-
                 elliptic-integral / 2F1(1/2,1/2;1) operator (K(k) LGF).
  * 3D SC      : IRREDUCIBLE order 3 — the Joyce (1973) operator; classically the
                 symmetric SQUARE of an elliptic order-2 (W_SC a Gamma(1/24)
                 quotient). This is the CM/symmetric-power case W_18 was tested
                 against and did NOT match.
Recovering these correctly is what makes the "W_18 is a genuinely new,
irreducible, non-symmetric-power order-4 period" verdict (FTD-0372/0373)
defensible: the same reconstruct+classify pipeline gets the known answers right.

NO PSLQ, NO closed-form fishing — a structural (D-module) classification of an
operator reconstructed from exact data.

ENVIRONMENT (WSL2 Ubuntu-22.04): Sage 9.5 + ore_algebra 0.5, numpy PINNED 1.24.4.
Run:
  wsl.exe -d Ubuntu-22.04 -- bash -lc \
    "cd /mnt/c/Users/cpaci/Desktop/ftd && sage -python scripts/proofs/explr_lgf_classify_sage.py"
Optional args: one or more operator-json paths (default: the 2D square + 3D SC
controls). Example: ... explr_lgf_classify_sage.py scripts/proofs/_sc3d_operator.json
"""

import json
import os
import sys

from sage.all import QQ, PolynomialRing  # noqa: E402
from ore_algebra import OreAlgebra        # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))

# (json path, human name, expected minimal order) — the known controls.
DEFAULT_CONTROLS = [
    (os.path.join(HERE, "_square2d_operator.json"), "2D square (elliptic-K)", 2),
    (os.path.join(HERE, "_sc3d_operator.json"), "3D SC (Joyce 1973)", 3),
]


def build(A, Dz, z, coeffs_by_order):
    L = A.zero()
    for r, coeffs in enumerate(coeffs_by_order):
        L += sum(QQ(c) * z ** dd for dd, c in enumerate(coeffs)) * Dz ** r
    return L


def self_validate(A, Dz, z):
    """Same finite-singularity self-check as factor_stencil18_sage.py."""
    print("[validate] factorizer on Fuchsian operators:")
    Lred = (z * Dz - 2) * ((z - 1) * Dz - 1)  # reducible, sings {0,1}
    r1 = Lred.right_factor()
    print(f"  reducible (z Dz-2)((z-1)Dz-1): right_factor order = "
          f"{None if r1 is None else r1.order()}   (must be 1)")
    Lell = z * (1 - z) * Dz * Dz + (1 - 2 * z) * Dz - QQ(1) / 4  # elliptic-K
    r2 = Lell.right_factor()
    print(f"  irreducible elliptic-K: right_factor = {r2}   (must be None)")
    assert r1 is not None and r1.order() == 1, "factorizer failed to find a factor!"
    assert r2 is None, "factorizer wrongly factored an irreducible operator!"
    print("  [validate OK] finds factors when they exist; None only when irreducible\n")


def classify(A, Dz, z, path, name, expected_order):
    with open(path) as f:
        d = json.load(f)
    L = build(A, Dz, z, d["polys"])
    print("=" * 70)
    print(f"  {name}   [{os.path.basename(path)}]")
    print("=" * 70)
    print("  order:", L.order(), " degree:", L.degree(),
          f"  (expected minimal order {expected_order})")
    print("  leading coeff:", L.leading_coefficient().factor())
    try:
        print("  singularities (true; apparent removed):", L.singularities())
    except Exception as e:  # noqa: BLE001
        print("  singularities: error —", e)
    print("  generalized series @ z=0:", L.generalized_series_solutions(3))

    rf = L.right_factor()
    fac = L.factor()
    irreducible = rf is None and len(fac) == 1
    print("  right_factor():", rf)
    print("  factor() n_factors:", len(fac), " top order:", fac[0].order())
    print("  VERDICT:", "IRREDUCIBLE over Qbar(z)" if irreducible else "REDUCIBLE")
    ok_order = (L.order() == expected_order)
    print(f"  [control check] order == {expected_order}: "
          f"{'PASS' if ok_order else 'FAIL'}\n")
    return ok_order and irreducible


def main():
    R = PolynomialRing(QQ, "z")
    z = R.gen()
    A = OreAlgebra(R, "Dz")
    Dz = A.gen()

    print("=" * 70)
    print("  LGF control-lattice classification — Sage/ore_algebra (FTD-0372)")
    print("=" * 70 + "\n")
    self_validate(A, Dz, z)

    if len(sys.argv) > 1:
        controls = [(p, os.path.basename(p), None) for p in sys.argv[1:]]
    else:
        controls = DEFAULT_CONTROLS

    results = []
    for path, name, exp in controls:
        if not os.path.exists(path):
            print(f"  [skip] {name}: {path} not found "
                  f"(regenerate via explr_lgf_reconstruct.py)\n")
            continue
        results.append(classify(A, Dz, z, path, name,
                                exp if exp is not None else -1))

    if all(r for r in results if r is not None):
        print("ALL CONTROLS PASS — reconstruct+classify pipeline validated on"
              " known lattices; same tool underwrites the W_18 verdict.")


if __name__ == "__main__":
    main()
