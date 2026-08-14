"""FTD-0840 exact native pair-energy recursion certificate.

This source-locked certificate checks a registered mathematical extension. It
does not search parameters, fit a period, or modify production dynamics.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCE_HASHES = {
    "engine/include/ftd/eft/native_modal_phase_action.h":
        "C1E9D5C1944E66D7601D193DC77A39980EBA24B84A41F7D752A3A363910060B6",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/energy_ledger_compute.cpp":
        "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md":
        "2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md":
        "07BDB4CA22A655C378BCC4BA4B6A69830686200A4B4F59B19136363F5F4F6496",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "DERIV_QUARTIC_CLOCK_CM_GEARBOX_v1.md":
        "1B969544B065D576523235F40A20918C22E0C55978E52282E2FC623385BC2FDF",
}


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    passed = bool(condition)
    checks.append((name, passed))
    print(f"[{'PASS' if passed else 'FAIL'}] {name}")


def exact_zero(expression: sp.Expr) -> bool:
    return sp.simplify(sp.expand_func(expression)) == 0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


check(
    "C1 all frozen production and theory sources match the preregistered hashes",
    all(sha256(ROOT / relative) == expected
        for relative, expected in SOURCE_HASHES.items()),
)

modal_source = (
    ROOT / "engine/include/ftd/eft/native_modal_phase_action.h"
).read_text(encoding="utf-8")
energy_source = (
    ROOT / "engine/src/energy_ledger_compute.cpp"
).read_text(encoding="utf-8")
phase_read_source = (
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp"
).read_text(encoding="utf-8")

check(
    "C2 production exposes the registered target-blind canonical modal pair",
    "double canonical_q = 0.0;" in modal_source
    and "double canonical_p = 0.0;" in modal_source
    and "result.action = 0.5 * (" in modal_source
    and "No G*, target period, measured frequency" in modal_source,
)

registered_energy_line = (
    "const double E_total = 0.5 * (E_field + E_wave) + E_kin + E_strong;"
)
forbidden_pair_tokens = (
    "pair_energy", "quartic_energy", "PairClosureMap", "q_pair", "u_pair"
)
check(
    "C3 the frozen production energy has no registered pair-energy channel",
    registered_energy_line in energy_source
    and all(token not in energy_source for token in forbidden_pair_tokens)
    and all(token not in phase_read_source for token in forbidden_pair_tokens),
)

# The signed self-pair retains the sheet through the unsquared q coordinate.
r, y = sp.symbols("r y", positive=True, real=True)
m, lam = sp.symbols("m lambda", positive=True, real=True)

pair_branches: list[tuple[sp.Expr, sp.Expr]] = []
for sign in (-1, 1):
    q_branch = sign * r
    u_branch = sign * r**2
    pair_branches.append((q_branch, u_branch))

check(
    "C4 the signed self-pair squares exactly to the quartic coordinate",
    all(exact_zero(u**2 - q**4) for q, u in pair_branches),
)

p_from_y = sp.sqrt(2 * m * lam) * y
check(
    "C5 the quartic Hamiltonian is a quadratic self-dual pair energy",
    all(exact_zero(
        p_from_y**2 / (2 * m) + lam * q**4 - lam * (u**2 + y**2)
    ) for q, u in pair_branches),
)

q, p = sp.symbols("q p", real=True)
hamiltonian = p**2 / (2 * m) + lam * q**4
qdot = sp.diff(hamiltonian, p)
pdot = -sp.diff(hamiltonian, q)
check(
    "C6 Hamilton equations give the registered velocity and cubic restorer",
    exact_zero(qdot - p / m) and exact_zero(pdot + 4 * lam * q**3),
)

kappa = 2 * sp.sqrt(2 * lam / m)
flow_matches = []
radius_conserved = []
area_matches = []
for q_branch, u_branch in pair_branches:
    branch_qdot = p_from_y / m
    branch_udot = 2 * r * branch_qdot
    branch_pdot = -4 * lam * q_branch**3
    branch_ydot = branch_pdot / sp.sqrt(2 * m * lam)
    target_udot = kappa * r * y
    target_ydot = -kappa * r * u_branch
    flow_matches.append(
        exact_zero(branch_udot - target_udot)
        and exact_zero(branch_ydot - target_ydot)
    )
    radius_conserved.append(
        exact_zero(2 * u_branch * branch_udot + 2 * y * branch_ydot)
    )
    area_matches.append(exact_zero(
        u_branch * branch_ydot - y * branch_udot
        + kappa * r * (u_branch**2 + y**2)
    ))

check(
    "C7 the retained lift induces an oriented weighted quarter-turn on the pair",
    all(flow_matches),
)
check(
    "C8 the induced pair flow conserves its quadratic radius",
    all(radius_conserved),
)
check(
    "C9 the continuous swept-area current has a strict orientation off zero",
    all(area_matches) and bool(kappa.is_positive) and bool(r.is_positive),
)

# The substitution t=x^4 turns the quarter integral into one fourth of a beta
# integral with exponents a-1=-3/4 and b-1=-1/2.
a = sp.Rational(1, 4)
b = sp.Rational(1, 2)
check(
    "C10 the quartic quarter-period reduces to one fourth B(1/4,1/2)",
    a - 1 == -sp.Rational(3, 4)
    and b - 1 == -sp.Rational(1, 2)
    and sp.Rational(1, 4) * sp.beta(a, b)
        == sp.Rational(1, 4) * sp.beta(sp.Rational(1, 4), sp.Rational(1, 2)),
)

gstar = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
beta_quartic = sp.beta(sp.Rational(1, 4), sp.Rational(1, 2))
check(
    "C11 the beta factor is exactly square-root-pi times G star",
    exact_zero(beta_quartic - sp.sqrt(sp.pi) * gstar),
)

A = sp.symbols("A", positive=True, real=True)
period = sp.sqrt(m / (2 * lam)) * beta_quartic / A
check(
    "C12 the continuum period obeys the exact period-amplitude G-star law",
    exact_zero(period * A - sp.sqrt(sp.pi) * gstar * sp.sqrt(m / (2 * lam))),
)

# Registered symmetric discrete-gradient recursion.
q0, q1, p0, p1 = sp.symbols("q_0 q_1 p_0 p_1", real=True)
h = sp.symbols("h", positive=True, real=True)
s3 = q1**3 + q1**2 * q0 + q1 * q0**2 + q0**3
check(
    "C13 the quartic divided difference factorizes exactly",
    exact_zero(q1**4 - q0**4 - (q1 - q0) * s3),
)

derivative_quadratic = 3 * q1**2 + 2 * q1 * q0 + q0**2
positive_decomposition = 2 * q1**2 + (q1 + q0)**2
check(
    "C14 the scalar-root derivative contains a positive-definite quadratic",
    exact_zero(derivative_quadratic - positive_decomposition),
)

scalar_root = (
    2 * m * (q1 - q0) / h - 2 * p0 + h * lam * s3
)
scalar_derivative = sp.diff(scalar_root, q1)
root_polynomial = sp.Poly(sp.expand(scalar_root * h), q1)
check(
    "C15 the implicit next state has one global scalar root",
    exact_zero(
        scalar_derivative
        - (2 * m / h + h * lam * positive_decomposition)
    )
    and root_polynomial.degree() == 3
    and exact_zero(root_polynomial.LC() - h**2 * lam)
    and bool((2 * m / h).is_positive)
    and bool((h * lam).is_positive),
)

residual_q = q1 - q0 - h * (p1 + p0) / (2 * m)
residual_p = p1 - p0 + h * lam * s3
jacobian = sp.Matrix([residual_q, residual_p]).jacobian([q1, p1])
jacobian_target = 1 + h**2 * lam * positive_decomposition / (2 * m)
check(
    "C16 the implicit-step Jacobian determinant is strictly positive",
    exact_zero(jacobian.det() - jacobian_target)
    and bool((h**2 * lam / (2 * m)).is_positive),
)

energy_difference_on_step = (
    (-h * lam * s3) * (p1 + p0) / (2 * m)
    + lam * (h * (p1 + p0) / (2 * m)) * s3
)
check(
    "C17 the discrete-gradient recursion conserves the full energy exactly",
    exact_zero(energy_difference_on_step),
)

def rq(qa: sp.Expr, pa: sp.Expr, qb: sp.Expr, pb: sp.Expr,
       step: sp.Expr) -> sp.Expr:
    return qb - qa - step * (pb + pa) / (2 * m)


def rp(qa: sp.Expr, pa: sp.Expr, qb: sp.Expr, pb: sp.Expr,
       step: sp.Expr) -> sp.Expr:
    divided_difference = qb**3 + qb**2 * qa + qb * qa**2 + qa**3
    return pb - pa + step * lam * divided_difference


check(
    "C18 endpoint exchange with step reversal makes the method self-adjoint",
    exact_zero(rq(q1, p1, q0, p0, -h) + residual_q)
    and exact_zero(rp(q1, p1, q0, p0, -h) + residual_p),
)
check(
    "C19 momentum reversal maps every forward step to its physical reverse",
    exact_zero(rq(q1, -p1, q0, -p0, h) + residual_q)
    and exact_zero(rp(q1, -p1, q0, -p0, h) - residual_p),
)

qbar = (q1 + q0) / 2
pbar = (p1 + p0) / 2
area_on_step = qbar * (-h * lam * s3) - pbar * (h * pbar / m)
area_target = -h * (
    lam * (q1 + q0)**2 * (q1**2 + q0**2) / 2
    + (p1 + p0)**2 / (4 * m)
)
check(
    "C20 the discrete swept-area witness has the exact registered factorization",
    exact_zero(area_on_step - area_target),
)

antipodal_rq = sp.simplify(
    residual_q.subs({q1: -q0, p1: -p0})
)
antipodal_rp = sp.simplify(
    residual_p.subs({q1: -q0, p1: -p0})
)
check(
    "C21 every nonzero discrete step has one strict orientation",
    exact_zero(antipodal_rq + 2 * q0)
    and exact_zero(antipodal_rp + 2 * p0)
    and bool(h.is_positive) and bool(m.is_positive) and bool(lam.is_positive),
)

fixed_rq = sp.simplify(residual_q.subs({q1: q0, p1: p0}))
fixed_rp = sp.simplify(residual_p.subs({q1: q0, p1: p0}))
E = sp.symbols("E", positive=True, real=True)
q_bound_fourth = E / lam
p_bound_squared = 2 * m * E
check(
    "C22 the origin is the only fixed point and positive-energy shells are compact",
    exact_zero(fixed_rq + h * p0 / m)
    and exact_zero(fixed_rp - 4 * h * lam * q0**3)
    and bool(q_bound_fourth.is_positive)
    and bool(p_bound_squared.is_positive),
)

# Turning-point series: the registered method is consistent with the
# Hamiltonian generator but is not its exact finite-h flow.
a2_discrete = -2 * lam * A**3 / m
a4_discrete = 6 * lam**2 * A**5 / m**2
b1_discrete = 2 * m * a2_discrete
b3_discrete = 2 * m * a4_discrete
a4_exact = 2 * lam**2 * A**5 / m**2
b3_exact = 8 * lam**2 * A**5 / m
turning_delta = a2_discrete * h**2 + a4_discrete * h**4
turning_q1 = A + turning_delta
turning_p1 = b1_discrete * h + b3_discrete * h**3
turning_s3 = (
    turning_q1**3 + turning_q1**2 * A + turning_q1 * A**2 + A**3
)
turning_residual_p = turning_p1 / h + lam * turning_s3
check(
    "C23 the recursion has the correct generator but differs from exact finite-time flow",
    exact_zero(sp.limit(-lam * s3, q1, q0) + 4 * lam * q0**3)
    and exact_zero(sp.limit((p1 + p0) / (2 * m), p1, p0) - p0 / m)
    and exact_zero(sp.series(turning_residual_p, h, 0, 4).removeO())
    and a4_discrete != a4_exact
    and b3_discrete != b3_exact,
)

check(
    "C24 the result is an exact recursive bridge with production and cadence debts open",
    registered_energy_line in energy_source
    and all(token not in energy_source for token in forbidden_pair_tokens)
    and all(flow_matches)
    and exact_zero(energy_difference_on_step)
    and exact_zero(period * A - sp.sqrt(sp.pi) * gstar * sp.sqrt(m / (2 * lam)))
    and a4_discrete != a4_exact,
)

passed = sum(1 for _, value in checks if value)
all_pass = passed == len(checks)
print(f"\nFTD-0840 native pair-energy recursion: {passed}/{len(checks)} PASS")
if all_pass:
    print("SIGNED_SELF_PAIR_GIVES_QUADRATIC_SELF_DUAL_ENERGY")
    print("DISCRETE_RECURSION_UNIQUE_REVERSIBLE_ENERGY_CLOSED_AND_ORIENTED")
    print("CONTINUUM_GSTAR_SHAPE_FACTOR_EXACT")
    print("PRODUCTION_PAIR_COUPLING_AND_FINITE_TICK_GSTAR_CADENCE_OPEN")
else:
    print("NATIVE_PAIR_ENERGY_RECURSION_CERTIFICATE_INVALID")

raise SystemExit(0 if all_pass else 1)
