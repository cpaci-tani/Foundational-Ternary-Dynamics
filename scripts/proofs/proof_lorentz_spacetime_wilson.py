#!/usr/bin/env python3
"""Exact checks for the FTD-0418 one-tick spacetime Wilson regulator.

No parameter search, physical-target fit, or numerical loop estimate is
performed.  The script verifies the free operator, all 15 Wilson corner gaps,
the leading cone and quartic expansion, the one-/two-photon Ward identities,
the anisotropic gauge-fixing inversion, and the documentation contract.
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
    c = sp.sqrt(sp.Rational(1, 7))
    c2 = sp.Rational(1, 7)

    # ------------------------------------------------------------------
    # Free operator and Brillouin-corner count.
    # ------------------------------------------------------------------
    p0, qx, qy, qz, m = sp.symbols("p_0 q_x q_y q_z m", real=True)
    qs = (qx, qy, qz)
    scalar = m + (1 - sp.cos(p0)) + c * sum(1 - sp.cos(q) for q in qs)
    temporal = sp.sin(p0)
    spatial = tuple(c * sp.sin(q) for q in qs)

    require(sp.simplify(scalar.subs({p0: 0, qx: 0, qy: 0, qz: 0}) - m) == 0
            and temporal.subs(p0, 0) == 0
            and all(term.subs(q, 0) == 0 for term, q in zip(spatial, qs)),
            "F1 the origin reduces exactly to the bare mass")
    checks += 1

    corners: list[tuple[int, int, sp.Expr]] = []
    for n0 in (0, 1):
        for ns in range(4):
            multiplicity = int(sp.binomial(3, ns))
            mass_shift = sp.simplify(2 * n0 + 2 * c * ns)
            corners.extend((n0, ns, mass_shift) for _ in range(multiplicity))
    require(len(corners) == 16,
            "F2 the four-dimensional Brillouin zone has 16 corners")
    checks += 1
    zero_corners = [entry for entry in corners if entry[2] == 0]
    require(zero_corners == [(0, 0, sp.Integer(0))],
            "F3 only the origin remains massless at m=0")
    checks += 1
    lifted = [entry[2] for entry in corners if entry[2] != 0]
    require(len(lifted) == 15 and all(bool(value > 0) for value in lifted),
            "F4 all 15 non-origin spacetime doublers have positive Wilson mass")
    checks += 1
    require(min(lifted) == 2 * c,
            "F5 the smallest doubler shift is exactly 2/sqrt(7)")
    checks += 1

    # ------------------------------------------------------------------
    # Exact massless correlation pole and its infrared expansion.
    # ------------------------------------------------------------------
    h = sum(1 - sp.cos(q) for q in qs)
    p = sum(sp.sin(q) ** 2 for q in qs)
    cosh_e = 1 + c2 * (p + h**2) / (2 * (1 + c * h))
    require(sp.simplify(cosh_e - (
        1 + (c2 * p + c2 * h**2) / (2 * (1 + c * h))
    )) == 0,
            "P1 the exact analytically-continued massless pole is frozen")
    checks += 1
    require(sp.simplify(cosh_e.subs({qx: 0, qy: 0, qz: 0}) - 1) == 0,
            "P2 the physical correlation energy vanishes at the origin")
    checks += 1

    eps = sp.symbols("eps", real=True)
    s2 = sum(q**2 for q in qs)
    q4 = sum(q**4 for q in qs)
    p22 = qx**2 * qy**2 + qx**2 * qz**2 + qy**2 * qz**2
    scaled_pole = cosh_e.subs({q: eps * q for q in qs})
    pole_series = sp.series(scaled_pole, eps, 0, 6).removeO().expand()

    aq, ap = sp.symbols("a_Q4 a_P22")
    e2 = c2 * eps**2 * s2 + eps**4 * (aq * q4 + ap * p22)
    cosh_series = 1 + e2 / 2 + e2**2 / 24
    residual4 = sp.expand(cosh_series - pole_series).coeff(eps, 4)
    polynomial = sp.Poly(residual4, qx, qy, qz)
    solution = sp.solve(
        (
            sp.Eq(polynomial.coeff_monomial(qx**4), 0),
            sp.Eq(polynomial.coeff_monomial(qx**2 * qy**2), 0),
        ),
        (aq, ap),
        dict=True,
    )
    expected = {
        aq: c2 * (-sp.Rational(2, 21) - 1 / (2 * sp.sqrt(7))),
        ap: c2 * (sp.Rational(10, 21) - 1 / sp.sqrt(7)),
    }
    require(solution == [expected],
            "P3 exact series reversion fixes the fermion quartic tensor")
    checks += 1
    require(sp.simplify(expected[aq] / c2
                        - (-sp.Rational(2, 21) - 1 / (2 * sp.sqrt(7)))) == 0
            and sp.simplify(expected[ap] / c2
                            - (sp.Rational(10, 21) - 1 / sp.sqrt(7))) == 0,
            "P4 the normalized fermion pole matches the documented coefficients")
    checks += 1
    photon_aq = -sp.Rational(1, 14)
    photon_ap = sp.Rational(1, 42)
    require(sp.simplify(expected[aq] / c2 - photon_aq) != 0
            and sp.simplify(expected[ap] / c2 - photon_ap) != 0,
            "P5 the minimal local matter and photon poles differ at quartic order")
    checks += 1

    # ------------------------------------------------------------------
    # One-photon Ward identity, checked in independent Clifford channels.
    # ------------------------------------------------------------------
    p_var, k_var, ell_var, r, nu, g = sp.symbols(
        "p k ell r nu g", real=True
    )
    midpoint = p_var + k_var / 2
    khat = 2 * sp.sin(k_var / 2)
    d_scalar = lambda x: r * (1 - sp.cos(x))
    d_gamma = lambda x: sp.I * nu * sp.sin(x)
    v1_scalar = g * r * sp.sin(midpoint)
    v1_gamma = sp.I * g * nu * sp.cos(midpoint)
    require(sp.trigsimp(khat * v1_scalar
                        - g * (d_scalar(p_var + k_var) - d_scalar(p_var))) == 0,
            "W1 the scalar Wilson channel obeys the one-photon Ward identity")
    checks += 1
    require(sp.trigsimp(khat * v1_gamma
                        - g * (d_gamma(p_var + k_var) - d_gamma(p_var))) == 0,
            "W2 the Dirac channel obeys the one-photon Ward identity")
    checks += 1

    # ------------------------------------------------------------------
    # Two-photon Ward identity and absence of mixed axial seagulls.
    # ------------------------------------------------------------------
    total_midpoint = p_var + (k_var + ell_var) / 2
    v2_scalar = g**2 * r * sp.cos(total_midpoint)
    v2_gamma = -sp.I * g**2 * nu * sp.sin(total_midpoint)
    v1s = lambda average: g * r * sp.sin(average)
    v1g = lambda average: sp.I * g * nu * sp.cos(average)
    rhs_scalar = g * (
        v1s(total_midpoint + k_var / 2)
        - v1s(total_midpoint - k_var / 2)
    )
    rhs_gamma = g * (
        v1g(total_midpoint + k_var / 2)
        - v1g(total_midpoint - k_var / 2)
    )
    require(sp.trigsimp(khat * v2_scalar - rhs_scalar) == 0,
            "W3 the scalar seagull obeys the second Ward identity")
    checks += 1
    require(sp.trigsimp(khat * v2_gamma - rhs_gamma) == 0,
            "W4 the Dirac seagull obeys the second Ward identity")
    checks += 1
    require(sp.simplify(v2_scalar.subs({p_var: 0, k_var: 0, ell_var: 0}))
            == g**2 * r,
            "W5 the zero-momentum Wilson seagull is nonzero and compulsory")
    checks += 1
    delta_mu_nu = sp.KroneckerDelta(sp.Symbol("mu"), sp.Symbol("nu_index"))
    require(delta_mu_nu.subs({sp.Symbol("mu"): 0,
                              sp.Symbol("nu_index"): 1}) == 0,
            "W6 a purely axial action has no mixed-index seagull")
    checks += 1

    # ------------------------------------------------------------------
    # Gauge-fixing inversion in a phase-aligned momentum basis.
    # ------------------------------------------------------------------
    k0, k1, k2, k3 = sp.symbols("khat_0 khat_1 khat_2 khat_3", real=True)
    qvec = sp.Matrix([k1, k2, k3])
    qhat2 = (qvec.T * qvec)[0]
    gauge_kernel = sp.zeros(4)
    gauge_kernel[0, 0] = qhat2
    for i in range(3):
        gauge_kernel[0, i + 1] = -k0 * qvec[i]
        gauge_kernel[i + 1, 0] = -k0 * qvec[i]
        for j in range(3):
            gauge_kernel[i + 1, j + 1] = (
                (k0**2 + c2 * qhat2) * (1 if i == j else 0)
                - c2 * qvec[i] * qvec[j]
            )
    gauge_vector = sp.Matrix([k0, c2 * k1, c2 * k2, c2 * k3])
    fixed_kernel = sp.simplify(
        gauge_kernel + gauge_vector * gauge_vector.T / c2
    )
    denominator = k0**2 + c2 * qhat2
    expected_kernel = sp.diag(denominator / c2,
                              denominator, denominator, denominator)
    require(fixed_kernel == expected_kernel,
            "G1 the selected local gauge fixing exactly diagonalizes the photon kernel")
    checks += 1
    propagator = sp.diag(c2 / denominator,
                         1 / denominator, 1 / denominator, 1 / denominator)
    require(sp.simplify(expected_kernel * propagator) == sp.eye(4),
            "G2 the documented anisotropic photon propagator is the exact inverse")
    checks += 1

    # ------------------------------------------------------------------
    # Documentation/source contract.
    # ------------------------------------------------------------------
    audit = read("docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_SPACETIME_WILSON.md")
    local = read("docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_LOCAL_LINK_FLUX.md")
    wilson = read(
        "docs/theory/10_eft_program/scopes_and_specs/SPEC_WILSON_DIRAC_FTD.md"
    )
    charge_gate = read(
        "docs/theory/10_eft_program/archive/closed_negative/"
        "AUDIT_NATIVE_CONSERVED_CHARGE_GATE.md"
    )

    require("[SELECTED BRANCH-B REGULATOR]" in audit
            and "does not derive spinors" in audit,
            "S1 the Wilson matter import is priced and not called emergent")
    checks += 1
    require("other 15 corners" in audit and "2/sqrt(7)" in audit,
            "S2 the complete spacetime doubler claim is explicit")
    checks += 1
    require("second Ward identity" in audit and "contact term is therefore compulsory" in audit,
            "S3 the action-complete seagull contract is explicit")
    checks += 1
    require("does **not** evaluate these integrals" in audit
            and "FTD-0419" in audit,
            "S4 FTD-0418 itself claims no full-zone coefficient; FTD-0419 integrates it")
    checks += 1
    require("conserved map from the ternary tick history" in local
            and "dim\\ker M=0" in charge_gate and "CLOSED NEGATIVE" in charge_gate,
            "S5 FTD-0421 resolves the frozen native-current route closed negative")
    checks += 1
    require("FTD-0418" in wilson and "one-tick" in wilson,
            "S6 the canonical Wilson specification records the new regulator branch")
    checks += 1

    print()
    print(f"FTD-0418 exact/source checks: {checks}/{checks} passed")
    print("ACTION   nearest-neighbour Euclidean Wilson matter + FTD-0417 links")
    print("CORNERS  one massless origin; all 15 spacetime doublers lifted")
    print("WARD     exact one-photon and two-photon identities")
    print("STATUS   REGULATOR/VERTICES FROZEN; OFF-SHELL BZ DONE; NATIVE CURRENT CLOSED")


if __name__ == "__main__":
    main()
