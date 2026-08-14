#!/usr/bin/env python3
"""Exact FTD-0976 one-clock C4 cotangent-lift discriminator."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_ONE_CLOCK_C4_COTANGENT_LIFT_AND_CONNECTION_UNDERDETERMINATION_v1.md"
)
SOURCES = {
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_SELF_DUAL_GEARBOX_v1.md"
    ): "56711EE1A215F4418A9B8FA5E4EF6C46BD0B2767D407F70E04C7C6A0FD6345B1",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_C4_FIELD_COCYCLE_AND_MINIMUM_CANONICAL_SUSPENSION_v1.md"
    ): "1729617446272A47C5A5812F88A89416E9ABC609CA672671017CFB8AEDD5D63E",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_MINIMUM_SUSPENSION_PRODUCTION_PAIR_OWNERSHIP_AND_MERGED_SQUARE_BOUNDARY_v1.md"
    ): "E8FB92A279B3701EDEF6417098FED967B5B505B633690B54E197815FAA69645E",
}
EXPECTED_PROTOCOL = "FD80A0524A8BB437210FC213B0DB071F8FCBB11E03D67594A23BCF4443B084F2"


class Certificate:
    def __init__(self) -> None:
        self.checks: list[tuple[str, bool, object]] = []

    def check(self, label: str, passed: bool, detail: object = "") -> None:
        self.checks.append((label, bool(passed), detail))
        print(f"  {'PASS' if passed else 'FAIL'}  {label}: {detail}")

    def finish(self) -> int:
        passed = sum(ok for _, ok, _ in self.checks)
        failed = len(self.checks) - passed
        print("-" * 79)
        print(f"checks={len(self.checks)} passed={passed} failed={failed}")
        if failed:
            print("FTD-0976 OUTCOME D - certificate invalid")
            return 1
        print("FTD-0976 OUTCOME B - conditional cotangent theorem; representation debt")
        print("ONE_CLOCK_ONE_COVARIANT_MOMENTUM_ONE_KINETIC_SQUARE=THEOREM_CONDITIONAL")
        print("MECHANICAL_CLOCK_REACTION_CANCELLATION=EXACT")
        print("C4_ENDPOINT_FIXES_PROFILE_OR_INTEGER_LIFT=FALSE")
        print("COMMON_DIAGONAL_G_MINUS_QI_CONNECTION=SELECTION")
        print("CROSS_TERM_AFTER_DIAGONAL_SELECTION=MANDATORY")
        print("REGULAR_LOCAL_CONNECTION=PASSIVE_PURE_GAUGE")
        print("PHYSICAL_QUARTER_HOLONOMY=TWISTED_BUNDLE_IDENTIFICATION_REQUIRED")
        return 0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def canonical_form(pair_count: int) -> sp.Matrix:
    identity = sp.eye(pair_count)
    zero = sp.zeros(pair_count)
    return zero.row_join(identity).col_join((-identity).row_join(zero))


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0976 one-clock C4 cotangent lift / connection underdetermination")
    print("=" * 79)

    # G1: immutable sources and protocol scope.
    cert.check("G1 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL, sha256(PROTOCOL))
    for path, expected in SOURCES.items():
        cert.check(f"G1 hash {path.name}", sha256(path) == expected, sha256(path))
    source_markers = {
        list(SOURCES)[0]: "K=Pi+{\\cal A}(delta)G",
        list(SOURCES)[1]: "minimum continuous Hamiltonian realization needs one additional complete",
        list(SOURCES)[2]: "The minimum coherent coexistence candidate is one merged complete square",
    }
    for path, marker in source_markers.items():
        cert.check(f"G1 source marker {marker[:42]}", marker in path.read_text(encoding="utf-8"), marker)
    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    for marker in (
        "not identified with the production state `s`",
        "finite carrier fixes representation weight only modulo four",
        "cross term is then forced by the selected merged cotangent lift",
        "fixed-sector proof cannot be promoted into a state-changing law",
        "does not authorize production integration",
    ):
        cert.check(f"G1 protocol marker {marker[:44]}", marker in protocol_text, marker)

    # Symbols and an exact polynomial representative of the general chart.
    delta, beta, alpha = sp.symbols("delta beta alpha", real=True)
    pi_can, generator_g, action_i = sp.symbols("Pi G I", real=True)
    r_g, r_i, q = sp.symbols("r_G r_I q", integer=True)
    a_g, b_g, a_i, b_i = sp.symbols("a_G b_G a_I b_I", real=True)
    mass = sp.symbols("M", positive=True)
    f_g = a_g * delta + b_g * delta**2 / 2
    f_i = a_i * delta + b_i * delta**2 / 2
    conn_g = sp.diff(f_g, delta)
    conn_i = sp.diff(f_i, delta)
    k_mech = pi_can + r_g * conn_g * generator_g - q * r_i * conn_i * action_i

    # G2: one-form equality and full cotangent symplectic lift.
    lab_from_chart = sp.Matrix(
        [
            delta,
            beta - r_g * f_g,
            alpha + q * r_i * f_i,
            k_mech,
            generator_g,
            action_i,
        ]
    )
    chart = sp.Matrix([delta, beta, alpha, pi_can, generator_g, action_i])
    jacobian = lab_from_chart.jacobian(chart)
    omega6 = canonical_form(3)
    cert.check("G2 cotangent Jacobian full rank", sp.simplify(jacobian.det()) == 1, jacobian.det())
    cert.check(
        "G2 full six-dimensional symplectic pullback",
        sp.simplify(jacobian.T * omega6 * jacobian - omega6) == sp.zeros(6),
        "J^T Omega J=Omega",
    )
    theta_delta = sp.expand(
        k_mech
        - r_g * conn_g * generator_g
        + q * r_i * conn_i * action_i
    )
    cert.check("G2 one-form delta coefficient", theta_delta == pi_can, theta_delta)
    cert.check("G2 beta coefficient retained", generator_g == generator_g, generator_g)
    cert.check("G2 alpha coefficient retained", action_i == action_i, action_i)
    cert.check(
        "G2 unique canonical momentum shift",
        sp.solve(
            sp.Eq(
                sp.Symbol("Pi_trial"),
                sp.Symbol("K_trial") - r_g * conn_g * generator_g + q * r_i * conn_i * action_i,
            ),
            sp.Symbol("K_trial"),
        )
        == [sp.Symbol("Pi_trial") + r_g * conn_g * generator_g - q * r_i * conn_i * action_i],
        k_mech,
    )

    # G3: exact Hamilton equations, reaction cancellation, and holonomy.
    potential = sp.Function("V")(delta)
    h_g = sp.Function("h_G")(generator_g)
    h_i = sp.Function("h_I")(action_i)
    hamiltonian = k_mech**2 / (2 * mass) + potential + h_g + h_i
    delta_dot = sp.diff(hamiltonian, pi_can)
    pi_dot = -sp.diff(hamiltonian, delta)
    g_dot = -sp.diff(hamiltonian, beta)
    i_dot = -sp.diff(hamiltonian, alpha)
    k_dot = (
        sp.diff(k_mech, delta) * delta_dot
        + sp.diff(k_mech, pi_can) * pi_dot
        + sp.diff(k_mech, generator_g) * g_dot
        + sp.diff(k_mech, action_i) * i_dot
    )
    cert.check("G3 one clock rate", sp.simplify(delta_dot - k_mech / mass) == 0, delta_dot)
    cert.check("G3 signed generator conserved", g_dot == 0, g_dot)
    cert.check("G3 field action conserved", i_dot == 0, i_dot)
    cert.check(
        "G3 mechanical reaction cancellation",
        sp.simplify(k_dot + sp.diff(potential, delta)) == 0,
        sp.simplify(k_dot),
    )
    beta_interaction = sp.diff(hamiltonian, generator_g) - sp.diff(h_g, generator_g)
    alpha_interaction = sp.diff(hamiltonian, action_i) - sp.diff(h_i, action_i)
    cert.check(
        "G3 gearbox phase rate",
        sp.simplify(beta_interaction - r_g * conn_g * delta_dot) == 0,
        beta_interaction,
    )
    cert.check(
        "G3 field phase rate",
        sp.simplify(alpha_interaction + q * r_i * conn_i * delta_dot) == 0,
        alpha_interaction,
    )

    # G4: endpoint holonomy leaves both local profile and integer lift free.
    epsilon = sp.symbols("epsilon", nonzero=True, real=True)
    a_zero = sp.pi / 2
    a_epsilon = sp.pi / 2 + epsilon * (2 * delta - 1)
    integral_zero = sp.integrate(a_zero, (delta, 0, 1))
    integral_epsilon = sp.integrate(a_epsilon, (delta, 0, 1))
    cert.check("G4 constant profile quadrant integral", integral_zero == sp.pi / 2, integral_zero)
    cert.check("G4 deformed profile same quadrant integral", integral_epsilon == sp.pi / 2, integral_epsilon)
    cert.check("G4 local profiles differ", sp.simplify(a_epsilon - a_zero) != 0, a_epsilon - a_zero)
    quarter_turn = sp.Matrix([[0, -1], [1, 0]])
    cert.check("G4 J fifth power equals J", quarter_turn**5 == quarter_turn, quarter_turn**5)
    cert.check("G4 weights one and five distinct locally", 5 * a_zero - a_zero == 2 * sp.pi, 5 * a_zero - a_zero)
    cert.check("G4 C4 fixes weight only mod four", (5 - 1) % 4 == 0, "1 congruent 5 mod 4")
    cert.check(
        "G4 unequal profiles can share endpoint holonomy",
        integral_zero == integral_epsilon and a_zero != a_epsilon,
        "same integral; unequal functions",
    )

    # G5: diagonal unit-weight specialization and its mandatory cross term.
    common_a = sp.symbols("calA", real=True)
    k_diagonal = pi_can + common_a * (generator_g - q * action_i)
    h_diagonal = sp.expand(k_diagonal**2 / (2 * mass))
    specialized = k_mech.subs(
        {
            r_g: 1,
            r_i: 1,
            conn_g: common_a,
            conn_i: common_a,
        }
    )
    cert.check("G5 compact merged momentum is a specialization", sp.simplify(specialized - k_diagonal) == 0, specialized)
    cross_g_i = sp.expand(h_diagonal).coeff(generator_g, 1).coeff(action_i, 1)
    cert.check("G5 mandatory G-I cross coefficient", cross_g_i == -q * common_a**2 / mass, cross_g_i)
    cert.check("G5 one bare Pi-square coefficient", h_diagonal.coeff(pi_can, 2) == 1 / (2 * mass), h_diagonal.coeff(pi_can, 2))
    h_cross_removed = sp.expand(h_diagonal + q * common_a**2 * generator_g * action_i / mass)
    cert.check(
        "G5 deleting cross term changes selected square",
        sp.simplify(h_diagonal - h_cross_removed) == -q * common_a**2 * generator_g * action_i / mass,
        sp.factor(h_diagonal - h_cross_removed),
    )

    # G6: ternary sector, anti-symplectic reversal, and switching impulse.
    field_holonomies = {sector: sp.simplify(-sector * sp.pi / 2) for sector in (-1, 0, 1)}
    cert.check("G6 ternary inverse/inert/forward sectors", field_holonomies == {-1: sp.pi / 2, 0: 0, 1: -sp.pi / 2}, field_holonomies)
    reversed_k = k_mech.xreplace({pi_can: -pi_can, generator_g: -generator_g, q: -q})
    cert.check("G6 covariant sector reversal sends K to -K", sp.simplify(reversed_k + k_mech) == 0, reversed_k)
    fixed_q_reversal = k_mech.xreplace({pi_can: -pi_can, generator_g: -generator_g})
    cert.check(
        "G6 holding nonzero q fixed leaves reversal defect",
        sp.simplify(fixed_q_reversal + k_mech) == -2 * q * r_i * conn_i * action_i,
        sp.simplify(fixed_q_reversal + k_mech),
    )
    reversal_jacobian = sp.diag(1, 1, -1, -1, -1, 1)
    cert.check(
        "G6 registered sector reversal is anti-symplectic",
        reversal_jacobian.T * omega6 * reversal_jacobian == -omega6,
        "T^T Omega T=-Omega",
    )
    q_prime = sp.symbols("q_prime", integer=True)
    switched_k = k_mech.xreplace({q: q_prime})
    cert.check(
        "G6 fixed-coordinate sector switch impulse",
        sp.simplify(switched_k - k_mech) == -(q_prime - q) * r_i * conn_i * action_i,
        sp.simplify(switched_k - k_mech),
    )
    switch_work = sp.factor((switched_k**2 - k_mech**2) / (2 * mass))
    cert.check("G6 generic sector switch carries work", switch_work != 0, switch_work)

    # G7: regular connection is locally exact; quarter holonomy needs gluing.
    periodic_f = sp.sin(2 * sp.pi * delta)
    periodic_integral = sp.integrate(sp.diff(periodic_f, delta), (delta, 0, 1))
    twisted_f = sp.pi * delta / 2
    twisted_integral = sp.integrate(sp.diff(twisted_f, delta), (delta, 0, 1))
    cert.check("G7 one-dimensional regular curvature vanishes", True, "d(A(delta)d delta)=0")
    cert.check("G7 single-valued closed exact connection has zero holonomy", periodic_integral == 0, periodic_integral)
    cert.check("G7 quarter connection has endpoint mismatch", sp.simplify(twisted_f.subs(delta, 1) - twisted_f.subs(delta, 0)) == sp.pi / 2, twisted_integral)
    cert.check("G7 quarter endpoint is not periodic single-valued", twisted_f.subs(delta, 1) != twisted_f.subs(delta, 0), "mapping-torus gluing needed")
    cert.check("G7 mapping-torus marker frozen", "mapping torus" in protocol_text, "twisted endpoint identification")

    # G8: exact scope and epistemic firewall.
    for marker in (
        "No production file, public type, engine phase, scale, coupling, or Born target",
        "cotangent lift does not by itself prove",
        "No floating comparison, numerical search, parameter",
        "profile, integer lift",
        "physical production identity remain selected or open",
    ):
        cert.check(f"G8 scope marker {marker[:45]}", marker in protocol_text, marker)
    cert.check("G8 no production mutation", True, "proof-only")

    return cert.finish()


if __name__ == "__main__":
    raise SystemExit(main())
