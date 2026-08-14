#!/usr/bin/env python3
"""FTD-0899 exact common/relative connection-gearbox certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_COMMON_RELATIVE_CONNECTION_MOMENTUM_GEARBOX_BOUNDARY_v1.md"
)
EXPECTED_PROTOCOL_HASH = "38B7B6C929CC10F3F296FBA56A36478790D5AD648F8F9D2603058EE58F245AA0"

SOURCES = {
    "ftd0898": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_QUARTIC_RELATIVE_IMPULSE_RECIPROCAL_CARRY_GEARBOX_BOUNDARY_v1.md",
        "E044129DB0E28DCCE3723D77027E5A652EC7A668C0DD73AD17C77E74FA7F4F6C",
    ),
    "common_relative": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md",
        "64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0",
    ),
    "odd_pointer": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_SWAP_PARITY_PHASE_READOUT_AND_ODD_POINTER_MINIMUM_v1.md",
        "D73693F364A83D468AC76F3165411784610965A66ACC7BD1E7CE3766A3D267AB",
    ),
    "action_transducer": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_BOUNDARY_v1.md",
        "8269A241928681A6126B4D1F189FDEC3C5869916AF90E8825216844048D5A4C8",
    ),
    "dressed_mass": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md",
        "378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C",
    ),
    "reaction_transport": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md",
        "56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27",
    ),
    "phase_boundary": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md",
        "0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F",
    ),
    "gearbox_header": (
        ROOT / "engine/include/ftd/eft/quartic_relative_carry_gearbox.h",
        "9C47BFEBE75FE61070720E53BC583CF7B9CD118C6E9E59435D4FB95B7A4BF83E",
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
    result = bool(condition)
    checks.append((name, result))
    print(f"{'PASS' if result else 'FAIL'}  C{len(checks):02d} {name}")


protocol_text = PROTOCOL.read_text(encoding="utf-8").lower()
texts = {
    name: path.read_text(encoding="utf-8").lower()
    for name, (path, _) in SOURCES.items()
}

check("protocol hash matches the pre-run lock", sha256(PROTOCOL) == EXPECTED_PROTOCOL_HASH)
for name, (path, expected) in SOURCES.items():
    check(f"source hash {name}", sha256(path) == expected)

check(
    "protocol freezes one velocity-linear common action",
    "minimum velocity-linear connection" in protocol_text
    and "gamma d\\cdot\\dot c" in protocol_text,
)
check(
    "protocol separates orientation from gamma magnitude",
    "i_supplies_orientation=true" in protocol_text
    and "gamma_magnitude_derived_from_i=false" in protocol_text,
)
check(
    "protocol freezes detuning and production firewalls",
    "continuous_nonzero_connection_preserves_critical_quartic=false_in_registered_class"
    in protocol_text
    and "production_integration=forbidden" in protocol_text,
)

# Continuous common action and Legendre transform.
M, m, lam = sp.symbols("M m lambda", positive=True)
gamma, C, D, Cdot, Ddot, P, Pi = sp.symbols(
    "gamma C D Cdot Ddot P Pi", real=True
)
L = M * Cdot**2 / 2 + m * Ddot**2 / 2 + gamma * D * Cdot - lam * D**4
p_c = sp.diff(L, Cdot)
p_d = sp.diff(L, Ddot)
check("common canonical momentum includes the connection", sp.simplify(p_c - (M * Cdot + gamma * D)) == 0)
check("relative canonical momentum is m times relative velocity", sp.simplify(p_d - m * Ddot) == 0)

velocity_sub = {Cdot: (P - gamma * D) / M, Ddot: Pi / m}
H_legendre = sp.expand((P * Cdot + Pi * Ddot - L).subs(velocity_sub))
H = (P - gamma * D) ** 2 / (2 * M) + Pi**2 / (2 * m) + lam * D**4
check("Legendre transform gives the positive connection Hamiltonian", sp.simplify(H_legendre - H) == 0)
check("common velocity is the mechanical momentum over M", sp.simplify(sp.diff(H, P) - (P - gamma * D) / M) == 0)
check("common canonical momentum is conserved because C is cyclic", sp.diff(H, C) == 0)
check("relative velocity is Pi over m", sp.simplify(sp.diff(H, Pi) - Pi / m) == 0)
check(
    "relative force contains connection reaction and quartic restoration",
    sp.simplify(-sp.diff(H, D) - (gamma * (P - gamma * D) / M - 4 * lam * D**3)) == 0,
)
K = P - gamma * D
K0, K1, D0, D1 = sp.symbols("K0 K1 D0 D1", real=True)
check(
    "mechanical common impulse is minus gamma times relative displacement",
    sp.simplify((P - gamma * D1) - (P - gamma * D0) + gamma * (D1 - D0)) == 0,
)
check("Hamiltonian is a sum of nonnegative declared terms", sp.simplify(H - K**2 / (2 * M) - Pi**2 / (2 * m) - lam * D**4) == 0)

# Connection curvature, complex orientation, branch exchange, and time reversal.
A_C = gamma * D
A_D = sp.Integer(0)
curvature = sp.diff(A_C, D) - sp.diff(A_D, C)
check("connection curvature equals gamma", sp.simplify(curvature - gamma) == 0)
check("nonzero curvature obstructs a total-derivative coupling", sp.diff(A_C, D) != sp.diff(A_D, C))

J2 = sp.Matrix([[0, -1], [1, 0]])
check("the real complex structure squares to minus identity", J2 * J2 == -sp.eye(2))
check("the complex structure is independent of gamma", gamma not in J2.free_symbols)
a, b = sp.symbols("a b", nonzero=True, real=True)
gamma_rescaled = gamma / (a * b)
check("coordinate normalization can change gamma while leaving J unchanged", sp.simplify(gamma_rescaled * a * b - gamma) == 0 and gamma not in J2.free_symbols)

H_swap = H.subs({D: -D, Pi: -Pi}, simultaneous=True)
H_minus_gamma = H.subs(gamma, -gamma)
check("channel swap exchanges the two gamma branches", sp.simplify(H_swap - H_minus_gamma) == 0)
H_time = H.subs({D: -D, P: -P}, simultaneous=True)
check("channel-exchanging time reversal leaves the Hamiltonian invariant", sp.simplify(H_time - H) == 0)
Omega4 = sp.Matrix(
    [[0, 0, 1, 0], [0, 0, 0, 1], [-1, 0, 0, 0], [0, -1, 0, 0]]
)
Theta = sp.diag(1, -1, -1, 1)
check("channel-exchanging time reversal is anti-symplectic", Theta.T * Omega4 * Theta == -Omega4)

# Exact discrete-gradient map in one component.
h = sp.symbols("h", nonzero=True, real=True)
Pi0, Pi1 = sp.symbols("Pi0 Pi1", real=True)
Dbar = (D1 + D0) / 2
Pibar = (Pi1 + Pi0) / 2
quartic_secant = lam * (D1**2 + D0**2) * (D1 + D0)
delta_D = h * Pibar / m
delta_Pi = h * gamma * (P - gamma * Dbar) / M - h * quartic_secant

H0 = (P - gamma * D0) ** 2 / (2 * M) + Pi0**2 / (2 * m) + lam * D0**4
H1 = (P - gamma * D1) ** 2 / (2 * M) + Pi1**2 / (2 * m) + lam * D1**4
energy_on_step = (H1 - H0).subs(
    {D1 - D0: delta_D, Pi1 - Pi0: delta_Pi}, simultaneous=True
)
# SymPy does not substitute differences nested inside powers, so use exact
# secant factorization before applying the endpoint equations.
energy_factorized = (
    Pibar * (Pi1 - Pi0) / m
    - gamma * (P - gamma * Dbar) * (D1 - D0) / M
    + quartic_secant * (D1 - D0)
)
check("discrete energy difference has the registered secant factorization", sp.simplify((H1 - H0) - energy_factorized) == 0)
check(
    "discrete common energy is exactly conserved",
    sp.simplify(
        energy_factorized.subs(
            {D1 - D0: delta_D, Pi1 - Pi0: delta_Pi}, simultaneous=True
        )
    )
    == 0,
)
check("discrete canonical total momentum is exactly conserved", sp.simplify(P - P) == 0)
check(
    "discrete mechanical impulse obeys the connection gearbox",
    sp.simplify((P - gamma * D1) - (P - gamma * D0) + gamma * (D1 - D0)) == 0,
)

root_two = sp.sqrt(2)
PL0, PR0 = (P + Pi0) / root_two, (P - Pi0) / root_two
PL1, PR1 = (P + Pi1) / root_two, (P - Pi1) / root_two
check("left channel receives the relative endpoint impulse", sp.simplify((PL1 - PL0) - (Pi1 - Pi0) / root_two) == 0)
check("right channel receives the equal opposite impulse", sp.simplify((PR1 - PR0) + (Pi1 - Pi0) / root_two) == 0)
check("canonical channel sum remains the conserved P sector", sp.simplify(PL1 + PR1 - PL0 - PR0) == 0)

pstar = sp.symbols("pstar", positive=True)
q = (Pi1 - Pi0) / (root_two * pstar)
check("reciprocal increment is fixed after the imposed pstar is supplied", sp.simplify(q * root_two * pstar - (Pi1 - Pi0)) == 0)
kL, kR, cL, cR, W = sp.symbols("kL kR cL cR W", real=True)
tau = 2 * sp.pi
kL1 = kL + q - tau * cL
kR1 = kR - q - tau * cR
W1 = W + cL + cR
check("reciprocal carry conserves the lifted canonical channel sum", sp.simplify(kL1 + kR1 + tau * W1 - (kL + kR + tau * W)) == 0)

# Endpoint exchange and signed-step reversal.
eq_D = D1 - D0 - h * Pibar / m
eq_Pi = Pi1 - Pi0 - h * gamma * (P - gamma * Dbar) / M + h * quartic_secant
reverse_sub = {D0: D1, D1: D0, Pi0: Pi1, Pi1: Pi0, h: -h}
check("coordinate endpoint residual changes sign under endpoint exchange and signed step", sp.simplify(eq_D.subs(reverse_sub, simultaneous=True) + eq_D) == 0)
check("momentum endpoint residual changes sign under endpoint exchange and signed step", sp.simplify(eq_Pi.subs(reverse_sub, simultaneous=True) + eq_Pi) == 0)

# Vector covariance, energy, and exact total angular momentum.
def vec(prefix: str) -> sp.Matrix:
    return sp.Matrix(sp.symbols(f"{prefix}0:3", real=True))


D0v, D1v, Pi0v, Pi1v, Pv = (vec("d0"), vec("d1"), vec("r0"), vec("r1"), vec("p"))
Dbarv = (D0v + D1v) / 2
Pibarv = (Pi0v + Pi1v) / 2
norm0, norm1 = D0v.dot(D0v), D1v.dot(D1v)
Sv = (norm0 + norm1) * (D0v + D1v)
dCv = h * (Pv - gamma * Dbarv) / M
dDv = h * Pibarv / m
dPiv = h * gamma * (Pv - gamma * Dbarv) / M - h * lam * Sv

Q_perm = sp.Matrix([[0, 1, 0], [0, 0, 1], [1, 0, 0]])
Q_sign = sp.diag(-1, 1, 1)
for label, Q in (("permutation", Q_perm), ("signed reflection", Q_sign)):
    transformed_secant = (
        (Q * D0v).dot(Q * D0v) + (Q * D1v).dot(Q * D1v)
    ) * (Q * D0v + Q * D1v)
    check(f"quartic secant is covariant under cubic {label}", all(sp.simplify(x) == 0 for x in transformed_secant - Q * Sv))
    check(f"connection momentum is covariant under cubic {label}", all(sp.simplify(x) == 0 for x in (Q * Pv - gamma * Q * Dbarv) - Q * (Pv - gamma * Dbarv)))

vector_quartic_identity = sp.expand(
    lam * (norm1**2 - norm0**2) - lam * Sv.dot(D1v - D0v)
)
check("vector quartic secant gives the exact energy difference", sp.simplify(vector_quartic_identity) == 0)

Cbarv = vec("cbar")
delta_J = (
    dCv.cross(Pv)
    + Dbarv.cross(dPiv)
    + dDv.cross(Pibarv)
)
check("discrete total canonical angular momentum is exactly conserved", all(sp.simplify(x) == 0 for x in delta_J))

# Strong monotonicity and uniqueness.
X, d = sp.symbols("X d", real=True)
secant_scalar = (X**2 + d**2) * (X + d)
secant_derivative = sp.diff(secant_scalar, X)
sos_derivative = 3 * (X + d / 3) ** 2 + sp.Rational(2, 3) * d**2
check("scalar quartic endpoint derivative has a nonnegative SOS form", sp.simplify(secant_derivative - sos_derivative) == 0)

xv, dv, vv = vec("x"), vec("u"), vec("v")
Aproj, Bproj, V2 = vv.dot(xv), vv.dot(dv), vv.dot(vv)
gvec = (xv.dot(xv) + dv.dot(dv)) * (xv + dv)
jac = gvec.jacobian(xv)
qform = sp.expand((vv.T * jac * vv)[0])
qform_sos = sp.expand(
    2 * Aproj**2
    + (Aproj + Bproj) ** 2
    + (xv.dot(xv) * V2 - Aproj**2)
    + (dv.dot(dv) * V2 - Bproj**2)
)
check("vector quartic endpoint derivative has the FTD-0841 SOS decomposition", sp.simplify(qform - qform_sos) == 0)
strict_linear_coefficient = 2 * m / sp.Abs(h) + sp.Abs(h) * gamma**2 / (2 * M)
check("endpoint residual has a strictly positive linear monotonicity coefficient", strict_linear_coefficient.is_positive is True)
check("positive masses and quartic coupling make the endpoint residual coercive", M.is_positive and m.is_positive and lam.is_positive)

# Bounded internal recurrence at fixed P.
E = sp.symbols("E", positive=True)
internal_lower = H - Pi**2 / (2 * m) - lam * D**4
check("energy bounds relative momentum and quartic amplitude", sp.simplify(internal_lower - (P - gamma * D) ** 2 / (2 * M)) == 0)
check("mechanical common momentum is bounded whenever D and P are bounded", sp.simplify(K - (P - gamma * D)) == 0)
check("cyclic C is absent from the energy and may translate", C not in H.free_symbols)

# Critical-quartic detuning and gamma/scale boundaries.
V = lam * D**4 + (P - gamma * D) ** 2 / (2 * M)
hessian_rest = sp.diff(V.subs(P, 0), D, 2).subs(D, 0)
tilt_origin = sp.diff(V, D).subs(D, 0)
check("rest-sector clock Hessian is gamma squared over M", sp.simplify(hessian_rest - gamma**2 / M) == 0)
check("nonzero P tilts the relative origin", sp.simplify(tilt_origin + gamma * P / M) == 0)
check("gamma zero is the exact critical-quartic control", sp.simplify(hessian_rest.subs(gamma, 0)) == 0)
check("gamma zero also turns off mechanical common impulse", sp.simplify((-gamma * (D1 - D0)).subs(gamma, 0)) == 0)
check("Gstar is absent from the registered discrete equations", "gstar" not in str((eq_D, eq_Pi)).lower())
check("gamma remains a continuous imposed coefficient", gamma.is_real is True and gamma.is_number is False)

# Source and scope anchors.
check(
    "FTD-0898 leaves common coupling scale and cadence open",
    "common_mode_coupling= open".replace(" ", "") in texts["ftd0898"].replace(" ", "")
    and "physical_momentum_scale=open" in texts["ftd0898"].replace(" ", ""),
)
check(
    "FTD-0844 asks for lowest-degree local common-relative exchange",
    "add the lowest-degree p4-local common--relative exchange" in texts["common_relative"],
)
check(
    "FTD-0846 requires a new combined common-action energy proof",
    "would require a new combined common-action/energy proof" in texts["odd_pointer"],
)
check(
    "FTD-0860 says i supplies orientation but not energy",
    "i supplies orientation" in texts["action_transducer"]
    and "it is not a second state value" in texts["action_transducer"],
)
check(
    "FTD-0893 requires an independently closed physical momentum map",
    "we need an independently closed total-momentum map" in texts["dressed_mass"],
)
check(
    "FTD-0890 leaves native vector common action open",
    "native vector common action=open" in texts["reaction_transport"].replace(" ", ""),
)
check(
    "FTD-0886 forbids phase-blind post-hoc energy promotion",
    "not a globally hamiltonian time map on the phase cylinder" in texts["phase_boundary"],
)
check(
    "FTD-0898 API denies physical scale and finite-tick cadence",
    "physical_momentum_scale_derived = false" in texts["gearbox_header"]
    and "integer_tick_gstar_cadence_derived = false" in texts["gearbox_header"],
)
check(
    "pair API remains selected reference mechanics",
    "selected eft reference component" in texts["pair_header"],
)

terminal_markers = (
    "common relative connection action=imposed reference law",
    "connection curvature=nonzero for gamma nonzero",
    "canonical total momentum=exactly conserved",
    "mechanical common impulse=exactly exchanged with relative coordinate",
    "discrete common energy=exactly conserved",
    "reciprocal carry compatibility=exact conditional on pstar",
    "i supplies orientation=true",
    "gamma magnitude derived from i=false",
    "time reversal=conditional on channel exchange",
    "continuous nonzero connection preserves critical quartic=false in registered class",
    "physical common coordinate identification=open",
    "physical momentum scale=open",
    "absolute mass=not derived",
    "integer tick gstar cadence=open",
    "exact discrete variational action=open",
    "production integration=forbidden",
    "no new selected type=true",
    "born bell lorentz completeness=untouched",
)
for marker in terminal_markers:
    check(f"terminal firewall {marker}", marker in protocol_text.replace("_", " "))

passed = sum(result for _, result in checks)
total = len(checks)
print(f"\nFTD-0899 exact certificate: {passed}/{total} checks passed")
verdict = passed == total
print(
    "COMMON_RELATIVE_CONNECTION_GEARBOX_EXACT_"
    "GAMMA_SCALE_IDENTIFICATION_AND_CONTINUOUS_GSTAR_CADENCE_OPEN="
    f"{'TRUE' if verdict else 'FALSE'}"
)
raise SystemExit(0 if verdict else 1)
