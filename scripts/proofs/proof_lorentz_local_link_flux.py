#!/usr/bin/env python3
"""Exact checks for the FTD-0417 local link-flux photon candidate.

No parameter search or physical-target fit is performed.  The script verifies
the unit-plaquette gauge identity, the exact transverse pole, full-band
stability at the inherited selected cone, the quartic tree-level defect, and
the documentation/source contract.
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

    # Exact d^2=0 gauge invariance of the unit-plaquette curvature.
    dmu_dnu_chi, dnu_dmu_chi = sp.symbols(
        "Delta_mu_Delta_nu_chi Delta_nu_Delta_mu_chi"
    )
    curvature_shift = -dmu_dnu_chi + dnu_dmu_chi
    require(curvature_shift.subs(dmu_dnu_chi, dnu_dmu_chi) == 0,
            "G1 commuting differences leave every plaquette curvature invariant")
    checks += 1

    # Exact free transverse pole.
    theta, qx, qy, qz = sp.symbols("theta q_x q_y q_z", real=True)
    qs = (qx, qy, qz)
    c2 = sp.Rational(1, 7)
    omega_hat2 = 4 * sp.sin(theta / 2) ** 2
    q_hat2 = 4 * sum(sp.sin(q / 2) ** 2 for q in qs)
    frozen_pole = 4 * (
        sp.sin(theta / 2) ** 2
        - c2 * sum(sp.sin(q / 2) ** 2 for q in qs)
    )
    require(sp.expand_trig(sp.simplify(
                omega_hat2 - c2 * q_hat2 - frozen_pole
            )) == 0,
            "P1 transverse inverse propagator has the frozen nearest-link pole")
    checks += 1

    # Complete spatial band: sum sin^2(q_i/2) <= 3.
    x_max = 3 * c2
    require(x_max == sp.Rational(3, 7) and x_max < 1,
            "P2 the complete Brillouin band is stable at c_A^2=1/7")
    checks += 1

    # Infrared series.  Scale every momentum by eps and solve theta^2 as a
    # formal invariant ansatz through order eps^4.
    eps = sp.symbols("eps", real=True)
    s2 = sum(q**2 for q in qs)
    q4 = sum(q**4 for q in qs)
    p22 = qx**2 * qy**2 + qx**2 * qz**2 + qy**2 * qz**2
    a_q4, a_p22 = sp.symbols("a_q4 a_p22")
    theta2_ansatz = c2 * eps**2 * s2 + eps**4 * (a_q4 * q4 + a_p22 * p22)

    # 4 sin^2(theta/2) = theta^2 - theta^4/12 + O(theta^6).
    lhs = theta2_ansatz - theta2_ansatz**2 / 12
    rhs = c2 * (eps**2 * s2 - eps**4 * q4 / 12)
    residual4 = sp.expand(lhs - rhs).coeff(eps, 4)
    poly = sp.Poly(residual4, qx, qy, qz)
    solution = sp.solve(
        (
            sp.Eq(poly.coeff_monomial(qx**4), 0),
            sp.Eq(poly.coeff_monomial(qx**2 * qy**2), 0),
        ),
        (a_q4, a_p22),
        dict=True,
    )
    expected = {
        a_q4: -sp.Rational(1, 98),
        a_p22: sp.Rational(1, 294),
    }
    require(solution == [expected],
            "P3 exact series reversion fixes the q^4 photon tensor")
    checks += 1

    normalized_q4 = sp.expand(
        (expected[a_q4] * q4 + expected[a_p22] * p22) / c2
    )
    require(normalized_q4 == -q4 / 14 + p22 / 42,
            "P4 normalized pole is S2-Q4/14+P22/42+O(q^6)")
    checks += 1

    # Directional group velocity coefficient.
    r4 = sp.symbols("R_4", real=True)
    pole_directional = sp.simplify(
        (-r4 / 14 + (1 - r4) / 84)
    )
    require(pole_directional == (1 - 7 * r4) / 84,
            "V1 directional phase-squared coefficient is (1-7R4)/84")
    checks += 1
    group_coefficient = sp.Rational(3, 2) * pole_directional
    require(group_coefficient == (1 - 7 * r4) / 56,
            "V2 photon group-speed correction is (1-7R4)q^2/56")
    checks += 1
    axis = sp.simplify(group_coefficient.subs(r4, 1))
    body = sp.simplify(group_coefficient.subs(r4, sp.Rational(1, 3)))
    require(axis == -sp.Rational(3, 28),
            "V3 largest matter/photon leading gap is 3q^2/28 on an axis")
    checks += 1
    require(sp.simplify(body - axis) == sp.Rational(1, 12),
            "V4 axis-to-body-diagonal photon spread is q^2/12")
    checks += 1

    # A site-centred display average is local but loses the antisymmetric
    # link component, so it cannot invert the connection/flux field.
    e_plus, e_minus = sp.symbols("E_plus E_minus")
    j_display = (e_plus + e_minus) / 2
    require(sp.simplify(j_display.subs({e_plus: 1, e_minus: -1})) == 0,
            "J1 the local site display map has a nontrivial kernel")
    checks += 1

    # Documentation/source contract.
    audit = read("docs/theory/07_assessment/AUDIT_LORENTZ_LOCAL_LINK_FLUX.md")
    rg_audit = read("docs/theory/07_assessment/AUDIT_LORENTZ_RG_ATTRACTION.md")
    bridge = read(
        "docs/theory/10_eft_program/derivations/"
        "DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md"
    )
    import_ledger = read("docs/theory/01_reference/import_ledger.json")
    charge_gate = read(
        "docs/theory/10_eft_program/archive/closed_negative/"
        "AUDIT_NATIVE_CONSERVED_CHARGE_GATE.md"
    )

    require("[SELECTED ONTOLOGY EXTENSION]" in audit
            and "one new continuous link type" in audit,
            "S1 the added link type is priced rather than presented as derived")
    checks += 1
    require("IMPROVED-CONE-SACRIFICED" in audit
            and "3q^2/28" in audit and "q^2/12" in audit,
            "S2 the loss of the q^4-improved cone is explicit and quantified")
    checks += 1
    require("dim\\ker M=0" in charge_gate and "CLOSED NEGATIVE" in charge_gate,
            "S3 FTD-0421 closes the frozen native additive-current route negative")
    checks += 1
    require("FTD-0419 now performs that integration" in audit
            and "A counterterm is required" in audit
            and "on-shell match" in audit,
            "S4 successors close one step scheme but retain the physical match gate")
    checks += 1
    require("A_mu = P_T J_mu" in rg_audit and "spatially nonlocal" in rg_audit,
            "S5 FTD-0416's projector obstruction remains on record")
    checks += 1
    require("Locality ceiling (FTD-0416)" in bridge,
            "S6 the old projection bridge retains its scoped locality ceiling")
    checks += 1
    require("gauge-connection carrier choice" in import_ledger
            and "FTD-0417 broadens IMP-S4" in import_ledger,
            "S7 the new carrier branch is priced under IMP-S4 without double counting")
    checks += 1

    print()
    print(f"FTD-0417 exact/source checks: {checks}/{checks} passed")
    print("ACTION   noncompact unit-plaquette U(1), c_A^2=1/7 [SELECTED]")
    print("POLE     4 sin^2(theta/2)=(1/7) 4 sum_i sin^2(q_i/2)")
    print("TREE     max matter/photon gap=3(ka)^2/28; directional spread=(ka)^2/12")
    print("STATUS   LOCAL PHOTON SELECTED; NATIVE CURRENT CLOSED; PHYSICAL ON-SHELL OPEN")


if __name__ == "__main__":
    main()
