"""derive_gstar_degeneracy_price.py — what G* is the constant OF.

Four exact statements, each checked here, which together fix an
interpretation of G* that does not require it to be recovered from any
physical carrier (it has not been).

  (1) MINIMALITY.  In the family V = lam q^n, n=2 is the only member with
      nonzero second-order stiffness.  Every n >= 4 is second-order FREE
      and still oscillates.  Quartic is the MINIMAL such case: exactly one
      Taylor coefficient must vanish, where n=6 needs two.  G* is the
      period constant of that minimal degeneracy.

  (2) THE PERIOD IDENTITY.  int_0^1 du/sqrt(1-u^4) = varpi/2 = sqrt(pi)G*/4,
      hence G* = 2 varpi / sqrt(pi).  G* is (up to sqrt(pi)) a period of
      the lemniscatic elliptic curve, the CM curve for Z[i].

  (3) THE PRICE OF STEERABILITY.  At fixed mass, displacement and period,
      E T^2 / (m A^2) = 2 pi^2 for a harmonic mode and (pi/2) G*^2 for a
      quartic one.  The stiffness cancels; the ratio is G*^2/(4 pi).

  (4) THE SMALL-DISPLACEMENT LIMIT.  At displacement a the ratio is
      (a/A)^2 G*^2/(4 pi) -> 0.  A degenerate direction is free to first
      order, so the quartic mode is arbitrarily cheaper to steer.

WHAT THIS IS NOT.  It is not a recovery of G* from a carrier.  The MVC
gave T A = sqrt(pi) G* but is a mechanical framework with a distance
potential, hence Galilean and disqualified as a dynamical carrier; the
phi^4 soliton shape-mode carrier is isochronous, i.e. a pi-clock.  G*
remains recovered as a theorem about lemniscatic integrals only.
"""
from __future__ import annotations

import numpy as np
from scipy.special import gamma as G, beta as B

GS = G(0.25) / G(0.75)                       # G* = Gamma(1/4)/Gamma(3/4)
VARPI = 2.0 * B(0.25, 0.5) / 4.0             # lemniscate constant, = 2 * int


def period_const(n):
    """int_0^1 du / sqrt(1 - u^n) = B(1/n, 1/2) / n."""
    return B(1.0 / n, 0.5) / n


def main():
    print("What G* is the constant of")
    print(f"  G*     = {GS:.12f}   = Gamma(1/4)/Gamma(3/4)")
    print(f"  varpi  = {VARPI:.12f}   (lemniscate constant)")

    print("\n  (1) MINIMALITY in the family V = lam q^n")
    print(f"      {'n':>3} {'2nd-order stiffness':>21} {'T scaling':>12} "
          f"{'vanishing coeffs':>18} {'period const':>15}")
    for n in (2, 4, 6, 8):
        nz = max(0, (n - 2) // 2)
        print(f"      {n:>3} {'NONZERO' if n == 2 else 'zero':>21} "
              f"{'A^%+d' % (1 - n // 2):>12} {nz:>18} "
              f"{period_const(n):>15.9f}")
    print("      => quartic is the minimal potential that is second-order")
    print("         free AND still oscillates: ONE vanishing coefficient.")

    print("\n  (2) THE PERIOD IDENTITY")
    lhs = period_const(4)
    print(f"      int_0^1 du/sqrt(1-u^4) = {lhs:.12f}")
    print(f"      varpi/2                = {VARPI/2:.12f}")
    print(f"      sqrt(pi) G* / 4        = {np.sqrt(np.pi)*GS/4:.12f}")
    assert abs(lhs - VARPI / 2) < 1e-13, "varpi identity"
    assert abs(lhs - np.sqrt(np.pi) * GS / 4) < 1e-13, "G* identity"
    print(f"      => G* = 2 varpi / sqrt(pi) = "
          f"{2*VARPI/np.sqrt(np.pi):.12f}   (check {GS:.12f})")
    assert abs(2 * VARPI / np.sqrt(np.pi) - GS) < 1e-13, "G*-varpi relation"

    print("\n  (3) THE PRICE OF STEERABILITY   (m = A = T = 1)")
    m = A = T = 1.0
    k = 4 * np.pi ** 2 * m / T ** 2                   # harmonic, from T
    lam = np.pi * GS ** 2 * m / (2 * T ** 2)          # quartic, from T A
    Eh, Eq = 0.5 * k * A ** 2, lam * A ** 4
    print(f"      harmonic:  k   = {k:10.6f}   E = {Eh:10.6f}  = 2 pi^2")
    print(f"      quartic :  lam = {lam:10.6f}   E = {Eq:10.6f}  "
          f"= (pi/2) G*^2")
    assert abs(Eh - 2 * np.pi ** 2) < 1e-9, "harmonic E T^2 / m A^2"
    assert abs(Eq - np.pi * GS ** 2 / 2) < 1e-9, "quartic E T^2 / m A^2"
    r = GS ** 2 / (4 * np.pi)
    print(f"      ratio = {Eq/Eh:.9f} = G*^2/(4 pi) = {r:.9f}"
          f"   ({100*(1-r):.1f}% cheaper)")
    assert abs(Eq / Eh - r) < 1e-12, "ratio is not G*^2/4pi"

    print("\n  (4) THE SMALL-DISPLACEMENT LIMIT")
    print(f"      {'a/A':>7} {'E_quartic/E_harmonic':>22} {'times cheaper':>15}")
    for f in (1.0, 0.5, 0.2, 0.1, 0.01):
        q = r * f * f
        print(f"      {f:>7.2f} {q:>22.3e} {1/q:>15.1f}")
    print("      => a degenerate direction is FREE to first order; the")
    print("         quartic mode is arbitrarily cheaper as a -> 0.")

    print(f"""
  READING
    A stiff mode oscillates but resists being moved.  A soft mode moves
    freely but does not oscillate.  The quartic mode is the minimal
    exception -- second-order free AND oscillatory -- and G* is its period
    constant.  So G* is the constant at which timekeeping and steerability
    stop trading against each other, and G*^2/(4 pi) = {r:.4f} is the
    exchange rate.

    SCOPE: this interprets G*; it does not recover it.  No valid carrier
    has produced G*.""")


if __name__ == "__main__":
    main()
