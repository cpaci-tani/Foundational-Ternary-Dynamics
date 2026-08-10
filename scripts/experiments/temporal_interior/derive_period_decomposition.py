"""Where does pi come from, where does G* come from, and what breaks them?

Two exact results and one caveat, all needed by the clock section of the
semantic-ontology paper.  The quadrature and sextic-family numbers come
from here; the separate harmonic-surrogate campaign does not.

1. THE DECOMPOSITION.  The quarter period of a degree-n symmetric
   oscillator V = lambda|x|^n, at amplitude A, is

       T/4 = A sqrt(m/2E) * I(n),   I(n) = int_0^1 du / sqrt(1 - u^n)
       I(n) = (1/n) B(1/n, 1/2) = sqrt(pi) * Gamma(1/n) / (n Gamma(1/n + 1/2))

   Two independent inputs make that constant.  The exponent 1/2 on the
   root -- the QUADRATURE ROOT, present because speed is
   v = sqrt(2(E-V)/m) and therefore present for EVERY n -- contributes
   Gamma(1/2) = sqrt(pi).  The potential's degree contributes the DEGREE
   RATIO Gamma(1/n)/(n Gamma(1/n+1/2)), which is what changes with n.

   n = 2 is the unique SELF-REPRODUCING case: its degree ratio is itself
   sqrt(pi)/2, so I(2)=pi/2 (and the full period coefficient is 2pi for
   m=1, V=x^2/2).  At n=4, I(4)=sqrt(pi)G*/4 (and the corresponding full
   T.A coefficient is sqrt(pi)G*).  This is why the harmonic row contains
   only a power of pi while the quartic row contains the Gamma ratio G*.

   (Terminology note: "surd" is NOT used for sqrt(pi) here.  In this
   repository the surd is delta = sqrt(G*(4G*-1)), the FC-W import of
   FTD-0784 -- a different object.)

2. THE FINITE-AMPLITUDE CAVEAT.  FTD-0794 records a degree-6 family
   F_c = 1 - (1+c)x^4 + c x^6 giving a CONTINUUM of clock constants, and
   it is a genuine counterexample to any claim that a quartic threshold
   fixes G* at finite amplitude.  It is NOT excluded by requiring
   lambda != 0: writing V = lambda q^4 + nu q^6 and r = nu A^2 / lambda,
   the family is exactly this potential with

       c = -r / (1 + r)

   so every member has lambda != 0.  The correction is amplitude, not
   degeneracy, and the proposition needs an A -> 0 clause rather than a
   lambda clause.

   This sextic family is an independent counterexample, not a model of the
   paper's harmonic-spring surrogate.  Positive r produces a deficit in
   T*A, whereas the surrogate reports a +0.33% excess dominated by
   amplitude-dependent effective mass.  The two must not be identified.

Reproduction:
    python scripts/experiments/temporal_interior/derive_period_decomposition.py
"""

import mpmath as mp

mp.mp.dps = 30

GSTAR = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)
SQPI = mp.sqrt(mp.pi)
TOL = mp.mpf(10) ** -13


def I_pure(n):
    """int_0^1 du / sqrt(1 - u^n), by quadrature (endpoint singularity)."""
    return mp.quad(lambda u: 1 / mp.sqrt(1 - u ** n), [0, 1])


def I_closed(n):
    """The same, in closed form: sqrt(pi) Gamma(1/n) / (n Gamma(1/n+1/2))."""
    n = mp.mpf(n)
    return SQPI * mp.gamma(1 / n) / (n * mp.gamma(1 / n + mp.mpf(1) / 2))


def degree_ratio(n):
    n = mp.mpf(n)
    return mp.gamma(1 / n) / (n * mp.gamma(1 / n + mp.mpf(1) / 2))


def T4_of_c(c):
    """FTD-0794's quarter period: int_0^1 dx / sqrt(F_c(x))."""
    c = mp.mpf(c)
    F = lambda x: 1 - (1 + c) * x ** 4 + c * x ** 6
    return mp.quad(lambda x: 1 / mp.sqrt(F(x)), [0, 1])


