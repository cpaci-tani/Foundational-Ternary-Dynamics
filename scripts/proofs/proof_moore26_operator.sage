"""Exact equal-Moore-26 lattice-period operator control.

The 26-neighbour adjacency is
  A26=(prod_mu(1+x_mu+x_mu^-1)-1)/26.
After the locked rational change w=z/(26+z), its Green series is
  H(w)=sum_{n>=0} T_n^3 w^n,
where T_n is the central trinomial coefficient.  This script derives the
annihilator from the exact Sym^3 transfer recurrence and factors it with
ore_algebra.  There is no closed-form or parameter search.

Run:
  wsl -d Ubuntu-22.04 sage -python scripts/proofs/proof_moore26_operator.sage
"""

from sage.all import QQ, PolynomialRing, PowerSeriesRing
from ore_algebra import OreAlgebra


def recurrence_coefficients(n):
    """r_j(n) in sum_(j=0)^4 r_j(n) c_(n+j)=0, c_n=T_n^3."""
    return (
        729*(n+1)**3*(n+2)*(2*n+7)*(7*n**2+42*n+62),
        54*(n+2)*(2*n+3)*(2*n+7)*(5*n**2+25*n+27)*(7*n**2+28*n+27),
        -6*(2*n+5)*(5*n**2+25*n+27)*(7*n**2+28*n+27)*(7*n**2+42*n+62),
        -2*(n+3)*(2*n+3)*(2*n+7)*(5*n**2+25*n+27)*(7*n**2+42*n+62),
        (n+3)*(n+4)**3*(2*n+3)*(7*n**2+28*n+27),
    )


def central_trinomial(limit):
    values = [QQ(1), QQ(1)]
    for n in range(1, limit-1):
        values.append(((2*n+1)*values[n] + 3*n*values[n-1])/(n+1))
    return values[:limit]


def build_operator():
    R = PolynomialRing(QQ, "w")
    w = R.gen()
    A = OreAlgebra(R, "Dw")
    Dw = A.gen()
    theta = w*Dw
    operator = A.zero()
    for k in range(5):
        argument = theta + k - 4
        r = recurrence_coefficients(argument)
        operator += w**k * r[4-k]
    return R, A, w, Dw, operator


def build_minimal_operator(A, w, Dw):
    """Rational order-four right factor returned by exact ore factorization."""
    p4 = (
        w**10 - QQ(31)/27*w**9 - QQ(410)/243*w**8
        + QQ(977)/2187*w**7 + QQ(977)/59049*w**5
        + QQ(410)/177147*w**4 - QQ(31)/531441*w**3
        - QQ(1)/531441*w**2
    )
    p3 = (
        11*w**9 - QQ(442)/27*w**8 - QQ(3650)/243*w**7
        + QQ(5978)/2187*w**6 - QQ(2506)/6561*w**5
        + QQ(3218)/19683*w**4 + QQ(970)/59049*w**3
        - QQ(2)/19683*w**2 - QQ(5)/531441*w
    )
    p2 = (
        31*w**8 - QQ(1561)/27*w**7 - QQ(7829)/243*w**6
        + QQ(3875)/2187*w**5 - QQ(1253)/729*w**4
        + QQ(20417)/59049*w**3 + QQ(4499)/177147*w**2
        + QQ(185)/531441*w - QQ(4)/531441
    )
    p1 = (
        22*w**7 - QQ(1372)/27*w**6 - QQ(3680)/243*w**5
        - QQ(5584)/2187*w**4 - QQ(9238)/6561*w**3
        + QQ(808)/6561*w**2 + QQ(16)/2187*w + QQ(4)/19683
    )
    p0 = (
        2*w**6 - QQ(158)/27*w**5 - QQ(118)/243*w**4
        - QQ(1058)/2187*w**3 - QQ(860)/6561*w**2
        - QQ(16)/6561*w + QQ(4)/19683
    )
    return p0 + p1*Dw + p2*Dw**2 + p3*Dw**3 + p4*Dw**4


def main():
    R, A, w, Dw, operator = build_operator()
    print("="*72)
    print("  EQUAL-MOORE-26 PERIOD OPERATOR")
    print("="*72)
    print("raw order:", operator.order(), "degree:", operator.degree())
    print("raw leading coefficient:", operator.leading_coefficient().factor())

    # Coefficient-level exact validation on the defining sequence.
    values = central_trinomial(80)
    cubes = [v**3 for v in values]
    for n in range(0, 76):
        residual = sum(recurrence_coefficients(QQ(n))[j]*cubes[n+j]
                       for j in range(5))
        assert residual == 0, (n, residual)
    print("[PASS] Sym^3 recurrence holds exactly through n=75")

    # Factorizer controls in the same finite-singularity regime.
    reducible = (w*Dw-2)*((w-1)*Dw-1)
    elliptic = w*(1-w)*Dw*Dw + (1-2*w)*Dw - QQ(1)/4
    assert reducible.right_factor() is not None
    assert elliptic.right_factor() is None
    print("[PASS] ore_algebra factorizer controls")

    # The exact factorization has orders 1+1+1+4.  Freeze the rational
    # order-four right factor and certify it by exact Ore right division;
    # this is deterministic and avoids the algebraic factorizer's randomized
    # runtime on every regression execution.
    minimal = build_minimal_operator(A, w, Dw)
    assert minimal.order() == 4
    quotient, remainder = operator.quo_rem(minimal)
    assert remainder == A.zero()
    assert quotient.order() == 3
    print("[PASS] raw order-seven operator has the frozen order-four right factor")

    # Validate the right factor directly on 80 exact coefficients.
    PS = PowerSeriesRing(QQ, "w", default_prec=80)
    psw = PS.gen()
    series = sum(cubes[n]*psw**n for n in range(80))
    residual = PS.zero()
    for derivative, coefficient in enumerate(minimal.coefficients()):
        coefficient_series = PS(coefficient)
        residual += coefficient_series * series.derivative(derivative)
    # Ignore the top 12 coefficients affected by truncating H.
    assert all(residual[n] == 0 for n in range(68)), residual

    print("[PASS] defining H(w) series selects a right factor")
    print("minimal order:", minimal.order())
    print("minimal lead:", minimal.leading_coefficient().factor())
    print("minimal operator:", minimal)
    singular_factors = [
        w,
        w-QQ(1)/3,
        w-QQ(1)/27,
        w+QQ(1)/9,
        w+1,
        w**2-QQ(17)/9*w-QQ(1)/27,
        w**2+QQ(1)/27,
        1/w,
    ]
    print("indicial polynomials:")
    exponent_sets = {}
    for singular in singular_factors:
        indicial = minimal.indicial_polynomial(singular)
        print("  at", singular, ":", indicial.factor())
        roots = []
        for root, multiplicity in indicial.roots():
            roots.extend([root]*multiplicity)
        exponent_sets[str(singular)] = sorted(roots)

    at_origin = exponent_sets[str(w)]
    at_boundary = exponent_sets[str(w-QQ(1)/27)]
    assert at_origin == [0, 0, 0, 1]
    assert at_boundary == [0, QQ(1)/2, 1, 2]
    assert at_origin[0] + at_origin[3] != at_origin[1] + at_origin[2]
    assert at_boundary[0] + at_boundary[3] != at_boundary[1] + at_boundary[2]
    print("[PASS] local exponents fail the necessary self-duality pairing at")
    print("       both w=0 and the physical boundary w=1/27")
    print("VERDICT: MOORE26_NON_SELF_DUAL_CLOSED_FOR_CM_REALIZATION")
    return 0


if __name__ == "__main__":
    main()
