"""explr_stencil18_selfduality_derived.py — settle W_18 self-duality FROM THE
OPERATOR (FTD-0373).

The rigid-Calabi-Yau / weight-4-newform / Sym^2 / Sym^3 / K3 identifications of
the FTD-0372 order-4 lattice Green's-function operator W_18 ALL require its
4-dimensional monodromy local system to be a SELF-DUAL polarized variation of
Hodge structure of some pure weight -- i.e. to carry a nondegenerate
monodromy-invariant bilinear form (symplectic -> rigid-CY H^3 / Sym^3-elliptic;
orthogonal -> K3/Sym^2 H^2).

DECISIVE necessary condition (per-point, gauge-invariant). A polarized VHS of
weight w carries a flat pairing V (x) V -> Q(-w); at EACH regular singular point
this forces the local exponents to be symmetric about THEIR OWN center (the
center may differ from point to point -- e.g. the mirror quintic is symmetric
about 0 at its MUM point, 1 at the conifold, 1/2 at infinity). For a 4-element
multiset, "symmetric about some center" is, sorted, exactly  a+d == b+c.
Failure at even ONE genuine singular point => NO invariant form => NOT self-dual,
conclusively. (A single GLOBAL weight is NOT required and is NOT the test -- a
genuine CY operator fails a single-global-weight test, so we do not rest on it.)

This script:
  (1) loads the exact operator L = sum_j p_j(z) D^j from _stencil18_operator.json;
  (2) DERIVES the indicial exponents at every rational singular point and at
      infinity by the theta-operator (Frobenius) method -- no hardcoded exponents;
  (3) VALIDATES the derivation on a hypergeometric operator with KNOWN exponents;
  (4) VALIDATES the per-point predicate on KNOWN self-dual controls, including the
      mirror-quintic order-4 operator whose three points have DIFFERENT centers
      (0, 1, 1/2) -- proving the test accepts point-varying self-duality;
  (5) applies the per-point test to W_18's corpus-established genuine finite locus
      {z=1,-2,-3} (EXPLR_STENCIL_SPECTRUM.md / LEDGER FTD-0372).

Verdict: at z=1,-2,-3 the multiset {0,1/2,1,2} is symmetric about NO center
(0+2 != 1/2+1), so W_18 carries no invariant bilinear form of either type and is
NOT self-dual. Hence it is NOT a rigid-CY period, NOT Sym^2/Sym^3 of an elliptic
curve, NOT a K3 transcendental piece, and its L-function is NOT a weight-4
newform L-function (Mazur-van Straten-Yui / Gouvea-Yui). FTD-0373; the rigid-CY /
weight-4 branch of the FTD-0372 residual is CLOSED NEGATIVE.

NO PSLQ / NO closed-form fishing: exact-arithmetic structural computation on the
operator's own coefficients. Tag: [THEOREM -- exact] (given the FTD-0372 operator
[NUMERICAL FACT]).
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

import sympy as sp

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

z, w, th = sp.symbols("z w theta")
R = sp.Rational

# The corpus-established genuine finite singular locus of the W_18 period
# (EXPLR_STENCIL_SPECTRUM.md §2 / LEDGER FTD-0372): z in {0, 1, -2, -3, inf};
# the decisive finite points carrying genuine monodromy are these:
GENUINE_FINITE = [sp.Integer(1), sp.Integer(-2), sp.Integer(-3)]


def ff(t, j):
    """Falling factorial t(t-1)...(t-j+1); ff(t,0)=1."""
    out = sp.Integer(1)
    for i in range(j):
        out *= (t - i)
    return sp.expand(out)


def polys_from_json(path):
    data = json.loads(Path(path).read_text())
    return [sum(sp.Integer(c) * z**k for k, c in enumerate(coeffs))
            for coeffs in data["polys"]]  # [p_0, ..., p_order]


def indicial_at_finite(pj, z0):
    """Indicial polynomial in `th` at finite z0 via w=z-z0, D_w^j = w^-j ff(th,j).
    Returns sorted exponents (roots of the lowest-w-power coefficient)."""
    order = len(pj) - 1
    pj_w = [sp.Poly(sp.expand(p.subs(z, z0 + w)), w) for p in pj]
    coeffs = {}
    for N in range(0, order + max((p.degree() for p in pj_w), default=0) + 1):
        acc = sum(pj_w[j].nth(N - order + j) * ff(th, j)
                  for j in range(order + 1) if N - order + j >= 0)
        acc = sp.expand(acc)
        if acc != 0:
            coeffs[N] = acc
    return sorted(sp.solve(sp.Eq(coeffs[min(coeffs)], 0), th))


def indicial_at_infinity(pj):
    """Exponents at infinity: D_z^j = z^-j ff(phi,j), phi = z D_z = -theta_u,
    u=1/z; collect lowest power of u.  y ~ u^rho = z^{-rho}."""
    order = len(pj) - 1
    coeffs = {}
    for j in range(order + 1):
        for (k,), a in sp.Poly(pj[j], z).terms():
            coeffs.setdefault(j - k, sp.Integer(0))
            coeffs[j - k] += a * ff(-th, j)
    coeffs = {M: sp.expand(v) for M, v in coeffs.items() if sp.expand(v) != 0}
    return sorted(sp.solve(sp.Eq(coeffs[min(coeffs)], 0), th))


def rational_roots(poly_expr):
    return sorted(sp.roots(sp.Poly(poly_expr, z), filter="Q").items(),
                  key=lambda t: t[0])


def symmetric_about_a_center(e):
    """4-multiset symmetric about SOME (point-specific) center? sorted a+d==b+c.
    Returns (bool, center_or_None)."""
    s = sorted(e)
    if len(s) == 4 and s[0] + s[3] == s[1] + s[2]:
        return True, (s[0] + s[3]) / 2
    return False, None


def hypergeometric_operator(a, b, c):
    """z(1-z)y'' + (c-(a+b+1)z)y' - a b y;  z=0:{0,1-c}, z=1:{0,c-a-b}, inf:{a,b}."""
    return [-a * b, sp.expand(c - (a + b + 1) * z), sp.expand(z * (1 - z))]


