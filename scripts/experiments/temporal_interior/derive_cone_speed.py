"""derive_cone_speed.py — where 1/sqrt(3) comes from, exactly.

The wave speed C_SPEED = 1/sqrt(3) is currently a declared selection: it
is NOT the stability (CFL) saturation.  This script establishes:

  (1) THE STABILITY BOUND, exactly.  max|L| over the Brillouin zone for
      the M18 stencil, and hence the largest stable C.  (Answer: 16/3 at
      the face-diagonal corner, giving C <= sqrt(3)/2.)

  (2) THE CAUSAL POLYTOPES.  In one tick, influence reaches a finite set
      of sites; its convex hull is the causal polytope, and after n ticks
      the reachable region is the n-fold dilation.  An ISOTROPIC light
      cone of speed c fits inside that region for all time iff
      c <= inradius of the polytope.  We compute the inradius exactly for
      each candidate neighbourhood.

  (3) THE 1/sqrt(D) COINCIDENCE.  The octahedron's inradius (unit
      circumradius) and the per-axis component of an isotropic unit
      spread are the SAME number, 1/sqrt(D), for the same reason: both
      distribute an isotropic quantity over D orthogonal axes.  This is
      the geometric content of the "Gaussian spread" reading.

Conclusion is stated at the end, including exactly which prior choice
1/sqrt(3) is forced by.
"""
from __future__ import annotations

import sympy as sp
import itertools

k1, k2, k3 = sp.symbols('k1 k2 k3', real=True)

print("=" * 72)
print("(1) EXACT STABILITY BOUND FOR THE M18 STENCIL")
print("=" * 72)
c1, c2, c3 = sp.cos(k1), sp.cos(k2), sp.cos(k3)
L = sp.Rational(2,3)*(c1+c2+c3) + sp.Rational(2,3)*(c1*c2+c2*c3+c3*c1) - 4

# L depends on k only through u_i = cos k_i in [-1,1]; minimise over the cube
u1, u2, u3 = sp.symbols('u1 u2 u3', real=True)
Lu = sp.Rational(2,3)*(u1+u2+u3) + sp.Rational(2,3)*(u1*u2+u2*u3+u3*u1) - 4
best = None
for vals in itertools.product([-1, 0, sp.Rational(1,2), 1], repeat=3):
    val = Lu.subs({u1: vals[0], u2: vals[1], u3: vals[2]})
    if best is None or val < best[0]:
        best = (sp.nsimplify(val), vals)
# the extremum is at vertices of the u-cube; enumerate those exactly
vert = []
for vals in itertools.product([-1, 1], repeat=3):
    vert.append((sp.nsimplify(Lu.subs({u1: vals[0], u2: vals[1], u3: vals[2]})), vals))
vert.sort(key=lambda t: t[0])
print("  L at the corners of the cos-cube (sorted):")
for val, vals in vert:
    kk = tuple('pi' if v == -1 else '0' for v in vals)
    print(f"    cos k = {vals}  (k = {kk})   L = {val}")
Lmin = vert[0][0]
print(f"\n  min L = {Lmin}   =>   max|L| = {abs(Lmin)}")
C = sp.symbols('C', positive=True)
C_cfl = sp.simplify(sp.solve(sp.Eq(C/2 * sp.sqrt(abs(Lmin)), 1), C)[0])
print(f"  leapfrog stability  (C/2)*sqrt(max|L|) <= 1  =>  C <= {C_cfl}"
      f"  = {float(C_cfl):.6f}")
print(f"  the selected value 1/sqrt(3) = {float(1/sp.sqrt(3)):.6f} is NOT this bound.")

print()
print("=" * 72)
print("(2) CAUSAL POLYTOPES AND THEIR INRADII")
print("=" * 72)


def inradius(points):
    """Inradius of the convex hull of a symmetric point set: the minimum
    over facets of the distance from the origin.  For our symmetric sets
    the facet normals are among the small integer directions, so we test
    the support function against candidate normals and take the minimum
    distance over active facets."""
    cands = set()
    for n in itertools.product([-1, 0, 1], repeat=3):
        if n != (0, 0, 0):
            cands.add(n)
    best = None
    for n in cands:
        nv = sp.Matrix(n)
        h = max(sp.Matrix(p).dot(nv) for p in points)      # support function
        d = sp.simplify(h / sp.sqrt(nv.dot(nv)))           # facet distance
        if best is None or d < best[0]:
            best = (sp.nsimplify(d), n)
    return best


face = [p for p in itertools.product([-1,0,1], repeat=3) if sum(abs(x) for x in p) == 1]
edge = [p for p in itertools.product([-1,0,1], repeat=3) if sum(abs(x) for x in p) == 2]
corner=[p for p in itertools.product([-1,0,1], repeat=3) if sum(abs(x) for x in p) == 3]

sets = [
    ("von Neumann (6): octahedron", face),
    ("M18 face+edge (18): cuboctahedron", face + edge),
    ("Moore (26): cube", face + edge + corner),
]
for name, pts in sets:
    r, n = inradius(pts)
    print(f"  {name:38s} inradius = {r} = {float(r):.6f}   (facet normal {n})")

print("""
  An isotropic light cone of speed c stays inside the causal region for
  all time iff c <= inradius.  So:
     octahedral causality   =>  c <= 1/sqrt(3) = 0.577350
     cuboctahedral/cubic    =>  c <= 1""")

print("=" * 72)
print("(3) THE 1/sqrt(D) COINCIDENCE")
print("=" * 72)
D = sp.symbols('D', positive=True, integer=True)
print("  (a) octahedron in D dims, vertices at distance 1:")
print("      facets are  sum_i (+-x_i) = 1,  distance from origin = 1/sqrt(D)")
for d in (2, 3, 4):
    print(f"        D = {d}:  inradius = 1/sqrt({d}) = {float(1/sp.sqrt(d)):.6f}")
print("  (b) isotropic spread of unit total RMS over D orthogonal axes:")
print("      sigma_total^2 = sum_i sigma_i^2 = D sigma^2 = 1 => sigma = 1/sqrt(D)")
for d in (2, 3, 4):
    print(f"        D = {d}:  per-axis sigma = {float(1/sp.sqrt(d)):.6f}")
print("""
  Identical, and for the same reason: both distribute one isotropic unit
  over D orthogonal axes.  This is the exact content of the "Gaussian
  spread" reading -- the per-axis component of an isotropic unit spread
  in D = 3 is 1/sqrt(3), which is also the largest isotropic speed that
  fits inside the octahedral causal cone.""")

print("=" * 72)
print("VERDICT")
print("=" * 72)
print(f"""  C = 1/sqrt(3) is NOT the stability bound (that is sqrt(3)/2 = {float(C_cfl):.4f}).
  It IS, exactly, the largest isotropic speed whose light cone is
  contained in the OCTAHEDRAL (6-neighbour) causal cone -- equivalently
  the per-axis component of an isotropic unit spread in three dimensions.

  So the selection of the cone speed reduces to a PRIOR selection:
  which neighbourhood defines causal reach.
     if first-order (gradient/divergence, 6 neighbours) => 1/sqrt(3) FORCED
     if the Laplacian's reach (18) or Moore (26)        => bound is 1, and
                                                           1/sqrt(3) is not forced
  That is a decidable question about the dynamics, not a free choice.""")
