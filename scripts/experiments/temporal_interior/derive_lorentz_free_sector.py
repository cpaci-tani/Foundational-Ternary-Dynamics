"""derive_lorentz_free_sector.py — the free-sector Lorentz-recovery proof.

Derives, exactly and symbolically, the dispersion relation of the
production M18 stencil

    Lap(f) = (1/3) sum_face f + (1/6) sum_edge f - 4 f

and extracts:
  (P1) the leading correction to -k^2, and whether it is isotropic;
  (P2) the leading ANISOTROPIC term, i.e. the first term that cannot be
       written as a function of k^2 alone -- this is the free-sector
       Lorentz violation;
  (P3) the fractional direction-dependent phase-velocity variation, and
       its numerical size at physically relevant momenta.

Everything is exact rational arithmetic; the numerical section only
evaluates the closed forms.  No fitting anywhere.
"""
from __future__ import annotations

import sympy as sp

k1, k2, k3, k, a = sp.symbols('k1 k2 k3 k a', positive=True)

# ---------------------------------------------------------------- symbol
c1, c2, c3 = sp.cos(k1), sp.cos(k2), sp.cos(k3)
face_sum = 2 * (c1 + c2 + c3)                       # 6 face neighbours
edge_sum = 4 * (c1*c2 + c2*c3 + c3*c1)              # 12 edge neighbours
L = sp.Rational(1, 3) * face_sum + sp.Rational(1, 6) * edge_sum - 4

print("=" * 70)
print("M18 stencil symbol  L(k) = (1/3)Sf + (1/6)Se - 4")
print("=" * 70)

# ---------------------------------------------------------------- expand
# Expand in a common scale: k_i -> t*k_i, series in t.
t = sp.symbols('t', positive=True)
Lt = L.subs({k1: t*k1, k2: t*k2, k3: t*k3})
ser = sp.series(Lt, t, 0, 9).removeO()
ser = sp.expand(ser)

K2 = k1**2 + k2**2 + k3**2
terms = {}
for order in (2, 4, 6, 8):
    coeff = sp.expand(ser.coeff(t, order))
    terms[order] = sp.simplify(coeff)
    print(f"\norder k^{order}:")
    print("   ", sp.factor(sp.simplify(coeff)))

# ---------------------------------------------------- isotropy tests
print("\n" + "=" * 70)
print("ISOTROPY OF EACH ORDER")
print("=" * 70)


def isotropic_part_test(expr, n):
    """Is expr a multiple of (k^2)^(n/2)?  Return the multiple or None."""
    target = K2**(n // 2)
    ratio = sp.simplify(sp.expand(expr) / target)
    if ratio.free_symbols & {k1, k2, k3}:
        return None
    return sp.nsimplify(ratio)


for order in (2, 4, 6):
    r = isotropic_part_test(terms[order], order)
    if r is not None:
        print(f"  k^{order}: ISOTROPIC, equals {r} * (k^2)^{order//2}")
    else:
        print(f"  k^{order}: ANISOTROPIC  <-- leading Lorentz violation")

# ------------------------------------------- quantify the anisotropy
print("\n" + "=" * 70)
print("LEADING ANISOTROPY (order k^6)")
print("=" * 70)
T6 = terms[6]

# evaluate along the two extremal directions of the cubic group
axis = {k1: k, k2: 0, k3: 0}
diag = {k1: k/sp.sqrt(3), k2: k/sp.sqrt(3), k3: k/sp.sqrt(3)}
face_diag = {k1: k/sp.sqrt(2), k2: k/sp.sqrt(2), k3: 0}

T6_axis = sp.simplify(T6.subs(axis))
T6_diag = sp.simplify(T6.subs(diag))
T6_fdiag = sp.simplify(T6.subs(face_diag))
print(f"  along <100>: {T6_axis}")
print(f"  along <110>: {T6_fdiag}")
print(f"  along <111>: {T6_diag}")
spread = sp.simplify(T6_axis - T6_diag)
print(f"  <100> - <111> spread: {spread}")

# ------------------------------------------------ phase velocity
# omega^2 = C^2 * (-L),  so v^2 = omega^2/k^2 = C^2 * (-L)/k^2
print("\n" + "=" * 70)
print("PHASE VELOCITY ANISOTROPY")
print("=" * 70)
mL = -ser                                   # -L(k), the positive symbol
v2_axis = sp.simplify((mL.subs(axis) / (t*k)**2).subs(t, 1))
v2_diag = sp.simplify((mL.subs(diag) / (t*k)**2).subs(t, 1))
v2_axis = sp.expand(v2_axis)
v2_diag = sp.expand(v2_diag)
print(f"  v^2/C^2 along <100>: {sp.nsimplify(v2_axis)}")
print(f"  v^2/C^2 along <111>: {sp.nsimplify(v2_diag)}")

dv2 = sp.simplify(v2_axis - v2_diag)
print(f"\n  Delta(v^2)/C^2 = {sp.factor(dv2)}")
lead = sp.simplify(sp.series(dv2, k, 0, 7).removeO())
print(f"  leading term   = {lead}")
# Delta v / v = (1/2) Delta v^2 / v^2, at leading order v^2 -> C^2
dv_over_v = sp.simplify(lead / 2)
print(f"  Delta v / v    = {dv_over_v}    (dimensionless, k in units of 1/a)")

coeff6 = sp.nsimplify(sp.expand(dv_over_v).coeff(k, 4))
print(f"\n  => Delta v / v = ({coeff6}) * (k a)^4 + O((ka)^6)")
print(f"     i.e. |Delta v / v| = (k a)^4 / {sp.denom(abs(coeff6))}")

# ------------------------------------------------ numbers
print("\n" + "=" * 70)
print("PHYSICAL SIZE  (a = Planck length; E_P = 1.22e28 eV)")
print("=" * 70)
C = float(coeff6)
E_P = 1.22e28          # eV
cases = [
    ("optical photon, 2 eV", 2.0),
    ("hard X-ray, 1e5 eV", 1e5),
    ("Fermi-LAT GeV photon", 1e9),
    ("highest gamma ray ~PeV", 1e15),
    ("UHECR primary ~1e20 eV", 1e20),
]
print(f"  coefficient |Delta v/v| / (E/E_P)^4 = {abs(C):.6g}")
for name, E in cases:
    x = E / E_P
    print(f"  {name:28s} (E/E_P = {x:8.2e}) -> |Dv/v| = {abs(C)*x**4:.3e}")

print("""
Comparison points (order of magnitude, current experiment):
  modern optical-cavity isotropy tests   |Dc/c| < ~1e-18
  astrophysical birefringence / dispersion bounds on the
  dimension-6 photon sector (SME)        far tighter than the above
So the free-sector anisotropy computed here is many tens of orders of
magnitude below anything measurable, for every observed photon energy.
""")

# ------------------------------------------------ 7-point comparison
print("=" * 70)
print("CONTROL: the naive 7-point stencil (face only) is NOT isotropic")
print("=" * 70)
L7 = 2*(c1 + c2 + c3) - 6
L7t = L7.subs({k1: t*k1, k2: t*k2, k3: t*k3})
ser7 = sp.expand(sp.series(L7t, t, 0, 7).removeO())
q4 = sp.expand(ser7.coeff(t, 4))
r7 = isotropic_part_test(q4, 4)
print(f"  7-point k^4 coefficient: {sp.factor(q4)}")
print(f"  isotropic? {'yes' if r7 is not None else 'NO -- anisotropic at k^4'}")
print(f"  M18 k^4 coefficient:     {sp.factor(terms[4])}")
print("  => the 12 edge terms are exactly what removes the k^4 anisotropy.")
