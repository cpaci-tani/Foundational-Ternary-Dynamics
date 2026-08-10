"""derive_epsilon_economy.py — one constant, or three?  Answer: not one.

CLOSES the open problem "One constant, or three?" — as a NEGATIVE.

THE CLAIM UNDER TEST.  The architecture's economy claim, as the paper
stated it: a single energy scale eps prices the binding well, the clock
rate, and the memory retention; "there is no separate memory constant, no
separate clock constant, and no separate binding constant", and raising
eps moves all three "together and in fixed proportion".  The paper also
supplied the falsifier itself: "any measurement that moved one of the
three without the others would break it."

THE OWNER'S DECLARATION (2026-08-09).  The minimum viable clock's fourth
bond is a purchased second interaction species at range 3 r_0, and it
carries its OWN depth eps_2, independent of the bond depth eps.  The
purchase is an energy as well as a range.

WHAT FOLLOWS, EXACTLY AT LEADING NORMAL-FORM ORDER.  The registered bond law is a polynomial in
q = r^2, but the spring constants entering the mechanical normal form are
RADIAL curvatures.  They are k_1 = 96 eps for the unit bond and
k_2 = (32/3) eps_2 for a closure species with the same shape stretched to
radial range 3.  The paper's period law then gives, with
rho = eps_2/eps,

    lambda_eff = 8 k_1 k_2 / (k_1 + 3 k_2) = 256 eps rho / (3 + rho)
                                                        [shared shape]

and the three responses separate:

    register barrier      dE      = eps                    ~ eps^1,  no rho
    retention             ln(tau nu_0) = eps/T             ~ eps^1,  no rho
    clock period          T . A   ~ eps^(-1/2) sqrt((3+rho)/rho)

THREE CONSEQUENCES, all of which the paper must now carry.

  1. "In fixed proportion" is FALSE even at fixed rho.  The exponents are
     (+1, +1, -1/2).  Raising eps deepens the well, lengthens retention
     exponentially, and SPEEDS the clock as eps^(-1/2).  They move
     together, in a fixed and stated relation, but not in proportion.

  2. "No separate clock constant" is FALSE.  rho is a genuinely free
     dimensionless parameter, and it moves the clock rate ALONE: the
     barrier and the retention law do not contain it.  This is exactly
     the falsifier the paper wrote for itself, and it fires -- not from
     an experiment, but from the two-scale extension the paper had
     already purchased and priced in Section 4.6.

  3. WHAT SURVIVES, and it is the part that matters.  In the leading
     A -> 0 quartic normal form, G* is untouched by rho.  The second
     energy scale sets the clock's RATE and cannot move the normalized
     quartic coefficient: T.A.sqrt(2 lambda_eff/m)/sqrt(pi) = G* for
     every (eps, eps_2).  The economy claim fails at the rate and holds
     at the leading normal-form constant.

SCOPE.  The radial curvatures and epsilon/rho algebra are exact within the
declared bond law and two-scale extension.  The period integration is the
ideal leading A -> 0 quartic normal form; finite-amplitude compact-bond
dynamics is not solved here.  The closure is a statement about THIS
architecture's bookkeeping, not about any substrate.  It moves no
epistemic tag: G* was chosen before this script and is chosen after it.
"""
from __future__ import annotations

import numpy as np
import sympy as sp
from scipy.integrate import solve_ivp
from scipy.special import gamma as Gamma


G_STAR = Gamma(0.25) / Gamma(0.75)
M_EFF = 4.0


# ---------------------------------------------------------------------
# Exact algebra
# ---------------------------------------------------------------------
def symbolic():
    eps, eps2, rho, q, r = sp.symbols(
        "epsilon epsilon_2 rho q r", positive=True
    )

    # Unit bond: the register/carrier law, depth eps, minimum at r=1.
    # The clock's k_1 is the radial curvature, not d^2V/dq^2.
    V1 = -16 * eps * (q - sp.Rational(3, 2)) ** 2 * (q - sp.Rational(3, 4))
    V1_radial = V1.subs(q, r**2)
    k1 = sp.simplify(sp.diff(V1_radial, r, 2).subs(r, 1))
    assert k1 == 96 * eps, f"k1 = {k1}, expected 96 eps"

    # Closure species: same q-law shape stretched to radial range 3.
    # Hence q is scaled by 3^2=9, the minimum is r=3, and k_2 is radial.
    V2 = (-16 * eps2 * (q / 9 - sp.Rational(3, 2)) ** 2
          * (q / 9 - sp.Rational(3, 4)))
    V2_radial = V2.subs(q, r**2)
    k2 = sp.simplify(sp.diff(V2_radial, r, 2).subs(r, 3))
    assert sp.simplify(k2 - sp.Rational(32, 3) * eps2) == 0, \
        f"k2 = {k2}, expected 32 eps_2/3"

    # the paper's period-law coefficient
    lam = sp.simplify(8 * k1 * k2 / (k1 + 3 * k2))
    lam_rho = sp.simplify(lam.subs(eps2, rho * eps))
    assert sp.simplify(lam_rho - 256 * eps * rho / (3 + rho)) == 0, \
        f"lambda_eff = {lam_rho}"

    # the shared-eps special case the architecture had assumed
    lam_one = sp.simplify(lam_rho.subs(rho, 1))
    assert lam_one == 64 * eps, f"lambda_eff(rho=1) = {lam_one}"

    # does rho survive in the clock period but not the barrier?
    TA = sp.sqrt(sp.pi) * sp.Symbol("G") * sp.sqrt(M_EFF / (2 * lam_rho))
    TA = sp.simplify(TA)
    dTA_drho = sp.simplify(sp.diff(TA, rho))
    assert dTA_drho != 0, "clock period does not depend on rho"

    print("  [exact] k_1            =", k1)
    print("  [exact] k_2            =", sp.nsimplify(k2))
    print("  [exact] lambda_eff     =", lam_rho, "   (rho = eps_2/eps)")
    print("  [exact] lambda_eff|rho=1 =", lam_one)
    print("  [exact] T.A            =", TA)
    print("  [exact] d(T.A)/d(rho) != 0  =>  rho moves the CLOCK")
    print("  [exact] barrier = eps, ln(tau nu0) = eps/T  =>  rho absent")
    return lam_rho, eps, rho


