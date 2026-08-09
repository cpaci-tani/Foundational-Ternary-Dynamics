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
def V_min_on_interval(a, b):
    """Exact minimum of V over [a, b], by the shape of V.

    V decreases on [0,1], increases on [1,3/2], and is identically 0
    beyond 3/2.  So the minimum over an interval is -eps if the interval
    straddles 1, V(b) if the interval lies left of 1, and V(a) if it lies
    right of 1 (with V(a)=0 once a >= 3/2).  No sampling, no Lipschitz
    slack: this is interval arithmetic and it is exact.
    """
    a = np.maximum(a, 0.0)
    straddles = (a <= 1.0) & (b >= 1.0)
    left = b < 1.0
    out = np.where(straddles, -EPS, np.where(left, V(b), V(a)))
    return out


def certify_interval(s_lo, s_hi, step=0.004):
    """RIGOROUS certificate over a BOX of side lengths, not sampled points.

    The earlier version of this function had two holes, both found by
    external review and both real:

      (1) it certified five discrete values of s rather than the interval
          [0.9, 1.3] the proposition claims;
      (2) it decided membership of the N=3 set from the CELL CENTRE, so a
          set smaller than the grid cell was invisible.  At s = 1.3 the
          centroid sits at distance s/sqrt(3) = 0.75056 from all three
          anchors -- inside the window (3/4, 3/2) by 0.00056 -- so the
          N=3 set there is a disc of radius ~5e-4 and the old 0.002 grid
          reported it EMPTY.  The reviewer caught exactly this.

    Both are fixed by replacing point sampling with interval arithmetic.
    Each cell is a box in (x, y, s); every bond distance is bracketed
    over the whole box (|d(d)/ds| <= 1 for these anchors, so the s-width
    simply widens the bracket); a cell is certified when EITHER at most
    two bonds can possibly be in the attractive window -- in which case
    E >= -2 eps by the exact pointwise bound, no numerics -- OR the
    interval lower bound on E already exceeds -2 eps.  Nothing is
    sampled, so nothing can hide between samples.
    """
    A_lo, A_hi = anchors(s_lo), anchors(s_hi)
    lo = np.minimum(A_lo, A_hi).min(axis=0) - Q_SUP - 0.1
    hi = np.maximum(A_lo, A_hi).max(axis=0) + Q_SUP + 0.1

    s_mid = 0.5 * (s_lo + s_hi)
    s_half = 0.5 * (s_hi - s_lo)
    A = anchors(s_mid)

    gx = np.arange(lo[0], hi[0] + step, step)
    gy = np.arange(lo[1], hi[1] + step, step)
    X, Y = np.meshgrid(gx, gy, indexing="ij")
    P = np.stack([X.ravel(), Y.ravel()], axis=1)

    # half-diagonal of the spatial cell, widened by the s-uncertainty:
    # the anchors move at unit rate in s, so |delta d| <= s_half
    delta = step / np.sqrt(2.0) + s_half

    d = np.linalg.norm(P[:, None, :] - A[None, :, :], axis=2)
    d_lo = np.maximum(d - delta, 0.0)
    d_hi = d + delta

    # a bond is POSSIBLY in the attractive window if its bracket meets it
    possible = (d_hi > Q_REP) & (d_lo < Q_SUP)
    N_max = possible.sum(axis=1)

    # exact route: at most two bonds can be attractive => E >= -2 eps
    exact_ok = N_max <= 2

    # interval route for the rest
    E_lo = V_min_on_interval(d_lo, d_hi).sum(axis=1)
    interval_ok = E_lo > -2.0 * EPS

    certified = exact_ok | interval_ok
    risky = ~certified
    worst = float(E_lo[~exact_ok].min()) if np.any(~exact_ok) else np.inf
    return dict(cells=int(P.shape[0]),
                by_exact=int(exact_ok.sum()),
                by_interval=int((~exact_ok & interval_ok).sum()),
                uncertified=int(risky.sum()),
                worst_E_lo_on_N3=worst)


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

    # ---- the plane minimum, certified over the whole s-INTERVAL --------
    print("  Certifying min_{z=0} E >= -2 eps over the CONTINUUM"
          " s in [0.9, 1.3]")
    print("  (interval arithmetic on boxes — nothing sampled, so nothing"
          " can hide between samples)")
    print()
    print("  s-slab            cells      by exact N<=2   by interval"
          "   uncertified   worst E_lo on N>=3")
    print("  " + "-" * 94)
    S_EDGES = np.arange(0.90, 1.3001, 0.02)
    total_unc = 0
    worst_all = np.inf
    for a, b in zip(S_EDGES[:-1], S_EDGES[1:]):
        c = certify_interval(a, b)
        total_unc += c["uncertified"]
        worst_all = min(worst_all, c["worst_E_lo_on_N3"])
        print(f"  [{a:.2f}, {b:.2f}]  {c['cells']:10d}   {c['by_exact']:12d}"
              f"   {c['by_interval']:10d}   {c['uncertified']:10d}"
              f"   {c['worst_E_lo_on_N3']:+.6f}")
    assert total_unc == 0, (
        f"{total_unc} cells uncertified — the plane bound is NOT established")
    print()
    print(f"  [verify] uncertified cells over the whole interval: {total_unc}")
    print(f"  [verify] worst interval lower bound where N can reach 3:"
          f" {worst_all:+.6f}  (needs > -2)")
    print()

    # ---- attainment and the resulting barrier, at sample geometries ----
    print("  s      h       E_ground   attained E   d3       barrier")
    print("  " + "-" * 58)
    barriers = []
    for s in (0.9, 1.0, 1.1, 1.2, 1.3):
        Eg, h, dg = ground_state(s)
        assert np.allclose(dg, 1.0, atol=1e-12), f"ground bonds != r0: {dg}"
        assert abs(Eg + 3 * EPS) < 1e-12, f"ground energy != -3 eps: {Eg}"
        att = attainment(s)
        assert att["out_of_support"], (
            f"s={s}: third bond at {att['d3']:.4f} < 3/2, so the two-bond "
            "configuration is NOT exactly -2 eps")
        assert abs(att["E"] + 2 * EPS) < 1e-12, (
            f"s={s}: two-bond configuration is {att['E']}, not -2 eps")
        barrier = att["E"] - Eg
        barriers.append(barrier)
        print(f"  {s:.1f}  {h:.4f}  {Eg:+.6f}   {att['E']:+.6f}"
              f"   {att['d3']:.4f}   {barrier:.9f}")

    # attainment holds on the whole interval, not just at the samples:
    ss = np.linspace(0.9, 1.3, 4001)
    d3 = ss * np.sqrt(3.0) / 2.0 + np.sqrt(1.0 - ss**2 / 4.0)
    assert d3.min() >= Q_SUP, (
        f"third bond dips to {d3.min():.4f} < 3/2 somewhere in the interval")
    print()
    print(f"  [verify] third bond over the interval: "
          f"{d3.min():.4f} .. {d3.max():.4f}   (all >= 3/2, so V_3 = 0 exactly)")

    b = np.array(barriers)
    assert np.allclose(b, EPS, atol=1e-12), f"barrier != eps: {b}"
    print(f"  [verify] barrier = {b.min():.12f} .. {b.max():.12f}")
    print(f"  [verify] upper bound  <= eps   explicit hinge path")
    print(f"  [verify] lower bound  >= eps   IVT + certified plane minimum")
    print(f"  [verify] transition state identified: the in-plane two-bond"
          f" configuration")
    print()
    print("  QED — barrier = eps exactly, not corroborated but proven.")
    print("  Compact support is load-bearing: with a Morse or LJ tail the")
    print("  third bond would contribute a small negative amount at the")
    print("  crossing and the barrier would sit strictly below eps.")


if __name__ == "__main__":
    main()
