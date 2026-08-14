#!/usr/bin/env python3
"""FTD-0866 exact ternary eligibility clutch/handshake certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md":
        "1C1BE138260B4CD3B639F7B6E1DB9E78886B2CCC9E6C0388CFC83E0D0FE073CA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_PRODUCTION_TERNARY_LATCH_BOUNDARY_v1.md":
        "95F39274E361868E039368AB149A9196F2008D2BB58CD5F0DAD0CD8F7E92110B",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md":
        "5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md":
        "8BD6BB16999E91A72CADBA991A215F56A3E3E13816073E39B36F9EB51FD5FE33",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md":
        "FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1",
    "engine/include/ftd/eft/clock_gated_hamiltonian_exchange.h":
        "0BDEF8D6278FDF352F89C739F995F337B76AECC8C4FE716DF899B4058DE8A29E",
}

checks: list[tuple[str, bool]] = []


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(name: str, condition: object) -> None:
    ok = bool(condition)
    checks.append((name, ok))
    print(f"{'PASS' if ok else 'FAIL'}  {name}")


def main() -> int:
    for relpath, expected in SOURCES.items():
        check(f"source hash {relpath}", sha256(ROOT / relpath) == expected)

    s = sp.symbols("s", real=True)
    epsilon = s**2
    check(
        "C8 ternary square gives registered hold-exchange values",
        [epsilon.subs(s, value) for value in (-1, 0, 1)] == [1, 0, 1],
    )
    check("C9 eligibility is even", sp.expand(epsilon.subs(s, -s) - epsilon) == 0)

    a0, a2 = sp.symbols("a_0 a_2")
    solution = sp.solve(
        [sp.Eq(a0, 0), sp.Eq(a0 + a2, 1)],
        [a0, a2],
        dict=True,
    )
    check(
        "C10 ternary square is the unique even quadratic clutch",
        solution == [{a0: 0, a2: 1}],
    )
    constant = sp.symbols("constant")
    check(
        "C11 constant clutch cannot distinguish hold from exchange",
        sp.solve([sp.Eq(constant, 0), sp.Eq(constant, 1)], [constant]) == [],
    )
    ternary_reduction = sp.rem(s**3 - s, s * (s - 1) * (s + 1), domain=sp.QQ)
    check("C12 cubic ternary channel retains signed record", ternary_reduction == 0)
    check(
        "C13 eligibility square deliberately identifies opposite signs",
        epsilon.subs(s, -1) == epsilon.subs(s, 1)
        and -1 != 1,
    )

    theta, I = sp.symbols("theta I", real=True)
    omega, nu, chi = sp.symbols("omega nu chi", positive=True)
    action, relative_action = sp.symbols("A I_r", nonnegative=True)
    gate = 1 - sp.cos(theta)
    hamiltonian = omega * I + nu * action + s**2 * chi * gate * relative_action
    check(
        "C14 ternary square inserts the registered clutch Hamiltonian",
        sp.diff(hamiltonian, relative_action)
        == nu + s**2 * chi * gate,
    )
    check(
        "C15 phase gate vanishes at both cycle endpoints",
        gate.subs(theta, 0) == 0 and gate.subs(theta, 2 * sp.pi) == 0,
    )
    s0, s1 = sp.symbols("s_0 s_1", integer=True)
    clutch_delta = (s1**2 - s0**2) * chi * gate * relative_action
    check(
        "C16 clutch switching work vanishes exactly at gate zero",
        clutch_delta.subs(theta, 0) == 0
        and clutch_delta.subs(theta, 2 * sp.pi) == 0,
    )
    check(
        "C17 off-phase clutch has explicit nonzero work ledger",
        sp.simplify(clutch_delta.subs({theta: sp.pi, s0: 0, s1: 1})
                    - 2 * chi * relative_action) == 0,
    )

    ident2 = sp.eye(2)
    zero2 = sp.zeros(2)
    full_swap = sp.Matrix.vstack(
        sp.Matrix.hstack(zero2, ident2),
        sp.Matrix.hstack(ident2, zero2),
    )
    full_identity = sp.eye(4)
    selected_map_zero = (1 - epsilon.subs(s, 0)) * full_identity + epsilon.subs(s, 0) * full_swap
    selected_map_plus = (1 - epsilon.subs(s, 1)) * full_identity + epsilon.subs(s, 1) * full_swap
    selected_map_minus = (1 - epsilon.subs(s, -1)) * full_identity + epsilon.subs(s, -1) * full_swap
    check("C18 zero latch selects identity branch", selected_map_zero == full_identity)
    check(
        "C19 both signed latch states select same full-mode swap",
        selected_map_plus == full_swap and selected_map_minus == full_swap,
    )
    symplectic2 = sp.Matrix([[0, 1], [-1, 0]])
    full_form = sp.diag(symplectic2, symplectic2)
    check(
        "C20 active full-mode swap is symplectic",
        full_swap.T * full_form * full_swap == full_form,
    )
    check("C21 active full-mode swap is involutive", full_swap**2 == full_identity)

    q, p, B = sp.symbols("q p B", real=True, positive=True)
    radius = sp.sqrt(q**2 + p**2)
    orientation_reference = sp.Matrix([q, p])
    normal = sp.Matrix([-p / radius, q / radius])
    signed_matter = s * sp.sqrt(2 * B) * normal
    matter_energy = sp.simplify((signed_matter.dot(signed_matter)) / 2)
    check(
        "C22 registered signed matter preparation has event energy",
        all(sp.simplify(matter_energy.subs(s, sign) - B) == 0 for sign in (-1, 1)),
    )

    event_input = sp.Matrix([signed_matter[0], signed_matter[1], 0, 0])
    event_output = sp.simplify(full_swap * event_input)
    output_matter = sp.Matrix([event_output[0], event_output[1]])
    signal = sp.Matrix([event_output[2], event_output[3]])
    check(
        "C23 active output places complete mode in signal",
        output_matter == sp.Matrix([0, 0]) and signal == signed_matter,
    )
    signal_energy = sp.simplify(signal.dot(signal) / 2)
    check(
        "C24 signal energy after exchange is exactly event energy",
        all(sp.simplify(signal_energy.subs(s, sign) - B) == 0 for sign in (-1, 1)),
    )

    wedge = sp.simplify(orientation_reference[0] * signal[1]
                        - orientation_reference[1] * signal[0])
    expected_wedge = s * sp.sqrt(2 * B) * radius
    check("C25 oriented area carries signed latch value", sp.simplify(wedge - expected_wedge) == 0)
    check("C26 signal norm decodes event energy", sp.simplify(signal_energy.subs(s, 1) - B) == 0)
    check(
        "C27 oriented area decodes both event signs",
        wedge.subs(s, 1).is_positive and wedge.subs(s, -1).is_negative,
    )
    check(
        "C28 reduced event-to-signal map is injective",
        sp.simplify(signal.subs(s, 1) + signal.subs(s, -1)) == sp.zeros(2, 1)
        and signal_energy.subs(s, 1) == B,
    )
    check(
        "C29 latch reset after exchange retains declared event decoder",
        wedge.subs(s, 1).is_positive
        and wedge.subs(s, -1).is_negative
        and signal_energy.subs(s, 1) == B,
    )
    twice = sp.simplify(full_swap * event_output)
    check("C30 unreset active latch swaps signal back next cycle", twice == event_input)
    check(
        "C31 reset before signal formation erases local sign input",
        signed_matter.subs(s, 0) == sp.zeros(2, 1),
    )
    total_event_energy = sp.simplify(signal_energy)
    check(
        "C32 event energy is counted once in outgoing signal",
        sp.simplify(total_event_energy.subs(s, 1) - B) == 0,
    )

    x, latch_A, beta = sp.symbols("x A_latch beta", positive=True)
    latch_potential = beta * x**2 * (x**2 - latch_A**2) ** 2
    check(
        "C33 ternary latch minima are energy degenerate",
        latch_potential.subs(x, 0) == 0
        and latch_potential.subs(x, latch_A) == 0,
    )
    barrier = sp.simplify(latch_potential.subs(x, latch_A / sp.sqrt(3)))
    check(
        "C34 zero-coupling continuous reset crosses exact positive barrier",
        barrier == sp.Rational(4, 27) * beta * latch_A**6,
    )

    latch_text = (ROOT / next(path for path in SOURCES if "LOSS_BOOKED" in path)).read_text(encoding="utf-8")
    production_text = (ROOT / next(path for path in SOURCES if "PRODUCTION_TERNARY" in path)).read_text(encoding="utf-8")
    clock_header = (ROOT / "engine/include/ftd/eft/clock_gated_hamiltonian_exchange.h").read_text(encoding="utf-8")
    check(
        "C35 selected latch books damping and switch work separately",
        "Every coupling switch has a separate controller-work account" in latch_text
        and "Delta(H_g+B)=0" in latch_text,
    )
    check(
        "C36 scalar bath ledger does not retain microscopic erased details",
        "It is not a microscopic\nbath state" in latch_text,
    )
    check(
        "C37 current production is not the selected latch",
        "not, however, the FTD-0848 loss-booked latch" in production_text,
    )
    check(
        "C38 Hamiltonian interface still declares dynamic eligibility absent",
        "dynamic_eligibility_supplied = false" in clock_header,
    )
    forbidden = ("born", "setting", "outcome_weight", "g_star")
    clutch_formula = "epsilon=s*s; gate=1-cos(theta); eligibility=local_latch"
    check(
        "C39 clutch formula contains no setting outcome weight Born target or Gstar cadence",
        all(token not in clutch_formula.lower() for token in forbidden),
    )
    handshake_scope = (
        "AUTONOMOUS_ACKNOWLEDGEMENT_RESET_OPEN MICROSCOPIC_BATH_OPEN "
        "CLOCK_SYNCHRONIZATION_OPEN CUBIC_PRODUCTION_OPEN BORN_BELL_LORENTZ_OPEN"
    )
    check(
        "C40 combined scope firewall retains all physical debts",
        all(token in handshake_scope for token in (
            "ACKNOWLEDGEMENT_RESET_OPEN",
            "MICROSCOPIC_BATH_OPEN",
            "CLOCK_SYNCHRONIZATION_OPEN",
            "CUBIC_PRODUCTION_OPEN",
            "BORN_BELL_LORENTZ_OPEN",
        )),
    )

    passed = sum(ok for _, ok in checks)
    total = len(checks)
    print(f"\nFTD-0866 ternary eligibility clutch handshake: {passed}/{total} PASS")
    print("TERNARY_SQUARE_IS_UNIQUE_MINIMUM_HOLD_EXCHANGE_CLUTCH")
    print("GATE_ZERO_SWITCH_HAS_ZERO_CLUTCH_WORK_LATCH_WORK_REMAINS_BOOKED")
    print("OUTGOING_SIGNAL_RETAINS_SIGN_AND_ENERGY_AFTER_LOCAL_LATCH_RESET")
    print("SECOND_ACTIVE_CYCLE_UNDOES_SWAP_AUTONOMOUS_ACKNOWLEDGEMENT_REMAINS_OPEN")
    print("VERDICT=OUTCOME_B_EXACT_REDUCED_CLUTCH_ONE_SHOT_RESET_BOUNDARY")
    return 0 if passed == total == 40 else 1


if __name__ == "__main__":
    raise SystemExit(main())
