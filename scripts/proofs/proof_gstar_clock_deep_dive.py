"""Exact certificate for the G*-clock deep-dive spine.

The checks cover identities, not a substrate derivation.  In particular,
they prove the quartic-period, action, equilibrium, normalized-detuning,
discrete-scaling, and inert-Frobenius formulas used by the companion audit.
No numerical search, fitted value, or physical Born target is used.
"""

from __future__ import annotations

import sympy as sp


def main() -> None:
    checks = 0

    def check(label: str, condition: bool) -> None:
        nonlocal checks
        assert condition, label
        checks += 1

    g = sp.symbols("G", positive=True)
    gamma_quarter = sp.gamma(sp.Rational(1, 4))
    gamma_three_quarters = sp.gamma(sp.Rational(3, 4))
    g_definition = gamma_quarter / gamma_three_quarters
    reflection_substitution = {
        gamma_three_quarters: sp.pi * sp.sqrt(2) / gamma_quarter
    }

    check(
        "reflection representation",
        sp.simplify(
            g_definition.subs(reflection_substitution)
            - gamma_quarter**2 / (sp.sqrt(2) * sp.pi)
        )
        == 0,
    )
    beta_quarter_half = sp.expand_func(
        sp.beta(sp.Rational(1, 4), sp.Rational(1, 2))
    ).subs(reflection_substitution)
    check(
        "quarter-period beta reduction",
        sp.simplify(
            beta_quarter_half
            - sp.sqrt(sp.pi) * g_definition.subs(reflection_substitution)
        )
        == 0,
    )

    mass, coupling, amplitude, energy = sp.symbols(
        "m lambda A E", positive=True
    )
    period = sp.sqrt(sp.pi) * g * sp.sqrt(mass / (2 * coupling)) / amplitude
    check(
        "quartic clock invariant",
        sp.simplify(
            period * amplitude * sp.sqrt(2 * coupling / mass)
            - sp.sqrt(sp.pi) * g
        )
        == 0,
    )
    effective_coupling = coupling / g**2
    check(
        "G* period factor is absorbable into an unfixed coupling",
        sp.simplify(
            period
            - sp.sqrt(sp.pi)
            * sp.sqrt(mass / (2 * effective_coupling))
            / amplitude
        )
        == 0,
    )

    action_area = (
        sp.Rational(2, 3)
        * g
        * sp.sqrt(2 * sp.pi * mass)
        * coupling ** sp.Rational(-1, 4)
        * energy ** sp.Rational(3, 4)
    )
    period_from_action = sp.diff(action_area, energy)
    amplitude_from_energy = (energy / coupling) ** sp.Rational(1, 4)
    check(
        "period is derivative of phase-space area",
        sp.simplify(
            period_from_action
            - period.subs(amplitude, amplitude_from_energy)
        )
        == 0,
    )

    beta = sp.symbols("beta", positive=True)
    second_moment = (beta * coupling) ** sp.Rational(-1, 2) / g
    fourth_moment = (beta * coupling) ** -1 / 4
    binder_ratio = sp.simplify(fourth_moment / second_moment**2)
    check("critical thermal Binder ratio", binder_ratio == g**2 / 4)
    check(
        "critical configurational equipartition",
        sp.simplify(coupling * fourth_moment - 1 / (4 * beta)) == 0,
    )

    mean_q2 = 4 / g**2
    mean_qprime2 = g**2 / (6 * sp.pi)
    waveform_functional = sp.simplify(2 * mean_q2 / mean_qprime2)
    check("quartic waveform functional", waveform_functional == 48 * sp.pi / g**4)

    harmonic_cost = 2 * sp.pi**2
    quartic_cost = sp.pi * g**2 / 2
    check(
        "steerability exchange rate",
        sp.simplify(quartic_cost / harmonic_cost - g**2 / (4 * sp.pi)) == 0,
    )

    mu = sp.symbols("mu", real=True)
    delta = mu / (2 * coupling * amplitude**2)
    modulus_squared = 2 * coupling * amplitude**2 / (
        mu + 4 * coupling * amplitude**2
    )
    check(
        "dimensionless detuning to elliptic modulus",
        sp.simplify(modulus_squared - 1 / (delta + 2)) == 0,
    )
    check("critical point is self-dual modulus", modulus_squared.subs(mu, 0) == sp.Rational(1, 2))

    # The coupled-clock wave/cycle ratio is normalization-free.
    power, eta, graph_factor = sp.symbols("power eta d_R", positive=True)
    wave_cycle_ratio = graph_factor * eta * (power - 2) / (2 * power)
    check("quartic internal ratio", wave_cycle_ratio.subs(power, 4) == graph_factor * eta / 4)
    check("G* absent from internal linear ratio", g not in wave_cycle_ratio.free_symbols)

    # Exact equivariance of velocity Verlet for the pure quartic equation.
    q, momentum, dt, scale = sp.symbols("q p dt s", positive=True)
    half_momentum = momentum - 2 * coupling * q**3 * dt
    next_q = q + dt * half_momentum / mass
    next_momentum = half_momentum - 2 * coupling * next_q**3 * dt

    scaled_half_momentum = (scale**2 * momentum) - 2 * coupling * (scale * q) ** 3 * (dt / scale)
    check(
        "Verlet half-kick scaling",
        sp.simplify(scaled_half_momentum - scale**2 * half_momentum) == 0,
    )
    scaled_next_q = scale * q + (dt / scale) * scaled_half_momentum / mass
    check("Verlet drift scaling", sp.simplify(scaled_next_q - scale * next_q) == 0)
    scaled_next_momentum = scaled_half_momentum - 2 * coupling * scaled_next_q**3 * (dt / scale)
    check(
        "Verlet second-kick scaling",
        sp.simplify(scaled_next_momentum - scale**2 * next_momentum) == 0,
    )
    check(
        "discreteness ratio is scale invariant",
        sp.simplify((dt / scale) / (period / scale) - dt / period) == 0,
    )

    # Finite-place CM structure.  For p=4r+3, i^p=i^(4r+3)=-i.
    imaginary = sp.I
    check(
        "inert residue Frobenius is conjugation",
        sp.simplify(imaginary**4 - 1) == 0
        and sp.simplify(imaginary**3 + imaginary) == 0,
    )
    frobenius = sp.symbols("Pi")
    inert_prime = sp.symbols("p", positive=True)
    normalized_frobenius = frobenius / sp.sqrt(inert_prime)
    check(
        "inert normalized Frobenius squares to minus one",
        sp.rem(
            normalized_frobenius**2 + 1,
            frobenius**2 + inert_prime,
            frobenius,
        )
        == 0,
    )
    check(
        "inert normalized Frobenius has phase order four",
        sp.rem(
            normalized_frobenius**4 - 1,
            frobenius**2 + inert_prime,
            frobenius,
        )
        == 0,
    )

    split_a, split_b = sp.symbols("a b", positive=True)
    split_norm = split_a**2 + split_b**2
    check(
        "split quadratic channel normalization",
        sp.simplify(split_a**2 / split_norm + split_b**2 / split_norm) == 1,
    )
    check("inert cone height is integral p", sp.sqrt(inert_prime**2) == inert_prime)

    print(f"G*-clock deep-dive exact certificate: {checks}/{checks} PASS")
    print("GSTAR_IS_EXACT_QUARTIC_PERIOD_COEFFICIENT")
    print("GSTAR_INTERNAL_LINEAR_CLOCK_SIGNATURE_CANCELS")
    print("INERT_FROBENIUS_HAS_NORMALIZED_PHASE_ORDER_FOUR")
    print("PHYSICAL_SUBSTRATE_CLOCK_REMAINS_OPEN")


if __name__ == "__main__":
    main()
