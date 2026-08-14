"""Exact certificate for the critical-quartic-clock/CM gearbox.

Run with:
  wsl -d Ubuntu-22.04 sage scripts/proofs/proof_quartic_clock_cm_gearbox.sage

This is an exact derivation certificate, not a numerical search.  The input
is the already-declared critical quartic Hamiltonian.  The script proves that
its dimensionless fixed-energy curve maps over QQ to y^2=x^3-x with the clock
differential and forward orientation preserved.  It does not derive the
quartic Hamiltonian from P1--P5 or identify increasing primes with time.
"""


class Certificate:
    def __init__(self):
        self.count = 0

    def check(self, name, condition):
        if not bool(condition):
            raise AssertionError(name)
        self.count += 1
        print(f"[PASS {self.count:02d}] {name}")


def main():
    cert = Certificate()
    print("Critical quartic clock / CM gearbox exact certificate")

    # Smooth completion of the dimensionless positive-energy shell
    #     C: y^2 = 1 - x^4.
    R0 = PolynomialRing(QQ, "z")
    z = R0.gen()
    C = HyperellipticCurve(1 - z**4)
    cert.check("quartic energy shell has genus one", C.genus() == 1)

    # Work exactly in the function field of C.
    R = PolynomialRing(QQ, names=("x", "y"))
    x, y = R.gens()
    A = R.quotient(R.ideal(y**2 - (1 - x**4)), names=("xb", "yb"))
    xb, yb = A.gens()
    K = A.fraction_field()
    xk, yk = K(xb), K(yb)

    # Direct degree-two map C -> E, with E: v^2=u^3-u.
    # The sign of v is chosen so the Neron differential pulls back to the
    # positive clock-time differential dx/y.
    u = 1 / xk**2
    v = -yk / xk**3
    cert.check("direct map lands on v^2=u^3-u", v**2 == u**3 - u)

    du_dx = -2 / xk**3
    cert.check("Neron differential pulls back as du/(2v)=dx/y",
               du_dx / (2 * v) == 1 / yk)
    cert.check("opposite CM sign reverses the clock differential",
               du_dx / (2 * (-v)) == -1 / yk)

    # The nontrivial deck transformation fixes (u,v), so the map is the
    # expected degree-two quotient after smooth completion.
    sigma_u = 1 / (-xk)**2
    sigma_v = -(-yk) / (-xk)**3
    cert.check("deck involution (x,y)->(-x,-y) fixes u", sigma_u == u)
    cert.check("deck involution (x,y)->(-x,-y) fixes v", sigma_v == v)

    # Exhibit the intermediate rational model E1: Y^2=X^3+4X.  This is
    # birational to C and 2-isogenous to the fixed curve E=32a2.
    X = 2 * (1 + yk) / xk**2
    Y = -4 * (1 + yk) / xk**3
    cert.check("quartic curve maps to Y^2=X^3+4X", Y**2 == X**3 + 4*X)
    cert.check("birational inverse recovers x", -2*X/Y == xk)
    cert.check("birational inverse recovers y", (X**2 - 4)/(X**2 + 4) == yk)

    dX_dx = 2 * ((-2*xk**3/yk) / xk**2 - 2*(1 + yk)/xk**3)
    cert.check("intermediate differential obeys dX/Y=dx/y",
               dX_dx / Y == 1 / yk)

    # The fixed 2-isogeny E1 -> E2 is written explicitly.  Its pullback of
    # du/(2v) is dX/Y, so the composite is exactly the direct clock map.
    S = PolynomialRing(QQ, names=("Xs", "Ys"))
    Xs, Ys = S.gens()
    B = S.quotient(S.ideal(Ys**2 - (Xs**3 + 4*Xs)), names=("Xb", "Yb"))
    Xb, Yb = B.gens()
    L = B.fraction_field()
    XL, YL = L(Xb), L(Yb)
    ui = XL/4 + 1/XL
    vi = YL*(XL**2 - 4)/(8*XL**2)
    cert.check("explicit 2-isogeny lands on v^2=u^3-u",
               vi**2 == ui**3 - ui)
    dui_dX = 1/4 - 1/XL**2
    cert.check("2-isogeny pulls back du/(2v) to dX/Y",
               dui_dX/(2*vi) == 1/YL)
    cert.check("composite is u=1/x^2", X/4 + 1/X == u)
    cert.check("composite is v=-y/x^3", Y*(X**2 - 4)/(8*X**2) == v)

    E1 = EllipticCurve(QQ, [0, 0, 0, 4, 0])
    E2 = EllipticCurve(QQ, [0, 0, 0, -1, 0])
    cert.check("intermediate curve is Cremona 32a1", E1.cremona_label() == "32a1")
    cert.check("target curve is Cremona 32a2", E2.cremona_label() == "32a2")
    cert.check("both curves have conductor 32",
               E1.conductor() == 32 and E2.conductor() == 32)
    cert.check("both curves have j=1728",
               E1.j_invariant() == 1728 and E2.j_invariant() == 1728)
    isogenies = [phi for phi in E1.isogenies_prime_degree(2)
                 if phi.codomain().ainvs() == E2.ainvs()]
    cert.check("32a1 has an exact degree-two isogeny to 32a2",
               len(isogenies) == 1 and isogenies[0].degree() == 2)
    cert.check("isogeny identifies the global L-function",
               E1.is_isogenous(E2))

    # Native orientation of the selected quartic Hamiltonian.  For
    # H=p^2/(2m)+lambda*q^4, Hamilton's equations give
    # Omega((q,p),X_H)=-4*lambda*q^4-p^2/m, strictly negative on E>0.
    P = PolynomialRing(QQ, names=("q", "p", "m", "lam"))
    q, p, m, lam = P.gens()
    qdot = p/m
    pdot = -4*lam*q**3
    swept_area = q*pdot - p*qdot
    cert.check("quartic Hamilton flow has the claimed signed area",
               swept_area == -4*lam*q**4 - p**2/m)
    area_numerator = P(m*swept_area)
    cert.check("signed area polynomial has only negative physical terms",
               area_numerator.monomial_coefficient(m*lam*q**4) == -4
               and area_numerator.monomial_coefficient(p**2) == -1)

    print(f"\nPASS {cert.count}/{cert.count}")
    print("Verdict: QUARTIC_CLOCK_CM_GEARBOX_CONDITIONAL_THEOREM")
    print("Boundary: the quartic law/critical maintenance remains selected/open;")
    print("prime labels are arithmetic places, not successive ontic ticks.")
    return 0


main()
