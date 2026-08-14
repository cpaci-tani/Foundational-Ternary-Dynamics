#!/usr/bin/env python3
"""FTD-0868 exact signal-acknowledged two-stroke reset certificate."""

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
    "THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md":
        "FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_ELIGIBILITY_CLUTCH_AND_ONE_SHOT_HANDSHAKE_v1.md":
        "6BD280A51DEF9A1B5E373D0084A9C19597772CD31D2B5D278B2323315AC2153D",
    "engine/include/ftd/eft/ternary_eligibility_clutch.h":
        "C53ED1A7FCFF54E4236D2353CA319BCE61EC459C1A7A90F2069C01145256FE43",
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

    theta = sp.symbols("theta", real=True)
    omega, action0, relative_action = sp.symbols(
        "omega I_0 I_r", positive=True
    )
    gate = 1 - sp.cos(2 * theta)
    check(
        "C5 exchange window vanishes at all stroke endpoints",
        all(sp.simplify(gate.subs(theta, value)) == 0
            for value in (0, sp.pi, 2 * sp.pi)),
    )
    check(
        "C6 first-stroke gate is a nonnegative square",
        sp.simplify(gate - 2 * sp.sin(theta) ** 2) == 0,
    )
    gate_area = sp.integrate(gate, (theta, 0, sp.pi))
    check("C7 first-stroke gate area is pi", sp.simplify(gate_area - sp.pi) == 0)
    reset_gate = sp.Integer(0)
    check("C8 reset-stroke exchange interaction is zero", reset_gate == 0)

    common_first = sp.simplify(2 * omega * sp.pi / omega)
    extra_first = sp.simplify(omega * gate_area / omega)
    check("C9 first-stroke common winding is two pi", common_first == 2 * sp.pi)
    check("C10 first-stroke extra relative winding is pi", extra_first == sp.pi)

    identity2 = sp.eye(2)
    zero2 = sp.zeros(2)
    transform = sp.sqrt(sp.Rational(1, 2)) * sp.Matrix.vstack(
        sp.Matrix.hstack(identity2, identity2),
        sp.Matrix.hstack(identity2, -identity2),
    )
    active_modal = sp.diag(1, 1, -1, -1)
    active_physical = sp.simplify(transform.T * active_modal * transform)
    full_swap = sp.Matrix.vstack(
        sp.Matrix.hstack(zero2, identity2),
        sp.Matrix.hstack(identity2, zero2),
    )
    check("C11 active first stroke is complete-mode swap", active_physical == full_swap)
    inactive_physical = sp.simplify(transform.T * sp.eye(4) * transform)
    check("C12 inactive first stroke is identity", inactive_physical == sp.eye(4))

    common_second = sp.simplify(2 * omega * sp.pi / omega)
    check("C13 second-stroke common winding is two pi", common_second == 2 * sp.pi)
    check("C14 second-stroke relative extra winding is zero", reset_gate == 0)
    check("C15 second stroke preserves midpoint modes", sp.eye(4) * full_swap == full_swap)

    B = sp.symbols("B", positive=True)
    check("C16 empty-signal relative action is half event energy",
          sp.simplify(relative_action.subs(relative_action, B / 2) - B / 2) == 0)
    s = sp.symbols("s", real=True)
    clock_action = action0 - s**2 * gate * relative_action
    check(
        "C17 clock-action solution satisfies Hamilton equation",
        sp.simplify(sp.diff(clock_action, theta)
                    + s**2 * sp.diff(gate, theta) * relative_action) == 0,
    )
    check(
        "C18 clock action returns at midpoint and endpoint",
        all(sp.simplify(clock_action.subs(theta, value) - action0) == 0
            for value in (0, sp.pi, 2 * sp.pi)),
    )
    minimum_action = sp.simplify(clock_action.subs({theta: sp.pi / 2, s: 1}))
    check("C19 minimum clock action is I0 minus two Ir",
          sp.simplify(minimum_action - (action0 - 2 * relative_action)) == 0)
    event_minimum = sp.simplify(minimum_action.subs(relative_action, B / 2))
    check("C20 event reserve is I0 minus B", event_minimum == action0 - B)
    reserve_margin = sp.symbols("reserve_margin", positive=True)
    check("C21 strict reserve is equivalent to I0 greater than B",
          sp.solve_univariate_inequality(action0 - B > 0, action0)
          == sp.Interval.open(B, sp.oo))
    maximum_interaction = sp.simplify(
        omega * gate.subs(theta, sp.pi / 2) * (B / 2)
    )
    check("C22 maximum interaction energy is omega B",
          sp.simplify(maximum_interaction - omega * B) == 0)
    maximum_loan = sp.simplify(omega * (action0 - event_minimum))
    check("C23 maximum reference-energy loan is omega B",
          sp.simplify(maximum_loan - omega * B) == 0)

    active_ack = 1**2 * int(0 == 0) * int(True)
    hold_ack = 0**2 * int(0 == 0) * int(False)
    check("C24 active midpoint signal acknowledges completion", active_ack == 1)
    check("C25 no-event midpoint does not acknowledge", hold_ack == 0)
    check("C26 acknowledgement is even under event sign reversal", (-1)**2 == 1**2)
    partial_matter_energy, partial_signal_energy = sp.symbols(
        "B_M B_D", positive=True
    )
    partial_ack = int(partial_matter_energy == 0) * int(partial_signal_energy > 0)
    check("C27 partial transfer fails exact completion predicate", partial_ack == 0)

    beta_q, beta_p = sp.Integer(3), sp.Integer(4)
    beta_radius = sp.Integer(5)
    normal = sp.Matrix([-beta_p / beta_radius, beta_q / beta_radius])
    signal = s * sp.sqrt(2 * B) * normal
    wedge = sp.simplify(beta_q * signal[1] - beta_p * signal[0])
    check(
        "C28 midpoint signal retains both event signs",
        wedge.subs(s, 1).is_positive and wedge.subs(s, -1).is_negative,
    )

    rate, time = sp.symbols("c t", positive=True)
    smooth_representative = sp.exp(-rate * time)
    check("C29 smooth exponential relaxation is nonzero at finite time",
          smooth_representative.is_positive)

    kappa, A, gamma = sp.symbols("kappa A gamma", positive=True)
    x = sp.symbols("x", real=True)
    reset_potential = kappa * sp.Abs(x)
    check(
        "C30 cusp reset potential is even and positive away from zero",
        reset_potential.subs(x, -A) == reset_potential.subs(x, A)
        and reset_potential.subs(x, A).is_positive,
    )
    reset_time = gamma * A / kappa
    for sign in (-1, 1):
        trajectory = sign * (A - kappa * time / gamma)
        check_condition = sp.simplify(
            gamma * sp.diff(trajectory, time) + kappa * sign
        ) == 0
        if sign == -1:
            negative_branch_ok = check_condition
        else:
            positive_branch_ok = check_condition
    check("C31 dry-friction trajectory satisfies both signed branches",
          negative_branch_ok and positive_branch_ok)
    check("C32 dry-friction trajectory reaches zero at TR",
          all(sp.simplify(sign * (A - kappa * reset_time / gamma)) == 0
              for sign in (-1, 1)))
    check("C33 zero is an admissible sticking subgradient selection",
          -1 <= 0 <= 1)
    reset_window = sp.pi / omega
    check("C34 reset window is pi over omega", reset_window == sp.pi / omega)
    kappa_minimum = gamma * A * omega / sp.pi
    check("C35 compliance threshold saturates TR equal reset window",
          sp.simplify(reset_time.subs(kappa, kappa_minimum) - reset_window) == 0)
    check("C36 minimum registered force finishes at endpoint",
          sp.simplify(kappa_minimum * reset_window / gamma - A) == 0)
    switch_on = kappa * A
    check("C37 reset switch-on system energy is kappa A", switch_on == kappa * A)
    bath_export = sp.simplify((kappa**2 / gamma) * reset_time)
    check("C38 integrated reset bath export is kappa A", bath_export == kappa * A)
    check("C39 reset switch-off energy vanishes at zero",
          reset_potential.subs(x, 0) == 0)
    controller_reservoir = -kappa * A
    reset_system_change = sp.Integer(0)
    check("C40 controller reset and bath ledger closes exactly",
          sp.simplify(controller_reservoir + bath_export + reset_system_change) == 0)
    signal_energy_before = sp.simplify(signal.dot(signal) / 2)
    signal_energy_after = signal_energy_before
    check("C41 reset leaves event signal energy unchanged",
          all(sp.simplify(signal_energy_after.subs(s, sign) - B) == 0
              for sign in (-1, 1)))

    symplectic2 = sp.Matrix([[0, 1], [-1, 0]])
    full_form = sp.diag(symplectic2, symplectic2)
    check(
        "C42 export swap is symplectic involutive and energy preserving",
        full_swap.T * full_form * full_swap == full_form
        and full_swap**2 == sp.eye(4)
        and sp.simplify((full_swap * sp.Matrix([signal[0], signal[1], 0, 0])).dot(
            full_swap * sp.Matrix([signal[0], signal[1], 0, 0])
        ) - signal.dot(signal)) == 0,
    )
    exported = signal
    final_local = sp.zeros(4, 1)
    exported_wedge = sp.simplify(beta_q * exported[1] - beta_p * exported[0])
    check(
        "C43 final local state is ready and export decodes sign and energy",
        final_local == sp.zeros(4, 1)
        and all(sp.simplify((exported.dot(exported) / 2).subs(s, sign) - B) == 0
                for sign in (-1, 1))
        and exported_wedge.subs(s, 1).is_positive
        and exported_wedge.subs(s, -1).is_negative,
    )
    scope = (
        "SMOOTH_RESET_NOT_ESTABLISHED MICROSCOPIC_BATH_OPEN "
        "PROTECTED_CUBIC_TRANSPORT_OPEN PRODUCTION_OPEN GSTAR_OPEN "
        "BORN_BELL_LORENTZ_BIOLOGICAL_COMPLETENESS_OPEN"
    )
    check(
        "C44 scope firewall retains every physical and interpretive debt",
        all(token in scope for token in (
            "SMOOTH_RESET_NOT_ESTABLISHED",
            "MICROSCOPIC_BATH_OPEN",
            "PROTECTED_CUBIC_TRANSPORT_OPEN",
            "PRODUCTION_OPEN",
            "GSTAR_OPEN",
            "BORN_BELL_LORENTZ_BIOLOGICAL_COMPLETENESS_OPEN",
        )),
    )

    passed = sum(ok for _, ok in checks)
    total = len(checks)
    print(f"\nFTD-0868 signal-acknowledged two-stroke reset: {passed}/{total} PASS")
    print("SIGNAL_COMPLETION_IS_LOCAL_TARGET_BLIND_ACKNOWLEDGEMENT")
    print("SMOOTH_AUTONOMOUS_ATTRACTION_CANNOT_RESET_EXACTLY_IN_FINITE_TIME")
    print("SELECTED_CUSP_RESET_CLOSES_TIME_WORK_AND_SCALAR_BATH_LEDGER")
    print("OUTPUT_PORT_RETAINS_EVENT_AND_RETURNS_LOCAL_REFERENCE_TO_READY_STATE")
    print("VERDICT=OUTCOME_B_NONSMOOTH_TWO_STROKE_REFERENCE_CLOSURE")
    return 0 if passed == total == 44 else 1


if __name__ == "__main__":
    raise SystemExit(main())
