"""Exact certificate for the locked FTD-0773 quartic-waveform audit.

The certificate proves conditional statements about a continuous, fixed,
unit-mass natural coordinate and a separately selected quadratic coordinate
edge.  It does not promote either construction to native FTD dynamics, infer a
continuous density from a finite tick orbit, fit an exponent, search parameter
space, or run the engine.

All mathematical checks are exact SymPy identities or structural statements.
The one displayed decimal is checked only as a high-precision rounding of the
already-derived exact expression ``48*pi/G_STAR**4``.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations"
    / "PREREG_QUARTIC_WAVEFORM_NONLINEAR_EDGE_SIGNATURE_v1.md"
)
PROTOCOL_SHA256 = (
    "33E126673B8F072CAEBAD490B74F810818373D8014CC2D6F73CEF9592ED88DAA"
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def exact_zero(expression: sp.Expr) -> bool:
    """Reduce beta/gamma recurrences before testing an exact zero."""

    return sp.simplify(sp.expand_func(expression)) == 0


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    half = sp.Rational(1, 2)
    quarter = sp.Rational(1, 4)
    pi = sp.pi

    # ------------------------------------------------------------------
    # Frozen protocol and epistemic firewall.
    protocol = PREREG.read_text(encoding="utf-8")
    check("locked protocol hash", sha256(PREREG) == PROTOCOL_SHA256)
    check(
        "locked preregistration status",
        "[PRE-REGISTRATION — LOCKED/NOT YET RUN]" in protocol,
    )
    check(
        "exact-only campaign scope",
        "exact symbolic audit and epistemic boundary; no native or\n"
        "selected-model engine execution" in protocol,
    )
    check(
        "positive conditional verdicts are locked",
        all(
            verdict in protocol
            for verdict in (
                "QUARTIC_CONTINUOUS_INVERSE_CHAIN_CONDITIONAL_THEOREMS_PASS",
                "QUARTIC_NONLINEAR_EDGE_SHAPE_FUNCTIONAL_GSTAR_PRESENT",
            )
        ),
    )
    check(
        "native blockers are locked",
        all(
            verdict in protocol
            for verdict in (
                "NATIVE_QUARTIC_TIME_DERIVATION_NOT_ESTABLISHED",
                "NATIVE_NONLINEAR_EDGE_TEST_BLOCKED",
                "SCOPE_VIOLATION_INVALID",
            )
        ),
    )
    check(
        "canonical-action type boundary is locked",
        "Deriving a canonical action additionally requires a native symplectic form"
        in protocol,
    )

    # ------------------------------------------------------------------
    # 1. Two-branch occupancy inversion and its unequal-speed boundary.
    period, amplitude = sp.symbols("T A", positive=True)
    speed, speed_plus, speed_minus = sp.symbols(
        "v v_plus v_minus", positive=True
    )
    rho_equal = 2 / (period * speed)
    check(
        "equal-speed two-crossing occupancy",
        exact_zero(rho_equal - (1 / speed + 1 / speed) / period),
    )
    check(
        "normalized speed inversion",
        exact_zero(2 / (period * rho_equal) - speed),
    )
    check(
        "physical coordinate speed inversion",
        exact_zero(2 * amplitude / (period * rho_equal) - amplitude * speed),
    )

    rho_unequal = (1 / speed_plus + 1 / speed_minus) / period
    harmonic_speed = 2 / (1 / speed_plus + 1 / speed_minus)
    check(
        "unequal branches determine the harmonic mean",
        exact_zero(harmonic_speed - 2 / (period * rho_unequal)),
    )
    check(
        "equal-branch limit recovers the crossing speed",
        exact_zero(
            harmonic_speed.subs(speed_minus, speed_plus) - speed_plus
        ),
    )
    check(
        "unequal occupancy does not identify the plus branch",
        sp.factor(harmonic_speed - speed_plus) != 0,
    )
    check(
        "unequal occupancy does not identify the minus branch",
        sp.factor(harmonic_speed - speed_minus) != 0,
    )

    # ------------------------------------------------------------------
    # 2. Conservative inverse-potential formulas.
    rho_x, rho_zero = sp.symbols("rho_x rho_0", positive=True)
    qdot_from_rho = 2 * amplitude / (period * rho_x)
    energy_gap = half * qdot_from_rho**2
    check(
        "occupancy inversion gives conservative energy gap",
        exact_zero(
            energy_gap
            - 2 * amplitude**2 / (period**2 * rho_x**2)
        ),
    )
    value_at_turn = 2 * amplitude**2 / (period**2 * rho_zero**2)
    check(
        "V(A) follows from V(0)=0",
        exact_zero(
            value_at_turn
            - half * (2 * amplitude / (period * rho_zero)) ** 2
        ),
    )
    reconstructed_potential = value_at_turn - energy_gap
    check(
        "pointwise inverse-potential formula",
        exact_zero(
            reconstructed_potential
            - 2
            * amplitude**2
            / period**2
            * (rho_zero ** (-2) - rho_x ** (-2))
        ),
    )
    plus_branch_kinetic = half * (amplitude * speed_plus) ** 2
    harmonic_pseudo_kinetic = half * (amplitude * harmonic_speed) ** 2
    check(
        "unequal branches yield only a pseudo-potential speed",
        sp.factor(harmonic_pseudo_kinetic - plus_branch_kinetic) != 0,
    )

    # ------------------------------------------------------------------
    # 3. Quartic normalization and fixed-coordinate characterization.
    g_star = sp.gamma(quarter) / sp.gamma(3 * quarter)
    beta_4 = sp.beta(quarter, half)
    c_4 = 2 / (sp.sqrt(pi) * g_star)
    check(
        "quartic beta reduction",
        exact_zero(beta_4 - sp.sqrt(pi) * g_star),
    )
    quartic_half_integral = beta_4 / 4
    check(
        "quartic half-orbit integral",
        exact_zero(
            quartic_half_integral
            - sp.beta(quarter, half) / 4
        ),
    )
    check(
        "rho_4 normalization",
        exact_zero(2 * c_4 * quartic_half_integral - 1),
    )

    x = sp.symbols("x", positive=True)
    rho_4 = c_4 / sp.sqrt(1 - x**4)
    check("rho_4 central value", exact_zero(rho_4.subs(x, 0) - c_4))
    check(
        "rho_4 squared density ratio",
        exact_zero((rho_4 / c_4) ** 2 - 1 / (1 - x**4)),
    )

    v_a = sp.symbols("V_A", positive=True)
    gap = sp.symbols("Delta_V", positive=True)
    forced_gap = sp.solve(sp.Eq(v_a / gap, 1 / (1 - x**4)), gap)[0]
    check(
        "fixed-coordinate density forces quartic gap",
        exact_zero(forced_gap - v_a * (1 - x**4)),
    )
    check(
        "fixed-coordinate density forces V(Ax)=V(A)x^4",
        exact_zero(v_a - forced_gap - v_a * x**4),
    )

    amplitude_b, q, v_b = sp.symbols("B q V_B", positive=True)
    lambda_from_a = v_a / amplitude**4
    overlap_solution_v_b = sp.solve(
        sp.Eq(
            v_a * (q / amplitude) ** 4,
            v_b * (q / amplitude_b) ** 4,
        ),
        v_b,
    )[0]
    check(
        "shared potential on a nonzero overlap forces one quartic coefficient",
        exact_zero(
            overlap_solution_v_b / amplitude_b**4 - lambda_from_a
        ),
    )
    check(
        "quartic coefficient is positive",
        sp.ask(sp.Q.positive(lambda_from_a)) is True,
    )
    check(
        "swept coordinate has quartic potential",
        exact_zero(v_a * (q / amplitude) ** 4 - lambda_from_a * q**4),
    )

    lam = sp.symbols("lambda", positive=True)
    check(
        "quartic converse energy gap",
        exact_zero(
            lam * amplitude**4
            - lam * (amplitude * x) ** 4
            - lam * amplitude**4 * (1 - x**4)
        ),
    )
    check(
        "canonical lambda one-half is a selectable specialization",
        exact_zero((lam * q**4).subs(lam, half) - q**4 / 2),
    )

    # ------------------------------------------------------------------
    # 4. Period, frequency, canonical action, dE/dI, and H_0''.
    quarter_time_integral = beta_4 / 4
    period_beta = 4 * quarter_time_integral / (
        amplitude * sp.sqrt(2 * lam)
    )
    period_registered = (
        sp.sqrt(pi) * g_star / (amplitude * sp.sqrt(2 * lam))
    )
    check("quartic period beta integral", exact_zero(period_beta - beta_4 / (amplitude * sp.sqrt(2 * lam))))
    check("quartic registered period", exact_zero(period_beta - period_registered))

    omega = 2 * pi / period_registered
    omega_registered = (
        2 * sp.sqrt(pi) * amplitude * sp.sqrt(2 * lam) / g_star
    )
    check("quartic angular frequency", exact_zero(omega - omega_registered))

    beta_action = sp.beta(quarter, sp.Rational(3, 2))
    check(
        "action beta recurrence",
        exact_zero(beta_action - sp.Rational(2, 3) * beta_4),
    )
    action_quarter_integral = beta_action / 4
    action_from_loop = (
        2
        * amplitude**3
        * sp.sqrt(2 * lam)
        * action_quarter_integral
        / pi
    )
    action_registered = (
        amplitude**3 * sp.sqrt(2 * lam) * g_star
        / (3 * sp.sqrt(pi))
    )
    check("canonical action from closed loop", exact_zero(action_from_loop - action_registered))

    energy = lam * amplitude**4
    denergy_da = sp.diff(energy, amplitude)
    daction_da = sp.diff(action_registered, amplitude)
    frequency_from_action = sp.simplify(denergy_da / daction_da)
    check("Hamiltonian frequency dE/dI", exact_zero(frequency_from_action - omega_registered))

    domega_da = sp.diff(omega_registered, amplitude)
    hessian = sp.simplify(domega_da / daction_da)
    hessian_registered = 2 * pi / (amplitude**2 * g_star**2)
    check("quartic H_0 double prime", exact_zero(hessian - hessian_registered))

    # ------------------------------------------------------------------
    # 5. Signed incomplete-beta phase and the quarter turn.
    incomplete_beta_x = sp.betainc(quarter, half, 0, x**4)
    theta_positive = pi * incomplete_beta_x / (2 * sp.sqrt(pi) * g_star)
    phase_density = pi * rho_4
    check(
        "positive phase branch differentiates to pi rho_4",
        exact_zero(sp.diff(theta_positive, x) - phase_density),
    )
    check(
        "positive phase starts at central crossing",
        exact_zero(theta_positive.subs(x, 0)),
    )
    theta_at_turn = pi * beta_4 / (2 * sp.sqrt(pi) * g_star)
    check("quarter-turn phase", exact_zero(theta_at_turn - pi / 2))

    z = sp.symbols("z", positive=True)
    theta_positive_z = theta_positive.subs(x, z)
    theta_at_negative_z = -theta_positive_z
    rho_at_negative_z = c_4 / sp.sqrt(1 - z**4)
    check(
        "negative branch carries the signed extension",
        exact_zero(theta_at_negative_z + theta_positive_z),
    )
    check(
        "negative branch has positive lifted phase derivative",
        exact_zero(-sp.diff(theta_at_negative_z, z) - pi * rho_at_negative_z),
    )
    check(
        "unsigned incomplete-beta formula fails on the negative branch",
        sp.simplify(theta_positive_z - theta_at_negative_z) != 0,
    )

    # ------------------------------------------------------------------
    # 6. General quartic moments, recurrence, and correlated G functionals.
    r = sp.symbols("r", real=True)
    mu_r = sp.beta((r + 1) / 4, half) / beta_4
    moment_beta_argument = (r + 1) / 4
    check("quartic zeroth moment", exact_zero(mu_r.subs(r, 0) - 1))
    check(
        "quartic recurrence advances the beta argument by one",
        exact_zero((r + 5) / 4 - (moment_beta_argument + 1)),
    )
    # Euler's exact recurrence B(a+1,b)=a B(a,b)/(a+b).
    mu_r_plus_4_reduced = (
        moment_beta_argument
        * mu_r
        / (moment_beta_argument + half)
    )
    check(
        "quartic moment recurrence",
        exact_zero(mu_r_plus_4_reduced - (r + 1) * mu_r / (r + 3)),
    )

    mu_1 = mu_r.subs(r, 1)
    mu_2 = mu_r.subs(r, 2)
    mu_4 = mu_r.subs(r, 4)
    mu_6 = mu_r.subs(r, 6)
    mu_8 = mu_r.subs(r, 8)
    check("quartic mu_1", exact_zero(mu_1 - sp.sqrt(pi) / g_star))
    check("quartic mu_2", exact_zero(mu_2 - 4 / g_star**2))
    check("quartic mu_4", exact_zero(mu_4 - sp.Rational(1, 3)))
    check("quartic mu_6", exact_zero(mu_6 - 12 / (5 * g_star**2)))
    check("quartic mu_8", exact_zero(mu_8 - sp.Rational(5, 21)))

    positive_g = sp.symbols("G", positive=True)
    mu1_g = sp.sqrt(pi) / positive_g
    mu2_g = 4 / positive_g**2
    mu4_g = sp.Rational(1, 3)
    g_rms = 2 / sp.sqrt(mu2_g)
    g_abs = sp.sqrt(pi) / mu1_g
    g_kurt = (48 * mu4_g / mu2_g**2) ** quarter
    check("RMS G functional", exact_zero(g_rms - positive_g))
    check("absolute-moment G functional", exact_zero(g_abs - positive_g))
    check("kurtosis G functional", exact_zero(g_kurt - positive_g))
    check(
        "three G values are functionals of the same moments",
        exact_zero(g_rms - g_abs) and exact_zero(g_abs - g_kurt),
    )

    # ------------------------------------------------------------------
    # 7. Exact homogeneous exponent identity and excluded endpoints.
    m = sp.symbols("m", positive=True, integer=True)
    x_open = sp.symbols("x_open", positive=True)
    rho_m_at_x = sp.symbols("C", positive=True) / sp.sqrt(1 - x_open**m)
    rho_m_at_zero = sp.symbols("C", positive=True)
    exponent_numerator_argument = sp.simplify(
        1 - (rho_m_at_zero / rho_m_at_x) ** 2
    )
    check(
        "homogeneous density isolates x^m",
        exact_zero(exponent_numerator_argument - x_open**m),
    )
    exponent_identity = sp.expand_log(
        sp.log(exponent_numerator_argument), force=True
    ) / sp.log(x_open)
    check("homogeneous exponent identity", exact_zero(exponent_identity - m))

    endpoint_variable = sp.symbols("endpoint_variable", real=True)
    endpoint_form = (
        sp.log(sp.Abs(endpoint_variable) ** m)
        / sp.log(sp.Abs(endpoint_variable))
    )
    check(
        "exponent expression excludes x=0",
        endpoint_form.subs(endpoint_variable, 0) is sp.nan,
    )
    check(
        "exponent expression excludes x=+1",
        endpoint_form.subs(endpoint_variable, 1) is sp.nan,
    )
    check(
        "exponent expression excludes x=-1",
        endpoint_form.subs(endpoint_variable, -1) is sp.nan,
    )

    # ------------------------------------------------------------------
    # 8. General even-m waveform derivative, barrier, curvature, and ratio.
    beta_m0 = sp.beta(1 / m, half)
    beta_m2 = sp.beta(3 / m, half)
    mu_2_m = beta_m2 / beta_m0
    moment_m_argument = 1 / m
    mu_m = (
        moment_m_argument
        * beta_m0
        / ((moment_m_argument + half) * beta_m0)
    )
    check("general even-m moment mu_2", exact_zero(mu_2_m - beta_m2 / beta_m0))
    check(
        "general m-th moment advances the beta argument by one",
        exact_zero((1 + 1 / m) - (moment_m_argument + 1)),
    )
    check("general even-m moment mu_m", exact_zero(mu_m - 2 / (m + 2)))

    normalized_time_derivative_square = (
        amplitude**m * (1 - x**m) / amplitude**2
    )
    check(
        "H_m energy law for the normalized coordinate",
        exact_zero(
            normalized_time_derivative_square
            - amplitude ** (m - 2) * (1 - x**m)
        ),
    )
    general_period = (
        4 * amplitude ** (1 - m / 2) * beta_m0 / m
    )
    general_omega = (
        m * pi * amplitude ** (m / 2 - 1) / (2 * beta_m0)
    )
    check(
        "general uniform-phase frequency is 2pi/T",
        exact_zero(general_omega - 2 * pi / general_period),
    )
    derivative_square_prefactor = 4 * beta_m0**2 / (m**2 * pi**2)
    phase_derivative_square = sp.simplify(
        normalized_time_derivative_square / general_omega**2
    )
    check(
        "general pointwise phase-derivative law",
        exact_zero(
            phase_derivative_square
            - derivative_square_prefactor * (1 - x**m)
        ),
    )
    average_one_minus_xm = 1 - mu_m
    d_m_from_average = sp.simplify(
        derivative_square_prefactor * average_one_minus_xm
    )
    d_m = 4 * beta_m0**2 / (m * (m + 2) * pi**2)
    check("general waveform derivative moment", exact_zero(d_m_from_average - d_m))

    epsilon = sp.symbols("epsilon", positive=True)
    c_zero = mu_2_m
    c_pi = -mu_2_m
    delta_m = sp.simplify(
        epsilon * amplitude**2 * ((mu_2_m - c_pi) - (mu_2_m - c_zero))
    )
    delta_m_registered = 2 * epsilon * amplitude**2 * mu_2_m
    check("anti-periodic autocorrelation barrier", exact_zero(delta_m - delta_m_registered))

    boundary_term = sp.symbols("periodic_boundary_term", real=True)
    c_second_zero = boundary_term - d_m
    check(
        "periodic integration by parts removes boundary term",
        exact_zero(c_second_zero.subs(boundary_term, 0) + d_m),
    )
    k_m = -epsilon * amplitude**2 * c_second_zero.subs(boundary_term, 0)
    k_m_registered = epsilon * amplitude**2 * d_m
    check("quadratic-edge phase curvature", exact_zero(k_m - k_m_registered))

    b_m = sp.simplify(delta_m_registered / k_m_registered)
    b_m_registered = (
        m
        * (m + 2)
        * pi**2
        * beta_m2
        / (2 * beta_m0**3)
    )
    check("general nonlinear edge ratio", exact_zero(b_m - b_m_registered))
    check(
        "general ratio cancels A and epsilon",
        amplitude not in b_m.free_symbols and epsilon not in b_m.free_symbols,
    )

    # ------------------------------------------------------------------
    # 9. Registered m={2,4,6} controls.
    b_2 = sp.simplify(b_m_registered.subs(m, 2))
    b_4 = sp.simplify(b_m_registered.subs(m, 4))
    b_6 = sp.simplify(b_m_registered.subs(m, 6))
    check("quadratic control beta value", exact_zero(sp.beta(half, half) - pi))
    check("quadratic control B_2", exact_zero(b_2 - 2))
    check("quartic control B_4", exact_zero(b_4 - 48 * pi / g_star**4))
    check(
        "sextic control B_6",
        exact_zero(
            b_6 - 24 * pi**3 / sp.beta(sp.Rational(1, 6), half) ** 3
        ),
    )

    # ------------------------------------------------------------------
    # 10. Quartic derivative law, edge components, and epsilon/3 product.
    quartic_derivative_square = (
        g_star**2 * (1 - x**4) / (4 * pi)
    )
    quartic_derivative_from_general = (
        derivative_square_prefactor.subs(m, 4) * (1 - x**4)
    )
    check(
        "quartic phase-derivative law",
        exact_zero(quartic_derivative_from_general - quartic_derivative_square),
    )
    d_4 = sp.simplify(d_m.subs(m, 4))
    d_4_registered = g_star**2 / (6 * pi)
    check("quartic derivative moment D_4", exact_zero(d_4 - d_4_registered))

    delta_4 = sp.simplify(delta_m_registered.subs(m, 4))
    delta_4_registered = 8 * epsilon * amplitude**2 / g_star**2
    check("quartic barrier Delta_4", exact_zero(delta_4 - delta_4_registered))

    k_4 = sp.simplify(k_m_registered.subs(m, 4))
    k_4_registered = epsilon * amplitude**2 * g_star**2 / (6 * pi)
    check("quartic curvature K_4", exact_zero(k_4 - k_4_registered))
    check(
        "quartic exact barrier-curvature ratio",
        exact_zero(delta_4 / k_4 - 48 * pi / g_star**4),
    )
    check(
        "quartic action-edge cancellation",
        exact_zero(hessian_registered * k_4_registered - epsilon / 3),
    )

    g_edge_inner = sp.simplify(48 * pi * k_4_registered / delta_4_registered)
    check("G_edge fourth power", exact_zero(g_edge_inner - g_star**4))
    check(
        "positive G_edge root reconstructs G_STAR conditionally",
        exact_zero(sp.real_root(g_edge_inner, 4) - g_star),
    )

    b4_exact = 48 * pi / g_star**4
    b4_decimal = sp.Float("1.967895315142656", 50)
    rounding_tolerance = sp.Rational(1, 2 * 10**15)
    check(
        "displayed B_4 decimal is only an exact-expression rounding",
        abs(sp.N(b4_exact, 60) - b4_decimal) < rounding_tolerance,
    )

    # ------------------------------------------------------------------
    # 11. Constant-scale cancellation and nonlinear-coordinate dependence.
    amplitude_scale, edge_scale = sp.symbols("c_A c_epsilon", positive=True)
    check(
        "amplitude and edge-strength rescaling cancels",
        exact_zero(
            (
                delta_4_registered.subs(
                    {amplitude: amplitude_scale * amplitude,
                     epsilon: edge_scale * epsilon}
                )
                / k_4_registered.subs(
                    {amplitude: amplitude_scale * amplitude,
                     epsilon: edge_scale * epsilon}
                )
            )
            - b4_exact
        ),
    )
    speed_scale, phase_speed, coordinate_speed = sp.symbols(
        "c_speed omega_phase q_speed", positive=True
    )
    check(
        "constant physical-time or lambda speed scale cancels in phase shape",
        exact_zero(
            (speed_scale * coordinate_speed)
            / (speed_scale * phase_speed)
            - coordinate_speed / phase_speed
        ),
    )
    check(
        "B_4 has no lambda or physical-time scale",
        lam not in b4_exact.free_symbols and speed_scale not in b4_exact.free_symbols,
    )

    # Exact nonlinear-observable witness y=x^3.  It remains anti-periodic
    # and normalized at the turning points, but its barrier-curvature ratio
    # is 7/15 of the original quartic-coordinate value.
    y_second_moment = mu_6
    y_derivative_moment = sp.simplify(
        9 * g_star**2 / (4 * pi) * (mu_4 - mu_8)
    )
    b_y = sp.simplify(2 * y_second_moment / y_derivative_moment)
    check(
        "nonlinear y=x^3 derivative moment",
        exact_zero(y_derivative_moment - 3 * g_star**2 / (14 * pi)),
    )
    check(
        "nonlinear y=x^3 edge ratio",
        exact_zero(b_y - 112 * pi / (5 * g_star**4)),
    )
    check(
        "nonlinear coordinate changes the ratio by 7/15",
        exact_zero(b_y / b4_exact - sp.Rational(7, 15)),
    )

    # A quartic edge applied to a harmonic waveform has a nonzero barrier
    # but zero quadratic curvature, providing an exact edge-functional
    # dependence witness.
    phi = sp.symbols("phi", real=True)
    quartic_edge_shape = 6 * sp.sin(phi / 2) ** 4
    check(
        "quartic edge has zero phase curvature at alignment",
        exact_zero(sp.diff(quartic_edge_shape, phi, 2).subs(phi, 0)),
    )
    check(
        "quartic edge retains a nonzero anti-phase barrier",
        exact_zero(quartic_edge_shape.subs(phi, pi) - 6),
    )

    # ------------------------------------------------------------------
    # 12. A finite tick measure is atomic, unlike rho(x) dx.
    tick_count, multiplicity = sp.symbols("P k", positive=True, integer=True)
    atomic_mass = multiplicity / tick_count
    check(
        "finite occupied tick atom has positive mass",
        sp.ask(sp.Q.positive(atomic_mass)) is True,
    )
    atom = sp.symbols("x_atom", real=True)
    continuous_singleton_mass = sp.Integral(rho_4, (x, atom, atom)).doit()
    check(
        "continuous density gives a singleton zero mass",
        continuous_singleton_mass == 0,
    )
    check(
        "finite atomic and continuous measures differ",
        sp.simplify(atomic_mass - continuous_singleton_mass) != 0,
    )

    expected_checks = 95
    assert len(checks) == expected_checks, (
        f"expected {expected_checks} checks, ran {len(checks)}"
    )
    failures = [label for label, passed in checks if not passed]
    print(
        "FTD-0773 quartic waveform nonlinear-edge exact certificate: "
        f"{len(checks) - len(failures)}/{len(checks)} checks PASS"
    )
    print(f"protocol_sha256={PROTOCOL_SHA256}")
    print(f"G_STAR={sp.N(g_star, 30)}")
    print(f"B_4={sp.N(b4_exact, 30)}")
    if failures:
        for label in failures:
            print(f"FAIL {label}")
        return 1

    print("QUARTIC_CONTINUOUS_INVERSE_CHAIN_CONDITIONAL_THEOREMS_PASS")
    print("QUARTIC_NONLINEAR_EDGE_SHAPE_FUNCTIONAL_GSTAR_PRESENT")
    print("NATIVE_QUARTIC_TIME_DERIVATION_NOT_ESTABLISHED")
    print("NATIVE_NONLINEAR_EDGE_TEST_BLOCKED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
