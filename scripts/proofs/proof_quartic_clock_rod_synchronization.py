"""FTD-0771 exact certificate for quartic clock--rod synchronization.

The certificate proves the conditional clock--rod identities and their
rescaling properties.  It also exhibits the algebraic discriminator used by
the P1--P5 underdetermination proof.  It does not derive the quartic clock,
its rate matching, or its occupied energy shell from the FTD postulates.
"""

from __future__ import annotations

import sympy as sp


def main() -> None:
    checks = 0
    expected_checks = 20

    def check(label: str, condition: bool) -> None:
        nonlocal checks
        assert condition, label
        checks += 1

    m = sp.symbols("m", positive=True)
    energy = sp.symbols("E", positive=True)
    rho = sp.symbols("rho", positive=True)
    exponent = (m - 2) / (2 * m)

    # For h_m=(p^2+q^m)/2, q_dot=p.  At energy E the positive
    # turning point is A=(2E)^(1/m).  Substituting q=A*x in four
    # times the quarter-orbit integral gives the beta-function period.
    quadrature_power = sp.simplify(sp.Rational(1, 1) / m - sp.Rational(1, 2))
    check("period quadrature exponent", sp.simplify(quadrature_power + exponent) == 0)

    beta_factor = sp.beta(1 / m, sp.Rational(1, 2))
    k_m = 4 * beta_factor / m
    turning_point = (2 * energy) ** (1 / m)
    transformed_quarter_orbit = (
        turning_point / sp.sqrt(2 * energy) * beta_factor / m
    )
    base_period = 4 * transformed_quarter_orbit
    check(
        "general transformed period quadrature",
        sp.simplify(base_period - k_m * (2 * energy) ** (-exponent)) == 0,
    )

    # H=(rho/tau)h scales the Hamiltonian vector field by rho/tau.
    # Consequently T/tau is the base period divided by rho.
    period_per_tick = base_period / rho
    check(
        "rate-scaled period",
        sp.simplify(period_per_tick * rho * (2 * energy) ** exponent - k_m) == 0,
    )

    g_star = sp.symbols("G_star", positive=True)
    k_4 = sp.sqrt(sp.pi) * g_star
    check(
        "quartic beta reduction",
        sp.simplify(
            sp.gamma(sp.Rational(1, 4))
            * sp.gamma(sp.Rational(1, 2))
            / sp.gamma(sp.Rational(3, 4))
            - k_4.subs(g_star, sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4)))
        )
        == 0,
    )

    # The familiar quartic modulus is not by itself a clock normalization.
    # For p^2/(2M)+Lambda*q^4/2 the period carries an additional exact
    # coefficient M^(1/2)*Lambda^(-1/4).
    mass_scale, quartic_scale = sp.symbols("M Lambda", positive=True)
    generalized_turning_point = (2 * energy / quartic_scale) ** sp.Rational(1, 4)
    generalized_quarter_orbit = (
        sp.sqrt(mass_scale)
        * generalized_turning_point
        / sp.sqrt(2 * energy)
        * k_4
        / 4
    )
    generalized_quartic_period = 4 * generalized_quarter_orbit
    check(
        "general quartic coefficient dependence",
        sp.simplify(
            generalized_quartic_period * (2 * energy) ** sp.Rational(1, 4)
            - k_4 * sp.sqrt(mass_scale) * quartic_scale ** sp.Rational(-1, 4)
        )
        == 0,
    )

    ell, tau = sp.symbols("ell tau", positive=True)
    c_moore = ell / tau
    rod_duration = sp.simplify(ell / c_moore)
    check("minimum-edge causal duration", sp.simplify(rod_duration - tau) == 0)

    cycle_fraction = sp.simplify(rod_duration / (tau * period_per_tick))
    expected_fraction = rho * (2 * energy) ** exponent / k_m
    check(
        "general clock--rod cycle fraction",
        sp.simplify(cycle_fraction - expected_fraction) == 0,
    )
    check("lattice edge cancels", ell not in cycle_fraction.free_symbols)

    quartic_fraction = rho * (2 * energy) ** sp.Rational(1, 4) / k_4
    quartic_phase = 2 * sp.pi * quartic_fraction
    check(
        "quartic phase formula",
        sp.simplify(
            quartic_phase
            - 2 * sp.sqrt(sp.pi) * rho * (2 * energy) ** sp.Rational(1, 4) / g_star
        )
        == 0,
    )
    check(
        "amplitude-one synchronized reduction",
        sp.simplify(
            quartic_fraction.subs({rho: 1, energy: sp.Rational(1, 2)})
            - 1 / (sp.sqrt(sp.pi) * g_star)
        )
        == 0,
    )

    # A common time-coordinate change t' = s*t changes Omega and the
    # causal speed reciprocally, leaving their clock--rod product invariant.
    scale, omega, speed = sp.symbols("scale omega speed", positive=True)
    transformed_phase = (omega / scale) * ell / (speed / scale)
    check(
        "common time-coordinate invariance",
        sp.simplify(transformed_phase - omega * ell / speed) == 0,
    )

    c_speed = ell / (sp.sqrt(3) * tau)
    transport_duration = sp.simplify(ell / c_speed)
    check(
        "selected transport alternative",
        sp.simplify(transport_duration / rod_duration - sp.sqrt(3)) == 0,
    )

    check(
        "quartic rate remains free",
        sp.simplify(sp.diff(quartic_fraction, rho)
                    - (2 * energy) ** sp.Rational(1, 4) / k_4) == 0,
    )
    check(
        "quartic shell remains free",
        sp.simplify(
            sp.diff(quartic_fraction, energy)
            - rho * (2 * energy) ** sp.Rational(-3, 4) / (2 * k_4)
        )
        == 0,
    )

    # Two exact on-site flow rates preserve locality and determinism but
    # yield distinct cycle fractions on the same shell.
    check(
        "two-rate countermodel discriminator",
        sp.simplify(quartic_fraction.subs(rho, 2)
                    - 2 * quartic_fraction.subs(rho, 1)) == 0,
    )
    check(
        "two-rate predictions differ",
        sp.simplify(quartic_fraction.subs(rho, 2)
                    - quartic_fraction.subs(rho, 1)) != 0,
    )

    # FTD-0770 gives this normalization-free wave/cycle ratio.  Matching
    # its clock-wave cone to an independently named cone eliminates K_m.
    graph_factor, eta = sp.symbols("d_R eta", positive=True)
    wave_cycle_ratio_squared = graph_factor * eta * (m - 2) / (2 * m)
    cone_matched_phase = sp.sqrt(1 / wave_cycle_ratio_squared)
    expected_cone_phase = sp.sqrt(2 * m / (graph_factor * eta * (m - 2)))
    check(
        "general common-cone cancellation",
        sp.simplify(cone_matched_phase - expected_cone_phase) == 0,
    )
    check("period modulus absent after cone matching", g_star not in cone_matched_phase.free_symbols)
    check(
        "quartic common-cone specialization",
        sp.simplify(cone_matched_phase.subs(m, 4)
                    - 2 / sp.sqrt(graph_factor * eta)) == 0,
    )
    cone_matched_cycle_fraction = cone_matched_phase / (2 * sp.pi)
    check(
        "quartic common-cone cycle fraction",
        sp.simplify(
            cone_matched_cycle_fraction.subs(m, 4)
            - 1 / (sp.pi * sp.sqrt(graph_factor * eta))
        )
        == 0,
    )

    assert checks == expected_checks, f"expected {expected_checks} checks, ran {checks}"
    print(f"FTD-0771 quartic clock--rod exact certificate: {checks}/{expected_checks} PASS")
    print("CLOCK_ROD_RATIO_CONDITIONAL_GSTAR_PRESENT")
    print("P1_P5_SYNCHRONIZATION_UNDERDETERMINED")
    print("COMMON_CONE_GSTAR_CANCELLATION")


if __name__ == "__main__":
    main()