def J_of_r(r):
    """T*A / (4 sqrt(m/2 lambda)) for V = lambda q^4 + nu q^6, r = nu A^2/lambda."""
    r = mp.mpf(r)
    return mp.quad(lambda u: 1 / mp.sqrt((1 - u ** 4) + r * (1 - u ** 6)), [0, 1])


def main():
    print(__doc__.strip().splitlines()[0])
    print()
    print("  G*    = Gamma(1/4)/Gamma(3/4) = %s" % mp.nstr(GSTAR, 15))
    print("  sqrt(pi)                      = %s" % mp.nstr(SQPI, 15))
    print()

    # ---- 1. the decomposition -------------------------------------------
    print("  1. QUADRATURE ROOT x DEGREE RATIO")
    print()
    print("    n   I(n) quadrature      I(n) closed form     degree ratio")
    print("   " + "-" * 68)
    for n in (2, 3, 4, 5, 6, 8):
        num, cls, dr = I_pure(n), I_closed(n), degree_ratio(n)
        assert abs(num - cls) < TOL, "n=%d: quadrature != closed form" % n
        assert abs(cls - SQPI * dr) < TOL, "n=%d: decomposition failed" % n
        print("   %2d   %s   %s   %s"
              % (n, mp.nstr(num, 15), mp.nstr(cls, 15), mp.nstr(dr, 15)))
    print()

    # n = 2 is self-reproducing; n = 4 gives G*/4.
    dr2, dr4 = degree_ratio(2), degree_ratio(4)
    assert abs(dr2 - SQPI / 2) < TOL, "n=2 is not self-reproducing"
    assert abs(I_closed(2) - mp.pi / 2) < TOL, "n=2 does not give pi/2"
    assert abs(dr4 - GSTAR / 4) < TOL, "n=4 degree ratio is not G*/4"
    assert abs(I_closed(4) - SQPI * GSTAR / 4) < TOL, "n=4 constant wrong"
    print("    [verify] n=2 degree ratio = sqrt(pi)/2 -> SELF-REPRODUCING,")
    print("             so I(2) = sqrt(pi)*sqrt(pi)/2 = pi/2   EXACT")
    print("    [verify] n=4 degree ratio = G*/4, so I(4) = sqrt(pi) G*/4")
    print("             = varpi/2 = %s   EXACT" % mp.nstr(I_closed(4), 15))
    print()

    # The lemniscate cross-check: 2 I(4) = varpi and G* = 2 varpi / sqrt(pi).
    varpi = 2 * I_closed(4)
    assert abs(GSTAR - 2 * varpi / SQPI) < TOL, "G* = 2 varpi/sqrt(pi) failed"
    print("    [verify] varpi = 2 I(4) = %s, G* = 2 varpi/sqrt(pi)  EXACT"
          % mp.nstr(varpi, 15))
    print()

    # ---- 2. FTD-0794's family -------------------------------------------
    print("  2. THE FINITE-AMPLITUDE CAVEAT (FTD-0794's degree-6 family)")
    print()
    print("     F_c = 1 - (1+c)x^4 + c x^6,   G*(c) = 4 T_4(c)/sqrt(pi)")
    print()
    print("       c      quarter period       implied G*        r = -c/(1+c)")
    print("    " + "-" * 68)
    # The five rows of FTD-0794 section 6, to be reproduced not transcribed.
    EXPECTED = {
        "-0.50": ("1.2573392505964", "2.8375108326875"),
        "-0.25": ("1.2825498301619", "2.8944050182316"),
        "0":     ("1.3110287771461", "2.9586751191886"),
        "+0.25": ("1.3436810383880", "3.0323633818750"),
        "+0.50": ("1.3818393432498", "3.1184774543902"),
    }
    # Tolerance note: FTD-0794 publishes 13 decimals.  The quarter periods
    # reproduce to ~4e-14, but the implied G* values differ by ~1.4e-12 at
    # every c EXCEPT c = 0, where agreement is 4e-14.  The exception is the
    # tell: c = 0 is the one row available in closed form, so the residual
    # is round-off in that document's own quadrature, in its last two
    # published digits -- not a disagreement about the family.
    worst_T = worst_G = mp.mpf(0)
    for key, cval in (("-0.50", -0.5), ("-0.25", -0.25), ("0", 0),
                      ("+0.25", 0.25), ("+0.50", 0.5)):
        T4 = T4_of_c(cval)
        Gc = 4 * T4 / SQPI
        r = -mp.mpf(cval) / (1 + mp.mpf(cval))
        want_T, want_G = (mp.mpf(s) for s in EXPECTED[key])
        worst_T = max(worst_T, abs(T4 - want_T))
        worst_G = max(worst_G, abs(Gc - want_G))
        assert abs(T4 - want_T) < mp.mpf(10) ** -12, "T_4(%s) mismatch" % key
        assert abs(Gc - want_G) < mp.mpf(10) ** -10, "G*(%s) mismatch" % key
        print("    %6s   %s   %s   %s"
              % (key, mp.nstr(T4, 14), mp.nstr(Gc, 14), mp.nstr(r, 8)))
    assert abs(4 * T4_of_c(0) / SQPI - GSTAR) < TOL, "c=0 must be exactly G*"
    print()
    print("    [verify] all five FTD-0794 rows reproduced: quarter periods to")
    print("             %s, implied G* to %s"
          % (mp.nstr(worst_T, 3), mp.nstr(worst_G, 3)))
    print("    [verify] c = 0 recovers G* exactly (%s), which locates the"
          % mp.nstr(abs(4 * T4_of_c(0) / SQPI - GSTAR), 3))
    print("             1e-12 residual in that document's quadrature rather")
    print("             than in the family")

    # The mapping: F_c IS V = lambda q^4 + nu q^6 with r = nu A^2/lambda.
    # (E - V)/E at x = Au is [(1-u^4) + r(1-u^6)]/(1+r), and matching the
    # x^4 coefficient gives c = -r/(1+r).  Check both directions.
    for r_val in (0, mp.mpf(1) / 4, 1, 3):
        c_val = -mp.mpf(r_val) / (1 + mp.mpf(r_val))
        lhs = T4_of_c(c_val)
        rhs = J_of_r(r_val) * mp.sqrt(1 + mp.mpf(r_val))
        assert abs(lhs - rhs) < TOL, "c <-> r mapping failed at r=%s" % r_val
    print("    [verify] c = -r/(1+r) maps the family onto V = lam q^4 + nu q^6")
    print("             for r = 0, 1/4, 1, 3   (T_4(c) = sqrt(1+r) J(r))")
    print()

    # ---- 3. the physical drift ------------------------------------------
    print("  3. WHAT THE DRIFT DOES TO T*A")
    print()
    print("     T*A = 4 sqrt(m/2 lambda) J(r),  r = nu A^2 / lambda")
    print()
    print("        r        4 J(r)          relative to r=0")
    print("    " + "-" * 52)
    J0 = J_of_r(0)
    assert abs(4 * J0 - SQPI * GSTAR) < TOL, "r=0 must give sqrt(pi) G*"
    for r_val in (0, mp.mpf("0.01"), mp.mpf("0.10"), 1):
        J = J_of_r(r_val)
        print("    %7s    %s    %s"
              % (mp.nstr(mp.mpf(r_val), 5), mp.nstr(4 * J, 12),
                 mp.nstr(J / J0 - 1, 6)))
    print()
    print("    [verify] r = 0 gives exactly sqrt(pi) G* = %s"
          % mp.nstr(SQPI * GSTAR, 15))

    print("    [scope] positive sextic r gives a T*A deficit; the MVC")
    print("            surrogate's +0.33% excess has a different sign and")
    print("            is dominated by amplitude-dependent effective mass.")
    print()

    # The pure sextic, i.e. what lambda = 0 actually gives (a single
    # constant, NOT the continuum -- worth stating so the two failure
    # modes are not conflated).
    sextic = 4 * I_closed(6) / SQPI
    assert abs(sextic - mp.mpf("2.7404387952294")) < mp.mpf(10) ** -12
    print("    [verify] lambda = 0 (pure sextic) gives ONE constant,")
    print("             4 I(6)/sqrt(pi) = %s -- a different failure"
          % mp.nstr(sextic, 14))
    print("             mode from the continuum above.")
    print()
    print("  RESULT: the decomposition is exact; the degree-6 continuum is a")
    print("          finite-amplitude effect with lambda != 0 throughout, so")
    print("          the forcing statement needs an A -> 0 clause.")


if __name__ == "__main__":
    main()
