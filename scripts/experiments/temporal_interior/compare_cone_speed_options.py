"""compare_cone_speed_options.py — full downstream comparison of the
candidate cone speeds.

Two constraints bound C:
   CONTAINMENT  the isotropic light cone must fit inside the causal
                polytope:  C <= inradius(polytope)
   STABILITY    the leapfrog must not blow up:  C <= 2/sqrt(max|L|)

Which one BINDS depends on the polytope, and that determines the value:
   octahedral (6)   inradius 1/sqrt3 = 0.5774 < 0.8660  -> containment binds
   cubocta/cubic    inradius 1       = 1.0000 > 0.8660  -> stability binds

So the choice of causal neighbourhood selects the constant, and there are
exactly two self-consistent saturating values plus the status quo.  This
script computes every downstream quantity that moves with C.
"""
from __future__ import annotations

import sympy as sp

k = sp.symbols('k', positive=True)
C = sp.symbols('C', positive=True)

# ---- exact symbol data (from derive_lorentz_free_sector / derive_cone_speed)
MAXL = sp.Rational(16, 3)            # max|L| for M18, at the face diagonal
C_CFL = sp.simplify(2 / sp.sqrt(MAXL))       # = sqrt(3)/2
INR = {'octahedron (6)': 1/sp.sqrt(3), 'cuboctahedron (18)': sp.Integer(1),
       'cube (26)': sp.Integer(1)}

print("=" * 74)
print("THE TWO BOUNDS")
print("=" * 74)
print(f"  stability (CFL):  C <= 2/sqrt(max|L|) = 2/sqrt({MAXL}) = "
      f"{C_CFL} = {float(C_CFL):.6f}")
for nm, r in INR.items():
    binds = "CONTAINMENT binds" if float(r) < float(C_CFL) else "STABILITY binds"
    print(f"  containment, {nm:20s}: C <= {float(r):.6f}   -> {binds}")
print(f"""
  => octahedral causality selects C = 1/sqrt(3) = {float(1/sp.sqrt(3)):.6f}
     cubic / cuboctahedral selects C = sqrt(3)/2 = {float(C_CFL):.6f}
     (the containment value 1 is NOT reachable: it exceeds CFL)""")

# ---------------------------------------------------------------- dispersion
# leapfrog: Omega = 2 asin( (C/2) sqrt(-L) );  expand along an axis and along
# the body diagonal to get the C-dependent dispersion and anisotropy.
t = sp.symbols('t', positive=True)
k1, k2, k3 = sp.symbols('k1 k2 k3', positive=True)
c1, c2, c3 = sp.cos(k1), sp.cos(k2), sp.cos(k3)
L = sp.Rational(2,3)*(c1+c2+c3) + sp.Rational(2,3)*(c1*c2+c2*c3+c3*c1) - 4


def phase_velocity_series(direction, order=7):
    subs = {k1: t*direction[0]*k, k2: t*direction[1]*k, k3: t*direction[2]*k}
    mL = sp.series((-L).subs(subs), t, 0, order).removeO()
    s = (C/2) * sp.sqrt(sp.expand(mL))
    Om = 2 * sp.asin(s)
    ser = sp.series(Om, t, 0, order).removeO()
    # v = Omega / |k| ; with t as the scale parameter |k| = t*k
    return sp.expand(sp.simplify(ser / (t*k)))


ax = phase_velocity_series((1, 0, 0))
dg = phase_velocity_series((1/sp.sqrt(3), 1/sp.sqrt(3), 1/sp.sqrt(3)))
ax = sp.expand(ax.subs(t, 1))
dg = sp.expand(dg.subs(t, 1))
disp_coeff = sp.simplify(sp.expand(ax/C - 1).coeff(k, 2))
aniso = sp.simplify(sp.expand((ax - dg) / C))
aniso4 = sp.nsimplify(sp.expand(aniso).coeff(k, 4))
print("=" * 74)
print("DISPERSION AND ANISOTROPY, WITH THE TIME DISCRETISATION INCLUDED")
print("=" * 74)
print(f"  v_axis/C = 1 + ({sp.simplify(disp_coeff)}) k^2 + ...")
print(f"     => the k^2 dispersion vanishes at C = 1 (the 1-D magic step),")
print(f"        which is unreachable here because 1 > {float(C_CFL):.4f}.")
print(f"  (v_axis - v_diag)/C = ({aniso4}) k^4 + ...")

# ------------------------------------------------------------------- table
cands = [
    ("1/sqrt(3)  (octahedral containment)", 1/sp.sqrt(3)),
    ("sqrt(3)/2  (stability saturation)", C_CFL),
    ("1          (cubic containment)", sp.Integer(1)),
]
G_STAR = sp.gamma(sp.Rational(1,4)) / sp.gamma(sp.Rational(3,4))

print()
print("=" * 74)
print("DOWNSTREAM QUANTITIES")
print("=" * 74)
hdr = f"{'quantity':38s}" + "".join(f"{n.split()[0]:>13s}" for n, _ in cands)
print(hdr)
print("-" * 74)


def row(label, fn, fmt="{:>13.6f}"):
    vals = []
    for _, cv in cands:
        try:
            vals.append(fmt.format(float(fn(cv))))
        except Exception:
            vals.append(f"{'--':>13s}")
    print(f"{label:38s}" + "".join(vals))


row("stable?  (CFL margin C_cfl - C)", lambda cv: C_CFL - cv)
row("axis band top  2 asin(C)", lambda cv: 2*sp.asin(cv))
row("full band top  2 asin(2C/sqrt3)",
    lambda cv: 2*sp.asin(2*cv/sp.sqrt(3)) if 2*cv/sp.sqrt(3) <= 1 else sp.nan)
row("dispersion coeff (C^2-1)/24", lambda cv: (cv**2 - 1)/24)
row("anisotropy coeff x 1e4", lambda cv: 1e4*aniso4.subs(C, cv))
# MVC C2 clearance: eps * A_max^2 > (omega_B G*/(2 sqrt(pi)))^2
row("MVC clearance  eps*A_max^2 >",
    lambda cv: (2*sp.asin(cv) * G_STAR / (2*sp.sqrt(sp.pi)))**2)
row("  => eps needed at A_max=0.5",
    lambda cv: (2*sp.asin(cv) * G_STAR / (2*sp.sqrt(sp.pi)))**2 / sp.Rational(1,4))
row("tick t_phys / t_Planck  (= C)", lambda cv: cv)
row("t_phys  [1e-44 s]", lambda cv: float(cv)*5.391247)

print("-" * 74)
print(f"""
NOTES
  * C = 1 is dynamically FORBIDDEN: it exceeds the CFL bound
    {float(C_CFL):.6f}, so the 'saturate cubic containment' option does not
    exist for this stencil.  Under cubic/cuboctahedral causality the
    binding constraint is stability, giving sqrt(3)/2.
  * The anisotropy coefficient is nearly C-independent (the spatial k^6
    term dominates); the time discretisation contributes at the same
    order only near the stability edge.
  * Raising C RAISES the band top, which makes the clock-carrier C2
    clearance strictly harder: the required eps grows as (2 asin C)^2.
  * t_phys = C * ell_P / c, so the tick duration scales linearly with C;
    every dimensional prediction downstream of the calibration moves.""")
