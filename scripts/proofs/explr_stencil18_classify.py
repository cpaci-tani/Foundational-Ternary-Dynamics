"""explr_stencil18_classify.py — B0(ii)/(iii): classify the 18-point LGF's
order-4 annihilating ODE operator (from explr_stencil18_reconstruct.py).

All indicial (local-exponent) computations are EXACT in sympy via the Euler-
operator method: at a point zeta, with w = z - zeta and theta_w = w d/dw,
    w^r D^r = theta_w (theta_w - 1) ... (theta_w - r + 1),
so  L = sum_r p_r(zeta+w) D^r = sum_{r,j} a_{r,j} w^{j-r} theta_w^{(r)}  where
p_r(zeta+w) = sum_j a_{r,j} w^j; bucketing by the power e = j - r, the minimal
bucket is the indicial polynomial and its roots are the local exponents.
Exact at zeta = 0 and at every RATIONAL root of the leading coefficient; z=inf
handled by z = 1/t.  This distinguishes:
  * an ORDINARY point (exponents 0,1,...,R-1),
  * an APPARENT singularity (distinct non-negative integer exponents),
  * a genuine branch point (fractional/irrational exponents), and
  * a maximally-unipotent (MUM / Calabi-Yau) point (all exponents equal).

Usage:
    python scripts/proofs/explr_stencil18_classify.py
"""

from __future__ import annotations

import json
import os
import sys

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

import sympy as sp

HERE = os.path.dirname(os.path.abspath(__file__))
OP = os.path.join(HERE, "_stencil18_operator.json")
z, w, s = sp.symbols("z w s")


def load_operator():
    with open(OP, encoding="utf-8") as f:
        d = json.load(f)
    return d["order"], d["degree"], d["polys"], d.get("N_moments")


def poly_from_coeffs(coeffs, var):
    return sum(sp.Integer(c) * var ** j for j, c in enumerate(coeffs))


def theta_falling(r):
    e = sp.Integer(1)
    for i in range(r):
        e *= (s - i)
    return e


def indicial_exponents(polys, order, zeta):
    """Exact local exponents at a finite point z=zeta (rational)."""
    buckets = {}
    for r, coeffs in enumerate(polys):
        pr = poly_from_coeffs(coeffs, z)
        # Taylor of p_r about zeta in w: a_{r,j} = coeff of w^j
        prw = sp.expand(pr.subs(z, zeta + w))
        pw = sp.Poly(prw, w) if prw != 0 else None
        if pw is None:
            continue
        for j in range(pw.degree() + 1):
            a = pw.coeff_monomial(w ** j)
            if a == 0:
                continue
            e = j - r
            buckets.setdefault(e, sp.Integer(0))
            buckets[e] += a * theta_falling(r)
    e_min = min(buckets)
    Q = sp.Poly(sp.expand(buckets[e_min]), s)
    return sp.roots(Q)


def indicial_at_infinity(polys, order):
    """Exponents at z=infinity via z = 1/t (D_z = -t^2 D_t), exact."""
    t = sp.symbols("t")
    # Build L in t: sum_r p_r(1/t) (D_z)^r acting; use D_z = -t^2 d/dt iterated.
    # Easier: exponents at infinity of L are the exponents at t=0 of the
    # transformed operator; construct it symbolically via a test monomial z^-a.
    # Substitute F = z^(-a) = t^a and find the indicial in a directly:
    #   p_r(z) D^r z^{-a} = p_r(z) * (-a)(-a-1)...(-a-r+1) z^{-a-r}.
    a = sp.symbols("a")
    buckets = {}
    for r, coeffs in enumerate(polys):
        ff = sp.Integer(1)
        for i in range(r):
            ff *= (-a - i)
        pr = poly_from_coeffs(coeffs, z)
        term = sp.expand(pr * ff * z ** (-a - r))  # sum of c * z^{deg - a - r}
        pd = sp.Poly(pr, z)
        for d in range(pd.degree() + 1):
            c = pd.coeff_monomial(z ** d)
            if c == 0:
                continue
            e = d - r          # power of z is e - a; group by e
            buckets.setdefault(e, sp.Integer(0))
            buckets[e] += c * ff
    # at infinity the dominant behaviour is the LARGEST power of z (e max),
    # exponent nu = -a from z^{-a}; indicial = coeff at e_max
    e_max = max(buckets)
    Q = sp.Poly(sp.expand(buckets[e_max]), a)
    roots = sp.roots(Q)
    # exponents at infinity are the a-values (F ~ z^{-a})
    return roots


