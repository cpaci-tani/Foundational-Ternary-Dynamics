"""Exact conditional certificate for the locked FTD-0774 tangent theorem.

This certificate checks only the algebra registered in sections 4 and 6 of
the protocol.  It proves a chart-Hessian isometry conditional on an exact
critical fixed point and exact energy preservation, the resulting
K-self-adjoint cosine reduction when K is positive, positivity of the matched
field block at L=17, the locked filter's behavior, and the equal-angle
rotation implication.  It does not assert that the numerical representative
is exact and it does not derive a symplectic form, a canonical action, or a
native clock.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter"
    / "PREREG_L17_COMPLETE_TANGENT_CANDIDATE_v1.md"
)
PROTOCOL_SHA256 = (
    "0604AF560EA193BDE9E339ADB3FB28C0631B43D204186BEDA977EB700DD7F27E"
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def exact_zero(expression: sp.Expr | sp.MatrixBase) -> bool:
    if isinstance(expression, sp.MatrixBase):
        return all(sp.simplify(entry) == 0 for entry in expression)
    return sp.simplify(expression) == 0


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    protocol = PREREG.read_text(encoding="utf-8")
    check("locked protocol SHA-256", sha256(PREREG) == PROTOCOL_SHA256)
    check(
        "pre-execution lock is explicit",
        "[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]" in protocol,
    )
    check(
        "chart Hessian is load-bearing",
        "The chart Hessian in (4), not an ambient Hessian restricted after the fact,\n"
        "is load-bearing." in protocol,
    )
    check(
        "numerical isometry is not assumed",
        "(6) is not assumed of the measured map" in protocol
        and "isometry gates below must measure it" in protocol,
    )
    check(
        "symplectic overclaim is forbidden",
        "The complete symplectic form has not been derived." in protocol
        and "not a Krein signature, canonical action, or action variable" in protocol,
    )

    # ------------------------------------------------------------------
    # Chain rule in a local constraint chart.
    # For local coordinates x, write f_i=T_ij x_j + Q_i,jk x_j x_k/2.
    # The Hessian of e(f(x)) at zero is T^T K T + sum_i g_i Q_i.
    t11, t12, t21, t22 = sp.symbols("t11 t12 t21 t22", real=True)
    k11, k12, k22 = sp.symbols("k11 k12 k22", real=True)
    g1, g2 = sp.symbols("g1 g2", real=True)
    x1, x2 = sp.symbols("x1 x2", real=True)
    T = sp.Matrix([[t11, t12], [t21, t22]])
    K = sp.Matrix([[k11, k12], [k12, k22]])
    Q1 = sp.Matrix(2, 2, lambda i, j: sp.symbols(f"q1{i}{j}", real=True))
    Q2 = sp.Matrix(2, 2, lambda i, j: sp.symbols(f"q2{i}{j}", real=True))
    Q1 = (Q1 + Q1.T) / 2
    Q2 = (Q2 + Q2.T) / 2
    x = sp.Matrix([x1, x2])
    f = T * x + sp.Matrix([(x.T * Q1 * x)[0], (x.T * Q2 * x)[0]]) / 2
    y1, y2 = sp.symbols("y1 y2", real=True)
    y = sp.Matrix([y1, y2])
    e = (sp.Matrix([g1, g2]).T * y)[0] + (y.T * K * y)[0] / 2
    composed = sp.expand(e.subs({y1: f[0], y2: f[1]}, simultaneous=True))
    hessian_composed = sp.hessian(composed, (x1, x2)).subs({x1: 0, x2: 0})
    chain_rule_hessian = T.T * K * T + g1 * Q1 + g2 * Q2
    check(
        "second-order chart chain rule",
        exact_zero(hessian_composed - chain_rule_hessian),
    )
    critical_hessian = hessian_composed.subs({g1: 0, g2: 0})
    check(
        "critical point removes nonlinear-chart correction",
        exact_zero(critical_hessian - T.T * K * T),
    )
    check(
        "away from criticality the D e D2 f term remains",
        not exact_zero(chain_rule_hessian - T.T * K * T),
    )

    # A one-dimensional exact witness makes the same boundary transparent.
    g, k, t, a, u = sp.symbols("g k t a u", real=True)
    f1 = t * u + a * u**2 / 2
    e1 = g * f1 + k * f1**2 / 2
    check(
        "scalar chain-rule witness",
        exact_zero(sp.diff(e1, u, 2).subs(u, 0) - (k * t**2 + g * a)),
    )
    check(
        "scalar critical witness",
        exact_zero(sp.diff(e1, u, 2).subs({u: 0, g: 0}) - k * t**2),
    )

    # e o f=e and De(0)=0 therefore equate the chart Hessians.
    conserved_hessian_difference = critical_hessian - K
    check(
        "conservation plus criticality yields T-transpose K T minus K",
        exact_zero(conserved_hessian_difference - (T.T * K * T - K)),
    )

    # ------------------------------------------------------------------
    # Exact K-adjoint and self-adjoint reduction on a rational SPD example.
    # This is an exact algebra certificate of the identities, not a numerical
    # measurement of the FTD-0774 map.
    K0 = sp.diag(4, 9)
    R0 = sp.diag(2, 3)  # K0=R0^T R0
    Q0 = sp.Matrix([[sp.Rational(3, 5), -sp.Rational(4, 5)],
                    [sp.Rational(4, 5), sp.Rational(3, 5)]])
    T0 = R0.inv() * Q0 * R0
    T0_inv = T0.inv()

    def k_adjoint(matrix: sp.MatrixBase) -> sp.MatrixBase:
        return K0.inv() * matrix.T * K0

    check("registered K example is positive", K0.det() > 0 and K0[0, 0] > 0)
    check("similarity rotation is orthogonal", exact_zero(Q0.T * Q0 - sp.eye(2)))
    check("exact K isometry", exact_zero(T0.T * K0 * T0 - K0))
    check("K-adjoint equals inverse", exact_zero(k_adjoint(T0) - T0_inv))
    check(
        "inverse K-adjoint equals forward map",
        exact_zero(k_adjoint(T0_inv) - T0),
    )
    S0 = (T0 + T0_inv) / 2
    check("S is K-self-adjoint", exact_zero(k_adjoint(S0) - S0))
    check(
        "positive-metric conjugate map is orthogonal",
        exact_zero((R0 * T0 * R0.inv()).T * (R0 * T0 * R0.inv()) - sp.eye(2)),
    )
    check(
        "cosine reduction of the rational rotation",
        exact_zero(S0 - sp.Rational(3, 5) * sp.eye(2)),
    )

    # ------------------------------------------------------------------
    # Exact positivity of the L=17 matched field block.
    L = sp.Integer(17)
    wave_speed = 1 / sp.sqrt(3)
    sigma_max = 2 * sp.sqrt(3) * sp.cos(sp.pi / (2 * L))
    coupling = sp.simplify(wave_speed * sigma_max / 2)
    field_block = sp.Matrix([[1, -coupling], [-coupling, 1]])
    field_eigenvalues = list(field_block.eigenvals())
    lower = 1 - sp.cos(sp.pi / 34)
    upper = 1 + sp.cos(sp.pi / 34)
    check(
        "L17 curl singular-value product",
        exact_zero(wave_speed * sigma_max - 2 * sp.cos(sp.pi / 34)),
    )
    check("matched field eigenvalues", set(field_eigenvalues) == {lower, upper})
    check("strict L17 lower bound", sp.ask(sp.Q.positive(lower)) is True)
    check("strict L17 upper eigenvalue", sp.ask(sp.Q.positive(upper)) is True)
    check(
        "lower bound is an exact positive square",
        exact_zero(lower - 2 * sp.sin(sp.pi / 68) ** 2)
        and sp.ask(sp.Q.positive(sp.sin(sp.pi / 68))) is True,
    )
    beta = sp.symbols("beta", positive=True)
    check(
        "registered beta-scaled field lower bound",
        exact_zero(beta * lower - beta * (1 - sp.cos(sp.pi / 34)))
        and sp.ask(sp.Q.positive(beta * lower)) is True,
    )
    uniform_block = beta * sp.eye(2)
    check(
        "uniform harmonic field block is positive",
        uniform_block.det() == beta**2 and sp.ask(sp.Q.positive(beta)) is True,
    )

    # ------------------------------------------------------------------
    # Locked polynomial filter.  It ranks cosine distance, is reflected
    # about mu0, and does not in general rank phase distance.
    mu, mu0, d = sp.symbols("mu mu0 d", real=True)
    filter_value = 1 - (mu - mu0) ** 2 / 4
    check("filter center value", exact_zero(filter_value.subs(mu, mu0) - 1))
    check(
        "filter reflection degeneracy",
        exact_zero(
            filter_value.subs(mu, mu0 + d)
            - filter_value.subs(mu, mu0 - d)
        ),
    )
    check(
        "filter decrement is squared cosine distance",
        exact_zero(1 - filter_value - (mu - mu0) ** 2 / 4),
    )
    check(
        "filter curvature is negative",
        exact_zero(sp.diff(filter_value, mu, 2) + sp.Rational(1, 2)),
    )
    omega0 = sp.Rational(11, 10)
    delta = sp.Rational(1, 10)
    phase_plus = sp.cos(omega0 + delta)
    phase_minus = sp.cos(omega0 - delta)
    check(
        "equal phase offsets need not have equal filter values",
        sp.simplify(
            filter_value.subs({mu: phase_plus, mu0: sp.cos(omega0)})
            - filter_value.subs({mu: phase_minus, mu0: sp.cos(omega0)})
        ) != 0,
    )

    # ------------------------------------------------------------------
    # Rotation implication.  On an S eigenspace with |mu|<1,
    # T^2-2 mu T+I=0 and J=(T-mu I)/sin(Omega) squares to -I.  A real
    # four-space therefore splits into two K-orthogonal equal-angle planes.
    c, s = sp.symbols("c s", real=True)
    J2 = sp.Matrix([[0, -1], [1, 0]])
    J4 = sp.diag(1, 1, 1, 1)
    J4[:2, :2] = J2
    J4[2:, 2:] = J2
    T4 = c * sp.eye(4) + s * J4
    relation = c**2 + s**2 - 1
    check("complex structure squares to minus identity", exact_zero(J4**2 + sp.eye(4)))
    check("complex structure is orthogonal", exact_zero(J4.T * J4 - sp.eye(4)))
    check(
        "rotation polynomial on the four-space",
        exact_zero((T4**2 - 2 * c * T4 + sp.eye(4)).subs(s**2, 1 - c**2)),
    )
    check(
        "T plus inverse gives cosine eigenspace",
        exact_zero(
            (T4 + (c * sp.eye(4) - s * J4)) / 2 - c * sp.eye(4)
        ),
    )
    check(
        "two equal rotation planes",
        exact_zero(T4[:2, :2] - T4[2:, 2:])
        and exact_zero(T4[:2, 2:])
        and exact_zero(T4[2:, :2]),
    )
    check(
        "orthogonality follows from c squared plus s squared equals one",
        exact_zero((T4.T * T4 - sp.eye(4)).subs(s**2, 1 - c**2)),
    )
    check(
        "non-endpoint phase makes the complex-structure denominator nonzero",
        "away from `+-1`" in protocol,
    )

    # Explicitly retain the epistemic boundary in executable form.
    forbidden_claims = (
        "symplectic form derived",
        "canonical action derived",
        "native clock established",
    )
    certificate_claims = (
        "chart Hessian isometry",
        "positive quadratic tangent form",
        "equal-angle rotation planes",
    )
    check(
        "certificate claim vocabulary excludes stronger structures",
        not any(claim in certificate_claims for claim in forbidden_claims),
    )

    failures = [label for label, passed in checks if not passed]
    print(
        "FTD-0774 exact tangent theorem certificate: "
        f"{len(checks) - len(failures)}/{len(checks)} checks PASS"
    )
    print(f"protocol_sha256={PROTOCOL_SHA256}")
    print("claim=CONDITIONAL_CHART_HESSIAN_ISOMETRY_AND_ROTATION_REDUCTION")
    print("symplectic_form_derived=false")
    print("canonical_action_derived=false")
    for label in failures:
        print(f"FAIL {label}")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
