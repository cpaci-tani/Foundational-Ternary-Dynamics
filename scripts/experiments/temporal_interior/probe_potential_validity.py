"""probe_potential_validity.py — when is a "distance potential" a licensed
description of substrate dynamics, and does the MVC qualify?

A single voxel is a state plus a local update rule.  It is not a potential
and has no notion of distance.  "V(|q_i - q_j|)" is an emergent TWO-BODY
description obtained by localising two sources, integrating out the field
between them, and taking the propagation time to zero.

TWO INDEPENDENT LIMITS hide in that description, and they are controlled by
different small parameters:

  (1) CONSTITUENT DISPERSION.  Replace the substrate excitation's
      relativistic dispersion  w = sqrt(c^2 k^2 + M^2)  by the Newtonian
      p^2/2m.  Controlled by  v/c.
  (2) BINDING RETARDATION.  Replace the retarded field-mediated
      interaction by an instantaneous V(r).  Controlled by  w r / c.

WHICH ONE KILLS TIME DILATION?  Only (1).  Hydrogen is the proof: the
Coulomb potential is instantaneous to O(alpha^2), i.e. limit (2) is taken
and is an excellent approximation, yet atoms dilate EXACTLY.  The dilation
comes from the electron's relativistic dispersion, not from retardation of
the binding -- which is precisely what the composite calculation showed:
delta_comp is a weighted average of the CONSTITUENTS' delta_a, with the
binding supplying only the weights.

So the diagnostic question for the MVC is not "is its potential
instantaneous?" but "are its nodes Newtonian?", and how much is thereby
discarded at its own operating point.
"""
from __future__ import annotations

import numpy as np
from scipy.special import gamma as Gamma

C = 1.0 / np.sqrt(3.0)
G_STAR = Gamma(0.25) / Gamma(0.75)

# MVC: quartic mode, lam = 2, m = 4  ->  T*A = sqrt(pi) G*
TA = np.sqrt(np.pi) * G_STAR


def mvc(A, lam=2.0, m=4.0):
    """Period, peak speed and internal frequency of the MVC at amplitude A.
    Energy conservation (m/2)v^2 + lam Q^4 = lam A^4 gives v_max = A^2
    sqrt(2 lam/m), which is A^2 for the canonical lam=2, m=4."""
    T = TA * np.sqrt(m / (2 * lam)) / A
    v = A ** 2 * np.sqrt(2 * lam / m)
    return T, v, 2 * np.pi / T


print("=" * 72)
print("(1) THE MVC AT ITS OWN OPERATING POINT")
print("=" * 72)
print(f"  cone speed C = {C:.6f};  axis band top 2 asin C = "
      f"{2*np.arcsin(C):.6f}")
print(f"\n{'A':>7} {'T':>9} {'v_max/C':>10} {'(v/C)^2':>10} "
      f"{'w':>9} {'w r/C, r=1':>12} {'r=3':>8} {'in band?':>10}")
for A in (0.12, 0.20, 0.30, 0.50):
    T, v, w = mvc(A)
    print(f"{A:7.2f} {T:9.2f} {v/C:10.4f} {(v/C)**2:10.5f} {w:9.4f} "
          f"{w*1/C:12.4f} {w*3/C:8.3f} "
          f"{'yes' if w < 2*np.arcsin(C) else 'no':>10}")

print("""
  Both limits are BADLY violated at the MVC's operating point.  The
  Newtonian-node approximation discards a (v/C)^2 effect of a few percent,
  and the instantaneous-binding approximation is not even close (w r/C is
  of order unity).  The MVC is not "Galilean by nature" -- it is
  UNDER-MODELLED, and by a margin that is easily measurable.""")

print()
print("=" * 72)
print("(2) HOW THAT COMPARES TO SYSTEMS WHERE THE POTENTIAL IS LICENSED")
print("=" * 72)
c = 2.99792458e8
rows = [
    ("Earth-Sun orbit", 1.990e-7, 1.496e11),          # w = 2pi/yr, r = 1 AU
    ("hydrogen atom", 2.0670e16, 5.2918e-11),          # Rydberg, a_0
    ("heavy nucleus (typ.)", 1.5e22, 6.0e-15),         # ~10 MeV, ~6 fm
]
print(f"{'system':>24} {'w r / c':>12}   binding-potential description")
for nm, w, r in rows:
    x = w * r / c
    verdict = ("licensed" if x < 0.05 else
               "marginal" if x < 0.3 else "NOT licensed")
    print(f"{nm:>24} {x:12.3e}   {verdict}")
T, v, w = mvc(0.30)
print(f"{'MVC at A=0.30':>24} {w*1/C:12.3e}   "
      f"{'NOT licensed':>12}")
print(f"{'MVC at A=0.30 (r=3)':>24} {w*3/C:12.3e}   "
      f"{'NOT licensed':>12}")
print("""
  Hydrogen sits at ~4e-3 and the Coulomb potential is excellent there --
  yet hydrogen dilates exactly.  That is the proof that retardation is not
  what supplies dilation.  The MVC is two to three orders of magnitude
  further from the quasi-static regime than an atom, and MORE relativistic
  internally than a heavy nucleus.""")

print()
print("=" * 72)
print("(3) WHAT IS ACTUALLY DISCARDED, AND WHAT IT WOULD TAKE TO KEEP IT")
print("=" * 72)
T, v, w = mvc(0.30)
print(f"""  Modelling the nodes as Newtonian point masses throws away the
  constituents' dispersion.  By the composite result already established,
  a composite's limiting-speed excess is the momentum-weighted average of
  its constituents' -- so a carrier whose nodes carry the lattice
  dispersion inherits dilation automatically, with the binding supplying
  only the weights.

  Size of the discarded physics at A = 0.30:   (v/C)^2 = {(v/C)**2:.4f}
  i.e. about {(v/C)**2*100:.1f}% -- not 1e-40.

  So the correction is: the clock gate is NOT "the MVC is the wrong kind
  of object."  It is "the MVC's nodes were given the wrong dispersion."
  Replacing p^2/2m by the lattice-KG dispersion is a modelling change, not
  a search for a new carrier.

  One caveat that does NOT go away: at A = 0.30 the internal frequency
  w = {w:.4f} lies inside the propagating band (top {2*np.arcsin(C):.4f}),
  so such a carrier couples to travelling modes and radiates.  That is the
  separately-recorded C2 band-clearance requirement, and it is a genuine
  obstruction to a STABLE clock -- distinct from, and not solved by, the
  dispersion fix above.""")