def kind_of(exps_dict):
    exps = []
    for r_, mult in exps_dict.items():
        exps.extend([r_] * mult)
    reals = [sp.nsimplify(e) for e in exps]
    allrealint = all(e.is_real and e.is_integer for e in reals)
    allsame = len(set(reals)) == 1 and len(reals) > 1
    hasfrac = any(e.is_rational and not e.is_integer for e in reals)
    hasirr = any(not e.is_rational for e in reals)
    if allsame:
        return "MUM (all equal -> Calabi-Yau/maximal-unipotent point)"
    if hasirr:
        return "genuine (irrational exponents)"
    if hasfrac:
        return "genuine BRANCH point (fractional exponents)"
    if allrealint:
        # distinct consecutive integers 0..R-1 = ordinary; other integer sets
        # are apparent-or-logarithmic (log-freeness not certified here)
        srt = sorted(int(e) for e in reals)
        if srt == list(range(len(srt))):
            return "ordinary (exponents 0..R-1) OR apparent"
        return "integer exponents (apparent or logarithmic — log-check needs CAS)"
    return "mixed"


def fmt(exps_dict):
    parts = []
    for r_, mult in sorted(exps_dict.items(), key=lambda kv: sp.re(kv[0])):
        parts.append(f"{sp.nsimplify(r_)}" + (f"(x{mult})" if mult > 1 else ""))
    return "{" + ", ".join(parts) + "}"


def main() -> int:
    order, degree, polys, Nm = load_operator()
    print("=" * 70)
    print(f"  18-pt LGF operator classification — order {order}, degree {degree}")
    print(f"  (reconstructed from {Nm} exact moments; exact sympy indicials)")
    print("=" * 70)

    Lp = polys[order]
    lead = poly_from_coeffs(Lp, z)
    print(f"\n  leading coeff p_{order}(z) = {sp.factor(lead)}")

    # factor to get exact rational + irrational roots
    fac = sp.factor_list(lead)
    print("\n  singular points (roots of leading coeff), exact exponents:")
    # z = 0 (if it's a root)
    done = set()
    # collect rational roots
    rat_roots = []
    for (fpoly, mult) in fac[1]:
        p = sp.Poly(fpoly, z)
        for rt in sp.roots(p):
            if rt.is_rational:
                rat_roots.append(rt)
    rat_roots = sorted(set(rat_roots), key=lambda r: (sp.Abs(r), r))
    if sp.Integer(0) not in rat_roots and lead.subs(z, 0) == 0:
        rat_roots = [sp.Integer(0)] + rat_roots

    for zeta in rat_roots:
        exps = indicial_exponents(polys, order, zeta)
        physical = "  <- z=1 PHYSICAL (radius of convergence, max|sigma_18|=1)" \
            if zeta == 1 else ("  <- z=0" if zeta == 0 else "")
        print(f"    z = {zeta}: exponents {fmt(exps)}  -> {kind_of(exps)}{physical}")

    # irrational / complex singular points: report count + that they need
    # algebraic-point indicials (done numerically only)
    irr = [(fpoly, mult) for (fpoly, mult) in fac[1]
           if sp.Poly(fpoly, z).degree() >= 2 and not all(r.is_rational for r in sp.roots(sp.Poly(fpoly, z)))]
    if irr:
        print("    (plus irrational/complex singular points from factors:",
              ", ".join(str(sp.Poly(f, z).as_expr()) for f, m in irr), ")")

    # z = infinity
    einf = indicial_at_infinity(polys, order)
    print(f"    z = oo: exponents {fmt(einf)}  -> {kind_of(einf)}")

    print("\n  READING:")
    print("  * order 4 (SC is order 3 / CM Gamma(1/24)-class; FCC is order 6):")
    print("    the 18-pt default stencil is NEITHER classical lattice's order.")
    print("  * z=0 carries a 1/2 exponent (a square-root branch) with a triple")
    print("    exponent-0 degeneracy; the physical point z=1 carries the 3D")
    print("    lattice's (1-z)^{1/2} branch. A 1/2 (not 0) at the origin means")
    print("    this is NOT a strict MUM/Calabi-Yau operator: the natural")
    print("    hypothesis is a symmetric-power / sqrt-twist of a lower-order")
    print("    (elliptic/modular) operator -- which, if confirmed by a D-module")
    print("    factorization (ore_algebra/HolonomicFunctions, out of env),")
    print("    would give W_18 a modular/CM (Gamma-quotient) closed form after")
    print("    all. That factorization is the remaining P3(b) step.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
