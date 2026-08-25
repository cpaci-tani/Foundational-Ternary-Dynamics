#!/usr/bin/env python3
"""Exact reciprocal packet/clock/recoil absorption-generator certificate.

The frozen type-2 generating function joins one discrete packet-ownership
branch to a clock action/phase pair and a material position/momentum pair.  It
delivers declared packet translation charge to matter, books recoil energy,
transfers the remainder into clock action, preserves the canonical two-form
and total energy, and has an exact history-complete inverse.

The generator, packet momentum, absorption trigger, clock frequency, inertia,
and field/clock scale compliance remain selected.  No alpha value, master
root, empirical target, or numerical search enters.
"""

from __future__ import annotations

import hashlib
from fractions import Fraction
from itertools import product
from pathlib import Path

from sympy import Matrix, Rational, Symbol, diff, pi, simplify


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "common_action_mechanics_reciprocity/"
    "PREREG_RECIPROCAL_PACKET_CLOCK_RECOIL_ABSORPTION_GENERATOR_v1.md"
)
LOCKED_HASHES = {
    PREREG: "C78C5C887367852AEEAD19F5DDEF7D71F3E85BEBCA240800D427399A842C2156",
    ROOT / "scripts/proofs/proof_c4_field_packet_reserve_current_and_atomic_clock_debit.py":
        "F58075539C396815F3942A70EE58A17AC04F139B4E205514666858D284CEADAB",
    ROOT / "scripts/proofs/proof_clocked_remainder_recoil_noether_boundary.py":
        "B73C82F9853123732A539E86760D14C4B8DE9DADBFA6CA633649D5A998434C44",
    ROOT / "scripts/proofs/proof_cotangent_charged_pole_reciprocal_alpha_measurement_protocol.py":
        "F903FF32FE4E38BB4EA5BFD6907A79DFA00C5F524E4B4624309C23D03A65EF87",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def kinetic(momentum: Matrix, mass):
    return (momentum.T * momentum)[0] / (2 * mass)


def canonical_matrix(pair_count: int) -> Matrix:
    zero = Matrix.zeros(pair_count, pair_count)
    identity = Matrix.eye(pair_count)
    return zero.row_join(identity).col_join((-identity).row_join(zero))


def symbolic_generator_checks() -> int:
    checks = 0
    theta, x, i_prime, p_prime = (
        Symbol("theta", real=True),
        Symbol("X", real=True),
        Symbol("I_prime", real=True),
        Symbol("P_prime", real=True),
    )
    packet_momentum = Symbol("p_field", real=True)
    mass, omega, gamma = (
        Symbol("m", positive=True),
        Symbol("omega", positive=True),
        Symbol("Gamma", positive=True),
    )
    packet_count = Symbol("d", positive=True, integer=True)

    def k(value):
        return value**2 / (2 * mass)

    work = (
        packet_count * gamma
        + k(p_prime - packet_momentum)
        - k(p_prime)
    ) / omega
    generator = (
        theta * i_prime
        + x * (p_prime - packet_momentum)
        - theta * work
    )

    input_action = simplify(diff(generator, theta))
    input_momentum = simplify(diff(generator, x))
    output_theta = simplify(diff(generator, i_prime))
    output_position = simplify(diff(generator, p_prime))
    assert simplify(input_action - (i_prime - work)) == 0
    assert simplify(input_momentum - (p_prime - packet_momentum)) == 0
    assert simplify(output_theta - theta) == 0
    assert simplify(
        output_position
        - (
            x
            - theta
            / omega
            * (
                (p_prime - packet_momentum) / mass
                - p_prime / mass
            )
        )
    ) == 0
    checks += 4

    # Express the generated map in old variables and verify its one-dimensional
    # canonical Jacobian exactly.
    old_action, old_momentum = Symbol("I", real=True), Symbol("P", real=True)
    new_momentum = old_momentum + packet_momentum
    new_action = simplify(
        old_action
        + (
            packet_count * gamma
            + k(old_momentum)
            - k(new_momentum)
        )
        / omega
    )
    new_position = simplify(x + theta * packet_momentum / (mass * omega))
    old_vector = Matrix([theta, x, old_action, old_momentum])
    new_vector = Matrix([theta, new_position, new_action, new_momentum])
    jacobian = new_vector.jacobian(old_vector)
    symplectic = canonical_matrix(2)
    assert simplify(jacobian.T * symplectic * jacobian - symplectic) == Matrix.zeros(4, 4)
    checks += 1

    energy_before = omega * old_action + k(old_momentum) + packet_count * gamma
    energy_after = omega * new_action + k(new_momentum)
    assert simplify(energy_after - energy_before) == 0
    assert simplify(new_momentum - (old_momentum + packet_momentum)) == 0
    assert new_position.subs(theta, 0) == x
    checks += 3
    return checks


def symbolic_three_dimensional_checks() -> int:
    checks = 0
    theta, action = Symbol("theta", real=True), Symbol("I", real=True)
    omega, gamma, mass = (
        Symbol("omega", positive=True),
        Symbol("Gamma", positive=True),
        Symbol("m", positive=True),
    )
    packet_count = Symbol("d", positive=True, integer=True)
    positions = Matrix([Symbol(f"X{axis}", real=True) for axis in range(3)])
    momenta = Matrix([Symbol(f"P{axis}", real=True) for axis in range(3)])
    packet_momentum = Matrix(
        [Symbol(f"p{axis}", real=True) for axis in range(3)]
    )

    new_momenta = momenta + packet_momentum
    kinetic_before = kinetic(momenta, mass)
    kinetic_after = kinetic(new_momenta, mass)
    clock_increment = simplify(
        (packet_count * gamma + kinetic_before - kinetic_after) / omega
    )
    new_action = action + clock_increment
    new_positions = positions + theta * packet_momentum / (mass * omega)

    old_vector = Matrix(
        [theta, *positions, action, *momenta]
    )
    new_vector = Matrix(
        [theta, *new_positions, new_action, *new_momenta]
    )
    jacobian = new_vector.jacobian(old_vector)
    symplectic = canonical_matrix(4)
    assert simplify(jacobian.T * symplectic * jacobian - symplectic) == Matrix.zeros(8, 8)
    checks += 1

    energy_before = omega * action + kinetic_before + packet_count * gamma
    energy_after = omega * new_action + kinetic_after
    assert simplify(energy_after - energy_before) == 0
    checks += 1

    # Total declared translation charge is matter momentum plus the incoming
    # packet charge.  After absorption the packet branch owns zero.
    assert new_momenta == momenta + packet_momentum
    checks += 1

    # Exact inverse, including the off-seam position reaction.
    recovered_momenta = new_momenta - packet_momentum
    recovered_action = simplify(
        new_action
        - (
            packet_count * gamma
            + kinetic(recovered_momenta, mass)
            - kinetic(new_momenta, mass)
        )
        / omega
    )
    recovered_positions = simplify(
        new_positions - theta * packet_momentum / (mass * omega)
    )
    assert recovered_momenta == momenta
    assert simplify(recovered_action - action) == 0
    assert recovered_positions == positions
    checks += 3

    # Quadratic specialization of the recoil partition.
    dot_term = (momenta.T * packet_momentum)[0]
    packet_norm_squared = (packet_momentum.T * packet_momentum)[0]
    expected_clock_energy = simplify(
        packet_count * gamma
        - (2 * dot_term + packet_norm_squared) / (2 * mass)
    )
    assert simplify(omega * clock_increment - expected_clock_energy) == 0
    rest_increment = simplify(clock_increment.subs({entry: 0 for entry in momenta}))
    assert simplify(
        omega * rest_increment
        - (packet_count * gamma - packet_norm_squared / (2 * mass))
    ) == 0
    neutral_increment = simplify(clock_increment.subs({entry: 0 for entry in packet_momentum}))
    assert neutral_increment == packet_count * gamma / omega
    checks += 3

    action_quantum = Symbol("I_star", positive=True)
    curvature = simplify(
        omega
        / (packet_count - packet_norm_squared / (2 * mass * gamma))
    )
    compliance_gamma = simplify(
        (omega * action_quantum + packet_norm_squared / (2 * mass))
        / packet_count
    )
    assert simplify(
        (gamma / action_quantum).subs(gamma, compliance_gamma)
        - curvature.subs(gamma, compliance_gamma)
    ) == 0
    neutral_curvature = curvature.subs(
        {entry: 0 for entry in packet_momentum}
    )
    assert neutral_curvature == omega / packet_count
    c_eff = Rational(1, 6)
    assert simplify(neutral_curvature / (4 * pi * c_eff)) == (
        3 * omega / (2 * pi * packet_count)
    )
    checks += 3
    return checks


def absorb_fixture(state, packet_history, packet_count, gamma, omega, mass, impulse):
    """Exact rational seam branch with retained discrete packet history."""

    theta, action, position, momentum = state
    if theta != 0 or packet_history is None or packet_count <= 0:
        return None
    new_momentum = tuple(p + q for p, q in zip(momentum, impulse))
    k_before = sum(value * value for value in momentum) / (2 * mass)
    k_after = sum(value * value for value in new_momentum) / (2 * mass)
    new_action = action + (packet_count * gamma + k_before - k_after) / omega
    if new_action < 0:
        return None
    new_state = (theta, new_action, position, new_momentum)
    retained = (
        "absorbed-history",
        packet_history,
        packet_count,
        gamma,
        tuple(impulse),
    )
    return new_state, retained


def emit_fixture(state, retained, omega, mass):
    theta, action_after, position, momentum_after = state
    if theta != 0 or retained is None or retained[0] != "absorbed-history":
        return None
    _label, packet_history, packet_count, gamma, impulse = retained
    momentum = tuple(p - q for p, q in zip(momentum_after, impulse))
    k_before = sum(value * value for value in momentum) / (2 * mass)
    k_after = sum(value * value for value in momentum_after) / (2 * mass)
    action = action_after - (packet_count * gamma + k_before - k_after) / omega
    return (theta, action, position, momentum), packet_history


def rational_fixture_checks() -> int:
    checks = 0
    packet_history = (
        "packet-17",
        (0, 2),
        "handed-flag",
        "route-r",
    )
    for action, gamma, omega, mass, packet_count, momentum, impulse in product(
        (Fraction(0), Fraction(1), Fraction(5, 2)),
        (Fraction(1, 3), Fraction(1), Fraction(7, 2)),
        (Fraction(1, 2), Fraction(2), Fraction(5, 1)),
        (Fraction(1), Fraction(3), Fraction(7, 2)),
        (1, 2, 3),
        ((0, 0, 0), (1, -1, 0), (-2, 1, 1)),
        ((0, 0, 0), (1, 0, 0), (-1, 1, 0)),
    ):
        state = (Fraction(0), action, (3, -2, 5), momentum)
        result = absorb_fixture(
            state,
            packet_history,
            packet_count,
            gamma,
            omega,
            mass,
            impulse,
        )
        k_before = sum(value * value for value in momentum) / (2 * mass)
        new_momentum = tuple(p + q for p, q in zip(momentum, impulse))
        k_after = sum(value * value for value in new_momentum) / (2 * mass)
        expected_action = action + (
            packet_count * gamma + k_before - k_after
        ) / omega
        if expected_action < 0:
            assert result is None
            checks += 1
            continue
        assert result is not None
        new_state, retained = result
        assert new_state[1] == expected_action
        assert new_state[2] == state[2]
        assert new_state[3] == new_momentum
        assert retained[1] == packet_history
        energy_before = omega * action + k_before + packet_count * gamma
        energy_after = omega * new_state[1] + k_after
        assert energy_before == energy_after
        restored = emit_fixture(new_state, retained, omega, mass)
        assert restored == (state, packet_history)
        checks += 7

    # Missing packet ownership and off-seam absorption fail before mutation.
    state = (Fraction(0), Fraction(1), (0, 0, 0), (0, 0, 0))
    assert absorb_fixture(state, None, 1, Fraction(1), Fraction(1), Fraction(1), (0, 0, 0)) is None
    off_seam = (Fraction(1, 4), Fraction(1), (0, 0, 0), (0, 0, 0))
    assert absorb_fixture(off_seam, packet_history, 1, Fraction(1), Fraction(1), Fraction(1), (0, 0, 0)) is None
    checks += 2
    return checks


def main() -> None:
    checks = 0
    for path, expected in LOCKED_HASHES.items():
        assert sha256(path) == expected, (path, sha256(path), expected)
        checks += 1

    checks += symbolic_generator_checks()
    checks += symbolic_three_dimensional_checks()
    checks += rational_fixture_checks()

    print("one type-2 generator yields clock-action, recoil, and source-reaction updates")
    print("the complete 3D map is exactly symplectic and history-invertible")
    print("packet energy equals clock gain plus material kinetic-energy change")
    print("declared field translation charge is transferred exactly to matter")
    print("at theta=0 the material position is unchanged during the local kick")
    print("complete local energy, hence scalar T00 ownership, is continuous")
    print("conditional compliance includes recoil and fixes no coupling value")
    print(
        "PASS: reciprocal packet/clock/recoil absorption generator "
        f"({checks} exact checks)"
    )
    print(
        "OUTCOME B: exact selected common-action vertex; trigger, packet momentum, "
        "inertia, gravity stress handoff, and action scale remain open"
    )


if __name__ == "__main__":
    main()