def main() -> int:
    print("=" * 72)
    print("  W_18 SELF-DUALITY (FTD-0373), derived from _stencil18_operator.json")
    print("=" * 72)

    # (3) validate the indicial derivation on a known operator
    print("\n[VALIDATE derivation] hypergeometric a=1/2,b=1/3,c=3/4:")
    hg = hypergeometric_operator(R(1, 2), R(1, 3), R(3, 4))
    e0 = indicial_at_finite(hg, sp.Integer(0))
    e1 = indicial_at_finite(hg, sp.Integer(1))
    ei = indicial_at_infinity(hg)
    print(f"    z=0 {e0} (exp [0,1/4]);  z=1 {e1} (exp [-1/12,0]);  inf {ei} (exp [1/3,1/2])")
    assert e0 == [R(0), R(1, 4)]
    assert e1 == sorted([R(0), R(3, 4) - R(1, 2) - R(1, 3)])
    assert ei == [R(1, 3), R(1, 2)]
    print("    -> derivation VALIDATED (finite + infinity).")

    # (4) validate the PER-POINT predicate, incl. a point-varying self-dual control
    print("\n[VALIDATE predicate] known self-dual exponent multisets (per-point):")
    controls = [
        ("mirror-quintic  MUM z=0", [R(0), R(0), R(0), R(0)]),
        ("mirror-quintic  conifold", [R(0), R(1), R(1), R(2)]),
        ("mirror-quintic  z=inf", [R(1, 5), R(2, 5), R(3, 5), R(4, 5)]),
        ("Sym^3-elliptic", [R(0), R(1, 2), R(1), R(3, 2)]),
    ]
    for name, e in controls:
        ok, c = symmetric_about_a_center(e)
        print(f"    {name:26s} {sorted(e)} symmetric? {ok} (center {c})")
        assert ok, name
    print("    -> predicate accepts self-dual operators with POINT-VARYING centers.")

    # (1)+(2) derive W_18 exponents from the operator
    pj = polys_from_json(Path(__file__).with_name("_stencil18_operator.json"))
    print(f"\n[W_18] order {len(pj)-1}; leading coeff:")
    print("   ", sp.factor(pj[-1]))
    print("    derived exponents at each rational singular point + infinity:")
    exps = {}
    for r, mult in rational_roots(pj[-1]):
        exps[str(r)] = indicial_at_finite(pj, r)
        print(f"    z={str(r):>3} (mult {mult}): {exps[str(r)]}")
    exps["inf"] = indicial_at_infinity(pj)
    print(f"    z=inf          : {exps['inf']}")

    # (5) per-point self-duality test on the genuine finite locus
    print("\n[TEST — decisive] per-point symmetry on the genuine finite locus "
          "{z=1,-2,-3}:")
    breaks = []
    for z0 in GENUINE_FINITE:
        e = exps[str(z0)]
        ok, c = symmetric_about_a_center(e)
        if not ok:
            breaks.append(z0)
        print(f"    z={str(z0):>3}: {sorted(e)}  symmetric about a center? {ok}"
              + ("" if ok else "   <== NO center: a+d != b+c"))

    # informational only (NOT an obstruction by itself): global weight
    glob = any(all(sorted(exps[str(p)]) == sorted(k - x for x in exps[str(p)])
                   for p in GENUINE_FINITE) for k in [R(n, 2) for n in range(0, 13)])
    print(f"\n[info, not load-bearing] a single global weight k works? {glob}"
          "  (a genuine CY operator would also fail this; per-point is the test)")

    print("\n" + "=" * 72)
    print("  VERDICT")
    print("=" * 72)
    assert breaks == GENUINE_FINITE, breaks  # all three break
    print("""  W_18 is NOT self-dual [THEOREM, exact].
  At z=1,-2,-3 the exponent multiset {0,1/2,1,2} is symmetric about NO center
  (sorted a+d = 0+2 = 2 != 1/2+1 = 3/2 = b+c), so the monodromy carries NO
  nondegenerate invariant bilinear form -- neither symplectic nor orthogonal.
  Therefore W_18 is NOT a rigid-Calabi-Yau H^3 period, NOT Sym^2/Sym^3 of an
  elliptic curve, NOT a K3 transcendental piece, and its L-function is NOT a
  weight-4 newform L-function (Mazur-van Straten-Yui / Gouvea-Yui). The rigid-CY
  / weight-4-modular branch of the FTD-0372 residual is CLOSED NEGATIVE (FTD-0373).
  W_18's correct home is the theory of diagonals of rational functions: the LGF
  is literally the diagonal of 1/(1 - z*sigma_18); its Hadamard-type closed form
  is the remaining open question.""")
    return 0


if __name__ == "__main__":
    sys.exit(main())
