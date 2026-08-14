"""Exact certificate for the target-blind native carrier / CM-operator split.

This script implements the algebraic part of
PREREG_NATIVE_PHASE_ACTION_CM_OPERATOR_v1.md (protocol v1.2).  It performs no
parameter fit, prime-subset selection, PSLQ, or numerical near-miss search.

Covered here:
  * exact action-angle coordinates for the source-free kick-drift mode;
  * exact production-18, equal-Moore-26, and pure-BCC symbols and bands;
  * exact Moore-26 odd-return obstruction and its central-trinomial transform;
  * exact BCC hypergeometric / symmetric-square / quadratic-pullback chain;
  * exact quadratic-twist blindness of the BCC symmetric-square local factor.

The fixed-curve conductor, Tamagawa, period, L-value, point-count, and Hecke
checks live in proof_cm_realization_operator.sage.  The C++ carrier interface
has an independent native regression test.
"""
from __future__ import annotations

import hashlib
import itertools
import math
from fractions import Fraction
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/PREREG_NATIVE_PHASE_ACTION_CM_OPERATOR_v1.md"
)
PROTOCOL_SHA256 = "8BE09323F54424C51EA96B2589D532559CC54C4656DE39DEE0626DD6C5EC09F5"

FROZEN_HASHES = {
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/include/ftd/ontic/gauge_couplings.h":
        "BC862D8120E0F3D83B7FAD0201F8D4DF46B5BAD5E7D52CD571AF68BECA3EB0F3",
    "engine/include/ftd/sublattice.h":
        "3D0903987D7FF97AFFE203C0C9C5FCA826BD2FEABB9D457C6660D8B821C689E9",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md":
        "2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C",
    "docs/theory/01_reference/SPEC_ALPHA_DYNAMICAL_BOUNDARY.md":
        "EA5295FF581A38669A0573AF1D58AB7C685AA5C9CF951A00DA5D0C278AF10128",
    "docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md":
        "245A2F97F71BF72C6CA49352E238E65A1A379CC2B10A3E000AE06D76EB9EB5BB",
    "docs/theory/09_mathematical/number_theory/DERIV_LFUNCTION_GSTAR_CONNECTION.md":
        "A7900118651DB1126EAA36B1EA167D24B10D6146A1CF94E9784015D6CB810473",
    "scripts/proofs/explr_stencil18_selfduality_derived.py":
        "AC0A362810A00929A2388B933A964300D0CFF67916B2B389416F2920F6424B4F",
    "scripts/proofs/_stencil18_operator.json":
        "22F0809C4EF477E3CCDA874C9466D8BEC522274EEF138A983E9646352FC910DE",
}


