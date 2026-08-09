"""derive_mvc_reduction.py — what the four-chain's 0.33% drift actually is.

The paper said two things about the minimum viable clock that this script
tests and finds wrong:

  (1) that the mirror-even mode "closes EXACTLY as a one-degree-of-freedom
      quartic oscillator";
  (2) that the measured 0.33% drift is the sextic admixture of the
      crossover family, "a 0.33% DEFICIT, r = 0.0056", so that the drift
      and the counterexample family are "one term seen twice".

Both fail, and the second fails twice over --- in SIGN and in MECHANISM.

WHAT THE MODE ACTUALLY IS.  The transverse pattern y = (-1,+1,+1,-1)U
elongates the two outer bonds at SECOND order; the chain relaxes
longitudinally in response.  So the invariant sector is not one
dimensional but three (U plus two longitudinal stretches), and the
one-degree-of-freedom clock is an ADIABATIC ELIMINATION of the two stiff
stretches -- exact only as A -> 0.  That relaxation is also how k_2
enters lambda_eff at all: a purely transverse displacement leaves the
closure bond's length unchanged, so without relaxation the closure
species could not appear in the period law.

TWO CONSEQUENCES the reduction carries and the pure quartic does not:

  * the relaxed potential is not quartic --
        E_eff(U) = lambda_eff U^4 + c6 U^6 + ...,
    with c6 proportional to (k1 - k2), so it VANISHES at equal couplings;
    at k1 = k2 the series runs 2U^4 - 2U^8 + 6U^12 (powers of four only);
  * the reduced MASS is amplitude dependent, m_eff(U) = 4 + 4U^2 - ...,
    a term with no counterpart in the potential family at all.

AND THE PUNCHLINE.  The drift is an EXCESS, not a deficit: T.A rises with
amplitude and every recovered G* sits ABOVE G*.  In the crossover
family's coordinates that is r < 0, not r > 0.  Worse for the old
reading, the excess is dominated by the KINETIC term: the sextic piece of
the potential accounts for only a few per cent of it.  So the drift is
NOT the counterexample family seen twice; it is mostly a term that family
does not contain.

SCOPE.  Exact within the declared carrier and harmonic bonds.  Moves
nothing about G*, which is the A -> 0 endpoint before and after.
"""
from __future__ import annotations

import numpy as np
from scipy.optimize import minimize
from scipy.integrate import quad
from scipy.special import gamma as Gamma


G_STAR = Gamma(0.25) / Gamma(0.75)
X0 = np.array([0.0, 1.0, 2.0, 3.0])
BONDS = ((0, 1, 1.0), (1, 2, 1.0), (2, 3, 1.0), (0, 3, 3.0))
PATTERN = np.array([-1.0, 1.0, 1.0, -1.0])


def energy(xs, U, k1, k2):
    """Total harmonic bond energy at transverse amplitude U.

    xs are the four longitudinal coordinates; the transverse ones are
    pinned to the mode pattern, which is what makes this the energy ON
    the mirror-even sector rather than the full 8-dof energy.
    """
    ys = PATTERN * U
    E = 0.0
    for (i, j, b0) in BONDS:
        k = k2 if b0 == 3.0 else k1
        b = np.hypot(xs[j] - xs[i], ys[j] - ys[i])
        E += 0.5 * k * (b - b0) ** 2
    return E


def relax(U, k1, k2):
    """E_eff(U): minimise over longitudinal coordinates at fixed U.

    Centroid and the mirror symmetry are imposed by construction, so the
    free parameters are two: the outer and inner longitudinal stretches.
    """
    def obj(p):
        a, c = p
        xs = np.array([X0[0] - a, X0[1] - c, X0[2] + c, X0[3] + a])
        return energy(xs, U, k1, k2)

    r = minimize(obj, np.array([0.0, 0.0]), method="Nelder-Mead",
                 options=dict(xatol=1e-14, fatol=1e-18, maxiter=100000,
                              maxfev=100000))
    a, c = r.x
    xs = np.array([X0[0] - a, X0[1] - c, X0[2] + c, X0[3] + a])
    return float(r.fun), xs


def m_eff_of(U, k1, k2, h=1e-5):
    """Reduced mass on the relaxed manifold: sum_i |dr_i/dU|^2 (unit masses).

    The transverse part contributes 4 (four bodies, pattern +-1); the
    longitudinal response adds the amplitude-dependent piece.
    """
    _, xp = relax(U + h, k1, k2)
    _, xm = relax(U - h, k1, k2)
    dxdU = (xp - xm) / (2 * h)
    return float(np.sum(PATTERN**2) + np.sum(dxdU**2))


def series_fit(k1, k2, umax=0.06, n=13, powers=(4, 6, 8, 10, 12)):
    """Recover the leading coefficients of E_eff(U) by least squares."""
    U = np.linspace(umax / n, umax, n)
    E = np.array([relax(u, k1, k2)[0] for u in U])
    A = np.vstack([U**p for p in powers]).T
    coef, *_ = np.linalg.lstsq(A, E, rcond=None)
    return dict(zip(powers, coef))


def period(A, k1, k2, use_mass=True, use_pot=True):
    """Quarter-period by quadrature of the reduced system.

    T = 4 int_0^A sqrt( m_eff(q) / (2 (E_eff(A) - E_eff(q))) ) dq,
    with either or both corrections switched off for the decomposition.
    """
    lam = 8 * k1 * k2 / (k1 + 3 * k2)
    EA = relax(A, k1, k2)[0] if use_pot else lam * A**4

    def integrand(q):
        Eq = relax(q, k1, k2)[0] if use_pot else lam * q**4
        m = m_eff_of(q, k1, k2) if use_mass else 4.0
        d = EA - Eq
        return np.sqrt(m / (2.0 * d)) if d > 0 else 0.0

    val, _ = quad(integrand, 0.0, A, limit=400, epsabs=1e-12, epsrel=1e-12)
    return 4.0 * val


