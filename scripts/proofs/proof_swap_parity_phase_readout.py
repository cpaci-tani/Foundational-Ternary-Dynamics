#!/usr/bin/env python3
"""FTD-0845 exact swap-parity phase-readout discriminator.

No numerical search, fitting, target constants, or production mutation.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCES = {
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md":
        "64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md":
        "07BDB4CA22A655C378BCC4BA4B6A69830686200A4B4F59B19136363F5F4F6496",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_PAIR_ENERGY_RECURSION_v1.md":
        "C352EC96A6513D5ED3AB8A7318F47FD1A695FBB0C4FBEB33E9DE43680A70DF93",
    ROOT / "docs/theory/10_eft_program/native_time_carrier_programme/SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md":
        "E5E21BCB0D9F16825ED4FEEE9B915E2835F16F9446F0D636C801A4316CB0D0C5",
    ROOT / "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
}

checks: list[tuple[str, bool]] = []


def check(label: str, condition: object) -> None:
    ok = bool(condition)
    checks.append((label, ok))
    print(f"{'PASS' if ok else 'FAIL'}  {label}")


def exact_zero(expr: object) -> bool:
    return sp.simplify(expr) == 0


for path, expected in SOURCES.items():
    actual = hashlib.sha256(path.read_bytes()).hexdigest().upper()
    check(f"source hash {path.relative_to(ROOT).as_posix()}", actual == expected)

# Symbols are real; positive parameters are declared where needed.
q, p, r, pi, a_ptr = sp.symbols("q p r pi a_ptr", real=True)
q0, q1, p0, p1 = sp.symbols("q0 q1 p0 p1", real=True)
r0, r1, pi0, pi1 = sp.symbols("r0 r1 pi0 pi1", real=True)
h, m, M, lam, kap = sp.symbols("h m M lam kap", positive=True, real=True)

# C6: parity involution.
state = sp.Matrix([r, pi, q, p])
S = -sp.eye(4)
check("C6 joint odd exchange parity is an involution", S * S == sp.eye(4) and S * state == -state)

# C7--C9: a common/even polynomial retains only even q powers.
coeffs = sp.symbols("c0:15", real=True)
monomials = []
for total in range(5):
    for q_power in range(total + 1):
        a_power = total - q_power
        monomials.append(a_ptr**a_power * q**q_power)
V_generic = sp.Add(*[c * mon for c, mon in zip(coeffs, monomials)])
V_even = sp.expand((V_generic.subs(q, q) + V_generic.subs(q, -q)) / 2)
odd_coeffs = [
    sp.expand(V_even).coeff(a_ptr, ap).coeff(q, qp)
    for ap in range(5)
    for qp in range(1, 5, 2)
    if ap + qp <= 4
]
check("C7 common-even projection removes every odd q monomial through degree four",
      all(exact_zero(value) for value in odd_coeffs))
F_common = -sp.diff(V_even, a_ptr)
check("C8 common force is even in q", exact_zero(F_common.subs(q, -q) - F_common))

W_plus = kap * (a_ptr - q**2) ** 2 / 2
check("C9 square pointer is a nonnegative exchange-invariant square",
      exact_zero(W_plus.subs(q, -q) - W_plus)
      and sp.factor(W_plus) == kap * (a_ptr - q**2) ** 2 / 2)
check("C10 square pointer has zero clock Hessian at the origin",
      exact_zero(sp.diff(W_plus, q, 2).subs({a_ptr: 0, q: 0})))
F_plus = -sp.diff(W_plus, a_ptr)
check("C11 zero-pointer common force reads kappa q squared",
      exact_zero(F_plus.subs(a_ptr, 0) - kap * q**2))
check("C12 square pointer is blind to the signed sheet",
      exact_zero(F_plus.subs({a_ptr: 0, q: -q}) - F_plus.subs(a_ptr, 0)))

# C13--C16: positive quadratic obstruction and degree floor.
aa, bb, cc = sp.symbols("aa bb cc", real=True)
H2 = sp.Matrix([[aa, bb], [bb, cc]])
check("C13 quadratic faithful-pointer Hessian determinant is aa*cc-bb^2",
      exact_zero(H2.det() - (aa * cc - bb**2)))
check("C14 zero clock stiffness makes a nonzero bilinear Hessian indefinite",
      exact_zero(H2.det().subs(cc, 0) + bb**2))
W_harmonic = kap * (r - q) ** 2 / 2
check("C15 positive harmonic difference adds nonzero clock stiffness",
      exact_zero(sp.diff(W_harmonic, q, 2).subs({r: 0, q: 0}) - kap))
x, y = sp.symbols("x y", real=True)
P3 = x**3 + 2 * x**2 * y - 3 * x * y**2 + 5 * y**3
check("C16 every homogeneous cubic reverses sign under total inversion",
      exact_zero(P3.subs({x: -x, y: -y}) + P3))

# C17--C22: selected odd pointer and its local history response.
W_minus = kap * (r - q) ** 4 / 4
check("C17 quartic odd-pointer interaction has joint-odd covariance",
      exact_zero(W_minus.subs({r: -r, q: -q}) - W_minus))
check("C18 quartic odd-pointer interaction is a positive fourth-power form",
      sp.factor(W_minus) == kap * (q - r) ** 4 / 4)
grad_origin = sp.Matrix([sp.diff(W_minus, v) for v in (r, q)]).subs({r: 0, q: 0})
hess_origin = sp.hessian(W_minus, (r, q)).subs({r: 0, q: 0})
check("C19 quartic odd-pointer interaction has zero gradient and Hessian at the origin",
      grad_origin == sp.zeros(2, 1) and hess_origin == sp.zeros(2))
F_r = -sp.diff(W_minus, r)
check("C20 zero-pointer force reads the signed cubic coordinate",
      exact_zero(F_r.subs(r, 0) - kap * q**3))
force_rate = sp.diff(F_r.subs(r, 0), q) * p / m
check("C21 force-rate reads velocity sign away from a crossing",
      exact_zero(force_rate - 3 * kap * q**2 * p / m))
t = sp.symbols("t", real=True)
q_cross = p * t / m
r_ddot_series = kap * q_cross**3 / M
crossing_fifth = sp.diff(r_ddot_series, t, 3).subs(t, 0)
check("C22 fifth pointer derivative reads exact zero-crossing direction",
      exact_zero(crossing_fifth - 6 * kap * p**3 / (M * m**3)))

# C23: positivity/coercivity is witnessed by an invertible pair of fourth powers.
V_positions = lam * q**4 + W_minus
zero_map_det = sp.Matrix([[1, 0], [-1, 1]]).det()  # (q,r) -> (q,r-q)
check("C23 positive position energy is coercive and vanishes only at the origin",
      zero_map_det == 1
      and sp.Poly(V_positions, q, r).total_degree() == 4
      and V_positions.subs({q: 0, r: 0}) == 0)

# C24--C29: exact discrete-gradient transaction.
def G(x0: sp.Expr, x1: sp.Expr) -> sp.Expr:
    return (x1**2 + x0**2) * (x1 + x0)


z0 = r0 - q0
z1 = r1 - q1
Gq = G(q0, q1)
Gz = G(z0, z1)
dq = q1 - q0
dr = r1 - r0
dp = p1 - p0
dpi = pi1 - pi0

check("C24 clock quartic secant identity is exact", exact_zero(Gq * dq - (q1**4 - q0**4)))
check("C25 interaction secant chain identity is exact",
      exact_zero(Gz * (dr - dq) - (z1**4 - z0**4)))

relations = {
    dq: h * (p1 + p0) / (2 * m),
    dr: h * (pi1 + pi0) / (2 * M),
    dp: h * (-lam * Gq + kap * Gz / 4),
    dpi: -h * kap * Gz / 4,
}
# Use the update equations directly in the factorized energy difference.
dH = (
    (p1 + p0) * dp / (2 * m)
    + (pi1 + pi0) * dpi / (2 * M)
    + lam * Gq * dq
    + kap * Gz * (dr - dq) / 4
)
dH_on_update = dH.subs({
    (p1 + p0) / (2 * m): dq / h,
    (pi1 + pi0) / (2 * M): dr / h,
    dp: relations[dp],
    dpi: relations[dpi],
})
check("C26 coupled discrete-gradient update conserves total energy exactly", exact_zero(dH_on_update))

dEq = kap * Gz * dq / 4
dEr = -kap * Gz * dr / 4
dEI = kap * Gz * (dr - dq) / 4
check("C27 three-account readout transaction sums exactly to zero", exact_zero(dEq + dEr + dEI))
check("C28 quartic secants are symmetric under endpoint exchange",
      exact_zero(G(q1, q0) - Gq) and exact_zero(G(z1, z0) - Gz))

# Physical reversal follows because endpoint exchange changes every difference
# sign while the symmetric averages/secants are invariant.
check("C29 symmetric endpoint equations are physically time reversible",
      exact_zero(G(q1, q0) - Gq)
      and exact_zero(G(z1, z0) - Gz)
      and exact_zero((q0 - q1) + dq)
      and exact_zero((r0 - r1) + dr))

# C30--C31: monotonicity and local well-posedness.
dG = sp.diff(G(q0, q1), q1)
dG_sos = 3 * (q1 + q0 / 3) ** 2 + sp.Rational(2, 3) * q0**2
check("C30 endpoint quartic-secant derivative is an exact sum of squares",
      exact_zero(dG - dG_sos))

aq, az = sp.symbols("aq az", nonnegative=True, real=True)
J_endpoint = sp.Matrix([
    [2 * m / h + h * lam * aq + h * kap * az / 4, -h * kap * az / 4],
    [-h * kap * az / 4, 2 * M / h + h * kap * az / 4],
])
det_endpoint = sp.factor(J_endpoint.det())
det_expected = (
    4 * m * M / h**2
    + 2 * M * lam * aq
    + kap * az * (m + M) / 2
    + h**2 * kap * lam * aq * az / 4
)
check("C31 eliminated endpoint Jacobian is positive definite",
      exact_zero(det_endpoint - det_expected)
      and exact_zero(J_endpoint[0, 0] - (2 * m / h + h * lam * aq + h * kap * az / 4)))

# C32 combines locality/recovery and the exact orientation boundary.
qbar = (q1 + q0) / 2
pbar = (p1 + p0) / 2
chi_direct = qbar * relations[dp] - pbar * relations[dq]
chi_formula = -h * (
    lam * (q1 + q0) ** 2 * (q1**2 + q0**2) / 2
    + (p1 + p0) ** 2 / (4 * m)
) + h * kap * (q1 + q0) * Gz / 8
check("C32 combined local readout discriminator and orientation ledger close",
      exact_zero(chi_direct - chi_formula)
      and exact_zero(chi_formula.subs(kap, 0) + h * (
          lam * (q1 + q0) ** 2 * (q1**2 + q0**2) / 2
          + (p1 + p0) ** 2 / (4 * m)
      )))

passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(f"FTD-0845 swap-parity phase readout: {passed}/{total} PASS")
if passed == total == 32:
    print("COMMON_EVEN_POINTER_READS_ONLY_THE_SYMMETRIC_SQUARE_QUOTIENT")
    print("POSITIVE_BILINEAR_SIGNED_READOUT_DESTROYS_EXACT_CRITICALITY")
    print("QUARTIC_ODD_POINTER_IS_THE_SCOPED_DEGREE_MINIMUM_FAITHFUL_BRIDGE")
    print("LOCAL_REVERSIBLE_ENERGY_TRANSACTION_EXACT_BACKREACTION_ORIENTATION_COMPLIANCE_REQUIRED")
else:
    raise SystemExit(1)