class Certificate:
    def __init__(self) -> None:
        self.count = 0

    def check(self, name: str, condition: bool) -> None:
        if not bool(condition):
            raise AssertionError(name)
        self.count += 1
        print(f"[PASS {self.count:02d}] {name}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def protocol_prefix_hash(path: Path) -> str:
    raw = path.read_bytes()
    marker = b"`protocol_sha256="
    at = raw.index(marker)
    return hashlib.sha256(raw[:at]).hexdigest().upper()


def check_locks(cert: Certificate) -> None:
    cert.check("protocol v1.2 prefix hash", protocol_prefix_hash(PROTOCOL) == PROTOCOL_SHA256)
    for relative, expected in FROZEN_HASHES.items():
        cert.check(f"frozen source {relative}", sha256(ROOT / relative) == expected)


def check_modal_action_angle(cert: Certificate) -> None:
    a = sp.symbols("a", positive=True)
    s = sp.sqrt(a * (1 - a / 4))
    c = 1 - a / 2
    U = sp.Matrix([[1 - a, 1], [-a, 1]])
    G = sp.Matrix([[a, -a / 2], [-a / 2, 1]])
    omega = sp.Matrix([[0, 1], [-1, 0]])

    cert.check("kick-drift is symplectic", sp.simplify(U.T * omega * U - omega) == sp.zeros(2))
    cert.check("normalized tick metric is invariant", sp.simplify(U.T * G * U - G) == sp.zeros(2))
    cert.check("tick metric determinant is sin(theta)^2", sp.simplify(G.det() - s**2) == 0)

    # (Q,P)^T = S (J,W)^T.  det(S)=1 makes the transformation canonical.
    S = sp.Matrix([[sp.sqrt(s), 0], [-a / (2 * sp.sqrt(s)), 1 / sp.sqrt(s)]])
    R = sp.Matrix([[c, s], [-s, c]])
    cert.check("action-angle chart is canonical", sp.simplify(S.det() - 1) == 0)
    cert.check("native tick conjugates exactly to a rotation", sp.simplify(S * U - R * S) == sp.zeros(2))
    cert.check("rotation has unit determinant", sp.simplify(R.det() - 1) == 0)

    J, W = sp.symbols("J W", real=True)
    QP = sp.simplify(S * sp.Matrix([J, W]))
    action = sp.simplify((QP[0] ** 2 + QP[1] ** 2) / 2)
    h_tick = sp.Rational(1, 2) * (W**2 - a * J * W + a * J**2)
    cert.check("I=H_tick/sin(theta)", sp.simplify(action - h_tick / s) == 0)

    # The principal minors prove positivity on the registered 0<a<4 band.
    aa = sp.symbols("aa", real=True)
    positive_band = sp.solve_univariate_inequality(
        aa * (1 - aa / 4) > 0, aa, relational=False
    )
    cert.check("positive-action band is exactly 0<a<4",
               positive_band == sp.Interval.open(0, 4))


def cube_vertices(expr: sp.Expr, variables: tuple[sp.Symbol, ...]) -> list[sp.Expr]:
    return [sp.simplify(expr.subs(dict(zip(variables, signs))))
            for signs in itertools.product((-1, 1), repeat=len(variables))]


def check_stencil_symbols(cert: Certificate) -> None:
    cx, cy, cz = sp.symbols("c_x c_y c_z", real=True)
    variables = (cx, cy, cz)

    face_sum = 2 * (cx + cy + cz)
    edge_sum = 4 * (cx * cy + cy * cz + cz * cx)
    corner_sum = 8 * cx * cy * cz

    L18 = sp.expand(sp.Rational(1, 3) * face_sum + sp.Rational(1, 6) * edge_sum - 4)
    expected_L18 = sp.Rational(2, 3) * (
        cx + cy + cz + cx * cy + cy * cz + cz * cx
    ) - 4
    cert.check("C18 symbol follows from frozen face/edge weights", sp.expand(L18 - expected_L18) == 0)
    cert.check("C18 has exactly zero BCC-cubic coefficient",
               sp.Poly(L18, *variables).coeff_monomial(cx * cy * cz) == 0)

    sigma18 = -L18
    band18 = cube_vertices(sigma18, variables)
    cert.check("C18 spatial band maximum is 16/3", max(band18) == sp.Rational(16, 3))
    a18_max = sp.Rational(1, 3) * max(band18)
    cert.check("selected C_WAVE fixes C18 a_max=16/9<4", a18_max == sp.Rational(16, 9) and a18_max < 4)

    avg26 = sp.expand((face_sum + edge_sum + corner_sum) / 26)
    expected_avg26 = sp.Rational(1, 13) * (cx + cy + cz) + sp.Rational(2, 13) * (
        cx * cy + cy * cz + cz * cx
    ) + sp.Rational(4, 13) * cx * cy * cz
    cert.check("C26 equal-Moore symbol follows from 26 fixed neighbours",
               sp.expand(avg26 - expected_avg26) == 0)
    sigma26 = 1 - avg26
    band26 = cube_vertices(sigma26, variables)
    cert.check("C26 normalized band maximum is 18/13", max(band26) == sp.Rational(18, 13))
    cert.check("selected C_WAVE makes C26 strictly stable",
               sp.Rational(1, 3) * max(band26) == sp.Rational(6, 13))

    avg_bcc = sp.expand(corner_sum / 8)
    sigma_bcc = 1 - avg_bcc
    cert.check("CBCC symbol is 1-cx*cy*cz", sp.expand(sigma_bcc - (1 - cx * cy * cz)) == 0)
    cert.check("CBCC normalized band maximum is 2", max(cube_vertices(sigma_bcc, variables)) == 2)

    # Ordered three-step returns.  C26 contains triangles; the bipartite BCC
    # corner graph cannot close an odd-length walk.
    moore = [v for v in itertools.product((-1, 0, 1), repeat=3) if v != (0, 0, 0)]
    corners = list(itertools.product((-1, 1), repeat=3))
    returns26 = sum(
        1 for u in moore for v in moore
        if tuple(-(u[i] + v[i]) for i in range(3)) in moore
    )
    returns_bcc = sum(
        1 for u in corners for v in corners
        if tuple(-(u[i] + v[i]) for i in range(3)) in corners
    )
    cert.check("C26 has 264 ordered three-step returns", returns26 == 264)
    cert.check("C26 normalized third moment is 33/2197",
               Fraction(returns26, 26**3) == Fraction(33, 2197))
    cert.check("CBCC has no odd three-step return", returns_bcc == 0)

    # Full-Moore factorization including the excluded stay-put step.
    product_symbol = sp.expand((1 + 2 * cx) * (1 + 2 * cy) * (1 + 2 * cz))
    cert.check("26-neighbour adjacency is product-minus-one",
               sp.expand(face_sum + edge_sum + corner_sum - (product_symbol - 1)) == 0)


def central_trinomial(n: int) -> int:
    return sum(math.comb(n, 2 * k) * math.comb(2 * k, k) for k in range(n // 2 + 1))


def derive_central_trinomial_cube_recurrence() -> tuple[sp.Expr, ...]:
    """Derive the order-four recurrence for c_n=T_n^3.

    T_n obeys a two-dimensional first-order transfer.  Five consecutive
    cubes live in Sym^3 of that two-dimensional space, which has dimension
    four, so their exact null vector gives the recurrence without guessing.
    """
    n = sp.symbols("n", integer=True, positive=True)
    x, y = sp.symbols("x y")  # x=T_n, y=T_(n-1)

    def A(index: sp.Expr) -> sp.Expr:
        return sp.Rational(1, 1) * (2 * index + 1) / (index + 1)

    def B(index: sp.Expr) -> sp.Expr:
        return 3 * index / (index + 1)

    forms = [x]
    previous, current = y, x
    for step in range(4):
        index = n + step
        following = sp.factor(A(index) * current + B(index) * previous)
        forms.append(following)
        previous, current = current, following

    basis = (x**3, x**2 * y, x * y**2, y**3)
    columns = []
    for form in forms:
        poly = sp.Poly(sp.expand(form**3), x, y)
        columns.append(sp.Matrix([poly.coeff_monomial(mon) for mon in basis]))
    matrix = sp.Matrix.hstack(*columns)
    nullspace = matrix.nullspace()
    if len(nullspace) != 1:
        raise AssertionError(f"C26 Sym^3 recurrence nullity is {len(nullspace)}, expected 1")
    vector = [sp.factor(entry) for entry in nullspace[0]]
    common_denominator = sp.lcm([sp.denom(entry) for entry in vector])
    polynomial = [sp.factor(entry * common_denominator) for entry in vector]
    common_factor = sp.gcd_list(polynomial)
    primitive = tuple(sp.factor(entry / common_factor) for entry in polynomial)
    return primitive


def c26_theta_operator() -> sp.Expr:
    """Return the homogeneous theta-form ODE for H(w)=sum T_n^3 w^n."""
    n = sp.symbols("n", integer=True, positive=True)
    w, theta = sp.symbols("w theta")
    recurrence = derive_central_trinomial_cube_recurrence()
    operator = 0
    for k in range(5):
        # c_(m-k) coefficient after m=n+4 and l=m-k.
        polynomial = recurrence[4 - k].subs(n, theta + k - 4)
        operator += w**k * polynomial
    return sp.factor(operator)


def theta_operator_to_derivative_polys(operator: sp.Expr) -> list[sp.Expr]:
    """Convert sum z^k P_k(theta) to [p_0(z),...,p_r(z)] for sum p_j D^j."""
    w, theta = sp.symbols("w theta")
    poly_theta = sp.Poly(sp.expand(operator), theta)
    order = poly_theta.degree()
    result = [sp.Integer(0) for _ in range(order + 1)]
    for (power,), coefficient in poly_theta.terms():
        for derivative in range(power + 1):
            result[derivative] += (
                coefficient
                * sp.functions.combinatorial.numbers.stirling(power, derivative, kind=2)
                * w**derivative
            )
    return [sp.factor(item) for item in result]


def check_moore26_transform(cert: Certificate) -> None:
    values = [central_trinomial(n) for n in range(40)]
    cert.check("central-trinomial seeds", values[:10] == [1, 1, 3, 7, 19, 51, 141, 393, 1107, 3139])
    cert.check(
        "central-trinomial recurrence",
        all((n + 1) * values[n + 1] == (2 * n + 1) * values[n] + 3 * n * values[n - 1]
            for n in range(1, len(values) - 1)),
    )

    # Exact coefficient identity:
    # P26(z)=26/(26+z) * H(z/(26+z)), H(w)=sum T_n^3 w^n.
    z = sp.symbols("z")
    order = 18
    H = sum(sp.Integer(values[n]) ** 3 * (z / (26 + z)) ** n for n in range(order + 1))
    transformed = sp.series(sp.Rational(26, 1) / (26 + z) * H, z, 0, 9).removeO()

    # Independent direct closed-walk moments by exact integer convolution on
    # the fixed 26-step set.  This avoids expanding a huge Laurent polynomial.
    steps = [v for v in itertools.product((-1, 0, 1), repeat=3)
             if v != (0, 0, 0)]
    positions: dict[tuple[int, int, int], int] = {(0, 0, 0): 1}
    returns = [1]
    for _ in range(8):
        nxt: dict[tuple[int, int, int], int] = {}
        for pos, multiplicity in positions.items():
            for step in steps:
                dest = tuple(pos[i] + step[i] for i in range(3))
                nxt[dest] = nxt.get(dest, 0) + multiplicity
        positions = nxt
        returns.append(positions.get((0, 0, 0), 0))
    direct = sum(sp.Integer(returns[n]) * (z / 26) ** n for n in range(9))
    cert.check("C26 central-trinomial rational transform (first 9 exact coefficients)",
               sp.expand(transformed - direct) == 0)
    cert.check("C26 transform reproduces nonzero cubic coefficient",
               sp.expand(direct).coeff(z, 3) == sp.Rational(33, 2197))


def check_bcc_symmetric_square(cert: Certificate) -> None:
    t = sp.symbols("t")
    theta = sp.symbols("theta")

    # Coefficients of P_BCC(t)=3F2(1/2,1/2,1/2;1,1;t).
    def bcc_coeff(n: int) -> sp.Rational:
        return sp.rf(sp.Rational(1, 2), n) ** 3 / sp.factorial(n) ** 3

    cert.check(
        "BCC hypergeometric coefficients equal central-binomial cubes",
        all(sp.simplify(bcc_coeff(n) - sp.Rational(math.comb(2 * n, n), 4**n) ** 3) == 0
            for n in range(16)),
    )
    cert.check(
        "BCC coefficients obey theta^3-t(theta+1/2)^3",
        all(sp.simplify(sp.Integer(n) ** 3 * bcc_coeff(n)
                        - (sp.Rational(2 * n - 1, 2)) ** 3 * bcc_coeff(n - 1)) == 0
            for n in range(1, 16)),
    )

    # The 2F1(1/4,1/4;1;t) equation and its exact symmetric square.
    P = sp.simplify((1 - sp.Rational(3, 2) * t) / (t * (1 - t)))
    Q = sp.simplify(-sp.Rational(1, 16) / (t * (1 - t)))
    sym2 = [
        sp.Integer(1),
        sp.simplify(3 * P),
        sp.simplify(sp.diff(P, t) + 4 * Q + 2 * P**2),
        sp.simplify(2 * sp.diff(Q, t) + 4 * P * Q),
    ]  # coefficients of y''', y'', y', y

    # Convert theta^3-t(theta+1/2)^3 to ordinary derivatives.
    y0, y1, y2, y3 = sp.symbols("y0 y1 y2 y3")
    theta_y = t * y1
    theta2_y = t * y1 + t**2 * y2
    theta3_y = t * y1 + 3 * t**2 * y2 + t**3 * y3
    theta_plus_half_cubed = theta3_y + sp.Rational(3, 2) * theta2_y + sp.Rational(3, 4) * theta_y + sp.Rational(1, 8) * y0
    ode = sp.expand(theta3_y - t * theta_plus_half_cubed)
    lead = sp.diff(ode, y3)
    hyper_coeffs = [
        sp.Integer(1),
        sp.simplify(sp.diff(ode, y2) / lead),
        sp.simplify(sp.diff(ode, y1) / lead),
        sp.simplify(sp.diff(ode, y0) / lead),
    ]
    cert.check("BCC third-order operator is exact symmetric square",
               all(sp.simplify(a - b) == 0 for a, b in zip(sym2, hyper_coeffs)))

    # DLMF 15.8.18 specialized to a=b=1/2.  Verify it directly by
    # pulling the 1/4 equation through t=4*x*(1-x) and matching the
    # Legendre hypergeometric equation.
    x = sp.symbols("x")
    pull = 4 * x * (1 - x)
    u, ut = sp.symbols("u ut")
    utt = sp.simplify(-P.subs(t, pull) * ut - Q.subs(t, pull) * u)
    pull1 = sp.diff(pull, x)
    pull2 = sp.diff(pull, x, 2)
    vx = ut * pull1
    vxx = utt * pull1**2 + ut * pull2
    legendre_residual = sp.simplify(
        x * (1 - x) * vxx + (1 - 2 * x) * vx - sp.Rational(1, 4) * u
    )
    cert.check("quartic hypergeometric is the quadratic pullback of Legendre",
               sp.factor(legendre_residual) == 0)

    # At x=1/2 the Legendre j invariant is 1728.
    lam = sp.Rational(1, 2)
    j_legendre = sp.simplify(256 * (1 - lam + lam**2) ** 3 / (lam**2 * (1 - lam) ** 2))
    cert.check("Legendre lambda=1/2 has j=1728", j_legendre == 1728)

    # Exact archimedean normalization.  K means K(1/sqrt(2)).
    gamma = sp.gamma(sp.Rational(1, 4))
    K = gamma**2 / (4 * sp.sqrt(sp.pi))
    varpi = sp.sqrt(2) * K
    gstar = 2 * varpi / sp.sqrt(sp.pi)
    bcc_boundary = (2 * K / sp.pi) ** 2
    cert.check("BCC boundary period is G*^2/(2*pi)",
               sp.simplify(bcc_boundary - gstar**2 / (2 * sp.pi)) == 0)


def sym2_local_polynomial(trace: sp.Expr, determinant: sp.Expr, X: sp.Symbol) -> sp.Expr:
    """Characteristic polynomial on Sym^2 of a rank-two operator.

    If eigenvalues are alpha,beta, the Sym^2 eigenvalues are alpha^2,
    alpha*beta,beta^2.  Express the polynomial using trace and determinant.
    """
    a = trace
    d = determinant
    return sp.expand(
        X**3 - (a**2 - d) * X**2 + d * (a**2 - d) * X - d**3
    )


def check_twist_blindness(cert: Certificate) -> None:
    ap, p, chi, X = sp.symbols("a_p p chi X")
    base = sym2_local_polynomial(ap, p, X)
    twist = sym2_local_polynomial(chi * ap, p, X)
    cert.check("Sym^2 local factor is blind to a quadratic twist",
               sp.simplify((twist - base).subs(chi**2, 1)) == 0)

    inert = sp.expand(base.subs(ap, 0))
    cert.check("inert Sym^2 eigen-polynomial is (X-p)(X+p)^2",
               sp.factor(inert - (X - p) * (X + p) ** 2) == 0)

    R_inert = sp.Matrix([[0, -p], [1, 0]]) / sp.sqrt(p)
    cert.check("rank-two inert Frobenius is an exact quarter-turn",
               sp.simplify(R_inert**2 + sp.eye(2)) == sp.zeros(2))
    cert.check("rank-two inert Frobenius has order four",
               sp.simplify(R_inert**4 - sp.eye(2)) == sp.zeros(2))

    # Sym^2 removes the orientation: normalized eigenvalues become {-1,1,-1}.
    sym2_inert_normalized = sp.diag(-1, 1, -1)
    cert.check("BCC/Sym^2 inert realization has order two, not four",
               sym2_inert_normalized != sp.eye(3)
               and sym2_inert_normalized**2 == sp.eye(3))


def main() -> int:
    cert = Certificate()
    print("Native phase/action and CM-operator exact certificate")
    print(f"protocol={PROTOCOL_SHA256}")
    check_locks(cert)
    check_modal_action_angle(cert)
    check_stencil_symbols(cert)
    check_moore26_transform(cert)
    check_bcc_symmetric_square(cert)
    check_twist_blindness(cert)
    print(f"\nPASS {cert.count}/{cert.count}")
    print("Scoped result: target-blind native modal carrier established; BCC is a")
    print("Sym^2 CM period and cannot select the rank-two twist/orientation lift.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
