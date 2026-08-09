"""derive_register_barrier_lower_bound.py — the register barrier, exactly.

CLOSES the open problem "The register's lower bound".

STATUS BEFORE THIS SCRIPT.  The paper proved  dE <= eps  by exhibiting an
explicit hinge path (break one bond, swing on the remaining two), and
CORROBORATED the matching lower bound with a watershed over a finite grid
that returned 1.016--1.021 eps.  A finite grid cannot exclude a channel
narrower than its cell, so the equality dE = eps was asserted at
proposition grade on the strength of an upper bound plus a measurement.
An external review flagged exactly this, correctly.

THE PROOF.  The lower bound needs no grid at all over most of the domain.

  (1) TOPOLOGY.  The two mirror equilibria sit at z = +h and z = -h with
      h = sqrt(1 - s^2/3) > 0.  Any continuous path between them is a
      continuous z(t) running from +h to -h, so by the intermediate value
      theorem it has a moment with z = 0: EVERY path crosses the anchor
      plane.  Hence
          barrier  >=  min_{z=0} E  -  E_ground.
      This converts a min-max over an infinite-dimensional path space
      into a minimisation over a PLANE.

  (2) THE PLANE MINIMUM IS EXACTLY -2 eps, and mostly by inspection.
      Write N(x) = #{i : the i-th bond is in its attractive window}.  The
      bond law  V(q) = -16 eps (q-3/2)^2 (q-3/4)  for q < 3/2, else 0,
      has three properties, each verified below:
          V >= -eps           everywhere, with equality only at q = 1
          V  =  0             for q >= 3/2      (COMPACT SUPPORT)
          V >= 0              for q <= 3/4
      Therefore E >= -eps * N pointwise.  So on the whole region where
      N <= 2 we get E >= -2 eps WITH NO NUMERICS -- an exact inequality
      from the shape of one function.  Only the N = 3 set can threaten
      the bound, and there the margin turns out to be ~1.5 eps, which a
      Lipschitz-certified scan covers with room to spare.

  (3) ATTAINMENT.  In-plane, the body can sit at distance exactly 1 from
      two anchors.  Its distance to the third is then
      s*sqrt(3)/2 + sqrt(1 - s^2/4), which for s in [0.9, 1.3] EXCEEDS
      the support radius 3/2 -- so the third bond contributes exactly
      zero, not a small tail.  E = -2 eps exactly.

  Compact support is what makes this an equality instead of an estimate:
  with a Lennard-Jones or Morse tail the third bond would contribute a
  small negative amount and the barrier would be slightly below eps.

  Together: min_{z=0} E = -2 eps exactly, E_ground = -3 eps, so
  barrier >= eps; the hinge path gives barrier <= eps; hence

      ***  barrier = eps  EXACTLY, for s in [0.9, 1.3]  ***

  and the transition state is identified: the in-plane two-bond
  configuration, which is precisely where the hinge path crosses.

SCOPE.  Exact within the declared bond law and the three-anchor geometry.
It does NOT establish anything about a substrate, and the compact support
is a modelling choice, not a derived fact -- the proof says what that
choice buys.
"""
from __future__ import annotations

import numpy as np


EPS = 1.0
R0 = 1.0
Q_SUP = 1.5          # compact support: V == 0 for q >= Q_SUP
Q_REP = 0.75         # V >= 0 for q <= Q_REP


def V(q):
    """Bond law with compact support at q = 3/2, depth eps at q = 1."""
    q = np.asarray(q, dtype=float)
    return np.where(q < Q_SUP, -16.0 * EPS * (q - 1.5) ** 2 * (q - 0.75), 0.0)


def dV(q):
    """V'(q) = -48 eps (q - 3/2)(q - 1) inside the support, else 0."""
    q = np.asarray(q, dtype=float)
    return np.where(q < Q_SUP, -48.0 * EPS * (q - 1.5) * (q - 1.0), 0.0)


def anchors(s):
    """Three anchors on an equilateral triangle of side s, in z = 0."""
    return np.array([[0.0, 0.0],
                     [s, 0.0],
                     [s / 2.0, s * np.sqrt(3.0) / 2.0]])


def E_plane(P, A):
    """Total energy of an in-plane body position P against anchors A."""
    d = np.linalg.norm(P[:, None, :] - A[None, :, :], axis=2)
    return V(d).sum(axis=1), d


