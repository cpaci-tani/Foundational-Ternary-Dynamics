"""derive_kink_clock_dilation.py — does the ONE-ENERGY carrier dilate,
and does it dilate UNIVERSALLY?

THE CARRIER.  phi^4 model, U = (lam/4)(phi^2 - v^2)^2, on the M18 axial
lattice.  ONE energy functional; no added potential.  A kink-antikink pair
(topologically consistent on a ring) is stable, and carries a discrete
internal shape mode BELOW the continuum edge -- so band clearance holds by
construction.  Rest-frame frequency (sqrt3/2) m, m = sqrt(2 lam) v.

THE TEST.  Boost the pair.  In the continuum the phi^4 model is exactly
Lorentz invariant, so the boosted kink is the CONTRACTED profile
    phi(x,t) = v tanh( gamma (x - u t) / w ),
    phidot   = -(u gamma v / w) sech^2( gamma (x - u t) / w ),
and its internal clock must satisfy
    Omega(u) = Omega(0) / gamma ,      gamma = 1/sqrt(1 - u^2/c^2),
with the SAME exponent for every lam.  Universality is the whole content
of dilation: the two-category (nodes + static well) model failed exactly
here, giving p in [-2.70, -0.94] instead of -1.

OBSERVABLE.  The shape mode is a width oscillation, so max|d phi/dx| =
v/w_eff is a translation-invariant probe -- necessary because the carrier
is moving and no fixed lattice site stays inside it.

WHAT IS BEING ASKED.  Not whether the lattice is exactly Lorentz invariant
(it is not), but whether the ONE-ENERGY structure restores the universality
that a separately-specified potential destroyed.
"""
from __future__ import annotations

import numpy as np

C = 1.0 / np.sqrt(3.0)
C2 = C * C
N = 4096
X = (np.arange(N) - N // 2).astype(float)
D = 900.0                      # kink-antikink half-separation


def pair(lam, v, u, squeeze=1.0):
    """Boosted, width-squeezed kink-antikink pair and its velocity field."""
    m = np.sqrt(2 * lam) * v
    w = C * np.sqrt(2.0 / lam) / v * squeeze
    g = 1.0 / np.sqrt(1.0 - (u / C) ** 2)
    a, b = g * (X + D) / w, g * (X - D) / w
    phi = v * (np.tanh(a) - np.tanh(b) - 1.0)
    dot = -(u * g * v / w) * (np.cosh(a) ** -2 - np.cosh(b) ** -2)
    return phi, dot, m, w, g


def lap(f):
    return np.roll(f, 1) - 2 * f + np.roll(f, -1)


def evolve(phi, dot, lam, v, T):
    prev = phi - dot                      # leapfrog start, dt = 1
    tr = np.empty(T)
    for t in range(T):
        acc = C2 * lap(phi) - lam * phi * (phi * phi - v * v)
        phi, prev = 2 * phi - prev + acc, phi
        gx = 0.5 * (np.roll(phi, -1) - np.roll(phi, 1))
        tr[t] = np.abs(gx).max()          # translation-invariant probe
    return phi, tr


def freq(tr, drop=0.15):
    """Peak of the FFT of the width oscillation, after a settling window."""
    s = tr[int(len(tr) * drop):]
    s = s - s.mean()
    T = len(s)
    P = np.abs(np.fft.rfft(s * np.hanning(T))) ** 2
    f = np.fft.rfftfreq(T, d=1.0)
    k = np.argmax(P[3:]) + 3
    return 2 * np.pi * f[k]


def main():
    print("Kink clock dilation — does the one-energy carrier dilate "
          "universally?")
    print(f"  C = 1/sqrt3 = {C:.6f},  N = {N},  separation 2d = {2*D:.0f}")

    v = 1.0
    T = 30000
    lams = (0.10, 0.06, 0.16)
    us = (0.0, 0.25, 0.40, 0.50, 0.60)          # in units of C

    print("\n  [1] REST-FRAME CHECK — the probe must recover (sqrt3/2) m")
    for lam in lams:
        phi, dot, m, w, _ = pair(lam, v, 0.0, squeeze=0.93)
        _, tr = evolve(phi, dot, lam, v, T)
        om = freq(tr)
        print(f"      lam={lam:.2f}  m={m:.6f}  measured {om:.6f}  "
              f"predicted {np.sqrt(3)/2*m:.6f}  ratio {om/(np.sqrt(3)/2*m):.4f}")

    print("\n  [2] BOOSTED — Omega(u) against Omega(0)/gamma")
    print(f"      {'lam':>5} {'u/C':>6} {'gamma':>8} {'Omega':>10} "
          f"{'Om(0)/gam':>11} {'ratio':>8} {'p':>8}")
    rows = {}
    for lam in lams:
        om0 = None
        ps = []
        for uc in us:
            u = uc * C
            phi, dot, m, w, g = pair(lam, v, u, squeeze=0.93)
            _, tr = evolve(phi, dot, lam, v, T)
            om = freq(tr)
            if uc == 0.0:
                om0 = om
                print(f"      {lam:5.2f} {uc:6.2f} {g:8.5f} {om:10.6f} "
                      f"{om0:11.6f} {1.0:8.4f} {'--':>8}")
                continue
            p = np.log(om / om0) / np.log(g)
            ps.append(p)
            print(f"      {lam:5.2f} {uc:6.2f} {g:8.5f} {om:10.6f} "
                  f"{om0/g:11.6f} {om/(om0/g):8.4f} {p:8.4f}")
        rows[lam] = np.array(ps)
        print()

    allp = np.concatenate([rows[l] for l in lams])
    means = np.array([rows[l].mean() for l in lams])
    print(f"  [verify] p per lam: " +
          "  ".join(f"lam={l:.2f}: {rows[l].mean():+.4f}" for l in lams))
    print(f"  [verify] spread of the per-lam means = "
          f"{means.max()-means.min():.4f}")
    print(f"  [verify] overall p = {allp.mean():+.4f} +/- {allp.std():.4f}"
          f"   (relativity requires exactly -1)")
    print(f"""
  COMPARISON
    two-category carrier (nodes + static well):  p spanned [-2.70, -0.94],
        spread 1.76 across constituent mass and binding fraction.
    one-energy carrier (phi^4 kink):             p spread across lam is
        {means.max()-means.min():.4f}.""")


if __name__ == "__main__":
    main()
