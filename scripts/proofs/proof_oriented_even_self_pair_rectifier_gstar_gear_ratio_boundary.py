#!/usr/bin/env python3
"""Exact FTD-0904 oriented even-self-pair/G* gear-ratio certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_ORIENTED_EVEN_SELF_PAIR_RECTIFIER_GSTAR_GEAR_RATIO_BOUNDARY_v1.md"
EXPECTED_PROTOCOL_HASH = "A166A7EA4BBEAFD887DD66B4D4FF1D865D6EF0861688A58ECB1B91E885843C22"

SOURCES = {
    "i_gamma": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md",
        "07BDB4CA22A655C378BCC4BA4B6A69830686200A4B4F59B19136363F5F4F6496",
    ),
    "vector_pair": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md",
        "62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB",
    ),
    "vector_transport": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md",
        "56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27",
    ),
    "self_pair_gearbox": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_POSITIVE_CONNECTION_ORDER_AND_SELF_PAIR_CRITICAL_GEARBOX_BOUNDARY_v1.md",
        "C6504B179463E2AA93F3B93F29FD672BC96771AF2BB9184A0FB1E1214F98F21D",
    ),
    "self_pair_header": (
        ROOT / "engine/include/ftd/eft/self_pair_connection_critical_gearbox.h",
        "038F48F4E99D3CD55CAE25CF09170670733057FF1A43279839D3C78B0DC74447",
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
    "protocol freezes the oriented even connection",
    "a(q,e,\\chi)=\\chi\\gamma q^2e" in normalized_protocol,
)
check(
    "protocol freezes the axis chirality and gamma separation",
    "e_supplies_polar_axis=imposed_reference_data" in protocol_text
    and "chi_supplies_clockwise_counterclockwise_orientation=true" in protocol_text
    and "gamma_magnitude_derived_from_chi_or_i=false" in protocol_text,
)
check(
    "protocol freezes production Born and cadence firewalls",
    "production_integration=forbidden" in protocol_text
    and "born_bell_lorentz_completeness=untouched" in protocol_text
    and "integer_tick_gstar_cadence=open" in protocol_text,
)

# An even polar vector of D alone cannot be inversion equivariant unless zero.
f1, f2, f3 = sp.symbols("f1 f2 f3", real=True)
F = sp.Matrix([f1, f2, f3])
inversion = -sp.eye(3)
forced_zero = sp.solve(list(F - inversion * F), [f1, f2, f3], dict=True)
check("spatial inversion is a signed cubic orthogonal transform", inversion.T * inversion == sp.eye(3))
check("evenness plus inversion covariance forces the polar rectifier to zero", forced_zero == [{f1: 0, f2: 0, f3: 0}])

ex, ey, ez, q = sp.symbols("ex ey ez q", real=True)
chi, gamma = sp.symbols("chi gamma", real=True)
e = sp.Matrix([ex, ey, ez])
Q = sp.Matrix([[0, 0, -1], [1, 0, 0], [0, -1, 0]])
A = chi * gamma * q**2 * e
check("registered signed cubic transform is orthogonal", Q.T * Q == sp.eye(3))
check("oriented even connection is signed-cubic covariant", chi * gamma * q**2 * (Q * e) == Q * A)
check("even connection is invariant under clock-sheet exchange", sp.simplify(A.subs(q, -q) - A) == sp.zeros(3, 1))
check("axis reversal reverses the connection", sp.simplify(chi * gamma * q**2 * (-e) + A) == sp.zeros(3, 1))
check("chirality reversal reverses the connection", sp.simplify(A.subs(chi, -chi) + A) == sp.zeros(3, 1))

# Scalar polarized action, Legendre transform, and rest-sector quartic.
M, m, lam = sp.symbols("M m lambda", positive=True)
Cdot, qdot, P, pi = sp.symbols("Cdot qdot P pi", real=True)
L = M * Cdot**2 / 2 + m * qdot**2 / 2 + chi * gamma * q**2 * Cdot - lam * q**4
p_common = sp.diff(L, Cdot)
p_relative = sp.diff(L, qdot)
check("canonical common momentum contains the even connection", sp.simplify(p_common - (M * Cdot + chi * gamma * q**2)) == 0)
check("relative canonical momentum remains m qdot", sp.simplify(p_relative - m * qdot) == 0)
H_expected = (P - chi * gamma * q**2) ** 2 / (2 * M) + pi**2 / (2 * m) + lam * q**4
H_legendre = (P * Cdot + pi * qdot - L).subs(
    {Cdot: (P - chi * gamma * q**2) / M, qdot: pi / m}, simultaneous=True
)
check("Legendre transform gives the positive even-connection Hamiltonian", sp.simplify(H_legendre - H_expected) == 0)

Lambda = lam + gamma**2 / (2 * M)
H_rest = pi**2 / (2 * m) + Lambda * q**4
rest_reduced = H_expected.subs(P, 0).subs(chi**2, 1)
check("chi-square branch condition folds the connection energy into the quartic", sp.simplify(rest_reduced - H_rest) == 0)
check("effective quartic coupling is positive", Lambda.is_positive is True)
check("origin connection derivative vanishes", sp.diff(chi * gamma * q**2, q).subs(q, 0) == 0)
check("rest-sector clock Hessian vanishes", sp.diff(Lambda * q**4, q, 2).subs(q, 0) == 0)
check("connection derivative is nonzero away for nonzero gamma", sp.diff(sp.diff(chi * gamma * q**2, q), gamma) == 2 * chi * q)

K = P - chi * gamma * q**2
q0, q1 = sp.symbols("q0 q1", real=True)
check(
    "mechanical common impulse is minus chi gamma Delta q squared",
    sp.simplify(
        (P - chi * gamma * q1**2) - (P - chi * gamma * q0**2)
        + chi * gamma * (q1**2 - q0**2)
    ) == 0,
)
PdotE = sp.symbols("PdotE", real=True)
moving = -chi * gamma * PdotE * q**2 / M + gamma**2 * q**4 / (2 * M) + lam * q**4
check("moving sector has the registered quadratic coefficient", sp.expand(moving).coeff(q, 2) == -chi * gamma * PdotE / M)
check("rest sector removes the moving quadratic coefficient", sp.expand(moving.subs(PdotE, 0)).coeff(q, 2) == 0)

# Branch-paired time reversal.
H_reversed = H_expected.subs({P: -P, pi: -pi, chi: -chi}, simultaneous=True)
check("momentum and chirality reversal leaves the branch-paired Hamiltonian invariant", sp.simplify(H_reversed - H_expected) == 0)
check("naive momentum reversal at fixed chirality is not a generic invariance", sp.simplify(H_expected.subs(P, -P) - H_expected) != 0)

# Exact beta/gamma traversal and rectified gear ratios.
a = sp.symbols("a", positive=True)
Gstar = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
B0 = sp.beta(sp.Rational(1, 4), sp.Rational(1, 2))
B2 = sp.beta(sp.Rational(3, 4), sp.Rational(1, 2))
check("quartic traversal beta identity equals sqrt pi Gstar", sp.simplify((B0 - sp.sqrt(sp.pi) * Gstar).rewrite(sp.gamma)) == 0)
check("quadratic moment beta identity equals four sqrt pi over Gstar", sp.simplify((B2 - 4 * sp.sqrt(sp.pi) / Gstar).rewrite(sp.gamma)) == 0)
check("quadratic moment to traversal ratio is four over Gstar squared", sp.simplify((B2 / B0 - 4 / Gstar**2).rewrite(sp.gamma)) == 0)

root_scale = sp.sqrt(m / (2 * Lambda))
period = root_scale * B0 / a
q2_time_integral = root_scale * a * B2
displacement = -chi * gamma * q2_time_integral / M
expected_period_amplitude = sp.sqrt(sp.pi) * Gstar * root_scale
expected_displacement = -4 * sp.sqrt(sp.pi) * chi * gamma * a * root_scale / (M * Gstar)
expected_mean_ratio = -4 * chi * gamma / (M * Gstar**2)
check("period-amplitude product has the exact Gstar factor", sp.simplify((period * a - expected_period_amplitude).rewrite(sp.gamma)) == 0)
check("cycle displacement has the exact inverse-Gstar factor", sp.simplify((displacement - expected_displacement).rewrite(sp.gamma)) == 0)
check("mean directed gear ratio has the exact inverse-Gstar-square factor", sp.simplify((displacement / period / a**2 - expected_mean_ratio).rewrite(sp.gamma)) == 0)
check("chirality reversal reverses continuum displacement", sp.simplify(displacement.subs(chi, -chi) + displacement) == 0)

# Exact discrete rest-sector witness and reciprocal carry.
pi0, pi1, h = sp.symbols("pi0 pi1 h", real=True, nonzero=True)
secant = Lambda * (q1**3 + q1**2 * q0 + q1 * q0**2 + q0**3)
eq_q = q1 - q0 - h * (pi1 + pi0) / (2 * m)
eq_pi = pi1 - pi0 + h * secant
E0 = pi0**2 / (2 * m) + Lambda * q0**4
E1 = pi1**2 / (2 * m) + Lambda * q1**4
energy_factorized = (pi1 + pi0) * (pi1 - pi0) / (2 * m) + secant * (q1 - q0)
check("quartic energy difference has the exact secant factorization", sp.simplify(E1 - E0 - energy_factorized) == 0)
check(
    "discrete effective-quartic energy is exactly conserved",
    sp.simplify(energy_factorized.subs(
        {q1 - q0: h * (pi1 + pi0) / (2 * m), pi1 - pi0: -h * secant},
        simultaneous=True,
    )) == 0,
)

C0, C1 = sp.symbols("C0 C1", real=True)
eq_C = C1 - C0 + h * chi * gamma * (q1**2 + q0**2) / (2 * M)
reverse = {C0: C1, C1: C0, q0: q1, q1: q0, pi0: pi1, pi1: pi0, h: -h}
check("common endpoint equation reverses under endpoint exchange and signed step", sp.simplify(eq_C.xreplace(reverse) + eq_C) == 0)
check("relative coordinate equation reverses under endpoint exchange and signed step", sp.simplify(eq_q.xreplace(reverse) + eq_q) == 0)
check("relative momentum equation reverses under endpoint exchange and signed step", sp.simplify(eq_pi.xreplace(reverse) + eq_pi) == 0)

drive = sp.symbols("drive", positive=True)
directed_step = -drive * (q1**2 + q0**2) / (2 * M)
check("fixed oriented drive gives nonpositive directed displacement each step", directed_step.is_nonpositive is True)
check(
    "directed displacement magnitude is the exact positive sum of two endpoint squares",
    sp.Poly(q1**2 + q0**2, q0, q1).terms()
    == [((2, 0), 1), ((0, 2), 1)],
)

root_two = sp.sqrt(2)
PL0, PR0 = (P + pi0) / root_two, (P - pi0) / root_two
PL1, PR1 = (P + pi1) / root_two, (P - pi1) / root_two
check("left channel receives the relative endpoint impulse", sp.simplify((PL1 - PL0) - (pi1 - pi0) / root_two) == 0)
check("right channel receives the opposite endpoint impulse", sp.simplify((PR1 - PR0) + (pi1 - pi0) / root_two) == 0)
check("canonical channel aggregate remains fixed", sp.simplify(PL1 + PR1 - PL0 - PR0) == 0)

pstar, tau = sp.symbols("pstar tau", positive=True)
kL, kR = sp.symbols("kL kR", real=True)
cL, cR, W = sp.symbols("cL cR W", integer=True)
increment = (pi1 - pi0) / (root_two * pstar)
kL1 = kL + increment - tau * cL
kR1 = kR - increment - tau * cR
W1 = W + cL + cR
check("reciprocal increment is fixed after imposed pstar is supplied", sp.simplify(increment * root_two * pstar - (pi1 - pi0)) == 0)
check("reciprocal carry preserves the lifted aggregate", sp.simplify(kL1 + kR1 + tau * W1 - kL - kR - tau * W) == 0)
check("Gstar is absent from the finite endpoint equations", "gstar" not in str((eq_C, eq_q, eq_pi)).lower())

# Frozen-source scope anchors.
check("FTD-0839 separates orientation from physical realization", "choosing multiplication by `+i` selects an orientation" in texts["i_gamma"])
check("FTD-0841 keeps production polarization and cadence open", "production coupling, polarization, support, and cadence" in texts["vector_pair"])
check(
    "FTD-0890 requires a vector carrier for orientation-free recoil",
    "an orientation-free spatial reaction carrier therefore requires three canonical pairs"
    in " ".join(texts["vector_transport"].split()),
)
check(
    "FTD-0903 books zero symmetric-cycle drift for the odd self-pair",
    "full_cycle_rest_sector_net_common_drift=zero" in texts["self_pair_gearbox"],
)
check("self-pair header leaves gamma scale mass and production open", "does not derive" in texts["self_pair_header"] and "production coupling" in texts["self_pair_header"])

terminal_markers = [
    "even_polar_rectifier_from_d_alone_with_inversion_equivariance=zero",
    "retained_polar_axis_required=true_in_registered_class",
    "clockwise_counterclockwise_branch_required_for_time_reversal=true",
    "oriented_even_connection=imposed_reference_law",
    "rest_sector_critical_quartic=exact",
    "rest_sector_continuum_gstar_period_factor=exact",
    "continuum_displacement_per_cycle_proportional_to_inverse_gstar=exact",
    "continuum_mean_gear_ratio_proportional_to_inverse_gstar_squared=exact",
    "discrete_directed_common_displacement=exact_per_step",
    "mechanical_common_impulse=exactly_exchanged_with_q_squared",
    "moving_sector_exact_quartic=false_generically",
    "gamma_magnitude_derived_from_chi_or_i=false",
    "polar_axis_substrate_formation=open",
    "chi_substrate_formation=open",
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
print(f"\nFTD-0904 exact certificate: {passed}/{len(checks)} checks passed")
if passed == len(checks):
    print("ORIENTED_EVEN_SELF_PAIR_RECTIFIER_AND_GSTAR_INVERSE_GEAR_RATIOS_EXACT_AXIS_CHIRALITY_GAMMA_SCALE_PRODUCTION_CADENCE_OPEN=TRUE")
raise SystemExit(0 if passed == len(checks) else 1)