# ---------------------------------------------------------------------
# Step 1 — the three properties of the bond law, verified
# ---------------------------------------------------------------------
def verify_bond_law():
    q = np.linspace(0.0, 3.0, 2_000_001)
    v = V(q)

    vmin, qmin = v.min(), q[v.argmin()]
    assert vmin >= -EPS - 1e-12, f"V dips below -eps: {vmin}"
    assert abs(vmin + EPS) < 1e-9, f"depth is not eps: {vmin}"
    assert abs(qmin - 1.0) < 1e-5, f"minimum not at q=1: {qmin}"

    assert np.all(v[q >= Q_SUP] == 0.0), "support is not compact at 3/2"
    assert np.all(v[q <= Q_REP] >= -1e-15), "V negative inside q <= 3/4"

    # analytic curvature at the minimum: V''(1) = 24 eps
    h = 1e-5
    k = (V(1.0 + h) - 2 * V(1.0) + V(1.0 - h)) / h**2
    assert abs(k - 24.0 * EPS) < 1e-3, f"curvature {k} != 24 eps"

    # Lipschitz constant on the region that can matter (all bonds >= 1/2;
    # a bond below 1/2 already costs >= +4 eps, see verify_repulsive_exit)
    qq = np.linspace(0.5, Q_SUP, 200_001)
    Lmax = np.abs(dV(qq)).max()
    print(f"  [verify] V min          = {vmin:+.12f}  at q = {qmin:.6f}"
          f"   (exact -eps at q=1)")
    print(f"  [verify] V(q>=3/2)      = 0 identically        (compact support)")
    print(f"  [verify] V(q<=3/4)      >= 0                   (repulsive core)")
    print(f"  [verify] V''(1)         = {k:.6f}              (exact 24 eps)")
    print(f"  [verify] max|V'| on [1/2, 3/2] = {Lmax:.6f}    (per bond)")
    return Lmax


def verify_repulsive_exit():
    """A bond closer than 1/2 costs at least +4 eps, so E >= +2 eps > -2 eps.

    V is decreasing on [0, 1] (V' = -48 eps (q-3/2)(q-1) < 0 there), so
    for q <= 1/2 we have V(q) >= V(1/2) = +4 eps.  With the other two
    bonds bounded below by -eps each, E >= 4 - 2 = +2 eps.  Such points
    can therefore be excluded from any scan without loss.
    """
    v_half = float(V(0.5))
    assert v_half > 3.99 * EPS, f"V(1/2) = {v_half}, expected +4 eps"
    q = np.linspace(0.0, 1.0, 100_001)
    assert np.all(np.diff(V(q)) <= 1e-12), "V not decreasing on [0,1]"
    print(f"  [verify] V(1/2)         = {v_half:+.6f}"
          f"               => any bond < 1/2 gives E >= +2 eps")
    return v_half


# ---------------------------------------------------------------------
# Step 2 — the exact part: N <= 2 needs no numerics
# ---------------------------------------------------------------------
def verify_exact_N2_bound():
    """E >= -eps * N pointwise, hence E >= -2 eps wherever N <= 2.

    This is an inequality between functions, not a measurement: V >= -eps
    everywhere and V = 0 outside the window, so a point with at most two
    bonds inside the window cannot go below -2 eps.  No grid involved.
    """
    rng = np.random.default_rng(20260809)
    q = rng.uniform(0.0, 3.0, 400_000)
    v = V(q)
    inside = (q > Q_REP) & (q < Q_SUP)
    assert np.all(v[~inside] >= -1e-15), "V < 0 outside the window"
    assert np.all(v >= -EPS - 1e-12), "V < -eps somewhere"
    print("  [verify] E >= -eps * N  pointwise  =>  N <= 2 gives E >= -2 eps"
          "   (exact, no grid)")


# ---------------------------------------------------------------------
# Step 3 — the only risky set: N = 3, certified by Lipschitz + grid
# ---------------------------------------------------------------------
def certify_N3(s, Lper, step=0.002):
    """Certified lower bound for E on the in-plane N=3 set.

    Any point of the plane lies within step/sqrt(2) of a grid node, and E
    is Lipschitz with constant 3*Lper on the region where every bond
    exceeds 1/2 (elsewhere E >= +2 eps by verify_repulsive_exit).  So

        min_plane E  >=  min_grid E  -  3 * Lper * step / sqrt(2).
    """
    A = anchors(s)
    lo = A.min(axis=0) - Q_SUP - 0.1
    hi = A.max(axis=0) + Q_SUP + 0.1
    gx = np.arange(lo[0], hi[0] + step, step)
    gy = np.arange(lo[1], hi[1] + step, step)
    X, Y = np.meshgrid(gx, gy, indexing="ij")
    P = np.stack([X.ravel(), Y.ravel()], axis=1)

    E, d = E_plane(P, A)
    N = ((d > Q_REP) & (d < Q_SUP)).sum(axis=1)
    near = (d.min(axis=1) >= 0.5)          # repulsive core excluded, proven

    m3 = N == 3
    e3 = E[m3 & near].min() if np.any(m3 & near) else np.inf
    slack = 3.0 * Lper * step / np.sqrt(2.0)
    certified3 = e3 - slack

    e_all = E[near].min()
    return dict(n3_count=int((m3 & near).sum()), e3_grid=float(e3),
                slack=float(slack), e3_certified=float(certified3),
                e_grid_min=float(e_all))


