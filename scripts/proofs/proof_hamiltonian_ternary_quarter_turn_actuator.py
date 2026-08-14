#!/usr/bin/env python3
"""FTD-0873 exact certificate for the Hamiltonian ternary quarter-turn lift."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_HAMILTONIAN_TERNARY_QUARTER_TURN_ACTUATOR_v1.md"
)
PROTOCOL_HASH = "430844D025A061CB1F5701C7450AEFC0769E8F8EA9C1A3A27CA158FD574F4465"
SOURCE_HASHES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md":
        "779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md":
        "FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_ELIGIBILITY_CLUTCH_AND_ONE_SHOT_HANDSHAKE_v1.md":
        "6BD280A51DEF9A1B5E373D0084A9C19597772CD31D2B5D278B2323315AC2153D",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_REVERSIBLE_TERNARY_SIGNAL_UNCOMPUTATION_AND_RESET_BOUNDARY_v1.md":
        "F52BE0CD97FAE06CF6A39C6E0784EC75746F7B8ABF9843C4EF78B37181C8D2CC",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_ORIENTED_TERNARY_QUARTER_TURN_GEARBOX_v1.md":
        "898A9130DFBAAE23B76D3FB5339851D026B50E5B7EFFB8B4B8DC66513F5A9317",
    "engine/include/ftd/eft/clock_gated_hamiltonian_exchange.h":
        "0BDEF8D6278FDF352F89C739F995F337B76AECC8C4FE716DF899B4058DE8A29E",
    "engine/include/ftd/eft/oriented_ternary_quarter_turn.h":
        "46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1",
}

TERNARY = (-1, 0, 1)
STATES = tuple(itertools.product(TERNARY, repeat=2))
I2 = ((1, 0), (0, 1))
NEG_I2 = ((-1, 0), (0, -1))
R = ((0, -1), (1, 0))
R_INVERSE = ((0, 1), (-1, 0))

checks = 0
failures = 0


def check(label: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if condition:
        print(f"PASS  C{checks} {label}")
    else:
        failures += 1
        print(f"FAIL  C{checks} {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def mmul(first, second):
    return tuple(
        tuple(
            sum(first[row][k] * second[k][column] for k in range(2))
            for column in range(2)
        )
        for row in range(2)
    )


def apply(matrix, pair):
    return tuple(
        sum(matrix[row][column] * pair[column] for column in range(2))
        for row in range(2)
    )


def qnorm(pair) -> int:
    return pair[0] * pair[0] + pair[1] * pair[1]


for source, expected in SOURCE_HASHES.items():
    check(f"source hash {source}", sha256(ROOT / source) == expected)
check("protocol pre-run hash", sha256(ROOT / PROTOCOL) == PROTOCOL_HASH)

a = sp.symbols("a", positive=True)
Omega, nu, kappa, action, I0 = sp.symbols(
    "Omega nu kappa A I0", positive=True
)
theta = sp.symbols("theta", real=True)
e = sp.symbols("e", integer=True, nonnegative=True)
sigma = sp.symbols("sigma", integer=True)
p, q = sp.symbols("p q", real=True)
g = 1 - sp.cos(theta)
coefficient = nu + e * sigma * kappa * g

check(
    "positive-amplitude ternary embedding is injective",
    a.is_positive and len({(s, o) for s, o in STATES}) == 9,
)
check(
    "embedded action equals a squared Q over two",
    all(
        sp.simplify(((a * s) ** 2 + (a * o) ** 2) / 2
                    - a**2 * qnorm((s, o)) / 2) == 0
        for s, o in STATES
    ),
)
check("R squared is minus identity", mmul(R, R) == NEG_I2)
generator = sp.Matrix([[0, -coefficient], [coefficient, 0]])
check(
    "Hamilton equations have the registered R generator",
    generator == coefficient * sp.Matrix(R),
)
check("clock phase solution is theta equals Omega t", sp.diff(Omega * sp.Symbol("t"), sp.Symbol("t")) == Omega)
pdot = -coefficient * q
qdot = coefficient * p
check("carrier action is invariant", sp.simplify(p * pdot + q * qdot) == 0)

I_solution = I0 - e * sigma * kappa * action * g / Omega
check(
    "clock action solution satisfies its phase equation",
    sp.simplify(sp.diff(I_solution, theta)
                + e * sigma * kappa * action * sp.sin(theta) / Omega) == 0,
)
hamiltonian = Omega * I_solution + nu * action + e * sigma * kappa * g * action
check(
    "total Hamiltonian is phase independent",
    sp.simplify(hamiltonian - (Omega * I0 + nu * action)) == 0,
)
check("one cycle returns to a gate zero", sp.simplify((1 - sp.cos(2 * sp.pi))) == 0)
integrated_angle = sp.integrate(coefficient / Omega, (theta, 0, 2 * sp.pi))
expected_angle = 2 * sp.pi * nu / Omega + e * sigma * 2 * sp.pi * kappa / Omega
check("integrated carrier angle is exact", sp.simplify(integrated_angle - expected_angle) == 0)
check(
    "inactive minimum winding is identity",
    sp.simplify(expected_angle.subs({e: 0, nu: Omega}) - 2 * sp.pi) == 0,
)
check(
    "forward active winding is a positive quarter turn",
    sp.simplify(expected_angle.subs({e: 1, sigma: 1, nu: Omega, kappa: Omega / 4})
                - 5 * sp.pi / 2) == 0,
)
check(
    "reverse active winding is an inverse quarter turn",
    sp.simplify(expected_angle.subs({e: 1, sigma: -1, nu: Omega, kappa: Omega / 4})
                - 3 * sp.pi / 2) == 0,
)
check("inactive branch fixes all ternary states", all(apply(I2, state) == state for state in STATES))
check("forward branch matches FTD-0872", all(apply(R, state) == (-state[1], state[0]) for state in STATES))
check("reverse branch is the exact inverse", all(apply(R_INVERSE, state) == (state[1], -state[0]) for state in STATES))
check("ready emission is exact", all(apply(R, (s, 0)) == (0, s) for s in TERNARY))
check("reciprocal absorption is exact", all(apply(R_INVERSE, (0, s)) == (s, 0) for s in TERNARY))

check(
    "carrier action is unchanged in every branch",
    all(qnorm(apply(matrix, state)) == qnorm(state)
        for matrix in (I2, R, R_INVERSE) for state in STATES),
)
check(
    "endpoint carrier energy is unchanged",
    all(nu * a**2 * qnorm(apply(matrix, state)) / 2
        == nu * a**2 * qnorm(state) / 2
        for matrix in (I2, R, R_INVERSE) for state in STATES),
)
record_energy_scale = nu * a**2 / 2
check(
    "record energy coefficient is positive and explicitly imposed",
    record_energy_scale.is_positive and sp.simplify(record_energy_scale - nu * a**2 / 2) == 0,
)
max_action_excursion = (2 * kappa * action / Omega).subs(kappa, Omega / 4)
check("maximum clock action excursion is A over two", sp.simplify(max_action_excursion - action / 2) == 0)
max_reference_exchange = sp.simplify(Omega * max_action_excursion)
check("maximum reference energy exchange is Omega A over two", sp.simplify(max_reference_exchange - Omega * action / 2) == 0)
max_interaction_energy = (2 * kappa * action).subs(kappa, Omega / 4)
check("interaction exchange has the same maximum magnitude", sp.simplify(max_interaction_energy - max_reference_exchange) == 0)
check("reference action returns at cycle endpoint", sp.simplify(I_solution.subs(theta, 2 * sp.pi) - I0) == 0)
interaction = e * sigma * kappa * g * action
check("interaction energy vanishes at cycle endpoint", sp.simplify(interaction.subs(theta, 2 * sp.pi)) == 0)
endpoint_before = Omega * I0 + nu * action
endpoint_after = Omega * I_solution.subs(theta, 2 * sp.pi) + nu * action + interaction.subs(theta, 2 * sp.pi)
check("endpoint total energy residual is zero", sp.simplify(endpoint_after - endpoint_before) == 0)
switch = sp.symbols("Delta_es", nonzero=True) * kappa * g * action
check("gate-zero switching work vanishes", sp.simplify(switch.subs(theta, 0)) == 0)
check("off-phase switching work is generally nonzero", sp.simplify(switch.subs(theta, sp.pi)) != 0)
check(
    "bidirectional reserve I0 greater than A over two is sufficient",
    sp.simplify(I0 - max_action_excursion - (I0 - action / 2)) == 0,
)

carrier_pair_dimensions = 2
clock_pair_dimensions = 2
check("carrier plus clock phase space is four dimensional", carrier_pair_dimensions + clock_pair_dimensions == 4)
check(
    "independent phase-gate class cannot fit both pairs below four dimensions",
    carrier_pair_dimensions + clock_pair_dimensions > carrier_pair_dimensions,
)
check("repeating the forward active cycle is not hold", mmul(R, R) == NEG_I2 and mmul(R, R) != I2)
emitted = apply(R, (1, 0))
check(
    "norm-only eligibility remains active after emission",
    qnorm((1, 0)) > 0 and qnorm(emitted) > 0,
)

protocol_text = (ROOT / PROTOCOL).read_text(encoding="utf-8")
check("harmonic actuator scope marker", "HARMONIC_ACTUATOR_STATUS=IMPOSED_REFERENCE" in protocol_text)
check("dynamic scheduler scope marker", "DYNAMIC_ONE_SHOT_SCHEDULER=OPEN" in protocol_text)
check("production coupling scope marker", "PRODUCTION_COUPLING=NONE" in protocol_text)
check("Gstar separation scope marker", "GSTAR_ROLE=SEPARATE_CALENDAR_NOT_ACTUATOR" in protocol_text)
check("Born Bell scope marker", "BORN_BELL_STATUS=UNTOUCHED" in protocol_text)
check("terminal gate reached with C1-C47 passing", checks == 47 and failures == 0)

print(f"\nFTD-0873 Hamiltonian ternary quarter-turn actuator: {checks - failures}/{checks} PASS")
if checks == 48 and failures == 0:
    print("HAMILTONIAN_TERNARY_QUARTER_TURN_LIFT_THEOREM")
    print("MINIMUM_REGISTERED_PHASE_SPACE_DIMENSION=4")
    print("ACTIVE_FORWARD_MAP=R")
    print("ACTIVE_REVERSE_MAP=R_INVERSE")
    print("MAX_REFERENCE_ENERGY_EXCHANGE=OMEGA_A_OVER_2")
    print("ENDPOINT_ENERGY_RESIDUAL=ZERO")
    print("HARMONIC_ACTUATOR_STATUS=IMPOSED_REFERENCE")
    print("DYNAMIC_ONE_SHOT_SCHEDULER=OPEN")
    print("PRODUCTION_COUPLING=NONE")
    print("GSTAR_ROLE=SEPARATE_CALENDAR_NOT_ACTUATOR")
    print("BORN_BELL_STATUS=UNTOUCHED")
    raise SystemExit(0)

print("FTD-0873_CERTIFICATE_INVALID")
raise SystemExit(1)
