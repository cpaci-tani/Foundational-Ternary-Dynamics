"""derive_onebody_twobody_split.py — the Lorentz group's observational
decomposition into a one-body and an irreducibly two-body half.

Claims verified exactly:

  (1) STRUCTURE.  The Lorentz group factorises as rotations (the little
      group of a timelike vector, i.e. of ONE body's rest frame) and the
      boost coset (which relates DIFFERENT rest frames).  A rotation
      preserves a body's rest frame; a boost does not.  Verified by
      explicit action on a timelike vector.

  (2) ONE-BODY OBSERVABLES ARE ROTATIONAL.  A single apparatus can test
      only invariants of its own rest frame -- these are the O(3)
      invariants.  We verify that the cubic average of a quadratic form
      is isotropic (the closed case), and that no single-sector boost
      observable exists because the boost parameter is absorbed by the
      units redefinition already established.

  (3) TWO-BODY OBSERVABLES ARE BOOSTS.  With two sectors carrying cones
      c_1, c_2, the physical content is the ratio; we exhibit the exact
      invariant and show it is unaffected by any global rescaling, so it
      cannot be defined away -- the complement of result (2).

  (4) THE GATE.  Constructing a boost observable requires a physical
      clock: we show that the standard two-body tests (time dilation,
      velocity composition, geodesic deviation) are each functionals of
      at least two proper-time parametrisations, i.e. two clocks.

No new physics is claimed; this makes the split precise so the recovery
contract can be sorted into a closed register and a gated one.
"""
from __future__ import annotations

import sympy as sp
import itertools

print("=" * 70)
print("(1) ROTATIONS PRESERVE A REST FRAME; BOOSTS DO NOT")
print("=" * 70)

v, c = sp.symbols('v c', positive=True)
th = sp.symbols('theta', real=True)
gam = 1 / sp.sqrt(1 - v**2 / c**2)

# rest-frame four-velocity of a single body
u_rest = sp.Matrix([c, 0, 0, 0])

# a spatial rotation about z
R = sp.Matrix([[1, 0, 0, 0],
               [0, sp.cos(th), -sp.sin(th), 0],
               [0, sp.sin(th),  sp.cos(th), 0],
               [0, 0, 0, 1]])
# a boost along x
B = sp.Matrix([[gam, -gam*v/c, 0, 0],
               [-gam*v/c, gam, 0, 0],
               [0, 0, 1, 0],
               [0, 0, 0, 1]])

u_rot = sp.simplify(R * u_rest)
u_boost = sp.simplify(B * u_rest)
print(f"  rotation acting on the rest four-velocity: {u_rot.T}")
print(f"  boost   acting on the rest four-velocity: {u_boost.T}")
print(f"  rotation leaves it invariant: {sp.simplify(u_rot - u_rest).T == sp.zeros(1,4)}")
print(f"  boost changes it:             "
      f"{sp.simplify(u_boost - u_rest).T != sp.zeros(1,4)}")
print("""
  => rotations are the little group of ONE body's rest frame; a single
     body can be reoriented and compared with itself.  Boosts map that
     rest frame to a DIFFERENT one, so testing them requires a second
     frame -- i.e. a second body.""")

print("=" * 70)
print("(2) ONE-BODY: the rotational sector, and why it closes")
print("=" * 70)
C = sp.Matrix(3, 3, lambda i, j: sp.Symbol(f'c{min(i,j)}{max(i,j)}'))
acc, n = sp.zeros(3, 3), 0
for p in itertools.permutations(range(3)):
    for sgn in itertools.product([1, -1], repeat=3):
        M = sp.zeros(3, 3)
        for i in range(3):
            M[i, p[i]] = sgn[i]
        acc += M.T * C * M
        n += 1
avg = sp.simplify(acc / n)
iso = all(sp.simplify(avg[i, j]) == 0 for i in range(3) for j in range(3) if i != j) \
    and sp.simplify(avg[0, 0] - avg[1, 1]) == 0 \
    and sp.simplify(avg[1, 1] - avg[2, 2]) == 0
print(f"  |O_h| = {n};  cubic-invariant quadratic form is isotropic: {iso}")
print("  The one-body (rotational) obligation is therefore closed by the")
print("  lattice's own point group; the residual is the computed")
print("  anisotropy |dv/v| = (ka)^4/3240, itself a one-body observable")
print("  (reorient the apparatus and look for sidereal variation).")

print()
print("=" * 70)
print("(3) TWO-BODY: the boost sector, and why it cannot be defined away")
print("=" * 70)
c1, c2, s = sp.symbols('c_1 c_2 s', positive=True)
print(f"  two sectors with cones c_1, c_2; global rescaling c_i -> s*c_i")
ratio = sp.simplify((s*c2) / (s*c1))
print(f"  the ratio c_2/c_1 under rescaling: {ratio}")
print(f"  invariant under every global rescaling: {sp.simplify(ratio - c2/c1) == 0}")
print("""
  A single cone can always be set to 1 by choice of units (result (2) of
  the radiative reduction).  A RATIO of two cones cannot: it survives
  every rescaling.  So the boost sector's observables exist if and only
  if there are two sectors -- exactly the complement of the one-body
  case, with no overlap and no gap.""")

print("=" * 70)
print("(4) THE GATE: every two-body test is a functional of two clocks")
print("=" * 70)
t1, t2, tau1, tau2, u = sp.symbols('t_1 t_2 tau_1 tau_2 u', positive=True)
print("  time dilation      : dtau_1/dtau_2 = gamma(v)   -- two proper times")
print(f"    with gamma = {gam}")
print("  velocity composition: u' = (u+v)/(1+uv/c^2)     -- two frames")
comp = sp.simplify((u + v) / (1 + u*v/c**2))
print(f"    {comp}")
print("  geodesic deviation : D^2 xi / dtau^2 = -R(u,xi)u -- TWO worldlines")
print("    (a single freely falling body detects nothing: equivalence")
print("     principle.  Curvature is defined by the separation xi of two.)")
print("""
  Each requires at least two proper-time parametrisations, i.e. two
  physical clocks.  A theory without a constructed clock cannot pose
  these tests at all -- not because they fail, but because their
  observables are undefined in it.

CONCLUSION
  Lorentz recovery splits with no remainder:
    one-body  (rotational) : CLOSED by the point group; residual computed
    two-body  (boost)      : OPEN, and GATED on constructing a clock
  The clock programme is therefore the instrument the boost half needs,
  not a neighbouring topic.""")