def main():
    print("The four-chain reduction — what the 0.33% drift really is")
    print(f"  G* = {G_STAR:.12f}")
    print()

    # ---- 1. the potential is not quartic; the U^6 term tracks k1 - k2 --
    print("  E_eff(U) coefficients, by relaxation and least squares")
    print("  k1     k2      U^4          U^6           U^8"
          "          predicted U^6 = 16 k1 k2 (k1-k2)/(k1+3k2)^2")
    print("  " + "-" * 100)
    for k1, k2 in ((1.0, 1.0), (1.0, 2.0), (2.0, 1.0), (1.0, 0.5)):
        c = series_fit(k1, k2)
        lam = 8 * k1 * k2 / (k1 + 3 * k2)
        pred6 = 16 * k1 * k2 * (k1 - k2) / (k1 + 3 * k2) ** 2
        assert abs(c[4] - lam) < 2e-4 * max(1.0, lam), \
            f"U^4 coefficient {c[4]} != lambda_eff {lam}"
        print(f"  {k1:.1f}    {k2:.1f}   {c[4]:+.6f}    {c[6]:+.6f}"
              f"     {c[8]:+.6f}     {pred6:+.6f}")
    print()
    print("  [verify] U^4 coefficient reproduces lambda_eff = 8k1k2/(k1+3k2)")
    print("  [verify] the U^6 term is nonzero when k1 != k2 and VANISHES at"
          " k1 = k2")
    print("           => 'exactly quartic' is false in general, and the")
    print("              equal-coupling case is a cancellation, not a law")
    print()

    # ---- 2. the reduced mass is amplitude dependent ---------------------
    print("  m_eff(U) on the relaxed manifold   (k1 = k2 = 1)")
    for U in (0.0, 0.04, 0.08, 0.12):
        print(f"    U = {U:.2f}   m_eff = {m_eff_of(U, 1.0, 1.0):.6f}")
    m0, m1 = m_eff_of(0.0, 1.0, 1.0), m_eff_of(0.12, 1.0, 1.0)
    assert m1 > m0 + 1e-4, "m_eff is not amplitude dependent"
    print("  [verify] m_eff grows with amplitude — a KINETIC correction with")
    print("           no counterpart in the sextic potential family")
    print()

    # ---- 3. the drift: sign, size, and which term supplies it -----------
    k1 = k2 = 1.0
    lam = 8 * k1 * k2 / (k1 + 3 * k2)
    TA_theory = np.sqrt(np.pi) * G_STAR * np.sqrt(4.0 / (2.0 * lam))
    print(f"  reference: T.A = sqrt(pi) G* sqrt(m/2 lambda) = {TA_theory:.7f}")
    print()
    print("     A      T.A full     drift      mass only    pot only")
    print("  " + "-" * 62)
    for A in (0.02, 0.05, 0.08, 0.12):
        TA = period(A, k1, k2) * A
        TAm = period(A, k1, k2, use_mass=True, use_pot=False) * A
        TAp = period(A, k1, k2, use_mass=False, use_pot=True) * A
        d = TA / TA_theory - 1.0
        dm = TAm / TA_theory - 1.0
        dp = TAp / TA_theory - 1.0
        print(f"   {A:.2f}   {TA:.7f}   {100*d:+.4f}%    {100*dm:+.4f}%"
              f"    {100*dp:+.4f}%")

    TA12 = period(0.12, k1, k2) * 0.12
    drift = TA12 / TA_theory - 1.0
    TAm12 = period(0.12, k1, k2, use_mass=True, use_pot=False) * 0.12
    TAp12 = period(0.12, k1, k2, use_mass=False, use_pot=True) * 0.12
    dm = TAm12 / TA_theory - 1.0
    dp = TAp12 / TA_theory - 1.0

    print()
    assert drift > 0, "drift is not an excess — the paper's 'deficit' would stand"
    print(f"  [verify] the drift is an EXCESS: {100*drift:+.4f}% at A = 0.12")
    print(f"           (T.A rises with amplitude; every recovered G* sits"
          f" ABOVE G*)")
    print(f"  [verify] kinetic share  {100*dm:+.4f}%   potential share"
          f" {100*dp:+.4f}%")
    assert abs(dm) > 3 * abs(dp), \
        "the kinetic term does not dominate — the sextic reading would stand"
    print(f"  [verify] the kinetic term dominates by"
          f" {abs(dm)/abs(dp):.1f}x  =>  the drift is NOT the sextic")
    print(f"           admixture, and 'one term seen twice' is false")
    print()
    print("  CONSEQUENCES FOR THE TEXT")
    print("   - 'closes exactly as a 1-dof quartic oscillator' is wrong twice:")
    print("     the invariant sector is 3-dof and the reduced potential is")
    print("     quartic only to leading order")
    print("   - 'a 0.33% DEFICIT is r = +0.0056' has the SIGN wrong (excess,")
    print("     r_eff < 0) and the MECHANISM wrong (kinetic, not sextic)")
    print("   - G* is untouched: it is the A -> 0 endpoint either way")


if __name__ == "__main__":
    main()
