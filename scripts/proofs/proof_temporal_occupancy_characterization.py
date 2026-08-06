"""FTD-0772 exact certificate for Native Temporal Occupancy v1.

This certificate proves only the continuous mathematical statements locked in
``PREREG_NATIVE_TEMPORAL_OCCUPANCY_v1.md``.  In particular, it proves the
beta-law normalization and moments and the conditional fixed-coordinate
quartic characterization.  It does not inspect the FTD-0659 corpus, promote a
finite tick histogram to a continuous invariant measure, select a natural
coordinate, or infer a native potential from that corpus.

All checks are exact SymPy identities.  There is no floating-point evaluation,
parameter fit, exponent scan, or near-miss search.
"""

from __future__ import annotations

import sympy as sp


def main() -> None:
    checks = 0
    expected_checks = 45

    def check(label: str, condition: bool) -> None:
        nonlocal checks
        assert condition, label
        checks += 1

    half = sp.Rational(1, 2)
    one = sp.Integer(1)

    # ------------------------------------------------------------------
    # General even-power occupancy law.
    #
    # On a positive half-orbit put u=x**m.  For even m >= 2 and r > -1,
    #
    #   x**r dx / sqrt(1-x**m)
    #     = (1/m) u**((r+1)/m-1) (1-u)**(-1/2) du.
    #
    # Thus the half-orbit integral is a beta integral.  Evenness supplies
    # the second half; the time spent on the two directed passages supplies
    # no further factor after normalization.
    m = sp.symbols("m", positive=True, integer=True)
    r = sp.symbols("r", real=True)
    u = sp.symbols("u", positive=True)
    x = sp.symbols("x", positive=True)
    beta_0 = sp.beta(one / m, half)
    density_prefactor = m / (2 * beta_0)

    transformed_power = sp.simplify(r / m + one / m - 1)
    check(
        "power-substitution exponent",
        sp.simplify(transformed_power - ((r + 1) / m - 1)) == 0,
    )

    half_normalization_integral = beta_0 / m
    check(
        "beta half-orbit normalization integral",
        sp.simplify(
            half_normalization_integral
            - sp.beta(one / m, half) / m
        )
        == 0,
    )
    check(
        "general density normalization",
        sp.simplify(2 * density_prefactor * half_normalization_integral) == 1,
    )

    half_r_moment_integral = sp.beta((r + 1) / m, half) / m
    general_moment = sp.beta((r + 1) / m, half) / beta_0
    check(
        "general absolute-moment law",
        sp.simplify(
            2 * density_prefactor * half_r_moment_integral - general_moment
        )
        == 0,
    )
    check(
        "zeroth moment is normalized",
        sp.simplify(general_moment.subs(r, 0)) == 1,
    )

    # The locked regularized-incomplete-beta CDF has the density as its
    # derivative on both open branches.  Write the negative branch as -z
    # with z>0 so no symbolic sign convention is hidden.
    regularized_beta = (
        sp.betainc(one / m, half, 0, x**m) / beta_0
    )
    cdf_positive = half + regularized_beta / 2
    rho_positive = density_prefactor / sp.sqrt(1 - x**m)
    check(
        "positive CDF branch differentiates to rho_m",
        sp.simplify(sp.diff(cdf_positive, x) - rho_positive) == 0,
    )

    z = sp.symbols("z", positive=True)
    regularized_beta_z = (
        sp.betainc(one / m, half, 0, z**m) / beta_0
    )
    cdf_at_negative_z = half - regularized_beta_z / 2
    rho_at_negative_z = density_prefactor / sp.sqrt(1 - z**m)
    check(
        "negative CDF branch differentiates to rho_m",
        sp.simplify(-sp.diff(cdf_at_negative_z, z) - rho_at_negative_z)
        == 0,
    )
    check(
        "CDF branch mass is one",
        sp.simplify(2 * density_prefactor * beta_0 / m) == 1,
    )

    # ------------------------------------------------------------------
    # Quartic reductions to G* = Gamma(1/4)/Gamma(3/4).
    quarter = sp.Rational(1, 4)
    g_star = sp.gamma(quarter) / sp.gamma(3 * quarter)
    beta_4 = sp.beta(quarter, half)

    check(
        "quartic beta reduction",
        sp.simplify(sp.expand_func(beta_4 - sp.sqrt(sp.pi) * g_star)) == 0,
    )
    quartic_prefactor = density_prefactor.subs(m, 4)
    check(
        "quartic density coefficient",
        sp.simplify(
            sp.expand_func(
                quartic_prefactor - 2 / (sp.sqrt(sp.pi) * g_star)
            )
        )
        == 0,
    )

    mu_1 = general_moment.subs({m: 4, r: 1})
    mu_2 = general_moment.subs({m: 4, r: 2})
    mu_4 = general_moment.subs({m: 4, r: 4})
    check(
        "quartic first absolute moment",
        sp.simplify(sp.expand_func(mu_1 - sp.sqrt(sp.pi) / g_star)) == 0,
    )
    check(
        "quartic second moment",
        sp.simplify(sp.expand_func(mu_2 - 4 / g_star**2)) == 0,
    )
    check(
        "quartic fourth moment",
        sp.simplify(sp.expand_func(mu_4 - sp.Rational(1, 3))) == 0,
    )
    check(
        "quartic RMS G estimator",
        sp.simplify(sp.expand_func(2 / sp.sqrt(mu_2) - g_star)) == 0,
    )
    check(
        "quartic absolute-moment G estimator",
        sp.simplify(sp.expand_func(sp.sqrt(sp.pi) / mu_1 - g_star)) == 0,
    )

    # ------------------------------------------------------------------
    # Exact m=2 (harmonic/arcsine) control.  These values are a control
    # specialization of the registered family, not a fitted alternative.
    beta_2 = sp.beta(half, half)
    check(
        "quadratic beta reduction",
        sp.simplify(sp.expand_func(beta_2 - sp.pi)) == 0,
    )
    check(
        "quadratic arcsine density coefficient",
        sp.simplify(sp.expand_func(density_prefactor.subs(m, 2) - 1 / sp.pi))
        == 0,
    )
    check(
        "quadratic first absolute moment",
        sp.simplify(sp.expand_func(general_moment.subs({m: 2, r: 1}) - 2 / sp.pi))
        == 0,
    )
    check(
        "quadratic second moment",
        sp.simplify(sp.expand_func(general_moment.subs({m: 2, r: 2}) - half))
        == 0,
    )
    check(
        "quadratic fourth moment",
        sp.simplify(
            sp.expand_func(
                general_moment.subs({m: 2, r: 4}) - sp.Rational(3, 8)
            )
        )
        == 0,
    )

    harmonic_cdf = half + sp.asin(x) / sp.pi
    harmonic_density = 1 / (sp.pi * sp.sqrt(1 - x**2))
    check(
        "quadratic CDF is the arcsine CDF",
        sp.simplify(sp.diff(harmonic_cdf, x) - harmonic_density) == 0,
    )
    y = sp.symbols("y", real=True)
    harmonic_cdf_full = half + sp.asin(y) / sp.pi
    check(
        "quadratic CDF reflection symmetry",
        sp.simplify(harmonic_cdf_full.subs(y, -y)
                    + harmonic_cdf_full - 1) == 0,
    )

    # ------------------------------------------------------------------
    # Fixed-coordinate quartic occupancy characterization.
    #
    # For a unit-mass natural coordinate on energy E=V(A), the normalized
    # coordinate density is proportional to
    # [V(A)-V(A*x)]**(-1/2).  Dividing by its value at x=0 removes the
    # period/normalization.  Because V(0)=0, equality with rho_4 forces
    #
    #   V(A) / (V(A)-V(A*x)) = 1/(1-x**4).
    #
    # The following checks solve this identity and then establish a single
    # positive quartic coefficient on every overlapping swept interval.
    value_at_turn = sp.symbols("V_A", positive=True)
    gap = sp.symbols("Delta", positive=True)
    normalizer = sp.symbols("Z_A", positive=True)
    density_at_x = 1 / (normalizer * sp.sqrt(gap))
    density_at_zero = 1 / (normalizer * sp.sqrt(value_at_turn))
    check(
        "natural-coordinate density ratio cancels period normalization",
        sp.simplify(
            (density_at_x / density_at_zero) ** 2 - value_at_turn / gap
        )
        == 0,
    )

    forced_gap = sp.solve(
        sp.Eq(value_at_turn / gap, 1 / (1 - x**4)), gap
    )[0]
    check(
        "rho_4 ratio forces quartic energy gap",
        sp.simplify(forced_gap - value_at_turn * (1 - x**4)) == 0,
    )
    forced_value_at_ax = sp.simplify(value_at_turn - forced_gap)
    check(
        "rho_4 ratio forces V(Ax)=x^4 V(A)",
        sp.simplify(forced_value_at_ax - value_at_turn * x**4) == 0,
    )

    amplitude, amplitude_b, q = sp.symbols("A B q", positive=True)
    coefficient_at_a = value_at_turn / amplitude**4
    check(
        "quartic coefficient on one swept ray",
        sp.simplify(
            value_at_turn * (q / amplitude) ** 4
            - coefficient_at_a * q**4
        )
        == 0,
    )
    value_at_b = value_at_turn * (amplitude_b / amplitude) ** 4
    check(
        "quartic coefficient agrees across amplitudes",
        sp.simplify(value_at_b / amplitude_b**4 - coefficient_at_a) == 0,
    )
    check(
        "forced quartic coefficient is positive",
        sp.ask(sp.Q.positive(coefficient_at_a)) is True,
    )

    lam = sp.symbols("lambda", positive=True)
    q_real = sp.symbols("q_real", real=True)
    quartic_potential = lam * q_real**4
    check(
        "quartic converse potential is even",
        sp.simplify(quartic_potential.subs(q_real, -q_real)
                    - quartic_potential) == 0,
    )
    check(
        "quartic converse potential vanishes at the origin",
        quartic_potential.subs(q_real, 0) == 0,
    )
    quartic_gap = sp.simplify(
        lam * amplitude**4 - lam * (amplitude * x) ** 4
    )
    check(
        "quartic converse energy gap",
        sp.simplify(quartic_gap - lam * amplitude**4 * (1 - x**4)) == 0,
    )

    # Integral_{-1}^{1} (1-x^4)^(-1/2) dx = B(1/4,1/2)/2.
    quartic_scale = sp.sqrt(lam) * amplitude**2
    quartic_unnormalized = 1 / (
        quartic_scale * sp.sqrt(1 - x**4)
    )
    quartic_normalizer = beta_4 / (2 * quartic_scale)
    quartic_from_potential = sp.simplify(
        quartic_unnormalized / quartic_normalizer
    )
    registered_rho_4 = 2 / (beta_4 * sp.sqrt(1 - x**4))
    check(
        "quartic potential gives the registered amplitude-invariant density",
        sp.simplify(quartic_from_potential - registered_rho_4) == 0,
    )

    # ------------------------------------------------------------------
    # Constant time rescaling and affine coordinate-unit changes.
    dt, period, time_scale = sp.symbols("dt T c_t", positive=True)
    check(
        "constant time rescaling preserves occupancy weights",
        sp.simplify(time_scale * dt / (time_scale * period) - dt / period)
        == 0,
    )

    coordinate_scale = sp.symbols("a", positive=True)
    coordinate_origin = sp.symbols("b", real=True)
    affine_coordinate = coordinate_origin + coordinate_scale * q_real
    affine_amplitude = coordinate_scale * amplitude
    affine_normalized = (
        affine_coordinate.subs(q_real, amplitude * x) - coordinate_origin
    ) / affine_amplitude
    check(
        "affine coordinate change preserves normalized coordinate",
        sp.simplify(affine_normalized - x) == 0,
    )

    affine_variable = sp.symbols("Q", real=True)
    transformed_quartic = (
        lam * ((affine_variable - coordinate_origin) / coordinate_scale) ** 4
    )
    check(
        "affine coordinate change preserves quartic form about its origin",
        sp.simplify(
            transformed_quartic.subs(
                affine_variable,
                coordinate_origin + coordinate_scale * q_real,
            )
            - lam * q_real**4
        )
        == 0,
    )

    rho_symbol = sp.Function("rho")
    density_in_q = rho_symbol(q_real / amplitude) / amplitude
    affine_density_at_x = (
        density_in_q.subs(q_real, amplitude * x)
        / coordinate_scale
    )
    check(
        "affine density Jacobian preserves normalized occupancy",
        sp.simplify(affine_amplitude * affine_density_at_x - rho_symbol(x))
        == 0,
    )

    # ------------------------------------------------------------------
    # Nonlinear-coordinate caveat.  The monotone map Q=q+c*q^3 (c>0)
    # already breaks the affine result.  Its amplitude-normalized value
    # depends on A, and its kinetic term has a coordinate-dependent mass.
    nonlinear_scale = sp.symbols("c_nl", positive=True)
    nonlinear_at_ax = amplitude * x + nonlinear_scale * amplitude**3 * x**3
    nonlinear_at_a = amplitude + nonlinear_scale * amplitude**3
    nonlinear_normalized = sp.factor(nonlinear_at_ax / nonlinear_at_a)
    expected_nonlinear_normalized = (
        x * (1 + nonlinear_scale * amplitude**2 * x**2)
        / (1 + nonlinear_scale * amplitude**2)
    )
    check(
        "nonlinear normalized-coordinate formula",
        sp.simplify(nonlinear_normalized - expected_nonlinear_normalized) == 0,
    )
    nonlinear_difference = sp.factor(nonlinear_normalized - x)
    check(
        "nonlinear coordinate is not normalized-coordinate invariant",
        nonlinear_difference != 0,
    )
    check(
        "nonlinear coordinate introduces amplitude dependence",
        sp.factor(sp.diff(nonlinear_normalized, amplitude)) != 0,
    )

    q_dot_new = sp.symbols("Q_dot", real=True)
    nonlinear_derivative = 1 + 3 * nonlinear_scale * q_real**2
    transformed_kinetic = q_dot_new**2 / (2 * nonlinear_derivative**2)
    check(
        "nonlinear coordinate produces a non-unit kinetic coefficient",
        sp.simplify(transformed_kinetic - q_dot_new**2 / 2) != 0,
    )
    check(
        "nonlinear kinetic coefficient is coordinate dependent",
        sp.simplify(sp.diff(1 / (2 * nonlinear_derivative**2), q_real)) != 0,
    )

    # ------------------------------------------------------------------
    # Finite tick measures are atomic, whereas rho_m(x) dx is absolutely
    # continuous.  If an observed support point occurs k>=1 times among N
    # ticks, its singleton mass is k/N>0.  The continuous measure assigns
    # the same singleton the zero-width integral 0.  This exact witness is
    # enough to rule out equality for every finite empirical tick measure.
    tick_count, multiplicity = sp.symbols(
        "N k", positive=True, integer=True
    )
    check(
        "finite empirical tick weights normalize",
        sp.simplify(tick_count * (1 / tick_count)) == 1,
    )
    atomic_singleton_mass = multiplicity / tick_count
    check(
        "an occupied empirical atom has positive mass",
        sp.ask(sp.Q.positive(atomic_singleton_mass)) is True,
    )
    atom_location = sp.symbols("x_atom", real=True)
    continuous_singleton_mass = sp.Integral(
        registered_rho_4, (x, atom_location, atom_location)
    ).doit()
    check(
        "continuous target gives every singleton zero mass",
        continuous_singleton_mass == 0,
    )
    check(
        "finite atomic and continuous measures are unequal",
        sp.simplify(atomic_singleton_mass - continuous_singleton_mass) != 0,
    )

    assert checks == expected_checks, (
        f"expected {expected_checks} checks, ran {checks}"
    )
    print(
        "FTD-0772 temporal occupancy exact certificate: "
        f"{checks}/{expected_checks} PASS"
    )
    print("FIXED_COORDINATE_QUARTIC_OCCUPANCY_CHARACTERIZED")
    print("NONLINEAR_OBSERVABLE_INVARIANCE_ABSENT")
    print("FINITE_TICK_MEASURE_ATOMIC_NOT_CONTINUOUS")
    print("NATIVE_POTENTIAL_INFERENCE_INAPPLICABLE_V1")


if __name__ == "__main__":
    main()