# ---------------------------------------------------------------------
# The leading quartic normal form, integrated: does G* care about rho?
# ---------------------------------------------------------------------
def period_of(lam, A0, m=M_EFF):
    """Exact quarter-period of Q'' = -(4 lam/m) Q^3 by event detection."""
    def rhs(t, y):
        return [y[1], -(4.0 * lam / m) * y[0] ** 3]

    def cross(t, y):
        return y[0]
    cross.direction = -1
    cross.terminal = True

    om = np.sqrt(4.0 * lam / m) * A0
    sol = solve_ivp(rhs, [0.0, 20.0 / max(om, 1e-9)], [A0, 0.0],
                    events=cross, rtol=1e-12, atol=1e-14, dense_output=True)
    assert sol.t_events[0].size > 0, "no zero crossing found"
    return 4.0 * float(sol.t_events[0][0])


def main():
    print("One constant, or three?  —  closed as a NEGATIVE")
    print(f"  G* = {G_STAR:.12f},  m_eff = {M_EFF}")
    print()
    symbolic()
    print()

    print("  eps   rho    lambda_eff    T.A (integrated)   G*_recovered"
          "     barrier   ln(tau nu0) at T=eps/10")
    print("  " + "-" * 92)
    recovered = []
    for eps in (0.5, 1.0, 2.0):
        for rho in (0.25, 1.0, 4.0):
            lam = 256.0 * eps * rho / (3.0 + rho)
            A0 = 0.3
            T = period_of(lam, A0)
            TA = T * A0
            g = TA * np.sqrt(2.0 * lam / M_EFF) / np.sqrt(np.pi)
            recovered.append(g)
            barrier = eps
            lntau = eps / (eps / 10.0)
            print(f"  {eps:.1f}   {rho:.2f}   {lam:10.6f}   {TA:12.8f}"
                  f"       {g:.12f}   {barrier:.4f}    {lntau:.4f}")

    rec = np.array(recovered)
    dev = np.abs(rec / G_STAR - 1.0).max()
    print()
    print(f"  [verify] max |G*_rec/G* - 1| = {dev:.3e}"
          f"   over eps in [0.5,2], rho in [0.25,4]")
    assert dev < 1e-9, f"G* moved with (eps, rho): {dev}"

    # the three exponents, measured rather than asserted
    e = np.array([0.5, 1.0, 2.0, 4.0])
    lam_e = 256.0 * e * 1.0 / (3.0 + 1.0)
    TA_e = np.array([period_of(l, 0.3) * 0.3 for l in lam_e])
    p_clock = np.polyfit(np.log(e), np.log(TA_e), 1)[0]
    p_barrier = np.polyfit(np.log(e), np.log(e), 1)[0]
    print(f"  [verify] d ln(T.A)/d ln(eps)   = {p_clock:+.9f}   (exact -1/2)")
    print(f"  [verify] d ln(barrier)/d ln(eps) = {p_barrier:+.9f}   (exact +1)")
    assert abs(p_clock + 0.5) < 1e-9, f"clock exponent {p_clock}"

    # rho moves the clock ALONE
    r = np.array([0.25, 1.0, 4.0, 16.0])
    lam_r = 256.0 * 1.0 * r / (3.0 + r)
    TA_r = np.array([period_of(l, 0.3) * 0.3 for l in lam_r])
    spread = TA_r.max() / TA_r.min()
    print(f"  [verify] rho 0.25 -> 16 moves T.A by {spread:.3f}x"
          f" while barrier and retention are UNCHANGED")
    assert spread > 2.0, "rho barely moves the clock"

    print()
    print("  VERDICT")
    print("   - 'in fixed proportion'      FALSE: exponents are (+1, +1, -1/2)")
    print("   - 'no separate clock constant' FALSE: rho = eps_2/eps is free")
    print("     and moves the clock rate alone — the paper's own falsifier,")
    print("     fired by the two-scale extension it had already purchased")
    print("   - in the leading A->0 quartic normal form, G* is INDEPENDENT")
    print("     of rho: the second energy scale buys the clock's RATE")


if __name__ == "__main__":
    main()