# ---------------------------------------------------------------------
# Step 4 — attainment: the in-plane two-bond configuration
# ---------------------------------------------------------------------
def attainment(s):
    """The point at distance exactly 1 from two anchors, third out of support."""
    A = anchors(s)
    # intersection of unit circles about a1, a2, taken on the far side of a3
    P = np.array([[s / 2.0, -np.sqrt(1.0 - s**2 / 4.0)]])
    E, d = E_plane(P, A)
    d = d[0]
    return dict(d1=float(d[0]), d2=float(d[1]), d3=float(d[2]),
                E=float(E[0]), out_of_support=bool(d[2] >= Q_SUP))


def ground_state(s):
    """Both mirror minima: body at the centroid, height +-h, all bonds at 1."""
    h = np.sqrt(1.0 - s**2 / 3.0)
    A3 = np.column_stack([anchors(s), np.zeros(3)])
    P = np.array([s / 2.0, s / (2.0 * np.sqrt(3.0)), h])
    d = np.linalg.norm(P - A3, axis=1)
    return float(V(d).sum()), float(h), d


def main():
    print("Register barrier — matching lower bound, proven")
    print(f"  law: V(q) = -16 eps (q-3/2)^2 (q-3/4) for q < 3/2, else 0;"
          f"  eps = {EPS}")
    print()
    Lper = verify_bond_law()
    verify_repulsive_exit()
    verify_exact_N2_bound()
    print()

    print("  s      h       E_ground   plane min  (grid)   N=3 certified"
          "   attained E   d3      barrier")
    print("  " + "-" * 88)
    barriers = []
    for s in (0.9, 1.0, 1.1, 1.2, 1.3):
        Eg, h, dg = ground_state(s)
        assert np.allclose(dg, 1.0, atol=1e-12), f"ground bonds != r0: {dg}"
        assert abs(Eg + 3 * EPS) < 1e-12, f"ground energy != -3 eps: {Eg}"

        cert = certify_N3(s, Lper)
        att = attainment(s)
        assert att["out_of_support"], (
            f"s={s}: third bond at {att['d3']:.4f} < 3/2, so the two-bond "
            "configuration is NOT exactly -2 eps")
        assert abs(att["E"] + 2 * EPS) < 1e-12, (
            f"s={s}: two-bond configuration is {att['E']}, not -2 eps")
        # the N=3 set must stay clear of -2 eps, certified
        assert cert["e3_certified"] > -2.0 * EPS, (
            f"s={s}: N=3 set certified only to {cert['e3_certified']:.4f}, "
            "which does not clear -2 eps")

        barrier = att["E"] - Eg
        barriers.append(barrier)
        print(f"  {s:.1f}  {h:.4f}  {Eg:+.6f}   {cert['e_grid_min']:+.6f}"
              f"        {cert['e3_certified']:+.6f}     {att['E']:+.6f}"
              f"   {att['d3']:.4f}  {barrier:.9f}")

    print()
    b = np.array(barriers)
    assert np.allclose(b, EPS, atol=1e-12), f"barrier != eps: {b}"
    print(f"  [verify] barrier = {b.min():.12f} .. {b.max():.12f}"
          f"   (exact eps across s in [0.9, 1.3])")
    print(f"  [verify] upper bound  <= eps   explicit hinge path")
    print(f"  [verify] lower bound  >= eps   IVT + plane minimum -2 eps")
    print(f"  [verify] transition state identified: the in-plane two-bond"
          f" configuration")
    print()
    print("  QED — barrier = eps exactly, not corroborated but proven.")
    print("  Compact support is load-bearing: with a Morse or LJ tail the")
    print("  third bond would contribute a small negative amount at the")
    print("  crossing and the barrier would sit strictly below eps.")


if __name__ == "__main__":
    main()
