#!/usr/bin/env python3
"""Exact finite-clock/packet coupling ladder and normalization boundary.

For a momentum-neutral absorption event, the selected reciprocal vertex gives

    omega I_* = d Gamma,
    chi_EM = Gamma/I_*.

If the receiving material clock is a finite phase cycle with total winding w
over T global ticks, omega=2*pi*w/T.  With the certified cotangent cone
c_eff=1/6, the conditional native coupling is therefore

    alpha_native = chi_EM/(4*pi*c_eff) = 3*w/(d*T).

Thus clock cadence plus integer packet debit gives an exact rational ladder,
not a unique coupling.  Tick refinement leaves the ratio invariant, and
distinct integer triples can yield the same value.  Recoil restores an
additional free dimensionless ratio.  No target value, master root, fit, or
near-miss search enters this certificate.
"""

from __future__ import annotations

import sys
from fractions import Fraction

from sympy import Rational, Symbol, pi, simplify


sys.stdout.reconfigure(encoding="utf-8")


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    winding = Symbol("w", integer=True, positive=True)
    debit = Symbol("d", integer=True, positive=True)
    period = Symbol("T", integer=True, positive=True)
    action_quantum = Symbol("I_star", positive=True)
    packet_energy = Symbol("Gamma", positive=True)
    c_eff = Rational(1, 6)

    omega = 2 * pi * winding / period
    compliance_packet_energy = simplify(omega * action_quantum / debit)
    chi = simplify(compliance_packet_energy / action_quantum)
    alpha = simplify(chi / (4 * pi * c_eff))

    check(
        "C1 finite phase winding fixes the global-tick angular cadence",
        omega == 2 * pi * winding / period,
    )
    check(
        "C2 momentum-neutral packet/clock compliance gives chi_EM=omega/d",
        chi == 2 * pi * winding / (debit * period),
    )
    check(
        "C3 cotangent c_eff=1/6 cancels pi and yields alpha=3w/(dT)",
        alpha == 3 * winding / (debit * period),
    )

    # Exact rational fixtures are structural examples only.  They are not
    # compared with a physical target.
    fixtures = {
        (1, 1, 4): Fraction(3, 4),
        (1, 1, 6): Fraction(1, 2),
        (1, 1, 8): Fraction(3, 8),
        (1, 2, 8): Fraction(3, 16),
        (2, 3, 12): Fraction(1, 6),
    }
    for (w_value, d_value, t_value), expected in fixtures.items():
        actual = Fraction(3 * w_value, d_value * t_value)
        assert actual == expected
    check(
        "C4 every finite-clock/integer-debit fixture lies on the rational ladder",
        len(fixtures) == 5,
    )

    # Refining the global tick by q while proportionally refining both the
    # represented period and winding leaves physical cadence and coupling
    # unchanged.
    refinement = Symbol("q", integer=True, positive=True)
    refined_alpha = simplify(
        3 * (refinement * winding) / (debit * refinement * period)
    )
    check(
        "C5 proportional tick refinement leaves the coupling ratio invariant",
        refined_alpha == alpha,
    )

    # The integer data are not selected by the compliance identity.  Scaling
    # winding and packet debit together leaves the same alpha, as does scaling
    # winding and period together.
    family_a = simplify(
        3 * (refinement * winding) / ((refinement * debit) * period)
    )
    family_b = simplify(
        3 * (refinement * winding) / (debit * (refinement * period))
    )
    check(
        "C6 infinitely many integer presentations can encode the same ladder value",
        family_a == alpha and family_b == alpha,
    )

    # A primitive cycle may impose gcd(w,T)=1, but it still leaves T and the
    # positive packet debit d as independent structural selections.
    primitive_examples = (
        (1, 5, 1),
        (2, 5, 1),
        (1, 7, 2),
        (3, 8, 3),
    )
    assert all(0 < w_value < t_value and d_value > 0 for w_value, t_value, d_value in primitive_examples)
    primitive_values = {
        Fraction(3 * w_value, d_value * t_value)
        for w_value, t_value, d_value in primitive_examples
    }
    check(
        "C7 primitive winding alone does not select period or packet debit",
        len(primitive_values) == len(primitive_examples),
    )

    # Recoil-corrected compliance contains the independent dimensionless
    # ratio r=|p|^2/(2m Gamma).  The finite clock does not determine r.
    recoil_ratio = Symbol("r", nonnegative=True)
    chi_recoil = simplify(omega / (debit - recoil_ratio))
    alpha_recoil = simplify(chi_recoil / (4 * pi * c_eff))
    check(
        "C8 recoil reintroduces an unfixed dimensionless response ratio",
        alpha_recoil == 3 * winding / (period * (debit - recoil_ratio))
        and simplify(alpha_recoil.subs(recoil_ratio, 0) - alpha) == 0,
    )

    # A generic field/action rescaling remains invisible unless the clock
    # action quantum participates in the same compliance relation.
    scale = Symbol("lambda", positive=True)
    scaled_ratio = simplify(
        (scale * packet_energy) / (scale * action_quantum)
    )
    check(
        "C9 common energy/action rescaling cancels but relative normalization remains physical",
        scaled_ratio == packet_energy / action_quantum,
    )

    missing = {
        "native selection of the material phase winding",
        "native selection of the operational clock period",
        "native selection of packet debit multiplicity",
        "derivation of recoil ratio or proof it vanishes",
        "one finite transaction realizing the charged pole and clock vertex",
        "blind equality of static and radiative curvature",
    }
    check(
        "C10 physical coupling normalization remains open at six structural debts",
        len(missing) == 6,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} finite-clock coupling-ladder checks pass")
    print("conditional_ladder=alpha_native=3*w/(d*T)")
    print("clock_result=finite cadence narrows normalization to integer data but does not select it")
    print("recoil_result=alpha_native=3*w/[T*(d-r)] with r unfixed")
    print("target_firewall=no master root, empirical alpha, fit, or near-miss search used")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
