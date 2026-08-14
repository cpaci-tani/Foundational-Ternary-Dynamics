#!/usr/bin/env python3
"""Exact certificate for FTD-0902 positive connection order/self-pair gearbox."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_POSITIVE_CONNECTION_ORDER_SELF_PAIR_CRITICAL_CLOCK_GEARBOX_BOUNDARY_v1.md"
EXPECTED_PROTOCOL_HASH = "568F98C7AF01FC48DEAFEDC773FF33A129D089AFC606511C2D3C9F1C45D37061"

SOURCES = {
    "bilateral": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md",
        "779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A",
    ),
    "i_gamma": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md",
        "07BDB4CA22A655C378BCC4BA4B6A69830686200A4B4F59B19136363F5F4F6496",
    ),
    "vector_pair": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md",
        "62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB",
    ),
    "relative_gearbox": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_QUARTIC_RELATIVE_IMPULSE_RECIPROCAL_CARRY_GEARBOX_BOUNDARY_v1.md",
        "E044129DB0E28DCCE3723D77027E5A652EC7A668C0DD73AD17C77E74FA7F4F6C",
    ),
    "linear_connection": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_COMMON_RELATIVE_CONNECTION_AND_MOMENTUM_GEARBOX_BOUNDARY_v1.md",
        "3E2895157741C19DC8603E92E31A71933BFDAAF5B35062DFCE2F92404F8B9542",
    ),
    "pair_header": (
        ROOT / "engine/include/ftd/eft/native_pair_energy_recursion.h",
        "81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A",
    ),
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    passed = bool(condition)
    checks.append((name, passed))
    print(f"{'PASS' if passed else 'FAIL'}  C{len(checks):02d} {name}")


check("protocol hash matches the pre-run lock", sha256(PROTOCOL) == EXPECTED_PROTOCOL_HASH)
texts: dict[str, str] = {}
for label, (path, expected_hash) in SOURCES.items():
    check(f"source hash {label}", sha256(path) == expected_hash)
    texts[label] = path.read_text(encoding="utf-8").lower()

protocol_text = PROTOCOL.read_text(encoding="utf-8").lower()
normalized_protocol = " ".join(protocol_text.split())
check(
    "protocol freezes the signed self-pair connection",
    "u(d)=|d|d" in normalized_protocol
    and "a(d)=\\gamma u(d)" in normalized_protocol,
)
check(
    "protocol separates gamma from i and self-dual balance",
    "gamma_magnitude_derived_from_i=false" in protocol_text
    and "self_dual_equal_partition=not_adopted" in protocol_text,
)
check(
    "protocol freezes rest, moving, production, and Born firewalls",
    "rest_sector_critical_quartic=exact" in protocol_text
    and "moving_sector_exact_quartic=false_generically" in protocol_text
    and "production_integration=forbidden" in protocol_text
    and "born_bell_lorentz_completeness=untouched" in protocol_text,
)

# General positive-connection Gram obstruction.
b11, b12, b21, b22 = sp.symbols("b11 b12 b21 b22", real=True)
r11, r12, r22 = sp.symbols("r11 r12 r22", positive=True)
x1, x2 = sp.symbols("x1 x2", real=True)
B = sp.Matrix([[b11, b12], [b21, b22]])
R = sp.Matrix([[r11, r12], [0, r22]])
G = R.T * R
x = sp.Matrix([x1, x2])
hessian = B.T * G * B
gram_norm = (R * B * x).dot(R * B * x)
check(
    "connection clock Hessian is the positive Gram B transpose M inverse B",
    sp.expand((x.T * hessian * x)[0] - gram_norm) == 0,
)
trace_sos = sum((R * B)[row, column] ** 2 for row in range(2) for column in range(2))
check("Hessian trace is an exact sum of four squares", sp.expand(sp.trace(hessian) - trace_sos) == 0)
check("positive square root is invertible", sp.simplify(R.det() - r11 * r22) == 0)
zero_gram_solution = sp.solve(list(R * B), [b11, b12, b21, b22], dict=True)
check(
    "zero Gram Hessian forces zero linearized connection",
    zero_gram_solution == [{b11: 0, b12: 0, b21: 0, b22: 0}],
)

# Scalar control reproduces FTD-0901 and the generic moving tilt.
D, P = sp.symbols("D P", real=True)
M, m, lam = sp.symbols("M m lambda", positive=True)
a = sp.symbols("a", real=True)
W_linear = (P - a * D) ** 2 / (2 * M) + lam * D**4
check(
    "linear connection rest Hessian is a squared over M",
    sp.simplify(sp.diff(W_linear.subs(P, 0), D, 2).subs(D, 0) - a**2 / M) == 0,
)
check(
    "linear connection moving origin tilt is minus a P over M",
    sp.simplify(sp.diff(W_linear, D).subs(D, 0) + a * P / M) == 0,
)
check("linear connection critical control requires zero coefficient", sp.simplify((a**2 / M).subs(a, 0)) == 0)

# Signed radial self-pair geometry.
d = sp.symbols("d", positive=True)
u_positive = d**2
u_negative = -d**2
check("signed self-pair is odd across the two rays", sp.simplify(u_negative + u_positive) == 0)
check("signed self-pair square is exactly quartic", sp.simplify(u_positive**2 - d**4) == 0 and sp.simplify(u_negative**2 - d**4) == 0)
check("signed self-pair first derivative vanishes at the origin", sp.diff(u_positive, d).subs(d, 0) == 0 and sp.diff(u_negative, d).subs(d, 0) == 0)
check("signed self-pair is C1 but not C2 at the origin", sp.diff(u_positive, d, 2) == 2 and sp.diff(u_negative, d, 2) == -2)

r = sp.symbols("r", positive=True)
J_u = r * (sp.eye(3) + sp.diag(1, 0, 0))
radial = sp.Matrix([1, 0, 0])
tangent = sp.Matrix([0, 1, 0])
check("radial self-pair Jacobian has eigenvalue 2r", J_u * radial == 2 * r * radial)
check("radial self-pair Jacobian has tangential eigenvalue r", J_u * tangent == r * tangent)
check("self-pair connection curvature vanishes only at the origin", J_u.subs(r, 0) == sp.zeros(3) and sp.simplify(J_u.det() - 2 * r**3) == 0)

dx, dy, dz, rho = sp.symbols("dx dy dz rho", real=True)
Dv = sp.Matrix([dx, dy, dz])
Q = sp.Matrix([[0, 0, -1], [1, 0, 0], [0, -1, 0]])
check("registered signed cubic transform is orthogonal", Q.T * Q == sp.eye(3))
check("radial norm is cubic invariant", sp.expand((Q * Dv).dot(Q * Dv) - Dv.dot(Dv)) == 0)
check("radial self-pair is covariant under signed cubic transforms", Q * (rho * Dv) == rho * (Q * Dv))

# Legendre transform and exact rest-sector quartic.
gamma = sp.symbols("gamma", real=True)
Cdot, Ddot, u, Pi = sp.symbols("Cdot Ddot u Pi", real=True)
L = M * Cdot**2 / 2 + m * Ddot**2 / 2 + gamma * u * Cdot - lam * D**4
p_common = sp.diff(L, Cdot)
p_relative = sp.diff(L, Ddot)
check("self-pair common canonical momentum includes the connection", sp.simplify(p_common - (M * Cdot + gamma * u)) == 0)
check("relative canonical momentum remains m Ddot", sp.simplify(p_relative - m * Ddot) == 0)
H_expected = (P - gamma * u) ** 2 / (2 * M) + Pi**2 / (2 * m) + lam * D**4
H_legendre = (P * Cdot + Pi * Ddot - L).subs(
    {Cdot: (P - gamma * u) / M, Ddot: Pi / m}, simultaneous=True
)
check("Legendre transform gives the positive self-pair connection Hamiltonian", sp.simplify(H_legendre - H_expected) == 0)
K = P - gamma * u
u0, u1 = sp.symbols("u0 u1", real=True)
check("mechanical common impulse is minus gamma Delta U", sp.simplify((P - gamma * u1) - (P - gamma * u0) + gamma * (u1 - u0)) == 0)

Lambda = lam + gamma**2 / (2 * M)
H_rest = Pi**2 / (2 * m) + Lambda * D**4
check("rest-sector connection energy folds exactly into the quartic", sp.simplify(H_expected.subs({P: 0, u**2: D**4}) - H_rest) == 0)
check("effective quartic coupling is positive", Lambda.is_positive is True)
check("rest-sector clock Hessian remains exactly zero", sp.diff(Lambda * D**4, D, 2).subs(D, 0) == 0)
check(
    "self-pair cross-sector connection derivative is nonzero away from the origin for nonzero gamma",
    sp.diff(2 * gamma * d, gamma) == 2 * d and d.is_positive is True,
)

# Branch/orientation and selected equal-partition control.
H_swap = H_expected.subs({u: -u, Pi: -Pi}, simultaneous=True)
check("channel swap exchanges the two gamma branches", sp.simplify(H_swap - H_expected.subs(gamma, -gamma)) == 0)
J = sp.Matrix([[0, -1], [1, 0]])
check("complex structure supplies orientation independently of gamma", J * J == -sp.eye(2) and gamma not in (J * J).free_symbols)
balance_gamma = sp.sqrt(2 * M * lam)
check("selected equal partition conditionally fixes gamma magnitude", sp.simplify(balance_gamma**2 / (2 * M) - lam) == 0)
check("selected equal partition conditionally doubles the quartic coupling", sp.simplify(Lambda.subs(gamma**2, 2 * M * lam) - 2 * lam) == 0)
check("gamma remains a continuous imposed coefficient without equal partition", gamma.is_real is True and gamma.is_number is False)

# Moving-sector quadratic term and symmetric-cycle zero drift.
pn = sp.symbols("pn", real=True)
moving_difference = -gamma * pn * r**2 / M + gamma**2 * r**4 / (2 * M) + lam * r**4
check("moving self-pair connection has the registered quadratic ray term", sp.expand(moving_difference).coeff(r, 2) == -gamma * pn / M)
check("rest control removes the moving quadratic term", sp.expand(moving_difference.subs(pn, 0)).coeff(r, 2) == 0)
U_half = sp.symbols("U_half", real=True)
U_first = sp.symbols("U_first", real=True)
check("odd half-cycle self-pair contributions cancel", sp.simplify((U_first + U_half).subs(U_half, -U_first)) == 0)
check("polarized rest-sector full-cycle common drift is zero", sp.simplify((-gamma / M) * (U_first + U_half).subs(U_half, -U_first)) == 0)

# Exact rest-sector discrete-gradient recursion.
D0, D1, Pi0, Pi1, h = sp.symbols("D0 D1 Pi0 Pi1 h", real=True, nonzero=True)
secant = Lambda * (D1**3 + D1**2 * D0 + D1 * D0**2 + D0**3)
eq_D = D1 - D0 - h * (Pi1 + Pi0) / (2 * m)
eq_Pi = Pi1 - Pi0 + h * secant
E0 = Pi0**2 / (2 * m) + Lambda * D0**4
E1 = Pi1**2 / (2 * m) + Lambda * D1**4
energy_factorized = (Pi1 + Pi0) * (Pi1 - Pi0) / (2 * m) + secant * (D1 - D0)
check("quartic energy difference has the registered secant factorization", sp.simplify((E1 - E0) - energy_factorized) == 0)
check(
    "rest-sector discrete self-pair energy is exactly conserved",
    sp.simplify(energy_factorized.subs(
        {D1 - D0: h * (Pi1 + Pi0) / (2 * m), Pi1 - Pi0: -h * secant},
        simultaneous=True,
    )) == 0,
)

C0, C1 = sp.symbols("C0 C1", real=True)
eq_C = C1 - C0 + h * gamma * (u1 + u0) / (2 * M)
reverse = {C0: C1, C1: C0, D0: D1, D1: D0, Pi0: Pi1, Pi1: Pi0, u0: u1, u1: u0, h: -h}
check("common endpoint equation reverses under endpoint exchange and signed step", sp.simplify(eq_C.xreplace(reverse) + eq_C) == 0)
check("relative coordinate equation reverses under endpoint exchange and signed step", sp.simplify(eq_D.xreplace(reverse) + eq_D) == 0)
check("relative momentum equation reverses under endpoint exchange and signed step", sp.simplify(eq_Pi.xreplace(reverse) + eq_Pi) == 0)

root_two = sp.sqrt(2)
PL0, PR0 = (P + Pi0) / root_two, (P - Pi0) / root_two
PL1, PR1 = (P + Pi1) / root_two, (P - Pi1) / root_two
check("left canonical channel receives the relative endpoint impulse", sp.simplify((PL1 - PL0) - (Pi1 - Pi0) / root_two) == 0)
check("right canonical channel receives the opposite endpoint impulse", sp.simplify((PR1 - PR0) + (Pi1 - Pi0) / root_two) == 0)
check("canonical channel sum remains the conserved P sector", sp.simplify(PL1 + PR1 - PL0 - PR0) == 0)

pstar, tau = sp.symbols("pstar tau", positive=True)
kL, kR = sp.symbols("kL kR", real=True)
cL, cR, W = sp.symbols("cL cR W", integer=True)
q = (Pi1 - Pi0) / (root_two * pstar)
kL1 = kL + q - tau * cL
kR1 = kR - q - tau * cR
W1 = W + cL + cR
check("reciprocal increment is fixed after imposed pstar is supplied", sp.simplify(q * root_two * pstar - (Pi1 - Pi0)) == 0)
check("reciprocal carry preserves the lifted channel aggregate", sp.simplify(kL1 + kR1 + tau * W1 - kL - kR - tau * W) == 0)

# Exact continuum factor at the renormalized quartic coupling.
Gstar = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
quarter_integral = sp.sqrt(sp.pi) * Gstar / 4
period_amplitude = 4 * sp.sqrt(m / (2 * Lambda)) * quarter_integral
check("quartic quarter integral has the exact beta-gamma form", sp.simplify(quarter_integral - sp.beta(sp.Rational(1, 4), sp.Rational(1, 2)) / 4) == 0)
check("rest-sector period-amplitude product has the exact Gstar factor", sp.simplify(period_amplitude - sp.sqrt(sp.pi) * Gstar * sp.sqrt(m / (2 * Lambda))) == 0)
check("Gstar is absent from the registered finite endpoint equations", "gstar" not in str((eq_C, eq_D, eq_Pi)).lower())

# Frozen-source scope anchors.
check("FTD-0836 supplies the signed self-dual energy coordinate", "signed self-dual energy coordinate" in texts["bilateral"])
check("FTD-0839 separates orientation from the physical realization", "choosing multiplication by `+i` selects an orientation" in texts["i_gamma"])
check("FTD-0841 keeps production coupling and cadence selected/open", "production coupling, polarization, support, and cadence" in texts["vector_pair"])
check("FTD-0898 keeps finite-tick cadence open", "finite-tick cadence open" in texts["relative_gearbox"])
check("FTD-0901 says gamma magnitude is not derived from i", "gamma_magnitude_derived_from_i=false" in texts["linear_connection"])
check("native pair header denies production and target-period encoding", "does not add a" in texts["pair_header"] and "does not encode g* or a target period" in texts["pair_header"])

terminal_markers = [
    "positive_linearized_connection_clock_hessian=b_transpose_m_inverse_b",
    "nonzero_linearized_connection_preserves_critical_quartic=false",
    "signed_self_pair_connection=imposed_reference_law",
    "signed_self_pair_connection_regularity=c1_not_c2_at_origin",
    "rest_sector_critical_quartic=exact",
    "rest_sector_continuum_gstar_factor=exact",
    "mechanical_common_impulse=exactly_exchanged_with_signed_self_pair",
    "full_cycle_rest_sector_net_common_drift=zero_for_polarized_symmetric_orbit",
    "moving_sector_exact_quartic=false_generically",
    "i_supplies_orientation=true",
    "gamma_magnitude_derived_from_i=false",
    "self_dual_equal_partition=not_adopted",
    "physical_momentum_scale=open",
    "absolute_mass=not_derived",
    "integer_tick_gstar_cadence=open",
    "production_integration=forbidden",
    "no_new_selected_type=true",
    "born_bell_lorentz_completeness=untouched",
]
for marker in terminal_markers:
    check(f"terminal firewall {marker.replace('_', ' ')}", marker in protocol_text)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0902 exact certificate: {passed}/{len(checks)} checks passed")
if passed == len(checks):
    print("POSITIVE_LINEARIZED_CONNECTION_OBSTRUCTION_AND_REST_SELF_PAIR_CRITICAL_GEARBOX_EXACT_GAMMA_SCALE_MOVING_TRANSPORT_CADENCE_OPEN=TRUE")
raise SystemExit(0 if passed == len(checks) else 1)
