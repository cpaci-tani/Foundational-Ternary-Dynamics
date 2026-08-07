#!/usr/bin/env python3
"""Exact algebra for the FTD-0416 optimistic Lorentz-RG surrogate.

This is not an FTD loop calculation and it performs no numerical search.  It
linearizes the published one-loop anisotropic-QED beta functions about their
common-cone fixed line, diagonalizes the velocity-mixing matrix, integrates
the relative-cone mode against the running charge, and verifies the source
contract of AUDIT_LORENTZ_RG_ATTRACTION.md.
"""

from __future__ import annotations

from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def main() -> None:
    checks = 0

    # ------------------------------------------------------------------
    # The selected BCC clock remains finite range in the IR surrogate.
    # ------------------------------------------------------------------
    theta = sp.symbols("theta", real=True)
    t_bcc = sp.Rational(2, 3) * (1 - sp.cos(theta) ** 3)
    harmonic = sp.Rational(1, 2) * (1 - sp.cos(theta)) \
        + sp.Rational(1, 6) * (1 - sp.cos(3 * theta))
    require(sp.trigsimp(t_bcc - harmonic) == 0,
            "A1 BCC time is exactly the positive 1-step/3-step harmonic kernel")
    checks += 1

    # The transverse representative A=P_T J is not a finite-support local
    # map.  P_xx has different directional limits at q=0.
    q = sp.symbols("q", real=True)
    p_xx_x_axis = sp.simplify(1 - q**2 / q**2)
    p_xx_y_axis = sp.Integer(1)
    require(p_xx_x_axis == 0 and p_xx_y_axis == 1,
            "A2 the transverse projector has no direction-independent q=0 limit")
    checks += 1

    # ------------------------------------------------------------------
    # Published one-loop anisotropic-QED flow, linearized at v=c.
    # beta_v = -(8/3) alpha (v-c)
    # beta_c = +(4 N_f/3) alpha (v-c)
    # ------------------------------------------------------------------
    nf, alpha = sp.symbols("N_f alpha", positive=True)
    v, c = sp.symbols("v c", positive=True)
    ratio = v / c
    beta_v_full = (
        4 * alpha * v / (3 * (1 + ratio) ** 2)
        * (1 + 2 * ratio + ratio**2 - 4 * ratio**3)
    )
    beta_c_full = -sp.Rational(2, 3) * alpha * c * nf * (1 - ratio**2)
    mixing = alpha * sp.Matrix([
        [-sp.Rational(8, 3), sp.Rational(8, 3)],
        [sp.Rational(4, 3) * nf, -sp.Rational(4, 3) * nf],
    ])

    jacobian = sp.Matrix([beta_v_full, beta_c_full]).jacobian([v, c])
    jacobian_common = sp.simplify(jacobian.subs(v, c))
    require(jacobian_common == mixing,
            "R0 the published nonlinear beta functions linearize to the frozen matrix")
    checks += 1

    common = sp.Matrix([1, 1])
    require(mixing * common == sp.zeros(2, 1),
            "R1 the common-speed direction is an exact zero mode")
    checks += 1

    eigenvalues = {sp.factor(value) for value in mixing.eigenvals()}
    lambda_rel = -sp.Rational(4, 3) * (nf + 2) * alpha
    require(sp.Integer(0) in eigenvalues
            and len(eigenvalues) == 2
            and any(sp.simplify(value - lambda_rel) == 0
                    for value in eigenvalues),
            "R2 the relative-speed eigenvalue is -4(N_f+2)alpha/3")
    checks += 1

    dv, dc = sp.symbols("delta_v delta_c")
    beta = mixing * sp.Matrix([dv, dc])
    beta_difference = sp.factor(beta[0] - beta[1])
    require(sp.simplify(beta_difference - lambda_rel * (dv - dc)) == 0,
            "R3 only the relative cone runs at linear order")
    checks += 1

    # Charge flow in the same convention:
    # beta_alpha = -(4/3) N_f alpha^2.
    beta_alpha = -sp.Rational(4, 3) * nf * alpha**2
    exponent = sp.factor(lambda_rel / (beta_alpha / alpha))
    require(exponent == (nf + 2) / nf,
            "R4 eliminating RG time gives d ln(delta)/d ln(alpha)=(N_f+2)/N_f")
    checks += 1

    require(sp.simplify(sp.diff(1 + 2 / nf, nf)) == -2 / nf**2,
            "R5 the attraction exponent decreases monotonically with species count")
    checks += 1

    # The most optimistic perturbative suppression for alpha_IR=1/137,
    # alpha_UV<=1 and integer N_f>=1 is N_f=1, alpha_UV=1.
    suppression_best = sp.Rational(1, 137) ** 3
    require(suppression_best == sp.Rational(1, 2571353),
            "Q1 strongest perturbative one-species suppression is exactly 1/137^3")
    checks += 1
    require(3.88e-7 < float(suppression_best) < 3.90e-7,
            "Q2 strongest suppression is 3.89e-7")
    checks += 1

    epsilon_15 = sp.Rational(1, 10**15)
    max_uv_15 = sp.simplify(epsilon_15 / suppression_best)
    require(max_uv_15 == sp.Rational(2571353, 10**15),
            "Q3 a 1e-15 IR tolerance requires UV mismatch below 2.571353e-9")
    checks += 1

    epsilon_21 = sp.Rational(1, 10**21)
    max_uv_21 = sp.simplify(epsilon_21 / suppression_best)
    require(max_uv_21 == sp.Rational(2571353, 10**21),
            "Q4 a 1e-21 IR tolerance requires UV mismatch below 2.571353e-15")
    checks += 1

    # ------------------------------------------------------------------
    # Source-contract checks prevent a continuum surrogate from being cited
    # as the missing FTD lattice calculation.
    # ------------------------------------------------------------------
    audit = read("docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_RG_ATTRACTION.md")
    radiative = read("docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_RADIATIVE_CLOSURE.md")
    wilson = read("docs/theory/10_eft_program/scopes_and_specs/SPEC_WILSON_DIRAC_FTD.md")
    u1_bridge = read(
        "docs/theory/10_eft_program/derivations/"
        "DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md"
    )

    require("[SELECTED IR SURROGATE]" in audit
            and "[EXTERNAL ONE-LOOP RESULT]" in audit,
            "S1 the action and beta functions retain their imported status")
    checks += 1
    require("does not calculate the FTD" in audit
            and "Brillouin-zone threshold" in audit,
            "S2 the audit explicitly withholds an FTD loop verdict")
    checks += 1
    require("A_mu = P_T J_mu" in audit and "spatially nonlocal" in audit,
            "S3 the flux-to-connection locality price is explicit")
    checks += 1
    require("a 1PI two-point calculation" in radiative
            and "complete coefficient" in radiative,
            "S4 FTD-0415's complete-matrix closure criterion remains present")
    checks += 1
    require("FTD-0416 optimistic RG surrogate" in wilson,
            "S5 the Wilson specification carries the non-promotion notice")
    checks += 1
    require("Locality ceiling (FTD-0416)" in u1_bridge
            and "inverse lattice Laplacian/Poisson" in u1_bridge
            and "not** a local microscopic flux-to-link" in u1_bridge,
            "S6 the canonical U(1) bridge records the projector locality ceiling")
    checks += 1

    print()
    print(f"FTD-0416 exact checks: {checks}/{checks} passed")
    print("MATRIX   alpha*[[-8/3, 8/3], [4*N_f/3, -4*N_f/3]]")
    print("MODES    lambda_common=0; lambda_relative=-4*(N_f+2)*alpha/3")
    print("FLOW     delta_IR/delta_UV=(alpha_IR/alpha_UV)^((N_f+2)/N_f)")
    print("BEST     alpha_IR=1/137, alpha_UV<=1, N_f>=1 => suppression >= 1/137^3")
    print("STATUS   EXTERNAL SURROGATE; OFF-SHELL FTD-0419 NONZERO; ON-SHELL OPEN")


if __name__ == "__main__":
    main()
